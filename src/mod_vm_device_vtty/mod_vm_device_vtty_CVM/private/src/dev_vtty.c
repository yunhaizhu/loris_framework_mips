/*
 *
 * Virtual console TTY.
 *
 * "Interactive" part idea by Mtve.
 * TCP console added by Mtve.
 * Serial console by Peter Ross (suxen_drol@hotmail.com)
 *
 * */


#include "dev_vtty.h"
#include <arpa/telnet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>


/* VTTY list */
static pthread_mutex_t vtty_list_mutex = PTHREAD_MUTEX_INITIALIZER;
static vtty_t *vtty_list = NULL;
static pthread_t vtty_thread;

#define VTTY_LIST_LOCK() pthread_mutex_lock(&vtty_list_mutex)
#define VTTY_LIST_UNLOCK() pthread_mutex_unlock(&vtty_list_mutex)

static struct termios tios, tios_orig;


/* Restore TTY original settings */
static void vtty_term_reset(void)
{
    tcsetattr(STDIN_FILENO, TCSANOW, &tios_orig);
}

/* Initialize real TTY */
static void vtty_term_init(void)
{
    tcgetattr(STDIN_FILENO, &tios);

    memcpy(&tios_orig, &tios, sizeof(struct termios));
    atexit(vtty_term_reset);

    tios.c_cc[VTIME] = 0;
    tios.c_cc[VMIN] = 1;

    /* Disable Ctrl-C, Ctrl-S, Ctrl-Q and Ctrl-Z */
    tios.c_cc[VINTR] = 0;
    tios.c_cc[VSTART] = 0;
    tios.c_cc[VSTOP] = 0;
    tios.c_cc[VSUSP] = 0;

    tios.c_lflag &= ~(ICANON | ECHO);
    tios.c_iflag &= ~ICRNL;
    tcsetattr(STDIN_FILENO, TCSANOW, &tios);
    tcflush(STDIN_FILENO, TCIFLUSH);
}


/* Create a virtual tty */
vtty_t *vtty_create(void *vm, char *name, int type, int tcp_port, const vtty_serial_option_t *option)
{
    vtty_t *vtty;

    if (!(vtty = malloc(sizeof(*vtty)))) {
        fprintf(stderr, "VTTY: unable to create new virtual tty.\n");
        return NULL;
    }

    memset(vtty, 0, sizeof(*vtty));
    vtty->name = name;
    vtty->type = type;
    vtty->vm = vm;
    vtty->fd = -1;
    vtty->fstream = NULL;
    vtty->accept_fd = -1;
    pthread_mutex_init(&vtty->lock, NULL);
    vtty->terminal_support = 1;
    vtty->input_state = VTTY_INPUT_TEXT;
    vtty->managed_flush = 0;

    switch (vtty->type) {
        case VTTY_TYPE_NONE:
            vtty->select_fd = NULL;
            break;

        case VTTY_TYPE_TERM:
            vtty_term_init();
            vtty->fd = STDIN_FILENO;
            vtty->select_fd = &vtty->fd;
            vtty->fstream = stdout;
            break;


        default:
            fprintf(stderr, "tty_create: bad vtty type %d\n", vtty->type);
            return NULL;
    }

    /* Add this new VTTY to the list */
    VTTY_LIST_LOCK();
    vtty->next = vtty_list;
    vtty->pprev = &vtty_list;

    if (vtty_list != NULL)
        vtty_list->pprev = &vtty->next;

    vtty_list = vtty;
    VTTY_LIST_UNLOCK();
    return vtty;
}

/* Delete a virtual tty */
void vtty_delete(vtty_t *vtty)
{
    if (vtty != NULL) {
        if (vtty->pprev != NULL) {
            VTTY_LIST_LOCK();
            if (vtty->next)
                vtty->next->pprev = vtty->pprev;
            *(vtty->pprev) = vtty->next;
            VTTY_LIST_UNLOCK();
        }

        if ((vtty->fstream) && (vtty->fstream != stdout))
            fclose(vtty->fstream);

        /* We don't close FD 0 since it is stdin */
        if (vtty->fd > 0) {
            printf("VTTY %s: closing FD %d\n", vtty->name, vtty->fd);
            close(vtty->fd);
        }

        if (vtty->accept_fd != -1) {
            printf("VTTY %s: closing accept FD %d\n", vtty->name, vtty->accept_fd);
            close(vtty->accept_fd);
        }

        free(vtty);
    }
}

/* Store a character in the FIFO buffer */
static int vtty_store(vtty_t *vtty, u_char c)
{
    u_int nwptr;

    VTTY_LOCK(vtty);
    nwptr = vtty->write_ptr + 1;
    if (nwptr == VTTY_BUFFER_SIZE)
        nwptr = 0;

    if (nwptr == vtty->read_ptr) {
        VTTY_UNLOCK(vtty);
        return -1;
    }

    vtty->buffer[vtty->write_ptr] = c;
    vtty->write_ptr = nwptr;
    VTTY_UNLOCK(vtty);
    return 0;
}

/* Store a string in the FIFO buffer */
int vtty_store_str(vtty_t *vtty, char *str)
{
    if (!vtty)
        return 0;

    while (*str != 0) {
        if (vtty_store(vtty, *str) == -1)
            return -1;

        str++;
    }

    vtty->input_pending = TRUE;
    return 0;
}

/* Store CTRL+C in buffer */
int vtty_store_ctrlc(vtty_t *vtty)
{
    if (vtty)
        vtty_store(vtty, 0x03);
    return 0;
}

/* 
 * Read a character from the terminal.
 */
static int vtty_term_read(vtty_t *vtty)
{
    u_char c;

    if (read(vtty->fd, &c, 1) == 1) {
        return c;
    }
    perror("read from vtty failed");
    return -1;
}


/*
 * Read a character from the virtual TTY.
 *
 * If the VTTY is a TCP connection, restart it in case of error.
 */
static int vtty_read(vtty_t *vtty)
{
    switch (vtty->type) {
        case VTTY_TYPE_TERM:
        case VTTY_TYPE_SERIAL:
            return (vtty_term_read(vtty));
        default:
            fprintf(stderr, "vtty_read: bad vtty type %d\n", vtty->type);
            return -1;
    }

    /* NOTREACHED */
    return -1;
}


/* Read a character (until one is available) and store it in buffer */
static void vtty_read_and_store(vtty_t *vtty)
{
    int c;

    /* wait until we get a character input */
    c = vtty_read(vtty);

    /* if read error, do nothing */
    if (c < 0)
        return;

    if (!vtty->terminal_support) {
        vtty_store(vtty, (u_char)c);
        return;
    }

    switch (vtty->input_state) {
        case VTTY_INPUT_TEXT:
            switch (c) {
                case 0x1b:
                    vtty->input_state = VTTY_INPUT_VT1;
                    return;

                    /* Ctrl + ']' (0x1d, 29), or Alt-Gr + '*' (0xb3, 179) */
                case 0:  /* NULL - Must be ignored - generated by Linux telnet */
                case 10: /* LF (Line Feed) - Must be ignored on Windows platform */
                    return;
                default:
                    /* Store a standard character */
                    vtty_store(vtty, (u_char)c);
                    return;
            }

        case VTTY_INPUT_VT1:
            switch (c) {
                case 0x5b:
                    vtty->input_state = VTTY_INPUT_VT2;
                    return;
                default:
                    vtty_store(vtty, 0x1b);
                    vtty_store(vtty, (u_char)c);
            }
            vtty->input_state = VTTY_INPUT_TEXT;
            return;

        case VTTY_INPUT_VT2:
            switch (c) {
                case 0x41: /* Up Arrow */
                    vtty_store(vtty, 16);
                    break;
                case 0x42: /* Down Arrow */
                    vtty_store(vtty, 14);
                    break;
                case 0x43: /* Right Arrow */
                    vtty_store(vtty, 6);
                    break;
                case 0x44: /* Left Arrow */
                    vtty_store(vtty, 2);
                    break;
                default:
                    vtty_store(vtty, 0x5b);
                    vtty_store(vtty, 0x1b);
                    vtty_store(vtty, (u_char)c);
                    break;
            }
            vtty->input_state = VTTY_INPUT_TEXT;
            return;
    }
}

/**
 * vtty_bytes
 * @brief   
 * @param   vtty
 * @return  int
 */
int vtty_bytes(vtty_t *vtty)
{
    if (vtty->read_ptr < vtty->write_ptr)
        return vtty->write_ptr - vtty->read_ptr;
    else if (vtty->read_ptr == vtty->write_ptr) {
        return VTTY_BUFFER_SIZE;
    } else
        return VTTY_BUFFER_SIZE - (vtty->read_ptr - vtty->write_ptr);
}

/**
 * vtty_is_full
 * @brief   
 * @param   vtty
 * @return  int
 */
int vtty_is_full(vtty_t *vtty)
{
    return (vtty->read_ptr == vtty->write_ptr);
}

/* Read a character from the buffer (-1 if the buffer is empty) */
int vtty_get_char(vtty_t *vtty)
{
    u_char c;

    VTTY_LOCK(vtty);

    if (vtty->read_ptr == vtty->write_ptr) {
        VTTY_UNLOCK(vtty);
        return -1;
    }

    c = vtty->buffer[vtty->read_ptr++];

    if (vtty->read_ptr == VTTY_BUFFER_SIZE)
        vtty->read_ptr = 0;

    VTTY_UNLOCK(vtty);
    return c;
}

/* Returns TRUE if a character is available in buffer */
int vtty_is_char_avail(vtty_t *vtty)
{
    int res;

    VTTY_LOCK(vtty);
    res = (vtty->read_ptr != vtty->write_ptr);
    VTTY_UNLOCK(vtty);
    return res;
}

/* Put char to vtty */
void vtty_put_char(vtty_t *vtty, char ch)
{
    switch (vtty->type) {
        case VTTY_TYPE_NONE:
            break;

        case VTTY_TYPE_TERM:
            fwrite(&ch, 1, 1, vtty->fstream);
            break;

        default:
            fprintf(stderr, "vtty_put_char: bad vtty type %d\n", vtty->type);
            exit(1);
    }
}

/* Put a buffer to vtty */
void vtty_put_buffer(vtty_t *vtty, char *buf, size_t len)
{
    size_t i;

    for (i = 0; i < len; i++)
        vtty_put_char(vtty, buf[i]);
}

/* Flush VTTY output */
void vtty_flush(vtty_t *vtty)
{
    switch (vtty->type) {
        case VTTY_TYPE_TERM:
            if (vtty->fstream)
                fflush(vtty->fstream);
            break;
    }
}

/* VTTY thread */
_Noreturn static void *vtty_thread_main(void *arg)
{
    vtty_t *vtty;
    struct timeval tv;
    int fd, fd_max, res;
    fd_set rfds;

    for (;;) {
        VTTY_LIST_LOCK();

        /* Build the FD set */
        FD_ZERO(&rfds);
        fd_max = -1;
        for (vtty = vtty_list; vtty; vtty = vtty->next) {
            if (!vtty->select_fd)
                continue;

            if ((fd = *vtty->select_fd) < 0)
                continue;

            if (fd > fd_max)
                fd_max = fd;
            FD_SET(fd, &rfds);
        }
        VTTY_LIST_UNLOCK();

        /* Wait for incoming data */
        tv.tv_sec = 0;
        tv.tv_usec = 50 * 1000; /* 50 ms */
        res = select(fd_max + 1, &rfds, NULL, NULL, &tv);

        if (res == -1) {
            if (errno != EINTR) {
                perror("vtty_thread: select");

                for (vtty = vtty_list; vtty; vtty = vtty->next) {
                    fprintf(stderr, "  : %s, FD %d\n", vtty->name, vtty->fd);
                }
            }
            continue;
        }

        /* Examine active FDs and call user handlers */
        VTTY_LIST_LOCK();


        for (vtty = vtty_list; vtty; vtty = vtty->next) {
            if (!vtty->select_fd)
                continue;

            if ((fd = *vtty->select_fd) < 0)
                continue;

            if (FD_ISSET(fd, &rfds)) {
                vtty_read_and_store(vtty);
                vtty->input_pending = TRUE;
            }

            if (vtty->input_pending) {
                if (vtty->read_notifier != NULL)
                    vtty->read_notifier(vtty);


                vtty->input_pending = FALSE;
            }


            /* Flush any pending output */
            if (!vtty->managed_flush)
                vtty_flush(vtty);
        }
        VTTY_LIST_UNLOCK();
    }
}

/* Initialize the VTTY thread */
int vtty_init(void)
{
    if (pthread_create(&vtty_thread, NULL, vtty_thread_main, NULL)) {
        perror("vtty: pthread_create");
        return -1;
    }

    return 0;
}
