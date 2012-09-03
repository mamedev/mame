/***********************************************************

    Astrocade custom 'IO' chip sound chip driver
    Aaron Giles
    based on original work by Frank Palazzolo

************************************************************

    Register Map
    ============

    Register 0:
        D7..D0: Master oscillator frequency

    Register 1:
        D7..D0: Tone generator A frequency

    Register 2:
        D7..D0: Tone generator B frequency

    Register 3:
        D7..D0: Tone generator C frequency

    Register 4:
        D7..D6: Vibrato speed
        D5..D0: Vibrato depth

    Register 5:
            D5: Noise AM enable
            D4: Mux source (0=vibrato, 1=noise)
        D3..D0: Tone generator C volume

    Register 6:
        D7..D4: Tone generator B volume
        D3..D0: Tone generator A volume

    Register 7:
        D7..D0: Noise volume

***********************************************************/

#include "emu.h"
#include "astrocde.h"


typedef struct _astrocade_state astrocade_state;
struct _astrocade_state
{
	sound_stream *stream;		/* sound stream */

	UINT8		reg[8];			/* 8 control registers */

	UINT8		master_count;	/* current master oscillator count */
	UINT16		vibrato_clock;	/* current vibrato clock */

	UINT8		noise_clock;	/* current noise generator clock */
	UINT16		noise_state;	/* current noise LFSR state */

	UINT8		a_count;		/* current tone generator A count */
	UINT8		a_state;		/* current tone generator A state */

	UINT8		b_count;		/* current tone generator B count */
	UINT8		b_state;		/* current tone generator B state */

	UINT8		c_count;		/* current tone generator C count */
	UINT8		c_state;		/* current tone generator C state */

	UINT8		bitswap[256];	/* bitswap table */
};


INLINE astrocade_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == ASTROCADE);
	return (astrocade_state *)downcast<astrocade_device *>(device)->token();
}



/*************************************
 *
 *  Core sound update
 *
 *************************************/

static STREAM_UPDATE( astrocade_update )
{
	astrocade_state *chip = (astrocade_state *)param;
	stream_sample_t *dest = outputs[0];
	UINT16 noise_state;
	UINT8 master_count;
	UINT8 noise_clock;

	/* load some locals */
	master_count = chip->master_count;
	noise_clock = chip->noise_clock;
	noise_state = chip->noise_state;

	/* loop over samples */
	while (samples > 0)
	{
		stream_sample_t cursample = 0;
		int samples_this_time;
		int samp;

		/* compute the number of cycles until the next master oscillator reset */
		/* or until the next noise boundary */
		samples_this_time = MIN(samples, 256 - master_count);
		samples_this_time = MIN(samples_this_time, 64 - noise_clock);
		samples -= samples_this_time;

		/* sum the output of the tone generators */
		if (chip->a_state)
			cursample += chip->reg[6] & 0x0f;
		if (chip->b_state)
			cursample += chip->reg[6] >> 4;
		if (chip->c_state)
			cursample += chip->reg[5] & 0x0f;

		/* add in the noise if it is enabled, based on the top bit of the LFSR */
		if ((chip->reg[5] & 0x20) && (noise_state & 0x4000))
			cursample += chip->reg[7] >> 4;

		/* scale to max and output */
		cursample = cursample * 32767 / 60;
		for (samp = 0; samp < samples_this_time; samp++)
			*dest++ = cursample;

		/* clock the noise; a 2-bit counter clocks a 4-bit counter which clocks the LFSR */
		noise_clock += samples_this_time;
		if (noise_clock >= 64)
		{
			/* update the noise state; this is a 15-bit LFSR with feedback from */
			/* the XOR of the top two bits */
			noise_state = (noise_state << 1) | (~((noise_state >> 14) ^ (noise_state >> 13)) & 1);
			noise_clock -= 64;

			/* the same clock also controls the vibrato clock, which is a 13-bit counter */
			chip->vibrato_clock++;
		}

		/* clock the master oscillator; this is an 8-bit up counter */
		master_count += samples_this_time;
		if (master_count == 0)
		{
			/* reload based on mux value -- the value from the register is negative logic */
			master_count = ~chip->reg[0];

			/* mux value 0 means reload based on the vibrato control */
			if ((chip->reg[5] & 0x10) == 0)
			{
				/* vibrato speed (register 4 bits 6-7) selects one of the top 4 bits */
				/* of the 13-bit vibrato clock to use (0=highest freq, 3=lowest) */
				if (!((chip->vibrato_clock >> (chip->reg[4] >> 6)) & 0x0200))
				{
					/* if the bit is clear, we add the vibrato volume to the counter */
					master_count += chip->reg[4] & 0x3f;
				}
			}

			/* mux value 1 means reload based on the noise control */
			else
			{
				/* the top 8 bits of the noise LFSR are ANDed with the noise volume */
				/* register and added to the count */
				master_count += chip->bitswap[(noise_state >> 7) & 0xff] & chip->reg[7];
			}

			/* clock tone A */
			if (++chip->a_count == 0)
			{
				chip->a_state ^= 1;
				chip->a_count = ~chip->reg[1];
			}

			/* clock tone B */
			if (++chip->b_count == 0)
			{
				chip->b_state ^= 1;
				chip->b_count = ~chip->reg[2];
			}

			/* clock tone C */
			if (++chip->c_count == 0)
			{
				chip->c_state ^= 1;
				chip->c_count = ~chip->reg[3];
			}
		}
	}

	/* put back the locals */
	chip->master_count = master_count;
	chip->noise_clock = noise_clock;
	chip->noise_state = noise_state;
}



/*************************************
 *
 *  Chip reset
 *
 *************************************/

static DEVICE_RESET( astrocade )
{
	astrocade_state *chip = get_safe_token(device);

	memset(chip->reg, 0, sizeof(chip->reg));

	chip->master_count = 0;
	chip->vibrato_clock = 0;

	chip->noise_clock = 0;
	chip->noise_state = 0;

	chip->a_count = 0;
	chip->a_state = 0;

	chip->b_count = 0;
	chip->b_state = 0;

	chip->c_count = 0;
	chip->c_state = 0;
}


/*************************************
 *
 *  Save state registration
 *
 *************************************/

static void astrocade_state_save_register(astrocade_state *chip, device_t *device)
{
	device->save_item(NAME(chip->reg));

	device->save_item(NAME(chip->master_count));
	device->save_item(NAME(chip->vibrato_clock));

	device->save_item(NAME(chip->noise_clock));
	device->save_item(NAME(chip->noise_state));

	device->save_item(NAME(chip->a_count));
	device->save_item(NAME(chip->a_state));

	device->save_item(NAME(chip->b_count));
	device->save_item(NAME(chip->b_state));

	device->save_item(NAME(chip->c_count));
	device->save_item(NAME(chip->c_state));
}



/*************************************
 *
 *  Chip initialization
 *
 *************************************/

static DEVICE_START( astrocade )
{
	astrocade_state *chip = get_safe_token(device);
	int i;

	/* generate a bitswap table for the noise */
	for (i = 0; i < 256; i++)
		chip->bitswap[i] = BITSWAP8(i, 0,1,2,3,4,5,6,7);

	/* allocate a stream for output */
	chip->stream = device->machine().sound().stream_alloc(*device, 0, 1, device->clock(), chip, astrocade_update);

	/* reset state */
	DEVICE_RESET_CALL(astrocade);
	astrocade_state_save_register(chip, device);
}



/*************************************
 *
 *  Sound write accessors
 *
 *************************************/

WRITE8_DEVICE_HANDLER( astrocade_sound_w )
{
	astrocade_state *chip = get_safe_token(device);

	if ((offset & 8) != 0)
		offset = (offset >> 8) & 7;
	else
		offset &= 7;

	/* update */
	chip->stream->update();

	/* stash the new register value */
	chip->reg[offset & 7] = data;
}



/*************************************
 *
 *  Get/set info callbacks
 *
 *************************************/

DEVICE_GET_INFO( astrocade )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(astrocade_state);					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME( astrocade );		break;
		case DEVINFO_FCT_RESET:							info->reset = DEVICE_RESET_NAME( astrocade );		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Astrocade");						break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "Bally");							break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "2.0");								break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);							break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}


const device_type ASTROCADE = &device_creator<astrocade_device>;

astrocade_device::astrocade_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, ASTROCADE, "Astrocade", tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(astrocade_state));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void astrocade_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void astrocade_device::device_start()
{
	DEVICE_START_NAME( astrocade )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void astrocade_device::device_reset()
{
	DEVICE_RESET_NAME( astrocade )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void astrocade_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


