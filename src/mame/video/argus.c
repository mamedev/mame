/***************************************************************************

Functions to emulate the video hardware of the machine.


BG RAM format [Argus and Butasan]
-----------------------------------------------------------------------------
 +0         +1
 xxxx xxxx  ---- ---- = 1st - 8th bits of tile number
 ---- ----  xx-- ---- = 9th and 10th bit of tile number
 ---- ----  --x- ---- = flip y
 ---- ----  ---x ---- = flip x
 ---- ----  ---- xxxx = color

BG RAM format [Valtric]
-----------------------------------------------------------------------------
 +0         +1
 xxxx xxxx  ---- ---- = 1st - 8th bits of tile number
 ---- ----  xx-- ---- = 9th and 10th bit of tile number
 ---- ----  --x- ---- = 11th bit of tile number
 ---- ----  ---- xxxx = color


Text RAM format [Argus, Valtric and Butasan]
-----------------------------------------------------------------------------
 +0         +1
 xxxx xxxx  ---- ---- = low bits of tile number
 ---- ----  xx-- ---- = high bits of tile number
 ---- ----  --x- ---- = flip y
 ---- ----  ---x ---- = flip x
 ---- ----  ---- xxxx = color


Sprite RAM format [Argus]
-----------------------------------------------------------------------------
 +11        +12        +13        +14        +15
 xxxx xxxx  ---- ----  ---- ----  ---- ----  ---- ---- = sprite y
 ---- ----  xxxx xxxx  ---- ----  ---- ----  ---- ---- = low bits of sprite x
 ---- ----  ---- ----  xx-- ----  ---- ----  ---- ---- = high bits of tile number
 ---- ----  ---- ----  --x- ----  ---- ----  ---- ---- = flip y
 ---- ----  ---- ----  ---x ----  ---- ----  ---- ---- = flip x
 ---- ----  ---- ----  ---- --x-  ---- ----  ---- ---- = high bit of sprite y
 ---- ----  ---- ----  ---- ---x  ---- ----  ---- ---- = high bit of sprite x
 ---- ----  ---- ----  ---- ----  xxxx xxxx  ---- ---- = low bits of tile number
 ---- ----  ---- ----  ---- ----  ---- ----  ---- x--- = BG1 / sprite priority (Argus only)
 ---- ----  ---- ----  ---- ----  ---- ----  ---- -xxx = color

Sprite RAM format [Valtric]
-----------------------------------------------------------------------------
 +11        +12        +13        +14        +15
 xxxx xxxx  ---- ----  ---- ----  ---- ----  ---- ---- = sprite y
 ---- ----  xxxx xxxx  ---- ----  ---- ----  ---- ---- = low bits of sprite x
 ---- ----  ---- ----  xx-- ----  ---- ----  ---- ---- = high bits of tile number
 ---- ----  ---- ----  --x- ----  ---- ----  ---- ---- = flip y
 ---- ----  ---- ----  ---x ----  ---- ----  ---- ---- = flip x
 ---- ----  ---- ----  ---- --x-  ---- ----  ---- ---- = high bit of sprite y
 ---- ----  ---- ----  ---- ---x  ---- ----  ---- ---- = high bit of sprite x
 ---- ----  ---- ----  ---- ----  xxxx xxxx  ---- ---- = low bits of tile number
 ---- ----  ---- ----  ---- ----  ---- ----  ---- xxxx = color

Sprite RAM format [Butasan]
-----------------------------------------------------------------------------
 +8         +9         +10        +11        +12
 ---- -x--  ---- ----  ---- ----  ---- ----  ---- ---- = flip y
 ---- ---x  ---- ----  ---- ----  ---- ----  ---- ---- = flip x
 ---- ----  ---- xxxx  ---- ----  ---- ----  ---- ---- = color ($00 - $0B)
 ---- ----  ---- ----  xxxx xxxx  ---- ----  ---- ---- = low bits of sprite x
 ---- ----  ---- ----  ---- ----  ---- ---x  ---- ---- = top bit of sprite x
 ---- ----  ---- ----  ---- ----  ---- ----  xxxx xxxx = low bits of sprite y
 +13        +14        +15
 ---- ---x  ---- ----  ---- ---- = top bit of sprite y
 ---- ----  xxxx xxxx  ---- ---- = low bits of tile number
 ---- ----  ---- ----  ---- xxxx = top bits of tile number

(*) Sprite size is defined by its offset.
    $F000 - $F0FF : 16x32    $F100 - $F2FF : 16x16
    $F300 - $F3FF : 16x32    $F400 - $F57F : 16x16
    $F580 - $F61F : 32x32    $F620 - $F67F : 64x64


Scroll RAM of X and Y coordinates [Argus, Valtric and Butasan]
-----------------------------------------------------------------------------
 +0         +1
 xxxx xxxx  ---- ---- = scroll value
 ---- ----  ---- ---x = top bit of scroll value


Video effect RAM ( $C30C )
-----------------------------------------------------------------------------
 +0
 ---- ---x  = BG enable bit
 ---- --x-  = gray scale effect or tile bank select.


Flip screen controller
-----------------------------------------------------------------------------
 +0
 x--- ----  = flip screen


BG0 palette intensity ( $C47F, $C4FF )
-----------------------------------------------------------------------------
 +0 (c47f)  +1 (c4ff)
 xxxx ----  ---- ---- = red intensity
 ---- xxxx  ---- ---- = green intensity
 ---- ----  xxxx ---- = blue intensity


(*) Things which are not emulated.
 - Color $000 - 00f, $01e, $02e ... are half transparent color.
 - Sprite priority bit may be present in Butasan. But I don't know
   what is happened when it is set.

***************************************************************************/

#include "driver.h"

#include "jalblend.c"

#define BUTASAN_TEXT_RAMSIZE		0x0800
#define BUTASAN_BG0_RAMSIZE			0x0800
#define BUTASAN_TXBACK_RAMSIZE		0x0800
#define BUTASAN_BG0BACK_RAMSIZE		0x0800


UINT8 *argus_paletteram;
UINT8 *argus_txram;
UINT8 *argus_bg0_scrollx;
UINT8 *argus_bg0_scrolly;
UINT8 *argus_bg1ram;
UINT8 *argus_bg1_scrollx;
UINT8 *argus_bg1_scrolly;
UINT8 *butasan_bg1ram;

static UINT8 *argus_dummy_bg0ram;
static UINT8 *butasan_txram;
static UINT8 *butasan_bg0ram;
static UINT8 *butasan_bg0backram;
static UINT8 *butasan_txbackram;

static tilemap *tx_tilemap  = NULL;
static tilemap *bg0_tilemap = NULL;
static tilemap *bg1_tilemap = NULL;
static tilemap *bombsa_bg_tilemap = NULL;

static UINT8 argus_bg_status    = 0x01;
static UINT8 butasan_bg1_status = 0x01;
static UINT8 argus_bg_purple  = 0;
static UINT8 argus_flipscreen = 0;

static int argus_palette_intensity = 0;

/* VROM scroll related for Argus */
static int lowbitscroll = 0;
static int prvscrollx = 0;

static UINT8 bombsa_ram_page;
static UINT8* bomba_otherram;

static int valtric_mosaic=0;
static bitmap_t *mosaicbitmap;


/***************************************************************************
  Callbacks for the tilemap code
***************************************************************************/

static TILE_GET_INFO( argus_get_tx_tile_info )
{
	int hi, lo;

	lo = argus_txram[  tile_index << 1  ];
	hi = argus_txram[ (tile_index << 1) + 1 ];

	SET_TILE_INFO(
			3,
			((hi & 0xc0) << 2) | lo,
			hi & 0x0f,
			TILE_FLIPYX((hi & 0x30) >> 4));
}

static TILE_GET_INFO( argus_get_bg0_tile_info )
{
	int hi, lo;

	lo = argus_dummy_bg0ram[  tile_index << 1  ];
	hi = argus_dummy_bg0ram[ (tile_index << 1) + 1 ];

	SET_TILE_INFO(
			1,
			((hi & 0xc0) << 2) | lo,
			hi & 0x0f,
			TILE_FLIPYX((hi & 0x30) >> 4));
}

static TILE_GET_INFO( argus_get_bg1_tile_info )
{
	int hi, lo;

	lo = argus_bg1ram[  tile_index << 1  ];
	hi = argus_bg1ram[ (tile_index << 1) + 1 ];

	SET_TILE_INFO(
			2,
			lo,
			hi & 0x0f,
			TILE_FLIPYX((hi & 0x30) >> 4));
}

static TILE_GET_INFO( valtric_get_tx_tile_info )
{
	int hi, lo;

	lo = argus_txram[  tile_index << 1  ];
	hi = argus_txram[ (tile_index << 1) + 1 ];

	SET_TILE_INFO(
			2,
			((hi & 0xc0) << 2) | lo,
			hi & 0x0f,
			TILE_FLIPYX((hi & 0x30) >> 4));
}

static TILE_GET_INFO( valtric_get_bg_tile_info )
{
	int hi, lo, color, tile;

	lo = argus_bg1ram[  tile_index << 1  ];
	hi = argus_bg1ram[ (tile_index << 1) + 1 ];

	tile = ((hi & 0xc0) << 2) | ((hi & 0x20) << 5) | lo;
	color = hi & 0x0f;

	SET_TILE_INFO(
			1,
			tile,
			color,
			0);
}

static TILE_GET_INFO( butasan_get_tx_tile_info )
{
	int hi, lo;

	tile_index ^= 0x3e0;

	lo = butasan_txram[  tile_index << 1  ];
	hi = butasan_txram[ (tile_index << 1) + 1 ];

	SET_TILE_INFO(
			3,
			((hi & 0xc0) << 2) | lo,
			hi & 0x0f,
			TILE_FLIPYX((hi & 0x30) >> 4));
}

static TILE_GET_INFO( butasan_get_bg0_tile_info )
{
	int hi, lo;
	int attrib;

	attrib = (tile_index & 0x00f) | ((tile_index & 0x1e0) >> 1);
	attrib |= ((tile_index & 0x200) >> 1) | ((tile_index & 0x010) << 5);
	attrib ^= 0x0f0;

	lo = butasan_bg0ram[  attrib << 1  ];
	hi = butasan_bg0ram[ (attrib << 1) + 1 ];

	SET_TILE_INFO(
			1,
			((hi & 0xc0) << 2) | lo,
			hi & 0x0f,
			TILE_FLIPYX((hi & 0x30) >> 4));
}






static TILE_GET_INFO( butasan_get_bg1_tile_info )
{
	int bank, tile, attrib, color;

	attrib = (tile_index & 0x00f) | ((tile_index & 0x3e0) >> 1)
				| ((tile_index & 0x010) << 5);
	attrib ^= 0x0f0;

	bank = (butasan_bg1_status & 0x02) << 7;
	tile = butasan_bg1ram[ attrib ] | bank;
	color = (tile & 0x80) >> 7;

	SET_TILE_INFO(
			2,
			tile,
			color,
			0);
}


/***************************************************************************
  Initialize and destroy video hardware emulation
***************************************************************************/

VIDEO_START( argus )
{
	lowbitscroll = 0;
	/*                           info                      offset             type                  w   h  col  row */
	bg0_tilemap = tilemap_create(argus_get_bg0_tile_info,  tilemap_scan_cols,       16, 16, 32, 32);
	bg1_tilemap = tilemap_create(argus_get_bg1_tile_info,  tilemap_scan_cols,  16, 16, 32, 32);
	tx_tilemap  = tilemap_create(argus_get_tx_tile_info,   tilemap_scan_cols,   8,  8, 32, 32);

	/* dummy RAM for back ground */
	argus_dummy_bg0ram = auto_malloc( 0x800 );
	memset( argus_dummy_bg0ram, 0, 0x800 );

	memset( argus_bg0_scrollx, 0x00, 2 );

	tilemap_set_transparent_pen( bg0_tilemap, 15 );
	tilemap_set_transparent_pen( bg1_tilemap, 15 );
	tilemap_set_transparent_pen( tx_tilemap,  15 );

	jal_blend_table = auto_malloc(0xc00);
	memset(jal_blend_table,0,0xc00) ;
}

VIDEO_START( valtric )
{
	/*                           info                       offset             w   h   col  row */
	bg1_tilemap = tilemap_create(valtric_get_bg_tile_info,  tilemap_scan_cols, 16, 16, 32, 32);
	tx_tilemap  = tilemap_create(valtric_get_tx_tile_info,  tilemap_scan_cols, 8,  8,  32, 32);

	tilemap_set_transparent_pen( bg1_tilemap, 15 );
	tilemap_set_transparent_pen( tx_tilemap,  15 );
	mosaicbitmap = video_screen_auto_bitmap_alloc(machine->primary_screen);
	jal_blend_table = auto_malloc(0xc00);
	memset(jal_blend_table,0,0xc00) ;
}

VIDEO_START( butasan )
{
	/*                           info                       offset             type                 w   h  col  row */
	bg0_tilemap = tilemap_create(butasan_get_bg0_tile_info, tilemap_scan_rows,       16, 16, 32, 32);
	bg1_tilemap = tilemap_create(butasan_get_bg1_tile_info, tilemap_scan_rows,       16, 16, 32, 32);
	tx_tilemap  = tilemap_create(butasan_get_tx_tile_info,  tilemap_scan_rows,   8,  8, 32, 32);

	butasan_txram = auto_malloc( BUTASAN_TEXT_RAMSIZE );
	butasan_bg0ram = auto_malloc( BUTASAN_BG0_RAMSIZE );
	butasan_txbackram = auto_malloc( BUTASAN_TXBACK_RAMSIZE );
	butasan_bg0backram = auto_malloc( BUTASAN_BG0BACK_RAMSIZE );

	memset( butasan_txram,      0x00, BUTASAN_TEXT_RAMSIZE );
	memset( butasan_bg0ram,     0x00, BUTASAN_BG0_RAMSIZE );
	memset( butasan_txbackram,  0x00, BUTASAN_TXBACK_RAMSIZE );
	memset( butasan_bg0backram, 0x00, BUTASAN_BG0BACK_RAMSIZE );

	tilemap_set_transparent_pen( tx_tilemap,  15 );

	jal_blend_table = auto_malloc(0xc00);
	memset(jal_blend_table,0,0xc00) ;
}

#if 0
static TILE_GET_INFO( bombsa_get_tx_alt_tile_info )
{
	int hi, lo;

	lo = bombsa_tx_alt_ram[  tile_index << 1  ];
	hi = bombsa_tx_alt_ram[ (tile_index << 1) + 1 ];

	SET_TILE_INFO(
			2,
			((hi & 0xc0) << 2) | lo,
			hi & 0x0f,
			TILE_FLIPYX((hi & 0x30) >> 4));
}
#endif

static TILE_GET_INFO( bombsa_get_bg_tile_info )
{
	int tileno;
	int col;

	tileno = bomba_otherram[tile_index*2] | ( bomba_otherram[tile_index*2+1]<<8);
	col = 0;

	SET_TILE_INFO(
			1,
			tileno,
			col,
			0);
}


VIDEO_START( bombsa )
{
	/*                           info                       offset             type                 w   h  col  row */
//  bg1_tilemap = tilemap_create(valtric_get_bg_tile_info,  tilemap_scan_cols,       16, 16, 32, 32);
	tx_tilemap  = tilemap_create(valtric_get_tx_tile_info,  tilemap_scan_cols,   8,  8, 32, 32);

#if 0
	/* this might be wrong.. but it looks like there may be a tilemap here that can change between 8x8 and 16x16 mode.. */
	/* when you start the game it gets 'corrupted' with data for another layer.. */
	tx_alt_tilemap  = tilemap_create(bombsa_get_tx_alt_tile_info,  tilemap_scan_cols,   8,  8, 32, 32);
#endif

	bombsa_bg_tilemap  = tilemap_create(bombsa_get_bg_tile_info,  tilemap_scan_cols,   16,  16, 64, 32);

#if 0
	bombsa_tx_alt_ram = auto_malloc(0x800);
#endif

//  tilemap_set_transparent_pen( bg1_tilemap, 15 );
	tilemap_set_transparent_pen( tx_tilemap,  15 );
#if 0
	tilemap_set_transparent_pen( tx_alt_tilemap,  15 );
#endif
//  jal_blend_table = auto_malloc(0xc00);
//  memset(jal_blend_table,0,0xc00) ;

	bomba_otherram = auto_malloc(0x1000);
}

/***************************************************************************
  Functions for handler of MAP roms in Argus and palette color
***************************************************************************/

/* Write bg0 pattern data to dummy bg0 ram */
static void argus_write_dummy_rams( int dramoffs, int vromoffs )
{
	int i;
	int voffs;
	int offs;

	UINT8 *VROM1 = memory_region( REGION_USER1 );		/* "ag_15.bin" */
	UINT8 *VROM2 = memory_region( REGION_USER2 );		/* "ag_16.bin" */

	/* offset in pattern data */
	offs = VROM1[ vromoffs ] | ( VROM1[ vromoffs + 1 ] << 8 );
	offs &= 0x7ff;

	voffs = offs * 16;
	for (i = 0 ; i < 8 ; i ++)
	{
		argus_dummy_bg0ram[ dramoffs ] = VROM2[ voffs ];
		argus_dummy_bg0ram[ dramoffs + 1 ] = VROM2[ voffs + 1 ];
		tilemap_mark_tile_dirty( bg0_tilemap, dramoffs >> 1 );
		dramoffs += 2;
		voffs += 2;
	}
}


static void bombsa_change_palette(running_machine *machine, int color, int data)
{
	palette_set_color_rgb(machine, color, pal4bit(data >> 12), pal4bit(data >> 8), pal4bit(data >> 4));
}


static void argus_change_palette(running_machine *machine, int color, int data)
{
	jal_blend_table[color] = data & 0x0f ;

	palette_set_color_rgb(machine, color, pal4bit(data >> 12), pal4bit(data >> 8), pal4bit(data >> 4));
}

static void argus_change_bg_palette(running_machine *machine, int color, int data)
{
	int r, g, b;
	int ir, ig, ib;

	r = (data >> 12) & 0x0f;
	g = (data >>  8) & 0x0f;
	b = (data >>  4) & 0x0f;

	ir = (argus_palette_intensity >> 12) & 0x0f;
	ig = (argus_palette_intensity >>  8) & 0x0f;
	ib = (argus_palette_intensity >>  4) & 0x0f;

	r = (r - ir > 0) ? r - ir : 0;
	g = (g - ig > 0) ? g - ig : 0;
	b = (b - ib > 0) ? b - ib : 0;

	if (argus_bg_status & 2)			/* Gray / purple scale */
	{
		r = (r + g + b) / 3;
		g = b = r;
		if (argus_bg_purple == 2)		/* Purple */
			g = 0;
	}

	palette_set_color_rgb(machine, color, pal4bit(r), pal4bit(g), pal4bit(b));
}


/***************************************************************************
  Memory handler
***************************************************************************/

WRITE8_HANDLER(valtric_mosaic_w)
{
	if(data!=0x80)
	{
		valtric_mosaic=0x0f-(data&0x0f);

		if(valtric_mosaic!=0)
		{
			valtric_mosaic++;
		}

		if(data&0x80)
		{
			valtric_mosaic*=-1;
		}
	}
}


WRITE8_HANDLER( bombsa_pageselect_w )
{
	bombsa_ram_page = data;
}

READ8_HANDLER( argus_txram_r )
{
	return argus_txram[ offset ];
}

WRITE8_HANDLER( argus_txram_w )
{
	argus_txram[ offset ] = data;
	tilemap_mark_tile_dirty(tx_tilemap, offset >> 1);
}

READ8_HANDLER( bombsa_txram_r )
{
#if 0
	if (bombsa_ram_page == 1)
	{
		return argus_txram[ offset ];
	}
	else
	{
		return bombsa_tx_alt_ram[ offset ];
	}
#endif

	return argus_txram[ offset ];

}

WRITE8_HANDLER( bombsa_txram_w )
{
#if 0
	if (bombsa_ram_page == 1)
	{
		argus_txram[ offset ] = data;
		tilemap_mark_tile_dirty(tx_tilemap, offset >> 1);
	}
	else
	{
		/* 8x8 .. */
		bombsa_tx_alt_ram[ offset ] = data;
		tilemap_mark_tile_dirty(tx_alt_tilemap, offset >> 1);
		/* also 16x16 ..? */
	}
#endif
	argus_txram[ offset ] = data;
	tilemap_mark_tile_dirty(tx_tilemap, offset >> 1);
}

READ8_HANDLER( butasan_txram_r )
{
	return butasan_txram[ offset ];
}

WRITE8_HANDLER( butasan_txram_w )
{
	butasan_txram[ offset ] = data;
	tilemap_mark_tile_dirty(tx_tilemap, (offset ^ 0x7c0) >> 1);
}

READ8_HANDLER( argus_bg1ram_r )
{
	return argus_bg1ram[ offset ];
}

WRITE8_HANDLER( argus_bg1ram_w )
{
	argus_bg1ram[ offset ] = data;
	tilemap_mark_tile_dirty(bg1_tilemap, offset >> 1);
}

READ8_HANDLER( butasan_bg0ram_r )
{
	return butasan_bg0ram[ offset ];
}

WRITE8_HANDLER( butasan_bg0ram_w )
{
	int idx;

	butasan_bg0ram[ offset ] = data;

	idx = ((offset & 0x01f) >> 1) | ((offset & 0x400) >> 6);
	idx |= (offset & 0x3e0) ^ 0x1e0;

	tilemap_mark_tile_dirty(bg0_tilemap, idx);
}

READ8_HANDLER( butasan_bg1ram_r )
{
	return butasan_bg1ram[ offset ];
}

WRITE8_HANDLER( butasan_bg1ram_w )
{
	int idx;

	butasan_bg1ram[ offset ] = data;

	idx = (offset & 0x00f) | ((offset & 0x200) >> 5) | ((offset & 0x1f0) << 1);
	idx ^= 0x0f0;

	tilemap_mark_tile_dirty(bg1_tilemap, idx);
}

WRITE8_HANDLER ( argus_bg0_scrollx_w )
{
	if (argus_bg0_scrollx[ offset ] != data)
	{
		argus_bg0_scrollx[ offset ] = data;
	}
}

WRITE8_HANDLER( argus_bg0_scrolly_w )
{
	if (argus_bg0_scrolly[ offset ] != data)
	{
		int scrolly;
		argus_bg0_scrolly[ offset ] = data;
		scrolly = argus_bg0_scrolly[0] | ( (argus_bg0_scrolly[1] & 0x01) << 8);
		if (!argus_flipscreen)
			tilemap_set_scrolly( bg0_tilemap, 0, scrolly );
		else
			tilemap_set_scrolly( bg0_tilemap, 0, (scrolly + 256) & 0x1ff );
	}
}

WRITE8_HANDLER( butasan_bg0_scrollx_w )
{
	if (argus_bg0_scrollx[ offset ] != data)
	{
		int scrollx;
		argus_bg0_scrollx[ offset ] = data;
		scrollx = argus_bg0_scrollx[0] | ( (argus_bg0_scrollx[1] & 0x01) << 8);
		if (!argus_flipscreen)
			tilemap_set_scrollx( bg0_tilemap, 0, scrollx );
		else
			tilemap_set_scrollx( bg0_tilemap, 0, (scrollx + 256) & 0x1ff );
	}
}

WRITE8_HANDLER( argus_bg1_scrollx_w )
{
	if (argus_bg1_scrollx[ offset ] != data)
	{
		int scrollx;
		argus_bg1_scrollx[ offset ] = data;
		scrollx = argus_bg1_scrollx[0] | ( (argus_bg1_scrollx[1] & 0x01) << 8);
		if (!argus_flipscreen)
			tilemap_set_scrollx( bg1_tilemap, 0, scrollx );
		else
			tilemap_set_scrollx( bg1_tilemap, 0, (scrollx + 256) & 0x1ff );
	}
}

WRITE8_HANDLER( argus_bg1_scrolly_w )
{
	if (argus_bg1_scrolly[ offset ] != data)
	{
		int scrolly;
		argus_bg1_scrolly[ offset ] = data;
		scrolly = argus_bg1_scrolly[0] | ( (argus_bg1_scrolly[1] & 0x01) << 8);
		if (!argus_flipscreen)
			tilemap_set_scrolly( bg1_tilemap, 0, scrolly );
		else
			tilemap_set_scrolly( bg1_tilemap, 0, (scrolly + 256) & 0x1ff );
	}
}

WRITE8_HANDLER( argus_bg_status_w )
{
	if (argus_bg_status != data)
	{
		argus_bg_status = data;

		/* Backgound enable */
		tilemap_set_enable(bg1_tilemap, argus_bg_status & 1);

		/* Gray / purple scale */
		if (argus_bg_status & 2)
		{
			int offs;

			for (offs = 0x400 ; offs < 0x500 ; offs ++)
			{
				argus_change_bg_palette( machine, (offs - 0x0400) + 128,
					(argus_paletteram[offs] << 8) | argus_paletteram[offs + 0x0400] );
			}
		}
	}
}

WRITE8_HANDLER( valtric_bg_status_w )
{
	if (argus_bg_status != data)
	{
		argus_bg_status = data;

		/* Backgound enable */
		tilemap_set_enable(bg1_tilemap, argus_bg_status & 1);

		/* Gray / purple scale */
		if (argus_bg_status & 2)
		{
			int offs;

			for (offs = 0x400 ; offs < 0x600 ; offs += 2)
			{
				argus_change_bg_palette( machine, ((offs - 0x0400) >> 1) + 256,
					argus_paletteram[offs | 1] | (argus_paletteram[offs & ~1] << 8));
			}
		}
	}
}

WRITE8_HANDLER( butasan_bg0_status_w )
{
	if (argus_bg_status != data)
	{
		argus_bg_status = data;

		/* Backgound enable */
		tilemap_set_enable(bg0_tilemap, argus_bg_status & 1);
	}
}

WRITE8_HANDLER( argus_flipscreen_w )
{
	if (argus_flipscreen != (data >> 7))
	{
		argus_flipscreen = data >> 7;
		tilemap_set_flip( ALL_TILEMAPS, (argus_flipscreen) ? TILEMAP_FLIPY | TILEMAP_FLIPX : 0);
		if (!argus_flipscreen)
		{
			int scrollx, scrolly;

			if (bg0_tilemap != NULL)
			{
				scrollx = argus_bg0_scrollx[0] | ( (argus_bg0_scrollx[1] & 0x01) << 8);
				tilemap_set_scrollx(bg0_tilemap, 0, scrollx & 0x1ff);

				scrolly = argus_bg0_scrolly[0] | ( (argus_bg0_scrolly[1] & 0x01) << 8);
				tilemap_set_scrolly(bg0_tilemap, 0, scrolly);
			}
			scrollx = argus_bg1_scrollx[0] | ( (argus_bg1_scrollx[1] & 0x01) << 8);
			tilemap_set_scrollx(bg1_tilemap, 0, scrollx);

			scrolly = argus_bg1_scrolly[0] | ( (argus_bg1_scrolly[1] & 0x01) << 8);
			tilemap_set_scrolly(bg1_tilemap, 0, scrolly);
		}
		else
		{
			int scrollx, scrolly;

			if (bg0_tilemap != NULL)
			{
				scrollx = argus_bg0_scrollx[0] | ( (argus_bg0_scrollx[1] & 0x01) << 8);
				tilemap_set_scrollx(bg0_tilemap, 0, (scrollx + 256) & 0x1ff);

				scrolly = argus_bg0_scrolly[0] | ( (argus_bg0_scrolly[1] & 0x01) << 8);
				tilemap_set_scrolly(bg0_tilemap, 0, (scrolly + 256) & 0x1ff);
			}
			scrollx = argus_bg1_scrollx[0] | ( (argus_bg1_scrollx[1] & 0x01) << 8);
			tilemap_set_scrollx(bg1_tilemap, 0, (scrollx + 256) & 0x1ff);

			scrolly = argus_bg1_scrolly[0] | ( (argus_bg1_scrolly[1] & 0x01) << 8);
			tilemap_set_scrolly(bg1_tilemap, 0, (scrolly + 256) & 0x1ff);
		}
	}
}

READ8_HANDLER( argus_paletteram_r )
{
	return argus_paletteram[ offset ];
}

WRITE8_HANDLER( argus_paletteram_w )
{
	int offs;

	argus_paletteram[ offset ] = data;

	if (offset != 0x007f && offset != 0x00ff)
	{
		if (offset >= 0x0000 && offset <= 0x00ff)				/* sprite color */
		{
			if (offset & 0x80)
				offset -= 0x80;

			argus_change_palette( machine, offset,
				(argus_paletteram[offset] << 8) | argus_paletteram[offset + 0x80] );
		}

		else if ( (offset >= 0x0400 && offset <= 0x04ff) ||
				  (offset >= 0x0800 && offset <= 0x08ff) )		/* BG0 color */
		{
			if (offset >= 0x0800)
				offset -= 0x0400;

			argus_change_bg_palette( machine, (offset - 0x0400) + 128,
				(argus_paletteram[offset] << 8) | argus_paletteram[offset + 0x0400] );
		}

		else if ( (offset >= 0x0500 && offset <= 0x05ff) ||
				  (offset >= 0x0900 && offset <= 0x09ff) )		/* BG1 color */
		{
			if (offset >= 0x0900)
				offset -= 0x0400;

			argus_change_palette( machine, (offset - 0x0500) + 384,
				(argus_paletteram[offset] << 8) | argus_paletteram[offset + 0x0400] );
		}

		else if ( (offset >= 0x0700 && offset <= 0x07ff) ||
				  (offset >= 0x0b00 && offset <= 0x0bff) )		/* text color */
		{
			if (offset >= 0x0b00)
				offset -= 0x0400;

			argus_change_palette( machine, (offset - 0x0700) + 640,
				(argus_paletteram[offset] << 8) | argus_paletteram[offset + 0x0400] );
		}
	}
	else
	{
		argus_palette_intensity = (argus_paletteram[0x007f] << 8) | argus_paletteram[0x00ff];

		for (offs = 0x400 ; offs < 0x500 ; offs ++)
		{
			argus_change_bg_palette( machine, (offs - 0x0400) + 128,
				(argus_paletteram[offs] << 8) | argus_paletteram[offs + 0x0400] );
		}

		argus_bg_purple = argus_paletteram[0x0ff] & 0x0f;
	}
}

WRITE8_HANDLER( valtric_paletteram_w )
{
	int offs;

	argus_paletteram[ offset ] = data;

	if (offset != 0x01fe && offset != 0x01ff)
	{
		if (offset >= 0x0000 && offset <= 0x01ff)
		{
			argus_change_palette( machine, offset >> 1,
				argus_paletteram[offset | 1] | (argus_paletteram[offset & ~1] << 8));
		}
		else if (offset >= 0x0400 && offset <= 0x05ff )
		{
			argus_change_bg_palette( machine, ((offset - 0x0400) >> 1) + 256,
				argus_paletteram[offset | 1] | (argus_paletteram[offset & ~1] << 8));
		}
		else if (offset >= 0x0600 && offset <= 0x07ff )
		{
			argus_change_palette( machine, ((offset - 0x0600) >> 1) + 512,
				argus_paletteram[offset | 1] | (argus_paletteram[offset & ~1] << 8));
		}
	}
	else
	{
		argus_palette_intensity = (argus_paletteram[0x01fe] << 8) | argus_paletteram[0x01ff];

		for (offs = 0x400 ; offs < 0x600 ; offs += 2)
		{
			argus_change_bg_palette( machine, ((offs - 0x0400) >> 1) + 256,
				argus_paletteram[offs | 1] | (argus_paletteram[offs & ~1] << 8));
		}

		argus_bg_purple = argus_paletteram[0x01ff] & 0x0f;
	}
}

WRITE8_HANDLER( butasan_paletteram_w )
{
	argus_paletteram[ offset ] = data;

	if (offset < 0x0200 )							/* BG1 color */
	{
		argus_change_palette( machine, ((offset - 0x0000) >> 1) + 256,
			argus_paletteram[offset | 1] | (argus_paletteram[offset & ~1] << 8));
	}
	else if (offset < 0x0240 )						/* BG0 color */
	{
		argus_change_palette( machine, ((offset - 0x0200) >> 1) + 192,
			argus_paletteram[offset | 1] | (argus_paletteram[offset & ~1] << 8));
	}
	else if (offset >= 0x0400 && offset <= 0x04ff )	/* Sprite color */
	{
		if (offset < 0x0480)			/* 16 colors */
			argus_change_palette( machine, ((offset - 0x0400) >> 1) + 0,
				argus_paletteram[offset | 1] | (argus_paletteram[offset & ~1] << 8));
		else							/* 8  colors */
		{
			argus_change_palette( machine, (offset & 0x70) + ((offset & 0x00f) >> 1) + 64,
				argus_paletteram[offset | 1] | (argus_paletteram[offset & ~1] << 8));
			argus_change_palette( machine, (offset & 0x70) + ((offset & 0x00f) >> 1) + 72,
				argus_paletteram[offset | 1] | (argus_paletteram[offset & ~1] << 8));
		}
	}
	else if (offset >= 0x0600 && offset <= 0x07ff )	/* Text color */
	{
		argus_change_palette( machine, ((offset - 0x0600) >> 1) + 512,
			argus_paletteram[offset | 1] | (argus_paletteram[offset & ~1] << 8));
	}
}

READ8_HANDLER( bombsa_paletteram_r )
{
	if (bombsa_ram_page==1)
	{
		return argus_paletteram[ offset ];
	}
	else if (bombsa_ram_page==0)
	{
		return bomba_otherram[ offset ];
	}
	else
	{
		mame_printf_debug("unknown bombsa_paletteram_r %02x %04x\n",bombsa_ram_page,offset);
		return 0x00;
	}
}

WRITE8_HANDLER( bombsa_paletteram_w )
{
	if (bombsa_ram_page==1)
	{
		argus_paletteram[ offset ] = data;
		bombsa_change_palette( machine, offset>>1, argus_paletteram[offset | 1] | (argus_paletteram[offset & ~1] << 8));
	}
	else if (bombsa_ram_page==0)
	{
		bomba_otherram[offset] = data;

		tilemap_mark_tile_dirty(bombsa_bg_tilemap, offset/2);
	}
	else
	{
		mame_printf_debug("unknown bombsa_paletteram_w %02x %04x %02x\n",bombsa_ram_page,offset, data);
	}
}

READ8_HANDLER( butasan_txbackram_r )
{
	return butasan_txbackram[ offset ];
}

WRITE8_HANDLER( butasan_txbackram_w )
{
	if (butasan_txbackram[ offset ] != data)
	{
		butasan_txbackram[ offset ] = data;
	}
}

READ8_HANDLER( butasan_bg0backram_r )
{
	return butasan_bg0backram[ offset ];
}

WRITE8_HANDLER( butasan_bg0backram_w )
{
	if (butasan_bg0backram[ offset ] != data)
	{
		butasan_bg0backram[ offset ] = data;
	}
}

WRITE8_HANDLER( butasan_bg1_status_w )
{
	if (butasan_bg1_status != data)
	{
		butasan_bg1_status = data;

		tilemap_set_enable(bg1_tilemap, butasan_bg1_status & 0x01);	/* Set enable flag */
		tilemap_mark_all_tiles_dirty( bg1_tilemap );				/* Bank changed */
	}
}


/***************************************************************************
  Screen refresh
***************************************************************************/

static void argus_bg0_scroll_handle( void )
{
	int delta;
	int scrollx;
	int dcolumn;

	/* Deficit between previous and current scroll value */
	scrollx = argus_bg0_scrollx[0] | (argus_bg0_scrollx[1] << 8);
	delta = scrollx - prvscrollx;
	prvscrollx = scrollx;

	if ( delta == 0 )
		return;

	if (delta > 0)
	{
		lowbitscroll += delta % 16;
		dcolumn = delta / 16;

		if (lowbitscroll >= 16)
		{
			dcolumn ++;
			lowbitscroll -= 16;
		}

		if (dcolumn != 0)
		{
			int i, j;
			int col, woffs, roffs;

			col = ( (scrollx / 16) + 16 ) % 32;
			woffs = 32 * 2 * col;
			roffs = ( ( (scrollx / 16) + 16 ) * 8 ) % 0x8000;

			if ( dcolumn >= 18 )
				dcolumn = 18;

			for ( i = 0 ; i < dcolumn ; i ++ )
			{
				for ( j = 0 ; j < 4 ; j ++ )
				{
					argus_write_dummy_rams( woffs, roffs );
					woffs += 16;
					roffs += 2;
				}
				woffs -= 128;
				roffs -= 16;
				if (woffs < 0)
					woffs += 0x800;
				if (roffs < 0)
					roffs += 0x8000;
			}
		}
	}
	else
	{
		lowbitscroll += (delta % 16);
		dcolumn = -(delta / 16);

		if (lowbitscroll <= 0)
		{
			dcolumn ++;
			lowbitscroll += 16;
		}

		if (dcolumn != 0)
		{
			int i, j;
			int col, woffs, roffs;

			col = ( (scrollx / 16) + 31 ) % 32;
			woffs = 32 * 2 * col;
			roffs = ( (scrollx / 16) - 1 ) * 8;
			if (roffs < 0)
				roffs += 0x08000;

			if (dcolumn >= 18)
				dcolumn = 18;

			for ( i = 0 ; i < dcolumn ; i ++ )
			{
				for ( j = 0 ; j < 4 ; j ++ )
				{
					argus_write_dummy_rams( woffs, roffs );
					woffs += 16;
					roffs += 2;
				}
				if (woffs >= 0x800)
					woffs -= 0x800;
				if (roffs >= 0x8000)
					roffs -= 0x8000;
			}
		}
	}

	if (!argus_flipscreen)
		tilemap_set_scrollx(bg0_tilemap, 0, scrollx & 0x1ff);
	else
		tilemap_set_scrollx(bg0_tilemap, 0, (scrollx + 256) & 0x1ff);

}

static void argus_draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int priority)
{
	int offs;

	/* Draw the sprites */
	for (offs = 11 ; offs < spriteram_size ; offs += 16)
	{
		if ( !(spriteram[offs+4] == 0 && spriteram[offs] == 0xf0) )
		{
			int sx, sy, tile, flipx, flipy, color, pri;

			sx = spriteram[offs + 1];
			sy = spriteram[offs];

			if (argus_flipscreen)
			{
				sx = 240 - sx;
				sy = 240 - sy;
			}

			if (!argus_flipscreen)
			{
				if (  spriteram[offs+2] & 0x01)  sx -= 256;
				if (!(spriteram[offs+2] & 0x02)) sy -= 256;
			}
			else
			{
				if (  spriteram[offs+2] & 0x01)  sx += 256;
				if (!(spriteram[offs+2] & 0x02)) sy += 256;
			}

			tile	 = spriteram[offs+3] + ((spriteram[offs+2] & 0xc0) << 2);
			flipx	 = spriteram[offs+2] & 0x10;
			flipy	 = spriteram[offs+2] & 0x20;
			color	 = spriteram[offs+4] & 0x07;
			pri      = (spriteram[offs+4] & 0x08) >> 3;

			if (argus_flipscreen)
			{
				flipx ^= 0x10;
				flipy ^= 0x20;
			}

			if (priority != pri)
				jal_blend_drawgfx(machine,
							bitmap,machine->gfx[0],
							tile,
							color,
							flipx, flipy,
							sx, sy,
							cliprect,
							TRANSPARENCY_PEN, 15
				);
		}
	}
}

static void valtric_draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	int offs;

	/* Draw the sprites */
	for (offs = 11 ; offs < spriteram_size ; offs += 16)
	{
		if ( !(spriteram[offs+4] == 0 && spriteram[offs] == 0xf0) )
		{
			int sx, sy, tile, flipx, flipy, color;

			sx = spriteram[offs + 1];
			sy = spriteram[offs];

			if (argus_flipscreen)
			{
				sx = 240 - sx;
				sy = 240 - sy;
			}

			if (!argus_flipscreen)
			{
				if (  spriteram[offs+2] & 0x01)  sx -= 256;
				if (!(spriteram[offs+2] & 0x02)) sy -= 256;
			}
			else
			{
				if (  spriteram[offs+2] & 0x01)  sx += 256;
				if (!(spriteram[offs+2] & 0x02)) sy += 256;
			}

			tile	 = spriteram[offs+3] + ((spriteram[offs+2] & 0xc0) << 2);
			flipx	 = spriteram[offs+2] & 0x10;
			flipy	 = spriteram[offs+2] & 0x20;
			color	 = spriteram[offs+4] & 0x0f;

			if (argus_flipscreen)
			{
				flipx ^= 0x10;
				flipy ^= 0x20;
			}

			jal_blend_drawgfx(machine,
						bitmap,machine->gfx[0],
						tile,
						color,
						flipx, flipy,
						sx, sy,
						cliprect,
						TRANSPARENCY_PEN, 15);
		}
	}
}

static void butasan_draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	int offs;

	/* Draw the sprites */
	for (offs = 8 ; offs < spriteram_size ; offs += 16)
	{
		int sx, sy, tile, flipx, flipy, color;

		sx = spriteram[offs + 2];
		sy = 240 - spriteram[offs + 4];

		if (spriteram[offs + 3] & 0x01) sx -= 256;
		if (spriteram[offs + 5] & 0x01) sy += 256;

		tile	 = spriteram[offs + 6] + ((spriteram[offs + 7] & 0x0f) << 8);
		flipx	 = spriteram[offs + 0] & 0x01;
		flipy	 = spriteram[offs + 0] & 0x04;
		color	 = spriteram[offs + 1] & 0x0f;

		if (!argus_flipscreen)
		{
			if ( (offs >= 0x100 && offs < 0x300) || (offs >= 0x400 && offs < 0x580) )
			{
				jal_blend_drawgfx(machine,
							bitmap,machine->gfx[0],
							tile,
							color,
							flipx, flipy,
							sx, sy,
							cliprect,
							TRANSPARENCY_PEN, 7);
			}
			else if ( (offs >= 0x000 && offs < 0x100) || (offs >= 0x300 && offs < 0x400) )
			{
				int i;

				for ( i = 0 ; i <= 1 ; i ++ )
				{
					int td;

					td = (flipx) ? (1 - i) : i;

					jal_blend_drawgfx(machine,
								bitmap,machine->gfx[0],
								tile + td,
								color,
								flipx, flipy,
								sx + i * 16, sy,
								cliprect,
								TRANSPARENCY_PEN, 7);
				}
			}
			else if ( offs >= 0x580 && offs < 0x620 )
			{
				int i, j;

				for ( i = 0 ; i <= 1 ; i ++ )
				{
					for ( j = 0 ; j <= 1 ; j ++ )
					{
						int td;

						if ( !flipy )
							td = (flipx) ? (i * 2) + 1 - j : i * 2 + j;
						else
							td = (flipx) ? ( (1 - i) * 2 ) + 1 - j : (1 - i) * 2 + j;

						jal_blend_drawgfx(machine,
									bitmap,machine->gfx[0],
									tile + td,
									color,
									flipx, flipy,
									sx + j * 16, sy - i * 16,
									cliprect,
									TRANSPARENCY_PEN, 7);
					}
				}
			}
			else if ( offs >= 0x620 && offs < 0x680 )
			{
				int i, j;

				for ( i = 0 ; i <= 3 ; i ++ )
				{
					for ( j = 0 ; j <= 3 ; j ++ )
					{
						int td;

						if ( !flipy )
							td = (flipx) ? (i * 4) + 3 - j : i * 4 + j;
						else
							td = (flipx) ? ( (3 - i) * 4 ) + 3 - j : (3 - i) * 4 + j;

						jal_blend_drawgfx(machine,
									bitmap,machine->gfx[0],
									tile + td,
									color,
									flipx, flipy,
									sx + j * 16, sy - i * 16,
									cliprect,
									TRANSPARENCY_PEN, 7);
					}
				}
			}
		}
		else
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx ^= 0x01;
			flipy ^= 0x04;

			if ( (offs >= 0x100 && offs < 0x300) || (offs >= 0x400 && offs < 0x580) )
			{
				jal_blend_drawgfx(machine,
							bitmap,machine->gfx[0],
							tile,
							color,
							flipx, flipy,
							sx, sy,
							cliprect,
							TRANSPARENCY_PEN, 7);
			}
			else if ( (offs >= 0x000 && offs < 0x100) || (offs >= 0x300 && offs < 0x400) )
			{
				int i;

				for ( i = 0 ; i <= 1 ; i ++ )
				{
					int td;

					td = (flipx) ? i : (1 - i);

					jal_blend_drawgfx(machine,
								bitmap,machine->gfx[0],
								tile + td,
								color,
								flipx, flipy,
								sx - i * 16, sy,
								cliprect,
								TRANSPARENCY_PEN, 7);
				}
			}
			else if ( offs >= 0x580 && offs < 0x620 )
			{
				int i, j;

				for ( i = 0 ; i <= 1 ; i ++ )
				{
					for ( j = 0 ; j <= 1 ; j ++ )
					{
						int td;

						if ( !flipy )
							td = (flipx) ? (1 - i) * 2 + j : ( (1 - i) * 2 ) + 1 - j;
						else
							td = (flipx) ? i * 2 + j : (i * 2) + 1 - j;

						jal_blend_drawgfx(machine,
									bitmap,machine->gfx[0],
									tile + td,
									color,
									flipx, flipy,
									sx - j * 16, sy + i * 16,
									cliprect,
									TRANSPARENCY_PEN, 7);
					}
				}
			}
			else if ( offs >= 0x620 && offs < 0x680 )
			{
				int i, j;

				for ( i = 0 ; i <= 3 ; i ++ )
				{
					for ( j = 0 ; j <= 3 ; j ++ )
					{
						int td;

						if ( !flipy )
							td = (flipx) ? (3 - i) * 4 + j : ( (3 - i) * 4 ) + 3 - j;
						else
							td = (flipx) ? i * 4 + j : (i * 4) + 3 - j;

						jal_blend_drawgfx(machine,
									bitmap,machine->gfx[0],
									tile + td,
									color,
									flipx, flipy,
									sx - j * 16, sy + i * 16,
									cliprect,
									TRANSPARENCY_PEN, 7);
					}
				}
			}
		}
	}
}


#ifdef MAME_DEBUG
static void butasan_log_vram(void)
{
	int offs;

	if ( input_code_pressed(KEYCODE_M) )
	{
		int i;
		logerror("\nSprite RAM\n");
		logerror("---------------------------------------\n");
		logerror("       +0 +1 +2 +3 +4 +5 +6 +7  +8 +9 +a +b +c +d +e +f\n");
		for (offs = 0 ; offs < spriteram_size ; offs += 16)
		{
			for (i = 0 ; i < 16 ; i ++)
			{
				if (i == 0)
				{
					logerror("%04x : ", offs + 0xf000);
					logerror("%02x ", spriteram[offs]);
				}
				else if (i == 7)
					logerror("%02x  ", spriteram[offs + 7]);
				else if (i == 15)
					logerror("%02x\n", spriteram[offs + 15]);
				else
					logerror("%02x ", spriteram[offs + i]);
			}
		}
		logerror("\nColor RAM\n");
		logerror("---------------------------------------\n");
		logerror("       +0 +1 +2 +3 +4 +5 +6 +7  +8 +9 +a +b +c +d +e +f\n");
		for (offs = 0 ; offs < 0xbf0 ; offs += 16)
		{
			for (i = 0 ; i < 16 ; i ++)
			{
				if (i == 0)
				{
					logerror("%04x : ", offs + 0xc400);
					logerror("%02x ", argus_paletteram[offs]);
				}
				else if (i == 7)
					logerror("%02x  ", argus_paletteram[offs + 7]);
				else if (i == 15)
					logerror("%02x\n", argus_paletteram[offs + 15]);
				else
					logerror("%02x ", argus_paletteram[offs + i]);
			}
		}
	}
}
#endif

static void bombsa_draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	int offs;

	/* Draw the sprites */
	for (offs = 11 ; offs < spriteram_size ; offs += 16)
	{
		if ( !(spriteram[offs+4] == 0 && spriteram[offs] == 0xf0) )
		{
			int sx, sy, tile, flipx, flipy, color, size;

			sx = spriteram[offs + 1];
			sy = spriteram[offs];

			if (spriteram[offs+2] & 0x01)  sx += 256;
			if (spriteram[offs+2] & 0x02)  sy += 256;


			tile	 = spriteram[offs+3] + ((spriteram[offs+2] & 0xc0) << 2);
			flipx	 = spriteram[offs+2] & 0x10;
			flipy	 = spriteram[offs+2] & 0x20;
			color	 = spriteram[offs+4] & 0x0f;
			size     = spriteram[offs+2] & 0x08;

			if(!size)
			{
				drawgfx(bitmap,machine->gfx[0],tile,color,flipx,flipy,sx,sy,cliprect,TRANSPARENCY_PEN,15);
			}
			else
			{
				if (!flipy && !flipx)
				{
					drawgfx(bitmap,machine->gfx[0],tile+0,color,flipx,flipy,sx,sy,cliprect,TRANSPARENCY_PEN,15);
					drawgfx(bitmap,machine->gfx[0],tile+1,color,flipx,flipy,sx,sy+16,cliprect,TRANSPARENCY_PEN,15);
					drawgfx(bitmap,machine->gfx[0],tile+2,color,flipx,flipy,sx+16,sy,cliprect,TRANSPARENCY_PEN,15);
					drawgfx(bitmap,machine->gfx[0],tile+3,color,flipx,flipy,sx+16,sy+16,cliprect,TRANSPARENCY_PEN,15);
				}
				else if (!flipy && flipx)
				{
					drawgfx(bitmap,machine->gfx[0],tile+0,color,flipx,flipy,sx+16,sy,cliprect,TRANSPARENCY_PEN,15);
					drawgfx(bitmap,machine->gfx[0],tile+1,color,flipx,flipy,sx+16,sy+16,cliprect,TRANSPARENCY_PEN,15);
					drawgfx(bitmap,machine->gfx[0],tile+2,color,flipx,flipy,sx,sy,cliprect,TRANSPARENCY_PEN,15);
					drawgfx(bitmap,machine->gfx[0],tile+3,color,flipx,flipy,sx,sy+16,cliprect,TRANSPARENCY_PEN,15);
				}
				else if (flipy && !flipx)
				{
					drawgfx(bitmap,machine->gfx[0],tile+0,color,flipx,flipy,sx,sy+16,cliprect,TRANSPARENCY_PEN,15);
					drawgfx(bitmap,machine->gfx[0],tile+1,color,flipx,flipy,sx,sy,cliprect,TRANSPARENCY_PEN,15);
					drawgfx(bitmap,machine->gfx[0],tile+2,color,flipx,flipy,sx+16,sy+16,cliprect,TRANSPARENCY_PEN,15);
					drawgfx(bitmap,machine->gfx[0],tile+3,color,flipx,flipy,sx+16,sy,cliprect,TRANSPARENCY_PEN,15);
				}
				else if (flipy && flipx)
				{
					drawgfx(bitmap,machine->gfx[0],tile+0,color,flipx,flipy,sx+16,sy+16,cliprect,TRANSPARENCY_PEN,15);
					drawgfx(bitmap,machine->gfx[0],tile+1,color,flipx,flipy,sx+16,sy,cliprect,TRANSPARENCY_PEN,15);
					drawgfx(bitmap,machine->gfx[0],tile+2,color,flipx,flipy,sx,sy+16,cliprect,TRANSPARENCY_PEN,15);
					drawgfx(bitmap,machine->gfx[0],tile+3,color,flipx,flipy,sx,sy,cliprect,TRANSPARENCY_PEN,15);
				}
			}
		}
	}
}

VIDEO_UPDATE( argus )
{
	/* scroll BG0 and render tile at proper position */
	argus_bg0_scroll_handle();

	fillbitmap(bitmap, screen->machine->pens[0], cliprect);

	tilemap_draw(bitmap, cliprect, bg0_tilemap, 0, 0);
	argus_draw_sprites(screen->machine, bitmap, cliprect, 0);
	tilemap_draw(bitmap, cliprect, bg1_tilemap, 0, 0);
	argus_draw_sprites(screen->machine, bitmap, cliprect, 1);
	tilemap_draw(bitmap, cliprect, tx_tilemap,  0, 0);
	return 0;
}

VIDEO_UPDATE( valtric )
{
	fillbitmap(bitmap, screen->machine->pens[0], cliprect);

	if(valtric_mosaic==0)
		tilemap_draw(bitmap, cliprect, bg1_tilemap, 0, 0);
	else
	{
		tilemap_draw(mosaicbitmap, cliprect, bg1_tilemap, 0, 0);
		{
			int step=valtric_mosaic;
			UINT32 *dest;
			int x,y,xx,yy;
			int width = video_screen_get_width(screen);
			int height = video_screen_get_height(screen);

			if(valtric_mosaic<0)step*=-1;

			for(y=0;y<width+step;y+=step)
				for(x=0;x<height+step;x+=step)
				{
					static int c=0;

					if(y<height && x< width)
						c=*BITMAP_ADDR32(mosaicbitmap, y, x);

					if(valtric_mosaic<0)
						if(y+step-1<height && x+step-1< width)
							c = *BITMAP_ADDR32(mosaicbitmap, y+step-1, x+step-1);

					for(yy=0;yy<step;yy++)
					 for(xx=0;xx<step;xx++)
					 {
							if(xx+x < width && yy+y<height)
							{
					 			dest=BITMAP_ADDR32(bitmap, y+yy, x+xx);
								*dest=c;
							}
					 }
				}
		 }
	}

	valtric_draw_sprites(screen->machine, bitmap, cliprect);
	tilemap_draw(bitmap, cliprect, tx_tilemap,  0, 0);
	return 0;
}

VIDEO_UPDATE( butasan )
{
	fillbitmap(bitmap, screen->machine->pens[0], cliprect);

	tilemap_draw(bitmap, cliprect, bg1_tilemap, 0, 0);
	tilemap_draw(bitmap, cliprect, bg0_tilemap, 0, 0);
	butasan_draw_sprites(screen->machine, bitmap, cliprect);
	tilemap_draw(bitmap, cliprect, tx_tilemap,  0, 0);

#ifdef MAME_DEBUG
	butasan_log_vram();
#endif
	return 0;
}


VIDEO_UPDATE( bombsa )
{
	fillbitmap(bitmap, screen->machine->pens[0], cliprect);

//  tilemap_draw(bitmap, cliprect, bg1_tilemap, 0, 0);
//  valtric_draw_sprites(screen->bitmap, cliprect);
	tilemap_draw(bitmap, cliprect, bombsa_bg_tilemap,  0, 0);

#if 0
	tilemap_draw(bitmap, cliprect, tx_alt_tilemap,  0, 0);
#endif
	tilemap_draw(bitmap, cliprect, tx_tilemap,  0, 0);
	bombsa_draw_sprites(screen->machine, bitmap,cliprect);
	return 0;
}
