// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    s35180.h

    Seiko S-35180 and S-3511A real-time clocks

    A family of 3-wire (chip select / serial clock / bidirectional data)
    RTCs sharing a command format and the date and time registers.  The
    S-35180 is used by the Nintendo DS; the S-3511A is the clock found in
    Game Boy Advance cartridges.

***************************************************************************/
#ifndef MAME_MACHINE_S35180_H
#define MAME_MACHINE_S35180_H

#pragma once

#include "dirtc.h"


class s35180_device : public device_t, public device_rtc_interface
{
public:
	s35180_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 32'768);

	void cs_w(int state);
	void sck_w(int state);
	void data_w(int state);
	int data_r() const { return m_outbit; }

protected:
	s35180_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_rtc_interface implementation
	virtual bool rtc_feature_y2k() const override { return false; }
	virtual bool rtc_feature_leap_year() const override { return true; }
	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second) override;

	virtual uint8_t read_param(int index);
	virtual void write_param(int index, uint8_t data);
	virtual bool hour_24() const { return BIT(m_stat1, 1); }
	virtual uint8_t hour_byte() const;

	uint8_t datetime_byte(int index) const;
	uint8_t time_byte(int index) const;

	int command_reg() const { return (m_cmd >> 4) & 7; }

	int m_year, m_month, m_day, m_weekday, m_hour, m_minute, m_second;

	uint8_t m_stat1, m_stat2;
	uint8_t m_adjust, m_free;
	uint8_t m_alarm[2][3];

private:
	TIMER_CALLBACK_MEMBER(clock_tick);

	void begin_command();
	void reset_transaction();

	emu_timer *m_clock_timer;

	// serial state
	uint8_t m_cs, m_sck, m_data;
	bool m_cs_changed;
	uint8_t m_cmd;
	uint8_t m_shift;
	uint8_t m_bitpos;
	uint8_t m_bytepos;
	uint8_t m_outbit;
	bool m_have_cmd;
	bool m_reading;
};


// the simpler S3511 used in some GBA carts
class s3511_device : public s35180_device
{
public:
	s3511_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 32'768);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual uint8_t read_param(int index) override;
	virtual void write_param(int index, uint8_t data) override;
	virtual bool hour_24() const override { return BIT(m_status, 6); }
	virtual uint8_t hour_byte() const override;

private:
	uint8_t m_status;
};

DECLARE_DEVICE_TYPE(S35180, s35180_device)
DECLARE_DEVICE_TYPE(S3511, s3511_device)

#endif // MAME_MACHINE_S35180_H
