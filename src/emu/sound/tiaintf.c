// license:GPL-2.0+
// copyright-holders:Ron Fries,Dan Boris
#include "emu.h"
#include "tiaintf.h"
#include "tiasound.h"

// device type definition
const device_type TIA = &device_creator<tia_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  tia_device - constructor
//-------------------------------------------------

tia_device::tia_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, TIA, "TIA", tag, owner, clock, "tia_sound", __FILE__),
		device_sound_interface(mconfig, *this),
		m_channel(NULL),
		m_chip(NULL)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tia_device::device_start()
{
	m_channel = stream_alloc(0, 1, clock());
	m_chip = tia_sound_init(this, clock(), clock(), 16);
	assert_always(m_chip != NULL, "Error creating TIA chip");
}


//-------------------------------------------------
//  device_stop - device-specific stop
//-------------------------------------------------

void tia_device::device_stop()
{
	tia_sound_free(m_chip);
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void tia_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	tia_process(m_chip, outputs[0], samples);
}


WRITE8_MEMBER( tia_device::tia_sound_w )
{
	m_channel->update();
	tia_write(m_chip, offset, data);
}
