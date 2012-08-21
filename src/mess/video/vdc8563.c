/***************************************************************************

  CBM Video Device Chip 8563

  Original code by PeT (peter.trauner@jk.uni-linz.ac.at)

    2010-02: converted to be a device

    TODO:
      - clean up the code
      - add RAM with an internal address map

***************************************************************************/
/*
 several graphic problems
 some are in the rastering engine and should be solved during its evalution
 rare and short documentation,
 registers and some words of description in the c128 user guide */
/* seems to be a motorola m6845 variant */


#include "emu.h"
#include "video/vdc8563.h"

typedef struct _vdc8563_state vdc8563_state;
struct _vdc8563_state
{
	screen_device *screen;

	int state;
	UINT8 reg[37];
	UINT8 index;

	UINT16 addr, src;

	UINT16 videoram_start, colorram_start, fontram_start;
	UINT16 videoram_size;

	int rastering;

	UINT8 *ram;
	UINT8 *dirty;
	UINT8 fontdirty[0x200];
	UINT16 mask, fontmask;

	double cursor_time;
	int	cursor_on;

	int changed;
};

/*****************************************************************************
    CONSTANTS
*****************************************************************************/

#define VERBOSE_LEVEL 0
#define DBG_LOG(N, M, A)      \
	do { \
		if (VERBOSE_LEVEL >= N) \
		{ \
			if (M) \
				logerror("%11.6f: %-24s",device->machine().time().as_double(),(char*)M ); \
			logerror A; \
		} \
	} while (0)


static const struct {
	int stored,
		read;
} reg_mask[]= {
	{ 0xff, 0 },
	{ 0xff, 0 },
	{ 0xff, 0 },
	{ 0xff, 0 },
	{ 0xff, 0 },
	{ 0x1f, 0 },
	{ 0xff, 0 },
	{ 0xff, 0 },
	{  0x3, 0 }, //8
	{  0x1f, 0 },
	{  0x7f, 0 },
	{  0x1f, 0 },
	{  0xff, 0xff },
	{  0xff, 0xff },
	{  0xff, 0xff },
	{  0xff, 0xff },
	{  -1, 0xff }, //0x10
	{  -1, 0xff },
	{  0xff, 0xff },
	{  0xff, 0xff },
	{  0xff, -1 },
	{  0x1f, -1 },
	{  0xff, -1 },
	{  0xff, -1 },
	{  0xff, -1 },//0x18
	{  0xff, -1 },
	{  0xff, -1 },
	{  0xff, -1 },
	{  0xf0, -1 },
	{  0x1f, -1 },
	{  0xff, -1 },
	{  0xff, -1 },
	{  0xff, -1 }, //0x20
	{  0xff, -1 },
	{  0xff, -1 },
	{  0xff, -1 },
	{  0x0f, -1 },
};
#define REG(x)          (vdc8563->reg[x] & reg_mask[x].stored)


#define CHAR_WIDTH      (((vdc8563->reg[0x16] & 0xf0) >> 4) + 1)
#define CHAR_WIDTH_VISIBLE ((vdc8563->reg[0x16] & 0x0f) + 1)

#define BLOCK_COPY      (vdc8563->reg[0x18] & 0x80)

#define MONOTEXT        ((vdc8563->reg[0x19] & 0xc0) == 0)
#define TEXT            ((vdc8563->reg[0x19] & 0xc0) == 0x40)
#define GRAPHIC         (vdc8563->reg[0x19] & 0x80)

#define FRAMECOLOR      (vdc8563->reg[0x1a] & 0x0f)
#define MONOCOLOR       (vdc8563->reg[0x1a] >> 4)

#define LINEDIFF        (vdc8563->reg[0x1b])
#define FONT_START      ((vdc8563->reg[0x1c] & 0xe0) << 8)
/* 0x1c 0x10 dram 0:4416, 1: 4164 */

/* 0x1d 0x1f counter for underlining */

#define FILLBYTE        vdc8563->reg[0x1f]

#define CLOCK_HALFING   (vdc8563->reg[25] & 0x10)


/* the regs below corresponds to the ones used by 6845, hence we borrow the macros */
#define CRTC6845_COLUMNS (REG(0) + 1)
#define CRTC6845_CHAR_COLUMNS (REG(1))
#define CRTC6845_CHAR_LINES REG(6)
#define CRTC6845_CHAR_HEIGHT ((REG(9) & 0x1f) + 1)
#define CRTC6845_LINES (REG(4) * CRTC6845_CHAR_HEIGHT + REG(5))
#define CRTC6845_VIDEO_START ((REG(0xc) << 8) | REG(0xd))
#define CRTC6845_INTERLACE_MODE (REG(8) & 3)
#define CRTC6845_INTERLACE_SIGNAL 1
#define CRTC6845_INTERLACE 3
#define CRTC6845_CURSOR_MODE (REG(0xa) & 0x60)
#define CRTC6845_CURSOR_OFF 0x20
#define CRTC6845_CURSOR_16FRAMES 0x40
#define CRTC6845_CURSOR_32FRAMES 0x60
#define CRTC6845_SKEW	(REG(8) & 15)
#define CRTC6845_CURSOR_POS ((REG(0xe) << 8) | REG(0xf))
#define CRTC6845_CURSOR_TOP	(REG(0xa) & 0x1f)
#define CRTC6845_CURSOR_BOTTOM REG(0xb)


/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE vdc8563_state *get_safe_token( device_t *device )
{
	assert(device != NULL);
	assert(device->type() == VDC8563);

	return (vdc8563_state *)downcast<legacy_device_base *>(device)->token();
}

INLINE const vdc8563_interface *get_interface( device_t *device )
{
	assert(device != NULL);
	assert((device->type() == VDC8563));
	return (const vdc8563_interface *) device->static_config();
}

/*****************************************************************************
    IMPLEMENTATION
*****************************************************************************/

static void vdc_videoram_w( device_t *device, int offset, int data )
{
	vdc8563_state *vdc8563 = get_safe_token(device);

	offset &= vdc8563->mask;

	if (vdc8563->ram[offset] != data)
	{
		vdc8563->ram[offset] = data;
		vdc8563->dirty[offset] = 1;
		if ((vdc8563->fontram_start & vdc8563->fontmask) == (offset & vdc8563->fontmask))
			vdc8563->fontdirty[(offset & 0x1ff0) >> 4] = 1;
	}
}

INLINE int vdc_videoram_r( device_t *device, int offset )
{
	vdc8563_state *vdc8563 = get_safe_token(device);
	return vdc8563->ram[offset & vdc8563->mask];
}

void vdc8563_set_rastering( device_t *device, int on )
{
	vdc8563_state *vdc8563 = get_safe_token(device);
	vdc8563->rastering = on;
	vdc8563->changed |= 1;
}



/* 0x22 number of chars from start of line to positiv edge of display enable */
/* 0x23 number of chars from start of line to negativ edge of display enable */
/* 0x24 0xf number of refresh cycles per line */

WRITE8_DEVICE_HANDLER( vdc8563_port_w )
{
	vdc8563_state *vdc8563 = get_safe_token(device);
	UINT8 i;

	if (offset & 1)
	{
		if ((vdc8563->index & 0x3f) < 37)
		{
			switch (vdc8563->index & 0x3f)
			{
			case 1: case 4: case 0x1b:
				vdc8563->reg[vdc8563->index] = data;
				vdc8563->videoram_size = CRTC6845_CHAR_LINES * (CRTC6845_CHAR_COLUMNS + LINEDIFF);
				vdc8563->changed = 1;
				break;
			case 0xe: case 0xf: case 0xa: case 0xb:
				vdc8563->dirty[CRTC6845_CURSOR_POS & vdc8563->mask] = 1;
				vdc8563->reg[vdc8563->index] = data;
				break;
			case 0xc: case 0xd:
				vdc8563->reg[vdc8563->index] = data;
				vdc8563->videoram_start = CRTC6845_VIDEO_START;
				vdc8563->changed = 1;
				break;
			case 0x12:
				vdc8563->addr = (vdc8563->addr & 0x00ff) | (data << 8);
				break;
			case 0x13:
				vdc8563->addr = (vdc8563->addr & 0xff00) | data;
				break;
			case 0x20:
				vdc8563->src = (vdc8563->src & 0x00ff) | (data << 8);
				break;
			case 0x21:
				vdc8563->src = (vdc8563->src & 0xff00) | data;
				break;
			case 0x14: case 0x15:
				vdc8563->reg[vdc8563->index] = data;
				vdc8563->colorram_start = (vdc8563->reg[0x14] << 8) | vdc8563->reg[0x15];
				vdc8563->changed = 1;
				break;
			case 0x1c:
				vdc8563->reg[vdc8563->index] = data;
				vdc8563->fontram_start = FONT_START;
				vdc8563->changed = 1;
				break;
			case 0x16: case 0x19: case 0x1a:
				vdc8563->reg[vdc8563->index] = data;
				vdc8563->changed = 1;
				break;
			case 0x1e:
				vdc8563->reg[vdc8563->index] = data;
				if (BLOCK_COPY)
				{
					DBG_LOG(2, "vdc block copy", ("src:%.4x dst:%.4x size:%.2x\n", vdc8563->src, vdc8563->addr, data));
					i = data;
					do {
						vdc_videoram_w(device, vdc8563->addr++, vdc_videoram_r(device, vdc8563->src++));
					} while (--i != 0);
				}
				else
				{
					DBG_LOG(2, "vdc block set", ("dest:%.4x value:%.2x size:%.2x\n", vdc8563->addr, FILLBYTE, data));
					i = data;
					do {
						vdc_videoram_w(device, vdc8563->addr++, FILLBYTE);
					} while (--i != 0);
				}
				break;
			case 0x1f:
				DBG_LOG(2, "vdc written", ("dest:%.4x size:%.2x\n", vdc8563->addr, data));
				vdc8563->reg[vdc8563->index] = data;
				vdc_videoram_w(device, vdc8563->addr++, data);
				break;
			default:
				vdc8563->reg[vdc8563->index] = data;
				DBG_LOG(2, "vdc8563_port_w", ("%.2x:%.2x\n", vdc8563->index, data));
				break;
			}
		}
		DBG_LOG(3, "vdc8563_port_w", ("%.2x:%.2x\n", vdc8563->index, data));
	}
	else
	{
		vdc8563->index = data;
	}
}

READ8_DEVICE_HANDLER( vdc8563_port_r )
{
	vdc8563_state *vdc8563 = get_safe_token(device);
	int val;

	val = 0xff;
	if (offset & 1)
	{
		if ((vdc8563->index & 0x3f) < 37)
		{
			switch (vdc8563->index & 0x3f)
			{
			case 0x12:
				val = vdc8563->addr >> 8;
				break;
			case 0x13:
				val = vdc8563->addr & 0xff;
				break;
			case 0x1e:
				val = 0;
				break;
			case 0x1f:
				val = vdc_videoram_r(device, vdc8563->addr);
				DBG_LOG(2, "vdc read", ("%.4x %.2x\n", vdc8563->addr, val));
				break;
			case 0x20:
				val = vdc8563->src >> 8;
				break;
			case 0x21:
				val = vdc8563->src & 0xff;
				break;
			default:
				val = vdc8563->reg[vdc8563->index & 0x3f] & reg_mask[vdc8563->index & 0x3f].read;
			}
		}
		DBG_LOG(2, "vdc8563_port_r", ("%.2x:%.2x\n", vdc8563->index, val));
	}
	else
	{
		val = vdc8563->index;
		if (vdc8563->state)
			val |= 0x80;
	}
	return val;
}

static int vdc8563_clocks_in_frame( device_t *device )
{
	vdc8563_state *vdc8563 = get_safe_token(device);
	int clocks = CRTC6845_COLUMNS * CRTC6845_LINES;

	switch (CRTC6845_INTERLACE_MODE)
	{
	case CRTC6845_INTERLACE_SIGNAL: // interlace generation of video signals only
	case CRTC6845_INTERLACE: // interlace
		return clocks / 2;
	default:
		return clocks;
	}
}

static void vdc8563_time( device_t *device )
{
	vdc8563_state *vdc8563 = get_safe_token(device);
	double newtime, ftime;
	newtime = device->machine().time().as_double();

	if (vdc8563_clocks_in_frame(device) == 0.0)
		return;

	ftime = 16 * vdc8563_clocks_in_frame(device) / 2000000.0;
	if (CLOCK_HALFING)
		ftime *= 2;
	switch (CRTC6845_CURSOR_MODE)
	{
	case CRTC6845_CURSOR_OFF:
		vdc8563->cursor_on = 0;
		break;
	case CRTC6845_CURSOR_32FRAMES:
		ftime *= 2;
	case CRTC6845_CURSOR_16FRAMES:
		if (newtime - vdc8563->cursor_time > ftime)
		{
			vdc8563->cursor_time += ftime;
			vdc8563->dirty[CRTC6845_CURSOR_POS & vdc8563->mask] = 1;
			vdc8563->cursor_on ^= 1;
		}
		break;
	default:
		vdc8563->cursor_on = 1;
		break;
	}
}

static void vdc8563_monotext_screenrefresh( device_t *device, bitmap_ind16 &bitmap, int full_refresh )
{
	vdc8563_state *vdc8563 = get_safe_token(device);
	running_machine &machine = device->machine();
	int x, y, i;
	rectangle rect;
	int w = CRTC6845_CHAR_COLUMNS;
	int h = CRTC6845_CHAR_LINES;
	int height = CRTC6845_CHAR_HEIGHT;

	rect.setx(vdc8563->screen->visible_area().min_x, vdc8563->screen->visible_area().max_x);

	if (full_refresh)
		memset(vdc8563->dirty + vdc8563->videoram_start, 1, vdc8563->videoram_size);

	for (y = 0, rect.min_y = height, rect.max_y = rect.min_y + height - 1, i = vdc8563->videoram_start & vdc8563->mask; y < h;
		y++, rect.min_y += height, rect.max_y += height)
	{
		for (x = 0; x < w; x++, i = (i + 1) & vdc8563->mask)
		{
			if (vdc8563->dirty[i])
			{
				drawgfx_opaque(bitmap,rect,machine.gfx[0], vdc8563->ram[i], FRAMECOLOR | (MONOCOLOR << 4), 0, 0,
						machine.gfx[0]->width * x + 8, height * y + height);

				if ((vdc8563->cursor_on) && (i == (CRTC6845_CURSOR_POS & vdc8563->mask)))
				{
					int k = height - CRTC6845_CURSOR_TOP;
					if (CRTC6845_CURSOR_BOTTOM < height)
						k = CRTC6845_CURSOR_BOTTOM - CRTC6845_CURSOR_TOP + 1;

					if (k > 0)
						bitmap.plot_box(machine.gfx[0]->width * x + 8, height * y + height + CRTC6845_CURSOR_TOP, machine.gfx[0]->width, k, FRAMECOLOR);
				}

				vdc8563->dirty[i] = 0;
			}
		}
		i += LINEDIFF;
	}
}

static void vdc8563_text_screenrefresh( device_t *device, bitmap_ind16 &bitmap, int full_refresh )
{
	vdc8563_state *vdc8563 = get_safe_token(device);
	running_machine &machine = device->machine();
	int x, y, i, j;
	rectangle rect;
	int w = CRTC6845_CHAR_COLUMNS;
	int h = CRTC6845_CHAR_LINES;
	int height = CRTC6845_CHAR_HEIGHT;

	rect.setx(vdc8563->screen->visible_area().min_x, vdc8563->screen->visible_area().max_x);

	if (full_refresh)
		memset(vdc8563->dirty + vdc8563->videoram_start, 1, vdc8563->videoram_size);

	for (y = 0, rect.min_y = height, rect.max_y = rect.min_y + height - 1, i = vdc8563->videoram_start & vdc8563->mask,
		j = vdc8563->colorram_start & vdc8563->mask; y < h; y++, rect.min_y += height, rect.max_y += height)
	{
		for (x = 0; x < w; x++, i = (i + 1) & vdc8563->mask, j = (j + 1) & vdc8563->mask)
		{
			if (vdc8563->dirty[i] || vdc8563->dirty[j])
			{
				{
					UINT16 ch, fg, bg;
					const UINT8 *charptr;
					int v, h2;
					UINT16 *pixel;

					ch = vdc8563->ram[i] | ((vdc8563->ram[j] & 0x80) ? 0x100 : 0);
					charptr = &vdc8563->ram[(vdc8563->fontram_start + (ch * 16)) & vdc8563->mask];
					fg = ((vdc8563->ram[j] & 0x0f) >> 0) + 0x10;
					bg = ((vdc8563->ram[j] & 0x70) >> 4) + 0x10;

					for (v = 0; v < 16; v++)
					{
						for (h2 = 0; h2 < 8; h2++)
						{
							pixel = &bitmap.pix16((y * height) + height + v, (x * 8) + 8 + h2);
							*pixel = (charptr[v] & (0x80 >> h2)) ? fg : bg;
						}
					}
				}

				if ((vdc8563->cursor_on) && (i == (CRTC6845_CURSOR_POS & vdc8563->mask)))
				{
					int k = height - CRTC6845_CURSOR_TOP;
					if (CRTC6845_CURSOR_BOTTOM < height)
						k = CRTC6845_CURSOR_BOTTOM - CRTC6845_CURSOR_TOP + 1;

					if (k > 0)
						bitmap.plot_box(machine.gfx[0]->width * x + 8, height * y + height + CRTC6845_CURSOR_TOP, machine.gfx[0]->width,
								k, 0x10 | (vdc8563->ram[j] & 0x0f));
				}

				vdc8563->dirty[i] = 0;
				vdc8563->dirty[j] = 0;
			}
		}
		i += LINEDIFF;
		j += LINEDIFF;
	}
}

static void vdc8563_graphic_screenrefresh( device_t *device, bitmap_ind16 &bitmap, int full_refresh )
{
	vdc8563_state *vdc8563 = get_safe_token(device);
	running_machine &machine = device->machine();
	int x, y, i, j, k;
	rectangle rect;
	int w = CRTC6845_CHAR_COLUMNS;
	int h = CRTC6845_CHAR_LINES;
	int height = CRTC6845_CHAR_HEIGHT;

	rect.setx(vdc8563->screen->visible_area().min_x, vdc8563->screen->visible_area().max_x);

	if (full_refresh)
		memset(vdc8563->dirty, 1, vdc8563->mask + 1);

	for (y = 0, rect.min_y = height, rect.max_y = rect.min_y + height - 1, i = vdc8563->videoram_start & vdc8563->mask; y < h;
		y++, rect.min_y += height, rect.max_y += height)
	{
		for (x = 0; x < w; x++, i = (i + 1) & vdc8563->mask)
		{
			for (j = 0; j < height; j++)
			{
				k = ((i << 4) + j) & vdc8563->mask;
				if (vdc8563->dirty[k])
				{
					drawgfx_opaque(bitmap, rect, machine.gfx[1], vdc8563->ram[k], FRAMECOLOR | (MONOCOLOR << 4), 0, 0,
							machine.gfx[0]->width * x + 8, height * y + height + j);
					vdc8563->dirty[k] = 0;
				}
			}
		}
		i += LINEDIFF;
	}
}

UINT32 vdc8563_video_update( device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	vdc8563_state *vdc8563 = get_safe_token(device);
	int i;
	int full_refresh = 1;

	if (!vdc8563->rastering)
		return 0;

	vdc8563_time(device);

	full_refresh |= vdc8563->changed;

	if (GRAPHIC)
	{
		vdc8563_graphic_screenrefresh(device, bitmap, full_refresh);
	}
	else
	{
		for (i = 0; i < 512; i++)
		{
			if (full_refresh || vdc8563->fontdirty[i])
			{
				gfx_element_mark_dirty(device->machine().gfx[0],i);
				vdc8563->fontdirty[i] = 0;
			}
		}
		if (TEXT)
			vdc8563_text_screenrefresh(device, bitmap, full_refresh);
		else
			vdc8563_monotext_screenrefresh(device, bitmap, full_refresh);
	}

	if (full_refresh)
	{
		int w = CRTC6845_CHAR_COLUMNS;
		int h = CRTC6845_CHAR_LINES;
		int height = CRTC6845_CHAR_HEIGHT;

		bitmap.plot_box(0, 0, device->machine().gfx[0]->width * (w + 2), height, FRAMECOLOR);

		bitmap.plot_box(0, height, device->machine().gfx[0]->width, height * h, FRAMECOLOR);

		bitmap.plot_box(device->machine().gfx[0]->width * (w + 1), height, device->machine().gfx[0]->width, height * h, FRAMECOLOR);

		bitmap.plot_box(0, height * (h + 1), device->machine().gfx[0]->width * (w + 2), height, FRAMECOLOR);
	}

	vdc8563->changed = 0;
	return 0;
}

/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

static DEVICE_START( vdc8563 )
{
	vdc8563_state *vdc8563 = get_safe_token(device);
	const vdc8563_interface *intf = (vdc8563_interface *)device->static_config();

	vdc8563->screen = device->machine().device<screen_device>(intf->screen);

	vdc8563->ram = auto_alloc_array_clear(device->machine(), UINT8, 0x20000);
	vdc8563->dirty = vdc8563->ram + 0x10000;

	/* currently no driver uses 16k only */
	if (intf->ram16konly)
	{
		vdc8563->mask = 0x3fff;
		vdc8563->fontmask = 0x2000;
	}
	else
	{
		vdc8563->mask = 0xffff;
		vdc8563->fontmask = 0xe000;
	}

	device->save_pointer(NAME(vdc8563->ram), 0x20000);

	device->save_item(NAME(vdc8563->reg));
	device->save_item(NAME(vdc8563->state));
	device->save_item(NAME(vdc8563->index));

	device->save_item(NAME(vdc8563->addr));
	device->save_item(NAME(vdc8563->src));

	device->save_item(NAME(vdc8563->videoram_start));
	device->save_item(NAME(vdc8563->colorram_start));
	device->save_item(NAME(vdc8563->fontram_start));
	device->save_item(NAME(vdc8563->videoram_size));

	device->save_item(NAME(vdc8563->rastering));

	device->save_item(NAME(vdc8563->fontdirty));

	device->save_item(NAME(vdc8563->cursor_time));
	device->save_item(NAME(vdc8563->cursor_on));

	device->save_item(NAME(vdc8563->changed));
}


static DEVICE_RESET( vdc8563 )
{
	vdc8563_state *vdc8563 = get_safe_token(device);

	memset(vdc8563->reg, 0, ARRAY_LENGTH(vdc8563->reg));
	memset(vdc8563->fontdirty, 0, ARRAY_LENGTH(vdc8563->fontdirty));

	vdc8563->cursor_time = 0.0;
	vdc8563->state = 1;

	vdc8563->index = 0;
	vdc8563->addr = 0;
	vdc8563->src = 0;
	vdc8563->videoram_start = 0;
	vdc8563->colorram_start = 0;
	vdc8563->fontram_start = 0;
	vdc8563->videoram_size = 0;
	vdc8563->rastering = 1;
	vdc8563->cursor_on = 0;
	vdc8563->changed = 0;
}

/*-------------------------------------------------
    device definition
-------------------------------------------------*/

static const char DEVTEMPLATE_SOURCE[] = __FILE__;

#define DEVTEMPLATE_ID(p,s)				p##vdc8563##s
#define DEVTEMPLATE_FEATURES			DT_HAS_START | DT_HAS_RESET
#define DEVTEMPLATE_NAME				"8563 / 8568 VDC"
#define DEVTEMPLATE_FAMILY				"8563 / 8568 VDC"
#include "devtempl.h"

DEFINE_LEGACY_DEVICE(VDC8563, vdc8563);
