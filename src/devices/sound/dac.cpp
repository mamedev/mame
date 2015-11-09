// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    dac.c

    DAC device emulator.

***************************************************************************/

#include "emu.h"
#include "dac.h"


// device type definition
const device_type DAC = &device_creator<dac_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  dac_device - constructor
//-------------------------------------------------

dac_device::dac_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, DAC, "DAC", tag, owner, clock, "dac", __FILE__),
		device_sound_interface(mconfig, *this),
		m_stream(NULL),
		m_output(0)
{
}


//-------------------------------------------------
//  write_unsigned8 - write an 8-bit value,
//  keeping the scaled result unsigned
//-------------------------------------------------

WRITE8_MEMBER( dac_device::write_unsigned8 )
{
	write_unsigned8(data);
}


//-------------------------------------------------
//  write_signed8 - write an 8-bit value,
//  keeping the scaled result signed
//-------------------------------------------------

WRITE8_MEMBER( dac_device::write_signed8 )
{
	write_signed8(data);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dac_device::device_start()
{
	// create the stream
	m_stream = stream_alloc(0, 1, DEFAULT_SAMPLE_RATE);

	// register for save states
	save_item(NAME(m_output));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void dac_device::device_reset()
{
	m_output = 0;
}


//-------------------------------------------------
//  sound_stream_update - handle update requests
//  for our sound stream
//-------------------------------------------------

void dac_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// just fill with current value
	for (int samp = 0; samp < samples; samp++)
		outputs[0][samp] = m_output;
}
