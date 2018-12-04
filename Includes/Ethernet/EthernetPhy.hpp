/*
 * EthernetPhy.hpp
 *
 *  Created on: 29 но€б. 2018 г.
 *      Author: Stalker1290
 */

#ifndef ETHERNETPHY_HPP_
#define ETHERNETPHY_HPP_

#include "chip.h"
#include "Defines.h"
#include "Ethernet/mdio.hpp"

class EthernetPhy_t{
public:
	EthernetPhy_t(MDIO_t* mdioPtr, uint32_t phyAddr);
	void configureSpeed(uint32_t speed);
	uint32_t getSpeed(void);
	static void setUpSLCRDivisors(uint32_t macBaseaddr, int32_t speed);
private:
	MDIO_t* mdioPtr;
	uint32_t phyAddr;

	uint32_t getTISpeed(void);
	uint32_t getMarvellSpeed(void);
};



#endif /* ETHERNETPHY_HPP_ */
