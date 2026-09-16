// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
#ifndef MAME_MACHINE_DS1307_H
#define MAME_MACHINE_DS1307_H

#include "dirtc.h"
#include "i2chle.h"
#include "machine/nvram.h"

class i2c_ds1307_device :  public device_t, public i2c_hle_interface, public device_rtc_interface
{
public:
	i2c_ds1307_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 32'768);

	TIMER_CALLBACK_MEMBER(timer_callback);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second) override;

	virtual u8 read_data(u16 offset) override;
	virtual void write_data(u16 offset, u8 data) override;

private:
	u8 m_cmos[0x40];
	required_device<nvram_device> m_nvram;
	emu_timer *m_timer;

};

DECLARE_DEVICE_TYPE(I2C_DS1307, i2c_ds1307_device)

#endif // MAME_MACHINE_DS1307_H
