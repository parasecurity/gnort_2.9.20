#ifndef _XDFA_H_
#define _XDFA_H_

#include <CL/opencl.h>

#include "dfa.h"

struct xdfactx {
	struct dfa *dfa;
	unsigned int maxpatlen;
	unsigned int trcount;
	unsigned int nregexps;
	int *trans;
	cl_mem ktrans;
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
