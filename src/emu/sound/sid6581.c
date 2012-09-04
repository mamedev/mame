/***************************************************************************

    sid6581.c

    MAME/MESS interface for SID6581 and SID8580 chips

***************************************************************************/

#include "emu.h"
#include "sid6581.h"
#include "sid.h"



static _SID6581 *get_sid(device_t *device)
{
	assert(device != NULL);
	assert((device->type() == SID6581) || (device->type() == SID8580));
	return (_SID6581 *) downcast<sid6581_device *>(device)->token();
}



static STREAM_UPDATE( sid_update )
{
	_SID6581 *sid = (_SID6581 *) param;
	sidEmuFillBuffer(sid, outputs[0], samples);
}



static void sid_start(device_t *device, SIDTYPE sidtype)
{
	_SID6581 *sid = get_sid(device);
	const sid6581_interface *iface = (const sid6581_interface*) device->static_config();
	assert(iface);
	
	// resolve callbacks
	sid->in_potx_func.resolve(iface->in_potx_cb, *device);
	sid->in_poty_func.resolve(iface->in_poty_cb, *device);

	sid->device = device;
	sid->mixer_channel = device->machine().sound().stream_alloc(*device, 0, 1,  device->machine().sample_rate(), (void *) sid, sid_update);
	sid->PCMfreq = device->machine().sample_rate();
	sid->clock = device->clock();
	sid->type = sidtype;

	sid6581_init(sid);
	sidInitWaveformTables(sidtype);
}



static DEVICE_RESET( sid )
{
	_SID6581 *sid = get_sid(device);
	sidEmuReset(sid);
}



static DEVICE_START( sid6581 )
{
	sid_start(device, MOS6581);
}



static DEVICE_START( sid8580 )
{
	sid_start(device, MOS8580);
}



READ8_DEVICE_HANDLER  ( sid6581_r )
{
	return sid6581_port_r(device->machine(), get_sid(device), offset);
}


WRITE8_DEVICE_HANDLER ( sid6581_w )
{
	sid6581_port_w(get_sid(device), offset, data);
}

const device_type SID6581 = &device_creator<sid6581_device>;

sid6581_device::sid6581_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, SID6581, "SID6581", tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(_SID6581));
}
sid6581_device::sid6581_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, type, name, tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(_SID6581));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void sid6581_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sid6581_device::device_start()
{
	DEVICE_START_NAME( sid6581 )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void sid6581_device::device_reset()
{
	DEVICE_RESET_NAME( sid )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void sid6581_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


const device_type SID8580 = &device_creator<sid8580_device>;

sid8580_device::sid8580_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: sid6581_device(mconfig, SID8580, "SID8580", tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sid8580_device::device_start()
{
	DEVICE_START_NAME( sid8580 )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void sid8580_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


