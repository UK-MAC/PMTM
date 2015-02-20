module f90_lib

contains

subroutine lib_routine(res)
    use PMTM
    implicit none

    real(8), intent(out) :: res

    type(pmtm_timer) :: loop_timer
    integer          :: loop_idx
    integer          :: err_code
    logical          :: pmtm_init_called = .false.

    if (.not. PMTM_initialized()) then
        pmtm_init_called = .true.
        call PMTM_init("f_lib_example_", "Example Library", err_code)
    end if

    call PMTM_create_timer(PMTM_DEFAULT_INSTANCE, loop_timer, "Loop Timer", PMTM_TIMER_ALL, err_code)

    call PMTM_timer_start(loop_timer)
    res = 1
    do loop_idx = 1, 10000
        res = res + (res / loop_idx)
    end do
    call PMTM_timer_stop(loop_timer)

    if (pmtm_init_called) then
        call PMTM_finalize(err_code)
    end if

end subroutine lib_routine

end module f90_lib
