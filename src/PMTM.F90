!>
!! File:   PMTM.F90
!! Author: AWE Plc.
!!
!! This file defines the FORTRAN API of PMTM. These routines call the C code to perform the actual timing.

#include "pmtm_defines.h"

module PMTM
    USE, INTRINSIC :: ISO_C_BINDING
    implicit none
    private

    public :: PMTM_init,                             & 
              PMTM_finalize,                         &
              PMTM_log_flags,                        &  
              PMTM_initialized,                      &
              PMTM_create_instance,                  &
              PMTM_destroy_instance,                 &
              PMTM_create_timer_group,               &
              PMTM_create_timer,                     &
              PMTM_timer_start,                      &
              PMTM_timer_stop,                       &
              PMTM_timer_pause,                      &
              PMTM_timer_continue,                   &
              PMTM_timer_output,                     &
              PMTM_get_cpu_time,                     &
              PMTM_get_last_cpu_time,                &
              PMTM_get_total_cpu_time,               &
              PMTM_get_wc_time,                      &
              PMTM_get_last_wc_time,                 &
              PMTM_get_total_wc_time,                &
              PMTM_parameter_output,                 & 
              PMTM_set_sample_mode,                  &
              PMTM_get_error_message,                &
              PMTM_set_file_name,                    &
              PMTM_output_specific_runtime_variable, &
              PMTM_set_option,                       &
              pmtm_timer

    integer, public, parameter :: PMTM_SUCCESS           	= 0 !< Handle for returning a success in PMTM
    integer, public, parameter :: PMTM_DEFAULT_GROUP     	= INTERNAL__DEFAULT_GROUP !< Reference handle to the default group
    integer, public, parameter :: PMTM_DEFAULT_INSTANCE  	= INTERNAL__DEFAULT_INSTANCE !< Reference handle to the default instance
    integer, public, parameter :: PMTM_TIMER_NONE        	= INTERNAL__TIMER_NONE !< Handle for setting the timer type to only output rank information
    integer, public, parameter :: PMTM_TIMER_MAX         	= INTERNAL__TIMER_MAX !< Handle for setting the timer type to output rank and maximum information
    integer, public, parameter :: PMTM_TIMER_MIN         	= INTERNAL__TIMER_MIN !< Handle for setting the timer type to output rank and minimum information
    integer, public, parameter :: PMTM_TIMER_AVG         	= INTERNAL__TIMER_AVG !< Handle for setting the timer type to output rank and average information
    integer, public, parameter :: PMTM_TIMER_ALL         	= IOR(IOR(INTERNAL__TIMER_MAX, INTERNAL__TIMER_MIN), INTERNAL__TIMER_AVG) !< Handle for setting the timer type to output rank, maximum, minimum and average information
    integer, public, parameter :: PMTM_TIMER_MMA         	= IOR(IOR(IOR(INTERNAL__TIMER_MMA, INTERNAL__TIMER_MAX), INTERNAL__TIMER_MIN), INTERNAL__TIMER_AVG) !< Handle for setting timer type to output just the maximum, minimum and average information
    integer, public, parameter :: PMTM_TIMER_INT         	= INTERNAL__TIMER_INT !< Handle for setting timer type to output nothing to file
    integer, public, parameter :: PMTM_TIMER_AVO         	= INTERNAL__TIMER_AVO !< Handle for setting timer type to output only the Average time across all ranks and nothing else
    integer, public, parameter :: PMTM_OUTPUT_ALWAYS     	= INTERNAL__OUTPUT_ALWAYS !< Handle to set a parameter to be output everytime \ref PMTM_parameter_output is called
    integer, public, parameter :: PMTM_OUTPUT_ON_CHANGE  	= INTERNAL__OUTPUT_ON_CHANGE !< Handle to set a parameter to be output only if it has changed since last called
    integer, public, parameter :: PMTM_OUTPUT_ONCE       	= INTERNAL__OUTPUT_ONCE !< Handle to set a parameter to be output only on the first call to \ref PMTM_parameter_output
    integer, public, parameter :: PMTM_NO_MAX            	= INTERNAL__NO_MAX !< Parameter to use if there is no maximum number of samples for a timer 
    integer, public, parameter :: PMTM_OPTION_OUTPUT_ENV 	= INTERNAL__OPTION_OUTPUT_ENV !< Parameter to set to decide whether or not to output the environment to file (Default: YES)
    integer, public, parameter :: PMTM_OPTION_NO_LOCAL_COPY     = INTERNAL__OPTION_NO_LOCAL_COPY !< Parameter to set to decide whether or not to delete the local copy of the output file or not (Default: NO)
    integer, public, parameter :: PMTM_OPTION_NO_STORED_COPY	= INTERNAL__OPTION_NO_STORED_COPY !< Parameter to set to decide whether or not to create a remote copy of the output file (Default: NO)
    
!    integer, parameter :: pmtm_timerk           = 4
   
!    type pmtm_timer
!        integer(pmtm_timerk) :: handle = INTERNAL__NULL_TIMER
!    end type

    ! Thread private variables are not allowed initializers, I really hope no app is
    ! assuming they are set.
   
    type pmtm_timer
        type(C_PTR) :: handle ! = C_NULL_PTR
    end type
   

!> \section PMTM_parameter_output
!! @addtogroup parameters
!!
!! <b><code>public pmtm::PMTM_parameter_output (instance, parameter_name, output_type, for_all_ranks, parameter_value, err_code)</code></b>
!!
!! Log a parameter to the output file specified in \ref PMTM_init. This is an \b interface routine which accepts the following \c parameter_value types: 
!! \b real(4), \b real(8), \b integer(4), \b integer(8), \b logical and \b character(len=*). If any other parameter types are required please contact the 
!! PMTM code custodian
!!
!! @param instance The handle of the instance to whose output file we writing this parameter
!! @param parameter_name The name of the parameter to output
!! @param output_type The conditions for outputting this parameter, the options being:
!! - \c PMTM_OUTPUT_ALWAYS Always the output whenever this function is called
!! - \c PMTM_OUTPUT_ON_CHANGE Only output the parameter if it has chaanged since the last output call
!! - \c PMTM_OUTPUT_ONCE Only output the parameter on the first call to this routine
!! @param for_all_ranks Whether to print the parameter for all ranks (\b .TRUE. ) or just for rank 0 (\b .FALSE. )
!! @param parameter_value The value to output
!! @param err_code <b>(FORTRAN Only)</b> Will be set to \c PMTM_SUCCESS if the call was successful and the appropriate \ref Error if not
!!
!! @test <b>\c tests_parameter.cpp/for_each_rank</b>    Outputting a parameter with for_each_rank == TRUE should print the parameter on each rank
!! @test <b>\c tests_parameter.cpp/rank0_only</b>       Outputting a parameter with for_each_rank == FALSE should print the parameter on rank0 only
!! @test <b>\c tests_parameter.cpp/output_types</b>     Should be able to output a variety of different parameter types using printf style notation
!! @test <b>\c tests_parameter.cpp/output_once</b>      With the output once type the parameter should only be printed on the first call
!! @test <b>\c tests_parameter.cpp/output_on_change</b>		With the output on change type the parameter should only be printed if it changes between calls
!! @test <b>\c tests.F90/test_parameter_output</b>	Tests that calling \ref PMTM_parameter_output with different valid variable types returns \c PMTM_SUCCESS
!! @test <b>\c tests_parameter.cpp/output_always</b>	With the output always type the parameter should always be printed
!!
!! \b Notes: The \c C version of this routine takes a \c printf style format string argument, which is used to parse the parameter values at the end.
!!
    interface PMTM_parameter_output
        module procedure parameter_output_real
        module procedure parameter_output_real8
        module procedure parameter_output_integer
        module procedure parameter_output_integer8
        module procedure parameter_output_logical
        module procedure parameter_output_character
    end interface


contains


!-----------------------------------------------------------------------------------------------------------------------------------
! Set PMTM library option.
!> \section PMTM_set_option
!! Sets the value of a specific option to a boolean value
!!
!! \ingroup controls
!! @param option The handle of the option to be set. Currently, it can only use the following:
!! - \c PMTM_OPTION_OUTPUT_ENV Controls whether or not to output all the environment variables to the file specified in \ref PMTM_init
!! - \c PMTM_OPTION_NO_LOCAL_COPY Controls whether or not to keep a copy of the output file in the working directory
!! - \c PMTM_OPTION_NO_STORED_COPY Controls whether or not to create a copy of the output file in the system PMTM output store (as set by \c PMTM_DATA_STORE)
!! @param value The value to set the option to, the options being:
!! - \c PMTM_TRUE Set the option as true
!! - \c PMTM_FALSE Set the option as false
!! @param err_code <b>(FORTRAN Only)</b> Will be set to \c PMTM_SUCCESS if the call was successful and the appropriate \ref Error if not
!! 
!! @test <b>\c tests_options.cpp/output_env</b>     Turning off the output environment options should stop the environment being output to the PMTM output file
!! @test <b>\c tests.F90/test_set_option</b>	Tests that when \ref PMTM_set_option is given correct values that it returns \c PMTM_SUCCESS
!!
subroutine PMTM_set_option(option, value, err_code)
    implicit none
    integer, intent(in)  :: option
    logical, intent(in)  :: value
    integer, intent(out) :: err_code

    integer :: value_int
    integer :: c_PMTM_set_option

    if (value) then
        value_int = INTERNAL__TRUE
    else
        value_int = INTERNAL__FALSE
    endif

    err_code = c_PMTM_set_option(option, value_int)
end subroutine PMTM_set_option

!-----------------------------------------------------------------------------------------------------------------------------------
! Initialise the PMTM library.
!> \section PMTM_init
!! Initialises PMTM creating all the required state for the generation of timers and opening the output file ready for writing to.
!!
!! \ingroup initialization
!! @param file_name The name of the CSV file which PMTM will output it's results ('.pmtm' will be appended to this name and if a file already exists with this name a number
!! will be appended to create a new file)
!! @param application_name The application name that will get logged to the output file e.g. 'DL Poly 2.17'
!! @param err_code <b>(FORTRAN only)</b> Will be set to \c PMTM_SUCCESS if the call was successful and the appropriate \ref Error if not
!!
!! \b Notes: This must be called \b after \c MPI_INIT. Using the empty string "" as the \c file_name will cause no file to be created and no output to be written. 
!! This can be useful if you want to use the library's timers but do not want to output them. Using the string "-" as the \c file_name will cause all PMTM output to be 
!! written to \c stdout
!!
!! \b OpenMP In an OpenMP application, \c PMTM_init should be either called outside of a parallel region, or if inside by a single running thread only. No thread should 
!! use any PMTM facility until its completion. An explicit or implicit barriering mechanism should be inserted to ensure this if threads are active and are not being
!! synchronised in any other way.
!! 
!! @test <b>\c tests_initialize.cpp/valid_file_name</b> 	Initialising PMTM with a valid noexistant file name should create that file with 0.pmtm suffix
!! @test <b>\c tests_initialize.cpp/invalid_file_name</b> 	Initialising PMTM with an invalid file name should cause initialisation to fail
!! @test <b>\c tests_initialize.cpp/stdout</b> 	Initialising PMTM with a file name of - should cause it to write to stdout
!! @test <b>\c tests_initialize.cpp/no_output</b> 	Initialising PMTM with an empty file name should cause it to not create an output file
!! @test <b>\c tests_initialize.cpp/file_header</b> 	Initialising PMTM should cause it to write a valid PMTM file header
!! @test <b>\c tests_initialize.cpp/overheads</b> 	Initialising PMTM should cause it to output timer overhead information into the output file
!! @test <b>\c tests_initialize.cpp/existing_file</b> 	Initialising PMTM with a valid but existing file name should create that file with an incremented suffix
!! @test <b>\c tests_options.cpp/get_specific_runtime_variables</b>	Test whether putting a specific Environment variable name in ${PWD}/.pmtmrc prints it to the output, but only once
!! @test <b>\c tests_options.cpp/from_pmtmrc_file</b>       Test whether putting a option pair in either VARIABLE VALUE or VARIABLE=VALUE form in .pmtmrc file results in change to option
!! @test <b>\c tests.F90/test_init</b>			Tests that when \ref PMTM_init is called with valid arguments that it returns \c PMTM_SUCCESS
!!
subroutine PMTM_init(file_name, application_name, err_code)
    implicit none
    character(len=*), intent(in) :: file_name
    character(len=*), intent(in) :: application_name
    integer, intent(out)         :: err_code

    integer :: c_PMTM_init
    err_code = c_PMTM_init(file_name, len_trim(file_name), application_name, len_trim(application_name))
end subroutine PMTM_init

!-----------------------------------------------------------------------------------------------------------------------------------
! Finalize the PMTM library.
!> \section PMTM_finalize
!! Finalises all PMTM instances, writing any remaining timers to the output files before closing them and destroying all global state. Upon successful completion of this routine PMTM will be left in a state where it can be reinitialised with \ref PMTM_init
!!
!! \ingroup finalization
!! @param err_code <b>(FORTRAN Only)</b> Will be set to \c PMTM_SUCCESS if the call was successful and the appropriate \ref Error if not
!! 
!! \b Notes: This must be called \b before \c MPI_Finalize
!!
!! \b OpenMP \c PMTM_finalize should be called outside of a parallel section or if inside by a single thread only. All usage of PMTM facilities should have 
!! ceased in all threads prior to the call. A synchronisation mechanism should be applied to ensure this if it cannot be guaranteed any other way.
!!
!! @test <b>\c tests_finalize.cpp/reinitialise</b>	Finalising PMTM leaves the library in a state where it can be initialised again without error
!! @test <b>\c tests_finalize.cpp/timer_output</b>	Finalising PMTM should cause it to output all unprinted timers followed by End of File to the output file
!! @test <b>\c tests_finalize.cpp/keep_local_copy_implicit</b>	Finalising PMTM should keep the local copy of the file if PMTM_KEEP_LOCAL_COPY and PMTM_DELETE_LOCAL_COPY are not set
!! @test <b>\c tests_finalize.cpp/keep_local_copy_explicit</b>	Finalising PMTM should keep the local copy if PMTM_KEEP_LOCAL_COPY is set and not '', '0' or case insensitive 'FALSE' and PMTM_DELETE_LOCAL_COPY is set
!! @test <b>\c tests_finalize.cpp/delete_local_copy_explicit</b>	Finalising PMTM should delete the local copy if PMTM_KEEP_LOCAL_COPY is set to '', '0' or case insensitive 'FALSE'
!! @test <b>\c tests_finalize.cpp/delete_local_copy_implicit</b>	Finalising PMTM should delete the local copy of the file if PMTM_KEEP_LOCAL_COPY is not set and PMTM_DELETE_LOCAL_COPY is set and not '', '0' or case insensitive 'FALSE'
!! @test <b>\c tests_finalize.cpp/delete_local_copy_internal</b>	Finalising PMTM should delete the local copy of the file if PMTM_OPTION_NO_LOCAL_COPY is set to PMTM_TRUE using the set_option function
!! @test <b>\c tests_finalize.cpp/file_movement</b>		Finalising PMTM with the environment variable PMTM_DATA_STORE set should copy the pmtm output to the directory indicated by PMTM_DATA_STORE
!! @test <b>\c tests_finalize.cpp/file_movement_disabled</b>	Finalising PMTM with the internal PMTM_OPTION_NO_STORED_COPY variable set to PMTM_TRUE using the internal set_option function should not copy file to PMTM_DATA_STORE
!! @test <b>\c tests.F90/test_finalize</b>	Tests that when \ref PMTM_finalize is called with valid arguments that it returns \c PMTM_SUCCESS
!!
subroutine PMTM_finalize(err_code)
    implicit none
    integer, intent(out) :: err_code

    integer :: c_PMTM_finalize
    err_code = c_PMTM_finalize()
end subroutine PMTM_finalize

!-----------------------------------------------------------------------------------------------------------------------------------
! Log the compiler flags.
!> \section PMTM_log_flags
!! Store the given flags to be written in the output files. This is used to keep a log of which flags the code was built with, so that they can be taken into account when comparing timing outputs
!!
!! \ingroup outputs
!! @param flags The flags used to compile the code, this will be split at spaces and output as a comma separated list
!! @param err_code <b>(FORTRAN Only)</b> Will be set to \c PMTM_SUCCESS if the call was successful and the appropriate \ref Error if not
!!
!! @test <b>\c tests.F90/test_log_flags</b>	ests that giving a flag list to \ref PMTM_log_flags returns \c PMTM_SUCCESS
!!
!! \b Notes: This must be \b before \ref PMTM_init for the flags to be written to the output file associated with the default instance
!!
subroutine PMTM_log_flags(flags, err_code)
    implicit none
    character(len=*) flags
    integer, intent(out) :: err_code
    
    integer :: c_PMTM_log_flags
    err_code = c_PMTM_log_flags(flags, len_trim(flags))
end subroutine

!-----------------------------------------------------------------------------------------------------------------------------------
! Return whether the PMTM library has already been initialised.
!> \section PMTM_initialized
!! Returns whether or not the PMTM library has already been initialised.
!!
!! \ingroup initialization
!!
!! @test <b>\c tests.F90/test_initialized</b>	Tests that \ref PMTM_initialized returns FALSE when \c PMTM is not initialised and TRUE when it is. 
!! 
logical function PMTM_initialized()
    implicit none

    integer :: c_PMTM_initialized
    if (c_PMTM_initialized() == INTERNAL__TRUE) then
        PMTM_initialized = .true.
    else
        PMTM_initialized = .false.
    endif
end function

!-----------------------------------------------------------------------------------------------------------------------------------
! Set the name of the PMTM output file, creating it if necessary.
!> \section PMTM_set_file_name
!! Update the given instance to use write it's output to the file with the given file name. If the file name is blank \b ("") then outputting will be turned off 
!! for the given instance and if the file name equals \b "-" then output will be diverted to \c stdout. If the instance already has an open file this file will be 
!! closed but no timing information will be written to it beforehand.
!!
!! \ingroup controls
!! @param instance The handle of the instance for which we are setting the output file
!! @param file_name The name of the output file
!! @param err_code <b>(FORTRAN Only)</b> Will be set to \c PMTM_SUCCESS if the call was successful and the appropriate \ref Error if not.
!!
!! @test <b>\c tests.F90/test_set_file_name</b>		Tests that \ref PMTM_set_file_name is called with valid arguments that it returns \c PMTM_SUCCESS. 
!!
subroutine PMTM_set_file_name(instance, file_name, err_code)
    implicit none
    integer, intent(in)          :: instance
    character(len=*), intent(in) :: file_name
    integer, intent(out)         :: err_code

    integer :: c_PMTM_set_file_name
    err_code = c_PMTM_set_file_name(instance, file_name, len_trim(file_name))
end subroutine PMTM_set_file_name

!-----------------------------------------------------------------------------------------------------------------------------------
! Output a specific runtime Environment variable directly to PMTM output - for use by Host code
!> \section PMTM_output_specific_runtime_variable
!! Outputs the value of a specific environment variable to the output file, if the variable exists and holds a value. Unlike using \ref pmtmrc this function does not check to see if the variable has already been output.
!!
!! \ingroup environment
!! @param instance The handle of the instance to write the variable to
!! @param variable_name The Environment Variable to output to file
!! @param err_code <b>(FORTRAN Only)</b> Will be set to \c PMTM_SUCCESS if the call was successful and the appropriate \ref Error if not.
!!
!! @test <b>\c tests_options.cpp/get_specific_runtime_variables</b>     Test whether putting a specific Environment variable name in ${PWD}/.pmtmrc prints it to the output, but only once
!! @test <b>\c tests_options.cpp/get_specific_runtime_variables_internal</b>    Test whether PMTM_output_specific_runtime_variable prints out the variable's value to the output
!!
subroutine PMTM_output_specific_runtime_variable(instance, variable_name, err_code)
    implicit none
    integer, intent(in)          :: instance
    character(len=*), intent(in) :: variable_name
    integer, intent(out)         :: err_code

    integer :: c_PMTM_output_specific_runtime_variable
    err_code = c_PMTM_output_specific_runtime_variable(instance, variable_name, len_trim(variable_name))
end subroutine PMTM_output_specific_runtime_variable

!-----------------------------------------------------------------------------------------------------------------------------------
! Create a new instance of PMTM which can be used to direct timers to a different output file.
!> \section PMTM_create_instance
!! Creates another instance of PMTM which can be used to direct the output of timers into this instance rather that the default one.
!!
!! \ingroup initialization
!! @param instance The returned handle to the instance
!! @param file_name The name of the CSV file which PMTM will output the results for the elements associated with this instance ('.pmtm' will be appended to this name and if a file already exists with this name a number will be appended to create a new file).
!! @param application_name The application name that will get logged to the output file e.g. 'DL Poly 2.17'
!! @param err_code <b>(FORTRAN Only)</b> Will be set to \c PMTM_SUCCESS if the call was successful and the appropriate \ref Error if not.
!!
!! @test <b>\c tests.F90/test_create_instance</b>	Tests that calling \ref PMTM_create_instance creates a new \c PMTM instance handle that is not the same as \c PMTM_DEFAULT_INSTANCE
!!
!! \b Note: See \ref PMTM_init for details about \c file_name
!!
subroutine PMTM_create_instance(instance, file_name, application_name, err_code)
    implicit none
    integer, intent(out)         :: instance
    character(len=*), intent(in) :: file_name
    character(len=*), intent(in) :: application_name
    integer, intent(out)         :: err_code

    integer :: c_PMTM_create_instance
    err_code = c_PMTM_create_instance(instance, file_name, len_trim(file_name), application_name, len_trim(application_name))
end subroutine PMTM_create_instance

!-----------------------------------------------------------------------------------------------------------------------------------
! Destroy an instance of PMTM, which will cause the output file to be finished
! and closed and all memory associated with this instance to be released.
!> \section PMTM_destroy_instance
!! Finalises the given PMTM instance, writing any remaining timers to the output file before closing it, and leaving the instance in an uninitialised state. Does not affect any other instance.
!!
!! \ingroup finalization
!! @param instance The handle to the instance
!! @param err_code <b>(FORTRAN Only)</b> Will be set to \c PMTM_SUCCESS if the call was successful and the appropriate \ref Error if not.
!!
!! @test <b>\c tests.F90/test_destroy_instance</b>	Tests that \ref PMTM_destroy_instance returns \c PMTM_SUCCESS when called on a created instance that is not \c PMTM_DEFAULT_INSTANCE
!!
subroutine PMTM_destroy_instance(instance, err_code)
    implicit none
    integer, intent(in)         :: instance
    integer, intent(out)        :: err_code

    integer :: c_PMTM_destroy_instance
    err_code = c_PMTM_destroy_instance(instance)
end subroutine PMTM_destroy_instance

!-----------------------------------------------------------------------------------------------------------------------------------
! Create a timer group used to orgranise the PMTM timers.
!> \section PMTM_create_timer_group
!! Creates a timer group which can be used to organise the timers
!!
!! \ingroup timer_setup
!! @param instance The handle to the instance (the handle to the default instance is given by \c PMTM_DEFAULT_INSTANCE)
!! @param group The returned handle to the created timer group
!! @param group_name The name to know the group by
!! @param err_code <b>(FORTRAN Only)</b> Will be set to \c PMTM_SUCCESS if the call was successful and the appropriate \ref Error if not
!! 
!! @test <b>\c tests.F90/test_create_timer_group</b>	Tests that calling \ref PMTM_create_timer_group in \c PMTM_DEFAULT_INSTANCE that is not \c PMTM_DEFAULT_GROUP returns \c PMTM_SUCCESS
!!
subroutine PMTM_create_timer_group(instance, group, group_name, err_code)
    implicit none
    integer, intent(in)         :: instance
    integer, intent(out)        :: group
    character(len=*), intent(in)   :: group_name
    integer, intent(out)        :: err_code

    integer :: c_PMTM_create_timer_group
    err_code = c_PMTM_create_timer_group(instance, group, group_name, len_trim(group_name))
end subroutine PMTM_create_timer_group

!-----------------------------------------------------------------------------------------------------------------------------------
! Create a PMTM timer.
!> \section PMTM_create_timer
!! Creates a timer in \p group with handle \p timer
!!
!! \ingroup timer_setup
!! @param group The timer group to which this timer will be associated (the handle to the default group is \c PMTM_DEFAULT_GROUP)
!! @param timer The returned handle to the timer
!! @param timer_name The name of timer that will be recorded in the output
!! @param timer_type The type of timer to be created, the options are:
!! - \c PMTM_TIMER_NONE Outputs only the average time for each rank
!! - \c PMTM_TIMER_AVG Outputs the average time for each rank and the average across all ranks
!! - \c PMTM_TIMER_MAX Outputs the average time for each rank and the maximum time across all ranks
!! - \c PMTM_TIMER_MIN Outputs the average time for each rank and the minimum time across all ranks
!! - \c PMTM_TIMER_ALL Outputs the average time for each rank and the maximum, minimum and average time across all ranks
!! - \c PMTM_TIMER_MMA Output \b ONLY the maximum, minimum and average time across all ranks
!! - \c PMTM_TIMER_AVO Output \b ONLY the average time across all ranks
!! - \c PMTM_TIMER_INT Output \b NO information for this timer_control
!! @param err_code <b>(FORTRAN Only)</b> Will be set to \c PMTM_SUCCESS if the call was successful and the appropriate \ref Error if not
!!
!! @test <b>\c tests_timer.cpp/create</b>	Creating a timer in the default timer group should return valid timer id
!! @test <b>\c tests_timer.cpp/commas</b>	Creating a timer with a comma in its name should result in a timer with the comma stripped
!! @test <b>\c tests_timer.cpp/timer_none</b>	With the timer none timer type only the times for each rank should be printed
!! @test <b>\c tests_timer.cpp/timer_max</b>	With the 'timer max' timer type the max time on the ranks should be printed along with the times from each rank
!! @test <b>\c tests_timer.cpp/timer_min</b>	With the 'timer min' timer type the min time on the ranks should be printed along with the times from each rank
!! @test <b>\c tests_timer.cpp/timer_avg</b>	With the 'timer avg' timer type the avg time on the ranks should be printed along with the times from each rank
!! @test <b>\c tests_timer.cpp/timer_all</b>	With the 'timer all' timer type the different stats should be printed along with the times from each rank
!! @test <b>\c tests_timer.cpp/timer_int</b>	With the 'timer int' timer type no timing information should be written to the file.
!! @test <b>\c tests_timer.cpp/timer_mma</b>	With the 'timer mma' timer type only the Max, Min and Average timer stats should be printed not any rank information
!! @test <b>\c tests_timer.cpp/timer_avo</b>	With the 'timer avo' timer type only the Average timer stats should be printed not any rank, max or min information.
!! @test <b>\c tests_threads.cpp/parallel_timing</b>	Timing a one second wait should return a time close to one second from each thread - uses \ref PMTM_create_instance
!! @test <b>\c tests.F90/test_create_timer</b>	Tests that calling \ref PMTM_create_timer with each of the different timer types in turn returns \c PMTM_SUCCESS
!!
!! \b OpenMP Timers running under OpenMP with an instance for each thread should have the same \p timer_name if they are expected to be associated during finalization. 
!! Also, once created, it is assumed all activities (except output) on a timer will be carried out by a single thread. If this is not the case, serialisation should be 
!! applied as no protection is provided by the PMTM library.
!!
subroutine PMTM_create_timer(group, timer, timer_name, timer_type, err_code)
    implicit none
    integer, intent(in)               :: group
    type(pmtm_timer), intent(out)     :: timer
    character(len=*), intent(in)         :: timer_name
    integer, intent(in)               :: timer_type
    integer, intent(out)              :: err_code

    integer :: c_PMTM_create_timer
    err_code = c_PMTM_create_timer(group, timer, timer_name, len_trim(timer_name), timer_type)
end subroutine PMTM_create_timer

!-----------------------------------------------------------------------------------------------------------------------------------
! Start a given stopped timer.
!> \section PMTM_timer_start
!! Starts a timer. The timer should be in the stopped state and will have undefined behaviour if it is not (if PMTM has been built in debug mode then an error will be reported)
!!
!! \ingroup timer_control
!! @param timer The handle of the timer to start
!!
!! @test <b>\c tests_timer.cpp/start_stop</b>	Starting and stopping a timer should increase that timer's count
!! @test <b>\c tests_timer.cpp/timing</b>	Timing a one second wait should return a time close to one second
!! @test <b>\c tests_threads.cpp/parallel_timing</b>	Timing a one second wait should return a time close to one second from each thread - uses \ref PMTM_create_instance
!!
subroutine PMTM_timer_start(timer)
    implicit none
    type(pmtm_timer), intent(in) :: timer

    call c_PMTM_timer_start(timer%handle)
endsubroutine PMTM_timer_start

!-----------------------------------------------------------------------------------------------------------------------------------
! Stop a given running timer.
!> \section PMTM_timer_stop
!! Stops a timer. The timer should be in the running state and will have undefined behaviour if it is not (if PMTM has been built in debug mode then an error will be reported)
!! 
!! \ingroup timer_control
!! @param timer The handle of the timer to stop
!!
!! @test <b>\c tests_timer.cpp/start_stop</b>	Starting and stopping a timer should increase that timer's count
!! @test <b>\c tests_timer.cpp/timing</b>	Timing a one second wait should return a time close to one second
!! @test <b>\c tests_threads.cpp/parallel_timing</b>	Timing a one second wait should return a time close to one second from each thread - uses \ref PMTM_create_instance
!!
subroutine PMTM_timer_stop(timer)
    implicit none
    type(pmtm_timer), intent(in) :: timer

    call c_PMTM_timer_stop(timer%handle)
endsubroutine PMTM_timer_stop

!-----------------------------------------------------------------------------------------------------------------------------------
! Pause a given running timer.
!> \section PMTM_timer_pause
!! Pause a running timer. The timer should be in the running state and will have undefined behaviour if it is not (if PMTM has been built in debug mode then an error will be reported)
!!
!! \ingroup timer_control
!! @param timer The handle of the timer to pause
!!
!! @test <b>\c tests_timer.cpp/pause_continue</b>	Pausing and continuing a timer should increase that timer's pause count
!!
subroutine PMTM_timer_pause(timer)
    implicit none
    type(pmtm_timer), intent(in) :: timer

    call c_PMTM_timer_pause(timer%handle)
endsubroutine PMTM_timer_pause

!-----------------------------------------------------------------------------------------------------------------------------------
! Continue a given paused timer.
!> \section PMTM_timer_continue
!! Continue a paused timer. The timer should be in the paused state and will have undefined behaviour if it is not (if PMTM has been built in debug mode then an error will be reported)
!!
!! \ingroup timer_control
!! @param timer The handle of the timer to continue
!!
!! @test <b>\c tests_timer.cpp/pause_continue</b>	Pausing and continuing a timer should increase that timer's pause count
!!
subroutine PMTM_timer_continue(timer)
    implicit none
    type(pmtm_timer), intent(in) :: timer

    call c_PMTM_timer_continue(timer%handle)
endsubroutine PMTM_timer_continue

!-----------------------------------------------------------------------------------------------------------------------------------
! Output all timers associated with the given instance.
!> \section PMTM_timer_output
!! Prints all the results of the timers associated with the given instance to the location specified in \ref PMTM_init
!!
!! \ingroup timer_output
!! @param instance The handle of the instance for whose timers to output (the default instance handle is \c PMTM_DEFAULT_INSTANCE)
!! @param err_code <b>(FORTRAN Only)</b> Will be set to \c PMTM_SUCCESS if the call was successful and the appropriate \ref Error if not.
!!
!! @test <b>\c tests.F90/test_timer_output</b>	Tests that calling \ref PMTM_timer_output on \c PMTM_DEFAULT_INSTANCE returns \c PMTM_SUCCESS
!!
!! \b OpenMP All activities on \c instance should have ceased prior to calling this subroutine
!!
subroutine PMTM_timer_output(instance, err_code)
    implicit none
    integer, intent(in)  :: instance
    integer, intent(out) :: err_code

    integer :: c_PMTM_timer_output
    err_code = c_PMTM_timer_output(instance)
end subroutine PMTM_timer_output

!-----------------------------------------------------------------------------------------------------------------------------------
! Get the current CPU time since the timer was last started.
!> \section PMTM_get_cpu_time
!! Retrieves the CPU time since this timer was last started. If the timer has never been started this will just return the current CPU time of the system
!!
!! \ingroup timer_output
!! @param timer The handle of the timer to query
!! @returns The CPU time since \p timer was last started
!!
!! @test <b>\c tests.F90/test_get_cpu_time</b>		ests that calling \ref PMTM_get_cpu_time before and after a timer has been started both return a time greater than zero and that they are not the same.
!!
function PMTM_get_cpu_time(timer)
    implicit none
    real(8)                      :: PMTM_get_cpu_time
    type(pmtm_timer), intent(in) :: timer

    real(8) :: c_PMTM_get_cpu_time
    PMTM_get_cpu_time = c_PMTM_get_cpu_time(timer%handle)
endfunction PMTM_get_cpu_time

!-----------------------------------------------------------------------------------------------------------------------------------
! Get the CPU time of the last stop/start interval of a given timer.
!> \section PMTM_get_last_cpu_time
!! Retrieves the CPU time of the last block stored in a given timer. The timer must be stopped for the returned time to be meaningful (this is checked when PMTM is built in debug mode)
!!
!! \ingroup timer_output
!! @param timer The handle of the timer to query
!! @return The CPU time of the last start-stop block for \p timer
!!
!! @test <b>\c tests.F90/test_get_last_cpu_time</b>	Tests calling \ref PMTM_get_last_cpu_time before and after a timer is started but before it was stopped returns a time of zero and that calling it after the timer is stopped returns a time that is non-zero.
!!
function PMTM_get_last_cpu_time(timer)
    implicit none
    real(8)                      :: PMTM_get_last_cpu_time
    type(pmtm_timer), intent(in) :: timer

    real(8) :: c_PMTM_get_last_cpu_time
    PMTM_get_last_cpu_time = c_PMTM_get_last_cpu_time(timer%handle)
endfunction PMTM_get_last_cpu_time

!-----------------------------------------------------------------------------------------------------------------------------------
! Get the total CPU time stored in a given timer.
!> \section PMTM_get_total_cpu_time
!! Retrieves the total CPU time spent in a given timer. 
!!
!! \ingroup timer_output
!! @param timer The handle of the timer to query
!! @returns The total CPU time spent in \p timer
!!
function PMTM_get_total_cpu_time(timer)
    implicit none
    real(8)                      :: PMTM_get_total_cpu_time
    type(pmtm_timer), intent(in) :: timer

    real(8) :: c_PMTM_get_total_cpu_time
    PMTM_get_total_cpu_time = c_PMTM_get_total_cpu_time(timer%handle)
endfunction PMTM_get_total_cpu_time

!-----------------------------------------------------------------------------------------------------------------------------------
! Get the current wallclock time since the timer was last started.
!> \section PMTM_get_wc_time
!! Retrieves the wallclock time since this timer was last started. If the timer has never been started this will just return the current wallclock time of the system.
!! 
!! \ingroup timer_output
!! @param timer The handle of the timer to query
!! @returns The wallclock timer since \p timer was last started
!!
!! @test <b>\c tests.F90/test_get_wc_time</b>	Tests that calling \ref PMTM_get_wc_time before and after a timer has been started both return a time greater than zero and that they are not the same.
!!
function PMTM_get_wc_time(timer)
    implicit none
    real(8)                      :: PMTM_get_wc_time
    type(pmtm_timer), intent(in) :: timer

    real(8) :: c_PMTM_get_wc_time
    PMTM_get_wc_time = c_PMTM_get_wc_time(timer%handle)
endfunction PMTM_get_wc_time

!-----------------------------------------------------------------------------------------------------------------------------------
! Get the wallclock time of the last stop/start interval of a given timer.
!> \section PMTM_get_last_wc_time
!! Retrieves the wallclock time of the last block stored in a given timer. The timer must be stopped for the returned time to be meaningful (this is checked when PMTM is built in debug mode)
!!
!! \ingroup timer_output
!! @param timer The handle of the timer to query
!! @return The wallclock time of the last start-stop block for \p timer
!! 
!! @test <b>\c tests.F90/test_get_last_wc_time</b>	Tests calling \ref PMTM_get_last_wc_time before and after a timer is started but before it was stopped returns a time of zero and that calling it after the timer is stopped returns a time greater than zero.
!!
function PMTM_get_last_wc_time(timer)
    implicit none
    real(8)                      :: PMTM_get_last_wc_time
    type(pmtm_timer), intent(in) :: timer

    real(8) :: c_PMTM_get_last_wc_time
    PMTM_get_last_wc_time = c_PMTM_get_last_wc_time(timer)
endfunction PMTM_get_last_wc_time

!-----------------------------------------------------------------------------------------------------------------------------------
! Get the total wallclock time stored in a given timer.
!> \section PMTM_get_total_wc_time
!! Retrieves the total wallclock time spent in a given timer. 
!!
!! \ingroup timer_output
!! @param timer The handle of the timer to query
!! @returns The total wallclock time spent in \p timer
!!
function PMTM_get_total_wc_time(timer)
    implicit none
    real(8)                      :: PMTM_get_total_wc_time
    type(pmtm_timer), intent(in) :: timer

    real(8) :: c_PMTM_get_total_wc_time
    PMTM_get_total_wc_time = c_PMTM_get_total_wc_time(timer)
endfunction PMTM_get_total_wc_time

!-----------------------------------------------------------------------------------------------------------------------------------
! Set the frequency at which a timer samples, and the max number of samples it will take.
!> \section PMTM_set_sample_mode
!! Set the sample mode of the timer. This allows the setting of how often the timer should sample and whether is should stop sampling after a given number of measurements. If this is not called then PMTM will not do any sampling and every call will be recorded
!!
!! \ingroup timer_setup
!! @param timer The handle of the timer to modify
!! @param frequency The sample frequency to set on the timer
!! @param max_samples The maximum number of samples for a timer to make or \c PMTM_NO_MAX to unset a maximum
!! @param err_code <b>(FORTRAN Only)</b> Will be set to \c PMTM_SUCCESS if the call was successful and the appropriate \ref Error if not
!!
!! @test <b>\c tests_timer.cpp/sampling_frequency</b>	Specifying the sampling frequency should cause the timer to take measurements at that rate
!! @test <b>\c tests_timer.cpp/sampling_maximum</b>	Specifying the maximum samples should cause the timer to stop sampling after that maximum
!! @test <b>\c tests.F90/test_set_sample_mode</b>	Tests that calling \ref PMTM_set_sample_mode with valid options returns \c PMTM_SUCCESS
!!
subroutine PMTM_set_sample_mode(timer, frequency, max_samples, err_code)
    implicit none
    type(pmtm_timer), intent(in) :: timer
    integer, intent(in)          :: frequency
    integer, intent(in)          :: max_samples
    integer, intent(out)         :: err_code

    integer :: c_PMTM_set_sample_mode
    err_code = c_PMTM_set_sample_mode(timer, frequency, max_samples)
endsubroutine PMTM_set_sample_mode

!-----------------------------------------------------------------------------------------------------------------------------------
! Return the error message associated with a given error code.
!> \section PMTM_get_error_message
!! Return a message string for a given error code. This can be used to produce a more human readable error report when a PMTM error is encountered. A list of \ref Error is available
!!
!! \ingroup outputs
!! @param err_code The error code of the message to retrieve
!!
!! @test <b>\c tests.F90/test_get_error_message</b>	Tests that when \c PMTM generates an error (in this case when trying to initialise an instance that is already initialised) that \ref PMTM_get_error_message returns a non-zero length message.
!!
function PMTM_get_error_message(err_code)
    implicit none
    integer, parameter      :: message_len = 200 
    character*(message_len) :: PMTM_get_error_message 
    integer, intent(in)     :: err_code

    call c_PMTM_get_error_message(PMTM_get_error_message, message_len, err_code)
endfunction PMTM_get_error_message

!-----------------------------------------------------------------------------------------------------------------------------------
! Write a real parameter to the output file.
!> \section Parameter Interface
!> \subsection parameter_output_real
!! Log a \b real parametter to the output file specified in \ref PMTM_init.
!!
!! \ingroup parameters
!! @param instance The handle of the instance to whose output file we writing this parameter
!! @param parameter_name The name of the parameter to output
!! @param output_type The conditions for outputting this parameter, the options being:
!! - \c PMTM_OUTPUT_ALWAYS Always the output whenever this function is called
!! - \c PMTM_OUTPUT_ON_CHANGE Only output the parameter if it has chaanged since the last output call
!! - \c PMTM_OUTPUT_ONCE Only output the parameter on the first call to this routine
!! @param for_all_ranks Whether to print the parameter for all ranks (\b .TRUE. ) or just for rank 0 (\b .FALSE. )
!! @param parameter_value The value to output
!! @param err_code <b>(FORTRAN Only)</b> Will be set to \c PMTM_SUCCESS if the call was successful and the appropriate \ref Error if not
!!
!! @test <b>\c tests.F90/test_parameter_output</b>	Tests that calling \ref PMTM_parameter_output with \c real(4) variable returns \c PMTM_SUCCESS
!!
!! \b Notes: The \c C version of this routine takes a \c printf style format string argument, which is used to parse the parameter values at the end.
!!
subroutine parameter_output_real(instance, parameter_name, output_type, for_all_ranks, parameter_value, err_code)
    implicit none
    integer, intent(in)          :: instance
    character(len=*), intent(in) :: parameter_name
    integer, intent(in)          :: output_type
    logical, intent(in)          :: for_all_ranks
    real(4), intent(in)          :: parameter_value
    integer, intent(out)         :: err_code

    integer :: c_PMTM_parameter_output_f
    character(len=*), parameter :: format_string = "%12.6E"
    integer :: for_all_ranks_int

    if (for_all_ranks) then
        for_all_ranks_int = INTERNAL__TRUE
    else
        for_all_ranks_int = INTERNAL__FALSE
    endif

    err_code = c_PMTM_parameter_output_f(instance, parameter_name, len_trim(parameter_name), output_type, for_all_ranks_int, &
            format_string, len_trim(format_string), parameter_value)
endsubroutine parameter_output_real

!-----------------------------------------------------------------------------------------------------------------------------------
! Write a real8 parameter to the output file.
!> 
!> \subsection parameter_output_real8
!! Log a \b real(8) parametter to the output file specified in \ref PMTM_init.
!!
!! \ingroup parameters
!! @param instance The handle of the instance to whose output file we writing this parameter
!! @param parameter_name The name of the parameter to output
!! @param output_type The conditions for outputting this parameter, the options being:
!! - \c PMTM_OUTPUT_ALWAYS Always the output whenever this function is called
!! - \c PMTM_OUTPUT_ON_CHANGE Only output the parameter if it has chaanged since the last output call
!! - \c PMTM_OUTPUT_ONCE Only output the parameter on the first call to this routine
!! @param for_all_ranks Whether to print the parameter for all ranks (\b .TRUE. ) or just for rank 0 (\b .FALSE. )
!! @param parameter_value The value to output
!! @param err_code <b>(FORTRAN Only)</b> Will be set to \c PMTM_SUCCESS if the call was successful and the appropriate \ref Error if not
!!
!! @test <b>\c tests.F90/test_parameter_output</b>	Tests that calling \ref PMTM_parameter_output with \c real(8) variable returns \c PMTM_SUCCESS
!!
!! \b Notes: The \c C version of this routine takes a \c printf style format string argument, which is used to parse the parameter values at the end.
!!
subroutine parameter_output_real8(instance, parameter_name, output_type, for_all_ranks, parameter_value, err_code)
    implicit none
    integer, intent(in)          :: instance
    character(len=*), intent(in) :: parameter_name
    integer, intent(in)          :: output_type
    logical, intent(in)          :: for_all_ranks
    real(8), intent(in)          :: parameter_value
    integer, intent(out)         :: err_code

    integer :: c_PMTM_parameter_output_d
    character(len=*), parameter :: format_string = "%12.6E"
    integer :: for_all_ranks_int

    if (for_all_ranks) then
        for_all_ranks_int = INTERNAL__TRUE
    else
        for_all_ranks_int = INTERNAL__FALSE
    endif

    err_code = c_PMTM_parameter_output_d(instance, parameter_name, len_trim(parameter_name), output_type, for_all_ranks_int, &
            format_string, len_trim(format_string), parameter_value)
endsubroutine parameter_output_real8

!-----------------------------------------------------------------------------------------------------------------------------------
! Write a integer parameter to the output file.
!> 
!> \subsection parameter_output_integer
!! Log an \b integer parametter to the output file specified in \ref PMTM_init.
!!
!! \ingroup parameters
!! @param instance The handle of the instance to whose output file we writing this parameter
!! @param parameter_name The name of the parameter to output
!! @param output_type The conditions for outputting this parameter, the options being:
!! - \c PMTM_OUTPUT_ALWAYS Always the output whenever this function is called
!! - \c PMTM_OUTPUT_ON_CHANGE Only output the parameter if it has chaanged since the last output call
!! - \c PMTM_OUTPUT_ONCE Only output the parameter on the first call to this routine
!! @param for_all_ranks Whether to print the parameter for all ranks (\b .TRUE. ) or just for rank 0 (\b .FALSE. )
!! @param parameter_value The value to output
!! @param err_code <b>(FORTRAN Only)</b> Will be set to \c PMTM_SUCCESS if the call was successful and the appropriate \ref Error if not
!!
!! @test <b>\c tests.F90/test_parameter_output</b>	Tests that calling \ref PMTM_parameter_output with \c integer(4) variable returns \c PMTM_SUCCESS
!!
!! \b Notes: The \c C version of this routine takes a \c printf style format string argument, which is used to parse the parameter values at the end.
!!
subroutine parameter_output_integer(instance, parameter_name, output_type, for_all_ranks, parameter_value, err_code)
    implicit none
    integer, intent(in)          :: instance
    character(len=*), intent(in) :: parameter_name
    integer, intent(in)          :: output_type
    logical, intent(in)          :: for_all_ranks
    integer(4), intent(in)       :: parameter_value
    integer, intent(out)         :: err_code

    integer :: c_PMTM_parameter_output_i
    character(len=*), parameter :: format_string = "%d"
    integer :: for_all_ranks_int

    if (for_all_ranks) then
        for_all_ranks_int = INTERNAL__TRUE
    else
        for_all_ranks_int = INTERNAL__FALSE
    endif

    err_code = c_PMTM_parameter_output_i(instance, parameter_name, len_trim(parameter_name), output_type, for_all_ranks_int, &
            format_string, len_trim(format_string), parameter_value)
endsubroutine parameter_output_integer

!-----------------------------------------------------------------------------------------------------------------------------------
! Write a integer8 parameter to the output file.
!> 
!> \subsection parameter_output_integer8
!! Log an \b integer(8) parametter to the output file specified in \ref PMTM_init.
!!
!! \ingroup parameters
!! @param instance The handle of the instance to whose output file we writing this parameter
!! @param parameter_name The name of the parameter to output
!! @param output_type The conditions for outputting this parameter, the options being:
!! - \c PMTM_OUTPUT_ALWAYS Always the output whenever this function is called
!! - \c PMTM_OUTPUT_ON_CHANGE Only output the parameter if it has chaanged since the last output call
!! - \c PMTM_OUTPUT_ONCE Only output the parameter on the first call to this routine
!! @param for_all_ranks Whether to print the parameter for all ranks (\b .TRUE. ) or just for rank 0 (\b .FALSE. )
!! @param parameter_value The value to output
!! @param err_code <b>(FORTRAN Only)</b> Will be set to \c PMTM_SUCCESS if the call was successful and the appropriate \ref Error if not
!!
!! @test <b>\c tests.F90/test_parameter_output</b>	Tests that calling \ref PMTM_parameter_output with \c integer(8) variable returns \c PMTM_SUCCESS
!!
!! \b Notes: The \c C version of this routine takes a \c printf style format string argument, which is used to parse the parameter values at the end.
!!
subroutine parameter_output_integer8(instance, parameter_name, output_type, for_all_ranks, parameter_value, err_code)
    implicit none
    integer, intent(in)          :: instance
    character(len=*), intent(in) :: parameter_name
    integer, intent(in)          :: output_type
    logical, intent(in)          :: for_all_ranks
    integer(8), intent(in)       :: parameter_value
    integer, intent(out)         :: err_code

    integer :: c_PMTM_parameter_output_l
    character(len=*), parameter :: format_string = "%ld"
    integer :: for_all_ranks_int

    if (for_all_ranks) then
        for_all_ranks_int = INTERNAL__TRUE
    else
        for_all_ranks_int = INTERNAL__FALSE
    endif

    err_code = c_PMTM_parameter_output_l(instance, parameter_name, len_trim(parameter_name), output_type, for_all_ranks_int, &
            format_string, len_trim(format_string), parameter_value)
endsubroutine parameter_output_integer8

!-----------------------------------------------------------------------------------------------------------------------------------
! Write a logical parameter to the output file.
!> 
!> \subsection parameter_output_logical
!! Log an \b logical parametter to the output file specified in \ref PMTM_init.
!!
!! \ingroup parameters
!! @param instance The handle of the instance to whose output file we writing this parameter
!! @param parameter_name The name of the parameter to output
!! @param output_type The conditions for outputting this parameter, the options being:
!! - \c PMTM_OUTPUT_ALWAYS Always the output whenever this function is called
!! - \c PMTM_OUTPUT_ON_CHANGE Only output the parameter if it has chaanged since the last output call
!! - \c PMTM_OUTPUT_ONCE Only output the parameter on the first call to this routine
!! @param for_all_ranks Whether to print the parameter for all ranks (\b .TRUE. ) or just for rank 0 (\b .FALSE. )
!! @param parameter_value The value to output
!! @param err_code <b>(FORTRAN Only)</b> Will be set to \c PMTM_SUCCESS if the call was successful and the appropriate \ref Error if not
!!
!! @test <b>\c tests.F90/test_parameter_output</b>	Tests that calling \ref PMTM_parameter_output with \c logical variable returns \c PMTM_SUCCESS
!!
!! \b Notes: The \c C version of this routine takes a \c printf style format string argument, which is used to parse the parameter values at the end.
!!
subroutine parameter_output_logical(instance, parameter_name, output_type, for_all_ranks, parameter_value, err_code)
    implicit none
    integer, intent(in)          :: instance
    character(len=*), intent(in) :: parameter_name
    integer, intent(in)          :: output_type
    logical, intent(in)          :: for_all_ranks
    logical, intent(in)          :: parameter_value
    integer, intent(out)         :: err_code

    integer :: c_PMTM_parameter_output_s
    character(len=*), parameter :: format_string = "%s"
    integer :: for_all_ranks_int
    character*(10) :: value_string

    if (for_all_ranks) then
        for_all_ranks_int = 1
    else
        for_all_ranks_int = 0
    endif

    if (parameter_value) then
        value_string = "true"
    else
        value_string = "false"
    endif

    err_code = c_PMTM_parameter_output_s(instance, parameter_name, len_trim(parameter_name), output_type, for_all_ranks_int, &
            format_string, len_trim(format_string), value_string, len_trim(value_string))
endsubroutine parameter_output_logical

!-----------------------------------------------------------------------------------------------------------------------------------
! Write a character parameter to the output file.
!> 
!> \subsection parameter_output_character
!! Log a \b character parametter to the output file specified in \ref PMTM_init.
!!
!! \ingroup parameters
!! @param instance The handle of the instance to whose output file we writing this parameter
!! @param parameter_name The name of the parameter to output
!! @param output_type The conditions for outputting this parameter, the options being:
!! - \c PMTM_OUTPUT_ALWAYS Always the output whenever this function is called
!! - \c PMTM_OUTPUT_ON_CHANGE Only output the parameter if it has chaanged since the last output call
!! - \c PMTM_OUTPUT_ONCE Only output the parameter on the first call to this routine
!! @param for_all_ranks Whether to print the parameter for all ranks (\b .TRUE. ) or just for rank 0 (\b .FALSE. )
!! @param parameter_value The value to output
!! @param err_code <b>(FORTRAN Only)</b> Will be set to \c PMTM_SUCCESS if the call was successful and the appropriate \ref Error if not
!!
!! @test <b>\c tests.F90/test_parameter_output</b>	Tests that calling \ref PMTM_parameter_output with \c character variable returns \c PMTM_SUCCESS
!!
!! \b Notes: The \c C version of this routine takes a \c printf style format string argument, which is used to parse the parameter values at the end.
!!
subroutine parameter_output_character(instance, parameter_name, output_type, for_all_ranks, parameter_value, err_code)
    implicit none
    integer, intent(in)          :: instance
    character(len=*), intent(in) :: parameter_name
    integer, intent(in)          :: output_type
    logical, intent(in)          :: for_all_ranks
    character(len=*), intent(in) :: parameter_value
    integer, intent(out)         :: err_code

    integer :: c_PMTM_parameter_output_s
    character(len=*), parameter :: format_string = "%s"
    integer :: for_all_ranks_int

    if (for_all_ranks) then
        for_all_ranks_int = INTERNAL__TRUE
    else
        for_all_ranks_int = INTERNAL__FALSE
    endif

    err_code = c_PMTM_parameter_output_s(instance, parameter_name, len_trim(parameter_name), output_type, for_all_ranks_int, &
            format_string, len_trim(format_string), parameter_value, len_trim(parameter_value))
endsubroutine parameter_output_character

end module PMTM


