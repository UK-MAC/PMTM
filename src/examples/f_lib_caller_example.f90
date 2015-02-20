program example_lib_caller
    use PMTM
    use f90_lib
    use MPI
    implicit none

    integer          :: mpi_err
    integer          :: err_code
    type(pmtm_timer) :: app_timer, fact_timer
    integer          :: loop_idx, fact_res, rank_id
    real(8)          :: res

    call MPI_Init(mpi_err)

    call PMTM_init("f_lib_caller_example_", "Example Application", err_code)

    call PMTM_create_timer(PMTM_DEFAULT_GROUP, app_timer, "Application Time", PMTM_TIMER_ALL, err_code)
    call PMTM_create_timer(PMTM_DEFAULT_GROUP, fact_timer, "Factorial Time", PMTM_TIMER_ALL, err_code)
    
    call PMTM_timer_start(app_timer)

    call lib_routine(res)

    print '(a,g12.6)', 'Result = ', res
    
    call MPI_Barrier(MPI_COMM_WORLD, err_code)
    
    call PMTM_timer_start(fact_timer)
    
    fact_res = 1
    call MPI_Comm_rank(MPI_COMM_WORLD, rank_id, err_code)
    do loop_idx = 1, (rank_id * 5)
	fact_res = fact_res * loop_idx
    end do
    
    print '(a,I,a,g12.6)', 'Factorial Rank ',rank_id,' = ',fact_res

    call PMTM_timer_stop(fact_timer)
    
    call MPI_Barrier(MPI_COMM_WORLD, err_code)
    
    call PMTM_timer_stop(app_timer)

    call PMTM_finalize(err_code)

    call MPI_Finalize(mpi_err)

end program

