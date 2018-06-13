/*
 * Web_interface.hpp
 *
 *  Created on: 24.01.2018
 *      Author: Stalker1290
 */

#ifndef WEB_INTERFACE_HPP_
#define WEB_INTERFACE_HPP_

#include "httpd.h"
#include "Defines.h"

#define CONTROL_WS_BIN_MAGIC 		0xDEAF
#define CONTROL_WS_BIN_MAGIC_SIZE 	2

#define CONTROL_WS_BIN_TYPE_SIZE 	1
#define COMM_KERNEL_PARAMS_WS_BIN_TYPE 0x01
#define PUPN_PARAMS_WS_BIN_TYPE 0x02


typedef struct FW_Upgrader_params_struct
{
	uint32_t Connection_id;
	uint32_t Content_Len;
	uint32_t POST_Recieved_Len;
	char Console_mes[FW_Console_mes_max_len];
	uint8_t* Rx_buf_ptr;
	uint32_t Write_Address;
	uint32_t FW_length;
	uint32_t Written_Len;
}FW_Upgrader_params_t;

typedef struct Ws_data_struct
{
	struct tcp_pcb** pcb_buf_ptr;
	struct tcp_pcb* pcb;
	uint8_t* data;
	u16_t data_len;
	uint8_t mode;
}Ws_data_t;

class Web_interface_t
{
public:
	static void Initialize(void);
	static void FW_Updater_console_write(char* mes);
	static void Control_ws_write(char* mes);
	static bool FW_upgrade_now;
	static bool Erase_QSPI_now;
	static FW_Upgrader_params_t FW_Upgrader_params;
private:
	static char* Void_command_cgi_handler(int iIndex, int iNumParams, char *pcParam[],char *pcValue[]);
	static u16_t SSI_Handler(const char* ssi_tag_name, char *pcInsert, int iInsertLen);

	static void websocket_cb(struct tcp_pcb *pcb, uint8_t *data, u16_t data_len, uint8_t mode);
	static void websocket_open_cb(struct tcp_pcb *pcb, const char *uri);

	static const tCGI cgi_uri_table[];

	static struct tcp_pcb* WS_Updater_console_pcb_ptrs[WS_MAX_NUM_CONNECTIONS];
	static struct tcp_pcb* WS_Control_pcb_ptrs[WS_MAX_NUM_CONNECTIONS];

	static uint32_t Bit_version;
	static uint32_t Bit_date_time;

	static Ws_data_t WS_data;
	static void Parse_ws_data(void);
	static void WS_Send_PUPN_params(void);
	static void WS_Send_COMM_Kernel_params(void);
};




#endif /* WEB_INTERFACE_HPP_ */
