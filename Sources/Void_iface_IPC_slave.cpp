/*
 * Void_iface_IPC_slave.cpp
 *
 *  Created on: 30.10.2017
 *      Author: Stalker1290
 */


#include "Void_iface_IPC_slave.hpp"
#include "xil_cache_l.h"

Void_iface_IPC_slave_t::Void_iface_IPC_slave_t(uint8_t gate,uint8_t IFACE_IPC_MSG_ID,uint32_t rx_buf_size):IPC_listener_t(),void_channel_t()
{
	this->IFACE_IPC_MSG_ID = IFACE_IPC_MSG_ID;
	this->setCode((uint64_t)1<<IFACE_IPC_MSG_ID);
	this->rx_buf_size = rx_buf_size;
	this->rx_buf_ptr = (uint8_t*)malloc_s(this->rx_buf_size);
	this->call_interface_ptr = (Channel_Call_Interface_t*)NULL;
	this->ipcProtoPtr = IPC_proto_t::get_IPC_proto(gate);
	this->ipcProtoPtr->Add_IPC_Listener(this);
}

Void_iface_IPC_slave_t::~Void_iface_IPC_slave_t(void)
{
	delete this->Tx_ring_buf_ptr;
	free_s(this->rx_buf_ptr);
	free_s(this->tx_buf_ptr);
}

void Void_iface_IPC_slave_t::Initialize(void* (*mem_alloc)(size_t),uint16_t tx_buf_size, Channel_Call_Interface_t* call_interface_ptr)
{
	this->tx_buf_size = tx_buf_size;
	this->tx_buf_ptr = (uint8_t*)malloc_s(this->tx_buf_size);
	this->Tx_ring_buf_ptr = new ring_buf_t(this->tx_buf_ptr, 1, this->tx_buf_size);
	this->call_interface_ptr = call_interface_ptr;
	this->ini_msg.msg_type = Initialize_enum;
	this->ini_msg.tx_buf_size = tx_buf_size;
	this->ipcProtoPtr->MsgPush(this->IFACE_IPC_MSG_ID,(uint32_t)&this->ini_msg);
}

void Void_iface_IPC_slave_t::DEInitialize(void)
{
	this->deini_msg.msg_type = DEInitialize_enum;
	this->ipcProtoPtr->MsgPush(this->IFACE_IPC_MSG_ID,(uint32_t)&this->deini_msg);
}

void Void_iface_IPC_slave_t::Tx(uint8_t *mes,uint16_t m_size,void* param)
{
	this->Tx_ring_buf_ptr->InsertMult(mes,m_size);
	this->tx_rx_msg.msg_type = Tx_enum;
	this->tx_rx_msg.ring_buf_ptr = this->Tx_ring_buf_ptr;
	this->ipcProtoPtr->MsgPush(this->IFACE_IPC_MSG_ID,(uint32_t)&this->tx_rx_msg);
}

void Void_iface_IPC_slave_t::GetParameter(uint8_t ParamName, void* ParamValue)
{

}

void Void_iface_IPC_slave_t::SetParameter(uint8_t ParamName, void* ParamValue)
{

}

void Void_iface_IPC_slave_t::IPC_MSG_HANDLER(ipcex_msg_t* msg)
{
	uint8_t type = *((uint8_t*)msg->data);
	if(type == Rx_call_enum)
	{
		IPC_iface_msg_Rx_Tx_t* msg_ptr = (IPC_iface_msg_Rx_Tx_t*)msg->data;
		if(this->call_interface_ptr != (Channel_Call_Interface_t*)NULL)
		{
			this->call_interface_ptr->channel_callback(this->rx_buf_ptr,msg_ptr->ring_buf_ptr->PopMult(this->rx_buf_ptr,this->rx_buf_size),this, NULL);
		}
	}
}
