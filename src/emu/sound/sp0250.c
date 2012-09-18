/*
   GI SP0250 digital LPC sound synthesizer

   By O. Galibert.

   Unknown:
   - Exact clock divider
   - Exact noise algorithm
   - Exact noise pitch (probably ok)
   - 7 bits output mapping
   - Whether the pitch starts counting from 0 or 1

   Unimplemented:
   - Direct Data test mode (pin 7)

   Sound quite reasonably already though.
*/

#include "emu.h"
#include "sp0250.h"

/*
standard external clock is 3.12MHz
the chip provides a 445.7kHz output clock, which is = 3.12MHz / 7
therefore I expect the clock divider to be a multiple of 7
Also there are 6 cascading filter stages so I expect the divider to be a multiple of 6.

The SP0250 manual states that the original speech is sampled at 10kHz, so the divider
should be 312, but 312 = 39*8 so it doesn't look right because a divider by 39 is unlikely.

7*6*8 = 336 gives a 9.286kHz sample rate and matches the samples from the Sega boards.
*/
#define CLOCK_DIVIDER (7*6*8)

struct sp0250_state
{
	INT16 amp;
	UINT8 pitch;
	UINT8 repeat;
	int pcount, rcount;
	int playing;
	UINT32 RNG;
	sound_stream * stream;
	int voiced;
	UINT8 fifo[15];
	int fifo_pos;

	device_t *device;
	void (*drq)(device_t *device, int state);

	struct
	{
		INT16 F, B;
		INT16 z1, z2;
	} filter[6];
};

INLINE sp0250_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == SP0250);
	return (sp0250_state *)downcast<sp0250_device *>(device)->token();
}


static UINT16 sp0250_ga(UINT8 v)
{
	return (v & 0x1f) << (v>>5);
}

static INT16 sp0250_gc(UINT8 v)
{
	// Internal ROM to the chip, cf. manual
	static const UINT16 coefs[128] =
	{
		  0,   9,  17,  25,  33,  41,  49,  57,  65,  73,  81,  89,  97, 105, 113, 121,
		129, 137, 145, 153, 161, 169, 177, 185, 193, 201, 203, 217, 225, 233, 241, 249,
		257, 265, 273, 281, 289, 297, 301, 305, 309, 313, 317, 321, 325, 329, 333, 337,
		341, 345, 349, 353, 357, 361, 365, 369, 373, 377, 381, 385, 389, 393, 397, 401,
		405, 409, 413, 417, 421, 425, 427, 429, 431, 433, 435, 437, 439, 441, 443, 445,
		447, 449, 451, 453, 455, 457, 459, 461, 463, 465, 467, 469, 471, 473, 475, 477,
		479, 481, 482, 483, 484, 485, 486, 487, 488, 489, 490, 491, 492, 493, 494, 495,
		496, 497, 498, 499, 500, 501, 502, 503, 504, 505, 506, 507, 508, 509, 510, 511
	};
	INT16 res = coefs[v & 0x7f];

	if (!(v & 0x80))
		res = -res;
	return res;
}

static void sp0250_load_values(sp0250_state *sp)
{
	int f;


	sp->filter[0].B = sp0250_gc(sp->fifo[ 0]);
	sp->filter[0].F = sp0250_gc(sp->fifo[ 1]);
	sp->amp         = sp0250_ga(sp->fifo[ 2]);
	sp->filter[1].B = sp0250_gc(sp->fifo[ 3]);
	sp->filter[1].F = sp0250_gc(sp->fifo[ 4]);
	sp->pitch       =           sp->fifo[ 5];
	sp->filter[2].B = sp0250_gc(sp->fifo[ 6]);
	sp->filter[2].F = sp0250_gc(sp->fifo[ 7]);
	sp->repeat      =           sp->fifo[ 8] & 0x3f;
	sp->voiced      =           sp->fifo[ 8] & 0x40;
	sp->filter[3].B = sp0250_gc(sp->fifo[ 9]);
	sp->filter[3].F = sp0250_gc(sp->fifo[10]);
	sp->filter[4].B = sp0250_gc(sp->fifo[11]);
	sp->filter[4].F = sp0250_gc(sp->fifo[12]);
	sp->filter[5].B = sp0250_gc(sp->fifo[13]);
	sp->filter[5].F = sp0250_gc(sp->fifo[14]);
	sp->fifo_pos = 0;
	if (sp->drq != NULL)
		sp->drq(sp->device, ASSERT_LINE);

	sp->pcount = 0;
	sp->rcount = 0;

	for (f = 0; f < 6; f++)
		sp->filter[f].z1 = sp->filter[f].z2 = 0;

	sp->playing = 1;
}

static TIMER_CALLBACK( sp0250_timer_tick )
{
	sp0250_state *sp = (sp0250_state *)ptr;
	sp->stream->update();
}

static STREAM_UPDATE( sp0250_update )
{
	sp0250_state *sp = (sp0250_state *)param;
	stream_sample_t *output = outputs[0];
	int i;
	for (i = 0; i < samples; i++)
	{
		if (sp->playing)
		{
			INT16 z0;
			int f;

			if (sp->voiced)
			{
				if(!sp->pcount)
					z0 = sp->amp;
				else
					z0 = 0;
			}
			else
			{
				// Borrowing the ay noise generation LFSR
				if(sp->RNG & 1)
				{
					z0 = sp->amp;
					sp->RNG ^= 0x24000;
				}
				else
					z0 = -sp->amp;

				sp->RNG >>= 1;
			}

			for (f = 0; f < 6; f++)
			{
				z0 += ((sp->filter[f].z1 * sp->filter[f].F) >> 8)
					+ ((sp->filter[f].z2 * sp->filter[f].B) >> 9);
				sp->filter[f].z2 = sp->filter[f].z1;
				sp->filter[f].z1 = z0;
			}

			// Physical resolution is only 7 bits, but heh

			// max amplitude is 0x0f80 so we have margin to push up the output
			output[i] = z0 << 3;

			sp->pcount++;
			if (sp->pcount >= sp->pitch)
			{
				sp->pcount = 0;
				sp->rcount++;
				if (sp->rcount >= sp->repeat)
					sp->playing = 0;
			}
		}
		else
			output[i] = 0;

		if (!sp->playing)
		{
			if(sp->fifo_pos == 15)
				sp0250_load_values(sp);
		}
	}
}


static DEVICE_START( sp0250 )
{
	const struct sp0250_interface *intf = (const struct sp0250_interface *)device->static_config();
	sp0250_state *sp = get_safe_token(device);

	sp->device = device;
	sp->RNG = 1;
	sp->drq = (intf != NULL) ? intf->drq_callback : NULL;
	if (sp->drq != NULL)
	{
		sp->drq(sp->device, ASSERT_LINE);
		device->machine().scheduler().timer_pulse(attotime::from_hz(device->clock()) * CLOCK_DIVIDER, FUNC(sp0250_timer_tick), 0, sp);
	}

	sp->stream = device->machine().sound().stream_alloc(*device, 0, 1, device->clock() / CLOCK_DIVIDER, sp, sp0250_update);
}


WRITE8_DEVICE_HANDLER( sp0250_w )
{
	sp0250_state *sp = get_safe_token(device);
	sp->stream->update();
	if (sp->fifo_pos != 15)
	{
		sp->fifo[sp->fifo_pos++] = data;
		if (sp->fifo_pos == 15 && sp->drq != NULL)
			sp->drq(sp->device, CLEAR_LINE);
	}
	else
		logerror("%s: overflow SP0250 FIFO\n", device->machine().describe_context());
}


UINT8 sp0250_drq_r(device_t *device)
{
	sp0250_state *sp = get_safe_token(device);
	sp->stream->update();
	return (sp->fifo_pos == 15) ? CLEAR_LINE : ASSERT_LINE;
}


const device_type SP0250 = &device_creator<sp0250_device>;

sp0250_device::sp0250_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, SP0250, "SP0250", tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_clear(sp0250_state);
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void sp0250_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sp0250_device::device_start()
{
	DEVICE_START_NAME( sp0250 )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void sp0250_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


