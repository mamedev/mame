#include "sndintrf.h"
#include "custom.h"


struct custom_info
{
	const custom_sound_interface *intf;
	void *		token;
};



static SND_START( custom )
{
	struct custom_info *info;

	info = auto_malloc(sizeof(*info));
	memset(info, 0, sizeof(*info));

	/* stash a pointer and call the start routine */
	info->intf = config;
	if (info->intf->start)
	{
		info->token = (*info->intf->start)(clock, config);
		if (!info->token)
			return NULL;
	}

	return info;
}


static SND_STOP( custom )
{
	struct custom_info *info = device->token;
	if (info->intf->stop)
		(*info->intf->stop)(info->token);
}


static SND_RESET( custom )
{
	struct custom_info *info = device->token;
	if (info->intf->reset)
		(*info->intf->reset)(info->token);
}


void *custom_get_token(int index)
{
	struct custom_info *token = sndti_token(SOUND_CUSTOM, index);
	return token->token;
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

static SND_SET_INFO( custom )
{
	switch (state)
	{
		/* no parameters to set */
	}
}


SND_GET_INFO( custom )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = SND_SET_INFO_NAME( custom );		break;
		case SNDINFO_PTR_START:							info->start = SND_START_NAME( custom );				break;
		case SNDINFO_PTR_STOP:							info->stop = SND_STOP_NAME( custom );				break;
		case SNDINFO_PTR_RESET:							info->reset = SND_RESET_NAME( custom );				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							info->s = "Custom";						break;
		case SNDINFO_STR_CORE_FAMILY:					info->s = "None";						break;
		case SNDINFO_STR_CORE_VERSION:					info->s = "1.0";						break;
		case SNDINFO_STR_CORE_FILE:						info->s = __FILE__;						break;
		case SNDINFO_STR_CORE_CREDITS:					info->s = "Copyright Nicola Salmoria and the MAME Team"; break;
	}
}

