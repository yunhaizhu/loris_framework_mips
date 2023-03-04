/**
 * Copyright (c) 2021 Yunhai Zhu <yunhaia2@gmail.com>
 *
 * see COPYRIGHT file.
 */

/**
 * @file    mod_vm_device_NORFLASH4M.c
 * @brief   implement functions
 * @version 1.0
 * @author  Yunhai Zhu
 * @date    2021-12-29
 *
 */

#include "mod_vm_device_NORFLASH4M.h"
#include "mod_vm_arch_mips_cp0.h"
#include "mod_vm_memory.h"
#include <sys/mman.h>

extern mod_vm_arch_mips_cp0_t *p_global_vm_arch_mips_cp0;
/**
 * mod_vm_device_NORFLASH4M_init
 * @brief   
 * @param   p_m
 * @param   arg
 * @param   arg_len
 * @return  STD_CALL std_rv_t
 */
STD_CALL std_rv_t mod_vm_device_NORFLASH4M_init(IN mod_vm_device_t *p_m, IN const std_char_t *arg,
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
                       shell_stub_mod_vm_device_NORFLASH4M_initiate, p_imp_m);

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_device_reset");
    mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)),
                       shell_stub_mod_vm_device_NORFLASH4M_reset, p_imp_m);

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_device_access");
    mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)),
                       shell_stub_mod_vm_device_NORFLASH4M_access, p_imp_m);

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_device_command");
    mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)),
                       shell_stub_mod_vm_device_NORFLASH4M_command, p_imp_m);

    return STD_RV_SUC;
}

/**
 * mod_vm_device_NORFLASH4M_cleanup
 * @brief   
 * @param   p_m
 * @return  STD_CALL std_rv_t
 */
STD_CALL std_rv_t mod_vm_device_NORFLASH4M_cleanup(mod_vm_device_t *p_m)
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
std_u16_t vendorID = 0x01;   // target is little end   0x0001
std_u16_t deviceID = 0x22f9; // target is little end   0x22F9
std_u16_t earse_ready = 0x80;//  target is little end    0X0080
std_u32_t last_offset = 0;

std_u32_t dump_data;
std_u32_t cfi_data[] = {
        0x51, 0x52, 0x59, 0x2, 0x0, 0x40, 0x0, 0x0, 0x0, 0x0, 0x0, 0x27, 0x36, 0x0, 0x0, 0x4,
        0x0, 0xa, 0x0, 0x5, 0x0, 0x4, 0x0, 0x16, 0x2, 0x0, 0x0, 0x0, 0x2, 0x7, 0x0, 0x20,
        0x0, 0x3e, 0x0, 0x0, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
        0x50, 0x52, 0x49, 0x31, 0x31, 0x0, 0x2, 0x4, 0x1, 0x4, 0x0, 0x0, 0x0, 0xb5, 0xc5, 0x2,//02 BOTTOM
};

/**
 * secotor_info
 * @brief   
 * @param   offset
 * @param   sector_start
 * @param   sector_size
 * @return  static void
 */
static void secotor_info(std_u32_t offset, std_u32_t *sector_start, std_u32_t *sector_size)
{
    if (offset <= 0x00FFF) {
        *sector_start = offset & 0xFFFFFE00;
        *sector_size = 0x2000;
        return;
    }

    *sector_start = offset & 0xffff0000;
    *sector_size = 0x10000;

    return;
}

/**
 * dev_flash_load
 * @brief   
 * @param   flash_file_name
 * @param   flash_len
 * @param   flash_data_hp
 * @param   create
 * @return  static int
 */
static int dev_flash_load(std_char_t *flash_file_name, std_u32_t flash_len, std_uchar_t **flash_data_hp, std_uint_t create)
{
    std_int_t fd;
    struct stat sb;
    std_uchar_t *temp;

    fd = open(flash_file_name, O_RDWR);
    if ((fd < 0) && (create == 1)) {
        fprintf(stderr, "Can not open flash file. name %s\n", flash_file_name);
        fprintf(stderr, "creating flash file. name %s\n", flash_file_name);
        fd = open(flash_file_name, O_RDWR | O_CREAT, S_IREAD | S_IWRITE | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
        if (fd < 0) {
            fprintf(stderr, "Can not create flash file. name %s\n", flash_file_name);
            return -1;
        }

        temp = malloc(flash_len);
        assert(temp != NULL);
        memset(temp, 0xff, flash_len);
        lseek(fd, 0, SEEK_SET);
        write(fd, (void *) temp, flash_len);
        free(temp);
        fprintf(stderr, "create flash file success. name %s\n", flash_file_name);
        lseek(fd, 0, SEEK_SET);
    } else if (fd < 0) {
        fprintf(stderr, "%s does not exist and not allowed to create.\n", flash_file_name);
        return -1;
    }

    assert(fd >= 0);
    fstat(fd, &sb);

    if (flash_len < sb.st_size) {
        fprintf(stderr, "Too large flash file. flash len:%d M, flash file name %s,"
                        "flash file legth: %d bytes.\n",
                flash_len, flash_file_name, (int) sb.st_size);
        return -1;
    }

    *flash_data_hp = mmap(NULL, sb.st_size, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
    if (*flash_data_hp == MAP_FAILED) {
        fprintf(stderr, "errno %d\n", errno);
        fprintf(stderr, "failed\n");
        return -1;
    }

    return 0;
}
/**
 * mod_vm_device_NORFLASH4M_initiate
 * @brief   
 * @param   p_m
 * @param   arg
 * @return  STD_CALL             std_void_t
 */
STD_CALL std_void_t mod_vm_device_NORFLASH4M_initiate(IN mod_vm_device_t *p_m, IN vm_device_info_t *arg)
{
    vm_device_info_t *p_init = &p_m->info;
    mod_vm_device_imp_t *p_int_info = (mod_vm_device_imp_t *) p_m;
    unsigned char *flash_data_hp = NULL;

    /*load rom data */
    dev_flash_load("flash.bin", 4 * 1048576, &flash_data_hp, 1);

    p_int_info->flash.flash_ptr = flash_data_hp;
    p_int_info->flash.flash_size = 4 * 1048576;
    p_int_info->flash.state = ROM_INIT_STATE;

    memcpy(p_init, arg, sizeof(vm_device_info_t));

    p_init->flags = VDEVICE_FLAG_NO_MTS_MMAP;

    mod_vm_device_reset(p_m);
}

/**
 * mod_vm_device_NORFLASH4M_reset
 * @brief   
 * @param   p_m
 * @return  STD_CALL             std_void_t
 */
STD_CALL std_void_t mod_vm_device_NORFLASH4M_reset(IN mod_vm_device_t *p_m)
{
    return;
}

/**
 * mod_vm_device_NORFLASH4M_access
 * @brief   
 * @param   p_m
 * @param   arg
 * @return  STD_CALL             std_void_t  *
 */
STD_CALL std_void_t *mod_vm_device_NORFLASH4M_access(IN mod_vm_device_t *p_m, IN vm_device_access_t *arg)
{
    mod_vm_device_imp_t *p_int_info = (mod_vm_device_imp_t *) p_m;
    flash_data_t *d = &(p_int_info->flash);
    std_u32_t r_offset;
    std_u32_t sector_size;
    std_u32_t sector_start;
    std_u32_t last_sector_start;
    std_u32_t last_sector_size;
    std_u32_t offset = arg->offset;
    std_uint_t op_size = arg->op_size;
    std_uint_t op_type = arg->op_type;
    std_u32_t *data = arg->data;
    std_u8_t *has_set_value = arg->has_set_value;

    if (offset >= d->flash_size) {
        *data = 0;
        return NULL;
    }
    if (op_type == MTS_READ) {
        switch (d->state) {
            case 0:
                return (BPTR(d, offset));
            case 99: /*cfi query */
                if ((offset >= 0x20) && (offset <= 0x9e)) {
                    *data = (cfi_data[(offset - 0x20) / 2]);//always littleend
                    *has_set_value = TRUE;
                } else {
                    d->state = 0;
                    return (BPTR(d, offset));
                }

                break;

            case 0x6:
                d->state = 0;
                if (offset == 0X0)
                    return &vendorID;
                if (offset == 0X2)
                    return &deviceID;
                break;
            case 10:
                //last cycle is chip erase or sector erase
                secotor_info(offset, &sector_start, &sector_size);
                secotor_info(last_offset, &last_sector_start, &last_sector_size);
                d->state = 0;
                if (last_sector_start == sector_start)
                    return &earse_ready;
                else
                    return (BPTR(d, offset));

            default:
                break;
        }

        return NULL;
    }


    if (op_type == MTS_WRITE) {
        r_offset = offset;
        if ((op_size == MTS_HALF_WORD) && (offset == 0X554))
            offset = 0X555;

        switch (d->state) {
            case ROM_INIT_STATE:
                switch (offset) {
                    case 0xAAA:
                        if (((*data) & 0xff) == 0xAA)
                            d->state = 1;
                        break;
                    case 0XAA:
                        if (((*data) & 0xff) == 0x98) {
                            d->state = 99;//CFI QUERY
                        }
                        break;
                    default:
                        switch ((*data) & 0xff) {
                            case 0xB0:
                                /* Erase/Program Suspend */
                                d->state = 0;
                                break;
                            case 0x30:
                                /* Erase/Program Resume */
                                d->state = 0;
                                break;
                            case 0xF0:
                            case 0xFF:
                                /*Read/Reset */
                                d->state = 0;
                                break;
                            default:
                                return ((void *) (d->flash_ptr + r_offset));
                        }
                }
                break;

            case 99:
                if (((*data & 0xff) == 0xff) || ((*data & 0xff) == 0xf0))
                    d->state = 0;
                else
                    return ((void *) (d->flash_ptr + r_offset));
                break;
            case 1:
                if ((offset != 0x555) && ((*data & 0xff) != 0x55))
                    d->state = 0;
                else
                    d->state = 2;
                break;

            case 2:
                d->state = 0;

                if (offset == 0xAAA) {
                    switch ((*data) & 0xff) {
                        case 0x80:
                            d->state = 3;
                            break;
                        case 0xA0:
                            /* Byte/Word program */
                            d->state = 4;
                            break;
                        case 0x90:
                            /* Product ID Entry / Status of Block B protection */
                            d->state = 6;
                            break;
                        default:
                            break;
                    }
                }
                break;

            case 3:
                if ((offset != 0xAAA) && (*data != 0xAA))
                    d->state = 0;
                else
                    d->state = 8;
                break;


            case 8:
                if ((offset != 0x555) && (*data != 0x55))
                    d->state = 0;
                else
                    d->state = 9;
                break;

            case 9:
                d->state = 10;
                last_offset = r_offset;

                switch ((*data) & 0xff) {
                    case 0x10:
                        /* Chip Erase */
                        memset(BPTR(d, 0), 0, d->flash_size);
                        break;

                    case 0x30:
                        /* Sector Erase */
                        secotor_info(r_offset, &sector_start, &sector_size);
                        break;

                    default:
                        break;
                }
                break;

                /* Byte/Word Program */
            case 4:
                d->state = 0;
                return ((void *) (d->flash_ptr + r_offset));

            default:
                break;
        }

        return &dump_data;
    }

    return NULL;
}

/**
 * mod_vm_device_NORFLASH4M_command
 * @brief   
 * @param   p_m
 * @param   arg
 * @return  STD_CALL             std_void_t
 */
STD_CALL std_void_t mod_vm_device_NORFLASH4M_command(IN mod_vm_device_t *p_m, IN std_void_t *arg)
{
    return;
}

struct mod_vm_device_ops_st mod_vm_device_NORFLASH4M_ops = {
        mod_vm_device_NORFLASH4M_init,
        mod_vm_device_NORFLASH4M_cleanup,

        /***func_ops***/
        mod_vm_device_NORFLASH4M_initiate,
        mod_vm_device_NORFLASH4M_reset,
        mod_vm_device_NORFLASH4M_access,
        mod_vm_device_NORFLASH4M_command,

};

/**
 * mod_vm_device_NORFLASH4M_create_instance
 * @brief   
 * @param   pp_handle
 * @return  STD_CALL std_rv_t
 */
STD_CALL std_rv_t mod_vm_device_NORFLASH4M_create_instance(INOUT std_void_t **pp_handle)
{
    mod_vm_device_imp_t *p_m = NULL;

    p_m = (mod_vm_device_imp_t *) CALLOC(1, sizeof(mod_vm_device_imp_t));
    p_m->unique_id = std_random_u64();
    p_m->p_ops = &mod_vm_device_NORFLASH4M_ops;

    mod_ownership_register_ops((mod_ownership_t *) p_m);
    mod_ownership_init((mod_ownership_t *) p_m);
    *pp_handle = p_m;

    return STD_RV_SUC;
}
