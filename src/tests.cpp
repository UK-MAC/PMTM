/* 
 * File:   tests.cpp
 * Author: Hollcombe (Tessella plc)
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

TEST_CASE( "finalize/reinitialise", "Finalising PMTM leaves the library in a state where it can be initialised again without error" )
{
    // Each wrapper instance initialises on creation and then finalizes on
    // destruction.
    PmtmWrapper("test_timing_file_");
    PmtmWrapper("test_timing_file_");
        
    MPI_Barrier(MPI_COMM_WORLD);
}

TEST_CASE( "initialise/valid_file_name", "Initialising PMTM with a valid noexistant file name should create that file with 0.pmtm suffix" )
{
    PmtmWrapper("test_timing_file_");
        
    MPI_Barrier(MPI_COMM_WORLD);
}

TEST_CASE( "initialise/invalid_file_name", "Initialising PMTM with an invalid file name should cause initialisation to fail" )
{
    PMTM_error_t err_code = PMTM_init("bad_dir/test_timing_file_", "Test App");
    REQUIRE( err_code == PMTM_ERROR_CANNOT_CREATE_FILE );
    PMTM_finalize();
        
    MPI_Barrier(MPI_COMM_WORLD);
}

TEST_CASE( "initialise/stdout", "Initialising PMTM with a file name of - should cause it to write to stdout" )
{
    const size_t buffer_sz = 8192;
    char buffer[buffer_sz];
    memset(buffer, '\0', buffer_sz);

    fflush(stdout);
    setbuf(stdout, buffer);

    PmtmWrapper("-");
    buffer[buffer_sz - 1] = '\0';

    if (rank == 0) {
        REQUIRE( strlen(buffer) > 0 );
    } else {
        REQUIRE( strlen(buffer) == 0 );
    }

    setbuf(stdout, NULL);
        
    MPI_Barrier(MPI_COMM_WORLD);
}

TEST_CASE( "initialise/no_output", "Initialising PMTM with an empty file name should cause it to not create an output file" )
{
    PmtmWrapper("");
        
    MPI_Barrier(MPI_COMM_WORLD);
}

TEST_CASE( "initialise/file_header", "Initialising PMTM should cause it to write a valid PMTM file header" )
{
    PmtmWrapper pmtm("test_timing_file_");
    pmtm.finalize(); // finalize to flush & close output file.

    check_header(pmtm.read_output_file());
        
    MPI_Barrier(MPI_COMM_WORLD);
}

TEST_CASE( "initialise/overheads", "Initialising PMTM should cause it to output timer overhead information into the output file" )
{
    PmtmWrapper pmtm("test_timing_file_");
    pmtm.finalize(); // finalize to flush & close output file.

    std::vector<std::string> lines = check_header(pmtm.read_output_file());
    check_overheads(lines);
        
    MPI_Barrier(MPI_COMM_WORLD);

}

TEST_CASE( "initialise/existing_file", "Initialising PMTM with a valid but existing file name should create that file with an incremented suffix" )
{
    std::string create_file = "./test_timing_file_0.pmtm";

    FileDeleter file_deleter("test_timing_file_");

    FILE * file = fopen(create_file.c_str(), "w");
    if (file == NULL) FAIL( "Failed to create test file" );
    if (fclose(file) != 0) FAIL( "Failed to close test file" );

    PmtmWrapper pmtm("test_timing_file_", 1);
        
    MPI_Barrier(MPI_COMM_WORLD);
}

TEST_CASE( "options/output_env", "Turn of the output environment options should stop the environment being output to the PMTM output file" )
{
    char * wc_command = "grep -c \"^Environ\" *.pmtm";
    
    
    {
        CHECKED_PMTM_CALL( PMTM_set_option(PMTM_OPTION_OUTPUT_ENV, PMTM_TRUE) );
        PmtmWrapper pmtm("test_timing_file_");
        pmtm.finalize();

        if (rank == 0) {
	      FILE * so = popen(wc_command,"r");
	      int bigup ;
	      if(fscanf(so,"%d",&bigup) != 1)
		bigup = -1;
	      pclose(so);
	      REQUIRE( bigup > 0 );
        }
    } // file gets deleted here.

    {
        CHECKED_PMTM_CALL( PMTM_set_option(PMTM_OPTION_OUTPUT_ENV, PMTM_FALSE) );
        PmtmWrapper pmtm("test_timing_file_");
        pmtm.finalize();

        if (rank == 0) {
	      FILE * so = popen(wc_command,"r");
	      int bigup ;
	      if(fscanf(so,"%d",&bigup) != 1)
		bigup = -1;
	      pclose(so);
	      REQUIRE( bigup == 0 );
        }
    } // file gets deleted here.

    PMTM_set_option(PMTM_OPTION_OUTPUT_ENV, PMTM_TRUE);
        
    MPI_Barrier(MPI_COMM_WORLD);
}

TEST_CASE( "options/get_specific_runtime_variables", "Test whether putting a specific Environment variable name in ${PWD}/.pmtmrc prints it to the output, but only once")
{
    char * cf_command = "echo \"PMTM_TEST_SPEC_ENV\" > .pmtmrc";
    system(cf_command);
    char * cf2_command = " echo \"PMTM_TEST_SPEC_ENV\" >> .pmtmrc";
    char * fs_command = "grep Specific *.pmtm | grep PMTM_TEST_SPEC_ENV | wc -l";
    char * rm_command = "rm .pmtmrc";
    
    setenv("PMTM_TEST_SPEC_ENV", "1", 1);
    
    {
      PmtmWrapper pmtm("test_timing_file_");
      pmtm.finalize();
      
      if (rank == 0) {
	    FILE * so = popen(fs_command,"r");
	    int bigup;
	    if(fscanf(so,"%d",&bigup) != 1)
	      bigup = -1;
	    pclose(so);
	    REQUIRE( bigup == 1 );
      }
    }
    
    system(cf2_command);
    
    {
      PmtmWrapper pmtm("test_timing_file_");
      pmtm.finalize();
      
      if (rank == 0) {
	    FILE * so = popen(fs_command,"r");
	    int bigup;
	    if(fscanf(so,"%d",&bigup) != 1)
	      bigup = -1;
	    pclose(so);
	    REQUIRE( bigup == 1 );
      }
    }
    
    if(rank == 0) {  system(rm_command); }
    
    unsetenv("PMTM_TEST_SPEC_ENV");
        
    MPI_Barrier(MPI_COMM_WORLD);
}


TEST_CASE( "options/get_specific_runtime_variables_internal", "Test whether PMTM_output_specific_runtime_variable prints out the variable's value to the output")
{
    char * fs_command = "grep Specific *.pmtm | grep PMTM_TEST_SPEC_ENV_INTERNAL | wc -l";
    
    setenv("PMTM_TEST_SPEC_ENV_INTERNAL", "1", 1);
    
    {
      PmtmWrapper pmtm("test_timing_file_");
      CHECKED_PMTM_CALL( PMTM_output_specific_runtime_variable(PMTM_DEFAULT_INSTANCE, "PMTM_TEST_SPEC_ENV_INTERNAL") );
      pmtm.finalize();
      
      if (rank == 0) {
	    FILE * so = popen(fs_command,"r");
	    int bigup;
	    if(fscanf(so,"%d",&bigup) != 1)
	      bigup = -1;
	    pclose(so);
	    REQUIRE( bigup == 1 );
      }
    }
    
    unsetenv("PMTM_TEST_SPEC_ENV_INTERNAL");
        
    MPI_Barrier(MPI_COMM_WORLD);
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

TEST_CASE( "parameter/for_each_rank", "Ouputting a parameter with for_each_rank == TRUE should print the parameter on each rank" )
{
    PmtmWrapper pmtm("test_timing_file_");

    CHECKED_PMTM_CALL( PMTM_parameter_output(PMTM_DEFAULT_INSTANCE, "RankParam", PMTM_OUTPUT_ALWAYS, PMTM_TRUE, "%d", rank) );

    pmtm.finalize();

    if (rank == 0) {
        std::vector<std::string> lines = check_header(pmtm.read_output_file());
        lines = check_overheads(lines);

        REQUIRE( lines.size() == nprocs + 2 );
        for (int idx = 0; idx < nprocs; ++idx) {
            check_param(lines.at(idx), idx, "RankParam", idx);
        }
        REQUIRE( lines.at(nprocs) == "" );
    }
        
    MPI_Barrier(MPI_COMM_WORLD);
}

TEST_CASE( "parameter/rank0_only", "Ouputting a parameter with for_each_rank == FALSE should print the parameter on rank0 only" )
{
    PmtmWrapper pmtm("test_timing_file_");

    CHECKED_PMTM_CALL( PMTM_parameter_output(PMTM_DEFAULT_INSTANCE, "RankParam", PMTM_OUTPUT_ALWAYS, PMTM_FALSE, "%d", rank) );

    pmtm.finalize();

    if (rank == 0) {
        std::vector<std::string> lines = check_header(pmtm.read_output_file());
        lines = check_overheads(lines);

        REQUIRE( lines.size() == 3 );
        check_param(lines.at(0), 0, "RankParam", 0);
        REQUIRE( lines.at(1) == "" );
    }
        
    MPI_Barrier(MPI_COMM_WORLD);
}

TEST_CASE( "parameter/output_types", "Should be able to output a variety of different parameter types using printf style notation" )
{
    PmtmWrapper pmtm("test_timing_file_");

    CHECKED_PMTM_CALL( PMTM_parameter_output(PMTM_DEFAULT_INSTANCE, "IntParam", PMTM_OUTPUT_ALWAYS, PMTM_FALSE, "%d", 1) );
    CHECKED_PMTM_CALL( PMTM_parameter_output(PMTM_DEFAULT_INSTANCE, "LngParam", PMTM_OUTPUT_ALWAYS, PMTM_FALSE, "%ld", 10000000000l) );
    CHECKED_PMTM_CALL( PMTM_parameter_output(PMTM_DEFAULT_INSTANCE, "StrParam", PMTM_OUTPUT_ALWAYS, PMTM_FALSE, "%s", "Value") );
    CHECKED_PMTM_CALL( PMTM_parameter_output(PMTM_DEFAULT_INSTANCE, "FltParam", PMTM_OUTPUT_ALWAYS, PMTM_FALSE, "%12.6f", 1.23f) );
    CHECKED_PMTM_CALL( PMTM_parameter_output(PMTM_DEFAULT_INSTANCE, "DblParam", PMTM_OUTPUT_ALWAYS, PMTM_FALSE, "%12.6f", 1.23456789) );

    pmtm.finalize();

    if (rank == 0) {
        std::vector<std::string> lines = check_header(pmtm.read_output_file());
        lines = check_overheads(lines);

        REQUIRE( lines.size() == 7 );
        check_param(lines.at(0), 0, "IntParam", 1);
        check_param(lines.at(1), 0, "LngParam", 10000000000l);
        check_param(lines.at(2), 0, "StrParam", "Value");
        check_param(lines.at(3), 0, "FltParam", 1.23f);
        check_param(lines.at(4), 0, "DblParam", 1.23456789);
        REQUIRE( lines.at(5) == "" );
    }
        
    MPI_Barrier(MPI_COMM_WORLD);
}

TEST_CASE( "parameter/output_once", "With the output once type the parameter should only be printed on the first call" )
{
    PmtmWrapper pmtm("test_timing_file_");

    CHECKED_PMTM_CALL( PMTM_parameter_output(PMTM_DEFAULT_INSTANCE, "RankParam", PMTM_OUTPUT_ONCE, PMTM_FALSE, "%d", rank) );
    CHECKED_PMTM_CALL( PMTM_parameter_output(PMTM_DEFAULT_INSTANCE, "RankParam", PMTM_OUTPUT_ONCE, PMTM_FALSE, "%d", nprocs + rank) );
    CHECKED_PMTM_CALL( PMTM_parameter_output(PMTM_DEFAULT_INSTANCE, "RankParam", PMTM_OUTPUT_ONCE, PMTM_FALSE, "%d", 2 * nprocs + rank) );

    pmtm.finalize();

    if (rank == 0) {
        std::vector<std::string> lines = check_header(pmtm.read_output_file());
        lines = check_overheads(lines);

        REQUIRE( lines.size() == 3 );
        check_param(lines.at(0), 0, "RankParam", 0);
        REQUIRE( lines.at(1) == "" );
    }
        
    MPI_Barrier(MPI_COMM_WORLD);
}

TEST_CASE( "parameter/output_on_change", "With the output on change type the parameter should only be printed if it changes between calls" )
{
    PmtmWrapper pmtm("test_timing_file_");

    CHECKED_PMTM_CALL( PMTM_parameter_output(PMTM_DEFAULT_INSTANCE, "RankParam", PMTM_OUTPUT_ON_CHANGE, PMTM_FALSE, "%d", rank) );
    CHECKED_PMTM_CALL( PMTM_parameter_output(PMTM_DEFAULT_INSTANCE, "RankParam", PMTM_OUTPUT_ON_CHANGE, PMTM_FALSE, "%d", rank) );
    CHECKED_PMTM_CALL( PMTM_parameter_output(PMTM_DEFAULT_INSTANCE, "RankParam", PMTM_OUTPUT_ON_CHANGE, PMTM_FALSE, "%d", nprocs + rank) );
    CHECKED_PMTM_CALL( PMTM_parameter_output(PMTM_DEFAULT_INSTANCE, "RankParam", PMTM_OUTPUT_ON_CHANGE, PMTM_FALSE, "%d", nprocs + rank) );
    CHECKED_PMTM_CALL( PMTM_parameter_output(PMTM_DEFAULT_INSTANCE, "RankParam", PMTM_OUTPUT_ON_CHANGE, PMTM_FALSE, "%d", 2 * nprocs + rank) );
    CHECKED_PMTM_CALL( PMTM_parameter_output(PMTM_DEFAULT_INSTANCE, "RankParam", PMTM_OUTPUT_ON_CHANGE, PMTM_FALSE, "%d", 2 * nprocs + rank) );

    pmtm.finalize();

    if (rank == 0) {
        std::vector<std::string> lines = check_header(pmtm.read_output_file());
        lines = check_overheads(lines);

        REQUIRE( lines.size() == 5 );
        check_param(lines.at(0), 0, "RankParam", 0);
        check_param(lines.at(1), 0, "RankParam2", nprocs);
        check_param(lines.at(2), 0, "RankParam3", 2 * nprocs);
        REQUIRE( lines.at(3) == "" );
    }
        
    MPI_Barrier(MPI_COMM_WORLD);
}

TEST_CASE( "parameter/output_always", "With the output always type the parameter should always be printed" )
{
    PmtmWrapper pmtm("test_timing_file_");

    CHECKED_PMTM_CALL( PMTM_parameter_output(PMTM_DEFAULT_INSTANCE, "RankParam", PMTM_OUTPUT_ALWAYS, PMTM_FALSE, "%d", rank) );
    CHECKED_PMTM_CALL( PMTM_parameter_output(PMTM_DEFAULT_INSTANCE, "RankParam", PMTM_OUTPUT_ALWAYS, PMTM_FALSE, "%d", rank) );
    CHECKED_PMTM_CALL( PMTM_parameter_output(PMTM_DEFAULT_INSTANCE, "RankParam", PMTM_OUTPUT_ALWAYS, PMTM_FALSE, "%d", nprocs + rank) );
    CHECKED_PMTM_CALL( PMTM_parameter_output(PMTM_DEFAULT_INSTANCE, "RankParam", PMTM_OUTPUT_ALWAYS, PMTM_FALSE, "%d", nprocs + rank) );
    CHECKED_PMTM_CALL( PMTM_parameter_output(PMTM_DEFAULT_INSTANCE, "RankParam", PMTM_OUTPUT_ALWAYS, PMTM_FALSE, "%d", 2 * nprocs + rank) );
    CHECKED_PMTM_CALL( PMTM_parameter_output(PMTM_DEFAULT_INSTANCE, "RankParam", PMTM_OUTPUT_ALWAYS, PMTM_FALSE, "%d", 2 * nprocs + rank) );

    pmtm.finalize();

    if (rank == 0) {
        std::vector<std::string> lines = check_header(pmtm.read_output_file());
        lines = check_overheads(lines);

        REQUIRE( lines.size() == 8 );
        check_param(lines.at(0), 0, "RankParam",  0);
        check_param(lines.at(1), 0, "RankParam2", 0);
        check_param(lines.at(2), 0, "RankParam3", nprocs);
        check_param(lines.at(3), 0, "RankParam4", nprocs);
        check_param(lines.at(4), 0, "RankParam5", 2 * nprocs);
        check_param(lines.at(5), 0, "RankParam6", 2 * nprocs);
        REQUIRE( lines.at(6) == "" );
    }
    
        
    MPI_Barrier(MPI_COMM_WORLD);
}

TEST_CASE( "finalize/timer_output", "Finalising PMTM should cause it to output all unprinted timers followed by End of File to the output file" )
{
    PmtmWrapper pmtm("test_timing_file_");
    PMTM_timer_t timer_id;
    PMTM_create_timer(PMTM_DEFAULT_GROUP,  &timer_id, "Timer1", PMTM_TIMER_NONE);

    pmtm.finalize(); // finalize to flush & close output file.

    if (rank == 0) {
        std::vector<std::string> lines = check_header(pmtm.read_output_file());
        lines = check_overheads(lines);

        REQUIRE( lines.size() == nprocs + 2 );

        for (int idx = 0; idx < nprocs; ++idx) {
            check_timer(lines.at(idx), idx, 0, "Timer1");
        }

        REQUIRE( lines.at(nprocs) == "" );
        REQUIRE( lines.at(nprocs + 1) == "End of File" );
    }
    
    MPI_Barrier(MPI_COMM_WORLD);
}

TEST_CASE( "finalize/keep_local_copy_implicit", "Finalising PMTM should keep the local copy of the file if PMTM_KEEP_LOCAL_COPY and PMTM_DELETE_LOCAL_COPY are not set")
{
    unsetenv("PMTM_KEEP_LOCAL_COPY");
    unsetenv("PMTM_DELETE_LOCAL_COPY");
    
    FileDeleter file_deleter("test_timing_file_");
      
    PmtmWrapper pmtm("test_timing_file_");
      
    pmtm.finalize();
    
    if (rank == 0) {
      int bigup = system("ls test_timing_file_*.pmtm");
      REQUIRE( bigup == 0 );
    }
    else {
      int noup = 1;
    }
      
          
    MPI_Barrier(MPI_COMM_WORLD);
}

TEST_CASE( "finalize/keep_local_copy_explicit", "Finalising PMTM should keep the local copy if PMTM_KEEP_LOCAL_COPY is set and not '', '0' or case insensitive 'FALSE' and PMTM_DELETE_LOCAL_COPY is set")
{
    setenv("PMTM_KEEP_LOCAL_COPY","ON",1);
    setenv("PMTM_DELETE_LOCAL_COPY","ON",1);
    
    FileDeleter file_deleter("test_timing_file_");
      
    PmtmWrapper pmtm("test_timing_file_");
      
    pmtm.finalize();
    
    if (rank == 0) {
      int bigup = system("ls test_timing_file_*.pmtm");
      REQUIRE( bigup == 0 );
    }
    else {
      int noup = 1;
    }
    unsetenv("PMTM_DELETE_LOCAL_COPY");
      
          
    MPI_Barrier(MPI_COMM_WORLD);
}

TEST_CASE( "finalize/delete_local_copy_explicit", "Finalising PMTM should delete the local copy if PMTM_KEEP_LOCAL_COPY is set to '', '0' or case insensitive 'FALSE'")
{
  int num_cases = 6;
  char * CASES[] = {"", "0", "FALSE", "False", "false", "FaLsE"};
  
  for (int i = 0; i < num_cases; i++)
  {
      setenv("PMTM_KEEP_LOCAL_COPY",CASES[i],1);
      
      FileDeleter file_deleter("test_timing_file_");
      
      PmtmWrapper pmtm("test_timing_file_");
	
      pmtm.finalize();
      
      if (rank == 0) {
	int bigup = system("ls test_timing_file_*.pmtm");
	REQUIRE( bigup > 0 );
      }
      else {
	int noup = 1;
      }
  }
  
  setenv("PMTM_KEEP_LOCAL_COPY","1",1);
  
      
  MPI_Barrier(MPI_COMM_WORLD);
}

TEST_CASE( "finalize/delete_local_copy_implicit", "Finalising PMTM should delete the local copy of the file if PMTM_KEEP_LOCAL_COPY is not set and PMTM_DELETE_LOCAL_COPY is set and not '', '0' or case insensitive 'FALSE'")
{
    unsetenv("PMTM_KEEP_LOCAL_COPY");
    setenv("PMTM_DELETE_LOCAL_COPY","ON",1);
    
    FileDeleter file_deleter("test_timing_file_");
      
    PmtmWrapper pmtm("test_timing_file_");
      
    pmtm.finalize();
    
    if (rank == 0) {
      int bigup = system("ls test_timing_file_*.pmtm");
      REQUIRE( bigup > 0 );
    }
    else {
      int noup = 1;
    }
    setenv("PMTM_KEEP_LOCAL_COPY","1",1);
    unsetenv("PMTM_DELETE_LOCAL_COPY");
      
          
    MPI_Barrier(MPI_COMM_WORLD);
}

TEST_CASE( "finalize/delete_local_copy_internal", "Finalising PMTM should delete the local copy of the file if PMTM_OPTION_NO_LOCAL_COPY is set to PMTM_TRUE using the set_option function")
{
    FileDeleter file_deleter("test_timing_file_");
      
    PmtmWrapper pmtm("test_timing_file_");
    
    CHECKED_PMTM_CALL( PMTM_set_option(PMTM_OPTION_NO_LOCAL_COPY, PMTM_TRUE) );
      
    pmtm.finalize();
    
    if (rank == 0) {
      int bigup = system("ls test_timing_file_*.pmtm");
      REQUIRE( bigup > 0 );

    }
    else {
      int noup = 1;
    }
      
    PMTM_set_option(PMTM_OPTION_NO_LOCAL_COPY, PMTM_FALSE);
          
    MPI_Barrier(MPI_COMM_WORLD);
}

TEST_CASE( "finalize/file_movement", "Finalising PMTM with the environment variable PMTM_DATA_STORE set should copy the pmtm output to the directory indicated by PMTM_DATA_STORE")
{
    char * test_dir = (char *) malloc(strlen(getenv("PWD")) + 20);
    strcpy(test_dir,getenv("PWD"));
    strcat(test_dir,"/TEST_FILE_MOVEMENT");
    // Set PMTM_DATA_STORE to point to a known directory
    setenv("PMTM_DATA_STORE",test_dir,1);
    char * mk_command = (char *) malloc(strlen(test_dir) + 10);
    sprintf(mk_command,"mkdir -p %s",test_dir);
    system(mk_command);
    
    PmtmWrapper pmtm("test_timing_file_");
    
    pmtm.finalize();
    
    if (rank == 0) {
	char * ls_command = (char *) malloc(strlen(test_dir) + 11);
	sprintf(ls_command,"ls %s/*.pmtm",test_dir);
	int bigup = system(ls_command);
	REQUIRE( bigup == 0 );
	
	char * rm_command = (char *) malloc(strlen(test_dir) + 8);
	sprintf(rm_command,"rm -rf %s",test_dir);
	system(rm_command);
    }
    else
    {
      int noup =1;
    }
    
    unsetenv("PMTM_DATA_STORE");
       
        
    MPI_Barrier(MPI_COMM_WORLD);
}

TEST_CASE( "finalize/file_movement_disabled", "Finalising PMTM with the internal PMTM_OPTION_NO_STORED_COPY variable set to PMTM_TRUE using the internal set_option function should not copy file to PMTM_DATA_STORE")
{
    char * test_dir = (char *) malloc(strlen(getenv("PWD")) + 22);
    strcpy(test_dir,getenv("PWD"));
    strcat(test_dir,"/TEST_FILE_MOVEMENT_2");
    // Set PMTM_DATA_STORE to point to a known directory
    setenv("PMTM_DATA_STORE",test_dir,1);
    char * mk_command = (char *) malloc(strlen(test_dir) + 10);
    sprintf(mk_command,"mkdir -p %s",test_dir);
    system(mk_command);
    
    MPI_Barrier(MPI_COMM_WORLD);
    
    PmtmWrapper pmtm("test_timing_file_");
    
    CHECKED_PMTM_CALL( PMTM_set_option(PMTM_OPTION_NO_STORED_COPY, PMTM_TRUE) );
    
    pmtm.finalize();
    
    if (rank == 0) {
	char * ls_command = (char *) malloc(strlen(test_dir) + 11);
	sprintf(ls_command,"ls %s/*.pmtm",test_dir);
	int bigup = system(ls_command);
	REQUIRE( bigup > 0 );
	
	char * rm_command = (char *) malloc(strlen(test_dir) + 8);
	sprintf(rm_command,"rm -rf %s",test_dir);
	system(rm_command);
    }
    else
    {
      int noup =1;
    }
    
    unsetenv("PMTM_DATA_STORE");
      
    PMTM_set_option(PMTM_OPTION_NO_STORED_COPY, PMTM_FALSE);
        
    MPI_Barrier(MPI_COMM_WORLD);
}


TEST_CASE( "options/from_pmtmrc_file", "Test whether putting a option pair in either VARIABLE VALUE or VARIABLE=VALUE form in .pmtmrc file results in change to option")
{
  
    FileDeleter file_deleter("test_timing_file_");
  
    char * test_dir = (char *) malloc(strlen(getenv("PWD")) + 22);
    strcpy(test_dir,getenv("PWD"));
    strcat(test_dir,"/TEST_FILE_MOVEMENT_3");
    char * cf_command = (char *) malloc(strlen(test_dir) + 36);
    sprintf(cf_command,"echo \"PMTM_DATA_STORE %s\" > .pmtmrc",test_dir);
    system(cf_command);
    char * cf2_command = " echo \"PMTM_OPTION_NO_LOCAL_COPY=1\" >> .pmtmrc";
    system(cf2_command);
    
    char * mk_command = (char *) malloc(strlen(test_dir) + 10);
    sprintf(mk_command,"mkdir -p %s",test_dir);
    system(mk_command);
    char * rm_command = "rm .pmtmrc";
    
    
    PmtmWrapper pmtm("test_timing_file_");
    
    pmtm.finalize();
    
    if (rank == 0) {
	char * ls_command = (char *) malloc(strlen(test_dir) + 11);
	sprintf(ls_command,"ls %s/*.pmtm",test_dir);
	int bigup = system(ls_command);
	REQUIRE( bigup == 0 );
	
	char * rm_command = (char *) malloc(strlen(test_dir) + 8);
	sprintf(rm_command,"rm -rf %s",test_dir);
	system(rm_command);
	
	bigup = system("ls test_timing_file_*.pmtm");
	REQUIRE( bigup > 0 );
    }
    else
    {
      int noup =1;
    }
    
    if(rank == 0) {  system(rm_command); }
   
    PMTM_set_option(PMTM_OPTION_NO_LOCAL_COPY, PMTM_FALSE);
    PMTM_set_option(PMTM_OPTION_NO_STORED_COPY, PMTM_TRUE);
    
    MPI_Barrier(MPI_COMM_WORLD);
    
}

#include <omp.h>

#ifdef _OPENMP
TEST_CASE( "threads/parallel_timing", "Timing a one second wait should return a time close to one second from each thread" )
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
#endif
