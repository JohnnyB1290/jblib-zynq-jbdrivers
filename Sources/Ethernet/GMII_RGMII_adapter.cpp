/*
 * GMII_RGMII_adapter.cpp
 *
 *  Created on: 29 но€б. 2018 г.
 *      Author: Stalker1290
 */

#include "Ethernet/GMII_RGMII_adapter.hpp"

GMIIRGMIIAdapter_t::GMIIRGMIIAdapter_t(MDIO_t* mdioPtr, uint32_t phyAddr, uint32_t statusRegNum){
	this->mdioPtr = mdioPtr;
	this->phyAddr = phyAddr;
	this->statusRegNum = statusRegNum;

	XBram_Config* configPtr = XBram_LookupConfig(BRAM_DEVICE_ID);
	XBram_CfgInitialize((XBram*)&this->bram, configPtr, configPtr->MemBaseAddress);
}

void GMIIRGMIIAdapter_t::reset(void){
	this->mdioPtr->phyWrite(this->phyAddr, ADAPTER_CONTROL_REG_ADDR, (1<<15));
}

void GMIIRGMIIAdapter_t::setSpeed(GMIIrgmiiSpeedModeControl_enum speed){
	uint32_t reg = 0;
	switch(speed){
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
	this->mdioPtr->phyWrite(this->phyAddr, ADAPTER_CONTROL_REG_ADDR, reg);
}

uint8_t GMIIRGMIIAdapter_t::getSpeed(void){
	uint16_t reg = this->mdioPtr->phyRead(this->phyAddr, ADAPTER_CONTROL_REG_ADDR);
	switch(reg){
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

uint32_t GMIIRGMIIAdapter_t::getStatus(void){
	return XBram_ReadReg(this->bram.Config.CtrlBaseAddress, (this->statusRegNum<<2));
}

uint8_t GMIIRGMIIAdapter_t::getLinkStatus(void){
	return (this->getStatus()&0x0F);
}

uint8_t GMIIRGMIIAdapter_t::getClockSpeedStatus(void){
	return ((this->getStatus()&0xF0)>>4);
}

uint8_t GMIIRGMIIAdapter_t::getDuplexStatus(void){
	return ((this->getStatus()&0xF00)>>8);
}

uint8_t GMIIRGMIIAdapter_t::getSpeedModeStatus(void){
	return ((this->getStatus()&0xF000)>>12);
}

