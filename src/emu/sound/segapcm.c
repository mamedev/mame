/*********************************************************/
/*    SEGA 16ch 8bit PCM                                 */
/*********************************************************/

#include "sndintrf.h"
#include "streams.h"
#include "segapcm.h"

struct segapcm
{
	UINT8  *ram;
	UINT8 low[16];
	const UINT8 *rom;
	int bankshift;
	int bankmask;
	sound_stream * stream;
};

static void SEGAPCM_update(void *param, stream_sample_t **inputs, stream_sample_t **buffer, int length)
{
	struct segapcm *spcm = param;
	int ch;

	/* clear the buffers */
	memset(buffer[0], 0, length*sizeof(*buffer[0]));
	memset(buffer[1], 0, length*sizeof(*buffer[1]));

	/* loop over channels */
	for (ch = 0; ch < 16; ch++)

		/* only process active channels */
		if (!(spcm->ram[0x86+8*ch] & 1))
		{
			UINT8 *base = spcm->ram+8*ch;
			UINT8 flags = base[0x86];
			const UINT8 *rom = spcm->rom + ((flags & spcm->bankmask) << spcm->bankshift);
			UINT32 addr = (base[5] << 16) | (base[4] << 8) | spcm->low[ch];
			UINT16 loop = (base[0x85] << 8) | base[0x84];
			UINT8 end = base[6] + 1;
			UINT8 delta = base[7];
			UINT8 voll = base[2];
			UINT8 volr = base[3];
			int i;

			/* loop over samples on this channel */
			for (i = 0; i < length; i++)
			{
				INT8 v = 0;

				/* handle looping if we've hit the end */
				if ((addr >> 16) == end)
				{
					if (!(flags & 2))
						addr = loop << 8;
					else
					{
						flags |= 1;
						break;
					}
				}

				/* fetch the sample */
				v = rom[addr >> 8] - 0x80;

				/* apply panning and advance */
				buffer[0][i] += v * voll;
				buffer[1][i] += v * volr;
				addr += delta;
			}

			/* store back the updated address and info */
			base[0x86] = flags;
			base[4] = addr >> 8;
			base[5] = addr >> 16;
			spcm->low[ch] = flags & 1 ? 0 : addr;
		}
}

static void *segapcm_start(int sndindex, int clock, const void *config)
{
	const struct SEGAPCMinterface *intf = config;
	int mask, rom_mask;
	struct segapcm *spcm;

	spcm = auto_malloc(sizeof(*spcm));
	memset(spcm, 0, sizeof(*spcm));

	spcm->rom = (const UINT8 *)memory_region(intf->region);
	spcm->ram = auto_malloc(0x800);

	memset(spcm->ram, 0xff, 0x800);

	spcm->bankshift = (UINT8)(intf->bank);
	mask = intf->bank >> 16;
	if(!mask)
		mask = BANK_MASK7>>16;

	for(rom_mask = 1; rom_mask < memory_region_length(intf->region); rom_mask *= 2);
	rom_mask--;

	spcm->bankmask = mask & (rom_mask >> spcm->bankshift);

	spcm->stream = stream_create(0, 2, clock / 128, spcm, SEGAPCM_update);

	state_save_register_item_array("segapcm", sndindex, spcm->low);
	state_save_register_item_pointer("segapcm", sndindex, spcm->ram, 0x800);

	return spcm;
}


WRITE8_HANDLER( SegaPCM_w )
{
	struct segapcm *spcm = sndti_token(SOUND_SEGAPCM, 0);
	stream_update(spcm->stream);
	spcm->ram[offset & 0x07ff] = data;
}

READ8_HANDLER( SegaPCM_r )
{
	struct segapcm *spcm = sndti_token(SOUND_SEGAPCM, 0);
	stream_update(spcm->stream);
	return spcm->ram[offset & 0x07ff];
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

static void segapcm_set_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* no parameters to set */
	}
}


void segapcm_get_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = segapcm_set_info;		break;
		case SNDINFO_PTR_START:							info->start = segapcm_start;			break;
		case SNDINFO_PTR_STOP:							/* Nothing */							break;
		case SNDINFO_PTR_RESET:							/* Nothing */							break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							info->s = "Sega PCM";					break;
		case SNDINFO_STR_CORE_FAMILY:					info->s = "Sega custom";				break;
		case SNDINFO_STR_CORE_VERSION:					info->s = "1.0";						break;
		case SNDINFO_STR_CORE_FILE:						info->s = __FILE__;						break;
		case SNDINFO_STR_CORE_CREDITS:					info->s = "Copyright (c) 2004, The MAME Team"; break;
	}
}
