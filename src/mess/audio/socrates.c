/***************************************************************************

    audio/socrates.c

    This handles the two squarewaves (plus the one weird wave) channels
    on the V-tech Socrates system 27-0769 ASIC.

****************************************************************************/

#include "emu.h"
#include "socrates.h"

typedef struct
{
	sound_stream *stream;
	UINT8 freq[2]; /* channel 1,2 frequencies */
	UINT8 vol[2]; /* channel 1,2 volume */
	UINT8 enable[2]; /* channel 1,2 enable */
	UINT8 channel3; /* channel 3 weird register */
	UINT8 state[3]; /* output states for channels 1,2,3 */
	UINT8 accum[3]; /* accumulators for channels 1,2,3 */
	UINT16 DAC_output; /* output */
} SocratesASIC;


INLINE SocratesASIC *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == SOCRATES);
	return (SocratesASIC *)downcast<legacy_device_base *>(device)->token();
}

static const UINT8 volumeLUT[16] =
{
0, 61, 100, 132, 158, 183, 201, 218,
233, 242, 253, 255, 250, 240, 224, 211
}; // this table is actually quite weird on the real console.
// 0, 0.033, 0.055, 0.07175, 0.086, 0.1, 0.11, 0.119, 0.127, 0.132, 0.138, 0.139, 0.136, 0.131, 0.122, 0.115 are the voltage amplitudes for the steps on channel 2. the last four are particularly bizarre, probably caused by some sort of internal clipping.
static void socrates_snd_clock(SocratesASIC *chip) /* called once per clock */
{
	int channel;
	for (channel = 0; channel < 2; channel++)
	{
		if ((chip->accum[channel] == 0) && chip->enable[channel])
		{
		chip->state[channel] = (chip->state[channel]^0x1);
		chip->accum[channel] = chip->freq[channel];
		}
		else if (chip->enable[channel])
		{
		chip->accum[channel]--;
		}
		else
		{
		chip->accum[channel] = 0; // channel is disabled
		chip->state[channel] = 0;
		}
	}
	// handle channel 3 here
	chip->DAC_output = (chip->state[0]?(volumeLUT[chip->vol[0]]*9.4):0); // channel 1 is ~2.4 times as loud as channel 2
	chip->DAC_output += (chip->state[1]?(volumeLUT[chip->vol[1]]<<2):0);
	// add channel 3 to dac output here
}

/*************************************
 *
 *  Stream updater
 *
 *************************************/
static STREAM_UPDATE( socrates_snd_pcm_update )
{
	SocratesASIC *chip = (SocratesASIC *)param;
	int i;

	for (i = 0; i < samples; i++)
	{
		socrates_snd_clock(chip);
		outputs[0][i] = ((int)chip->DAC_output<<4);
	}
}



/*************************************
 *
 *  Sound handler start
 *
 *************************************/

static DEVICE_START( socrates_snd )
{
	SocratesASIC *chip = get_safe_token(device);
	chip->freq[0] = chip->freq[1] = 0xff; /* channel 1,2 frequency */
	chip->vol[0] = chip->vol[1] = 0x07; /* channel 1,2 volume */
	chip->enable[0] = chip->enable[1] = 0x01; /* channel 1,2 enable */
	chip->channel3 = 0x00; /* channel 3 weird register */
	chip->DAC_output = 0x00; /* output */
	chip->state[0] = chip->state[1] = chip->state[2] = 0;
	chip->accum[0] = chip->accum[1] = chip->accum[2] = 0xFF;
	chip->stream = device->machine().sound().stream_alloc(*device, 0, 1, device->clock() ? device->clock() : device->machine().sample_rate(), chip, socrates_snd_pcm_update);
}


void socrates_snd_reg0_w(device_t *device, int data)
{
	SocratesASIC *chip = get_safe_token(device);
	chip->stream->update();
	chip->freq[0] = data;
}

void socrates_snd_reg1_w(device_t *device, int data)
{
	SocratesASIC *chip = get_safe_token(device);
	chip->stream->update();
	chip->freq[1] = data;
}

void socrates_snd_reg2_w(device_t *device, int data)
{
	SocratesASIC *chip = get_safe_token(device);
	chip->stream->update();
	chip->vol[0] = data&0xF;
	chip->enable[0] = (data&0x10)>>4;
}

void socrates_snd_reg3_w(device_t *device, int data)
{
	SocratesASIC *chip = get_safe_token(device);
	chip->stream->update();
	chip->vol[1] = data&0xF;
	chip->enable[1] = (data&0x10)>>4;
}

void socrates_snd_reg4_w(device_t *device, int data)
{
	SocratesASIC *chip = get_safe_token(device);
	chip->stream->update();
	chip->channel3 = data;
}


/**************************************************************************
 * Generic get_info
 **************************************************************************/

DEVICE_GET_INFO( socrates_snd )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(SocratesASIC);				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME( socrates_snd );	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Socrates Sound");					break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "Socrates Sound");					break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.0");						break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);					break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright Jonathan Gevaryahu and The MESS Team"); break;
	}
}

DEFINE_LEGACY_SOUND_DEVICE(SOCRATES, socrates_snd);
