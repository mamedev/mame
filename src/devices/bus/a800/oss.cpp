// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************

 A800 ROM cart emulation

***********************************************************************************************************/


#include "emu.h"
#include "oss.h"


//-------------------------------------------------
//  constructor
//-------------------------------------------------

const device_type A800_ROM_OSS8K = &device_creator<a800_rom_oss8k_device>;
const device_type A800_ROM_OSS34 = &device_creator<a800_rom_oss34_device>;
const device_type A800_ROM_OSS43 = &device_creator<a800_rom_oss43_device>;
const device_type A800_ROM_OSS91 = &device_creator<a800_rom_oss91_device>;


a800_rom_oss8k_device::a800_rom_oss8k_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: a800_rom_device(mconfig, A800_ROM_OSS8K, "Atari 800 ROM Carts OSS 8K", tag, owner, clock, "a800_oss8k", __FILE__),
	m_bank(0)
				{
}


a800_rom_oss34_device::a800_rom_oss34_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: a800_rom_device(mconfig, A800_ROM_OSS34, "Atari 800 ROM Carts OSS-034M", tag, owner, clock, "a800_034m", __FILE__), m_bank(0)
				{
}


a800_rom_oss43_device::a800_rom_oss43_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: a800_rom_device(mconfig, A800_ROM_OSS43, "Atari 800 ROM Carts OSS-043M", tag, owner, clock, "a800_043m", __FILE__), m_bank(0)
				{
}


a800_rom_oss91_device::a800_rom_oss91_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: a800_rom_device(mconfig, A800_ROM_OSS91, "Atari 800 ROM Carts OSS-M091", tag, owner, clock, "a800_m091", __FILE__), m_bank(0)
				{
}



void a800_rom_oss8k_device::device_start()
{
	save_item(NAME(m_bank));
}

void a800_rom_oss8k_device::device_reset()
{
	m_bank = 0;
}


void a800_rom_oss34_device::device_start()
{
	save_item(NAME(m_bank));
}

void a800_rom_oss34_device::device_reset()
{
	m_bank = 1;
}


void a800_rom_oss43_device::device_start()
{
	save_item(NAME(m_bank));
}

void a800_rom_oss43_device::device_reset()
{
	m_bank = 0;
}


void a800_rom_oss91_device::device_start()
{
	save_item(NAME(m_bank));
}

void a800_rom_oss91_device::device_reset()
{
	m_bank = 0;
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 OSS 8K

 This is used by The Writer's Tool only.

 -------------------------------------------------*/

READ8_MEMBER(a800_rom_oss8k_device::read_80xx)
{
	if (offset >= 0x1000)
		return m_rom[offset & 0xfff];
	else
		return m_rom[(offset & 0xfff) + (m_bank * 0x1000)];
}

WRITE8_MEMBER(a800_rom_oss8k_device::write_d5xx)
{
	switch (offset & 0x09)
	{
		case 0:
		case 1:
			m_bank = 1;
			break;
		case 9:
			m_bank = 0;
			break;
		default:
			break;
	}
}


/*-------------------------------------------------

 OSS 034M

 This apparently comes from a dump with the wrong bank order...
 investigate whether we should remove it!

 -------------------------------------------------*/

READ8_MEMBER(a800_rom_oss34_device::read_80xx)
{
	if (offset >= 0x1000)
		return m_rom[(offset & 0xfff) + 0x3000];
	else if (m_bank == 3)
		return 0xff;
	else
		return m_rom[(offset & 0xfff) + (m_bank * 0x1000)];
}

WRITE8_MEMBER(a800_rom_oss34_device::write_d5xx)
{
	switch (offset & 0x0f)
	{
		case 0:
		case 1:
			m_bank = 0;
			break;
		case 2:
		case 6:
			m_bank = 3; // in this case the ROM gets disabled and 0xff is returned in 0xa000-0xafff
			break;
		case 3:
		case 7:
			m_bank = 1;
			break;
		case 4:
		case 5:
			m_bank = 2;
			break;
		default:
			break;
	}
}


/*-------------------------------------------------

 OSS 043M

 Same as above but with correct bank order

 -------------------------------------------------*/

READ8_MEMBER(a800_rom_oss43_device::read_80xx)
{
	if (offset >= 0x1000)
		return m_rom[(offset & 0xfff) + 0x3000];
	else if (m_bank == 3)
		return 0xff;
	else
		return m_rom[(offset & 0xfff) + (m_bank * 0x1000)];
}

WRITE8_MEMBER(a800_rom_oss43_device::write_d5xx)
{
	switch (offset & 0x0f)
	{
		case 0:
		case 1:
			m_bank = 0;
			break;
		case 2:
		case 6:
			m_bank = 3; // in this case the ROM gets disabled and 0xff is returned in 0xa000-0xafff
			break;
		case 3:
		case 7:
			m_bank = 2;
			break;
		case 4:
		case 5:
			m_bank = 1;
			break;
		default:
			break;
	}
}


/*-------------------------------------------------

 OSS M091

 Simplified banking system which only uses two
 address lines (A0 & A3)

 -------------------------------------------------*/

READ8_MEMBER(a800_rom_oss91_device::read_80xx)
{
	if (offset >= 0x1000)
		return m_rom[offset & 0xfff];
	else
		return m_rom[(offset & 0xfff) + (m_bank * 0x1000)];
}

WRITE8_MEMBER(a800_rom_oss91_device::write_d5xx)
{
	switch (offset & 0x09)
	{
		case 0:
			m_bank = 1;
			break;
		case 1:
			m_bank = 3;
			break;
		case 9:
			m_bank = 2;
			break;
		default:
			break;
	}
}
