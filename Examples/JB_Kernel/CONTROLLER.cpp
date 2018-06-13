/*
 * CONTROLLER.cpp
 *
 *  Created on: 22.11.2017
 *      Author: Stalker1290
 */

#include "CONTROLLER.hpp"
#include "SLCR.hpp"
#include "xdevcfg.h"

#ifdef __cplusplus
extern "C"
{
#endif
void HandoffExit(u32 ExecAddr);
#ifdef __cplusplus
}
#endif


CONTROLLER_t* CONTROLLER_t::CONTROLLER_ptr = (CONTROLLER_t*)NULL;

uint32_t CONTROLLER_t::BOARD_GPIOs[]  = {LED0_PIN,LED1_PIN};


CONTROLLER_t* CONTROLLER_t::get_CONTROLLER(void)
{
	if(CONTROLLER_t::CONTROLLER_ptr == (CONTROLLER_t*)NULL) CONTROLLER_t::CONTROLLER_ptr =  new CONTROLLER_t();
	return CONTROLLER_t::CONTROLLER_ptr;
}


CONTROLLER_t::CONTROLLER_t(void)
{
	this->Go_to_App = false;
	this->Addr_to_Go = 0;
	this->Initialize_done = 0;
	for(int i = 0; i<main_proc_num; i++)
	{
		this->main_procedures[i] = (Callback_Interface_t*)NULL;
		this->main_procedures_data[i] = NULL;
	}
}


void CONTROLLER_t::Initialize(void)
{
	XGpioPs_Config* ConfigPtr;

	if(this->Initialize_done == 0)
	{
		XDcfg_WriteReg(XPAR_XDCFG_0_BASEADDR,XDCFG_MULTIBOOT_ADDR_OFFSET,QSPI_FLASH_APPLICATION_ADDR/0x8000);

		ConfigPtr = XGpioPs_LookupConfig(GPIO_DEVICE_ID);
		XGpioPs_CfgInitialize(&this->GpioPs, ConfigPtr, ConfigPtr->BaseAddr);

		for(uint32_t i=0; i<( sizeof(CONTROLLER_t::BOARD_GPIOs)/sizeof(uint32_t) ); i++)
		{
			XGpioPs_SetDirectionPin(&this->GpioPs, CONTROLLER_t::BOARD_GPIOs[i], 1);
			XGpioPs_SetOutputEnablePin(&this->GpioPs, CONTROLLER_t::BOARD_GPIOs[i], 1);
			/* Set the GPIO output to be low. */
			XGpioPs_WritePin(&this->GpioPs, CONTROLLER_t::BOARD_GPIOs[i], 0x0);
		}
		this->LED_music();
		this->Initialize_done = 1;
	}

}

void CONTROLLER_t::GPIO_ON(uint8_t num)
{
	XGpioPs_WritePin(&this->GpioPs, CONTROLLER_t::BOARD_GPIOs[num], 0x1);
}

void CONTROLLER_t::GPIO_OFF(uint8_t num)
{
	XGpioPs_WritePin(&this->GpioPs, CONTROLLER_t::BOARD_GPIOs[num], 0x0);
}

void CONTROLLER_t::GPIO_TGL(uint8_t num)
{
	uint32_t data;
	data = XGpioPs_ReadPin(&this->GpioPs, CONTROLLER_t::BOARD_GPIOs[num]);
	if(data) XGpioPs_WritePin(&this->GpioPs, CONTROLLER_t::BOARD_GPIOs[num], 0);
	else XGpioPs_WritePin(&this->GpioPs, CONTROLLER_t::BOARD_GPIOs[num], 1);
}

void CONTROLLER_t::_delay_ms(uint32_t ms)
{
	for(uint32_t i=0;i<ms;i++)
	{
		for(uint32_t j=0; j<250000; j++) asm ("nop");
	}
}

void CONTROLLER_t::_delay_us(uint32_t us)
{
	for(uint32_t i=0;i<us*250;i++) asm ("nop");
}

void CONTROLLER_t::Start_A7_1_APP(uint32_t img_addr)
{
	#ifdef USE_CONSOLE
	printf("Start of ARM A9_1\n\r");
	#endif
}

void CONTROLLER_t::Reset_periph(void)
{



}

void CONTROLLER_t::Soft_reset(void)
{
	this->Go_to_app(QSPI_FLASH_APPLICATION_ADDR);
}


void CONTROLLER_t::Go_to_app(uint32_t App_addr)
{
	this->Addr_to_Go = App_addr;
	this->Go_to_App = true;
}


#include "xbram.h"

void CONTROLLER_t::Jump_to_Addr(uint32_t App_addr)
{
	WDT_t::get_WDT()->Stop();

	for(uint32_t i = 0; i<XSCUGIC_MAX_NUM_INTR_INPUTS; i++)
	{
		IRQ_CONTROLLER_t::getIRQController()->DisableInterrupt(i);
	}

	Xil_ExceptionDisable();

	*(volatile unsigned int *)(SLCR_UNLOCK_ADDR) = SLCR_UNLOCK_KEY_VALUE;

	*(volatile unsigned int *)(SLCR_DMAC_RST_CTRL_ADDR) |= 1;
	_delay_ms(10);
	*(volatile unsigned int *)(SLCR_DMAC_RST_CTRL_ADDR) &= ~1;

	*(volatile unsigned int *)(SLCR_GEM_RST_CTRL_ADDR) |= 0xF3;
	_delay_ms(10);
	*(volatile unsigned int *)(SLCR_GEM_RST_CTRL_ADDR) &= ~0xF3;

	*(volatile unsigned int *)(SLCR_SDIO_RST_CTRL_ADDR) |= 0x33;
	_delay_ms(10);
	*(volatile unsigned int *)(SLCR_SDIO_RST_CTRL_ADDR) &= ~0x33;

	*(volatile unsigned int *)(SLCR_GPIO_RST_CTRL_ADDR) |= 1;
	_delay_ms(10);
	*(volatile unsigned int *)(SLCR_GPIO_RST_CTRL_ADDR) &= ~1;

	*(volatile unsigned int *)(SLCR_LQSPI_RST_CTRL_ADDR) |= 3;
	_delay_ms(10);
	*(volatile unsigned int *)(SLCR_LQSPI_RST_CTRL_ADDR) &= ~3;

	*(volatile unsigned int *)(SLCR_FPGA_RST_CTRL_ADDR) |= 0x0F;
	_delay_ms(10);
	*(volatile unsigned int *)(SLCR_FPGA_RST_CTRL_ADDR) &= ~0x0F;

	*(unsigned int *)(SLCR_LOCK_ADDR) = SLCR_LOCK_KEY_VALUE;

	for(uint32_t i = 0; i<XSCUGIC_MAX_NUM_INTR_INPUTS; i++)
	{
		XScuGic_WriteReg(XPAR_SCUGIC_0_CPU_BASEADDR, XSCUGIC_EOI_OFFSET, i);
	}

	XDcfg_WriteReg(XPAR_XDCFG_0_BASEADDR,XDCFG_MULTIBOOT_ADDR_OFFSET,App_addr/0x8000);


//	printf("ARM0 RESET!\n\r");
//	static XBram Bram;
//	XBram_Config* ConfigPtr;
//	ConfigPtr = XBram_LookupConfig(BRAM_DEVICE_ID);
//	XBram_CfgInitialize((XBram*)&Bram, ConfigPtr,ConfigPtr->MemBaseAddress);
//	XBram_WriteReg(Bram.Config.CtrlBaseAddress, ARM0_RESET_ADDR, 1);
//
//	this->Go_to_App = false;

	HandoffExit(0);
}

void CONTROLLER_t::LED_music(void)
{
	uint8_t i;

	for(i=0; i<2; i++)
	{
		this->GPIO_ON(i);
		this->_delay_ms(100);
	}
	for(i=0; i<2; i++)
	{
		this->GPIO_OFF(i);
		this->_delay_ms(100);
	}
}

void CONTROLLER_t::Do_main(void)
{
	if(this->Go_to_App)
	{
		this->Jump_to_Addr(this->Addr_to_Go);
	}
	for(int i = 0; i<main_proc_num; i++)
	{
		if(this->main_procedures[i] != (Callback_Interface_t*)NULL) this->main_procedures[i]->void_callback((void*)this, this->main_procedures_data[i]);
		else break;
	}
}

void CONTROLLER_t::Add_main_procedure(Callback_Interface_t* main_proc)
{
	for(int i = 0; i < main_proc_num; i++)
	{
		if(this->main_procedures[i] == (Callback_Interface_t*)NULL)
		{
			this->main_procedures[i] = main_proc;
			this->main_procedures_data[i] = NULL;
			break;
		}
	}
}

void CONTROLLER_t::Add_main_procedure(Callback_Interface_t* main_proc, void* call_data)
{
	for(int i = 0; i < main_proc_num; i++)
	{
		if(this->main_procedures[i] == (Callback_Interface_t*)NULL)
		{
			this->main_procedures[i] = main_proc;
			this->main_procedures_data[i] = call_data;
			break;
		}
	}
}

void CONTROLLER_t::Delete_main_procedure(Callback_Interface_t* main_proc)
{
	uint32_t index = 0;
	for(int i = 0; i < main_proc_num; i++)
	{
		if(this->main_procedures[i] == main_proc) break;
		else index++;
	}
	if(index == (main_proc_num-1))
	{
		if(this->main_procedures[index] == main_proc)
		{
			this->main_procedures[index] = (Callback_Interface_t*)NULL;
			this->main_procedures_data[index] = NULL;
		}
	}
	else
	{
		for(int i = index; i < (main_proc_num-1); i++)
		{
			this->main_procedures[i] = this->main_procedures[i+1];
			this->main_procedures_data[i] = this->main_procedures_data[i+1];
			if(this->main_procedures[i+1] == (Callback_Interface_t*)NULL) break;
		}
	}
}


