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

#include "bus/generic/slot.h"


DEFINE_DEVICE_TYPE(NMK112, nmk112_device, "nmk112", "NMK112")

nmk112_device::nmk112_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NMK112, tag, owner, clock)
	, m_samplebank{ { *this, "samplebank_0_%u", 0U }, { *this, "samplebank_1_%u", 0U } }
	, m_tablebank{ { *this, "tablebank_0_%u", 0U }, { *this, "tablebank_1_%u", 0U } }
	, m_rom(*this, { finder_base::DUMMY_TAG, finder_base::DUMMY_TAG })
	, m_page_mask(0xff)
	, m_bankmask{ 0, 0 }
{
}

void nmk112_device::device_start()
{
	for (int c = 0; c < 2; c++)
	{
		if (m_rom[c])
		{
			const auto size = m_rom[c].bytes();
			if (size & 0x0ffff)
			{
				throw emu_fatalerror(
						"%s: ROM region %s has unsupported size 0x%X (must be a multiple of 64KiB)",
						tag(),
						m_rom[c].finder_tag(),
						size);
			}

			// bank slot configurations
			m_bankmask[c] = device_generic_cart_interface::map_non_power_of_two(
					std::min<unsigned>(size / 0x10000, 0x100),
					[this, c] (unsigned entry, unsigned page)
					{
						const uint32_t pagebase = uint32_t(page) << 16;
						for (unsigned i = 0; i < 4; i++)
						{
							const uint32_t tablebase = pagebase | (i << 8);
							m_samplebank[c][i]->configure_entry(entry, &m_rom[c][pagebase]);
							if (is_paged(c))
								m_tablebank[c][i]->configure_entry(entry, &m_rom[c][tablebase]);
						}
					});

			for (unsigned i = 0; i < 4; i++)
			{
				m_samplebank[c][i]->set_entry(0);
				if (is_paged(c))
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

	if (m_bankmask[chip])
	{
		m_samplebank[chip][banknum]->set_entry(data & m_bankmask[chip]);

		if (is_paged(chip))
			m_tablebank[chip][banknum]->set_entry(data & m_bankmask[chip]);
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
