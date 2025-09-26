// license:BSD-3-Clause
// copyright-holders: NaokiS

/*
Heber Pluto 6 PIC18 Customer Security Device

The CSD is used as part of the security, anti-tampering and system controller functions within the Pluto 6.
When the pluto 6 is powered down, a Varta 3.3V Ni-MH battery keeps the PIC18 running in a lower power state.
Is this state, the CSD does the following:
    * Keeps the RTC running.
    * Reads 4 inputs (called power down inputs) and logs the state of them if they change with a timestamp.
    * Retains any current logs.

When the Pluto 6 is powered up, it does the following:
    * Go into a fully powered state
    * Load the encrypted FPGA bitstream using the CPLD to decrypt it.
    * Release reset on the CPU.
    * Read and supply the following over I2C:
        * Stake and Percentage dongles
        * 2x 8-Way DIP switches
        * PIC18 Revision
        * PIC18 Signature

    TODO:
        * _almost_ everything.
        * "upload" FPGA bitstream
        * Hold CPU in reset?
*/

#ifndef MAME_JPM_PL6_PIC_H
#define MAME_JPM_PL6_PIC_H

#pragma once

#include "dirtc.h"
#include "machine/i2chle.h"

INPUT_PORTS_EXTERN(pluto6_csd_pic);

class pl6pic_device :
	public device_t,
	public i2c_hle_interface,
	public device_rtc_interface
{
public:
	pl6pic_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto sda_rx_cb(){ return write_sda.bind(); }

protected:
	virtual void device_start() override;

	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second) override;

	virtual bool rtc_feature_y2k() const override { return true; }
	virtual bool rtc_battery_backed() const override { return true; }
	virtual bool rtc_feature_leap_year() const override { return true; }

	virtual ioport_constructor device_input_ports() const override;

	virtual void write_data(u16 offset, u8 data) override { };
	virtual const char *get_tag() override { return tag(); }

private:
	TIMER_CALLBACK_MEMBER(heartbeat_callback);

	uint8_t m_rtc_minute;
	uint8_t m_rtc_hour;
	uint8_t m_rtc_day;

	emu_timer *m_timer;

	uint8_t m_heartbeat = 1;
	output_finder<> m_hb_led;

	required_ioport_array<2> m_dip;
	required_ioport m_stake;
	required_ioport m_perc;
	required_ioport m_secsw;
};

DECLARE_DEVICE_TYPE(HEBER_PLUTO6_PIC, pl6pic_device)

#endif // MAME_JPM_PL6_PIC_H
