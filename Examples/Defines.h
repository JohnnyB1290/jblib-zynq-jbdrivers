/*
 * Defines.h
 *
 *  Created on: 22.11.2017
 *      Author: Stalker1290
 */

#ifndef DEFINES_H_
#define DEFINES_H_

#include "Workspace_Defines.h"

/*******************************IRQ PRIORITY**************************************************************/
#ifdef CORE_A7_0

//* @param	Priority is the new priority for the IRQ source. 0 is highest
//* 			priority, 0xF8 (248) is lowest. There are 32 priority levels
//*			supported with a step of 8. Hence the supported priorities are
//*			0, 8, 16, 32, 40 ..., 248.

//Used IRQ
#define Axi_TMR_0_interrupt_priority 			72 //Lowest_priority_delay
#define Axi_TMR_1_interrupt_priority 			64 //Medium_priority_delay

#define GEM_0_interrupt_priority  				56
#define GEM_1_interrupt_priority 				56

#define AXI_UART16550_0_interrupt_priority		48
#define AXI_UART16550_1_interrupt_priority		48
#define AXI_UART16550_2_interrupt_priority		48
#define AXI_UART16550_3_interrupt_priority		48
#define AXI_UART16550_4_interrupt_priority		48
#define AXI_UART16550_5_interrupt_priority		48
#define AXI_UART16550_6_interrupt_priority		48
#define AXI_UART16550_7_interrupt_priority		48
#define AXI_UART16550_8_interrupt_priority		48
#define AXI_UART16550_9_interrupt_priority		48

#define WDT_interrupt_priority					0

//Unused IRQ

#define Private_Timer_interrupt_priority 		72
#define Axi_TMR_2_interrupt_priority 			72
#define Axi_TMR_3_interrupt_priority 			72
#define Axi_TMR_4_interrupt_priority 			72
#define Axi_TMR_5_interrupt_priority 			72

#endif

#ifdef CORE_A7_1

//Used IRQ
#define Private_Timer_interrupt_priority 		72

//Unused IRQ
#define GEM_0_interrupt_priority  				56
#define GEM_1_interrupt_priority 				56

#endif
/*****************************************************************************************************/


/*************************************Library Common Defines***********************************************/
//Void_Console
#define CONSOLE_CMD_BUF_SIZE 64
#define Console_listeners_num 8
#define Console_Rx_buf_size	64
//


//Private Timer
#define PRIVATE_TIMER_ID			XPAR_XSCUTIMER_0_DEVICE_ID
#define PRIVATE_TIMER_INTR_ID		XPAR_SCUTIMER_INTR
//


//CONTROLLER
#define main_proc_num 8
//

//IRQ_Controller
#define INTC_DEVICE_ID          	XPAR_SCUGIC_0_DEVICE_ID
#define Peripheral_Listeners_num 	32
//

//Time Engine
#define Lowest_priority_delay_step_us					1000
#define Medium_priority_delay_step_us					100


//Event_Timer
#define Event_TMR_size_of_events 16
//

//TCP Server
#define TCP_Server_max_num_broadcast_connections 16
//


//QSPI
#define QSPI_DEVICE_ID		XPAR_XQSPIPS_0_DEVICE_ID
//

//AXI_UART16550
#define AXI_UART16550_num_def 10
#define AXI_UART16550_ptrs_def {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}


#define AXI_UART16550_BASEADDR_0		XPAR_UARTNS550_0_BASEADDR
#define AXI_UART16550_BASEADDR_1		XPAR_UARTNS550_1_BASEADDR
#define AXI_UART16550_BASEADDR_2		XPAR_UARTNS550_3_BASEADDR
#define AXI_UART16550_BASEADDR_3		XPAR_UARTNS550_4_BASEADDR
#define AXI_UART16550_BASEADDR_4		XPAR_UARTNS550_5_BASEADDR
#define AXI_UART16550_BASEADDR_5		XPAR_UARTNS550_6_BASEADDR
#define AXI_UART16550_BASEADDR_6		XPAR_UARTNS550_7_BASEADDR
#define AXI_UART16550_BASEADDR_7		XPAR_UARTNS550_8_BASEADDR
#define AXI_UART16550_BASEADDR_8		XPAR_UARTNS550_9_BASEADDR
#define AXI_UART16550_BASEADDR_9		XPAR_UARTNS550_2_BASEADDR

#define AXI_UART16550_CLOCK_HZ_0		XPAR_UARTNS550_0_CLOCK_FREQ_HZ
#define AXI_UART16550_CLOCK_HZ_1		XPAR_UARTNS550_1_CLOCK_FREQ_HZ
#define AXI_UART16550_CLOCK_HZ_2		XPAR_UARTNS550_3_CLOCK_FREQ_HZ
#define AXI_UART16550_CLOCK_HZ_3		XPAR_UARTNS550_4_CLOCK_FREQ_HZ
#define AXI_UART16550_CLOCK_HZ_4		XPAR_UARTNS550_5_CLOCK_FREQ_HZ
#define AXI_UART16550_CLOCK_HZ_5		XPAR_UARTNS550_6_CLOCK_FREQ_HZ
#define AXI_UART16550_CLOCK_HZ_6		XPAR_UARTNS550_7_CLOCK_FREQ_HZ
#define AXI_UART16550_CLOCK_HZ_7		XPAR_UARTNS550_8_CLOCK_FREQ_HZ
#define AXI_UART16550_CLOCK_HZ_8		XPAR_UARTNS550_9_CLOCK_FREQ_HZ
#define AXI_UART16550_CLOCK_HZ_9		XPAR_UARTNS550_2_CLOCK_FREQ_HZ


#define AXI_UART16550_INTR_ID_0		XPAR_FABRIC_AXI_UART16550_0_IP2INTC_IRPT_INTR
#define AXI_UART16550_INTR_ID_1		XPAR_FABRIC_AXI_UART16550_1_IP2INTC_IRPT_INTR
#define AXI_UART16550_INTR_ID_2		XPAR_FABRIC_AXI_UART16550_2_IP2INTC_IRPT_INTR
#define AXI_UART16550_INTR_ID_3		XPAR_FABRIC_AXI_UART16550_3_IP2INTC_IRPT_INTR
#define AXI_UART16550_INTR_ID_4		XPAR_FABRIC_AXI_UART16550_4_IP2INTC_IRPT_INTR
#define AXI_UART16550_INTR_ID_5		XPAR_FABRIC_AXI_UART16550_6_IP2INTC_IRPT_INTR
#define AXI_UART16550_INTR_ID_6		XPAR_FABRIC_AXI_UART16550_7_IP2INTC_IRPT_INTR
#define AXI_UART16550_INTR_ID_7		XPAR_FABRIC_AXI_UART16550_8_IP2INTC_IRPT_INTR
#define AXI_UART16550_INTR_ID_8		XPAR_FABRIC_AXI_UART16550_9_IP2INTC_IRPT_INTR
#define AXI_UART16550_INTR_ID_9		XPAR_FABRIC_AXI_UART16550_10_IP2INTC_IRPT_INTR


#define Axi_Uart16550_Base_addr_def {AXI_UART16550_BASEADDR_0, AXI_UART16550_BASEADDR_1, AXI_UART16550_BASEADDR_2, \
									AXI_UART16550_BASEADDR_3, AXI_UART16550_BASEADDR_4, AXI_UART16550_BASEADDR_5, \
									AXI_UART16550_BASEADDR_6, AXI_UART16550_BASEADDR_7, AXI_UART16550_BASEADDR_8, \
									AXI_UART16550_BASEADDR_9}

#define Axi_Uart16550_Clock_freq_def {AXI_UART16550_CLOCK_HZ_0, AXI_UART16550_CLOCK_HZ_1, AXI_UART16550_CLOCK_HZ_2, \
									AXI_UART16550_CLOCK_HZ_3, AXI_UART16550_CLOCK_HZ_4, AXI_UART16550_CLOCK_HZ_5, \
									AXI_UART16550_CLOCK_HZ_6, AXI_UART16550_CLOCK_HZ_7, AXI_UART16550_CLOCK_HZ_8, \
									AXI_UART16550_CLOCK_HZ_9}

#define Axi_Uart16550_INT_ID_def {AXI_UART16550_INTR_ID_0, AXI_UART16550_INTR_ID_1, AXI_UART16550_INTR_ID_2, \
								AXI_UART16550_INTR_ID_3, AXI_UART16550_INTR_ID_4, AXI_UART16550_INTR_ID_5, \
								AXI_UART16550_INTR_ID_6, AXI_UART16550_INTR_ID_7, AXI_UART16550_INTR_ID_8, \
								AXI_UART16550_INTR_ID_9}


#define Axi_Uart16550_INT_priority_def {AXI_UART16550_0_interrupt_priority, AXI_UART16550_1_interrupt_priority, \
										AXI_UART16550_2_interrupt_priority, AXI_UART16550_3_interrupt_priority, \
										AXI_UART16550_4_interrupt_priority, AXI_UART16550_5_interrupt_priority, \
										AXI_UART16550_6_interrupt_priority, AXI_UART16550_7_interrupt_priority, \
										AXI_UART16550_8_interrupt_priority, AXI_UART16550_9_interrupt_priority}

//

//WDT
#define WDT_DEVICE_ID		XPAR_SCUWDT_0_DEVICE_ID
#define WDT_RESET_ADDR		QSPI_FLASH_APPLICATION_ADDR
#define WDT_INT_ID			XPS_SCU_WDT_INT_ID
//


//FileSystem
#define FF_Storage_Dev_max_num	2
//

//Logger
#define LOGGER_INSTANCE_NUM 		1
#define LOGGER_MAX_PATH_LENGTH 		64
#define LOGGER_DEFAULT_DIR_NAME 	"log"

//
/*****************************************************************************************************/

/**************************************AXI_Timer************************************************************/
#define Axi_timer_num			6
#define Axi_timer_sub_num 		2

#define Axi_TMR_CLK_Hz			100000000

#define Axi_Tmr_0_INTERRUPT_ID XPAR_FABRIC_AXI_TIMER_0_INTERRUPT_INTR
#define Axi_Tmr_1_INTERRUPT_ID XPAR_FABRIC_AXI_TIMER_1_INTERRUPT_INTR
#define Axi_Tmr_2_INTERRUPT_ID XPAR_FABRIC_AXI_TIMER_2_INTERRUPT_INTR
#define Axi_Tmr_3_INTERRUPT_ID XPAR_FABRIC_AXI_TIMER_3_INTERRUPT_INTR
#define Axi_Tmr_4_INTERRUPT_ID XPAR_FABRIC_AXI_TIMER_4_INTERRUPT_INTR
#define Axi_Tmr_5_INTERRUPT_ID XPAR_FABRIC_AXI_TIMER_5_INTERRUPT_INTR


#define Axi_Tmr_0_DEVICE_ID	XPAR_AXI_TIMER_0_DEVICE_ID
#define Axi_Tmr_1_DEVICE_ID	XPAR_AXI_TIMER_1_DEVICE_ID
#define Axi_Tmr_2_DEVICE_ID	XPAR_AXI_TIMER_2_DEVICE_ID
#define Axi_Tmr_3_DEVICE_ID	XPAR_AXI_TIMER_3_DEVICE_ID
#define Axi_Tmr_4_DEVICE_ID	XPAR_AXI_TIMER_4_DEVICE_ID
#define Axi_Tmr_5_DEVICE_ID	XPAR_AXI_TIMER_5_DEVICE_ID

#define Axi_tmr_dev_IDs_def {Axi_Tmr_0_DEVICE_ID, Axi_Tmr_1_DEVICE_ID, Axi_Tmr_2_DEVICE_ID, \
							Axi_Tmr_3_DEVICE_ID, Axi_Tmr_4_DEVICE_ID, Axi_Tmr_5_DEVICE_ID}

#define Axi_tmr_intr_IDs_def {Axi_Tmr_0_INTERRUPT_ID, Axi_Tmr_1_INTERRUPT_ID, Axi_Tmr_2_INTERRUPT_ID,\
							Axi_Tmr_3_INTERRUPT_ID, Axi_Tmr_4_INTERRUPT_ID, Axi_Tmr_5_INTERRUPT_ID}

#define Axi_tmr_int_prioryties_def {Axi_TMR_0_interrupt_priority, Axi_TMR_1_interrupt_priority, \
									Axi_TMR_2_interrupt_priority, Axi_TMR_3_interrupt_priority, \
									Axi_TMR_4_interrupt_priority, Axi_TMR_5_interrupt_priority}

#define Axi_Timer_void_ptrs_def {NULL, NULL, NULL, NULL, NULL, NULL}
/***********************************************************************************************************/

/*************************************ETHERNET********************************************************/

//GEM

#define GEM_RX_QUEUE_LENGTH 256    //MUST BE GRATER RXBD_CNT!
#define GEM_RX_queue_treshold 250

#define GEM_TX_QUEUE_LENGTH 256

#define EMACPS_0_DEVICE_ID		XPAR_XEMACPS_0_DEVICE_ID
#define EMACPS_0_INTR_ID		XPS_GEM0_INT_ID
#define EMACPS_1_DEVICE_ID		XPAR_XEMACPS_1_DEVICE_ID//XPAR_XEMACPS_1_DEVICE_ID
#define EMACPS_1_INTR_ID		XPS_GEM1_INT_ID//XPS_GEM1_INT_ID

#define GEM_RXBD_CNT_0       128	/* Number of RxBDs to use */
#define GEM_RXBD_CNT_1       128 	/* Number of RxBDs to use */
#define GEM_TXBD_CNT	     128	/* Number of TxBDs to use */

#define GEM_TX_free_line_0 GEM_TXBD_CNT-5
#define GEM_TX_free_line_1 GEM_TXBD_CNT-5

//

//#define Autonegotiation


//Ethernet_phy

#define GEM_0_Default_MAC 					{0x08,0x10,0x34, 0x06, 0x05, 0x04}
#define GEM_0_Default_speed 					speed_100Mbit
#define GEM_0_Check_link_event_period_us		100000


#define GEM_1_Default_MAC 					{0x08,0x10,0x35, 0x06, 0x05, 0x04}
#define GEM_1_Default_speed 					speed_100Mbit
#define GEM_1_Check_link_event_period_us		100000

//ARP Module
#define ETX_ARP_REFRESH_RECORDS_TIME_s		((uint32_t)60UL)
#define ETX_ARP_DELETE_RECORDS_TIME_s		((uint32_t)600UL)
#define ETX_IP_TABLE_FOR_ARP_MAX_NUMBER 	((0x10U))
#define ETX_ARP_TABLE_MAX_NUMBER 			((0x20U))

//
/*****************************************************************************************************/

/*********************************ADRESSES************************************************************/


/******************************************************************************************************/


/**********************************IPC MODULE**********************************************************/

#define IPCEX_QUEUE_SZ        32
#define IPCEX_MAX_IDS         16
#define IPCEX_MAX_GBLVAL      8

#define IPCEX_ID_FREEMEM					1  /*!< Frees memory allocated by other core */
#define IPCEX_ID_GBLUPDATE					2  /*!< Update global variable or other core */

/*******************************************************************************************************/

/********************************************USER***********************************************************/

//#define Default_IP_0 					{192,168,139,125}
//#define Default_GW_0 					{192,168,139,1}
//#define Default_NETMASK_0 				{255,255,255,0}
//
//#define Default_IP_1 					{192,168,139,124}
//#define Default_GW_1 					{192,168,139,1}
//#define Default_NETMASK_1 				{255,255,255,0}


#define Default_IP_0 					{192,168,0,11}
#define Default_GW_0 					{192,168,0,1}
#define Default_NETMASK_0 				{255,255,255,0}

#define Default_IP_1 					{192,168,1,1}
#define Default_GW_1 					{192,168,1,1}
#define Default_NETMASK_1 				{255,255,255,0}

#define ETH_ROUTER_NUM_OF_NETWORK_INTF 	2
#define ETH_ROUTER_NUM_OF_LISTENERS		8

#define WS_MAX_NUM_CONNECTIONS	16 //Websocets pcb ptrs

#define FW_Console_mes_max_len	256

#define App_type_GAZPROM_PLUS	 		12


#define BRAM_DEVICE_ID					XPAR_BRAM_0_DEVICE_ID
#define BITSTREAM_OFFSET_DATE_TIME		(255<<2)
#define BITSTREAM_OFFSET_VERSION		(254<<2)

#define ARM0_RESET_ADDR					(100<<2)

#define APP_Description			"Gazprom_plus Application"
#define APP_Version_high 		1
#define APP_Version_low	 		1
#define APP_type				App_type_GAZPROM_PLUS
#define APP_Compile_date		__DATE__
#define APP_Compile_time		__TIME__



#define PROTO_ENGINE_BUF_SIZE 					384
#define PAYLOAD_COUNT_ON_RF_CHANNEL 			2
#define MAX_THREADS_COUNT 						32

#define CROSS_BUF_SIZE 							384
#define GAMMA_BUF_SIZE 							512
#define TO_PAYLOAD_STORAGE_SLOTS_COUNT 			32
#define TO_PAYLOAD_STORAGE_SLOT_SIZE 			132 //1 for busy (count) + 1 for LC + 2 for packetNumber + 128 for data

#define PROCESS_TREAD_CH_TX_BUF_SIZE			2048
#define PROCESS_TREAD_CH_RX_BUF_SIZE			2048
#define PROCESS_TREAD_CH_RX_LIMIT_EVENT 		64

#define COMM_KERNEL_SERVICE_LC					210


/***********************************************************************************************************/


#endif /* DEFINES_H_ */
