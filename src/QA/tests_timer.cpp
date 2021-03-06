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

/**
 * @ingroup tests_timer
 * 
 * Tests that correctly creating a new timer in the default timer group using \ref PMTM_create_timer returns a valid timer id (not -1)
 * 
 */
TEST_CASE( "tests_timer.cpp/create", "Creating a timer in the default timer group should return valid timer id" )
{
    PmtmWrapper pmtm("test_timing_file_");

    PMTM_timer_t timer_id = ((PMTM_timer_t) -1);
    CHECKED_PMTM_CALL( PMTM_create_timer(PMTM_DEFAULT_GROUP, &timer_id, "Timer1", PMTM_TIMER_NONE) );
    REQUIRE( timer_id != ((PMTM_timer_t) -1) );
        
    MPI_Barrier(MPI_COMM_WORLD);
}

/**
 * @ingroup tests_timer
 * 
 * Tests that giving \ref PMTM_create_timer a timer name containing a comma prints out a timer name with the comma removed.
 * 
 */
TEST_CASE( "tests_timer.cpp/commas", "Creating a timer with a comma in its name should result in a timer with the comma stripped" )
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

/**
 * @ingroup tests_timer
 * 
 * Tests that using the timer type \c PMTM_TIMER_NONE in \ref PMTM_create_timer results in only the rank information being printed
 * 
 */
TEST_CASE( "tests_timer.cpp/timer_none", "With the timer none timer type only the times for each rank should be printed" )
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

/**
 * @ingroup tests_timer
 * 
 * Tests that using the timer type \c PMTM_TIMER_MAX in \ref PMTM_create_timer results in only the rank information and maximum time being printed
 * 
 */
TEST_CASE( "tests_timer.cpp/timer_max", "With the 'timer max' timer type the max time on the ranks should be printed along with the times from each rank" )
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

/**
 * @ingroup tests_timer
 * 
 * Tests that using the timer type \c PMTM_TIMER_MIN in \ref PMTM_create_timer results in only the rank information and the minimum time being printed
 * 
 */
TEST_CASE( "tests_timer.cpp/timer_min", "With the 'timer min' timer type the min time on the ranks should be printed along with the times from each rank" )
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

/**
 * @ingroup tests_timer
 * 
 * Tests that using the timer type \c PMTM_TIMER_AVG in \ref PMTM_create_timer results in only the rank information and the overall average time being printed
 * 
 */
TEST_CASE( "tests_timer.cpp/timer_avg", "With the 'timer avg' timer type the avg time on the ranks should be printed along with the times from each rank" )
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

/**
 * @ingroup tests_timer
 * 
 * Tests that using the timer type \c PMTM_TIMER_ALL in \ref PMTM_create_timer results in the rank information, maximum, minimum and average times being printed
 * 
 */
TEST_CASE( "tests_timer.cpp/timer_all", "With the 'timer all' timer type the different stats should be printed along with the times from each rank" )
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

/**
 * @ingroup tests_timer
 * 
 * Tests that using the timer type \c PMTM_TIMER_INT in \ref PMTM_create_timer results in no timer information being printed
 * 
 */
TEST_CASE( "tests_timer.cpp/timer_int", "With the 'timer int' timer type no timing information should be written to the file." )
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

/**
 * @ingroup tests_timer
 * 
 * Tests that using the timer type \c PMTM_TIMER_MMA in \ref PMTM_create_timer results in only maximum, minimum and average times being printed
 * 
 */
TEST_CASE( "tests_timer.cpp/timer_mma", "With the 'timer mma' timer type only the Max, Min and Average timer stats should be printed not any rank information" )
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

/**
 * @ingroup tests_timer
 * 
 * Tests that using the timer type \c PMTM_TIMER_AVO in \ref PMTM_create_timer results in only the average time being printed
 * 
 */
TEST_CASE( "tests_timer.cpp/timer_avo", "With the 'timer avo' timer type only the Average timer stats should be printed not any rank, max or min information." )
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

/**
 * @ingroup tests_timer
 * 
 * Tests that starting and stopping a timer using \ref PMTM_timer_start and \ref PMTM_timer_stop results in the timer's count being incremented
 * 
 */
TEST_CASE( "tests_timer.cpp/start_stop", "Starting and stopping a timer should increase that timer's count")
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

/**
 * @ingroup tests_timer
 * 
 * Tests pausing and continuing a timer using the \ref PMTM_timer_pause and \ref PMTM_timer_continue functions results in the timer's pause count being incremented.
 * 
 */
TEST_CASE( "tests_timer.cpp/pause_continue", "Pausing and continuing a timer should increase that timer's pause count" )
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

/**
 * @ingroup tests_timer
 * 
 * Tests that timing a one second wait returns a time close to one second
 * 
 */
TEST_CASE( "tests_timer.cpp/timing", "Timing a one second wait should return a time close to one second" )
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

/**
 * @ingroup tests_timer
 * 
 * Tests that using \ref PMTM_set_sample_mode to specify the sample frequency correctly enables \c PMTM to only sample at the required rate
 * 
 */
TEST_CASE( "tests_timer.cpp/sampling_frequency", "Specifying the sampling frequency should cause the timer to take measurements at that rate" )
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

/**
 * @ingroup tests_timer
 * 
 * Tests that using \ref PMTM_set_sample_mode to set the maximum number of samples should set \c PMTM to stop sampling after the required count is reached.
 * 
 */
TEST_CASE( "tests_timer.cpp/sampling_maximum", "Specifying the maximum samples should cause the timer to stop sampling after that maximum" )
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


