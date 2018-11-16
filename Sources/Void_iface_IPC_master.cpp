/*
 * Void_iface_IPC_master.cpp
 *
 *  Created on: 30.10.2017
 *      Author: Stalker1290
 */

#include "Void_iface_IPC_master.hpp"
#include "stdlib.h"
#include "xil_cache_l.h"

Void_iface_IPC_master_t::Void_iface_IPC_master_t(uint8_t gate,void_channel_t* Channel_ptr,
		uint8_t IFACE_IPC_MSG_ID, uint32_t rx_buf_size):IPC_listener_t(),Channel_Call_Interface_t()
{
	this->Channel_ptr = Channel_ptr;
	this->IFACE_IPC_MSG_ID = IFACE_IPC_MSG_ID;
	this->rx_buf_size = rx_buf_size;
	if(this->Channel_ptr != (void_channel_t*)NULL)
	{
		this->setCode((uint64_t)1<<IFACE_IPC_MSG_ID);
		this->rx_buf_ptr = (uint8_t*)malloc_s(this->rx_buf_size);
		this->Rx_ring_buf_ptr = new ring_buf_t((void*)this->rx_buf_ptr, 1, this->rx_buf_size);
		this->ipcProtoPtr = IPC_proto_t::get_IPC_proto(gate);
		this->ipcProtoPtr->Add_IPC_Listener(this);
	}
}

Void_iface_IPC_master_t::~Void_iface_IPC_master_t(void)
{
	if(this->Channel_ptr != (void_channel_t*)NULL)
	{
		this->Channel_ptr->DEInitialize();
		free_s(this->rx_buf_ptr);
		delete this->Rx_ring_buf_ptr;
		free_s(this->tx_buf_ptr);
	}
}

void Void_iface_IPC_master_t::channel_callback(uint8_t *mes,uint16_t m_size,void* channel_pointer,void* parameters)
{
	this->Rx_ring_buf_ptr->InsertMult(mes,m_size);
	this->rx_call_msg.msg_type = Rx_call_enum;
	this->rx_call_msg.ring_buf_ptr = this->Rx_ring_buf_ptr;
	this->ipcProtoPtr->MsgPush(this->IFACE_IPC_MSG_ID, (uint32_t)&this->rx_call_msg);
}

void Void_iface_IPC_master_t::IPC_MSG_HANDLER(ipcex_msg_t* msg)
{
	uint8_t type = *((uint8_t*)msg->data);
	if(type == Initialize_enum)
	{
		IPC_iface_msg_initialize_t* msg_ptr = (IPC_iface_msg_initialize_t*)msg->data;
		this->tx_buf_size = msg_ptr->tx_buf_size;
		if(this->Channel_ptr != (void_channel_t*)NULL) this->Channel_ptr->Initialize(malloc_s,msg_ptr->tx_buf_size,this);
		this->tx_buf_ptr = (uint8_t*)malloc_s(this->tx_buf_size);
	}
	if(type == DEInitialize_enum)
	{
		if(this->Channel_ptr != (void_channel_t*)NULL) this->Channel_ptr->DEInitialize();
	}
	if(type == Tx_enum)
	{
		IPC_iface_msg_Rx_Tx_t* msg_ptr = (IPC_iface_msg_Rx_Tx_t*)msg->data;
		if(this->Channel_ptr != (void_channel_t*)NULL)
		{
			this->Channel_ptr->Tx(this->tx_buf_ptr, msg_ptr->ring_buf_ptr->PopMult(this->tx_buf_ptr,this->tx_buf_size),NULL);
		}
	}
}
