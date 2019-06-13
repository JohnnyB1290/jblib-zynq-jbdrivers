/**
 * @file
 * @brief CAN Driver Description
 *
 *
 * @note
 * Copyright � 2019 Evgeniy Ivanov. Contacts: <strelok1290@gmail.com>
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

#ifndef CAN_HPP_
#define CAN_HPP_

#include "jb_common.h"
#include "IrqController.hpp"
#include "xcanps.h"
#include "RingBuffer.hpp"
#if CAN_USE_UAVCAN
#include "uavcan/driver/can.hpp"
#include "SystemClock.hpp"
#endif

#ifdef CAN_TX_QUEUE_SIZE
	#if !(CAN_TX_QUEUE_SIZE && !(CAN_TX_QUEUE_SIZE & (CAN_TX_QUEUE_SIZE - 1)))
		#error "Size of CAN TX Query must be a power of 2!"
	#endif
#endif

#ifdef CAN_RX_QUEUE_SIZE
	#if !(CAN_RX_QUEUE_SIZE && !(CAN_RX_QUEUE_SIZE & (CAN_RX_QUEUE_SIZE - 1)))
		#error "Size of CAN RX Query must be a power of 2!"
	#endif
#endif


namespace jblib
{
namespace jbdrivers
{


using namespace jbutilities;
#if CAN_USE_UAVCAN
using namespace jbuavcan;
using namespace uavcan;
#endif

#pragma pack(push, 1)
typedef struct
{
	uint32_t idr;
	uint32_t dlcr;
	uint8_t data[8];
	#if CAN_USE_UAVCAN
	uint64_t timestamp;
	#endif
}CanFrame_t;
#pragma pack(pop)

#if CAN_USE_UAVCAN
class Can : protected IIrqListener, public ICanDriver, public ICanIface
#else
class Can : protected IIrqListener
#endif
{
public:
	static Can* getCan(uint8_t number);
	#if CAN_USE_UAVCAN
	void initialize(uint32_t speedBitS, SystemClock* systemClock);
	#else
	void initialize(uint32_t speedBitS);
	#endif
	void deinitialize(void);
	void pushToTxQueue(uint32_t id, const uint8_t* data, const uint8_t size);
	void pushToTxQueue(uint32_t id, const uint8_t* data,
			const uint8_t size, bool extId);
	uint8_t pullFromRxQueue(uint32_t* id, uint8_t* data);
	uint8_t pullFromRxQueue(uint32_t* id, uint8_t* data, bool* extId);
	#if CAN_USE_UAVCAN
	uint8_t pullFromRxQueue(uint32_t* id, uint8_t* data,
			bool* extId, uint64_t* timestamp);
	uavcan::int16_t send(const uavcan::CanFrame& frame,
	                         uavcan::MonotonicTime tx_deadline,
	                         uavcan::CanIOFlags flags) override;
	uavcan::int16_t receive(uavcan::CanFrame& out_frame,
							uavcan::MonotonicTime& out_ts_monotonic,
							uavcan::UtcTime& out_ts_utc,
							uavcan::CanIOFlags& out_flags) override;
	uavcan::int16_t select(uavcan::CanSelectMasks& inout_masks,
						   const uavcan::CanFrame* (&)[uavcan::MaxCanIfaces],
						   uavcan::MonotonicTime blocking_deadline) override;
	uavcan::int16_t configureFilters(const uavcan::CanFilterConfig* filter_configs,
									 uavcan::uint16_t num_configs) override;
	uavcan::uint64_t getErrorCount() const override;
	uavcan::uint16_t getNumFilters() const override;
	uavcan::ICanIface* getIface(uavcan::uint8_t iface_index) override;
	uavcan::uint8_t getNumIfaces() const override;
	#endif

private:
	Can(uint8_t number);
	void setTimingFields(uint32_t speedBitS);
	void errorHandler(uint32_t errorStatus);
	void eventHandler(uint32_t eventIntr);
	void recvHandler(void);
	void sendHandler(void);
	virtual void irqHandler(uint32_t irqNumber);

	static Can* cans_[CAN_NUM_INSTANCES];
	static uint32_t interruptIds_[CAN_NUM_INSTANCES];
	static uint32_t interruptPriorities_[CAN_NUM_INSTANCES];
	static uint32_t deviceIds_[CAN_NUM_INSTANCES];
	static uint32_t clocks_[CAN_NUM_INSTANCES];
	static XCanPs xCanPs_[CAN_NUM_INSTANCES];
	uint8_t number_ = 0;
	RingBuffer* txRingBuffer_ = NULL;
	RingBuffer* rxRingBuffer_ = NULL;
	XCanPs* xCanPsPtr_ = NULL;
	bool isInitialized_ = false;
	uint32_t speedBitS_ = 0;
	/*
	 * These values are for a 40 Kbps baudrate assuming the CAN input clock
	 * frequency is 24 MHz.
	 */
	uint8_t prescaler_ = 29;
	uint8_t syncJumpWidth_ = 3;
	uint8_t timeSegment2_ = 2;
	uint8_t timeSegment1_ = 15;

	#if CAN_USE_UAVCAN
	SystemClock* systemClock_ = NULL;
	uint32_t rxQueueOverflowCnt_ = 0;
	uint32_t txQueueOverflowCnt_ = 0;
	#endif
};

}
}

#endif /* CAN_HPP_ */
