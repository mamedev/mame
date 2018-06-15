// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Thomson EF9369

***************************************************************************/

#include "emu.h"
#include "ef9369.h"

#include <algorithm>

/*static*/ constexpr int ef9369_device::NUMCOLORS;

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(EF9369, ef9369_device, "ef9369", "Thomson EF9369 Single Chip Color Palette")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  ef9369_device - constructor
//-------------------------------------------------

ef9369_device::ef9369_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, EF9369, tag, owner, clock)
	, m_address(0)
{
	std::fill(m_ca, m_ca + NUMCOLORS, 0);
	std::fill(m_cb, m_cb + NUMCOLORS, 0);
	std::fill(m_cc, m_cc + NUMCOLORS, 0);
	std::fill(m_m, m_m + NUMCOLORS, 0);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ef9369_device::device_start()
{
	// bind delegate
	m_color_update_cb.bind_relative_to(*owner());

	// register for save states
	save_pointer(NAME(m_ca), NUMCOLORS);
	save_pointer(NAME(m_cb), NUMCOLORS);
	save_pointer(NAME(m_cc), NUMCOLORS);
	save_pointer(NAME(m_m), NUMCOLORS);
	save_item(NAME(m_address));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ef9369_device::device_reset()
{
	m_address = 0;
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

READ8_MEMBER( ef9369_device::data_r )
{
	if (m_address & 1)
		return m_m[m_address >> 1] << 4 | m_cc[m_address >> 1];
	else
		return m_cb[m_address >> 1] << 4 | m_ca[m_address >> 1];
}

WRITE8_MEMBER( ef9369_device::data_w )
{
	const int entry = m_address >> 1;

	if (m_address & 1)
	{
		m_m[entry] = (data >> 4) & 0x1;
		m_cc[entry] = (data >> 0) & 0xf;
	}
	else
	{
		m_cb[entry] = (data >> 4) & 0xf;
		m_ca[entry] = (data >> 0) & 0xf;
	}

	// update color
	if (!m_color_update_cb.isnull())
		m_color_update_cb(entry, m_m[entry], m_ca[entry], m_cb[entry], m_cc[entry]);

	// auto-increment
	m_address++;
	m_address &= 0x1f;
}

WRITE8_MEMBER( ef9369_device::address_w )
{
	m_address = data & 0x1f;    // 5-bit
}
