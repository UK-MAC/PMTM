module pmtm_timers
  use PMTM
  implicit none
  private

  integer, public :: pmtm_status
  type(pmtm_timer), public :: PMTM_APPLICATION_TIMER
  type(pmtm_timer), public :: PMTM_COMMUNICATION_TIMER
  type(pmtm_timer), public :: PMTM_COMPUTATION_TIMER
  
  public :: create_timers

contains

  subroutine create_timers()
    implicit none
    call create_timer(PMTM_APPLICATION_TIMER, &
       "Application Time")
    call create_timer(PMTM_COMMUNICATION_TIMER, &
       "Communication Time")
    call create_timer(PMTM_COMPUTATION_TIMER, &
       "Computation Time")
  endsubroutine create_timers

  subroutine create_timer(timer, timer_name)
    implicit none
    type(pmtm_timer), intent(out) :: timer
    character(len=*), intent(in)  :: timer_name

    call PMTM_create_timer(PMTM_DEFAULT_GROUP, timer, timer_name, PMTM_TIMER_ALL, pmtm_status)
    if (pmtm_status /= 0) then
      write(6,*) PMTM_get_error_message(pmtm_status)
    endif
  endsubroutine create_timer

end module