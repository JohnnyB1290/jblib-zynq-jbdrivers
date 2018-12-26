/*
 * CAN.cpp
 *
 *  Created on: 14 дек. 2018 г.
 *      Author: Stalker1290
 */

#include "CAN.hpp"
#include "string.h"
#ifdef USE_CONSOLE
#include "stdio.h"
#endif

uint32_t CAN_t::canIntrIds[CAN_NUM_DEF] = CAN_INT_IDS_DEF;
uint32_t CAN_t::canInterruptPriors[CAN_NUM_DEF] = CAN_INT_PRIORITIES_DEF;
uint32_t CAN_t::canDevIds[CAN_NUM_DEF] = CAN_DEVICE_IDS_DEF;
uint32_t CAN_t::canClocks[CAN_NUM_DEF] = CAN_CLOCKS_DEF;
CAN_t* CAN_t::canInstancePtr[CAN_NUM_DEF] = CAN_VOID_PTR_DEF;
XCanPs CAN_t::canInstanceXil[CAN_NUM_DEF];

CAN_t* CAN_t::getCan(uint8_t num){
	if(num >= CAN_NUM_DEF) return NULL;
	if(CAN_t::canInstancePtr[num] == NULL)
		CAN_t::canInstancePtr[num] = new CAN_t(num);
	return CAN_t::canInstancePtr[num];
}

CAN_t::CAN_t(uint8_t num):IRQ_LISTENER_t(){
	this->num = num;
	this->txRingBufPtr = new ring_buf_t(
			malloc_s(sizeof(CanFrame_t) * CAN_TX_QUERY_SIZE),
			sizeof(CanFrame_t), CAN_TX_QUERY_SIZE);
	this->rxRingBufPtr = new ring_buf_t(
			malloc_s(sizeof(CanFrame_t) * CAN_RX_QUERY_SIZE),
			sizeof(CanFrame_t), CAN_RX_QUERY_SIZE);
	this->canInstanceXilPtr = &CAN_t::canInstanceXil[this->num];
	this->ini = 0;
}

void CAN_t::IRQ(uint32_t IRQ_num){

	uint32_t pendingIntr;
	uint32_t eventIntr;
	uint32_t errorStatus;

	if(this->ini == 0) return;

	pendingIntr = XCanPs_IntrGetStatus(this->canInstanceXilPtr);
	pendingIntr &= XCanPs_IntrGetEnabled(this->canInstanceXilPtr);

	XCanPs_IntrClear(this->canInstanceXilPtr, pendingIntr);

	if ((pendingIntr & XCANPS_IXR_ERROR_MASK) != 0){
		errorStatus = XCanPs_GetBusErrorStatus(this->canInstanceXilPtr);
		this->errorHandler(errorStatus);
		XCanPs_ClearBusErrorStatus(this->canInstanceXilPtr, errorStatus);
	}

	eventIntr = pendingIntr & ((u32)XCANPS_IXR_RXOFLW_MASK |
				(u32)XCANPS_IXR_RXUFLW_MASK |
				(u32)XCANPS_IXR_TXBFLL_MASK |
				(u32)XCANPS_IXR_TXFLL_MASK |
				(u32)XCANPS_IXR_WKUP_MASK |
				(u32)XCANPS_IXR_SLP_MASK |
				(u32)XCANPS_IXR_BSOFF_MASK |
				(u32)XCANPS_IXR_ARBLST_MASK);
	if (eventIntr != (u32)0){
		this->eventHandler(eventIntr);
		if ((eventIntr & XCANPS_IXR_BSOFF_MASK) != (u32)0) return;
	}

	if ((pendingIntr & (XCANPS_IXR_RXFWMFLL_MASK | XCANPS_IXR_RXNEMP_MASK)) != 0)
		this->recvHandler();

	if ((pendingIntr & (XCANPS_IXR_TXOK_MASK | XCANPS_IXR_TXFWMEMP_MASK)) != 0)
		this->sendHandler();
}

void CAN_t::recvHandler(void){
	printf("CAN %i RX handler! \n\r", this->num);
	CanFrame_t frame;
	XCanPs_Recv(this->canInstanceXilPtr, (uint32_t*)&frame);
	this->rxRingBufPtr->Insert(&frame);
}

void CAN_t::sendHandler(void){
	printf("CAN %i TX handler! \n\r", this->num);
	CanFrame_t frame;
	while((XCanPs_IsTxFifoFull(this->canInstanceXilPtr) == FALSE) &&
			(this->txRingBufPtr->Pop(&frame))){
		XCanPs_Send(this->canInstanceXilPtr, (uint32_t*)&frame);
	}
}

void CAN_t::initialize(uint32_t speedKBs){

	if(this->ini == 0)
	{
		int status;
		XCanPs_Config* configPtr;

		this->speedKBs = speedKBs;

		configPtr = XCanPs_LookupConfig(CAN_t::canDevIds[this->num]);
		XCanPs_CfgInitialize(this->canInstanceXilPtr, configPtr, configPtr->BaseAddr);

		status = XCanPs_SelfTest(this->canInstanceXilPtr);
		if (status != XST_SUCCESS) {
			#ifdef USE_CONSOLE
			#ifdef CAN_CONSOLE
			printf("CAN %i self test failure!\n\r", this->num);
			#endif
			#endif
			return;
		}

		XCanPs_EnterMode(this->canInstanceXilPtr, XCANPS_MODE_CONFIG);
		while(XCanPs_GetMode(this->canInstanceXilPtr) != XCANPS_MODE_CONFIG);

		this->setTimingFields(speedKBs);
		XCanPs_SetBaudRatePrescaler(this->canInstanceXilPtr, this->prescaler);
		XCanPs_SetBitTiming(this->canInstanceXilPtr, this->syncJumpWidth,
				this->timeSegment2, this->timeSegment1);

		XCanPs_IntrEnable(this->canInstanceXilPtr, XCANPS_IXR_ALL);

		IRQ_CONTROLLER_t::getIRQController()->Add_Peripheral_IRQ_Listener(this,
				CAN_t::canIntrIds[this->num]);
		IRQ_CONTROLLER_t::getIRQController()->SetPriority(CAN_t::canIntrIds[this->num],
				CAN_t::canInterruptPriors[this->num]);
		IRQ_CONTROLLER_t::getIRQController()->EnableInterrupt(CAN_t::canIntrIds[this->num]);

		XCanPs_EnterMode(this->canInstanceXilPtr, XCANPS_MODE_NORMAL);
		while(XCanPs_GetMode(this->canInstanceXilPtr) != XCANPS_MODE_NORMAL);

		this->ini = 1;
	}
}

void CAN_t::deinitialize(void){
	IRQ_CONTROLLER_t::getIRQController()->DisableInterrupt(CAN_t::canIntrIds[this->num]);
	IRQ_CONTROLLER_t::getIRQController()->Delete_Peripheral_IRQ_Listener(this);
	XCanPs_Reset(this->canInstanceXilPtr);
	this->ini = 0;
}

/*
 * Timing parameters to be set in the Bit Timing Register (BTR).
 * These values are for a 40 Kbps baudrate assuming the CAN input clock
 * frequency is 24 MHz.
 */
#define TEST_BTR_SYNCJUMPWIDTH		3
#define TEST_BTR_SECOND_TIMESEGMENT	2
#define TEST_BTR_FIRST_TIMESEGMENT	15

/*
 * The Baud rate Prescalar value in the Baud Rate Prescaler Register
 * needs to be set based on the input clock  frequency to the CAN core and
 * the desired CAN baud rate.
 * This value is for a 40 Kbps baudrate assuming the CAN input clock frequency
 * is 24 MHz.
 */
#define TEST_BRPR_BAUD_PRESCALAR	29

void CAN_t::setTimingFields(uint32_t speedKBs){
	this->prescaler = TEST_BRPR_BAUD_PRESCALAR;
	this->syncJumpWidth = TEST_BTR_SYNCJUMPWIDTH;
	this->timeSegment2 = TEST_BTR_SECOND_TIMESEGMENT;
	this->timeSegment1 = TEST_BTR_FIRST_TIMESEGMENT;
}



void CAN_t::pushToTxQuery(uint32_t mesId, uint8_t* dataPtr, uint8_t dataSize, bool extId){
	CanFrame_t frame;

	memset(&frame, 0, sizeof(CanFrame_t));

	//StandardId, SubRemoteTransReq, IdExtension, ExtendedId, RemoteTransReq
	if(extId)
		frame.idr = (u32)XCanPs_CreateIdValue(mesId >> 18, 1, 1, mesId, 0);
	else
		frame.idr = (u32)XCanPs_CreateIdValue(mesId, 0, 0, 0, 0);

	frame.dlcr = (dataSize <= 8) ? dataSize : 8;
	memcpy(frame.data, dataPtr, frame.dlcr);

	frame.dlcr = (u32)XCanPs_CreateDlcValue(frame.dlcr);

	if(XCanPs_IntrGetStatus(this->canInstanceXilPtr) & XCANPS_IXR_TXFEMP_MASK){
		XCanPs_Send(this->canInstanceXilPtr, (uint32_t*)&frame);
		XCanPs_IntrClear(this->canInstanceXilPtr, XCANPS_IXR_TXFEMP_MASK);
	}
	else this->txRingBufPtr->Insert((void*)&frame);
}

uint8_t CAN_t::pullFromRxQuery(uint32_t* mesId, uint8_t* dataPtr, bool* extId){
	CanFrame_t frame;
	uint8_t frameSize = 0;

	if(this->rxRingBufPtr->Pop(&frame)){
		if(frame.idr & XCANPS_IDR_IDE_MASK){
			*extId = true;
			*mesId = (((frame.idr & XCANPS_IDR_ID1_MASK) >> XCANPS_IDR_ID1_SHIFT) << 18) |
					((frame.idr & XCANPS_IDR_ID2_MASK) >> XCANPS_IDR_ID2_SHIFT);
		}
		else{
			*extId = false;
			*mesId = (frame.idr & XCANPS_IDR_ID1_MASK) >> XCANPS_IDR_ID1_SHIFT;
		}
		frameSize = (frame.dlcr & XCANPS_DLCR_DLC_MASK) >> XCANPS_DLCR_DLC_SHIFT;

		if(frameSize != 0) memcpy(dataPtr, frame.data, frameSize);

		return frameSize;
	}
	else return 0;
}

void CAN_t::pushToTxQuery(uint32_t mesId, uint8_t* dataPtr, uint8_t dataSize){
	this->pushToTxQuery(mesId, dataPtr, dataSize, false);
}


uint8_t CAN_t::pullFromRxQuery(uint32_t* mesId, uint8_t* dataPtr){
	bool extId;
	return this->pullFromRxQuery(mesId, dataPtr, &extId);
}


void CAN_t::errorHandler(uint32_t errorStatus){
	if(errorStatus & XCANPS_ESR_ACKER_MASK) {
		#ifdef USE_CONSOLE
		#ifdef CAN_CONSOLE
		printf("CAN %i ACK Error!\n\r", this->num);
		#endif
		#endif
	}
	if(errorStatus & XCANPS_ESR_BERR_MASK) {
		#ifdef USE_CONSOLE
		#ifdef CAN_CONSOLE
		printf("CAN %i Bit Error!\n\r", this->num);
		#endif
		#endif
	}
	if(errorStatus & XCANPS_ESR_STER_MASK) {
		#ifdef USE_CONSOLE
		#ifdef CAN_CONSOLE
		printf("CAN %i Stuff Error!\n\r", this->num);
		#endif
		#endif
	}
	if(errorStatus & XCANPS_ESR_FMER_MASK) {
		#ifdef USE_CONSOLE
		#ifdef CAN_CONSOLE
		printf("CAN %i Form Error!\n\r", this->num);
		#endif
		#endif
	}
	if(errorStatus & XCANPS_ESR_CRCER_MASK) {
		#ifdef USE_CONSOLE
		#ifdef CAN_CONSOLE
		printf("CAN %i CRC Error!\n\r", this->num);
		#endif
		#endif
	}
}

void CAN_t::eventHandler(uint32_t eventIntr){
	if (eventIntr & XCANPS_IXR_BSOFF_MASK) {
		#ifdef USE_CONSOLE
		#ifdef CAN_CONSOLE
		printf("CAN %i Bus off Event!\n\r", this->num);
		#endif
		#endif
		this->deinitialize();
		this->initialize(this->speedKBs);
		return;
	}
	if(eventIntr & XCANPS_IXR_RXOFLW_MASK) {
		#ifdef USE_CONSOLE
		#ifdef CAN_CONSOLE
		printf("CAN %i RX FIFO Overflow Event!\n\r", this->num);
		#endif
		#endif
	}

	if(eventIntr & XCANPS_IXR_RXUFLW_MASK) {
		#ifdef USE_CONSOLE
		#ifdef CAN_CONSOLE
		printf("CAN %i RX FIFO Underflow Event!\n\r", this->num);
		#endif
		#endif
	}

	if(eventIntr& XCANPS_IXR_TXBFLL_MASK) {
		#ifdef USE_CONSOLE
		#ifdef CAN_CONSOLE
		printf("CAN %i TX High Priority Buffer Full Event!\n\r", this->num);
		#endif
		#endif
	}

	if(eventIntr & XCANPS_IXR_TXFLL_MASK) {
		#ifdef USE_CONSOLE
		#ifdef CAN_CONSOLE
		printf("CAN %i TX FIFO Full Event!\n\r", this->num);
		#endif
		#endif
	}

	if (eventIntr & XCANPS_IXR_WKUP_MASK) {
		#ifdef USE_CONSOLE
		#ifdef CAN_CONSOLE
		printf("CAN %i Wake up from sleep mode Event!\n\r", this->num);
		#endif
		#endif
	}

	if (eventIntr & XCANPS_IXR_SLP_MASK) {
		#ifdef USE_CONSOLE
		#ifdef CAN_CONSOLE
		printf("CAN %i Enter sleep mode Event!\n\r", this->num);
		#endif
		#endif
	}

	if (eventIntr & XCANPS_IXR_ARBLST_MASK) {
		#ifdef USE_CONSOLE
		#ifdef CAN_CONSOLE
		printf("CAN %i Lost bus arbitration Event!\n\r", this->num);
		#endif
		#endif
	}
}

