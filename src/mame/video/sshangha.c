/***************************************************************************

    Uses Data East custom chip 55 for backgrounds, with a special 8bpp mode
    2 times custom chips 52/71 for sprites.

***************************************************************************/

#include "emu.h"
#include "includes/sshangha.h"


/******************************************************************************/

static void sshangha_tilemap_draw(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	sshangha_state *state = machine->driver_data<sshangha_state>();
	const bitmap_t *bitmap0 = tilemap_get_pixmap(state->pf1_16x16_tilemap);
	const bitmap_t *bitmap1 = tilemap_get_pixmap(state->pf2_tilemap);
	int x,y,p;

	for (y=0; y<240; y++) {
		for (x=0; x<320; x++) {
			p=*BITMAP_ADDR16(bitmap0, y, x)&0xf;
			p|=(*BITMAP_ADDR16(bitmap1, y, x)&0xf)<<4;

			*BITMAP_ADDR16(bitmap, y, x) = p|0x300;
		}
	}
}

WRITE16_HANDLER (sshangha_video_w)
{
	sshangha_state *state = space->machine->driver_data<sshangha_state>();
	/* 0x4: Special video mode, other bits unknown */
	state->video_control=data;
//  popmessage("%04x",data);
}

/******************************************************************************/

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, UINT16 *spritesrc, UINT16 pmask, UINT16 pval)
{
	int offs;

	for (offs = 0;offs < 0x800;offs += 4)
	{
		int x,y,sprite,colour,multi,fx,fy,inc,flash,mult;

		sprite = spritesrc[offs+1] & 0x3fff;
		if (!sprite) continue;

		if ((spritesrc[offs+2]&pmask)!=pval)
			continue;

		y = spritesrc[offs];
		flash=y&0x1000;
		if (flash && (machine->primary_screen->frame_number() & 1)) continue;

		x = spritesrc[offs+2];
		colour = (x >>9) & 0x1f;

		fx = y & 0x2000;
		fy = y & 0x4000;
		multi = (1 << ((y & 0x0600) >> 9)) - 1;	/* 1x, 2x, 4x, 8x height */

		x = x & 0x01ff;
		y = y & 0x01ff;
		if (x >= 320) x -= 512;
		if (y >= 256) y -= 512;

		sprite &= ~multi;

		if (flip_screen_get(machine))
		{
			y=240-y;
			x=304-x;
			if (fx) fx=0; else fx=1;
			if (fy) fy=0; else fy=1;
			mult=-16;
		}

		if (fy)
			inc = -1;
		else
		{
			sprite += multi;
			inc = 1;
		}

		mult=+16;

		if (fx) fx=0; else fx=1;
		if (fy) fy=0; else fy=1;

		while (multi >= 0)
		{
			drawgfx_transpen(bitmap,cliprect,machine->gfx[2],
					sprite - multi * inc,
					colour,
					fx,fy,
					x,y + mult * multi,0);

			multi--;
		}
	}
}

/******************************************************************************/

WRITE16_HANDLER( sshangha_pf2_data_w )
{
	sshangha_state *state = space->machine->driver_data<sshangha_state>();
	COMBINE_DATA(&state->pf2_data[offset]);
	tilemap_mark_tile_dirty(state->pf2_tilemap,offset);
}

WRITE16_HANDLER( sshangha_pf1_data_w )
{
	sshangha_state *state = space->machine->driver_data<sshangha_state>();
	COMBINE_DATA(&state->pf1_data[offset]);
	tilemap_mark_tile_dirty(state->pf1_8x8_tilemap,offset);
	tilemap_mark_tile_dirty(state->pf1_16x16_tilemap,offset);
}

WRITE16_HANDLER( sshangha_control_0_w )
{
	sshangha_state *state = space->machine->driver_data<sshangha_state>();
	COMBINE_DATA(&state->control_0[offset]);
}

/******************************************************************************/

#if 0
static UINT32 sshangha_scan(UINT32 col,UINT32 row,UINT32 num_cols,UINT32 num_rows)
{
	/* logical (col,row) -> memory offset */
	return (col & 0x1f) + ((row & 0x1f) << 5) + ((col & 0x20) << 5);
}
#endif

static TILE_GET_INFO( get_pf2_tile_info )
{
	sshangha_state *state = machine->driver_data<sshangha_state>();
	UINT16 tile=state->pf2_data[tile_index];
	SET_TILE_INFO(1,(tile&0xfff)|state->pf2_bank,(tile>>12)|32,0);
}

static TILE_GET_INFO( get_pf1_16x16_tile_info )
{
	sshangha_state *state = machine->driver_data<sshangha_state>();
	UINT16 tile=state->pf1_data[tile_index];
	SET_TILE_INFO(1,(tile&0xfff)|state->pf1_bank,tile>>12,0);
}

static TILE_GET_INFO( get_pf1_8x8_tile_info )
{
	sshangha_state *state = machine->driver_data<sshangha_state>();
	UINT16 tile=state->pf1_data[tile_index];
	SET_TILE_INFO(0,(tile&0xfff)|state->pf1_bank,tile>>12,0);
}

VIDEO_START( sshangha )
{
	sshangha_state *state = machine->driver_data<sshangha_state>();
	state->pf1_8x8_tilemap   = tilemap_create(machine, get_pf1_8x8_tile_info,  tilemap_scan_rows, 8, 8,64,32);
	state->pf1_16x16_tilemap = tilemap_create(machine, get_pf1_16x16_tile_info,tilemap_scan_rows,16,16,32,32);
	state->pf2_tilemap = tilemap_create(machine, get_pf2_tile_info,tilemap_scan_rows,         16,16,32,32);

	tilemap_set_transparent_pen(state->pf1_8x8_tilemap,0);
	tilemap_set_transparent_pen(state->pf1_16x16_tilemap,0);
}

/******************************************************************************/

SCREEN_UPDATE( sshangha )
{
	sshangha_state *state = screen->machine->driver_data<sshangha_state>();
	int offs;

	flip_screen_set_no_update(screen->machine, state->control_0[0]&0x80);
	tilemap_set_flip_all(screen->machine,flip_screen_x_get(screen->machine) ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	tilemap_set_enable( state->pf2_tilemap, state->control_0[5]&0x8000);
	tilemap_set_enable( state->pf1_8x8_tilemap, state->control_0[5]&0x0080);
	tilemap_set_enable( state->pf1_16x16_tilemap, state->control_0[5]&0x0080);

	state->pf1_bank=((state->control_0[7]>> 4)&0xf)*0x1000;
	state->pf2_bank=((state->control_0[7]>>12)&0xf)*0x1000;

	if (state->pf1_bank!=state->last_pf1_bank) tilemap_mark_all_tiles_dirty(state->pf1_8x8_tilemap);
	if (state->pf1_bank!=state->last_pf1_bank) tilemap_mark_all_tiles_dirty(state->pf1_16x16_tilemap);
	if (state->pf2_bank!=state->last_pf2_bank) tilemap_mark_all_tiles_dirty(state->pf2_tilemap);
	state->last_pf1_bank=state->pf1_bank;
	state->last_pf2_bank=state->pf2_bank;

	/* Rowscroll - todo, this might actually be col scroll, or both row & col combined.  Check.. */
	if (state->control_0[6]&0x40) {
		tilemap_set_scroll_rows(state->pf1_8x8_tilemap,256);
		tilemap_set_scroll_rows(state->pf1_16x16_tilemap,256);
		for (offs=0; offs<256; offs++) {
			tilemap_set_scrollx( state->pf1_8x8_tilemap,0, state->control_0[1] + state->pf1_rowscroll[offs+0x200] );
			tilemap_set_scrollx( state->pf1_16x16_tilemap,0, state->control_0[1] + state->pf1_rowscroll[offs+0x200] );
		}
	} else {
		tilemap_set_scroll_rows(state->pf1_16x16_tilemap,1);
		tilemap_set_scroll_rows(state->pf1_8x8_tilemap,1);
		tilemap_set_scrollx( state->pf1_8x8_tilemap,0, state->control_0[1] );
		tilemap_set_scrollx( state->pf1_16x16_tilemap,0, state->control_0[1] );
	}

	if (state->control_0[6]&0x4000) {
		tilemap_set_scroll_rows(state->pf2_tilemap,256);
		for (offs=0; offs<256; offs++) {
			tilemap_set_scrollx( state->pf2_tilemap,0, state->control_0[3] - 3 + state->pf2_rowscroll[offs+0x200] );
		}
	} else {
		tilemap_set_scroll_rows(state->pf2_tilemap,1);
		tilemap_set_scrollx( state->pf2_tilemap,0, state->control_0[3] - 3 );
	}

	tilemap_set_scrolly( state->pf2_tilemap,0, state->control_0[4] );
	tilemap_set_scrolly( state->pf1_8x8_tilemap,0, state->control_0[2] );
	tilemap_set_scrolly( state->pf1_16x16_tilemap,0, state->control_0[2] );

	//if ((state->control_0[5]&0x8000)==0) /* <- used on hot-b logo and girl presentation screens */
	bitmap_fill(bitmap,cliprect,get_black_pen(screen->machine));

	/* Super Shanghai has a mode where the two tilemaps are combined to
    produce a 6bpp tilemap.  We can't precompute this as any tiles can be
    used in any tilemap, so we plot it on the fly */
	if ((state->video_control&4)==0) {
		sshangha_tilemap_draw(screen->machine, bitmap, cliprect);
		draw_sprites(screen->machine, bitmap, cliprect, screen->machine->generic.spriteram.u16,0x4000,0x4000);
	}
	else {
		tilemap_draw(bitmap,cliprect,state->pf2_tilemap,0,0);
		draw_sprites(screen->machine, bitmap, cliprect, screen->machine->generic.spriteram.u16,0x4000,0x4000);

		if (state->control_0[6]&0x80)
			tilemap_draw(bitmap,cliprect,state->pf1_8x8_tilemap,0,0);
		else
			tilemap_draw(bitmap,cliprect,state->pf1_16x16_tilemap,0,0);
	}

	draw_sprites(screen->machine, bitmap, cliprect, screen->machine->generic.spriteram2.u16,0x0000,0x0000);
	draw_sprites(screen->machine, bitmap, cliprect, screen->machine->generic.spriteram.u16,0x4000,0x0000);
	return 0;
}
