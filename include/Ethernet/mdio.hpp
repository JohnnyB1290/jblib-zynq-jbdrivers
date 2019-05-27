/*
 * mdio.hpp
 *
 *  Created on: 29 но€б. 2018 г.
 *      Author: Stalker1290
 */

#ifndef ETHERNET_MDIO_HPP_
#define ETHERNET_MDIO_HPP_

#include "chip.h"
#include "Defines.h"
#include "xemacps.h"

#define NUM_OF_MDIO 2

class MDIO_t{
public:
	static MDIO_t* getMdio(uint8_t num);
	void setMdioDivisor(XEmacPs_MdcDiv divisor);
	uint16_t phyRead(uint32_t phyAddress, uint32_t registerNum);
	void phyWrite(uint32_t phyAddress,uint32_t registerNum, uint16_t phyData);
private:
	MDIO_t(uint8_t num);

	static MDIO_t* mdioPtr[NUM_OF_MDIO];
	uint8_t num;
	XEmacPs emacPsInstance;
};



#endif /* ETHERNET_MDIO_HPP_ */
