/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/
#include "driver.h"
#include "deprecat.h"


UINT8 *bigevglf_spriteram1;
UINT8 *bigevglf_spriteram2;


static UINT32 vidram_bank = 0;
static UINT32 plane_selected = 0;
static UINT32 plane_visible = 0;
static UINT8 *vidram;


static mame_bitmap *tmp_bitmap[4];

WRITE8_HANDLER(bigevglf_palette_w)
{
	int color;

	paletteram[offset] = data;
	color = paletteram[offset&0x3ff] | (paletteram[0x400+(offset&0x3ff)] << 8);
	palette_set_color_rgb(Machine, offset&0x3ff, pal4bit(color >> 4), pal4bit(color >> 0), pal4bit(color >> 8));
}

WRITE8_HANDLER( bigevglf_gfxcontrol_w )
{
/* bits used: 0,1,2,3
 0 and 2 select plane,
 1 and 3 select visible plane,
*/
	plane_selected=((data & 4)>>1) | (data&1);
	plane_visible =((data & 8)>>2) | ((data&2)>>1);
}

WRITE8_HANDLER( bigevglf_vidram_addr_w )
{
	vidram_bank = (data & 0xff) * 0x100;
}

WRITE8_HANDLER( bigevglf_vidram_w )
{
	UINT32 x,y,o;
	o = vidram_bank + offset;
	vidram[ o+0x10000*plane_selected ] = data;
	y = o >>8;
	x = (o & 255);
	*BITMAP_ADDR16(tmp_bitmap[plane_selected], y, x) = data;
}

READ8_HANDLER( bigevglf_vidram_r )
{
	return vidram[ 0x10000 * plane_selected + vidram_bank + offset];
}

VIDEO_START( bigevglf )
{
	tmp_bitmap[0] = auto_bitmap_alloc(machine->screen[0].width,machine->screen[0].height,machine->screen[0].format);
	tmp_bitmap[1] = auto_bitmap_alloc(machine->screen[0].width,machine->screen[0].height,machine->screen[0].format);
	tmp_bitmap[2] = auto_bitmap_alloc(machine->screen[0].width,machine->screen[0].height,machine->screen[0].format);
	tmp_bitmap[3] = auto_bitmap_alloc(machine->screen[0].width,machine->screen[0].height,machine->screen[0].format);
	vidram = auto_malloc(0x100*0x100 * 4);
}

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	int i,j;
	for (i = 0xc0-4; i >= 0; i-=4)
	{
		int code,sx,sy;
		code = bigevglf_spriteram2[i+1];
		sx = bigevglf_spriteram2[i+3];
		sy = 200-bigevglf_spriteram2[i];
		for(j=0;j<16;j++)
			drawgfx(bitmap,machine->gfx[0],
				bigevglf_spriteram1[(code<<4)+j]+((bigevglf_spriteram1[0x400+(code<<4)+j]&0xf)<<8),
				bigevglf_spriteram2[i+2] & 0xf,
				0,0,
				sx+((j&1)<<3),sy+((j>>1)<<3),
				cliprect,TRANSPARENCY_PEN,0);
	}
}

VIDEO_UPDATE( bigevglf )
{
	copybitmap(bitmap,tmp_bitmap[ plane_visible ],0,0,0,0,cliprect,TRANSPARENCY_NONE, 0);
	draw_sprites(machine,bitmap,cliprect);
	return 0;
}
