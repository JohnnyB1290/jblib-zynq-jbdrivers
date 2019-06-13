/**
 * @file
 * @brief AXI UART16550 Driver Realization
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

#include "jbkernel/jb_common.h"
#if JBDRIVERS_USE_AXI_UART_16550

#include "jbdrivers/AxiUart16550.hpp"
#include "xuartns550_l.h"

namespace jblib
{
namespace jbdrivers
{

using namespace jbkernel;
using namespace jbutilities;

AxiUart16550* AxiUart16550::axiUart16550s_[AXI_UART_16550_NUM_INSTANCES] = {NULL};
uint32_t AxiUart16550::baseAddresses_[AXI_UART_16550_NUM_INSTANCES] =
		AXI_UART_16550_BASE_ADDRESSES;
uint32_t AxiUart16550::clocks_[AXI_UART_16550_NUM_INSTANCES] =
		AXI_UART_16550_CLOCKS;
uint32_t AxiUart16550::interruptIds_[AXI_UART_16550_NUM_INSTANCES] =
		AXI_UART_16550_INTERRUPT_IDS;
uint32_t AxiUart16550::interruptPriorities_[AXI_UART_16550_NUM_INSTANCES] =
		AXI_UART_16550_INTERRUPT_PRIORITIES;



AxiUart16550* AxiUart16550::getAxiUart16550(uint8_t number, uint32_t baudrate)
{
	if(number >= AXI_UART_16550_NUM_INSTANCES)
		return NULL;
	if(axiUart16550s_[number] == (AxiUart16550*)NULL)
		axiUart16550s_[number] = new AxiUart16550(number, baudrate);
	return axiUart16550s_[number];
}



AxiUart16550::AxiUart16550(uint8_t number, uint32_t baudrate) : IVoidChannel(), IIrqListener()
{
	this->number_ = number;
	this->baudrate_ = baudrate;
}



void AxiUart16550::initialize(void* (* const mallocFunc)(size_t),
		const uint16_t txBufferSize, IChannelCallback* const callback)
{
	if(!this->isInitialized_) {
		if(txBufferSize == 0)
			return;
		this->txBufferSize_ = txBufferSize;
		this->txBuffer_ = (uint8_t*)mallocFunc(this->txBufferSize_);
		if(this->txBuffer_ == (uint8_t*)NULL)
			return;
		this->callback_ = callback;
		this->txRingBuffer_ = new RingBuffer(this->txBuffer_, 1,
				this->txBufferSize_);

		XUartNs550_SetBaud(baseAddresses_[this->number_], clocks_[this->number_],
				this->baudrate_);
		XUartNs550_SetLineControlReg(baseAddresses_[this->number_],
				XUN_LCR_8_DATA_BITS);

		IrqController::getIrqController()->addPeripheralIrqListener(this,
				interruptIds_[this->number_]);
		IrqController::getIrqController()->setPriority(interruptIds_[this->number_],
				interruptPriorities_[this->number_]);
		IrqController::getIrqController()->enableInterrupt(interruptIds_[this->number_]);

		XUartNs550_WriteReg(baseAddresses_[this->number_], XUN_IER_OFFSET,
				XUartNs550_ReadReg(baseAddresses_[this->number_], XUN_IER_OFFSET)|
				(XUN_IER_TX_EMPTY | XUN_IER_RX_DATA));
		this->isInitialized_ = true;
	}
}



void AxiUart16550::irqHandler(uint32_t irqNumber)
{
	static u8 isrStatus = 0;
	static uint8_t recievedByte = 0;
	static uint8_t byteToTx = 0;

	if(this->isInitialized_) {
		isrStatus = (u8)XUartNs550_ReadReg(baseAddresses_[this->number_],
				XUN_IIR_OFFSET) &XUN_INT_ID_MASK;
		if(isrStatus == 2) {  // Tramsm Empty
			uint32_t count  = this->txRingBuffer_->getCount();
			if(count) {
				if(this->txRingBuffer_->pop(&byteToTx)) {
					this->isWriteBusy_ = true;
					XUartNs550_SendByte(baseAddresses_[this->number_], byteToTx);
				}
				else
					this->isWriteBusy_ = false;
			}
			else
				this->isWriteBusy_ = false;
		}
		if(isrStatus == 4) { // Recieve data avaliable
			recievedByte = XUartNs550_RecvByte(baseAddresses_[this->number_]);
			if(this->callback_)
				this->callback_->channelCallback(&recievedByte, 1, (void*)this, NULL);
		}
	}
}



void AxiUart16550::deinitialize(void)
{
	if(this->isInitialized_){
		delete this->txRingBuffer_;
		free_s(this->txBuffer_);
		IrqController::getIrqController()->deletePeripheralIrqListener(this);
		this->isInitialized_ = false;
	}
}



AxiUart16550::~AxiUart16550(void)
{
	this->deinitialize();
}



void AxiUart16550::tx(uint8_t* const buffer, const uint16_t size,
		void* parameter)
{
	if(this->isInitialized_) {
		this->txRingBuffer_->insertMult(buffer, size);
		u8 txEmptyStatus = (u8)XUartNs550_ReadReg(baseAddresses_[this->number_],
				XUN_LSR_OFFSET) &0x40; //Check if transmitter empty
		if( (!this->isWriteBusy_) || (this->isWriteBusy_ && (txEmptyStatus != 0))){
			uint8_t byteToTx = 0;
			if(this->txRingBuffer_->pop(&byteToTx)) {
				this->isWriteBusy_ = true;
				XUartNs550_SendByte(baseAddresses_[this->number_], byteToTx);
			}
			else
				this->isWriteBusy_ = false;
		}
	}
}



void AxiUart16550::getParameter(const uint8_t number, void* const value)
{
	if(this->isInitialized_) {
		if(number == PARAMETER_CTS) {
			uint8_t mctrlReg = (u8)XUartNs550_ReadReg(baseAddresses_[this->number_],
							XUN_MSR_OFFSET);
			uint32_t ctsSignal = (mctrlReg & 16) >> 4;
			ctsSignal ^= 1;
			*((uint32_t*)value) = ctsSignal;
		}
		else if(number == PARAMETER_IS_TX_EMPTY) {
			if(this->txRingBuffer_->getCount())
				*((uint32_t*)value) = 0;
			else
				*((uint32_t*)value) = 1;
		}
	}
}



void AxiUart16550::setParameter(const uint8_t number, void* const value)
{
	if(this->isInitialized_) {
		uint8_t mctrlReg = (u8)XUartNs550_ReadReg(baseAddresses_[this->number_],
				XUN_MCR_OFFSET);
		if(number == PARAMETER_CLEAR_RTS) {
			XUartNs550_WriteReg(baseAddresses_[this->number_],  XUN_MCR_OFFSET,
					mctrlReg | XUN_MCR_RTS);
		}
		else if(number == PARAMETER_SET_RTS) {
			XUartNs550_WriteReg(baseAddresses_[this->number_],  XUN_MCR_OFFSET,
					mctrlReg & ( ~XUN_MCR_RTS ));
		}
	}
}


}
}

#endif
