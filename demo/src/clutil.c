#include <stdio.h>
#include <stdlib.h>

/* XXX README XXX
 * The `clCreateCommandQueue` function became deprecated after OpenCL 2.0, and 
 * replaced with the `clCreateCommandQueueWithProperties` function. Run the 
 * clinfo utility in order to check the OpenCL that each device uses. If the 
 * targeted devices support OpenCL 2.0, you can safely use the new function. 
 * Otherwise, use the following definition `CL_USE_DEPRECATED_OPENCL_1_2_APIS`,
 * to avoid any errors/warnings about deprecated functions. 
 */
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#define CL_TARGET_OPENCL_VERSION 120 // argyris, force opencl 1.2 version

/* ^^^ */

#include <CL/opencl.h>
#include <CL/cl_ext.h>

#include "common.h"
#include "clutil.h"

/* load source code */
char *
strload(const char *path);

void
clinitdev(struct clconf *cl, int pos)
{
	int e;
	int i;
	int k = 0; /* global index */
	int done = 0;
	cl_uint nplatforms;
	cl_platform_id *platform;

	/* get platforms */
	e = clGetPlatformIDs(0, NULL, &nplatforms);
	if (e != CL_SUCCESS)
		ERRXV(1, "nplatforms: %s", clstrerror(e));
	DPRINTF_U(nplatforms);
	platform = calloc(nplatforms, sizeof(cl_platform_id));
	e = clGetPlatformIDs(nplatforms, platform, NULL);
	if (e != CL_SUCCESS)
		ERRXV(1, "platform: %s", clstrerror(e));

	/* for all platforms */
	for (i = 0; i < nplatforms; i++) {
		int j;
		cl_uint ndevs;
		cl_device_id *dev;

		/* get devices */
		e = clGetDeviceIDs(platform[i], CL_DEVICE_TYPE_ALL,
		    0, NULL, &ndevs);
		if (e != CL_SUCCESS)
			ERRXV(1, "ndevs: %s", clstrerror(e));
		DPRINTF_U(ndevs);
		dev = calloc(ndevs, sizeof(cl_device_id));
		e = clGetDeviceIDs(platform[i], CL_DEVICE_TYPE_ALL,
		    ndevs, dev, NULL);
		if (e != CL_SUCCESS)
			ERRXV(1, "dev: %s", clstrerror(e));

		/* for all devices */
		for (j = 0; j < ndevs; j++) {
			if (k++ == pos) {
				cl->platform = platform[i];
				cl->dev = dev[j];
				done = 1;
			}
		}

		free(dev);

	}

	free(platform);

	if (!done)
		ERRX(1, "invalid dev pos");

	e = clGetDeviceInfo(cl->dev, CL_DEVICE_TYPE,
	    sizeof(cl->type), &(cl->type), NULL);
	if (e != CL_SUCCESS)
		ERRXV(1, "type: %s", clstrerror(e));
}

void
clsetup(struct clconf *cl, const char *kstr, const char *kname,
    const char *opts, int pos, int subpos)
{
	clinitctx(cl, pos, subpos);
	clinitcode(cl, kstr, kname, opts);
}

void
clsetupf(struct clconf *cl, const char *kpath, const char *kname,
    const char *opts, int pos, int subpos)
{
	clinitctx(cl, pos, subpos);
	clinitcodef(cl, kpath, kname, opts);
}

void
clinitcodef(struct clconf *cl, const char *kpath, const char *kname,
    const char *opts)
{
	char *kstr;

	kstr = strload(kpath);
	DPRINTF_S(kstr);
	clinitcode(cl, kstr, kname, opts);
	free(kstr);
}

void
clinitctx(struct clconf *cl, int pos, int subpos)
{
	int e;
	int i;
	int k = 0; /* global index */
	int done = 0;
	cl_uint nplatforms;
	cl_platform_id *platform;

	/* get platforms */
	e = clGetPlatformIDs(0, NULL, &nplatforms);
	if (e != CL_SUCCESS)
		ERRXV(1, "nplatforms: %s", clstrerror(e));
	DPRINTF_U(nplatforms);
	platform = calloc(nplatforms, sizeof(cl_platform_id));
	e = clGetPlatformIDs(nplatforms, platform, NULL);
	if (e != CL_SUCCESS)
		ERRXV(1, "platform: %s", clstrerror(e));

	/* for all platforms */
	for (i = 0; i < nplatforms; i++) {
		int j;
		cl_uint ndevs;
		cl_device_id *dev;

		/* get devices */
		e = clGetDeviceIDs(platform[i], CL_DEVICE_TYPE_ALL,
		    0, NULL, &ndevs);
		if (e != CL_SUCCESS)
			ERRXV(1, "ndevs: %s", clstrerror(e));
		DPRINTF_U(ndevs);
		dev = calloc(ndevs, sizeof(cl_device_id));
		e = clGetDeviceIDs(platform[i], CL_DEVICE_TYPE_ALL,
		    ndevs, dev, NULL);
		if (e != CL_SUCCESS)
			ERRXV(1, "dev: %s", clstrerror(e));

		/* for all devices */
		for (j = 0; j < ndevs; j++) {
			if (k++ == pos) {
				cl->platform = platform[i];
				cl->dev = dev[j];
				done = 1;
			}
		}

		free(dev);

	}

	free(platform);

	if (!done)
		ERRX(1, "invalid dev pos");

#ifdef _DEVPART
#define cl_device_partition_property cl_device_partition_property_ext
#define CL_PROPERTIES_LIST_END CL_PROPERTIES_LIST_END_EXT
#define CLSUB_MAX clsub_max_compute_units
/* clCreateSubDevices is available for 
 * devices on OpenCL v.1.2 and above. */

	/* Use CLSUB_NUM and CLSUB_IDX to select some of the CPU cores. */

	int clsub_num = 4; /* zero means disabled by default */
	int clsub_idx = subpos; /* the first num cores by default */
	char * clsub_mode = strdup("EQUALLY"); /* partition equally by default */
	/* any device has at least 1 compute unit by default */
	int clsub_max_compute_units = 1;
	const char *opt;

	/* getenv with " sudo -E ./a.out " for sudoers */
	opt = getenv("CLSUB_MODE");
	if (opt != NULL)
		clsub_mode = strdup(opt);
	opt = getenv("CLSUB_NUM");
	if (opt != NULL)
		clsub_num = atoi(opt);
	opt = getenv("CLSUB_IDX");
	if (opt != NULL)
		clsub_idx = atoi(opt);
	opt = getenv("CLSUB_MAX_COMPUTE_UNITS");
	if (opt != NULL)
		clsub_max_compute_units = atoi(opt);
	
	/* device partition */
	if (strcmp(clsub_mode, "EQUALLY") == 0 && clsub_num != 0) {
	/* equally : default mode or environment variable (CLSUB_MODE) */
		cl_uint ndevs;
		cl_device_id idev = cl->dev;
		cl_device_partition_property prop[] =
		    {CL_DEVICE_PARTITION_EQUALLY,
		     clsub_num,
		     CL_PROPERTIES_LIST_END};
		cl_device_id odev[CLSUB_MAX];
		cl_uint odevlen = CLSUB_MAX;

		e = clCreateSubDevices(idev, prop, odevlen, odev, &ndevs);
		if (e != CL_SUCCESS)
			ERRXV(1, "subdev: %s", clstrerror(e));
		cl->dev = odev[clsub_idx]; /* select the idx subdevice */
	} else if (strcmp(clsub_mode, "BY_COUNTS") == 0) {
	/* by counts : environment variable (CLSUB_MODE) */
	/* The choice of the cores is NOT sequential starting from 0. */
	/* If you  need specific core sequence, use the property partition by names. */
		cl_uint ndevs;
		cl_device_id idev = cl->dev;
		cl_device_partition_property prop[] =
		    {CL_DEVICE_PARTITION_BY_COUNTS,
		     1,
		     CL_DEVICE_PARTITION_BY_COUNTS_LIST_END,
		     CL_PROPERTIES_LIST_END};
		cl_device_id odev[CLSUB_MAX];
		cl_uint odevlen = CLSUB_MAX;

		e = clCreateSubDevices(idev, prop, odevlen, odev, &ndevs);
		if (e != CL_SUCCESS)
			ERRXV(1, "subdev: %s", clstrerror(e));
		cl->dev = odev[clsub_idx]; /* select the idx subdevice */
	} else if (strcmp(clsub_mode, "BY_NAMES") == 0) {
	/* by names : environment variable (CLSUB_MODE) */
	/* This property is an Intel Extension. Include CL/cl_ext.h */
	/* For examples on this extension, see: 
	 * https://www.khronos.org/registry/cl/extensions/ext/cl_ext_device_fission.txt */
	/* Each clCreateSubDevices call returns only one subdevice
	 * that contains the cores given in the properties list. */
		cl_uint ndevs;
		cl_device_id idev = cl->dev;
		cl_device_partition_property prop[] =
		    {CL_DEVICE_PARTITION_BY_NAMES_EXT,
		     0, 1, 2, 3, 4, 6,
		     CL_PARTITION_BY_NAMES_LIST_END_EXT,
		     CL_PROPERTIES_LIST_END_EXT};
		cl_device_id odev[CLSUB_MAX];
		cl_uint odevlen = CLSUB_MAX;

		e = clCreateSubDevices(idev, prop, odevlen, odev, &ndevs);
		if (e != CL_SUCCESS)
			ERRXV(1, "subdev: %s", clstrerror(e));
		cl->dev = odev[clsub_idx]; /* select the idx subdevice */
	}

#endif /* _DEVPART */

	/* create environment */
	cl->ctx = clCreateContext(NULL, 1, &(cl->dev), NULL, NULL, &e);
	if (e != CL_SUCCESS)
		ERRXV(1, "ctx: %s", clstrerror(e));
	cl->queue = clCreateCommandQueue(cl->ctx, cl->dev,
	    CL_QUEUE_PROFILING_ENABLE, &e);
	if (e != CL_SUCCESS)
		ERRXV(1, "queue: %s", clstrerror(e));
	e = clGetDeviceInfo(cl->dev, CL_DEVICE_TYPE,
	    sizeof(cl->type), &(cl->type), NULL);
	if (e != CL_SUCCESS)
		ERRXV(1, "type: %s", clstrerror(e));

	return;
}

void
clinitenv(struct clconf *cl)
{
	int e;

	cl->ctx = clCreateContext(NULL, 1, &(cl->dev), NULL, NULL, &e);
	if (e != CL_SUCCESS)
		ERRXV(1, "ctx: %s", clstrerror(e));
	cl->queue = clCreateCommandQueue(cl->ctx, cl->dev,
	    CL_QUEUE_PROFILING_ENABLE, &e);
	if (e != CL_SUCCESS)
		ERRXV(1, "queue: %s", clstrerror(e));
	e = clGetDeviceInfo(cl->dev, CL_DEVICE_TYPE,
	    sizeof(cl->type), &(cl->type), NULL);
	if (e != CL_SUCCESS)
		ERRXV(1, "type: %s", clstrerror(e));

	return;
}

void
clinitcode(struct clconf *cl, const char *kstr, const char *kname,
    const char *opts)
{
	int e;
	char *optbuf;
	unsigned int optlen;

	/* add cwd to include path to keep the amd sdk happy */
#define CWDINCSTR "-I./ "
	optlen = strlen(CWDINCSTR) + 1;
	if (opts != NULL)
		optlen += strlen(opts);
	optbuf = calloc(1, optlen);
	if (optbuf == NULL)
		ERRX(1, "malloc optbuf");
	strcpy(optbuf, CWDINCSTR);
	if (opts != NULL)
		strcpy(optbuf, opts);

	DPRINTF_S(opts);

	/* generate code */
	cl->program = clCreateProgramWithSource(cl->ctx, 1,
	    &kstr, NULL, &e);
	if (e != CL_SUCCESS)
		ERRXV(1, "program: %s", clstrerror(e));
	e = clBuildProgram(cl->program, 0, NULL, optbuf, NULL, NULL);
	clputlog(cl);
	if (e != CL_SUCCESS)
		ERRXV(1, "build: %s", clstrerror(e));
	cl->kernel = clCreateKernel(cl->program, kname, &e);
	if (e != CL_SUCCESS)
		ERRXV(1, "kernel: %s", clstrerror(e));

	return;
}

void
clputlog(struct clconf *cl)
{
	int e;
	cl_build_status status;
	size_t logsiz;
	char *log;

	e = clGetProgramBuildInfo(cl->program, cl->dev,
	    CL_PROGRAM_BUILD_STATUS, sizeof(cl_build_status), &status,
	    NULL);
	if (e != CL_SUCCESS)
		ERRXV(1, "status: %s", clstrerror(e));

	if (status == CL_BUILD_SUCCESS)
		return;

	e = clGetProgramBuildInfo(cl->program, cl->dev,
	    CL_PROGRAM_BUILD_LOG, 0, NULL, &logsiz);

	DPRINTF_U(logsiz);

	log = malloc(logsiz);
	clGetProgramBuildInfo(cl->program, cl->dev,
	    CL_PROGRAM_BUILD_LOG, logsiz, log, NULL);

	fprintf(stderr, "%s\n", log);

	return;
}

char *
strload(const char *path)
{
	FILE *fp;
	char *str;
	size_t len;

	/* find out the size */
	fp = fopen(path, "r");
	if (fp == NULL)
		ERRXV(1, "fopen: %s", path);
	fseek(fp, 0, SEEK_END);
	len = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	/* allocate and load */
	str = calloc(len + 1, sizeof(char));
	if (fread(str, sizeof(char), len, fp) != len)
		ERRX(1, "fread");
	str[len] = '\0'; /* terminate */

	fclose(fp);

	return str;
}

/* error code to string */
char *
clstrerror(int err)
{
	static char ebuf[64];

	switch (err) {
	case CL_SUCCESS:
		return "CL_SUCCESS";
	case CL_DEVICE_NOT_FOUND:
		return "CL_DEVICE_NOT_FOUND";
	case CL_DEVICE_NOT_AVAILABLE:
		return "CL_DEVICE_NOT_AVAILABLE";
	case CL_COMPILER_NOT_AVAILABLE:
		return "CL_COMPILER_NOT_AVAILABLE";
	case CL_MEM_OBJECT_ALLOCATION_FAILURE:
		return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
	case CL_OUT_OF_RESOURCES:
		return "CL_OUT_OF_RESOURCES";
	case CL_OUT_OF_HOST_MEMORY:
		return "CL_OUT_OF_HOST_MEMORY";
	case CL_PROFILING_INFO_NOT_AVAILABLE:
		return "CL_PROFILING_INFO_NOT_AVAILABLE";
	case CL_MEM_COPY_OVERLAP:
		return "CL_MEM_COPY_OVERLAP";
	case CL_IMAGE_FORMAT_MISMATCH:
		return "CL_IMAGE_FORMAT_MISMATCH";
	case CL_IMAGE_FORMAT_NOT_SUPPORTED:
		return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
	case CL_BUILD_PROGRAM_FAILURE:
		return "CL_BUILD_PROGRAM_FAILURE";
	case CL_MAP_FAILURE:
		return "CL_MAP_FAILURE";
	case CL_MISALIGNED_SUB_BUFFER_OFFSET:
		return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
	case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:
		return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";

	case CL_INVALID_VALUE:
		return "CL_INVALID_VALUE";
	case CL_INVALID_DEVICE_TYPE:
		return "CL_INVALID_DEVICE_TYPE";
	case CL_INVALID_PLATFORM:
		return "CL_INVALID_PLATFORM";
	case CL_INVALID_DEVICE:
		return "CL_INVALID_DEVICE";
	case CL_INVALID_CONTEXT:
		return "CL_INVALID_CONTEXT";
	case CL_INVALID_QUEUE_PROPERTIES:
		return "CL_INVALID_QUEUE_PROPERTIES";
	case CL_INVALID_COMMAND_QUEUE:
		return "CL_INVALID_COMMAND_QUEUE";
	case CL_INVALID_HOST_PTR:
		return "CL_INVALID_HOST_PTR";
	case CL_INVALID_MEM_OBJECT:
		return "CL_INVALID_MEM_OBJECT";
	case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:
		return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
	case CL_INVALID_IMAGE_SIZE:
		return "CL_INVALID_IMAGE_SIZE";
	case CL_INVALID_SAMPLER:
		return "CL_INVALID_SAMPLER";
	case CL_INVALID_BINARY:
		return "CL_INVALID_BINARY";
	case CL_INVALID_BUILD_OPTIONS:
		return "CL_INVALID_BUILD_OPTIONS";
	case CL_INVALID_PROGRAM:
		return "CL_INVALID_PROGRAM";
	case CL_INVALID_PROGRAM_EXECUTABLE:
		return "CL_INVALID_PROGRAM_EXECUTABLE";
	case CL_INVALID_KERNEL_NAME:
		return "CL_INVALID_KERNEL_NAME";
	case CL_INVALID_KERNEL_DEFINITION:
		return "CL_INVALID_KERNEL_DEFINITION";
	case CL_INVALID_KERNEL:
		return "CL_INVALID_KERNEL";
	case CL_INVALID_ARG_INDEX:
		return "CL_INVALID_ARG_INDEX";
	case CL_INVALID_ARG_VALUE:
		return "CL_INVALID_ARG_VALUE";
	case CL_INVALID_ARG_SIZE:
		return "CL_INVALID_ARG_SIZE";
	case CL_INVALID_KERNEL_ARGS:
		return "CL_INVALID_KERNEL_ARGS";
	case CL_INVALID_WORK_DIMENSION:
		return "CL_INVALID_WORK_DIMENSION";
	case CL_INVALID_WORK_GROUP_SIZE:
		return "CL_INVALID_WORK_GROUP_SIZE";
	case CL_INVALID_WORK_ITEM_SIZE:
		return "CL_INVALID_WORK_ITEM_SIZE";
	case CL_INVALID_GLOBAL_OFFSET:
		return "CL_INVALID_GLOBAL_OFFSET";
	case CL_INVALID_EVENT_WAIT_LIST:
		return "CL_INVALID_EVENT_WAIT_LIST";
	case CL_INVALID_EVENT:
		return "CL_INVALID_EVENT";
	case CL_INVALID_OPERATION:
		return "CL_INVALID_OPERATION";
	case CL_INVALID_GL_OBJECT:
		return "CL_INVALID_GL_OBJECT";
	case CL_INVALID_BUFFER_SIZE:
		return "CL_INVALID_BUFFER_SIZE";
	case CL_INVALID_MIP_LEVEL:
		return "CL_INVALID_MIP_LEVEL";
	case CL_INVALID_GLOBAL_WORK_SIZE:
		return "CL_INVALID_GLOBAL_WORK_SIZE";
	case CL_INVALID_PROPERTY:
		return "CL_INVALID_PROPERTY";
	
	case CL_DEVICE_PARTITION_FAILED:
		return "CL_DEVICE_PARTITION_FAILED";
	case CL_INVALID_DEVICE_PARTITION_COUNT:
		return "CL_INVALID_DEVICE_PARTITION_COUNT";
	default:
		snprintf(ebuf, sizeof ebuf, "Unknown error: code %d", err);
		return ebuf;
	}
}

#ifdef _UTEST

int
main(void)
{
	char *str = strload("clutil.h");
	printf("%s\n", str);

	return 0;
}
#endif /* _UTEST */
