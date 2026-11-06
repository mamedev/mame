// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

PC-9801 S[oftware]DIP interface

References:
- https://bitchinbits.foolproofdesigns.com/pc-9821/pc-9821-cheat-sheet/

TODO:
- complete legacy i/f
- ext_sdip: understand how Thrash Protection really kicks in, understand what kind of meme it
  uses for purging SDIP contents this time around (doesn't access stealth SRAM).

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
	save_item(NAME(m_sdip_ram));
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

/*
 *
 * Ext SDIP (Ap, As, Ae)
 *
 * Undocumented, it never r/w the conventional ports.
 * Some of these ports may also be I/O for something else (namely thrash protection).
 * 0xf834f: checks against aa55, throws "ROM SUM ERROR" if bypassed (fixed with autoinc)
 * 0xf8363: sum8 contents of [0x02]++, 10 times. One of these overrides [0x03] with a 0x20
 * If $00f2 bit 1 reads low then it tries writing all the way from 0 to 0xff.
 *
 */

DEFINE_DEVICE_TYPE(PC98_EXT_SDIP, pc98_ext_sdip_device, "pc98_ext_sdip", "NEC PC-98 Ext SDIP device")

pc98_ext_sdip_device::pc98_ext_sdip_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PC98_EXT_SDIP, tag, owner, clock)
	, device_nvram_interface(mconfig, *this)
{
}

void pc98_ext_sdip_device::device_start()
{
	save_item(NAME(m_sdip_ram));
	save_item(NAME(m_address));
}

void pc98_ext_sdip_device::device_reset()
{
//	m_sdip_ram[0x0c] =
	m_sdip_ram[0x1f] = 0;
}

void pc98_ext_sdip_device::nvram_default()
{
	std::fill(std::begin(m_sdip_ram), std::end(m_sdip_ram), 0xff);
}

bool pc98_ext_sdip_device::nvram_read(util::read_stream &file)
{
	auto const [err, actual_size] = util::read(file, m_sdip_ram, m_sdip_size);
	return !err && (actual_size == m_sdip_size);
}

bool pc98_ext_sdip_device::nvram_write(util::write_stream &file)
{
	auto const [err, actual_size] = util::write(file, m_sdip_ram, m_sdip_size);
	return !err;
}

u8 pc98_ext_sdip_device::data_r(offs_t offset)
{
	const u8 res = m_sdip_ram[m_address];
	logerror("%s: data read [%02x] %02x\n", machine().describe_context(), m_address, res);

	if (!machine().side_effects_disabled())
	{
		//printf("%d R\n", m_address);
		// TODO: does it always autoinc? What's the rollover cutoff?
		m_address ++;
		m_address &= 0xff; //%= 44;
	}
	return res;
}

void pc98_ext_sdip_device::data_w(offs_t offset, u8 data)
{
	logerror("%s: data write [%02x] -> %02x\n", machine().describe_context(), m_address, data);

	bool write_enabled = true;
	// TODO: inconclusive as hell
	// What is sure is that *something* enables Thrash protection, discarded over parity rules?
	// sum8 of [3], [4], [5], [6] and [0xc] looks always 0x01 for some reason.
	if (m_sdip_ram[0x1f])
	{
		if (m_address == 3 && !BIT(data, 0))
		{
			write_enabled = false;
			logerror("\tignored\n");
		}

		if ((m_address == 4 || m_address == 5) && !data)
		{
			write_enabled = false;
			logerror("\tignored\n");
		}
	}

	if (write_enabled)
		m_sdip_ram[m_address] = data;

	m_address ++;
	m_address &= 0xff; //%= 44;
}

void pc98_ext_sdip_device::access_w(offs_t offset, u8 data)
{
	logerror("%s: access pattern write [%02x] -> %02x\n", machine().describe_context(), m_address, data);
}

void pc98_ext_sdip_device::address_w(offs_t offset, uint8_t data)
{
	m_address = data;
}
