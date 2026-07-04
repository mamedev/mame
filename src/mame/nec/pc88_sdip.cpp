// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

PC-8801 S[oftware]DIP interface

93C06 EEPROM hooked up in a way to replace reading of physical dip-switches in BIOS software.
Clearly an ancestor of PC-98 SDIP device.
PC-8801MC changed this and made it a MEMSW

PC-8801MA SRAM contents:
[0]
--x- ---- Duplex Half/Full (DSW2 bit 5)
---x ---- Enable X parameter (DSW2 bit 4)
---- x--- Stop bit (DSW2 bit 3)
---- -x-- Serial char length (DSW2 bit 2)
---- --x- Parity Type even/odd (DSW2 bit 1)
---- ---x Parity Generate (DSW2 bit 0)

[1]
---- xxxx Baud rate (set by BIOS at startup to port $6f)

[2]
-x-- ---- Memory weight (DSW1 bit 6)

[3]
x--- ---- Built-in FDD i/f (?, the other setting is <prohibited> anyway)
-x-- ---- Auto-boot floppy (CTRL bit 3)
--x- ---- Enable DEL code (DSW1 bit 5)
---x ---- Enable S parameter (DSW1 bit 4)
---- x--- Text Height (DSW1 bit 3)
---- -x-- Text Width (DSW1 bit 2)
---- --x- Terminal Mode '1' Basic '0' (same as DSW1 bit 1)

TODO:
- define aliases for MA2/MC (CPU clock switch and BASIC mode);
- "SDIP" as name is unconfirmed;

**************************************************************************************************/

#include "emu.h"
#include "pc88_sdip.h"

DEFINE_DEVICE_TYPE(PC88_SDIP, pc88_sdip_device, "pc88_sdip", "NEC PC-88 SDIP device (93C06 serial EEPROM)")

pc88_sdip_device::pc88_sdip_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: eeprom_serial_93c06_16bit_device(mconfig, PC88_SDIP, tag, owner, clock)
{
}

ioport_value pc88_sdip_device::dsw1_r() { return (m_data[3] & 0x3e) >> 1; }
ioport_value pc88_sdip_device::dsw2_r() { return m_data[0] & 0x3f; }
ioport_value pc88_sdip_device::auto_boot_floppy_r() { return BIT(m_data[3], 6); }
//ioport_value pc88_sdip_device::built_in_fdd_r() { return BIT(m_data[3], 7); }
ioport_value pc88_sdip_device::memory_weight_r() { return BIT(m_data[2], 6); }

/*
 * PC-8801MC MEMSW section
 */

DEFINE_DEVICE_TYPE(PC8801MC_MEMSW, pc8801mc_memsw_device, "pc8801mc_memsw", "NEC PC-8801MC Memory Switch device")

pc8801mc_memsw_device::pc8801mc_memsw_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, PC8801MC_MEMSW, tag, owner, clock)
	, device_nvram_interface(mconfig, *this)
{
}

void pc8801mc_memsw_device::device_start()
{
	save_item(NAME(m_bram));
}

void pc8801mc_memsw_device::device_reset()
{
}

void pc8801mc_memsw_device::nvram_default()
{
	std::fill(std::begin(m_bram), std::end(m_bram), 0xff);

	// start with opinionated defaults:
	// 8 MHzH V2 mode, CD-ROM boot enabled and 25 text lines
	const uint8_t default_memsw_data[0x30] =
	{
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
		0x00, 0x04, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00,
		0x01, 0xff, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x0b,  'N',  'E',  'C',  ' ',  'P',  'C',  '-',
		 '8',  '8',  '0',  '1', 0x00, 0x00, 0x00, 0x00
	};

	for (int i = 0; i < 0x30; i++)
	{
		m_bram[i + 0x00] = m_bram[i + 0x30] = default_memsw_data[i];
	}
}

bool pc8801mc_memsw_device::nvram_read(util::read_stream &file)
{
	auto const [err, actual_size] = util::read(file, m_bram, m_bram_size);
	return !err && (actual_size == m_bram_size);
}

bool pc8801mc_memsw_device::nvram_write(util::write_stream &file)
{
	auto const [err, actual_size] = util::write(file, m_bram, m_bram_size);
	return !err;
}

u8 pc8801mc_memsw_device::read(offs_t offset)
{
	return m_bram[offset];
}

void pc8801mc_memsw_device::write(offs_t offset, u8 data)
{
	m_bram[offset] = data;
}

ioport_value pc8801mc_memsw_device::dsw1_r()
{
	u8 res = 0;
	res |= !BIT(m_bram[0x4], 0) << 0;
	res |=  BIT(m_bram[0x5], 0) << 1;
	res |= !BIT(m_bram[0x6], 0) << 2;
	res |=  BIT(m_bram[0xe], 0) << 3;
	// TODO: DEL code assumed to be inverted again
	res |= !BIT(m_bram[0xf], 0) << 4;
	res |=  BIT(m_bram[0x8], 0) << 5;
	return res;
}

// TODO: serial settings, verify if they need the xor negation once we have one
ioport_value pc8801mc_memsw_device::dsw2_r()
{
	u8 res = 0;
	res |= (m_bram[0x10] & 3) << 0;
	res |=  BIT(m_bram[0xb], 0) << 2;
	res |=  BIT(m_bram[0xc], 0) << 3;
	res |=  BIT(m_bram[0xd], 0) << 4;
	res |=  BIT(m_bram[0xa], 0) << 5;
	return res;
}
ioport_value pc8801mc_memsw_device::auto_boot_floppy_r() { return BIT(m_bram[7], 0); }
ioport_value pc8801mc_memsw_device::boot_mode_r() { return m_bram[1] & 3; }
ioport_value pc8801mc_memsw_device::cpu_clock_r() { return BIT(m_bram[0], 0); }

