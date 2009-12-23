/***************************************************************************

    PSX GPU

    Preliminary software renderer by smf.
    Thanks to Ryan Holtz, Pete B & Farfetch'd.

    Supports:
      type 1 1024x1024 framebuffer (CXD8538Q)
      type 2 1024x512 framebuffer (CXD8561Q)
      type 2 1024x1024 framebuffer (CXD8514Q/CXD8561Q/CXD8654Q)

    Debug Keys:
        M toggles mesh viewer.
        V toggles vram viewer.
        I toggles interleave in vram viewer.

***************************************************************************/

#include "driver.h"
#include "includes/psx.h"

#define STOP_ON_ERROR ( 0 )

#define VERBOSE_LEVEL ( 0 )

INLINE void ATTR_PRINTF(3,4) verboselog( running_machine *machine, int n_level, const char *s_fmt, ... )
{
	if( VERBOSE_LEVEL >= n_level )
	{
		va_list v;
		char buf[ 32768 ];
		va_start( v, s_fmt );
		vsprintf( buf, s_fmt, v );
		va_end( v );
		logerror( "%s: %s", cpuexec_describe_context(machine), buf );
	}
}

struct FLATVERTEX
{
	PAIR n_coord;
};

struct GOURAUDVERTEX
{
	PAIR n_bgr;
	PAIR n_coord;
};

struct FLATTEXTUREDVERTEX
{
	PAIR n_coord;
	PAIR n_texture;
};

struct GOURAUDTEXTUREDVERTEX
{
	PAIR n_bgr;
	PAIR n_coord;
	PAIR n_texture;
};

static union
{
	UINT32 n_entry[ 16 ];

	struct
	{
		PAIR n_cmd;
		struct FLATVERTEX vertex[ 2 ];
		PAIR n_size;
	} MoveImage;

	struct
	{
		PAIR n_bgr;
		PAIR n_coord;
		PAIR n_size;
	} FlatRectangle;

	struct
	{
		PAIR n_bgr;
		PAIR n_coord;
	} FlatRectangle8x8;

	struct
	{
		PAIR n_bgr;
		PAIR n_coord;
	} FlatRectangle16x16;

	struct
	{
		PAIR n_bgr;
		PAIR n_coord;
		PAIR n_texture;
	} Sprite8x8;

	struct
	{
		PAIR n_bgr;
		PAIR n_coord;
		PAIR n_texture;
	} Sprite16x16;

	struct
	{
		PAIR n_bgr;
		PAIR n_coord;
		PAIR n_texture;
		PAIR n_size;
	} FlatTexturedRectangle;

	struct
	{
		PAIR n_bgr;
		struct FLATVERTEX vertex[ 4 ];
	} FlatPolygon;

	struct
	{
		struct GOURAUDVERTEX vertex[ 4 ];
	} GouraudPolygon;

	struct
	{
		PAIR n_bgr;
		struct FLATVERTEX vertex[ 2 ];
	} MonochromeLine;

	struct
	{
		struct GOURAUDVERTEX vertex[ 2 ];
	} GouraudLine;

	struct
	{
		PAIR n_bgr;
		struct FLATTEXTUREDVERTEX vertex[ 4 ];
	} FlatTexturedPolygon;

	struct
	{
		struct GOURAUDTEXTUREDVERTEX vertex[ 4 ];
	} GouraudTexturedPolygon;

	struct
	{
		PAIR n_bgr;
		struct FLATVERTEX vertex;
	} Dot;
} m_packet;

static struct PSXGPU
{
	INT32 n_tx;
	INT32 n_ty;
	INT32 n_abr;
	INT32 n_tp;
	INT32 n_ix;
	INT32 n_iy;
	INT32 n_ti;
} psxgpu;

static UINT16 *m_p_vram;
static UINT32 m_n_vram_size;
static UINT32 m_n_gpu_buffer_offset;
static UINT32 m_n_vramx;
static UINT32 m_n_vramy;
static UINT32 m_n_twy;
static UINT32 m_n_twx;
static UINT32 m_n_twh;
static UINT32 m_n_tww;
static UINT32 m_n_drawarea_x1;
static UINT32 m_n_drawarea_y1;
static UINT32 m_n_drawarea_x2;
static UINT32 m_n_drawarea_y2;
static UINT32 m_n_horiz_disstart;
static UINT32 m_n_horiz_disend;
static UINT32 m_n_vert_disstart;
static UINT32 m_n_vert_disend;
static UINT32 m_b_reverseflag;
static INT32 m_n_drawoffset_x;
static INT32 m_n_drawoffset_y;
static UINT32 m_n_displaystartx;
static UINT32 m_n_displaystarty;
static UINT32 m_n_gpustatus;
static UINT32 m_n_gpuinfo;
static UINT32 m_n_lightgun_x;
static UINT32 m_n_lightgun_y;
static UINT32 m_n_screenwidth;
static UINT32 m_n_screenheight;

#define MAX_LEVEL ( 32 )
#define MID_LEVEL ( ( MAX_LEVEL / 2 ) << 8 )
#define MAX_SHADE ( 0x100 )
#define MID_SHADE ( 0x80 )

static int m_n_gputype;

static UINT16 *m_p_p_vram[ 1024 ];

static UINT16 m_p_n_redshade[ MAX_LEVEL * MAX_SHADE ];
static UINT16 m_p_n_greenshade[ MAX_LEVEL * MAX_SHADE ];
static UINT16 m_p_n_blueshade[ MAX_LEVEL * MAX_SHADE ];
static UINT16 m_p_n_redlevel[ 0x10000 ];
static UINT16 m_p_n_greenlevel[ 0x10000 ];
static UINT16 m_p_n_bluelevel[ 0x10000 ];

static UINT16 m_p_n_f025[ MAX_LEVEL * MAX_SHADE ];
static UINT16 m_p_n_f05[ MAX_LEVEL * MAX_SHADE ];
static UINT16 m_p_n_f1[ MAX_LEVEL * MAX_SHADE ];
static UINT16 m_p_n_redb05[ 0x10000 ];
static UINT16 m_p_n_greenb05[ 0x10000 ];
static UINT16 m_p_n_blueb05[ 0x10000 ];
static UINT16 m_p_n_redb1[ 0x10000 ];
static UINT16 m_p_n_greenb1[ 0x10000 ];
static UINT16 m_p_n_blueb1[ 0x10000 ];
static UINT16 m_p_n_redaddtrans[ MAX_LEVEL * MAX_LEVEL ];
static UINT16 m_p_n_greenaddtrans[ MAX_LEVEL * MAX_LEVEL ];
static UINT16 m_p_n_blueaddtrans[ MAX_LEVEL * MAX_LEVEL ];
static UINT16 m_p_n_redsubtrans[ MAX_LEVEL * MAX_LEVEL ];
static UINT16 m_p_n_greensubtrans[ MAX_LEVEL * MAX_LEVEL ];
static UINT16 m_p_n_bluesubtrans[ MAX_LEVEL * MAX_LEVEL ];
static const UINT16 m_p_n_nextpointlist4[] = { 1, 3, 0, 2 };
static const UINT16 m_p_n_prevpointlist4[] = { 2, 0, 3, 1 };
static const UINT16 m_p_n_nextpointlist3[] = { 1, 2, 0 };
static const UINT16 m_p_n_prevpointlist3[] = { 2, 0, 1 };

static UINT16 m_p_n_g0r0[ 0x10000 ];
static UINT16 m_p_n_b0[ 0x10000 ];
static UINT16 m_p_n_r1[ 0x10000 ];
static UINT16 m_p_n_b1g1[ 0x10000 ];

static int need_sianniv_vblank_hack;

#define SINT11( x ) ( ( (INT32)( x ) << 21 ) >> 21 )

#define ADJUST_COORD( a ) \
	a.w.l = COORD_X( a ) + m_n_drawoffset_x; \
	a.w.h = COORD_Y( a ) + m_n_drawoffset_y;

#define COORD_X( a ) ( (INT16)a.w.l )
#define COORD_Y( a ) ( (INT16)a.w.h )
#define SIZE_W( a ) ( a.w.l )
#define SIZE_H( a ) ( a.w.h )
#define BGR_C( a ) ( a.b.h3 )
#define BGR_B( a ) ( a.b.h2 )
#define BGR_G( a ) ( a.b.h )
#define BGR_R( a ) ( a.b.l )
#define TEXTURE_V( a ) ( (UINT8)a.b.h )
#define TEXTURE_U( a ) ( (UINT8)a.b.l )

PALETTE_INIT( psx )
{
	UINT32 n_colour;

	for( n_colour = 0; n_colour < 0x10000; n_colour++ )
	{
		palette_set_color_rgb( machine, n_colour, pal5bit(n_colour >> 0), pal5bit(n_colour >> 5), pal5bit(n_colour >> 10) );
	}
}

#if defined( MAME_DEBUG )

#define DEBUG_COORDS ( 10 )
static struct
{
	running_machine *machine;
	bitmap_t *mesh;
	int b_clear;
	int b_mesh;
	int n_skip;
	int b_texture;
	int n_interleave;
	int n_coord;
	int n_coordx[ DEBUG_COORDS ];
	int n_coordy[ DEBUG_COORDS ];
} m_debug;

#define DEBUG_MAX ( 512 )

static void DebugMeshInit( running_machine *machine )
{
	int width = video_screen_get_width(machine->primary_screen);
	int height = video_screen_get_height(machine->primary_screen);

	m_debug.b_mesh = 0;
	m_debug.b_texture = 0;
	m_debug.n_interleave = -1;
	m_debug.b_clear = 1;
	m_debug.n_coord = 0;
	m_debug.n_skip = 0;
	m_debug.mesh = auto_bitmap_alloc(machine, width, height, BITMAP_FORMAT_INDEXED16 );
	m_debug.machine = machine;
}

static void DebugMesh( int n_coordx, int n_coordy )
{
	running_machine *machine = m_debug.machine;
	int n_coord;
	int n_colour;
	int width = video_screen_get_width(machine->primary_screen);
	int height = video_screen_get_height(machine->primary_screen);

	if( m_debug.b_clear )
	{
		bitmap_fill( m_debug.mesh, NULL , 0x0000);
		m_debug.b_clear = 0;
	}

	if( m_debug.n_coord < DEBUG_COORDS )
	{
		n_coordx += m_n_displaystartx;
		n_coordy += m_n_displaystarty;

		n_coordx *= 511;
		n_coordx /= DEBUG_MAX - 1;
		n_coordx += 256;
		n_coordy *= 511;
		n_coordy /= DEBUG_MAX - 1;
		n_coordy += 256;

		m_debug.n_coordx[ m_debug.n_coord ] = n_coordx;
		m_debug.n_coordy[ m_debug.n_coord ] = n_coordy;
		m_debug.n_coord++;
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

		n_x.w.h = n_xstart; n_x.w.l = 0;
		n_y.w.h = n_ystart; n_y.w.l = 0;

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
				if( *BITMAP_ADDR16(m_debug.mesh, n_y.w.h, n_x.w.h) != 0xffff )
					*BITMAP_ADDR16(m_debug.mesh, n_y.w.h, n_x.w.h) = n_colour;
			}
			n_x.d += n_dx;
			n_y.d += n_dy;
			n_len--;
		}
	}
}

static void DebugMeshEnd( void )
{
	m_debug.n_coord = 0;
}

static void DebugCheckKeys( running_machine *machine )
{
	if( input_code_pressed_once( machine, KEYCODE_M ) )
		m_debug.b_mesh = !m_debug.b_mesh;

	if( input_code_pressed_once( machine, KEYCODE_V ) )
		m_debug.b_texture = !m_debug.b_texture;

	if( m_debug.b_mesh || m_debug.b_texture )
	{
		int width = video_screen_get_width(machine->primary_screen);
		int height = video_screen_get_height(machine->primary_screen);
		video_screen_set_visarea(machine->primary_screen, 0, width - 1, 0, height - 1 );
	}
	else
		video_screen_set_visarea(machine->primary_screen, 0, m_n_screenwidth - 1, 0, m_n_screenheight - 1 );

	if( input_code_pressed_once( machine, KEYCODE_I ) )
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
	if( input_code_pressed_once( machine, KEYCODE_D ) )
	{
		FILE *f;
		int n_x;
		f = fopen( "dump.txt", "w" );
		for( n_y = 256; n_y < 512; n_y++ )
			for( n_x = 640; n_x < 1024; n_x++ )
				fprintf( f, "%04u,%04u = %04x\n", n_y, n_x, m_p_p_vram[ n_y ][ n_x ] );
		fclose( f );
	}
	if( input_code_pressed_once( machine, KEYCODE_S ) )
	{
		FILE *f;
		popmessage( "saving..." );
		f = fopen( "VRAM.BIN", "wb" );
		for( n_y = 0; n_y < 1024; n_y++ )
			fwrite( m_p_p_vram[ n_y ], 1024 * 2, 1, f );
		fclose( f );
	}
	if( input_code_pressed_once( machine, KEYCODE_L ) )
	{
		FILE *f;
		popmessage( "loading..." );
		f = fopen( "VRAM.BIN", "rb" );
		for( n_y = 0; n_y < 1024; n_y++ )
			fread( m_p_p_vram[ n_y ], 1024 * 2, 1, f );
		fclose( f );
	}
#endif
}

static int DebugMeshDisplay( bitmap_t *bitmap, const rectangle *cliprect )
{
	if( m_debug.mesh )
	{
		copybitmap( bitmap, m_debug.mesh, 0, 0, 0, 0, cliprect );
	}
	m_debug.b_clear = 1;
	return m_debug.b_mesh;
}

static int DebugTextureDisplay( running_machine *machine, bitmap_t *bitmap )
{
	UINT32 n_y;

	if( m_debug.b_texture )
	{
		int width = video_screen_get_width(machine->primary_screen);
		int height = video_screen_get_height(machine->primary_screen);

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
				p_n_interleave[ n_x ] = m_p_p_vram[ n_yi ][ n_xi ];
			}
			draw_scanline16( bitmap, 0, n_y, width, p_n_interleave, machine->pens );
		}
	}
	return m_debug.b_texture;
}

#endif

static STATE_POSTLOAD( updatevisiblearea )
{
	rectangle visarea;
	float refresh;

	if( ( m_n_gpustatus & ( 1 << 0x14 ) ) != 0 )
	{
		/* pal */
		refresh = 50;
		switch( ( m_n_gpustatus >> 0x13 ) & 1 )
		{
		case 0:
			m_n_screenheight = 256;
			break;
		case 1:
			m_n_screenheight = 512;
			break;
		}
	}
	else
	{
		/* ntsc */
		refresh = 60;
		switch( ( m_n_gpustatus >> 0x13 ) & 1 )
		{
		case 0:
			m_n_screenheight = 240;
			break;
		case 1:
			m_n_screenheight = 480;
			break;
		}
	}
	switch( ( m_n_gpustatus >> 0x11 ) & 3 )
	{
	case 0:
		switch( ( m_n_gpustatus >> 0x10 ) & 1 )
		{
		case 0:
			m_n_screenwidth = 256;
			break;
		case 1:
			m_n_screenwidth = 368;
			break;
		}
		break;
	case 1:
		switch( ( m_n_gpustatus >> 0x10 ) & 1 )
		{
		case 0:
			m_n_screenwidth = 320;
			break;
		case 1:
			m_n_screenwidth = 384;
			break;
		}
		break;
	case 2:
		m_n_screenwidth = 512;
		break;
	case 3:
		m_n_screenwidth = 640;
		break;
	}
	visarea.min_x = visarea.min_y = 0;
	visarea.max_x = m_n_screenwidth - 1;
	visarea.max_y = m_n_screenheight - 1;
	video_screen_configure(machine->primary_screen, m_n_screenwidth, m_n_screenheight, &visarea, HZ_TO_ATTOSECONDS(refresh));
}

static void psx_gpu_init( running_machine *machine )
{
	int n_line;
	int n_level;
	int n_level2;
	int n_shade;
	int n_shaded;
	int width = video_screen_get_width(machine->primary_screen);
	int height = video_screen_get_height(machine->primary_screen);

	need_sianniv_vblank_hack = !strcmp(machine->gamedrv->name, "sianniv");

#if defined( MAME_DEBUG )
	DebugMeshInit(machine);
#endif

	m_n_gpustatus = 0x14802000;
	m_n_gpuinfo = 0;
	m_n_gpu_buffer_offset = 0;
	m_n_lightgun_x = 0;
	m_n_lightgun_y = 0;

	m_n_vram_size = width * height;
	m_p_vram = auto_alloc_array_clear(machine, UINT16, m_n_vram_size );

	for( n_line = 0; n_line < 1024; n_line++ )
	{
		m_p_p_vram[ n_line ] = &m_p_vram[ ( n_line % height ) * width ];
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
			m_p_n_redshade[ ( n_level * MAX_SHADE ) | n_shade ] = n_shaded;
			m_p_n_greenshade[ ( n_level * MAX_SHADE ) | n_shade ] = n_shaded << 5;
			m_p_n_blueshade[ ( n_level * MAX_SHADE ) | n_shade ] = n_shaded << 10;

			/* 1/4 x transparency */
			n_shaded = ( n_level * n_shade ) / MID_SHADE;
			n_shaded >>= 2;
			if( n_shaded > MAX_LEVEL - 1 )
			{
				n_shaded = MAX_LEVEL - 1;
			}
			m_p_n_f025[ ( n_level * MAX_SHADE ) | n_shade ] = n_shaded;

			/* 1/2 x transparency */
			n_shaded = ( n_level * n_shade ) / MID_SHADE;
			n_shaded >>= 1;
			if( n_shaded > MAX_LEVEL - 1 )
			{
				n_shaded = MAX_LEVEL - 1;
			}
			m_p_n_f05[ ( n_level * MAX_SHADE ) | n_shade ] = n_shaded;

			/* 1 x transparency */
			n_shaded = ( n_level * n_shade ) / MID_SHADE;
			if( n_shaded > MAX_LEVEL - 1 )
			{
				n_shaded = MAX_LEVEL - 1;
			}
			m_p_n_f1[ ( n_level * MAX_SHADE ) | n_shade ] = n_shaded;
		}
	}

	for( n_level = 0; n_level < 0x10000; n_level++ )
	{
		m_p_n_redlevel[ n_level ] = ( n_level & ( MAX_LEVEL - 1 ) ) * MAX_SHADE;
		m_p_n_greenlevel[ n_level ] = ( ( n_level >> 5 ) & ( MAX_LEVEL - 1 ) ) * MAX_SHADE;
		m_p_n_bluelevel[ n_level ] = ( ( n_level >> 10 ) & ( MAX_LEVEL - 1 ) ) * MAX_SHADE;

		/* 0.5 * background */
		m_p_n_redb05[ n_level ] = ( ( n_level & ( MAX_LEVEL - 1 ) ) / 2 ) * MAX_LEVEL;
		m_p_n_greenb05[ n_level ] = ( ( ( n_level >> 5 ) & ( MAX_LEVEL - 1 ) ) / 2 ) * MAX_LEVEL;
		m_p_n_blueb05[ n_level ] = ( ( ( n_level >> 10 ) & ( MAX_LEVEL - 1 ) ) / 2 ) * MAX_LEVEL;

		/* 1 * background */
		m_p_n_redb1[ n_level ] = ( n_level & ( MAX_LEVEL - 1 ) ) * MAX_LEVEL;
		m_p_n_greenb1[ n_level ] = ( ( n_level >> 5 ) & ( MAX_LEVEL - 1 ) ) * MAX_LEVEL;
		m_p_n_blueb1[ n_level ] = ( ( n_level >> 10 ) & ( MAX_LEVEL - 1 ) ) * MAX_LEVEL;

		/* 24bit to 15 bit conversion */
		m_p_n_g0r0[ n_level ] = ( ( ( n_level >> 11 ) & ( MAX_LEVEL - 1 ) ) << 5 ) | ( ( ( n_level >> 3 ) & ( MAX_LEVEL - 1 ) ) << 0 );
		m_p_n_b0[ n_level ] = ( ( n_level >> 3 ) & ( MAX_LEVEL - 1 ) ) << 10;
		m_p_n_r1[ n_level ] = ( ( n_level >> 11 ) & ( MAX_LEVEL - 1 ) ) << 0;
		m_p_n_b1g1[ n_level ] = ( ( ( n_level >> 11 ) & ( MAX_LEVEL - 1 ) ) << 10 ) | ( ( ( n_level >> 3 ) & ( MAX_LEVEL - 1 ) ) << 5 );
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
			m_p_n_redaddtrans[ ( n_level * MAX_LEVEL ) | n_level2 ] = n_shaded;
			m_p_n_greenaddtrans[ ( n_level * MAX_LEVEL ) | n_level2 ] = n_shaded << 5;
			m_p_n_blueaddtrans[ ( n_level * MAX_LEVEL ) | n_level2 ] = n_shaded << 10;

			/* sub transparency */
			n_shaded = ( n_level - n_level2 );
			if( n_shaded < 0 )
			{
				n_shaded = 0;
			}
			m_p_n_redsubtrans[ ( n_level * MAX_LEVEL ) | n_level2 ] = n_shaded;
			m_p_n_greensubtrans[ ( n_level * MAX_LEVEL ) | n_level2 ] = n_shaded << 5;
			m_p_n_bluesubtrans[ ( n_level * MAX_LEVEL ) | n_level2 ] = n_shaded << 10;
		}
	}

	// icky!!!
	state_save_register_memory( machine, "globals", NULL, 0, "m_packet", (UINT8 *)&m_packet, 1, sizeof( m_packet ), __FILE__, __LINE__ );

	state_save_register_global_pointer(machine,  m_p_vram, m_n_vram_size );
	state_save_register_global(machine,  m_n_gpu_buffer_offset );
	state_save_register_global(machine,  m_n_vramx );
	state_save_register_global(machine,  m_n_vramy );
	state_save_register_global(machine,  m_n_twy );
	state_save_register_global(machine,  m_n_twx );
	state_save_register_global(machine,  m_n_tww );
	state_save_register_global(machine,  m_n_drawarea_x1 );
	state_save_register_global(machine,  m_n_drawarea_y1 );
	state_save_register_global(machine,  m_n_drawarea_x2 );
	state_save_register_global(machine,  m_n_drawarea_y2 );
	state_save_register_global(machine,  m_n_horiz_disstart );
	state_save_register_global(machine,  m_n_horiz_disend );
	state_save_register_global(machine,  m_n_vert_disstart );
	state_save_register_global(machine,  m_n_vert_disend );
	state_save_register_global(machine,  m_b_reverseflag );
	state_save_register_global(machine,  m_n_drawoffset_x );
	state_save_register_global(machine,  m_n_drawoffset_y );
	state_save_register_global(machine,  m_n_displaystartx );
	state_save_register_global(machine,  m_n_displaystarty );
	state_save_register_global(machine,  m_n_gpustatus );
	state_save_register_global(machine,  m_n_gpuinfo );
	state_save_register_global(machine,  m_n_lightgun_x );
	state_save_register_global(machine,  m_n_lightgun_y );
	state_save_register_global(machine,  psxgpu.n_tx );
	state_save_register_global(machine,  psxgpu.n_ty );
	state_save_register_global(machine,  psxgpu.n_abr );
	state_save_register_global(machine,  psxgpu.n_tp );
	state_save_register_global(machine,  psxgpu.n_ix );
	state_save_register_global(machine,  psxgpu.n_iy );
	state_save_register_global(machine,  psxgpu.n_ti );

	state_save_register_postload( machine, updatevisiblearea, NULL );
}

VIDEO_START( psx_type1 )
{
	m_n_gputype = 1;
	psx_gpu_init(machine);
}

VIDEO_START( psx_type2 )
{
	m_n_gputype = 2;
	psx_gpu_init(machine);
}

VIDEO_UPDATE( psx )
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

#if defined( MAME_DEBUG )
	if( DebugMeshDisplay( bitmap, cliprect ) )
	{
		return 0;
	}
	if( DebugTextureDisplay( screen->machine, bitmap ) )
	{
		return 0;
	}
#endif

	if( ( m_n_gpustatus & ( 1 << 0x17 ) ) != 0 )
	{
		/* todo: only draw to necessary area */
		bitmap_fill( bitmap, cliprect , 0);
	}
	else
	{
		if( m_b_reverseflag )
		{
			n_displaystartx = ( 1023 - m_n_displaystartx );
			/* todo: make this flip the screen, in the meantime.. */
			n_displaystartx -= ( m_n_screenwidth - 1 );
		}
		else
		{
			n_displaystartx = m_n_displaystartx;
		}

		if( ( m_n_gpustatus & ( 1 << 0x14 ) ) != 0 )
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

		n_top = (INT32)m_n_vert_disstart - n_overscantop;
		n_lines = (INT32)m_n_vert_disend - (INT32)m_n_vert_disstart;
		if( n_top < 0 )
		{
			n_y = -n_top;
			n_lines += n_top;
		}
		else
		{
			/* todo: draw top border */
			n_y = 0;
		}
		if( ( m_n_gpustatus & ( 1 << 0x16 ) ) != 0 )
		{
			/* interlaced */
			n_lines *= 2;
		}
		if( n_lines > m_n_screenheight - ( n_y + n_top ) )
		{
			n_lines = m_n_screenheight - ( n_y + n_top );
		}
		else
		{
			/* todo: draw bottom border */
		}

		n_left = ( ( (INT32)m_n_horiz_disstart - n_overscanleft ) * (INT32)m_n_screenwidth ) / 2560;
		n_columns = ( ( ( (INT32)m_n_horiz_disend - m_n_horiz_disstart ) * (INT32)m_n_screenwidth ) / 2560 );
		if( n_left < 0 )
		{
			n_x = -n_left;
			n_columns += n_left;
		}
		else
		{
			/* todo: draw left border */
			n_x = 0;
		}
		if( n_columns > m_n_screenwidth - ( n_x + n_left ) )
		{
			n_columns = m_n_screenwidth - ( n_x + n_left );
		}
		else
		{
			/* todo: draw right border */
		}

		if( ( m_n_gpustatus & ( 1 << 0x15 ) ) != 0 )
		{
			/* 24bit */
			n_line = n_lines;
			while( n_line > 0 )
			{
				UINT16 *p_n_src = m_p_p_vram[ n_y + m_n_displaystarty ] + n_x + n_displaystartx;
				UINT16 *p_n_dest = BITMAP_ADDR16(bitmap, n_y + n_top, n_x + n_left);

				n_column = n_columns;
				while( n_column > 0 )
				{
					UINT32 n_g0r0 = *( p_n_src++ );
					UINT32 n_r1b0 = *( p_n_src++ );
					UINT32 n_b1g1 = *( p_n_src++ );

					*( p_n_dest++ ) = m_p_n_g0r0[ n_g0r0 ] | m_p_n_b0[ n_r1b0 ];
					n_column--;
					if( n_column > 0 )
					{
						*( p_n_dest++ ) = m_p_n_r1[ n_r1b0 ] | m_p_n_b1g1[ n_b1g1 ];
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
				draw_scanline16( bitmap, n_x + n_left, n_y + n_top, n_columns, m_p_p_vram[ n_y + m_n_displaystarty ] + n_x + n_displaystartx, NULL );
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

INLINE void decode_tpage( running_machine *machine, struct PSXGPU *p_psxgpu, UINT32 tpage )
{
	if( m_n_gputype == 2 )
	{
		m_n_gpustatus = ( m_n_gpustatus & 0xfffff800 ) | ( tpage & 0x7ff );

		p_psxgpu->n_tx = ( tpage & 0x0f ) << 6;
		p_psxgpu->n_ty = ( ( tpage & 0x10 ) << 4 ) | ( ( tpage & 0x800 ) >> 2 );
		p_psxgpu->n_abr = ( tpage & 0x60 ) >> 5;
		p_psxgpu->n_tp = ( tpage & 0x180 ) >> 7;
		p_psxgpu->n_ix = ( tpage & 0x1000 ) >> 12;
		p_psxgpu->n_iy = ( tpage & 0x2000 ) >> 13;
		p_psxgpu->n_ti = 0;
		if( ( tpage & ~0x39ff ) != 0 )
		{
			verboselog( machine, 1, "not handled: draw mode %08x\n", tpage & ~0x39ff );
		}
		if( p_psxgpu->n_tp == 3 )
		{
			verboselog( machine, 0, "not handled: tp == 3\n" );
		}
	}
	else
	{
		m_n_gpustatus = ( m_n_gpustatus & 0xffffe000 ) | ( tpage & 0x1fff );

		p_psxgpu->n_tx = ( tpage & 0x0f ) << 6;
		p_psxgpu->n_ty = ( ( tpage & 0x60 ) << 3 );
		p_psxgpu->n_abr = ( tpage & 0x180 ) >> 7;
		p_psxgpu->n_tp = ( tpage & 0x600 ) >> 9;
		p_psxgpu->n_ti = ( tpage & 0x2000 ) >> 13;
		p_psxgpu->n_ix = 0;
		p_psxgpu->n_iy = 0;
		if( ( tpage & ~0x27ef ) != 0 )
		{
			verboselog( machine, 1, "not handled: draw mode %08x\n", tpage & ~0x27ef );
		}
		if( p_psxgpu->n_tp == 3 )
		{
			verboselog( machine, 0, "not handled: tp == 3\n" );
		}
		else if( p_psxgpu->n_tp == 2 && p_psxgpu->n_ti != 0 )
		{
			verboselog( machine, 0, "not handled: interleaved 15 bit texture\n" );
		}
	}
}

#define SPRITESETUP \
	if( psxgpu.n_iy != 0 ) \
	{ \
		n_dv = -1; \
	} \
	else \
	{ \
		n_dv = 1; \
	} \
	if( psxgpu.n_ix != 0 ) \
	{ \
		n_du = -1; \
	} \
	else \
	{ \
		n_du = 1; \
	}

#define TRANSPARENCYSETUP \
	p_n_f = m_p_n_f1; \
	p_n_redb = m_p_n_redb1; \
	p_n_greenb = m_p_n_greenb1; \
	p_n_blueb = m_p_n_blueb1; \
	p_n_redtrans = m_p_n_redaddtrans; \
	p_n_greentrans = m_p_n_greenaddtrans; \
	p_n_bluetrans = m_p_n_blueaddtrans; \
 \
	switch( n_cmd & 0x02 ) \
	{ \
	case 0x02: \
		switch( psxgpu.n_abr ) \
		{ \
		case 0x00: \
			p_n_f = m_p_n_f05; \
			p_n_redb = m_p_n_redb05; \
			p_n_greenb = m_p_n_greenb05; \
			p_n_blueb = m_p_n_blueb05; \
			p_n_redtrans = m_p_n_redaddtrans; \
			p_n_greentrans = m_p_n_greenaddtrans; \
			p_n_bluetrans = m_p_n_blueaddtrans; \
			verboselog( machine, 2, "Transparency Mode: 0.5*B + 0.5*F\n" ); \
			break; \
		case 0x01: \
			p_n_f = m_p_n_f1; \
			p_n_redb = m_p_n_redb1; \
			p_n_greenb = m_p_n_greenb1; \
			p_n_blueb = m_p_n_blueb1; \
			p_n_redtrans = m_p_n_redaddtrans; \
			p_n_greentrans = m_p_n_greenaddtrans; \
			p_n_bluetrans = m_p_n_blueaddtrans; \
			verboselog( machine, 2, "Transparency Mode: 1.0*B + 1.0*F\n" ); \
			break; \
		case 0x02: \
			p_n_f = m_p_n_f1; \
			p_n_redb = m_p_n_redb1; \
			p_n_greenb = m_p_n_greenb1; \
			p_n_blueb = m_p_n_blueb1; \
			p_n_redtrans = m_p_n_redsubtrans; \
			p_n_greentrans = m_p_n_greensubtrans; \
			p_n_bluetrans = m_p_n_bluesubtrans; \
			verboselog( machine, 2, "Transparency Mode: 1.0*B - 1.0*F\n" ); \
			break; \
		case 0x03: \
			p_n_f = m_p_n_f025; \
			p_n_redb = m_p_n_redb1; \
			p_n_greenb = m_p_n_greenb1; \
			p_n_blueb = m_p_n_blueb1; \
			p_n_redtrans = m_p_n_redaddtrans; \
			p_n_greentrans = m_p_n_greenaddtrans; \
			p_n_bluetrans = m_p_n_blueaddtrans; \
			verboselog( machine, 2, "Transparency Mode: 1.0*B + 0.25*F\n" ); \
			break; \
		} \
		break; \
	}

#define SOLIDSETUP \
	TRANSPARENCYSETUP

#define TEXTURESETUP \
	n_tx = psxgpu.n_tx; \
	n_ty = psxgpu.n_ty; \
	p_clut = m_p_p_vram[ n_cluty ] + n_clutx; \
	switch( psxgpu.n_tp ) \
	{ \
	case 0: \
		n_tx += m_n_twx >> 2; \
		n_ty += m_n_twy; \
		break; \
	case 1: \
		n_tx += m_n_twx >> 1; \
		n_ty += m_n_twy; \
		break; \
	case 2: \
		n_tx += m_n_twx >> 0; \
		n_ty += m_n_twy; \
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
	if( n_distance > ( (INT32)m_n_drawarea_x2 - n_x ) + 1 ) \
	{ \
		n_distance = ( m_n_drawarea_x2 - n_x ) + 1; \
	} \
	p_vram = m_p_p_vram[ n_y ] + n_x; \
 \
	switch( n_cmd & 0x02 ) \
	{ \
	case 0x00: \
		/* transparency off */ \
		while( n_distance > 0 ) \
		{ \
			WRITE_PIXEL( \
				m_p_n_redshade[ MID_LEVEL | n_r.w.h ] | \
				m_p_n_greenshade[ MID_LEVEL | n_g.w.h ] | \
				m_p_n_blueshade[ MID_LEVEL | n_b.w.h ] ); \
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
	} \

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
		n_bgr = p_clut[ ( *( m_p_p_vram[ n_ty + TXV ] + n_tx + ( TXU >> 2 ) ) >> ( ( TXU & 0x03 ) << 2 ) ) & 0x0f ];

#define TEXTURE8BIT( TXV, TXU ) \
	TEXTURE_LOOP \
		n_bgr = p_clut[ ( *( m_p_p_vram[ n_ty + TXV ] + n_tx + ( TXU >> 1 ) ) >> ( ( TXU & 0x01 ) << 3 ) ) & 0xff ];

#define TEXTURE15BIT( TXV, TXU ) \
	TEXTURE_LOOP \
		n_bgr = *( m_p_p_vram[ n_ty + TXV ] + n_tx + TXU );

#define TEXTUREWINDOW4BIT( TXV, TXU ) TEXTURE4BIT( ( TXV & m_n_twh ), ( TXU & m_n_tww ) )
#define TEXTUREWINDOW8BIT( TXV, TXU ) TEXTURE8BIT( ( TXV & m_n_twh ), ( TXU & m_n_tww ) )
#define TEXTUREWINDOW15BIT( TXV, TXU ) TEXTURE15BIT( ( TXV & m_n_twh ), ( TXU & m_n_tww ) )

#define TEXTUREINTERLEAVED4BIT( TXV, TXU ) \
	TEXTURE_LOOP \
		int n_xi = ( ( TXU >> 2 ) & ~0x3c ) + ( ( TXV << 2 ) & 0x3c ); \
		int n_yi = ( TXV & ~0xf ) + ( ( TXU >> 4 ) & 0xf ); \
		n_bgr = p_clut[ ( *( m_p_p_vram[ n_ty + n_yi ] + n_tx + n_xi ) >> ( ( TXU & 0x03 ) << 2 ) ) & 0x0f ];

#define TEXTUREINTERLEAVED8BIT( TXV, TXU ) \
	TEXTURE_LOOP \
		int n_xi = ( ( TXU >> 1 ) & ~0x78 ) + ( ( TXU << 2 ) & 0x40 ) + ( ( TXV << 3 ) & 0x38 ); \
		int n_yi = ( TXV & ~0x7 ) + ( ( TXU >> 5 ) & 0x7 ); \
		n_bgr = p_clut[ ( *( m_p_p_vram[ n_ty + n_yi ] + n_tx + n_xi ) >> ( ( TXU & 0x01 ) << 3 ) ) & 0xff ];

#define TEXTUREINTERLEAVED15BIT( TXV, TXU ) \
	TEXTURE_LOOP \
		int n_xi = TXU; \
		int n_yi = TXV; \
		n_bgr = *( m_p_p_vram[ n_ty + n_yi ] + n_tx + n_xi );

#define TEXTUREWINDOWINTERLEAVED4BIT( TXV, TXU ) TEXTUREINTERLEAVED4BIT( ( TXV & m_n_twh ), ( TXU & m_n_tww ) )
#define TEXTUREWINDOWINTERLEAVED8BIT( TXV, TXU ) TEXTUREINTERLEAVED8BIT( ( TXV & m_n_twh ), ( TXU & m_n_tww ) )
#define TEXTUREWINDOWINTERLEAVED15BIT( TXV, TXU ) TEXTUREINTERLEAVED15BIT( ( TXV & m_n_twh ), ( TXU & m_n_tww ) )

#define SHADEDPIXEL( PIXELUPDATE ) \
		if( n_bgr != 0 ) \
		{ \
			WRITE_PIXEL( \
				m_p_n_redshade[ m_p_n_redlevel[ n_bgr ] | n_r.w.h ] | \
				m_p_n_greenshade[ m_p_n_greenlevel[ n_bgr ] | n_g.w.h ] | \
				m_p_n_blueshade[ m_p_n_bluelevel[ n_bgr ] | n_b.w.h ] ); \
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
					p_n_redtrans[ p_n_f[ m_p_n_redlevel[ n_bgr ] | n_r.w.h ] | p_n_redb[ *( p_vram ) ] ] | \
					p_n_greentrans[ p_n_f[ m_p_n_greenlevel[ n_bgr ] | n_g.w.h ] | p_n_greenb[ *( p_vram ) ] ] | \
					p_n_bluetrans[ p_n_f[ m_p_n_bluelevel[ n_bgr ] | n_b.w.h ] | p_n_blueb[ *( p_vram ) ] ] ); \
			} \
			else \
			{ \
				WRITE_PIXEL( \
					m_p_n_redshade[ m_p_n_redlevel[ n_bgr ] | n_r.w.h ] | \
					m_p_n_greenshade[ m_p_n_greenlevel[ n_bgr ] | n_g.w.h ] | \
					m_p_n_blueshade[ m_p_n_bluelevel[ n_bgr ] | n_b.w.h ] ); \
			} \
		} \
		p_vram++; \
		PIXELUPDATE \
		n_distance--; \
	TEXTURE_ENDLOOP

#define TEXTUREFILL( PIXELUPDATE, TXU, TXV ) \
	if( n_distance > ( (INT32)m_n_drawarea_x2 - n_x ) + 1 ) \
	{ \
		n_distance = ( m_n_drawarea_x2 - n_x ) + 1; \
	} \
	p_vram = m_p_p_vram[ n_y ] + n_x; \
 \
	if( psxgpu.n_ti != 0 ) \
	{ \
		/* interleaved texture */ \
		if( m_n_twh != 255 || \
			m_n_tww != 255 || \
			m_n_twx != 0 || \
			m_n_twy != 0 ) \
		{ \
			/* texture window */ \
			switch( n_cmd & 0x02 ) \
			{ \
			case 0x00: \
				/* shading */ \
				switch( psxgpu.n_tp ) \
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
				switch( psxgpu.n_tp ) \
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
				switch( psxgpu.n_tp ) \
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
				switch( psxgpu.n_tp ) \
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
		if( m_n_twh != 255 || \
			m_n_tww != 255 || \
			m_n_twx != 0 || \
			m_n_twy != 0 ) \
		{ \
			/* texture window */ \
			switch( n_cmd & 0x02 ) \
			{ \
			case 0x00: \
				/* shading */ \
				switch( psxgpu.n_tp ) \
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
				switch( psxgpu.n_tp ) \
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
				switch( psxgpu.n_tp ) \
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
				switch( psxgpu.n_tp ) \
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

static void FlatPolygon( running_machine *machine, int n_points )
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

#if defined( MAME_DEBUG )
	if( m_debug.n_skip == 1 )
	{
		return;
	}
	for( n_point = 0; n_point < n_points; n_point++ )
	{
		DebugMesh( COORD_X( m_packet.FlatPolygon.vertex[ n_point ].n_coord ) + m_n_drawoffset_x, COORD_Y( m_packet.FlatPolygon.vertex[ n_point ].n_coord ) + m_n_drawoffset_y );
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

	if( n_points == 4 )
	{
		p_n_rightpointlist = m_p_n_nextpointlist4;
		p_n_leftpointlist = m_p_n_prevpointlist4;
	}
	else
	{
		p_n_rightpointlist = m_p_n_nextpointlist3;
		p_n_leftpointlist = m_p_n_prevpointlist3;
	}

	for( n_point = 0; n_point < n_points; n_point++ )
	{
		ADJUST_COORD( m_packet.FlatPolygon.vertex[ n_point ].n_coord );
	}

	n_leftpoint = 0;
	for( n_point = 1; n_point < n_points; n_point++ )
	{
		if( COORD_Y( m_packet.FlatPolygon.vertex[ n_point ].n_coord ) < COORD_Y( m_packet.FlatPolygon.vertex[ n_leftpoint ].n_coord ) ||
			( COORD_Y( m_packet.FlatPolygon.vertex[ n_point ].n_coord ) == COORD_Y( m_packet.FlatPolygon.vertex[ n_leftpoint ].n_coord ) &&
			COORD_X( m_packet.FlatPolygon.vertex[ n_point ].n_coord ) < COORD_X( m_packet.FlatPolygon.vertex[ n_leftpoint ].n_coord ) ) )
		{
			n_leftpoint = n_point;
		}
	}
	n_rightpoint = n_leftpoint;

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
			n_cx1.w.h = COORD_X( m_packet.FlatPolygon.vertex[ n_leftpoint ].n_coord ); n_cx1.w.l = 0;
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
			n_cx2.w.h = COORD_X( m_packet.FlatPolygon.vertex[ n_rightpoint ].n_coord ); n_cx2.w.l = 0;
			n_rightpoint = p_n_rightpointlist[ n_rightpoint ];
			n_distance = COORD_Y( m_packet.FlatPolygon.vertex[ n_rightpoint ].n_coord ) - n_y;
			if( n_distance < 1 )
			{
				break;
			}
			n_dx2 = (INT32)( ( COORD_X( m_packet.FlatPolygon.vertex[ n_rightpoint ].n_coord ) << 16 ) - n_cx2.d ) / n_distance;
		}
		if( (INT16)n_cx1.w.h != (INT16)n_cx2.w.h && n_y >= (INT32)m_n_drawarea_y1 && n_y <= (INT32)m_n_drawarea_y2 )
		{
			if( (INT16)n_cx1.w.h < (INT16)n_cx2.w.h )
			{
				n_x = n_cx1.w.h;
				n_distance = (INT16)n_cx2.w.h - n_x;
			}
			else
			{
				n_x = n_cx2.w.h;
				n_distance = (INT16)n_cx1.w.h - n_x;
			}

			if( ( (INT32)m_n_drawarea_x1 - n_x ) > 0 )
			{
				n_distance -= ( m_n_drawarea_x1 - n_x );
				n_x = m_n_drawarea_x1;
			}
			SOLIDFILL( FLATPOLYGONUPDATE )
		}
		n_cx1.d += n_dx1;
		n_cx2.d += n_dx2;
		n_y++;
	}
}

static void FlatTexturedPolygon( running_machine *machine, int n_points )
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

#if defined( MAME_DEBUG )
	if( m_debug.n_skip == 2 )
	{
		return;
	}
	for( n_point = 0; n_point < n_points; n_point++ )
	{
		DebugMesh( COORD_X( m_packet.FlatTexturedPolygon.vertex[ n_point ].n_coord ) + m_n_drawoffset_x, COORD_Y( m_packet.FlatTexturedPolygon.vertex[ n_point ].n_coord ) + m_n_drawoffset_y );
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

	decode_tpage( machine, &psxgpu, m_packet.FlatTexturedPolygon.vertex[ 1 ].n_texture.w.h );
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

	if( n_points == 4 )
	{
		p_n_rightpointlist = m_p_n_nextpointlist4;
		p_n_leftpointlist = m_p_n_prevpointlist4;
	}
	else
	{
		p_n_rightpointlist = m_p_n_nextpointlist3;
		p_n_leftpointlist = m_p_n_prevpointlist3;
	}

	for( n_point = 0; n_point < n_points; n_point++ )
	{
		ADJUST_COORD( m_packet.FlatTexturedPolygon.vertex[ n_point ].n_coord );
	}

	n_leftpoint = 0;
	for( n_point = 1; n_point < n_points; n_point++ )
	{
		if( COORD_Y( m_packet.FlatTexturedPolygon.vertex[ n_point ].n_coord ) < COORD_Y( m_packet.FlatTexturedPolygon.vertex[ n_leftpoint ].n_coord ) ||
			( COORD_Y( m_packet.FlatTexturedPolygon.vertex[ n_point ].n_coord ) == COORD_Y( m_packet.FlatTexturedPolygon.vertex[ n_leftpoint ].n_coord ) &&
			COORD_X( m_packet.FlatTexturedPolygon.vertex[ n_point ].n_coord ) < COORD_X( m_packet.FlatTexturedPolygon.vertex[ n_leftpoint ].n_coord ) ) )
		{
			n_leftpoint = n_point;
		}
	}
	n_rightpoint = n_leftpoint;

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
			n_cx1.w.h = COORD_X( m_packet.FlatTexturedPolygon.vertex[ n_leftpoint ].n_coord ); n_cx1.w.l = 0;
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
			n_cx2.w.h = COORD_X( m_packet.FlatTexturedPolygon.vertex[ n_rightpoint ].n_coord ); n_cx2.w.l = 0;
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
		if( (INT16)n_cx1.w.h != (INT16)n_cx2.w.h && n_y >= (INT32)m_n_drawarea_y1 && n_y <= (INT32)m_n_drawarea_y2 )
		{
			if( (INT16)n_cx1.w.h < (INT16)n_cx2.w.h )
			{
				n_x = n_cx1.w.h;
				n_distance = (INT16)n_cx2.w.h - n_x;

				n_u.d = n_cu1.d;
				n_v.d = n_cv1.d;
				n_du = (INT32)( n_cu2.d - n_cu1.d ) / n_distance;
				n_dv = (INT32)( n_cv2.d - n_cv1.d ) / n_distance;
			}
			else
			{
				n_x = n_cx2.w.h;
				n_distance = (INT16)n_cx1.w.h - n_x;

				n_u.d = n_cu2.d;
				n_v.d = n_cv2.d;
				n_du = (INT32)( n_cu1.d - n_cu2.d ) / n_distance;
				n_dv = (INT32)( n_cv1.d - n_cv2.d ) / n_distance;
			}

			if( ( (INT32)m_n_drawarea_x1 - n_x ) > 0 )
			{
				n_u.d += n_du * ( m_n_drawarea_x1 - n_x );
				n_v.d += n_dv * ( m_n_drawarea_x1 - n_x );
				n_distance -= ( m_n_drawarea_x1 - n_x );
				n_x = m_n_drawarea_x1;
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

static void GouraudPolygon( running_machine *machine, int n_points )
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

#if defined( MAME_DEBUG )
	if( m_debug.n_skip == 3 )
	{
		return;
	}
	for( n_point = 0; n_point < n_points; n_point++ )
	{
		DebugMesh( COORD_X( m_packet.GouraudPolygon.vertex[ n_point ].n_coord ) + m_n_drawoffset_x, COORD_Y( m_packet.GouraudPolygon.vertex[ n_point ].n_coord ) + m_n_drawoffset_y );
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

	if( n_points == 4 )
	{
		p_n_rightpointlist = m_p_n_nextpointlist4;
		p_n_leftpointlist = m_p_n_prevpointlist4;
	}
	else
	{
		p_n_rightpointlist = m_p_n_nextpointlist3;
		p_n_leftpointlist = m_p_n_prevpointlist3;
	}

	for( n_point = 0; n_point < n_points; n_point++ )
	{
		ADJUST_COORD( m_packet.GouraudPolygon.vertex[ n_point ].n_coord );
	}

	n_leftpoint = 0;
	for( n_point = 1; n_point < n_points; n_point++ )
	{
		if( COORD_Y( m_packet.GouraudPolygon.vertex[ n_point ].n_coord ) < COORD_Y( m_packet.GouraudPolygon.vertex[ n_leftpoint ].n_coord ) ||
			( COORD_Y( m_packet.GouraudPolygon.vertex[ n_point ].n_coord ) == COORD_Y( m_packet.GouraudPolygon.vertex[ n_leftpoint ].n_coord ) &&
			COORD_X( m_packet.GouraudPolygon.vertex[ n_point ].n_coord ) < COORD_X( m_packet.GouraudPolygon.vertex[ n_leftpoint ].n_coord ) ) )
		{
			n_leftpoint = n_point;
		}
	}
	n_rightpoint = n_leftpoint;

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
			n_cx1.w.h = COORD_X( m_packet.GouraudPolygon.vertex[ n_leftpoint ].n_coord ); n_cx1.w.l = 0;
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
			n_cx2.w.h = COORD_X( m_packet.GouraudPolygon.vertex[ n_rightpoint ].n_coord ); n_cx2.w.l = 0;
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
		if( (INT16)n_cx1.w.h != (INT16)n_cx2.w.h && n_y >= (INT32)m_n_drawarea_y1 && n_y <= (INT32)m_n_drawarea_y2 )
		{
			if( (INT16)n_cx1.w.h < (INT16)n_cx2.w.h )
			{
				n_x = n_cx1.w.h;
				n_distance = (INT16)n_cx2.w.h - n_x;

				n_r.d = n_cr1.d;
				n_g.d = n_cg1.d;
				n_b.d = n_cb1.d;
				n_dr = (INT32)( n_cr2.d - n_cr1.d ) / n_distance;
				n_dg = (INT32)( n_cg2.d - n_cg1.d ) / n_distance;
				n_db = (INT32)( n_cb2.d - n_cb1.d ) / n_distance;
			}
			else
			{
				n_x = n_cx2.w.h;
				n_distance = (INT16)n_cx1.w.h - n_x;

				n_r.d = n_cr2.d;
				n_g.d = n_cg2.d;
				n_b.d = n_cb2.d;
				n_dr = (INT32)( n_cr1.d - n_cr2.d ) / n_distance;
				n_dg = (INT32)( n_cg1.d - n_cg2.d ) / n_distance;
				n_db = (INT32)( n_cb1.d - n_cb2.d ) / n_distance;
			}

			if( ( (INT32)m_n_drawarea_x1 - n_x ) > 0 )
			{
				n_r.d += n_dr * ( m_n_drawarea_x1 - n_x );
				n_g.d += n_dg * ( m_n_drawarea_x1 - n_x );
				n_b.d += n_db * ( m_n_drawarea_x1 - n_x );
				n_distance -= ( m_n_drawarea_x1 - n_x );
				n_x = m_n_drawarea_x1;
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

static void GouraudTexturedPolygon( running_machine *machine, int n_points )
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

#if defined( MAME_DEBUG )
	if( m_debug.n_skip == 4 )
	{
		return;
	}
	for( n_point = 0; n_point < n_points; n_point++ )
	{
		DebugMesh( COORD_X( m_packet.GouraudTexturedPolygon.vertex[ n_point ].n_coord ) + m_n_drawoffset_x, COORD_Y( m_packet.GouraudTexturedPolygon.vertex[ n_point ].n_coord ) + m_n_drawoffset_y );
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

	decode_tpage( machine, &psxgpu, m_packet.GouraudTexturedPolygon.vertex[ 1 ].n_texture.w.h );
	TEXTURESETUP

	if( n_points == 4 )
	{
		p_n_rightpointlist = m_p_n_nextpointlist4;
		p_n_leftpointlist = m_p_n_prevpointlist4;
	}
	else
	{
		p_n_rightpointlist = m_p_n_nextpointlist3;
		p_n_leftpointlist = m_p_n_prevpointlist3;
	}

	for( n_point = 0; n_point < n_points; n_point++ )
	{
		ADJUST_COORD( m_packet.GouraudTexturedPolygon.vertex[ n_point ].n_coord );
	}

	n_leftpoint = 0;
	for( n_point = 1; n_point < n_points; n_point++ )
	{
		if( COORD_Y( m_packet.GouraudTexturedPolygon.vertex[ n_point ].n_coord ) < COORD_Y( m_packet.GouraudTexturedPolygon.vertex[ n_leftpoint ].n_coord ) ||
			( COORD_Y( m_packet.GouraudTexturedPolygon.vertex[ n_point ].n_coord ) == COORD_Y( m_packet.GouraudTexturedPolygon.vertex[ n_leftpoint ].n_coord ) &&
			COORD_X( m_packet.GouraudTexturedPolygon.vertex[ n_point ].n_coord ) < COORD_X( m_packet.GouraudTexturedPolygon.vertex[ n_leftpoint ].n_coord ) ) )
		{
			n_leftpoint = n_point;
		}
	}
	n_rightpoint = n_leftpoint;

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
			n_cx1.w.h = COORD_X( m_packet.GouraudTexturedPolygon.vertex[ n_leftpoint ].n_coord ); n_cx1.w.l = 0;
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
			n_cx2.w.h = COORD_X( m_packet.GouraudTexturedPolygon.vertex[ n_rightpoint ].n_coord ); n_cx2.w.l = 0;
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
		if( (INT16)n_cx1.w.h != (INT16)n_cx2.w.h && n_y >= (INT32)m_n_drawarea_y1 && n_y <= (INT32)m_n_drawarea_y2 )
		{
			if( (INT16)n_cx1.w.h < (INT16)n_cx2.w.h )
			{
				n_x = n_cx1.w.h;
				n_distance = (INT16)n_cx2.w.h - n_x;

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
				n_x = n_cx2.w.h;
				n_distance = (INT16)n_cx1.w.h - n_x;

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

			if( ( (INT32)m_n_drawarea_x1 - n_x ) > 0 )
			{
				n_r.d += n_dr * ( m_n_drawarea_x1 - n_x );
				n_g.d += n_dg * ( m_n_drawarea_x1 - n_x );
				n_b.d += n_db * ( m_n_drawarea_x1 - n_x );
				n_u.d += n_du * ( m_n_drawarea_x1 - n_x );
				n_v.d += n_dv * ( m_n_drawarea_x1 - n_x );
				n_distance -= ( m_n_drawarea_x1 - n_x );
				n_x = m_n_drawarea_x1;
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

static void MonochromeLine( void )
{
	PAIR n_x;
	PAIR n_y;
	INT32 n_dx;
	INT32 n_dy;
	INT32 n_dr;
	INT32 n_dg;
	INT32 n_db;
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

#if defined( MAME_DEBUG )
	if( m_debug.n_skip == 5 )
	{
		return;
	}
	DebugMesh( COORD_X( m_packet.MonochromeLine.vertex[ 0 ].n_coord ) + m_n_drawoffset_x, COORD_Y( m_packet.MonochromeLine.vertex[ 0 ].n_coord ) + m_n_drawoffset_y );
	DebugMesh( COORD_X( m_packet.MonochromeLine.vertex[ 1 ].n_coord ) + m_n_drawoffset_x, COORD_Y( m_packet.MonochromeLine.vertex[ 1 ].n_coord ) + m_n_drawoffset_y );
	DebugMeshEnd();
#endif

	n_xstart = COORD_X( m_packet.MonochromeLine.vertex[ 0 ].n_coord ) + m_n_drawoffset_x;
	n_xend = COORD_X( m_packet.MonochromeLine.vertex[ 1 ].n_coord ) + m_n_drawoffset_x;
	n_ystart = COORD_Y( m_packet.MonochromeLine.vertex[ 0 ].n_coord ) + m_n_drawoffset_y;
	n_yend = COORD_Y( m_packet.MonochromeLine.vertex[ 1 ].n_coord ) + m_n_drawoffset_y;

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

	n_x.w.h = n_xstart; n_x.w.l = 0;
	n_y.w.h = n_ystart; n_y.w.l = 0;

	n_dx = (INT32)( ( n_xend << 16 ) - n_x.d ) / n_len;
	n_dy = (INT32)( ( n_yend << 16 ) - n_y.d ) / n_len;
	n_dr = 0;
	n_dg = 0;
	n_db = 0;

	while( n_len > 0 )
	{
		if( (INT16)n_x.w.h >= (INT32)m_n_drawarea_x1 &&
			(INT16)n_y.w.h >= (INT32)m_n_drawarea_y1 &&
			(INT16)n_x.w.h <= (INT32)m_n_drawarea_x2 &&
			(INT16)n_y.w.h <= (INT32)m_n_drawarea_y2 )
		{
			p_vram = m_p_p_vram[ n_y.w.h ] + n_x.w.h;
			WRITE_PIXEL(
				m_p_n_redshade[ MID_LEVEL | n_r ] |
				m_p_n_greenshade[ MID_LEVEL | n_g ] |
				m_p_n_blueshade[ MID_LEVEL | n_b ] );
		}
		n_x.d += n_dx;
		n_y.d += n_dy;
		n_len--;
	}
}

static void GouraudLine( void )
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

#if defined( MAME_DEBUG )
	if( m_debug.n_skip == 6 )
	{
		return;
	}
	DebugMesh( COORD_X( m_packet.GouraudLine.vertex[ 0 ].n_coord ) + m_n_drawoffset_x, COORD_Y( m_packet.GouraudLine.vertex[ 0 ].n_coord ) + m_n_drawoffset_y );
	DebugMesh( COORD_X( m_packet.GouraudLine.vertex[ 1 ].n_coord ) + m_n_drawoffset_x, COORD_Y( m_packet.GouraudLine.vertex[ 1 ].n_coord ) + m_n_drawoffset_y );
	DebugMeshEnd();
#endif

	n_xstart = COORD_X( m_packet.GouraudLine.vertex[ 0 ].n_coord ) + m_n_drawoffset_x;
	n_ystart = COORD_Y( m_packet.GouraudLine.vertex[ 0 ].n_coord ) + m_n_drawoffset_y;
	n_cr1.w.h = BGR_R( m_packet.GouraudLine.vertex[ 0 ].n_bgr ); n_cr1.w.l = 0;
	n_cg1.w.h = BGR_G( m_packet.GouraudLine.vertex[ 0 ].n_bgr ); n_cg1.w.l = 0;
	n_cb1.w.h = BGR_B( m_packet.GouraudLine.vertex[ 0 ].n_bgr ); n_cb1.w.l = 0;

	n_xend = COORD_X( m_packet.GouraudLine.vertex[ 1 ].n_coord ) + m_n_drawoffset_x;
	n_yend = COORD_Y( m_packet.GouraudLine.vertex[ 1 ].n_coord ) + m_n_drawoffset_y;
	n_cr2.w.h = BGR_R( m_packet.GouraudLine.vertex[ 1 ].n_bgr ); n_cr1.w.l = 0;
	n_cg2.w.h = BGR_G( m_packet.GouraudLine.vertex[ 1 ].n_bgr ); n_cg1.w.l = 0;
	n_cb2.w.h = BGR_B( m_packet.GouraudLine.vertex[ 1 ].n_bgr ); n_cb1.w.l = 0;

	n_x.w.h = n_xstart; n_x.w.l = 0;
	n_y.w.h = n_ystart; n_y.w.l = 0;
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

	n_dx = (INT32)( ( n_xend << 16 ) - n_x.d ) / n_distance;
	n_dy = (INT32)( ( n_yend << 16 ) - n_y.d ) / n_distance;
	n_dr = (INT32)( n_cr2.d - n_cr1.d ) / n_distance;
	n_dg = (INT32)( n_cg2.d - n_cg1.d ) / n_distance;
	n_db = (INT32)( n_cb2.d - n_cb1.d ) / n_distance;

	while( n_distance > 0 )
	{
		if( (INT16)n_x.w.h >= (INT32)m_n_drawarea_x1 &&
			(INT16)n_y.w.h >= (INT32)m_n_drawarea_y1 &&
			(INT16)n_x.w.h <= (INT32)m_n_drawarea_x2 &&
			(INT16)n_y.w.h <= (INT32)m_n_drawarea_y2 )
		{
			p_vram = m_p_p_vram[ n_y.w.h ] + n_x.w.h;
			WRITE_PIXEL(
				m_p_n_redshade[ MID_LEVEL | n_r.w.h ] |
				m_p_n_greenshade[ MID_LEVEL | n_g.w.h ] |
				m_p_n_blueshade[ MID_LEVEL | n_b.w.h ] );
		}
		n_x.d += n_dx;
		n_y.d += n_dy;
		n_r.d += n_dr;
		n_g.d += n_dg;
		n_b.d += n_db;
		n_distance--;
	}
}

static void FrameBufferRectangleDraw( void )
{
	PAIR n_r;
	PAIR n_g;
	PAIR n_b;
	INT32 n_distance;
	INT32 n_h;
	INT16 n_y;
	INT16 n_x;
	UINT16 *p_vram;

#if defined( MAME_DEBUG )
	if( m_debug.n_skip == 7 )
	{
		return;
	}
	DebugMesh( COORD_X( m_packet.FlatRectangle.n_coord ), COORD_Y( m_packet.FlatRectangle.n_coord ) );
	DebugMesh( COORD_X( m_packet.FlatRectangle.n_coord ) + SIZE_W( m_packet.FlatRectangle.n_size ), COORD_Y( m_packet.FlatRectangle.n_coord ) );
	DebugMesh( COORD_X( m_packet.FlatRectangle.n_coord ), COORD_Y( m_packet.FlatRectangle.n_coord ) + SIZE_H( m_packet.FlatRectangle.n_size ) );
	DebugMesh( COORD_X( m_packet.FlatRectangle.n_coord ) + SIZE_W( m_packet.FlatRectangle.n_size ), COORD_Y( m_packet.FlatRectangle.n_coord ) + SIZE_H( m_packet.FlatRectangle.n_size ) );
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
			p_vram = m_p_p_vram[ n_y & 1023 ] + ( n_x & 1023 );
			WRITE_PIXEL(
				m_p_n_redshade[ MID_LEVEL | n_r.w.h ] |
				m_p_n_greenshade[ MID_LEVEL | n_g.w.h ] |
				m_p_n_blueshade[ MID_LEVEL | n_b.w.h ] );
			n_x++;
			n_distance--;
		}
		n_y++;
		n_h--;
	}
}

static void FlatRectangle( running_machine *machine )
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

#if defined( MAME_DEBUG )
	if( m_debug.n_skip == 8 )
	{
		return;
	}
	DebugMesh( COORD_X( m_packet.FlatRectangle.n_coord ) + m_n_drawoffset_x, COORD_Y( m_packet.FlatRectangle.n_coord ) + m_n_drawoffset_y );
	DebugMesh( COORD_X( m_packet.FlatRectangle.n_coord ) + m_n_drawoffset_x + SIZE_W( m_packet.FlatRectangle.n_size ), COORD_Y( m_packet.FlatRectangle.n_coord ) + m_n_drawoffset_y );
	DebugMesh( COORD_X( m_packet.FlatRectangle.n_coord ) + m_n_drawoffset_x, COORD_Y( m_packet.FlatRectangle.n_coord ) + m_n_drawoffset_y + SIZE_H( m_packet.FlatRectangle.n_size ) );
	DebugMesh( COORD_X( m_packet.FlatRectangle.n_coord ) + m_n_drawoffset_x + SIZE_W( m_packet.FlatRectangle.n_size ), COORD_Y( m_packet.FlatRectangle.n_coord ) + m_n_drawoffset_y + SIZE_H( m_packet.FlatRectangle.n_size ) );
	DebugMeshEnd();
#endif

	n_cmd = BGR_C( m_packet.FlatRectangle.n_bgr );

	SOLIDSETUP

	n_r.w.h = BGR_R( m_packet.FlatRectangle.n_bgr ); n_r.w.l = 0;
	n_g.w.h = BGR_G( m_packet.FlatRectangle.n_bgr ); n_g.w.l = 0;
	n_b.w.h = BGR_B( m_packet.FlatRectangle.n_bgr ); n_b.w.l = 0;

	n_y = COORD_Y( m_packet.FlatRectangle.n_coord ) + m_n_drawoffset_y;
	n_h = SIZE_H( m_packet.FlatRectangle.n_size );

	while( n_h > 0 )
	{
		n_x = COORD_X( m_packet.FlatRectangle.n_coord ) + m_n_drawoffset_x;

		n_distance = SIZE_W( m_packet.FlatRectangle.n_size );
		if( n_distance > 0 && n_y >= (INT32)m_n_drawarea_y1 && n_y <= (INT32)m_n_drawarea_y2 )
		{
			if( ( (INT32)m_n_drawarea_x1 - n_x ) > 0 )
			{
				n_distance -= ( m_n_drawarea_x1 - n_x );
				n_x = m_n_drawarea_x1;
			}
			SOLIDFILL( FLATRECTANGEUPDATE )
		}
		n_y++;
		n_h--;
	}
}

static void FlatRectangle8x8( running_machine *machine )
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

#if defined( MAME_DEBUG )
	if( m_debug.n_skip == 9 )
	{
		return;
	}
	DebugMesh( COORD_X( m_packet.FlatRectangle8x8.n_coord ) + m_n_drawoffset_x, COORD_Y( m_packet.FlatRectangle8x8.n_coord ) + m_n_drawoffset_y );
	DebugMesh( COORD_X( m_packet.FlatRectangle8x8.n_coord ) + m_n_drawoffset_x + 8, COORD_Y( m_packet.FlatRectangle8x8.n_coord ) + m_n_drawoffset_y );
	DebugMesh( COORD_X( m_packet.FlatRectangle8x8.n_coord ) + m_n_drawoffset_x, COORD_Y( m_packet.FlatRectangle8x8.n_coord ) + m_n_drawoffset_y + 8 );
	DebugMesh( COORD_X( m_packet.FlatRectangle8x8.n_coord ) + m_n_drawoffset_x + 8, COORD_Y( m_packet.FlatRectangle8x8.n_coord ) + m_n_drawoffset_y + 8 );
	DebugMeshEnd();
#endif

	n_cmd = BGR_C( m_packet.FlatRectangle8x8.n_bgr );

	SOLIDSETUP

	n_r.w.h = BGR_R( m_packet.FlatRectangle8x8.n_bgr ); n_r.w.l = 0;
	n_g.w.h = BGR_G( m_packet.FlatRectangle8x8.n_bgr ); n_g.w.l = 0;
	n_b.w.h = BGR_B( m_packet.FlatRectangle8x8.n_bgr ); n_b.w.l = 0;

	n_y = COORD_Y( m_packet.FlatRectangle8x8.n_coord ) + m_n_drawoffset_y;
	n_h = 8;

	while( n_h > 0 )
	{
		n_x = COORD_X( m_packet.FlatRectangle8x8.n_coord ) + m_n_drawoffset_x;

		n_distance = 8;
		if( n_distance > 0 && n_y >= (INT32)m_n_drawarea_y1 && n_y <= (INT32)m_n_drawarea_y2 )
		{
			if( ( (INT32)m_n_drawarea_x1 - n_x ) > 0 )
			{
				n_distance -= ( m_n_drawarea_x1 - n_x );
				n_x = m_n_drawarea_x1;
			}
			SOLIDFILL( FLATRECTANGEUPDATE )
		}
		n_y++;
		n_h--;
	}
}

static void FlatRectangle16x16( running_machine *machine )
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

#if defined( MAME_DEBUG )
	if( m_debug.n_skip == 10 )
	{
		return;
	}
	DebugMesh( COORD_X( m_packet.FlatRectangle16x16.n_coord ) + m_n_drawoffset_x, COORD_Y( m_packet.FlatRectangle16x16.n_coord ) + m_n_drawoffset_y );
	DebugMesh( COORD_X( m_packet.FlatRectangle16x16.n_coord ) + m_n_drawoffset_x + 16, COORD_Y( m_packet.FlatRectangle16x16.n_coord ) + m_n_drawoffset_y );
	DebugMesh( COORD_X( m_packet.FlatRectangle16x16.n_coord ) + m_n_drawoffset_x, COORD_Y( m_packet.FlatRectangle16x16.n_coord ) + m_n_drawoffset_y + 16 );
	DebugMesh( COORD_X( m_packet.FlatRectangle16x16.n_coord ) + m_n_drawoffset_x + 16, COORD_Y( m_packet.FlatRectangle16x16.n_coord ) + m_n_drawoffset_y + 16 );
	DebugMeshEnd();
#endif

	n_cmd = BGR_C( m_packet.FlatRectangle16x16.n_bgr );

	SOLIDSETUP

	n_r.w.h = BGR_R( m_packet.FlatRectangle16x16.n_bgr ); n_r.w.l = 0;
	n_g.w.h = BGR_G( m_packet.FlatRectangle16x16.n_bgr ); n_g.w.l = 0;
	n_b.w.h = BGR_B( m_packet.FlatRectangle16x16.n_bgr ); n_b.w.l = 0;

	n_y = COORD_Y( m_packet.FlatRectangle16x16.n_coord ) + m_n_drawoffset_y;
	n_h = 16;

	while( n_h > 0 )
	{
		n_x = COORD_X( m_packet.FlatRectangle16x16.n_coord ) + m_n_drawoffset_x;

		n_distance = 16;
		if( n_distance > 0 && n_y >= (INT32)m_n_drawarea_y1 && n_y <= (INT32)m_n_drawarea_y2 )
		{
			if( ( (INT32)m_n_drawarea_x1 - n_x ) > 0 )
			{
				n_distance -= ( m_n_drawarea_x1 - n_x );
				n_x = m_n_drawarea_x1;
			}
			SOLIDFILL( FLATRECTANGEUPDATE )
		}
		n_y++;
		n_h--;
	}
}

static void FlatTexturedRectangle( running_machine *machine )
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

#if defined( MAME_DEBUG )
	if( m_debug.n_skip == 11 )
	{
		return;
	}
	DebugMesh( COORD_X( m_packet.FlatTexturedRectangle.n_coord ) + m_n_drawoffset_x, COORD_Y( m_packet.FlatTexturedRectangle.n_coord ) + m_n_drawoffset_y );
	DebugMesh( COORD_X( m_packet.FlatTexturedRectangle.n_coord ) + m_n_drawoffset_x + SIZE_W( m_packet.FlatTexturedRectangle.n_size ), COORD_Y( m_packet.FlatTexturedRectangle.n_coord ) + m_n_drawoffset_y );
	DebugMesh( COORD_X( m_packet.FlatTexturedRectangle.n_coord ) + m_n_drawoffset_x, COORD_Y( m_packet.FlatTexturedRectangle.n_coord ) + m_n_drawoffset_y + SIZE_H( m_packet.FlatTexturedRectangle.n_size ) );
	DebugMesh( COORD_X( m_packet.FlatTexturedRectangle.n_coord ) + m_n_drawoffset_x + SIZE_W( m_packet.FlatTexturedRectangle.n_size ), COORD_Y( m_packet.FlatTexturedRectangle.n_coord ) + m_n_drawoffset_y + SIZE_H( m_packet.FlatTexturedRectangle.n_size ) );
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

	n_v = TEXTURE_V( m_packet.FlatTexturedRectangle.n_texture );
	n_y = COORD_Y( m_packet.FlatTexturedRectangle.n_coord ) + m_n_drawoffset_y;
	n_h = SIZE_H( m_packet.FlatTexturedRectangle.n_size );

	while( n_h > 0 )
	{
		n_x = COORD_X( m_packet.FlatTexturedRectangle.n_coord ) + m_n_drawoffset_x;
		n_u = TEXTURE_U( m_packet.FlatTexturedRectangle.n_texture );

		n_distance = SIZE_W( m_packet.FlatTexturedRectangle.n_size );
		if( n_distance > 0 && n_y >= (INT32)m_n_drawarea_y1 && n_y <= (INT32)m_n_drawarea_y2 )
		{
			if( ( (INT32)m_n_drawarea_x1 - n_x ) > 0 )
			{
				n_u += ( m_n_drawarea_x1 - n_x ) * n_du;
				n_distance -= ( m_n_drawarea_x1 - n_x );
				n_x = m_n_drawarea_x1;
			}
			TEXTUREFILL( FLATTEXTUREDRECTANGLEUPDATE, n_u, n_v );
		}
		n_v += n_dv;
		n_y++;
		n_h--;
	}
}

static void Sprite8x8( running_machine *machine )
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

#if defined( MAME_DEBUG )
	if( m_debug.n_skip == 12 )
	{
		return;
	}
	DebugMesh( COORD_X( m_packet.Sprite8x8.n_coord ) + m_n_drawoffset_x, COORD_Y( m_packet.Sprite8x8.n_coord ) + m_n_drawoffset_y );
	DebugMesh( COORD_X( m_packet.Sprite8x8.n_coord ) + m_n_drawoffset_x + 7, COORD_Y( m_packet.Sprite8x8.n_coord ) + m_n_drawoffset_y );
	DebugMesh( COORD_X( m_packet.Sprite8x8.n_coord ) + m_n_drawoffset_x, COORD_Y( m_packet.Sprite8x8.n_coord ) + m_n_drawoffset_y + 7 );
	DebugMesh( COORD_X( m_packet.Sprite8x8.n_coord ) + m_n_drawoffset_x + 7, COORD_Y( m_packet.Sprite8x8.n_coord ) + m_n_drawoffset_y + 7 );
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

	n_v = TEXTURE_V( m_packet.Sprite8x8.n_texture );
	n_y = COORD_Y( m_packet.Sprite8x8.n_coord ) + m_n_drawoffset_y;
	n_h = 8;

	while( n_h > 0 )
	{
		n_x = COORD_X( m_packet.Sprite8x8.n_coord ) + m_n_drawoffset_x;
		n_u = TEXTURE_U( m_packet.Sprite8x8.n_texture );

		n_distance = 8;
		if( n_distance > 0 && n_y >= (INT32)m_n_drawarea_y1 && n_y <= (INT32)m_n_drawarea_y2 )
		{
			if( ( (INT32)m_n_drawarea_x1 - n_x ) > 0 )
			{
				n_u += ( m_n_drawarea_x1 - n_x ) * n_du;
				n_distance -= ( m_n_drawarea_x1 - n_x );
				n_x = m_n_drawarea_x1;
			}
			TEXTUREFILL( FLATTEXTUREDRECTANGLEUPDATE, n_u, n_v );
		}
		n_v += n_dv;
		n_y++;
		n_h--;
	}
}

static void Sprite16x16( running_machine *machine )
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

#if defined( MAME_DEBUG )
	if( m_debug.n_skip == 13 )
	{
		return;
	}
	DebugMesh( COORD_X( m_packet.Sprite16x16.n_coord ) + m_n_drawoffset_x, COORD_Y( m_packet.Sprite16x16.n_coord ) + m_n_drawoffset_y );
	DebugMesh( COORD_X( m_packet.Sprite16x16.n_coord ) + m_n_drawoffset_x + 7, COORD_Y( m_packet.Sprite16x16.n_coord ) + m_n_drawoffset_y );
	DebugMesh( COORD_X( m_packet.Sprite16x16.n_coord ) + m_n_drawoffset_x, COORD_Y( m_packet.Sprite16x16.n_coord ) + m_n_drawoffset_y + 7 );
	DebugMesh( COORD_X( m_packet.Sprite16x16.n_coord ) + m_n_drawoffset_x + 7, COORD_Y( m_packet.Sprite16x16.n_coord ) + m_n_drawoffset_y + 7 );
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

	n_v = TEXTURE_V( m_packet.Sprite16x16.n_texture );
	n_y = COORD_Y( m_packet.Sprite16x16.n_coord ) + m_n_drawoffset_y;
	n_h = 16;

	while( n_h > 0 )
	{
		n_x = COORD_X( m_packet.Sprite16x16.n_coord ) + m_n_drawoffset_x;
		n_u = TEXTURE_U( m_packet.Sprite16x16.n_texture );

		n_distance = 16;
		if( n_distance > 0 && n_y >= (INT32)m_n_drawarea_y1 && n_y <= (INT32)m_n_drawarea_y2 )
		{
			if( ( (INT32)m_n_drawarea_x1 - n_x ) > 0 )
			{
				n_u += ( m_n_drawarea_x1 - n_x ) * n_du;
				n_distance -= ( m_n_drawarea_x1 - n_x );
				n_x = m_n_drawarea_x1;
			}
			TEXTUREFILL( FLATTEXTUREDRECTANGLEUPDATE, n_u, n_v );
		}
		n_v += n_dv;
		n_y++;
		n_h--;
	}
}

static void Dot( void )
{
	INT32 n_x;
	INT32 n_y;
	UINT32 n_r;
	UINT32 n_g;
	UINT32 n_b;
	UINT16 *p_vram;

#if defined( MAME_DEBUG )
	if( m_debug.n_skip == 14 )
	{
		return;
	}
	DebugMesh( COORD_X( m_packet.Dot.vertex.n_coord ) + m_n_drawoffset_x, COORD_Y( m_packet.Dot.vertex.n_coord ) + m_n_drawoffset_y );
	DebugMeshEnd();
#endif

	n_r = BGR_R( m_packet.Dot.n_bgr );
	n_g = BGR_G( m_packet.Dot.n_bgr );
	n_b = BGR_B( m_packet.Dot.n_bgr );
	n_x = COORD_X( m_packet.Dot.vertex.n_coord ) + m_n_drawoffset_x;
	n_y = COORD_Y( m_packet.Dot.vertex.n_coord ) + m_n_drawoffset_y;

	if( (INT16)n_x >= (INT32)m_n_drawarea_x1 &&
		(INT16)n_y >= (INT32)m_n_drawarea_y1 &&
		(INT16)n_x <= (INT32)m_n_drawarea_x2 &&
		(INT16)n_y <= (INT32)m_n_drawarea_y2 )
	{
		p_vram = m_p_p_vram[ n_y ] + n_x;
		WRITE_PIXEL(
			m_p_n_redshade[ MID_LEVEL | n_r ] |
			m_p_n_greenshade[ MID_LEVEL | n_g ] |
			m_p_n_blueshade[ MID_LEVEL | n_b ] );
	}
}

static void MoveImage( void )
{
	INT16 n_w;
	INT16 n_h;
	INT16 n_srcx;
	INT16 n_srcy;
	INT16 n_dsty;
	INT16 n_dstx;
	UINT16 *p_vram;

#if defined( MAME_DEBUG )
	if( m_debug.n_skip == 15 )
	{
		return;
	}
	DebugMesh( COORD_X( m_packet.MoveImage.vertex[ 1 ].n_coord ), COORD_Y( m_packet.MoveImage.vertex[ 1 ].n_coord ) );
	DebugMesh( COORD_X( m_packet.MoveImage.vertex[ 1 ].n_coord ) + SIZE_W( m_packet.MoveImage.n_size ), COORD_Y( m_packet.MoveImage.vertex[ 1 ].n_coord ) );
	DebugMesh( COORD_X( m_packet.MoveImage.vertex[ 1 ].n_coord ), COORD_Y( m_packet.MoveImage.vertex[ 1 ].n_coord ) + SIZE_H( m_packet.MoveImage.n_size ) );
	DebugMesh( COORD_X( m_packet.MoveImage.vertex[ 1 ].n_coord ) + SIZE_W( m_packet.MoveImage.n_size ), COORD_Y( m_packet.MoveImage.vertex[ 1 ].n_coord ) + SIZE_H( m_packet.MoveImage.n_size ) );
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
			p_vram = m_p_p_vram[ n_dsty & 1023 ] + ( n_dstx & 1023 );
			WRITE_PIXEL( *( m_p_p_vram[ n_srcy & 1023 ] + ( n_srcx & 1023 ) ) );
			n_srcx++;
			n_dstx++;
			n_w--;
		}
		n_srcy++;
		n_dsty++;
		n_h--;
	}
}

void psx_gpu_write( running_machine *machine, UINT32 *p_ram, INT32 n_size )
{
	while( n_size > 0 )
	{
		UINT32 data = *( p_ram );

		verboselog( machine, 2, "PSX Packet #%u %08x\n", m_n_gpu_buffer_offset, data );
		m_packet.n_entry[ m_n_gpu_buffer_offset ] = data;
		switch( m_packet.n_entry[ 0 ] >> 24 )
		{
		case 0x00:
			verboselog( machine, 1, "not handled: GPU Command 0x00: (%08x)\n", data );
			break;
		case 0x01:
			verboselog( machine, 1, "not handled: clear cache\n" );
			break;
		case 0x02:
			if( m_n_gpu_buffer_offset < 2 )
			{
				m_n_gpu_buffer_offset++;
			}
			else
			{
				verboselog( machine, 1, "%02x: frame buffer rectangle %u,%u %u,%u\n", m_packet.n_entry[ 0 ] >> 24,
					m_packet.n_entry[ 1 ] & 0xffff, m_packet.n_entry[ 1 ] >> 16, m_packet.n_entry[ 2 ] & 0xffff, m_packet.n_entry[ 2 ] >> 16 );
				FrameBufferRectangleDraw();
				m_n_gpu_buffer_offset = 0;
			}
			break;
		case 0x20:
		case 0x21:
		case 0x22:
		case 0x23:
			if( m_n_gpu_buffer_offset < 3 )
			{
				m_n_gpu_buffer_offset++;
			}
			else
			{
				verboselog( machine, 1, "%02x: monochrome 3 point polygon\n", m_packet.n_entry[ 0 ] >> 24 );
				FlatPolygon( machine, 3 );
				m_n_gpu_buffer_offset = 0;
			}
			break;
		case 0x24:
		case 0x25:
		case 0x26:
		case 0x27:
			if( m_n_gpu_buffer_offset < 6 )
			{
				m_n_gpu_buffer_offset++;
			}
			else
			{
				verboselog( machine, 1, "%02x: textured 3 point polygon\n", m_packet.n_entry[ 0 ] >> 24 );
				FlatTexturedPolygon( machine, 3 );
				m_n_gpu_buffer_offset = 0;
			}
			break;
		case 0x28:
		case 0x29:
		case 0x2a:
		case 0x2b:
			if( m_n_gpu_buffer_offset < 4 )
			{
				m_n_gpu_buffer_offset++;
			}
			else
			{
				verboselog( machine, 1, "%02x: monochrome 4 point polygon\n", m_packet.n_entry[ 0 ] >> 24 );
				FlatPolygon( machine, 4 );
				m_n_gpu_buffer_offset = 0;
			}
			break;
		case 0x2c:
		case 0x2d:
		case 0x2e:
		case 0x2f:
			if( m_n_gpu_buffer_offset < 8 )
			{
				m_n_gpu_buffer_offset++;
			}
			else
			{
				verboselog( machine, 1, "%02x: textured 4 point polygon\n", m_packet.n_entry[ 0 ] >> 24 );
				FlatTexturedPolygon( machine, 4 );
				m_n_gpu_buffer_offset = 0;
			}
			break;
		case 0x30:
		case 0x31:
		case 0x32:
		case 0x33:
			if( m_n_gpu_buffer_offset < 5 )
			{
				m_n_gpu_buffer_offset++;
			}
			else
			{
				verboselog( machine, 1, "%02x: gouraud 3 point polygon\n", m_packet.n_entry[ 0 ] >> 24 );
				GouraudPolygon( machine, 3 );
				m_n_gpu_buffer_offset = 0;
			}
			break;
		case 0x34:
		case 0x35:
		case 0x36:
		case 0x37:
			if( m_n_gpu_buffer_offset < 8 )
			{
				m_n_gpu_buffer_offset++;
			}
			else
			{
				verboselog( machine, 1, "%02x: gouraud textured 3 point polygon\n", m_packet.n_entry[ 0 ] >> 24 );
				GouraudTexturedPolygon( machine, 3 );
				m_n_gpu_buffer_offset = 0;
			}
			break;
		case 0x38:
		case 0x39:
		case 0x3a:
		case 0x3b:
			if( m_n_gpu_buffer_offset < 7 )
			{
				m_n_gpu_buffer_offset++;
			}
			else
			{
				verboselog( machine, 1, "%02x: gouraud 4 point polygon\n", m_packet.n_entry[ 0 ] >> 24 );
				GouraudPolygon( machine, 4 );
				m_n_gpu_buffer_offset = 0;
			}
			break;
		case 0x3c:
		case 0x3d:
		case 0x3e:
		case 0x3f:
			if( m_n_gpu_buffer_offset < 11 )
			{
				m_n_gpu_buffer_offset++;
			}
			else
			{
				verboselog( machine, 1, "%02x: gouraud textured 4 point polygon\n", m_packet.n_entry[ 0 ] >> 24 );
				GouraudTexturedPolygon( machine, 4 );
				m_n_gpu_buffer_offset = 0;
			}
			break;
		case 0x40:
		case 0x41:
		case 0x42:
			if( m_n_gpu_buffer_offset < 2 )
			{
				m_n_gpu_buffer_offset++;
			}
			else
			{
				verboselog( machine, 1, "%02x: monochrome line\n", m_packet.n_entry[ 0 ] >> 24 );
				MonochromeLine();
				m_n_gpu_buffer_offset = 0;
			}
			break;
		case 0x48:
		case 0x4a:
		case 0x4c:
		case 0x4e:
			if( m_n_gpu_buffer_offset < 3 )
			{
				m_n_gpu_buffer_offset++;
			}
			else
			{
				verboselog( machine, 1, "%02x: monochrome polyline\n", m_packet.n_entry[ 0 ] >> 24 );
				MonochromeLine();
				if( ( m_packet.n_entry[ 3 ] & 0xf000f000 ) != 0x50005000 )
				{
					m_packet.n_entry[ 1 ] = m_packet.n_entry[ 2 ];
					m_packet.n_entry[ 2 ] = m_packet.n_entry[ 3 ];
					m_n_gpu_buffer_offset = 3;
				}
				else
				{
					m_n_gpu_buffer_offset = 0;
				}
			}
			break;
		case 0x50:
		case 0x52:

			if( m_n_gpu_buffer_offset < 3 )
			{
				m_n_gpu_buffer_offset++;
			}
			else
			{
				verboselog( machine, 1, "%02x: gouraud line\n", m_packet.n_entry[ 0 ] >> 24 );
				GouraudLine();
				m_n_gpu_buffer_offset = 0;
			}
			break;
		case 0x58:
		case 0x5a:
		case 0x5c:
		case 0x5e:
			if( m_n_gpu_buffer_offset < 5 &&
				( m_n_gpu_buffer_offset != 4 || ( m_packet.n_entry[ 4 ] & 0xf000f000 ) != 0x50005000 ) )
			{
				m_n_gpu_buffer_offset++;
			}
			else
			{
				verboselog( machine, 1, "%02x: gouraud polyline\n", m_packet.n_entry[ 0 ] >> 24 );
				GouraudLine();
				if( ( m_packet.n_entry[ 4 ] & 0xf000f000 ) != 0x50005000 )
				{
					m_packet.n_entry[ 0 ] = ( m_packet.n_entry[ 0 ] & 0xff000000 ) | ( m_packet.n_entry[ 2 ] & 0x00ffffff );
					m_packet.n_entry[ 1 ] = m_packet.n_entry[ 3 ];
					m_packet.n_entry[ 2 ] = m_packet.n_entry[ 4 ];
					m_packet.n_entry[ 3 ] = m_packet.n_entry[ 5 ];
					m_n_gpu_buffer_offset = 4;
				}
				else
				{
					m_n_gpu_buffer_offset = 0;
				}
			}
			break;
		case 0x60:
		case 0x61:
		case 0x62:
		case 0x63:
			if( m_n_gpu_buffer_offset < 2 )
			{
				m_n_gpu_buffer_offset++;
			}
			else
			{
				verboselog( machine, 1, "%02x: rectangle %d,%d %d,%d\n",
					m_packet.n_entry[ 0 ] >> 24,
					(INT16)( m_packet.n_entry[ 1 ] & 0xffff ), (INT16)( m_packet.n_entry[ 1 ] >> 16 ),
					(INT16)( m_packet.n_entry[ 2 ] & 0xffff ), (INT16)( m_packet.n_entry[ 2 ] >> 16 ) );
				FlatRectangle(machine);
				m_n_gpu_buffer_offset = 0;
			}
			break;
		case 0x64:
		case 0x65:
		case 0x66:
		case 0x67:
			if( m_n_gpu_buffer_offset < 3 )
			{
				m_n_gpu_buffer_offset++;
			}
			else
			{
				verboselog( machine, 1, "%02x: sprite %d,%d %u,%u %08x, %08x\n",
					m_packet.n_entry[ 0 ] >> 24,
					(INT16)( m_packet.n_entry[ 1 ] & 0xffff ), (INT16)( m_packet.n_entry[ 1 ] >> 16 ),
					m_packet.n_entry[ 3 ] & 0xffff, m_packet.n_entry[ 3 ] >> 16,
					m_packet.n_entry[ 0 ], m_packet.n_entry[ 2 ] );
				FlatTexturedRectangle(machine);
				m_n_gpu_buffer_offset = 0;
			}
			break;
		case 0x68:
		case 0x6a:
			if( m_n_gpu_buffer_offset < 1 )
			{
				m_n_gpu_buffer_offset++;
			}
			else
			{
				verboselog( machine, 1, "%02x: dot %d,%d %08x\n",
					m_packet.n_entry[ 0 ] >> 24,
					(INT16)( m_packet.n_entry[ 1 ] & 0xffff ), (INT16)( m_packet.n_entry[ 1 ] >> 16 ),
					m_packet.n_entry[ 0 ] & 0xffffff );
				Dot();
				m_n_gpu_buffer_offset = 0;
			}
			break;
		case 0x70:
		case 0x71:
			/* 8*8 rectangle */
			if( m_n_gpu_buffer_offset < 1 )
			{
				m_n_gpu_buffer_offset++;
			}
			else
			{
				verboselog( machine, 1, "%02x: 16x16 rectangle %08x %08x\n", m_packet.n_entry[ 0 ] >> 24,
					m_packet.n_entry[ 0 ], m_packet.n_entry[ 1 ] );
				FlatRectangle8x8(machine);
				m_n_gpu_buffer_offset = 0;
			}
			break;
		case 0x74:
		case 0x75:
		case 0x76:
		case 0x77:
			if( m_n_gpu_buffer_offset < 2 )
			{
				m_n_gpu_buffer_offset++;
			}
			else
			{
				verboselog( machine, 1, "%02x: 8x8 sprite %08x %08x %08x\n", m_packet.n_entry[ 0 ] >> 24,
					m_packet.n_entry[ 0 ], m_packet.n_entry[ 1 ], m_packet.n_entry[ 2 ] );
				Sprite8x8(machine);
				m_n_gpu_buffer_offset = 0;
			}
			break;
		case 0x78:
		case 0x79:
			/* 16*16 rectangle */
			if( m_n_gpu_buffer_offset < 1 )
			{
				m_n_gpu_buffer_offset++;
			}
			else
			{
				verboselog( machine, 1, "%02x: 16x16 rectangle %08x %08x\n", m_packet.n_entry[ 0 ] >> 24,
					m_packet.n_entry[ 0 ], m_packet.n_entry[ 1 ] );
				FlatRectangle16x16(machine);
				m_n_gpu_buffer_offset = 0;
			}
			break;
		case 0x7c:
		case 0x7d:
		case 0x7e:
		case 0x7f:
			if( m_n_gpu_buffer_offset < 2 )
			{
				m_n_gpu_buffer_offset++;
			}
			else
			{
				verboselog( machine, 1, "%02x: 16x16 sprite %08x %08x %08x\n", m_packet.n_entry[ 0 ] >> 24,
					m_packet.n_entry[ 0 ], m_packet.n_entry[ 1 ], m_packet.n_entry[ 2 ] );
				Sprite16x16(machine);
				m_n_gpu_buffer_offset = 0;
			}
			break;
		case 0x80:
			if( m_n_gpu_buffer_offset < 3 )
			{
				m_n_gpu_buffer_offset++;
			}
			else
			{
				verboselog( machine, 1, "move image in frame buffer %08x %08x %08x %08x\n", m_packet.n_entry[ 0 ], m_packet.n_entry[ 1 ], m_packet.n_entry[ 2 ], m_packet.n_entry[ 3 ] );
				MoveImage();
				m_n_gpu_buffer_offset = 0;
			}
			break;
		case 0xa0:
			if( m_n_gpu_buffer_offset < 3 )
			{
				m_n_gpu_buffer_offset++;
			}
			else
			{
				UINT32 n_pixel;
				for( n_pixel = 0; n_pixel < 2; n_pixel++ )
				{
					UINT16 *p_vram;

					verboselog( machine, 2, "send image to framebuffer ( pixel %u,%u = %u )\n",
						( m_n_vramx + m_packet.n_entry[ 1 ] ) & 1023,
						( m_n_vramy + ( m_packet.n_entry[ 1 ] >> 16 ) ) & 1023,
						data & 0xffff );

					p_vram = m_p_p_vram[ ( m_n_vramy + ( m_packet.n_entry[ 1 ] >> 16 ) ) & 1023 ] + ( ( m_n_vramx + m_packet.n_entry[ 1 ] ) & 1023 );
					WRITE_PIXEL( data & 0xffff );
					m_n_vramx++;
					if( m_n_vramx >= ( m_packet.n_entry[ 2 ] & 0xffff ) )
					{
						m_n_vramx = 0;
						m_n_vramy++;
						if( m_n_vramy >= ( m_packet.n_entry[ 2 ] >> 16 ) )
						{
							verboselog( machine, 1, "%02x: send image to framebuffer %u,%u %u,%u\n", m_packet.n_entry[ 0 ] >> 24,
								m_packet.n_entry[ 1 ] & 0xffff, ( m_packet.n_entry[ 1 ] >> 16 ),
								m_packet.n_entry[ 2 ] & 0xffff, ( m_packet.n_entry[ 2 ] >> 16 ) );
							m_n_gpu_buffer_offset = 0;
							m_n_vramx = 0;
							m_n_vramy = 0;
							break;
						}
					}
					data >>= 16;
				}
			}
			break;
		case 0xc0:
			if( m_n_gpu_buffer_offset < 2 )
			{
				m_n_gpu_buffer_offset++;
			}
			else
			{
				verboselog( machine, 1, "%02x: copy image from frame buffer\n", m_packet.n_entry[ 0 ] >> 24 );
				m_n_gpustatus |= ( 1L << 0x1b );
			}
			break;
		case 0xe1:
			verboselog( machine, 1, "%02x: draw mode %06x\n", m_packet.n_entry[ 0 ] >> 24,
				m_packet.n_entry[ 0 ] & 0xffffff );
			decode_tpage( machine, &psxgpu, m_packet.n_entry[ 0 ] & 0xffffff );
			break;
		case 0xe2:
			m_n_twy = ( ( ( m_packet.n_entry[ 0 ] >> 15 ) & 0x1f ) << 3 );
			m_n_twx = ( ( ( m_packet.n_entry[ 0 ] >> 10 ) & 0x1f ) << 3 );
			m_n_twh = 255 - ( ( ( m_packet.n_entry[ 0 ] >> 5 ) & 0x1f ) << 3 );
			m_n_tww = 255 - ( ( m_packet.n_entry[ 0 ] & 0x1f ) << 3 );
			verboselog( machine, 1, "%02x: texture window %u,%u %u,%u\n", m_packet.n_entry[ 0 ] >> 24,
				m_n_twx, m_n_twy, m_n_tww, m_n_twh );
			break;
		case 0xe3:
			m_n_drawarea_x1 = m_packet.n_entry[ 0 ] & 1023;
			if( m_n_gputype == 2 )
			{
				m_n_drawarea_y1 = ( m_packet.n_entry[ 0 ] >> 10 ) & 1023;
			}
			else
			{
				m_n_drawarea_y1 = ( m_packet.n_entry[ 0 ] >> 12 ) & 1023;
			}
			verboselog( machine, 1, "%02x: drawing area top left %d,%d\n", m_packet.n_entry[ 0 ] >> 24,
				m_n_drawarea_x1, m_n_drawarea_y1 );
			break;
		case 0xe4:
			m_n_drawarea_x2 = m_packet.n_entry[ 0 ] & 1023;
			if( m_n_gputype == 2 )
			{
				m_n_drawarea_y2 = ( m_packet.n_entry[ 0 ] >> 10 ) & 1023;
			}
			else
			{
				m_n_drawarea_y2 = ( m_packet.n_entry[ 0 ] >> 12 ) & 1023;
			}
			verboselog( machine, 1, "%02x: drawing area bottom right %d,%d\n", m_packet.n_entry[ 0 ] >> 24,
				m_n_drawarea_x2, m_n_drawarea_y2 );
			break;
		case 0xe5:
			m_n_drawoffset_x = SINT11( m_packet.n_entry[ 0 ] & 2047 );
			if( m_n_gputype == 2 )
			{
				m_n_drawoffset_y = SINT11( ( m_packet.n_entry[ 0 ] >> 11 ) & 2047 );
			}
			else
			{
				m_n_drawoffset_y = SINT11( ( m_packet.n_entry[ 0 ] >> 12 ) & 2047 );
			}
			verboselog( machine, 1, "%02x: drawing offset %d,%d\n", m_packet.n_entry[ 0 ] >> 24,
				m_n_drawoffset_x, m_n_drawoffset_y );
			break;
		case 0xe6:
			m_n_gpustatus &= ~( 3L << 0xb );
			m_n_gpustatus |= ( data & 0x03 ) << 0xb;
			if( ( m_packet.n_entry[ 0 ] & 3 ) != 0 )
			{
				verboselog( machine, 1, "not handled: mask setting %d\n", m_packet.n_entry[ 0 ] & 3 );
			}
			else
			{
				verboselog( machine, 1, "mask setting %d\n", m_packet.n_entry[ 0 ] & 3 );
			}
			break;
		default:
#if defined( MAME_DEBUG )
			popmessage( "unknown GPU packet %08x", m_packet.n_entry[ 0 ] );
#endif
			verboselog( machine, 0, "unknown GPU packet %08x (%08x)\n", m_packet.n_entry[ 0 ], data );
#if ( STOP_ON_ERROR )
			m_n_gpu_buffer_offset = 1;
#endif
			break;
		}
		p_ram++;
		n_size--;
	}
}

WRITE32_HANDLER( psx_gpu_w )
{
	running_machine *machine = space->machine;

	switch( offset )
	{
	case 0x00:
		psx_gpu_write( machine, &data, 1 );
		break;
	case 0x01:
		switch( data >> 24 )
		{
		case 0x00:
			verboselog( machine, 1, "reset gpu\n" );
			m_n_gpu_buffer_offset = 0;
			m_n_gpustatus = 0x14802000;
			m_n_drawarea_x1 = 0;
			m_n_drawarea_y1 = 0;
			m_n_drawarea_x2 = 1023;
			m_n_drawarea_y2 = 1023;
			m_n_drawoffset_x = 0;
			m_n_drawoffset_y = 0;
			m_n_displaystartx = 0;
			m_n_displaystarty = 0;
			m_n_horiz_disstart = 0x260;
			m_n_horiz_disend = 0xc60;
			m_n_vert_disstart = 0x010;
			m_n_vert_disend = 0x100;
			m_n_vramx = 0;
			m_n_vramy = 0;
			m_n_twx = 0;
			m_n_twy = 0;
			m_n_twh = 255;
			m_n_tww = 255;
			updatevisiblearea(space->machine, NULL);
			break;
		case 0x01:
			verboselog( machine, 1, "not handled: reset command buffer\n" );
			m_n_gpu_buffer_offset = 0;
			break;
		case 0x02:
			verboselog( machine, 1, "not handled: reset irq\n" );
			break;
		case 0x03:
			m_n_gpustatus &= ~( 1L << 0x17 );
			m_n_gpustatus |= ( data & 0x01 ) << 0x17;
			break;
		case 0x04:
			verboselog( machine, 1, "dma setup %d\n", data & 3 );
			m_n_gpustatus &= ~( 3L << 0x1d );
			m_n_gpustatus |= ( data & 0x03 ) << 0x1d;
			m_n_gpustatus &= ~( 1L << 0x19 );
			if( ( data & 3 ) == 1 || ( data & 3 ) == 2 )
			{
				m_n_gpustatus |= ( 1L << 0x19 );
			}
			break;
		case 0x05:
			m_n_displaystartx = data & 1023;
			if( m_n_gputype == 2 )
			{
				m_n_displaystarty = ( data >> 10 ) & 1023;
			}
			else
			{
				m_n_displaystarty = ( data >> 12 ) & 1023;
			}
			verboselog( machine, 1, "start of display area %d %d\n", m_n_displaystartx, m_n_displaystarty );
			break;
		case 0x06:
			m_n_horiz_disstart = data & 4095;
			m_n_horiz_disend = ( data >> 12 ) & 4095;
			verboselog( machine, 1, "horizontal display range %d %d\n", m_n_horiz_disstart, m_n_horiz_disend );
			break;
		case 0x07:
			m_n_vert_disstart = data & 1023;
			m_n_vert_disend = ( data >> 10 ) & 2047;
			verboselog( machine, 1, "vertical display range %d %d\n", m_n_vert_disstart, m_n_vert_disend );
			break;
		case 0x08:
			verboselog( machine, 1, "display mode %02x\n", data & 0xff );
			m_n_gpustatus &= ~( 127L << 0x10 );
			m_n_gpustatus |= ( data & 0x3f ) << 0x11; /* width 0 + height + videmode + isrgb24 + isinter */
			m_n_gpustatus |= ( ( data & 0x40 ) >> 0x06 ) << 0x10; /* width 1 */
			if( m_n_gputype == 1 )
			{
				m_b_reverseflag = ( data >> 7 ) & 1;
			}
			updatevisiblearea(space->machine, NULL);
			break;
		case 0x09:
			verboselog( machine, 1, "not handled: GPU Control 0x09: %08x\n", data );
			break;
		case 0x0d:
			verboselog( machine, 1, "reset lightgun coordinates %08x\n", data );
			m_n_lightgun_x = 0;
			m_n_lightgun_y = 0;
			break;
		case 0x10:
			switch( data & 0xff )
			{
			case 0x03:
				if( m_n_gputype == 2 )
				{
					m_n_gpuinfo = m_n_drawarea_x1 | ( m_n_drawarea_y1 << 10 );
				}
				else
				{
					m_n_gpuinfo = m_n_drawarea_x1 | ( m_n_drawarea_y1 << 12 );
				}
				verboselog( machine, 1, "GPU Info - Draw area top left %08x\n", m_n_gpuinfo );
				break;
			case 0x04:
				if( m_n_gputype == 2 )
				{
					m_n_gpuinfo = m_n_drawarea_x2 | ( m_n_drawarea_y2 << 10 );
				}
				else
				{
					m_n_gpuinfo = m_n_drawarea_x2 | ( m_n_drawarea_y2 << 12 );
				}
				verboselog( machine, 1, "GPU Info - Draw area bottom right %08x\n", m_n_gpuinfo );
				break;
			case 0x05:
				if( m_n_gputype == 2 )
				{
					m_n_gpuinfo = ( m_n_drawoffset_x & 2047 ) | ( ( m_n_drawoffset_y & 2047 ) << 11 );
				}
				else
				{
					m_n_gpuinfo = ( m_n_drawoffset_x & 2047 ) | ( ( m_n_drawoffset_y & 2047 ) << 12 );
				}
				verboselog( machine, 1, "GPU Info - Draw offset %08x\n", m_n_gpuinfo );
				break;
			case 0x07:
				m_n_gpuinfo = m_n_gputype;
				verboselog( machine, 1, "GPU Info - GPU Type %08x\n", m_n_gpuinfo );
				break;
			case 0x08:
				m_n_gpuinfo = m_n_lightgun_x | ( m_n_lightgun_y << 16 );
				verboselog( machine, 1, "GPU Info - lightgun coordinates %08x\n", m_n_gpuinfo );
				break;
			default:
				verboselog( machine, 0, "GPU Info - unknown request (%08x)\n", data );
				m_n_gpuinfo = 0;
				break;
			}
			break;
		case 0x20:
			verboselog( machine, 1, "not handled: GPU Control 0x20: %08x\n", data );
			break;
		default:
#if defined( MAME_DEBUG )
			popmessage( "unknown GPU command %08x", data );
#endif
			verboselog( machine, 0, "gpu_w( %08x ) unknown GPU command\n", data );
			break;
		}
		break;
	default:
		verboselog( machine, 0, "gpu_w( %08x, %08x, %08x ) unknown register\n", offset, data, mem_mask );
		break;
	}
}


void psx_gpu_read( running_machine *machine, UINT32 *p_ram, INT32 n_size )
{
	while( n_size > 0 )
	{
		if( ( m_n_gpustatus & ( 1L << 0x1b ) ) != 0 )
		{
			UINT32 n_pixel;
			PAIR data;

			verboselog( machine, 2, "copy image from frame buffer ( %d, %d )\n", m_n_vramx, m_n_vramy );
			data.d = 0;
			for( n_pixel = 0; n_pixel < 2; n_pixel++ )
			{
				data.w.l = data.w.h;
				data.w.h = *( m_p_p_vram[ m_n_vramy + ( m_packet.n_entry[ 1 ] >> 16 ) ] + m_n_vramx + ( m_packet.n_entry[ 1 ] & 0xffff ) );
				m_n_vramx++;
				if( m_n_vramx >= ( m_packet.n_entry[ 2 ] & 0xffff ) )
				{
					m_n_vramx = 0;
					m_n_vramy++;
					if( m_n_vramy >= ( m_packet.n_entry[ 2 ] >> 16 ) )
					{
						verboselog( machine, 1, "copy image from frame buffer end\n" );
						m_n_gpustatus &= ~( 1L << 0x1b );
						m_n_gpu_buffer_offset = 0;
						m_n_vramx = 0;
						m_n_vramy = 0;
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
			verboselog( machine, 2, "read GPU info (%08x)\n", m_n_gpuinfo );
			*( p_ram ) = m_n_gpuinfo;
		}
		p_ram++;
		n_size--;
	}
}

READ32_HANDLER( psx_gpu_r )
{
	UINT32 data;

	switch( offset )
	{
	case 0x00:
		psx_gpu_read( space->machine, &data, 1 );
		break;
	case 0x01:
		verboselog( space->machine, 1, "read GPU status (%08x)\n", m_n_gpustatus );
		data = m_n_gpustatus;
		break;
	default:
		verboselog( space->machine, 0, "gpu_r( %08x, %08x ) unknown register\n", offset, mem_mask );
		data = 0;
		break;
	}
	return data;
}

INTERRUPT_GEN( psx_vblank )
{
#if defined( MAME_DEBUG )
	DebugCheckKeys(device->machine);
#endif

	m_n_gpustatus ^= ( 1L << 31 );

	if(need_sianniv_vblank_hack)
	{
		UINT32 pc = cpu_get_pc(device);
		if((pc >= 0x80010018 && pc <= 0x80010028) || pc == 0x8002a4f0)
			return;
	}

	psx_irq_set( device->machine, 0x0001 );
}

void psx_gpu_reset( running_machine *machine )
{
	const address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);

	psx_gpu_w(space, 1, 0, 0xffffffff );
}

void psx_lightgun_set( int n_x, int n_y )
{
	m_n_lightgun_x = n_x;
	m_n_lightgun_y = n_y;
}
