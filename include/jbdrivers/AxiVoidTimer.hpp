/**
 * @file
 * @brief Void Timer on AXI Timer Driver Description
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

#ifndef AXI_VOID_TIMER_HPP_
#define AXI_VOID_TIMER_HPP_

#include "jbkernel/jb_common.h"
#include "jbkernel/IVoidTimer.hpp"
#include "jbdrivers/IrqController.hpp"
#include "xtmrctr.h"

namespace jblib
{
namespace jbdrivers
{

using namespace jbkernel;


class AxiVoidTimer : public IVoidTimer, protected IIrqListener
{
public:
	static AxiVoidTimer* getAxiVoidTimer(uint8_t number);
	virtual void initialize(uint32_t us);
	virtual void start(void);
	virtual void stop(void);
	virtual void reset(void);
	virtual uint32_t getCounter(void);
	virtual uint32_t getUsecCounter(void);
	virtual void setCounter(uint32_t count);
	virtual void addCallback(IVoidCallback* const callback);
	virtual void deleteCallback(void);
	virtual void deinitialize(void);

private:
	static void timerHandler(void* callbackRef, u8 subTimerNumber);
	AxiVoidTimer(uint8_t number);
	virtual void irqHandler(uint32_t irqNumber);

	static AxiVoidTimer* axiVoidTimers_[AXI_TIMER_NUM_TIMERS];
	static uint32_t interruptIds_[AXI_TIMER_NUM_TIMERS];
	static uint32_t interruptPriorities_[AXI_TIMER_NUM_TIMERS];
	static uint32_t deviceIds_[AXI_TIMER_NUM_TIMERS];
	static uint32_t clocks_[AXI_TIMER_NUM_TIMERS];

	XTmrCtr xTmrCtr_;
	IVoidCallback* callback_ = NULL;
	uint8_t number_ = 0;
	uint32_t uSecTicks_ = 0;
	uint32_t compareValue_ = 0;
};

}
}

#endif /* AXI_VOID_TIMER_HPP_ */
