/**
 * @file
 * @brief Inter-Processor Communication Driver Realization
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
#if JBDRIVERS_USE_IPC
#include <string.h>
#include "jbdrivers/Ipc.hpp"

#define HEAD_INDEX(q)                ((q)->head & ((q)->size - 1))
#define TAIL_INDEX(q)              	 ((q)->tail & ((q)->size - 1))

namespace jblib
{
namespace jbdrivers
{

using namespace jbkernel;

typedef struct
{
	uint32_t index = 0;
	uint32_t value = 0;
}IpcGlobalValue_t;


Ipc* Ipc::ipcs_[2] = {(Ipc*)NULL, (Ipc*)NULL };



Ipc* Ipc::getIpc(uint8_t gate)
{
	#ifdef CORE_A9_0
		if(gate == GATE_CORE_A9_0)
			return (Ipc*)NULL;
	#endif
	#ifdef CORE_A9_1
		if(gate == GATE_CORE_A9_1)
			return (Ipc*)NULL;
	#endif

	if(ipcs_[gate] == (Ipc*)NULL)
		ipcs_[gate] = new Ipc(gate);
	return ipcs_[gate];
}



Ipc::Ipc(uint8_t gate) : IIpc(), IIrqListener()
{
	#ifdef CORE_A9_0
	this->writeQueue_ = (IpcQueue_t*)IPC_SHARED_MEMORY_A9_0_GATE;
	this->readQueue_ = (IpcQueue_t*)IPC_SHARED_MEMORY_A9_1_GATE;
	this->writeQueue_->data = (uint8_t*)(IPC_SHARED_MEMORY_A9_0_GATE + sizeof(IpcQueue_t));
	#endif
	#ifdef CORE_A9_1
	this->writeQueue_ = (IpcQueue_t*)IPC_SHARED_MEMORY_A9_1_GATE;
	this->readQueue_ = (IpcQueue_t*)IPC_SHARED_MEMORY_A9_0_GATE;
	this->writeQueue_->data = (uint8_t*)(IPC_SHARED_MEMORY_A9_1_GATE + sizeof(IpcQueue_t));
	#endif

	for(uint32_t i = 0; i < IPC_NUM_GLOBAL_VALUES; i++)
		this->globalValues_[i] = 0;

	this->writeQueue_->size = IPC_QUEUE_SIZE;
	this->writeQueue_->itemSize = sizeof(IpcMsg_t);
	this->writeQueue_->valid = IPC_QUEUE_MAGIC_VALID;
	this->writeQueue_->head = 0;
	this->writeQueue_->tail = 0;
	this->writeQueue_->reserved[0] = 0;
	this->writeQueue_->reserved[1] = 0;

	IrqController::getIrqController()->addPeripheralIrqListener(this, IPC_INTERRUPT_ID);
	IrqController::getIrqController()->setPriority(IPC_INTERRUPT_ID,
			IPC_INTERRUPT_PRIORITY);
	IrqController::getIrqController()->enableInterrupt(IPC_INTERRUPT_ID);
}




void Ipc::addIpcListener(IIpcListener* listener)
{
	this->listenersList_.push_front(listener);
}




void Ipc::deleteIpcListener(IIpcListener* listener)
{
	this->listenersDeleteList_.push_front(listener);
}




void Ipc::irqHandler(uint32_t irqNumber)
{
	if (!IPC_QUEUE_IS_VALID(this->readQueue_))
		return;
	for(uint32_t j = 0; j < IPC_QUEUE_SIZE; j++) {
		if (IPC_QUEUE_IS_EMPTY(this->readQueue_))
			return;
		IpcMsg_t msg;
		memcpy(&msg, this->readQueue_->data +
				(TAIL_INDEX(this->readQueue_) * this->readQueue_->itemSize),
				this->readQueue_->itemSize);
		this->readQueue_->tail++;

		if (msg.id < IPC_MSG_ID_MAX) {
			if(msg.id == IPC_MSG_ID_GLOBAL_VALUE_UPDATE) {
				IpcGlobalValue_t* gv = (IpcGlobalValue_t*)msg.data;
				if (gv->index < IPC_NUM_GLOBAL_VALUES)
					this->globalValues_[gv->index] = gv->value;
				this->pushMsg(IPC_MSG_ID_FREE_MEMORY, msg.data);
			}
			else if(msg.id == IPC_MSG_ID_FREE_MEMORY)
				free_s((void *)msg.data);
			for(std::forward_list<IIpcListener*>::iterator it = this->listenersList_.begin();
					it != this->listenersList_.end(); ++it){
				IIpcListener* listener = *it;
				if(((listener->getCode()) >> msg.id) & 1){
					listener->handleIpcMsg(&msg);
				}
			}
			if(!this->listenersDeleteList_.empty()){
				for(std::forward_list<IIpcListener*>::iterator it = this->listenersDeleteList_.begin();
						it != this->listenersDeleteList_.end(); ++it){
					IIpcListener* listener = *it;
					this->listenersList_.remove_if([listener](IIpcListener* item){
						if(listener == item)
							return true;
						else
							 return false;
					});
				}
				this->listenersDeleteList_.clear();
			}
		}
	}
}



int Ipc::pushMsg(uint32_t id, uint32_t data)
{
	disableInterrupts();
	IpcMsg_t msg;
	#ifdef CORE_A9_0
	msg.sender = GATE_CORE_A9_0;
	#endif
	#ifdef CORE_A9_1
	msg.sender = GATE_CORE_A9_1;
	#endif
	msg.id = id;
	msg.data = data;

	if (!IPC_QUEUE_IS_VALID(this->writeQueue_)){
		enableInterrupts();
		return IPC_QUEUE_ERROR;
	}
	if (IPC_QUEUE_IS_FULL(this->writeQueue_)){
		enableInterrupts();
		return IPC_QUEUE_FULL;
	}
	memcpy(this->writeQueue_->data + (HEAD_INDEX(this->writeQueue_) * this->writeQueue_->itemSize),
			&msg, this->writeQueue_->itemSize);
	this->writeQueue_->head++;
	this->sendInterrupt();
	enableInterrupts();
	return IPC_QUEUE_INSERT;
}



int Ipc::getWriteQueueMsgCount(void)
{
	if (!IPC_QUEUE_IS_VALID(this->writeQueue_))
		return IPC_QUEUE_ERROR;
	else
		return IPC_QUEUE_DATA_COUNT(this->writeQueue_);
}



int Ipc::setGlobalValue(uint32_t index, uint32_t value)
{
	if (index >= IPC_NUM_GLOBAL_VALUES)
		return 1;
	IpcGlobalValue_t* gv =
			(IpcGlobalValue_t*)malloc_s(sizeof(IpcGlobalValue_t));
	if (gv == NULL)
		return 1;
	gv->value = this->globalValues_[index] = value;
	gv->index = index;
	if(this->pushMsg(IPC_MSG_ID_GLOBAL_VALUE_UPDATE,
			(uint32_t) gv) != IPC_QUEUE_INSERT) {
		free_s(gv);
		return 1;
	}
	return 0;
}



uint32_t Ipc::getGlobalValue(uint32_t index)
{
	if (index < IPC_NUM_GLOBAL_VALUES)
		return this->globalValues_[index];
	else
		return 0;
}


void Ipc::sendInterrupt(void)
{
	#ifdef CORE_A9_0
	IrqController::getIrqController()->sendSoftwareInterruptToCpu1(IPC_INTERRUPT_ID);
	#endif
	#ifdef CORE_A9_1
	IrqController::getIrqController()->sendSoftwareInterruptToCpu0(IPC_INTERRUPT_ID);
	#endif
}

}
}

#endif
