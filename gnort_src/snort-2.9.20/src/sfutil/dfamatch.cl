/* defining _LCACHE is good only for graphics processors */

__kernel void
dfamatch(
    __global uint4 *buf,
    __global int *offary,
    __global unsigned short *lenary,
    __global unsigned int *retary,
    const unsigned int cnt,
    const unsigned int bufsiz,
    __global int *trans,
	//========
	__global unsigned int *retchar)
	//========
{
	/* map packet to different thread */
	uint id = get_global_id(0);
	uint lid = get_local_id(0);
	uint lsiz = get_local_size(0);
	uint ngrp = get_num_groups(0);
	int off = offary[id];
	unsigned short len = lenary[id];
	int offset = 0;
	unsigned char c;
	unsigned int size;
	uint4 packed;
	unsigned char *p;
	int i, k;
	int state = 0;
	int state_prev = 0;
#ifdef _LCACHE
#define NROWS 32
#define CACHESIZ (NROWS * 256)
	__local int hrows[CACHESIZ];
#endif

	if (id >= cnt)
		return;

	/* divide and ceil */
#define CEILDIV(x, y) (((x) + (y) - 1) / (y))
	size = CEILDIV(len, sizeof(uint4));

	/* memset zero */
	retary[id] = 0;
	retchar[id] = 0;

#ifdef _LCACHE
	/* cache the first n rows */
	if (lid < CACHESIZ) {
		for (i = 0; i < CEILDIV(CACHESIZ, lsiz); i++)
			hrows[i * lsiz + lid] = trans[i * lsiz + lid];
	}
	barrier(CLK_LOCAL_MEM_FENCE);
#endif

	/* loop on the packed data */
	for (i = 0; i < size; i++) {
		packed = *(buf + off / sizeof(uint4) + i);
		p = (unsigned char *)&packed;

		/* loop on the unpacked data */
		for (k = 0; k < sizeof(uint4); k++) {
			c = p[k];

#ifdef _LCACHE
			if (state < NROWS)
				state = *(hrows + (unsigned long)256 *
					  (unsigned long)state +
					   (unsigned long)c);
			else
#endif
				state = *(trans + (unsigned long)256 *
					  (unsigned long)state +
					  (unsigned long)c);
			
			if (!state_prev && state)
				offset = (i * sizeof(uint4)) + k;


			/* match found */
			if (state < 0) {
				retary[id] = state_prev; /*1;*/
				retchar[id] = (int)c;
				/* XXX: 'state = -state' duplicate alerts */
				// state = 0;
				state = -state;
			}
			state_prev = state;
		}
	}
}
