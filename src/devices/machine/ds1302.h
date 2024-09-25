// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Dallas DS1302 Trickle-Charge Timekeeping Chip emulation

**********************************************************************
                            _____   _____
                  Vcc2   1 |*    \_/     | 8   Vcc1
                    X1   2 |             | 7   SCLK
                    X2   3 |             | 6   I/O
                   GND   4 |_____________| 5   CE

**********************************************************************/

#ifndef MAME_MACHINE_DS1302_H
#define MAME_MACHINE_DS1302_H

#pragma once

#include "dirtc.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> ds1302_device

class ds1302_device :  public device_t,
						public device_rtc_interface,
						public device_nvram_interface
{
public:
	// construction/destruction
	ds1302_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void ce_w(int state);
	void sclk_w(int state);
	void io_w(int state);
	int io_r();

protected:
	ds1302_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint8_t ram_size);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

	// device_rtc_interface overrides
	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second) override;
	virtual bool rtc_feature_leap_year() const override { return true; }

private:
	TIMER_CALLBACK_MEMBER(clock_tick);

	void load_shift_register();
	void input_bit();
	void output_bit();

	const uint8_t m_ram_size;

	int m_ce;
	int m_clk;
	int m_io;
	int m_state;
	int m_bits;
	uint8_t m_cmd;
	uint8_t m_data;
	int m_addr;

	uint8_t m_reg[9];
	uint8_t m_user[9];
	uint8_t m_ram[31];

	// timers
	emu_timer *m_clock_timer;
};

// ======================> ds1202_device

class ds1202_device : public ds1302_device
{
public:
	// construction/destruction
	ds1202_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// device type declarations
DECLARE_DEVICE_TYPE(DS1202, ds1202_device)
DECLARE_DEVICE_TYPE(DS1302, ds1302_device)

#endif // MAME_MACHINE_DS1302_H
