/***************************************************************************

  Video Hardware for Championship V'ball by Paul Hampson
  Generally copied from China Gate by Paul Hampson
  "Mainly copied from video of Double Dragon (bootleg) & Double Dragon II"

***************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "includes/vball.h"

int vb_scrollx_hi=0;
int vb_scrollx_lo=0;
int vb_scrolly_hi=0;
int vb_scrollx[256];

UINT8 *vb_scrolly_lo;
UINT8 *vb_videoram;
UINT8 *vb_attribram;
int vball_gfxset=0;
static int vb_bgprombank;
static int vb_spprombank;

static tilemap_t *bg_tilemap;

/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILEMAP_MAPPER( background_scan )
{
	/* logical (col,row) -> memory offset */
	return (col & 0x1f) + ((row & 0x1f) << 5) + ((col & 0x20) << 5) + ((row & 0x20) <<6);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	UINT8 code = vb_videoram[tile_index];
	UINT8 attr = vb_attribram[tile_index];
	SET_TILE_INFO(
			0,
			code + ((attr & 0x1f) << 8) + (vball_gfxset<<8),
			(attr >> 5) & 0x7,
			0);
}


VIDEO_START( vb )
{
	bg_tilemap = tilemap_create(machine, get_bg_tile_info,background_scan, 8, 8,64,64);

	tilemap_set_scroll_rows(bg_tilemap,32);
	vball_gfxset=0;
	vb_bgprombank=0xff;
	vb_spprombank=0xff;
}

WRITE8_HANDLER( vb_videoram_w )
{
	vb_videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap,offset);
}

#ifdef UNUSED_FUNCTION
READ8_HANDLER( vb_attrib_r )
{
	return vb_attribram[offset];
}
#endif

WRITE8_HANDLER( vb_attrib_w )
{
	vb_attribram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap,offset);
}

void vb_bgprombank_w( running_machine *machine, int bank )
{
	int i;
	UINT8* color_prom;

	if (bank==vb_bgprombank) return;

	color_prom = memory_region(machine, "proms") + bank*0x80;
	for (i=0;i<128;i++, color_prom++) {
		palette_set_color_rgb(machine,i,pal4bit(color_prom[0] >> 0),pal4bit(color_prom[0] >> 4),
				       pal4bit(color_prom[0x800] >> 0));
	}
	vb_bgprombank=bank;
}

void vb_spprombank_w( running_machine *machine, int bank )
{

	int i;
	UINT8* color_prom;

	if (bank==vb_spprombank) return;

	color_prom = memory_region(machine, "proms")+0x400 + bank*0x80;
	for (i=128;i<256;i++,color_prom++)	{
		palette_set_color_rgb(machine,i,pal4bit(color_prom[0] >> 0),pal4bit(color_prom[0] >> 4),
				       pal4bit(color_prom[0x800] >> 0));
	}
	vb_spprombank=bank;
}

void vb_mark_all_dirty( void )
{
	tilemap_mark_all_tiles_dirty(bg_tilemap);
}

#define DRAW_SPRITE( order, sx, sy ) drawgfx_transpen( bitmap, \
					cliprect,gfx, \
					(which+order),color,flipx,flipy,sx,sy,0);

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	const gfx_element *gfx = machine->gfx[1];
	UINT8 *src = machine->generic.spriteram.u8;
	int i;

/*  240-Y    S|X|CLR|WCH WHICH    240-X
    xxxxxxxx x|x|xxx|xxx xxxxxxxx xxxxxxxx
*/
	for (i = 0;i < machine->generic.spriteram_size;i += 4)
	{
		int attr = src[i+1];
		int which = src[i+2]+((attr & 0x07)<<8);
		int sx = ((src[i+3] + 8) & 0xff) - 7;
		int sy = 240 - src[i];
		int size = (attr & 0x80) >> 7;
		int color = (attr & 0x38) >> 3;
		int flipx = ~attr & 0x40;
		int flipy = 0;
		int dy = -16;

		if (flip_screen_get(machine))
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
			dy = -dy;
		}

		switch (size)
		{
			case 0: /* normal */
			DRAW_SPRITE(0,sx,sy);
			break;

			case 1: /* double y */
			DRAW_SPRITE(0,sx,sy + dy);
			DRAW_SPRITE(1,sx,sy);
			break;
		}
	}
}

#undef DRAW_SPRITE

VIDEO_UPDATE( vb )
{
	int i;

	tilemap_set_scrolly(bg_tilemap,0,vb_scrolly_hi + *vb_scrolly_lo);

	/*To get linescrolling to work properly, we must ignore the 1st two scroll values, no idea why! -SJE */
	for (i = 2; i < 256; i++) {
		tilemap_set_scrollx(bg_tilemap,i,vb_scrollx[i-2]);
		//logerror("scrollx[%d] = %d\n",i,vb_scrollx[i]);
	}
	tilemap_draw(bitmap,cliprect,bg_tilemap,0,0);
	draw_sprites(screen->machine,bitmap,cliprect);
	return 0;
}
