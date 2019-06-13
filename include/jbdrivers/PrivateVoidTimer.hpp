/**
 * @file
 * @brief Void Timer on Private Timer Driver Description
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

#ifndef PRIVATE_VOID_TIMER_HPP_
#define PRIVATE_VOID_TIMER_HPP_

#include "jbkernel/jb_common.h"
#include "jbkernel/IVoidTimer.hpp"
#include "jbdrivers/IrqController.hpp"
#include "xscutimer.h"

namespace jblib
{
namespace jbdrivers
{

using namespace jbkernel;

class PrivateVoidTimer : public IVoidTimer, protected IIrqListener
{
public:
	static PrivateVoidTimer* getPrivateVoidTimer(void);
	virtual void initialize(uint32_t us);
	virtual void start(void);
	virtual void stop(void);
	virtual void reset(void);
	virtual uint32_t getCounter(void);
	virtual void setCounter(uint32_t count);
	virtual void addCallback(IVoidCallback* const callback);
	virtual void deleteCallback(void);
	virtual void deinitialize(void);

private:
	PrivateVoidTimer(void);
	virtual void irqHandler(uint32_t irqNumber);

	static PrivateVoidTimer* privateVoidTimer_;
	XScuTimer xScuTimer_;
	IVoidCallback* callback_ = NULL;
};

}
}

#endif /* PRIVATE_VOID_TIMER_HPP_ */
