/**********************************************************************

    Toshiba T6721A C2MOS Voice Synthesizing LSI emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "t6721a.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

// device type definition
const device_type T6721A = &device_creator<t6721a_device>;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  t6721a_device - constructor
//-------------------------------------------------

t6721a_device::t6721a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, T6721A, "T6721A", tag, owner, clock, "t6721a", __FILE__),
		device_sound_interface(mconfig, *this),
		m_eos_handler(*this),
		m_dtrd_handler(*this),
		m_apd_handler(*this),
		m_stream(NULL)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void t6721a_device::device_start()
{
	// resolve callbacks
	m_eos_handler.resolve_safe();
	m_dtrd_handler.resolve_safe();
	m_apd_handler.resolve_safe();

	// create sound stream
	m_stream = machine().sound().stream_alloc(*this, 0, 1, machine().sample_rate());
}


//-------------------------------------------------
//  sound_stream_update - handle update requests for
//  our sound stream
//-------------------------------------------------

void t6721a_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

READ8_MEMBER( t6721a_device::read )
{
	return 0;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

WRITE8_MEMBER( t6721a_device::write )
{
}


//-------------------------------------------------
//  di_w - data input write
//-------------------------------------------------

WRITE_LINE_MEMBER( t6721a_device::di_w )
{
}
