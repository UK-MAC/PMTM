/** 
 * @file   pmtm.c
 * @author AWE Plc.
 * 
 * This file defines the working of the PMTM API. Most of the actually work is
 * performed in the pmtm_internal code with these functions forwarding on the
 * calls to there. However, all MPI calls (if the code is compiled with MPI
 * enabled) are restricted to this file, with the pmtm_internal code MPI
 * agnostic.
 */

#ifndef SERIAL
#  include "mpi.h"
#endif

#include "pmtm.h"
#include "pmtm_wrapper.h"
#include "pmtm_internal.h"

#include <math.h>
#include <stdarg.h>

#ifdef HW_COUNTERS
#  include "hardware_counters.h"
#endif

#ifdef	__cplusplus
extern "C" {
#endif

#ifndef SERIAL
static MPI_Comm PMTM_COMM;
#endif

/**
 * Initialises PMTM creating all the require state for the creation of timers
 * and opening the output file ready for writing to. The output file is only
 * opened on the IO_RANK (usually Rank 0). For OpenMP, this should be called
 * once only with an appreciation of the needs of MPI calls based on the MPI
 * thread mode. Please "barrier" threads before calling other routines.
 *
 * @param file_name [IN] The name of the PMTM output file.
 * @param app_name  [IN] The name of the application that is being timed.
 * @returns PMTM_SUCCESS if successful, or one of PMTM_ERROR_* codes if not.
 */
PMTM_error_t PMTM_init(
        const char * file_name,
        const char * app_name)
{
#ifndef SERIAL
    int flag;
    MPI_Initialized(&flag);
    if (!flag) {
        pmtm_warn("MPI must be intialised before PMTM");
        return PMTM_ERROR_MPI_NOT_INITIALISED;
    }
    int mpi_error;
    mpi_error = MPI_Comm_dup(MPI_COMM_WORLD, &PMTM_COMM);
    if (mpi_error != MPI_SUCCESS) { return PMTM_ERROR_MPI_COMM_DUP_FAILED; }
#endif

    if (is_initialised()) {
        pmtm_warn("PMTM is already initialised");
        return PMTM_ERROR_ALREADY_INITIALISED;
    }

    PMTM_instance_t id = new_instance();
    if (id < 0) {
        return PMTM_ERROR_CREATE_INSTANCE_FAILED;
    }

    int rank;
    int nranks;

#ifndef SERIAL
    mpi_error = MPI_Comm_rank(PMTM_COMM, &rank);
    if (mpi_error != MPI_SUCCESS) { return PMTM_ERROR_MPI_COMM_RANK_FAILED; }
    mpi_error = MPI_Comm_size(PMTM_COMM, &nranks);
    if (mpi_error != MPI_SUCCESS) { return PMTM_ERROR_MPI_COMM_SIZE_FAILED; }
#else
    rank   = IO_RANK;
    nranks = 1;
#endif

#ifdef HW_COUNTERS
    start_counters();
#endif

    PMTM_error_t err_code;
    err_code = construct_instance(get_instance(PMTM_DEFAULT_INSTANCE), file_name, app_name, rank, nranks);

#ifndef SERIAL
    /* Ensure all procs know about any errors. */
    MPI_Bcast(&err_code, 1, MPI_INT, IO_RANK, PMTM_COMM);
#endif

    return err_code;
}

/**
 * Set the file name of the PMTM output file, creating the file if necessary.
 * If an existing file was already set (either at PMTM_init or using this
 * routine) then this file will be closed without any timing information being
 * written to it. Assumed only one thread will act on an instance in this way.
 *
 * @param instance_id [IN]  The ID of the PMTM instance.
 * @param file_name   [IN]  The name of the file which will be used as the
 *                          output file for this instance.
 */
PMTM_error_t PMTM_set_file_name(
        PMTM_instance_t instance_id,
        const char * file_name)
{
    if (!is_initialised()) {
        pmtm_warn("PMTM must be initialised before PMTM_set_file_name is called");
        return PMTM_ERROR_NOT_INITIALISED;
    }

    struct PMTM_instance * instance = get_instance(instance_id);
    if (instance == NULL) {
        return PMTM_ERROR_INVALID_INSTANCE_ID;
    }

    return set_file(instance, file_name);
}

/**
 * Print out a specific environment variable to file, controlled by 
 * the host code.
 *
 * @param instance_id     [IN]  The ID of the PMTM instance.
 * @param variable_name   [IN]  The name of the Environment Variable
 *                              whose value (if it exists) will be written
 *                              to the output File
 */
PMTM_error_t PMTM_output_specific_runtime_variable(
        PMTM_instance_t instance_id,
        const char * variable_name)
{
    if (!is_initialised()) {
        pmtm_warn("PMTM must be initialised before PMTM_output_specific_runtime_variable is called");
        return PMTM_ERROR_NOT_INITIALISED;
    }

    struct PMTM_instance * instance = get_instance(instance_id);
    if (instance == NULL) {
        return PMTM_ERROR_INVALID_INSTANCE_ID;
    }

    return output_specific_runtime_variable(instance, variable_name);
}

/**
 * Create another instance of PMTM which can be used to direct the output of
 * timers into this instance rather that the default one. The output file is
 * only opened on the IO_RANK (usually Rank 0). This routine can only be
 * called by in an MPI sensitive fashion in an OpenMP environment. Also don't
 * use the instance_id until this function has completed.
 *
 * @param instance_id [OUT] The ID of the created instance.
 * @param file_name   [IN]  The name of the file which will be used as the
 *                          output file for this instance.
 * @param app_name    [IN]  The name which will be associated with this instance.
 * @returns PMTM_SUCCESS if successful, or one of PMTM_ERROR_* codes if not.
 */
PMTM_error_t PMTM_create_instance(
        PMTM_instance_t * instance_id,
        const char * file_name,
        const char * app_name)
{
    if (!is_initialised()) {
        pmtm_warn("PMTM has not been initialised");
        return PMTM_ERROR_NOT_INITIALISED;
    }

    int rank;
    int nranks;

#ifndef SERIAL
    int mpi_error;
    mpi_error = MPI_Comm_rank(PMTM_COMM, &rank);
    if (mpi_error != MPI_SUCCESS) { return PMTM_ERROR_MPI_COMM_RANK_FAILED; }
    mpi_error = MPI_Comm_size(PMTM_COMM, &nranks);
    if (mpi_error != MPI_SUCCESS) { return PMTM_ERROR_MPI_COMM_SIZE_FAILED; }
#else
    rank   = IO_RANK;
    nranks = 1;
#endif

#ifdef _OPENMP
#pragma omp critical(pmtm)
#endif
    {
        *instance_id = new_instance();
    }

    if (*instance_id < 0) {
        return PMTM_ERROR_CREATE_INSTANCE_FAILED;
    }

    PMTM_error_t err_code;
    err_code = construct_instance(get_instance(*instance_id), file_name, app_name, rank, nranks);

#ifndef SERIAL
    /* Ensure all procs know about any errors. */
    MPI_Bcast(&err_code, 1, MPI_INT, IO_RANK, PMTM_COMM);
#endif

    return err_code;
}

/**
 * Tokenise and store the given flags to be written to the output file. This
 * function should be called _before_ PMTM_init no flags will be written.
 *
 * @param flags [IN] The flags to log to the output file.
 * @returns PMTM_SUCCESS if successful, or one of PMTM_ERROR_* codes if not.
 */
PMTM_error_t PMTM_log_flags(const char * flags)
{
    uint idx = 0;
    uint last_idx = 0;
    const char * tokens[1024];
    uint token_idx = 0;
    
    char prev_char = '\0';
    size_t flag_len = strlen(flags);

    for (idx = 0; idx < flag_len; ++idx) {
        if (flags[idx] == '-' && prev_char == ' ') {
            uint len = idx - last_idx - 1;
            if (len > 0) {
                char * token = (char *) malloc(len+1);
                strncpy(token, &flags[last_idx], len);
                token[len] = '\0';
                if (token_idx == 1024) {
                    return PMTM_ERROR_TOO_MANY_FLAGS;
                }
                tokens[token_idx] = token;
                ++token_idx;
            }
            last_idx = idx;
        } else if (idx == flag_len - 1) {
            uint len = flag_len - last_idx;
            if (len > 0) {
                char * token = (char *) malloc(len+1);
                strncpy(token, &flags[last_idx], len);
                token[len] = '\0';
                if (token_idx == 1024) {
                    return PMTM_ERROR_TOO_MANY_FLAGS;
                }
                tokens[token_idx] = token;
                ++token_idx;
            }
        } else if (flags[idx] == ' ' && prev_char == ' ') {
            last_idx = idx;
        }
        prev_char = flags[idx];
    }
    
    log_flags(tokens, token_idx);
    
    for (idx = 0; idx < token_idx; ++idx) {
        free((void *) tokens[idx]);
    }
    
    return PMTM_SUCCESS;
}

/**
 * Finalises all PMTM instances, writing any remaining timers to the output files
 * before closing the files and destroying all global state. Upon successful
 * completion of this function PMTM will be left in a state where it can be
 * reinitialised with PMTM_init. Please only call this once in an multi-threaded
 * context and ensure all use of PMTM has ceased before this call.
 *
 * @returns PMTM_SUCCESS if successful, or one of PMTM_ERROR_* codes if not.
 */
PMTM_error_t PMTM_finalize()
{
    PMTM_instance_t instance_id;
    for (instance_id = 0; instance_id < get_instance_count(); ++instance_id) {
        PMTM_error_t err_code = PMTM_timer_output(instance_id);
        if (err_code != 0 && err_code != PMTM_ERROR_INVALID_INSTANCE_ID) {
            return err_code;
        }
    }
    
#ifdef HW_COUNTERS
    stop_counters();
#endif
    
    finalize();

    return PMTM_SUCCESS;
}

/**
 * Finalize the given PMTM instance, writing any remaining timers to the output
 * file before closing it, and leaving the instance in an uninitialised state.
 *
 * @param instance_id
 * @returns PMTM_SUCCESS if successful, or one of PMTM_ERROR_* codes if not.
 */
PMTM_error_t PMTM_destroy_instance(PMTM_instance_t instance_id)
{
    PMTM_error_t err_code = PMTM_timer_output(instance_id);
    if (err_code != 0) {
        return err_code;
    }
    
    struct PMTM_instance * instance = get_instance(instance_id);
    if (instance == NULL) {
        return PMTM_ERROR_INVALID_INSTANCE_ID;
    }
    destruct_instance(instance);

    return PMTM_SUCCESS;
}

/**
 * Creates a timer group which can be used to organised the timers.
 *
 * @param instance_id    [IN]  The instance with which this group will be
 *                             associated.
 * @param timer_group_id [OUT] The id of the group created.
 * @param group_name     [IN]  The name that will be associated with this group.
 * @returns PMTM_SUCCESS if successful, or one of PMTM_ERROR_* codes if not.
 */
PMTM_error_t PMTM_create_timer_group(
        PMTM_instance_t instance_id,
        PMTM_timer_group_t * timer_group_id,
        const char * group_name)
{
    struct PMTM_instance * instance = get_instance(instance_id);
    if (instance == NULL) {
        return PMTM_ERROR_INVALID_INSTANCE_ID;
    }

    PMTM_timer_group_t id;

#ifdef _OPENMP
#pragma omp critical(pmtm)
#endif
    {
        id = new_timer_group(instance);
    }

    if (id < 0) {
        return PMTM_ERROR_CREATE_GROUP_FAILED;
    }

    int err_code = construct_timer_group(get_timer_group(id), group_name);
    if (err_code != 0) {
        return err_code;
    }
    
    *timer_group_id = id;
    return PMTM_SUCCESS;
}

/**
 * Creates a timer.
 *
 * @param timer_group_id [IN]  The timer group to which this timer will be
 *                             associated.
 * @param timer_id       [OUT] The ID of the timer created.
 * @param timer_name     [IN]  The name of the timer.
 * @param timer_type     [IN]  The type of the timer, i.e. whether the timer
 *                             will report the max across all ranks, or the
 *                             average.
 * @returns PMTM_SUCCESS if successful, or one of PMTM_ERROR_* codes if not.
 */
PMTM_error_t PMTM_create_timer(
        PMTM_timer_group_t timer_group_id,
        PMTM_timer_t * timer_id,
        const char * timer_name,
        PMTM_timer_type_t timer_type)
{
    struct PMTM_timer_group * group = get_timer_group(timer_group_id);
    if (group == NULL) {
        return PMTM_ERROR_INVALID_TIMER_GROUP_ID;
    }

    PMTM_timer_t id;

#ifdef _OPENMP
#pragma omp critical(pmtm)
#endif
    {
        id = new_timer(group, timer_name);
    }

    if (id == FAILED_TIMER_ADD) {
        return PMTM_ERROR_CREATE_TIMER_FAILED;
    }

    int err_code = construct_timer(get_timer(id), NULL, timer_type);
    if (err_code != 0) {
        return err_code;
    }

    *timer_id = id;
    return PMTM_SUCCESS;
}

/**
 * Set the sample mode of the timer. This allows the setting of how often the
 * timer should sample and whether it should stop sampling after a given number
 * of measurements.
 *
 * @param timer_id    [IN] The ID of the timer to modify.
 * @param frequency   [IN] The sample frequency to set on the timer.
 * @param max_samples [IN] The maximum sample value to set on the timer.
 * @returns PMTM_SUCCESS if successful, or one of PMTM_ERROR_* codes if not.
 */
PMTM_error_t PMTM_set_sample_mode(
        PMTM_timer_t timer_id,
        int frequency,
        int max_samples)
{
    struct PMTM_timer * timer = get_timer(timer_id);

    timer->frequency = frequency;
    timer->max_samples = max_samples;
    
    return PMTM_SUCCESS;
}

/**
 * Start a timer. The timer should be in the stopped state, and an error will
 * be reported if it is not and PMTM has been compiled in debug mode.
 *
 * @param timer_id [IN] The ID of the timer to start.
 * @returns PMTM_SUCCESS if successful, or one of PMTM_ERROR_* codes if not.
 */
void PMTM_timer_start(PMTM_timer_t timer_id)
{
    struct PMTM_timer * timer = get_timer(timer_id);
    if (timer == NULL) {
        pmtm_warn("Invalid timer id: %d", timer_id);
    }

    start_timer(timer);
}

/**
 * Stop a timer. The timer should be in the running state, and an error will
 * be reported if it is not and PMTM has been compiled in debug mode.
 *
 * @param timer_id [IN] The ID of the timer to stop.
 * @returns PMTM_SUCCESS if successful, or one of PMTM_ERROR_* codes if not.
 */
void PMTM_timer_stop(PMTM_timer_t timer_id)
{
    struct PMTM_timer * timer = get_timer(timer_id);
    if (timer == NULL) {
        pmtm_warn("Invalid timer id: %d", timer_id);
    }

    stop_timer(timer);
}

/**
 * Pause a timer. The timer should be in the running state, and an error will
 * be reported if it is not and PMTM has been compiled in debug mode.
 *
 * @param timer_id [IN] The ID of the timer to pause.
 * @returns PMTM_SUCCESS if successful, or one of PMTM_ERROR_* codes if not.
 */
void PMTM_timer_pause(PMTM_timer_t timer_id)
{
    struct PMTM_timer * timer = get_timer(timer_id);
    if (timer == NULL) {
        pmtm_warn("Invalid timer id: %d", timer_id);
    }

    pause_timer(timer);
}

/**
 * Continue a timer. The timer should be in the paused state, and an error will
 * be reported if it is not and PMTM has been compiled in debug mode.
 *
 * @param timer_id [IN] The ID of the timer to continue.
 * @returns PMTM_SUCCESS if successful, or one of PMTM_ERROR_* codes if not.
 */
void PMTM_timer_continue(PMTM_timer_t timer_id)
{
    struct PMTM_timer * timer = get_timer(timer_id);
    if (timer == NULL) {
        pmtm_warn("Invalid timer id: %d", timer_id);
    }

    continue_timer(timer);
}

/**
 * Return the CPU time elapsed since the given timer was started. If the timer
 * has not been started then this will return the current CPU time.
 *
 * @param timer_id [IN] The ID of the timer for which to retreive the time.
 * @returns the CPU time since the timer was started.
 */
double PMTM_get_cpu_time(PMTM_timer_t timer_id)
{
    struct PMTM_timer * timer = get_timer(timer_id);
    if (timer == NULL) {
        pmtm_warn("Invalid timer id: %d", timer_id);
    }

    return get_cpu_time(timer);
}

/**
 * Retreive the total cpu time stored in a given timer. The timer must be
 * stopped for the returned time to be meaningful (This is checked when PMTM is
 * compiled in DEBUG mode).
 *
 * @param timer_id [IN] The ID of the timer for which to retreive the time.
 * @returns the total CPU time stored for the timer.
 */
double PMTM_get_total_cpu_time(PMTM_timer_t timer_id)
{
    struct PMTM_timer * timer = get_timer(timer_id);
    if (timer == NULL) {
        pmtm_warn("Invalid timer id: %d", timer_id);
    }

    return get_total_cpu_time(timer);
}

/**
 * Retreive the cpu time of the last block stored in a given timer. The timer
 * must be stopped for the returned time to be meaningful (This is checked when
 * PMTM is compiled in DEBUG mode).
 *
 * @param timer_id [IN] The ID of the timer for which to retreive the time.
 * @returns the CPU time of the last block stored for the timer.
 */
double PMTM_get_last_cpu_time(PMTM_timer_t timer_id)
{
    struct PMTM_timer * timer = get_timer(timer_id);
    if (timer == NULL) {
        pmtm_warn("Invalid timer id: %d", timer_id);
    }

    return get_last_cpu_time(timer);
}

/**
 * Return the wallclock time elapsed since the given timer was started. If the timer
 * has not been started then this will return the current wallclock time.
 *
 * @param timer_id [IN] The ID of the timer for which to retreive the time.
 * @returns the wallclock time since the timer was started.
 */
double PMTM_get_wc_time(PMTM_timer_t timer_id)
{
    struct PMTM_timer * timer = get_timer(timer_id);
    if (timer == NULL) {
        pmtm_warn("Invalid timer id: %d", timer_id);
    }

    return get_wc_time(timer);
}


/**
 * Retreive the total wallclock time stored in a given timer. The timer
 * must be stopped for the returned time to be meaningful (This is checked when
 * PMTM is compiled in DEBUG mode).
 *
 * @param timer_id [IN] The ID of the timer for which to retreive the time.
 * @returns the total wallclock time stored for the timer.
 */
double PMTM_get_total_wc_time(PMTM_timer_t timer_id)
{
    struct PMTM_timer * timer = get_timer(timer_id);
    if (timer == NULL) {
        pmtm_warn("Invalid timer id: %d", timer_id);
    }

    return get_total_wc_time(timer);
}

/**
 * Retreive the wallclock time of the last block stored in a given timer. The
 * timer must be stopped for the returned time to be meaningful (This is checked
 * when PMTM is compiled in DEBUG mode).
 *
 * @param timer_id [IN] The ID of the timer for which to retreive the time.
 * @returns the wallclock time of the last block stored for the timer.
 */
double PMTM_get_last_wc_time(PMTM_timer_t timer_id)
{
    struct PMTM_timer * timer = get_timer(timer_id);
    if (timer == NULL) {
        pmtm_warn("Invalid timer id: %d", timer_id);
    }

    return get_last_wc_time(timer);
}

/**
 * Log a parameter to the output file. This function handles parameters of
 * any type that can be printed via a "printf" style format string, e.g. "%s"
 * or "%5.3f". The format string provided will be used for the conversion.
 *
 * @param instance_id    [IN] The ID of the instance to whose output file we are
 *                            writing this parameter.
 * @param parameter_name [IN] The name of the parameter.
 * @param output_type    [IN] The conditions for outputing this parameter, e.g.
 *                            PMTM_ALWAYS_OUTPUT, PMTM_OUTPUT_ONCE, etc.
 * @param for_all_ranks  [IN] Whether we are printing rank 0's value or the
 *                            value on each rank.
 * @param format_string  [IN] The printf style format string used to print the
 *                            parameter.
 * @param ...            [IN] Variadic input value which will be converted to
 *                            a string via the given format string.
 * @returns PMTM_SUCCESS if successful, or one of PMTM_ERROR_* codes if not.
 */
PMTM_error_t PMTM_parameter_output(
        PMTM_instance_t instance_id,
        const char * parameter_name,
        PMTM_output_type_t output_type,
        PMTM_BOOL for_all_ranks,
        const char * format_string,
        ...)
{
    struct PMTM_instance * instance = get_instance(instance_id);
    if (instance == NULL || instance->initialised == 0) {
        return PMTM_ERROR_INVALID_INSTANCE_ID;
    }

    const size_t value_sz = 200;
    char value_str[value_sz];

    va_list list;
    va_start(list, format_string);
    vsnprintf(value_str, value_sz, format_string, list);
    value_str[value_sz - 1] = '\0';
    va_end(list);
    
    int count;

    PMTM_BOOL should_output = check_parameter(instance, parameter_name, value_str, output_type, &count);

    char * param_name;
    if (count > 1) {
        /* Enough space for the name and number. */
        size_t str_len = strlen(parameter_name) + 10;
        param_name = (char *) malloc((str_len + 1) * sizeof(char));
        snprintf(param_name, str_len + 1, "%s%d", parameter_name, count);
        param_name[str_len] = '\0';
    } else {
        copy_string(&param_name, parameter_name);
    }

    check_for_commas(param_name);
    check_for_commas(value_str);

#ifndef SERIAL
    if (for_all_ranks == PMTM_TRUE) {
        int string_sz;
        if (should_output == PMTM_TRUE) {
            string_sz = strlen(value_str) + 1;
        } else {
            string_sz = 0;
        }
        
        /* The parameter name will the same across all the ranks but the
         * parameter value might not be. Unfortunately this is a string so we
         * first need to find out how much data will need to gathered.
         */
        int string_sizes[instance->nranks];
        int status = MPI_Gather(&string_sz,   1, MPI_INT,
                                string_sizes, 1, MPI_INT,
                                IO_RANK, PMTM_COMM);
        if (status != MPI_SUCCESS) {
            free(param_name);
            return PMTM_ERROR_MPI_GATHER_FAILED;
        }

        int displacements[instance->nranks];
        int total_sz = 0;
        if (instance->rank == IO_RANK) {
            uint rank_idx;
            for (rank_idx = 0; rank_idx < instance->nranks; ++rank_idx) {
                displacements[rank_idx] = total_sz;
                total_sz += string_sizes[rank_idx];
            }
        }

        char all_values[total_sz];

        status = MPI_Gatherv(value_str, string_sz, MPI_CHAR,
                             all_values, string_sizes, displacements, MPI_CHAR,
                             IO_RANK, PMTM_COMM);
        if (status != MPI_SUCCESS) {
            free(param_name);
            return PMTM_ERROR_MPI_GATHER_FAILED;
        }

        if (instance->rank == IO_RANK && total_sz > 0) {
            print_parameter_array(instance, param_name, all_values, instance->nranks, displacements);
        }
    } else if (instance->rank == IO_RANK && should_output == PMTM_TRUE) {
        int displacements[] = { 0 };
        print_parameter_array(instance, param_name, value_str, 1, displacements);
    }
#else
    if (should_output == PMTM_TRUE) {
        int displacements[] = { 0 };
        print_parameter_array(instance, param_name, value_str, 1, displacements);
    }
#endif

    free(param_name);

    return PMTM_SUCCESS;
}

/**
 * Print the results of the timers associated with the given instance. For OpenMP,
 * do not be affecting the contents of anything held in the instance while the
 * print is occuring otherwise expect strange results; crashes if new timers or
 * timer groups are created belonging to the same instance.
 *
 * @param instance_id [IN] The ID of the instance for whom we are printing the
 *                         timers.
 * @returns PMTM_SUCCESS if successful, or one of PMTM_ERROR_* codes if not.
 */

// The previous version used a collective to bring the data together but instead
// this uses sends and recvs since in the threaded version the amount of data can
// vary per rank, although it probably won't for most HPC applications. I'm not
// sure the impact of this in the big scheme of things or whether some tree
// scheme would be needed in very large cases. Still, I guess just see what
// happens for big cases. The recvs are done in order, as the only way to
// guarantee anything with MPI is to have a unique rank or tag specified. If
// not, it is easy for early arriving rank data to get considered as relevant.

PMTM_error_t PMTM_timer_output(PMTM_instance_t instance_id)
{
    struct PMTM_instance * instance = get_instance(instance_id);
    if (instance == NULL || instance->initialised == 0) {
        return PMTM_ERROR_INVALID_INSTANCE_ID;
    }

    uint group_idx;
    uint timer_idx;
    
    for (group_idx = 0; group_idx < instance->num_groups; ++group_idx) {
        PMTM_timer_group_t group_id = instance->group_ids[group_idx];
        struct PMTM_timer_group * group = get_timer_group(group_id);

        for (timer_idx = 0; timer_idx < group->num_timers; ++timer_idx) {
            struct PMTM_timer * timer = group->timer_ids[timer_idx];

            if (!timer->is_printed) {
#ifdef PMTM_DEBUG
                if (timer->state != TIMER_STOPPED) {
                    const char * this_state = get_state_desc(timer->state);
                    const char * good_state = get_state_desc(TIMER_STOPPED);
                    pmtm_warn("Requested wallclock time on timer %s that was in state %s, timer must be in %s state",
                            timer->timer_name, this_state, good_state);
                    return PMTM_ERROR_BAD_OUTPUT;
                }
#endif
                uint threadcount = 1;
                struct PMTM_timer * local_timers;
#ifdef _OPENMP
                struct PMTM_timer * tim = timer->thread_next;

                while (tim != NULL) {
                    tim = tim->thread_next;
                    threadcount++;
                }

                local_timers = calloc(sizeof(struct PMTM_timer), threadcount);
                int inserted = 0;
                tim = timer;

                while (tim != NULL) {
                    local_timers[inserted] = *tim;
                    local_timers[inserted].rank = instance->rank;
                    inserted++;
                    tim->is_printed = 1;
                    tim = tim->thread_next;
                }
#else
                local_timers = timer;
		local_timers->rank = instance->rank;
#endif

                uint totalthreads = 0;
                uint * all_threadcounts;
                struct PMTM_timer * all_timers;

#ifndef SERIAL
                if (instance->rank == IO_RANK)
                    all_threadcounts = calloc(sizeof(uint), instance->nranks+1);

                MPI_Gather(&threadcount, 1, MPI_INT,
                           all_threadcounts, 1, MPI_INT,
                           IO_RANK, PMTM_COMM);

                if (instance->rank != IO_RANK) {
                    MPI_Send(local_timers, threadcount*sizeof(struct PMTM_timer), MPI_BYTE,
                             IO_RANK, 0, PMTM_COMM);

                } else {
                    uint r, maxthreads = 0;

                    for (r = 0; r < instance->nranks; r++) {
                        uint threads = all_threadcounts[r];
                        if (threads > maxthreads) maxthreads = threads;
                        all_threadcounts[r] = totalthreads;
                        totalthreads = totalthreads + threads;
                    }

                    all_threadcounts[instance->nranks] = totalthreads;

                    all_timers = calloc(sizeof(struct PMTM_timer), totalthreads);
                    memcpy(all_timers + all_threadcounts[IO_RANK], local_timers, threadcount*sizeof(struct PMTM_timer));

                    for (r = 0; r < instance->nranks; r++) {
                        if (r != IO_RANK) {
                            uint offset = all_threadcounts[r];
                            uint size = all_threadcounts[r+1] - offset;

                            MPI_Recv(all_timers + offset, size*sizeof(struct PMTM_timer), MPI_BYTE,
                                     r, 0, PMTM_COMM, MPI_STATUS_IGNORE);
                        }
                    }
                }
#else
                totalthreads = threadcount;
                all_threadcounts = &totalthreads;
                all_timers = local_timers;
#endif
                uint t;

                for (t = 0; t < totalthreads; ++t) {
                    all_timers[t].timer_name = timer->timer_name;
                }
                
                if (instance->rank == IO_RANK) {
                    print_timer_array(instance, totalthreads, all_timers, timer->timer_name, timer->timer_type);
                }

#ifdef _OPENMP
                free(local_timers);
#endif
#ifndef SERIAL
                if (instance->rank == IO_RANK) {
                    free(all_threadcounts);
                    free(all_timers);
                }
#endif
            }
        }
    }

    return PMTM_SUCCESS;
}

/**
 * Returns whether or not the PMTM library has already been initialised.
 *
 * \return PMTM_TRUE if the library has already been initialised, PMTM_FALSE
 * otherwise.
 */
PMTM_BOOL PMTM_initialised() {
    if (is_initialised()) {
        return PMTM_TRUE;
    } else {
        return PMTM_FALSE;
    }
}

/**
 * Set a library option.
 *
 * @param option [IN] The option to set.
 * @param value  [IN] The value to set for the option.
 */
PMTM_error_t PMTM_set_option(PMTM_option_t option, PMTM_BOOL value)
{
    return set_option(option, value);
}

/**
 * Convert a PMTM error code into an error message.
 *
 * @param err_code [IN] The code of the error for which to get the appropriate
 *                      error message.
 * @returns The error message.
 */
const char * PMTM_get_error_message(PMTM_error_t err_code)
{   
    switch (err_code) {
        case PMTM_ERROR_ALREADY_INITIALISED:    return "PMTM can only be initialised once";
        case PMTM_ERROR_NOT_INITIALISED:        return "PMTM must be initialised before any PMTM functions are used";
        case PMTM_ERROR_FAILED_ALLOCATION:      return "Memory allocation failure";
        case PMTM_ERROR_OVERWRITING_INSTANCE:   return "Attempting to overwrite a PMTM instance";
        case PMTM_ERROR_FILE_NAME_TOO_LONG:     return "Supplied file name was too long";
        case PMTM_ERROR_INVALID_INSTANCE_ID:    return "Unknown instance id passed to function";
        case PMTM_ERROR_INVALID_TIMER_GROUP_ID: return "Uknown timer group id passed to function";
        case PMTM_ERROR_INVALID_TIMER_ID:       return "Uknown timer id passed to function";
        case PMTM_ERROR_CREATE_INSTANCE_FAILED: return "Failed to create PMTM instance";
        case PMTM_ERROR_CREATE_GROUP_FAILED:    return "Failed to create timer group";
        case PMTM_ERROR_CREATE_TIMER_FAILED:    return "Failed to create timer";
        case PMTM_ERROR_BAD_START:              return "Start called on timer that was not stopped";
        case PMTM_ERROR_BAD_STOP:               return "Stop called on timer that was not running";
        case PMTM_ERROR_BAD_PAUSE:              return "Pause called on timer that was not running";
        case PMTM_ERROR_BAD_CONTINUE:           return "Continue called on timer that was not paused";
        case PMTM_ERROR_CANNOT_CREATE_FILE:     return "Failed to create PMTM output file";
        case PMTM_ERROR_BAD_OUTPUT:             return "Output called on timer that was not stopped";
        case PMTM_ERROR_MPI_NOT_INITIALISED:    return "MPI must be initialised before PMTM";
        case PMTM_ERROR_MPI_COMM_DUP_FAILED:    return "MPI error whilst duplicating comm";
        case PMTM_ERROR_MPI_COMM_RANK_FAILED:   return "MPI error whilst getting comm rank";
        case PMTM_ERROR_MPI_COMM_SIZE_FAILED:   return "MPI error whilst getting comm size";
        case PMTM_ERROR_MPI_GATHER_FAILED:      return "MPI error whilst performing gather across ranks";
        case PMTM_ERROR_UNKNOWN_OPTION:         return "Unknown option passed to PMTM_set_option";
        default: return "Unknown error";
    }
}

#ifdef	__cplusplus
}
#endif
