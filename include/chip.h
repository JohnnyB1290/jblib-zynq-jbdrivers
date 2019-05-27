/*
 * chip.h
 *
 *  Created on: 22.11.2017 г.
 *      Author: Stalker1290
 */
#ifndef __CHIP_H_
#define __CHIP_H_

#ifdef __cplusplus
extern "C"
{
#endif


#include "Compile_Defines.h"
#include "xparameters.h"
#include "xil_cache.h"
#include "xemacps.h"
#include "PCB_defines.hpp"
#include "ring_buffer.h"

#define IS_POWER_OF_TWO(x) (x && !(x & (x-1)))

// S=b1(Sharable) TEX=b001 AP=b11(Access permission Full Access),
// Domain=b1111, C=b0, B=b0
#define setNonCached1Mb(addr) Xil_SetTlbAttributes(addr,NORM_NONCACHE)

#define setNonCachedBlock(addr, sizeInMb) for(uint32_t i = 0; i < sizeInMb; i++) \
											Xil_SetTlbAttributes(addr + i*0x00100000,NORM_NONCACHE);

#define sev() __asm__("sev")

#define LongToBin(n) (((n >> 21 ) & 0x80) | ((n >> 18 ) & 0x40) | ((n >> 15 ) & 0x20) | ((n >> 12 ) & 0x10) | ((n >> 9 ) & 0x08) | ((n >> 6 ) & 0x04) | ((n >> 3 ) & 0x02) |  ((n ) & 0x01) )
#define Bin(n) LongToBin(0x##n##l)

typedef void (*VOID_CALLBACK_t)(void);

#define EMAC_ETH_MAX_FLEN XEMACPS_MAX_VLAN_FRAME_SIZE

typedef char EthernetFrame[XEMACPS_MAX_VLAN_FRAME_SIZE] __attribute__ ((aligned(64)));
typedef bool (*Out_packet_callback_t)(EthernetFrame* Frame_ptr, uint16_t frame_size);

#define __disable_irq() Xil_ExceptionDisable()
#define __enable_irq()	Xil_ExceptionEnable()

#define CRITICAL_SECTION(code) { __disable_irq(); \
								code \
								__enable_irq(); }

#if !defined(MAX)
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif
#if !defined(MIN)
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

#define D_A_MIN_B_MOD_C(a,b,c) ((a >= b)? ((a - b)):((a + c) - b))

#ifdef USE_THREAD_SAFE_MALLOC
#include "stdlib.h"
#include "xil_exception.h"

__inline void* malloc_s(size_t size)
{
	void* ret_ptr = NULL;
	Xil_ExceptionDisable();
	ret_ptr = malloc(size);
	Xil_ExceptionEnable();
	return ret_ptr;
}

__inline void free_s(void * ptr)
{
	Xil_ExceptionDisable();
	free(ptr);
	Xil_ExceptionEnable();
}
#else
#define malloc_s malloc
#define free_s free
#endif

#ifdef __cplusplus
}
#endif

#endif /* __CHIP_H_ */
