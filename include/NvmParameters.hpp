/**
 * @file
 * @brief Non Volatile Memory Parameters Description
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

#ifndef NVMPARAMETERS_HPP_
#define NVMPARAMETERS_HPP_

#include "jb_common.h"
#include "INvmParameters.hpp"

namespace jblib
{
namespace jbdrivers
{
using namespace jbkernel;


class NvmParameters : public INvmParameters
{
public:
	static NvmParameters* getNvmParameters(void);
	virtual NvmParametersCell_t* getParameter(char* description,
			uint8_t* data, uint8_t dataSize);
	virtual NvmParametersCell_t* getParameter(char* description);
	virtual void setParameter(NvmParametersCell_t* cell);
	virtual void setParameter(uint8_t type, char* description,
			uint8_t* data, uint8_t dataSize);
	virtual void setParameter(uint8_t type, uint8_t uid,
			char* description, uint8_t* data, uint8_t dataSize);
	virtual void setParameter(uint8_t type, uint8_t uid,
			uint8_t groupId, char* description,
			uint8_t* data, uint8_t dataSize);
	virtual void deleteParameter(char* description);
	virtual void eraseAllParameters(void);
	virtual NvmParametersHeader_t* getHeader(void);
	virtual uint32_t getParametersSize(void);
	virtual void setAllParameters(void* ptr);
	virtual uint32_t getCompressedParametersSize(void);
	virtual uint32_t getCompressedParameters(uint8_t* data);
	virtual void setChangeCallback(IVoidCallback* callback);
	virtual NvmParametersCell_t* getLastSetCellPtr(void);

private:
	NvmParameters(void);
	void copyToRam(void);
	void saveToNvm(void);

	static NvmParameters* nvmParameters_;
	static uint32_t baseAddress_;
	static NvmParametersHeader_t* parametersHeader_;
	static const uint32_t parametersHeaderSize_ =
			sizeof(NvmParametersHeader_t);
	static const uint32_t parametersCellSize_ =
			sizeof(NvmParametersCell_t);
	static const uint32_t cellHeaderSize_ = sizeof(NvmParametersCell_t) -
			NVM_PARAMETERS_CELL_DESCRIPTION_SIZE -
			NVM_PARAMETERS_CELL_DATA_SIZE;
	NvmParametersCell_t lastSetCell_;
	IVoidCallback* changeCallback_ = NULL;
};

}
}

#endif /* NVMPARAMETERS_HPP_ */
