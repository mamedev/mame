// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    PCD3311 DTMF/modem/musical tone generator emulation

**********************************************************************/

#include "emu.h"
#include "pcd3311.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(PCD3311, pcd3311_device, "pcd3311", "PCD3311")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  pcd3311_device - constructor
//-------------------------------------------------

pcd3311_device::pcd3311_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, PCD3311, tag, owner, clock),
	device_sound_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pcd3311_device::device_start()
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

void pcd3311_device::sound_stream_update(sound_stream &stream)
{
}
