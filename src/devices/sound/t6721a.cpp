// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Toshiba T6721A C2MOS Voice Synthesizing LSI emulation

**********************************************************************/

#include "emu.h"
#include "t6721a.h"

//#define VERBOSE 1
#include "logmacro.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(T6721A, t6721a_device, "t6721a", "Toshiba T6721A")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  t6721a_device - constructor
//-------------------------------------------------

t6721a_device::t6721a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, T6721A, tag, owner, clock),
	device_sound_interface(mconfig, *this),
	m_write_eos(*this),
	m_write_phi2(*this),
	m_write_dtrd(*this),
	m_write_apd(*this),
	m_stream(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void t6721a_device::device_start()
{
	// create sound stream
	m_stream = stream_alloc(0, 1, machine().sample_rate());
}


//-------------------------------------------------
//  sound_stream_update - handle update requests for
//  our sound stream
//-------------------------------------------------

void t6721a_device::sound_stream_update(sound_stream &stream)
{
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

uint8_t t6721a_device::read()
{
	return 0;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

void t6721a_device::write(uint8_t data)
{
}


//-------------------------------------------------
//  di_w - data input write
//-------------------------------------------------

void t6721a_device::di_w(int state)
{
}


//-------------------------------------------------
//  eos_r - eos read
//-------------------------------------------------

int t6721a_device::eos_r()
{
	return 1;
}
