/*********************************************************/
/*    SEGA 16ch 8bit PCM                                 */
/*********************************************************/

#include "emu.h"
#include "streams.h"
#include "segapcm.h"

typedef struct _segapcm_state segapcm_state;
struct _segapcm_state
{
	UINT8  *ram;
	UINT8 low[16];
	const UINT8 *rom;
	int bankshift;
	int bankmask;
	int rgnmask;
	sound_stream * stream;
};

INLINE segapcm_state *get_safe_token(running_device *device)
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == SOUND);
	assert(sound_get_type(device) == SOUND_SEGAPCM);
	return (segapcm_state *)device->token;
}

static STREAM_UPDATE( SEGAPCM_update )
{
	segapcm_state *spcm = (segapcm_state *)param;
	int rgnmask = spcm->rgnmask;
	int ch;

	/* clear the buffers */
	memset(outputs[0], 0, samples*sizeof(*outputs[0]));
	memset(outputs[1], 0, samples*sizeof(*outputs[1]));

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
			for (i = 0; i < samples; i++)
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
				v = rom[(addr >> 8) & rgnmask] - 0x80;

				/* apply panning and advance */
				outputs[0][i] += v * voll;
				outputs[1][i] += v * volr;
				addr += delta;
			}

			/* store back the updated address and info */
			base[0x86] = flags;
			base[4] = addr >> 8;
			base[5] = addr >> 16;
			spcm->low[ch] = flags & 1 ? 0 : addr;
		}
}

static DEVICE_START( segapcm )
{
	const sega_pcm_interface *intf = (const sega_pcm_interface *)device->baseconfig().static_config;
	int mask, rom_mask, len;
	segapcm_state *spcm = get_safe_token(device);

	spcm->rom = *device->region;
	spcm->ram = auto_alloc_array(device->machine, UINT8, 0x800);

	memset(spcm->ram, 0xff, 0x800);

	spcm->bankshift = (UINT8)(intf->bank);
	mask = intf->bank >> 16;
	if(!mask)
		mask = BANK_MASK7>>16;

	len = device->region->bytes();
	spcm->rgnmask = len - 1;

	for(rom_mask = 1; rom_mask < len; rom_mask *= 2);

	rom_mask--;

	spcm->bankmask = mask & (rom_mask >> spcm->bankshift);

	spcm->stream = stream_create(device, 0, 2, device->clock / 128, spcm, SEGAPCM_update);

	state_save_register_device_item_array(device, 0, spcm->low);
	state_save_register_device_item_pointer(device, 0, spcm->ram, 0x800);
}


WRITE8_DEVICE_HANDLER( sega_pcm_w )
{
	segapcm_state *spcm = get_safe_token(device);
	stream_update(spcm->stream);
	spcm->ram[offset & 0x07ff] = data;
}

READ8_DEVICE_HANDLER( sega_pcm_r )
{
	segapcm_state *spcm = get_safe_token(device);
	stream_update(spcm->stream);
	return spcm->ram[offset & 0x07ff];
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

DEVICE_GET_INFO( segapcm )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(segapcm_state);				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME( segapcm );		break;
		case DEVINFO_FCT_STOP:							/* Nothing */									break;
		case DEVINFO_FCT_RESET:							/* Nothing */									break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Sega PCM");					break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "Sega custom");					break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}
