/* 
 * File:   timers.h
 * Author: AWE Plc.
 *
 * This file specifies the routine needed to access time resources for the
 * given system.
 *
 * NOTE: This routine mush return both CPU and Elapsed (Wallclock) time. Vendor
 * must supply "system specific code" to do this. The routine included in
 * linux_timers.c is a sample only; alternative implementations are allowed.
 */

#ifndef _PMTM_INCLUDE_TIMERS_H
#define	_PMTM_INCLUDE_TIMERS_H

#ifdef	__cplusplus
extern "C" {
#endif

void set_timers(double * cpu_time, double * elapsed_time);

#ifdef	__cplusplus
}
#endif

#endif	/* _PMTM_INCLUDE_TIMERS_H */

