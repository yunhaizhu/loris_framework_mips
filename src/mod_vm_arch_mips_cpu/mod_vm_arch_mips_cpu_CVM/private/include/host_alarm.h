/**
 * Copyright (c) 2021 Yunhai Zhu <yunhaia2@gmail.com>
 *
 * see COPYRIGHT file.  
 */
     
/**
 * @file    host_alarm.h
 * @brief   define structure & functions
 * @version 1.0
 * @author  Yunhai Zhu
 * @date    2022-02-14
 *
 */
//
// Created by yun on 2/14/22.
//

#ifndef LORIS_VM_HOST_ALARM_H
#define LORIS_VM_HOST_ALARM_H

#include "std_common.h"
#include "vp_timer.h"
#include <linux/rtc.h>
#include <sys/ioctl.h>
#include "mod_vm_device.h"
#include "mod_vm_arch_mips_cpu.h"
#include "cvm_arch_mips_cpu.h"
#include "mod_vm_arch_mips_cpu_CVM.h"

#define RTC_FREQ 1024
#define RTC_TIMEOUT  1 //1000       //1000MS=1S

/**
 * mips64_main_loop_wait
 * @brief   
 * @param   cpu
 * @param   timeout
 * @return  STD_CALL static void forced_inline
 */
STD_CALL static void forced_inline mips64_main_loop_wait(cvm_arch_mips_cpu_t * cpu, int timeout)
{
    vp_run_timers(&active_timers[VP_TIMER_REALTIME], vp_get_clock(rt_clock));
}

/**
 * host_alarm_init
 * @brief   
 * @param   p_m
 * @return  std_void_t
 */
std_void_t host_alarm_init(IN mod_vm_arch_mips_cpu_t * p_m);
/**
 * mips64_init_host_alarm
 * @brief   
 * @param   void
 * @return  std_void_t
 */
std_void_t mips64_init_host_alarm(void);

#endif//LORIS_VM_HOST_ALARM_H
