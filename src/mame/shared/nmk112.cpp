// license:BSD-3-Clause
// copyright-holders:Alex W. Jackson
/*  NMK112 - NMK custom IC for bankswitching the sample ROMs of a pair of
    OKI6295 ADPCM chips

    The address space of each OKI6295 is divided into four banks, each one
    independently controlled. The sample table at the beginning of the
    address space may be divided in four pages as well, banked together
    with the sample data.  This allows each of the four voices on the chip
    to play a sample from a different bank at the same time. */

#include "emu.h"
#include "nmk112.h"

#define TABLESIZE   0x100
#define BANKSIZE    0x10000



DEFINE_DEVICE_TYPE(NMK112, nmk112_device, "nmk112", "NMK112")

nmk112_device::nmk112_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NMK112, tag, owner, clock)
	, m_page_mask(0xff)
	, m_rom0(*this, finder_base::DUMMY_TAG)
	, m_rom1(*this, finder_base::DUMMY_TAG)
	, m_size0(0)
	, m_size1(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void nmk112_device::device_start()
{
	save_item(NAME(m_current_bank));

	if (m_rom0)
		m_size0 = m_rom0.bytes() - 0x40000;

	if (m_rom1)
		m_size1 = m_rom1.bytes() - 0x40000;
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void nmk112_device::device_reset()
{
	for (int i = 0; i < 8; i++)
	{
		m_current_bank[i] = 0;
		do_bankswitch(i, m_current_bank[i]);
	}
}

void nmk112_device::do_bankswitch( int offset, int data )
{
	int chip = (offset & 4) >> 2;
	int banknum = offset & 3;
	int paged = (m_page_mask & (1 << chip));

	uint8_t *rom = chip ? m_rom1 : m_rom0;
	int size = chip ? m_size1 : m_size0;

	m_current_bank[offset] = data;

	if (size == 0) return;

	int bankaddr = (data * BANKSIZE) % size;

	/* copy the samples */
	if ((paged) && (banknum == 0))
		memcpy(rom + 0x400, rom + 0x40000 + bankaddr + 0x400, BANKSIZE - 0x400);
	else
		memcpy(rom + banknum * BANKSIZE, rom + 0x40000 + bankaddr, BANKSIZE);

	/* also copy the sample address table, if it is paged on this chip */
	if (paged)
	{
		rom += banknum * TABLESIZE;
		memcpy(rom, rom + 0x40000 + bankaddr, TABLESIZE);
	}
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

void nmk112_device::okibank_w(offs_t offset, u8 data)
{
	if (m_current_bank[offset] != data)
		do_bankswitch(offset, data);
}

void nmk112_device::device_post_load()
{
	for (int i = 0; i < 8; i++)
		do_bankswitch(i, m_current_bank[i]);
}
