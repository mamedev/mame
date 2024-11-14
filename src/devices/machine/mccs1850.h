// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Motorola MCCS1850 Serial Real-Time Clock emulation

**********************************************************************
                            _____   _____
                  Vbat   1 |*    \_/     | 16  Vdd
                  _POR   2 |             | 15  TEST
                  _INT   3 |             | 14  XTAL1
                   SCK   4 |   MCCS1850  | 13  XTAL2
                   SDI   5 |             | 12  _PWRSW
                   SDO   6 |             | 11  NUC
                    CE   7 |             | 10  _PSE
                   Vss   8 |_____________| 9   PSE

**********************************************************************/

#ifndef MAME_MACHINE_MCCS1850_H
#define MAME_MACHINE_MCCS1850_H

#pragma once

#include "dirtc.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mccs1850_device

class mccs1850_device : public device_t,
						public device_rtc_interface,
						public device_nvram_interface
{
public:
	// construction/destruction
	mccs1850_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto int_wr_callback() { return int_cb.bind(); }
	auto pse_wr_callback() { return pse_cb.bind(); }
	auto nuc_wr_callback() { return nuc_cb.bind(); }

	void ce_w(int state);
	void sck_w(int state);
	int sdo_r();
	void sdi_w(int state);
	void pwrsw_w(int state);
	void por_w(int state);
	void test_w(int state);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

	// device_rtc_interface overrides
	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second) override;

private:
	void check_interrupt();
	void set_pse_line(bool state);
	uint8_t read_register(offs_t offset);
	void write_register(offs_t offset, uint8_t data);
	TIMER_CALLBACK_MEMBER(advance_seconds);

	devcb_write_line int_cb, pse_cb, nuc_cb;

	uint8_t m_ram[0x80];          // RAM

	// power supply
	int m_pse;                  // power supply enable

	// counter
	uint32_t m_counter;           // seconds counter

	// serial interface
	int m_ce;                   // chip enable
	int m_sck;                  // serial clock
	int m_sdo;                  // serial data out
	int m_sdi;                  // serial data in
	int m_state;                // serial interface state
	uint8_t m_address;            // address counter
	int m_bits;                 // bit counter
	uint8_t m_shift;              // shift register

	// timers
	emu_timer *m_clock_timer;
};


// device type definition
DECLARE_DEVICE_TYPE(MCCS1850, mccs1850_device)

#endif // MAME_MACHINE_MCCS1850_H
