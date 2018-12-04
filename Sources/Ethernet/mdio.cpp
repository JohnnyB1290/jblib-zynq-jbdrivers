/*
 * mdio.cpp
 *
 *  Created on: 29 но€б. 2018 г.
 *      Author: Stalker1290
 */

#include "Ethernet/mdio.hpp"
#include "sleep.h"
#include "stdio.h"

MDIO_t* MDIO_t::mdioPtr[NUM_OF_MDIO] = {NULL, NULL};

MDIO_t* MDIO_t::getMdio(uint8_t num){
	if(num >= NUM_OF_MDIO) return (MDIO_t*)NULL;
	if(MDIO_t::mdioPtr[num] == NULL)
		MDIO_t::mdioPtr[num] = new MDIO_t(num);
	return MDIO_t::mdioPtr[num];
}

MDIO_t::MDIO_t(uint8_t num){

	XEmacPs_Config* Config = NULL;
	LONG Status;

	this->num = num;
	Config = XEmacPs_LookupConfig((this->num == 0) ? EMACPS_0_DEVICE_ID : EMACPS_1_DEVICE_ID);
	Status = XEmacPs_CfgInitialize(&this->emacPsInstance, Config, Config->BaseAddress);

	if (Status != XST_SUCCESS){
		#ifdef USE_CONSOLE
		#ifdef EthM_console
		printf("Error in CfgInitialize emacps %i\n\r",this->num);
		#endif
		#endif
	}

	XEmacPs_SetMdioDivisor(&this->emacPsInstance, MDC_DIV_224);
	sleep(1);
}


void MDIO_t::setMdioDivisor(XEmacPs_MdcDiv divisor){
	XEmacPs_SetMdioDivisor(&this->emacPsInstance, divisor);
	sleep(1);
}

uint16_t MDIO_t::phyRead(uint32_t phyAddress, uint32_t registerNum){
	uint16_t ret;
	XEmacPs_PhyRead(&this->emacPsInstance, phyAddress, registerNum, &ret);
	return ret;
}

void MDIO_t::phyWrite(uint32_t phyAddress,uint32_t registerNum, uint16_t phyData){
	XEmacPs_PhyWrite(&this->emacPsInstance, phyAddress, registerNum, phyData);
}








