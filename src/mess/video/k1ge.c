/******************************************************************************

K1GE/K2GE graphics emulation

The K1GE graphics were used in the Neogeo pocket mono; the K2GE graphics were
used in the Neogeo pocket color.

******************************************************************************/

#include "emu.h"
#include "k1ge.h"

struct k1ge_t
{
	const k1ge_interface *intf;
	screen_device *screen;
	devcb_resolved_write8 vblank_pin_w;
	devcb_resolved_write8 hblank_pin_w;
	UINT8 *vram;
	UINT8 wba_h, wba_v, wsi_h, wsi_v;

	void (*draw)( device_t *device, int line );

	emu_timer *timer;
	emu_timer *hblank_on_timer;
	bitmap_ind16 *bitmap;
};


PALETTE_INIT( k1ge )
{
	int i;

	for ( i = 0; i < 8; i++ )
	{
		int j = ( i << 5 ) | ( i << 2 ) | ( i >> 1 );

		palette_set_color_rgb( machine, 7-i, j, j, j );
	}
}


PALETTE_INIT( k2ge )
{
	int r,g,b;

	for ( b = 0; b < 16; b++ )
	{
		for ( g = 0; g < 16; g++ )
		{
			for ( r = 0; r < 16; r++ )
			{
				palette_set_color_rgb( machine, ( b << 8 ) | ( g << 4 ) | r, ( r << 4 ) | r, ( g << 4 ) | g, ( b << 4 ) | b );
			}
		}
	}
}


INLINE k1ge_t *get_safe_token( device_t *device )
{
	assert( device != NULL );
	assert( device->type() == K1GE || device->type() == K2GE );

	return ( k1ge_t *) downcast<k1ge_device *>(device)->token();
}


READ8_DEVICE_HANDLER( k1ge_r )
{
	k1ge_t  *k1ge = get_safe_token( device );
	UINT8   data = k1ge->vram[offset & 0x7ff];

	switch( offset )
	{
	case 0x008:     /* RAS.H */
		data = k1ge->screen->hpos() >> 2;
		break;
	case 0x009:     /* RAS.V */
		data = k1ge->screen->vpos();
		break;
	}
	return data;
}


WRITE8_DEVICE_HANDLER( k1ge_w )
{
	k1ge_t  *k1ge = get_safe_token( device );

	switch( offset )
	{
	case 0x000:
		if (!k1ge->vblank_pin_w.isnull())
			k1ge->vblank_pin_w(0, ( data & 0x80 ) ? ( ( k1ge->vram[0x010] & 0x40 ) ? 1 : 0 ) : 0 );
		break;
	case 0x030:
		data &= 0x80;
		break;
	case 0x101: case 0x102: case 0x103:
	case 0x105: case 0x106: case 0x107:
	case 0x109: case 0x10a: case 0x10b:
	case 0x10d: case 0x10e: case 0x10f:
	case 0x111: case 0x112: case 0x113:
	case 0x115: case 0x116: case 0x117:
		data &= 0x07;
		break;
	case 0x7e2:
		if ( k1ge->vram[0x7f0] != 0xAA )
			return;
		data &= 0x80;
		break;
	}

	/* Only the lower 4 bits of the palette entry high bytes can be written */
	if ( offset >= 0x0200 && offset < 0x0400 && ( offset & 1 ) )
	{
		data &= 0x0f;
	}

	k1ge->vram[offset & 0x7ff] = data;
}


INLINE void k1ge_draw_scroll_plane( k1ge_t *k1ge, UINT16 *p, UINT16 base, int line, int scroll_x, int scroll_y, int pal_base )
{
	int i;
	int offset_x = ( scroll_x >> 3 ) * 2;
	int px = scroll_x & 0x07;
	UINT16 map_data;
	UINT16 hflip;
	UINT16 pcode;
	UINT16 tile_addr;
	UINT16 tile_data;

	base += ( ( ( ( scroll_y + line ) >> 3 ) * 0x0040 ) & 0x7ff );

	/* setup */
	map_data = k1ge->vram[ base + offset_x  ] | ( k1ge->vram[ base + offset_x + 1 ] << 8 );
	hflip = map_data & 0x8000;
	pcode = pal_base + ( ( map_data & 0x2000 ) ? 4 : 0 );
	tile_addr = 0x2000 + ( ( map_data & 0x1ff ) * 16 );
	if ( map_data & 0x4000 )
		tile_addr += ( 7 - ( ( scroll_y + line ) & 0x07 ) ) * 2;
	else
		tile_addr += ( ( scroll_y + line ) & 0x07 ) * 2;
	tile_data = k1ge->vram[ tile_addr ] | ( k1ge->vram[ tile_addr + 1 ] << 8 );
	if ( hflip )
		tile_data >>= 2 * ( scroll_x & 0x07 );
	else
		tile_data <<= 2 * ( scroll_x & 0x07 );

	/* draw pixels */
	for ( i = 0; i < 160; i++ )
	{
		UINT16 col;

		if ( hflip )
		{
			col = tile_data & 0x0003;
			tile_data >>= 2;
		}
		else
		{
			col = tile_data >> 14;
			tile_data <<= 2;
		}

		if ( col )
		{
			p[ i ] = k1ge->vram[ pcode + col ];
		}

		px++;
		if ( px >= 8 )
		{
			offset_x = ( offset_x + 2 ) & 0x3f;
			map_data = k1ge->vram[ base + offset_x ] | ( k1ge->vram[ base + offset_x + 1 ] << 8 );
			hflip = map_data & 0x8000;
			pcode = pal_base + ( ( map_data & 0x2000 ) ? 4 : 0 );
			tile_addr = 0x2000 + ( ( map_data & 0x1ff ) * 16 );
			if ( map_data & 0x4000 )
				tile_addr += ( 7 - ( ( scroll_y + line ) & 0x07 ) ) * 2;
			else
				tile_addr += ( ( scroll_y + line ) & 0x07 ) * 2;
			tile_data = k1ge->vram[ tile_addr ] | ( k1ge->vram[ tile_addr + 1 ] << 8 );
			px = 0;
		}
	}
}


INLINE void k1ge_draw_sprite_plane( k1ge_t *k1ge, UINT16 *p, UINT16 priority, int line, int scroll_x, int scroll_y )
{
	struct {
		UINT16 spr_data;
		UINT8 x;
		UINT8 y;
	} spr[64];
	int num_sprites = 0;
	UINT8 spr_y = 0;
	UINT8 spr_x = 0;
	int i;

	priority <<= 11;

	/* Select sprites */
	for ( i = 0; i < 256; i += 4 )
	{
		UINT16 spr_data = k1ge->vram[ 0x800 + i ] | ( k1ge->vram[ 0x801 + i ] << 8 );
		UINT8 x = k1ge->vram[ 0x802 + i ];
		UINT8 y = k1ge->vram[ 0x803 + i ];

		spr_x = ( spr_data & 0x0400 ) ? ( spr_x + x ) :  ( scroll_x + x );
		spr_y = ( spr_data & 0x0200 ) ? ( spr_y + y ) :  ( scroll_y + y );

		if ( ( spr_data & 0x1800 ) == priority )
		{
			if ( ( line >= spr_y || spr_y > 0xf8 ) && line < ( ( spr_y + 8 ) & 0xff ) )
			{
				spr[num_sprites].spr_data = spr_data;
				spr[num_sprites].y = spr_y;
				spr[num_sprites].x = spr_x;
				num_sprites++;
			}
		}
	}

	/* Draw sprites */
	for ( i = num_sprites-1; i >= 0; i-- )
	{
		int j;
		UINT16 tile_addr;
		UINT16 tile_data;
		UINT16 pcode = 0x100 + ( ( spr[i].spr_data & 0x2000 ) ? 4 : 0 );

		tile_addr = 0x2000 + ( ( spr[i].spr_data & 0x1ff ) * 16 );
		if ( spr[i].spr_data & 0x4000 )
			tile_addr += ( 7 - ( ( line - spr[i].y ) & 0x07 ) ) * 2;
		else
			tile_addr += ( ( line - spr[i].y ) & 0x07 ) * 2;
		tile_data = k1ge->vram[ tile_addr ] | ( k1ge->vram[ tile_addr + 1 ] << 8 );

		for ( j = 0; j < 8; j++ )
		{
			UINT16 col;

			spr_x = spr[i].x + j;

			if ( spr[i].spr_data & 0x8000 )
			{
				col = tile_data & 0x03;
				tile_data >>= 2;
			}
			else
			{
				col = tile_data >> 14;
				tile_data <<= 2;
			}

			if ( spr_x < 160 && col )
			{
				p[ spr_x ] = k1ge->vram[ pcode + col ];
			}
		}
	}
}


static void k1ge_draw( device_t *device, int line )
{
	k1ge_t *k1ge = get_safe_token( device );
	UINT16 *p = &k1ge->bitmap->pix16(line);
	UINT16 oowcol = k1ge->vram[0x012] & 0x07;
	int i;

	if ( line < k1ge->wba_v || line >= k1ge->wba_v + k1ge->wsi_v )
	{
		for( i = 0; i < 160; i++ )
		{
			p[i] = oowcol;
		}
	}
	else
	{
		UINT16 col = ( ( k1ge->vram[0x118] & 0xc0 ) == 0x80 ) ? k1ge->vram[0x118] & 0x07 : 0;

		for ( i = 0; i < 160; i++ )
			p[i] = col;

		if ( k1ge->vram[0x030] & 0x80 )
		{
			/* Draw sprites with 01 priority */
			k1ge_draw_sprite_plane( k1ge, p, 1, line, k1ge->vram[0x020], k1ge->vram[0x021] );

			/* Draw PF1 */
			k1ge_draw_scroll_plane( k1ge, p, 0x1000, line, k1ge->vram[0x032], k1ge->vram[0x033], 0x108 );

			/* Draw sprites with 10 priority */
			k1ge_draw_sprite_plane( k1ge, p, 2, line, k1ge->vram[0x020], k1ge->vram[0x021] );

			/* Draw PF2 */
			k1ge_draw_scroll_plane( k1ge, p, 0x1800, line, k1ge->vram[0x034], k1ge->vram[0x035], 0x110 );

			/* Draw sprites with 11 priority */
			k1ge_draw_sprite_plane( k1ge, p, 3, line, k1ge->vram[0x020], k1ge->vram[0x021] );
		}
		else
		{
			/* Draw sprites with 01 priority */
			k1ge_draw_sprite_plane( k1ge, p, 1, line, k1ge->vram[0x020], k1ge->vram[0x021] );

			/* Draw PF2 */
			k1ge_draw_scroll_plane( k1ge, p, 0x1800, line, k1ge->vram[0x034], k1ge->vram[0x035], 0x110 );

			/* Draw sprites with 10 priority */
			k1ge_draw_sprite_plane( k1ge, p, 2, line, k1ge->vram[0x020], k1ge->vram[0x021] );

			/* Draw PF1 */
			k1ge_draw_scroll_plane( k1ge, p, 0x1000, line, k1ge->vram[0x032], k1ge->vram[0x033], 0x108 );

			/* Draw sprites with 11 priority */
			k1ge_draw_sprite_plane( k1ge, p, 3, line, k1ge->vram[0x020], k1ge->vram[0x021] );
		}

		for( i = 0; i < k1ge->wba_h; i++ )
		{
			p[i] = oowcol;
		}

		for( i = k1ge->wba_h + k1ge->wsi_h; i < 160; i++ )
		{
			p[i] = oowcol;
		}
	}
}


INLINE void k2ge_draw_scroll_plane( k1ge_t *k1ge, UINT16 *p, UINT16 base, int line, int scroll_x, int scroll_y, UINT16 pal_base )
{
	int i;
	int offset_x = ( scroll_x >> 3 ) * 2;
	int px = scroll_x & 0x07;
	UINT16 map_data;
	UINT16 hflip;
	UINT16 pcode;
	UINT16 tile_addr;
	UINT16 tile_data;

	base += ( ( ( ( scroll_y + line ) >> 3 ) * 0x0040 ) & 0x7ff );

	/* setup */
	map_data = k1ge->vram[ base + offset_x  ] | ( k1ge->vram[ base + offset_x + 1 ] << 8 );
	hflip = map_data & 0x8000;
	pcode = pal_base + ( ( map_data & 0x1e00 ) >> 6 );
	tile_addr = 0x2000 + ( ( map_data & 0x1ff ) * 16 );
	if ( map_data & 0x4000 )
		tile_addr += ( 7 - ( ( scroll_y + line ) & 0x07 ) ) * 2;
	else
		tile_addr += ( ( scroll_y + line ) & 0x07 ) * 2;
	tile_data = k1ge->vram[ tile_addr ] | ( k1ge->vram[ tile_addr + 1 ] << 8 );
	if ( hflip )
		tile_data >>= 2 * ( scroll_x & 0x07 );
	else
		tile_data <<= 2 * ( scroll_x & 0x07 );

	/* draw pixels */
	for ( i = 0; i < 160; i++ )
	{
		UINT16 col;

		if ( hflip )
		{
			col = tile_data & 0x0003;
			tile_data >>= 2;
		}
		else
		{
			col = tile_data >> 14;
			tile_data <<= 2;
		}

		if ( col )
		{
			p[ i ]  = k1ge->vram[ pcode + col * 2 ] | ( k1ge->vram[ pcode + col * 2 + 1 ] << 8 );
		}

		px++;
		if ( px >= 8 )
		{
			offset_x = ( offset_x + 2 ) & 0x3f;
			map_data = k1ge->vram[ base + offset_x ] | ( k1ge->vram[ base + offset_x + 1 ] << 8 );
			hflip = map_data & 0x8000;
			pcode = pal_base + ( ( map_data & 0x1e00 ) >> 6 );
			tile_addr = 0x2000 + ( ( map_data & 0x1ff ) * 16 );
			if ( map_data & 0x4000 )
				tile_addr += ( 7 - ( ( scroll_y + line ) & 0x07 ) ) * 2;
			else
				tile_addr += ( ( scroll_y + line ) & 0x07 ) * 2;
			tile_data = k1ge->vram[ tile_addr ] | ( k1ge->vram[ tile_addr + 1 ] << 8 );
			px = 0;
		}
	}
}


INLINE void k2ge_draw_sprite_plane( k1ge_t *k1ge, UINT16 *p, UINT16 priority, int line, int scroll_x, int scroll_y )
{
	struct {
		UINT16 spr_data;
		UINT8 x;
		UINT8 y;
		UINT8 index;
	} spr[64];
	int num_sprites = 0;
	UINT8 spr_y = 0;
	UINT8 spr_x = 0;
	int i;

	priority <<= 11;

	/* Select sprites */
	for ( i = 0; i < 256; i += 4 )
	{
		UINT16 spr_data = k1ge->vram[ 0x800 + i ] | ( k1ge->vram[ 0x801 + i ] << 8 );
		UINT8 x = k1ge->vram[ 0x802 + i ];
		UINT8 y = k1ge->vram[ 0x803 + i ];

		spr_x = ( spr_data & 0x0400 ) ? ( spr_x + x ) :  ( scroll_x + x );
		spr_y = ( spr_data & 0x0200 ) ? ( spr_y + y ) :  ( scroll_y + y );

		if ( ( spr_data & 0x1800 ) == priority )
		{
			if ( ( line >= spr_y || spr_y > 0xf8 ) && line < ( ( spr_y + 8 ) & 0xff ) )
			{
				spr[num_sprites].spr_data = spr_data;
				spr[num_sprites].y = spr_y;
				spr[num_sprites].x = spr_x;
				spr[num_sprites].index = i >> 2;
				num_sprites++;
			}
		}
	}

	/* Draw sprites */
	for ( i = num_sprites-1; i >= 0; i-- )
	{
		int j;
		UINT16 tile_addr;
		UINT16 tile_data;
		UINT16 pcode = 0x0200 + ( ( k1ge->vram[0x0c00 + spr[i].index ] & 0x0f ) << 3 );

		tile_addr = 0x2000 + ( ( spr[i].spr_data & 0x1ff ) * 16 );
		if ( spr[i].spr_data & 0x4000 )
			tile_addr += ( 7 - ( ( line - spr[i].y ) & 0x07 ) ) * 2;
		else
			tile_addr += ( ( line - spr[i].y ) & 0x07 ) * 2;
		tile_data = k1ge->vram[ tile_addr ] | ( k1ge->vram[ tile_addr + 1 ] << 8 );

		for ( j = 0; j < 8; j++ )
		{
			UINT16 col;

			spr_x = spr[i].x + j;

			if ( spr[i].spr_data & 0x8000 )
			{
				col = tile_data & 0x03;
				tile_data >>= 2;
			}
			else
			{
				col = tile_data >> 14;
				tile_data <<= 2;
			}

			if ( spr_x < 160 && col )
			{
				p[ spr_x ] = k1ge->vram[ pcode + col * 2 ] | ( k1ge->vram[ pcode + col * 2 + 1 ] << 8 );
			}
		}
	}
}


INLINE void k2ge_k1ge_draw_scroll_plane( k1ge_t *k1ge, UINT16 *p, UINT16 base, int line, int scroll_x, int scroll_y, UINT16 pal_lut_base, UINT16 k2ge_lut_base )
{
	int i;
	int offset_x = ( scroll_x >> 3 ) * 2;
	int px = scroll_x & 0x07;
	UINT16 map_data;
	UINT16 hflip;
	UINT16 pcode;
	UINT16 tile_addr;
	UINT16 tile_data;

	base += ( ( ( ( scroll_y + line ) >> 3 ) * 0x0040 ) & 0x7ff );

	/* setup */
	map_data = k1ge->vram[ base + offset_x  ] | ( k1ge->vram[ base + offset_x + 1 ] << 8 );
	hflip = map_data & 0x8000;
	pcode = ( map_data & 0x2000 ) ? 1 : 0;
	tile_addr = 0x2000 + ( ( map_data & 0x1ff ) * 16 );
	if ( map_data & 0x4000 )
		tile_addr += ( 7 - ( ( scroll_y + line ) & 0x07 ) ) * 2;
	else
		tile_addr += ( ( scroll_y + line ) & 0x07 ) * 2;
	tile_data = k1ge->vram[ tile_addr ] | ( k1ge->vram[ tile_addr + 1 ] << 8 );
	if ( hflip )
		tile_data >>= 2 * ( scroll_x & 0x07 );
	else
		tile_data <<= 2 * ( scroll_x & 0x07 );

	/* draw pixels */
	for ( i = 0; i < 160; i++ )
	{
		UINT16 col;

		if ( hflip )
		{
			col = tile_data & 0x0003;
			tile_data >>= 2;
		}
		else
		{
			col = tile_data >> 14;
			tile_data <<= 2;
		}

		if ( col )
		{
			UINT16 col2 = 16 * pcode + ( k1ge->vram[ pal_lut_base + 4 * pcode + col ] * 2 );
			p[ i ]  = k1ge->vram[ k2ge_lut_base + col2 ] | ( k1ge->vram[ k2ge_lut_base + col2 + 1 ] << 8 );
		}

		px++;
		if ( px >= 8 )
		{
			offset_x = ( offset_x + 2 ) & 0x3f;
			map_data = k1ge->vram[ base + offset_x ] | ( k1ge->vram[ base + offset_x + 1 ] << 8 );
			hflip = map_data & 0x8000;
			pcode = ( map_data & 0x2000 ) ? 1 : 0;
			tile_addr = 0x2000 + ( ( map_data & 0x1ff ) * 16 );
			if ( map_data & 0x4000 )
				tile_addr += ( 7 - ( ( scroll_y + line ) & 0x07 ) ) * 2;
			else
				tile_addr += ( ( scroll_y + line ) & 0x07 ) * 2;
			tile_data = k1ge->vram[ tile_addr ] | ( k1ge->vram[ tile_addr + 1 ] << 8 );
			px = 0;
		}
	}
}


INLINE void k2ge_k1ge_draw_sprite_plane( k1ge_t *k1ge, UINT16 *p, UINT16 priority, int line, int scroll_x, int scroll_y )
{
	struct {
		UINT16 spr_data;
		UINT8 x;
		UINT8 y;
	} spr[64];
	int num_sprites = 0;
	UINT8 spr_y = 0;
	UINT8 spr_x = 0;
	int i;

	priority <<= 11;

	/* Select sprites */
	for ( i = 0; i < 256; i += 4 )
	{
		UINT16 spr_data = k1ge->vram[ 0x800 + i ] | ( k1ge->vram[ 0x801 + i ] << 8 );
		UINT8 x = k1ge->vram[ 0x802 + i ];
		UINT8 y = k1ge->vram[ 0x803 + i ];

		spr_x = ( spr_data & 0x0400 ) ? ( spr_x + x ) :  ( scroll_x + x );
		spr_y = ( spr_data & 0x0200 ) ? ( spr_y + y ) :  ( scroll_y + y );

		if ( ( spr_data & 0x1800 ) == priority )
		{
			if ( ( line >= spr_y || spr_y > 0xf8 ) && line < ( ( spr_y + 8 ) & 0xff ) )
			{
				spr[num_sprites].spr_data = spr_data;
				spr[num_sprites].y = spr_y;
				spr[num_sprites].x = spr_x;
				num_sprites++;
			}
		}
	}

	/* Draw sprites */
	for ( i = num_sprites-1; i >= 0; i-- )
	{
		int j;
		UINT16 tile_addr;
		UINT16 tile_data;
		UINT16 pcode = ( spr[i].spr_data & 0x2000 ) ? 1 : 0;

		tile_addr = 0x2000 + ( ( spr[i].spr_data & 0x1ff ) * 16 );
		if ( spr[i].spr_data & 0x4000 )
			tile_addr += ( 7 - ( ( line - spr[i].y ) & 0x07 ) ) * 2;
		else
			tile_addr += ( ( line - spr[i].y ) & 0x07 ) * 2;
		tile_data = k1ge->vram[ tile_addr ] | ( k1ge->vram[ tile_addr + 1 ] << 8 );

		for ( j = 0; j < 8; j++ )
		{
			UINT16 col;

			spr_x = spr[i].x + j;

			if ( spr[i].spr_data & 0x8000 )
			{
				col = tile_data & 0x03;
				tile_data >>= 2;
			}
			else
			{
				col = tile_data >> 14;
				tile_data <<= 2;
			}

			if ( spr_x < 160 && col )
			{
				UINT16 col2 = 16 * pcode + k1ge->vram[ 0x100 + 4 * pcode + col ] * 2;
				p[ spr_x ] = k1ge->vram[ 0x380 + col2 ] | ( k1ge->vram[ 0x381 + col2 ] << 8 );
			}
		}
	}
}


static void k2ge_draw( device_t *device, int line )
{
	k1ge_t *k1ge = get_safe_token( device );
	UINT16 *p = &k1ge->bitmap->pix16(line);
	UINT16 col = 0;
	UINT16 oowcol;
	int i;

	oowcol = ( k1ge->vram[0x012] & 0x07 ) * 2;
	oowcol = k1ge->vram[0x3f0 + oowcol ] | ( k1ge->vram[0x3f1 + oowcol ] << 8 );

	if ( line < k1ge->wba_v || line >= k1ge->wba_v + k1ge->wsi_v )
	{
		for( i = 0; i < 160; i++ )
		{
			p[i] = oowcol;
		}
	}
	else
	{
		/* Determine the background color */
		if ( ( k1ge->vram[0x118] & 0xc0 ) == 0x80 )
		{
			col = ( k1ge->vram[0x118] & 0x07 ) * 2;
		}
		col = k1ge->vram[0x3e0 + col ] | ( k1ge->vram[0x3e1 + col ] << 8 );

		/* Set the bacground color */
		for ( i = 0; i < 160; i++ )
		{
			p[i] = col;
		}

		if ( k1ge->vram[0x7e2] & 0x80 )
		{
			/* K1GE compatibility mode */
			if ( k1ge->vram[0x030] & 0x80 )
			{
				/* Draw sprites with 01 priority */
				k2ge_k1ge_draw_sprite_plane( k1ge, p, 1, line, k1ge->vram[0x020], k1ge->vram[0x021] );

				/* Draw PF1 */
				k2ge_k1ge_draw_scroll_plane( k1ge, p, 0x1000, line, k1ge->vram[0x032], k1ge->vram[0x033], 0x108, 0x3a0 );

				/* Draw sprites with 10 priority */
				k2ge_k1ge_draw_sprite_plane( k1ge, p, 2, line, k1ge->vram[0x020], k1ge->vram[0x021] );

				/* Draw PF2 */
				k2ge_k1ge_draw_scroll_plane( k1ge, p, 0x1800, line, k1ge->vram[0x034], k1ge->vram[0x035], 0x110, 0x3c0 );

				/* Draw sprites with 11 priority */
				k2ge_k1ge_draw_sprite_plane( k1ge, p, 3, line, k1ge->vram[0x020], k1ge->vram[0x021] );
			}
			else
			{
				/* Draw sprites with 01 priority */
				k2ge_k1ge_draw_sprite_plane( k1ge, p, 1, line, k1ge->vram[0x020], k1ge->vram[0x021] );

				/* Draw PF2 */
				k2ge_k1ge_draw_scroll_plane( k1ge, p, 0x1800, line, k1ge->vram[0x034], k1ge->vram[0x035], 0x110, 0x3c0 );

				/* Draw sprites with 10 priority */
				k2ge_k1ge_draw_sprite_plane( k1ge, p, 2, line, k1ge->vram[0x020], k1ge->vram[0x021] );

				/* Draw PF1 */
				k2ge_k1ge_draw_scroll_plane( k1ge, p, 0x1000, line, k1ge->vram[0x032], k1ge->vram[0x033], 0x108, 0x3a0 );

				/* Draw sprites with 11 priority */
				k2ge_k1ge_draw_sprite_plane( k1ge, p, 3, line, k1ge->vram[0x020], k1ge->vram[0x021] );
			}
		}
		else
		{
			/* K2GE mode */
			if ( k1ge->vram[0x030] & 0x80 )
			{
				/* Draw sprites with 01 priority */
				k2ge_draw_sprite_plane( k1ge, p, 1, line, k1ge->vram[0x020], k1ge->vram[0x021] );

				/* Draw PF1 */
				k2ge_draw_scroll_plane( k1ge, p, 0x1000, line, k1ge->vram[0x032], k1ge->vram[0x033], 0x280 );

				/* Draw sprites with 10 priority */
				k2ge_draw_sprite_plane( k1ge, p, 2, line, k1ge->vram[0x020], k1ge->vram[0x021] );

				/* Draw PF2 */
				k2ge_draw_scroll_plane( k1ge, p, 0x1800, line, k1ge->vram[0x034], k1ge->vram[0x035], 0x300 );

				/* Draw sprites with 11 priority */
				k2ge_draw_sprite_plane( k1ge, p, 3, line, k1ge->vram[0x020], k1ge->vram[0x021] );
			}
			else
			{
				/* Draw sprites with 01 priority */
				k2ge_draw_sprite_plane( k1ge, p, 1, line, k1ge->vram[0x020], k1ge->vram[0x021] );

				/* Draw PF2 */
				k2ge_draw_scroll_plane( k1ge, p, 0x1800, line, k1ge->vram[0x034], k1ge->vram[0x035], 0x300 );

				/* Draw sprites with 10 priority */
				k2ge_draw_sprite_plane( k1ge, p, 2, line, k1ge->vram[0x020], k1ge->vram[0x021] );

				/* Draw PF1 */
				k2ge_draw_scroll_plane( k1ge, p, 0x1000, line, k1ge->vram[0x032], k1ge->vram[0x033], 0x280 );

				/* Draw sprites with 11 priority */
				k2ge_draw_sprite_plane( k1ge, p, 3, line, k1ge->vram[0x020], k1ge->vram[0x021] );
			}
		}

		for ( i = 0; i < k1ge->wba_h; i++ )
		{
			p[i] = oowcol;
		}

		for ( i = k1ge->wba_h + k1ge->wsi_h; i < 160; i++ )
		{
			p[i] = oowcol;
		}
	}
}


static TIMER_CALLBACK( k1ge_hblank_on_timer_callback )
{
	device_t *device = (device_t *)ptr;
	k1ge_t *k1ge = get_safe_token( device );

	if (!k1ge->hblank_pin_w.isnull())
		k1ge->hblank_pin_w(0, 0);
}


static TIMER_CALLBACK( k1ge_timer_callback )
{
	device_t *device = (device_t *)ptr;
	k1ge_t *k1ge = get_safe_token( device );
	int y = k1ge->screen->vpos();

	/* Check for start of VBlank */
	if ( y >= 152 )
	{
		k1ge->vram[0x010] |= 0x40;
		if ((k1ge->vram[0x000] & 0x80 ) && !k1ge->vblank_pin_w.isnull())
				k1ge->vblank_pin_w(0, 1);
	}

	/* Check for end of VBlank */
	if ( y == 0 )
	{
		k1ge->wba_h = k1ge->vram[0x002];
		k1ge->wba_v = k1ge->vram[0x003];
		k1ge->wsi_h = k1ge->vram[0x004];
		k1ge->wsi_v = k1ge->vram[0x005];
		k1ge->vram[0x010] &= ~ 0x40;
		if ((k1ge->vram[0x000] & 0x80 ) && !k1ge->vblank_pin_w.isnull())
			k1ge->vblank_pin_w(0, 0);
	}

	/* Check if Hint should be triggered */
	if ( y == K1GE_SCREEN_HEIGHT - 1 || y < 151 )
	{
		if (!k1ge->hblank_pin_w.isnull())
		{
			if ( k1ge->vram[0x000] & 0x40 )
				k1ge->hblank_pin_w(0, 1);
			k1ge->hblank_on_timer->adjust( k1ge->screen->time_until_pos(y, 480 ) );
		}
	}

	/* Draw a line when inside visible area */
	if ( y && y < 153 )
	{
		k1ge->draw( device, y - 1 );
	}

	k1ge->timer->adjust( k1ge->screen->time_until_pos(( y + 1 ) % K1GE_SCREEN_HEIGHT, 0 ) );
}


void k1ge_update( device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	k1ge_t *k1ge = get_safe_token( device );

	copybitmap( bitmap, *k1ge->bitmap, 0, 0, 0, 0, cliprect );
}


static DEVICE_START( k1ge )
{
	k1ge_t *k1ge = get_safe_token( device );

	k1ge->intf = (const k1ge_interface*)device->static_config();

	k1ge->vblank_pin_w.resolve(k1ge->intf->vblank_pin_w, *device);
	k1ge->hblank_pin_w.resolve(k1ge->intf->hblank_pin_w, *device);

	k1ge->timer = device->machine().scheduler().timer_alloc(FUNC(k1ge_timer_callback), (void *) device );
	k1ge->hblank_on_timer = device->machine().scheduler().timer_alloc(FUNC(k1ge_hblank_on_timer_callback), (void *) device );
	k1ge->screen = device->machine().device<screen_device>(k1ge->intf->screen_tag);
	k1ge->vram = device->machine().root_device().memregion( k1ge->intf->vram_tag )->base();
	k1ge->bitmap = auto_bitmap_ind16_alloc( device->machine(), k1ge->screen->width(), k1ge->screen->height() );
	k1ge->draw = k1ge_draw;
}


static DEVICE_START( k2ge )
{
	k1ge_t *k1ge = get_safe_token( device );

	DEVICE_START_CALL( k1ge );
	k1ge->draw = k2ge_draw;
}


static DEVICE_RESET( k1ge )
{
	k1ge_t *k1ge = get_safe_token( device );

	k1ge->vram[0x000] = 0x00;   /* Interrupt enable */
	k1ge->vram[0x002] = 0x00;   /* WBA.H */
	k1ge->vram[0x003] = 0x00;   /* WVA.V */
	k1ge->vram[0x004] = 0xFF;   /* WSI.H */
	k1ge->vram[0x005] = 0xFF;   /* WSI.V */
	k1ge->vram[0x007] = 0xc6;   /* REF */
	k1ge->vram[0x012] = 0x00;   /* 2D control */
	k1ge->vram[0x020] = 0x00;   /* PO.H */
	k1ge->vram[0x021] = 0x00;   /* PO.V */
	k1ge->vram[0x030] = 0x00;   /* PF */
	k1ge->vram[0x032] = 0x00;   /* S1SO.H */
	k1ge->vram[0x033] = 0x00;   /* S1SO.V */
	k1ge->vram[0x034] = 0x00;   /* S2SO.H */
	k1ge->vram[0x035] = 0x00;   /* S2SO.V */
	k1ge->vram[0x101] = 0x07;   /* SPPLT01 */
	k1ge->vram[0x102] = 0x07;   /* SPPLT02 */
	k1ge->vram[0x103] = 0x07;   /* SPPLT03 */
	k1ge->vram[0x105] = 0x07;   /* SPPLT11 */
	k1ge->vram[0x106] = 0x07;   /* SPPLT12 */
	k1ge->vram[0x107] = 0x07;   /* SPPLT13 */
	k1ge->vram[0x109] = 0x07;   /* SC1PLT01 */
	k1ge->vram[0x10a] = 0x07;   /* SC1PLT02 */
	k1ge->vram[0x10b] = 0x07;   /* SC1PLT03 */
	k1ge->vram[0x10d] = 0x07;   /* SC1PLT11 */
	k1ge->vram[0x10e] = 0x07;   /* SC1PLT12 */
	k1ge->vram[0x10f] = 0x07;   /* SC1PLT13 */
	k1ge->vram[0x111] = 0x07;   /* SC2PLT01 */
	k1ge->vram[0x112] = 0x07;   /* SC2PLT02 */
	k1ge->vram[0x113] = 0x07;   /* SC2PLT03 */
	k1ge->vram[0x115] = 0x07;   /* SC2PLT11 */
	k1ge->vram[0x116] = 0x07;   /* SC2PLT12 */
	k1ge->vram[0x117] = 0x07;   /* SC2PLT13 */
	k1ge->vram[0x118] = 0x07;   /* BG */
	k1ge->vram[0x400] = 0xFF;   /* LED control */
	k1ge->vram[0x402] = 0x80;   /* LEDFREG */
	k1ge->vram[0x7e0] = 0x52;   /* RESET */
	k1ge->vram[0x7e2] = 0x00;   /* MODE */

	k1ge->timer->adjust( k1ge->screen->time_until_pos(( k1ge->screen->vpos() + 1 ) % K1GE_SCREEN_HEIGHT, 0 ) );
}


const device_type K1GE = &device_creator<k1ge_device>;

k1ge_device::k1ge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, K1GE, "", tag, owner, clock)
{
	m_token = global_alloc_clear(k1ge_t);
}

k1ge_device::k1ge_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, type, name, tag, owner, clock)
{
	m_token = global_alloc_clear(k1ge_t);
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void k1ge_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k1ge_device::device_start()
{
	DEVICE_START_NAME( k1ge )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void k1ge_device::device_reset()
{
	DEVICE_RESET_NAME( k1ge )(this);
}


const device_type K2GE = &device_creator<k2ge_device>;

k2ge_device::k2ge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: k1ge_device(mconfig, K2GE, "", tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k2ge_device::device_start()
{
	DEVICE_START_NAME( k2ge )(this);
}
