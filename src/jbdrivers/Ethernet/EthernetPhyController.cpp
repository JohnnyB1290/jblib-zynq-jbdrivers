/**
 * @file
 * @brief Ethernet Phy Controller Driver Realization
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

#include "Ethernet/EthernetPhyController.hpp"
#include "sleep.h"
#include "JbController.hpp"
#if (USE_CONSOLE && ETHERNET_PHY_USE_CONSOLE)
#include <stdio.h>
#endif

/* Advertisement control register. */
#define ADVERTISE_10HALF		0x0020  /* Try for 10mbps half-duplex  */
#define ADVERTISE_10FULL		0x0040  /* Try for 10mbps full-duplex  */
#define ADVERTISE_100HALF		0x0080  /* Try for 100mbps half-duplex */
#define ADVERTISE_100FULL		0x0100  /* Try for 100mbps full-duplex */

#define ADVERTISE_100			(ADVERTISE_100FULL | ADVERTISE_100HALF)
#define ADVERTISE_10			(ADVERTISE_10FULL | ADVERTISE_10HALF)
#define ADVERTISE_1000			0x0300

#define IEEE_CONTROL_REG_OFFSET				0
#define IEEE_STATUS_REG_OFFSET				1
#define IEEE_AUTONEGO_ADVERTISE_REG			4
#define IEEE_PARTNER_ABILITIES_1_REG_OFFSET	5
#define IEEE_1000_ADVERTISE_REG_OFFSET		9
#define IEEE_COPPER_SPECIFIC_CONTROL_REG	16
#define IEEE_SPECIFIC_STATUS_REG			17
#define IEEE_COPPER_SPECIFIC_STATUS_REG_2	19
#define IEEE_CONTROL_REG_MAC				21
#define IEEE_PAGE_ADDRESS_REGISTER			22

#define IEEE_CTRL_1GBPS_LINKSPEED_MASK		0x2040
#define IEEE_CTRL_LINKSPEED_MASK			0x0040
#define IEEE_CTRL_LINKSPEED_1000M			0x0040
#define IEEE_CTRL_LINKSPEED_100M			0x2000
#define IEEE_CTRL_LINKSPEED_10M				0x0000
#define IEEE_CTRL_RESET_MASK				0x8000

#define IEEE_SPEED_MASK		0xC000
#define IEEE_SPEED_1000		0x8000
#define IEEE_SPEED_100		0x4000

#define IEEE_CTRL_RESET_MASK				0x8000
#define IEEE_CTRL_AUTONEGOTIATE_ENABLE		0x1000
#define IEEE_STAT_AUTONEGOTIATE_COMPLETE	0x0020
#define IEEE_STAT_AUTONEGOTIATE_RESTART		0x0200
#define IEEE_RGMII_TXRX_CLOCK_DELAYED_MASK	0x0030
#define IEEE_ASYMMETRIC_PAUSE_MASK			0x0800
#define IEEE_PAUSE_MASK						0x0400
#define IEEE_AUTONEG_ERROR_MASK				0x8000

#define PHY_DETECT_REG1 2
#define PHY_MARVELL_IDENTIFIER				0x0141
#define PHY_TI_IDENTIFIER					0x2000

#define PHY_REGCR		0x0D
#define PHY_ADDAR		0x0E
#define PHY_RGMIIDCTL	0x86
#define PHY_RGMIICTL	0x32
#define PHY_STS			0x11

#define PHY_REGCR_ADDR	0x001F
#define PHY_REGCR_DATA	0x401F

/* Frequency setting */
#define SLCR_LOCK_ADDR			(XPS_SYS_CTRL_BASEADDR + 0x4)
#define SLCR_UNLOCK_ADDR		(XPS_SYS_CTRL_BASEADDR + 0x8)
#define SLCR_GEM0_CLK_CTRL_ADDR	(XPS_SYS_CTRL_BASEADDR + 0x140)
#define SLCR_GEM1_CLK_CTRL_ADDR	(XPS_SYS_CTRL_BASEADDR + 0x144)
#define SLCR_LOCK_KEY_VALUE 	0x767B
#define SLCR_UNLOCK_KEY_VALUE	0xDF0D
#define EMACPS_SLCR_DIV_MASK	0xFC0FC0FF

namespace jblib
{
namespace jbdrivers
{


EthernetPhyController::EthernetPhyController(MdioController* mdioController,
		uint32_t phyAddr)
{
	this->mdioController_ = mdioController;
	this->phyAddr_ = phyAddr;
}



void EthernetPhyController::configureSpeed(uint32_t speed)
{
	this->mdioController_->phyWrite(this->phyAddr_, IEEE_PAGE_ADDRESS_REGISTER, 2);
	uint16_t control = this->mdioController_->phyRead(this->phyAddr_,
			IEEE_CONTROL_REG_MAC);
	control |= IEEE_RGMII_TXRX_CLOCK_DELAYED_MASK;
	this->mdioController_->phyWrite(this->phyAddr_, IEEE_CONTROL_REG_MAC, control);
	this->mdioController_->phyWrite(this->phyAddr_, IEEE_PAGE_ADDRESS_REGISTER, 0);

	uint16_t autonereg = this->mdioController_->phyRead(this->phyAddr_,
			IEEE_AUTONEGO_ADVERTISE_REG);
	autonereg |= IEEE_ASYMMETRIC_PAUSE_MASK;
	autonereg |= IEEE_PAUSE_MASK;
	this->mdioController_->phyWrite(this->phyAddr_, IEEE_AUTONEGO_ADVERTISE_REG,
			autonereg);

	control = this->mdioController_->phyRead(this->phyAddr_,
			IEEE_CONTROL_REG_OFFSET);
	control &= ~IEEE_CTRL_LINKSPEED_1000M;
	control &= ~IEEE_CTRL_LINKSPEED_100M;
	control &= ~IEEE_CTRL_LINKSPEED_10M;

	if (speed == 1000) {
		control |= IEEE_CTRL_LINKSPEED_1000M;

		/* Dont advertise PHY speed of 100 Mbps */
		autonereg = this->mdioController_->phyRead(this->phyAddr_,
				IEEE_AUTONEGO_ADVERTISE_REG);
		autonereg &= (~ADVERTISE_100);
		this->mdioController_->phyWrite(this->phyAddr_,
				IEEE_AUTONEGO_ADVERTISE_REG, autonereg);

		/* Dont advertise PHY speed of 10 Mbps */
		autonereg = this->mdioController_->phyRead(this->phyAddr_,
				IEEE_AUTONEGO_ADVERTISE_REG);
		autonereg &= (~ADVERTISE_10);
		this->mdioController_->phyWrite(this->phyAddr_,
				IEEE_AUTONEGO_ADVERTISE_REG, autonereg);

		/* Advertise PHY speed of 1000 Mbps */
		autonereg = this->mdioController_->phyRead(this->phyAddr_,
				IEEE_1000_ADVERTISE_REG_OFFSET);
		autonereg |= ADVERTISE_1000;
		this->mdioController_->phyWrite(this->phyAddr_,
				IEEE_1000_ADVERTISE_REG_OFFSET, autonereg);
	}
	else if (speed == 100) {
		control |= IEEE_CTRL_LINKSPEED_100M;

		/* Dont advertise PHY speed of 1000 Mbps */
		autonereg = this->mdioController_->phyRead(this->phyAddr_,
				IEEE_1000_ADVERTISE_REG_OFFSET);
		autonereg &= (~ADVERTISE_1000);
		this->mdioController_->phyWrite(this->phyAddr_,
				IEEE_1000_ADVERTISE_REG_OFFSET, autonereg);

		/* Dont advertise PHY speed of 10 Mbps */
		autonereg = this->mdioController_->phyRead(this->phyAddr_,
				IEEE_AUTONEGO_ADVERTISE_REG);
		autonereg &= (~ADVERTISE_10);
		this->mdioController_->phyWrite(this->phyAddr_,
				IEEE_AUTONEGO_ADVERTISE_REG, autonereg);

		/* Advertise PHY speed of 100 Mbps */
		autonereg = this->mdioController_->phyRead(this->phyAddr_,
				IEEE_AUTONEGO_ADVERTISE_REG);
		autonereg |= ADVERTISE_100;
		this->mdioController_->phyWrite(this->phyAddr_,
				IEEE_AUTONEGO_ADVERTISE_REG, autonereg);
	}
	else if (speed == 10) {
		control |= IEEE_CTRL_LINKSPEED_10M;

		/* Dont advertise PHY speed of 1000 Mbps */
		autonereg = this->mdioController_->phyRead(this->phyAddr_,
				IEEE_1000_ADVERTISE_REG_OFFSET);
		autonereg &= (~ADVERTISE_1000);
		this->mdioController_->phyWrite(this->phyAddr_,
				IEEE_1000_ADVERTISE_REG_OFFSET, autonereg);

		/* Dont advertise PHY speed of 100 Mbps */
		autonereg = this->mdioController_->phyRead(this->phyAddr_,
				IEEE_AUTONEGO_ADVERTISE_REG);
		autonereg &= (~ADVERTISE_100);
		this->mdioController_->phyWrite(this->phyAddr_,
				IEEE_AUTONEGO_ADVERTISE_REG, autonereg);

		/* Advertise PHY speed of 10 Mbps */
		autonereg = this->mdioController_->phyRead(this->phyAddr_,
				IEEE_AUTONEGO_ADVERTISE_REG);
		autonereg |= ADVERTISE_10;
		this->mdioController_->phyWrite(this->phyAddr_,
				IEEE_AUTONEGO_ADVERTISE_REG, autonereg);
	}

	this->mdioController_->phyWrite(this->phyAddr_, IEEE_CONTROL_REG_OFFSET,
			control | IEEE_CTRL_RESET_MASK);
	JbController::delayUs(400);
}



uint32_t EthernetPhyController::getSpeed(void)
{
	uint16_t phyIdentitifier =
			this->mdioController_->phyRead(this->phyAddr_, PHY_DETECT_REG1);
	if (phyIdentitifier == PHY_TI_IDENTIFIER)
		return this->getTiSpeed();
	else
		return this->getMarvellSpeed();
}



uint32_t EthernetPhyController::getTiSpeed(void)
{
	#if (USE_CONSOLE && ETHERNET_PHY_USE_CONSOLE)
	printf("Ethernet Phy Controller: Start TI PHY autonegotiation \r\n");
	#endif
	uint32_t tempPhyReg = this->mdioController_->phyRead(this->phyAddr_, 0x1F);
	tempPhyReg |= 0x4000;
	this->mdioController_->phyWrite(this->phyAddr_, 0x1F, tempPhyReg);
	tempPhyReg = this->mdioController_->phyRead(this->phyAddr_, 0x1F);

	tempPhyReg = this->mdioController_->phyRead(this->phyAddr_, 0);
	tempPhyReg |= 0x8000;
	this->mdioController_->phyWrite(this->phyAddr_, 0, tempPhyReg);

	JbController::delayMs(4000);

	tempPhyReg = this->mdioController_->phyRead(this->phyAddr_, 0);

	/* FIFO depth */
	this->mdioController_->phyWrite(this->phyAddr_, 0x10, 0x5048);
	this->mdioController_->phyWrite(this->phyAddr_, 0x10, tempPhyReg);
	tempPhyReg = this->mdioController_->phyRead(this->phyAddr_, 0x10);

	/* TX/RX tuning */
	/* Write to PHY_RGMIIDCTL */
	this->mdioController_->phyWrite(this->phyAddr_, PHY_REGCR, PHY_REGCR_ADDR);
	this->mdioController_->phyWrite(this->phyAddr_, PHY_ADDAR, PHY_RGMIIDCTL);
	this->mdioController_->phyWrite(this->phyAddr_, PHY_REGCR, PHY_REGCR_DATA);
	this->mdioController_->phyWrite(this->phyAddr_, PHY_ADDAR, 0xA8);

	/* Read PHY_RGMIIDCTL */
	this->mdioController_->phyWrite(this->phyAddr_, PHY_REGCR, PHY_REGCR_ADDR);
	this->mdioController_->phyWrite(this->phyAddr_, PHY_ADDAR, PHY_RGMIIDCTL);
	this->mdioController_->phyWrite(this->phyAddr_, PHY_REGCR, PHY_REGCR_DATA);
	tempPhyReg = this->mdioController_->phyRead(this->phyAddr_, PHY_ADDAR);

	/* Write PHY_RGMIICTL */
	this->mdioController_->phyWrite(this->phyAddr_, PHY_REGCR, PHY_REGCR_ADDR);
	this->mdioController_->phyWrite(this->phyAddr_, PHY_ADDAR, PHY_RGMIICTL);
	this->mdioController_->phyWrite(this->phyAddr_, PHY_REGCR, PHY_REGCR_DATA);
	this->mdioController_->phyWrite(this->phyAddr_, PHY_ADDAR, 0xD3);

	/* Read PHY_RGMIICTL */
	this->mdioController_->phyWrite(this->phyAddr_, PHY_REGCR, PHY_REGCR_ADDR);
	this->mdioController_->phyWrite(this->phyAddr_, PHY_ADDAR, PHY_RGMIICTL);
	this->mdioController_->phyWrite(this->phyAddr_, PHY_REGCR, PHY_REGCR_DATA);
	tempPhyReg = this->mdioController_->phyRead(this->phyAddr_, PHY_ADDAR);

	uint16_t control = this->mdioController_->phyRead(this->phyAddr_,
			IEEE_AUTONEGO_ADVERTISE_REG);
	control |= IEEE_ASYMMETRIC_PAUSE_MASK;
	control |= IEEE_PAUSE_MASK;
	control |= ADVERTISE_100;
	control |= ADVERTISE_10;
	this->mdioController_->phyWrite(this->phyAddr_, IEEE_AUTONEGO_ADVERTISE_REG,
			control);

	control = this->mdioController_->phyRead(this->phyAddr_,
			IEEE_1000_ADVERTISE_REG_OFFSET);
	control |= ADVERTISE_1000;
	this->mdioController_->phyWrite(this->phyAddr_, IEEE_1000_ADVERTISE_REG_OFFSET,
			control);

	control = this->mdioController_->phyRead(this->phyAddr_, IEEE_CONTROL_REG_OFFSET);
	control |= IEEE_CTRL_AUTONEGOTIATE_ENABLE;
	control |= IEEE_STAT_AUTONEGOTIATE_RESTART;
	this->mdioController_->phyWrite(this->phyAddr_, IEEE_CONTROL_REG_OFFSET, control);

	control = this->mdioController_->phyRead(this->phyAddr_, IEEE_CONTROL_REG_OFFSET);
	uint16_t status = this->mdioController_->phyRead(this->phyAddr_,
			IEEE_STATUS_REG_OFFSET);

	#if (USE_CONSOLE && ETHERNET_PHY_USE_CONSOLE)
	printf("Ethernet Phy Controller: Waiting for PHY to complete autonegotiation\r\n");
	#endif

	uint32_t timeoutCounter = 0;
	while ( !(status & IEEE_STAT_AUTONEGOTIATE_COMPLETE) ) {
		sleep(1);
		timeoutCounter++;
		if (timeoutCounter == 30) {
			#if (USE_CONSOLE && ETHERNET_PHY_USE_CONSOLE)
			printf("Ethernet Phy Controller: Autonegotiation error!\r\n");
			#endif
			return XST_FAILURE;
		}
		status = this->mdioController_->phyRead(this->phyAddr_, IEEE_STATUS_REG_OFFSET);
	}

	#if (USE_CONSOLE && ETHERNET_PHY_USE_CONSOLE)
	printf("Ethernet Phy Controller: Autonegotiation complete\r\n");
	#endif
	uint16_t statusSpeed = this->mdioController_->phyRead(this->phyAddr_, PHY_STS);
	if ((statusSpeed & 0xC000) == 0x8000)
		return 1000;
	else if ((statusSpeed & 0xC000) == 0x4000)
		return 100;
	else
		return 10;
	return XST_SUCCESS;
}



uint32_t EthernetPhyController::getMarvellSpeed(void)
{
	#if (USE_CONSOLE && ETHERNET_PHY_USE_CONSOLE)
	printf("Ethernet Phy Controller: Start Marvell PHY autonegotiation \r\n");
	#endif
	this->mdioController_->phyWrite(this->phyAddr_, IEEE_PAGE_ADDRESS_REGISTER, 2);
	uint16_t control = this->mdioController_->phyRead(this->phyAddr_,
			IEEE_CONTROL_REG_MAC);
	control |= IEEE_RGMII_TXRX_CLOCK_DELAYED_MASK;
	this->mdioController_->phyWrite(this->phyAddr_, IEEE_CONTROL_REG_MAC, control);

	this->mdioController_->phyWrite(this->phyAddr_, IEEE_PAGE_ADDRESS_REGISTER, 0);

	control = this->mdioController_->phyRead(this->phyAddr_,
			IEEE_AUTONEGO_ADVERTISE_REG);
	control |= IEEE_ASYMMETRIC_PAUSE_MASK;
	control |= IEEE_PAUSE_MASK;
	control |= ADVERTISE_100;
	control |= ADVERTISE_10;
	this->mdioController_->phyWrite(this->phyAddr_, IEEE_AUTONEGO_ADVERTISE_REG,
			control);

	control = this->mdioController_->phyRead(this->phyAddr_,
			IEEE_1000_ADVERTISE_REG_OFFSET);
	control |= ADVERTISE_1000;
	this->mdioController_->phyWrite(this->phyAddr_, IEEE_1000_ADVERTISE_REG_OFFSET,
			control);

	this->mdioController_->phyWrite(this->phyAddr_, IEEE_PAGE_ADDRESS_REGISTER, 0);
	control = this->mdioController_->phyRead(this->phyAddr_,
			IEEE_COPPER_SPECIFIC_CONTROL_REG);
	control |= (7 << 12);	/* max number of gigabit attempts */
	control |= (1 << 11);	/* enable downshift */
	this->mdioController_->phyWrite(this->phyAddr_, IEEE_COPPER_SPECIFIC_CONTROL_REG,
			control);
	control = this->mdioController_->phyRead(this->phyAddr_, IEEE_CONTROL_REG_OFFSET);
	control |= IEEE_CTRL_AUTONEGOTIATE_ENABLE;
	control |= IEEE_STAT_AUTONEGOTIATE_RESTART;
	this->mdioController_->phyWrite(this->phyAddr_, IEEE_CONTROL_REG_OFFSET, control);

	control = this->mdioController_->phyRead(this->phyAddr_, IEEE_CONTROL_REG_OFFSET);
	control |= IEEE_CTRL_RESET_MASK;
	this->mdioController_->phyWrite(this->phyAddr_, IEEE_CONTROL_REG_OFFSET, control);

	while (1) {
		control = this->mdioController_->phyRead(this->phyAddr_,
				IEEE_CONTROL_REG_OFFSET);
		if ((control & IEEE_CTRL_RESET_MASK) == 0)
			break;
	}
	uint16_t status = this->mdioController_->phyRead(this->phyAddr_,
			IEEE_STATUS_REG_OFFSET);
	#if (USE_CONSOLE && ETHERNET_PHY_USE_CONSOLE)
	printf("Ethernet Phy Controller: Waiting for PHY to complete autonegotiation\r\n");
	#endif
	uint32_t timeoutCounter = 0;
	while ( !(status & IEEE_STAT_AUTONEGOTIATE_COMPLETE) ) {
		sleep(1);
		this->mdioController_->phyRead(this->phyAddr_, IEEE_COPPER_SPECIFIC_STATUS_REG_2);
		timeoutCounter++;
		if (timeoutCounter == 30) {
			#if (USE_CONSOLE && ETHERNET_PHY_USE_CONSOLE)
			printf("Ethernet Phy Controller: Autonegotiation error!\r\n");
			#endif
			return XST_FAILURE;
		}
		status = this->mdioController_->phyRead(this->phyAddr_, IEEE_STATUS_REG_OFFSET);
	}

	#if (USE_CONSOLE && ETHERNET_PHY_USE_CONSOLE)
	printf("Ethernet Phy Controller: Autonegotiation complete\r\n");
	#endif
	uint16_t statusSpeed = this->mdioController_->phyRead(this->phyAddr_,
			IEEE_SPECIFIC_STATUS_REG);
	if (statusSpeed & 0x400) {
		uint32_t tempSpeed = statusSpeed & IEEE_SPEED_MASK;
		if (tempSpeed == IEEE_SPEED_1000)
			return 1000;
		else if(tempSpeed == IEEE_SPEED_100)
			return 100;
		else
			return 10;
	}
	return XST_SUCCESS;
}



void EthernetPhyController::setUpSlcrDivisors(uint32_t macBaseAddress, int32_t speed)
{
	volatile uint32_t slcrBaseAddress = 0;
	uint32_t SlcrDiv0 = 0;
	uint32_t SlcrDiv1 = 0;

	uint32_t gigeversion = ((Xil_In32(macBaseAddress + 0xFC)) >> 16) & 0xFFF;
	if (gigeversion == 2) {

		*(volatile uint32_t *)(SLCR_UNLOCK_ADDR) = SLCR_UNLOCK_KEY_VALUE;

		if (macBaseAddress == XPAR_XEMACPS_0_BASEADDR) {
			slcrBaseAddress = SLCR_GEM0_CLK_CTRL_ADDR;
		} else {
			slcrBaseAddress = SLCR_GEM1_CLK_CTRL_ADDR;
		}
		if (speed == 1000) {
			if (macBaseAddress == XPAR_XEMACPS_0_BASEADDR) {
#ifdef XPAR_PS7_ETHERNET_0_ENET_SLCR_1000MBPS_DIV0
				SlcrDiv0 = XPAR_PS7_ETHERNET_0_ENET_SLCR_1000MBPS_DIV0;
				SlcrDiv1 = XPAR_PS7_ETHERNET_0_ENET_SLCR_1000MBPS_DIV1;
#endif
			} else {
#ifdef XPAR_PS7_ETHERNET_1_ENET_SLCR_1000MBPS_DIV0
				SlcrDiv0 = XPAR_PS7_ETHERNET_1_ENET_SLCR_1000MBPS_DIV0;
				SlcrDiv1 = XPAR_PS7_ETHERNET_1_ENET_SLCR_1000MBPS_DIV1;
#endif
			}
		} else if (speed == 100) {
			if (macBaseAddress == XPAR_XEMACPS_0_BASEADDR) {
#ifdef XPAR_PS7_ETHERNET_0_ENET_SLCR_100MBPS_DIV0
				SlcrDiv0 = XPAR_PS7_ETHERNET_0_ENET_SLCR_100MBPS_DIV0;
				SlcrDiv1 = XPAR_PS7_ETHERNET_0_ENET_SLCR_100MBPS_DIV1;
#endif
			} else {
#ifdef XPAR_PS7_ETHERNET_1_ENET_SLCR_100MBPS_DIV0
				SlcrDiv0 = XPAR_PS7_ETHERNET_1_ENET_SLCR_100MBPS_DIV0;
				SlcrDiv1 = XPAR_PS7_ETHERNET_1_ENET_SLCR_100MBPS_DIV1;
#endif
			}
		} else {
			if (macBaseAddress == XPAR_XEMACPS_0_BASEADDR) {
#ifdef XPAR_PS7_ETHERNET_0_ENET_SLCR_10MBPS_DIV0
				SlcrDiv0 = XPAR_PS7_ETHERNET_0_ENET_SLCR_10MBPS_DIV0;
				SlcrDiv1 = XPAR_PS7_ETHERNET_0_ENET_SLCR_10MBPS_DIV1;
#endif
			} else {
#ifdef XPAR_PS7_ETHERNET_1_ENET_SLCR_10MBPS_DIV0
				SlcrDiv0 = XPAR_PS7_ETHERNET_1_ENET_SLCR_10MBPS_DIV0;
				SlcrDiv1 = XPAR_PS7_ETHERNET_1_ENET_SLCR_10MBPS_DIV1;
#endif
			}
		}
		uint32_t SlcrTxClkCntrl = *(volatile uint32_t *)(UINTPTR)(slcrBaseAddress);
		SlcrTxClkCntrl &= EMACPS_SLCR_DIV_MASK;
		SlcrTxClkCntrl |= (SlcrDiv1 << 20);
		SlcrTxClkCntrl |= (SlcrDiv0 << 8);
		*(volatile uint32_t *)(UINTPTR)(slcrBaseAddress) = SlcrTxClkCntrl;
		*(volatile uint32_t *)(SLCR_LOCK_ADDR) = SLCR_LOCK_KEY_VALUE;
	}
	return;
}

}
}
