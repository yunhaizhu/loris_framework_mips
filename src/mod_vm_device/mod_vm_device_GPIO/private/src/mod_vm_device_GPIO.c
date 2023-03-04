/**
 * Copyright (c) 2021 Yunhai Zhu <yunhaia2@gmail.com>
 *
 * see COPYRIGHT file.
 */

/**
 * @file    mod_vm_device_GPIO.c
 * @brief   implement functions
 * @version 1.0
 * @author  Yunhai Zhu
 * @date    2021-12-29
 *
 */

#include "mod_vm_device_GPIO.h"
#include "mod_vm_memory.h"

std_u32_t jz4740_gpio_table[JZ4740_GPIO_INDEX_MAX];
/**
 * mod_vm_device_GPIO_init
 * @brief   
 * @param   p_m
 * @param   arg
 * @param   arg_len
 * @return  STD_CALL std_rv_t
 */
STD_CALL std_rv_t mod_vm_device_GPIO_init(IN mod_vm_device_t *p_m, IN const std_char_t *arg, IN std_int_t arg_len)
{
    mod_vm_device_imp_t *p_imp_m = (mod_vm_device_imp_t *) p_m;
    mod_iid_t mod_shell_iid = MOD_SHELL_IID;
    std_char_t key[BUF_SIZE_128] = "\0";

    mod_create_instance(&mod_shell_iid, (std_void_t **) &p_imp_m->p_mod_shell, (mod_ownership_t *) p_imp_m);

    mod_shell_init(p_imp_m->p_mod_shell, NULL, 0);

    /****shell_register******/

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_device_initiate");
    mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)),
                       shell_stub_mod_vm_device_GPIO_initiate, p_imp_m);

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_device_reset");
    mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)),
                       shell_stub_mod_vm_device_GPIO_reset, p_imp_m);

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_device_access");
    mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)),
                       shell_stub_mod_vm_device_GPIO_access, p_imp_m);

    snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_device_command");
    mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)),
                       shell_stub_mod_vm_device_GPIO_command, p_imp_m);

    return STD_RV_SUC;
}

/**
 * mod_vm_device_GPIO_cleanup
 * @brief   
 * @param   p_m
 * @return  STD_CALL std_rv_t
 */
STD_CALL std_rv_t mod_vm_device_GPIO_cleanup(mod_vm_device_t *p_m)
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
#define ASSERT(a, format, args...)                                       \
    do {                                                                 \
        if ((format != NULL) && (!(a))) fprintf(stderr, format, ##args); \
        assert((a));                                                     \
    } while (0)
/**
 * dev_jz4740_gpio_setirq
 * @brief   
 * @param   irq
 */
void dev_jz4740_gpio_setirq(int irq)
{
    int group_no;
    int pin_no;

    ASSERT((irq >= IRQ_GPIO_0) && (irq < IRQ_GPIO_0 + 128), "wrong gpio irq 0x%x\n", irq);

    group_no = (irq - IRQ_GPIO_0) / 32;
    pin_no = (irq - IRQ_GPIO_0) % 32;
    jz4740_gpio_table[GPIO_PXFLG(group_no) / 4] |= 1 << pin_no;
}

/**
 * dev_jz4740_gpio_clearirq
 * @brief   
 * @param   irq
 */
void dev_jz4740_gpio_clearirq(int irq)
{
    int group_no;
    int pin_no;

    ASSERT((irq >= IRQ_GPIO_0) && (irq < IRQ_GPIO_0 + 128), "wrong gpio irq 0x%x\n", irq);

    group_no = (irq - IRQ_GPIO_0) / 32;
    pin_no = (irq - IRQ_GPIO_0) % 32;
    jz4740_gpio_table[GPIO_PXFLG(group_no) / 4] &= ~(1 << pin_no);
}
/**
 * mod_vm_device_GPIO_initiate
 * @brief   
 * @param   p_m
 * @param   arg
 * @return  STD_CALL             std_void_t
 */
STD_CALL std_void_t mod_vm_device_GPIO_initiate(IN mod_vm_device_t *p_m, IN vm_device_info_t *arg)
{
    vm_device_info_t *p_init = &p_m->info;
    mod_vm_device_imp_t *p_gpio_info = (mod_vm_device_imp_t *) p_m;

    memcpy(p_init, arg, sizeof(vm_device_info_t));

    p_init->flags = VDEVICE_FLAG_NO_MTS_MMAP;
    p_gpio_info->gpio_data.jz4740_gpio_ptr = (std_u8_t *) &jz4740_gpio_table[0];
    p_gpio_info->gpio_data.jz4740_gpio_size = p_init->phys_len;

    mod_vm_device_reset(p_m);
}

/**
 * mod_vm_device_GPIO_reset
 * @brief   
 * @param   p_m
 * @return  STD_CALL             std_void_t
 */
STD_CALL std_void_t mod_vm_device_GPIO_reset(IN mod_vm_device_t *p_m)
{
    /*RESET GPIO*/
    memset(jz4740_gpio_table, 0x0, sizeof(jz4740_gpio_table));

    jz4740_gpio_table[GPIO_PXIM(0) / 4] = 0xffffffff;
    jz4740_gpio_table[GPIO_PXIM(1) / 4] = 0xffffffff;
    jz4740_gpio_table[GPIO_PXIM(2) / 4] = 0xffffffff;
    jz4740_gpio_table[GPIO_PXIM(3) / 4] = 0xffffffff;
}

/**
 * mod_vm_device_GPIO_access
 * @brief   
 * @param   p_m
 * @param   arg
 * @return  STD_CALL             std_void_t  *
 */
STD_CALL std_void_t *mod_vm_device_GPIO_access(IN mod_vm_device_t *p_m, IN vm_device_access_t *arg)
{
    mod_vm_device_imp_t *p_gpio_info = (mod_vm_device_imp_t *) p_m;
    struct jz4740_gpio_data *d = &(p_gpio_info->gpio_data);

    std_u32_t offset = arg->offset;
    std_uint_t op_size = arg->op_size;
    std_uint_t op_type = arg->op_type;
    std_u32_t *data = arg->data;
    std_u8_t *has_set_value = arg->has_set_value;

    std_u32_t group;
    std_u32_t mask;
    std_u32_t mask_data;

    if (offset >= d->jz4740_gpio_size) {
        *data = 0;
        return NULL;
    }

#if VALIDE_GPIO_OPERATION
    if (op_type == MTS_WRITE) {
        ASSERT(offset != GPIO_PXPIN(0), "Write to read only register in GPIO. offset %x\n", offset);
        ASSERT(offset != GPIO_PXPIN(1), "Write to read only register in GPIO. offset %x\n", offset);
        ASSERT(offset != GPIO_PXPIN(2), "Write to read only register in GPIO. offset %x\n", offset);
        ASSERT(offset != GPIO_PXPIN(3), "Write to read only register in GPIO. offset %x\n", offset);

        ASSERT(offset != GPIO_PXDAT(0), "Write to read only register in GPIO. offset %x\n", offset);
        ASSERT(offset != GPIO_PXDAT(1), "Write to read only register in GPIO. offset %x\n", offset);
        ASSERT(offset != GPIO_PXDAT(2), "Write to read only register in GPIO. offset %x\n", offset);
        ASSERT(offset != GPIO_PXDAT(3), "Write to read only register in GPIO. offset %x\n", offset);

        ASSERT(offset != GPIO_PXIM(0), "Write to read only register in GPIO. offset %x\n", offset);
        ASSERT(offset != GPIO_PXIM(1), "Write to read only register in GPIO. offset %x\n", offset);
        ASSERT(offset != GPIO_PXIM(2), "Write to read only register in GPIO. offset %x\n", offset);
        ASSERT(offset != GPIO_PXIM(3), "Write to read only register in GPIO. offset %x\n", offset);

        ASSERT(offset != GPIO_PXPE(0), "Write to read only register in GPIO. offset %x\n", offset);
        ASSERT(offset != GPIO_PXPE(1), "Write to read only register in GPIO. offset %x\n", offset);
        ASSERT(offset != GPIO_PXPE(2), "Write to read only register in GPIO. offset %x\n", offset);
        ASSERT(offset != GPIO_PXPE(3), "Write to read only register in GPIO. offset %x\n", offset);

        ASSERT(offset != GPIO_PXFUN(0), "Write to read only register in GPIO. offset %x\n", offset);
        ASSERT(offset != GPIO_PXFUN(1), "Write to read only register in GPIO. offset %x\n", offset);
        ASSERT(offset != GPIO_PXFUN(2), "Write to read only register in GPIO. offset %x\n", offset);
        ASSERT(offset != GPIO_PXFUN(3), "Write to read only register in GPIO. offset %x\n", offset);

        ASSERT(offset != GPIO_PXSEL(0), "Write to read only register in GPIO. offset %x\n", offset);
        ASSERT(offset != GPIO_PXSEL(1), "Write to read only register in GPIO. offset %x\n", offset);
        ASSERT(offset != GPIO_PXSEL(2), "Write to read only register in GPIO. offset %x\n", offset);
        ASSERT(offset != GPIO_PXSEL(3), "Write to read only register in GPIO. offset %x\n", offset);

        ASSERT(offset != GPIO_PXDIR(0), "Write to read only register in GPIO. offset %x\n", offset);
        ASSERT(offset != GPIO_PXDIR(1), "Write to read only register in GPIO. offset %x\n", offset);
        ASSERT(offset != GPIO_PXDIR(2), "Write to read only register in GPIO. offset %x\n", offset);
        ASSERT(offset != GPIO_PXDIR(3), "Write to read only register in GPIO. offset %x\n", offset);

        ASSERT(offset != GPIO_PXTRG(0), "Write to read only register in GPIO. offset %x\n", offset);
        ASSERT(offset != GPIO_PXTRG(1), "Write to read only register in GPIO. offset %x\n", offset);
        ASSERT(offset != GPIO_PXTRG(2), "Write to read only register in GPIO. offset %x\n", offset);
        ASSERT(offset != GPIO_PXTRG(3), "Write to read only register in GPIO. offset %x\n", offset);

        ASSERT(offset != GPIO_PXFLG(0), "Write to read only register in GPIO. offset %x\n", offset);
        ASSERT(offset != GPIO_PXFLG(1), "Write to read only register in GPIO. offset %x\n", offset);
        ASSERT(offset != GPIO_PXFLG(2), "Write to read only register in GPIO. offset %x\n", offset);
        ASSERT(offset != GPIO_PXFLG(3), "Write to read only register in GPIO. offset %x\n", offset);
    }
    if (op_type == MTS_READ) {
        ASSERT(offset != GPIO_PXDATS(0), "Read write only register in GPIO. offset %x\n", offset);
        ASSERT(offset != GPIO_PXDATS(0), "Read write only register in GPIO. offset %x\n", offset);
        ASSERT(offset != GPIO_PXDATS(0), "Read write only register in GPIO. offset %x\n", offset);
        ASSERT(offset != GPIO_PXDATS(0), "Read write only register in GPIO. offset %x\n", offset);

        ASSERT(offset != GPIO_PXDATC(0), "Read write only register in GPIO. offset %x\n", offset);
        ASSERT(offset != GPIO_PXDATC(0), "Read write only register in GPIO. offset %x\n", offset);
        ASSERT(offset != GPIO_PXDATC(0), "Read write only register in GPIO. offset %x\n", offset);
        ASSERT(offset != GPIO_PXDATC(0), "Read write only register in GPIO. offset %x\n", offset);

        ASSERT(offset != GPIO_PXIMS(0), "Read write only register in GPIO. offset %x\n", offset);
        ASSERT(offset != GPIO_PXIMS(0), "Read write only register in GPIO. offset %x\n", offset);
        ASSERT(offset != GPIO_PXIMS(0), "Read write only register in GPIO. offset %x\n", offset);
        ASSERT(offset != GPIO_PXIMS(0), "Read write only register in GPIO. offset %x\n", offset);

        ASSERT(offset != GPIO_PXIMC(0), "Read write only register in GPIO. offset %x\n", offset);
        ASSERT(offset != GPIO_PXIMC(0), "Read write only register in GPIO. offset %x\n", offset);
        ASSERT(offset != GPIO_PXIMC(0), "Read write only register in GPIO. offset %x\n", offset);
        ASSERT(offset != GPIO_PXIMC(0), "Read write only register in GPIO. offset %x\n", offset);

        ASSERT(offset != GPIO_PXPES(0), "Read write only register in GPIO. offset %x\n", offset);
        ASSERT(offset != GPIO_PXPES(0), "Read write only register in GPIO. offset %x\n", offset);
        ASSERT(offset != GPIO_PXPES(0), "Read write only register in GPIO. offset %x\n", offset);
        ASSERT(offset != GPIO_PXPES(0), "Read write only register in GPIO. offset %x\n", offset);

        ASSERT(offset != GPIO_PXPEC(0), "Read write only register in GPIO. offset %x\n", offset);
        ASSERT(offset != GPIO_PXPEC(0), "Read write only register in GPIO. offset %x\n", offset);
        ASSERT(offset != GPIO_PXPEC(0), "Read write only register in GPIO. offset %x\n", offset);
        ASSERT(offset != GPIO_PXPEC(0), "Read write only register in GPIO. offset %x\n", offset);

        ASSERT(offset != GPIO_PXFUNS(0), "Read write only register in GPIO. offset %x\n", offset);
        ASSERT(offset != GPIO_PXFUNS(0), "Read write only register in GPIO. offset %x\n", offset);
        ASSERT(offset != GPIO_PXFUNS(0), "Read write only register in GPIO. offset %x\n", offset);
        ASSERT(offset != GPIO_PXFUNS(0), "Read write only register in GPIO. offset %x\n", offset);

        ASSERT(offset != GPIO_PXFUNC(0), "Read write only register in GPIO. offset %x\n", offset);
        ASSERT(offset != GPIO_PXFUNC(0), "Read write only register in GPIO. offset %x\n", offset);
        ASSERT(offset != GPIO_PXFUNC(0), "Read write only register in GPIO. offset %x\n", offset);
        ASSERT(offset != GPIO_PXFUNC(0), "Read write only register in GPIO. offset %x\n", offset);

        ASSERT(offset != GPIO_PXDIRS(0), "Read write only register in GPIO. offset %x\n", offset);
        ASSERT(offset != GPIO_PXDIRS(0), "Read write only register in GPIO. offset %x\n", offset);
        ASSERT(offset != GPIO_PXDIRS(0), "Read write only register in GPIO. offset %x\n", offset);
        ASSERT(offset != GPIO_PXDIRS(0), "Read write only register in GPIO. offset %x\n", offset);

        ASSERT(offset != GPIO_PXDIRC(0), "Read write only register in GPIO. offset %x\n", offset);
        ASSERT(offset != GPIO_PXDIRC(0), "Read write only register in GPIO. offset %x\n", offset);
        ASSERT(offset != GPIO_PXDIRC(0), "Read write only register in GPIO. offset %x\n", offset);
        ASSERT(offset != GPIO_PXDIRC(0), "Read write only register in GPIO. offset %x\n", offset);

        ASSERT(offset != GPIO_PXTRGS(0), "Read write only register in GPIO. offset %x\n", offset);
        ASSERT(offset != GPIO_PXTRGS(0), "Read write only register in GPIO. offset %x\n", offset);
        ASSERT(offset != GPIO_PXTRGS(0), "Read write only register in GPIO. offset %x\n", offset);
        ASSERT(offset != GPIO_PXTRGS(0), "Read write only register in GPIO. offset %x\n", offset);

        ASSERT(offset != GPIO_PXTRGC(0), "Read write only register in GPIO. offset %x\n", offset);
        ASSERT(offset != GPIO_PXTRGC(0), "Read write only register in GPIO. offset %x\n", offset);
        ASSERT(offset != GPIO_PXTRGC(0), "Read write only register in GPIO. offset %x\n", offset);
        ASSERT(offset != GPIO_PXTRGC(0), "Read write only register in GPIO. offset %x\n", offset);

        ASSERT(offset != GPIO_PXFLGC(0), "Read write only register in GPIO. offset %x\n", offset);
        ASSERT(offset != GPIO_PXFLGC(0), "Read write only register in GPIO. offset %x\n", offset);
        ASSERT(offset != GPIO_PXFLGC(0), "Read write only register in GPIO. offset %x\n", offset);
        ASSERT(offset != GPIO_PXFLGC(0), "Read write only register in GPIO. offset %x\n", offset);
    }
#endif


    if (op_type == MTS_READ) {
#ifdef SIM_PAVO
        /*PAVO GPIO(C) PIN 30  -> NAND FLASH R/B. */
        if (offset == GPIO_PXPIN(2)) {
            /*FOR NAND FLASH.PIN 30 ----|_____|------ */
            temp = jz4740_gpio_table[GPIO_PXPIN(2) / 4];
            temp &= 0x40000000;
            if (temp)
                temp &= ~0x40000000;
            else
                temp |= 0x40000000;
            jz4740_gpio_table[GPIO_PXPIN(2) / 4] = temp;
        }
#endif
        return ((void *) (d->jz4740_gpio_ptr + offset));
    } else if (op_type == MTS_WRITE) {
        switch (op_size) {
            case 1:
                mask = 0xff;
                break;
            case 2:
                mask = 0xffff;
                break;
            case 4:
                mask = 0xffffffff;
                break;
            default:
                assert(0);
        }

        switch (offset) {
            case GPIO_PXDATS(0):
            case GPIO_PXDATS(1):
            case GPIO_PXDATS(2):
            case GPIO_PXDATS(3):
                group = offset / 0x100;
                mask_data = (*data) & mask;
                jz4740_gpio_table[GPIO_PXDAT(group) / 4] |= mask_data;
                break;

            case GPIO_PXDATC(0):
            case GPIO_PXDATC(1):
            case GPIO_PXDATC(2):
            case GPIO_PXDATC(3):
                group = offset / 0x100;
                mask_data = (*data) & mask;
                mask_data = ~mask_data;
                jz4740_gpio_table[GPIO_PXDAT(group) / 4] &= mask_data;
                break;

            case GPIO_PXIMS(0):
            case GPIO_PXIMS(1):
            case GPIO_PXIMS(2):
            case GPIO_PXIMS(3):
                group = offset / 0x100;
                mask_data = (*data) & mask;
                jz4740_gpio_table[GPIO_PXIM(group) / 4] |= mask_data;
                break;

            case GPIO_PXIMC(0):
            case GPIO_PXIMC(1):
            case GPIO_PXIMC(2):
            case GPIO_PXIMC(3):
                group = offset / 0x100;
                mask_data = (*data) & mask;
                mask_data = ~mask_data;
                jz4740_gpio_table[GPIO_PXIM(group) / 4] &= mask_data;
                break;

            case GPIO_PXPES(0):
            case GPIO_PXPES(1):
            case GPIO_PXPES(2):
            case GPIO_PXPES(3):
                group = offset / 0x100;
                mask_data = (*data) & mask;
                jz4740_gpio_table[GPIO_PXPE(group) / 4] |= mask_data;
                break;

            case GPIO_PXPEC(0):
            case GPIO_PXPEC(1):
            case GPIO_PXPEC(2):
            case GPIO_PXPEC(3):
                group = offset / 0x100;
                mask_data = (*data) & mask;
                mask_data = ~mask_data;
                jz4740_gpio_table[GPIO_PXPE(group) / 4] &= mask_data;
                break;


            case GPIO_PXFUNS(0):
            case GPIO_PXFUNS(1):
            case GPIO_PXFUNS(2):
            case GPIO_PXFUNS(3):
                group = offset / 0x100;
                mask_data = (*data) & mask;
                jz4740_gpio_table[GPIO_PXFUN(group) / 4] |= mask_data;
                break;

            case GPIO_PXFUNC(0):
            case GPIO_PXFUNC(1):
            case GPIO_PXFUNC(2):
            case GPIO_PXFUNC(3):
                group = offset / 0x100;
                mask_data = (*data) & mask;
                mask_data = ~mask_data;
                jz4740_gpio_table[GPIO_PXFUN(group) / 4] &= mask_data;
                break;


            case GPIO_PXSELS(0):
            case GPIO_PXSELS(1):
            case GPIO_PXSELS(2):
            case GPIO_PXSELS(3):
                group = offset / 0x100;
                mask_data = (*data) & mask;
                jz4740_gpio_table[GPIO_PXSEL(group) / 4] |= mask_data;
                break;

            case GPIO_PXSELC(0):
            case GPIO_PXSELC(1):
            case GPIO_PXSELC(2):
            case GPIO_PXSELC(3):
                group = offset / 0x100;
                mask_data = (*data) & mask;
                mask_data = ~mask_data;
                jz4740_gpio_table[GPIO_PXSEL(group) / 4] &= mask_data;
                break;


            case GPIO_PXDIRS(0):
            case GPIO_PXDIRS(1):
            case GPIO_PXDIRS(2):
            case GPIO_PXDIRS(3):
                group = offset / 0x100;
                mask_data = (*data) & mask;
                jz4740_gpio_table[GPIO_PXDIR(group) / 4] |= mask_data;
                break;

            case GPIO_PXDIRC(0):
            case GPIO_PXDIRC(1):
            case GPIO_PXDIRC(2):
            case GPIO_PXDIRC(3):
                group = offset / 0x100;
                mask_data = (*data) & mask;
                mask_data = ~mask_data;
                jz4740_gpio_table[GPIO_PXDIR(group) / 4] &= mask_data;
                break;

            case GPIO_PXTRGS(0):
            case GPIO_PXTRGS(1):
            case GPIO_PXTRGS(2):
            case GPIO_PXTRGS(3):
                group = offset / 0x100;
                mask_data = (*data) & mask;
                jz4740_gpio_table[GPIO_PXTRG(group) / 4] |= mask_data;
                break;

            case GPIO_PXTRGC(0):
            case GPIO_PXTRGC(1):
            case GPIO_PXTRGC(2):
            case GPIO_PXTRGC(3):
                group = offset / 0x100;
                mask_data = (*data) & mask;
                mask_data = ~mask_data;
                jz4740_gpio_table[GPIO_PXTRG(group) / 4] &= mask_data;
                break;


            case GPIO_PXFLGC(0):
            case GPIO_PXFLGC(1):
            case GPIO_PXFLGC(2):
            case GPIO_PXFLGC(3):
                group = offset / 0x100;
                mask_data = (*data) & mask;
                mask_data = ~mask_data;
                jz4740_gpio_table[GPIO_PXFLG(group) / 4] &= mask_data;
                break;


            default:
                printf("invalid offset in GPIO. offset %x\n", offset);
                return NULL;
        }
        *has_set_value = TRUE;
        return NULL;
    }

    return NULL;
}

/**
 * mod_vm_device_GPIO_command
 * @brief   
 * @param   p_m
 * @param   arg
 * @return  STD_CALL             std_void_t
 */
STD_CALL std_void_t mod_vm_device_GPIO_command(IN mod_vm_device_t *p_m, IN std_void_t *arg)
{
    return;
}

struct mod_vm_device_ops_st mod_vm_device_GPIO_ops = {
        mod_vm_device_GPIO_init,
        mod_vm_device_GPIO_cleanup,

        /***func_ops***/
        mod_vm_device_GPIO_initiate,
        mod_vm_device_GPIO_reset,
        mod_vm_device_GPIO_access,
        mod_vm_device_GPIO_command,

};

/**
 * mod_vm_device_GPIO_create_instance
 * @brief   
 * @param   pp_handle
 * @return  STD_CALL std_rv_t
 */
STD_CALL std_rv_t mod_vm_device_GPIO_create_instance(INOUT std_void_t **pp_handle)
{
    mod_vm_device_imp_t *p_m = NULL;

    p_m = (mod_vm_device_imp_t *) CALLOC(1, sizeof(mod_vm_device_imp_t));
    p_m->unique_id = std_random_u64();
    p_m->p_ops = &mod_vm_device_GPIO_ops;

    mod_ownership_register_ops((mod_ownership_t *) p_m);
    mod_ownership_init((mod_ownership_t *) p_m);
    *pp_handle = p_m;

    return STD_RV_SUC;
}
