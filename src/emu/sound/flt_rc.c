#include "emu.h"
#include "flt_rc.h"

typedef struct _filter_rc_state filter_rc_state;
struct _filter_rc_state
{
	device_t *device;
	sound_stream *	stream;
	int				k;
	int				memory;
	int				type;
};

INLINE filter_rc_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == FILTER_RC);
	return (filter_rc_state *)downcast<filter_rc_device *>(device)->token();
}

const flt_rc_config flt_rc_ac_default = {FLT_RC_AC, 10000, 0, 0, CAP_U(1)};


static STREAM_UPDATE( filter_rc_update )
{
	stream_sample_t *src = inputs[0];
	stream_sample_t *dst = outputs[0];
	filter_rc_state *info = (filter_rc_state *)param;
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

static void set_RC_info(filter_rc_state *info, int type, double R1, double R2, double R3, double C)
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
	info->k = 0x10000 - 0x10000 * (exp(-1 / (Req * C) / info->device->machine().sample_rate()));
}


static DEVICE_START( filter_rc )
{
	filter_rc_state *info = get_safe_token(device);
	const flt_rc_config *conf = (const flt_rc_config *)device->static_config();

	info->device = device;
	info->stream = device->machine().sound().stream_alloc(*device, 1, 1, device->machine().sample_rate(), info, filter_rc_update);
	if (conf)
		set_RC_info(info, conf->type, conf->R1, conf->R2, conf->R3, conf->C);
	else
		set_RC_info(info, FLT_RC_LOWPASS, 1, 1, 1, 0);
}


void filter_rc_set_RC(device_t *device, int type, double R1, double R2, double R3, double C)
{
	filter_rc_state *info = get_safe_token(device);

	info->stream->update();

	set_RC_info(info, type, R1, R2, R3, C);

}

const device_type FILTER_RC = &device_creator<filter_rc_device>;

filter_rc_device::filter_rc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, FILTER_RC, "RC Filter", tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(filter_rc_state));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void filter_rc_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void filter_rc_device::device_start()
{
	DEVICE_START_NAME( filter_rc )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void filter_rc_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


