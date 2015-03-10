/* 
 * File:   tests_threads.cpp
 * Author: Miller (AWE Plc)
 *
 */

#include "tests_utils.hpp"

#include <vector>
#include <string>

#include <string.h>
#include <unistd.h>

#include "catch.hpp"

#include "pmtm.h"

#include <omp.h>

int main(int argc, char** argv)
{
    return run_tests(argc, argv);
}


TEST_CASE( "tests_threads.cpp/parallel_timing", "Timing a one second wait should return a time close to one second from each thread" )
{
    PmtmWrapper pmtm("test_timing_file_");

    int threads;
    PMTM_timer_t * timer_id = NULL;
    PMTM_error_t * error_code = NULL;

    #pragma omp parallel default(none) shared(threads, timer_id, error_code)
    {
        #pragma omp master
        {
            threads = omp_get_num_threads();
            timer_id = new PMTM_timer_t [threads];
            error_code = new PMTM_error_t [threads];
        }

        #pragma omp barrier

        int thr = omp_get_thread_num();
        error_code[thr] = PMTM_create_timer(PMTM_DEFAULT_GROUP, &timer_id[thr], "Timer1", PMTM_TIMER_NONE);
    }

    // Not clear if the test rig is thread safe so drop out of the parallel region to test outputs.

    for (int thr = 0; thr < threads; ++thr) {
        REQUIRE( error_code[thr] == PMTM_SUCCESS );
        REQUIRE( timer_id[thr] != ((PMTM_timer_t) -1) );
    }

    const int num_seconds = 1;

    #pragma omp parallel default(none) shared(timer_id)
    {
        int thr = omp_get_thread_num();
        PMTM_timer_start(timer_id[thr]);
        sleep(num_seconds);
        PMTM_timer_stop(timer_id[thr]);
    }

    pmtm.finalize();

    if (rank == 0) {
        std::vector<std::string> lines = check_header(pmtm.read_output_file());
        lines = check_overheads(lines);

        REQUIRE( lines.size() == nprocs*threads + 2 );
        for (int idx = 0; idx < nprocs; ++idx) {
            for (int thr = 0; thr < threads; ++thr) {
                check_timer(lines.at(idx*threads + thr), idx, thr, "Timer1", 1, 0, num_seconds);
            }
        }
        REQUIRE( lines.at(nprocs*threads) == "" );
    }
        
    MPI_Barrier(MPI_COMM_WORLD);
}

