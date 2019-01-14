/*
 * uavCAN.hpp
 *
 *  Created on: 11.01.2019.
 *      Author: Stalker1290
 */

#ifndef UAVCAN_HPP_
#define UAVCAN_HPP_

#pragma once

#include <uavcan/driver/can.hpp>
#include <jbuavcan/clock.hpp>
#include "chip.h"
#include "IRQ_Controller.hpp"
#include "Defines.h"
#include "xcanps.h"
#include "Ring_buf.hpp"

namespace jbuavcan
{

#ifdef CAN_TX_QUERY_SIZE
	#if !(CAN_TX_QUERY_SIZE && !(CAN_TX_QUERY_SIZE & (CAN_TX_QUERY_SIZE - 1)))
		#error "Size of CAN TX Query must be a power of 2!"
	#endif
#endif

#ifdef CAN_RX_QUERY_SIZE
	#if !(CAN_RX_QUERY_SIZE && !(CAN_RX_QUERY_SIZE & (CAN_RX_QUERY_SIZE - 1)))
		#error "Size of CAN RX Query must be a power of 2!"
	#endif
#endif


#pragma pack(push, 1)

typedef struct CanFrame_struct{
	uint32_t idr;
	uint32_t dlcr;
	uint8_t data[8];
	uint64_t timestamp;
}CanFrame_t;

#pragma pack(pop)

class CAN_t:public IRQ_LISTENER_t,public uavcan::ICanDriver, public uavcan::ICanIface{
public:
	static CAN_t* getCan(uint8_t num);
	void initialize(uint32_t speedBs, jbuavcan::SystemClock* systemClockPtr);
	void deinitialize(void);
	void pushToTxQuery(uint32_t mesId, const uint8_t* dataPtr, const uint8_t dataSize, bool extId);
	uint8_t pullFromRxQuery(uint32_t* mesId, uint8_t* dataPtr);
	uint8_t pullFromRxQuery(uint32_t* mesId, uint8_t* dataPtr, bool* extId);
	uint8_t pullFromRxQuery(uint32_t* mesId, uint8_t* dataPtr, bool* extId, uint64_t* timestamp);
	virtual void IRQ(uint32_t IRQ_num);

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
private:
	static CAN_t* canInstancePtr[CAN_NUM_DEF];
	static uint32_t canIntrIds[CAN_NUM_DEF];
	static uint32_t canInterruptPriors[CAN_NUM_DEF];
	static uint32_t canDevIds[CAN_NUM_DEF];
	static uint32_t canClocks[CAN_NUM_DEF];
	static XCanPs canInstanceXil[CAN_NUM_DEF];


	CAN_t(uint8_t num);
	void setTimingFields(uint32_t speedBs);
	void errorHandler(uint32_t errorStatus);
	void eventHandler(uint32_t eventIntr);
	void recvHandler(void);
	void sendHandler(void);

	uint8_t num;
	ring_buf_t* txRingBufPtr;
	ring_buf_t* rxRingBufPtr;
	XCanPs* canInstanceXilPtr;
	uint8_t ini;
	uint8_t prescaler;
	uint8_t syncJumpWidth;
	uint8_t timeSegment2;
	uint8_t timeSegment1;
	uint32_t speedBs;
	jbuavcan::SystemClock* systemClockPtr;
	uint32_t rxQueryOverflowCnt;
	uint32_t txQueryOverflowCnt;
};

}

#endif /* UAVCAN_HPP_ */
