/***************************************************************************

    saa5050.c

    Functions to emulate the
    SAA5050 - Teletext Character Generator.

    TODO:
    -  Implement BOX and dirtybuffer
    -  Add support for non-English version (SAA505x), possibly merging
       src/mess/video/saa505x.c unsed by bbc.c in MESS
    -  Investigate why supporting code 0 behavior breaks p2000t vscroll
       in MESS (but not supporting breaks malzak title background)
    -  x,y sizes should probably be calculated from the screen parameters
       rather than passed in the device interface

***************************************************************************/

#include "emu.h"
#include "video/saa5050.h"

#define SAA5050_DBLHI   0x0001
#define SAA5050_SEPGR   0x0002
#define SAA5050_FLASH   0x0004
#define SAA5050_BOX     0x0008
#define SAA5050_GRAPH   0x0010
#define SAA5050_CONCEAL 0x0020
#define SAA5050_HOLDGR  0x0040

#define SAA5050_BLACK   0
#define SAA5050_WHITE   7


typedef struct _saa5050_state  saa5050_state;
struct _saa5050_state
{
	device_t *screen;
	int         gfxnum;
	int         x, y;
	int         size;
	int         rev;

	UINT8 *     videoram;
	UINT16      flags;
	UINT8	      forecol;
	UINT8	      backcol;
	UINT8	      prvcol;
	UINT8	      prvchr;
	INT8        frame_count;
};


/*************************************
 *
 *  Inline functions
 *
 *************************************/

INLINE saa5050_state *get_safe_token( device_t *device )
{
	assert(device != NULL);
	assert(device->type() == SAA5050);

	return (saa5050_state *)downcast<legacy_device_base *>(device)->token();
}

INLINE const saa5050_interface *get_interface( device_t *device )
{
	assert(device != NULL);
	assert((device->type() == SAA5050));
	return (const saa5050_interface *) device->static_config();
}

/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout saa5050_charlayout =
{
	6, 10,
	256,
	1,
	{ 0 },
	{ 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8,
	  5*8, 6*8, 7*8, 8*8, 9*8 },
	8 * 10
};

static const gfx_layout saa5050_hilayout =
{
	6, 10,
	256,
	1,
	{ 0 },
	{ 2, 3, 4, 5, 6, 7 },
	{ 0*8, 0*8, 1*8, 1*8, 2*8,
	  2*8, 3*8, 3*8, 4*8, 4*8 },
	8 * 10
};

static const gfx_layout saa5050_lolayout =
{
	6, 10,
	256,
	1,
	{ 0 },
	{ 2, 3, 4, 5, 6, 7 },
	{ 5*8, 5*8, 6*8, 6*8, 7*8,
	  7*8, 8*8, 8*8, 9*8, 9*8 },
	8 * 10
};

GFXDECODE_START( saa5050 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, saa5050_charlayout, 0, 64 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, saa5050_hilayout, 0, 64 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, saa5050_lolayout, 0, 64 )
GFXDECODE_END


/*************************************
 *
 *  Palette initialization
 *
 *************************************/

static const UINT8 saa5050_colors[8 * 3] =
{
	0x00, 0x00, 0x00,	/* black */
	0xff, 0x00, 0x00,	/* red */
	0x00, 0xff, 0x00,	/* green */
	0xff, 0xff, 0x00,	/* yellow */
	0x00, 0x00, 0xff,	/* blue */
	0xff, 0x00, 0xff,	/* magenta */
	0x00, 0xff, 0xff,	/* cyan */
	0xff, 0xff, 0xff	/* white */
};

static const UINT16 saa5050_palette[64 * 2] =	/* bgnd, fgnd */
{
	0,1, 0,1, 0,2, 0,3, 0,4, 0,5, 0,6, 0,7,
	1,0, 1,1, 1,2, 1,3, 1,4, 1,5, 1,6, 1,7,
	2,0, 2,1, 2,2, 2,3, 2,4, 2,5, 2,6, 2,7,
	3,0, 3,1, 3,2, 3,3, 3,4, 3,5, 3,6, 3,7,
	4,0, 4,1, 4,2, 4,3, 4,4, 4,5, 4,6, 4,7,
	5,0, 5,1, 5,2, 5,3, 5,4, 5,5, 5,6, 5,7,
	6,0, 6,1, 6,2, 6,3, 6,4, 6,5, 6,6, 6,7,
	7,0, 7,1, 7,2, 7,3, 7,4, 7,5, 7,6, 7,7
};

PALETTE_INIT( saa5050 )
{
	UINT8 i, r, g, b;

	machine.colortable = colortable_alloc(machine, 8);

	for ( i = 0; i < 8; i++ )
	{
		r = saa5050_colors[i * 3];
		g = saa5050_colors[i * 3 + 1];
		b = saa5050_colors[i * 3 + 2];
		colortable_palette_set_color(machine.colortable, i, MAKE_RGB(r, g, b));
	}

	for (i = 0; i < 128; i++)
		colortable_entry_set_value(machine.colortable, i, saa5050_palette[i]);
}

/*************************************
 *
 *  Videoram access handlers
 *
 *************************************/

WRITE8_DEVICE_HANDLER( saa5050_videoram_w )
{
	saa5050_state *saa5050 = get_safe_token(device);
	saa5050->videoram[offset] = data;
}


READ8_DEVICE_HANDLER( saa5050_videoram_r )
{
	saa5050_state *saa5050 = get_safe_token(device);
	return saa5050->videoram[offset];
}

/*************************************
 *
 *  Emulation
 *
 *************************************/


/* this should probably be put at the end of saa5050 update,
but p2000t in MESS does not seem to currently support it.
Hence, we leave it independent for the moment */
void saa5050_frame_advance( device_t *device )
{
	saa5050_state *saa5050 = get_safe_token(device);

	saa5050->frame_count++;
	if (saa5050->frame_count > 50)
		saa5050->frame_count = 0;
}

void saa5050_update( device_t *device, bitmap_t *bitmap, const rectangle *cliprect  )
{
	saa5050_state *saa5050 = get_safe_token(device);
	int code, colour;
	int sx, sy, ssy;

	for (sy = 0; sy <= saa5050->y; sy++)
	{
		/* Set start of line state */
		saa5050->flags = 0;
		saa5050->prvchr = 32;
		saa5050->forecol = SAA5050_WHITE;
		saa5050->prvcol = SAA5050_WHITE;
		saa5050->backcol = SAA5050_BLACK;

		/* should we go in reverse order? */
		ssy = saa5050->rev ? saa5050->y - sy : sy;

		for (sx = 0; sx < saa5050->x; sx++)
		{
			int blank = 0;
			code = saa5050->videoram[ssy * saa5050->size + sx];
			if (code < 32)
			{
				switch (code)
				{
				case 0x00:
					// Temporary hack until proper docs are found
					if (saa5050->rev) // This is not ok, but it is done only in case of malzak
						blank = 1;  // code 0x00 should not display anything, unless HOLDGR is set
					break;
				case 0x01: case 0x02: case 0x03: case 0x04:
				case 0x05: case 0x06: case 0x07:
					saa5050->prvcol = saa5050->forecol = code;
					saa5050->flags &= ~(SAA5050_GRAPH | SAA5050_CONCEAL);
					break;
				case 0x11: case 0x12: case 0x13: case 0x14:
				case 0x15: case 0x16: case 0x17:
					saa5050->prvcol = (saa5050->forecol = (code & 0x07));
					saa5050->flags &= ~SAA5050_CONCEAL;
					saa5050->flags |= SAA5050_GRAPH;
					break;
				case 0x08:
					saa5050->flags |= SAA5050_FLASH;
					break;
				case 0x09:
					saa5050->flags &= ~SAA5050_FLASH;
					break;
				case 0x0a:
					saa5050->flags |= SAA5050_BOX;
					break;
				case 0x0b:
					saa5050->flags &= ~SAA5050_BOX;
					break;
				case 0x0c:
					saa5050->flags &= ~SAA5050_DBLHI;
					break;
				case 0x0d:
					saa5050->flags |= SAA5050_DBLHI;
					break;
				case 0x18:
					saa5050->flags |= SAA5050_CONCEAL;
					break;
				case 0x19:
					saa5050->flags |= SAA5050_SEPGR;
					break;
				case 0x1a:
					saa5050->flags &= ~SAA5050_SEPGR;
					break;
				case 0x1c:
					saa5050->backcol = SAA5050_BLACK;
					break;
				case 0x1d:
					saa5050->backcol = saa5050->prvcol;
					break;
				case 0x1e:
					saa5050->flags |= SAA5050_HOLDGR;
					break;
				case 0x1f:
					saa5050->flags &= ~SAA5050_HOLDGR;
					break;
				}

				if (saa5050->flags & SAA5050_HOLDGR)
					code = saa5050->prvchr;
				else
					code = 32;
			}

			if (code & 0x80)
				colour = (saa5050->forecol << 3) | saa5050->backcol;
			else
				colour = saa5050->forecol | (saa5050->backcol << 3);

			if (saa5050->flags & SAA5050_CONCEAL)
				code = 32;
			else if ((saa5050->flags & SAA5050_FLASH) && (saa5050->frame_count > 38))
				code = 32;
			else
			{
				saa5050->prvchr = code;
				if ((saa5050->flags & SAA5050_GRAPH) && (code & 0x20))
				{
					code += (code & 0x40) ? 64 : 96;
					if (saa5050->flags & SAA5050_SEPGR)
						code += 64;
				}
			}

			if((blank == 0) || (saa5050->flags & SAA5050_HOLDGR))
			{
				if (saa5050->flags & SAA5050_DBLHI)
				{
					drawgfxzoom_opaque(bitmap, cliprect, saa5050->screen->machine().gfx[saa5050->gfxnum + 1], code, colour, 0, 0, sx * 12, ssy * 20, 0x20000, 0x20000);
					drawgfxzoom_opaque(bitmap, cliprect, saa5050->screen->machine().gfx[saa5050->gfxnum + 2], code, colour, 0, 0, sx * 12, (ssy + 1) * 20, 0x20000, 0x20000);
				}
				else
				{
					drawgfxzoom_opaque(bitmap, cliprect, saa5050->screen->machine().gfx[saa5050->gfxnum + 0], code, colour, 0, 0, sx * 12, ssy * 20, 0x20000, 0x20000);
				}
			}
		}

		if (saa5050->flags & SAA5050_DBLHI)
		{
			sy++;
			saa5050->flags &= ~SAA5050_DBLHI;
		}
	}
}

/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

static DEVICE_START( saa5050 )
{
	saa5050_state *saa5050 = get_safe_token(device);
	const saa5050_interface *intf = get_interface(device);

	saa5050->screen = device->machine().device(intf->screen);
	saa5050->gfxnum = intf->gfxnum;
	saa5050->x = intf->x;
	saa5050->y = intf->y;
	saa5050->size = intf->size;
	saa5050->rev = intf->rev;

	saa5050->videoram = auto_alloc_array(device->machine(), UINT8, 0x800);

	device->save_pointer(NAME(saa5050->videoram), 0x800);
	device->save_item(NAME(saa5050->flags));
	device->save_item(NAME(saa5050->forecol));
	device->save_item(NAME(saa5050->backcol));
	device->save_item(NAME(saa5050->prvcol));
	device->save_item(NAME(saa5050->prvchr));
	device->save_item(NAME(saa5050->frame_count));
}

static DEVICE_RESET( saa5050 )
{
	saa5050_state *saa5050 = get_safe_token(device);

	memset(saa5050->videoram, 0x00, 0x800);

	saa5050->flags = 0;
	saa5050->forecol = SAA5050_WHITE;
	saa5050->backcol = SAA5050_BLACK;
	saa5050->prvcol = SAA5050_WHITE;
	saa5050->prvchr = 32;
	saa5050->frame_count = 0;
}

static const char DEVTEMPLATE_SOURCE[] = __FILE__;

#define DEVTEMPLATE_ID( p, s )	p##saa5050##s
#define DEVTEMPLATE_FEATURES	DT_HAS_START | DT_HAS_RESET
#define DEVTEMPLATE_NAME		"SAA5050"
#define DEVTEMPLATE_FAMILY		"SAA5050 Teletext Character Generator"
#include "devtempl.h"


DEFINE_LEGACY_DEVICE(SAA5050, saa5050);
