#include "flags.h"

program example
    use PMTM
    use MPI
    implicit none

    integer          :: err_code
    type(pmtm_timer) :: app_timer, loop_timer, fact_timer
    integer          :: loop_idx, fact_res, rank_id
    real(8)          :: res

    call MPI_Init(err_code)

    call PMTM_log_flags(COMPILE_FLAGS, err_code)
    call PMTM_init("f_example_", "Example Application", err_code)
    
    call PMTM_create_timer(PMTM_DEFAULT_GROUP, app_timer, "Application Time", PMTM_TIMER_ALL, err_code)
    call PMTM_create_timer(PMTM_DEFAULT_GROUP, loop_timer, "Loop Time", PMTM_TIMER_ALL, err_code)
    call PMTM_create_timer(PMTM_DEFAULT_GROUP, fact_timer, "Factorial Time", PMTM_TIMER_ALL, err_code)

    call PMTM_timer_start(app_timer)

    do loop_idx = 1, 2
        call PMTM_parameter_output(PMTM_DEFAULT_INSTANCE, "Always - Rank 0 ",       PMTM_OUTPUT_ALWAYS,    .false., loop_idx, err_code)
        call PMTM_parameter_output(PMTM_DEFAULT_INSTANCE, "Always - All Ranks",    PMTM_OUTPUT_ALWAYS,    .true.,  loop_idx, err_code)
    end do
    do loop_idx = 1, 2
        call PMTM_parameter_output(PMTM_DEFAULT_INSTANCE, "Once - Rank 0 ",         PMTM_OUTPUT_ONCE,      .false., loop_idx, err_code)
        call PMTM_parameter_output(PMTM_DEFAULT_INSTANCE, "Once - All Ranks",      PMTM_OUTPUT_ONCE,      .true.,  loop_idx, err_code)
    end do
    do loop_idx = 1, 5
        call PMTM_parameter_output(PMTM_DEFAULT_INSTANCE, "On Change - Rank 0 ",    PMTM_OUTPUT_ON_CHANGE, .false., loop_idx / 5, err_code)
        call PMTM_parameter_output(PMTM_DEFAULT_INSTANCE, "On Change - All Ranks", PMTM_OUTPUT_ON_CHANGE, .true.,  loop_idx / 5, err_code)
    end do

    call PMTM_timer_start(loop_timer)
    res = 1
    do loop_idx = 1, 10000
        res = res + (res / loop_idx)
    end do

    print '(a,g12.6)', 'Result = ', res
    
    call PMTM_timer_stop(loop_timer)
    
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
    
    call MPI_Finalize(err_code)

end program
