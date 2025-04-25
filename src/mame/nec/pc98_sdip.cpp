// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

PC-9801 S[oftware]DIP interface

References:
- https://bitchinbits.foolproofdesigns.com/pc-9821/pc-9821-cheat-sheet/

TODO:
- Discards saved settings in PC-9821 and later, access thru MMIO?

===================================================================================================

To enter setup mode:
- Target loopy check for i8251 keyboard status bit 1, pull high;
- help key should pop up in keyboard data as 0x3f
\- pc9821 and later just do individual scanning of the key repeat
   i.e. for pc9821ap2 bp f8a32,1,{esi=0x40;g}
\- pc9801fs is a bit more involved given it scans from a fixed table instead.
   bp f88ea,1,{eax|=2;g}
   bp f88fc,1,{eax=3f;g}

**************************************************************************************************/

#include "emu.h"
#include "pc98_sdip.h"

DEFINE_DEVICE_TYPE(PC98_SDIP, pc98_sdip_device, "pc98_sdip", "NEC PC-98 SDIP device")

pc98_sdip_device::pc98_sdip_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PC98_SDIP, tag, owner, clock)
	, device_nvram_interface(mconfig, *this)
{
}

void pc98_sdip_device::device_start()
{
	save_pointer(NAME(m_sdip_ram), m_sdip_size);
	save_item(NAME(m_bank));
}


void pc98_sdip_device::device_reset()
{
	m_bank = 0;
}


void pc98_sdip_device::nvram_default()
{
	std::fill(std::begin(m_sdip_ram), std::end(m_sdip_ram), 0xff);
}

bool pc98_sdip_device::nvram_read(util::read_stream &file)
{
	auto const [err, actual_size] = util::read(file, m_sdip_ram, m_sdip_size);
	return !err && (actual_size == m_sdip_size);
}

bool pc98_sdip_device::nvram_write(util::write_stream &file)
{
	auto const [err, actual_size] = util::write(file, m_sdip_ram, m_sdip_size);
	return !err;
}


u8 pc98_sdip_device::read(offs_t offset)
{
	u8 sdip_offset = offset + (m_bank * 12);

	return m_sdip_ram[sdip_offset];
}

void pc98_sdip_device::write(offs_t offset, u8 data)
{
	u8 sdip_offset = offset + (m_bank * 12);

	m_sdip_ram[sdip_offset] = data;
}

void pc98_sdip_device::bank_w(int state)
{
	m_bank = !!(state);
}
