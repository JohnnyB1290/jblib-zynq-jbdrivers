/*
 * Void_Console.cpp
 *
 *  Created on: 14.11.2018.
 *      Author: Stalker1290
 */

#include "Void_Console.hpp"
#include "string.h"

Void_Console_t* Void_Console_t::Void_console_ptr = (Void_Console_t*)NULL;

#ifdef __cplusplus
extern "C" {
#endif

int _write(int iFileHandle, char *pcBuffer, int iLength)
{
	if(Void_Console_t::Get_Console(0)->Tx_ring_buffer_ptr != (ring_buf_t*)NULL)
		Void_Console_t::Get_Console(0)->Tx_ring_buffer_ptr->InsertMult((uint8_t*)pcBuffer,iLength );
	return 0;
}

#ifdef __cplusplus
}
#endif

Void_Console_t* Void_Console_t::Get_Console(uint16_t tx_buf_size)
{
	if(Void_Console_t::Void_console_ptr == (Void_Console_t*)NULL) Void_Console_t::Void_console_ptr = new Void_Console_t(tx_buf_size);
	return Void_Console_t::Void_console_ptr;
}

Void_Console_t::Void_Console_t(uint16_t tx_buf_size):Channel_Call_Interface_t(),Callback_Interface_t()
{
	this->Rx_rinf_buffer_ptr = new ring_buf_t(this->rx_buf,1,Console_Rx_buf_size);
	this->Rx_rinf_buffer_ptr->Flush();
	this->Output_channel = (void_channel_t*)NULL;
	CONTROLLER_t::get_CONTROLLER()->Add_main_procedure(this);
	this->tx_buf_size = 0;
	this->tx_buf_ptr = (uint8_t*)NULL;
	this->Tx_ring_buffer_ptr = (ring_buf_t*)NULL;
	if(tx_buf_size != 0)
	{
		this->tx_buf_size = tx_buf_size;
		this->tx_buf_ptr = (uint8_t*)malloc(tx_buf_size);
		if(this->tx_buf_ptr != (uint8_t*)NULL)  this->Tx_ring_buffer_ptr = new ring_buf_t(this->tx_buf_ptr, 1, tx_buf_size);
	}
	memset(this->Cmd_buf, 0, CONSOLE_CMD_BUF_SIZE);
	this->Cmd_index = 0;
}

void Void_Console_t::Set_output_channel(void_channel_t* Output_channel)
{
	this->Output_channel = Output_channel;
}

void_channel_t* Void_Console_t::Get_output_channel(void)
{
	return this->Output_channel;
}

void Void_Console_t::Add_console_listener(Console_listener_t* listener)
{
	for(int i = 0; i < Console_listeners_num; i++)
	{
		if(this->Console_Listeners[i] == listener) break;
		if(this->Console_Listeners[i] == (Console_listener_t*)NULL)
		{
			this->Console_Listeners[i] = listener;
			break;
		}
	}
}

void Void_Console_t::Delete_console_listener(Console_listener_t* listener)
{
	uint32_t index = 0;
	for(int i = 0; i < Console_listeners_num; i++)
	{
		if(this->Console_Listeners[i] == listener) break;
		else index++;
	}
	if(index == (Console_listeners_num-1))
	{
		if(this->Console_Listeners[index] == listener) this->Console_Listeners[index] = (Console_listener_t*)NULL;
	}
	else
	{
		for(int i = index; i < (Console_listeners_num-1); i++)
		{
			this->Console_Listeners[i] = this->Console_Listeners[i+1];
			if(this->Console_Listeners[i+1] == (Console_listener_t*)NULL) break;
		}
	}
}

void Void_Console_t::channel_callback(uint8_t *mes,uint16_t m_size,void* channel_pointer,void* parameters)
{
	this->Rx_rinf_buffer_ptr->InsertMult(mes, m_size);
}

void Void_Console_t::void_callback(void* Intf_ptr, void* parameters)
{
	uint8_t temp_buf[Console_Tx_temp_buf_size];
	uint16_t m_size = 0;
	uint8_t temp_ch;

	if(this->Output_channel != (void_channel_t*)NULL)
	{
		m_size = this->Tx_ring_buffer_ptr->PopMult(temp_buf,Console_Tx_temp_buf_size);
		if(m_size)
		{
			this->Output_channel->Tx(temp_buf, m_size,NULL);
		}
	}

	m_size = this->Rx_rinf_buffer_ptr->Pop(&temp_ch);
	while(m_size)
	{
		if(temp_ch == 0x0A)
		{
			if(this->Cmd_index > 0)
			{
				if(this->Cmd_buf[this->Cmd_index - 1] == 0x0D) this->Cmd_buf[this->Cmd_index - 1] = 0;
			}
			this->Cmd_buf[this->Cmd_index] = 0;
			printf("#JBravo::%s\n\r",(char*)this->Cmd_buf);
			for(int i = 0; i < Console_listeners_num; i++)
			{
				if(this->Console_Listeners[i] != (Console_listener_t*)NULL)
							this->Console_Listeners[i]->Parse_console_mes(this->Cmd_buf, this->Cmd_index);
				else break;
			}
			this->Cmd_index = 0;
		}
		else
		{
			this->Cmd_buf[this->Cmd_index] = temp_ch;
			this->Cmd_index++;
			if(this->Cmd_index == CONSOLE_CMD_BUF_SIZE) this->Cmd_index = 0;
		}

		m_size = this->Rx_rinf_buffer_ptr->Pop(&temp_ch);
	}
}
