/***************************************************************************

    sid6581.c

    MAME/MESS interface for SID6581 and SID8580 chips

***************************************************************************/

#include "emu.h"
#include "sid6581.h"
#include "sid.h"



static SID6581 *get_sid(running_device *device)
{
	assert(device != NULL);
	assert((sound_get_type(device) == SOUND_SID6581) || (sound_get_type(device) == SOUND_SID8580));
	return (SID6581 *) device->token;
}



static STREAM_UPDATE( sid_update )
{
	SID6581 *sid = (SID6581 *) param;
	sidEmuFillBuffer(sid, outputs[0], samples);
}



static void sid_start(running_device *device, SIDTYPE sidtype)
{
	SID6581 *sid = get_sid(device);
	const sid6581_interface *iface = (const sid6581_interface*) device->baseconfig().static_config;

	sid->device = device;
	sid->mixer_channel = stream_create (device, 0, 1,  device->machine->sample_rate, (void *) sid, sid_update);
	sid->PCMfreq = device->machine->sample_rate;
	sid->clock = device->clock;
	sid->ad_read = iface ? iface->ad_read : NULL;
	sid->type = sidtype;

	sid6581_init(sid);
	sidInitWaveformTables(sidtype);
}



static DEVICE_RESET( sid )
{
	SID6581 *sid = get_sid(device);
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
	return sid6581_port_r(device->machine, get_sid(device), offset);
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
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(SID6581);						break;

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

