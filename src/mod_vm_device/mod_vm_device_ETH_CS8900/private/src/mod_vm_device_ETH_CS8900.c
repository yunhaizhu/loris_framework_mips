/**
 * Copyright (c) 2021 Yunhai Zhu <yunhaia2@gmail.com>
 *
 * see COPYRIGHT file.
 */

/**
 * @file    mod_vm_device_ETH_CS8900.c
 * @brief   implement functions
 * @version 1.0
 * @author  Yunhai Zhu
 * @date    2021-12-29
 *
 */

#include "mod_vm_device_ETH_CS8900.h"
#include "mod_vm_arch_mips_cpu.h"
#include "dev_netio.h"
#include <assert.h>
#include <sys/time.h>
#include <time.h>

mod_vm_device_t *p_global_device_INT;

std_u32_t crc32_array[256];
/*00:62:9c:61:cf:16*/
static std_u8_t cs8900a_default_mac[6] = { 0x00, 0x62, 0x9c, 0x61, 0xcf, 0x16 };

#define ASSERT(a,format,args...)  do{ if ((format!=NULL)&&(!(a)))   fprintf(stderr,format, ##args); assert((a));} while(0)
/**
 * mod_vm_device_ETH_CS8900_init
 * @brief   
 * @param   p_m
 * @param   arg
 * @param   arg_len
 * @return  STD_CALL std_rv_t
 */
STD_CALL std_rv_t mod_vm_device_ETH_CS8900_init(IN mod_vm_device_t *p_m, IN const std_char_t *arg,
                                                IN std_int_t arg_len)
{
    mod_vm_device_imp_t *p_imp_m = (mod_vm_device_imp_t *) p_m;
    mod_iid_t mod_shell_iid = MOD_SHELL_IID;
    std_char_t key[BUF_SIZE_128] = "\0";

    mod_create_instance(&mod_shell_iid, (std_void_t **) &p_imp_m->p_mod_shell, (mod_ownership_t *) p_imp_m);

    mod_shell_init(p_imp_m->p_mod_shell, NULL, 0);

    /****shell_register******/

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_device_initiate");
    mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)),
                       shell_stub_mod_vm_device_ETH_CS8900_initiate, p_imp_m);

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_device_reset");
    mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)),
                       shell_stub_mod_vm_device_ETH_CS8900_reset, p_imp_m);

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_device_access");
    mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)),
                       shell_stub_mod_vm_device_ETH_CS8900_access, p_imp_m);

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_device_command");
    mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)),
                       shell_stub_mod_vm_device_ETH_CS8900_command, p_imp_m);

    return STD_RV_SUC;
}

/**
 * mod_vm_device_ETH_CS8900_cleanup
 * @brief   
 * @param   p_m
 * @return  STD_CALL std_rv_t
 */
STD_CALL std_rv_t mod_vm_device_ETH_CS8900_cleanup(mod_vm_device_t *p_m)
{
    mod_vm_device_imp_t *p_imp_m = (mod_vm_device_imp_t *) p_m;
    mod_iid_t mod_shell_iid = MOD_SHELL_IID;
    std_char_t key[BUF_SIZE_128] = "\0";

    /****shell_unregister******/

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_device_initiate");
    mod_shell_unregister(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)));

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_device_reset");
    mod_shell_unregister(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)));

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_device_access");
    mod_shell_unregister(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)));

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_device_command");
    mod_shell_unregister(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)));

    mod_shell_cleanup(p_imp_m->p_mod_shell);

    mod_delete_instance(&mod_shell_iid, (std_void_t **) &p_imp_m->p_mod_shell, (mod_ownership_t *) p_imp_m);

    return STD_RV_SUC;
}

/***func_implementation***/
/* Compute a CRC-32 on the specified block */
static forced_inline std_u32_t crc32_compute(std_u32_t crc_accum, std_u8_t * ptr, int len)
{
    unsigned long c = crc_accum;
    int n;

    for (n = 0; n < len; n++)
    {
        c = crc32_array[(c ^ ptr[n]) & 0xff] ^ (c >> 8);
    }

    return ~c;
}

/* Initialize CRC-32 algorithm */
static void crc32_init(void)
{
    unsigned long c;
    int n, k;

    for (n = 0; n < 256; n++)
    {
        c = (unsigned long) n;
        for (k = 0; k < 8; k++)
        {
            if (c & 1)
                c = CRC32_POLY ^ (c >> 1);
            else
                c = c >> 1;
        }
        crc32_array[n] = c;
    }
}



/* Check for a broadcast/multicast ethernet address */
static inline int eth_addr_is_bcast(n_eth_addr_t * addr)
{
    return ((addr->eth_addr_byte[0] == 0xff)
            && (addr->eth_addr_byte[1] == 0xff)
            && (addr->eth_addr_byte[2] == 0xff)
            && (addr->eth_addr_byte[3] == 0xff) && (addr->eth_addr_byte[4] == 0xff) && (addr->eth_addr_byte[5] == 0xff));
}

/**
 * eth_addr_is_mcast
 * @brief   
 * @param   addr
 * @return  static inline int
 */
static inline int eth_addr_is_mcast(n_eth_addr_t * addr)
{
    return ((!eth_addr_is_bcast(addr)) && (addr->eth_addr_byte[0] & 1));

}

/**
 * clr_irq
 * @brief   
 * @return  int
 */
int clr_irq()
{
    typedef struct command_arg{
        std_u32_t command;
        std_u32_t irq;
    }command_arg_t;

#define SET_IRQ 1
#define CLR_IRQ 2
    command_arg_t cmd;

    cmd.command = CLR_IRQ;
    cmd.irq = 4;

    //printf("GEN ETH Interrupt\n");
    mod_vm_device_command(p_global_device_INT, &cmd);
    return 0;
}


/**
 * get_clock
 * @brief   
 * @param   void
 * @return  static std_u32_t
 */
static std_u32_t get_clock(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec * 1000000000LL + ts.tv_nsec) / 1000000;
}

/**
 * dev_cs8900_gen_interrupt
 * @brief   
 * @param   d
 * @return  static void
 */
static void dev_cs8900_gen_interrupt(struct cs8900_data *d)
{
    /*must check RQ bit in 0x116 */
    std_u8_t *ram_base;
    ram_base = (std_u8_t *) (&(d->internal_ram[0]));
    if ((*(std_u16_t *) (ram_base + PP_BusCTL)) & (0x8000)) {
        /*generate IRQ */

        typedef struct command_arg{
            std_u32_t command;
            std_u32_t irq;
        }command_arg_t;

#define SET_IRQ 1
#define CLR_IRQ 2
        command_arg_t cmd;

        cmd.command = SET_IRQ;
        cmd.irq = d->irq_no;

        mod_vm_device_command(p_global_device_INT, &cmd);

    }
}


/* Check if a packet must be delivered to the emulated chip based on length*/
static inline int cs8900_handle_len(struct cs8900_data *d, std_u8_t * pkt, ssize_t pkt_len)
{
    /*we do not check CRC !!!! */
    //	std_u8_t *ram_base;
    //
    //	ram_base = (std_u8_t *) (&(d->internal_ram[0]));
    if (pkt_len < CS8900_MIN_PKT_SIZE)
        return FALSE;

    assert((pkt_len >= CS8900_RUN_PKT_SIZE) && (pkt_len <= CS8900_MAX_PKT_SIZE));
    /*64<LEN<1518 */

    return TRUE;

}

/* Check if a packet must be delivered to the emulated chip */
static inline int cs8900_handle_mac_addr(struct cs8900_data *d, std_u8_t * pkt)
{
    n_eth_hdr_t *hdr = (n_eth_hdr_t *) pkt;
    std_u8_t *ram_base;

    ram_base = (std_u8_t *) (&(d->internal_ram[0]));

    if ((*(std_u16_t *) (ram_base + PP_RxCTL)) & PromiscuousA) {
        goto rx_dest_int;
    }
    if (eth_addr_is_bcast(&hdr->daddr)) {
        if ((*(std_u16_t *) (ram_base + PP_RxCTL)) & BroadcastA) {
            *(std_u16_t *) (ram_base + PP_RxEvent) |= BroadcastA;
            *(std_u16_t *) (ram_base + PP_RxStatus) |= BroadcastA;
            goto rx_dest_int;
        }  else {
            return FALSE;
        }
    }
    if (eth_addr_is_mcast(&hdr->daddr)) {
        if ((*(std_u16_t *) (ram_base + PP_RxCTL)) & MulticastA) {
            *(std_u16_t *) (ram_base + PP_RxEvent) |= MulticastA;
            *(std_u16_t *) (ram_base + PP_RxStatus) |= MulticastA;
            goto rx_dest_int;
        } else {
            return FALSE;
        }
    }

    if ((*(std_u16_t *) (ram_base + PP_RxCTL)) & IndividualA) {
        /* Accept frames directly for us, discard others */
        if (!memcmp((ram_base + PP_IA), &hdr->daddr, N_ETH_ALEN)) {
            *(std_u16_t *) (ram_base + PP_RxEvent) |= IndividualA;
            *(std_u16_t *) (ram_base + PP_RxStatus) |= IndividualA;
            goto rx_dest_int;
        } else {
            return FALSE;
        }
    }

rx_dest_int:
    return (TRUE);
}

/**
 * dev_cs8900_receive_pkt
 * @brief   
 * @param   d
 * @param   pkt
 * @param   pkt_len
 * @return  static int
 */
static int dev_cs8900_receive_pkt(struct cs8900_data *d, std_u8_t * pkt, ssize_t pkt_len)
{
    std_u8_t *ram_base;
    ram_base = (std_u8_t *) (&(d->internal_ram[0]));

    /* Truncate the packet if it is too big */
    pkt_len = MIN(pkt_len, CS8900_MAX_PKT_SIZE);
    /*set RX len */
    *(std_u16_t *) (ram_base + PP_RxLength) = pkt_len;
    /*Rx status has been set */
    /*just copy frame to internal ram */
    memcpy(ram_base + PP_RxFrame, pkt, pkt_len);
    /*generate interrupt */

    *(std_u16_t *) (ram_base + PP_RxEvent) |= RxOKA;
    *(std_u16_t *) (ram_base + PP_RxStatus) |= RxOKA;
    if ((*(std_u16_t *) (ram_base + PP_RxCFG)) & RxOKiE)
    {
        //*(std_u16_t*)(ram_base+PP_ISQ) &= ~0x3f;
        *(std_u16_t *) (ram_base + PP_ISQ) |= RxEvent;
        dev_cs8900_gen_interrupt(d);
    }

    return TRUE;

}
static int dev_cs8900_rx(int nio, std_u8_t * pkt, ssize_t pkt_len, struct cs8900_data *d)
{
    std_u8_t *ram_base;
    std_u16_t real_len;
    int i;
    std_u32_t ifcs;

    ram_base = (std_u8_t *) (&(d->internal_ram[0]));

    if (!((*(std_u16_t *) (ram_base + PP_LineCTL)) & SerRxON))
        return FALSE;
    real_len = pkt_len;


    /*FXME: yajin
      jzdriver discard <64 bytes packet. But arp packet has 40 bytes. Pad it to 64 bytes to meet jz driver's requirement
    */
    if (unlikely(pkt_len < 64))
    {
        /*pad to 60 bytes */
        for (i = pkt_len; i < 60; i++)
        {
            *(pkt + i) = 0x0;
        }
        /*add crc */
        ifcs = crc32_compute(0xFFFFFFFF, pkt, 60);
        *(pkt + 60) = ifcs & 0xff;
        *(pkt + 61) = (ifcs >> 8) & 0xff;
        *(pkt + 62) = (ifcs >> 16) & 0xff;
        *(pkt + 63) = ifcs >> 24;
        real_len = 64;
    }

    /*check MAC address */
    if (!(cs8900_handle_mac_addr(d, pkt)))
        return FALSE;

    /*check frame len */
    if (!(cs8900_handle_len(d, pkt, real_len)))
        return FALSE;

    return (dev_cs8900_receive_pkt(d, pkt, real_len));



}

/**
 * dev_cs8900_tx
 * @brief   
 * @param   d
 * @return  static int
 */
static int dev_cs8900_tx(struct cs8900_data *d)
{

    std_u8_t *ram_base;
    ram_base = (std_u8_t *) (&(d->internal_ram[0]));
    std_u16_t send_len;
    int i;
    std_u32_t ifcs;

    send_len = *(std_u16_t *) (ram_base + PP_TxLength);
    /*check if tx is enabled */
    if ((*(std_u16_t *) (ram_base + PP_LineCTL)) & SerTxON)
    {
        /*pad if len<60 */
        if (send_len <= (CS8900_RUN_PKT_SIZE - 4))
        {
            if (!((*(std_u16_t *) (ram_base + PP_TxCMD)) & TxPadDis))
            {
                /*pad to 60 bytes */
                for (i = send_len; i < 60; i++)
                {
                    *(ram_base + PP_TxFrame + i) = 0x0;
                }
                send_len = 60;
                if (!((*(std_u16_t *) (ram_base + PP_TxCMD)) & InhibitCRC))
                {
                    /*append crc */
                    ifcs = crc32_compute(0xFFFFFFFF, ram_base + PP_TxFrame, send_len);
                    *(ram_base + PP_TxFrame + send_len) = ifcs & 0xff;
                    *(ram_base + PP_TxFrame + send_len + 1) = (ifcs >> 8) & 0xff;
                    *(ram_base + PP_TxFrame + send_len + 2) = (ifcs >> 16) & 0xff;
                    *(ram_base + PP_TxFrame + send_len + 3) = ifcs >> 24;
                    send_len += 4;


                }
            }
        }
        *(std_u16_t *) (ram_base + PP_TxLength) = send_len;
        netio_tap_send(d->netio_fd, ram_base + PP_TxFrame, send_len);
        *(std_u16_t *) (ram_base + PP_TxEvent) = TxOK | 0x8;     /*is = not |.  all other bits must be cleared */
        if ((*(std_u16_t *) (ram_base + PP_TxCFG)) & TxOKiE)
        {
            /*if TXOKIE, generate an interrupt */
            /*set ISQ (regno=TX Event) */
            //*(std_u16_t*)(ram_base+PP_ISQ) &= ~0x3f;
            *(std_u16_t *) (ram_base + PP_ISQ) |= TxEvent;
            dev_cs8900_gen_interrupt(d);
        }
    }

    return TRUE;
}


static std_u32_t expire = 0;
static std_u32_t active_time = 0;
static void active_timer(){
    active_time = 1;
    expire = get_clock() + cs8900a_rx_timeout;
}



/* Maximum packet size */
#define NETIO_MAX_PKT_SIZE  32768

std_u8_t rx_pkt[NETIO_MAX_PKT_SIZE];


/**
 * dev_cs8900_cb
 * @brief   
 * @param   opaque
 */
void dev_cs8900_cb(void *opaque)
{
    struct cs8900_data *d = opaque;
    ssize_t pkt_len;
    static std_u8_t status = 0;

    pkt_len = netio_tap_recv(d->netio_fd, rx_pkt, sizeof(rx_pkt));

    if (pkt_len > 0)
    {
        /*rx packet */
        dev_cs8900_rx(d->netio_fd, rx_pkt, pkt_len, d);

        /*Why we need to adjust CS8900_MAX_RX_TIMEOUT? yajin
	If CS8900_MAX_RX_TIMEOUT is small, that means rx packets quickly. Tx can not get enough time to tell cpu
	that tx ok.
	If CS8900_MAX_RX_TIMEOUT is big, that means rx packets slow. This will decrease network throughtput and
	some applications will complain about rx timeout.
	So I adjut the CS8900_MAX_RX_TIMEOUT dynamicly when receiving a packet .

	Please use TCP protocol instead of UDP when mounting directory using nfs.
      */

        if (cs8900a_rx_timeout >= CS8900_MAX_RX_TIMEOUT)
            status = 1;
        else if (cs8900a_rx_timeout <= CS8900_MIN_RX_TIMEOUT)
            status = 2;

        if (status == 0)
            cs8900a_rx_timeout -= CS8900_RX_TIMEOUT_STEP;
        if (status == 1)
            cs8900a_rx_timeout -= CS8900_RX_TIMEOUT_STEP;
        else if (status == 2)
            cs8900a_rx_timeout += CS8900_RX_TIMEOUT_STEP;

        //cs8900a_rx_timeout = CS8900_DEFAULT_RX_TIMEOUT;

    }

    active_timer();


}

/**
 * mod_vm_device_ETH_CS8900_initiate
 * @brief   
 * @param   p_m
 * @param   arg
 * @return  STD_CALL             std_void_t
 */
STD_CALL std_void_t mod_vm_device_ETH_CS8900_initiate(IN mod_vm_device_t *p_m, IN vm_device_info_t *arg)
{
    vm_device_info_t *p_init = &p_m->info;
    mod_vm_device_imp_t *p_eth_cs8900_info = (mod_vm_device_imp_t *)p_m;
    struct cs8900_data *d = &(p_eth_cs8900_info->eth_cs8900_data);
    mod_iid_t mod_vm_device_int_iid = MOD_VM_DEVICE_INT_IID;

    mod_query_instance(&mod_vm_device_int_iid, (std_void_t **) &p_global_device_INT, (mod_ownership_t *) p_m);

    memcpy(p_init, arg, sizeof(vm_device_info_t));
	
    p_init->flags = VDEVICE_FLAG_NO_MTS_MMAP;
    
    d->netio_fd = netio_tap_open();
    d->irq_no = CS8900_DEFAULT_IRQ;
    d->cs8900_size = p_init->phys_len;

    mod_vm_device_reset(p_m);
    crc32_init();
}

/**
 * mod_vm_device_ETH_CS8900_reset
 * @brief   
 * @param   p_m
 * @return  STD_CALL             std_void_t
 */
STD_CALL std_void_t mod_vm_device_ETH_CS8900_reset(IN mod_vm_device_t *p_m)
{
    mod_vm_device_imp_t *p_eth_cs8900_info = (mod_vm_device_imp_t *)p_m;
    struct cs8900_data *d = &(p_eth_cs8900_info->eth_cs8900_data);
    std_u8_t *ram_base;
    memset(d->internal_ram, 0, sizeof(d->internal_ram));	
    
    /*RESET ETH_CS8900*/
    ram_base = (std_u8_t *) (&(d->internal_ram[0]));
	
    *(std_u32_t *) (ram_base + PP_ProductID) = CS8900A_PRODUCT_ID;
    *(std_u16_t *) (ram_base + PP_ISAIOB) = 0x300;
    *(std_u16_t *) (ram_base + PP_IntNum) = 0x4;
    *(std_u16_t *) (ram_base + PP_IntNum) = 0x4;
	
    *(std_u16_t *) (ram_base + PP_RxCFG) = 0x3;
    *(std_u16_t *) (ram_base + PP_RxEvent) = 0x4;
	
    *(std_u16_t *) (ram_base + PP_RxCTL) = 0x5;
    *(std_u16_t *) (ram_base + PP_TxCFG) = 0x7;
    *(std_u16_t *) (ram_base + PP_TxEvent) = 0x8;
    *(std_u16_t *) (ram_base + 0x108) = 0x9;
	
    *(std_u16_t *) (ram_base + PP_BufCFG) = 0xb;
    *(std_u16_t *) (ram_base + PP_BufEvent) = 0xc;
	
    *(std_u16_t *) (ram_base + PP_RxMISS) = 0x10;
	
    *(std_u16_t *) (ram_base + PP_TxCOL) = 0x12;
    *(std_u16_t *) (ram_base + PP_LineCTL) = 0x13;
    *(std_u16_t *) (ram_base + PP_LineST) = 0x14;
    *(std_u16_t *) (ram_base + PP_SelfCTL) = 0x15;
	
    *(std_u16_t *) (ram_base + PP_SelfST) = 0x16;
    *(std_u16_t *) (ram_base + PP_BusCTL) = 0x17;
	
    *(std_u16_t *) (ram_base + PP_BusST) = 0x18;
    *(std_u16_t *) (ram_base + PP_TestCTL) = 0x19;
	
    *(std_u16_t *) (ram_base + PP_TDR) = 0x1c;
	
    *(std_u16_t *) (ram_base + PP_TxCMD) = 0x9;
	
    *(ram_base + PP_IA) = cs8900a_default_mac[0];
    *(ram_base + PP_IA + 1) = cs8900a_default_mac[1];
    *(ram_base + PP_IA + 2) = cs8900a_default_mac[2];
    *(ram_base + PP_IA + 3) = cs8900a_default_mac[3];
    *(ram_base + PP_IA + 4) = cs8900a_default_mac[4];
    *(ram_base + PP_IA + 5) = cs8900a_default_mac[5];
}

/**
 * mod_vm_device_ETH_CS8900_access
 * @brief   
 * @param   p_m
 * @param   arg
 * @return  STD_CALL             std_void_t  *
 */
STD_CALL std_void_t *mod_vm_device_ETH_CS8900_access(IN mod_vm_device_t *p_m, IN vm_device_access_t *arg)
{
    mod_vm_device_imp_t *p_eth_cs8900_info = (mod_vm_device_imp_t *)p_m;
    struct cs8900_data *d = &(p_eth_cs8900_info->eth_cs8900_data);
    std_u32_t offset = arg->offset;
    std_uint_t op_size = arg->op_size;
    std_uint_t op_type = arg->op_type;
    std_u32_t * data = arg->data;
    std_u8_t * has_set_value  = arg->has_set_value;
    void *ret;
    std_u8_t *ram_base;
    std_u16_t io_address;
    std_u16_t isq;

    ram_base = (std_u8_t *) (&(d->internal_ram[0]));
	
	
    if (offset >= d->cs8900_size) {
        *data = 0;
        return NULL;
    }
	
#if  VALIDE_CS8900_OPERATION
    if (op_type == MTS_WRITE) {
        ASSERT(offset != PP_IO_ISQ, "Write to read only register in CS8900. offset %x\n", offset);
    } else if (op_type == MTS_READ) {
        ASSERT(offset != PP_TX_CMD, "Read write only register in CS8900. offset %x\n", offset);
        ASSERT(offset != PP_TX_LEN, "Read write only register in CS8900. offset %x\n", offset);
    }
#endif
	
    switch (offset)
    {
        case PP_RT_DATA0:
        case PP_RT_DATA0 + 1:
        case PP_RT_DATA1:
        case PP_RT_DATA1 + 1:
            if (op_type == MTS_READ)
            {
                ASSERT(d->rx_read_index < (*(std_u16_t *) (ram_base + PP_RxLength)),
                       "read out of data rx_read_index %x data len %x \n", d->rx_read_index,
                       (*(std_u16_t *) (ram_base + PP_RxLength)));
                ret = (void *) (ram_base + PP_RxFrame + d->rx_read_index);
                d->rx_read_index += op_size;
                /****if read all data,set d->rx_read_index=0*/
                if (d->rx_read_index >= *(std_u16_t *) (ram_base + PP_RxLength))
                    d->rx_read_index = 0;
                return ret;
            }
            else if (op_type == MTS_WRITE)
            {
                ret = (void *) (ram_base + PP_TxFrame + d->tx_send_index);
                if (op_size == MTS_BYTE)
                    *(std_u8_t *) ret = *data;
                if (op_size == MTS_HALF_WORD)
                    *(std_u16_t *) ret = *data;
                else
                    *(std_u32_t *) ret = *data;
                *has_set_value = TRUE;
                d->tx_send_index += op_size;
                /*if write all data into tx buffer, set d->tx_send_index=0 */
                if (d->tx_send_index >= *(std_u16_t *) (ram_base + PP_TxLength))
                {
                    d->tx_send_index = 0;
                    /*start tx a frame */
                    dev_cs8900_tx(d);
                }
                return NULL;
            }
            break;
        case PP_TX_CMD:

            ret = (void *) (ram_base + PP_TxCMD);
            return ret;
        case PP_TX_LEN:
            ret = (void *) (ram_base + PP_TxLength);
            return ret;
        case PP_IO_ISQ:
            ASSERT(0, "not support PP_IO_ISQ \n");
        case PP_ADDRESS:
            return (void *) (ram_base + PP_ADDRESS);
        case PP_DATA0:
        case PP_DATA1:
            if (offset == PP_DATA0)
                ASSERT(op_size == MTS_HALF_WORD, "op_size must be 2. op_size %x\n", op_size);
            else if (offset == PP_DATA1)
                ASSERT(0, "cs8900 only support 16 bit IO operation");
            io_address = *(std_u16_t *) (ram_base + PP_ADDRESS);
            switch (io_address)
            {
                case PP_ProductID:
                    ASSERT(op_type == MTS_READ, "write to read only register %x\n", *(std_u16_t *) (ram_base + PP_ADDRESS));
                    *data = CS8900A_PRODUCT_ID;
                    *has_set_value = TRUE;
                    return NULL;
                case PP_ProductID + 2:   /*16 bit */
                    *data = 0;
                    *has_set_value = TRUE;
                    return NULL;
                case PP_ISAIOB:
                case PP_IntNum:
                    return (void *) (ram_base + io_address);
                case PP_ISASOF:
                case PP_DmaFrameCnt:
                case PP_DmaByteCnt:
                case PP_MemBase:
                case PP_EEPROMCommand:
                case PP_EEPROMData:
                    ASSERT(0, "Not support yet offset %x \n", io_address);
                    break;
                case PP_RxCFG:
                    if (op_type == MTS_WRITE)
                    {
                        if (*data & Skip_1)
                        {
                            memset(ram_base + PP_RxFrame, 0x0, PP_TxFrame - 1 - PP_RxFrame);
                        }
                        *(std_u16_t *) (ram_base + PP_RxCFG) = *data | 0x3;
                        *has_set_value = TRUE;
                        return NULL;
                    }
                    else                   /*read */
                        return (void *) (ram_base + io_address);
                case PP_RxCTL:
                    if (op_type == MTS_WRITE)
                    {
                        *(std_u16_t *) (ram_base + PP_RxCTL) = *data | 0x5;
                        *has_set_value = TRUE;
                        if (*data & IAHashA)
                            ASSERT(0, "Hash dest address is not support yet \n");
                        return NULL;
                    }
                    else
                        return (void *) (ram_base + io_address);

                case PP_TxCFG:
                    if (op_type == MTS_WRITE)
                    {
                        *(std_u16_t *) (ram_base + PP_TxCFG) = *data | 0x7;
                        *has_set_value = TRUE;
                        return NULL;
                    }
                    else
                        return (void *) (ram_base + io_address);
                case 0x108:
                    /*read 0x108 actually read 0x144 TXcmd */
                    ASSERT(op_type == MTS_READ, "CS8900 write to read only register. IO address 0x108 \n");
                    return (void *) (ram_base + 0x144);
                case PP_BufCFG:
                    if (op_type == MTS_WRITE)
                    {
                        if (*data & SWint_X)
                        {
                            *(std_u16_t *) (ram_base + PP_BufEvent) |= SWint;
                            //*(std_u16_t*)(ram_base+PP_ISQ) &= ~0x3f;
                            *(std_u16_t *) (ram_base + PP_ISQ) |= BufEvent;
                            dev_cs8900_gen_interrupt(d);
                        }
                        if (*data & Rdy4TxiE)
                        {
                            /*if host set rdy4tx, we are always ready for tx */
                            *(std_u16_t *) (ram_base + PP_BufEvent) |= Rdy4Tx;
                            //*(std_u16_t*)(ram_base+PP_ISQ) &= ~0x3f;
                            *(std_u16_t *) (ram_base + PP_ISQ) |= BufEvent;
                            dev_cs8900_gen_interrupt(d);
                        }
                        *(std_u16_t *) (ram_base + PP_BufCFG) = *data | 0xb;
                        *has_set_value = TRUE;
                        return NULL;
                    }
                    else
                        return (void *) (ram_base + io_address);

                case PP_LineCTL:
                    if (op_type == MTS_WRITE)
                    {
                        *(std_u16_t *) (ram_base + PP_LineCTL) = *data | 0x13;
                        if ((*data & SerRxON) || (*data & SerTxON))
                            active_timer();
                        *has_set_value = TRUE;
                        return NULL;
                    }
                    else
                        return (void *) (ram_base + io_address);
                case PP_SelfCTL:
                    if (op_type == MTS_WRITE)
                    {
                        if (*data & RESET)
                        {
                            mod_vm_device_reset(p_m);
                        }
                        *(std_u16_t *) (ram_base + PP_SelfCTL) = *data | 0x15;
                        *has_set_value = TRUE;
                        return NULL;
                    }
                    else
                        return (void *) (ram_base + io_address);
                case PP_BusCTL:
                    if (op_type == MTS_WRITE)
                    {
                        *(std_u16_t *) (ram_base + PP_BusCTL) = *data | 0x17;
                        *has_set_value = TRUE;
                        return NULL;
                    }
                    else
                        return (void *) (ram_base + io_address);

                case PP_TestCTL:
                    if (op_type == MTS_WRITE)
                    {
                        *(std_u16_t *) (ram_base + PP_TestCTL) = *data | 0x19;
                        *has_set_value = TRUE;
                        return NULL;
                    }
                    else
                        return (void *) (ram_base + io_address);

                case PP_ISQ:
                    clr_irq();
                    isq = *(std_u16_t *) (ram_base + PP_ISQ);
                    if (op_type == MTS_WRITE)
                    {
                        *(std_u16_t *) (ram_base + PP_ISQ) = 0;
                        *has_set_value = TRUE;
                        return NULL;
                    }
                    /*Readonly? But sometimes, kernel will write to this register. */
                    //ASSERT(op_type == MTS_READ, "wirte to read only register io_address %x.", io_address);
                    /*SHOULD be read */
                    if (isq & TxEvent)
                    {
                        *(std_u16_t *) (ram_base + PP_ISQ) &= ~TxEvent;
                        *(std_u16_t *) data = *(std_u16_t *) (ram_base + PP_TxEvent);
                        *(std_u16_t *) (ram_base + PP_TxEvent) = 0X8;
                        //return (void*)(ram_base+PP_TxEvent); 
                    }
                    else if (isq & RxEvent)
                    {
                        *(std_u16_t *) (ram_base + PP_ISQ) &= ~RxEvent;
                        *(std_u16_t *) data = *(std_u16_t *) (ram_base + PP_RxEvent);
                        *(std_u16_t *) (ram_base + PP_RxEvent) = 0X4;

                        //return (void*)(ram_base+PP_RxEvent); 
                    }
                    else if (isq & BufEvent)
                    {
                        *(std_u16_t *) (ram_base + PP_ISQ) &= ~BufEvent;
                        *(std_u16_t *) data = *(std_u16_t *) (ram_base + PP_BufEvent);
                        *(std_u16_t *) (ram_base + PP_BufEvent) = 0Xc;

                        //return (void*)(ram_base+PP_BufEvent);  
                    }

                    else if (isq & RxMISS)
                    {
                        *(std_u16_t *) (ram_base + PP_ISQ) &= ~RxMISS;
                        *(std_u16_t *) data = *(std_u16_t *) (ram_base + PP_RxMISS);
                        *(std_u16_t *) (ram_base + PP_RxMISS) = 0x10;
                        //return (void*)(ram_base+PP_RxMISS);  
                    }
                    else if (isq & TxCOL)
                    {
                        *(std_u16_t *) (ram_base + PP_ISQ) &= ~TxCOL;
                        *(std_u16_t *) data = *(std_u16_t *) (ram_base + PP_TxCOL);
                        *(std_u16_t *) (ram_base + PP_TxCOL) = 0x12;
                        //return (void*)(ram_base+PP_TxCOL);  
                    }
                    else
                    {
                        return (void *) (ram_base + PP_ISQ);
                    }
                    *has_set_value = TRUE;
                    return NULL;
                    break;
                case PP_RxEvent:
                    /*read rx event will clear it */
                    ASSERT(op_type == MTS_READ, "CS8900 write to read only register. IO address %x \n", io_address);
                    *(std_u16_t *) data = *(std_u16_t *) (ram_base + PP_RxEvent);
                    *has_set_value = TRUE;
                    *(std_u16_t *) (ram_base + PP_RxEvent) = 0X4;
                    return NULL;
                case PP_TxEvent:
                    /*read tx event will clear it */
                    ASSERT(op_type == MTS_READ, "CS8900 write to read only register. IO address %x \n", io_address);
                    *(std_u16_t *) data = *(std_u16_t *) (ram_base + PP_TxEvent);
                    *has_set_value = TRUE;
                    *(std_u16_t *) (ram_base + PP_TxEvent) = 0X8;
                    return NULL;
                case PP_BufEvent:
                    /*read BufEvent event will clear it */
                    ASSERT(op_type == MTS_READ, "CS8900 write to read only register. IO address %x \n", io_address);
                    *(std_u16_t *) data = *(std_u16_t *) (ram_base + PP_BufEvent);
                    *has_set_value = TRUE;
                    *(std_u16_t *) (ram_base + PP_BufEvent) = 0Xc;
                    return NULL;

                case PP_RxMISS:
                case PP_TxCOL:
                case PP_LineST:
                case PP_SelfST:
                case PP_TDR:
                    ASSERT(op_type == MTS_READ, "CS8900 write to read only register. IO address %x \n", io_address);
                    return (void *) (ram_base + io_address);
                case PP_BusST:
                    *(std_u16_t *) (ram_base + PP_BusST) |= Rdy4TxNOW;
                    //else 
                    //{
                    //      *(std_u16_t*)(ram_base+PP_BusST) &= ~Rdy4TxNOW;
                    //}
                    return (void *) (ram_base + io_address);
                case PP_TxCMD:
                    ASSERT(op_type == MTS_WRITE, "CS8900 read write only register. IO address %x \n", PP_TxCMD);
                    *(std_u16_t *) (ram_base + PP_TxCMD) = *data | 0x9;
                    *has_set_value = TRUE;
                    return NULL;

                case PP_TxLength:
                    ASSERT(op_type == MTS_WRITE, "CS8900 read write only register. IO address %x \n", PP_TxLength);
                    *(std_u16_t *) (ram_base + PP_TxLength) = *data;
                    *has_set_value = TRUE;
                    if (*(std_u16_t *) (ram_base + PP_TxLength) > 1518)
                    {
                        *(std_u16_t *) (ram_base + PP_BusST) |= TxBidErr;
                    }
                    else if (*(std_u16_t *) (ram_base + PP_TxLength) > 1514)
                    {
                        if (!((*(std_u16_t *) (ram_base + PP_TxCMD)) & InhibitCRC))
                            *(std_u16_t *) (ram_base + PP_BusST) |= TxBidErr;
                        else
                            *(std_u16_t *) (ram_base + PP_BusST) &= ~TxBidErr;
                    }
                    else
                        *(std_u16_t *) (ram_base + PP_BusST) &= ~TxBidErr;
                    return NULL;
                case PP_LAF:
                case PP_LAF + 1:
                case PP_LAF + 2:
                case PP_LAF + 3:
                case PP_LAF + 4:
                case PP_LAF + 5:
                case PP_LAF + 6:
                case PP_LAF + 7:
                case PP_IA:
                case PP_IA + 1:
                case PP_IA + 2:
                case PP_IA + 3:
                case PP_IA + 4:
                case PP_IA + 5:
                case PP_RxStatus:
                case PP_RxLength:
                    return (void *) (ram_base + io_address);

                default:
                    ASSERT(0, "error io address %x\n", io_address);

            }

    }

    return NULL;
}

/**
 * mod_vm_device_ETH_CS8900_command
 * @brief   
 * @param   p_m
 * @param   arg
 * @return  STD_CALL             std_void_t
 */
STD_CALL std_void_t mod_vm_device_ETH_CS8900_command(IN mod_vm_device_t *p_m, IN std_void_t *arg)
{
    mod_vm_device_imp_t *p_eth_cs8900_info = (mod_vm_device_imp_t *)p_m;
    struct cs8900_data *d = &(p_eth_cs8900_info->eth_cs8900_data);
    
    if (active_time && (expire <= get_clock())){
        active_time = 0;
        dev_cs8900_cb(d);
    }
}

struct mod_vm_device_ops_st mod_vm_device_ETH_CS8900_ops = {
        mod_vm_device_ETH_CS8900_init,
        mod_vm_device_ETH_CS8900_cleanup,

        /***func_ops***/
        mod_vm_device_ETH_CS8900_initiate,
        mod_vm_device_ETH_CS8900_reset,
        mod_vm_device_ETH_CS8900_access,
        mod_vm_device_ETH_CS8900_command,

};

/**
 * mod_vm_device_ETH_CS8900_create_instance
 * @brief   
 * @param   pp_handle
 * @return  STD_CALL std_rv_t
 */
STD_CALL std_rv_t mod_vm_device_ETH_CS8900_create_instance(INOUT std_void_t **pp_handle)
{
    mod_vm_device_imp_t *p_m = NULL;

    p_m = (mod_vm_device_imp_t *) CALLOC(1, sizeof(mod_vm_device_imp_t));
    p_m->unique_id = std_random_u64();
    p_m->p_ops = &mod_vm_device_ETH_CS8900_ops;

    mod_ownership_register_ops((mod_ownership_t *) p_m);
    mod_ownership_init((mod_ownership_t *) p_m);
    *pp_handle = p_m;

    return STD_RV_SUC;
}
