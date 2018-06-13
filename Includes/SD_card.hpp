/*
 * SD_card.hpp
 *
 *  Created on: 27 марта 2018 г.
 *      Author: Stalker1290
 */

#ifndef SD_CARD_HPP_
#define SD_CARD_HPP_

#include "chip.h"
#include "xsdps.h"		/* SD device driver */

#ifdef USE_FS
#include "FileSystem.hpp"
#endif


#ifdef USE_FS
class SD_Card_t:public Storage_Device_FS_t
#else
class SD_Card_t
#endif
{
public:
	static SD_Card_t* getSDCard(void);
	#ifdef USE_FS
	virtual DSTATUS disk_initialize(void);
	virtual DSTATUS disk_status(void);
	virtual DRESULT disk_read(BYTE* buff, DWORD sector, UINT count);
	virtual DRESULT disk_write(const BYTE* buff, DWORD sector, UINT count);
	virtual DRESULT disk_ioctl(BYTE cmd, void* buff);
	#endif
private:
	static SD_Card_t* SDCardInstancePtr;
	SD_Card_t(void);
	#ifdef USE_FS
	DSTATUS Stat;
	#endif
	XSdPs SdInstance;
	u32 BaseAddress;
	u32 CardDetect;
	u32 WriteProtect;
	u32 SlotType;
	u8 HostCntrlrVer;
	u8 ExtCsd[512] __attribute__ ((aligned(32)));
};

#endif /* ZYNQ_INCLUDES_SD_CARD_HPP_ */
