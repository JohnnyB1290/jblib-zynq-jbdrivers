/*
 * NVMParameters.cpp
 *
 *  Created on: 11 сент. 2018 г.
 *      Author: Stalker1290
 */

#include "NVMParameters.hpp"
#include "QSPI_Flash.hpp"
#include "CRC.hpp"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "xil_exception.h"

NVMParameters_t* NVMParameters_t::nvmParametersPtr = (NVMParameters_t*)NULL;
uint32_t NVMParameters_t::baseAddr = 0;
NVMParamsHeader_t* NVMParameters_t::paramsHeaderPtr = NULL;
uint8_t NVMParameters_t::paramsHeaderSize = sizeof(NVMParamsHeader_t);
uint8_t NVMParameters_t::paramsCellSize = sizeof(NVMParamsCell_t);

NVMParameters_t* NVMParameters_t::getNVMParametersPtr(void){
	if(NVMParameters_t::nvmParametersPtr == (NVMParameters_t*)NULL)
		NVMParameters_t::nvmParametersPtr = new NVMParameters_t();
	return NVMParameters_t::nvmParametersPtr;
}

NVMParameters_t::NVMParameters_t(void){

	this->changeCall = (Callback_Interface_t*)NULL;
	QSPI_Flash_t::Get_QSPI_flash()->Initialize();

	NVMParameters_t::baseAddr = (uint32_t)malloc_s(QSPI_FLASH_NVM_PARAMS_SIZE);
	if(!NVMParameters_t::baseAddr) return;
	NVMParameters_t::paramsHeaderPtr = (NVMParamsHeader_t*)NVMParameters_t::baseAddr;

	this->copyToRam();

	if(NVMParameters_t::paramsHeaderPtr->magic != NVM_PARAMETERS_MAGIC){
		this->eraseAllParameters();
		printf("NVM Parameters Error: invalid Magic!\r\n");
		return;
	}

	if(NVMParameters_t::paramsHeaderPtr->size == 0) return;

	uint16_t calcCrc = CRC_t::Crc16((uint8_t*)(NVMParameters_t::baseAddr + NVMParameters_t::paramsHeaderSize),
			(NVMParameters_t::paramsHeaderPtr->size)*NVMParameters_t::paramsCellSize);

	if(NVMParameters_t::paramsHeaderPtr->crc != calcCrc){
		this->eraseAllParameters();
		printf("NVM Parameters Error: invalid CRC!\r\n");
	}
}

NVMParamsCell_t* NVMParameters_t::getParameter(char* paramDescription){

	if(!NVMParameters_t::baseAddr) return (NVMParamsCell_t*)NULL;
	if(NVMParameters_t::paramsHeaderPtr->size == 0) return (NVMParamsCell_t*)NULL;

	NVMParamsCell_t* cellPtr = (NVMParamsCell_t*)(NVMParameters_t::baseAddr + NVMParameters_t::paramsHeaderSize);
	for(uint8_t i = 0; i < NVMParameters_t::paramsHeaderPtr->size; i++){
		if(strncmp(cellPtr->description, paramDescription, NVM_PARAMETERS_CELL_DESCRIPTION_SIZE) == 0){
			return cellPtr;
		}
		cellPtr++;
	}
	return (NVMParamsCell_t*)NULL;
}

NVMParamsCell_t* NVMParameters_t::getParameter(char* paramDescription, uint8_t* buf, uint8_t bufSize){

	NVMParamsCell_t* cellPtr = this->getParameter(paramDescription);

	if(cellPtr != NULL)
		memcpy(buf, cellPtr->data, (cellPtr->dataSize <= bufSize) ? cellPtr->dataSize : bufSize);

	return cellPtr;
}

void NVMParameters_t::setParameter(NVMParamsCell_t* paramsCellPtr){
	if(!NVMParameters_t::baseAddr) return;
	__disable_irq();
	NVMParamsHeader_t tempHeader;
	memcpy(&tempHeader, NVMParameters_t::paramsHeaderPtr, NVMParameters_t::paramsHeaderSize);

	NVMParamsCell_t* cellPtr = this->getParameter(paramsCellPtr->description);

	if(cellPtr == (NVMParamsCell_t*)NULL){

		cellPtr = (NVMParamsCell_t*)(NVMParameters_t::baseAddr + NVMParameters_t::paramsHeaderSize +
				NVMParameters_t::paramsCellSize * NVMParameters_t::paramsHeaderPtr->size);

		tempHeader.size++;
	}

	memcpy((void*)cellPtr, (void*)paramsCellPtr, NVMParameters_t::paramsCellSize);

	tempHeader.crc = CRC_t::Crc16((uint8_t*)(NVMParameters_t::baseAddr + NVMParameters_t::paramsHeaderSize),
				tempHeader.size * NVMParameters_t::paramsCellSize);


	memcpy((void*)NVMParameters_t::paramsHeaderPtr, (void*)&tempHeader, NVMParameters_t::paramsHeaderSize);

	this->saveToNVM();

	if(paramsCellPtr->uid != 0xFF)
		memcpy((void*)&this->lastSetCell, (void*)paramsCellPtr, NVMParameters_t::paramsCellSize);

	__enable_irq();

	if((paramsCellPtr->uid != 0xFF) && (this->changeCall != (Callback_Interface_t*)NULL)){
		uint32_t tempPtr = paramsCellPtr->uid;
		this->changeCall->void_callback(this,(void*)tempPtr);
	}
}

void NVMParameters_t::setParameter(uint8_t type, char* description, uint8_t* data, uint8_t dataSize){

	this->setParameter(type, 0xFF, description, data, dataSize);
}

void NVMParameters_t::setParameter(uint8_t type, uint8_t uid, char* description, uint8_t* data, uint8_t dataSize){
	this->setParameter(type, 0xFF, 0xFF, description, data, dataSize);
}

void NVMParameters_t::setParameter(uint8_t type, uint8_t uid, uint8_t groupId, char* description, uint8_t* data, uint8_t dataSize){

	NVMParamsCell_t tempCell;
	memset(&tempCell, 0, NVMParameters_t::paramsCellSize);

	tempCell.uid = uid;
	tempCell.groupId = groupId;
	tempCell.type = type;
	tempCell.dataSize = (NVM_PARAMETERS_CELL_DATA_SIZE <= dataSize) ? NVM_PARAMETERS_CELL_DATA_SIZE : dataSize;
	strncpy(tempCell.description, description, NVM_PARAMETERS_CELL_DESCRIPTION_SIZE);
	memcpy(tempCell.data, data, tempCell.dataSize);
	tempCell.descriptionSize = strlen(tempCell.description);

	this->setParameter(&tempCell);
}

void NVMParameters_t::deleteParameter(char* paramDescription){

	if(!NVMParameters_t::baseAddr) return;
	__disable_irq();

	if(NVMParameters_t::paramsHeaderPtr->size == 0) return;

	NVMParamsHeader_t tempHeader;
	memcpy(&tempHeader, NVMParameters_t::paramsHeaderPtr, NVMParameters_t::paramsHeaderSize);

	NVMParamsCell_t* cellPtr = this->getParameter(paramDescription);
	if(cellPtr != (NVMParamsCell_t*)NULL){
		uint32_t tailSize = ((uint32_t)cellPtr - NVMParameters_t::baseAddr -
				NVMParameters_t::paramsHeaderSize) / NVMParameters_t::paramsCellSize;
		tailSize = NVMParameters_t::paramsHeaderPtr->size - tailSize - 1;
		tailSize = tailSize * NVMParameters_t::paramsCellSize;
		memcpy((void*)cellPtr,(void*)((uint32_t)cellPtr + NVMParameters_t::paramsCellSize),tailSize);
	}

	tempHeader.size--;
	tempHeader.crc = CRC_t::Crc16((uint8_t*)(NVMParameters_t::baseAddr + NVMParameters_t::paramsHeaderSize),
					tempHeader.size * NVMParameters_t::paramsCellSize);

	memcpy((void*)NVMParameters_t::paramsHeaderPtr,(void*)&tempHeader, NVMParameters_t::paramsHeaderSize);

	this->saveToNVM();
	__enable_irq();
}

void NVMParameters_t::eraseAllParameters(void){
	__disable_irq();
	NVMParamsHeader_t tempHeader;

	memset(&tempHeader, 0, NVMParameters_t::paramsHeaderSize);
	tempHeader.magic = NVM_PARAMETERS_MAGIC;
	tempHeader.size = 0;
	memcpy((void*)NVMParameters_t::paramsHeaderPtr,(void*)&tempHeader, NVMParameters_t::paramsHeaderSize);
	this->saveToNVM();
	__enable_irq();
}

NVMParamsHeader_t* NVMParameters_t::getHeaderPtr(void){
	return NVMParameters_t::paramsHeaderPtr;
}

uint32_t NVMParameters_t::getParametersSize(void){
	return (NVMParameters_t::paramsHeaderSize +
			NVMParameters_t::paramsCellSize * NVMParameters_t::paramsHeaderPtr->size);
}

void NVMParameters_t::setAllParameters(void* ptr){
	__disable_irq();
	memcpy((void*)NVMParameters_t::paramsHeaderPtr,
			(void*)ptr,
			NVMParameters_t::paramsCellSize * ((NVMParamsHeader_t*)ptr)->size);
	this->saveToNVM();
	__enable_irq();
}

uint32_t NVMParameters_t::getCompressedParametersSize(void){

	uint32_t retSize = NVMParameters_t::paramsHeaderSize;
	NVMParamsCell_t* cellPtr = (NVMParamsCell_t*)(NVMParameters_t::baseAddr + NVMParameters_t::paramsHeaderSize);

	for(uint8_t i = 0; i < NVMParameters_t::paramsHeaderPtr->size; i++){
		retSize += this->cellHeaderSize + cellPtr->descriptionSize + cellPtr->dataSize;
		cellPtr++;
	}
	return retSize;
}

uint32_t NVMParameters_t::getCompressedParameters(uint8_t* buf){

	uint32_t retSize = NVMParameters_t::paramsHeaderSize;
	uint8_t* ptr = buf;
	NVMParamsCell_t* cellPtr = (NVMParamsCell_t*)(NVMParameters_t::baseAddr + NVMParameters_t::paramsHeaderSize);

	memcpy(ptr,(uint8_t*)NVMParameters_t::paramsHeaderPtr, NVMParameters_t::paramsHeaderSize);
	ptr += NVMParameters_t::paramsHeaderSize;

	for(uint8_t i = 0; i < NVMParameters_t::paramsHeaderPtr->size; i++){

		retSize += this->cellHeaderSize + cellPtr->descriptionSize + cellPtr->dataSize;
		memcpy(ptr,(uint8_t*)cellPtr, this->cellHeaderSize + cellPtr->descriptionSize);
		ptr += (this->cellHeaderSize + cellPtr->descriptionSize);
		memcpy(ptr,
				((uint8_t*)cellPtr) + this->cellHeaderSize + NVM_PARAMETERS_CELL_DESCRIPTION_SIZE,
				cellPtr->dataSize);
		ptr += cellPtr->dataSize;
		cellPtr++;
	}
	return retSize;
}

NVMParamsCell_t* NVMParameters_t::getLastSetCellPtr(void){
	return &this->lastSetCell;
}

void NVMParameters_t::setChangeCallback(Callback_Interface_t* changeCall){
	this->changeCall = changeCall;
}

void NVMParameters_t::copyToRam(void){
	if(NVMParameters_t::baseAddr)
		QSPI_Flash_t::Get_QSPI_flash()->Read_flash(QSPI_FLASH_NVM_PARAMS_ADDR,
				(uint8_t*)NVMParameters_t::paramsHeaderPtr,QSPI_FLASH_NVM_PARAMS_SIZE);
}

void NVMParameters_t::saveToNVM(void){
	if(NVMParameters_t::baseAddr){
		QSPI_Flash_t::Get_QSPI_flash()->FlashErase(QSPI_FLASH_NVM_PARAMS_ADDR, QSPI_FLASH_NVM_PARAMS_SIZE);
		QSPI_Flash_t::Get_QSPI_flash()->Write_flash(QSPI_FLASH_NVM_PARAMS_ADDR,
						(uint8_t*)NVMParameters_t::paramsHeaderPtr,QSPI_FLASH_NVM_PARAMS_SIZE);
	}
}
