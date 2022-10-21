#ifndef _COMMON_H_
#define _COMMON_H_

#include <errno.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* error reporting helpers */
#define ERRX(ret, str) \
    do { fprintf(stderr, str "\n"); exit(ret); } while (0)
#define ERRXV(ret, fmt, ...) \
    do { fprintf(stderr, fmt "\n", __VA_ARGS__); exit(ret); } while (0)
#define ERR(ret, str) \
    do { fprintf(stderr, str ": %s\n", strerror(errno)); exit(ret); } while (0)
#define ERRV(ret, fmt, ...) \
    do { fprintf(stderr, fmt ": %s\n", __VA_ARGS__, \
    strerror(errno)); exit(ret); } while (0)

/* debug printing helpers */
#ifdef _DEBUG
#define DPRINTF printf
#define DPRINTF_D(t) printf(#t"=%d\n", t)
#define DPRINTF_U(t) printf(#t"=%u\n", t)
#define DPRINTF_S(t) printf(#t"=%s\n", t)
#define DPRINTF_F(t) printf(#t"=%f\n", t)
#define DPRINTF_LD(t) printf(#t"=%ld\n", t)
#define DPRINTF_LU(t) printf(#t"=%lu\n", t)
#define DPRINTF_LLU(t) printf(#t"=%llu\n", t)
#define DPRINTF_X(t) printf(#t"=0x%lx\n", (unsigned long)t)
#else
#define DPRINTF
#define DPRINTF_D(t)
#define DPRINTF_U(t)
#define DPRINTF_S(t)
#define DPRINTF_F(t)
#define DPRINTF_LD(t)
#define DPRINTF_LU(t)
#define DPRINTF_LLU(t)
#define DPRINTF_X(t)
#endif

/* stringify whatever exists in the parentheses */
#define MKSTR(s ...) #s

/* calculations with timers */
#define TVDIFFMICRO(tvi, tvf) \
    ((tvf.tv_sec * 1000000 + tvf.tv_usec) \
   - (tvi.tv_sec * 1000000 + tvi.tv_usec))

/* how many y-sized blocks are needed for x elements */
#define CEILDIV(x, y) (((x) + ((y) - 1)) / (y))

/* round up x to the next y-sized block boundary */
#define ROUNDUP(x, y) (((x) + ((y) - 1)) & ~((y) - 1))

/* read timestamp counter for benchmarking */
#ifdef _DEBUG
#define RDTSC(t) do { __asm__ volatile ("rdtsc" : "=A" (t)); } while (0)
#else
#define RDTSC(t)
#endif

/* aligned malloc and free */
#ifdef _WIN32
#define MALLOC(n) _aligned_malloc(n, 0x1000)
#define FREE(p) _aligned_free(p)
#else
#define MALLOC(n) memalign(0x1000, n)
#define FREE(p) free(p)
#endif /* _WIN32 */

/* get the size of statically allocated arrays */
#define LEN(a) (sizeof(a) / sizeof(*(a)))

/* 32-bit integer manipulation macros (little endian) */
#ifndef GET_UINT32_LE
#define GET_UINT32_LE(n, b, i) do {			\
	(n) = ((uint32_t)(b)[(i)    ]      )		\
	    | ((uint32_t)(b)[(i) + 1] <<  8)		\
	    | ((uint32_t)(b)[(i) + 2] << 16)		\
	    | ((uint32_t)(b)[(i) + 3] << 24);		\
} while (0)
#endif

#ifndef PUT_UINT32_LE
#define PUT_UINT32_LE(n, b, i) do {			\
	(b)[(i)    ] = (unsigned char)((n)      );	\
	(b)[(i) + 1] = (unsigned char)((n) >>  8);	\
	(b)[(i) + 2] = (unsigned char)((n) >> 16);	\
	(b)[(i) + 3] = (unsigned char)((n) >> 24);	\
} while (0)
#endif

#endif /* _COMMON_H_ */
