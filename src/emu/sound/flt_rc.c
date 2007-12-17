#include "sndintrf.h"
#include "streams.h"
#include "flt_rc.h"
#include <math.h>

struct filter_rc_info
{
	sound_stream *	stream;
	int				k;
	int				memory;
	int				type;
};

flt_rc_config flt_rc_ac_default = {FLT_RC_AC, 10000, 0, 0, CAP_U(1)};


static void filter_rc_update(void *param, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	stream_sample_t *src = inputs[0];
	stream_sample_t *dst = outputs[0];
	struct filter_rc_info *info = param;
	int memory = info->memory;

	switch (info->type)
	{
		case FLT_RC_LOWPASS:
			while (samples--)
			{
				memory += ((*src++ - memory) * info->k) / 0x10000;
				*dst++ = memory;
			}
			break;
		case FLT_RC_HIGHPASS:
		case FLT_RC_AC:
			while (samples--)
			{
				*dst++ = *src - memory;
				memory += ((*src++ - memory) * info->k) / 0x10000;
			}
			break;
	}
	info->memory = memory;
}

static void set_RC_info(struct filter_rc_info *info, int type, double R1, double R2, double R3, double C)
{
	double Req;

	info->type = type;

	switch (info->type)
	{
		case FLT_RC_LOWPASS:
			if (C == 0.0)
			{
				/* filter disabled */
				info->k = 0x10000;
				return;
			}
			Req = (R1 * (R2 + R3)) / (R1 + R2 + R3);
			break;
		case FLT_RC_HIGHPASS:
		case FLT_RC_AC:
			if (C == 0.0)
			{
				/* filter disabled */
				info->k = 0x0;
				info->memory = 0x0;
				return;
			}
			Req = R1;
			break;
		default:
			fatalerror("filter_rc_setRC: Wrong filter type %d\n", info->type);
	}

	/* Cut Frequency = 1/(2*Pi*Req*C) */
	/* k = (1-(EXP(-TIMEDELTA/RC)))    */
	info->k = 0x10000 - 0x10000 * (exp(-1 / (Req * C) / Machine->sample_rate));
}


static void *filter_rc_start(int sndindex, int clock, const void *config)
{
	struct filter_rc_info *info;
	const flt_rc_config *conf = config;

	info = auto_malloc(sizeof(*info));
	memset(info, 0, sizeof(*info));

	info->stream = stream_create(1, 1, Machine->sample_rate, info, filter_rc_update);
	if (conf)
		set_RC_info(info, conf->type, conf->R1, conf->R2, conf->R3, conf->C);
	else
		set_RC_info(info, FLT_RC_LOWPASS, 1, 1, 1, 0);

	return info;
}


void filter_rc_set_RC(int num, int type, double R1, double R2, double R3, double C)
{
	struct filter_rc_info *info = sndti_token(SOUND_FILTER_RC, num);

	if(!info)
		return;

	stream_update(info->stream);

	set_RC_info(info, type, R1, R2, R3, C);

}

/**************************************************************************
 * Generic get_info
 **************************************************************************/

static void filter_rc_set_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* no parameters to set */
	}
}


void filter_rc_get_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = filter_rc_set_info;	break;
		case SNDINFO_PTR_START:							info->start = filter_rc_start;			break;
		case SNDINFO_PTR_STOP:							/* Nothing */							break;
		case SNDINFO_PTR_RESET:							/* Nothing */							break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							info->s = "RC Filter";					break;
		case SNDINFO_STR_CORE_FAMILY:					info->s = "Filters";					break;
		case SNDINFO_STR_CORE_VERSION:					info->s = "1.0";						break;
		case SNDINFO_STR_CORE_FILE:						info->s = __FILE__;						break;
		case SNDINFO_STR_CORE_CREDITS:					info->s = "Copyright (c) 2004, The MAME Team"; break;
	}
}

