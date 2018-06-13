/*
 * Axi_timer.cpp
 *
 *  Created on: 19.02.2018 ã.
 *      Author: Stalker1290
 */

#include "Axi_timer.hpp"


Axi_Timer_void_t* Axi_Timer_void_t::Axi_Timer_void_ptrs[Axi_timer_num] = Axi_Timer_void_ptrs_def;

uint32_t Axi_Timer_void_t::Timers_INTR_ID[Axi_timer_num] = Axi_tmr_intr_IDs_def;
uint32_t Axi_Timer_void_t::Timers_INTR_prioryty[Axi_timer_num] = Axi_tmr_int_prioryties_def;
uint32_t Axi_Timer_void_t::Timers_dev_IDs[Axi_timer_num] = Axi_tmr_dev_IDs_def;


Axi_Timer_void_t* Axi_Timer_void_t::getAxi_Timer_void(uint8_t Timer_num)
{
	if(Axi_Timer_void_t::Axi_Timer_void_ptrs[Timer_num] == (Axi_Timer_void_t*)NULL)
		Axi_Timer_void_t::Axi_Timer_void_ptrs[Timer_num] = new Axi_Timer_void_t(Timer_num);
	return Axi_Timer_void_t::Axi_Timer_void_ptrs[Timer_num];
}

Axi_Timer_void_t::Axi_Timer_void_t(uint8_t Timer_num):VOID_TIMER_t()
{
	this->Timer_num = Timer_num;
	this->callback = (VOID_CALLBACK_t)NULL;
	this->callback_intrf_ptr = (Callback_Interface_t*)NULL;

	XTmrCtr_Initialize(&this->TimerInstance, Axi_Timer_void_t::Timers_dev_IDs[this->Timer_num]);

	IRQ_CONTROLLER_t::getIRQController()->Add_Peripheral_IRQ_Listener(this,
			Axi_Timer_void_t::Timers_INTR_ID[this->Timer_num]);
	IRQ_CONTROLLER_t::getIRQController()->SetPriority(Axi_Timer_void_t::Timers_INTR_ID[this->Timer_num],
			Axi_Timer_void_t::Timers_INTR_prioryty[this->Timer_num]);
	IRQ_CONTROLLER_t::getIRQController()->EnableInterrupt(Axi_Timer_void_t::Timers_INTR_ID[this->Timer_num]);
}

void Axi_Timer_void_t::Timer_Handler(void *CallBackRef, u8 Sub_Tmr_Number)
{
	Axi_Timer_void_t* Void_TMR_ptr = (Axi_Timer_void_t*)CallBackRef;
	if(Sub_Tmr_Number == 0)
	{
		if(Void_TMR_ptr->callback != (VOID_CALLBACK_t)NULL) Void_TMR_ptr->callback();
		else if(Void_TMR_ptr->callback_intrf_ptr != (Callback_Interface_t*)NULL)
			Void_TMR_ptr->callback_intrf_ptr->void_callback(CallBackRef, NULL);
	}
}


void Axi_Timer_void_t::Initialize(uint32_t us)
{
	volatile uint32_t cmp_value;

	XTmrCtr_SetHandler(&this->TimerInstance, Axi_Timer_void_t::Timer_Handler,this);
	XTmrCtr_SetOptions(&this->TimerInstance, 0, XTC_INT_MODE_OPTION | XTC_AUTO_RELOAD_OPTION|XTC_DOWN_COUNT_OPTION);

	cmp_value = (Axi_TMR_CLK_Hz / 1000000) * us;

	XTmrCtr_SetResetValue(&this->TimerInstance, 0, cmp_value-2);
}

void Axi_Timer_void_t::Start(void)
{
	XTmrCtr_Start(&this->TimerInstance, 0);
}

void Axi_Timer_void_t::Stop(void)
{
	XTmrCtr_Stop(&this->TimerInstance, 0);
}

void Axi_Timer_void_t::Reset(void)
{
	XTmrCtr_Reset(&this->TimerInstance, 0);
}

uint32_t Axi_Timer_void_t::GetCounter(void)
{
	return XTmrCtr_GetValue(&this->TimerInstance, 0);
}

void Axi_Timer_void_t::SetCounter(uint32_t count)
{
	XTmrCtr_WriteReg(this->TimerInstance.Config.BaseAddress, 0, XTC_TCR_OFFSET, count);
}

void Axi_Timer_void_t::AddCall(VOID_CALLBACK_t IntCallback)
{
	if(this->callback == (VOID_CALLBACK_t)NULL) this->callback = IntCallback;
}

void Axi_Timer_void_t::AddCall(Callback_Interface_t* IntCallback)
{
	if(this->callback_intrf_ptr == (Callback_Interface_t*)NULL) this->callback_intrf_ptr = IntCallback;
}

void Axi_Timer_void_t::DeleteCall(void)
{
	this->callback = (VOID_CALLBACK_t)NULL;
	this->callback_intrf_ptr = (Callback_Interface_t*)NULL;
}

void Axi_Timer_void_t::Deinitialize(void)
{
	this->Stop();
	IRQ_CONTROLLER_t::getIRQController()->Delete_Peripheral_IRQ_Listener(this);
	this->callback = (VOID_CALLBACK_t)NULL;
	this->callback_intrf_ptr = (Callback_Interface_t*)NULL;
}

void Axi_Timer_void_t::IRQ(uint32_t IRQ_num)
{
	XTmrCtr_InterruptHandler(&this->TimerInstance);
}

