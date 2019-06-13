/**
 * @file
 * @brief AXI UART16550 Driver Description
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

#ifndef AXI_UART_16550_HPP_
#define AXI_UART_16550_HPP_

#include "jbkernel/jb_common.h"
#include "jbkernel/IVoidChannel.hpp"
#include "jbdrivers/IrqController.hpp"
#include "jbutilities/RingBuffer.hpp"

namespace jblib
{
namespace jbdrivers
{

using namespace jbkernel;
using namespace jbutilities;

typedef enum
{
	PARAMETER_CTS,
	PARAMETER_CLEAR_RTS,
	PARAMETER_SET_RTS,
	PARAMETER_IS_TX_EMPTY,
}AxiUart16550ChannelParameters_t;



class AxiUart16550 : public IVoidChannel, protected IIrqListener
{
public:
	static AxiUart16550* getAxiUart16550(uint8_t number,
			uint32_t baudrate);
	virtual void initialize(void* (* const mallocFunc)(size_t),
			const uint16_t txBufferSize, IChannelCallback* const callback);
	virtual void deinitialize(void);
	virtual void tx(uint8_t* const buffer, const uint16_t size,
			void* parameter);
	virtual void getParameter(const uint8_t number, void* const value);
	virtual void setParameter(const uint8_t number, void* const value);

private:
	AxiUart16550(uint8_t number, uint32_t baudrate);
	virtual ~AxiUart16550(void);
	virtual void irqHandler(uint32_t irqNumber);

	static AxiUart16550* axiUart16550s_[AXI_UART_16550_NUM_INSTANCES];
	static uint32_t baseAddresses_[AXI_UART_16550_NUM_INSTANCES];
	static uint32_t clocks_[AXI_UART_16550_NUM_INSTANCES];
	static uint32_t interruptIds_[AXI_UART_16550_NUM_INSTANCES];
	static uint32_t interruptPriorities_[AXI_UART_16550_NUM_INSTANCES];
	uint8_t number_ = 0;
	uint32_t baudrate_ = 0;
	RingBuffer* txRingBuffer_ = NULL;
	uint8_t* txBuffer_ = NULL;
	uint16_t txBufferSize_ = 0;
	IChannelCallback* callback_ = NULL;
	bool isInitialized_ = false;
	bool isWriteBusy_ = false;
};

}
}

#endif /* AXI_UART_16550_HPP_ */
