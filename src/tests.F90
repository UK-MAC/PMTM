
#include "pFUnit.inc"

module tests_mod
  use pFUnit
  use PMTM
  implicit none

!  include 'pFUnit.inc'

contains

!------------------------------------------------------------------------------

  subroutine test_set_option()
    integer :: err

    call PMTM_set_option(PMTM_OPTION_OUTPUT_ENV, .false., err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)
  end subroutine test_set_option

!------------------------------------------------------------------------------

  subroutine test_init()
    integer :: err

    call PMTM_init("fortran_tests_", "Fortran Tests", err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)
  end subroutine test_init

!------------------------------------------------------------------------------

  subroutine test_finalize()
    integer :: err

    call PMTM_finalize(err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)
  end subroutine test_finalize

!------------------------------------------------------------------------------

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

  subroutine test_log_flags()
    integer :: err

    call PMTM_log_flags("-O0 -traceback", err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)    
  end subroutine test_log_flags

!------------------------------------------------------------------------------

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

  subroutine test_create_timer()
    integer :: err
    type(pmtm_timer) :: timer_none, timer_max, timer_min, timer_avg, timer_all, timer_mma, timer_int

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
    
    call PMTM_create_timer(PMTM_DEFAULT_GROUP, timer_int, "New Timer INT", PMTM_TIMER_INT, err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)

    call PMTM_finalize(err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)
  end subroutine test_create_timer

!------------------------------------------------------------------------------

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

  subroutine test_get_cpu_time()
    integer :: err
    type(pmtm_timer) :: timer
    real(8) :: time

    call PMTM_init("fortran_tests_", "Fortran Tests", err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)

    call PMTM_create_timer(PMTM_DEFAULT_GROUP, timer, "New Timer", PMTM_TIMER_ALL, err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)

    time = PMTM_get_cpu_time(timer)
    call ASSERTTRUE(time > 0) ! time here is system time

    call PMTM_timer_start(timer)

    time = PMTM_get_cpu_time(timer)
    call ASSERTTRUE(time > 0) ! time here is time since timer was started
    
    call PMTM_timer_stop(timer)

    call PMTM_finalize(err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)
  end subroutine test_get_cpu_time

!------------------------------------------------------------------------------

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

  subroutine test_get_wc_time()
    integer :: err
    type(pmtm_timer) :: timer
    real(8) :: time

    call PMTM_init("fortran_tests_", "Fortran Tests", err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)

    call PMTM_create_timer(PMTM_DEFAULT_GROUP, timer, "New Timer", PMTM_TIMER_ALL, err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)

    time = PMTM_get_wc_time(timer)
    call ASSERTTRUE(time > 0) ! time here is just system time

    call PMTM_timer_start(timer)

    time = PMTM_get_wc_time(timer)
    call ASSERTTRUE(time > 0) ! time here is time since timer was started
    
    call PMTM_timer_stop(timer)

    call PMTM_finalize(err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)
  end subroutine test_get_wc_time

!------------------------------------------------------------------------------

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

  subroutine test_parameter_output()
    integer :: err
    real(8)            :: rval
    integer(4)         :: ival
    character(len=100) :: sval
    logical            :: lval

    call PMTM_init("fortran_tests_", "Fortran Tests", err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)

    rval = 1.0
    ival = 2
    sval = "Test"
    lval = .true.

    call PMTM_parameter_output(PMTM_DEFAULT_INSTANCE, "Real Param", PMTM_OUTPUT_ALWAYS, .true., rval, err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)

    call PMTM_parameter_output(PMTM_DEFAULT_INSTANCE, "Int Param", PMTM_OUTPUT_ALWAYS, .true., ival, err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)

    call PMTM_parameter_output(PMTM_DEFAULT_INSTANCE, "Str Param", PMTM_OUTPUT_ALWAYS, .true., sval, err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)

    call PMTM_parameter_output(PMTM_DEFAULT_INSTANCE, "Log Param", PMTM_OUTPUT_ALWAYS, .true., lval, err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)

    call PMTM_finalize(err)
    call ASSERTEQUAL(PMTM_SUCCESS, err)
  end subroutine test_parameter_output

!------------------------------------------------------------------------------

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
