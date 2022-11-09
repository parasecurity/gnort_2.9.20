#ifndef _XPKTBUF_H_
#define _XPKTBUF_H_

/*
 *       +---------+-----+----+-------+---     ----------+
 *  buf  | xxxxxxx | xxx | xx | xxxxx |    ...           |
 *       +---------+-----+----+-------+---     ----------+
 *
 *       ^         ^     ^    ^       ^                  ^
 *       |         |     |    |       |                  |
 *       '--.    .-'  .--'    |       |                  '---- bufsiz
 *          |    |    |    .--'       '----------------------- used
 *          |    |    |    |
 *       +----+----+----+----+----+---     ---+----+
 *  off  |  0 | 20 | 32 | 42 |    |    ...    |    |
 *       +----+----+----+----+----+---     ---+----+
 *       +----+----+----+----+----+---     ---+----+
 *  len  | 20 | 12 | 10 | 16 |    |    ...    |    |
 *       +----+----+----+----+----+---     ---+----+
 *
 *               ^              ^                ^
 *               |              |                |
 *               |              |                '------ max - 1
 *               |              '----------------------- cnt
 *               '-------------------------------------- pos
 */

#include <CL/opencl.h>

struct pktbuf {
	unsigned char	*buf;	/* Packet storage */
	int		*off;	/* Packet offsets */
	unsigned short	*len;	/* Packet lengths */
	unsigned char	*usr;	/* Packet user data */
	unsigned int	*res;	/* Packet results */
	unsigned int	*reschar;
	unsigned int	max;	/* Max number of packets */
	unsigned int	cnt;	/* Packets counter */
	unsigned int	pos;	/* Packets iterator */
	unsigned int	bufsiz;	/* Buffer size */
	unsigned int	used;	/* Amount used */
	unsigned int	reslen;	/* Result value size */
	/* The 'k' things are OpenCL mapped objects or copies */
	/* The 'i' things are registered malloced mem handles */
	cl_mem		kbuf, ibuf;
	cl_mem		koff, ioff;
	cl_mem		klen, ilen;
	cl_mem		kusr, iusr;
	cl_mem		kres, ires;
	cl_mem		kretchar, iretchar;
};

struct xpktbuf {
	struct	pktbuf *in;	/* Input buffer */
	struct	pktbuf *out;	/* Output buffer */
	struct	pktbuf *tmp;	/* Swap buffer */
	int	flags;		/* Operation mode */
/* MAPPED and all RD / WR flags are mutually exclusive */
#define XPB_MAPPED (1 << 0)
#define XPB_NONE 0
#define XPB_RDBUF (1 << 1)
#define XPB_RDOFF (1 << 2)
#define XPB_RDLEN (1 << 3)
#define XPB_RDUSR (1 << 4)
#define XPB_RDRES (1 << 5)
#define XPB_RDALL (XPB_RDBUF | XPB_RDOFF | XPB_RDLEN | XPB_RDUSR | XPB_RDRES)
#define XPB_WRUSR (1 << 6)
#define XPB_WRALL (XPB_WRUSR)
#define XPB_DEFAULT (XPB_RDRES)
	cl_command_queue queue; /* OpenCL queue */
};

/*
 * Allocates packet buffers for both host and device use.
 * The flags argument defines runtime options.
 */
struct xpktbuf *
xpktbuf_new(unsigned int max, unsigned int bufsiz, unsigned int reslen,
    int flags, cl_context ctx, cl_command_queue queue);

/*
 * Returns the packet counter.
 */
unsigned int
pktbuf_getcnt(struct pktbuf *pb);

/*
 * Returns the bytes used.
 */
unsigned int
pktbuf_getused(struct pktbuf *pb);

/*
 * Returns 1 if the packet buffer is full, 0 otherwise.
 */
unsigned int
pktbuf_isfull(struct pktbuf *pb);

/*
 * Frees all packet buffers on both the host and the device.
 */
void
xpktbuf_free(struct xpktbuf *xpb);

/*
 * Appends one packet to the input packet buffer.
 */
int
xpktbuf_addtoin(struct xpktbuf *xpb, unsigned char *pkt, unsigned short len);

/*
 * Synchronizes the device operations and does buffer swapping
 * to prepare for the next round.  Afterwards, the input buffer is
 * empty, the swap buffer contains all input so far and the output
 * buffer holds all packets from the previous copydevtoout call.
 */
void
xpktbuf_syncstreams(struct xpktbuf *xpb);

/*
 * Copies the swap buffer over to the device
 * if not already mapped.
 */
void
xpktbuf_copytodev(struct xpktbuf *xpb);

/*
 * Copies the device buffer back to the swap buffer
 * if not already mapped.
 */
void
xpktbuf_copytohost(struct xpktbuf *xpb);

/*
 * Gets the next packet from the output buffer.
 */
int
xpktbuf_nextout(struct xpktbuf *xpb, unsigned char **pkt, unsigned short *len,
    unsigned char *usr);

/*
 * Checks if the output buffer has been consumed.
 */
int
xpktbuf_isemptyout(struct xpktbuf *xpb);

/*
 * Returns the number of packets left on the output buffer.
 */
unsigned int
xpktbuf_nleftout(struct xpktbuf *xpb);

/*
 * Fill the input packet buffer with textual data from the file in path.
 */
void
xpktbuf_fillin(struct xpktbuf *xpb, const char *path);

#endif /* _XPKTBUF_H_ */
