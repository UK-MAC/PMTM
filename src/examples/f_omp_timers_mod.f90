module openmp_pmtm_timers
  use PMTM
  implicit none

  type timers_type
    type(pmtm_timer) :: timer1
    type(pmtm_timer) :: timer2
    type(pmtm_timer) :: timer3
  end type

  type(timers_type) :: timers
  common /timers_private/ timers
  !$omp threadprivate (/timers_private/)

contains

  ! Call this interface if the timers should be created from a serial context prior to any parallel work.

  subroutine create_timers_serial()
    !$omp parallel
    call create_timers_parallel
    !$omp end parallel
  end subroutine

  ! Call this interface if already within a "parallel" region.

  subroutine create_timers_parallel()
    call create_timer(timers%timer1, "Timer 1")
    call create_timer(timers%timer2, "Timer 2")
    call create_timer(timers%timer3, "Timer 3")
  endsubroutine create_timers_parallel

  subroutine create_timer(timer, timer_name)
    type(pmtm_timer), intent(out) :: timer
    character(len=*), intent(in)  :: timer_name

    call PMTM_create_timer(PMTM_DEFAULT_GROUP, timer, timer_name, PMTM_TIMER_ALL, pmtm_status)
    if (pmtm_status /= 0) then
      write(6,*) PMTM_get_error_message(pmtm_status)
    endif
  endsubroutine create_timer

end module