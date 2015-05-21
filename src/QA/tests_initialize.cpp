/* 
 * File:   tests_initialize.cpp
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
 * @ingroup tests_init
 * 
 * Tests that \ref PMTM_init creates a valid file, when no file currently exists.
 * 
 */
TEST_CASE( "tests_initialize.cpp/valid_file_name", "Initialising PMTM with a valid noexistant file name should create that file with 0.pmtm suffix" )
{
    PmtmWrapper("test_timing_file_");
        
    MPI_Barrier(MPI_COMM_WORLD);
}

/**
 * @ingroup tests_init
 * 
 * Tests that \ref PMTM_init throws an error when an invalid filename is given to it. In this the filename contains a directory
 * 
 */
TEST_CASE( "tests_initialize.cpp/invalid_file_name", "Initialising PMTM with an invalid file name should cause initialisation to fail" )
{
    PMTM_error_t err_code = PMTM_init("bad_dir/test_timing_file_", "Test App");
    REQUIRE( err_code == PMTM_ERROR_CANNOT_CREATE_FILE );
    PMTM_finalize();
        
    MPI_Barrier(MPI_COMM_WORLD);
}

/**
 * @ingroup tests_init
 * 
 * Tests that \ref PMTM_init correctly writes to stdout if a filename of "-" is given.
 * 
 */
TEST_CASE( "tests_initialize.cpp/stdout", "Initialising PMTM with a file name of - should cause it to write to stdout" )
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

/**
 * @ingroup tests_init
 * 
 * Tests that giving \ref PMTM_init no filename creates no output file.
 * 
 */
TEST_CASE( "tests_initialize.cpp/no_output", "Initialising PMTM with an empty file name should cause it to not create an output file" )
{
    PmtmWrapper("");
        
    MPI_Barrier(MPI_COMM_WORLD);
}

/**
 * @ingroup tests_init
 * 
 * Tests that \ref PMTM_init creates and writes a valid header to the output file.
 * 
 */
TEST_CASE( "tests_initialize.cpp/file_header", "Initialising PMTM should cause it to write a valid PMTM file header" )
{
    PmtmWrapper pmtm("test_timing_file_");
    pmtm.finalize(); // finalize to flush & close output file.

    check_header(pmtm.read_output_file());
        
    MPI_Barrier(MPI_COMM_WORLD);
}

/**
 * @ingroup tests_init
 * 
 * Tests that \ref PMTM_init calculates the timer overhead and writes the findings to the output file.
 * 
 */
TEST_CASE( "tests_initialize.cpp/overheads", "Initialising PMTM should cause it to output timer overhead information into the output file" )
{
    PmtmWrapper pmtm("test_timing_file_");
    pmtm.finalize(); // finalize to flush & close output file.

    std::vector<std::string> lines = check_header(pmtm.read_output_file());
    check_overheads(lines);
        
    MPI_Barrier(MPI_COMM_WORLD);

}

/**
 * @ingroup tests_init
 * 
 * Tests that \ref PMTM_init creates a new file with an incremented suffix if a existing file has the same prefix as that given.
 * 
 */
TEST_CASE( "tests_initialize.cpp/existing_file", "Initialising PMTM with a valid but existing file name should create that file with an incremented suffix" )
{
    std::string create_file = "./test_timing_file_0.pmtm";

    FileDeleter file_deleter("test_timing_file_");

    FILE * file = fopen(create_file.c_str(), "w");
    if (file == NULL) FAIL( "Failed to create test file" );
    if (fclose(file) != 0) FAIL( "Failed to close test file" );

    PmtmWrapper pmtm("test_timing_file_", 1);
        
    MPI_Barrier(MPI_COMM_WORLD);
}

