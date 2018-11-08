/*
 * QSPI_Flash.cpp
 *
 *  Created on: 18.01.2018 ã.
 *      Author: Stalker1290
 */

#include "QSPI_Flash.hpp"
#include "stdlib.h"


FlashInfo QSPI_Flash_t::Flash_Config_Table[33] = {
		/* Spansion */
		{0x10000, 0x100, 256, 0x10000, 0x1000000,
				SPANSION_ID_BYTE0, SPANSION_ID_BYTE2_128, 0xFFFF0000, 1},
		{0x10000, 0x200, 256, 0x20000, 0x1000000,
				SPANSION_ID_BYTE0, SPANSION_ID_BYTE2_128, 0xFFFF0000, 1},
		{0x20000, 0x100, 512, 0x10000, 0x1000000,
				SPANSION_ID_BYTE0, SPANSION_ID_BYTE2_128, 0xFFFE0000, 1},
		{0x10000, 0x200, 256, 0x20000, 0x2000000,
				SPANSION_ID_BYTE0, SPANSION_ID_BYTE2_256, 0xFFFF0000, 1},
		{0x10000, 0x400, 256, 0x40000, 0x2000000,
				SPANSION_ID_BYTE0, SPANSION_ID_BYTE2_256, 0xFFFF0000, 1},
		{0x20000, 0x200, 512, 0x20000, 0x2000000,
				SPANSION_ID_BYTE0, SPANSION_ID_BYTE2_256, 0xFFFE0000, 1},
		{0x40000, 0x100, 512, 0x20000, 0x4000000,
				SPANSION_ID_BYTE0, SPANSION_ID_BYTE2_512, 0xFFFC0000, 1},
		{0x40000, 0x200, 512, 0x40000, 0x4000000,
				SPANSION_ID_BYTE0, SPANSION_ID_BYTE2_512, 0xFFFC0000, 1},
		{0x80000, 0x100, 1024, 0x20000, 0x4000000,
				SPANSION_ID_BYTE0, SPANSION_ID_BYTE2_512, 0xFFF80000, 1},
		/* Spansion 1Gbit is handled as 512Mbit stacked */
		/* Micron */
		{0x10000, 0x100, 256, 0x10000, 0x1000000,
				MICRON_ID_BYTE0, MICRON_ID_BYTE2_128, 0xFFFF0000, 1},
		{0x10000, 0x200, 256, 0x20000, 0x1000000,
				MICRON_ID_BYTE0, MICRON_ID_BYTE2_128, 0xFFFF0000, 1},
		{0x20000, 0x100, 512, 0x10000, 0x1000000,
				MICRON_ID_BYTE0, MICRON_ID_BYTE2_128, 0xFFFE0000, 1},
		{0x10000, 0x200, 256, 0x20000, 0x2000000,
				MICRON_ID_BYTE0, MICRON_ID_BYTE2_256, 0xFFFF0000, 1},
		{0x10000, 0x400, 256, 0x40000, 0x2000000,
				MICRON_ID_BYTE0, MICRON_ID_BYTE2_256, 0xFFFF0000, 1},
		{0x20000, 0x200, 512, 0x20000, 0x2000000,
				MICRON_ID_BYTE0, MICRON_ID_BYTE2_256, 0xFFFE0000, 1},
		{0x10000, 0x400, 256, 0x40000, 0x4000000,
				MICRON_ID_BYTE0, MICRON_ID_BYTE2_512, 0xFFFF0000, 2},
		{0x10000, 0x800, 256, 0x80000, 0x4000000,
				MICRON_ID_BYTE0, MICRON_ID_BYTE2_512, 0xFFFF0000, 2},
		{0x20000, 0x400, 512, 0x40000, 0x4000000,
				MICRON_ID_BYTE0, MICRON_ID_BYTE2_512, 0xFFFE0000, 2},
		{0x10000, 0x800, 256, 0x80000, 0x8000000,
				MICRON_ID_BYTE0, MICRON_ID_BYTE2_1G, 0xFFFF0000, 4},
		{0x10000, 0x1000, 256, 0x100000, 0x8000000,
				MICRON_ID_BYTE0, MICRON_ID_BYTE2_1G, 0xFFFF0000, 4},
		{0x20000, 0x800, 512, 0x80000, 0x8000000,
				MICRON_ID_BYTE0, MICRON_ID_BYTE2_1G, 0xFFFE0000, 4},
		/* Winbond */
		{0x10000, 0x100, 256, 0x10000, 0x1000000,
				WINBOND_ID_BYTE0, WINBOND_ID_BYTE2_128, 0xFFFF0000, 1},
		{0x10000, 0x200, 256, 0x20000, 0x1000000,
				WINBOND_ID_BYTE0, WINBOND_ID_BYTE2_128, 0xFFFF0000, 1},
		{0x20000, 0x100, 512, 0x10000, 0x1000000,
				WINBOND_ID_BYTE0, WINBOND_ID_BYTE2_128, 0xFFFE0000, 1},
		/* Macronix */
		{0x10000, 0x200, 256, 0x20000, 0x2000000,
				MACRONIX_ID_BYTE0, MACRONIX_ID_BYTE2_256, 0xFFFF0000, 1},
		{0x10000, 0x400, 256, 0x40000, 0x2000000,
				MACRONIX_ID_BYTE0, MACRONIX_ID_BYTE2_256, 0xFFFF0000, 1},
		{0x20000, 0x200, 512, 0x20000, 0x2000000,
				MACRONIX_ID_BYTE0, MACRONIX_ID_BYTE2_256, 0xFFFE0000, 1},
		{0x10000, 0x400, 256, 0x40000, 0x4000000,
				MACRONIX_ID_BYTE0, MACRONIX_ID_BYTE2_512, 0xFFFF0000, 1},
		{0x10000, 0x800, 256, 0x80000, 0x4000000,
				MACRONIX_ID_BYTE0, MACRONIX_ID_BYTE2_512, 0xFFFF0000, 1},
		{0x20000, 0x400, 512, 0x40000, 0x4000000,
				MACRONIX_ID_BYTE0, MACRONIX_ID_BYTE2_512, 0xFFFE0000, 1},
		{0x2000, 0x4000, 256, 0x80000, 0x8000000,
				MACRONIX_ID_BYTE0, MACRONIX_ID_BYTE2_1G, 0xFFFF0000, 1},
		{0x2000, 0x8000, 256, 0x100000, 0x8000000,
				MACRONIX_ID_BYTE0, MACRONIX_ID_BYTE2_1G, 0xFFFF0000, 1},
		{0x4000, 0x4000, 512, 0x80000, 0x8000000,
				MACRONIX_ID_BYTE0, MACRONIX_ID_BYTE2_1G, 0xFFFE0000, 1}
};				/**< Flash Config Table */

QSPI_Flash_t* QSPI_Flash_t::QSPI_FLASH_instance_ptr = (QSPI_Flash_t*)NULL;

QSPI_Flash_t* QSPI_Flash_t::Get_QSPI_flash(void)
{
	if(QSPI_Flash_t::QSPI_FLASH_instance_ptr == (QSPI_Flash_t*)NULL) QSPI_Flash_t::QSPI_FLASH_instance_ptr = new QSPI_Flash_t();
	return QSPI_Flash_t::QSPI_FLASH_instance_ptr;
}

#ifdef USE_FS
QSPI_Flash_t::QSPI_Flash_t(void):Storage_Device_FS_t()
#else
QSPI_Flash_t::QSPI_Flash_t(void)
#endif
{
	this->FlashMake = 0;
	this->FCTIndex = 0;
	this->initialized = false;
	#ifdef USE_FS
	this->Disc_Stat = STA_NOINIT;
	#endif
}


void QSPI_Flash_t::Initialize(void)
{
	XQspiPs_Config* QspiConfig;
	int Status;

	if(!this->initialized)
	{
		QspiConfig = XQspiPs_LookupConfig(QSPI_DEVICE_ID);

		Status = XQspiPs_CfgInitialize(&this->QspiInstance, QspiConfig, QspiConfig->BaseAddress);

		if (Status != XST_SUCCESS){
			#ifdef USE_CONSOLE
			#ifdef QSPI_console
			printf("%s\r\n", "QSPI Flash Initialization Fail");
			#endif
			#endif
		}

		/*
		 * Set the pre-scaler for QSPI clock
		 */
		XQspiPs_SetClkPrescaler(&this->QspiInstance, XQSPIPS_CLK_PRESCALE_8);

		/*
		 * Set Auto Start and Manual Chip select options and drive the
		 * HOLD_B high.
		*/
		XQspiPs_SetOptions(&this->QspiInstance, XQSPIPS_FORCE_SSELECT_OPTION | XQSPIPS_HOLD_B_DRIVE_OPTION);

		if(QspiConfig->ConnectionMode == XQSPIPS_CONNECTION_MODE_STACKED)
		{
			/*
			 * Enable two flash memories, Shared bus (NOT separate bus),
			 * L_PAGE selected by default
			 */
			XQspiPs_SetLqspiConfigReg(&this->QspiInstance, DUAL_STACK_CONFIG_WRITE);
		}

		if(QspiConfig->ConnectionMode == XQSPIPS_CONNECTION_MODE_PARALLEL)
		{
			/*
			 * Enable two flash memories on separate buses
			 */
			XQspiPs_SetLqspiConfigReg(&this->QspiInstance, DUAL_QSPI_CONFIG_WRITE);
		}

		/*
		 * Assert the Flash chip select.
		 */
		XQspiPs_SetSlaveSelect(&this->QspiInstance);

		/*
		 * Read flash ID and obtain all flash related information
		 * It is important to call the read id function before
		 * performing proceeding to any operation, including
		 * preparing the WriteBuffer
		 */

		Status = FlashReadID(&this->QspiInstance);

		if (Status != XST_SUCCESS){
			#ifdef USE_CONSOLE
			#ifdef QSPI_console
			xil_printf("%s\r\n", "QSPI Flash Read ID Fail");
			#endif
			#endif
		}

		#ifdef USE_FS
		if (Status == XST_SUCCESS) this->Disc_Stat &= ~STA_NOINIT;
		else this->Disc_Stat |= STA_NODISK;
		#endif

		this->initialized = true;
	}
}

/******************************************************************************
*
* This function reads serial Flash ID connected to the SPI interface.
* It then deduces the make and size of the flash and obtains the
* connection mode to point to corresponding parameters in the flash
* configuration table. The flash driver will function based on this and
* it presently supports Micron and Spansion - 128, 256 and 512Mbit and
* Winbond 128Mbit
*
* @param	QspiPtr is a pointer to the QSPI driver component to use.
* @param	Pointer to the write buffer (which is to be transmitted)
* @param	Pointer to the read buffer to which valid received data should be
* 			written
*
* @return	XST_SUCCESS if read id, otherwise XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int QSPI_Flash_t::FlashReadID(XQspiPs *QspiPtr)
{
	int Status;
	int StartIndex = 0;
	u8 WriteBuf[RD_ID_SIZE];
	u8* WriteBfrPtr = WriteBuf;
	u8 ReadBuf[RD_ID_SIZE];
	u8* ReadBfrPtr = ReadBuf;
	/*
	 * Read ID in Auto mode.
	 */
	WriteBfrPtr[COMMAND_OFFSET]   = READ_ID;
	WriteBfrPtr[ADDRESS_1_OFFSET] = 0x23;		/* 3 dummy bytes */
	WriteBfrPtr[ADDRESS_2_OFFSET] = 0x08;
	WriteBfrPtr[ADDRESS_3_OFFSET] = 0x09;

	Status = XQspiPs_PolledTransfer(QspiPtr, WriteBfrPtr, ReadBfrPtr,
				RD_ID_SIZE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Deduce flash make
	 */
	if(ReadBfrPtr[1] == MICRON_ID_BYTE0) {
		this->FlashMake = MICRON_ID_BYTE0;
		StartIndex = MICRON_INDEX_START;
	}else if(ReadBfrPtr[1] == SPANSION_ID_BYTE0) {
		this->FlashMake = SPANSION_ID_BYTE0;
		StartIndex = SPANSION_INDEX_START;
	}else if(ReadBfrPtr[1] == WINBOND_ID_BYTE0) {
		this->FlashMake = WINBOND_ID_BYTE0;
		StartIndex = WINBOND_INDEX_START;
	} else if(ReadBfrPtr[1] == MACRONIX_ID_BYTE0) {
		this->FlashMake = MACRONIX_ID_BYTE0;
		StartIndex = MACRONIX_INDEX_START;
	}


	/*
	 * If valid flash ID, then check connection mode & size and
	 * assign corresponding index in the Flash configuration table
	 */
	if(((this->FlashMake == MICRON_ID_BYTE0) || (this->FlashMake == SPANSION_ID_BYTE0)||
			(this->FlashMake == WINBOND_ID_BYTE0)) &&
			(ReadBfrPtr[3] == MICRON_ID_BYTE2_128)) {

		switch(QspiPtr->Config.ConnectionMode)
		{
			case XQSPIPS_CONNECTION_MODE_SINGLE:
				this->FCTIndex = FLASH_CFG_TBL_SINGLE_128_SP + StartIndex;
				break;
			case XQSPIPS_CONNECTION_MODE_PARALLEL:
				this->FCTIndex = FLASH_CFG_TBL_PARALLEL_128_SP + StartIndex;
				break;
			case XQSPIPS_CONNECTION_MODE_STACKED:
				this->FCTIndex = FLASH_CFG_TBL_STACKED_128_SP + StartIndex;
				break;
			default:
				this->FCTIndex = 0;
				break;
		}
	}
	/* 256 and 512Mbit supported only for Micron and Spansion, not Winbond */
	if(((this->FlashMake == MICRON_ID_BYTE0) || (this->FlashMake == SPANSION_ID_BYTE0)
			|| (this->FlashMake == MACRONIX_ID_BYTE0)) &&
			(ReadBfrPtr[3] == MICRON_ID_BYTE2_256)) {

		switch(QspiPtr->Config.ConnectionMode)
		{
			case XQSPIPS_CONNECTION_MODE_SINGLE:
				this->FCTIndex = FLASH_CFG_TBL_SINGLE_256_SP + StartIndex;
				break;
			case XQSPIPS_CONNECTION_MODE_PARALLEL:
				this->FCTIndex = FLASH_CFG_TBL_PARALLEL_256_SP + StartIndex;
				break;
			case XQSPIPS_CONNECTION_MODE_STACKED:
				this->FCTIndex = FLASH_CFG_TBL_STACKED_256_SP + StartIndex;
				break;
			default:
				this->FCTIndex = 0;
				break;
		}
	}
	if ((((this->FlashMake == MICRON_ID_BYTE0) || (this->FlashMake == SPANSION_ID_BYTE0)) &&
			(ReadBfrPtr[3] == MICRON_ID_BYTE2_512)) || ((this->FlashMake ==
			MACRONIX_ID_BYTE0) && (ReadBfrPtr[3] == MACRONIX_ID_BYTE2_512))) {

		switch(QspiPtr->Config.ConnectionMode)
		{
			case XQSPIPS_CONNECTION_MODE_SINGLE:
				this->FCTIndex = FLASH_CFG_TBL_SINGLE_512_SP + StartIndex;
				break;
			case XQSPIPS_CONNECTION_MODE_PARALLEL:
				this->FCTIndex = FLASH_CFG_TBL_PARALLEL_512_SP + StartIndex;
				break;
			case XQSPIPS_CONNECTION_MODE_STACKED:
				this->FCTIndex = FLASH_CFG_TBL_STACKED_512_SP + StartIndex;
				break;
			default:
				this->FCTIndex = 0;
				break;
		}
	}
	/*
	 * 1Gbit Single connection supported for Spansion.
	 * The ConnectionMode will indicate stacked as this part has 2 SS
	 * The device ID will indicate 512Mbit.
	 * This configuration is handled as the above 512Mbit stacked configuration
	 */
	/* 1Gbit single, parallel and stacked supported for Micron */
	if(((this->FlashMake == MICRON_ID_BYTE0) &&
		(ReadBfrPtr[3] == MICRON_ID_BYTE2_1G)) ||
		((this->FlashMake == MACRONIX_ID_BYTE0) &&
		 (ReadBfrPtr[3] == MACRONIX_ID_BYTE2_1G))) {

		switch(QspiPtr->Config.ConnectionMode)
		{
			case XQSPIPS_CONNECTION_MODE_SINGLE:
				this->FCTIndex = FLASH_CFG_TBL_SINGLE_1GB_MC;
				break;
			case XQSPIPS_CONNECTION_MODE_PARALLEL:
				this->FCTIndex = FLASH_CFG_TBL_PARALLEL_1GB_MC;
				break;
			case XQSPIPS_CONNECTION_MODE_STACKED:
				this->FCTIndex = FLASH_CFG_TBL_STACKED_1GB_MC;
				break;
			default:
				this->FCTIndex = 0;
				break;
		}
	}

	return XST_SUCCESS;
}

/******************************************************************************
*
*
* This function erases the sectors in the  serial Flash connected to the
* QSPI interface.
*
* @param	QspiPtr is a pointer to the QSPI driver component to use.
* @param	Address contains the address of the first sector which needs to
*		be erased.
* @param	ByteCount contains the total size to be erased.
* @param	Pointer to the write buffer (which is to be transmitted)
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void QSPI_Flash_t::FlashErase(u32 Address, u32 ByteCount)
{
	u8 WriteEnableCmd = { WRITE_ENABLE_CMD };
	u8 ReadStatusCmd[] = { READ_STATUS_CMD, 0 };  /* Must send 2 bytes */
	u8 FlashStatus[2];
	u32 Sector;
	u32 RealAddr = 0;
	u32 LqspiCr;
	u32 NumSect;
	u32 BankSel = 0;
	u8 BankInitFlag = 1;
	u8 ReadFlagSRCmd[] = { READ_FLAG_STATUS_CMD, 0 };
	u8 FlagStatus[2];
	u8 WriteBuf[SEC_ERASE_SIZE];
	u8* WriteBfrPtr = WriteBuf;
	XQspiPs* QspiPtr = &this->QspiInstance;

	/*
	 * If erase size is same as the total size of the flash, use bulk erase
	 * command or die erase command multiple times as required
	 */
	if (ByteCount == ((QSPI_Flash_t::Flash_Config_Table[this->FCTIndex]).NumSect *
			(QSPI_Flash_t::Flash_Config_Table[this->FCTIndex]).SectSize) ) {

		if(QspiPtr->Config.ConnectionMode == XQSPIPS_CONNECTION_MODE_STACKED){

			/*
			 * Get the current LQSPI configuration register value
			 */
			LqspiCr = XQspiPs_GetLqspiConfigReg(QspiPtr);
			/*
			 * Set selection to L_PAGE
			 */
			XQspiPs_SetLqspiConfigReg(QspiPtr,
					LqspiCr & (~XQSPIPS_LQSPI_CR_U_PAGE_MASK));

			/*
			 * Assert the Flash chip select.
			 */
			XQspiPs_SetSlaveSelect(QspiPtr);
		}

		if(QSPI_Flash_t::Flash_Config_Table[this->FCTIndex].NumDie == 1) {
			/*
			 * Call Bulk erase
			 */
			BulkErase(QspiPtr);
		}

		if(QSPI_Flash_t::Flash_Config_Table[this->FCTIndex].NumDie > 1) {
			/*
			 * Call Die erase
			 */
			DieErase(QspiPtr);
		}
		/*
		 * If stacked mode, bulk erase second flash
		 */
		if(QspiPtr->Config.ConnectionMode == XQSPIPS_CONNECTION_MODE_STACKED){

			/*
			 * Get the current LQSPI configuration register value
			 */
			LqspiCr = XQspiPs_GetLqspiConfigReg(QspiPtr);
			/*
			 * Set selection to U_PAGE
			 */
			XQspiPs_SetLqspiConfigReg(QspiPtr,
					LqspiCr | XQSPIPS_LQSPI_CR_U_PAGE_MASK);

			/*
			 * Assert the Flash chip select.
			 */
			XQspiPs_SetSlaveSelect(QspiPtr);

			if(QSPI_Flash_t::Flash_Config_Table[this->FCTIndex].NumDie == 1) {
				/*
				 * Call Bulk erase
				 */
				BulkErase(QspiPtr);
			}

			if(QSPI_Flash_t::Flash_Config_Table[this->FCTIndex].NumDie > 1) {
				/*
				 * Call Die erase
				 */
				DieErase(QspiPtr);
			}
		}

		return;
	}

	/*
	 * If the erase size is less than the total size of the flash, use
	 * sector erase command
	 */

	/*
	 * Calculate no. of sectors to erase based on byte count
	 */
	NumSect = ByteCount/(QSPI_Flash_t::Flash_Config_Table[this->FCTIndex].SectSize) + 1;

	/*
	 * If ByteCount to k sectors,
	 * but the address range spans from N to N+k+1 sectors, then
	 * increment no. of sectors to be erased
	 */

	if( ((Address + ByteCount) & QSPI_Flash_t::Flash_Config_Table[this->FCTIndex].SectMask) ==
			((Address + (NumSect * QSPI_Flash_t::Flash_Config_Table[this->FCTIndex].SectSize)) &
					QSPI_Flash_t::Flash_Config_Table[this->FCTIndex].SectMask) ) {
		NumSect++;
	}

	for (Sector = 0; Sector < NumSect; Sector++) {

		/*
		 * Translate address based on type of connection
		 * If stacked assert the slave select based on address
		 */
		RealAddr = GetRealAddr(QspiPtr, Address);

		/*
		 * Initial bank selection
		 */
		if((BankInitFlag) &&
				(QSPI_Flash_t::Flash_Config_Table[this->FCTIndex].FlashDeviceSize > SIXTEENMB)) {
			/*
			 * Reset initial bank select flag
			 */
			BankInitFlag = 0;
			/*
			 * Calculate initial bank
			 */
			BankSel = RealAddr/SIXTEENMB;
			/*
			 * Select bank
			 */
			SendBankSelect(QspiPtr, BankSel);
		}
		/*
		 * Check bank and send bank select if new bank
		 */
		if((BankSel != RealAddr/SIXTEENMB) &&
				(QSPI_Flash_t::Flash_Config_Table[this->FCTIndex].FlashDeviceSize > SIXTEENMB)) {
			/*
			 * Calculate initial bank
			 */
			BankSel = RealAddr/SIXTEENMB;
			/*
			 * Select bank
			 */
			SendBankSelect(QspiPtr, BankSel);
		}

		/*
		 * Send the write enable command to the SEEPOM so that it can be
		 * written to, this needs to be sent as a separate transfer
		 * before the write
		 */
		XQspiPs_PolledTransfer(QspiPtr, &WriteEnableCmd, NULL,
				  sizeof(WriteEnableCmd));

		/*
		 * Setup the write command with the specified address and data
		 * for the Flash
		 */
		/*
		 * This ensures 3B address is sent to flash even with address
		 * greater than 128Mb.
		 */
		WriteBfrPtr[COMMAND_OFFSET]   = SEC_ERASE_CMD;
		WriteBfrPtr[ADDRESS_1_OFFSET] = (u8)(RealAddr >> 16);
		WriteBfrPtr[ADDRESS_2_OFFSET] = (u8)(RealAddr >> 8);
		WriteBfrPtr[ADDRESS_3_OFFSET] = (u8)(RealAddr & 0xFF);

		/*
		 * Send the sector erase command and address; no receive buffer
		 * is specified since there is nothing to receive
		 */
		XQspiPs_PolledTransfer(QspiPtr, WriteBfrPtr, NULL,
					SEC_ERASE_SIZE);

		if((QSPI_Flash_t::Flash_Config_Table[this->FCTIndex].NumDie > 1) &&
				(this->FlashMake == MICRON_ID_BYTE0)) {
			XQspiPs_PolledTransfer(QspiPtr, ReadFlagSRCmd, FlagStatus,
						sizeof(ReadFlagSRCmd));
		}
		/*
		 * Wait for the sector erase command to the Flash to be completed
		 */
		while (1) {
			/*
			 * Poll the status register of the device to determine
			 * when it completes, by sending a read status command
			 * and receiving the status byte
			 */
			XQspiPs_PolledTransfer(QspiPtr, ReadStatusCmd,
						FlashStatus,
						sizeof(ReadStatusCmd));

			/*
			 * If the status indicates the write is done, then stop
			 * waiting, if a value of 0xFF in the status byte is
			 * read from the device and this loop never exits, the
			 * device slave select is possibly incorrect such that
			 * the device status is not being read
			 */
			if ((FlashStatus[1] & 0x01) == 0) {
				break;
			}
		}

		if((QSPI_Flash_t::Flash_Config_Table[this->FCTIndex].NumDie > 1) &&
				(this->FlashMake == MICRON_ID_BYTE0)) {
			XQspiPs_PolledTransfer(QspiPtr, ReadFlagSRCmd, FlagStatus,
						sizeof(ReadFlagSRCmd));
		}

		Address += QSPI_Flash_t::Flash_Config_Table[this->FCTIndex].SectSize;

	}
}

/******************************************************************************
*
* This functions performs a bulk erase operation when the
* flash device has a single die. Works for both Spansion and Micron
*
* @param	QspiPtr is a pointer to the QSPI driver component to use.
* @param	WritBfrPtr is the pointer to command+address to be sent
*
* @return	None
*
* @note		None.
*
******************************************************************************/
void QSPI_Flash_t::BulkErase(XQspiPs *QspiPtr)
{
	u8 WriteEnableCmd = { WRITE_ENABLE_CMD };
	u8 ReadStatusCmd[] = { READ_STATUS_CMD, 0 };  /* Must send 2 bytes */
	u8 FlashStatus[2];
	u8 WriteBuf[BULK_ERASE_SIZE];
	u8* WriteBfrPtr = WriteBuf;

	/*
	 * Send the write enable command to the Flash so that it can be
	 * written to, this needs to be sent as a separate transfer
	 * before the erase
	 */
	XQspiPs_PolledTransfer(QspiPtr, &WriteEnableCmd, NULL,
			  sizeof(WriteEnableCmd));

	/*
	 * Setup the bulk erase command
	 */
	WriteBfrPtr[COMMAND_OFFSET]   = BULK_ERASE_CMD;

	/*
	 * Send the bulk erase command; no receive buffer is specified
	 * since there is nothing to receive
	 */
	XQspiPs_PolledTransfer(QspiPtr, WriteBfrPtr, NULL,
				BULK_ERASE_SIZE);

	/*
	 * Wait for the erase command to the Flash to be completed
	 */
	while (1) {
		/*
		 * Poll the status register of the device to determine
		 * when it completes, by sending a read status command
		 * and receiving the status byte
		 */
		XQspiPs_PolledTransfer(QspiPtr, ReadStatusCmd,
					FlashStatus,
					sizeof(ReadStatusCmd));

		/*
		 * If the status indicates the write is done, then stop
		 * waiting; if a value of 0xFF in the status byte is
		 * read from the device and this loop never exits, the
		 * device slave select is possibly incorrect such that
		 * the device status is not being read
		 */
		if ((FlashStatus[1] & 0x01) == 0) {
			break;
		}
	}
}

/******************************************************************************
*
* This functions performs a die erase operation on all the die in
* the flash device. This function uses the die erase command for
* Micron 512Mbit and 1Gbit
*
* @param	QspiPtr is a pointer to the QSPI driver component to use.
* @param	WritBfrPtr is the pointer to command+address to be sent
*
* @return	None
*
* @note		None.
*
******************************************************************************/
void QSPI_Flash_t::DieErase(XQspiPs *QspiPtr)
{
	u8 WriteEnableCmd = { WRITE_ENABLE_CMD };
	u8 DieCnt;
	u8 ReadFlagSRCmd[] = { READ_FLAG_STATUS_CMD, 0 };
	u8 FlagStatus[2];
	u8 WriteBuf[DIE_ERASE_SIZE];
	u8* WriteBfrPtr = WriteBuf;

	for(DieCnt = 0; DieCnt < QSPI_Flash_t::Flash_Config_Table[this->FCTIndex].NumDie; DieCnt++) {
		/*
		 * Select bank - the lower of the 2 banks in each die
		 * This is specific to Micron flash
		 */
		SendBankSelect(QspiPtr, DieCnt*2);

		/*
		 * Send the write enable command to the SEEPOM so that it can be
		 * written to, this needs to be sent as a separate transfer
		 * before the write
		 */
		XQspiPs_PolledTransfer(QspiPtr, &WriteEnableCmd, NULL,
				  sizeof(WriteEnableCmd));

		/*
		 * Setup the write command with the specified address and data
		 * for the Flash
		 */
		/*
		 * This ensures 3B address is sent to flash even with address
		 * greater than 128Mb.
		 * The address is the start address of die - MSB bits will be
		 * derived from bank select by the flash
		 */
		WriteBfrPtr[COMMAND_OFFSET]   = DIE_ERASE_CMD;
		WriteBfrPtr[ADDRESS_1_OFFSET] = 0x00;
		WriteBfrPtr[ADDRESS_2_OFFSET] = 0x00;
		WriteBfrPtr[ADDRESS_3_OFFSET] = 0x00;

		/*
		 * Send the sector erase command and address; no receive buffer
		 * is specified since there is nothing to receive
		 */
		XQspiPs_PolledTransfer(QspiPtr, WriteBfrPtr, NULL,
				DIE_ERASE_SIZE);

		/*
		 * Wait for the sector erase command to the Flash to be completed
		 */
		while (1) {
			/*
			 * Poll the status register of the device to determine
			 * when it completes, by sending a read status command
			 * and receiving the status byte
			 */
			XQspiPs_PolledTransfer(QspiPtr, ReadFlagSRCmd, FlagStatus,
					sizeof(ReadFlagSRCmd));

			/*
			 * If the status indicates the write is done, then stop
			 * waiting, if a value of 0xFF in the status byte is
			 * read from the device and this loop never exits, the
			 * device slave select is possibly incorrect such that
			 * the device status is not being read
			 */
			if ((FlagStatus[1] & 0x80) == 0x80) {
				break;
			}
		}

	}
}

/******************************************************************************
*
* This functions selects the current bank
*
* @param	QspiPtr is a pointer to the QSPI driver component to use.
* @param	Pointer to the write buffer which contains data to be transmitted
* @param	BankSel is the bank to be selected in the flash device(s).
*
* @return	XST_SUCCESS if bank selected, otherwise XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
void QSPI_Flash_t::SendBankSelect(XQspiPs *QspiPtr, u32 BankSel)
{
	u8 WriteEnableCmd = { WRITE_ENABLE_CMD };
	u8 WriteBuf[BANK_SEL_SIZE];
	u8 *WriteBfrPtr = WriteBuf;

	/*
	 * Bank select commands for Micron and Spansion are different
	 */
	if(this->FlashMake == MICRON_ID_BYTE0) {
		/*
		 * For Micron command WREN should be sent first
		 * except for some specific feature set
		 */
		XQspiPs_PolledTransfer(QspiPtr, &WriteEnableCmd, NULL,
					sizeof(WriteEnableCmd));

		WriteBfrPtr[COMMAND_OFFSET]   = EXTADD_REG_WR;
		WriteBfrPtr[ADDRESS_1_OFFSET] = BankSel;

		/*
		 * Send the Extended address register write command
		 * written, no receive buffer required
		 */
		XQspiPs_PolledTransfer(QspiPtr, WriteBfrPtr, NULL,
				BANK_SEL_SIZE);

	}
	if(this->FlashMake == SPANSION_ID_BYTE0) {
		WriteBfrPtr[COMMAND_OFFSET]   = BANK_REG_WR;
		WriteBfrPtr[ADDRESS_1_OFFSET] = BankSel;

		/*
		 * Send the Extended address register write command
		 * written, no receive buffer required
		 */
		XQspiPs_PolledTransfer(QspiPtr, WriteBfrPtr, NULL,
				BANK_SEL_SIZE);
	}

	/* Winbond can be added here */

}


/******************************************************************************
*
* This functions translates the address based on the type of interconnection.
* In case of stacked, this function asserts the corresponding slave select.
*
* @param	QspiPtr is a pointer to the QSPI driver component to use.
* @param	Address which is to be accessed (for erase, write or read)
*
* @return	RealAddr is the translated address - for single it is unchanged;
* 			for stacked, the lower flash size is subtracted;
* 			for parallel the address is divided by 2.
*
* @note		None.
*
******************************************************************************/
u32 QSPI_Flash_t::GetRealAddr(XQspiPs *QspiPtr, u32 Address)
{
	u32 LqspiCr;
	u32 RealAddr = 0;

	switch(QspiPtr->Config.ConnectionMode) {
	case XQSPIPS_CONNECTION_MODE_SINGLE:
		RealAddr = Address;
		break;
	case XQSPIPS_CONNECTION_MODE_STACKED:
		/*
		 * Get the current LQSPI Config reg value
		 */
		LqspiCr = XQspiPs_GetLqspiConfigReg(QspiPtr);

		/* Select lower or upper Flash based on sector address */
		if(Address & QSPI_Flash_t::Flash_Config_Table[this->FCTIndex].FlashDeviceSize) {
			/*
			 * Set selection to U_PAGE
			 */
			XQspiPs_SetLqspiConfigReg(QspiPtr,
					LqspiCr | XQSPIPS_LQSPI_CR_U_PAGE_MASK);

			/*
			 * Subtract first flash size when accessing second flash
			 */
			RealAddr = Address &
					(~QSPI_Flash_t::Flash_Config_Table[this->FCTIndex].FlashDeviceSize);

		}else{

			/*
			 * Set selection to L_PAGE
			 */
			XQspiPs_SetLqspiConfigReg(QspiPtr,
					LqspiCr & (~XQSPIPS_LQSPI_CR_U_PAGE_MASK));

			RealAddr = Address;

		}

		/*
		 * Assert the Flash chip select.
		 */
		XQspiPs_SetSlaveSelect(QspiPtr);
		break;
	case XQSPIPS_CONNECTION_MODE_PARALLEL:
		/*
		 * The effective address in each flash is the actual
		 * address / 2
		 */
		RealAddr = Address / 2;
		break;
	default:
		/* RealAddr wont be assigned in this case; */
	break;

	}

	return(RealAddr);

}

/******************************************************************************
*
* This function performs an I/O read.
*
* @param	QspiPtr is a pointer to the QSPI driver component to use.
* @param	Address contains the address which needs to
*			be read.
* @param	ByteCount contains the total size to be read.
* @param	Command is the command used to read data from the flash. Supports
* 			normal, fast, dual and quad read commands.
* @param	Pointer to the write buffer which contains data to be transmitted
* @param	Pointer to the read buffer to which valid received data should be
* 			written
*
* @return	none.
*
* @note		None.
*
******************************************************************************/
void QSPI_Flash_t::FlashRead(XQspiPs *QspiPtr, u32 Address, u32 ByteCount, u8 Command, u8 *OutReadBfrPtr)
{
	u32 RealAddr = 0;
	u32 RealByteCnt;
	u32 BankSel;
	u32 BufferIndex;
	u32 TotalByteCnt;
	u8 ShiftSize;
	u8 WriteBuf[ByteCount + OVERHEAD_SIZE + DUMMY_SIZE + 4];  // if N = ByteCount%4 != 0 need buffer size >= (ByteCount+(4-N))
	u8 *WriteBfrPtr = WriteBuf;
	u8 ReadBuf[ByteCount + OVERHEAD_SIZE + DUMMY_SIZE + 4];
	u8 *ReadBfrPtr = ReadBuf;

	/*
	 * Retain the actual byte count
	 */
	TotalByteCnt = ByteCount;

	while(((signed long)(ByteCount)) > 0) {

		/*
		 * Translate address based on type of connection
		 * If stacked assert the slave select based on address
		 */
		RealAddr = GetRealAddr(QspiPtr, Address);

		/*
		 * Select bank
		 */
		if(QSPI_Flash_t::Flash_Config_Table[this->FCTIndex].FlashDeviceSize > SIXTEENMB) {
			BankSel = RealAddr/SIXTEENMB;
			SendBankSelect(QspiPtr, BankSel);
		}

		/*
		 * If data to be read spans beyond the current bank, then
		 * calculate RealByteCnt in current bank. Else
		 * RealByteCnt is the same as ByteCount
		 */
		if((Address & BANKMASK) != ((Address+ByteCount) & BANKMASK)) {
			RealByteCnt = (Address & BANKMASK) + SIXTEENMB - Address;
		}else {
			RealByteCnt = ByteCount;
		}


		/*
		 * Setup the write command with the specified address and data for the
		 * Flash
		 */
		WriteBfrPtr[COMMAND_OFFSET]   = Command;
		WriteBfrPtr[ADDRESS_1_OFFSET] = (u8)((RealAddr & 0xFF0000) >> 16);
		WriteBfrPtr[ADDRESS_2_OFFSET] = (u8)((RealAddr & 0xFF00) >> 8);
		WriteBfrPtr[ADDRESS_3_OFFSET] = (u8)(RealAddr & 0xFF);

		if ((Command == FAST_READ_CMD) || (Command == DUAL_READ_CMD) ||
		    (Command == QUAD_READ_CMD)) {
			RealByteCnt += DUMMY_SIZE;
		}
		/*
		 * Send the read command to the Flash to read the specified number
		 * of bytes from the Flash, send the read command and address and
		 * receive the specified number of bytes of data in the data buffer
		 */
		XQspiPs_PolledTransfer(QspiPtr, WriteBfrPtr,
				&(ReadBfrPtr[TotalByteCnt - ByteCount]),
				RealByteCnt + OVERHEAD_SIZE);

		/*
		 * To discard the first 5 dummy bytes, shift the data in read buffer
		 */
		if((Command == FAST_READ_CMD) || (Command == DUAL_READ_CMD) ||
			    (Command == QUAD_READ_CMD)){
			ShiftSize = OVERHEAD_SIZE + DUMMY_SIZE;
		}else{
			ShiftSize =  OVERHEAD_SIZE;
		}

		for(BufferIndex = (TotalByteCnt - ByteCount);
				BufferIndex < (TotalByteCnt - ByteCount) + RealByteCnt;
				BufferIndex++) {
			OutReadBfrPtr[BufferIndex] = ReadBfrPtr[BufferIndex + ShiftSize];
		}

		/*
		 * Increase address to next bank
		 */
		Address = (Address & BANKMASK) + SIXTEENMB;
		/*
		 * Decrease byte count by bytes already read.
		 */
		if ((Command == FAST_READ_CMD) || (Command == DUAL_READ_CMD) ||
		    (Command == QUAD_READ_CMD)) {
			ByteCount = ByteCount - (RealByteCnt - DUMMY_SIZE);
		}else {
			ByteCount = ByteCount - RealByteCnt;
		}

	}

}

/******************************************************************************
*
*
* This function writes to the  serial Flash connected to the QSPI interface.
* All the data put into the buffer must be in the same page of the device with
* page boundaries being on 256 byte boundaries.
*
* @param	QspiPtr is a pointer to the QSPI driver component to use.
* @param	Address contains the address to write data to in the Flash.
* @param	ByteCount contains the number of bytes to write.
* @param	Command is the command used to write data to the flash. QSPI
*		device supports only Page Program command to write data to the
*		flash.
* @param	Pointer to the write buffer (which is to be transmitted)
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void QSPI_Flash_t::FlashWrite(XQspiPs *QspiPtr, u32 Address, u32 ByteCount, u8 Command,u8 *WrBfr)
{
	u8 WriteEnableCmd = { WRITE_ENABLE_CMD };
	u8 ReadStatusCmd[] = { READ_STATUS_CMD, 0 };  /* Must send 2 bytes */
	u8 FlashStatus[2];
	u32 RealAddr = 0;
	u32 BankSel;
	u8 ReadFlagSRCmd[] = {READ_FLAG_STATUS_CMD, 0};
	u8 FlagStatus[2];
	u8 WriteBuf[ByteCount + OVERHEAD_SIZE + 4]; // if N = ByteCount%4 != 0 need buffer size >= (ByteCount+(4-N))
	u8 *WriteBfrPtr = WriteBuf;

	memcpy(&(WriteBuf[OVERHEAD_SIZE]),WrBfr,ByteCount);
	/*
	 * Translate address based on type of connection
	 * If stacked assert the slave select based on address
	 */
	RealAddr = GetRealAddr(QspiPtr, Address);
	/*
	 * Bank Select
	 */
	if(QSPI_Flash_t::Flash_Config_Table[this->FCTIndex].FlashDeviceSize > SIXTEENMB) {
		/*
		 * Calculate bank
		 */
		BankSel = RealAddr/SIXTEENMB;
		/*
		 * Select bank
		 */
		SendBankSelect(QspiPtr, BankSel);
	}

	/*
	 * Send the write enable command to the Flash so that it can be
	 * written to, this needs to be sent as a separate transfer before
	 * the write
	 */
	XQspiPs_PolledTransfer(QspiPtr, &WriteEnableCmd, NULL,
				sizeof(WriteEnableCmd));


	/*
	 * Setup the write command with the specified address and data for the
	 * Flash
	 */
	/*
	 * This will ensure a 3B address is transferred even when address
	 * is greater than 128Mb.
	 */
	WriteBfrPtr[COMMAND_OFFSET]   = Command;
	WriteBfrPtr[ADDRESS_1_OFFSET] = (u8)((RealAddr & 0xFF0000) >> 16);
	WriteBfrPtr[ADDRESS_2_OFFSET] = (u8)((RealAddr & 0xFF00) >> 8);
	WriteBfrPtr[ADDRESS_3_OFFSET] = (u8)(RealAddr & 0xFF);

	/*
	 * Send the write command, address, and data to the Flash to be
	 * written, no receive buffer is specified since there is nothing to
	 * receive
	 */
	XQspiPs_PolledTransfer(QspiPtr, WriteBfrPtr, NULL,
				ByteCount + OVERHEAD_SIZE);

	if((QSPI_Flash_t::Flash_Config_Table[this->FCTIndex].NumDie > 1) &&
			(this->FlashMake == MICRON_ID_BYTE0)) {
		XQspiPs_PolledTransfer(QspiPtr, ReadFlagSRCmd, FlagStatus,
					sizeof(ReadFlagSRCmd));
	}
	/*
	 * Wait for the write command to the Flash to be completed, it takes
	 * some time for the data to be written
	 */
	while (1) {
		/*
		 * Poll the status register of the Flash to determine when it
		 * completes, by sending a read status command and receiving the
		 * status byte
		 */
		XQspiPs_PolledTransfer(QspiPtr, ReadStatusCmd, FlashStatus,
					sizeof(ReadStatusCmd));

		/*
		 * If the status indicates the write is done, then stop waiting,
		 * if a value of 0xFF in the status byte is read from the
		 * device and this loop never exits, the device slave select is
		 * possibly incorrect such that the device status is not being
		 * read
		 */
		if ((FlashStatus[1] & 0x01) == 0) {
			break;
		}
	}

		if((QSPI_Flash_t::Flash_Config_Table[this->FCTIndex].NumDie > 1) &&
			(this->FlashMake == MICRON_ID_BYTE0)) {
		XQspiPs_PolledTransfer(QspiPtr, ReadFlagSRCmd, FlagStatus,
					sizeof(ReadFlagSRCmd));
	}

}

void QSPI_Flash_t::Read_flash(uint32_t Adress,uint8_t* Buf, uint32_t size)
{
	FlashRead(&this->QspiInstance, Adress, size, READ_CMD, Buf);
}



void QSPI_Flash_t::Write_page_part(uint32_t Adress,uint8_t* Buf, uint32_t size)
{
	uint8_t Page_buf[QSPI_Flash_t::Flash_Config_Table[this->FCTIndex].PageSize];
	uint32_t Page_offset;
	uint32_t Page_addr;

	if(size <= QSPI_Flash_t::Flash_Config_Table[this->FCTIndex].PageSize)
	{
		Page_offset = Adress%(QSPI_Flash_t::Flash_Config_Table[this->FCTIndex].PageSize);
		Page_addr = Adress - Page_offset;
		FlashRead(&this->QspiInstance, Page_addr, QSPI_Flash_t::Flash_Config_Table[this->FCTIndex].PageSize, READ_CMD, Page_buf);
		memcpy(&Page_buf[Page_offset],Buf,size);
		FlashWrite(&this->QspiInstance, Page_addr, QSPI_Flash_t::Flash_Config_Table[this->FCTIndex].PageSize, WRITE_CMD, Page_buf);
	}

}

void QSPI_Flash_t::Write_flash(uint32_t Adress,uint8_t* Buf, uint32_t size)
{
	uint32_t Page;
	uint32_t Page_count;
	uint32_t bytes_bw;
	uint32_t bytes_br;
	uint32_t Page_offset;
	uint32_t temp_size = 0;

	Page_offset = Adress%(QSPI_Flash_t::Flash_Config_Table[this->FCTIndex].PageSize);

	if(Page_offset!=0)
	{
		temp_size = QSPI_Flash_t::Flash_Config_Table[this->FCTIndex].PageSize - Page_offset;
		if(size<=temp_size) temp_size = size;
		Write_page_part(Adress,Buf,temp_size);
		size = size - temp_size;
		Adress = Adress - Page_offset + QSPI_Flash_t::Flash_Config_Table[this->FCTIndex].PageSize;
		bytes_bw = temp_size;
	}
	else
	{
		bytes_bw = 0;
	}

	bytes_br = size;

	if(size)
	{
		Page_count = size/QSPI_Flash_t::Flash_Config_Table[this->FCTIndex].PageSize + 1;
		for (Page = 0; Page < Page_count; Page++)
		{
			if(bytes_br > QSPI_Flash_t::Flash_Config_Table[this->FCTIndex].PageSize)
			{
				FlashWrite(&this->QspiInstance, Adress, QSPI_Flash_t::Flash_Config_Table[this->FCTIndex].PageSize, WRITE_CMD, &Buf[bytes_bw]);
				bytes_br = bytes_br - QSPI_Flash_t::Flash_Config_Table[this->FCTIndex].PageSize;
				bytes_bw = bytes_bw + QSPI_Flash_t::Flash_Config_Table[this->FCTIndex].PageSize;
				Adress = Adress + QSPI_Flash_t::Flash_Config_Table[this->FCTIndex].PageSize;
			}
			else
			{
				Write_page_part(Adress,&Buf[bytes_bw], bytes_br);
			}
		}
	}
}


uint32_t QSPI_Flash_t::Get_FCTIndex(void)
{
	return this->FCTIndex;
}

#ifdef USE_FS

DSTATUS QSPI_Flash_t::disk_status(void)
{
	return this->Disc_Stat;
}

DSTATUS QSPI_Flash_t::disk_initialize(void)
{
	if(!this->initialized) this->Initialize();
	return this->Disc_Stat;
}

DRESULT QSPI_Flash_t::disk_read(BYTE* buff, DWORD sector, UINT count)
{
	uint32_t start_addr;

	if (this->Disc_Stat & STA_NOINIT) return RES_NOTRDY;

	start_addr = QSPI_FLASH_DISK_BASE + sector*FF_MAX_SS;

	this->Read_flash(start_addr,buff, FF_MAX_SS * count);

	return RES_OK;
}

DRESULT QSPI_Flash_t::disk_write(const BYTE* buff, DWORD sector, UINT count)
{
	uint32_t start_addr;
	static uint32_t NumSect = 0;
	uint32_t ByteCount = 0;
	uint32_t StartSect_addr = 0;
	static uint8_t* ptr = NULL;

	if (this->Disc_Stat & STA_NOINIT) return RES_NOTRDY;

	ByteCount = FF_MAX_SS * count;

	NumSect = ByteCount/(QSPI_Flash_t::Flash_Config_Table[this->FCTIndex].SectSize) + 1;

	start_addr = QSPI_FLASH_DISK_BASE + sector*FF_MAX_SS;

	StartSect_addr = (start_addr/QSPI_Flash_t::Flash_Config_Table[this->FCTIndex].SectSize);
	StartSect_addr = StartSect_addr*QSPI_Flash_t::Flash_Config_Table[this->FCTIndex].SectSize;

	if(ptr == NULL ) ptr = (uint8_t*)malloc_s(NumSect * QSPI_Flash_t::Flash_Config_Table[this->FCTIndex].SectSize);
	if(ptr == (uint8_t*)NULL) return RES_NOTRDY;

	QSPI_Flash_t::Get_QSPI_flash()->Read_flash(StartSect_addr,ptr,NumSect*QSPI_Flash_t::Flash_Config_Table[this->FCTIndex].SectSize);

	QSPI_Flash_t::Get_QSPI_flash()->FlashErase(start_addr, ByteCount);

	memcpy(&ptr[start_addr - StartSect_addr], buff, ByteCount);

	QSPI_Flash_t::Get_QSPI_flash()->Write_flash(StartSect_addr,ptr,NumSect*QSPI_Flash_t::Flash_Config_Table[this->FCTIndex].SectSize);

	return RES_OK;
}

DRESULT QSPI_Flash_t::disk_ioctl(BYTE cmd, void* buff)
{
	DRESULT res;

	if (this->Disc_Stat & STA_NOINIT) return RES_NOTRDY;

	res = RES_ERROR;

	switch (cmd) {
	case CTRL_SYNC:	/* Make sure that no pending write process */

		res = RES_OK;
		break;

	case GET_SECTOR_COUNT:	/* Get number of sectors on the disk (DWORD) */
		*(DWORD *) buff = QSPI_FLASH_DISK_SIZE/FF_MAX_SS;
		res = RES_OK;
		break;

	case GET_SECTOR_SIZE:	/* Get R/W sector size (WORD) */
		*(WORD *) buff = FF_MAX_SS;
		res = RES_OK;
		break;

	case GET_BLOCK_SIZE:/* Get erase block size in unit of sector (DWORD) */
		*(DWORD *) buff = QSPI_Flash_t::Flash_Config_Table[this->FCTIndex].SectSize / FF_MAX_SS;
		res = RES_OK;
		break;

	default:
		res = RES_PARERR;
		break;
	}

	return res;
}

#endif
