// license:BSD-3-Clause
// copyright-holders:Luca Elia, Olivier Galibert
/*

IGS012 protection device

implementation based on igs/igs011.cpp, by Luca Elia/Olivier Galibert.

Used by:
- igs/igs011.cpp
- igs/pgmprot_igs025_igs012.cpp used this chip, but not (or incorrectly) implemented
*/

#include "emu.h"
#include "igs012.h"

DEFINE_DEVICE_TYPE(IGS012, igs012_device, "igs012", "IGS012 Protection Device")

void igs012_device::map(address_map &map)
{
	map(0x00, 0x0f).w(FUNC(igs012_device::prot_swap_w));     // swap (a5 / 55)
	map(0x10, 0x1f).r(FUNC(igs012_device::prot_r));          // read (mode 0)
	map(0x20, 0x2f).w(FUNC(igs012_device::prot_dec_inc_w));  // dec  (aa), inc  (fa)
	map(0x30, 0x3f).w(FUNC(igs012_device::prot_inc_w));      // inc  (ff)
	map(0x40, 0x4f).w(FUNC(igs012_device::prot_copy_w));     // copy (22)
	map(0x50, 0x5f).w(FUNC(igs012_device::prot_dec_copy_w)); // dec  (5a), copy (33)
	map(0x60, 0x6f).r(FUNC(igs012_device::prot_r));          // read (mode 1)
	map(0x70, 0x7f).w(FUNC(igs012_device::prot_mode_w));     // mode (cc / dd)
}

igs012_device::igs012_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, IGS012, tag, owner, clock)
	, m_prot(0)
	, m_prot_swap(0)
	, m_prot_mode(0)
{
}

void igs012_device::device_start()
{
	save_item(NAME(m_prot));
	save_item(NAME(m_prot_swap));
	save_item(NAME(m_prot_mode));
}

/***************************************************************************

    IGS012 Protection ("ASIC12 CHECK PORT ERROR")

    The chip holds an internal value, a buffered value and a mode.
    These are manipulated by issuing commands, where each command is assigned
    a specific address range, and is triggered by writing a specific byte value
    to that range. Possible commands:

    - INC:   increment value
    - DEC:   decrement value
    - SWAP:  write bitswap1(value) to buffer
    - COPY:  copy buffer to value
    - MODE:  toggle mode (toggles address ranges to write/read and byte values to write)
    - RESET: value = 0, mode = 0

    The protection value is read from an additional address range:
    - READ: read bitswap2(value). Only 2 bits are checked.

***************************************************************************/


void igs012_device::prot_reset_w(u16 data)
{
	m_prot = 0x00;
	m_prot_swap = 0x00;

	m_prot_mode = 0;
}

#if 0
u16 igs012_device::prot_fake_r(offs_t offset)
{
	switch (offset)
	{
		case 0: return m_prot;
		case 1: return m_prot_swap;
		case 2: return m_prot_mode;
	}
	return 0;
}
#endif

// Macro that checks whether the current mode and data byte written match the arguments
#define MODE_AND_DATA(MODE, DATA)  ((m_prot_mode == (MODE)) && ((data & 0xff) == (DATA)))

void igs012_device::prot_mode_w(offs_t offset, u8 data)
{
	if (MODE_AND_DATA(0, 0xcc) || MODE_AND_DATA(1, 0xcc) || MODE_AND_DATA(0, 0xdd) || MODE_AND_DATA(1, 0xdd))
		m_prot_mode = m_prot_mode ^ 1;
	else
		logerror("%s: warning, unknown prot_mode_w( %04x, %04x ), mode %x\n", machine().describe_context(), offset, data, m_prot_mode);
}

void igs012_device::prot_inc_w(offs_t offset, u8 data)
{
	if (MODE_AND_DATA(0, 0xff))
		m_prot = (m_prot + 1) & 0x1f;
	else
		logerror("%s: warning, unknown prot_inc_w( %04x, %04x ), mode %x\n", machine().describe_context(), offset, data, m_prot_mode);
}

void igs012_device::prot_dec_inc_w(offs_t offset, u8 data)
{
	if (MODE_AND_DATA(0, 0xaa))
		m_prot = (m_prot - 1) & 0x1f;
	else if (MODE_AND_DATA(1, 0xfa))
		m_prot = (m_prot + 1) & 0x1f;
	else
		logerror("%s: warning, unknown prot_dec_inc_w( %04x, %04x ), mode %x\n", machine().describe_context(), offset, data, m_prot_mode);
}

void igs012_device::prot_dec_copy_w(offs_t offset, u8 data)
{
	if (MODE_AND_DATA(0, 0x33))
		m_prot = m_prot_swap;
	else if (MODE_AND_DATA(1, 0x5a))
		m_prot = (m_prot - 1) & 0x1f;
	else
		logerror("%s: warning, unknown prot_dec_copy_w( %04x, %04x ), mode %x\n", machine().describe_context(), offset, data, m_prot_mode);
}

void igs012_device::prot_copy_w(offs_t offset, u8 data)
{
	if (MODE_AND_DATA(1, 0x22))
		m_prot = m_prot_swap;
	else
		logerror("%s: warning, unknown prot_copy_w( %04x, %04x ), mode %x\n", machine().describe_context(), offset, data, m_prot_mode);
}

void igs012_device::prot_swap_w(offs_t offset, u8 data)
{
	if (MODE_AND_DATA(0, 0x55) || MODE_AND_DATA(1, 0xa5))
	{
		// !(3 | 1)..(2 & 1)..(3 ^ 0)..(!2)
		u8 const x = m_prot;

		u8 const b3 = (BIT(x, 3) | BIT(x, 1)) ^ 1;
		u8 const b2 = BIT(x, 2) & BIT(x, 1);
		u8 const b1 = BIT(x, 3) ^ BIT(x, 0);
		u8 const b0 = BIT(x, 2) ^ 1;

		m_prot_swap = (b3 << 3) | (b2 << 2) | (b1 << 1) | (b0 << 0);
	}
	else
		logerror("%s: warning, unknown prot_swap_w( %04x, %04x ), mode %x\n", machine().describe_context(), offset, data, m_prot_mode);
}

u16 igs012_device::prot_r()
{
	// FIXME: mode 0 and mode 1 are mapped to different memory ranges
	u8 const x = m_prot;

	u8 const b1 = (BIT(x, 3) | BIT(x, 1)) ^ 1;
	u8 const b0 = BIT(x, 3) ^ BIT(x, 0);

	return (b1 << 1) | (b0 << 0);
}
