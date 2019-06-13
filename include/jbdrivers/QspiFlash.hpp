/**
 * @file
 * @brief QSPI Flash Driver Description
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

#ifndef QSPI_FLASH_HPP_
#define QSPI_FLASH_HPP_

#include "jbkernel/jb_common.h"
#include "xqspips.h"		/* QSPI device driver */
#if QSPI_FLASH_USE_FILE_SYSTEM
#include "jbfatfs/IStorageDevice.hpp"
#endif

namespace jblib
{
namespace jbdrivers
{

#if QSPI_FLASH_USE_FILE_SYSTEM
using namespace jbfatfs;
#endif

typedef struct
{
	u32 SectSize = 0;			/* Individual sector size or
						 	 	 * combined sector size in case of parallel config*/
	u32 NumSect = 0;			/* Total no. of sectors in one/two flash devices */
	u32 PageSize = 0;			/* Individual page size or
						 	 	 * combined page size in case of parallel config*/
	u32 NumPage = 0;			/* Total no. of pages in one/two flash devices */
	u32 FlashDeviceSize = 0;	/* This is the size of one flash device
						 	 	 * NOT the combination of both devices, if present
						 	 	 */
	u8 ManufacturerID = 0;		/* Manufacturer ID - used to identify make */
	u8 DeviceIDMemSize = 0;		/* Byte of device ID indicating the memory size */
	u32 SectMask = 0;			/* Mask to get sector start address */
	u8 NumDie = 0;				/* No. of die forming a single flash */
}QspiFlashInfo_t;



#if QSPI_FLASH_USE_FILE_SYSTEM
	class QspiFlash : public IStorageDevice
#else
	class QspiFlash
#endif
{
public:
	static QspiFlash* getQspiFlash(void);
	void initialize(void);
	void read(uint32_t address,uint8_t* buffer, uint32_t size);
	void write(uint32_t address,uint8_t* buffer, uint32_t size);
	void erase(u32 address, u32 size);
	QspiFlashInfo_t* getInfo(void);
	#if QSPI_FLASH_USE_FILE_SYSTEM
	virtual DSTATUS diskInitialize(void);
	virtual DSTATUS diskStatus(void);
	virtual DRESULT diskRead(BYTE* buff, DWORD sector, UINT count);
	virtual DRESULT diskWrite(const BYTE* buff, DWORD sector, UINT count);
	virtual DRESULT diskIoctl(BYTE cmd, void* buff);
	#endif

private:
	QspiFlash(void);
	void sendBankSelect(u32 bank);
	void dieErase(void);
	void bulkErase(void);
	u32 getRealAddress(u32 address);
	int readFlashId(void);
	void readFlash(u32 address, u32 size, u8 command,
			u8* readBuffer);
	void writeFlash(u32 address, u32 size, u8 command,
			u8* writeBuffer);
	void writePagePart(uint32_t address,uint8_t* buffer,
			uint32_t size);
	uint32_t getInfoTableIndex(void);

	static QspiFlash* qspiFlash_;
	static QspiFlashInfo_t flashInfoTable_[];
	u32 flashManufacturer_ = 0;
	u32 infoTableIndex_ = 0;
	XQspiPs xQspiPs_;
	bool isInitialized_ = false;

	#if QSPI_FLASH_USE_FILE_SYSTEM
	DSTATUS diskStatus_ = STA_NOINIT;
	#endif
};

}
}

#endif /* QSPI_FLASH_HPP_ */
