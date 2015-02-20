/*
 * File:   example_c_lib.c
 * Author: AWE Plc.
 *
 */

#include "c_lib_example.h"

#include "pmtm.h"

double lib_function()
{
    PMTM_timer_t loop_timer;
    PMTM_BOOL pmtm_init_called = PMTM_FALSE;

    if (!PMTM_initialised()) {
        pmtm_init_called = PMTM_TRUE;
        PMTM_init("c_lib_example_", "Example Library");
    }
    
    PMTM_create_timer(PMTM_DEFAULT_INSTANCE, &loop_timer, "Loop Timer", PMTM_TIMER_ALL);

    PMTM_timer_start(loop_timer);
    double res = 1;
    uint loop_idx;
    for (loop_idx = 1; loop_idx <= 10000; ++loop_idx) {
        res += (res / loop_idx);
    }
    PMTM_timer_stop(loop_timer);

    if (pmtm_init_called) {
        PMTM_finalize();
    }

    return res;
}
