/**
 * Copyright (c) 2021 Yunhai Zhu <yunhaia2@gmail.com>
 *
 * see COPYRIGHT file.  
 */
     
/**
 * @file    dev_netio.h
 * @brief   define structure & functions
 * @version 1.0
 * @author  Yunhai Zhu
 * @date    2022-02-14
 *
 */
#ifndef DEV_NETIO_H
#define DEV_NETIO_H

/**
 * netio_tap_free
 * @brief   
 * @param   fd
 */
void netio_tap_free(int fd);

/**
 * netio_tap_open
 * @brief   
 * @return  int
 */
int netio_tap_open();

/**
 * netio_tap_send
 * @brief   
 * @param   fd
 * @param   pkt
 * @param   pkt_len
 * @return  ssize_t
 */
ssize_t netio_tap_send(int fd, void *pkt, size_t pkt_len);

/**
 * netio_tap_recv
 * @brief   
 * @param   fd
 * @param   pkt
 * @param   max_len
 * @return  ssize_t
 */
ssize_t netio_tap_recv(int fd, void *pkt, size_t max_len);

#endif
