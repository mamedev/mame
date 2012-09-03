/****************************************************************************
 *
 * geebee.c
 *
 * sound driver
 * juergen buchmueller <pullmoll@t-online.de>, jan 2000
 *
 ****************************************************************************/

#include "emu.h"
#include "includes/warpwarp.h"


typedef struct _geebee_sound_state geebee_sound_state;
struct _geebee_sound_state
{
	emu_timer *m_volume_timer;
	UINT16 *m_decay;
	sound_stream *m_channel;
	int m_sound_latch;
	int m_sound_signal;
	int m_volume;
	int m_noise;
	int m_vcount;
};

INLINE geebee_sound_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == GEEBEE);

	return (geebee_sound_state *)downcast<geebee_sound_device *>(device)->token();
}

static TIMER_CALLBACK( volume_decay )
{
	geebee_sound_state *state = (geebee_sound_state *)ptr;
	if( --state->m_volume < 0 )
		state->m_volume = 0;
}

WRITE8_DEVICE_HANDLER( geebee_sound_w )
{
	geebee_sound_state *state = get_safe_token(device);

	state->m_channel->update();
	state->m_sound_latch = data;
	state->m_volume = 0x7fff; /* set volume */
	state->m_noise = 0x0000;  /* reset noise shifter */
	/* faster decay enabled? */
	if( state->m_sound_latch & 8 )
	{
		/*
         * R24 is 10k, Rb is 0, C57 is 1uF
         * charge time t1 = 0.693 * (R24 + Rb) * C57 -> 0.22176s
         * discharge time t2 = 0.693 * (Rb) * C57 -> 0
         * Then C33 is only charged via D6 (1N914), not discharged!
         * Decay:
         * discharge C33 (1uF) through R50 (22k) -> 0.14058s
         */
		attotime period = attotime::from_hz(32768) * 14058 / 100000;
		state->m_volume_timer->adjust(period, 0, period);
	}
	else
	{
		/*
         * discharge only after R49 (100k) in the amplifier section,
         * so the volume shouldn't very fast and only when the signal
         * is gated through 6N (4066).
         * I can only guess here that the decay should be slower,
         * maybe half as fast?
         */
		attotime period = attotime::from_hz(32768) * 29060 / 100000;
		state->m_volume_timer->adjust(period, 0, period);
    }
}

static STREAM_UPDATE( geebee_sound_update )
{
	geebee_sound_state *state = get_safe_token(device);
	stream_sample_t *buffer = outputs[0];

	while (samples--)
	{
		*buffer++ = state->m_sound_signal;
		/* 1V = HSYNC = 18.432MHz / 3 / 2 / 384 = 8000Hz */
		{
			state->m_vcount++;
			/* noise clocked with raising edge of 2V */
			if ((state->m_vcount & 3) == 2)
			{
				/* bit0 = bit0 ^ !bit10 */
				if ((state->m_noise & 1) == ((state->m_noise >> 10) & 1))
					state->m_noise = ((state->m_noise << 1) & 0xfffe) | 1;
				else
					state->m_noise = (state->m_noise << 1) & 0xfffe;
			}
			switch (state->m_sound_latch & 7)
			{
			case 0: /* 4V */
				state->m_sound_signal = (state->m_vcount & 0x04) ? state->m_decay[state->m_volume] : 0;
				break;
			case 1: /* 8V */
				state->m_sound_signal = (state->m_vcount & 0x08) ? state->m_decay[state->m_volume] : 0;
				break;
			case 2: /* 16V */
				state->m_sound_signal = (state->m_vcount & 0x10) ? state->m_decay[state->m_volume] : 0;
				break;
			case 3: /* 32V */
				state->m_sound_signal = (state->m_vcount & 0x20) ? state->m_decay[state->m_volume] : 0;
				break;
			case 4: /* TONE1 */
				state->m_sound_signal = !(state->m_vcount & 0x01) && !(state->m_vcount & 0x10) ? state->m_decay[state->m_volume] : 0;
				break;
			case 5: /* TONE2 */
				state->m_sound_signal = !(state->m_vcount & 0x02) && !(state->m_vcount & 0x20) ? state->m_decay[state->m_volume] : 0;
				break;
			case 6: /* TONE3 */
				state->m_sound_signal = !(state->m_vcount & 0x04) && !(state->m_vcount & 0x40) ? state->m_decay[state->m_volume] : 0;
				break;
			default: /* NOISE */
				/* QH of 74164 #4V */
				state->m_sound_signal = (state->m_noise & 0x8000) ? state->m_decay[state->m_volume] : 0;
			}
		}
	}
}

static DEVICE_START( geebee_sound )
{
	geebee_sound_state *state = get_safe_token(device);
	running_machine &machine = device->machine();
	int i;

	state->m_decay = auto_alloc_array(machine, UINT16, 32768);

	for( i = 0; i < 0x8000; i++ )
		state->m_decay[0x7fff-i] = (INT16) (0x7fff/exp(1.0*i/4096));

	/* 1V = HSYNC = 18.432MHz / 3 / 2 / 384 = 8000Hz */
	state->m_channel = device->machine().sound().stream_alloc(*device, 0, 1, 18432000 / 3 / 2 / 384, NULL, geebee_sound_update);
	state->m_vcount = 0;

	state->m_volume_timer = machine.scheduler().timer_alloc(FUNC(volume_decay), state);
}

DEVICE_GET_INFO( geebee_sound )
{
	switch (state)
	{
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(geebee_sound_state);			break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(geebee_sound);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Gee Bee Custom");				break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);						break;
	}
}


const device_type GEEBEE = &device_creator<geebee_sound_device>;

geebee_sound_device::geebee_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, GEEBEE, "Gee Bee Custom", tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(geebee_sound_state));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void geebee_sound_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void geebee_sound_device::device_start()
{
	DEVICE_START_NAME( geebee_sound )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void geebee_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


