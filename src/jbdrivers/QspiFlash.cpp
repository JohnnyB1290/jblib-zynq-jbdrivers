/**
 * @file
 * @brief QSPI Flash Driver Realization
 *
 *
 * @note
 * Copyright © 2010 - 2014 Xilinx, Inc.  All rights reserved.
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
#if JBDRIVERS_USE_QSPI_FLASH
#include <stdlib.h>
#include <string.h>
#include "jbdrivers/QspiFlash.hpp"
#if (USE_CONSOLE && QSPI_FLASH_USE_CONSOLE)
#include <stdio.h>
#endif

//************************** Constant Definitions *****************************/
/*
 * The following constants define the commands which may be sent to the Flash
 * device.
 */
#define WRITE_STATUS_CMD		0x01
#define WRITE_CMD				0x02
#define READ_CMD				0x03
#define WRITE_DISABLE_CMD		0x04
#define READ_STATUS_CMD			0x05
#define WRITE_ENABLE_CMD		0x06
#define FAST_READ_CMD			0x0B
#define DUAL_READ_CMD			0x3B
#define QUAD_READ_CMD			0x6B
#define BULK_ERASE_CMD			0xC7
#define	SEC_ERASE_CMD			0xD8
#define READ_ID					0x9F
#define READ_CONFIG_CMD			0x35
#define WRITE_CONFIG_CMD		0x01
#define BANK_REG_RD				0x16
#define BANK_REG_WR				0x17
/* Bank register is called Extended address Register in Micron */
#define EXTADD_REG_RD			0xC8
#define EXTADD_REG_WR			0xC5
#define	DIE_ERASE_CMD			0xC4
#define READ_FLAG_STATUS_CMD	0x70

/*
 * The following constants define the offsets within a FlashBuffer data
 * type for each kind of data.  Note that the read data offset is not the
 * same as the write data because the QSPI driver is designed to allow full
 * duplex transfers such that the number of bytes received is the number
 * sent and received.
 */
#define COMMAND_OFFSET		0 /* Flash instruction */
#define ADDRESS_1_OFFSET	1 /* MSB byte of address to read or write */
#define ADDRESS_2_OFFSET	2 /* Middle byte of address to read or write */
#define ADDRESS_3_OFFSET	3 /* LSB byte of address to read or write */
#define DATA_OFFSET			4 /* Start of Data for Read/Write */
#define DUMMY_OFFSET		4 /* Dummy byte offset for fast, dual and quad
				     	 	 	 reads */
#define DUMMY_SIZE			1 /* Number of dummy bytes for fast, dual and
				     	 	 	 quad reads */
#define RD_ID_SIZE			4 /* Read ID command + 3 bytes ID response */
#define BULK_ERASE_SIZE		1 /* Bulk Erase command size */
#define SEC_ERASE_SIZE		4 /* Sector Erase command + Sector address */
#define BANK_SEL_SIZE		2 /* BRWR or EARWR command + 1 byte bank value */
#define RD_CFG_SIZE			2 /* 1 byte Configuration register + RD CFG command*/
#define WR_CFG_SIZE			3 /* WRR command + 1 byte each status and Config Reg*/
#define DIE_ERASE_SIZE		4	/* Die Erase command + Die address */

/*
 * The following constants specify the extra bytes which are sent to the
 * Flash on the QSPI interface, that are not data, but control information
 * which includes the command and address
 */
#define OVERHEAD_SIZE		4

/*
 * Base address of Flash1
 */
#define FLASH1BASE 0x0000000

/*
 * Sixteen MB
 */
#define SIXTEENMB 0x1000000

/*
 * Mask for quad enable bit in Flash configuration register
 */
#define FLASH_QUAD_EN_MASK 	0x02

#define FLASH_SRWD_MASK 	0x80

/*
 * Bank mask
 */
#define BANKMASK 0xF000000

/*
 * Identification of Flash
 * Micron:
 * Byte 0 is Manufacturer ID;
 * Byte 1 is first byte of Device ID - 0xBB or 0xBA
 * Byte 2 is second byte of Device ID describes flash size:
 * 128Mbit : 0x18; 256Mbit : 0x19; 512Mbit : 0x20
 * Spansion:
 * Byte 0 is Manufacturer ID;
 * Byte 1 is Device ID - Memory Interface type - 0x20 or 0x02
 * Byte 2 is second byte of Device ID describes flash size:
 * 128Mbit : 0x18; 256Mbit : 0x19; 512Mbit : 0x20
 */
#define MICRON_ID_BYTE0		0x20
#define MICRON_ID_BYTE2_128	0x18
#define MICRON_ID_BYTE2_256	0x19
#define MICRON_ID_BYTE2_512	0x20
#define MICRON_ID_BYTE2_1G	0x21

#define SPANSION_ID_BYTE0		0x01
#define SPANSION_ID_BYTE2_128	0x18
#define SPANSION_ID_BYTE2_256	0x19
#define SPANSION_ID_BYTE2_512	0x20

#define WINBOND_ID_BYTE0		0xEF
#define WINBOND_ID_BYTE2_128	0x18

#define MACRONIX_ID_BYTE0		0xC2
#define MACRONIX_ID_BYTE2_256	0x19
#define MACRONIX_ID_BYTE2_512	0x1A
#define MACRONIX_ID_BYTE2_1G	0x1B

/*
 * The index for Flash config table
 */
/* Spansion*/
#define SPANSION_INDEX_START			0
#define FLASH_CFG_TBL_SINGLE_128_SP		SPANSION_INDEX_START
#define FLASH_CFG_TBL_STACKED_128_SP	(SPANSION_INDEX_START + 1)
#define FLASH_CFG_TBL_PARALLEL_128_SP	(SPANSION_INDEX_START + 2)
#define FLASH_CFG_TBL_SINGLE_256_SP		(SPANSION_INDEX_START + 3)
#define FLASH_CFG_TBL_STACKED_256_SP	(SPANSION_INDEX_START + 4)
#define FLASH_CFG_TBL_PARALLEL_256_SP	(SPANSION_INDEX_START + 5)
#define FLASH_CFG_TBL_SINGLE_512_SP		(SPANSION_INDEX_START + 6)
#define FLASH_CFG_TBL_STACKED_512_SP	(SPANSION_INDEX_START + 7)
#define FLASH_CFG_TBL_PARALLEL_512_SP	(SPANSION_INDEX_START + 8)

/* Micron */
#define MICRON_INDEX_START				(FLASH_CFG_TBL_PARALLEL_512_SP + 1)
#define FLASH_CFG_TBL_SINGLE_128_MC		MICRON_INDEX_START
#define FLASH_CFG_TBL_STACKED_128_MC	(MICRON_INDEX_START + 1)
#define FLASH_CFG_TBL_PARALLEL_128_MC	(MICRON_INDEX_START + 2)
#define FLASH_CFG_TBL_SINGLE_256_MC		(MICRON_INDEX_START + 3)
#define FLASH_CFG_TBL_STACKED_256_MC	(MICRON_INDEX_START + 4)
#define FLASH_CFG_TBL_PARALLEL_256_MC	(MICRON_INDEX_START + 5)
#define FLASH_CFG_TBL_SINGLE_512_MC		(MICRON_INDEX_START + 6)
#define FLASH_CFG_TBL_STACKED_512_MC	(MICRON_INDEX_START + 7)
#define FLASH_CFG_TBL_PARALLEL_512_MC	(MICRON_INDEX_START + 8)
#define FLASH_CFG_TBL_SINGLE_1GB_MC		(MICRON_INDEX_START + 9)
#define FLASH_CFG_TBL_STACKED_1GB_MC	(MICRON_INDEX_START + 10)
#define FLASH_CFG_TBL_PARALLEL_1GB_MC	(MICRON_INDEX_START + 11)

/* Winbond */
#define WINBOND_INDEX_START				(FLASH_CFG_TBL_PARALLEL_1GB_MC + 1)
#define FLASH_CFG_TBL_SINGLE_128_WB		WINBOND_INDEX_START
#define FLASH_CFG_TBL_STACKED_128_WB	(WINBOND_INDEX_START + 1)
#define FLASH_CFG_TBL_PARALLEL_128_WB	(WINBOND_INDEX_START + 2)

/* Macronix */
#define MACRONIX_INDEX_START			(FLASH_CFG_TBL_PARALLEL_128_WB + 1 - 3)
#define FLASH_CFG_TBL_SINGLE_256_MX		MACRONIX_INDEX_START
#define FLASH_CFG_TBL_STACKED_256_MX	(MACRONIX_INDEX_START + 1)
#define FLASH_CFG_TBL_PARALLEL_256_MX	(MACRONIX_INDEX_START + 2)
#define FLASH_CFG_TBL_SINGLE_512_MX		(MACRONIX_INDEX_START + 3)
#define FLASH_CFG_TBL_STACKED_512_MX	(MACRONIX_INDEX_START + 4)
#define FLASH_CFG_TBL_PARALLEL_512_MX	(MACRONIX_INDEX_START + 5)
#define FLASH_CFG_TBL_SINGLE_1G_MX		(MACRONIX_INDEX_START + 6)
#define FLASH_CFG_TBL_STACKED_1G_MX		(MACRONIX_INDEX_START + 7)
#define FLASH_CFG_TBL_PARALLEL_1G_MX	(MACRONIX_INDEX_START + 8)

/*
 * The following defines are for dual flash stacked mode interface.
 */
#define LQSPI_CR_FAST_QUAD_READ		0x0000006B /* Fast Quad Read output */
#define LQSPI_CR_1_DUMMY_BYTE		0x00000100 /* 1 Dummy Byte between
						     address and return data */

#define DUAL_STACK_CONFIG_WRITE		(XQSPIPS_LQSPI_CR_TWO_MEM_MASK | \
					 LQSPI_CR_1_DUMMY_BYTE | \
					 LQSPI_CR_FAST_QUAD_READ)

#define DUAL_QSPI_CONFIG_WRITE		(XQSPIPS_LQSPI_CR_TWO_MEM_MASK | \
					 XQSPIPS_LQSPI_CR_SEP_BUS_MASK | \
					 LQSPI_CR_1_DUMMY_BYTE | \
					 LQSPI_CR_FAST_QUAD_READ)

 namespace jblib
 {
 namespace jbdrivers
 {

 #if QSPI_FLASH_USE_FILE_SYSTEM
 using namespace jbfatfs;
 #endif

QspiFlashInfo_t QspiFlash::flashInfoTable_[33] = {
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

QspiFlash* QspiFlash::qspiFlash_ = (QspiFlash*)NULL;



QspiFlash* QspiFlash::getQspiFlash(void)
{
	if(qspiFlash_ == (QspiFlash*)NULL)
		qspiFlash_ = new QspiFlash();
	return qspiFlash_;
}



#if QSPI_FLASH_USE_FILE_SYSTEM
QspiFlash::QspiFlash(void) : IStorageDevice()
#else
QspiFlash::QspiFlash(void)
#endif
{

}



QspiFlashInfo_t* QspiFlash::getInfo(void)
{
	return &this->flashInfoTable_[this->infoTableIndex_];
}



void QspiFlash::initialize(void)
{
	if(!this->isInitialized_) {
		XQspiPs_Config* config = XQspiPs_LookupConfig(QSPI_DEVICE_ID);
		int status = XQspiPs_CfgInitialize(&this->xQspiPs_, config, config->BaseAddress);
		if (status != XST_SUCCESS){
			#if (USE_CONSOLE && QSPI_FLASH_USE_CONSOLE)
			printf("QSPI Flash Error: Initialization Fail\r\n");
			#endif
		}
		/*
		 * Set the pre-scaler for QSPI clock
		 */
		XQspiPs_SetClkPrescaler(&this->xQspiPs_, XQSPIPS_CLK_PRESCALE_8);
		/*
		 * Set Auto Start and Manual Chip select options and drive the
		 * HOLD_B high.
		*/
		XQspiPs_SetOptions(&this->xQspiPs_,
				XQSPIPS_FORCE_SSELECT_OPTION | XQSPIPS_HOLD_B_DRIVE_OPTION);
		if(config->ConnectionMode == XQSPIPS_CONNECTION_MODE_STACKED) {
			/*
			 * Enable two flash memories, Shared bus (NOT separate bus),
			 * L_PAGE selected by default
			 */
			XQspiPs_SetLqspiConfigReg(&this->xQspiPs_, DUAL_STACK_CONFIG_WRITE);
		}
		if(config->ConnectionMode == XQSPIPS_CONNECTION_MODE_PARALLEL){
			/*
			 * Enable two flash memories on separate buses
			 */
			XQspiPs_SetLqspiConfigReg(&this->xQspiPs_, DUAL_QSPI_CONFIG_WRITE);
		}
		/*
		 * Assert the Flash chip select.
		 */
		XQspiPs_SetSlaveSelect(&this->xQspiPs_);
		/*
		 * Read flash ID and obtain all flash related information
		 * It is important to call the read id function before
		 * performing proceeding to any operation, including
		 * preparing the WriteBuffer
		 */
		status = readFlashId();
		if (status != XST_SUCCESS){
			#if (USE_CONSOLE && QSPI_FLASH_USE_CONSOLE)
			printf("QSPI Flash Error: Read ID Fail\r\n");
			#endif
		}
		#if QSPI_FLASH_USE_FILE_SYSTEM
		if (status == XST_SUCCESS)
			this->diskStatus_ &= ~STA_NOINIT;
		else
			this->diskStatus_ |= STA_NODISK;
		#endif
		this->isInitialized_ = true;
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
******************************************************************************/
int QspiFlash::readFlashId(void)
{
	u8 writeBuffer[RD_ID_SIZE];
	u8 readBuffer[RD_ID_SIZE];
	/*
	 * Read ID in Auto mode.
	 */
	writeBuffer[COMMAND_OFFSET]   = READ_ID;
	writeBuffer[ADDRESS_1_OFFSET] = 0x23;		/* 3 dummy bytes */
	writeBuffer[ADDRESS_2_OFFSET] = 0x08;
	writeBuffer[ADDRESS_3_OFFSET] = 0x09;
	int status = XQspiPs_PolledTransfer(&this->xQspiPs_, writeBuffer, readBuffer,
				RD_ID_SIZE);
	if (status != XST_SUCCESS)
		return XST_FAILURE;
	/*
	 * Deduce flash make
	 */
	int startIndex = 0;
	if(readBuffer[1] == MICRON_ID_BYTE0) {
		this->flashManufacturer_ = MICRON_ID_BYTE0;
		startIndex = MICRON_INDEX_START;
	}else if(readBuffer[1] == SPANSION_ID_BYTE0) {
		this->flashManufacturer_ = SPANSION_ID_BYTE0;
		startIndex = SPANSION_INDEX_START;
	}else if(readBuffer[1] == WINBOND_ID_BYTE0) {
		this->flashManufacturer_ = WINBOND_ID_BYTE0;
		startIndex = WINBOND_INDEX_START;
	} else if(readBuffer[1] == MACRONIX_ID_BYTE0) {
		this->flashManufacturer_ = MACRONIX_ID_BYTE0;
		startIndex = MACRONIX_INDEX_START;
	}
	/*
	 * If valid flash ID, then check connection mode & size and
	 * assign corresponding index in the Flash configuration table
	 */
	if(((this->flashManufacturer_ == MICRON_ID_BYTE0) ||
			(this->flashManufacturer_ == SPANSION_ID_BYTE0)||
			(this->flashManufacturer_ == WINBOND_ID_BYTE0)) &&
			(readBuffer[3] == MICRON_ID_BYTE2_128)) {

		switch(this->xQspiPs_.Config.ConnectionMode)
		{
			case XQSPIPS_CONNECTION_MODE_SINGLE:
				this->infoTableIndex_ = FLASH_CFG_TBL_SINGLE_128_SP + startIndex;
				break;
			case XQSPIPS_CONNECTION_MODE_PARALLEL:
				this->infoTableIndex_ = FLASH_CFG_TBL_PARALLEL_128_SP + startIndex;
				break;
			case XQSPIPS_CONNECTION_MODE_STACKED:
				this->infoTableIndex_ = FLASH_CFG_TBL_STACKED_128_SP + startIndex;
				break;
			default:
				this->infoTableIndex_ = 0;
				break;
		}
	}
	/* 256 and 512Mbit supported only for Micron and Spansion, not Winbond */
	if(((this->flashManufacturer_ == MICRON_ID_BYTE0) ||
			(this->flashManufacturer_ == SPANSION_ID_BYTE0)
			|| (this->flashManufacturer_ == MACRONIX_ID_BYTE0)) &&
			(readBuffer[3] == MICRON_ID_BYTE2_256)) {

		switch(this->xQspiPs_.Config.ConnectionMode)
		{
			case XQSPIPS_CONNECTION_MODE_SINGLE:
				this->infoTableIndex_ = FLASH_CFG_TBL_SINGLE_256_SP + startIndex;
				break;
			case XQSPIPS_CONNECTION_MODE_PARALLEL:
				this->infoTableIndex_ = FLASH_CFG_TBL_PARALLEL_256_SP + startIndex;
				break;
			case XQSPIPS_CONNECTION_MODE_STACKED:
				this->infoTableIndex_ = FLASH_CFG_TBL_STACKED_256_SP + startIndex;
				break;
			default:
				this->infoTableIndex_ = 0;
				break;
		}
	}
	if ((((this->flashManufacturer_ == MICRON_ID_BYTE0) ||
			(this->flashManufacturer_ == SPANSION_ID_BYTE0)) &&
			(readBuffer[3] == MICRON_ID_BYTE2_512)) ||
			((this->flashManufacturer_ == MACRONIX_ID_BYTE0) &&
					(readBuffer[3] == MACRONIX_ID_BYTE2_512))) {

		switch(this->xQspiPs_.Config.ConnectionMode)
		{
			case XQSPIPS_CONNECTION_MODE_SINGLE:
				this->infoTableIndex_ = FLASH_CFG_TBL_SINGLE_512_SP + startIndex;
				break;
			case XQSPIPS_CONNECTION_MODE_PARALLEL:
				this->infoTableIndex_ = FLASH_CFG_TBL_PARALLEL_512_SP + startIndex;
				break;
			case XQSPIPS_CONNECTION_MODE_STACKED:
				this->infoTableIndex_ = FLASH_CFG_TBL_STACKED_512_SP + startIndex;
				break;
			default:
				this->infoTableIndex_ = 0;
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
	if(((this->flashManufacturer_ == MICRON_ID_BYTE0) &&
		(readBuffer[3] == MICRON_ID_BYTE2_1G)) ||
		((this->flashManufacturer_ == MACRONIX_ID_BYTE0) &&
		 (readBuffer[3] == MACRONIX_ID_BYTE2_1G))) {

		switch(this->xQspiPs_.Config.ConnectionMode)
		{
			case XQSPIPS_CONNECTION_MODE_SINGLE:
				this->infoTableIndex_ = FLASH_CFG_TBL_SINGLE_1GB_MC;
				break;
			case XQSPIPS_CONNECTION_MODE_PARALLEL:
				this->infoTableIndex_ = FLASH_CFG_TBL_PARALLEL_1GB_MC;
				break;
			case XQSPIPS_CONNECTION_MODE_STACKED:
				this->infoTableIndex_ = FLASH_CFG_TBL_STACKED_1GB_MC;
				break;
			default:
				this->infoTableIndex_ = 0;
				break;
		}
	}
	return XST_SUCCESS;
}



/******************************************************************************
*
* This function erases the sectors in the  serial Flash connected to the
* QSPI interface.
*
******************************************************************************/
void QspiFlash::erase(u32 address, u32 size)
{
	/*
	 * If erase size is same as the total size of the flash, use bulk erase
	 * command or die erase command multiple times as required
	 */
	if (size == ((flashInfoTable_[this->infoTableIndex_]).NumSect *
			(flashInfoTable_[this->infoTableIndex_]).SectSize) ) {

		u32 lqspiConfigReg = 0;
		if(this->xQspiPs_.Config.ConnectionMode == XQSPIPS_CONNECTION_MODE_STACKED){
			/*
			 * Get the current LQSPI configuration register value
			 */
			lqspiConfigReg = XQspiPs_GetLqspiConfigReg((&this->xQspiPs_));
			/*
			 * Set selection to L_PAGE
			 */
			XQspiPs_SetLqspiConfigReg(&this->xQspiPs_,
					lqspiConfigReg & (~XQSPIPS_LQSPI_CR_U_PAGE_MASK));
			/*
			 * Assert the Flash chip select.
			 */
			XQspiPs_SetSlaveSelect(&this->xQspiPs_);
		}
		if(flashInfoTable_[this->infoTableIndex_].NumDie == 1) {
			bulkErase();
		}
		if(flashInfoTable_[this->infoTableIndex_].NumDie > 1) {
			dieErase();
		}
		/*
		 * If stacked mode, bulk erase second flash
		 */
		if(this->xQspiPs_.Config.ConnectionMode == XQSPIPS_CONNECTION_MODE_STACKED){
			/*
			 * Get the current LQSPI configuration register value
			 */
			lqspiConfigReg = XQspiPs_GetLqspiConfigReg((&this->xQspiPs_));
			/*
			 * Set selection to U_PAGE
			 */
			XQspiPs_SetLqspiConfigReg(&this->xQspiPs_,
					lqspiConfigReg | XQSPIPS_LQSPI_CR_U_PAGE_MASK);
			/*
			 * Assert the Flash chip select.
			 */
			XQspiPs_SetSlaveSelect(&this->xQspiPs_);
			if(flashInfoTable_[this->infoTableIndex_].NumDie == 1) {
				bulkErase();
			}
			if(flashInfoTable_[this->infoTableIndex_].NumDie > 1) {
				dieErase();
			}
		}
		return;
	}
	/*
	 * If the erase size is less than the total size of the flash, use
	 * sector erase command
	 */
	u32 numSect = size / (flashInfoTable_[this->infoTableIndex_].SectSize) + 1;
	/*
	 * If size to k sectors,
	 * but the address range spans from N to N+k+1 sectors, then
	 * increment no. of sectors to be erased
	 */
	if( ((address + size) & flashInfoTable_[this->infoTableIndex_].SectMask) ==
			((address + (numSect * flashInfoTable_[this->infoTableIndex_].SectSize)) &
					flashInfoTable_[this->infoTableIndex_].SectMask) ) {
		numSect++;
	}

	u32 bank = 0;
	u8 bankInitFlag = 1;
	u8 readFlagStatusCmd[] = { READ_FLAG_STATUS_CMD, 0 };
	u8 flagStatus[2] = {0,0};
	u8 writeEnableCmd = WRITE_ENABLE_CMD;
	u8 writeBuffer[SEC_ERASE_SIZE];
	for (u32 sector = 0; sector < numSect; sector++) {
		/*
		 * Translate address based on type of connection
		 * If stacked assert the slave select based on address
		 */
		u32 realAddress = getRealAddress(address);
		/*
		 * Initial bank selection
		 */
		if((bankInitFlag) &&
				(flashInfoTable_[this->infoTableIndex_].FlashDeviceSize > SIXTEENMB)) {
			bankInitFlag = 0;
			bank = realAddress / SIXTEENMB;
			sendBankSelect(bank);
		}
		/*
		 * Check bank and send bank select if new bank
		 */
		if((bank != realAddress / SIXTEENMB) &&
				(flashInfoTable_[this->infoTableIndex_].FlashDeviceSize > SIXTEENMB)) {

			bank = realAddress / SIXTEENMB;
			sendBankSelect(bank);
		}
		/*
		 * Send the write enable command to the SEEPOM so that it can be
		 * written to, this needs to be sent as a separate transfer
		 * before the write
		 */
		XQspiPs_PolledTransfer(&this->xQspiPs_, &writeEnableCmd, NULL,
				  sizeof(writeEnableCmd));
		/*
		 * Setup the write command with the specified address and data
		 * for the Flash
		 */
		/*
		 * This ensures 3B address is sent to flash even with address
		 * greater than 128Mb.
		 */
		writeBuffer[COMMAND_OFFSET]   = SEC_ERASE_CMD;
		writeBuffer[ADDRESS_1_OFFSET] = (u8)(realAddress >> 16);
		writeBuffer[ADDRESS_2_OFFSET] = (u8)(realAddress >> 8);
		writeBuffer[ADDRESS_3_OFFSET] = (u8)(realAddress & 0xFF);
		/*
		 * Send the sector erase command and address; no receive buffer
		 * is specified since there is nothing to receive
		 */
		XQspiPs_PolledTransfer(&this->xQspiPs_, writeBuffer, NULL,
					SEC_ERASE_SIZE);

		if((flashInfoTable_[this->infoTableIndex_].NumDie > 1) &&
				(this->flashManufacturer_ == MICRON_ID_BYTE0)) {
			XQspiPs_PolledTransfer(&this->xQspiPs_, readFlagStatusCmd, flagStatus,
						sizeof(readFlagStatusCmd));
		}
		/*
		 * Wait for the sector erase command to the Flash to be completed
		 */
		u8 readStatusCmd[] = { READ_STATUS_CMD, 0 };
		u8 flashStatus[2] = {0,0};
		while (1) {
			/*
			 * Poll the status register of the device to determine
			 * when it completes, by sending a read status command
			 * and receiving the status byte
			 */
			XQspiPs_PolledTransfer(&this->xQspiPs_, readStatusCmd,
						flashStatus,
						sizeof(readStatusCmd));

			/*
			 * If the status indicates the write is done, then stop
			 * waiting, if a value of 0xFF in the status byte is
			 * read from the device and this loop never exits, the
			 * device slave select is possibly incorrect such that
			 * the device status is not being read
			 */
			if ((flashStatus[1] & 0x01) == 0)
				break;
		}
		if((flashInfoTable_[this->infoTableIndex_].NumDie > 1) &&
				(this->flashManufacturer_ == MICRON_ID_BYTE0)) {
			XQspiPs_PolledTransfer(&this->xQspiPs_, readFlagStatusCmd, flagStatus,
						sizeof(readFlagStatusCmd));
		}
		address += flashInfoTable_[this->infoTableIndex_].SectSize;
	}
}



/******************************************************************************
*
* This functions performs a bulk erase operation when the
* flash device has a single die. Works for both Spansion and Micron
*
******************************************************************************/
void QspiFlash::bulkErase(void)
{
	u8 writeEnableCmd = WRITE_ENABLE_CMD;
	u8 writeBuffer[BULK_ERASE_SIZE];
	/*
	 * Send the write enable command to the Flash so that it can be
	 * written to, this needs to be sent as a separate transfer
	 * before the erase
	 */
	XQspiPs_PolledTransfer(&this->xQspiPs_, &writeEnableCmd, NULL,
			  sizeof(writeEnableCmd));
	/*
	 * Setup the bulk erase command
	 */
	writeBuffer[COMMAND_OFFSET] = BULK_ERASE_CMD;

	/*
	 * Send the bulk erase command; no receive buffer is specified
	 * since there is nothing to receive
	 */
	XQspiPs_PolledTransfer(&this->xQspiPs_, writeBuffer, NULL,
				BULK_ERASE_SIZE);
	/*
	 * Wait for the erase command to the Flash to be completed
	 */
	u8 readStatusCmd[] = { READ_STATUS_CMD, 0 };
	u8 flashStatus[2] = {0,0};
	while (1) {
		/*
		 * Poll the status register of the device to determine
		 * when it completes, by sending a read status command
		 * and receiving the status byte
		 */
		XQspiPs_PolledTransfer(&this->xQspiPs_, readStatusCmd,
					flashStatus,
					sizeof(readStatusCmd));
		/*
		 * If the status indicates the write is done, then stop
		 * waiting; if a value of 0xFF in the status byte is
		 * read from the device and this loop never exits, the
		 * device slave select is possibly incorrect such that
		 * the device status is not being read
		 */
		if ((flashStatus[1] & 0x01) == 0)
			break;
	}
}



/******************************************************************************
*
* This functions performs a die erase operation on all the die in
* the flash device. This function uses the die erase command for
* Micron 512Mbit and 1Gbit
*
* @param	&this->xQspiPs_ is a pointer to the QSPI driver component to use.
* @param	WritBfrPtr is the pointer to command+address to be sent
*
* @return	None
*
* @note		None.
*
******************************************************************************/
void QspiFlash::dieErase(void)
{
	u8 writeEnableCmd = WRITE_ENABLE_CMD;
	u8 readFlagStatusCmd[] = { READ_FLAG_STATUS_CMD, 0 };
	u8 flagStatus[2] = {0, 0};
	u8 writeBuffer[DIE_ERASE_SIZE];

	for(u8 dieCounter = 0; dieCounter < flashInfoTable_[this->infoTableIndex_].NumDie;
			dieCounter++) {
		/*
		 * Select bank - the lower of the 2 banks in each die
		 * This is specific to Micron flash
		 */
		sendBankSelect(dieCounter * 2);
		/*
		 * Send the write enable command to the SEEPOM so that it can be
		 * written to, this needs to be sent as a separate transfer
		 * before the write
		 */
		XQspiPs_PolledTransfer(&this->xQspiPs_, &writeEnableCmd, NULL,
				  sizeof(writeEnableCmd));
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
		writeBuffer[COMMAND_OFFSET]   = DIE_ERASE_CMD;
		writeBuffer[ADDRESS_1_OFFSET] = 0x00;
		writeBuffer[ADDRESS_2_OFFSET] = 0x00;
		writeBuffer[ADDRESS_3_OFFSET] = 0x00;
		/*
		 * Send the sector erase command and address; no receive buffer
		 * is specified since there is nothing to receive
		 */
		XQspiPs_PolledTransfer(&this->xQspiPs_, writeBuffer, NULL,
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
			XQspiPs_PolledTransfer(&this->xQspiPs_, readFlagStatusCmd, flagStatus,
					sizeof(readFlagStatusCmd));

			/*
			 * If the status indicates the write is done, then stop
			 * waiting, if a value of 0xFF in the status byte is
			 * read from the device and this loop never exits, the
			 * device slave select is possibly incorrect such that
			 * the device status is not being read
			 */
			if ((flagStatus[1] & 0x80) == 0x80)
				break;
		}

	}
}



/******************************************************************************
*
* This functions selects the current bank
*
******************************************************************************/
void QspiFlash::sendBankSelect(u32 bank)
{
	u8 writeEnableCmd = WRITE_ENABLE_CMD;
	u8 writeBuffer[BANK_SEL_SIZE];
	/*
	 * Bank select commands for Micron and Spansion are different
	 */
	if(this->flashManufacturer_ == MICRON_ID_BYTE0) {
		/*
		 * For Micron command WREN should be sent first
		 * except for some specific feature set
		 */
		XQspiPs_PolledTransfer(&this->xQspiPs_, &writeEnableCmd, NULL,
					sizeof(writeEnableCmd));
		writeBuffer[COMMAND_OFFSET]   = EXTADD_REG_WR;
		writeBuffer[ADDRESS_1_OFFSET] = bank;
		/*
		 * Send the Extended address register write command
		 * written, no receive buffer required
		 */
		XQspiPs_PolledTransfer(&this->xQspiPs_, writeBuffer, NULL,
				BANK_SEL_SIZE);
	}
	if(this->flashManufacturer_ == SPANSION_ID_BYTE0) {
		writeBuffer[COMMAND_OFFSET]   = BANK_REG_WR;
		writeBuffer[ADDRESS_1_OFFSET] = bank;
		/*
		 * Send the Extended address register write command
		 * written, no receive buffer required
		 */
		XQspiPs_PolledTransfer(&this->xQspiPs_, writeBuffer, NULL,
				BANK_SEL_SIZE);
	}
}



/******************************************************************************
*
* This functions translates the address based on the type of interconnection.
* In case of stacked, this function asserts the corresponding slave select.
*
******************************************************************************/
u32 QspiFlash::getRealAddress(u32 address)
{
	u32 realAddress = 0;
	switch(this->xQspiPs_.Config.ConnectionMode)
	{
		case XQSPIPS_CONNECTION_MODE_SINGLE:
			realAddress = address;
			break;

		case XQSPIPS_CONNECTION_MODE_STACKED:
		{
			/*
			 * Get the current LQSPI Config reg value
			 */
			u32 lqspiConfigReg = XQspiPs_GetLqspiConfigReg((&this->xQspiPs_));

			/* Select lower or upper Flash based on sector address */
			if(address & flashInfoTable_[this->infoTableIndex_].FlashDeviceSize) {
				/*
				 * Set selection to U_PAGE
				 */
				XQspiPs_SetLqspiConfigReg(&this->xQspiPs_,
						lqspiConfigReg | XQSPIPS_LQSPI_CR_U_PAGE_MASK);
				/*
				 * Subtract first flash size when accessing second flash
				 */
				realAddress = address &
						(~flashInfoTable_[this->infoTableIndex_].FlashDeviceSize);
			}else{
				/*
				 * Set selection to L_PAGE
				 */
				XQspiPs_SetLqspiConfigReg(&this->xQspiPs_,
						lqspiConfigReg & (~XQSPIPS_LQSPI_CR_U_PAGE_MASK));
				realAddress = address;
			}
			/*
			 * Assert the Flash chip select.
			 */
			XQspiPs_SetSlaveSelect(&this->xQspiPs_);
		}
		break;

		case XQSPIPS_CONNECTION_MODE_PARALLEL:
			/*
			 * The effective address in each flash is the actual
			 * address / 2
			 */
			realAddress = address / 2;
			break;

		default:
			/* realAddress wont be assigned in this case; */
		break;
	}
	return(realAddress);
}



/******************************************************************************
*
* This function performs an I/O read.
*
******************************************************************************/
void QspiFlash::readFlash(u32 address, u32 size, u8 command, u8* readBuffer)
{
	// if N = size%4 != 0 need buffer size >= (size+(4-N))
	u8 writeBuffer[size + OVERHEAD_SIZE + DUMMY_SIZE + 4];
	u8 tempReadBuffer[size + OVERHEAD_SIZE + DUMMY_SIZE + 4];
	u32 totalByteCnt = size;

	while(((signed long)(size)) > 0) {
		/*
		 * Translate address based on type of connection
		 * If stacked assert the slave select based on address
		 */
		u32 realAddress = getRealAddress(address);

		if(flashInfoTable_[this->infoTableIndex_].FlashDeviceSize > SIXTEENMB) {
			u32 bank = realAddress / SIXTEENMB;
			sendBankSelect(bank);
		}
		/*
		 * If data to be read spans beyond the current bank, then
		 * calculate realByteCnt in current bank. Else
		 * realByteCnt is the same as size
		 */
		u32 realByteCnt = 0;
		if((address & BANKMASK) != ((address + size) & BANKMASK))
			realByteCnt = (address & BANKMASK) + SIXTEENMB - address;
		else
			realByteCnt = size;
		/*
		 * Setup the write command with the specified address and data for the
		 * Flash
		 */
		writeBuffer[COMMAND_OFFSET]   = command;
		writeBuffer[ADDRESS_1_OFFSET] = (u8)((realAddress & 0xFF0000) >> 16);
		writeBuffer[ADDRESS_2_OFFSET] = (u8)((realAddress & 0xFF00) >> 8);
		writeBuffer[ADDRESS_3_OFFSET] = (u8)(realAddress & 0xFF);

		if ((command == FAST_READ_CMD) || (command == DUAL_READ_CMD) ||
		    (command == QUAD_READ_CMD)) {
			realByteCnt += DUMMY_SIZE;
		}
		/*
		 * Send the read command to the Flash to read the specified number
		 * of bytes from the Flash, send the read command and address and
		 * receive the specified number of bytes of data in the data buffer
		 */
		XQspiPs_PolledTransfer(&this->xQspiPs_, writeBuffer,
				&(tempReadBuffer[totalByteCnt - size]),
				realByteCnt + OVERHEAD_SIZE);
		/*
		 * To discard the first 5 dummy bytes, shift the data in read buffer
		 */
		u8 shiftSize = 0;
		if((command == FAST_READ_CMD) || (command == DUAL_READ_CMD) ||
			    (command == QUAD_READ_CMD)){
			shiftSize = OVERHEAD_SIZE + DUMMY_SIZE;
		}
		else
			shiftSize =  OVERHEAD_SIZE;

		for(u32 bufferIndex = (totalByteCnt - size);
				bufferIndex < (totalByteCnt - size) + realByteCnt;
				bufferIndex++) {
			readBuffer[bufferIndex] = tempReadBuffer[bufferIndex + shiftSize];
		}
		/*
		 * Increase address to next bank
		 */
		address = (address & BANKMASK) + SIXTEENMB;
		/*
		 * Decrease byte count by bytes already read.
		 */
		if ((command == FAST_READ_CMD) || (command == DUAL_READ_CMD) ||
		    (command == QUAD_READ_CMD)) {
			size = size - (realByteCnt - DUMMY_SIZE);
		}else {
			size = size - realByteCnt;
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
******************************************************************************/
void QspiFlash::writeFlash(u32 address, u32 size, u8 command, u8* writeBuffer)
{
	// if N = size%4 != 0 need buffer size >= (size+(4-N))
	u8 tempWriteBuffer[size + OVERHEAD_SIZE + 4];
	memcpy(&(tempWriteBuffer[OVERHEAD_SIZE]), writeBuffer, size);
	/*
	 * Translate address based on type of connection
	 * If stacked assert the slave select based on address
	 */
	u32 realAddress = getRealAddress(address);
	/*
	 * Bank Select
	 */
	if(flashInfoTable_[this->infoTableIndex_].FlashDeviceSize > SIXTEENMB) {
		u32 bank = realAddress/SIXTEENMB;
		sendBankSelect(bank);
	}
	/*
	 * Send the write enable command to the Flash so that it can be
	 * written to, this needs to be sent as a separate transfer before
	 * the write
	 */
	u8 writeEnableCmd = WRITE_ENABLE_CMD;
	XQspiPs_PolledTransfer(&this->xQspiPs_, &writeEnableCmd, NULL,
				sizeof(writeEnableCmd));
	/*
	 * Setup the write command with the specified address and data for the
	 * Flash
	 */
	/*
	 * This will ensure a 3B address is transferred even when address
	 * is greater than 128Mb.
	 */
	tempWriteBuffer[COMMAND_OFFSET]   = command;
	tempWriteBuffer[ADDRESS_1_OFFSET] = (u8)((realAddress & 0xFF0000) >> 16);
	tempWriteBuffer[ADDRESS_2_OFFSET] = (u8)((realAddress & 0xFF00) >> 8);
	tempWriteBuffer[ADDRESS_3_OFFSET] = (u8)(realAddress & 0xFF);
	/*
	 * Send the write command, address, and data to the Flash to be
	 * written, no receive buffer is specified since there is nothing to
	 * receive
	 */
	XQspiPs_PolledTransfer(&this->xQspiPs_, tempWriteBuffer, NULL,
				size + OVERHEAD_SIZE);
	u8 readFlagStatusCmd[] = {READ_FLAG_STATUS_CMD, 0};
	u8 flagStatus[2] = {0, 0};
	if((QspiFlash::flashInfoTable_[this->infoTableIndex_].NumDie > 1) &&
			(this->flashManufacturer_ == MICRON_ID_BYTE0)) {
		XQspiPs_PolledTransfer(&this->xQspiPs_, readFlagStatusCmd, flagStatus,
					sizeof(readFlagStatusCmd));
	}
	/*
	 * Wait for the write command to the Flash to be completed, it takes
	 * some time for the data to be written
	 */
	u8 readStatusCmd[] = { READ_STATUS_CMD, 0 };  /* Must send 2 bytes */
	u8 flashStatus[2] = {0, 0};
	while (1) {
		/*
		 * Poll the status register of the Flash to determine when it
		 * completes, by sending a read status command and receiving the
		 * status byte
		 */
		XQspiPs_PolledTransfer(&this->xQspiPs_, readStatusCmd, flashStatus,
					sizeof(readStatusCmd));
		/*
		 * If the status indicates the write is done, then stop waiting,
		 * if a value of 0xFF in the status byte is read from the
		 * device and this loop never exits, the device slave select is
		 * possibly incorrect such that the device status is not being
		 * read
		 */
		if ((flashStatus[1] & 0x01) == 0)
			break;
	}
	if((flashInfoTable_[this->infoTableIndex_].NumDie > 1) &&
			(this->flashManufacturer_ == MICRON_ID_BYTE0)) {
		XQspiPs_PolledTransfer(&this->xQspiPs_, readFlagStatusCmd, flagStatus,
					sizeof(readFlagStatusCmd));
	}
}



void QspiFlash::read(uint32_t address,uint8_t* buffer, uint32_t size)
{
	readFlash(address, size, READ_CMD, buffer);
}



void QspiFlash::writePagePart(uint32_t address,uint8_t* buffer, uint32_t size)
{
	if(size <= flashInfoTable_[this->infoTableIndex_].PageSize) {
		uint8_t pageBuffer[flashInfoTable_[this->infoTableIndex_].PageSize];
		uint32_t pageOffset = address % (flashInfoTable_[this->infoTableIndex_].PageSize);
		uint32_t pageAddress = address - pageOffset;
		readFlash(pageAddress, flashInfoTable_[this->infoTableIndex_].PageSize,
				READ_CMD, pageBuffer);
		memcpy(&pageBuffer[pageOffset], buffer, size);
		writeFlash(pageAddress, flashInfoTable_[this->infoTableIndex_].PageSize,
				WRITE_CMD, pageBuffer);
	}
}



void QspiFlash::write(uint32_t address,uint8_t* buffer, uint32_t size)
{
	uint32_t pageOffset = address % (flashInfoTable_[this->infoTableIndex_].PageSize);
	uint32_t bw = 0;
	if(pageOffset != 0) {
		uint32_t tempSize = flashInfoTable_[this->infoTableIndex_].PageSize - pageOffset;
		if(size <= tempSize)
			tempSize = size;
		writePagePart(address, buffer, tempSize);
		size = size - tempSize;
		address = address - pageOffset + flashInfoTable_[this->infoTableIndex_].PageSize;
		bw = tempSize;
	}
	else
		bw = 0;
	if(size) {
		uint32_t br = size;
		uint32_t pageCount = size / flashInfoTable_[this->infoTableIndex_].PageSize + 1;
		for (uint32_t page = 0; page < pageCount; page++) {
			if(br > flashInfoTable_[this->infoTableIndex_].PageSize) {
				writeFlash(address, flashInfoTable_[this->infoTableIndex_].PageSize,
						WRITE_CMD, &buffer[bw]);
				br -= flashInfoTable_[this->infoTableIndex_].PageSize;
				bw += flashInfoTable_[this->infoTableIndex_].PageSize;
				address += flashInfoTable_[this->infoTableIndex_].PageSize;
			}
			else {
				writePagePart(address, &buffer[bw], br);
			}
		}
	}
}



uint32_t QspiFlash::getInfoTableIndex(void)
{
	return this->infoTableIndex_;
}



#if QSPI_FLASH_USE_FILE_SYSTEM

DSTATUS QspiFlash::diskStatus(void)
{
	return this->diskStatus_;
}



DSTATUS QspiFlash::diskInitialize(void)
{
	if(!this->isInitialized_)
		this->initialize();
	return this->diskStatus_;
}



DRESULT QspiFlash::diskRead(BYTE* buff, DWORD sector, UINT count)
{
	if (this->diskStatus_ & STA_NOINIT)
		return RES_NOTRDY;
	uint32_t startAddr = QSPI_FLASH_DISK_BASE_ADDRESS + sector * FF_MAX_SS;
	this->read(startAddr, buff, FF_MAX_SS * count);
	return RES_OK;
}



DRESULT QspiFlash::diskWrite(const BYTE* buff, DWORD sector, UINT count)
{
	static uint32_t numSect = 0;
	static uint8_t* ptr = NULL;

	if (this->diskStatus_ & STA_NOINIT)
		return RES_NOTRDY;
	uint32_t size = FF_MAX_SS * count;
	numSect = size / (flashInfoTable_[this->infoTableIndex_].SectSize) + 1;

	uint32_t startAddress = QSPI_FLASH_DISK_BASE_ADDRESS + sector * FF_MAX_SS;
	uint32_t startSectAddress = (startAddress /
			flashInfoTable_[this->infoTableIndex_].SectSize);
	startSectAddress = startSectAddress * flashInfoTable_[this->infoTableIndex_].SectSize;

	if(ptr == NULL )
		ptr = (uint8_t*)malloc_s(numSect * flashInfoTable_[this->infoTableIndex_].SectSize);
	if(ptr == (uint8_t*)NULL)
		return RES_NOTRDY;
	this->read(startSectAddress, ptr, numSect *
			flashInfoTable_[this->infoTableIndex_].SectSize);
	this->erase(startAddress, size);
	memcpy(&ptr[startAddress - startSectAddress], buff, size);
	this->write(startSectAddress, ptr, numSect *
			flashInfoTable_[this->infoTableIndex_].SectSize);
	return RES_OK;
}



DRESULT QspiFlash::diskIoctl(BYTE cmd, void* buff)
{
	if (this->diskStatus_ & STA_NOINIT)
		return RES_NOTRDY;
	DRESULT res = RES_ERROR;
	switch (cmd)
	{
		case CTRL_SYNC:	/* Make sure that no pending write process */
			res = RES_OK;
			break;

		case GET_SECTOR_COUNT:	/* Get number of sectors on the disk (DWORD) */
			*(DWORD *) buff = QSPI_FLASH_DISK_SIZE / FF_MAX_SS;
			res = RES_OK;
			break;

		case GET_SECTOR_SIZE:	/* Get R/W sector size (WORD) */
			*(WORD *) buff = FF_MAX_SS;
			res = RES_OK;
			break;

		case GET_BLOCK_SIZE:/* Get erase block size in unit of sector (DWORD) */
			*(DWORD *) buff = flashInfoTable_[this->infoTableIndex_].SectSize / FF_MAX_SS;
			res = RES_OK;
			break;

		default:
			res = RES_PARERR;
			break;
	}

	return res;
}

#endif

 }
 }

#endif
