# ============== UNDER DEVELOPMENT ==================

# gnort_2.9.20
Snort (version: 2.9.20): Pattern Matching/search in GPU


## gpuregex 4

To identify the device position (e.g., 0, 1, 2, ...), run:

1) $ clinfo -l

->Compile the source code

2) $ cd gpuregex; make

->Run the executable 

3) $ ./bin/gpuregex -p patterns.dat -i TheAdventuresOfSherlockHolmes.dat -d 0 -v -m 0

Notes: 
-p: pattern file
-i: input file
-d: device position
-v: verbose (prints matches -- default does not print)
-m: 1 for cpu & shared gpu, 0 for discrete gpu

## snort

1) capture and send packet: fpdetect.c
2) use custom method provided by gpuregex repo (instead of acsmx, bnfe etc.) -> mpse.c possibly
3) ...

## REPO CHANGES LOG:

CUDA files were moved to obsolete/ (use openCL instead of CUDA).

Opencl acsmxCL.c, snort's pattern matching algorithm moved to obsolete/ (use provided repo by Eva Papadogiannaki).
