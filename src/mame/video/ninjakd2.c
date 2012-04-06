#include "emu.h"
#include "includes/ninjakd2.h"


#define TRANSPARENTCODE (15)

/*************************************
 *
 *  Callbacks for the TileMap code
 *
 *************************************/

static TILE_GET_INFO( get_fg_tile_info )
{
	ninjakd2_state *state = machine.driver_data<ninjakd2_state>();
	int const lo = state->m_fg_videoram[(tile_index << 1)];
	int const hi = state->m_fg_videoram[(tile_index << 1) | 1];
	int const tile = ((hi & 0xc0) << 2) | lo;
	int const flipyx = (hi & 0x30) >> 4;
	int const color = hi & 0x0f;

	SET_TILE_INFO(
			0,
			tile,
			color,
			TILE_FLIPYX(flipyx));
}

static TILE_GET_INFO( ninjakd2_get_bg_tile_info )
{
	ninjakd2_state *state = machine.driver_data<ninjakd2_state>();
	int const lo = state->m_bg_videoram[(tile_index << 1)];
	int const hi = state->m_bg_videoram[(tile_index << 1) | 1];
	int const tile = ((hi & 0xc0) << 2) | lo;
	int const flipyx = (hi & 0x30) >> 4;
	int const color = hi & 0x0f;

	SET_TILE_INFO(
			2,
			tile,
			color,
			TILE_FLIPYX(flipyx));
}

static TILE_GET_INFO( mnight_get_bg_tile_info )
{
	ninjakd2_state *state = machine.driver_data<ninjakd2_state>();
	int const lo = state->m_bg_videoram[(tile_index << 1)];
	int const hi = state->m_bg_videoram[(tile_index << 1) | 1];
	int const tile = ((hi & 0x10) << 6) | ((hi & 0xc0) << 2) | lo;
	int const flipy = (hi & 0x20) >> 5;
	int const color = hi & 0x0f;

	SET_TILE_INFO(
			2,
			tile,
			color,
			flipy ? TILE_FLIPY : 0);
}

static TILEMAP_MAPPER( robokid_bg_scan )
{
	/* logical (col,row) -> memory offset */
	return (col & 0x0f) | ((row & 0x1f) << 4) | ((col & 0x10) << 5);
}

static TILEMAP_MAPPER( omegaf_bg_scan )
{
	/* logical (col,row) -> memory offset */
	return (col & 0x0f) | ((row & 0x1f) << 4) | ((col & 0x70) << 5);
}

static void robokid_get_bg_tile_info(running_machine& machine, tile_data& tileinfo, tilemap_memory_index const tile_index, int const gfxnum, const UINT8* const videoram)
{
	int const lo = videoram[(tile_index << 1)];
	int const hi = videoram[(tile_index << 1) | 1];
	int const tile = ((hi & 0x10) << 7) | ((hi & 0x20) << 5) | ((hi & 0xc0) << 2) | lo;
	int const color = hi & 0x0f;

	SET_TILE_INFO(
			gfxnum,
			tile,
			color,
			0);
}

static TILE_GET_INFO( robokid_get_bg0_tile_info )
{
	ninjakd2_state *state = machine.driver_data<ninjakd2_state>();
	robokid_get_bg_tile_info(machine, tileinfo, tile_index, 2, state->m_robokid_bg0_videoram);
}

static TILE_GET_INFO( robokid_get_bg1_tile_info )
{
	ninjakd2_state *state = machine.driver_data<ninjakd2_state>();
	robokid_get_bg_tile_info(machine, tileinfo, tile_index, 3, state->m_robokid_bg1_videoram);
}

static TILE_GET_INFO( robokid_get_bg2_tile_info )
{
	ninjakd2_state *state = machine.driver_data<ninjakd2_state>();
	robokid_get_bg_tile_info(machine, tileinfo, tile_index, 4, state->m_robokid_bg2_videoram);
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

static void videoram_alloc(running_machine& machine, int const size)
{
	ninjakd2_state *state = machine.driver_data<ninjakd2_state>();
	if (size)
	{
		/* create video ram */
		state->m_robokid_bg0_videoram = auto_alloc_array_clear(machine, UINT8, size);
		state->m_robokid_bg1_videoram = auto_alloc_array_clear(machine, UINT8, size);
		state->m_robokid_bg2_videoram = auto_alloc_array_clear(machine, UINT8, size);
	}

	machine.primary_screen->register_screen_bitmap(state->m_sp_bitmap);
}

static int stencil_ninjakd2( UINT16 pal );
static int stencil_mnight(   UINT16 pal );
static int stencil_arkarea(  UINT16 pal );
static int stencil_robokid(  UINT16 pal );
static int stencil_omegaf(   UINT16 pal );

VIDEO_START( ninjakd2 )
{
	ninjakd2_state *state = machine.driver_data<ninjakd2_state>();
	videoram_alloc(machine, 0);

	state->m_fg_tilemap = tilemap_create(         machine, get_fg_tile_info, tilemap_scan_rows,   8,  8, 32, 32);
	state->m_bg_tilemap = tilemap_create(machine, ninjakd2_get_bg_tile_info, tilemap_scan_rows,  16, 16, 32, 32);

	state->m_fg_tilemap->set_transparent_pen(TRANSPARENTCODE);

	state->m_robokid_sprites = 0;
	state->m_stencil_compare_function = stencil_ninjakd2;
}

VIDEO_START( mnight )
{
	ninjakd2_state *state = machine.driver_data<ninjakd2_state>();
	videoram_alloc(machine, 0);

	state->m_fg_tilemap = tilemap_create(       machine, get_fg_tile_info, tilemap_scan_rows,   8,  8, 32, 32);
	state->m_bg_tilemap = tilemap_create(machine, mnight_get_bg_tile_info, tilemap_scan_rows,  16, 16, 32, 32);

	state->m_fg_tilemap->set_transparent_pen(TRANSPARENTCODE);

	state->m_robokid_sprites = 0;
	state->m_stencil_compare_function = stencil_mnight;
}

VIDEO_START( arkarea )
{
	ninjakd2_state *state = machine.driver_data<ninjakd2_state>();
	videoram_alloc(machine, 0);

	state->m_fg_tilemap = tilemap_create(       machine, get_fg_tile_info, tilemap_scan_rows,   8,  8, 32, 32);
	state->m_bg_tilemap = tilemap_create(machine, mnight_get_bg_tile_info, tilemap_scan_rows,  16, 16, 32, 32);

	state->m_fg_tilemap->set_transparent_pen(TRANSPARENTCODE);

	state->m_robokid_sprites = 0;
	state->m_stencil_compare_function = stencil_arkarea;
}

VIDEO_START( robokid )
{
	ninjakd2_state *state = machine.driver_data<ninjakd2_state>();
	state->m_bank_mask = 1;

	videoram_alloc(machine, 0x0800);

	state->m_fg_tilemap  = tilemap_create(        machine, get_fg_tile_info,  tilemap_scan_rows,   8,  8, 32, 32);
	state->m_bg0_tilemap = tilemap_create(machine, robokid_get_bg0_tile_info, robokid_bg_scan,    16, 16, 32, 32);
	state->m_bg1_tilemap = tilemap_create(machine, robokid_get_bg1_tile_info, robokid_bg_scan,    16, 16, 32, 32);
	state->m_bg2_tilemap = tilemap_create(machine, robokid_get_bg2_tile_info, robokid_bg_scan,    16, 16, 32, 32);

	state->m_fg_tilemap->set_transparent_pen(TRANSPARENTCODE);
	state->m_bg1_tilemap->set_transparent_pen(TRANSPARENTCODE);
	state->m_bg2_tilemap->set_transparent_pen(TRANSPARENTCODE);

	state->m_robokid_sprites = 1;
	state->m_stencil_compare_function = stencil_robokid;
}

VIDEO_START( omegaf )
{
	ninjakd2_state *state = machine.driver_data<ninjakd2_state>();
	state->m_bank_mask = 7;

	videoram_alloc(machine, 0x2000);

	state->m_fg_tilemap  = tilemap_create(        machine, get_fg_tile_info,  tilemap_scan_rows,   8,  8,  32, 32);
	state->m_bg0_tilemap = tilemap_create(machine, robokid_get_bg0_tile_info, omegaf_bg_scan,     16, 16, 128, 32);
	state->m_bg1_tilemap = tilemap_create(machine, robokid_get_bg1_tile_info, omegaf_bg_scan,     16, 16, 128, 32);
	state->m_bg2_tilemap = tilemap_create(machine, robokid_get_bg2_tile_info, omegaf_bg_scan,     16, 16, 128, 32);

	state->m_fg_tilemap->set_transparent_pen(TRANSPARENTCODE);
	state->m_bg0_tilemap->set_transparent_pen(TRANSPARENTCODE);
	state->m_bg1_tilemap->set_transparent_pen(TRANSPARENTCODE);
	state->m_bg2_tilemap->set_transparent_pen(TRANSPARENTCODE);

	state->m_robokid_sprites = 1;
	state->m_stencil_compare_function = stencil_omegaf;
}



/*************************************
 *
 *  Memory handlers
 *
 *************************************/

WRITE8_MEMBER(ninjakd2_state::ninjakd2_bgvideoram_w)
{
	m_bg_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset >> 1);
}

WRITE8_MEMBER(ninjakd2_state::ninjakd2_fgvideoram_w)
{
	m_fg_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset >> 1);
}



WRITE8_MEMBER(ninjakd2_state::robokid_bg0_bank_w)
{
	m_robokid_bg0_bank = data & m_bank_mask;
}

WRITE8_MEMBER(ninjakd2_state::robokid_bg1_bank_w)
{
	m_robokid_bg1_bank = data & m_bank_mask;
}

WRITE8_MEMBER(ninjakd2_state::robokid_bg2_bank_w)
{
	m_robokid_bg2_bank = data & m_bank_mask;
}

READ8_MEMBER(ninjakd2_state::robokid_bg0_videoram_r)
{
	return m_robokid_bg0_videoram[(m_robokid_bg0_bank << 10) | offset];
}

READ8_MEMBER(ninjakd2_state::robokid_bg1_videoram_r)
{
	return m_robokid_bg1_videoram[(m_robokid_bg1_bank << 10) | offset];
}

READ8_MEMBER(ninjakd2_state::robokid_bg2_videoram_r)
{
	return m_robokid_bg2_videoram[(m_robokid_bg2_bank << 10) | offset];
}

WRITE8_MEMBER(ninjakd2_state::robokid_bg0_videoram_w)
{
	int const address = (m_robokid_bg0_bank << 10 ) | offset;

	m_robokid_bg0_videoram[address] = data;
	m_bg0_tilemap->mark_tile_dirty(address >> 1);
}

WRITE8_MEMBER(ninjakd2_state::robokid_bg1_videoram_w)
{
	int const address = (m_robokid_bg1_bank << 10 ) | offset;

	m_robokid_bg1_videoram[address] = data;
	m_bg1_tilemap->mark_tile_dirty(address >> 1);
}

WRITE8_MEMBER(ninjakd2_state::robokid_bg2_videoram_w)
{
	int const address = (m_robokid_bg2_bank << 10 ) | offset;

	m_robokid_bg2_videoram[address] = data;
	m_bg2_tilemap->mark_tile_dirty(address >> 1);
}



static void bg_ctrl(int offset, int data, tilemap_t* tilemap)
{
	int scrollx = tilemap->scrollx(0);
	int scrolly = tilemap->scrolly(0);

	switch (offset)
	{
		case 0:	scrollx = ((scrollx & 0x100) | data);        break;
		case 1:	scrollx = ((scrollx & 0x0ff) | (data << 8)); break;
		case 2:	scrolly = ((scrolly & 0x100) | data);        break;
		case 3:	scrolly = ((scrolly & 0x0ff) | (data << 8)); break;
		case 4:	tilemap->enable(data & 1);       break;
	}

	tilemap->set_scrollx(0, scrollx);
	tilemap->set_scrolly(0, scrolly);
}

WRITE8_MEMBER(ninjakd2_state::ninjakd2_bg_ctrl_w)
{
	bg_ctrl(offset, data, m_bg_tilemap);
}

WRITE8_MEMBER(ninjakd2_state::robokid_bg0_ctrl_w)
{
	bg_ctrl(offset, data, m_bg0_tilemap);
}

WRITE8_MEMBER(ninjakd2_state::robokid_bg1_ctrl_w)
{
	bg_ctrl(offset, data, m_bg1_tilemap);
}

WRITE8_MEMBER(ninjakd2_state::robokid_bg2_ctrl_w)
{
	bg_ctrl(offset, data, m_bg2_tilemap);
}



WRITE8_MEMBER(ninjakd2_state::ninjakd2_sprite_overdraw_w)
{
	m_next_sprite_overdraw_enabled = data & 1;
}



/*************************************
 *
 *  Video update
 *
 *************************************/

static void draw_sprites(running_machine& machine, bitmap_ind16 &bitmap)
{
	ninjakd2_state *state = machine.driver_data<ninjakd2_state>();
	const gfx_element* const gfx = machine.gfx[1];
	int const big_xshift = state->m_robokid_sprites ? 1 : 0;
	int const big_yshift = state->m_robokid_sprites ? 0 : 1;

	UINT8* sprptr = &state->m_spriteram[11];
	int sprites_drawn = 0;

	// the sprite generator draws exactly 96 16x16 sprites per frame. When big
	// (32x32) sprites are drawn, this counts for 4 sprites drawn, so the sprite
	// list is reduced accordingly (i.e. three slots at the end of the list will
	// be ignored). Note that a disabled sprite, even if it is not drawn, still
	// counts as one sprite drawn.
	// This is proven by Mutant Night, which doesn't work correctly (leaves shots
	// on screen) if we don't take big sprites into account.

	for (;;)
	{
		if (sprptr[2] & 0x02)
		{
			int sx = sprptr[1] - ((sprptr[2] & 0x01) << 8);
			int sy = sprptr[0];
			// Ninja Kid II doesn't use the topmost bit (it has smaller ROMs) so it might not be connected on the board
			int code = sprptr[3] + ((sprptr[2] & 0xc0) << 2) + ((sprptr[2] & 0x08) << 7);
			int flipx = (sprptr[2] & 0x10) >> 4;
			int flipy = (sprptr[2] & 0x20) >> 5;
			int const color = sprptr[4] & 0x0f;
			// Ninja Kid II doesn't use the 'big' feature so it might not be available on the board
			int const big = (sprptr[2] & 0x04) >> 2;

			if (flip_screen_get(machine))
			{
				sx = 240 - 16*big - sx;
				sy = 240 - 16*big - sy;
				flipx ^= 1;
				flipy ^= 1;
			}

			if (big)
			{
				code &= ~3;
				code ^= flipx << big_xshift;
				code ^= flipy << big_yshift;
			}

			for (int y = 0; y <= big; ++y)
			{
				for (int x = 0; x <= big; ++x)
				{
					int const tile = code ^ (x << big_xshift) ^ (y << big_yshift);

					drawgfx_transpen(bitmap, bitmap.cliprect(), gfx,
							tile,
							color,
							flipx,flipy,
							sx + 16*x, sy + 16*y, TRANSPARENTCODE);

					++sprites_drawn;
					if (sprites_drawn >= 96)
						return;
				}
			}
		}
		else
		{
			++sprites_drawn;
			if (sprites_drawn >= 96)
				return;
		}

		sprptr += 16;
	}
}

static int stencil_ninjakd2( UINT16 pal ) { return( (pal & 0xf0) == 0xf0 ); }
static int stencil_mnight(   UINT16 pal ) { return( (pal & 0xf0) == 0xf0 ); }
static int stencil_arkarea(  UINT16 pal ) { return( (pal & 0xf0) == 0xf0 ); }
static int stencil_robokid(  UINT16 pal ) { return( (pal & 0xf0) <  0xe0 ); }
static int stencil_omegaf(   UINT16 pal ) { return( TRUE ); }
//////            OVERDRAW     STENCIL     UNKNOWN
//////  NINJAKD2  023459ABCDE  F           1678
//////    MNIGHT  0134568ABCDE F           279
//////   ARKAREA  012345679BDE             8ACF
//////   ROBOKID  EF           01236       45789ABCD
//////    OMEGAF  -            -           -         (unused)
// I could not find a port to select overdraw or stencil.
// Temporarily, I compare with constant number.
// This is very hackish.
// (Is there a possibility that software can't select it but hardware can?)

static void erase_sprites(running_machine& machine, bitmap_ind16 &bitmap)
{
	ninjakd2_state *state = machine.driver_data<ninjakd2_state>();
	// if sprite overdraw is disabled, clear the sprite framebuffer
	if (!state->m_next_sprite_overdraw_enabled)
		state->m_sp_bitmap.fill(TRANSPARENTCODE);
	else
		for (int y = 0; y < state->m_sp_bitmap.height(); ++y)
		{
			for (int x = 0; x < state->m_sp_bitmap.width(); ++x)
			{
				UINT16* const ptr = &state->m_sp_bitmap.pix16(y, x);

				if ( (*state->m_stencil_compare_function)(*ptr) ) *ptr = TRANSPARENTCODE ;
			}
		}
}


static void update_sprites(running_machine& machine)
{
	ninjakd2_state *state = machine.driver_data<ninjakd2_state>();
	erase_sprites(machine, state->m_sp_bitmap);
	draw_sprites(machine, state->m_sp_bitmap);
}
	////// Before modified, this was written.
		// we want to erase the sprites with the old setting and draw them with the
		// new one. Not doing this causes a glitch in Ninja Kid II when taking the top
		// exit from stage 3.
	////// The glitch is correct behavior.


SCREEN_UPDATE_IND16( ninjakd2 )
{
	ninjakd2_state *state = screen.machine().driver_data<ninjakd2_state>();
	// updating sprites here instead than in screen_eof avoids a palette glitch
	// at the end of the "rainbow sky" screens.
	update_sprites(screen.machine());
	state->m_sprites_updated = 1;

	bitmap.fill(0, cliprect);

	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);

	copybitmap_trans(bitmap, state->m_sp_bitmap, 0, 0, 0, 0, cliprect, TRANSPARENTCODE);

	state->m_fg_tilemap->draw(bitmap, cliprect, 0, 0);

	return 0;
}

SCREEN_UPDATE_IND16( robokid )
{
	ninjakd2_state *state = screen.machine().driver_data<ninjakd2_state>();
	update_sprites(screen.machine());
	state->m_sprites_updated = 1;

	bitmap.fill(0, cliprect);

	state->m_bg0_tilemap->draw(bitmap, cliprect, 0, 0);

	state->m_bg1_tilemap->draw(bitmap, cliprect, 0, 0);

	copybitmap_trans(bitmap, state->m_sp_bitmap, 0, 0, 0, 0, cliprect, TRANSPARENTCODE);

	state->m_bg2_tilemap->draw(bitmap, cliprect, 0, 0);

	state->m_fg_tilemap->draw(bitmap, cliprect, 0, 0);

	return 0;
}

SCREEN_UPDATE_IND16( omegaf )
{
	ninjakd2_state *state = screen.machine().driver_data<ninjakd2_state>();
	update_sprites(screen.machine());
	state->m_sprites_updated = 1;

	bitmap.fill(0, cliprect);

	state->m_bg0_tilemap->draw(bitmap, cliprect, 0, 0);

	state->m_bg1_tilemap->draw(bitmap, cliprect, 0, 0);

	state->m_bg2_tilemap->draw(bitmap, cliprect, 0, 0);

	copybitmap_trans(bitmap, state->m_sp_bitmap, 0, 0, 0, 0, cliprect, TRANSPARENTCODE);

	state->m_fg_tilemap->draw(bitmap, cliprect, 0, 0);

	return 0;
}


SCREEN_VBLANK( ninjakd2 )
{
	// rising edge
	if (vblank_on)
	{
		ninjakd2_state *state = screen.machine().driver_data<ninjakd2_state>();
		if (!state->m_sprites_updated)
			update_sprites(screen.machine());

		state->m_sprites_updated = 0;
	}
}
