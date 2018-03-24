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

const u8 dm9368_device::s_segment_data[16] =
{
	0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x67, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71
};



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

dm9368_device::dm9368_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, DM9368, tag, owner, clock),
	m_update_cb(*this),
	m_rbo_cb(*this),
	m_rbi(1),
	m_rbo(1)
{
}


void dm9368_device::a_w(u8 data)
{
	int const a(data & 0x0f);
	int const rbo((m_rbi || a) ? 1 : 0);
	u8 const value(rbo ? s_segment_data[a] : 0);

	if (!rbo)
		LOG("DM9368 Blanked Rippling Zero\n");
	else
		LOG("DM9368 Output Data: %u = %02x\n", a, value);

	m_update_cb(0, value, 0x7f);
	if (rbo != m_rbo)
		m_rbo_cb(m_rbo = rbo);
}


void dm9368_device::device_resolve_objects()
{
	// resolve callbacks
	m_update_cb.resolve_safe();
	m_rbo_cb.resolve_safe();
}


void dm9368_device::device_start()
{
	// state saving
	save_item(NAME(m_rbi));
	save_item(NAME(m_rbo));
}
