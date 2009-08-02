/*****************************************************************************

    Irem M92 video hardware, Bryan McPhail, mish@tendril.co.uk

    Brief Overview:

        3 scrolling playfields, 512 by 512.
        Each playfield can enable rowscroll, change shape (to 1024 by 512),
        be enabled/disabled and change position in VRAM.

        Tiles can have several priority values:
            0 = standard
            1 = Top 8 pens appear over sprites (split tilemap)
            2 = Whole tile appears over sprites
            3 = ?  Seems to be the whole tile is over sprites (as 2).

        Sprites have 2 priority values:
            0 = standard
            1 = Sprite appears over all tiles, including high priority pf1

        Raster interrupts can be triggered at any line of the screen redraw,
        typically used in games like R-Type Leo to multiplex the top playfield.

*****************************************************************************

    Master Control registers:

        Word 0: Playfield 1 control
            Bit  0x40:  1 = Rowscroll enable, 0 = disable
            Bit  0x10:  0 = Playfield enable, 1 = disable
            Bit  0x04:  0 = 512 x 512 playfield, 1 = 1024 x 512 playfield
            Bits 0x03:  Playfield location in VRAM (0, 0x4000, 0x8000, 0xc000)
        Word 1: Playfield 2 control (as above)
        Word 2: Playfield 3 control (as above)
        Word 3: Raster IRQ position.

    The raster IRQ position is offset by 128+8 from the first visible line,
    suggesting there are 8 lines before the first visible one.

*****************************************************************************/

#include "driver.h"
#include "m92.h"

typedef struct _pf_layer_info pf_layer_info;
struct _pf_layer_info
{
	tilemap *		tmap;
	tilemap *		wide_tmap;
	UINT16			vram_base;
	UINT16			control[4];
};

static pf_layer_info pf_layer[3];

static UINT16 pf_master_control[4];
static INT32 m92_sprite_list;

UINT32 m92_raster_irq_position;
UINT16 *m92_vram_data,*m92_spritecontrol;
UINT8 m92_game_kludge;

extern void m92_sprite_interrupt(running_machine *machine);
UINT8 m92_sprite_buffer_busy;
static int m92_palette_bank;

/*****************************************************************************/

static TIMER_CALLBACK( spritebuffer_callback )
{
	m92_sprite_buffer_busy = 1;
	if (m92_game_kludge!=2) /* Major Title 2 doesn't like this interrupt!? */
		m92_sprite_interrupt(machine);
}

WRITE16_HANDLER( m92_spritecontrol_w )
{
	COMBINE_DATA(&m92_spritecontrol[offset]);

	/* Sprite list size register - used in spriteroutine */

	/* Sprite control - display all sprites, or partial list */
	if (offset==2 && ACCESSING_BITS_0_7)
	{
		if ((data & 0xff) == 8)
			m92_sprite_list = (((0x100 - m92_spritecontrol[0]) & 0xff) * 4);
		else
			m92_sprite_list = 0x400;

		/* Bit 0 is also significant */
	}

	/* Sprite buffer - the data written doesn't matter (confirmed by several games) */
	if (offset==4)
	{
			buffer_spriteram16_w(space,0,0,0xffff);
		m92_sprite_buffer_busy = 0;

		/* Pixel clock is 26.6666 MHz, we have 0x800 bytes, or 0x400 words
           to copy from spriteram to the buffer.  It seems safe to assume 1
           word can be copied per clock.*/
		timer_set(space->machine, attotime_mul(ATTOTIME_IN_HZ(26666000), 0x400), NULL, 0, spritebuffer_callback);
	}
//  logerror("%04x: m92_spritecontrol_w %08x %08x\n",cpu_get_pc(space->cpu),offset,data);
}

WRITE16_HANDLER( m92_videocontrol_w )
{
	/*
        Many games write:
            0x2000
            0x201b in alternate frames.

        Some games write to this both before and after the sprite buffer
        register - perhaps some kind of acknowledge bit is in there?

        Lethal Thunder fails it's RAM test with the upper palette bank
        enabled.  This was one of the earlier games and could actually
        be a different motherboard revision (most games use M92-A-B top
        pcb, a M92-A-A revision could exist...).
    */
	if (ACCESSING_BITS_0_7)
	{
		/* Access to upper palette bank */
		m92_palette_bank = (data >> 1) & 1;
	}
//  logerror("%04x: m92_videocontrol_w %d = %02x\n",cpu_get_pc(space->cpu),offset,data);
}

READ16_HANDLER( m92_paletteram_r )
{
	return paletteram16[offset + 0x400 * m92_palette_bank];
}

WRITE16_HANDLER( m92_paletteram_w )
{
	paletteram16_xBBBBBGGGGGRRRRR_word_w(space, offset + 0x400 * m92_palette_bank, data, mem_mask);
}

/*****************************************************************************/

static TILE_GET_INFO( get_pf_tile_info )
{
	pf_layer_info *layer = (pf_layer_info *)param;
	int tile, attrib;
	tile_index = 2 * tile_index + layer->vram_base;

	attrib = m92_vram_data[tile_index + 1];
	tile = m92_vram_data[tile_index] + ((attrib & 0x8000) << 1);

	SET_TILE_INFO(
			0,
			tile,
			attrib & 0x7f,
			TILE_FLIPYX(attrib >> 9));
	if (attrib & 0x100) tileinfo->group = 2;
	else if (attrib & 0x80) tileinfo->group = 1;
	else tileinfo->group = 0;
}

/*****************************************************************************/

WRITE16_HANDLER( m92_vram_w )
{
	int laynum;

	COMBINE_DATA(&m92_vram_data[offset]);

	for (laynum = 0; laynum < 3; laynum++)
	{
		if ((offset & 0x6000) == pf_layer[laynum].vram_base)
		{
			tilemap_mark_tile_dirty(pf_layer[laynum].tmap, (offset & 0x1fff) / 2);
			tilemap_mark_tile_dirty(pf_layer[laynum].wide_tmap, (offset & 0x3fff) / 2);
		}
		if ((offset & 0x6000) == pf_layer[laynum].vram_base + 0x2000)
			tilemap_mark_tile_dirty(pf_layer[laynum].wide_tmap, (offset & 0x3fff) / 2);
	}
}

/*****************************************************************************/

WRITE16_HANDLER( m92_pf1_control_w )
{
	COMBINE_DATA(&pf_layer[0].control[offset]);
}

WRITE16_HANDLER( m92_pf2_control_w )
{
	COMBINE_DATA(&pf_layer[1].control[offset]);
}

WRITE16_HANDLER( m92_pf3_control_w )
{
	COMBINE_DATA(&pf_layer[2].control[offset]);
}

WRITE16_HANDLER( m92_master_control_w )
{
	UINT16 old = pf_master_control[offset];
	pf_layer_info *layer;

	COMBINE_DATA(&pf_master_control[offset]);

	switch (offset)
	{
		case 0: /* Playfield 1 (top layer) */
		case 1: /* Playfield 2 (middle layer) */
		case 2: /* Playfield 3 (bottom layer) */
			layer = &pf_layer[offset];

			/* update VRAM base (bits 0-1) */
			layer->vram_base = (pf_master_control[offset] & 3) * 0x2000;

			/* update size (bit 2) */
			if (pf_master_control[offset] & 0x04)
			{
				tilemap_set_enable(layer->tmap, FALSE);
				tilemap_set_enable(layer->wide_tmap, (~pf_master_control[offset] >> 4) & 1);
			}
			else
			{
				tilemap_set_enable(layer->tmap, (~pf_master_control[offset] >> 4) & 1);
				tilemap_set_enable(layer->wide_tmap, FALSE);
			}

			/* mark everything dirty of the VRAM base or size changes */
			if ((old ^ pf_master_control[offset]) & 0x07)
			{
				tilemap_mark_all_tiles_dirty(layer->tmap);
				tilemap_mark_all_tiles_dirty(layer->wide_tmap);
			}
			break;

		case 3:
			m92_raster_irq_position = pf_master_control[3] - 128;
			break;
	}
}

/*****************************************************************************/

VIDEO_START( m92 )
{
	int laynum;

	memset(&pf_layer, 0, sizeof(pf_layer));
	for (laynum = 0; laynum < 3; laynum++)
	{
		pf_layer_info *layer = &pf_layer[laynum];

		/* allocate two tilemaps per layer, one normal, one wide */
		layer->tmap = tilemap_create(machine, get_pf_tile_info, tilemap_scan_rows,  8,8, 64,64);
		layer->wide_tmap = tilemap_create(machine, get_pf_tile_info, tilemap_scan_rows,  8,8, 128,64);

		/* set the user data for each one to point to the layer */
		tilemap_set_user_data(layer->tmap, &pf_layer[laynum]);
		tilemap_set_user_data(layer->wide_tmap, &pf_layer[laynum]);

		/* set scroll offsets */
		tilemap_set_scrolldx(layer->tmap, 2 * laynum, -2 * laynum + 8);
		tilemap_set_scrolldy(layer->tmap, -128, -128);
		tilemap_set_scrolldx(layer->wide_tmap, 2 * laynum - 256, -2 * laynum + 8 - 256);
		tilemap_set_scrolldy(layer->wide_tmap, -128, -128);

		/* layer group 0 - totally transparent in front half */
		tilemap_set_transmask(layer->tmap, 0, 0xffff, (laynum == 2) ? 0x0000 : 0x0001);
		tilemap_set_transmask(layer->wide_tmap, 0, 0xffff, (laynum == 2) ? 0x0000 : 0x0001);

		/* layer group 1 - pens 0-7 transparent in front half */
		tilemap_set_transmask(layer->tmap, 1, 0x00ff, (laynum == 2) ? 0xff00 : 0xff01);
		tilemap_set_transmask(layer->wide_tmap, 1, 0x00ff, (laynum == 2) ? 0xff00 : 0xff01);

		/* layer group 2 - pen 0 transparent in front half */
		tilemap_set_transmask(layer->tmap, 2, 0x0001, (laynum == 2) ? 0xfffe : 0xffff);
		tilemap_set_transmask(layer->wide_tmap, 2, 0x0001, (laynum == 2) ? 0xfffe : 0xffff);

		state_save_register_item(machine, "layer", NULL, laynum, layer->vram_base);
		state_save_register_item_array(machine, "layer", NULL, laynum, layer->control);
	}

	paletteram16 = auto_alloc_array(machine, UINT16, 0x1000/2);

	memset(spriteram16,0,0x800);
	memset(buffered_spriteram16,0,0x800);

	state_save_register_global_array(machine, pf_master_control);

	state_save_register_global(machine, m92_sprite_list);
	state_save_register_global(machine, m92_raster_irq_position);
	state_save_register_global(machine, m92_sprite_buffer_busy);
	state_save_register_global(machine, m92_palette_bank);

	state_save_register_global_pointer(machine, paletteram16, 0x1000);
}

/*****************************************************************************/

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	int offs,k;

	for (k=0; k<8; k++)
	{
		for (offs = 0; offs < m92_sprite_list; )
		{
			int x,y,sprite,colour,fx,fy,x_multi,y_multi,i,j,s_ptr,pri_back,pri_sprite;

			y = buffered_spriteram16[offs+0] & 0x1ff;
			x = buffered_spriteram16[offs+3] & 0x1ff;

			if (buffered_spriteram16[offs+2] & 0x0080) pri_back=0; else pri_back=2;

		 	sprite= buffered_spriteram16[offs+1];
			colour = buffered_spriteram16[offs+2] & 0x007f;
			pri_sprite= (buffered_spriteram16[offs+0] & 0xe000) >> 13;

			fx = (buffered_spriteram16[offs+2] >> 8) & 1;
			fy = (buffered_spriteram16[offs+2] >> 9) & 1;
			y_multi = (buffered_spriteram16[offs+0] >> 9) & 3;
			x_multi = (buffered_spriteram16[offs+0] >> 11) & 3;

			y_multi = 1 << y_multi;
			x_multi = 1 << x_multi;

			offs += 4 * x_multi;
			if (pri_sprite != k) continue;

			x = x - 16;
			y = 384 - 16 - y;

			if (fx) x+=16 * (x_multi - 1);

			for (j=0; j<x_multi; j++)
			{
				s_ptr=8 * j;
				if (!fy) s_ptr+=y_multi-1;

				x &= 0x1ff;
				for (i=0; i<y_multi; i++)
				{
					if (flip_screen_get(machine)) {
						pdrawgfx_transpen(bitmap,cliprect,machine->gfx[1],
								sprite + s_ptr,
								colour,
								!fx,!fy,
								464-x,240-(y-i*16),
								machine->priority_bitmap,pri_back,0);

						// wrap around x
						pdrawgfx_transpen(bitmap,cliprect,machine->gfx[1],
								sprite + s_ptr,
								colour,
								!fx,!fy,
								464-x+512,240-(y-i*16),
								machine->priority_bitmap,pri_back,0);
					} else {
						pdrawgfx_transpen(bitmap,cliprect,machine->gfx[1],
								sprite + s_ptr,
								colour,
								fx,fy,
								x,y-i*16,
								machine->priority_bitmap,pri_back,0);

						// wrap around x
						pdrawgfx_transpen(bitmap,cliprect,machine->gfx[1],
								sprite + s_ptr,
								colour,
								fx,fy,
								x-512,y-i*16,
								machine->priority_bitmap,pri_back,0);
					}
					if (fy) s_ptr++; else s_ptr--;
				}
				if (fx) x-=16; else x+=16;
			}
		}
	}
}


/*****************************************************************************/

static void m92_update_scroll_positions(void)
{
	int laynum;
	int i;

	/*  Playfield 3 rowscroll data is 0xdfc00 - 0xdffff
        Playfield 2 rowscroll data is 0xdf800 - 0xdfbff
        Playfield 1 rowscroll data is 0xdf400 - 0xdf7ff

        It appears to be hardwired to those locations.

        In addition, each playfield is staggered 2 pixels horizontally from the
        previous one.  This is most obvious in Hook & Blademaster.

    */

    for (laynum = 0; laynum < 3; laynum++)
    {
    	pf_layer_info *layer = &pf_layer[laynum];

		if (pf_master_control[laynum] & 0x40)
		{
			const UINT16 *scrolldata = m92_vram_data + (0xf400 + 0x400 * laynum) / 2;

			tilemap_set_scroll_rows(layer->tmap, 512);
			tilemap_set_scroll_rows(layer->wide_tmap, 512);
			for (i = 0; i < 512; i++)
			{
				tilemap_set_scrollx(layer->tmap, i, scrolldata[i]);
				tilemap_set_scrollx(layer->wide_tmap, i, scrolldata[i]);
			}
		}
		else
		{
			tilemap_set_scroll_rows(layer->tmap, 1);
			tilemap_set_scroll_rows(layer->wide_tmap, 1);
			tilemap_set_scrollx(layer->tmap, 0, layer->control[2]);
			tilemap_set_scrollx(layer->wide_tmap, 0, layer->control[2]);
		}

		tilemap_set_scrolly(layer->tmap, 0, layer->control[0]);
		tilemap_set_scrolly(layer->wide_tmap, 0, layer->control[0]);
	}
}

/*****************************************************************************/

static void m92_screenrefresh(running_machine *machine, bitmap_t *bitmap,const rectangle *cliprect)
{
	bitmap_fill(machine->priority_bitmap, cliprect, 0);

	if ((~pf_master_control[2] >> 4) & 1)
	{
		tilemap_draw(bitmap, cliprect, pf_layer[2].wide_tmap, TILEMAP_DRAW_LAYER1, 0);
		tilemap_draw(bitmap, cliprect, pf_layer[2].tmap,      TILEMAP_DRAW_LAYER1, 0);
		tilemap_draw(bitmap, cliprect, pf_layer[2].wide_tmap, TILEMAP_DRAW_LAYER0, 1);
		tilemap_draw(bitmap, cliprect, pf_layer[2].tmap,      TILEMAP_DRAW_LAYER0, 1);
	}
	else
		bitmap_fill(bitmap, cliprect, 0);

	tilemap_draw(bitmap, cliprect, pf_layer[1].wide_tmap, TILEMAP_DRAW_LAYER1, 0);
	tilemap_draw(bitmap, cliprect, pf_layer[1].tmap,      TILEMAP_DRAW_LAYER1, 0);
	tilemap_draw(bitmap, cliprect, pf_layer[1].wide_tmap, TILEMAP_DRAW_LAYER0, 1);
	tilemap_draw(bitmap, cliprect, pf_layer[1].tmap,      TILEMAP_DRAW_LAYER0, 1);

	tilemap_draw(bitmap, cliprect, pf_layer[0].wide_tmap, TILEMAP_DRAW_LAYER1, 0);
	tilemap_draw(bitmap, cliprect, pf_layer[0].tmap,      TILEMAP_DRAW_LAYER1, 0);
	tilemap_draw(bitmap, cliprect, pf_layer[0].wide_tmap, TILEMAP_DRAW_LAYER0, 1);
	tilemap_draw(bitmap, cliprect, pf_layer[0].tmap,      TILEMAP_DRAW_LAYER0, 1);

	draw_sprites(machine, bitmap, cliprect);
}


VIDEO_UPDATE( m92 )
{
	m92_update_scroll_positions();
	m92_screenrefresh(screen->machine, bitmap, cliprect);

	/* Flipscreen appears hardwired to the dipswitch - strange */
	if (input_port_read(screen->machine, "DSW") & 0x100)
		flip_screen_set(screen->machine, 0);
	else
		flip_screen_set(screen->machine, 1);
	return 0;
}
