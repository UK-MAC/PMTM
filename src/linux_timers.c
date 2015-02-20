/*
 * File:   linux_timers.c
 * Author: AWE Plc.
 *
 * Implementation of set_timers which gets the CPU and Elapsed time on systems
 * running the GNU Linux operating system.
 */

#include "timers.h"

#include <time.h>
#include <stdlib.h>

#ifdef	__cplusplus
extern "C" {
#endif

void set_timers(double * cpu_time, double * elapsed_time)
{
    struct timespec ts;
    struct timespec cpu_ts;

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &cpu_ts);
    *cpu_time = cpu_ts.tv_sec + cpu_ts.tv_nsec * 1.0E-9;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    *elapsed_time = ts.tv_sec + ts.tv_nsec * 1.0E-9; 
}

#ifdef	__cplusplus
}
#endif
