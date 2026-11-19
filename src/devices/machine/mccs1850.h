// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Motorola MCCS1850 / MC68HC68T1 Serial Real-Time Clock emulation

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

// ======================> motorola_serial_rtc_device_base

class motorola_serial_rtc_device_base : public device_t,
										public device_rtc_interface,
										public device_nvram_interface
{
public:
	auto int_wr_callback() { return int_cb.bind(); }
	auto pse_wr_callback() { return pse_cb.bind(); }

	void ce_w(int state);
	void sck_w(int state);
	int sdo_r();
	void sdi_w(int state);

protected:
	motorola_serial_rtc_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	// device_nvram_interface implementation
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

	virtual uint8_t read_register(offs_t offset) = 0;
	virtual void write_register(offs_t offset, uint8_t data) = 0;
	virtual offs_t next_address(offs_t offset) const;
	virtual TIMER_CALLBACK_MEMBER(advance_seconds) = 0;

	void set_pse_line(bool state);

	devcb_write_line int_cb, pse_cb;

	uint8_t m_ram[0x80];          // RAM

	// power supply
	int m_pse;                  // power supply enable

	// timers
	emu_timer *m_clock_timer;

private:
	// serial interface
	int m_ce;                   // chip enable
	int m_sck;                  // serial clock
	int m_sdo;                  // serial data out
	int m_sdi;                  // serial data in
	int m_state;                // serial interface state
	uint8_t m_address;            // address counter
	int m_bits;                 // bit counter
	uint8_t m_shift;              // shift register
};


// ======================> mccs1850_device

class mccs1850_device : public motorola_serial_rtc_device_base
{
public:
	// construction/destruction
	mccs1850_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto nuc_wr_callback() { return nuc_cb.bind(); }

	void pwrsw_w(int state);
	void por_w(int state);
	void test_w(int state);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_rtc_interface implementation
	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second) override;

	virtual uint8_t read_register(offs_t offset) override;
	virtual void write_register(offs_t offset, uint8_t data) override;
	virtual TIMER_CALLBACK_MEMBER(advance_seconds) override;

private:
	void check_interrupt();

	devcb_write_line nuc_cb;

	// counter
	uint32_t m_counter;           // seconds counter
};


// ======================> mc68hc68t1_device

// The earlier part shares the serial interface but keeps a BCD calendar.
class mc68hc68t1_device : public motorola_serial_rtc_device_base
{
public:
	mc68hc68t1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t implementation
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override ATTR_COLD;

	// device_nvram_interface implementation
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;

	// device_rtc_interface implementation
	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second) override;

	virtual uint8_t read_register(offs_t offset) override;
	virtual void write_register(offs_t offset, uint8_t data) override;
	virtual offs_t next_address(offs_t offset) const override;
	virtual TIMER_CALLBACK_MEMBER(advance_seconds) override;

private:
	void decode_calendar(offs_t offset);
	void registers_updated();
};


// device type definition
DECLARE_DEVICE_TYPE(MCCS1850, mccs1850_device)
DECLARE_DEVICE_TYPE(MC68HC68T1, mc68hc68t1_device)

#endif // MAME_MACHINE_MCCS1850_H
