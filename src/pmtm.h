/**
 * @file   pmtm.h
 * @author AWE Plc.
 *
 * This file defines the PMTM C API. See the User Guide for more information on
 * using PMTM.
 */

#ifndef _PMTM_INCLUDE_PMTM_H
#define	_PMTM_INCLUDE_PMTM_H

#include <stdlib.h>
#include "pmtm_defines.h"

#ifdef	__cplusplus
extern "C" {
#endif

/**
 * @page Error Error Codes 
 *
 * | Error Name | Value | Description | 
 * | :----------| :---: | :---------- | 
 * | PMTM_SUCCESS | 0 | Success value. \b Not an error code. |
 * |  PMTM_ERROR_ALREADY_INITIALISED     | -1  | PMTM can only be initialised once. |
 * |  PMTM_ERROR_NOT_INITIALISED         | -2  | PMTM must be initialised before any PMTM functions are used. |
 * |  PMTM_ERROR_FAILED_ALLOCATION       | -3  | Memory allocation failure. |
 * |  PMTM_ERROR_OVERWRITING_INSTANCE    | -4  | Attempting to overwrite a PMTM instance. |
 * |  PMTM_ERROR_FILE_NAME_TOO_LONG      | -5  | Supplied file name was too long. |
 * |  PMTM_ERROR_INVALID_INSTANCE_ID     | -6  | Unknown instance id passed to function. |
 * |  PMTM_ERROR_INVALID_TIMER_GROUP_ID  | -7  | Unknown timer group id passed to function. |
 * |  PMTM_ERROR_INVALID_TIMER_ID        | -8  | Unknown timer id passed to function. | 
 * |  PMTM_ERROR_CREATE_INSTANCE_FAILED  | -9  | Failed to create PMTM instance. |
 * |  PMTM_ERROR_CREATE_GROUP_FAILED     | -10 | Failed to create timer group. |
 * |  PMTM_ERROR_CREATE_TIMER_FAILED     | -11 | Failed to create timer. |
 * |  PMTM_ERROR_BAD_START               | -12 | Start called on timer that was not stopped. |
 * |  PMTM_ERROR_BAD_STOP                | -13 | Stop called on timer that was not running. |
 * |  PMTM_ERROR_BAD_PAUSE               | -14 | Pause called on timer that was not running. |
 * |  PMTM_ERROR_BAD_CONTINUE            | -15 | Continue called on timer that was not paused. |
 * |  PMTM_ERROR_CANNOT_CREATE_FILE      | -16 | Failed to create PMTM output file. |
 * |  PMTM_ERROR_BAD_OUTPUT              | -17 | Output called on timer that was not stopped. |
 * |  PMTM_ERROR_MPI_NOT_INITIALISED     | -18 | MPI must be initialised before PMTM. |
 * |  PMTM_ERROR_MPI_COMM_RANK_FAILED    | -19 | MPI error whilst getting comm rank. |
 * |  PMTM_ERROR_MPI_COMM_SIZE_FAILED    | -20 | MPI error whilst getting comm size. |
 * |  PMTM_ERROR_MPI_GATHER_FAILED       | -21 | MPI error whilst performing gather across ranks. |
 * |  PMTM_ERROR_TOO_MANY_FLAGS          | -22 | Too many flags have been given to \ref PMTM_log_flags, not all of them will be output. |
 * |  PMTM_ERROR_MPI_COMM_DUP_FAILED     | -23 | MPI error whilst duplicating communicator. |
 * |  PMTM_ERROR_HW_COUNTERS_INIT_FAILED | -24 | Error whilst trying to initialise hardware counters. |
 * |  PMTM_ERROR_HW_COUNTERS_READ_FAILED | -25 | Error whilst trying to read hardware counters. |
 * |  PMTM_ERROR_UNKNOWN_OPTION          | -26 | Unknown PMTM Error - Should never return this. |
 @{ */
#define PMTM_SUCCESS                        0
#define PMTM_ERROR_ALREADY_INITIALISED     -1
#define PMTM_ERROR_NOT_INITIALISED         -2
#define PMTM_ERROR_FAILED_ALLOCATION       -3
#define PMTM_ERROR_OVERWRITING_INSTANCE    -4
#define PMTM_ERROR_FILE_NAME_TOO_LONG      -5
#define PMTM_ERROR_INVALID_INSTANCE_ID     -6
#define PMTM_ERROR_INVALID_TIMER_GROUP_ID  -7
#define PMTM_ERROR_INVALID_TIMER_ID        -8
#define PMTM_ERROR_CREATE_INSTANCE_FAILED  -9
#define PMTM_ERROR_CREATE_GROUP_FAILED     -10
#define PMTM_ERROR_CREATE_TIMER_FAILED     -11
#define PMTM_ERROR_BAD_START               -12
#define PMTM_ERROR_BAD_STOP                -13
#define PMTM_ERROR_BAD_PAUSE               -14
#define PMTM_ERROR_BAD_CONTINUE            -15
#define PMTM_ERROR_CANNOT_CREATE_FILE      -16
#define PMTM_ERROR_BAD_OUTPUT              -17
#define PMTM_ERROR_MPI_NOT_INITIALISED     -18
#define PMTM_ERROR_MPI_COMM_RANK_FAILED    -19
#define PMTM_ERROR_MPI_COMM_SIZE_FAILED    -20
#define PMTM_ERROR_MPI_GATHER_FAILED       -21
#define PMTM_ERROR_TOO_MANY_FLAGS          -22
#define PMTM_ERROR_MPI_COMM_DUP_FAILED     -23
#define PMTM_ERROR_HW_COUNTERS_INIT_FAILED -24
#define PMTM_ERROR_HW_COUNTERS_READ_FAILED -25
#define PMTM_ERROR_UNKNOWN_OPTION          -26
/* @} */

#ifdef __cplusplus
typedef bool PMTM_BOOL;
#else
typedef int PMTM_BOOL;
#endif

// OpenMP - the Fortran API needs the top two to be integers, PMTM_timer_t is
//          a structure in Fortran too so it is okay to populate as best needed.
//          So, a pointer should be fine.

typedef int PMTM_instance_t;
typedef int PMTM_timer_group_t;
typedef struct PMTM_timer * PMTM_timer_t;
typedef int PMTM_error_t;
typedef int PMTM_option_t;

typedef int PMTM_timer_type_t;
typedef int PMTM_output_type_t;

/** @name Initialisation functions
 @{ */
PMTM_error_t PMTM_init(const char * file_name, const char * application_name);
PMTM_error_t PMTM_finalize();
PMTM_error_t PMTM_destroy_instance(PMTM_instance_t instance_id);
PMTM_error_t PMTM_create_instance(PMTM_instance_t * instance_id, const char * file_name, const char * application_name);
PMTM_error_t PMTM_create_timer_group(PMTM_instance_t instance_id, PMTM_timer_group_t * timer_group_id, const char * group_name);
PMTM_error_t PMTM_create_timer(PMTM_timer_group_t timer_group_id, PMTM_timer_t * timer, const char * timer_name, PMTM_timer_type_t timer_type);
PMTM_error_t PMTM_log_flags(const char * flags);
PMTM_error_t PMTM_set_file_name(PMTM_instance_t instance_id, const char * file_name);
/* @} */

/** @name Timer functions
 @{ */
void PMTM_timer_start(PMTM_timer_t timer_id);
void PMTM_timer_stop(PMTM_timer_t timer_id);
void PMTM_timer_pause(PMTM_timer_t timer_id);
void PMTM_timer_continue(PMTM_timer_t timer_id);
PMTM_error_t PMTM_timer_output(PMTM_instance_t instance_id);
PMTM_error_t PMTM_set_sample_mode(PMTM_timer_t timer_id, int sample_freq, int sample_max);
double PMTM_get_cpu_time(PMTM_timer_t timer);
double PMTM_get_total_cpu_time(PMTM_timer_t timer);
double PMTM_get_last_cpu_time(PMTM_timer_t timer);
double PMTM_get_wc_time(PMTM_timer_t timer);
double PMTM_get_total_wc_time(PMTM_timer_t timer);
double PMTM_get_last_wc_time(PMTM_timer_t timer);
/* @} */

/** @name Parameter functions
 @{ */
PMTM_error_t PMTM_parameter_output(PMTM_instance_t instance_id, const char * parameter_name, PMTM_output_type_t output_type, PMTM_BOOL for_all_ranks, const char * format_string, ...);
PMTM_error_t PMTM_output_specific_runtime_variable(PMTM_instance_t instance_id, const char * variable_name);
/* @} */

/** @name Misc functions
 @{ */
PMTM_error_t PMTM_set_option(PMTM_option_t option, PMTM_BOOL value);
const char * PMTM_get_error_message(PMTM_error_t err_code);
PMTM_BOOL PMTM_initialised();
/* @} */

/** @name Constants
 @{ */
extern const PMTM_instance_t PMTM_DEFAULT_INSTANCE; /*!< The ID of the default PMTM instance. */
extern const PMTM_timer_group_t PMTM_DEFAULT_GROUP; /*!< The ID of the default timer group in each instance. */
extern const PMTM_timer_t PMTM_NULL_TIMER;          /*!< The ID of a timer that has not been created. */

extern const PMTM_timer_type_t PMTM_TIMER_NRK;  /*!< Do not print any rank timers, intended only for use internal to PMTM. */
extern const PMTM_timer_type_t PMTM_TIMER_NONE; /*!< Only print the rank timers with no additional statistics. */
extern const PMTM_timer_type_t PMTM_TIMER_MAX;  /*!< Print the rank timers as well as the maximum across the ranks. */
extern const PMTM_timer_type_t PMTM_TIMER_MIN;  /*!< Print the rank timers as well as the minimum across the ranks. */
extern const PMTM_timer_type_t PMTM_TIMER_AVG;  /*!< Print the rank timers as well as the average across the ranks. */
extern const PMTM_timer_type_t PMTM_TIMER_ALL;  /*!< Print the rank timers as well as the maximum, minimum and average.*/
extern const PMTM_timer_type_t PMTM_TIMER_MMA;  /*!< Print the maximum, minimum and average but not the rank timers. */
extern const PMTM_timer_type_t PMTM_TIMER_INT;  /*!< Do no print anything to the output file. Used for internal timers only. */
extern const PMTM_timer_type_t PMTM_TIMER_AVO;  /*!< Only print the average time for a timer. */

extern const PMTM_output_type_t PMTM_OUTPUT_ALWAYS;    /*!< Always output the parameter. */
extern const PMTM_output_type_t PMTM_OUTPUT_ON_CHANGE; /*!< Only output the parameter if it has changed since the last output. */
extern const PMTM_output_type_t PMTM_OUTPUT_ONCE;      /*!< Only output the parameter on the first call of the function. */

extern const int PMTM_NO_MAX;       /*!< Specify the timer to have no maximum number of samples. */
extern const int PMTM_DEFAULT_FREQ; /*!< Specify that the timer should sample at the default rate. */
extern const int PMTM_DEFAULT_MAX;  /*!< Specify the timer should stop after the default number of samples. */

//extern const PMTM_BOOL PMTM_TRUE;  /*!< "True" as returned and used in PMTM. */
//extern const PMTM_BOOL PMTM_FALSE; /*!< "False" as returned and used in PMTM. */
#define PMTM_TRUE INTERNAL__TRUE
#define PMTM_FALSE INTERNAL__FALSE

//extern const PMTM_option_t PMTM_OPTION_OUTPUT_ENV; /*!< Sets whether or not to output the environment in the PMTM header. */
//extern const PMTM_option_t PMTM_OPTION_NO_LOCAL_COPY; /*!< Sets whether or not to keep the local copy of an output file. */
//extern const PMTM_option_t PMTM_OPTION_NO_STORED_COPY; /*!< Sets whether or not to create a copy of the output file in the PMTM_DATA_STORE location. */
#define PMTM_OPTION_OUTPUT_ENV INTERNAL__OPTION_OUTPUT_ENV
#define PMTM_OPTION_NO_LOCAL_COPY INTERNAL__OPTION_NO_LOCAL_COPY
#define PMTM_OPTION_NO_STORED_COPY INTERNAL__OPTION_NO_STORED_COPY
/* @} */

#ifdef	__cplusplus
}
#endif

#endif	/* _PMTM_INCLUDE_PMTM_H */

