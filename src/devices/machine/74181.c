// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    74181

    4-Bit Arithmetic Logic Unit

***************************************************************************/

#include "74181.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type TTL74181 = &device_creator<ttl74181_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  ttl74181_device - constructor
//-------------------------------------------------

ttl74181_device::ttl74181_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, TTL74181, "74181 TTL", tag, owner, clock, "ttl74181", __FILE__),
	m_a(0),
	m_b(0),
	m_s(0),
	m_m(0),
	m_c(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ttl74181_device::device_start()
{
	// register for state saving
	save_item(NAME(m_a));
	save_item(NAME(m_b));
	save_item(NAME(m_s));
	save_item(NAME(m_m));
	save_item(NAME(m_c));

	update();
}

//-------------------------------------------------
//  device_post_load - called after loading a saved state
//-------------------------------------------------

void ttl74181_device::device_post_load()
{
	// after loading a state re-initialize our output lines
	update();
}


//**************************************************************************
//  ARITHMETHIC UNIT
//**************************************************************************

void ttl74181_device::update()
{
	// inputs
	int a0 = BIT(m_a, 0), a1 = BIT(m_a, 1), a2 = BIT(m_a, 2), a3 = BIT(m_a, 3);
	int b0 = BIT(m_b, 0), b1 = BIT(m_b, 1), b2 = BIT(m_b, 2), b3 = BIT(m_b, 3);
	int s0 = BIT(m_s, 0), s1 = BIT(m_s, 1), s2 = BIT(m_s, 2), s3 = BIT(m_s, 3);
	int mp = !m_m;

	// intermediate calculations
	int ap0 = !(a0 | (b0 & s0) | (s1 & !b0));
	int bp0 = !(((!b0) & s2 & a0) | (a0 & b0 & s3));
	int ap1 = !(a1 | (b1 & s0) | (s1 & !b1));
	int bp1 = !(((!b1) & s2 & a1) | (a1 & b1 & s3));
	int ap2 = !(a2 | (b2 & s0) | (s1 & !b2));
	int bp2 = !(((!b2) & s2 & a2) | (a2 & b2 & s3));
	int ap3 = !(a3 | (b3 & s0) | (s1 & !b3));
	int bp3 = !(((!b3) & s2 & a3) | (a3 & b3 & s3));

	int fp0 = !(m_c & mp) ^ ((!ap0) & bp0);
	int fp1 = (!((mp & ap0) | (mp & bp0 & m_c))) ^ ((!ap1) & bp1);
	int fp2 = (!((mp & ap1) | (mp & ap0 & bp1) | (mp & m_c & bp0 & bp1))) ^ ((!ap2) & bp2);
	int fp3 = (!((mp & ap2) | (mp & ap1 & bp2) | (mp & ap0 & bp1 & bp2) | (mp & m_c & bp0 & bp1 & bp2))) ^ ((!ap3) & bp3);

	// outputs
	m_f = fp0 | fp1 << 1 | fp2 << 2 | fp3 << 3;
	m_equals = fp0 & fp1 & fp2 & fp3;
	m_p = !(bp0 & bp1 & bp2 & bp3);
	m_g = !((ap0 & bp1 & bp2 & bp3) | (ap1 & bp2 & bp3) | (ap2 & bp3) | ap3);
	m_cn = (!(m_c & bp0 & bp1 & bp2 & bp3)) | m_g;
}

void ttl74181_device::input_a_w(UINT8 data)
{
	data &= 0x0f;

	if (m_a != data)
	{
		m_a = data;
		update();
	}
}

void ttl74181_device::input_b_w(UINT8 data)
{
	data &= 0x0f;

	if (m_b != data)
	{
		m_b = data;
		update();
	}
}

void ttl74181_device::select_w(UINT8 data)
{
	data &= 0x0f;

	if (m_s != data)
	{
		m_s = data;
		update();
	}
}

WRITE_LINE_MEMBER( ttl74181_device::mode_w )
{
	if (m_m != state)
	{
		m_m = state;
		update();
	}
}

WRITE_LINE_MEMBER( ttl74181_device::carry_w )
{
	if (m_c != state)
	{
		m_c = state;
		update();
	}
}
