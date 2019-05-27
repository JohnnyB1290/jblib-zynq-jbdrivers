/*
 * IPC.cpp
 *
 *  Created on: 07.09.2017
 *      Author: Stalker1290
 */

#include "IPC.hpp"
#include "string.h"
#include "CONTROLLER.hpp"
#include "xil_cache_l.h"

typedef struct ipc_gbl_val_struct
{
	int index;
	uint32_t val;
}ipc_gbl_val_t;

IPC_proto_t* IPC_proto_t::IPC_proto_ptr[2] = {(IPC_proto_t*)NULL, (IPC_proto_t*)NULL};

IPC_proto_t* IPC_proto_t::get_IPC_proto(uint8_t gate)
{
	#ifdef CORE_A9_0
		if(gate == CORE_A9_0_GATE) return (IPC_proto_t*)NULL;
	#endif
	#ifdef CORE_A9_1
		if(gate == CORE_A9_1_GATE) return (IPC_proto_t*)NULL;
	#endif

	if(IPC_proto_t::IPC_proto_ptr[gate] == (IPC_proto_t*)NULL) IPC_proto_t::IPC_proto_ptr[gate] = new IPC_proto_t(gate);
	return IPC_proto_t::IPC_proto_ptr[gate];
}

IPC_proto_t::IPC_proto_t(uint8_t gate):IRQ_LISTENER_t()
{
#ifdef CORE_A9_0
	this->qwr = (ipc_queue_t*)IPC_SHARED_MEM_CORE_A9_0_ADDR;
	this->qrd = (ipc_queue_t*)IPC_SHARED_MEM_CORE_A9_1_ADDR;
	this->qwr->data = (uint8_t*)(IPC_SHARED_MEM_CORE_A9_0_ADDR + sizeof(ipc_queue_t));
#endif

#ifdef CORE_A9_1
	this->qwr = (ipc_queue_t*)IPC_SHARED_MEM_CORE_A9_1_ADDR;
	this->qrd = (ipc_queue_t*)IPC_SHARED_MEM_CORE_A9_0_ADDR;
	this->qwr->data = (uint8_t*)(IPC_SHARED_MEM_CORE_A9_1_ADDR + sizeof(ipc_queue_t));
#endif

	for(int i=0; i<IPC_Listeners_num; i++) this->ipcex_listeners[i] = (IPC_listener_t*)NULL;
	for(int i=0; i<IPC_MAX_GBLVAL; i++) this->gblval[i] = 0;
	/* Check if size is a power of 2 and >0*/
	if ((IPC_QUEUE_SZ & (IPC_QUEUE_SZ - 1))|| IPC_QUEUE_SZ == 0) while (1); /* BUG: Size must always be power of 2 */

	memset(this->qwr, 0, sizeof(*this->qwr));
	this->qwr->count = IPC_QUEUE_SZ;
	this->qwr->size = sizeof(ipcex_msg_t);
	this->qwr->valid = QUEUE_MAGIC_VALID;

	IRQ_CONTROLLER_t::getIRQController()->Add_Peripheral_IRQ_Listener(this, IPC_INT_ID);
	IRQ_CONTROLLER_t::getIRQController()->SetPriority(IPC_INT_ID, IPC_interrupt_priority);
	IRQ_CONTROLLER_t::getIRQController()->EnableInterrupt(IPC_INT_ID);
}

void IPC_proto_t::Add_IPC_Listener(IPC_listener_t* listener)
{
	for(int i = 0; i < IPC_Listeners_num; i++)
	{
		if(this->ipcex_listeners[i] == listener) break;
		if(this->ipcex_listeners[i] == (IPC_listener_t*)NULL)
		{
			this->ipcex_listeners[i] = listener;
			break;
		}
	}
}

void IPC_proto_t::Delete_IPC_Listener(IPC_listener_t* listener)
{
	uint32_t index = 0;
	for(int i = 0; i < IPC_Listeners_num; i++)
	{
		if(this->ipcex_listeners[i] == listener) break;
		else index++;
	}
	if(index == (IPC_Listeners_num-1))
	{
		if(this->ipcex_listeners[index] == listener) this->ipcex_listeners[index] = (IPC_listener_t*)NULL;
	}
	else
	{
		for(int i = index; i < (IPC_Listeners_num-1); i++)
		{
			this->ipcex_listeners[i] = this->ipcex_listeners[i+1];
			if(this->ipcex_listeners[i+1] == (IPC_listener_t*)NULL) break;
		}
	}
}

void IPC_proto_t::IRQ(uint32_t IRQ_num)
{
	ipcex_msg_t msg;
	ipc_gbl_val_t* gv;

	if (!QUEUE_IS_VALID(this->qrd)) return;

	for(int j=0; j< IPC_QUEUE_SZ; j++)
	{
		if (QUEUE_IS_EMPTY(this->qrd)) return;

		/* Pop the queue Item */
		memcpy(&msg, this->qrd->data + ((this->qrd->tail & (this->qrd->count - 1)) * this->qrd->size),
				this->qrd->size);
		this->qrd->tail++;

		if (msg.id < IPC_MAX_IDS)
		{
			if(msg.id == IPC_MSG_ID_GBLUPDATE)
			{
				gv = (ipc_gbl_val_t*)msg.data;
				if (gv->index < IPC_MAX_GBLVAL) this->gblval[gv->index] = gv->val;
				this->MsgPush(IPC_MSG_ID_FREEMEM, msg.data);
			}
			if(msg.id == IPC_MSG_ID_FREEMEM) free_s((void *)msg.data);

			for(int i = 0; i < IPC_Listeners_num; i++)
			{
				if(this->ipcex_listeners[i] != (IPC_listener_t*)NULL)
				{
					if(((this->ipcex_listeners[i]->getCode())>>msg.id) & 1) this->ipcex_listeners[i]->IPC_MSG_HANDLER(&msg);
				}
				else break;
			}
		}
	}
}

/* Get number of pending items in queue */
int IPC_proto_t::Qwr_msg_count(void)
{
	if (!QUEUE_IS_VALID(this->qwr)) return QUEUE_ERROR;
	return QUEUE_DATA_COUNT(this->qwr);
}

int IPC_proto_t::MsgPush(uint32_t id, uint32_t data)
{
	ipcex_msg_t msg;

#ifdef CORE_A9_0
	msg.sender = CORE_A9_0_GATE;
#endif
#ifdef CORE_A9_1
	msg.sender = CORE_A9_1_GATE;
#endif

	msg.id = id;
	msg.data = data;

	if (!QUEUE_IS_VALID(this->qwr)) return QUEUE_ERROR;
	if (QUEUE_IS_FULL(this->qwr)) return QUEUE_FULL;

	memcpy(IPC_proto_t::qwr->data + ((this->qwr->head & (this->qwr->count - 1)) * this->qwr->size), &msg,
			this->qwr->size);
	this->qwr->head++;

	this->sendInt();

	return QUEUE_INSERT;
}

int IPC_proto_t::SetGblVal(int index, uint32_t val)
{
	ipc_gbl_val_t* gv;

	if (index >= IPC_MAX_GBLVAL) return 1;

	gv = (ipc_gbl_val_t*)malloc_s(sizeof(ipc_gbl_val_t));
	if (gv == NULL) return 1; /* Something wrong */

	gv->val = this->gblval[index] = val;
	gv->index = index;
	if(this->MsgPush(IPC_MSG_ID_GBLUPDATE, (uint32_t) gv) != QUEUE_INSERT) {
		free_s(gv);
		return 1;
	}
	return 0;
}

uint32_t IPC_proto_t::GetGblVal(int index)
{
	if (index < IPC_MAX_GBLVAL) return this->gblval[index];
	else return 0;
}

void IPC_proto_t::sendInt(void){
	#ifdef CORE_A9_0
	IRQ_CONTROLLER_t::getIRQController()->sendSoftwareIntToCPU1(IPC_INT_ID);
	#endif
	#ifdef CORE_A9_1
	IRQ_CONTROLLER_t::getIRQController()->sendSoftwareIntToCPU0(IPC_INT_ID);
	#endif
}

