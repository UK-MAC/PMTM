/** 
 * @file   pmtm_wrapper.h
 * @author AWE Plc.
 *
 * This file provides the C functions which can be called from the FORTRAN API.
 * These wrapper functions also handles any conversions that are required
 * between the two APIs.
 */

#ifndef _PMTM_INCLUDE_PMTM_WRAPPER_H
#define	_PMTM_INCLUDE_PMTM_WRAPPER_H

#include "pmtm.h"
// #include "pmtm_internal.h"
#include "f2c.h"

#ifdef	__cplusplus
extern "C" {
#endif

PMTM_error_t F2C( c_pmtm_init, C_PMTM_INIT )(const char * file_name, int * file_name_len, const char * app_name, int * app_name_len);
PMTM_error_t F2C( c_pmtm_finalize, C_PMTM_FINALIZE )();
PMTM_error_t F2C( c_pmtm_log_flags, C_PMTM_LOG_FLAGS )(const char * flags, int * flags_len);
PMTM_error_t F2C( c_pmtm_set_file_name, C_PMTM_SET_FILE_NAME )(PMTM_instance_t * instance_id, const char * file_name, int * file_name_len);
PMTM_error_t F2C( c_pmtm_create_instance, C_PMTM_CREATE_INSTANCE )(PMTM_instance_t * instance_id, const char * file_name, int * file_name_len, const char * app_name, int * app_name_len);
PMTM_error_t F2C( c_pmtm_destroy_instance, C_PMTM_DESTROY_INSTANCE )(PMTM_instance_t * instance_id);
PMTM_error_t F2C( c_pmtm_create_timer_group, C_PMTM_CREATE_TIMER_GROUP )(PMTM_instance_t * instance_id, PMTM_timer_group_t * timer_group_id, const char * group_name, int * group_name_len);
PMTM_error_t F2C( c_pmtm_create_timer, C_PMTM_CREATE_TIMER )(PMTM_timer_group_t * timer_group_id, PMTM_timer_t * timer_id, const char * timer_name, int * timer_name_len, PMTM_timer_type_t * timer_type);
PMTM_error_t F2C( c_pmtm_set_sample_mode, C_PMTM_SET_SAMPLE_MODE )(PMTM_timer_t * timer_id, int * frequency, int * max_samples);

void F2C( c_pmtm_timer_start, C_PMTM_TIMER_START )(PMTM_timer_t * timer_id);
void F2C( c_pmtm_timer_stop, C_PMTM_TIMER_STOP )(PMTM_timer_t * timer_id);
void F2C( c_pmtm_timer_pause, C_PMTM_TIMER_PAUSE )(PMTM_timer_t * timer_id);
void F2C( c_pmtm_timer_continue, C_PMTM_TIMER_CONTINUE )(PMTM_timer_t * timer_id);
PMTM_error_t F2C( c_pmtm_timer_output, C_PMTM_TIMER_OUTPUT )(PMTM_instance_t * instance_id);
double F2C( c_pmtm_get_cpu_time, C_PMTM_GET_CPU_TIME )(PMTM_timer_t * timer_id);
double F2C( c_pmtm_get_last_cpu_time, C_PMTM_GET_LAST_CPU_TIME )(PMTM_timer_t * timer_id);
double F2C( c_pmtm_get_total_cpu_time, C_PMTM_GET_TOTAL_CPU_TIME )(PMTM_timer_t * timer_id);
double F2C( c_pmtm_get_wc_time, C_PMTM_GET_WC_TIME )(PMTM_timer_t * timer_id);
double F2C( c_pmtm_get_last_wc_time, C_PMTM_GET_LAST_WC_TIME )(PMTM_timer_t * timer_id);
double F2C( c_pmtm_get_total_wc_time, C_PMTM_GET_TOTAL_WC_TIME )(PMTM_timer_t * timer_id);

PMTM_error_t F2C( c_pmtm_output_specific_runtime_variable, C_PMTM_OUTPUT_SPECIFIC_RUNTIME_VARIABLE )(PMTM_instance_t * instance_id, const char * variable_name, int * variable_name_len);
PMTM_error_t F2C( c_pmtm_parameter_output_s, C_PMTM_PARAMETER_OUTPUT_S )(PMTM_instance_t * instance_id, const char * parameter_name, int * parameter_name_len, PMTM_output_type_t * output_type, int * for_all_ranks, const char * format_string, int * format_string_len, const char * parameter_value, int * parameter_value_len);
PMTM_error_t F2C( c_pmtm_parameter_output_i, C_PMTM_PARAMETER_OUTPUT_S )(PMTM_instance_t * instance_id, const char * parameter_name, int * parameter_name_len, PMTM_output_type_t * output_type, int * for_all_ranks, const char * format_string, int * format_string_len, int * parameter_value);
PMTM_error_t F2C( c_pmtm_parameter_output_l, C_PMTM_PARAMETER_OUTPUT_S )(PMTM_instance_t * instance_id, const char * parameter_name, int * parameter_name_len, PMTM_output_type_t * output_type, int * for_all_ranks, const char * format_string, int * format_string_len, long * parameter_value);
PMTM_error_t F2C( c_pmtm_parameter_output_f, C_PMTM_PARAMETER_OUTPUT_S )(PMTM_instance_t * instance_id, const char * parameter_name, int * parameter_name_len, PMTM_output_type_t * output_type, int * for_all_ranks, const char * format_string, int * format_string_len, float * parameter_value);
PMTM_error_t F2C( c_pmtm_parameter_output_d, C_PMTM_PARAMETER_OUTPUT_S )(PMTM_instance_t * instance_id, const char * parameter_name, int * parameter_name_len, PMTM_output_type_t * output_type, int * for_all_ranks, const char * format_string, int * format_string_len, double * parameter_value);
void F2C( c_pmtm_get_error_message, C_PMTM_GET_ERROR_MESSAGE )(char * buffer, int * buffer_len, PMTM_error_t * err_code);
PMTM_BOOL F2C( c_pmtm_initialized, C_PMTM_INITIALIZED )();

PMTM_error_t F2C( c_pmtm_set_option, C_PMTM_SET_OPTION )(
        int * option,
        int * value)
{
    return PMTM_set_option(*option, *value);
}

PMTM_error_t F2C( c_pmtm_init, C_PMTM_INIT )(
        const char * file_name,
        int        * file_name_len,
        const char * app_name,
        int        * app_name_len)
{
    char c_file_name[*file_name_len + 1];
    char c_app_name[*app_name_len + 1];
    F2C_strcpy(c_file_name, file_name, *file_name_len);
    F2C_strcpy(c_app_name, app_name, *app_name_len);
    return PMTM_init(c_file_name, c_app_name);
}

PMTM_error_t F2C( c_pmtm_finalize, C_PMTM_FINALIZE )()
{
    return PMTM_finalize();
}

PMTM_error_t F2C( c_pmtm_log_flags, C_PMTM_LOG_FLAGS )(
        const char * flags,
        int        * flags_len)
{
    char c_flags[*flags_len + 1];
    F2C_strcpy(c_flags, flags, *flags_len);
    return PMTM_log_flags(c_flags);
}

PMTM_error_t F2C( c_pmtm_set_file_name, C_PMTM_SET_FILE_NAME )(
        PMTM_instance_t * instance_id,
        const char * file_name,
        int * file_name_len)
{
    char c_file_name[*file_name_len + 1];
    F2C_strcpy(c_file_name, file_name, *file_name_len);
    return PMTM_set_file_name(*instance_id, c_file_name);
}

PMTM_error_t F2C( c_pmtm_create_instance, C_PMTM_CREATE_INSTANCE )(
        PMTM_instance_t * instance_id,
        const char      * file_name,
        int             * file_name_len,
        const char      * app_name,
        int             * app_name_len)
{
    char c_file_name[*file_name_len + 1];
    char c_app_name[*app_name_len + 1];
    F2C_strcpy(c_file_name, file_name, *file_name_len);
    F2C_strcpy(c_app_name, app_name, *app_name_len);
    return PMTM_create_instance(instance_id, c_file_name, c_app_name);
}

PMTM_error_t F2C( c_pmtm_destroy_instance, C_PMTM_DESTROY_INSTANCE )(
        PMTM_instance_t * instance_id)
{
    return PMTM_destroy_instance(*instance_id);
}

PMTM_error_t F2C( c_pmtm_create_timer_group, C_PMTM_CREATE_TIMER_GROUP )(
        PMTM_instance_t    * instance_id,
        PMTM_timer_group_t * timer_group_id,
        const char         * group_name,
        int                * group_name_len)
{
    char c_group_name[*group_name_len + 1];
    F2C_strcpy(c_group_name, group_name, *group_name_len);
    return PMTM_create_timer_group(*instance_id, timer_group_id, c_group_name);
}

PMTM_error_t F2C( c_pmtm_create_timer, C_PMTM_CREATE_TIMER )(
        PMTM_timer_group_t * timer_group_id,
        PMTM_timer_t       * timer_id,
        const char         * timer_name,
        int                * timer_name_len,
        PMTM_timer_type_t  * timer_type)
{
    char c_timer_name[*timer_name_len + 1];
    F2C_strcpy(c_timer_name, timer_name, *timer_name_len);
    return PMTM_create_timer(*timer_group_id, timer_id, c_timer_name, *timer_type);
}

PMTM_error_t F2C( c_pmtm_set_sample_mode, C_PMTM_SET_SAMPLE_MODE )(
        PMTM_timer_t * timer,
        int          * frequency,
        int          * max_samples)
{
    return PMTM_set_sample_mode(*timer, *frequency, *max_samples);
}

void F2C( c_pmtm_timer_start, C_PMTM_TIMER_START )(
        PMTM_timer_t * timer_id)
{
    PMTM_timer_start(*timer_id);
}

void F2C( c_pmtm_timer_stop, C_PMTM_TIMER_STOP )(
        PMTM_timer_t * timer_id)
{
    PMTM_timer_stop(*timer_id);
}

void F2C( c_pmtm_timer_pause, C_PMTM_TIMER_PAUSE )(
        PMTM_timer_t * timer_id)
{
    PMTM_timer_pause(*timer_id);
}

void F2C( c_pmtm_timer_continue, C_PMTM_TIMER_CONTINUE )(
        PMTM_timer_t * timer_id)
{
    PMTM_timer_continue(*timer_id);
}

PMTM_error_t F2C( c_pmtm_timer_output, C_PMTM_TIMER_OUTPUT )(
        PMTM_instance_t * instance_id)
{
    return PMTM_timer_output(*instance_id);
}

double F2C( c_pmtm_get_cpu_time, C_PMTM_GET_CPU_TIME )(
        PMTM_timer_t * timer_id)
{
    return PMTM_get_cpu_time(*timer_id);
}

double F2C( c_pmtm_get_last_cpu_time, C_PMTM_GET_LAST_CPU_TIME )(
        PMTM_timer_t * timer_id)
{
    return PMTM_get_last_cpu_time(*timer_id);
}

double F2C( c_pmtm_get_total_cpu_time, C_PMTM_GET_TOTAL_CPU_TIME )(
        PMTM_timer_t * timer_id)
{
    return PMTM_get_total_cpu_time(*timer_id);
}

double F2C( c_pmtm_get_wc_time, C_PMTM_GET_WC_TIME )(
        PMTM_timer_t * timer_id)
{
    return PMTM_get_wc_time(*timer_id);
}

double F2C( c_pmtm_get_last_wc_time, C_PMTM_GET_LAST_WC_TIME )(
        PMTM_timer_t * timer_id)
{
    return PMTM_get_last_wc_time(*timer_id);
}

double F2C( c_pmtm_get_total_wc_time, C_PMTM_GET_TOTAL_WC_TIME )(
        PMTM_timer_t * timer_id)
{
    return PMTM_get_total_wc_time(*timer_id);
}

PMTM_error_t F2C( c_pmtm_output_specific_runtime_variable, C_PMTM_OUTPUT_SPECIFIC_RUNTIME_VARIABLE )(
        PMTM_instance_t * instance_id,
        const char * variable_name,
        int * variable_name_len)
{
    char c_variable_name[*variable_name_len + 1];
    F2C_strcpy(c_variable_name, variable_name, *variable_name_len);
    return PMTM_output_specific_runtime_variable(*instance_id, c_variable_name);
}

PMTM_error_t F2C( c_pmtm_parameter_output_s, C_PMTM_PARAMETER_OUTPUT_S )(
        PMTM_instance_t    * instance_id,
        const char         * parameter_name,
        int                * parameter_name_len,
        PMTM_output_type_t * output_type,
        int                * for_all_ranks,
        const char         * format_string,
        int                * format_string_len,
        const char         * parameter_value,
        int                * parameter_value_len)
{
    char c_parameter_name[*parameter_name_len + 1];
    char c_format_string[*format_string_len + 1];
    char c_parameter_value[*parameter_value_len + 1];
    F2C_strcpy(c_parameter_name,  parameter_name,  *parameter_name_len);
    F2C_strcpy(c_format_string,   format_string,   *format_string_len);
    F2C_strcpy(c_parameter_value, parameter_value, *parameter_value_len);
    return PMTM_parameter_output(*instance_id, c_parameter_name, *output_type, (PMTM_BOOL) *for_all_ranks, c_format_string, c_parameter_value);
}

#define C_PMTM_PARAMETER_OUTPUT(letter, LETTER, type) \
    PMTM_error_t F2C( c_pmtm_parameter_output_##letter, C_PMTM_PARAMETER_OUTPUT_##LETTER )( \
            PMTM_instance_t    * instance_id, \
            const char         * parameter_name, \
            int                * parameter_name_len, \
            PMTM_output_type_t * output_type, \
            int                * for_all_ranks, \
            const char         * format_string, \
            int                * format_string_len, \
            type               * parameter_value) \
    { \
        char c_parameter_name[*parameter_name_len + 1]; \
        char c_format_string[*format_string_len + 1]; \
        F2C_strcpy(c_parameter_name, parameter_name, *parameter_name_len); \
        F2C_strcpy(c_format_string,  format_string,  *format_string_len); \
        return PMTM_parameter_output(*instance_id, c_parameter_name, *output_type, (PMTM_BOOL) *for_all_ranks, c_format_string, *parameter_value); \
    }

C_PMTM_PARAMETER_OUTPUT(i, I, int)
C_PMTM_PARAMETER_OUTPUT(l, L, long)
C_PMTM_PARAMETER_OUTPUT(d, D, double)
C_PMTM_PARAMETER_OUTPUT(f, F, float)

void F2C( c_pmtm_get_error_message, C_PMTM_GET_ERROR_MESSAGE )(
        char         * buffer,
        int          * buffer_len,
        PMTM_error_t * err_code)
{
    const char * message = PMTM_get_error_message(*err_code);
    if (strlen(message) > *buffer_len) {
        pmtm_warn("Error message is too long to fit into given buffer");
    }

    char * tmp_msg = (char *) malloc(*buffer_len * sizeof(char));
    strncpy(tmp_msg, message, *buffer_len);
    tmp_msg[*buffer_len - 1] = '\0';
    C2F_strcpy(buffer, *buffer_len, tmp_msg);
    free(tmp_msg);
}

PMTM_BOOL F2C( c_pmtm_initialized, C_PMTM_INITIALIZED )()
{
    return PMTM_initialised();
}

#ifdef	__cplusplus
}
#endif

#endif	/* _PMTM_INCLUDE_PMTM_WRAPPER_H */

