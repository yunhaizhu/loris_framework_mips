/**
 * Copyright (c) 2021 Yunhai Zhu <yunhaia2@gmail.com>
 *
 * see COPYRIGHT file.  
 */
     
/**
 * @file    dev_netio.c
 * @brief   implement functions
 * @version 1.0
 * @author  Yunhai Zhu
 * @date    2022-02-14
 *
 */

#include <net/if.h>
#include <linux/if_tun.h>

#include <sys/ioctl.h>
#include <sys/socket.h>


#include "std_common.h"

/* Free a NetIO TAP descriptor */
void netio_tap_free(int fd)
{
	close(fd);
}

/* Open a TAP device */
int netio_tap_open()
{
   static struct ifreq ifr;
   int fd, err;

   if ((fd = open("/dev/net/tun", O_RDWR)) < 0)
      return (-1);

   memset(&ifr, 0, sizeof(ifr));

   /* Flags: IFF_TUN   - TUN device (no Ethernet headers)
    *        IFF_TAP   - TAP device
    *
    *        IFF_NO_PI - Do not provide packet information
    */
   ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
   
   strncpy(ifr.ifr_name, "tap0", IFNAMSIZ);

   if ((err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0)
   {
     printf("Set file  failed\n");
     close(fd);
     return err;
   }
   
   
   /*SET NO BLOCKING */
   if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
	   printf("Set file descriptor to non-blocking mode failed\n");
   
   printf("TAP CREATED %d\n", fd);
   return (fd);
}

/* Write a packet to a TAP device */
ssize_t netio_tap_send(int fd, void *pkt, size_t pkt_len)
{
	return (write(fd, pkt, pkt_len));
}

/* Receive a packet through a TAP device */
ssize_t netio_tap_recv(int fd, void *pkt, size_t max_len)
{
   return (read(fd, pkt, max_len));
}
