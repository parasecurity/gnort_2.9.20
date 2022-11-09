#ifndef _DFAMATCH_H_
#define _DFAMATCH_H_

#include <CL/opencl.h>

/* dfa match kernel wrapper */
void
kdfamatch(cl_kernel kernel, cl_command_queue queue,
    cl_mem kbuf,
    cl_mem koffary,
    cl_mem klenary,
    cl_mem kretary,
    cl_uint cnt,
    cl_uint bufsiz,
    cl_mem ktrans,
    size_t blksiz,
    cl_event *event,
    cl_mem kretchar);

#endif /* _DFAMATCH_H_ */
