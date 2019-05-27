/*
 * Axi_timer.hpp
 *
 *  Created on: 19.02.2018 ã.
 *      Author: Stalker1290
 */

#ifndef SRC_AXI_TIMER_HPP_
#define SRC_AXI_TIMER_HPP_

#include "VOIDTMR.hpp"
#include "xtmrctr.h"


class Axi_Timer_void_t:public VOID_TIMER_t
{
public:
	static Axi_Timer_void_t* getAxi_Timer_void(uint8_t Timer_num);
	virtual void Initialize(uint32_t us);
	virtual void Start(void);
	virtual void Stop(void);
	virtual void Reset(void);
	virtual uint32_t GetCounter(void);
	virtual uint32_t GetUsecCounter(void);
	virtual void SetCounter(uint32_t count);
	virtual void AddCall(Callback_Interface_t* IntCallback);
	virtual void DeleteCall(void);
	virtual void Deinitialize(void);
	virtual void IRQ(uint32_t IRQ_num);
private:
	static void Timer_Handler(void *CallBackRef, u8 Sub_Tmr_Number);
	static Axi_Timer_void_t* Axi_Timer_void_ptrs[Axi_timer_num];

	static uint32_t Timers_INTR_ID[Axi_timer_num];
	static uint32_t Timers_INTR_prioryty[Axi_timer_num];
	static uint32_t Timers_dev_IDs[Axi_timer_num];
	static uint32_t Timers_Clocks[Axi_timer_num];

	Axi_Timer_void_t(uint8_t Timer_num);
	XTmrCtr TimerInstance;
	Callback_Interface_t* callback_intrf_ptr;
	uint8_t Timer_num;
	uint32_t uSecTicks;
	uint32_t cmp_value;
};


#endif /* SRC_AXI_TIMER_H_ */
