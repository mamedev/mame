// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Fairchild DM9368 7-Segment Decoder/Driver/Latch emulation

**********************************************************************/

#include "emu.h"
#include "dm9368.h"

//#define VERBOSE 1
#include "logmacro.h"


//**************************************************************************
//  DEVICE DEFINITION
//**************************************************************************

DEFINE_DEVICE_TYPE(DM9368, dm9368_device, "dm9368", "Fairchild DM9368 7-Segment Decoder")



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

const uint8_t dm9368_device::s_segment_data[16] =
{
	0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x67, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71
};



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  dm9368_device - constructor
//-------------------------------------------------

dm9368_device::dm9368_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, DM9368, tag, owner, clock),
	device_output_interface(mconfig, *this),
	m_write_rbo(*this),
	m_rbi(1),
	m_rbo(1)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dm9368_device::device_start()
{
	// resolve callbacks
	m_write_rbo.resolve_safe();

	// state saving
	save_item(NAME(m_rbi));
	save_item(NAME(m_rbo));
}


//-------------------------------------------------
//  a_w -
//-------------------------------------------------

void dm9368_device::a_w(uint8_t data)
{
	int const a = data & 0x0f;
	uint8_t value = 0;

	if (!m_rbi && !a)
	{
		LOG("DM9368 Blanked Rippling Zero\n");

		// blank rippling 0
		m_rbo = 0;
	}
	else
	{
		LOG("DM9368 Output Data: %u = %02x\n", a, s_segment_data[a]);

		value = s_segment_data[a];

		m_rbo = 1;
	}

	set_digit_value(value);

	m_write_rbo(m_rbo);
}
