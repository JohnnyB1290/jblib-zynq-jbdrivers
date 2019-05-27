/*
 * GPTimers.cpp
 *
 *  Created on: 19 но€б. 2018 г.
 *      Author: Stalker1290
 */

#include "GPTimers.hpp"

GPTimer_t* GPTimer_t::gpTimerPtrs[GPTIMER_NUM_INSTANCES] = GPTIMER_PTRS_DEF;

uint32_t GPTimer_t::timersBaseAddrs[GPTIMER_NUM_INSTANCES] = GPTIMER_BASE_ADDRS_DEF;
uint32_t GPTimer_t::timersIntrIDs[GPTIMER_NUM_INSTANCES] = GPTIMER_INTR_IDS_DEF;
uint32_t GPTimer_t::timersIntrPriorities[GPTIMER_NUM_INSTANCES] = GPTIMER_INT_PRIORITIES_DEF;
uint32_t GPTimer_t::timersClocks[GPTIMER_NUM_INSTANCES] = GPTIMER_CLOCKS_DEF;

GPTimer_t* GPTimer_t::get_GPTimer(uint8_t timerNum){
	if(timerNum < GPTIMER_NUM_INSTANCES){
		if(GPTimer_t::gpTimerPtrs[timerNum] == (GPTimer_t*)NULL)
			GPTimer_t::gpTimerPtrs[timerNum] = new GPTimer_t(timerNum);
		return GPTimer_t::gpTimerPtrs[timerNum];
	}
	else return (GPTimer_t*)NULL;
}

GPTimer_t::GPTimer_t(uint8_t timerNum):IRQ_LISTENER_t(){
	this->timerNum = timerNum;

	for(int i = 0; i < Num_of_matches; i++)
		this->matchCallbackIntrfPtrs[i] = (Callback_Interface_t*)NULL;

	IRQ_CONTROLLER_t::getIRQController()->Add_Peripheral_IRQ_Listener(this,
			GPTimer_t::timersIntrIDs[this->timerNum]);
}

void GPTimer_t::IRQ(uint32_t IRQ_num){
	static uint8_t matchNum;
	uint32_t reg;

	for(uint32_t i = 0; i < Num_of_matches; i++){
		matchNum = i;
		reg  = Xil_In32(GPTimer_t::timersBaseAddrs[this->timerNum] + TRR_REG_OFFSET);
		if(reg & (1 << matchNum)){
			Xil_Out32(GPTimer_t::timersBaseAddrs[this->timerNum] + TRST_REG_OFFSET, (1 << matchNum));
			if(this->matchCallbackIntrfPtrs[matchNum] != (Callback_Interface_t*)NULL)
				this->matchCallbackIntrfPtrs[matchNum]->void_callback((void*)this, (void*)&matchNum);
		}
	}
	reg = Xil_In32(GPTimer_t::timersBaseAddrs[this->timerNum] + TCSR_REG_OFFSET);
	reg |= XTC_CSR_INT_OCCURED_MASK;
	Xil_Out32(GPTimer_t::timersBaseAddrs[this->timerNum] + TCSR_REG_OFFSET, reg);
}

void GPTimer_t::Initialize(void){

	Xil_Out32(GPTimer_t::timersBaseAddrs[this->timerNum] + TCSR_REG_OFFSET,
			XTC_CSR_ENABLE_INT_MASK | XTC_CSR_AUTO_RELOAD_MASK);
	Xil_Out32(GPTimer_t::timersBaseAddrs[this->timerNum] + TPER_REG_OFFSET, 0xFFFFFFFF);

	IRQ_CONTROLLER_t::getIRQController()->SetPriority(GPTimer_t::timersIntrIDs[this->timerNum],
			GPTimer_t::timersIntrPriorities[this->timerNum]);
	IRQ_CONTROLLER_t::getIRQController()->EnableInterrupt(GPTimer_t::timersIntrIDs[this->timerNum]);
}

void GPTimer_t::Start(void){
	uint32_t reg = Xil_In32(GPTimer_t::timersBaseAddrs[this->timerNum] + TCSR_REG_OFFSET);
	reg |= XTC_CSR_ENABLE_TMR_MASK;
	Xil_Out32(GPTimer_t::timersBaseAddrs[this->timerNum] + TCSR_REG_OFFSET, reg);
}

void GPTimer_t::Stop(void){
	uint32_t reg = Xil_In32(GPTimer_t::timersBaseAddrs[this->timerNum] + TCSR_REG_OFFSET);
	reg &= ~XTC_CSR_ENABLE_TMR_MASK;
	Xil_Out32(GPTimer_t::timersBaseAddrs[this->timerNum] + TCSR_REG_OFFSET, reg);
}

void GPTimer_t::Reset(void){
	this->SetCounter(0);
}

void GPTimer_t::SetMatch(uint8_t matchNum, uint32_t us,bool resetOnMatch){
	uint32_t ticks;
	ticks = (GPTimer_t::timersClocks[this->timerNum] / 1000000) * us;
	this->SetMatch_ticks(matchNum, ticks, resetOnMatch);
}

void GPTimer_t::SetMatch_ticks(uint8_t matchNum, uint32_t ticks,bool resetOnMatch){
	uint32_t reg = 0;
	if(matchNum < Num_of_matches){

		Xil_Out32(GPTimer_t::timersBaseAddrs[this->timerNum] + MATCH0_REG_OFFSET + matchNum * 4, ticks);
		reg = Xil_In32(GPTimer_t::timersBaseAddrs[this->timerNum] + TER_REG_OFFSET);
		reg |= (1 << matchNum);
		Xil_Out32(GPTimer_t::timersBaseAddrs[this->timerNum] + TER_REG_OFFSET, reg);
	}
}

void GPTimer_t::DeleteMatch(uint8_t matchNum){
	uint32_t reg = 0;
	reg = Xil_In32(GPTimer_t::timersBaseAddrs[this->timerNum] + TER_REG_OFFSET);
	reg &= ~(1 << matchNum);
	Xil_Out32(GPTimer_t::timersBaseAddrs[this->timerNum] + TER_REG_OFFSET, reg);
}

void GPTimer_t::Deinitialize(void){
	uint32_t reg;

	this->Stop();
	this->Reset();

	reg = Xil_In32(GPTimer_t::timersBaseAddrs[this->timerNum] + TCSR_REG_OFFSET);
	reg |= XTC_CSR_INT_OCCURED_MASK;
	Xil_Out32(GPTimer_t::timersBaseAddrs[this->timerNum] + TCSR_REG_OFFSET, reg);

	Xil_Out32(GPTimer_t::timersBaseAddrs[this->timerNum] + TCSR_REG_OFFSET, 0);
	Xil_Out32(GPTimer_t::timersBaseAddrs[this->timerNum] + TRST_REG_OFFSET, 0xFFFFFFFF);
	Xil_Out32(GPTimer_t::timersBaseAddrs[this->timerNum] + TER_REG_OFFSET, 0);

	IRQ_CONTROLLER_t::getIRQController()->DisableInterrupt(GPTimer_t::timersIntrIDs[this->timerNum]);

	for(uint32_t i = 0; i < Num_of_matches; i++)
		this->matchCallbackIntrfPtrs[i] = NULL;
}

void GPTimer_t::SetMatch_call(uint8_t matchNum,Callback_Interface_t* matchCallback){
	this->matchCallbackIntrfPtrs[matchNum] = matchCallback;
}

void GPTimer_t::DeleteMatch_call(uint8_t matchNum){
	this->matchCallbackIntrfPtrs[matchNum] = NULL;
}

uint32_t GPTimer_t::GetCounter(void){
	return Xil_In32(GPTimer_t::timersBaseAddrs[this->timerNum] + TCR_REG_OFFSET);
}

void GPTimer_t::SetCounter(uint32_t count){
	uint32_t reg;

	Xil_Out32(GPTimer_t::timersBaseAddrs[this->timerNum] + TLR_REG_OFFSET, count);
	reg = Xil_In32(GPTimer_t::timersBaseAddrs[this->timerNum] + TCSR_REG_OFFSET);
	reg |= XTC_CSR_LOAD_MASK;
	Xil_Out32(GPTimer_t::timersBaseAddrs[this->timerNum] + TCSR_REG_OFFSET, reg);
	reg &= ~XTC_CSR_LOAD_MASK;
	Xil_Out32(GPTimer_t::timersBaseAddrs[this->timerNum] + TCSR_REG_OFFSET, reg);
}

