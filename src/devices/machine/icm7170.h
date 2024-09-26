// license:BSD-3-Clause
// copyright-holders:R. Belmont
/**********************************************************************

    Intersil/Renesas ICM7170 Real Time Clock

**********************************************************************
 PDIP package:
                            _____   _____
                   _WR   1 |     \_/     | 24  _RD
                   ALE   2 |             | 23  VDD
                   _CS   3 |             | 22  D7
                    A4   4 |   ICM7170   | 21  D6
                    A3   5 |             | 20  D5
                    A2   6 |             | 19  D4
                    A1   7 |             | 18  D3
                    A0   8 |             | 17  D2
               OSC OUT   9 |             | 16  D1
                OSC IN  10 |             | 15  D0
            INT SOURCE  11 |             | 14  VBACKUP
            /INTERRUPT  12 |_____________| 13  VSS (GND)

SOIC package:
                            _____   _____
                    A1   1 |     \_/     | 24  A2
                    A0   2 |             | 23  A3
               OSC OUT   3 |             | 22  A4
                OSC IN   4 |   ICM7170   | 21  _CS
            INT SOURCE   5 |             | 20  ALE
            /INTERRUPT   6 |             | 19  _WR
                   VSS   7 |             | 18  _RD
               VBACKUP   8 |             | 17  VDD
                    D0   9 |             | 16  D7
                    D1  10 |             | 15  D6
                    D2  11 |             | 14  D5
                    D3  12 |_____________| 13  D4

**********************************************************************/

#ifndef MAME_MACHINE_ICM7170_H
#define MAME_MACHINE_ICM7170_H

#pragma once

#include "dirtc.h"

class icm7170_device :  public device_t,
						public device_rtc_interface,
						public device_nvram_interface
{
public:
	// construction/destruction
	icm7170_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto irq() { return m_out_irq_cb.bind(); }

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_rtc_interface overrides
	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second) override;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

	TIMER_CALLBACK_MEMBER(clock_tick);

private:
	void recalc_irqs();

	devcb_write_line m_out_irq_cb;
	bool m_out_irq_state;

	uint8_t m_regs[0x20];
	uint8_t m_irq_mask, m_irq_status;

	emu_timer *m_timer;
};

DECLARE_DEVICE_TYPE(ICM7170, icm7170_device)

#endif // MAME_MACHINE_ICM7170_H
