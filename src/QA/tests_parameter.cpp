/* 
 * File:   tests_parameter.cpp
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
 * @ingroup tests_param
 * 
 * Tests that setting \c for_each_rank to \c TRUE in \ref PMTM_parameter_output causes the parameter value to be written out for each rank.
 * 
 */
TEST_CASE( "tests_parameter.cpp/for_each_rank", "Ouputting a parameter with for_each_rank == TRUE should print the parameter on each rank" )
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

/**
 * @ingroup tests_param
 * 
 * Tests that setting \c for_each_rank to \c FALSE in \ref PMTM_parameter_output causes the parameter value to be written out on rank0 only.
 * 
 */
TEST_CASE( "tests_parameter.cpp/rank0_only", "Ouputting a parameter with for_each_rank == FALSE should print the parameter on rank0 only" )
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

/**
 * @ingroup tests_param
 * 
 * Tests \ref PMTM_parameter_output can handle writting out ints, longs, strings, float and doubles (using C format).
 * 
 */
TEST_CASE( "tests_parameter.cpp/output_types", "Should be able to output a variety of different parameter types using printf style notation" )
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

/**
 * @ingroup tests_param
 * 
 * Tests that using \c PMTM_OUTPUT_ONCE in \ref PMTM_parameter_output only outputs the parameter value the first time it is called.
 * 
 */
TEST_CASE( "tests_parameter.cpp/output_once", "With the output once type the parameter should only be printed on the first call" )
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

/**
 * @ingroup tests_param
 * 
 * Tests that using \c PMTM_OUTPUT_ON_CHANGE in \ref PMTM_parameter_output only outputs the parameter value when it has changed since the last call.
 * 
 */
TEST_CASE( "tests_parameter.cpp/output_on_change", "With the output on change type the parameter should only be printed if it changes between calls" )
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

/**
 * @ingroup tests_param
 * 
 * Tests that using \c PMTM_OUTPUT_ALWAYS in \ref PMTM_parameter_output outputs the parameter value everytime it is called.
 * 
 */
TEST_CASE( "tests_parameter.cpp/output_always", "With the output always type the parameter should always be printed" )
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

