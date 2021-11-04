// license:BSD-3-Clause
// copyright-holders: Angelo Salese
/***************************************************************************

    uPD4991a RTC

***************************************************************************/

#ifndef MAME_MACHINE_UPD4991A_H
#define MAME_MACHINE_UPD4991A_H

#pragma once

#include "dirtc.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> upd4991a_device

class upd4991a_device : public device_t, public device_rtc_interface
{
public:
	// construction/destruction
	upd4991a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// I/O operations
	void address_w(offs_t offset, u8 data);
	u8 data_r(offs_t offset);
	void data_w(offs_t offset, u8 data);

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
	u8 m_rtc_regs[8];
	u8 m_address;
};


// device type definition
DECLARE_DEVICE_TYPE(UPD4991A, upd4991a_device)

#endif // MAME_MACHINE_UPD4991A_H
