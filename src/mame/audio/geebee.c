/****************************************************************************
 *
 * geebee.c
 *
 * sound driver
 * juergen buchmueller <pullmoll@t-online.de>, jan 2000
 *
 ****************************************************************************/

#include "emu.h"
#include "streams.h"
#include "includes/warpwarp.h"

static emu_timer *volume_timer = NULL;
static UINT16 *decay = NULL;
static sound_stream *channel;
static int sound_latch = 0;
static int sound_signal = 0;
static int volume = 0;
static int noise = 0;
static int vcount = 0;

static TIMER_CALLBACK( volume_decay )
{
	if( --volume < 0 )
		volume = 0;
}

WRITE8_HANDLER( geebee_sound_w )
{
	stream_update(channel);
	sound_latch = data;
	volume = 0x7fff; /* set volume */
	noise = 0x0000;  /* reset noise shifter */
	/* faster decay enabled? */
	if( sound_latch & 8 )
	{
		/*
         * R24 is 10k, Rb is 0, C57 is 1uF
         * charge time t1 = 0.693 * (R24 + Rb) * C57 -> 0.22176s
         * discharge time t2 = 0.693 * (Rb) * C57 -> 0
         * Then C33 is only charged via D6 (1N914), not discharged!
         * Decay:
         * discharge C33 (1uF) through R50 (22k) -> 0.14058s
         */
        attotime period = attotime_div(attotime_mul(ATTOTIME_IN_HZ(32768), 14058), 100000);
		timer_adjust_periodic(volume_timer, period, 0, period);
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
        attotime period = attotime_div(attotime_mul(ATTOTIME_IN_HZ(32768), 29060), 100000);
		timer_adjust_periodic(volume_timer, period, 0, period);
    }
}

static STREAM_UPDATE( geebee_sound_update )
{
    stream_sample_t *buffer = outputs[0];

    while (samples--)
    {
		*buffer++ = sound_signal;
		/* 1V = HSYNC = 18.432MHz / 3 / 2 / 384 = 8000Hz */
        {
            vcount++;
			/* noise clocked with raising edge of 2V */
			if ((vcount & 3) == 2)
			{
				/* bit0 = bit0 ^ !bit10 */
				if ((noise & 1) == ((noise >> 10) & 1))
					noise = ((noise << 1) & 0xfffe) | 1;
				else
					noise = (noise << 1) & 0xfffe;
			}
            switch (sound_latch & 7)
            {
            case 0: /* 4V */
				sound_signal = (vcount & 0x04) ? decay[volume] : 0;
                break;
            case 1: /* 8V */
				sound_signal = (vcount & 0x08) ? decay[volume] : 0;
                break;
            case 2: /* 16V */
				sound_signal = (vcount & 0x10) ? decay[volume] : 0;
                break;
            case 3: /* 32V */
				sound_signal = (vcount & 0x20) ? decay[volume] : 0;
                break;
            case 4: /* TONE1 */
				sound_signal = !(vcount & 0x01) && !(vcount & 0x10) ? decay[volume] : 0;
                break;
            case 5: /* TONE2 */
				sound_signal = !(vcount & 0x02) && !(vcount & 0x20) ? decay[volume] : 0;
                break;
            case 6: /* TONE3 */
				sound_signal = !(vcount & 0x04) && !(vcount & 0x40) ? decay[volume] : 0;
                break;
			default: /* NOISE */
				/* QH of 74164 #4V */
                sound_signal = (noise & 0x8000) ? decay[volume] : 0;
            }
        }
    }
}

static DEVICE_START( geebee_sound )
{
	running_machine *machine = device->machine;
	int i;

	decay = auto_alloc_array(machine, UINT16, 32768);

    for( i = 0; i < 0x8000; i++ )
		decay[0x7fff-i] = (INT16) (0x7fff/exp(1.0*i/4096));

	/* 1V = HSYNC = 18.432MHz / 3 / 2 / 384 = 8000Hz */
	channel = stream_create(device, 0, 1, 18432000 / 3 / 2 / 384, NULL, geebee_sound_update);
	vcount = 0;

	volume_timer = timer_alloc(machine, volume_decay, NULL);
}

DEVICE_GET_INFO( geebee_sound )
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(geebee_sound);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Gee Bee Custom");				break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);						break;
	}
}


DEFINE_LEGACY_SOUND_DEVICE(GEEBEE, geebee_sound);
