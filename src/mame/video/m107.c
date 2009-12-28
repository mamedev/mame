/****************************************************************************

    Irem M107 video hardware, Bryan McPhail, mish@tendril.co.uk

    Close to M92 hardware, but with 4 playfields, not 3.
    Twice as many colours, twice as many sprites.

*****************************************************************************

    Port:
        0x80: pf1 Y scroll
        0x82: pf1 X scroll
        0x84: pf2 Y scroll
        0x86: pf2 X scroll
        0x88: pf3 Y scroll
        0x8a: pf3 X scroll
        0x8c: pf4 Y scroll
        0x8e: pf4 X scroll

        0x90: pf1 control
        0x92: pf2 control
        0x94: pf3 control
        0x96: pf4 control

        0x98: Priority?
        0x9a:
        0x9c:
        0x9e: Raster interrupt value

    Playfield control:
        Bit  0x0f00:    Playfield location in VRAM (in steps of 0x1000)
        Bit  0x0080:    0 = Playfield enable, 1 = disable
        Bit  0x0002:    1 = Rowscroll enable, 0 = disable

*****************************************************************************/

#include "driver.h"
#include "includes/m107.h"

typedef struct _pf_layer_info pf_layer_info;
struct _pf_layer_info
{
	tilemap_t *		tmap;
	UINT16			vram_base;
	UINT16			control[4];
};

static pf_layer_info pf_layer[4];


static UINT16 m107_control[0x10];
static UINT16 *m107_spriteram;
UINT16 *m107_vram_data;
UINT16 m107_raster_irq_position;
UINT8 m107_spritesystem;


/*****************************************************************************/

static TILE_GET_INFO( get_pf_tile_info )
{
	pf_layer_info *layer = (pf_layer_info *)param;
	int tile, attrib;
	tile_index = 2 * tile_index + layer->vram_base;

	attrib = m107_vram_data[tile_index + 1];
	tile = m107_vram_data[tile_index] + ((attrib & 0x1000) << 4);

	SET_TILE_INFO(
			0,
			tile,
			attrib & 0x7f,
			TILE_FLIPYX(attrib >> 10));

	/* Priority 1 = tile appears above sprites */
	tileinfo->category = (attrib >> 9) & 1;
}

/*****************************************************************************/

WRITE16_HANDLER( m107_vram_w )
{
	int laynum;

	COMBINE_DATA(&m107_vram_data[offset]);
	for (laynum = 0; laynum < 4; laynum++)
		if ((offset & 0x6000) == pf_layer[laynum].vram_base)
			tilemap_mark_tile_dirty(pf_layer[laynum].tmap, (offset & 0x1fff) / 2);
}

/*****************************************************************************/

WRITE16_HANDLER( m107_control_w )
{
	UINT16 old = m107_control[offset];
	pf_layer_info *layer;

	COMBINE_DATA(&m107_control[offset]);

	switch (offset)
	{
		case 0x08: /* Playfield 1 (top layer) */
		case 0x09: /* Playfield 2 */
		case 0x0a: /* Playfield 3 */
		case 0x0b: /* Playfield 4 (bottom layer) */
			layer = &pf_layer[offset - 0x08];

			/* update VRAM base (bits 8-11) */
			layer->vram_base = ((m107_control[offset] >> 8) & 15) * 0x800;

			/* update enable (bit 7) */
			tilemap_set_enable(layer->tmap, (~m107_control[offset] >> 7) & 1);

			/* mark everything dirty of the VRAM base changes */
			if ((old ^ m107_control[offset]) & 0x0f00)
				tilemap_mark_all_tiles_dirty(layer->tmap);
			break;

		case 0x0f:
			m107_raster_irq_position = m107_control[offset] - 128;
			break;
	}
}

/*****************************************************************************/

VIDEO_START( m107 )
{
	int laynum;

	for (laynum = 0; laynum < 4; laynum++)
	{
		pf_layer_info *layer = &pf_layer[laynum];

		/* allocate a tilemaps per layer */
		layer->tmap = tilemap_create(machine, get_pf_tile_info, tilemap_scan_rows,  8,8, 64,64);

		/* set the user data to point to the layer */
		tilemap_set_user_data(layer->tmap, &pf_layer[laynum]);

		/* set scroll offsets */
		tilemap_set_scrolldx(layer->tmap, -3 + 2 * laynum, -3 + 2 * laynum);
		tilemap_set_scrolldy(layer->tmap, -128, -128);

		/* set pen 0 to transparent for all tilemaps except #4 */
		if (laynum != 3)
			tilemap_set_transparent_pen(layer->tmap, 0);
	}

	m107_spriteram = auto_alloc_array_clear(machine, UINT16, 0x1000/2);
}

/*****************************************************************************/

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int pri)
{
	int offs;
	UINT8 *rom = memory_region(machine, "user1");

	for (offs = 0x800-4;offs >= 0;offs -= 4)
	{
		int x,y,sprite,colour,fx,fy,y_multi,i,s_ptr;

		if (((m107_spriteram[offs+2]&0x80)==0x80) && pri==0) continue;
		if (((m107_spriteram[offs+2]&0x80)==0x00) && pri==1) continue;

		y=m107_spriteram[offs+0];
		x=m107_spriteram[offs+3];
		x&=0x1ff;
		y&=0x1ff;

		if (x==0 || y==0) continue; /* offscreen */

	    sprite=m107_spriteram[offs+1]&0x7fff;

		x = x - 16;
		y = 384 - 16 - y;

		colour=m107_spriteram[offs+2]&0x7f;
		fx=m107_spriteram[offs+2]&0x100;
		fy=m107_spriteram[offs+2]&0x200;
		y_multi=(m107_spriteram[offs+0]>>11)&0x3;

		if (m107_spritesystem == 0)
		{
			y_multi=1 << y_multi; /* 1, 2, 4 or 8 */

			s_ptr = 0;
			if (!fy) s_ptr+=y_multi-1;

			for (i=0; i<y_multi; i++)
			{
				drawgfx_transpen(bitmap,cliprect,machine->gfx[1],
						sprite + s_ptr,
						colour,
						fx,fy,
						x,y-i*16,0);
				if (fy) s_ptr++; else s_ptr--;
			}
		}
		else
		{
			int rom_offs = sprite*8;

			if (rom[rom_offs+1] || rom[rom_offs+3] || rom[rom_offs+5] || rom[rom_offs+7])
			{
				while (rom_offs < 0x40000)	/* safety check */
				{
					int xdisp = rom[rom_offs+6]+256*rom[rom_offs+7];
					int ydisp = rom[rom_offs+2]+256*rom[rom_offs+3];
					int ffx=fx^(rom[rom_offs+1]&1);
					int ffy=fy^(rom[rom_offs+1]&2);
					sprite=rom[rom_offs+4]+256*rom[rom_offs+5];
					y_multi=1<<((rom[rom_offs+3]>>1)&0x3);
					if (fx) xdisp = -xdisp;
					if (fy) ydisp = -ydisp - (16*y_multi-1);
					if (!ffy) sprite+=y_multi-1;
					for (i=0; i<y_multi; i++)
					{
						drawgfx_transpen(bitmap,cliprect,machine->gfx[1],
								sprite+(ffy?i:-i),
								colour,
								ffx,ffy,
								(x+xdisp)&0x1ff,(y-ydisp-16*i)&0x1ff,0);
					}

					if (rom[rom_offs+1]&0x80) break;	/* end of block */

					rom_offs += 8;
				}
			}
		}
	}
}

/*****************************************************************************/

static void m107_update_scroll_positions(void)
{
	int laynum;
	int i;

	/*  Playfield 4 rowscroll data is 0xde800 - 0xdebff???
        Playfield 3 rowscroll data is 0xdf800 - 0xdfbff
        Playfield 2 rowscroll data is 0xdf400 - 0xdf7ff
        Playfield 1 rowscroll data is 0xde800 - 0xdebff     ??
    */

    for (laynum = 0; laynum < 4; laynum++)
    {
    	pf_layer_info *layer = &pf_layer[laynum];

		if (m107_control[0x08 + laynum] & 0x02)
		{
			const UINT16 *scrolldata = m107_vram_data + (0xe800 + 0x400 * laynum) / 2;

			tilemap_set_scroll_rows(layer->tmap, 512);
			for (i = 0; i < 512; i++)
				tilemap_set_scrollx(layer->tmap, i, scrolldata[i] + m107_control[1 + 2 * laynum]);
		}
		else
		{
			tilemap_set_scroll_rows(layer->tmap, 1);
			tilemap_set_scrollx(layer->tmap, 0, m107_control[1 + 2 * laynum]);
		}

		tilemap_set_scrolly(layer->tmap, 0, m107_control[0 + 2 * laynum]);
	}
}

/*****************************************************************************/

static void m107_screenrefresh(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	if ((~m107_control[0x0b] >> 7) & 1)
	{
		tilemap_draw(bitmap, cliprect, pf_layer[3].tmap, 0, 0);
		tilemap_draw(bitmap, cliprect, pf_layer[3].tmap, 1, 0);
	}
	else
		bitmap_fill(bitmap, cliprect, 0);

	tilemap_draw(bitmap, cliprect, pf_layer[2].tmap, 0, 0);
	tilemap_draw(bitmap, cliprect, pf_layer[2].tmap, 1, 0);

	tilemap_draw(bitmap, cliprect, pf_layer[1].tmap, 0, 0);
	tilemap_draw(bitmap, cliprect, pf_layer[0].tmap, 0, 0);

	draw_sprites(machine, bitmap, cliprect, 0);
	tilemap_draw(bitmap, cliprect, pf_layer[1].tmap, 1, 0);
	tilemap_draw(bitmap, cliprect, pf_layer[0].tmap, 1, 0);

	draw_sprites(machine, bitmap, cliprect, 1);

	/* This hardware probably has more priority values - but I haven't found
        any used yet */
}

/*****************************************************************************/

WRITE16_HANDLER( m107_spritebuffer_w )
{
	if (ACCESSING_BITS_0_7) {
//      logerror("%04x: buffered spriteram\n",cpu_get_pc(space->cpu));
		memcpy(m107_spriteram,space->machine->generic.spriteram.u16,0x1000);
	}
}

/*****************************************************************************/

VIDEO_UPDATE( m107 )
{
	m107_update_scroll_positions();
	m107_screenrefresh(screen->machine, bitmap, cliprect);
	return 0;
}

