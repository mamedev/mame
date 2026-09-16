// license:BSD-3-Clause
// copyright-holders:Tim Schuerewegen
/*********************************************************************

    Philips PCF8593 CMOS clock/calendar circuit

    (c) 2001-2007 Tim Schuerewegen

*********************************************************************/

#ifndef MAME_MACHINE_PCF8593_H
#define MAME_MACHINE_PCF8593_H

#pragma once

#include "dirtc.h"

// ======================> pcf8593_device

class pcf8593_device :  public device_t,
						public device_rtc_interface,
						public device_nvram_interface
{
public:
	pcf8593_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void scl_w(int state);
	void sda_w(int state);
	int sda_r();

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_rtc_interface overrides
	virtual bool rtc_feature_y2k() const override { return true; }
	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second) override;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

	TIMER_CALLBACK_MEMBER(clock_tick);

private:
	void clear_buffer_rx();

	// internal state
	uint8_t       m_data[16];
	int         m_pin_scl;
	int         m_pin_sda;
	int         m_inp;
	int         m_active;
	int         m_bits;
	uint8_t       m_data_recv_index;
	uint8_t       m_data_recv[50];
	uint8_t       m_mode;
	uint8_t       m_pos;
	emu_timer * m_timer;
	enum        { RTC_MODE_NONE, RTC_MODE_SEND, RTC_MODE_RECV };
};

// device type definition
DECLARE_DEVICE_TYPE(PCF8593, pcf8593_device)

#endif // MAME_MACHINE_PCF8593_H
