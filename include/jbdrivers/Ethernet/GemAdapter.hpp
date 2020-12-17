/**
 * @file
 * @brief GEM Adapter Driver Description
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

#ifndef GEM_ADAPTER_HPP_
#define GEM_ADAPTER_HPP_

#include "jbkernel/jb_common.h"
#include "jbkernel/callback_interfaces.hpp"
#include "jbkernel/IVoidEthernet.hpp"
#include "jbdrivers/IrqController.hpp"
#include "jbdrivers/Ethernet/MdioController.hpp"
#include "jbdrivers/Ethernet/GmiiRgmiiAdapter.hpp"
#include "jbdrivers/Ethernet/EthernetPhyController.hpp"
#include "xemacps.h"

#define GEM_ADAPTER_NUM_GEM 2

namespace jblib
{
namespace jbdrivers
{

using namespace jbkernel;

typedef struct
{
	EthernetFrame frames[GEM_ADAPTER_RX_QUEUE_SIZE];
	uint16_t frameSizes[GEM_ADAPTER_RX_QUEUE_SIZE];
	XEmacPs_Bd* bdRx[GEM_ADAPTER_RX_QUEUE_SIZE];
	uint16_t bw = 0;
	uint16_t br = 0;
}GemRxQueue_t;



typedef struct
{
	EthernetFrame frames[GEM_ADAPTER_TX_QUEUE_SIZE];
	uint16_t frameSizes[GEM_ADAPTER_TX_QUEUE_SIZE];
	XEmacPs_Bd* bdTx[GEM_ADAPTER_NUM_TX_BD];
	uint16_t queueBw = 0;
	uint16_t queueBr = 0;
	uint16_t bdBw = 0;
	uint16_t bdBr = 0;
}GemTxQueue_t;



class GemAdapter : public IVoidEthernet, IVoidCallback, protected IIrqListener
{
public:
	static GemAdapter* getGemAdapter(uint8_t number,
			MdioController* mdioController, uint32_t phyAddr,
			GmiiRgmiiAdapter* gmiiRgmiiAdapter);
	static GemAdapter* getGemAdapter(uint8_t number,
			MdioController* mdioController, uint32_t phyAddr);
	static GemAdapter* getGemAdapter(uint8_t number,
			MdioController* mdioController);
	static GemAdapter* getGemAdapter(uint8_t number);
	virtual void initialize(void);
	virtual void start(void);
	virtual void reset(void);
	virtual void getParameter(const uint8_t number, void* const value);
	virtual void setParameter(const uint8_t number, void* const value);
	virtual bool isTxQueueFull(void);
	virtual void addToTxQueue(EthernetFrame* const frame, uint16_t frameSize);
#if USE_LWIP
	virtual void addToTxQueue(struct pbuf* p);
#endif
	virtual uint16_t pullOutRxFrame(EthernetFrame* const frame);
private:
	static void errorHandler(void* callbackData, u8 direction,
			u32 errorWord);
	static void sendHandler(void* callbackData);
	static void recieveHandler(void* callbackData);
	GemAdapter(uint8_t number, MdioController* mdioController,
			uint32_t phyAddr, GmiiRgmiiAdapter* gmiiRgmiiAdapter);
	bool checkLink(void);
	uint8_t checkTxBdReady(void);
	uint8_t tx(void);
	void eventNegotiationCallback(void);
	void pushTxQueue(void);
	virtual void irqHandler(uint32_t irqNumber);
	virtual void voidCallback(void* const source, void* parameter);

	static GemAdapter* gemAdapters_[GEM_ADAPTER_NUM_GEM];
	static uint8_t macs_[GEM_ADAPTER_NUM_GEM][6];
	uint8_t number_ = 0;
	uint8_t isSuccessfulInitialized_ = 0;
	GemTxQueue_t txQueue_{};
	uint16_t txFreeLine_ = 0;
	GemRxQueue_t rxQueue_{};
	u16 numRxBd_ = 0;
	XEmacPs emacPs_;
	uint8_t isSuccessfulAutonegotiation_ = 0;
	bool txUnlocked_ = true;
	char name_[9] = "Eth_phy";
	IVoidEthernetSpeed_t speed_ = SPEED_100_M;
	MdioController* mdioController_ = NULL;
	GmiiRgmiiAdapter* gmiiRgmiiAdapter_ = NULL;
	uint32_t phyAddr_ = 0;
	EthernetPhyController* ethernetPhyController_ = NULL;
};

}
}

#endif /* GEM_ADAPTER_HPP_ */
