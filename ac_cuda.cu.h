#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cuda.h>
#include "acsmx.h"


#define MEM_OPTS
#define ALPHABET_SIZE 256
#define BUFFER_SIZE (8192)
#define PKT_SNAPLEN	1540

typedef short acstate_t;

typedef struct _ac_pattern {      
    //struct _ac_pattern *next;
    //unsigned char *patrn;
    unsigned char *casepatrn;
    //unsigned long *pattern_mask;
    int n;
    int nocase;
    int offset;
    int depth;
    void* id;
    int iid;
} ACCUDA_PATTERN_ATTR;

typedef struct {
    ACSM_STRUCT *acsm;

    cudaArray *acPatternsArray_d;
    #ifdef MEM_OPTS
        acstate_t *acStateTableArray_d;
        unsigned int acStateTableOffset;
    #else
    	cudaArray *acStateTableArray_d;
    #endif

    unsigned int		acPatternSize;
	unsigned int		acPatternCurr;

	unsigned int		maxPatternLen; // the maximum pattern length
    #ifndef MEM_OPTS
        #ifdef DOUBLE_BUFFER
            unsigned char		bufNo; // 0 or 1
            unsigned char		buf[2][BUFFER_SIZE][PKT_SNAPLEN];
        #else
            unsigned char		bufNo; // 0
            unsigned char		buf[1][BUFFER_SIZE][PKT_SNAPLEN]; // buffer to copy packets + size
        #endif
    #endif
    unsigned int *retval;
	unsigned int *retval_d;
    #ifndef MEM_OPTS
	    unsigned int bufCurr;
    #endif
    unsigned acPatternCnt;		/* obsolete by kmpPatternCurr */
} ACCUDA_STRUCT;


ACCUDA_STRUCT * acNew (void);