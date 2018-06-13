/*
 * @brief LWIP build option override file
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2012
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * @par
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */

#ifndef __LWIPOPTS_H_
#define __LWIPOPTS_H_

#define WS_TIMEOUT						150  /* WebSocket timeout: X*(HTTPD_POLL_INTERVAL), default is WS_TIMEOUT*4*TCP_TMR_INTERVAL ms */
#define TCP_TMR_INTERVAL 				20
#define TCP_MSL 						5000UL

#define LWIP_DEBUG						1

/* Standalone build */
#define NO_SYS                          1

/* Use LWIP timers */
#define NO_SYS_NO_TIMERS                0

/* Need for memory protection */
#define SYS_LIGHTWEIGHT_PROT            0


/* pbuf buffers in pool. In zero-copy mode, these buffers are
   located in peripheral RAM. In copied mode, they are located in
   internal IRAM */
#define PBUF_POOL_SIZE                  16

/* No padding needed */
#define ETH_PAD_SIZE                    0

#define ETHARP_TABLE_MATCH_NETIF        1

//#define IP_SOF_BROADCAST                1
//#define IP_SOF_BROADCAST_RECV           1

/* The ethernet FCS is performed in hardware. The IP, TCP, and UDP
   CRCs still need to be done in hardware. */
#define CHECKSUM_GEN_IP                 1
#define CHECKSUM_GEN_UDP                0
#define CHECKSUM_GEN_TCP                1
#define CHECKSUM_CHECK_IP               1
#define CHECKSUM_CHECK_UDP              1
#define CHECKSUM_CHECK_TCP              1
#define LWIP_CHECKSUM_ON_COPY           1

/* Use LWIP version of htonx() to allow generic functionality across
   all platforms. If you are using the Cortex Mx devices, you might
   be able to use the Cortex __rev instruction instead. */
#define LWIP_PLATFORM_BYTESWAP          0

/* Raw interface not needed */
#define LWIP_RAW                        0

/* DHCP is ok, UDP is required with DHCP */
#define LWIP_DHCP                       0

#define LWIP_IGMP 						1
#define LWIP_UDP                        1
#define LWIP_ICMP                       1


/* Hostname can be used */
#define LWIP_NETIF_HOSTNAME             1

#define LWIP_BROADCAST_PING             1


#define LWIP_SOCKET                     0
#define LWIP_NETCONN                    0
#define MEMP_NUM_SYS_TIMEOUT            300

#define LWIP_STATS                     	0
#define LINK_STATS                      0
#define LWIP_STATS_DISPLAY              0


#define ETHARP_DEBUG                    LWIP_DBG_OFF
#define NETIF_DEBUG                     LWIP_DBG_OFF
#define PBUF_DEBUG                      LWIP_DBG_OFF
#define ICMP_DEBUG                      LWIP_DBG_OFF
#define IGMP_DEBUG                      LWIP_DBG_OFF
#define IP_DEBUG                        LWIP_DBG_OFF
#define IP_REASS_DEBUG                  LWIP_DBG_OFF
#define RAW_DEBUG                       LWIP_DBG_OFF
#define MEM_DEBUG                       LWIP_DBG_OFF
#define MEMP_DEBUG                      LWIP_DBG_OFF
#define SYS_DEBUG                       LWIP_DBG_OFF
#define TIMERS_DEBUG                    LWIP_DBG_OFF
#define TCP_DEBUG                       LWIP_DBG_OFF
#define TCP_INPUT_DEBUG                 LWIP_DBG_OFF
#define TCP_FR_DEBUG                    LWIP_DBG_OFF
#define TCP_RTO_DEBUG                   LWIP_DBG_OFF
#define TCP_CWND_DEBUG                  LWIP_DBG_OFF
#define TCP_WND_DEBUG                   LWIP_DBG_OFF
#define TCP_OUTPUT_DEBUG                LWIP_DBG_OFF
#define TCP_RST_DEBUG                   LWIP_DBG_OFF
#define TCP_QLEN_DEBUG                  LWIP_DBG_OFF
#define UDP_DEBUG                       LWIP_DBG_OFF
#define TCPIP_DEBUG                     LWIP_DBG_OFF
#define DNS_DEBUG                       LWIP_DBG_OFF

#define HTTPD_DEBUG         			LWIP_DBG_OFF
//#define FTPD_DEBUG

/* This define is custom for the LPC EMAC driver. Enabled it to
   get debug messages for the driver. */
#define EMAC_DEBUG                    LWIP_DBG_OFF

#define LWIP_IP_ACCEPT_UDP_PORT(p)      (((p) == PP_NTOHS(67)))

//#define LWIP_HTTPD_DYNAMIC_HEADERS 		1
//#define LWIP_HTTPD_CUSTOM_FILES       	1
//#define LWIP_HTTPD_DYNAMIC_FILE_READ  	1
#define HTTPD_USE_MEM_POOL  			1

#define LWIP_HTTPD_ABORT_ON_CLOSE_MEM_ERROR 		 1
#define LWIP_HTTPD_KILL_OLD_ON_CONNECTIONS_EXCEEDED	 1
#define LWIP_HTTPD_SUPPORT_11_KEEPALIVE    			 0

#define LWIP_HTTPD_CGI            	1

#define LWIP_HTTPD_SUPPORT_POST		1

#define LWIP_HTTPD_STRNSTR_PRIVATE	1

#define LWIP_HTTPD_Default_USER_Filenames  {"/boot.shtml",   1 },

#define LWIP_HTTPD_SSI            		1
#define LWIP_HTTPD_SSI_INCLUDE_TAG      0
#define LWIP_HTTPD_SSI_RAW        		1
#define LWIP_HTTPD_MAX_TAG_NAME_LEN 	16
#define LWIP_HTTPD_MAX_TAG_INSERT_LEN 	512

/*  ------------------------------------
   ---------- Memory options ----------
   ------------------------------------
*/
/**
 * @defgroup lwip_opts_mem Heap and memory pools
 * @ingroup lwip_opts_infrastructure
 * @{
 */
/**
 * MEM_LIBC_MALLOC==1: Use malloc/free/realloc provided by your C-library
 * instead of the lwip internal allocator. Can save code size if you
 * already use it.
 */
#define MEM_LIBC_MALLOC                 0

/**
 * MEMP_MEM_MALLOC==1: Use mem_malloc/mem_free instead of the lwip pool allocator.
 * Especially useful with MEM_LIBC_MALLOC but handle with care regarding execution
 * speed (heap alloc can be much slower than pool alloc) and usage from interrupts
 * (especially if your netif driver allocates PBUF_POOL pbufs for received frames
 * from interrupt)!
 * ATTENTION: Currently, this uses the heap for ALL pools (also for private pools,
 * not only for internal pools defined in memp_std.h)!
 */
#define MEMP_MEM_MALLOC                 1

/**
 * MEM_ALIGNMENT: should be set to the alignment of the CPU
 *    4 byte alignment -> \#define MEM_ALIGNMENT 4
 *    2 byte alignment -> \#define MEM_ALIGNMENT 2
 */
#define MEM_ALIGNMENT                   4

/**
 * MEM_SIZE: the size of the heap memory. If the application will send
 * a lot of data that needs to be copied, this should be set high.
 */
#define MEM_SIZE                        (20 * 1024 * 1024)

/**
 * MEMP_OVERFLOW_CHECK: memp overflow protection reserves a configurable
 * amount of bytes before and after each memp element in every pool and fills
 * it with a prominent default value.
 *    MEMP_OVERFLOW_CHECK == 0 no checking
 *    MEMP_OVERFLOW_CHECK == 1 checks each element when it is freed
 *    MEMP_OVERFLOW_CHECK >= 2 checks each element in every pool every time
 *      memp_malloc() or memp_free() is called (useful but slow!)
 */
#define MEMP_OVERFLOW_CHECK             0

/**
 * MEMP_SANITY_CHECK==1: run a sanity check after each memp_free() to make
 * sure that there are no cycles in the linked lists.
 */
#define MEMP_SANITY_CHECK               1


/**
 * Set this to 1 if you want to free PBUF_RAM pbufs (or call mem_free()) from
 * interrupt context (or another context that doesn't allow waiting for a
 * semaphore).
 * If set to 1, mem_malloc will be protected by a semaphore and SYS_ARCH_PROTECT,
 * while mem_free will only use SYS_ARCH_PROTECT. mem_malloc SYS_ARCH_UNPROTECTs
 * with each loop so that mem_free can run.
 *
 * ATTENTION: As you can see from the above description, this leads to dis-/
 * enabling interrupts often, which can be slow! Also, on low memory, mem_malloc
 * can need longer.
 *
 * If you don't want that, at least for NO_SYS=0, you can still use the following
 * functions to enqueue a deallocation call which then runs in the tcpip_thread
 * context:
 * - pbuf_free_callback(p);
 * - mem_free_callback(m);
 */
#define LWIP_ALLOW_MEM_FREE_FROM_OTHER_CONTEXT 1


/*
   ---------------------------------
   ---------- TCP options ----------
   ---------------------------------
*/
/**
 * @defgroup lwip_opts_tcp TCP
 * @ingroup lwip_opts_callback
 * @{
 */
/**
 * LWIP_TCP==1: Turn on TCP.
 */
#define LWIP_TCP                        1

/**
 * TCP_TTL: Default Time-To-Live value.
 */
#define TCP_TTL                         (IP_DEFAULT_TTL)

/**
 * TCP_WND: The size of a TCP window.  This must be at least
 * (2 * TCP_MSS) for things to work well.
 * ATTENTION: when using TCP_RCV_SCALE, TCP_WND is the total size
 * with scaling applied. Maximum window value in the TCP header
 * will be TCP_WND >> TCP_RCV_SCALE
 */
#define TCP_WND                         (2 * TCP_MSS)

/**
 * TCP_MAXRTX: Maximum number of retransmissions of data segments.
 */
#define TCP_MAXRTX                      12

/**
 * TCP_SYNMAXRTX: Maximum number of retransmissions of SYN segments.
 */
#define TCP_SYNMAXRTX                   6

/**
 * TCP_QUEUE_OOSEQ==1: TCP will queue segments that arrive out of order.
 * Define to 0 if your device is low on memory.
 */
#define TCP_QUEUE_OOSEQ                 (LWIP_TCP)

/**
 * TCP_MSS: TCP Maximum segment size. (default is 536, a conservative default,
 * you might want to increase this.)
 * For the receive side, this MSS is advertised to the remote side
 * when opening a connection. For the transmit size, this MSS sets
 * an upper limit on the MSS advertised by the remote host.
 */
#define TCP_MSS                         1460

/**
 * TCP_CALCULATE_EFF_SEND_MSS: "The maximum size of a segment that TCP really
 * sends, the 'effective send MSS,' MUST be the smaller of the send MSS (which
 * reflects the available reassembly buffer size at the remote host) and the
 * largest size permitted by the IP layer" (RFC 1122)
 * Setting this to 1 enables code that checks TCP_MSS against the MTU of the
 * netif used for a connection and limits the MSS if it would be too big otherwise.
 */
#define TCP_CALCULATE_EFF_SEND_MSS      1

/**
 * TCP_SND_BUF: TCP sender buffer space (bytes).
 * To achieve good performance, this should be at least 2 * TCP_MSS.
 */
#define TCP_SND_BUF                     (4 * TCP_MSS)

/**
 * TCP_SND_QUEUELEN: TCP sender buffer space (pbufs). This must be at least
 * as much as (2 * TCP_SND_BUF/TCP_MSS) for things to work.
 */
#define TCP_SND_QUEUELEN                ((4 * (TCP_SND_BUF) + (TCP_MSS - 1))/(TCP_MSS))

/**
 * TCP_SNDLOWAT: TCP writable space (bytes). This must be less than
 * TCP_SND_BUF. It is the amount of space which must be available in the
 * TCP snd_buf for select to return writable (combined with TCP_SNDQUEUELOWAT).
 */
#define TCP_SNDLOWAT                    LWIP_MIN(LWIP_MAX(((TCP_SND_BUF)/2), (2 * TCP_MSS) + 1), (TCP_SND_BUF) - 1)

/**
 * TCP_SNDQUEUELOWAT: TCP writable bufs (pbuf count). This must be less
 * than TCP_SND_QUEUELEN. If the number of pbufs queued on a pcb drops below
 * this number, select returns writable (combined with TCP_SNDLOWAT).
 */
#define TCP_SNDQUEUELOWAT               LWIP_MAX(((TCP_SND_QUEUELEN)/2), 5)

/**
 * TCP_OOSEQ_MAX_BYTES: The maximum number of bytes queued on ooseq per pcb.
 * Default is 0 (no limit). Only valid for TCP_QUEUE_OOSEQ==1.
 */
#define TCP_OOSEQ_MAX_BYTES             0

/**
 * TCP_OOSEQ_MAX_PBUFS: The maximum number of pbufs queued on ooseq per pcb.
 * Default is 0 (no limit). Only valid for TCP_QUEUE_OOSEQ==1.
 */
#define TCP_OOSEQ_MAX_PBUFS             0

/**
 * TCP_LISTEN_BACKLOG: Enable the backlog option for tcp listen pcb.
 */
#define TCP_LISTEN_BACKLOG              0

/**
 * The maximum allowed backlog for TCP listen netconns.
 * This backlog is used unless another is explicitly specified.
 * 0xff is the maximum (u8_t).
 */
#define TCP_DEFAULT_LISTEN_BACKLOG      0xff

/**
 * TCP_OVERSIZE: The maximum number of bytes that tcp_write may
 * allocate ahead of time in an attempt to create shorter pbuf chains
 * for transmission. The meaningful range is 0 to TCP_MSS. Some
 * suggested values are:
 *
 * 0:         Disable oversized allocation. Each tcp_write() allocates a new
              pbuf (old behaviour).
 * 1:         Allocate size-aligned pbufs with minimal excess. Use this if your
 *            scatter-gather DMA requires aligned fragments.
 * 128:       Limit the pbuf/memory overhead to 20%.
 * TCP_MSS:   Try to create unfragmented TCP packets.
 * TCP_MSS/4: Try to create 4 fragments or less per TCP packet.
 */
#define TCP_OVERSIZE                    TCP_MSS

/**
 * LWIP_TCP_TIMESTAMPS==1: support the TCP timestamp option.
 * The timestamp option is currently only used to help remote hosts, it is not
 * really used locally. Therefore, it is only enabled when a TS option is
 * received in the initial SYN packet from a remote host.
 */
#define LWIP_TCP_TIMESTAMPS             0

/**
 * TCP_WND_UPDATE_THRESHOLD: difference in window to trigger an
 * explicit window update
 */
#define TCP_WND_UPDATE_THRESHOLD   LWIP_MIN((TCP_WND / 4), (TCP_MSS * 4))

/**
 * LWIP_WND_SCALE and TCP_RCV_SCALE:
 * Set LWIP_WND_SCALE to 1 to enable window scaling.
 * Set TCP_RCV_SCALE to the desired scaling factor (shift count in the
 * range of [0..14]).
 * When LWIP_WND_SCALE is enabled but TCP_RCV_SCALE is 0, we can use a large
 * send window while having a small receive window only.
 */
#define LWIP_WND_SCALE                  1
#define TCP_RCV_SCALE                   0
/**
 * @}
 */
/* Needed for malloc/free */
#include <stdlib.h>

#endif /* __LWIPOPTS_H_ */
