/** 
 * @file   pmtm.c
 * @author AWE Plc.
 * 
 * This file defines the working of the PMTM API. Most of the actually work is
 * performed in the pmtm_internal code with these functions forwarding on the
 * calls to there. However, all MPI calls (if the code is compiled with MPI
 * enabled) are restricted to this file, with the pmtm_internal code MPI
 * agnostic.
 */

#ifndef SERIAL
#  include "mpi.h"
#endif

#include "pmtm.h"
#include "pmtm_internal.h"

#include <math.h>
#include <stdarg.h>
#include <stdint.h>

#ifdef HW_COUNTERS
#  include "hardware_counters.h"
#endif

#ifdef	__cplusplus
extern "C" {
#endif

// The previous version used multiple collectives to bring the data together but
// instead this now uses just two, plus a couple of reduces to check for error
// conditions. This is much faster now.

// Most recent change is to allow arbitrary timers and groups in each rank. All ranks
// in the communicator in use should have the same instances however and should check 
// in with those in a consistent fashion. The real downside to this addition flexibility
// is the amount of additional data sent, since all ranks must send all names as well as
// timers. If that scales badly at any point, tree style reduction probably good to get
// partial combined results and then do overall merge at the IO_RANK. Probably should
// use a "send" version of the timer structure as too much data is getting sent on top
// of the names.

// Things to think about:
//
//    - Instances: How should these function? I think they should be the unit of
//      differentiation for communicators if that is seen as relevant to do. Already,
//      providing all ranks in PMTM_COMM_WORLD promise to init and finalize PMTM the
//      group feature probably can provide the segmentation feature that would allow
//      packages to create namespaces. Otherwise, I think instances are currently
//      a bit problematic and need a bit of cleaning up.
//
//    - The printing mechanism does not show the group name if it is differentiated.
//      Should that happen?
//
//    - The unit of transfer between ranks is "struct PMTM_timer". This has some
//      pointers and other temporary stuff that has no use in a transmission, hence
//      a slimmed down transfer version might be better to allow for a reduction in
//      the amount of data sent.

// Potential improvements?:
//
//    - Some timers have a min/max/average output form. Currently this is just of printing
//      benefit, but potentially the MPI's reduction behaviour could get used. The downside
//      is that this would probably throw away the ability to have mismatched timers and
//      groups in different ranks, which is one of the big additions from the coding below.


static void compute_txamount_and_package(struct PMTM_instance * instance, int *ret_txcnt, char **ret_txbuffer) {

    // Should we lock something during this count? No, the user manual says all
    // activity should have ceased in threads.

    uint group_idx;
    uint timer_idx;

    int timers = 0, unique_timers = 0, txcnt =  0;
    char *txbuffer;

    // Count the space needed

    for (group_idx = 0; group_idx < instance->num_groups; ++group_idx) {
        PMTM_timer_group_t group_id = instance->group_ids[group_idx];
        struct PMTM_timer_group * group = get_timer_group(group_id);

        txcnt += sizeof(group->num_timers) + strlen(group->group_name) + 1;
        txcnt += group->num_timers * sizeof(int);
        txcnt += group->total_timers * sizeof(struct PMTM_timer);

        for (timer_idx = 0; timer_idx < group->num_timers; ++timer_idx) {
            struct PMTM_timer * timer = group->timer_ids[timer_idx];
            txcnt += strlen(timer->timer_name) + 1;

#ifdef PMTM_DEBUG
            if (timer->state != TIMER_STOPPED) {
                const char * this_state = get_state_desc(timer->state);
                const char * good_state = get_state_desc(TIMER_STOPPED);
                pmtm_warn("Requested wallclock time on timer %s that was in state %s, timer must be in %s state",
                        timer->timer_name, this_state, good_state);

                // Removed abort here as the ranks could get out of sync if the timers don't match otherwise.
            }
#endif
        }
    }

    // Allocate a buffer

#define COPY_DATA(dst, src, len) do { memcpy((dst), (src), (len)); } while (0)
#define COPY_TX(src, len) do { COPY_DATA(txcurr, (src), (len)); txcurr += (len); } while (0)

    if ((txbuffer = malloc(txcnt)) != NULL) {
        // Pack data

        char *txcurr = txbuffer;

        for (group_idx = 0; group_idx < instance->num_groups; ++group_idx) {
            PMTM_timer_group_t group_id = instance->group_ids[group_idx];
            struct PMTM_timer_group * group = get_timer_group(group_id);

            // Should pack with memcpy to cope with platforms that can't do
            // unaligned access without SIGSEGV-ing.

            COPY_TX(&group->num_timers, sizeof(group->num_timers));
            COPY_TX(group->group_name, strlen(group->group_name)+1);

            for (timer_idx = 0; timer_idx < group->num_timers; ++timer_idx) {
                struct PMTM_timer * timer = group->timer_ids[timer_idx];

                COPY_TX(timer->timer_name, strlen(timer->timer_name) + 1);
                char *tx_tclocation = txcurr;
                txcurr += sizeof(int);

                timer->rank = instance->rank;
                COPY_TX(timer, sizeof(*timer));

                int threadcount = 1;
#ifdef _OPENMP
                struct PMTM_timer * tim = timer->thread_next;

                while (tim != NULL) {
                    tim->rank = instance->rank;
                    COPY_TX(tim, sizeof(*tim));
                    tim = tim->thread_next;
                    threadcount++;
                }
#endif
                COPY_DATA(tx_tclocation, &threadcount, sizeof(int));
            }
        }
    }

    *ret_txcnt = txcnt;
    *ret_txbuffer = txbuffer;
}


struct Collected_Timer {
    struct Collected_Timer *hash_next, *next;

    char *group_name;
    char *timer_name;

    // An array of character pointers into the receive buffer. An entry for each rank
    // pointer or NULL if rank not contributing.

    char **timerset;
};


static void free_collected_timers(struct Collected_Timer *first) {
    while (first != NULL) {
       struct Collected_Timer *del = first;
       first = first->next;
       free(del->timerset);
       free(del);
    }
}


static unsigned long hash_timername(const char *group_name, const char *timer_name) {
    unsigned const char *gn = (unsigned const char *) group_name;
    unsigned const char *tn = (unsigned const char *) timer_name;

    unsigned long hash = 5381;
    int c;

    while (c = *gn++) hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    hash = ((hash << 5) + hash); // Put a zero separator in
    while (c = *tn++) hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

#define HASHSIZE 1024

static int collect_timers(
          struct PMTM_instance *instance, char *rxbuffer, size_t rxcnt, int *rxdispls, int *rxcnts,
          struct Collected_Timer ** ctimers) {

    int rank, i, threadcount;
    size_t group_timers;

    struct Collected_Timer *hash[HASHSIZE];
    struct Collected_Timer *first = NULL, **curr = &first;
    struct Collected_Timer *new_timer = NULL;
    char **timerset;

    for (i = 0; i < HASHSIZE; i++)
         hash[i] = NULL;

    for (rank = 0; rank < instance->nranks; rank++) {
        char *rxrank = rxbuffer + rxdispls[rank];
        char *rxrankend = rxrank + rxcnts[rank];

        char *group_name = rxrank + sizeof(group_timers);
        COPY_DATA(&group_timers, rxrank, sizeof(group_timers));

        rxrank += sizeof(group_timers) + strlen(group_name) + 1;

        for (i = 0; i < group_timers; i++) {
             // Should probably check for a block overrun here and implausible threadcount
             char *timer_name = rxrank;
             char *timers = rxrank + strlen(timer_name) + 1;

             int h = hash_timername(group_name, timer_name) & (HASHSIZE-1);
             struct Collected_Timer **posn = &hash[h];
             int cmp = 1;

             while (*posn != NULL) {
                 cmp = strcmp(group_name, (*posn)->group_name);
                 if (cmp == 0) cmp = strcmp(timer_name, (*posn)->timer_name);
                 if (cmp <= 0) break;
                 posn = &((*posn)->hash_next);
             }

             if (cmp != 0) {
                 new_timer = malloc(sizeof(struct Collected_Timer));
                 timerset = calloc(instance->nranks, sizeof(*timerset));

                 if (new_timer == NULL || timerset == NULL) goto memory_abort;

                 new_timer->group_name = group_name;
                 new_timer->timer_name = timer_name;
                 new_timer->timerset = timerset;
                 new_timer->hash_next = *posn;
                 *posn = new_timer;

                 new_timer->next = NULL;
                 *curr = new_timer;
                 curr = &new_timer->next;
             }

             // If there is already an entry in the timerset then we've got a clash. Do we
             // really care?
             (*posn)->timerset[rank] = timers;

             COPY_DATA(&threadcount, timers, sizeof(threadcount));
             rxrank = timers + sizeof(threadcount) + threadcount*sizeof(struct PMTM_timer);
        }
    }

    *ctimers = first;

    return 0;

memory_abort:
    if (new_timer != NULL) free(new_timer);
    if (timerset != NULL) free(timerset);

    free_collected_timers(first);

    return 1;
}

static int print_collected_timers(struct PMTM_instance * instance, struct Collected_Timer *ctimers) {

    struct Collected_Timer *ct = ctimers;
    struct PMTM_timer *all_timers = NULL;

    while (ct != NULL) {
        int threads = 0, threadcount;
        int r, t;

        for (r = 0; r < instance->nranks; r++) {
            if (ct->timerset[r] != NULL) {
                COPY_DATA(&threadcount, ct->timerset[r], sizeof(threadcount));
                threads += threadcount;
            }
        }

        all_timers = malloc(threads * sizeof(struct PMTM_timer));
        if (all_timers == NULL) return 1;

        threads = 0;

        for (r = 0; r < instance->nranks; r++) {
            if (ct->timerset[r] != NULL) {
                COPY_DATA(&threadcount, ct->timerset[r], sizeof(threadcount));
                COPY_DATA(all_timers + threads, ct->timerset[r] + sizeof(threadcount),
                          threadcount * sizeof(struct PMTM_timer));

                threads += threadcount;
            }
        }

        for (t = 0; t < threads; t++) {
            all_timers[t].timer_name = ct->timer_name;
        }

        print_timer_array(instance, threads, all_timers, ct->timer_name, all_timers->timer_type);

        ct = ct->next;
    }

    return 0;
}

// MPI Error propagation macro. Please set PMTM_COMM.


PMTM_error_t PMTM_internal_timer_output(struct PMTM_instance * instance, MPI_Comm PMTM_COMM)
{
    int status = PMTM_SUCCESS;
    int malloc_fail = 0;

    char *txbuffer = NULL;
    int *rxcnts = NULL;
    int *rxdispls = NULL;
    char *rxbuffer = NULL;
    struct Collected_Timer *ctimers = NULL;
    int txcnt;
    size_t total_rxcnt =  0;

#ifndef SERIAL

#define PROPAGATE_ABORT(test, error) do { \
    int local_fail = ((test) ? 1 : 0), global_fail; \
    MPI_Allreduce(&local_fail, &global_fail, 1, MPI_INT, MPI_SUM, PMTM_COMM); \
    if (global_fail > 0) { status = (error); goto abort; } \
} while (0)

#else

#define PROPAGATE_ABORT(test, error) do { \
    if (test) { status = (error); goto abort; } \
} while (0)

#endif

    // Work out the total number of timers, the number of unique timers
    // that have several thread instances, and work out the amount of space
    // needed to send everything.

    compute_txamount_and_package(instance, &txcnt, &txbuffer);

#ifndef SERIAL
    if (instance->rank == IO_RANK) {
       rxcnts = malloc(sizeof(*rxcnts) * (instance->nranks+1)); // +1 ?
       rxdispls = malloc(sizeof(*rxdispls) * (instance->nranks+1)); // +1 ?

       malloc_fail = (rxcnts == NULL || rxdispls == NULL);
    }
#endif

    PROPAGATE_ABORT(txbuffer == NULL || malloc_fail, PMTM_ERROR_FAILED_ALLOCATION);

    // Transmit the package sizes to the IO_RANK.

    total_rxcnt = txcnt;

#ifndef SERIAL
    MPI_Gather(&txcnt, sizeof(*rxcnts), MPI_BYTE,
               rxcnts, sizeof(*rxcnts), MPI_BYTE, IO_RANK, PMTM_COMM);

    total_rxcnt =  0;

    if (instance->rank == IO_RANK) {
        int r;

        for (r = 0; r < instance->nranks; r++) {
            size_t current_rxcnt = rxcnts[r];
            rxdispls[r] = total_rxcnt;
            total_rxcnt += current_rxcnt;
        }
    }
#endif

    // Gather data at IO_RANK

    rxbuffer = txbuffer;

#ifndef SERIAL
    if (instance->rank == IO_RANK) {
        rxbuffer = malloc(total_rxcnt);
        malloc_fail = (rxbuffer == NULL);
    }

    PROPAGATE_ABORT(malloc_fail, PMTM_ERROR_FAILED_ALLOCATION);

    MPI_Gatherv(txbuffer, txcnt, MPI_BYTE,
                rxbuffer, rxcnts, rxdispls, MPI_BYTE, IO_RANK, PMTM_COMM);

    // Make sense of and combine up the data for printing...

    if (instance->rank == IO_RANK) {
#endif
        malloc_fail = collect_timers(instance, rxbuffer, total_rxcnt, rxdispls, rxcnts, &ctimers);

        if (!malloc_fail) {
            malloc_fail = print_collected_timers(instance, ctimers);
            free_collected_timers(ctimers);
        }
#ifndef SERIAL
    }
#endif

    PROPAGATE_ABORT(malloc_fail, PMTM_ERROR_FAILED_ALLOCATION);

abort:
    if (txbuffer != NULL) free(txbuffer);

#ifndef SERIAL
    if (rxbuffer != NULL && rxbuffer != txbuffer) free(rxbuffer);
    if (rxcnts != NULL) free(rxcnts);
    if (rxdispls != NULL) free(rxdispls);
#endif

    return status;
}

#ifdef	__cplusplus
}
#endif
