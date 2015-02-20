/**
 * @file   hardware_counters.c
 * @author AWE Plc.
 *
 */

#include "hardware_counters.h"
#include "pmtm.h"
#include "pmtm_defines.h"
#include "pmtm_internal.h"

#include "papi.h"

#ifdef	__cplusplus
extern "C" {
#endif

#define HW_STATS 7
#define NUM_HW_EVENTS sizeof(pmtm_hw_events) / sizeof(*pmtm_hw_events)

struct pmtm_hw_event {
    char * name;
    int stat[HW_STATS];
};

struct pmtm_hw_event pmtm_hw_events[] = {
    { "UNHALTED_CORE_CYCLES",              { 0, 0, 0, 1, 0, 0, 0 } },
    { "INSTRUCTIONS_RETIRED",              { 0, 0, 0, 0, 1, 0, 0 } },
#ifdef HW_COUNTERS_NEHALEM
/* Nehalem and Westmere: */
    { "SSEX_UOPS_RETIRED:SCALAR_SINGLE",   { 1, 0, 0, 0, 0, 1, 0 } },
    { "SSEX_UOPS_RETIRED:SCALAR_DOUBLE",   { 1, 0, 0, 0, 0, 0, 1 } },
    { "SSEX_UOPS_RETIRED:PACKED_SINGLE",   { 0, 4, 1, 0, 0, 4, 0 } },
    { "SSEX_UOPS_RETIRED:PACKED_DOUBLE",   { 0, 2, 1, 0, 0, 0, 2 } },
    { "SSEX_UOPS_RETIRED:VECTOR_INTEGER",  { 0, 0, 0, 0, 0, 0, 0 } },
#endif
#ifdef HW_COUNTERS_SANDYBRIDGE
/* Sandy Bridge: */
    { "FP_COMP_OPS_EXE:SSE_FP_SCALAR_SINGLE", { 1, 0, 0, 0, 0, 1, 0 } },
    { "FP_COMP_OPS_EXE:SSE_SCALAR_DOUBLE", { 1, 0, 0, 0, 0, 0, 1 } },
    { "FP_COMP_OPS_EXE:SSE_PACKED_SINGLE", { 0, 4, 1, 0, 0, 4, 0 } },
    { "FP_COMP_OPS_EXE:SSE_FP_PACKED_DOUBLE", { 0, 2, 1, 0, 0, 0, 2 } },
    { "SIMD_FP_256:PACKED_SINGLE",         { 0, 8, 1, 0, 0, 8, 0 } },
    { "SIMD_FP_256:PACKED_DOUBLE",         { 0, 4, 1, 0, 0, 0, 4 } },
#endif
    { "RESOURCE_STALLS:ANY",               { 0, 0, 0, 0, 0, 0, 0 } },
    { "LAST_LEVEL_CACHE_MISSES",           { 0, 0, 0, 0, 0, 0, 0 } },
    { "UOPS_RETIRED:STALL_CYCLES",         { 0, 0, 0, 0, 0, 0, 0 } }
};

int pmtm_hw_eventset = PAPI_NULL;
int pmtm_hw_counters_active = 0;
int pmtm_hw_counter_active[NUM_HW_EVENTS];

size_t get_num_counters() {
    return NUM_HW_EVENTS;
}

const char * get_counter_name(size_t counter_idx) {
    return pmtm_hw_events[counter_idx][0];
}

int pmtm_hw_starteventset(PMTM_BOOL firstcall);

PMTM_error_t start_counters()
{
    int check = PAPI_library_init(PAPI_VER_CURRENT);

    if (check < 0 || check != PAPI_VER_CURRENT) {
        pmtm_warn("Could not initialise PAPI library: %s", PAPI_strerror(check));
        return PMTM_ERROR_HW_COUNTERS_INIT_FAILED;
    } else {
        pmtm_hw_eventset = pmtm_hw_starteventset(PMTM_TRUE);
        pmtm_hw_counters_active = (pmtm_hw_eventset != PAPI_NULL);
    }
    
    return PMTM_SUCCESS;
}

PMTM_error_t set_counters(hw_counter_t * counters)
{
    if (pmtm_hw_counters_active) {
        int check = PAPI_read(pmtm_hw_eventset, counters);

        if (check != PAPI_OK) {
            return PMTM_ERROR_HW_COUNTERS_READ_FAILED;
        }
    }
    return PMTM_SUCCESS;
}

PMTM_error_t stop_counters()
{
    if (pmtm_hw_counters_active) {
        long long values[NUM_HW_EVENTS];
        int check = PAPI_stop(pmtm_hw_eventset, values);
        if (check != PAPI_OK) {
            pmtm_warn("Could not stop the PAPI counters: %s", PAPI_strerror(check));
        }

        check = PAPI_cleanup_eventset(pmtm_hw_eventset);
        if (check != PAPI_OK) {
            pmtm_warn("Could not cleanup the PAPI counters: %s", PAPI_strerror(check));
        }

        check = PAPI_destroy_eventset(&pmtm_hw_eventset);
        if (check != PAPI_OK) {
            pmtm_warn("Could not destroy the PAPI counters: %s", PAPI_strerror(check));
        }

        PAPI_shutdown();
    }

    return PMTM_SUCCESS;
}

/**
 * 
 * @param firstcall says if this is the first call to this function. If not and
 * the counters we can activate isn't the same as already in the list, then we
 * shall complain and start nothing for the thread.
 * @return 
 */
int pmtm_hw_starteventset(PMTM_BOOL firstcall) {

    PMTM_BOOL multiplex     = PMTM_FALSE;
    PMTM_BOOL starteventset = PMTM_FALSE;
    PMTM_BOOL fatalerror    = PMTM_FALSE;
    int eventcode;

    int eventset = PAPI_NULL;

    if (PAPI_create_eventset(&eventset) != PAPI_OK) {
        pmtm_warn("Could not create PAPI event set");
    } else {
        if (PAPI_multiplex_init() != PAPI_OK) {
            pmtm_warn("Could not init PAPI multiplexing");
        } else {
            int ev;
            for (ev = 0; ev != NUM_HW_EVENTS; ev++) {
                if (firstcall || pmtm_hw_counter_active[ev]) {
                    if (PAPI_event_name_to_code(pmtm_hw_events[ev].name, &eventcode) != PAPI_OK) {
                        // Don't complain as we don't expect to find them all...
                    } else {
                        int check = PAPI_add_event(eventset, eventcode);

                        if (check == PAPI_ECNFLCT && !multiplex) {
                            PAPI_set_multiplex(eventset);
                            check = PAPI_add_event(eventset, eventcode);
                            multiplex = PMTM_TRUE;
                        }

                        PMTM_BOOL counteractive = (check != PAPI_NULL) ? PMTM_TRUE : PMTM_FALSE;

                        if (firstcall)
                            pmtm_hw_counter_active[ev] = counteractive;

                        if (counteractive) {
                            starteventset = PMTM_TRUE;
                        } else {
                            pmtm_warn("Timing: Could not add PAPI counter '%s'%s",
                                    pmtm_hw_events[ev].name, firstcall ? "" : " to thread");

                            fatalerror = !firstcall;
                        }
                    }
                }
            }

            if (fatalerror) starteventset = PMTM_FALSE;

            if (starteventset) {
                if (PAPI_start(eventset) != PAPI_OK) {
                    pmtm_warn("Could not start PAPI hardware counters");
                    starteventset = 0;
                }
            }

            if (!starteventset) {
                PAPI_destroy_eventset(&eventset);
                eventset = PAPI_NULL;
            }
        }
    }

    return eventset;
}


#ifdef	__cplusplus
}
#endif
