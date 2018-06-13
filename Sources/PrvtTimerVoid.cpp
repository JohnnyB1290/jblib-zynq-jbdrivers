/*
 * PrvtTimerVoid.cpp
 *
 *  Created on: 10 џэт. 2018 у.
 *      Author: Stalker1290
 */

#include "PrvtTimerVoid.hpp"

Prvt_Timer_void_t* Prvt_Timer_void_t::Prvt_Tmr_void_ptr = (Prvt_Timer_void_t*)NULL;

Prvt_Timer_void_t* Prvt_Timer_void_t::getPrvt_Timer_void(void)
{
	if(Prvt_Timer_void_t::Prvt_Tmr_void_ptr == (Prvt_Timer_void_t*)NULL) Prvt_Timer_void_t::Prvt_Tmr_void_ptr = new Prvt_Timer_void_t();
	return Prvt_Timer_void_t::Prvt_Tmr_void_ptr;
}

Prvt_Timer_void_t::Prvt_Timer_void_t(void):VOID_TIMER_t()
{
	this->callback = (VOID_CALLBACK_t)NULL;
	this->callback_intrf_ptr = (Callback_Interface_t*)NULL;
}

void Prvt_Timer_void_t::IRQ(uint32_t IRQ_num)
{
	if(XScuTimer_IsExpired(&this->TimerInstance))
	{
		if(this->callback != (VOID_CALLBACK_t)NULL) this->callback();
		else if(this->callback_intrf_ptr != (Callback_Interface_t*)NULL) this->callback_intrf_ptr->void_callback((void*)this, NULL);
		XScuTimer_ClearInterruptStatus(&this->TimerInstance);
	}
}

void Prvt_Timer_void_t::Initialize(uint32_t us)
{
	volatile uint32_t cmp_value;
	XScuTimer_Config *ConfigPtr;

	ConfigPtr = XScuTimer_LookupConfig(PRIVATE_TIMER_ID);

	XScuTimer_CfgInitialize(&this->TimerInstance, ConfigPtr,ConfigPtr->BaseAddr);

	XScuTimer_EnableAutoReload(&this->TimerInstance);

	cmp_value = (XPAR_CPU_CORTEXA9_0_CPU_CLK_FREQ_HZ / 2000000) * us;

	XScuTimer_LoadTimer(&this->TimerInstance, cmp_value);

	XScuTimer_EnableInterrupt(&this->TimerInstance);

	IRQ_CONTROLLER_t::getIRQController()->Add_Peripheral_IRQ_Listener(this,PRIVATE_TIMER_INTR_ID);
	IRQ_CONTROLLER_t::getIRQController()->SetPriority(PRIVATE_TIMER_INTR_ID,Private_Timer_interrupt_priority);
	IRQ_CONTROLLER_t::getIRQController()->EnableInterrupt(PRIVATE_TIMER_INTR_ID);
}

void Prvt_Timer_void_t::Start(void)
{
	XScuTimer_Start(&this->TimerInstance);
}

void Prvt_Timer_void_t::Stop(void)
{
	XScuTimer_Stop(&this->TimerInstance);
}

void Prvt_Timer_void_t::Reset(void)
{
	XScuTimer_RestartTimer(&this->TimerInstance);
}

uint32_t Prvt_Timer_void_t::GetCounter(void)
{
	return XScuTimer_GetCounterValue(&this->TimerInstance);
}

void Prvt_Timer_void_t::SetCounter(uint32_t count)
{
	XScuTimer_WriteReg(this->TimerInstance.Config.BaseAddr, XSCUTIMER_COUNTER_OFFSET, count);
}

void Prvt_Timer_void_t::AddCall(VOID_CALLBACK_t IntCallback)
{
	if(this->callback == (VOID_CALLBACK_t)NULL) this->callback = IntCallback;
}

void Prvt_Timer_void_t::AddCall(Callback_Interface_t* IntCallback)
{
	if(this->callback_intrf_ptr == (Callback_Interface_t*)NULL) this->callback_intrf_ptr = IntCallback;
}

void Prvt_Timer_void_t::DeleteCall(void)
{
	this->callback = (VOID_CALLBACK_t)NULL;
	this->callback_intrf_ptr = (Callback_Interface_t*)NULL;
}

void Prvt_Timer_void_t::Deinitialize(void)
{
	this->Stop();
	IRQ_CONTROLLER_t::getIRQController()->DisableInterrupt(PRIVATE_TIMER_INTR_ID);
	IRQ_CONTROLLER_t::getIRQController()->Delete_Peripheral_IRQ_Listener(this);
	this->callback = (VOID_CALLBACK_t)NULL;
	this->callback_intrf_ptr = (Callback_Interface_t*)NULL;
}


