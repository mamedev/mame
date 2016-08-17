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


const device_type TC0110PCR = &device_creator<tc0110pcr_device>;

tc0110pcr_device::tc0110pcr_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, TC0110PCR, "Taito TC0110PCR", tag, owner, clock, "tc0110pcr", __FILE__),
		m_ram(nullptr),
		m_type(0),
		m_addr(0),
		m_palette(*this)
{
}

//-------------------------------------------------
//  static_set_palette_tag: Set the tag of the
//  palette device
//-------------------------------------------------

void tc0110pcr_device::static_set_palette_tag(device_t &device, const char *tag)
{
	downcast<tc0110pcr_device &>(device).m_palette.set_tag(tag);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tc0110pcr_device::device_start()
{
	m_ram = make_unique_clear<UINT16[]>(TC0110PCR_RAM_SIZE);

	save_pointer(NAME(m_ram.get()), TC0110PCR_RAM_SIZE);
	save_item(NAME(m_type));
	save_item(NAME(m_addr));
	machine().save().register_postload(save_prepost_delegate(FUNC(tc0110pcr_device::restore_colors), this));

}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tc0110pcr_device::device_reset()
{
	m_type = 0;    /* default, xBBBBBGGGGGRRRRR */
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
//logerror("PC %06x: warning - read TC0110PCR address %02x\n",space.device().safe_pc(),offset);
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
//logerror("PC %06x: warning - write %04x to TC0110PCR address %02x\n",space.device().safe_pc(),data,offset);
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
//logerror("PC %06x: warning - write %04x to TC0110PCR address %02x\n",space.device().safe_pc(),data,offset);
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
//logerror("PC %06x: warning - write %04x to TC0110PCR offset %02x\n",space.device().safe_pc(),data,offset);
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
//logerror("PC %06x: warning - write %04x to TC0110PCR address %02x\n",space.device().safe_pc(),data,offset);
			break;
	}
}
