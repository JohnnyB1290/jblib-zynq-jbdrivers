/**
 * @file
 * @brief GMII RGMII Adapter Driver Description
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

#ifndef GMII_RGMII_ADAPTER_HPP_
#define GMII_RGMII_ADAPTER_HPP_

#include "jbkernel/jb_common.h"
#include "jbdrivers/Ethernet/MdioController.hpp"
#include "xbram.h"


namespace jblib
{
namespace jbdrivers
{

typedef enum{
	ADAPTER_SPEED_MODE_10 	= 0,
	ADAPTER_SPEED_MODE_100 	= 1,
	ADAPTER_SPEED_MODE_1000 = 2,
}GmiiRgmiiSpeedModeControl_t;



class GmiiRgmiiAdapter{
public:
	GmiiRgmiiAdapter(MdioController* mdioController, uint32_t phyAddr,
			uint32_t statusRegNum);
	void reset(void);
	void setSpeed(GmiiRgmiiSpeedModeControl_t speed);
	uint8_t getSpeed(void);
	uint32_t getStatus(void);
	uint8_t getLinkStatus(void);
	uint8_t getClockSpeedStatus(void);
	uint8_t getDuplexStatus(void);
	uint8_t getSpeedModeStatus(void);

private:
	MdioController* mdioController_ = NULL;
	uint32_t phyAddr_ = 0;
	uint32_t statusRegNum_ = 0;
	XBram bram_;
};

}
}

#endif /* GMII_RGMII_ADAPTER_HPP_ */
