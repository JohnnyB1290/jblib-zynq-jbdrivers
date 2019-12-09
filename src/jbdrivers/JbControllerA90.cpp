/**
 * @file
 * @brief JbController Core A9_0 Core class realization
 *
 *
 * @note
 * Copyright � 2019 Evgeniy Ivanov. Contacts: <strelok1290@gmail.com>
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

#ifdef CORE_A9_0
#include <string.h>
#include "jbdrivers/JbControllerA90.hpp"
#include "jbdrivers/IrqController.hpp"
#include "jbdrivers/Slcr.hpp"
#include "xdevcfg.h"
#include "xil_mmu.h"
#include "xil_cache_l.h"
#if JBDRIVERS_USE_PRIVATE_TIMER
#include "jbdrivers/PrivateVoidTimer.hpp"
#endif

#ifdef __cplusplus
extern "C"
{
#endif
void HandoffExit(u32 ExecAddr);
#ifdef __cplusplus
}
#endif

namespace jblib
{
namespace jbdrivers
{

using namespace jbkernel;

uint32_t JbController::boardGpios_[] = JBCONTROLLER_BOARD_GPIOS;
XGpioPs JbController::gpioPs_;


void JbController::initialize(void)
{
	static bool isInitialized = false;
	if(!isInitialized) {
		XGpioPs_Config* config = XGpioPs_LookupConfig(GPIO_DEVICE_ID);
		XGpioPs_CfgInitialize(&gpioPs_, config, config->BaseAddr);

		for(uint32_t i = 0; i < (sizeof(boardGpios_)/sizeof(uint32_t)); i++){
			XGpioPs_SetDirectionPin(&gpioPs_, boardGpios_[i], 1);
			XGpioPs_SetOutputEnablePin(&gpioPs_, boardGpios_[i], 1);
			/* Set the GPIO output to be low. */
			XGpioPs_WritePin(&gpioPs_, boardGpios_[i], 0x0);
		}
		isInitialized = true;
	}
}



void JbController::gpioOn(uint8_t number)
{
	XGpioPs_WritePin(&gpioPs_, boardGpios_[number], 0x1);
}



void JbController::gpioOff(uint8_t number)
{
	XGpioPs_WritePin(&gpioPs_, boardGpios_[number], 0x0);
}



void JbController::gpioTgl(uint8_t number)
{
	uint32_t data;
	data = XGpioPs_ReadPin(&gpioPs_, boardGpios_[number]);
	if(data) XGpioPs_WritePin(&gpioPs_, boardGpios_[number], 0);
	else XGpioPs_WritePin(&gpioPs_, boardGpios_[number], 1);
}



void JbController::startA91Core(uint32_t imageAddress)
{
	#ifdef USE_CONSOLE
	printf("Start of ARM A9_1\r\n");
	#endif
    //Disable cache on OCM
	setNonCached1Mb(0xFFFF0000);

	Xil_Out32(0xfffffff0, imageAddress);
	dmb(); //waits until write has finished
	#ifdef USE_CONSOLE
	printf("CPU0: sending the SEV to wake up CPU1\r\n");
	#endif
	sev();
}



void JbController::goToApp(uint32_t applicationAddress)
{
	#if JBDRIVERS_USE_PRIVATE_TIMER
	PrivateVoidTimer::getPrivateVoidTimer()->deinitialize();
	#endif
	for(uint32_t i = 0; i < XSCUGIC_MAX_NUM_INTR_INPUTS; i++)
		IrqController::getIrqController()->disableInterrupt(i);

	Xil_ExceptionDisable();

	*(volatile unsigned int *)(SLCR_UNLOCK_ADDR) = SLCR_UNLOCK_KEY_VALUE;

	*(volatile unsigned int *)(SLCR_DMAC_RST_CTRL_ADDR) |= 1;
	JbKernel::delayMs(10);
	*(volatile unsigned int *)(SLCR_DMAC_RST_CTRL_ADDR) &= ~1;

	*(volatile unsigned int *)(SLCR_GEM_RST_CTRL_ADDR) |= 0xF3;
	JbKernel::delayMs(10);
	*(volatile unsigned int *)(SLCR_GEM_RST_CTRL_ADDR) &= ~0xF3;

	*(volatile unsigned int *)(SLCR_SDIO_RST_CTRL_ADDR) |= 0x33;
	JbKernel::delayMs(10);
	*(volatile unsigned int *)(SLCR_SDIO_RST_CTRL_ADDR) &= ~0x33;

	*(volatile unsigned int *)(SLCR_GPIO_RST_CTRL_ADDR) |= 1;
	JbKernel::delayMs(10);
	*(volatile unsigned int *)(SLCR_GPIO_RST_CTRL_ADDR) &= ~1;

	*(volatile unsigned int *)(SLCR_LQSPI_RST_CTRL_ADDR) |= 3;
	JbKernel::delayMs(10);
	*(volatile unsigned int *)(SLCR_LQSPI_RST_CTRL_ADDR) &= ~3;

	*(volatile unsigned int *)(SLCR_FPGA_RST_CTRL_ADDR) |= 0x0F;
	JbKernel::delayMs(10);
	*(volatile unsigned int *)(SLCR_FPGA_RST_CTRL_ADDR) &= ~0x0F;

	*(unsigned int *)(SLCR_LOCK_ADDR) = SLCR_LOCK_KEY_VALUE;

	for(uint32_t i = 0; i<XSCUGIC_MAX_NUM_INTR_INPUTS; i++)
		XScuGic_WriteReg(XPAR_SCUGIC_0_CPU_BASEADDR, XSCUGIC_EOI_OFFSET, i);

	XDcfg_WriteReg(XPAR_XDCFG_0_BASEADDR, XDCFG_MULTIBOOT_ADDR_OFFSET,
			applicationAddress / 0x8000);
	HandoffExit(0);
}

}
}

#endif
