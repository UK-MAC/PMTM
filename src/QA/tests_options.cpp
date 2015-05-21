/* 
 * File:   tests_options.cpp
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
 * @ingroup tests_opts
 * 
 * Tests that setting \c PMTM_OPTION_OUTPUT_ENV to \c PMTM_FALSE using the \ref PMTM_set_option function stops the values of the Environment values being written to the output file.
 * 
 */
TEST_CASE( "tests_options.cpp/output_env", "Turning off the output environment options should stop the environment being output to the PMTM output file" )
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

/**
 * @ingroup tests_opts
 * 
 * Tests that putting an environment variable name in a .pmtmrc file outputs the value of the environment variable to the output file during the call to \ref PMTM_init. Also, ensures that multiple calls to output the same variable only cause it to be written out once.
 * 
 */
TEST_CASE( "tests_options.cpp/get_specific_runtime_variables", "Test whether putting a specific Environment variable name in ${PWD}/.pmtmrc prints it to the output, but only once")
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

/**
 * @ingroup tests_opts
 * 
 * Tests that calling \ref PMTM_output_specific_runtime_variable within a code causes the value of the specified environment variable to be written to the output file.
 * 
 */
TEST_CASE( "tests_options.cpp/get_specific_runtime_variables_internal", "Test whether PMTM_output_specific_runtime_variable prints out the variable's value to the output")
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

/**
 * @ingroup tests_opts
 * 
 * Tests that putting a valid option pair in either \c VARIABLE \c VALUE or \c VARIABLE=VALUE form in a .pmtmrc file causes \ref PMTM_set_option to be called and the value of the relevant option changed.
 * 
 */
TEST_CASE( "tests_options.cpp/from_pmtmrc_file", "Test whether putting a option pair in either VARIABLE VALUE or VARIABLE=VALUE form in .pmtmrc file results in change to option")
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

