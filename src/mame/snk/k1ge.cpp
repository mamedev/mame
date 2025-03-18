// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/******************************************************************************

  K1GE/K2GE graphics emulation

  The K1GE graphics were used in the Neogeo pocket mono; the K2GE graphics were
  used in the Neogeo pocket color.

******************************************************************************/

#include "emu.h"
#include "k1ge.h"
#include "screen.h"


void k1ge_device::palette_init()
{
	for (int i = 0; i < 8; i++)
	{
		const u8 j = pal3bit(i);

		set_pen_color(7 - i, j, j, j);
	}
}


void k2ge_device::palette_init()
{
	for (int b = 0; b < 16; b++)
	{
		for (int g = 0; g < 16; g++)
		{
			for (int r = 0; r < 16; r++)
			{
				set_pen_color((b << 8) | (g << 4) | r, (r << 4) | r, (g << 4) | g, (b << 4) | b);
			}
		}
	}
}


uint8_t k1ge_device::read(offs_t offset)
{
	assert(offset < 0x4000);

	uint8_t data = m_vram[offset];

	switch( offset )
	{
	case 0x008:     /* RAS.H */
		data = screen().hpos() >> 2;
		break;
	case 0x009:     /* RAS.V */
		data = screen().vpos();
		break;
	}
	return data;
}


void k1ge_device::write(offs_t offset, uint8_t data)
{
	assert(offset < 0x4000);

	switch( offset )
	{
	case 0x000:
		m_vblank_pin_w( ( data & 0x80 ) ? ( ( m_vram[0x010] & 0x40 ) ? 1 : 0 ) : 0 );
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
		if ( m_vram[0x7f0] != 0xaa )
			return;
		data &= 0x80;
		break;
	}

	/* Only the lower 4 bits of the palette entry high bytes can be written */
	if ( offset >= 0x0200 && offset < 0x0400 && ( offset & 1 ) )
	{
		data &= 0x0f;
	}

	m_vram[offset] = data;
}


void k1ge_device::draw_scroll_plane( uint16_t *p, uint16_t base, int line, int scroll_x, int scroll_y, int pal_base )
{
	int i;
	int offset_x = ( scroll_x >> 3 ) * 2;
	int px = scroll_x & 0x07;
	uint16_t map_data;
	uint16_t hflip;
	uint16_t pcode;
	uint16_t tile_addr;
	uint16_t tile_data;

	base += ( ( ( ( scroll_y + line ) >> 3 ) * 0x0040 ) & 0x7ff );

	/* setup */
	map_data = m_vram[ base + offset_x  ] | ( m_vram[ base + offset_x + 1 ] << 8 );
	hflip = map_data & 0x8000;
	pcode = pal_base + ( ( map_data & 0x2000 ) ? 4 : 0 );
	tile_addr = 0x2000 + ( ( map_data & 0x1ff ) * 16 );
	if ( map_data & 0x4000 )
		tile_addr += ( 7 - ( ( scroll_y + line ) & 0x07 ) ) * 2;
	else
		tile_addr += ( ( scroll_y + line ) & 0x07 ) * 2;
	tile_data = m_vram[ tile_addr ] | ( m_vram[ tile_addr + 1 ] << 8 );
	if ( hflip )
		tile_data >>= 2 * ( scroll_x & 0x07 );
	else
		tile_data <<= 2 * ( scroll_x & 0x07 );

	/* draw pixels */
	for ( i = 0; i < 160; i++ )
	{
		uint16_t col;

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
			p[ i ] = m_vram[ pcode + col ];
		}

		px++;
		if ( px >= 8 )
		{
			offset_x = ( offset_x + 2 ) & 0x3f;
			map_data = m_vram[ base + offset_x ] | ( m_vram[ base + offset_x + 1 ] << 8 );
			hflip = map_data & 0x8000;
			pcode = pal_base + ( ( map_data & 0x2000 ) ? 4 : 0 );
			tile_addr = 0x2000 + ( ( map_data & 0x1ff ) * 16 );
			if ( map_data & 0x4000 )
				tile_addr += ( 7 - ( ( scroll_y + line ) & 0x07 ) ) * 2;
			else
				tile_addr += ( ( scroll_y + line ) & 0x07 ) * 2;
			tile_data = m_vram[ tile_addr ] | ( m_vram[ tile_addr + 1 ] << 8 );
			px = 0;
		}
	}
}


void k1ge_device::draw_sprite_plane( uint16_t *p, uint16_t priority, int line, int scroll_x, int scroll_y )
{
	struct {
		uint16_t spr_data;
		uint8_t x;
		uint8_t y;
	} spr[64];
	int num_sprites = 0;
	uint8_t spr_y = 0;
	uint8_t spr_x = 0;
	int i;

	priority <<= 11;

	/* Select sprites */
	for ( i = 0; i < 256; i += 4 )
	{
		uint16_t spr_data = m_vram[ 0x800 + i ] | ( m_vram[ 0x801 + i ] << 8 );
		uint8_t x = m_vram[ 0x802 + i ];
		uint8_t y = m_vram[ 0x803 + i ];

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
		uint16_t tile_addr;
		uint16_t tile_data;
		uint16_t pcode = 0x100 + ( ( spr[i].spr_data & 0x2000 ) ? 4 : 0 );

		tile_addr = 0x2000 + ( ( spr[i].spr_data & 0x1ff ) * 16 );
		if ( spr[i].spr_data & 0x4000 )
			tile_addr += ( 7 - ( ( line - spr[i].y ) & 0x07 ) ) * 2;
		else
			tile_addr += ( ( line - spr[i].y ) & 0x07 ) * 2;
		tile_data = m_vram[ tile_addr ] | ( m_vram[ tile_addr + 1 ] << 8 );

		for ( j = 0; j < 8; j++ )
		{
			uint16_t col;

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
				p[ spr_x ] = m_vram[ pcode + col ];
			}
		}
	}
}


void k1ge_device::draw( int line )
{
	uint16_t *const p = &m_bitmap->pix(line);
	uint16_t oowcol = m_vram[0x012] & 0x07;

	if ( line < m_wba_v || line >= m_wba_v + m_wsi_v )
	{
		for( int i = 0; i < 160; i++ )
		{
			p[i] = oowcol;
		}
	}
	else
	{
		uint16_t col = ( ( m_vram[0x118] & 0xc0 ) == 0x80 ) ? m_vram[0x118] & 0x07 : 0;

		for ( int i = 0; i < 160; i++ )
			p[i] = col;

		if ( m_vram[0x030] & 0x80 )
		{
			/* Draw sprites with 01 priority */
			draw_sprite_plane( p, 1, line, m_vram[0x020], m_vram[0x021] );

			/* Draw PF1 */
			draw_scroll_plane( p, 0x1000, line, m_vram[0x032], m_vram[0x033], 0x108 );

			/* Draw sprites with 10 priority */
			draw_sprite_plane( p, 2, line, m_vram[0x020], m_vram[0x021] );

			/* Draw PF2 */
			draw_scroll_plane( p, 0x1800, line, m_vram[0x034], m_vram[0x035], 0x110 );

			/* Draw sprites with 11 priority */
			draw_sprite_plane( p, 3, line, m_vram[0x020], m_vram[0x021] );
		}
		else
		{
			/* Draw sprites with 01 priority */
			draw_sprite_plane( p, 1, line, m_vram[0x020], m_vram[0x021] );

			/* Draw PF2 */
			draw_scroll_plane( p, 0x1800, line, m_vram[0x034], m_vram[0x035], 0x110 );

			/* Draw sprites with 10 priority */
			draw_sprite_plane( p, 2, line, m_vram[0x020], m_vram[0x021] );

			/* Draw PF1 */
			draw_scroll_plane( p, 0x1000, line, m_vram[0x032], m_vram[0x033], 0x108 );

			/* Draw sprites with 11 priority */
			draw_sprite_plane( p, 3, line, m_vram[0x020], m_vram[0x021] );
		}

		for( int i = 0; i < m_wba_h; i++ )
		{
			p[i] = oowcol;
		}

		for( int i = m_wba_h + m_wsi_h; i < 160; i++ )
		{
			p[i] = oowcol;
		}
	}
}


void k2ge_device::draw_scroll_plane( uint16_t *p, uint16_t base, int line, int scroll_x, int scroll_y, uint16_t pal_base )
{
	int i;
	int offset_x = ( scroll_x >> 3 ) * 2;
	int px = scroll_x & 0x07;
	uint16_t map_data;
	uint16_t hflip;
	uint16_t pcode;
	uint16_t tile_addr;
	uint16_t tile_data;

	base += ( ( ( ( scroll_y + line ) >> 3 ) * 0x0040 ) & 0x7ff );

	/* setup */
	map_data = m_vram[ base + offset_x  ] | ( m_vram[ base + offset_x + 1 ] << 8 );
	hflip = map_data & 0x8000;
	pcode = pal_base + ( ( map_data & 0x1e00 ) >> 6 );
	tile_addr = 0x2000 + ( ( map_data & 0x1ff ) * 16 );
	if ( map_data & 0x4000 )
		tile_addr += ( 7 - ( ( scroll_y + line ) & 0x07 ) ) * 2;
	else
		tile_addr += ( ( scroll_y + line ) & 0x07 ) * 2;
	tile_data = m_vram[ tile_addr ] | ( m_vram[ tile_addr + 1 ] << 8 );
	if ( hflip )
		tile_data >>= 2 * ( scroll_x & 0x07 );
	else
		tile_data <<= 2 * ( scroll_x & 0x07 );

	/* draw pixels */
	for ( i = 0; i < 160; i++ )
	{
		uint16_t col;

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
			p[ i ]  = m_vram[ pcode + col * 2 ] | ( m_vram[ pcode + col * 2 + 1 ] << 8 );
		}

		px++;
		if ( px >= 8 )
		{
			offset_x = ( offset_x + 2 ) & 0x3f;
			map_data = m_vram[ base + offset_x ] | ( m_vram[ base + offset_x + 1 ] << 8 );
			hflip = map_data & 0x8000;
			pcode = pal_base + ( ( map_data & 0x1e00 ) >> 6 );
			tile_addr = 0x2000 + ( ( map_data & 0x1ff ) * 16 );
			if ( map_data & 0x4000 )
				tile_addr += ( 7 - ( ( scroll_y + line ) & 0x07 ) ) * 2;
			else
				tile_addr += ( ( scroll_y + line ) & 0x07 ) * 2;
			tile_data = m_vram[ tile_addr ] | ( m_vram[ tile_addr + 1 ] << 8 );
			px = 0;
		}
	}
}


void k2ge_device::draw_sprite_plane( uint16_t *p, uint16_t priority, int line, int scroll_x, int scroll_y )
{
	struct {
		uint16_t spr_data;
		uint8_t x;
		uint8_t y;
		uint8_t index;
	} spr[64];
	int num_sprites = 0;
	uint8_t spr_y = 0;
	uint8_t spr_x = 0;
	int i;

	priority <<= 11;

	/* Select sprites */
	for ( i = 0; i < 256; i += 4 )
	{
		uint16_t spr_data = m_vram[ 0x800 + i ] | ( m_vram[ 0x801 + i ] << 8 );
		uint8_t x = m_vram[ 0x802 + i ];
		uint8_t y = m_vram[ 0x803 + i ];

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
		uint16_t tile_addr;
		uint16_t tile_data;
		uint16_t pcode = 0x0200 + ( ( m_vram[0x0c00 + spr[i].index ] & 0x0f ) << 3 );

		tile_addr = 0x2000 + ( ( spr[i].spr_data & 0x1ff ) * 16 );
		if ( spr[i].spr_data & 0x4000 )
			tile_addr += ( 7 - ( ( line - spr[i].y ) & 0x07 ) ) * 2;
		else
			tile_addr += ( ( line - spr[i].y ) & 0x07 ) * 2;
		tile_data = m_vram[ tile_addr ] | ( m_vram[ tile_addr + 1 ] << 8 );

		for ( j = 0; j < 8; j++ )
		{
			uint16_t col;

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
				p[ spr_x ] = m_vram[ pcode + col * 2 ] | ( m_vram[ pcode + col * 2 + 1 ] << 8 );
			}
		}
	}
}


void k2ge_device::k1ge_draw_scroll_plane( uint16_t *p, uint16_t base, int line, int scroll_x, int scroll_y, uint16_t pal_lut_base, uint16_t k2ge_lut_base )
{
	int i;
	int offset_x = ( scroll_x >> 3 ) * 2;
	int px = scroll_x & 0x07;
	uint16_t map_data;
	uint16_t hflip;
	uint16_t pcode;
	uint16_t tile_addr;
	uint16_t tile_data;

	base += ( ( ( ( scroll_y + line ) >> 3 ) * 0x0040 ) & 0x7ff );

	/* setup */
	map_data = m_vram[ base + offset_x  ] | ( m_vram[ base + offset_x + 1 ] << 8 );
	hflip = map_data & 0x8000;
	pcode = ( map_data & 0x2000 ) ? 1 : 0;
	tile_addr = 0x2000 + ( ( map_data & 0x1ff ) * 16 );
	if ( map_data & 0x4000 )
		tile_addr += ( 7 - ( ( scroll_y + line ) & 0x07 ) ) * 2;
	else
		tile_addr += ( ( scroll_y + line ) & 0x07 ) * 2;
	tile_data = m_vram[ tile_addr ] | ( m_vram[ tile_addr + 1 ] << 8 );
	if ( hflip )
		tile_data >>= 2 * ( scroll_x & 0x07 );
	else
		tile_data <<= 2 * ( scroll_x & 0x07 );

	/* draw pixels */
	for ( i = 0; i < 160; i++ )
	{
		uint16_t col;

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
			uint16_t col2 = 16 * pcode + ( m_vram[ pal_lut_base + 4 * pcode + col ] * 2 );
			p[ i ]  = m_vram[ k2ge_lut_base + col2 ] | ( m_vram[ k2ge_lut_base + col2 + 1 ] << 8 );
		}

		px++;
		if ( px >= 8 )
		{
			offset_x = ( offset_x + 2 ) & 0x3f;
			map_data = m_vram[ base + offset_x ] | ( m_vram[ base + offset_x + 1 ] << 8 );
			hflip = map_data & 0x8000;
			pcode = ( map_data & 0x2000 ) ? 1 : 0;
			tile_addr = 0x2000 + ( ( map_data & 0x1ff ) * 16 );
			if ( map_data & 0x4000 )
				tile_addr += ( 7 - ( ( scroll_y + line ) & 0x07 ) ) * 2;
			else
				tile_addr += ( ( scroll_y + line ) & 0x07 ) * 2;
			tile_data = m_vram[ tile_addr ] | ( m_vram[ tile_addr + 1 ] << 8 );
			px = 0;
		}
	}
}


void k2ge_device::k1ge_draw_sprite_plane( uint16_t *p, uint16_t priority, int line, int scroll_x, int scroll_y )
{
	struct {
		uint16_t spr_data;
		uint8_t x;
		uint8_t y;
	} spr[64];
	int num_sprites = 0;
	uint8_t spr_y = 0;
	uint8_t spr_x = 0;
	int i;

	priority <<= 11;

	/* Select sprites */
	for ( i = 0; i < 256; i += 4 )
	{
		uint16_t spr_data = m_vram[ 0x800 + i ] | ( m_vram[ 0x801 + i ] << 8 );
		uint8_t x = m_vram[ 0x802 + i ];
		uint8_t y = m_vram[ 0x803 + i ];

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
		uint16_t tile_addr;
		uint16_t tile_data;
		uint16_t pcode = ( spr[i].spr_data & 0x2000 ) ? 1 : 0;

		tile_addr = 0x2000 + ( ( spr[i].spr_data & 0x1ff ) * 16 );
		if ( spr[i].spr_data & 0x4000 )
			tile_addr += ( 7 - ( ( line - spr[i].y ) & 0x07 ) ) * 2;
		else
			tile_addr += ( ( line - spr[i].y ) & 0x07 ) * 2;
		tile_data = m_vram[ tile_addr ] | ( m_vram[ tile_addr + 1 ] << 8 );

		for ( j = 0; j < 8; j++ )
		{
			uint16_t col;

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
				uint16_t col2 = 16 * pcode + m_vram[ 0x100 + 4 * pcode + col ] * 2;
				p[ spr_x ] = m_vram[ 0x380 + col2 ] | ( m_vram[ 0x381 + col2 ] << 8 );
			}
		}
	}
}


void k2ge_device::draw( int line )
{
	uint16_t *const p = &m_bitmap->pix(line);
	uint16_t col = 0;
	uint16_t oowcol;

	oowcol = ( m_vram[0x012] & 0x07 ) * 2;
	oowcol = m_vram[0x3f0 + oowcol ] | ( m_vram[0x3f1 + oowcol ] << 8 );

	if ( line < m_wba_v || line >= m_wba_v + m_wsi_v )
	{
		for( int i = 0; i < 160; i++ )
		{
			p[i] = oowcol;
		}
	}
	else
	{
		/* Determine the background color */
		if ( ( m_vram[0x118] & 0xc0 ) == 0x80 )
		{
			col = ( m_vram[0x118] & 0x07 ) * 2;
		}
		col = m_vram[0x3e0 + col ] | ( m_vram[0x3e1 + col ] << 8 );

		/* Set the bacground color */
		for ( int i = 0; i < 160; i++ )
		{
			p[i] = col;
		}

		if ( m_vram[0x7e2] & 0x80 )
		{
			/* K1GE compatibility mode */
			if ( m_vram[0x030] & 0x80 )
			{
				/* Draw sprites with 01 priority */
				k1ge_draw_sprite_plane( p, 1, line, m_vram[0x020], m_vram[0x021] );

				/* Draw PF1 */
				k1ge_draw_scroll_plane( p, 0x1000, line, m_vram[0x032], m_vram[0x033], 0x108, 0x3a0 );

				/* Draw sprites with 10 priority */
				k1ge_draw_sprite_plane( p, 2, line, m_vram[0x020], m_vram[0x021] );

				/* Draw PF2 */
				k1ge_draw_scroll_plane( p, 0x1800, line, m_vram[0x034], m_vram[0x035], 0x110, 0x3c0 );

				/* Draw sprites with 11 priority */
				k1ge_draw_sprite_plane( p, 3, line, m_vram[0x020], m_vram[0x021] );
			}
			else
			{
				/* Draw sprites with 01 priority */
				k1ge_draw_sprite_plane( p, 1, line, m_vram[0x020], m_vram[0x021] );

				/* Draw PF2 */
				k1ge_draw_scroll_plane( p, 0x1800, line, m_vram[0x034], m_vram[0x035], 0x110, 0x3c0 );

				/* Draw sprites with 10 priority */
				k1ge_draw_sprite_plane( p, 2, line, m_vram[0x020], m_vram[0x021] );

				/* Draw PF1 */
				k1ge_draw_scroll_plane( p, 0x1000, line, m_vram[0x032], m_vram[0x033], 0x108, 0x3a0 );

				/* Draw sprites with 11 priority */
				k1ge_draw_sprite_plane( p, 3, line, m_vram[0x020], m_vram[0x021] );
			}
		}
		else
		{
			/* K2GE mode */
			if ( m_vram[0x030] & 0x80 )
			{
				/* Draw sprites with 01 priority */
				draw_sprite_plane( p, 1, line, m_vram[0x020], m_vram[0x021] );

				/* Draw PF1 */
				draw_scroll_plane( p, 0x1000, line, m_vram[0x032], m_vram[0x033], 0x280 );

				/* Draw sprites with 10 priority */
				draw_sprite_plane( p, 2, line, m_vram[0x020], m_vram[0x021] );

				/* Draw PF2 */
				draw_scroll_plane( p, 0x1800, line, m_vram[0x034], m_vram[0x035], 0x300 );

				/* Draw sprites with 11 priority */
				draw_sprite_plane( p, 3, line, m_vram[0x020], m_vram[0x021] );
			}
			else
			{
				/* Draw sprites with 01 priority */
				draw_sprite_plane( p, 1, line, m_vram[0x020], m_vram[0x021] );

				/* Draw PF2 */
				draw_scroll_plane( p, 0x1800, line, m_vram[0x034], m_vram[0x035], 0x300 );

				/* Draw sprites with 10 priority */
				draw_sprite_plane( p, 2, line, m_vram[0x020], m_vram[0x021] );

				/* Draw PF1 */
				draw_scroll_plane( p, 0x1000, line, m_vram[0x032], m_vram[0x033], 0x280 );

				/* Draw sprites with 11 priority */
				draw_sprite_plane( p, 3, line, m_vram[0x020], m_vram[0x021] );
			}
		}

		for ( int i = 0; i < m_wba_h; i++ )
		{
			p[i] = oowcol;
		}

		for ( int i = m_wba_h + m_wsi_h; i < 160; i++ )
		{
			p[i] = oowcol;
		}
	}
}


TIMER_CALLBACK_MEMBER( k1ge_device::hblank_on_timer_callback )
{
	m_hblank_pin_w(0);
}


TIMER_CALLBACK_MEMBER( k1ge_device::timer_callback )
{
	int y = screen().vpos();

	/* Check for start of VBlank */
	if ( y >= 152 )
	{
		m_vram[0x010] |= 0x40;
		if (m_vram[0x000] & 0x80)
		{
			m_vblank_pin_w(1);
		}
	}

	/* Check for end of VBlank */
	if ( y == 0 )
	{
		m_wba_h = m_vram[0x002];
		m_wba_v = m_vram[0x003];
		m_wsi_h = m_vram[0x004];
		m_wsi_v = m_vram[0x005];
		m_vram[0x010] &= ~ 0x40;
		if (m_vram[0x000] & 0x80)
		{
			m_vblank_pin_w(0);
		}
	}

	/* Check if Hint should be triggered */
	if ( y == K1GE_SCREEN_HEIGHT - 1 || y < 151 )
	{
		if (!m_hblank_pin_w.isunset())
		{
			if (m_vram[0x000] & 0x40)
			{
				m_hblank_pin_w(1);
			}
			m_hblank_on_timer->adjust( screen().time_until_pos(y, 480 ) );
		}
	}

	/* Draw a line when inside visible area */
	if ( y && y < 153 )
	{
		draw( y - 1 );
	}

	m_timer->adjust( screen().time_until_pos(( y + 1 ) % K1GE_SCREEN_HEIGHT, 0 ) );
}


void k1ge_device::update( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	copybitmap( bitmap, *m_bitmap, 0, 0, 0, 0, cliprect );
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k1ge_device::device_start()
{
	m_timer = timer_alloc(FUNC(k1ge_device::timer_callback), this);
	m_hblank_on_timer = timer_alloc(FUNC(k1ge_device::hblank_on_timer_callback), this);
	m_vram = make_unique_clear<uint8_t[]>(0x4000);
	m_bitmap = std::make_unique<bitmap_ind16>(screen().width(), screen().height() );

	save_pointer(NAME(m_vram), 0x4000);
	save_item(NAME(m_wba_h));
	save_item(NAME(m_wba_v));
	save_item(NAME(m_wsi_h));
	save_item(NAME(m_wsi_v));

	palette_init();
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void k1ge_device::device_reset()
{
	m_vram[0x000] = 0x00;   /* Interrupt enable */
	m_vram[0x002] = 0x00;   /* WBA.H */
	m_vram[0x003] = 0x00;   /* WVA.V */
	m_vram[0x004] = 0xFF;   /* WSI.H */
	m_vram[0x005] = 0xFF;   /* WSI.V */
	m_vram[0x007] = 0xc6;   /* REF */
	m_vram[0x012] = 0x00;   /* 2D control */
	m_vram[0x020] = 0x00;   /* PO.H */
	m_vram[0x021] = 0x00;   /* PO.V */
	m_vram[0x030] = 0x00;   /* PF */
	m_vram[0x032] = 0x00;   /* S1SO.H */
	m_vram[0x033] = 0x00;   /* S1SO.V */
	m_vram[0x034] = 0x00;   /* S2SO.H */
	m_vram[0x035] = 0x00;   /* S2SO.V */
	m_vram[0x101] = 0x07;   /* SPPLT01 */
	m_vram[0x102] = 0x07;   /* SPPLT02 */
	m_vram[0x103] = 0x07;   /* SPPLT03 */
	m_vram[0x105] = 0x07;   /* SPPLT11 */
	m_vram[0x106] = 0x07;   /* SPPLT12 */
	m_vram[0x107] = 0x07;   /* SPPLT13 */
	m_vram[0x109] = 0x07;   /* SC1PLT01 */
	m_vram[0x10a] = 0x07;   /* SC1PLT02 */
	m_vram[0x10b] = 0x07;   /* SC1PLT03 */
	m_vram[0x10d] = 0x07;   /* SC1PLT11 */
	m_vram[0x10e] = 0x07;   /* SC1PLT12 */
	m_vram[0x10f] = 0x07;   /* SC1PLT13 */
	m_vram[0x111] = 0x07;   /* SC2PLT01 */
	m_vram[0x112] = 0x07;   /* SC2PLT02 */
	m_vram[0x113] = 0x07;   /* SC2PLT03 */
	m_vram[0x115] = 0x07;   /* SC2PLT11 */
	m_vram[0x116] = 0x07;   /* SC2PLT12 */
	m_vram[0x117] = 0x07;   /* SC2PLT13 */
	m_vram[0x118] = 0x07;   /* BG */
	m_vram[0x400] = 0xFF;   /* LED control */
	m_vram[0x402] = 0x80;   /* LEDFREG */
	m_vram[0x7e0] = 0x52;   /* RESET */
	m_vram[0x7e2] = 0x00;   /* MODE */

	m_timer->adjust( screen().time_until_pos(( screen().vpos() + 1 ) % K1GE_SCREEN_HEIGHT, 0 ) );
}


DEFINE_DEVICE_TYPE(K1GE, k1ge_device, "k1ge", "K1GE Monochrome Graphics + LCD")

k1ge_device::k1ge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: k1ge_device(mconfig, K1GE, tag, owner, clock)
{
}

k1ge_device::k1ge_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_video_interface(mconfig, *this)
	, device_palette_interface(mconfig, *this)
	, m_vblank_pin_w(*this)
	, m_hblank_pin_w(*this)
{
}


DEFINE_DEVICE_TYPE(K2GE, k2ge_device, "k2ge", "K2GE Color Graphics + LCD")

k2ge_device::k2ge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: k1ge_device(mconfig, K2GE, tag, owner, clock)
{
}
