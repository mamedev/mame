// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    RCA CDP1852 Byte-Wide Input/Output Port emulation

**********************************************************************
                            _____   _____
              CSI/_CSI   1 |*    \_/     | 24  Vdd
                  MODE   2 |             | 23  _SR/SR
                   DI0   3 |             | 22  DI7
                   DO0   4 |             | 21  DO7
                   DI1   5 |             | 20  DI6
                   DO1   6 |   CDP1852   | 19  DO6
                   DI2   7 |             | 18  DI5
                   DO2   8 |             | 17  DO5
                   DI3   9 |             | 16  DI4
                   DO3  10 |             | 15  DO4
                 CLOCK  11 |             | 14  _CLEAR
                   Vss  12 |_____________| 13  CS2

**********************************************************************/

#ifndef MAME_MACHINE_CDP1852_H
#define MAME_MACHINE_CDP1852_H

#pragma once

class cdp1852_device : public device_t
{
public:
	// construction/destruction
	cdp1852_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	auto mode_cb() { return m_read_mode.bind(); }
	auto sr_cb() { return m_write_sr.bind(); }
	auto di_cb() { return m_read_data.bind(); }
	auto do_cb() { return m_write_data.bind(); }

	uint8_t read();
	void write(uint8_t data);

	void clock_w(int state);

	uint8_t do_r() { return m_data; }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void set_sr_line(bool state);

	TIMER_CALLBACK_MEMBER(update_do);
	TIMER_CALLBACK_MEMBER(update_sr);

	devcb_read_line    m_read_mode;
	devcb_write_line   m_write_sr;
	devcb_read8        m_read_data;
	devcb_write8       m_write_data;

	bool m_new_data;            // new data written
	u8 m_data;                  // data latch

	bool m_clock_active;        // input clock
	bool m_sr;                  // service request flag
	bool m_next_sr;             // next value of service request flag

	// timers
	emu_timer *m_update_do_timer;
	emu_timer *m_update_sr_timer;
};

DECLARE_DEVICE_TYPE(CDP1852, cdp1852_device)

#endif // MAME_MACHINE_CDP1852_H
