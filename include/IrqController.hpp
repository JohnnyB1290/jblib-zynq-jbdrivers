/**
 * @file
 * @brief IRQ Controller class definition
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

#ifndef IRQ_CONTROLLER_HPP_
#define IRQ_CONTROLLER_HPP_

#include "jb_common.h"
#include "xil_exception.h"
#include "xscugic.h"

namespace jblib
{
namespace jbdrivers
{

class IIrqListener
{
public:
    IIrqListener(void){}
    virtual ~IIrqListener(void){}
    virtual void irqHandler(uint32_t irqNumber) = 0;
};



class IrqController
{
public:
	static IrqController* getIrqController(void);
	void enableInterrupt(u32 interruptId);
	void disableInterrupt(u32 interruptId);
	void setPriority(u32 interruptId, u8 priority, u8 trigger);
	void setPriority(u32 interruptId, u8 priority);
    void addPeripheralIrqListener(IIrqListener* listener, uint32_t irqNumber);
    void deletePeripheralIrqListener(IIrqListener* listener);
    void sendSoftwareInterruptToCpu0(uint32_t interruptId);
    void sendSoftwareInterruptToCpu1(uint32_t interruptId);

private:
    static void irqHandler(void* irqNumber);
    static void undefinedInterruptHandler(void* data);
    static void swiInterruptHandler(void* data);
    static void prefetchAbortInterruptHandler(void* data);
    static void dataAbortInterruptHandler(void* data);
    static void fiqInterruptHandler(void* data);
    IrqController(void);

    static IrqController* irqController_;
    IIrqListener* irqListeners_[IRQ_CONTROLLER_NUM_PERIPHERAL_LISTENERS];
    uint32_t irqListenersInterruptIds_[IRQ_CONTROLLER_NUM_PERIPHERAL_LISTENERS];
    XScuGic xScuGic_;
    XScuGic_Config* xScuGicConfig_ = NULL;
};

}
}

#endif /* IRQ_CONTROLLER_HPP_ */
