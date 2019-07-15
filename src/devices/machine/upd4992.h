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
	upd4992_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 32'768);

	// I/O operations
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ8_MEMBER( read );

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
DECLARE_DEVICE_TYPE(UPD4992, upd4992_device)

#endif // MAME_MACHINE_UPD4992_H
