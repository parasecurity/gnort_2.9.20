#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "clutil.h"
#include "dfa.h"
#include "xdfa.h"

#define IGNORE_CASE 0

struct xdfactx *
xdfa_new(void)
{
	struct xdfactx *xdfa;

	xdfa = malloc(sizeof(struct xdfactx));
	if (xdfa == NULL)
		ERR(1, "malloc xdfa");

	xdfa->maxpatlen = 0;
	xdfa->nregexps = 0;

	xdfa->dfa = malloc(sizeof(struct dfa));
	if (xdfa->dfa == NULL)
		ERR(1, "malloc dfa");

	dfainit(xdfa->dfa);

	dfasyntax(RE_SYNTAX_POSIX_EGREP|RE_BACKSLASH_ESCAPE_IN_LISTS, IGNORE_CASE, '\n');

	return xdfa;
}

void
xdfa_addpattern(struct xdfactx *xdfa, char *pattern, unsigned int length)
{
	if (length > xdfa->maxpatlen)
		xdfa->maxpatlen = length;

	xdfa->nregexps += 1;

	dfaaddpatt(xdfa->dfa, pattern, length);
}

void
xdfa_compile(struct xdfactx *xdfa)
{
	int i;

	dfacomp(xdfa->dfa->regexp, xdfa->dfa->re_count, xdfa->dfa, 0);
	dfabuild(xdfa->dfa);
	xdfa->trcount = xdfa->dfa->trcount;

	xdfa->trans = malloc(256 * xdfa->trcount * sizeof(int));
	if (xdfa->trans == NULL)
		ERR(1, "malloc trans");

	/* copy state table to consecutive memory */
	for (i = 0; i < xdfa->dfa->trcount; ++i) {
		if (xdfa->dfa->fails[i]) {
			memcpy((xdfa->trans + i * 256), xdfa->dfa->fails[i],
			    sizeof(int) * 256);
			continue;
		}

		memcpy((xdfa->trans + i * 256), xdfa->dfa->trans[i],
		    sizeof(int) * 256);
	}

	dfafree(xdfa->dfa);
	xdfa->dfa = NULL;
}

void
xdfa_dump(struct xdfactx *xdfa, const char *path, int print)
{
	FILE *fp;
	int i, j;

	DPRINTF_U(xdfa->maxpatlen);
	DPRINTF_U(xdfa->trcount);

	fp = fopen(path, "wb");
	if (fp == NULL)
		ERRXV(1, "fopen: %s", path);

	fwrite(&(xdfa->maxpatlen), sizeof(xdfa->maxpatlen), 1, fp);
	fwrite(&(xdfa->trcount), sizeof(xdfa->trcount), 1, fp);
	fwrite(xdfa->trans, sizeof(int) * 256, xdfa->trcount, fp);

	fclose(fp);

	if (print) {
		printf("maxpatlen: %d\n", xdfa->maxpatlen);
		printf("trcount: %d\n", xdfa->trcount);
		printf("nregexps: %d\n", xdfa->nregexps);
		for (i = 0; i < xdfa->trcount; i++) {
			printf("[%d]\t", i);
			for (j = 0; j < 256; j++) {
				if (xdfa->trans[(i * 256) + j] != 0)
					printf("%c: %d ",
                                            (j > 31 && j < 127) ? j : '.', xdfa->trans[(i * 256) + j]);
			}
			printf("\n");
		}
	}	
}

void
xdfa_load(struct xdfactx *xdfa, const char *path)
{
	FILE *fp;
	size_t rsz;

	fp = fopen(path, "rb");
	if (fp == NULL)
		ERRXV(1, "fopen: %s", path);

	fread(&(xdfa->maxpatlen), sizeof(xdfa->maxpatlen), 1, fp);
	fread(&(xdfa->trcount), sizeof(xdfa->trcount), 1, fp);

	DPRINTF_U(xdfa->maxpatlen);
	DPRINTF_U(xdfa->trcount);

	xdfa->trans = malloc((unsigned long)256 *
			     (unsigned long)xdfa->trcount *
			     (unsigned long)sizeof(int));
	if (xdfa->trans == NULL)
		ERR(1, "malloc trans");

	rsz = fread(xdfa->trans, sizeof(int) * 256, xdfa->trcount, fp);
	if (rsz != xdfa->trcount)
		ERR(1, "Loading state table failed");

	fclose(fp);
}

/* helper to copy transition table to the device */
void
xdfa_cl_init(struct xdfactx *xdfa, cl_context ctx)
{
	int e;

	xdfa->ktrans = clCreateBuffer(ctx,
	    CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
	    (unsigned long)256 * (unsigned long)xdfa->trcount *
	    (unsigned long)sizeof(int), xdfa->trans, &e);
	if (e != CL_SUCCESS)
		ERRXV(1, "copy trans: %s", clstrerror(e));

	return;
}
