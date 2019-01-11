// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*
Taito TC0110PCR
---------
Interface to palette RAM, and simple tilemap/sprite priority handler. The
priority order seems to be fixed.
The data bus is 16 bits wide.

000  W selects palette RAM address
002 RW read/write palette RAM
004  W unknown, often written to
*/

#include "emu.h"
#include "tc0110pcr.h"

#define TC0110PCR_RAM_SIZE 0x2000


DEFINE_DEVICE_TYPE(TC0110PCR, tc0110pcr_device, "tc0110pcr", "Taito TC0110PCR")

tc0110pcr_device::tc0110pcr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TC0110PCR, tag, owner, clock)
	, m_ram(nullptr)
	, m_type(0)
	, m_addr(0)
	, m_palette(*this, finder_base::DUMMY_TAG)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tc0110pcr_device::device_start()
{
	m_ram = make_unique_clear<uint16_t[]>(TC0110PCR_RAM_SIZE);

	save_pointer(NAME(m_ram), TC0110PCR_RAM_SIZE);
	save_item(NAME(m_type));
	save_item(NAME(m_addr));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tc0110pcr_device::device_reset()
{
	m_type = 0;    /* default, xBBBBBGGGGGRRRRR */
}

//-------------------------------------------------
//  device_post_load - device-specific postload
//-------------------------------------------------

void tc0110pcr_device::device_post_load()
{
	restore_colors();
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

void tc0110pcr_device::restore_colors()
{
	int i, color, r = 0, g = 0, b = 0;

	for (i = 0; i < (256 * 16); i++)
	{
		color = m_ram[i];

		switch (m_type)
		{
			case 0x00:
			{
				r = pal5bit(color >>  0);
				g = pal5bit(color >>  5);
				b = pal5bit(color >> 10);
				break;
			}

			case 0x01:
			{
				b = pal5bit(color >>  0);
				g = pal5bit(color >>  5);
				r = pal5bit(color >> 10);
				break;
			}

			case 0x02:
			{
				r = pal4bit(color >> 0);
				g = pal4bit(color >> 4);
				b = pal4bit(color >> 8);
				break;
			}
		}

		m_palette->set_pen_color(i, rgb_t(r, g, b));
	}
}


READ16_MEMBER(tc0110pcr_device::word_r )
{
	switch (offset)
	{
		case 1:
			return m_ram[m_addr];

		default:
//logerror("%s: warning - read TC0110PCR address %02x\n",m_maincpu->pc(),offset);
			return 0xff;
	}
}

WRITE16_MEMBER(tc0110pcr_device::word_w )
{
	switch (offset)
	{
		case 0:
			/* In test mode game writes to odd register number so (data>>1) */
			m_addr = (data >> 1) & 0xfff;
			if (data > 0x1fff)
				logerror ("Write to palette index > 0x1fff\n");
			break;

		case 1:
			m_ram[m_addr] = data & 0xffff;
			m_palette->set_pen_color(m_addr, pal5bit(data >> 0), pal5bit(data >> 5), pal5bit(data >> 10));
			break;

		default:
//logerror("%s: warning - write %04x to TC0110PCR address %02x\n",m_maincpu->pc(),data,offset);
			break;
	}
}

WRITE16_MEMBER(tc0110pcr_device::step1_word_w )
{
	switch (offset)
	{
		case 0:
			m_addr = data & 0xfff;
			if (data > 0xfff)
				logerror ("Write to palette index > 0xfff\n");
			break;

		case 1:
			m_ram[m_addr] = data & 0xffff;
			m_palette->set_pen_color(m_addr, pal5bit(data >> 0), pal5bit(data >> 5), pal5bit(data >> 10));
			break;

		default:
//logerror("%s: warning - write %04x to TC0110PCR address %02x\n",m_maincpu->pc(),data,offset);
			break;
	}
}

WRITE16_MEMBER(tc0110pcr_device::step1_rbswap_word_w )
{
	m_type = 1;    /* xRRRRRGGGGGBBBBB */

	switch (offset)
	{
		case 0:
			m_addr = data & 0xfff;
			if (data > 0xfff)
				logerror ("Write to palette index > 0xfff\n");
			break;

		case 1:
			m_ram[m_addr] = data & 0xffff;
			m_palette->set_pen_color(m_addr, pal5bit(data >> 10), pal5bit(data >> 5), pal5bit(data >> 0));
			break;

		default:
//logerror("%s: warning - write %04x to TC0110PCR offset %02x\n",m_maincpu->pc(),data,offset);
			break;
	}
}

WRITE16_MEMBER(tc0110pcr_device::step1_4bpg_word_w )
{
	m_type = 2;    /* xxxxBBBBGGGGRRRR */

	switch (offset)
	{
		case 0:
			m_addr = data & 0xfff;
			if (data > 0xfff)
				logerror ("Write to palette index > 0xfff\n");
			break;

		case 1:
			m_ram[m_addr] = data & 0xffff;
			m_palette->set_pen_color(m_addr, pal4bit(data >> 0), pal4bit(data >> 4), pal4bit(data >> 8));
			break;

		default:
//logerror("%s: warning - write %04x to TC0110PCR address %02x\n",m_maincpu->pc(),data,offset);
			break;
	}
}
