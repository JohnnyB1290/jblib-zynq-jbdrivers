/*
 * wdt.hpp
 *
 *  Created on: 14.03.2018 ã.
 *      Author: Stalker1290
 */

#ifndef WDT_HPP_
#define WDT_HPP_

#include "chip.h"
#include "IRQ_Controller.hpp"
#include "Common_interfaces.hpp"
#include "xparameters.h"
#include "xscuwdt.h"

class WDT_t:public IRQ_LISTENER_t, public Callback_Interface_t
{
public:
	static WDT_t* get_WDT(void);
	void Initialize(uint16_t reload_time_s);
	void Start(void);
	void Stop(void);
	void Reset(void);
	bool Is_last_reset_wd(void);
    void IRQ(uint32_t IRQ_num);
	void void_callback(void* Intf_ptr, void* parameters);
private:
	WDT_t(void);
	static WDT_t* WDT_t_instance_ptr;

	XScuWdt Watchdog;		/* Cortex SCU Private WatchDog Timer Instance */
	XScuWdt* WdtInstancePtr;
	bool Last_reset_wd;
	bool Initialized;
};


#endif /* WDT_HPP_ */
