/* 
 * File:   tests_finalize.cpp
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
 * @ingroup tests_final
 * 
 * Tests that \ref PMTM_finalize completes properly and that a new instance of PMTM can be created afterwards.
 */
TEST_CASE( "tests_finalize.cpp/reinitialise", "Finalising PMTM leaves the library in a state where it can be initialised again without error" )
{
    // Each wrapper instance initialises on creation and then finalizes on
    // destruction.
    PmtmWrapper("test_timing_file_");
    PmtmWrapper("test_timing_file_");
        
    MPI_Barrier(MPI_COMM_WORLD);
}

/**
 * @ingroup tests_final
 * 
 * Tests that \ref PMTM_finalize correctly prints timers to the output file and then writes "End of File"
 */
TEST_CASE( "tests_finalize.cpp/timer_output", "Finalising PMTM should cause it to output all unprinted timers followed by End of File to the output file" )
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

/**
 * @ingroup tests_final
 * 
 * Tests that \ref PMTM_finalize keeps a local copy of the output file by default with no environment variable being set.
 */
TEST_CASE( "tests_finalize.cpp/keep_local_copy_implicit", "Finalising PMTM should keep the local copy of the file if PMTM_KEEP_LOCAL_COPY and PMTM_DELETE_LOCAL_COPY are not set")
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

/**
 * @ingroup tests_final
 * 
 * Tests that \ref PMTM_finalize correctly lets \c PMTM_KEEP_LOCAL_COPY override \c PMTM_DELETE_LOCAL_COPY, provided that it is not set to a false value.
 */
TEST_CASE( "tests_finalize.cpp/keep_local_copy_explicit", "Finalising PMTM should keep the local copy if PMTM_KEEP_LOCAL_COPY is set and not '', '0' or case insensitive 'FALSE' and PMTM_DELETE_LOCAL_COPY is set")
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

/**
 * @ingroup tests_final
 * 
 * Tests that \ref PMTM_finalize deletes the local output file if \c PMTM_KEEP_LOCAL_COPY is set to a false value.
 */
TEST_CASE( "tests_finalize.cpp/delete_local_copy_explicit", "Finalising PMTM should delete the local copy if PMTM_KEEP_LOCAL_COPY is set to '', '0' or case insensitive 'FALSE'")
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

/**
 * @ingroup tests_final
 * 
 * Tests that \ref PMTM_finalize will delete the local output file if \c PMTM_DELETE_LOCAL_COPY is set to a true value and \c PMTM_KEEP_LOCAL_COPY is not set.
 */
TEST_CASE( "tests_finalize.cpp/delete_local_copy_implicit", "Finalising PMTM should delete the local copy of the file if PMTM_KEEP_LOCAL_COPY is not set and PMTM_DELETE_LOCAL_COPY is set and not '', '0' or case insensitive 'FALSE'")
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

/**
 * @ingroup tests_final
 * 
 * Tests using the \ref PMTM_set_option command to set the \c PMTM_OPTION_NO_LOCAL_COPY to \c PMTM_TRUE and that \ref PMTM_finalize then deletes the local copy of the output file.
 */
TEST_CASE( "tests_finalize.cpp/delete_local_copy_internal", "Finalising PMTM should delete the local copy of the file if PMTM_OPTION_NO_LOCAL_COPY is set to PMTM_TRUE using the set_option function")
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

/**
 * @ingroup tests_final
 * 
 * Tests \ref PMTM_finalize correctly creates a copy of the output file in the location set by \c PMTM_DATA_STORE.
 * 
 */
TEST_CASE( "tests_finalize.cpp/file_movement", "Finalising PMTM with the environment variable PMTM_DATA_STORE set should copy the pmtm output to the directory indicated by PMTM_DATA_STORE")
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

/**
 * @ingroup tests_final
 * 
 * Tests that setting \c PMTM_OPTION_NO_STORED_COPY to \c PMTM_TRUE using the \ref PMTM_set_option function causes \ref PMTM_finalize to not create a copy of the output file in the location identified in \c PMTM_DATA_STORE.
 * 
 */
TEST_CASE( "tests_finalize.cpp/file_movement_disabled", "Finalising PMTM with the internal PMTM_OPTION_NO_STORED_COPY variable set to PMTM_TRUE using the internal set_option function should not copy file to PMTM_DATA_STORE")
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

