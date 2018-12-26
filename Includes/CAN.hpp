/*
 * CAN.hpp
 *
 *  Created on: 14 дек. 2018 г.
 *      Author: Stalker1290
 */

#ifndef CAN_HPP_
#define CAN_HPP_

#include "chip.h"
#include "IRQ_Controller.hpp"
#include "Defines.h"
#include "xcanps.h"
#include "Ring_buf.hpp"

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
}CanFrame_t;

#pragma pack(pop)

class CAN_t:public IRQ_LISTENER_t{
public:
	static CAN_t* getCan(uint8_t num);
	void initialize(uint32_t speedKBs);
	void deinitialize(void);
	void pushToTxQuery(uint32_t mesId, uint8_t* dataPtr, uint8_t dataSize);
	void pushToTxQuery(uint32_t mesId, uint8_t* dataPtr, uint8_t dataSize, bool extId);
	uint8_t pullFromRxQuery(uint32_t* mesId, uint8_t* dataPtr);
	uint8_t pullFromRxQuery(uint32_t* mesId, uint8_t* dataPtr, bool* extId);
	virtual void IRQ(uint32_t IRQ_num);
private:
	static CAN_t* canInstancePtr[CAN_NUM_DEF];
	static uint32_t canIntrIds[CAN_NUM_DEF];
	static uint32_t canInterruptPriors[CAN_NUM_DEF];
	static uint32_t canDevIds[CAN_NUM_DEF];
	static uint32_t canClocks[CAN_NUM_DEF];
	static XCanPs canInstanceXil[CAN_NUM_DEF];

	CAN_t(uint8_t num);
	void setTimingFields(uint32_t speedKBs);
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
	uint32_t speedKBs;
};


#endif /* CAN_HPP_ */
