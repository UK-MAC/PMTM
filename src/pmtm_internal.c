/**
 * @file   pmtm_internal.c
 * @author AWE Plc.
 *
 * This file contains the inner workings of PMTM. It is not MPI aware and will
 * run in both serial and MPI modes but the code is rank aware as the PMTM
 * instance will be constructed with a given rank (this should always be rank 0
 * when in serial mode).
 */

#include "versions.h"
#include "pmtm.h"
#include "pmtm_internal.h"
#include "pmtm_defines.h"

#include "system_pickup/OS.h"
#include "system_pickup/Processor.h"
#include "system_pickup/Compiler.h"
#include "system_pickup/ext_MPI.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <float.h>
#include <limits.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef _OPENMP
#include <omp.h>
#endif

#ifdef	__cplusplus
extern "C" {
#endif

const PMTM_instance_t    PMTM_DEFAULT_INSTANCE = INTERNAL__DEFAULT_INSTANCE;
const PMTM_timer_group_t PMTM_DEFAULT_GROUP    = INTERNAL__DEFAULT_GROUP;
const PMTM_timer_t       PMTM_NULL_TIMER       = INTERNAL__NULL_TIMER;

const PMTM_timer_type_t PMTM_TIMER_NONE = INTERNAL__TIMER_NONE;
const PMTM_timer_type_t PMTM_TIMER_MAX  = INTERNAL__TIMER_MAX;
const PMTM_timer_type_t PMTM_TIMER_MIN  = INTERNAL__TIMER_MIN;
const PMTM_timer_type_t PMTM_TIMER_AVG  = INTERNAL__TIMER_AVG;
const PMTM_timer_type_t PMTM_TIMER_ALL  = INTERNAL__TIMER_MAX | INTERNAL__TIMER_MIN | INTERNAL__TIMER_AVG;
const PMTM_timer_type_t PMTM_TIMER_MMA  = INTERNAL__TIMER_MMA | INTERNAL__TIMER_MAX | INTERNAL__TIMER_MIN | INTERNAL__TIMER_AVG;
const PMTM_timer_type_t PMTM_TIMER_INT  = INTERNAL__TIMER_INT;
const PMTM_timer_type_t PMTM_TIMER_AVO  = INTERNAL__TIMER_AVO;


const PMTM_output_type_t PMTM_OUTPUT_ALL_RANKS = INTERNAL__OUTPUT_ALL_RANKS;
const PMTM_output_type_t PMTM_OUTPUT_ALWAYS    = INTERNAL__OUTPUT_ALWAYS;
const PMTM_output_type_t PMTM_OUTPUT_ON_CHANGE = INTERNAL__OUTPUT_ON_CHANGE;
const PMTM_output_type_t PMTM_OUTPUT_ONCE      = INTERNAL__OUTPUT_ONCE;

const int PMTM_NO_MAX       = INTERNAL__NO_MAX;
const int PMTM_DEFAULT_FREQ = 1;
const int PMTM_DEFAULT_MAX  = INTERNAL__NO_MAX;

//const PMTM_BOOL PMTM_TRUE  = INTERNAL__TRUE;
//const PMTM_BOOL PMTM_FALSE = INTERNAL__FALSE;

// const PMTM_option_t PMTM_OPTION_OUTPUT_ENV = INTERNAL__OPTION_OUTPUT_ENV;
// const PMTM_option_t PMTM_OPTION_NO_LOCAL_COPY = INTERNAL__OPTION_NO_LOCAL_COPY;
// const PMTM_option_t PMTM_OPTION_NO_STORED_COPY = INTERNAL__OPTION_NO_STORED_COPY;

size_t instance_count = 0;
size_t group_count    = 0;
size_t timer_count    = 0;

//struct PMTM_instance       * instance_array = NULL;
//struct PMTM_timer_group    * group_array    = NULL;
//struct PMTM_timer       * timer_array    = NULL;

// Making these three containers lists allows the data items to stay in a fixed
// location after creation and as such there is no need to lock the structures other
// than during the addition of extra objects. Modification to the contained objects
// does not require locking providing the calling process can guarantee two threads
// will not try to update the same object simultaneously.
// 
// The only problem is potentially for anything accessing the group_ids and timer_ids
// arrays in the instance and timer group objects which can move when new entries are
// added. However, the only thing that does that beyond creation is the print routine,
// which I think only happens when the instance is shutting down anyway, I hope...
//
// Linked lists aren't very efficient for searches typically. That does happen for the
// instance and group lists as their IDs are returned are integer place holders. The
// expectation is that there is unlikely to be many of them, in many cases may be only
// one of each or single figures worth. Also, they only really get accessed during
// timer creation and not at all after until printing and destruction.
//
// There will be more timers, but their ID is the actual object pointer so no lookups
// are needed. A side benefit for the timers of the linked list is that each thread
// gets to allocate its own timers in memory. This means each will keep its timers in
// local memory if possible where as an array or bucket scheme would be more likely
// to position things remotely and likely lead to some false sharing. If a linked
// list becomes wasteful of space, then some kind of chunking scheme with thread
// specific pools is probably best. However, there is unlikely to be many more than
// a few hundred timers in most cases which should not create a problem.

struct PMTM_instance       * instance_head = NULL;
struct PMTM_instance       ** instance_tail = &instance_head;
struct PMTM_timer_group    * group_head = NULL;
struct PMTM_timer_group    ** group_tail = &group_head;
struct PMTM_timer          * timer_head = NULL;
struct PMTM_timer          ** timer_tail = &timer_head;

const char ** flag_array = NULL;
uint flag_array_sz       = 0;
PMTM_BOOL output_env     = PMTM_TRUE;
PMTM_BOOL no_local_copy  = PMTM_FALSE;
PMTM_BOOL no_stored_copy = PMTM_FALSE;

char * pmtm_file_store = NULL; 

#define RETURN_ON_ERR(exp) { PMTM_error_t err_code = (exp); if (err_code) return err_code; }

/**
 * Print a warning message to stderr.
 *
 * @param message [IN] The message to print.
 */
void pmtm_warn(const char * message, ...)
{
    const int buffer_sz = 200;
    char buffer[buffer_sz];

    va_list list;
    va_start(list, message);
    vsnprintf(buffer, buffer_sz, message, list);
    buffer[buffer_sz - 1] = '\0';
    va_end(list);

    struct PMTM_instance * instance = get_instance(INTERNAL__DEFAULT_INSTANCE);
    if (instance == NULL) {
        return;
    } else if (instance->rank == IO_RANK) {
        fputs("PMTM Error: ", stderr);
        fputs(buffer, stderr);
        fputs("!\n", stderr);
        fflush(stderr);
    }
}


/**
 * Set a library option.
 *
 * @param option [IN] The option to set.
 * @param value  [IN] The value to set for the option.
 */
PMTM_error_t set_option(PMTM_option_t option, PMTM_BOOL value)
{
    switch (option) {
        case PMTM_OPTION_OUTPUT_ENV:
            output_env = value;
            break;
	case PMTM_OPTION_NO_LOCAL_COPY:
	    no_local_copy = value;
	    break;
	case PMTM_OPTION_NO_STORED_COPY:
	    no_stored_copy = value;
	    break;
        default:
            return PMTM_ERROR_UNKNOWN_OPTION;
    }
    return PMTM_SUCCESS;
}

/**
 * Get a library option.
 *
 * @param option [IN] The option to set.
 * @param value  [IN] The value to set for the option.
 * 
 * @TODO Actually create working code
 */
/*PMTM_error_t get_option(PMTM_option_t option)
{
    switch (option) {
        case PMTM_OPTION_OUTPUT_ENV:
            break;
	case PMTM_OPTION_NO_LOCAL_COPY:
	    break;
	case PMTM_OPTION_NO_STORED_COPY:
	    break;
        default:
            return PMTM_ERROR_UNKNOWN_OPTION;
    }
    return PMTM_SUCCESS;
}*/

/**
 * Set PMTM_DATA_STORE_INTERNAL.
 *
 * @TODO Create methodL
 */
/*PMTM_error_t set_pmtm_data_store(char * store)
{
  
}*/

/**
 * Allocated the required memory and copy the data from source into *dest. This
 * function requires that source is a valid null terminated string, and
 * guarentees that *dest will also be null terminated.
 *
 * Note: the memory allocated in this copy will need to be manually freed
 * when the copy is done with.
 *
 * @param dest   [OUT] The address of the char buffer to which we are writting.
 * @param source [IN]  The string which we are copying.
 */
void copy_string(char ** dest, const char * source)
{
    size_t str_len = strlen(source);
    *dest = (char *) malloc((str_len + 1) * sizeof(char)); // Andy - despite very thorough malloc checks, here is an unchecked one!
    strncpy(*dest, source, str_len);
    (*dest)[str_len] = '\0';
}

/**
 * Store the given array of compiler flags which can be used when writing the
 * PMTM file headers
 *
 * @param flags     [IN] The flags to store.
 * @param num_flags [IN] The number of flags to store.
 */
void log_flags(const char ** flags, uint num_flags)
{
    uint idx;

    /* Clear any existing flags */
    for (idx = 0; idx < flag_array_sz; ++idx) {
        free((void * ) flag_array[idx]);
    }
    free(flag_array);
    flag_array_sz = 0;

    flag_array = (const char **) malloc(sizeof(char *) * num_flags);
    for (idx = 0; idx < num_flags; ++idx) {
        flag_array[idx] = strdup(flags[idx]);
    }
    flag_array_sz = num_flags;
}


/**
 * Construct an instanct struct, allocating the memory required for it's fields
 * and populating the fields with the given data.
 *
 * @param instance  [IN] The instance we are constructing, the memory for which
 *                       should already be allocated.
 * @param file_name [IN] The name of the PMTM to create.
 * @param app_name  [IN] The name of the application which will be logged in the
 *                       PMTM file.
 * @param rank      [IN] The MPI rank on which this instance is running, this is
 *                       always 0 if compiled without MPI support.
 * @param nranks    [IN] The number of MPI ranks in the communicator, this is
 *                       always 1 if compiled without MPI support.
 * @returns PMTM_SUCCESS if successful, or one of PMTM_ERROR_* codes if not.
 */
PMTM_error_t construct_instance(
        struct PMTM_instance * instance,
        const char * file_name,
        const char * app_name,
        int rank,
        int nranks)
{
    if (instance->initialised) {
        return PMTM_ERROR_OVERWRITING_INSTANCE;
    }

    instance->initialised = 1;

    instance->file_name = NULL;
    instance->fid = NULL;
    instance->nranks = nranks;
    instance->rank = rank;
    instance->num_groups = 0;
    instance->group_ids = NULL;
    instance->num_parameters = 0;
    instance->parameters = NULL;

    copy_string(&instance->application_name, app_name);
    check_for_commas(instance->application_name);

    PMTM_error_t err_code = set_file(instance, file_name);
    if (err_code != 0) {
        destruct_instance(instance);
        return err_code;
    }

    // Andy - I think this has a bug. I'm guessing this creates the default timer group.
    // The problem is that by the API, only the default instance can haev a default group
    // as there is no way to distinguish. So, possibly this should only happen for the
    // first instance, not every time.

    PMTM_timer_group_t group_id = new_timer_group(instance);
    if (group_id < 0) {
        return PMTM_ERROR_CREATE_GROUP_FAILED;
    }

    err_code = construct_timer_group(get_timer_group(group_id), DEFAULT_GROUP_NAME);
    if (err_code != 0) {
        destruct_instance(instance);
        return err_code;
    }

    return PMTM_SUCCESS;
}

/**
 * Create the PMTM output file, writing the header and timer overheads. This
 * can be called to change the file used by PMTM but the existing file will be
 * closed without any timing information stored in it.
 * 
 * @param file_name [IN] The base name of the file, this will be appended with
 *                       a ".pmtm" suffix as well as an integer >= 0 so that the
 *                       file name does not already exist.
 * @param instance  [IN] The instance with which this file will be associated.
 * @returns PMTM_SUCCESS if successful, or one of PMTM_ERROR_* codes if not.
 */
PMTM_error_t set_file(
        struct PMTM_instance * instance,
        const char * file_name)
{
    PMTM_error_t err_code;
    
    if (instance->rank == IO_RANK && instance->fid != NULL) {
        fputs("\nEnd of File\n", instance->fid);
        if (instance->fid != stdout) {
            fclose(instance->fid);
	    if (instance->rank == IO_RANK) {
	      move_output_file(instance);
	    }
        }
    }
    
    if (instance->rank == IO_RANK) {
        err_code = create_file(instance, file_name);
        if (err_code != 0) {
            return err_code;
        }
    }

    if (instance->fid != NULL) {
        err_code = calc_overhead(instance);
        if (err_code != 0) {
            destruct_instance(instance);
            return err_code;
        }
    }
    
    return PMTM_SUCCESS;
}

/**
 * Create the PMTM output file associated with this instance and open it ready
 * for writing. The file handle to this open file is then stored in the instance
 * so we can use it to write to the correct file.
 *
 * @param file_name [IN] The base name of the file, this will be appended with
 *                       a ".pmtm" suffix as well as an integer >= 0 so that the
 *                       file name does not already exist.
 * @param instance  [IN] The instance with which this file will be associated.
 * @returns PMTM_SUCCESS if successful, or one of PMTM_ERROR_* codes if not.
 */
PMTM_error_t create_file(
        struct PMTM_instance * instance,
        const char * file_name)
{
    if (file_name[0] == '\0') {
        instance->fid = NULL;
        instance->file_name = NULL;
        return PMTM_SUCCESS;
    } else if (file_name[0] == '-' && file_name[1] == '\0') {
        instance->fid = stdout;
        copy_string(&instance->file_name, "<stdout>");
    } else {
        const size_t buffer_sz = 200;
        char buffer[buffer_sz];

        PMTM_BOOL nonexisting_file = PMTM_FALSE;
        int file_number = 0;

        while (!nonexisting_file) {
            int n = snprintf(buffer, buffer_sz, "%s%d.pmtm", file_name, file_number);
            if (n == -1 || n >= buffer_sz) {
                return PMTM_ERROR_FILE_NAME_TOO_LONG;
            }
            if (access(buffer, F_OK) != 0) {
                nonexisting_file = INTERNAL__TRUE;
            } else {
                ++file_number;
            }
        }

        copy_string(&instance->file_name, buffer);
	
#ifdef NOLOCAL
	char * file_store = "%DIRECTORY%" ;
	char * filename = malloc(strlen(file_store) + strlen(instance->file_name) + 1);
	strcpy(filename,file_store);
	strcat(filename,instance->file_name);
        instance->fid = fopen(filename, "w"); //instance->file_name, "w");
        if (instance->fid == NULL) {
            return PMTM_ERROR_CANNOT_CREATE_FILE;
        }
        free(filename);
#else
	instance->fid = fopen(instance->file_name, "w");
	
	if (instance->fid == NULL) {
            return PMTM_ERROR_CANNOT_CREATE_FILE;
        }
#endif
    }
    
    return write_file_header(instance);
}

/**
 * Write the PMTM file header.
 *
 * @param instance [IN] The instance to whose file we are writing the header.
 */
PMTM_error_t write_file_header(struct PMTM_instance * instance)
{
    FILE * fid = instance->fid;

    fputs("Performance Modelling Timing File\n\n", fid);
#ifdef PMTM_DEBUG
    fprintf(fid, "PMTM Version, =, %d.%d.%d, (debug)\n",
            INTERNAL__VERSION_MAJOR, INTERNAL__VERSION_MINOR, INTERNAL__VERSION_BUILD);
#else
    fprintf(fid, "PMTM Version, =, %d.%d.%d\n",
            INTERNAL__VERSION_MAJOR, INTERNAL__VERSION_MINOR, INTERNAL__VERSION_BUILD);
#endif
    fprintf(fid, "Application, =, %s\n", instance->application_name);

    time_t timer = time(NULL);
    struct tm * tm_ptr = localtime(&timer);

    const uint string_len = 20;
    const uint pickup_string_len = 200;
    char date_string[string_len];
    char time_string[string_len];
    char id_string[string_len];

    strftime(date_string, string_len, "%d-%m-%Y", tm_ptr);
    strftime(time_string, string_len, "%H:%M", tm_ptr);
    strftime(id_string, string_len, "%Y%m%d-%H%M%S", tm_ptr);
    
    fprintf(fid, "Date, =, %s\n", date_string);
    fprintf(fid, "Time, =, %s\n", time_string);
    fprintf(fid, "Run ID, =, %s-%010d\n", id_string, getpid());
    fprintf(fid, "NProcs, =, %d\n", instance->nranks);
#ifdef _OPENMP
    fprintf(fid, "Max OpenMP Threads, =, %d\n", omp_get_max_threads());
#else
    fprintf(fid, "Max OpenMP Threads, =, 1\n");
#endif
  
    fprintf(fid, "Machine, =, %s, %s\n", QUOTE(MACHINE_VENDOR), QUOTE(MACHINE_NAME));
    
    
    /*
     * The below sections have been changed to use the System Pickup routines.
     * I have commented out the original lines that use the information from the Makefile.
     * 
     * In the future it might be prudent to do a check between what is set and what is used
     * 
     * Also, it may be a good idea to print the originals as parameters at a later date.
     *
     */  
    
    /*
     * The Processor statistics
     * 
     * fprintf(fid, "Processor, =, %s, %s, %s, %s, %s, %s\n", QUOTE(PROC_VENDOR), QUOTE(PROC_NAME),
     *       QUOTE(PROC_ARCH), QUOTE(PROC_CLOCK), QUOTE(PROC_CORES), QUOTE(PROC_THREADS));
     *
     */
    char procVendor[pickup_string_len],procName[pickup_string_len],procArch[pickup_string_len];
    int procClock,procCores,procThreads;
    
    getProcInfo(procVendor,procName,procArch,&procClock,&procCores,&procThreads);
    
    fprintf(fid, "Processor, =, %s, %s, %s, %d, %d, %d\n", procVendor, procName, procArch, procClock, procCores, procThreads);
    
    /*
     * The Operating System statistics
     *  
     * fprintf(fid, "OS, =, %s, %s, %s, %s\n", QUOTE(SYSTEM_VENDOR), QUOTE(SYSTEM_NAME), QUOTE(SYSTEM_VERSION), QUOTE(SYSTE*M_KERNEL));
     * 
     */
    char osVendor[pickup_string_len],osName[pickup_string_len],osVersion[pickup_string_len],osKernel[pickup_string_len],osNode[pickup_string_len];  
    
    getOSInfo(osVendor,osName,osVersion,osKernel,osNode);
    
    fprintf(fid, "OS, =, %s, %s, %s, %s\n", osVendor, osName, osVersion, osKernel);
    
    /*
     * The Compiler statistics
     * 
     * It may well be that this is duplicated code as the Compiler class does more or less the 
     * same thing as versions.h but it gives us scope for changing the compiler method if a better 
     * way is found (maybe calling icc -V or gcc -v and parsing the output - but this assumes that
     * a module or some such is needed to run the binary)
     * /* Compiler type detected in versions.h 
     * 
     * fprintf(fid, "Compiler, =, %s, %s, %s\n", COMPILER_VENDOR, COMPILER_NAME, COMPILER_VERSION);
     * 
     */
     char compVendor[pickup_string_len], compName[pickup_string_len], compVersion[pickup_string_len];
     
     getCompInfo(compVendor,compName,compVersion);
     
     fprintf(fid, "Compiler, =, %s, %s, %s\n", compVendor, compName, compVersion);
     
#ifndef SERIAL
     /*
      * The MPI statistics
      * 
      * fprintf(fid, "MPI, =, %s, %s, %s\n", QUOTE(MPI_VENDOR), QUOTE(MPI_NAME), QUOTE(MPI_LIB_VER));
      * 
      */
     char mpiVendor[pickup_string_len],mpiName[pickup_string_len],mpiVersion[pickup_string_len],mpiStandard[pickup_string_len];
     
     getMpiInfo(mpiVendor,mpiName,mpiVersion,mpiStandard);
     
     fprintf(fid, "MPI, =, %s, %s, %s\n", mpiVendor, mpiName, mpiVersion);
     
#else
    fprintf(fid, "MPI, =, %s\n", "Serial");
#endif
    /* Output the value of the PMTM_TAG environmental variable if it exists. */
    char * tag = getenv("PMTM_TAG");
    if (tag != NULL && strlen(tag) > 0) {
        fprintf(fid, "Tag, =, %s\n", tag);
    }
    /* Output any flags that have been given with PMTM_log_flags function. */
    if (flag_array_sz > 0) {
        fputs("Flags, =,", fid);
        uint flag_idx;
        for (flag_idx = 0; flag_idx < flag_array_sz; ++flag_idx) {
            fprintf(fid, " %s,", flag_array[flag_idx]);
            free((void *) flag_array[flag_idx]);
        }
        fputs("\n", fid);
        free(flag_array);
        flag_array_sz = 0;
    }
    /* Output any variables specifically mentioned in any existent .pmtmrc file */
    PMTM_error_t err_code = get_specific_runtime_variables(instance);
    if (err_code != 0) {
            return err_code;
    }
    
    if (output_env == PMTM_TRUE) {
        /* Output the raw environmental for later post-processing as necessary. */
        int env_idx = 0;
        while (1) {
            if (environ[env_idx] == NULL)
                break;
            fprintf(fid, "Environ, =, %s\n", environ[env_idx]);
            ++env_idx;
        }
    }
    fputs("#Type, , MPI Rank, , Name, , Value, (, StDev, ), , Count\n", fid);
    
    return PMTM_SUCCESS;
}

/**
 * Perform the overhead calculations and print the times to the output file of
 * the given instance.
 *
 * @param instance [IN] The instance for which to perform the calculations.
 * @returns PMTM_SUCCESS if successful, or one of PMTM_ERROR_* codes if not.
 */
PMTM_error_t calc_overhead(const struct PMTM_instance * instance)
{
    if (instance->rank == IO_RANK) {
        struct PMTM_timer timer[3];

        RETURN_ON_ERR(construct_timer(&timer[0], "", INTERNAL__TIMER_NONE));
        RETURN_ON_ERR(construct_timer(&timer[1], "start-stop", INTERNAL__TIMER_NONE));
        RETURN_ON_ERR(construct_timer(&timer[2], "pause-continue", INTERNAL__TIMER_NONE));

        const uint overhead_repeats = 20;
        const uint timer_repeats    = 10000;

        uint repeat_idx;
        for (repeat_idx = 0; repeat_idx < overhead_repeats; ++repeat_idx) {
            /* Start/Stop overhead. */
            start_timer(&timer[1]);
            uint ss_idx;
            for (ss_idx = 0; ss_idx < timer_repeats; ++ss_idx) {
                start_timer(&timer[0]);
                stop_timer(&timer[0]);
            }
            stop_timer(&timer[1]);

            start_timer(&timer[0]);

            /* Pause/Continue overhead. */
            start_timer(&timer[2]);
            uint pc_idx;
            for (pc_idx = 0; pc_idx < timer_repeats; ++pc_idx) {
                pause_timer(&timer[0]);
                continue_timer(&timer[0]);
            }
            stop_timer(&timer[2]);

            stop_timer(&timer[0]);
        }

        print_overhead(instance, &timer[1], timer_repeats);
        print_overhead(instance, &timer[2], timer_repeats);

        destruct_timer(&timer[0]);
        destruct_timer(&timer[1]);
        destruct_timer(&timer[2]);
    }

    return PMTM_SUCCESS;
}

/**
 * Extracts data written in file one word at a time.
 * 
 * @param filename [IN] The file to read from 
 * @param instance [IN] The instance for which to write out the information
 */
void get_file_words(const char * filename, char * wanted[], int *wIndex) //, const struct PMTM_instance * instance)
{
	FILE * sysrcid = fopen(filename,"rt");
	      
	  if(sysrcid){
		  
		  char *line = NULL;
		  size_t len = 0;
		  ssize_t read;

		  while ((read = getline(&line, &len, sysrcid)) != -1) {
		    if(read > 0 && line[read -1] == '\n') {
		      line[read-1] = 0;
		    }
		    
		    if(isVariable(line,read) == PMTM_TRUE)
		    {
		    
		    // Check to see if line is already in wanted
		    int success = check(wanted,line,wIndex);
		    if(success)
		      (*wIndex)++;
		    }
		  }

		  free(line);
	  }
}

/**
 * Takes a line from a .pmtmrc file and checks to see if it is a
 * VARIABLE VALUE pair. If it is then it sets the appropriate internal variable with 
 * the value and returns 0, else it returns 1 to let the caller now that the line is 
 * just a environment variable name
 *
 * @param line  [IN] The line to chech for VARIABLE VALUE pair
 * @param lsize [IN] The size of the line
 * @returns PMTM_TRUE if it is just a variable name or PMTM_FALSE if it is VARIABLE 
 *          VALUE pair
 */
PMTM_BOOL isVariable(char * line, ssize_t lsize)
{
    int i;
    PMTM_BOOL isVar = PMTM_TRUE;
    
    for(i = 0; i<lsize-1; i++)
    {
      if(line[i] == ' ' || line[i] == '=')
      {
	isVar = PMTM_FALSE;
	
	// Setting up parseVal as a pointer to the first element in line that contains the value
	char * parseVal = &line[i+1];
	
	if(strncmp(line,"PMTM_DATA_STORE", 15) == 0)
	{
	    pmtm_file_store = (char *) malloc(((lsize-2)-i)*sizeof(char));
	    strcpy(pmtm_file_store,parseVal);
	}
	else if(strncmp(line,"PMTM_OPTION_NO_LOCAL_COPY", 25) == 0)
	{
	    if(   parseVal[0] != '\0'
	       && strncmp(parseVal,"0",1)  != 0
	       && strncmp(toUpper(parseVal),"FALSE",5) != 0)
	    {
	      no_local_copy = PMTM_TRUE;
	    }
	}
	else if(strncmp(line,"PMTM_OPTION_NO_STORED_COPY", 26) == 0)
	{
	    if(   parseVal[0] != '\0'
	       && strncmp(parseVal,"0",1)  != 0
	       && strncmp(toUpper(parseVal),"FALSE",5) != 0)
	    {
	      no_stored_copy = PMTM_TRUE;
	    }
	}
	else if(strncmp(line,"PMTM_OPTION_OUTPUT_ENV", 22) == 0)
	{
	    if(   parseVal[0] == '\0'
	       || strncmp(parseVal,"0",1)  == 0
	       || strncmp(toUpper(parseVal),"FALSE",5) == 0)
	    {
	      output_env = PMTM_FALSE;
	    }
	    else
	    {
	      output_env = PMTM_TRUE;
	    }
	}
      }
    }
    
    return isVar;
}

/**
 * Finds and writes out specific runtime environment variables set in an 
 * .pmtmrc file. The file can be in either /etc or ${HOME} or the run
 * directory
 *
 * @param instance [IN] The instance for which to write out the information
 * @returns PMTM_SUCCESS if successful, or one of PMTM_ERROR_* codes if not.
 */
PMTM_error_t get_specific_runtime_variables(const struct PMTM_instance * instance)
{
    if (instance->rank == IO_RANK) {
      static const int INITIAL_VAR_SIZE    = 1000; 
      int  alloc_size; 
      int     indx,i,success; 
      char ** env_vars;
      
      indx = 0; 
      alloc_size = INITIAL_VAR_SIZE; 
      env_vars = (char **)malloc(alloc_size * sizeof(char *));
  
      // Find out what Specific Environment Variables are required from /etc/INTERNAL__RCFILENAME   
      char * etcname = malloc(strlen(INTERNAL__RCFILENAME) + 9);
      strcpy(etcname,"/awe/etc");
      strcat(etcname,INTERNAL__RCFILENAME);
      get_file_words(etcname, env_vars,&indx); //, instance);
      
      // Find out what Specific Environment Variables are required from /etc/INTERNAL__RCFILENAME
      char * fn1 = getenv("HOME");
      char * fname1 = malloc(strlen(fn1) + strlen(INTERNAL__RCFILENAME) + 1);
      if ( fname1 == NULL ) return PMTM_ERROR_FAILED_ALLOCATION;
      strcpy(fname1,fn1);
      strcat(fname1,INTERNAL__RCFILENAME);
      get_file_words(fname1, env_vars, &indx); //instance);
      free(fname1);
      
      // Find out what Specific Environment Variables are required from ${PWD}/INTERNAL__RCFILENAME
      char * fn2 = getenv("PWD");
      char * fname2 = malloc(strlen(fn2) + strlen(INTERNAL__RCFILENAME) + 1);
      if ( fname2 == NULL ) return PMTM_ERROR_FAILED_ALLOCATION;
      strcpy(fname2,fn2);
      strcat(fname2,INTERNAL__RCFILENAME);
      get_file_words(fname2, env_vars, &indx); //instance);
      free(fname2);
      
      for(i = 0; i< indx; ++i){ 
	success = output_specific_runtime_variable(instance, env_vars[i]);
      }
      free(env_vars);
    }
    
    return PMTM_SUCCESS;
}

PMTM_error_t output_specific_runtime_variable(const struct PMTM_instance * instance, const char * envVar)
{
    char * envval = getenv(envVar);
    if (envval != NULL && strlen(envval) > 0 && instance->rank == IO_RANK) {
      fprintf(instance->fid, "Specific, %s, =, %s\n", envVar, envval);
    }
    
    return PMTM_SUCCESS;
}

/**
 * Check to see if an environment variable is already in the list of specific 
 * variables to record and if not add it to the array, kindly written by DT
 * 
 * @param env_vars [IN] The current array of variables to print 
 * @param new_var [IN] The variable to check
 * @param indx [IN] The index of the current end of array
 * @returns 0 if the check was successful.
 */

int check(char * env_vars[], char * new_var, int *indx){ 
  const int  STR_SIZE = 32; 
  char * test_var; 
  int i; 
  int gexisting = 0; 
  
  

  for(i = 0; i< *indx; ++i){ 
    if (0 == strncmp(new_var,env_vars[i],STR_SIZE-1)){ 
      gexisting = 1; 
    } 
  } 
  if (0 == gexisting){ 
    test_var = (char *)malloc(STR_SIZE * sizeof(char));
    strcpy(test_var,new_var);
    env_vars[*indx] =  test_var;  
//     ++(*indx); 
//     printf( "\nadding %d %s" ,i,env_vars[i]); 
    return 1;
  }else{ 
   return 0; 
  } 
  return 0; 
} 


/**
 * Destroy a given instance, reclaiming any memory allocated in its construction
 * and use, and wiping out all data stored within it.
 *
 * @param instance The instance to destroy.
 */
void destruct_instance(struct PMTM_instance * instance)
{
    if (instance->initialised) {
        
        if (instance->rank == IO_RANK && instance->fid != NULL) {
            fputs("\nEnd of File\n", instance->fid);
            if (instance->fid != stdout) {
                fclose(instance->fid);
		move_output_file(instance);
            }
        }
        
        instance->initialised = 0;

        free(instance->application_name);
        free(instance->file_name);

        uint group_idx;
        for (group_idx = 0; group_idx < instance->num_groups; ++group_idx) {
            struct PMTM_timer_group * group = get_timer_group(instance->group_ids[group_idx]);
            destruct_timer_group(group);
        }
        free(instance->group_ids);

        uint param_idx;
        for (param_idx = 0; param_idx < instance->num_parameters; ++param_idx) {
            struct parameter * param = &instance->parameters[param_idx];
            destruct_parameter(param);
        }
        free(instance->parameters);
    }
}

/**
 * Construct a timer group struct, allocating the memory required for it's
 * fields and populating the fields with the given data.
 *
 * @param group      [IN] The timer group to construct, the memory for which
 *                        should already be allocated.
 * @param group_name [IN] The timer of the timer group.
 * @returns PMTM_SUCCESS if successful, or one of PMTM_ERROR_* codes if not.
 */
PMTM_error_t construct_timer_group(
        struct PMTM_timer_group * group,
        const char * group_name)
{
    copy_string(&group->group_name, group_name);
    check_for_commas(group->group_name);

    group->num_timers = 0;
    group->timer_ids = NULL;
    group->total_timers = 0;

    return PMTM_SUCCESS;
}

/**
 * Destroy a given timer group, reclaiming any memory allocated in its
 * construction and use, and wiping out all data stored within it.
 *
 * @param group The timer group to destroy.
 */
void destruct_timer_group(struct PMTM_timer_group * group)
{
    free(group->group_name);

    uint timer_idx;
    for (timer_idx = 0; timer_idx < group->num_timers; ++timer_idx) {
        struct PMTM_timer * timer = get_timer(group->timer_ids[timer_idx]);
        destruct_timer(timer);
    }
    free(group->timer_ids);
}

/**
 * Construct a timer struct, allocating the memory required for it's fields and
 * populating the fields with the given data.
 *
 * @param timer      [IN] The timer to construct, the memory for which should
 *                        already be allocated.
 * @param timer_name [IN] The name of the timer, or NULL if the timer is already named.
 *                        Do not specify timer_name to this if this timer might be part of
 *                        a thread group and already active.
 * @param timer_type [IN] The type of the timer, e.g. PMTM_TIMER_MAX.
 * @returns PMTM_SUCCESS if successful, or one of PMTM_ERROR_* codes if not.
 */
PMTM_error_t construct_timer(
        struct PMTM_timer * timer,
        const char * timer_name,
        PMTM_timer_type_t timer_type)
{   
#ifdef HW_COUNTERS
    timer->start_counters = (hw_counter_t *) calloc(get_num_hw_events(), sizeof(hw_counter_t));
    timer->stop_counters  = (hw_counter_t *) calloc(get_num_hw_events(), sizeof(hw_counter_t));
    timer->total_counters = (hw_counter_t *) calloc(get_num_hw_events(), sizeof(hw_counter_t));
#endif

    if (timer_name != NULL) {
        copy_string(&timer->timer_name, timer_name);
        check_for_commas(timer->timer_name);
    }

    timer->timer_type = timer_type;
    timer->last_wc = 0;
    timer->last_cpu = 0;
    timer->current_wc = 0;
    timer->current_cpu = 0;
    timer->total_wc = 0;
    timer->total_square_wc = 0;
    timer->total_cpu = 0;
    timer->total_square_cpu = 0;
    timer->timer_count = 0;
    timer->pause_count = 0;
    timer->frequency = 1;
    timer->max_samples = PMTM_NO_MAX;
    timer->num_samples = 0;
    timer->ignore = INTERNAL__FALSE;
    timer->rank = -1;
    timer->is_printed = INTERNAL__FALSE;
#ifdef PMTM_DEBUG
    timer->state = TIMER_STOPPED;
#endif

    return PMTM_SUCCESS;
}

/**
 * Destroy a given timer, reclaiming any memory allocated in its construction
 * and use, and wiping out all data stored within it.
 *
 * @param timer The timer to destroy.
 */
void destruct_timer(struct PMTM_timer * timer)
{
    free(timer->timer_name);
#ifdef HW_COUNTERS
    free(timer->start_counters);
    free(timer->stop_counters);
    free(timer->total_counters);
#endif
}

/**
 * Construct a parameter struct, allocating the memory required for it's fields
 * and populating the fields with the given data.
 *
 * @param param           [IN] The parameter to construct, the memory for which
 *                             should already be allocated.
 * @param parameter_name  [IN] The name of the parameter.
 * @param parameter_value [IN] The value of the parameter (as a string).
 * @param rank            [IN] The rank on which the parameter was created.
 */
void construct_parameter(
        struct parameter * param,
        const char * parameter_name,
        const char * parameter_value,
        int rank)
{
    memset(param, '\0', sizeof(struct parameter));

    copy_string(&param->parameter_name, parameter_name);
    copy_string(&param->parameter_value, parameter_value);

    param->rank = rank;
    param->count = 1;
}

/**
 * Destroy a given parameter, reclaiming any memory allocated in its
 * construction and use, and wiping out all data stored within it.
 *
 * @param param The parameter to destory.
 */
void destruct_parameter(struct parameter * param)
{
    free(param->parameter_name);
    free(param->parameter_value);
    
    memset(param, '\0', sizeof(struct parameter));
}

/**
 * Finalize the PMTM library, destroying all remaining instances and cleaning up
 * any memory that it has been using. This function should leave the library in
 * a state that can be correctly reinitialised.
 */
void finalize()
{    
    struct PMTM_instance * curr_instance = instance_head;

    while (curr_instance != NULL) {
        struct PMTM_instance * next_instance = curr_instance->next;
        destruct_instance(curr_instance);
        free(curr_instance);
        curr_instance = next_instance;
    }

    instance_head = NULL;
    instance_tail = &instance_head;
    instance_count = 0;

    struct PMTM_timer_group * curr_tg = group_head;

    while (curr_tg != NULL) {
        struct PMTM_timer_group * next_tg = curr_tg->next;
        free(curr_tg);
        curr_tg = next_tg;
    }

    group_head = NULL;
    group_tail = &group_head;
    group_count = 0;

    struct PMTM_timer * curr_timer = timer_head;

    while (curr_timer != NULL) {
        struct PMTM_timer * next_timer = curr_timer->next;
        free(curr_timer);
        curr_timer = next_timer;
    }

    timer_head = NULL;
    timer_tail = &timer_head;
    timer_count = 0;
}


/**
 * Expand the memory of an array to fit in exactly one more element and return
 * a pointer to the new allocated memory.
 *
 * @param array        [IN/OUT] The array to expand.
 * @param num_elements [IN]     The number of elements currently in the array.
 * @param element_size [IN]     The size of a single element.
 */

void * add_array_element(
        void ** array,
        int num_elements,
        size_t element_size)
{
    const size_t old_size = num_elements * element_size;
    const size_t new_size = (num_elements + 1) * element_size;
    
    void * tmp_array = malloc(old_size);
    memcpy(tmp_array, *array, old_size);
    free(*array);

    *array = malloc(new_size);
    if (*array == NULL) return NULL;

    memset(*array, '\0', new_size);
    memcpy(*array, tmp_array, old_size);
    free(tmp_array);

    return ((char *)(*array) + old_size);
}


/**
 * Allocate the memory for a new instance structure and return the ID to this
 * newly created instance. For OpenMP, this needs locking of the instance list.
 *
 * @returns The ID of the new instance.
 */
PMTM_instance_t new_instance()
{
    PMTM_instance_t new_id;

    struct PMTM_instance * instance = malloc(sizeof(struct PMTM_instance));

    if (instance == NULL) {
        return FAILED_ARRAY_ADD;
    }

    instance->initialised = 0;
    instance->next = NULL;
    *instance_tail = instance;
    instance_tail = &instance->next;

    new_id = instance_count;
    ++instance_count;
    
    return new_id;
}

/**
 * Allocate the memory for a new timer group structure and return the ID to this
 * newly created group. For OpenMP, this routine needs locking of the group list
 * and parent instance.
 *
 * @param instance [IN] The instance to associate this new group with.
 * @returns The ID of the new group.
 */
PMTM_timer_group_t new_timer_group(struct PMTM_instance * instance)
{
    PMTM_timer_group_t group_id;

    struct PMTM_timer_group * group = malloc(sizeof(struct PMTM_timer_group));

    if (group == NULL) {
        return FAILED_ARRAY_ADD;
    }

    PMTM_timer_group_t * id = (PMTM_timer_group_t *)
        add_array_element((void **) &instance->group_ids, instance->num_groups, sizeof(PMTM_timer_group_t));

    if (id == NULL) {
        free(group);
        return FAILED_ARRAY_ADD;
    }

    
    group->instance = instance;
    group->next = NULL;
    *group_tail = group;
    group_tail = &group->next;

    group_id = group_count;
    ++group_count;
    ++(instance->num_groups);
    *id = group_id;

    return group_id;
}

/**
 * Allocate the memory for a new timer structure and return the ID to this
 * newly created timer. For OpenMP, this routine needs locking of the
 * timer list and parent group.
 *
 * @param group [IN] The timer group to associate this new timer with.
 * @param timer_name [IN] The name of the timer.
 * @returns The ID of the new timer.
 */
PMTM_timer_t new_timer(struct PMTM_timer_group * group, const char * timer_name)
{
    PMTM_timer_t result = FAILED_TIMER_ADD;

    struct PMTM_timer * timer = malloc(sizeof(struct PMTM_timer));
    if (timer == NULL) return FAILED_TIMER_ADD;

    // Put in the name of the timer

    char * copy_timer_name;

    copy_string(&copy_timer_name, timer_name);
    check_for_commas(copy_timer_name);
    timer->timer_name = copy_timer_name;

#ifdef _OPENMP
    // Put in the thread ID

    int thread_id = omp_get_thread_num();
    timer->thread_id = thread_id;
#endif

    // Based on the name, find its position in the group timer_ids array or add a
    // new entry if not found.

    int position = 0;
    int num_timers = group->num_timers;

#ifdef _OPENMP
    struct PMTM_timer ** timer_ids = group->timer_ids;

    while (position < num_timers && strcmp(copy_timer_name, timer_ids[position]->timer_name) != 0) {
        ++position;
    }
#else
    position = num_timers;
#endif

    if (position == num_timers) {
        struct PMTM_timer ** id = (struct PMTM_timer **)
            add_array_element((void **) &group->timer_ids, num_timers, sizeof(PMTM_timer_t));

        if (id == NULL) {
            free(copy_timer_name);
            free(timer);
            return FAILED_TIMER_ADD;
        }

        *id = NULL;
        ++(group->num_timers);
    }

    ++(group->total_timers);

    // Insert the new entry into the timer_ids list.

    struct PMTM_timer ** id = &group->timer_ids[position];

#ifdef _OPENMP
    while (*id != NULL && (*id)->thread_id <= thread_id) {
        id = &(*id)->thread_next;
    }
#endif

    timer->thread_next = *id;
    *id = timer;

    // Insert the new entry into the timer list.

    timer->next = NULL;
    *timer_tail = timer;
    timer_tail = &timer->next;
    ++timer_count;

    return timer;
}



/**
 * Return the number of instances that have been created. This also includes
 * the default instance so will return >= 1 if PMTM has been initialised.
 *
 * @returns the number of PMTM instances.
 */
int get_instance_count() {
    return instance_count;
}

/**
 * Return the instance structure associated with the given ID. No restriction on
 * on OpenMP use, locking not required. The calling program should guarantee two
 * threads not using the returned instance for unprotected simultaneous updates. 
 *
 * @param instance_id [IN] The ID of the instance to retrieve.
 */
struct PMTM_instance * get_instance(PMTM_instance_t instance_id)
{
    struct PMTM_instance * instance = instance_head;
    int id = 0;

    while (instance != NULL && instance_id != id) {
        instance = instance->next;
        ++id;
    }

    return instance;
}

/**
 * Return the timer group structure associated with the given ID. No restriction on
 * on OpenMP use, locking not required. The calling program should guarantee two
 * threads not using the returned group for unprotected simultaneous updates.
 *
 * @param group_id The ID of the group to retrieve.
 */
struct PMTM_timer_group * get_timer_group(PMTM_timer_group_t group_id)
{
    struct PMTM_timer_group * group = group_head;
    int id = 0;

    while (group != NULL && group_id != id) {
        group = group->next;
        ++id;
    }

    return group;
}

/**
 * Return the timer structure associated with the given ID. No restriction on
 * on OpenMP use, locking not required. The calling program should guarantee two
 * threads not using the returned timer for unprotected simultaneous updates.
 *
 * @param timer_id The ID of the timer to retrieve.
 */
struct PMTM_timer * get_timer(PMTM_timer_t timer_id)
{
    return timer_id;
}


/**
 * Returns whether or not PMTM is in an initialised state.
 */
uint is_initialised()
{
    return (instance_head != NULL
            && get_instance(INTERNAL__DEFAULT_INSTANCE)->initialised);
}

#ifdef PMTM_DEBUG
/**
 * Returns the textual representation of a timer state.
 *
 * @param state [IN] The timer state.
 * @returns a string describing the timer state.
 */
const char * get_state_desc(int state)
{
    switch (state) {
        case TIMER_STOPPED: return "<STOPPED>";
        case TIMER_ACTIVE:  return "<STARTED>";
        case TIMER_PAUSED:  return "<PAUSED>";
        default:            return "<UNKNOWN>";
    }
}
#endif

/**
 * Print an "Overhead" line to the PMTM output file using the results stored in
 * the given timer.
 *
 * @param instance      [IN] The instance to whose output file we are writing.
 * @param timer         [IN] The timer containing the overhead timing results.
 * @param timer_repeats [IN] The number of times the overhead timings was
 *                           repeated.
 */
void print_overhead(
        const struct PMTM_instance * instance,
        const struct PMTM_timer * timer,
        uint timer_repeats)
{
    double avg = timer->total_wc / timer->timer_count;
    double std_dev = timer->total_square_wc / timer->timer_count - pow(avg, 2);

    avg /= timer_repeats;
    std_dev /= timer_repeats;

    fprintf(instance->fid, "Overhead, (, 0, ), %s, =, %12.6E, (, %12.6E, )\n",
            timer->timer_name, avg, std_dev);
}

/**
 * Print a "Timer" line to the PMTM output file using the results stored in the
 * given timer.
 *
 * @param instance [IN] The instance to whose output file we are writing.
 * @param timer    [IN] The timer containing the timing results.
 */
void print_timer(
        const struct PMTM_instance * instance,
        struct PMTM_timer * timer)
{
    double avg_time = 0;
    double std_dev = 0;
    int pause_per_block = 0;

    if (timer->timer_count != 0) {
        avg_time = timer->total_wc / timer->timer_count;
        std_dev = timer->total_square_wc / timer->timer_count - pow(avg_time, 2);
        pause_per_block = timer->pause_count / timer->timer_count;
    }

    char rank_text[20];

    if (timer->rank != -1) {
#ifdef _OPENMP
        sprintf(rank_text, "%d.%d", timer->rank, timer->thread_id);
#else
        sprintf(rank_text, "%d.0", timer->rank);
#endif
    } else {
        switch (timer->timer_type) {
            case INTERNAL__TIMER_NONE: sprintf(rank_text, "%d.0", 0); break;
            case INTERNAL__TIMER_AVG:  sprintf(rank_text, "%s", "Rank Average"); break;
            case INTERNAL__TIMER_MAX:  sprintf(rank_text, "%s", "Rank Maximum"); break;
            case INTERNAL__TIMER_MIN:  sprintf(rank_text, "%s", "Rank Minimum"); break;
            default:          sprintf(rank_text, "%s", "Unknown Type"); break;
        }
    }
   
#ifndef HW_COUNTERS 
    fprintf(instance->fid,
            "Timer, : (, %s, ), %s, =, %12.6E, (, %12.6E, ), count, %d, paused, %d\n",
            rank_text, timer->timer_name, avg_time, std_dev,
            timer->timer_count, pause_per_block);
#else
    fprintf(instance->fid,
            "Timer, : (, %s, ), %s, =, %12.6E, (, %12.6E, ), count, %d, paused, %d",
            rank_text, timer->timer_name, avg_time, std_dev,
            timer->timer_count, pause_per_block);
    int counter_idx;
    for (counter_idx = 0; counter_idx < get_num_hw_counters(); ++counter_idx) {
        fprintf(instance->fid, ", hw_counter, %s, %d",
                get_counter_name(counter_idx), timer->total_counters[counter_idx]);
    }
    fputs("\n");
#endif

    timer->is_printed = INTERNAL__TRUE;
}

/**
 * Check whether the given parameter matches the given output condition within
 * the context of the given instance.
 *
 * @param instance        [IN] The instance we are checking the parameter
 *                             against.
 * @param parameter_name  [IN] The name of the parameter.
 * @param parameter_value [IN] The value of the parameter.
 * @param output_type     [IN] The output condition for the parameter.
 * @returns TRUE if the parameter matches the output condition, FALSE otherwise.
 */
PMTM_BOOL
check_parameter(
        struct PMTM_instance * instance,
        const char * parameter_name,
        const char * parameter_value,
        PMTM_output_type_t output_type,
        int * count)
{
    *count = 0;

    switch (output_type) {
        case INTERNAL__OUTPUT_ALWAYS: {
            *count = store_parameter_value(instance, parameter_name, parameter_value);
            return INTERNAL__TRUE;
        }
        case INTERNAL__OUTPUT_ONCE: {
            const char * stored_value = get_parameter_value(instance, parameter_name);
            if (stored_value == NULL) {
                *count = store_parameter_value(instance, parameter_name, parameter_value);
                return INTERNAL__TRUE;
            } else {
                return INTERNAL__FALSE;
            }
        }
        case INTERNAL__OUTPUT_ON_CHANGE: {
            const char * stored_value = get_parameter_value(instance, parameter_name);
            if (stored_value == NULL || strcmp(parameter_value, stored_value) != 0) {
                *count = store_parameter_value(instance, parameter_name, parameter_value);
                return INTERNAL__TRUE;
            } else {
                return INTERNAL__FALSE;
            }
        }
        default: {
            pmtm_warn("Unknown parameter output type: %d", output_type);
            return INTERNAL__FALSE;
        }
    }
}

/**
 * Print a list of parameters, one on each line, with the same name but
 * values taken from the character buffer given. This is used to output the
 * parameter value on each rank.
 *
 * @param instance         [IN] The instance to whose output file we will be
 *                              outputting.
 * @param parameter_name   [IN] The name of the parameter.
 * @param parameter_values [IN] A character array containing all the parameter
 *                              values.
 * @param num_values       [IN] The number of values given.
 * @param displacements    [IN] The postion along \a parameter_values where
 *                              each value occurs.
 */
void
print_parameter_array(
        struct PMTM_instance * instance,
        const char * parameter_name,
        const char * parameter_values,
        int num_values,
        int * displacements)
{
    if (instance->fid == NULL) {
        return;
    }

    const char * format_string = "Parameter, : (, %d, ), %s, =, %s\n";

    int last_displacement = -1;
    uint value_idx;
    for (value_idx = 0; value_idx < num_values; ++value_idx) {
        int displacement = displacements[value_idx];
        if (displacement != last_displacement) {
            const char * parameter_value = &parameter_values[displacement];
            fprintf(instance->fid, format_string, value_idx, parameter_name, parameter_value);
        }
        last_displacement = displacement;
    }
}

/**
 * Retrieve the value stored against the given parameter name in the given
 * instance.
 *
 * @param instance       [IN] The instance from which to retrieve the parameter.
 * @param parameter_name [IN] The name of the parameter to retrieve.
 * @returns the stored parameter value.
 */

const char * 
get_parameter_value(
        const struct PMTM_instance * instance,
        const char * parameter_name)
{
    struct parameter * param = get_parameter(instance, parameter_name);
    if (param == NULL) {
        return NULL;
    }
    return param->parameter_value;
}

/**
 * Store the given value against the given parameter name within the given
 * instance.
 *
 * @param instance        [IN] The instance in which to store the parameter.
 * @param parameter_name  [IN] The name of the parameter to store.
 * @param parameter_value [IN] The value of the parameter to store.
 * @returns the number of times this parameter has been stored.
 */
int
store_parameter_value(
        struct PMTM_instance * instance,
        const char * parameter_name,
        const char * parameter_value)
{
    struct parameter * param = get_parameter(instance, parameter_name);
    if (param == NULL) {
        param = new_parameter(instance);
        if (param == NULL) {
            pmtm_warn("Failed to store parameter: %s", parameter_name);
            return 0;
        }
        construct_parameter(param, parameter_name, parameter_value, instance->rank);
    } else {
        free(param->parameter_value);
        
        copy_string(&param->parameter_value, parameter_value);
        check_for_commas(param->parameter_value);

        ++(param->count);
    }
    return param->count;
}

/**
 * Allocate the memory for another parameter within the given instance and
 * return the address of this newly allocated memory.
 *
 * @param instance [IN] The instance in which to create the new parameter.
 * @returns a pointer to a newly allocated parameter struct.
 */

struct parameter *
new_parameter(
        struct PMTM_instance * instance)
{
    struct parameter * param = (struct parameter *)
        add_array_element((void **) &instance->parameters, instance->num_parameters, sizeof(struct parameter));

    if (param == NULL) {
        return NULL;
    }
    ++(instance->num_parameters);

    return param;
}

/**
 * Retrieve the parameter structure stored in the given instance with the given
 * parameter name.
 *
 * @param instance       [IN] The instance from which to retrieve the parameter.
 * @param parameter_name [IN] The name of the parameter to retrieve.
 * @returns a pointer to the matching parameter or NULL if no matching parameter
 * was found.
 */

struct parameter *
get_parameter(
        const struct PMTM_instance * instance,
        const char * parameter_name)
{
    struct parameter * param = NULL;
    
    uint param_idx;
    for (param_idx = 0; param_idx < instance->num_parameters; ++param_idx) {
        struct parameter * tmp_param = &instance->parameters[param_idx];
        if (strcmp(tmp_param->parameter_name, parameter_name) == 0) {
            param = tmp_param;
            break;
        }
    }
    return param;
}

/**
 * Start the given timer. If compiled in debug mode also check that the state
 * of the timer is consistent for starting.
 *
 * @param timer [IN] The timer to start.
 */
void start_timer(struct PMTM_timer * timer)
{
    if (timer->num_samples < timer->max_samples
            && timer->num_samples % timer->frequency == 0) {
        timer->ignore = INTERNAL__FALSE;
    } else {
        timer->ignore = INTERNAL__TRUE;
    }

    if (timer->ignore == INTERNAL__FALSE) {
        timer->current_wc  = 0;
        timer->current_cpu = 0;
        set_timers(&timer->last_cpu, &timer->last_wc);
        
#ifdef HW_COUNTERS
        set_counters(timer->start_counters);
#endif
    }

#ifdef PMTM_DEBUG
    if (timer->state != TIMER_STOPPED) {
        const char * this_state = get_state_desc(timer->state);
        const char * good_state = get_state_desc(TIMER_STOPPED);
        pmtm_warn("Timer %s started whilst in state %s, timer must be in %s state",
                timer->timer_name, this_state, good_state);
    }
    timer->state = TIMER_ACTIVE;
#endif
}

/**
 * Stop the given timer. If compiled in debug mode also check that the state
 * of the timer is consistent for stopping.
 *
 * @param timer [IN] The timer to stop.
 */
void stop_timer(struct PMTM_timer * timer)
{
    if (timer->ignore == INTERNAL__FALSE) {
        double cpu_time, wc_time;
        set_timers(&cpu_time, &wc_time);

        /* Add last time block to current sum. */
        timer->current_wc  += (wc_time  - timer->last_wc);
        timer->current_cpu += (cpu_time - timer->last_cpu);

        /* Add elapsed time to sum and squared sum. */
        timer->total_wc         += timer->current_wc;
        timer->total_square_wc  += pow(timer->current_wc, 2);
        timer->total_cpu        += timer->current_cpu;
        timer->total_square_cpu += pow(timer->current_cpu, 2);

        /* Add to the number of times this timer has been counted. */
        ++timer->timer_count;
        
#ifdef HW_COUNTERS
        set_counters(timer->stop_counters);
        
        int hw_counter_idx;
        for (hw_counter_idx = 0; hw_counter_idx < get_num_hw_events(); ++hw_counter_idx) {
            timer->total_counters[hw_counter_idx]
                    += (timer->stop_counters[hw_counter_idx] - timer->start_counters[hw_counter_idx]);
        }
#endif
    }

    ++timer->num_samples;

#ifdef PMTM_DEBUG
    if (timer->state != TIMER_ACTIVE) {
        const char * this_state = get_state_desc(timer->state);
        const char * good_state = get_state_desc(TIMER_ACTIVE);
        pmtm_warn("Timer %s stopped whilst in state %s, timer must be in %s state",
                timer->timer_name, this_state, good_state);
    }
    timer->state = TIMER_STOPPED;
#endif
}

/**
 * Pause the given timer. If compiled in debug mode also check that the state
 * of the timer is consistent for pausing.
 *
 * @param timer [IN] The timer to pause.
 */
void pause_timer(struct PMTM_timer * timer)
{
    if (timer->ignore == INTERNAL__FALSE) {
        double cpu_time, wc_time;
        set_timers(&cpu_time, &wc_time);

        /* Add last time block to current sum. */
        timer->current_wc  += (wc_time  - timer->last_wc);
        timer->current_cpu += (cpu_time - timer->last_cpu);
        ++timer->pause_count;
    }

#ifdef PMTM_DEBUG
    if (timer->state != TIMER_ACTIVE) {
        const char * this_state = get_state_desc(timer->state);
        const char * good_state = get_state_desc(TIMER_ACTIVE);
        pmtm_warn("Timer %s paused whilst in state %s, timer must be in %s state",
                timer->timer_name, this_state, good_state);
    }
    timer->state = TIMER_PAUSED;
#endif
}

/**
 * Continue the given timer. If compiled in debug mode also check that the state
 * of the timer is consistent for continuing.
 *
 * @param timer [IN] The timer to continue.
 */
void continue_timer(struct PMTM_timer * timer)
{
    if (timer->ignore == INTERNAL__FALSE) {
        set_timers(&timer->last_cpu, &timer->last_wc);
    }

#ifdef PMTM_DEBUG
    if (timer->state != TIMER_PAUSED) {
        const char * this_state = get_state_desc(timer->state);
        const char * good_state = get_state_desc(TIMER_PAUSED);
        pmtm_warn("Timer %s continued whilst in state %s, timer must be in %s state",
                timer->timer_name, this_state, good_state);
    }
    timer->state = TIMER_ACTIVE;
#endif
}

/**
 * Return the CPU time since this timer was started.
 *
 * @param timer [IN] The timer for whose CPU time to return.
 * @returns the cpu time.
 */
double get_cpu_time(struct PMTM_timer * timer)
{
    double cpu_time, wc_time;
    set_timers(&cpu_time, &wc_time);

    return (cpu_time - timer->last_cpu);
}

/**
 * Return the CPU time stored in this timer. If compiled in debug mode will also
 * check that the timer is currently stopped.
 *
 * @param timer [IN] The timer for whose CPU time to return.
 * @returns the cpu time.
 */
double get_total_cpu_time(struct PMTM_timer * timer)
{
#ifdef PMTM_DEBUG
    if (timer->state != TIMER_STOPPED) {
        const char * this_state = get_state_desc(timer->state);
        const char * good_state = get_state_desc(TIMER_STOPPED);
        pmtm_warn("Requested CPU time on timer %s that was in state %s, timer must be in %s state",
                timer->timer_name, this_state, good_state);
    }
#endif
    return timer->total_cpu;
}

/**
 * Return the CPU time stored in a timer of the last block timed. If compiled
 * in debug mode will also check that the timer is correctly stopped.
 *
 * @param timer [IN] The timer for whose CPU time to return.
 * @returns the cpu time.
 */
double get_last_cpu_time(struct PMTM_timer * timer)
{
#ifdef PMTM_DEBUG
    if (timer->state != TIMER_STOPPED) {
        const char * this_state = get_state_desc(timer->state);
        const char * good_state = get_state_desc(TIMER_STOPPED);
        pmtm_warn("Requested CPU time on timer %s that was in state %s, timer must be in %s state",
                timer->timer_name, this_state, good_state);
    }
#endif
    return timer->current_cpu;
}

/**
 * Return the wallclock time since this timer was started.
 *
 * @param timer [IN] The timer for whose wallclock time to return.
 * @returns the wall clock time.
 */
double get_wc_time(struct PMTM_timer * timer)
{
    double cpu_time, wc_time;
    set_timers(&cpu_time, &wc_time);

    return (wc_time - timer->last_wc);
}

/**
 * Return the wallclock time stored in this timer. If compiled in debug mode will also
 * check that the timer is currently stopped.
 *
 * @param timer [IN] The timer for whose wallclock time to return.
 * @returns the wall clock time.
 */
double get_total_wc_time(struct PMTM_timer * timer)
{
#ifdef PMTM_DEBUG
    if (timer->state != TIMER_STOPPED) {
        const char * this_state = get_state_desc(timer->state);
        const char * good_state = get_state_desc(TIMER_STOPPED);
        pmtm_warn("Requested wallclock time on timer %s that was in state %s, timer must be in %s state",
                timer->timer_name, this_state, good_state);
    }
#endif
    return timer->total_wc;
}

/**
 * Return the wallclock time stored in a timer of the last block timed. If
 * compiled in debug mode will also check that the timer is correctly stopped.
 *
 * @param timer [IN] The timer for whose wallclock time to return.
 * @returns the wall clock time.
 */
double get_last_wc_time(struct PMTM_timer * timer)
{
#ifdef PMTM_DEBUG
    if (timer->state != TIMER_STOPPED) {
        const char * this_state = get_state_desc(timer->state);
        const char * good_state = get_state_desc(TIMER_STOPPED);
        pmtm_warn("Requested wallclock time on timer %s that was in state %s, timer must be in %s state",
                timer->timer_name, this_state, good_state);
    }
#endif
    return timer->current_wc;
}

/**
 * Print an array of timers, one on each line, all with the same timer name but
 * with different timer values. This is used to print the timers for all ranks.
 *
 * @param instance     [IN] The instance to whose output file we are printing.
 * @param totalthreads [IN] The total number of threads represented in timer_array.
 * @param timer_array  [IN] The array of timers to print.
 * @param timer_name   [IN] The name of the timers.
 * @param timer_type   [IN] The type of the timers.
 */
void print_timer_array(
        const struct PMTM_instance * instance,
        uint totalthreads,
        struct PMTM_timer * timer_array,
        const char * timer_name,
        PMTM_timer_type_t timer_type)
{
    if (instance->fid == NULL) {
        return;
    }
    
    if (timer_type == PMTM_TIMER_INT) {
      return;
    }

    struct PMTM_timer avg_timer;
    struct PMTM_timer max_timer;
    struct PMTM_timer min_timer;

    construct_timer(&avg_timer, timer_name, PMTM_TIMER_AVG);
    construct_timer(&max_timer, timer_name, PMTM_TIMER_MAX);
    construct_timer(&min_timer, timer_name, PMTM_TIMER_MIN);

    avg_timer.total_square_wc = 0;
    max_timer.total_wc = DBL_MIN;
    min_timer.total_wc = DBL_MAX;

    uint rank_idx;

    for (rank_idx = 0; rank_idx < totalthreads; ++rank_idx) {
	struct PMTM_timer * rank_timer = &timer_array[rank_idx];
	
         if ((timer_type != PMTM_TIMER_MMA) && (timer_type != PMTM_TIMER_AVO)) {
	    print_timer(instance, rank_timer);
 	}

        if (timer_type & PMTM_TIMER_AVG) {
            avg_timer.total_wc += rank_timer->total_wc;
            avg_timer.total_square_wc += rank_timer->total_square_wc;
            avg_timer.timer_count += rank_timer->timer_count;
        }

        if (timer_type & PMTM_TIMER_MAX) {
            if (rank_timer->total_wc > max_timer.total_wc) {
                max_timer.total_wc = rank_timer->total_wc;
                max_timer.total_square_wc = rank_timer->total_square_wc;
                max_timer.timer_count = rank_timer->timer_count;
            }
        }

        if (timer_type & PMTM_TIMER_MIN) {
            if (rank_timer->total_wc < min_timer.total_wc) {
                min_timer.total_wc = rank_timer->total_wc;
                min_timer.total_square_wc = rank_timer->total_square_wc;
                min_timer.timer_count = rank_timer->timer_count;
            }
        }
    }

    if ((timer_type & PMTM_TIMER_AVG) || (timer_type & PMTM_TIMER_AVO)) {
        print_timer(instance, &avg_timer);
    }

    if (timer_type & PMTM_TIMER_MAX) {
        print_timer(instance, &max_timer);
    }

    if (timer_type & PMTM_TIMER_MIN) {
        print_timer(instance, &min_timer);
    }

    destruct_timer(&avg_timer);
    destruct_timer(&max_timer);
    destruct_timer(&min_timer);
}

/**
 * Scan a string for commas, printing a warning if any where found and
 * replacing them with spaces. This is needed as commas in timer names,
 * etc. can break the PMTM CSV output format.
 *
 * @param word [IN/OUT] The string to scan.
 */
void check_for_commas(char * word)
{
    PMTM_BOOL stripped = INTERNAL__FALSE;
    uint char_idx;
    for (char_idx = 0; char_idx < strlen(word); ++char_idx) {
        if (word[char_idx] == ',') {
            if (stripped == INTERNAL__FALSE) {
                pmtm_warn("Stripped comma(s) from string: %s", word);
            }
            stripped = INTERNAL__TRUE;
            word[char_idx] = ' ';
        }
    }
}

/**
 * Interrogates the environment variables PMTM_DATA_STORE and 
 * PMTM_KEEP_LOCAL_COPY to see whether the .pmtm file needs to be copied
 * to a central pmtm file store and whether to also keep a copy in the 
 * run directory.
 * 
 * @param instance [IN]  The instance which contains the file information
 *                       that we are copying and/or deleting
 */
void move_output_file( const struct PMTM_instance * instance )
{
//     char * pmtm_file_store = NULL;
// //     size_t len = 0;
//  

    struct stat buf;

    if (pmtm_file_store == NULL)
        pmtm_file_store = getenv("PMTM_DATA_STORE");

    if (pmtm_file_store != NULL && pmtm_file_store[0] != '\0' && no_stored_copy == PMTM_FALSE) {
        struct stat buf;
        int status = stat(pmtm_file_store, &buf);

        if (status == 0 && (buf.st_mode & S_IFDIR)) {
            char * sysname = QUOTE(SYSTEM_NAME);
            char now[20];
            time_t timer = time(NULL);
            struct tm * tm_ptr = localtime(&timer);
            strftime(now,20, "%Y%m%d-%H%M%S", tm_ptr) ;
            char num_procs[20];
            sprintf(num_procs,"%d",instance->nranks);

            char * newfilename = malloc(strlen(instance->file_name) + strlen(sysname) + 10 + strlen(now) + strlen(num_procs) + 6);
            sprintf(newfilename,"%.*s_%s_%010d_%s_%sp.pmtm",(int) strlen(instance->file_name)-5,instance->file_name,sysname,getpid(),now,num_procs);
            //printf("%s\n",newfilename);

            char * destination = malloc(strlen(pmtm_file_store) + strlen(newfilename) + 2);
            strcpy(destination,pmtm_file_store);
            strcat(destination,"/");
            strcat(destination,newfilename);
            // 	printf("%s\n",destination);

            char * copy_command = malloc(strlen(instance->file_name) + strlen(destination) + 5);
            sprintf(copy_command,"cp %s %s",instance->file_name,destination);
            system(copy_command);
        }
    }
//     else {
// 	setenv("PMTM_KEEP_LOCAL_COPY","1",1);
//     }
    
    
    char * pmtm_keep_local = NULL;
    char * pmtm_delete_local = NULL;
    pmtm_keep_local = getenv("PMTM_KEEP_LOCAL_COPY");
    pmtm_delete_local = getenv("PMTM_DELETE_LOCAL_COPY");
    int doDelete = 0;
        
    if(no_local_copy == PMTM_TRUE)
    {
	doDelete = 1;
    }
    else if (pmtm_keep_local != NULL)
    {
      if(   strncmp(pmtm_keep_local,"0",1) == 0 
	|| strncmp(toUpper(pmtm_keep_local),"FALSE",5) == 0 
	|| pmtm_keep_local[0] == '\0' 
	) 
      {
	doDelete = 1;
      }
    }
    else if (pmtm_keep_local == NULL && pmtm_delete_local != NULL)
    {
      if(   strncmp(pmtm_delete_local,"0",1) != 0 
	&& strncmp(toUpper(pmtm_delete_local),"FALSE",5) != 0 
	&& pmtm_delete_local[0] != '\0' 
	) 
      {
	doDelete = 1;  
      }
    }
    
    if(doDelete)
    {
      char * rm_command = malloc(strlen(instance->file_name) + 4);
      sprintf(rm_command,"rm %s",instance->file_name);
      system(rm_command);
    }
    
    
}

char * toUpper(char * string)
{
   char * UPPER = malloc(strlen(string) + 1);
    
   int p = 0;
   while(string[p] != '\0')
   {
      UPPER[p] = toupper(string[p]);
      p++;
   }
   UPPER[p] = '\0';
   
   return UPPER;
}

char * toLower(char * string)
{
   char * LOWER = malloc(strlen(string) + 1);
    
   int p = 0;
   while(string[p] != '\0')
   {
      LOWER[p] = toupper(string[p]);
      p++;
   }
   LOWER[p] = '\0';
   
   return LOWER;
}


#ifdef	__cplusplus
}
#endif
