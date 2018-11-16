/*
 * Void_iface_IPC_defines.hpp
 *
 *  Created on: 30.10.2017
 *      Author: Stalker1290
 */

#ifndef VOID_IFACE_IPC_DEFINES_HPP_
#define VOID_IFACE_IPC_DEFINES_HPP_

#include "chip.h"
#include "Ring_buf.hpp"

typedef enum
{
	Initialize_enum = 0,
	DEInitialize_enum = 1,
	Tx_enum = 2,
	Rx_call_enum = 3,
}IPC_iface_msg_type_t;

typedef struct IPC_iface_msg_initialize_struct
{
	uint8_t msg_type;
	uint16_t tx_buf_size;
}IPC_iface_msg_initialize_t;

typedef struct IPC_iface_msg_DEInitialize_struct
{
	uint8_t msg_type;
}IPC_iface_msg_DEInitialize_t;

typedef struct IPC_iface_msg_Rx_Tx_struct
{
	uint8_t msg_type;
	ring_buf_t* ring_buf_ptr;
}IPC_iface_msg_Rx_Tx_t;


#endif /* VOID_IFACE_IPC_DEFINES_HPP_ */
