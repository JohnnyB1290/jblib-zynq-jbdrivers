/**
 * @file
 * @brief xScuWdt_ Driver Realization
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
#if JBDRIVERS_USE_WATCHDOG

#include "jbdrivers/Wdt.hpp"
#include "jbdrivers/JbController.hpp"
#if USE_CONSOLE
#include <stdio.h>
#endif

namespace jblib
{
namespace jbdrivers
{

using namespace jbkernel;

Wdt* Wdt::wdt_ = NULL;



Wdt* Wdt::getWdt(void)
{
	if(wdt_ == NULL)
		wdt_ = new Wdt();
	return wdt_;
}



Wdt::Wdt(void) : IIrqListener(), IVoidCallback()
{

}



void Wdt::initialize(uint16_t reloadTimeS, WdMode_t mode)
{
	if(!this->isInitialized_) {
		XScuWdt_Config* config = XScuWdt_LookupConfig(WDT_DEVICE_ID);
		XScuWdt_CfgInitialize((XScuWdt*)&this->xScuWdt_, config,
				config->BaseAddr);
		this->xScuWdtPtr_ = (XScuWdt*)&this->xScuWdt_;

		this->mode_ = mode;
		if(this->mode_ == MODE_TIMER){
			XScuWdt_SetTimerMode(this->xScuWdtPtr_);
			this->isLastResetWasWdt_ = ((XScuWdt_ReadReg(this->xScuWdt_.Config.BaseAddr,
					XSCUWDT_ISR_OFFSET) & XSCUWDT_ISR_EVENT_FLAG_MASK) ==
							XSCUWDT_ISR_EVENT_FLAG_MASK);
			XScuWdt_WriteReg(this->xScuWdt_.Config.BaseAddr, XSCUWDT_ISR_OFFSET,
							XSCUWDT_ISR_EVENT_FLAG_MASK);
			uint32_t reg = XScuWdt_ReadReg(this->xScuWdt_.Config.BaseAddr,
							XSCUWDT_CONTROL_OFFSET);
			XScuWdt_SetControlReg(this->xScuWdtPtr_, reg |XSCUWDT_CONTROL_IT_ENABLE_MASK);
		}
		else{
			XScuWdt_SetWdMode(this->xScuWdtPtr_);
			this->isLastResetWasWdt_ = XScuWdt_ReadReg(this->xScuWdt_.Config.BaseAddr,
					XSCUWDT_RST_STS_OFFSET) & XSCUWDT_RST_STS_RESET_FLAG_MASK;
			XScuWdt_WriteReg(this->xScuWdt_.Config.BaseAddr, XSCUWDT_RST_STS_OFFSET,
					XSCUWDT_RST_STS_RESET_FLAG_MASK);
		}

		uint32_t load = XPAR_PS7_CORTEXA9_0_CPU_CLK_FREQ_HZ >> 1;
		load = reloadTimeS * load;
		XScuWdt_LoadWdt(this->xScuWdtPtr_, load);

		if(this->mode_ == MODE_TIMER){
			IrqController::getIrqController()->addPeripheralIrqListener(this,
					WDT_INTERRUPT_ID);
			IrqController::getIrqController()->setPriority(WDT_INTERRUPT_ID,
					WDT_INTERRUPT_PRIORITY);
			IrqController::getIrqController()->enableInterrupt(WDT_INTERRUPT_ID);
		}
		this->isInitialized_ = true;
	}
}



void Wdt::start(void)
{
	this->reset();
	#if USE_CONSOLE
	printf("WatchDog Timer start\r\n");
	#endif
	XScuWdt_Start(this->xScuWdtPtr_);
}



void Wdt::stop(void)
{
	#if USE_CONSOLE
	printf("WatchDog Timer stop\r\n");
	#endif
	if(this->mode_ == MODE_WD){
		XScuWdt_SetTimerMode(this->xScuWdtPtr_);
	}
	XScuWdt_Stop(this->xScuWdtPtr_);
}



void Wdt::reset(void)
{
	XScuWdt_RestartWdt(this->xScuWdtPtr_);
}



bool Wdt::isLastResetWasWdt(void)
{
	return this->isLastResetWasWdt_;
}



void Wdt::irqHandler(uint32_t irqNumber)
{
	#if USE_CONSOLE
	printf("WatchDog Timer Expired!\r\n");
	#endif
	XScuWdt_Stop(this->xScuWdtPtr_);
	IrqController::getIrqController()->disableInterrupt(WDT_INTERRUPT_ID);
	JbController::goToApp(WDT_RESET_ADDRESS);
}



void Wdt::voidCallback(void* const source, void* parameter)
{
	this->reset();
}

}
}

#endif
