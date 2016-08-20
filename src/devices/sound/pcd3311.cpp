// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

	PCD3311 DTMF/modem/musical tone generator emulation

**********************************************************************/

#include "pcd3311.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type PCD3311 = &device_creator<pcd3311_t>;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  pcd3311_t - constructor
//-------------------------------------------------

pcd3311_t::pcd3311_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, PCD3311, "PCD3311", tag, owner, clock, "pcd3311", __FILE__),
	device_sound_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pcd3311_t::device_start()
{
	save_item(NAME(m_a0));
	save_item(NAME(m_mode));
	save_item(NAME(m_strobe));
	save_item(NAME(m_data));
}


//-------------------------------------------------
//  sound_stream_update - handle update requests for
//  our sound stream
//-------------------------------------------------

void pcd3311_t::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
}
