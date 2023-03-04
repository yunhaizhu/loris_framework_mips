/*
  * Copyright (C) yajin 2008<yajinzhou@gmail.com >
  *     
  * This file is part of the virtualmips distribution. 
  * See LICENSE file for terms of the license. 
  *
  */


#ifndef __VP_TIMER_H__
#define __VP_TIMER_H__

#include "vp_clock.h"


#define VP_TIMER_BASE 1000000000LL

/**
 * vp_timer_cb
 * @brief   
 * @param   opaque
 * @return  typedef void
 */
typedef void vp_timer_cb(void *opaque);

struct vp_timer {
    vp_clock_t *clock;
    std_64_t expire_time;
    std_64_t set_time;
    vp_timer_cb *cb;
    void *opaque;
    struct vp_timer *next;
};
typedef struct vp_timer vp_timer_t;
extern vp_timer_t *active_timers[2];


/**
 * vp_new_timer
 * @brief   
 * @param   clock
 * @param   cb
 * @param   opaque
 * @return  vp_timer_t *
 */
vp_timer_t *vp_new_timer(vp_clock_t *clock, vp_timer_cb *cb, void *opaque);
/**
 * vp_free_timer
 * @brief   
 * @param   ts
 */
void vp_free_timer(vp_timer_t *ts);
/**
 * vp_mod_timer
 * @brief   
 * @param   ts
 * @param   expire_time
 */
void vp_mod_timer(vp_timer_t *ts, std_64_t expire_time);
/**
 * vp_del_timer
 * @brief   
 * @param   ts
 */
void vp_del_timer(vp_timer_t *ts);
/**
 * vp_timer_pending
 * @brief   
 * @param   ts
 * @return  int
 */
int vp_timer_pending(vp_timer_t *ts);
/**
 * vp_timer_expired
 * @brief   
 * @param   timer_head
 * @param   current_time
 * @return  int
 */
int vp_timer_expired(vp_timer_t *timer_head, std_64_t current_time);
/**
 * vp_run_timers
 * @brief   
 * @param   ptimer_head
 * @param   current_time
 */
void vp_run_timers(vp_timer_t **ptimer_head, std_64_t current_time);
/**
 * init_timers
 * @brief   
 * @param   void
 */
void init_timers(void);


#endif
