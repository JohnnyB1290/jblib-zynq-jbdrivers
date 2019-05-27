/*
 * AXI_UART16550.cpp
 *
 *  Created on: 20.02.2018 ã.
 *      Author: Stalker1290
 */

#include "AXI_UART16550.hpp"
#include "xuartns550_l.h"

AXI_UART16550_int_t* AXI_UART16550_int_t::UART_ptrs[AXI_UART16550_num_def] = AXI_UART16550_ptrs_def;
uint32_t AXI_UART16550_int_t::Axi_Uart16550_Base_addr[AXI_UART16550_num_def] = Axi_Uart16550_Base_addr_def;
uint32_t AXI_UART16550_int_t::Axi_Uart16550_Clock_freq[AXI_UART16550_num_def] = Axi_Uart16550_Clock_freq_def;
uint32_t AXI_UART16550_int_t::Axi_Uart16550_INT_ID[AXI_UART16550_num_def] = Axi_Uart16550_INT_ID_def;
uint32_t AXI_UART16550_int_t::Axi_Uart16550_INT_priority[AXI_UART16550_num_def] = Axi_Uart16550_INT_priority_def;


AXI_UART16550_int_t* AXI_UART16550_int_t::get_AXI_UART16550_int(uint8_t u_num, uint32_t baudrate)
{
	if(u_num >= AXI_UART16550_num_def) return NULL;
	if(AXI_UART16550_int_t::UART_ptrs[u_num] == (AXI_UART16550_int_t*)NULL)
		AXI_UART16550_int_t::UART_ptrs[u_num] = new AXI_UART16550_int_t(u_num, baudrate);
	return AXI_UART16550_int_t::UART_ptrs[u_num];
}


AXI_UART16550_int_t::AXI_UART16550_int_t(uint8_t u_num, uint32_t baudrate):void_channel_t(),IRQ_LISTENER_t()
{
	this->Tx_buf_ptr = (uint8_t*)NULL;;
	this->tx_buf_size = 0;
	this->call_interface_ptr = (Channel_Call_Interface_t*)NULL;
	this->UART_initialize = 0;
	this->u_num = u_num;
	this->baudrate = baudrate;
	this->w_busy = 0;
}


void AXI_UART16550_int_t::Initialize(void* (*mem_alloc)(size_t),uint16_t tx_buf_size, Channel_Call_Interface_t* call_interface_ptr)
{
	if(this->UART_initialize == 0)
	{
		if(tx_buf_size == 0) return;
		this->tx_buf_size = tx_buf_size;
		this->Tx_buf_ptr = (uint8_t*)mem_alloc(this->tx_buf_size);

		if(this->Tx_buf_ptr == (uint8_t*)NULL) return;
		if(call_interface_ptr == (Channel_Call_Interface_t*)NULL) return;
		this->call_interface_ptr = call_interface_ptr;
		RingBuffer_Init(&(this->Tx_ring_buf), this->Tx_buf_ptr, 1, this->tx_buf_size);


		XUartNs550_SetBaud(AXI_UART16550_int_t::Axi_Uart16550_Base_addr[this->u_num],
				AXI_UART16550_int_t::Axi_Uart16550_Clock_freq[this->u_num], this->baudrate);

		XUartNs550_SetLineControlReg(AXI_UART16550_int_t::Axi_Uart16550_Base_addr[this->u_num], XUN_LCR_8_DATA_BITS);

		IRQ_CONTROLLER_t::getIRQController()->Add_Peripheral_IRQ_Listener(this,
				AXI_UART16550_int_t::Axi_Uart16550_INT_ID[this->u_num]);
		IRQ_CONTROLLER_t::getIRQController()->SetPriority(AXI_UART16550_int_t::Axi_Uart16550_INT_ID[this->u_num],
				AXI_UART16550_int_t::Axi_Uart16550_INT_priority[this->u_num]);
		IRQ_CONTROLLER_t::getIRQController()->EnableInterrupt(AXI_UART16550_int_t::Axi_Uart16550_INT_ID[this->u_num]);

		XUartNs550_WriteReg(AXI_UART16550_int_t::Axi_Uart16550_Base_addr[this->u_num], XUN_IER_OFFSET,
				XUartNs550_ReadReg(AXI_UART16550_int_t::Axi_Uart16550_Base_addr[this->u_num], XUN_IER_OFFSET)|
				(XUN_IER_TX_EMPTY | XUN_IER_RX_DATA));
		this->UART_initialize = 1;
	}
}

void AXI_UART16550_int_t::IRQ(uint32_t IRQ_num)
{
	static u8 IsrStatus;
	int count;
	static uint8_t rec_byte = 0;
	static uint8_t Byte_to_tx = 0;

	if(this->UART_initialize == 1)
	{
		IsrStatus = (u8)XUartNs550_ReadReg(AXI_UART16550_int_t::Axi_Uart16550_Base_addr[this->u_num],
											XUN_IIR_OFFSET) &XUN_INT_ID_MASK;

		if(IsrStatus == 2) // Tramsm Empty
		{
			count = RingBuffer_GetCount(&this->Tx_ring_buf);
			if(count>0)
			{
				if(RingBuffer_Pop(&this->Tx_ring_buf, &Byte_to_tx))
				{
					this->w_busy = 1;
					XUartNs550_SendByte(AXI_UART16550_int_t::Axi_Uart16550_Base_addr[this->u_num], Byte_to_tx);
				}
				else this->w_busy = 0;
			}
			else
			{
				this->w_busy = 0;
			}
		}
		if(IsrStatus == 4) // Recieve data avaliable
		{
			rec_byte = XUartNs550_RecvByte(AXI_UART16550_int_t::Axi_Uart16550_Base_addr[this->u_num]);
			this->call_interface_ptr->channel_callback(&rec_byte,1,(void*)this, NULL);
		}
	}
}

void AXI_UART16550_int_t::DEInitialize(void)
{
	if(this->UART_initialize == 1)
	{
		IRQ_CONTROLLER_t::getIRQController()->Delete_Peripheral_IRQ_Listener(this);
	}
}

AXI_UART16550_int_t::~AXI_UART16550_int_t(void)
{
	IRQ_CONTROLLER_t::getIRQController()->Delete_Peripheral_IRQ_Listener(this);
}


void AXI_UART16550_int_t::Tx(uint8_t *mes,uint16_t m_size,void* param)
{
	u8 TxEmptyStatus;
	uint8_t Byte_to_tx;

	if(this->UART_initialize == 1)
	{
		RingBuffer_InsertMult(&this->Tx_ring_buf, (void*)mes, m_size);

		TxEmptyStatus = (u8)XUartNs550_ReadReg(AXI_UART16550_int_t::Axi_Uart16550_Base_addr[this->u_num],
				XUN_LSR_OFFSET) &0x40; //Check if transmitter empty

		if( (this->w_busy == 0) || ((this->w_busy == 1)&&(TxEmptyStatus != 0)))
		{
			if(RingBuffer_Pop(&this->Tx_ring_buf, &Byte_to_tx))
			{
				this->w_busy = 1;
				XUartNs550_SendByte(AXI_UART16550_int_t::Axi_Uart16550_Base_addr[this->u_num], Byte_to_tx);
			}
			else this->w_busy = 0;
		}
	}
}


void AXI_UART16550_int_t::GetParameter(uint8_t ParamName, void* ParamValue)
{
	uint8_t MCTRL_Reg;
	uint8_t cts_signal;
	uint8_t* ParamValue_ptr = (uint8_t*)ParamValue;

	if(this->UART_initialize == 1)
	{
		if(ParamName == AXI_UART16550_CTS_param)
		{
			MCTRL_Reg = (u8)XUartNs550_ReadReg(AXI_UART16550_int_t::Axi_Uart16550_Base_addr[this->u_num], XUN_MSR_OFFSET);
			cts_signal = (MCTRL_Reg&16)>>4;
			cts_signal ^= 1;
			*ParamValue_ptr = cts_signal;
		}
		if(ParamName == AXI_UART16550_TXEmpty_param)
		{
			if(RingBuffer_GetCount(&this->Tx_ring_buf)) *ParamValue_ptr = 0;
			else *ParamValue_ptr = 1;
		}
	}
}

void AXI_UART16550_int_t::SetParameter(uint8_t ParamName, void* ParamValue)
{
	uint8_t MCTRL_Reg;

	if(this->UART_initialize == 1)
	{
		MCTRL_Reg = (u8)XUartNs550_ReadReg(AXI_UART16550_int_t::Axi_Uart16550_Base_addr[this->u_num], XUN_MCR_OFFSET);

		if(ParamName == AXI_UART16550_RTS_clr_param)
		{
			XUartNs550_WriteReg(AXI_UART16550_int_t::Axi_Uart16550_Base_addr[this->u_num],  XUN_MCR_OFFSET,
					MCTRL_Reg|XUN_MCR_RTS);
		}
		if(ParamName == AXI_UART16550_RTS_set_param)
		{
			XUartNs550_WriteReg(AXI_UART16550_int_t::Axi_Uart16550_Base_addr[this->u_num],  XUN_MCR_OFFSET,
					MCTRL_Reg&(~XUN_MCR_RTS));
		}
	}
}

