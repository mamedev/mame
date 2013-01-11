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
    Fighter's History           MBF     101     52,153          56,74           [Scratched] 200, 153, 170
    Heavy Smash                 MBG     156     52              141                         153*3
    Night Slashers              MBH     156     52,52,52,153,153,153    74, 141 104         99,200
    Locked N Loaded             MBM     101     ?               74,74           146         113,186,187
    Joe & Mac Return            MBN     156     52              141                         223,223
2   Charlie Ninja               MBR     156     52              141                         223,223
    World Cup Volleyball 95     MBX     156     52              141             ?
    Backfire!                   MBZ     156     52,52,153,153   141,141         ?           223
2*  Ganbare Gonta               MCB     156     52              141                         223,223
    Chain Reaction/Magical Drop MCC     156     52              141                         223,223
    Dunk Dream 95               MCE     156     [MLC]           [MLC]
2   Osman/Cannon Dancer         MCF     156     52              141                         223,223
    Avengers In Galactic Storm  MCG     SH2     [MLC]           [MLC]
    Stadium Hero 96             MCM     156     [MLC]           [MLC]           146

    Sotsugyo Shousho                    59      52              74              146?
    Lemmings                    ---     59      52,52,71,71     None            75
    Tattoo Assassins            ---     101     52,52,71,71     141, 141?       ?           99, ?

Note 1: Mitchell game on DECO PCB board number DEC-22V0 (S-NK-3220)
Note 2: Mitchell games on DECO PCB board number MT5601-0
Note *: Ganbare! Gonta!! 2 / Lady Killer Part 2 - Party Time

    Custom chip 59  = 68000 cpu
    Custom chip 101 = Arm6 cpu
    Custom chip 113 = Alpha blending
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
#include "ui.h"


struct deco16ic_state
{
	screen_device *screen;

	UINT16 *pf1_data, *pf2_data;
	UINT16 *pf12_control;

	const UINT16 *pf1_rowscroll_ptr, *pf2_rowscroll_ptr;

	tilemap_t *pf1_tilemap_16x16, *pf2_tilemap_16x16;
	tilemap_t *pf1_tilemap_8x8, *pf2_tilemap_8x8;

	deco16_bank_cb  bank_cb[2];

	int use_custom_pf1, use_custom_pf2;

	int pf1_bank, pf2_bank;
	int pf12_16x16_gfx_bank, pf12_8x8_gfx_bank;
	int pf1_colourmask, pf2_colourmask;
	int pf1_colour_bank, pf2_colour_bank;
	int pf1_trans_mask, pf2_trans_mask;

	int pf12_last_small, pf12_last_big;

	int pf1_8bpp_mode;
};

/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE deco16ic_state *get_safe_token( device_t *device )
{
	assert(device != NULL);
	assert(device->type() == DECO16IC);

	return (deco16ic_state *)downcast<deco16ic_device *>(device)->token();
}

INLINE const deco16ic_interface *get_interface( device_t *device )
{
	assert(device != NULL);
	assert((device->type() == DECO16IC));
	return (const deco16ic_interface *) device->static_config();
}

/*****************************************************************************************/

TILEMAP_MAPPER_MEMBER(deco16ic_device::deco16_scan_rows)
{
	/* logical (col,row) -> memory offset */
	return (col & 0x1f) + ((row & 0x1f) << 5) + ((col & 0x20) << 5) + ((row & 0x20) << 6);
}

TILE_GET_INFO_MEMBER(deco16ic_device::get_pf2_tile_info)
{
	deco16ic_state *deco16ic = get_safe_token(this);
	UINT16 tile = deco16ic->pf2_data[tile_index];
	UINT8 colour = (tile >> 12) & 0xf;
	UINT8 flags = 0;

	if (tile & 0x8000)
	{
		if ((deco16ic->pf12_control[6] >> 8) & 0x01)
		{
			flags |= TILE_FLIPX;
			colour &= 0x7;
		}
		if ((deco16ic->pf12_control[6] >> 8) & 0x02)
		{
			flags |= TILE_FLIPY;
			colour &= 0x7;
		}
	}

	SET_TILE_INFO_MEMBER(
			deco16ic->pf12_16x16_gfx_bank,
			(tile & 0xfff) | deco16ic->pf2_bank,
			(colour & deco16ic->pf2_colourmask) + deco16ic->pf2_colour_bank,
			flags);
}

TILE_GET_INFO_MEMBER(deco16ic_device::get_pf1_tile_info)
{
	deco16ic_state *deco16ic = get_safe_token(this);
	UINT16 tile = deco16ic->pf1_data[tile_index];
	UINT8 colour = (tile >> 12) & 0xf;
	UINT8 flags = 0;

	if (tile & 0x8000)
	{
		if ((deco16ic->pf12_control[6] >> 0) & 0x01)
		{
			flags |= TILE_FLIPX;
			colour &= 0x7;
		}
		if ((deco16ic->pf12_control[6] >> 0) & 0x02)
		{
			flags |= TILE_FLIPY;
			colour &= 0x7;
		}
	}

	if (deco16ic->pf1_8bpp_mode)
	{
		// Captain America operates this chip in 8bpp mode.
		// In 8bpp mode you appear to only get 1 layer, not 2, but you also
		// have an extra 2 tile bits, and 2 less colour bits.
		SET_TILE_INFO_MEMBER(
				deco16ic->pf12_16x16_gfx_bank,
				(tile & 0x3fff) | deco16ic->pf1_bank,
				((colour & deco16ic->pf1_colourmask) + deco16ic->pf1_colour_bank)>>2,
				flags);
	}
	else
	{
		SET_TILE_INFO_MEMBER(
				deco16ic->pf12_16x16_gfx_bank,
				(tile & 0xfff) | deco16ic->pf1_bank,
				(colour & deco16ic->pf1_colourmask) + deco16ic->pf1_colour_bank,
				flags);
	}
}

TILE_GET_INFO_MEMBER(deco16ic_device::get_pf2_tile_info_b)
{
	deco16ic_state *deco16ic = get_safe_token(this);
	UINT16 tile = deco16ic->pf2_data[tile_index];
	UINT8 colour = (tile >> 12) & 0xf;
	UINT8 flags = 0;

	if (tile & 0x8000)
	{
		if ((deco16ic->pf12_control[6] >> 8) & 0x01)
		{
			flags |= TILE_FLIPX;
			colour &= 0x7;
		}
		if ((deco16ic->pf12_control[6] >> 8) & 0x02)
		{
			flags |= TILE_FLIPY;
			colour &= 0x7;
		}
	}

	SET_TILE_INFO_MEMBER(
			deco16ic->pf12_8x8_gfx_bank,
			(tile & 0xfff) | deco16ic->pf2_bank,
			(colour & deco16ic->pf2_colourmask) + deco16ic->pf2_colour_bank,
			flags);
}

TILE_GET_INFO_MEMBER(deco16ic_device::get_pf1_tile_info_b)
{
	deco16ic_state *deco16ic = get_safe_token(this);
	UINT16 tile = deco16ic->pf1_data[tile_index];
	UINT8 colour = (tile >> 12) & 0xf;
	UINT8 flags = 0;

	if (tile & 0x8000)
	{
		if ((deco16ic->pf12_control[6] >> 0) & 0x01)
		{
			flags |= TILE_FLIPX;
			colour &= 0x7;
		}
		if ((deco16ic->pf12_control[6] >> 0) & 0x02)
		{
			flags |= TILE_FLIPY;
			colour &= 0x7;
		}
	}

	SET_TILE_INFO_MEMBER(
			deco16ic->pf12_8x8_gfx_bank,
			(tile & 0xfff) | deco16ic->pf1_bank,
			(colour & deco16ic->pf1_colourmask) + deco16ic->pf1_colour_bank,
			flags);
}


/*****************************************************************************************/

/*
    Consider this the 'reference rasterizer' for the 56/74/141 tilemap chips - it implements
    simultaneous row & column scroll which the Mame tilemap core cannot do.  It also
    implements combining the 4BPP output of two tilemaps into 8BPP output.  This function
    is automatically called when the tilemap is in a state the cannot be properly rendered
    by the Mame core.
*/

template<class _BitmapClass>
static void custom_tilemap_draw(
	device_t *device,
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
	running_machine &machine = device->machine();
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
				bitmap.pix(y, x) = machine.pens[p];
				if (machine.priority_bitmap.valid())
				{
					UINT8 *pri = &machine.priority_bitmap.pix8(y);
					pri[x] |= priority;
				}
			}
		}

		src_y = (src_y + 1) & height_mask;
	}
}

/******************************************************************************/

/* captain america seems to have a similar 8bpp feature to robocop2, investigate merging */
void deco16ic_set_pf1_8bpp_mode(device_t *device, int mode)
{
	deco16ic_state *deco16ic = get_safe_token(device);
	deco16ic->pf1_8bpp_mode = mode;
}

/* robocop 2 can switch between 2 tilemaps at 4bpp, or 1 at 8bpp */
void deco16ic_set_tilemap_colour_mask( device_t *device, int tmap, int mask )
{
	deco16ic_state *deco16ic = get_safe_token(device);

	switch (tmap)
	{
	case 0: deco16ic->pf1_colourmask = mask; break;
	case 1: deco16ic->pf2_colourmask = mask; break;
	}
}

void deco16ic_pf12_set_gfxbank( device_t *device, int small, int big )
{
	deco16ic_state *deco16ic = get_safe_token(device);

	if (deco16ic->pf12_last_big != big)
	{
		if (deco16ic->pf1_tilemap_16x16)
			deco16ic->pf1_tilemap_16x16->mark_all_dirty();
		if (deco16ic->pf2_tilemap_16x16)
			deco16ic->pf2_tilemap_16x16->mark_all_dirty();

		deco16ic->pf12_last_big = big;
	}
	deco16ic->pf12_16x16_gfx_bank = big;

	if (deco16ic->pf12_last_small != small)
	{
		if (deco16ic->pf1_tilemap_8x8)
			deco16ic->pf1_tilemap_8x8->mark_all_dirty();
		if (deco16ic->pf2_tilemap_8x8)
			deco16ic->pf2_tilemap_8x8->mark_all_dirty();

		deco16ic->pf12_last_small = small;
	}
	deco16ic->pf12_8x8_gfx_bank = small;
}

/* stoneage has broken scroll registers */
void deco16ic_set_scrolldx( device_t *device, int tmap, int size, int dx, int dx_if_flipped )
{
	deco16ic_state *deco16ic = get_safe_token(device);

	switch (tmap)
	{
	case 0:
		if (!size)
			deco16ic->pf1_tilemap_16x16->set_scrolldx(dx, dx_if_flipped);
		else
			deco16ic->pf1_tilemap_8x8->set_scrolldx(dx, dx_if_flipped);
		break;
	case 1:
		if (!size)
			deco16ic->pf2_tilemap_16x16->set_scrolldx(dx, dx_if_flipped);
		else
			deco16ic->pf2_tilemap_8x8->set_scrolldx(dx, dx_if_flipped);
		break;
	}
}

/* cninjabl does not enable background layers */
void deco16ic_set_enable( device_t *device, int tmap, int enable )
{
	deco16ic_state *deco16ic = get_safe_token(device);

	int shift = (tmap & 1) ? 15 : 7;
	deco16ic->pf12_control[5] &= ~(1 << shift);
	deco16ic->pf12_control[5] |= (enable & 1) << shift;
}


/******************************************************************************/

WRITE16_DEVICE_HANDLER( deco16ic_pf1_data_w )
{
	deco16ic_state *deco16ic = get_safe_token(device);

	COMBINE_DATA(&deco16ic->pf1_data[offset]);

	deco16ic->pf1_tilemap_8x8->mark_tile_dirty(offset);
//  if (offset < 0x800)
		deco16ic->pf1_tilemap_16x16->mark_tile_dirty(offset);
}

WRITE16_DEVICE_HANDLER( deco16ic_pf2_data_w )
{
	deco16ic_state *deco16ic = get_safe_token(device);

	COMBINE_DATA(&deco16ic->pf2_data[offset]);

	deco16ic->pf2_tilemap_8x8->mark_tile_dirty(offset);
//  if (offset < 0x800)
		deco16ic->pf2_tilemap_16x16->mark_tile_dirty(offset);
}

READ16_DEVICE_HANDLER( deco16ic_pf1_data_r )
{
	deco16ic_state *deco16ic = get_safe_token(device);
	return deco16ic->pf1_data[offset];
}

READ16_DEVICE_HANDLER( deco16ic_pf2_data_r )
{
	deco16ic_state *deco16ic = get_safe_token(device);
	return deco16ic->pf2_data[offset];
}

WRITE16_DEVICE_HANDLER( deco16ic_pf_control_w )
{
	deco16ic_state *deco16ic = get_safe_token(device);
	space.machine().primary_screen->update_partial(space.machine().primary_screen->vpos());

	COMBINE_DATA(&deco16ic->pf12_control[offset]);
}

READ16_DEVICE_HANDLER( deco16ic_pf_control_r )
{
	deco16ic_state *deco16ic = get_safe_token(device);
	return deco16ic->pf12_control[offset];
}


READ32_DEVICE_HANDLER ( deco16ic_pf_control_dword_r )
{
	return deco16ic_pf_control_r(device, space, offset, 0xffff)^0xffff0000;
}

WRITE32_DEVICE_HANDLER( deco16ic_pf_control_dword_w )
{
	deco16ic_pf_control_w(device, space, offset, data & 0xffff, mem_mask & 0xffff);
}

READ32_DEVICE_HANDLER( deco16ic_pf1_data_dword_r )
{
	return deco16ic_pf1_data_r(device, space, offset, 0xffff)^0xffff0000;
}

WRITE32_DEVICE_HANDLER( deco16ic_pf1_data_dword_w )
{
	deco16ic_pf1_data_w(device, space, offset, data & 0xffff, mem_mask & 0xffff);
}

READ32_DEVICE_HANDLER( deco16ic_pf2_data_dword_r )
{
	return deco16ic_pf2_data_r(device, space, offset, 0xffff)^0xffff0000;
}

WRITE32_DEVICE_HANDLER( deco16ic_pf2_data_dword_w )
{
	deco16ic_pf2_data_w(device, space, offset, data & 0xffff, mem_mask & 0xffff);
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

void deco16ic_pf_update( device_t *device, const UINT16 *rowscroll_1_ptr, const UINT16 *rowscroll_2_ptr )
{
	deco16ic_state *deco16ic = get_safe_token(device);
	int bank1, bank2;

	/* Update scrolling and tilemap enable */
	deco16ic->pf1_rowscroll_ptr = rowscroll_1_ptr;
	deco16ic->pf2_rowscroll_ptr = rowscroll_2_ptr;
	deco16ic->use_custom_pf2 = deco16_pf_update(deco16ic->pf2_tilemap_8x8, deco16ic->pf2_tilemap_16x16, rowscroll_2_ptr, deco16ic->pf12_control[3], deco16ic->pf12_control[4], deco16ic->pf12_control[5] >> 8, deco16ic->pf12_control[6] >> 8);
	deco16ic->use_custom_pf1 = deco16_pf_update(deco16ic->pf1_tilemap_8x8, deco16ic->pf1_tilemap_16x16, rowscroll_1_ptr, deco16ic->pf12_control[1], deco16ic->pf12_control[2], deco16ic->pf12_control[5] & 0xff, deco16ic->pf12_control[6] & 0xff);

	/* Update banking and global flip state */
	if (deco16ic->bank_cb[0])
	{
		bank1 = deco16ic->bank_cb[0](deco16ic->pf12_control[7] & 0xff);

		if (bank1 != deco16ic->pf1_bank)
		{
			if (deco16ic->pf1_tilemap_8x8)
				deco16ic->pf1_tilemap_8x8->mark_all_dirty();
			if (deco16ic->pf1_tilemap_16x16)
				deco16ic->pf1_tilemap_16x16->mark_all_dirty();

			deco16ic->pf1_bank = bank1;
		}
	}

	if (deco16ic->bank_cb[1])
	{
		bank2 = deco16ic->bank_cb[1](deco16ic->pf12_control[7] >> 8);

		if (bank2 != deco16ic->pf2_bank)
		{
			if (deco16ic->pf2_tilemap_8x8)
				deco16ic->pf2_tilemap_8x8->mark_all_dirty();
			if (deco16ic->pf2_tilemap_16x16)
				deco16ic->pf2_tilemap_16x16->mark_all_dirty();

			deco16ic->pf2_bank = bank2;
		}
	}
}

/*****************************************************************************************/

void deco16ic_print_debug_info(device_t *device, bitmap_ind16 &bitmap)
{
	deco16ic_state *deco16ic = get_safe_token(device);
	char buf[64*5];

	if (device->machine().input().code_pressed(KEYCODE_O))
		return;

	if (deco16ic->pf12_control)
	{
		sprintf(buf,"%04X %04X %04X %04X\n", deco16ic->pf12_control[0], deco16ic->pf12_control[1], deco16ic->pf12_control[2], deco16ic->pf12_control[3]);
		sprintf(&buf[strlen(buf)],"%04X %04X %04X %04X\n", deco16ic->pf12_control[4], deco16ic->pf12_control[5], deco16ic->pf12_control[6], deco16ic->pf12_control[7]);
	}
	else
		sprintf(buf, "\n\n");

	ui_draw_text(&device->machine().render().ui_container(), buf, 60, 40);
}

/*****************************************************************************************/

template<class _BitmapClass>
void deco16ic_tilemap_1_draw_common( device_t *device, _BitmapClass &bitmap, const rectangle &cliprect, int flags, UINT32 priority )
{
	deco16ic_state *deco16ic = get_safe_token(device);

	if (deco16ic->use_custom_pf1)
	{
		custom_tilemap_draw(device, bitmap, cliprect, deco16ic->pf1_tilemap_8x8, deco16ic->pf1_tilemap_16x16, 0, 0, deco16ic->pf1_rowscroll_ptr, deco16ic->pf12_control[1], deco16ic->pf12_control[2], deco16ic->pf12_control[5] & 0xff, deco16ic->pf12_control[6] & 0xff, 0, 0, deco16ic->pf1_trans_mask, flags, priority, 0);
	}
	else
	{
		if (deco16ic->pf1_tilemap_8x8)
			deco16ic->pf1_tilemap_8x8->draw(bitmap, cliprect, flags, priority);
		if (deco16ic->pf1_tilemap_16x16)
			deco16ic->pf1_tilemap_16x16->draw(bitmap, cliprect, flags, priority);
	}
}

void deco16ic_tilemap_1_draw( device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect, int flags, UINT32 priority )
{ deco16ic_tilemap_1_draw_common(device, bitmap, cliprect, flags, priority); }

void deco16ic_tilemap_1_draw( device_t *device, bitmap_rgb32 &bitmap, const rectangle &cliprect, int flags, UINT32 priority )
{ deco16ic_tilemap_1_draw_common(device, bitmap, cliprect, flags, priority); }


template<class _BitmapClass>
void deco16ic_tilemap_2_draw_common(device_t *device, _BitmapClass &bitmap, const rectangle &cliprect, int flags, UINT32 priority)
{
	deco16ic_state *deco16ic = get_safe_token(device);

	if (deco16ic->use_custom_pf2)
	{
		custom_tilemap_draw(device, bitmap, cliprect, deco16ic->pf2_tilemap_8x8, deco16ic->pf2_tilemap_16x16, 0, 0, deco16ic->pf2_rowscroll_ptr, deco16ic->pf12_control[3], deco16ic->pf12_control[4], deco16ic->pf12_control[5] >> 8, deco16ic->pf12_control[6] >> 8, 0, 0, deco16ic->pf2_trans_mask, flags, priority, 0);
	}
	else
	{
		if (deco16ic->pf2_tilemap_8x8)
			deco16ic->pf2_tilemap_8x8->draw(bitmap, cliprect, flags, priority);
		if (deco16ic->pf2_tilemap_16x16)
			deco16ic->pf2_tilemap_16x16->draw(bitmap, cliprect, flags, priority);
	}
}

void deco16ic_tilemap_2_draw( device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect, int flags, UINT32 priority )
{ deco16ic_tilemap_2_draw_common(device, bitmap, cliprect, flags, priority); }

void deco16ic_tilemap_2_draw( device_t *device, bitmap_rgb32 &bitmap, const rectangle &cliprect, int flags, UINT32 priority )
{ deco16ic_tilemap_2_draw_common(device, bitmap, cliprect, flags, priority); }


/*****************************************************************************************/

// Combines the output of two 4BPP tilemaps into an 8BPP tilemap
void deco16ic_tilemap_12_combine_draw(device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect, int flags, UINT32 priority, int is_tattoo)
{
	deco16ic_state *deco16ic = get_safe_token(device);
	custom_tilemap_draw(device, bitmap, cliprect, 0, deco16ic->pf1_tilemap_16x16, 0, deco16ic->pf2_tilemap_16x16, deco16ic->pf1_rowscroll_ptr, deco16ic->pf12_control[1], deco16ic->pf12_control[2], deco16ic->pf12_control[5] & 0xff, deco16ic->pf12_control[6] & 0xff, 0xf, 4, 0xff, flags, priority, is_tattoo);
}

void deco16ic_tilemap_12_combine_draw(device_t *device, bitmap_rgb32 &bitmap, const rectangle &cliprect, int flags, UINT32 priority, int is_tattoo)
{
	deco16ic_state *deco16ic = get_safe_token(device);
	custom_tilemap_draw(device, bitmap, cliprect, 0, deco16ic->pf1_tilemap_16x16, 0, deco16ic->pf2_tilemap_16x16, deco16ic->pf1_rowscroll_ptr, deco16ic->pf12_control[1], deco16ic->pf12_control[2], deco16ic->pf12_control[5] & 0xff, deco16ic->pf12_control[6] & 0xff, 0xf, 4, 0xff, flags, priority, is_tattoo);
}

const device_type DECO16IC = &device_creator<deco16ic_device>;

deco16ic_device::deco16ic_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, DECO16IC, "Data East IC 55 / 56 / 74 / 141", tag, owner, clock)
{
	m_token = global_alloc_clear(deco16ic_state);
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void deco16ic_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void deco16ic_device::device_start()
{
	deco16ic_state *deco16ic = get_safe_token(this);
	const deco16ic_interface *intf = get_interface(this);

	deco16ic->bank_cb[0] = intf->bank_cb0;
	deco16ic->bank_cb[1] = intf->bank_cb1;

	deco16ic->pf1_trans_mask = intf->trans_mask1;
	deco16ic->pf2_trans_mask = intf->trans_mask2;

	deco16ic->pf1_colour_bank = intf->col_base1;
	deco16ic->pf2_colour_bank = intf->col_base2;

	deco16ic->pf1_colourmask = intf->col_mask1;
	deco16ic->pf2_colourmask = intf->col_mask2;

	int fullheight = 0;
	int fullwidth = 0;

	if (intf->full_width12&2)
		fullheight = 1;

	if (intf->full_width12&1)
		fullwidth = 1;

	deco16ic->pf1_tilemap_16x16 = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(deco16ic_device::get_pf1_tile_info),this), tilemap_mapper_delegate(FUNC(deco16ic_device::deco16_scan_rows),this), 16, 16, fullwidth ? 64 : 32, fullheight ?64 : 32);
//  deco16ic->pf1_tilemap_8x8 = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(deco16ic_device::get_pf1_tile_info_b),this), TILEMAP_SCAN_ROWS, 8, 8, intf->full_width12 ? 64 : 32, 32);
	deco16ic->pf1_tilemap_8x8 = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(deco16ic_device::get_pf1_tile_info_b),this), TILEMAP_SCAN_ROWS, 8, 8, 64 , 32); // nitroball

	deco16ic->pf12_8x8_gfx_bank = intf->_8x8_gfxregion;
	deco16ic->pf12_16x16_gfx_bank = intf->_16x16_gfxregion;

	if (intf->split)
		deco16ic->pf2_tilemap_16x16 = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(deco16ic_device::get_pf2_tile_info),this), tilemap_mapper_delegate(FUNC(deco16ic_device::deco16_scan_rows),this), 16, 16, fullwidth ? 64 : 32, fullheight ? 64 : 32);
	else
		deco16ic->pf2_tilemap_16x16 = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(deco16ic_device::get_pf2_tile_info),this), tilemap_mapper_delegate(FUNC(deco16ic_device::deco16_scan_rows),this), 16, 16, fullwidth ? 64 : 32, fullheight ? 64 : 32);

	deco16ic->pf2_tilemap_8x8 = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(deco16ic_device::get_pf2_tile_info_b),this), TILEMAP_SCAN_ROWS, 8, 8, fullwidth ? 64 : 32, fullheight ? 64 : 32);

	deco16ic->pf1_tilemap_8x8->set_transparent_pen(0);
	deco16ic->pf2_tilemap_8x8->set_transparent_pen(0);
	deco16ic->pf1_tilemap_16x16->set_transparent_pen(0);
	deco16ic->pf2_tilemap_16x16->set_transparent_pen(0);

	if (intf->split) /* Caveman Ninja only */
		deco16ic->pf2_tilemap_16x16->set_transmask(0, 0x00ff, 0xff01);

	deco16ic->pf1_8bpp_mode = 0;

	deco16ic->pf1_data = auto_alloc_array_clear(machine(), UINT16, 0x2000 / 2);
	deco16ic->pf2_data = auto_alloc_array_clear(machine(), UINT16, 0x2000 / 2);
	deco16ic->pf12_control = auto_alloc_array_clear(machine(), UINT16, 0x10 / 2);


	save_item(NAME(deco16ic->use_custom_pf1));
	save_item(NAME(deco16ic->use_custom_pf2));
	save_item(NAME(deco16ic->pf1_bank));
	save_item(NAME(deco16ic->pf2_bank));
	save_item(NAME(deco16ic->pf12_8x8_gfx_bank));
	save_item(NAME(deco16ic->pf12_16x16_gfx_bank));
	save_item(NAME(deco16ic->pf12_last_small));
	save_item(NAME(deco16ic->pf12_last_big));

	save_item(NAME(deco16ic->pf1_8bpp_mode));

	save_pointer(NAME(deco16ic->pf1_data), 0x2000 / 2);
	save_pointer(NAME(deco16ic->pf2_data), 0x2000 / 2);
	save_pointer(NAME(deco16ic->pf12_control), 0x10 / 2);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void deco16ic_device::device_reset()
{
	deco16ic_state *deco16ic = get_safe_token(this);

	deco16ic->pf1_bank = deco16ic->pf2_bank = 0;
	deco16ic->pf12_last_small = deco16ic->pf12_last_big = -1;
	deco16ic->use_custom_pf1 = deco16ic->use_custom_pf2 = 0;
	deco16ic->pf1_rowscroll_ptr = 0;
	deco16ic->pf2_rowscroll_ptr = 0;
}
