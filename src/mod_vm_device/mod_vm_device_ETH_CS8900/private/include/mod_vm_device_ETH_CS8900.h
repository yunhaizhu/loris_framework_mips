/**
 * Copyright (c) 2021 Yunhai Zhu <yunhaia2@gmail.com>
 *
 * see COPYRIGHT file.
 */

/**
 * @file    mod_vm_device_ETH_CS8900.h
 * @brief   define structure & functions
 * @version 1.0
 * @author  Yunhai Zhu
 * @date    2021-12-29
 *
 */
#ifndef MOD_VM_DEVICE_ETH_CS8900_H
#define MOD_VM_DEVICE_ETH_CS8900_H

#include "mod_shell.h"
#include "mod_vm_device.h"



#define CS8900_DEFAULT_RX_TIMEOUT  40
#define CS8900_MIN_RX_TIMEOUT  20
#define CS8900_MAX_RX_TIMEOUT  100
#define CS8900_RX_TIMEOUT_STEP 5

static std_u32_t cs8900a_rx_timeout = CS8900_DEFAULT_RX_TIMEOUT;

/* Maximum packet size */
#define CS8900_MAX_PKT_SIZE     1518
#define CS8900_MIN_PKT_SIZE     8
#define CS8900_RUN_PKT_SIZE     64

#define CS8900A_PRODUCT_ID  0x630e      /*little endian */

#define PP_RT_DATA0              0x00
#define PP_RT_DATA1              0x02
#define PP_TX_CMD              0X04
#define PP_TX_LEN              0X06
#define PP_IO_ISQ              0X08
#define PP_ADDRESS              0x0a    /* PacketPage Pointer Port (Section 4.10.10) */
#define PP_DATA0                 0x0c   /* PacketPage Data Port (Section 4.10.10) */
#define PP_DATA1               0X0e

#define PP_ProductID            0x0000  /* Section 4.3.1   Product Identification Code */
#define PP_ISAIOB 					0x0020  /*  IO base address */
#define PP_IntNum                       0x0022  /* Section 3.2.3   Interrupt Number */
#define PP_ISASOF						 0x0026 /*  ISA DMA offset */
#define PP_DmaFrameCnt 				0x0028  /*  ISA DMA Frame count */
#define PP_DmaByteCnt 					0x002A  /*  ISA DMA Byte count */
#define PP_MemBase                      0x002c  /* Section 4.9.2   Memory Base Address Register */
#define PP_EEPROMCommand        0x0040  /* Section 4.3.11  EEPROM Command */
#define PP_EEPROMData           0x0042  /* Section 4.3.12  EEPROM Data */


#define PP_RxCFG                        0x0102  /* Section 4.4.6   Receiver Configuration */
#define PP_RxCTL                        0x0104  /* Section 4.4.8   Receiver Control */
#define PP_TxCFG                        0x0106  /* Section 4.4.9   Transmit Configuration */
#define PP_BufCFG                       0x010a  /* Section 4.4.12  Buffer Configuration */
#define PP_LineCTL                      0x0112  /* Section 4.4.16  Line Control */
#define PP_SelfCTL                      0x0114  /* Section 4.4.18  Self Control */
#define PP_BusCTL                       0x0116  /* Section 4.4.20  Bus Control */
#define PP_TestCTL                      0x0118  /* Section 4.4.22  Test Control */
#define PP_AutoNegCTL 				0x011C  /*  Auto Negotiation Ctrl */
#define PP_ISQ                            0x0120        /* Section 4.4.5   Interrupt Status Queue */
#define PP_RxEvent 							0x0124  /*  Rx Event Register */
#define PP_TxEvent                      0x0128  /* Section 4.4.10  Transmitter Event */
#define PP_BufEvent                     0x012c  /* Section 4.4.13  Buffer Event */
#define PP_RxMISS                       0x0130  /* Section 4.4.14  Receiver Miss Counter */
#define PP_TxCOL                        0x0132  /* Section 4.4.15  Transmit Collision Counter */
#define PP_LineST							 0x0134 /*  Line State Register */
#define PP_SelfST                       0x0136  /* Section 4.4.19  Self Status */
#define PP_BusST                        0x0138  /* Section 4.4.21  Bus Status */
#define PP_TDR 								0x013C  /*  Time Domain Reflectometry */
#define PP_AutoNegST 				0x013E  /*  Auto Neg Status */
#define PP_TxCMD                        0x0144  /* Section 4.4.11  Transmit Command */
#define PP_TxLength                     0x0146  /* Section 4.5.2   Transmit Length */
#define PP_LAF								 0x0150 /*  Hash Table */
#define PP_IA                           	 0x0158 /* Section 4.6.2   Individual Address (IEEE Address) */

#define PP_RxStatus                     0x0400  /* Section 4.7.1   Receive Status */
#define PP_RxLength                     0x0402  /* Section 4.7.1   Receive Length (in bytes) */
#define PP_RxFrame                      0x0404  /* Section 4.7.2   Receive Frame Location */
#define PP_TxFrame                      0x0a00  /* Section 4.7.2   Transmit Frame Location */

/* PP_RxCFG */
#define Skip_1                  0x0040
#define StreamE                 0x0080
#define RxOKiE                  0x0100
#define RxDMAonly               0x0200
#define AutoRxDMAE              0x0400
#define BufferCRC               0x0800
#define CRCerroriE              0x1000
#define RuntiE                  0x2000
#define ExtradataiE             0x4000


/* PP_TxCFG */
#define Loss_of_CRSiE   0x0040
#define SQErroriE               0x0080
#define TxOKiE                  0x0100
#define Out_of_windowiE 0x0200
#define JabberiE                0x0400
#define AnycolliE               0x0800
#define T16colliE               0x8000

/* PP_BufCFG */
#define SWint_X                 0x0040
#define RxDMAiE                 0x0080
#define Rdy4TxiE                0x0100
#define TxUnderruniE    0x0200
#define RxMissiE                0x0400
#define Rx128iE                 0x0800
#define TxColOvfiE              0x1000
#define MissOvfloiE             0x2000
#define RxDestiE                0x8000

/* PP_RxCTL */
#define IAHashA                 0x0040
#define PromiscuousA    0x0080
#define RxOKA                   0x0100
#define MulticastA              0x0200
#define IndividualA             0x0400
#define BroadcastA              0x0800
#define CRCerrorA               0x1000
#define RuntA                   0x2000
#define ExtradataA              0x4000

/* PP_SelfCTL */
#define RESET                   0x0040
#define SWSuspend               0x0100
#define HWSleepE                0x0200
#define HWStandbyE              0x0400
#define HC0E                    0x1000
#define HC1E                    0x2000
#define HCB0                    0x4000
#define HCB1                    0x8000

/* PP_LineCTL */
#define SerRxON                 0x0040
#define SerTxON                 0x0080
#define AUIonly                 0x0100
#define AutoAUI_10BT    0x0200
#define ModBackoffE             0x0800
#define PolarityDis             0x1000
#define L2_partDefDis   0x2000
#define LoRxSquelch             0x4000

/* PP_TxEvent */
#define Loss_of_CRS             0x0040
#define SQEerror                0x0080
#define TxOK                    0x0100
#define Out_of_window   0x0200
#define Jabber                  0x0400
#define T16coll                 0x8000

#define RxEvent                 0x0004
#define TxEvent                 0x0008
#define BufEvent                0x000c
#define RxMISS                  0x0010
#define TxCOL                   0x0012

/* PP_BufEvent */
#define SWint                   0x0040
#define RxDMAFrame              0x0080
#define Rdy4Tx                  0x0100
#define TxUnderrun              0x0200
#define RxMiss                  0x0400
#define Rx128                   0x0800
#define RxDest                  0x8000

/* PP_TxCMD */
#define After5                  0
#define After381                1
#define After1021               2
#define AfterAll                3
#define TxStart(x) ((x) << 6)

#define Force                   0x0100
#define Onecoll                 0x0200
#define InhibitCRC              0x1000
#define TxPadDis                0x2000


/* PP_BusST */
#define TxBidErr                0x0080
#define Rdy4TxNOW               0x0100


/* Ethernet Constants */
#define N_ETH_ALEN  6
#define N_ETH_HLEN  sizeof(n_eth_hdr_t)

/* Ethernet Address */
typedef struct
{
    std_u8_t eth_addr_byte[N_ETH_ALEN];
} __attribute__ ((__packed__)) n_eth_addr_t;
/* Ethernet Header */
typedef struct
{
    n_eth_addr_t daddr;          /* destination eth addr */
    n_eth_addr_t saddr;          /* source ether addr    */
    std_u16_t type;             /* packet type ID field */
} __attribute__ ((__packed__)) n_eth_hdr_t;

#define CS8900_INTERNAL_RAM_SIZE   0x1000       /*4K */
/* CS8900 Data */
typedef struct cs8900_data
{
    std_char_t *name;
    std_u32_t cs8900_size;

    /* NetIO descriptor */
    std_int_t netio_fd;           /*one nio can have multi listener */

    /*internal RAM 4K bytes */
    std_u32_t internal_ram[CS8900_INTERNAL_RAM_SIZE / 4];
    std_u32_t irq_no;

    std_u16_t rx_read_index;
    std_u16_t tx_send_index;

    //vp_timer_t *cs8900_timer;
}cs8900_data_t;
typedef struct mod_vm_device_imp_st {
    mod_ownership_t ownership;
    std_u64_t unique_id;
    struct mod_vm_device_ops_st *p_ops;
    vm_device_info_t info;
    mod_shell_t *p_mod_shell;

    cs8900_data_t eth_cs8900_data;
} mod_vm_device_imp_t;

#define CS8900_DEFAULT_IRQ  9

#define CRC32_POLY  0xedb88320L

/* MTS operation */
#define MTS_READ  0
#define MTS_WRITE 1

#define MTS_BYTE           1
#define MTS_HALF_WORD      2
#define MTS_WORD           4

/****shell_interface*****/

/**
 * shell_stub_mod_vm_device_ETH_CS8900_initiate
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_device_ETH_CS8900_initiate(IN std_void_t *p_handle,
                                                                         IN std_char_t *params);

/**
 * shell_stub_mod_vm_device_ETH_CS8900_reset
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_device_ETH_CS8900_reset(IN std_void_t *p_handle, IN std_char_t *params);

/**
 * shell_stub_mod_vm_device_ETH_CS8900_access
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_device_ETH_CS8900_access(IN std_void_t *p_handle,
                                                                       IN std_char_t *params);

/**
 * shell_stub_mod_vm_device_ETH_CS8900_command
 * @brief   
 * @param   p_handle
 * @param   params
 * @return  extern STD_CALL std_char_t *
 */
extern STD_CALL std_char_t *shell_stub_mod_vm_device_ETH_CS8900_command(IN std_void_t *p_handle,
                                                                        IN std_char_t *params);

/****rpc_service_interface*****/

/**
 * mod_vm_device_ETH_CS8900_create_instance
 * @brief   
 * @param   pp_handle
 * @return  extern std_int_t
 */
extern std_int_t mod_vm_device_ETH_CS8900_create_instance(INOUT std_void_t **pp_handle);

#endif
