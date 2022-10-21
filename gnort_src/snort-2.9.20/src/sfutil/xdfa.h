#ifndef _XDFA_H_
#define _XDFA_H_

#include <CL/opencl.h>

#include "dfa.h"
#include "clutil.h"

#define MAXPKTLEN 1536

struct xdfactx {
	struct dfa *dfa;
	unsigned int maxpatlen;
	unsigned int trcount;
	unsigned int nregexps;
	int *trans;
	cl_mem ktrans;

	// argyris
	int opt, e;
	int devpos; /* the first available device */
	int mapped; /* 0 for discrete devices */
	int verbose; /* do not print matches by default*/
	unsigned int matches;
	unsigned int queuesiz; /* size of packet buffer */
	unsigned int blksiz; /* partitioning */
	unsigned long long elapsed_time;
	// FILE *input = stdin; /* default input stream */
	// char *patterns = "patterns.dat"; /* path of default patterns file */
	// char buf[1500];
	struct clconf cl;
	struct xpktbuf *xpb;
	// struct xdfactx *xdfa;
	struct timeval start_time, end_time;
};

struct xdfactx *
xdfa_new(void);

void
xdfa_addpattern(struct xdfactx *xdfa, char *pattern, unsigned int length);

void
xdfa_compile(struct xdfactx *xdfa);

void
xdfa_dump(struct xdfactx *xdfa, const char *path, int print);

void
xdfa_load(struct xdfactx *xdfa, const char *path);

/* helper to copy transition table to the device */
void
xdfa_cl_init(struct xdfactx *xdfa, cl_context ctx);

#endif /* _XDFA_H_ */
