/*
 * IRQ_CONTROLLER.hpp
 *
 *  Created on: 09.01.2018
 *      Author: Stalker1290
 */

#ifndef IRQ_CONTROLLER_HPP_
#define IRQ_CONTROLLER_HPP_

#include "chip.h"
#include "Common_interfaces.hpp"
#include "Defines.h"
#include "xil_exception.h"
#include "xscugic.h"

class IRQ_LISTENER_t
{
public:
    IRQ_LISTENER_t(void){}
    virtual ~IRQ_LISTENER_t(void){}
    virtual void IRQ(uint32_t IRQ_num) = 0;
};

class IRQ_CONTROLLER_t
{
public:
	static IRQ_CONTROLLER_t* getIRQController(void);
	void EnableInterrupt(u32 InterruptID);
	void DisableInterrupt(u32 InterruptID);
	void SetPriority(u32 InterruptID, u8 Priority, u8 Trigger);
	void SetPriority(u32 InterruptID, u8 Priority);
    void Add_Peripheral_IRQ_Listener(IRQ_LISTENER_t* listener, uint32_t IRQ_num);
    void Delete_Peripheral_IRQ_Listener(IRQ_LISTENER_t* listener);
private:
    static IRQ_CONTROLLER_t* IRQ_Controller_ptr;
    IRQ_LISTENER_t* Peripheral_IRQ_LISTENERS[Peripheral_Listeners_num];
    uint32_t Peripheral_IRQ_LISTENERS_INTR_ID[Peripheral_Listeners_num];
    XScuGic IController_instance;
    XScuGic_Config *GicConfig;

    static void HANDLE_Peripheral_IRQ(void* IRQ_num);

    static void HANDLE_Undefined_INT(void *data);
    static void HANDLE_SWI_INT(void *data);
    static void HANDLE_Prefetch_Abort_INT(void *data);
    static void HANDLE_Data_Abort_INT(void *data);
    static void HANDLE_FIQ_INT(void *data);

    IRQ_CONTROLLER_t(void);
};


#endif /* IRQ_CONTROLLER_HPP_ */
