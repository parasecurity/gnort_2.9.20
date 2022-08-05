# include <stdio.h>
#include <cuda.h>



/*
*  Text Data Buffer
*/ 
static unsigned char text[512];
static const char *text_ptr = (const char*)text;

ACCUDA_STRUCT * acNew () {
  // print_packet_buffer_info();

  // if (isStreamInitialized == 0) {
	//   cudaStreamCreate(&stream);
  //   #ifdef DEBUG
  //   CUT_CHECK_ERROR("cudaStreamCreate");
  //   #endif
  //   isStreamInitialized = 1;
}

void printCommandInfo(int n, const char ** args){
    for (int i = 0; i < n; i++){
      printf("ARGUEMENT %d: %s\n", i, args[i]);
    }
}
#define ACCUDA_MAIN // uncomment this to run individually: nvcc -o ac_cuda ../src/sfutil/ac_cuda.cu
#ifdef ACCUDA_MAIN

int main (int argc, const char **argv){
  int i, nocase = 0;

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

