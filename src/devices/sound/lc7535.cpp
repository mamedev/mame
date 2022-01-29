// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Sanyo LC7535

    Electronic Volume/Loudness Control with Serial Data Control

    It uses the Sanyo CCB (Computer Control Bus) to communicate. The
    device address is 0x08 or 0x09 (depending on the select pin).

***************************************************************************/

#include "emu.h"
#include "lc7535.h"


//**************************************************************************
//  CONSTEXPR DEFINITIONS
//**************************************************************************

constexpr int lc7535_device::m_5db[];
constexpr int lc7535_device::m_1db[];


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(LC7535, lc7535_device, "lc7535", "Sanyo LC7535")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  lc7535_device - constructor
//-------------------------------------------------

lc7535_device::lc7535_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, LC7535, tag, owner, clock),
	m_select_cb(*this),
	m_volume_cb(*this),
	m_addr(0), m_data(0),
	m_count(0),
	m_ce(0), m_di(0), m_clk(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void lc7535_device::device_start()
{
	// resolve callbacks
	m_select_cb.resolve();
	m_volume_cb.resolve();

	// register for save states
	save_item(NAME(m_addr));
	save_item(NAME(m_data));
	save_item(NAME(m_count));
	save_item(NAME(m_ce));
	save_item(NAME(m_di));
	save_item(NAME(m_clk));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void lc7535_device::device_reset()
{
}

//-------------------------------------------------
//  normalize - convert attenuation to range 0-1
//-------------------------------------------------

float lc7535_device::normalize(int attenuation)
{
	return (attenuation + (MAX * (-1.0))) / (MAX * (-1.0));
}


//**************************************************************************
//  INTERFACE
//**************************************************************************

WRITE_LINE_MEMBER( lc7535_device::ce_w )
{
	m_ce = state;
}

WRITE_LINE_MEMBER( lc7535_device::di_w )
{
	m_di = state;
}

WRITE_LINE_MEMBER( lc7535_device::clk_w )
{
	if (m_clk == 0 && state == 1)
	{
		if (m_ce == 0)
		{
			if (m_count == 0)
				m_addr = 0;

			m_addr <<= 1;
			m_addr |= m_di;

			if (++m_count == 4)
			{
				m_addr = bitswap(m_addr, 0, 1, 2, 3);
				m_count = 0;
			}
		}
		else
		{
			// our address is either 8 or 9
			if (m_addr == (8 | (m_select_cb() ? 0 : 1)))
			{
				m_data <<= 1;
				m_data |= m_di;

				if (++m_count == 16)
				{
					m_data = bitswap(m_data, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);

					// right channel
					int att_r = m_1db[(m_data >>  4) & 0x07];
					if (att_r != MAX)
						att_r += m_5db[(m_data >> 0) & 0x0f];

					// left channel
					int att_l = m_1db[(m_data >>  12) & 0x07];
					if (att_l != MAX)
						att_l += m_5db[(m_data >> 8) & 0x0f];

					bool loudness = bool(BIT(m_data, 7));

					if (!m_volume_cb.isnull())
						m_volume_cb(att_r, att_l, loudness);

					m_addr = 0;
					m_count = 0;
				}
			}
		}
	}

	m_clk = state;
}
