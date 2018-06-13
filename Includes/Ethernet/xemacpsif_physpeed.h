/*
 * xemacpsif_physpeed.h
 *
 *  Created on: 6 сент. 2016 г.
 *      Author: Johnny
 */

#ifndef SRC_XEMACPSIF_PHYSPEED_H_
#define SRC_XEMACPSIF_PHYSPEED_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "xemacps.h"		/* defines XEmacPs API */
#include "sleep.h"
#include "xparameters_ps.h"	/* defines XPAR values */
#include"xparameters.h"
#include "xil_types.h"
#include "xil_printf.h"

#define ZYNQ_EMACPS_0_BASEADDR XPAR_XEMACPS_0_BASEADDR
#define ZYNQ_EMACPS_1_BASEADDR XPAR_XEMACPS_1_BASEADDR

uint32_t detect_phy(XEmacPs *xemacpsp);
uint32_t phy_setup (XEmacPs* xemacpsp_mdio,XEmacPs* xemacpsp, uint32_t phy_addr);
uint32_t configure_IEEE_phy_speed(XEmacPs *xemacpsp, uint32_t phy_addr, uint32_t speed);
void SetUpSLCRDivisors(uint32_t mac_baseaddr, int32_t speed);

#ifdef __cplusplus
}
#endif

#endif /* SRC_XEMACPSIF_PHYSPEED_H_ */
