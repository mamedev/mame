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
	return (_SID6581 *) downcast<legacy_device_base *>(device)->token();
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



/**************************************************************************
 * Generic get_info
 **************************************************************************/

DEVICE_GET_INFO( sid6581 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(_SID6581);						break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME( sid6581 );		break;
		case DEVINFO_FCT_STOP:							info->stop = NULL;								break;
		case DEVINFO_FCT_RESET:							info->reset = DEVICE_RESET_NAME( sid );			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "SID6581");						break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "SID");							break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright The MESS Team"); 	break;
	}
}


DEVICE_GET_INFO( sid8580 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME( sid8580 );		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "SID8580");						break;
		default:										DEVICE_GET_INFO_CALL(sid6581);						break;
	}
}


DEFINE_LEGACY_SOUND_DEVICE(SID6581, sid6581);
DEFINE_LEGACY_SOUND_DEVICE(SID8580, sid8580);
