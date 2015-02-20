!
! File:   PMTM.F90
! Author: AWE Plc.
!
! This file defines the FORTRAN API of PMTM. These routines call the C code to perform the actual timing.

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

    integer, public, parameter :: PMTM_SUCCESS           = 0
    integer, public, parameter :: PMTM_DEFAULT_GROUP     = INTERNAL__DEFAULT_GROUP
    integer, public, parameter :: PMTM_DEFAULT_INSTANCE  = INTERNAL__DEFAULT_INSTANCE
    integer, public, parameter :: PMTM_TIMER_NONE        = INTERNAL__TIMER_NONE
    integer, public, parameter :: PMTM_TIMER_MAX         = INTERNAL__TIMER_MAX
    integer, public, parameter :: PMTM_TIMER_MIN         = INTERNAL__TIMER_MIN
    integer, public, parameter :: PMTM_TIMER_AVG         = INTERNAL__TIMER_AVG
    integer, public, parameter :: PMTM_TIMER_ALL         = IOR(IOR(INTERNAL__TIMER_MAX, INTERNAL__TIMER_MIN), INTERNAL__TIMER_AVG)
    integer, public, parameter :: PMTM_TIMER_MMA         = IOR(IOR(IOR(INTERNAL__TIMER_MMA, INTERNAL__TIMER_MAX), INTERNAL__TIMER_MIN), INTERNAL__TIMER_AVG)
    integer, public, parameter :: PMTM_TIMER_INT         = INTERNAL__TIMER_INT
    integer, public, parameter :: PMTM_TIMER_AVO         = INTERNAL__TIMER_AVO
    integer, public, parameter :: PMTM_OUTPUT_ALWAYS     = INTERNAL__OUTPUT_ALWAYS
    integer, public, parameter :: PMTM_OUTPUT_ON_CHANGE  = INTERNAL__OUTPUT_ON_CHANGE
    integer, public, parameter :: PMTM_OUTPUT_ONCE       = INTERNAL__OUTPUT_ONCE
    integer, public, parameter :: PMTM_NO_MAX            = INTERNAL__NO_MAX
    integer, public, parameter :: PMTM_OPTION_OUTPUT_ENV = INTERNAL__OPTION_OUTPUT_ENV
    integer, public, parameter :: PMTM_NO_LOCAL_COPY     = INTERNAL__OPTION_NO_LOCAL_COPY
    integer, public, parameter :: PMTM_NO_STORED_COPY    = INTERNAL__OPTION_NO_STORED_COPY
    
!    integer, parameter :: pmtm_timerk           = 4
   
!    type pmtm_timer
!        integer(pmtm_timerk) :: handle = INTERNAL__NULL_TIMER
!    end type

    ! Thread private variables are not allowed initializers, I really hope no app is
    ! assuming they are set.
   
    type pmtm_timer
        type(C_PTR) :: handle ! = C_NULL_PTR
    end type

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
subroutine PMTM_finalize(err_code)
    implicit none
    integer, intent(out) :: err_code

    integer :: c_PMTM_finalize
    err_code = c_PMTM_finalize()
end subroutine PMTM_finalize

!-----------------------------------------------------------------------------------------------------------------------------------
! Log the compiler flags.
subroutine PMTM_log_flags(flags, err_code)
    implicit none
    character(len=*) flags
    integer, intent(out) :: err_code
    
    integer :: c_PMTM_log_flags
    err_code = c_PMTM_log_flags(flags, len_trim(flags))
end subroutine

!-----------------------------------------------------------------------------------------------------------------------------------
! Return whether the PMTM library has already been initialised.
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
subroutine PMTM_destroy_instance(instance, err_code)
    implicit none
    integer, intent(in)         :: instance
    integer, intent(out)        :: err_code

    integer :: c_PMTM_destroy_instance
    err_code = c_PMTM_destroy_instance(instance)
end subroutine PMTM_destroy_instance

!-----------------------------------------------------------------------------------------------------------------------------------
! Create a timer group used to orgranise the PMTM timers.
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
subroutine PMTM_timer_start(timer)
    implicit none
    type(pmtm_timer), intent(in) :: timer

    call c_PMTM_timer_start(timer%handle)
endsubroutine PMTM_timer_start

!-----------------------------------------------------------------------------------------------------------------------------------
! Stop a given running timer.
subroutine PMTM_timer_stop(timer)
    implicit none
    type(pmtm_timer), intent(in) :: timer

    call c_PMTM_timer_stop(timer%handle)
endsubroutine PMTM_timer_stop

!-----------------------------------------------------------------------------------------------------------------------------------
! Pause a given running timer.
subroutine PMTM_timer_pause(timer)
    implicit none
    type(pmtm_timer), intent(in) :: timer

    call c_PMTM_timer_pause(timer%handle)
endsubroutine PMTM_timer_pause

!-----------------------------------------------------------------------------------------------------------------------------------
! Continue a given paused timer.
subroutine PMTM_timer_continue(timer)
    implicit none
    type(pmtm_timer), intent(in) :: timer

    call c_PMTM_timer_continue(timer%handle)
endsubroutine PMTM_timer_continue

!-----------------------------------------------------------------------------------------------------------------------------------
! Output all timers associated with the given instance.
subroutine PMTM_timer_output(instance, err_code)
    implicit none
    integer, intent(in)  :: instance
    integer, intent(out) :: err_code

    integer :: c_PMTM_timer_output
    err_code = c_PMTM_timer_output(instance)
end subroutine PMTM_timer_output

!-----------------------------------------------------------------------------------------------------------------------------------
! Get the current CPU time since the timer was last started.
function PMTM_get_cpu_time(timer)
    implicit none
    real(8)                      :: PMTM_get_cpu_time
    type(pmtm_timer), intent(in) :: timer

    real(8) :: c_PMTM_get_cpu_time
    PMTM_get_cpu_time = c_PMTM_get_cpu_time(timer%handle)
endfunction PMTM_get_cpu_time

!-----------------------------------------------------------------------------------------------------------------------------------
! Get the CPU time of the last stop/start interval of a given timer.
function PMTM_get_last_cpu_time(timer)
    implicit none
    real(8)                      :: PMTM_get_last_cpu_time
    type(pmtm_timer), intent(in) :: timer

    real(8) :: c_PMTM_get_last_cpu_time
    PMTM_get_last_cpu_time = c_PMTM_get_last_cpu_time(timer%handle)
endfunction PMTM_get_last_cpu_time

!-----------------------------------------------------------------------------------------------------------------------------------
! Get the total CPU time stored in a given timer.
function PMTM_get_total_cpu_time(timer)
    implicit none
    real(8)                      :: PMTM_get_total_cpu_time
    type(pmtm_timer), intent(in) :: timer

    real(8) :: c_PMTM_get_total_cpu_time
    PMTM_get_total_cpu_time = c_PMTM_get_total_cpu_time(timer%handle)
endfunction PMTM_get_total_cpu_time

!-----------------------------------------------------------------------------------------------------------------------------------
! Get the current wallclock time since the timer was last started.
function PMTM_get_wc_time(timer)
    implicit none
    real(8)                      :: PMTM_get_wc_time
    type(pmtm_timer), intent(in) :: timer

    real(8) :: c_PMTM_get_wc_time
    PMTM_get_wc_time = c_PMTM_get_wc_time(timer%handle)
endfunction PMTM_get_wc_time

!-----------------------------------------------------------------------------------------------------------------------------------
! Get the wallclock time of the last stop/start interval of a given timer.
function PMTM_get_last_wc_time(timer)
    implicit none
    real(8)                      :: PMTM_get_last_wc_time
    type(pmtm_timer), intent(in) :: timer

    real(8) :: c_PMTM_get_last_wc_time
    PMTM_get_last_wc_time = c_PMTM_get_last_wc_time(timer)
endfunction PMTM_get_last_wc_time

!-----------------------------------------------------------------------------------------------------------------------------------
! Get the total wallclock time stored in a given timer.
function PMTM_get_total_wc_time(timer)
    implicit none
    real(8)                      :: PMTM_get_total_wc_time
    type(pmtm_timer), intent(in) :: timer

    real(8) :: c_PMTM_get_total_wc_time
    PMTM_get_total_wc_time = c_PMTM_get_total_wc_time(timer)
endfunction PMTM_get_total_wc_time

!-----------------------------------------------------------------------------------------------------------------------------------
! Set the frequency at which a timer samples, and the max number of samples it will take.
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
function PMTM_get_error_message(err_code)
    implicit none
    integer, parameter      :: message_len = 200 
    character*(message_len) :: PMTM_get_error_message 
    integer, intent(in)     :: err_code

    call c_PMTM_get_error_message(PMTM_get_error_message, message_len, err_code)
endfunction PMTM_get_error_message

!-----------------------------------------------------------------------------------------------------------------------------------
! Write a real parameter to the output file.
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
