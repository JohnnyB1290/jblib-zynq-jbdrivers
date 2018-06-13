/*
 * CONTROLLER.hpp
 *
 *  Created on: 22.11.2017
 *      Author: Stalker1290
 */

#ifndef CONTROLLER_HPP_
#define CONTROLLER_HPP_

#include <stdio.h>
#include "xgpiops.h"
#include "xstatus.h"
#include "chip.h"
#include "Defines.h"
#include "Common_interfaces.hpp"
#include "IRQ_Controller.hpp"
#include "VOIDTMR.hpp"
#include "PrvtTimerVoid.hpp"
#include "Time_Engine.hpp"
#include "Ethernet_router.hpp"
#include "GEM_zynq.hpp"
#include "QSPI_Flash.hpp"
#include "Axi_timer.hpp"
#include "AXI_UART16550.hpp"
#include "wdt.hpp"
#include "PUPN_Parameters.hpp"
#ifdef USE_CONSOLE
//#include "Void_Console.hpp"
#endif

class CONTROLLER_t
{
public:
	static CONTROLLER_t* get_CONTROLLER(void);
	void Initialize(void);
	void GPIO_ON(uint8_t num);
	void GPIO_OFF(uint8_t num);
	void GPIO_TGL(uint8_t num);
	void _delay_ms(uint32_t ms);
	void _delay_us(uint32_t us);
	void Start_A7_1_APP(uint32_t img_addr);
	void Reset_periph(void);
	void Soft_reset(void);
	void Go_to_app(uint32_t App_addr);
	void Jump_to_Addr(uint32_t App_addr);
	void Do_main(void);
	void Add_main_procedure(Callback_Interface_t* main_proc);
	void Add_main_procedure(Callback_Interface_t* main_proc, void* call_data);
	void Delete_main_procedure(Callback_Interface_t* main_proc);
	static VOID_TIMER_t* get_Private_Timer_void(void){ return Prvt_Timer_void_t::getPrvt_Timer_void(); }
	static VOID_TIMER_t* get_Axi_TMR_0_void(void){ return Axi_Timer_void_t::getAxi_Timer_void(0); }
	static VOID_TIMER_t* get_Axi_TMR_1_void(void){ return Axi_Timer_void_t::getAxi_Timer_void(1); }
	static VOID_TIMER_t* get_Axi_TMR_2_void(void){ return Axi_Timer_void_t::getAxi_Timer_void(2); }
	static VOID_TIMER_t* get_Axi_TMR_3_void(void){ return Axi_Timer_void_t::getAxi_Timer_void(3); }
	static VOID_TIMER_t* get_Axi_TMR_4_void(void){ return Axi_Timer_void_t::getAxi_Timer_void(4); }
	static VOID_TIMER_t* get_Axi_TMR_5_void(void){ return Axi_Timer_void_t::getAxi_Timer_void(5); }
	static Time_Engine_t* get_Time_Engine(void){ return Time_Engine_t::Get_Time_Engine(); }
	static Ethernet_router_t* get_Ethernet_router(void){ return Ethernet_router_t::get_Ethernet_router(); }
	static Ethernet_t* get_Ethernet_phy_0(void) {return Eth_phy_t::get_Ethernet_phy(0); }
	static Ethernet_t* get_Ethernet_phy_1(void) {return Eth_phy_t::get_Ethernet_phy(1); }
	static QSPI_Flash_t* get_QSPI_Flash(void){ return QSPI_Flash_t::Get_QSPI_flash();}
	static AXI_UART16550_int_t* get_AXI_UART_0(uint32_t baudrate){ return AXI_UART16550_int_t::get_AXI_UART16550_int(0, baudrate); }
	static AXI_UART16550_int_t* get_AXI_UART_1(uint32_t baudrate){ return AXI_UART16550_int_t::get_AXI_UART16550_int(1, baudrate); }
	static AXI_UART16550_int_t* get_AXI_UART_2(uint32_t baudrate){ return AXI_UART16550_int_t::get_AXI_UART16550_int(2, baudrate); }
	static AXI_UART16550_int_t* get_AXI_UART_3(uint32_t baudrate){ return AXI_UART16550_int_t::get_AXI_UART16550_int(3, baudrate); }
	static AXI_UART16550_int_t* get_AXI_UART_4(uint32_t baudrate){ return AXI_UART16550_int_t::get_AXI_UART16550_int(4, baudrate); }
	static AXI_UART16550_int_t* get_AXI_UART_5(uint32_t baudrate){ return AXI_UART16550_int_t::get_AXI_UART16550_int(5, baudrate); }
	static AXI_UART16550_int_t* get_AXI_UART_6(uint32_t baudrate){ return AXI_UART16550_int_t::get_AXI_UART16550_int(6, baudrate); }
	static AXI_UART16550_int_t* get_AXI_UART_7(uint32_t baudrate){ return AXI_UART16550_int_t::get_AXI_UART16550_int(7, baudrate); }
	static AXI_UART16550_int_t* get_AXI_UART_8(uint32_t baudrate){ return AXI_UART16550_int_t::get_AXI_UART16550_int(8, baudrate); }
	static AXI_UART16550_int_t* get_AXI_UART_9(uint32_t baudrate){ return AXI_UART16550_int_t::get_AXI_UART16550_int(9, baudrate); }
	static WDT_t* get_WDT(void){ return WDT_t::get_WDT(); }

private:
	static CONTROLLER_t* CONTROLLER_ptr;
	static uint32_t BOARD_GPIOs[];
	uint8_t Initialize_done;
	Callback_Interface_t* main_procedures[main_proc_num];
	void* main_procedures_data[main_proc_num];
	XGpioPs GpioPs;	/* The driver instance for GPIO Device. */

	bool Go_to_App;
	uint32_t Addr_to_Go;

	CONTROLLER_t(void);
	void LED_music(void);
};


#endif /* CONTROLLER_HPP_ */
