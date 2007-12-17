/*

  Malzak

  Video functions

  SAA 5050 -- Character display
  S2636 (x2) -- Sprites, Sprite->Sprite collisions
  Playfield graphics generator
      (TODO: probably best to switch this to tilemaps one day, figure out banking)

*/


#include "driver.h"
#include "video/s2636.h"

static INT8 frame_count;

#define SAA5050_DBLHI	0x0001
#define SAA5050_SEPGR	0x0002
#define SAA5050_FLASH	0x0004
#define SAA5050_BOX		0x0008
#define SAA5050_GRAPH	0x0010
#define SAA5050_CONCEAL	0x0020
#define SAA5050_HOLDGR	0x0040

#define SAA5050_BLACK   0
#define SAA5050_WHITE   7

struct	{
	UINT16	saa5050_flags;
	UINT8	saa5050_forecol;
	UINT8	saa5050_backcol;
	UINT8	saa5050_prvcol;
	UINT8	saa5050_prvchr;
} saa5050_state;


UINT8* saa5050_vidram;  /* Video RAM for SAA 5050 */

static mame_bitmap* collision_bitmap;

int malzak_x;
int malzak_y;

static struct playfield
{
	//int x;
	//int y;
	int code;
} field[256];

VIDEO_START( malzak )
{
	collision_bitmap = auto_bitmap_alloc(machine->screen[0].width,machine->screen[0].height,BITMAP_FORMAT_INDEXED8);

	saa5050_vidram = auto_malloc(0x800);
	s2636_1_ram = auto_malloc(0x100);
	s2636_2_ram = auto_malloc(0x100);
}

VIDEO_UPDATE( malzak )
{
	int code, colour;
	int sx, sy;
	int x,y;

	fillbitmap(bitmap,0,0);

	// SAA 5050 - Teletext character generator
	for (sy = 24; sy >= 0; sy--)
	{
		/* Set start of line state */
		saa5050_state.saa5050_flags = 0;
		saa5050_state.saa5050_prvchr = 32;
		saa5050_state.saa5050_forecol = SAA5050_WHITE;
		saa5050_state.saa5050_prvcol = SAA5050_WHITE;
		saa5050_state.saa5050_backcol = SAA5050_BLACK;

		for (sx = 0; sx < 42; sx++)
		{
			int blank = 0;
			code = saa5050_vidram[sy * 64 + sx];
			if (code < 32)
			{
				switch (code) {
				case 0x00:
					blank = 1;  // code 0x00 should not display anything
					break;      // unless HOLDGR is set
				case 0x01: case 0x02: case 0x03: case 0x04:
				case 0x05: case 0x06: case 0x07:
					saa5050_state.saa5050_prvcol = saa5050_state.saa5050_forecol = code;
					saa5050_state.saa5050_flags &= ~(SAA5050_GRAPH | SAA5050_CONCEAL);
					break;
				case 0x11: case 0x12: case 0x13: case 0x14:
				case 0x15: case 0x16: case 0x17:
					saa5050_state.saa5050_prvcol = (saa5050_state.saa5050_forecol =
						(code & 0x07));
					saa5050_state.saa5050_flags &= ~SAA5050_CONCEAL;
					saa5050_state.saa5050_flags |= SAA5050_GRAPH;
					break;
				case 0x08:
					saa5050_state.saa5050_flags |= SAA5050_FLASH;
					break;
				case 0x09:
					saa5050_state.saa5050_flags &= ~SAA5050_FLASH;
					break;
				case 0x0a:
					saa5050_state.saa5050_flags |= SAA5050_BOX;
					break;
				case 0x0b:
					saa5050_state.saa5050_flags &= ~SAA5050_BOX;
					break;
				case 0x0c:
					saa5050_state.saa5050_flags &= ~SAA5050_DBLHI;
					break;
				case 0x0d:
					saa5050_state.saa5050_flags |= SAA5050_DBLHI;
					break;
				case 0x18:
					saa5050_state.saa5050_flags |= SAA5050_CONCEAL;
					break;
				case 0x19:
					saa5050_state.saa5050_flags |= SAA5050_SEPGR;
					break;
				case 0x1a:
					saa5050_state.saa5050_flags &= ~SAA5050_SEPGR;
					break;
				case 0x1c:
					saa5050_state.saa5050_backcol = SAA5050_BLACK;
					break;
				case 0x1d:
                  saa5050_state.saa5050_backcol = saa5050_state.saa5050_prvcol;
					break;
				case 0x1e:
					saa5050_state.saa5050_flags |= SAA5050_HOLDGR;
					break;
				case 0x1f:
					saa5050_state.saa5050_flags &= ~SAA5050_HOLDGR;
					break;
				}
				if (saa5050_state.saa5050_flags & SAA5050_HOLDGR)
	  				code = saa5050_state.saa5050_prvchr;
				else
					code = 32;
			}

			if (code & 0x80)
				colour = (saa5050_state.saa5050_forecol << 3) | saa5050_state.saa5050_backcol;
			else
				colour = saa5050_state.saa5050_forecol | (saa5050_state.saa5050_backcol << 3);

			if (saa5050_state.saa5050_flags & SAA5050_CONCEAL)
				code = 32;
			else if ((saa5050_state.saa5050_flags & SAA5050_FLASH) && (frame_count > 38))
				code = 32;
			else
			{
				saa5050_state.saa5050_prvchr = code;
				if ((saa5050_state.saa5050_flags & SAA5050_GRAPH) && (code & 0x20))
				{
					code += (code & 0x40) ? 64 : 96;
					if (saa5050_state.saa5050_flags & SAA5050_SEPGR)
						code += 64;
				}
			}

			if((blank == 0) || (saa5050_state.saa5050_flags & SAA5050_HOLDGR))
			{
				if (saa5050_state.saa5050_flags & SAA5050_DBLHI)
				{
					drawgfx (bitmap, machine->gfx[4], code, colour, 0, 0,
						sx * 6, sy * 10, &machine->screen[0].visarea, TRANSPARENCY_NONE, 0);
					drawgfx (bitmap, machine->gfx[5], code, colour, 0, 0,
						sx * 6, (sy + 1) * 10, &machine->screen[0].visarea, TRANSPARENCY_NONE, 0);
				}
				else
				{
					drawgfx (bitmap, machine->gfx[3], code, colour, 0, 0,
						sx * 6, sy * 10, &machine->screen[0].visarea, TRANSPARENCY_NONE, 0);
				}
			}
		}
		if (saa5050_state.saa5050_flags & SAA5050_DBLHI)
		{
			sy--;
			saa5050_state.saa5050_flags &= ~SAA5050_DBLHI;
		}
	}

	frame_count++;
	if(frame_count > 50)
		frame_count = 0;

	// playfield - not sure exactly how this works...
	for(x = 0;x < 16;x++)
		for(y = 0; y < 16;y++)
		{
			sx = ((x*16-48) - malzak_x);
			sy = ((y*16) - malzak_y);

			if(sx < -271)
				sx+=512;
			if(sx < -15)
				sx+=256;

			drawgfx(bitmap,machine->gfx[0],field[x*16 + y].code,7,0,0,
			sx, sy, &machine->screen[0].visarea, TRANSPARENCY_COLOR, 0);
		}

	// S2636 - Sprites / Collision detection (x2)

	s2636_x_offset = -16;
//  s2636_y_offset = -8;

	s2636_update_bitmap(machine,bitmap,s2636_1_ram,s2636_1_dirty,1,collision_bitmap);
	s2636_update_bitmap(machine,bitmap,s2636_2_ram,s2636_2_dirty,2,collision_bitmap);
	return 0;
}

WRITE8_HANDLER( playfield_w )
{
	int tile = ((malzak_x / 16) * 16) + (offset / 16);

//  field[tile].x = malzak_x / 16;
//  field[tile].y = malzak_y;
	field[tile].code = (data & 0x1f);
	logerror("GFX: 0x16%02x write 0x%02x\n",offset,data);
}
