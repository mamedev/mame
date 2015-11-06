// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/********************************************************************************

     deco16ic.c

    Implementation of Data East tilemap ICs
    Data East IC 55 / 56 / 74 / 141

    original work by Bryan McPhail, various updates David Haywood

    Data East video emulation & information by Bryan McPhail, mish@tendril.co.uk (c) 2000-2005 Bryan McPhail
    Please send me any additions to the table below.


    Game                    Mask Rom    CPU     Sprites         Playfields  IO/Protection   Other
    ==============================================================================================
    Vapor Trail/Kuhga           MAA     59      MXC-06          55, 55
    Crude Buster/Two Crude      MAB     59      52              55, 55
    Dark Seal/Gate Of Doom      MAC     59      52              55, 55
    Edward Randy                MAD     59      52              55, 55          60
    Super Burger Time           MAE     59      52              55
    Mutant Fighter/Death Brade  MAF     59      52              55, 56          66
    Caveman Ninja/Joe & Mac     MAG     59      52              55, 55          104
    Robocop 2                   MAH     59      52              55, 55          75
    Desert Assault/Thunderzone  MAJ     59,59   52,52           55, 55
    China Town                  MAK     59      52              55
    Rogha/Wolf Fang             MAM     59      52,52,71,71     55, 56          104         113
    Captain America             MAN     101     52,71           56, 56          75
    Tumblepop                   MAP     59      52              56
    Dragon Gun                  MAR     101     ?               74, 74          146         113,186,187
    Wizard Fire/Dark Seal 2     MAS     59      52,52,71,71     74, 74          104         113
    Funky Jet                   MAT     59      52              74              146
    Nitro Ball                  MAV     59      52,52,71,71     56, 74          146         113
    Diet GoGo                   MAY     102     52,71           141             104         113
    Pocket Gal DX               MAZ     102     52,71           56              104         153
    Boogie Wings                MBD     102     52,52,71,71     141, 141        104         113,99,200
1   Double Wings                MBE     102     52              141             104
3   Fighter's History DE-0380-0 MBF     156     52              56, 74          75          153,153,200,170
3   Fighter's History DE-0395-1 MBF     156     52              56, 74          75          113,113,200
3   Fighter's History DE-0396-0 MBF     156     52              74, 141         75          153,153,200
    Heavy Smash                 MBG     156     52              141                         153,153,153
    Night Slashers DE-0397-0    MBH     156     52,52,52        74, 141         104         153,153,153,99,200
    Night Slashers DE-0395-1    MBH     156     52,52,52        74, 141         104         113,113,153,99,200
    Locked N Loaded             MBM     101     ?               74, 74          146         113,186,187
    Joe & Mac Return            MBN     156     52              141                         223,223
2   Charlie Ninja               MBR     156     52              141                         223,223
    World Cup Volleyball 95     MBX     156     52              141             ?
    Backfire!                   MBZ     156     52,52           141, 141        ?           153,153,223
2*  Ganbare! Gonta!! 2          MCB     156     52              141                         223,223
    Chain Reaction/Magical Drop MCC     156     52              141                         223,223
    Dunk Dream 95               MCE     156     [MLC]           [MLC]
2   Osman/Cannon Dancer         MCF     156     52              141                         223,223
    Avengers In Galactic Storm  MCG     SH2     [MLC]           [MLC]
    Skull Fang                  MCH     156     [MLC]           [MLC]
    Stadium Hero 96             MCM     156     [MLC]           [MLC]           146

    Sotsugyo Shousho                    59      52              74              146?
    Lemmings                    ---     59      52,52,71,71     None            75
    Tattoo Assassins            ---     101     52,52,71,71     141, 141?       104? (same as Night Slashers)           99, ?

Note 1: Mitchell game on DECO PCB board number DEC-22V0 (S-NK-3220)
Note 2: Mitchell games on DECO PCB board number MT5601-0
Note 3: Fighter's History uses the 156 Encrypted ARM cpu with the encryption disabled
Note *: Ganbare! Gonta!! 2 / Lady Killer Part 2 - Party Time

Note: A version of Night Slashers runs on the DE-0395-1 using the 156 encryption, while a version
      of Fighter's History also runs on the DE-0395-1 PCB without using the 156 encryption

    Custom chip 59  = 68000 cpu
    Custom chip 101 = Arm6 cpu
    Custom chip 113 = Alpha blending
    Custom chip 153 = Alpha blending (same functions as 113, smaller PQFP package)
    Custom chip 99  = 'Ace' chip (Special alpha blending?)
    Custom chip 156 = Encrypted ARM cpu
    Custom chip 102 = Encrypted 68000 cpu

    Custom chip 55 provides two playfields of 4bpp tiles, with optional
    rowscroll and column scroll.  Some games use two of these to give
    4 playfields.  Palette banking, tile banking and priority are outside
    the scope of this chip, and usually differ between games.  Some games
    combine the 4bpp output of each playfield to give an effective 8bpp
    display.

    Custom chip 56 is the same as 55 but with on-chip decryption, so
    encrypted roms can be used.

    Custom chip 74 is the same as 56 but with different decryption tables.

    Custom chip 141 is same as 56, but can output up to 8BPP per layer.

    Custom chip 55/56/74/141 control register layout:

    Word 0:
        Mask 0x0080: Flip screen
        Mask 0x007f: ?  Possibly a bit to set transparent pen 0 on/off
    Word 2:
        Mask 0xffff: Playfield 2 X scroll
    Word 4:
        Mask 0xffff: Playfield 2 Y scroll
    Word 6:
        Mask 0xffff: Playfield 1 X scroll
    Word 8:
        Mask 0xffff: Playfield 1 Y scroll
    Word 0xa:
        Mask 0x8000: Playfield 1 enable
        Mask 0x7800: Playfield 1 rowscroll style
        Mask 0x0700: Playfield 1 colscroll style

        Mask 0x0080: Playfield 2 enable
        Mask 0x0078: Playfield 2 rowscroll style
        Mask 0x0007: Playfield 2 colscroll style
    Word 0xc:
        Mask 0x8000: Playfield 1 is 8*8 tiles else 16*16
        Mask 0x4000: Playfield 1 rowscroll enabled
        Mask 0x2000: Playfield 1 colscroll enabled
        Mask 0x1800: Seemingly unused
        Mask 0x0400: Use alternate palette bank (unimplemented/unused by any game)
        Mask 0x0200: Y flip tiles with top bit of palette code set (palette becomes 3 bits for this tile)
        Mask 0x0100: X flip tiles with top bit of palette code set (palette becomes 3 bits for this tile)

        Mask 0x0080: Playfield 2 is 8*8 tiles else 16*16
        Mask 0x0040: Playfield 2 rowscroll enabled
        Mask 0x0020: Playfield 2 colscroll enabled
        Mask 0x0018: Seemingly unused
        Mask 0x0004: Use alternate palette bank (unimplemented/unused by any game)
        Mask 0x0002: Y flip tiles with top bit of palette code set (palette becomes 3 bits for this tile)
        Mask 0x0001: X flip tiles with top bit of palette code set (palette becomes 3 bits for this tile)

    Word 0xe:
        Mask 0xff00: Playfield 1 gfx rom banking (Usually different hookup per game)
        Mask 0x00ff: Playfield 2 gfx rom banking

Colscroll style:
    0   8 pixel columns across bitmap
    1   16 pixel columns across bitmap
    2   32
    3   64
    4   128
    5   256
    6   512
    7   1024  (Effectively 0 as 1024 is the width of the bitmap)

Rowscroll style:
    0   512 rows down bitmap in 16x16 tile mode, 256 rows in 8x8 mode
    1   256 rows in 16x16, 128 in 8x8
    2   128 / 64
    3   64 / 32
    4   32 / 16
    5   16 / 8
    6   8 / 4
    7   4 / 2
    8   2 / 1
    9-15 Untested on hardware, no known games set this anyway.  Assumed invalid (no rowscroll)

    Column and rowscroll can both be applied at once.


    2010-02: Converted to be a device.
    TODO:
        - convert to c++ device
        - should the decryption functions for the tilemap chips be here too?

    it seems overall height / width of the tilemaps can be configured somehow
    darkseal clearly needs 64 rows, whereas other games need 32.

    width seems configurable in a similar way, with nitroball and lockload
    needing narrower tilemaps.  lockload/dragngun might be a good study case
    as they run on the same harwdare, and both rely on different behavior.

***************************************************************************/

#include "emu.h"
#include "video/deco16ic.h"
#include "ui/ui.h"
#include "render.h"

const device_type DECO16IC = &device_creator<deco16ic_device>;

deco16ic_device::deco16ic_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, DECO16IC, "DECO 55 / 56 / 74 / 141 IC", tag, owner, clock, "deco16ic", __FILE__),
		device_video_interface(mconfig, *this),
		m_pf1_data(NULL),
		m_pf2_data(NULL),
		m_pf12_control(NULL),
		m_pf1_rowscroll_ptr(NULL),
		m_pf2_rowscroll_ptr(NULL),
		m_use_custom_pf1(0),
		m_use_custom_pf2(0),
		m_pf1_bank(0),
		m_pf2_bank(0),
		m_pf12_last_small(0),
		m_pf12_last_big(0),
		m_pf1_8bpp_mode(0),
		m_split(0),
		m_full_width12(1),
		m_pf1_trans_mask(0xf),
		m_pf2_trans_mask(0xf),
		m_pf1_colour_bank(0),
		m_pf2_colour_bank(0),
		m_pf1_colourmask(0xf),
		m_pf2_colourmask(0xf),
		m_pf12_8x8_gfx_bank(0),
		m_pf12_16x16_gfx_bank(0),
		m_gfxdecode(*this),
		m_palette(*this)
{
}

//-------------------------------------------------
//  static_set_gfxdecode_tag: Set the tag of the
//  gfx decoder
//-------------------------------------------------

void deco16ic_device::static_set_gfxdecode_tag(device_t &device, const char *tag)
{
	downcast<deco16ic_device &>(device).m_gfxdecode.set_tag(tag);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void deco16ic_device::device_start()
{
	if(!m_gfxdecode->started())
		throw device_missing_dependencies();

	m_bank1_cb.bind_relative_to(*owner());
	m_bank2_cb.bind_relative_to(*owner());

	int fullheight = 0;
	int fullwidth = 0;

	if (m_full_width12&2)
		fullheight = 1;

	if (m_full_width12&1)
		fullwidth = 1;

	m_pf1_tilemap_16x16 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(deco16ic_device::get_pf1_tile_info),this), tilemap_mapper_delegate(FUNC(deco16ic_device::deco16_scan_rows),this), 16, 16, fullwidth ? 64 : 32, fullheight ?64 : 32);
//  m_pf1_tilemap_8x8 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(deco16ic_device::get_pf1_tile_info_b),this), TILEMAP_SCAN_ROWS, 8, 8, m_full_width12 ? 64 : 32, 32);
	m_pf1_tilemap_8x8 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(deco16ic_device::get_pf1_tile_info_b),this), TILEMAP_SCAN_ROWS, 8, 8, 64 , 32); // nitroball

	if (m_split)
		m_pf2_tilemap_16x16 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(deco16ic_device::get_pf2_tile_info),this), tilemap_mapper_delegate(FUNC(deco16ic_device::deco16_scan_rows),this), 16, 16, fullwidth ? 64 : 32, fullheight ? 64 : 32);
	else
		m_pf2_tilemap_16x16 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(deco16ic_device::get_pf2_tile_info),this), tilemap_mapper_delegate(FUNC(deco16ic_device::deco16_scan_rows),this), 16, 16, fullwidth ? 64 : 32, fullheight ? 64 : 32);

	m_pf2_tilemap_8x8 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(deco16ic_device::get_pf2_tile_info_b),this), TILEMAP_SCAN_ROWS, 8, 8, fullwidth ? 64 : 32, fullheight ? 64 : 32);

	m_pf1_tilemap_8x8->set_transparent_pen(0);
	m_pf2_tilemap_8x8->set_transparent_pen(0);
	m_pf1_tilemap_16x16->set_transparent_pen(0);
	m_pf2_tilemap_16x16->set_transparent_pen(0);

	if (m_split) /* Caveman Ninja only */
		m_pf2_tilemap_16x16->set_transmask(0, 0x00ff, 0xff01);

	m_pf1_8bpp_mode = 0;

	m_pf1_data = auto_alloc_array_clear(machine(), UINT16, 0x2000 / 2);
	m_pf2_data = auto_alloc_array_clear(machine(), UINT16, 0x2000 / 2);
	m_pf12_control = auto_alloc_array_clear(machine(), UINT16, 0x10 / 2);


	save_item(NAME(m_use_custom_pf1));
	save_item(NAME(m_use_custom_pf2));
	save_item(NAME(m_pf1_bank));
	save_item(NAME(m_pf2_bank));
	save_item(NAME(m_pf12_8x8_gfx_bank));
	save_item(NAME(m_pf12_16x16_gfx_bank));
	save_item(NAME(m_pf12_last_small));
	save_item(NAME(m_pf12_last_big));

	save_item(NAME(m_pf1_8bpp_mode));

	save_pointer(NAME(m_pf1_data), 0x2000 / 2);
	save_pointer(NAME(m_pf2_data), 0x2000 / 2);
	save_pointer(NAME(m_pf12_control), 0x10 / 2);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void deco16ic_device::device_reset()
{
	m_pf1_bank = m_pf2_bank = 0;
	m_pf12_last_small = m_pf12_last_big = -1;
	m_use_custom_pf1 = m_use_custom_pf2 = 0;
	m_pf1_rowscroll_ptr = 0;
	m_pf2_rowscroll_ptr = 0;
}

/*****************************************************************************************/

TILEMAP_MAPPER_MEMBER(deco16ic_device::deco16_scan_rows)
{
	/* logical (col,row) -> memory offset */
	return (col & 0x1f) + ((row & 0x1f) << 5) + ((col & 0x20) << 5) + ((row & 0x20) << 6);
}

TILE_GET_INFO_MEMBER(deco16ic_device::get_pf2_tile_info)
{
	UINT16 tile = m_pf2_data[tile_index];
	UINT8 colour = (tile >> 12) & 0xf;
	UINT8 flags = 0;

	if (tile & 0x8000)
	{
		if ((m_pf12_control[6] >> 8) & 0x01)
		{
			flags |= TILE_FLIPX;
			colour &= 0x7;
		}
		if ((m_pf12_control[6] >> 8) & 0x02)
		{
			flags |= TILE_FLIPY;
			colour &= 0x7;
		}
	}

	SET_TILE_INFO_MEMBER(m_pf12_16x16_gfx_bank,
			(tile & 0xfff) | m_pf2_bank,
			(colour & m_pf2_colourmask) + m_pf2_colour_bank,
			flags);
}

TILE_GET_INFO_MEMBER(deco16ic_device::get_pf1_tile_info)
{
	UINT16 tile = m_pf1_data[tile_index];
	UINT8 colour = (tile >> 12) & 0xf;
	UINT8 flags = 0;

	if (tile & 0x8000)
	{
		if ((m_pf12_control[6] >> 0) & 0x01)
		{
			flags |= TILE_FLIPX;
			colour &= 0x7;
		}
		if ((m_pf12_control[6] >> 0) & 0x02)
		{
			flags |= TILE_FLIPY;
			colour &= 0x7;
		}
	}

	if (m_pf1_8bpp_mode)
	{
		// Captain America operates this chip in 8bpp mode.
		// In 8bpp mode you appear to only get 1 layer, not 2, but you also
		// have an extra 2 tile bits, and 2 less colour bits.
		SET_TILE_INFO_MEMBER(m_pf12_16x16_gfx_bank,
				(tile & 0x3fff) | m_pf1_bank,
				((colour & m_pf1_colourmask) + m_pf1_colour_bank)>>2,
				flags);
	}
	else
	{
		SET_TILE_INFO_MEMBER(m_pf12_16x16_gfx_bank,
				(tile & 0xfff) | m_pf1_bank,
				(colour & m_pf1_colourmask) + m_pf1_colour_bank,
				flags);
	}
}

TILE_GET_INFO_MEMBER(deco16ic_device::get_pf2_tile_info_b)
{
	UINT16 tile = m_pf2_data[tile_index];
	UINT8 colour = (tile >> 12) & 0xf;
	UINT8 flags = 0;

	if (tile & 0x8000)
	{
		if ((m_pf12_control[6] >> 8) & 0x01)
		{
			flags |= TILE_FLIPX;
			colour &= 0x7;
		}
		if ((m_pf12_control[6] >> 8) & 0x02)
		{
			flags |= TILE_FLIPY;
			colour &= 0x7;
		}
	}

	SET_TILE_INFO_MEMBER(m_pf12_8x8_gfx_bank,
			(tile & 0xfff) | m_pf2_bank,
			(colour & m_pf2_colourmask) + m_pf2_colour_bank,
			flags);
}

TILE_GET_INFO_MEMBER(deco16ic_device::get_pf1_tile_info_b)
{
	UINT16 tile = m_pf1_data[tile_index];
	UINT8 colour = (tile >> 12) & 0xf;
	UINT8 flags = 0;

	if (tile & 0x8000)
	{
		if ((m_pf12_control[6] >> 0) & 0x01)
		{
			flags |= TILE_FLIPX;
			colour &= 0x7;
		}
		if ((m_pf12_control[6] >> 0) & 0x02)
		{
			flags |= TILE_FLIPY;
			colour &= 0x7;
		}
	}

	SET_TILE_INFO_MEMBER(m_pf12_8x8_gfx_bank,
			(tile & 0xfff) | m_pf1_bank,
			(colour & m_pf1_colourmask) + m_pf1_colour_bank,
			flags);
}


/*****************************************************************************************/

/*
    Consider this the 'reference rasterizer' for the 56/74/141 tilemap chips - it implements
    simultaneous row & column scroll which the Mame tilemap core cannot do.  It also
    implements combining the 4BPP output of two tilemaps into 8BPP output.  This function
    is automatically called when the tilemap is in a state that cannot be properly rendered
    by the Mame core.
*/

template<class _BitmapClass>
void deco16ic_device::custom_tilemap_draw(
	screen_device &screen,
	_BitmapClass &bitmap,
	const rectangle &cliprect,
	tilemap_t *tilemap0_8x8,
	tilemap_t *tilemap0_16x16,
	tilemap_t *tilemap1_8x8,
	tilemap_t *tilemap1_16x16,
	const UINT16 *rowscroll_ptr,
	const UINT16 scrollx,
	const UINT16 scrolly,
	const UINT16 control0,
	const UINT16 control1,
	int combine_mask,
	int combine_shift,
	int trans_mask,
	int flags,
	UINT32 priority,
	int is_tattoo
	)
{
	tilemap_t *tilemap0 = BIT(control1, 7) ? tilemap0_8x8 : tilemap0_16x16;
	tilemap_t *tilemap1 = BIT(control1, 7) ? tilemap1_8x8 : tilemap1_16x16;
	const bitmap_ind16 *src_bitmap0 = tilemap0 ? &tilemap0->pixmap() : NULL;
	const bitmap_ind16 *src_bitmap1 = tilemap1 ? &tilemap1->pixmap() : NULL;
	int width_mask, height_mask, x, y, p;
	int column_offset, src_x = 0, src_y = 0;
	int row_type = 1 << ((control0 >> 3) & 0xf);
	int col_type = 8 << (control0 & 7);

	if (!src_bitmap0)
		return;

	// Playfield disable
	if (!BIT(control0, 7))
		return;

	int starty = cliprect.min_y;
	int endy = cliprect.max_y+1;

	width_mask = src_bitmap0->width() - 1;
	height_mask = src_bitmap0->height() - 1;
	src_y = (scrolly + starty) & height_mask;


	for (y = starty; y < endy; y++)
	{
		if (rowscroll_ptr && BIT(control1, 6))
			src_x = scrollx + rowscroll_ptr[src_y / row_type];
		else
			src_x = scrollx;

		src_x &= width_mask;

		/* boogwing */
		for (x = 0; x < 320; x++)
		{
			if (rowscroll_ptr && BIT(control1, 5))
				column_offset = rowscroll_ptr[0x200 + ((src_x & 0x1ff) / col_type)];
			else
				column_offset = 0;

			p = src_bitmap0->pix16((src_y + column_offset) & height_mask, src_x);

			if (src_bitmap1)
			{
				if (!is_tattoo)
				{
					// does boogie wings actually use this, or is the tattoo assassing code correct in this mode?
					p |= (src_bitmap1->pix16((src_y + column_offset) & height_mask, src_x) & combine_mask) << combine_shift;
				}
				else
				{
					UINT16 p2 = src_bitmap1->pix16((src_y + column_offset) & height_mask, src_x);
					p = 0x200+( ((p&0x30)<<4) | (p&0x0f) | ((p2 & 0x0f)<<4));
				}
			}
			src_x = (src_x + 1) & width_mask;

			if ((flags & TILEMAP_DRAW_OPAQUE) || (p & trans_mask))
			{
				bitmap.pix(y, x) = m_palette->pen(p);
				if (screen.priority().valid())
				{
					UINT8 *pri = &screen.priority().pix8(y);
					pri[x] |= priority;
				}
			}
		}

		src_y = (src_y + 1) & height_mask;
	}
}

/******************************************************************************/

/* captain america seems to have a similar 8bpp feature to robocop2, investigate merging */
void deco16ic_device::set_pf1_8bpp_mode(int mode)
{
	m_pf1_8bpp_mode = mode;
}

/* robocop 2 can switch between 2 tilemaps at 4bpp, or 1 at 8bpp */
void deco16ic_device::set_tilemap_colour_mask( int tmap, int mask )
{
	switch (tmap)
	{
	case 0: m_pf1_colourmask = mask; break;
	case 1: m_pf2_colourmask = mask; break;
	}
}

void deco16ic_device::pf12_set_gfxbank( int small, int big )
{
	if (m_pf12_last_big != big)
	{
		if (m_pf1_tilemap_16x16)
			m_pf1_tilemap_16x16->mark_all_dirty();
		if (m_pf2_tilemap_16x16)
			m_pf2_tilemap_16x16->mark_all_dirty();

		m_pf12_last_big = big;
	}
	m_pf12_16x16_gfx_bank = big;

	if (m_pf12_last_small != small)
	{
		if (m_pf1_tilemap_8x8)
			m_pf1_tilemap_8x8->mark_all_dirty();
		if (m_pf2_tilemap_8x8)
			m_pf2_tilemap_8x8->mark_all_dirty();

		m_pf12_last_small = small;
	}
	m_pf12_8x8_gfx_bank = small;
}

/* stoneage has broken scroll registers */
void deco16ic_device::set_scrolldx( int tmap, int size, int dx, int dx_if_flipped )
{
	switch (tmap)
	{
	case 0:
		if (!size)
			m_pf1_tilemap_16x16->set_scrolldx(dx, dx_if_flipped);
		else
			m_pf1_tilemap_8x8->set_scrolldx(dx, dx_if_flipped);
		break;
	case 1:
		if (!size)
			m_pf2_tilemap_16x16->set_scrolldx(dx, dx_if_flipped);
		else
			m_pf2_tilemap_8x8->set_scrolldx(dx, dx_if_flipped);
		break;
	}
}

/* cninjabl does not enable background layers */
void deco16ic_device::set_enable( int tmap, int enable )
{
	int shift = (tmap & 1) ? 15 : 7;
	m_pf12_control[5] &= ~(1 << shift);
	m_pf12_control[5] |= (enable & 1) << shift;
}


/******************************************************************************/

WRITE16_MEMBER( deco16ic_device::pf1_data_w )
{
	COMBINE_DATA(&m_pf1_data[offset]);

	m_pf1_tilemap_8x8->mark_tile_dirty(offset);
//  if (offset < 0x800)
		m_pf1_tilemap_16x16->mark_tile_dirty(offset);
}

WRITE16_MEMBER( deco16ic_device::pf2_data_w )
{
	COMBINE_DATA(&m_pf2_data[offset]);

	m_pf2_tilemap_8x8->mark_tile_dirty(offset);
//  if (offset < 0x800)
		m_pf2_tilemap_16x16->mark_tile_dirty(offset);
}

READ16_MEMBER( deco16ic_device::pf1_data_r )
{
	return m_pf1_data[offset];
}

READ16_MEMBER( deco16ic_device::pf2_data_r )
{
	return m_pf2_data[offset];
}

WRITE16_MEMBER( deco16ic_device::pf_control_w )
{
	m_screen->update_partial(m_screen->vpos());

	COMBINE_DATA(&m_pf12_control[offset]);
}

READ16_MEMBER( deco16ic_device::pf_control_r )
{
	return m_pf12_control[offset];
}


READ32_MEMBER( deco16ic_device::pf_control_dword_r )
{
	return pf_control_r(space, offset, 0xffff)^0xffff0000;
}

WRITE32_MEMBER( deco16ic_device::pf_control_dword_w )
{
	pf_control_w(space, offset, data & 0xffff, mem_mask & 0xffff);
}

READ32_MEMBER( deco16ic_device::pf1_data_dword_r )
{
	return pf1_data_r(space, offset, 0xffff)^0xffff0000;
}

WRITE32_MEMBER( deco16ic_device::pf1_data_dword_w )
{
	pf1_data_w(space, offset, data & 0xffff, mem_mask & 0xffff);
}

READ32_MEMBER( deco16ic_device::pf2_data_dword_r )
{
	return pf2_data_r(space, offset, 0xffff)^0xffff0000;
}

WRITE32_MEMBER( deco16ic_device::pf2_data_dword_w )
{
	pf2_data_w(space, offset, data & 0xffff, mem_mask & 0xffff);
}



/*****************************************************************************************/

static int deco16_pf_update(
	tilemap_t *tilemap_8x8,
	tilemap_t *tilemap_16x16,
	const UINT16 *rowscroll_ptr,
	const UINT16 scrollx,
	const UINT16 scrolly,
	const UINT16 control0,
	const UINT16 control1)
{
	int rows, cols, offs, use_custom = 0;

	/* Toggle between 8x8 and 16x16 modes (and master enable bit) */
	if (BIT(control1, 7))
	{
		if (tilemap_8x8)
			tilemap_8x8->enable(BIT(control0, 7));

		if (tilemap_16x16)
			tilemap_16x16->enable(0);

	}
	else
	{
		if (tilemap_8x8)
			tilemap_8x8->enable(0);

		if (tilemap_16x16)
			tilemap_16x16->enable(BIT(control0, 7));
	}

	/* Rowscroll enable */
	if (rowscroll_ptr && (control1 & 0x60) == 0x40)
	{
		/* Several different rowscroll styles */
		switch ((control0 >> 3) & 0xf)
		{
			case 0:     rows = 512;     break;/* Every line of 512 height bitmap */
			case 1:     rows = 256;     break;
			case 2:     rows = 128;     break;
			case 3:     rows = 64;      break;
			case 4:     rows = 32;      break;
			case 5:     rows = 16;      break;
			case 6:     rows = 8;       break;
			case 7:     rows = 4;       break;
			case 8:     rows = 2;       break;
			default:    rows = 1;       break;
		}

		if (tilemap_16x16)
		{
			tilemap_16x16->set_scroll_cols(1);
			tilemap_16x16->set_scroll_rows(rows);
			tilemap_16x16->set_scrolly(0, scrolly);

			for (offs = 0; offs < rows; offs++)
				tilemap_16x16->set_scrollx(offs, scrollx + rowscroll_ptr[offs]);
		}

		if (tilemap_8x8)
		{
			tilemap_8x8->set_scroll_cols(1);
			tilemap_8x8->set_scroll_rows(rows / 2);
			tilemap_8x8->set_scrolly(0, scrolly);

			for (offs = 0; offs < rows / 2; offs++)
				tilemap_8x8->set_scrollx(offs, scrollx + rowscroll_ptr[offs]);
		}
	}
	else if (rowscroll_ptr && (control1 & 0x60) == 0x20)  /* Column scroll */
	{
		/* Column scroll ranges from 8 pixel columns to 512 pixel columns */
		int mask = (0x40 >> (control0 & 7)) - 1;
		if (mask == -1)
			mask = 0;

		cols = (8 << (control0 & 7)) & 0x3ff;
		if (!cols)
			cols = 1024;

		cols = 1024 / cols;

		if (tilemap_16x16)
		{
			tilemap_16x16->set_scroll_cols(cols);
			tilemap_16x16->set_scroll_rows(1);
			tilemap_16x16->set_scrollx(0, scrollx);

			for (offs = 0; offs < cols; offs++)
				tilemap_16x16->set_scrolly(offs, scrolly + rowscroll_ptr[(offs & mask) + 0x200]);
		}

		if (tilemap_8x8)
		{
			cols = cols / 2; /* Adjust because 8x8 tilemap is half the width of 16x16 */

			tilemap_8x8->set_scroll_cols(cols);
			tilemap_8x8->set_scroll_rows(1);
			tilemap_8x8->set_scrollx(0, scrollx);

			for (offs = 0; offs < cols; offs++)
				tilemap_8x8->set_scrolly(offs, scrolly + rowscroll_ptr[(offs & mask) + 0x200]);
		}
	}
	else if (control1 & 0x60)
	{
		/* Simultaneous row & column scroll requested - use custom renderer */
		use_custom = 1;

		if (tilemap_16x16)
		{
			tilemap_16x16->set_scroll_rows(1);
			tilemap_16x16->set_scroll_cols(1);
			tilemap_16x16->set_scrollx(0, scrollx);
			tilemap_16x16->set_scrolly(0, scrolly);
		}

		if (tilemap_8x8)
		{
			tilemap_8x8->set_scroll_rows(1);
			tilemap_8x8->set_scroll_cols(1);
			tilemap_8x8->set_scrollx(0, scrollx);
			tilemap_8x8->set_scrolly(0, scrolly);

		}

	}
	else
	{
		if (tilemap_16x16)
		{
			tilemap_16x16->set_scroll_rows(1);
			tilemap_16x16->set_scroll_cols(1);
			tilemap_16x16->set_scrollx(0, scrollx);
			tilemap_16x16->set_scrolly(0, scrolly);
		}

		if (tilemap_8x8)
		{
			tilemap_8x8->set_scroll_rows(1);
			tilemap_8x8->set_scroll_cols(1);
			tilemap_8x8->set_scrollx(0, scrollx);
			tilemap_8x8->set_scrolly(0, scrolly);
		}
	}

	return use_custom;
}

void deco16ic_device::pf_update( const UINT16 *rowscroll_1_ptr, const UINT16 *rowscroll_2_ptr )
{
	int bank1, bank2;

	/* Update scrolling and tilemap enable */
	m_pf1_rowscroll_ptr = rowscroll_1_ptr;
	m_pf2_rowscroll_ptr = rowscroll_2_ptr;
	m_use_custom_pf2 = deco16_pf_update(m_pf2_tilemap_8x8, m_pf2_tilemap_16x16, rowscroll_2_ptr, m_pf12_control[3], m_pf12_control[4], m_pf12_control[5] >> 8, m_pf12_control[6] >> 8);
	m_use_custom_pf1 = deco16_pf_update(m_pf1_tilemap_8x8, m_pf1_tilemap_16x16, rowscroll_1_ptr, m_pf12_control[1], m_pf12_control[2], m_pf12_control[5] & 0xff, m_pf12_control[6] & 0xff);

	/* Update banking and global flip state */
	if (!m_bank1_cb.isnull())
	{
		bank1 = m_bank1_cb(m_pf12_control[7] & 0xff);

		if (bank1 != m_pf1_bank)
		{
			if (m_pf1_tilemap_8x8)
				m_pf1_tilemap_8x8->mark_all_dirty();
			if (m_pf1_tilemap_16x16)
				m_pf1_tilemap_16x16->mark_all_dirty();

			m_pf1_bank = bank1;
		}
	}

	if (!m_bank2_cb.isnull())
	{
		bank2 = m_bank2_cb(m_pf12_control[7] >> 8);

		if (bank2 != m_pf2_bank)
		{
			if (m_pf2_tilemap_8x8)
				m_pf2_tilemap_8x8->mark_all_dirty();
			if (m_pf2_tilemap_16x16)
				m_pf2_tilemap_16x16->mark_all_dirty();

			m_pf2_bank = bank2;
		}
	}
}

/*****************************************************************************************/

void deco16ic_device::print_debug_info(bitmap_ind16 &bitmap)
{
	char buf[64*5];

	if (machine().input().code_pressed(KEYCODE_O))
		return;

	if (m_pf12_control)
	{
		sprintf(buf,"%04X %04X %04X %04X\n", m_pf12_control[0], m_pf12_control[1], m_pf12_control[2], m_pf12_control[3]);
		sprintf(&buf[strlen(buf)],"%04X %04X %04X %04X\n", m_pf12_control[4], m_pf12_control[5], m_pf12_control[6], m_pf12_control[7]);
	}
	else
		sprintf(buf, "\n\n");

	machine().ui().draw_text(&machine().render().ui_container(), buf, 60, 40);
}

/*****************************************************************************************/

template<class _BitmapClass>
void deco16ic_device::tilemap_1_draw_common( screen_device &screen, _BitmapClass &bitmap, const rectangle &cliprect, int flags, UINT32 priority )
{
	if (m_use_custom_pf1)
	{
		custom_tilemap_draw(screen, bitmap, cliprect, m_pf1_tilemap_8x8, m_pf1_tilemap_16x16, 0, 0, m_pf1_rowscroll_ptr, m_pf12_control[1], m_pf12_control[2], m_pf12_control[5] & 0xff, m_pf12_control[6] & 0xff, 0, 0, m_pf1_trans_mask, flags, priority, 0);
	}
	else
	{
		if (m_pf1_tilemap_8x8)
			m_pf1_tilemap_8x8->draw(screen, bitmap, cliprect, flags, priority);
		if (m_pf1_tilemap_16x16)
			m_pf1_tilemap_16x16->draw(screen, bitmap, cliprect, flags, priority);
	}
}

void deco16ic_device::tilemap_1_draw( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int flags, UINT32 priority )
{ tilemap_1_draw_common(screen, bitmap, cliprect, flags, priority); }

void deco16ic_device::tilemap_1_draw( screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int flags, UINT32 priority )
{ tilemap_1_draw_common(screen, bitmap, cliprect, flags, priority); }


template<class _BitmapClass>
void deco16ic_device::tilemap_2_draw_common(screen_device &screen, _BitmapClass &bitmap, const rectangle &cliprect, int flags, UINT32 priority)
{
	if (m_use_custom_pf2)
	{
		custom_tilemap_draw(screen, bitmap, cliprect, m_pf2_tilemap_8x8, m_pf2_tilemap_16x16, 0, 0, m_pf2_rowscroll_ptr, m_pf12_control[3], m_pf12_control[4], m_pf12_control[5] >> 8, m_pf12_control[6] >> 8, 0, 0, m_pf2_trans_mask, flags, priority, 0);
	}
	else
	{
		if (m_pf2_tilemap_8x8)
			m_pf2_tilemap_8x8->draw(screen, bitmap, cliprect, flags, priority);
		if (m_pf2_tilemap_16x16)
			m_pf2_tilemap_16x16->draw(screen, bitmap, cliprect, flags, priority);
	}
}

void deco16ic_device::tilemap_2_draw( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int flags, UINT32 priority )
{ tilemap_2_draw_common(screen, bitmap, cliprect, flags, priority); }

void deco16ic_device::tilemap_2_draw( screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int flags, UINT32 priority )
{ tilemap_2_draw_common(screen, bitmap, cliprect, flags, priority); }


/*****************************************************************************************/

// Combines the output of two 4BPP tilemaps into an 8BPP tilemap
void deco16ic_device::tilemap_12_combine_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int flags, UINT32 priority, int is_tattoo)
{
	custom_tilemap_draw(screen, bitmap, cliprect, 0, m_pf1_tilemap_16x16, 0, m_pf2_tilemap_16x16, m_pf1_rowscroll_ptr, m_pf12_control[1], m_pf12_control[2], m_pf12_control[5] & 0xff, m_pf12_control[6] & 0xff, 0xf, 4, 0xff, flags, priority, is_tattoo);
}

void deco16ic_device::tilemap_12_combine_draw(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int flags, UINT32 priority, int is_tattoo)
{
	custom_tilemap_draw(screen, bitmap, cliprect, 0, m_pf1_tilemap_16x16, 0, m_pf2_tilemap_16x16, m_pf1_rowscroll_ptr, m_pf12_control[1], m_pf12_control[2], m_pf12_control[5] & 0xff, m_pf12_control[6] & 0xff, 0xf, 4, 0xff, flags, priority, is_tattoo);
}

//-------------------------------------------------
//  static_set_palette_tag: Set the tag of the
//  palette device
//-------------------------------------------------

void deco16ic_device::static_set_palette_tag(device_t &device, const char *tag)
{
	downcast<deco16ic_device &>(device).m_palette.set_tag(tag);
}
