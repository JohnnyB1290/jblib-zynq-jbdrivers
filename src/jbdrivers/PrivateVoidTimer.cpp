/**
 * @file
 * @brief Void Timer on Private Timer Driver Realization
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

#include "jb_common.h"
#if JBDRIVERS_USE_PRIVATE_TIMER

#include "PrivateVoidTimer.hpp"

namespace jblib
{
namespace jbdrivers
{

using namespace jbkernel;

PrivateVoidTimer* PrivateVoidTimer::privateVoidTimer_ = (PrivateVoidTimer*)NULL;



PrivateVoidTimer* PrivateVoidTimer::getPrivateVoidTimer(void)
{
	if(privateVoidTimer_ == (PrivateVoidTimer*)NULL)
		privateVoidTimer_ = new PrivateVoidTimer();
	return privateVoidTimer_;
}



PrivateVoidTimer::PrivateVoidTimer(void) : IVoidTimer(), IIrqListener()
{
	this->callback_ = (IVoidCallback*)NULL;
}



void PrivateVoidTimer::irqHandler(uint32_t irqNumber)
{
	if(XScuTimer_IsExpired(&this->xScuTimer_)) {
		if(this->callback_)
			this->callback_->voidCallback((void*)this, NULL);
		XScuTimer_ClearInterruptStatus(&this->xScuTimer_);
	}
}



void PrivateVoidTimer::initialize(uint32_t us)
{
	XScuTimer_Config* config = XScuTimer_LookupConfig(PRIVATE_TIMER_DEVICE_ID);
	XScuTimer_CfgInitialize(&this->xScuTimer_, config, config->BaseAddr);

	XScuTimer_EnableAutoReload(&this->xScuTimer_);
	uint32_t compareValue = (XPAR_CPU_CORTEXA9_0_CPU_CLK_FREQ_HZ / 2000000) * us;
	XScuTimer_LoadTimer(&this->xScuTimer_, compareValue);
	XScuTimer_EnableInterrupt(&this->xScuTimer_);

	IrqController::getIrqController()->addPeripheralIrqListener(this,
			PRIVATE_TIMER_INTERRUPT_ID);
	IrqController::getIrqController()->setPriority(PRIVATE_TIMER_INTERRUPT_ID,
			PRIVATE_TIMER_INTERRUPT_PRIORITY);
	IrqController::getIrqController()->enableInterrupt(PRIVATE_TIMER_INTERRUPT_ID);
}



void PrivateVoidTimer::start(void)
{
	XScuTimer_Start(&this->xScuTimer_);
}



void PrivateVoidTimer::stop(void)
{
	XScuTimer_Stop(&this->xScuTimer_);
}



void PrivateVoidTimer::reset(void)
{
	XScuTimer_RestartTimer(&this->xScuTimer_);
}



uint32_t PrivateVoidTimer::getCounter(void)
{
	return XScuTimer_GetCounterValue(&this->xScuTimer_);
}



void PrivateVoidTimer::setCounter(uint32_t count)
{
	XScuTimer_WriteReg(this->xScuTimer_.Config.BaseAddr,
			XSCUTIMER_COUNTER_OFFSET, count);
}



void PrivateVoidTimer::addCallback(IVoidCallback* const callback)
{
	if(this->callback_ == (IVoidCallback*)NULL)
		this->callback_ = callback;
}



void PrivateVoidTimer::deleteCallback(void)
{
	this->callback_ = (IVoidCallback*)NULL;
}



void PrivateVoidTimer::deinitialize(void)
{
	this->stop();
	IrqController::getIrqController()->disableInterrupt(PRIVATE_TIMER_INTERRUPT_ID);
	IrqController::getIrqController()->deletePeripheralIrqListener(this);
	this->callback_ = (IVoidCallback*)NULL;
}

}
}

#endif

