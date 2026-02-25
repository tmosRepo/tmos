/*
 * lwipopts.h
 *
 *  Created on: May 19, 2015
 *      Author: miro
 */

#ifndef BOARDS_RPR250_LWIPOPTS_H_
#define BOARDS_RPR250_LWIPOPTS_H_

#include <tmos.h>

#define LWIP_TCP_PCBS_CNT 11 // it was 8 (2 servers + 3 services * 3 sockets)

#if USE_LWIP_2_2_0
// LWIP_2_2_0
#if USE_LWIP_MDNS
#define LWIP_MDNS_RESPONDER   1
#define MDNS_MAX_SERVICES     3
#else
#define LWIP_MDNS_RESPONDER   0
#endif

#ifdef __cplusplus
extern "C" {
#endif

uint32_t rand(void);
#define LWIP_RAND() rand()

#ifdef __cplusplus
}
#endif
// LWIP_2_2_0
#endif // USE_LWIP_2_2_0
//*****************************************************************************
//
// ---------- lwIP Port Options ----------
//
//*****************************************************************************
#define TCP_TMR_INTERVAL				100			//default is 250
//#define DHCP_EXPIRE_TIMER_MSECS         (60 * 1000)
//#define INCLUDE_HTTPD_SSI
//#define INCLUDE_HTTPD_CGI
#define DYNAMIC_HTTP_HEADERS
//#define INCLUDE_HTTPD_DEBUG

//*****************************************************************************
//
// ---------- Platform specific locking ----------
//
//*****************************************************************************
#define SYS_LIGHTWEIGHT_PROT            0           // default is 0
#define NO_SYS                          1           // default is 0
//#define MEMCPY(dst,src,len)             memcpy(dst,src,len)
//#define SMEMCPY(dst,src,len)            memcpy(dst,src,len)

//*****************************************************************************
//
// ---------- Memory options ----------
//
//*****************************************************************************
#define MEM_LIBC_MALLOC                 1
#if USE_LWIP_2_2_0
#define mem_clib_free tsk_free
#define mem_clib_malloc tsk_malloc
#define mem_clib_calloc tsk_malloc_clear
#else
#define malloc		tsk_malloc
#define free		tsk_free
#endif // USE_LWIP_2_2_0

#define MEM_ALIGNMENT                   4           // default is 1
#define MEM_SIZE                        (32 * 1024)  // was 22K default is 1600, was 16K
//#define MEMP_OVERFLOW_CHECK             0
//#define MEMP_SANITY_CHECK               0
//#define MEM_USE_POOLS                   0
//#define MEMP_USE_CUSTOM_POOLS           0

//*****************************************************************************
//
// ---------- Internal Memory Pool Sizes ----------
//
//*****************************************************************************
#define MEMP_NUM_PBUF                     24    // Default 16, was 16
//#define MEMP_NUM_RAW_PCB                4
#if USE_LWIP_2_2_0
#define MEMP_NUM_UDP_PCB                  5     // Defauul 4 + MDNS
#define MEMP_NUM_TCP_PCB                  20    // Default 5, was 12
#else
#define MEMP_NUM_UDP_PCB                  4     // Defauul 4
#define MEMP_NUM_TCP_PCB                  16    // Default 5, was 12
#endif
//#define MEMP_NUM_TCP_PCB_LISTEN         8
//#define MEMP_NUM_TCP_SEG                16
//#define MEMP_NUM_REASSDATA              5
//#define MEMP_NUM_ARP_QUEUE              30
//#define MEMP_NUM_IGMP_GROUP             8
#if USE_LWIP_2_2_0
#define MEMP_NUM_SYS_TIMEOUT              (7 +3 +3)	//LWIP_TCP + IP_REASSEMBLY + LWIP_ARP + (2*LWIP_DHCP) + LWIP_ACD + LWIP_IGMP + LWIP_DNS + PPP_NUM_TIMEOUTS + (LWIP_IPV6 * (1 + LWIP_IPV6_REASS + LWIP_IPV6_MLD + LWIP_IPV6_DHCP6)))
#else
#define MEMP_NUM_SYS_TIMEOUT              6			//Default 3 (LWIP_DHCP )
#endif //USE_LWIP_2_2_0
//#define MEMP_NUM_NETBUF                 2
//#define MEMP_NUM_NETCONN                4
//#define MEMP_NUM_TCPIP_MSG_API          8
//#define MEMP_NUM_TCPIP_MSG_INPKT        8
#if USE_LWIP_2_2_0
#define PBUF_POOL_SIZE                    40    // Default , was 24
#else
#define PBUF_POOL_SIZE                    24    // Default 16, was 36
#endif
//*****************************************************************************
//
// ---------- ARP options ----------
//
//*****************************************************************************
//#define LWIP_ARP                        1
//#define ARP_TABLE_SIZE                  10
//#define ARP_QUEUEING                    1
//#define ETHARP_TRUST_IP_MAC             1

//*****************************************************************************
//
// ---------- IP options ----------
//
//*****************************************************************************
//#define IP_FORWARD                      0
//#define IP_OPTIONS_ALLOWED              1
#define IP_REASSEMBLY                   0           // default is 1
#define IP_FRAG                         0           // default is 1
//#define IP_REASS_MAXAGE                 3
//#define IP_REASS_MAX_PBUFS              10
//#define IP_FRAG_USES_STATIC_BUF         1
//#define IP_FRAG_MAX_MTU                 1500
//#define IP_DEFAULT_TTL                  255

//*****************************************************************************
//
// ---------- ICMP options ----------
//
//*****************************************************************************
#define LWIP_ICMP                       1
#define ICMP_TTL                       (IP_DEFAULT_TTL)

//*****************************************************************************
//
// ---------- RAW options ----------
//
//*****************************************************************************
//#define LWIP_RAW                        1
//#define RAW_TTL                        (IP_DEFAULT_TTL)

//*****************************************************************************
//
// ---------- DHCP options ----------
//
//*****************************************************************************
#define LWIP_DHCP                       1           // default is 0
//#define DHCP_DOES_ARP_CHECK             ((LWIP_DHCP) && (LWIP_ARP))

//*****************************************************************************
//
// ---------- UPNP options ----------
//
//*****************************************************************************
//#define LWIP_UPNP                       0

//*****************************************************************************
//
// ---------- PTPD options ----------
//
//*****************************************************************************
//#define LWIP_PTPD                       0

//*****************************************************************************
//
// ---------- AUTOIP options ----------
//
//*****************************************************************************
#define LWIP_AUTOIP                     1           // default is 0
#define LWIP_DHCP_AUTOIP_COOP           ((LWIP_DHCP) && (LWIP_AUTOIP))
                                                    // default is 0

//*****************************************************************************
//
// ---------- SNMP options ----------
//
//*****************************************************************************
//#define LWIP_SNMP                       0
//#define SNMP_CONCURRENT_REQUESTS        1
//#define SNMP_TRAP_DESTINATIONS          1
//#define SNMP_PRIVATE_MIB                0
//#define SNMP_SAFE_REQUESTS              1

//*****************************************************************************
//
// ---------- IGMP options ----------
//
//*****************************************************************************
#define LWIP_IGMP                       (LWIP_MDNS_RESPONDER)

//*****************************************************************************
//
// ---------- DNS options -----------
//
//*****************************************************************************
#define LWIP_DNS                        1
//#define DNS_TABLE_SIZE                  4
//#define DNS_MAX_NAME_LENGTH             256
//#define DNS_MAX_SERVERS                 2
//#define DNS_DOES_NAME_CHECK             1
//#define DNS_USES_STATIC_BUF             1
//#define DNS_MSG_SIZE                    512

//*****************************************************************************
//
// ---------- UDP options ----------
//
//*****************************************************************************
#define LWIP_UDP                        1
//#define LWIP_UDPLITE                    0
//#define UDP_TTL                         (IP_DEFAULT_TTL)
#define LWIP_UPNP 1		// Pass UPNP messages regardless of destination address
#define LWIP_PTPD 1		// Pass PTPD messages regardless of destination address

//*****************************************************************************
//
// ---------- TCP options ----------
//
//*****************************************************************************
//#define LWIP_TCP                        1
//#define TCP_TTL                         (IP_DEFAULT_TTL)
#define TCP_MAXRTX                      5		//default is 12
//#define TCP_SYNMAXRTX                   6
//#define TCP_QUEUE_OOSEQ                 1
#define TCP_MSS                    		(1500 - 40)        // default is 536
//#define TCP_CALCULATE_EFF_SEND_MSS      1
#define TCP_SND_BUF                 	(4 * TCP_MSS)
                                                    // default is 256, was 6 *
//#define TCP_SND_QUEUELEN                (4 * (TCP_SND_BUF/TCP_MSS))
#define TCP_WND                        	(2*TCP_MSS)   // default is (4 * TCP_MSS)
//#define TCP_SNDLOWAT                    (TCP_SND_BUF/2)
//#define TCP_LISTEN_BACKLOG              0
//#define TCP_DEFAULT_LISTEN_BACKLOG      0xff

//*****************************************************************************
//
// ---------- API options ----------
//
//*****************************************************************************
//#define LWIP_EVENT_API                  0
//#define LWIP_CALLBACK_API               1

//*****************************************************************************
//
// ---------- Pbuf options ----------
//
//*****************************************************************************
#define PBUF_LINK_HLEN                  16          // default is 14
#define PBUF_POOL_BUFSIZE               256
                                                    // default is LWIP_MEM_ALIGN_SIZE(TCP_MSS+40+PBUF_LINK_HLEN)
#define ETH_PAD_SIZE                    0           // default is 0

//*****************************************************************************
//
// ---------- Network Interfaces options ----------
//
//*****************************************************************************
#define LWIP_NETIF_HOSTNAME               1
//#define LWIP_NETIF_API                  0
#define LWIP_NETIF_STATUS_CALLBACK        1
#define LWIP_NETIF_LINK_CALLBACK          1
//#define LWIP_NETIF_HWADDRHINT           0
//Increase LWIP_NUM_NETIF_CLIENT_DATA by 1 (MDNS needs one entry on netif).
#define LWIP_NUM_NETIF_CLIENT_DATA        (LWIP_MDNS_RESPONDER)


//*****************************************************************************
//
// ---------- LOOPIF options ----------
//
//*****************************************************************************
//#define LWIP_HAVE_LOOPIF                0
//#define LWIP_LOOPIF_MULTITHREADING      1

//*****************************************************************************
//
// ---------- Thread options ----------
//
//*****************************************************************************
//#define TCPIP_THREAD_NAME              "tcpip_thread"
//#define TCPIP_THREAD_STACKSIZE          0
//#define TCPIP_THREAD_PRIO               1
//#define TCPIP_MBOX_SIZE                 0
//#define SLIPIF_THREAD_NAME             "slipif_loop"
//#define SLIPIF_THREAD_STACKSIZE         0
//#define SLIPIF_THREAD_PRIO              1
//#define PPP_THREAD_NAME                "pppMain"
//#define PPP_THREAD_STACKSIZE            0
//#define PPP_THREAD_PRIO                 1
//#define DEFAULT_THREAD_NAME            "lwIP"
//#define DEFAULT_THREAD_STACKSIZE        0
//#define DEFAULT_THREAD_PRIO             1
//#define DEFAULT_RAW_RECVMBOX_SIZE       0
//#define DEFAULT_UDP_RECVMBOX_SIZE       0
//#define DEFAULT_TCP_RECVMBOX_SIZE       0
//#define DEFAULT_ACCEPTMBOX_SIZE         0

//*****************************************************************************
//
// ---------- Sequential layer options ----------
//
//*****************************************************************************
//#define LWIP_TCPIP_CORE_LOCKING         0
#define LWIP_NETCONN                    0           // default is 1

//*****************************************************************************
//
// ---------- Socket Options ----------
//
//*****************************************************************************
#define LWIP_SOCKET                     0           // default is 1
//#define LWIP_COMPAT_SOCKETS             1
//#define LWIP_POSIX_SOCKETS_IO_NAMES     1
//#define LWIP_TCP_KEEPALIVE              0
//#define LWIP_SO_RCVTIMEO                0
//#define LWIP_SO_RCVBUF                  0
#define SO_REUSE                        1

//*****************************************************************************
//
// ---------- Statistics options ----------
//
//*****************************************************************************
//#define LWIP_STATS                      1
//#define LWIP_STATS_DISPLAY              0
//#define LINK_STATS                      1
//#define ETHARP_STATS                    (LWIP_ARP)
//#define IP_STATS                        1
//#define IPFRAG_STATS                    (IP_REASSEMBLY || IP_FRAG)
//#define ICMP_STATS                      1
//#define IGMP_STATS                      (LWIP_IGMP)
//#define UDP_STATS                       (LWIP_UDP)
//#define TCP_STATS                       (LWIP_TCP)
//#define MEM_STATS                       1
//#define MEMP_STATS                      1
//#define SYS_STATS                       1

//*****************************************************************************
//
// ---------- PPP options ----------
//
//*****************************************************************************
//#define PPP_SUPPORT                     0
//#define PPPOE_SUPPORT                   0
//#define PPPOS_SUPPORT                   PPP_SUPPORT

#if PPP_SUPPORT
//#define NUM_PPP                         1
//#define PAP_SUPPORT                     0
//#define CHAP_SUPPORT                    0
//#define MSCHAP_SUPPORT                  0
//#define CBCP_SUPPORT                    0
//#define CCP_SUPPORT                     0
//#define VJ_SUPPORT                      0
//#define MD5_SUPPORT                     0
//#define FSM_DEFTIMEOUT                  6
//#define FSM_DEFMAXTERMREQS              2
//#define FSM_DEFMAXCONFREQS              10
//#define FSM_DEFMAXNAKLOOPS              5
//#define UPAP_DEFTIMEOUT                 6
//#define UPAP_DEFREQTIME                 30
//#define CHAP_DEFTIMEOUT                 6
//#define CHAP_DEFTRANSMITS               10
//#define LCP_ECHOINTERVAL                0
//#define LCP_MAXECHOFAILS                3
//#define PPP_MAXIDLEFLAG                 100

//#define PPP_MAXMTU                      1500
//#define PPP_DEFMRU                      296
#endif

//*****************************************************************************
//
// ---------- checksum options ----------
//
//*****************************************************************************
#define CHECKSUM_BY_HARDWARE


#ifdef CHECKSUM_BY_HARDWARE
  /* CHECKSUM_GEN_IP==0: Generate checksums by hardware for outgoing IP packets.*/
  #define CHECKSUM_GEN_IP                 0
  /* CHECKSUM_GEN_UDP==0: Generate checksums by hardware for outgoing UDP packets.*/
  #define CHECKSUM_GEN_UDP                0
  /* CHECKSUM_GEN_TCP==0: Generate checksums by hardware for outgoing TCP packets.*/
  #define CHECKSUM_GEN_TCP                0
  /* CHECKSUM_CHECK_IP==0: Check checksums by hardware for incoming IP packets.*/
  #define CHECKSUM_CHECK_IP               0
  /* CHECKSUM_CHECK_UDP==0: Check checksums by hardware for incoming UDP packets.*/
  #define CHECKSUM_CHECK_UDP              0
  /* CHECKSUM_CHECK_TCP==0: Check checksums by hardware for incoming TCP packets.*/
  #define CHECKSUM_CHECK_TCP              0
  /* CHECKSUM_CHECK_ICMP==0: Check checksums by hardware for incoming ICMP packets.*/
  #define CHECKSUM_GEN_ICMP               0
#else
  /* CHECKSUM_GEN_IP==1: Generate checksums in software for outgoing IP packets.*/
  #define CHECKSUM_GEN_IP                 1
  /* CHECKSUM_GEN_UDP==1: Generate checksums in software for outgoing UDP packets.*/
  #define CHECKSUM_GEN_UDP                1
  /* CHECKSUM_GEN_TCP==1: Generate checksums in software for outgoing TCP packets.*/
  #define CHECKSUM_GEN_TCP                1
  /* CHECKSUM_CHECK_IP==1: Check checksums in software for incoming IP packets.*/
  #define CHECKSUM_CHECK_IP               1
  /* CHECKSUM_CHECK_UDP==1: Check checksums in software for incoming UDP packets.*/
  #define CHECKSUM_CHECK_UDP              1
  /* CHECKSUM_CHECK_TCP==1: Check checksums in software for incoming TCP packets.*/
  #define CHECKSUM_CHECK_TCP              1
  /* CHECKSUM_CHECK_ICMP==1: Check checksums by hardware for incoming ICMP packets.*/
  #define CHECKSUM_GEN_ICMP               1
#endif
//#define CHECKSUM_GEN_IP                 1
//#define CHECKSUM_GEN_UDP                1
//#define CHECKSUM_GEN_TCP                1
//#define CHECKSUM_CHECK_IP               1
//#define CHECKSUM_CHECK_UDP              1
//#define CHECKSUM_CHECK_TCP              1

//*****************************************************************************
//
// ---------- Debugging options ----------
//
//*****************************************************************************
#if USE_LWIP_1_4_1
#if 0
#define U8_F "c"
#define S8_F "c"
#define X8_F "x"
#define U16_F "u"
#define S16_F "d"
#define X16_F "x"
#define U32_F "u"
#define S32_F "d"
#define X32_F "x"
//extern void UARTprintf(const char *pcString, ...);
#define LWIP_PLATFORM_DIAG(x) {TRACE x; tsk_sleep(2);}
#define LWIP_DEBUG
#endif

//#define LWIP_DBG_MIN_LEVEL              LWIP_DBG_LEVEL_OFF
#define LWIP_DBG_MIN_LEVEL              LWIP_DBG_LEVEL_OFF
//#define LWIP_DBG_MIN_LEVEL              LWIP_DBG_LEVEL_WARNING
//#define LWIP_DBG_MIN_LEVEL              LWIP_DBG_LEVEL_SERIOUS
//#define LWIP_DBG_MIN_LEVEL              LWIP_DBG_LEVEL_SEVERE

//#define LWIP_DBG_TYPES_ON               LWIP_DBG_STATE
#define LWIP_DBG_TYPES_ON               (LWIP_DBG_ON|LWIP_DBG_TRACE|LWIP_DBG_STATE|LWIP_DBG_FRESH)

//#define ETHARP_DEBUG                    LWIP_DBG_ON     // default is OFF
#define NETIF_DEBUG                     LWIP_DBG_ON     // default is OFF
//#define PBUF_DEBUG                      LWIP_DBG_ON     // default is OFF
#define API_LIB_DEBUG                   LWIP_DBG_ON     // default is OFF
#define API_MSG_DEBUG                   LWIP_DBG_ON     // default is OFF
//#define SOCKETS_DEBUG                   LWIP_DBG_ON     // default is OFF
#define ICMP_DEBUG                      LWIP_DBG_ON     // default is OFF
#define IGMP_DEBUG                      LWIP_DBG_ON     // default is OFF
#define INET_DEBUG                      LWIP_DBG_ON     // default is OFF
#define IP_DEBUG                        LWIP_DBG_ON     // default is OFF
#define IP_DEBUG1                       LWIP_DBG_OFF
//#define IP_REASS_DEBUG                  LWIP_DBG_ON     // default is OFF
//#define RAW_DEBUG                       LWIP_DBG_ON     // default is OFF
//#define MEM_DEBUG                       LWIP_DBG_ON     // default is OFF
//#define MEMP_DEBUG                      LWIP_DBG_ON     // default is OFF
#define SYS_DEBUG                       LWIP_DBG_ON     // default is OFF
#define TCP_DEBUG                       LWIP_DBG_ON     // default is OFF
#define TCP_DEBUG1                      LWIP_DBG_OFF
#define TCP_INPUT_DEBUG                 LWIP_DBG_ON     // default is OFF
#define TCP_FR_DEBUG                    LWIP_DBG_ON     // default is OFF
#define TCP_RTO_DEBUG                   LWIP_DBG_ON     // default is OFF
//#define TCP_CWND_DEBUG                  LWIP_DBG_ON     // default is OFF
#define TCP_WND_DEBUG                   LWIP_DBG_ON     // default is OFF
#define TCP_OUTPUT_DEBUG                LWIP_DBG_ON     // default is OFF
#define TCP_OUTPUT_DEBUG1               LWIP_DBG_OFF
#define TCP_RST_DEBUG                   LWIP_DBG_ON     // default is OFF
#define TCP_QLEN_DEBUG                  LWIP_DBG_ON     // default is OFF
//#define UDP_DEBUG                       LWIP_DBG_ON     // default is OFF
#define TCPIP_DEBUG                     LWIP_DBG_ON     // default is OFF
//#define PPP_DEBUG                       LWIP_DBG_ON     // default is OFF
#define SLIP_DEBUG                      LWIP_DBG_ON     // default is OFF
#define DHCP_DEBUG                      LWIP_DBG_ON     // default is OFF
#define AUTOIP_DEBUG                    LWIP_DBG_ON     // default is OFF
//#define SNMP_MSG_DEBUG                  LWIP_DBG_ON     // default is OFF
//#define SNMP_MIB_DEBUG                  LWIP_DBG_ON     // default is OFF
#define DNS_DEBUG                       LWIP_DBG_ON     // default is OFF

#define LWIP_DEBUG_TIMERNAMES			0

#endif //USE_LWIP_1_4_1

#if USE_LWIP_2_2_0
#define LWIP_DEBUG	1
#ifdef LWIP_DEBUG
#define LWIP_PLATFORM_DIAG(x)   do {TRACE x; TRACE_CHAR('\r');} while(0)
#define LWIP_PLATFORM_ASSERT(x) do {TRACE("Assertion \"%s\" failed at line %d in %s\r\n", \
                                     x, __LINE__, __FILE__); } while(0)
#else
#define LWIP_PLATFORM_DIAG(x)
#define LWIP_PLATFORM_ASSERT(x)
#endif // LWIP_2_2_0
/*
   ---------------------------------------
   ---------- Debugging options ----------
   ---------------------------------------
*/
/**
 * @defgroup lwip_opts_debugmsg Debug messages
 * @ingroup lwip_opts_debug
 * @{
 */
/**
 * LWIP_DBG_MIN_LEVEL: After masking, the value of the debug is
 * compared against this value. If it is smaller, then debugging
 * messages are written.
 * @see debugging_levels
 */
#if !defined LWIP_DBG_MIN_LEVEL
#define LWIP_DBG_MIN_LEVEL              LWIP_DBG_LEVEL_ALL
#endif

/**
 * LWIP_DBG_TYPES_ON: A mask that can be used to globally enable/disable
 * debug messages of certain types.
 * @see debugging_levels
 */
#if !defined LWIP_DBG_TYPES_ON
#define LWIP_DBG_TYPES_ON               LWIP_DBG_ON
#endif

/**
 * ETHARP_DEBUG: Enable debugging in etharp.c.
 */
#define ETHARP_DEBUG                    LWIP_DBG_OFF

/**
 * NETIF_DEBUG: Enable debugging in netif.c.
 */
#define NETIF_DEBUG                     LWIP_DBG_OFF

/**
 * PBUF_DEBUG: Enable debugging in pbuf.c.
 */
#define PBUF_DEBUG                      LWIP_DBG_OFF

/**
 * API_LIB_DEBUG: Enable debugging in api_lib.c.
 */
#define API_LIB_DEBUG                   LWIP_DBG_OFF

/**
 * API_MSG_DEBUG: Enable debugging in api_msg.c.
 */
#define API_MSG_DEBUG                   LWIP_DBG_OFF

/**
 * SOCKETS_DEBUG: Enable debugging in sockets.c.
 */
#define SOCKETS_DEBUG                   LWIP_DBG_OFF

/**
 * ICMP_DEBUG: Enable debugging in icmp.c.
 */
#define ICMP_DEBUG                      LWIP_DBG_OFF

/**
 * IGMP_DEBUG: Enable debugging in igmp.c.
 */
#define IGMP_DEBUG                      LWIP_DBG_OFF

/**
 * INET_DEBUG: Enable debugging in inet.c.
 */
#define INET_DEBUG                      LWIP_DBG_OFF

/**
 * IP_DEBUG: Enable debugging for IP.
 */
#define IP_DEBUG                        LWIP_DBG_OFF

/**
 * IP_REASS_DEBUG: Enable debugging in ip_frag.c for both frag & reass.
 */
#define IP_REASS_DEBUG                  LWIP_DBG_OFF

/**
 * RAW_DEBUG: Enable debugging in raw.c.
 */
#define RAW_DEBUG                       LWIP_DBG_OFF

/**
 * MEM_DEBUG: Enable debugging in mem.c.
 */
#define MEM_DEBUG                       LWIP_DBG_ON

/**
 * MEMP_DEBUG: Enable debugging in memp.c.
 */
#define MEMP_DEBUG                      LWIP_DBG_ON

/**
 * SYS_DEBUG: Enable debugging in sys.c.
 */
#define SYS_DEBUG                       LWIP_DBG_OFF

/**
 * TIMERS_DEBUG: Enable debugging in timers.c.
 */

#define TIMERS_DEBUG                    LWIP_DBG_OFF
#define LWIP_DEBUG_TIMERNAMES			0

/**
 * TCP_DEBUG: Enable debugging for TCP.
 */
#define TCP_DEBUG                       LWIP_DBG_OFF

/**
 * TCP_INPUT_DEBUG: Enable debugging in tcp_in.c for incoming debug.
 */
#define TCP_INPUT_DEBUG                 LWIP_DBG_OFF

/**
 * TCP_FR_DEBUG: Enable debugging in tcp_in.c for fast retransmit.
 */
#define TCP_FR_DEBUG                    LWIP_DBG_OFF

/**
 * TCP_RTO_DEBUG: Enable debugging in TCP for retransmit
 * timeout.
 */
#define TCP_RTO_DEBUG                   LWIP_DBG_OFF

/**
 * TCP_CWND_DEBUG: Enable debugging for TCP congestion window.
 */
#define TCP_CWND_DEBUG                  LWIP_DBG_OFF

/**
 * TCP_WND_DEBUG: Enable debugging in tcp_in.c for window updating.
 */
#define TCP_WND_DEBUG                   LWIP_DBG_OFF

/**
 * TCP_OUTPUT_DEBUG: Enable debugging in tcp_out.c output functions.
 */
#define TCP_OUTPUT_DEBUG                LWIP_DBG_OFF

/**
 * TCP_RST_DEBUG: Enable debugging for TCP with the RST message.
 */
#define TCP_RST_DEBUG                   LWIP_DBG_OFF

/**
 * TCP_QLEN_DEBUG: Enable debugging for TCP queue lengths.
 */
#define TCP_QLEN_DEBUG                  LWIP_DBG_OFF

/**
 * UDP_DEBUG: Enable debugging in UDP.
 */
#define UDP_DEBUG                       LWIP_DBG_OFF

/**
 * TCPIP_DEBUG: Enable debugging in tcpip.c.
 */
#define TCPIP_DEBUG                     LWIP_DBG_OFF

/**
 * SLIP_DEBUG: Enable debugging in slipif.c.
 */
#define SLIP_DEBUG                      LWIP_DBG_OFF

/**
 * DHCP_DEBUG: Enable debugging in dhcp.c.
 */
#define DHCP_DEBUG                      LWIP_DBG_OFF

/**
 * AUTOIP_DEBUG: Enable debugging in autoip.c.
 */
#define AUTOIP_DEBUG                    LWIP_DBG_OFF

/**
 * ACD_DEBUG: Enable debugging in acd.c.
 */
#define ACD_DEBUG                       LWIP_DBG_OFF

/**
 * DNS_DEBUG: Enable debugging for DNS.
 */
#define DNS_DEBUG                       LWIP_DBG_OFF

/**
 * IP6_DEBUG: Enable debugging for IPv6.
 */
#define IP6_DEBUG                       LWIP_DBG_OFF

/**
 * DHCP6_DEBUG: Enable debugging in dhcp6.c.
 */
#define DHCP6_DEBUG                     LWIP_DBG_OFF
/**
 * @}
 */

/**
 * LWIP_TESTMODE: Changes to make unit test possible
 */
#define LWIP_TESTMODE                   0


#define MDNS_DEBUG						LWIP_DBG_OFF

/**
 *  ETH hardware receive,transmit and interrupts
 */
#define ETH_DEBUG						LWIP_DBG_OFF

/**
 *  LWIP_PORT_API: Enable debugging in ports/xxx/lwapi.cpp
 */
#define LWIP_PORT_API					LWIP_DBG_OFF

#endif //USE_LWIP_2_2_0

#endif /* BOARDS_RPR250_LWIPOPTS_H_ */
