/*
 * File:   example.c
 * Author: AWE Plc.
 *
 */

#include "pmtm.h"
#include "flags.h"

#include <stdio.h>
#include <stdlib.h>

#ifndef SERIAL
#  include "mpi.h"
#endif

int main(int argc, char ** argv)
{
#ifndef SERIAL
    MPI_Init(&argc, &argv);
#endif

    PMTM_timer_t app_time;
    
    PMTM_log_flags(COMPILE_FLAGS);
    PMTM_init("c_example_", "Example Application");

    uint loop_idx;
    for (loop_idx = 1; loop_idx <= 2; ++loop_idx) {
        PMTM_parameter_output(PMTM_DEFAULT_INSTANCE, "Always - Rank 0",       PMTM_OUTPUT_ALWAYS,    PMTM_FALSE, "%d", loop_idx);
        PMTM_parameter_output(PMTM_DEFAULT_INSTANCE, "Always - All Ranks",    PMTM_OUTPUT_ALWAYS,    PMTM_TRUE,  "%d", loop_idx);
    }
    for (loop_idx = 1; loop_idx <= 2; ++loop_idx) {
        PMTM_parameter_output(PMTM_DEFAULT_INSTANCE, "Once - Rank 0",         PMTM_OUTPUT_ONCE,      PMTM_FALSE, "%d", loop_idx);
        PMTM_parameter_output(PMTM_DEFAULT_INSTANCE, "Once - All Ranks",      PMTM_OUTPUT_ONCE,      PMTM_TRUE,  "%d", loop_idx);
    }
    for (loop_idx = 1; loop_idx <= 5; ++loop_idx) {
        PMTM_parameter_output(PMTM_DEFAULT_INSTANCE, "On Change - Rank 0",    PMTM_OUTPUT_ON_CHANGE, PMTM_FALSE, "%d", loop_idx / 5);
        PMTM_parameter_output(PMTM_DEFAULT_INSTANCE, "On Change - All Ranks", PMTM_OUTPUT_ON_CHANGE, PMTM_TRUE,  "%d", loop_idx / 5);
    }
    
    PMTM_create_timer(PMTM_DEFAULT_GROUP, &app_time, "Application Time", PMTM_TIMER_ALL);

    PMTM_timer_start(app_time);
    double res = 1;
    for (loop_idx = 1; loop_idx <= 10000; ++loop_idx) {
        res += (res / loop_idx);
    }
    PMTM_timer_stop(app_time);

    printf("Result = %12.6E\n", res);

    PMTM_finalize();

#ifndef SERIAL
    MPI_Finalize();
#endif

    return EXIT_SUCCESS;
}
