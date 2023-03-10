/*
 * Cisco router simulation platform.
 * Copyright (c) 2005,2006 Christophe Fillot (cf@utc.fr)
 *
 * Virtual console TTY.
 */

 /*
  * Copyright (C) yajin 2008 <yajinzhou@gmail.com >
  *     
  * This file is part of the virtualmips distribution. 
  * See LICENSE file for terms of the license. 
  *
  */


#ifndef __DEV_VTTY_H__
#define __DEV_VTTY_H__

#include "std_common.h"
#include "mod_vm_device_vtty.h"
#include <pthread.h>



/* 4 Kb should be enough for a keyboard buffer */
#define VTTY_BUFFER_SIZE  4096

/* VTTY connection types */
enum
{
   VTTY_TYPE_NONE = 0,
   VTTY_TYPE_TERM,
   VTTY_TYPE_TCP,
   VTTY_TYPE_SERIAL,
};

/* VTTY connection states (for TCP) */
enum
{
   VTTY_STATE_TCP_INVALID,      /* connection is not working */
   VTTY_STATE_TCP_WAITING,      /* waiting for incoming connection */
   VTTY_STATE_TCP_RUNNING,      /* character reading/writing ok */
};

/* VTTY input states */
enum
{
   VTTY_INPUT_TEXT,
   VTTY_INPUT_VT1,
   VTTY_INPUT_VT2,
   VTTY_INPUT_REMOTE,
   VTTY_INPUT_TELNET,
   VTTY_INPUT_TELNET_IYOU,
   VTTY_INPUT_TELNET_SB1,
   VTTY_INPUT_TELNET_SB2,
   VTTY_INPUT_TELNET_SB_TTYPE,
   VTTY_INPUT_TELNET_NEXT
};



/* Commmand line support utility */


/**
 * vtty_parse_serial_option
 * @brief   
 * @param   params
 * @param   optarg
 * @return  int
 */
int vtty_parse_serial_option(vtty_serial_option_t * params, char *optarg);

/* Virtual TTY structure */
typedef struct virtual_tty
{
	void *vm;
   char *name;
   int type, state;
   int tcp_port;
   int terminal_support;
   int input_state;
   int input_pending;
   int telnet_cmd, telnet_opt, telnet_qual;
   int fd, accept_fd, *select_fd;
   int managed_flush;
   FILE *fstream;
   u_char buffer[VTTY_BUFFER_SIZE];
   u_int read_ptr, write_ptr;
   pthread_mutex_t lock;
   struct virtual_tty *next, **pprev;
   void *priv_data;

   /* Read notification */
   void (*read_notifier) (struct virtual_tty *);
}vtty_t;

#define VTTY_LOCK(tty) pthread_mutex_lock(&(tty)->lock);
#define VTTY_UNLOCK(tty) pthread_mutex_unlock(&(tty)->lock);

/* create a virtual tty */
vtty_t *vtty_create(void * vm, char *name, int type, int tcp_port, const vtty_serial_option_t * option);

/* delete a virtual tty */
void vtty_delete(vtty_t * vtty);

/* Store a string in the FIFO buffer */
int vtty_store_str(vtty_t * vtty, char *str);

/* read a character from the buffer (-1 if the buffer is empty) */
int vtty_get_char(vtty_t * vtty);

/* print a character to vtty */
void vtty_put_char(vtty_t * vtty, char ch);

/* Put a buffer to vtty */
void vtty_put_buffer(vtty_t * vtty, char *buf, size_t len);

/* Flush VTTY output */
void vtty_flush(vtty_t * vtty);

/* returns TRUE if a character is available in buffer */
int vtty_is_char_avail(vtty_t * vtty);

/* write CTRL+C to buffer */
int vtty_store_ctrlc(vtty_t *);

/* Initialize the VTTY thread */
int vtty_init(void);

/**
 * vtty_is_full
 * @brief   
 * @param   vtty
 * @return  int
 */
int vtty_is_full(vtty_t * vtty);

/**
 * vtty_bytes
 * @brief   
 * @param   vtty
 * @return  int
 */
int vtty_bytes(vtty_t * vtty);
#endif
