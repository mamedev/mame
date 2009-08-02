#include "driver.h"
#include "machine/eeprom.h"
#include "video/konamiic.h"
#include "includes/tmnt.h"


static int layer_colorbase[3],sprite_colorbase,bg_colorbase;
static int priorityflag;
static int layerpri[3];
static int prmrsocr_sprite_bank;
static int sorted_layer[3];
static int dim_c,dim_v;	/* ssriders, tmnt2 */
static int lastdim,lasten;

static tilemap *roz_tilemap;


static int glfgreat_roz_rom_bank,glfgreat_roz_char_bank,glfgreat_roz_rom_mode;

static TILE_GET_INFO( glfgreat_get_roz_tile_info )
{
	UINT8 *rom = memory_region(machine, "user1");
	int code;

	tile_index += 0x40000 * glfgreat_roz_rom_bank;

	code = rom[tile_index+0x80000] + 256*rom[tile_index] + 256*256*((rom[tile_index/4+0x100000]>>(2*(tile_index&3)))&3);

	SET_TILE_INFO(0,code & 0x3fff,code >> 14,0);
}

static TILE_GET_INFO( prmrsocr_get_roz_tile_info )
{
	UINT8 *rom = memory_region(machine, "user1");
	int code = rom[tile_index+0x20000] + 256*rom[tile_index];

	SET_TILE_INFO(0,code & 0x1fff,code >> 13,0);
}



/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

/* Missing in Action */

static void mia_tile_callback(int layer,int bank,int *code,int *color,int *flags,int *priority)
{
	*flags = (*color & 0x04) ? TILE_FLIPX : 0;
	if (layer == 0)
	{
		*code |= ((*color & 0x01) << 8);
		*color = layer_colorbase[layer] + ((*color & 0x80) >> 5) + ((*color & 0x10) >> 1);
	}
	else
	{
		*code |= ((*color & 0x01) << 8) | ((*color & 0x18) << 6) | (bank << 11);
		*color = layer_colorbase[layer] + ((*color & 0xe0) >> 5);
	}
}

static void cuebrick_tile_callback(int layer,int bank,int *code,int *color,int *flags,int *priority)
{
	if ((K052109_get_RMRD_line() == CLEAR_LINE) && (layer == 0))
	{
		*code |= ((*color & 0x01) << 8);
		*color = layer_colorbase[layer]  + ((*color & 0x80) >> 5) + ((*color & 0x10) >> 1);
	}
	else
	{
		*code |= ((*color & 0xf) << 8);
		*color = layer_colorbase[layer] + ((*color & 0xe0) >> 5);
	}
}

static void tmnt_tile_callback(int layer,int bank,int *code,int *color,int *flags,int *priority)
{
	*code |= ((*color & 0x03) << 8) | ((*color & 0x10) << 6) | ((*color & 0x0c) << 9)
			| (bank << 13);
	*color = layer_colorbase[layer] + ((*color & 0xe0) >> 5);
}

static void ssbl_tile_callback(int layer,int bank,int *code,int *color,int *flags,int *priority)
{
	if (layer == 0)
	{
		*code |= ((*color & 0x03) << 8) | ((*color & 0x10) << 6) | ((*color & 0x0c) << 9)
				| (bank << 13);
	}
	else
	{
		*code |= ((*color & 0x03) << 8) | ((*color & 0x10) << 6) | ((*color & 0x0c) << 9)
				| (bank << 13);
//      mame_printf_debug("L%d: bank %d code %x color %x\n", layer, bank, *code, *color);
	}

	*color = layer_colorbase[layer] + ((*color & 0xe0) >> 5);
}

static int blswhstl_rombank;

static void blswhstl_tile_callback(int layer,int bank,int *code,int *color,int *flags,int *priority)
{
	/* (color & 0x02) is flip y handled internally by the 052109 */
	*code |= ((*color & 0x01) << 8) | ((*color & 0x10) << 5) | ((*color & 0x0c) << 8)
			| (bank << 12) | blswhstl_rombank << 14;
	*color = layer_colorbase[layer] + ((*color & 0xe0) >> 5);
}



/***************************************************************************

  Callbacks for the K051960

***************************************************************************/

static void mia_sprite_callback(int *code,int *color,int *priority,int *shadow)
{
	*color = sprite_colorbase + (*color & 0x0f);
}

static void tmnt_sprite_callback(int *code,int *color,int *priority,int *shadow)
{
	*code |= (*color & 0x10) << 9;
	*color = sprite_colorbase + (*color & 0x0f);
}

static void punkshot_sprite_callback(int *code,int *color,int *priority_mask,int *shadow)
{
	int pri = 0x20 | ((*color & 0x60) >> 2);
	if (pri <= layerpri[2])								*priority_mask = 0;
	else if (pri > layerpri[2] && pri <= layerpri[1])	*priority_mask = 0xf0;
	else if (pri > layerpri[1] && pri <= layerpri[0])	*priority_mask = 0xf0|0xcc;
	else 												*priority_mask = 0xf0|0xcc|0xaa;

	*code |= (*color & 0x10) << 9;
	*color = sprite_colorbase + (*color & 0x0f);
}

static void thndrx2_sprite_callback(int *code,int *color,int *priority_mask,int *shadow)
{
	int pri = 0x20 | ((*color & 0x60) >> 2);
	if (pri <= layerpri[2])								*priority_mask = 0;
	else if (pri > layerpri[2] && pri <= layerpri[1])	*priority_mask = 0xf0;
	else if (pri > layerpri[1] && pri <= layerpri[0])	*priority_mask = 0xf0|0xcc;
	else 												*priority_mask = 0xf0|0xcc|0xaa;

	*color = sprite_colorbase + (*color & 0x0f);
}


/***************************************************************************

  Callbacks for the K053245

***************************************************************************/

static void lgtnfght_sprite_callback(int *code,int *color,int *priority_mask)
{
	int pri = 0x20 | ((*color & 0x60) >> 2);
	if (pri <= layerpri[2])								*priority_mask = 0;
	else if (pri > layerpri[2] && pri <= layerpri[1])	*priority_mask = 0xf0;
	else if (pri > layerpri[1] && pri <= layerpri[0])	*priority_mask = 0xf0|0xcc;
	else 												*priority_mask = 0xf0|0xcc|0xaa;

	*color = sprite_colorbase + (*color & 0x1f);
}

static void blswhstl_sprite_callback(int *code,int *color,int *priority_mask)
{
#if 0
if (input_code_pressed(KEYCODE_Q) && (*color & 0x20)) *color = rand();
if (input_code_pressed(KEYCODE_W) && (*color & 0x40)) *color = rand();
if (input_code_pressed(KEYCODE_E) && (*color & 0x80)) *color = rand();
#endif
	int pri = 0x20 | ((*color & 0x60) >> 2);
	if (pri <= layerpri[2])								*priority_mask = 0;
	else if (pri > layerpri[2] && pri <= layerpri[1])	*priority_mask = 0xf0;
	else if (pri > layerpri[1] && pri <= layerpri[0])	*priority_mask = 0xf0|0xcc;
	else 												*priority_mask = 0xf0|0xcc|0xaa;

	*color = sprite_colorbase + (*color & 0x1f);
}

static void prmrsocr_sprite_callback(int *code,int *color,int *priority_mask)
{
	int pri = 0x20 | ((*color & 0x60) >> 2);
	if (pri <= layerpri[2])								*priority_mask = 0;
	else if (pri > layerpri[2] && pri <= layerpri[1])	*priority_mask = 0xf0;
	else if (pri > layerpri[1] && pri <= layerpri[0])	*priority_mask = 0xf0|0xcc;
	else 												*priority_mask = 0xf0|0xcc|0xaa;

	*code |= prmrsocr_sprite_bank << 14;

	*color = sprite_colorbase + (*color & 0x1f);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( mia )
{
	layer_colorbase[0] = 0;
	layer_colorbase[1] = 32;
	layer_colorbase[2] = 40;
	sprite_colorbase = 16;
	K052109_vh_start(machine,"gfx1",NORMAL_PLANE_ORDER,mia_tile_callback);
	K051960_vh_start(machine,"gfx2",REVERSE_PLANE_ORDER,mia_sprite_callback);
}

VIDEO_START( cuebrick )
{
	layer_colorbase[0] = 0;
	layer_colorbase[1] = 32;
	layer_colorbase[2] = 40;
	sprite_colorbase = 16;
	K052109_vh_start(machine,"gfx1",NORMAL_PLANE_ORDER,cuebrick_tile_callback);
	K051960_vh_start(machine,"gfx2",REVERSE_PLANE_ORDER,mia_sprite_callback);
}

VIDEO_START( tmnt )
{
	layer_colorbase[0] = 0;
	layer_colorbase[1] = 32;
	layer_colorbase[2] = 40;
	sprite_colorbase = 16;
	K052109_vh_start(machine,"gfx1",NORMAL_PLANE_ORDER,tmnt_tile_callback);
	K051960_vh_start(machine,"gfx2",REVERSE_PLANE_ORDER,tmnt_sprite_callback);
}

VIDEO_START( punkshot )
{
	K053251_vh_start(machine);
	K052109_vh_start(machine,"gfx1",NORMAL_PLANE_ORDER,tmnt_tile_callback);
	K051960_vh_start(machine,"gfx2",NORMAL_PLANE_ORDER,punkshot_sprite_callback);
}

VIDEO_START( lgtnfght )	/* also tmnt2, ssriders */
{
	K053251_vh_start(machine);
	K052109_vh_start(machine,"gfx1",NORMAL_PLANE_ORDER,tmnt_tile_callback);
	K053245_vh_start(machine,0, "gfx2",NORMAL_PLANE_ORDER,lgtnfght_sprite_callback);

	K05324x_set_z_rejection(0);

	dim_c = dim_v = lastdim = lasten = 0;

	state_save_register_global(machine, dim_c);
	state_save_register_global(machine, dim_v);
	state_save_register_global(machine, lastdim);
	state_save_register_global(machine, lasten);
}

VIDEO_START( sunsetbl )
{
	K053251_vh_start(machine);
	K052109_vh_start(machine,"gfx1",NORMAL_PLANE_ORDER,ssbl_tile_callback);
	K053245_vh_start(machine,0, "gfx2",NORMAL_PLANE_ORDER,lgtnfght_sprite_callback);
}

VIDEO_START( blswhstl )
{
	K053251_vh_start(machine);
	K052109_vh_start(machine,"gfx1",NORMAL_PLANE_ORDER,blswhstl_tile_callback);
	K053245_vh_start(machine,0, "gfx2",NORMAL_PLANE_ORDER,blswhstl_sprite_callback);
}

VIDEO_START( glfgreat )
{
	K053251_vh_start(machine);
	K052109_vh_start(machine,"gfx1",NORMAL_PLANE_ORDER,tmnt_tile_callback);
	K053245_vh_start(machine,0, "gfx2",NORMAL_PLANE_ORDER,lgtnfght_sprite_callback);

	roz_tilemap = tilemap_create(machine, glfgreat_get_roz_tile_info,tilemap_scan_rows,16,16,512,512);

	tilemap_set_transparent_pen(roz_tilemap,0);

	K053936_wraparound_enable(0, 1);
	K053936_set_offset(0, 85, 0);
}

VIDEO_START( thndrx2 )
{
	K053251_vh_start(machine);
	K052109_vh_start(machine,"gfx1",NORMAL_PLANE_ORDER,tmnt_tile_callback);
	K051960_vh_start(machine,"gfx2",NORMAL_PLANE_ORDER,thndrx2_sprite_callback);
}

VIDEO_START( prmrsocr )
{
	K053251_vh_start(machine);
	K052109_vh_start(machine,"gfx1",NORMAL_PLANE_ORDER,tmnt_tile_callback);
	K053245_vh_start(machine,0, "gfx2",NORMAL_PLANE_ORDER,prmrsocr_sprite_callback);

	roz_tilemap = tilemap_create(machine, prmrsocr_get_roz_tile_info,tilemap_scan_rows,16,16,512,256);

	tilemap_set_transparent_pen(roz_tilemap,0);

	K053936_wraparound_enable(0, 0);
	K053936_set_offset(0, 85, 1);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE16_HANDLER( tmnt_paletteram_word_w )
{
	COMBINE_DATA(paletteram16 + offset);
	offset &= ~1;

	data = (paletteram16[offset] << 8) | paletteram16[offset+1];
	palette_set_color_rgb(space->machine,offset / 2,pal5bit(data >> 0),pal5bit(data >> 5),pal5bit(data >> 10));
}



WRITE16_HANDLER( tmnt_0a0000_w )
{
	if (ACCESSING_BITS_0_7)
	{
		static int last;

		/* bit 0/1 = coin counters */
		coin_counter_w(0, data & 0x01);
		coin_counter_w(1, data & 0x02);	/* 2 players version */

		/* bit 3 high then low triggers irq on sound CPU */
		if (last == 0x08 && (data & 0x08) == 0)
			cputag_set_input_line_and_vector(space->machine, "audiocpu", 0, HOLD_LINE, 0xff);

		last = data & 0x08;

		/* bit 5 = irq enable */
		interrupt_enable_w(space, 0, data & 0x20);

		/* bit 7 = enable char ROM reading through the video RAM */
		K052109_set_RMRD_line((data & 0x80) ? ASSERT_LINE : CLEAR_LINE);

		/* other bits unused */
	}
}

WRITE16_HANDLER( punkshot_0a0020_w )
{
	if (ACCESSING_BITS_0_7)
	{
		static int last;


		/* bit 0 = coin counter */
		coin_counter_w(0, data & 0x01);

		/* bit 2 = trigger irq on sound CPU */
		if (last == 0x04 && (data & 0x04) == 0)
			cputag_set_input_line_and_vector(space->machine, "audiocpu", 0, HOLD_LINE, 0xff);

		last = data & 0x04;

		/* bit 3 = enable char ROM reading through the video RAM */
		K052109_set_RMRD_line((data & 0x08) ? ASSERT_LINE : CLEAR_LINE);
	}
}

WRITE16_HANDLER( lgtnfght_0a0018_w )
{
	if (ACCESSING_BITS_0_7)
	{
		static int last;


		/* bit 0,1 = coin counter */
		coin_counter_w(0, data & 0x01);
		coin_counter_w(1, data & 0x02);

		/* bit 2 = trigger irq on sound CPU */
		if (last == 0x00 && (data & 0x04) == 0x04)
			cputag_set_input_line_and_vector(space->machine, "audiocpu", 0, HOLD_LINE, 0xff);

		last = data & 0x04;

		/* bit 3 = enable char ROM reading through the video RAM */
		K052109_set_RMRD_line((data & 0x08) ? ASSERT_LINE : CLEAR_LINE);
	}
}

WRITE16_HANDLER( blswhstl_700300_w )
{
	if (ACCESSING_BITS_0_7)
	{
		/* bit 0,1 = coin counter */
		coin_counter_w(0,data & 0x01);
		coin_counter_w(1,data & 0x02);

		/* bit 3 = enable char ROM reading through the video RAM */
		K052109_set_RMRD_line((data & 0x08) ? ASSERT_LINE : CLEAR_LINE);

		/* bit 7 = select char ROM bank */
		if (blswhstl_rombank != ((data & 0x80) >> 7))
		{
			blswhstl_rombank = (data & 0x80) >> 7;
			tilemap_mark_all_tiles_dirty(ALL_TILEMAPS);
		}

		/* other bits unknown */
	}
}


READ16_HANDLER( glfgreat_rom_r )
{
	if (glfgreat_roz_rom_mode)
		return memory_region(space->machine, "gfx3")[glfgreat_roz_char_bank * 0x80000 + offset];
	else if (offset < 0x40000)
	{
		UINT8 *usr = memory_region(space->machine, "user1");
		return usr[offset + 0x80000 + glfgreat_roz_rom_bank * 0x40000] +
				256 * usr[offset + glfgreat_roz_rom_bank * 0x40000];
	}
	else
		return memory_region(space->machine, "user1")[((offset & 0x3ffff) >> 2) + 0x100000 + glfgreat_roz_rom_bank * 0x10000];
}

WRITE16_HANDLER( glfgreat_122000_w )
{
	if (ACCESSING_BITS_0_7)
	{
		/* bit 0,1 = coin counter */
		coin_counter_w(0,data & 0x01);
		coin_counter_w(1,data & 0x02);

		/* bit 4 = enable char ROM reading through the video RAM */
		K052109_set_RMRD_line((data & 0x10) ? ASSERT_LINE : CLEAR_LINE);

		/* bit 5 = 53596 tile rom bank selection */
		if (glfgreat_roz_rom_bank != (data & 0x20) >> 5)
		{
			glfgreat_roz_rom_bank = (data & 0x20) >> 5;
			tilemap_mark_all_tiles_dirty(roz_tilemap);
		}

		/* bit 6,7 = 53596 char bank selection for ROM test */
		glfgreat_roz_char_bank = (data & 0xc0) >> 6;

		/* other bits unknown */
	}
	if (ACCESSING_BITS_8_15)
	{
		/* bit 8 = 53596 char/rom selection for ROM test */
		glfgreat_roz_rom_mode = data & 0x100;
	}
}


WRITE16_HANDLER( ssriders_eeprom_w )
{
	if (ACCESSING_BITS_0_7)
	{
		/* bit 0 is data */
		/* bit 1 is cs (active low) */
		/* bit 2 is clock (active high) */
		eeprom_write_bit(data & 0x01);
		eeprom_set_cs_line((data & 0x02) ? CLEAR_LINE : ASSERT_LINE);
		eeprom_set_clock_line((data & 0x04) ? ASSERT_LINE : CLEAR_LINE);

		/* bits 3-4 control palette dimming */
		/* 4 = DIMPOL = when set, negate SHAD */
		/* 3 = DIMMOD = when set, or BRIT with [negated] SHAD */
		dim_c = data & 0x18;

		/* bit 5 selects sprite ROM for testing in TMNT2 (bits 5-7, actually, according to the schematics) */
		K053244_bankselect(0, ((data & 0x20) >> 5) << 2);
	}
}

WRITE16_HANDLER( ssriders_1c0300_w )
{
	if (ACCESSING_BITS_0_7)
	{
		/* bit 0,1 = coin counter */
		coin_counter_w(0,data & 0x01);
		coin_counter_w(1,data & 0x02);

		/* bit 3 = enable char ROM reading through the video RAM */
		K052109_set_RMRD_line((data & 0x08) ? ASSERT_LINE : CLEAR_LINE);

		/* bits 4-6 control palette dimming (DIM0-DIM2) */
		dim_v = (data & 0x70) >> 4;
	}
}

WRITE16_HANDLER( prmrsocr_122000_w )
{
	if (ACCESSING_BITS_0_7)
	{
		/* bit 0,1 = coin counter */
		coin_counter_w(0,data & 0x01);
		coin_counter_w(1,data & 0x02);

		/* bit 4 = enable char ROM reading through the video RAM */
		K052109_set_RMRD_line((data & 0x10) ? ASSERT_LINE : CLEAR_LINE);

		/* bit 6 = sprite ROM bank */
		prmrsocr_sprite_bank = (data & 0x40) >> 6;
		K053244_bankselect(0, prmrsocr_sprite_bank << 2);

		/* bit 7 = 53596 region selector for ROM test */
		glfgreat_roz_char_bank = (data & 0x80) >> 7;

		/* other bits unknown (unused?) */
	}
}

READ16_HANDLER( prmrsocr_rom_r )
{
	if(glfgreat_roz_char_bank)
		return memory_region(space->machine, "gfx3")[offset];
	else
	{
		UINT8 *usr = memory_region(space->machine, "user1");
		return 256 * usr[offset] + usr[offset + 0x020000];
	}
}

WRITE16_HANDLER( tmnt_priority_w )
{
	if (ACCESSING_BITS_0_7)
	{
		/* bit 2/3 = priority; other bits unused */
		/* bit2 = PRI bit3 = PRI2
              sprite/playfield priority is controlled by these two bits, by bit 3
              of the background tile color code, and by the SHADOW sprite
              attribute bit.
              Priorities are encoded in a PROM (G19 for TMNT). However, in TMNT,
              the PROM only takes into account the PRI and SHADOW bits.
              PRI  Priority
               0   bg fg spr text
               1   bg spr fg text
              The SHADOW bit, when set, torns a sprite into a shadow which makes
              color below it darker (this is done by turning off three resistors
              in parallel with the RGB output).

              Note: the background color (color used when all of the four layers
              are 0) is taken from the *foreground* palette, not the background
              one as would be more intuitive.
        */
		priorityflag = (data & 0x0c) >> 2;
	}
}



/***************************************************************************

  Display refresh

***************************************************************************/

/* useful function to sort the three tile layers by priority order */
static void sortlayers(int *layer,int *pri)
{
#define SWAP(a,b) \
	if (pri[a] < pri[b]) \
	{ \
		int t; \
		t = pri[a]; pri[a] = pri[b]; pri[b] = t; \
		t = layer[a]; layer[a] = layer[b]; layer[b] = t; \
	}

	SWAP(0,1)
	SWAP(0,2)
	SWAP(1,2)
}

VIDEO_UPDATE( mia )
{
	K052109_tilemap_update();

	tilemap_draw(bitmap,cliprect,K052109_tilemap[2],TILEMAP_DRAW_OPAQUE,0);
	if ((priorityflag & 1) == 1) K051960_sprites_draw(screen->machine,bitmap,cliprect,0,0);
	tilemap_draw(bitmap,cliprect,K052109_tilemap[1],0,0);
	if ((priorityflag & 1) == 0) K051960_sprites_draw(screen->machine,bitmap,cliprect,0,0);
	tilemap_draw(bitmap,cliprect,K052109_tilemap[0],0,0);
	return 0;
}

VIDEO_UPDATE( tmnt )
{
	K052109_tilemap_update();

	tilemap_draw(bitmap,cliprect,K052109_tilemap[2],TILEMAP_DRAW_OPAQUE,0);
	if ((priorityflag & 1) == 1) K051960_sprites_draw(screen->machine,bitmap,cliprect,0,0);
	tilemap_draw(bitmap,cliprect,K052109_tilemap[1],0,0);
	if ((priorityflag & 1) == 0) K051960_sprites_draw(screen->machine,bitmap,cliprect,0,0);
	tilemap_draw(bitmap,cliprect,K052109_tilemap[0],0,0);
	return 0;
}


VIDEO_UPDATE( punkshot )
{
	bg_colorbase       = K053251_get_palette_index(K053251_CI0);
	sprite_colorbase   = K053251_get_palette_index(K053251_CI1);
	layer_colorbase[0] = K053251_get_palette_index(K053251_CI2);
	layer_colorbase[1] = K053251_get_palette_index(K053251_CI4);
	layer_colorbase[2] = K053251_get_palette_index(K053251_CI3);

	K052109_tilemap_update();

	sorted_layer[0] = 0;
	layerpri[0] = K053251_get_priority(K053251_CI2);
	sorted_layer[1] = 1;
	layerpri[1] = K053251_get_priority(K053251_CI4);
	sorted_layer[2] = 2;
	layerpri[2] = K053251_get_priority(K053251_CI3);

	sortlayers(sorted_layer,layerpri);

	bitmap_fill(screen->machine->priority_bitmap,cliprect,0);
	tilemap_draw(bitmap,cliprect,K052109_tilemap[sorted_layer[0]],TILEMAP_DRAW_OPAQUE,1);
	tilemap_draw(bitmap,cliprect,K052109_tilemap[sorted_layer[1]],0,2);
	tilemap_draw(bitmap,cliprect,K052109_tilemap[sorted_layer[2]],0,4);

	K051960_sprites_draw(screen->machine,bitmap,cliprect,-1,-1);
	return 0;
}


VIDEO_UPDATE( lgtnfght )
{
	bg_colorbase       = K053251_get_palette_index(K053251_CI0);
	sprite_colorbase   = K053251_get_palette_index(K053251_CI1);
	layer_colorbase[0] = K053251_get_palette_index(K053251_CI2);
	layer_colorbase[1] = K053251_get_palette_index(K053251_CI4);
	layer_colorbase[2] = K053251_get_palette_index(K053251_CI3);

	K052109_tilemap_update();

	sorted_layer[0] = 0;
	layerpri[0] = K053251_get_priority(K053251_CI2);
	sorted_layer[1] = 1;
	layerpri[1] = K053251_get_priority(K053251_CI4);
	sorted_layer[2] = 2;
	layerpri[2] = K053251_get_priority(K053251_CI3);

	sortlayers(sorted_layer,layerpri);

	bitmap_fill(screen->machine->priority_bitmap,cliprect,0);
	bitmap_fill(bitmap,cliprect,16 * bg_colorbase);
	tilemap_draw(bitmap,cliprect,K052109_tilemap[sorted_layer[0]],0,1);
	tilemap_draw(bitmap,cliprect,K052109_tilemap[sorted_layer[1]],0,2);
	tilemap_draw(bitmap,cliprect,K052109_tilemap[sorted_layer[2]],0,4);

	K053245_sprites_draw(screen->machine, 0, bitmap,cliprect);
	return 0;
}

static int glfgreat_pixel;

READ16_HANDLER( glfgreat_ball_r )
{
#ifdef MAME_DEBUG
popmessage("%04x",glfgreat_pixel);
#endif
	/* if out of the ROZ layer palette range, it's in the water - return 0 */
	if (glfgreat_pixel < 0x400 || glfgreat_pixel >= 0x500) return 0;
	else return glfgreat_pixel & 0xff;
}

VIDEO_UPDATE( glfgreat )
{
	K053251_set_tilemaps(NULL,NULL,K052109_tilemap[0],K052109_tilemap[1],K052109_tilemap[2]);

	bg_colorbase       = K053251_get_palette_index(K053251_CI0);
	sprite_colorbase   = K053251_get_palette_index(K053251_CI1);
	layer_colorbase[0] = K053251_get_palette_index(K053251_CI2);
	layer_colorbase[1] = K053251_get_palette_index(K053251_CI3) + 8;	/* weird... */
	layer_colorbase[2] = K053251_get_palette_index(K053251_CI4);

	K052109_tilemap_update();

	sorted_layer[0] = 0;
	layerpri[0] = K053251_get_priority(K053251_CI2);
	sorted_layer[1] = 1;
	layerpri[1] = K053251_get_priority(K053251_CI3);
	sorted_layer[2] = 2;
	layerpri[2] = K053251_get_priority(K053251_CI4);

	sortlayers(sorted_layer,layerpri);

	/* not sure about the 053936 priority, but it seems to work */

	bitmap_fill(screen->machine->priority_bitmap,cliprect,0);
	bitmap_fill(bitmap,cliprect,16 * bg_colorbase);
	tilemap_draw(bitmap,cliprect,K052109_tilemap[sorted_layer[0]],0,1);
	if (layerpri[0] >= 0x30 && layerpri[1] < 0x30)
	{
		K053936_0_zoom_draw(bitmap,cliprect,roz_tilemap,0,1,1);
		glfgreat_pixel = *BITMAP_ADDR16(bitmap,0x80,0x105);
	}
	tilemap_draw(bitmap,cliprect,K052109_tilemap[sorted_layer[1]],0,2);
	if (layerpri[1] >= 0x30 && layerpri[2] < 0x30)
	{
		K053936_0_zoom_draw(bitmap,cliprect,roz_tilemap,0,1,1);
		glfgreat_pixel = *BITMAP_ADDR16(bitmap,0x80,0x105);
	}
	tilemap_draw(bitmap,cliprect,K052109_tilemap[sorted_layer[2]],0,4);
	if (layerpri[2] >= 0x30)
	{
		K053936_0_zoom_draw(bitmap,cliprect,roz_tilemap,0,1,1);
		glfgreat_pixel = *BITMAP_ADDR16(bitmap,0x80,0x105);
	}

	K053245_sprites_draw(screen->machine, 0, bitmap,cliprect);
	return 0;
}

VIDEO_UPDATE( tmnt2 )
{
	double brt;
	int i, newdim, newen, cb, ce;

	newdim = dim_v | ((~dim_c & 0x10) >> 1);
	newen  = (K053251_get_priority(5) && K053251_get_priority(5) != 0x3e);

	if (newdim != lastdim || newen != lasten)
	{
		brt = 1.0;
		if (newen) brt -= (1.0-PALETTE_DEFAULT_SHADOW_FACTOR)*newdim/8;
		lastdim = newdim;
		lasten  = newen;

		/*
            Only affect the background and sprites, not text layer.
            Instead of dimming each layer we dim the entire palette
            except text colors because palette bases may change
            anytime and there's no guarantee a dimmed color will be
            reset properly.
        */

		// find the text layer's palette range
		cb = layer_colorbase[sorted_layer[2]] << 4;
		ce = cb + 128;

		// dim all colors before it
		for (i=0; i<cb; i++)
			palette_set_pen_contrast(screen->machine,i,brt);

		// reset all colors in range
		for (i=cb; i<ce; i++)
			palette_set_pen_contrast(screen->machine,i,1.0);

		// dim all colors after it
		for (i=ce; i<2048; i++)
			palette_set_pen_contrast(screen->machine,i,brt);

		// toggle shadow/highlight
		if (~dim_c & 0x10)
			palette_set_shadow_mode(screen->machine, 1);
		else
			palette_set_shadow_mode(screen->machine, 0);
	}

	VIDEO_UPDATE_CALL(lgtnfght);
	return 0;
}


VIDEO_UPDATE( thndrx2 )
{
	bg_colorbase       = K053251_get_palette_index(K053251_CI0);
	sprite_colorbase   = K053251_get_palette_index(K053251_CI1);
	layer_colorbase[0] = K053251_get_palette_index(K053251_CI2);
	layer_colorbase[1] = K053251_get_palette_index(K053251_CI4);
	layer_colorbase[2] = K053251_get_palette_index(K053251_CI3);

	K052109_tilemap_update();

	sorted_layer[0] = 0;
	layerpri[0] = K053251_get_priority(K053251_CI2);
	sorted_layer[1] = 1;
	layerpri[1] = K053251_get_priority(K053251_CI4);
	sorted_layer[2] = 2;
	layerpri[2] = K053251_get_priority(K053251_CI3);

	sortlayers(sorted_layer,layerpri);

	bitmap_fill(screen->machine->priority_bitmap,cliprect,0);
	bitmap_fill(bitmap,cliprect,16 * bg_colorbase);
	tilemap_draw(bitmap,cliprect,K052109_tilemap[sorted_layer[0]],0,1);
	tilemap_draw(bitmap,cliprect,K052109_tilemap[sorted_layer[1]],0,2);
	tilemap_draw(bitmap,cliprect,K052109_tilemap[sorted_layer[2]],0,4);

	K051960_sprites_draw(screen->machine,bitmap,cliprect,-1,-1);
	return 0;
}



/***************************************************************************

  Housekeeping

***************************************************************************/

VIDEO_EOF( blswhstl )
{
	K053245_clear_buffer(0);
}
