// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    SN54/74153

***************************************************************************/

#include "emu.h"
#include <algorithm>
#include "74153.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type TTL153 = device_creator<ttl153_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  ttl153_device - constructor
//-------------------------------------------------

ttl153_device::ttl153_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, TTL153, "SN54/74153", tag, owner, clock, "ttl153", __FILE__),
	m_za_cb(*this),
	m_zb_cb(*this),
	m_s{ false, false },
	m_ia{ false, false, false, false },
	m_ib{ false, false, false, false },
	m_z{ false, false }
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ttl153_device::device_start()
{
	// resolve callbacks
	m_za_cb.resolve_safe();
	m_zb_cb.resolve_safe();

	// register for save states
	save_pointer(NAME(m_s), 2);
	save_pointer(NAME(m_ia), 4);
	save_pointer(NAME(m_ib), 4);
	save_pointer(NAME(m_z), 2);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ttl153_device::device_reset()
{
	std::fill(std::begin(m_s), std::end(m_s), false);
	std::fill(std::begin(m_ia), std::end(m_ia), false);
	std::fill(std::begin(m_ib), std::end(m_ib), false);
	std::fill(std::begin(m_z), std::end(m_z), false);
}

//-------------------------------------------------
//  update_a - update output a state
//-------------------------------------------------

void ttl153_device::update_a()
{
	// calculate state
	bool za = 0;
	za |= m_ia[0] && !m_s[1] && !m_s[0];
	za |= m_ia[1] && !m_s[1] &&  m_s[0];
	za |= m_ia[2] &&  m_s[1] && !m_s[0];
	za |= m_ia[3] &&  m_s[1] &&  m_s[0];

	// output
	if (za != m_z[0])
		m_za_cb(za ? 1 : 0);

	m_z[0] = za;
}

//-------------------------------------------------
//  update_b - update output b state
//-------------------------------------------------

void ttl153_device::update_b()
{
	// calculate state
	bool zb = 0;
	zb |= m_ib[0] && !m_s[1] && !m_s[0];
	zb |= m_ib[1] && !m_s[1] &&  m_s[0];
	zb |= m_ib[2] &&  m_s[1] && !m_s[0];
	zb |= m_ib[3] &&  m_s[1] &&  m_s[0];

	// output
	if (zb != m_z[1])
		m_zb_cb(zb ? 1 : 0);

	m_z[1] = zb;
}


//**************************************************************************
//  INTERFACE
//**************************************************************************

WRITE_LINE_MEMBER( ttl153_device::s0_w )
{
	m_s[0] = bool(state);
	update_a();
	update_b();
}

WRITE_LINE_MEMBER( ttl153_device::s1_w )
{
	m_s[1] = bool(state);
	update_a();
	update_b();
}

WRITE_LINE_MEMBER( ttl153_device::i0a_w )
{
	m_ia[0] = bool(state);
	update_a();
}

WRITE_LINE_MEMBER( ttl153_device::i1a_w )
{
	m_ia[1] = bool(state);
	update_a();
}

WRITE_LINE_MEMBER( ttl153_device::i2a_w )
{
	m_ia[2] = bool(state);
	update_a();
}

WRITE_LINE_MEMBER( ttl153_device::i3a_w )
{
	m_ia[3] = bool(state);
	update_a();
}

WRITE_LINE_MEMBER( ttl153_device::i0b_w )
{
	m_ib[0] = bool(state);
	update_b();
}

WRITE_LINE_MEMBER( ttl153_device::i1b_w )
{
	m_ib[1] = bool(state);
	update_b();
}

WRITE_LINE_MEMBER( ttl153_device::i2b_w )
{
	m_ib[2] = bool(state);
	update_b();
}

WRITE_LINE_MEMBER( ttl153_device::i3b_w )
{
	m_ib[3] = bool(state);
	update_b();
}
