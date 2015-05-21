#include <stdlib.h>

#include "pmtm.h"
#include "mpi.h"

int main(int argc, char ** argv)
{
  double res;
  const uint N = 10000;

  MPI_Init(&argc, &argv);

  // These calls should only occur once, hence they appear before the parallel region.

  PMTM_init("example_file_", "Example Application");
  PMTM_parameter_output(PMTM_DEFAULT_INSTANCE, "Loop Count", PMTM_OUTPUT_ALWAYS, PMTM_FALSE, "%d", N);

  // Launch the parallel region. In particular, notice the "loop_timer" variable is declared local to the region such that each thread has its own copy.

#pragma omp parallel default(none) shared(N) reduction(+:res)
  { PMTM_timer_t loop_timer;
    uint loop_idx;

   // Running parallel, create a timer in each thread and start it counting. So, if OMP_NUM_THREADS=16 for example, there would be 16 timers running after these two calls.

    PMTM_create_timer(PMTM_DEFAULT_GROUP, &loop_timer, "Loop Timer", PMTM_TIMER_ALL);
    PMTM_timer_start(loop_timer);

    // Run the parallel loop.

#pragma omp for
    for (loop_idx = 0; loop_idx < N; ++loop_idx) {
      res += loop_idx;
    }

    // Stop all the timers

    PMTM_timer_stop(loop_timer);
  }

  // Only one thread will exist here since the "end parallel" will have stopped all but one. Call finalize in series which will tidy up and report.

  PMTM_finalize();

  MPI_Finalize();

  return EXIT_SUCCESS;
}