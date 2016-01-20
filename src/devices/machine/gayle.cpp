// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    GAYLE

    Gate array used in the Amiga 600 and Amiga 1200 computers.

***************************************************************************/

#include "gayle.h"


//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define VERBOSE 0


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type GAYLE = &device_creator<gayle_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  gayle_device - constructor
//-------------------------------------------------

gayle_device::gayle_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, GAYLE, "GAYLE", tag, owner, clock, "gayle", __FILE__),
	m_int2_w(*this),
	m_cs0_read(*this),
	m_cs0_write(*this),
	m_cs1_read(*this),
	m_cs1_write(*this),
	m_gayle_id(0xff),
	m_gayle_id_count(0)
{
}

//-------------------------------------------------
//  set_id - set gayle id
//-------------------------------------------------

void gayle_device::set_id(device_t &device, UINT8 id)
{
	gayle_device &gayle = downcast<gayle_device &>(device);
	gayle.m_gayle_id = id;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void gayle_device::device_start()
{
	// resolve callbacks
	m_int2_w.resolve_safe();
	m_cs0_read.resolve_safe(0xffff);
	m_cs0_write.resolve_safe();
	m_cs1_read.resolve_safe(0xffff);
	m_cs1_write.resolve_safe();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void gayle_device::device_reset()
{
	m_gayle_reg[0] = 0;
	m_gayle_reg[1] = 0;
	m_gayle_reg[2] = 0;
	m_gayle_reg[3] = 0;
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

READ16_MEMBER( gayle_device::gayle_r )
{
	UINT16 data = 0xffff;
	offset <<= 1;

	// swap
	mem_mask = (mem_mask << 8) | (mem_mask >> 8);

	if (BIT(offset, 15))
	{
		switch (offset & 0x7fff)
		{
		case 0x0000: data = m_gayle_reg[0]; break;
		case 0x1000: data = m_gayle_reg[1]; break;
		case 0x2000: data = m_gayle_reg[2]; break;
		case 0x3000: data = m_gayle_reg[3]; break;
		}
	}
	else
	{
		if (!BIT(offset, 14))
		{
			if (BIT(offset, 13))
				data = m_cs0_read(space, (offset >> 2) & 0x07, mem_mask);
			else
				data = m_cs1_read(space, (offset >> 2) & 0x07, mem_mask);
		}
	}

	if (VERBOSE)
		logerror("gayle_r(%06x): %04x & %04x\n", offset, data, mem_mask);

	// swap data
	data = (data << 8) | (data >> 8);

	return data;
}

WRITE16_MEMBER( gayle_device::gayle_w )
{
	offset <<= 1;

	// swap
	mem_mask = (mem_mask << 8) | (mem_mask >> 8);
	data = ((data << 8) | (data >> 8)) & mem_mask;

	if (VERBOSE)
		logerror("gayle_w(%06x): %04x & %04x\n", offset, data, mem_mask);

	if (BIT(offset, 15))
	{
		switch (offset & 0x7fff)
		{
		case 0x0000:
			m_gayle_reg[0] = data;
			break;
		case 0x1000:
			m_gayle_reg[1] &= data;
			m_gayle_reg[1] |= data & 0x03;
			break;
		case 0x2000:
			m_gayle_reg[2] = data;
			break;
		case 0x3000:
			m_gayle_reg[3] = data;
			break;
		}
	}
	else
	{
		if (!BIT(offset, 14))
		{
			if (BIT(offset, 13))
				m_cs0_write(space, (offset >> 2) & 0x07, data, mem_mask);
			else
				m_cs1_write(space, (offset >> 2) & 0x07, data, mem_mask);
		}
	}
}

WRITE_LINE_MEMBER( gayle_device::ide_interrupt_w )
{
	if (VERBOSE)
		logerror("ide_interrupt_w: %d\n", state);

	// did we change state?
	if (BIT(m_gayle_reg[GAYLE_CS], 7) != state)
		m_gayle_reg[GAYLE_IRQ] |= 1 << 7;

	// set line state
	if (state)
		m_gayle_reg[GAYLE_CS] |= 1 << 7;
	else
		m_gayle_reg[GAYLE_CS] &= ~(1 << 7);

	// update interrupts
	if (BIT(m_gayle_reg[GAYLE_INTEN], 7))
		m_int2_w(BIT(m_gayle_reg[GAYLE_CS], 7));
}

READ16_MEMBER( gayle_device::gayle_id_r )
{
	UINT16 data;

	if (ACCESSING_BITS_8_15)
		data = ((m_gayle_id << m_gayle_id_count++) & 0x80) << 8;
	else
		data = 0xffff;

	if (VERBOSE)
		logerror("gayle_id_r(%06x): %04x & %04x (id=%02x)\n", offset, data, mem_mask, m_gayle_id);

	return data;
}

WRITE16_MEMBER( gayle_device::gayle_id_w )
{
	if (VERBOSE)
		logerror("gayle_id_w(%06x): %04x & %04x (id=%02x)\n", offset, data, mem_mask, m_gayle_id);

	m_gayle_id_count = 0;
}
