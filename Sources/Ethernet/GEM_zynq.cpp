/*
 * GEM_zynq.c
 *
 *  Created on: 12 июля 2016 г.
 *      Author: Stalker1290
 */

#include "Ethernet/GEM_zynq.hpp"
#include "CONTROLLER.hpp"

Eth_phy_t* Eth_phy_t::Eth_phy_ptr[Num_of_GEM] = {(Eth_phy_t*)NULL,(Eth_phy_t*)NULL};
uint8_t Eth_phy_t::EmacPsMAC[Num_of_GEM][6] = {GEM_0_Default_MAC ,GEM_1_Default_MAC };
uint8_t Eth_phy_t::Autoneg_lock = 0;


Eth_phy_t* Eth_phy_t::get_Ethernet_phy(uint8_t num)
{
	if(num>=Num_of_GEM) return (Eth_phy_t*)NULL;
	if(Eth_phy_t::Eth_phy_ptr[num] == (Eth_phy_t*)NULL) Eth_phy_t::Eth_phy_ptr[num] = new  Eth_phy_t(num);
	return Eth_phy_t::Eth_phy_ptr[num];
}

Eth_phy_t::Eth_phy_t(uint8_t num):Ethernet_t(),Callback_Interface_t(),IRQ_LISTENER_t()
{
	XEmacPs_Config* Config = NULL;
	LONG Status;


	this->num = num;
	this->initialize_success = 0;
	this->Autonegotiation_succ_flag = 0;
	this->Tx_Unlocked = 1;

	this->RxQueue.br = 0;
	this->RxQueue.bw = 0;

	this->TxqQueue.bd_br = 0;
	this->TxqQueue.bd_bw = 0;
	this->TxqQueue.queue_br = 0;
	this->TxqQueue.queue_bw = 0;

	switch(num)
	{
	case 0:
		memcpy(this->Adapter_name, "Eth_phy0",9);
		this->TX_free_line = GEM_TX_free_line_0;
		this->RXBD_CNT = GEM_RXBD_CNT_0;
		Config = XEmacPs_LookupConfig(EMACPS_0_DEVICE_ID);
		this->speed = GEM_0_Default_speed;
		break;
	case 1:
		memcpy(this->Adapter_name, "Eth_phy1",9);
		this->TX_free_line = GEM_TX_free_line_1;
		this->RXBD_CNT = GEM_RXBD_CNT_1;
		Config = XEmacPs_LookupConfig(EMACPS_1_DEVICE_ID);
		this->speed = GEM_1_Default_speed;
		break;
	}

	Status = XEmacPs_CfgInitialize(&this->EmacPsInstance, Config, Config->BaseAddress);

	if (Status != XST_SUCCESS){
		#ifdef USE_CONSOLE
		#ifdef EthM_console
		printf("Error in initialize Eth %i\n\r",num);
		#endif
		#endif
	}
}

void Eth_phy_t::Initialize(void)
{
	LONG Status;
	XEmacPs_Bd BdTemplate;
	u16 i=0;
//	u32 Reg;
//	uint32_t link_speed;

	#ifdef USE_CONSOLE
	#ifdef EthM_console
	if(this->num == 0) printf("Start_initialize Eth_phy_0\n\r");
	if(this->num == 1) printf("Start_initialize Eth_phy_1\n\r");
	#endif
	#endif

	Status = XEmacPs_SetOptions(&this->EmacPsInstance, XEMACPS_PROMISC_OPTION);
	#ifdef USE_CONSOLE
	#ifdef EthM_console
	if (Status != XST_SUCCESS) printf("Error in set promisc mode\n\r");
	#endif
	#endif
	Status = XEmacPs_SetOptions(&this->EmacPsInstance, XEMACPS_RX_CHKSUM_ENABLE_OPTION);
	#ifdef USE_CONSOLE
	#ifdef EthM_console
	if (Status != XST_SUCCESS) printf("Error in set RX_CHKSUM_ENABLE\n\r");
	#endif
	#endif
	Status = XEmacPs_SetOptions(&this->EmacPsInstance, XEMACPS_TX_CHKSUM_ENABLE_OPTION);
	#ifdef USE_CONSOLE
	#ifdef EthM_console
	if (Status != XST_SUCCESS) printf("Error in set TX_CHKSUM_ENABLE\n\r");
	#endif
	#endif
//	Status = XEmacPs_SetOptions(&this->EmacPsInstance, XEMACPS_FCS_INSERT_OPTION);
//	#ifdef USE_CONSOLE
//	#ifdef EthM_console
//	if (Status != XST_SUCCESS) printf("Error in set FCS INSERT OPT\n\r");
//	#endif
//	#endif

	XEmacPs_SetMdioDivisor(&this->EmacPsInstance, MDC_DIV_224);
	sleep(1);

//	wdt_ptr->Stop();
//	this->Autoneg_lock = 1;
//	link_speed = phy_setup (&this->EmacPsInstance,&this->EmacPsInstance, this->num);
//	if(link_speed != XST_FAILURE)
//	{
//		XEmacPs_SetOperatingSpeed(&this->EmacPsInstance, link_speed);
//		this->Autonegotiation_succ_flag = 1;
//	}
//	else
//	{
//		configure_IEEE_phy_speed(&this->EmacPsInstance, this->num, 100);
//		SetUpSLCRDivisors(this->EmacPsInstance.Config.BaseAddress, 100);
//		XEmacPs_SetOperatingSpeed(&this->EmacPsInstance, 100);
//	}
//	Autoneg_lock = 0;
//	wdt_ptr->Start();

////	wdt_ptr->Stop();
	this->Autoneg_lock = 1;

	configure_IEEE_phy_speed(&this->EmacPsInstance, num, 100);
	SetUpSLCRDivisors(this->EmacPsInstance.Config.BaseAddress, 100);
	XEmacPs_SetOperatingSpeed(&this->EmacPsInstance, 100);
	this->Autoneg_lock = 0;
//	wdt_ptr->Start();

	Status = XEmacPs_SetMacAddress(&this->EmacPsInstance, (void*)Eth_phy_t::EmacPsMAC[this->num], 1);
	#ifdef USE_CONSOLE
	#ifdef EthM_console
	if (Status != XST_SUCCESS) printf("Error setting MAC address\n\r");
	#endif
	#endif

	Status = XEmacPs_SetHandler(&this->EmacPsInstance, XEMACPS_HANDLER_DMASEND,(void*) Eth_phy_t::SendHandler, this);
	Status |= XEmacPs_SetHandler(&this->EmacPsInstance, XEMACPS_HANDLER_DMARECV,(void*) Eth_phy_t::RecvHandler, this);
	Status |= XEmacPs_SetHandler(&this->EmacPsInstance, XEMACPS_HANDLER_ERROR,(void*) Eth_phy_t::ErrorHandler, this);
	#ifdef USE_CONSOLE
	#ifdef EthM_console
	if (Status != XST_SUCCESS) printf("Error assigning handlers\n\r");
	#endif
	#endif

	XEmacPs_BdClear(&BdTemplate);

	if(this->num == 0) Status = XEmacPs_BdRingCreate(&(XEmacPs_GetRxRing(&this->EmacPsInstance)),
			(UINTPTR) RX_BD_LIST_START_ADDRESS_0,(UINTPTR)RX_BD_LIST_START_ADDRESS_0, XEMACPS_BD_ALIGNMENT, GEM_RXBD_CNT_0);

	if(this->num == 1) Status = XEmacPs_BdRingCreate(&(XEmacPs_GetRxRing(&this->EmacPsInstance)),
			(UINTPTR) RX_BD_LIST_START_ADDRESS_1,(UINTPTR)RX_BD_LIST_START_ADDRESS_1, XEMACPS_BD_ALIGNMENT, GEM_RXBD_CNT_1);

	#ifdef USE_CONSOLE
	#ifdef EthM_console
	if (Status != XST_SUCCESS) printf("Error setting up RxBD space, BdRingCreate\n\r");
	#endif
	#endif

	Status = XEmacPs_BdRingClone(&(XEmacPs_GetRxRing(&this->EmacPsInstance)),&BdTemplate, XEMACPS_RECV);
	#ifdef USE_CONSOLE
	#ifdef EthM_console
	if(Status != XST_SUCCESS) printf("Error setting up RxBD space, BdRingClone\n\r");
	#endif
	#endif
	/*
	 * The BDs need to be allocated in uncached memory. Hence the 1 MB
	 * address range that starts at address 0xFF00000 is made uncached.
	 */
	if(this->num == 0)
	{
		Xil_SetTlbAttributes(RX_BD_LIST_START_ADDRESS_0, 0xc02);
		Xil_SetTlbAttributes(TX_BD_LIST_START_ADDRESS_0, 0xc02);
	}
	if(this->num == 1)
	{
		Xil_SetTlbAttributes(RX_BD_LIST_START_ADDRESS_1, 0xc02);
		Xil_SetTlbAttributes(TX_BD_LIST_START_ADDRESS_1, 0xc02);
	}

	XEmacPs_BdClear(&BdTemplate);
	XEmacPs_BdSetStatus(&BdTemplate, XEMACPS_TXBUF_USED_MASK);

	if(this->num == 0) Status = XEmacPs_BdRingCreate(&(XEmacPs_GetTxRing(&this->EmacPsInstance)),
			(UINTPTR) TX_BD_LIST_START_ADDRESS_0,(UINTPTR) TX_BD_LIST_START_ADDRESS_0, XEMACPS_BD_ALIGNMENT, GEM_TXBD_CNT);

	if(this->num == 1) Status = XEmacPs_BdRingCreate(&(XEmacPs_GetTxRing(&this->EmacPsInstance)),
			(UINTPTR) TX_BD_LIST_START_ADDRESS_1,(UINTPTR) TX_BD_LIST_START_ADDRESS_1, XEMACPS_BD_ALIGNMENT, GEM_TXBD_CNT);
	#ifdef USE_CONSOLE
	#ifdef EthM_console
	if (Status != XST_SUCCESS) printf("Error setting up TxBD space, BdRingCreate\n\r");
	#endif
	#endif

	Status = XEmacPs_BdRingClone(&(XEmacPs_GetTxRing(&this->EmacPsInstance)),&BdTemplate, XEMACPS_SEND);
	#ifdef USE_CONSOLE
	#ifdef EthM_console
	if (Status != XST_SUCCESS) printf("Error setting up TxBD space, BdRingClone\n\r");
	#endif
	#endif

	if(this->num == 0)
	{
		IRQ_CONTROLLER_t::getIRQController()->Add_Peripheral_IRQ_Listener(this,EMACPS_0_INTR_ID);
		IRQ_CONTROLLER_t::getIRQController()->SetPriority(EMACPS_0_INTR_ID,GEM_0_interrupt_priority);
		IRQ_CONTROLLER_t::getIRQController()->EnableInterrupt(EMACPS_0_INTR_ID);
	}
	if(this->num == 1)
	{
		IRQ_CONTROLLER_t::getIRQController()->Add_Peripheral_IRQ_Listener(this,EMACPS_1_INTR_ID);
		IRQ_CONTROLLER_t::getIRQController()->SetPriority(EMACPS_1_INTR_ID,GEM_1_interrupt_priority);
		IRQ_CONTROLLER_t::getIRQController()->EnableInterrupt(EMACPS_1_INTR_ID);
	}

	for(i = 0; i < this->RXBD_CNT; i++)
	{
		Xil_DCacheFlushRange((UINTPTR)&(this->RxQueue.Frames[i]), sizeof(EthernetFrame));
		Status = XEmacPs_BdRingAlloc(&(XEmacPs_GetRxRing(&this->EmacPsInstance)),1,(XEmacPs_Bd**)&(this->RxQueue.BdRxPtr[i]));
		#ifdef USE_CONSOLE
		#ifdef EthM_console
		if (Status != XST_SUCCESS) printf("Error allocating RxBD, Eth%i\n\r",this->num);
		#endif
		#endif
		XEmacPs_BdSetAddressRx(this->RxQueue.BdRxPtr[i], (UINTPTR)&(this->RxQueue.Frames[i]));
		Status = XEmacPs_BdRingToHw(&(XEmacPs_GetRxRing(&this->EmacPsInstance)),1, this->RxQueue.BdRxPtr[i]);
		#ifdef USE_CONSOLE
		#ifdef EthM_console
		if (Status != XST_SUCCESS) printf("Error committing RxBD to HW, Eth%i\n\r",this->num);
		#endif
		#endif
	}

	XEmacPs_SetQueuePtr(&this->EmacPsInstance, (&this->EmacPsInstance)->RxBdRing.BaseBdAddr, 0, XEMACPS_RECV);
	XEmacPs_SetQueuePtr(&this->EmacPsInstance, (&this->EmacPsInstance)->TxBdRing.BaseBdAddr, 0, XEMACPS_SEND);

	XEmacPs_Start(&this->EmacPsInstance);


	this->initialize_success = 1;


	#ifdef USE_CONSOLE
	#ifdef EthM_console
	if(num == 0) printf("Eth_0 Successfull_ini\n\r");
	if(num == 1) printf("Eth_1 Successfull_ini\n\r");
	#endif
	#endif

}

void Eth_phy_t::Start(void)
{
	XEmacPs_Start(&this->EmacPsInstance);
}

void Eth_phy_t::ResetDevice(void)
{
	this->initialize_success = 0;
	XEmacPs_Reset(&this->EmacPsInstance);
	this->Initialize();
}

void Eth_phy_t::GetParameter(uint8_t ParamName, void* ParamValue)
{
	if(ParamName == MAC_param)
	{
		*((uint8_t**)ParamValue) = Eth_phy_t::EmacPsMAC[this->num];
	}

	if(ParamName == Tx_Unlock_param)
	{
		*((uint8_t*)ParamValue) = this->Tx_Unlocked;
	}

	if(ParamName == LINK_param)
	{
		if(this->Check_link()) 	*((uint8_t*)ParamValue) = 1;
		else *((uint8_t*)ParamValue) = 0;
	}

	if(ParamName == name_param)
	{
		*((char**)ParamValue) = (char*)this->Adapter_name;
	}
}

void Eth_phy_t::SetParameter(uint8_t ParamName, void* ParamValue)
{
	uint8_t* Param = (uint8_t*)ParamValue;
	if(ParamName == MAC_param)
	{
		memcpy((char*)(&(Eth_phy_t::EmacPsMAC[this->num][0])),Param,6);
	}
	if(ParamName == Tx_Unlock_param)
	{
		this->Tx_Unlocked = *Param;
	}
	if(ParamName == name_param)
	{
		memcpy(this->Adapter_name, ParamValue, 9);
	}
	if(ParamName == speed_param)
	{
		this->speed = *((Ethernet_speed_t*)ParamValue);
	}
}

uint8_t Eth_phy_t::Check_if_TX_queue_not_full(void)
{
	uint16_t count;
	count = (this->TxqQueue.queue_bw >= this->TxqQueue.queue_br ?
			((this->TxqQueue.queue_bw - this->TxqQueue.queue_br) % GEM_TX_QUEUE_LENGTH) :
			(GEM_TX_QUEUE_LENGTH - this->TxqQueue.queue_br + this->TxqQueue.queue_bw));

	if(count >= (GEM_TX_QUEUE_LENGTH - GEM_TXBD_CNT) ) return 0;
	else return 1;
}

void Eth_phy_t::Add_to_TX_queue(EthernetFrame* mes,uint16_t m_size)
{
	uint16_t q_index;
	uint16_t count;

	if(this->Tx_Unlocked)
	{
		count = (this->TxqQueue.queue_bw >= this->TxqQueue.queue_br ?
				((this->TxqQueue.queue_bw - this->TxqQueue.queue_br) % GEM_TX_QUEUE_LENGTH) :
				(GEM_TX_QUEUE_LENGTH - this->TxqQueue.queue_br + this->TxqQueue.queue_bw));

		if(count == (GEM_TX_QUEUE_LENGTH - GEM_TXBD_CNT) )
		{
			#ifdef USE_CONSOLE
			#ifdef EthM_console
			printf("TX_queue_overflow\n\r");
			#endif
			#endif
			return;
		}
		q_index = this->TxqQueue.queue_bw;

		memcpy((uint8_t*)&(this->TxqQueue.Frames[q_index]),mes,m_size);
		this->TxqQueue.frame_size[q_index] = m_size;

		this->TxqQueue.queue_bw++;
		if(this->TxqQueue.queue_bw == GEM_TX_QUEUE_LENGTH) this->TxqQueue.queue_bw = 0;
	}
	this->Push_TX_queue();
}

#ifdef USE_LWIP
void Eth_phy_t::Add_to_TX_queue(struct pbuf* p)
{
	uint16_t q_index;
	uint16_t count;
	uint16_t frame_index = 0;
	uint16_t m_size = 0;

	if(p == NULL) return;
	m_size = p->tot_len;
	if(m_size == 0) return;

	if(this->Tx_Unlocked)
	{
		count = (this->TxqQueue.queue_bw >= this->TxqQueue.queue_br ?
				((this->TxqQueue.queue_bw - this->TxqQueue.queue_br) % GEM_TX_QUEUE_LENGTH) :
				(GEM_TX_QUEUE_LENGTH - this->TxqQueue.queue_br + this->TxqQueue.queue_bw));

		if(count == (GEM_TX_QUEUE_LENGTH - GEM_TXBD_CNT) )
		{
			#ifdef USE_CONSOLE
			#ifdef EthM_console
			printf("TX_queue_overflow\n\r");
			#endif
			#endif
			return;
		}
		q_index = this->TxqQueue.queue_bw;

		if(p->next != NULL)
		{
			while (p != NULL)
			{
				memcpy((uint8_t*)&(this->TxqQueue.Frames[q_index][frame_index]),p->payload,p->len);
				frame_index += p->len;
				p = p->next;
			}
		}
		else
		{
			memcpy((uint8_t*)&(this->TxqQueue.Frames[q_index]),p->payload,p->tot_len);
		}

		this->TxqQueue.frame_size[q_index] = m_size;

		this->TxqQueue.queue_bw++;
		if(this->TxqQueue.queue_bw == GEM_TX_QUEUE_LENGTH) this->TxqQueue.queue_bw = 0;
	}
	this->Push_TX_queue();
}
#endif


uint8_t Eth_phy_t::Tx(void)
{
	LONG Status;
	u16 q_index;
	u16 bd_index;

	if(this->Check_Tx_bd_ready() == 0) return 0;

	q_index = this->TxqQueue.queue_br;
	bd_index = this->TxqQueue.bd_bw;

	Xil_DCacheFlushRange((UINTPTR)&(this->TxqQueue.Frames[q_index]), sizeof(EthernetFrame));

	Status = XEmacPs_BdRingAlloc(&(XEmacPs_GetTxRing(&this->EmacPsInstance)),1,
			(XEmacPs_Bd**)&(this->TxqQueue.BdTxPtr[bd_index]));

	#ifdef USE_CONSOLE
	#ifdef EthM_console
	if (Status != XST_SUCCESS) printf("Error allocating TxBD\n\r");
	#endif
	#endif

	XEmacPs_BdSetAddressTx(this->TxqQueue.BdTxPtr[bd_index], (UINTPTR)&(this->TxqQueue.Frames[q_index]));
	XEmacPs_BdSetLength(this->TxqQueue.BdTxPtr[bd_index], this->TxqQueue.frame_size[q_index]);
	XEmacPs_BdClearTxUsed(this->TxqQueue.BdTxPtr[bd_index]);
	XEmacPs_BdSetLast(this->TxqQueue.BdTxPtr[bd_index]);

	Status = XEmacPs_BdRingToHw(&(XEmacPs_GetTxRing(&this->EmacPsInstance)),1, this->TxqQueue.BdTxPtr[bd_index]);

	#ifdef USE_CONSOLE
	#ifdef EthM_console
	if (Status != XST_SUCCESS) printf("Error committing TxBD to HW\n\r");
	#endif
	#endif

	Xil_DCacheFlushRange((UINTPTR)this->TxqQueue.BdTxPtr[bd_index], 64);

	this->TxqQueue.bd_bw++;
	if(this->TxqQueue.bd_bw >= GEM_TXBD_CNT) this->TxqQueue.bd_bw = 0;

	XEmacPs_Transmit(&this->EmacPsInstance);
	return 1;
}


uint16_t Eth_phy_t::Pull_out_RX_Frame(EthernetFrame* Frame_ptr)
{
	uint16_t count;
	int16_t size;
	count = (this->RxQueue.bw >= this->RxQueue.br ? ((this->RxQueue.bw - this->RxQueue.br) % GEM_RX_QUEUE_LENGTH) :
			(GEM_RX_QUEUE_LENGTH - this->RxQueue.br + this->RxQueue.bw));
	if(count == 0) return 0;
	else
	{
		size = this->RxQueue.frame_size[this->RxQueue.br];
		memcpy(Frame_ptr,(uint8_t*)&(this->RxQueue.Frames[this->RxQueue.br]),size);
		this->RxQueue.br++;
		if(this->RxQueue.br == GEM_RX_QUEUE_LENGTH) this->RxQueue.br = 0;
		return size;
	}
}

void Eth_phy_t::IRQ(uint32_t IRQ_num)
{
	XEmacPs_IntrHandler((void*)&this->EmacPsInstance);
}

void Eth_phy_t::ErrorHandler(void *CallbackData, u8 Direction, u32 ErrorWord)
{

	printf("ErrorHandler\n\r");
#ifdef USE_CONSOLE
#ifdef EthM_console
	Eth_phy_t* Eth_phy_ptr = (Eth_phy_t*)CallbackData;

	if(Eth_phy_ptr->num == 0) printf("Ethernet_0: ");
	if(Eth_phy_ptr->num == 1) printf("Ethernet_1: ");

	switch (Direction) {
	case XEMACPS_RECV:
		if (ErrorWord & XEMACPS_RXSR_HRESPNOK_MASK) printf("Receive DMA error\n\r");
		if (ErrorWord & XEMACPS_RXSR_RXOVR_MASK) printf("Receive over run\n\r");
		if (ErrorWord & XEMACPS_RXSR_BUFFNA_MASK)printf("Receive buffer not available\n\r");
		break;
	case XEMACPS_SEND:
		if (ErrorWord & XEMACPS_TXSR_HRESPNOK_MASK) printf("Transmit DMA error\n\r");
		if (ErrorWord & XEMACPS_TXSR_URUN_MASK) printf("Transmit under run\n\r");
		if (ErrorWord & XEMACPS_TXSR_BUFEXH_MASK) printf("Transmit buffer exhausted\n\r");
		if (ErrorWord & XEMACPS_TXSR_RXOVR_MASK) printf("Transmit retry excessed limits\n\r");
		if (ErrorWord & XEMACPS_TXSR_FRAMERX_MASK) printf("Transmit collision\n\r");
		if (ErrorWord & XEMACPS_TXSR_USEDREAD_MASK) printf("Transmit buffer not available\n\r");
		break;
	}
#endif
#endif
}

void Eth_phy_t::SendHandler(void *CallbackData)
{

}

void Eth_phy_t::RecvHandler(void *CallbackData)
{
	LONG Status;
	u16 i =0 ;
	volatile u16 index = 0;
	u16 count = 0;
	Eth_phy_t* Eth_phy_ptr = (Eth_phy_t*)CallbackData;

	count = (Eth_phy_ptr->RxQueue.bw >= Eth_phy_ptr->RxQueue.br ?
			((Eth_phy_ptr->RxQueue.bw - Eth_phy_ptr->RxQueue.br) % GEM_RX_QUEUE_LENGTH) :
			(GEM_RX_QUEUE_LENGTH - Eth_phy_ptr->RxQueue.br + Eth_phy_ptr->RxQueue.bw));

	if(count >= (GEM_RX_QUEUE_LENGTH - Eth_phy_ptr->RXBD_CNT))
	{
		#ifdef USE_CONSOLE
		#ifdef EthM_console
		printf("RX_Queue overflow\n\r");
		#endif
		#endif
		Eth_phy_ptr->RxQueue.br = Eth_phy_ptr->RxQueue.bw;
	}

	for(i = 0; i < Eth_phy_ptr->RXBD_CNT; i++)
	{
		index = Eth_phy_ptr->RxQueue.bw;
		if( XEmacPs_BdRingFromHwRx( &( XEmacPs_GetRxRing(&Eth_phy_ptr->EmacPsInstance) ), 1,
				(XEmacPs_Bd **)&(Eth_phy_ptr->RxQueue.BdRxPtr[index]) ) )
		{

			Eth_phy_ptr->RxQueue.frame_size[index] = XEmacPs_BdGetLength(Eth_phy_ptr->RxQueue.BdRxPtr[index]);

			Xil_DCacheInvalidateRange((UINTPTR)&(Eth_phy_ptr->RxQueue.Frames[index]), sizeof(EthernetFrame));
			Xil_DCacheFlushRange((UINTPTR)&(Eth_phy_ptr->RxQueue.Frames[index]), sizeof(EthernetFrame));

			XEmacPs_BdClearRxNew(Eth_phy_ptr->RxQueue.BdRxPtr[index]);
			Status = XEmacPs_BdRingFree(&(XEmacPs_GetRxRing(&Eth_phy_ptr->EmacPsInstance)),1,
					Eth_phy_ptr->RxQueue.BdRxPtr[index]);

			#ifdef USE_CONSOLE
			#ifdef EthM_console
			if (Status != XST_SUCCESS) printf("Error freeing up RxBDs\n\r");
			#endif
			#endif
			index = (Eth_phy_ptr->RxQueue.bw + Eth_phy_ptr->RXBD_CNT)%GEM_RX_QUEUE_LENGTH;

			Eth_phy_ptr->RxQueue.bw++;
			if(Eth_phy_ptr->RxQueue.bw == GEM_RX_QUEUE_LENGTH) Eth_phy_ptr->RxQueue.bw = 0;

			Xil_DCacheFlushRange((UINTPTR)&(Eth_phy_ptr->RxQueue.Frames[index]), sizeof(EthernetFrame));

			Status = XEmacPs_BdRingAlloc(&(XEmacPs_GetRxRing(&Eth_phy_ptr->EmacPsInstance)),1,
					(XEmacPs_Bd **)&(Eth_phy_ptr->RxQueue.BdRxPtr[index]));

			#ifdef USE_CONSOLE
			#ifdef EthM_console
			if (Status != XST_SUCCESS) printf("Error allocating RxBD, Eth\n\r");
			#endif
			#endif

			XEmacPs_BdSetAddressRx(Eth_phy_ptr->RxQueue.BdRxPtr[index], (UINTPTR)&(Eth_phy_ptr->RxQueue.Frames[index]));

			Status = XEmacPs_BdRingToHw(&(XEmacPs_GetRxRing(&Eth_phy_ptr->EmacPsInstance)),1, Eth_phy_ptr->RxQueue.BdRxPtr[index]);

			#ifdef USE_CONSOLE
			#ifdef EthM_console
			if (Status != XST_SUCCESS) printf("Error committing RxBD to HW, Eth\n\r");
			#endif
			#endif
		}
		else break;
	}

}

void Eth_phy_t::void_callback(void* Intf_ptr, void* parameters)
{

}

uint8_t Eth_phy_t::Check_Tx_bd_ready(void)
{
	uint16_t count = 0;
	uint16_t i = 0;
	uint16_t index = 0;
	LONG Status;

	if(this->initialize_success == 0) return 0;

	count = (this->TxqQueue.bd_bw >= this->TxqQueue.bd_br ?
			((this->TxqQueue.bd_bw - this->TxqQueue.bd_br) % GEM_TXBD_CNT) :
			(GEM_TXBD_CNT - this->TxqQueue.bd_br + this->TxqQueue.bd_bw));

	if(count)
	{
		for(i=0; i<count; i++)
		{
			index = this->TxqQueue.bd_br;
			Status = XEmacPs_BdRingFromHwTx(&(XEmacPs_GetTxRing(&this->EmacPsInstance)),1,
					(XEmacPs_Bd**)&(this->TxqQueue.BdTxPtr[index]));
			if(Status)
			{
				Status = XEmacPs_BdRingFree(&(XEmacPs_GetTxRing(&this->EmacPsInstance)),1, this->TxqQueue.BdTxPtr[index]);
				if (Status != XST_SUCCESS)
				{
					#ifdef USE_CONSOLE
					#ifdef EthM_console
					printf("Error freeing up TxBDs\n\r");
					#endif
					#endif
					break;
				}
				this->TxqQueue.bd_br = this->TxqQueue.bd_br+1;
				if(this->TxqQueue.bd_br == GEM_TXBD_CNT) this->TxqQueue.bd_br = 0;
			}
			else break;
		}
	}

	count = (this->TxqQueue.bd_bw >= this->TxqQueue.bd_br ?
			((this->TxqQueue.bd_bw - this->TxqQueue.bd_br) % GEM_TXBD_CNT) :
			(GEM_TXBD_CNT - this->TxqQueue.bd_br + this->TxqQueue.bd_bw));

	if(count >= this->TX_free_line) return 0;

	return 1;
}


bool Eth_phy_t::Check_link(void)
{
	u16 PhyReg17;
	u32 Status;
	//Status = XEmacPs_PhyRead(&this->EmacPsInstance, this->num, 17, &PhyReg17);
	Status = XEmacPs_PhyRead(&Eth_phy_t::Eth_phy_ptr[0]->EmacPsInstance, this->num, 17, &PhyReg17);
	if(Status == XST_SUCCESS)
	{
		PhyReg17 &= 0x400;
		if(PhyReg17) return true;
		else return false;
	}
	else return false;

}


void Eth_phy_t::Event_negotiation_call(void)
{
	#ifdef Autonegotiation
	uint32_t link_speed;
	#endif

	if(this->initialize_success != 0)
	{
		if(this->Autonegotiation_succ_flag == 0)
		{
			if(this->Check_link())
			{
				if(this->Autoneg_lock == 0)
				{
					#ifdef Autonegotiation
					wdt_ptr->Stop();
					if(j == BSPN_eth_num) ErrorMes("Start Autonegotiation BSPN Eth!");
					if(j == SKZI_eth_num) ErrorMes("Start Autonegotiation SKZI Eth!");
					link_speed = phy_setup (&EmacPsInstance[0],&EmacPsInstance[j], j);
					if(link_speed != XST_FAILURE)
					{
						XEmacPs_SetOperatingSpeed(&EmacPsInstance[j], link_speed);
						Autonegotiation_succ_flag[j] = 1;
					}
					wdt_ptr->Start();
					#else
					this->Autonegotiation_succ_flag = 1;
					#endif
				}
			}
		}
	}
}

void Eth_phy_t::Push_TX_queue(void)
{
	uint16_t count;

	count = (this->TxqQueue.queue_bw >= this->TxqQueue.queue_br ?
			((this->TxqQueue.queue_bw - this->TxqQueue.queue_br) % GEM_TX_QUEUE_LENGTH) :
			(GEM_TX_QUEUE_LENGTH - this->TxqQueue.queue_br + this->TxqQueue.queue_bw));

	if((this->Tx()) && count)
	{
		this->TxqQueue.queue_br++;
		if(this->TxqQueue.queue_br == GEM_TX_QUEUE_LENGTH) this->TxqQueue.queue_br = 0;
	}

}

