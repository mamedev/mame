/* notes...

 drawing sprites in a single pass with pdrawgfx breaks Thunder Dragon 2,
  which seems to expect the sprite priority values to affect sprite-sprite
  priority.  Thunder Dragon 2 also breaks if you support sprite flipping,
  the collectible point score / power up names appear flipped..

*/

#include "emu.h"
#include "includes/nmk16.h"

// the larger tilemaps on macross2, rapid hero and thunder dragon 2 appear to act like 4 'banks'
// of the smaller tilemaps, rather than being able to scroll into each other (not verified on real hw,
// but see raphero intro / 1st level cases)


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

#define TILES_PER_PAGE_X	(0x10)
#define TILES_PER_PAGE_Y	(0x10)
#define PAGES_PER_TMAP_X	(0x10)
#define PAGES_PER_TMAP_Y	(0x02)

static TILEMAP_MAPPER( afega_tilemap_scan_pages )
{
	return	(row / TILES_PER_PAGE_Y) * TILES_PER_PAGE_X * TILES_PER_PAGE_Y * PAGES_PER_TMAP_X +
			(row % TILES_PER_PAGE_Y) +

			(col / TILES_PER_PAGE_X) * TILES_PER_PAGE_X * TILES_PER_PAGE_Y +
			(col % TILES_PER_PAGE_X) * TILES_PER_PAGE_Y;
}

static TILE_GET_INFO( macross_get_bg0_tile_info )
{
	nmk16_state *state = machine.driver_data<nmk16_state>();
	int code = state->m_nmk_bgvideoram0[tile_index];
	SET_TILE_INFO(1,(code & 0xfff) + (state->m_bgbank << 12),code >> 12,0);
}

static TILE_GET_INFO( macross_get_bg1_tile_info )
{
	nmk16_state *state = machine.driver_data<nmk16_state>();
	int code = state->m_nmk_bgvideoram1[tile_index];
	SET_TILE_INFO(1,(code & 0xfff) + (state->m_bgbank << 12),code >> 12,0);
}

static TILE_GET_INFO( macross_get_bg2_tile_info )
{
	nmk16_state *state = machine.driver_data<nmk16_state>();
	int code = state->m_nmk_bgvideoram2[tile_index];
	SET_TILE_INFO(1,(code & 0xfff) + (state->m_bgbank << 12),code >> 12,0);
}

static TILE_GET_INFO( macross_get_bg3_tile_info )
{
	nmk16_state *state = machine.driver_data<nmk16_state>();
	int code = state->m_nmk_bgvideoram3[tile_index];
	SET_TILE_INFO(1,(code & 0xfff) + (state->m_bgbank << 12),code >> 12,0);
}


static TILE_GET_INFO( strahl_get_fg_tile_info )
{
	nmk16_state *state = machine.driver_data<nmk16_state>();
	int code = state->m_nmk_fgvideoram[tile_index];
	SET_TILE_INFO(
			3,
			(code & 0xfff),
			code >> 12,
			0);
}

static TILE_GET_INFO( macross_get_tx_tile_info )
{
	nmk16_state *state = machine.driver_data<nmk16_state>();
	int code = state->m_nmk_txvideoram[tile_index];
	SET_TILE_INFO(
			0,
			code & 0xfff,
			code >> 12,
			0);
}

static TILE_GET_INFO( bjtwin_get_bg_tile_info )
{
	nmk16_state *state = machine.driver_data<nmk16_state>();
	int code = state->m_nmk_bgvideoram0[tile_index];
	int bank = (code & 0x800) ? 1 : 0;
	SET_TILE_INFO(
			bank,
			(code & 0x7ff) + ((bank) ? (state->m_bgbank << 11) : 0),
			code >> 12,
			0);
}

static TILE_GET_INFO( get_tile_info_0_8bit )
{
	nmk16_state *state = machine.driver_data<nmk16_state>();
	UINT16 code = state->m_nmk_bgvideoram0[tile_index];
	SET_TILE_INFO(
			1,
			code,
			0,
			0);
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

static void nmk16_video_init(running_machine &machine)
{
	nmk16_state *state = machine.driver_data<nmk16_state>();
	state->m_spriteram_old = auto_alloc_array_clear(machine, UINT16, 0x1000/2);
	state->m_spriteram_old2 = auto_alloc_array_clear(machine, UINT16, 0x1000/2);

	state->m_videoshift = 0;		/* 256x224 screen, no shift */
	state->m_background_bitmap = NULL;
	state->m_simple_scroll = 1;
}


VIDEO_START( bioship )
{
	nmk16_state *state = machine.driver_data<nmk16_state>();
	state->m_bg_tilemap0 = tilemap_create(machine, macross_get_bg0_tile_info, afega_tilemap_scan_pages,16,16,TILES_PER_PAGE_X*PAGES_PER_TMAP_X,TILES_PER_PAGE_Y*PAGES_PER_TMAP_Y);
	state->m_tx_tilemap = tilemap_create(machine, macross_get_tx_tile_info,tilemap_scan_cols,8,8,32,32);

	state->m_bg_tilemap0->set_transparent_pen(15);
	state->m_tx_tilemap->set_transparent_pen(15);

	nmk16_video_init(machine);
	state->m_background_bitmap = auto_bitmap_ind16_alloc(machine,8192,512);
	state->m_bioship_background_bank=0;
	state->m_redraw_bitmap = 1;

}

VIDEO_START( strahl )
{
	nmk16_state *state = machine.driver_data<nmk16_state>();
	state->m_bg_tilemap0 = tilemap_create(machine, macross_get_bg0_tile_info, afega_tilemap_scan_pages,16,16,TILES_PER_PAGE_X*PAGES_PER_TMAP_X,TILES_PER_PAGE_Y*PAGES_PER_TMAP_Y);
	state->m_fg_tilemap = tilemap_create(machine, strahl_get_fg_tile_info, afega_tilemap_scan_pages,16,16,TILES_PER_PAGE_X*PAGES_PER_TMAP_X,TILES_PER_PAGE_Y*PAGES_PER_TMAP_Y);
	state->m_tx_tilemap = tilemap_create(machine, macross_get_tx_tile_info,tilemap_scan_cols,8,8,32,32);

	state->m_fg_tilemap->set_transparent_pen(15);
	state->m_tx_tilemap->set_transparent_pen(15);

	nmk16_video_init(machine);
}

VIDEO_START( macross )
{
	nmk16_state *state = machine.driver_data<nmk16_state>();
	state->m_bg_tilemap0 = tilemap_create(machine, macross_get_bg0_tile_info, afega_tilemap_scan_pages,16,16,TILES_PER_PAGE_X*PAGES_PER_TMAP_X,TILES_PER_PAGE_Y*PAGES_PER_TMAP_Y);
	state->m_tx_tilemap = tilemap_create(machine, macross_get_tx_tile_info,tilemap_scan_cols,8,8,32,32);

	state->m_tx_tilemap->set_transparent_pen(15);

	nmk16_video_init(machine);
}

VIDEO_START( gunnail )
{
	nmk16_state *state = machine.driver_data<nmk16_state>();
	state->m_bg_tilemap0 = tilemap_create(machine, macross_get_bg0_tile_info, afega_tilemap_scan_pages,16,16,TILES_PER_PAGE_X*PAGES_PER_TMAP_X,TILES_PER_PAGE_Y*PAGES_PER_TMAP_Y);
	state->m_tx_tilemap = tilemap_create(machine, macross_get_tx_tile_info,tilemap_scan_cols,8,8,64,32);

	state->m_tx_tilemap->set_transparent_pen(15);
	state->m_bg_tilemap0->set_scroll_rows(512);

	nmk16_video_init(machine);
	state->m_videoshift = 64;	/* 384x224 screen, leftmost 64 pixels have to be retrieved */
						/* from the other side of the tilemap (!) */
	state->m_simple_scroll = 0;
}

VIDEO_START( macross2 )
{
	nmk16_state *state = machine.driver_data<nmk16_state>();
	state->m_bg_tilemap0 = tilemap_create(machine, macross_get_bg0_tile_info, afega_tilemap_scan_pages,16,16,TILES_PER_PAGE_X*PAGES_PER_TMAP_X,TILES_PER_PAGE_Y*PAGES_PER_TMAP_Y);
	state->m_bg_tilemap1 = tilemap_create(machine, macross_get_bg1_tile_info, afega_tilemap_scan_pages,16,16,TILES_PER_PAGE_X*PAGES_PER_TMAP_X,TILES_PER_PAGE_Y*PAGES_PER_TMAP_Y);
	state->m_bg_tilemap2 = tilemap_create(machine, macross_get_bg2_tile_info, afega_tilemap_scan_pages,16,16,TILES_PER_PAGE_X*PAGES_PER_TMAP_X,TILES_PER_PAGE_Y*PAGES_PER_TMAP_Y);
	state->m_bg_tilemap3 = tilemap_create(machine, macross_get_bg3_tile_info, afega_tilemap_scan_pages,16,16,TILES_PER_PAGE_X*PAGES_PER_TMAP_X,TILES_PER_PAGE_Y*PAGES_PER_TMAP_Y);

	state->m_tx_tilemap = tilemap_create(machine, macross_get_tx_tile_info,tilemap_scan_cols,8,8,64,32);

	state->m_tx_tilemap->set_transparent_pen(15);

	nmk16_video_init(machine);
	state->m_videoshift = 64;	/* 384x224 screen, leftmost 64 pixels have to be retrieved */
						/* from the other side of the tilemap (!) */
}

VIDEO_START( raphero )
{
	nmk16_state *state = machine.driver_data<nmk16_state>();
	VIDEO_START_CALL( macross2 );
	state->m_simple_scroll = 0;
}

VIDEO_START( bjtwin )
{
	nmk16_state *state = machine.driver_data<nmk16_state>();
	state->m_bg_tilemap0 = tilemap_create(machine, bjtwin_get_bg_tile_info,tilemap_scan_cols,8,8,64,32);

	nmk16_video_init(machine);
	state->m_videoshift = 64;	/* 384x224 screen, leftmost 64 pixels have to be retrieved */
						/* from the other side of the tilemap (!) */
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE16_MEMBER(nmk16_state::nmk_bgvideoram0_w)
{
	COMBINE_DATA(&m_nmk_bgvideoram0[offset]);
	m_bg_tilemap0->mark_tile_dirty(offset);
}

WRITE16_MEMBER(nmk16_state::nmk_bgvideoram1_w)
{
	COMBINE_DATA(&m_nmk_bgvideoram1[offset]);
	m_bg_tilemap1->mark_tile_dirty(offset);
}

WRITE16_MEMBER(nmk16_state::nmk_bgvideoram2_w)
{
	COMBINE_DATA(&m_nmk_bgvideoram2[offset]);
	m_bg_tilemap2->mark_tile_dirty(offset);
}

WRITE16_MEMBER(nmk16_state::nmk_bgvideoram3_w)
{
	COMBINE_DATA(&m_nmk_bgvideoram3[offset]);
	m_bg_tilemap3->mark_tile_dirty(offset);
}

WRITE16_MEMBER(nmk16_state::nmk_fgvideoram_w)
{
	COMBINE_DATA(&m_nmk_fgvideoram[offset]);
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(nmk16_state::nmk_txvideoram_w)
{
	COMBINE_DATA(&m_nmk_txvideoram[offset]);
	m_tx_tilemap->mark_tile_dirty(offset);
}


WRITE16_MEMBER(nmk16_state::mustang_scroll_w)
{
//  mame_printf_debug("mustang %04x %04x %04x\n",offset,data,mem_mask);

	switch (data & 0xff00)
	{
		case 0x0000:
			m_mustang_bg_xscroll = (m_mustang_bg_xscroll & 0x00ff) | ((data & 0x00ff)<<8);
			break;

		case 0x0100:
			m_mustang_bg_xscroll = (m_mustang_bg_xscroll & 0xff00) | (data & 0x00ff);
			break;

		case 0x0200:
			break;

		case 0x0300:
			break;

		default:
			break;
	}

	m_bg_tilemap0->set_scrollx(0,m_mustang_bg_xscroll - m_videoshift);
}

WRITE16_MEMBER(nmk16_state::bioshipbg_scroll_w)
{

	if (ACCESSING_BITS_8_15)
	{
		m_scroll[offset] = (data >> 8) & 0xff;

		if (offset & 2)
			m_bg_tilemap0->set_scrolly(0,m_scroll[2] * 256 + m_scroll[3]);
		else
			m_bg_tilemap0->set_scrollx(0,m_scroll[0] * 256 + m_scroll[1] - m_videoshift);
	}
}

WRITE16_MEMBER(nmk16_state::nmk_scroll_w)
{
	if (ACCESSING_BITS_0_7)
	{

		m_scroll[offset] = data & 0xff;

		if (offset & 2)
			m_bg_tilemap0->set_scrolly(0,m_scroll[2] * 256 + m_scroll[3]);
		else
			m_bg_tilemap0->set_scrollx(0,m_scroll[0] * 256 + m_scroll[1] - m_videoshift);
	}
}

WRITE16_MEMBER(nmk16_state::nmk_scroll_2_w)
{
	if (ACCESSING_BITS_0_7)
	{

		m_scroll_2[offset] = data & 0xff;

		if (offset & 2)
			m_fg_tilemap->set_scrolly(0,m_scroll_2[2] * 256 + m_scroll_2[3]);
		else
			m_fg_tilemap->set_scrollx(0,m_scroll_2[0] * 256 + m_scroll_2[1] - m_videoshift);
	}
}

WRITE16_MEMBER(nmk16_state::vandyke_scroll_w)
{

	m_vscroll[offset] = data;

	m_bg_tilemap0->set_scrollx(0,m_vscroll[0] * 256 + (m_vscroll[1] >> 8));
	m_bg_tilemap0->set_scrolly(0,m_vscroll[2] * 256 + (m_vscroll[3] >> 8));
}

WRITE16_MEMBER(nmk16_state::vandykeb_scroll_w)
{

	switch (offset)
	{
	case 0: COMBINE_DATA(&m_vscroll[3]); break;
	case 1: COMBINE_DATA(&m_vscroll[2]); break;
	case 5: COMBINE_DATA(&m_vscroll[1]); break;
	case 6: COMBINE_DATA(&m_vscroll[0]); break;
	}

	m_bg_tilemap0->set_scrollx(0,m_vscroll[0] * 256 + (m_vscroll[1] >> 8));
	m_bg_tilemap0->set_scrolly(0,m_vscroll[2] * 256 + (m_vscroll[3] >> 8));
}

WRITE16_MEMBER(nmk16_state::manybloc_scroll_w)
{
	COMBINE_DATA(&m_gunnail_scrollram[offset]);

	m_bg_tilemap0->set_scrollx(0,m_gunnail_scrollram[0x82/2]-m_videoshift);
	m_bg_tilemap0->set_scrolly(0,m_gunnail_scrollram[0xc2/2]);
}

WRITE16_MEMBER(nmk16_state::nmk_flipscreen_w)
{
	if (ACCESSING_BITS_0_7)
		flip_screen_set(data & 0x01);
}

WRITE16_MEMBER(nmk16_state::nmk_tilebank_w)
{
	if (ACCESSING_BITS_0_7)
	{
		if (m_bgbank != (data & 0xff))
		{
			m_bgbank = data & 0xff;
			m_bg_tilemap0->mark_all_dirty();
		}
	}
}

WRITE16_MEMBER(nmk16_state::bioship_scroll_w)
{
	if (ACCESSING_BITS_8_15)
		m_bioship_scroll[offset]=data>>8;
}

WRITE16_MEMBER(nmk16_state::bioship_bank_w)
{
	if (ACCESSING_BITS_0_7)
	{
		if (m_bioship_background_bank != data)
		{
			m_bioship_background_bank = data;
			m_redraw_bitmap=1;
		}
	}
}

/***************************************************************************

  Display refresh

***************************************************************************/



// manybloc uses extra flip bits on the sprites, but these break other games

static inline void nmk16_draw_sprite(nmk16_state *state, running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int priority, UINT16 *spr)
{
  if ((spr[0] & 0x0001))
  {
    int sx    = (spr[4] & 0x1FF) + state->m_videoshift;
    int sy    =  spr[6] & 0x1FF;
    int code  =  spr[3];
    int color =  spr[7];
    int w     =  spr[1] & 0x00F;
    int h     = (spr[1] & 0x0F0) >> 4;
    int pri   = (spr[0] & 0x0C0) >> 6;
    int xx,yy,x;
    int delta = 16;

    if(pri != priority)
      return;

    if (state->flip_screen())
    {
      sx = 368 - sx;
      sy = 240 - sy;
      delta = -16;
    }

    yy = h;
    do
    {
      x = sx;
      xx = w;
      do
      {
        drawgfx_transpen(bitmap,cliprect,machine.gfx[2],
            code,
            color,
            state->flip_screen(), state->flip_screen(),
            ((x + 16) & 0x1FF) - 16,sy & 0x1FF,15);
        code++;
        x += delta;
      } while (--xx >= 0);

      sy += delta;
    } while (--yy >= 0);
  }
}

static inline void nmk16_draw_sprite_flipsupported(nmk16_state *state, running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int priority, UINT16 *spr)
{
  if ((spr[0] & 0x0001))
  {
    int sx    = (spr[4] & 0x1FF) + state->m_videoshift;
    int sy    =  spr[6] & 0x1FF;
    int code  =  spr[3];
    int color =  spr[7];
    int w     =  spr[1] & 0x00F;
    int h     = (spr[1] & 0x0F0) >> 4;
    int pri   = (spr[0] & 0x0C0) >> 6;
    int flipy = (spr[1] & 0x200) >> 9;
    int flipx = (spr[1] & 0x100) >> 8;

    int xx,yy,x;
    int delta = 16;

    if(pri != priority)
      return;

    flipx ^= state->flip_screen();
    flipy ^= state->flip_screen();

    if (state->flip_screen())
    {
      sx = 368 - sx;
      sy = 240 - sy;
      delta = -16;
    }

    yy = h;
    sy += flipy ? (delta*h) : 0;
    do
    {
      x = sx + (flipx ? (delta*w) : 0);
      xx = w;
      do
      {
        drawgfx_transpen(bitmap,cliprect,machine.gfx[2],
            code,
            color,
            flipx, flipy,
            ((x + 16) & 0x1FF) - 16,sy & 0x1FF,15);
        code++;
        x += delta * (flipx ? -1 : 1);
      } while (--xx >= 0);
      sy += delta * (flipy ? -1 : 1);
    } while (--yy >= 0);
  }
}

static void nmk16_draw_sprites_swap(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int *bittbl)
{
	nmk16_state *state = machine.driver_data<nmk16_state>();
	int i, prio;

	// Priority level (4 levels, 0x100 sprites)
    for ( prio = 3; prio >= 0; prio-- )
    {
      // Sprite
		for ( i = 0; i < 0x100; i++ )
      {
			int spr = BITSWAP8(i, bittbl[0], bittbl[1], bittbl[2], bittbl[3], bittbl[4], bittbl[5], bittbl[6], bittbl[7]);
			nmk16_draw_sprite(state, machine, bitmap, cliprect, prio, state->m_spriteram_old2 + (spr * 8));
    }
	}
}

static void nmk16_draw_sprites_swap_flipsupported(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int *bittbl)
{
	nmk16_state *state = machine.driver_data<nmk16_state>();
	int i, prio;

	// Priority level (4 levels, 0x100 sprites)
    for ( prio = 3; prio >= 0; prio-- )
    {
      // Sprite
		for ( i = 0; i < 0x100; i++ )
      {
			int spr = BITSWAP8(i, bittbl[0], bittbl[1], bittbl[2], bittbl[3], bittbl[4], bittbl[5], bittbl[6], bittbl[7]);
			nmk16_draw_sprite_flipsupported(state, machine, bitmap, cliprect, prio, state->m_spriteram_old2 + (spr * 16/2));
    }
	}
}

static void nmk16_draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int priority)
{
	nmk16_state *state = machine.driver_data<nmk16_state>();
	int offs;

	for (offs = 0;offs < 0x1000/2;offs += 8)
	{
		nmk16_draw_sprite(state, machine, bitmap, cliprect, priority, state->m_spriteram_old2 + offs);
	}
}

static void nmk16_draw_sprites_flipsupported(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int priority)
{
	nmk16_state *state = machine.driver_data<nmk16_state>();
	int offs;

	for (offs = 0;offs < 0x1000/2;offs += 8)
	{
		if (state->m_spriteram_old2[offs] & 0x0001)
		{
			nmk16_draw_sprite_flipsupported(state, machine, bitmap, cliprect, priority, state->m_spriteram_old2 + offs);
		}
	}
}

/***************************************************************************


                            Generic Screen Updates


***************************************************************************/

int nmk16_bg_spr_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	nmk16_state *state = screen.machine().driver_data<nmk16_state>();
	state->m_bg_tilemap0->set_scrollx(0,-state->m_videoshift);

	state->m_bg_tilemap0->draw(bitmap, cliprect, 0,0);

	nmk16_draw_sprites(screen.machine(), bitmap,cliprect,3);
	nmk16_draw_sprites(screen.machine(), bitmap,cliprect,2);
	nmk16_draw_sprites(screen.machine(), bitmap,cliprect,1);
	nmk16_draw_sprites(screen.machine(), bitmap,cliprect,0);
	return 0;
}

int nmk16_bg_fg_spr_tx_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	nmk16_state *state = screen.machine().driver_data<nmk16_state>();
	state->m_tx_tilemap->set_scrollx(0,-state->m_videoshift);

	state->m_bg_tilemap0->draw(bitmap, cliprect, 0,0);
	state->m_fg_tilemap->draw(bitmap, cliprect, 0,0);

	nmk16_draw_sprites(screen.machine(), bitmap,cliprect,3);
	nmk16_draw_sprites(screen.machine(), bitmap,cliprect,2);
	nmk16_draw_sprites(screen.machine(), bitmap,cliprect,1);
	nmk16_draw_sprites(screen.machine(), bitmap,cliprect,0);

	state->m_tx_tilemap->draw(bitmap, cliprect, 0,0);
	return 0;
}

int nmk16_bg_spr_tx_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	nmk16_state *state = screen.machine().driver_data<nmk16_state>();
	state->m_tx_tilemap->set_scrollx(0,-state->m_videoshift);

	state->m_bg_tilemap0->draw(bitmap, cliprect, 0,0);

	nmk16_draw_sprites(screen.machine(), bitmap,cliprect,3);
    nmk16_draw_sprites(screen.machine(), bitmap,cliprect,2);
    nmk16_draw_sprites(screen.machine(), bitmap,cliprect,1);
	nmk16_draw_sprites(screen.machine(), bitmap,cliprect,0);

	state->m_tx_tilemap->draw(bitmap, cliprect, 0,0);
	return 0;
}

int nmk16_bg_sprflip_tx_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	nmk16_state *state = screen.machine().driver_data<nmk16_state>();
	state->m_tx_tilemap->set_scrollx(0,-state->m_videoshift);

	state->m_bg_tilemap0->draw(bitmap, cliprect, 0,0);

	nmk16_draw_sprites_flipsupported(screen.machine(), bitmap,cliprect,3);
	nmk16_draw_sprites_flipsupported(screen.machine(), bitmap,cliprect,2);
	nmk16_draw_sprites_flipsupported(screen.machine(), bitmap,cliprect,1);
	nmk16_draw_sprites_flipsupported(screen.machine(), bitmap,cliprect,0);

	state->m_tx_tilemap->draw(bitmap, cliprect, 0,0);
	return 0;
}

int nmk16_bioshipbg_sprflip_tx_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	nmk16_state *state = screen.machine().driver_data<nmk16_state>();
	UINT16 *tilerom = (UINT16 *)state->memregion("gfx5")->base();
	int scrollx=-(state->m_bioship_scroll[1] + state->m_bioship_scroll[0]*256);
	int scrolly=-(state->m_bioship_scroll[3] + state->m_bioship_scroll[2]*256);

	state->m_tx_tilemap->set_scrollx(0,-state->m_videoshift);

	if (state->m_redraw_bitmap)
	{
		int bank = state->m_bioship_background_bank * 0x2000;
		int sx=0, sy=0, offs;
		state->m_redraw_bitmap=0;

		/* Draw background from tile rom */
		for (offs = 0;offs <0x1000;offs++) {
				UINT16 data = tilerom[offs+bank];
				int numtile = data&0xfff;
				int color = (data&0xf000)>>12;

				drawgfx_opaque(*state->m_background_bitmap,state->m_background_bitmap->cliprect(),screen.machine().gfx[3],
						numtile,
						color,
						0,0,   /* no flip */
						16*sx,16*sy);

				data = tilerom[offs+0x1000+bank];
				numtile = data&0xfff;
				color = (data&0xf000)>>12;
				drawgfx_opaque(*state->m_background_bitmap,state->m_background_bitmap->cliprect(),screen.machine().gfx[3],
						numtile,
						color,
						0,0,   /* no flip */
						16*sx,(16*sy)+256);

				sy++;
				if (sy==16) {sy=0; sx++;}
		}
	}

	copyscrollbitmap(bitmap,*state->m_background_bitmap,1,&scrollx,1,&scrolly,cliprect);
	state->m_bg_tilemap0->draw(bitmap, cliprect, 0,0);

	nmk16_draw_sprites(screen.machine(), bitmap,cliprect,3);
	nmk16_draw_sprites(screen.machine(), bitmap,cliprect,2);
	nmk16_draw_sprites(screen.machine(), bitmap,cliprect,1);
	nmk16_draw_sprites(screen.machine(), bitmap,cliprect,0);

	state->m_tx_tilemap->draw(bitmap, cliprect, 0,0);
	return 0;
}

int nmk16_bg_sprswap_tx_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int bittbl[8])
{
	nmk16_state *state = screen.machine().driver_data<nmk16_state>();
	state->m_tx_tilemap->set_scrollx(0,-state->m_videoshift);

	state->m_bg_tilemap0->draw(bitmap, cliprect, 0,0);

	nmk16_draw_sprites_swap(screen.machine(), bitmap,cliprect, bittbl);

	state->m_tx_tilemap->draw(bitmap, cliprect, 0,0);
	return 0;
}

int nmk16_bg_sprswapflip_tx_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int bittbl[8])
{
	nmk16_state *state = screen.machine().driver_data<nmk16_state>();
	state->m_tx_tilemap->set_scrollx(0,-state->m_videoshift);

	state->m_bg_tilemap0->draw(bitmap, cliprect, 0,0);

	nmk16_draw_sprites_swap_flipsupported(screen.machine(), bitmap,cliprect, bittbl);

	state->m_tx_tilemap->draw(bitmap, cliprect, 0,0);
	return 0;
}

int nmk16_complexbg_sprswap_tx_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int bittbl[8])
{
	nmk16_state *state = screen.machine().driver_data<nmk16_state>();
	state->m_tx_tilemap->set_scrollx(0,-state->m_videoshift);

	// the hardware supports per-scanline X *and* Y scroll which isn't
	// supported by tilemaps so we have to draw the tilemap one line at a time
	if (!state->m_simple_scroll)
	{
    int i=16;
    int y1;
    rectangle bgclip = cliprect;
    y1 = cliprect.min_y;
		while (y1 <= cliprect.max_y)
		{
			int const yscroll = state->m_gunnail_scrollramy[0] + state->m_gunnail_scrollramy[y1];
			int tilemap_bank_select;
			tilemap_t* bg_tilemap = state->m_bg_tilemap0;

			bgclip.min_y = y1;
			bgclip.max_y = y1;


			tilemap_bank_select = (state->m_gunnail_scrollram[0]&0x3000)>>12;
			switch (tilemap_bank_select)
			{
				case 0: if (state->m_bg_tilemap0) bg_tilemap = state->m_bg_tilemap0; break;
				case 1: if (state->m_bg_tilemap1) bg_tilemap = state->m_bg_tilemap1; break;
				case 2: if (state->m_bg_tilemap2) bg_tilemap = state->m_bg_tilemap2; break;
				case 3: if (state->m_bg_tilemap3) bg_tilemap = state->m_bg_tilemap3; break;
			}

			bg_tilemap->set_scroll_rows(512);

			bg_tilemap->set_scrolly(0, yscroll);
			bg_tilemap->set_scrollx((i + yscroll) & 0x1ff, state->m_gunnail_scrollram[0] + state->m_gunnail_scrollram[i] - state->m_videoshift);

			bg_tilemap->draw(bitmap, bgclip, 0,0);

			y1++;
			i++;
		}
	}
	else
	{
		UINT16 yscroll = ((state->m_gunnail_scrollram[2]&0xff)<<8) | ((state->m_gunnail_scrollram[3]&0xff)<<0);
		UINT16 xscroll = ((state->m_gunnail_scrollram[0]&0xff)<<8) | ((state->m_gunnail_scrollram[1]&0xff)<<0);
		int tilemap_bank_select;
		tilemap_t* bg_tilemap = state->m_bg_tilemap0;

		//popmessage( "scroll %04x, %04x", yscroll,xscroll);

		tilemap_bank_select = (xscroll&0x3000)>>12;
		switch (tilemap_bank_select)
		{
			case 0: if (state->m_bg_tilemap0) bg_tilemap = state->m_bg_tilemap0; break;
			case 1: if (state->m_bg_tilemap1) bg_tilemap = state->m_bg_tilemap1; break;
			case 2: if (state->m_bg_tilemap2) bg_tilemap = state->m_bg_tilemap2; break;
			case 3: if (state->m_bg_tilemap3) bg_tilemap = state->m_bg_tilemap3; break;
		}

		bg_tilemap->set_scroll_rows(1);

		bg_tilemap->set_scrolly(0, yscroll);
		bg_tilemap->set_scrollx(0, xscroll - state->m_videoshift);

		bg_tilemap->draw(bitmap, cliprect, 0,0);
	}

	nmk16_draw_sprites_swap(screen.machine(), bitmap,cliprect, bittbl);

	state->m_tx_tilemap->draw(bitmap, cliprect, 0,0);
	return 0;
}

/***************************************************************************


                            Screen update functions


***************************************************************************/

SCREEN_UPDATE_IND16( macross )
{
  return nmk16_bg_spr_tx_update(screen, bitmap, cliprect);
}

SCREEN_UPDATE_IND16( manybloc )
{
  return nmk16_bg_sprflip_tx_update(screen, bitmap, cliprect);
}

SCREEN_UPDATE_IND16( tharrier )
{
	nmk16_state *state = screen.machine().driver_data<nmk16_state>();
	/* I think the protection device probably copies this to the regs... */
	UINT16 tharrier_scroll = state->m_mainram[0x9f00/2];

	state->m_bg_tilemap0->set_scrollx(0,tharrier_scroll);

  return nmk16_bg_sprflip_tx_update(screen, bitmap, cliprect);
}

SCREEN_UPDATE_IND16( tdragon2 )
{
	static int bittbl[8] = {
    4, 6, 5, 7, 3, 2, 1, 0
	};

  return nmk16_complexbg_sprswap_tx_update(screen, bitmap, cliprect, bittbl);
}

SCREEN_UPDATE_IND16( gunnail )
{
	static int bittbl[8] = {
    7, 6, 5, 4, 3, 2, 1, 0
	};

  return nmk16_complexbg_sprswap_tx_update(screen, bitmap, cliprect, bittbl);
}

SCREEN_UPDATE_IND16( bioship )
{
  return nmk16_bioshipbg_sprflip_tx_update(screen, bitmap, cliprect);
}

SCREEN_UPDATE_IND16( strahl )
{
  return nmk16_bg_fg_spr_tx_update(screen, bitmap, cliprect);
}

SCREEN_UPDATE_IND16( bjtwin )
{
  return nmk16_bg_spr_update(screen, bitmap, cliprect);
}

SCREEN_VBLANK( nmk )
{
	// rising edge
	if (vblank_on)
	{
		nmk16_state *state = screen.machine().driver_data<nmk16_state>();
		/* sprites are DMA'd from Main RAM to a private buffer automatically
           (or at least this is how I interpret the datasheet) */

		/* -- I actually see little evidence to support this, sprite lag
              in some games should be checked on real boards */

	//  memcpy(state->m_spriteram_old2,state->m_spriteram_old,0x1000);
		memcpy(state->m_spriteram_old2,state->m_mainram+0x8000/2,0x1000);
	}
}

SCREEN_VBLANK( strahl )
{
	// rising edge
	if (vblank_on)
	{
		nmk16_state *state = screen.machine().driver_data<nmk16_state>();
		/* sprites are DMA'd from Main RAM to a private buffer automatically
           (or at least this is how I interpret the datasheet) */

		/* -- I actually see little evidence to support this, sprite lag
              in some games should be checked on real boards */

		/* strahl sprites are allocated in memory range FF000-FFFFF */

		memcpy(state->m_spriteram_old2,state->m_mainram+0xF000/2,0x1000);
	}
}



/***************************************************************************


                            Video Hardware Init


***************************************************************************/

VIDEO_START( afega )
{
	nmk16_state *state = machine.driver_data<nmk16_state>();
	state->m_spriteram_old = auto_alloc_array_clear(machine, UINT16, 0x1000/2);
	state->m_spriteram_old2 = auto_alloc_array_clear(machine, UINT16, 0x1000/2);

	state->m_bg_tilemap0 = tilemap_create(	machine, macross_get_bg0_tile_info, afega_tilemap_scan_pages,

								16,16,
								TILES_PER_PAGE_X*PAGES_PER_TMAP_X,TILES_PER_PAGE_Y*PAGES_PER_TMAP_Y);

	state->m_tx_tilemap = tilemap_create(	machine, macross_get_tx_tile_info, tilemap_scan_cols,

								8,8,
								32,32);

	state->m_tx_tilemap->set_transparent_pen(0xf);
}


VIDEO_START( grdnstrm )
{
	nmk16_state *state = machine.driver_data<nmk16_state>();
	state->m_spriteram_old = auto_alloc_array_clear(machine, UINT16, 0x1000/2);
	state->m_spriteram_old2 = auto_alloc_array_clear(machine, UINT16, 0x1000/2);


	state->m_bg_tilemap0 = tilemap_create(	machine, get_tile_info_0_8bit, afega_tilemap_scan_pages,

								16,16,
								TILES_PER_PAGE_X*PAGES_PER_TMAP_X,TILES_PER_PAGE_Y*PAGES_PER_TMAP_Y);

	state->m_tx_tilemap = tilemap_create(	machine, macross_get_tx_tile_info, tilemap_scan_cols,

								8,8,
								32,32);

	state->m_tx_tilemap->set_transparent_pen(0xf);
}


VIDEO_START( firehawk )
{
	nmk16_state *state = machine.driver_data<nmk16_state>();
	state->m_spriteram_old = auto_alloc_array_clear(machine, UINT16, 0x1000/2);
	state->m_spriteram_old2 = auto_alloc_array_clear(machine, UINT16, 0x1000/2);


	state->m_bg_tilemap0 = tilemap_create(	machine, get_tile_info_0_8bit, afega_tilemap_scan_pages,

								16,16,
								TILES_PER_PAGE_X*PAGES_PER_TMAP_X,TILES_PER_PAGE_Y*PAGES_PER_TMAP_Y);

	state->m_tx_tilemap = tilemap_create(	machine, macross_get_tx_tile_info, tilemap_scan_cols,

								8,8,
								32,32);

	state->m_tx_tilemap->set_transparent_pen(0xf);
}


/***************************************************************************


                                Screen Drawing


***************************************************************************/

static void video_update(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect,
	int dsw_flipscreen,			// 1 = Horizontal and vertical screen flip are hardwired to 2 dip switches
	int xoffset, int yoffset,	// bg_tilemap0 offsets
	int attr_mask				// "sprite active" mask
	)
{
	nmk16_state *state = machine.driver_data<nmk16_state>();



	if (dsw_flipscreen)
	{

		state->flip_screen_x_set(~state->ioport("DSW1")->read() & 0x0100);
		state->flip_screen_y_set(~state->ioport("DSW1")->read() & 0x0200);
	}


	state->m_bg_tilemap0->set_scrollx(0, state->m_afega_scroll_0[1] + xoffset);
	state->m_bg_tilemap0->set_scrolly(0, state->m_afega_scroll_0[0] + yoffset);

	state->m_tx_tilemap->set_scrollx(0, state->m_afega_scroll_1[1]);
	state->m_tx_tilemap->set_scrolly(0, state->m_afega_scroll_1[0]);


	state->m_bg_tilemap0->draw(bitmap, cliprect, 0,0);

	nmk16_draw_sprites_flipsupported(machine, bitmap,cliprect,3);
	nmk16_draw_sprites_flipsupported(machine, bitmap,cliprect,2);
	nmk16_draw_sprites_flipsupported(machine, bitmap,cliprect,1);
	nmk16_draw_sprites_flipsupported(machine, bitmap,cliprect,0);

	state->m_tx_tilemap->draw(bitmap, cliprect, 0,0);
}

static void redhawki_video_update(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect	)
{
	nmk16_state *state = machine.driver_data<nmk16_state>();


	state->m_bg_tilemap0->set_scrollx(0, state->m_afega_scroll_1[0]&0xff);
	state->m_bg_tilemap0->set_scrolly(0, state->m_afega_scroll_1[1]&0xff);

	state->m_bg_tilemap0->draw(bitmap, cliprect, 0,0);

	nmk16_draw_sprites_flipsupported(machine, bitmap,cliprect,3);
	nmk16_draw_sprites_flipsupported(machine, bitmap,cliprect,2);
	nmk16_draw_sprites_flipsupported(machine, bitmap,cliprect,1);
	nmk16_draw_sprites_flipsupported(machine, bitmap,cliprect,0);
}

SCREEN_UPDATE_IND16( afega )		{	video_update(screen.machine(),bitmap,cliprect, 1, -0x100,+0x000, 0x0001);	return 0; }
SCREEN_UPDATE_IND16( bubl2000 )	{	video_update(screen.machine(),bitmap,cliprect, 0, -0x100,+0x000, 0x0001);	return 0; }	// no flipscreen support, I really would confirmation from the schematics
SCREEN_UPDATE_IND16( redhawkb )	{	video_update(screen.machine(),bitmap,cliprect, 0, +0x000,+0x100, 0x0001);	return 0; }
SCREEN_UPDATE_IND16( redhawki )	{	redhawki_video_update(screen.machine(),bitmap,cliprect); return 0;} // strange scroll regs

SCREEN_UPDATE_IND16( firehawk )
{
	nmk16_state *state = screen.machine().driver_data<nmk16_state>();
	state->m_bg_tilemap0->set_scrolly(0, state->m_afega_scroll_1[1] + 0x100);
	state->m_bg_tilemap0->set_scrollx(0, state->m_afega_scroll_1[0]);

	state->m_bg_tilemap0->draw(bitmap, cliprect, 0,0);

	nmk16_draw_sprites_flipsupported(screen.machine(), bitmap,cliprect,3);
	nmk16_draw_sprites_flipsupported(screen.machine(), bitmap,cliprect,2);
	nmk16_draw_sprites_flipsupported(screen.machine(), bitmap,cliprect,1);
	nmk16_draw_sprites_flipsupported(screen.machine(), bitmap,cliprect,0);

	state->m_tx_tilemap->draw(bitmap, cliprect, 0,0);
	return 0;
}

