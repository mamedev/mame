// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/**********************************************************************

    LH5810/LH5811 Input/Output Port Controller

    TODO:
    - serial data transfer
    - data transfer to the cassette tape

**********************************************************************/

#include "emu.h"
#include "lh5810.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type LH5810 = &device_creator<lh5810_device>;

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  lh5810_device - constructor
//-------------------------------------------------

lh5810_device::lh5810_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, LH5810, "LH5810", tag, owner, clock, "lh5810", __FILE__),
	m_porta_r_cb(*this),
	m_porta_w_cb(*this),
	m_portb_r_cb(*this),
	m_portb_w_cb(*this),
	m_portc_w_cb(*this),
	m_out_int_cb(*this), m_irq(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void lh5810_device::device_start()
{
	// resolve callbacks
	m_porta_r_cb.resolve_safe(0);
	m_porta_w_cb.resolve_safe();
	m_portb_r_cb.resolve_safe(0);
	m_portb_w_cb.resolve_safe();
	m_portc_w_cb.resolve_safe();
	m_out_int_cb.resolve_safe();

	// register for state saving
	save_item(NAME(m_irq));
	save_item(NAME(m_reg));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void lh5810_device::device_reset()
{
	memset(m_reg, 0, sizeof(m_reg));
	m_irq = 0;
}


//-------------------------------------------------
//  data_r - data read
//-------------------------------------------------

READ8_MEMBER( lh5810_device::data_r )
{
	switch (offset)
	{
		case LH5810_U:
		case LH5810_L:
		case LH5810_G:
		case LH5810_DDA:
		case LH5810_DDB:
		case LH5810_OPC:
		case LH5820_F:
			return m_reg[offset];

		case LH5810_IF:
			if (BIT(m_portb_r_cb(0) & ~m_reg[LH5810_DDB], 7))
				m_reg[offset] |= 2;
			else
				m_reg[offset] &= 0xfd;

			return m_reg[offset];

		case LH5810_MSK:
			return (m_reg[offset]&0x0f) | (m_irq<<4) | (BIT(m_reg[LH5810_OPB],7)<<5);

		case LH5810_OPA:
			m_reg[offset] = (m_reg[offset] & m_reg[LH5810_DDA]) | (m_porta_r_cb(0) & ~m_reg[LH5810_DDA]);
			return m_reg[offset];

		case LH5810_OPB:
			m_reg[offset] = (m_reg[offset] & m_reg[LH5810_DDB]) | (m_portb_r_cb(0) & ~m_reg[LH5810_DDB]);
			m_out_int_cb((m_reg[offset] & 0x80 && m_reg[LH5810_MSK] & 0x02) ? ASSERT_LINE : CLEAR_LINE);
			return m_reg[offset];

		default:
			return 0x00;
	}
}


//-------------------------------------------------
//  data_w - data write
//-------------------------------------------------

WRITE8_MEMBER( lh5810_device::data_w )
{
	switch (offset)
	{
		case LH5810_RESET:
			break;

		case LH5810_G:
		case LH5820_F:
		case LH5810_DDA:
		case LH5810_DDB:
			m_reg[offset] = data;
			break;

		case LH5810_U:
			//writing on U register clear the RD flag of IF register
			m_reg[LH5810_IF] &= 0xfb;
			m_reg[offset] = data;
			break;

		case LH5810_L:
			//writing on L register clear the TD flag of IF register
			m_reg[LH5810_IF] &= 0xf7;
			m_reg[offset] = data;
			break;

		case LH5810_MSK:
			m_reg[offset] = data & 0x0f;
			break;

		case LH5810_IF:
			//only bit 0 and 1 are writable
			m_reg[offset] = (m_reg[offset] & 0xfc) | (data & 0x03);
			break;

		case LH5810_OPA:
			m_reg[offset] = (data & m_reg[LH5810_DDA]) | (m_reg[offset] & ~m_reg[LH5810_DDA]);
			m_porta_w_cb((offs_t)0, m_reg[offset]);
			break;

		case LH5810_OPB:
			m_reg[offset] = (data & m_reg[LH5810_DDB]) | (m_reg[offset] & ~m_reg[LH5810_DDB]);
			m_portb_w_cb((offs_t)0, m_reg[offset]);
			m_out_int_cb((m_reg[offset] & 0x80 && m_reg[LH5810_MSK] & 0x02) ? ASSERT_LINE : CLEAR_LINE);
			break;

		case LH5810_OPC:
			m_reg[offset] = data;
			m_portc_w_cb((offs_t)0, m_reg[offset]);
			break;
	}
}
