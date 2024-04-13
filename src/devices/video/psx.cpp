// license:BSD-3-Clause
// copyright-holders:smf
/*
 * PlayStation GPU emulator
 *
 * Copyright 2003-2014 smf
 *
 */

#include "emu.h"
#include "psx.h"

#include "cpu/psx/psx.h"

#include "screen.h"


#define STOP_ON_ERROR ( 0 )

#define LOG_WRITE        (1U << 1)
#define LOG_READ         (1U << 2)
#define LOG_TRANSPARENCY (1U << 3)
#define VERBOSE          (0)
#include "logmacro.h"

// device type definition
DEFINE_DEVICE_TYPE(CXD8514Q,  cxd8514q_device,  "cxd8514q",  "CXD8514Q GPU") // VRAM
DEFINE_DEVICE_TYPE(CXD8538Q,  cxd8538q_device,  "cxd8538q",  "CXD8538Q GPU") // VRAM
DEFINE_DEVICE_TYPE(CXD8561Q,  cxd8561q_device,  "cxd8561q",  "CXD8561Q GPU") // SGRAM
DEFINE_DEVICE_TYPE(CXD8561BQ, cxd8561bq_device, "cxd8561bq", "CXD8561BQ GPU") // SGRAM
DEFINE_DEVICE_TYPE(CXD8561CQ, cxd8561cq_device, "cxd8561cq", "CXD8561CQ GPU") // SGRAM
DEFINE_DEVICE_TYPE(CXD8654Q,  cxd8654q_device,  "cxd8654q",  "CXD8654Q GPU") // SGRAM

psxgpu_device::psxgpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint32_t vram_size, psxcpu_device *cpu)
	: psxgpu_device(mconfig, type, tag, owner, clock)
{
	set_vram_size(vram_size);
	cpu->gpu_read().set(tag, FUNC(psxgpu_device::read));
	cpu->gpu_write().set(tag, FUNC(psxgpu_device::write));
	cpu->subdevice<psxdma_device>("dma")->install_read_handler(2, psxdma_device::read_delegate(&psxgpu_device::dma_read, this));
	cpu->subdevice<psxdma_device>("dma")->install_write_handler(2, psxdma_device::write_delegate(&psxgpu_device::dma_write, this));
	vblank_callback().set(*cpu->subdevice<psxirq_device>("irq"), FUNC(psxirq_device::intin0));
}

psxgpu_device::psxgpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_video_interface(mconfig, *this)
	, device_palette_interface(mconfig, *this)
	, m_vblank_handler(*this)
{
}

void psxgpu_device::device_start()
{
	screen().register_vblank_callback(vblank_state_delegate(&psxgpu_device::vblank, this));

	for( int n_colour = 0; n_colour < 0x10000; n_colour++ )
	{
		set_pen_color( n_colour, pal555(n_colour,0, 5, 10) );
	}

	if (type() == CXD8538Q)
	{
		psx_gpu_init( 1 );
	}
	else
	{
		psx_gpu_init( 2 );
	}
}

void psxgpu_device::device_reset()
{
	gpu_reset();
}

cxd8514q_device::cxd8514q_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, uint32_t vram_size, psxcpu_device *cpu)
	: psxgpu_device(mconfig, CXD8514Q, tag, owner, clock, vram_size, cpu)
{
}

cxd8514q_device::cxd8514q_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: psxgpu_device(mconfig, CXD8514Q, tag, owner, clock)
{
}

cxd8538q_device::cxd8538q_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, uint32_t vram_size, psxcpu_device *cpu)
	: psxgpu_device(mconfig, CXD8538Q, tag, owner, clock, vram_size, cpu)
{
}

cxd8538q_device::cxd8538q_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: psxgpu_device(mconfig, CXD8538Q, tag, owner, clock)
{
}

cxd8561q_device::cxd8561q_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, uint32_t vram_size, psxcpu_device *cpu)
	: psxgpu_device(mconfig, CXD8561Q, tag, owner, clock, vram_size, cpu)
{
}

cxd8561q_device::cxd8561q_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: psxgpu_device(mconfig, CXD8561Q, tag, owner, clock)
{
}

cxd8561bq_device::cxd8561bq_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, uint32_t vram_size, psxcpu_device *cpu)
	: psxgpu_device(mconfig, CXD8561BQ, tag, owner, clock, vram_size, cpu)
{
}

cxd8561bq_device::cxd8561bq_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: psxgpu_device(mconfig, CXD8561BQ, tag, owner, clock)
{
}

cxd8561cq_device::cxd8561cq_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, uint32_t vram_size, psxcpu_device *cpu)
	: psxgpu_device(mconfig, CXD8561CQ, tag, owner, clock, vram_size, cpu)
{
}

cxd8561cq_device::cxd8561cq_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: psxgpu_device(mconfig, CXD8561CQ, tag, owner, clock)
{
}

cxd8654q_device::cxd8654q_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, uint32_t vram_size, psxcpu_device *cpu)
	: psxgpu_device(mconfig, CXD8654Q, tag, owner, clock, vram_size, cpu)
{
}

cxd8654q_device::cxd8654q_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: psxgpu_device(mconfig, CXD8654Q, tag, owner, clock)
{
}

static const int m_p_n_nextpointlist4[] = { 1, 3, 0, 2 };
static const int m_p_n_prevpointlist4[] = { 2, 0, 3, 1 };
static const int m_p_n_nextpointlist4b[] = { 0, 3, 1, 2 };
static const int m_p_n_prevpointlist4b[] = { 0, 2, 3, 1 };
static const int m_p_n_nextpointlist3[] = { 1, 2, 0 };
static const int m_p_n_prevpointlist3[] = { 2, 0, 1 };

#define COORD_X( a ) ( a.sw.l )
#define COORD_Y( a ) ( a.sw.h )
#define S11_COORD_X( a ) util::sext( a.sw.l, 11 )
#define S11_COORD_Y( a ) util::sext( a.sw.h, 11 )
#define SIZE_W( a ) ( a.w.l )
#define SIZE_H( a ) ( a.w.h )
#define BGR_C( a ) ( a.b.h3 )
#define BGR_B( a ) ( a.b.h2 )
#define BGR_G( a ) ( a.b.h )
#define BGR_R( a ) ( a.b.l )
#define TEXTURE_V( a ) ( a.b.h )
#define TEXTURE_U( a ) ( a.b.l )

#if PSXGPU_DEBUG_VIEWER

void psxgpu_device::DebugMeshInit()
{
	int width = screen().width();
	int height = screen().height();

	m_debug.b_mesh = 0;
	m_debug.b_texture = 0;
	m_debug.n_interleave = -1;
	m_debug.b_clear = 1;
	m_debug.n_coord = 0;
	m_debug.n_skip = 0;
	m_debug.mesh = std::make_unique<bitmap_ind16>(width, height );
}

void psxgpu_device::DebugMesh( int n_coordx, int n_coordy )
{
	int width = screen().width();
	int height = screen().height();

	n_coordx += m_n_displaystartx;
	n_coordy += n_displaystarty;

	if( m_debug.b_clear )
	{
		m_debug.mesh->fill(0x0000);
		m_debug.b_clear = 0;
	}

	int n_coord;
	int n_colour = 0x1f;

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
		int32_t n_xstart = m_debug.n_coordx[ n_coord ];
		int32_t n_xend = n_coordx;
		int32_t n_xlen;
		if( n_xend > n_xstart )
		{
			n_xlen = n_xend - n_xstart;
		}
		else
		{
			n_xlen = n_xstart - n_xend;
		}

		int32_t n_ystart = m_debug.n_coordy[ n_coord ];
		int32_t n_yend = n_coordy;
		int32_t n_ylen;
		if( n_yend > n_ystart )
		{
			n_ylen = n_yend - n_ystart;
		}
		else
		{
			n_ylen = n_ystart - n_yend;
		}

		int32_t n_len;
		if( n_xlen > n_ylen )
		{
			n_len = n_xlen;
		}
		else
		{
			n_len = n_ylen;
		}

		PAIR n_x; n_x.sw.h = n_xstart; n_x.sw.l = 0;
		PAIR n_y; n_y.sw.h = n_ystart; n_y.sw.l = 0;

		if( n_len == 0 )
		{
			n_len = 1;
		}

		int32_t n_dx = (int32_t)( ( n_xend << 16 ) - n_x.d ) / n_len;
		int32_t n_dy = (int32_t)( ( n_yend << 16 ) - n_y.d ) / n_len;

		while( n_len > 0 )
		{
			if( (int16_t)n_x.w.h >= 0 &&
				(int16_t)n_y.w.h >= 0 &&
				(int16_t)n_x.w.h <= width - 1 &&
				(int16_t)n_y.w.h <= height - 1 )
			{
				if( m_debug.mesh->pix( n_y.w.h, n_x.w.h ) != 0xffff )
					m_debug.mesh->pix( n_y.w.h, n_x.w.h ) = n_colour;
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

void psxgpu_device::DebugMeshEnd()
{
	m_debug.n_coord = 0;
}

void psxgpu_device::DebugCheckKeys()
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
		FILE *f = fopen( "dump.txt", "w" );
		for( int n_y = 256; n_y < 512; n_y++ )
			for( int n_x = 640; n_x < 1024; n_x++ )
				fprintf( f, "%04u,%04u = %04x\n", n_y, n_x, p_p_vram[ n_y ][ n_x ] );
		fclose( f );
	}
	if( machine().input().code_pressed_once( KEYCODE_S ) )
	{
		popmessage( "saving..." );

		FILE *f = fopen( "VRAM.BIN", "wb" );
		for( int n_y = 0; n_y < 1024; n_y++ )
			fwrite( p_p_vram[ n_y ], 1024 * 2, 1, f );
		fclose( f );
	}
	if( machine().input().code_pressed_once( KEYCODE_L ) )
	{
		popmessage( "loading..." );

		FILE *f = fopen( "VRAM.BIN", "rb" );
		for( int n_y = 0; n_y < 1024; n_y++ )
			fread( p_p_vram[ n_y ], 1024 * 2, 1, f );
		fclose( f );
	}
#endif
}

int psxgpu_device::DebugMeshDisplay( bitmap_rgb32 &bitmap, const rectangle &cliprect )
{
	if( m_debug.b_mesh )
	{
		for( int y = cliprect.min_y; y <= cliprect.max_y; y++ )
			draw_scanline16( bitmap, cliprect.min_x, y, cliprect.max_x + 1 - cliprect.min_x, &m_debug.mesh->pix( y ), pens() );
	}

	m_debug.b_clear = 1;
	return m_debug.b_mesh;
}

int psxgpu_device::DebugTextureDisplay( bitmap_rgb32 &bitmap )
{
	if( m_debug.b_texture )
	{
		int width = screen().width();
		int height = screen().height();

		for( int n_y = 0; n_y < height; n_y++ )
		{
			uint16_t p_n_interleave[ 1024 ];

			for( int n_x = 0; n_x < width; n_x++ )
			{
				int n_xi;
				int n_yi;

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

			draw_scanline16( bitmap, 0, n_y, width, p_n_interleave, pens() );
		}
	}

	return m_debug.b_texture;
}

#endif

void psxgpu_device::updatevisiblearea()
{
	rectangle visarea;
	double refresh;

	if( ( n_gpustatus & ( 1 << 0x14 ) ) != 0 )
	{
		/* pal */
		refresh = 50; // TODO: it's not exactly 50Hz
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
		// refresh rate derived from 53.693175MHz
		// TODO: emulate display timings at lower level
		switch( ( n_gpustatus >> 0x13 ) & 1 )
		{
		case 0:
			refresh = 59.8260978565;
			n_screenheight = 240;
			break;
		case 1:
			refresh = 59.9400523286;
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

#if PSXGPU_DEBUG_VIEWER
	if( m_debug.b_mesh || m_debug.b_texture )
	{
		n_screenheight = 1024;
		n_screenwidth = 1024;
	}
#endif

	visarea.set(0, n_screenwidth - 1, 0, n_screenheight - 1);
	screen().configure(n_screenwidth, n_screenheight, visarea, HZ_TO_ATTOSECONDS(refresh));
}

void psxgpu_device::psx_gpu_init( int n_gputype )
{
	int width = 1024;
	int height = ( vramSize / width ) / sizeof( uint16_t );

	m_n_gputype = n_gputype;

#if PSXGPU_DEBUG_VIEWER
	DebugMeshInit();
#endif

	n_gpustatus = 0x14802000;
	n_gpuinfo = 0;
	n_gpu_buffer_offset = 0;
	n_lightgun_x = 0;
	n_lightgun_y = 0;
	b_reverseflag = 0;

	p_vram = make_unique_clear<uint16_t[]>(width * height );

	for( int n_line = 0; n_line < 1024; n_line++ )
	{
		p_p_vram[ n_line ] = &p_vram[ ( n_line % height ) * width ];
	}

	for( int n_level = 0; n_level < MAX_LEVEL; n_level++ )
	{
		for( int n_shade = 0; n_shade < MAX_SHADE; n_shade++ )
		{
			/* shaded */
			int n_shaded = ( n_level * n_shade ) / MID_SHADE;
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

	for( int n_level = 0; n_level < 0x10000; n_level++ )
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

		/* 24bit color */
		p_n_g0r0[ n_level ] = ( ( ( n_level >> 8 ) & 0xff ) << 8 ) | ( ( ( n_level >> 0 ) & 0xff ) << 16 );
		p_n_b0[ n_level ] = ( ( n_level >> 0 ) & 0xff ) << 0;
		p_n_r1[ n_level ] = ( ( n_level >> 8 ) & 0xff ) << 16;
		p_n_b1g1[ n_level ] = ( ( ( n_level >> 8 ) & 0xff ) << 0 ) | ( ( ( n_level >> 0 ) & 0xff ) << 8 );
	}

	for( int n_level = 0; n_level < MAX_LEVEL; n_level++ )
	{
		for( int n_level2 = 0; n_level2 < MAX_LEVEL; n_level2++ )
		{
			/* add transparency */
			int n_shaded = ( n_level + n_level2 );
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

	save_pointer(NAME(p_vram), width * height );
	save_item(NAME(m_packet.n_entry));
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
	save_item(NAME(m_draw_stp));
	save_item(NAME(m_check_stp));
}

void psxgpu_device::device_post_load()
{
	updatevisiblearea();
}

uint32_t psxgpu_device::update_screen(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint32_t n_x;
	uint32_t n_y;
	int n_top;
	int n_line;
	int n_lines;
	int n_left;
	int n_column;
	int n_columns;
	int n_displaystartx;
	int n_overscantop;
	int n_overscanleft;

#if PSXGPU_DEBUG_VIEWER
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

		n_top = (int32_t)n_vert_disstart - n_overscantop;
		n_lines = (int32_t)n_vert_disend - (int32_t)n_vert_disstart;
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

		n_left = ( ( (int32_t)n_horiz_disstart - n_overscanleft ) * (int32_t)n_screenwidth ) / 2560;
		n_columns = ( ( ( (int32_t)n_horiz_disend - n_horiz_disstart ) * (int32_t)n_screenwidth ) / 2560 );

		/* adjustment to prevent the screen is cut off */
		if( n_left > n_screenwidth - n_columns )
		{
			n_left = n_screenwidth - n_columns;
		}

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
				uint16_t *p_n_src = p_p_vram[ n_y + n_displaystarty ] + 3 * n_x + n_displaystartx;
				uint32_t *p_n_dest = &bitmap.pix(n_y + n_top, n_x + n_left);

				n_column = n_columns;
				while( n_column > 0 )
				{
					uint32_t n_g0r0 = *( p_n_src++ );
					uint32_t n_r1b0 = *( p_n_src++ );
					uint32_t n_b1g1 = *( p_n_src++ );

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
				draw_scanline16( bitmap, n_x + n_left, n_y + n_top, n_columns, p_p_vram[ ( n_y + n_displaystarty ) & 1023 ] + n_x + n_displaystartx, pens() );
				n_y++;
				n_line--;
			}
		}
	}
	return 0;
}

#define WRITE_PIXEL( p ) \
	{ \
		if( !m_check_stp || ( *( p_vram ) & 0x8000 ) == 0 ) \
		{ \
			if( m_draw_stp ) \
				*( p_vram ) = ( p ) | 0x8000; \
			else \
				*( p_vram ) = p; \
		} \
	}

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

void psxgpu_device::decode_tpage( uint32_t tpage )
{
	if( m_n_gputype == 2 )
	{
		n_gpustatus = ( n_gpustatus & 0xffff7800 ) | ( tpage & 0x7ff ) | ( ( tpage & 0x800 ) << 4 );

		m_n_tx = ( tpage & 0x0f ) << 6;
		m_n_ty = ( ( tpage & 0x10 ) << 4 ) | ( ( tpage & 0x800 ) >> 2 );
		n_abr = ( tpage & 0x60 ) >> 5;
		n_tp = ( tpage & 0x180 ) >> 7;
		n_ix = ( tpage & 0x1000 ) >> 12;
		n_iy = ( tpage & 0x2000 ) >> 13;
		n_ti = 0;
		if( ( tpage & ~0x39ff ) != 0 )
		{
			LOG("not handled: draw mode %08x\n", tpage & ~0x39ff);
		}
		if( n_tp == 3 )
		{
			logerror("not handled: tp == 3\n");
		}
	}
	else
	{
		// TODO: confirm status bits on real type 1 gpu
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
			LOG("not handled: draw mode %08x\n", tpage & ~0x27ef);
		}
		if( n_tp == 3 )
		{
			logerror("not handled: tp == 3\n");
		}
		else if( n_tp == 2 && n_ti != 0 )
		{
			logerror("not handled: interleaved 15 bit texture\n");
		}
	}
}

#define SPRITESETUP \
	int n_dv; \
	if( n_iy != 0 ) \
	{ \
		n_dv = -1; \
	} \
	else \
	{ \
		n_dv = 1; \
	} \
	int n_du; \
	if( n_ix != 0 ) \
	{ \
		n_du = -1; \
	} \
	else \
	{ \
		n_du = 1; \
	}

#define TRANSPARENCYSETUP \
	uint16_t *p_n_f = p_n_f1; \
	uint16_t *p_n_redb = p_n_redb1; \
	uint16_t *p_n_greenb = p_n_greenb1; \
	uint16_t *p_n_blueb = p_n_blueb1; \
	uint16_t *p_n_redtrans = p_n_redaddtrans; \
	uint16_t *p_n_greentrans = p_n_greenaddtrans; \
	uint16_t *p_n_bluetrans = p_n_blueaddtrans; \
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
			LOGMASKED(LOG_TRANSPARENCY, "Transparency Mode: 0.5*B + 0.5*F\n"); \
			break; \
		case 0x01: \
			p_n_f = p_n_f1; \
			p_n_redb = p_n_redb1; \
			p_n_greenb = p_n_greenb1; \
			p_n_blueb = p_n_blueb1; \
			p_n_redtrans = p_n_redaddtrans; \
			p_n_greentrans = p_n_greenaddtrans; \
			p_n_bluetrans = p_n_blueaddtrans; \
			LOGMASKED(LOG_TRANSPARENCY, "Transparency Mode: 1.0*B + 1.0*F\n"); \
			break; \
		case 0x02: \
			p_n_f = p_n_f1; \
			p_n_redb = p_n_redb1; \
			p_n_greenb = p_n_greenb1; \
			p_n_blueb = p_n_blueb1; \
			p_n_redtrans = p_n_redsubtrans; \
			p_n_greentrans = p_n_greensubtrans; \
			p_n_bluetrans = p_n_bluesubtrans; \
			LOGMASKED(LOG_TRANSPARENCY, "Transparency Mode: 1.0*B - 1.0*F\n"); \
			break; \
		case 0x03: \
			p_n_f = p_n_f025; \
			p_n_redb = p_n_redb1; \
			p_n_greenb = p_n_greenb1; \
			p_n_blueb = p_n_blueb1; \
			p_n_redtrans = p_n_redaddtrans; \
			p_n_greentrans = p_n_greenaddtrans; \
			p_n_bluetrans = p_n_blueaddtrans; \
			LOGMASKED(LOG_TRANSPARENCY, "Transparency Mode: 1.0*B + 0.25*F\n"); \
			break; \
		} \
		break; \
	}

#define SOLIDSETUP \
	TRANSPARENCYSETUP

#define TEXTURESETUP \
	int n_tx = m_n_tx; \
	int n_ty = m_n_ty; \
	uint16_t *p_clut = p_p_vram[ n_cluty ] + n_clutx; \
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
	if( n_distance > ( (int32_t)n_drawarea_x2 - drawx ) + 1 ) \
	{ \
		n_distance = ( n_drawarea_x2 - drawx ) + 1; \
	} \
	uint16_t *p_vram = p_p_vram[ drawy ] + drawx; \
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
				p_n_blueshade[ MID_LEVEL | n_b.w.h ] ) \
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
				p_n_bluetrans[ p_n_f[ MID_LEVEL | n_b.w.h ] | p_n_blueb[ *( p_vram ) ] ] ) \
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
		uint16_t n_bgr = p_clut[ ( *( p_p_vram[ n_ty + TXV ] + n_tx + ( TXU >> 2 ) ) >> ( ( TXU & 0x03 ) << 2 ) ) & 0x0f ];

#define TEXTURE8BIT( TXV, TXU ) \
	TEXTURE_LOOP \
		uint16_t n_bgr = p_clut[ ( *( p_p_vram[ n_ty + TXV ] + n_tx + ( TXU >> 1 ) ) >> ( ( TXU & 0x01 ) << 3 ) ) & 0xff ];

#define TEXTURE15BIT( TXV, TXU ) \
	TEXTURE_LOOP \
		uint16_t n_bgr = *( p_p_vram[ n_ty + TXV ] + n_tx + TXU );

#define TEXTUREWINDOW4BIT( TXV, TXU ) TEXTURE4BIT( ( TXV & n_twh ), ( TXU & n_tww ) )
#define TEXTUREWINDOW8BIT( TXV, TXU ) TEXTURE8BIT( ( TXV & n_twh ), ( TXU & n_tww ) )
#define TEXTUREWINDOW15BIT( TXV, TXU ) TEXTURE15BIT( ( TXV & n_twh ), ( TXU & n_tww ) )

#define TEXTUREINTERLEAVED4BIT( TXV, TXU ) \
	TEXTURE_LOOP \
		int n_xi = ( ( TXU >> 2 ) & ~0x3c ) + ( ( TXV << 2 ) & 0x3c ); \
		int n_yi = ( TXV & ~0xf ) + ( ( TXU >> 4 ) & 0xf ); \
		uint16_t n_bgr = p_clut[ ( *( p_p_vram[ n_ty + n_yi ] + n_tx + n_xi ) >> ( ( TXU & 0x03 ) << 2 ) ) & 0x0f ];

#define TEXTUREINTERLEAVED8BIT( TXV, TXU ) \
	TEXTURE_LOOP \
		int n_xi = ( ( TXU >> 1 ) & ~0x78 ) + ( ( TXU << 2 ) & 0x40 ) + ( ( TXV << 3 ) & 0x38 ); \
		int n_yi = ( TXV & ~0x7 ) + ( ( TXU >> 5 ) & 0x7 ); \
		uint16_t n_bgr = p_clut[ ( *( p_p_vram[ n_ty + n_yi ] + n_tx + n_xi ) >> ( ( TXU & 0x01 ) << 3 ) ) & 0xff ];

#define TEXTUREINTERLEAVED15BIT( TXV, TXU ) \
	TEXTURE_LOOP \
		int n_xi = TXU; \
		int n_yi = TXV; \
		uint16_t n_bgr = *( p_p_vram[ n_ty + n_yi ] + n_tx + n_xi );

#define TEXTUREWINDOWINTERLEAVED4BIT( TXV, TXU ) TEXTUREINTERLEAVED4BIT( ( TXV & n_twh ), ( TXU & n_tww ) )
#define TEXTUREWINDOWINTERLEAVED8BIT( TXV, TXU ) TEXTUREINTERLEAVED8BIT( ( TXV & n_twh ), ( TXU & n_tww ) )
#define TEXTUREWINDOWINTERLEAVED15BIT( TXV, TXU ) TEXTUREINTERLEAVED15BIT( ( TXV & n_twh ), ( TXU & n_tww ) )

#define SHADEDPIXEL( PIXELUPDATE ) \
		if( n_bgr != 0 ) \
		{ \
			WRITE_PIXEL( \
				p_n_redshade[ p_n_redlevel[ n_bgr ] | n_r.w.h ] | \
				p_n_greenshade[ p_n_greenlevel[ n_bgr ] | n_g.w.h ] | \
				p_n_blueshade[ p_n_bluelevel[ n_bgr ] | n_b.w.h ] | \
				( n_bgr & 0x8000 ) ) \
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
					p_n_bluetrans[ p_n_f[ p_n_bluelevel[ n_bgr ] | n_b.w.h ] | p_n_blueb[ *( p_vram ) ] ] | \
					0x8000 ) \
			} \
			else \
			{ \
				WRITE_PIXEL( \
					p_n_redshade[ p_n_redlevel[ n_bgr ] | n_r.w.h ] | \
					p_n_greenshade[ p_n_greenlevel[ n_bgr ] | n_g.w.h ] | \
					p_n_blueshade[ p_n_bluelevel[ n_bgr ] | n_b.w.h ] ) \
			} \
		} \
		p_vram++; \
		PIXELUPDATE \
		n_distance--; \
	TEXTURE_ENDLOOP

#define TEXTUREFILL( PIXELUPDATE, TXU, TXV ) \
	if( n_distance > ( (int32_t)n_drawarea_x2 - drawx ) + 1 ) \
	{ \
		n_distance = ( n_drawarea_x2 - drawx ) + 1; \
	} \
	uint16_t *p_vram = p_p_vram[ drawy ] + drawx; \
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
	a.sw.l = S11_COORD_X( a ); \
	a.sw.h = S11_COORD_Y( a );

static inline int CullVertex( int a, int b )
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
	for( int n_point = 0; n_point < n_points; n_point++ ) \
	{ \
		GET_COORD( m_packet.PacketType.vertex[ n_point ].n_coord ); \
	} \
	\
	const int *p_n_rightpointlist; \
	const int *p_n_leftpointlist; \
	int n_leftpoint = 0; \
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
	for( int n_point = n_leftpoint + 1; n_point < n_points; n_point++ ) \
	{ \
		if( COORD_Y( m_packet.PacketType.vertex[ n_point ].n_coord ) < COORD_Y( m_packet.PacketType.vertex[ n_leftpoint ].n_coord ) || \
			( COORD_Y( m_packet.PacketType.vertex[ n_point ].n_coord ) == COORD_Y( m_packet.PacketType.vertex[ n_leftpoint ].n_coord ) && \
			COORD_X( m_packet.PacketType.vertex[ n_point ].n_coord ) < COORD_X( m_packet.PacketType.vertex[ n_leftpoint ].n_coord ) ) ) \
		{ \
			n_leftpoint = n_point; \
		} \
	} \
	int n_rightpoint = n_leftpoint;

void psxgpu_device::FlatPolygon( int n_points )
{
#if PSXGPU_DEBUG_VIEWER
	if( m_debug.n_skip == 1 )
	{
		return;
	}
	for( int n_point = 0; n_point < n_points; n_point++ )
	{
		DebugMesh( S11_COORD_X( m_packet.FlatPolygon.vertex[ n_point ].n_coord ) + n_drawoffset_x, S11_COORD_Y( m_packet.FlatPolygon.vertex[ n_point ].n_coord ) + n_drawoffset_y );
	}
	DebugMeshEnd();
#endif

	uint8_t n_cmd = BGR_C( m_packet.FlatPolygon.n_bgr );

	PAIR n_cx1; n_cx1.d = 0;
	PAIR n_cx2; n_cx2.d = 0;

	SOLIDSETUP

	PAIR n_r; n_r.w.h = BGR_R( m_packet.FlatPolygon.n_bgr ); n_r.w.l = 0;
	PAIR n_g; n_g.w.h = BGR_G( m_packet.FlatPolygon.n_bgr ); n_g.w.l = 0;
	PAIR n_b; n_b.w.h = BGR_B( m_packet.FlatPolygon.n_bgr ); n_b.w.l = 0;

	FINDTOPLEFT( FlatPolygon )

	int32_t n_dx1 = 0;
	int32_t n_dx2 = 0;

	int16_t n_y = COORD_Y( m_packet.FlatPolygon.vertex[ n_rightpoint ].n_coord );

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

			int32_t n_distance = COORD_Y( m_packet.FlatPolygon.vertex[ n_leftpoint ].n_coord ) - n_y;
			if( n_distance < 1 )
			{
				break;
			}

			n_dx1 = (int32_t)( ( COORD_X( m_packet.FlatPolygon.vertex[ n_leftpoint ].n_coord ) << 16 ) - n_cx1.d ) / n_distance;
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

			int32_t n_distance = COORD_Y( m_packet.FlatPolygon.vertex[ n_rightpoint ].n_coord ) - n_y;
			if( n_distance < 1 )
			{
				break;
			}

			n_dx2 = (int32_t)( ( COORD_X( m_packet.FlatPolygon.vertex[ n_rightpoint ].n_coord ) << 16 ) - n_cx2.d ) / n_distance;
		}

		int drawy = n_y + n_drawoffset_y;

		if( (int16_t)n_cx1.sw.h != (int16_t)n_cx2.sw.h && drawy >= (int32_t)n_drawarea_y1 && drawy <= (int32_t)n_drawarea_y2 )
		{
			int16_t n_x;
			int32_t n_distance;

			if( (int16_t)n_cx1.sw.h < (int16_t)n_cx2.sw.h )
			{
				n_x = n_cx1.sw.h;
				n_distance = (int16_t)n_cx2.sw.h - n_x;
			}
			else
			{
				n_x = n_cx2.sw.h;
				n_distance = (int16_t)n_cx1.sw.h - n_x;
			}

			int drawx = n_x + n_drawoffset_x;

			if( ( (int32_t)n_drawarea_x1 - drawx ) > 0 )
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
#if PSXGPU_DEBUG_VIEWER
	if( m_debug.n_skip == 2 )
	{
		return;
	}
	for( int n_point = 0; n_point < n_points; n_point++ )
	{
		DebugMesh( S11_COORD_X( m_packet.FlatTexturedPolygon.vertex[ n_point ].n_coord ) + n_drawoffset_x, S11_COORD_Y( m_packet.FlatTexturedPolygon.vertex[ n_point ].n_coord ) + n_drawoffset_y );
	}
	DebugMeshEnd();
#endif

	uint8_t n_cmd = BGR_C( m_packet.FlatTexturedPolygon.n_bgr );

	uint32_t n_clutx = ( m_packet.FlatTexturedPolygon.vertex[ 0 ].n_texture.w.h & 0x3f ) << 4;
	uint32_t n_cluty = ( m_packet.FlatTexturedPolygon.vertex[ 0 ].n_texture.w.h >> 6 ) & 0x3ff;

	PAIR n_cx1; n_cx1.d = 0;
	PAIR n_cu1; n_cu1.d = 0;
	PAIR n_cv1; n_cv1.d = 0;
	PAIR n_cx2; n_cx2.d = 0;
	PAIR n_cu2; n_cu2.d = 0;
	PAIR n_cv2; n_cv2.d = 0;

	decode_tpage( m_packet.FlatTexturedPolygon.vertex[ 1 ].n_texture.w.h );
	TEXTURESETUP

	PAIR n_r; n_r.w.h = n_cmd & 0x01 ? 0x80 : BGR_R( m_packet.FlatTexturedPolygon.n_bgr ); n_r.w.l = 0;
	PAIR n_g; n_g.w.h = n_cmd & 0x01 ? 0x80 : BGR_G( m_packet.FlatTexturedPolygon.n_bgr ); n_g.w.l = 0;
	PAIR n_b; n_b.w.h = n_cmd & 0x01 ? 0x80 : BGR_B( m_packet.FlatTexturedPolygon.n_bgr ); n_b.w.l = 0;

	FINDTOPLEFT( FlatTexturedPolygon )

	int32_t n_dx1 = 0;
	int32_t n_dx2 = 0;
	int32_t n_du1 = 0;
	int32_t n_du2 = 0;
	int32_t n_dv1 = 0;
	int32_t n_dv2 = 0;

	int16_t n_y = COORD_Y( m_packet.FlatTexturedPolygon.vertex[ n_rightpoint ].n_coord );

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

			int32_t n_distance = COORD_Y( m_packet.FlatTexturedPolygon.vertex[ n_leftpoint ].n_coord ) - n_y;
			if( n_distance < 1 )
			{
				break;
			}

			n_dx1 = (int32_t)( ( COORD_X( m_packet.FlatTexturedPolygon.vertex[ n_leftpoint ].n_coord ) << 16 ) - n_cx1.d ) / n_distance;
			n_du1 = (int32_t)( ( TEXTURE_U( m_packet.FlatTexturedPolygon.vertex[ n_leftpoint ].n_texture ) << 16 ) - n_cu1.d ) / n_distance;
			n_dv1 = (int32_t)( ( TEXTURE_V( m_packet.FlatTexturedPolygon.vertex[ n_leftpoint ].n_texture ) << 16 ) - n_cv1.d ) / n_distance;
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

			int32_t n_distance = COORD_Y( m_packet.FlatTexturedPolygon.vertex[ n_rightpoint ].n_coord ) - n_y;
			if( n_distance < 1 )
			{
				break;
			}

			n_dx2 = (int32_t)( ( COORD_X( m_packet.FlatTexturedPolygon.vertex[ n_rightpoint ].n_coord ) << 16 ) - n_cx2.d ) / n_distance;
			n_du2 = (int32_t)( ( TEXTURE_U( m_packet.FlatTexturedPolygon.vertex[ n_rightpoint ].n_texture ) << 16 ) - n_cu2.d ) / n_distance;
			n_dv2 = (int32_t)( ( TEXTURE_V( m_packet.FlatTexturedPolygon.vertex[ n_rightpoint ].n_texture ) << 16 ) - n_cv2.d ) / n_distance;
		}

		int drawy = n_y + n_drawoffset_y;

		if( (int16_t)n_cx1.sw.h != (int16_t)n_cx2.sw.h && drawy >= (int32_t)n_drawarea_y1 && drawy <= (int32_t)n_drawarea_y2 )
		{
			int16_t n_x;
			int32_t n_distance;
			PAIR n_u;
			PAIR n_v;
			int32_t n_du;
			int32_t n_dv;

			if( (int16_t)n_cx1.sw.h < (int16_t)n_cx2.sw.h )
			{
				n_x = n_cx1.sw.h;
				n_distance = (int16_t)n_cx2.sw.h - n_x;

				n_u.d = n_cu1.d;
				n_v.d = n_cv1.d;
				n_du = (int32_t)( n_cu2.d - n_cu1.d ) / n_distance;
				n_dv = (int32_t)( n_cv2.d - n_cv1.d ) / n_distance;
			}
			else
			{
				n_x = n_cx2.sw.h;
				n_distance = (int16_t)n_cx1.sw.h - n_x;

				n_u.d = n_cu2.d;
				n_v.d = n_cv2.d;
				n_du = (int32_t)( n_cu1.d - n_cu2.d ) / n_distance;
				n_dv = (int32_t)( n_cv1.d - n_cv2.d ) / n_distance;
			}

			int drawx = n_x + n_drawoffset_x;

			if( ( (int32_t)n_drawarea_x1 - drawx ) > 0 )
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
#if PSXGPU_DEBUG_VIEWER
	if( m_debug.n_skip == 3 )
	{
		return;
	}
	for( int n_point = 0; n_point < n_points; n_point++ )
	{
		DebugMesh( S11_COORD_X( m_packet.GouraudPolygon.vertex[ n_point ].n_coord ) + n_drawoffset_x, S11_COORD_Y( m_packet.GouraudPolygon.vertex[ n_point ].n_coord ) + n_drawoffset_y );
	}
	DebugMeshEnd();
#endif

	uint8_t n_cmd = BGR_C( m_packet.GouraudPolygon.vertex[ 0 ].n_bgr );

	PAIR n_cx1; n_cx1.d = 0;
	PAIR n_cr1; n_cr1.d = 0;
	PAIR n_cg1; n_cg1.d = 0;
	PAIR n_cb1; n_cb1.d = 0;
	PAIR n_cx2; n_cx2.d = 0;
	PAIR n_cr2; n_cr2.d = 0;
	PAIR n_cg2; n_cg2.d = 0;
	PAIR n_cb2; n_cb2.d = 0;

	SOLIDSETUP

	FINDTOPLEFT( GouraudPolygon )

	int32_t n_dx1 = 0;
	int32_t n_dx2 = 0;
	int32_t n_dr1 = 0;
	int32_t n_dr2 = 0;
	int32_t n_dg1 = 0;
	int32_t n_dg2 = 0;
	int32_t n_db1 = 0;
	int32_t n_db2 = 0;

	int16_t n_y = COORD_Y( m_packet.GouraudPolygon.vertex[ n_rightpoint ].n_coord );

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

			int32_t n_distance = COORD_Y( m_packet.GouraudPolygon.vertex[ n_leftpoint ].n_coord ) - n_y;
			if( n_distance < 1 )
			{
				break;
			}

			n_dx1 = (int32_t)( ( COORD_X( m_packet.GouraudPolygon.vertex[ n_leftpoint ].n_coord ) << 16 ) - n_cx1.d ) / n_distance;
			n_dr1 = (int32_t)( ( BGR_R( m_packet.GouraudPolygon.vertex[ n_leftpoint ].n_bgr ) << 16 ) - n_cr1.d ) / n_distance;
			n_dg1 = (int32_t)( ( BGR_G( m_packet.GouraudPolygon.vertex[ n_leftpoint ].n_bgr ) << 16 ) - n_cg1.d ) / n_distance;
			n_db1 = (int32_t)( ( BGR_B( m_packet.GouraudPolygon.vertex[ n_leftpoint ].n_bgr ) << 16 ) - n_cb1.d ) / n_distance;
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

			int32_t n_distance = COORD_Y( m_packet.GouraudPolygon.vertex[ n_rightpoint ].n_coord ) - n_y;
			if( n_distance < 1 )
			{
				break;
			}

			n_dx2 = (int32_t)( ( COORD_X( m_packet.GouraudPolygon.vertex[ n_rightpoint ].n_coord ) << 16 ) - n_cx2.d ) / n_distance;
			n_dr2 = (int32_t)( ( BGR_R( m_packet.GouraudPolygon.vertex[ n_rightpoint ].n_bgr ) << 16 ) - n_cr2.d ) / n_distance;
			n_dg2 = (int32_t)( ( BGR_G( m_packet.GouraudPolygon.vertex[ n_rightpoint ].n_bgr ) << 16 ) - n_cg2.d ) / n_distance;
			n_db2 = (int32_t)( ( BGR_B( m_packet.GouraudPolygon.vertex[ n_rightpoint ].n_bgr ) << 16 ) - n_cb2.d ) / n_distance;
		}

		int drawy = n_y + n_drawoffset_y;

		if( (int16_t)n_cx1.sw.h != (int16_t)n_cx2.sw.h && drawy >= (int32_t)n_drawarea_y1 && drawy <= (int32_t)n_drawarea_y2 )
		{
			int16_t n_x;
			int32_t n_distance;
			PAIR n_r;
			PAIR n_g;
			PAIR n_b;
			int32_t n_dr;
			int32_t n_dg;
			int32_t n_db;

			if( (int16_t)n_cx1.sw.h < (int16_t)n_cx2.sw.h )
			{
				n_x = n_cx1.sw.h;
				n_distance = (int16_t)n_cx2.sw.h - n_x;

				n_r.d = n_cr1.d;
				n_g.d = n_cg1.d;
				n_b.d = n_cb1.d;
				n_dr = (int32_t)( n_cr2.d - n_cr1.d ) / n_distance;
				n_dg = (int32_t)( n_cg2.d - n_cg1.d ) / n_distance;
				n_db = (int32_t)( n_cb2.d - n_cb1.d ) / n_distance;
			}
			else
			{
				n_x = n_cx2.sw.h;
				n_distance = (int16_t)n_cx1.sw.h - n_x;

				n_r.d = n_cr2.d;
				n_g.d = n_cg2.d;
				n_b.d = n_cb2.d;
				n_dr = (int32_t)( n_cr1.d - n_cr2.d ) / n_distance;
				n_dg = (int32_t)( n_cg1.d - n_cg2.d ) / n_distance;
				n_db = (int32_t)( n_cb1.d - n_cb2.d ) / n_distance;
			}

			int drawx = n_x + n_drawoffset_x;

			if( ( (int32_t)n_drawarea_x1 - drawx ) > 0 )
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
#if PSXGPU_DEBUG_VIEWER
	if( m_debug.n_skip == 4 )
	{
		return;
	}
	for( int n_point = 0; n_point < n_points; n_point++ )
	{
		DebugMesh( S11_COORD_X( m_packet.GouraudTexturedPolygon.vertex[ n_point ].n_coord ) + n_drawoffset_x, S11_COORD_Y( m_packet.GouraudTexturedPolygon.vertex[ n_point ].n_coord ) + n_drawoffset_y );
	}
	DebugMeshEnd();
#endif

	uint8_t n_cmd = BGR_C( m_packet.GouraudTexturedPolygon.vertex[ 0 ].n_bgr );

	uint32_t n_clutx = ( m_packet.GouraudTexturedPolygon.vertex[ 0 ].n_texture.w.h & 0x3f ) << 4;
	uint32_t n_cluty = ( m_packet.GouraudTexturedPolygon.vertex[ 0 ].n_texture.w.h >> 6 ) & 0x3ff;

	PAIR n_cx1; n_cx1.d = 0;
	PAIR n_cr1; n_cr1.d = 0;
	PAIR n_cg1; n_cg1.d = 0;
	PAIR n_cb1; n_cb1.d = 0;
	PAIR n_cu1; n_cu1.d = 0;
	PAIR n_cv1; n_cv1.d = 0;
	PAIR n_cx2; n_cx2.d = 0;
	PAIR n_cr2; n_cr2.d = 0;
	PAIR n_cg2; n_cg2.d = 0;
	PAIR n_cb2; n_cb2.d = 0;
	PAIR n_cu2; n_cu2.d = 0;
	PAIR n_cv2; n_cv2.d = 0;

	decode_tpage( m_packet.GouraudTexturedPolygon.vertex[ 1 ].n_texture.w.h );
	TEXTURESETUP

	FINDTOPLEFT( GouraudTexturedPolygon )

	int32_t n_dx1 = 0;
	int32_t n_dx2 = 0;
	int32_t n_du1 = 0;
	int32_t n_du2 = 0;
	int32_t n_dr1 = 0;
	int32_t n_dr2 = 0;
	int32_t n_dg1 = 0;
	int32_t n_dg2 = 0;
	int32_t n_db1 = 0;
	int32_t n_db2 = 0;
	int32_t n_dv1 = 0;
	int32_t n_dv2 = 0;

	int16_t n_y = COORD_Y( m_packet.GouraudTexturedPolygon.vertex[ n_rightpoint ].n_coord );

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
			n_cr1.w.h = n_cmd & 0x01 ? 0x80 : BGR_R( m_packet.GouraudTexturedPolygon.vertex[ n_leftpoint ].n_bgr ); n_cr1.w.l = 0;
			n_cg1.w.h = n_cmd & 0x01 ? 0x80 : BGR_G( m_packet.GouraudTexturedPolygon.vertex[ n_leftpoint ].n_bgr ); n_cg1.w.l = 0;
			n_cb1.w.h = n_cmd & 0x01 ? 0x80 : BGR_B( m_packet.GouraudTexturedPolygon.vertex[ n_leftpoint ].n_bgr ); n_cb1.w.l = 0;
			n_cu1.w.h = TEXTURE_U( m_packet.GouraudTexturedPolygon.vertex[ n_leftpoint ].n_texture ); n_cu1.w.l = 0;
			n_cv1.w.h = TEXTURE_V( m_packet.GouraudTexturedPolygon.vertex[ n_leftpoint ].n_texture ); n_cv1.w.l = 0;
			n_leftpoint = p_n_leftpointlist[ n_leftpoint ];

			int32_t n_distance = COORD_Y( m_packet.GouraudTexturedPolygon.vertex[ n_leftpoint ].n_coord ) - n_y;
			if( n_distance < 1 )
			{
				break;
			}

			n_dx1 = (int32_t)( ( COORD_X( m_packet.GouraudTexturedPolygon.vertex[ n_leftpoint ].n_coord ) << 16 ) - n_cx1.d ) / n_distance;
			n_dr1 = n_cmd & 0x01 ? 0 : (int32_t)( ( BGR_R( m_packet.GouraudTexturedPolygon.vertex[ n_leftpoint ].n_bgr ) << 16 ) - n_cr1.d ) / n_distance;
			n_dg1 = n_cmd & 0x01 ? 0 : (int32_t)( ( BGR_G( m_packet.GouraudTexturedPolygon.vertex[ n_leftpoint ].n_bgr ) << 16 ) - n_cg1.d ) / n_distance;
			n_db1 = n_cmd & 0x01 ? 0 : (int32_t)( ( BGR_B( m_packet.GouraudTexturedPolygon.vertex[ n_leftpoint ].n_bgr ) << 16 ) - n_cb1.d ) / n_distance;
			n_du1 = (int32_t)( ( TEXTURE_U( m_packet.GouraudTexturedPolygon.vertex[ n_leftpoint ].n_texture ) << 16 ) - n_cu1.d ) / n_distance;
			n_dv1 = (int32_t)( ( TEXTURE_V( m_packet.GouraudTexturedPolygon.vertex[ n_leftpoint ].n_texture ) << 16 ) - n_cv1.d ) / n_distance;
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
			n_cr2.w.h = n_cmd & 0x01 ? 0x80 : BGR_R( m_packet.GouraudTexturedPolygon.vertex[ n_rightpoint ].n_bgr ); n_cr2.w.l = 0;
			n_cg2.w.h = n_cmd & 0x01 ? 0x80 : BGR_G( m_packet.GouraudTexturedPolygon.vertex[ n_rightpoint ].n_bgr ); n_cg2.w.l = 0;
			n_cb2.w.h = n_cmd & 0x01 ? 0x80 : BGR_B( m_packet.GouraudTexturedPolygon.vertex[ n_rightpoint ].n_bgr ); n_cb2.w.l = 0;
			n_cu2.w.h = TEXTURE_U( m_packet.GouraudTexturedPolygon.vertex[ n_rightpoint ].n_texture ); n_cu2.w.l = 0;
			n_cv2.w.h = TEXTURE_V( m_packet.GouraudTexturedPolygon.vertex[ n_rightpoint ].n_texture ); n_cv2.w.l = 0;
			n_rightpoint = p_n_rightpointlist[ n_rightpoint ];

			int32_t n_distance = COORD_Y( m_packet.GouraudTexturedPolygon.vertex[ n_rightpoint ].n_coord ) - n_y;
			if( n_distance < 1 )
			{
				break;
			}

			n_dx2 = (int32_t)( ( COORD_X( m_packet.GouraudTexturedPolygon.vertex[ n_rightpoint ].n_coord ) << 16 ) - n_cx2.d ) / n_distance;
			n_dr2 = n_cmd & 0x01 ? 0 : (int32_t)( ( BGR_R( m_packet.GouraudTexturedPolygon.vertex[ n_rightpoint ].n_bgr ) << 16 ) - n_cr2.d ) / n_distance;
			n_dg2 = n_cmd & 0x01 ? 0 : (int32_t)( ( BGR_G( m_packet.GouraudTexturedPolygon.vertex[ n_rightpoint ].n_bgr ) << 16 ) - n_cg2.d ) / n_distance;
			n_db2 = n_cmd & 0x01 ? 0 : (int32_t)( ( BGR_B( m_packet.GouraudTexturedPolygon.vertex[ n_rightpoint ].n_bgr ) << 16 ) - n_cb2.d ) / n_distance;
			n_du2 = (int32_t)( ( TEXTURE_U( m_packet.GouraudTexturedPolygon.vertex[ n_rightpoint ].n_texture ) << 16 ) - n_cu2.d ) / n_distance;
			n_dv2 = (int32_t)( ( TEXTURE_V( m_packet.GouraudTexturedPolygon.vertex[ n_rightpoint ].n_texture ) << 16 ) - n_cv2.d ) / n_distance;
		}

		int drawy = n_y + n_drawoffset_y;

		if( (int16_t)n_cx1.sw.h != (int16_t)n_cx2.sw.h && drawy >= (int32_t)n_drawarea_y1 && drawy <= (int32_t)n_drawarea_y2 )
		{
			int16_t n_x;
			int32_t n_distance;
			PAIR n_r;
			PAIR n_g;
			PAIR n_b;
			PAIR n_u;
			PAIR n_v;
			int32_t n_dr;
			int32_t n_dg;
			int32_t n_db;
			int32_t n_du;
			int32_t n_dv;

			if( (int16_t)n_cx1.sw.h < (int16_t)n_cx2.sw.h )
			{
				n_x = n_cx1.sw.h;
				n_distance = (int16_t)n_cx2.sw.h - n_x;

				n_r.d = n_cr1.d;
				n_g.d = n_cg1.d;
				n_b.d = n_cb1.d;
				n_u.d = n_cu1.d;
				n_v.d = n_cv1.d;
				n_dr = (int32_t)( n_cr2.d - n_cr1.d ) / n_distance;
				n_dg = (int32_t)( n_cg2.d - n_cg1.d ) / n_distance;
				n_db = (int32_t)( n_cb2.d - n_cb1.d ) / n_distance;
				n_du = (int32_t)( n_cu2.d - n_cu1.d ) / n_distance;
				n_dv = (int32_t)( n_cv2.d - n_cv1.d ) / n_distance;
			}
			else
			{
				n_x = n_cx2.sw.h;
				n_distance = (int16_t)n_cx1.sw.h - n_x;

				n_r.d = n_cr2.d;
				n_g.d = n_cg2.d;
				n_b.d = n_cb2.d;
				n_u.d = n_cu2.d;
				n_v.d = n_cv2.d;
				n_dr = (int32_t)( n_cr1.d - n_cr2.d ) / n_distance;
				n_dg = (int32_t)( n_cg1.d - n_cg2.d ) / n_distance;
				n_db = (int32_t)( n_cb1.d - n_cb2.d ) / n_distance;
				n_du = (int32_t)( n_cu1.d - n_cu2.d ) / n_distance;
				n_dv = (int32_t)( n_cv1.d - n_cv2.d ) / n_distance;
			}

			int drawx = n_x + n_drawoffset_x;

			if( ( (int32_t)n_drawarea_x1 - drawx ) > 0 )
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

void psxgpu_device::MonochromeLine()
{
#if PSXGPU_DEBUG_VIEWER
	if( m_debug.n_skip == 5 )
	{
		return;
	}
	DebugMesh( S11_COORD_X( m_packet.MonochromeLine.vertex[ 0 ].n_coord ) + n_drawoffset_x, S11_COORD_Y( m_packet.MonochromeLine.vertex[ 0 ].n_coord ) + n_drawoffset_y );
	DebugMesh( S11_COORD_X( m_packet.MonochromeLine.vertex[ 1 ].n_coord ) + n_drawoffset_x, S11_COORD_Y( m_packet.MonochromeLine.vertex[ 1 ].n_coord ) + n_drawoffset_y );
	DebugMeshEnd();
#endif

	int32_t n_xstart = S11_COORD_X( m_packet.MonochromeLine.vertex[ 0 ].n_coord );
	int32_t n_xend = S11_COORD_X( m_packet.MonochromeLine.vertex[ 1 ].n_coord );
	int32_t n_ystart = S11_COORD_Y( m_packet.MonochromeLine.vertex[ 0 ].n_coord );
	int32_t n_yend = S11_COORD_Y( m_packet.MonochromeLine.vertex[ 1 ].n_coord );

	uint8_t n_cmd = BGR_C( m_packet.MonochromeLine.n_bgr );
	uint8_t n_r = BGR_R( m_packet.MonochromeLine.n_bgr );
	uint8_t n_g = BGR_G( m_packet.MonochromeLine.n_bgr );
	uint8_t n_b = BGR_B( m_packet.MonochromeLine.n_bgr );

	TRANSPARENCYSETUP

	int32_t n_xlen;
	if( n_xend > n_xstart )
	{
		n_xlen = n_xend - n_xstart;
	}
	else
	{
		n_xlen = n_xstart - n_xend;
	}

	int32_t n_ylen;
	if( n_yend > n_ystart )
	{
		n_ylen = n_yend - n_ystart;
	}
	else
	{
		n_ylen = n_ystart - n_yend;
	}

	int32_t n_len;
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

	PAIR n_x; n_x.sw.h = n_xstart; n_x.sw.l = 0;
	PAIR n_y; n_y.sw.h = n_ystart; n_y.sw.l = 0;

	int32_t n_dx = (int32_t)( ( n_xend << 16 ) - n_x.d ) / n_len;
	int32_t n_dy = (int32_t)( ( n_yend << 16 ) - n_y.d ) / n_len;

	while( n_len > 0 )
	{
		int drawx = n_x.sw.h + n_drawoffset_x;
		int drawy = n_y.sw.h + n_drawoffset_y;

		if( drawx >= (int32_t)n_drawarea_x1 && drawy >= (int32_t)n_drawarea_y1 &&
			drawx <= (int32_t)n_drawarea_x2 && drawy <= (int32_t)n_drawarea_y2 )
		{
			uint16_t *p_vram = p_p_vram[ drawy ] + drawx;

			switch( n_cmd & 0x02 )
			{
			case 0x00:
				/* transparency off */
				WRITE_PIXEL(
					p_n_redshade[ MID_LEVEL | n_r ] |
					p_n_greenshade[ MID_LEVEL | n_g ] |
					p_n_blueshade[ MID_LEVEL | n_b ] )
				break;
			case 0x02:
				/* transparency on */
				WRITE_PIXEL(
					p_n_redtrans[ p_n_f[ MID_LEVEL | n_r ] | p_n_redb[ *( p_vram ) ] ] |
					p_n_greentrans[ p_n_f[ MID_LEVEL | n_g ] | p_n_greenb[ *( p_vram ) ] ] |
					p_n_bluetrans[ p_n_f[ MID_LEVEL | n_b ] | p_n_blueb[ *( p_vram ) ] ] )
				break;
			}
		}

		n_x.d += n_dx;
		n_y.d += n_dy;
		n_len--;
	}
}

void psxgpu_device::GouraudLine()
{
#if PSXGPU_DEBUG_VIEWER
	if( m_debug.n_skip == 6 )
	{
		return;
	}
	DebugMesh( S11_COORD_X( m_packet.GouraudLine.vertex[ 0 ].n_coord ) + n_drawoffset_x, S11_COORD_Y( m_packet.GouraudLine.vertex[ 0 ].n_coord ) + n_drawoffset_y );
	DebugMesh( S11_COORD_X( m_packet.GouraudLine.vertex[ 1 ].n_coord ) + n_drawoffset_x, S11_COORD_Y( m_packet.GouraudLine.vertex[ 1 ].n_coord ) + n_drawoffset_y );
	DebugMeshEnd();
#endif

	uint8_t n_cmd = BGR_C( m_packet.GouraudLine.vertex[ 0 ].n_bgr );

	TRANSPARENCYSETUP

	int32_t n_xstart = S11_COORD_X( m_packet.GouraudLine.vertex[ 0 ].n_coord );
	int32_t n_ystart = S11_COORD_Y( m_packet.GouraudLine.vertex[ 0 ].n_coord );
	PAIR n_cr1; n_cr1.w.h = BGR_R( m_packet.GouraudLine.vertex[ 0 ].n_bgr ); n_cr1.w.l = 0;
	PAIR n_cg1; n_cg1.w.h = BGR_G( m_packet.GouraudLine.vertex[ 0 ].n_bgr ); n_cg1.w.l = 0;
	PAIR n_cb1; n_cb1.w.h = BGR_B( m_packet.GouraudLine.vertex[ 0 ].n_bgr ); n_cb1.w.l = 0;

	int32_t n_xend = S11_COORD_X( m_packet.GouraudLine.vertex[ 1 ].n_coord );
	int32_t n_yend = S11_COORD_Y( m_packet.GouraudLine.vertex[ 1 ].n_coord );
	PAIR n_cr2; n_cr2.w.h = BGR_R( m_packet.GouraudLine.vertex[ 1 ].n_bgr ); n_cr2.w.l = 0;
	PAIR n_cg2; n_cg2.w.h = BGR_G( m_packet.GouraudLine.vertex[ 1 ].n_bgr ); n_cg2.w.l = 0;
	PAIR n_cb2; n_cb2.w.h = BGR_B( m_packet.GouraudLine.vertex[ 1 ].n_bgr ); n_cb2.w.l = 0;


	PAIR n_x; n_x.sw.h = n_xstart; n_x.sw.l = 0;
	PAIR n_y; n_y.sw.h = n_ystart; n_y.sw.l = 0;
	PAIR n_r; n_r.d = n_cr1.d;
	PAIR n_g; n_g.d = n_cg1.d;
	PAIR n_b; n_b.d = n_cb1.d;

	int32_t n_xlen;
	if( n_xend > n_xstart )
	{
		n_xlen = n_xend - n_xstart;
	}
	else
	{
		n_xlen = n_xstart - n_xend;
	}

	int32_t n_ylen;
	if( n_yend > n_ystart )
	{
		n_ylen = n_yend - n_ystart;
	}
	else
	{
		n_ylen = n_ystart - n_yend;
	}

	int32_t n_distance;
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

	int32_t n_dx = (int32_t)( ( n_xend << 16 ) - n_x.sd ) / n_distance;
	int32_t n_dy = (int32_t)( ( n_yend << 16 ) - n_y.sd ) / n_distance;
	int32_t n_dr = (int32_t)( n_cr2.d - n_cr1.d ) / n_distance;
	int32_t n_dg = (int32_t)( n_cg2.d - n_cg1.d ) / n_distance;
	int32_t n_db = (int32_t)( n_cb2.d - n_cb1.d ) / n_distance;

	while( n_distance > 0 )
	{
		int drawx = n_x.sw.h + n_drawoffset_x;
		int drawy = n_y.sw.h + n_drawoffset_y;

		if( drawx >= (int32_t)n_drawarea_x1 && drawy >= (int32_t)n_drawarea_y1 &&
			drawx <= (int32_t)n_drawarea_x2 && drawy <= (int32_t)n_drawarea_y2 )
		{
			uint16_t *p_vram = p_p_vram[ drawy ] + drawx;

			switch( n_cmd & 0x02 )
			{
			case 0x00:
				/* transparency off */
				WRITE_PIXEL(
					p_n_redshade[ MID_LEVEL | n_r.w.h ] |
					p_n_greenshade[ MID_LEVEL | n_g.w.h ] |
					p_n_blueshade[ MID_LEVEL | n_b.w.h ] )
				break;
			case 0x02:
				/* transparency on */
				WRITE_PIXEL(
					p_n_redtrans[ p_n_f[ MID_LEVEL | n_r.w.h ] | p_n_redb[ *( p_vram ) ] ] |
					p_n_greentrans[ p_n_f[ MID_LEVEL | n_g.w.h ] | p_n_greenb[ *( p_vram ) ] ] |
					p_n_bluetrans[ p_n_f[ MID_LEVEL | n_b.w.h ] | p_n_blueb[ *( p_vram ) ] ] )
				break;
			}
		}

		n_x.sd += n_dx;
		n_y.sd += n_dy;
		n_r.d += n_dr;
		n_g.d += n_dg;
		n_b.d += n_db;
		n_distance--;
	}
}

void psxgpu_device::FrameBufferRectangleDraw()
{
#if PSXGPU_DEBUG_VIEWER
	if( m_debug.n_skip == 7 )
	{
		return;
	}
	DebugMesh( S11_COORD_X( m_packet.FlatRectangle.n_coord ), S11_COORD_Y( m_packet.FlatRectangle.n_coord ) );
	DebugMesh( S11_COORD_X( m_packet.FlatRectangle.n_coord ) + SIZE_W( m_packet.FlatRectangle.n_size ), S11_COORD_Y( m_packet.FlatRectangle.n_coord ) );
	DebugMesh( S11_COORD_X( m_packet.FlatRectangle.n_coord ), S11_COORD_Y( m_packet.FlatRectangle.n_coord ) + SIZE_H( m_packet.FlatRectangle.n_size ) );
	DebugMesh( S11_COORD_X( m_packet.FlatRectangle.n_coord ) + SIZE_W( m_packet.FlatRectangle.n_size ), S11_COORD_Y( m_packet.FlatRectangle.n_coord ) + SIZE_H( m_packet.FlatRectangle.n_size ) );
	DebugMeshEnd();
#endif

	PAIR n_r; n_r.w.h = BGR_R( m_packet.FlatRectangle.n_bgr ); n_r.w.l = 0;
	PAIR n_g; n_g.w.h = BGR_G( m_packet.FlatRectangle.n_bgr ); n_g.w.l = 0;
	PAIR n_b; n_b.w.h = BGR_B( m_packet.FlatRectangle.n_bgr ); n_b.w.l = 0;

	int16_t n_y = COORD_Y( m_packet.FlatRectangle.n_coord );
	int32_t n_h = SIZE_H( m_packet.FlatRectangle.n_size );

	while( n_h > 0 )
	{
		int16_t n_x = COORD_X( m_packet.FlatRectangle.n_coord );
		int32_t n_distance = SIZE_W( m_packet.FlatRectangle.n_size );

		while( n_distance > 0 )
		{
			p_p_vram[ n_y & 1023 ][ n_x & 1023 ] =
				p_n_redshade[ MID_LEVEL | n_r.w.h ] |
				p_n_greenshade[ MID_LEVEL | n_g.w.h ] |
				p_n_blueshade[ MID_LEVEL | n_b.w.h ];
			n_x++;
			n_distance--;
		}

		n_y++;
		n_h--;
	}
}

void psxgpu_device::FlatRectangle()
{
#if PSXGPU_DEBUG_VIEWER
	if( m_debug.n_skip == 8 )
	{
		return;
	}
	DebugMesh( S11_COORD_X( m_packet.FlatRectangle.n_coord ) + n_drawoffset_x, S11_COORD_Y( m_packet.FlatRectangle.n_coord ) + n_drawoffset_y );
	DebugMesh( S11_COORD_X( m_packet.FlatRectangle.n_coord ) + n_drawoffset_x + SIZE_W( m_packet.FlatRectangle.n_size ), S11_COORD_Y( m_packet.FlatRectangle.n_coord ) + n_drawoffset_y );
	DebugMesh( S11_COORD_X( m_packet.FlatRectangle.n_coord ) + n_drawoffset_x, S11_COORD_Y( m_packet.FlatRectangle.n_coord ) + n_drawoffset_y + SIZE_H( m_packet.FlatRectangle.n_size ) );
	DebugMesh( S11_COORD_X( m_packet.FlatRectangle.n_coord ) + n_drawoffset_x + SIZE_W( m_packet.FlatRectangle.n_size ), S11_COORD_Y( m_packet.FlatRectangle.n_coord ) + n_drawoffset_y + SIZE_H( m_packet.FlatRectangle.n_size ) );
	DebugMeshEnd();
#endif

	uint8_t n_cmd = BGR_C( m_packet.FlatRectangle.n_bgr );

	SOLIDSETUP

	PAIR n_r; n_r.w.h = BGR_R( m_packet.FlatRectangle.n_bgr ); n_r.w.l = 0;
	PAIR n_g; n_g.w.h = BGR_G( m_packet.FlatRectangle.n_bgr ); n_g.w.l = 0;
	PAIR n_b; n_b.w.h = BGR_B( m_packet.FlatRectangle.n_bgr ); n_b.w.l = 0;

	int16_t n_x = S11_COORD_X( m_packet.FlatRectangle.n_coord );
	int16_t n_y = S11_COORD_Y( m_packet.FlatRectangle.n_coord );
	int32_t n_h = SIZE_H( m_packet.FlatRectangle.n_size );

	while( n_h > 0 )
	{
		int32_t n_distance = SIZE_W( m_packet.FlatRectangle.n_size );
		int drawy = n_y + n_drawoffset_y;

		if( n_distance > 0 && drawy >= (int32_t)n_drawarea_y1 && drawy <= (int32_t)n_drawarea_y2 )
		{
			int drawx = n_x + n_drawoffset_x;

			if( ( (int32_t)n_drawarea_x1 - drawx ) > 0 )
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

void psxgpu_device::FlatRectangle8x8()
{
#if PSXGPU_DEBUG_VIEWER
	if( m_debug.n_skip == 9 )
	{
		return;
	}
	DebugMesh( S11_COORD_X( m_packet.FlatRectangle8x8.n_coord ) + n_drawoffset_x, S11_COORD_Y( m_packet.FlatRectangle8x8.n_coord ) + n_drawoffset_y );
	DebugMesh( S11_COORD_X( m_packet.FlatRectangle8x8.n_coord ) + n_drawoffset_x + 8, S11_COORD_Y( m_packet.FlatRectangle8x8.n_coord ) + n_drawoffset_y );
	DebugMesh( S11_COORD_X( m_packet.FlatRectangle8x8.n_coord ) + n_drawoffset_x, S11_COORD_Y( m_packet.FlatRectangle8x8.n_coord ) + n_drawoffset_y + 8 );
	DebugMesh( S11_COORD_X( m_packet.FlatRectangle8x8.n_coord ) + n_drawoffset_x + 8, S11_COORD_Y( m_packet.FlatRectangle8x8.n_coord ) + n_drawoffset_y + 8 );
	DebugMeshEnd();
#endif

	uint8_t n_cmd = BGR_C( m_packet.FlatRectangle8x8.n_bgr );

	SOLIDSETUP

	PAIR n_r; n_r.w.h = BGR_R( m_packet.FlatRectangle8x8.n_bgr ); n_r.w.l = 0;
	PAIR n_g; n_g.w.h = BGR_G( m_packet.FlatRectangle8x8.n_bgr ); n_g.w.l = 0;
	PAIR n_b; n_b.w.h = BGR_B( m_packet.FlatRectangle8x8.n_bgr ); n_b.w.l = 0;

	int16_t n_x = S11_COORD_X( m_packet.FlatRectangle8x8.n_coord );
	int16_t n_y = S11_COORD_Y( m_packet.FlatRectangle8x8.n_coord );
	int32_t n_h = 8;

	while( n_h > 0 )
	{
		int32_t n_distance = 8;
		int drawy = n_y + n_drawoffset_y;

		if( n_distance > 0 && drawy >= (int32_t)n_drawarea_y1 && drawy <= (int32_t)n_drawarea_y2 )
		{
			int drawx = n_x + n_drawoffset_x;

			if( ( (int32_t)n_drawarea_x1 - drawx ) > 0 )
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

void psxgpu_device::FlatRectangle16x16()
{
#if PSXGPU_DEBUG_VIEWER
	if( m_debug.n_skip == 10 )
	{
		return;
	}
	DebugMesh( S11_COORD_X( m_packet.FlatRectangle16x16.n_coord ) + n_drawoffset_x, S11_COORD_Y( m_packet.FlatRectangle16x16.n_coord ) + n_drawoffset_y );
	DebugMesh( S11_COORD_X( m_packet.FlatRectangle16x16.n_coord ) + n_drawoffset_x + 16, S11_COORD_Y( m_packet.FlatRectangle16x16.n_coord ) + n_drawoffset_y );
	DebugMesh( S11_COORD_X( m_packet.FlatRectangle16x16.n_coord ) + n_drawoffset_x, S11_COORD_Y( m_packet.FlatRectangle16x16.n_coord ) + n_drawoffset_y + 16 );
	DebugMesh( S11_COORD_X( m_packet.FlatRectangle16x16.n_coord ) + n_drawoffset_x + 16, S11_COORD_Y( m_packet.FlatRectangle16x16.n_coord ) + n_drawoffset_y + 16 );
	DebugMeshEnd();
#endif

	uint8_t n_cmd = BGR_C( m_packet.FlatRectangle16x16.n_bgr );

	SOLIDSETUP

	PAIR n_r; n_r.w.h = BGR_R( m_packet.FlatRectangle16x16.n_bgr ); n_r.w.l = 0;
	PAIR n_g; n_g.w.h = BGR_G( m_packet.FlatRectangle16x16.n_bgr ); n_g.w.l = 0;
	PAIR n_b; n_b.w.h = BGR_B( m_packet.FlatRectangle16x16.n_bgr ); n_b.w.l = 0;

	int16_t n_x = S11_COORD_X( m_packet.FlatRectangle16x16.n_coord );
	int16_t n_y = S11_COORD_Y( m_packet.FlatRectangle16x16.n_coord );
	int32_t n_h = 16;

	while( n_h > 0 )
	{
		int32_t n_distance = 16;
		int drawy = n_y + n_drawoffset_y;

		if( n_distance > 0 && n_y >= (int32_t)n_drawarea_y1 && n_y <= (int32_t)n_drawarea_y2 )
		{
			int drawx = n_x + n_drawoffset_x;

			if( ( (int32_t)n_drawarea_x1 - drawx ) > 0 )
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

void psxgpu_device::FlatTexturedRectangle()
{
#if PSXGPU_DEBUG_VIEWER
	if( m_debug.n_skip == 11 )
	{
		return;
	}
	DebugMesh( S11_COORD_X( m_packet.FlatTexturedRectangle.n_coord ) + n_drawoffset_x, S11_COORD_Y( m_packet.FlatTexturedRectangle.n_coord ) + n_drawoffset_y );
	DebugMesh( S11_COORD_X( m_packet.FlatTexturedRectangle.n_coord ) + n_drawoffset_x + SIZE_W( m_packet.FlatTexturedRectangle.n_size ), S11_COORD_Y( m_packet.FlatTexturedRectangle.n_coord ) + n_drawoffset_y );
	DebugMesh( S11_COORD_X( m_packet.FlatTexturedRectangle.n_coord ) + n_drawoffset_x, S11_COORD_Y( m_packet.FlatTexturedRectangle.n_coord ) + n_drawoffset_y + SIZE_H( m_packet.FlatTexturedRectangle.n_size ) );
	DebugMesh( S11_COORD_X( m_packet.FlatTexturedRectangle.n_coord ) + n_drawoffset_x + SIZE_W( m_packet.FlatTexturedRectangle.n_size ), S11_COORD_Y( m_packet.FlatTexturedRectangle.n_coord ) + n_drawoffset_y + SIZE_H( m_packet.FlatTexturedRectangle.n_size ) );
	DebugMeshEnd();
#endif

	uint8_t n_cmd = BGR_C( m_packet.FlatTexturedRectangle.n_bgr );

	uint32_t n_clutx = ( m_packet.FlatTexturedRectangle.n_texture.w.h & 0x3f ) << 4;
	uint32_t n_cluty = ( m_packet.FlatTexturedRectangle.n_texture.w.h >> 6 ) & 0x3ff;

	TEXTURESETUP
	SPRITESETUP

	PAIR n_r; n_r.w.h = n_cmd & 0x01 ? 0x80 : BGR_R( m_packet.FlatTexturedRectangle.n_bgr ); n_r.w.l = 0;
	PAIR n_g; n_g.w.h = n_cmd & 0x01 ? 0x80 : BGR_G( m_packet.FlatTexturedRectangle.n_bgr ); n_g.w.l = 0;
	PAIR n_b; n_b.w.h = n_cmd & 0x01 ? 0x80 : BGR_B( m_packet.FlatTexturedRectangle.n_bgr ); n_b.w.l = 0;

	int16_t n_x = S11_COORD_X( m_packet.FlatTexturedRectangle.n_coord );
	int16_t n_y = S11_COORD_Y( m_packet.FlatTexturedRectangle.n_coord );
	uint8_t n_v = TEXTURE_V( m_packet.FlatTexturedRectangle.n_texture );
	uint32_t n_h = SIZE_H( m_packet.FlatTexturedRectangle.n_size );

	while( n_h > 0 )
	{
		uint8_t n_u = TEXTURE_U( m_packet.FlatTexturedRectangle.n_texture );
		int16_t n_distance = SIZE_W( m_packet.FlatTexturedRectangle.n_size );
		int drawy = n_y + n_drawoffset_y;

		if( n_distance > 0 && drawy >= (int32_t)n_drawarea_y1 && drawy <= (int32_t)n_drawarea_y2 )
		{
			int drawx = n_x + n_drawoffset_x;

			if( ( (int32_t)n_drawarea_x1 - drawx ) > 0 )
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

void psxgpu_device::Sprite8x8()
{
#if PSXGPU_DEBUG_VIEWER
	if( m_debug.n_skip == 12 )
	{
		return;
	}
	DebugMesh( S11_COORD_X( m_packet.Sprite8x8.n_coord ) + n_drawoffset_x, S11_COORD_Y( m_packet.Sprite8x8.n_coord ) + n_drawoffset_y );
	DebugMesh( S11_COORD_X( m_packet.Sprite8x8.n_coord ) + n_drawoffset_x + 7, S11_COORD_Y( m_packet.Sprite8x8.n_coord ) + n_drawoffset_y );
	DebugMesh( S11_COORD_X( m_packet.Sprite8x8.n_coord ) + n_drawoffset_x, S11_COORD_Y( m_packet.Sprite8x8.n_coord ) + n_drawoffset_y + 7 );
	DebugMesh( S11_COORD_X( m_packet.Sprite8x8.n_coord ) + n_drawoffset_x + 7, S11_COORD_Y( m_packet.Sprite8x8.n_coord ) + n_drawoffset_y + 7 );
	DebugMeshEnd();
#endif

	uint8_t n_cmd = BGR_C( m_packet.Sprite8x8.n_bgr );

	uint32_t n_clutx = ( m_packet.Sprite8x8.n_texture.w.h & 0x3f ) << 4;
	uint32_t n_cluty = ( m_packet.Sprite8x8.n_texture.w.h >> 6 ) & 0x3ff;

	TEXTURESETUP
	SPRITESETUP

	PAIR n_r; n_r.w.h = n_cmd & 0x01 ? 0x80 : BGR_R( m_packet.Sprite8x8.n_bgr ); n_r.w.l = 0;
	PAIR n_g; n_g.w.h = n_cmd & 0x01 ? 0x80 : BGR_G( m_packet.Sprite8x8.n_bgr ); n_g.w.l = 0;
	PAIR n_b; n_b.w.h = n_cmd & 0x01 ? 0x80 : BGR_B( m_packet.Sprite8x8.n_bgr ); n_b.w.l = 0;

	int16_t n_x = S11_COORD_X( m_packet.Sprite8x8.n_coord );
	int16_t n_y = S11_COORD_Y( m_packet.Sprite8x8.n_coord );
	uint8_t n_v = TEXTURE_V( m_packet.Sprite8x8.n_texture );
	uint32_t n_h = 8;

	while( n_h > 0 )
	{
		uint8_t n_u = TEXTURE_U( m_packet.Sprite8x8.n_texture );
		int16_t n_distance = 8;

		int drawy = n_y + n_drawoffset_y;

		if( n_distance > 0 && drawy >= (int32_t)n_drawarea_y1 && drawy <= (int32_t)n_drawarea_y2 )
		{
			int drawx = n_x + n_drawoffset_x;

			if( ( (int32_t)n_drawarea_x1 - drawx ) > 0 )
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

void psxgpu_device::Sprite16x16()
{
#if PSXGPU_DEBUG_VIEWER
	if( m_debug.n_skip == 13 )
	{
		return;
	}
	DebugMesh( S11_COORD_X( m_packet.Sprite16x16.n_coord ) + n_drawoffset_x, S11_COORD_Y( m_packet.Sprite16x16.n_coord ) + n_drawoffset_y );
	DebugMesh( S11_COORD_X( m_packet.Sprite16x16.n_coord ) + n_drawoffset_x + 7, S11_COORD_Y( m_packet.Sprite16x16.n_coord ) + n_drawoffset_y );
	DebugMesh( S11_COORD_X( m_packet.Sprite16x16.n_coord ) + n_drawoffset_x, S11_COORD_Y( m_packet.Sprite16x16.n_coord ) + n_drawoffset_y + 7 );
	DebugMesh( S11_COORD_X( m_packet.Sprite16x16.n_coord ) + n_drawoffset_x + 7, S11_COORD_Y( m_packet.Sprite16x16.n_coord ) + n_drawoffset_y + 7 );
	DebugMeshEnd();
#endif

	uint8_t n_cmd = BGR_C( m_packet.Sprite16x16.n_bgr );

	uint32_t n_clutx = ( m_packet.Sprite16x16.n_texture.w.h & 0x3f ) << 4;
	uint32_t n_cluty = ( m_packet.Sprite16x16.n_texture.w.h >> 6 ) & 0x3ff;

	TEXTURESETUP
	SPRITESETUP

	PAIR n_r; n_r.w.h = n_cmd & 0x01 ? 0x80 : BGR_R( m_packet.Sprite16x16.n_bgr ); n_r.w.l = 0;
	PAIR n_g; n_g.w.h = n_cmd & 0x01 ? 0x80 : BGR_G( m_packet.Sprite16x16.n_bgr ); n_g.w.l = 0;
	PAIR n_b; n_b.w.h = n_cmd & 0x01 ? 0x80 : BGR_B( m_packet.Sprite16x16.n_bgr ); n_b.w.l = 0;

	int16_t n_x = S11_COORD_X( m_packet.Sprite16x16.n_coord );
	int16_t n_y = S11_COORD_Y( m_packet.Sprite16x16.n_coord );
	uint8_t n_v = TEXTURE_V( m_packet.Sprite16x16.n_texture );
	uint32_t n_h = 16;

	while( n_h > 0 )
	{
		uint8_t n_u = TEXTURE_U( m_packet.Sprite16x16.n_texture );
		int16_t n_distance = 16;

		int drawy = n_y + n_drawoffset_y;

		if( n_distance > 0 && drawy >= (int32_t)n_drawarea_y1 && drawy <= (int32_t)n_drawarea_y2 )
		{
			int drawx = n_x + n_drawoffset_x;

			if( ( (int32_t)n_drawarea_x1 - drawx ) > 0 )
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

void psxgpu_device::Dot()
{
#if PSXGPU_DEBUG_VIEWER
	if( m_debug.n_skip == 14 )
	{
		return;
	}
	DebugMesh( S11_COORD_X( m_packet.Dot.vertex.n_coord ) + n_drawoffset_x, S11_COORD_Y( m_packet.Dot.vertex.n_coord ) + n_drawoffset_y );
	DebugMeshEnd();
#endif

	uint8_t n_cmd = BGR_C( m_packet.Dot.n_bgr );
	uint8_t n_r = BGR_R( m_packet.Dot.n_bgr );
	uint8_t n_g = BGR_G( m_packet.Dot.n_bgr );
	uint8_t n_b = BGR_B( m_packet.Dot.n_bgr );
	int32_t n_x = S11_COORD_X( m_packet.Dot.vertex.n_coord );
	int32_t n_y = S11_COORD_Y( m_packet.Dot.vertex.n_coord );

	TRANSPARENCYSETUP

	int drawx = n_x + n_drawoffset_x;
	int drawy = n_y + n_drawoffset_y;

	if( drawx >= (int32_t)n_drawarea_x1 && drawy >= (int32_t)n_drawarea_y1 &&
		drawx <= (int32_t)n_drawarea_x2 && drawy <= (int32_t)n_drawarea_y2 )
	{
		uint16_t *p_vram = p_p_vram[ drawy ] + drawx;

		switch( n_cmd & 0x02 )
		{
		case 0x00:
			/* transparency off */
			WRITE_PIXEL(
				p_n_redshade[ MID_LEVEL | n_r ] |
				p_n_greenshade[ MID_LEVEL | n_g ] |
				p_n_blueshade[ MID_LEVEL | n_b ] )
			break;
		case 0x02:
			/* transparency on */
			WRITE_PIXEL(
				p_n_redtrans[ p_n_f[ MID_LEVEL | n_r ] | p_n_redb[ *( p_vram ) ] ] |
				p_n_greentrans[ p_n_f[ MID_LEVEL | n_g ] | p_n_greenb[ *( p_vram ) ] ] |
				p_n_bluetrans[ p_n_f[ MID_LEVEL | n_b ] | p_n_blueb[ *( p_vram ) ] ] )
			break;
		}
	}
}

void psxgpu_device::TexturedDot()
{
#if PSXGPU_DEBUG_VIEWER
	if (m_debug.n_skip == 15)
	{
		return;
	}
	DebugMesh( S11_COORD_X( m_packet.TexturedDot.vertex.n_coord ) + n_drawoffset_x, S11_COORD_Y( m_packet.TexturedDot.vertex.n_coord ) + n_drawoffset_y );
	DebugMeshEnd();
#endif

	uint8_t n_cmd = BGR_C( m_packet.TexturedDot.n_bgr );

	PAIR n_r; n_r.w.h = n_cmd & 0x01 ? 0x80 : BGR_R( m_packet.TexturedDot.n_bgr ); n_r.w.l = 0;
	PAIR n_g; n_g.w.h = n_cmd & 0x01 ? 0x80 : BGR_G( m_packet.TexturedDot.n_bgr ); n_g.w.l = 0;
	PAIR n_b; n_b.w.h = n_cmd & 0x01 ? 0x80 : BGR_B( m_packet.TexturedDot.n_bgr ); n_b.w.l = 0;

	int32_t n_x = S11_COORD_X( m_packet.TexturedDot.vertex.n_coord );
	int32_t n_y = S11_COORD_Y( m_packet.TexturedDot.vertex.n_coord );
	uint8_t n_u = TEXTURE_U(m_packet.TexturedDot.vertex.n_texture );
	uint8_t n_v = TEXTURE_V(m_packet.TexturedDot.vertex.n_texture );
	uint32_t n_clutx = ( m_packet.TexturedDot.vertex.n_texture.w.h & 0x3f ) << 4;
	uint32_t n_cluty = ( m_packet.TexturedDot.vertex.n_texture.w.h >> 6 ) & 0x3ff;

	TEXTURESETUP

	int32_t n_distance = 1;

	int drawx = n_x + n_drawoffset_x;
	int drawy = n_y + n_drawoffset_y;

	if( drawx >= (int32_t)n_drawarea_x1 && drawy >= (int32_t)n_drawarea_y1 &&
		drawx <= (int32_t)n_drawarea_x2 && drawy <= (int32_t)n_drawarea_y2 )
	{
		TEXTUREFILL( {}, n_u, n_v );
	}
}

void psxgpu_device::MoveImage()
{
#if PSXGPU_DEBUG_VIEWER
	if( m_debug.n_skip == 16 )
	{
		return;
	}
	DebugMesh( S11_COORD_X( m_packet.MoveImage.vertex[ 1 ].n_coord ), S11_COORD_Y( m_packet.MoveImage.vertex[ 1 ].n_coord ) );
	DebugMesh( S11_COORD_X( m_packet.MoveImage.vertex[ 1 ].n_coord ) + SIZE_W( m_packet.MoveImage.n_size ), S11_COORD_Y( m_packet.MoveImage.vertex[ 1 ].n_coord ) );
	DebugMesh( S11_COORD_X( m_packet.MoveImage.vertex[ 1 ].n_coord ), S11_COORD_Y( m_packet.MoveImage.vertex[ 1 ].n_coord ) ) + SIZE_H( m_packet.MoveImage.n_size ) );
	DebugMesh( S11_COORD_X( m_packet.MoveImage.vertex[ 1 ].n_coord ) + SIZE_W( m_packet.MoveImage.n_size ), S11_COORD_Y( m_packet.MoveImage.vertex[ 1 ].n_coord ) + SIZE_H( m_packet.MoveImage.n_size ) );
	DebugMeshEnd();
#endif

	int16_t n_srcy = COORD_Y( m_packet.MoveImage.vertex[ 0 ].n_coord );
	int16_t n_dsty = COORD_Y( m_packet.MoveImage.vertex[ 1 ].n_coord );
	int16_t n_h = SIZE_H( m_packet.MoveImage.n_size );

	while( n_h > 0 )
	{
		int16_t n_srcx = COORD_X( m_packet.MoveImage.vertex[ 0 ].n_coord );
		int16_t n_dstx = COORD_X( m_packet.MoveImage.vertex[ 1 ].n_coord );
		int16_t n_w = SIZE_W( m_packet.MoveImage.n_size );

		while( n_w > 0 )
		{
			uint16_t *p_vram = p_p_vram[ n_dsty & 1023 ] + ( n_dstx & 1023 );
			WRITE_PIXEL( *( p_p_vram[ n_srcy & 1023 ] + ( n_srcx & 1023 ) ) )
			n_srcx++;
			n_dstx++;
			n_w--;
		}

		n_srcy++;
		n_dsty++;
		n_h--;
	}
}

void psxgpu_device::dma_write( uint32_t *p_n_psxram, uint32_t n_address, int32_t n_size )
{
	gpu_write( &p_n_psxram[ n_address / 4 ], n_size );
}

void psxgpu_device::gpu_write( uint32_t *p_ram, int32_t n_size )
{
	while( n_size > 0 )
	{
		uint32_t data = *( p_ram );

		LOG("PSX Packet #%u %08x\n", n_gpu_buffer_offset, data);
		m_packet.n_entry[ n_gpu_buffer_offset ] = data;
		switch( m_packet.n_entry[ 0 ] >> 24 )
		{
		case 0x00:
			LOGMASKED(LOG_WRITE, "%s: not handled: GPU Command 0x00: (%08x)\n", machine().describe_context(), data);
			break;
		case 0x01:
			LOGMASKED(LOG_WRITE, "%s: not handled: clear cache\n", machine().describe_context());
			break;
		case 0x02:
			if( n_gpu_buffer_offset < 2 )
			{
				n_gpu_buffer_offset++;
			}
			else
			{
				LOGMASKED(LOG_WRITE, "%s: %02x: frame buffer rectangle %u,%u %u,%u\n", machine().describe_context(), m_packet.n_entry[ 0 ] >> 24,
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
				LOGMASKED(LOG_WRITE, machine().describe_context(), "%s: %02x: monochrome 3 point polygon\n", machine().describe_context(), m_packet.n_entry[ 0 ] >> 24);
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
				LOGMASKED(LOG_WRITE, "%s: %02x: textured 3 point polygon\n", machine().describe_context(), m_packet.n_entry[ 0 ] >> 24);
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
				LOGMASKED(LOG_WRITE, "%s: %02x: monochrome 4 point polygon\n", machine().describe_context(), m_packet.n_entry[ 0 ] >> 24);
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
				LOGMASKED(LOG_WRITE, "%s: %02x: textured 4 point polygon\n", machine().describe_context(), m_packet.n_entry[ 0 ] >> 24);
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
				LOGMASKED(LOG_WRITE, "%s: %02x: gouraud 3 point polygon\n", machine().describe_context(), m_packet.n_entry[ 0 ] >> 24 );
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
				LOGMASKED(LOG_WRITE, "%s: %02x: gouraud textured 3 point polygon\n", machine().describe_context(), m_packet.n_entry[ 0 ] >> 24);
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
				LOGMASKED(LOG_WRITE, "%s: %02x: gouraud 4 point polygon\n", machine().describe_context(), m_packet.n_entry[ 0 ] >> 24);
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
				LOGMASKED(LOG_WRITE, "%s: %02x: gouraud textured 4 point polygon\n", machine().describe_context(), m_packet.n_entry[ 0 ] >> 24);
				GouraudTexturedPolygon( 4 );
				n_gpu_buffer_offset = 0;
			}
			break;
		case 0x40:
		case 0x41:
		case 0x42:
		case 0x43:
			if( n_gpu_buffer_offset < 2 )
			{
				n_gpu_buffer_offset++;
			}
			else
			{
				LOGMASKED(LOG_WRITE, "%s: %02x: monochrome line\n", machine().describe_context(), m_packet.n_entry[ 0 ] >> 24);
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
				LOGMASKED(LOG_WRITE, "%s: %02x: monochrome polyline\n", machine().describe_context(), m_packet.n_entry[ 0 ] >> 24);
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
				LOGMASKED(LOG_WRITE, "%s: %02x: gouraud line\n", machine().describe_context(), m_packet.n_entry[ 0 ] >> 24);
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
				LOGMASKED(LOG_WRITE, "%s: %02x: gouraud polyline\n", machine().describe_context(), m_packet.n_entry[ 0 ] >> 24);
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
				LOGMASKED(LOG_WRITE, "%s: 02x: rectangle %d,%d %d,%d\n", machine().describe_context(),
					m_packet.n_entry[ 0 ] >> 24,
					(int16_t)( m_packet.n_entry[ 1 ] & 0xffff ), (int16_t)( m_packet.n_entry[ 1 ] >> 16 ),
					(int16_t)( m_packet.n_entry[ 2 ] & 0xffff ), (int16_t)( m_packet.n_entry[ 2 ] >> 16 ) );
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
				LOGMASKED(LOG_WRITE, "%s: %02x: sprite %d,%d %u,%u %08x, %08x\n", machine().describe_context(),
					m_packet.n_entry[ 0 ] >> 24,
					(int16_t)( m_packet.n_entry[ 1 ] & 0xffff ), (int16_t)( m_packet.n_entry[ 1 ] >> 16 ),
					m_packet.n_entry[ 3 ] & 0xffff, m_packet.n_entry[ 3 ] >> 16,
					m_packet.n_entry[ 0 ], m_packet.n_entry[ 2 ] );
				FlatTexturedRectangle();
				n_gpu_buffer_offset = 0;
			}
			break;
		case 0x68:
		case 0x69:
		case 0x6a:
		case 0x6b:
			if( n_gpu_buffer_offset < 1 )
			{
				n_gpu_buffer_offset++;
			}
			else
			{
				LOGMASKED(LOG_WRITE, "%s: %02x: dot %d,%d %08x\n", machine().describe_context(),
					m_packet.n_entry[ 0 ] >> 24,
					(int16_t)( m_packet.n_entry[ 1 ] & 0xffff ), (int16_t)( m_packet.n_entry[ 1 ] >> 16 ),
					m_packet.n_entry[ 0 ] & 0xffffff );
				Dot();
				n_gpu_buffer_offset = 0;
			}
			break;
		case 0x6c:
		case 0x6d:
		case 0x6e:
		case 0x6f:
			if (n_gpu_buffer_offset < 2)
			{
				n_gpu_buffer_offset++;
			}
			else
			{
				LOGMASKED(LOG_WRITE, "%s: %02x: textured dot %d,%d %08x\n", machine().describe_context(),
					m_packet.n_entry[ 0 ] >> 24,
					(int16_t)( m_packet.n_entry[ 1 ] & 0xffff ), (int16_t)( m_packet.n_entry[ 1 ] >> 16 ),
					m_packet.n_entry[ 0 ] & 0xffffff );
				TexturedDot();
				n_gpu_buffer_offset = 0;
			}
			break;
		case 0x70:
		case 0x71:
		case 0x72:
		case 0x73:
			/* 8*8 rectangle */
			if( n_gpu_buffer_offset < 1 )
			{
				n_gpu_buffer_offset++;
			}
			else
			{
				LOGMASKED(LOG_WRITE, "%s; %02x: 16x16 rectangle %08x %08x\n", machine().describe_context(), m_packet.n_entry[ 0 ] >> 24,
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
				LOGMASKED(LOG_WRITE, "%s: %02x: 8x8 sprite %08x %08x %08x\n", machine().describe_context(), m_packet.n_entry[ 0 ] >> 24,
					m_packet.n_entry[ 0 ], m_packet.n_entry[ 1 ], m_packet.n_entry[ 2 ] );
				Sprite8x8();
				n_gpu_buffer_offset = 0;
			}
			break;
		case 0x78:
		case 0x79:
		case 0x7a:
		case 0x7b:
			/* 16*16 rectangle */
			if( n_gpu_buffer_offset < 1 )
			{
				n_gpu_buffer_offset++;
			}
			else
			{
				LOGMASKED(LOG_WRITE, "%s: %02x: 16x16 rectangle %08x %08x\n", machine().describe_context(), m_packet.n_entry[ 0 ] >> 24,
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
				LOGMASKED(LOG_WRITE, "%s: %02x: 16x16 sprite %08x %08x %08x\n", machine().describe_context(), m_packet.n_entry[ 0 ] >> 24,
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
				LOGMASKED(LOG_WRITE, "%s: move image in frame buffer %08x %08x %08x %08x\n", machine().describe_context(),
					m_packet.n_entry[ 0 ], m_packet.n_entry[ 1 ], m_packet.n_entry[ 2 ], m_packet.n_entry[ 3 ]);
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
				for( int n_pixel = 0; n_pixel < 2; n_pixel++ )
				{
					LOGMASKED(LOG_WRITE, "%s: send image to framebuffer ( pixel %u,%u = %u )\n",
						machine().describe_context(),
						( n_vramx + m_packet.n_entry[ 1 ] ) & 1023,
						( n_vramy + ( m_packet.n_entry[ 1 ] >> 16 ) ) & 1023,
						data & 0xffff );

					uint16_t *p_vram = p_p_vram[ ( n_vramy + ( m_packet.n_entry[ 1 ] >> 16 ) ) & 1023 ] + ( ( n_vramx + m_packet.n_entry[ 1 ] ) & 1023 );
					WRITE_PIXEL( data & 0xffff )
					n_vramx++;
					if( n_vramx >= ( m_packet.n_entry[ 2 ] & 0xffff ) )
					{
						n_vramx = 0;
						n_vramy++;
						if( n_vramy >= ( m_packet.n_entry[ 2 ] >> 16 ) )
						{
							LOGMASKED(LOG_WRITE, "%s: %02x: send image to framebuffer %u,%u %u,%u\n", machine().describe_context(), m_packet.n_entry[ 0 ] >> 24,
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
				LOGMASKED(LOG_WRITE, "%s: %02x: copy image from frame buffer\n", machine().describe_context(), m_packet.n_entry[ 0 ] >> 24);
				n_gpustatus |= ( 1L << 0x1b );
			}
			break;
		case 0xe1:
			LOGMASKED(LOG_WRITE, "%s: %02x: draw mode %06x\n", machine().describe_context(), m_packet.n_entry[ 0 ] >> 24,
				m_packet.n_entry[ 0 ] & 0xffffff );
			decode_tpage( m_packet.n_entry[ 0 ] & 0xffffff );
			break;
		case 0xe2:
			n_twy = ( ( ( m_packet.n_entry[ 0 ] >> 15 ) & 0x1f ) << 3 );
			n_twx = ( ( ( m_packet.n_entry[ 0 ] >> 10 ) & 0x1f ) << 3 );
			n_twh = 255 - ( ( ( m_packet.n_entry[ 0 ] >> 5 ) & 0x1f ) << 3 );
			n_tww = 255 - ( ( m_packet.n_entry[ 0 ] & 0x1f ) << 3 );
			LOGMASKED(LOG_WRITE, "%s: %02x: texture window %u,%u %u,%u\n", machine().describe_context(), m_packet.n_entry[ 0 ] >> 24,
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
			LOGMASKED(LOG_WRITE, "%s: %02x: drawing area top left %d,%d\n", machine().describe_context(), m_packet.n_entry[ 0 ] >> 24,
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
			LOGMASKED(LOG_WRITE, "%s: %02x: drawing area bottom right %d,%d\n", machine().describe_context(), m_packet.n_entry[ 0 ] >> 24,
				n_drawarea_x2, n_drawarea_y2 );
			break;
		case 0xe5:
			n_drawoffset_x = util::sext( m_packet.n_entry[ 0 ] & 2047, 11 );
			if( m_n_gputype == 2 )
			{
				n_drawoffset_y = util::sext( ( m_packet.n_entry[ 0 ] >> 11 ) & 2047, 11 );
			}
			else
			{
				n_drawoffset_y = util::sext( ( m_packet.n_entry[ 0 ] >> 12 ) & 2047, 11 );
			}
			LOGMASKED(LOG_WRITE, "%s: %02x: drawing offset %d,%d\n", machine().describe_context(), m_packet.n_entry[ 0 ] >> 24,
				n_drawoffset_x, n_drawoffset_y );
			break;
		case 0xe6:
			m_draw_stp = BIT( data, 0 );
			m_check_stp = BIT( data, 1 );
			// TODO: confirm status bits on real type 1 gpu
			n_gpustatus &= ~( 3L << 0xb );
			n_gpustatus |= ( data & 0x03 ) << 0xb;
			LOGMASKED(LOG_WRITE, "%s: mask setting %d\n", machine().describe_context(), m_packet.n_entry[ 0 ] & 3);
			break;
		default:
#if defined( MAME_DEBUG )
			popmessage( "unknown GPU packet %08x", m_packet.n_entry[ 0 ] );
#endif
			logerror("%s: unknown GPU packet %08x (%08x)\n", machine().describe_context(), m_packet.n_entry[ 0 ], data);
#if ( STOP_ON_ERROR )
			n_gpu_buffer_offset = 1;
#endif
			break;
		}
		p_ram++;
		n_size--;
	}
}

void psxgpu_device::write(offs_t offset, uint32_t data, uint32_t mem_mask)
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
			LOGMASKED(LOG_WRITE, "%s: reset gpu\n", machine().describe_context());
			gpu_reset();
			break;
		case 0x01:
			LOGMASKED(LOG_WRITE, "%s: not handled: reset command buffer\n", machine().describe_context());
			n_gpu_buffer_offset = 0;
			break;
		case 0x02:
			LOGMASKED(LOG_WRITE, "%s: not handled: reset irq\n", machine().describe_context());
			break;
		case 0x03:
			n_gpustatus &= ~( 1L << 0x17 );
			n_gpustatus |= ( data & 0x01 ) << 0x17;
			break;
		case 0x04:
			LOGMASKED(LOG_WRITE, "%s: dma setup %d\n", machine().describe_context(), data & 3);
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
			LOGMASKED(LOG_WRITE, "%s: start of display area %d %d\n", machine().describe_context(), m_n_displaystartx, n_displaystarty);
			break;
		case 0x06:
			n_horiz_disstart = data & 4095;
			n_horiz_disend = ( data >> 12 ) & 4095;
			LOGMASKED(LOG_WRITE, "%s: horizontal display range %d %d\n", machine().describe_context(), n_horiz_disstart, n_horiz_disend);
			break;
		case 0x07:
			n_vert_disstart = data & 1023;
			n_vert_disend = ( data >> 10 ) & 2047;
			LOGMASKED(LOG_WRITE, "%s: vertical display range %d %d\n", machine().describe_context(), n_vert_disstart, n_vert_disend);
			break;
		case 0x08:
			LOGMASKED(LOG_WRITE, "%s: display mode %02x\n", machine().describe_context(), data & 0xff);
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
			LOGMASKED(LOG_WRITE, "%s: not handled: GPU Control 0x09: %08x\n", machine().describe_context(), data);
			break;
		case 0x0d:
			LOGMASKED(LOG_WRITE, "%s: reset lightgun coordinates %08x\n", machine().describe_context(), data);
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
				LOGMASKED(LOG_WRITE, "%s: GPU Info - Draw area top left %08x\n", machine().describe_context(), n_gpuinfo);
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
				LOGMASKED(LOG_WRITE, "%s: GPU Info - Draw area bottom right %08x\n", machine().describe_context(), n_gpuinfo);
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
				LOGMASKED(LOG_WRITE, "%s: GPU Info - Draw offset %08x\n", machine().describe_context(), n_gpuinfo);
				break;
			case 0x07:
				n_gpuinfo = m_n_gputype;
				LOGMASKED(LOG_WRITE, "%s: GPU Info - GPU Type %08x\n", machine().describe_context(), n_gpuinfo);
				break;
			case 0x08:
				n_gpuinfo = n_lightgun_x | ( n_lightgun_y << 16 );
				LOGMASKED(LOG_WRITE, "%s: GPU Info - lightgun coordinates %08x\n", machine().describe_context(), n_gpuinfo);
				break;
			default:
				logerror("%s: GPU Info - unknown request (%08x)\n", machine().describe_context(), data);
				n_gpuinfo = 0;
				break;
			}
			break;
		case 0x20:
			LOGMASKED(LOG_WRITE, "%s: not handled: GPU Control 0x20: %08x\n", machine().describe_context(), data);
			break;
		default:
#if defined( MAME_DEBUG )
			popmessage( "unknown GPU command %08x", data );
#endif
			logerror("%s: gpu_w( %08x ) unknown GPU command\n", machine().describe_context(), data);
			break;
		}
		break;
	default:
		logerror("%s: gpu_w( %08x, %08x, %08x ) unknown register\n", machine().describe_context(), offset, data, mem_mask);
		break;
	}
}


void psxgpu_device::dma_read( uint32_t *p_n_psxram, uint32_t n_address, int32_t n_size )
{
	gpu_read( &p_n_psxram[ n_address / 4 ], n_size );
}

void psxgpu_device::gpu_read( uint32_t *p_ram, int32_t n_size )
{
	while( n_size > 0 )
	{
		if( ( n_gpustatus & ( 1L << 0x1b ) ) != 0 )
		{
			PAIR data;

			LOGMASKED(LOG_READ, "%s: copy image from frame buffer ( %d, %d )\n", machine().describe_context(), n_vramx, n_vramy);
			data.d = 0;
			for( int n_pixel = 0; n_pixel < 2; n_pixel++ )
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
						LOGMASKED(LOG_READ, "%s: copy image from frame buffer end\n", machine().describe_context());
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
			LOGMASKED(LOG_READ, "%s: read GPU info (%08x)\n", machine().describe_context(), n_gpuinfo);
			*( p_ram ) = n_gpuinfo;
		}
		p_ram++;
		n_size--;
	}
}

uint32_t psxgpu_device::read(offs_t offset, uint32_t mem_mask)
{
	uint32_t data;

	switch( offset )
	{
	case 0x00:
		gpu_read( &data, 1 );
		break;
	case 0x01:
		data = n_gpustatus;
		LOGMASKED(LOG_READ, "%s: read GPU status (%08x)\n", machine().describe_context(), data);
		break;
	default:
		logerror("%s: gpu_r( %08x, %08x ) unknown register\n", machine().describe_context(), offset, mem_mask);
		data = 0;
		break;
	}
	return data;
}

void psxgpu_device::vblank(screen_device &screen, bool vblank_state)
{
	if( vblank_state )
	{
#if PSXGPU_DEBUG_VIEWER
		DebugCheckKeys();
#endif

		n_gpustatus ^= ( 1L << 31 );
		m_vblank_handler(1);
	}
}

void psxgpu_device::gpu_reset()
{
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
	m_draw_stp = false;
	m_check_stp = false;
	updatevisiblearea();
}

void psxgpu_device::lightgun_set( int n_x, int n_y )
{
	n_lightgun_x = n_x;
	n_lightgun_y = n_y;
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void psxgpu_device::device_config_complete()
{
	if (!has_screen())
		return;

	if (!screen().refresh_attoseconds())
	{
		screen().set_refresh_hz(60);
		screen().set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
		screen().set_size(1024, 1024);
		screen().set_visarea(0, 639, 0, 479);
	}

	if (!screen().has_screen_update())
		screen().set_screen_update(*this, FUNC(psxgpu_device::update_screen));
}
