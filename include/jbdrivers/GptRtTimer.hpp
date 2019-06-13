/**
 * @file
 * @brief RT Timer on GPTimer class Description
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


#ifndef GPT_RT_TIMER_HPP_
#define GPT_RT_TIMER_HPP_

#include "jbkernel/jb_common.h"
#include "jbdrivers/IrqController.hpp"
#include "jbkernel/IRtTimer.hpp"

#define GPT_NUM_MATCHES 	24

namespace jblib
{
namespace jbdrivers
{

using namespace jbkernel;


class GptRtTimer : public IRtTimer, protected IIrqListener
{
public:
	static GptRtTimer* getGptRtTimer(uint8_t number);
	virtual void initialize(void);
	virtual void start(void);
	virtual void stop(void);
	virtual void reset(void);
	virtual void addMatchUs(const uint8_t matchNumber, uint32_t us,
			bool isResetOnMatch);
	virtual void addMatchTicks(const uint8_t matchNumber,
			uint32_t ticks, bool isResetOnMatch);
	virtual void deleteMatch(const uint8_t matchNumber);
	virtual void deinitialize(void);
	virtual void setMatchCallback(const uint8_t matchNumber,
			IVoidCallback* const matchCallback);
	virtual void deleteMatchCallback(const uint8_t matchNumber);
	virtual uint32_t getCounter(void);
	virtual void setCounter(uint32_t count);

private:
	GptRtTimer(uint8_t number);
	virtual void irqHandler(uint32_t irqNumber);

	static GptRtTimer* gptRtTimers_[GPT_NUM_INSTANCES];
	static uint32_t baseAddresses_[GPT_NUM_INSTANCES];
	static uint32_t interruptIds_[GPT_NUM_INSTANCES];
	static uint32_t interruptPriorities_[GPT_NUM_INSTANCES];
	static uint32_t clocks_[GPT_NUM_INSTANCES];
	uint8_t number_ = 0;
	IVoidCallback* matchCallbacks_[GPT_NUM_MATCHES];
};

}
}

#endif /* GPT_RT_TIMER_HPP_ */
