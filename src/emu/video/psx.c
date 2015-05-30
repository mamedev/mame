// license:BSD-3-Clause
// copyright-holders:smf
/*
 * PlayStation GPU emulator
 *
 * Copyright 2003-2014 smf
 *
 */

#define DEBUG_VIEWER ( 0 )

#include "emu.h"
#include "video/psx.h"

#define VERBOSE_LEVEL ( 0 )

// device type definition
const device_type CXD8514Q = &device_creator<cxd8514q_device>;
const device_type CXD8538Q = &device_creator<cxd8538q_device>;
const device_type CXD8561Q = &device_creator<cxd8561q_device>;
const device_type CXD8561BQ = &device_creator<cxd8561bq_device>;
const device_type CXD8561CQ = &device_creator<cxd8561cq_device>;
const device_type CXD8654Q = &device_creator<cxd8654q_device>;

psxgpu_device::psxgpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	m_vblank_handler(*this)
#if DEBUG_VIEWER
,
	m_screen(*this, "screen")
#endif
{
}

void psxgpu_device::device_start( void )
{
	m_vblank_handler.resolve_safe();

	if( m_type == CXD8538Q )
	{
		psx_gpu_init( 1 );
	}
	else
	{
		psx_gpu_init( 2 );
	}
}

void psxgpu_device::device_reset( void )
{
	gpu_reset();
}

cxd8514q_device::cxd8514q_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: psxgpu_device(mconfig, CXD8514Q, "CXD8514Q GPU", tag, owner, clock, "cxd8514q", __FILE__)
{
}

cxd8538q_device::cxd8538q_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: psxgpu_device(mconfig, CXD8538Q, "CXD8538Q GPU", tag, owner, clock, "cxd8538q", __FILE__)
{
}

cxd8561q_device::cxd8561q_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: psxgpu_device(mconfig, CXD8561Q, "CXD8561Q GPU", tag, owner, clock, "cxd8561q", __FILE__)
{
}

cxd8561bq_device::cxd8561bq_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: psxgpu_device(mconfig, CXD8561BQ, "CXD8561BQ", tag, owner, clock, "cxd8561bq", __FILE__)
{
}

cxd8561cq_device::cxd8561cq_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: psxgpu_device(mconfig, CXD8561CQ, "CXD8561CQ GPU", tag, owner, clock, "cxd8561cq", __FILE__)
{
}

cxd8654q_device::cxd8654q_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: psxgpu_device(mconfig, CXD8654Q, "CXD8654Q GPU", tag, owner, clock, "cxd8654q", __FILE__)
{
}

static const UINT16 m_p_n_nextpointlist4[] = { 1, 3, 0, 2 };
static const UINT16 m_p_n_prevpointlist4[] = { 2, 0, 3, 1 };
static const UINT16 m_p_n_nextpointlist4b[] = { 0, 3, 1, 2 };
static const UINT16 m_p_n_prevpointlist4b[] = { 0, 2, 3, 1 };
static const UINT16 m_p_n_nextpointlist3[] = { 1, 2, 0 };
static const UINT16 m_p_n_prevpointlist3[] = { 2, 0, 1 };

#define SINT11( x ) ( ( (INT32)( x ) << 21 ) >> 21 )

#define COORD_X( a ) ( a.sw.l )
#define COORD_Y( a ) ( a.sw.h )
#define SIZE_W( a ) ( a.w.l )
#define SIZE_H( a ) ( a.w.h )
#define BGR_C( a ) ( a.b.h3 )
#define BGR_B( a ) ( a.b.h2 )
#define BGR_G( a ) ( a.b.h )
#define BGR_R( a ) ( a.b.l )
#define TEXTURE_V( a ) ( a.b.h )
#define TEXTURE_U( a ) ( a.b.l )

INLINE void ATTR_PRINTF(3,4) verboselog( running_machine& machine, int n_level, const char *s_fmt, ... )
{
	if( VERBOSE_LEVEL >= n_level )
	{
		va_list v;
		char buf[ 32768 ];
		va_start( v, s_fmt );
		vsprintf( buf, s_fmt, v );
		va_end( v );
		logerror( "%s: %s", machine.describe_context(), buf );
	}
}

#if DEBUG_VIEWER

void psxgpu_device::DebugMeshInit( void )
{
	int width = m_screen->width();
	int height = m_screen->height();

	m_debug.b_mesh = 0;
	m_debug.b_texture = 0;
	m_debug.n_interleave = -1;
	m_debug.b_clear = 1;
	m_debug.n_coord = 0;
	m_debug.n_skip = 0;
	m_debug.mesh = auto_bitmap_ind16_alloc( machine(), width, height );
}

void psxgpu_device::DebugMesh( int n_coordx, int n_coordy )
{
	int n_coord;
	int n_colour;
	int width = m_screen->width();
	int height = m_screen->height();

	n_coordx += m_n_displaystartx;
	n_coordy += n_displaystarty;

	if( m_debug.b_clear )
	{
		m_debug.mesh->fill(0x0000);
		m_debug.b_clear = 0;
	}

	n_colour = 0x1f;
	for( n_coord = 0; n_coord < m_debug.n_coord; n_coord++ )
	{
		if( n_coordx != m_debug.n_coordx[ n_coord ] ||
			n_coordy != m_debug.n_coordy[ n_coord ] )
		{
			break;
		}
	}
	if( n_coord == m_debug.n_coord && m_debug.n_coord > 1 )
	{
		n_colour = 0xffff;
	}
	for( n_coord = 0; n_coord < m_debug.n_coord; n_coord++ )
	{
		PAIR n_x;
		PAIR n_y;
		INT32 n_xstart;
		INT32 n_ystart;
		INT32 n_xend;
		INT32 n_yend;
		INT32 n_xlen;
		INT32 n_ylen;
		INT32 n_len;
		INT32 n_dx;
		INT32 n_dy;

		n_xstart = m_debug.n_coordx[ n_coord ];
		n_xend = n_coordx;
		if( n_xend > n_xstart )
		{
			n_xlen = n_xend - n_xstart;
		}
		else
		{
			n_xlen = n_xstart - n_xend;
		}

		n_ystart = m_debug.n_coordy[ n_coord ];
		n_yend = n_coordy;
		if( n_yend > n_ystart )
		{
			n_ylen = n_yend - n_ystart;
		}
		else
		{
			n_ylen = n_ystart - n_yend;
		}

		if( n_xlen > n_ylen )
		{
			n_len = n_xlen;
		}
		else
		{
			n_len = n_ylen;
		}

		n_x.sw.h = n_xstart; n_x.sw.l = 0;
		n_y.sw.h = n_ystart; n_y.sw.l = 0;

		if( n_len == 0 )
		{
			n_len = 1;
		}

		n_dx = (INT32)( ( n_xend << 16 ) - n_x.d ) / n_len;
		n_dy = (INT32)( ( n_yend << 16 ) - n_y.d ) / n_len;
		while( n_len > 0 )
		{
			if( (INT16)n_x.w.h >= 0 &&
				(INT16)n_y.w.h >= 0 &&
				(INT16)n_x.w.h <= width - 1 &&
				(INT16)n_y.w.h <= height - 1 )
			{
				if( m_debug.mesh->pix16(n_y.w.h, n_x.w.h) != 0xffff )
					m_debug.mesh->pix16(n_y.w.h, n_x.w.h) = n_colour;
			}
			n_x.d += n_dx;
			n_y.d += n_dy;
			n_len--;
		}
	}

	if( m_debug.n_coord < DEBUG_COORDS )
	{
		m_debug.n_coordx[ m_debug.n_coord ] = n_coordx;
		m_debug.n_coordy[ m_debug.n_coord ] = n_coordy;
		m_debug.n_coord++;
	}
}

void psxgpu_device::DebugMeshEnd( void )
{
	m_debug.n_coord = 0;
}

void psxgpu_device::DebugCheckKeys( void )
{
	if( machine().input().code_pressed_once( KEYCODE_M ) )
	{
		m_debug.b_mesh = !m_debug.b_mesh;
		updatevisiblearea();
	}

	if( machine().input().code_pressed_once( KEYCODE_V ) )
	{
		m_debug.b_texture = !m_debug.b_texture;
		updatevisiblearea();
	}

	if( machine().input().code_pressed_once( KEYCODE_I ) )
	{
		if( m_debug.b_texture )
		{
			m_debug.n_interleave++;

			if( m_debug.n_interleave == 2 )
				m_debug.n_interleave = -1;

			if( m_debug.n_interleave == -1 )
				popmessage( "interleave off" );
			else if( m_debug.n_interleave == 0 )
				popmessage( "4 bit interleave" );
			else if( m_debug.n_interleave == 1 )
				popmessage( "8 bit interleave" );
		}
		else
		{
			m_debug.n_skip++;

			if( m_debug.n_skip > 15 )
				m_debug.n_skip = 0;

			popmessage( "debug skip %d", m_debug.n_skip );
		}
	}

#if 0
	if( machine().input().code_pressed_once( KEYCODE_D ) )
	{
		FILE *f;
		int n_x;
		f = fopen( "dump.txt", "w" );
		for( n_y = 256; n_y < 512; n_y++ )
			for( n_x = 640; n_x < 1024; n_x++ )
				fprintf( f, "%04u,%04u = %04x\n", n_y, n_x, p_p_vram[ n_y ][ n_x ] );
		fclose( f );
	}
	if( machine().input().code_pressed_once( KEYCODE_S ) )
	{
		FILE *f;
		popmessage( "saving..." );
		f = fopen( "VRAM.BIN", "wb" );
		for( n_y = 0; n_y < 1024; n_y++ )
			fwrite( p_p_vram[ n_y ], 1024 * 2, 1, f );
		fclose( f );
	}
	if( machine().input().code_pressed_once( KEYCODE_L ) )
	{
		FILE *f;
		popmessage( "loading..." );
		f = fopen( "VRAM.BIN", "rb" );
		for( n_y = 0; n_y < 1024; n_y++ )
			fread( p_p_vram[ n_y ], 1024 * 2, 1, f );
		fclose( f );
	}
#endif
}

int psxgpu_device::DebugMeshDisplay( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	if( m_debug.b_mesh )
	{
		copybitmap( bitmap, *m_debug.mesh, 0, 0, 0, 0, cliprect );
	}
	m_debug.b_clear = 1;
	return m_debug.b_mesh;
}

int psxgpu_device::DebugTextureDisplay( bitmap_ind16 &bitmap )
{
	UINT32 n_y;

	if( m_debug.b_texture )
	{
		int width = m_screen->width();
		int height = m_screen->height();

		for( n_y = 0; n_y < height; n_y++ )
		{
			int n_x;
			int n_xi;
			int n_yi;
			UINT16 p_n_interleave[ 1024 ];

			for( n_x = 0; n_x < width; n_x++ )
			{
				if( m_debug.n_interleave == 0 )
				{
					n_xi = ( n_x & ~0x3c ) + ( ( n_y << 2 ) & 0x3c );
					n_yi = ( n_y & ~0xf ) + ( ( n_x >> 2 ) & 0xf );
				}
				else if( m_debug.n_interleave == 1 )
				{
					n_xi = ( n_x & ~0x78 ) + ( ( n_x << 3 ) & 0x40 ) + ( ( n_y << 3 ) & 0x38 );
					n_yi = ( n_y & ~0x7 ) + ( ( n_x >> 4 ) & 0x7 );
				}
				else
				{
					n_xi = n_x;
					n_yi = n_y;
				}
				p_n_interleave[ n_x ] = p_p_vram[ n_yi ][ n_xi ];
			}
			draw_scanline16( bitmap, 0, n_y, width, p_n_interleave, m_screen->palette()->pens() );
		}
	}
	return m_debug.b_texture;
}

#endif

void psxgpu_device::updatevisiblearea()
{
	rectangle visarea;
	float refresh;

	if( ( n_gpustatus & ( 1 << 0x14 ) ) != 0 )
	{
		/* pal */
		refresh = 50;
		switch( ( n_gpustatus >> 0x13 ) & 1 )
		{
		case 0:
			n_screenheight = 256;
			break;
		case 1:
			n_screenheight = 512;
			break;
		}
	}
	else
	{
		/* ntsc */
		refresh = 60;
		switch( ( n_gpustatus >> 0x13 ) & 1 )
		{
		case 0:
			n_screenheight = 240;
			break;
		case 1:
			n_screenheight = 480;
			break;
		}
	}
	switch( ( n_gpustatus >> 0x11 ) & 3 )
	{
	case 0:
		switch( ( n_gpustatus >> 0x10 ) & 1 )
		{
		case 0:
			n_screenwidth = 256;
			break;
		case 1:
			n_screenwidth = 368;
			break;
		}
		break;
	case 1:
		switch( ( n_gpustatus >> 0x10 ) & 1 )
		{
		case 0:
			n_screenwidth = 320;
			break;
		case 1:
			n_screenwidth = 384;
			break;
		}
		break;
	case 2:
		n_screenwidth = 512;
		break;
	case 3:
		n_screenwidth = 640;
		break;
	}

#if DEBUG_VIEWER
	if( m_debug.b_mesh || m_debug.b_texture )
	{
		n_screenheight = 1024;
		n_screenwidth = 1024;
	}
#endif

	visarea.set(0, n_screenwidth - 1, 0, n_screenheight - 1);
	machine().first_screen()->configure(n_screenwidth, n_screenheight, visarea, HZ_TO_ATTOSECONDS(refresh));
}

void psxgpu_device::psx_gpu_init( int n_gputype )
{
	int n_line;
	int n_level;
	int n_level2;
	int n_shade;
	int n_shaded;
	int width = 1024;
	int height = ( vramSize / width ) / sizeof( UINT16 );

	m_n_gputype = n_gputype;

#if DEBUG_VIEWER
	DebugMeshInit();
#endif

	n_gpustatus = 0x14802000;
	n_gpuinfo = 0;
	n_gpu_buffer_offset = 0;
	n_lightgun_x = 0;
	n_lightgun_y = 0;
	b_reverseflag = 0;

	p_vram = auto_alloc_array_clear( machine(), UINT16, width * height );

	for( n_line = 0; n_line < 1024; n_line++ )
	{
		p_p_vram[ n_line ] = &p_vram[ ( n_line % height ) * width ];
	}

	for( n_level = 0; n_level < MAX_LEVEL; n_level++ )
	{
		for( n_shade = 0; n_shade < MAX_SHADE; n_shade++ )
		{
			/* shaded */
			n_shaded = ( n_level * n_shade ) / MID_SHADE;
			if( n_shaded > MAX_LEVEL - 1 )
			{
				n_shaded = MAX_LEVEL - 1;
			}
			p_n_redshade[ ( n_level * MAX_SHADE ) | n_shade ] = n_shaded;
			p_n_greenshade[ ( n_level * MAX_SHADE ) | n_shade ] = n_shaded << 5;
			p_n_blueshade[ ( n_level * MAX_SHADE ) | n_shade ] = n_shaded << 10;

			/* 1/4 x transparency */
			n_shaded = ( n_level * n_shade ) / MID_SHADE;
			n_shaded >>= 2;
			if( n_shaded > MAX_LEVEL - 1 )
			{
				n_shaded = MAX_LEVEL - 1;
			}
			p_n_f025[ ( n_level * MAX_SHADE ) | n_shade ] = n_shaded;

			/* 1/2 x transparency */
			n_shaded = ( n_level * n_shade ) / MID_SHADE;
			n_shaded >>= 1;
			if( n_shaded > MAX_LEVEL - 1 )
			{
				n_shaded = MAX_LEVEL - 1;
			}
			p_n_f05[ ( n_level * MAX_SHADE ) | n_shade ] = n_shaded;

			/* 1 x transparency */
			n_shaded = ( n_level * n_shade ) / MID_SHADE;
			if( n_shaded > MAX_LEVEL - 1 )
			{
				n_shaded = MAX_LEVEL - 1;
			}
			p_n_f1[ ( n_level * MAX_SHADE ) | n_shade ] = n_shaded;
		}
	}

	for( n_level = 0; n_level < 0x10000; n_level++ )
	{
		p_n_redlevel[ n_level ] = ( n_level & ( MAX_LEVEL - 1 ) ) * MAX_SHADE;
		p_n_greenlevel[ n_level ] = ( ( n_level >> 5 ) & ( MAX_LEVEL - 1 ) ) * MAX_SHADE;
		p_n_bluelevel[ n_level ] = ( ( n_level >> 10 ) & ( MAX_LEVEL - 1 ) ) * MAX_SHADE;

		/* 0.5 * background */
		p_n_redb05[ n_level ] = ( ( n_level & ( MAX_LEVEL - 1 ) ) / 2 ) * MAX_LEVEL;
		p_n_greenb05[ n_level ] = ( ( ( n_level >> 5 ) & ( MAX_LEVEL - 1 ) ) / 2 ) * MAX_LEVEL;
		p_n_blueb05[ n_level ] = ( ( ( n_level >> 10 ) & ( MAX_LEVEL - 1 ) ) / 2 ) * MAX_LEVEL;

		/* 1 * background */
		p_n_redb1[ n_level ] = ( n_level & ( MAX_LEVEL - 1 ) ) * MAX_LEVEL;
		p_n_greenb1[ n_level ] = ( ( n_level >> 5 ) & ( MAX_LEVEL - 1 ) ) * MAX_LEVEL;
		p_n_blueb1[ n_level ] = ( ( n_level >> 10 ) & ( MAX_LEVEL - 1 ) ) * MAX_LEVEL;

		/* 24bit to 15 bit conversion */
		p_n_g0r0[ n_level ] = ( ( ( n_level >> 11 ) & ( MAX_LEVEL - 1 ) ) << 5 ) | ( ( ( n_level >> 3 ) & ( MAX_LEVEL - 1 ) ) << 0 );
		p_n_b0[ n_level ] = ( ( n_level >> 3 ) & ( MAX_LEVEL - 1 ) ) << 10;
		p_n_r1[ n_level ] = ( ( n_level >> 11 ) & ( MAX_LEVEL - 1 ) ) << 0;
		p_n_b1g1[ n_level ] = ( ( ( n_level >> 11 ) & ( MAX_LEVEL - 1 ) ) << 10 ) | ( ( ( n_level >> 3 ) & ( MAX_LEVEL - 1 ) ) << 5 );
	}

	for( n_level = 0; n_level < MAX_LEVEL; n_level++ )
	{
		for( n_level2 = 0; n_level2 < MAX_LEVEL; n_level2++ )
		{
			/* add transparency */
			n_shaded = ( n_level + n_level2 );
			if( n_shaded > MAX_LEVEL - 1 )
			{
				n_shaded = MAX_LEVEL - 1;
			}
			p_n_redaddtrans[ ( n_level * MAX_LEVEL ) | n_level2 ] = n_shaded;
			p_n_greenaddtrans[ ( n_level * MAX_LEVEL ) | n_level2 ] = n_shaded << 5;
			p_n_blueaddtrans[ ( n_level * MAX_LEVEL ) | n_level2 ] = n_shaded << 10;

			/* sub transparency */
			n_shaded = ( n_level - n_level2 );
			if( n_shaded < 0 )
			{
				n_shaded = 0;
			}
			p_n_redsubtrans[ ( n_level * MAX_LEVEL ) | n_level2 ] = n_shaded;
			p_n_greensubtrans[ ( n_level * MAX_LEVEL ) | n_level2 ] = n_shaded << 5;
			p_n_bluesubtrans[ ( n_level * MAX_LEVEL ) | n_level2 ] = n_shaded << 10;
		}
	}

	// icky!!!
	machine().save().save_memory( this, "globals", NULL, 0, "m_packet", (UINT8 *)&m_packet, 1, sizeof( m_packet ) );

	save_pointer(NAME(p_vram), width * height );
	save_item(NAME(n_gpu_buffer_offset));
	save_item(NAME(n_vramx));
	save_item(NAME(n_vramy));
	save_item(NAME(n_twy));
	save_item(NAME(n_twx));
	save_item(NAME(n_tww));
	save_item(NAME(n_drawarea_x1));
	save_item(NAME(n_drawarea_y1));
	save_item(NAME(n_drawarea_x2));
	save_item(NAME(n_drawarea_y2));
	save_item(NAME(n_horiz_disstart));
	save_item(NAME(n_horiz_disend));
	save_item(NAME(n_vert_disstart));
	save_item(NAME(n_vert_disend));
	save_item(NAME(b_reverseflag));
	save_item(NAME(n_drawoffset_x));
	save_item(NAME(n_drawoffset_y));
	save_item(NAME(m_n_displaystartx));
	save_item(NAME(n_displaystarty));
	save_item(NAME(n_gpustatus));
	save_item(NAME(n_gpuinfo));
	save_item(NAME(n_lightgun_x));
	save_item(NAME(n_lightgun_y));
	save_item(NAME(m_n_tx));
	save_item(NAME(m_n_ty));
	save_item(NAME(n_abr));
	save_item(NAME(n_tp));
	save_item(NAME(n_ix));
	save_item(NAME(n_iy));
	save_item(NAME(n_ti));

	machine().save().register_postload( save_prepost_delegate( FUNC( psxgpu_device::updatevisiblearea ), this ) );
}

UINT32 psxgpu_device::update_screen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT32 n_x;
	UINT32 n_y;
	int n_top;
	int n_line;
	int n_lines;
	int n_left;
	int n_column;
	int n_columns;
	int n_displaystartx;
	int n_overscantop;
	int n_overscanleft;

#if DEBUG_VIEWER
	if( DebugMeshDisplay( bitmap, cliprect ) )
	{
		return 0;
	}
	if( DebugTextureDisplay( bitmap ) )
	{
		return 0;
	}
#endif

	if( ( n_gpustatus & ( 1 << 0x17 ) ) != 0 )
	{
		/* todo: only draw to necessary area */
		bitmap.fill(0, cliprect);
	}
	else
	{
		if( b_reverseflag )
		{
			n_displaystartx = ( 1023 - m_n_displaystartx );
			/* todo: make this flip the screen, in the meantime.. */
			n_displaystartx -= ( n_screenwidth - 1 );
		}
		else
		{
			n_displaystartx = m_n_displaystartx;
		}

		if( ( n_gpustatus & ( 1 << 0x14 ) ) != 0 )
		{
			/* pal */
			n_overscantop = 0x23;
			n_overscanleft = 0x27e;
		}
		else
		{
			/* ntsc */
			n_overscantop = 0x10;
			n_overscanleft = 0x260;
		}

		n_top = (INT32)n_vert_disstart - n_overscantop;
		n_lines = (INT32)n_vert_disend - (INT32)n_vert_disstart;
		if( n_top < 0 )
		{
			n_y = -n_top;
			n_lines += n_top;
		}
		else
		{
			n_y = 0;

			/* draw top border */
			rectangle clip(cliprect.left(), cliprect.right(), cliprect.top(), n_top);
			bitmap.fill(0, clip);
		}
		if( ( n_gpustatus & ( 1 << 0x16 ) ) != 0 )
		{
			/* interlaced */
			n_lines *= 2;
		}
		if( n_lines > n_screenheight - ( n_y + n_top ) )
		{
			n_lines = n_screenheight - ( n_y + n_top );
		}
		else
		{
			/* draw bottom border */
			rectangle clip(cliprect.left(), cliprect.right(), n_y + n_top + n_lines, cliprect.bottom());
			bitmap.fill(0, clip);
		}

		n_left = ( ( (INT32)n_horiz_disstart - n_overscanleft ) * (INT32)n_screenwidth ) / 2560;
		n_columns = ( ( ( (INT32)n_horiz_disend - n_horiz_disstart ) * (INT32)n_screenwidth ) / 2560 );
		if( n_left < 0 )
		{
			n_x = -n_left;
			n_columns += n_left;
		}
		else
		{
			n_x = 0;

			/* draw left border */
			rectangle clip(cliprect.left(), n_x + n_left, cliprect.top(), cliprect.bottom());
			bitmap.fill(0, clip);
		}
		if( n_columns > n_screenwidth - ( n_x + n_left ) )
		{
			n_columns = n_screenwidth - ( n_x + n_left );
		}
		else
		{
			/* draw right border */
			rectangle clip(n_x + n_left + n_columns, cliprect.right(), cliprect.top(), cliprect.bottom());
			bitmap.fill(0, clip);
		}

		if( ( n_gpustatus & ( 1 << 0x15 ) ) != 0 )
		{
			/* 24bit */
			n_line = n_lines;
			while( n_line > 0 )
			{
				UINT16 *p_n_src = p_p_vram[ n_y + n_displaystarty ] + ((n_x + n_displaystartx) * 3);
				UINT16 *p_n_dest = &bitmap.pix16(n_y + n_top, n_x + n_left);

				n_column = n_columns;
				while( n_column > 0 )
				{
					UINT32 n_g0r0 = *( p_n_src++ );
					UINT32 n_r1b0 = *( p_n_src++ );
					UINT32 n_b1g1 = *( p_n_src++ );

					*( p_n_dest++ ) = p_n_g0r0[ n_g0r0 ] | p_n_b0[ n_r1b0 ];
					n_column--;
					if( n_column > 0 )
					{
						*( p_n_dest++ ) = p_n_r1[ n_r1b0 ] | p_n_b1g1[ n_b1g1 ];
						n_column--;
					}
				}
				n_y++;
				n_line--;
			}
		}
		else
		{
			/* 15bit */
			n_line = n_lines;
			while( n_line > 0 )
			{
				draw_scanline16( bitmap, n_x + n_left, n_y + n_top, n_columns, p_p_vram[ ( n_y + n_displaystarty ) & 1023 ] + n_x + n_displaystartx, NULL );
				n_y++;
				n_line--;
			}
		}
	}
	return 0;
}

#define WRITE_PIXEL( p ) *( p_vram ) = p

/*
type 1
f  e| d| c  b| a  9| 8  7| 6  5| 4| 3  2  1  0
    |ti|     |   tp|  abr|   ty|  |         tx
*/

/*
type 2
f  e| d  c| b| a  9| 8  7| 6  5| 4| 3  2  1  0
    |iy|ix|ty|     |   tp|  abr|ty|         tx
*/

void psxgpu_device::decode_tpage( UINT32 tpage )
{
	if( m_n_gputype == 2 )
	{
		n_gpustatus = ( n_gpustatus & 0xfffff800 ) | ( tpage & 0x7ff );

		m_n_tx = ( tpage & 0x0f ) << 6;
		m_n_ty = ( ( tpage & 0x10 ) << 4 ) | ( ( tpage & 0x800 ) >> 2 );
		n_abr = ( tpage & 0x60 ) >> 5;
		n_tp = ( tpage & 0x180 ) >> 7;
		n_ix = ( tpage & 0x1000 ) >> 12;
		n_iy = ( tpage & 0x2000 ) >> 13;
		n_ti = 0;
		if( ( tpage & ~0x39ff ) != 0 )
		{
			verboselog( machine(), 1, "not handled: draw mode %08x\n", tpage & ~0x39ff );
		}
		if( n_tp == 3 )
		{
			verboselog( machine(), 0, "not handled: tp == 3\n" );
		}
	}
	else
	{
		n_gpustatus = ( n_gpustatus & 0xffffe000 ) | ( tpage & 0x1fff );

		m_n_tx = ( tpage & 0x0f ) << 6;
		m_n_ty = ( ( tpage & 0x60 ) << 3 );
		n_abr = ( tpage & 0x180 ) >> 7;
		n_tp = ( tpage & 0x600 ) >> 9;
		n_ti = ( tpage & 0x2000 ) >> 13;
		n_ix = 0;
		n_iy = 0;
		if( ( tpage & ~0x27ef ) != 0 )
		{
			verboselog( machine(), 1, "not handled: draw mode %08x\n", tpage & ~0x27ef );
		}
		if( n_tp == 3 )
		{
			verboselog( machine(), 0, "not handled: tp == 3\n" );
		}
		else if( n_tp == 2 && n_ti != 0 )
		{
			verboselog( machine(), 0, "not handled: interleaved 15 bit texture\n" );
		}
	}
}

#define SPRITESETUP \
	if( n_iy != 0 ) \
	{ \
		n_dv = -1; \
	} \
	else \
	{ \
		n_dv = 1; \
	} \
	if( n_ix != 0 ) \
	{ \
		n_du = -1; \
	} \
	else \
	{ \
		n_du = 1; \
	}

#define TRANSPARENCYSETUP \
	p_n_f = p_n_f1; \
	p_n_redb = p_n_redb1; \
	p_n_greenb = p_n_greenb1; \
	p_n_blueb = p_n_blueb1; \
	p_n_redtrans = p_n_redaddtrans; \
	p_n_greentrans = p_n_greenaddtrans; \
	p_n_bluetrans = p_n_blueaddtrans; \
	\
	switch( n_cmd & 0x02 ) \
	{ \
	case 0x02: \
		switch( n_abr ) \
		{ \
		case 0x00: \
			p_n_f = p_n_f05; \
			p_n_redb = p_n_redb05; \
			p_n_greenb = p_n_greenb05; \
			p_n_blueb = p_n_blueb05; \
			p_n_redtrans = p_n_redaddtrans; \
			p_n_greentrans = p_n_greenaddtrans; \
			p_n_bluetrans = p_n_blueaddtrans; \
			verboselog( machine(), 2, "Transparency Mode: 0.5*B + 0.5*F\n" ); \
			break; \
		case 0x01: \
			p_n_f = p_n_f1; \
			p_n_redb = p_n_redb1; \
			p_n_greenb = p_n_greenb1; \
			p_n_blueb = p_n_blueb1; \
			p_n_redtrans = p_n_redaddtrans; \
			p_n_greentrans = p_n_greenaddtrans; \
			p_n_bluetrans = p_n_blueaddtrans; \
			verboselog( machine(), 2, "Transparency Mode: 1.0*B + 1.0*F\n" ); \
			break; \
		case 0x02: \
			p_n_f = p_n_f1; \
			p_n_redb = p_n_redb1; \
			p_n_greenb = p_n_greenb1; \
			p_n_blueb = p_n_blueb1; \
			p_n_redtrans = p_n_redsubtrans; \
			p_n_greentrans = p_n_greensubtrans; \
			p_n_bluetrans = p_n_bluesubtrans; \
			verboselog( machine(), 2, "Transparency Mode: 1.0*B - 1.0*F\n" ); \
			break; \
		case 0x03: \
			p_n_f = p_n_f025; \
			p_n_redb = p_n_redb1; \
			p_n_greenb = p_n_greenb1; \
			p_n_blueb = p_n_blueb1; \
			p_n_redtrans = p_n_redaddtrans; \
			p_n_greentrans = p_n_greenaddtrans; \
			p_n_bluetrans = p_n_blueaddtrans; \
			verboselog( machine(), 2, "Transparency Mode: 1.0*B + 0.25*F\n" ); \
			break; \
		} \
		break; \
	}

#define SOLIDSETUP \
	TRANSPARENCYSETUP

#define TEXTURESETUP \
	n_tx = m_n_tx; \
	n_ty = m_n_ty; \
	p_clut = p_p_vram[ n_cluty ] + n_clutx; \
	switch( n_tp ) \
	{ \
	case 0: \
		n_tx += n_twx >> 2; \
		n_ty += n_twy; \
		break; \
	case 1: \
		n_tx += n_twx >> 1; \
		n_ty += n_twy; \
		break; \
	case 2: \
		n_tx += n_twx >> 0; \
		n_ty += n_twy; \
		break; \
	} \
	TRANSPARENCYSETUP

#define FLATPOLYGONUPDATE
#define FLATRECTANGEUPDATE
#define GOURAUDPOLYGONUPDATE \
	n_r.d += n_dr; \
	n_g.d += n_dg; \
	n_b.d += n_db;

#define SOLIDFILL( PIXELUPDATE ) \
	if( n_distance > ( (INT32)n_drawarea_x2 - drawx ) + 1 ) \
	{ \
		n_distance = ( n_drawarea_x2 - drawx ) + 1; \
	} \
	p_vram = p_p_vram[ drawy ] + drawx; \
	\
	switch( n_cmd & 0x02 ) \
	{ \
	case 0x00: \
		/* transparency off */ \
		while( n_distance > 0 ) \
		{ \
			WRITE_PIXEL( \
				p_n_redshade[ MID_LEVEL | n_r.w.h ] | \
				p_n_greenshade[ MID_LEVEL | n_g.w.h ] | \
				p_n_blueshade[ MID_LEVEL | n_b.w.h ] ); \
			p_vram++; \
			PIXELUPDATE \
			n_distance--; \
		} \
		break; \
	case 0x02: \
		/* transparency on */ \
		while( n_distance > 0 ) \
		{ \
			WRITE_PIXEL( \
				p_n_redtrans[ p_n_f[ MID_LEVEL | n_r.w.h ] | p_n_redb[ *( p_vram ) ] ] | \
				p_n_greentrans[ p_n_f[ MID_LEVEL | n_g.w.h ] | p_n_greenb[ *( p_vram ) ] ] | \
				p_n_bluetrans[ p_n_f[ MID_LEVEL | n_b.w.h ] | p_n_blueb[ *( p_vram ) ] ] ); \
			p_vram++; \
			PIXELUPDATE \
			n_distance--; \
		} \
		break; \
	}

#define FLATTEXTUREDPOLYGONUPDATE \
	n_u.d += n_du; \
	n_v.d += n_dv;

#define GOURAUDTEXTUREDPOLYGONUPDATE \
	n_r.d += n_dr; \
	n_g.d += n_dg; \
	n_b.d += n_db; \
	n_u.d += n_du; \
	n_v.d += n_dv;

#define FLATTEXTUREDRECTANGLEUPDATE \
	n_u += n_du;

#define TEXTURE_LOOP \
	while( n_distance > 0 ) \
	{
#define TEXTURE_ENDLOOP \
	}

#define TEXTURE4BIT( TXV, TXU ) \
	TEXTURE_LOOP \
		n_bgr = p_clut[ ( *( p_p_vram[ n_ty + TXV ] + n_tx + ( TXU >> 2 ) ) >> ( ( TXU & 0x03 ) << 2 ) ) & 0x0f ];

#define TEXTURE8BIT( TXV, TXU ) \
	TEXTURE_LOOP \
		n_bgr = p_clut[ ( *( p_p_vram[ n_ty + TXV ] + n_tx + ( TXU >> 1 ) ) >> ( ( TXU & 0x01 ) << 3 ) ) & 0xff ];

#define TEXTURE15BIT( TXV, TXU ) \
	TEXTURE_LOOP \
		n_bgr = *( p_p_vram[ n_ty + TXV ] + n_tx + TXU );

#define TEXTUREWINDOW4BIT( TXV, TXU ) TEXTURE4BIT( ( TXV & n_twh ), ( TXU & n_tww ) )
#define TEXTUREWINDOW8BIT( TXV, TXU ) TEXTURE8BIT( ( TXV & n_twh ), ( TXU & n_tww ) )
#define TEXTUREWINDOW15BIT( TXV, TXU ) TEXTURE15BIT( ( TXV & n_twh ), ( TXU & n_tww ) )

#define TEXTUREINTERLEAVED4BIT( TXV, TXU ) \
	TEXTURE_LOOP \
		int n_xi = ( ( TXU >> 2 ) & ~0x3c ) + ( ( TXV << 2 ) & 0x3c ); \
		int n_yi = ( TXV & ~0xf ) + ( ( TXU >> 4 ) & 0xf ); \
		n_bgr = p_clut[ ( *( p_p_vram[ n_ty + n_yi ] + n_tx + n_xi ) >> ( ( TXU & 0x03 ) << 2 ) ) & 0x0f ];

#define TEXTUREINTERLEAVED8BIT( TXV, TXU ) \
	TEXTURE_LOOP \
		int n_xi = ( ( TXU >> 1 ) & ~0x78 ) + ( ( TXU << 2 ) & 0x40 ) + ( ( TXV << 3 ) & 0x38 ); \
		int n_yi = ( TXV & ~0x7 ) + ( ( TXU >> 5 ) & 0x7 ); \
		n_bgr = p_clut[ ( *( p_p_vram[ n_ty + n_yi ] + n_tx + n_xi ) >> ( ( TXU & 0x01 ) << 3 ) ) & 0xff ];

#define TEXTUREINTERLEAVED15BIT( TXV, TXU ) \
	TEXTURE_LOOP \
		int n_xi = TXU; \
		int n_yi = TXV; \
		n_bgr = *( p_p_vram[ n_ty + n_yi ] + n_tx + n_xi );

#define TEXTUREWINDOWINTERLEAVED4BIT( TXV, TXU ) TEXTUREINTERLEAVED4BIT( ( TXV & n_twh ), ( TXU & n_tww ) )
#define TEXTUREWINDOWINTERLEAVED8BIT( TXV, TXU ) TEXTUREINTERLEAVED8BIT( ( TXV & n_twh ), ( TXU & n_tww ) )
#define TEXTUREWINDOWINTERLEAVED15BIT( TXV, TXU ) TEXTUREINTERLEAVED15BIT( ( TXV & n_twh ), ( TXU & n_tww ) )

#define SHADEDPIXEL( PIXELUPDATE ) \
		if( n_bgr != 0 ) \
		{ \
			WRITE_PIXEL( \
				p_n_redshade[ p_n_redlevel[ n_bgr ] | n_r.w.h ] | \
				p_n_greenshade[ p_n_greenlevel[ n_bgr ] | n_g.w.h ] | \
				p_n_blueshade[ p_n_bluelevel[ n_bgr ] | n_b.w.h ] ); \
		} \
		p_vram++; \
		PIXELUPDATE \
		n_distance--; \
	TEXTURE_ENDLOOP

#define TRANSPARENTPIXEL( PIXELUPDATE ) \
		if( n_bgr != 0 ) \
		{ \
			if( ( n_bgr & 0x8000 ) != 0 ) \
			{ \
				WRITE_PIXEL( \
					p_n_redtrans[ p_n_f[ p_n_redlevel[ n_bgr ] | n_r.w.h ] | p_n_redb[ *( p_vram ) ] ] | \
					p_n_greentrans[ p_n_f[ p_n_greenlevel[ n_bgr ] | n_g.w.h ] | p_n_greenb[ *( p_vram ) ] ] | \
					p_n_bluetrans[ p_n_f[ p_n_bluelevel[ n_bgr ] | n_b.w.h ] | p_n_blueb[ *( p_vram ) ] ] ); \
			} \
			else \
			{ \
				WRITE_PIXEL( \
					p_n_redshade[ p_n_redlevel[ n_bgr ] | n_r.w.h ] | \
					p_n_greenshade[ p_n_greenlevel[ n_bgr ] | n_g.w.h ] | \
					p_n_blueshade[ p_n_bluelevel[ n_bgr ] | n_b.w.h ] ); \
			} \
		} \
		p_vram++; \
		PIXELUPDATE \
		n_distance--; \
	TEXTURE_ENDLOOP

#define TEXTUREFILL( PIXELUPDATE, TXU, TXV ) \
	if( n_distance > ( (INT32)n_drawarea_x2 - drawx ) + 1 ) \
	{ \
		n_distance = ( n_drawarea_x2 - drawx ) + 1; \
	} \
	p_vram = p_p_vram[ drawy ] + drawx; \
	\
	if( n_ti != 0 ) \
	{ \
		/* interleaved texture */ \
		if( n_twh != 255 || \
			n_tww != 255 || \
			n_twx != 0 || \
			n_twy != 0 ) \
		{ \
			/* texture window */ \
			switch( n_cmd & 0x02 ) \
			{ \
			case 0x00: \
				/* shading */ \
				switch( n_tp ) \
				{ \
				case 0: \
					/* 4 bit clut */ \
					TEXTUREWINDOWINTERLEAVED4BIT( TXV, TXU ) \
					SHADEDPIXEL( PIXELUPDATE ) \
					break; \
				case 1: \
					/* 8 bit clut */ \
					TEXTUREWINDOWINTERLEAVED8BIT( TXV, TXU ) \
					SHADEDPIXEL( PIXELUPDATE ) \
					break; \
				case 2: \
					/* 15 bit */ \
					TEXTUREWINDOWINTERLEAVED15BIT( TXV, TXU ) \
					SHADEDPIXEL( PIXELUPDATE ) \
					break; \
				} \
				break; \
			case 0x02: \
				/* semi transparency */ \
				switch( n_tp ) \
				{ \
				case 0: \
					/* 4 bit clut */ \
					TEXTUREWINDOWINTERLEAVED4BIT( TXV, TXU ) \
					TRANSPARENTPIXEL( PIXELUPDATE ) \
					break; \
				case 1: \
					/* 8 bit clut */ \
					TEXTUREWINDOWINTERLEAVED8BIT( TXV, TXU ) \
					TRANSPARENTPIXEL( PIXELUPDATE ) \
					break; \
				case 2: \
					/* 15 bit */ \
					TEXTUREWINDOWINTERLEAVED15BIT( TXV, TXU ) \
					TRANSPARENTPIXEL( PIXELUPDATE ) \
					break; \
				} \
				break; \
			} \
		} \
		else \
		{ \
			/* no texture window */ \
			switch( n_cmd & 0x02 ) \
			{ \
			case 0x00: \
				/* shading */ \
				switch( n_tp ) \
				{ \
				case 0: \
					/* 4 bit clut */ \
					TEXTUREINTERLEAVED4BIT( TXV, TXU ) \
					SHADEDPIXEL( PIXELUPDATE ) \
					break; \
				case 1: \
					/* 8 bit clut */ \
					TEXTUREINTERLEAVED8BIT( TXV, TXU ) \
					SHADEDPIXEL( PIXELUPDATE ) \
					break; \
				case 2: \
					/* 15 bit */ \
					TEXTUREINTERLEAVED15BIT( TXV, TXU ) \
					SHADEDPIXEL( PIXELUPDATE ) \
					break; \
				} \
				break; \
			case 0x02: \
				/* semi transparency */ \
				switch( n_tp ) \
				{ \
				case 0: \
					/* 4 bit clut */ \
					TEXTUREINTERLEAVED4BIT( TXV, TXU ) \
					TRANSPARENTPIXEL( PIXELUPDATE ) \
					break; \
				case 1: \
					/* 8 bit clut */ \
					TEXTUREINTERLEAVED8BIT( TXV, TXU ) \
					TRANSPARENTPIXEL( PIXELUPDATE ) \
					break; \
				case 2: \
					/* 15 bit */ \
					TEXTUREINTERLEAVED15BIT( TXV, TXU ) \
					TRANSPARENTPIXEL( PIXELUPDATE ) \
					break; \
				} \
				break; \
			} \
		} \
	} \
	else \
	{ \
		/* standard texture */ \
		if( n_twh != 255 || \
			n_tww != 255 || \
			n_twx != 0 || \
			n_twy != 0 ) \
		{ \
			/* texture window */ \
			switch( n_cmd & 0x02 ) \
			{ \
			case 0x00: \
				/* shading */ \
				switch( n_tp ) \
				{ \
				case 0: \
					/* 4 bit clut */ \
					TEXTUREWINDOW4BIT( TXV, TXU ) \
					SHADEDPIXEL( PIXELUPDATE ) \
					break; \
				case 1: \
					/* 8 bit clut */ \
					TEXTUREWINDOW8BIT( TXV, TXU ) \
					SHADEDPIXEL( PIXELUPDATE ) \
					break; \
				case 2: \
					/* 15 bit */ \
					TEXTUREWINDOW15BIT( TXV, TXU ) \
					SHADEDPIXEL( PIXELUPDATE ) \
					break; \
				} \
				break; \
			case 0x02: \
				/* semi transparency */ \
				switch( n_tp ) \
				{ \
				case 0: \
					/* 4 bit clut */ \
					TEXTUREWINDOW4BIT( TXV, TXU ) \
					TRANSPARENTPIXEL( PIXELUPDATE ) \
					break; \
				case 1: \
					/* 8 bit clut */ \
					TEXTUREWINDOW8BIT( TXV, TXU ) \
					TRANSPARENTPIXEL( PIXELUPDATE ) \
					break; \
				case 2: \
					/* 15 bit */ \
					TEXTUREWINDOW15BIT( TXV, TXU ) \
					TRANSPARENTPIXEL( PIXELUPDATE ) \
					break; \
				} \
				break; \
			} \
		} \
		else \
		{ \
			/* no texture window */ \
			switch( n_cmd & 0x02 ) \
			{ \
			case 0x00: \
				/* shading */ \
				switch( n_tp ) \
				{ \
				case 0: \
					TEXTURE4BIT( TXV, TXU ) \
					SHADEDPIXEL( PIXELUPDATE ) \
					break; \
				case 1: \
					/* 8 bit clut */ \
					TEXTURE8BIT( TXV, TXU ) \
					SHADEDPIXEL( PIXELUPDATE ) \
					break; \
				case 2: \
					/* 15 bit */ \
					TEXTURE15BIT( TXV, TXU ) \
					SHADEDPIXEL( PIXELUPDATE ) \
					break; \
				} \
				break; \
			case 0x02: \
				/* semi transparency */ \
				switch( n_tp ) \
				{ \
				case 0: \
					/* 4 bit clut */ \
					TEXTURE4BIT( TXV, TXU ) \
					TRANSPARENTPIXEL( PIXELUPDATE ) \
					break; \
				case 1: \
					/* 8 bit clut */ \
					TEXTURE8BIT( TXV, TXU ) \
					TRANSPARENTPIXEL( PIXELUPDATE ) \
					break; \
				case 2: \
					/* 15 bit */ \
					TEXTURE15BIT( TXV, TXU ) \
					TRANSPARENTPIXEL( PIXELUPDATE ) \
					break; \
				} \
				break; \
			} \
		} \
	}

#define GET_COORD( a ) \
	a.sw.l = SINT11( COORD_X( a ) ); \
	a.sw.h = SINT11( COORD_Y( a ) );

INLINE int CullVertex( int a, int b )
{
	int d = a - b;
	if( d < -1023 || d > 1023 )
	{
		return 1;
	}

	return 0;
}

#define CULLPOINT( PacketType, p1, p2 ) \
( \
	CullVertex( COORD_Y( m_packet.PacketType.vertex[ p1 ].n_coord ), COORD_Y( m_packet.PacketType.vertex[ p2 ].n_coord ) ) || \
	CullVertex( COORD_X( m_packet.PacketType.vertex[ p1 ].n_coord ), COORD_X( m_packet.PacketType.vertex[ p2 ].n_coord ) ) \
)

#define CULLTRIANGLE( PacketType, start ) \
( \
	CULLPOINT( PacketType, start, start + 1 ) || CULLPOINT( PacketType, start + 1, start + 2 ) || CULLPOINT( PacketType, start + 2, start ) \
)

#define FINDTOPLEFT( PacketType ) \
	for( n_point = 0; n_point < n_points; n_point++ ) \
	{ \
		GET_COORD( m_packet.PacketType.vertex[ n_point ].n_coord ); \
	} \
	\
	n_leftpoint = 0; \
	if( n_points == 4 ) \
	{ \
		if( CULLTRIANGLE( PacketType, 0 ) ) \
		{ \
			if( CULLTRIANGLE( PacketType, 1 ) ) \
			{ \
				return; \
			} \
			\
			p_n_rightpointlist = m_p_n_nextpointlist4b; \
			p_n_leftpointlist = m_p_n_prevpointlist4b; \
			n_leftpoint++; \
		} \
		else if( CULLTRIANGLE( PacketType, 1 ) ) \
		{ \
			p_n_rightpointlist = m_p_n_nextpointlist3; \
			p_n_leftpointlist = m_p_n_prevpointlist3; \
			n_points--; \
		} \
		else \
		{ \
			p_n_rightpointlist = m_p_n_nextpointlist4; \
			p_n_leftpointlist = m_p_n_prevpointlist4; \
		} \
	} \
	else if( CULLTRIANGLE( PacketType, 0 ) ) \
	{ \
		return; \
	} \
	else \
	{ \
		p_n_rightpointlist = m_p_n_nextpointlist3; \
		p_n_leftpointlist = m_p_n_prevpointlist3; \
	} \
	\
	for( n_point = n_leftpoint + 1; n_point < n_points; n_point++ ) \
	{ \
		if( COORD_Y( m_packet.PacketType.vertex[ n_point ].n_coord ) < COORD_Y( m_packet.PacketType.vertex[ n_leftpoint ].n_coord ) || \
			( COORD_Y( m_packet.PacketType.vertex[ n_point ].n_coord ) == COORD_Y( m_packet.PacketType.vertex[ n_leftpoint ].n_coord ) && \
			COORD_X( m_packet.PacketType.vertex[ n_point ].n_coord ) < COORD_X( m_packet.PacketType.vertex[ n_leftpoint ].n_coord ) ) ) \
		{ \
			n_leftpoint = n_point; \
		} \
	} \
	n_rightpoint = n_leftpoint;

void psxgpu_device::FlatPolygon( int n_points )
{
	INT16 n_y;
	INT16 n_x;

	UINT16 *p_n_f;
	UINT16 *p_n_redb;
	UINT16 *p_n_greenb;
	UINT16 *p_n_blueb;
	UINT16 *p_n_redtrans;
	UINT16 *p_n_greentrans;
	UINT16 *p_n_bluetrans;

	PAIR n_r;
	PAIR n_g;
	PAIR n_b;
	PAIR n_cx1;
	PAIR n_cx2;
	INT32 n_dx1;
	INT32 n_dx2;

	UINT8 n_cmd;

	INT32 n_distance;

	UINT16 n_point;
	UINT16 n_rightpoint;
	UINT16 n_leftpoint;
	const UINT16 *p_n_rightpointlist;
	const UINT16 *p_n_leftpointlist;

	UINT16 *p_vram;

#if DEBUG_VIEWER
	if( m_debug.n_skip == 1 )
	{
		return;
	}
	for( n_point = 0; n_point < n_points; n_point++ )
	{
		DebugMesh( SINT11( COORD_X( m_packet.FlatPolygon.vertex[ n_point ].n_coord ) ) + n_drawoffset_x, SINT11( COORD_Y( m_packet.FlatPolygon.vertex[ n_point ].n_coord ) ) + n_drawoffset_y );
	}
	DebugMeshEnd();
#endif

	n_cmd = BGR_C( m_packet.FlatPolygon.n_bgr );

	n_cx1.d = 0;
	n_cx2.d = 0;

	SOLIDSETUP

	n_r.w.h = BGR_R( m_packet.FlatPolygon.n_bgr ); n_r.w.l = 0;
	n_g.w.h = BGR_G( m_packet.FlatPolygon.n_bgr ); n_g.w.l = 0;
	n_b.w.h = BGR_B( m_packet.FlatPolygon.n_bgr ); n_b.w.l = 0;

	FINDTOPLEFT( FlatPolygon )

	n_dx1 = 0;
	n_dx2 = 0;

	n_y = COORD_Y( m_packet.FlatPolygon.vertex[ n_rightpoint ].n_coord );

	for( ;; )
	{
		if( n_y == COORD_Y( m_packet.FlatPolygon.vertex[ n_leftpoint ].n_coord ) )
		{
			while( n_y == COORD_Y( m_packet.FlatPolygon.vertex[ p_n_leftpointlist[ n_leftpoint ] ].n_coord ) )
			{
				n_leftpoint = p_n_leftpointlist[ n_leftpoint ];
				if( n_leftpoint == n_rightpoint )
				{
					break;
				}
			}
			n_cx1.sw.h = COORD_X( m_packet.FlatPolygon.vertex[ n_leftpoint ].n_coord ); n_cx1.sw.l = 0;
			n_leftpoint = p_n_leftpointlist[ n_leftpoint ];
			n_distance = COORD_Y( m_packet.FlatPolygon.vertex[ n_leftpoint ].n_coord ) - n_y;
			if( n_distance < 1 )
			{
				break;
			}
			n_dx1 = (INT32)( ( COORD_X( m_packet.FlatPolygon.vertex[ n_leftpoint ].n_coord ) << 16 ) - n_cx1.d ) / n_distance;
		}
		if( n_y == COORD_Y( m_packet.FlatPolygon.vertex[ n_rightpoint ].n_coord ) )
		{
			while( n_y == COORD_Y( m_packet.FlatPolygon.vertex[ p_n_rightpointlist[ n_rightpoint ] ].n_coord ) )
			{
				n_rightpoint = p_n_rightpointlist[ n_rightpoint ];
				if( n_rightpoint == n_leftpoint )
				{
					break;
				}
			}
			n_cx2.sw.h = COORD_X( m_packet.FlatPolygon.vertex[ n_rightpoint ].n_coord ); n_cx2.sw.l = 0;
			n_rightpoint = p_n_rightpointlist[ n_rightpoint ];
			n_distance = COORD_Y( m_packet.FlatPolygon.vertex[ n_rightpoint ].n_coord ) - n_y;
			if( n_distance < 1 )
			{
				break;
			}
			n_dx2 = (INT32)( ( COORD_X( m_packet.FlatPolygon.vertex[ n_rightpoint ].n_coord ) << 16 ) - n_cx2.d ) / n_distance;
		}

		int drawy = n_y + n_drawoffset_y;

		if( (INT16)n_cx1.sw.h != (INT16)n_cx2.sw.h && drawy >= (INT32)n_drawarea_y1 && drawy <= (INT32)n_drawarea_y2 )
		{
			if( (INT16)n_cx1.sw.h < (INT16)n_cx2.sw.h )
			{
				n_x = n_cx1.sw.h;
				n_distance = (INT16)n_cx2.sw.h - n_x;
			}
			else
			{
				n_x = n_cx2.sw.h;
				n_distance = (INT16)n_cx1.sw.h - n_x;
			}

			int drawx = n_x + n_drawoffset_x;

			if( ( (INT32)n_drawarea_x1 - drawx ) > 0 )
			{
				n_distance -= ( n_drawarea_x1 - drawx );
				drawx = n_drawarea_x1;
			}

			SOLIDFILL( FLATPOLYGONUPDATE )
		}
		n_cx1.d += n_dx1;
		n_cx2.d += n_dx2;
		n_y++;
	}
}

void psxgpu_device::FlatTexturedPolygon( int n_points )
{
	INT16 n_y;
	INT16 n_x;
	int n_tx;
	int n_ty;

	UINT8 n_cmd;

	UINT32 n_clutx;
	UINT32 n_cluty;

	UINT16 *p_n_f;
	UINT16 *p_n_redb;
	UINT16 *p_n_greenb;
	UINT16 *p_n_blueb;
	UINT16 *p_n_redtrans;
	UINT16 *p_n_greentrans;
	UINT16 *p_n_bluetrans;

	PAIR n_r;
	PAIR n_g;
	PAIR n_b;
	PAIR n_u;
	PAIR n_v;

	PAIR n_cx1;
	PAIR n_cx2;
	PAIR n_cu1;
	PAIR n_cv1;
	PAIR n_cu2;
	PAIR n_cv2;
	INT32 n_du;
	INT32 n_dv;
	INT32 n_dx1;
	INT32 n_dx2;
	INT32 n_du1;
	INT32 n_dv1;
	INT32 n_du2;
	INT32 n_dv2;

	INT32 n_distance;
	UINT16 n_point;
	UINT16 n_rightpoint;
	UINT16 n_leftpoint;
	const UINT16 *p_n_rightpointlist;
	const UINT16 *p_n_leftpointlist;
	UINT16 *p_clut;
	UINT16 *p_vram;
	UINT32 n_bgr;

#if DEBUG_VIEWER
	if( m_debug.n_skip == 2 )
	{
		return;
	}
	for( n_point = 0; n_point < n_points; n_point++ )
	{
		DebugMesh( SINT11( COORD_X( m_packet.FlatTexturedPolygon.vertex[ n_point ].n_coord ) ) + n_drawoffset_x, SINT11( COORD_Y( m_packet.FlatTexturedPolygon.vertex[ n_point ].n_coord ) ) + n_drawoffset_y );
	}
	DebugMeshEnd();
#endif

	n_cmd = BGR_C( m_packet.FlatTexturedPolygon.n_bgr );

	n_clutx = ( m_packet.FlatTexturedPolygon.vertex[ 0 ].n_texture.w.h & 0x3f ) << 4;
	n_cluty = ( m_packet.FlatTexturedPolygon.vertex[ 0 ].n_texture.w.h >> 6 ) & 0x3ff;

	n_r.d = 0;
	n_g.d = 0;
	n_b.d = 0;
	n_cx1.d = 0;
	n_cu1.d = 0;
	n_cv1.d = 0;
	n_cx2.d = 0;
	n_cu2.d = 0;
	n_cv2.d = 0;

	decode_tpage( m_packet.FlatTexturedPolygon.vertex[ 1 ].n_texture.w.h );
	TEXTURESETUP

	switch( n_cmd & 0x01 )
	{
	case 0:
		n_r.w.h = BGR_R( m_packet.FlatTexturedPolygon.n_bgr ); n_r.w.l = 0;
		n_g.w.h = BGR_G( m_packet.FlatTexturedPolygon.n_bgr ); n_g.w.l = 0;
		n_b.w.h = BGR_B( m_packet.FlatTexturedPolygon.n_bgr ); n_b.w.l = 0;
		break;
	case 1:
		n_r.w.h = 0x80; n_r.w.l = 0;
		n_g.w.h = 0x80; n_g.w.l = 0;
		n_b.w.h = 0x80; n_b.w.l = 0;
		break;
	}

	FINDTOPLEFT( FlatTexturedPolygon )

	n_dx1 = 0;
	n_dx2 = 0;
	n_du1 = 0;
	n_du2 = 0;
	n_dv1 = 0;
	n_dv2 = 0;

	n_y = COORD_Y( m_packet.FlatTexturedPolygon.vertex[ n_rightpoint ].n_coord );

	for( ;; )
	{
		if( n_y == COORD_Y( m_packet.FlatTexturedPolygon.vertex[ n_leftpoint ].n_coord ) )
		{
			while( n_y == COORD_Y( m_packet.FlatTexturedPolygon.vertex[ p_n_leftpointlist[ n_leftpoint ] ].n_coord ) )
			{
				n_leftpoint = p_n_leftpointlist[ n_leftpoint ];
				if( n_leftpoint == n_rightpoint )
				{
					break;
				}
			}
			n_cx1.sw.h = COORD_X( m_packet.FlatTexturedPolygon.vertex[ n_leftpoint ].n_coord ); n_cx1.sw.l = 0;
			n_cu1.w.h = TEXTURE_U( m_packet.FlatTexturedPolygon.vertex[ n_leftpoint ].n_texture ); n_cu1.w.l = 0;
			n_cv1.w.h = TEXTURE_V( m_packet.FlatTexturedPolygon.vertex[ n_leftpoint ].n_texture ); n_cv1.w.l = 0;
			n_leftpoint = p_n_leftpointlist[ n_leftpoint ];
			n_distance = COORD_Y( m_packet.FlatTexturedPolygon.vertex[ n_leftpoint ].n_coord ) - n_y;
			if( n_distance < 1 )
			{
				break;
			}
			n_dx1 = (INT32)( ( COORD_X( m_packet.FlatTexturedPolygon.vertex[ n_leftpoint ].n_coord ) << 16 ) - n_cx1.d ) / n_distance;
			n_du1 = (INT32)( ( TEXTURE_U( m_packet.FlatTexturedPolygon.vertex[ n_leftpoint ].n_texture ) << 16 ) - n_cu1.d ) / n_distance;
			n_dv1 = (INT32)( ( TEXTURE_V( m_packet.FlatTexturedPolygon.vertex[ n_leftpoint ].n_texture ) << 16 ) - n_cv1.d ) / n_distance;
		}
		if( n_y == COORD_Y( m_packet.FlatTexturedPolygon.vertex[ n_rightpoint ].n_coord ) )
		{
			while( n_y == COORD_Y( m_packet.FlatTexturedPolygon.vertex[ p_n_rightpointlist[ n_rightpoint ] ].n_coord ) )
			{
				n_rightpoint = p_n_rightpointlist[ n_rightpoint ];
				if( n_rightpoint == n_leftpoint )
				{
					break;
				}
			}
			n_cx2.sw.h = COORD_X( m_packet.FlatTexturedPolygon.vertex[ n_rightpoint ].n_coord ); n_cx2.sw.l = 0;
			n_cu2.w.h = TEXTURE_U( m_packet.FlatTexturedPolygon.vertex[ n_rightpoint ].n_texture ); n_cu2.w.l = 0;
			n_cv2.w.h = TEXTURE_V( m_packet.FlatTexturedPolygon.vertex[ n_rightpoint ].n_texture ); n_cv2.w.l = 0;
			n_rightpoint = p_n_rightpointlist[ n_rightpoint ];
			n_distance = COORD_Y( m_packet.FlatTexturedPolygon.vertex[ n_rightpoint ].n_coord ) - n_y;
			if( n_distance < 1 )
			{
				break;
			}
			n_dx2 = (INT32)( ( COORD_X( m_packet.FlatTexturedPolygon.vertex[ n_rightpoint ].n_coord ) << 16 ) - n_cx2.d ) / n_distance;
			n_du2 = (INT32)( ( TEXTURE_U( m_packet.FlatTexturedPolygon.vertex[ n_rightpoint ].n_texture ) << 16 ) - n_cu2.d ) / n_distance;
			n_dv2 = (INT32)( ( TEXTURE_V( m_packet.FlatTexturedPolygon.vertex[ n_rightpoint ].n_texture ) << 16 ) - n_cv2.d ) / n_distance;
		}

		int drawy = n_y + n_drawoffset_y;

		if( (INT16)n_cx1.sw.h != (INT16)n_cx2.sw.h && drawy >= (INT32)n_drawarea_y1 && drawy <= (INT32)n_drawarea_y2 )
		{
			if( (INT16)n_cx1.sw.h < (INT16)n_cx2.sw.h )
			{
				n_x = n_cx1.sw.h;
				n_distance = (INT16)n_cx2.sw.h - n_x;

				n_u.d = n_cu1.d;
				n_v.d = n_cv1.d;
				n_du = (INT32)( n_cu2.d - n_cu1.d ) / n_distance;
				n_dv = (INT32)( n_cv2.d - n_cv1.d ) / n_distance;
			}
			else
			{
				n_x = n_cx2.sw.h;
				n_distance = (INT16)n_cx1.sw.h - n_x;

				n_u.d = n_cu2.d;
				n_v.d = n_cv2.d;
				n_du = (INT32)( n_cu1.d - n_cu2.d ) / n_distance;
				n_dv = (INT32)( n_cv1.d - n_cv2.d ) / n_distance;
			}

			int drawx = n_x + n_drawoffset_x;

			if( ( (INT32)n_drawarea_x1 - drawx ) > 0 )
			{
				n_u.d += n_du * ( n_drawarea_x1 - drawx );
				n_v.d += n_dv * ( n_drawarea_x1 - drawx );
				n_distance -= ( n_drawarea_x1 - drawx );
				drawx = n_drawarea_x1;
			}

			TEXTUREFILL( FLATTEXTUREDPOLYGONUPDATE, n_u.w.h, n_v.w.h );
		}
		n_cx1.d += n_dx1;
		n_cu1.d += n_du1;
		n_cv1.d += n_dv1;
		n_cx2.d += n_dx2;
		n_cu2.d += n_du2;
		n_cv2.d += n_dv2;
		n_y++;
	}
}

void psxgpu_device::GouraudPolygon( int n_points )
{
	INT16 n_y;
	INT16 n_x;

	UINT16 *p_n_f;
	UINT16 *p_n_redb;
	UINT16 *p_n_greenb;
	UINT16 *p_n_blueb;
	UINT16 *p_n_redtrans;
	UINT16 *p_n_greentrans;
	UINT16 *p_n_bluetrans;

	UINT8 n_cmd;

	PAIR n_r;
	PAIR n_g;
	PAIR n_b;
	PAIR n_cx1;
	PAIR n_cx2;
	PAIR n_cr1;
	PAIR n_cg1;
	PAIR n_cb1;
	PAIR n_cr2;
	PAIR n_cg2;
	PAIR n_cb2;
	INT32 n_dr;
	INT32 n_dg;
	INT32 n_db;
	INT32 n_dx1;
	INT32 n_dx2;
	INT32 n_dr1;
	INT32 n_dg1;
	INT32 n_db1;
	INT32 n_dr2;
	INT32 n_dg2;
	INT32 n_db2;

	INT32 n_distance;

	UINT16 n_point;
	UINT16 n_rightpoint;
	UINT16 n_leftpoint;
	const UINT16 *p_n_rightpointlist;
	const UINT16 *p_n_leftpointlist;

	UINT16 *p_vram;

#if DEBUG_VIEWER
	if( m_debug.n_skip == 3 )
	{
		return;
	}
	for( n_point = 0; n_point < n_points; n_point++ )
	{
		DebugMesh( SINT11( COORD_X( m_packet.GouraudPolygon.vertex[ n_point ].n_coord ) ) + n_drawoffset_x, SINT11( COORD_Y( m_packet.GouraudPolygon.vertex[ n_point ].n_coord ) ) + n_drawoffset_y );
	}
	DebugMeshEnd();
#endif

	n_cmd = BGR_C( m_packet.GouraudPolygon.vertex[ 0 ].n_bgr );

	n_cx1.d = 0;
	n_cr1.d = 0;
	n_cg1.d = 0;
	n_cb1.d = 0;
	n_cx2.d = 0;
	n_cr2.d = 0;
	n_cg2.d = 0;
	n_cb2.d = 0;

	SOLIDSETUP

	FINDTOPLEFT( GouraudPolygon )

	n_dx1 = 0;
	n_dx2 = 0;
	n_dr1 = 0;
	n_dr2 = 0;
	n_dg1 = 0;
	n_dg2 = 0;
	n_db1 = 0;
	n_db2 = 0;

	n_y = COORD_Y( m_packet.GouraudPolygon.vertex[ n_rightpoint ].n_coord );

	for( ;; )
	{
		if( n_y == COORD_Y( m_packet.GouraudPolygon.vertex[ n_leftpoint ].n_coord ) )
		{
			while( n_y == COORD_Y( m_packet.GouraudPolygon.vertex[ p_n_leftpointlist[ n_leftpoint ] ].n_coord ) )
			{
				n_leftpoint = p_n_leftpointlist[ n_leftpoint ];
				if( n_leftpoint == n_rightpoint )
				{
					break;
				}
			}
			n_cx1.sw.h = COORD_X( m_packet.GouraudPolygon.vertex[ n_leftpoint ].n_coord ); n_cx1.sw.l = 0;
			n_cr1.w.h = BGR_R( m_packet.GouraudPolygon.vertex[ n_leftpoint ].n_bgr ); n_cr1.w.l = 0;
			n_cg1.w.h = BGR_G( m_packet.GouraudPolygon.vertex[ n_leftpoint ].n_bgr ); n_cg1.w.l = 0;
			n_cb1.w.h = BGR_B( m_packet.GouraudPolygon.vertex[ n_leftpoint ].n_bgr ); n_cb1.w.l = 0;
			n_leftpoint = p_n_leftpointlist[ n_leftpoint ];
			n_distance = COORD_Y( m_packet.GouraudPolygon.vertex[ n_leftpoint ].n_coord ) - n_y;
			if( n_distance < 1 )
			{
				break;
			}
			n_dx1 = (INT32)( ( COORD_X( m_packet.GouraudPolygon.vertex[ n_leftpoint ].n_coord ) << 16 ) - n_cx1.d ) / n_distance;
			n_dr1 = (INT32)( ( BGR_R( m_packet.GouraudPolygon.vertex[ n_leftpoint ].n_bgr ) << 16 ) - n_cr1.d ) / n_distance;
			n_dg1 = (INT32)( ( BGR_G( m_packet.GouraudPolygon.vertex[ n_leftpoint ].n_bgr ) << 16 ) - n_cg1.d ) / n_distance;
			n_db1 = (INT32)( ( BGR_B( m_packet.GouraudPolygon.vertex[ n_leftpoint ].n_bgr ) << 16 ) - n_cb1.d ) / n_distance;
		}
		if( n_y == COORD_Y( m_packet.GouraudPolygon.vertex[ n_rightpoint ].n_coord ) )
		{
			while( n_y == COORD_Y( m_packet.GouraudPolygon.vertex[ p_n_rightpointlist[ n_rightpoint ] ].n_coord ) )
			{
				n_rightpoint = p_n_rightpointlist[ n_rightpoint ];
				if( n_rightpoint == n_leftpoint )
				{
					break;
				}
			}
			n_cx2.sw.h = COORD_X( m_packet.GouraudPolygon.vertex[ n_rightpoint ].n_coord ); n_cx2.sw.l = 0;
			n_cr2.w.h = BGR_R( m_packet.GouraudPolygon.vertex[ n_rightpoint ].n_bgr ); n_cr2.w.l = 0;
			n_cg2.w.h = BGR_G( m_packet.GouraudPolygon.vertex[ n_rightpoint ].n_bgr ); n_cg2.w.l = 0;
			n_cb2.w.h = BGR_B( m_packet.GouraudPolygon.vertex[ n_rightpoint ].n_bgr ); n_cb2.w.l = 0;
			n_rightpoint = p_n_rightpointlist[ n_rightpoint ];
			n_distance = COORD_Y( m_packet.GouraudPolygon.vertex[ n_rightpoint ].n_coord ) - n_y;
			if( n_distance < 1 )
			{
				break;
			}
			n_dx2 = (INT32)( ( COORD_X( m_packet.GouraudPolygon.vertex[ n_rightpoint ].n_coord ) << 16 ) - n_cx2.d ) / n_distance;
			n_dr2 = (INT32)( ( BGR_R( m_packet.GouraudPolygon.vertex[ n_rightpoint ].n_bgr ) << 16 ) - n_cr2.d ) / n_distance;
			n_dg2 = (INT32)( ( BGR_G( m_packet.GouraudPolygon.vertex[ n_rightpoint ].n_bgr ) << 16 ) - n_cg2.d ) / n_distance;
			n_db2 = (INT32)( ( BGR_B( m_packet.GouraudPolygon.vertex[ n_rightpoint ].n_bgr ) << 16 ) - n_cb2.d ) / n_distance;
		}

		int drawy = n_y + n_drawoffset_y;

		if( (INT16)n_cx1.sw.h != (INT16)n_cx2.sw.h && drawy >= (INT32)n_drawarea_y1 && drawy <= (INT32)n_drawarea_y2 )
		{
			if( (INT16)n_cx1.sw.h < (INT16)n_cx2.sw.h )
			{
				n_x = n_cx1.sw.h;
				n_distance = (INT16)n_cx2.sw.h - n_x;

				n_r.d = n_cr1.d;
				n_g.d = n_cg1.d;
				n_b.d = n_cb1.d;
				n_dr = (INT32)( n_cr2.d - n_cr1.d ) / n_distance;
				n_dg = (INT32)( n_cg2.d - n_cg1.d ) / n_distance;
				n_db = (INT32)( n_cb2.d - n_cb1.d ) / n_distance;
			}
			else
			{
				n_x = n_cx2.sw.h;
				n_distance = (INT16)n_cx1.sw.h - n_x;

				n_r.d = n_cr2.d;
				n_g.d = n_cg2.d;
				n_b.d = n_cb2.d;
				n_dr = (INT32)( n_cr1.d - n_cr2.d ) / n_distance;
				n_dg = (INT32)( n_cg1.d - n_cg2.d ) / n_distance;
				n_db = (INT32)( n_cb1.d - n_cb2.d ) / n_distance;
			}

			int drawx = n_x + n_drawoffset_x;

			if( ( (INT32)n_drawarea_x1 - drawx ) > 0 )
			{
				n_r.d += n_dr * ( n_drawarea_x1 - drawx );
				n_g.d += n_dg * ( n_drawarea_x1 - drawx );
				n_b.d += n_db * ( n_drawarea_x1 - drawx );
				n_distance -= ( n_drawarea_x1 - drawx );
				drawx = n_drawarea_x1;
			}

			SOLIDFILL( GOURAUDPOLYGONUPDATE )
		}
		n_cx1.d += n_dx1;
		n_cr1.d += n_dr1;
		n_cg1.d += n_dg1;
		n_cb1.d += n_db1;
		n_cx2.d += n_dx2;
		n_cr2.d += n_dr2;
		n_cg2.d += n_dg2;
		n_cb2.d += n_db2;
		n_y++;
	}
}

void psxgpu_device::GouraudTexturedPolygon( int n_points )
{
	INT16 n_y;
	INT16 n_x;
	int n_tx;
	int n_ty;

	UINT8 n_cmd;

	UINT32 n_clutx;
	UINT32 n_cluty;

	UINT16 *p_n_f;
	UINT16 *p_n_redb;
	UINT16 *p_n_greenb;
	UINT16 *p_n_blueb;
	UINT16 *p_n_redtrans;
	UINT16 *p_n_greentrans;
	UINT16 *p_n_bluetrans;

	PAIR n_r;
	PAIR n_g;
	PAIR n_b;
	PAIR n_u;
	PAIR n_v;

	PAIR n_cx1;
	PAIR n_cx2;
	PAIR n_cu1;
	PAIR n_cv1;
	PAIR n_cu2;
	PAIR n_cv2;
	PAIR n_cr1;
	PAIR n_cg1;
	PAIR n_cb1;
	PAIR n_cr2;
	PAIR n_cg2;
	PAIR n_cb2;
	INT32 n_dr;
	INT32 n_dg;
	INT32 n_db;
	INT32 n_du;
	INT32 n_dv;
	INT32 n_dx1;
	INT32 n_dx2;
	INT32 n_dr1;
	INT32 n_dg1;
	INT32 n_db1;
	INT32 n_dr2;
	INT32 n_dg2;
	INT32 n_db2;
	INT32 n_du1;
	INT32 n_dv1;
	INT32 n_du2;
	INT32 n_dv2;

	INT32 n_distance;
	UINT16 n_point;
	UINT16 n_rightpoint;
	UINT16 n_leftpoint;
	const UINT16 *p_n_rightpointlist;
	const UINT16 *p_n_leftpointlist;
	UINT16 *p_clut;
	UINT16 *p_vram;
	UINT32 n_bgr;

#if DEBUG_VIEWER
	if( m_debug.n_skip == 4 )
	{
		return;
	}
	for( n_point = 0; n_point < n_points; n_point++ )
	{
		DebugMesh( SINT11( COORD_X( m_packet.GouraudTexturedPolygon.vertex[ n_point ].n_coord ) ) + n_drawoffset_x, SINT11( COORD_Y( m_packet.GouraudTexturedPolygon.vertex[ n_point ].n_coord ) ) + n_drawoffset_y );
	}
	DebugMeshEnd();
#endif

	n_cmd = BGR_C( m_packet.GouraudTexturedPolygon.vertex[ 0 ].n_bgr );

	n_clutx = ( m_packet.GouraudTexturedPolygon.vertex[ 0 ].n_texture.w.h & 0x3f ) << 4;
	n_cluty = ( m_packet.GouraudTexturedPolygon.vertex[ 0 ].n_texture.w.h >> 6 ) & 0x3ff;

	n_cx1.d = 0;
	n_cr1.d = 0;
	n_cg1.d = 0;
	n_cb1.d = 0;
	n_cu1.d = 0;
	n_cv1.d = 0;
	n_cx2.d = 0;
	n_cr2.d = 0;
	n_cg2.d = 0;
	n_cb2.d = 0;
	n_cu2.d = 0;
	n_cv2.d = 0;

	decode_tpage( m_packet.GouraudTexturedPolygon.vertex[ 1 ].n_texture.w.h );
	TEXTURESETUP

	FINDTOPLEFT( GouraudTexturedPolygon )

	n_dx1 = 0;
	n_dx2 = 0;
	n_du1 = 0;
	n_du2 = 0;
	n_dr1 = 0;
	n_dr2 = 0;
	n_dg1 = 0;
	n_dg2 = 0;
	n_db1 = 0;
	n_db2 = 0;
	n_dv1 = 0;
	n_dv2 = 0;

	n_y = COORD_Y( m_packet.GouraudTexturedPolygon.vertex[ n_rightpoint ].n_coord );

	for( ;; )
	{
		if( n_y == COORD_Y( m_packet.GouraudTexturedPolygon.vertex[ n_leftpoint ].n_coord ) )
		{
			while( n_y == COORD_Y( m_packet.GouraudTexturedPolygon.vertex[ p_n_leftpointlist[ n_leftpoint ] ].n_coord ) )
			{
				n_leftpoint = p_n_leftpointlist[ n_leftpoint ];
				if( n_leftpoint == n_rightpoint )
				{
					break;
				}
			}
			n_cx1.sw.h = COORD_X( m_packet.GouraudTexturedPolygon.vertex[ n_leftpoint ].n_coord ); n_cx1.sw.l = 0;
			switch( n_cmd & 0x01 )
			{
			case 0x00:
				n_cr1.w.h = BGR_R( m_packet.GouraudTexturedPolygon.vertex[ n_leftpoint ].n_bgr ); n_cr1.w.l = 0;
				n_cg1.w.h = BGR_G( m_packet.GouraudTexturedPolygon.vertex[ n_leftpoint ].n_bgr ); n_cg1.w.l = 0;
				n_cb1.w.h = BGR_B( m_packet.GouraudTexturedPolygon.vertex[ n_leftpoint ].n_bgr ); n_cb1.w.l = 0;
				break;
			case 0x01:
				n_cr1.w.h = 0x80; n_cr1.w.l = 0;
				n_cg1.w.h = 0x80; n_cg1.w.l = 0;
				n_cb1.w.h = 0x80; n_cb1.w.l = 0;
				break;
			}
			n_cu1.w.h = TEXTURE_U( m_packet.GouraudTexturedPolygon.vertex[ n_leftpoint ].n_texture ); n_cu1.w.l = 0;
			n_cv1.w.h = TEXTURE_V( m_packet.GouraudTexturedPolygon.vertex[ n_leftpoint ].n_texture ); n_cv1.w.l = 0;
			n_leftpoint = p_n_leftpointlist[ n_leftpoint ];
			n_distance = COORD_Y( m_packet.GouraudTexturedPolygon.vertex[ n_leftpoint ].n_coord ) - n_y;
			if( n_distance < 1 )
			{
				break;
			}
			n_dx1 = (INT32)( ( COORD_X( m_packet.GouraudTexturedPolygon.vertex[ n_leftpoint ].n_coord ) << 16 ) - n_cx1.d ) / n_distance;
			switch( n_cmd & 0x01 )
			{
			case 0x00:
				n_dr1 = (INT32)( ( BGR_R( m_packet.GouraudTexturedPolygon.vertex[ n_leftpoint ].n_bgr ) << 16 ) - n_cr1.d ) / n_distance;
				n_dg1 = (INT32)( ( BGR_G( m_packet.GouraudTexturedPolygon.vertex[ n_leftpoint ].n_bgr ) << 16 ) - n_cg1.d ) / n_distance;
				n_db1 = (INT32)( ( BGR_B( m_packet.GouraudTexturedPolygon.vertex[ n_leftpoint ].n_bgr ) << 16 ) - n_cb1.d ) / n_distance;
				break;
			case 0x01:
				n_dr1 = 0;
				n_dg1 = 0;
				n_db1 = 0;
				break;
			}
			n_du1 = (INT32)( ( TEXTURE_U( m_packet.GouraudTexturedPolygon.vertex[ n_leftpoint ].n_texture ) << 16 ) - n_cu1.d ) / n_distance;
			n_dv1 = (INT32)( ( TEXTURE_V( m_packet.GouraudTexturedPolygon.vertex[ n_leftpoint ].n_texture ) << 16 ) - n_cv1.d ) / n_distance;
		}
		if( n_y == COORD_Y( m_packet.GouraudTexturedPolygon.vertex[ n_rightpoint ].n_coord ) )
		{
			while( n_y == COORD_Y( m_packet.GouraudTexturedPolygon.vertex[ p_n_rightpointlist[ n_rightpoint ] ].n_coord ) )
			{
				n_rightpoint = p_n_rightpointlist[ n_rightpoint ];
				if( n_rightpoint == n_leftpoint )
				{
					break;
				}
			}
			n_cx2.sw.h = COORD_X( m_packet.GouraudTexturedPolygon.vertex[ n_rightpoint ].n_coord ); n_cx2.sw.l = 0;
			switch( n_cmd & 0x01 )
			{
			case 0x00:
				n_cr2.w.h = BGR_R( m_packet.GouraudTexturedPolygon.vertex[ n_rightpoint ].n_bgr ); n_cr2.w.l = 0;
				n_cg2.w.h = BGR_G( m_packet.GouraudTexturedPolygon.vertex[ n_rightpoint ].n_bgr ); n_cg2.w.l = 0;
				n_cb2.w.h = BGR_B( m_packet.GouraudTexturedPolygon.vertex[ n_rightpoint ].n_bgr ); n_cb2.w.l = 0;
				break;
			case 0x01:
				n_cr2.w.h = 0x80; n_cr2.w.l = 0;
				n_cg2.w.h = 0x80; n_cg2.w.l = 0;
				n_cb2.w.h = 0x80; n_cb2.w.l = 0;
				break;
			}
			n_cu2.w.h = TEXTURE_U( m_packet.GouraudTexturedPolygon.vertex[ n_rightpoint ].n_texture ); n_cu2.w.l = 0;
			n_cv2.w.h = TEXTURE_V( m_packet.GouraudTexturedPolygon.vertex[ n_rightpoint ].n_texture ); n_cv2.w.l = 0;
			n_rightpoint = p_n_rightpointlist[ n_rightpoint ];
			n_distance = COORD_Y( m_packet.GouraudTexturedPolygon.vertex[ n_rightpoint ].n_coord ) - n_y;
			if( n_distance < 1 )
			{
				break;
			}
			n_dx2 = (INT32)( ( COORD_X( m_packet.GouraudTexturedPolygon.vertex[ n_rightpoint ].n_coord ) << 16 ) - n_cx2.d ) / n_distance;
			switch( n_cmd & 0x01 )
			{
			case 0x00:
				n_dr2 = (INT32)( ( BGR_R( m_packet.GouraudTexturedPolygon.vertex[ n_rightpoint ].n_bgr ) << 16 ) - n_cr2.d ) / n_distance;
				n_dg2 = (INT32)( ( BGR_G( m_packet.GouraudTexturedPolygon.vertex[ n_rightpoint ].n_bgr ) << 16 ) - n_cg2.d ) / n_distance;
				n_db2 = (INT32)( ( BGR_B( m_packet.GouraudTexturedPolygon.vertex[ n_rightpoint ].n_bgr ) << 16 ) - n_cb2.d ) / n_distance;
				break;
			case 0x01:
				n_dr2 = 0;
				n_dg2 = 0;
				n_db2 = 0;
				break;
			}
			n_du2 = (INT32)( ( TEXTURE_U( m_packet.GouraudTexturedPolygon.vertex[ n_rightpoint ].n_texture ) << 16 ) - n_cu2.d ) / n_distance;
			n_dv2 = (INT32)( ( TEXTURE_V( m_packet.GouraudTexturedPolygon.vertex[ n_rightpoint ].n_texture ) << 16 ) - n_cv2.d ) / n_distance;
		}

		int drawy = n_y + n_drawoffset_y;

		if( (INT16)n_cx1.sw.h != (INT16)n_cx2.sw.h && drawy >= (INT32)n_drawarea_y1 && drawy <= (INT32)n_drawarea_y2 )
		{
			if( (INT16)n_cx1.sw.h < (INT16)n_cx2.sw.h )
			{
				n_x = n_cx1.sw.h;
				n_distance = (INT16)n_cx2.sw.h - n_x;

				n_r.d = n_cr1.d;
				n_g.d = n_cg1.d;
				n_b.d = n_cb1.d;
				n_u.d = n_cu1.d;
				n_v.d = n_cv1.d;
				n_dr = (INT32)( n_cr2.d - n_cr1.d ) / n_distance;
				n_dg = (INT32)( n_cg2.d - n_cg1.d ) / n_distance;
				n_db = (INT32)( n_cb2.d - n_cb1.d ) / n_distance;
				n_du = (INT32)( n_cu2.d - n_cu1.d ) / n_distance;
				n_dv = (INT32)( n_cv2.d - n_cv1.d ) / n_distance;
			}
			else
			{
				n_x = n_cx2.sw.h;
				n_distance = (INT16)n_cx1.sw.h - n_x;

				n_r.d = n_cr2.d;
				n_g.d = n_cg2.d;
				n_b.d = n_cb2.d;
				n_u.d = n_cu2.d;
				n_v.d = n_cv2.d;
				n_dr = (INT32)( n_cr1.d - n_cr2.d ) / n_distance;
				n_dg = (INT32)( n_cg1.d - n_cg2.d ) / n_distance;
				n_db = (INT32)( n_cb1.d - n_cb2.d ) / n_distance;
				n_du = (INT32)( n_cu1.d - n_cu2.d ) / n_distance;
				n_dv = (INT32)( n_cv1.d - n_cv2.d ) / n_distance;
			}

			int drawx = n_x + n_drawoffset_x;

			if( ( (INT32)n_drawarea_x1 - drawx ) > 0 )
			{
				n_r.d += n_dr * ( n_drawarea_x1 - drawx );
				n_g.d += n_dg * ( n_drawarea_x1 - drawx );
				n_b.d += n_db * ( n_drawarea_x1 - drawx );
				n_u.d += n_du * ( n_drawarea_x1 - drawx );
				n_v.d += n_dv * ( n_drawarea_x1 - drawx );
				n_distance -= ( n_drawarea_x1 - drawx );
				drawx = n_drawarea_x1;
			}

			TEXTUREFILL( GOURAUDTEXTUREDPOLYGONUPDATE, n_u.w.h, n_v.w.h );
		}
		n_cx1.d += n_dx1;
		n_cr1.d += n_dr1;
		n_cg1.d += n_dg1;
		n_cb1.d += n_db1;
		n_cu1.d += n_du1;
		n_cv1.d += n_dv1;
		n_cx2.d += n_dx2;
		n_cr2.d += n_dr2;
		n_cg2.d += n_dg2;
		n_cb2.d += n_db2;
		n_cu2.d += n_du2;
		n_cv2.d += n_dv2;
		n_y++;
	}
}

void psxgpu_device::MonochromeLine( void )
{
	PAIR n_x;
	PAIR n_y;
	INT32 n_dx;
	INT32 n_dy;
	INT32 n_len;
	INT32 n_xlen;
	INT32 n_ylen;
	INT32 n_xstart;
	INT32 n_ystart;
	INT32 n_xend;
	INT32 n_yend;
	UINT32 n_r;
	UINT32 n_g;
	UINT32 n_b;
	UINT16 *p_vram;

#if DEBUG_VIEWER
	if( m_debug.n_skip == 5 )
	{
		return;
	}
	DebugMesh( SINT11( COORD_X( m_packet.MonochromeLine.vertex[ 0 ].n_coord ) ) + n_drawoffset_x, SINT11( COORD_Y( m_packet.MonochromeLine.vertex[ 0 ].n_coord ) ) + n_drawoffset_y );
	DebugMesh( SINT11( COORD_X( m_packet.MonochromeLine.vertex[ 1 ].n_coord ) ) + n_drawoffset_x, SINT11( COORD_Y( m_packet.MonochromeLine.vertex[ 1 ].n_coord ) ) + n_drawoffset_y );
	DebugMeshEnd();
#endif

	n_xstart = SINT11( COORD_X( m_packet.MonochromeLine.vertex[ 0 ].n_coord ) );
	n_xend = SINT11( COORD_X( m_packet.MonochromeLine.vertex[ 1 ].n_coord ) );
	n_ystart = SINT11( COORD_Y( m_packet.MonochromeLine.vertex[ 0 ].n_coord ) );
	n_yend = SINT11( COORD_Y( m_packet.MonochromeLine.vertex[ 1 ].n_coord ) );

	n_r = BGR_R( m_packet.MonochromeLine.n_bgr );
	n_g = BGR_G( m_packet.MonochromeLine.n_bgr );
	n_b = BGR_B( m_packet.MonochromeLine.n_bgr );

	if( n_xend > n_xstart )
	{
		n_xlen = n_xend - n_xstart;
	}
	else
	{
		n_xlen = n_xstart - n_xend;
	}

	if( n_yend > n_ystart )
	{
		n_ylen = n_yend - n_ystart;
	}
	else
	{
		n_ylen = n_ystart - n_yend;
	}

	if( n_xlen > n_ylen )
	{
		n_len = n_xlen;
	}
	else
	{
		n_len = n_ylen;
	}

	if( n_len == 0 )
	{
		n_len = 1;
	}

	n_x.sw.h = n_xstart; n_x.sw.l = 0;
	n_y.sw.h = n_ystart; n_y.sw.l = 0;

	n_dx = (INT32)( ( n_xend << 16 ) - n_x.d ) / n_len;
	n_dy = (INT32)( ( n_yend << 16 ) - n_y.d ) / n_len;

	while( n_len > 0 )
	{
		int drawx = n_x.sw.h + n_drawoffset_x;
		int drawy = n_y.sw.h + n_drawoffset_y;

		if( drawx >= (INT32)n_drawarea_x1 && drawy >= (INT32)n_drawarea_y1 &&
			drawx <= (INT32)n_drawarea_x2 && drawy <= (INT32)n_drawarea_y2 )
		{
			p_vram = p_p_vram[ drawy ] + drawx;
			WRITE_PIXEL(
				p_n_redshade[ MID_LEVEL | n_r ] |
				p_n_greenshade[ MID_LEVEL | n_g ] |
				p_n_blueshade[ MID_LEVEL | n_b ] );
		}
		n_x.d += n_dx;
		n_y.d += n_dy;
		n_len--;
	}
}

void psxgpu_device::GouraudLine( void )
{
	PAIR n_x;
	PAIR n_y;
	INT32 n_dx;
	INT32 n_dy;
	INT32 n_dr;
	INT32 n_dg;
	INT32 n_db;
	INT32 n_distance;
	INT32 n_xlen;
	INT32 n_ylen;
	INT32 n_xstart;
	INT32 n_ystart;
	INT32 n_xend;
	INT32 n_yend;
	PAIR n_r;
	PAIR n_g;
	PAIR n_b;
	PAIR n_cr1;
	PAIR n_cg1;
	PAIR n_cb1;
	PAIR n_cr2;
	PAIR n_cg2;
	PAIR n_cb2;
	UINT16 *p_vram;

#if DEBUG_VIEWER
	if( m_debug.n_skip == 6 )
	{
		return;
	}
	DebugMesh( SINT11( COORD_X( m_packet.GouraudLine.vertex[ 0 ].n_coord ) ) + n_drawoffset_x, SINT11( COORD_Y( m_packet.GouraudLine.vertex[ 0 ].n_coord ) ) + n_drawoffset_y );
	DebugMesh( SINT11( COORD_X( m_packet.GouraudLine.vertex[ 1 ].n_coord ) ) + n_drawoffset_x, SINT11( COORD_Y( m_packet.GouraudLine.vertex[ 1 ].n_coord ) ) + n_drawoffset_y );
	DebugMeshEnd();
#endif

	n_xstart = SINT11( COORD_X( m_packet.GouraudLine.vertex[ 0 ].n_coord ) );
	n_ystart = SINT11( COORD_Y( m_packet.GouraudLine.vertex[ 0 ].n_coord ) );
	n_cr1.w.h = BGR_R( m_packet.GouraudLine.vertex[ 0 ].n_bgr ); n_cr1.w.l = 0;
	n_cg1.w.h = BGR_G( m_packet.GouraudLine.vertex[ 0 ].n_bgr ); n_cg1.w.l = 0;
	n_cb1.w.h = BGR_B( m_packet.GouraudLine.vertex[ 0 ].n_bgr ); n_cb1.w.l = 0;

	n_xend = SINT11( COORD_X( m_packet.GouraudLine.vertex[ 1 ].n_coord ) );
	n_yend = SINT11( COORD_Y( m_packet.GouraudLine.vertex[ 1 ].n_coord ) );
	n_cr2.w.h = BGR_R( m_packet.GouraudLine.vertex[ 1 ].n_bgr ); n_cr1.w.l = 0;
	n_cg2.w.h = BGR_G( m_packet.GouraudLine.vertex[ 1 ].n_bgr ); n_cg1.w.l = 0;
	n_cb2.w.h = BGR_B( m_packet.GouraudLine.vertex[ 1 ].n_bgr ); n_cb1.w.l = 0;

	n_x.sw.h = n_xstart; n_x.sw.l = 0;
	n_y.sw.h = n_ystart; n_y.sw.l = 0;
	n_r.d = n_cr1.d;
	n_g.d = n_cg1.d;
	n_b.d = n_cb1.d;

	if( n_xend > n_xstart )
	{
		n_xlen = n_xend - n_xstart;
	}
	else
	{
		n_xlen = n_xstart - n_xend;
	}

	if( n_yend > n_ystart )
	{
		n_ylen = n_yend - n_ystart;
	}
	else
	{
		n_ylen = n_ystart - n_yend;
	}

	if( n_xlen > n_ylen )
	{
		n_distance = n_xlen;
	}
	else
	{
		n_distance = n_ylen;
	}

	if( n_distance == 0 )
	{
		n_distance = 1;
	}

	n_dx = (INT32)( ( n_xend << 16 ) - n_x.sd ) / n_distance;
	n_dy = (INT32)( ( n_yend << 16 ) - n_y.sd ) / n_distance;
	n_dr = (INT32)( n_cr2.d - n_cr1.d ) / n_distance;
	n_dg = (INT32)( n_cg2.d - n_cg1.d ) / n_distance;
	n_db = (INT32)( n_cb2.d - n_cb1.d ) / n_distance;

	while( n_distance > 0 )
	{
		int drawx = n_x.sw.h + n_drawoffset_x;
		int drawy = n_y.sw.h + n_drawoffset_y;

		if( drawx >= (INT32)n_drawarea_x1 && drawy >= (INT32)n_drawarea_y1 &&
			drawx <= (INT32)n_drawarea_x2 && drawy <= (INT32)n_drawarea_y2 )
		{
			p_vram = p_p_vram[ drawy ] + drawx;
			WRITE_PIXEL(
				p_n_redshade[ MID_LEVEL | n_r.w.h ] |
				p_n_greenshade[ MID_LEVEL | n_g.w.h ] |
				p_n_blueshade[ MID_LEVEL | n_b.w.h ] );
		}
		n_x.sd += n_dx;
		n_y.sd += n_dy;
		n_r.d += n_dr;
		n_g.d += n_dg;
		n_b.d += n_db;
		n_distance--;
	}
}

void psxgpu_device::FrameBufferRectangleDraw( void )
{
	PAIR n_r;
	PAIR n_g;
	PAIR n_b;
	INT32 n_distance;
	INT32 n_h;
	INT16 n_y;
	INT16 n_x;
	UINT16 *p_vram;

#if DEBUG_VIEWER
	if( m_debug.n_skip == 7 )
	{
		return;
	}
	DebugMesh( SINT11( COORD_X( m_packet.FlatRectangle.n_coord ) ), SINT11( COORD_Y( m_packet.FlatRectangle.n_coord ) ) );
	DebugMesh( SINT11( COORD_X( m_packet.FlatRectangle.n_coord ) ) + SIZE_W( m_packet.FlatRectangle.n_size ), SINT11( COORD_Y( m_packet.FlatRectangle.n_coord ) ) );
	DebugMesh( SINT11( COORD_X( m_packet.FlatRectangle.n_coord ) ), SINT11( COORD_Y( m_packet.FlatRectangle.n_coord ) ) + SIZE_H( m_packet.FlatRectangle.n_size ) );
	DebugMesh( SINT11( COORD_X( m_packet.FlatRectangle.n_coord ) ) + SIZE_W( m_packet.FlatRectangle.n_size ), SINT11( COORD_Y( m_packet.FlatRectangle.n_coord ) ) + SIZE_H( m_packet.FlatRectangle.n_size ) );
	DebugMeshEnd();
#endif

	n_r.w.h = BGR_R( m_packet.FlatRectangle.n_bgr ); n_r.w.l = 0;
	n_g.w.h = BGR_G( m_packet.FlatRectangle.n_bgr ); n_g.w.l = 0;
	n_b.w.h = BGR_B( m_packet.FlatRectangle.n_bgr ); n_b.w.l = 0;

	n_y = COORD_Y( m_packet.FlatRectangle.n_coord );
	n_h = SIZE_H( m_packet.FlatRectangle.n_size );

	while( n_h > 0 )
	{
		n_x = COORD_X( m_packet.FlatRectangle.n_coord );

		n_distance = SIZE_W( m_packet.FlatRectangle.n_size );
		while( n_distance > 0 )
		{
			p_vram = p_p_vram[ n_y & 1023 ] + ( n_x & 1023 );
			WRITE_PIXEL(
				p_n_redshade[ MID_LEVEL | n_r.w.h ] |
				p_n_greenshade[ MID_LEVEL | n_g.w.h ] |
				p_n_blueshade[ MID_LEVEL | n_b.w.h ] );
			n_x++;
			n_distance--;
		}
		n_y++;
		n_h--;
	}
}

void psxgpu_device::FlatRectangle( void )
{
	INT16 n_y;
	INT16 n_x;

	UINT8 n_cmd;

	UINT16 *p_n_f;
	UINT16 *p_n_redb;
	UINT16 *p_n_greenb;
	UINT16 *p_n_blueb;
	UINT16 *p_n_redtrans;
	UINT16 *p_n_greentrans;
	UINT16 *p_n_bluetrans;

	PAIR n_r;
	PAIR n_g;
	PAIR n_b;

	INT32 n_distance;
	INT32 n_h;
	UINT16 *p_vram;

#if DEBUG_VIEWER
	if( m_debug.n_skip == 8 )
	{
		return;
	}
	DebugMesh( SINT11( COORD_X( m_packet.FlatRectangle.n_coord ) ) + n_drawoffset_x, SINT11( COORD_Y( m_packet.FlatRectangle.n_coord ) ) + n_drawoffset_y );
	DebugMesh( SINT11( COORD_X( m_packet.FlatRectangle.n_coord ) ) + n_drawoffset_x + SIZE_W( m_packet.FlatRectangle.n_size ), SINT11( COORD_Y( m_packet.FlatRectangle.n_coord ) ) + n_drawoffset_y );
	DebugMesh( SINT11( COORD_X( m_packet.FlatRectangle.n_coord ) ) + n_drawoffset_x, SINT11( COORD_Y( m_packet.FlatRectangle.n_coord ) ) + n_drawoffset_y + SIZE_H( m_packet.FlatRectangle.n_size ) );
	DebugMesh( SINT11( COORD_X( m_packet.FlatRectangle.n_coord ) ) + n_drawoffset_x + SIZE_W( m_packet.FlatRectangle.n_size ), SINT11( COORD_Y( m_packet.FlatRectangle.n_coord ) ) + n_drawoffset_y + SIZE_H( m_packet.FlatRectangle.n_size ) );
	DebugMeshEnd();
#endif

	n_cmd = BGR_C( m_packet.FlatRectangle.n_bgr );

	SOLIDSETUP

	n_r.w.h = BGR_R( m_packet.FlatRectangle.n_bgr ); n_r.w.l = 0;
	n_g.w.h = BGR_G( m_packet.FlatRectangle.n_bgr ); n_g.w.l = 0;
	n_b.w.h = BGR_B( m_packet.FlatRectangle.n_bgr ); n_b.w.l = 0;

	n_x = SINT11( COORD_X( m_packet.FlatRectangle.n_coord ) );
	n_y = SINT11( COORD_Y( m_packet.FlatRectangle.n_coord ) );
	n_h = SIZE_H( m_packet.FlatRectangle.n_size );

	while( n_h > 0 )
	{
		n_distance = SIZE_W( m_packet.FlatRectangle.n_size );

		int drawy = n_y + n_drawoffset_y;

		if( n_distance > 0 && drawy >= (INT32)n_drawarea_y1 && drawy <= (INT32)n_drawarea_y2 )
		{
			int drawx = n_x + n_drawoffset_x;

			if( ( (INT32)n_drawarea_x1 - drawx ) > 0 )
			{
				n_distance -= ( n_drawarea_x1 - drawx );
				drawx = n_drawarea_x1;
			}

			SOLIDFILL( FLATRECTANGEUPDATE )
		}
		n_y++;
		n_h--;
	}
}

void psxgpu_device::FlatRectangle8x8( void )
{
	INT16 n_y;
	INT16 n_x;

	UINT8 n_cmd;

	UINT16 *p_n_f;
	UINT16 *p_n_redb;
	UINT16 *p_n_greenb;
	UINT16 *p_n_blueb;
	UINT16 *p_n_redtrans;
	UINT16 *p_n_greentrans;
	UINT16 *p_n_bluetrans;

	PAIR n_r;
	PAIR n_g;
	PAIR n_b;

	INT32 n_distance;
	INT32 n_h;
	UINT16 *p_vram;

#if DEBUG_VIEWER
	if( m_debug.n_skip == 9 )
	{
		return;
	}
	DebugMesh( SINT11( COORD_X( m_packet.FlatRectangle8x8.n_coord ) ) + n_drawoffset_x, SINT11( COORD_Y( m_packet.FlatRectangle8x8.n_coord ) ) + n_drawoffset_y );
	DebugMesh( SINT11( COORD_X( m_packet.FlatRectangle8x8.n_coord ) ) + n_drawoffset_x + 8, SINT11( COORD_Y( m_packet.FlatRectangle8x8.n_coord ) ) + n_drawoffset_y );
	DebugMesh( SINT11( COORD_X( m_packet.FlatRectangle8x8.n_coord ) ) + n_drawoffset_x, SINT11( COORD_Y( m_packet.FlatRectangle8x8.n_coord ) ) + n_drawoffset_y + 8 );
	DebugMesh( SINT11( COORD_X( m_packet.FlatRectangle8x8.n_coord ) ) + n_drawoffset_x + 8, SINT11( COORD_Y( m_packet.FlatRectangle8x8.n_coord ) ) + n_drawoffset_y + 8 );
	DebugMeshEnd();
#endif

	n_cmd = BGR_C( m_packet.FlatRectangle8x8.n_bgr );

	SOLIDSETUP

	n_r.w.h = BGR_R( m_packet.FlatRectangle8x8.n_bgr ); n_r.w.l = 0;
	n_g.w.h = BGR_G( m_packet.FlatRectangle8x8.n_bgr ); n_g.w.l = 0;
	n_b.w.h = BGR_B( m_packet.FlatRectangle8x8.n_bgr ); n_b.w.l = 0;

	n_x = SINT11( COORD_X( m_packet.FlatRectangle8x8.n_coord ) );
	n_y = SINT11( COORD_Y( m_packet.FlatRectangle8x8.n_coord ) );
	n_h = 8;

	while( n_h > 0 )
	{
		n_distance = 8;

		int drawy = n_y + n_drawoffset_y;

		if( n_distance > 0 && drawy >= (INT32)n_drawarea_y1 && drawy <= (INT32)n_drawarea_y2 )
		{
			int drawx = n_x + n_drawoffset_x;

			if( ( (INT32)n_drawarea_x1 - drawx ) > 0 )
			{
				n_distance -= ( n_drawarea_x1 - drawx );
				drawx = n_drawarea_x1;
			}

			SOLIDFILL( FLATRECTANGEUPDATE )
		}
		n_y++;
		n_h--;
	}
}

void psxgpu_device::FlatRectangle16x16( void )
{
	INT16 n_y;
	INT16 n_x;

	UINT8 n_cmd;

	UINT16 *p_n_f;
	UINT16 *p_n_redb;
	UINT16 *p_n_greenb;
	UINT16 *p_n_blueb;
	UINT16 *p_n_redtrans;
	UINT16 *p_n_greentrans;
	UINT16 *p_n_bluetrans;

	PAIR n_r;
	PAIR n_g;
	PAIR n_b;

	INT32 n_distance;
	INT32 n_h;
	UINT16 *p_vram;

#if DEBUG_VIEWER
	if( m_debug.n_skip == 10 )
	{
		return;
	}
	DebugMesh( SINT11( COORD_X( m_packet.FlatRectangle16x16.n_coord ) ) + n_drawoffset_x, SINT11( COORD_Y( m_packet.FlatRectangle16x16.n_coord ) ) + n_drawoffset_y );
	DebugMesh( SINT11( COORD_X( m_packet.FlatRectangle16x16.n_coord ) ) + n_drawoffset_x + 16, SINT11( COORD_Y( m_packet.FlatRectangle16x16.n_coord ) ) + n_drawoffset_y );
	DebugMesh( SINT11( COORD_X( m_packet.FlatRectangle16x16.n_coord ) ) + n_drawoffset_x, SINT11( COORD_Y( m_packet.FlatRectangle16x16.n_coord ) ) + n_drawoffset_y + 16 );
	DebugMesh( SINT11( COORD_X( m_packet.FlatRectangle16x16.n_coord ) ) + n_drawoffset_x + 16, SINT11( COORD_Y( m_packet.FlatRectangle16x16.n_coord ) ) + n_drawoffset_y + 16 );
	DebugMeshEnd();
#endif

	n_cmd = BGR_C( m_packet.FlatRectangle16x16.n_bgr );

	SOLIDSETUP

	n_r.w.h = BGR_R( m_packet.FlatRectangle16x16.n_bgr ); n_r.w.l = 0;
	n_g.w.h = BGR_G( m_packet.FlatRectangle16x16.n_bgr ); n_g.w.l = 0;
	n_b.w.h = BGR_B( m_packet.FlatRectangle16x16.n_bgr ); n_b.w.l = 0;

	n_x = SINT11( COORD_X( m_packet.FlatRectangle16x16.n_coord ) );
	n_y = SINT11( COORD_Y( m_packet.FlatRectangle16x16.n_coord ) );
	n_h = 16;

	while( n_h > 0 )
	{
		n_distance = 16;

		int drawy = n_y + n_drawoffset_y;

		if( n_distance > 0 && n_y >= (INT32)n_drawarea_y1 && n_y <= (INT32)n_drawarea_y2 )
		{
			int drawx = n_x + n_drawoffset_x;

			if( ( (INT32)n_drawarea_x1 - drawx ) > 0 )
			{
				n_distance -= ( n_drawarea_x1 - drawx );
				drawx = n_drawarea_x1;
			}

			SOLIDFILL( FLATRECTANGEUPDATE )
		}
		n_y++;
		n_h--;
	}
}

void psxgpu_device::FlatTexturedRectangle( void )
{
	INT16 n_y;
	INT16 n_x;
	int n_tx;
	int n_ty;

	UINT8 n_cmd;

	UINT32 n_clutx;
	UINT32 n_cluty;

	UINT16 *p_n_f;
	UINT16 *p_n_redb;
	UINT16 *p_n_greenb;
	UINT16 *p_n_blueb;
	UINT16 *p_n_redtrans;
	UINT16 *p_n_greentrans;
	UINT16 *p_n_bluetrans;

	PAIR n_r;
	PAIR n_g;
	PAIR n_b;
	UINT8 n_u;
	UINT8 n_v;
	int n_du;
	int n_dv;

	INT16 n_distance;
	UINT32 n_h;
	UINT16 *p_vram;
	UINT16 *p_clut;
	UINT16 n_bgr;

#if DEBUG_VIEWER
	if( m_debug.n_skip == 11 )
	{
		return;
	}
	DebugMesh( SINT11( COORD_X( m_packet.FlatTexturedRectangle.n_coord ) ) + n_drawoffset_x, SINT11( COORD_Y( m_packet.FlatTexturedRectangle.n_coord ) ) + n_drawoffset_y );
	DebugMesh( SINT11( COORD_X( m_packet.FlatTexturedRectangle.n_coord ) ) + n_drawoffset_x + SIZE_W( m_packet.FlatTexturedRectangle.n_size ), SINT11( COORD_Y( m_packet.FlatTexturedRectangle.n_coord ) ) + n_drawoffset_y );
	DebugMesh( SINT11( COORD_X( m_packet.FlatTexturedRectangle.n_coord ) ) + n_drawoffset_x, SINT11( COORD_Y( m_packet.FlatTexturedRectangle.n_coord ) ) + n_drawoffset_y + SIZE_H( m_packet.FlatTexturedRectangle.n_size ) );
	DebugMesh( SINT11( COORD_X( m_packet.FlatTexturedRectangle.n_coord ) ) + n_drawoffset_x + SIZE_W( m_packet.FlatTexturedRectangle.n_size ), SINT11( COORD_Y( m_packet.FlatTexturedRectangle.n_coord ) ) + n_drawoffset_y + SIZE_H( m_packet.FlatTexturedRectangle.n_size ) );
	DebugMeshEnd();
#endif

	n_cmd = BGR_C( m_packet.FlatTexturedRectangle.n_bgr );

	n_clutx = ( m_packet.FlatTexturedRectangle.n_texture.w.h & 0x3f ) << 4;
	n_cluty = ( m_packet.FlatTexturedRectangle.n_texture.w.h >> 6 ) & 0x3ff;

	n_r.d = 0;
	n_g.d = 0;
	n_b.d = 0;

	TEXTURESETUP
	SPRITESETUP

	switch( n_cmd & 0x01 )
	{
	case 0:
		n_r.w.h = BGR_R( m_packet.FlatTexturedRectangle.n_bgr ); n_r.w.l = 0;
		n_g.w.h = BGR_G( m_packet.FlatTexturedRectangle.n_bgr ); n_g.w.l = 0;
		n_b.w.h = BGR_B( m_packet.FlatTexturedRectangle.n_bgr ); n_b.w.l = 0;
		break;
	case 1:
		n_r.w.h = 0x80; n_r.w.l = 0;
		n_g.w.h = 0x80; n_g.w.l = 0;
		n_b.w.h = 0x80; n_b.w.l = 0;
		break;
	}

	n_x = SINT11( COORD_X( m_packet.FlatTexturedRectangle.n_coord ) );
	n_y = SINT11( COORD_Y( m_packet.FlatTexturedRectangle.n_coord ) );
	n_v = TEXTURE_V( m_packet.FlatTexturedRectangle.n_texture );
	n_h = SIZE_H( m_packet.FlatTexturedRectangle.n_size );

	while( n_h > 0 )
	{
		n_u = TEXTURE_U( m_packet.FlatTexturedRectangle.n_texture );
		n_distance = SIZE_W( m_packet.FlatTexturedRectangle.n_size );

		int drawy = n_y + n_drawoffset_y;

		if( n_distance > 0 && drawy >= (INT32)n_drawarea_y1 && drawy <= (INT32)n_drawarea_y2 )
		{
			int drawx = n_x + n_drawoffset_x;

			if( ( (INT32)n_drawarea_x1 - drawx ) > 0 )
			{
				n_u += ( n_drawarea_x1 - drawx ) * n_du;
				n_distance -= ( n_drawarea_x1 - drawx );
				drawx = n_drawarea_x1;
			}

			TEXTUREFILL( FLATTEXTUREDRECTANGLEUPDATE, n_u, n_v );
		}
		n_v += n_dv;
		n_y++;
		n_h--;
	}
}

void psxgpu_device::Sprite8x8( void )
{
	INT16 n_y;
	INT16 n_x;
	int n_tx;
	int n_ty;

	UINT8 n_cmd;

	UINT32 n_clutx;
	UINT32 n_cluty;

	UINT16 *p_n_f;
	UINT16 *p_n_redb;
	UINT16 *p_n_greenb;
	UINT16 *p_n_blueb;
	UINT16 *p_n_redtrans;
	UINT16 *p_n_greentrans;
	UINT16 *p_n_bluetrans;

	PAIR n_r;
	PAIR n_g;
	PAIR n_b;
	UINT8 n_u;
	UINT8 n_v;
	int n_du;
	int n_dv;

	INT16 n_distance;
	UINT32 n_h;
	UINT16 *p_vram;
	UINT16 *p_clut;
	UINT16 n_bgr;

#if DEBUG_VIEWER
	if( m_debug.n_skip == 12 )
	{
		return;
	}
	DebugMesh( SINT11( COORD_X( m_packet.Sprite8x8.n_coord ) ) + n_drawoffset_x, SINT11( COORD_Y( m_packet.Sprite8x8.n_coord ) ) + n_drawoffset_y );
	DebugMesh( SINT11( COORD_X( m_packet.Sprite8x8.n_coord ) ) + n_drawoffset_x + 7, SINT11( COORD_Y( m_packet.Sprite8x8.n_coord ) ) + n_drawoffset_y );
	DebugMesh( SINT11( COORD_X( m_packet.Sprite8x8.n_coord ) ) + n_drawoffset_x, SINT11( COORD_Y( m_packet.Sprite8x8.n_coord ) ) + n_drawoffset_y + 7 );
	DebugMesh( SINT11( COORD_X( m_packet.Sprite8x8.n_coord ) ) + n_drawoffset_x + 7, SINT11( COORD_Y( m_packet.Sprite8x8.n_coord ) ) + n_drawoffset_y + 7 );
	DebugMeshEnd();
#endif

	n_cmd = BGR_C( m_packet.Sprite8x8.n_bgr );

	n_clutx = ( m_packet.Sprite8x8.n_texture.w.h & 0x3f ) << 4;
	n_cluty = ( m_packet.Sprite8x8.n_texture.w.h >> 6 ) & 0x3ff;

	n_r.d = 0;
	n_g.d = 0;
	n_b.d = 0;

	TEXTURESETUP
	SPRITESETUP

	switch( n_cmd & 0x01 )
	{
	case 0:
		n_r.w.h = BGR_R( m_packet.Sprite8x8.n_bgr ); n_r.w.l = 0;
		n_g.w.h = BGR_G( m_packet.Sprite8x8.n_bgr ); n_g.w.l = 0;
		n_b.w.h = BGR_B( m_packet.Sprite8x8.n_bgr ); n_b.w.l = 0;
		break;
	case 1:
		n_r.w.h = 0x80; n_r.w.l = 0;
		n_g.w.h = 0x80; n_g.w.l = 0;
		n_b.w.h = 0x80; n_b.w.l = 0;
		break;
	}

	n_x = SINT11( COORD_X( m_packet.Sprite8x8.n_coord ) );
	n_y = SINT11( COORD_Y( m_packet.Sprite8x8.n_coord ) );
	n_v = TEXTURE_V( m_packet.Sprite8x8.n_texture );
	n_h = 8;

	while( n_h > 0 )
	{
		n_u = TEXTURE_U( m_packet.Sprite8x8.n_texture );
		n_distance = 8;

		int drawy = n_y + n_drawoffset_y;

		if( n_distance > 0 && drawy >= (INT32)n_drawarea_y1 && drawy <= (INT32)n_drawarea_y2 )
		{
			int drawx = n_x + n_drawoffset_x;

			if( ( (INT32)n_drawarea_x1 - drawx ) > 0 )
			{
				n_u += ( n_drawarea_x1 - drawx ) * n_du;
				n_distance -= ( n_drawarea_x1 - drawx );
				drawx = n_drawarea_x1;
			}

			TEXTUREFILL( FLATTEXTUREDRECTANGLEUPDATE, n_u, n_v );
		}
		n_v += n_dv;
		n_y++;
		n_h--;
	}
}

void psxgpu_device::Sprite16x16( void )
{
	INT16 n_y;
	INT16 n_x;
	int n_tx;
	int n_ty;

	UINT8 n_cmd;

	UINT32 n_clutx;
	UINT32 n_cluty;

	UINT16 *p_n_f;
	UINT16 *p_n_redb;
	UINT16 *p_n_greenb;
	UINT16 *p_n_blueb;
	UINT16 *p_n_redtrans;
	UINT16 *p_n_greentrans;
	UINT16 *p_n_bluetrans;

	PAIR n_r;
	PAIR n_g;
	PAIR n_b;
	UINT8 n_u;
	UINT8 n_v;
	int n_du;
	int n_dv;

	INT16 n_distance;
	UINT32 n_h;
	UINT16 *p_vram;
	UINT16 *p_clut;
	UINT16 n_bgr;

#if DEBUG_VIEWER
	if( m_debug.n_skip == 13 )
	{
		return;
	}
	DebugMesh( SINT11( COORD_X( m_packet.Sprite16x16.n_coord ) ) + n_drawoffset_x, SINT11( COORD_Y( m_packet.Sprite16x16.n_coord ) ) + n_drawoffset_y );
	DebugMesh( SINT11( COORD_X( m_packet.Sprite16x16.n_coord ) ) + n_drawoffset_x + 7, SINT11( COORD_Y( m_packet.Sprite16x16.n_coord ) ) + n_drawoffset_y );
	DebugMesh( SINT11( COORD_X( m_packet.Sprite16x16.n_coord ) ) + n_drawoffset_x, SINT11( COORD_Y( m_packet.Sprite16x16.n_coord ) ) + n_drawoffset_y + 7 );
	DebugMesh( SINT11( COORD_X( m_packet.Sprite16x16.n_coord ) ) + n_drawoffset_x + 7, SINT11( COORD_Y( m_packet.Sprite16x16.n_coord ) ) + n_drawoffset_y + 7 );
	DebugMeshEnd();
#endif

	n_cmd = BGR_C( m_packet.Sprite16x16.n_bgr );

	n_clutx = ( m_packet.Sprite16x16.n_texture.w.h & 0x3f ) << 4;
	n_cluty = ( m_packet.Sprite16x16.n_texture.w.h >> 6 ) & 0x3ff;

	n_r.d = 0;
	n_g.d = 0;
	n_b.d = 0;

	TEXTURESETUP
	SPRITESETUP

	switch( n_cmd & 0x01 )
	{
	case 0:
		n_r.w.h = BGR_R( m_packet.Sprite16x16.n_bgr ); n_r.w.l = 0;
		n_g.w.h = BGR_G( m_packet.Sprite16x16.n_bgr ); n_g.w.l = 0;
		n_b.w.h = BGR_B( m_packet.Sprite16x16.n_bgr ); n_b.w.l = 0;
		break;
	case 1:
		n_r.w.h = 0x80; n_r.w.l = 0;
		n_g.w.h = 0x80; n_g.w.l = 0;
		n_b.w.h = 0x80; n_b.w.l = 0;
		break;
	}

	n_x = SINT11( COORD_X( m_packet.Sprite16x16.n_coord ) );
	n_y = SINT11( COORD_Y( m_packet.Sprite16x16.n_coord ) );
	n_v = TEXTURE_V( m_packet.Sprite16x16.n_texture );
	n_h = 16;

	while( n_h > 0 )
	{
		n_u = TEXTURE_U( m_packet.Sprite16x16.n_texture );
		n_distance = 16;

		int drawy = n_y + n_drawoffset_y;

		if( n_distance > 0 && drawy >= (INT32)n_drawarea_y1 && drawy <= (INT32)n_drawarea_y2 )
		{
			int drawx = n_x + n_drawoffset_x;

			if( ( (INT32)n_drawarea_x1 - drawx ) > 0 )
			{
				n_u += ( n_drawarea_x1 - drawx ) * n_du;
				n_distance -= ( n_drawarea_x1 - drawx );
				drawx = n_drawarea_x1;
			}

			TEXTUREFILL( FLATTEXTUREDRECTANGLEUPDATE, n_u, n_v );
		}
		n_v += n_dv;
		n_y++;
		n_h--;
	}
}

void psxgpu_device::Dot( void )
{
	INT32 n_x;
	INT32 n_y;
	UINT32 n_r;
	UINT32 n_g;
	UINT32 n_b;
	UINT16 *p_vram;

#if DEBUG_VIEWER
	if( m_debug.n_skip == 14 )
	{
		return;
	}
	DebugMesh( SINT11( COORD_X( m_packet.Dot.vertex.n_coord ) ) + n_drawoffset_x, SINT11( COORD_Y( m_packet.Dot.vertex.n_coord ) ) + n_drawoffset_y );
	DebugMeshEnd();
#endif

	n_r = BGR_R( m_packet.Dot.n_bgr );
	n_g = BGR_G( m_packet.Dot.n_bgr );
	n_b = BGR_B( m_packet.Dot.n_bgr );
	n_x = SINT11( COORD_X( m_packet.Dot.vertex.n_coord ) );
	n_y = SINT11( COORD_Y( m_packet.Dot.vertex.n_coord ) );

	int drawx = n_x + n_drawoffset_x;
	int drawy = n_y + n_drawoffset_y;

	if( drawx >= (INT32)n_drawarea_x1 && drawy >= (INT32)n_drawarea_y1 &&
		drawx <= (INT32)n_drawarea_x2 && drawy <= (INT32)n_drawarea_y2 )
	{
		p_vram = p_p_vram[ drawy ] + drawx;
		WRITE_PIXEL(
			p_n_redshade[ MID_LEVEL | n_r ] |
			p_n_greenshade[ MID_LEVEL | n_g ] |
			p_n_blueshade[ MID_LEVEL | n_b ] );
	}
}

void psxgpu_device::MoveImage( void )
{
	INT16 n_w;
	INT16 n_h;
	INT16 n_srcx;
	INT16 n_srcy;
	INT16 n_dsty;
	INT16 n_dstx;
	UINT16 *p_vram;

#if DEBUG_VIEWER
	if( m_debug.n_skip == 15 )
	{
		return;
	}
	DebugMesh( SINT11( COORD_X( m_packet.MoveImage.vertex[ 1 ].n_coord ) ), SINT11( COORD_Y( m_packet.MoveImage.vertex[ 1 ].n_coord ) ) );
	DebugMesh( SINT11( COORD_X( m_packet.MoveImage.vertex[ 1 ].n_coord ) ) + SIZE_W( m_packet.MoveImage.n_size ), SINT11( COORD_Y( m_packet.MoveImage.vertex[ 1 ].n_coord ) ) );
	DebugMesh( SINT11( COORD_X( m_packet.MoveImage.vertex[ 1 ].n_coord ) ), SINT11( COORD_Y( m_packet.MoveImage.vertex[ 1 ].n_coord ) ) + SIZE_H( m_packet.MoveImage.n_size ) );
	DebugMesh( SINT11( COORD_X( m_packet.MoveImage.vertex[ 1 ].n_coord ) ) + SIZE_W( m_packet.MoveImage.n_size ), SINT11( COORD_Y( m_packet.MoveImage.vertex[ 1 ].n_coord ) ) + SIZE_H( m_packet.MoveImage.n_size ) );
	DebugMeshEnd();
#endif

	n_srcy = COORD_Y( m_packet.MoveImage.vertex[ 0 ].n_coord );
	n_dsty = COORD_Y( m_packet.MoveImage.vertex[ 1 ].n_coord );
	n_h = SIZE_H( m_packet.MoveImage.n_size );

	while( n_h > 0 )
	{
		n_srcx = COORD_X( m_packet.MoveImage.vertex[ 0 ].n_coord );
		n_dstx = COORD_X( m_packet.MoveImage.vertex[ 1 ].n_coord );
		n_w = SIZE_W( m_packet.MoveImage.n_size );
		while( n_w > 0 )
		{
			p_vram = p_p_vram[ n_dsty & 1023 ] + ( n_dstx & 1023 );
			WRITE_PIXEL( *( p_p_vram[ n_srcy & 1023 ] + ( n_srcx & 1023 ) ) );
			n_srcx++;
			n_dstx++;
			n_w--;
		}
		n_srcy++;
		n_dsty++;
		n_h--;
	}
}

void psxgpu_device::dma_write( UINT32 *p_n_psxram, UINT32 n_address, INT32 n_size )
{
	gpu_write( &p_n_psxram[ n_address / 4 ], n_size );
}

void psxgpu_device::gpu_write( UINT32 *p_ram, INT32 n_size )
{
	while( n_size > 0 )
	{
		UINT32 data = *( p_ram );

		verboselog( machine(), 2, "PSX Packet #%u %08x\n", n_gpu_buffer_offset, data );
		m_packet.n_entry[ n_gpu_buffer_offset ] = data;
		switch( m_packet.n_entry[ 0 ] >> 24 )
		{
		case 0x00:
			verboselog( machine(), 1, "not handled: GPU Command 0x00: (%08x)\n", data );
			break;
		case 0x01:
			verboselog( machine(), 1, "not handled: clear cache\n" );
			break;
		case 0x02:
			if( n_gpu_buffer_offset < 2 )
			{
				n_gpu_buffer_offset++;
			}
			else
			{
				verboselog( machine(), 1, "%02x: frame buffer rectangle %u,%u %u,%u\n", m_packet.n_entry[ 0 ] >> 24,
					m_packet.n_entry[ 1 ] & 0xffff, m_packet.n_entry[ 1 ] >> 16, m_packet.n_entry[ 2 ] & 0xffff, m_packet.n_entry[ 2 ] >> 16 );
				FrameBufferRectangleDraw();
				n_gpu_buffer_offset = 0;
			}
			break;
		case 0x20:
		case 0x21:
		case 0x22:
		case 0x23:
			if( n_gpu_buffer_offset < 3 )
			{
				n_gpu_buffer_offset++;
			}
			else
			{
				verboselog( machine(), 1, "%02x: monochrome 3 point polygon\n", m_packet.n_entry[ 0 ] >> 24 );
				FlatPolygon( 3 );
				n_gpu_buffer_offset = 0;
			}
			break;
		case 0x24:
		case 0x25:
		case 0x26:
		case 0x27:
			if( n_gpu_buffer_offset < 6 )
			{
				n_gpu_buffer_offset++;
			}
			else
			{
				verboselog( machine(), 1, "%02x: textured 3 point polygon\n", m_packet.n_entry[ 0 ] >> 24 );
				FlatTexturedPolygon( 3 );
				n_gpu_buffer_offset = 0;
			}
			break;
		case 0x28:
		case 0x29:
		case 0x2a:
		case 0x2b:
			if( n_gpu_buffer_offset < 4 )
			{
				n_gpu_buffer_offset++;
			}
			else
			{
				verboselog( machine(), 1, "%02x: monochrome 4 point polygon\n", m_packet.n_entry[ 0 ] >> 24 );
				FlatPolygon( 4 );
				n_gpu_buffer_offset = 0;
			}
			break;
		case 0x2c:
		case 0x2d:
		case 0x2e:
		case 0x2f:
			if( n_gpu_buffer_offset < 8 )
			{
				n_gpu_buffer_offset++;
			}
			else
			{
				verboselog( machine(), 1, "%02x: textured 4 point polygon\n", m_packet.n_entry[ 0 ] >> 24 );
				FlatTexturedPolygon( 4 );
				n_gpu_buffer_offset = 0;
			}
			break;
		case 0x30:
		case 0x31:
		case 0x32:
		case 0x33:
			if( n_gpu_buffer_offset < 5 )
			{
				n_gpu_buffer_offset++;
			}
			else
			{
				verboselog( machine(), 1, "%02x: gouraud 3 point polygon\n", m_packet.n_entry[ 0 ] >> 24 );
				GouraudPolygon( 3 );
				n_gpu_buffer_offset = 0;
			}
			break;
		case 0x34:
		case 0x35:
		case 0x36:
		case 0x37:
			if( n_gpu_buffer_offset < 8 )
			{
				n_gpu_buffer_offset++;
			}
			else
			{
				verboselog( machine(), 1, "%02x: gouraud textured 3 point polygon\n", m_packet.n_entry[ 0 ] >> 24 );
				GouraudTexturedPolygon( 3 );
				n_gpu_buffer_offset = 0;
			}
			break;
		case 0x38:
		case 0x39:
		case 0x3a:
		case 0x3b:
			if( n_gpu_buffer_offset < 7 )
			{
				n_gpu_buffer_offset++;
			}
			else
			{
				verboselog( machine(), 1, "%02x: gouraud 4 point polygon\n", m_packet.n_entry[ 0 ] >> 24 );
				GouraudPolygon( 4 );
				n_gpu_buffer_offset = 0;
			}
			break;
		case 0x3c:
		case 0x3d:
		case 0x3e:
		case 0x3f:
			if( n_gpu_buffer_offset < 11 )
			{
				n_gpu_buffer_offset++;
			}
			else
			{
				verboselog( machine(), 1, "%02x: gouraud textured 4 point polygon\n", m_packet.n_entry[ 0 ] >> 24 );
				GouraudTexturedPolygon( 4 );
				n_gpu_buffer_offset = 0;
			}
			break;
		case 0x40:
		case 0x41:
		case 0x42:
			if( n_gpu_buffer_offset < 2 )
			{
				n_gpu_buffer_offset++;
			}
			else
			{
				verboselog( machine(), 1, "%02x: monochrome line\n", m_packet.n_entry[ 0 ] >> 24 );
				MonochromeLine();
				n_gpu_buffer_offset = 0;
			}
			break;
		case 0x48:
		case 0x4a:
		case 0x4c:
		case 0x4e:
			if( n_gpu_buffer_offset < 3 )
			{
				n_gpu_buffer_offset++;
			}
			else
			{
				verboselog( machine(), 1, "%02x: monochrome polyline\n", m_packet.n_entry[ 0 ] >> 24 );
				MonochromeLine();
				if( ( m_packet.n_entry[ 3 ] & 0xf000f000 ) != 0x50005000 )
				{
					m_packet.n_entry[ 1 ] = m_packet.n_entry[ 2 ];
					m_packet.n_entry[ 2 ] = m_packet.n_entry[ 3 ];
					n_gpu_buffer_offset = 3;
				}
				else
				{
					n_gpu_buffer_offset = 0;
				}
			}
			break;
		case 0x50:
		case 0x51:
		case 0x52:
		case 0x53:
			if( n_gpu_buffer_offset < 3 )
			{
				n_gpu_buffer_offset++;
			}
			else
			{
				verboselog( machine(), 1, "%02x: gouraud line\n", m_packet.n_entry[ 0 ] >> 24 );
				GouraudLine();
				n_gpu_buffer_offset = 0;
			}
			break;
		case 0x58:
		case 0x5a:
		case 0x5c:
		case 0x5e:
			if( n_gpu_buffer_offset < 5 &&
				( n_gpu_buffer_offset != 4 || ( m_packet.n_entry[ 4 ] & 0xf000f000 ) != 0x50005000 ) )
			{
				n_gpu_buffer_offset++;
			}
			else
			{
				verboselog( machine(), 1, "%02x: gouraud polyline\n", m_packet.n_entry[ 0 ] >> 24 );
				GouraudLine();
				if( ( m_packet.n_entry[ 4 ] & 0xf000f000 ) != 0x50005000 )
				{
					m_packet.n_entry[ 0 ] = ( m_packet.n_entry[ 0 ] & 0xff000000 ) | ( m_packet.n_entry[ 2 ] & 0x00ffffff );
					m_packet.n_entry[ 1 ] = m_packet.n_entry[ 3 ];
					m_packet.n_entry[ 2 ] = m_packet.n_entry[ 4 ];
					m_packet.n_entry[ 3 ] = m_packet.n_entry[ 5 ];
					n_gpu_buffer_offset = 4;
				}
				else
				{
					n_gpu_buffer_offset = 0;
				}
			}
			break;
		case 0x60:
		case 0x61:
		case 0x62:
		case 0x63:
			if( n_gpu_buffer_offset < 2 )
			{
				n_gpu_buffer_offset++;
			}
			else
			{
				verboselog( machine(), 1, "%02x: rectangle %d,%d %d,%d\n",
					m_packet.n_entry[ 0 ] >> 24,
					(INT16)( m_packet.n_entry[ 1 ] & 0xffff ), (INT16)( m_packet.n_entry[ 1 ] >> 16 ),
					(INT16)( m_packet.n_entry[ 2 ] & 0xffff ), (INT16)( m_packet.n_entry[ 2 ] >> 16 ) );
				FlatRectangle();
				n_gpu_buffer_offset = 0;
			}
			break;
		case 0x64:
		case 0x65:
		case 0x66:
		case 0x67:
			if( n_gpu_buffer_offset < 3 )
			{
				n_gpu_buffer_offset++;
			}
			else
			{
				verboselog( machine(), 1, "%02x: sprite %d,%d %u,%u %08x, %08x\n",
					m_packet.n_entry[ 0 ] >> 24,
					(INT16)( m_packet.n_entry[ 1 ] & 0xffff ), (INT16)( m_packet.n_entry[ 1 ] >> 16 ),
					m_packet.n_entry[ 3 ] & 0xffff, m_packet.n_entry[ 3 ] >> 16,
					m_packet.n_entry[ 0 ], m_packet.n_entry[ 2 ] );
				FlatTexturedRectangle();
				n_gpu_buffer_offset = 0;
			}
			break;
		case 0x68:
		case 0x6a:
			if( n_gpu_buffer_offset < 1 )
			{
				n_gpu_buffer_offset++;
			}
			else
			{
				verboselog( machine(), 1, "%02x: dot %d,%d %08x\n",
					m_packet.n_entry[ 0 ] >> 24,
					(INT16)( m_packet.n_entry[ 1 ] & 0xffff ), (INT16)( m_packet.n_entry[ 1 ] >> 16 ),
					m_packet.n_entry[ 0 ] & 0xffffff );
				Dot();
				n_gpu_buffer_offset = 0;
			}
			break;
		case 0x70:
		case 0x71:
			/* 8*8 rectangle */
			if( n_gpu_buffer_offset < 1 )
			{
				n_gpu_buffer_offset++;
			}
			else
			{
				verboselog( machine(), 1, "%02x: 16x16 rectangle %08x %08x\n", m_packet.n_entry[ 0 ] >> 24,
					m_packet.n_entry[ 0 ], m_packet.n_entry[ 1 ] );
				FlatRectangle8x8();
				n_gpu_buffer_offset = 0;
			}
			break;
		case 0x74:
		case 0x75:
		case 0x76:
		case 0x77:
			if( n_gpu_buffer_offset < 2 )
			{
				n_gpu_buffer_offset++;
			}
			else
			{
				verboselog( machine(), 1, "%02x: 8x8 sprite %08x %08x %08x\n", m_packet.n_entry[ 0 ] >> 24,
					m_packet.n_entry[ 0 ], m_packet.n_entry[ 1 ], m_packet.n_entry[ 2 ] );
				Sprite8x8();
				n_gpu_buffer_offset = 0;
			}
			break;
		case 0x78:
		case 0x79:
			/* 16*16 rectangle */
			if( n_gpu_buffer_offset < 1 )
			{
				n_gpu_buffer_offset++;
			}
			else
			{
				verboselog( machine(), 1, "%02x: 16x16 rectangle %08x %08x\n", m_packet.n_entry[ 0 ] >> 24,
					m_packet.n_entry[ 0 ], m_packet.n_entry[ 1 ] );
				FlatRectangle16x16();
				n_gpu_buffer_offset = 0;
			}
			break;
		case 0x7c:
		case 0x7d:
		case 0x7e:
		case 0x7f:
			if( n_gpu_buffer_offset < 2 )
			{
				n_gpu_buffer_offset++;
			}
			else
			{
				verboselog( machine(), 1, "%02x: 16x16 sprite %08x %08x %08x\n", m_packet.n_entry[ 0 ] >> 24,
					m_packet.n_entry[ 0 ], m_packet.n_entry[ 1 ], m_packet.n_entry[ 2 ] );
				Sprite16x16();
				n_gpu_buffer_offset = 0;
			}
			break;
		case 0x80:
			if( n_gpu_buffer_offset < 3 )
			{
				n_gpu_buffer_offset++;
			}
			else
			{
				verboselog( machine(), 1, "move image in frame buffer %08x %08x %08x %08x\n", m_packet.n_entry[ 0 ], m_packet.n_entry[ 1 ], m_packet.n_entry[ 2 ], m_packet.n_entry[ 3 ] );
				MoveImage();
				n_gpu_buffer_offset = 0;
			}
			break;
		case 0xa0:
			if( n_gpu_buffer_offset < 3 )
			{
				n_gpu_buffer_offset++;
			}
			else
			{
				UINT32 n_pixel;
				for( n_pixel = 0; n_pixel < 2; n_pixel++ )
				{
					UINT16 *p_vram;

					verboselog( machine(), 2, "send image to framebuffer ( pixel %u,%u = %u )\n",
						( n_vramx + m_packet.n_entry[ 1 ] ) & 1023,
						( n_vramy + ( m_packet.n_entry[ 1 ] >> 16 ) ) & 1023,
						data & 0xffff );

					p_vram = p_p_vram[ ( n_vramy + ( m_packet.n_entry[ 1 ] >> 16 ) ) & 1023 ] + ( ( n_vramx + m_packet.n_entry[ 1 ] ) & 1023 );
					WRITE_PIXEL( data & 0xffff );
					n_vramx++;
					if( n_vramx >= ( m_packet.n_entry[ 2 ] & 0xffff ) )
					{
						n_vramx = 0;
						n_vramy++;
						if( n_vramy >= ( m_packet.n_entry[ 2 ] >> 16 ) )
						{
							verboselog( machine(), 1, "%02x: send image to framebuffer %u,%u %u,%u\n", m_packet.n_entry[ 0 ] >> 24,
								m_packet.n_entry[ 1 ] & 0xffff, ( m_packet.n_entry[ 1 ] >> 16 ),
								m_packet.n_entry[ 2 ] & 0xffff, ( m_packet.n_entry[ 2 ] >> 16 ) );
							n_gpu_buffer_offset = 0;
							n_vramx = 0;
							n_vramy = 0;
							break;
						}
					}
					data >>= 16;
				}
			}
			break;
		case 0xc0:
			if( n_gpu_buffer_offset < 2 )
			{
				n_gpu_buffer_offset++;
			}
			else
			{
				verboselog( machine(), 1, "%02x: copy image from frame buffer\n", m_packet.n_entry[ 0 ] >> 24 );
				n_gpustatus |= ( 1L << 0x1b );
			}
			break;
		case 0xe1:
			verboselog( machine(), 1, "%02x: draw mode %06x\n", m_packet.n_entry[ 0 ] >> 24,
				m_packet.n_entry[ 0 ] & 0xffffff );
			decode_tpage( m_packet.n_entry[ 0 ] & 0xffffff );
			break;
		case 0xe2:
			n_twy = ( ( ( m_packet.n_entry[ 0 ] >> 15 ) & 0x1f ) << 3 );
			n_twx = ( ( ( m_packet.n_entry[ 0 ] >> 10 ) & 0x1f ) << 3 );
			n_twh = 255 - ( ( ( m_packet.n_entry[ 0 ] >> 5 ) & 0x1f ) << 3 );
			n_tww = 255 - ( ( m_packet.n_entry[ 0 ] & 0x1f ) << 3 );
			verboselog( machine(), 1, "%02x: texture window %u,%u %u,%u\n", m_packet.n_entry[ 0 ] >> 24,
				n_twx, n_twy, n_tww, n_twh );
			break;
		case 0xe3:
			n_drawarea_x1 = m_packet.n_entry[ 0 ] & 1023;
			if( m_n_gputype == 2 )
			{
				n_drawarea_y1 = ( m_packet.n_entry[ 0 ] >> 10 ) & 1023;
			}
			else
			{
				n_drawarea_y1 = ( m_packet.n_entry[ 0 ] >> 12 ) & 1023;
			}
			verboselog( machine(), 1, "%02x: drawing area top left %d,%d\n", m_packet.n_entry[ 0 ] >> 24,
				n_drawarea_x1, n_drawarea_y1 );
			break;
		case 0xe4:
			n_drawarea_x2 = m_packet.n_entry[ 0 ] & 1023;
			if( m_n_gputype == 2 )
			{
				n_drawarea_y2 = ( m_packet.n_entry[ 0 ] >> 10 ) & 1023;
			}
			else
			{
				n_drawarea_y2 = ( m_packet.n_entry[ 0 ] >> 12 ) & 1023;
			}
			verboselog( machine(), 1, "%02x: drawing area bottom right %d,%d\n", m_packet.n_entry[ 0 ] >> 24,
				n_drawarea_x2, n_drawarea_y2 );
			break;
		case 0xe5:
			n_drawoffset_x = SINT11( m_packet.n_entry[ 0 ] & 2047 );
			if( m_n_gputype == 2 )
			{
				n_drawoffset_y = SINT11( ( m_packet.n_entry[ 0 ] >> 11 ) & 2047 );
			}
			else
			{
				n_drawoffset_y = SINT11( ( m_packet.n_entry[ 0 ] >> 12 ) & 2047 );
			}
			verboselog( machine(), 1, "%02x: drawing offset %d,%d\n", m_packet.n_entry[ 0 ] >> 24,
				n_drawoffset_x, n_drawoffset_y );
			break;
		case 0xe6:
			n_gpustatus &= ~( 3L << 0xb );
			n_gpustatus |= ( data & 0x03 ) << 0xb;
			if( ( m_packet.n_entry[ 0 ] & 3 ) != 0 )
			{
				verboselog( machine(), 1, "not handled: mask setting %d\n", m_packet.n_entry[ 0 ] & 3 );
			}
			else
			{
				verboselog( machine(), 1, "mask setting %d\n", m_packet.n_entry[ 0 ] & 3 );
			}
			break;
		default:
#if defined( MAME_DEBUG )
			popmessage( "unknown GPU packet %08x", m_packet.n_entry[ 0 ] );
#endif
			verboselog( machine(), 0, "unknown GPU packet %08x (%08x)\n", m_packet.n_entry[ 0 ], data );
#if ( STOP_ON_ERROR )
			n_gpu_buffer_offset = 1;
#endif
			break;
		}
		p_ram++;
		n_size--;
	}
}

WRITE32_MEMBER( psxgpu_device::write )
{
	switch( offset )
	{
	case 0x00:
		gpu_write( &data, 1 );
		break;
	case 0x01:
		switch( data >> 24 )
		{
		case 0x00:
			gpu_reset();
			break;
		case 0x01:
			verboselog( machine(), 1, "not handled: reset command buffer\n" );
			n_gpu_buffer_offset = 0;
			break;
		case 0x02:
			verboselog( machine(), 1, "not handled: reset irq\n" );
			break;
		case 0x03:
			n_gpustatus &= ~( 1L << 0x17 );
			n_gpustatus |= ( data & 0x01 ) << 0x17;
			break;
		case 0x04:
			verboselog( machine(), 1, "dma setup %d\n", data & 3 );
			n_gpustatus &= ~( 3L << 0x1d );
			n_gpustatus |= ( data & 0x03 ) << 0x1d;
			n_gpustatus &= ~( 1L << 0x19 );
			if( ( data & 3 ) == 1 || ( data & 3 ) == 2 )
			{
				n_gpustatus |= ( 1L << 0x19 );
			}
			break;
		case 0x05:
			m_n_displaystartx = data & 1023;
			if( m_n_gputype == 2 )
			{
				n_displaystarty = ( data >> 10 ) & 1023;
			}
			else
			{
				n_displaystarty = ( data >> 12 ) & 1023;
			}
			verboselog( machine(), 1, "start of display area %d %d\n", m_n_displaystartx, n_displaystarty );
			break;
		case 0x06:
			n_horiz_disstart = data & 4095;
			n_horiz_disend = ( data >> 12 ) & 4095;
			verboselog( machine(), 1, "horizontal display range %d %d\n", n_horiz_disstart, n_horiz_disend );
			break;
		case 0x07:
			n_vert_disstart = data & 1023;
			n_vert_disend = ( data >> 10 ) & 2047;
			verboselog( machine(), 1, "vertical display range %d %d\n", n_vert_disstart, n_vert_disend );
			break;
		case 0x08:
			verboselog( machine(), 1, "display mode %02x\n", data & 0xff );
			n_gpustatus &= ~( 127L << 0x10 );
			n_gpustatus |= ( data & 0x3f ) << 0x11; /* width 0 + height + videmode + isrgb24 + isinter */
			n_gpustatus |= ( ( data & 0x40 ) >> 0x06 ) << 0x10; /* width 1 */
			if( m_n_gputype == 1 )
			{
				b_reverseflag = ( data >> 7 ) & 1;
			}
			updatevisiblearea();
			break;
		case 0x09:
			verboselog( machine(), 1, "not handled: GPU Control 0x09: %08x\n", data );
			break;
		case 0x0d:
			verboselog( machine(), 1, "reset lightgun coordinates %08x\n", data );
			n_lightgun_x = 0;
			n_lightgun_y = 0;
			break;
		case 0x10:
			switch( data & 0xff )
			{
			case 0x03:
				if( m_n_gputype == 2 )
				{
					n_gpuinfo = n_drawarea_x1 | ( n_drawarea_y1 << 10 );
				}
				else
				{
					n_gpuinfo = n_drawarea_x1 | ( n_drawarea_y1 << 12 );
				}
				verboselog( machine(), 1, "GPU Info - Draw area top left %08x\n", n_gpuinfo );
				break;
			case 0x04:
				if( m_n_gputype == 2 )
				{
					n_gpuinfo = n_drawarea_x2 | ( n_drawarea_y2 << 10 );
				}
				else
				{
					n_gpuinfo = n_drawarea_x2 | ( n_drawarea_y2 << 12 );
				}
				verboselog( machine(), 1, "GPU Info - Draw area bottom right %08x\n", n_gpuinfo );
				break;
			case 0x05:
				if( m_n_gputype == 2 )
				{
					n_gpuinfo = ( n_drawoffset_x & 2047 ) | ( ( n_drawoffset_y & 2047 ) << 11 );
				}
				else
				{
					n_gpuinfo = ( n_drawoffset_x & 2047 ) | ( ( n_drawoffset_y & 2047 ) << 12 );
				}
				verboselog( machine(), 1, "GPU Info - Draw offset %08x\n", n_gpuinfo );
				break;
			case 0x07:
				n_gpuinfo = m_n_gputype;
				verboselog( machine(), 1, "GPU Info - GPU Type %08x\n", n_gpuinfo );
				break;
			case 0x08:
				n_gpuinfo = n_lightgun_x | ( n_lightgun_y << 16 );
				verboselog( machine(), 1, "GPU Info - lightgun coordinates %08x\n", n_gpuinfo );
				break;
			default:
				verboselog( machine(), 0, "GPU Info - unknown request (%08x)\n", data );
				n_gpuinfo = 0;
				break;
			}
			break;
		case 0x20:
			verboselog( machine(), 1, "not handled: GPU Control 0x20: %08x\n", data );
			break;
		default:
#if defined( MAME_DEBUG )
			popmessage( "unknown GPU command %08x", data );
#endif
			verboselog( machine(), 0, "gpu_w( %08x ) unknown GPU command\n", data );
			break;
		}
		break;
	default:
		verboselog( machine(), 0, "gpu_w( %08x, %08x, %08x ) unknown register\n", offset, data, mem_mask );
		break;
	}
}


void psxgpu_device::dma_read( UINT32 *p_n_psxram, UINT32 n_address, INT32 n_size )
{
	gpu_read( &p_n_psxram[ n_address / 4 ], n_size );
}

void psxgpu_device::gpu_read( UINT32 *p_ram, INT32 n_size )
{
	while( n_size > 0 )
	{
		if( ( n_gpustatus & ( 1L << 0x1b ) ) != 0 )
		{
			UINT32 n_pixel;
			PAIR data;

			verboselog( machine(), 2, "copy image from frame buffer ( %d, %d )\n", n_vramx, n_vramy );
			data.d = 0;
			for( n_pixel = 0; n_pixel < 2; n_pixel++ )
			{
				data.w.l = data.w.h;
				data.w.h = *( p_p_vram[ ( n_vramy + ( m_packet.n_entry[ 1 ] >> 16 ) ) & 0x3ff ] + ( ( n_vramx + ( m_packet.n_entry[ 1 ] & 0xffff ) ) & 0x3ff ) );
				n_vramx++;
				if( n_vramx >= ( m_packet.n_entry[ 2 ] & 0xffff ) )
				{
					n_vramx = 0;
					n_vramy++;
					if( n_vramy >= ( m_packet.n_entry[ 2 ] >> 16 ) )
					{
						verboselog( machine(), 1, "copy image from frame buffer end\n" );
						n_gpustatus &= ~( 1L << 0x1b );
						n_gpu_buffer_offset = 0;
						n_vramx = 0;
						n_vramy = 0;
						if( n_pixel == 0 )
						{
							data.w.l = data.w.h;
							data.w.h = 0;
						}
						break;
					}
				}
			}
			*( p_ram ) = data.d;
		}
		else
		{
			verboselog( machine(), 2, "read GPU info (%08x)\n", n_gpuinfo );
			*( p_ram ) = n_gpuinfo;
		}
		p_ram++;
		n_size--;
	}
}

READ32_MEMBER( psxgpu_device::read )
{
	UINT32 data;

	switch( offset )
	{
	case 0x00:
		gpu_read( &data, 1 );
		break;
	case 0x01:
		data = n_gpustatus;
		verboselog( machine(), 1, "read GPU status (%08x)\n", data );
		break;
	default:
		verboselog( machine(), 0, "gpu_r( %08x, %08x ) unknown register\n", offset, mem_mask );
		data = 0;
		break;
	}
	return data;
}

void psxgpu_device::vblank(screen_device &screen, bool vblank_state)
{
	if( vblank_state )
	{
#if DEBUG_VIEWER
		DebugCheckKeys();
#endif

		n_gpustatus ^= ( 1L << 31 );
		m_vblank_handler(1);
	}
}

void psxgpu_device::gpu_reset( void )
{
	verboselog( machine(), 1, "reset gpu\n" );
	n_gpu_buffer_offset = 0;
	n_gpustatus = 0x14802000;
	n_drawarea_x1 = 0;
	n_drawarea_y1 = 0;
	n_drawarea_x2 = 1023;
	n_drawarea_y2 = 1023;
	n_drawoffset_x = 0;
	n_drawoffset_y = 0;
	m_n_displaystartx = 0;
	n_displaystarty = 0;
	n_horiz_disstart = 0x260;
	n_horiz_disend = 0xc60;
	n_vert_disstart = 0x010;
	n_vert_disend = 0x100;
	n_vramx = 0;
	n_vramy = 0;
	n_twx = 0;
	n_twy = 0;
	n_twh = 255;
	n_tww = 255;
	updatevisiblearea();
}

void psxgpu_device::lightgun_set( int n_x, int n_y )
{
	n_lightgun_x = n_x;
	n_lightgun_y = n_y;
}

PALETTE_INIT_MEMBER( psxgpu_device, psx )
{
	UINT32 n_colour;

	for( n_colour = 0; n_colour < 0x10000; n_colour++ )
	{
		palette.set_pen_color( n_colour, pal555(n_colour,0, 5, 10) );
	}
}

MACHINE_CONFIG_FRAGMENT( psxgpu )
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE( 60 )
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE( 1024, 1024 )
	MCFG_SCREEN_VISIBLE_AREA( 0, 639, 0, 479 )
	MCFG_SCREEN_UPDATE_DEVICE( DEVICE_SELF, psxgpu_device, update_screen )
	((screen_device *)device)->register_vblank_callback(vblank_state_delegate(FUNC(psxgpu_device::vblank), (psxgpu_device *) owner));
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD( "palette", 65536 )
	MCFG_PALETTE_INIT_OWNER(psxgpu_device, psx)
MACHINE_CONFIG_END

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor psxgpu_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( psxgpu );
}
