// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    v3021.h

    EM Microelectronic-Marin SA Ultra Low Power 32kHz CMOS RTC (DIP8)

    Serial Real Time Clock

***************************************************************************/

#ifndef MAME_MACHINE_V3021_H
#define MAME_MACHINE_V3021_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> v3021_device

class v3021_device : public device_t
{
public:
	// construction/destruction
	v3021_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 32'768);

	// I/O operations
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ8_MEMBER( read );
	TIMER_CALLBACK_MEMBER(timer_callback);

protected:
	struct rtc_regs_t
	{
		uint8_t sec, min, hour, day, wday, month, year;
	};

	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override;
	virtual void device_reset() override;

	uint8_t m_cal_mask,m_cal_com,m_cal_cnt,m_cal_val;

	rtc_regs_t m_rtc;

	emu_timer *m_timer;
};


// device type definition
DECLARE_DEVICE_TYPE(V3021, v3021_device)

#endif // MAME_MACHINE_V3021_H
