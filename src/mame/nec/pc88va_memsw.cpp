// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

PC-88VA MEMSW interface (メモリスイッチ)

Similar to PC-98 version, where it overlays on Kanji/PCG area.
I/O ports $30-$31 on host are aliased, as original PC-8801 have those physically mapped.
64 bytes according to documentation, system seems to initialize 32 bytes only for some reason.

TODO:
- Alias for system operational mode (VA2+ only?)

**************************************************************************************************/

#include "emu.h"
#include "pc88va_memsw.h"

DEFINE_DEVICE_TYPE(PC88VA_MEMSW, pc88va_memsw_device, "pc88va_memsw", "NEC PC-88VA Memory Switch device")

pc88va_memsw_device::pc88va_memsw_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, PC88VA_MEMSW, tag, owner, clock)
	, device_nvram_interface(mconfig, *this)
{
}

void pc88va_memsw_device::device_start()
{
	save_item(NAME(m_bram));
	save_item(NAME(m_write_protected));
}


void pc88va_memsw_device::device_reset()
{
	m_write_protected = true;
}


void pc88va_memsw_device::nvram_default()
{
	std::fill(std::begin(m_bram), std::end(m_bram), 0xff);
}

bool pc88va_memsw_device::nvram_read(util::read_stream &file)
{
	auto const [err, actual_size] = util::read(file, m_bram, m_bram_size);
	return !err && (actual_size == m_bram_size);
}

bool pc88va_memsw_device::nvram_write(util::write_stream &file)
{
	auto const [err, actual_size] = util::write(file, m_bram, m_bram_size);
	return !err;
}


u8 pc88va_memsw_device::read(offs_t offset)
{
	return m_bram[offset];
}

void pc88va_memsw_device::write(offs_t offset, u8 data)
{
	if (!m_write_protected)
		m_bram[offset] = data;
}
