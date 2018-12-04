/*
 * GMII_RGMII_adapter.hpp
 *
 *  Created on: 29 но€б. 2018 г.
 *      Author: Stalker1290
 */

#ifndef GMII_RGMII_ADAPTER_HPP_
#define GMII_RGMII_ADAPTER_HPP_

#include "chip.h"
#include "Ethernet/mdio.hpp"
#include "Defines.h"
#include "xbram.h"

#define ADAPTER_CONTROL_REG_ADDR 0x10

typedef enum{
	ADAPTER_STATUS_LINK_DOWN = 0,
	ADAPTER_STATUS_LINK_UP = 1
}GMIIrgmiiLinkStatus_enum;

typedef enum{
	ADAPTER_STATUS_CLK_SPEED_10 = 0,
	ADAPTER_STATUS_CLK_SPEED_100 = 1,
	ADAPTER_STATUS_CLK_SPEED_1000 = 2
}GMIIrgmiiClockSpeedStatus_enum;

typedef enum{
	ADAPTER_STATUS_HALF_DUPLEX = 0,
	ADAPTER_STATUS_FULL_DUPLEX = 1
}GMIIrgmiiDuplexStatus_enum;

typedef enum{
	ADAPTER_STATUS_SPEED_MODE_10 = 0,
	ADAPTER_STATUS_SPEED_MODE_100 = 1,
	ADAPTER_STATUS_SPEED_MODE_1000 = 2,
}GMIIrgmiiSpeedModeStatus_enum;

typedef enum{
	ADAPTER_SPEED_MODE_10 = 0,
	ADAPTER_SPEED_MODE_100 = 1,
	ADAPTER_SPEED_MODE_1000 = 2,
}GMIIrgmiiSpeedModeControl_enum;


class GMIIRGMIIAdapter_t{
public:
	GMIIRGMIIAdapter_t(MDIO_t* mdioPtr, uint32_t phyAddr, uint32_t statusRegNum);
	void reset(void);
	void setSpeed(GMIIrgmiiSpeedModeControl_enum speed);
	uint8_t getSpeed(void);
	uint32_t getStatus(void);
	uint8_t getLinkStatus(void);
	uint8_t getClockSpeedStatus(void);
	uint8_t getDuplexStatus(void);
	uint8_t getSpeedModeStatus(void);
private:
	MDIO_t* mdioPtr;
	uint32_t phyAddr;
	uint32_t statusRegNum;
	XBram bram;
};

#endif /* GMII_RGMII_ADAPTER_HPP_ */
