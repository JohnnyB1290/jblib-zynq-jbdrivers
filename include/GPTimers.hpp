/*
 * GPTimers.hpp
 *
 *  Created on: 19 но€б. 2018 г.
 *      Author: Stalker1290
 */

#ifndef GPTIMERS_HPP_
#define GPTIMERS_HPP_

#include "chip.h"
#include "IRQ_Controller.hpp"
#include "Defines.h"

#define Num_of_matches 	24

#define TCSR_REG_OFFSET		0x00
#define TLR_REG_OFFSET		0x04
#define TCR_REG_OFFSET		0x08
#define TER_REG_OFFSET		0x0C
#define TRR_REG_OFFSET		0x10
#define TRST_REG_OFFSET		0x14
#define TLST_REG_OFFSET		0x18
#define TPER_REG_OFFSET		0x1C
#define TADD_REG_OFFSET		0x20
#define TEM_REG_OFFSET		0x24
#define MATCH0_REG_OFFSET	0x80
#define CAPT0_REG_OFFSET	0xE0

#define XTC_CSR_CASC_MASK			0x00000800 /**< Cascade Mode */
#define XTC_CSR_ENABLE_ALL_MASK		0x00000400 /**< Enables all timer counters */
#define XTC_CSR_ENABLE_PWM_MASK		0x00000200 /**< Enables the Pulse Width Modulation */
#define XTC_CSR_INT_OCCURED_MASK	0x00000100 /**< If bit is set, an interrupt has occured. If set and '1' is written to this bit position,
							bit is cleared. */
#define XTC_CSR_ENABLE_TMR_MASK		0x00000080 /**< Enables only the specific timer */
#define XTC_CSR_ENABLE_INT_MASK		0x00000040 /**< Enables the interrupt output. */
#define XTC_CSR_LOAD_MASK			0x00000020 /**< Loads the timer using the load value provided earlier in the Load Register, XTC_TLR_OFFSET. */
#define XTC_CSR_AUTO_RELOAD_MASK	0x00000010 /**< In compare mode, configures the timer counter to reload  from the Load Register. The default  mode
							causes the timer counter to hold when the compare value is hit. In capture mode, configures  the timer counter to not
							hold the previous capture value if a new event occurs. The default mode cause the timer counter to hold the capture value
							until recognized. */
#define XTC_CSR_EXT_CAPTURE_MASK	0x00000008 /**< Enables the external input to the timer counter. */
#define XTC_CSR_EXT_GENERATE_MASK	0x00000004 /**< Enables the external generate output for the timer. */
#define XTC_CSR_DOWN_COUNT_MASK		0x00000002 /**< Configures the timer counter to count down from start value, the default is to count up.*/
#define XTC_CSR_CAPTURE_MODE_MASK	0x00000001 /**< Enables the timer to capture the timer counter value when the external capture line is asserted.
							The default mode is compare mode.*/

class GPTimer_t:protected IRQ_LISTENER_t
{
public:
	static GPTimer_t* get_GPTimer(uint8_t timerNum);
	void Initialize(void);
	void Start(void);
	void Stop(void);
	void Reset(void);
	void SetMatch(uint8_t matchNum, uint32_t us,bool resetOnMatch);
	void SetMatch_ticks(uint8_t matchNum, uint32_t ticks,bool resetOnMatch);
	void DeleteMatch(uint8_t matchNum);
	void Deinitialize(void);
	void SetMatch_call(uint8_t matchNum,Callback_Interface_t* matchCallback);
	void DeleteMatch_call(uint8_t matchNum);
	uint32_t GetCounter(void);
	void SetCounter(uint32_t count);
private:
	static GPTimer_t* gpTimerPtrs[GPTIMER_NUM_INSTANCES];

	static uint32_t timersBaseAddrs[GPTIMER_NUM_INSTANCES];
	static uint32_t timersIntrIDs[GPTIMER_NUM_INSTANCES];
	static uint32_t timersIntrPriorities[GPTIMER_NUM_INSTANCES];
	static uint32_t timersClocks[GPTIMER_NUM_INSTANCES];

	GPTimer_t(uint8_t timerNum);
	virtual void IRQ(uint32_t IRQ_num);
	uint8_t timerNum;
	Callback_Interface_t* matchCallbackIntrfPtrs[Num_of_matches];
};

#endif /* GPTIMERS_HPP_ */
