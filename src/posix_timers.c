/*
 * File:   posix_timers.c
 * Author: AWE Plc.
 *
 * Implementation of set_timers which gets the CPU and Elapsed time on systems
 * using basic POSIX timing functions.
 */

#include "timers.h"

#include <sys/time.h>
#include <sys/times.h>
#include <sys/resource.h>
#include <stdlib.h>

#ifdef	__cplusplus
extern "C" {
#endif

void set_timers(double * cpu_time, double * elapsed_time)
{
    struct rusage r;
    struct timeval t;

    getrusage(RUSAGE_SELF, &r); // OR for THREAD getrusage(RUSAGE_THREAD, &r);
    *cpu_time = r.ru_utime.tv_sec + r.ru_utime.tv_usec * 1.0E-6;

    gettimeofday(&t, (struct timezone *) NULL);
    *elapsed_time = t.tv_sec + t.tv_usec * 1.0E-6;
}

#if 0
// POSIX clock_ version - Need to test

void set_timers(double * cpu_time, double * elapsed_time)
{
    struct timespec ts;

    // To get the thread CPUTIME instead of the process CPUTIME use
    // CLOCK_THREAD_CPUTIME_ID.
    //
    // Need to just test the CPUTIME to make sure it runs like the rusage
    // timer as it ought to be quicker due to not getting lots of guff.

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
    *cpu_time = ts.tv_sec + ts.tv_nsec * (1.0 / 1000000000.0);

    clock_gettime(CLOCK_REALTIME, &ts);
    *elapsed_time = ts.tv_sec + ts.tv_nsec * (1.0 / 1000000000.0);
}
#endif


#ifdef	__cplusplus
}
#endif
