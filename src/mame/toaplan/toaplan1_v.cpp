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
#include "toaplan1.h"


static constexpr u32 TOAPLAN1_TILEVRAM_SIZE      = 0x4000; /* 4 tile layers each this RAM size */
static constexpr u32 TOAPLAN1_SPRITERAM_SIZE     = 0x800;  /* sprite ram */
static constexpr u32 TOAPLAN1_SPRITESIZERAM_SIZE = 0x80;   /* sprite size ram */


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

template<unsigned Layer>
TILE_GET_INFO_MEMBER(toaplan1_state::get_tile_info)
{
	const u16 tile_number = m_tilevram[Layer][2 * tile_index + 1] & 0x7fff;
	const u16 attrib = m_tilevram[Layer][2 * tile_index];
	const u16 color = attrib & 0x3f;
	tileinfo.set(0,
			tile_number,
			color,
			0);
	// "disabled" tiles are behind everything else
	if (BIT(m_tilevram[Layer][2 * tile_index + 1], 15)) tileinfo.category = 16;
	else tileinfo.category = (attrib & 0xf000) >> 12;
}

/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void toaplan1_state::create_tilemaps()
{
	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(toaplan1_state::get_tile_info<0>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(toaplan1_state::get_tile_info<1>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_tilemap[2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(toaplan1_state::get_tile_info<2>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_tilemap[3] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(toaplan1_state::get_tile_info<3>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);

	m_tilemap[0]->set_transparent_pen(0);
	m_tilemap[1]->set_transparent_pen(0);
	m_tilemap[2]->set_transparent_pen(0);
	m_tilemap[3]->set_transparent_pen(0);
}

void toaplan1_state::vram_alloc()
{
	for (int i = 0; i < 4; i++)
	{
		m_tilevram[i] = make_unique_clear<u16[]>(TOAPLAN1_TILEVRAM_SIZE / 2);
		save_pointer(NAME(m_tilevram[i]), TOAPLAN1_TILEVRAM_SIZE / 2, i);
	}

#ifdef MAME_DEBUG
	m_display_pf[0] = true;
	m_display_pf[1] = true;
	m_display_pf[2] = true;
	m_display_pf[3] = true;
	m_displog = false;
#endif
}

void toaplan1_state::spritevram_alloc()
{
	m_buffered_spriteram = make_unique_clear<u16[]>(TOAPLAN1_SPRITERAM_SIZE / 2);
	m_spritesizeram = make_unique_clear<u16[]>(TOAPLAN1_SPRITESIZERAM_SIZE / 2);
	m_buffered_spritesizeram = make_unique_clear<u16[]>(TOAPLAN1_SPRITESIZERAM_SIZE / 2);

	save_pointer(NAME(m_buffered_spriteram), TOAPLAN1_SPRITERAM_SIZE / 2);
	save_pointer(NAME(m_spritesizeram), TOAPLAN1_SPRITESIZERAM_SIZE / 2);
	save_pointer(NAME(m_buffered_spritesizeram), TOAPLAN1_SPRITESIZERAM_SIZE / 2);
}

void toaplan1_state::set_scrolls()
{
	for (int i = 0; i < 4; i++)
	{
		m_tilemap[i]->set_scrollx(0, (m_scrollx[i] >> 7) - m_tiles_offsetx);
		m_tilemap[i]->set_scrolly(0, (m_scrolly[i] >> 7) - m_tiles_offsety);
	}
}

void toaplan1_state::register_common()
{
	save_item(NAME(m_bcu_flipscreen));
	save_item(NAME(m_fcu_flipscreen));

	save_item(NAME(m_scrollx));
	save_item(NAME(m_scrolly));

	save_item(NAME(m_tiles_offsetx));
	save_item(NAME(m_tiles_offsety));
	save_item(NAME(m_pf_voffs));
	save_item(NAME(m_spriteram_offs));

	m_pf_voffs = 0;
}


void toaplan1_rallybik_state::video_start()
{
	m_spritegen->gfx(0)->set_colorbase(64 * 16);

	create_tilemaps();
	vram_alloc();

	m_buffered_spriteram = make_unique_clear<u16[]>(m_spriteram.bytes() / 2);
	save_pointer(NAME(m_buffered_spriteram), m_spriteram.bytes() / 2);

	m_tilemap[0]->set_scrolldx(-0x00d - 6, -0x80 + 6);
	m_tilemap[1]->set_scrolldx(-0x00d - 4, -0x80 + 4);
	m_tilemap[2]->set_scrolldx(-0x00d - 2, -0x80 + 2);
	m_tilemap[3]->set_scrolldx(-0x00d - 0, -0x80 + 0);
	m_tilemap[0]->set_scrolldy(-0x111, 0x8);
	m_tilemap[1]->set_scrolldy(-0x111, 0x8);
	m_tilemap[2]->set_scrolldy(-0x111, 0x8);
	m_tilemap[3]->set_scrolldy(-0x111, 0x8);

	m_bcu_flipscreen = -1;
	m_fcu_flipscreen = false;

	register_common();
}

void toaplan1_state::video_start()
{
	create_tilemaps();
	vram_alloc();
	spritevram_alloc();

	m_tilemap[0]->set_scrolldx(-0x1ef - 6, -0x11 + 6);
	m_tilemap[1]->set_scrolldx(-0x1ef - 4, -0x11 + 4);
	m_tilemap[2]->set_scrolldx(-0x1ef - 2, -0x11 + 2);
	m_tilemap[3]->set_scrolldx(-0x1ef - 0, -0x11 + 0);
	m_tilemap[0]->set_scrolldy(-0x101, -0xff);
	m_tilemap[1]->set_scrolldy(-0x101, -0xff);
	m_tilemap[2]->set_scrolldy(-0x101, -0xff);
	m_tilemap[3]->set_scrolldy(-0x101, -0xff);

	m_bcu_flipscreen = -1;
	m_fcu_flipscreen = false;

	register_common();
}


/***************************************************************************

  Video I/O port hardware.

***************************************************************************/

u16 toaplan1_state::frame_done_r()
{
	return m_screen->vblank();
}

void toaplan1_state::tile_offsets_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (offset == 0)
	{
		COMBINE_DATA(&m_tiles_offsetx);
		logerror("Tiles_offsetx now = %08x\n", m_tiles_offsetx);
	}
	else
	{
		COMBINE_DATA(&m_tiles_offsety);
		logerror("Tiles_offsety now = %08x\n", m_tiles_offsety);
	}
	set_scrolls();
}

void toaplan1_state::bcu_flipscreen_w(u8 data)
{
	if (data != m_bcu_flipscreen)
	{
		logerror("Setting BCU controller flipscreen port to %02x\n", data);
		m_bcu_flipscreen = BIT(data, 0);     /* 0x0001 = flip, 0x0000 = no flip */
		machine().tilemap().set_flip_all((data ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0));

		set_scrolls();
	}
}

void toaplan1_state::fcu_flipscreen_w(u8 data)
{
	logerror("Setting FCU controller flipscreen port to %02x\n", data);
	m_fcu_flipscreen = BIT(data, 7);   /* 0x80 = flip, 0x00 = no flip */
}

u16 toaplan1_state::spriteram_offs_r() // this aint really needed ?
{
	return m_spriteram_offs;
}

void toaplan1_state::spriteram_offs_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_spriteram_offs);
}


void toaplan1_state::bgpalette_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_bgpaletteram[offset]);
	data = m_bgpaletteram[offset];
	m_palette->set_pen_color(offset, pal5bit(data >> 0), pal5bit(data >> 5), pal5bit(data >> 10));
}

void toaplan1_state::fgpalette_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_fgpaletteram[offset]);
	data = m_fgpaletteram[offset];
	m_palette->set_pen_color(offset + 64 * 16, pal5bit(data >> 0), pal5bit(data >> 5), pal5bit(data >> 10));
}


u16 toaplan1_state::spriteram_r()
{
	return m_spriteram[m_spriteram_offs & ((TOAPLAN1_SPRITERAM_SIZE / 2) - 1)];
}

void toaplan1_state::spriteram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_spriteram[m_spriteram_offs & ((TOAPLAN1_SPRITERAM_SIZE / 2) - 1)]);

#ifdef MAME_DEBUG
	if (m_spriteram_offs >= (TOAPLAN1_SPRITERAM_SIZE / 2))
	{
		logerror("Sprite_RAM_word_w, %08x out of range !\n", m_spriteram_offs);
		return;
	}
#endif

	m_spriteram_offs++;
}

u16 toaplan1_state::spritesizeram_r()
{
	return m_spritesizeram[m_spriteram_offs & ((TOAPLAN1_SPRITESIZERAM_SIZE / 2) - 1)];
}

void toaplan1_state::spritesizeram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_spritesizeram[m_spriteram_offs & ((TOAPLAN1_SPRITESIZERAM_SIZE / 2) - 1)]);

#ifdef MAME_DEBUG
	if (m_spriteram_offs >= (TOAPLAN1_SPRITESIZERAM_SIZE / 2))
	{
		logerror("Sprite_Size_RAM_word_w, %08x out of range !\n", m_spriteram_offs);
		return;
	}
#endif

	m_spriteram_offs++; /// really ? shouldn't happen on the sizeram
}



void toaplan1_state::bcu_control_w(offs_t offset, u16 data)
{
	logerror("BCU tile controller register:%02x now = %04x\n", offset, data);
}

u16 toaplan1_state::tileram_offs_r()
{
	return m_pf_voffs;
}

void toaplan1_state::tileram_offs_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (data >= 0x4000)
		logerror("Hmmm, unknown video layer being selected (%08x)\n", data);
	COMBINE_DATA(&m_pf_voffs);
}


u16 toaplan1_state::tileram_r(offs_t offset)
{
	const int layer = m_pf_voffs >> 12;
	const int offs = m_pf_voffs & 0xfff;
	u16 video_data = 0;

	switch (layer)    /* Locate Layer (PlayField) */
	{
		case 0x0:
		case 0x1:
		case 0x2:
		case 0x3:
		{
			offs_t vram_offset = ((offs * 2) + offset) & ((TOAPLAN1_TILEVRAM_SIZE / 2) - 1);
			video_data = m_tilevram[layer][vram_offset];
			break;
		}
		default:
			if (!machine().side_effects_disabled())
				logerror("Hmmm, reading %04x from unknown playfield layer address %06x  Offset:%01x !!!\n", video_data, m_pf_voffs, offset);
			break;
	}

	return video_data;
}

u16 toaplan1_rallybik_state::tileram_r(offs_t offset)
{
	u16 data = toaplan1_state::tileram_r(offset);

	if (offset == 0)    /* some bit lines may be stuck to others */
	{
		data |= ((data & 0xf000) >> 4);
		data |= ((data & 0x0030) << 2);
	}
	return data;
}

void toaplan1_state::tileram_w(offs_t offset, u16 data, u16 mem_mask)
{
	const int layer = m_pf_voffs >> 12;
	const int offs = m_pf_voffs & 0xfff;

	switch (layer)    /* Locate Layer (PlayField) */
	{
		case 0x0:
		case 0x1:
		case 0x2:
		case 0x3:
		{
			offs_t vram_offset = ((offs * 2) + offset) & ((TOAPLAN1_TILEVRAM_SIZE / 2) - 1);
			COMBINE_DATA(&m_tilevram[layer][vram_offset]);
			m_tilemap[layer]->mark_tile_dirty(vram_offset / 2);
			break;
		}
		default:
			logerror("Hmmm, writing %04x to unknown playfield layer address %06x  Offset:%01x\n", data, m_pf_voffs, offset);
			break;
	}
}



u16 toaplan1_state::scroll_regs_r(offs_t offset)
{
	const int layer = (offset >> 1);
	u16 scroll = 0;

	switch (offset)
	{
		case 00:
		case 02:
		case 04:
		case 06: scroll = m_scrollx[layer]; break;
		case 01:
		case 03:
		case 05:
		case 07: scroll = m_scrolly[layer]; break;
		default:
			if (!machine().side_effects_disabled())
				logerror("Hmmm, reading unknown video scroll register (%02x) !!!\n",offset);
			break;
	}
	return scroll;
}


void toaplan1_state::scroll_regs_w(offs_t offset, u16 data, u16 mem_mask)
{
	const int layer = (offset >> 1);
	switch (offset)
	{
		case 00:      /* 1D3h */
		case 02:      /* 1D5h */
		case 04:      /* 1D7h */
		case 06:      /* 1D9h */
			COMBINE_DATA(&m_scrollx[layer]);
			m_tilemap[layer]->set_scrollx(0, (m_scrollx[layer] >> 7) - m_tiles_offsetx);
			break;
		case 01:      /* 1EBh */
		case 03:      /* 1EBh */
		case 05:      /* 1EBh */
		case 07:      /* 1EBh */
			COMBINE_DATA(&m_scrolly[layer]);
			m_tilemap[layer]->set_scrolly(0, (m_scrolly[layer] >> 7) - m_tiles_offsety);
			break;
		default: logerror("Hmmm, writing %04x to unknown video scroll register (%02x) !!!\n",data ,offset);
			break;
	}
}




void toaplan1_state::log_vram()
{
#ifdef MAME_DEBUG

	if (machine().input().code_pressed(KEYCODE_M))
	{
		u16 const *const spriteram = m_spriteram;
		u16 const *const buffered_spriteram = m_buffered_spriteram.get();
		while (machine().input().code_pressed(KEYCODE_M)) ;
		if (m_spritesizeram)           /* FCU controller */
		{
			u16 const *const size  = (u16 *)(m_spritesizeram.get());
			u16 const *const bsize = (u16 *)(m_buffered_spritesizeram.get());
			logerror("Scrolls    PF1-X  PF1-Y     PF2-X  PF2-Y     PF3-X  PF3-Y     PF4-X  PF4-Y\n");
			logerror("------>    #%04x  #%04x     #%04x  #%04x     #%04x  #%04x     #%04x  #%04x\n",
				m_scrollx[0], m_scrolly[0], m_scrollx[1], m_scrolly[1], m_scrollx[2], m_scrolly[2], m_scrollx[3], m_scrolly[3]);
			for (offs_t sprite_voffs = 0; sprite_voffs < m_spriteram.bytes() / 2; sprite_voffs += 4)
			{
				const int bschar = buffered_spriteram[sprite_voffs];
				const int bsattr = buffered_spriteram[sprite_voffs + 1];
				const int bsxpos = buffered_spriteram[sprite_voffs + 2];
				const int bsypos = buffered_spriteram[sprite_voffs + 3];
				const int schar = spriteram[sprite_voffs];
				const int sattr = spriteram[sprite_voffs + 1];
				const int sxpos = spriteram[sprite_voffs + 2];
				const int sypos = spriteram[sprite_voffs + 3];
				logerror("$(%04x)  Tile-Attr-Xpos-Ypos Now:%04x %04x %04x.%01x %04x.%01x  nxt:%04x %04x %04x.%01x %04x.%01x\n", sprite_voffs,
												schar, sattr, sxpos, size[( sattr >> 6) & 0x3f] & 0xf, sypos,( size[( sattr >> 6) & 0x3f] >> 4) & 0xf,
											bschar,bsattr,bsxpos,bsize[(bsattr >> 6) & 0x3f] & 0xf,bsypos,(bsize[(bsattr >> 6) & 0x3f] >> 4) & 0xf);
			}
		}
		else                                    /* SCU controller */
		{
			logerror("Scrolls    PF1-X  PF1-Y     PF2-X  PF2-Y     PF3-X  PF3-Y     PF4-X  PF4-Y\n");
			logerror("------>    #%04x  #%04x     #%04x  #%04x     #%04x  #%04x     #%04x  #%04x\n",
				m_scrollx[0], m_scrolly[0], m_scrollx[1], m_scrolly[1], m_scrollx[2], m_scrolly[2], m_scrollx[3], m_scrolly[3]);
			for (offs_t sprite_voffs = 0; sprite_voffs < m_spriteram.bytes() / 2; sprite_voffs += 4)
			{
				const int bschar = buffered_spriteram[sprite_voffs];
				const int bsattr = buffered_spriteram[sprite_voffs + 1];
				const int bsypos = buffered_spriteram[sprite_voffs + 2];
				const int bsxpos = buffered_spriteram[sprite_voffs + 3];
				const int schar = spriteram[sprite_voffs];
				const int sattr = spriteram[sprite_voffs + 1];
				const int sypos = spriteram[sprite_voffs + 2];
				const int sxpos = spriteram[sprite_voffs + 3];
				logerror("$(%04x)  Tile-Attr-Xpos-Ypos Now:%04x %04x %04x %04x  nxt:%04x %04x %04x %04x\n", sprite_voffs,
												schar, sattr, sxpos, sypos,
											bschar,bsattr,bsxpos, bsypos);
			}
		}
	}

	if (machine().input().code_pressed(KEYCODE_SLASH))
	{
		while (machine().input().code_pressed(KEYCODE_SLASH)) ;
		if (m_spritesizeram)           /* FCU controller */
		{
			u16 const *const size  = (u16 *)(m_spritesizeram.get());
			u16 const *const bsize = (u16 *)(m_buffered_spritesizeram.get());
			logerror("Scrolls    PF1-X  PF1-Y     PF2-X  PF2-Y     PF3-X  PF3-Y     PF4-X  PF4-Y\n");
			logerror("------>    #%04x  #%04x     #%04x  #%04x     #%04x  #%04x     #%04x  #%04x\n",
				m_scrollx[0], m_scrolly[0], m_scrollx[1], m_scrolly[1], m_scrollx[2], m_scrolly[2], m_scrollx[3], m_scrolly[3]);
			for (offs_t offs = 0; offs < (TOAPLAN1_SPRITESIZERAM_SIZE / 2); offs += 4)
			{
				logerror("SizeOffs:%04x   now:%04x %04x %04x %04x    next: %04x %04x %04x %04x\n", offs,
												bsize[offs + 0], bsize[offs + 1],
												bsize[offs + 2], bsize[offs + 3],
												size[offs + 0], size[offs + 1],
												size[offs + 2], size[offs + 3]);
			}
		}
	}

	if (machine().input().code_pressed(KEYCODE_N))
	{
		int tchar[4]{}, tattr[4]{};
		while (machine().input().code_pressed(KEYCODE_N)) ;   /* BCU controller */
		logerror("Scrolls    PF1-X  PF1-Y     PF2-X  PF2-Y     PF3-X  PF3-Y     PF4-X  PF4-Y\n");
		logerror("------>    #%04x  #%04x     #%04x  #%04x     #%04x  #%04x     #%04x  #%04x\n",
			m_scrollx[0], m_scrolly[0], m_scrollx[1], m_scrolly[1], m_scrollx[2], m_scrolly[2], m_scrollx[3], m_scrolly[3]);
		for (offs_t tile_voffs = 0; tile_voffs < (TOAPLAN1_TILEVRAM_SIZE / 2); tile_voffs += 2)
		{
			tchar[0] = m_tilevram[0][tile_voffs + 1];
			tattr[0] = m_tilevram[0][tile_voffs];
			tchar[1] = m_tilevram[1][tile_voffs + 1];
			tattr[1] = m_tilevram[1][tile_voffs];
			tchar[2] = m_tilevram[2][tile_voffs + 1];
			tattr[2] = m_tilevram[2][tile_voffs];
			tchar[3] = m_tilevram[3][tile_voffs + 1];
			tattr[3] = m_tilevram[3][tile_voffs];
//          logerror("PF3 offs:%04x   Tile:%04x  Attr:%04x\n", tile_voffs, tchar, tattr);
			logerror("$(%04x)  Attr-Tile PF1:%04x-%04x  PF2:%04x-%04x  PF3:%04x-%04x  PF4:%04x-%04x\n", tile_voffs,
									tattr[0], tchar[0],  tattr[1], tchar[1],
									tattr[2], tchar[2],  tattr[3], tchar[3]);
		}
	}

	if (machine().input().code_pressed(KEYCODE_W))
	{
		while (machine().input().code_pressed(KEYCODE_W)) ;
		logerror("Mark here\n");
	}
	if (machine().input().code_pressed(KEYCODE_E))
	{
		while (machine().input().code_pressed(KEYCODE_E)) ;
		m_displog = !m_displog;
	}
	if (m_displog)
	{
		logerror("Scrolls    PF1-X  PF1-Y     PF2-X  PF2-Y     PF3-X  PF3-Y     PF4-X  PF4-Y\n");
		logerror("------>    #%04x  #%04x     #%04x  #%04x     #%04x  #%04x     #%04x  #%04x\n",
			m_scrollx[0], m_scrolly[0], m_scrollx[1], m_scrolly[1], m_scrollx[2], m_scrolly[2], m_scrollx[3], m_scrolly[3]);
	}
	if (machine().input().code_pressed(KEYCODE_L))      /* Turn Playfield 4 on/off */
	{
		while (machine().input().code_pressed(KEYCODE_L)) ;
		m_display_pf[3] = !m_display_pf[3];
		m_tilemap[3]->enable(m_display_pf[3]);
	}
	if (machine().input().code_pressed(KEYCODE_K))      /* Turn Playfield 3 on/off */
	{
		while (machine().input().code_pressed(KEYCODE_K)) ;
		m_display_pf[2] = !m_display_pf[2];
		m_tilemap[2]->enable(m_display_pf[2]);
	}
	if (machine().input().code_pressed(KEYCODE_J))      /* Turn Playfield 2 on/off */
	{
		while (machine().input().code_pressed(KEYCODE_J)) ;
		m_display_pf[1] = !m_display_pf[1];
		m_tilemap[1]->enable(m_display_pf[1]);
	}
	if (machine().input().code_pressed(KEYCODE_H))      /* Turn Playfield 1 on/off */
	{
		while (machine().input().code_pressed(KEYCODE_H)) ;
		m_display_pf[0] = !m_display_pf[0];
		m_tilemap[0]->enable(m_display_pf[0]);
	}
#endif
}



/***************************************************************************
    Sprite Handlers
***************************************************************************/

void toaplan1_rallybik_state::pri_cb(u8 priority, u32 &pri_mask)
{
	switch (priority)
	{
		case 0: pri_mask = GFX_PMASK_1|GFX_PMASK_2|GFX_PMASK_4|GFX_PMASK_8; break; // disable?
		case 1: pri_mask = GFX_PMASK_2|GFX_PMASK_4|GFX_PMASK_8;             break; // over tilemap priority ~0x4, under tilemap priority 0x5~
		case 2: pri_mask = GFX_PMASK_4|GFX_PMASK_8;                         break; // over tilemap priority ~0x8, under tilemap priority 0x9~
		case 3: pri_mask = GFX_PMASK_8;                                     break; // over tilemap priority ~0xc, under tilemap priority 0xd~
	}
}


void toaplan1_state::draw_sprites(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	u16 const *const source = (u16 *)m_buffered_spriteram.get();
	u16 const *const size   = (u16 *)m_buffered_spritesizeram.get();
	const bool fcu_flipscreen = m_fcu_flipscreen;

	for (int offs = m_spriteram.bytes() / 2 - 4; offs >= 0; offs -= 4)
	{
		if (BIT(~source[offs], 15))
		{
			const u16 attrib = source[offs + 1];
			const u8 priority = (attrib & 0xf000) >> 12;

			u32 sprite = source[offs] & 0x7fff;
			const u32 color = attrib & 0x3f;

			/****** find sprite dimension ******/
			const u32 sizeram_ptr = (attrib >> 6) & 0x3f;
			const u32 sprite_sizex = ( size[sizeram_ptr]       & 0x0f) * 8;
			const u32 sprite_sizey = ((size[sizeram_ptr] >> 4) & 0x0f) * 8;

			/****** find position to display sprite ******/
			int sx_base = (source[offs + 2] >> 7) & 0x1ff;
			int sy_base = (source[offs + 3] >> 7) & 0x1ff;

			if (sx_base >= 0x180) sx_base -= 0x200;
			if (sy_base >= 0x180) sy_base -= 0x200;

			/****** flip the sprite layer ******/
			if (fcu_flipscreen)
			{
				const rectangle &visarea = m_screen->visible_area();

				sx_base = visarea.width() - (sx_base + 8);  /* visarea.x = 320 */
				sy_base = visarea.height() - (sy_base + 8); /* visarea.y = 240 */
				sy_base += ((visarea.max_y + 1) - visarea.height()) * 2;    /* Horizontal games are offset so adjust by  + 0x20 */
			}

			for (int dim_y = 0; dim_y < sprite_sizey; dim_y += 8)
			{
				int sy;
				if (fcu_flipscreen) sy = sy_base - dim_y;
				else                sy = sy_base + dim_y;

				for (int dim_x = 0; dim_x < sprite_sizex; dim_x += 8)
				{
					int sx;
					if (fcu_flipscreen) sx = sx_base - dim_x;
					else                sx = sx_base + dim_x;

					m_gfxdecode->gfx(1)->prio_transpen(bitmap, cliprect,
												sprite, color,
												fcu_flipscreen, fcu_flipscreen,
												sx, sy,
												screen.priority(), u32(~0) << priority, 0); // ~0 for behind tilemap in same priority
					sprite++;
				}
			}
		}
	}
}




/***************************************************************************
    Draw the game screen in the given bitmap.
***************************************************************************/

u32 toaplan1_rallybik_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	log_vram();

	screen.priority().fill(0, cliprect);

	// first draw everything, including "disabled" tiles and priority 0
	m_tilemap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE | TILEMAP_DRAW_ALL_CATEGORIES, 1);

	// then draw the higher priority layers in order
	for (int priority = 1; priority < 16; priority++)
	{
		// get priority mask
		// value: 0x1 (tilemap priority 0x0~0x4), 0x2 (tilemap priority 0x5~0x8), 0x4 (tilemap priority 0x9~0xc), 0x8 (tilemap priority 0xd~)
		const u32 primask = 1 << (((priority - 1) >> 2));
		m_tilemap[3]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_CATEGORY(priority), primask);
		m_tilemap[2]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_CATEGORY(priority), primask);
		m_tilemap[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_CATEGORY(priority), primask);
		m_tilemap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_CATEGORY(priority), primask);
	}

	m_spritegen->draw_sprites(bitmap, cliprect, m_buffered_spriteram.get(), m_spriteram.bytes());
	return 0;
}

u32 toaplan1_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	log_vram();

	screen.priority().fill(0, cliprect);

	// first draw everything, including "disabled" tiles and priority 0
	m_tilemap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE | TILEMAP_DRAW_ALL_CATEGORIES, 0);

	// then draw the higher priority layers in order
	for (int priority = 1; priority < 16; priority++)
	{
		m_tilemap[3]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_CATEGORY(priority), priority, 0);
		m_tilemap[2]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_CATEGORY(priority), priority, 0);
		m_tilemap[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_CATEGORY(priority), priority, 0);
		m_tilemap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_CATEGORY(priority), priority, 0);
	}

	draw_sprites(screen, bitmap, cliprect);
	return 0;
}

/****************************************************************************
    Spriteram is always 1 frame ahead, suggesting spriteram buffering.
    There are no CPU output registers that control this so we
    assume it happens automatically every frame, at the end of vblank
****************************************************************************/

void toaplan1_rallybik_state::screen_vblank(int state)
{
	// rising edge
	if (state)
	{
		memcpy(m_buffered_spriteram.get(), m_spriteram, m_spriteram.bytes());
		interrupt();
	}
}

void toaplan1_state::screen_vblank(int state)
{
	// rising edge
	if (state)
	{
		memcpy(m_buffered_spriteram.get(), m_spriteram, m_spriteram.bytes());
		memcpy(m_buffered_spritesizeram.get(), m_spritesizeram.get(), TOAPLAN1_SPRITESIZERAM_SIZE);
		interrupt();
	}
}

void toaplan1_samesame_state::screen_vblank(int state)
{
	// rising edge
	if (state)
	{
		memcpy(m_buffered_spriteram.get(), m_spriteram, m_spriteram.bytes());
		memcpy(m_buffered_spritesizeram.get(), m_spritesizeram.get(), TOAPLAN1_SPRITESIZERAM_SIZE);
		interrupt();
		m_maincpu->set_input_line(M68K_IRQ_2, HOLD_LINE);   /* Frame done */
	}
}
