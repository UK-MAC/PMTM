/// @defgroup initialization Initialization functions
/// This group contains all the subroutines needed to initialize PMTM

/// @defgroup finalization Finalization functions
/// This group contains all the subroutines needed to finalize PMTM

/// @defgroup timers Timer functions
/// This group contains all the subroutines needed to create and manage timers

/// @defgroup outputs Output functions
/// This group contains all the subroutines needed to output data captured by PMTM

/// @defgroup controls Control Functions
/// This group contains all the subroutines needed to control the usage of PMTM

/// @defgroup timer_setup Functions to setup timers
/// @ingroup timers
/// The functions needed to setup timers for use within PMTM

/// @defgroup timer_control Functions to control timers
/// @ingroup timers
/// The functions needed to control the use of timers

/// @defgroup timer_output Functions to control the output of timers
/// @ingroup timers outputs
/// The functions needed to control how output and access information stored in the timers

/// @defgroup parameters Parameter Functions 
/// @ingroup outputs
/// Functions to control what parameters are recorded by PMTM

/// @defgroup environment Environment Functions
/// @ingroup outputs
/// Functions used to output environment information to the PMTM output

/// @mainpage The Performance Modelling Timing Module
///
/// @section intro Introduction
/// The Performance Modelling Timing Module (PMTM) is a timing library that was
/// created to support cross platform timing for the Benchmark CD used to evalu-
/// ate machines prior to purchase. Currently most of the platforms tested will be
/// POSIX compliant x86 architecture however the library has been run on novel
/// architectures such as the IBM BlueGene, on which the standard POSIX timing
/// statements do not work as expected. At AWE the library is available on the
/// Linux desktops and the main HPC platforms with versions available for both
/// non-threaded and OpenMP applications.
///
/// As well as providing a standardised timing library across multiple platforms
/// the PMTM library also outputs into an easy to parse Comma Separated (CSV)
/// file which can be opened using a standard spreadsheet application (such as Mi-
/// crosoft Excel or OpenOffice.org Calc) or can be imported via the Performance
/// Modelling Analysis Tool (PMAT) into the database used to store and analyse
/// the performance modelling results.
///
/// PMTM can also output parameter information, allowing for the values of
/// parameters to be logged for a single rank or for all ranks, as well as other
/// options, such as only logging the parameter if it changes during a run.
///
/// @section using Using PMTM
/// Before PMTM can be used it must be initialised (see @ref initialization). 
/// This must be done before any any other PMTM call but after MPI has been 
/// initialised (if not using the serial version of the library).
/// Once PMTM has been initialised then timers can be created (see @ref timer_setup)
/// and used throughout the code (see @ref timer_control). When a timer is created
/// a handle to the timer is returned which can be used to start, stop and output
/// the associated timer. If the timer is used in multiple subroutines then the timer handles should be stored in a module which can be @c USE’d from those routines.
///
/// Timers can be manually output to the program’s output using the routines
/// described in @ref timer_output . As well as creating and outputting timers, PMTM can be used to log parameter values (see @ref parameters), where the
/// condition under which the parameter value is output can be chosen such that:
/// - It is only output on the first call;
/// - It is output on every call
/// - It is only output when the value has changed since the last call.
/// This is useful if the parameter being output is in a subroutine which is called multiple times but the value should not change.
///
/// Finally, when all the desired parameters have been logged and all the timers
/// stopped, the PMTM library should be finalised. This will cause any output files to be finished and closed and the library to free any memory it was using.
/// The finalise call will leave the library in a state where it can be safely initialised again if so desired.
/// The finalisation of PMTM can also copy the output file to a central location
/// using the environment variable @c PMTM_DATA_STORE, but it is recommended that this
/// is only set by system administrators and not individual users. 
///@c PMTM_DATE_STORE can also be set in a @c .pmtmrc file.
///
/// There is also the option not to keep a local copy of the output file using 
/// theenvironment variables @c PMTM_KEEP_LOCAL_COPY and @c PMTM_DELETE_LOCAL_COPY
/// with the behaviour:
///
/// | @c PMTM_KEEP_LOCAL_COPY | @c PMTM_DELETE_LOCAL_COPY | Local Copy |
/// | :---------------------- | :-----------------------: | ---------: |
/// | unset/NULL              | unset/NULL                | Kept       |
/// | "" or "0" or "FALSE"    | does not matter           | Deleted    |
/// | "ANYTHING ELSE"         | does not matter           | Kept       | 
/// | unset/NULL              | "" or "0" or "FALSE"      | Kept       |
/// | unset/NULL              | "ANYTHING ELSE"           | Deleted    |
///
/// “FALSE" is case insensitive, so can be something as horrid as “FaLsE” or
/// “fALsE”. “ANYTHIN ELSE" is literally anything other than 0, “” or @c FALSE.
///
/// The PMTM library has a debug build which can be used to check for correct
/// usage of the library. This will warn if timers have been used incorrectly (i.e.
/// stopping a timer that has never been started).
///
/// @b OpenMP:
/// With OpenMP, the main change to the library itself is the addition
/// of thread safety, and the ability to recognise and report on threads during final-
/// isation. The timers themselves do not change and can only time a single set of
/// serial events, but the facility is provided to coalesce timers with the same names
/// from different threads. This allows timing of regions with separate counts for
/// each involved thread.
///
/// The changes necessary for this are mostly to keep separate timer IDs for
/// each thread, and then to ensure each thread starts and stops its versions of the
/// timers as necessary to keep track. Thread private storage is likely a suitable
/// method to keep the separate timers. 
///
/// In a similar way to timer control, all subroutines in PMTM gain thread safety,
/// but not a mechanism to avoid corruption issues in the event of several threads
/// trying to act on the same object. In a parallel context then, it is left to the
/// programmer to ensure that duplicate calls acting on the same things are not
/// made at the same time. For calls inside a parallel region, an OpenMP @c master
/// or @c single region would be a suitable for making the event occur in one thread
/// only, and @c critical to serialise the action should several threads need to repeat
/// it.
///
/// @subsection initfinal Initialising and Finalising
///
/// Before any other PMTM routine is call the library must be initialised. This is
/// done using the @ref PMTM_init routine, which also opens up the PMTM file ready 
/// for writing. 
///
/// The PMTM library cannot be initialised more than once and doing so
/// will cause an error. To open multiple PMTM instances with different output files
/// (if you were timing an application library separately for example) you can use the
/// @ref PMTM_create_instance routine. A handle to the default instance,  which is 
/// created by the call to @ref PMTM_init, is given by the @c PMTM_DEFAULT_INSTANCE
/// parameter. To avoid multiple initialisations you can use the @ref PMTM_initialized
/// function, which returns whether or not PMTM has been initialised already.
///
/// When the program is finished using PMTM the library should be finalised using
/// @ref PMTM_finalize. This will cause all output files to be finished
/// and closed, and any allocated memory to be freed. To remove an instance created 
/// with @ref PMTM_create_instance and deallocate its associated memory without 
/// finalising the entire library you can use the @ref PMTM_destroy_instance routine
///
/// @b OpenMP:
/// In OpenMP, similarly, @ref PMTM_init and @ref PMTM_finalize should only be
/// called once. Also, no thread should attempt to use any PMTM feature until
/// either the initialisation completes or after finalisation has commenced. Explicit
/// or implicit barriers following the init call or preceding the finalise call would be
/// a suitable synchronisation to determine readiness.
///
/// @subsection timeset Timer Setup
///
/// PMTM timers encapsulate system time calls as well as keeping track of various
/// statistics such as the standard deviation of the time. Each timer is stored
/// in a timer group which can be used to organise the timers. When PMTM is
/// initialised a default timer group @c PMTM_DEFAULT_GROUP is created and associated
/// with the default PMTM instance which can be used to create timers without
/// needing to create any groups.
/// 
/// To create additional timer groups use the @ref PMTM_create_timer_group routine and the
/// @ref PMTM_create_timer routine to create the /// timers themselves.
///
/// Once a timer has been created you can modify some of its sampling prop-
/// erties via the @ref PMTM_set_sample_mode routine. This routine allows
/// you to modify how often the timer will sample and how many times it will sample
/// before any further timing calls with the timer are ignored. These properties are
/// useful when wanting to time a routine that is called millions of times but whose
/// time per call is not thought to fluctuate much between each call. If this routine
/// is not used the default sample mode is used which is to sample every time, and
/// to have no maximum sample limit.
///
/// @b OpenMP:
/// Under OpenMP, for timers expected to have separate counts for each
/// thread, it is best to make the stored ID a thread private variable, as demonstrated
/// in the examples section, and then allow each thread to perform the same actions of
/// creating, then starting and stopping the thread local timer. Providing all local 
/// timers have the same name, the report will group them and show relevant
/// separated counts for each thread.
///
/// Currently, there is no way to separate timers for codes using the single
/// line <b>“parallel do”</b> or <b>“parallel for”</b> directives unless it is satisfactory 
/// to time separate iterations accumulated together. In this situation, prior to the
/// directive the threads are not active and inside visibility is only to individual 
/// iterations not the threads’ total activity. Separating the directive is one 
/// possible solution to this; the above example shows this.
///
/// @subsection timecont Timer Control
///
/// The routines in @ref timer_control are used to control the state of the timers. A 
/// timer must be started and stopped for a timing block to be counted and can only be 
/// paused after it has been started and continued after it has been paused for the 
/// time to be valid. These states are not checked in the optimised version but there 
/// is a debug version available in which these states are checked and any erroneous
/// states are reported.
///
/// @b OpenMP:
/// Once created, timers provide no thread protection mechanisms
/// such that no two threads should access the same timer at the same time. If
/// multiple access is needed a serialisation mechanism like a barrier or critical
/// region should separate the actions
///
/// @subsection timeout Outputting Timers
/// 
/// The results of the timers will be output when PMTM is finalised with
/// @ref PMTM_finalize or using the @ref PMTM_timer_output routine to output
/// all timers associated with a given instance. The times can also be retrieved
/// directly from the timers for general use with the calling code, i.e. to output to
/// the results file.
///
/// The @ref PMTM_get_cpu_time and @ref PMTM_get_wc_time routines retrieve the current
/// timer CPU time and wall-clock time respectively, i.e. the times recorded for all
/// timing blocks so far. The @ref PMTM_get_last_cpu_time and @ref PMTM_get_last_wc_time
/// routines retrieve the times for the last timing block only.
///
/// @subsection paramout Outputting Parameters
///
/// You may also output parameters to the performance modelling file using the
/// @ref PMTM_parameter_output routine
///
/// @subsection envout Outputting the Environment
///
/// By default PMTM examines the local environment during initialisation and outputs 
/// all the set environment variables and their values to the output file. The
/// value of this can be changed using @ref PMTM_set_option routine. To change the 
/// value of @c PMTM_OPTION_OUTPUT_ENV to either @c PMTM_TRUE or @c PMTM_FALSE.
///
/// @subsubsection specenvout Outputting Specific Environment Variables
///
/// If you want to make sure that specific environment variables are printed to the
/// output file then this can be done on three levels by adding the (case sensitive)
/// name of the variable to a @c .pmtmrc file with one variable name per line. The
/// @c .pmtmrc files can be located in :
/// - @b /etc for recording variables on a system level
/// - @b $HOME for recording variables on a user level
/// - @b $PWD for recording variables on a job level. $PWD is generally the working
/// directory.
/// If a variable name appears in several @c .pmtmrc files then it will only be output
/// once.
///
/// In addition a host code can call @ref PMTM_output_specific_runtime_variable to
/// output an @e ENVIRONMENT_VARIABLE to a given @e INSTANCE
///
/// @subsection pmtmrc The .pmtmrc File
///
/// During initialisation PMTM checks for the presence of three @c .pmtmrc files; a
/// system defined @b /etc location, the users @b homespace and the directory the code
/// is run from. The main use for the @c .pmtmrc is to set a list of Environment 
/// Variables to output.
/// 
/// It can also be used to set the options @c PMTM_DATA_STORE, @c PMTM_OPTION_OUTPUT_ENV,
/// @c PMTM_OPTION_NO_LOCAL_COPY and @c PMTM_OPTION_NO_STORED_COPY. To set one of these
/// variables add a line to the @c .pmtmrc file in either of the following formats:
///
/// `VARIABLE VALUE`
/// 
/// or
///
/// `VARIABLE=VALUE`
///