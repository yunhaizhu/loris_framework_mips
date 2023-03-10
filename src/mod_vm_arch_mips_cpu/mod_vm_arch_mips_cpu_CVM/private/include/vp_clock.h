/*
  * Copyright (C) yajin 2008<yajinzhou@gmail.com >
  *     
  * This file is part of the virtualmips distribution. 
  * See LICENSE file for terms of the license. 
  *
  */

#ifndef __VP_CLOCK_H__
#define __VP_CLOCK_H__

#include "std_common.h"

/*clock utils from qemu*/

#define VP_TIMER_REALTIME 0
#define VP_TIMER_VIRTUAL 1

struct vp_clock {
    int type;
    /* XXX: add frequency */
};
typedef struct vp_clock vp_clock_t;

extern vp_clock_t *rt_clock;
extern vp_clock_t *vm_clock;


/**
 * init_get_clock
 * @brief   
 * @param   void
 */
void init_get_clock(void);
/**
 * vp_new_clock
 * @brief   
 * @param   type
 * @return  vp_clock_t *
 */
vp_clock_t *vp_new_clock(int type);
/**
 * vp_get_clock
 * @brief   
 * @param   clock
 * @return  std_64_t
 */
std_64_t vp_get_clock(vp_clock_t *clock);

#endif
