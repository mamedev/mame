/*********************************************************

Irem GA20 PCM Sound Chip

It's not currently known whether this chip is stereo.


Revisions:

04-15-2002 Acho A. Tang
- rewrote channel mixing
- added prelimenary volume and sample rate emulation

05-30-2002 Acho A. Tang
- applied hyperbolic gain control to volume and used
  a musical-note style progression in sample rate
  calculation(still very inaccurate)

02-18-2004 R. Belmont
- sample rate calculation reverse-engineered.
  Thanks to Fujix, Yasuhiro Ogawa, the Guru, and Tormod
  for real PCB samples that made this possible.

02-03-2007 R. Belmont
- Cleaned up faux x86 assembly.

*********************************************************/
#include <math.h>
#include "sndintrf.h"
#include "streams.h"
#include "iremga20.h"

#define MAX_VOL 256

struct IremGA20_channel_def
{
	UINT32 rate;
	UINT32 size;
	UINT32 start;
	UINT32 pos;
	UINT32 frac;
	UINT32 end;
	UINT32 volume;
	UINT32 pan;
	UINT32 effect;
	UINT32 play;
};

struct IremGA20_chip_def
{
	UINT8 *rom;
	INT32 rom_size;
	sound_stream * stream;
	UINT16 regs[0x40];
	struct IremGA20_channel_def channel[4];
};

static STREAM_UPDATE( IremGA20_update )
{
	struct IremGA20_chip_def *chip = param;
	UINT32 rate[4], pos[4], frac[4], end[4], vol[4], play[4];
	UINT8 *pSamples;
	stream_sample_t *outL, *outR;
	int i, sampleout;

	/* precache some values */
	for (i=0; i < 4; i++)
	{
		rate[i] = chip->channel[i].rate;
		pos[i] = chip->channel[i].pos;
		frac[i] = chip->channel[i].frac;
		end[i] = chip->channel[i].end - 0x20;
		vol[i] = chip->channel[i].volume;
		play[i] = chip->channel[i].play;
	}

	i = samples;
	pSamples = chip->rom;
	outL = outputs[0];
	outR = outputs[1];

	for (i = 0; i < samples; i++)
	{
		sampleout = 0;

		// update the 4 channels inline
		if (play[0])
		{
			sampleout += (pSamples[pos[0]] - 0x80) * vol[0];
			frac[0] += rate[0];
			pos[0] += frac[0] >> 24;
			frac[0] &= 0xffffff;
			play[0] = (pos[0] < end[0]);
		}
		if (play[1])
		{
			sampleout += (pSamples[pos[1]] - 0x80) * vol[1];
			frac[1] += rate[1];
			pos[1] += frac[1] >> 24;
			frac[1] &= 0xffffff;
			play[1] = (pos[1] < end[1]);
		}
		if (play[2])
		{
			sampleout += (pSamples[pos[2]] - 0x80) * vol[2];
			frac[2] += rate[2];
			pos[2] += frac[2] >> 24;
			frac[2] &= 0xffffff;
			play[2] = (pos[2] < end[2]);
		}
		if (play[3])
		{
			sampleout += (pSamples[pos[3]] - 0x80) * vol[3];
			frac[3] += rate[3];
			pos[3] += frac[3] >> 24;
			frac[3] &= 0xffffff;
			play[3] = (pos[3] < end[3]);
		}

		sampleout >>= 2;
		outL[i] = sampleout;
		outR[i] = sampleout;
	}

	/* update the regs now */
	for (i=0; i < 4; i++)
	{
		chip->channel[i].pos = pos[i];
		chip->channel[i].frac = frac[i];
		chip->channel[i].play = play[i];
	}
}

WRITE16_HANDLER( irem_ga20_w )
{
	struct IremGA20_chip_def *chip = sndti_token(SOUND_IREMGA20, 0);
	int channel;

	/* only low byte hooked up? */
	if (!ACCESSING_BITS_0_7)
		return;

	//logerror("GA20:  Offset %02x, data %04x\n",offset,data);

	stream_update(chip->stream);

	channel = offset >> 3;

	chip->regs[offset] = data;

	switch (offset & 0x7)
	{
		case 0: /* start address low */
			chip->channel[channel].start = ((chip->channel[channel].start)&0xff000) | (data<<4);
			break;

		case 1: /* start address high */
			chip->channel[channel].start = ((chip->channel[channel].start)&0x00ff0) | (data<<12);
			break;

		case 2: /* end address low */
			chip->channel[channel].end = ((chip->channel[channel].end)&0xff000) | (data<<4);
			break;

		case 3: /* end address high */
			chip->channel[channel].end = ((chip->channel[channel].end)&0x00ff0) | (data<<12);
			break;

		case 4:
			chip->channel[channel].rate = 0x1000000 / (256 - data);
			break;

		case 5: //AT: gain control
			chip->channel[channel].volume = (data * MAX_VOL) / (data + 10);
			break;

		case 6: //AT: this is always written 2(enabling both channels?)
			chip->channel[channel].play = data;
			chip->channel[channel].pos = chip->channel[channel].start;
			chip->channel[channel].frac = 0;
			break;
	}
}

READ16_HANDLER( irem_ga20_r )
{
	struct IremGA20_chip_def *chip = sndti_token(SOUND_IREMGA20, 0);
	int channel;

	stream_update(chip->stream);

	channel = offset >> 3;

	switch (offset & 0x7)
	{
		case 7:	// voice status.  bit 0 is 1 if active. (routine around 0xccc in rtypeleo)
			return chip->channel[channel].play ? 1 : 0;
			break;

		default:
			logerror("GA20: read unk. register %d, channel %d\n", offset & 0xf, channel);
			break;
	}

	return 0;
}

static void iremga20_reset(struct IremGA20_chip_def *chip)
{
	int i;

	for( i = 0; i < 4; i++ ) {
	chip->channel[i].rate = 0;
	chip->channel[i].size = 0;
	chip->channel[i].start = 0;
	chip->channel[i].pos = 0;
	chip->channel[i].frac = 0;
	chip->channel[i].end = 0;
	chip->channel[i].volume = 0;
	chip->channel[i].pan = 0;
	chip->channel[i].effect = 0;
	chip->channel[i].play = 0;
	}
}


static SND_RESET( iremga20 )
{
	iremga20_reset(device->token);
}

static SND_START( iremga20 )
{
	struct IremGA20_chip_def *chip;
	int i;

	chip = auto_malloc(sizeof(*chip));
	memset(chip, 0, sizeof(*chip));

	/* Initialize our chip structure */
	chip->rom = device->region;
	chip->rom_size = device->regionbytes;

	iremga20_reset(chip);

	for ( i = 0; i < 0x40; i++ )
		chip->regs[i] = 0;

	chip->stream = stream_create( device, 0, 2, clock/4, chip, IremGA20_update );

	state_save_register_device_item_array(device, sndindex, chip->regs);
	for (i = 0; i < 4; i++)
	{
		state_save_register_device_item(device, i, chip->channel[i].rate);
		state_save_register_device_item(device, i, chip->channel[i].size);
		state_save_register_device_item(device, i, chip->channel[i].start);
		state_save_register_device_item(device, i, chip->channel[i].pos);
		state_save_register_device_item(device, i, chip->channel[i].end);
		state_save_register_device_item(device, i, chip->channel[i].volume);
		state_save_register_device_item(device, i, chip->channel[i].pan);
		state_save_register_device_item(device, i, chip->channel[i].effect);
		state_save_register_device_item(device, i, chip->channel[i].play);
	}

	return chip;
}




/**************************************************************************
 * Generic get_info
 **************************************************************************/

static SND_SET_INFO( iremga20 )
{
	switch (state)
	{
		/* no parameters to set */
	}
}


SND_GET_INFO( iremga20 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = SND_SET_INFO_NAME( iremga20 );		break;
		case SNDINFO_PTR_START:							info->start = SND_START_NAME( iremga20 );			break;
		case SNDINFO_PTR_STOP:							/* nothing */							break;
		case SNDINFO_PTR_RESET:							info->reset = SND_RESET_NAME( iremga20 );			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							info->s = "Irem GA20";					break;
		case SNDINFO_STR_CORE_FAMILY:					info->s = "Irem custom";				break;
		case SNDINFO_STR_CORE_VERSION:					info->s = "1.0";						break;
		case SNDINFO_STR_CORE_FILE:						info->s = __FILE__;						break;
		case SNDINFO_STR_CORE_CREDITS:					info->s = "Copyright Nicola Salmoria and the MAME Team"; break;
	}
}

