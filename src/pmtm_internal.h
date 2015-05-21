/**
 * @file   pmtm_internal.h
 * @author AWE Plc.
 *
 * This file defines the structs and functions that are using in the internals
 * of PMTM. This file should _not_ be included by any user codes or by any of
 * API headers.
 */

#ifndef _PMTM_INCLUDE_PMTM_INTERNAL_H
#define	_PMTM_INCLUDE_PMTM_INTERNAL_H

#ifndef SERIAL
#  include "mpi.h"
#endif

#include "timers.h"
#include "pmtm.h"

#ifdef HW_COUNTERS
#  include "hardware_counters.h"
#endif

#include <stdio.h>

#ifdef	__cplusplus
extern "C" {
#endif

#define DEFAULT_GROUP_NAME "Default"
#define FAILED_ARRAY_ADD -1
#define FAILED_TIMER_ADD ((PMTM_timer_t) -1)
#define IO_RANK 0


extern char ** environ;

#ifdef PMTM_DEBUG
/**
 * An enum specifying the different states that a timer can be in. This is used
 * in debug mode to check that the timer is moving between valid states and that
 * it has been properly stopped before outputting.
 */
enum timer_states
{
    TIMER_STOPPED, /**< The timer is stopping. */
    TIMER_PAUSED,  /**< The timer is paused. */
    TIMER_ACTIVE   /**< The timer is running. */
};
#endif

/**
 * This struct holds the parameters that need to be stored in order to check for
 * the OUTPUT_ONCE and OUTPUT_ON_CHANGE output conditions.
 */
struct parameter
{
    char * parameter_name;  /**< The name of the parameter. */
    char * parameter_value; /**< The value of the parameter, stored as a string. */
    int rank;               /**< The rank on which the parameter was generated. */
    int count;              /**< The number of times this parameter has been stored. */
};


// OpenMP - All the next structures are linked lists. Previously they were arrays.
//          By making them linked lists the actual storage no longer moves which
//          makes it safe to avoid large scale locking in a threaded environment
//          that the array scheme would need. The actual objects are mostly accessed
//          via IDs and so the lists really only act as a memory device until the
//          final print point. None of the base storage is ever really freed until
//          the library is finalized such that baring the circumstance all pointers
//          remain valid pretty much throughout.

/**
 * This structure represents a PMTM instance. This structure keeps track of
 * whether this instance has been fully initialised and other useful info, such
 * as what file it is writing to and on what rank (if running with MPI enabled).
 */
struct PMTM_instance
{
    struct PMTM_instance * next;    /**< The next instance on the list. */

    int initialised;                /**< Whether the instance is in an initialised state. */
    char * application_name;        /**< The name of the application to write to the output file. */
    char * file_name;               /**< The name of the output file to create. */
    FILE * fid;                     /**< The file id of the opened output file. */
    int nranks;                     /**< The number of ranks in the MPI communicator, 1 if compiled in serial mode. */
    int rank;                       /**< The rank this instance was created on, 0 if compiled in serial mode. */
    size_t num_groups;              /**< The number of timer groups in the group_ids array. */
    PMTM_timer_group_t * group_ids; /**< The timer groups associated with this instance. */
    size_t num_parameters;          /**< The number of parameters that have been stored in this instance. */
    struct parameter * parameters;  /**< The array holding the parameters stored. */
};

/**
 * This structure represents a named group of timers. This is mainly for better
 * organisation of timers in the code.
 */
struct PMTM_timer_group
{
    struct PMTM_timer_group * next;  /**< The next timer group in the list. */

    struct PMTM_instance * instance; /**< The instance to which this group is associated. */
    char * group_name;               /**< The name of the timer group. */
    size_t num_timers;               /**< The number of timers in the timer_ids array. */
    struct PMTM_timer ** timer_ids;  /**< The timers associated with this timer group. Threaded timers will only carry one entry for the set. */
    size_t total_timers;             /**< The total number of timers represented by the group. All timers in thread groups are counted in this figure. */
};

/**
 * This structue keeps track of an individual timer. This includes it's name
 * and type as well as all the timing data gathered for this timer.
 */
struct PMTM_timer
{
    struct PMTM_timer * next;        /**< The next timer in the global timer list or NULL. */
    struct PMTM_timer * thread_next; /**< The next timer with the same name as this. Used for group timer_ids. */

    char * timer_name;             /**< The name of the timer. This better be unique. */
    PMTM_timer_type_t timer_type;  /**< The type of the timer, see the PMTM_TIMER_* constants in pmtm.h. */
    double last_wc;                /**< The wallclock time at the point this timer was last started/continued. */
    double last_cpu;               /**< The cpu time at the point this timer was last started/continued. */
    double current_wc;             /**< The sum of all the start->pause and continue->pause wallclock times. */
    double current_cpu;            /**< The sum of all the start->pause and continue->pause cpu times. */
    double total_wc;               /**< The total wallclock time that has been counted. */
    double total_square_wc;        /**< The total square sum of the wallclock time that has been counted (for stddev). */
    double total_cpu;              /**< The total cpu time that has been counted. */
    double total_square_cpu;       /**< The total square sum of the wallclock time that has been counted (for stddev). */
    int timer_count;               /**< The number of times this timer has been started & stopped. */
    int pause_count;               /**< The number of times this timer has been paused. */
    int frequency;                 /**< The frequency at which to take measurements. */
    int max_samples;               /**< The maximum number of measurements to take. */
    int num_samples;               /**< The number of times this timer has been stopped. */
    PMTM_BOOL ignore;              /**< Currently ignore this timer, i.e. if timer_count > max_samples. */
    int rank;                      /**< The rank of the timer, used when gathering all the timers onto rank 0. */
    PMTM_BOOL is_printed;          /**< Whether or not this timer has been printed. */
#ifdef HW_COUNTERS
    hw_counter_t * start_counters; /**< The hardware counters when this timer was started. */
    hw_counter_t * stop_counters;  /**< The hardware counters when this timer was stopped. */
    hw_counter_t * total_counters; /**< The total recorded hardware counters for this timer. */
#endif
#ifdef PMTM_DEBUG
    enum timer_states state;       /**< The state of the timer, debug mode only. */
#endif
#ifdef _OPENMP
    int thread_id;                 /**< OpenMP thread id, to allow for result amalgamation during printing, may be. */
#endif
};


/**
 * This structure provides a storage for timers for a thread. When this starts up, tail will point at head,
 * after that it will point at the next element of the last timer in the list.
 */
struct PMTM_timer_list
{
    struct PMTM_timer * head;             /**< The head of the timer list. */
    struct PMTM_timer ** tail;            /**< A pointer to the point holding the end of the list. */
};


/**
 * This structure provides a storage container for the timers. 
 */
struct PMTM_timer_store
{
    int threads;                          /**< The number of threads represented. */
    struct PMTM_timer_list * thread;      /**< An array of timer lists, one for each thread. */
};




/** @name Constructors
 @{ */
PMTM_error_t construct_instance(struct PMTM_instance * instance, const char * file_name, const char * app_name, int rank, int nranks);
PMTM_error_t construct_timer_group(struct PMTM_timer_group * group, const char * group_name);
PMTM_error_t construct_timer(struct PMTM_timer * timer, const char * timer_name, PMTM_timer_type_t timer_type);
void         construct_parameter(struct parameter * param, const char * parameter_name, const char * parameter_value, int rank);
/* @} */

/** @name Destructors
 @{ */
void destruct_instance(struct PMTM_instance * instance);
void destruct_timer_group(struct PMTM_timer_group * group);
void destruct_timer(struct PMTM_timer * timer);
void destruct_parameter(struct parameter * param);
void finalize();
/* @} */

/** @name Getters
 @{ */
struct PMTM_instance    * get_instance(const PMTM_instance_t instance_id);
struct PMTM_timer_group * get_timer_group(const PMTM_timer_group_t group_id);
struct PMTM_timer       * get_timer(const PMTM_timer_t timer_id);
struct parameter        * get_parameter(const struct PMTM_instance * instance, const char * parameter_name);
const char              * get_parameter_value(const struct PMTM_instance * instance, const char * parameter_name);
int                       get_instance_count();
/* @} */

/** @name Setters
 @{ */
int store_parameter_value(struct PMTM_instance * instance, const char * parameter_name, const char * parameter_value);
/* @} */

/** @name Array allocators
 @{ */
void * add_array_element(void ** array, int num_elements, size_t element_size);
PMTM_instance_t    new_instance();
PMTM_timer_group_t new_timer_group(struct PMTM_instance * instance);
PMTM_timer_t       new_timer(struct PMTM_timer_group * group, const char * timer_name);
struct parameter * new_parameter(struct PMTM_instance * instance);
/* @} */

/** @name Output functions
 @{ */
PMTM_error_t PMTM_internal_timer_output(struct PMTM_instance * instance, MPI_Comm PMTM_COMM);
PMTM_BOOL check_parameter(struct PMTM_instance * instance, const char * parameter_name, const char * parameter_value, PMTM_output_type_t output_type, int * count);
void print_parameter_array(struct PMTM_instance * instance, const char * parameter_name, const char * parameter_values, int num_values, int * displacements);
void print_timer(const struct PMTM_instance * instance, struct PMTM_timer * timer);
void print_overhead(const struct PMTM_instance * instance, const struct PMTM_timer * timer, uint timer_repeats);
void print_timer_array(const struct PMTM_instance * instance, uint totalthreads, struct PMTM_timer * timer_array, const char * timer_name, PMTM_timer_type_t timer_type);
/* @} */

/** @name Timing functions
 @{ */
PMTM_error_t calc_overhead(const struct PMTM_instance * instance);
void start_timer(struct PMTM_timer * timer);
void stop_timer(struct PMTM_timer * timer);
void pause_timer(struct PMTM_timer * timer);
void continue_timer(struct PMTM_timer * timer);
double get_cpu_time(struct PMTM_timer * timer);
double get_total_cpu_time(struct PMTM_timer * timer);
double get_last_cpu_time(struct PMTM_timer * timer);
double get_wc_time(struct PMTM_timer * timer);
double get_total_wc_time(struct PMTM_timer * timer);
double get_last_wc_time(struct PMTM_timer * timer);
/* @} */

/** @name Miscalaneous functions
 @{ */
void log_flags(const char ** flags, uint num_flags);
uint is_initialised();
PMTM_error_t set_file(struct PMTM_instance * instance, const char * file_name);
PMTM_error_t create_file(struct PMTM_instance * instance, const char * file_name);
PMTM_error_t write_file_header(struct PMTM_instance * instance);
PMTM_error_t get_specific_runtime_variables(const struct PMTM_instance * instance);
PMTM_error_t output_specific_runtime_variable(const struct PMTM_instance * instance, const char * envVar);
void get_file_words(const char * filename, char * wanted[], int *wIndex); //, const struct PMTM_instance * instance);
int check(char * env_vars[], char * new_var, int *indx);
void pmtm_warn(const char * message, ...);
void copy_string(char ** dest, const char * source);
const char * get_state_desc(int state);
void check_for_commas(char * string);
void move_output_file( const struct PMTM_instance * instance );
char * toUpper(char * string);
char * toLower(char * string);
/* @} */

#ifdef	__cplusplus
}
#endif

#endif	/* _PMTM_INCLUDE_PMTM_INTERNAL_H */

