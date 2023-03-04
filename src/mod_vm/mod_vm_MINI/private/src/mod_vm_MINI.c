/**
 * Copyright (c) 2021 Yunhai Zhu <yunhaia2@gmail.com>
 *
 * see COPYRIGHT file.  
 */

/**
 * @file    mod_vm_MINI.c
 * @brief   implement functions
 * @version 1.0
 * @author  Yunhai Zhu
 * @date    2022-03-01
 *
 */

#include "mod_vm_MINI.h"
#include "mod_vm_arch.h"
#include "mod_vm_arch_mips_cp0.h"
#include "mod_vm_arch_mips_cpu.h"
#include "mod_vm_device_manager.h"
#include "mod_vm_memory.h"
#include <libelf.h>

static mod_vm_arch_t *p_global_vm_arch = NULL;
static mod_vm_arch_mips_cpu_t *p_global_vm_arch_mips_cpu = NULL;
static mod_vm_arch_mips_cp0_t *p_global_vm_arch_mips_cp0 = NULL;
static mod_vm_device_manager_t *p_global_dev_manger = NULL;
static mod_vm_memory_t *p_global_vm_memory = NULL;


/**
 * mod_vm_MINI_init
 * @brief   
 * @param   p_m
 * @param   arg
 * @param   arg_len
 * @return  STD_CALL std_rv_t
 */
STD_CALL std_rv_t mod_vm_MINI_init(IN mod_vm_t * p_m, IN const std_char_t * arg, IN std_int_t arg_len)
{
	mod_vm_imp_t *p_imp_m = (mod_vm_imp_t *) p_m;
	mod_iid_t mod_shell_iid = MOD_SHELL_IID;
	std_char_t key[BUF_SIZE_128] = "\0";

	mod_create_instance(&mod_shell_iid, (std_void_t **) & p_imp_m->p_mod_shell, (mod_ownership_t *) p_imp_m);

	mod_shell_init(p_imp_m->p_mod_shell, NULL, 0);

    /****shell_register******/

	snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_initiate");
	mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)),
			   shell_stub_mod_vm_MINI_initiate, p_imp_m);

	snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_start");
	mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)), shell_stub_mod_vm_MINI_start,
			   p_imp_m);

	snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_stop");
	mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)), shell_stub_mod_vm_MINI_stop,
			   p_imp_m);

	snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_suspend");
	mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)), shell_stub_mod_vm_MINI_suspend,
			   p_imp_m);

	snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_resume");
	mod_shell_register(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)), shell_stub_mod_vm_MINI_resume,
			   p_imp_m);

	return STD_RV_SUC;
}

/**
 * mod_vm_MINI_cleanup
 * @brief   
 * @param   p_m
 * @return  STD_CALL std_rv_t
 */
STD_CALL std_rv_t mod_vm_MINI_cleanup(mod_vm_t * p_m)
{
	mod_vm_imp_t *p_imp_m = (mod_vm_imp_t *) p_m;
	mod_iid_t mod_shell_iid = MOD_SHELL_IID;
	std_char_t key[BUF_SIZE_128] = "\0";

    /****shell_unregister******/

	snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_initiate");
	mod_shell_unregister(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)));

	snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_start");
	mod_shell_unregister(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)));

	snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_stop");
	mod_shell_unregister(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)));

	snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_suspend");
	mod_shell_unregister(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)));

	snprintf(key, sizeof(key), "%lul-%s", p_imp_m->unique_id, "mod_vm_resume");
	mod_shell_unregister(p_imp_m->p_mod_shell, key, std_safe_strlen(key, sizeof(key)));

	mod_shell_cleanup(p_imp_m->p_mod_shell);

	mod_delete_instance(&mod_shell_iid, (std_void_t **) & p_imp_m->p_mod_shell, (mod_ownership_t *) p_imp_m);

	return STD_RV_SUC;
}

/***func_implementation***/
int device_init(char *name, std_u32_t addr, std_u32_t len, mod_vm_device_t *p_dev)
{
    vm_device_info_t init;

    memset(&init, 0, sizeof(init));

    sprintf(init.name, "%s", name);
    init.phys_addr = addr;
    init.phys_len = len;

    STD_LOG(DISPLAY, "0x%x addr begin 0x%x addr end\n", addr, addr + len);
    mod_vm_device_initiate(p_dev, &init);
    mod_vm_device_manager_add(p_global_dev_manger, p_dev, init.phys_addr, init.phys_len);
    return 0;
}


/* Load an ELF image into the simulated memory. Using libelf*/
int cvm_load_elf_image(char *filename, std_u32_t *entry_point)
{
    std_u32_t vaddr;
    std_u32_t remain;
    void *haddr;
    Elf32_Ehdr *ehdr;
    Elf32_Shdr *shdr;
    Elf_Scn *scn;
    Elf *img_elf;
    size_t len, clen;
    char *name;
    int i, fd;
    FILE *bfd;

    if (!filename)
        return (-1);

    fd = open(filename, O_RDONLY);

    printf("Loading ELF file '%s'...\n", filename);
    if (fd == -1) {
        perror("load_elf_image: open");
        return (-1);
    }

    if (elf_version(EV_CURRENT) == EV_NONE) {
        fprintf(stderr, "load_elf_image: library out of date\n");
        return (-1);
    }

    if (!(img_elf = elf_begin(fd, ELF_C_READ, NULL))) {
        fprintf(stderr, "load_elf_image: elf_begin: %s\n", elf_errmsg(elf_errno()));
        return (-1);
    }

    if (!(ehdr = elf32_getehdr(img_elf))) {
        fprintf(stderr, "load_elf_image: invalid ELF file\n");
        return (-1);
    }

    bfd = fdopen(fd, "rb");

    if (!bfd) {
        perror("load_elf_image: fdopen");
        return (-1);
    }

    for (i = 0; i < ehdr->e_shnum; i++) {
        scn = elf_getscn(img_elf, i);

        shdr = elf32_getshdr(scn);
        name = elf_strptr(img_elf, ehdr->e_shstrndx, (size_t) shdr->sh_name);
        len = shdr->sh_size;

        if (!(shdr->sh_flags & SHF_ALLOC) || !len)
            continue;

        fseek(bfd, shdr->sh_offset, SEEK_SET);
        vaddr = sign_extend(shdr->sh_addr, 32);

        {
            STD_LOG(DISPLAY, " %s  * Adding section at virtual address 0x%8.8"
                             "x "
                             "(len=0x%8.8zx)\n",
                    name, vaddr & 0xFFFFFFFF, len);
        }

        while (len > 0) {
            haddr = mod_vm_memory_lookup(p_global_vm_memory, vaddr);

            if (!haddr) {
                fprintf(stderr, "load_elf_image: invalid load address 0x%"
                                "x\n",
                        vaddr);
                return (-1);
            }

            if (len > MIPS_MIN_PAGE_SIZE)
                clen = MIPS_MIN_PAGE_SIZE;
            else
                clen = len;

            remain = MIPS_MIN_PAGE_SIZE;
            remain -= (vaddr - (vaddr & MIPS_MIN_PAGE_SIZE));

            clen = MIN(clen, remain);

            if (fread((u_char *) haddr, clen, 1, bfd) < 1)
                break;

            vaddr += clen;
            len -= clen;
        }
    }

    printf("ELF entry point: 0x%x\n", ehdr->e_entry);

    if (entry_point)
        *entry_point = ehdr->e_entry;

    elf_end(img_elf);
    fclose(bfd);
    return (0);
}

STD_CALL std_int_t vm_main_initiate(IN mod_vm_t *p_owner_m)
{
    mod_vm_device_t *p_ram = NULL;

    std_u32_t entry_point;

    mod_iid_t mod_vm_arch_iid = MOD_VM_ARCH_IID;
    mod_iid_t mod_vm_arch_mips_cpu_iid = MOD_VM_ARCH_MIPS_CPU_IID;
    mod_iid_t mod_vm_arch_mips_cp0_iid = MOD_VM_ARCH_MIPS_CP0_IID;
    mod_iid_t mod_vm_memory_iid = MOD_VM_MEMORY_IID;
    mod_iid_t mod_vm_device_manager_iid = MOD_VM_DEVICE_MANAGER_IID;
    mod_iid_t mod_vm_device_RAM_iid = MOD_VM_DEVICE_RAM_IID;

    mod_create_instance(&mod_vm_arch_iid, (void **) &p_global_vm_arch, (mod_ownership_t *) p_owner_m);
    mod_create_instance(&mod_vm_arch_mips_cpu_iid, (void **) &p_global_vm_arch_mips_cpu, (mod_ownership_t *) p_owner_m);
    mod_create_instance(&mod_vm_arch_mips_cp0_iid, (void **) &p_global_vm_arch_mips_cp0, (mod_ownership_t *) p_owner_m);
    mod_create_instance(&mod_vm_memory_iid, (void **) &p_global_vm_memory, (mod_ownership_t *) p_owner_m);
    mod_create_instance(&mod_vm_device_manager_iid, (void **) &p_global_dev_manger, (mod_ownership_t *) p_owner_m);
    mod_create_instance(&mod_vm_device_RAM_iid, (void **) &p_ram, (mod_ownership_t *) p_owner_m);

    mod_vm_device_manager_init(p_global_dev_manger, NULL, 0);

    device_init("RAM", 0x00000000ULL, 32 * 1048576, p_ram);

    mod_vm_memory_initiate(p_global_vm_memory);

    mod_vm_arch_mips_cp0_init(p_global_vm_arch_mips_cp0, NULL, 0);
    mod_vm_arch_mips_cp0_reset(p_global_vm_arch_mips_cp0);

    mod_vm_arch_mips_cpu_init(p_global_vm_arch_mips_cpu, NULL, 0);

    cvm_load_elf_image("profile_mips2", &entry_point);
    entry_point = 0x740;

    mod_vm_arch_initiate(p_global_vm_arch, entry_point);

    return 0;
}
/**
 * mod_vm_MINI_initiate
 * @brief   
 * @param   p_m
 * @param   conf_name
 * @return  STD_CALL             std_void_t
 */
STD_CALL std_void_t mod_vm_MINI_initiate(IN mod_vm_t * p_m, IN std_char_t * conf_name)
{
    vm_main_initiate(p_m);
}

/**
 * mod_vm_MINI_start
 * @brief   
 * @param   p_m
 * @return  STD_CALL             std_void_t
 */
STD_CALL std_void_t mod_vm_MINI_start(IN mod_vm_t * p_m)
{
    mod_vm_imp_t *p_vm_info = (mod_vm_imp_t *) p_m;
    vm_system_info_t *p_vm_sys = NULL;

    p_vm_sys = &(p_vm_info->vm_sys);
    p_vm_sys->vm_status = VM_STATUS_RUNNING;

    mod_vm_arch_start(p_global_vm_arch);
}

/**
 * mod_vm_MINI_stop
 * @brief   
 * @param   p_m
 * @return  STD_CALL             std_void_t
 */
STD_CALL std_void_t mod_vm_MINI_stop(IN mod_vm_t * p_m)
{
    mod_vm_imp_t *p_vm_info = (mod_vm_imp_t *) p_m;
    vm_system_info_t *p_vm_sys = NULL;

    p_vm_sys = &(p_vm_info->vm_sys);
    p_vm_sys->vm_status = VM_STATUS_SHUTDOWN;

    mod_vm_arch_stop(p_global_vm_arch);
}

/**
 * mod_vm_MINI_suspend
 * @brief   
 * @param   p_m
 * @return  STD_CALL             std_void_t
 */
STD_CALL std_void_t mod_vm_MINI_suspend(IN mod_vm_t * p_m)
{
    mod_vm_imp_t *p_vm_info = (mod_vm_imp_t *) p_m;
    vm_system_info_t *p_vm_sys = NULL;

    p_vm_sys = &(p_vm_info->vm_sys);
    p_vm_sys->vm_status = VM_STATUS_SUSPENDED;

    mod_vm_arch_suspend(p_global_vm_arch);
}

/**
 * mod_vm_MINI_resume
 * @brief   
 * @param   p_m
 * @return  STD_CALL             std_void_t
 */
STD_CALL std_void_t mod_vm_MINI_resume(IN mod_vm_t * p_m)
{
    mod_vm_imp_t *p_vm_info = (mod_vm_imp_t *) p_m;
    vm_system_info_t *p_vm_sys = NULL;

    p_vm_sys = &(p_vm_info->vm_sys);
    p_vm_sys->vm_status = VM_STATUS_RUNNING;

    mod_vm_arch_resume(p_global_vm_arch);
}

struct mod_vm_ops_st mod_vm_MINI_ops = {
	mod_vm_MINI_init,
	mod_vm_MINI_cleanup,

    /***func_ops***/
	mod_vm_MINI_initiate,
	mod_vm_MINI_start,
	mod_vm_MINI_stop,
	mod_vm_MINI_suspend,
	mod_vm_MINI_resume,

};

/**
 * mod_vm_MINI_create_instance
 * @brief   
 * @param   pp_handle
 * @return  STD_CALL std_rv_t
 */
STD_CALL std_rv_t mod_vm_MINI_create_instance(INOUT std_void_t ** pp_handle)
{
	mod_vm_imp_t *p_m = NULL;

	p_m = (mod_vm_imp_t *) CALLOC(1, sizeof(mod_vm_imp_t));
	p_m->unique_id = std_random_u64();
	p_m->p_ops = &mod_vm_MINI_ops;

	mod_ownership_register_ops((mod_ownership_t *) p_m);
	mod_ownership_init((mod_ownership_t *) p_m);
	*pp_handle = p_m;

	return STD_RV_SUC;
}
