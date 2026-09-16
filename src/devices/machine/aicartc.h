// license:BSD-3-Clause
// copyright-holders:Angelo Salese
#ifndef MAME_MACHINE_AICARTC_H
#define MAME_MACHINE_AICARTC_H

#pragma once

#include "dirtc.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> aicartc_device

class aicartc_device : public device_t, public device_rtc_interface
{
public:
	// construction/destruction
	aicartc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// I/O operations
	void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t read(offs_t offset);

	uint16_t m_rtc_reg_lo,m_rtc_reg_hi;
	uint16_t m_rtc_tick;
	uint8_t m_we;

protected:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_rtc_interface overrides
	virtual bool rtc_feature_y2k() const override { return true; }
	virtual bool rtc_feature_leap_year() const override { return true; }
	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second) override;

	TIMER_CALLBACK_MEMBER(clock_tick);

private:
	emu_timer *m_clock_timer;
};


// device type definition
DECLARE_DEVICE_TYPE(AICARTC, aicartc_device)

#endif // MAME_MACHINE_AICARTC_H
