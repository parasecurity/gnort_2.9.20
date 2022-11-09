#ifndef _XDFA_H_
#define _XDFA_H_

#include <CL/opencl.h>

#include "dfa.h"
#include "clutil.h"
#include "acsmx2.h"
#include "fpdetect.h"

#define MAXPKTLEN 1536

struct xdfactx {
	struct dfa *dfa;
	unsigned int maxpatlen;
	unsigned int trcount;
	unsigned int nregexps;
	int *trans;
	cl_mem ktrans;

	// ========
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


	//=======
	struct pattern * patterns;
    struct pattern *** mlist;

	void         (*userfree)(void *p);
    void         (*optiontreefree)(void **p);
    void         (*neg_list_free)(void **p);
	//=======

	ACSM_STRUCT2 * acsm;
	OTNX_MATCH_DATA ** omd;
	unsigned char ** payloads;
};

// ========
struct pattern {
	struct  pattern *next;

    unsigned char         *patrn;
    unsigned char         *casepatrn;
    int      n;
    int      nocase;
    int      offset;
    int      depth;
    int      negative;
    void *udata;
    int      iid;
    void   * rule_option_tree;
    void   * neg_list;
};
// ========

struct xdfactx *
xdfa_new(userfree, optiontreefree, neg_list_free);

// =======
void
create_matchlist(struct xdfactx *xdfa);

void
xdfa_print_plist(struct xdfactx *xdfa);

void
print_matchlist(struct xdfactx *xdfa);
// =======

void
xdfa_addpattern(struct xdfactx *xdfa, char *pattern, unsigned int length, int nocase, int offset, int depth, int negative, void * id, int iid);

struct _SnortConfig;
void
xdfa_compile_with_sc(struct xdfactx *xdfa, struct _SnortConfig *,
                                int (*build_tree)(struct _SnortConfig *, void * id, void **existing_tree),
                                int (*neg_list_func)(void *id, void **list));

void
xdfa_compile(struct xdfactx *xdfa,
                                int (*build_tree)(struct _SnortConfig *, void * id, void **existing_tree),
                                int (*neg_list_func)(void *id, void **list));

void
xdfa_dump(struct xdfactx *xdfa, const char *path, int print);

void
xdfa_load(struct xdfactx *xdfa, const char *path);

/* helper to copy transition table to the device */
void
xdfa_cl_init(struct xdfactx *xdfa, cl_context ctx);

#endif /* _XDFA_H_ */
