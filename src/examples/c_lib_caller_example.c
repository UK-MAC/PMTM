/* 
 * File:   example_lib_caller.c
 * Author: AWE Plc.
 *
 */

#include "c_lib_example.h"
#include "pmtm.h"

#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

#define CHECKED_MPI_CALL(f) do { mpi_status = f; \
        if (mpi_status != MPI_SUCCESS) { char msg[1024]; int msg_len; \
        MPI_Error_string(mpi_status, msg, &msg_len); fputs(msg, stderr); } } while (0)
#define CHECKED_PMTM_CALL(f) do { pmtm_status = f; \
        if (pmtm_status != PMTM_SUCCESS) { fputs(PMTM_get_error_message(pmtm_status), stderr); } } while (0) 

int main(int argc, char** argv)
{
    int mpi_status;
    PMTM_error_t pmtm_status;

    CHECKED_MPI_CALL( MPI_Init(&argc, &argv) );

    PMTM_timer_t app_timer;

    CHECKED_PMTM_CALL( PMTM_init("c_lib_caller_example_", "Example Application") );
    
    CHECKED_PMTM_CALL( PMTM_create_timer(PMTM_DEFAULT_GROUP, &app_timer, "Application Time", PMTM_TIMER_ALL) );

    PMTM_timer_start(app_timer);
    double res = lib_function();
    PMTM_timer_stop(app_timer);

    printf("Result = %12.6E\n", res);

    CHECKED_PMTM_CALL( PMTM_finalize() );

    CHECKED_MPI_CALL( MPI_Finalize() );

    return EXIT_SUCCESS;
}

