/* 
 * File:   tests_utils.hpp
 * Author: Hollcombe (Tessella plc)
 *
 */

#ifndef _TESTS_UTILS_HPP
#define	_TESTS_UTILS_HPP

#ifndef SERIAL
#  include "mpi.h"
#endif

#include <vector>
#include <string>
#include <sstream>

#include <stdio.h>
#include <unistd.h>

#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

#include "pmtm.h"

#define CHECKED_PMTM_CALL( func ) \
    { PMTM_error_t err_code = func; REQUIRE( err_code == PMTM_SUCCESS ); }

int rank   = 0;
int nprocs = 1;

#define BUFFER_SZ 32768

/**
 * Initialise MPI if required and launch the tests.
 *
 * @param argc [IN] The number of command line arguments.
 * @param argv [IN] The command line arguments.
 */
int run_tests(int argc, char** argv)
{
    Catch::Config config;
    
    unsetenv("PMTM_DATA_STORE");
    setenv("PMTM_KEEP_LOCAL_COPY","1",1);

#ifndef SERIAL
    int status = MPI_Init(&argc, &argv);
    if (status != MPI_SUCCESS) return status;
    status = MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if (status != MPI_SUCCESS) return status;
    status = MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    if (status != MPI_SUCCESS) return status;
    std::stringstream outfile_ss;
    outfile_ss << "tests_rank_" << rank << ".out";
    config.setFilename(outfile_ss.str());
#endif
    int result = Catch::Main(argc, argv, config);
#ifndef SERIAL
    status = MPI_Finalize();
    if (status != MPI_SUCCESS) return status;
#endif
    return result;
}

/**
 * A class which which handles the cleaning up of the files created by PMTM
 * after each test.
 */
class FileDeleter
{
public:
    /**
     * Construct the file deleter with the name of the file to delete.
     *
     * @param file_name The base name of the file to delete.
     * @param file_num The number to append to the base file name.
     */
    FileDeleter(const std::string& file_name, int file_num = 0)
    : m_file_name()
    {
        std::stringstream ss;
        ss << file_name << file_num << ".pmtm";
        m_file_name = ss.str();
    }
    /**
     * Destruct the file deleter, which also triggers the deletion of the file.
     */
    ~FileDeleter()
    {
        if (rank == 0 && access(m_file_name.c_str(), F_OK) == 0) {
            int status = remove(m_file_name.c_str());
            REQUIRE( status == 0 );
        }
    }
private:
    std::string m_file_name;
};

/**
 * An RAII style wrapper for PMTM which deals with the initialisation in its
 * constructor and tidying up in its destructor.
 */
class PmtmWrapper
{
public:
    /**
     * Construct a PMTM wrapper with the given output file.
     *
     * @param file_name The name of the PMTM output file to use.
     * @param file_num The number to append to the base file name.
     */
    PmtmWrapper(std::string file_name, int file_num = 0)
    : m_file_name(file_name)
    , m_file_deleter()
    , m_finalized(false)
    {
    
        PMTM_BOOL is_initialised = PMTM_initialised();
        REQUIRE( is_initialised == PMTM_FALSE );

        int rank;
#ifndef SERIAL
        int status = MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        REQUIRE( status == MPI_SUCCESS );
#else
        rank = 0;
#endif

        if (rank == 0 && file_name != "" && file_name != "-") {
            std::stringstream exp_file;
            exp_file << file_name << file_num << ".pmtm";
            m_file_name = exp_file.str();

            if(access(m_file_name.c_str(), F_OK) == 0)
                FAIL( "Expected file already exists" );

            m_file_deleter = std::auto_ptr<FileDeleter>(new FileDeleter(file_name, file_num));
        }

        PMTM_error_t err_code = PMTM_init(file_name.c_str(), "Test App");
        REQUIRE( err_code == PMTM_SUCCESS );

#ifndef SERIAL
        status = MPI_Barrier(MPI_COMM_WORLD);
        REQUIRE( status == MPI_SUCCESS );
#endif

        if (rank == 0 && file_name != "" && file_name != "-") {
            REQUIRE( access(m_file_name.c_str(), F_OK) == 0 );
        }
    }
    /**
     * Destruct the PMTM wrapper, finalizing PMTM if not done so already. This
     * will also trigger the destructor of the associated file deleter, deleting
     * the PMTM output file.
     */
    ~PmtmWrapper()
    {
        finalize();
    }
    /**
     * Finalize PMTM, causing the output file to be flushed and closed.
     */
    void finalize()
    {
        if (!m_finalized) {
            CHECKED_PMTM_CALL( PMTM_finalize() );
            m_finalized = true;
#ifndef SERIAL
            MPI_Barrier(MPI_COMM_WORLD);
#endif
        }
    }
    /**
     * Open the PMTM output file and read it in, putting the output lines into
     * a vector of strings.
     *
     * @returns The output file split into lines.
     */
    std::vector<std::string> read_output_file()
    {
        char buffer[BUFFER_SZ];
        std::vector<std::string> lines;
        std::ifstream ifs(m_file_name.c_str());
        while (ifs) {
            ifs.getline(buffer, BUFFER_SZ);
            if (ifs) lines.push_back(std::string(buffer));
        }
        return lines;
    }
private:
    std::string m_file_name;
    std::auto_ptr<FileDeleter> m_file_deleter;
    bool m_finalized;
};

/**
 * Read in the lines of the output file and check that the first few lines
 * match the expected header.
 *
 * @param lines The lines to check.
 * @returns The lines minus the header.
 */
std::vector<std::string> check_header(
        std::vector<std::string> lines)
{
    std::vector<std::string> return_vec;
    std::vector<std::string>::size_type line_idx = 0;
    bool in_header = true;
    
    for (std::vector<std::string>::const_iterator iter = lines.begin(); iter != lines.end(); ++iter) {
        std::string line = iter->substr(0, iter->find(',', 0));
        if (in_header) {
            switch (line_idx) {
                case 0:  REQUIRE( line == "Performance Modelling Timing File" ); break;
                case 1:  REQUIRE( line == "" ); break;
                case 2:  REQUIRE( line == "PMTM Version" ); break;
                case 3:  REQUIRE( line == "Application" ); break;
                case 4:  REQUIRE( line == "Date" ); break;
                case 5:  REQUIRE( line == "Time" ); break;
                case 6:  REQUIRE( line == "Run ID" ); break;
                case 7:  REQUIRE( line == "NProcs" ); break;
                case 8:  REQUIRE( line == "Max OpenMP Threads"); break;
                case 9:  REQUIRE( line == "Machine" ); break;
                case 10: REQUIRE( line == "Processor"); break;
                case 11: REQUIRE( line == "OS" ); break;
                case 12: REQUIRE( line == "Compiler" ); break;
                case 13: REQUIRE( line == "MPI" ); break;
                default:
                    if ( line == "#Type" ) {
                        in_header = false;
                    } else if ( line == "Specific" ) {
		        in_header = true;
		    } else {
                        REQUIRE( line == "Environ" );
                    }
                    break;
            }
        } else {
            return_vec.push_back(*iter);
        }
        ++line_idx;
    }

    return return_vec;
}

/**
 * Read in the lines of the output file and check that the first few lines
 * match the expected overhead lines.
 *
 * @param lines The lines to check.
 * @returns The lines minus the overhead.
 */
std::vector<std::string> check_overheads(
        std::vector<std::string> lines)
{
    std::vector<std::string> return_vec;
    std::vector<std::string>::size_type line_idx = 0;

    for (std::vector<std::string>::const_iterator iter = lines.begin(); iter != lines.end(); ++iter) {
        std::string line = iter->substr(0, iter->find('=', 0));
        switch (line_idx) {
            case 0: REQUIRE( line == "Overhead, (, 0, ), start-stop, " ); break;
            case 1: REQUIRE( line == "Overhead, (, 0, ), pause-continue, " ); break;
            default: return_vec.push_back(*iter);
        }
        ++line_idx;
    }

    return return_vec;
}

/**
 * Split up a line using the given deliminator and return a vector containing
 * the tokens.
 *
 * @param line  The line to tokenize.
 * @param delim The deliminator to use.
 * @returns A vector of tokens.
 */
std::vector<std::string> tokenize(
        const std::string& line,
        const char delim = ',')
{
    std::vector<std::string> tokens;

    std::vector<std::string>::size_type pos = 0;
    while (pos != line.npos) {
        if ( line.at(pos) == ',' ) ++pos;
        if ( line.at(pos) == ' ' ) ++pos;
        std::vector<std::string>::size_type end_pos = line.find(delim, pos);
        tokens.push_back(line.substr(pos, end_pos - pos));
        pos = end_pos;
    }

    return tokens;
}

/**
 * Check that the line matches the expected timer output format for a timer of
 * the given name and type.
 *
 * @param line       The line to check.
 * @param type       The type of the timer, e.g. "Rank Maximum".
 * @param timer_name The name of the timer.
 * @param count      The number of times the timer should have been called
 *                   (default = 0).
 * @param pauses     The number of times the timer should have been paused
 *                   (default = 0).
 * @param seconds    The number of seconds the timer should have measured,
 *                   within a certain tollerance (defaults to -1 which means to
 *                   ignore this param).
 */
void check_timer(
        const std::string& line,
        const std::string& type,
        const std::string& timer_name,
        int count = 0,
        int pauses = 0,
        int seconds = -1)
{
    std::stringstream count_ss;
    count_ss << count;
    std::stringstream pause_ss;
    pause_ss << pauses;

    std::vector<std::string> tokens = tokenize(line);

    REQUIRE( tokens.at(0) == "Timer" );
    REQUIRE( tokens.at(2) == type);
    REQUIRE( tokens.at(4) == timer_name );
    if (seconds > 0) {
        std::stringstream seconds_ss(tokens.at(6));
        double time;
        seconds_ss >> time;
        REQUIRE( fabs(time - seconds) < 2E-2 );
    }
    REQUIRE( tokens.at(11) == count_ss.str() );
    REQUIRE( tokens.at(13) == pause_ss.str() );
}

/**
 * Check that the line matches the expected timer output format for a timer of
 * the given name and rank.
 *
 * @param line       The line to check.
 * @param rank       The rank of the timer.
 * @param timer_name The name of the timer.
 * @param count      The number of times the timer should have been called
 *                   (default = 0).
 * @param pauses     The number of times the timer should have been paused
 *                   (default = 0).
 * @param seconds    The number of seconds the timer should have measured,
 *                   within a certain tollerance (defaults to -1 which means to
 *                   ignore this param).
 */
void check_timer(
        const std::string& line,
        int rank,
        int thread,
        const std::string& timer_name,
        int count = 0,
        int pauses = 0,
        int seconds = -1)
{
    std::stringstream ss;
    ss << rank << "." << thread;
    check_timer(line, ss.str(), timer_name, count, pauses, seconds);
}

/**
 * Check that the given line contains the expected parameter output.
 *
 * @param line       The line to check.
 * @param rank       The rank on which the parameter was printed.
 * @param param_name The name of the parameter.
 * @param value      The value of the parameter.
 */
template <typename T>
void check_param(
        const std::string& line,
        int rank,
        const std::string& param_name,
        T value)
{
    std::stringstream ss;
    ss << "Parameter, : (, " << rank << ", ), " << param_name << ", =, " << value;
    REQUIRE( line == ss.str() );
}

/**
 * Check that the given line contains the expected parameter output.
 *
 * @param line       The line to check.
 * @param rank       The rank on which the parameter was printed.
 * @param param_name The name of the parameter.
 * @param value      The value of the parameter.
 */
template <>
void check_param(
        const std::string& line,
        int rank,
        const std::string& param_name,
        float value)
{
    char buffer[32];
    sprintf(buffer, "%12.6f", value);
    check_param(line, rank, param_name, buffer);
}

/**
 * Check that the given line contains the expected parameter output.
 *
 * @param line       The line to check.
 * @param rank       The rank on which the parameter was printed.
 * @param param_name The name of the parameter.
 * @param value      The value of the parameter.
 */
template <>
void check_param(
        const std::string& line,
        int rank,
        const std::string& param_name,
        double value)
{
    char buffer[32];
    sprintf(buffer, "%12.6f", value);
    check_param(line, rank, param_name, buffer);
}

#endif	/* _TESTS_UTILS_HPP */

