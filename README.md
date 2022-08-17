# gnort_2.9.20
Snort (version: 2.9.20) running Aho Corasick (Pattern Matching) in GPU


Step 1: give input text and patterns in ac_cuda.cu (main) so it runs AC algorithm in GPU device.

Step 2: Snort utilizes ac_cuda.cu


./ac_cuda ushers he us she hers his


Text sting: ushers

Pattern[0]: he

Pattern[1]: us

Pattern[2]: she

Pattern[3]: hers

Pattern[4]: his

numPatterns: 5


Found pattern: us

Found pattern: he

Found pattern: she

Found pattern: hers

nfound: 4

normal pgm end



Printing output fixed.
