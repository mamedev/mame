/***************************************************************************

    JPM IMPACT with Video hardware

****************************************************************************/

#include "driver.h"
#include "cpu/tms34010/tms34010.h"
#include "jpmimpct.h"


UINT16 *tms_vram;


/*************************************
 *
 *  Brooktree Bt477 RAMDAC
 *
 *************************************/

static struct
{
	UINT8 address;
	UINT8 addr_cnt;
	UINT8 pixmask;
	UINT8 command;
	rgb_t color;
} bt477;


/*
 *  0 0 0    Address register (RAM write mode)
 *  0 0 1    Color palette RAMs
 *  0 1 0    Pixel read mask register
 *  0 1 1    Address register (RAM read mode)
 *  1 0 0    Address register (overlay write mode)
 *  1 1 1    Address register (overlay read mode)
 *  1 0 1    Overlay register
 *  1 1 0    Command register
 */

WRITE16_HANDLER( bt477_w )
{
	UINT8 val = data & 0xff;

	switch (offset)
	{
		case 0x0:
		{
			bt477.address = val;
			bt477.addr_cnt = 0;
			break;
		}
		case 0x1:
		{
			UINT8 *addr_cnt = &bt477.addr_cnt;
			rgb_t *color = &bt477.color;

			color[*addr_cnt] = val;

			if (++*addr_cnt == 3)
			{
				palette_set_color(Machine, bt477.address, MAKE_RGB(color[0], color[1], color[2]));
				*addr_cnt = 0;

				/* Address register increments */
				bt477.address++;
			}
			break;
		}
		case 0x2:
		{
			bt477.pixmask = val;
			break;
		}
		case 0x6:
		{
			bt477.command = val;
			break;
		}
		default:
		{
			popmessage("Bt477: Unhandled write access (offset:%x, data:%x)", offset, val);
		}
	}
}

READ16_HANDLER( bt477_r )
{
	popmessage("Bt477: Unhandled read access (offset:%x)", offset);
	return 0;
}


/*************************************
 *
 *  VRAM shift register callbacks
 *
 *************************************/

void jpmimpct_to_shiftreg(UINT32 address, UINT16 *shiftreg)
{
	memcpy(shiftreg, &tms_vram[TOWORD(address)], 512 * sizeof(UINT16));
}

void jpmimpct_from_shiftreg(UINT32 address, UINT16 *shiftreg)
{
	memcpy(&tms_vram[TOWORD(address)], shiftreg, 512 * sizeof(UINT16));
}


/*************************************
 *
 *  Main video refresh
 *
 *************************************/

void jpmimpct_scanline_update(running_machine *machine, int screen, mame_bitmap *bitmap, int scanline, const tms34010_display_params *params)
{
	UINT16 *vram = &tms_vram[(params->rowaddr << 8) & 0x3ff00];
	UINT32 *dest = BITMAP_ADDR32(bitmap, scanline, 0);
	int coladdr = params->coladdr;
	int x;

	for (x = params->heblnk; x < params->hsblnk; x += 2)
	{
		UINT16 pixels = vram[coladdr++ & 0xff];
		dest[x + 0]	= machine->pens[pixels & 0xff];
		dest[x + 1] = machine->pens[pixels >> 8];
	}
}


/*************************************
 *
 *  Video emulation start
 *
 *************************************/

VIDEO_START( jpmimpct )
{
	memset(&bt477, 0, sizeof(bt477));

	state_save_register_global(bt477.address);
	state_save_register_global(bt477.addr_cnt);
	state_save_register_global(bt477.pixmask);
	state_save_register_global(bt477.command);
	state_save_register_global(bt477.color);

	video_start_generic_bitmapped(machine);
}
