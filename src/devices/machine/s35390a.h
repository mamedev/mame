// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn


#ifndef MAME_MACHINE_S35390A_H
#define MAME_MACHINE_S35390A_H

#pragma once

#include "dirtc.h"


class s35390a_device :
	public device_t,
	public device_nvram_interface,
	public device_rtc_interface
{
public:
	s35390a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void scl_w(int state);
	void sda_w(int state);
	int sda_r() const { return m_sda_out; }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override ATTR_COLD;

	// device_nvram_interface implementation
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

	// device_rtc_interface implementation
	virtual bool rtc_feature_y2k() const override;
	virtual bool rtc_feature_leap_year() const override;
	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second) override;

private:
	enum : u8
	{
		STATE_IDLE,
		STATE_COMMAND,
		STATE_COMMAND_ACK,
		STATE_WRITE_DATA,
		STATE_WRITE_ACK,
		STATE_READ_DATA,
		STATE_READ_ACK
	};

	enum : u8
	{
		REG_YEAR,
		REG_MONTH,
		REG_DAY,
		REG_DAY_OF_WEEK,
		REG_HOUR,
		REG_MINUTE,
		REG_SECOND,
		REG_STATUS_1,
		REG_STATUS_2,
		REG_INT_1,
		REG_INT_1_HOUR,
		REG_INT_1_MINUTE,
		REG_INT_2,
		REG_INT_2_HOUR,
		REG_INT_2_MINUTE,
		REG_CORRECTION,
		REG_FREE,
		REG_COUNT
	};

	TIMER_CALLBACK_MEMBER(clock_tick);

	void bus_start();
	void bus_stop();
	void rising_edge();
	void falling_edge();
	void prepare_read();
	void finish_write();
	void finish_read_byte();
	void reset_registers(u8 status);
	void sync_clock();
	void write_time(bool full);

	u8 command_length() const;
	bool alarm_mode() const;
	u8 encode_hour() const;
	int decode_hour(u8 data) const;

	emu_timer *m_clock_timer;
	u8 m_data[REG_COUNT];
	u8 m_transfer[7];
	u8 m_state;
	u8 m_command;
	u8 m_shift;
	u8 m_bit_count;
	u8 m_byte_count;
	u8 m_expected;
	u8 m_scl;
	u8 m_sda_in;
	u8 m_sda_out;
	u8 m_master_ack;
};


DECLARE_DEVICE_TYPE(S35390A, s35390a_device)

#endif // MAME_MACHINE_S35390A_H
