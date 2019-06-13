/**
 * @file
 * @brief Void Timer on AXI Timer Driver Realization
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
#if JBDRIVERS_USE_AXI_TIMER

#include "jbdrivers/AxiVoidTimer.hpp"

namespace jblib
{
namespace jbdrivers
{

using namespace jbkernel;


AxiVoidTimer* AxiVoidTimer::axiVoidTimers_[AXI_TIMER_NUM_TIMERS] = {NULL};
uint32_t AxiVoidTimer::interruptIds_[AXI_TIMER_NUM_TIMERS] = AXI_TIMER_INTERRUPT_IDS;
uint32_t AxiVoidTimer::interruptPriorities_[AXI_TIMER_NUM_TIMERS] = AXI_TIMER_INTERRUPT_PRIORITIES;
uint32_t AxiVoidTimer::deviceIds_[AXI_TIMER_NUM_TIMERS] = AXI_TIMER_DEVICE_IDS;
uint32_t AxiVoidTimer::clocks_[AXI_TIMER_NUM_TIMERS] = AXI_TIMER_CLOCKS;



AxiVoidTimer* AxiVoidTimer::getAxiVoidTimer(uint8_t number)
{
	if(number >= AXI_TIMER_NUM_TIMERS)
		return NULL;
	if(axiVoidTimers_[number] == (AxiVoidTimer*)NULL)
		axiVoidTimers_[number] = new AxiVoidTimer(number);
	return axiVoidTimers_[number];
}



AxiVoidTimer::AxiVoidTimer(uint8_t number) : IVoidTimer(), IIrqListener()
{
	this->number_ = number;
	this->uSecTicks_ = clocks_[this->number_] / 1000000;
	XTmrCtr_Initialize(&this->xTmrCtr_, deviceIds_[this->number_]);
	IrqController::getIrqController()->addPeripheralIrqListener(this,
			interruptIds_[this->number_]);
	IrqController::getIrqController()->setPriority(interruptIds_[this->number_],
			interruptPriorities_[this->number_]);
	IrqController::getIrqController()->enableInterrupt(interruptIds_[this->number_]);
}



void AxiVoidTimer::timerHandler(void* callbackRef, u8 subTimerNumber)
{
	AxiVoidTimer* axiVoidTimer = (AxiVoidTimer*)callbackRef;
	if((subTimerNumber == 0) && axiVoidTimer->callback_)
		axiVoidTimer->callback_->voidCallback(callbackRef, NULL);
}



void AxiVoidTimer::initialize(uint32_t us)
{
	XTmrCtr_SetHandler(&this->xTmrCtr_, timerHandler, this);
	XTmrCtr_SetOptions(&this->xTmrCtr_, 0,
			XTC_INT_MODE_OPTION | XTC_AUTO_RELOAD_OPTION|XTC_DOWN_COUNT_OPTION);
	this->compareValue_ = this->uSecTicks_ * us;
	XTmrCtr_SetResetValue(&this->xTmrCtr_, 0, this->compareValue_- 2);
}



void AxiVoidTimer::start(void)
{
	XTmrCtr_Start(&this->xTmrCtr_, 0);
}



void AxiVoidTimer::stop(void)
{
	XTmrCtr_Stop(&this->xTmrCtr_, 0);
}



void AxiVoidTimer::reset(void)
{
	XTmrCtr_Reset(&this->xTmrCtr_, 0);
}



uint32_t AxiVoidTimer::getCounter(void)
{
	return XTmrCtr_GetValue(&this->xTmrCtr_, 0);
}



uint32_t AxiVoidTimer::getUsecCounter(void)
{
	uint32_t value = XTmrCtr_GetValue(&this->xTmrCtr_, 0);
	return (this->compareValue_ - value) / this->uSecTicks_;
}



void AxiVoidTimer::setCounter(uint32_t count)
{
	XTmrCtr_WriteReg(this->xTmrCtr_.Config.BaseAddress, 0, XTC_TCR_OFFSET, count);
}



void AxiVoidTimer::addCallback(IVoidCallback* const callback)
{
	if(this->callback_ == (IVoidCallback*)NULL)
		this->callback_ = callback;
}



void AxiVoidTimer::deleteCallback(void)
{
	this->callback_ = (IVoidCallback*)NULL;
}



void AxiVoidTimer::deinitialize(void)
{
	this->stop();
	IrqController::getIrqController()->deletePeripheralIrqListener(this);
	this->callback_ = (IVoidCallback*)NULL;
}



void AxiVoidTimer::irqHandler(uint32_t irqNumber)
{
	XTmrCtr_InterruptHandler(&this->xTmrCtr_);
}

}
}

#endif
