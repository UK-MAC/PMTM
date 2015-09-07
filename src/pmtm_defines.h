#if 0
/* 
 * File:   pmtm_defines.h
 * Author: AWE Plc.
 *
 * This file includes all the definitions that need to be shared between the C
 * and FORTRAN APIs. It is _not_ designed to be included into user code.
 */
#endif

#ifndef _PMTM_INCLUDE_PMTM_DEFINES_H
#define	_PMTM_INCLUDE_PMTM_DEFINES_H

#define INTERNAL__NULL_TIMER ((void *) -1)

#define INTERNAL__TRUE  1
#define INTERNAL__FALSE 0

#define INTERNAL__VERSION_MAJOR 2
#define INTERNAL__VERSION_MINOR 6
#define INTERNAL__VERSION_BUILD 0

#define INTERNAL__DEFAULT_INSTANCE  0
#define INTERNAL__DEFAULT_GROUP     0

#define INTERNAL__TIMER_NONE 0
#define INTERNAL__TIMER_MAX  1
#define INTERNAL__TIMER_MIN  2
#define INTERNAL__TIMER_AVG  4
#define INTERNAL__TIMER_MMA  8
#define INTERNAL__TIMER_INT  16
#define INTERNAL__TIMER_AVO  32

#define INTERNAL__OUTPUT_ALL_RANKS 1
#define INTERNAL__OUTPUT_ALWAYS    2
#define INTERNAL__OUTPUT_ON_CHANGE 4
#define INTERNAL__OUTPUT_ONCE      8

#define INTERNAL__OPTION_OUTPUT_ENV 1
#define INTERNAL__OPTION_NO_LOCAL_COPY 2
#define INTERNAL__OPTION_NO_STORED_COPY 3

#define INTERNAL__NO_MAX 2147483647

/* 
 * 
 * This is currently true for Linux. Unlikely to work for other OS's 
 * particularly Windows.
 * 
 */
#define INTERNAL__RCFILENAME "/.pmtmrc"

#endif	/* _PMTM_INCLUDE_PMTM_DEFINES_H */

