/**
 * @file
 * @brief SD Card Driver Description
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

#ifndef SD_CARD_HPP_
#define SD_CARD_HPP_

#include "jb_common.h"
#include "IStorageDevice.hpp"
#include "xsdps.h"

namespace jblib
{
namespace jbdrivers
{

using namespace jbfatfs;


class SdCard : public IStorageDevice
{
public:
	static SdCard* getSdCard(void);
	virtual DSTATUS diskInitialize(void);
	virtual DSTATUS diskStatus(void);
	virtual DRESULT diskRead(BYTE* buff, DWORD sector, UINT count);
	virtual DRESULT diskWrite(const BYTE* buff, DWORD sector, UINT count);
	virtual DRESULT diskIoctl(BYTE cmd, void* buff);

private:
	SdCard(void);

	static SdCard* sdCard_;
	DSTATUS status_ = STA_NOINIT;
	XSdPs xSdPs_;
	u32 baseAddress_ = 0;
	u32 cardDetect_ = 0;
	u32 writeProtect_ = 0;
	u32 slotType_ = 0;
	u8 hostControllerVersion_ = 0;
	u8 extCsdBuffer_[512] __attribute__ ((aligned(32)));
};

}
}

#endif /* SD_CARD_HPP_ */
