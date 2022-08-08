#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include <cuda.h>
#include <assert.h>
// #define ACCUDA_MAIN // uncomment this to run individually: nvcc -o ac_cuda ../src/sfutil/ac_cuda.cu
#ifndef ACCUDA_MAIN
#include "/home/zxcv/snort_2.9.20/snort-2.9.20/src/snort.h" // for PacketCount struct
#endif

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#include "ac_cuda.cu.h"

#define MEM_OPTS
#ifdef MEM_OPTS
 #ifdef TEX_STATE_TABLE
 static texture <acstate_t, 1, cudaReadModeElementType> texStateTable;
 #endif
#else
static texture <acstate_t, 2, cudaReadModeElementType> texStateTable;
#endif

/*
*  Text Data Buffer
*/ 
static unsigned char text[512];
static const char *text_ptr = (const char*)text;

extern unsigned int UsePinnedMem;
#ifdef MEM_OPTS
  typedef struct packet_buffer_ {
    unsigned char bufNo; // 0 or 1
    unsigned char buf[2][BUFFER_SIZE][PKT_SNAPLEN];
    unsigned int bufPos;
  } packet_buffer_t;

  static Packet pktHdr[2][BUFFER_SIZE];

  static PORT_GROUP *pktPg[2][BUFFER_SIZE];

  static packet_buffer_t *packetBuffer = 0x00;

  static acstate_t *stateTable = NULL;
  static unsigned int stateTablePos = 0;


  static cudaStream_t stream;
  static unsigned int isStreamInitialized = 0;
  #ifdef TEX_STATE_TABLE
    static unsigned int stateTableSize = 131072; // 128MB; max number of states that can be bound to texture
  #else
    static unsigned int stateTableSize = 1048576; // 1GB;
  #endif

#endif


static void print_packet_buffer_info() {
  static int first_time=1;

  if (first_time) {
    first_time = 0;

    fprintf(stdout, "Packet buffer [%s]: %d packets, %d bytes each (Total: %d bytes)\n",
                        (UsePinnedMem) ? "PINNED" : "PAGEABLE",
                        BUFFER_SIZE, PKT_SNAPLEN, BUFFER_SIZE * PKT_SNAPLEN);
    fflush(stdout);
  }
}

ACCUDA_STRUCT * acNew () {
  // print_packet_buffer_info();

  // if (isStreamInitialized == 0) {
	//   cudaStreamCreate(&stream);
  //   #ifdef DEBUG
  //     CUT_CHECK_ERROR("cudaStreamCreate");
  //   #endif
  //   isStreamInitialized = 1;
  // }
}

void printCommandInfo(int n, const char ** args){
    for (int i = 0; i < n; i++){
      printf("ARGUEMENT %d: %s\n", i, args[i]);
    }
}
//#define ACCUDA_MAIN 
#ifdef ACCUDA_MAIN

int main (int argc, const char **argv){
  int i, nocase = 0;
  ACCUDA_STRUCT * ac;

  if (argc < 4)
  {
    fprintf (stderr,
      "Usage: ./ac_cuda acx text pattern-1 pattern-2 ... pattern-n  -nocase\n");
      // ---->argv[0], v[1] v[2]    v[3]      v[4]   ...   v[n+2]       v[n+3]
    exit (0);
  }

  // printCommandInfo(argc, argv);
  strcpy ((char *)text, argv[2]);
  for (int i = 2; i < argc; i++)
    if (strcmp (argv[i], "-nocase") == 0)
      nocase = 1;
  printf("Text string: %s\n", text);

  ac = acNew();


  for (i = 3; i < argc; i++)
  {
    if (argv[i][0] == '-')
      continue;
    memcpy ((char *)text, argv[i], strlen(argv[i]) + 1);
    printf("Pattern[%d]: %s\n",i-2 , text);
    //acAddPattern (ac, (unsigned char*)text, strlen (text_ptr), nocase, 0, 0, (unsigned char*)argv[i], i - 2);
  }

}

#endif

