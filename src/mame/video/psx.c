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

#include "emu.h"
#include "includes/psx.h"

#define STOP_ON_ERROR ( 0 )

#define VERBOSE_LEVEL ( 0 )

#define MAX_LEVEL ( 32 )
#define MID_LEVEL ( ( MAX_LEVEL / 2 ) << 8 )
#define MAX_SHADE ( 0x100 )
#define MID_SHADE ( 0x80 )

#define DEBUG_COORDS ( 10 )
#define DEBUG_MAX ( 512 )

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

union PACKET
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
};

typedef struct _psx_gpu_debug psx_gpu_debug;
struct _psx_gpu_debug
{
	bitmap_t *mesh;
	int b_clear;
	int b_mesh;
	int n_skip;
	int b_texture;
	int n_interleave;
	int n_coord;
	int n_coordx[ DEBUG_COORDS ];
	int n_coordy[ DEBUG_COORDS ];
};

struct _psx_gpu
{
	running_machine *machine;
	INT32 n_tx;
	INT32 n_ty;
	INT32 n_abr;
	INT32 n_tp;
	INT32 n_ix;
	INT32 n_iy;
	INT32 n_ti;

	UINT16 *p_vram;
	UINT32 n_vram_size;
	UINT32 n_vramx;
	UINT32 n_vramy;
	UINT32 n_twy;
	UINT32 n_twx;
	UINT32 n_twh;
	UINT32 n_tww;
	UINT32 n_drawarea_x1;
	UINT32 n_drawarea_y1;
	UINT32 n_drawarea_x2;
	UINT32 n_drawarea_y2;
	UINT32 n_horiz_disstart;
	UINT32 n_horiz_disend;
	UINT32 n_vert_disstart;
	UINT32 n_vert_disend;
	UINT32 b_reverseflag;
	INT32 n_drawoffset_x;
	INT32 n_drawoffset_y;
	UINT32 n_displaystartx;
	UINT32 n_displaystarty;
	int n_gputype;
	UINT32 n_gpustatus;
	UINT32 n_gpuinfo;
	UINT32 n_gpu_buffer_offset;
	UINT32 n_lightgun_x;
	UINT32 n_lightgun_y;
	UINT32 n_screenwidth;
	UINT32 n_screenheight;

	int b_need_sianniv_vblank_hack;
	PACKET m_packet;

	psx_gpu_debug m_debug;

	UINT16 *p_p_vram[ 1024 ];

	UINT16 p_n_redshade[ MAX_LEVEL * MAX_SHADE ];
	UINT16 p_n_greenshade[ MAX_LEVEL * MAX_SHADE ];
	UINT16 p_n_blueshade[ MAX_LEVEL * MAX_SHADE ];
	UINT16 p_n_redlevel[ 0x10000 ];
	UINT16 p_n_greenlevel[ 0x10000 ];
	UINT16 p_n_bluelevel[ 0x10000 ];

	UINT16 p_n_f025[ MAX_LEVEL * MAX_SHADE ];
	UINT16 p_n_f05[ MAX_LEVEL * MAX_SHADE ];
	UINT16 p_n_f1[ MAX_LEVEL * MAX_SHADE ];
	UINT16 p_n_redb05[ 0x10000 ];
	UINT16 p_n_greenb05[ 0x10000 ];
	UINT16 p_n_blueb05[ 0x10000 ];
	UINT16 p_n_redb1[ 0x10000 ];
	UINT16 p_n_greenb1[ 0x10000 ];
	UINT16 p_n_blueb1[ 0x10000 ];
	UINT16 p_n_redaddtrans[ MAX_LEVEL * MAX_LEVEL ];
	UINT16 p_n_greenaddtrans[ MAX_LEVEL * MAX_LEVEL ];
	UINT16 p_n_blueaddtrans[ MAX_LEVEL * MAX_LEVEL ];
	UINT16 p_n_redsubtrans[ MAX_LEVEL * MAX_LEVEL ];
	UINT16 p_n_greensubtrans[ MAX_LEVEL * MAX_LEVEL ];
	UINT16 p_n_bluesubtrans[ MAX_LEVEL * MAX_LEVEL ];

	UINT16 p_n_g0r0[ 0x10000 ];
	UINT16 p_n_b0[ 0x10000 ];
	UINT16 p_n_r1[ 0x10000 ];
	UINT16 p_n_b1g1[ 0x10000 ];
};

static const UINT16 m_p_n_nextpointlist4[] = { 1, 3, 0, 2 };
static const UINT16 m_p_n_prevpointlist4[] = { 2, 0, 3, 1 };
static const UINT16 m_p_n_nextpointlist3[] = { 1, 2, 0 };
static const UINT16 m_p_n_prevpointlist3[] = { 2, 0, 1 };

#define SINT11( x ) ( ( (INT32)( x ) << 21 ) >> 21 )

#define ADJUST_COORD( a ) \
	a.w.l = COORD_X( a ) + p_psxgpu->n_drawoffset_x; \
	a.w.h = COORD_Y( a ) + p_psxgpu->n_drawoffset_y;

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

INLINE void ATTR_PRINTF(3,4) verboselog( psx_gpu *p_psxgpu, int n_level, const char *s_fmt, ... )
{
	if( VERBOSE_LEVEL >= n_level )
	{
		va_list v;
		char buf[ 32768 ];
		va_start( v, s_fmt );
		vsprintf( buf, s_fmt, v );
		va_end( v );
		logerror( "%s: %s", cpuexec_describe_context(p_psxgpu->machine), buf );
	}
}

PALETTE_INIT( psx )
{
	UINT32 n_colour;

	for( n_colour = 0; n_colour < 0x10000; n_colour++ )
	{
		palette_set_color_rgb( machine, n_colour, pal5bit(n_colour >> 0), pal5bit(n_colour >> 5), pal5bit(n_colour >> 10) );
	}
}

#if defined( MAME_DEBUG )

static void DebugMeshInit( psx_gpu *p_psxgpu )
{
	screen_device *screen = p_psxgpu->machine->primary_screen;
	int width = screen->width();
	int height = screen->height();

	p_psxgpu->m_debug.b_mesh = 0;
	p_psxgpu->m_debug.b_texture = 0;
	p_psxgpu->m_debug.n_interleave = -1;
	p_psxgpu->m_debug.b_clear = 1;
	p_psxgpu->m_debug.n_coord = 0;
	p_psxgpu->m_debug.n_skip = 0;
	p_psxgpu->m_debug.mesh = auto_bitmap_alloc( p_psxgpu->machine, width, height, BITMAP_FORMAT_INDEXED16 );
}

static void DebugMesh( psx_gpu *p_psxgpu, int n_coordx, int n_coordy )
{
	int n_coord;
	int n_colour;
	screen_device *screen = p_psxgpu->machine->primary_screen;
	int width = screen->width();
	int height = screen->height();

	if( p_psxgpu->m_debug.b_clear )
	{
		bitmap_fill( p_psxgpu->m_debug.mesh, NULL , 0x0000);
		p_psxgpu->m_debug.b_clear = 0;
	}

	if( p_psxgpu->m_debug.n_coord < DEBUG_COORDS )
	{
		n_coordx += p_psxgpu->n_displaystartx;
		n_coordy += p_psxgpu->n_displaystarty;

		n_coordx *= 511;
		n_coordx /= DEBUG_MAX - 1;
		n_coordx += 256;
		n_coordy *= 511;
		n_coordy /= DEBUG_MAX - 1;
		n_coordy += 256;

		p_psxgpu->m_debug.n_coordx[ p_psxgpu->m_debug.n_coord ] = n_coordx;
		p_psxgpu->m_debug.n_coordy[ p_psxgpu->m_debug.n_coord ] = n_coordy;
		p_psxgpu->m_debug.n_coord++;
	}

	n_colour = 0x1f;
	for( n_coord = 0; n_coord < p_psxgpu->m_debug.n_coord; n_coord++ )
	{
		if( n_coordx != p_psxgpu->m_debug.n_coordx[ n_coord ] ||
			n_coordy != p_psxgpu->m_debug.n_coordy[ n_coord ] )
		{
			break;
		}
	}
	if( n_coord == p_psxgpu->m_debug.n_coord && p_psxgpu->m_debug.n_coord > 1 )
	{
		n_colour = 0xffff;
	}
	for( n_coord = 0; n_coord < p_psxgpu->m_debug.n_coord; n_coord++ )
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

		n_xstart = p_psxgpu->m_debug.n_coordx[ n_coord ];
		n_xend = n_coordx;
		if( n_xend > n_xstart )
		{
			n_xlen = n_xend - n_xstart;
		}
		else
		{
			n_xlen = n_xstart - n_xend;
		}

		n_ystart = p_psxgpu->m_debug.n_coordy[ n_coord ];
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
				if( *BITMAP_ADDR16(p_psxgpu->m_debug.mesh, n_y.w.h, n_x.w.h) != 0xffff )
					*BITMAP_ADDR16(p_psxgpu->m_debug.mesh, n_y.w.h, n_x.w.h) = n_colour;
			}
			n_x.d += n_dx;
			n_y.d += n_dy;
			n_len--;
		}
	}
}

static void DebugMeshEnd( psx_gpu *p_psxgpu )
{
	p_psxgpu->m_debug.n_coord = 0;
}

static void DebugCheckKeys( psx_gpu *p_psxgpu )
{
	if( input_code_pressed_once( p_psxgpu->machine, KEYCODE_M ) )
		p_psxgpu->m_debug.b_mesh = !p_psxgpu->m_debug.b_mesh;

	if( input_code_pressed_once( p_psxgpu->machine, KEYCODE_V ) )
		p_psxgpu->m_debug.b_texture = !p_psxgpu->m_debug.b_texture;

	if( p_psxgpu->m_debug.b_mesh || p_psxgpu->m_debug.b_texture )
	{
		screen_device *screen = p_psxgpu->machine->primary_screen;
		int width = screen->width();
		int height = screen->height();
		p_psxgpu->machine->primary_screen->set_visible_area( 0, width - 1, 0, height - 1 );
	}
	else
		p_psxgpu->machine->primary_screen->set_visible_area( 0, p_psxgpu->n_screenwidth - 1, 0, p_psxgpu->n_screenheight - 1 );

	if( input_code_pressed_once( p_psxgpu->machine, KEYCODE_I ) )
	{
		if( p_psxgpu->m_debug.b_texture )
		{
			p_psxgpu->m_debug.n_interleave++;

			if( p_psxgpu->m_debug.n_interleave == 2 )
				p_psxgpu->m_debug.n_interleave = -1;

			if( p_psxgpu->m_debug.n_interleave == -1 )
				popmessage( "interleave off" );
			else if( p_psxgpu->m_debug.n_interleave == 0 )
				popmessage( "4 bit interleave" );
			else if( p_psxgpu->m_debug.n_interleave == 1 )
				popmessage( "8 bit interleave" );
		}
		else
		{
			p_psxgpu->m_debug.n_skip++;

			if( p_psxgpu->m_debug.n_skip > 15 )
				p_psxgpu->m_debug.n_skip = 0;

			popmessage( "debug skip %d", p_psxgpu->m_debug.n_skip );
		}
	}

#if 0
	if( input_code_pressed_once( p_psxgpu->machine, KEYCODE_D ) )
	{
		FILE *f;
		int n_x;
		f = fopen( "dump.txt", "w" );
		for( n_y = 256; n_y < 512; n_y++ )
			for( n_x = 640; n_x < 1024; n_x++ )
				fprintf( f, "%04u,%04u = %04x\n", n_y, n_x, p_psxgpu->p_p_vram[ n_y ][ n_x ] );
		fclose( f );
	}
	if( input_code_pressed_once( p_psxgpu->machine, KEYCODE_S ) )
	{
		FILE *f;
		popmessage( "saving..." );
		f = fopen( "VRAM.BIN", "wb" );
		for( n_y = 0; n_y < 1024; n_y++ )
			fwrite( p_psxgpu->p_p_vram[ n_y ], 1024 * 2, 1, f );
		fclose( f );
	}
	if( input_code_pressed_once( p_psxgpu->machine, KEYCODE_L ) )
	{
		FILE *f;
		popmessage( "loading..." );
		f = fopen( "VRAM.BIN", "rb" );
		for( n_y = 0; n_y < 1024; n_y++ )
			fread( p_psxgpu->p_p_vram[ n_y ], 1024 * 2, 1, f );
		fclose( f );
	}
#endif
}

static int DebugMeshDisplay( psx_gpu *p_psxgpu, bitmap_t *bitmap, const rectangle *cliprect )
{
	if( p_psxgpu->m_debug.mesh )
	{
		copybitmap( bitmap, p_psxgpu->m_debug.mesh, 0, 0, 0, 0, cliprect );
	}
	p_psxgpu->m_debug.b_clear = 1;
	return p_psxgpu->m_debug.b_mesh;
}

static int DebugTextureDisplay( psx_gpu *p_psxgpu, bitmap_t *bitmap )
{
	UINT32 n_y;

	if( p_psxgpu->m_debug.b_texture )
	{
		screen_device *screen = p_psxgpu->machine->primary_screen;
		int width = screen->width();
		int height = screen->height();

		for( n_y = 0; n_y < height; n_y++ )
		{
			int n_x;
			int n_xi;
			int n_yi;
			UINT16 p_n_interleave[ 1024 ];

			for( n_x = 0; n_x < width; n_x++ )
			{
				if( p_psxgpu->m_debug.n_interleave == 0 )
				{
					n_xi = ( n_x & ~0x3c ) + ( ( n_y << 2 ) & 0x3c );
					n_yi = ( n_y & ~0xf ) + ( ( n_x >> 2 ) & 0xf );
				}
				else if( p_psxgpu->m_debug.n_interleave == 1 )
				{
					n_xi = ( n_x & ~0x78 ) + ( ( n_x << 3 ) & 0x40 ) + ( ( n_y << 3 ) & 0x38 );
					n_yi = ( n_y & ~0x7 ) + ( ( n_x >> 4 ) & 0x7 );
				}
				else
				{
					n_xi = n_x;
					n_yi = n_y;
				}
				p_n_interleave[ n_x ] = p_psxgpu->p_p_vram[ n_yi ][ n_xi ];
			}
			draw_scanline16( bitmap, 0, n_y, width, p_n_interleave, p_psxgpu->machine->pens );
		}
	}
	return p_psxgpu->m_debug.b_texture;
}

#endif

static STATE_POSTLOAD( updatevisiblearea )
{
	psx_gpu *p_psxgpu = machine->driver_data<psx_state>()->p_psxgpu;
	rectangle visarea;
	float refresh;

	if( ( p_psxgpu->n_gpustatus & ( 1 << 0x14 ) ) != 0 )
	{
		/* pal */
		refresh = 50;
		switch( ( p_psxgpu->n_gpustatus >> 0x13 ) & 1 )
		{
		case 0:
			p_psxgpu->n_screenheight = 256;
			break;
		case 1:
			p_psxgpu->n_screenheight = 512;
			break;
		}
	}
	else
	{
		/* ntsc */
		refresh = 60;
		switch( ( p_psxgpu->n_gpustatus >> 0x13 ) & 1 )
		{
		case 0:
			p_psxgpu->n_screenheight = 240;
			break;
		case 1:
			p_psxgpu->n_screenheight = 480;
			break;
		}
	}
	switch( ( p_psxgpu->n_gpustatus >> 0x11 ) & 3 )
	{
	case 0:
		switch( ( p_psxgpu->n_gpustatus >> 0x10 ) & 1 )
		{
		case 0:
			p_psxgpu->n_screenwidth = 256;
			break;
		case 1:
			p_psxgpu->n_screenwidth = 368;
			break;
		}
		break;
	case 1:
		switch( ( p_psxgpu->n_gpustatus >> 0x10 ) & 1 )
		{
		case 0:
			p_psxgpu->n_screenwidth = 320;
			break;
		case 1:
			p_psxgpu->n_screenwidth = 384;
			break;
		}
		break;
	case 2:
		p_psxgpu->n_screenwidth = 512;
		break;
	case 3:
		p_psxgpu->n_screenwidth = 640;
		break;
	}
	visarea.min_x = visarea.min_y = 0;
	visarea.max_x = p_psxgpu->n_screenwidth - 1;
	visarea.max_y = p_psxgpu->n_screenheight - 1;
	machine->primary_screen->configure(p_psxgpu->n_screenwidth, p_psxgpu->n_screenheight, visarea, HZ_TO_ATTOSECONDS(refresh));
}

static void psx_gpu_init( running_machine *machine, int n_gputype )
{
	psx_gpu *p_psxgpu = auto_alloc_clear(machine, psx_gpu);
	int n_line;
	int n_level;
	int n_level2;
	int n_shade;
	int n_shaded;
	int width = machine->primary_screen->width();
	int height = machine->primary_screen->height();

	p_psxgpu->machine = machine;
	p_psxgpu->n_gputype = n_gputype;
	p_psxgpu->b_need_sianniv_vblank_hack = !strcmp(machine->gamedrv->name, "sianniv");

#if defined( MAME_DEBUG )
	DebugMeshInit(p_psxgpu);
#endif

	p_psxgpu->n_gpustatus = 0x14802000;
	p_psxgpu->n_gpuinfo = 0;
	p_psxgpu->n_gpu_buffer_offset = 0;
	p_psxgpu->n_lightgun_x = 0;
	p_psxgpu->n_lightgun_y = 0;

	p_psxgpu->n_vram_size = width * height;
	p_psxgpu->p_vram = auto_alloc_array_clear( machine, UINT16, p_psxgpu->n_vram_size );

	for( n_line = 0; n_line < 1024; n_line++ )
	{
		p_psxgpu->p_p_vram[ n_line ] = &p_psxgpu->p_vram[ ( n_line % height ) * width ];
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
			p_psxgpu->p_n_redshade[ ( n_level * MAX_SHADE ) | n_shade ] = n_shaded;
			p_psxgpu->p_n_greenshade[ ( n_level * MAX_SHADE ) | n_shade ] = n_shaded << 5;
			p_psxgpu->p_n_blueshade[ ( n_level * MAX_SHADE ) | n_shade ] = n_shaded << 10;

			/* 1/4 x transparency */
			n_shaded = ( n_level * n_shade ) / MID_SHADE;
			n_shaded >>= 2;
			if( n_shaded > MAX_LEVEL - 1 )
			{
				n_shaded = MAX_LEVEL - 1;
			}
			p_psxgpu->p_n_f025[ ( n_level * MAX_SHADE ) | n_shade ] = n_shaded;

			/* 1/2 x transparency */
			n_shaded = ( n_level * n_shade ) / MID_SHADE;
			n_shaded >>= 1;
			if( n_shaded > MAX_LEVEL - 1 )
			{
				n_shaded = MAX_LEVEL - 1;
			}
			p_psxgpu->p_n_f05[ ( n_level * MAX_SHADE ) | n_shade ] = n_shaded;

			/* 1 x transparency */
			n_shaded = ( n_level * n_shade ) / MID_SHADE;
			if( n_shaded > MAX_LEVEL - 1 )
			{
				n_shaded = MAX_LEVEL - 1;
			}
			p_psxgpu->p_n_f1[ ( n_level * MAX_SHADE ) | n_shade ] = n_shaded;
		}
	}

	for( n_level = 0; n_level < 0x10000; n_level++ )
	{
		p_psxgpu->p_n_redlevel[ n_level ] = ( n_level & ( MAX_LEVEL - 1 ) ) * MAX_SHADE;
		p_psxgpu->p_n_greenlevel[ n_level ] = ( ( n_level >> 5 ) & ( MAX_LEVEL - 1 ) ) * MAX_SHADE;
		p_psxgpu->p_n_bluelevel[ n_level ] = ( ( n_level >> 10 ) & ( MAX_LEVEL - 1 ) ) * MAX_SHADE;

		/* 0.5 * background */
		p_psxgpu->p_n_redb05[ n_level ] = ( ( n_level & ( MAX_LEVEL - 1 ) ) / 2 ) * MAX_LEVEL;
		p_psxgpu->p_n_greenb05[ n_level ] = ( ( ( n_level >> 5 ) & ( MAX_LEVEL - 1 ) ) / 2 ) * MAX_LEVEL;
		p_psxgpu->p_n_blueb05[ n_level ] = ( ( ( n_level >> 10 ) & ( MAX_LEVEL - 1 ) ) / 2 ) * MAX_LEVEL;

		/* 1 * background */
		p_psxgpu->p_n_redb1[ n_level ] = ( n_level & ( MAX_LEVEL - 1 ) ) * MAX_LEVEL;
		p_psxgpu->p_n_greenb1[ n_level ] = ( ( n_level >> 5 ) & ( MAX_LEVEL - 1 ) ) * MAX_LEVEL;
		p_psxgpu->p_n_blueb1[ n_level ] = ( ( n_level >> 10 ) & ( MAX_LEVEL - 1 ) ) * MAX_LEVEL;

		/* 24bit to 15 bit conversion */
		p_psxgpu->p_n_g0r0[ n_level ] = ( ( ( n_level >> 11 ) & ( MAX_LEVEL - 1 ) ) << 5 ) | ( ( ( n_level >> 3 ) & ( MAX_LEVEL - 1 ) ) << 0 );
		p_psxgpu->p_n_b0[ n_level ] = ( ( n_level >> 3 ) & ( MAX_LEVEL - 1 ) ) << 10;
		p_psxgpu->p_n_r1[ n_level ] = ( ( n_level >> 11 ) & ( MAX_LEVEL - 1 ) ) << 0;
		p_psxgpu->p_n_b1g1[ n_level ] = ( ( ( n_level >> 11 ) & ( MAX_LEVEL - 1 ) ) << 10 ) | ( ( ( n_level >> 3 ) & ( MAX_LEVEL - 1 ) ) << 5 );
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
			p_psxgpu->p_n_redaddtrans[ ( n_level * MAX_LEVEL ) | n_level2 ] = n_shaded;
			p_psxgpu->p_n_greenaddtrans[ ( n_level * MAX_LEVEL ) | n_level2 ] = n_shaded << 5;
			p_psxgpu->p_n_blueaddtrans[ ( n_level * MAX_LEVEL ) | n_level2 ] = n_shaded << 10;

			/* sub transparency */
			n_shaded = ( n_level - n_level2 );
			if( n_shaded < 0 )
			{
				n_shaded = 0;
			}
			p_psxgpu->p_n_redsubtrans[ ( n_level * MAX_LEVEL ) | n_level2 ] = n_shaded;
			p_psxgpu->p_n_greensubtrans[ ( n_level * MAX_LEVEL ) | n_level2 ] = n_shaded << 5;
			p_psxgpu->p_n_bluesubtrans[ ( n_level * MAX_LEVEL ) | n_level2 ] = n_shaded << 10;
		}
	}

	// icky!!!
	state_save_register_memory( machine, "globals", NULL, 0, "m_packet", (UINT8 *)&p_psxgpu->m_packet, 1, sizeof( p_psxgpu->m_packet ), __FILE__, __LINE__ );

	state_save_register_global_pointer( machine, p_psxgpu->p_vram, p_psxgpu->n_vram_size );
	state_save_register_global( machine, p_psxgpu->n_gpu_buffer_offset );
	state_save_register_global( machine, p_psxgpu->n_vramx );
	state_save_register_global( machine, p_psxgpu->n_vramy );
	state_save_register_global( machine, p_psxgpu->n_twy );
	state_save_register_global( machine, p_psxgpu->n_twx );
	state_save_register_global( machine, p_psxgpu->n_tww );
	state_save_register_global( machine, p_psxgpu->n_drawarea_x1 );
	state_save_register_global( machine, p_psxgpu->n_drawarea_y1 );
	state_save_register_global( machine, p_psxgpu->n_drawarea_x2 );
	state_save_register_global( machine, p_psxgpu->n_drawarea_y2 );
	state_save_register_global( machine, p_psxgpu->n_horiz_disstart );
	state_save_register_global( machine, p_psxgpu->n_horiz_disend );
	state_save_register_global( machine, p_psxgpu->n_vert_disstart );
	state_save_register_global( machine, p_psxgpu->n_vert_disend );
	state_save_register_global( machine, p_psxgpu->b_reverseflag );
	state_save_register_global( machine, p_psxgpu->n_drawoffset_x );
	state_save_register_global( machine, p_psxgpu->n_drawoffset_y );
	state_save_register_global( machine, p_psxgpu->n_displaystartx );
	state_save_register_global( machine, p_psxgpu->n_displaystarty );
	state_save_register_global( machine, p_psxgpu->n_gpustatus );
	state_save_register_global( machine, p_psxgpu->n_gpuinfo );
	state_save_register_global( machine, p_psxgpu->n_lightgun_x );
	state_save_register_global( machine, p_psxgpu->n_lightgun_y );
	state_save_register_global( machine, p_psxgpu->n_tx );
	state_save_register_global( machine, p_psxgpu->n_ty );
	state_save_register_global( machine, p_psxgpu->n_abr );
	state_save_register_global( machine, p_psxgpu->n_tp );
	state_save_register_global( machine, p_psxgpu->n_ix );
	state_save_register_global( machine, p_psxgpu->n_iy );
	state_save_register_global( machine, p_psxgpu->n_ti );

	state_save_register_postload( machine, updatevisiblearea, NULL );
	machine->driver_data<psx_state>()->p_psxgpu = p_psxgpu;
}

VIDEO_START( psx_type1 )
{
	psx_gpu_init(machine, 1);
}

VIDEO_START( psx_type2 )
{
	psx_gpu_init(machine, 2);
}

VIDEO_UPDATE( psx )
{
	psx_gpu *p_psxgpu = screen->machine->driver_data<psx_state>()->p_psxgpu;
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
	if( DebugMeshDisplay( p_psxgpu, bitmap, cliprect ) )
	{
		return 0;
	}
	if( DebugTextureDisplay( p_psxgpu, bitmap ) )
	{
		return 0;
	}
#endif

	if( ( p_psxgpu->n_gpustatus & ( 1 << 0x17 ) ) != 0 )
	{
		/* todo: only draw to necessary area */
		bitmap_fill( bitmap, cliprect , 0);
	}
	else
	{
		if( p_psxgpu->b_reverseflag )
		{
			n_displaystartx = ( 1023 - p_psxgpu->n_displaystartx );
			/* todo: make this flip the screen, in the meantime.. */
			n_displaystartx -= ( p_psxgpu->n_screenwidth - 1 );
		}
		else
		{
			n_displaystartx = p_psxgpu->n_displaystartx;
		}

		if( ( p_psxgpu->n_gpustatus & ( 1 << 0x14 ) ) != 0 )
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

		n_top = (INT32)p_psxgpu->n_vert_disstart - n_overscantop;
		n_lines = (INT32)p_psxgpu->n_vert_disend - (INT32)p_psxgpu->n_vert_disstart;
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
		if( ( p_psxgpu->n_gpustatus & ( 1 << 0x16 ) ) != 0 )
		{
			/* interlaced */
			n_lines *= 2;
		}
		if( n_lines > p_psxgpu->n_screenheight - ( n_y + n_top ) )
		{
			n_lines = p_psxgpu->n_screenheight - ( n_y + n_top );
		}
		else
		{
			/* todo: draw bottom border */
		}

		n_left = ( ( (INT32)p_psxgpu->n_horiz_disstart - n_overscanleft ) * (INT32)p_psxgpu->n_screenwidth ) / 2560;
		n_columns = ( ( ( (INT32)p_psxgpu->n_horiz_disend - p_psxgpu->n_horiz_disstart ) * (INT32)p_psxgpu->n_screenwidth ) / 2560 );
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
		if( n_columns > p_psxgpu->n_screenwidth - ( n_x + n_left ) )
		{
			n_columns = p_psxgpu->n_screenwidth - ( n_x + n_left );
		}
		else
		{
			/* todo: draw right border */
		}

		if( ( p_psxgpu->n_gpustatus & ( 1 << 0x15 ) ) != 0 )
		{
			/* 24bit */
			n_line = n_lines;
			while( n_line > 0 )
			{
				UINT16 *p_n_src = p_psxgpu->p_p_vram[ n_y + p_psxgpu->n_displaystarty ] + n_x + n_displaystartx;
				UINT16 *p_n_dest = BITMAP_ADDR16(bitmap, n_y + n_top, n_x + n_left);

				n_column = n_columns;
				while( n_column > 0 )
				{
					UINT32 n_g0r0 = *( p_n_src++ );
					UINT32 n_r1b0 = *( p_n_src++ );
					UINT32 n_b1g1 = *( p_n_src++ );

					*( p_n_dest++ ) = p_psxgpu->p_n_g0r0[ n_g0r0 ] | p_psxgpu->p_n_b0[ n_r1b0 ];
					n_column--;
					if( n_column > 0 )
					{
						*( p_n_dest++ ) = p_psxgpu->p_n_r1[ n_r1b0 ] | p_psxgpu->p_n_b1g1[ n_b1g1 ];
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
				draw_scanline16( bitmap, n_x + n_left, n_y + n_top, n_columns, p_psxgpu->p_p_vram[ n_y + p_psxgpu->n_displaystarty ] + n_x + n_displaystartx, NULL );
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

INLINE void decode_tpage( psx_gpu *p_psxgpu, UINT32 tpage )
{
	if( p_psxgpu->n_gputype == 2 )
	{
		p_psxgpu->n_gpustatus = ( p_psxgpu->n_gpustatus & 0xfffff800 ) | ( tpage & 0x7ff );

		p_psxgpu->n_tx = ( tpage & 0x0f ) << 6;
		p_psxgpu->n_ty = ( ( tpage & 0x10 ) << 4 ) | ( ( tpage & 0x800 ) >> 2 );
		p_psxgpu->n_abr = ( tpage & 0x60 ) >> 5;
		p_psxgpu->n_tp = ( tpage & 0x180 ) >> 7;
		p_psxgpu->n_ix = ( tpage & 0x1000 ) >> 12;
		p_psxgpu->n_iy = ( tpage & 0x2000 ) >> 13;
		p_psxgpu->n_ti = 0;
		if( ( tpage & ~0x39ff ) != 0 )
		{
			verboselog( p_psxgpu, 1, "not handled: draw mode %08x\n", tpage & ~0x39ff );
		}
		if( p_psxgpu->n_tp == 3 )
		{
			verboselog( p_psxgpu, 0, "not handled: tp == 3\n" );
		}
	}
	else
	{
		p_psxgpu->n_gpustatus = ( p_psxgpu->n_gpustatus & 0xffffe000 ) | ( tpage & 0x1fff );

		p_psxgpu->n_tx = ( tpage & 0x0f ) << 6;
		p_psxgpu->n_ty = ( ( tpage & 0x60 ) << 3 );
		p_psxgpu->n_abr = ( tpage & 0x180 ) >> 7;
		p_psxgpu->n_tp = ( tpage & 0x600 ) >> 9;
		p_psxgpu->n_ti = ( tpage & 0x2000 ) >> 13;
		p_psxgpu->n_ix = 0;
		p_psxgpu->n_iy = 0;
		if( ( tpage & ~0x27ef ) != 0 )
		{
			verboselog( p_psxgpu, 1, "not handled: draw mode %08x\n", tpage & ~0x27ef );
		}
		if( p_psxgpu->n_tp == 3 )
		{
			verboselog( p_psxgpu, 0, "not handled: tp == 3\n" );
		}
		else if( p_psxgpu->n_tp == 2 && p_psxgpu->n_ti != 0 )
		{
			verboselog( p_psxgpu, 0, "not handled: interleaved 15 bit texture\n" );
		}
	}
}

#define SPRITESETUP \
	if( p_psxgpu->n_iy != 0 ) \
	{ \
		n_dv = -1; \
	} \
	else \
	{ \
		n_dv = 1; \
	} \
	if( p_psxgpu->n_ix != 0 ) \
	{ \
		n_du = -1; \
	} \
	else \
	{ \
		n_du = 1; \
	}

#define TRANSPARENCYSETUP \
	p_n_f = p_psxgpu->p_n_f1; \
	p_n_redb = p_psxgpu->p_n_redb1; \
	p_n_greenb = p_psxgpu->p_n_greenb1; \
	p_n_blueb = p_psxgpu->p_n_blueb1; \
	p_n_redtrans = p_psxgpu->p_n_redaddtrans; \
	p_n_greentrans = p_psxgpu->p_n_greenaddtrans; \
	p_n_bluetrans = p_psxgpu->p_n_blueaddtrans; \
 \
	switch( n_cmd & 0x02 ) \
	{ \
	case 0x02: \
		switch( p_psxgpu->n_abr ) \
		{ \
		case 0x00: \
			p_n_f = p_psxgpu->p_n_f05; \
			p_n_redb = p_psxgpu->p_n_redb05; \
			p_n_greenb = p_psxgpu->p_n_greenb05; \
			p_n_blueb = p_psxgpu->p_n_blueb05; \
			p_n_redtrans = p_psxgpu->p_n_redaddtrans; \
			p_n_greentrans = p_psxgpu->p_n_greenaddtrans; \
			p_n_bluetrans = p_psxgpu->p_n_blueaddtrans; \
			verboselog( p_psxgpu, 2, "Transparency Mode: 0.5*B + 0.5*F\n" ); \
			break; \
		case 0x01: \
			p_n_f = p_psxgpu->p_n_f1; \
			p_n_redb = p_psxgpu->p_n_redb1; \
			p_n_greenb = p_psxgpu->p_n_greenb1; \
			p_n_blueb = p_psxgpu->p_n_blueb1; \
			p_n_redtrans = p_psxgpu->p_n_redaddtrans; \
			p_n_greentrans = p_psxgpu->p_n_greenaddtrans; \
			p_n_bluetrans = p_psxgpu->p_n_blueaddtrans; \
			verboselog( p_psxgpu, 2, "Transparency Mode: 1.0*B + 1.0*F\n" ); \
			break; \
		case 0x02: \
			p_n_f = p_psxgpu->p_n_f1; \
			p_n_redb = p_psxgpu->p_n_redb1; \
			p_n_greenb = p_psxgpu->p_n_greenb1; \
			p_n_blueb = p_psxgpu->p_n_blueb1; \
			p_n_redtrans = p_psxgpu->p_n_redsubtrans; \
			p_n_greentrans = p_psxgpu->p_n_greensubtrans; \
			p_n_bluetrans = p_psxgpu->p_n_bluesubtrans; \
			verboselog( p_psxgpu, 2, "Transparency Mode: 1.0*B - 1.0*F\n" ); \
			break; \
		case 0x03: \
			p_n_f = p_psxgpu->p_n_f025; \
			p_n_redb = p_psxgpu->p_n_redb1; \
			p_n_greenb = p_psxgpu->p_n_greenb1; \
			p_n_blueb = p_psxgpu->p_n_blueb1; \
			p_n_redtrans = p_psxgpu->p_n_redaddtrans; \
			p_n_greentrans = p_psxgpu->p_n_greenaddtrans; \
			p_n_bluetrans = p_psxgpu->p_n_blueaddtrans; \
			verboselog( p_psxgpu, 2, "Transparency Mode: 1.0*B + 0.25*F\n" ); \
			break; \
		} \
		break; \
	}

#define SOLIDSETUP \
	TRANSPARENCYSETUP

#define TEXTURESETUP \
	n_tx = p_psxgpu->n_tx; \
	n_ty = p_psxgpu->n_ty; \
	p_clut = p_psxgpu->p_p_vram[ n_cluty ] + n_clutx; \
	switch( p_psxgpu->n_tp ) \
	{ \
	case 0: \
		n_tx += p_psxgpu->n_twx >> 2; \
		n_ty += p_psxgpu->n_twy; \
		break; \
	case 1: \
		n_tx += p_psxgpu->n_twx >> 1; \
		n_ty += p_psxgpu->n_twy; \
		break; \
	case 2: \
		n_tx += p_psxgpu->n_twx >> 0; \
		n_ty += p_psxgpu->n_twy; \
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
	if( n_distance > ( (INT32)p_psxgpu->n_drawarea_x2 - n_x ) + 1 ) \
	{ \
		n_distance = ( p_psxgpu->n_drawarea_x2 - n_x ) + 1; \
	} \
	p_vram = p_psxgpu->p_p_vram[ n_y ] + n_x; \
 \
	switch( n_cmd & 0x02 ) \
	{ \
	case 0x00: \
		/* transparency off */ \
		while( n_distance > 0 ) \
		{ \
			WRITE_PIXEL( \
				p_psxgpu->p_n_redshade[ MID_LEVEL | n_r.w.h ] | \
				p_psxgpu->p_n_greenshade[ MID_LEVEL | n_g.w.h ] | \
				p_psxgpu->p_n_blueshade[ MID_LEVEL | n_b.w.h ] ); \
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
		n_bgr = p_clut[ ( *( p_psxgpu->p_p_vram[ n_ty + TXV ] + n_tx + ( TXU >> 2 ) ) >> ( ( TXU & 0x03 ) << 2 ) ) & 0x0f ];

#define TEXTURE8BIT( TXV, TXU ) \
	TEXTURE_LOOP \
		n_bgr = p_clut[ ( *( p_psxgpu->p_p_vram[ n_ty + TXV ] + n_tx + ( TXU >> 1 ) ) >> ( ( TXU & 0x01 ) << 3 ) ) & 0xff ];

#define TEXTURE15BIT( TXV, TXU ) \
	TEXTURE_LOOP \
		n_bgr = *( p_psxgpu->p_p_vram[ n_ty + TXV ] + n_tx + TXU );

#define TEXTUREWINDOW4BIT( TXV, TXU ) TEXTURE4BIT( ( TXV & p_psxgpu->n_twh ), ( TXU & p_psxgpu->n_tww ) )
#define TEXTUREWINDOW8BIT( TXV, TXU ) TEXTURE8BIT( ( TXV & p_psxgpu->n_twh ), ( TXU & p_psxgpu->n_tww ) )
#define TEXTUREWINDOW15BIT( TXV, TXU ) TEXTURE15BIT( ( TXV & p_psxgpu->n_twh ), ( TXU & p_psxgpu->n_tww ) )

#define TEXTUREINTERLEAVED4BIT( TXV, TXU ) \
	TEXTURE_LOOP \
		int n_xi = ( ( TXU >> 2 ) & ~0x3c ) + ( ( TXV << 2 ) & 0x3c ); \
		int n_yi = ( TXV & ~0xf ) + ( ( TXU >> 4 ) & 0xf ); \
		n_bgr = p_clut[ ( *( p_psxgpu->p_p_vram[ n_ty + n_yi ] + n_tx + n_xi ) >> ( ( TXU & 0x03 ) << 2 ) ) & 0x0f ];

#define TEXTUREINTERLEAVED8BIT( TXV, TXU ) \
	TEXTURE_LOOP \
		int n_xi = ( ( TXU >> 1 ) & ~0x78 ) + ( ( TXU << 2 ) & 0x40 ) + ( ( TXV << 3 ) & 0x38 ); \
		int n_yi = ( TXV & ~0x7 ) + ( ( TXU >> 5 ) & 0x7 ); \
		n_bgr = p_clut[ ( *( p_psxgpu->p_p_vram[ n_ty + n_yi ] + n_tx + n_xi ) >> ( ( TXU & 0x01 ) << 3 ) ) & 0xff ];

#define TEXTUREINTERLEAVED15BIT( TXV, TXU ) \
	TEXTURE_LOOP \
		int n_xi = TXU; \
		int n_yi = TXV; \
		n_bgr = *( p_psxgpu->p_p_vram[ n_ty + n_yi ] + n_tx + n_xi );

#define TEXTUREWINDOWINTERLEAVED4BIT( TXV, TXU ) TEXTUREINTERLEAVED4BIT( ( TXV & p_psxgpu->n_twh ), ( TXU & p_psxgpu->n_tww ) )
#define TEXTUREWINDOWINTERLEAVED8BIT( TXV, TXU ) TEXTUREINTERLEAVED8BIT( ( TXV & p_psxgpu->n_twh ), ( TXU & p_psxgpu->n_tww ) )
#define TEXTUREWINDOWINTERLEAVED15BIT( TXV, TXU ) TEXTUREINTERLEAVED15BIT( ( TXV & p_psxgpu->n_twh ), ( TXU & p_psxgpu->n_tww ) )

#define SHADEDPIXEL( PIXELUPDATE ) \
		if( n_bgr != 0 ) \
		{ \
			WRITE_PIXEL( \
				p_psxgpu->p_n_redshade[ p_psxgpu->p_n_redlevel[ n_bgr ] | n_r.w.h ] | \
				p_psxgpu->p_n_greenshade[ p_psxgpu->p_n_greenlevel[ n_bgr ] | n_g.w.h ] | \
				p_psxgpu->p_n_blueshade[ p_psxgpu->p_n_bluelevel[ n_bgr ] | n_b.w.h ] ); \
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
					p_n_redtrans[ p_n_f[ p_psxgpu->p_n_redlevel[ n_bgr ] | n_r.w.h ] | p_n_redb[ *( p_vram ) ] ] | \
					p_n_greentrans[ p_n_f[ p_psxgpu->p_n_greenlevel[ n_bgr ] | n_g.w.h ] | p_n_greenb[ *( p_vram ) ] ] | \
					p_n_bluetrans[ p_n_f[ p_psxgpu->p_n_bluelevel[ n_bgr ] | n_b.w.h ] | p_n_blueb[ *( p_vram ) ] ] ); \
			} \
			else \
			{ \
				WRITE_PIXEL( \
					p_psxgpu->p_n_redshade[ p_psxgpu->p_n_redlevel[ n_bgr ] | n_r.w.h ] | \
					p_psxgpu->p_n_greenshade[ p_psxgpu->p_n_greenlevel[ n_bgr ] | n_g.w.h ] | \
					p_psxgpu->p_n_blueshade[ p_psxgpu->p_n_bluelevel[ n_bgr ] | n_b.w.h ] ); \
			} \
		} \
		p_vram++; \
		PIXELUPDATE \
		n_distance--; \
	TEXTURE_ENDLOOP

#define TEXTUREFILL( PIXELUPDATE, TXU, TXV ) \
	if( n_distance > ( (INT32)p_psxgpu->n_drawarea_x2 - n_x ) + 1 ) \
	{ \
		n_distance = ( p_psxgpu->n_drawarea_x2 - n_x ) + 1; \
	} \
	p_vram = p_psxgpu->p_p_vram[ n_y ] + n_x; \
 \
	if( p_psxgpu->n_ti != 0 ) \
	{ \
		/* interleaved texture */ \
		if( p_psxgpu->n_twh != 255 || \
			p_psxgpu->n_tww != 255 || \
			p_psxgpu->n_twx != 0 || \
			p_psxgpu->n_twy != 0 ) \
		{ \
			/* texture window */ \
			switch( n_cmd & 0x02 ) \
			{ \
			case 0x00: \
				/* shading */ \
				switch( p_psxgpu->n_tp ) \
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
				switch( p_psxgpu->n_tp ) \
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
				switch( p_psxgpu->n_tp ) \
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
				switch( p_psxgpu->n_tp ) \
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
		if( p_psxgpu->n_twh != 255 || \
			p_psxgpu->n_tww != 255 || \
			p_psxgpu->n_twx != 0 || \
			p_psxgpu->n_twy != 0 ) \
		{ \
			/* texture window */ \
			switch( n_cmd & 0x02 ) \
			{ \
			case 0x00: \
				/* shading */ \
				switch( p_psxgpu->n_tp ) \
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
				switch( p_psxgpu->n_tp ) \
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
				switch( p_psxgpu->n_tp ) \
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
				switch( p_psxgpu->n_tp ) \
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

static void FlatPolygon( psx_gpu *p_psxgpu, int n_points )
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
	if( p_psxgpu->m_debug.n_skip == 1 )
	{
		return;
	}
	for( n_point = 0; n_point < n_points; n_point++ )
	{
		DebugMesh( p_psxgpu, COORD_X( p_psxgpu->m_packet.FlatPolygon.vertex[ n_point ].n_coord ) + p_psxgpu->n_drawoffset_x, COORD_Y( p_psxgpu->m_packet.FlatPolygon.vertex[ n_point ].n_coord ) + p_psxgpu->n_drawoffset_y );
	}
	DebugMeshEnd(p_psxgpu);
#endif

	n_cmd = BGR_C( p_psxgpu->m_packet.FlatPolygon.n_bgr );

	n_cx1.d = 0;
	n_cx2.d = 0;

	SOLIDSETUP

	n_r.w.h = BGR_R( p_psxgpu->m_packet.FlatPolygon.n_bgr ); n_r.w.l = 0;
	n_g.w.h = BGR_G( p_psxgpu->m_packet.FlatPolygon.n_bgr ); n_g.w.l = 0;
	n_b.w.h = BGR_B( p_psxgpu->m_packet.FlatPolygon.n_bgr ); n_b.w.l = 0;

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
		ADJUST_COORD( p_psxgpu->m_packet.FlatPolygon.vertex[ n_point ].n_coord );
	}

	n_leftpoint = 0;
	for( n_point = 1; n_point < n_points; n_point++ )
	{
		if( COORD_Y( p_psxgpu->m_packet.FlatPolygon.vertex[ n_point ].n_coord ) < COORD_Y( p_psxgpu->m_packet.FlatPolygon.vertex[ n_leftpoint ].n_coord ) ||
			( COORD_Y( p_psxgpu->m_packet.FlatPolygon.vertex[ n_point ].n_coord ) == COORD_Y( p_psxgpu->m_packet.FlatPolygon.vertex[ n_leftpoint ].n_coord ) &&
			COORD_X( p_psxgpu->m_packet.FlatPolygon.vertex[ n_point ].n_coord ) < COORD_X( p_psxgpu->m_packet.FlatPolygon.vertex[ n_leftpoint ].n_coord ) ) )
		{
			n_leftpoint = n_point;
		}
	}
	n_rightpoint = n_leftpoint;

	n_dx1 = 0;
	n_dx2 = 0;

	n_y = COORD_Y( p_psxgpu->m_packet.FlatPolygon.vertex[ n_rightpoint ].n_coord );

	for( ;; )
	{
		if( n_y == COORD_Y( p_psxgpu->m_packet.FlatPolygon.vertex[ n_leftpoint ].n_coord ) )
		{
			while( n_y == COORD_Y( p_psxgpu->m_packet.FlatPolygon.vertex[ p_n_leftpointlist[ n_leftpoint ] ].n_coord ) )
			{
				n_leftpoint = p_n_leftpointlist[ n_leftpoint ];
				if( n_leftpoint == n_rightpoint )
				{
					break;
				}
			}
			n_cx1.w.h = COORD_X( p_psxgpu->m_packet.FlatPolygon.vertex[ n_leftpoint ].n_coord ); n_cx1.w.l = 0;
			n_leftpoint = p_n_leftpointlist[ n_leftpoint ];
			n_distance = COORD_Y( p_psxgpu->m_packet.FlatPolygon.vertex[ n_leftpoint ].n_coord ) - n_y;
			if( n_distance < 1 )
			{
				break;
			}
			n_dx1 = (INT32)( ( COORD_X( p_psxgpu->m_packet.FlatPolygon.vertex[ n_leftpoint ].n_coord ) << 16 ) - n_cx1.d ) / n_distance;
		}
		if( n_y == COORD_Y( p_psxgpu->m_packet.FlatPolygon.vertex[ n_rightpoint ].n_coord ) )
		{
			while( n_y == COORD_Y( p_psxgpu->m_packet.FlatPolygon.vertex[ p_n_rightpointlist[ n_rightpoint ] ].n_coord ) )
			{
				n_rightpoint = p_n_rightpointlist[ n_rightpoint ];
				if( n_rightpoint == n_leftpoint )
				{
					break;
				}
			}
			n_cx2.w.h = COORD_X( p_psxgpu->m_packet.FlatPolygon.vertex[ n_rightpoint ].n_coord ); n_cx2.w.l = 0;
			n_rightpoint = p_n_rightpointlist[ n_rightpoint ];
			n_distance = COORD_Y( p_psxgpu->m_packet.FlatPolygon.vertex[ n_rightpoint ].n_coord ) - n_y;
			if( n_distance < 1 )
			{
				break;
			}
			n_dx2 = (INT32)( ( COORD_X( p_psxgpu->m_packet.FlatPolygon.vertex[ n_rightpoint ].n_coord ) << 16 ) - n_cx2.d ) / n_distance;
		}
		if( (INT16)n_cx1.w.h != (INT16)n_cx2.w.h && n_y >= (INT32)p_psxgpu->n_drawarea_y1 && n_y <= (INT32)p_psxgpu->n_drawarea_y2 )
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

			if( ( (INT32)p_psxgpu->n_drawarea_x1 - n_x ) > 0 )
			{
				n_distance -= ( p_psxgpu->n_drawarea_x1 - n_x );
				n_x = p_psxgpu->n_drawarea_x1;
			}
			SOLIDFILL( FLATPOLYGONUPDATE )
		}
		n_cx1.d += n_dx1;
		n_cx2.d += n_dx2;
		n_y++;
	}
}

static void FlatTexturedPolygon( psx_gpu *p_psxgpu, int n_points )
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
	if( p_psxgpu->m_debug.n_skip == 2 )
	{
		return;
	}
	for( n_point = 0; n_point < n_points; n_point++ )
	{
		DebugMesh( p_psxgpu, COORD_X( p_psxgpu->m_packet.FlatTexturedPolygon.vertex[ n_point ].n_coord ) + p_psxgpu->n_drawoffset_x, COORD_Y( p_psxgpu->m_packet.FlatTexturedPolygon.vertex[ n_point ].n_coord ) + p_psxgpu->n_drawoffset_y );
	}
	DebugMeshEnd(p_psxgpu);
#endif

	n_cmd = BGR_C( p_psxgpu->m_packet.FlatTexturedPolygon.n_bgr );

	n_clutx = ( p_psxgpu->m_packet.FlatTexturedPolygon.vertex[ 0 ].n_texture.w.h & 0x3f ) << 4;
	n_cluty = ( p_psxgpu->m_packet.FlatTexturedPolygon.vertex[ 0 ].n_texture.w.h >> 6 ) & 0x3ff;

	n_r.d = 0;
	n_g.d = 0;
	n_b.d = 0;
	n_cx1.d = 0;
	n_cu1.d = 0;
	n_cv1.d = 0;
	n_cx2.d = 0;
	n_cu2.d = 0;
	n_cv2.d = 0;

	decode_tpage( p_psxgpu, p_psxgpu->m_packet.FlatTexturedPolygon.vertex[ 1 ].n_texture.w.h );
	TEXTURESETUP

	switch( n_cmd & 0x01 )
	{
	case 0:
		n_r.w.h = BGR_R( p_psxgpu->m_packet.FlatTexturedPolygon.n_bgr ); n_r.w.l = 0;
		n_g.w.h = BGR_G( p_psxgpu->m_packet.FlatTexturedPolygon.n_bgr ); n_g.w.l = 0;
		n_b.w.h = BGR_B( p_psxgpu->m_packet.FlatTexturedPolygon.n_bgr ); n_b.w.l = 0;
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
		ADJUST_COORD( p_psxgpu->m_packet.FlatTexturedPolygon.vertex[ n_point ].n_coord );
	}

	n_leftpoint = 0;
	for( n_point = 1; n_point < n_points; n_point++ )
	{
		if( COORD_Y( p_psxgpu->m_packet.FlatTexturedPolygon.vertex[ n_point ].n_coord ) < COORD_Y( p_psxgpu->m_packet.FlatTexturedPolygon.vertex[ n_leftpoint ].n_coord ) ||
			( COORD_Y( p_psxgpu->m_packet.FlatTexturedPolygon.vertex[ n_point ].n_coord ) == COORD_Y( p_psxgpu->m_packet.FlatTexturedPolygon.vertex[ n_leftpoint ].n_coord ) &&
			COORD_X( p_psxgpu->m_packet.FlatTexturedPolygon.vertex[ n_point ].n_coord ) < COORD_X( p_psxgpu->m_packet.FlatTexturedPolygon.vertex[ n_leftpoint ].n_coord ) ) )
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

	n_y = COORD_Y( p_psxgpu->m_packet.FlatTexturedPolygon.vertex[ n_rightpoint ].n_coord );

	for( ;; )
	{
		if( n_y == COORD_Y( p_psxgpu->m_packet.FlatTexturedPolygon.vertex[ n_leftpoint ].n_coord ) )
		{
			while( n_y == COORD_Y( p_psxgpu->m_packet.FlatTexturedPolygon.vertex[ p_n_leftpointlist[ n_leftpoint ] ].n_coord ) )
			{
				n_leftpoint = p_n_leftpointlist[ n_leftpoint ];
				if( n_leftpoint == n_rightpoint )
				{
					break;
				}
			}
			n_cx1.w.h = COORD_X( p_psxgpu->m_packet.FlatTexturedPolygon.vertex[ n_leftpoint ].n_coord ); n_cx1.w.l = 0;
			n_cu1.w.h = TEXTURE_U( p_psxgpu->m_packet.FlatTexturedPolygon.vertex[ n_leftpoint ].n_texture ); n_cu1.w.l = 0;
			n_cv1.w.h = TEXTURE_V( p_psxgpu->m_packet.FlatTexturedPolygon.vertex[ n_leftpoint ].n_texture ); n_cv1.w.l = 0;
			n_leftpoint = p_n_leftpointlist[ n_leftpoint ];
			n_distance = COORD_Y( p_psxgpu->m_packet.FlatTexturedPolygon.vertex[ n_leftpoint ].n_coord ) - n_y;
			if( n_distance < 1 )
			{
				break;
			}
			n_dx1 = (INT32)( ( COORD_X( p_psxgpu->m_packet.FlatTexturedPolygon.vertex[ n_leftpoint ].n_coord ) << 16 ) - n_cx1.d ) / n_distance;
			n_du1 = (INT32)( ( TEXTURE_U( p_psxgpu->m_packet.FlatTexturedPolygon.vertex[ n_leftpoint ].n_texture ) << 16 ) - n_cu1.d ) / n_distance;
			n_dv1 = (INT32)( ( TEXTURE_V( p_psxgpu->m_packet.FlatTexturedPolygon.vertex[ n_leftpoint ].n_texture ) << 16 ) - n_cv1.d ) / n_distance;
		}
		if( n_y == COORD_Y( p_psxgpu->m_packet.FlatTexturedPolygon.vertex[ n_rightpoint ].n_coord ) )
		{
			while( n_y == COORD_Y( p_psxgpu->m_packet.FlatTexturedPolygon.vertex[ p_n_rightpointlist[ n_rightpoint ] ].n_coord ) )
			{
				n_rightpoint = p_n_rightpointlist[ n_rightpoint ];
				if( n_rightpoint == n_leftpoint )
				{
					break;
				}
			}
			n_cx2.w.h = COORD_X( p_psxgpu->m_packet.FlatTexturedPolygon.vertex[ n_rightpoint ].n_coord ); n_cx2.w.l = 0;
			n_cu2.w.h = TEXTURE_U( p_psxgpu->m_packet.FlatTexturedPolygon.vertex[ n_rightpoint ].n_texture ); n_cu2.w.l = 0;
			n_cv2.w.h = TEXTURE_V( p_psxgpu->m_packet.FlatTexturedPolygon.vertex[ n_rightpoint ].n_texture ); n_cv2.w.l = 0;
			n_rightpoint = p_n_rightpointlist[ n_rightpoint ];
			n_distance = COORD_Y( p_psxgpu->m_packet.FlatTexturedPolygon.vertex[ n_rightpoint ].n_coord ) - n_y;
			if( n_distance < 1 )
			{
				break;
			}
			n_dx2 = (INT32)( ( COORD_X( p_psxgpu->m_packet.FlatTexturedPolygon.vertex[ n_rightpoint ].n_coord ) << 16 ) - n_cx2.d ) / n_distance;
			n_du2 = (INT32)( ( TEXTURE_U( p_psxgpu->m_packet.FlatTexturedPolygon.vertex[ n_rightpoint ].n_texture ) << 16 ) - n_cu2.d ) / n_distance;
			n_dv2 = (INT32)( ( TEXTURE_V( p_psxgpu->m_packet.FlatTexturedPolygon.vertex[ n_rightpoint ].n_texture ) << 16 ) - n_cv2.d ) / n_distance;
		}
		if( (INT16)n_cx1.w.h != (INT16)n_cx2.w.h && n_y >= (INT32)p_psxgpu->n_drawarea_y1 && n_y <= (INT32)p_psxgpu->n_drawarea_y2 )
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

			if( ( (INT32)p_psxgpu->n_drawarea_x1 - n_x ) > 0 )
			{
				n_u.d += n_du * ( p_psxgpu->n_drawarea_x1 - n_x );
				n_v.d += n_dv * ( p_psxgpu->n_drawarea_x1 - n_x );
				n_distance -= ( p_psxgpu->n_drawarea_x1 - n_x );
				n_x = p_psxgpu->n_drawarea_x1;
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

static void GouraudPolygon( psx_gpu *p_psxgpu, int n_points )
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
	if( p_psxgpu->m_debug.n_skip == 3 )
	{
		return;
	}
	for( n_point = 0; n_point < n_points; n_point++ )
	{
		DebugMesh( p_psxgpu, COORD_X( p_psxgpu->m_packet.GouraudPolygon.vertex[ n_point ].n_coord ) + p_psxgpu->n_drawoffset_x, COORD_Y( p_psxgpu->m_packet.GouraudPolygon.vertex[ n_point ].n_coord ) + p_psxgpu->n_drawoffset_y );
	}
	DebugMeshEnd(p_psxgpu);
#endif

	n_cmd = BGR_C( p_psxgpu->m_packet.GouraudPolygon.vertex[ 0 ].n_bgr );

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
		ADJUST_COORD( p_psxgpu->m_packet.GouraudPolygon.vertex[ n_point ].n_coord );
	}

	n_leftpoint = 0;
	for( n_point = 1; n_point < n_points; n_point++ )
	{
		if( COORD_Y( p_psxgpu->m_packet.GouraudPolygon.vertex[ n_point ].n_coord ) < COORD_Y( p_psxgpu->m_packet.GouraudPolygon.vertex[ n_leftpoint ].n_coord ) ||
			( COORD_Y( p_psxgpu->m_packet.GouraudPolygon.vertex[ n_point ].n_coord ) == COORD_Y( p_psxgpu->m_packet.GouraudPolygon.vertex[ n_leftpoint ].n_coord ) &&
			COORD_X( p_psxgpu->m_packet.GouraudPolygon.vertex[ n_point ].n_coord ) < COORD_X( p_psxgpu->m_packet.GouraudPolygon.vertex[ n_leftpoint ].n_coord ) ) )
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

	n_y = COORD_Y( p_psxgpu->m_packet.GouraudPolygon.vertex[ n_rightpoint ].n_coord );

	for( ;; )
	{
		if( n_y == COORD_Y( p_psxgpu->m_packet.GouraudPolygon.vertex[ n_leftpoint ].n_coord ) )
		{
			while( n_y == COORD_Y( p_psxgpu->m_packet.GouraudPolygon.vertex[ p_n_leftpointlist[ n_leftpoint ] ].n_coord ) )
			{
				n_leftpoint = p_n_leftpointlist[ n_leftpoint ];
				if( n_leftpoint == n_rightpoint )
				{
					break;
				}
			}
			n_cx1.w.h = COORD_X( p_psxgpu->m_packet.GouraudPolygon.vertex[ n_leftpoint ].n_coord ); n_cx1.w.l = 0;
			n_cr1.w.h = BGR_R( p_psxgpu->m_packet.GouraudPolygon.vertex[ n_leftpoint ].n_bgr ); n_cr1.w.l = 0;
			n_cg1.w.h = BGR_G( p_psxgpu->m_packet.GouraudPolygon.vertex[ n_leftpoint ].n_bgr ); n_cg1.w.l = 0;
			n_cb1.w.h = BGR_B( p_psxgpu->m_packet.GouraudPolygon.vertex[ n_leftpoint ].n_bgr ); n_cb1.w.l = 0;
			n_leftpoint = p_n_leftpointlist[ n_leftpoint ];
			n_distance = COORD_Y( p_psxgpu->m_packet.GouraudPolygon.vertex[ n_leftpoint ].n_coord ) - n_y;
			if( n_distance < 1 )
			{
				break;
			}
			n_dx1 = (INT32)( ( COORD_X( p_psxgpu->m_packet.GouraudPolygon.vertex[ n_leftpoint ].n_coord ) << 16 ) - n_cx1.d ) / n_distance;
			n_dr1 = (INT32)( ( BGR_R( p_psxgpu->m_packet.GouraudPolygon.vertex[ n_leftpoint ].n_bgr ) << 16 ) - n_cr1.d ) / n_distance;
			n_dg1 = (INT32)( ( BGR_G( p_psxgpu->m_packet.GouraudPolygon.vertex[ n_leftpoint ].n_bgr ) << 16 ) - n_cg1.d ) / n_distance;
			n_db1 = (INT32)( ( BGR_B( p_psxgpu->m_packet.GouraudPolygon.vertex[ n_leftpoint ].n_bgr ) << 16 ) - n_cb1.d ) / n_distance;
		}
		if( n_y == COORD_Y( p_psxgpu->m_packet.GouraudPolygon.vertex[ n_rightpoint ].n_coord ) )
		{
			while( n_y == COORD_Y( p_psxgpu->m_packet.GouraudPolygon.vertex[ p_n_rightpointlist[ n_rightpoint ] ].n_coord ) )
			{
				n_rightpoint = p_n_rightpointlist[ n_rightpoint ];
				if( n_rightpoint == n_leftpoint )
				{
					break;
				}
			}
			n_cx2.w.h = COORD_X( p_psxgpu->m_packet.GouraudPolygon.vertex[ n_rightpoint ].n_coord ); n_cx2.w.l = 0;
			n_cr2.w.h = BGR_R( p_psxgpu->m_packet.GouraudPolygon.vertex[ n_rightpoint ].n_bgr ); n_cr2.w.l = 0;
			n_cg2.w.h = BGR_G( p_psxgpu->m_packet.GouraudPolygon.vertex[ n_rightpoint ].n_bgr ); n_cg2.w.l = 0;
			n_cb2.w.h = BGR_B( p_psxgpu->m_packet.GouraudPolygon.vertex[ n_rightpoint ].n_bgr ); n_cb2.w.l = 0;
			n_rightpoint = p_n_rightpointlist[ n_rightpoint ];
			n_distance = COORD_Y( p_psxgpu->m_packet.GouraudPolygon.vertex[ n_rightpoint ].n_coord ) - n_y;
			if( n_distance < 1 )
			{
				break;
			}
			n_dx2 = (INT32)( ( COORD_X( p_psxgpu->m_packet.GouraudPolygon.vertex[ n_rightpoint ].n_coord ) << 16 ) - n_cx2.d ) / n_distance;
			n_dr2 = (INT32)( ( BGR_R( p_psxgpu->m_packet.GouraudPolygon.vertex[ n_rightpoint ].n_bgr ) << 16 ) - n_cr2.d ) / n_distance;
			n_dg2 = (INT32)( ( BGR_G( p_psxgpu->m_packet.GouraudPolygon.vertex[ n_rightpoint ].n_bgr ) << 16 ) - n_cg2.d ) / n_distance;
			n_db2 = (INT32)( ( BGR_B( p_psxgpu->m_packet.GouraudPolygon.vertex[ n_rightpoint ].n_bgr ) << 16 ) - n_cb2.d ) / n_distance;
		}
		if( (INT16)n_cx1.w.h != (INT16)n_cx2.w.h && n_y >= (INT32)p_psxgpu->n_drawarea_y1 && n_y <= (INT32)p_psxgpu->n_drawarea_y2 )
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

			if( ( (INT32)p_psxgpu->n_drawarea_x1 - n_x ) > 0 )
			{
				n_r.d += n_dr * ( p_psxgpu->n_drawarea_x1 - n_x );
				n_g.d += n_dg * ( p_psxgpu->n_drawarea_x1 - n_x );
				n_b.d += n_db * ( p_psxgpu->n_drawarea_x1 - n_x );
				n_distance -= ( p_psxgpu->n_drawarea_x1 - n_x );
				n_x = p_psxgpu->n_drawarea_x1;
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

static void GouraudTexturedPolygon( psx_gpu *p_psxgpu, int n_points )
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
	if( p_psxgpu->m_debug.n_skip == 4 )
	{
		return;
	}
	for( n_point = 0; n_point < n_points; n_point++ )
	{
		DebugMesh( p_psxgpu, COORD_X( p_psxgpu->m_packet.GouraudTexturedPolygon.vertex[ n_point ].n_coord ) + p_psxgpu->n_drawoffset_x, COORD_Y( p_psxgpu->m_packet.GouraudTexturedPolygon.vertex[ n_point ].n_coord ) + p_psxgpu->n_drawoffset_y );
	}
	DebugMeshEnd(p_psxgpu);
#endif

	n_cmd = BGR_C( p_psxgpu->m_packet.GouraudTexturedPolygon.vertex[ 0 ].n_bgr );

	n_clutx = ( p_psxgpu->m_packet.GouraudTexturedPolygon.vertex[ 0 ].n_texture.w.h & 0x3f ) << 4;
	n_cluty = ( p_psxgpu->m_packet.GouraudTexturedPolygon.vertex[ 0 ].n_texture.w.h >> 6 ) & 0x3ff;

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

	decode_tpage( p_psxgpu, p_psxgpu->m_packet.GouraudTexturedPolygon.vertex[ 1 ].n_texture.w.h );
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
		ADJUST_COORD( p_psxgpu->m_packet.GouraudTexturedPolygon.vertex[ n_point ].n_coord );
	}

	n_leftpoint = 0;
	for( n_point = 1; n_point < n_points; n_point++ )
	{
		if( COORD_Y( p_psxgpu->m_packet.GouraudTexturedPolygon.vertex[ n_point ].n_coord ) < COORD_Y( p_psxgpu->m_packet.GouraudTexturedPolygon.vertex[ n_leftpoint ].n_coord ) ||
			( COORD_Y( p_psxgpu->m_packet.GouraudTexturedPolygon.vertex[ n_point ].n_coord ) == COORD_Y( p_psxgpu->m_packet.GouraudTexturedPolygon.vertex[ n_leftpoint ].n_coord ) &&
			COORD_X( p_psxgpu->m_packet.GouraudTexturedPolygon.vertex[ n_point ].n_coord ) < COORD_X( p_psxgpu->m_packet.GouraudTexturedPolygon.vertex[ n_leftpoint ].n_coord ) ) )
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

	n_y = COORD_Y( p_psxgpu->m_packet.GouraudTexturedPolygon.vertex[ n_rightpoint ].n_coord );

	for( ;; )
	{
		if( n_y == COORD_Y( p_psxgpu->m_packet.GouraudTexturedPolygon.vertex[ n_leftpoint ].n_coord ) )
		{
			while( n_y == COORD_Y( p_psxgpu->m_packet.GouraudTexturedPolygon.vertex[ p_n_leftpointlist[ n_leftpoint ] ].n_coord ) )
			{
				n_leftpoint = p_n_leftpointlist[ n_leftpoint ];
				if( n_leftpoint == n_rightpoint )
				{
					break;
				}
			}
			n_cx1.w.h = COORD_X( p_psxgpu->m_packet.GouraudTexturedPolygon.vertex[ n_leftpoint ].n_coord ); n_cx1.w.l = 0;
			switch( n_cmd & 0x01 )
			{
			case 0x00:
				n_cr1.w.h = BGR_R( p_psxgpu->m_packet.GouraudTexturedPolygon.vertex[ n_leftpoint ].n_bgr ); n_cr1.w.l = 0;
				n_cg1.w.h = BGR_G( p_psxgpu->m_packet.GouraudTexturedPolygon.vertex[ n_leftpoint ].n_bgr ); n_cg1.w.l = 0;
				n_cb1.w.h = BGR_B( p_psxgpu->m_packet.GouraudTexturedPolygon.vertex[ n_leftpoint ].n_bgr ); n_cb1.w.l = 0;
				break;
			case 0x01:
				n_cr1.w.h = 0x80; n_cr1.w.l = 0;
				n_cg1.w.h = 0x80; n_cg1.w.l = 0;
				n_cb1.w.h = 0x80; n_cb1.w.l = 0;
				break;
			}
			n_cu1.w.h = TEXTURE_U( p_psxgpu->m_packet.GouraudTexturedPolygon.vertex[ n_leftpoint ].n_texture ); n_cu1.w.l = 0;
			n_cv1.w.h = TEXTURE_V( p_psxgpu->m_packet.GouraudTexturedPolygon.vertex[ n_leftpoint ].n_texture ); n_cv1.w.l = 0;
			n_leftpoint = p_n_leftpointlist[ n_leftpoint ];
			n_distance = COORD_Y( p_psxgpu->m_packet.GouraudTexturedPolygon.vertex[ n_leftpoint ].n_coord ) - n_y;
			if( n_distance < 1 )
			{
				break;
			}
			n_dx1 = (INT32)( ( COORD_X( p_psxgpu->m_packet.GouraudTexturedPolygon.vertex[ n_leftpoint ].n_coord ) << 16 ) - n_cx1.d ) / n_distance;
			switch( n_cmd & 0x01 )
			{
			case 0x00:
				n_dr1 = (INT32)( ( BGR_R( p_psxgpu->m_packet.GouraudTexturedPolygon.vertex[ n_leftpoint ].n_bgr ) << 16 ) - n_cr1.d ) / n_distance;
				n_dg1 = (INT32)( ( BGR_G( p_psxgpu->m_packet.GouraudTexturedPolygon.vertex[ n_leftpoint ].n_bgr ) << 16 ) - n_cg1.d ) / n_distance;
				n_db1 = (INT32)( ( BGR_B( p_psxgpu->m_packet.GouraudTexturedPolygon.vertex[ n_leftpoint ].n_bgr ) << 16 ) - n_cb1.d ) / n_distance;
				break;
			case 0x01:
				n_dr1 = 0;
				n_dg1 = 0;
				n_db1 = 0;
				break;
			}
			n_du1 = (INT32)( ( TEXTURE_U( p_psxgpu->m_packet.GouraudTexturedPolygon.vertex[ n_leftpoint ].n_texture ) << 16 ) - n_cu1.d ) / n_distance;
			n_dv1 = (INT32)( ( TEXTURE_V( p_psxgpu->m_packet.GouraudTexturedPolygon.vertex[ n_leftpoint ].n_texture ) << 16 ) - n_cv1.d ) / n_distance;
		}
		if( n_y == COORD_Y( p_psxgpu->m_packet.GouraudTexturedPolygon.vertex[ n_rightpoint ].n_coord ) )
		{
			while( n_y == COORD_Y( p_psxgpu->m_packet.GouraudTexturedPolygon.vertex[ p_n_rightpointlist[ n_rightpoint ] ].n_coord ) )
			{
				n_rightpoint = p_n_rightpointlist[ n_rightpoint ];
				if( n_rightpoint == n_leftpoint )
				{
					break;
				}
			}
			n_cx2.w.h = COORD_X( p_psxgpu->m_packet.GouraudTexturedPolygon.vertex[ n_rightpoint ].n_coord ); n_cx2.w.l = 0;
			switch( n_cmd & 0x01 )
			{
			case 0x00:
				n_cr2.w.h = BGR_R( p_psxgpu->m_packet.GouraudTexturedPolygon.vertex[ n_rightpoint ].n_bgr ); n_cr2.w.l = 0;
				n_cg2.w.h = BGR_G( p_psxgpu->m_packet.GouraudTexturedPolygon.vertex[ n_rightpoint ].n_bgr ); n_cg2.w.l = 0;
				n_cb2.w.h = BGR_B( p_psxgpu->m_packet.GouraudTexturedPolygon.vertex[ n_rightpoint ].n_bgr ); n_cb2.w.l = 0;
				break;
			case 0x01:
				n_cr2.w.h = 0x80; n_cr2.w.l = 0;
				n_cg2.w.h = 0x80; n_cg2.w.l = 0;
				n_cb2.w.h = 0x80; n_cb2.w.l = 0;
				break;
			}
			n_cu2.w.h = TEXTURE_U( p_psxgpu->m_packet.GouraudTexturedPolygon.vertex[ n_rightpoint ].n_texture ); n_cu2.w.l = 0;
			n_cv2.w.h = TEXTURE_V( p_psxgpu->m_packet.GouraudTexturedPolygon.vertex[ n_rightpoint ].n_texture ); n_cv2.w.l = 0;
			n_rightpoint = p_n_rightpointlist[ n_rightpoint ];
			n_distance = COORD_Y( p_psxgpu->m_packet.GouraudTexturedPolygon.vertex[ n_rightpoint ].n_coord ) - n_y;
			if( n_distance < 1 )
			{
				break;
			}
			n_dx2 = (INT32)( ( COORD_X( p_psxgpu->m_packet.GouraudTexturedPolygon.vertex[ n_rightpoint ].n_coord ) << 16 ) - n_cx2.d ) / n_distance;
			switch( n_cmd & 0x01 )
			{
			case 0x00:
				n_dr2 = (INT32)( ( BGR_R( p_psxgpu->m_packet.GouraudTexturedPolygon.vertex[ n_rightpoint ].n_bgr ) << 16 ) - n_cr2.d ) / n_distance;
				n_dg2 = (INT32)( ( BGR_G( p_psxgpu->m_packet.GouraudTexturedPolygon.vertex[ n_rightpoint ].n_bgr ) << 16 ) - n_cg2.d ) / n_distance;
				n_db2 = (INT32)( ( BGR_B( p_psxgpu->m_packet.GouraudTexturedPolygon.vertex[ n_rightpoint ].n_bgr ) << 16 ) - n_cb2.d ) / n_distance;
				break;
			case 0x01:
				n_dr2 = 0;
				n_dg2 = 0;
				n_db2 = 0;
				break;
			}
			n_du2 = (INT32)( ( TEXTURE_U( p_psxgpu->m_packet.GouraudTexturedPolygon.vertex[ n_rightpoint ].n_texture ) << 16 ) - n_cu2.d ) / n_distance;
			n_dv2 = (INT32)( ( TEXTURE_V( p_psxgpu->m_packet.GouraudTexturedPolygon.vertex[ n_rightpoint ].n_texture ) << 16 ) - n_cv2.d ) / n_distance;
		}
		if( (INT16)n_cx1.w.h != (INT16)n_cx2.w.h && n_y >= (INT32)p_psxgpu->n_drawarea_y1 && n_y <= (INT32)p_psxgpu->n_drawarea_y2 )
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

			if( ( (INT32)p_psxgpu->n_drawarea_x1 - n_x ) > 0 )
			{
				n_r.d += n_dr * ( p_psxgpu->n_drawarea_x1 - n_x );
				n_g.d += n_dg * ( p_psxgpu->n_drawarea_x1 - n_x );
				n_b.d += n_db * ( p_psxgpu->n_drawarea_x1 - n_x );
				n_u.d += n_du * ( p_psxgpu->n_drawarea_x1 - n_x );
				n_v.d += n_dv * ( p_psxgpu->n_drawarea_x1 - n_x );
				n_distance -= ( p_psxgpu->n_drawarea_x1 - n_x );
				n_x = p_psxgpu->n_drawarea_x1;
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

static void MonochromeLine( psx_gpu *p_psxgpu )
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
	if( p_psxgpu->m_debug.n_skip == 5 )
	{
		return;
	}
	DebugMesh( p_psxgpu, COORD_X( p_psxgpu->m_packet.MonochromeLine.vertex[ 0 ].n_coord ) + p_psxgpu->n_drawoffset_x, COORD_Y( p_psxgpu->m_packet.MonochromeLine.vertex[ 0 ].n_coord ) + p_psxgpu->n_drawoffset_y );
	DebugMesh( p_psxgpu, COORD_X( p_psxgpu->m_packet.MonochromeLine.vertex[ 1 ].n_coord ) + p_psxgpu->n_drawoffset_x, COORD_Y( p_psxgpu->m_packet.MonochromeLine.vertex[ 1 ].n_coord ) + p_psxgpu->n_drawoffset_y );
	DebugMeshEnd(p_psxgpu);
#endif

	n_xstart = COORD_X( p_psxgpu->m_packet.MonochromeLine.vertex[ 0 ].n_coord ) + p_psxgpu->n_drawoffset_x;
	n_xend = COORD_X( p_psxgpu->m_packet.MonochromeLine.vertex[ 1 ].n_coord ) + p_psxgpu->n_drawoffset_x;
	n_ystart = COORD_Y( p_psxgpu->m_packet.MonochromeLine.vertex[ 0 ].n_coord ) + p_psxgpu->n_drawoffset_y;
	n_yend = COORD_Y( p_psxgpu->m_packet.MonochromeLine.vertex[ 1 ].n_coord ) + p_psxgpu->n_drawoffset_y;

	n_r = BGR_R( p_psxgpu->m_packet.MonochromeLine.n_bgr );
	n_g = BGR_G( p_psxgpu->m_packet.MonochromeLine.n_bgr );
	n_b = BGR_B( p_psxgpu->m_packet.MonochromeLine.n_bgr );

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
		if( (INT16)n_x.w.h >= (INT32)p_psxgpu->n_drawarea_x1 &&
			(INT16)n_y.w.h >= (INT32)p_psxgpu->n_drawarea_y1 &&
			(INT16)n_x.w.h <= (INT32)p_psxgpu->n_drawarea_x2 &&
			(INT16)n_y.w.h <= (INT32)p_psxgpu->n_drawarea_y2 )
		{
			p_vram = p_psxgpu->p_p_vram[ n_y.w.h ] + n_x.w.h;
			WRITE_PIXEL(
				p_psxgpu->p_n_redshade[ MID_LEVEL | n_r ] |
				p_psxgpu->p_n_greenshade[ MID_LEVEL | n_g ] |
				p_psxgpu->p_n_blueshade[ MID_LEVEL | n_b ] );
		}
		n_x.d += n_dx;
		n_y.d += n_dy;
		n_len--;
	}
}

static void GouraudLine( psx_gpu *p_psxgpu )
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
	if( p_psxgpu->m_debug.n_skip == 6 )
	{
		return;
	}
	DebugMesh( p_psxgpu, COORD_X( p_psxgpu->m_packet.GouraudLine.vertex[ 0 ].n_coord ) + p_psxgpu->n_drawoffset_x, COORD_Y( p_psxgpu->m_packet.GouraudLine.vertex[ 0 ].n_coord ) + p_psxgpu->n_drawoffset_y );
	DebugMesh( p_psxgpu, COORD_X( p_psxgpu->m_packet.GouraudLine.vertex[ 1 ].n_coord ) + p_psxgpu->n_drawoffset_x, COORD_Y( p_psxgpu->m_packet.GouraudLine.vertex[ 1 ].n_coord ) + p_psxgpu->n_drawoffset_y );
	DebugMeshEnd(p_psxgpu);
#endif

	n_xstart = COORD_X( p_psxgpu->m_packet.GouraudLine.vertex[ 0 ].n_coord ) + p_psxgpu->n_drawoffset_x;
	n_ystart = COORD_Y( p_psxgpu->m_packet.GouraudLine.vertex[ 0 ].n_coord ) + p_psxgpu->n_drawoffset_y;
	n_cr1.w.h = BGR_R( p_psxgpu->m_packet.GouraudLine.vertex[ 0 ].n_bgr ); n_cr1.w.l = 0;
	n_cg1.w.h = BGR_G( p_psxgpu->m_packet.GouraudLine.vertex[ 0 ].n_bgr ); n_cg1.w.l = 0;
	n_cb1.w.h = BGR_B( p_psxgpu->m_packet.GouraudLine.vertex[ 0 ].n_bgr ); n_cb1.w.l = 0;

	n_xend = COORD_X( p_psxgpu->m_packet.GouraudLine.vertex[ 1 ].n_coord ) + p_psxgpu->n_drawoffset_x;
	n_yend = COORD_Y( p_psxgpu->m_packet.GouraudLine.vertex[ 1 ].n_coord ) + p_psxgpu->n_drawoffset_y;
	n_cr2.w.h = BGR_R( p_psxgpu->m_packet.GouraudLine.vertex[ 1 ].n_bgr ); n_cr1.w.l = 0;
	n_cg2.w.h = BGR_G( p_psxgpu->m_packet.GouraudLine.vertex[ 1 ].n_bgr ); n_cg1.w.l = 0;
	n_cb2.w.h = BGR_B( p_psxgpu->m_packet.GouraudLine.vertex[ 1 ].n_bgr ); n_cb1.w.l = 0;

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
		if( (INT16)n_x.w.h >= (INT32)p_psxgpu->n_drawarea_x1 &&
			(INT16)n_y.w.h >= (INT32)p_psxgpu->n_drawarea_y1 &&
			(INT16)n_x.w.h <= (INT32)p_psxgpu->n_drawarea_x2 &&
			(INT16)n_y.w.h <= (INT32)p_psxgpu->n_drawarea_y2 )
		{
			p_vram = p_psxgpu->p_p_vram[ n_y.w.h ] + n_x.w.h;
			WRITE_PIXEL(
				p_psxgpu->p_n_redshade[ MID_LEVEL | n_r.w.h ] |
				p_psxgpu->p_n_greenshade[ MID_LEVEL | n_g.w.h ] |
				p_psxgpu->p_n_blueshade[ MID_LEVEL | n_b.w.h ] );
		}
		n_x.d += n_dx;
		n_y.d += n_dy;
		n_r.d += n_dr;
		n_g.d += n_dg;
		n_b.d += n_db;
		n_distance--;
	}
}

static void FrameBufferRectangleDraw( psx_gpu *p_psxgpu )
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
	if( p_psxgpu->m_debug.n_skip == 7 )
	{
		return;
	}
	DebugMesh( p_psxgpu, COORD_X( p_psxgpu->m_packet.FlatRectangle.n_coord ), COORD_Y( p_psxgpu->m_packet.FlatRectangle.n_coord ) );
	DebugMesh( p_psxgpu, COORD_X( p_psxgpu->m_packet.FlatRectangle.n_coord ) + SIZE_W( p_psxgpu->m_packet.FlatRectangle.n_size ), COORD_Y( p_psxgpu->m_packet.FlatRectangle.n_coord ) );
	DebugMesh( p_psxgpu, COORD_X( p_psxgpu->m_packet.FlatRectangle.n_coord ), COORD_Y( p_psxgpu->m_packet.FlatRectangle.n_coord ) + SIZE_H( p_psxgpu->m_packet.FlatRectangle.n_size ) );
	DebugMesh( p_psxgpu, COORD_X( p_psxgpu->m_packet.FlatRectangle.n_coord ) + SIZE_W( p_psxgpu->m_packet.FlatRectangle.n_size ), COORD_Y( p_psxgpu->m_packet.FlatRectangle.n_coord ) + SIZE_H( p_psxgpu->m_packet.FlatRectangle.n_size ) );
	DebugMeshEnd(p_psxgpu);
#endif

	n_r.w.h = BGR_R( p_psxgpu->m_packet.FlatRectangle.n_bgr ); n_r.w.l = 0;
	n_g.w.h = BGR_G( p_psxgpu->m_packet.FlatRectangle.n_bgr ); n_g.w.l = 0;
	n_b.w.h = BGR_B( p_psxgpu->m_packet.FlatRectangle.n_bgr ); n_b.w.l = 0;

	n_y = COORD_Y( p_psxgpu->m_packet.FlatRectangle.n_coord );
	n_h = SIZE_H( p_psxgpu->m_packet.FlatRectangle.n_size );

	while( n_h > 0 )
	{
		n_x = COORD_X( p_psxgpu->m_packet.FlatRectangle.n_coord );

		n_distance = SIZE_W( p_psxgpu->m_packet.FlatRectangle.n_size );
		while( n_distance > 0 )
		{
			p_vram = p_psxgpu->p_p_vram[ n_y & 1023 ] + ( n_x & 1023 );
			WRITE_PIXEL(
				p_psxgpu->p_n_redshade[ MID_LEVEL | n_r.w.h ] |
				p_psxgpu->p_n_greenshade[ MID_LEVEL | n_g.w.h ] |
				p_psxgpu->p_n_blueshade[ MID_LEVEL | n_b.w.h ] );
			n_x++;
			n_distance--;
		}
		n_y++;
		n_h--;
	}
}

static void FlatRectangle( psx_gpu *p_psxgpu )
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
	if( p_psxgpu->m_debug.n_skip == 8 )
	{
		return;
	}
	DebugMesh( p_psxgpu, COORD_X( p_psxgpu->m_packet.FlatRectangle.n_coord ) + p_psxgpu->n_drawoffset_x, COORD_Y( p_psxgpu->m_packet.FlatRectangle.n_coord ) + p_psxgpu->n_drawoffset_y );
	DebugMesh( p_psxgpu, COORD_X( p_psxgpu->m_packet.FlatRectangle.n_coord ) + p_psxgpu->n_drawoffset_x + SIZE_W( p_psxgpu->m_packet.FlatRectangle.n_size ), COORD_Y( p_psxgpu->m_packet.FlatRectangle.n_coord ) + p_psxgpu->n_drawoffset_y );
	DebugMesh( p_psxgpu, COORD_X( p_psxgpu->m_packet.FlatRectangle.n_coord ) + p_psxgpu->n_drawoffset_x, COORD_Y( p_psxgpu->m_packet.FlatRectangle.n_coord ) + p_psxgpu->n_drawoffset_y + SIZE_H( p_psxgpu->m_packet.FlatRectangle.n_size ) );
	DebugMesh( p_psxgpu, COORD_X( p_psxgpu->m_packet.FlatRectangle.n_coord ) + p_psxgpu->n_drawoffset_x + SIZE_W( p_psxgpu->m_packet.FlatRectangle.n_size ), COORD_Y( p_psxgpu->m_packet.FlatRectangle.n_coord ) + p_psxgpu->n_drawoffset_y + SIZE_H( p_psxgpu->m_packet.FlatRectangle.n_size ) );
	DebugMeshEnd(p_psxgpu);
#endif

	n_cmd = BGR_C( p_psxgpu->m_packet.FlatRectangle.n_bgr );

	SOLIDSETUP

	n_r.w.h = BGR_R( p_psxgpu->m_packet.FlatRectangle.n_bgr ); n_r.w.l = 0;
	n_g.w.h = BGR_G( p_psxgpu->m_packet.FlatRectangle.n_bgr ); n_g.w.l = 0;
	n_b.w.h = BGR_B( p_psxgpu->m_packet.FlatRectangle.n_bgr ); n_b.w.l = 0;

	n_y = COORD_Y( p_psxgpu->m_packet.FlatRectangle.n_coord ) + p_psxgpu->n_drawoffset_y;
	n_h = SIZE_H( p_psxgpu->m_packet.FlatRectangle.n_size );

	while( n_h > 0 )
	{
		n_x = COORD_X( p_psxgpu->m_packet.FlatRectangle.n_coord ) + p_psxgpu->n_drawoffset_x;

		n_distance = SIZE_W( p_psxgpu->m_packet.FlatRectangle.n_size );
		if( n_distance > 0 && n_y >= (INT32)p_psxgpu->n_drawarea_y1 && n_y <= (INT32)p_psxgpu->n_drawarea_y2 )
		{
			if( ( (INT32)p_psxgpu->n_drawarea_x1 - n_x ) > 0 )
			{
				n_distance -= ( p_psxgpu->n_drawarea_x1 - n_x );
				n_x = p_psxgpu->n_drawarea_x1;
			}
			SOLIDFILL( FLATRECTANGEUPDATE )
		}
		n_y++;
		n_h--;
	}
}

static void FlatRectangle8x8( psx_gpu *p_psxgpu )
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
	if( p_psxgpu->m_debug.n_skip == 9 )
	{
		return;
	}
	DebugMesh( p_psxgpu, COORD_X( p_psxgpu->m_packet.FlatRectangle8x8.n_coord ) + p_psxgpu->n_drawoffset_x, COORD_Y( p_psxgpu->m_packet.FlatRectangle8x8.n_coord ) + p_psxgpu->n_drawoffset_y );
	DebugMesh( p_psxgpu, COORD_X( p_psxgpu->m_packet.FlatRectangle8x8.n_coord ) + p_psxgpu->n_drawoffset_x + 8, COORD_Y( p_psxgpu->m_packet.FlatRectangle8x8.n_coord ) + p_psxgpu->n_drawoffset_y );
	DebugMesh( p_psxgpu, COORD_X( p_psxgpu->m_packet.FlatRectangle8x8.n_coord ) + p_psxgpu->n_drawoffset_x, COORD_Y( p_psxgpu->m_packet.FlatRectangle8x8.n_coord ) + p_psxgpu->n_drawoffset_y + 8 );
	DebugMesh( p_psxgpu, COORD_X( p_psxgpu->m_packet.FlatRectangle8x8.n_coord ) + p_psxgpu->n_drawoffset_x + 8, COORD_Y( p_psxgpu->m_packet.FlatRectangle8x8.n_coord ) + p_psxgpu->n_drawoffset_y + 8 );
	DebugMeshEnd(p_psxgpu);
#endif

	n_cmd = BGR_C( p_psxgpu->m_packet.FlatRectangle8x8.n_bgr );

	SOLIDSETUP

	n_r.w.h = BGR_R( p_psxgpu->m_packet.FlatRectangle8x8.n_bgr ); n_r.w.l = 0;
	n_g.w.h = BGR_G( p_psxgpu->m_packet.FlatRectangle8x8.n_bgr ); n_g.w.l = 0;
	n_b.w.h = BGR_B( p_psxgpu->m_packet.FlatRectangle8x8.n_bgr ); n_b.w.l = 0;

	n_y = COORD_Y( p_psxgpu->m_packet.FlatRectangle8x8.n_coord ) + p_psxgpu->n_drawoffset_y;
	n_h = 8;

	while( n_h > 0 )
	{
		n_x = COORD_X( p_psxgpu->m_packet.FlatRectangle8x8.n_coord ) + p_psxgpu->n_drawoffset_x;

		n_distance = 8;
		if( n_distance > 0 && n_y >= (INT32)p_psxgpu->n_drawarea_y1 && n_y <= (INT32)p_psxgpu->n_drawarea_y2 )
		{
			if( ( (INT32)p_psxgpu->n_drawarea_x1 - n_x ) > 0 )
			{
				n_distance -= ( p_psxgpu->n_drawarea_x1 - n_x );
				n_x = p_psxgpu->n_drawarea_x1;
			}
			SOLIDFILL( FLATRECTANGEUPDATE )
		}
		n_y++;
		n_h--;
	}
}

static void FlatRectangle16x16( psx_gpu *p_psxgpu )
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
	if( p_psxgpu->m_debug.n_skip == 10 )
	{
		return;
	}
	DebugMesh( p_psxgpu, COORD_X( p_psxgpu->m_packet.FlatRectangle16x16.n_coord ) + p_psxgpu->n_drawoffset_x, COORD_Y( p_psxgpu->m_packet.FlatRectangle16x16.n_coord ) + p_psxgpu->n_drawoffset_y );
	DebugMesh( p_psxgpu, COORD_X( p_psxgpu->m_packet.FlatRectangle16x16.n_coord ) + p_psxgpu->n_drawoffset_x + 16, COORD_Y( p_psxgpu->m_packet.FlatRectangle16x16.n_coord ) + p_psxgpu->n_drawoffset_y );
	DebugMesh( p_psxgpu, COORD_X( p_psxgpu->m_packet.FlatRectangle16x16.n_coord ) + p_psxgpu->n_drawoffset_x, COORD_Y( p_psxgpu->m_packet.FlatRectangle16x16.n_coord ) + p_psxgpu->n_drawoffset_y + 16 );
	DebugMesh( p_psxgpu, COORD_X( p_psxgpu->m_packet.FlatRectangle16x16.n_coord ) + p_psxgpu->n_drawoffset_x + 16, COORD_Y( p_psxgpu->m_packet.FlatRectangle16x16.n_coord ) + p_psxgpu->n_drawoffset_y + 16 );
	DebugMeshEnd(p_psxgpu);
#endif

	n_cmd = BGR_C( p_psxgpu->m_packet.FlatRectangle16x16.n_bgr );

	SOLIDSETUP

	n_r.w.h = BGR_R( p_psxgpu->m_packet.FlatRectangle16x16.n_bgr ); n_r.w.l = 0;
	n_g.w.h = BGR_G( p_psxgpu->m_packet.FlatRectangle16x16.n_bgr ); n_g.w.l = 0;
	n_b.w.h = BGR_B( p_psxgpu->m_packet.FlatRectangle16x16.n_bgr ); n_b.w.l = 0;

	n_y = COORD_Y( p_psxgpu->m_packet.FlatRectangle16x16.n_coord ) + p_psxgpu->n_drawoffset_y;
	n_h = 16;

	while( n_h > 0 )
	{
		n_x = COORD_X( p_psxgpu->m_packet.FlatRectangle16x16.n_coord ) + p_psxgpu->n_drawoffset_x;

		n_distance = 16;
		if( n_distance > 0 && n_y >= (INT32)p_psxgpu->n_drawarea_y1 && n_y <= (INT32)p_psxgpu->n_drawarea_y2 )
		{
			if( ( (INT32)p_psxgpu->n_drawarea_x1 - n_x ) > 0 )
			{
				n_distance -= ( p_psxgpu->n_drawarea_x1 - n_x );
				n_x = p_psxgpu->n_drawarea_x1;
			}
			SOLIDFILL( FLATRECTANGEUPDATE )
		}
		n_y++;
		n_h--;
	}
}

static void FlatTexturedRectangle( psx_gpu *p_psxgpu )
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
	if( p_psxgpu->m_debug.n_skip == 11 )
	{
		return;
	}
	DebugMesh( p_psxgpu, COORD_X( p_psxgpu->m_packet.FlatTexturedRectangle.n_coord ) + p_psxgpu->n_drawoffset_x, COORD_Y( p_psxgpu->m_packet.FlatTexturedRectangle.n_coord ) + p_psxgpu->n_drawoffset_y );
	DebugMesh( p_psxgpu, COORD_X( p_psxgpu->m_packet.FlatTexturedRectangle.n_coord ) + p_psxgpu->n_drawoffset_x + SIZE_W( p_psxgpu->m_packet.FlatTexturedRectangle.n_size ), COORD_Y( p_psxgpu->m_packet.FlatTexturedRectangle.n_coord ) + p_psxgpu->n_drawoffset_y );
	DebugMesh( p_psxgpu, COORD_X( p_psxgpu->m_packet.FlatTexturedRectangle.n_coord ) + p_psxgpu->n_drawoffset_x, COORD_Y( p_psxgpu->m_packet.FlatTexturedRectangle.n_coord ) + p_psxgpu->n_drawoffset_y + SIZE_H( p_psxgpu->m_packet.FlatTexturedRectangle.n_size ) );
	DebugMesh( p_psxgpu, COORD_X( p_psxgpu->m_packet.FlatTexturedRectangle.n_coord ) + p_psxgpu->n_drawoffset_x + SIZE_W( p_psxgpu->m_packet.FlatTexturedRectangle.n_size ), COORD_Y( p_psxgpu->m_packet.FlatTexturedRectangle.n_coord ) + p_psxgpu->n_drawoffset_y + SIZE_H( p_psxgpu->m_packet.FlatTexturedRectangle.n_size ) );
	DebugMeshEnd(p_psxgpu);
#endif

	n_cmd = BGR_C( p_psxgpu->m_packet.FlatTexturedRectangle.n_bgr );

	n_clutx = ( p_psxgpu->m_packet.FlatTexturedRectangle.n_texture.w.h & 0x3f ) << 4;
	n_cluty = ( p_psxgpu->m_packet.FlatTexturedRectangle.n_texture.w.h >> 6 ) & 0x3ff;

	n_r.d = 0;
	n_g.d = 0;
	n_b.d = 0;

	TEXTURESETUP
	SPRITESETUP

	switch( n_cmd & 0x01 )
	{
	case 0:
		n_r.w.h = BGR_R( p_psxgpu->m_packet.FlatTexturedRectangle.n_bgr ); n_r.w.l = 0;
		n_g.w.h = BGR_G( p_psxgpu->m_packet.FlatTexturedRectangle.n_bgr ); n_g.w.l = 0;
		n_b.w.h = BGR_B( p_psxgpu->m_packet.FlatTexturedRectangle.n_bgr ); n_b.w.l = 0;
		break;
	case 1:
		n_r.w.h = 0x80; n_r.w.l = 0;
		n_g.w.h = 0x80; n_g.w.l = 0;
		n_b.w.h = 0x80; n_b.w.l = 0;
		break;
	}

	n_v = TEXTURE_V( p_psxgpu->m_packet.FlatTexturedRectangle.n_texture );
	n_y = COORD_Y( p_psxgpu->m_packet.FlatTexturedRectangle.n_coord ) + p_psxgpu->n_drawoffset_y;
	n_h = SIZE_H( p_psxgpu->m_packet.FlatTexturedRectangle.n_size );

	while( n_h > 0 )
	{
		n_x = COORD_X( p_psxgpu->m_packet.FlatTexturedRectangle.n_coord ) + p_psxgpu->n_drawoffset_x;
		n_u = TEXTURE_U( p_psxgpu->m_packet.FlatTexturedRectangle.n_texture );

		n_distance = SIZE_W( p_psxgpu->m_packet.FlatTexturedRectangle.n_size );
		if( n_distance > 0 && n_y >= (INT32)p_psxgpu->n_drawarea_y1 && n_y <= (INT32)p_psxgpu->n_drawarea_y2 )
		{
			if( ( (INT32)p_psxgpu->n_drawarea_x1 - n_x ) > 0 )
			{
				n_u += ( p_psxgpu->n_drawarea_x1 - n_x ) * n_du;
				n_distance -= ( p_psxgpu->n_drawarea_x1 - n_x );
				n_x = p_psxgpu->n_drawarea_x1;
			}
			TEXTUREFILL( FLATTEXTUREDRECTANGLEUPDATE, n_u, n_v );
		}
		n_v += n_dv;
		n_y++;
		n_h--;
	}
}

static void Sprite8x8( psx_gpu *p_psxgpu )
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
	if( p_psxgpu->m_debug.n_skip == 12 )
	{
		return;
	}
	DebugMesh( p_psxgpu, COORD_X( p_psxgpu->m_packet.Sprite8x8.n_coord ) + p_psxgpu->n_drawoffset_x, COORD_Y( p_psxgpu->m_packet.Sprite8x8.n_coord ) + p_psxgpu->n_drawoffset_y );
	DebugMesh( p_psxgpu, COORD_X( p_psxgpu->m_packet.Sprite8x8.n_coord ) + p_psxgpu->n_drawoffset_x + 7, COORD_Y( p_psxgpu->m_packet.Sprite8x8.n_coord ) + p_psxgpu->n_drawoffset_y );
	DebugMesh( p_psxgpu, COORD_X( p_psxgpu->m_packet.Sprite8x8.n_coord ) + p_psxgpu->n_drawoffset_x, COORD_Y( p_psxgpu->m_packet.Sprite8x8.n_coord ) + p_psxgpu->n_drawoffset_y + 7 );
	DebugMesh( p_psxgpu, COORD_X( p_psxgpu->m_packet.Sprite8x8.n_coord ) + p_psxgpu->n_drawoffset_x + 7, COORD_Y( p_psxgpu->m_packet.Sprite8x8.n_coord ) + p_psxgpu->n_drawoffset_y + 7 );
	DebugMeshEnd(p_psxgpu);
#endif

	n_cmd = BGR_C( p_psxgpu->m_packet.Sprite8x8.n_bgr );

	n_clutx = ( p_psxgpu->m_packet.Sprite8x8.n_texture.w.h & 0x3f ) << 4;
	n_cluty = ( p_psxgpu->m_packet.Sprite8x8.n_texture.w.h >> 6 ) & 0x3ff;

	n_r.d = 0;
	n_g.d = 0;
	n_b.d = 0;

	TEXTURESETUP
	SPRITESETUP

	switch( n_cmd & 0x01 )
	{
	case 0:
		n_r.w.h = BGR_R( p_psxgpu->m_packet.Sprite8x8.n_bgr ); n_r.w.l = 0;
		n_g.w.h = BGR_G( p_psxgpu->m_packet.Sprite8x8.n_bgr ); n_g.w.l = 0;
		n_b.w.h = BGR_B( p_psxgpu->m_packet.Sprite8x8.n_bgr ); n_b.w.l = 0;
		break;
	case 1:
		n_r.w.h = 0x80; n_r.w.l = 0;
		n_g.w.h = 0x80; n_g.w.l = 0;
		n_b.w.h = 0x80; n_b.w.l = 0;
		break;
	}

	n_v = TEXTURE_V( p_psxgpu->m_packet.Sprite8x8.n_texture );
	n_y = COORD_Y( p_psxgpu->m_packet.Sprite8x8.n_coord ) + p_psxgpu->n_drawoffset_y;
	n_h = 8;

	while( n_h > 0 )
	{
		n_x = COORD_X( p_psxgpu->m_packet.Sprite8x8.n_coord ) + p_psxgpu->n_drawoffset_x;
		n_u = TEXTURE_U( p_psxgpu->m_packet.Sprite8x8.n_texture );

		n_distance = 8;
		if( n_distance > 0 && n_y >= (INT32)p_psxgpu->n_drawarea_y1 && n_y <= (INT32)p_psxgpu->n_drawarea_y2 )
		{
			if( ( (INT32)p_psxgpu->n_drawarea_x1 - n_x ) > 0 )
			{
				n_u += ( p_psxgpu->n_drawarea_x1 - n_x ) * n_du;
				n_distance -= ( p_psxgpu->n_drawarea_x1 - n_x );
				n_x = p_psxgpu->n_drawarea_x1;
			}
			TEXTUREFILL( FLATTEXTUREDRECTANGLEUPDATE, n_u, n_v );
		}
		n_v += n_dv;
		n_y++;
		n_h--;
	}
}

static void Sprite16x16( psx_gpu *p_psxgpu )
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
	if( p_psxgpu->m_debug.n_skip == 13 )
	{
		return;
	}
	DebugMesh( p_psxgpu, COORD_X( p_psxgpu->m_packet.Sprite16x16.n_coord ) + p_psxgpu->n_drawoffset_x, COORD_Y( p_psxgpu->m_packet.Sprite16x16.n_coord ) + p_psxgpu->n_drawoffset_y );
	DebugMesh( p_psxgpu, COORD_X( p_psxgpu->m_packet.Sprite16x16.n_coord ) + p_psxgpu->n_drawoffset_x + 7, COORD_Y( p_psxgpu->m_packet.Sprite16x16.n_coord ) + p_psxgpu->n_drawoffset_y );
	DebugMesh( p_psxgpu, COORD_X( p_psxgpu->m_packet.Sprite16x16.n_coord ) + p_psxgpu->n_drawoffset_x, COORD_Y( p_psxgpu->m_packet.Sprite16x16.n_coord ) + p_psxgpu->n_drawoffset_y + 7 );
	DebugMesh( p_psxgpu, COORD_X( p_psxgpu->m_packet.Sprite16x16.n_coord ) + p_psxgpu->n_drawoffset_x + 7, COORD_Y( p_psxgpu->m_packet.Sprite16x16.n_coord ) + p_psxgpu->n_drawoffset_y + 7 );
	DebugMeshEnd(p_psxgpu);
#endif

	n_cmd = BGR_C( p_psxgpu->m_packet.Sprite16x16.n_bgr );

	n_clutx = ( p_psxgpu->m_packet.Sprite16x16.n_texture.w.h & 0x3f ) << 4;
	n_cluty = ( p_psxgpu->m_packet.Sprite16x16.n_texture.w.h >> 6 ) & 0x3ff;

	n_r.d = 0;
	n_g.d = 0;
	n_b.d = 0;

	TEXTURESETUP
	SPRITESETUP

	switch( n_cmd & 0x01 )
	{
	case 0:
		n_r.w.h = BGR_R( p_psxgpu->m_packet.Sprite16x16.n_bgr ); n_r.w.l = 0;
		n_g.w.h = BGR_G( p_psxgpu->m_packet.Sprite16x16.n_bgr ); n_g.w.l = 0;
		n_b.w.h = BGR_B( p_psxgpu->m_packet.Sprite16x16.n_bgr ); n_b.w.l = 0;
		break;
	case 1:
		n_r.w.h = 0x80; n_r.w.l = 0;
		n_g.w.h = 0x80; n_g.w.l = 0;
		n_b.w.h = 0x80; n_b.w.l = 0;
		break;
	}

	n_v = TEXTURE_V( p_psxgpu->m_packet.Sprite16x16.n_texture );
	n_y = COORD_Y( p_psxgpu->m_packet.Sprite16x16.n_coord ) + p_psxgpu->n_drawoffset_y;
	n_h = 16;

	while( n_h > 0 )
	{
		n_x = COORD_X( p_psxgpu->m_packet.Sprite16x16.n_coord ) + p_psxgpu->n_drawoffset_x;
		n_u = TEXTURE_U( p_psxgpu->m_packet.Sprite16x16.n_texture );

		n_distance = 16;
		if( n_distance > 0 && n_y >= (INT32)p_psxgpu->n_drawarea_y1 && n_y <= (INT32)p_psxgpu->n_drawarea_y2 )
		{
			if( ( (INT32)p_psxgpu->n_drawarea_x1 - n_x ) > 0 )
			{
				n_u += ( p_psxgpu->n_drawarea_x1 - n_x ) * n_du;
				n_distance -= ( p_psxgpu->n_drawarea_x1 - n_x );
				n_x = p_psxgpu->n_drawarea_x1;
			}
			TEXTUREFILL( FLATTEXTUREDRECTANGLEUPDATE, n_u, n_v );
		}
		n_v += n_dv;
		n_y++;
		n_h--;
	}
}

static void Dot( psx_gpu *p_psxgpu )
{
	INT32 n_x;
	INT32 n_y;
	UINT32 n_r;
	UINT32 n_g;
	UINT32 n_b;
	UINT16 *p_vram;

#if defined( MAME_DEBUG )
	if( p_psxgpu->m_debug.n_skip == 14 )
	{
		return;
	}
	DebugMesh( p_psxgpu, COORD_X( p_psxgpu->m_packet.Dot.vertex.n_coord ) + p_psxgpu->n_drawoffset_x, COORD_Y( p_psxgpu->m_packet.Dot.vertex.n_coord ) + p_psxgpu->n_drawoffset_y );
	DebugMeshEnd(p_psxgpu);
#endif

	n_r = BGR_R( p_psxgpu->m_packet.Dot.n_bgr );
	n_g = BGR_G( p_psxgpu->m_packet.Dot.n_bgr );
	n_b = BGR_B( p_psxgpu->m_packet.Dot.n_bgr );
	n_x = COORD_X( p_psxgpu->m_packet.Dot.vertex.n_coord ) + p_psxgpu->n_drawoffset_x;
	n_y = COORD_Y( p_psxgpu->m_packet.Dot.vertex.n_coord ) + p_psxgpu->n_drawoffset_y;

	if( (INT16)n_x >= (INT32)p_psxgpu->n_drawarea_x1 &&
		(INT16)n_y >= (INT32)p_psxgpu->n_drawarea_y1 &&
		(INT16)n_x <= (INT32)p_psxgpu->n_drawarea_x2 &&
		(INT16)n_y <= (INT32)p_psxgpu->n_drawarea_y2 )
	{
		p_vram = p_psxgpu->p_p_vram[ n_y ] + n_x;
		WRITE_PIXEL(
			p_psxgpu->p_n_redshade[ MID_LEVEL | n_r ] |
			p_psxgpu->p_n_greenshade[ MID_LEVEL | n_g ] |
			p_psxgpu->p_n_blueshade[ MID_LEVEL | n_b ] );
	}
}

static void MoveImage( psx_gpu *p_psxgpu )
{
	INT16 n_w;
	INT16 n_h;
	INT16 n_srcx;
	INT16 n_srcy;
	INT16 n_dsty;
	INT16 n_dstx;
	UINT16 *p_vram;

#if defined( MAME_DEBUG )
	if( p_psxgpu->m_debug.n_skip == 15 )
	{
		return;
	}
	DebugMesh( p_psxgpu, COORD_X( p_psxgpu->m_packet.MoveImage.vertex[ 1 ].n_coord ), COORD_Y( p_psxgpu->m_packet.MoveImage.vertex[ 1 ].n_coord ) );
	DebugMesh( p_psxgpu, COORD_X( p_psxgpu->m_packet.MoveImage.vertex[ 1 ].n_coord ) + SIZE_W( p_psxgpu->m_packet.MoveImage.n_size ), COORD_Y( p_psxgpu->m_packet.MoveImage.vertex[ 1 ].n_coord ) );
	DebugMesh( p_psxgpu, COORD_X( p_psxgpu->m_packet.MoveImage.vertex[ 1 ].n_coord ), COORD_Y( p_psxgpu->m_packet.MoveImage.vertex[ 1 ].n_coord ) + SIZE_H( p_psxgpu->m_packet.MoveImage.n_size ) );
	DebugMesh( p_psxgpu, COORD_X( p_psxgpu->m_packet.MoveImage.vertex[ 1 ].n_coord ) + SIZE_W( p_psxgpu->m_packet.MoveImage.n_size ), COORD_Y( p_psxgpu->m_packet.MoveImage.vertex[ 1 ].n_coord ) + SIZE_H( p_psxgpu->m_packet.MoveImage.n_size ) );
	DebugMeshEnd(p_psxgpu);
#endif

	n_srcy = COORD_Y( p_psxgpu->m_packet.MoveImage.vertex[ 0 ].n_coord );
	n_dsty = COORD_Y( p_psxgpu->m_packet.MoveImage.vertex[ 1 ].n_coord );
	n_h = SIZE_H( p_psxgpu->m_packet.MoveImage.n_size );

	while( n_h > 0 )
	{
		n_srcx = COORD_X( p_psxgpu->m_packet.MoveImage.vertex[ 0 ].n_coord );
		n_dstx = COORD_X( p_psxgpu->m_packet.MoveImage.vertex[ 1 ].n_coord );
		n_w = SIZE_W( p_psxgpu->m_packet.MoveImage.n_size );
		while( n_w > 0 )
		{
			p_vram = p_psxgpu->p_p_vram[ n_dsty & 1023 ] + ( n_dstx & 1023 );
			WRITE_PIXEL( *( p_psxgpu->p_p_vram[ n_srcy & 1023 ] + ( n_srcx & 1023 ) ) );
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
	psx_gpu *p_psxgpu = machine->driver_data<psx_state>()->p_psxgpu;

	while( n_size > 0 )
	{
		UINT32 data = *( p_ram );

		verboselog( p_psxgpu, 2, "PSX Packet #%u %08x\n", p_psxgpu->n_gpu_buffer_offset, data );
		p_psxgpu->m_packet.n_entry[ p_psxgpu->n_gpu_buffer_offset ] = data;
		switch( p_psxgpu->m_packet.n_entry[ 0 ] >> 24 )
		{
		case 0x00:
			verboselog( p_psxgpu, 1, "not handled: GPU Command 0x00: (%08x)\n", data );
			break;
		case 0x01:
			verboselog( p_psxgpu, 1, "not handled: clear cache\n" );
			break;
		case 0x02:
			if( p_psxgpu->n_gpu_buffer_offset < 2 )
			{
				p_psxgpu->n_gpu_buffer_offset++;
			}
			else
			{
				verboselog( p_psxgpu, 1, "%02x: frame buffer rectangle %u,%u %u,%u\n", p_psxgpu->m_packet.n_entry[ 0 ] >> 24,
					p_psxgpu->m_packet.n_entry[ 1 ] & 0xffff, p_psxgpu->m_packet.n_entry[ 1 ] >> 16, p_psxgpu->m_packet.n_entry[ 2 ] & 0xffff, p_psxgpu->m_packet.n_entry[ 2 ] >> 16 );
				FrameBufferRectangleDraw( p_psxgpu );
				p_psxgpu->n_gpu_buffer_offset = 0;
			}
			break;
		case 0x20:
		case 0x21:
		case 0x22:
		case 0x23:
			if( p_psxgpu->n_gpu_buffer_offset < 3 )
			{
				p_psxgpu->n_gpu_buffer_offset++;
			}
			else
			{
				verboselog( p_psxgpu, 1, "%02x: monochrome 3 point polygon\n", p_psxgpu->m_packet.n_entry[ 0 ] >> 24 );
				FlatPolygon( p_psxgpu, 3 );
				p_psxgpu->n_gpu_buffer_offset = 0;
			}
			break;
		case 0x24:
		case 0x25:
		case 0x26:
		case 0x27:
			if( p_psxgpu->n_gpu_buffer_offset < 6 )
			{
				p_psxgpu->n_gpu_buffer_offset++;
			}
			else
			{
				verboselog( p_psxgpu, 1, "%02x: textured 3 point polygon\n", p_psxgpu->m_packet.n_entry[ 0 ] >> 24 );
				FlatTexturedPolygon( p_psxgpu, 3 );
				p_psxgpu->n_gpu_buffer_offset = 0;
			}
			break;
		case 0x28:
		case 0x29:
		case 0x2a:
		case 0x2b:
			if( p_psxgpu->n_gpu_buffer_offset < 4 )
			{
				p_psxgpu->n_gpu_buffer_offset++;
			}
			else
			{
				verboselog( p_psxgpu, 1, "%02x: monochrome 4 point polygon\n", p_psxgpu->m_packet.n_entry[ 0 ] >> 24 );
				FlatPolygon( p_psxgpu, 4 );
				p_psxgpu->n_gpu_buffer_offset = 0;
			}
			break;
		case 0x2c:
		case 0x2d:
		case 0x2e:
		case 0x2f:
			if( p_psxgpu->n_gpu_buffer_offset < 8 )
			{
				p_psxgpu->n_gpu_buffer_offset++;
			}
			else
			{
				verboselog( p_psxgpu, 1, "%02x: textured 4 point polygon\n", p_psxgpu->m_packet.n_entry[ 0 ] >> 24 );
				FlatTexturedPolygon( p_psxgpu, 4 );
				p_psxgpu->n_gpu_buffer_offset = 0;
			}
			break;
		case 0x30:
		case 0x31:
		case 0x32:
		case 0x33:
			if( p_psxgpu->n_gpu_buffer_offset < 5 )
			{
				p_psxgpu->n_gpu_buffer_offset++;
			}
			else
			{
				verboselog( p_psxgpu, 1, "%02x: gouraud 3 point polygon\n", p_psxgpu->m_packet.n_entry[ 0 ] >> 24 );
				GouraudPolygon( p_psxgpu, 3 );
				p_psxgpu->n_gpu_buffer_offset = 0;
			}
			break;
		case 0x34:
		case 0x35:
		case 0x36:
		case 0x37:
			if( p_psxgpu->n_gpu_buffer_offset < 8 )
			{
				p_psxgpu->n_gpu_buffer_offset++;
			}
			else
			{
				verboselog( p_psxgpu, 1, "%02x: gouraud textured 3 point polygon\n", p_psxgpu->m_packet.n_entry[ 0 ] >> 24 );
				GouraudTexturedPolygon( p_psxgpu, 3 );
				p_psxgpu->n_gpu_buffer_offset = 0;
			}
			break;
		case 0x38:
		case 0x39:
		case 0x3a:
		case 0x3b:
			if( p_psxgpu->n_gpu_buffer_offset < 7 )
			{
				p_psxgpu->n_gpu_buffer_offset++;
			}
			else
			{
				verboselog( p_psxgpu, 1, "%02x: gouraud 4 point polygon\n", p_psxgpu->m_packet.n_entry[ 0 ] >> 24 );
				GouraudPolygon( p_psxgpu, 4 );
				p_psxgpu->n_gpu_buffer_offset = 0;
			}
			break;
		case 0x3c:
		case 0x3d:
		case 0x3e:
		case 0x3f:
			if( p_psxgpu->n_gpu_buffer_offset < 11 )
			{
				p_psxgpu->n_gpu_buffer_offset++;
			}
			else
			{
				verboselog( p_psxgpu, 1, "%02x: gouraud textured 4 point polygon\n", p_psxgpu->m_packet.n_entry[ 0 ] >> 24 );
				GouraudTexturedPolygon( p_psxgpu, 4 );
				p_psxgpu->n_gpu_buffer_offset = 0;
			}
			break;
		case 0x40:
		case 0x41:
		case 0x42:
			if( p_psxgpu->n_gpu_buffer_offset < 2 )
			{
				p_psxgpu->n_gpu_buffer_offset++;
			}
			else
			{
				verboselog( p_psxgpu, 1, "%02x: monochrome line\n", p_psxgpu->m_packet.n_entry[ 0 ] >> 24 );
				MonochromeLine( p_psxgpu );
				p_psxgpu->n_gpu_buffer_offset = 0;
			}
			break;
		case 0x48:
		case 0x4a:
		case 0x4c:
		case 0x4e:
			if( p_psxgpu->n_gpu_buffer_offset < 3 )
			{
				p_psxgpu->n_gpu_buffer_offset++;
			}
			else
			{
				verboselog( p_psxgpu, 1, "%02x: monochrome polyline\n", p_psxgpu->m_packet.n_entry[ 0 ] >> 24 );
				MonochromeLine( p_psxgpu );
				if( ( p_psxgpu->m_packet.n_entry[ 3 ] & 0xf000f000 ) != 0x50005000 )
				{
					p_psxgpu->m_packet.n_entry[ 1 ] = p_psxgpu->m_packet.n_entry[ 2 ];
					p_psxgpu->m_packet.n_entry[ 2 ] = p_psxgpu->m_packet.n_entry[ 3 ];
					p_psxgpu->n_gpu_buffer_offset = 3;
				}
				else
				{
					p_psxgpu->n_gpu_buffer_offset = 0;
				}
			}
			break;
		case 0x50:
		case 0x52:

			if( p_psxgpu->n_gpu_buffer_offset < 3 )
			{
				p_psxgpu->n_gpu_buffer_offset++;
			}
			else
			{
				verboselog( p_psxgpu, 1, "%02x: gouraud line\n", p_psxgpu->m_packet.n_entry[ 0 ] >> 24 );
				GouraudLine( p_psxgpu );
				p_psxgpu->n_gpu_buffer_offset = 0;
			}
			break;
		case 0x58:
		case 0x5a:
		case 0x5c:
		case 0x5e:
			if( p_psxgpu->n_gpu_buffer_offset < 5 &&
				( p_psxgpu->n_gpu_buffer_offset != 4 || ( p_psxgpu->m_packet.n_entry[ 4 ] & 0xf000f000 ) != 0x50005000 ) )
			{
				p_psxgpu->n_gpu_buffer_offset++;
			}
			else
			{
				verboselog( p_psxgpu, 1, "%02x: gouraud polyline\n", p_psxgpu->m_packet.n_entry[ 0 ] >> 24 );
				GouraudLine( p_psxgpu );
				if( ( p_psxgpu->m_packet.n_entry[ 4 ] & 0xf000f000 ) != 0x50005000 )
				{
					p_psxgpu->m_packet.n_entry[ 0 ] = ( p_psxgpu->m_packet.n_entry[ 0 ] & 0xff000000 ) | ( p_psxgpu->m_packet.n_entry[ 2 ] & 0x00ffffff );
					p_psxgpu->m_packet.n_entry[ 1 ] = p_psxgpu->m_packet.n_entry[ 3 ];
					p_psxgpu->m_packet.n_entry[ 2 ] = p_psxgpu->m_packet.n_entry[ 4 ];
					p_psxgpu->m_packet.n_entry[ 3 ] = p_psxgpu->m_packet.n_entry[ 5 ];
					p_psxgpu->n_gpu_buffer_offset = 4;
				}
				else
				{
					p_psxgpu->n_gpu_buffer_offset = 0;
				}
			}
			break;
		case 0x60:
		case 0x61:
		case 0x62:
		case 0x63:
			if( p_psxgpu->n_gpu_buffer_offset < 2 )
			{
				p_psxgpu->n_gpu_buffer_offset++;
			}
			else
			{
				verboselog( p_psxgpu, 1, "%02x: rectangle %d,%d %d,%d\n",
					p_psxgpu->m_packet.n_entry[ 0 ] >> 24,
					(INT16)( p_psxgpu->m_packet.n_entry[ 1 ] & 0xffff ), (INT16)( p_psxgpu->m_packet.n_entry[ 1 ] >> 16 ),
					(INT16)( p_psxgpu->m_packet.n_entry[ 2 ] & 0xffff ), (INT16)( p_psxgpu->m_packet.n_entry[ 2 ] >> 16 ) );
				FlatRectangle(p_psxgpu);
				p_psxgpu->n_gpu_buffer_offset = 0;
			}
			break;
		case 0x64:
		case 0x65:
		case 0x66:
		case 0x67:
			if( p_psxgpu->n_gpu_buffer_offset < 3 )
			{
				p_psxgpu->n_gpu_buffer_offset++;
			}
			else
			{
				verboselog( p_psxgpu, 1, "%02x: sprite %d,%d %u,%u %08x, %08x\n",
					p_psxgpu->m_packet.n_entry[ 0 ] >> 24,
					(INT16)( p_psxgpu->m_packet.n_entry[ 1 ] & 0xffff ), (INT16)( p_psxgpu->m_packet.n_entry[ 1 ] >> 16 ),
					p_psxgpu->m_packet.n_entry[ 3 ] & 0xffff, p_psxgpu->m_packet.n_entry[ 3 ] >> 16,
					p_psxgpu->m_packet.n_entry[ 0 ], p_psxgpu->m_packet.n_entry[ 2 ] );
				FlatTexturedRectangle(p_psxgpu);
				p_psxgpu->n_gpu_buffer_offset = 0;
			}
			break;
		case 0x68:
		case 0x6a:
			if( p_psxgpu->n_gpu_buffer_offset < 1 )
			{
				p_psxgpu->n_gpu_buffer_offset++;
			}
			else
			{
				verboselog( p_psxgpu, 1, "%02x: dot %d,%d %08x\n",
					p_psxgpu->m_packet.n_entry[ 0 ] >> 24,
					(INT16)( p_psxgpu->m_packet.n_entry[ 1 ] & 0xffff ), (INT16)( p_psxgpu->m_packet.n_entry[ 1 ] >> 16 ),
					p_psxgpu->m_packet.n_entry[ 0 ] & 0xffffff );
				Dot( p_psxgpu );
				p_psxgpu->n_gpu_buffer_offset = 0;
			}
			break;
		case 0x70:
		case 0x71:
			/* 8*8 rectangle */
			if( p_psxgpu->n_gpu_buffer_offset < 1 )
			{
				p_psxgpu->n_gpu_buffer_offset++;
			}
			else
			{
				verboselog( p_psxgpu, 1, "%02x: 16x16 rectangle %08x %08x\n", p_psxgpu->m_packet.n_entry[ 0 ] >> 24,
					p_psxgpu->m_packet.n_entry[ 0 ], p_psxgpu->m_packet.n_entry[ 1 ] );
				FlatRectangle8x8(p_psxgpu);
				p_psxgpu->n_gpu_buffer_offset = 0;
			}
			break;
		case 0x74:
		case 0x75:
		case 0x76:
		case 0x77:
			if( p_psxgpu->n_gpu_buffer_offset < 2 )
			{
				p_psxgpu->n_gpu_buffer_offset++;
			}
			else
			{
				verboselog( p_psxgpu, 1, "%02x: 8x8 sprite %08x %08x %08x\n", p_psxgpu->m_packet.n_entry[ 0 ] >> 24,
					p_psxgpu->m_packet.n_entry[ 0 ], p_psxgpu->m_packet.n_entry[ 1 ], p_psxgpu->m_packet.n_entry[ 2 ] );
				Sprite8x8(p_psxgpu);
				p_psxgpu->n_gpu_buffer_offset = 0;
			}
			break;
		case 0x78:
		case 0x79:
			/* 16*16 rectangle */
			if( p_psxgpu->n_gpu_buffer_offset < 1 )
			{
				p_psxgpu->n_gpu_buffer_offset++;
			}
			else
			{
				verboselog( p_psxgpu, 1, "%02x: 16x16 rectangle %08x %08x\n", p_psxgpu->m_packet.n_entry[ 0 ] >> 24,
					p_psxgpu->m_packet.n_entry[ 0 ], p_psxgpu->m_packet.n_entry[ 1 ] );
				FlatRectangle16x16(p_psxgpu);
				p_psxgpu->n_gpu_buffer_offset = 0;
			}
			break;
		case 0x7c:
		case 0x7d:
		case 0x7e:
		case 0x7f:
			if( p_psxgpu->n_gpu_buffer_offset < 2 )
			{
				p_psxgpu->n_gpu_buffer_offset++;
			}
			else
			{
				verboselog( p_psxgpu, 1, "%02x: 16x16 sprite %08x %08x %08x\n", p_psxgpu->m_packet.n_entry[ 0 ] >> 24,
					p_psxgpu->m_packet.n_entry[ 0 ], p_psxgpu->m_packet.n_entry[ 1 ], p_psxgpu->m_packet.n_entry[ 2 ] );
				Sprite16x16(p_psxgpu);
				p_psxgpu->n_gpu_buffer_offset = 0;
			}
			break;
		case 0x80:
			if( p_psxgpu->n_gpu_buffer_offset < 3 )
			{
				p_psxgpu->n_gpu_buffer_offset++;
			}
			else
			{
				verboselog( p_psxgpu, 1, "move image in frame buffer %08x %08x %08x %08x\n", p_psxgpu->m_packet.n_entry[ 0 ], p_psxgpu->m_packet.n_entry[ 1 ], p_psxgpu->m_packet.n_entry[ 2 ], p_psxgpu->m_packet.n_entry[ 3 ] );
				MoveImage(p_psxgpu);
				p_psxgpu->n_gpu_buffer_offset = 0;
			}
			break;
		case 0xa0:
			if( p_psxgpu->n_gpu_buffer_offset < 3 )
			{
				p_psxgpu->n_gpu_buffer_offset++;
			}
			else
			{
				UINT32 n_pixel;
				for( n_pixel = 0; n_pixel < 2; n_pixel++ )
				{
					UINT16 *p_vram;

					verboselog( p_psxgpu, 2, "send image to framebuffer ( pixel %u,%u = %u )\n",
						( p_psxgpu->n_vramx + p_psxgpu->m_packet.n_entry[ 1 ] ) & 1023,
						( p_psxgpu->n_vramy + ( p_psxgpu->m_packet.n_entry[ 1 ] >> 16 ) ) & 1023,
						data & 0xffff );

					p_vram = p_psxgpu->p_p_vram[ ( p_psxgpu->n_vramy + ( p_psxgpu->m_packet.n_entry[ 1 ] >> 16 ) ) & 1023 ] + ( ( p_psxgpu->n_vramx + p_psxgpu->m_packet.n_entry[ 1 ] ) & 1023 );
					WRITE_PIXEL( data & 0xffff );
					p_psxgpu->n_vramx++;
					if( p_psxgpu->n_vramx >= ( p_psxgpu->m_packet.n_entry[ 2 ] & 0xffff ) )
					{
						p_psxgpu->n_vramx = 0;
						p_psxgpu->n_vramy++;
						if( p_psxgpu->n_vramy >= ( p_psxgpu->m_packet.n_entry[ 2 ] >> 16 ) )
						{
							verboselog( p_psxgpu, 1, "%02x: send image to framebuffer %u,%u %u,%u\n", p_psxgpu->m_packet.n_entry[ 0 ] >> 24,
								p_psxgpu->m_packet.n_entry[ 1 ] & 0xffff, ( p_psxgpu->m_packet.n_entry[ 1 ] >> 16 ),
								p_psxgpu->m_packet.n_entry[ 2 ] & 0xffff, ( p_psxgpu->m_packet.n_entry[ 2 ] >> 16 ) );
							p_psxgpu->n_gpu_buffer_offset = 0;
							p_psxgpu->n_vramx = 0;
							p_psxgpu->n_vramy = 0;
							break;
						}
					}
					data >>= 16;
				}
			}
			break;
		case 0xc0:
			if( p_psxgpu->n_gpu_buffer_offset < 2 )
			{
				p_psxgpu->n_gpu_buffer_offset++;
			}
			else
			{
				verboselog( p_psxgpu, 1, "%02x: copy image from frame buffer\n", p_psxgpu->m_packet.n_entry[ 0 ] >> 24 );
				p_psxgpu->n_gpustatus |= ( 1L << 0x1b );
			}
			break;
		case 0xe1:
			verboselog( p_psxgpu, 1, "%02x: draw mode %06x\n", p_psxgpu->m_packet.n_entry[ 0 ] >> 24,
				p_psxgpu->m_packet.n_entry[ 0 ] & 0xffffff );
			decode_tpage( p_psxgpu, p_psxgpu->m_packet.n_entry[ 0 ] & 0xffffff );
			break;
		case 0xe2:
			p_psxgpu->n_twy = ( ( ( p_psxgpu->m_packet.n_entry[ 0 ] >> 15 ) & 0x1f ) << 3 );
			p_psxgpu->n_twx = ( ( ( p_psxgpu->m_packet.n_entry[ 0 ] >> 10 ) & 0x1f ) << 3 );
			p_psxgpu->n_twh = 255 - ( ( ( p_psxgpu->m_packet.n_entry[ 0 ] >> 5 ) & 0x1f ) << 3 );
			p_psxgpu->n_tww = 255 - ( ( p_psxgpu->m_packet.n_entry[ 0 ] & 0x1f ) << 3 );
			verboselog( p_psxgpu, 1, "%02x: texture window %u,%u %u,%u\n", p_psxgpu->m_packet.n_entry[ 0 ] >> 24,
				p_psxgpu->n_twx, p_psxgpu->n_twy, p_psxgpu->n_tww, p_psxgpu->n_twh );
			break;
		case 0xe3:
			p_psxgpu->n_drawarea_x1 = p_psxgpu->m_packet.n_entry[ 0 ] & 1023;
			if( p_psxgpu->n_gputype == 2 )
			{
				p_psxgpu->n_drawarea_y1 = ( p_psxgpu->m_packet.n_entry[ 0 ] >> 10 ) & 1023;
			}
			else
			{
				p_psxgpu->n_drawarea_y1 = ( p_psxgpu->m_packet.n_entry[ 0 ] >> 12 ) & 1023;
			}
			verboselog( p_psxgpu, 1, "%02x: drawing area top left %d,%d\n", p_psxgpu->m_packet.n_entry[ 0 ] >> 24,
				p_psxgpu->n_drawarea_x1, p_psxgpu->n_drawarea_y1 );
			break;
		case 0xe4:
			p_psxgpu->n_drawarea_x2 = p_psxgpu->m_packet.n_entry[ 0 ] & 1023;
			if( p_psxgpu->n_gputype == 2 )
			{
				p_psxgpu->n_drawarea_y2 = ( p_psxgpu->m_packet.n_entry[ 0 ] >> 10 ) & 1023;
			}
			else
			{
				p_psxgpu->n_drawarea_y2 = ( p_psxgpu->m_packet.n_entry[ 0 ] >> 12 ) & 1023;
			}
			verboselog( p_psxgpu, 1, "%02x: drawing area bottom right %d,%d\n", p_psxgpu->m_packet.n_entry[ 0 ] >> 24,
				p_psxgpu->n_drawarea_x2, p_psxgpu->n_drawarea_y2 );
			break;
		case 0xe5:
			p_psxgpu->n_drawoffset_x = SINT11( p_psxgpu->m_packet.n_entry[ 0 ] & 2047 );
			if( p_psxgpu->n_gputype == 2 )
			{
				p_psxgpu->n_drawoffset_y = SINT11( ( p_psxgpu->m_packet.n_entry[ 0 ] >> 11 ) & 2047 );
			}
			else
			{
				p_psxgpu->n_drawoffset_y = SINT11( ( p_psxgpu->m_packet.n_entry[ 0 ] >> 12 ) & 2047 );
			}
			verboselog( p_psxgpu, 1, "%02x: drawing offset %d,%d\n", p_psxgpu->m_packet.n_entry[ 0 ] >> 24,
				p_psxgpu->n_drawoffset_x, p_psxgpu->n_drawoffset_y );
			break;
		case 0xe6:
			p_psxgpu->n_gpustatus &= ~( 3L << 0xb );
			p_psxgpu->n_gpustatus |= ( data & 0x03 ) << 0xb;
			if( ( p_psxgpu->m_packet.n_entry[ 0 ] & 3 ) != 0 )
			{
				verboselog( p_psxgpu, 1, "not handled: mask setting %d\n", p_psxgpu->m_packet.n_entry[ 0 ] & 3 );
			}
			else
			{
				verboselog( p_psxgpu, 1, "mask setting %d\n", p_psxgpu->m_packet.n_entry[ 0 ] & 3 );
			}
			break;
		default:
#if defined( MAME_DEBUG )
			popmessage( "unknown GPU packet %08x", p_psxgpu->m_packet.n_entry[ 0 ] );
#endif
			verboselog( p_psxgpu, 0, "unknown GPU packet %08x (%08x)\n", p_psxgpu->m_packet.n_entry[ 0 ], data );
#if ( STOP_ON_ERROR )
			p_psxgpu->n_gpu_buffer_offset = 1;
#endif
			break;
		}
		p_ram++;
		n_size--;
	}
}

WRITE32_HANDLER( psx_gpu_w )
{
	psx_gpu *p_psxgpu = space->machine->driver_data<psx_state>()->p_psxgpu;

	switch( offset )
	{
	case 0x00:
		psx_gpu_write( space->machine, &data, 1 );
		break;
	case 0x01:
		switch( data >> 24 )
		{
		case 0x00:
			verboselog( p_psxgpu, 1, "reset gpu\n" );
			p_psxgpu->n_gpu_buffer_offset = 0;
			p_psxgpu->n_gpustatus = 0x14802000;
			p_psxgpu->n_drawarea_x1 = 0;
			p_psxgpu->n_drawarea_y1 = 0;
			p_psxgpu->n_drawarea_x2 = 1023;
			p_psxgpu->n_drawarea_y2 = 1023;
			p_psxgpu->n_drawoffset_x = 0;
			p_psxgpu->n_drawoffset_y = 0;
			p_psxgpu->n_displaystartx = 0;
			p_psxgpu->n_displaystarty = 0;
			p_psxgpu->n_horiz_disstart = 0x260;
			p_psxgpu->n_horiz_disend = 0xc60;
			p_psxgpu->n_vert_disstart = 0x010;
			p_psxgpu->n_vert_disend = 0x100;
			p_psxgpu->n_vramx = 0;
			p_psxgpu->n_vramy = 0;
			p_psxgpu->n_twx = 0;
			p_psxgpu->n_twy = 0;
			p_psxgpu->n_twh = 255;
			p_psxgpu->n_tww = 255;
			updatevisiblearea(space->machine, NULL);
			break;
		case 0x01:
			verboselog( p_psxgpu, 1, "not handled: reset command buffer\n" );
			p_psxgpu->n_gpu_buffer_offset = 0;
			break;
		case 0x02:
			verboselog( p_psxgpu, 1, "not handled: reset irq\n" );
			break;
		case 0x03:
			p_psxgpu->n_gpustatus &= ~( 1L << 0x17 );
			p_psxgpu->n_gpustatus |= ( data & 0x01 ) << 0x17;
			break;
		case 0x04:
			verboselog( p_psxgpu, 1, "dma setup %d\n", data & 3 );
			p_psxgpu->n_gpustatus &= ~( 3L << 0x1d );
			p_psxgpu->n_gpustatus |= ( data & 0x03 ) << 0x1d;
			p_psxgpu->n_gpustatus &= ~( 1L << 0x19 );
			if( ( data & 3 ) == 1 || ( data & 3 ) == 2 )
			{
				p_psxgpu->n_gpustatus |= ( 1L << 0x19 );
			}
			break;
		case 0x05:
			p_psxgpu->n_displaystartx = data & 1023;
			if( p_psxgpu->n_gputype == 2 )
			{
				p_psxgpu->n_displaystarty = ( data >> 10 ) & 1023;
			}
			else
			{
				p_psxgpu->n_displaystarty = ( data >> 12 ) & 1023;
			}
			verboselog( p_psxgpu, 1, "start of display area %d %d\n", p_psxgpu->n_displaystartx, p_psxgpu->n_displaystarty );
			break;
		case 0x06:
			p_psxgpu->n_horiz_disstart = data & 4095;
			p_psxgpu->n_horiz_disend = ( data >> 12 ) & 4095;
			verboselog( p_psxgpu, 1, "horizontal display range %d %d\n", p_psxgpu->n_horiz_disstart, p_psxgpu->n_horiz_disend );
			break;
		case 0x07:
			p_psxgpu->n_vert_disstart = data & 1023;
			p_psxgpu->n_vert_disend = ( data >> 10 ) & 2047;
			verboselog( p_psxgpu, 1, "vertical display range %d %d\n", p_psxgpu->n_vert_disstart, p_psxgpu->n_vert_disend );
			break;
		case 0x08:
			verboselog( p_psxgpu, 1, "display mode %02x\n", data & 0xff );
			p_psxgpu->n_gpustatus &= ~( 127L << 0x10 );
			p_psxgpu->n_gpustatus |= ( data & 0x3f ) << 0x11; /* width 0 + height + videmode + isrgb24 + isinter */
			p_psxgpu->n_gpustatus |= ( ( data & 0x40 ) >> 0x06 ) << 0x10; /* width 1 */
			if( p_psxgpu->n_gputype == 1 )
			{
				p_psxgpu->b_reverseflag = ( data >> 7 ) & 1;
			}
			updatevisiblearea(space->machine, NULL);
			break;
		case 0x09:
			verboselog( p_psxgpu, 1, "not handled: GPU Control 0x09: %08x\n", data );
			break;
		case 0x0d:
			verboselog( p_psxgpu, 1, "reset lightgun coordinates %08x\n", data );
			p_psxgpu->n_lightgun_x = 0;
			p_psxgpu->n_lightgun_y = 0;
			break;
		case 0x10:
			switch( data & 0xff )
			{
			case 0x03:
				if( p_psxgpu->n_gputype == 2 )
				{
					p_psxgpu->n_gpuinfo = p_psxgpu->n_drawarea_x1 | ( p_psxgpu->n_drawarea_y1 << 10 );
				}
				else
				{
					p_psxgpu->n_gpuinfo = p_psxgpu->n_drawarea_x1 | ( p_psxgpu->n_drawarea_y1 << 12 );
				}
				verboselog( p_psxgpu, 1, "GPU Info - Draw area top left %08x\n", p_psxgpu->n_gpuinfo );
				break;
			case 0x04:
				if( p_psxgpu->n_gputype == 2 )
				{
					p_psxgpu->n_gpuinfo = p_psxgpu->n_drawarea_x2 | ( p_psxgpu->n_drawarea_y2 << 10 );
				}
				else
				{
					p_psxgpu->n_gpuinfo = p_psxgpu->n_drawarea_x2 | ( p_psxgpu->n_drawarea_y2 << 12 );
				}
				verboselog( p_psxgpu, 1, "GPU Info - Draw area bottom right %08x\n", p_psxgpu->n_gpuinfo );
				break;
			case 0x05:
				if( p_psxgpu->n_gputype == 2 )
				{
					p_psxgpu->n_gpuinfo = ( p_psxgpu->n_drawoffset_x & 2047 ) | ( ( p_psxgpu->n_drawoffset_y & 2047 ) << 11 );
				}
				else
				{
					p_psxgpu->n_gpuinfo = ( p_psxgpu->n_drawoffset_x & 2047 ) | ( ( p_psxgpu->n_drawoffset_y & 2047 ) << 12 );
				}
				verboselog( p_psxgpu, 1, "GPU Info - Draw offset %08x\n", p_psxgpu->n_gpuinfo );
				break;
			case 0x07:
				p_psxgpu->n_gpuinfo = p_psxgpu->n_gputype;
				verboselog( p_psxgpu, 1, "GPU Info - GPU Type %08x\n", p_psxgpu->n_gpuinfo );
				break;
			case 0x08:
				p_psxgpu->n_gpuinfo = p_psxgpu->n_lightgun_x | ( p_psxgpu->n_lightgun_y << 16 );
				verboselog( p_psxgpu, 1, "GPU Info - lightgun coordinates %08x\n", p_psxgpu->n_gpuinfo );
				break;
			default:
				verboselog( p_psxgpu, 0, "GPU Info - unknown request (%08x)\n", data );
				p_psxgpu->n_gpuinfo = 0;
				break;
			}
			break;
		case 0x20:
			verboselog( p_psxgpu, 1, "not handled: GPU Control 0x20: %08x\n", data );
			break;
		default:
#if defined( MAME_DEBUG )
			popmessage( "unknown GPU command %08x", data );
#endif
			verboselog( p_psxgpu, 0, "gpu_w( %08x ) unknown GPU command\n", data );
			break;
		}
		break;
	default:
		verboselog( p_psxgpu, 0, "gpu_w( %08x, %08x, %08x ) unknown register\n", offset, data, mem_mask );
		break;
	}
}


void psx_gpu_read( running_machine *machine, UINT32 *p_ram, INT32 n_size )
{
	psx_gpu *p_psxgpu = machine->driver_data<psx_state>()->p_psxgpu;

	while( n_size > 0 )
	{
		if( ( p_psxgpu->n_gpustatus & ( 1L << 0x1b ) ) != 0 )
		{
			UINT32 n_pixel;
			PAIR data;

			verboselog( p_psxgpu, 2, "copy image from frame buffer ( %d, %d )\n", p_psxgpu->n_vramx, p_psxgpu->n_vramy );
			data.d = 0;
			for( n_pixel = 0; n_pixel < 2; n_pixel++ )
			{
				data.w.l = data.w.h;
				data.w.h = *( p_psxgpu->p_p_vram[ p_psxgpu->n_vramy + ( p_psxgpu->m_packet.n_entry[ 1 ] >> 16 ) ] + p_psxgpu->n_vramx + ( p_psxgpu->m_packet.n_entry[ 1 ] & 0xffff ) );
				p_psxgpu->n_vramx++;
				if( p_psxgpu->n_vramx >= ( p_psxgpu->m_packet.n_entry[ 2 ] & 0xffff ) )
				{
					p_psxgpu->n_vramx = 0;
					p_psxgpu->n_vramy++;
					if( p_psxgpu->n_vramy >= ( p_psxgpu->m_packet.n_entry[ 2 ] >> 16 ) )
					{
						verboselog( p_psxgpu, 1, "copy image from frame buffer end\n" );
						p_psxgpu->n_gpustatus &= ~( 1L << 0x1b );
						p_psxgpu->n_gpu_buffer_offset = 0;
						p_psxgpu->n_vramx = 0;
						p_psxgpu->n_vramy = 0;
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
			verboselog( p_psxgpu, 2, "read GPU info (%08x)\n", p_psxgpu->n_gpuinfo );
			*( p_ram ) = p_psxgpu->n_gpuinfo;
		}
		p_ram++;
		n_size--;
	}
}

READ32_HANDLER( psx_gpu_r )
{
	psx_gpu *p_psxgpu = space->machine->driver_data<psx_state>()->p_psxgpu;
	UINT32 data;

	switch( offset )
	{
	case 0x00:
		psx_gpu_read( space->machine, &data, 1 );
		break;
	case 0x01:
		data = p_psxgpu->n_gpustatus;
		verboselog( p_psxgpu, 1, "read GPU status (%08x)\n", data );
		break;
	default:
		verboselog( p_psxgpu, 0, "gpu_r( %08x, %08x ) unknown register\n", offset, mem_mask );
		data = 0;
		break;
	}
	return data;
}

INTERRUPT_GEN( psx_vblank )
{
	psx_gpu *p_psxgpu = device->machine->driver_data<psx_state>()->p_psxgpu;

#if defined( MAME_DEBUG )
	DebugCheckKeys(p_psxgpu);
#endif

	p_psxgpu->n_gpustatus ^= ( 1L << 31 );

	if(p_psxgpu->b_need_sianniv_vblank_hack)
	{
		UINT32 pc = cpu_get_pc(device);
		if((pc >= 0x80010018 && pc <= 0x80010028) || pc == 0x8002a4f0)
			return;
	}

	psx_irq_set( device->machine, 0x0001 );
}

void psx_gpu_reset( running_machine *machine )
{
	address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);

	psx_gpu_w(space, 1, 0, 0xffffffff );
}

void psx_lightgun_set( running_machine *machine, int n_x, int n_y )
{
	psx_gpu *p_psxgpu = machine->driver_data<psx_state>()->p_psxgpu;

	p_psxgpu->n_lightgun_x = n_x;
	p_psxgpu->n_lightgun_y = n_y;
}
