/**
 * @file
 * @brief IRQ Controller class realization
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

#include "jbdrivers/IrqController.hpp"
#ifdef USE_CONSOLE
#include <stdio.h>
#endif

namespace jblib
{
namespace jbdrivers
{

IrqController* IrqController::irqController_ = (IrqController*)NULL;



IrqController* IrqController::getIrqController(void)
{
	if(irqController_ == (IrqController*)NULL)
		irqController_ = new IrqController();
	return irqController_;
}



IrqController::IrqController(void)
{
	for(uint32_t i = 0; i < IRQ_CONTROLLER_NUM_PERIPHERAL_LISTENERS; i++) {
		this->irqListeners_[i] = (IIrqListener*)NULL;
		this->irqListenersInterruptIds_[i] = 0;
	}
	for(uint32_t i = 0; i < XSCUGIC_MAX_NUM_INTR_INPUTS; i++)
		XScuGic_WriteReg(XPAR_SCUGIC_0_CPU_BASEADDR, XSCUGIC_EOI_OFFSET, i);

	this->xScuGicConfig_ = XScuGic_LookupConfig(IRQ_CONTROLLER_DEVICE_ID);
	this->xScuGic_.Config = this->xScuGicConfig_;
	XScuGic_CfgInitialize(&this->xScuGic_, this->xScuGicConfig_,
			this->xScuGicConfig_->CpuBaseAddress);
	XScuGic_CPUWriteReg(&this->xScuGic_, XSCUGIC_BIN_PT_OFFSET,0x03);

	Xil_ExceptionInit();
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
			(Xil_ExceptionHandler) XScuGic_InterruptHandler, &this->xScuGic_);

	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_UNDEFINED_INT,
			(Xil_ExceptionHandler) undefinedInterruptHandler,
			&this->xScuGic_);

	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_SWI_INT,
			(Xil_ExceptionHandler) swiInterruptHandler,
			&this->xScuGic_);

	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_PREFETCH_ABORT_INT,
			(Xil_ExceptionHandler) prefetchAbortInterruptHandler,
			&this->xScuGic_);

	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_DATA_ABORT_INT,
				(Xil_ExceptionHandler) dataAbortInterruptHandler,
				&this->xScuGic_);

	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_FIQ_INT,
				(Xil_ExceptionHandler) fiqInterruptHandler,
				&this->xScuGic_);
	Xil_ExceptionEnable();
}



void IrqController::enableInterrupt(u32 interruptId)
{
	XScuGic_Connect(&this->xScuGic_, interruptId,
			(Xil_InterruptHandler)irqHandler, (void*)interruptId);
	XScuGic_Enable(&this->xScuGic_, interruptId);
}



void IrqController::disableInterrupt(u32 interruptId)
{
	XScuGic_Disable(&this->xScuGic_, interruptId);
	XScuGic_Disconnect(&this->xScuGic_, interruptId);
}



void IrqController::setPriority(u32 interruptId, u8 priority, u8 trigger)
{
	XScuGic_SetPriorityTriggerType(&this->xScuGic_, interruptId, priority, trigger);
}



void IrqController::setPriority(u32 interruptId, u8 priority)
{
	u8 getPriority = 0;
	u8 getTrigger = 0;
	XScuGic_GetPriorityTriggerType(&xScuGic_, interruptId, &getPriority,
			&getPriority);
	XScuGic_SetPriorityTriggerType(&xScuGic_,interruptId, priority, getTrigger);
}



void IrqController::addPeripheralIrqListener(IIrqListener* listener,
		uint32_t irqNumber)
{
	Xil_ExceptionDisable();
	for(uint32_t i = 0; i < IRQ_CONTROLLER_NUM_PERIPHERAL_LISTENERS; i++) {
		if(this->irqListeners_[i] == (IIrqListener*)NULL) {
			this->irqListeners_[i] = listener;
			this->irqListenersInterruptIds_[i] = irqNumber;
			break;
		}
	}
	Xil_ExceptionEnable();
}



void IrqController::deletePeripheralIrqListener(IIrqListener* listener)
{
	Xil_ExceptionDisable();
	uint32_t index = 0;
	for(uint32_t i = 0; i < IRQ_CONTROLLER_NUM_PERIPHERAL_LISTENERS; i++) {
		if(this->irqListeners_[i] == listener)
			break;
		else
			index++;
	}
	if(index == (IRQ_CONTROLLER_NUM_PERIPHERAL_LISTENERS - 1)) {
		if(this->irqListeners_[index] == listener) {
			this->irqListeners_[index] = (IIrqListener*)NULL;
			this->irqListenersInterruptIds_[index] = 0;
		}
	}
	else {
		for(uint32_t i = index; i < (IRQ_CONTROLLER_NUM_PERIPHERAL_LISTENERS - 1); i++) {
			this->irqListeners_[i] = this->irqListeners_[i+1];
			this->irqListenersInterruptIds_[i] =
					this->irqListenersInterruptIds_[i+1];
			if(this->irqListeners_[i+1] == (IIrqListener*)NULL)
				break;
		}
	}
	Xil_ExceptionEnable();
	if(index < (IRQ_CONTROLLER_NUM_PERIPHERAL_LISTENERS-1))
		this->deletePeripheralIrqListener(listener);
}



void IrqController::irqHandler(void* irqNumber)
{
	Xil_EnableNestedInterrupts();
	uint32_t irqNumberUint = (uint32_t)irqNumber;
	for(uint32_t i = 0; i < IRQ_CONTROLLER_NUM_PERIPHERAL_LISTENERS; i++) {
		if(getIrqController()->irqListeners_[i]) {
			if(getIrqController()->irqListenersInterruptIds_[i] == irqNumberUint)
				getIrqController()->irqListeners_[i]->irqHandler(irqNumberUint);
		}
		else
			break;
	}
	Xil_DisableNestedInterrupts();
}



void IrqController::sendSoftwareInterruptToCpu0(uint32_t interruptId)
{
	XScuGic_SoftwareIntr(&this->xScuGic_, interruptId, XSCUGIC_SPI_CPU0_MASK);
}



void IrqController::sendSoftwareInterruptToCpu1(uint32_t interruptId)
{
	XScuGic_SoftwareIntr(&this->xScuGic_, interruptId, XSCUGIC_SPI_CPU1_MASK);
}



void IrqController::undefinedInterruptHandler(void* data)
{
	#ifdef USE_CONSOLE
	printf("IRQ Controller: UNDEFINED INTERRUPT!\r\n");
	#endif
	while(1);
}



void IrqController::swiInterruptHandler(void* data)
{
	#ifdef USE_CONSOLE
	printf("IRQ Controller: SWI INTERRUPT!\r\n");
	#endif
	while(1);
}



void IrqController::prefetchAbortInterruptHandler(void* data)
{
	#ifdef USE_CONSOLE
	printf("IRQ Controller: PREFETCH ABORT INTERRUPT!\r\n");
	#endif
	while(1);
}



void IrqController::dataAbortInterruptHandler(void* data)
{
	#ifdef USE_CONSOLE
	printf("IRQ Controller: DATA ABORT INTERRUPT!\r\n");
	#endif
	while(1);
}



void IrqController::fiqInterruptHandler(void* data)
{
	#ifdef USE_CONSOLE
	printf("IRQ Controller: FIQ INTERRUPT!\r\n");
	#endif
	while(1);
}


}
}
