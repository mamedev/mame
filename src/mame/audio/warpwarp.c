/****************************************************************************
 *
 * warpwarp.c
 *
 * sound driver
 * juergen buchmueller <pullmoll@t-online.de>, jan 2000
 *
 ****************************************************************************/

#include "emu.h"
#include "includes/warpwarp.h"

#define CLOCK_16H	(18432000/3/2/16)
#define CLOCK_1V    (18432000/3/2/384)

typedef struct _warpwarp_sound_state warpwarp_sound_state;
struct _warpwarp_sound_state
{
	INT16 *m_decay;
	sound_stream *m_channel;
	int m_sound_latch;
	int m_music1_latch;
	int m_music2_latch;
	int m_sound_signal;
	int m_sound_volume;
	emu_timer *m_sound_volume_timer;
	int m_music_signal;
	int m_music_volume;
	emu_timer *m_music_volume_timer;
	int m_noise;

	int m_vcarry;
	int m_vcount;
	int m_mcarry;
	int m_mcount;
};

INLINE warpwarp_sound_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == WARPWARP);

	return (warpwarp_sound_state *)downcast<warpwarp_sound_device *>(device)->token();
}

static TIMER_CALLBACK( sound_volume_decay )
{
	warpwarp_sound_state *state = (warpwarp_sound_state *)ptr;

	if( --state->m_sound_volume < 0 )
		state->m_sound_volume = 0;
}

WRITE8_DEVICE_HANDLER( warpwarp_sound_w )
{
	warpwarp_sound_state *state = get_safe_token(device);

	state->m_channel->update();
	state->m_sound_latch = data & 0x0f;
	state->m_sound_volume = 0x7fff; /* set sound_volume */
	state->m_noise = 0x0000;  /* reset noise shifter */

	/* faster decay enabled? */
	if( state->m_sound_latch & 8 )
	{
		/*
         * R85(?) is 10k, Rb is 0, C92 is 1uF
         * charge time t1 = 0.693 * (R24 + Rb) * C57 -> 0.22176s
         * discharge time t2 = 0.693 * (Rb) * C57 -> 0
         * C90(?) is only charged via D17 (1N914), no discharge!
         * Decay:
         * discharge C90(?) (1uF) through R13||R14 (22k||47k)
         * 0.639 * 15k * 1uF -> 0.9585s
         */
		attotime period = attotime::from_hz(32768) * 95850 / 100000;
		state->m_sound_volume_timer->adjust(period, 0, period);
	}
	else
	{
		/*
         * discharge only after R93 (100k) and through the 10k
         * potentiometerin the amplifier section.
         * 0.639 * 110k * 1uF -> 7.0290s
         * ...but this is not very realistic for the game sound :(
         * maybe there _is_ a discharge through the diode D17?
         */
		//attotime period = attotime::from_hz(32768) * 702900 / 100000;
		attotime period = attotime::from_hz(32768) * 191700 / 100000;
		state->m_sound_volume_timer->adjust(period, 0, period);
	}
}

WRITE8_DEVICE_HANDLER( warpwarp_music1_w )
{
	warpwarp_sound_state *state = get_safe_token(device);

	state->m_channel->update();
	state->m_music1_latch = data & 0x3f;
}

static TIMER_CALLBACK( music_volume_decay )
{
	warpwarp_sound_state *state = (warpwarp_sound_state *)ptr;
	if( --state->m_music_volume < 0 )
		state->m_music_volume = 0;
}

WRITE8_DEVICE_HANDLER( warpwarp_music2_w )
{
	warpwarp_sound_state *state = get_safe_token(device);
	state->m_channel->update();
	state->m_music2_latch = data & 0x3f;
	state->m_music_volume = 0x7fff;
	/* fast decay enabled? */
	if( state->m_music2_latch & 0x10 )
	{
		/*
         * Ra (R83?) is 10k, Rb is 0, C92 is 1uF
         * charge time t1 = 0.693 * (Ra + Rb) * C -> 0.22176s
         * discharge time is (nearly) zero, because Rb is zero
         * C95(?) is only charged via D17, not discharged!
         * Decay:
         * discharge C95(?) (10uF) through R13||R14 (22k||47k)
         * 0.639 * 15k * 10uF -> 9.585s
         * ...I'm sure this is off by one number of magnitude :/
         */
		attotime period = attotime::from_hz(32768) * 95850 / 100000;
		state->m_music_volume_timer->adjust(period, 0, period);
	}
	else
	{
		/*
         * discharge through R14 (47k),
         * discharge C95(?) (10uF) through R14 (47k)
         * 0.639 * 47k * 10uF -> 30.033s
         */
		//attotime period = attotime::from_hz(32768) * 3003300 / 100000;
		attotime period = attotime::from_hz(32768) * 300330 / 100000;
		state->m_music_volume_timer->adjust(period, 0, period);
	}

}

static STREAM_UPDATE( warpwarp_sound_update )
{
	warpwarp_sound_state *state = get_safe_token(device);
	stream_sample_t *buffer = outputs[0];

	while (samples--)
	{
		*buffer++ = (state->m_sound_signal + state->m_music_signal) / 2;

		/*
         * The music signal is selected at a rate of 2H (1.536MHz) from the
         * four bits of a 4 bit binary counter which is clocked with 16H,
         * which is 192kHz, and is divided by 4 times (64 - music1_latch).
         *  0 = 256 steps -> 750 Hz
         *  1 = 252 steps -> 761.9 Hz
         * ...
         * 32 = 128 steps -> 1500 Hz
         * ...
         * 48 =  64 steps -> 3000 Hz
         * ...
         * 63 =   4 steps -> 48 kHz
         */
		state->m_mcarry -= CLOCK_16H / (4 * (64 - state->m_music1_latch));
		while( state->m_mcarry < 0 )
		{
			state->m_mcarry += CLOCK_16H;
			state->m_mcount++;
			state->m_music_signal = (state->m_mcount & ~state->m_music2_latch & 15) ? state->m_decay[state->m_music_volume] : 0;
			/* override by noise gate? */
			if( (state->m_music2_latch & 32) && (state->m_noise & 0x8000) )
				state->m_music_signal = state->m_decay[state->m_music_volume];
		}

		/* clock 1V = 8kHz */
		state->m_vcarry -= CLOCK_1V;
		while (state->m_vcarry < 0)
		{
			state->m_vcarry += CLOCK_16H;
			state->m_vcount++;

			/* noise is clocked with raising edge of 2V */
			if ((state->m_vcount & 3) == 2)
			{
				/* bit0 = bit0 ^ !bit10 */
				if ((state->m_noise & 1) == ((state->m_noise >> 10) & 1))
					state->m_noise = (state->m_noise << 1) | 1;
				else
					state->m_noise = state->m_noise << 1;
			}

			switch (state->m_sound_latch & 7)
			{
			case 0: /* 4V */
				state->m_sound_signal = (state->m_vcount & 0x04) ? state->m_decay[state->m_sound_volume] : 0;
				break;
			case 1: /* 8V */
				state->m_sound_signal = (state->m_vcount & 0x08) ? state->m_decay[state->m_sound_volume] : 0;
				break;
			case 2: /* 16V */
				state->m_sound_signal = (state->m_vcount & 0x10) ? state->m_decay[state->m_sound_volume] : 0;
				break;
			case 3: /* 32V */
				state->m_sound_signal = (state->m_vcount & 0x20) ? state->m_decay[state->m_sound_volume] : 0;
				break;
			case 4: /* TONE1 */
				state->m_sound_signal = !(state->m_vcount & 0x01) && !(state->m_vcount & 0x10) ? state->m_decay[state->m_sound_volume] : 0;
				break;
			case 5: /* TONE2 */
				state->m_sound_signal = !(state->m_vcount & 0x02) && !(state->m_vcount & 0x20) ? state->m_decay[state->m_sound_volume] : 0;
				break;
			case 6: /* TONE3 */
				state->m_sound_signal = !(state->m_vcount & 0x04) && !(state->m_vcount & 0x40) ? state->m_decay[state->m_sound_volume] : 0;
				break;
			default: /* NOISE */
				/* QH of 74164 #4V */
				state->m_sound_signal = (state->m_noise & 0x8000) ? state->m_decay[state->m_sound_volume] : 0;
			}

		}
	}
}

static DEVICE_START( warpwarp_sound )
{
	warpwarp_sound_state *state = get_safe_token(device);
	running_machine &machine = device->machine();
	int i;

	state->m_decay = auto_alloc_array(machine, INT16, 32768);

	for( i = 0; i < 0x8000; i++ )
		state->m_decay[0x7fff-i] = (INT16) (0x7fff/exp(1.0*i/4096));

	state->m_channel = device->machine().sound().stream_alloc(*device, 0, 1, CLOCK_16H, NULL, warpwarp_sound_update);

	state->m_sound_volume_timer = machine.scheduler().timer_alloc(FUNC(sound_volume_decay), state);
	state->m_music_volume_timer = machine.scheduler().timer_alloc(FUNC(music_volume_decay), state);
}



const device_type WARPWARP = &device_creator<warpwarp_sound_device>;

warpwarp_sound_device::warpwarp_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, WARPWARP, "Warp Warp Custom", tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(warpwarp_sound_state));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void warpwarp_sound_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void warpwarp_sound_device::device_start()
{
	DEVICE_START_NAME( warpwarp_sound )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void warpwarp_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


