/**
 * @file   hardware_counters.h
 * @author AWE Plc.
 *
 */

#ifndef _PMTM_INCLUDE_HARDWARE_COUNTERS_H
#define	_PMTM_INCLUDE_HARDWARE_COUNTERS_H

#include <stdint.h>
#include <stdlib.h>

#include "pmtm.h"

#ifdef	__cplusplus
extern "C" {
#endif

typedef long long hw_counter_t;

size_t get_num_counters();
const char * get_counter_name(size_t counter_idx);
PMTM_error_t start_counters();
PMTM_error_t set_counters(hw_counter_t * counters);
PMTM_error_t stop_counters();

#ifdef	__cplusplus
}
#endif

#endif	/* _PMTM_INCLUDE_HARDWARE_COUNTERS_H */

