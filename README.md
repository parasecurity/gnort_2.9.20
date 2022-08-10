# gnort_2.9.20
Snort (version: 2.9.20) running Aho Corasick (Pattern Matching) in GPU


Step 1: give input text and patterns in ac_cuda.cu (main) so it runs AC algorithm in GPU device.

Step 2: Snort utilizes ac_cuda.cu


./ac_cuda ushers he she his hers


Text sting: ushers
Pattern[0]: he
Pattern[1]: she
Pattern[2]: his
Pattern[3]: hers
numPatterns: 4
he
hers
nfound: 2
normal pgm end


output wrong, needs to be fixed
