/**
 * @file
 * @brief MDIO Controller Driver Description
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

#ifndef MDIO_CONTROLLER_HPP_
#define MDIO_CONTROLLER_HPP_

#include "jb_common.h"

#define MDIO_CONTROLLER_NUM_MDIO 2

namespace jblib
{
namespace jbdrivers
{

class MdioController
{
public:
	static MdioController* getMdioController(uint8_t number);
	void setMdioDivisor(XEmacPs_MdcDiv divisor);
	uint16_t phyRead(uint32_t phyAddress, uint32_t registerNum);
	void phyWrite(uint32_t phyAddress,uint32_t registerNum, uint16_t phyData);

private:
	MdioController(uint8_t number);

	static MdioController* mdioControllers_[MDIO_CONTROLLER_NUM_MDIO];
	uint8_t number_ = 0;
	XEmacPs emacPs_;
};

}
}

#endif /* MDIO_CONTROLLER_HPP_ */
