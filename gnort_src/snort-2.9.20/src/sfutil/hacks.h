#ifndef _HACKS_H_
#define _HACKS_H_

/* alarm once using handler and timeout */
void
setalarmonce(void (*handler)(void), unsigned msec);

/* alarm periodic using handler and timeout */
void
setalarmperiodic(void (*handler)(void), unsigned msec);

/* set process affinity to one cpu */
int
setaffinity(int cpuid);

/* go to sleep for sec seconds */
#ifdef _WIN32
void
sleep(unsigned sec);
#else
/* it is found in unistd.h */
#endif /* _WIN32 */

/* return counter in usecs */
unsigned long long
gettime(void);

#endif /* _HACKS_H_ */
