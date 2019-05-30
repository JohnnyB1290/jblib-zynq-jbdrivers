/**
 * @file
 * @brief CAN Driver Realization
 *
 *
 * @note
 * Copyright © 2019 Evgeniy Ivanov. Contacts: <strelok1290@gmail.com>
 * All rights reserved.
 * @note
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 * @note
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @note
 * This file is a part of JB_Lib.
 */

// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "jb_common.h"
#if JBDRIVERS_USE_CAN
#include <string.h>
#include "Can.hpp"
#if (USE_CONSOLE && CAN_USE_CONSOLE)
#include "stdio.h"
#endif

namespace jblib
{
namespace jbdrivers
{

using namespace jbutilities;

uint32_t Can::interruptIds_[CAN_NUM_INSTANCES] = CAN_INTERRUPT_IDS;
uint32_t Can::interruptPriorities_[CAN_NUM_INSTANCES] = CAN_INTERRUPT_PRIORITIES;
uint32_t Can::deviceIds_[CAN_NUM_INSTANCES] = CAN_DEVICE_IDS;
uint32_t Can::clocks_[CAN_NUM_INSTANCES] = CAN_CLOCKS;
Can* Can::cans_[CAN_NUM_INSTANCES] = {NULL};
XCanPs Can::xCanPs_[CAN_NUM_INSTANCES];



Can* Can::getCan(uint8_t number)
{
	if(number >= CAN_NUM_INSTANCES)
		return NULL;
	if(cans_[number] == NULL)
		cans_[number] = new Can(number);
	return cans_[number];
}



Can::Can(uint8_t number) : IIrqListener()
{
	this->number_ = number;
	this->txRingBuffer_ = new RingBuffer(
			malloc_s(sizeof(CanFrame_t) * CAN_TX_QUEUE_SIZE),
			sizeof(CanFrame_t), CAN_TX_QUEUE_SIZE);
	this->rxRingBuffer_ = new RingBuffer(
			malloc_s(sizeof(CanFrame_t) * CAN_RX_QUEUE_SIZE),
			sizeof(CanFrame_t), CAN_RX_QUEUE_SIZE);
	this->xCanPsPtr_ = &Can::xCanPs_[this->number_];
}



void Can::irqHandler(uint32_t irqNumber)
{
	if(!this->isInitialized_)
		return;
	uint32_t pendingIntr = XCanPs_IntrGetStatus(this->xCanPsPtr_);
	pendingIntr &= XCanPs_IntrGetEnabled(this->xCanPsPtr_);

	XCanPs_IntrClear(this->xCanPsPtr_, pendingIntr);

	if ((pendingIntr & XCANPS_IXR_ERROR_MASK) != 0) {
		uint32_t errorStatus = XCanPs_GetBusErrorStatus(this->xCanPsPtr_);
		this->errorHandler(errorStatus);
		XCanPs_ClearBusErrorStatus(this->xCanPsPtr_, errorStatus);
	}

	uint32_t eventIntr = pendingIntr & ((u32)XCANPS_IXR_RXOFLW_MASK |
				(u32)XCANPS_IXR_RXUFLW_MASK |
				(u32)XCANPS_IXR_TXBFLL_MASK |
				(u32)XCANPS_IXR_TXFLL_MASK |
				(u32)XCANPS_IXR_WKUP_MASK |
				(u32)XCANPS_IXR_SLP_MASK |
				(u32)XCANPS_IXR_BSOFF_MASK |
				(u32)XCANPS_IXR_ARBLST_MASK);
	if (eventIntr != (u32)0){
		this->eventHandler(eventIntr);
		if ((eventIntr & XCANPS_IXR_BSOFF_MASK) != (u32)0)
			return;
	}

	if ((pendingIntr & (XCANPS_IXR_RXFWMFLL_MASK | XCANPS_IXR_RXNEMP_MASK)) != 0)
		this->recvHandler();

	if ((pendingIntr & (XCANPS_IXR_TXOK_MASK | XCANPS_IXR_TXFWMEMP_MASK)) != 0)
		this->sendHandler();
}



void Can::recvHandler(void)
{
	CanFrame_t frame;
	XCanPs_Recv(this->xCanPsPtr_, (uint32_t*)&frame);
	this->rxRingBuffer_->insert(&frame);
}



void Can::sendHandler(void)
{
	CanFrame_t frame;
	while((XCanPs_IsTxFifoFull(this->xCanPsPtr_) == FALSE) &&
			(this->txRingBuffer_->pop(&frame))){
		XCanPs_Send(this->xCanPsPtr_, (uint32_t*)&frame);
	}
}



void Can::initialize(uint32_t speedBitS)
{
	if(!this->isInitialized_) {

		this->speedBitS_ = speedBitS;

		XCanPs_Config* config = XCanPs_LookupConfig(deviceIds_[this->number_]);
		XCanPs_CfgInitialize(this->xCanPsPtr_, config, config->BaseAddr);

		int status = XCanPs_SelfTest(this->xCanPsPtr_);
		if (status != XST_SUCCESS) {
			#if (USE_CONSOLE && CAN_USE_CONSOLE)
			printf("CAN %i Error: self test failure!\r\n", this->number_);
			#endif
			return;
		}

		XCanPs_EnterMode(this->xCanPsPtr_, XCANPS_MODE_CONFIG);
		while(XCanPs_GetMode(this->xCanPsPtr_) != XCANPS_MODE_CONFIG);

		this->setTimingFields(speedBitS);
		XCanPs_SetBaudRatePrescaler(this->xCanPsPtr_, this->prescaler_);
		XCanPs_SetBitTiming(this->xCanPsPtr_, this->syncJumpWidth_,
				this->timeSegment2_, this->timeSegment1_);

		XCanPs_IntrEnable(this->xCanPsPtr_, XCANPS_IXR_ALL);

		IrqController::getIrqController()->addPeripheralIrqListener(this,
				interruptIds_[this->number_]);
		IrqController::getIrqController()->setPriority(interruptIds_[this->number_],
				interruptPriorities_[this->number_]);
		IrqController::getIrqController()->enableInterrupt(interruptIds_[this->number_]);

		XCanPs_EnterMode(this->xCanPsPtr_, XCANPS_MODE_NORMAL);
		while(XCanPs_GetMode(this->xCanPsPtr_) != XCANPS_MODE_NORMAL);

		this->isInitialized_ = true;
	}
}



void Can::deinitialize(void)
{
	IrqController::getIrqController()->disableInterrupt(interruptIds_[this->number_]);
	IrqController::getIrqController()->deletePeripheralIrqListener(this);
	XCanPs_Reset(this->xCanPsPtr_);
	this->isInitialized_ = false;
}



void Can::setTimingFields(uint32_t speedBitS)
{
    static const int MaxBS1 = 16;
    static const int MaxBS2 = 8;

	if(speedBitS < 1000)
    	return;
    /*
     * Ref. "Automatic Baudrate Detection in CANopen Networks", U. Koppe, MicroControl GmbH & Co. KG
     *      CAN in Automation, 2003
     *
     * According to the source, optimal quanta per bit are:
     *   Bitrate        Optimal Maximum
     *   1000 kbps      8       10
     *   500  kbps      16      17
     *   250  kbps      16      17
     *   125  kbps      16      17
     */
    const int max_quanta_per_bit = (speedBitS >= 1000000) ? 10 : 17;

    if(max_quanta_per_bit > (MaxBS1 + MaxBS2)) return;

    static const int MaxSamplePointLocationPermill = 900;

    /*
     * Computing (prescaler_ * BS):
     *   BITRATE = 1 / (PRESCALER * (1 / PCLK) * (1 + BS1 + BS2))       -- See the Reference Manual
     *   BITRATE = PCLK / (PRESCALER * (1 + BS1 + BS2))                 -- Simplified
     * let:
     *   BS = 1 + BS1 + BS2                                             -- Number of time quanta per bit
     *   PRESCALER_BS = PRESCALER * BS
     * ==>
     *   PRESCALER_BS = PCLK / BITRATE
     */
    const uint32_t prescaler_bs = Can::clocks_[this->number_] / speedBitS;

    /*
     * Searching for such prescaler_ value so that the number of quanta per bit is highest.
     */
    uint8_t bs1_bs2_sum = (uint8_t)(max_quanta_per_bit - 1);

    while ((prescaler_bs % (1 + bs1_bs2_sum)) != 0)
    {
        if (bs1_bs2_sum <= 2) return;
        bs1_bs2_sum--;
    }

    const uint32_t prescaler_ = prescaler_bs / (1 + bs1_bs2_sum);
    if ((prescaler_ < 1U) || (prescaler_ > 256U)) return;

    /*
     * Now we have a constraint: (BS1 + BS2) == bs1_bs2_sum.
     * We need to find such values so that the sample point is as close as possible to the optimal value,
     * which is 87.5%, which is 7/8.
     *
     *   Solve[(1 + bs1)/(1 + bs1 + bs2) == 7/8, bs2]  (* Where 7/8 is 0.875, the recommended sample point location *)
     *   {{bs2 -> (1 + bs1)/7}}
     *
     * Hence:
     *   bs2 = (1 + bs1) / 7
     *   bs1 = (7 * bs1_bs2_sum - 1) / 8
     *
     * Sample point location can be computed as follows:
     *   Sample point location = (1 + bs1) / (1 + bs1 + bs2)
     *
     * Since the optimal solution is so close to the maximum, we prepare two solutions, and then pick the best one:
     *   - With rounding to nearest
     *   - With rounding to zero
     */
    uint8_t bs1 = (uint8_t)(((7 * bs1_bs2_sum - 1) + 4) / 8);       // Trying rounding to nearest first
    uint8_t bs2 = (uint8_t)(bs1_bs2_sum - bs1);

    if(bs1_bs2_sum <= bs1) return;

    {
        const uint16_t sample_point_permill = (uint16_t)(1000 * (1 + bs1) / (1 + bs1 + bs2));

        if (sample_point_permill > MaxSamplePointLocationPermill)   // Strictly more!
        {
            bs1 = (uint8_t)((7 * bs1_bs2_sum - 1) / 8);             // Nope, too far; now rounding to zero
            bs2 = (uint8_t)(bs1_bs2_sum - bs1);
        }
    }

    const bool valid = (bs1 >= 1) && (bs1 <= MaxBS1) && (bs2 >= 1) && (bs2 <= MaxBS2);

    /*
     * Final validation
     * Helpful Python:
     * def sample_point_from_btr(x):
     *     assert 0b0011110010000000111111000000000 & x == 0
     *     ts2,ts1,brp = (x>>20)&7, (x>>16)&15, x&511
     *     return (1+ts1+1)/(1+ts1+1+ts2+1)
     */
    if ((speedBitS != (clocks_[this->number_] / (prescaler_ * (1 + bs1 + bs2)))) ||
        !valid) return;

	this->prescaler_ = (prescaler_ - 1);
	this->syncJumpWidth_ = 1; // One is recommended by UAVCAN, CANOpen, and DeviceNet
	this->timeSegment2_ = bs2 - 1;
	this->timeSegment1_ = bs1 - 1;

	#if (USE_CONSOLE && CAN_USE_CONSOLE)
	printf("CAN %i Set Timings OK: prescaler_ = %i, SJW = %i, TS1 = %i, "
			"TS2 = %i\r\n", this->number_, this->prescaler_, this->syncJumpWidth_,
			this->timeSegment1_, this->timeSegment2_);
	#endif
}



void Can::pushToTxQueue(uint32_t id, uint8_t* data, uint8_t size, bool extId)
{
	CanFrame_t frame;
	memset(&frame, 0, sizeof(CanFrame_t));
	//StandardId, SubRemoteTransReq, IdExtension, ExtendedId, RemoteTransReq
	if(extId)
		frame.idr = (u32)XCanPs_CreateIdValue(id >> 18, 1, 1, id, 0);
	else
		frame.idr = (u32)XCanPs_CreateIdValue(id, 0, 0, 0, 0);

	frame.dlcr = (size <= 8) ? size : 8;
	memcpy(frame.data, data, frame.dlcr);

	frame.dlcr = (u32)XCanPs_CreateDlcValue(frame.dlcr);

	if(XCanPs_IntrGetStatus(this->xCanPsPtr_) & XCANPS_IXR_TXFEMP_MASK){
		XCanPs_Send(this->xCanPsPtr_, (uint32_t*)&frame);
		XCanPs_IntrClear(this->xCanPsPtr_, XCANPS_IXR_TXFEMP_MASK);
	}
	else this->txRingBuffer_->insert((void*)&frame);
}



uint8_t Can::pullFromRxQueue(uint32_t* id, uint8_t* data, bool* extId)
{
	CanFrame_t frame;
	if(this->rxRingBuffer_->pop(&frame)) {
		if(frame.idr & XCANPS_IDR_IDE_MASK){
			*extId = true;
			*id = (((frame.idr & XCANPS_IDR_ID1_MASK) >> XCANPS_IDR_ID1_SHIFT) << 18) |
					((frame.idr & XCANPS_IDR_ID2_MASK) >> XCANPS_IDR_ID2_SHIFT);
		}
		else{
			*extId = false;
			*id = (frame.idr & XCANPS_IDR_ID1_MASK) >> XCANPS_IDR_ID1_SHIFT;
		}
		uint8_t frameSize = (frame.dlcr & XCANPS_DLCR_DLC_MASK) >> XCANPS_DLCR_DLC_SHIFT;
		if(frameSize != 0)
			memcpy(data, frame.data, frameSize);
		return frameSize;
	}
	else return 0;
}



void Can::pushToTxQueue(uint32_t id, uint8_t* data, uint8_t size)
{
	this->pushToTxQueue(id, data, size, false);
}



uint8_t Can::pullFromRxQueue(uint32_t* id, uint8_t* data)
{
	bool extId;
	return this->pullFromRxQueue(id, data, &extId);
}



void Can::errorHandler(uint32_t errorStatus)
{
	if(errorStatus & XCANPS_ESR_ACKER_MASK) {
		#if (USE_CONSOLE && CAN_USE_CONSOLE)
		printf("CAN %i ACK Error!\r\n", this->number_);
		#endif
	}
	if(errorStatus & XCANPS_ESR_BERR_MASK) {
		#if (USE_CONSOLE && CAN_USE_CONSOLE)
		printf("CAN %i Bit Error!\r\n", this->number_);
		#endif
	}
	if(errorStatus & XCANPS_ESR_STER_MASK) {
		#if (USE_CONSOLE && CAN_USE_CONSOLE)
		printf("CAN %i Stuff Error!\r\n", this->number_);
		#endif
	}
	if(errorStatus & XCANPS_ESR_FMER_MASK) {
		#if (USE_CONSOLE && CAN_USE_CONSOLE)
		printf("CAN %i Form Error!\r\n", this->number_);
		#endif
	}
	if(errorStatus & XCANPS_ESR_CRCER_MASK) {
		#if (USE_CONSOLE && CAN_USE_CONSOLE)
		printf("CAN %i CRC Error!\r\n", this->number_);
		#endif
	}
}




void Can::eventHandler(uint32_t eventIntr)
{
	if (eventIntr & XCANPS_IXR_BSOFF_MASK) {
		#if (USE_CONSOLE && CAN_USE_CONSOLE)
		printf("CAN %i Bus off Event!\r\n", this->number_);
		#endif
		this->deinitialize();
		this->initialize(this->speedBitS_);
		return;
	}
	if(eventIntr & XCANPS_IXR_RXOFLW_MASK) {
		#if (USE_CONSOLE && CAN_USE_CONSOLE)
		printf("CAN %i RX FIFO Overflow Event!\r\n", this->number_);
		#endif
	}

	if(eventIntr & XCANPS_IXR_RXUFLW_MASK) {
		#if (USE_CONSOLE && CAN_USE_CONSOLE)
		printf("CAN %i RX FIFO Underflow Event!\r\n", this->number_);
		#endif
	}

	if(eventIntr& XCANPS_IXR_TXBFLL_MASK) {
		#if (USE_CONSOLE && CAN_USE_CONSOLE)
		printf("CAN %i TX High Priority Buffer Full Event!\r\n", this->number_);
		#endif
	}

	if(eventIntr & XCANPS_IXR_TXFLL_MASK) {
		#if (USE_CONSOLE && CAN_USE_CONSOLE)
		printf("CAN %i TX FIFO Full Event!\r\n", this->number_);
		#endif
	}

	if (eventIntr & XCANPS_IXR_WKUP_MASK) {
		#if (USE_CONSOLE && CAN_USE_CONSOLE)
		printf("CAN %i Wake up from sleep mode Event!\r\n", this->number_);
		#endif
	}

	if (eventIntr & XCANPS_IXR_SLP_MASK) {
		#if (USE_CONSOLE && CAN_USE_CONSOLE)
		printf("CAN %i Enter sleep mode Event!\r\n", this->number_);
		#endif
	}

	if (eventIntr & XCANPS_IXR_ARBLST_MASK) {
		#if (USE_CONSOLE && CAN_USE_CONSOLE)
		printf("CAN %i Lost bus arbitration Event!\r\n", this->number_);
		#endif
	}
}

}
}

#endif
