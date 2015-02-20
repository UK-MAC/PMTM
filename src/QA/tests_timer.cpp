/* 
 * File:   tests_timer.cpp
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

int main(int argc, char** argv)
{
    return run_tests(argc, argv);
}

TEST_CASE( "timer/create", "Creating a timer in the default timer group should return valid timer id" )
{
    PmtmWrapper pmtm("test_timing_file_");

    PMTM_timer_t timer_id = ((PMTM_timer_t) -1);
    CHECKED_PMTM_CALL( PMTM_create_timer(PMTM_DEFAULT_GROUP, &timer_id, "Timer1", PMTM_TIMER_NONE) );
    REQUIRE( timer_id != ((PMTM_timer_t) -1) );
        
    MPI_Barrier(MPI_COMM_WORLD);
}

TEST_CASE( "timer/commas", "Creating a timer with a comma in its name should result in a timer with the comma stripped" )
{
    PmtmWrapper pmtm("test_timing_file_");

    PMTM_timer_t timer_id = ((PMTM_timer_t) -1);
    CHECKED_PMTM_CALL( PMTM_create_timer(PMTM_DEFAULT_GROUP, &timer_id, "Timer1,2,3", PMTM_TIMER_NONE) );
    REQUIRE( timer_id != ((PMTM_timer_t) -1) );

    pmtm.finalize();

    if (rank == 0) {
        std::vector<std::string> lines = check_header(pmtm.read_output_file());
        lines = check_overheads(lines);
        check_timer(lines.at(0), 0, 0, "Timer1 2 3");
    }
        
    MPI_Barrier(MPI_COMM_WORLD);
}

TEST_CASE( "timer/timer_none", "With the timer none timer type only the times for each rank should be printed" )
{
    PmtmWrapper pmtm("test_timing_file_");

    PMTM_timer_t timer_id = ((PMTM_timer_t) -1);
    CHECKED_PMTM_CALL( PMTM_create_timer(PMTM_DEFAULT_GROUP, &timer_id, "Timer1", PMTM_TIMER_NONE) );
    REQUIRE( timer_id != ((PMTM_timer_t) -1) );

    pmtm.finalize();

    if (rank == 0) {
        std::vector<std::string> lines = check_header(pmtm.read_output_file());
        lines = check_overheads(lines);

        REQUIRE( lines.size() == nprocs + 2 );
        for (int idx = 0; idx < nprocs; ++idx) {
            check_timer(lines.at(idx), idx, 0, "Timer1");
        }
        REQUIRE( lines.at(nprocs) == "" );
    }
        
    MPI_Barrier(MPI_COMM_WORLD);
}

TEST_CASE( "timer/timer_max", "With the 'timer max' timer type the max time on the ranks should be printed along with the times from each rank" )
{
    PmtmWrapper pmtm("test_timing_file_");

    PMTM_timer_t timer_id = ((PMTM_timer_t) -1);
    CHECKED_PMTM_CALL( PMTM_create_timer(PMTM_DEFAULT_GROUP, &timer_id, "Timer1", PMTM_TIMER_MAX) );
    REQUIRE( timer_id != ((PMTM_timer_t) -1) );

    pmtm.finalize();

    if (rank == 0) {
        std::vector<std::string> lines = check_header(pmtm.read_output_file());
        lines = check_overheads(lines);

        REQUIRE( lines.size() == nprocs + 3 );
        for (int idx = 0; idx < nprocs; ++idx) {
            check_timer(lines.at(idx), idx, 0, "Timer1");
        }
        check_timer(lines.at(nprocs), "Rank Maximum", "Timer1");
        REQUIRE( lines.at(nprocs + 1) == "" );
    }
        
    MPI_Barrier(MPI_COMM_WORLD);
}

TEST_CASE( "timer/timer_min", "With the 'timer min' timer type the min time on the ranks should be printed along with the times from each rank" )
{
    PmtmWrapper pmtm("test_timing_file_");

    PMTM_timer_t timer_id = ((PMTM_timer_t) -1);
    CHECKED_PMTM_CALL( PMTM_create_timer(PMTM_DEFAULT_GROUP, &timer_id, "Timer1", PMTM_TIMER_MIN) );
    REQUIRE( timer_id != ((PMTM_timer_t) -1) );

    pmtm.finalize();

    if (rank == 0) {
        std::vector<std::string> lines = check_header(pmtm.read_output_file());
        lines = check_overheads(lines);

        REQUIRE( lines.size() == nprocs + 3 );
        for (int idx = 0; idx < nprocs; ++idx) {
            check_timer(lines.at(idx), idx, 0, "Timer1");
        }
        check_timer(lines.at(nprocs), "Rank Minimum", "Timer1");
        REQUIRE( lines.at(nprocs + 1) == "" );
    }
        
    MPI_Barrier(MPI_COMM_WORLD);
}

TEST_CASE( "timer/timer_avg", "With the 'timer avg' timer type the avg time on the ranks should be printed along with the times from each rank" )
{
    PmtmWrapper pmtm("test_timing_file_");

    PMTM_timer_t timer_id = ((PMTM_timer_t) -1);
    CHECKED_PMTM_CALL( PMTM_create_timer(PMTM_DEFAULT_GROUP, &timer_id, "Timer1", PMTM_TIMER_AVG) );
    REQUIRE( timer_id != ((PMTM_timer_t) -1) );

    pmtm.finalize();

    if (rank == 0) {
        std::vector<std::string> lines = check_header(pmtm.read_output_file());
        lines = check_overheads(lines);

        REQUIRE( lines.size() == nprocs + 3 );
        for (int idx = 0; idx < nprocs; ++idx) {
            check_timer(lines.at(idx), idx, 0, "Timer1");
        }
        check_timer(lines.at(nprocs), "Rank Average", "Timer1");
        REQUIRE( lines.at(nprocs + 1) == "" );
    }
        
    MPI_Barrier(MPI_COMM_WORLD);
}

TEST_CASE( "timer/timer_all", "With the 'timer all' timer type the different stats should be printed along with the times from each rank" )
{
    PmtmWrapper pmtm("test_timing_file_");

    PMTM_timer_t timer_id = ((PMTM_timer_t) -1);
    CHECKED_PMTM_CALL( PMTM_create_timer(PMTM_DEFAULT_GROUP, &timer_id, "Timer1", PMTM_TIMER_ALL) );
    REQUIRE( timer_id != ((PMTM_timer_t) -1) );

    pmtm.finalize();

    if (rank == 0) {
        std::vector<std::string> lines = check_header(pmtm.read_output_file());
        lines = check_overheads(lines);

        REQUIRE( lines.size() == nprocs + 5 );
        for (int idx = 0; idx < nprocs; ++idx) {
            check_timer(lines.at(idx), idx, 0, "Timer1");
        }
        check_timer(lines.at(nprocs), "Rank Average", "Timer1");
        check_timer(lines.at(nprocs + 1), "Rank Maximum", "Timer1");
        check_timer(lines.at(nprocs + 2), "Rank Minimum", "Timer1");
        REQUIRE( lines.at(nprocs + 3) == "" );
    }
        
    MPI_Barrier(MPI_COMM_WORLD);
}

TEST_CASE( "timer/timer_int", "With the 'timer int' timer type no timing information should be written to the file." )
{
    PmtmWrapper pmtm("test_timing_file_");

    PMTM_timer_t timer_id = ((PMTM_timer_t) -1);
    CHECKED_PMTM_CALL( PMTM_create_timer(PMTM_DEFAULT_GROUP, &timer_id, "Timer1", PMTM_TIMER_INT) );
    REQUIRE( timer_id != ((PMTM_timer_t) -1) );

    pmtm.finalize();

    if (rank == 0) {
        std::vector<std::string> lines = check_header(pmtm.read_output_file());
        lines = check_overheads(lines);

	REQUIRE( lines.size() == 2 );
        REQUIRE( lines.at(0) == "" );
    }
        
    MPI_Barrier(MPI_COMM_WORLD);
}

TEST_CASE( "timer/timer_mma", "With the 'timer mma' timer type only the Max, Min and Average timer stats should be printed not any rank information" )
{
    PmtmWrapper pmtm("test_timing_file_");

    PMTM_timer_t timer_id = ((PMTM_timer_t) -1);
    CHECKED_PMTM_CALL( PMTM_create_timer(PMTM_DEFAULT_GROUP, &timer_id, "Timer1", PMTM_TIMER_MMA) );
    REQUIRE( timer_id != ((PMTM_timer_t) -1) );

    pmtm.finalize();

    if (rank == 0) {
        std::vector<std::string> lines = check_header(pmtm.read_output_file());
        lines = check_overheads(lines);

        REQUIRE( lines.size() == 5 );
        check_timer(lines.at(0), "Rank Average", "Timer1");
        check_timer(lines.at(1), "Rank Maximum", "Timer1");
        check_timer(lines.at(2), "Rank Minimum", "Timer1");
        REQUIRE( lines.at(3) == "" );
    }
        
    MPI_Barrier(MPI_COMM_WORLD);
} 

TEST_CASE( "timer/timer_avo", "With the 'timer avo' timer type only the Average timer stats should be printed not any rank, max or min information." )
{
    PmtmWrapper pmtm("test_timing_file_");

    PMTM_timer_t timer_id = ((PMTM_timer_t) -1);
    CHECKED_PMTM_CALL( PMTM_create_timer(PMTM_DEFAULT_GROUP, &timer_id, "Timer1", PMTM_TIMER_AVO) );
    REQUIRE( timer_id != ((PMTM_timer_t) -1) );

    pmtm.finalize();

    if (rank == 0) {
        std::vector<std::string> lines = check_header(pmtm.read_output_file());
        lines = check_overheads(lines);

        REQUIRE( lines.size() == 3 );
        check_timer(lines.at(0), "Rank Average", "Timer1");
        REQUIRE( lines.at(1) == "" );
    }
        
    MPI_Barrier(MPI_COMM_WORLD);
} 

TEST_CASE( "timer/start_stop", "Starting and stopping a timer should increase that timer's count")
{
    PmtmWrapper pmtm("test_timing_file_");

    PMTM_timer_t timer_id = ((PMTM_timer_t) -1);
    CHECKED_PMTM_CALL( PMTM_create_timer(PMTM_DEFAULT_GROUP, &timer_id, "Timer1", PMTM_TIMER_NONE) );
    REQUIRE( timer_id != ((PMTM_timer_t) -1) );

    const int num_timings = 10;

    uint timing_idx;
    for (timing_idx = 0; timing_idx < num_timings; ++timing_idx) {
        PMTM_timer_start(timer_id);
        PMTM_timer_stop(timer_id);
    }

    pmtm.finalize();

    if (rank == 0) {
        std::vector<std::string> lines = check_header(pmtm.read_output_file());
        lines = check_overheads(lines);

        REQUIRE( lines.size() == nprocs + 2 );
        for (int idx = 0; idx < nprocs; ++idx) {
            check_timer(lines.at(idx), idx, 0, "Timer1", num_timings);
        }
        REQUIRE( lines.at(nprocs) == "" );
    }
        
    MPI_Barrier(MPI_COMM_WORLD);
}

TEST_CASE( "timer/pause_continue", "Pausing and continuing a timer should increase that timer's pause count" )
{
    PmtmWrapper pmtm("test_timing_file_");

    PMTM_timer_t timer_id = ((PMTM_timer_t) -1);
    CHECKED_PMTM_CALL( PMTM_create_timer(PMTM_DEFAULT_GROUP, &timer_id, "Timer1", PMTM_TIMER_NONE) );
    REQUIRE( timer_id != ((PMTM_timer_t) -1) );

    const int num_pauses = 10;
    PMTM_timer_start(timer_id);

    uint pause_idx;
    for (pause_idx = 0; pause_idx < num_pauses; ++pause_idx) {
        PMTM_timer_pause(timer_id);
        PMTM_timer_continue(timer_id);
    }

    PMTM_timer_stop(timer_id);

    pmtm.finalize();

    if (rank == 0) {
        std::vector<std::string> lines = check_header(pmtm.read_output_file());
        lines = check_overheads(lines);

        REQUIRE( lines.size() == nprocs + 2 );
        for (int idx = 0; idx < nprocs; ++idx) {
            check_timer(lines.at(idx), idx, 0, "Timer1", 1, num_pauses);
        }
        REQUIRE( lines.at(nprocs) == "" );
    }
        
    MPI_Barrier(MPI_COMM_WORLD);
}

TEST_CASE( "timer/timing", "Timing a one second wait should return a time close to one second" )
{
    PmtmWrapper pmtm("test_timing_file_");

    PMTM_timer_t timer_id = ((PMTM_timer_t) -1);
    CHECKED_PMTM_CALL( PMTM_create_timer(PMTM_DEFAULT_GROUP, &timer_id, "Timer1", PMTM_TIMER_NONE) );
    REQUIRE( timer_id != ((PMTM_timer_t) -1) );

    const int num_seconds = 1;

    PMTM_timer_start(timer_id);
    sleep(num_seconds);
    PMTM_timer_stop(timer_id);

    pmtm.finalize();

    if (rank == 0) {
        std::vector<std::string> lines = check_header(pmtm.read_output_file());
        lines = check_overheads(lines);

        REQUIRE( lines.size() == nprocs + 2 );
        for (int idx = 0; idx < nprocs; ++idx) {
            check_timer(lines.at(idx), idx, 0, "Timer1", 1, 0, num_seconds);
        }
        REQUIRE( lines.at(nprocs) == "" );
    }
        
    MPI_Barrier(MPI_COMM_WORLD);
}

TEST_CASE( "timer/sampling_frequency", "Specifying the sampling frequency should cause the timer to take measurements at that rate" )
{
    PmtmWrapper pmtm("test_timing_file_");

    const int frequency = 2;

    PMTM_timer_t timer_id = ((PMTM_timer_t) -1);
    CHECKED_PMTM_CALL( PMTM_create_timer(PMTM_DEFAULT_GROUP, &timer_id, "Timer1", PMTM_TIMER_NONE) );
    CHECKED_PMTM_CALL( PMTM_set_sample_mode(timer_id, frequency, PMTM_NO_MAX) );

    const int num_timings = 10;
    const int exp_count = num_timings / frequency;

    uint timing_idx;
    for (timing_idx = 0; timing_idx < num_timings; ++timing_idx) {
        PMTM_timer_start(timer_id);
        PMTM_timer_stop(timer_id);
    }

    pmtm.finalize();

    if (rank == 0) {
        std::vector<std::string> lines = check_header(pmtm.read_output_file());
        lines = check_overheads(lines);

        REQUIRE( lines.size() == nprocs + 2 );
        for (int idx = 0; idx < nprocs; ++idx) {
            check_timer(lines.at(idx), idx, 0, "Timer1", exp_count);
        }
        REQUIRE( lines.at(nprocs) == "" );
    }
        
    MPI_Barrier(MPI_COMM_WORLD);
}

TEST_CASE( "timer/sampling_maximum", "Specifying the maximum samples should cause the timer to stop sampling after that maximum" )
{
    PmtmWrapper pmtm("test_timing_file_");

    const int max_samples = 7;

    PMTM_timer_t timer_id = ((PMTM_timer_t) -1);
    CHECKED_PMTM_CALL( PMTM_create_timer(PMTM_DEFAULT_GROUP, &timer_id, "Timer1", PMTM_TIMER_NONE) );
    CHECKED_PMTM_CALL( PMTM_set_sample_mode(timer_id, PMTM_DEFAULT_FREQ, max_samples) );

    const int num_timings = 10;

    uint timing_idx;
    for (timing_idx = 0; timing_idx < num_timings; ++timing_idx) {
        PMTM_timer_start(timer_id);
        PMTM_timer_stop(timer_id);
    }

    pmtm.finalize();

    if (rank == 0) {
        std::vector<std::string> lines = check_header(pmtm.read_output_file());
        lines = check_overheads(lines);

        REQUIRE( lines.size() == nprocs + 2 );
        for (int idx = 0; idx < nprocs; ++idx) {
            check_timer(lines.at(idx), idx, 0, "Timer1", max_samples);
        }
        REQUIRE( lines.at(nprocs) == "" );
    }
        
    MPI_Barrier(MPI_COMM_WORLD);
}


