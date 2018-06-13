/*
 * Compile_Defines.h
 *
 *  Created on: 23.05.2017
 *      Author: Stalker1290
 */

#ifndef __COMPILE_DEFINES_H_
#define __COMPILE_DEFINES_H_


#ifdef CORE_A7_1
#define USE_CONSOLE
//#define EthM_console
//#define Arp_console
//#define SPIFI_console

#define USE_Ethernet

#endif


#ifdef CORE_A7_0
#define USE_CONSOLE
//#define EthM_console
//#define Arp_console
//#define USB_console
//#define RNDIS_console
//#define VCOM_console
#define QSPI_console
//#define TCP_server_CONSOLE

#define USE_Ethernet
#define USE_LWIP
#define USE_FS

#endif


#endif /* COMPILE_DEFINES_H_ */
