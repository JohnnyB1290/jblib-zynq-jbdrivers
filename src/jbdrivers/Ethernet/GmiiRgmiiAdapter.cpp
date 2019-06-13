/**
 * @file
 * @brief GMII RGMII Adapter Driver Realization
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

#include "Ethernet/GmiiRgmiiAdapter.hpp"

namespace jblib
{
namespace jbdrivers
{

typedef enum
{
	ADAPTER_STATUS_LINK_DOWN 	= 0,
	ADAPTER_STATUS_LINK_UP 		= 1
}GmiiRgmiiLinkStatus_t;



typedef enum
{
	ADAPTER_STATUS_CLK_SPEED_10 	= 0,
	ADAPTER_STATUS_CLK_SPEED_100 	= 1,
	ADAPTER_STATUS_CLK_SPEED_1000 	= 2
}GmiiRgmiiClockSpeedStatus_t;



typedef enum
{
	ADAPTER_STATUS_HALF_DUPLEX 	= 0,
	ADAPTER_STATUS_FULL_DUPLEX 	= 1
}GmiiRgmiiDuplexStatus_t;



typedef enum{
	ADAPTER_STATUS_SPEED_MODE_10 	= 0,
	ADAPTER_STATUS_SPEED_MODE_100 	= 1,
	ADAPTER_STATUS_SPEED_MODE_1000 	= 2,
}GmiiRgmiiSpeedModeStatus_t;



GmiiRgmiiAdapter::GmiiRgmiiAdapter(MdioController* mdioController,
		uint32_t phyAddr, uint32_t statusRegNum)
{
	this->mdioController_ = mdioController;
	this->phyAddr_ = phyAddr;
	this->statusRegNum_ = statusRegNum;
	XBram_Config* config = XBram_LookupConfig(BRAM_DEVICE_ID);
	XBram_CfgInitialize((XBram*)&this->bram_, config, config->MemBaseAddress);
}



void GmiiRgmiiAdapter::reset(void)
{
	this->mdioController_->phyWrite(this->phyAddr_,
			GMII_RGMII_ADAPTER_CONTROL_REG_ADDR, (1 << 15));
}



void GmiiRgmiiAdapter::setSpeed(GmiiRgmiiSpeedModeControl_t speed)
{
	uint32_t reg = 0;
	switch(speed)
	{
		case ADAPTER_SPEED_MODE_10:
			reg = 0;
			break;
		case ADAPTER_SPEED_MODE_100:
			reg = 0x2000;
			break;
		case ADAPTER_SPEED_MODE_1000:
			reg = 0x40;
			break;
		default:
			reg = 0;
			break;
	}
	this->mdioController_->phyWrite(this->phyAddr_,
			GMII_RGMII_ADAPTER_CONTROL_REG_ADDR, reg);
}



uint8_t GmiiRgmiiAdapter::getSpeed(void)
{
	uint16_t reg = this->mdioController_->phyRead(this->phyAddr_,
			GMII_RGMII_ADAPTER_CONTROL_REG_ADDR);
	switch(reg)
	{
		case 0:
			return ADAPTER_SPEED_MODE_10;
			break;
		case 0x2000:
			return ADAPTER_SPEED_MODE_100;
			break;
		case 0x40:
			return ADAPTER_SPEED_MODE_1000;
			break;
		default:
			return ADAPTER_SPEED_MODE_10;
			break;
	}
}



uint32_t GmiiRgmiiAdapter::getStatus(void)
{
	return XBram_ReadReg(this->bram_.Config.CtrlBaseAddress,
			(this->statusRegNum_ << 2));
}



uint8_t GmiiRgmiiAdapter::getLinkStatus(void)
{
	return (this->getStatus() & 0x0F);
}



uint8_t GmiiRgmiiAdapter::getClockSpeedStatus(void)
{
	return ((this->getStatus() & 0xF0) >> 4);
}



uint8_t GmiiRgmiiAdapter::getDuplexStatus(void)
{
	return ((this->getStatus() & 0xF00) >> 8);
}



uint8_t GmiiRgmiiAdapter::getSpeedModeStatus(void)
{
	return ((this->getStatus() & 0xF000) >> 12);
}

}
}
