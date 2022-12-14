#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <CL/opencl.h>
#include <sys/time.h>

#include "common.h"
#include "clutil.h"
#include "dfamatch.h"
#include "xdfa.h"
#include "xpktbuf.h"
#include "hacks.h"


/* dfa match kernel wrapper */
void
kdfamatch(cl_kernel kernel, cl_command_queue queue, cl_mem kbuf, cl_mem koffary,
    cl_mem klenary, cl_mem kretary, cl_uint cnt, cl_uint bufsiz, cl_mem ktrans,
    size_t blksiz, cl_event *event)
{
	int e;
	static int counter = 0;
	int ret[65536];
	size_t global = ROUNDUP(cnt, blksiz);
	size_t local = blksiz;

#ifdef _DFAMATCH_DEBUG
	for (int i = 0; i < 65536; i++) 
		ret[i] = 0;

	e = clEnqueueWriteBuffer(queue, kretary, CL_TRUE, 0,
	    sizeof(int) * 65536, ret, 0, NULL, NULL);
#endif /* _DFAMATCH_DEBUG */

	/* call arguments */
	clSetKernelArg(kernel, 0, sizeof(cl_mem), &kbuf);
	clSetKernelArg(kernel, 1, sizeof(cl_mem), &koffary);
	clSetKernelArg(kernel, 2, sizeof(cl_mem), &klenary);
	clSetKernelArg(kernel, 3, sizeof(cl_mem), &kretary);
	clSetKernelArg(kernel, 4, sizeof(cl_uint), &cnt);
	clSetKernelArg(kernel, 5, sizeof(cl_uint), &bufsiz);
	clSetKernelArg(kernel, 6, sizeof(cl_mem), &ktrans);

	/* execution */
	e = clEnqueueNDRangeKernel(queue, kernel, 1, NULL,
	    &global, &local, 0, NULL, event);
	if (e != CL_SUCCESS)
		ERRXV(1, "execute: %s", clstrerror(e));
	if (event != NULL)
		e = clWaitForEvents(1, event);
	if (e != CL_SUCCESS)
		ERRXV(1, "event: %s", clstrerror(e));
	clFlush(queue);

#ifdef _DFAMATCH_DEBUG
	e = clEnqueueReadBuffer(queue, kretary, CL_TRUE, 0, sizeof(int) * 65536,
	    ret, 0, NULL, NULL);
	for (e = 0; e < 65536; e++) {
		if (ret[e] == 1)
			counter++;
	}
	printf("[matches]\t%d\n", counter);
	counter = 0;
#endif /* _DFAMATCH_DEBUG */

	clFlush(queue);
}

/* dfa match packet buffer kernel wrapper */
void
kdfamatchpb(struct clconf *cl, struct pktbuf *pb, struct xdfactx *xdfa,
    size_t blksiz)
{
	kdfamatch(cl->kernel, cl->queue, pb->kbuf, pb->koff, pb->klen, pb->kres,
	    pb->cnt, pb->used, xdfa->ktrans, blksiz, &cl->event);
}

void
load_patterns(struct xdfactx *xdfa, char *path)
{
	char buf[100];

	FILE *fd = fopen(path, "r");
	if (fd == NULL) {
		perror("");
		exit(-1);
	}

	while (fgets(buf, 100, fd)) {
		xdfa_addpattern(xdfa, buf, strlen(buf));
	}

	fclose(fd);

	return;
}

int
main(int argc, char *argv[])
{
	int opt, e;
	int devpos = 0; /* the first available device */
	int mapped = 1; /* unset for discrete devices */
	int verbose = 0; /* do not print matches by default*/
	unsigned int matches = 0;
	unsigned int queuesiz = 8192; /* size of packet buffer */
	unsigned int blksiz = 16; /* partitioning */
	unsigned long long elapsed_time;
	FILE *input = stdin; /* default input stream */
	char *patterns = "patterns.dat"; /* path of default patterns file */
	char buf[1500];
	struct clconf cl;
	struct xpktbuf *xpb;
	struct xdfactx *xdfa;
	struct timeval start_time, end_time;

	while ((opt = getopt(argc, argv, ":d:i:m:p:q:v")) != -1) {
		switch (opt) {
		case 'd':
			devpos = atoi(optarg);
			break;
		case 'i':
			if ((input = fopen(optarg, "r")) == NULL) {
				perror("");
				exit(-1);
			}
			break;
		case 'm':
			mapped = atoi(optarg);
			break;
		case 'p':
			patterns = optarg;
			break;
		case 'q':
			queuesiz = atoi(optarg);
			break;
		case 'v':
			verbose = 1;
			break;
		case ':':
			printf("Missing option value, proceeding with the defaults\n");
			break;
		case '?':
			printf("Unknown option: -%c\n", optopt);
			break;
		}
	}

	/* create context and build program */
	clsetupf(&cl, "src/dfamatch.cl", "dfamatch", NULL, devpos, -1);

	/* create a packet buffer */
#define MAXPKTLEN 1536
	xpb = xpktbuf_new(queuesiz, queuesiz * MAXPKTLEN, sizeof(unsigned int),
	    mapped ? XPB_MAPPED : XPB_DEFAULT, cl.ctx, cl.queue);

	/* create the dfa, load some patterns from a file and compile it  */
	xdfa = xdfa_new();
	load_patterns(xdfa, patterns);
	xdfa_compile(xdfa);

	/* map transition table to device memory */
	xdfa_cl_init(xdfa, cl.ctx);
	DPRINTF_LU(256 * xdfa->trcount * sizeof(int));

	while (1) {
		if (fgets(buf, 1500, input) == NULL) {
			DPRINTF_D(xpb->in->cnt);
			DPRINTF_D(xpb->in->used);

			xpktbuf_syncstreams(xpb);
			
			/* erase return cells */
			memset(xpb->tmp->res, 0, queuesiz * sizeof(unsigned int));

			gettimeofday(&start_time, NULL);
			
			/* run */
			xpktbuf_copytodev(xpb);
			kdfamatchpb(&cl, xpb->tmp, xdfa, blksiz);
			xpktbuf_copytohost(xpb);
		
			gettimeofday(&end_time, NULL);
			elapsed_time = (end_time.tv_sec * 1000 + end_time.tv_usec) - (start_time.tv_sec * 1000 + start_time.tv_usec); 


			/* get results */
			e = clFinish(xpb->queue);
			if (e != CL_SUCCESS)
				ERRXV(1, "finish: %s", clstrerror(e));
			printf("\n");
			for (int i = 0; i < xpb->tmp->cnt; i++) {
				if (xpb->tmp->res[i]) {
					if (verbose == 1){
						printf("\nMatch: ", i);
						printf(&xpb->tmp->buf[xpb->tmp->off[i]]);
						printf("\n");
					}

					matches++;
				}
			}

			printf("Total matches: %d\n", matches);
			printf("Execution time (usecs): %lld\n", elapsed_time);
			break;
		}

		if(xpktbuf_addtoin(xpb, (unsigned char *)buf, strlen(buf) + 1) > 0)
			continue;

		DPRINTF_D(xpb->in->cnt);
		DPRINTF_D(xpb->in->used);

		xpktbuf_syncstreams(xpb);
		
		/* erase return cells */
		memset(xpb->tmp->res, 0, queuesiz * sizeof(unsigned int));
		
		gettimeofday(&start_time, NULL);

		/* run */
		xpktbuf_copytodev(xpb);
		kdfamatchpb(&cl, xpb->tmp, xdfa, blksiz);
		xpktbuf_copytohost(xpb);

		gettimeofday(&end_time, NULL);
		elapsed_time = (end_time.tv_sec * 1000 + end_time.tv_usec) - (start_time.tv_sec * 1000 + start_time.tv_usec); 

		/* get results */
		e = clFinish(xpb->queue);
		if (e != CL_SUCCESS)
			ERRXV(1, "finish: %s", clstrerror(e));

		for (int i = 0; i < xpb->tmp->cnt; i++) {
			if (xpb->tmp->res[i]) {
				if (verbose == 1){
					printf("\nMatch: ", i);
					printf(&xpb->tmp->buf[xpb->tmp->off[i]]);
					printf("\n");			
				}		
				matches++;
			}
		}
		xpktbuf_addtoin(xpb, (unsigned char *)buf, strlen(buf + 1));
	}

	return 0;
}
