/*
 * IRQ_CONTROLLER.cpp
 *
 *  Created on: 12.07.2017
 *      Author: Stalker1290
 */

#include "IRQ_Controller.hpp"
#include "stdio.h"

IRQ_CONTROLLER_t* IRQ_CONTROLLER_t::IRQ_Controller_ptr = (IRQ_CONTROLLER_t*)NULL;


IRQ_CONTROLLER_t* IRQ_CONTROLLER_t::getIRQController(void)
{
	if(IRQ_CONTROLLER_t::IRQ_Controller_ptr == (IRQ_CONTROLLER_t*)NULL) IRQ_CONTROLLER_t::IRQ_Controller_ptr = new IRQ_CONTROLLER_t();
	return IRQ_CONTROLLER_t::IRQ_Controller_ptr;
}


IRQ_CONTROLLER_t::IRQ_CONTROLLER_t(void)
{
	for(int i = 0; i < Peripheral_Listeners_num; i++)
	{
		this->Peripheral_IRQ_LISTENERS[i] = (IRQ_LISTENER_t*)NULL;
		this->Peripheral_IRQ_LISTENERS_INTR_ID[i] = 0;
	}

	for(uint32_t i = 0; i<XSCUGIC_MAX_NUM_INTR_INPUTS; i++)
	{
		XScuGic_WriteReg(XPAR_SCUGIC_0_CPU_BASEADDR, XSCUGIC_EOI_OFFSET, i);
	}

	this->GicConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);
	this->IController_instance.Config = this->GicConfig;

	XScuGic_CfgInitialize(&this->IController_instance, this->GicConfig, this->GicConfig->CpuBaseAddress);

	XScuGic_CPUWriteReg(&this->IController_instance, XSCUGIC_BIN_PT_OFFSET,0x03);

	Xil_ExceptionInit();
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,(Xil_ExceptionHandler) XScuGic_InterruptHandler,
			&this->IController_instance);

	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_UNDEFINED_INT,
			(Xil_ExceptionHandler) IRQ_CONTROLLER_t::HANDLE_Undefined_INT,
			&this->IController_instance);

	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_SWI_INT,
			(Xil_ExceptionHandler) IRQ_CONTROLLER_t::HANDLE_SWI_INT,
			&this->IController_instance);

	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_PREFETCH_ABORT_INT,
			(Xil_ExceptionHandler) IRQ_CONTROLLER_t::HANDLE_Prefetch_Abort_INT,
			&this->IController_instance);

	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_DATA_ABORT_INT,
				(Xil_ExceptionHandler) IRQ_CONTROLLER_t::HANDLE_Data_Abort_INT,
				&this->IController_instance);

	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_FIQ_INT,
				(Xil_ExceptionHandler) IRQ_CONTROLLER_t::HANDLE_FIQ_INT,
				&this->IController_instance);

	Xil_ExceptionEnable();

}


void IRQ_CONTROLLER_t::EnableInterrupt(u32 InterruptID)
{
	XScuGic_Connect(&this->IController_instance, InterruptID,
			(Xil_InterruptHandler)IRQ_CONTROLLER_t::HANDLE_Peripheral_IRQ, (void*)InterruptID);
	XScuGic_Enable(&this->IController_instance, InterruptID);
}

void IRQ_CONTROLLER_t::DisableInterrupt(u32 InterruptID)
{
	XScuGic_Disable(&this->IController_instance, InterruptID);
	XScuGic_Disconnect(&this->IController_instance, InterruptID);
}

void IRQ_CONTROLLER_t::SetPriority(u32 InterruptID, u8 Priority, u8 Trigger)
{
	XScuGic_SetPriorityTriggerType(&this->IController_instance, InterruptID, Priority, Trigger);
}

void IRQ_CONTROLLER_t::SetPriority(u32 InterruptID, u8 Priority)
{
	u8 Get_Priority;
	u8 Get_Trigger;

	XScuGic_GetPriorityTriggerType(&IController_instance, InterruptID, &Get_Priority, &Get_Trigger);
	XScuGic_SetPriorityTriggerType(&IController_instance,InterruptID ,Priority, Get_Trigger);
}

void IRQ_CONTROLLER_t::Add_Peripheral_IRQ_Listener(IRQ_LISTENER_t* listener, uint32_t IRQ_num)
{
	Xil_ExceptionDisable();
	for(int i = 0; i < Peripheral_Listeners_num; i++)
	{
		if(this->Peripheral_IRQ_LISTENERS[i] == (IRQ_LISTENER_t*)NULL)
		{
			this->Peripheral_IRQ_LISTENERS[i] = listener;
			this->Peripheral_IRQ_LISTENERS_INTR_ID[i] = IRQ_num;
			break;
		}
	}
	Xil_ExceptionEnable();
}

void IRQ_CONTROLLER_t::Delete_Peripheral_IRQ_Listener(IRQ_LISTENER_t* listener)
{
	uint32_t index = 0;
	Xil_ExceptionDisable();

	for(int i = 0; i < Peripheral_Listeners_num; i++)
	{
		if(this->Peripheral_IRQ_LISTENERS[i] == listener) break;
		else index++;
	}
	if(index == (Peripheral_Listeners_num-1))
	{
		if(this->Peripheral_IRQ_LISTENERS[index] == listener)
		{
			this->Peripheral_IRQ_LISTENERS[index] = (IRQ_LISTENER_t*)NULL;
			this->Peripheral_IRQ_LISTENERS_INTR_ID[index] = 0;
		}
	}
	else
	{
		for(int i = index; i < (Peripheral_Listeners_num-1); i++)
		{
			this->Peripheral_IRQ_LISTENERS[i] = this->Peripheral_IRQ_LISTENERS[i+1];
			this->Peripheral_IRQ_LISTENERS_INTR_ID[i] = this->Peripheral_IRQ_LISTENERS_INTR_ID[i+1];
			if(this->Peripheral_IRQ_LISTENERS[i+1] == (IRQ_LISTENER_t*)NULL) break;
		}
	}
	Xil_ExceptionEnable();

	if(index < (Peripheral_Listeners_num-1)) this->Delete_Peripheral_IRQ_Listener(listener);
}

void IRQ_CONTROLLER_t::HANDLE_Peripheral_IRQ(void* IRQ_num)
{
	uint32_t irq_num_uint = (uint32_t)IRQ_num;

	Xil_EnableNestedInterrupts();

	for(int i = 0; i < Peripheral_Listeners_num; i++)
	{
		if(IRQ_CONTROLLER_t::getIRQController()->Peripheral_IRQ_LISTENERS[i] != (IRQ_LISTENER_t*)NULL)
		{
			if(IRQ_CONTROLLER_t::getIRQController()->Peripheral_IRQ_LISTENERS_INTR_ID[i] == irq_num_uint)
			{
				IRQ_CONTROLLER_t::getIRQController()->Peripheral_IRQ_LISTENERS[i]->IRQ(irq_num_uint);
			}
		}
		else break;
	}
	Xil_DisableNestedInterrupts();
}


void IRQ_CONTROLLER_t::HANDLE_Undefined_INT(void *data)
{
	printf("UNDEFINED INTERRUPT!!!\n\r");
	while(1);
	//CONTROLLER_t::get_CONTROLLER()->Jump_to_Addr(WDT_RESET_ADDR);
}

void IRQ_CONTROLLER_t::HANDLE_SWI_INT(void *data)
{
	printf("SWI INTERRUPT!!!\n\r");
	while(1);
	//CONTROLLER_t::get_CONTROLLER()->Jump_to_Addr(WDT_RESET_ADDR);
}

void IRQ_CONTROLLER_t::HANDLE_Prefetch_Abort_INT(void *data)
{
	printf("Prefetch Abort INTERRUPT!!!\n\r");
	while(1);
	//CONTROLLER_t::get_CONTROLLER()->Jump_to_Addr(WDT_RESET_ADDR);
}

void IRQ_CONTROLLER_t::HANDLE_Data_Abort_INT(void *data)
{
	printf("DATA ABORT INTERRUPT!!!\n\r");
	while(1);
	//CONTROLLER_t::get_CONTROLLER()->Jump_to_Addr(WDT_RESET_ADDR);
}

void IRQ_CONTROLLER_t::HANDLE_FIQ_INT(void *data)
{
	printf("FIQ INTERRUPT!!!\n\r");
	while(1);
//	CONTROLLER_t::get_CONTROLLER()->Jump_to_Addr(WDT_RESET_ADDR);
}
