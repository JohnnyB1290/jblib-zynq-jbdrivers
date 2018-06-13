/*
 * Web_interface.cpp
 *
 *  Created on: 15.12.2017
 *      Author: Stalker1290
 */

#include "Web_interface.hpp"
#include "stdio.h"
#include "Defines.h"
#include "string.h"
#include "CONTROLLER.hpp"
#include "Common_interfaces.hpp"
#include "lwip/tcp.h"
#include "xbram.h"
#include "COMM_KERNEL_Params.hpp"
#include "CRC.hpp"


const tCGI Web_interface_t::cgi_uri_table[] =
{
	{ "/void_cmd.cgi", (tCGIHandler)Web_interface_t::Void_command_cgi_handler },
};
struct tcp_pcb* Web_interface_t::WS_Updater_console_pcb_ptrs[WS_MAX_NUM_CONNECTIONS];
struct tcp_pcb* Web_interface_t::WS_Control_pcb_ptrs[WS_MAX_NUM_CONNECTIONS];

bool Web_interface_t::FW_upgrade_now = false;
bool Web_interface_t::Erase_QSPI_now = false;

FW_Upgrader_params_t Web_interface_t::FW_Upgrader_params;

uint32_t Web_interface_t::Bit_version = 0;
uint32_t Web_interface_t::Bit_date_time = 0;

Ws_data_t Web_interface_t::WS_data;


static C_void_callback_t* FW_Updater_parse_form_iface_ptr;
static C_void_callback_t* Parse_ws_data_call_iface_ptr;

static void Parse_FW_Updater_form(void);
static void Exit_Parse_FW_Updater_form(void);
static uint8_t Get_form_field(char** Next_Field_ptr_ptr, char** field_name_ptr_ptr, uint8_t** Data_Ptr_ptr, uint32_t* data_len);



void Web_interface_t::Initialize(void)
{
	static XBram Bram;
	XBram_Config* ConfigPtr;
	ConfigPtr = XBram_LookupConfig(BRAM_DEVICE_ID);
	XBram_CfgInitialize((XBram*)&Bram, ConfigPtr,ConfigPtr->MemBaseAddress);
	Web_interface_t::Bit_version = XBram_ReadReg(Bram.Config.CtrlBaseAddress, BITSTREAM_OFFSET_VERSION);
	Web_interface_t::Bit_date_time = XBram_ReadReg(Bram.Config.CtrlBaseAddress, BITSTREAM_OFFSET_DATE_TIME);

	for(uint32_t i = 0; i<WS_MAX_NUM_CONNECTIONS; i++)
	{
		Web_interface_t::WS_Updater_console_pcb_ptrs[i] = NULL;
		Web_interface_t::WS_Control_pcb_ptrs[i] = NULL;
	}

	FW_Updater_parse_form_iface_ptr = new C_void_callback_t(Parse_FW_Updater_form);
	Parse_ws_data_call_iface_ptr = new C_void_callback_t(Web_interface_t::Parse_ws_data);

	Web_interface_t::WS_data.data = NULL;

	Web_interface_t::WS_data.pcb_buf_ptr = NULL;
	Web_interface_t::WS_data.pcb = NULL;
	Web_interface_t::WS_data.data = NULL;
	Web_interface_t::WS_data.data_len = 0;
	Web_interface_t::WS_data.mode = 0;

	Web_interface_t::FW_Upgrader_params.Connection_id = 0;
	Web_interface_t::FW_Upgrader_params.Content_Len = 0;
	Web_interface_t::FW_Upgrader_params.POST_Recieved_Len = 0;
	Web_interface_t::FW_Upgrader_params.Rx_buf_ptr = NULL;
	Web_interface_t::FW_Upgrader_params.Write_Address = 0;
	Web_interface_t::FW_Upgrader_params.FW_length = 0;
	Web_interface_t::FW_Upgrader_params.Written_Len = 0;

	http_set_ssi_handler(Web_interface_t::SSI_Handler, NULL, 0);
	websocket_register_callbacks((tWsOpenHandler) Web_interface_t::websocket_open_cb,(tWsHandler) Web_interface_t::websocket_cb);
	http_set_cgi_handlers(Web_interface_t::cgi_uri_table, sizeof(Web_interface_t::cgi_uri_table) / sizeof(tCGI));
	httpd_init();

	CONTROLLER_t::get_CONTROLLER()->Add_main_procedure(Parse_ws_data_call_iface_ptr);
}


/**
 * This function is called when websocket frame is received.
 *
 * Note: this function is executed on TCP thread and should return as soon
 * as possible.
 */
void Web_interface_t::websocket_cb(struct tcp_pcb *pcb, uint8_t *data, u16_t data_len, uint8_t mode)
{
	if(Web_interface_t::WS_data.data != NULL) return;
	Web_interface_t::WS_data.data = (uint8_t*)malloc(data_len);
	if(Web_interface_t::WS_data.data == NULL) return;
	memcpy(Web_interface_t::WS_data.data, data, data_len);
	Web_interface_t::WS_data.pcb = pcb;
	Web_interface_t::WS_data.data_len = data_len;
	Web_interface_t::WS_data.mode = mode;

}

void Web_interface_t::Parse_ws_data(void)
{
	if(Web_interface_t::WS_data.data != NULL)
	{
		if(Web_interface_t::WS_data.mode == WS_TEXT_MODE)
		{
			if(!strncmp((char*)Web_interface_t::WS_data.data, (char*)"Keep on", Web_interface_t::WS_data.data_len))
			{
				if(Web_interface_t::WS_data.pcb != NULL)
				{
					if(Web_interface_t::WS_data.pcb->state == ESTABLISHED)
						websocket_write(Web_interface_t::WS_data.pcb, (uint8_t*)"Keep on", 7, WS_TEXT_MODE);
				}
			}
			else if(!strncmp((char*)Web_interface_t::WS_data.data, (char*)"Erase COMM Params", Web_interface_t::WS_data.data_len))
			{
				COMM_KERNEL_NVM_params_t::getComm_NVM_params_ptr()->Reset_To_Default();
			}
			else if(!strncmp((char*)Web_interface_t::WS_data.data, (char*)"Get COMM Params", Web_interface_t::WS_data.data_len))
			{
				Web_interface_t::WS_Send_COMM_Kernel_params();
			}
			else if(!strncmp((char*)Web_interface_t::WS_data.data, (char*)"Erase PUPN Params", Web_interface_t::WS_data.data_len))
			{
				PUPN_params_control_t::getPUPN_params_control_ptr()->Reset_To_Default();
				Web_interface_t::WS_Send_PUPN_params();
			}
			else if(!strncmp((char*)Web_interface_t::WS_data.data, (char*)"Get PUPN Params", Web_interface_t::WS_data.data_len))
			{
				Web_interface_t::WS_Send_PUPN_params();
			}
		}
		else if(Web_interface_t::WS_data.mode == WS_BIN_MODE)
		{
			if(Web_interface_t::WS_data.data_len >= 3)
			{
				uint16_t magic = Web_interface_t::WS_data.data[0]|((uint16_t)Web_interface_t::WS_data.data[1]<<8);
				if(magic == CONTROL_WS_BIN_MAGIC)
				{
					if(Web_interface_t::WS_data.data[2] == PUPN_PARAMS_WS_BIN_TYPE)
					{
						if(Web_interface_t::WS_data.data_len == (3+sizeof(PUPN_Params_t)))
						{
							uint16_t calculated_crc = 0;
							uint16_t recieved_crc = 0;
							calculated_crc = CRC_t::Crc16((uint8_t*)&Web_interface_t::WS_data.data[3],sizeof(PUPN_Params_t) - 2);
							recieved_crc = Web_interface_t::WS_data.data[3 + sizeof(PUPN_Params_t) - 2] |
									((uint16_t)Web_interface_t::WS_data.data[3 + sizeof(PUPN_Params_t) - 1]<<8);
							if(calculated_crc == recieved_crc)
							{
								memcpy((uint8_t*)PUPN_params_control_t::getPUPN_params_control_ptr()->get_PUPN_Parameters_Ptr(),
										&Web_interface_t::WS_data.data[3], sizeof(PUPN_Params_t));
								PUPN_params_control_t::getPUPN_params_control_ptr()->Save_params_to_NVM();
							}
							else printf("WS: Error in Pupn Parameters CRC\n\r");
							Web_interface_t::WS_Send_PUPN_params();
						}
					}
					else if(Web_interface_t::WS_data.data[2] == COMM_KERNEL_PARAMS_WS_BIN_TYPE)
					{
						if(Web_interface_t::WS_data.data_len == (3 + sizeof(CommKernelParams_t)))
						{
							uint16_t calculated_crc = 0;
							uint16_t recieved_crc = 0;
							calculated_crc = COMM_KERNEL_NVM_params_t::Calculate_Checksum((uint8_t*)&Web_interface_t::WS_data.data[3],
									sizeof(CommKernelParams_t) - 2);


							recieved_crc = Web_interface_t::WS_data.data[3 + sizeof(CommKernelParams_t) - 2] |
									((uint16_t)Web_interface_t::WS_data.data[3 + sizeof(CommKernelParams_t) - 1]<<8);
							if(calculated_crc == recieved_crc)
							{
								memcpy((uint8_t*)COMM_KERNEL_NVM_params_t::getComm_NVM_params_ptr()->get_Parameters_Ptr(),
										&Web_interface_t::WS_data.data[3], sizeof(CommKernelParams_t));
								COMM_KERNEL_NVM_params_t::getComm_NVM_params_ptr()->Save_params_to_NVM();
							}
							else printf("WS: Error in COMM Kernel Parameters CRC\n\r");
							Web_interface_t::WS_Send_COMM_Kernel_params();
						}
					}
				}
			}

//			printf("Websocket Binary mode data:");
//			for(uint16_t i = 0; i<Web_interface_t::WS_data.data_len; i++) printf("%c", Web_interface_t::WS_data.data[i]);
//			printf("\n\r");
		}

		free(Web_interface_t::WS_data.data);
		Web_interface_t::WS_data.data = NULL;
	}

}

void Web_interface_t::WS_Send_COMM_Kernel_params(void)
{
	uint8_t* temp_out_buf_ptr = NULL;
	for(uint32_t i = 0; i<WS_MAX_NUM_CONNECTIONS; i++)
	{
		if(Web_interface_t::WS_Control_pcb_ptrs[i] == NULL)
		{
			continue;
		}
		else if(Web_interface_t::WS_Control_pcb_ptrs[i]->state != ESTABLISHED)
		{
			Web_interface_t::WS_Control_pcb_ptrs[i] = NULL;
			continue;
		}
		else
		{
			temp_out_buf_ptr = (uint8_t*)malloc(sizeof(CommKernelParams_t) + CONTROL_WS_BIN_MAGIC_SIZE + CONTROL_WS_BIN_TYPE_SIZE);
			if(temp_out_buf_ptr != NULL)
			{
				temp_out_buf_ptr[0] = CONTROL_WS_BIN_MAGIC&0xff;
				temp_out_buf_ptr[1] = (CONTROL_WS_BIN_MAGIC&0xff00)>>8;
				temp_out_buf_ptr[2] = COMM_KERNEL_PARAMS_WS_BIN_TYPE;
				memcpy(&temp_out_buf_ptr[CONTROL_WS_BIN_MAGIC_SIZE + CONTROL_WS_BIN_TYPE_SIZE],
						(uint8_t*)COMM_KERNEL_NVM_params_t::getComm_NVM_params_ptr()->get_Parameters_Ptr(),
						sizeof(CommKernelParams_t));
				websocket_write(Web_interface_t::WS_Control_pcb_ptrs[i],temp_out_buf_ptr,
						sizeof(CommKernelParams_t) + CONTROL_WS_BIN_MAGIC_SIZE + CONTROL_WS_BIN_TYPE_SIZE,
						WS_BIN_MODE);
				free(temp_out_buf_ptr);
			}
		}
	}
}


void Web_interface_t::WS_Send_PUPN_params(void)
{
	uint8_t* temp_out_buf_ptr = NULL;
	for(uint32_t i = 0; i<WS_MAX_NUM_CONNECTIONS; i++)
	{
		if(Web_interface_t::WS_Control_pcb_ptrs[i] == NULL)
		{
			continue;
		}
		else if(Web_interface_t::WS_Control_pcb_ptrs[i]->state != ESTABLISHED)
		{
			Web_interface_t::WS_Control_pcb_ptrs[i] = NULL;
			continue;
		}
		else
		{
			temp_out_buf_ptr = (uint8_t*)malloc(sizeof(PUPN_Params_t) + CONTROL_WS_BIN_MAGIC_SIZE + CONTROL_WS_BIN_TYPE_SIZE);
			if(temp_out_buf_ptr != NULL)
			{
				temp_out_buf_ptr[0] = CONTROL_WS_BIN_MAGIC&0xff;
				temp_out_buf_ptr[1] = (CONTROL_WS_BIN_MAGIC&0xff00)>>8;
				temp_out_buf_ptr[2] = PUPN_PARAMS_WS_BIN_TYPE;
				memcpy(&temp_out_buf_ptr[CONTROL_WS_BIN_MAGIC_SIZE + CONTROL_WS_BIN_TYPE_SIZE],
						(uint8_t*)PUPN_params_control_t::getPUPN_params_control_ptr()->get_PUPN_Parameters_Ptr(),
						sizeof(PUPN_Params_t));
				websocket_write(Web_interface_t::WS_Control_pcb_ptrs[i],temp_out_buf_ptr,
						sizeof(PUPN_Params_t) + CONTROL_WS_BIN_MAGIC_SIZE + CONTROL_WS_BIN_TYPE_SIZE,
						WS_BIN_MODE);
				free(temp_out_buf_ptr);
			}
		}
	}
}


/**
 * This function is called when new websocket is open and
 * creates a new websocket_task if requested URI equals '/stream'.
 */
void Web_interface_t::websocket_open_cb(struct tcp_pcb *pcb, const char *uri)
{
	static struct tcp_pcb** pcb_buf_ptr;

	if (!strcmp(uri, "/FW_console")) pcb_buf_ptr = Web_interface_t::WS_Updater_console_pcb_ptrs;
	else if(!strcmp(uri, "/Control_websocket")) pcb_buf_ptr = Web_interface_t::WS_Control_pcb_ptrs;
	else return;

	for(uint32_t i = 0; i<WS_MAX_NUM_CONNECTIONS; i++)
	{
		if (pcb_buf_ptr[i] == NULL)
		{
			pcb_buf_ptr[i] = pcb;
			break;
		}
		else if (pcb_buf_ptr[i]->state != ESTABLISHED)
		{
			pcb_buf_ptr[i] = pcb;
			break;
		}
	}
}

void Web_interface_t::FW_Updater_console_write(char* mes)
{
	for(uint32_t i = 0; i<WS_MAX_NUM_CONNECTIONS; i++)
	{
		if(Web_interface_t::WS_Updater_console_pcb_ptrs[i] == NULL)
		{
			continue;
		}
		else if(Web_interface_t::WS_Updater_console_pcb_ptrs[i]->state != ESTABLISHED)
		{
			Web_interface_t::WS_Updater_console_pcb_ptrs[i] = NULL;
			continue;
		}
		else
		{
			websocket_write(Web_interface_t::WS_Updater_console_pcb_ptrs[i],
					(uint8_t*)mes, strlen(mes), WS_TEXT_MODE);
		}
	}
}

void Web_interface_t::Control_ws_write(char* mes)
{
	for(uint32_t i = 0; i<WS_MAX_NUM_CONNECTIONS; i++)
	{
		if(Web_interface_t::WS_Control_pcb_ptrs[i] == NULL)
		{
			continue;
		}
		else if(Web_interface_t::WS_Control_pcb_ptrs[i]->state != ESTABLISHED)
		{
			Web_interface_t::WS_Control_pcb_ptrs[i] = NULL;
			continue;
		}
		else
		{
			websocket_write(Web_interface_t::WS_Control_pcb_ptrs[i],
					(uint8_t*)mes, strlen(mes), WS_TEXT_MODE);
		}
	}
}


/* These functions must be implemented by the application */

/** Called when a POST request has been received. The application can decide
 * whether to accept it or not.
 *
 * @param connection Unique connection identifier, valid until httpd_post_end
 *        is called.
 * @param uri The HTTP header URI receiving the POST request.
 * @param http_request The raw HTTP request (the first packet, normally).
 * @param http_request_len Size of 'http_request'.
 * @param content_len Content-Length from HTTP header.
 * @param response_uri Filename of response file, to be filled when denying the
 *        request
 * @param response_uri_len Size of the 'response_uri' buffer.
 * @param post_auto_wnd Set this to 0 to let the callback code handle window
 *        updates by calling 'httpd_post_data_recved' (to throttle rx speed)
 *        default is 1 (httpd handles window updates automatically)
 * @return ERR_OK: Accept the POST request, data may be passed in
 *         another err_t: Deny the POST request, send back 'bad request'.
 */
err_t httpd_post_begin(void *connection, const char *uri, const char *http_request,
                       u16_t http_request_len, int content_len, char *response_uri,
                       u16_t response_uri_len, u8_t *post_auto_wnd)
{
	if(strcmp(uri,"/write_firmware") == 0)
	{
		if((!Web_interface_t::FW_upgrade_now)&&(!Web_interface_t::Erase_QSPI_now))
		{
			Web_interface_t::FW_Upgrader_params.Rx_buf_ptr = (uint8_t*)malloc(content_len+1);
			if(Web_interface_t::FW_Upgrader_params.Rx_buf_ptr == NULL)
			{
				Web_interface_t::FW_Updater_console_write((char*)"Not enough memory, abort.\n\r");
				return ERR_ABRT;
			}
			Web_interface_t::FW_Upgrader_params.Rx_buf_ptr[content_len] = 0;
			if(content_len == 0)
			{
				Web_interface_t::FW_Updater_console_write((char*)"Content length = 0\n\r");
				return ERR_ABRT;
			}
			Web_interface_t::FW_upgrade_now = true;
			Web_interface_t::FW_Upgrader_params.Connection_id = (uint32_t)connection;
			Web_interface_t::FW_Upgrader_params.Content_Len = content_len;
			Web_interface_t::FW_Upgrader_params.POST_Recieved_Len = 0;
			Web_interface_t::FW_Upgrader_params.FW_length = 0;
			Web_interface_t::FW_Upgrader_params.Written_Len = 0;
			Web_interface_t::FW_Upgrader_params.Write_Address = 0;

			Web_interface_t::FW_Updater_console_write((char*)"Receiving form: ");

			strcpy(response_uri, (char*)"/blank.html");
			return ERR_OK;
		}
		else
		{
			Web_interface_t::FW_Updater_console_write((char*)"Please wait..!");
		}
	}

	return ERR_ABRT;
}

/** Called for each pbuf of data that has been received for a POST.
 * ATTENTION: The application is responsible for freeing the pbufs passed in!
 *
 * @param connection Unique connection identifier.
 * @param p Received data.
 * @return ERR_OK: Data accepted.
 *         another err_t: Data denied, http_post_get_response_uri will be called.
 */
err_t httpd_post_receive_data(void *connection, struct pbuf *p)
{
	static int recieved_percent = 0;
	float recieved_part = 0;

	if(p == NULL) return ERR_ABRT;

	if(Web_interface_t::FW_upgrade_now && (Web_interface_t::FW_Upgrader_params.Connection_id == (uint32_t)connection))
	{
		if (p->next != NULL)
		{
			while (p != NULL)
			{
				memcpy(&Web_interface_t::FW_Upgrader_params.Rx_buf_ptr[Web_interface_t::FW_Upgrader_params.POST_Recieved_Len],
						p->payload, p->len);
				Web_interface_t::FW_Upgrader_params.POST_Recieved_Len += p->len;
				p = p->next;
			}
		}
		else
		{
			memcpy(&Web_interface_t::FW_Upgrader_params.Rx_buf_ptr[Web_interface_t::FW_Upgrader_params.POST_Recieved_Len],
					p->payload, p->tot_len);
			Web_interface_t::FW_Upgrader_params.POST_Recieved_Len += p->tot_len;
		}


		recieved_part = (float)Web_interface_t::FW_Upgrader_params.POST_Recieved_Len/(float)Web_interface_t::FW_Upgrader_params.Content_Len;
		recieved_part = recieved_part*100;
		if(recieved_percent != (int)recieved_part)
		{
			recieved_percent = recieved_part;
			snprintf(Web_interface_t::FW_Upgrader_params.Console_mes,FW_Console_mes_max_len,"%i%% ",recieved_percent);
			Web_interface_t::FW_Updater_console_write(Web_interface_t::FW_Upgrader_params.Console_mes);
		}

		pbuf_free(p);
		return ERR_OK;
	}
	else
	{
		pbuf_free(p);
		return ERR_ABRT;
	}

}

/** Called when all data is received or when the connection is closed.
 * The application must return the filename/URI of a file to send in response
 * to this POST request. If the response_uri buffer is untouched, a 404
 * response is returned.
 *
 * @param connection Unique connection identifier.
 * @param response_uri Filename of response file, to be filled when denying the request
 * @param response_uri_len Size of the 'response_uri' buffer.
 */
void httpd_post_finished(void *connection, char *response_uri, u16_t response_uri_len)
{
	if(Web_interface_t::FW_Upgrader_params.POST_Recieved_Len == Web_interface_t::FW_Upgrader_params.Content_Len)
	{
		Web_interface_t::FW_Updater_console_write((char*)"\n\rForm Recieved successfully!\n\r");

		CONTROLLER_t::get_CONTROLLER()->Add_main_procedure(FW_Updater_parse_form_iface_ptr);
		strcpy(response_uri, (char*)"/blank.html");
	}
	else
	{
		Web_interface_t::FW_Updater_console_write((char*)"\n\rForm Recieved with Error!\n\r");
		free(Web_interface_t::FW_Upgrader_params.Rx_buf_ptr);
		Web_interface_t::FW_upgrade_now = false;
	}
	strcpy(response_uri, (char*)"/blank.html");

}



static void Parse_FW_Updater_form(void)
{
	static char* Field_ptr = NULL;
	static char* Field_name = NULL;
	static uint8_t* Data_ptr = NULL;
	static uint32_t Data_len = 0;
	static uint32_t Write_Address = 0;
	static int written_percent = 0;
	float written_part = 0;
	uint32_t Written_data_Len = 0;
	uint8_t ret = 0;
	uint32_t Flash_page_size = 0;

	written_percent = 0;
	Field_ptr = NULL;
	Field_name = NULL;
	Data_ptr = NULL;
	Data_len = 0;
	Write_Address = 0;

	Field_ptr = (char*)Web_interface_t::FW_Upgrader_params.Rx_buf_ptr;

	ret = Get_form_field(&Field_ptr, &Field_name, &Data_ptr, &Data_len);
	if(ret != 0)
	{
		snprintf(Web_interface_t::FW_Upgrader_params.Console_mes,FW_Console_mes_max_len,"Error #%i Get Adrr form field!\n\r",ret);
		Web_interface_t::FW_Updater_console_write(Web_interface_t::FW_Upgrader_params.Console_mes);
		Exit_Parse_FW_Updater_form();
		return;
	}

	if(Field_ptr == NULL)
	{
		Web_interface_t::FW_Updater_console_write((char*)"Error No File Field!\n\r");
		Exit_Parse_FW_Updater_form();
		return;
	}

	if(strcmp(Field_name,(char*)"img_addr"))
	{
		Web_interface_t::FW_Updater_console_write((char*)"Error Image Addr Field Name!\n\r");
		Exit_Parse_FW_Updater_form();
		return;
	}

	sscanf((char*)Data_ptr,"%lX",&Write_Address);

	if((Write_Address < QSPI_FLASH_APPLICATION_ADDR) || (Write_Address >= (QSPI_FLASH_FAT_FS_ADDR-4)))
	{
		Web_interface_t::FW_Updater_console_write((char*)"Invalid Image Address!\n\r");
		Exit_Parse_FW_Updater_form();
		return;
	}

	snprintf(Web_interface_t::FW_Upgrader_params.Console_mes,FW_Console_mes_max_len,"Write Address = 0x%lX\n\r",Write_Address);
	Web_interface_t::FW_Updater_console_write(Web_interface_t::FW_Upgrader_params.Console_mes);


	ret = Get_form_field(&Field_ptr, &Field_name, &Data_ptr, &Data_len);
	if(ret != 0)
	{
		snprintf(Web_interface_t::FW_Upgrader_params.Console_mes,FW_Console_mes_max_len,"Error #%i Get File form field!\n\r",ret);
		Web_interface_t::FW_Updater_console_write(Web_interface_t::FW_Upgrader_params.Console_mes);
		Exit_Parse_FW_Updater_form();
		return;
	}

	if(Data_len == 0)
	{
		Web_interface_t::FW_Updater_console_write((char*)"Error File length = 0!\n\r");
		Exit_Parse_FW_Updater_form();
		return;
	}

	if(Write_Address + Data_len >= QSPI_FLASH_FAT_FS_ADDR)
	{
		Web_interface_t::FW_Updater_console_write((char*)"Error Firmware is Too big!\n\r");
		Exit_Parse_FW_Updater_form();
		return;
	}

	CONTROLLER_t::get_WDT()->Stop();

	Web_interface_t::FW_Updater_console_write((char*)"Erase part of QSPI...\n\r");

	QSPI_Flash_t::Get_QSPI_flash()->FlashErase(Write_Address,Data_len);

	Web_interface_t::FW_Updater_console_write((char*)"Erase Done!\n\r");

	Web_interface_t::FW_Updater_console_write((char*)"Write Firmware on QSPI: ");

	Flash_page_size = QSPI_Flash_t::Flash_Config_Table[QSPI_Flash_t::Get_QSPI_flash()->Get_FCTIndex()].PageSize;

	while(Written_data_Len < Data_len)
	{
		if((Data_len - Written_data_Len)>= Flash_page_size)
		{
			QSPI_Flash_t::Get_QSPI_flash()->Write_flash((Write_Address + Written_data_Len),
					&Data_ptr[Written_data_Len], Flash_page_size);
			Written_data_Len = Written_data_Len + Flash_page_size;
		}
		else
		{
			QSPI_Flash_t::Get_QSPI_flash()->Write_flash((Write_Address + Written_data_Len),
								&Data_ptr[Written_data_Len], Data_len - Written_data_Len);
			Written_data_Len = Data_len;
		}

		written_part = (float)Written_data_Len/(float)Data_len;
		written_part = written_part*100;
		if(written_percent != (int)written_part)
		{
			written_percent = written_part;
			snprintf(Web_interface_t::FW_Upgrader_params.Console_mes,FW_Console_mes_max_len,"%i%% ",written_percent);
			Web_interface_t::FW_Updater_console_write(Web_interface_t::FW_Upgrader_params.Console_mes);
		}
	}

	Web_interface_t::FW_Updater_console_write((char*)"\n\rFirmware Updated successfully!\n\r");

	CONTROLLER_t::get_WDT()->Start();

	Exit_Parse_FW_Updater_form();
}

static void Exit_Parse_FW_Updater_form(void)
{
	free(Web_interface_t::FW_Upgrader_params.Rx_buf_ptr);
	Web_interface_t::FW_upgrade_now = false;
	CONTROLLER_t::get_CONTROLLER()->Delete_main_procedure(FW_Updater_parse_form_iface_ptr);
}


static uint8_t Get_form_field(char** Next_Field_ptr_ptr, char** field_name_ptr_ptr, uint8_t** Data_Ptr_ptr, uint32_t* data_len)
{
	static char* Field_ptr = *Next_Field_ptr_ptr;
	static char* boundary = NULL;
	static uint8_t return_oversize_len = 0;
	char* Content_dispos = NULL;

	Field_ptr = *Next_Field_ptr_ptr;
	boundary = NULL;
	return_oversize_len = 0;

	if(Field_ptr == NULL) return 1;

	if((((uint8_t*)Field_ptr)[0] != 0x2D) || (((uint8_t*)Field_ptr)[1] != 0x2D)) return 2;

	boundary = strtok(Field_ptr,"\n");
	if(boundary == NULL) return 3;

	return_oversize_len = 0;
	for(uint32_t i = strlen(boundary); i>0; i--)
	{
		if(((uint8_t*)boundary)[i-1] != 0x0d)
		{
			((uint8_t*)boundary)[i] = 0;
			break;
		}
		return_oversize_len++;
	}

	Field_ptr = strtok(NULL,"\n");
	if(Field_ptr == NULL) return 4;

	Content_dispos = Field_ptr;

	for(uint32_t i = 0; i < 16; i++)
	{
		Field_ptr = strtok(NULL,"\n");
		if(Field_ptr == NULL) return 5;

		if(strlen(Field_ptr) == return_oversize_len) break;
	}

	Field_ptr = Field_ptr + strlen(Field_ptr) + 1;

	*Data_Ptr_ptr = (uint8_t*)Field_ptr;

	for(uint32_t i = 0; i<0xC800000; i++)
	{
		if(((uint8_t*)Field_ptr)[i] == 0x0A)
		{
			if(!memcmp(Field_ptr+i+1, boundary, strlen(boundary)))
			{
				((uint8_t*)Field_ptr)[i-return_oversize_len] = 0;
				*data_len = i-return_oversize_len;
				Field_ptr = Field_ptr + i + 1;
				break;
			}
		}
		if(i == (0xC800000-1)) return 6;
	}


	Content_dispos = strstr(Content_dispos, (char*)"name=\"");
	if(Content_dispos == NULL) return 7;

	Content_dispos = strtok(Content_dispos,"\"");
	if(Content_dispos == NULL) return 8;

	Content_dispos = strtok(NULL,"\"");
	if(Content_dispos == NULL) return 9;

	*field_name_ptr_ptr = Content_dispos;

	if((((uint8_t*)Field_ptr)[strlen(boundary)] == 0x2D) && (((uint8_t*)Field_ptr)[strlen(boundary)+1] == 0x2D))
	{
		*Next_Field_ptr_ptr = NULL;
	}
	else *Next_Field_ptr_ptr = Field_ptr;

	return 0;
}

char* Web_interface_t::Void_command_cgi_handler(int iIndex, int iNumParams, char *pcParam[],char *pcValue[])
{
	if(strcmp(pcParam[0],"Soft_Reset") == 0)
	{
		CONTROLLER_t::get_CONTROLLER()->Go_to_app(QSPI_FLASH_APPLICATION_ADDR);
	}

	return (char*)"/blank.html";
}

u16_t Web_interface_t::SSI_Handler(const char* ssi_tag_name, char *pcInsert, int iInsertLen)
{
	if(!strcmp(ssi_tag_name,(char*)"description"))
	{
		return snprintf(pcInsert, iInsertLen, "%s", (char*)APP_Description);
	}
	if(!strcmp(ssi_tag_name,(char*)"version"))
	{
		return snprintf(pcInsert, iInsertLen, "%i.%i Type: %i", APP_Version_high, APP_Version_low, APP_type);
	}
	if(!strcmp(ssi_tag_name,(char*)"compileDate"))
	{
		return snprintf(pcInsert, iInsertLen, "%s %s", APP_Compile_date, APP_Compile_time);
	}
	if(!strcmp(ssi_tag_name,(char*)"BitVer"))
	{
		return snprintf(pcInsert, iInsertLen, "%li.%li.%li.%li", (Web_interface_t::Bit_version>>24)&0xff,
				(Web_interface_t::Bit_version>>0)&0xff, (Web_interface_t::Bit_version>>16)&0xff,
				(Web_interface_t::Bit_version>>8)&0xff);
	}
	if(!strcmp(ssi_tag_name,(char*)"BitcompileDate"))
	{
		return snprintf(pcInsert, iInsertLen, "%02li.%02li.%04li %02li:%02li:%02li", ((Web_interface_t::Bit_date_time>>27)&0x1f),
				((Web_interface_t::Bit_date_time>>23)&0x0f),(((Web_interface_t::Bit_date_time>>17)&0x3f) + 2000),
				((Web_interface_t::Bit_date_time>>12)&0x1f),((Web_interface_t::Bit_date_time>>6)&0x3f),
				((Web_interface_t::Bit_date_time>>0)&0x3f));
	}
	if(!strcmp(ssi_tag_name,(char*)"AppVer"))
	{
		return snprintf(pcInsert, iInsertLen, "Unknown");
	}

	if(!strcmp(ssi_tag_name,(char*)"AppRadioAddr"))
	{
		return snprintf(pcInsert, iInsertLen, "<input type=\"radio\" name=\"img_addr\" value=\"0x%X\" checked>",QSPI_FLASH_APPLICATION_ADDR);
	}

	return 0;
}


