// license:BSD-3-Clause
// copyright-holders:hap
/*

  Epson SED1500 series LCD Driver

*/

#ifndef MAME_VIDEO_SED1500_H
#define MAME_VIDEO_SED1500_H

#pragma once

/*

pinout reference (brief)

OSC: oscillator (resistors or XTAL)
A0-A6: address
D0-D7: data (I/O)
WR/RD: write/read signal
CS: chip select
SYNC: frame synchronize (I/O)

CL: OSC output
COM: LCD commons
SEG: LCD segments

*/

class sed1500_device : public device_t
{
public:
	sed1500_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration helpers
	auto write_segs() { return m_write_segs.bind(); } // common number in offset, segment data in data

	void write(offs_t offset, u8 data);
	u8 read(offs_t offset);

protected:
	sed1500_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 cmax, u8 smax);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(update_segs);

	emu_timer *m_lcd_timer;

	const u8 m_cmax; // number of COL pins
	const u8 m_smax; // number of SEG pins
	u8 m_mode;
	u8 m_cout;
	u8 m_ram[0x80];

	// callbacks
	devcb_write64 m_write_segs;
};

class sed1501_device : public sed1500_device
{
public:
	sed1501_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class sed1502_device : public sed1500_device
{
public:
	sed1502_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class sed1503_device : public sed1500_device
{
public:
	sed1503_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


DECLARE_DEVICE_TYPE(SED1500, sed1500_device)
DECLARE_DEVICE_TYPE(SED1501, sed1501_device)
DECLARE_DEVICE_TYPE(SED1502, sed1502_device)
DECLARE_DEVICE_TYPE(SED1503, sed1503_device)

#endif // MAME_VIDEO_SED1500_H
