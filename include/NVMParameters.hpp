/*
 * NVMParameters.hpp
 *
 *  Created on: 11 сент. 2018 г.
 *      Author: Stalker1290
 */

#ifndef NVMPARAMETERS_HPP_
#define NVMPARAMETERS_HPP_

#include "chip.h"
#include "Common_interfaces.hpp"

#define NVM_PARAMETERS_MAGIC 					0xAFDE
#define NVM_PARAMETERS_CELL_DESCRIPTION_SIZE	32
#define NVM_PARAMETERS_CELL_DATA_SIZE			32

#pragma pack(push, 1)

typedef struct NVMParamsHeader_struct{
	uint16_t magic;
	uint16_t crc;
	uint8_t size; //number of parameters
	uint8_t reserved[3];
}NVMParamsHeader_t;


typedef struct NVMParamsCell_struct{
	uint8_t type;
	uint8_t descriptionSize;
	uint8_t dataSize;
	uint8_t uid;
	uint8_t groupId;
	uint8_t reserved[3];
	char description[NVM_PARAMETERS_CELL_DESCRIPTION_SIZE];
	uint8_t data[NVM_PARAMETERS_CELL_DATA_SIZE];
}NVMParamsCell_t;

#pragma pack(pop)

#define PARAMS_CELL_TYPE_ARRAY_bm 			(1<<7) //1 - data is array
#define PARAMS_CELL_TYPE_HEX_bm 			(1<<6) //1 - show data as hex, 0 - show data in dec
#define PARAMS_CELL_TYPE_NOT_DISPLAY_bm 	(1<<5) //0 - display in WebIface, 1 - not display

typedef enum{
	nvmParamTypeU8 = 0,
	nvmParamTypeU16 = 1,
	nvmParamTypeU32 = 2,
	nvmParamTypeString = 3,
	nvmParamTypeDouble = 4,
	nvmParamTypeI8 = 5,
	nvmParamTypeI16 = 6,
	nvmParamTypeI32 = 7,
}nmvParamsCellType_t;


class NVMParameters_t{
public:
	static NVMParameters_t* getNVMParametersPtr(void);
	NVMParamsCell_t* getParameter(char* paramDescription, uint8_t* buf, uint8_t bufSize);
	NVMParamsCell_t* getParameter(char* paramDescription);
	void setParameter(NVMParamsCell_t* paramsCellPtr);
	void setParameter(uint8_t type, char* description, uint8_t* data, uint8_t dataSize);
	void setParameter(uint8_t type, uint8_t uid, char* description, uint8_t* data, uint8_t dataSize);
	void setParameter(uint8_t type, uint8_t uid, uint8_t groupId, char* description, uint8_t* data, uint8_t dataSize);
	void deleteParameter(char* paramDescription);
	void eraseAllParameters(void);
	NVMParamsHeader_t* getHeaderPtr(void);
	uint32_t getParametersSize(void);
	void setAllParameters(void* ptr);
	uint32_t getCompressedParametersSize(void);
	uint32_t getCompressedParameters(uint8_t* buf);
	void setChangeCallback(Callback_Interface_t* changeCall);
	NVMParamsCell_t* getLastSetCellPtr(void);
private:
	NVMParameters_t(void);
	void copyToRam(void);
	void saveToNVM(void);

	static NVMParameters_t* nvmParametersPtr;
	static uint32_t baseAddr;
	static NVMParamsHeader_t* paramsHeaderPtr;
	static uint8_t paramsHeaderSize;
	static uint8_t paramsCellSize;

	static const uint32_t cellHeaderSize = sizeof(NVMParamsCell_t) -
			NVM_PARAMETERS_CELL_DESCRIPTION_SIZE -
			NVM_PARAMETERS_CELL_DATA_SIZE;

	NVMParamsCell_t lastSetCell;
	Callback_Interface_t* changeCall;
};



#endif /* NVMPARAMETERS_HPP_ */
