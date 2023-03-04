/*
  * Copyright (C) yajin 2008<yajinzhou@gmail.com >
  *     
  * This file is part of the virtualmips distribution. 
  * See LICENSE file for terms of the license. 
  *
  */

/*
Timer routine.
Emulator has many timers. Every 1ms, emulator will check 
whether there is timer request and pause the cpu when 
processing timer event. 
Codes are from qemu.
yajin
*/
#include "vp_timer.h"
#include "vp_clock.h"
#include <stdlib.h>
#include <string.h>

vp_timer_t *active_timers[2];


/**
 * vp_new_timer
 * @brief   
 * @param   clock
 * @param   cb
 * @param   opaque
 * @return  vp_timer_t *
 */
vp_timer_t *vp_new_timer(vp_clock_t *clock, vp_timer_cb *cb, void *opaque)
{
    vp_timer_t *ts;

    ts = malloc(sizeof(vp_timer_t));
    memset(ts, 0x0, sizeof(*ts));
    if (ts == NULL)
        return NULL;
    ts->clock = clock;
    ts->cb = cb;
    ts->opaque = opaque;
    return ts;
}

/**
 * vp_free_timer
 * @brief   
 * @param   ts
 */
void vp_free_timer(vp_timer_t *ts)
{
    assert(ts != NULL);
    free(ts);
}

/* stop a timer, but do not dealloc it */
void vp_del_timer(vp_timer_t *ts)
{
    vp_timer_t **pt, *t;

    /* NOTE: this code must be signal safe because
      qemu_timer_expired() can be called from a signal. */
    pt = &active_timers[ts->clock->type];
    for (;;) {
        t = *pt;
        if (!t)
            break;
        if (t == ts) {
            *pt = t->next;
            break;
        }
        pt = &t->next;
    }
}

/* modify the current timer so that it will be fired when current_time
   >= expire_time. The corresponding callback will be called. */
void vp_mod_timer(vp_timer_t *ts, std_64_t expire_time)
{
    vp_timer_t **pt, *t;

    vp_del_timer(ts);

    /* add the timer in the sorted list */
    /* NOTE: this code must be signal safe because
      qemu_timer_expired() can be called from a signal. */
    pt = &active_timers[ts->clock->type];
    for (;;) {
        t = *pt;
        if (!t)
            break;
        if (t->expire_time > expire_time)
            break;
        pt = &t->next;
    }
    ts->expire_time = expire_time;
    ts->next = *pt;
    *pt = ts;
}

/**
 * vp_timer_pending
 * @brief   
 * @param   ts
 * @return  int
 */
int vp_timer_pending(vp_timer_t *ts)
{
    vp_timer_t *t;
    for (t = active_timers[ts->clock->type]; t != NULL; t = t->next) {
        if (t == ts)
            return 1;
    }
    return 0;
}

/**
 * vp_timer_expired
 * @brief   
 * @param   timer_head
 * @param   current_time
 * @return  inline int
 */
inline int vp_timer_expired(vp_timer_t *timer_head, std_64_t current_time)
{
    if (!timer_head)
        return 0;
    return (timer_head->expire_time <= current_time);
}

/**
 * vp_run_timers
 * @brief   
 * @param   ptimer_head
 * @param   current_time
 */
void vp_run_timers(vp_timer_t **ptimer_head, std_64_t current_time)
{
    vp_timer_t *ts;

    for (;;) {
        ts = *ptimer_head;
        if (!ts || ts->expire_time > current_time)
            break;
        /* remove timer from the list before calling the callback */
        *ptimer_head = ts->next;
        ts->next = NULL;
        /* run the callback (the timer list can be modified) */
        ts->cb(ts->opaque);
    }
}

/**
 * init_timers
 * @brief   
 * @param   void
 */
void init_timers(void)
{
    init_get_clock();

    rt_clock = vp_new_clock(VP_TIMER_REALTIME);
    vm_clock = vp_new_clock(VP_TIMER_VIRTUAL);
}
