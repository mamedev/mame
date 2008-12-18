/***************************************************************************

    sid6581.c

    MAME/MESS interface for SID6581 and SID8580 chips

***************************************************************************/

#include "sndintrf.h"
#include "sid6581.h"
#include "sid.h"



static SID6581 *get_sid(int indx)
{
	sound_type type = sndnum_to_sndti(indx, NULL);
	assert((type == SOUND_SID6581) || (type == SOUND_SID8580));
	return (SID6581 *) sndti_token(type, indx);
}



static STREAM_UPDATE( sid_update )
{
	SID6581 *sid = (SID6581 *) param;
	sidEmuFillBuffer(sid, outputs[0], samples);
}



static void *sid_start(const device_config *device, int clock, SIDTYPE sidtype)
{
	SID6581 *sid;
	const sid6581_interface *iface = (const sid6581_interface*) device->static_config;

	sid = (SID6581 *) auto_malloc(sizeof(*sid));
	memset(sid, 0, sizeof(*sid));

	sid->device = device;
	sid->mixer_channel = stream_create (device, 0, 1,  device->machine->sample_rate, (void *) sid, sid_update);
	sid->PCMfreq = device->machine->sample_rate;
	sid->clock = clock;
	sid->ad_read = iface ? iface->ad_read : NULL;
	sid->type = sidtype;

	sid6581_init(sid);
	sidInitWaveformTables(sidtype);
	return sid;
}



static SND_RESET( sid )
{
	SID6581 *sid = device->token;
	sidEmuReset(sid);
}



static SND_START( sid6581 )
{
	return sid_start(device, clock, MOS6581);
}



static SND_START( sid8580 )
{
	return sid_start(device, clock, MOS8580);
}



READ8_HANDLER ( sid6581_0_port_r )
{
	return sid6581_port_r(get_sid(0), offset);
}

READ8_HANDLER ( sid6581_1_port_r )
{
	return sid6581_port_r(get_sid(1), offset);
}

WRITE8_HANDLER ( sid6581_0_port_w )
{
	sid6581_port_w(get_sid(0), offset, data);
}

WRITE8_HANDLER ( sid6581_1_port_w )
{
	sid6581_port_w(get_sid(1), offset, data);
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

static SND_SET_INFO( sid6581 )
{
	switch (state)
	{
		/* no parameters to set */
	}
}


SND_GET_INFO( sid6581 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = SND_SET_INFO_NAME( sid6581 );		break;
		case SNDINFO_PTR_START:							info->start = SND_START_NAME( sid6581 );			break;
		case SNDINFO_PTR_STOP:							info->stop = NULL;						break;
		case SNDINFO_PTR_RESET:							info->reset = SND_RESET_NAME( sid );				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							info->s = "SID6581";					break;
		case SNDINFO_STR_CORE_FAMILY:					info->s = "SID";						break;
		case SNDINFO_STR_CORE_VERSION:					info->s = "1.0";						break;
		case SNDINFO_STR_CORE_FILE:						info->s = __FILE__;						break;
		case SNDINFO_STR_CORE_CREDITS:					info->s = "Copyright The MESS Team"; break;
	}
}


SND_GET_INFO( sid8580 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_START:							info->start = SND_START_NAME( sid8580 );			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							info->s = "SID8580";					break;
		default:										SND_GET_INFO_CALL(sid6581);	break;
	}
}

