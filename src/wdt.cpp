/*
 * wdt.cpp
 *
 *  Created on: 14.03.2018 ã.
 *      Author: Stalker1290
 */

#include "wdt.hpp"
#include "stdio.h"
#include "Defines.h"
#include "CONTROLLER.hpp"

WDT_t* WDT_t::WDT_t_instance_ptr = NULL;

WDT_t* WDT_t::get_WDT(void)
{
	if(WDT_t::WDT_t_instance_ptr == NULL) WDT_t::WDT_t_instance_ptr = new WDT_t();
	return WDT_t::WDT_t_instance_ptr;
}

WDT_t::WDT_t(void):IRQ_LISTENER_t(), Callback_Interface_t()
{
	this->Initialized = false;
	this->Last_reset_wd = false;
}

void WDT_t::Initialize(uint16_t reload_time_s)
{
	XScuWdt_Config *ConfigPtr;
	uint32_t Reg;
	uint32_t WD_load;

	if(!this->Initialized)
	{
		ConfigPtr = XScuWdt_LookupConfig(WDT_DEVICE_ID);
		XScuWdt_CfgInitialize((XScuWdt*)&this->Watchdog, ConfigPtr,ConfigPtr->BaseAddr);
		this->WdtInstancePtr = (XScuWdt*)&this->Watchdog;

		XScuWdt_SetTimerMode(this->WdtInstancePtr);

		this->Last_reset_wd = ((XScuWdt_ReadReg(this->Watchdog.Config.BaseAddr,	XSCUWDT_ISR_OFFSET) & XSCUWDT_ISR_EVENT_FLAG_MASK) == XSCUWDT_ISR_EVENT_FLAG_MASK);
		XScuWdt_WriteReg(this->Watchdog.Config.BaseAddr, XSCUWDT_ISR_OFFSET, XSCUWDT_ISR_EVENT_FLAG_MASK);

		Reg = XScuWdt_ReadReg(this->Watchdog.Config.BaseAddr,XSCUWDT_CONTROL_OFFSET);
		XScuWdt_SetControlReg(this->WdtInstancePtr, Reg|XSCUWDT_CONTROL_IT_ENABLE_MASK);

		WD_load = XPAR_PS7_CORTEXA9_0_CPU_CLK_FREQ_HZ>>1;
		WD_load = reload_time_s * WD_load;
		XScuWdt_LoadWdt(this->WdtInstancePtr,WD_load);

		IRQ_CONTROLLER_t::getIRQController()->Add_Peripheral_IRQ_Listener(this, WDT_INT_ID);
		IRQ_CONTROLLER_t::getIRQController()->SetPriority(WDT_INT_ID, WDT_interrupt_priority);
		IRQ_CONTROLLER_t::getIRQController()->EnableInterrupt(WDT_INT_ID);
		this->Initialized = true;
	}
}

void WDT_t::Start(void)
{
	this->Reset();
	printf("WDT Start\n\r");
	XScuWdt_Start(this->WdtInstancePtr);
}

void WDT_t::Stop(void)
{
	printf("WDT Stop\n\r");
	XScuWdt_Stop(this->WdtInstancePtr);
}

void WDT_t::Reset(void)
{
	XScuWdt_RestartWdt(this->WdtInstancePtr);
}

bool WDT_t::Is_last_reset_wd(void)
{
	return this->Last_reset_wd;
}

void WDT_t::IRQ(uint32_t IRQ_num)
{
	if(IRQ_num == WDT_INT_ID)
	{
		printf("Oh NO! WatchDog Expired!\n\r");
		XScuWdt_Stop(this->WdtInstancePtr);
		IRQ_CONTROLLER_t::getIRQController()->DisableInterrupt(WDT_INT_ID);
		CONTROLLER_t::get_CONTROLLER()->Jump_to_Addr(WDT_RESET_ADDR);
	}
}

void WDT_t::void_callback(void* Intf_ptr, void* parameters)
{
	this->Reset();
}
