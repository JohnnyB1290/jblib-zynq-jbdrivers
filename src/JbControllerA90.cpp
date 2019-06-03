/**
 * @file
 * @brief JbController Core A9_0 Core class realization
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

#ifdef CORE_A9_0
#include <string.h>
#include "JbControllerA90.hpp"
#include "IrqController.hpp"
#include "Slcr.hpp"
#include "xdevcfg.h"
#include "xil_mmu.h"
#include "xil_cache_l.h"
#if JBDRIVERS_USE_PRIVATE_TIMER
#include "PrivateVoidTimer.hpp"
#endif
#ifdef USE_CONSOLE
#include <stdio.h>
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
bool JbController::isInitialized_ = false;
IVoidCallback* JbController::mainProcedures_[JBCONTROLLER_NUM_MAIN_PROCEDURES];
void* JbController::mainProceduresParameters_[JBCONTROLLER_NUM_MAIN_PROCEDURES];



void JbController::initialize(void)
{
	if(!isInitialized_) {
		for(uint32_t i = 0; i < JBCONTROLLER_NUM_MAIN_PROCEDURES; i++){
			mainProcedures_[i] = NULL;
			mainProceduresParameters_[i] = NULL;
		}
		XGpioPs_Config* config = XGpioPs_LookupConfig(GPIO_DEVICE_ID);
		XGpioPs_CfgInitialize(&gpioPs_, config, config->BaseAddr);

		for(uint32_t i = 0; i < (sizeof(boardGpios_)/sizeof(uint32_t)); i++){
			XGpioPs_SetDirectionPin(&gpioPs_, boardGpios_[i], 1);
			XGpioPs_SetOutputEnablePin(&gpioPs_, boardGpios_[i], 1);
			/* Set the GPIO output to be low. */
			XGpioPs_WritePin(&gpioPs_, boardGpios_[i], 0x0);
		}
		isInitialized_ = true;
	}
}



void JbController::delayMs(uint32_t ms)  //For 204MHz Clock
{
	for(uint32_t i = 0; i < ms; i++)
		for(uint32_t j = 0; j < JBCONTROLLER_NUM_NOP_DELAY_MS; j++)
			asm ("nop");
}



void JbController::delayUs(uint32_t us) //For 204MHz Clock
{
	for(uint32_t i = 0; i < us * JBCONTROLLER_NUM_NOP_DELAY_US; i++)
		asm ("nop");
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
	delayMs(10);
	*(volatile unsigned int *)(SLCR_DMAC_RST_CTRL_ADDR) &= ~1;

	*(volatile unsigned int *)(SLCR_GEM_RST_CTRL_ADDR) |= 0xF3;
	delayMs(10);
	*(volatile unsigned int *)(SLCR_GEM_RST_CTRL_ADDR) &= ~0xF3;

	*(volatile unsigned int *)(SLCR_SDIO_RST_CTRL_ADDR) |= 0x33;
	delayMs(10);
	*(volatile unsigned int *)(SLCR_SDIO_RST_CTRL_ADDR) &= ~0x33;

	*(volatile unsigned int *)(SLCR_GPIO_RST_CTRL_ADDR) |= 1;
	delayMs(10);
	*(volatile unsigned int *)(SLCR_GPIO_RST_CTRL_ADDR) &= ~1;

	*(volatile unsigned int *)(SLCR_LQSPI_RST_CTRL_ADDR) |= 3;
	delayMs(10);
	*(volatile unsigned int *)(SLCR_LQSPI_RST_CTRL_ADDR) &= ~3;

	*(volatile unsigned int *)(SLCR_FPGA_RST_CTRL_ADDR) |= 0x0F;
	delayMs(10);
	*(volatile unsigned int *)(SLCR_FPGA_RST_CTRL_ADDR) &= ~0x0F;

	*(unsigned int *)(SLCR_LOCK_ADDR) = SLCR_LOCK_KEY_VALUE;

	for(uint32_t i = 0; i<XSCUGIC_MAX_NUM_INTR_INPUTS; i++)
		XScuGic_WriteReg(XPAR_SCUGIC_0_CPU_BASEADDR, XSCUGIC_EOI_OFFSET, i);

	XDcfg_WriteReg(XPAR_XDCFG_0_BASEADDR, XDCFG_MULTIBOOT_ADDR_OFFSET,
			applicationAddress / 0x8000);
	HandoffExit(0);
}



void JbController::doMain(void)
{
	for(uint32_t i = 0; i < JBCONTROLLER_NUM_MAIN_PROCEDURES; i++) {
		if(mainProcedures_[i])
			mainProcedures_[i]->voidCallback(NULL,
					mainProceduresParameters_[i]);
		else
			break;
	}
}



void JbController::addMainProcedure(IVoidCallback* callback, void* parameter)
{
	for(uint32_t i = 0; i < JBCONTROLLER_NUM_MAIN_PROCEDURES; i++) {
		if((mainProcedures_[i] == callback) &&
				mainProceduresParameters_[i] == parameter){
			break;
		}
		if(mainProcedures_[i] == NULL) {
			mainProcedures_[i] = callback;
			mainProceduresParameters_[i] = parameter;
			break;
		}
	}
}



void JbController::deleteMainProcedure(IVoidCallback* callback, void* parameter)
{
	uint32_t index = 0;
	for(uint32_t i = 0; i < JBCONTROLLER_NUM_MAIN_PROCEDURES; i++) {
		if((mainProcedures_[i] == callback) &&
				mainProceduresParameters_[i] == parameter){
			break;
		}
		else
			index++;
	}
	if(index == (JBCONTROLLER_NUM_MAIN_PROCEDURES-1)) {
		if((mainProcedures_[index] == callback) &&
				mainProceduresParameters_[index] == parameter){
			mainProcedures_[index] = NULL;
			mainProceduresParameters_[index] = NULL;
		}
	}
	else {
		for(uint32_t i = index; i < (JBCONTROLLER_NUM_MAIN_PROCEDURES - 1); i++) {
			mainProcedures_[i] = mainProcedures_[i+1];
			mainProceduresParameters_[i] = mainProceduresParameters_[i+1];
			if(mainProcedures_[i+1] == NULL)
				break;
		}
	}
}



void JbController::addMainProcedure(IVoidCallback* callback)
{
	addMainProcedure(callback, NULL);
}



void JbController::deleteMainProcedure(IVoidCallback* callback)
{
	deleteMainProcedure(callback, NULL);
}



uint32_t JbController::getHeapFree(void)
{
    uint32_t ret = 1024;
    __disable_irq();
    void* ptr = malloc(ret);
    while(ptr != NULL){
        free(ptr);
        ret += 1024;
        ptr = malloc(ret);
    }
    __enable_irq();
    return ret;
}

}
}

#endif
