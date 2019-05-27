/*
 * Void_iface_IPC_master.hpp
 *
 *  Created on: 30.10.2017
 *      Author: Stalker1290
 */

#ifndef VOID_IFACE_IPC_MASTER_HPP_
#define VOID_IFACE_IPC_MASTER_HPP_

#include "chip.h"
#include "IPC.hpp"
#include "Void_Channel.hpp"
#include "Void_iface_IPC_defines.hpp"


class Void_iface_IPC_master_t:protected IPC_listener_t, public Channel_Call_Interface_t
{
public:
	Void_iface_IPC_master_t(uint8_t gate,void_channel_t* Channel_ptr, uint8_t IFACE_IPC_MSG_ID, uint32_t rx_buf_size);
	virtual ~Void_iface_IPC_master_t(void);
	virtual void channel_callback(uint8_t *mes,uint16_t m_size,void* channel_pointer,void* parameters);
private:
	virtual void IPC_MSG_HANDLER(ipcex_msg_t* msg);
	void_channel_t* Channel_ptr;
	uint8_t IFACE_IPC_MSG_ID;
	uint32_t rx_buf_size;
	uint8_t* rx_buf_ptr;
	ring_buf_t* Rx_ring_buf_ptr;
	IPC_iface_msg_Rx_Tx_t rx_call_msg;
	uint32_t tx_buf_size;
	uint8_t* tx_buf_ptr;
	IPC_proto_t* ipcProtoPtr;
};

#endif /* VOID_IFACE_IPC_MASTER_HPP_ */
