/*
 * EthernetPhy.cpp
 *
 *  Created on: 29 но€б. 2018 г.
 *      Author: Stalker1290
 */

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

#include "Ethernet/EthernetPhy.hpp"
#include "CONTROLLER.hpp"


EthernetPhy_t::EthernetPhy_t(MDIO_t* mdioPtr, uint32_t phyAddr){
	this->mdioPtr = mdioPtr;
	this->phyAddr = phyAddr;
}

void EthernetPhy_t::configureSpeed(uint32_t speed){

	uint16_t control;
	uint16_t autonereg;

	this->mdioPtr->phyWrite(this->phyAddr, IEEE_PAGE_ADDRESS_REGISTER, 2);
	control = this->mdioPtr->phyRead(this->phyAddr, IEEE_CONTROL_REG_MAC);
	control |= IEEE_RGMII_TXRX_CLOCK_DELAYED_MASK;
	this->mdioPtr->phyWrite(this->phyAddr, IEEE_CONTROL_REG_MAC, control);

	this->mdioPtr->phyWrite(this->phyAddr, IEEE_PAGE_ADDRESS_REGISTER, 0);

	autonereg = this->mdioPtr->phyRead(this->phyAddr, IEEE_AUTONEGO_ADVERTISE_REG);
	autonereg |= IEEE_ASYMMETRIC_PAUSE_MASK;
	autonereg |= IEEE_PAUSE_MASK;
	this->mdioPtr->phyWrite(this->phyAddr, IEEE_AUTONEGO_ADVERTISE_REG, autonereg);

	control = this->mdioPtr->phyRead(this->phyAddr, IEEE_CONTROL_REG_OFFSET);
	control &= ~IEEE_CTRL_LINKSPEED_1000M;
	control &= ~IEEE_CTRL_LINKSPEED_100M;
	control &= ~IEEE_CTRL_LINKSPEED_10M;

	if (speed == 1000) {
		control |= IEEE_CTRL_LINKSPEED_1000M;

		/* Dont advertise PHY speed of 100 Mbps */
		autonereg = this->mdioPtr->phyRead(this->phyAddr, IEEE_AUTONEGO_ADVERTISE_REG);
		autonereg &= (~ADVERTISE_100);
		this->mdioPtr->phyWrite(this->phyAddr, IEEE_AUTONEGO_ADVERTISE_REG, autonereg);

		/* Dont advertise PHY speed of 10 Mbps */
		autonereg = this->mdioPtr->phyRead(this->phyAddr, IEEE_AUTONEGO_ADVERTISE_REG);
		autonereg &= (~ADVERTISE_10);
		this->mdioPtr->phyWrite(this->phyAddr, IEEE_AUTONEGO_ADVERTISE_REG, autonereg);

		/* Advertise PHY speed of 1000 Mbps */
		autonereg = this->mdioPtr->phyRead(this->phyAddr, IEEE_1000_ADVERTISE_REG_OFFSET);
		autonereg |= ADVERTISE_1000;
		this->mdioPtr->phyWrite(this->phyAddr, IEEE_1000_ADVERTISE_REG_OFFSET, autonereg);
	}
	else if (speed == 100) {
		control |= IEEE_CTRL_LINKSPEED_100M;

		/* Dont advertise PHY speed of 1000 Mbps */
		autonereg = this->mdioPtr->phyRead(this->phyAddr, IEEE_1000_ADVERTISE_REG_OFFSET);
		autonereg &= (~ADVERTISE_1000);
		this->mdioPtr->phyWrite(this->phyAddr, IEEE_1000_ADVERTISE_REG_OFFSET, autonereg);

		/* Dont advertise PHY speed of 10 Mbps */
		autonereg = this->mdioPtr->phyRead(this->phyAddr, IEEE_AUTONEGO_ADVERTISE_REG);
		autonereg &= (~ADVERTISE_10);
		this->mdioPtr->phyWrite(this->phyAddr, IEEE_AUTONEGO_ADVERTISE_REG, autonereg);

		/* Advertise PHY speed of 100 Mbps */
		autonereg = this->mdioPtr->phyRead(this->phyAddr, IEEE_AUTONEGO_ADVERTISE_REG);
		autonereg |= ADVERTISE_100;
		this->mdioPtr->phyWrite(this->phyAddr, IEEE_AUTONEGO_ADVERTISE_REG, autonereg);
	}
	else if (speed == 10) {
		control |= IEEE_CTRL_LINKSPEED_10M;

		/* Dont advertise PHY speed of 1000 Mbps */
		autonereg = this->mdioPtr->phyRead(this->phyAddr, IEEE_1000_ADVERTISE_REG_OFFSET);
		autonereg &= (~ADVERTISE_1000);
		this->mdioPtr->phyWrite(this->phyAddr, IEEE_1000_ADVERTISE_REG_OFFSET, autonereg);

		/* Dont advertise PHY speed of 100 Mbps */
		autonereg = this->mdioPtr->phyRead(this->phyAddr, IEEE_AUTONEGO_ADVERTISE_REG);
		autonereg &= (~ADVERTISE_100);
		this->mdioPtr->phyWrite(this->phyAddr, IEEE_AUTONEGO_ADVERTISE_REG, autonereg);

		/* Advertise PHY speed of 10 Mbps */
		autonereg = this->mdioPtr->phyRead(this->phyAddr, IEEE_AUTONEGO_ADVERTISE_REG);
		autonereg |= ADVERTISE_10;
		this->mdioPtr->phyWrite(this->phyAddr, IEEE_AUTONEGO_ADVERTISE_REG, autonereg);
	}

	this->mdioPtr->phyWrite(this->phyAddr, IEEE_CONTROL_REG_OFFSET,control | IEEE_CTRL_RESET_MASK);

	CONTROLLER_t::get_CONTROLLER()->_delay_us(400);
}

uint32_t EthernetPhy_t::getSpeed(void){
	uint16_t phy_identity;

	phy_identity = this->mdioPtr->phyRead(this->phyAddr, PHY_DETECT_REG1);
	if (phy_identity == PHY_TI_IDENTIFIER) return this->getTISpeed();
	else return this->getMarvellSpeed();
}

uint32_t EthernetPhy_t::getTISpeed(void){
	uint16_t control;
	uint16_t status;
	uint16_t status_speed;
	uint32_t timeout_counter = 0;
	uint32_t phyregtemp;

#ifdef USE_CONSOLE
#ifdef EthM_console
	printf("Start TI PHY autonegotiation \n\r");
#endif
#endif

	phyregtemp = this->mdioPtr->phyRead(this->phyAddr, 0x1F);
	phyregtemp |= 0x4000;
	this->mdioPtr->phyWrite(this->phyAddr, 0x1F, phyregtemp);
	phyregtemp = this->mdioPtr->phyRead(this->phyAddr, 0x1F);

	phyregtemp = this->mdioPtr->phyRead(this->phyAddr, 0);
	phyregtemp |= 0x8000;
	this->mdioPtr->phyWrite(this->phyAddr, 0, phyregtemp);

	CONTROLLER_t::get_CONTROLLER()->_delay_ms(4000);

	phyregtemp = this->mdioPtr->phyRead(this->phyAddr, 0);

	/* FIFO depth */
	this->mdioPtr->phyWrite(this->phyAddr, 0x10, 0x5048);
	this->mdioPtr->phyWrite(this->phyAddr, 0x10, phyregtemp);
	phyregtemp = this->mdioPtr->phyRead(this->phyAddr, 0x10);

	/* TX/RX tuning */
	/* Write to PHY_RGMIIDCTL */
	this->mdioPtr->phyWrite(this->phyAddr, PHY_REGCR, PHY_REGCR_ADDR);
	this->mdioPtr->phyWrite(this->phyAddr, PHY_ADDAR, PHY_RGMIIDCTL);
	this->mdioPtr->phyWrite(this->phyAddr, PHY_REGCR, PHY_REGCR_DATA);
	this->mdioPtr->phyWrite(this->phyAddr, PHY_ADDAR, 0xA8);

	/* Read PHY_RGMIIDCTL */
	this->mdioPtr->phyWrite(this->phyAddr, PHY_REGCR, PHY_REGCR_ADDR);
	this->mdioPtr->phyWrite(this->phyAddr, PHY_ADDAR, PHY_RGMIIDCTL);
	this->mdioPtr->phyWrite(this->phyAddr, PHY_REGCR, PHY_REGCR_DATA);
	phyregtemp = this->mdioPtr->phyRead(this->phyAddr, PHY_ADDAR);

	/* Write PHY_RGMIICTL */
	this->mdioPtr->phyWrite(this->phyAddr, PHY_REGCR, PHY_REGCR_ADDR);
	this->mdioPtr->phyWrite(this->phyAddr, PHY_ADDAR, PHY_RGMIICTL);
	this->mdioPtr->phyWrite(this->phyAddr, PHY_REGCR, PHY_REGCR_DATA);
	this->mdioPtr->phyWrite(this->phyAddr, PHY_ADDAR, 0xD3);

	/* Read PHY_RGMIICTL */
	this->mdioPtr->phyWrite(this->phyAddr, PHY_REGCR, PHY_REGCR_ADDR);
	this->mdioPtr->phyWrite(this->phyAddr, PHY_ADDAR, PHY_RGMIICTL);
	this->mdioPtr->phyWrite(this->phyAddr, PHY_REGCR, PHY_REGCR_DATA);
	phyregtemp = this->mdioPtr->phyRead(this->phyAddr, PHY_ADDAR);

	control = this->mdioPtr->phyRead(this->phyAddr, IEEE_AUTONEGO_ADVERTISE_REG);
	control |= IEEE_ASYMMETRIC_PAUSE_MASK;
	control |= IEEE_PAUSE_MASK;
	control |= ADVERTISE_100;
	control |= ADVERTISE_10;
	this->mdioPtr->phyWrite(this->phyAddr, IEEE_AUTONEGO_ADVERTISE_REG, control);

	control = this->mdioPtr->phyRead(this->phyAddr, IEEE_1000_ADVERTISE_REG_OFFSET);
	control |= ADVERTISE_1000;
	this->mdioPtr->phyWrite(this->phyAddr, IEEE_1000_ADVERTISE_REG_OFFSET,control);

	control = this->mdioPtr->phyRead(this->phyAddr, IEEE_CONTROL_REG_OFFSET);
	control |= IEEE_CTRL_AUTONEGOTIATE_ENABLE;
	control |= IEEE_STAT_AUTONEGOTIATE_RESTART;
	this->mdioPtr->phyWrite(this->phyAddr, IEEE_CONTROL_REG_OFFSET, control);

	control = this->mdioPtr->phyRead(this->phyAddr, IEEE_CONTROL_REG_OFFSET);
	status = this->mdioPtr->phyRead(this->phyAddr, IEEE_STATUS_REG_OFFSET);

#ifdef USE_CONSOLE
#ifdef EthM_console
	printf("Waiting for PHY to complete autonegotiation.\n\r");
#endif
#endif

	while ( !(status & IEEE_STAT_AUTONEGOTIATE_COMPLETE) ) {
		sleep(1);
		timeout_counter++;

		if (timeout_counter == 30) {
			#ifdef USE_CONSOLE
			#ifdef EthM_console
			printf("Auto negotiation error \n\r");
			#endif
			#endif
			return XST_FAILURE;
		}
		status = this->mdioPtr->phyRead(this->phyAddr, IEEE_STATUS_REG_OFFSET);
	}

#ifdef USE_CONSOLE
#ifdef EthM_console
	printf("Autonegotiation complete \n\r");
#endif
#endif

	status_speed = this->mdioPtr->phyRead(this->phyAddr, PHY_STS);
	if ((status_speed & 0xC000) == 0x8000) {
		return 1000;
	} else if ((status_speed & 0xC000) == 0x4000) {
		return 100;
	} else {
		return 10;
	}

	return XST_SUCCESS;
}

uint32_t EthernetPhy_t::getMarvellSpeed(void){
	uint16_t control;
	uint16_t status;
	uint16_t status_speed;
	uint32_t timeout_counter = 0;
	uint32_t temp_speed;

#ifdef USE_CONSOLE
#ifdef EthM_console
	printf("Start Marvell PHY autonegotiation \n\r");
#endif
#endif

	this->mdioPtr->phyWrite(this->phyAddr, IEEE_PAGE_ADDRESS_REGISTER, 2);
	control = this->mdioPtr->phyRead(this->phyAddr, IEEE_CONTROL_REG_MAC);
	control |= IEEE_RGMII_TXRX_CLOCK_DELAYED_MASK;
	this->mdioPtr->phyWrite(this->phyAddr, IEEE_CONTROL_REG_MAC, control);

	this->mdioPtr->phyWrite(this->phyAddr, IEEE_PAGE_ADDRESS_REGISTER, 0);

	control = this->mdioPtr->phyRead(this->phyAddr, IEEE_AUTONEGO_ADVERTISE_REG);
	control |= IEEE_ASYMMETRIC_PAUSE_MASK;
	control |= IEEE_PAUSE_MASK;
	control |= ADVERTISE_100;
	control |= ADVERTISE_10;
	this->mdioPtr->phyWrite(this->phyAddr, IEEE_AUTONEGO_ADVERTISE_REG, control);

	control = this->mdioPtr->phyRead(this->phyAddr, IEEE_1000_ADVERTISE_REG_OFFSET);
	control |= ADVERTISE_1000;
	this->mdioPtr->phyWrite(this->phyAddr, IEEE_1000_ADVERTISE_REG_OFFSET,control);

	this->mdioPtr->phyWrite(this->phyAddr, IEEE_PAGE_ADDRESS_REGISTER, 0);
	control = this->mdioPtr->phyRead(this->phyAddr, IEEE_COPPER_SPECIFIC_CONTROL_REG);
	control |= (7 << 12);	/* max number of gigabit attempts */
	control |= (1 << 11);	/* enable downshift */
	this->mdioPtr->phyWrite(this->phyAddr, IEEE_COPPER_SPECIFIC_CONTROL_REG,
																control);
	control = this->mdioPtr->phyRead(this->phyAddr, IEEE_CONTROL_REG_OFFSET);
	control |= IEEE_CTRL_AUTONEGOTIATE_ENABLE;
	control |= IEEE_STAT_AUTONEGOTIATE_RESTART;
	this->mdioPtr->phyWrite(this->phyAddr, IEEE_CONTROL_REG_OFFSET, control);

	control = this->mdioPtr->phyRead(this->phyAddr, IEEE_CONTROL_REG_OFFSET);
	control |= IEEE_CTRL_RESET_MASK;
	this->mdioPtr->phyWrite(this->phyAddr, IEEE_CONTROL_REG_OFFSET, control);

	while (1) {
		control = this->mdioPtr->phyRead(this->phyAddr, IEEE_CONTROL_REG_OFFSET);
		if ((control & IEEE_CTRL_RESET_MASK) == 0) break;
	}

	status = this->mdioPtr->phyRead(this->phyAddr, IEEE_STATUS_REG_OFFSET);

#ifdef USE_CONSOLE
#ifdef EthM_console
	printf("Waiting for PHY to complete autonegotiation.\n\r");
#endif
#endif

	while ( !(status & IEEE_STAT_AUTONEGOTIATE_COMPLETE) ) {
		sleep(1);
		this->mdioPtr->phyRead(this->phyAddr,IEEE_COPPER_SPECIFIC_STATUS_REG_2);
		timeout_counter++;

		if (timeout_counter == 30) {
			#ifdef USE_CONSOLE
			#ifdef EthM_console
			printf("Auto negotiation error \n\r");
			#endif
			#endif
			return XST_FAILURE;
		}
		status = this->mdioPtr->phyRead(this->phyAddr, IEEE_STATUS_REG_OFFSET);
	}

#ifdef USE_CONSOLE
#ifdef EthM_console
	printf("Autonegotiation complete \n\r");
#endif
#endif

	status_speed = this->mdioPtr->phyRead(this->phyAddr,IEEE_SPECIFIC_STATUS_REG);
	if (status_speed & 0x400) {
		temp_speed = status_speed & IEEE_SPEED_MASK;

		if (temp_speed == IEEE_SPEED_1000)
			return 1000;
		else if(temp_speed == IEEE_SPEED_100)
			return 100;
		else
			return 10;
	}

	return XST_SUCCESS;
}

void EthernetPhy_t::setUpSLCRDivisors(uint32_t macBaseaddr, int32_t speed){

	volatile uint32_t slcrBaseAddress;
	uint32_t SlcrDiv0 = 0;
	uint32_t SlcrDiv1 = 0;
	uint32_t SlcrTxClkCntrl;
	uint32_t gigeversion;

	gigeversion = ((Xil_In32(macBaseaddr + 0xFC)) >> 16) & 0xFFF;
	if (gigeversion == 2) {

		*(volatile uint32_t *)(SLCR_UNLOCK_ADDR) = SLCR_UNLOCK_KEY_VALUE;

		if (macBaseaddr == XPAR_XEMACPS_0_BASEADDR) {
			slcrBaseAddress = SLCR_GEM0_CLK_CTRL_ADDR;
		} else {
			slcrBaseAddress = SLCR_GEM1_CLK_CTRL_ADDR;
		}
		if (speed == 1000) {
			if (macBaseaddr == XPAR_XEMACPS_0_BASEADDR) {
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
			if (macBaseaddr == XPAR_XEMACPS_0_BASEADDR) {
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
			if (macBaseaddr == XPAR_XEMACPS_0_BASEADDR) {
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
		SlcrTxClkCntrl = *(volatile uint32_t *)(UINTPTR)(slcrBaseAddress);
		SlcrTxClkCntrl &= EMACPS_SLCR_DIV_MASK;
		SlcrTxClkCntrl |= (SlcrDiv1 << 20);
		SlcrTxClkCntrl |= (SlcrDiv0 << 8);
		*(volatile uint32_t *)(UINTPTR)(slcrBaseAddress) = SlcrTxClkCntrl;
		*(volatile uint32_t *)(SLCR_LOCK_ADDR) = SLCR_LOCK_KEY_VALUE;
	}
	return;
}

