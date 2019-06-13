/**
 * @file
 * @brief Ethernet Phy Controller Driver Description
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

#ifndef ETHERNET_PHY_CONTROLLER_HPP_
#define ETHERNET_PHY_CONTROLLER_HPP_

#include "jbkernel/jb_common.h"
#include "jbdrivers/Ethernet/MdioController.hpp"

namespace jblib
{
namespace jbdrivers
{

class EthernetPhyController
{
public:
	static void setUpSlcrDivisors(uint32_t macBaseAddress, int32_t speed);
	EthernetPhyController(MdioController* mdioController, uint32_t phyAddr);
	void configureSpeed(uint32_t speed);
	uint32_t getSpeed(void);

private:
	uint32_t getTiSpeed(void);
	uint32_t getMarvellSpeed(void);

	MdioController* mdioController_ = NULL;
	uint32_t phyAddr_ = 0;
};

}
}

#endif /* ETHERNET_PHY_CONTROLLER_HPP_ */
