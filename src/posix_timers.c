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

    getrusage(RUSAGE_SELF, &r);
    *cpu_time = r.ru_utime.tv_sec + r.ru_utime.tv_usec * 1.0E-6;

    gettimeofday(&t, (struct timezone *) NULL);
    *elapsed_time = t.tv_sec + t.tv_usec * 1.0E-6;
}

#ifdef	__cplusplus
}
#endif
