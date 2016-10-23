// license:BSD-3-Clause
// copyright-holders: Angelo Salese
/***************************************************************************

    uPD4992 RTC

***************************************************************************/

#pragma once

#ifndef __UPD4992DEV_H__
#define __UPD4992DEV_H__

#include "dirtc.h"


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_UPD4992_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, UPD4992, XTAL_32_768kHz)

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> upd4992_device

class upd4992_device : public device_t,
						public device_rtc_interface
{
public:
	// construction/destruction
	upd4992_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// I/O operations
	void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

protected:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second) override;

private:
	enum
	{
		TIMER_CLOCK
		//TIMER_TP,
		//TIMER_DATA_OUT,
		//TIMER_TEST_MODE
	};

	emu_timer *m_timer_clock;
	uint8_t m_rtc_regs[8];
};


// device type definition
extern const device_type UPD4992;



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************



#endif
