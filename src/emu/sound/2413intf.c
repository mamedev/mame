// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
/****************************************************************

    MAME / MESS functions

****************************************************************/

#include "emu.h"
#include "ym2413.h"
#include "2413intf.h"


static void ym2413_update_request(void *param, int interval)
{
	ym2413_device *ym2413 = (ym2413_device *) param;
	ym2413->_ym2413_update_request();
}

void ym2413_device::_ym2413_update_request()
{
	m_stream->update();
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void ym2413_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	ym2413_update_one(m_chip, outputs, samples);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ym2413_device::device_start()
{
	int rate = clock()/72;

	/* emulator create */
	m_chip = ym2413_init(this, clock(), rate);
	assert_always(m_chip != NULL, "Error creating YM2413 chip");

	/* stream system initialize */
	m_stream = machine().sound().stream_alloc(*this,0,2,rate);

	ym2413_set_update_handler(m_chip, ym2413_update_request, this);
}

//-------------------------------------------------
//  device_stop - device-specific stop
//-------------------------------------------------

void ym2413_device::device_stop()
{
	ym2413_shutdown(m_chip);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ym2413_device::device_reset()
{
	ym2413_reset_chip(m_chip);
}


WRITE8_MEMBER( ym2413_device::write )
{
	ym2413_write(m_chip, offset & 1, data);
}

WRITE8_MEMBER( ym2413_device::register_port_w ) { write(space, 0, data); }
WRITE8_MEMBER( ym2413_device::data_port_w ) { write(space, 1, data); }

const device_type YM2413 = &device_creator<ym2413_device>;

ym2413_device::ym2413_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, YM2413, "YM2413", tag, owner, clock, "ym2413", __FILE__),
		device_sound_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void ym2413_device::device_config_complete()
{
}
