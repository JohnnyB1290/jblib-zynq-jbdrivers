/**
 * @file
 * @brief SLCR Defines
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

#ifndef SLCR_HPP_
#define SLCR_HPP_

#include "jbkernel/jb_common.h"

#define DCFG_DEVICE_ID					XPAR_XDCFG_0_DEVICE_ID

#define SLCR_LOCK_ADDR					(XPS_SYS_CTRL_BASEADDR + 0x4)
#define SLCR_UNLOCK_ADDR				(XPS_SYS_CTRL_BASEADDR + 0x8)

#define SLCR_PS_RST_CTRL_ADDR			(XPS_SYS_CTRL_BASEADDR + 0x200) /* PS Software Reset Control */
#define SLCR_DDR_RST_CTRL_ADDR			(XPS_SYS_CTRL_BASEADDR + 0x204) /* DDR Software Reset Control */
#define SLCR_TOPSW_RST_CTRL_ADDR		(XPS_SYS_CTRL_BASEADDR + 0x208) /* Central Interconnect Reset Control */
#define SLCR_DMAC_RST_CTRL_ADDR			(XPS_SYS_CTRL_BASEADDR + 0x20C) /* DMAC Software Reset Control */
#define SLCR_USB_RST_CTRL_ADDR			(XPS_SYS_CTRL_BASEADDR + 0x210) /* USB Software Reset Control */
#define SLCR_GEM_RST_CTRL_ADDR			(XPS_SYS_CTRL_BASEADDR + 0x214) /* Gigabit Ethernet SW Reset Control */
#define SLCR_SDIO_RST_CTRL_ADDR			(XPS_SYS_CTRL_BASEADDR + 0x218) /* SDIO Software Reset Control */
#define SLCR_SPI_RST_CTRL_ADDR			(XPS_SYS_CTRL_BASEADDR + 0x21C) /* SPI Software Reset Control */
#define SLCR_CAN_RST_CTRL_ADDR			(XPS_SYS_CTRL_BASEADDR + 0x220) /* CAN Software Reset Control */
#define SLCR_UART_RST_CTRL_ADDR			(XPS_SYS_CTRL_BASEADDR + 0x228) /* UART Software Reset Control */
#define SLCR_GPIO_RST_CTRL_ADDR			(XPS_SYS_CTRL_BASEADDR + 0x22C) /* GPIO Software Reset Control */
#define SLCR_LQSPI_RST_CTRL_ADDR		(XPS_SYS_CTRL_BASEADDR + 0x230) /* Quad SPI Software Reset Control */
#define SLCR_SMC_RST_CTRL_ADDR			(XPS_SYS_CTRL_BASEADDR + 0x234) /* SMC Software Reset Control */
#define SLCR_OCM_RST_CTRL_ADDR			(XPS_SYS_CTRL_BASEADDR + 0x238) /* OCM Software Reset Control */
#define SLCR_FPGA_RST_CTRL_ADDR			(XPS_SYS_CTRL_BASEADDR + 0x240) /* FPGA Software Reset Control */
#define SLCR_A9_CPU_RST_CTRL_ADDR		(XPS_SYS_CTRL_BASEADDR + 0x244) /* CPU Software Reset Control */
#define SLCR_AWDT_CTRL_ADDR				(XPS_SYS_CTRL_BASEADDR + 0x24C) /* CPU Software Reset Control */
#define SLCR_REBOOT_STATUS_ADDR			(XPS_SYS_CTRL_BASEADDR + 0x258) /* PS Reboot Status */

#define SLCR_CAN_CLK_CTRL_ADDR			(XPS_SYS_CTRL_BASEADDR + 0x15C) /* CAN Ref Clock Control  */

#define SLCR_LOCK_KEY_VALUE				0x767B
#define SLCR_UNLOCK_KEY_VALUE			0xDF0D

#endif /* SLCR_HPP_ */
