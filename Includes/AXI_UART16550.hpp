/*
 * AXI_UART16550.hpp
 *
 *  Created on: 20 февр. 2018 г.
 *      Author: Stalker1290
 */

#ifndef AXI_UART16550_HPP_
#define AXI_UART16550_HPP_

#include "Void_Channel.hpp"
#include "IRQ_Controller.hpp"

class AXI_UART16550_int_t:public void_channel_t,protected IRQ_LISTENER_t
{
public:
	static AXI_UART16550_int_t* get_AXI_UART16550_int(uint8_t u_num, uint32_t baudrate);
	virtual void Initialize(void* (*mem_alloc)(size_t),uint16_t tx_buf_size, Channel_Call_Interface_t* call_interface_ptr);
	virtual void DEInitialize(void);
	virtual void Tx(uint8_t *mes,uint16_t m_size,void* param);
	virtual void GetParameter(uint8_t ParamName, void* ParamValue);
	virtual void SetParameter(uint8_t ParamName, void* ParamValue);
private:
	AXI_UART16550_int_t(uint8_t u_num, uint32_t baudrate);
	virtual ~AXI_UART16550_int_t(void);
	virtual void IRQ(uint32_t IRQ_num);
	uint8_t u_num;
	uint32_t baudrate;
	RINGBUFF_T Tx_ring_buf;
	uint8_t* Tx_buf_ptr;
	uint16_t tx_buf_size;
	Channel_Call_Interface_t* call_interface_ptr;
	uint8_t UART_initialize;
	uint8_t w_busy;
	static AXI_UART16550_int_t* UART_ptrs[AXI_UART16550_num_def];

	static uint32_t Axi_Uart16550_Base_addr[AXI_UART16550_num_def];
	static uint32_t Axi_Uart16550_Clock_freq[AXI_UART16550_num_def];
	static uint32_t Axi_Uart16550_INT_ID[AXI_UART16550_num_def];
	static uint32_t Axi_Uart16550_INT_priority[AXI_UART16550_num_def];
};




#endif /* AXI_UART16550_HPP_ */
