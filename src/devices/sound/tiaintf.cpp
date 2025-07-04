// license:GPL-2.0+
// copyright-holders:Ron Fries,Dan Boris
#include "emu.h"
#include "tiaintf.h"
#include "tiasound.h"

// device type definition
DEFINE_DEVICE_TYPE(TIA, tia_device, "tia_sound", "Atari TIA (Sound)")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  tia_device - constructor
//-------------------------------------------------

tia_device::tia_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TIA, tag, owner, clock),
		device_sound_interface(mconfig, *this),
		m_channel(nullptr),
		m_chip(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tia_device::device_start()
{
	m_channel = stream_alloc(0, 1, clock());
	m_chip = tia_sound_init(this, clock(), clock(), 16);
	if (!m_chip)
		throw emu_fatalerror("tia_device(%s): Error creating TIA chip", tag());
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

void tia_device::sound_stream_update(sound_stream &stream)
{
	tia_process(m_chip, stream);
}


void tia_device::tia_sound_w(offs_t offset, uint8_t data)
{
	m_channel->update();
	tia_write(m_chip, offset, data);
}
