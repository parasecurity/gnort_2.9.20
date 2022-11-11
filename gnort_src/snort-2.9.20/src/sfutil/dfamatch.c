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
#include "acsmx2.h"
#include "fpdetect.h"


/* dfa match kernel wrapper */
void
kdfamatch(cl_kernel kernel, cl_command_queue queue, cl_mem kbuf, cl_mem koffary,
    cl_mem klenary, cl_mem kretary, cl_uint cnt, cl_uint bufsiz, cl_mem ktrans,
    size_t blksiz, cl_event *event, cl_mem kretchar, cl_mem kretpktid)
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
	clSetKernelArg(kernel, 7, sizeof(cl_mem), &kretchar);
	clSetKernelArg(kernel, 8, sizeof(cl_mem), &kretpktid);

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
	    pb->cnt, pb->used, xdfa->ktrans, blksiz, &cl->event, pb->kretchar, pb->kretpktid);
}

#ifdef DFA_MATCH_MAIN
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
#endif



// argyris===========================================================
int xdfa_search(struct xdfactx *xdfa, unsigned char *Tx, int n,
				int (*Match)(void * id, void *tree, int index, void *data, void *neg_list),
           		void *data){
	
	char pkt[1500];
	// unsigned char * pkt;
	// pkt = (unsigned char *)malloc(sizeof(unsigned char));
	struct pattern * mlist;

	OTNX_MATCH_DATA ** omd;
	void ** datalist;

	

	// printf("Tx: %s.\n", Tx);
	int size_payload;
	size_payload = n;

	unsigned char *T_new;
	


	// decode the Tx string
	int i;
	for(i = 0; i < size_payload; i++) {
		if (i >= 1500){
			return -1;
		}
		if (isprint(Tx[i])){
			pkt[i] = Tx[i];
		}
		else{
			pkt[i] = '.';
			// pkt[i] = Tx[i];
		}
	}
	if (i >= 1500){
		return -1;
	}
	// pkt[i] = '\0';
	// printf("pkt: %s.\n", pkt);

	xdfa->omd[xdfa->xpb->in->cnt] = (OTNX_MATCH_DATA *)data;
	// printf("id: %d\n", xdfa->xpb->in->cnt);


	// xdfa->payloads[xdfa->xpb->in->cnt] = (unsigned char *)pkt;

	// xdfa->omd[xdfa->xpb->in->cnt] = (OTNX_MATCH_DATA *)data;
	// xdfa->payloads[xdfa->xpb->in->cnt] = (unsigned char *)pkt;

	xdfa->omd[xdfa->xpb->in->cnt] = (OTNX_MATCH_DATA *)data;
	xdfa->datalist[xdfa->xpb->in->cnt] = data;
	xdfa->payloads[xdfa->xpb->in->cnt] = (unsigned char *)pkt;

	if(xpktbuf_addtoin(xdfa->xpb, (unsigned char *)pkt, strlen(pkt) + 1) > 0){
		
		// printf("omd[%d]: %s\n",xdfa->xpb->in->cnt - 1, xdfa->omd[xdfa->xpb->in->cnt - 1]->p);
        // printf("Returned!\n");
		return 0;
    }

	// if(xpktbuf_addtoin(xdfa->xpb, Tx, n) > 0){
    //     // printf("Returned!\n");
	// 	return 0;
    // }

	DPRINTF_D(xdfa->xpb->in->cnt);
    DPRINTF_D(xdfa->xpb->in->used);

    xpktbuf_syncstreams(xdfa->xpb);
    
    /* erase return cells */
    memset(xdfa->xpb->tmp->res, 0, xdfa->queuesiz * sizeof(unsigned int));
	memset(xdfa->xpb->tmp->reschar, 0, xdfa->queuesiz * sizeof(unsigned int));
    
    gettimeofday(&xdfa->start_time, NULL);

    /* run */
    xpktbuf_copytodev(xdfa->xpb);

    kdfamatchpb(&xdfa->cl, xdfa->xpb->tmp, xdfa, xdfa->blksiz);
    xpktbuf_copytohost(xdfa->xpb);

    gettimeofday(&xdfa->end_time, NULL);
    xdfa->elapsed_time = (xdfa->end_time.tv_sec * 1000 + xdfa->end_time.tv_usec) - (xdfa->start_time.tv_sec * 1000 + xdfa->start_time.tv_usec); 

    /* get results */
    xdfa->e = clFinish(xdfa->xpb->queue);
    if (xdfa->e != CL_SUCCESS)
        ERRXV(1, "finish: %s", clstrerror(xdfa->e));

    for (int i = 0; i < xdfa->xpb->tmp->cnt; i++) {
        if (xdfa->xpb->tmp->res[i] != 0) {		// or < 0
			mlist = xdfa->mlist[xdfa->xpb->tmp->res[i]][xdfa->xpb->tmp->reschar[i]];


			// Match (mlist->udata, mlist->rule_option_tree, 0, omd[i], mlist->neg_list);
			// printf("id: %d\n", (int)mlist->udata);
			int k;
			for (k = 0; k < xdfa->acsm->acsmMaxStates; k++){
				if (xdfa->acsm->acsmMatchList[k]){
					if (mlist->udata == xdfa->acsm->acsmMatchList[k]->udata){
						// printf("udata: %d\n", mlist->udata);
						// printf("omd[]->p: %s", xdfa->omd[i]->p);
						Match (xdfa->acsm->acsmMatchList[k]->udata, xdfa->acsm->acsmMatchList[k]->rule_option_tree, 0, xdfa->omd[(int)xdfa->xpb->tmp->retpktid[i]], xdfa->acsm->acsmMatchList[k]->neg_list);
						Match (mlist->udata, mlist->rule_option_tree, 0, xdfa->omd[(int)xdfa->xpb->tmp->retpktid[i]], mlist->neg_list);

						Match (xdfa->acsm->acsmMatchList[k]->udata, xdfa->acsm->acsmMatchList[k]->rule_option_tree, 0, xdfa->datalist[(int)xdfa->xpb->tmp->retpktid[i]], xdfa->acsm->acsmMatchList[k]->neg_list);
						Match (mlist->udata, mlist->rule_option_tree, 0, xdfa->datalist[(int)xdfa->xpb->tmp->retpktid[i]], mlist->neg_list);
					}
				}
			}
            if (xdfa->verbose == 1){
                printf("Match[%d]: \n", i);
                printf("off: %s\n", &xdfa->xpb->tmp->buf[xdfa->xpb->tmp->off[i]]);
				// printf("buf: %s\n", xdfa->xpb->tmp->buf);
				printf("pattern: %s\n", mlist->patrn);
				printf("gpuid: %d, trueid: %hu\n", i, xdfa->xpb->tmp->retpktid[i]); // this is always the same
				// printf("pattern: %s\n", xdfa->mlist[xdfa->xpb->tmp->res[i]][xdfa->xpb->tmp->reschar[i]]->patrn);
            }		
            xdfa->matches++;
        }
    }
	xdfa->omd[xdfa->xpb->in->cnt] = (OTNX_MATCH_DATA *)data;
	xdfa->datalist[xdfa->xpb->in->cnt] = data;
	xdfa->payloads[xdfa->xpb->in->cnt] = (unsigned char *)pkt;
    xpktbuf_addtoin(xdfa->xpb, (unsigned char *)pkt, strlen(pkt + 1));


	return 0;
}
// ===========================================================
#ifdef DFA_MATCH_MAIN
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

			for (int i = 0; i < xpb->tmp->cnt; i++) {
				if (xpb->tmp->res[i]) {
					if (verbose == 1)
						printf(&xpb->tmp->buf[xpb->tmp->off[i]]);
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
				if (verbose == 1)
					printf(&xpb->tmp->buf[xpb->tmp->off[i]]);
				matches++;
			}
		}
		xpktbuf_addtoin(xpb, (unsigned char *)buf, strlen(buf + 1));
	}

	return 0;
}

#endif
