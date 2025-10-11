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



DEFINE_DEVICE_TYPE(NMK112, nmk112_device, "nmk112", "NMK112")

nmk112_device::nmk112_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NMK112, tag, owner, clock)
	, m_samplebank{ { *this, "samplebank_0_%u", 0U }, { *this, "samplebank_1_%u", 0U } }
	, m_tablebank{ { *this, "tablebank_0_%u", 0U }, { *this, "tablebank_1_%u", 0U } }
	, m_rom(*this, { finder_base::DUMMY_TAG, finder_base::DUMMY_TAG })
	, m_page_mask(0xff)
	, m_size{ 0, 0 }
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void nmk112_device::device_start()
{
	for (int c = 0; c < 2; c++)
	{
		if (m_rom[c])
		{
			m_size[c] = m_rom[c].bytes();

			// bank slot configurations
			for (int i = 0; i < 4; i++)
			{
				for (int b = 0; b < 256; b++)
				{
					if (m_samplebank[c][i])
						m_samplebank[c][i]->configure_entry(b, &m_rom[c][(b << 16) % m_size[c]]);

					if (m_tablebank[c][i])
						m_tablebank[c][i]->configure_entry(b, &m_rom[c][((i << 8) + (b << 16)) % m_size[c]]);
				}

				if (m_samplebank[c][i])
					m_samplebank[c][i]->set_entry(0);

				if (m_tablebank[c][i])
					m_tablebank[c][i]->set_entry(0);
			}
		}
	}
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void nmk112_device::device_reset()
{
	for (offs_t i = 0; i < 8; i++)
		okibank_w(i, 0);
}


/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

void nmk112_device::okibank_w(offs_t offset, u8 data)
{
	const int chip = BIT(offset, 2);
	const int banknum = offset & 3;

	if (m_size[chip])
	{
		if (m_samplebank[chip][banknum])
			m_samplebank[chip][banknum]->set_entry(data);

		if (m_tablebank[chip][banknum])
			m_tablebank[chip][banknum]->set_entry(data);
	}
}


/*****************************************************************************
    ADDRESS MAPS
*****************************************************************************/

void nmk112_device::oki0_map(address_map &map)
{
	oki_map(0, map);
}

void nmk112_device::oki1_map(address_map &map)
{
	oki_map(1, map);
}

void nmk112_device::oki_map(unsigned which, address_map &map)
{
	map(0x00000, 0x0ffff).bankr(m_samplebank[which][0]);
	map(0x10000, 0x1ffff).bankr(m_samplebank[which][1]);
	map(0x20000, 0x2ffff).bankr(m_samplebank[which][2]);
	map(0x30000, 0x3ffff).bankr(m_samplebank[which][3]);

	// these get installed over the top of the first bank if present
	if (is_paged(which))
	{
		map(0x00000, 0x000ff).bankr(m_tablebank[which][0]);
		map(0x00100, 0x001ff).bankr(m_tablebank[which][1]);
		map(0x00200, 0x002ff).bankr(m_tablebank[which][2]);
		map(0x00300, 0x003ff).bankr(m_tablebank[which][3]);
	}
}
