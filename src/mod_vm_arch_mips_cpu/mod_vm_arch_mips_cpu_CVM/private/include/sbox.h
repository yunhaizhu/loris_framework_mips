/*
 * Cisco router simulation platform.
 * Copyright (c) 2007 Christophe Fillot (cf@utc.fr)
 *
 * S-box functions.
 */

#ifndef __SBOX_H__
#define __SBOX_H__

#include "std_common.h"
#include <sys/types.h>

extern std_u32_t sbox_array[];

/**
 * sbox_compute
 * @brief   
 * @param   data
 * @param   len
 * @return  static inline std_u32_t
 */
static inline std_u32_t sbox_compute(std_u8_t *data, int len)
{
    std_u32_t hash = 0;

    while (len > 0) {
        hash ^= sbox_array[*data];
        hash *= 3;
        data++;
    }

    return (hash);
}

/**
 * sbox_u32
 * @brief   
 * @param   val
 * @return  static forced_inline std_u32_t
 */
static forced_inline std_u32_t sbox_u32(std_u32_t val)
{
    std_u32_t hash = 0;

    hash ^= sbox_array[(std_u8_t) val];
    hash *= 3;
    val >>= 8;

    hash ^= sbox_array[(std_u8_t) val];
    hash *= 3;
    val >>= 8;

    hash ^= sbox_array[(std_u8_t) val];
    hash *= 3;
    val >>= 8;

    hash ^= sbox_array[(std_u8_t) val];
    return (hash);
}

#endif
