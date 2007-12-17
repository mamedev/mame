#include "sndintrf.h"
#include "custom.h"


struct custom_info
{
	const struct CustomSound_interface *intf;
	void *		token;
};



static void *custom_start(int sndindex, int clock, const void *config)
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


static void custom_stop(void *token)
{
	struct custom_info *info = token;
	if (info->intf->stop)
		(*info->intf->stop)(info->token);
}


static void custom_reset(void *token)
{
	struct custom_info *info = token;
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

static void custom_set_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* no parameters to set */
	}
}


void custom_get_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = custom_set_info;		break;
		case SNDINFO_PTR_START:							info->start = custom_start;				break;
		case SNDINFO_PTR_STOP:							info->stop = custom_stop;				break;
		case SNDINFO_PTR_RESET:							info->reset = custom_reset;				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							info->s = "Custom";						break;
		case SNDINFO_STR_CORE_FAMILY:					info->s = "None";						break;
		case SNDINFO_STR_CORE_VERSION:					info->s = "1.0";						break;
		case SNDINFO_STR_CORE_FILE:						info->s = __FILE__;						break;
		case SNDINFO_STR_CORE_CREDITS:					info->s = "Copyright (c) 2004, The MAME Team"; break;
	}
}

