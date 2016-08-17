// license:BSD-3-Clause
// copyright-holders:Darren Olafson, Quench
/***************************************************************************

  Functions to emulate the video hardware of some Toaplan games,
  which use the BCU-2 tile controller, and the FCU-2 Sprite controller -
  and SCU Sprite controller (Only Rally Bike uses the SCU controller).


  There are 4 scrolling layers of graphics, stored in planes of 64x64 tiles.
  Each tile in each plane is assigned a priority between 1 and 15, higher
  numbers have greater priority.

 BCU controller. Each tile takes up 32 bits - the format is:

  0         1         2         3
  ---- ---- ---- ---- -ttt tttt tttt tttt = Tile number (0 - $7fff)
  ---- ---- ---- ---- h--- ---- ---- ---- = Hidden
  ---- ---- --cc cccc ---- ---- ---- ---- = Color (0 - $3f)
  pppp ---- ---- ---- ---- ---- ---- ---- = Priority (0-$f)
  ---- ???? ??-- ---- ---- ---- ---- ---- = Unknown / Unused

  Scroll Reg

  0         1         2         3
  xxxx xxxx x--- ---- ---- ---- ---- ---- = X position
  ---- ---- ---- ---- yyyy yyyy y--- ---- = Y position
  ---- ---- -??? ???? ---- ---- -??? ???? = Unknown / Unused



 FCU controller. Sprite RAM format  (except Rally Bike)

  0         1         2         3
  -sss ssss ssss ssss ---- ---- ---- ---- = Sprite number (0 - $7fff)
  h--- ---- ---- ---- ---- ---- ---- ---- = Hidden
  ---- ---- ---- ---- ---- ---- --cc cccc = Color (0 - $3f)
  ---- ---- ---- ---- ---- dddd dd-- ---- = Dimension (pointer to Size RAM)
  ---- ---- ---- ---- pppp ---- ---- ---- = Priority (0-$f)

  4         5         6         7
  ---- ---- ---- ---- xxxx xxxx x--- ---- = X position
  yyyy yyyy y--- ---- ---- ---- ---- ---- = Y position
  ---- ---- -??? ???? ---- ---- -??? ???? = Unknown



 SCU controller. Sprite RAM format  (Rally Bike)

  0         1         2         3
  ---- -sss ssss ssss ---- ---- ---- ---- = Sprite number (0 - $7FF)
  ---- ---- ---- ---- ---- ---- --cc cccc = Color (0 - $3F)
  ---- ---- ---- ---- ---- ---x ---- ---- = Flip X
  ---- ---- ---- ---- ---- --y- ---- ---- = Flip Y
  ---- ---- ---- ---- ---- pp-- ---- ---- = Priority (0h,4h,8h,Ch (shifted < 2 places))
  ???? ?--- ---- ---- ???? ---- ??-- ---- = Unknown / Unused

  4         5         6         7
  xxxx xxxx x--- ---- ---- ---- ---- ---- = X position
  ---- ---- ---- ---- yyyy yyyy y--- ---- = Y position
  ---- ---- -??? ???? ---- ---- -??? ???? = Unknown



  The tiles use a palette of 1024 colors, the sprites use a different palette
  of 1024 colors.


           BCU Controller writes                Tile Offsets
 Game      reg0  reg1  reg2  reg3         X     Y     flip-X  flip-Y
RallyBik   41e0  2e1e  148c  0f09        01e6  00fc     <- same --
Truxton    41e0  2717  0e86  0c06        01b7  00f2     0188  01fd
HellFire   41e0  2717  0e86  0c06        01b7  0102     0188  000d
ZeroWing   41e0  2717  0e86  0c06        01b7  0102     0188  000d
DemonWld   41e0  2e1e  148c  0f09        01a9  00fc     0196  0013
FireShrk   41e0  2717  0e86  0c06        01b7  00f2     0188  01fd
Out-Zone   41e0  2e1e  148c  0f09        01a9  00ec     0196  0003
Vimana     41e0  2717  0e86  0c06        01b7  00f2     0188  01fd


Sprites are of varying sizes between 8x8 and 128x128 with any variation
in between, in multiples of 8 either way.
Here we draw the first 8x8 part of the sprite, then by using the sprite
dimensions, we draw the rest of the 8x8 parts to produce the complete
sprite.


Abnormalities:
 How/when do priority 0 Tile layers really get displayed ?

 What are the video PROMs for ? Priority maybe ?

 Zerowing flashes red when an enemy is shot, and this is done by flipping
 layer 2 into its oversized second half which is all red. In flipscreen
 mode, this doesn't work properly as the flip scroll value doesn't equate
 properly.
 Possibly a bug with the game itself using a wrong scroll value ??
 Here's some notes:
 First values are non red flash scrolls. Second values are red flash scrolls.

 Scrolls    PF1-X  PF1-Y    PF2-X  PF2-Y    PF3-X  PF3-Y    PF4-X  PF4-Y
 ------>    #4180  #f880    #1240  #f880    #4380  #f880    #e380  #f880
 -flip->    #1500  #7f00    #e8c0  #7f00    #1300  #7f00    #bb00  #7f00

 ------>    #4100  #f880    #1200  #7880    #4300  #f880    #e380  #f880
 -flip->    #1500  #7f00    #e8c0  #8580??  #1300  #7f00    #bb00  #7f00
                                      |
                                      |
                                    f880 = 111110001xxxxxxx -> 1f1 scroll
                                    7f00 = 011111110xxxxxxx -> 0fe scroll
                                    7880 = 011110001xxxxxxx -> 0f1 scroll
                                    8580 = 100001011xxxxxxx -> 10b scroll

 So a snapshot of the scroll equations become: (from the functions below)
    1f1 - (102 - 101) == 1f0   star background
    0fe - (00d - 1ef) == 0e0   star background (flipscreen)
    0f1 - (102 - 101) == 0f0   red  background
    10b - (00d - 1ef) == 0ed   red  background (flipscreen) wrong!
    10b - (00d - 0ef) == 1ed   red  background (flipscreen) should somehow equate to this


***************************************************************************/


#include "emu.h"
#include "includes/toaplan1.h"


#define TOAPLAN1_TILEVRAM_SIZE       0x4000 /* 4 tile layers each this RAM size */
#define TOAPLAN1_SPRITERAM_SIZE      0x800  /* sprite ram */
#define TOAPLAN1_SPRITESIZERAM_SIZE  0x80   /* sprite size ram */


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(toaplan1_state::get_pf1_tile_info)
{
	int color, tile_number, attrib;

	tile_number = m_pf1_tilevram16[2*tile_index+1] & 0x7fff;
	attrib = m_pf1_tilevram16[2*tile_index];
	color = attrib & 0x3f;
	SET_TILE_INFO_MEMBER(0,
			tile_number,
			color,
			0);
	// "disabled" tiles are behind everything else
	if (m_pf1_tilevram16[2*tile_index+1] & 0x8000) tileinfo.category = 16;
		else tileinfo.category = (attrib & 0xf000) >> 12;
}

TILE_GET_INFO_MEMBER(toaplan1_state::get_pf2_tile_info)
{
	int color, tile_number, attrib;

	tile_number = m_pf2_tilevram16[2*tile_index+1] & 0x7fff;
	attrib = m_pf2_tilevram16[2*tile_index];
	color = attrib & 0x3f;
	SET_TILE_INFO_MEMBER(0,
			tile_number,
			color,
			0);
	// "disabled" tiles are behind everything else
	if (m_pf2_tilevram16[2*tile_index+1] & 0x8000) tileinfo.category = 16;
		else tileinfo.category = (attrib & 0xf000) >> 12;
}

TILE_GET_INFO_MEMBER(toaplan1_state::get_pf3_tile_info)
{
	int color, tile_number, attrib;

	tile_number = m_pf3_tilevram16[2*tile_index+1] & 0x7fff;
	attrib = m_pf3_tilevram16[2*tile_index];
	color = attrib & 0x3f;
	SET_TILE_INFO_MEMBER(0,
			tile_number,
			color,
			0);
	// "disabled" tiles are behind everything else
	if (m_pf3_tilevram16[2*tile_index+1] & 0x8000) tileinfo.category = 16;
		else tileinfo.category = (attrib & 0xf000) >> 12;
}

TILE_GET_INFO_MEMBER(toaplan1_state::get_pf4_tile_info)
{
	int color, tile_number, attrib;

	tile_number = m_pf4_tilevram16[2*tile_index+1] & 0x7fff;
	attrib = m_pf4_tilevram16[2*tile_index];
	color = attrib & 0x3f;
	SET_TILE_INFO_MEMBER(0,
			tile_number,
			color,
			0);
	// "disabled" tiles are behind everything else
	if (m_pf4_tilevram16[2*tile_index+1] & 0x8000) tileinfo.category = 16;
		else tileinfo.category = (attrib & 0xf000) >> 12;
}

/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void toaplan1_state::toaplan1_create_tilemaps()
{
	m_pf1_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(toaplan1_state::get_pf1_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_pf2_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(toaplan1_state::get_pf2_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_pf3_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(toaplan1_state::get_pf3_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_pf4_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(toaplan1_state::get_pf4_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);

	m_pf1_tilemap->set_transparent_pen(0);
	m_pf2_tilemap->set_transparent_pen(0);
	m_pf3_tilemap->set_transparent_pen(0);
	m_pf4_tilemap->set_transparent_pen(0);
}

void toaplan1_state::toaplan1_vram_alloc()
{
	m_pf1_tilevram16 = make_unique_clear<UINT16[]>(TOAPLAN1_TILEVRAM_SIZE/2);
	m_pf2_tilevram16 = make_unique_clear<UINT16[]>(TOAPLAN1_TILEVRAM_SIZE/2);
	m_pf3_tilevram16 = make_unique_clear<UINT16[]>(TOAPLAN1_TILEVRAM_SIZE/2);
	m_pf4_tilevram16 = make_unique_clear<UINT16[]>(TOAPLAN1_TILEVRAM_SIZE/2);

	save_pointer(NAME(m_pf1_tilevram16.get()), TOAPLAN1_TILEVRAM_SIZE/2);
	save_pointer(NAME(m_pf2_tilevram16.get()), TOAPLAN1_TILEVRAM_SIZE/2);
	save_pointer(NAME(m_pf3_tilevram16.get()), TOAPLAN1_TILEVRAM_SIZE/2);
	save_pointer(NAME(m_pf4_tilevram16.get()), TOAPLAN1_TILEVRAM_SIZE/2);

#ifdef MAME_DEBUG
	m_display_pf1 = 1;
	m_display_pf2 = 1;
	m_display_pf3 = 1;
	m_display_pf4 = 1;
	m_displog = 0;
#endif
}

void toaplan1_state::toaplan1_spritevram_alloc()
{
	m_spriteram.allocate(TOAPLAN1_SPRITERAM_SIZE/2);
	m_buffered_spriteram = make_unique_clear<UINT16[]>(TOAPLAN1_SPRITERAM_SIZE/2);
	m_spritesizeram16 = make_unique_clear<UINT16[]>(TOAPLAN1_SPRITESIZERAM_SIZE/2);
	m_buffered_spritesizeram16 = make_unique_clear<UINT16[]>(TOAPLAN1_SPRITESIZERAM_SIZE/2);

	save_pointer(NAME(m_buffered_spriteram.get()), TOAPLAN1_SPRITERAM_SIZE/2);
	save_pointer(NAME(m_spritesizeram16.get()), TOAPLAN1_SPRITESIZERAM_SIZE/2);
	save_pointer(NAME(m_buffered_spritesizeram16.get()), TOAPLAN1_SPRITESIZERAM_SIZE/2);
}

void toaplan1_state::toaplan1_set_scrolls()
{
	m_pf1_tilemap->set_scrollx(0, (m_pf1_scrollx >> 7) - m_tiles_offsetx);
	m_pf2_tilemap->set_scrollx(0, (m_pf2_scrollx >> 7) - m_tiles_offsetx);
	m_pf3_tilemap->set_scrollx(0, (m_pf3_scrollx >> 7) - m_tiles_offsetx);
	m_pf4_tilemap->set_scrollx(0, (m_pf4_scrollx >> 7) - m_tiles_offsetx);
	m_pf1_tilemap->set_scrolly(0, (m_pf1_scrolly >> 7) - m_tiles_offsety);
	m_pf2_tilemap->set_scrolly(0, (m_pf2_scrolly >> 7) - m_tiles_offsety);
	m_pf3_tilemap->set_scrolly(0, (m_pf3_scrolly >> 7) - m_tiles_offsety);
	m_pf4_tilemap->set_scrolly(0, (m_pf4_scrolly >> 7) - m_tiles_offsety);
}

void toaplan1_state::register_common()
{
	save_item(NAME(m_bcu_flipscreen));
	save_item(NAME(m_fcu_flipscreen));

	save_item(NAME(m_pf1_scrollx));
	save_item(NAME(m_pf1_scrolly));
	save_item(NAME(m_pf2_scrollx));
	save_item(NAME(m_pf2_scrolly));
	save_item(NAME(m_pf3_scrollx));
	save_item(NAME(m_pf3_scrolly));
	save_item(NAME(m_pf4_scrollx));
	save_item(NAME(m_pf4_scrolly));

	save_item(NAME(m_tiles_offsetx));
	save_item(NAME(m_tiles_offsety));
	save_item(NAME(m_pf_voffs));
	save_item(NAME(m_spriteram_offs));
}


VIDEO_START_MEMBER(toaplan1_rallybik_state,rallybik)
{
	m_spritegen->alloc_sprite_bitmap(*m_screen);
	m_spritegen->gfx(0)->set_colorbase(64*16);

	toaplan1_create_tilemaps();
	toaplan1_vram_alloc();

	m_buffered_spriteram = make_unique_clear<UINT16[]>(m_spriteram.bytes()/2);
	save_pointer(NAME(m_buffered_spriteram.get()), m_spriteram.bytes()/2);

	m_pf1_tilemap->set_scrolldx(-0x00d-6, -0x80+6);
	m_pf2_tilemap->set_scrolldx(-0x00d-4, -0x80+4);
	m_pf3_tilemap->set_scrolldx(-0x00d-2, -0x80+2);
	m_pf4_tilemap->set_scrolldx(-0x00d-0, -0x80+0);
	m_pf1_tilemap->set_scrolldy(-0x111, 0x8);
	m_pf2_tilemap->set_scrolldy(-0x111, 0x8);
	m_pf3_tilemap->set_scrolldy(-0x111, 0x8);
	m_pf4_tilemap->set_scrolldy(-0x111, 0x8);

	m_bcu_flipscreen = -1;
	m_fcu_flipscreen = 0;

	register_common();
}

VIDEO_START_MEMBER(toaplan1_state,toaplan1)
{
	toaplan1_create_tilemaps();
	toaplan1_vram_alloc();
	toaplan1_spritevram_alloc();

	m_pf1_tilemap->set_scrolldx(-0x1ef-6, -0x11+6);
	m_pf2_tilemap->set_scrolldx(-0x1ef-4, -0x11+4);
	m_pf3_tilemap->set_scrolldx(-0x1ef-2, -0x11+2);
	m_pf4_tilemap->set_scrolldx(-0x1ef-0, -0x11+0);
	m_pf1_tilemap->set_scrolldy(-0x101, -0xff);
	m_pf2_tilemap->set_scrolldy(-0x101, -0xff);
	m_pf3_tilemap->set_scrolldy(-0x101, -0xff);
	m_pf4_tilemap->set_scrolldy(-0x101, -0xff);

	m_bcu_flipscreen = -1;
	m_fcu_flipscreen = 0;

	register_common();
}


/***************************************************************************

  Video I/O port hardware.

***************************************************************************/

READ16_MEMBER(toaplan1_state::toaplan1_frame_done_r)
{
	return m_screen->vblank();
}

WRITE16_MEMBER(toaplan1_state::toaplan1_tile_offsets_w)
{
	if ( offset == 0 )
	{
		COMBINE_DATA(&m_tiles_offsetx);
		logerror("Tiles_offsetx now = %08x\n", m_tiles_offsetx);
	}
	else
	{
		COMBINE_DATA(&m_tiles_offsety);
		logerror("Tiles_offsety now = %08x\n", m_tiles_offsety);
	}
	toaplan1_set_scrolls();
}

WRITE16_MEMBER(toaplan1_state::toaplan1_bcu_flipscreen_w)
{
	if (ACCESSING_BITS_0_7 && (data != m_bcu_flipscreen))
	{
		logerror("Setting BCU controller flipscreen port to %04x\n",data);
		m_bcu_flipscreen = data & 0x01;     /* 0x0001 = flip, 0x0000 = no flip */
		machine().tilemap().set_flip_all((data ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0));

		toaplan1_set_scrolls();
	}
}

WRITE16_MEMBER(toaplan1_state::toaplan1_fcu_flipscreen_w)
{
	if (ACCESSING_BITS_8_15)
	{
		logerror("Setting FCU controller flipscreen port to %04x\n",data);
		m_fcu_flipscreen = data & 0x8000;   /* 0x8000 = flip, 0x0000 = no flip */
	}
}

READ16_MEMBER(toaplan1_state::toaplan1_spriteram_offs_r)/// this aint really needed ?
{
	return m_spriteram_offs;
}

WRITE16_MEMBER(toaplan1_state::toaplan1_spriteram_offs_w)
{
	COMBINE_DATA(&m_spriteram_offs);
}


WRITE16_MEMBER(toaplan1_state::toaplan1_bgpalette_w)
{
	COMBINE_DATA(&m_bgpaletteram[offset]);
	data = m_bgpaletteram[offset];
	m_palette->set_pen_color(offset, pal5bit(data>>0), pal5bit(data>>5), pal5bit(data>>10));
}

WRITE16_MEMBER(toaplan1_state::toaplan1_fgpalette_w)
{
	COMBINE_DATA(&m_fgpaletteram[offset]);
	data = m_fgpaletteram[offset];
	m_palette->set_pen_color(offset + 64*16, pal5bit(data>>0), pal5bit(data>>5), pal5bit(data>>10));
}


READ16_MEMBER(toaplan1_state::toaplan1_spriteram16_r)
{
	return m_spriteram[m_spriteram_offs & ((TOAPLAN1_SPRITERAM_SIZE/2)-1)];
}

WRITE16_MEMBER(toaplan1_state::toaplan1_spriteram16_w)
{
	COMBINE_DATA(&m_spriteram[m_spriteram_offs & ((TOAPLAN1_SPRITERAM_SIZE/2)-1)]);

#ifdef MAME_DEBUG
	if (m_spriteram_offs >= (TOAPLAN1_SPRITERAM_SIZE/2))
	{
		logerror("Sprite_RAM_word_w, %08x out of range !\n", m_spriteram_offs);
		return;
	}
#endif

	m_spriteram_offs++;
}

READ16_MEMBER(toaplan1_state::toaplan1_spritesizeram16_r)
{
	return m_spritesizeram16[m_spriteram_offs & ((TOAPLAN1_SPRITESIZERAM_SIZE/2)-1)];
}

WRITE16_MEMBER(toaplan1_state::toaplan1_spritesizeram16_w)
{
	COMBINE_DATA(&m_spritesizeram16[m_spriteram_offs & ((TOAPLAN1_SPRITESIZERAM_SIZE/2)-1)]);

#ifdef MAME_DEBUG
	if (m_spriteram_offs >= (TOAPLAN1_SPRITESIZERAM_SIZE/2))
	{
		logerror("Sprite_Size_RAM_word_w, %08x out of range !\n", m_spriteram_offs);
		return;
	}
#endif

	m_spriteram_offs++; /// really ? shouldn't happen on the sizeram
}



WRITE16_MEMBER(toaplan1_state::toaplan1_bcu_control_w)
{
	logerror("BCU tile controller register:%02x now = %04x\n",offset,data);
}

READ16_MEMBER(toaplan1_state::toaplan1_tileram_offs_r)
{
	return m_pf_voffs;
}

WRITE16_MEMBER(toaplan1_state::toaplan1_tileram_offs_w)
{
	if (data >= 0x4000)
		logerror("Hmmm, unknown video layer being selected (%08x)\n",data);
	COMBINE_DATA(&m_pf_voffs);
}


READ16_MEMBER(toaplan1_state::toaplan1_tileram16_r)
{
	offs_t vram_offset;
	UINT16 video_data = 0;

	switch (m_pf_voffs & 0xf000)    /* Locate Layer (PlayField) */
	{
		case 0x0000:
				vram_offset = ((m_pf_voffs * 2) + offset) & ((TOAPLAN1_TILEVRAM_SIZE/2)-1);
				video_data = m_pf1_tilevram16[vram_offset];
				break;
		case 0x1000:
				vram_offset = ((m_pf_voffs * 2) + offset) & ((TOAPLAN1_TILEVRAM_SIZE/2)-1);
				video_data = m_pf2_tilevram16[vram_offset];
				break;
		case 0x2000:
				vram_offset = ((m_pf_voffs * 2) + offset) & ((TOAPLAN1_TILEVRAM_SIZE/2)-1);
				video_data = m_pf3_tilevram16[vram_offset];
				break;
		case 0x3000:
				vram_offset = ((m_pf_voffs * 2) + offset) & ((TOAPLAN1_TILEVRAM_SIZE/2)-1);
				video_data = m_pf4_tilevram16[vram_offset];
				break;
		default:
				logerror("Hmmm, reading %04x from unknown playfield layer address %06x  Offset:%01x !!!\n", video_data, m_pf_voffs, offset);
				break;
	}

	return video_data;
}

READ16_MEMBER(toaplan1_rallybik_state::rallybik_tileram16_r)
{
	UINT16 data = toaplan1_tileram16_r(space, offset, mem_mask);

	if (offset == 0)    /* some bit lines may be stuck to others */
	{
		data |= ((data & 0xf000) >> 4);
		data |= ((data & 0x0030) << 2);
	}
	return data;
}

WRITE16_MEMBER(toaplan1_state::toaplan1_tileram16_w)
{
	offs_t vram_offset;

	switch (m_pf_voffs & 0xf000)    /* Locate Layer (PlayField) */
	{
		case 0x0000:
				vram_offset = ((m_pf_voffs * 2) + offset) & ((TOAPLAN1_TILEVRAM_SIZE/2)-1);
				COMBINE_DATA(&m_pf1_tilevram16[vram_offset]);
				m_pf1_tilemap->mark_tile_dirty(vram_offset/2);
				break;
		case 0x1000:
				vram_offset = ((m_pf_voffs * 2) + offset) & ((TOAPLAN1_TILEVRAM_SIZE/2)-1);
				COMBINE_DATA(&m_pf2_tilevram16[vram_offset]);
				m_pf2_tilemap->mark_tile_dirty(vram_offset/2);
				break;
		case 0x2000:
				vram_offset = ((m_pf_voffs * 2) + offset) & ((TOAPLAN1_TILEVRAM_SIZE/2)-1);
				COMBINE_DATA(&m_pf3_tilevram16[vram_offset]);
				m_pf3_tilemap->mark_tile_dirty(vram_offset/2);
				break;
		case 0x3000:
				vram_offset = ((m_pf_voffs * 2) + offset) & ((TOAPLAN1_TILEVRAM_SIZE/2)-1);
				COMBINE_DATA(&m_pf4_tilevram16[vram_offset]);
				m_pf4_tilemap->mark_tile_dirty(vram_offset/2);
				break;
		default:
				logerror("Hmmm, writing %04x to unknown playfield layer address %06x  Offset:%01x\n", data, m_pf_voffs, offset);
				break;
	}
}



READ16_MEMBER(toaplan1_state::toaplan1_scroll_regs_r)
{
	UINT16 scroll = 0;

	switch(offset)
	{
		case 00: scroll = m_pf1_scrollx; break;
		case 01: scroll = m_pf1_scrolly; break;
		case 02: scroll = m_pf2_scrollx; break;
		case 03: scroll = m_pf2_scrolly; break;
		case 04: scroll = m_pf3_scrollx; break;
		case 05: scroll = m_pf3_scrolly; break;
		case 06: scroll = m_pf4_scrollx; break;
		case 07: scroll = m_pf4_scrolly; break;
		default: logerror("Hmmm, reading unknown video scroll register (%08x) !!!\n",offset);
					break;
	}
	return scroll;
}


WRITE16_MEMBER(toaplan1_state::toaplan1_scroll_regs_w)
{
	switch(offset)
	{
		case 00: COMBINE_DATA(&m_pf1_scrollx);      /* 1D3h */
					m_pf1_tilemap->set_scrollx(0, (m_pf1_scrollx >> 7) - m_tiles_offsetx);
					break;
		case 01: COMBINE_DATA(&m_pf1_scrolly);      /* 1EBh */
					m_pf1_tilemap->set_scrolly(0, (m_pf1_scrolly >> 7) - m_tiles_offsety);
					break;
		case 02: COMBINE_DATA(&m_pf2_scrollx);      /* 1D5h */
					m_pf2_tilemap->set_scrollx(0, (m_pf2_scrollx >> 7) - m_tiles_offsetx);
					break;
		case 03: COMBINE_DATA(&m_pf2_scrolly);      /* 1EBh */
					m_pf2_tilemap->set_scrolly(0, (m_pf2_scrolly >> 7) - m_tiles_offsety);
					break;
		case 04: COMBINE_DATA(&m_pf3_scrollx);      /* 1D7h */
					m_pf3_tilemap->set_scrollx(0, (m_pf3_scrollx >> 7) - m_tiles_offsetx);
					break;
		case 05: COMBINE_DATA(&m_pf3_scrolly);      /* 1EBh */
					m_pf3_tilemap->set_scrolly(0, (m_pf3_scrolly >> 7) - m_tiles_offsety);
					break;
		case 06: COMBINE_DATA(&m_pf4_scrollx);      /* 1D9h */
					m_pf4_tilemap->set_scrollx(0, (m_pf4_scrollx >> 7) - m_tiles_offsetx);
					break;
		case 07: COMBINE_DATA(&m_pf4_scrolly);      /* 1EBh */
					m_pf4_tilemap->set_scrolly(0, (m_pf4_scrolly >> 7) - m_tiles_offsety);
					break;
		default: logerror("Hmmm, writing %08x to unknown video scroll register (%08x) !!!\n",data ,offset);
					break;
	}
}




void toaplan1_state::toaplan1_log_vram()
{
#ifdef MAME_DEBUG

	if ( machine().input().code_pressed(KEYCODE_M) )
	{
		UINT16 *spriteram16 = m_spriteram;
		UINT16 *buffered_spriteram16 = m_buffered_spriteram.get();
		offs_t sprite_voffs;
		while (machine().input().code_pressed(KEYCODE_M)) ;
		if (m_spritesizeram16)           /* FCU controller */
		{
			int schar,sattr,sxpos,sypos,bschar,bsattr,bsxpos,bsypos;
			UINT16 *size  = (UINT16 *)(m_spritesizeram16.get());
			UINT16 *bsize = (UINT16 *)(m_buffered_spritesizeram16.get());
			logerror("Scrolls    PF1-X  PF1-Y     PF2-X  PF2-Y     PF3-X  PF3-Y     PF4-X  PF4-Y\n");
			logerror("------>    #%04x  #%04x     #%04x  #%04x     #%04x  #%04x     #%04x  #%04x\n",
				m_pf1_scrollx, m_pf1_scrolly, m_pf2_scrollx, m_pf2_scrolly, m_pf3_scrollx, m_pf3_scrolly, m_pf4_scrollx, m_pf4_scrolly);
			for ( sprite_voffs = 0; sprite_voffs < m_spriteram.bytes()/2; sprite_voffs += 4 )
			{
				bschar = buffered_spriteram16[sprite_voffs];
				bsattr = buffered_spriteram16[sprite_voffs + 1];
				bsxpos = buffered_spriteram16[sprite_voffs + 2];
				bsypos = buffered_spriteram16[sprite_voffs + 3];
				schar = spriteram16[sprite_voffs];
				sattr = spriteram16[sprite_voffs + 1];
				sxpos = spriteram16[sprite_voffs + 2];
				sypos = spriteram16[sprite_voffs + 3];
				logerror("$(%04x)  Tile-Attr-Xpos-Ypos Now:%04x %04x %04x.%01x %04x.%01x  nxt:%04x %04x %04x.%01x %04x.%01x\n", sprite_voffs,
												schar, sattr, sxpos, size[( sattr>>6)&0x3f]&0xf, sypos,( size[( sattr>>6)&0x3f]>>4)&0xf,
											bschar,bsattr,bsxpos,bsize[(bsattr>>6)&0x3f]&0xf,bsypos,(bsize[(bsattr>>6)&0x3f]>>4)&0xf);
			}
		}
		else                                    /* SCU controller */
		{
			int schar,sattr,sxpos,sypos,bschar,bsattr,bsxpos,bsypos;
			logerror("Scrolls    PF1-X  PF1-Y     PF2-X  PF2-Y     PF3-X  PF3-Y     PF4-X  PF4-Y\n");
			logerror("------>    #%04x  #%04x     #%04x  #%04x     #%04x  #%04x     #%04x  #%04x\n",
				m_pf1_scrollx, m_pf1_scrolly, m_pf2_scrollx, m_pf2_scrolly, m_pf3_scrollx, m_pf3_scrolly, m_pf4_scrollx, m_pf4_scrolly);
			for ( sprite_voffs = 0; sprite_voffs < m_spriteram.bytes()/2; sprite_voffs += 4 )
			{
				bschar = buffered_spriteram16[sprite_voffs];
				bsattr = buffered_spriteram16[sprite_voffs + 1];
				bsypos = buffered_spriteram16[sprite_voffs + 2];
				bsxpos = buffered_spriteram16[sprite_voffs + 3];
				schar = spriteram16[sprite_voffs];
				sattr = spriteram16[sprite_voffs + 1];
				sypos = spriteram16[sprite_voffs + 2];
				sxpos = spriteram16[sprite_voffs + 3];
				logerror("$(%04x)  Tile-Attr-Xpos-Ypos Now:%04x %04x %04x %04x  nxt:%04x %04x %04x %04x\n", sprite_voffs,
												schar, sattr, sxpos, sypos,
											bschar,bsattr,bsxpos, bsypos);
			}
		}
	}

	if ( machine().input().code_pressed(KEYCODE_SLASH) )
	{
		UINT16 *size  = (UINT16 *)(m_spritesizeram16.get());
		UINT16 *bsize = (UINT16 *)(m_buffered_spritesizeram16.get());
		offs_t offs;
		while (machine().input().code_pressed(KEYCODE_SLASH)) ;
		if (m_spritesizeram16)           /* FCU controller */
		{
			logerror("Scrolls    PF1-X  PF1-Y     PF2-X  PF2-Y     PF3-X  PF3-Y     PF4-X  PF4-Y\n");
			logerror("------>    #%04x  #%04x     #%04x  #%04x     #%04x  #%04x     #%04x  #%04x\n",
				m_pf1_scrollx, m_pf1_scrolly, m_pf2_scrollx, m_pf2_scrolly, m_pf3_scrollx, m_pf3_scrolly, m_pf4_scrollx, m_pf4_scrolly);
			for ( offs = 0; offs < (TOAPLAN1_SPRITESIZERAM_SIZE/2); offs +=4 )
			{
				logerror("SizeOffs:%04x   now:%04x %04x %04x %04x    next: %04x %04x %04x %04x\n", offs,
												bsize[offs+0], bsize[offs+1],
												bsize[offs+2], bsize[offs+3],
												size[offs+0], size[offs+1],
												size[offs+2], size[offs+3]);
			}
		}
	}

	if ( machine().input().code_pressed(KEYCODE_N) )
	{
		offs_t tile_voffs;
		int tchar[5], tattr[5];
		while (machine().input().code_pressed(KEYCODE_N)) ;   /* BCU controller */
		logerror("Scrolls    PF1-X  PF1-Y     PF2-X  PF2-Y     PF3-X  PF3-Y     PF4-X  PF4-Y\n");
		logerror("------>    #%04x  #%04x     #%04x  #%04x     #%04x  #%04x     #%04x  #%04x\n",
			m_pf1_scrollx, m_pf1_scrolly, m_pf2_scrollx, m_pf2_scrolly, m_pf3_scrollx, m_pf3_scrolly, m_pf4_scrollx, m_pf4_scrolly);
		for ( tile_voffs = 0; tile_voffs < (TOAPLAN1_TILEVRAM_SIZE/2); tile_voffs += 2 )
		{
			tchar[1] = m_pf1_tilevram16[tile_voffs + 1];
			tattr[1] = m_pf1_tilevram16[tile_voffs];
			tchar[2] = m_pf2_tilevram16[tile_voffs + 1];
			tattr[2] = m_pf2_tilevram16[tile_voffs];
			tchar[3] = m_pf3_tilevram16[tile_voffs + 1];
			tattr[3] = m_pf3_tilevram16[tile_voffs];
			tchar[4] = m_pf4_tilevram16[tile_voffs + 1];
			tattr[4] = m_pf4_tilevram16[tile_voffs];
//          logerror("PF3 offs:%04x   Tile:%04x  Attr:%04x\n", tile_voffs, tchar, tattr);
			logerror("$(%04x)  Attr-Tile PF1:%04x-%04x  PF2:%04x-%04x  PF3:%04x-%04x  PF4:%04x-%04x\n", tile_voffs,
									tattr[1], tchar[1],  tattr[2], tchar[2],
									tattr[3], tchar[3],  tattr[4], tchar[4]);
		}
	}

	if ( machine().input().code_pressed(KEYCODE_W) )
	{
		while (machine().input().code_pressed(KEYCODE_W)) ;
		logerror("Mark here\n");
	}
	if ( machine().input().code_pressed(KEYCODE_E) )
	{
		while (machine().input().code_pressed(KEYCODE_E)) ;
		m_displog += 1;
		m_displog &= 1;
	}
	if (m_displog)
	{
		logerror("Scrolls    PF1-X  PF1-Y     PF2-X  PF2-Y     PF3-X  PF3-Y     PF4-X  PF4-Y\n");
		logerror("------>    #%04x  #%04x     #%04x  #%04x     #%04x  #%04x     #%04x  #%04x\n",
			m_pf1_scrollx, m_pf1_scrolly, m_pf2_scrollx, m_pf2_scrolly, m_pf3_scrollx, m_pf3_scrolly, m_pf4_scrollx, m_pf4_scrolly);
	}
	if ( machine().input().code_pressed(KEYCODE_L) )      /* Turn Playfield 4 on/off */
	{
		while (machine().input().code_pressed(KEYCODE_L)) ;
		m_display_pf4 += 1;
		m_display_pf4 &= 1;
		m_pf4_tilemap->enable(m_display_pf4);
	}
	if ( machine().input().code_pressed(KEYCODE_K) )      /* Turn Playfield 3 on/off */
	{
		while (machine().input().code_pressed(KEYCODE_K)) ;
		m_display_pf3 += 1;
		m_display_pf3 &= 1;
		m_pf3_tilemap->enable(m_display_pf3);
	}
	if ( machine().input().code_pressed(KEYCODE_J) )      /* Turn Playfield 2 on/off */
	{
		while (machine().input().code_pressed(KEYCODE_J)) ;
		m_display_pf2 += 1;
		m_display_pf2 &= 1;
		m_pf2_tilemap->enable(m_display_pf2);
	}
	if ( machine().input().code_pressed(KEYCODE_H) )      /* Turn Playfield 1 on/off */
	{
		while (machine().input().code_pressed(KEYCODE_H)) ;
		m_display_pf1 += 1;
		m_display_pf1 &= 1;
		m_pf1_tilemap->enable(m_display_pf1);
	}
#endif
}



/***************************************************************************
    Sprite Handlers
***************************************************************************/

// custom function to draw a single sprite. needed to keep correct sprites - sprites and sprites - tilemaps priorities
static void toaplan1_draw_sprite_custom(screen_device &screen, bitmap_ind16 &dest_bmp,const rectangle &clip,gfx_element *gfx,
		UINT32 code,UINT32 color,int flipx,int flipy,int sx,int sy,
		int priority)
{
	int pal_base = gfx->colorbase() + gfx->granularity() * (color % gfx->colors());
	const UINT8 *source_base = gfx->get_data(code % gfx->elements());
	bitmap_ind8 &priority_bitmap = screen.priority();
	int sprite_screen_height = ((1<<16)*gfx->height()+0x8000)>>16;
	int sprite_screen_width = ((1<<16)*gfx->width()+0x8000)>>16;

	if (sprite_screen_width && sprite_screen_height)
	{
		/* compute sprite increment per screen pixel */
		int dx = (gfx->width()<<16)/sprite_screen_width;
		int dy = (gfx->height()<<16)/sprite_screen_height;

		int ex = sx+sprite_screen_width;
		int ey = sy+sprite_screen_height;

		int x_index_base;
		int y_index;

		if( flipx )
		{
			x_index_base = (sprite_screen_width-1)*dx;
			dx = -dx;
		}
		else
		{
			x_index_base = 0;
		}

		if( flipy )
		{
			y_index = (sprite_screen_height-1)*dy;
			dy = -dy;
		}
		else
		{
			y_index = 0;
		}

		if( sx < clip.min_x)
		{ /* clip left */
			int pixels = clip.min_x-sx;
			sx += pixels;
			x_index_base += pixels*dx;
		}
		if( sy < clip.min_y )
		{ /* clip top */
			int pixels = clip.min_y-sy;
			sy += pixels;
			y_index += pixels*dy;
		}
		/* NS 980211 - fixed incorrect clipping */
		if( ex > clip.max_x+1 )
		{ /* clip right */
			int pixels = ex-clip.max_x-1;
			ex -= pixels;
		}
		if( ey > clip.max_y+1 )
		{ /* clip bottom */
			int pixels = ey-clip.max_y-1;
			ey -= pixels;
		}

		if( ex>sx )
		{ /* skip if inner loop doesn't draw anything */
			int y;

			for( y=sy; y<ey; y++ )
			{
				const UINT8 *source = source_base + (y_index>>16) * gfx->rowbytes();
				UINT16 *dest = &dest_bmp.pix16(y);
				UINT8 *pri = &priority_bitmap.pix8(y);

				int x, x_index = x_index_base;
				for( x=sx; x<ex; x++ )
				{
					int c = source[x_index>>16];
					if( c != 0 )
					{
						if (pri[x] < priority)
							dest[x] = pal_base+c;
						pri[x] = 0xff; // mark it "already drawn"
					}
					x_index += dx;
				}

				y_index += dy;
			}
		}
	}
}


void toaplan1_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	UINT16 *source = (UINT16 *)m_buffered_spriteram.get();
	UINT16 *size   = (UINT16 *)m_buffered_spritesizeram16.get();
	int fcu_flipscreen = m_fcu_flipscreen;
	int offs;

	for (offs = m_spriteram.bytes()/2 - 4; offs >= 0; offs -= 4)
	{
		if (!(source[offs] & 0x8000))
		{
			int attrib, sprite, color, priority, sx, sy;
			int sprite_sizex, sprite_sizey, dim_x, dim_y, sx_base, sy_base;
			int sizeram_ptr;

			attrib = source[offs+1];
			priority = (attrib & 0xf000) >> 12;

			sprite = source[offs] & 0x7fff;
			color = attrib & 0x3f;

			/****** find sprite dimension ******/
			sizeram_ptr = (attrib >> 6) & 0x3f;
			sprite_sizex = ( size[sizeram_ptr]       & 0x0f) * 8;
			sprite_sizey = ((size[sizeram_ptr] >> 4) & 0x0f) * 8;

			/****** find position to display sprite ******/
			sx_base = (source[offs + 2] >> 7) & 0x1ff;
			sy_base = (source[offs + 3] >> 7) & 0x1ff;

			if (sx_base >= 0x180) sx_base -= 0x200;
			if (sy_base >= 0x180) sy_base -= 0x200;

			/****** flip the sprite layer ******/
			if (fcu_flipscreen)
			{
				const rectangle &visarea = m_screen->visible_area();

				sx_base = visarea.width() - (sx_base + 8);  /* visarea.x = 320 */
				sy_base = visarea.height() - (sy_base + 8); /* visarea.y = 240 */
				sy_base += ((visarea.max_y + 1) - visarea.height()) * 2;    /* Horizontal games are offset so adjust by +0x20 */
			}

			for (dim_y = 0; dim_y < sprite_sizey; dim_y += 8)
			{
				if (fcu_flipscreen) sy = sy_base - dim_y;
				else                sy = sy_base + dim_y;

				for (dim_x = 0; dim_x < sprite_sizex; dim_x += 8)
				{
					if (fcu_flipscreen) sx = sx_base - dim_x;
					else                sx = sx_base + dim_x;

					toaplan1_draw_sprite_custom(screen,bitmap,cliprect,m_gfxdecode->gfx(1),
												sprite,color,
												fcu_flipscreen,fcu_flipscreen,
												sx,sy,
												priority);

					sprite++ ;
				}
			}
		}
	}
}




/***************************************************************************
    Draw the game screen in the given bitmap_ind16.
***************************************************************************/

UINT32 toaplan1_rallybik_state::screen_update_rallybik(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int priority;

	toaplan1_log_vram();

	m_spritegen->draw_sprites_to_tempbitmap(cliprect, m_buffered_spriteram.get(),  m_spriteram.bytes());

	// first draw everything, including "disabled" tiles and priority 0
	m_pf1_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE | TILEMAP_DRAW_ALL_CATEGORIES, 0);

	// then draw the higher priority layers in order
	for (priority = 1; priority < 16; priority++)
	{
		m_pf4_tilemap->draw(screen, bitmap, cliprect, priority, 0);
		m_pf3_tilemap->draw(screen, bitmap, cliprect, priority, 0);
		m_pf2_tilemap->draw(screen, bitmap, cliprect, priority, 0);
		m_pf1_tilemap->draw(screen, bitmap, cliprect, priority, 0);

		//if (pririoty==0x00)  m_spritegen->copy_sprites_from_tempbitmap(bitmap,cliprect,0);
		if (priority==0x04)  m_spritegen->copy_sprites_from_tempbitmap(bitmap,cliprect,1);
		if (priority==0x08)  m_spritegen->copy_sprites_from_tempbitmap(bitmap,cliprect,2);
		if (priority==0x0c)  m_spritegen->copy_sprites_from_tempbitmap(bitmap,cliprect,3);

	}

	return 0;
}

UINT32 toaplan1_state::screen_update_toaplan1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int priority;

	toaplan1_log_vram();

	screen.priority().fill(0, cliprect);

	// first draw everything, including "disabled" tiles and priority 0
	m_pf1_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE | TILEMAP_DRAW_ALL_CATEGORIES, 0);

	// then draw the higher priority layers in order
	for (priority = 1; priority < 16; priority++)
	{
		m_pf4_tilemap->draw(screen, bitmap, cliprect, priority, priority, 0);
		m_pf3_tilemap->draw(screen, bitmap, cliprect, priority, priority, 0);
		m_pf2_tilemap->draw(screen, bitmap, cliprect, priority, priority, 0);
		m_pf1_tilemap->draw(screen, bitmap, cliprect, priority, priority, 0);
	}

	draw_sprites(screen, bitmap, cliprect);
	return 0;
}

/****************************************************************************
    Spriteram is always 1 frame ahead, suggesting spriteram buffering.
    There are no CPU output registers that control this so we
    assume it happens automatically every frame, at the end of vblank
****************************************************************************/

void toaplan1_rallybik_state::screen_eof_rallybik(screen_device &screen, bool state)
{
	// rising edge
	if (state)
	{
		memcpy(m_buffered_spriteram.get(), m_spriteram, m_spriteram.bytes());
	}
}

void toaplan1_state::screen_eof_toaplan1(screen_device &screen, bool state)
{
	// rising edge
	if (state)
	{
		memcpy(m_buffered_spriteram.get(), m_spriteram, m_spriteram.bytes());
		memcpy(m_buffered_spritesizeram16.get(), m_spritesizeram16.get(), TOAPLAN1_SPRITESIZERAM_SIZE);
	}
}

void toaplan1_state::screen_eof_samesame(screen_device &screen, bool state)
{
	// rising edge
	if (state)
	{
		memcpy(m_buffered_spriteram.get(), m_spriteram, m_spriteram.bytes());
		memcpy(m_buffered_spritesizeram16.get(), m_spritesizeram16.get(), TOAPLAN1_SPRITESIZERAM_SIZE);
		m_maincpu->set_input_line(M68K_IRQ_2, HOLD_LINE);   /* Frame done */
	}
}
