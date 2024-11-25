// license:BSD-3-Clause
// copyright-holders:Tim Lindner
/*********************************************************************

    ds1315.h

    Dallas Semiconductor's Phantom Time Chip DS1315.

    by tim lindner, November 2001.

*********************************************************************/

#ifndef MAME_MACHINE_DS1315_H
#define MAME_MACHINE_DS1315_H

#pragma once

class ds1315_device : public device_t
{
public:
	ds1315_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~ds1315_device() {}

	auto read_backing() { return m_backing_read.bind(); }

	// this handler automates the bits 0/2 stuff
	uint8_t read(offs_t offset);

	uint8_t read_0();
	uint8_t read_1();
	uint8_t read_data();
	uint8_t write_data(offs_t offset);

	bool chip_enable();
	void chip_reset();

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	devcb_read8 m_backing_read;

	enum mode_t : u8
	{
		DS_SEEK_MATCHING,
		DS_CALENDAR_IO
	};

	// internal state
	mode_t m_mode;

	void fill_raw_data();
	void input_raw_data();

	int m_count;
	uint8_t m_raw_data[8*8];
};

ALLOW_SAVE_TYPE(ds1315_device::mode_t);

DECLARE_DEVICE_TYPE(DS1315, ds1315_device)

#endif // MAME_MACHINE_DS1315_H
