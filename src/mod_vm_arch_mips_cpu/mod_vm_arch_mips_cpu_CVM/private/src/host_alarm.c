/**
 * Copyright (c) 2021 Yunhai Zhu <yunhaia2@gmail.com>
 *
 * see COPYRIGHT file.  
 */

/**
 * @file    host_alarm.c
 * @brief   implement functions
 * @version 1.0
 * @author  Yunhai Zhu
 * @date    2022-02-14
 *
 */
//
// Created by yun on 2/14/22.
//

#include "host_alarm.h"

#include "mod_vm_device.h"
#include "vp_timer.h"
#include <linux/rtc.h>
#include <sys/ioctl.h>

static mod_vm_device_t *p_global_device_SW;
static mod_vm_device_t *p_global_device_ETH;
static mod_vm_arch_mips_cpu_t *p_global_vm_arch_mips_cpu;

static std_int_t timer_freq;
vp_timer_t *adm5120_timer;
#define RTC_TIMEOUT 1 //1000       //1000MS=1S

void dev_sw_rtc_cb(void *opaque);

/**
 * host_alarm_init
 * @brief   
 * @param   p_m
 * @return  std_void_t
 */
std_void_t host_alarm_init(IN mod_vm_arch_mips_cpu_t *p_m)
{
    mod_vm_arch_mips_cpu_imp_t *p_imp_m = (mod_vm_arch_mips_cpu_imp_t *) p_m;
    mod_iid_t mod_vm_device_sw_iid = MOD_VM_DEVICE_SW_IID;
    mod_iid_t mod_vm_device_eth_iid = MOD_VM_DEVICE_ETH_CS8900_IID;

    adm5120_timer = vp_new_timer(rt_clock, dev_sw_rtc_cb, NULL);

    vp_mod_timer(adm5120_timer, vp_get_clock(rt_clock) + RTC_TIMEOUT);

    mod_query_instance(&mod_vm_device_sw_iid, (std_void_t **) &p_global_device_SW, (mod_ownership_t *) p_imp_m);
    mod_query_instance(&mod_vm_device_eth_iid, (std_void_t **) &p_global_device_ETH, (mod_ownership_t *) p_imp_m);

    p_global_vm_arch_mips_cpu = p_m;
}

/**
 * host_alarm_handler
 * @brief   
 * @param   host_signum
 */
void host_alarm_handler(IN int host_signum)
{
    mod_vm_arch_mips_cpu_imp_t *cvm_info = (mod_vm_arch_mips_cpu_imp_t *) p_global_vm_arch_mips_cpu;
    cvm_arch_mips_cpu_t *cpu = &(cvm_info->cpu);

    if (unlikely(cpu->state != CPU_STATE_RUNNING))
        return;

    if (vp_timer_expired(active_timers[VP_TIMER_REALTIME], vp_get_clock(rt_clock))) {
        /*tell cpu we need to pause because timer out */
        cpu->pause_request |= CPU_INTERRUPT_EXIT;
    }
}


/**
 * dev_sw_rtc_cb
 * @brief   
 * @param   opaque
 */
void dev_sw_rtc_cb(IN void *opaque)
{
    mod_vm_device_command(p_global_device_ETH, NULL);
    mod_vm_device_command(p_global_device_SW, NULL);

    vp_mod_timer(adm5120_timer, vp_get_clock(rt_clock) + RTC_TIMEOUT);
}

#define RTC_FREQ 1024

static int rtc_fd;
/**
 * start_rtc_timer
 * @brief   
 * @param   void
 * @return  static int
 */
static int start_rtc_timer(void)
{
    rtc_fd = open("/dev/rtc", O_RDONLY);
    if (rtc_fd < 0){
        return -1;
    }

    if (ioctl(rtc_fd, RTC_IRQP_SET, RTC_FREQ) < 0) {
        fprintf(stderr, "Could not configure '/dev/rtc' to have a 1024 Hz timer. This is not a fatal\n"
                        "error, but for better emulation accuracy either use a 2.6 host Linux kernel or\n"
                        "type 'echo 1024 > /proc/sys/dev/rtc/max-user-freq' as root.\n");
        goto FAIL;
    }
    if (ioctl(rtc_fd, RTC_PIE_ON, 0) < 0) {
        goto FAIL;
    }
    return 0;

FAIL:
    close(rtc_fd);
    return -1;
}

/*host alarm*/
void mips64_init_host_alarm(void)
{
    struct sigaction act;
    struct itimerval itv;

    /* get times() syscall frequency */
    timer_freq = (std_int_t)sysconf(_SC_CLK_TCK);

    /* timer signal */
    sigfillset(&act.sa_mask);
    act.sa_flags = 0;
#if defined(TARGET_I386) && defined(USE_CODE_COPY)
    act.sa_flags |= SA_ONSTACK;
#endif
    act.sa_handler = host_alarm_handler;
    sigaction(SIGALRM, &act, NULL);

    itv.it_interval.tv_sec = 0;
    itv.it_interval.tv_usec = 999; /* for i386 kernel 2.6 to get 1 ms */
    itv.it_value.tv_sec = 0;
    itv.it_value.tv_usec = 10 * 1000;
    setitimer(ITIMER_REAL, &itv, NULL);
    /* we probe the tick duration of the kernel to inform the user if
      the emulated kernel requested a too high timer frequency */
    getitimer(ITIMER_REAL, &itv);

    /* XXX: force /dev/rtc usage because even 2.6 kernels may not
      have timers with 1 ms resolution. The correct solution will
      be to use the POSIX real time timers available in recent
      2.6 kernels */
    /*
      Qemu uses rtc to get 1 ms resolution timer. However, it always crashs my
      os(arch linux (core dump) ). So I do not use rtc for timer.  (yajin)
    */

    if (itv.it_interval.tv_usec > 1000 || 0) {
        /* try to use /dev/rtc to have a faster timer */
        if (start_rtc_timer() < 0)
            return;
        /* disable itimer */
        itv.it_interval.tv_sec = 0;
        itv.it_interval.tv_usec = 0;
        itv.it_value.tv_sec = 0;
        itv.it_value.tv_usec = 0;
        setitimer(ITIMER_REAL, &itv, NULL);

        /* use the RTC */
        sigaction(SIGIO, &act, NULL);
        fcntl(rtc_fd, F_SETFL, O_ASYNC);
        fcntl(rtc_fd, F_SETOWN, getpid());
    }
}