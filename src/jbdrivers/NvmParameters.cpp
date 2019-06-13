/**
 * @file
 * @brief Non Volatile Memory Parameters Realization
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

#include "jbkernel/jb_common.h"
#if JBDRIVERS_USE_QSPI_FLASH
#include <string.h>
#include <stdlib.h>
#include "jbdrivers/NvmParameters.hpp"
#include "jbdrivers/QspiFlash.hpp"
#include "jbutilities/Crc.hpp"
#if USE_CONSOLE
#include <stdio.h>
#endif

namespace jblib
{
namespace jbdrivers
{

using namespace jbkernel;
using namespace	jbutilities;


NvmParameters* NvmParameters::nvmParameters_ = (NvmParameters*)NULL;
uint32_t NvmParameters::baseAddress_ = 0;
NvmParametersHeader_t* NvmParameters::parametersHeader_ = NULL;



NvmParameters* NvmParameters::getNvmParameters(void)
{
	if(nvmParameters_ == (NvmParameters*)NULL)
		nvmParameters_ = new NvmParameters();
	return nvmParameters_;
}



NvmParameters::NvmParameters(void)
{
	QspiFlash::getQspiFlash()->initialize();
	baseAddress_ = (uint32_t)malloc_s(NVM_PARAMETERS_SIZE);
	if(baseAddress_ == 0)
		return;
	parametersHeader_ = (NvmParametersHeader_t*)baseAddress_;
	this->copyToRam();
	if(parametersHeader_->magic != NVM_PARAMETERS_MAGIC){
		this->eraseAllParameters();
		#if USE_CONSOLE
		printf("NVM Parameters Error: invalid Magic!\r\n");
		#endif
		return;
	}
	if(NvmParameters::parametersHeader_->size == 0)
		return;
	uint16_t calculatedCrc = Crc::crc16((uint8_t*)(baseAddress_ + parametersHeaderSize_),
			(parametersHeader_->size) * parametersCellSize_);
	if(NvmParameters::parametersHeader_->crc != calculatedCrc){
		this->eraseAllParameters();
		#if USE_CONSOLE
		printf("NVM Parameters Error: invalid CRC!\r\n");
		#endif
	}
}



NvmParametersCell_t* NvmParameters::getParameter(char* description)
{
	if((baseAddress_ == 0) || (parametersHeader_->size == 0))
		return (NvmParametersCell_t*)NULL;
	NvmParametersCell_t* cell =
			(NvmParametersCell_t*)(baseAddress_ + parametersHeaderSize_);
	for(uint32_t i = 0; i < parametersHeader_->size; i++){
		if(strncmp(cell->description, description, NVM_PARAMETERS_CELL_DESCRIPTION_SIZE) == 0)
			return cell;
		cell++;
	}
	return (NvmParametersCell_t*)NULL;
}



NvmParametersCell_t* NvmParameters::getParameter(char* description,
		uint8_t* data, uint8_t dataSize)
{

	NvmParametersCell_t* cell = this->getParameter(description);
	if(cell != NULL)
		memcpy(data, cell->data, (cell->dataSize <= dataSize) ? cell->dataSize : dataSize);
	return cell;
}



void NvmParameters::setParameter(NvmParametersCell_t* cell)
{
	if(baseAddress_ == 0)
		return;
	__disable_irq();
	NvmParametersHeader_t tempHeader;
	memcpy(&tempHeader, parametersHeader_, parametersHeaderSize_);
	NvmParametersCell_t* tempCell = this->getParameter(cell->description);
	if(tempCell == (NvmParametersCell_t*)NULL){
		tempCell = (NvmParametersCell_t*)(baseAddress_ + parametersHeaderSize_ +
				parametersCellSize_ * parametersHeader_->size);
		tempHeader.size++;
	}

	memcpy((void*)tempCell, (void*)cell, parametersCellSize_);
	tempHeader.crc = Crc::crc16((uint8_t*)(baseAddress_ + parametersHeaderSize_),
				tempHeader.size * parametersCellSize_);
	memcpy((void*)parametersHeader_, (void*)&tempHeader, parametersHeaderSize_);
	this->saveToNvm();
	if(cell->uid != 0xFF)
		memcpy((void*)&this->lastSetCell_, (void*)cell, parametersCellSize_);
	__enable_irq();
	if((cell->uid != 0xFF) && (this->changeCallback_ != (IVoidCallback*)NULL)){
		uint32_t tempPtr = cell->uid;
		this->changeCallback_->voidCallback(this, (void*)tempPtr);
	}
}



void NvmParameters::setParameter(uint8_t type, char* description,
		uint8_t* data, uint8_t dataSize)
{
	this->setParameter(type, 0xFF, description, data, dataSize);
}



void NvmParameters::setParameter(uint8_t type, uint8_t uid,
		char* description, uint8_t* data, uint8_t dataSize)
{
	this->setParameter(type, 0xFF, 0xFF, description, data, dataSize);
}



void NvmParameters::setParameter(uint8_t type, uint8_t uid,
		uint8_t groupId, char* description,
		uint8_t* data, uint8_t dataSize)
{
	NvmParametersCell_t tempCell;
	memset(&tempCell, 0, parametersCellSize_);

	tempCell.uid = uid;
	tempCell.groupId = groupId;
	tempCell.type = type;
	tempCell.dataSize = (NVM_PARAMETERS_CELL_DATA_SIZE <= dataSize) ?
			NVM_PARAMETERS_CELL_DATA_SIZE : dataSize;
	strncpy(tempCell.description, description, NVM_PARAMETERS_CELL_DESCRIPTION_SIZE);
	memcpy(tempCell.data, data, tempCell.dataSize);
	tempCell.descriptionSize = strlen(tempCell.description);
	this->setParameter(&tempCell);
}



void NvmParameters::deleteParameter(char* description)
{
	if((baseAddress_ == 0) || (parametersHeader_->size == 0))
		return;
	__disable_irq();
	NvmParametersHeader_t tempHeader;
	memcpy(&tempHeader, parametersHeader_, parametersHeaderSize_);

	NvmParametersCell_t* cell = this->getParameter(description);
	if(cell != (NvmParametersCell_t*)NULL){
		uint32_t tailSize = ((uint32_t)cell - baseAddress_ - parametersHeaderSize_) /
				parametersCellSize_;
		tailSize = parametersHeader_->size - tailSize - 1;
		tailSize = tailSize * parametersCellSize_;
		memcpy((void*)cell, (void*)((uint32_t)cell + parametersCellSize_),tailSize);
	}
	tempHeader.size--;
	tempHeader.crc = Crc::crc16((uint8_t*)(baseAddress_ + parametersHeaderSize_),
					tempHeader.size * parametersCellSize_);
	memcpy((void*)parametersHeader_, (void*)&tempHeader, parametersHeaderSize_);
	this->saveToNvm();
	__enable_irq();
}



void NvmParameters::eraseAllParameters(void)
{
	__disable_irq();
	NvmParametersHeader_t tempHeader;
	memset(&tempHeader, 0, parametersHeaderSize_);
	tempHeader.magic = NVM_PARAMETERS_MAGIC;
	tempHeader.size = 0;
	memcpy((void*)parametersHeader_, (void*)&tempHeader, parametersHeaderSize_);
	this->saveToNvm();
	__enable_irq();
}



NvmParametersHeader_t* NvmParameters::getHeader(void)
{
	return parametersHeader_;
}



uint32_t NvmParameters::getParametersSize(void)
{
	return (parametersHeaderSize_ + parametersCellSize_ * parametersHeader_->size);
}



void NvmParameters::setAllParameters(void* ptr)
{
	__disable_irq();
	memcpy((void*)parametersHeader_, (void*)ptr,
			parametersCellSize_ * ((NvmParametersHeader_t*)ptr)->size);
	this->saveToNvm();
	__enable_irq();
}



uint32_t NvmParameters::getCompressedParametersSize(void)
{
	uint32_t retSize = parametersHeaderSize_;
	NvmParametersCell_t* cell = (NvmParametersCell_t*)(baseAddress_ +
			parametersHeaderSize_);
	for(uint32_t i = 0; i < NvmParameters::parametersHeader_->size; i++){
		retSize += this->cellHeaderSize_ + cell->descriptionSize + cell->dataSize;
		cell++;
	}
	return retSize;
}



uint32_t NvmParameters::getCompressedParameters(uint8_t* data)
{
	uint32_t retSize = parametersHeaderSize_;
	uint8_t* ptr = data;
	NvmParametersCell_t* cell = (NvmParametersCell_t*)(baseAddress_ + parametersHeaderSize_);

	memcpy(ptr, (uint8_t*)parametersHeader_, parametersHeaderSize_);
	ptr += parametersHeaderSize_;

	for(uint32_t i = 0; i < parametersHeader_->size; i++){
		retSize += this->cellHeaderSize_ + cell->descriptionSize + cell->dataSize;
		memcpy(ptr, (uint8_t*)cell, this->cellHeaderSize_ + cell->descriptionSize);
		ptr += (this->cellHeaderSize_ + cell->descriptionSize);
		memcpy(ptr, ((uint8_t*)cell) + this->cellHeaderSize_ +
				NVM_PARAMETERS_CELL_DESCRIPTION_SIZE, cell->dataSize);
		ptr += cell->dataSize;
		cell++;
	}
	return retSize;
}



NvmParametersCell_t* NvmParameters::getLastSetCellPtr(void)
{
	return &this->lastSetCell_;
}



void NvmParameters::setChangeCallback(IVoidCallback* callback)
{
	this->changeCallback_ = callback;
}



void NvmParameters::copyToRam(void)
{
	if(baseAddress_)
		QspiFlash::getQspiFlash()->read(NVM_PARAMETERS_BASE_ADDRESS,
				(uint8_t*)parametersHeader_, NVM_PARAMETERS_SIZE);
}



void NvmParameters::saveToNvm(void)
{
	if(baseAddress_){
		QspiFlash::getQspiFlash()->erase(NVM_PARAMETERS_BASE_ADDRESS, NVM_PARAMETERS_SIZE);
		QspiFlash::getQspiFlash()->write(NVM_PARAMETERS_BASE_ADDRESS,
				(uint8_t*)parametersHeader_,NVM_PARAMETERS_SIZE);
	}
}

}
}

#endif
