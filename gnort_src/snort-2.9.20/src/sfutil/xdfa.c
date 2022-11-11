#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "clutil.h"
#include "dfa.h"
#include "xdfa.h"
#include "xpktbuf.h"
#include "acsmx2.h"

#define IGNORE_CASE 0

struct xdfactx *
xdfa_new(void (*userfree)(void *p),
                         void (*optiontreefree)(void **p),
                         void (*neg_list_free)(void **p))
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
	xdfa->patterns = NULL;

	dfainit(xdfa->dfa);

	dfasyntax(RE_SYNTAX_POSIX_EGREP|RE_BACKSLASH_ESCAPE_IN_LISTS, IGNORE_CASE, '\n');

	// argyris===========


	xdfa->devpos = 0; /* the first available device */
	xdfa->mapped = 0; /* 0 for discrete devices */
	xdfa->verbose = 0; /* do not print matches by default*/
	xdfa->matches = 0;
	xdfa->queuesiz = 50; /* size of packet buffer */
	xdfa->blksiz = 16; /* partitioning */
	xdfa->elapsed_time = 0;
	// FILE *input = stdin; /* default input stream */
	// char *patterns = "patterns.dat"; /* path of default patterns file */
	// char buf[1500];


	struct clconf cl;
	struct xpktbuf *xpb;
	// struct timeval start_time, end_time;

	clsetupf(&cl, "src/sfutil/dfamatch.cl", "dfamatch", NULL, xdfa->devpos, -1);
	// xdfa->xpb = xpktbuf_new(xdfa->queuesiz, xdfa->queuesiz * MAXPKTLEN, sizeof(unsigned int),
	//     xdfa->mapped ? XPB_MAPPED : XPB_DEFAULT, xdfa->cl.ctx, xdfa->cl.queue);
	xpb = xpktbuf_new(xdfa->queuesiz, xdfa->queuesiz * MAXPKTLEN, sizeof(unsigned int),
	    xdfa->mapped ? XPB_MAPPED : XPB_DEFAULT, cl.ctx, cl.queue);

	// xdfa_cl_init(xdfa, cl.ctx);
	xdfa->cl = cl;
	xdfa->xpb = xpb;

    xdfa->userfree              = userfree;
    xdfa->optiontreefree        = optiontreefree;
    xdfa->neg_list_free         = neg_list_free;

	xdfa->acsm = acsmNew2(userfree, optiontreefree, neg_list_free);
	xdfa->omd = (OTNX_MATCH_DATA **)malloc(xdfa->queuesiz * sizeof(OTNX_MATCH_DATA *));
	xdfa->datalist = (void **)malloc(xdfa->queuesiz * sizeof(void *));

	xdfa->payloads = (unsigned char **)malloc(xdfa->queuesiz * sizeof(unsigned char *));
	return xdfa;
}

void
xdfa_print_plist(struct xdfactx *xdfa){
	struct pattern * plist;
	plist = xdfa->patterns;
	while(plist != NULL){
		printf("pattern from plist: %s\n", plist->patrn);
		if (plist->next != NULL){
			plist = plist->next;
		}
		else {
			break;
		}
		
	}
	return;
}

struct pattern *
insert_bottom(struct pattern * head, char *pattern, unsigned int len, int nocase, int offset, int depth, int negative, void * id, int iid){
	struct pattern *current_node = head;
	struct pattern *new_node;
	while ( current_node != NULL && current_node->next != NULL) {
	current_node = current_node->next;
	}

	new_node = (struct pattern *) malloc(sizeof(struct pattern));
	new_node->patrn = malloc(len);
	strcpy(new_node->patrn, pattern);
	// printf("strlen pattern: %d, %s\n", len, pattern);
	// printf("strlen pattern: %d, %s\n", strlen(pattern), pattern);
	// printf("strlen pattern: %d, %s\n", strlen((unsigned char *)pattern), pattern);
	new_node->n = len;
	new_node->next = NULL;
	new_node->neg_list = NULL;
	new_node->rule_option_tree = NULL;
	new_node->nocase = nocase;
	new_node->offset = offset;
	new_node->depth  = depth;
	new_node->negative = negative;
	new_node->iid    = iid;
	new_node->udata = id;

	if (current_node != NULL)
		current_node->next = new_node;
	else
		head = new_node;

return head;
}

void
xdfa_addpattern(struct xdfactx *xdfa, char *pattern, unsigned int length, int nocase,
        int offset, int depth, int negative, void * id, int iid)
{
	// printf("pattern: %s\n", pattern);
	if (length > xdfa->maxpatlen)
		xdfa->maxpatlen = length;

	xdfa->nregexps += 1;
	xdfa->patterns = insert_bottom(xdfa->patterns, pattern, length, nocase, offset, depth, negative, id, iid);

	dfaaddpatt(xdfa->dfa, pattern, length);
	printf("id: %d, iid: %d\n", id, iid);
	printf("////////////Successfully added pattern(%d): %s\n", length, pattern);

	acsmAddPattern2(xdfa->acsm, (unsigned char *)pattern, length,
              nocase, offset, depth, negative, id, iid);
}

int xdfa_patterncount(struct xdfactx *xdfa){
	return xdfa->nregexps;
}

void
xdfa_compile(struct xdfactx *xdfa,
                                int (*build_tree)(struct _SnortConfig *, void * id, void **existing_tree),
                                int (*neg_list_func)(void *id, void **list))
{
	int i;

	dfacomp(xdfa->dfa->regexp, xdfa->dfa->re_count, xdfa->dfa, 0);
	printf("==GPUREGEX== dfa compiled!\n");
	dfabuild(xdfa->dfa);
	printf("==GPUREGEX== dfa built!\n");
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

	// argyris ============================================
	// dfaprint(xdfa->dfa);
	// before freeing must enter the match listings
	create_matchlist(xdfa);
	// print_matchlist(xdfa);
	// ====================================================

	dfafree(xdfa->dfa);
	xdfa->dfa = NULL;
	struct clconf cl;
	cl = xdfa->cl;
	xdfa_cl_init(xdfa, cl.ctx);
	xdfa->cl = cl;

	// if (build_tree && neg_list_func)
    // {
    //     xdfa_build_match_state_trees_with_sc(xdfa, sc, build_tree, neg_list_func);
    // }
	printf("==GPUREGEX== Compiled successfully!\n");
}

void
xdfa_compile_with_sc(struct xdfactx *xdfa, struct _SnortConfig * sc,
                                int (*build_tree)(struct _SnortConfig *, void * id, void **existing_tree),
                                int (*neg_list_func)(void *id, void **list))
{
	int i;


	dfacomp(xdfa->dfa->regexp, xdfa->dfa->re_count, xdfa->dfa, 0);
	printf("==GPUREGEX== dfa compiled!\n");
	dfabuild(xdfa->dfa);
	printf("==GPUREGEX== dfa built!\n");
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

	// argyris ============================================
	// dfaprint(xdfa->dfa);
	// before freeing must enter the match listings
	create_matchlist(xdfa);
	print_matchlist(xdfa);
	// ====================================================

	dfafree(xdfa->dfa);
	xdfa->dfa = NULL;
	struct clconf cl;
	cl = xdfa->cl;
	xdfa_cl_init(xdfa, cl.ctx);
	xdfa->cl = cl;
	// printf("all good here 21\n");

	// if (build_tree && neg_list_func)
    // {
    //     xdfa_build_match_state_trees_with_sc(xdfa, sc, build_tree, neg_list_func);
    // }
	printf("==GPUREGEX== Compiled successfully!\n");
	acsmCompile2WithSnortConf( sc, xdfa->acsm, build_tree, neg_list_func );
}



int xdfa_build_match_state_trees_with_sc(struct xdfactx *xdfa, struct _SnortConfig *sc,
                                                   int (*build_tree)(struct _SnortConfig *, void * id, void **existing_tree),
                                                   int (*neg_list_func)(void *id, void **list) )
{
    int i, j, cnt = 0;
    // struct pattern  *** MatchList = xdfa->mlist;
    struct pattern * mlist;

	for (i = 0; i<xdfa->trcount; i++){
		for (j = 0; j < 256; j++){
			mlist = xdfa->mlist[i][j];
			if (mlist != NULL){
				// printf("build match state trees with sc mlist: %s\n", mlist->patrn);
				printf("all good here 23\n");
				if (mlist->udata)
				{
					printf("all good here 24\n");
					if (mlist->negative)
					{
						printf("all good here 25\n");
						neg_list_func(mlist->udata, &xdfa->mlist[i][j]->neg_list);
						printf("all good here 26\n");
					}
					else
					{
						printf("all good here 27\n");
						build_tree(sc, mlist->udata, &xdfa->mlist[i][j]->rule_option_tree);
						printf("all good here 28\n");
					}
				}

				cnt++;
			}
			if (xdfa->mlist[i][j])
			{
				printf("all good here 35\n");
				/* Last call to finalize the tree */
				build_tree(sc, NULL, &xdfa->mlist[i][j]->rule_option_tree);
				printf("all good here 40\n");

			}

		}
	}


	printf("all good here RETURN\n");

    return cnt;
}


void
print_matchlist(struct xdfactx *xdfa){
	int i, j;
	struct pattern *** mlist = xdfa->mlist;
	for (i = 0; i < xdfa->trcount; i++){

	
		for (j = 0; j < 256; j++){
			if (mlist[i][j] != NULL)
				printf("mlist: %s\n", mlist[i][j]->patrn);
		}
	}
}

void
create_matchlist(struct xdfactx *xdfa){
	int i, j, l, state, prev_state, ch;
	int plen = 0;

	struct pattern * plist = xdfa->patterns;
	struct pattern temp_pat;
	struct pattern * p;



	struct pattern *** mlist = (struct pattern ***)malloc(xdfa->trcount * 256 * sizeof(struct pattern *)); // make it triple pointer
	for (i = 0; i < xdfa->trcount; ++i){
		mlist[i] = (struct pattern **)malloc(256 * sizeof(struct pattern));
		for (j = 0; j < 256; j++) {
			// mlist[i][j] = (struct pattern *)malloc(sizeof(struct pattern));
			mlist[i][j] = NULL;
		}
	}
    	
			


	while (plist != NULL){
		
		plen = strlen(plist->patrn);
		// printf("mlist curr pattern (%d): %s\n", plen, plist->patrn);
		state = 0;
		prev_state = 0;
		for (l = 0; l < plen; l++){
			prev_state = state;
			ch = plist->patrn[l];
			// /*						// COMMENT THIS ON ACTUAL SNORT
			// if (ch == '\n'){
			// 	break;
			// }		
			// */						// COMMENT THIS ON ACTUAL SNORT									
			if (xdfa->dfa->fails[state]){
				// printf("state: %d (%d, %c) -> %d\n",state, ch, ch, xdfa->dfa->fails[prev_state][ch]);
				state = xdfa->dfa->fails[prev_state][ch];
			}else{
				// printf("state: %d (%d, %c) -> %d\n",state, ch, ch, xdfa->dfa->trans[prev_state][ch]);
				state = xdfa->dfa->trans[prev_state][ch];
			}

			if (state < 0){
				mlist[prev_state][ch] = (struct pattern *)malloc(sizeof(struct pattern));
				memcpy (mlist[prev_state][ch], plist, sizeof (struct pattern));

				mlist[prev_state][ch]->next = NULL;
				// mlist[prev_state][ch]->rule_option_tree = NULL;
				state = -state; // add more subsets of patterns, comment to discard duplicates
			}
			// d->trans[i][j], i is the current state, j is the next character, and returns the next state, 
			// since here we parse the patterns it will never return 0, and will always result in a match state
		}
		printf("\n");

		// end of loop
		if (plist->next == NULL){
			break;
		}
		plist = plist->next;
	}

	xdfa->mlist = mlist;
	return;
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
