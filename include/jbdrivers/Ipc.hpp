/**
 * @file
 * @brief Inter-Processor Communication Driver Description
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

#ifndef IPC_HPP_
#define IPC_HPP_

#include "jb_common.h"
#include "callback_interfaces.hpp"
#include "IrqController.hpp"
#include "IIpc.hpp"

namespace jblib
{
namespace jbdrivers
{

using namespace jbkernel;

typedef enum
{
	GATE_CORE_A9_0 = 0,
	GATE_CORE_A9_1 = 1,
}IpcGate_t;

class Ipc : public IIPC, protected IIrqListener
{
public:
	static Ipc* getIpc(uint8_t gate);
	virtual void addIpcListener(IIpcListener* listener);
    virtual void deleteIpcListener(IIpcListener* listener);
    virtual int pushMsg(uint32_t id, uint32_t data);
    virtual int getWriteQueueMsgCount(void);
    virtual int setGlobalValue(uint32_t index, uint32_t value);
    virtual uint32_t getGlobalValue(uint32_t index);

private:
	Ipc(uint8_t gate);
	virtual void irqHandler(uint32_t irqNumber);
	void sendInterrupt(void);

	static Ipc* ipcs_[2];
    IpcQueue_t* readQueue_ = NULL;
    IpcQueue_t* writeQueue_ = NULL;
    IpcMsg_t writeQueueData_[IPC_QUEUE_SIZE];
    uint32_t globalValues_[IPC_NUM_GLOBAL_VALUES];
    IIpcListener* listeners_[IPC_NUM_LISTENERS];
};

}
}

#endif /* IPC_HPP_ */
