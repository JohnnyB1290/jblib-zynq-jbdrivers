/**
 * @file
 * @brief RT Timer on GPTimer class Realization
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
#if JBDRIVERS_USE_GPT
#include "jbdrivers/GptRtTimer.hpp"

#define TCSR_REG_OFFSET				0x00
#define TLR_REG_OFFSET				0x04
#define TCR_REG_OFFSET				0x08
#define TER_REG_OFFSET				0x0C
#define TRR_REG_OFFSET				0x10
#define TRST_REG_OFFSET				0x14
#define TLST_REG_OFFSET				0x18
#define TPER_REG_OFFSET				0x1C
#define TADD_REG_OFFSET				0x20
#define TEM_REG_OFFSET				0x24
#define MATCH0_REG_OFFSET			0x80
#define CAPT0_REG_OFFSET			0xE0

#define XTC_CSR_CASC_MASK			0x00000800 /**< Cascade Mode */
#define XTC_CSR_ENABLE_ALL_MASK		0x00000400 /**< Enables all timer counters */
#define XTC_CSR_ENABLE_PWM_MASK		0x00000200 /**< Enables the Pulse Width Modulation */
#define XTC_CSR_INT_OCCURED_MASK	0x00000100 /**< If bit is set, an interrupt has occured. If set and '1' is written to this bit position,
													bit is cleared. */
#define XTC_CSR_ENABLE_TMR_MASK		0x00000080 /**< Enables only the specific timer */
#define XTC_CSR_ENABLE_INT_MASK		0x00000040 /**< Enables the interrupt output. */
#define XTC_CSR_LOAD_MASK			0x00000020 /**< Loads the timer using the load value provided earlier in the Load Register, XTC_TLR_OFFSET. */
#define XTC_CSR_AUTO_RELOAD_MASK	0x00000010 /**< In compare mode, configures the timer counter to reload  from the Load Register. The default  mode
													causes the timer counter to hold when the compare value is hit. In capture mode, configures  the timer counter to not
													hold the previous capture value if a new event occurs. The default mode cause the timer counter to hold the capture value
													until recognized. */
#define XTC_CSR_EXT_CAPTURE_MASK	0x00000008 /**< Enables the external input to the timer counter. */
#define XTC_CSR_EXT_GENERATE_MASK	0x00000004 /**< Enables the external generate output for the timer. */
#define XTC_CSR_DOWN_COUNT_MASK		0x00000002 /**< Configures the timer counter to count down from start value, the default is to count up.*/
#define XTC_CSR_CAPTURE_MODE_MASK	0x00000001 /**< Enables the timer to capture the timer counter value when the external capture line is asserted.
													The default mode is compare mode.*/

namespace jblib
{
namespace jbdrivers
{

using namespace jbkernel;


GptRtTimer* GptRtTimer::gptRtTimers_[GPT_NUM_INSTANCES] = {NULL};
uint32_t GptRtTimer::baseAddresses_[GPT_NUM_INSTANCES] = GPT_BASE_ADDRESSES;
uint32_t GptRtTimer::interruptIds_[GPT_NUM_INSTANCES] = GPT_INTERRUPT_IDS;
uint32_t GptRtTimer::interruptPriorities_[GPT_NUM_INSTANCES] =
		GPT_INTERRUPT_PRIORITIES;
uint32_t GptRtTimer::clocks_[GPT_NUM_INSTANCES] = GPT_CLOCKS;



GptRtTimer* GptRtTimer::getGptRtTimer(uint8_t number)
{
	if(number < GPT_NUM_INSTANCES){
		if(gptRtTimers_[number] == (GptRtTimer*)NULL)
			gptRtTimers_[number] = new GptRtTimer(number);
		return gptRtTimers_[number];
	}
	else
		return (GptRtTimer*)NULL;
}



GptRtTimer::GptRtTimer(uint8_t number) : IRtTimer(), IIrqListener()
{
	this->number_ = number;
	for(uint32_t i = 0; i < GPT_NUM_MATCHES; i++)
		this->matchCallbacks_[i] = (IVoidCallback*)NULL;
	IrqController::getIrqController()->addPeripheralIrqListener(this,
			interruptIds_[this->number_]);
}



void GptRtTimer::irqHandler(uint32_t irqNumber)
{
	static uint8_t matchNum = 0;

	uint32_t reg = 0;
	for(uint32_t i = 0; i < GPT_NUM_MATCHES; i++) {
		matchNum = i;
		reg  = Xil_In32(baseAddresses_[this->number_] + TRR_REG_OFFSET);
		if(reg & (1 << matchNum)) {
			Xil_Out32(baseAddresses_[this->number_] + TRST_REG_OFFSET, (1 << matchNum));
			if(this->matchCallbacks_[matchNum])
				this->matchCallbacks_[matchNum]->voidCallback((void*)this,
						(void*)&matchNum);
		}
	}
	reg = Xil_In32(baseAddresses_[this->number_] + TCSR_REG_OFFSET);
	reg |= XTC_CSR_INT_OCCURED_MASK;
	Xil_Out32(baseAddresses_[this->number_] + TCSR_REG_OFFSET, reg);
}



void GptRtTimer::initialize(void)
{
	Xil_Out32(baseAddresses_[this->number_] + TCSR_REG_OFFSET,
			XTC_CSR_ENABLE_INT_MASK | XTC_CSR_AUTO_RELOAD_MASK);
	Xil_Out32(baseAddresses_[this->number_] + TPER_REG_OFFSET, 0xFFFFFFFF);
	IrqController::getIrqController()->setPriority(interruptIds_[this->number_],
			interruptPriorities_[this->number_]);
	IrqController::getIrqController()->enableInterrupt(interruptIds_[this->number_]);
}



void GptRtTimer::start(void)
{
	uint32_t reg = Xil_In32(baseAddresses_[this->number_] + TCSR_REG_OFFSET);
	reg |= XTC_CSR_ENABLE_TMR_MASK;
	Xil_Out32(baseAddresses_[this->number_] + TCSR_REG_OFFSET, reg);
}



void GptRtTimer::stop(void)
{
	uint32_t reg = Xil_In32(baseAddresses_[this->number_] + TCSR_REG_OFFSET);
	reg &= ~XTC_CSR_ENABLE_TMR_MASK;
	Xil_Out32(baseAddresses_[this->number_] + TCSR_REG_OFFSET, reg);
}



void GptRtTimer::reset(void)
{
	this->setCounter(0);
}



void GptRtTimer::addMatchUs(const uint8_t matchNumber, uint32_t us,
		bool isResetOnMatch)
{
	uint32_t ticks = (clocks_[this->number_] / 1000000) * us;
	this->addMatchTicks(matchNumber, ticks, isResetOnMatch);
}



void GptRtTimer::addMatchTicks(const uint8_t matchNumber,
		uint32_t ticks, bool isResetOnMatch)
{
	if(matchNumber < GPT_NUM_MATCHES){
		Xil_Out32(baseAddresses_[this->number_] + MATCH0_REG_OFFSET + matchNumber * 4,
				ticks);
		uint32_t reg = Xil_In32(baseAddresses_[this->number_] + TER_REG_OFFSET);
		reg |= (1 << matchNumber);
		Xil_Out32(baseAddresses_[this->number_] + TER_REG_OFFSET, reg);
	}
}



void GptRtTimer::deleteMatch(const uint8_t matchNumber)
{
	uint32_t reg = Xil_In32(baseAddresses_[this->number_] + TER_REG_OFFSET);
	reg &= ~(1 << matchNumber);
	Xil_Out32(baseAddresses_[this->number_] + TER_REG_OFFSET, reg);
}



void GptRtTimer::deinitialize(void)
{
	this->stop();
	this->reset();
	uint32_t reg = Xil_In32(baseAddresses_[this->number_] + TCSR_REG_OFFSET);
	reg |= XTC_CSR_INT_OCCURED_MASK;
	Xil_Out32(baseAddresses_[this->number_] + TCSR_REG_OFFSET, reg);

	Xil_Out32(baseAddresses_[this->number_] + TCSR_REG_OFFSET, 0);
	Xil_Out32(baseAddresses_[this->number_] + TRST_REG_OFFSET, 0xFFFFFFFF);
	Xil_Out32(baseAddresses_[this->number_] + TER_REG_OFFSET, 0);

	IrqController::getIrqController()->disableInterrupt(interruptIds_[this->number_]);
	for(uint32_t i = 0; i < GPT_NUM_MATCHES; i++)
		this->matchCallbacks_[i] = NULL;
}



void GptRtTimer::setMatchCallback(const uint8_t matchNumber,
		IVoidCallback* const matchCallback)
{
	this->matchCallbacks_[matchNumber] = matchCallback;
}



void GptRtTimer::deleteMatchCallback(const uint8_t matchNumber)
{
	this->matchCallbacks_[matchNumber] = NULL;
}



uint32_t GptRtTimer::getCounter(void)
{
	return Xil_In32(baseAddresses_[this->number_] + TCR_REG_OFFSET);
}



void GptRtTimer::setCounter(uint32_t count)
{
	Xil_Out32(baseAddresses_[this->number_] + TLR_REG_OFFSET, count);
	uint32_t reg = Xil_In32(baseAddresses_[this->number_] + TCSR_REG_OFFSET);
	reg |= XTC_CSR_LOAD_MASK;
	Xil_Out32(baseAddresses_[this->number_] + TCSR_REG_OFFSET, reg);
	reg &= ~XTC_CSR_LOAD_MASK;
	Xil_Out32(baseAddresses_[this->number_] + TCSR_REG_OFFSET, reg);
}

}
}

#endif
