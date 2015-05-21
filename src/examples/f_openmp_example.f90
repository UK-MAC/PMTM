program example
  use PMTM
  use MPI
  implicit none

  integer :: err_code
  type(pmtm_timer) :: loop_timer
  integer :: loop_idx
  real    :: res
  integer, parameter :: N = 10000

  call MPI_Init(err_code)

  ! These calls should only occur once, hence they appear before the parallel region.

  call PMTM_init("example_file_", "Example Application", err_code)

  call PMTM_parameter_output(PMTM_DEFAULT_INSTANCE, &
     "Loop Count", PMTM_OUTPUT_ALWAYS, .false., N, err_code)

  ! Launch the parallel region. Most importantly the loop_timer needs to be "private".

  !$omp parallel default(none), private(err_code, loop_timer), reduction(+:res)

    ! Running parallel, create a timer in each thread and start it counting. So, if OMP_NUM_THREADS=16 for example, there would be 16 timers running after these two calls.

    call PMTM_create_timer(PMTM_DEFAULT_GROUP, loop_timer, "Loop Timer", PMTM_TIMER_ALL, err_code)
    call PMTM_timer_start(loop_timer)

    ! Run the parallel loop.

    !$omp do
      do loop_idx = 1, N
        res = res + loop_idx
      end do
    !$omp end do

    ! Stop all the timers

    call PMTM_timer_stop(loop_timer)
  !$omp end parallel

  ! Only one thread will exist here since the "end parallel" will have stopped all but one. Call finalize in series which will tidy up and report.

  call PMTM_finalize(err_code)

  call MPI_finalize(err_code)

end program