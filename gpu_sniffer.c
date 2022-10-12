#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <CL/opencl.h>
#include <sys/time.h>
#include <pcap.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "common.h"
#include "clutil.h"
#include "dfamatch.h"
#include "xdfa.h"
#include "xpktbuf.h"
#include "hacks.h"


int opt, e;
int devpos = 0; /* the first available device */
int mapped = 1; /* unset for discrete devices */
int verbose = 0; /* do not print matches by default*/
unsigned int matches = 0;
// unsigned int queuesiz = 8192; /* size of packet buffer */
unsigned int queuesiz = 50;
unsigned int blksiz = 16; /* partitioning */
unsigned long long elapsed_time;
// FILE *input = stdin; /* default input stream */
char *patterns = "patterns.dat"; /* path of default patterns file */
char buf[1500];
struct clconf cl;
struct xpktbuf *xpb;
struct xdfactx *xdfa;
struct timeval start_time, end_time;
FILE *f_p;
char filename[] = "traces.dat";


#define MAXPKTLEN 1536

/* default snap length (maximum bytes per packet to capture) */
#define SNAP_LEN 1518

/* ethernet headers are always exactly 14 bytes [1] */
#define SIZE_ETHERNET 14

/* Ethernet addresses are 6 bytes */
#define ETHER_ADDR_LEN	6

/* Ethernet header */
struct sniff_ethernet {
        u_char  ether_dhost[ETHER_ADDR_LEN];    /* destination host address */
        u_char  ether_shost[ETHER_ADDR_LEN];    /* source host address */
        u_short ether_type;                     /* IP? ARP? RARP? etc */
};

/* IP header */
struct sniff_ip {
        u_char  ip_vhl;                 /* version << 4 | header length >> 2 */
        u_char  ip_tos;                 /* type of service */
        u_short ip_len;                 /* total length */
        u_short ip_id;                  /* identification */
        u_short ip_off;                 /* fragment offset field */
        #define IP_RF 0x8000            /* reserved fragment flag */
        #define IP_DF 0x4000            /* don't fragment flag */
        #define IP_MF 0x2000            /* more fragments flag */
        #define IP_OFFMASK 0x1fff       /* mask for fragmenting bits */
        u_char  ip_ttl;                 /* time to live */
        u_char  ip_p;                   /* protocol */
        u_short ip_sum;                 /* checksum */
        struct  in_addr ip_src,ip_dst;  /* source and dest address */
};
#define IP_HL(ip)               (((ip)->ip_vhl) & 0x0f)
#define IP_V(ip)                (((ip)->ip_vhl) >> 4)

/* TCP header */
typedef u_int tcp_seq;
int SAVE_TCP_ONLY = 0;

struct sniff_tcp {
        u_short th_sport;               /* source port */
        u_short th_dport;               /* destination port */
        tcp_seq th_seq;                 /* sequence number */
        tcp_seq th_ack;                 /* acknowledgement number */
        u_char  th_offx2;               /* data offset, rsvd */
#define TH_OFF(th)      (((th)->th_offx2 & 0xf0) >> 4)
        u_char  th_flags;
        #define TH_FIN  0x01
        #define TH_SYN  0x02
        #define TH_RST  0x04
        #define TH_PUSH 0x08
        #define TH_ACK  0x10
        #define TH_URG  0x20
        #define TH_ECE  0x40
        #define TH_CWR  0x80
        #define TH_FLAGS        (TH_FIN|TH_SYN|TH_RST|TH_ACK|TH_URG|TH_ECE|TH_CWR)
        u_short th_win;                 /* window */
        u_short th_sum;                 /* checksum */
        u_short th_urp;                 /* urgent pointer */
};

struct sniff_udp {
        u_short uh_sport;               /* source port */
        u_short uh_dport;               /* destination port */
        u_short uh_ulen;                /* udp length */
        u_short uh_sum;                 /* udp checksum */
};

/* dfa match kernel wrapper */
void
kdfamatch(cl_kernel kernel, cl_command_queue queue, cl_mem kbuf, cl_mem koffary,
    cl_mem klenary, cl_mem kretary, cl_uint cnt, cl_uint bufsiz, cl_mem ktrans,
    size_t blksiz, cl_event *event)
{
	int e;
	static int counter = 0;
	int ret[65536];
	size_t global = ROUNDUP(cnt, blksiz);
	size_t local = blksiz;

#ifdef _DFAMATCH_DEBUG
	for (int i = 0; i < 65536; i++) 
		ret[i] = 0;

	e = clEnqueueWriteBuffer(queue, kretary, CL_TRUE, 0,
	    sizeof(int) * 65536, ret, 0, NULL, NULL);
#endif /* _DFAMATCH_DEBUG */

	/* call arguments */
	clSetKernelArg(kernel, 0, sizeof(cl_mem), &kbuf);
	clSetKernelArg(kernel, 1, sizeof(cl_mem), &koffary);
	clSetKernelArg(kernel, 2, sizeof(cl_mem), &klenary);
	clSetKernelArg(kernel, 3, sizeof(cl_mem), &kretary);
	clSetKernelArg(kernel, 4, sizeof(cl_uint), &cnt);
	clSetKernelArg(kernel, 5, sizeof(cl_uint), &bufsiz);
	clSetKernelArg(kernel, 6, sizeof(cl_mem), &ktrans);

	/* execution */
	e = clEnqueueNDRangeKernel(queue, kernel, 1, NULL,
	    &global, &local, 0, NULL, event);
	if (e != CL_SUCCESS)
		ERRXV(1, "execute: %s", clstrerror(e));
	if (event != NULL)
		e = clWaitForEvents(1, event);
	if (e != CL_SUCCESS)
		ERRXV(1, "event: %s", clstrerror(e));
	clFlush(queue);

#ifdef _DFAMATCH_DEBUG
	e = clEnqueueReadBuffer(queue, kretary, CL_TRUE, 0, sizeof(int) * 65536,
	    ret, 0, NULL, NULL);
	for (e = 0; e < 65536; e++) {
		if (ret[e] == 1)
			counter++;
	}
	printf("[matches]\t%d\n", counter);
	counter = 0;
#endif /* _DFAMATCH_DEBUG */

	clFlush(queue);
}

/* dfa match packet buffer kernel wrapper */
void
kdfamatchpb(struct clconf *cl, struct pktbuf *pb, struct xdfactx *xdfa,
    size_t blksiz)
{
	kdfamatch(cl->kernel, cl->queue, pb->kbuf, pb->koff, pb->klen, pb->kres,
	    pb->cnt, pb->used, xdfa->ktrans, blksiz, &cl->event);
}

void
load_patterns(struct xdfactx *xdfa, char *path)
{
	char buf[100];

	FILE *fd = fopen(path, "r");
	if (fd == NULL) {
		perror("");
		exit(-1);
	}

	while (fgets(buf, 100, fd)) {
		xdfa_addpattern(xdfa, buf, strlen(buf));
	}

	fclose(fd);

	return;
}

void search(char pkt[]){
    if(xpktbuf_addtoin(xpb, (unsigned char *)pkt, strlen(pkt) + 1) > 0){
        // printf("Returned!\n");
		return;
    }
    // printf("Didnt return :)\n");
        

    DPRINTF_D(xpb->in->cnt);
    DPRINTF_D(xpb->in->used);

    xpktbuf_syncstreams(xpb);
    
    /* erase return cells */
    memset(xpb->tmp->res, 0, queuesiz * sizeof(unsigned int));
    
    gettimeofday(&start_time, NULL);

    /* run */
    xpktbuf_copytodev(xpb);
    kdfamatchpb(&cl, xpb->tmp, xdfa, blksiz);
    xpktbuf_copytohost(xpb);

    gettimeofday(&end_time, NULL);
    elapsed_time = (end_time.tv_sec * 1000 + end_time.tv_usec) - (start_time.tv_sec * 1000 + start_time.tv_usec); 

    /* get results */
    e = clFinish(xpb->queue);
    if (e != CL_SUCCESS)
        ERRXV(1, "finish: %s", clstrerror(e));

    for (int i = 0; i < xpb->tmp->cnt; i++) {
        if (xpb->tmp->res[i]) {
            if (verbose == 1){
                printf("\nMatch: ", i);
                printf(&xpb->tmp->buf[xpb->tmp->off[i]]);
                printf("\n");			
            }		
            matches++;
        }
    }
    xpktbuf_addtoin(xpb, (unsigned char *)buf, strlen(buf + 1));
}

void
save_packets_to_file(u_char protocol, char * sa, char * da, int sp, int dp, const u_char *payload, int len){
    /* determine protocol */
	// switch(protocol) {
	// 	case IPPROTO_TCP:
	// 		printf("   Protocol: TCP\n");
	// 		break;
	// 	case IPPROTO_UDP:
	// 		printf("   Protocol: UDP\n");
	// 		break;
	// 	case IPPROTO_ICMP:
	// 		printf("   Protocol: ICMP\n");
	// 		return;
	// 	case IPPROTO_IP:
	// 		printf("   Protocol: IP\n");
	// 		return;
	// 	default:
	// 		printf("   Protocol: unknown\n");
	// 		return;
	// }
    // printf("src addr: %s\n", sa);
    // printf("dst addr: %s\n", da);
    // printf("src port: %d\n", sp);
    // printf("dst port: %d\n", dp);
    
    // printf("%s %s %d %d\n", sa, da, sp, dp);

    // char sa_str[] = 
    
    // Open file in write mode

    f_p = fopen(filename,"a");

    // If file opened successfully, then write the string to file
    if ( f_p )
    {

        switch(protocol) {
            case IPPROTO_TCP:
                // printf("   Protocol: TCP\n");
                fputs("tcp", f_p);
                break;
            case IPPROTO_UDP:
                // printf("   Protocol: UDP\n");
                fputs("udp", f_p);
                break;
            case IPPROTO_ICMP:
                // printf("   Protocol: ICMP\n");
                fputs("icmp", f_p);
                break;
            case IPPROTO_IP:
                // printf("   Protocol: IP\n");
                fputs("ip", f_p);
                break;
            default:
                printf("Protocol: unknown\n");
                return;
	    }
        fputs(" ", f_p);
        fputs(sa,f_p);
        fputs(" ", f_p);
        fputs(da,f_p);
        fputs(" ", f_p);
        // fputs(str(sp), f_p);
        fprintf(f_p, "%d", sp); 
        fputs(" ", f_p);
        // fputs(str(dp), f_p);
        fprintf(f_p, "%d", dp); 
        fputs(" ", f_p);
        

        const u_char *ch;
        ch = payload;
        for(int i = 0; i < len; i++) {
            if (isprint(*ch)){
                // printf("%c", *ch);
                fputc(*ch, f_p);
            }
            else{
                // printf(".");
                fputs(".", f_p);
            }
            ch++;
	    }
        // printf("\n");
        fputs("\n", f_p);


    }
    else
    {
        printf("Failed to open the file\n");
    }
    //Close the file
    
    fclose(f_p);
    return;
}
void
save_payload(char buf[]){
   

    f_p = fopen(filename,"a");

    // If file opened successfully, then write the string to file
    if ( f_p )
    {
		fputs(buf, f_p);
        fputs("\n", f_p);
    }
    else
    {
        printf("Failed to open the file\n");
    }
    //Close the file
    
    fclose(f_p);
    return;
}

void
got_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet)
{

	static int count = 1;                   /* packet counter */

	/* declare pointers to packet headers */
	const struct sniff_ethernet *ethernet;  /* The ethernet header [1] */
	const struct sniff_ip *ip;              /* The IP header */
	const struct sniff_tcp *tcp;            /* The TCP header */
    const struct sniff_udp *udp;
	unsigned char *payload;                    /* Packet payload */

	int size_ip;
	int size_tcp;
	int size_payload;

    int size_udp;



	count++;

	/* define ethernet header */
	ethernet = (struct sniff_ethernet*)(packet);
	/* define/compute ip header offset */
	ip = (struct sniff_ip*)(packet + SIZE_ETHERNET);
	size_ip = IP_HL(ip)*4;
	if (size_ip < 20) {
		printf("   * Invalid IP header length: %u bytes\n", size_ip);
		return;
	}




	char tuple[MAXPKTLEN] = "";
	char ch[1500];
	/* determine protocol */
    // printf("%c\n", ip->ip_p);
	switch(ip->ip_p) {
		case IPPROTO_TCP:
			// printf("   Protocol: TCP\n");
            // return;
			break;
		case IPPROTO_UDP:
			// printf("   Protocol: UDP\n");
            if (SAVE_TCP_ONLY)
                return;
            udp = (struct sniff_udp*)(packet + SIZE_ETHERNET + size_ip);
            size_udp = ntohs(udp->uh_ulen);
            /* define/compute udp payload (daragram) offset */
            payload = (u_char *)(packet + SIZE_ETHERNET + size_ip + 8);

            /* compute udp payload (datagram) size */
            size_payload = ntohs(ip->ip_len) - (size_ip + 8);
			

            /*
            * Print payload data; it might be binary, so don't just
            * treat it as a string.
            */
            if (1 /*size_payload > 0*/) {
                // printf("Going into save packets to file\n");
                char * src_adr; // = inet_ntoa(ip->ip_src);
                src_adr = (char *)malloc(strlen(inet_ntoa(ip->ip_src)));
                strcpy(src_adr, inet_ntoa(ip->ip_src));
                char * dst_adr; // = inet_ntoa(ip->ip_dst);
                dst_adr = (char *)malloc(strlen(inet_ntoa(ip->ip_dst)));
                strcpy(dst_adr, inet_ntoa(ip->ip_dst));
                // save_packets_to_file(ip->ip_p, src_adr, dst_adr, ntohs(udp->uh_sport), ntohs(udp->uh_dport), payload, size_payload);

                int len = strlen(payload);
                strcpy(ch, payload);
				char new_payload[1500];
				int i;
				for(i = 0; i < size_payload; i++) {
					if (i >= 1500){
						return;
					}
					if (isprint(payload[i])){
						new_payload[i] = payload[i];
					}
					else{
						new_payload[i] = '.';
					}
				}
				if (i >= 1500){
					return;
				}
				new_payload[i] = '.';
				
				for(int i = 0; i < len; i++) {
					if (isprint(ch[i])){
						// printf("%c", *ch);
						// fputc(*ch, f_p);
					}
					else{
						ch[i] = '.';
						// fputs(".", f_p);
					}
				}
				
				sprintf(tuple, "%s %s %s %d %d %s", "udp", src_adr, dst_adr, ntohs(tcp->th_sport), ntohs(tcp->th_dport), ch);
				// printf("%s\n", tuple);
                search(new_payload);
				save_payload(new_payload);
				
				
            }
			return;
		case IPPROTO_ICMP:
			printf("   Protocol: ICMP\n");
			return;
		case IPPROTO_IP:
			printf("   Protocol: IP\n");
			return;
		default:
			printf("   Protocol: unknown\n");
			return;
	}



	/* define/compute tcp header offset */
	tcp = (struct sniff_tcp*)(packet + SIZE_ETHERNET + size_ip);
	size_tcp = TH_OFF(tcp)*4;
	if (size_tcp < 20) {
		printf("   * Invalid TCP header length: %u bytes\n", size_tcp);
		return;
	}


	/* define/compute tcp payload (segment) offset */
	payload = (u_char *)(packet + SIZE_ETHERNET + size_ip + size_tcp);

	/* compute tcp payload (segment) size */
	size_payload = ntohs(ip->ip_len) - (size_ip + size_tcp);


    char * src_adr; // = inet_ntoa(ip->ip_src);
    src_adr = (char *)malloc(strlen(inet_ntoa(ip->ip_src)));
    strcpy(src_adr, inet_ntoa(ip->ip_src));
    char * dst_adr; // = inet_ntoa(ip->ip_dst);
    dst_adr = (char *)malloc(strlen(inet_ntoa(ip->ip_dst)));
    strcpy(dst_adr, inet_ntoa(ip->ip_dst));
	
	int len = strlen(payload);
    // ch = (char *)malloc(len);
    strcpy(ch, payload);
    // printf("%s\n%s\n", ch, payload);
	// printf("ch: %s\npayload: %s\n", ch, payload);
	char new_payload[1500];
	int i;
	for(i = 0; i < size_payload; i++) {
		if (i >= 1500){
			return;
		}
		if (isprint(payload[i])){
			new_payload[i] = payload[i];
		}
		else{
			new_payload[i] = '.';
		}
	}
	if (i >= 1500){
		return;
	}
	new_payload[i] = '.';
	

	for(int i = 0; i < len; i++) {
		if (isprint(ch[i])){
			// printf("%c", *ch);
			// fputc(*ch, f_p);
		}
		else{
			ch[i] = '.';
			// fputs(".", f_p);
		}
	}
	// printf("%s\n%s\n", ch, payload);
	sprintf(tuple, "%s %s %s %d %d %s", "tcp", src_adr, dst_adr, ntohs(tcp->th_sport), ntohs(tcp->th_dport), ch);
	// printf("%s\n", tuple);
    search(new_payload);
	save_payload(new_payload);

    if (0 /*size_payload > 0*/) {
        save_packets_to_file(ip->ip_p, src_adr, dst_adr, ntohs(tcp->th_sport), ntohs(tcp->th_dport), payload, size_payload);
		
    }

return;
}

int
main(int argc, char *argv[])
{

    f_p = fopen(filename,"w");
    fclose(f_p);

    // FILE *input = stdin;
	while ((opt = getopt(argc, argv, ":d:i:m:p:q:v")) != -1) {
		switch (opt) {
		case 'd':
			devpos = atoi(optarg);
			break;
		// case 'i':
		// 	if ((input = fopen(optarg, "r")) == NULL) {
		// 		perror("");
		// 		exit(-1);
		// 	}
		// 	break;
		case 'm':
			mapped = atoi(optarg);
			break;
		case 'p':
			patterns = optarg;
			break;
		case 'q':
			queuesiz = atoi(optarg);
			break;
		case 'v':
			verbose = 1;
			break;
		case ':':
			printf("Missing option value, proceeding with the defaults\n");
			break;
		case '?':
			printf("Unknown option: -%c\n", optopt);
			break;
		}
	}

	char *dev = NULL;			/* capture device name */
	char errbuf[PCAP_ERRBUF_SIZE];		/* error buffer */
	pcap_t *handle;				/* packet capture handle */

	char filter_exp[] = "ip";		/* filter expression [3] */
	struct bpf_program fp;			/* compiled filter program (expression) */
	bpf_u_int32 mask;			/* subnet mask */
	bpf_u_int32 net;			/* ip */

	/* find a capture device if not specified on command-line */
	dev = pcap_lookupdev(errbuf);
	if (dev == NULL) {
		fprintf(stderr, "Couldn't find default device: %s\n",
			errbuf);
		exit(EXIT_FAILURE);
	}

	/* get network number and mask associated with capture device */
	if (pcap_lookupnet(dev, &net, &mask, errbuf) == -1) {
		fprintf(stderr, "Couldn't get netmask for device %s: %s\n",
		    dev, errbuf);
		net = 0;
		mask = 0;
	}
	/* print capture info */
	printf("Device: %s\n", dev);
	printf("Number of packets: inf\n");
	printf("Filter expression: %s\n", filter_exp);

	/* open capture device */
	handle = pcap_open_live(dev, SNAP_LEN, 1, 1000, errbuf);
	if (handle == NULL) {
		fprintf(stderr, "Couldn't open device %s: %s\n", dev, errbuf);
		exit(EXIT_FAILURE);
	}

	/* make sure we're capturing on an Ethernet device [2] */
	if (pcap_datalink(handle) != DLT_EN10MB) {
		fprintf(stderr, "%s is not an Ethernet\n", dev);
		exit(EXIT_FAILURE);
	}

	/* compile the filter expression */
	if (pcap_compile(handle, &fp, filter_exp, 0, net) == -1) {
		fprintf(stderr, "Couldn't parse filter %s: %s\n",
		    filter_exp, pcap_geterr(handle));
		exit(EXIT_FAILURE);
	}

	/* apply the compiled filter */
	if (pcap_setfilter(handle, &fp) == -1) {
		fprintf(stderr, "Couldn't install filter %s: %s\n",
		    filter_exp, pcap_geterr(handle));
		exit(EXIT_FAILURE);
	}

	clsetupf(&cl, "src/dfamatch.cl", "dfamatch", NULL, devpos, -1);

	xpb = xpktbuf_new(queuesiz, queuesiz * MAXPKTLEN, sizeof(unsigned int),
	    mapped ? XPB_MAPPED : XPB_DEFAULT, cl.ctx, cl.queue);
	xdfa = xdfa_new();
	load_patterns(xdfa, patterns);
	xdfa_compile(xdfa);

	/* map transition table to device memory */
	xdfa_cl_init(xdfa, cl.ctx);
	DPRINTF_LU(256 * xdfa->trcount * sizeof(int));

	/* now we can set our callback function */
	pcap_loop(handle, 0, got_packet, NULL); // -1 or 0 for permanent packet capturing



    
	// while (1) {
	// 	if (fgets(buf, 1500, input) == NULL) {
	// 		DPRINTF_D(xpb->in->cnt);
	// 		DPRINTF_D(xpb->in->used);

	// 		xpktbuf_syncstreams(xpb);
			
	// 		/* erase return cells */
	// 		memset(xpb->tmp->res, 0, queuesiz * sizeof(unsigned int));

	// 		gettimeofday(&start_time, NULL);
			
	// 		/* run */
	// 		xpktbuf_copytodev(xpb);
	// 		kdfamatchpb(&cl, xpb->tmp, xdfa, blksiz);
	// 		xpktbuf_copytohost(xpb);
		
	// 		gettimeofday(&end_time, NULL);
	// 		elapsed_time = (end_time.tv_sec * 1000 + end_time.tv_usec) - (start_time.tv_sec * 1000 + start_time.tv_usec); 


	// 		/* get results */
	// 		e = clFinish(xpb->queue);
	// 		if (e != CL_SUCCESS)
	// 			ERRXV(1, "finish: %s", clstrerror(e));
	// 		printf("\n");
	// 		for (int i = 0; i < xpb->tmp->cnt; i++) {
	// 			if (xpb->tmp->res[i]) {
	// 				if (verbose == 1){
	// 					printf("\nMatch: ", i);
	// 					printf(&xpb->tmp->buf[xpb->tmp->off[i]]);
	// 					printf("\n");
	// 				}

	// 				matches++;
	// 			}
	// 		}

	// 		printf("Total matches: %d\n", matches);
	// 		printf("Execution time (usecs): %lld\n", elapsed_time);
	// 		break;
	// 	}

	// 	if(xpktbuf_addtoin(xpb, (unsigned char *)buf, strlen(buf) + 1) > 0)
	// 		continue;

	// 	DPRINTF_D(xpb->in->cnt);
	// 	DPRINTF_D(xpb->in->used);

	// 	xpktbuf_syncstreams(xpb);
		
	// 	/* erase return cells */
	// 	memset(xpb->tmp->res, 0, queuesiz * sizeof(unsigned int));
		
	// 	gettimeofday(&start_time, NULL);

	// 	/* run */
	// 	xpktbuf_copytodev(xpb);
	// 	kdfamatchpb(&cl, xpb->tmp, xdfa, blksiz);
	// 	xpktbuf_copytohost(xpb);

	// 	gettimeofday(&end_time, NULL);
	// 	elapsed_time = (end_time.tv_sec * 1000 + end_time.tv_usec) - (start_time.tv_sec * 1000 + start_time.tv_usec); 

	// 	/* get results */
	// 	e = clFinish(xpb->queue);
	// 	if (e != CL_SUCCESS)
	// 		ERRXV(1, "finish: %s", clstrerror(e));

	// 	for (int i = 0; i < xpb->tmp->cnt; i++) {
	// 		if (xpb->tmp->res[i]) {
	// 			if (verbose == 1){
	// 				printf("\nMatch: ", i);
	// 				printf(&xpb->tmp->buf[xpb->tmp->off[i]]);
	// 				printf("\n");			
	// 			}		
	// 			matches++;
	// 		}
	// 	}
	// 	xpktbuf_addtoin(xpb, (unsigned char *)buf, strlen(buf + 1));
	// }

	return 0;
}
