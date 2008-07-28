/***************************************************************************

    Capcom CPS-3 Sound Hardware

***************************************************************************/
#include "driver.h"
#include "deprecat.h"
#include "streams.h"
#include "includes/cps3.h"

#define CPS3_VOICES		16

static sound_stream *cps3_stream;

typedef struct _cps3_voice_
{
	UINT32 regs[8];
	UINT32 pos;
	UINT16 frac;
} cps3_voice;

static struct
{
	cps3_voice voice[CPS3_VOICES];
	UINT16     key;
} chip;

static void cps3_stream_update(void *param, stream_sample_t **inputs, stream_sample_t **buffer, int length)
{
	int i;
	INT8 *base = (INT8*)memory_region(Machine, RGNCLASS_USER, "user5");

	/* Clear the buffers */
	memset(buffer[0], 0, length*sizeof(*buffer[0]));
	memset(buffer[1], 0, length*sizeof(*buffer[1]));

	for (i = 0; i < CPS3_VOICES; i ++)
	{
		if (chip.key & (1 << i))
		{
			int j;

			/* TODO */
			#define SWAP(a) ((a >> 16) | ((a & 0xffff) << 16))

			cps3_voice *vptr = &chip.voice[i];

			UINT32 start = vptr->regs[1];
			UINT32 end   = vptr->regs[5];
			UINT32 loop  = (vptr->regs[3] & 0xffff) + ((vptr->regs[4] & 0xffff) << 16);
			UINT32 step  = (vptr->regs[3] >> 16);

			INT16 vol_l = (vptr->regs[7] & 0xffff);
			INT16 vol_r = ((vptr->regs[7] >> 16) & 0xffff);

			UINT32 pos = vptr->pos;
			UINT16 frac = vptr->frac;

			/* TODO */
			start = SWAP(start) - 0x400000;
			end = SWAP(end) - 0x400000;
			loop -= 0x400000;

			/* Go through the buffer and add voice contributions */
			for (j = 0; j < length; j ++)
			{
				INT32 sample;

				pos += (frac >> 12);
				frac &= 0xfff;


				if (start + pos >= end)
				{
					if (vptr->regs[2])
					{
						pos = loop - start;
					}
					else
					{
						chip.key &= ~(1 << i);
						break;
					}
				}

				sample = base[BYTE4_XOR_LE(start + pos)];
				frac += step;

				buffer[0][j] += (sample * (vol_l >> 8));
				buffer[1][j] += (sample * (vol_r >> 8));
			}

			vptr->pos = pos;
			vptr->frac = frac;
		}
	}

}

void *cps3_sh_start(int clock, const struct CustomSound_interface *config)
{
	/* Allocate the stream */
	cps3_stream = stream_create(0, 2, clock / 384, NULL, cps3_stream_update);

	memset(&chip, 0, sizeof(chip));

	return auto_malloc(1);
}

WRITE32_HANDLER( cps3_sound_w )
{
	stream_update(cps3_stream);

	if (offset < 0x80)
	{
		COMBINE_DATA(&chip.voice[offset / 8].regs[offset & 7]);
	}
	else if (offset == 0x80)
	{
		int i;
		UINT16 key = data >> 16;

		for (i = 0; i < CPS3_VOICES; i++)
		{
			// Key off -> Key on
			if ((key & (1 << i)) && !(chip.key & (1 << i)))
			{
				chip.voice[i].frac = 0;
				chip.voice[i].pos = 0;
			}
		}
		chip.key = key;
	}
	else
	{
		printf("Sound [%x] %x\n", offset, data);
	}
}

READ32_HANDLER( cps3_sound_r )
{
	stream_update(cps3_stream);

	if (offset < 0x80)
	{
		return chip.voice[offset / 8].regs[offset & 7] & mem_mask;
	}
	else if (offset == 0x80)
	{
		return chip.key << 16;
	}
	else
	{
		printf("Unk sound read : %x\n", offset);
		return 0;
	}
}
