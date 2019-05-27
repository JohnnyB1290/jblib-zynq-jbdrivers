/*
 * PrvtTimerVoid.hpp
 *
 *  Created on: 10 џэт. 2018 у.
 *      Author: Stalker1290
 */

#ifndef PRVTTIMERVOID_HPP_
#define PRVTTIMERVOID_HPP_

#include "VOIDTMR.hpp"
#include "xscutimer.h"

class Prvt_Timer_void_t:public VOID_TIMER_t
{
public:
	static Prvt_Timer_void_t* getPrvt_Timer_void(void);
	virtual void Initialize(uint32_t us);
	virtual void Start(void);
	virtual void Stop(void);
	virtual void Reset(void);
	virtual uint32_t GetCounter(void);
	virtual void SetCounter(uint32_t count);
	virtual void AddCall(VOID_CALLBACK_t IntCallback);
	virtual void AddCall(Callback_Interface_t* IntCallback);
	virtual void DeleteCall(void);
	virtual void Deinitialize(void);
	virtual void IRQ(uint32_t IRQ_num);
private:
	static Prvt_Timer_void_t* Prvt_Tmr_void_ptr;
	Prvt_Timer_void_t(void);
	XScuTimer TimerInstance;
	VOID_CALLBACK_t callback;
	Callback_Interface_t* callback_intrf_ptr;
};

#endif /* ZYNQ_INCLUDES_PRVTTIMERVOID_HPP_ */
