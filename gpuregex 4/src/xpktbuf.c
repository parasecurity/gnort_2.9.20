#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common.h"
#include "clutil.h"
#include "xpktbuf.h"

#define MEM_READ(addr)		(*((volatile  unsigned int *)(addr)))
#define MEM_WRITE(addr,data)	(*((volatile  unsigned int *)(addr)) = data)
/*
 * Creates a new packet buffer that can hold up to max packets
 * or bufsiz bytes.  Result values should take reslen bytes each.
 */
struct pktbuf *
pktbuf_new(unsigned int max, unsigned int bufsiz, int ismapped,
    unsigned int reslen, cl_context ctx, cl_command_queue queue)
{
	struct pktbuf *pb;
	int e;

	pb = (struct pktbuf *)malloc(sizeof(struct pktbuf));
	if (pb == NULL)
		ERR(1, "malloc pb");

	/* allocate device buffers */
	pb->kbuf = clCreateBuffer(ctx,
	    CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR,
	    bufsiz * sizeof(cl_uchar), NULL, &e);
	if (e != CL_SUCCESS)
		ERRXV(1, "alloc buf: %s", clstrerror(e));
	pb->koff = clCreateBuffer(ctx,
	    CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR,
	    max * sizeof(cl_int), NULL, &e);
	if (e != CL_SUCCESS)
		ERRXV(1, "alloc off: %s", clstrerror(e));
	pb->klen = clCreateBuffer(ctx,
	    CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR,
	    max * sizeof(cl_ushort), NULL, &e);
	if (e != CL_SUCCESS)
		ERRXV(1, "alloc len: %s", clstrerror(e));
	pb->kusr = clCreateBuffer(ctx,
	    CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR,
	    max * sizeof(cl_uchar), NULL, &e);
	if (e != CL_SUCCESS)
		ERRXV(1, "alloc usr: %s", clstrerror(e));
	pb->kres = clCreateBuffer(ctx,
	    CL_MEM_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR,
	    max * reslen, NULL, &e);
	if (e != CL_SUCCESS)
		ERRXV(1, "alloc res: %s", clstrerror(e));

	if (ismapped) {
		/* map buffers to host */
		pb->buf = clEnqueueMapBuffer(queue, pb->kbuf, CL_TRUE,
		    CL_MAP_READ | CL_MAP_WRITE, 0,
		    bufsiz * sizeof(unsigned char),
		    0, NULL, NULL, &e);
		if (e != CL_SUCCESS)
			ERRXV(1, "map buf: %s", clstrerror(e));
		pb->off = clEnqueueMapBuffer(queue, pb->koff, CL_TRUE,
		    CL_MAP_READ | CL_MAP_WRITE, 0,
		    max * sizeof(int),
		    0, NULL, NULL, &e);
		if (e != CL_SUCCESS)
			ERRXV(1, "map off: %s", clstrerror(e));
		pb->len = clEnqueueMapBuffer(queue, pb->klen, CL_TRUE,
		    CL_MAP_READ | CL_MAP_WRITE, 0,
		    max * sizeof(unsigned short),
		    0, NULL, NULL, &e);
		if (e != CL_SUCCESS)
			ERRXV(1, "map len: %s", clstrerror(e));
		pb->usr = clEnqueueMapBuffer(queue, pb->kusr, CL_TRUE,
		    CL_MAP_READ | CL_MAP_WRITE, 0,
		    max * sizeof(unsigned char),
		    0, NULL, NULL, &e);
		if (e != CL_SUCCESS)
			ERRXV(1, "map usr: %s", clstrerror(e));
		pb->res = clEnqueueMapBuffer(queue, pb->kres, CL_TRUE,
		    CL_MAP_READ | CL_MAP_WRITE, 0,
		    max * reslen,
		    0, NULL, NULL, &e);
		if (e != CL_SUCCESS)
			ERRXV(1, "map res: %s", clstrerror(e));
	} else {
		/* allocate host buffers */
		pb->buf = MALLOC(bufsiz * sizeof(unsigned char));
		if (pb->buf == NULL)
			ERR(1, "malloc buf");
		pb->off = MALLOC(max * sizeof(int));
		if (pb->off == NULL)
			ERR(1, "malloc off");
		pb->len = MALLOC(max * sizeof(unsigned short));
		if (pb->len == NULL)
			ERR(1, "malloc len");
		pb->usr = MALLOC(max * sizeof(unsigned char));
		if (pb->usr == NULL)
			ERR(1, "malloc usr");
		pb->res = MALLOC(max * reslen);
		if (pb->res == NULL)
			ERR(1, "malloc res");

		/* register host buffers */
		pb->ibuf = clCreateBuffer(ctx,
		    CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
		    bufsiz * sizeof(cl_uchar), pb->buf, &e);
		if (e != CL_SUCCESS)
			ERRXV(1, "register buf: %s", clstrerror(e));
		pb->ioff = clCreateBuffer(ctx,
		    CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
		    max * sizeof(cl_int), pb->off, &e);
		if (e != CL_SUCCESS)
			ERRXV(1, "register off: %s", clstrerror(e));
		pb->ilen = clCreateBuffer(ctx,
		    CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
		    max * sizeof(cl_ushort), pb->len, &e);
		if (e != CL_SUCCESS)
			ERRXV(1, "register len: %s", clstrerror(e));
		pb->iusr = clCreateBuffer(ctx,
		    CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
		    max * sizeof(cl_uchar), pb->usr, &e);
		if (e != CL_SUCCESS)
			ERRXV(1, "register usr: %s", clstrerror(e));
		pb->ires = clCreateBuffer(ctx,
		    CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
		    max * reslen, pb->res, &e);
		if (e != CL_SUCCESS)
			ERRXV(1, "register res: %s", clstrerror(e));
	}

	pb->max = max;
	pb->cnt = 0;
	pb->pos = 0;
	pb->bufsiz = bufsiz;
	pb->used = 0;
	pb->reslen = reslen;

	return pb;
}

/*
 * Frees all memory used by the packet buffer.
 */
void
pktbuf_free(struct pktbuf *pb, int ismapped, cl_command_queue queue)
{
	cl_uint e;

	if (ismapped) {
		e = clEnqueueUnmapMemObject(queue, pb->kbuf, pb->buf,
		    0, NULL, NULL);
		if (e != CL_SUCCESS)
			ERRXV(1, "enqueue release mem object:  %s",
			    clstrerror(e));
		e = clEnqueueUnmapMemObject(queue, pb->koff, pb->off,
		    0, NULL, NULL);
		if (e != CL_SUCCESS)
			ERRXV(1, "enqueue unmap mem object:  %s",
			    clstrerror(e));
		e = clEnqueueUnmapMemObject(queue, pb->klen, pb->len,
		    0, NULL, NULL);
		if (e != CL_SUCCESS)
			ERRXV(1, "enqueue unmap mem object:  %s",
			    clstrerror(e));
		e = clEnqueueUnmapMemObject(queue, pb->kusr, pb->usr,
		    0, NULL, NULL);
		if (e != CL_SUCCESS)
			ERRXV(1, "enqueue unmap mem object:  %s",
			    clstrerror(e));
		e = clEnqueueUnmapMemObject(queue, pb->kres, pb->res,
		    0, NULL, NULL);
		if (e != CL_SUCCESS)
			ERRXV(1, "enqueue unmap mem object:  %s",
			    clstrerror(e));
	} else {
		e = clReleaseMemObject(pb->ibuf);
		if (e != CL_SUCCESS)
			ERRXV(1, "release mem object:  %s", clstrerror(e));
		e = clReleaseMemObject(pb->ioff);
		if (e != CL_SUCCESS)
			ERRXV(1, "release mem object:  %s", clstrerror(e));
		e = clReleaseMemObject(pb->ilen);
		if (e != CL_SUCCESS)
			ERRXV(1, "release mem object:  %s", clstrerror(e));
		e = clReleaseMemObject(pb->iusr);
		if (e != CL_SUCCESS)
			ERRXV(1, "release mem object:  %s", clstrerror(e));
		e = clReleaseMemObject(pb->ires);
		if (e != CL_SUCCESS)
			ERRXV(1, "release mem object:  %s", clstrerror(e));
		FREE(pb->buf);
		FREE(pb->off);
		FREE(pb->len);
		FREE(pb->usr);
		FREE(pb->res);
	}
	e = clReleaseMemObject(pb->kbuf);
	if (e != CL_SUCCESS)
		ERRXV(1, "release mem object:  %s", clstrerror(e));
	e = clReleaseMemObject(pb->koff);
	if (e != CL_SUCCESS)
		ERRXV(1, "release mem object:  %s", clstrerror(e));
	e = clReleaseMemObject(pb->klen);
	if (e != CL_SUCCESS)
		ERRXV(1, "release mem object:  %s", clstrerror(e));
	e = clReleaseMemObject(pb->kusr);
	if (e != CL_SUCCESS)
		ERRXV(1, "release mem object:  %s", clstrerror(e));
	e = clReleaseMemObject(pb->kres);
	if (e != CL_SUCCESS)
		ERRXV(1, "release mem object:  %s", clstrerror(e));
	free(pb);

	return;
}

/*
 * Allocates packet buffers for both host and device use.
 * The flags argument defines runtime options.
 */
struct xpktbuf *
xpktbuf_new(unsigned int max, unsigned int bufsiz, unsigned int reslen,
    int flags, cl_context ctx, cl_command_queue queue)
{
	struct xpktbuf *xpb;
	int ismapped = flags & XPB_MAPPED;

	if ((flags & XPB_MAPPED) && (flags & XPB_RDALL))
		ERRX(1, "invalid flags");

	if (max < 0)
		ERRX(1, "negative max");

	xpb = (struct xpktbuf *)malloc(sizeof(struct xpktbuf));
	if (xpb == NULL)
		ERR(1, "malloc xpb");

	xpb->in = pktbuf_new(max, bufsiz, ismapped, reslen, ctx, queue);
	xpb->out = pktbuf_new(max, bufsiz, ismapped, reslen, ctx, queue);
	xpb->tmp = pktbuf_new(max, bufsiz, ismapped, reslen, ctx, queue);

	xpb->flags = flags;
	xpb->queue = queue;

	return xpb;
}

/*
 * Frees all packet buffers on both the host and the device.
 */
void
xpktbuf_free(struct xpktbuf *xpb)
{
	int ismapped = xpb->flags & XPB_MAPPED;

	pktbuf_free(xpb->in, ismapped, xpb->queue);
	pktbuf_free(xpb->out, ismapped, xpb->queue);
	pktbuf_free(xpb->tmp, ismapped, xpb->queue);
	free(xpb);

	return;
}

/*
 * Returns the packet counter.
 */
unsigned int
pktbuf_getcnt(struct pktbuf *pb)
{
	return pb->cnt;
}

/*
 * Returns 1 if the packet buffer is full, 0 otherwise.
 */
unsigned int
pktbuf_isfull(struct pktbuf *pb)
{
	return pb->cnt == pb->max;
}

/*
 * Returns the bytes used.
 */
unsigned int
pktbuf_getused(struct pktbuf *pb)
{
	return pb->used;
}

/*
 * Appends one packet of length len to the host buffer pointed to by pb.
 * Returns the number of bytes copied.  On error returns:
 *     -1 when the max packets number is reached
 *     -2 when the buffer size would be exceeded
 */
int
pktbuf_add(struct pktbuf *pb, unsigned char *pkt, unsigned short len)
{
#define ALIGNCEIL(x, y) (((((x) - 1) / (y)) + 1) * (y))
	if (pb->cnt == pb->max)
		return -1;
	if (pb->used + ALIGNCEIL(len, 16) > pb->bufsiz)
		return -2;

	memcpy(pb->buf + pb->used, pkt, len);
	pb->off[pb->cnt] = pb->used;
	pb->len[pb->cnt] = len;
	pb->used += ALIGNCEIL(len, 16);
	pb->cnt++;

	return len;
}

/*
 * Appends one packet to the input packet buffer.
 */
int
xpktbuf_addtoin(struct xpktbuf *xpb, unsigned char *pkt, unsigned short len)
{
	return pktbuf_add(xpb->in, pkt, len);
}

/*
 * Clears the packet buffer pointed to by pb.
 */
void
pktbuf_clear(struct pktbuf *pb)
{
	/* Zero status counters */
	pb->cnt = 0;
	pb->used = 0;
	pb->pos = 0; /* Reset the iterator */
	return;
}

/*
 * Synchronizes the device operations and does buffer swapping
 * to prepare for the next round.  Afterwards, the input buffer is
 * empty, the swap buffer contains all input so far and the output
 * buffer holds all packets that were previously in the swap buffer.
 */
void
xpktbuf_syncstreams(struct xpktbuf *xpb)
{
	int e;
	
	e = clFinish(xpb->queue);
	if (e != CL_SUCCESS)
		ERRXV(1, "finish: %s", clstrerror(e));

#define SWAP(x, y) \
    do { void *tmp; tmp = (x), (x) = (y), (y) = tmp; } while (0)
	SWAP(xpb->tmp, xpb->out); /* Old swap goes to output */
	SWAP(xpb->in, xpb->tmp); /* Current input is swapped */

	pktbuf_clear(xpb->in);

	return;
}

/*
 * Copies the swap buffer over to the device
 * if not already mapped.
 */
void
xpktbuf_copytodev(struct xpktbuf *xpb)
{
	int e;

	if (xpb->flags & XPB_MAPPED)
		return;

	/* always write {buf, off, len} to dev */
	e = clEnqueueWriteBuffer(xpb->queue, xpb->tmp->kbuf,
	    CL_TRUE, 0, xpb->tmp->used * sizeof(cl_uchar),
	    xpb->tmp->buf, 0, NULL, NULL);
	if (e != CL_SUCCESS)
		ERRXV(1, "write buf: %s", clstrerror(e));


	e = clEnqueueWriteBuffer(xpb->queue, xpb->tmp->koff,
	    CL_TRUE, 0, xpb->tmp->cnt * sizeof(cl_int),
	    xpb->tmp->off, 0, NULL, NULL);
	if (e != CL_SUCCESS)
		ERRXV(1, "write off: %s", clstrerror(e));
	e = clEnqueueWriteBuffer(xpb->queue, xpb->tmp->klen,
	    CL_TRUE, 0, xpb->tmp->cnt * sizeof(cl_ushort),
	    xpb->tmp->len, 0, NULL, NULL);
	if (e != CL_SUCCESS)
		ERRXV(1, "write len: %s", clstrerror(e));
	/* optionally write usr to dev */
	if (xpb->flags & XPB_WRUSR) {
		e = clEnqueueWriteBuffer(xpb->queue, xpb->tmp->kusr,
		    CL_TRUE, 0, xpb->tmp->cnt * sizeof(cl_uchar),
		    xpb->tmp->usr, 0, NULL, NULL);
		if (e != CL_SUCCESS)
			ERRXV(1, "write usr: %s", clstrerror(e));
	}
	/* never write res to dev */

	return;
}

/*
 * Copies the device buffer back to the swap buffer
 * if not already mapped.
 */
void
xpktbuf_copytohost(struct xpktbuf *xpb)
{
	int e;

	if (xpb->flags & XPB_MAPPED)
		return;

	if (xpb->flags & XPB_RDBUF) {
		e = clEnqueueReadBuffer(xpb->queue, xpb->tmp->kbuf,
		    CL_TRUE, 0, xpb->tmp->used * sizeof(cl_uchar),
		    xpb->tmp->buf, 0, NULL, NULL);
		if (e != CL_SUCCESS)
			ERRXV(1, "read buf: %s", clstrerror(e));
	}
	if (xpb->flags & XPB_RDOFF) {
		e = clEnqueueReadBuffer(xpb->queue, xpb->tmp->koff,
		    CL_TRUE, 0, xpb->tmp->cnt * sizeof(cl_int),
		    xpb->tmp->off, 0, NULL, NULL);
		if (e != CL_SUCCESS)
			ERRXV(1, "read off: %s", clstrerror(e));
	}
	if (xpb->flags & XPB_RDLEN) {
		e = clEnqueueReadBuffer(xpb->queue, xpb->tmp->klen,
		    CL_TRUE, 0, xpb->tmp->cnt * sizeof(cl_ushort),
		    xpb->tmp->len, 0, NULL, NULL);
		if (e != CL_SUCCESS)
			ERRXV(1, "read len: %s", clstrerror(e));
	}
	if (xpb->flags & XPB_RDUSR) {
		e = clEnqueueReadBuffer(xpb->queue, xpb->tmp->kusr,
		    CL_TRUE, 0, xpb->tmp->cnt * sizeof(cl_uchar),
		    xpb->tmp->usr, 0, NULL, NULL);
		if (e != CL_SUCCESS)
			ERRXV(1, "read usr: %s", clstrerror(e));
	}
	if (xpb->flags & XPB_RDRES) {
		e = clEnqueueReadBuffer(xpb->queue, xpb->tmp->kres,
		    CL_TRUE, 0, xpb->tmp->cnt * xpb->tmp->reslen,
		    xpb->tmp->res, 0, NULL, NULL);
		if (e != CL_SUCCESS)
			ERRXV(1, "read res: %s", clstrerror(e));
	}

	return;
}

/*
 * The pointer pointed to by pkt is set to point to the next packet's
 * data and the integer pointed to by len is set to the packet's length.
 * The usr argument is set to point to the user data for this packet.
 * Returns the packet length or -1 if the packet buffer is empty.
 */
int
pktbuf_next(struct pktbuf *pb, unsigned char **pkt, unsigned short *len,
    unsigned char *usr)
{
	/* Skip invalid offsets */
	while (pb->pos < pb->cnt && pb->off[pb->pos] < 0)
		pb->pos++;
	if (pb->pos == pb->cnt)
		return -1;

	*pkt = pb->buf + pb->off[pb->pos];
	*len = pb->len[pb->pos];
	*usr = pb->usr[pb->pos];
	pb->pos++;

	return *len;
}

/*
 * Returns 1 if the packet buffer is empty and 0 otherwise.
 */
int
pktbuf_isempty(struct pktbuf *pb)
{
	return (pb->pos == pb->cnt);
}

/*
 * Returns the number of packets left on the buffer.
 */
unsigned int
pktbuf_nleft(struct pktbuf *pb)
{
	return (pb->cnt - pb->pos);
}

/*
 * Gets the next packet from the output buffer.
 */
int
xpktbuf_nextout(struct xpktbuf *xpb, unsigned char **pkt, unsigned short *len,
    unsigned char *usr)
{
	return pktbuf_next(xpb->out, pkt, len, usr);
}

/*
 * Checks if the output buffer has been consumed.
 */
int
xpktbuf_isemptyout(struct xpktbuf *xpb)
{
	return pktbuf_isempty(xpb->out);
}

/*
 * Returns the number of packets left on the output buffer.
 */
unsigned int
xpktbuf_nleftout(struct xpktbuf *xpb)
{
	return pktbuf_nleft(xpb->out);
}

/*
 * Print debug info for the packet buffer.
 */
void
pktbuf_debug(struct pktbuf *pb)
{
	int i;

	printf("max=%d cnt=%d pos=%d bufsiz=%d used=%d\n",
	    pb->max, pb->cnt, pb->pos, pb->bufsiz, pb->used);
	printf("off:");
	for (i = pb->pos; i < pb->cnt; i++)
		printf(" %d", pb->off[i]);
	printf("\n");
	printf("len:");
	for (i = pb->pos; i < pb->cnt; i++)
		printf(" %d", pb->len[i]);
	printf("\n");
	return;
}

/*
 * Fill the packet buffer with textual data from the file in path.
 */
void
pktbuf_fill(struct pktbuf *pb, const char *path)
{
	FILE *fp;
	char line[128];
	unsigned int i;

	fp = fopen(path, "r");
	if (fp == NULL)
		ERRXV(1, "fopen: %s", path);

	for (i = 0; i < pb->cnt; i++) {
		int off = 0;
		while (off < pb->len[i]) {
			char *s;
			size_t len;
get:
			s = fgets(line, sizeof(line), fp);
			if (s == NULL) {
				rewind(fp);
				goto get;
			}
			len = strlen(s) - 1; /* ignore newlines */
			if (len > pb->len[i] - off)
				len = pb->len[i] - off;
			/* inject match */
			memcpy(pb->buf + pb->off[i] + off, s, len);
			off += len;
		}
	}
}

/*
 * Fill the input packet buffer with textual data from the file in path.
 */
void
xpktbuf_fillin(struct xpktbuf *xpb, const char *path)
{
	return pktbuf_fill(xpb->in, path);
}

#ifdef _UTEST

const char *dummystr = MKSTR(
__kernel void
dummy(__global unsigned char *buf, __global int *off,
    __global unsigned short *len, __global unsigned char *usr,
    __global unsigned int *res,
    const int n)
{
	int i;

	i = get_global_id(0);

	if ((off[i] % 3) == 0)
		off[i] = -1;
}
);

void
krun(struct pktbuf *pb, struct clconf *cl)
{
	int e;
	size_t global = pb->cnt;
	size_t local = 8;

	clSetKernelArg(cl->kernel, 0, sizeof(cl_mem), &(pb->kbuf));
	clSetKernelArg(cl->kernel, 1, sizeof(cl_mem), &(pb->koff));
	clSetKernelArg(cl->kernel, 2, sizeof(cl_mem), &(pb->klen));
	clSetKernelArg(cl->kernel, 3, sizeof(cl_mem), &(pb->kusr));
	clSetKernelArg(cl->kernel, 4, sizeof(cl_mem), &(pb->kres));
	clSetKernelArg(cl->kernel, 5, sizeof(cl_int), &(pb->cnt));

	e = clEnqueueNDRangeKernel(cl->queue, cl->kernel, 1, NULL,
	    &global, &local, 0, NULL, NULL);
	if (e != CL_SUCCESS)
		ERRXV(1, "execute: %s", clstrerror(e));
}

int
main(void)
{
	int i;
	short len;
	unsigned char usr;
	struct xpktbuf *xpb;
	unsigned char buf[9999], *p;
	int offary[] = {0, 111, 222, 333, 444, 555, 666, 777, 888};
	unsigned short lenary[] = {2, 4, 8, 16, 32, 64, 32, 16, 8};
	unsigned int n = 9;
	struct clconf cl;

	int mapped = 1; /* unset for discrete devices */
	clsetup(&cl, dummystr, "dummy", NULL, 0, -1);

	memset(buf, 'X', 9999);

#define MAXPKTLEN 1500

	printf("test addtoin:\n");
	xpb = xpktbuf_new(32, 32 * MAXPKTLEN, sizeof(unsigned int),
	    mapped ? XPB_MAPPED : XPB_DEFAULT,
	    cl.ctx, cl.queue);

	pktbuf_debug(xpb->in); /* in is empty */
	while (xpktbuf_addtoin(xpb, buf, 128) > 0)
		printf(">");
	printf("\n");
	pktbuf_debug(xpb->in); /* in is full */
	xpktbuf_syncstreams(xpb);
	krun(xpb->tmp, &cl);
	pktbuf_debug(xpb->tmp); /* tmp is full */
	xpktbuf_syncstreams(xpb);
	pktbuf_debug(xpb->out); /* out is full */
	while (xpktbuf_nextout(xpb, &p, &len, &usr) > 0)
		printf("<");
	printf("\n");
	pktbuf_debug(xpb->out); /* out is empty */

	xpktbuf_free(xpb);

	return 0;
}
#endif /* _UTEST */
