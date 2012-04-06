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

#include "emu.h"
#include "includes/tumbleb.h"
#include "video/decospr.h"

/******************************************************************************/


/******************************************************************************/

WRITE16_MEMBER(tumbleb_state::bcstory_tilebank_w)
{

	m_tilebank = data;
	m_pf1_tilemap->mark_all_dirty();
	m_pf1_alt_tilemap->mark_all_dirty();
	m_pf2_tilemap->mark_all_dirty();
}

WRITE16_MEMBER(tumbleb_state::chokchok_tilebank_w)
{

	m_tilebank = data << 1;
	m_pf1_tilemap->mark_all_dirty();
	m_pf1_alt_tilemap->mark_all_dirty();
	m_pf2_tilemap->mark_all_dirty();
}

WRITE16_MEMBER(tumbleb_state::wlstar_tilebank_w)
{

	/* it just writes 0000 or ffff */
	m_tilebank = data & 0x4000;
	m_pf1_tilemap->mark_all_dirty();
	m_pf1_alt_tilemap->mark_all_dirty();
	m_pf2_tilemap->mark_all_dirty();
}


WRITE16_MEMBER(tumbleb_state::suprtrio_tilebank_w)
{

	m_tilebank = data << 14; // shift it here, makes using bcstory_tilebank easier
	m_pf1_tilemap->mark_all_dirty();
	m_pf1_alt_tilemap->mark_all_dirty();
	m_pf2_tilemap->mark_all_dirty();
}


WRITE16_MEMBER(tumbleb_state::tumblepb_pf1_data_w)
{

	COMBINE_DATA(&m_pf1_data[offset]);
	m_pf1_tilemap->mark_tile_dirty(offset);
	m_pf1_alt_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(tumbleb_state::tumblepb_pf2_data_w)
{

	COMBINE_DATA(&m_pf2_data[offset]);
	m_pf2_tilemap->mark_tile_dirty(offset);

	if (m_pf2_alt_tilemap)
		m_pf2_alt_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(tumbleb_state::fncywld_pf1_data_w)
{

	COMBINE_DATA(&m_pf1_data[offset]);
	m_pf1_tilemap->mark_tile_dirty(offset / 2);
	m_pf1_alt_tilemap->mark_tile_dirty(offset / 2);
}

WRITE16_MEMBER(tumbleb_state::fncywld_pf2_data_w)
{

	COMBINE_DATA(&m_pf2_data[offset]);
	m_pf2_tilemap->mark_tile_dirty(offset / 2);
}

WRITE16_MEMBER(tumbleb_state::tumblepb_control_0_w)
{
	COMBINE_DATA(&m_control_0[offset]);
}


WRITE16_MEMBER(tumbleb_state::pangpang_pf1_data_w)
{

	COMBINE_DATA(&m_pf1_data[offset]);
	m_pf1_tilemap->mark_tile_dirty(offset / 2);
	m_pf1_alt_tilemap->mark_tile_dirty(offset / 2);
}

WRITE16_MEMBER(tumbleb_state::pangpang_pf2_data_w)
{

	COMBINE_DATA(&m_pf2_data[offset]);
	m_pf2_tilemap->mark_tile_dirty(offset / 2);

	if (m_pf2_alt_tilemap)
		m_pf2_alt_tilemap->mark_tile_dirty(offset / 2);
}

/******************************************************************************/

static TILEMAP_MAPPER( tumblep_scan )
{
	/* logical (col,row) -> memory offset */
	return (col & 0x1f) + ((row & 0x1f) << 5) + ((col & 0x60) << 5);
}

INLINE void get_bg_tile_info( running_machine &machine, tile_data &tileinfo, int tile_index, int gfx_bank, UINT16 *gfx_base)
{
	tumbleb_state *state = machine.driver_data<tumbleb_state>();
	int data = gfx_base[tile_index];

	SET_TILE_INFO(
			gfx_bank,
			(data & 0x0fff) | (state->m_tilebank >> 2),
			data >> 12,
			0);
}

static TILE_GET_INFO( get_bg1_tile_info ) { tumbleb_state *state = machine.driver_data<tumbleb_state>();	get_bg_tile_info(machine, tileinfo, tile_index, 2, state->m_pf1_data); }
static TILE_GET_INFO( get_bg2_tile_info ) { tumbleb_state *state = machine.driver_data<tumbleb_state>();	get_bg_tile_info(machine, tileinfo, tile_index, 1, state->m_pf2_data); }

static TILE_GET_INFO( get_fg_tile_info )
{
	tumbleb_state *state = machine.driver_data<tumbleb_state>();
	int data = state->m_pf1_data[tile_index];

	SET_TILE_INFO(
			0,
			(data & 0x0fff) | state->m_tilebank,
			data >> 12,
			0);
}

INLINE void get_fncywld_bg_tile_info( running_machine &machine, tile_data &tileinfo, int tile_index, int gfx_bank, UINT16 *gfx_base)
{
	int data = gfx_base[tile_index * 2];
	int attr = gfx_base[tile_index * 2 + 1];

	SET_TILE_INFO(
			gfx_bank,
			data & 0x1fff,
			attr & 0x1f,
			0);
}

static TILE_GET_INFO( get_fncywld_bg1_tile_info ) { tumbleb_state *state = machine.driver_data<tumbleb_state>();	get_fncywld_bg_tile_info(machine, tileinfo, tile_index, 2, state->m_pf1_data); }
static TILE_GET_INFO( get_fncywld_bg2_tile_info ) { tumbleb_state *state = machine.driver_data<tumbleb_state>();	get_fncywld_bg_tile_info(machine, tileinfo, tile_index, 1, state->m_pf2_data); }

static TILE_GET_INFO( get_fncywld_fg_tile_info )
{
	tumbleb_state *state = machine.driver_data<tumbleb_state>();
	int data = state->m_pf1_data[tile_index * 2];
	int attr = state->m_pf1_data[tile_index * 2 + 1];

	SET_TILE_INFO(
			0,
			data & 0x1fff,
			attr & 0x1f,
			0);
}


/* jump pop */
static TILE_GET_INFO( get_jumppop_bg1_tile_info )
{
	tumbleb_state *state = machine.driver_data<tumbleb_state>();
	int data = state->m_pf1_data[tile_index];

	SET_TILE_INFO(
			2,
			data & 0x3fff,
			0,
			0);
}

static TILE_GET_INFO( get_jumppop_bg2_tile_info )
{
	tumbleb_state *state = machine.driver_data<tumbleb_state>();
	int data = state->m_pf2_data[tile_index];

	SET_TILE_INFO(
			1,
			data & 0x1fff,
			1,
			0);
}

static TILE_GET_INFO( get_jumppop_bg2_alt_tile_info )
{
	tumbleb_state *state = machine.driver_data<tumbleb_state>();
	int data = state->m_pf2_data[tile_index];

	SET_TILE_INFO(
			0,
			data & 0x7fff,
			1,
			0);
}


static TILE_GET_INFO( get_jumppop_fg_tile_info )
{
	tumbleb_state *state = machine.driver_data<tumbleb_state>();
	int data = state->m_pf1_data[tile_index];

	SET_TILE_INFO(
			0,
			data & 0x7fff,
			0,
			0);
}

INLINE void pangpang_get_bg_tile_info( running_machine &machine, tile_data &tileinfo, int tile_index, int gfx_bank, UINT16 *gfx_base )
{
	int data = gfx_base[tile_index * 2 + 1];
	int attr = gfx_base[tile_index * 2];

	SET_TILE_INFO(
			gfx_bank,
			data & 0x1fff,
			(attr >>12) & 0xf,
			0);
}

INLINE void pangpang_get_bg2x_tile_info( running_machine &machine, tile_data &tileinfo, int tile_index, int gfx_bank, UINT16 *gfx_base )
{
	int data = gfx_base[tile_index * 2 + 1];
	int attr = gfx_base[tile_index * 2];

	SET_TILE_INFO(
			gfx_bank,
			(data & 0xfff) + 0x1000,
			(attr >>12) & 0xf,
			0);
}


static TILE_GET_INFO( pangpang_get_bg1_tile_info ) { tumbleb_state *state = machine.driver_data<tumbleb_state>();	pangpang_get_bg_tile_info(machine, tileinfo, tile_index, 2, state->m_pf1_data); }
static TILE_GET_INFO( pangpang_get_bg2_tile_info ) { tumbleb_state *state = machine.driver_data<tumbleb_state>();	pangpang_get_bg2x_tile_info(machine, tileinfo, tile_index, 1, state->m_pf2_data); }

static TILE_GET_INFO( pangpang_get_fg_tile_info )
{
	tumbleb_state *state = machine.driver_data<tumbleb_state>();
	int data = state->m_pf1_data[tile_index * 2 + 1];
	int attr = state->m_pf1_data[tile_index * 2];

	SET_TILE_INFO(
			0,
			data & 0x1fff,
			(attr >> 12)& 0x1f,
			0);
}


static void tumbleb_tilemap_redraw(running_machine &machine)
{
	tumbleb_state *state = machine.driver_data<tumbleb_state>();

	state->m_pf1_tilemap->mark_all_dirty();
	state->m_pf1_alt_tilemap->mark_all_dirty();
	state->m_pf2_tilemap->mark_all_dirty();
	if (state->m_pf2_alt_tilemap)
		state->m_pf2_alt_tilemap->mark_all_dirty();
}

VIDEO_START( pangpang )
{
	tumbleb_state *state = machine.driver_data<tumbleb_state>();

	state->m_pf1_tilemap =     tilemap_create(machine, pangpang_get_fg_tile_info,  tilemap_scan_rows, 8,  8, 64, 32);
	state->m_pf1_alt_tilemap = tilemap_create(machine, pangpang_get_bg1_tile_info, tumblep_scan,     16, 16, 64, 32);
	state->m_pf2_tilemap =     tilemap_create(machine, pangpang_get_bg2_tile_info, tumblep_scan,     16, 16, 64, 32);

	state->m_pf1_tilemap->set_transparent_pen(0);
	state->m_pf1_alt_tilemap->set_transparent_pen(0);

	machine.save().register_postload(save_prepost_delegate(FUNC(tumbleb_tilemap_redraw), &machine));
}


VIDEO_START( tumblepb )
{
	tumbleb_state *state = machine.driver_data<tumbleb_state>();

	state->m_pf1_tilemap =     tilemap_create(machine, get_fg_tile_info,  tilemap_scan_rows, 8,  8, 64, 32);
	state->m_pf1_alt_tilemap = tilemap_create(machine, get_bg1_tile_info, tumblep_scan,     16, 16, 64, 32);
	state->m_pf2_tilemap =     tilemap_create(machine, get_bg2_tile_info, tumblep_scan,     16, 16, 64, 32);

	state->m_pf1_tilemap->set_transparent_pen(0);
	state->m_pf1_alt_tilemap->set_transparent_pen(0);

	machine.save().register_postload(save_prepost_delegate(FUNC(tumbleb_tilemap_redraw), &machine));
}

VIDEO_START( sdfight )
{
	tumbleb_state *state = machine.driver_data<tumbleb_state>();

	state->m_pf1_tilemap =     tilemap_create(machine, get_fg_tile_info,  tilemap_scan_rows, 8,  8, 64, 64); // 64*64 to prevent bad tilemap wrapping? - check real behavior
	state->m_pf1_alt_tilemap = tilemap_create(machine, get_bg1_tile_info, tumblep_scan,     16, 16, 64, 32);
	state->m_pf2_tilemap =     tilemap_create(machine, get_bg2_tile_info, tumblep_scan,     16, 16, 64, 32);

	state->m_pf1_tilemap->set_transparent_pen(0);
	state->m_pf1_alt_tilemap->set_transparent_pen(0);

	machine.save().register_postload(save_prepost_delegate(FUNC(tumbleb_tilemap_redraw), &machine));
}

VIDEO_START( fncywld )
{
	tumbleb_state *state = machine.driver_data<tumbleb_state>();

	state->m_pf1_tilemap =     tilemap_create(machine, get_fncywld_fg_tile_info,  tilemap_scan_rows, 8,  8, 64, 32);
	state->m_pf1_alt_tilemap = tilemap_create(machine, get_fncywld_bg1_tile_info, tumblep_scan,     16, 16, 64, 32);
	state->m_pf2_tilemap =     tilemap_create(machine, get_fncywld_bg2_tile_info, tumblep_scan,     16, 16, 64, 32);

	state->m_pf1_tilemap->set_transparent_pen(15);
	state->m_pf1_alt_tilemap->set_transparent_pen(15);

	machine.save().register_postload(save_prepost_delegate(FUNC(tumbleb_tilemap_redraw), &machine));
}

VIDEO_START( jumppop )
{
	tumbleb_state *state = machine.driver_data<tumbleb_state>();

	state->m_pf1_tilemap =     tilemap_create(machine, get_jumppop_fg_tile_info,      tilemap_scan_rows,     8,  8, 128, 64);
	state->m_pf1_alt_tilemap = tilemap_create(machine, get_jumppop_bg1_tile_info,     tilemap_scan_rows,    16, 16,  64, 64);
	state->m_pf2_tilemap =     tilemap_create(machine, get_jumppop_bg2_tile_info,     tilemap_scan_rows,    16, 16,  64, 64);
	state->m_pf2_alt_tilemap = tilemap_create(machine, get_jumppop_bg2_alt_tile_info, tilemap_scan_rows,     8,  8, 128, 64);

	state->m_pf1_tilemap->set_transparent_pen(0);
	state->m_pf1_alt_tilemap->set_transparent_pen(0);

	state->m_pf1_tilemap->set_flip(TILEMAP_FLIPX);
	state->m_pf1_alt_tilemap->set_flip(TILEMAP_FLIPX);
	state->m_pf2_tilemap->set_flip(TILEMAP_FLIPX);
	state->m_pf2_alt_tilemap->set_flip(TILEMAP_FLIPX);

	machine.save().register_postload(save_prepost_delegate(FUNC(tumbleb_tilemap_redraw), &machine));
}


VIDEO_START( suprtrio )
{
	tumbleb_state *state = machine.driver_data<tumbleb_state>();

	state->m_pf1_tilemap =     tilemap_create(machine, get_fg_tile_info,  tilemap_scan_rows, 8,  8, 64, 32);
	state->m_pf1_alt_tilemap = tilemap_create(machine, get_bg1_tile_info, tumblep_scan,     16, 16, 64, 32);
	state->m_pf2_tilemap =     tilemap_create(machine, get_bg2_tile_info, tumblep_scan,     16, 16, 64, 32);

	state->m_pf1_alt_tilemap->set_transparent_pen(0);

	machine.save().register_postload(save_prepost_delegate(FUNC(tumbleb_tilemap_redraw), &machine));
}

/******************************************************************************/

void tumbleb_draw_common(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int pf1x_offs, int pf1y_offs, int pf2x_offs, int pf2y_offs)
{
	tumbleb_state *state = machine.driver_data<tumbleb_state>();

	state->m_pf1_tilemap->set_scrollx(0, state->m_control_0[1] + pf1x_offs);
	state->m_pf1_tilemap->set_scrolly(0, state->m_control_0[2] + pf1y_offs);
	state->m_pf1_alt_tilemap->set_scrollx(0, state->m_control_0[1] + pf1x_offs);
	state->m_pf1_alt_tilemap->set_scrolly(0, state->m_control_0[2] + pf1y_offs);
	state->m_pf2_tilemap->set_scrollx(0, state->m_control_0[3] + pf2x_offs);
	state->m_pf2_tilemap->set_scrolly(0, state->m_control_0[4] + pf2y_offs);

	state->m_pf2_tilemap->draw(bitmap, cliprect, 0, 0);

	if (state->m_control_0[6] & 0x80)
		state->m_pf1_tilemap->draw(bitmap, cliprect, 0, 0);
	else
		state->m_pf1_alt_tilemap->draw(bitmap, cliprect, 0, 0);

	machine.device<decospr_device>("spritegen")->draw_sprites(bitmap, cliprect, state->m_spriteram, state->m_spriteram_size/2);
}

SCREEN_UPDATE_IND16( tumblepb )
{
	tumbleb_state *state = screen.machine().driver_data<tumbleb_state>();
	int offs, offs2;

	state->m_flipscreen = state->m_control_0[0] & 0x80;
	screen.machine().tilemap().set_flip_all(state->m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	if (state->m_flipscreen)
		offs = 1;
	else
		offs = -1;

	if (state->m_flipscreen)
		offs2 = -3;
	else
		offs2 = -5;

	tumbleb_draw_common(screen.machine(),bitmap,cliprect, offs2, 0, offs, 0);

	return 0;
}

SCREEN_UPDATE_IND16( jumpkids )
{
	tumbleb_state *state = screen.machine().driver_data<tumbleb_state>();
	int offs, offs2;

	state->m_flipscreen = state->m_control_0[0] & 0x80;
	screen.machine().tilemap().set_flip_all(state->m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	if (state->m_flipscreen)
		offs = 1;
	else
		offs = -1;

	if (state->m_flipscreen)
		offs2 = -3;
	else
		offs2 = -5;

	tumbleb_draw_common(screen.machine(),bitmap,cliprect, offs2, 0, offs, 0);
	return 0;
}

SCREEN_UPDATE_IND16( semicom )
{
	tumbleb_state *state = screen.machine().driver_data<tumbleb_state>();
	int offs, offs2;

	state->m_flipscreen = state->m_control_0[0] & 0x80;
	screen.machine().tilemap().set_flip_all(state->m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	if (state->m_flipscreen)
		offs = 1;
	else
		offs = -1;

	if (state->m_flipscreen)
		offs2 = -3;
	else
		offs2 = -5;

	tumbleb_draw_common(screen.machine(),bitmap,cliprect, offs2, 0, offs, 0);
	return 0;
}

SCREEN_UPDATE_IND16( semicom_altoffsets )
{
	tumbleb_state *state = screen.machine().driver_data<tumbleb_state>();
	int offsx, offsy, offsx2;

	state->m_flipscreen = state->m_control_0[0] & 0x80;

	offsx = -1;
	offsy = 2;
	offsx2 = -5;

	tumbleb_draw_common(screen.machine(),bitmap,cliprect, offsx2, 0, offsx, offsy);

	return 0;
}

SCREEN_UPDATE_IND16( bcstory )
{
	tumbleb_state *state = screen.machine().driver_data<tumbleb_state>();
	int offs, offs2;

	state->m_flipscreen = state->m_control_0[0] & 0x80;
	screen.machine().tilemap().set_flip_all(state->m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	/* not sure of this */
	if (state->m_flipscreen)
		offs = 1;
	else
		offs = 8;

	/* not sure of this */
	if (state->m_flipscreen)
		offs2 = -3;
	else
		offs2 = 8;

	tumbleb_draw_common(screen.machine(),bitmap,cliprect, offs2, 0, offs, 0);
	return 0;
}

SCREEN_UPDATE_IND16( semibase )
{
	tumbleb_state *state = screen.machine().driver_data<tumbleb_state>();
	int offs, offs2;

	state->m_flipscreen = state->m_control_0[0] & 0x80;
	screen.machine().tilemap().set_flip_all(state->m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
	offs = -1;
	offs2 = -2;

	tumbleb_draw_common(screen.machine(),bitmap,cliprect, offs2, 0, offs, 0);

	return 0;
}

SCREEN_UPDATE_IND16( sdfight )
{
	tumbleb_state *state = screen.machine().driver_data<tumbleb_state>();
	int offs, offs2;

	state->m_flipscreen = state->m_control_0[0] & 0x80;
	screen.machine().tilemap().set_flip_all(state->m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
	offs = -1;
	offs2 = -5; // foreground scroll..

	tumbleb_draw_common(screen.machine(),bitmap,cliprect, offs2, -16, offs, 0);

	screen.machine().device<decospr_device>("spritegen")->draw_sprites(bitmap, cliprect, state->m_spriteram, state->m_spriteram_size/2);
	return 0;
}

SCREEN_UPDATE_IND16( fncywld )
{
	tumbleb_state *state = screen.machine().driver_data<tumbleb_state>();
	int offs, offs2;

	state->m_flipscreen = state->m_control_0[0] & 0x80;
	screen.machine().tilemap().set_flip_all(state->m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	if (state->m_flipscreen)
		offs = 1;
	else
		offs = -1;

	if (state->m_flipscreen)
		offs2 = -3;
	else
		offs2 = -5;

	tumbleb_draw_common(screen.machine(),bitmap,cliprect, offs2, 0, offs, 0);

	return 0;
}

SCREEN_UPDATE_IND16( pangpang )
{
	tumbleb_state *state = screen.machine().driver_data<tumbleb_state>();
	int offs, offs2;

	state->m_flipscreen = state->m_control_0[0] & 0x80;
	screen.machine().tilemap().set_flip_all(state->m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	if (state->m_flipscreen)
		offs = 1;
	else
		offs = -1;

	if (state->m_flipscreen)
		offs2 = -3;
	else
		offs2 = -5;

	tumbleb_draw_common(screen.machine(),bitmap,cliprect, offs2, 0, offs, 0);
	return 0;
}



SCREEN_UPDATE_IND16( jumppop )
{
	tumbleb_state *state = screen.machine().driver_data<tumbleb_state>();

	state->m_pf1_tilemap->set_scrollx(0, state->m_control[2] - 0x3a0);
	state->m_pf1_tilemap->set_scrolly(0, state->m_control[3]);
	state->m_pf1_alt_tilemap->set_scrollx(0, state->m_control[2] - 0x3a0);
	state->m_pf1_alt_tilemap->set_scrolly(0, state->m_control[3]);
	state->m_pf2_tilemap->set_scrollx(0, state->m_control[0] - 0x3a2);
	state->m_pf2_tilemap->set_scrolly(0, state->m_control[1]);
	state->m_pf2_alt_tilemap->set_scrollx(0, state->m_control[0] - 0x3a2);
	state->m_pf2_alt_tilemap->set_scrolly(0, state->m_control[1]);

	if (state->m_control[7] & 1)
		state->m_pf2_tilemap->draw(bitmap, cliprect, 0, 0);
	else
		state->m_pf2_alt_tilemap->draw(bitmap, cliprect, 0, 0);

	if (state->m_control[7] & 2)
		state->m_pf1_alt_tilemap->draw(bitmap, cliprect, 0, 0);
	else
		state->m_pf1_tilemap->draw(bitmap, cliprect, 0, 0);

	screen.machine().device<decospr_device>("spritegen")->draw_sprites(bitmap, cliprect, state->m_spriteram, state->m_spriteram_size/2);

	return 0;
}


SCREEN_UPDATE_IND16( suprtrio )
{
	tumbleb_state *state = screen.machine().driver_data<tumbleb_state>();

	state->m_pf1_alt_tilemap->set_scrollx(0, -state->m_control[1] - 6);
	state->m_pf1_alt_tilemap->set_scrolly(0, -state->m_control[2]);
	state->m_pf2_tilemap->set_scrollx(0, -state->m_control[3] - 2);
	state->m_pf2_tilemap->set_scrolly(0, -state->m_control[4]);

	state->m_pf2_tilemap->draw(bitmap, cliprect, 0, 0);
	state->m_pf1_alt_tilemap->draw(bitmap, cliprect, 0, 0);

	screen.machine().device<decospr_device>("spritegen")->draw_sprites(bitmap, cliprect, state->m_spriteram, state->m_spriteram_size/2);
	return 0;
}

