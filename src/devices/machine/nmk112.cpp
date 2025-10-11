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
	, m_samplebank{ {*this, "samplebank_0_%u", 0U}, {*this, "samplebank_1_%u", 0U} }
	, m_tablebank{ {*this, "tablebank_0_%u", 0U}, {*this, "tablebank_1_%u", 0U} }
	, m_rom(*this, { finder_base::DUMMY_TAG, finder_base::DUMMY_TAG })
	, m_oki0_space(*this, finder_base::DUMMY_TAG, -1)
	, m_oki1_space(*this, finder_base::DUMMY_TAG, -1)
	, m_page_mask(0xff)
	, m_current_bank{0}
	, m_size{0}
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void nmk112_device::device_start()
{
	save_item(NAME(m_current_bank));

	for (int c = 0; c < 2; c++)
	{
		if (m_rom[c])
		{
			const bool paged = is_paged(c);
			m_size[c] = m_rom[c].bytes();
			// bank slot configurations
			for (int i = 0; i < 4; i++)
			{
				for (int b = 0; b < 256; b++)
				{
					m_samplebank[c][i]->configure_entry(b,
						m_rom[c] + ((page_offset(c, i) + (b << 16)) % m_size[c]));
					if (paged)
						m_tablebank[c][i]->configure_entry(b,
							m_rom[c] + (((i << 8) + (b << 16)) % m_size[c]));
				}
				m_samplebank[c][i]->set_entry(0);
				if (paged)
					m_tablebank[c][i]->set_entry(0);
			}
			// install banks
			address_space *space = (c == 0) ? m_oki0_space : m_oki1_space;
			if (space != nullptr)
			{
				space->unmap_read(0x00000, 0x3ffff);
				for (int i = 0; i < 4; i++)
				{
					space->install_read_bank((i << 16) | page_offset(c, i), (i << 16) | 0xffff, m_samplebank[c][i]);
					if (paged)
						space->install_read_bank(i << 8, (i << 8) | 0xff, m_tablebank[c][i]);
				}
			}
		}
	}
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

void nmk112_device::do_bankswitch(offs_t offset, uint8_t data)
{
	const int chip = BIT(offset, 2);
	const int banknum = offset & 3;

	m_current_bank[offset] = data;

	if (m_size[chip] == 0) return;

	m_samplebank[chip][banknum]->set_entry(data);
	if (is_paged(chip))
		m_tablebank[chip][banknum]->set_entry(data);
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
