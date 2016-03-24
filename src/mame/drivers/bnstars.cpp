// license:BSD-3-Clause
// copyright-holders:David Haywood, ???
/*
Vs. Janshi Brandnew Stars
(c)1997 Jaleco

Single board version with Dual Screen output
(MS32 version also exists)

for the time being most of this driver is copied
from ms32.c, with some adjustments for dual screen.


Main PCB
--------

PCB ID  : MB-93141 EB91022-20081
CPU     : NEC D70632GD-20 (V70)
OSC     : 48.000MHz, 40.000MHz
RAM     : Cypress CY7C199-25 (x20)
          OKI M511664-80 (x8)
DIPs    : 8 position (x3)
OTHER   : Some PALs

          Custom chips:
                       JALECO SS91022-01 (208 PIN PQFP)
                       JALECO SS91022-02 (100 PIN PQFP) (x2)
                       JALECO SS91022-03 (176 PIN PQFP) (x2)
                       JALECO SS91022-05 (120 PIN PQFP) (x2)
                       JALECO SS91022-07 (208 PIN PQFP) (x2)
                       JALECO GS91022-01 (120 PIN PQFP)
                       JALECO GS91022-02 (160 PIN PQFP)
                       JALECO GS91022-03 (100 PIN PQFP)
                       JALECO GS91022-04 (100 PIN PQFP)
                       JALECO GS90015-03 ( 80 PIN PQFP) (x3) (not present on MS32)


ROMs:     None


ROM PCB (equivalent to MS32 cartridge)
--------------------------------------
PCB ID  : MB-93142 EB93007-20082
DIPs    : determine the size of ROMs?
          8 position (x1, 6 and 8 is on, others are off)
          4 position (x2, both are off on off off)
OTHER   : Some PALs
          Custom chip: JALECO SS92046-01 (144 pin PQFP) (x2)
          (located on small plug-in board with) (ID: SE93139 EB91022-30056)
ROMs    : MB-93142.36   [2eb6a503] (IC42, also printed "?u?????j???[Ver1.2")
          MB-93142.37   [49f60882] (IC57, also printed "?u?????j???[Ver1.2")
          MB-93142.38   [6e1312cd] (IC58, also printed "?u?????j???[Ver1.2")
          MB-93142.39   [56b98539] (IC59, also printed "?u?????j???[Ver1.2")

          VSJANSHI5.6   [fdbbac21] (IC9, actual label is "VS?W?????V 5 Ver1.0")
          VSJANSHI6.5   [fdbbac21] (IC8, actual label is "VS?W?????V 6 Ver1.0")

?@?@?@?@?@MR96004-01.20 [3366d104] (IC29)
?@?@?@?@?@MR96004-02.28 [ad556664] (IC49)
?@?@?@?@?@MR96004-03.21 [b399e2b1] (IC30)
?@?@?@?@?@MR96004-04.29 [f4f4cf4a] (IC50)
?@?@?@?@?@MR96004-05.22 [cd6c357e] (IC31)
?@?@?@?@?@MR96004-06.30 [fc6daad7] (IC51)
?@?@?@?@?@MR96004-07.23 [177e32fa] (IC32)
?@?@?@?@?@MR96004-08.31 [f6df27b2] (IC52)

?@?@?@?@?@MR96004-09.1  [603143e8] (IC4)
?@?@?@?@?@MR96004-09.7  [603143e8] (IC10)

?@?@?@?@?@MR96004-11.11 [e6da552c] (IC17)
?@?@?@?@?@MR96004-11.13 [e6da552c] (IC19)


Sound PCB
---------
PCB ID  : SB-93143 EB91022-20083
SOUND   : Z80, YMF271-F, YAC513(x2)
OSC     : 8.000MHz, 16.9344MHz
DIPs    : 4 position (all off)
ROMs    : SB93145.5     [0424e899] (IC34, also printed "Ver1.0") - Sound program
          MR96004-10.1  [125661cd] (IC19 - Samples)

Sound sub PCB
-------------
PCB ID  : SB-93145 EB93007-20086
SOUND   : YMF271-F
ROMs    : MR96004-10.1  [125661cd] (IC5 - Samples)

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/v60/v60.h"
#include "rendlay.h"
#include "sound/ymf271.h"
#include "includes/ms32.h"
#include "machine/jalcrpt.h"


class bnstars_state : public ms32_state
{
public:
	bnstars_state(const machine_config &mconfig, device_type type, const char *tag)
		: ms32_state(mconfig, type, tag),
			m_ms32_tx0_ram(*this, "tx0_ram"),
			m_ms32_tx1_ram(*this, "tx1_ram"),
			m_ms32_bg0_ram(*this, "bg0_ram"),
			m_ms32_bg1_ram(*this, "bg1_ram"),
			m_ms32_roz0_ram(*this, "roz0_ram"),
			m_ms32_roz1_ram(*this, "roz1_ram"),
			m_ms32_roz_ctrl(*this, "roz_ctrl"),
			m_ms32_spram(*this, "spram"),
			m_ms32_tx0_scroll(*this, "tx0_scroll"),
			m_ms32_bg0_scroll(*this, "bg0_scroll"),
			m_ms32_tx1_scroll(*this, "tx1_scroll"),
			m_ms32_bg1_scroll(*this, "bg1_scroll"),
			m_p1_keys(*this, "P1KEY"),
			m_p2_keys(*this, "P2KEY") { }

	tilemap_t *m_ms32_tx_tilemap[2];
	tilemap_t *m_ms32_bg_tilemap[2];
	tilemap_t *m_ms32_roz_tilemap[2];
	required_shared_ptr<UINT32> m_ms32_tx0_ram;
	required_shared_ptr<UINT32> m_ms32_tx1_ram;
	required_shared_ptr<UINT32> m_ms32_bg0_ram;
	required_shared_ptr<UINT32> m_ms32_bg1_ram;
	required_shared_ptr<UINT32> m_ms32_roz0_ram;
	required_shared_ptr<UINT32> m_ms32_roz1_ram;
	required_shared_ptr_array<UINT32, 2> m_ms32_roz_ctrl;
	required_shared_ptr<UINT32> m_ms32_spram;
	required_shared_ptr<UINT32> m_ms32_tx0_scroll;
	required_shared_ptr<UINT32> m_ms32_bg0_scroll;
	required_shared_ptr<UINT32> m_ms32_tx1_scroll;
	required_shared_ptr<UINT32> m_ms32_bg1_scroll;

	required_ioport_array<4> m_p1_keys;
	required_ioport_array<4> m_p2_keys;

	UINT32 m_bnstars1_mahjong_select;
	DECLARE_WRITE32_MEMBER(ms32_tx0_ram_w);
	DECLARE_WRITE32_MEMBER(ms32_tx1_ram_w);
	DECLARE_WRITE32_MEMBER(ms32_bg0_ram_w);
	DECLARE_WRITE32_MEMBER(ms32_bg1_ram_w);
	DECLARE_WRITE32_MEMBER(ms32_roz0_ram_w);
	DECLARE_WRITE32_MEMBER(ms32_roz1_ram_w);
	DECLARE_WRITE32_MEMBER(bnstars1_mahjong_select_w);
	DECLARE_CUSTOM_INPUT_MEMBER(mahjong_ctrl_r);
	DECLARE_DRIVER_INIT(bnstars);
	TILE_GET_INFO_MEMBER(get_ms32_tx0_tile_info);
	TILE_GET_INFO_MEMBER(get_ms32_tx1_tile_info);
	TILE_GET_INFO_MEMBER(get_ms32_bg0_tile_info);
	TILE_GET_INFO_MEMBER(get_ms32_bg1_tile_info);
	TILE_GET_INFO_MEMBER(get_ms32_roz0_tile_info);
	TILE_GET_INFO_MEMBER(get_ms32_roz1_tile_info);
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_bnstars_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_bnstars_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_roz(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int priority, int chip);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, UINT32 *sprram_top, size_t sprram_size);
};



TILE_GET_INFO_MEMBER(bnstars_state::get_ms32_tx0_tile_info)
{
	int tileno, colour;

	tileno = m_ms32_tx0_ram[tile_index *2+0] & 0x0000ffff;
	colour = m_ms32_tx0_ram[tile_index *2+1] & 0x0000000f;

	SET_TILE_INFO_MEMBER(3,tileno,colour,0);
}

TILE_GET_INFO_MEMBER(bnstars_state::get_ms32_tx1_tile_info)
{
	int tileno, colour;

	tileno = m_ms32_tx1_ram[tile_index *2+0] & 0x0000ffff;
	colour = m_ms32_tx1_ram[tile_index *2+1] & 0x0000000f;

	SET_TILE_INFO_MEMBER(6,tileno,colour,0);
}

WRITE32_MEMBER(bnstars_state::ms32_tx0_ram_w)
{
	COMBINE_DATA(&m_ms32_tx0_ram[offset]);
	m_ms32_tx_tilemap[0]->mark_tile_dirty(offset/2);
}

WRITE32_MEMBER(bnstars_state::ms32_tx1_ram_w)
{
	COMBINE_DATA(&m_ms32_tx1_ram[offset]);
	m_ms32_tx_tilemap[1]->mark_tile_dirty(offset/2);
}

/* BG Layers */

TILE_GET_INFO_MEMBER(bnstars_state::get_ms32_bg0_tile_info)
{
	int tileno,colour;

	tileno = m_ms32_bg0_ram[tile_index *2+0] & 0x0000ffff;
	colour = m_ms32_bg0_ram[tile_index *2+1] & 0x0000000f;

	SET_TILE_INFO_MEMBER(2,tileno,colour,0);
}

TILE_GET_INFO_MEMBER(bnstars_state::get_ms32_bg1_tile_info)
{
	int tileno,colour;

	tileno = m_ms32_bg1_ram[tile_index *2+0] & 0x0000ffff;
	colour = m_ms32_bg1_ram[tile_index *2+1] & 0x0000000f;

	SET_TILE_INFO_MEMBER(5,tileno,colour,0);
}

WRITE32_MEMBER(bnstars_state::ms32_bg0_ram_w)
{
	COMBINE_DATA(&m_ms32_bg0_ram[offset]);
	m_ms32_bg_tilemap[0]->mark_tile_dirty(offset/2);
}

WRITE32_MEMBER(bnstars_state::ms32_bg1_ram_w)
{
	COMBINE_DATA(&m_ms32_bg1_ram[offset]);
	m_ms32_bg_tilemap[1]->mark_tile_dirty(offset/2);
}

/* ROZ Layers */

void bnstars_state::draw_roz(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int priority, int chip)
{
	/* TODO: registers 0x40/4 / 0x44/4 and 0x50/4 / 0x54/4 are used, meaning unknown */

	if (m_ms32_roz_ctrl[chip][0x5c/4] & 1)   /* "super" mode */
	{
		printf("no lineram!\n");
		return;
		/*
		rectangle my_clip;
		int y,maxy;

		my_clip.min_x = cliprect.min_x;
		my_clip.max_x = cliprect.max_x;

		y = cliprect.min_y;
		maxy = cliprect.max_y;

		while (y <= maxy)
		{
		    UINT32 *lineaddr = ms32_lineram + 8 * (y & 0xff);

		    int start2x = (lineaddr[0x00/4] & 0xffff) | ((lineaddr[0x04/4] & 3) << 16);
		    int start2y = (lineaddr[0x08/4] & 0xffff) | ((lineaddr[0x0c/4] & 3) << 16);
		    int incxx  = (lineaddr[0x10/4] & 0xffff) | ((lineaddr[0x14/4] & 1) << 16);
		    int incxy  = (lineaddr[0x18/4] & 0xffff) | ((lineaddr[0x1c/4] & 1) << 16);
		    int startx = (m_ms32_roz_ctrl[0x00/4] & 0xffff) | ((m_ms32_roz_ctrl[0x04/4] & 3) << 16);
		    int starty = (m_ms32_roz_ctrl[0x08/4] & 0xffff) | ((m_ms32_roz_ctrl[0x0c/4] & 3) << 16);
		    int offsx  = m_ms32_roz_ctrl[0x30/4];
		    int offsy  = m_ms32_roz_ctrl[0x34/4];

		    my_clip.min_y = my_clip.max_y = y;

		    offsx += (m_ms32_roz_ctrl[0x38/4] & 1) * 0x400;   // ??? gratia, hayaosi1...
		    offsy += (m_ms32_roz_ctrl[0x3c/4] & 1) * 0x400;   // ??? gratia, hayaosi1...

		    // extend sign
		    if (start2x & 0x20000) start2x |= ~0x3ffff;
		    if (start2y & 0x20000) start2y |= ~0x3ffff;
		    if (startx & 0x20000) startx |= ~0x3ffff;
		    if (starty & 0x20000) starty |= ~0x3ffff;
		    if (incxx & 0x10000) incxx |= ~0x1ffff;
		    if (incxy & 0x10000) incxy |= ~0x1ffff;

		    m_ms32_roz_tilemap->draw_roz(screen, bitmap, &my_clip,
		            (start2x+startx+offsx)<<16, (start2y+starty+offsy)<<16,
		            incxx<<8, incxy<<8, 0, 0,
		            1, // Wrap
		            0, priority);

		    y++;
		}
		*/
	}
	else    /* "simple" mode */
	{
		int startx = (m_ms32_roz_ctrl[chip][0x00/4] & 0xffff) | ((m_ms32_roz_ctrl[chip][0x04/4] & 3) << 16);
		int starty = (m_ms32_roz_ctrl[chip][0x08/4] & 0xffff) | ((m_ms32_roz_ctrl[chip][0x0c/4] & 3) << 16);
		int incxx  = (m_ms32_roz_ctrl[chip][0x10/4] & 0xffff) | ((m_ms32_roz_ctrl[chip][0x14/4] & 1) << 16);
		int incxy  = (m_ms32_roz_ctrl[chip][0x18/4] & 0xffff) | ((m_ms32_roz_ctrl[chip][0x1c/4] & 1) << 16);
		int incyy  = (m_ms32_roz_ctrl[chip][0x20/4] & 0xffff) | ((m_ms32_roz_ctrl[chip][0x24/4] & 1) << 16);
		int incyx  = (m_ms32_roz_ctrl[chip][0x28/4] & 0xffff) | ((m_ms32_roz_ctrl[chip][0x2c/4] & 1) << 16);
		int offsx  = m_ms32_roz_ctrl[chip][0x30/4];
		int offsy  = m_ms32_roz_ctrl[chip][0x34/4];

		offsx += (m_ms32_roz_ctrl[chip][0x38/4] & 1) * 0x400;    // ??? gratia, hayaosi1...
		offsy += (m_ms32_roz_ctrl[chip][0x3c/4] & 1) * 0x400;    // ??? gratia, hayaosi1...

		/* extend sign */
		if (startx & 0x20000) startx |= ~0x3ffff;
		if (starty & 0x20000) starty |= ~0x3ffff;
		if (incxx & 0x10000) incxx |= ~0x1ffff;
		if (incxy & 0x10000) incxy |= ~0x1ffff;
		if (incyy & 0x10000) incyy |= ~0x1ffff;
		if (incyx & 0x10000) incyx |= ~0x1ffff;

		m_ms32_roz_tilemap[chip]->draw_roz(screen, bitmap, cliprect,
				(startx+offsx)<<16, (starty+offsy)<<16,
				incxx<<8, incxy<<8, incyx<<8, incyy<<8,
				1, // Wrap
				0, priority);
	}
}


TILE_GET_INFO_MEMBER(bnstars_state::get_ms32_roz0_tile_info)
{
	int tileno,colour;

	tileno = m_ms32_roz0_ram[tile_index *2+0] & 0x0000ffff;
	colour = m_ms32_roz0_ram[tile_index *2+1] & 0x0000000f;

	SET_TILE_INFO_MEMBER(1,tileno,colour,0);
}

TILE_GET_INFO_MEMBER(bnstars_state::get_ms32_roz1_tile_info)
{
	int tileno,colour;

	tileno = m_ms32_roz1_ram[tile_index *2+0] & 0x0000ffff;
	colour = m_ms32_roz1_ram[tile_index *2+1] & 0x0000000f;

	SET_TILE_INFO_MEMBER(4,tileno,colour,0);
}

WRITE32_MEMBER(bnstars_state::ms32_roz0_ram_w)
{
	COMBINE_DATA(&m_ms32_roz0_ram[offset]);
	m_ms32_roz_tilemap[0]->mark_tile_dirty(offset/2);
}

WRITE32_MEMBER(bnstars_state::ms32_roz1_ram_w)
{
	COMBINE_DATA(&m_ms32_roz1_ram[offset]);
	m_ms32_roz_tilemap[1]->mark_tile_dirty(offset/2);
}


/* SPRITES based on tetrisp2 for now, readd priority bits later */
void bnstars_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, UINT32 *sprram_top, size_t sprram_size)
{
/***************************************************************************


                                Sprites Drawing

    Offset:     Bits:                   Meaning:

    0.w         fedc ba98 ---- ----
                ---- ---- 7654 ----     Priority
                ---- ---- ---- 3---
                ---- ---- ---- -2--     Draw this sprite
                ---- ---- ---- --1-     Flip Y
                ---- ---- ---- ---0     Flip X

    1.w         fedc ba98 ---- ----     Tile's Y position in the tile page (*)
                ---- ---- 7654 3210     Tile's X position in the tile page (*)

    2.w         fedc ---- ---- ----     Color
                ---- ba98 7654 3210     Tile Page (32x32 tiles = 256x256 pixels each)

    3.w         fedc ba98 ---- ----     Y Size - 1 (*)
                ---- ---- 7654 3210     X Size - 1 (*)

    4.w         fedc ba-- ---- ----
                ---- --98 7654 3210     Y (Signed)

    5.w         fedc b--- ---- ----
                ---- -a98 7654 3210     X (Signed)

    6.w         fedc ba98 7654 3210     Zoom Y

    7.w         fedc ba98 7654 3210     Zoom X

(*) 1 pixel granularity

***************************************************************************/

	int tx, ty, sx, sy, flipx, flipy;
	int xsize, ysize, xzoom, yzoom;
	int code, attr, color, size, pri, pri_mask;
	gfx_element *gfx = m_gfxdecode->gfx(0);

	UINT32      *source = sprram_top;
	const UINT32    *finish = sprram_top + (sprram_size - 0x10) / 4;


	if (m_reverse_sprite_order == 1)
	{
		source  = sprram_top + (sprram_size - 0x10) / 4;
		finish  = sprram_top;
	}


	for (;m_reverse_sprite_order ? (source>=finish) : (source<finish); m_reverse_sprite_order ? (source-=4) : (source+=4))
	{
		attr    =   source[ 0 ];

		if ((attr & 0x0004) == 0)           continue;

		flipx   =   attr & 1;
		flipy   =   attr & 2;

		pri = (attr >> 4)&0xf;

		code    =   source[ 1 ];
		color   =   source[ 2 ];

		tx      =   (code >> 0) & 0xff;
		ty      =   (code >> 8) & 0xff;

		code    =   (color & 0x0fff);

		color   =   (color >> 12) & 0xf;

		size    =   source[ 3 ];

		xsize   =   ((size >> 0) & 0xff) + 1;
		ysize   =   ((size >> 8) & 0xff) + 1;

		sy      =   source[ 4 ];
		sx      =   source[ 5 ];

		sx      =   (sx & 0x3ff) - (sx & 0x400);
		sy      =   (sy & 0x1ff) - (sy & 0x200);

		xzoom   =   (source[ 6 ]&0xffff);
		yzoom   =   (source[ 7 ]&0xffff);

		if (!yzoom || !xzoom)               continue;

		yzoom = 0x1000000/yzoom;
		xzoom = 0x1000000/xzoom;

		// there are surely also shadows (see gametngk) but how they're enabled we don't know

		if (m_flipscreen)
		{
			sx = 320 - ((xsize*xzoom)>>16) - sx;
			sy = 224 - ((ysize*yzoom)>>16) - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		/* TODO: priority handling is completely wrong, but better than nothing */
		if (pri == 0x0)
			pri_mask = 0x00;
		else if (pri <= 0xd)
			pri_mask = 0xf0;
		else if (pri <= 0xe)
			pri_mask = 0xfc;
		else
			pri_mask = 0xfe;

		gfx->set_source_clip(tx, xsize, ty, ysize);
		gfx->prio_zoom_transpen(bitmap,cliprect,
				code,
				color,
				flipx, flipy,
				sx,sy,
				xzoom, yzoom, screen.priority(),pri_mask, 0);
	}   /* end sprite loop */
}


void bnstars_state::video_start()
{
	m_ms32_tx_tilemap[0] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(bnstars_state::get_ms32_tx0_tile_info),this),TILEMAP_SCAN_ROWS, 8, 8,64,64);
	m_ms32_tx_tilemap[1] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(bnstars_state::get_ms32_tx1_tile_info),this),TILEMAP_SCAN_ROWS, 8, 8,64,64);
	m_ms32_tx_tilemap[0]->set_transparent_pen(0);
	m_ms32_tx_tilemap[1]->set_transparent_pen(0);

	m_ms32_bg_tilemap[0] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(bnstars_state::get_ms32_bg0_tile_info),this),TILEMAP_SCAN_ROWS,16,16,64,64);
	m_ms32_bg_tilemap[1] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(bnstars_state::get_ms32_bg1_tile_info),this),TILEMAP_SCAN_ROWS,16,16,64,64);
	m_ms32_bg_tilemap[0]->set_transparent_pen(0);
	m_ms32_bg_tilemap[1]->set_transparent_pen(0);

	m_ms32_roz_tilemap[0] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(bnstars_state::get_ms32_roz0_tile_info),this),TILEMAP_SCAN_ROWS,16,16,128,128);
	m_ms32_roz_tilemap[1] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(bnstars_state::get_ms32_roz1_tile_info),this),TILEMAP_SCAN_ROWS,16,16,128,128);
	m_ms32_roz_tilemap[0]->set_transparent_pen(0);
	m_ms32_roz_tilemap[1]->set_transparent_pen(0);


}





UINT32 bnstars_state::screen_update_bnstars_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);

	bitmap.fill(0, cliprect);   /* bg color */


	m_ms32_bg_tilemap[0]->set_scrollx(0, m_ms32_bg0_scroll[0x00/4] + m_ms32_bg0_scroll[0x08/4] + 0x10 );
	m_ms32_bg_tilemap[0]->set_scrolly(0, m_ms32_bg0_scroll[0x0c/4] + m_ms32_bg0_scroll[0x14/4] );
	m_ms32_bg_tilemap[0]->draw(screen, bitmap, cliprect, 0,1);

	draw_roz(screen,bitmap,cliprect,2,0);

	m_ms32_tx_tilemap[0]->set_scrollx(0, m_ms32_tx0_scroll[0x00/4] + m_ms32_tx0_scroll[0x08/4] + 0x18);
	m_ms32_tx_tilemap[0]->set_scrolly(0, m_ms32_tx0_scroll[0x0c/4] + m_ms32_tx0_scroll[0x14/4]);
	m_ms32_tx_tilemap[0]->draw(screen, bitmap, cliprect, 0,4);


	draw_sprites(screen,bitmap,cliprect, m_ms32_spram, 0x20000);

	return 0;
}

UINT32 bnstars_state::screen_update_bnstars_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);

	bitmap.fill(0, cliprect);    /* bg color */


	m_ms32_bg_tilemap[1]->set_scrollx(0, m_ms32_bg1_scroll[0x00/4] + m_ms32_bg1_scroll[0x08/4] + 0x10 );
	m_ms32_bg_tilemap[1]->set_scrolly(0, m_ms32_bg1_scroll[0x0c/4] + m_ms32_bg1_scroll[0x14/4] );
	m_ms32_bg_tilemap[1]->draw(screen, bitmap, cliprect, 0,1);

	draw_roz(screen,bitmap,cliprect,2,1);

	m_ms32_tx_tilemap[1]->set_scrollx(0, m_ms32_tx1_scroll[0x00/4] + m_ms32_tx1_scroll[0x08/4] + 0x18);
	m_ms32_tx_tilemap[1]->set_scrolly(0, m_ms32_tx1_scroll[0x0c/4] + m_ms32_tx1_scroll[0x14/4]);
	m_ms32_tx_tilemap[1]->draw(screen, bitmap, cliprect, 0,4);

	draw_sprites(screen,bitmap,cliprect, m_ms32_spram+(0x20000/4), 0x20000);

	return 0;
}

static INPUT_PORTS_START( bnstars )
	PORT_START("P1")
	PORT_BIT( 0x000000ff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, bnstars_state, mahjong_ctrl_r, (void *)0)
	PORT_BIT( 0x0000ff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Test?") PORT_CODE(KEYCODE_F1)

	PORT_START("P1KEY.0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1KEY.1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1KEY.2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1KEY.3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x000000ff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, bnstars_state, mahjong_ctrl_r, (void *)1)
	PORT_BIT( 0x0000ff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Test?") PORT_CODE(KEYCODE_F2)

	PORT_START("P2KEY.0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_A ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_E ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_I ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_KAN ) PORT_PLAYER(2)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2KEY.1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_B ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_F ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_J ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_REACH ) PORT_PLAYER(2)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2KEY.2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_C ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_G ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_K ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_RON ) PORT_PLAYER(2)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2KEY.3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_D ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_H ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_L ) PORT_PLAYER(2)
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME(     0x00000001, 0x00000001, "Test Mode" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(  0x00000001, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00000002, 0x00000002, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(  0x00000002, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x0000001c, 0x0000001c, "First Point" ) PORT_DIPLOCATION("SW1:6,5,4")
	PORT_DIPSETTING(  0x0000000c, "5000"  )
	PORT_DIPSETTING(  0x00000014, "10000" )
	PORT_DIPSETTING(  0x00000004, "15000" )
	PORT_DIPSETTING(  0x0000001c, "20000" )
	PORT_DIPSETTING(  0x00000018, "23000" )
	PORT_DIPSETTING(  0x00000008, "25000" )
	PORT_DIPSETTING(  0x00000010, "26000" )
	PORT_DIPSETTING(  0x00000000, "30000" )
	PORT_DIPNAME(     0x000000e0, 0x000000e0, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:3,2,1")
	PORT_DIPSETTING(  0x00000000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(  0x00000080, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(  0x00000040, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(  0x000000c0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(  0x000000e0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(  0x00000060, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(  0x000000a0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(  0x00000020, DEF_STR( 1C_4C ) )

	PORT_DIPUNUSED_DIPLOC( 0x00000100, 0x00000100, "SW2:8" )
	PORT_DIPUNUSED_DIPLOC( 0x00000200, 0x00000200, "SW2:7" )
	PORT_DIPNAME(     0x00000400, 0x00000400, "Taisen Only" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(  0x00000400, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00000800, 0x00000800, "Tumo Pinfu" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(  0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000800, DEF_STR( On ) )
	PORT_DIPNAME(     0x00001000, 0x00001000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(  0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00001000, DEF_STR( On ) )
	PORT_DIPNAME(     0x0000e000, 0x0000e000, DEF_STR ( Difficulty ) ) PORT_DIPLOCATION("SW2:3,2,1")
	PORT_DIPSETTING(  0x00000000, DEF_STR( Easiest ) )
	PORT_DIPSETTING(  0x00008000, DEF_STR( Very_Easy) )
	PORT_DIPSETTING(  0x00004000, DEF_STR( Easier ) )
	PORT_DIPSETTING(  0x0000c000, DEF_STR( Easy ) )
	PORT_DIPSETTING(  0x0000e000, DEF_STR( Normal ) )
	PORT_DIPSETTING(  0x00006000, DEF_STR( Hard ) )
	PORT_DIPSETTING(  0x0000a000, DEF_STR( Harder ) )
	PORT_DIPSETTING(  0x00002000, DEF_STR( Hardest ) )

	PORT_DIPUNUSED_DIPLOC( 0x00010000, 0x00010000, "SW3:8" )
	PORT_DIPUNUSED_DIPLOC( 0x00020000, 0x00020000, "SW3:7" )
	PORT_DIPUNUSED_DIPLOC( 0x00040000, 0x00040000, "SW3:6" )
	PORT_DIPUNUSED_DIPLOC( 0x00080000, 0x00080000, "SW3:5" )
	PORT_DIPUNUSED_DIPLOC( 0x00100000, 0x00100000, "SW3:4" )
	PORT_DIPUNUSED_DIPLOC( 0x00200000, 0x00200000, "SW3:3" )
	PORT_DIPUNUSED_DIPLOC( 0x00400000, 0x00400000, "SW3:2" )
	PORT_DIPUNUSED_DIPLOC( 0x00800000, 0x00800000, "SW3:1" )
	PORT_BIT( 0xff000000, IP_ACTIVE_LOW, IPT_UNUSED )   // Unused?
INPUT_PORTS_END


/* sprites are contained in 256x256 "tiles" */
static GFXLAYOUT_RAW( spritelayout, 256, 256, 256*8, 256*256*8 )
static GFXLAYOUT_RAW( bglayout, 16, 16, 16*8, 16*16*8 )
static GFXLAYOUT_RAW( txlayout, 8, 8, 8*8, 8*8*8 )

static GFXDECODE_START( bnstars )
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout, 0x0000, 0x10 )
	GFXDECODE_ENTRY( "gfx2", 0, bglayout,     0x5000, 0x10 ) /* Roz scr1 */
	GFXDECODE_ENTRY( "gfx4", 0, bglayout,     0x1000, 0x10 ) /* Bg scr1 */
	GFXDECODE_ENTRY( "gfx5", 0, txlayout,     0x6000, 0x10 ) /* Tx scr1 */

	GFXDECODE_ENTRY( "gfx3", 0, bglayout,     0x5000, 0x10 ) /* Roz scr2 */
	GFXDECODE_ENTRY( "gfx6", 0, bglayout,     0x1000, 0x10 ) /* Bg scr2 */
	GFXDECODE_ENTRY( "gfx7", 0, txlayout,     0x6000, 0x10 ) /* Tx scr2 */
GFXDECODE_END

CUSTOM_INPUT_MEMBER(bnstars_state::mahjong_ctrl_r)
{
	required_ioport_array<4> &keys = (((int)(FPTR)param) == 0) ? m_p1_keys : m_p2_keys;

	switch (m_bnstars1_mahjong_select & 0x2080)
	{
		default:
			printf("unk bnstars1_r %08x\n",m_bnstars1_mahjong_select);
			return 0xff;

		case 0x0000:
			return keys[0]->read();

		case 0x0080:
			return keys[1]->read();

		case 0x2000:
			return keys[2]->read();

		case 0x2080:
			return keys[3]->read();
	}
}

WRITE32_MEMBER(bnstars_state::bnstars1_mahjong_select_w)
{
	m_bnstars1_mahjong_select = data;
//  printf("%08x\n",m_bnstars1_mahjong_select);
}


static ADDRESS_MAP_START( bnstars_map, AS_PROGRAM, 32, bnstars_state )
	AM_RANGE(0x00000000, 0x001fffff) AM_ROM

	AM_RANGE(0xfc800000, 0xfc800003) AM_WRITE(ms32_sound_w)

	AM_RANGE(0xfcc00004, 0xfcc00007) AM_READ_PORT("P1")
	AM_RANGE(0xfcc00008, 0xfcc0000b) AM_READ_PORT("P2")
	AM_RANGE(0xfcc00010, 0xfcc00013) AM_READ_PORT("DSW")

	AM_RANGE(0xfce00034, 0xfce00037) AM_WRITENOP
	AM_RANGE(0xfce00038, 0xfce0003b) AM_WRITE(reset_sub_w)

	AM_RANGE(0xfce00050, 0xfce00053) AM_WRITENOP
	AM_RANGE(0xfce00058, 0xfce0005b) AM_WRITENOP
	AM_RANGE(0xfce0005c, 0xfce0005f) AM_WRITENOP
	AM_RANGE(0xfce00300, 0xfce00303) AM_WRITENOP

	AM_RANGE(0xfce00400, 0xfce0045f) AM_WRITEONLY AM_SHARE("roz_ctrl.0")
	AM_RANGE(0xfce00700, 0xfce0075f) AM_WRITEONLY AM_SHARE("roz_ctrl.1") // guess
	AM_RANGE(0xfce00a00, 0xfce00a17) AM_WRITEONLY AM_SHARE("tx0_scroll")
	AM_RANGE(0xfce00a20, 0xfce00a37) AM_WRITEONLY AM_SHARE("bg0_scroll")
	AM_RANGE(0xfce00c00, 0xfce00c17) AM_WRITEONLY AM_SHARE("tx1_scroll")
	AM_RANGE(0xfce00c20, 0xfce00c37) AM_WRITEONLY AM_SHARE("bg1_scroll")

	AM_RANGE(0xfce00e00, 0xfce00e03) AM_WRITE(bnstars1_mahjong_select_w) // ?

	AM_RANGE(0xfd000000, 0xfd000003) AM_READ(ms32_sound_r)

	/* wrote together */
	AM_RANGE(0xfd040000, 0xfd047fff) AM_RAM // priority ram
	AM_RANGE(0xfd080000, 0xfd087fff) AM_RAM
	AM_RANGE(0xfd200000, 0xfd237fff) AM_DEVREADWRITE16("palette2", palette_device, read, write, 0x0000ffff) AM_SHARE("palette2")
	AM_RANGE(0xfd400000, 0xfd437fff) AM_DEVREADWRITE16("palette", palette_device, read, write, 0x0000ffff) AM_SHARE("palette")
	AM_RANGE(0xfe000000, 0xfe01ffff) AM_RAM_WRITE(ms32_roz1_ram_w) AM_SHARE("roz1_ram")
	AM_RANGE(0xfe400000, 0xfe41ffff) AM_RAM_WRITE(ms32_roz0_ram_w) AM_SHARE("roz0_ram")
	AM_RANGE(0xfe800000, 0xfe83ffff) AM_RAM AM_SHARE("spram")
	AM_RANGE(0xfea00000, 0xfea07fff) AM_RAM_WRITE(ms32_tx1_ram_w) AM_SHARE("tx1_ram")
	AM_RANGE(0xfea08000, 0xfea0ffff) AM_RAM_WRITE(ms32_bg1_ram_w) AM_SHARE("bg1_ram")
	AM_RANGE(0xfec00000, 0xfec07fff) AM_RAM_WRITE(ms32_tx0_ram_w) AM_SHARE("tx0_ram")
	AM_RANGE(0xfec08000, 0xfec0ffff) AM_RAM_WRITE(ms32_bg0_ram_w) AM_SHARE("bg0_ram")

	AM_RANGE(0xfee00000, 0xfee1ffff) AM_RAM
	AM_RANGE(0xffe00000, 0xffffffff) AM_ROM AM_REGION("maincpu", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( bnstars_sound_map, AS_PROGRAM, 8, bnstars_state )
	AM_RANGE(0x0000, 0x3eff) AM_ROM
	AM_RANGE(0x3f00, 0x3f0f) AM_DEVREADWRITE("ymf2", ymf271_device, read, write)
	AM_RANGE(0x3f10, 0x3f10) AM_READWRITE(latch_r,to_main_w)
	AM_RANGE(0x3f20, 0x3f2f) AM_DEVREADWRITE("ymf1", ymf271_device, read, write)
	AM_RANGE(0x3f40, 0x3f40) AM_WRITENOP   /* YMF271 pin 4 (bit 1) , YMF271 pin 39 (bit 4) */
	AM_RANGE(0x3f70, 0x3f70) AM_WRITENOP   // watchdog? banking? very noisy
	AM_RANGE(0x3f80, 0x3f80) AM_WRITE(ms32_snd_bank_w)
	AM_RANGE(0x4000, 0x7fff) AM_RAM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank4")
	AM_RANGE(0xc000, 0xffff) AM_ROMBANK("bank5")
ADDRESS_MAP_END

void bnstars_state::machine_reset()
{
	irq_init();
	membank("bank4")->set_entry(0);
	membank("bank5")->set_entry(1);
}


static MACHINE_CONFIG_START( bnstars, bnstars_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", V70, 20000000) // 20MHz
	MCFG_CPU_PROGRAM_MAP(bnstars_map)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(ms32_state,irq_callback)

	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", bnstars_state, ms32_interrupt, "lscreen", 0, 1)

	MCFG_CPU_ADD("audiocpu", Z80, 4000000)
	MCFG_CPU_PROGRAM_MAP(bnstars_sound_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(60000))

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", bnstars)

	MCFG_PALETTE_ADD("palette", 0x8000)
	MCFG_PALETTE_FORMAT(XBRG)
	MCFG_PALETTE_MEMBITS(16)

	MCFG_PALETTE_ADD("palette2", 0x8000)
	MCFG_PALETTE_FORMAT(XBRG)
	MCFG_PALETTE_MEMBITS(16)

	MCFG_DEFAULT_LAYOUT(layout_dualhsxs)

	MCFG_SCREEN_ADD("lscreen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(40*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 0*8, 28*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(bnstars_state, screen_update_bnstars_left)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_SCREEN_ADD("rscreen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(40*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 0*8, 28*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(bnstars_state, screen_update_bnstars_right)
	MCFG_SCREEN_PALETTE("palette2")


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("ymf1", YMF271, 16934400)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)

	MCFG_SOUND_ADD("ymf2", YMF271, 16934400)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)

MACHINE_CONFIG_END


ROM_START( bnstars1 )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* V70 code */
	ROM_LOAD32_BYTE( "mb-93142.36", 0x000003, 0x80000, CRC(2eb6a503) SHA1(27c02ab1b4321924fd4499844467ea4dc97de25d) )
	ROM_LOAD32_BYTE( "mb-93142.37", 0x000002, 0x80000, CRC(49f60882) SHA1(2ff5b0989aaf970103304a453773e0b9517ebb8d) )
	ROM_LOAD32_BYTE( "mb-93142.38", 0x000001, 0x80000, CRC(6e1312cd) SHA1(4c22f8f9f1574eefd96147453cf240f50c17f5dc) )
	ROM_LOAD32_BYTE( "mb-93142.39", 0x000000, 0x80000, CRC(56b98539) SHA1(5eb0e77729b31e6a100c1b43813a39fea57bedee) )

	/* Sprites - shared by both screens? */
	ROM_REGION( 0x1000000, "gfx1", 0 ) /* sprites */
	ROM_LOAD32_WORD( "mr96004-01.20",   0x0000000, 0x200000, CRC(3366d104) SHA1(2de0cabe2ead777b5b02cade7f2003ef7f90b75b) )
	ROM_LOAD32_WORD( "mr96004-02.28",   0x0000002, 0x200000, CRC(ad556664) SHA1(4b36f8d8d9efa37cf515af41d14433e7eafa27a2) )
	ROM_LOAD32_WORD( "mr96004-03.21",   0x0400000, 0x200000, CRC(b399e2b1) SHA1(9b6a00a219db8d66dcf592160b7b5f7a86b8f0c9) )
	ROM_LOAD32_WORD( "mr96004-04.29",   0x0400002, 0x200000, CRC(f4f4cf4a) SHA1(fe497989cf96c68602f68f14920aed44fd934573) )
	ROM_LOAD32_WORD( "mr96004-05.22",   0x0800000, 0x200000, CRC(cd6c357e) SHA1(44cd2d0607c7ccd80f701cf1675fd283acb07252) )
	ROM_LOAD32_WORD( "mr96004-06.30",   0x0800002, 0x200000, CRC(fc6daad7) SHA1(99f14ac6b06ad9a8a3d2e9f69b693c7ce420a47d) )
	ROM_LOAD32_WORD( "mr96004-07.23",   0x0c00000, 0x200000, CRC(177e32fa) SHA1(3ca1f397dc28f1fa3a4136705b92c63e4e438f05) )
	ROM_LOAD32_WORD( "mr96004-08.31",   0x0c00002, 0x200000, CRC(f6df27b2) SHA1(60590976020d86bdccd4eaf57b349ea31bec6830) )

	/* Roz Tiles #1 (Screen 1) */
	ROM_REGION( 0x400000, "gfx2", 0 ) /* roz tiles */
	ROM_LOAD( "mr96004-09.1", 0x000000, 0x400000, CRC(7f8ea9f0) SHA1(f1fe682dcb884f1aa4a5536e17ab94157a99f519) )

	/* Roz Tiles #2 (Screen 2) */
	ROM_REGION( 0x400000, "gfx3", 0 ) /* roz tiles */
	ROM_LOAD( "mr96004-09.7", 0x000000, 0x400000, CRC(7f8ea9f0) SHA1(f1fe682dcb884f1aa4a5536e17ab94157a99f519) )

	/* BG Tiles #1 (Screen 1?) */
	ROM_REGION( 0x200000, "gfx4", 0 ) /* bg tiles */
	ROM_LOAD( "mr96004-11.11", 0x000000, 0x200000,  CRC(e6da552c) SHA1(69a5af3015883793c7d1343243ccae23db9ef77c) )

	/* TX Tiles #1 (Screen 1?) */
	ROM_REGION( 0x080000, "gfx5", 0 ) /* tx tiles */
	ROM_LOAD( "vsjanshi6.5", 0x000000, 0x080000, CRC(fdbbac21) SHA1(c77d852e53126cc8ebfe1e79d1134e42b54d1aab) )

	/* BG Tiles #2 (Screen 2?) */
	ROM_REGION( 0x200000, "gfx6", 0 ) /* bg tiles */
	ROM_LOAD( "mr96004-11.13", 0x000000, 0x200000, CRC(e6da552c) SHA1(69a5af3015883793c7d1343243ccae23db9ef77c) )

	/* TX Tiles #2 (Screen 2?) */
	ROM_REGION( 0x080000, "gfx7", 0 ) /* tx tiles */
	ROM_LOAD( "vsjanshi5.6", 0x000000, 0x080000, CRC(fdbbac21) SHA1(c77d852e53126cc8ebfe1e79d1134e42b54d1aab) )

	/* Sound Program (one, driving both screen sound) */
	ROM_REGION( 0x40000, "audiocpu", 0 ) /* z80 program */
	ROM_LOAD( "sb93145.5",  0x000000, 0x040000, CRC(0424e899) SHA1(fbcdebfa3d5f52b10cf30f7e416f5f53994e4d55) )

	/* Samples #1 (Screen 1?) */
	ROM_REGION( 0x400000, "ymf1", 0 ) /* samples - 8-bit signed PCM */
	ROM_LOAD( "mr96004-10.1",  0x000000, 0x400000, CRC(83f4303a) SHA1(90ee010591afe1d35744925ef0e8d9a7e2ef3378) )

	/* Samples #2 (Screen 2?) */
	ROM_REGION( 0x400000, "ymf2", 0 ) /* samples - 8-bit signed PCM */
	ROM_LOAD( "mr96004-10.1",  0x000000, 0x400000, CRC(83f4303a) SHA1(90ee010591afe1d35744925ef0e8d9a7e2ef3378) )
ROM_END


/* SS92046_01: bbbxing, f1superb, tetrisp, hayaosi1 */
DRIVER_INIT_MEMBER(bnstars_state,bnstars)
{
	ms32_rearrange_sprites(machine(), "gfx1");

	decrypt_ms32_tx(machine(), 0x00020,0x7e, "gfx5");
	decrypt_ms32_bg(machine(), 0x00001,0x9b, "gfx4");
	decrypt_ms32_tx(machine(), 0x00020,0x7e, "gfx7");
	decrypt_ms32_bg(machine(), 0x00001,0x9b, "gfx6");

	configure_banks();
}

GAME( 1997, bnstars1, 0,        bnstars, bnstars, bnstars_state, bnstars, ROT0,   "Jaleco", "Vs. Janshi Brandnew Stars", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
