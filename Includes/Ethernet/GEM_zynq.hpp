/*
 * GEM_zynq.hpp
 *
 *  Created on: 10.01.2018 ã.
 *      Author: Stalker1290
 */

#ifndef SRC_GEM_ZYNQ_HPP_
#define SRC_GEM_ZYNQ_HPP_

#include "chip.h"
#include "Defines.h"
#include "Void_Ethernet.hpp"
#include "IRQ_Controller.hpp"
#include "Common_interfaces.hpp"
#include "xemacpsif_physpeed.h"
#include "xemacps.h"		/* defines XEmacPs API */
#include "sleep.h"
#include "xpseudo_asm.h"
#include "xil_mmu.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "xil_types.h"
#include "xil_assert.h"
#include "xil_io.h"
#include "xil_cache.h"

#define Num_of_GEM 2

#define EMACPS_SLCR_DIV_MASK		0xFC0FC0FF
/*
 * SLCR setting
 */
#define SLCR_LOCK_ADDR			(XPS_SYS_CTRL_BASEADDR + 0x4)
#define SLCR_UNLOCK_ADDR		(XPS_SYS_CTRL_BASEADDR + 0x8)
#define SLCR_GEM0_CLK_CTRL_ADDR		(XPS_SYS_CTRL_BASEADDR + 0x140)
#define SLCR_GEM1_CLK_CTRL_ADDR		(XPS_SYS_CTRL_BASEADDR + 0x144)


#define SLCR_LOCK_KEY_VALUE		0x767B
#define SLCR_UNLOCK_KEY_VALUE		0xDF0D
#define SLCR_ADDR_GEM_RST_CTRL		(XPS_SYS_CTRL_BASEADDR + 0x214)

/* CRL APB registers for GEM clock control */
#define CRL_GEM0_REF_CTRL	(XPAR_PSU_CRL_APB_S_AXI_BASEADDR + 0x50)
#define CRL_GEM1_REF_CTRL	(XPAR_PSU_CRL_APB_S_AXI_BASEADDR + 0x54)
#define CRL_GEM2_REF_CTRL	(XPAR_PSU_CRL_APB_S_AXI_BASEADDR + 0x58)
#define CRL_GEM3_REF_CTRL	(XPAR_PSU_CRL_APB_S_AXI_BASEADDR + 0x5C)

#define CRL_GEM_DIV_MASK	0x003F3F00
#define CRL_GEM_1G_DIV0		0x00000C00
#define CRL_GEM_1G_DIV1		0x00010000

#define CSU_VERSION			0xFFCA0044
#define PLATFORM_MASK		0xF000
#define PLATFORM_SILICON	0x0000

#define RXBD_SPACE_BYTES_0 XEmacPs_BdRingMemCalc(XEMACPS_BD_ALIGNMENT, GEM_RXBD_CNT_0)
#define TXBD_SPACE_BYTES_0 XEmacPs_BdRingMemCalc(XEMACPS_BD_ALIGNMENT, GEM_TXBD_CNT_0)
#define RXBD_SPACE_BYTES_1 XEmacPs_BdRingMemCalc(XEMACPS_BD_ALIGNMENT, GEM_RXBD_CNT_1)
#define TXBD_SPACE_BYTES_1 XEmacPs_BdRingMemCalc(XEMACPS_BD_ALIGNMENT, GEM_TXBD_CNT_1)


typedef struct RX_Queue_struct
{
	EthernetFrame Frames[GEM_RX_QUEUE_LENGTH];
	uint16_t frame_size[GEM_RX_QUEUE_LENGTH];
	XEmacPs_Bd* BdRxPtr[GEM_RX_QUEUE_LENGTH];
	uint16_t bw;
	uint16_t br;
}RX_Queue_t;


typedef struct TX_Queue_struct
{
	EthernetFrame Frames[GEM_TX_QUEUE_LENGTH];
	uint16_t frame_size[GEM_TX_QUEUE_LENGTH];
	XEmacPs_Bd* BdTxPtr[GEM_TXBD_CNT];
	uint16_t queue_bw;
	uint16_t queue_br;
	uint16_t bd_bw;
	uint16_t bd_br;
}TX_Queue_t;

class Eth_phy_t:public Ethernet_t,Callback_Interface_t,protected IRQ_LISTENER_t
{
public:
	static Eth_phy_t* get_Ethernet_phy(uint8_t num);
	virtual void Initialize(void);
	virtual void Start(void);
	virtual void ResetDevice(void);
	virtual void GetParameter(uint8_t ParamName, void* ParamValue);
	virtual void SetParameter(uint8_t ParamName, void* ParamValue);
	virtual uint8_t Check_if_TX_queue_not_full(void);
	virtual void Add_to_TX_queue(EthernetFrame* mes,uint16_t m_size);
#ifdef USE_LWIP
	virtual void Add_to_TX_queue(struct pbuf* p);
#endif
	virtual uint16_t Pull_out_RX_Frame(EthernetFrame* Frame_ptr);
private:
	Eth_phy_t(uint8_t num);
	bool Check_link(void);
	uint8_t Check_Tx_bd_ready(void);
	uint8_t Tx(void);
	void Event_negotiation_call(void);
	void Push_TX_queue(void);
	virtual void IRQ(uint32_t IRQ_num);
	virtual void void_callback(void* Intf_ptr, void* parameters);

	static void ErrorHandler(void *CallbackData, u8 Direction, u32 ErrorWord);
	static void SendHandler(void *CallbackData);
	static void RecvHandler(void *CallbackData);


	static Eth_phy_t* Eth_phy_ptr[Num_of_GEM];

	static uint8_t EmacPsMAC[Num_of_GEM][6];
	static uint8_t Autoneg_lock;

	uint8_t num;
	uint8_t initialize_success;
	TX_Queue_t TxqQueue;
	uint16_t TX_free_line;
	RX_Queue_t RxQueue;
	u16 RXBD_CNT;
	XEmacPs EmacPsInstance;
	uint8_t Autonegotiation_succ_flag;
	uint8_t Tx_Unlocked;
	uint8_t Adapter_name[9];
	Ethernet_speed_t speed;
};


#endif /* SRC_GEM_ZYNQ_HPP_ */
