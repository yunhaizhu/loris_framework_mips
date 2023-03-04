/*
  * Copyright (C) yajin 2008<yajinzhou@gmail.com >
  *     
  * This file is part of the virtualmips distribution. 
  * See LICENSE file for terms of the license. 
  *
  */

#include "vp_clock.h"
#include "std_common.h"
#include <stdlib.h>
#include <string.h>

static int use_rt_clock;
vp_clock_t *rt_clock;
vp_clock_t *vm_clock;


/**
 * init_get_clock
 * @brief   
 * @param   void
 */
void init_get_clock(void)
{
    use_rt_clock = 0;
#if defined(__linux__)
    {
        struct timespec ts;
        if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
            use_rt_clock = 1;
        }
    }
#endif
}

/*ns*/
static std_u64_t get_clock(void)
{
#if defined(__linux__)
    if (use_rt_clock) {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        return ts.tv_sec * 1000000000LL + ts.tv_nsec;
    } else
#endif
    {
        /* XXX: using gettimeofday leads to problems if the date
         changes, so it should be avoided. */
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return tv.tv_sec * 1000000000LL + (tv.tv_usec * 1000);
    }
}

/*ms*/
std_64_t vp_get_clock(vp_clock_t *clock)
{
    switch (clock->type) {
        case VP_TIMER_REALTIME:
            return get_clock() / 1000000;

        case VP_TIMER_VIRTUAL:
        default:
            break;
    }
    return 0;
}

/**
 * vp_new_clock
 * @brief   
 * @param   type
 * @return  vp_clock_t *
 */
vp_clock_t *vp_new_clock(int type)
{
    vp_clock_t *clock;
    clock = malloc(sizeof(vp_clock_t));
    memset(clock, 0x0, sizeof(*clock));
    if (!clock)
        return NULL;
    clock->type = type;
    return clock;
}
