/*
 * Void_iface_IPC_slave.hpp
 *
 *  Created on: 30.10.2017
 *      Author: Stalker1290
 */

#ifndef VOID_IFACE_IPC_SLAVE_HPP_
#define VOID_IFACE_IPC_SLAVE_HPP_

#include "chip.h"
#include "IPC.hpp"
#include "Void_Channel.hpp"
#include "Void_iface_IPC_defines.hpp"

class Void_iface_IPC_slave_t:protected IPC_listener_t, public void_channel_t
{
public:
	Void_iface_IPC_slave_t(uint8_t gate,uint8_t IFACE_IPC_MSG_ID,uint32_t rx_buf_size);
	virtual ~Void_iface_IPC_slave_t(void);
	virtual void Initialize(void* (*mem_alloc)(size_t),uint16_t tx_buf_size, Channel_Call_Interface_t* call_interface_ptr);
	virtual void DEInitialize(void);
	virtual void Tx(uint8_t *mes,uint16_t m_size,void* param);
	virtual void GetParameter(uint8_t ParamName, void* ParamValue);
	virtual void SetParameter(uint8_t ParamName, void* ParamValue);
private:
	virtual void IPC_MSG_HANDLER(ipcex_msg_t* msg);
	uint8_t IFACE_IPC_MSG_ID;
	IPC_iface_msg_initialize_t ini_msg;
	IPC_iface_msg_DEInitialize_t deini_msg;
	IPC_iface_msg_Rx_Tx_t tx_rx_msg;
	uint32_t tx_buf_size;
	uint8_t* tx_buf_ptr;
	ring_buf_t* Tx_ring_buf_ptr;
	uint32_t rx_buf_size;
	uint8_t* rx_buf_ptr;
	Channel_Call_Interface_t* call_interface_ptr;
	IPC_proto_t* ipcProtoPtr;
};

#endif /* VOID_IFACE_IPC_SLAVE_HPP_ */
