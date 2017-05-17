// license:BSD-3-Clause
// copyright-holders:AJR

/***************************************************************************

    74LS157/74HCT157 Quad 2 to 1-Line Data Selectors/Multiplexers (TTL)

    Often used to help feed 8-bit ROM data into a MSM5205, and for many
    other purposes.

    74LS158 has inverted outputs; 74LS157 is non-inverting.

***************************************************************************/

#include "emu.h"
#include "74157.h"


//**************************************************************************
//  74LS157 DEVICE
//**************************************************************************

DEFINE_DEVICE_TYPE(LS157, ls157_device, "74ls157", "74LS157 Quad 2-to-1 Multiplexer")

//-------------------------------------------------
//  ls157_device - constructor
//-------------------------------------------------

ls157_device::ls157_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ls157_device(mconfig, LS157, tag, owner, clock)
{
}

ls157_device::ls157_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_out_cb(*this)
{
	m_a = 0;
	m_b = 0;
	m_select = false;
	m_strobe = false;
}


//-------------------------------------------------
//  device_start - perform device-specific
//  startup
//-------------------------------------------------

void ls157_device::device_start()
{
	// resolve callbacks
	m_out_cb.resolve_safe();

	// register items for save state
	save_item(NAME(m_a));
	save_item(NAME(m_b));
	save_item(NAME(m_select));
	save_item(NAME(m_strobe));
}


//**************************************************************************
//  DATA INPUTS
//**************************************************************************

//-------------------------------------------------
//  a_w -- write nibble to A1-A4
//-------------------------------------------------

WRITE8_MEMBER(ls157_device::a_w)
{
	a_w(data);
}

void ls157_device::a_w(u8 data)
{
	m_a = data & 0xf;
	update_output();
}


//-------------------------------------------------
//  b_w -- write nibble to B1-B4
//-------------------------------------------------

WRITE8_MEMBER(ls157_device::b_w)
{
	b_w(data);
}

void ls157_device::b_w(u8 data)
{
	m_b = data & 0xf;
	update_output();
}


//-------------------------------------------------
//  ab_w -- write high nibble to A1-A4 and write
//  low nibble to B1-B4
//-------------------------------------------------

WRITE8_MEMBER(ls157_device::ab_w)
{
	ab_w(data);
}

void ls157_device::ab_w(u8 data)
{
	m_a = data >> 4;
	m_b = data & 0xf;
	update_output();
}


//-------------------------------------------------
//  ba_w -- write high nibble to B1-B4 and write
//  low nibble to A1-A4
//-------------------------------------------------

WRITE8_MEMBER(ls157_device::ba_w)
{
	ba_w(data);
}

void ls157_device::ba_w(u8 data)
{
	m_b = data >> 4;
	m_a = data & 0xf;
	update_output();
}


//-------------------------------------------------
//  interleave_w -- write even-numbered bits to
//  A1-A4 and write odd-numbered bits to B1-B4
//-------------------------------------------------

WRITE8_MEMBER(ls157_device::interleave_w)
{
	interleave_w(data);
}

void ls157_device::interleave_w(u8 data)
{
	m_b = ((data >> 4) & 8)
		| ((data >> 3) & 4)
		| ((data >> 2) & 2)
		| ((data >> 1) & 1);
	m_a = ((data >> 3) & 8)
		| ((data >> 2) & 4)
		| ((data >> 1) & 2)
		| ((data >> 0) & 1);
	update_output();
}


//**************************************************************************
//  CONTROL LINE INPUTS
//**************************************************************************

//-------------------------------------------------
//  select_w -- set select input
//-------------------------------------------------

WRITE_LINE_MEMBER(ls157_device::select_w)
{
	if (m_select != bool(state))
	{
		m_select = bool(state);
		update_output();
	}
}


//-------------------------------------------------
//  strobe_w -- set strobe input (active low)
//-------------------------------------------------

WRITE_LINE_MEMBER(ls157_device::strobe_w)
{
	if (m_strobe != bool(state))
	{
		m_strobe = bool(state);

		// Clear output when strobe goes high
		if (m_strobe)
			m_out_cb(0);
		else
			m_out_cb(m_select ? m_b : m_a);
	}
}


//-------------------------------------------------
//  update_output -- update output lines Y1-Y4
//  unless strobe is high
//-------------------------------------------------

void ls157_device::update_output()
{
	// S high, strobe low: Y1-Y4 = B1-B4
	// S low, strobe low:  Y1-Y4 = A1-A4
	if (!m_strobe)
		m_out_cb(m_select ? m_b : m_a);
}


//**************************************************************************
//  74HCT157 DEVICE
//**************************************************************************

DEFINE_DEVICE_TYPE(HCT157, hct157_device, "74hct157", "74HCT157 Quad 2-to-1 Multiplexer")

hct157_device::hct157_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ls157_device(mconfig, HCT157, tag, owner, clock)
{
}
