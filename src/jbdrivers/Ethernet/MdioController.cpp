/**
 * @file
 * @brief MDIO Controller Driver Realization
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

#include <stdio.h>
#include "sleep.h"
#include "Ethernet/MdioController.hpp"
#if (USE_CONSOLE && ETHERNET_PHY_USE_CONSOLE)
#include <stdio.h>
#endif

namespace jblib::jbdrivers
{


MdioController* MdioController::mdioControllers_[MDIO_CONTROLLER_NUM_MDIO] = {
		NULL, NULL
};



MdioController* MdioController::getMdioController(uint8_t number)
{
	if(number >= MDIO_CONTROLLER_NUM_MDIO)
		return (MdioController*)NULL;
	if(mdioControllers_[number] == NULL)
		mdioControllers_[number] = new MdioController(number);
	return mdioControllers_[number];
}



MdioController::MdioController(uint8_t number)
{
	this->number_ = number;
	XEmacPs_Config* config = XEmacPs_LookupConfig((this->number_ == 0) ?
					EMACPS_0_DEVICE_ID : EMACPS_1_DEVICE_ID);
	LONG status = XEmacPs_CfgInitialize(&this->emacPs_, config, config->BaseAddress);
	if (status != XST_SUCCESS) {
		#if (USE_CONSOLE && ETHERNET_PHY_USE_CONSOLE)
		printf("MDIO Controller Error: XEmacPs_CfgInitialize %i\r\n", this->number_);
		#endif
	}
	XEmacPs_SetMdioDivisor(&this->emacPs_, MDC_DIV_224);
	sleep(1);
}



void MdioController::setMdioDivisor(XEmacPs_MdcDiv divisor)
{
	XEmacPs_SetMdioDivisor(&this->emacPs_, divisor);
	sleep(1);
}



uint16_t MdioController::phyRead(uint32_t phyAddress, uint32_t registerNum)
{
	uint16_t ret;
	XEmacPs_PhyRead(&this->emacPs_, phyAddress, registerNum, &ret);
	return ret;
}



void MdioController::phyWrite(uint32_t phyAddress,uint32_t registerNum,
		uint16_t phyData)
{
	XEmacPs_PhyWrite(&this->emacPs_, phyAddress, registerNum, phyData);
}

}
