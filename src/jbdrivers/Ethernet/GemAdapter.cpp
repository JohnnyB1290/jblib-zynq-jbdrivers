/**
 * @file
 * @brief GEM Adapter Driver Realization
 *
 *
 * @note
 * Copyright © 2019 Evgeniy Ivanov. Contacts: <strelok1290@gmail.com>
 * All rights reserved.
 * @note
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 * @note
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @note
 * This file is a part of JB_Lib.
 */

// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include <string.h>
#include "jbdrivers/Ethernet/GemAdapter.hpp"
#include "jbdrivers/JbController.hpp"
#include "sleep.h"
#include "xil_mmu.h"
#include "xil_cache.h"
#if (USE_CONSOLE && ETHERNET_PHY_USE_CONSOLE)
#include <stdio.h>
#endif

/*
 * SLCR setting
 */
#define SLCR_LOCK_ADDR				(XPS_SYS_CTRL_BASEADDR + 0x4)
#define SLCR_UNLOCK_ADDR			(XPS_SYS_CTRL_BASEADDR + 0x8)
#define SLCR_GEM0_CLK_CTRL_ADDR		(XPS_SYS_CTRL_BASEADDR + 0x140)
#define SLCR_GEM1_CLK_CTRL_ADDR		(XPS_SYS_CTRL_BASEADDR + 0x144)

#define SLCR_LOCK_KEY_VALUE			0x767B
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

#define RXBD_SPACE_BYTES_0 XEmacPs_BdRingMemCalc(XEMACPS_BD_ALIGNMENT, GEM_ADAPTER_0_NUM_RX_BD)
#define TXBD_SPACE_BYTES_0 XEmacPs_BdRingMemCalc(XEMACPS_BD_ALIGNMENT, GEM_TXBD_CNT_0)
#define RXBD_SPACE_BYTES_1 XEmacPs_BdRingMemCalc(XEMACPS_BD_ALIGNMENT, GEM_ADAPTER_1_NUM_RX_BD)
#define TXBD_SPACE_BYTES_1 XEmacPs_BdRingMemCalc(XEMACPS_BD_ALIGNMENT, GEM_TXBD_CNT_1)

namespace jblib
{
namespace jbdrivers
{

using namespace jbkernel;

GemAdapter* GemAdapter::gemAdapters_[GEM_ADAPTER_NUM_GEM] = {
		(GemAdapter*)NULL,(GemAdapter*)NULL
};
uint8_t GemAdapter::macs_[GEM_ADAPTER_NUM_GEM][6];



GemAdapter* GemAdapter::getGemAdapter(uint8_t number, MdioController* mdioController,
		uint32_t phyAddr, GmiiRgmiiAdapter* gmiiRgmiiAdapter)
{
	if(number >= GEM_ADAPTER_NUM_GEM)
		return (GemAdapter*)NULL;
	if(gemAdapters_[number] == (GemAdapter*)NULL)
		gemAdapters_[number] = new GemAdapter(number, mdioController, phyAddr,
				gmiiRgmiiAdapter);
	return gemAdapters_[number];
}



GemAdapter* GemAdapter::getGemAdapter(uint8_t number, MdioController* mdioController,
		uint32_t phyAddr)
{
	if(number >= GEM_ADAPTER_NUM_GEM)
		return (GemAdapter*)NULL;
	if(gemAdapters_[number] == (GemAdapter*)NULL)
		gemAdapters_[number] = new GemAdapter(number, mdioController, phyAddr, NULL);
	return gemAdapters_[number];
}



GemAdapter* GemAdapter::getGemAdapter(uint8_t number, MdioController* mdioController)
{
	if(number >= GEM_ADAPTER_NUM_GEM)
		return (GemAdapter*)NULL;
	if(gemAdapters_[number] == (GemAdapter*)NULL)
		gemAdapters_[number] = new GemAdapter(number, mdioController, number, NULL);
	return gemAdapters_[number];
}



GemAdapter* GemAdapter::getGemAdapter(uint8_t number)
{
	if(number >= GEM_ADAPTER_NUM_GEM)
		return (GemAdapter*)NULL;
	if(gemAdapters_[number] == (GemAdapter*)NULL)
		gemAdapters_[number] = new GemAdapter(number,
				MdioController::getMdioController(number), number, NULL);
	return gemAdapters_[number];
}



GemAdapter::GemAdapter(uint8_t number, MdioController* mdioController,
		uint32_t phyAddr, GmiiRgmiiAdapter* gmiiRgmiiAdapter) :
				IVoidEthernet(), IVoidCallback(), IIrqListener()
{
	GemAdapter::macs_[number][0] = 0x08;
	GemAdapter::macs_[number][1] = 0x12;
	GemAdapter::macs_[number][2] = (std::rand()) & 0xff;
	GemAdapter::macs_[number][3] = (std::rand()) & 0xff;
	GemAdapter::macs_[number][4] = (std::rand()) & 0xff;
	GemAdapter::macs_[number][5] = (std::rand()) & 0xff;

	this->number_ = number;
	this->mdioController_ = mdioController;
	this->phyAddr_ = phyAddr;
	this->gmiiRgmiiAdapter_ = gmiiRgmiiAdapter;
	this->isSuccessfulInitialized_ = 0;
	this->isSuccessfulAutonegotiation_ = 0;
	this->txUnlocked_ = true;

	this->ethernetPhyController_ =
			new EthernetPhyController(this->mdioController_, this->phyAddr_);

	XEmacPs_Config* config = NULL;
	switch(number)
	{
		case 0:
			memcpy(this->name_, "Eth_phy0", 9);
			this->txFreeLine_ = GEM_ADAPTER_0_TX_BD_FREE_LINE;
			this->numRxBd_ = GEM_ADAPTER_0_NUM_RX_BD;
			config = XEmacPs_LookupConfig(EMACPS_0_DEVICE_ID);
			break;
		case 1:
			memcpy(this->name_, "Eth_phy1",9);
			this->txFreeLine_ = GEM_ADAPTER_1_TX_BD_FREE_LINE;
			this->numRxBd_ = GEM_ADAPTER_1_NUM_RX_BD;
			config = XEmacPs_LookupConfig(EMACPS_1_DEVICE_ID);
			break;
	}

	LONG status = XEmacPs_CfgInitialize(&this->emacPs_, config, config->BaseAddress);
	if (status != XST_SUCCESS){
		#if (USE_CONSOLE && ETHERNET_PHY_USE_CONSOLE)
		printf("GEM Adapter Error: in CfgInitialize Emacps %i\r\n", number);
		#endif
	}
	XEmacPs_SetMdioDivisor(&this->emacPs_, MDC_DIV_224);
	sleep(1);
}



void GemAdapter::initialize(void)
{
	#if (USE_CONSOLE && ETHERNET_PHY_USE_CONSOLE)
	printf("GEM Adapter: Start initialize GEM%u\r\n", this->number_);
	#endif

	LONG status = XEmacPs_SetOptions(&this->emacPs_, XEMACPS_PROMISC_OPTION);
	#if (USE_CONSOLE && ETHERNET_PHY_USE_CONSOLE)
	if (status != XST_SUCCESS)
		printf("GEM Adapter Error: in set promisc mode\r\n");
	#endif

	status = XEmacPs_SetOptions(&this->emacPs_, XEMACPS_RX_CHKSUM_ENABLE_OPTION);
	#if (USE_CONSOLE && ETHERNET_PHY_USE_CONSOLE)
	if (status != XST_SUCCESS)
		printf("GEM Adapter Error: in set RX_CHKSUM_ENABLE\r\n");
	#endif

	status = XEmacPs_SetOptions(&this->emacPs_, XEMACPS_TX_CHKSUM_ENABLE_OPTION);
	#if (USE_CONSOLE && ETHERNET_PHY_USE_CONSOLE)
	if (status != XST_SUCCESS)
		printf("GEM Adapter Error: in set TX_CHKSUM_ENABLE\r\n");
	#endif

	uint32_t tempSpeed = 0;
	GmiiRgmiiSpeedModeControl_t adapterSpeed = ADAPTER_SPEED_MODE_100;
	switch(this->speed_)
	{
		case SPEED_10_M:
		{
			tempSpeed = 10;
			adapterSpeed = ADAPTER_SPEED_MODE_10;
		}
		break;

		case SPEED_100_M:
		{
			tempSpeed = 100;
			adapterSpeed = ADAPTER_SPEED_MODE_100;
		}
		break;

		case SPEED_1000_M:
		{
			tempSpeed = 1000;
			adapterSpeed = ADAPTER_SPEED_MODE_1000;
		}
		break;

		case SPEED_AUTONEG:
		{
			tempSpeed = this->ethernetPhyController_->getSpeed();
			if(tempSpeed != XST_FAILURE){
				#if (USE_CONSOLE && ETHERNET_PHY_USE_CONSOLE)
				printf("GEM Adapter: Speed after autoneg = %lu\r\n", tempSpeed);
				#endif
				switch(tempSpeed)
				{
					case 10:
						adapterSpeed = ADAPTER_SPEED_MODE_10;
						break;
					case 100:
						adapterSpeed = ADAPTER_SPEED_MODE_100;
						break;
					case 1000:
						adapterSpeed = ADAPTER_SPEED_MODE_1000;
						break;
					default:
						adapterSpeed = ADAPTER_SPEED_MODE_100;
						break;
				}
				this->isSuccessfulAutonegotiation_ = 1;
			}
			else{
				tempSpeed = 100;
				adapterSpeed = ADAPTER_SPEED_MODE_100;
				JbController::addMainProcedure(this);
			}
		}
		break;

		default:
		{
			tempSpeed = 100;
			adapterSpeed = ADAPTER_SPEED_MODE_100;
		}
		break;
	}

	if(this->speed_ != SPEED_AUTONEG)
		this->ethernetPhyController_->configureSpeed(tempSpeed);
	EthernetPhyController::setUpSlcrDivisors(this->emacPs_.Config.BaseAddress,
			tempSpeed);
	if(this->gmiiRgmiiAdapter_)
		this->gmiiRgmiiAdapter_->setSpeed(adapterSpeed);
	XEmacPs_SetOperatingSpeed(&this->emacPs_, tempSpeed);

	status = XEmacPs_SetMacAddress(&this->emacPs_,
			(void*)macs_[this->number_], 1);
	#if (USE_CONSOLE && ETHERNET_PHY_USE_CONSOLE)
	if (status != XST_SUCCESS)
		printf("GEM Adapter Error: setting MAC address\r\n");
	#endif

	status = XEmacPs_SetHandler(&this->emacPs_, XEMACPS_HANDLER_DMASEND,
			(void*) GemAdapter::sendHandler, this);
	status |= XEmacPs_SetHandler(&this->emacPs_, XEMACPS_HANDLER_DMARECV,
			(void*) GemAdapter::recieveHandler, this);
	status |= XEmacPs_SetHandler(&this->emacPs_, XEMACPS_HANDLER_ERROR,
			(void*) GemAdapter::errorHandler, this);

	#if (USE_CONSOLE && ETHERNET_PHY_USE_CONSOLE)
	if (status != XST_SUCCESS)
		printf("GEM Adapter Error: assigning handlers\r\n");
	#endif

	XEmacPs_Bd bdTemplate;
	XEmacPs_BdClear(&bdTemplate);

	if(this->number_ == 0)
		status = XEmacPs_BdRingCreate(&(XEmacPs_GetRxRing(&this->emacPs_)),
			(UINTPTR)GEM_ADAPTER_0_RX_BD_LIST_START_ADDRESS,
			(UINTPTR)GEM_ADAPTER_0_RX_BD_LIST_START_ADDRESS,
			XEMACPS_BD_ALIGNMENT, GEM_ADAPTER_0_NUM_RX_BD);
	else if(this->number_ == 1)
		status = XEmacPs_BdRingCreate(&(XEmacPs_GetRxRing(&this->emacPs_)),
			(UINTPTR)GEM_ADAPTER_1_RX_BD_LIST_START_ADDRESS,
			(UINTPTR)GEM_ADAPTER_1_RX_BD_LIST_START_ADDRESS,
			XEMACPS_BD_ALIGNMENT, GEM_ADAPTER_1_NUM_RX_BD);

	#if (USE_CONSOLE && ETHERNET_PHY_USE_CONSOLE)
	if (status != XST_SUCCESS)
		printf("GEM Adapter Error: setting up RxBD space, BdRingCreate\r\n");
	#endif

	status = XEmacPs_BdRingClone(&(XEmacPs_GetRxRing(&this->emacPs_)),&bdTemplate,
			XEMACPS_RECV);
	#if (USE_CONSOLE && ETHERNET_PHY_USE_CONSOLE)
	if(status != XST_SUCCESS)
		printf("GEM Adapter Error: setting up RxBD space, BdRingClone\r\n");
	#endif
	/*
	 * The BDs need to be allocated in uncached memory. Hence the 1 MB
	 * address range that starts at address 0xFF00000 is made uncached.
	 */
	if(this->number_ == 0) {
		Xil_SetTlbAttributes(GEM_ADAPTER_0_RX_BD_LIST_START_ADDRESS, STRONG_ORDERED);
		Xil_SetTlbAttributes(GEM_ADAPTER_0_TX_BD_LIST_START_ADDRESS, STRONG_ORDERED);
	}
	else if(this->number_ == 1) {
		Xil_SetTlbAttributes(GEM_ADAPTER_1_RX_BD_LIST_START_ADDRESS, STRONG_ORDERED);
		Xil_SetTlbAttributes(GEM_ADAPTER_1_TX_BD_LIST_START_ADDRESS, STRONG_ORDERED);
	}

	XEmacPs_BdClear(&bdTemplate);
	XEmacPs_BdSetStatus(&bdTemplate, XEMACPS_TXBUF_USED_MASK);

	if(this->number_ == 0)
		status = XEmacPs_BdRingCreate(&(XEmacPs_GetTxRing(&this->emacPs_)),
			(UINTPTR) GEM_ADAPTER_0_TX_BD_LIST_START_ADDRESS,
			(UINTPTR) GEM_ADAPTER_0_TX_BD_LIST_START_ADDRESS,
			XEMACPS_BD_ALIGNMENT, GEM_ADAPTER_NUM_TX_BD);
	else if(this->number_ == 1)
		status = XEmacPs_BdRingCreate(&(XEmacPs_GetTxRing(&this->emacPs_)),
			(UINTPTR) GEM_ADAPTER_1_TX_BD_LIST_START_ADDRESS,
			(UINTPTR) GEM_ADAPTER_1_TX_BD_LIST_START_ADDRESS,
			XEMACPS_BD_ALIGNMENT, GEM_ADAPTER_NUM_TX_BD);
	#if (USE_CONSOLE && ETHERNET_PHY_USE_CONSOLE)
	if (status != XST_SUCCESS)
		printf("GEM Adapter Error: setting up TxBD space, BdRingCreate\r\n");
	#endif

	status = XEmacPs_BdRingClone(&(XEmacPs_GetTxRing(&this->emacPs_)),&bdTemplate,
			XEMACPS_SEND);
	#if (USE_CONSOLE && ETHERNET_PHY_USE_CONSOLE)
	if (status != XST_SUCCESS)
		printf("GEM Adapter Error: setting up TxBD space, BdRingClone\r\n");
	#endif

	if(this->number_ == 0) {
		IrqController::getIrqController()->addPeripheralIrqListener(this,
				EMACPS_0_INTR_ID);
		IrqController::getIrqController()->setPriority(EMACPS_0_INTR_ID,
				GEM_ADAPTER_0_INTERRUPT_PRIORITY);
		IrqController::getIrqController()->enableInterrupt(EMACPS_0_INTR_ID);
	}
	else if(this->number_ == 1) {
		IrqController::getIrqController()->addPeripheralIrqListener(this,
				EMACPS_1_INTR_ID);
		IrqController::getIrqController()->setPriority(EMACPS_1_INTR_ID,
				GEM_ADAPTER_1_INTERRUPT_PRIORITY);
		IrqController::getIrqController()->enableInterrupt(EMACPS_1_INTR_ID);
	}

	for(uint32_t i = 0; i < this->numRxBd_; i++) {
		Xil_DCacheFlushRange((UINTPTR)&(this->rxQueue_.frames[i]),
				sizeof(EthernetFrame));
		status = XEmacPs_BdRingAlloc(&(XEmacPs_GetRxRing(&this->emacPs_)), 1,
				(XEmacPs_Bd**)&(this->rxQueue_.bdRx[i]));
		#if (USE_CONSOLE && ETHERNET_PHY_USE_CONSOLE)
		if (status != XST_SUCCESS)
			printf("GEM Adapter Error: allocating RxBD, GEM%u\r\n", this->number_);
		#endif
		XEmacPs_BdSetAddressRx(this->rxQueue_.bdRx[i],
				(UINTPTR)&(this->rxQueue_.frames[i]));
		status = XEmacPs_BdRingToHw(&(XEmacPs_GetRxRing(&this->emacPs_)), 1,
				this->rxQueue_.bdRx[i]);
		#if (USE_CONSOLE && ETHERNET_PHY_USE_CONSOLE)
		if (status != XST_SUCCESS)
			printf("GEM Adapter Error: committing RxBD to HW, GEM%u\r\n", this->number_);
		#endif
	}

	XEmacPs_SetQueuePtr(&this->emacPs_, (&this->emacPs_)->RxBdRing.BaseBdAddr, 0,
			XEMACPS_RECV);
	XEmacPs_SetQueuePtr(&this->emacPs_, (&this->emacPs_)->TxBdRing.BaseBdAddr, 0,
			XEMACPS_SEND);
	XEmacPs_Start(&this->emacPs_);

	this->isSuccessfulInitialized_ = 1;

	#if (USE_CONSOLE && ETHERNET_PHY_USE_CONSOLE)
	printf("GEM Adapter: GEM%u Successfully initialized!\r\n", this->number_);
	#endif

	this->txFramesCounter_ = 0;
	this->txBytesCounter_ = 0;
	this->rxFramesCounter_ = 0;
	this->rxBytesCounter_ = 0;
	this->errorsCounter_ = 0;
}



void GemAdapter::start(void)
{
	XEmacPs_Start(&this->emacPs_);
}



void GemAdapter::reset(void)
{
	this->isSuccessfulInitialized_ = 0;
	this->isSuccessfulAutonegotiation_ = 0;
	XEmacPs_Reset(&this->emacPs_);
	this->initialize();
}



void GemAdapter::getParameter(const uint8_t number, void* const value)
{
	if(number == PARAMETER_MAC)
		*((uint8_t**)value) = macs_[this->number_];
	else if(number == PARAMETER_LINK){
		if(this->checkLink())
			*((uint32_t*)value) = 1;
		else
			*((uint32_t*)value) = 0;
	}
	else if(number == PARAMETER_TX_UNLOCK){
		if(this->txUnlocked_)
			*((uint32_t*)value) = 1;
		else
			*((uint32_t*)value) = 0;
	}
	else if(number == PARAMETER_NAME)
		*((char**)value) = (char*)this->name_;
}



void GemAdapter::setParameter(const uint8_t number, void* const value)
{
	if(number == PARAMETER_MAC)
		memcpy(macs_[this->number_], value, 6);
	else if(number == PARAMETER_TX_UNLOCK){
		if(*((uint32_t*)value))
			this->txUnlocked_ = true;
		else
			this->txUnlocked_ = false;
	}
	else if(number == PARAMETER_NAME)
		memcpy(this->name_, value, 9);
	else if(number == PARAMETER_SPEED)
		this->speed_ = *((IVoidEthernetSpeed_t*)value);
}



bool GemAdapter::isTxQueueFull(void)
{
	uint16_t count = D_A_MIN_B_MOD_C(this->txQueue_.queueBw,
			this->txQueue_.queueBr, GEM_ADAPTER_TX_QUEUE_SIZE);
	if(count >= (GEM_ADAPTER_TX_QUEUE_SIZE - GEM_ADAPTER_NUM_TX_BD))
		return true;
	else
		return false;
}



void GemAdapter::addToTxQueue(EthernetFrame* const frame, uint16_t frameSize)
{
	disableInterrupts();
	if(this->txUnlocked_) {
		uint16_t count = D_A_MIN_B_MOD_C(this->txQueue_.queueBw,
				this->txQueue_.queueBr, GEM_ADAPTER_TX_QUEUE_SIZE);
		if(count == (GEM_ADAPTER_TX_QUEUE_SIZE - GEM_ADAPTER_NUM_TX_BD) ) {
			#if (USE_CONSOLE && ETHERNET_PHY_USE_CONSOLE)
			printf("GEM Adapter: TX queue overflow\r\n");
			#endif
			enableInterrupts();
			return;
		}
		uint16_t qIndex = this->txQueue_.queueBw;
		memcpy((uint8_t*)&(this->txQueue_.frames[qIndex]), frame, frameSize);
		this->txQueue_.frameSizes[qIndex] = frameSize;
		this->txQueue_.queueBw++;
		if(this->txQueue_.queueBw == GEM_ADAPTER_TX_QUEUE_SIZE)
			this->txQueue_.queueBw = 0;
	}
	this->pushTxQueue();
	enableInterrupts();
}



#if USE_LWIP
void GemAdapter::addToTxQueue(struct pbuf* p)
{
	if(p == NULL)
		return;
	uint16_t frameSize = p->tot_len;
	if(frameSize == 0)
		return;
	disableInterrupts();
	if(this->txUnlocked_) {
		uint16_t count = D_A_MIN_B_MOD_C(this->txQueue_.queueBw,
				this->txQueue_.queueBr, GEM_ADAPTER_TX_QUEUE_SIZE);
		if(count == (GEM_ADAPTER_TX_QUEUE_SIZE - GEM_ADAPTER_NUM_TX_BD) ) {
			#if (USE_CONSOLE && ETHERNET_PHY_USE_CONSOLE)
			printf("GEM Adapter: TX queue overflow\r\n");
			#endif
			enableInterrupts();
			return;
		}
		uint16_t qIndex = this->txQueue_.queueBw;
		if(p->next != NULL) {
			uint16_t frameIndex = 0;
			while (p) {
				memcpy(&(this->txQueue_.frames[qIndex][frameIndex]),
						p->payload,p->len);
				frameIndex += p->len;
				p = p->next;
			}
		}
		else
			memcpy(&(this->txQueue_.frames[qIndex]), p->payload, p->tot_len);
		this->txQueue_.frameSizes[qIndex] = frameSize;
		this->txQueue_.queueBw++;
		if(this->txQueue_.queueBw == GEM_ADAPTER_TX_QUEUE_SIZE)
			this->txQueue_.queueBw = 0;
	}
	this->pushTxQueue();
	enableInterrupts();
}
#endif



uint8_t GemAdapter::tx(void)
{
	if(this->checkTxBdReady() == 0)
		return 0;
	u16 qIndex = this->txQueue_.queueBr;
	u16 bdIndex = this->txQueue_.bdBw;
	Xil_DCacheFlushRange((UINTPTR)&(this->txQueue_.frames[qIndex]),
			sizeof(EthernetFrame));

	LONG status = XEmacPs_BdRingAlloc(&(XEmacPs_GetTxRing(&this->emacPs_)), 1,
			(XEmacPs_Bd**)&(this->txQueue_.bdTx[bdIndex]));
	if (status != XST_SUCCESS){
		#if (USE_CONSOLE && ETHERNET_PHY_USE_CONSOLE)
		printf("GEM Adapter Error: allocating TxBD\r\n");
		#endif
	}
	XEmacPs_BdSetAddressTx(this->txQueue_.bdTx[bdIndex],
			(UINTPTR)&(this->txQueue_.frames[qIndex]));
	XEmacPs_BdSetLength(this->txQueue_.bdTx[bdIndex],
			this->txQueue_.frameSizes[qIndex]);
	XEmacPs_BdClearTxUsed(this->txQueue_.bdTx[bdIndex]);
	XEmacPs_BdSetLast(this->txQueue_.bdTx[bdIndex]);

	status = XEmacPs_BdRingToHw(&(XEmacPs_GetTxRing(&this->emacPs_)), 1,
			this->txQueue_.bdTx[bdIndex]);
	if (status != XST_SUCCESS){
		#if (USE_CONSOLE && ETHERNET_PHY_USE_CONSOLE)
		printf("GEM Adapter Error: committing TxBD to HW\r\n");
		#endif
	}
	Xil_DCacheFlushRange((UINTPTR)this->txQueue_.bdTx[bdIndex], 64);
	this->txQueue_.bdBw++;
	if(this->txQueue_.bdBw >= GEM_ADAPTER_NUM_TX_BD)
		this->txQueue_.bdBw = 0;
	XEmacPs_Transmit(&this->emacPs_);
	return 1;
}



uint16_t GemAdapter::pullOutRxFrame(EthernetFrame* const frame)
{
	uint16_t count = D_A_MIN_B_MOD_C(this->rxQueue_.bw, this->rxQueue_.br,
			GEM_ADAPTER_RX_QUEUE_SIZE);
	if(count == 0)
		return 0;
	else {
		int16_t size = this->rxQueue_.frameSizes[this->rxQueue_.br];
		memcpy(frame, &(this->rxQueue_.frames[this->rxQueue_.br]), size);
		this->rxQueue_.br++;
		if(this->rxQueue_.br == GEM_ADAPTER_RX_QUEUE_SIZE)
			this->rxQueue_.br = 0;
		return size;
	}
}



void GemAdapter::irqHandler(uint32_t irqNumber)
{
	XEmacPs_IntrHandler((void*)&this->emacPs_);
}



void GemAdapter::errorHandler(void* callbackData, u8 direction, u32 errorWord)
{
	GemAdapter* gemAdapter = (GemAdapter*)callbackData;
	gemAdapter->errorsCounter_++;
	#if (USE_CONSOLE && ETHERNET_PHY_USE_CONSOLE)
	printf("GEM Adapter Error Handler: \r\n");
	printf("GEM%u: ", gemAdapter->number_);
	switch (direction)
	{
		case XEMACPS_RECV:
			if (errorWord & XEMACPS_RXSR_HRESPNOK_MASK)
				printf("Receive DMA error\r\n");
			if (errorWord & XEMACPS_RXSR_RXOVR_MASK)
				printf("Receive over run\r\n");
			if (errorWord & XEMACPS_RXSR_BUFFNA_MASK)
				printf("Receive buffer not available\r\n");
			break;
		case XEMACPS_SEND:
			if (errorWord & XEMACPS_TXSR_HRESPNOK_MASK)
				printf("Transmit DMA error\r\n");
			if (errorWord & XEMACPS_TXSR_URUN_MASK)
				printf("Transmit under run\r\n");
			if (errorWord & XEMACPS_TXSR_BUFEXH_MASK)
				printf("Transmit buffer exhausted\r\n");
			if (errorWord & XEMACPS_TXSR_RXOVR_MASK)
				printf("Transmit retry excessed limits\r\n");
			if (errorWord & XEMACPS_TXSR_FRAMERX_MASK)
				printf("Transmit collision\r\n");
			if (errorWord & XEMACPS_TXSR_USEDREAD_MASK)
				printf("Transmit buffer not available\r\n");
			break;
	}
	#endif
}



void GemAdapter::sendHandler(void* callbackData)
{

}



void GemAdapter::recieveHandler(void* callbackData)
{
	GemAdapter* gemAdapter = (GemAdapter*)callbackData;

	uint16_t count = D_A_MIN_B_MOD_C(gemAdapter->rxQueue_.bw, gemAdapter->rxQueue_.br,
			GEM_ADAPTER_RX_QUEUE_SIZE);

	if(count >= (GEM_ADAPTER_RX_QUEUE_SIZE - gemAdapter->numRxBd_)) {
		#if (USE_CONSOLE && ETHERNET_PHY_USE_CONSOLE)
		printf("GEM Adapter : RX Queue overflow\r\n");
		#endif
		gemAdapter->rxQueue_.br = gemAdapter->rxQueue_.bw;
	}

	for(uint32_t i = 0; i < gemAdapter->numRxBd_; i++) {
		u16 index = gemAdapter->rxQueue_.bw;
		if(XEmacPs_BdRingFromHwRx( &( XEmacPs_GetRxRing(&gemAdapter->emacPs_) ), 1,
				(XEmacPs_Bd **)&(gemAdapter->rxQueue_.bdRx[index]) ) ) {

			gemAdapter->rxQueue_.frameSizes[index] =
					XEmacPs_BdGetLength(gemAdapter->rxQueue_.bdRx[index]);

			Xil_DCacheInvalidateRange((UINTPTR)&(gemAdapter->rxQueue_.frames[index]),
					sizeof(EthernetFrame));
			Xil_DCacheFlushRange((UINTPTR)&(gemAdapter->rxQueue_.frames[index]),
					sizeof(EthernetFrame));

			XEmacPs_BdClearRxNew(gemAdapter->rxQueue_.bdRx[index]);
			LONG status = XEmacPs_BdRingFree(&(XEmacPs_GetRxRing(&gemAdapter->emacPs_)),
					1, gemAdapter->rxQueue_.bdRx[index]);

			if (status != XST_SUCCESS){
				#if (USE_CONSOLE && ETHERNET_PHY_USE_CONSOLE)
				printf("GEM Adapter Error: freeing up RxBDs\r\n");
				#endif
				gemAdapter->errorsCounter_++;
			}

			gemAdapter->rxFramesCounter_++;
			gemAdapter->rxBytesCounter_ += gemAdapter->rxQueue_.frameSizes[index];

			index = (gemAdapter->rxQueue_.bw + gemAdapter->numRxBd_) %
					GEM_ADAPTER_RX_QUEUE_SIZE;

			gemAdapter->rxQueue_.bw++;
			if(gemAdapter->rxQueue_.bw == GEM_ADAPTER_RX_QUEUE_SIZE)
				gemAdapter->rxQueue_.bw = 0;

			Xil_DCacheFlushRange((UINTPTR)&(gemAdapter->rxQueue_.frames[index]),
					sizeof(EthernetFrame));

			status = XEmacPs_BdRingAlloc(&(XEmacPs_GetRxRing(&gemAdapter->emacPs_)), 1,
					(XEmacPs_Bd **)&(gemAdapter->rxQueue_.bdRx[index]));

			if (status != XST_SUCCESS){
				#if (USE_CONSOLE && ETHERNET_PHY_USE_CONSOLE)
				printf("GEM Adapter Error: allocating RxBD\r\n");
				#endif
				gemAdapter->errorsCounter_++;
			}

			XEmacPs_BdSetAddressRx(gemAdapter->rxQueue_.bdRx[index],
					(UINTPTR)&(gemAdapter->rxQueue_.frames[index]));

			status = XEmacPs_BdRingToHw(&(XEmacPs_GetRxRing(&gemAdapter->emacPs_)), 1,
					gemAdapter->rxQueue_.bdRx[index]);

			if (status != XST_SUCCESS){
				#if (USE_CONSOLE && ETHERNET_PHY_USE_CONSOLE)
				printf("GEM Adapter Error: committing RxBD to HW\r\n");
				#endif
				gemAdapter->errorsCounter_++;
			}
		}
		else
			break;
	}
}



void GemAdapter::voidCallback(void* const source, void* parameter)
{
	this->eventNegotiationCallback();
	if(this->isSuccessfulAutonegotiation_)
		JbController::deleteMainProcedure(this);
}



void GemAdapter::eventNegotiationCallback(void)
{
	if(this->isSuccessfulInitialized_ && (this->isSuccessfulAutonegotiation_ == 0)){
		if(this->checkLink()){
			uint32_t tempSpeed = this->ethernetPhyController_->getSpeed();
			if(tempSpeed != XST_FAILURE){
				#if (USE_CONSOLE && ETHERNET_PHY_USE_CONSOLE)
				printf("GEM Adapter: Speed after autoneg = %lu\r\n", tempSpeed);
				#endif
				GmiiRgmiiSpeedModeControl_t adapterSpeed;
				switch(tempSpeed)
				{
					case 10:
						adapterSpeed = ADAPTER_SPEED_MODE_10;
						break;
					case 100:
						adapterSpeed = ADAPTER_SPEED_MODE_100;
						break;
					case 1000:
						adapterSpeed = ADAPTER_SPEED_MODE_1000;
						break;
					default:
						adapterSpeed = ADAPTER_SPEED_MODE_100;
						break;
				}
				EthernetPhyController::setUpSlcrDivisors(this->emacPs_.Config.BaseAddress,
						tempSpeed);
				if(this->gmiiRgmiiAdapter_)
					this->gmiiRgmiiAdapter_->setSpeed(adapterSpeed);
				XEmacPs_SetOperatingSpeed(&this->emacPs_, tempSpeed);
				this->isSuccessfulAutonegotiation_ = 1;
			}
		}
	}
}



uint8_t GemAdapter::checkTxBdReady(void)
{
	if(this->isSuccessfulInitialized_ == 0)
		return 0;
	uint16_t count = D_A_MIN_B_MOD_C(this->txQueue_.bdBw, this->txQueue_.bdBr,
			GEM_ADAPTER_NUM_TX_BD);
	if(count) {
		for(uint32_t i = 0; i < count; i++) {
			uint16_t index = this->txQueue_.bdBr;
			LONG status = XEmacPs_BdRingFromHwTx(&(XEmacPs_GetTxRing(&this->emacPs_)), 1,
					(XEmacPs_Bd**)&(this->txQueue_.bdTx[index]));
			if(status) {
				status = XEmacPs_BdRingFree(&(XEmacPs_GetTxRing(&this->emacPs_)), 1,
						this->txQueue_.bdTx[index]);
				if (status != XST_SUCCESS) {
					#if (USE_CONSOLE && ETHERNET_PHY_USE_CONSOLE)
					printf("GEM Adapter Error: freeing up TxBDs\r\n");
					#endif
					this->errorsCounter_++;
					break;
				}
				this->txQueue_.bdBr++;
				if(this->txQueue_.bdBr == GEM_ADAPTER_NUM_TX_BD)
					this->txQueue_.bdBr = 0;
			}
			else
				break;
		}
	}
	count = D_A_MIN_B_MOD_C(this->txQueue_.bdBw, this->txQueue_.bdBr,
				GEM_ADAPTER_NUM_TX_BD);
	if(count >= this->txFreeLine_)
		return 0;
	else
		return 1;
}



bool GemAdapter::checkLink(void)
{
	if((this->mdioController_->phyRead(this->phyAddr_, 17)) & 0x400)
		return true;
	else
		return false;
}



void GemAdapter::pushTxQueue(void)
{
	uint16_t count = D_A_MIN_B_MOD_C(this->txQueue_.queueBw,
			this->txQueue_.queueBr, GEM_ADAPTER_TX_QUEUE_SIZE);
	if((this->tx()) && count) {
		this->txFramesCounter_++;
		this->txBytesCounter_ += this->txQueue_.frameSizes[this->txQueue_.queueBr];
		this->txQueue_.queueBr++;
		if(this->txQueue_.queueBr == GEM_ADAPTER_TX_QUEUE_SIZE)
			this->txQueue_.queueBr = 0;
	}
}

}
}
