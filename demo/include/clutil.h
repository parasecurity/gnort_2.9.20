#ifndef _CLUTIL_H_
#define _CLUTIL_H_

#include <CL/opencl.h>

/* opencl config */
struct clconf {
	cl_platform_id platform;
	cl_device_id dev;
	cl_context ctx;
	cl_command_queue queue;
	cl_program program;
	cl_kernel kernel;
	cl_device_type type;
	cl_event event;
};

/* initialize device with device specific information */
void
clinitdev(struct clconf *cl, int pos);

/* create context and build program */
void
clsetup(struct clconf *cl, const char *kstr, const char *kname,
    const char *opts, int pos, int subpos);

/* create context and build program from file */
void
clsetupf(struct clconf *cl, const char *kpath, const char *kname,
    const char *opts, int pos, int subpos);

/* create context */
void
clinitctx(struct clconf *cl, int pos, int subpos);

/* initialize the environment */
void clinitenv(struct clconf *);

/* build program */
void
clinitcode(struct clconf *cl, const char *kstr, const char *kname,
    const char *opts);

/* build program from file */
void
clinitcodef(struct clconf *cl, const char *kpath, const char *kname,
    const char *opts);

/* print the program build log */
void
clputlog(struct clconf *cl);

/* error code to string */
char *
clstrerror(int err);

#endif /* _CLUTIL_H_ */
