/***************************************************************************

   Tumblepop (bootlegs) and similar hardware
   Video emulation - Bryan McPhail, mish@tendril.co.uk

*********************************************************************

The original uses Data East custom chip 55 for backgrounds,
custom chip 52 for sprites.  The bootlegs use generic chips to perform similar
functions

Tumblepop is one of few games to take advantage of the playfields ability
to switch between 8*8 tiles and 16*16 tiles.

***************************************************************************/

#include "driver.h"

static UINT16 tumblepb_control_0[8];
UINT16 *tumblepb_pf1_data,*tumblepb_pf2_data;
static tilemap *pf1_tilemap,*pf1_alt_tilemap,*pf2_tilemap,*pf2_alt_tilemap;
static int flipscreen;
extern UINT16* jumppop_control;
static UINT16 bcstory_tilebank;
extern UINT16* suprtrio_control;
static int sprite_xoffset;
static int sprite_yoffset;


/******************************************************************************/

static void tumblepb_draw_sprites(running_machine *machine, mame_bitmap *bitmap,const rectangle *cliprect)
{
	int offs;

	for (offs = 0;offs < 0x400;offs += 4)
	{
		int x,y,sprite,colour,multi,fx,fy,inc,flash,mult;

		sprite = spriteram16[offs+1] & 0x3fff;
		if (!sprite) continue;

		y = spriteram16[offs];
		flash=y&0x1000;
		if (flash && (cpu_getcurrentframe() & 1)) continue;

		x = spriteram16[offs+2];
		colour = (x >>9) & 0xf;

		fx = y & 0x2000;
		fy = y & 0x4000;
		multi = (1 << ((y & 0x0600) >> 9)) - 1;	/* 1x, 2x, 4x, 8x height */

		x = x & 0x01ff;
		y = y & 0x01ff;
		if (x >= 320) x -= 512;
		if (y >= 256) y -= 512;
		y = 240 - y;
        x = 304 - x;

		sprite &= ~multi;
		if (fy)
			inc = -1;
		else
		{
			sprite += multi;
			inc = 1;
		}

		if (flipscreen)
		{
			y=240-y;
			x=304-x;
			if (fx) fx=0; else fx=1;
			if (fy) fy=0; else fy=1;
			mult=16;
		}
		else mult=-16;

		while (multi >= 0)
		{
			drawgfx(bitmap,machine->gfx[3],
					sprite - multi * inc,
					colour,
					fx,fy,
					sprite_xoffset + x,sprite_yoffset + y + mult * multi,
					cliprect,TRANSPARENCY_PEN,0);

			multi--;
		}
	}
}

static void jumpkids_draw_sprites(running_machine *machine, mame_bitmap *bitmap,const rectangle *cliprect)
{
	int offs;

	for (offs = 0;offs < spriteram_size/2;offs += 4)
	{
		int x,y,sprite,colour,multi,fx,fy,inc,flash,mult;

		sprite = spriteram16[offs+1] & 0x7fff;
		if (!sprite) continue;

		y = spriteram16[offs];
		flash=y&0x1000;
		if (flash && (cpu_getcurrentframe() & 1)) continue;

		x = spriteram16[offs+2];
		colour = (x >>9) & 0xf;

		fx = y & 0x2000;
		fy = y & 0x4000;
		multi = (1 << ((y & 0x0600) >> 9)) - 1;	/* 1x, 2x, 4x, 8x height */

		x = x & 0x01ff;
		y = y & 0x01ff;
		if (x >= 320) x -= 512;
		if (y >= 256) y -= 512;
		y = 240 - y;
        x = 304 - x;

	//  sprite &= ~multi; /* Todo:  I bet TumblePop bootleg doesn't do this either */
		if (fy)
			inc = -1;
		else
		{
			sprite += multi;
			inc = 1;
		}

		if (flipscreen)
		{
			y=240-y;
			x=304-x;
			if (fx) fx=0; else fx=1;
			if (fy) fy=0; else fy=1;
			mult=16;
		}
		else mult=-16;

		while (multi >= 0)
		{
			drawgfx(bitmap,machine->gfx[3],
					sprite - multi * inc,
					colour,
					fx,fy,
					sprite_xoffset+x,sprite_yoffset + y + mult * multi, // x-1 for bcstory .. realign other layers?
					cliprect,TRANSPARENCY_PEN,0);

			multi--;
		}
	}
}

static void fncywld_draw_sprites(running_machine *machine, mame_bitmap *bitmap,const rectangle *cliprect)
{
	int offs;

	for (offs = 0;offs < 0x400;offs += 4)
	{
		int x,y,sprite,colour,multi,fx,fy,inc,flash,mult;

		sprite = spriteram16[offs+1] & 0x3fff;
		if (!sprite) continue;

		y = spriteram16[offs];
		flash=y&0x1000;
		if (flash && (cpu_getcurrentframe() & 1)) continue;

		x = spriteram16[offs+2];
		colour = (x >>9) & 0x3f;

		fx = y & 0x2000;
		fy = y & 0x4000;
		multi = (1 << ((y & 0x0600) >> 9)) - 1;	/* 1x, 2x, 4x, 8x height */

		x = x & 0x01ff;
		y = y & 0x01ff;
		if (x >= 320) x -= 512;
		if (y >= 256) y -= 512;
		y = 240 - y;
        x = 304 - x;

	//  sprite &= ~multi; /* Todo:  I bet TumblePop bootleg doesn't do this either */
		if (fy)
			inc = -1;
		else
		{
			sprite += multi;
			inc = 1;
		}

		if (flipscreen)
		{
			y=240-y;
			x=304-x;
			if (fx) fx=0; else fx=1;
			if (fy) fy=0; else fy=1;
			mult=16;
		}
		else mult=-16;

		while (multi >= 0)
		{
			drawgfx(bitmap,machine->gfx[3],
					sprite - multi * inc,
					colour,
					fx,fy,
					sprite_xoffset+x,sprite_yoffset+ y + mult * multi,
					cliprect,TRANSPARENCY_PEN,15);

			multi--;
		}
	}
}

/******************************************************************************/

WRITE16_HANDLER( bcstory_tilebank_w )
{
	bcstory_tilebank = data;
	tilemap_mark_all_tiles_dirty(pf1_tilemap);
	tilemap_mark_all_tiles_dirty(pf1_alt_tilemap);
	tilemap_mark_all_tiles_dirty(pf2_tilemap);
}

WRITE16_HANDLER( chokchok_tilebank_w )
{
	bcstory_tilebank = data<<1;
	tilemap_mark_all_tiles_dirty(pf1_tilemap);
	tilemap_mark_all_tiles_dirty(pf1_alt_tilemap);
	tilemap_mark_all_tiles_dirty(pf2_tilemap);
}

WRITE16_HANDLER( wlstar_tilebank_w )
{
	/* it just writes 0000 or ffff */
	bcstory_tilebank = data&0x4000;
	tilemap_mark_all_tiles_dirty(pf1_tilemap);
	tilemap_mark_all_tiles_dirty(pf1_alt_tilemap);
	tilemap_mark_all_tiles_dirty(pf2_tilemap);
}


WRITE16_HANDLER( suprtrio_tilebank_w )
{
	bcstory_tilebank = data<<14; // shift it here, makes using bcstory_tilebank easier
	tilemap_mark_all_tiles_dirty(pf1_tilemap);
	tilemap_mark_all_tiles_dirty(pf1_alt_tilemap);
	tilemap_mark_all_tiles_dirty(pf2_tilemap);
}





WRITE16_HANDLER( tumblepb_pf1_data_w )
{
	COMBINE_DATA(&tumblepb_pf1_data[offset]);
	tilemap_mark_tile_dirty(pf1_tilemap,offset);
	tilemap_mark_tile_dirty(pf1_alt_tilemap,offset);
}

WRITE16_HANDLER( tumblepb_pf2_data_w )
{
	COMBINE_DATA(&tumblepb_pf2_data[offset]);
	tilemap_mark_tile_dirty(pf2_tilemap,offset);

	if (pf2_alt_tilemap)
		tilemap_mark_tile_dirty(pf2_alt_tilemap,offset);
}

WRITE16_HANDLER( fncywld_pf1_data_w )
{
	COMBINE_DATA(&tumblepb_pf1_data[offset]);
	tilemap_mark_tile_dirty(pf1_tilemap,offset/2);
	tilemap_mark_tile_dirty(pf1_alt_tilemap,offset/2);
}

WRITE16_HANDLER( fncywld_pf2_data_w )
{
	COMBINE_DATA(&tumblepb_pf2_data[offset]);
	tilemap_mark_tile_dirty(pf2_tilemap,offset/2);
}

WRITE16_HANDLER( tumblepb_control_0_w )
{
	COMBINE_DATA(&tumblepb_control_0[offset]);
}

/******************************************************************************/

static TILEMAP_MAPPER( tumblep_scan )
{
	/* logical (col,row) -> memory offset */
	return (col & 0x1f) + ((row & 0x1f) << 5) + ((col & 0x60) << 5);
}

INLINE void get_bg_tile_info(running_machine *machine,tile_data *tileinfo,int tile_index,int gfx_bank,UINT16 *gfx_base)
{
	int data = gfx_base[tile_index];

	SET_TILE_INFO(
			gfx_bank,
			(data & 0x0fff)|(bcstory_tilebank>>2),
			data >> 12,
			0);
}

static TILE_GET_INFO( get_bg1_tile_info ) { get_bg_tile_info(machine,tileinfo,tile_index,2,tumblepb_pf1_data); }
static TILE_GET_INFO( get_bg2_tile_info ) { get_bg_tile_info(machine,tileinfo,tile_index,1,tumblepb_pf2_data); }

static TILE_GET_INFO( get_fg_tile_info )
{
	int data = tumblepb_pf1_data[tile_index];

	SET_TILE_INFO(
			0,
			(data & 0x0fff)|bcstory_tilebank,
			data >> 12,
			0);
}

INLINE void get_fncywld_bg_tile_info(running_machine *machine,tile_data *tileinfo,int tile_index,int gfx_bank,UINT16 *gfx_base)
{
	int data = gfx_base[tile_index*2];
	int attr = gfx_base[tile_index*2+1];

	SET_TILE_INFO(
			gfx_bank,
			data & 0x1fff,
			attr & 0x1f,
			0);
}

static TILE_GET_INFO( get_fncywld_bg1_tile_info ) { get_fncywld_bg_tile_info(machine,tileinfo,tile_index,2,tumblepb_pf1_data); }
static TILE_GET_INFO( get_fncywld_bg2_tile_info ) { get_fncywld_bg_tile_info(machine,tileinfo,tile_index,1,tumblepb_pf2_data); }

static TILE_GET_INFO( get_fncywld_fg_tile_info )
{
	int data = tumblepb_pf1_data[tile_index*2];
	int attr = tumblepb_pf1_data[tile_index*2+1];

	SET_TILE_INFO(
			0,
			data & 0x1fff,
			attr & 0x1f,
			0);
}


/* jump pop */
static TILE_GET_INFO( get_jumppop_bg1_tile_info )
{
	int data = tumblepb_pf1_data[tile_index];

	SET_TILE_INFO(
			2,
			data & 0x3fff,
			0,
			0);
}

static TILE_GET_INFO( get_jumppop_bg2_tile_info )
{
	int data = tumblepb_pf2_data[tile_index];

	SET_TILE_INFO(
			1,
			data & 0x1fff,
			1,
			0);
}

static TILE_GET_INFO( get_jumppop_bg2_alt_tile_info )
{
	int data = tumblepb_pf2_data[tile_index];

	SET_TILE_INFO(
			0,
			data & 0x7fff,
			1,
			0);
}


static TILE_GET_INFO( get_jumppop_fg_tile_info )
{
	int data = tumblepb_pf1_data[tile_index];

	SET_TILE_INFO(
			0,
			data & 0x7fff,
			0,
			0);
}


WRITE16_HANDLER( pangpang_pf1_data_w )
{
	COMBINE_DATA(&tumblepb_pf1_data[offset]);
	tilemap_mark_tile_dirty(pf1_tilemap,offset/2);
	tilemap_mark_tile_dirty(pf1_alt_tilemap,offset/2);
}

WRITE16_HANDLER( pangpang_pf2_data_w )
{
	COMBINE_DATA(&tumblepb_pf2_data[offset]);
	tilemap_mark_tile_dirty(pf2_tilemap,offset/2);

	if (pf2_alt_tilemap)
		tilemap_mark_tile_dirty(pf2_alt_tilemap,offset/2);
}



INLINE void pangpang_get_bg_tile_info(running_machine *machine, tile_data *tileinfo, int tile_index,int gfx_bank,UINT16 *gfx_base)
{
	int data = gfx_base[tile_index*2+1];
	int attr = gfx_base[tile_index*2];

	SET_TILE_INFO(
			gfx_bank,
			data & 0x1fff,
			(attr >>12) & 0xf,
			0);
}

INLINE void pangpang_get_bg2x_tile_info(running_machine *machine, tile_data *tileinfo, int tile_index,int gfx_bank,UINT16 *gfx_base)
{
	int data = gfx_base[tile_index*2+1];
	int attr = gfx_base[tile_index*2];

	SET_TILE_INFO(
			gfx_bank,
			(data & 0xfff)+0x1000,
			(attr >>12) & 0xf,
			0);
}


static TILE_GET_INFO( pangpang_get_bg1_tile_info ) { pangpang_get_bg_tile_info(machine,tileinfo,tile_index,2,tumblepb_pf1_data); }
static TILE_GET_INFO( pangpang_get_bg2_tile_info ) { pangpang_get_bg2x_tile_info(machine,tileinfo,tile_index,1,tumblepb_pf2_data); }

static TILE_GET_INFO( pangpang_get_fg_tile_info )
{
	int data = tumblepb_pf1_data[tile_index*2+1];
	int attr = tumblepb_pf1_data[tile_index*2];

	SET_TILE_INFO(
			0,
			data & 0x1fff,
			(attr >> 12)& 0x1f,
			0);
}


VIDEO_START( pangpang )
{
	pf1_tilemap =     tilemap_create(pangpang_get_fg_tile_info, tilemap_scan_rows,TILEMAP_TYPE_PEN, 8, 8,64,32);
	pf1_alt_tilemap = tilemap_create(pangpang_get_bg1_tile_info,tumblep_scan,TILEMAP_TYPE_PEN,16,16,64,32);
	pf2_tilemap =     tilemap_create(pangpang_get_bg2_tile_info,tumblep_scan,TILEMAP_TYPE_PEN,     16,16,64,32);

	tilemap_set_transparent_pen(pf1_tilemap,0);
	tilemap_set_transparent_pen(pf1_alt_tilemap,0);
	bcstory_tilebank = 0;

	sprite_xoffset = -1;
	sprite_yoffset = 0;
}


VIDEO_START( tumblepb )
{
	pf1_tilemap =     tilemap_create(get_fg_tile_info, tilemap_scan_rows,TILEMAP_TYPE_PEN, 8, 8,64,32);
	pf1_alt_tilemap = tilemap_create(get_bg1_tile_info,tumblep_scan,TILEMAP_TYPE_PEN,16,16,64,32);
	pf2_tilemap =     tilemap_create(get_bg2_tile_info,tumblep_scan,TILEMAP_TYPE_PEN,     16,16,64,32);

	tilemap_set_transparent_pen(pf1_tilemap,0);
	tilemap_set_transparent_pen(pf1_alt_tilemap,0);
	bcstory_tilebank = 0;

	sprite_xoffset = -1;
	sprite_yoffset = 0;
}

VIDEO_START( sdfight )
{
	pf1_tilemap =     tilemap_create(get_fg_tile_info, tilemap_scan_rows,TILEMAP_TYPE_PEN, 8, 8,64,64); // 64*64 to prevent bad tilemap wrapping? - check real behavior
	pf1_alt_tilemap = tilemap_create(get_bg1_tile_info,tumblep_scan,TILEMAP_TYPE_PEN,16,16,64,32);
	pf2_tilemap =     tilemap_create(get_bg2_tile_info,tumblep_scan,TILEMAP_TYPE_PEN,     16,16,64,32);

	tilemap_set_transparent_pen(pf1_tilemap,0);
	tilemap_set_transparent_pen(pf1_alt_tilemap,0);
	bcstory_tilebank = 0;

	/* aligned to monitor test */
	sprite_xoffset = 0;
	sprite_yoffset = 1;
}

VIDEO_START( fncywld )
{
	pf1_tilemap =     tilemap_create(get_fncywld_fg_tile_info, tilemap_scan_rows,TILEMAP_TYPE_PEN, 8, 8,64,32);
	pf1_alt_tilemap = tilemap_create(get_fncywld_bg1_tile_info,tumblep_scan,TILEMAP_TYPE_PEN,16,16,64,32);
	pf2_tilemap =     tilemap_create(get_fncywld_bg2_tile_info,tumblep_scan,TILEMAP_TYPE_PEN,     16,16,64,32);

	tilemap_set_transparent_pen(pf1_tilemap,15);
	tilemap_set_transparent_pen(pf1_alt_tilemap,15);
	bcstory_tilebank = 0;

	sprite_xoffset = -1;
	sprite_yoffset = 0;
}

VIDEO_START( jumppop )
{
	pf1_tilemap =     tilemap_create(get_jumppop_fg_tile_info, tilemap_scan_rows,TILEMAP_TYPE_PEN, 8, 8,128,64);
	pf1_alt_tilemap = tilemap_create(get_jumppop_bg1_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,16,16,64,64);
	pf2_tilemap =     tilemap_create(get_jumppop_bg2_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,     16,16,64,64);
	pf2_alt_tilemap =     tilemap_create(get_jumppop_bg2_alt_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,     8,8,128,64);

	tilemap_set_transparent_pen(pf1_tilemap,0);
	tilemap_set_transparent_pen(pf1_alt_tilemap,0);

	tilemap_set_flip(pf1_tilemap, TILEMAP_FLIPX);
	tilemap_set_flip(pf1_alt_tilemap, TILEMAP_FLIPX);
	tilemap_set_flip(pf2_tilemap, TILEMAP_FLIPX);
	tilemap_set_flip(pf2_alt_tilemap, TILEMAP_FLIPX);
	bcstory_tilebank = 0;

	sprite_xoffset = -1;
	sprite_yoffset = 0;
}


/******************************************************************************/


VIDEO_UPDATE( tumblepb )
{
	int offs,offs2;

	flipscreen=tumblepb_control_0[0]&0x80;
	tilemap_set_flip(ALL_TILEMAPS,flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
	if (flipscreen) offs=1; else offs=-1;
	if (flipscreen) offs2=-3; else offs2=-5;

	tilemap_set_scrollx( pf1_tilemap,0, tumblepb_control_0[1]+offs2 );
	tilemap_set_scrolly( pf1_tilemap,0, tumblepb_control_0[2] );
	tilemap_set_scrollx( pf1_alt_tilemap,0, tumblepb_control_0[1]+offs2 );
	tilemap_set_scrolly( pf1_alt_tilemap,0, tumblepb_control_0[2] );
	tilemap_set_scrollx( pf2_tilemap,0, tumblepb_control_0[3]+offs );
	tilemap_set_scrolly( pf2_tilemap,0, tumblepb_control_0[4] );

	tilemap_draw(bitmap,cliprect,pf2_tilemap,0,0);
	if (tumblepb_control_0[6]&0x80)
		tilemap_draw(bitmap,cliprect,pf1_tilemap,0,0);
	else
		tilemap_draw(bitmap,cliprect,pf1_alt_tilemap,0,0);
	tumblepb_draw_sprites(machine,bitmap,cliprect);
	return 0;
}

VIDEO_UPDATE( jumpkids )
{
	int offs,offs2;

	flipscreen=tumblepb_control_0[0]&0x80;
	tilemap_set_flip(ALL_TILEMAPS,flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
	if (flipscreen) offs=1; else offs=-1;
	if (flipscreen) offs2=-3; else offs2=-5;

	tilemap_set_scrollx( pf1_tilemap,0, tumblepb_control_0[1]+offs2 );
	tilemap_set_scrolly( pf1_tilemap,0, tumblepb_control_0[2] );
	tilemap_set_scrollx( pf1_alt_tilemap,0, tumblepb_control_0[1]+offs2 );
	tilemap_set_scrolly( pf1_alt_tilemap,0, tumblepb_control_0[2] );
	tilemap_set_scrollx( pf2_tilemap,0, tumblepb_control_0[3]+offs );
	tilemap_set_scrolly( pf2_tilemap,0, tumblepb_control_0[4] );

	tilemap_draw(bitmap,cliprect,pf2_tilemap,0,0);
	if (tumblepb_control_0[6]&0x80)
		tilemap_draw(bitmap,cliprect,pf1_tilemap,0,0);
	else
		tilemap_draw(bitmap,cliprect,pf1_alt_tilemap,0,0);
	jumpkids_draw_sprites(machine,bitmap,cliprect);
	return 0;
}

VIDEO_UPDATE( semicom )
{
	int offs,offs2;

	flipscreen=tumblepb_control_0[0]&0x80;
	tilemap_set_flip(ALL_TILEMAPS,flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
	if (flipscreen) offs=1; else offs=-1;	/* fixed */
	if (flipscreen) offs2=-3; else offs2=-5;	/* fixed */

	tilemap_set_scrollx( pf1_tilemap,0, tumblepb_control_0[1]+offs2 );
	tilemap_set_scrolly( pf1_tilemap,0, tumblepb_control_0[2] );
	tilemap_set_scrollx( pf1_alt_tilemap,0, tumblepb_control_0[1]+offs2 );
	tilemap_set_scrolly( pf1_alt_tilemap,0, tumblepb_control_0[2] );
	tilemap_set_scrollx( pf2_tilemap,0, tumblepb_control_0[3]+offs );
	tilemap_set_scrolly( pf2_tilemap,0, tumblepb_control_0[4] );

	tilemap_draw(bitmap,cliprect,pf2_tilemap,0,0);
	if (tumblepb_control_0[6]&0x80)
		tilemap_draw(bitmap,cliprect,pf1_tilemap,0,0);
	else
		tilemap_draw(bitmap,cliprect,pf1_alt_tilemap,0,0);
	jumpkids_draw_sprites(machine,bitmap,cliprect);
	return 0;
}

VIDEO_UPDATE( semicom_altoffsets )
{
	int offsx,offsy,offsx2;

	flipscreen=tumblepb_control_0[0]&0x80;

	offsx = -1;
	offsy = 2;
	offsx2 = -5;

	tilemap_set_scrollx( pf1_tilemap,0, tumblepb_control_0[1]+offsx2 );
	tilemap_set_scrolly( pf1_tilemap,0, tumblepb_control_0[2] );
	tilemap_set_scrollx( pf1_alt_tilemap,0, tumblepb_control_0[1]+offsx2 );
	tilemap_set_scrolly( pf1_alt_tilemap,0, tumblepb_control_0[2] );
	tilemap_set_scrollx( pf2_tilemap,0, tumblepb_control_0[3]+offsx );
	tilemap_set_scrolly( pf2_tilemap,0, tumblepb_control_0[4]+offsy );

	tilemap_draw(bitmap,cliprect,pf2_tilemap,0,0);
	if (tumblepb_control_0[6]&0x80)
		tilemap_draw(bitmap,cliprect,pf1_tilemap,0,0);
	else
		tilemap_draw(bitmap,cliprect,pf1_alt_tilemap,0,0);
	jumpkids_draw_sprites(machine,bitmap,cliprect);
	return 0;
}

VIDEO_UPDATE( bcstory )
{
	int offs,offs2;

	flipscreen=tumblepb_control_0[0]&0x80;
	tilemap_set_flip(ALL_TILEMAPS,flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
	if (flipscreen) offs=1; else offs=8;	/* not sure of this */
	if (flipscreen) offs2=-3; else offs2=8;	/* not sure of this */

	tilemap_set_scrollx( pf1_tilemap,0, tumblepb_control_0[1]+offs2 );
	tilemap_set_scrolly( pf1_tilemap,0, tumblepb_control_0[2] );
	tilemap_set_scrollx( pf1_alt_tilemap,0, tumblepb_control_0[1]+offs2 );
	tilemap_set_scrolly( pf1_alt_tilemap,0, tumblepb_control_0[2] );
	tilemap_set_scrollx( pf2_tilemap,0, tumblepb_control_0[3]+offs );
	tilemap_set_scrolly( pf2_tilemap,0, tumblepb_control_0[4] );

	tilemap_draw(bitmap,cliprect,pf2_tilemap,0,0);
	if (tumblepb_control_0[6]&0x80)
		tilemap_draw(bitmap,cliprect,pf1_tilemap,0,0);
	else
		tilemap_draw(bitmap,cliprect,pf1_alt_tilemap,0,0);
	jumpkids_draw_sprites(machine,bitmap,cliprect);
	return 0;
}

VIDEO_UPDATE( semibase )
{
	int offs,offs2;

	flipscreen=tumblepb_control_0[0]&0x80;
	tilemap_set_flip(ALL_TILEMAPS,flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
	offs=-1;
	offs2=-2;

	/* sprites need an offset too */

	tilemap_set_scrollx( pf1_tilemap,0, tumblepb_control_0[1]+offs2 );
	tilemap_set_scrolly( pf1_tilemap,0, tumblepb_control_0[2] );
	tilemap_set_scrollx( pf1_alt_tilemap,0, tumblepb_control_0[1]+offs2 );
	tilemap_set_scrolly( pf1_alt_tilemap,0, tumblepb_control_0[2] );
	tilemap_set_scrollx( pf2_tilemap,0, tumblepb_control_0[3]+offs );
	tilemap_set_scrolly( pf2_tilemap,0, tumblepb_control_0[4] );

	tilemap_draw(bitmap,cliprect,pf2_tilemap,0,0);
	if (tumblepb_control_0[6]&0x80)
		tilemap_draw(bitmap,cliprect,pf1_tilemap,0,0);
	else
		tilemap_draw(bitmap,cliprect,pf1_alt_tilemap,0,0);
	jumpkids_draw_sprites(machine,bitmap,cliprect);
	return 0;
}

VIDEO_UPDATE( sdfight )
{
	int offs,offs2;

	flipscreen=tumblepb_control_0[0]&0x80;
	tilemap_set_flip(ALL_TILEMAPS,flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
	offs=-1;
	offs2=-5; // foreground scroll..

	/* sprites need an offset too */

	tilemap_set_scrollx( pf1_tilemap,0, tumblepb_control_0[1]+offs2 );
	tilemap_set_scrolly( pf1_tilemap,0, tumblepb_control_0[2]-16 ); // needed for the ground ...
	tilemap_set_scrollx( pf1_alt_tilemap,0, tumblepb_control_0[1]+offs2 );
	tilemap_set_scrolly( pf1_alt_tilemap,0, tumblepb_control_0[2]-16 );
	tilemap_set_scrollx( pf2_tilemap,0, tumblepb_control_0[3]+offs );
	tilemap_set_scrolly( pf2_tilemap,0, tumblepb_control_0[4] );

	tilemap_draw(bitmap,cliprect,pf2_tilemap,0,0);
	if (tumblepb_control_0[6]&0x80)
		tilemap_draw(bitmap,cliprect,pf1_tilemap,0,0);
	else
		tilemap_draw(bitmap,cliprect,pf1_alt_tilemap,0,0);
	jumpkids_draw_sprites(machine,bitmap,cliprect);
	return 0;
}



VIDEO_UPDATE( fncywld )
{
	int offs,offs2;

	flipscreen=tumblepb_control_0[0]&0x80;
	tilemap_set_flip(ALL_TILEMAPS,flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
	if (flipscreen) offs=1; else offs=-1;
	if (flipscreen) offs2=-3; else offs2=-5;

	tilemap_set_scrollx( pf1_tilemap,0, tumblepb_control_0[1]+offs2 );
	tilemap_set_scrolly( pf1_tilemap,0, tumblepb_control_0[2] );
	tilemap_set_scrollx( pf1_alt_tilemap,0, tumblepb_control_0[1]+offs2 );
	tilemap_set_scrolly( pf1_alt_tilemap,0, tumblepb_control_0[2] );
	tilemap_set_scrollx( pf2_tilemap,0, tumblepb_control_0[3]+offs );
	tilemap_set_scrolly( pf2_tilemap,0, tumblepb_control_0[4] );

	tilemap_draw(bitmap,cliprect,pf2_tilemap,0,0);
	if (tumblepb_control_0[6]&0x80)
		tilemap_draw(bitmap,cliprect,pf1_tilemap,0,0);
	else
		tilemap_draw(bitmap,cliprect,pf1_alt_tilemap,0,0);
	fncywld_draw_sprites(machine,bitmap,cliprect);
	return 0;
}


VIDEO_UPDATE( jumppop )
{
//  fillbitmap(bitmap, get_black_pen(machine), cliprect);

	tilemap_set_scrollx( pf1_tilemap,0, jumppop_control[2]-0x3a0 );
	tilemap_set_scrolly( pf1_tilemap,0, jumppop_control[3] );
	tilemap_set_scrollx( pf1_alt_tilemap,0, jumppop_control[2]-0x3a0 );
	tilemap_set_scrolly( pf1_alt_tilemap,0, jumppop_control[3] );
	tilemap_set_scrollx( pf2_tilemap,0, jumppop_control[0]-0x3a2  );
	tilemap_set_scrolly( pf2_tilemap,0, jumppop_control[1] );
	tilemap_set_scrollx( pf2_alt_tilemap,0, jumppop_control[0]-0x3a2 );
	tilemap_set_scrolly( pf2_alt_tilemap,0, jumppop_control[1] );

	if (jumppop_control[7]&1)
		tilemap_draw(bitmap,cliprect,pf2_tilemap,0,0);
	else
		tilemap_draw(bitmap,cliprect,pf2_alt_tilemap,0,0);

	if (jumppop_control[7]&2)
		tilemap_draw(bitmap,cliprect,pf1_alt_tilemap,0,0);
	else
		tilemap_draw(bitmap,cliprect,pf1_tilemap,0,0);

//popmessage("%04x %04x %04x %04x %04x %04x %04x %04x", jumppop_control[0],jumppop_control[1],jumppop_control[2],jumppop_control[3],jumppop_control[4],jumppop_control[5],jumppop_control[6],jumppop_control[7]);

	jumpkids_draw_sprites(machine,bitmap,cliprect);
	return 0;
}


VIDEO_UPDATE( suprtrio )
{
	tilemap_set_scrollx( pf1_alt_tilemap,0, -suprtrio_control[1]-6 );
	tilemap_set_scrolly( pf1_alt_tilemap,0, -suprtrio_control[2] );
	tilemap_set_scrollx( pf2_tilemap,0, -suprtrio_control[3]-2 );
	tilemap_set_scrolly( pf2_tilemap,0, -suprtrio_control[4] );

	tilemap_draw(bitmap,cliprect,pf2_tilemap,0,0);
	tilemap_draw(bitmap,cliprect,pf1_alt_tilemap,0,0);

	jumpkids_draw_sprites(machine,bitmap,cliprect);
#if 0
popmessage("%04x %04x %04x %04x %04x %04x %04x %04x",
 suprtrio_control[0],
 suprtrio_control[1],
 suprtrio_control[2],
 suprtrio_control[3],
 suprtrio_control[4],
 suprtrio_control[5],
 suprtrio_control[6],
 suprtrio_control[7]);
#endif

	return 0;
}

VIDEO_UPDATE( pangpang )
{
	int offs,offs2;

	flipscreen=tumblepb_control_0[0]&0x80;
	tilemap_set_flip(ALL_TILEMAPS,flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
	if (flipscreen) offs=1; else offs=-1;
	if (flipscreen) offs2=-3; else offs2=-5;

	tilemap_set_scrollx( pf1_tilemap,0, tumblepb_control_0[1]+offs2 );
	tilemap_set_scrolly( pf1_tilemap,0, tumblepb_control_0[2] );
	tilemap_set_scrollx( pf1_alt_tilemap,0, tumblepb_control_0[1]+offs2 );
	tilemap_set_scrolly( pf1_alt_tilemap,0, tumblepb_control_0[2] );
	tilemap_set_scrollx( pf2_tilemap,0, tumblepb_control_0[3]+offs );
	tilemap_set_scrolly( pf2_tilemap,0, tumblepb_control_0[4] );

	tilemap_draw(bitmap,cliprect,pf2_tilemap,0,0);
	if (tumblepb_control_0[6]&0x80)
		tilemap_draw(bitmap,cliprect,pf1_tilemap,0,0);
	else
		tilemap_draw(bitmap,cliprect,pf1_alt_tilemap,0,0);
	jumpkids_draw_sprites(machine,bitmap,cliprect);
	return 0;
}

VIDEO_START( suprtrio )
{
	pf1_tilemap =     tilemap_create(get_fg_tile_info, tilemap_scan_rows,TILEMAP_TYPE_PEN, 8, 8,64,32);
	pf1_alt_tilemap = tilemap_create(get_bg1_tile_info,tumblep_scan,TILEMAP_TYPE_PEN,16,16,64,32);
	pf2_tilemap =     tilemap_create(get_bg2_tile_info,tumblep_scan,TILEMAP_TYPE_PEN,     16,16,64,32);

	tilemap_set_transparent_pen(pf1_alt_tilemap,0);
	bcstory_tilebank = 0;
}


