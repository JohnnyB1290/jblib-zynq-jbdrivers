/*
 * Void_Console.hpp
 *
 *  Created on: 10 окт. 2017 г.
 *      Author: Stalker1290
 */

#include "Common_interfaces.hpp"
#include "Void_Channel.hpp"
#include "stdio.h"
#include "stdlib.h"
#include "Defines.h"
#include "Ring_buf.hpp"
#include "CONTROLLER.hpp"

#ifndef VOID_CONSOLE_HPP_
#define VOID_CONSOLE_HPP_


class Console_listener_t
{
public:
	Console_listener_t(void){}
    virtual ~Console_listener_t(void){}
    virtual void Parse_console_mes(uint8_t *mes,uint16_t m_size) = 0;
};


class Void_Console_t:public Channel_Call_Interface_t,Callback_Interface_t
{
public:
	static Void_Console_t* Get_Console(uint16_t tx_buf_size);
	void Set_output_channel(void_channel_t* Output_channel);
	void_channel_t* Get_output_channel(void);
	void Add_console_listener(Console_listener_t* listener);
	void Delete_console_listener(Console_listener_t* listener);
	virtual void channel_callback(uint8_t *mes,uint16_t m_size,void* channel_pointer,void* parameters);
	virtual void void_callback(void* Intf_ptr, void* parameters);
	ring_buf_t* Tx_ring_buffer_ptr;
private:
	static Void_Console_t* Void_console_ptr;
	void_channel_t* Output_channel;
	Console_listener_t* Console_Listeners[Console_listeners_num];
	Void_Console_t(uint16_t tx_buf_size);
	uint8_t rx_buf[Console_Rx_buf_size];
	ring_buf_t* Rx_rinf_buffer_ptr;
	uint8_t* tx_buf_ptr;
	uint16_t tx_buf_size;
	uint8_t Cmd_buf[CONSOLE_CMD_BUF_SIZE];
	uint16_t Cmd_index;
};



#endif /* VOID_CONSOLE_HPP_ */
