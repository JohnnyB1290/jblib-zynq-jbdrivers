/**
 * @file
 * @brief SD Card Driver Realization
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

#include "jbkernel/jb_common.h"
#if JBDRIVERS_USE_SD_CARD
#include <string.h>
#include "jbdrivers/SdCard.hpp"
#include "sleep.h"


#define HIGH_SPEED_SUPPORT					0x01U
#define WIDTH_4_BIT_SUPPORT					0x4U
#define SD_CLK_25_MHZ						25000000U
#define SD_CLK_26_MHZ						26000000U
#define SD_CLK_52_MHZ						52000000U
#define EXT_CSD_DEVICE_TYPE_BYTE			196
#define EXT_CSD_4_BIT_WIDTH_BYTE			183
#define EXT_CSD_HIGH_SPEED_BYTE				185
#define EXT_CSD_DEVICE_TYPE_HIGH_SPEED		0x3
#define SD_CD_DELAY							10000U

namespace jblib
{
namespace jbdrivers
{

using namespace jbfatfs;

SdCard* SdCard::sdCard_ = NULL;

SdCard* SdCard::getSdCard(void)
{
	if(SdCard::sdCard_ == NULL) SdCard::sdCard_ = new SdCard();
	return SdCard::sdCard_;
}

SdCard::SdCard(void)
{
	memset(&this->xSdPs_, 0, sizeof(XSdPs));
	memset(this->extCsdBuffer_, 0, 512);
}



DSTATUS SdCard::diskStatus(void)
{
	DSTATUS s = this->status_;
	if (this->xSdPs_.Config.baseAddress_ == (u32)0) {
		this->baseAddress_ = XPAR_XSDPS_0_BASEADDR;
		this->cardDetect_ = XPAR_XSDPS_0_HAS_CD;
		this->writeProtect_ = XPAR_XSDPS_0_HAS_WP;
		this->hostControllerVersion_ = (u8)(XSdPs_ReadReg16(this->baseAddress_,
				XSDPS_HOST_CTRL_VER_OFFSET) & XSDPS_HC_SPEC_VER_MASK);
		if (this->hostControllerVersion_ == XSDPS_HC_SPEC_V3)
			this->slotType_ = XSdPs_ReadReg(this->baseAddress_,
					XSDPS_CAPS_OFFSET) & XSDPS_CAPS_SLOT_TYPE_MASK;
		else
			this->slotType_ = 0;
	}
	u32 statusReg = XSdPs_GetPresentStatusReg((u32 )this->baseAddress_);
	if (this->slotType_ != XSDPS_CAPS_EMB_SLOT) {
		if (this->cardDetect_) {
			u32 delayCount = 0;
			while ((statusReg & XSDPS_PSR_CARD_INSRT_MASK) == 0U) {
				if (delayCount == 500U) {
					s = STA_NODISK | STA_NOINIT;
					this->status_ = s;
					return s;
				}
				else {
					/* Wait for 10 msec */
					usleep(SD_CD_DELAY);
					delayCount++;
					statusReg = XSdPs_GetPresentStatusReg((u32 )this->baseAddress_);
				}
			}
		}
		s &= ~STA_NODISK;
		if (this->writeProtect_) {
			if ((statusReg & XSDPS_PSR_WPS_PL_MASK) == 0U) {
				s |= STA_PROTECT;
				this->status_ = s;
				return s;
			}
		}
		s &= ~STA_PROTECT;
	}
	else {
		s &= ~STA_NODISK & ~STA_PROTECT;
	}
	this->status_ = s;
	return s;
}



DSTATUS SdCard::diskInitialize(void)
{
	/*
	 * Check if card is in the socket
	 */
	DSTATUS s = this->diskStatus();
	if ((s & STA_NODISK) != 0U)
		return s;
	/* If disk is already initialized */
	if ((s & STA_NOINIT) == 0U)
		return s;
	if (this->cardDetect_) {
		/*
		 * Card detection check
		 * If the HC detects the No Card State, power will be cleared
		 */
		while (!((XSdPs_GetPresentStatusReg(this->baseAddress_) &
				(XSDPS_PSR_CARD_DPL_MASK | XSDPS_PSR_CARD_STABLE_MASK |
						XSDPS_PSR_CARD_INSRT_MASK)) == (XSDPS_PSR_CARD_DPL_MASK |
								XSDPS_PSR_CARD_STABLE_MASK | XSDPS_PSR_CARD_INSRT_MASK)));
	}
	/*
	 * Initialize the host controller
	 */
	XSdPs_Config* sdConfig = XSdPs_LookupConfig((u16)0);
	if (sdConfig == NULL) {
		s |= STA_NOINIT;
		return s;
	}
	s32 status = XSdPs_CfgInitialize(&this->xSdPs_, sdConfig, sdConfig->baseAddress_);
	if (status != XST_SUCCESS) {
		s |= STA_NOINIT;
		return s;
	}
	status = XSdPs_CardInitialize(&this->xSdPs_);
	if (status != XST_SUCCESS){
		s |= STA_NOINIT;
		return s;
	}
	/*
	 * Disk is initialized.
	 * Store the same in status_.
	 */
	s &= (~STA_NOINIT);
	this->status_ = s;
	return s;
}



DRESULT SdCard::diskRead(BYTE* buff, DWORD sector, UINT count)
{
	DWORD locSector = sector;
	DSTATUS s = this->disk_status();

	if ((s & STA_NOINIT) != 0U)
		return RES_NOTRDY;
	if (count == 0U)
		return RES_PARERR;

	/* Convert LBA to byte address if needed */
	if ((this->xSdPs_.HCS) == 0U)
		locSector *= (DWORD) XSDPS_BLK_SIZE_512_MASK;
	s32 status = XSdPs_ReadPolled(&this->xSdPs_, (u32) locSector, count, buff);
	if (status != XST_SUCCESS)
		return RES_ERROR;
	else
		return RES_OK;
}



DRESULT SdCard::diskWrite(const BYTE* buff, DWORD sector, UINT count)
{
	DWORD locSector = sector;
	DSTATUS s = this->disk_status();

	if ((s & STA_NOINIT) != 0U)
		return RES_NOTRDY;
	if (count == 0U)
		return RES_PARERR;
	/* Convert LBA to byte address if needed */
	if ((this->xSdPs_.HCS) == 0U)
		locSector *= (DWORD) XSDPS_BLK_SIZE_512_MASK;
	s32 status = XSdPs_WritePolled(&this->xSdPs_, (u32) locSector, count, buff);
	if (status != XST_SUCCESS)
		return RES_ERROR;
	else
		return RES_OK;
}



DRESULT SdCard::diskIoctl(BYTE cmd, void* buff)
{
	void* locBuff = buff;
	if ((this->disk_status() & STA_NOINIT) != 0U)	/* Check if card is in the socket */
		return RES_NOTRDY;
	DRESULT res = RES_ERROR;
	switch (cmd)
	{
		case (BYTE)CTRL_SYNC :	/* Make sure that no pending write process */
			res = RES_OK;
			break;

		case (BYTE)GET_SECTOR_COUNT : /* Get number of sectors on the disk (DWORD) */
			(*((DWORD *)(void *)locBuff)) = (DWORD)this->xSdPs_.SectorCount;
			res = RES_OK;
			break;

		case GET_SECTOR_SIZE:	/* Get R/W sector size (WORD) */
			(*((DWORD *)(void *)locBuff)) = FF_MIN_SS;
			res = RES_OK;
			break;


		case (BYTE)GET_BLOCK_SIZE :	/* Get erase block size in unit of sector (DWORD) */
			(*((DWORD *)((void *)locBuff))) = ((DWORD)128);
			res = RES_OK;
			break;

		default:
			res = RES_PARERR;
			break;
	}
		return res;
}

}
}


#endif

