!>
!! \file
!!
#include "pFUnit.inc"

!> \brief Series of tests to ensure that the Fortran API is working correctly with the \c C library sections of \c PMTM
!!
module tests_mod
  use pFUnit
  use PMTM
  implicit none

!  include 'pFUnit.inc'

contains

!------------------------------------------------------------------------------
!>\section test_set_option
!! Test for Fortran API of \ref PMTM_set_option
!!@ingroup tests_fortran
!! 
!! Tests that when \ref PMTM_set_option is given correct values that it returns \c PMTM_SUCCESS
!!
  subroutine test_set_option()
    integer :: err

    call PMTM_set_option(PMTM_OPTION_OUTPUT_ENV, .false., err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)
    
    call PMTM_set_option(PMTM_OPTION_NO_LOCAL_COPY, .false., err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)
    
    call PMTM_set_option(PMTM_OPTION_NO_STORED_COPY, .false., err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)
  end subroutine test_set_option

!------------------------------------------------------------------------------
!> \section test_init
!! Test for Fortran API of \ref PMTM_init
!! @ingroup tests_fortran
!! 
!! Tests that when \ref PMTM_init is called with valid arguments that it returns \c PMTM_SUCCESS
!!
  subroutine test_init()
    integer :: err

    call PMTM_init("fortran_tests_", "Fortran Tests", err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)
  end subroutine test_init

!------------------------------------------------------------------------------
!> \section test_finalize
!! Test for Fortran API of \ref PMTM_finalize
!! @ingroup tests_fortran
!! 
!! Tests that when \ref PMTM_finalize is called with valid arguments that it returns \c PMTM_SUCCESS
!!
  subroutine test_finalize()
    integer :: err

    call PMTM_finalize(err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)
  end subroutine test_finalize

!------------------------------------------------------------------------------
!> \section test_log_flags
!! Test for Fortran API of \ref PMTM_log_flags
!! @ingroup tests_fortran
!! 
!! Tests that giving a flag list to \ref PMTM_log_flags returns \c PMTM_SUCCESS
!!
  subroutine test_log_flags()
    integer :: err

    call PMTM_log_flags("-O0 -traceback", err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)    
  end subroutine test_log_flags

!------------------------------------------------------------------------------
!> \section test_initialized
!! Test for Fortran API of \ref PMTM_initialized
!! @ingroup tests_fortran
!! 
!! Tests that \ref PMTM_initialized returns FALSE when \c PMTM is not initialised and TRUE when it is. The test encompasses the situations of:
!! - PMTM has never been initialised : returns FALSE
!! - PMTM has been initialised       : returns TRUE
!! - PMTM has been finalised         : returns FALSE
!!
  subroutine test_initialized()
    integer :: err

    call ASSERTFALSE(PMTM_initialized())

    call PMTM_init("fortran_tests_", "Fortran Tests", err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)

    call ASSERTTRUE(PMTM_initialized())

    call PMTM_finalize(err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)

    call ASSERTFALSE(PMTM_initialized())
  end subroutine test_initialized

  
!------------------------------------------------------------------------------
!> \section test_set_file_name
!! Test for Fortran API of \ref PMTM_set_file_name
!! @ingroup tests_fortran
!! 
!! Tests that \ref PMTM_set_file_name is called with valid arguments that it returns \c PMTM_SUCCESS. \ref PMTM_init and \ref PMTM_finalize also need to be called to ensure that \ref PMTM_set_file_name works correctly.
!!
  subroutine test_set_file_name()
    integer :: err
    call PMTM_init("", "Fortran Tests", err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)

    call PMTM_set_file_name(PMTM_DEFAULT_INSTANCE, "fortran_tests_", err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)

    call PMTM_finalize(err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)
  end subroutine test_set_file_name

!------------------------------------------------------------------------------
!> \section test_create_instance
!! Test for Fortran API of \ref PMTM_create_instance
!! @ingroup tests_fortran
!! 
!! Tests that calling \ref PMTM_create_instance creates a new \c PMTM instance handle that is not the same as \c PMTM_DEFAULT_INSTANCE
!!
  subroutine test_create_instance()
    integer :: err
    integer :: instance

    call PMTM_init("fortran_tests_", "Fortran Tests", err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)

    instance = PMTM_DEFAULT_INSTANCE

    call PMTM_create_instance(instance, "fortran_tests__2", "More Tests", err)
    call ASSERTNOTEQUAL(PMTM_DEFAULT_INSTANCE, instance)
    call ASSERTEQUAL(PMTM_SUCCESS, err)

    call PMTM_finalize(err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)
  end subroutine test_create_instance

!------------------------------------------------------------------------------
!> \section test_destroy_instance
!! Test for Fortran API of \ref PMTM_destroy_instance
!! @ingroup tests_fortran
!! 
!! Tests that \ref PMTM_destroy_instance returns \c PMTM_SUCCESS when called on a created instance that is not \c PMTM_DEFAULT_INSTANCE
!!
  subroutine test_destroy_instance()
    integer :: err
    integer :: instance

    call PMTM_init("fortran_tests_", "Fortran Tests", err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)

    call PMTM_create_instance(instance, "fortran_tests_2_", "More Tests", err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)

    call PMTM_destroy_instance(instance, err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)

    call PMTM_finalize(err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)
  end subroutine test_destroy_instance

!------------------------------------------------------------------------------
!> \section test_create_timer_group
!! Test for Fortran API of \ref PMTM_create_timer_group
!! @ingroup tests_fortran
!! 
!! Tests that calling \ref PMTM_create_timer_group in \c PMTM_DEFAULT_INSTANCE that is not \c PMTM_DEFAULT_GROUP returns \c PMTM_SUCCESS
!!
  subroutine test_create_timer_group()
    integer :: err
    integer :: group

    call PMTM_init("fortran_tests_", "Fortran Tests", err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)

    call PMTM_create_timer_group(PMTM_DEFAULT_INSTANCE, group, "New Group", err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)

    call PMTM_finalize(err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)
  end subroutine test_create_timer_group

!------------------------------------------------------------------------------
!> \section test_create_timer
!! Test for Fortran API of \ref PMTM_create_timer
!! @ingroup tests_fortran
!! 
!! Tests that calling \ref PMTM_create_timer with each of the different timer types in turn returns \c PMTM_SUCCESS
!!
  subroutine test_create_timer()
    integer :: err
    type(pmtm_timer) :: timer_none, timer_max, timer_min, timer_avg, timer_all, timer_mma, timer_avo, timer_int

    call PMTM_init("fortran_tests_", "Fortran Tests", err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)

    call PMTM_create_timer(PMTM_DEFAULT_GROUP, timer_none, "New Timer NONE", PMTM_TIMER_NONE, err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)
    
    call PMTM_create_timer(PMTM_DEFAULT_GROUP, timer_max, "New Timer MAX", PMTM_TIMER_MAX, err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)
    
    call PMTM_create_timer(PMTM_DEFAULT_GROUP, timer_min, "New Timer MIN", PMTM_TIMER_MIN, err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)
    
    call PMTM_create_timer(PMTM_DEFAULT_GROUP, timer_avg, "New Timer AVG", PMTM_TIMER_AVG, err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)
    
    call PMTM_create_timer(PMTM_DEFAULT_GROUP, timer_all, "New Timer ALL", PMTM_TIMER_ALL, err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)
    
    call PMTM_create_timer(PMTM_DEFAULT_GROUP, timer_mma, "New Timer MMA", PMTM_TIMER_MMA, err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)
    
    call PMTM_create_timer(PMTM_DEFAULT_GROUP, timer_avo, "New Timer AVO", PMTM_TIMER_AVO, err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)
    
    call PMTM_create_timer(PMTM_DEFAULT_GROUP, timer_int, "New Timer INT", PMTM_TIMER_INT, err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)

    call PMTM_finalize(err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)
  end subroutine test_create_timer

!------------------------------------------------------------------------------
!> \section test_timer_output
!! Test for Fortran API of \ref PMTM_timer_output
!! @ingroup tests_fortran
!! 
!! Tests that calling \ref PMTM_timer_output on \c PMTM_DEFAULT_INSTANCE returns \c PMTM_SUCCESS
!!
  subroutine test_timer_output()
    integer :: err

    call PMTM_init("fortran_tests_", "Fortran Tests", err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)

    call PMTM_timer_output(PMTM_DEFAULT_INSTANCE, err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)

    call PMTM_finalize(err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)
  end subroutine test_timer_output

!------------------------------------------------------------------------------
!> \section test_get_cpu_time
!! Test for Fortran API of \ref PMTM_get_cpu_time
!! @ingroup tests_fortran
!! 
!! Tests that calling \ref PMTM_get_cpu_time before and after a timer has been started both return a time greater than zero and that they are not the same.
!!
  subroutine test_get_cpu_time()
    integer :: err
    type(pmtm_timer) :: timer
    real(8) :: time_before, time_after

    call PMTM_init("fortran_tests_", "Fortran Tests", err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)

    call PMTM_create_timer(PMTM_DEFAULT_GROUP, timer, "New Timer", PMTM_TIMER_ALL, err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)

    time_before = PMTM_get_cpu_time(timer)
    call ASSERTTRUE(time_before > 0) ! time here is system time

    call PMTM_timer_start(timer)

    time_after = PMTM_get_cpu_time(timer)
    call ASSERTTRUE(time_after > 0) ! time here is time since timer was started
    call ASSERTNOTEQUAL(time_before, time_after)
    
    call PMTM_timer_stop(timer)

    call PMTM_finalize(err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)
  end subroutine test_get_cpu_time

!------------------------------------------------------------------------------
!> \section test_get_last_cpu_time
!! Test for Fortran API of \ref PMTM_get_last_cpu_time
!! @ingroup tests_fortran
!! 
!! Tests calling \ref PMTM_get_last_cpu_time before and after a timer is started but before it was stopped returns a time of zero and that calling it after the timer is stopped returns a time that is non-zero.
!!
  subroutine test_get_last_cpu_time()
    integer :: err
    type(pmtm_timer) :: timer
    real(8) :: time

    call PMTM_init("fortran_tests_", "Fortran Tests", err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)

    call PMTM_create_timer(PMTM_DEFAULT_GROUP, timer, "New Timer", PMTM_TIMER_ALL, err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)

    time = PMTM_get_last_cpu_time(timer)
    call ASSERTEQUAL(0, time)

    call PMTM_timer_start(timer)

    time = PMTM_get_last_cpu_time(timer)
    call ASSERTEQUAL(0, time)

    call PMTM_timer_stop(timer)

    time = PMTM_get_last_cpu_time(timer)
    call ASSERTNOTEQUAL(0, time)

    call PMTM_finalize(err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)
  end subroutine test_get_last_cpu_time

!------------------------------------------------------------------------------
!> \section test_get_wc_time
!! Test for Fortran API of \ref PMTM_get_wc_time
!! @ingroup tests_fortran
!! 
!! Tests that calling \ref PMTM_get_wc_time before and after a timer has been started both return a time greater than zero and that they are not the same.
!!
  subroutine test_get_wc_time()
    integer :: err
    type(pmtm_timer) :: timer
    real(8) :: time_before, time_after

    call PMTM_init("fortran_tests_", "Fortran Tests", err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)

    call PMTM_create_timer(PMTM_DEFAULT_GROUP, timer, "New Timer", PMTM_TIMER_ALL, err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)

    time_before = PMTM_get_wc_time(timer)
    call ASSERTTRUE(time_before > 0) ! time here is just system time

    call PMTM_timer_start(timer)

    time_after = PMTM_get_wc_time(timer)
    call ASSERTTRUE(time_after > 0) ! time here is time since timer was started
    call ASSERTNOTEQUAL(time_before, time_after)
    
    call PMTM_timer_stop(timer)

    call PMTM_finalize(err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)
  end subroutine test_get_wc_time

!------------------------------------------------------------------------------
!> \section test_get_last_wc_time
!! Test for Fortran API of \ref PMTM_get_last_wc_time
!! @ingroup tests_fortran
!! 
!! Tests calling \ref PMTM_get_last_wc_time before and after a timer is started but before it was stopped returns a time of zero and that calling it after the timer is stopped returns a time greater than zero.
!!
  subroutine test_get_last_wc_time()
    integer :: err
    type(pmtm_timer) :: timer
    real(8) :: time

    call PMTM_init("fortran_tests_", "Fortran Tests", err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)

    call PMTM_create_timer(PMTM_DEFAULT_GROUP, timer, "New Timer", PMTM_TIMER_ALL, err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)

    time = PMTM_get_last_wc_time(timer)
    call ASSERTEQUAL(0, time)

    call PMTM_timer_start(timer)

    time = PMTM_get_last_wc_time(timer)
    call ASSERTEQUAL(0, time)

    call PMTM_timer_stop(timer)

    time = PMTM_get_last_wc_time(timer)
    call ASSERTTRUE(time > 0)

    call PMTM_finalize(err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)
  end subroutine test_get_last_wc_time
!------------------------------------------------------------------------------
!> \section test_parameter_output
!! Test for Fortran API of \ref PMTM_parameter_output
!! @ingroup tests_fortran
!! 
!! Tests that calling \ref PMTM_parameter_output with different valid variable types returns \c PMTM_SUCCESS
!!
  subroutine test_parameter_output()
    integer :: err
    real(8)            :: rval
    real(4)	       :: r4val
    integer(4)         :: ival
    integer(8)	       :: i8val
    character(len=100) :: sval
    logical            :: lval

    call PMTM_init("fortran_tests_", "Fortran Tests", err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)

    rval  = 1.0
    r4val = 0.1
    ival  = 2
    
    sval  = "Test"
    lval  = .true.

    call PMTM_parameter_output(PMTM_DEFAULT_INSTANCE, "Real Param", PMTM_OUTPUT_ALWAYS, .true., rval, err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)

    call PMTM_parameter_output(PMTM_DEFAULT_INSTANCE, "Real4 Param", PMTM_OUTPUT_ALWAYS, .true., r4val, err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)
    
    call PMTM_parameter_output(PMTM_DEFAULT_INSTANCE, "Int Param", PMTM_OUTPUT_ALWAYS, .true., ival, err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)

    call PMTM_parameter_output(PMTM_DEFAULT_INSTANCE, "Int8 Param", PMTM_OUTPUT_ALWAYS, .true., i8val, err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)
    
    call PMTM_parameter_output(PMTM_DEFAULT_INSTANCE, "Str Param", PMTM_OUTPUT_ALWAYS, .true., sval, err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)

    call PMTM_parameter_output(PMTM_DEFAULT_INSTANCE, "Log Param", PMTM_OUTPUT_ALWAYS, .true., lval, err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)

    call PMTM_finalize(err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)
  end subroutine test_parameter_output

!------------------------------------------------------------------------------
!> \section test_set_sample_mode
!! Test for Fortran API of \ref PMTM_set_sample_mode
!! @ingroup tests_fortran
!! 
!! Tests that calling \ref PMTM_set_sample_mode with valid options returns \c PMTM_SUCCESS
!!
  subroutine test_set_sample_mode()
    integer :: err
    type(pmtm_timer) :: timer

    call PMTM_init("fortran_tests_", "Fortran Tests", err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)

    call PMTM_create_timer(PMTM_DEFAULT_GROUP, timer, "New Timer", PMTM_TIMER_ALL, err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)

    call PMTM_set_sample_mode(timer, 5, 100, err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)

    call PMTM_finalize(err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)
  end subroutine test_set_sample_mode

!------------------------------------------------------------------------------
!> \section test_get_error_message
!! Test for Fortran API of \ref PMTM_get_error_message
!! @ingroup tests_fortran
!! 
!! Tests that when \c PMTM generates an error (in this case when trying to initialise an instance that is already initialised) that \ref PMTM_get_error_message returns a non-zero length message.
!!
  subroutine test_get_error_message()
    integer :: err
    character(len=1000) :: msg

    call PMTM_init("fortran_tests_", "Fortran Tests", err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)

    call PMTM_init("fortran_tests_", "Fortran Tests", err)
    call ASSERTNOTEQUAL(PMTM_SUCCESS, err)

    msg = ""
    msg = PMTM_get_error_message(err)
    call ASSERTNOTEQUAL("", msg)
    call ASSERTTRUE(len_trim(msg) > 0)

    call PMTM_finalize(err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)
  end subroutine test_get_error_message

!------------------------------------------------------------------------------

end module tests_mod
