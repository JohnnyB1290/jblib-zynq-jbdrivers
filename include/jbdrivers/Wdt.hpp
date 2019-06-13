/**
 * @file
 * @brief xScuWdt_ Driver Description
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

#ifndef WDT_HPP_
#define WDT_HPP_

#include "jb_common.h"
#include "callback_interfaces.hpp"
#include "IrqController.hpp"
#include "xscuwdt.h"

namespace jblib
{
namespace jbdrivers
{

using namespace jbkernel;

class Wdt :protected IIrqListener, public IVoidCallback
{
public:
	static Wdt* getWdt(void);
	void initialize(uint16_t reloadTimeS);
	void start(void);
	void stop(void);
	void reset(void);
	bool isLastResetWasWdt(void);
	virtual void voidCallback(void* const source, void* parameter);

private:
	Wdt(void);
	virtual void irqHandler(uint32_t irqNumber);

	static Wdt* wdt_;
	XScuWdt xScuWdt_;		/* Cortex SCU Private WatchDog Timer Instance */
	XScuWdt* xScuWdtPtr_ = NULL;
	bool isLastResetWasWdt_ = false;
	bool isInitialized_ = false;
};

}
}

#endif /* WDT_HPP_ */
