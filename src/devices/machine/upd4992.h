// license:BSD-3-Clause
// copyright-holders: Angelo Salese
/***************************************************************************

    uPD4992 RTC

***************************************************************************/

#ifndef MAME_MACHINE_UPD4992_H
#define MAME_MACHINE_UPD4992_H

#pragma once

#include "dirtc.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> upd4992_device

class upd4992_device : public device_t, public device_rtc_interface
{
public:
	// construction/destruction
	upd4992_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// I/O operations
	void write(offs_t offset, u8 data);
	u8 read(offs_t offset);

protected:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override ATTR_COLD;
	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second) override;

	TIMER_CALLBACK_MEMBER(clock_tick);

private:
	emu_timer *m_timer_clock;
	u8 m_rtc_regs[8];
};


// device type definition
DECLARE_DEVICE_TYPE(UPD4992, upd4992_device)

#endif // MAME_MACHINE_UPD4992_H
