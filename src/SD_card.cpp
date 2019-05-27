/*
 * SD_card.cpp
 *
 *  Created on: 27 марта 2018 г.
 *      Author: Stalker1290
 */

#include "SD_card.hpp"
#include "string.h"

#ifdef USE_FS
#include "sleep.h"
#endif

#define HIGH_SPEED_SUPPORT	0x01U
#define WIDTH_4_BIT_SUPPORT	0x4U
#define SD_CLK_25_MHZ		25000000U
#define SD_CLK_26_MHZ		26000000U
#define SD_CLK_52_MHZ		52000000U
#define EXT_CSD_DEVICE_TYPE_BYTE	196
#define EXT_CSD_4_BIT_WIDTH_BYTE	183
#define EXT_CSD_HIGH_SPEED_BYTE		185
#define EXT_CSD_DEVICE_TYPE_HIGH_SPEED	0x3
#define SD_CD_DELAY		10000U


SD_Card_t* SD_Card_t::SDCardInstancePtr = NULL;

SD_Card_t* SD_Card_t::getSDCard(void)
{
	if(SD_Card_t::SDCardInstancePtr == NULL) SD_Card_t::SDCardInstancePtr = new SD_Card_t();
	return SD_Card_t::SDCardInstancePtr;
}

SD_Card_t::SD_Card_t(void)
{
	#ifdef USE_FS
	this->Stat = STA_NOINIT;
	#endif
	memset(&this->SdInstance, 0, sizeof(XSdPs));
	this->BaseAddress = 0;
	this->CardDetect = 0;
	this->WriteProtect = 0;
	this->SlotType = 0;
	this->HostCntrlrVer = 0;
	memset(this->ExtCsd, 0, 512);
}

#ifdef USE_FS

DSTATUS SD_Card_t::disk_status(void)
{
	DSTATUS s = this->Stat;
	u32 StatusReg;
	u32 DelayCount = 0;

	if (this->SdInstance.Config.BaseAddress == (u32)0)
	{
		this->BaseAddress = XPAR_XSDPS_0_BASEADDR;
		this->CardDetect = XPAR_XSDPS_0_HAS_CD;
		this->WriteProtect = XPAR_XSDPS_0_HAS_WP;

		this->HostCntrlrVer = (u8)(XSdPs_ReadReg16(this->BaseAddress, XSDPS_HOST_CTRL_VER_OFFSET) & XSDPS_HC_SPEC_VER_MASK);
		if (this->HostCntrlrVer == XSDPS_HC_SPEC_V3) this->SlotType = XSdPs_ReadReg(this->BaseAddress, XSDPS_CAPS_OFFSET) & XSDPS_CAPS_SLOT_TYPE_MASK;
		else this->SlotType = 0;
	}
	StatusReg = XSdPs_GetPresentStatusReg((u32 )this->BaseAddress);
	if (this->SlotType != XSDPS_CAPS_EMB_SLOT)
	{
		if (this->CardDetect)
		{
			while ((StatusReg & XSDPS_PSR_CARD_INSRT_MASK) == 0U)
			{
				if (DelayCount == 500U)
				{
					s = STA_NODISK | STA_NOINIT;
					goto Label;
				}
				else
				{
					/* Wait for 10 msec */
					usleep(SD_CD_DELAY);
					DelayCount++;
					StatusReg = XSdPs_GetPresentStatusReg((u32 )this->BaseAddress);
				}
			}
		}
		s &= ~STA_NODISK;
		if (this->WriteProtect)
		{
			if ((StatusReg & XSDPS_PSR_WPS_PL_MASK) == 0U)
			{
				s |= STA_PROTECT;
				goto Label;
			}
		}
		s &= ~STA_PROTECT;
	}
	else
	{
		s &= ~STA_NODISK & ~STA_PROTECT;
	}

	Label: this->Stat = s;

	return s;
}

DSTATUS SD_Card_t::disk_initialize(void)
{
	DSTATUS s;
	s32 Status;

	XSdPs_Config *SdConfig;

	/*
	 * Check if card is in the socket
	 */
	s = this->disk_status();
	if ((s & STA_NODISK) != 0U) return s;

	/* If disk is already initialized */
	if ((s & STA_NOINIT) == 0U) return s;

	if (this->CardDetect)
	{
		/*
		 * Card detection check
		 * If the HC detects the No Card State, power will be cleared
		 */
		while (!((XSDPS_PSR_CARD_DPL_MASK | XSDPS_PSR_CARD_STABLE_MASK | XSDPS_PSR_CARD_INSRT_MASK)
				== ( XSdPs_GetPresentStatusReg((u32 )this->BaseAddress)
						& (XSDPS_PSR_CARD_DPL_MASK | XSDPS_PSR_CARD_STABLE_MASK | XSDPS_PSR_CARD_INSRT_MASK))));
	}

	/*
	 * Initialize the host controller
	 */
	SdConfig = XSdPs_LookupConfig((u16)0);
	if (NULL == SdConfig)
	{
		s |= STA_NOINIT;
		return s;
	}

	Status = XSdPs_CfgInitialize(&this->SdInstance, SdConfig, SdConfig->BaseAddress);
	if (Status != XST_SUCCESS)
	{
		s |= STA_NOINIT;
		return s;
	}

	Status = XSdPs_CardInitialize(&this->SdInstance);
	if (Status != XST_SUCCESS)
	{
		s |= STA_NOINIT;
		return s;
	}

	/*
	 * Disk is initialized.
	 * Store the same in Stat.
	 */
	s &= (~STA_NOINIT);

	this->Stat = s;

	return s;
}

DRESULT SD_Card_t::disk_read(BYTE* buff, DWORD sector, UINT count)
{
	DSTATUS s;
	s32 Status;
	DWORD LocSector = sector;

	s = this->disk_status();

	if ((s & STA_NOINIT) != 0U) return RES_NOTRDY;
	if (count == 0U) return RES_PARERR;

	/* Convert LBA to byte address if needed */
	if ((this->SdInstance.HCS) == 0U) LocSector *= (DWORD) XSDPS_BLK_SIZE_512_MASK;

	Status = XSdPs_ReadPolled(&this->SdInstance, (u32) LocSector, count, buff);
	if (Status != XST_SUCCESS) return RES_ERROR;

	return RES_OK;
}

DRESULT SD_Card_t::disk_write(const BYTE* buff, DWORD sector, UINT count)
{
	DSTATUS s;
	s32 Status;
	DWORD LocSector = sector;

	s = this->disk_status();

	if ((s & STA_NOINIT) != 0U)
		return RES_NOTRDY;

	if (count == 0U)
		return RES_PARERR;

	/* Convert LBA to byte address if needed */
	if ((this->SdInstance.HCS) == 0U) LocSector *= (DWORD) XSDPS_BLK_SIZE_512_MASK;

	Status = XSdPs_WritePolled(&this->SdInstance, (u32) LocSector, count, buff);
	if (Status != XST_SUCCESS) return RES_ERROR;

	return RES_OK;
}

DRESULT SD_Card_t::disk_ioctl(BYTE cmd, void* buff)
{
	DRESULT res;
	void *LocBuff = buff;
	if ((this->disk_status() & STA_NOINIT) != 0U) {	/* Check if card is in the socket */
		return RES_NOTRDY;
	}

	res = RES_ERROR;
	switch (cmd) {
		case (BYTE)CTRL_SYNC :	/* Make sure that no pending write process */
			res = RES_OK;
			break;

		case (BYTE)GET_SECTOR_COUNT : /* Get number of sectors on the disk (DWORD) */
			(*((DWORD *)(void *)LocBuff)) = (DWORD)this->SdInstance.SectorCount;
			res = RES_OK;
			break;

		case GET_SECTOR_SIZE:	/* Get R/W sector size (WORD) */
			(*((DWORD *)(void *)LocBuff)) = FF_MIN_SS;
			res = RES_OK;
			break;


		case (BYTE)GET_BLOCK_SIZE :	/* Get erase block size in unit of sector (DWORD) */
			(*((DWORD *)((void *)LocBuff))) = ((DWORD)128);
			res = RES_OK;
			break;

		default:
			res = RES_PARERR;
			break;
	}
		return res;
}

#endif





