// license:BSD-3-Clause
// copyright-holders:cam900
/***************************************************************************

    Alpha denshi ALPHA-8921 emulation

    Also known as
    SNK PRO-CT0
    SNK-9201

    This chip is sprite ROM data serializer, or optional security device.
    used in some later 80s Alpha Denshi hardware(ex: Gang Wars),
    Some early Neogeo MVS motherboards, AES cartridges.
    also integrated in NEO-ZMC2, NEO-CMC.

    reference: https://wiki.neogeodev.org/index.php?title=PRO-CT0

***************************************************************************/

#include "emu.h"
#include "alpha_8921.h"
#include <algorithm>


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(ALPHA_8921, alpha_8921_device, "alpha_8921", "Alpha denshi ALPHA-8921")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  alpha_8921_device - constructor
//-------------------------------------------------

alpha_8921_device::alpha_8921_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ALPHA_8921, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void alpha_8921_device::device_start()
{
	// save states
	save_item(NAME(m_clk));
	save_item(NAME(m_load));
	save_item(NAME(m_even));
	save_item(NAME(m_h));
	save_item(NAME(m_c));
	save_item(NAME(m_gad));
	save_item(NAME(m_gbd));
	save_item(NAME(m_dota));
	save_item(NAME(m_dotb));
	save_item(NAME(m_sr));
	save_item(NAME(m_old_sr));
	save_item(NAME(m_old_even));
	save_item(NAME(m_old_h));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void alpha_8921_device::device_reset()
{
	// force update outputs
	m_old_sr = ~m_sr;
	m_old_even = !m_even;
	m_old_h = !m_h;
	update_output();
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

//-------------------------------------------------
//  clk_w - Change clock pin status
//-------------------------------------------------

WRITE_LINE_MEMBER(alpha_8921_device::clk_w)
{
	if (m_clk != state)
	{
		m_clk = state;
		if (!m_clk) // falling edge
		{
			if (m_load)
				m_sr = m_c;
			else if (m_h)
				m_sr = (BIT(m_sr, 24, 6) << 26) | (BIT(m_sr, 16, 6) << 18) | (BIT(m_sr, 8, 6) << 10) | (BIT(m_sr, 0, 6) << 2);
			else
				m_sr = ((BIT(m_sr, 26, 6)) << 24) | ((BIT(m_sr, 18, 6)) << 16) | ((BIT(m_sr, 10, 6)) << 8) | (BIT(m_sr, 2, 6));
		}
	}
}

//-------------------------------------------------
//  load_w - Change LOAD pin status
//-------------------------------------------------

WRITE_LINE_MEMBER(alpha_8921_device::load_w)
{
	m_load = state;
}

//-------------------------------------------------
//  even_w - Change EVEN pin status
//-------------------------------------------------

WRITE_LINE_MEMBER(alpha_8921_device::even_w)
{
	m_even = state;
}

//-------------------------------------------------
//  h_w - Change H pin status
//-------------------------------------------------

WRITE_LINE_MEMBER(alpha_8921_device::h_w)
{
	m_h = state;
}

//-------------------------------------------------
//  c_w - Change C data
//-------------------------------------------------

void alpha_8921_device::c_w(u32 data)
{
	m_c = data;
}

//-------------------------------------------------
//  update_output - Update output results
//-------------------------------------------------

void alpha_8921_device::update_output()
{
	if ((m_old_sr != m_sr) || (m_old_even != m_even) || (m_old_h != m_h))
	{
		if (m_h)
		{
			m_gbd = bitswap<4>(m_sr, 30, 22, 14, 6);
			m_gad = bitswap<4>(m_sr, 31, 23, 15, 7);
		}
		else
		{
			m_gbd = bitswap<4>(m_sr, 25, 17, 9, 1);
			m_gad = bitswap<4>(m_sr, 24, 16, 8, 0);
		}
		if (m_even)
			std::swap<u8>(m_gad, m_gbd);

		m_dota = m_gad ? true : false;
		m_dotb = m_gbd ? true : false;
		m_old_sr = m_sr;
		m_old_even = m_even;
		m_old_h = m_h;
	}
}

//-------------------------------------------------
//  gad_r - Read GAD data
//-------------------------------------------------

u8 alpha_8921_device::gad_r()
{
	update_output();
	return m_gad & 0xf;
}

//-------------------------------------------------
//  gbd_r - Read GBD data
//-------------------------------------------------

u8 alpha_8921_device::gbd_r()
{
	update_output();
	return m_gbd & 0xf;
}

//-------------------------------------------------
//  dota_r - Read DOTA pin data (GAD isn't 0)
//-------------------------------------------------

READ_LINE_MEMBER(alpha_8921_device::dota_r)
{
	update_output();
	return m_dota;
}

//-------------------------------------------------
//  dotb_r - Read DOTB pin data (GBD isn't 0)
//-------------------------------------------------

READ_LINE_MEMBER(alpha_8921_device::dotb_r)
{
	update_output();
	return m_dotb;
}
