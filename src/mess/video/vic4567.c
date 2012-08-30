/***************************************************************************

    Video Interface Chip (4567)

    original emulation by PeT (mess@utanet.at)

    2010-02: converted to be a device and split from vic II

    TODO:
      - plenty of cleanups
      - emulate variants of the vic chip
      - update vic III to use the new vic6567.c code for the vic II comaptibility

***************************************************************************/

#include "emu.h"
#include "video/vic4567.h"

#define SPRITE_BASE_X_SIZE		24
#define SPRITE_BASE_Y_SIZE		21

struct vic3_sprite
{
	int x, y;

	int repeat;                         /* expand, line once drawn */
	int line;                           /* 0 not painting, else painting */

	/* buffer for currently painted line */
	int paintedline[8];
	UINT8 bitmap[8][SPRITE_BASE_X_SIZE * 2 / 8 + 1  /*for simplier sprite collision detection*/];
};

typedef struct _vic3_state  vic3_state;
struct _vic3_state
{
	vic3_type  type;

	screen_device *main_screen;			// screen which sets bitmap properties

	device_t *cpu;

	UINT8 reg[0x80];
	int on;								/* rastering of the screen */

	int lines;

	UINT16 chargenaddr, videoaddr, bitmapaddr;

	bitmap_ind16 *bitmap;
	int x_begin, x_end;
	int y_begin, y_end;

	UINT16 c64_bitmap[2], bitmapmulti[4], mono[2], multi[4], ecmcolor[2], colors[4], spritemulti[4];

	int lastline, rasterline;

	int interlace;
	int columns, rows;

	/* background/foreground for sprite collision */
	UINT8 *screen[216], shift[216];

	/* convert multicolor byte to background/foreground for sprite collision */
	UINT8 foreground[256];
	UINT16 expandx[256];
	UINT16 expandx_multi[256];

	/* converts sprite multicolor info to info for background collision checking */
	UINT8 multi_collision[256];

	vic3_sprite sprites[8];

	/* DMA */
	vic3_dma_read          dma_read;
	vic3_dma_read_color    dma_read_color;

	/* IRQ */
	vic3_irq               interrupt;

	/* Port Changed */
	vic3_port_changed_callback      port_changed;

	/* lightpen */
	vic3_lightpen_button_callback lightpen_button_cb;
	vic3_lightpen_x_callback lightpen_x_cb;
	vic3_lightpen_y_callback lightpen_y_cb;

	/* C64 memory access */
	vic3_c64mem_callback      c64_mem_r;

	/* palette - vic3 specific items (the ones above are used for VIC II as well) */
	UINT8 palette_red[0x100];
	UINT8 palette_green[0x100];
	UINT8 palette_blue[0x100];
	int palette_dirty;
};

/*****************************************************************************
    CONSTANTS
*****************************************************************************/

#define VERBOSE_LEVEL 0
#define DBG_LOG(N,M,A) \
	do { \
		if(VERBOSE_LEVEL >= N) \
		{ \
			if( M ) \
				logerror("%11.6f: %-24s", device->machine().time().as_double(), (char*) M ); \
			logerror A; \
		} \
	} while (0)

#define VREFRESHINLINES			28

#define VIC2_YPOS				50
#define RASTERLINE_2_C64(a)		(a)
#define C64_2_RASTERLINE(a)		(a)
#define XPOS				(VIC2_STARTVISIBLECOLUMNS + (VIC2_VISIBLECOLUMNS - VIC2_HSIZE) / 2)
#define YPOS				(VIC2_STARTVISIBLELINES /* + (VIC2_VISIBLELINES - VIC2_VSIZE) / 2 */)
#define FIRSTLINE				10 /* 36 ((VIC2_VISIBLELINES - VIC2_VSIZE)/2) */
#define FIRSTCOLUMN			50

/* 2008-05 FP: lightpen code needs to read input port from c64.c and cbmb.c */

#define LIGHTPEN_BUTTON		(vic3->lightpen_button_cb(machine))
#define LIGHTPEN_X_VALUE	(vic3->lightpen_x_cb(machine))
#define LIGHTPEN_Y_VALUE	(vic3->lightpen_y_cb(machine))

/* lightpen delivers values from internal counters; they do not start with the visual area or frame area */
#define VIC2_MAME_XPOS			0
#define VIC2_MAME_YPOS			0
#define VIC6567_X_BEGIN			38
#define VIC6567_Y_BEGIN			-6			   /* first 6 lines after retrace not for lightpen! */
#define VIC6569_X_BEGIN			38
#define VIC6569_Y_BEGIN			-6
#define VIC2_X_BEGIN			((vic3->type == VIC4567_PAL) ? VIC6569_X_BEGIN : VIC6567_X_BEGIN)
#define VIC2_Y_BEGIN			((vic3->type == VIC4567_PAL) ? VIC6569_Y_BEGIN : VIC6567_Y_BEGIN)
#define VIC2_X_VALUE			((LIGHTPEN_X_VALUE + VIC2_X_BEGIN + VIC2_MAME_XPOS) / 2)
#define VIC2_Y_VALUE			((LIGHTPEN_Y_VALUE + VIC2_Y_BEGIN + VIC2_MAME_YPOS))

#define VIC2E_K0_LEVEL			(vic3->reg[0x2f] & 0x01)
#define VIC2E_K1_LEVEL			(vic3->reg[0x2f] & 0x02)
#define VIC2E_K2_LEVEL			(vic3->reg[0x2f] & 0x04)

/*#define VIC3_P5_LEVEL (vic3->reg[0x30] & 0x20) */
#define VIC3_BITPLANES			(vic3->reg[0x31] & 0x10)
#define VIC3_80COLUMNS			(vic3->reg[0x31] & 0x80)
#define VIC3_LINES			((vic3->reg[0x31] & 0x19) == 0x19 ? 400 : 200)
#define VIC3_BITPLANES_WIDTH		(vic3->reg[0x31] & 0x80 ? 640 : 320)

/*#define VIC2E_TEST (vic2[0x30] & 2) */
#define DOUBLE_CLOCK			(vic3->reg[0x30] & 0x01)

/* sprites 0 .. 7 */
#define SPRITEON(nr)			(vic3->reg[0x15] & (1 << nr))
#define SPRITE_Y_EXPAND(nr)		(vic3->reg[0x17] & (1 << nr))
#define SPRITE_Y_SIZE(nr)		(SPRITE_Y_EXPAND(nr) ? 2 * 21 : 21)
#define SPRITE_X_EXPAND(nr)		(vic3->reg[0x1d] & (1 << nr))
#define SPRITE_X_SIZE(nr)		(SPRITE_X_EXPAND(nr) ? 2 * 24 : 24)
#define SPRITE_X_POS(nr)		((vic3->reg[(nr) * 2] | (vic3->reg[0x10] & (1 <<(nr)) ? 0x100 : 0)) - 24 + XPOS)
#define SPRITE_X_POS2(nr)		(vic3->reg[(nr) * 2] | (vic3->reg[0x10] & (1 <<(nr)) ? 0x100 : 0))
#define SPRITE_Y_POS(nr)		(vic3->reg[1+2*(nr)] - 50 + YPOS)
#define SPRITE_Y_POS2(nr)		(vic3->reg[1 + 2 *(nr)])
#define SPRITE_MULTICOLOR(nr)		(vic3->reg[0x1c] & (1 << nr))
#define SPRITE_PRIORITY(nr)		(vic3->reg[0x1b] & (1 << nr))
#define SPRITE_MULTICOLOR1		(vic3->reg[0x25] & 0x0f)
#define SPRITE_MULTICOLOR2		(vic3->reg[0x26] & 0x0f)
#define SPRITE_COLOR(nr)		(vic3->reg[0x27+nr] & 0x0f)
#define SPRITE_ADDR(nr)			(vic3->videoaddr | 0x3f8 | nr)
#define SPRITE_BG_COLLISION(nr)	(vic3->reg[0x1f] & (1 << nr))
#define SPRITE_COLLISION(nr)		(vic3->reg[0x1e] & (1 << nr))
#define SPRITE_SET_BG_COLLISION(nr) (vic3->reg[0x1f] |= (1 << nr))
#define SPRITE_SET_COLLISION(nr)	(vic3->reg[0x1e] |= (1 << nr))
#define SPRITE_COLL			(vic3->reg[0x1e])
#define SPRITE_BG_COLL			(vic3->reg[0x1f])

#define GFXMODE				((vic3->reg[0x11] & 0x60) | (vic3->reg[0x16] & 0x10)) >> 4
#define SCREENON				(vic3->reg[0x11] & 0x10)
#define VERTICALPOS			(vic3->reg[0x11] & 0x07)
#define HORIZONTALPOS			(vic3->reg[0x16] & 0x07)
#define ECMON				(vic3->reg[0x11] & 0x40)
#define HIRESON				(vic3->reg[0x11] & 0x20)
#define MULTICOLORON			(vic3->reg[0x16] & 0x10)
#define LINES25				(vic3->reg[0x11] & 0x08)		   /* else 24 Lines */
#define LINES				(LINES25 ? 25 : 24)
#define YSIZE				(LINES * 8)
#define COLUMNS40				(vic3->reg[0x16] & 0x08)		   /* else 38 Columns */
#define COLUMNS				(COLUMNS40 ? 40 : 38)
#define XSIZE				(COLUMNS * 8)

#define VIDEOADDR				((vic3->reg[0x18] & 0xf0) << (10 - 4))
#define CHARGENADDR			((vic3->reg[0x18] & 0x0e) << 10)
#define BITMAPADDR			((data & 0x08) << 10)

#define RASTERLINE			(((vic3->reg[0x11] & 0x80) << 1) | vic3->reg[0x12])

#define FRAMECOLOR			(vic3->reg[0x20] & 0x0f)
#define BACKGROUNDCOLOR			(vic3->reg[0x21] & 0x0f)
#define MULTICOLOR1			(vic3->reg[0x22] & 0x0f)
#define MULTICOLOR2			(vic3->reg[0x23] & 0x0f)
#define FOREGROUNDCOLOR			(vic3->reg[0x24] & 0x0f)


#define VIC2_LINES		(vic3->type == VIC4567_PAL ? VIC6569_LINES : VIC6567_LINES)
#define VIC2_VISIBLELINES	(vic3->type == VIC4567_PAL ? VIC6569_VISIBLELINES : VIC6567_VISIBLELINES)
#define VIC2_VISIBLECOLUMNS	(vic3->type == VIC4567_PAL ? VIC6569_VISIBLECOLUMNS : VIC6567_VISIBLECOLUMNS)
#define VIC2_STARTVISIBLELINES ((VIC2_LINES - VIC2_VISIBLELINES)/2)
#define VIC2_FIRSTRASTERLINE  (vic3->type == VIC4567_PAL ? VIC6569_FIRSTRASTERLINE : VIC6567_FIRSTRASTERLINE)
#define VIC2_COLUMNS          (vic3->type == VIC4567_PAL ? VIC6569_COLUMNS : VIC6567_COLUMNS)
#define VIC2_STARTVISIBLECOLUMNS ((VIC2_COLUMNS - VIC2_VISIBLECOLUMNS)/2)

#define VIC3_BITPLANES_MASK (vic3->reg[0x32])
/* bit 0, 4 not used !?*/
/* I think hinibbles contains the banknumbers for interlaced modes */
/* if hinibble set then x&1==0 should be in bank1 (0x10000), x&1==1 in bank 0 */
#define VIC3_BITPLANE_ADDR_HELPER(x)  ((vic3->reg[0x33 + x] & 0x0f) << 12)
#define VIC3_BITPLANE_ADDR(x) (x & 1 ? VIC3_BITPLANE_ADDR_HELPER(x) + 0x10000 : VIC3_BITPLANE_ADDR_HELPER(x) )
#define VIC3_BITPLANE_IADDR_HELPER(x)  ((vic3->reg[0x33 + x] & 0xf0) << 8)
#define VIC3_BITPLANE_IADDR(x) (x & 1 ? VIC3_BITPLANE_IADDR_HELPER(x) + 0x10000 : VIC3_BITPLANE_IADDR_HELPER(x))


/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE vic3_state *get_safe_token( device_t *device )
{
	assert(device != NULL);
	assert(device->type() == VIC3);

	return (vic3_state *)downcast<legacy_device_base *>(device)->token();
}

INLINE const vic3_interface *get_interface( device_t *device )
{
	assert(device != NULL);
	assert((device->type() == VIC3));
	return (const vic3_interface *) device->static_config();
}

/*****************************************************************************
    IMPLEMENTATION
*****************************************************************************/

INLINE int vic3_getforeground( device_t *device, int y, int x )
{
	vic3_state *vic3 = get_safe_token(device);
	return ((vic3->screen[y][x >> 3] << 8) | (vic3->screen[y][(x >> 3) + 1])) >> (8 - (x & 7));
}

INLINE int vic3_getforeground16(device_t *device, int y, int x )
{
	vic3_state *vic3 = get_safe_token(device);
	return ((vic3->screen[y][x >> 3] << 16) | (vic3->screen[y][(x >> 3) + 1] << 8) | (vic3->screen[y][(x >> 3) + 2])) >> (8 - (x & 7));
}

static void vic3_set_interrupt( running_machine &machine, int mask, vic3_state *vic3 )
{
	if (((vic3->reg[0x19] ^ mask) & vic3->reg[0x1a] & 0xf))
	{
		if (!(vic3->reg[0x19] & 0x80))
		{
			//DBG_LOG(2, "vic2", ("irq start %.2x\n", mask));
			vic3->reg[0x19] |= 0x80;
			vic3->interrupt(machine, 1);
		}
	}
	vic3->reg[0x19] |= mask;
}

static void vic3_clear_interrupt( running_machine &machine, int mask, vic3_state *vic3 )
{
	vic3->reg[0x19] &= ~mask;
	if ((vic3->reg[0x19] & 0x80) && !(vic3->reg[0x19] & vic3->reg[0x1a] & 0xf))
	{
		//DBG_LOG(2, "vic2", ("irq end %.2x\n", mask));
		vic3->reg[0x19] &= ~0x80;
		vic3->interrupt(machine, 0);
	}
}

static TIMER_CALLBACK(vic3_timer_timeout)
{
	vic3_state *vic3 = (vic3_state *)ptr;
	int which = param;
	//DBG_LOG(3, "vic3 ", ("timer %d timeout\n", which));
	switch (which)
	{
	case 1:						   /* light pen */
		/* and diode must recognize light */
		if (1)
		{
			vic3->reg[0x13] = VIC2_X_VALUE;
			vic3->reg[0x14] = VIC2_Y_VALUE;
		}
		vic3_set_interrupt(machine, 8, vic3);
		break;
	}
}
static void vic3_draw_character( device_t *device, int ybegin, int yend, int ch, int yoff, int xoff, UINT16 *color, int start_x, int end_x )
{
	vic3_state *vic3 = get_safe_token(device);
	int y, code;

	for (y = ybegin; y <= yend; y++)
	{
		code = vic3->dma_read(device->machine(), vic3->chargenaddr + ch * 8 + y);
		vic3->screen[y + yoff][xoff >> 3] = code;
		if ((xoff + 0 > start_x) && (xoff + 0 < end_x)) vic3->bitmap->pix16(y + yoff + FIRSTLINE, xoff + 0) = color[code >> 7];
		if ((xoff + 1 > start_x) && (xoff + 0 < end_x)) vic3->bitmap->pix16(y + yoff + FIRSTLINE, xoff + 1) = color[(code >> 6) & 1];
		if ((xoff + 2 > start_x) && (xoff + 0 < end_x)) vic3->bitmap->pix16(y + yoff + FIRSTLINE, xoff + 2) = color[(code >> 5) & 1];
		if ((xoff + 3 > start_x) && (xoff + 0 < end_x)) vic3->bitmap->pix16(y + yoff + FIRSTLINE, xoff + 3) = color[(code >> 4) & 1];
		if ((xoff + 4 > start_x) && (xoff + 0 < end_x)) vic3->bitmap->pix16(y + yoff + FIRSTLINE, xoff + 4) = color[(code >> 3) & 1];
		if ((xoff + 5 > start_x) && (xoff + 0 < end_x)) vic3->bitmap->pix16(y + yoff + FIRSTLINE, xoff + 5) = color[(code >> 2) & 1];
		if ((xoff + 6 > start_x) && (xoff + 0 < end_x)) vic3->bitmap->pix16(y + yoff + FIRSTLINE, xoff + 6) = color[(code >> 1) & 1];
		if ((xoff + 7 > start_x) && (xoff + 0 < end_x)) vic3->bitmap->pix16(y + yoff + FIRSTLINE, xoff + 7) = color[code & 1];
	}
}

static void vic3_draw_character_multi( device_t *device, int ybegin, int yend, int ch, int yoff, int xoff, int start_x, int end_x )
{
	vic3_state *vic3 = get_safe_token(device);
	int y, code;

	for (y = ybegin; y <= yend; y++)
	{
		code = vic3->dma_read(device->machine(), vic3->chargenaddr + ch * 8 + y);
		vic3->screen[y + yoff][xoff >> 3] = vic3->foreground[code];
		if ((xoff + 0 > start_x) && (xoff + 0 < end_x)) vic3->bitmap->pix16(y + yoff + FIRSTLINE, xoff + 0) = vic3->multi[code >> 6];
		if ((xoff + 1 > start_x) && (xoff + 1 < end_x)) vic3->bitmap->pix16(y + yoff + FIRSTLINE, xoff + 1) = vic3->multi[code >> 6];
		if ((xoff + 2 > start_x) && (xoff + 2 < end_x)) vic3->bitmap->pix16(y + yoff + FIRSTLINE, xoff + 2) = vic3->multi[(code >> 4) & 3];
		if ((xoff + 3 > start_x) && (xoff + 3 < end_x)) vic3->bitmap->pix16(y + yoff + FIRSTLINE, xoff + 3) = vic3->multi[(code >> 4) & 3];
		if ((xoff + 4 > start_x) && (xoff + 4 < end_x)) vic3->bitmap->pix16(y + yoff + FIRSTLINE, xoff + 4) = vic3->multi[(code >> 2) & 3];
		if ((xoff + 5 > start_x) && (xoff + 5 < end_x)) vic3->bitmap->pix16(y + yoff + FIRSTLINE, xoff + 5) = vic3->multi[(code >> 2) & 3];
		if ((xoff + 6 > start_x) && (xoff + 6 < end_x)) vic3->bitmap->pix16(y + yoff + FIRSTLINE, xoff + 6) = vic3->multi[code & 3];
		if ((xoff + 7 > start_x) && (xoff + 7 < end_x)) vic3->bitmap->pix16(y + yoff + FIRSTLINE, xoff + 7) = vic3->multi[code & 3];
	}
}

static void vic3_draw_bitmap( device_t *device, int ybegin, int yend, int ch, int yoff, int xoff, int start_x, int end_x )
{
	vic3_state *vic3 = get_safe_token(device);
	int y, code;

	for (y = ybegin; y <= yend; y++)
	{
		code = vic3->dma_read(device->machine(), (vic3->chargenaddr & 0x2000) + ch * 8 + y);
		vic3->screen[y + yoff][xoff >> 3] = code;
		if ((xoff + 0 > start_x) && (xoff + 0 < end_x)) vic3->bitmap->pix16(y + yoff + FIRSTLINE, xoff + 0) = vic3->c64_bitmap[code >> 7];
		if ((xoff + 1 > start_x) && (xoff + 1 < end_x)) vic3->bitmap->pix16(y + yoff + FIRSTLINE, xoff + 1) = vic3->c64_bitmap[(code >> 6) & 1];
		if ((xoff + 2 > start_x) && (xoff + 2 < end_x)) vic3->bitmap->pix16(y + yoff + FIRSTLINE, xoff + 2) = vic3->c64_bitmap[(code >> 5) & 1];
		if ((xoff + 3 > start_x) && (xoff + 3 < end_x)) vic3->bitmap->pix16(y + yoff + FIRSTLINE, xoff + 3) = vic3->c64_bitmap[(code >> 4) & 1];
		if ((xoff + 4 > start_x) && (xoff + 4 < end_x)) vic3->bitmap->pix16(y + yoff + FIRSTLINE, xoff + 4) = vic3->c64_bitmap[(code >> 3) & 1];
		if ((xoff + 5 > start_x) && (xoff + 5 < end_x)) vic3->bitmap->pix16(y + yoff + FIRSTLINE, xoff + 5) = vic3->c64_bitmap[(code >> 2) & 1];
		if ((xoff + 6 > start_x) && (xoff + 6 < end_x)) vic3->bitmap->pix16(y + yoff + FIRSTLINE, xoff + 6) = vic3->c64_bitmap[(code >> 1) & 1];
		if ((xoff + 7 > start_x) && (xoff + 7 < end_x)) vic3->bitmap->pix16(y + yoff + FIRSTLINE, xoff + 7) = vic3->c64_bitmap[code & 1];
	}
}

static void vic3_draw_bitmap_multi( device_t *device, int ybegin, int yend, int ch, int yoff, int xoff, int start_x, int end_x )
{
	vic3_state *vic3 = get_safe_token(device);
	int y, code;

	for (y = ybegin; y <= yend; y++)
	{
		code = vic3->dma_read(device->machine(), (vic3->chargenaddr & 0x2000) + ch * 8 + y);
		vic3->screen[y + yoff][xoff >> 3] = vic3->foreground[code];
		if ((xoff + 0 > start_x) && (xoff + 0 < end_x)) vic3->bitmap->pix16(y + yoff + FIRSTLINE, xoff + 0) = vic3->bitmapmulti[code >> 6];
		if ((xoff + 1 > start_x) && (xoff + 1 < end_x)) vic3->bitmap->pix16(y + yoff + FIRSTLINE, xoff + 1) = vic3->bitmapmulti[code >> 6];
		if ((xoff + 2 > start_x) && (xoff + 2 < end_x)) vic3->bitmap->pix16(y + yoff + FIRSTLINE, xoff + 2) = vic3->bitmapmulti[(code >> 4) & 3];
		if ((xoff + 3 > start_x) && (xoff + 3 < end_x)) vic3->bitmap->pix16(y + yoff + FIRSTLINE, xoff + 3) = vic3->bitmapmulti[(code >> 4) & 3];
		if ((xoff + 4 > start_x) && (xoff + 4 < end_x)) vic3->bitmap->pix16(y + yoff + FIRSTLINE, xoff + 4) = vic3->bitmapmulti[(code >> 2) & 3];
		if ((xoff + 5 > start_x) && (xoff + 5 < end_x)) vic3->bitmap->pix16(y + yoff + FIRSTLINE, xoff + 5) = vic3->bitmapmulti[(code >> 2) & 3];
		if ((xoff + 6 > start_x) && (xoff + 6 < end_x)) vic3->bitmap->pix16(y + yoff + FIRSTLINE, xoff + 6) = vic3->bitmapmulti[code & 3];
		if ((xoff + 7 > start_x) && (xoff + 7 < end_x)) vic3->bitmap->pix16(y + yoff + FIRSTLINE, xoff + 7) = vic3->bitmapmulti[code & 3];
	}
}

static void vic3_draw_sprite_code( device_t *device, int y, int xbegin, int code, int color, int start_x, int end_x )
{
	vic3_state *vic3 = get_safe_token(device);
	register int mask, x;

	if ((y < YPOS) || (y >= (VIC2_STARTVISIBLELINES + VIC2_VISIBLELINES)) || (xbegin <= 1) || (xbegin >= (VIC2_STARTVISIBLECOLUMNS + VIC2_VISIBLECOLUMNS)))
		return;

	for (x = 0, mask = 0x80; x < 8; x++, mask >>= 1)
	{
		if (code & mask)
		{
			if ((xbegin + x > start_x) && (xbegin + x < end_x))
				vic3->bitmap->pix16(y + FIRSTLINE, xbegin + x) = color;
		}
	}
}

static void vic3_draw_sprite_code_multi( device_t *device, int y, int xbegin, int code, int prior, int start_x, int end_x )
{
	vic3_state *vic3 = get_safe_token(device);
	register int x, mask, shift;

	if ((y < YPOS) || (y >= (VIC2_STARTVISIBLELINES + VIC2_VISIBLELINES)) || (xbegin <= 1) || (xbegin >= (VIC2_STARTVISIBLECOLUMNS + VIC2_VISIBLECOLUMNS)))
		{
			return;
		}

	for (x = 0, mask = 0xc0, shift = 6; x < 8; x += 2, mask >>= 2, shift -= 2)
	{
		if (code & mask)
		{
			switch ((prior & mask) >> shift)
			{
			case 1:
				if ((xbegin + x + 1 > start_x) && (xbegin + x + 1 < end_x))
					vic3->bitmap->pix16(y + FIRSTLINE, xbegin + x + 1) = vic3->spritemulti[(code >> shift) & 3];
				break;
			case 2:
				if ((xbegin + x > start_x) && (xbegin + x < end_x))
					vic3->bitmap->pix16(y + FIRSTLINE, xbegin + x) = vic3->spritemulti[(code >> shift) & 3];
				break;
			case 3:
				if ((xbegin + x > start_x) && (xbegin + x < end_x))
					vic3->bitmap->pix16(y + FIRSTLINE, xbegin + x) = vic3->spritemulti[(code >> shift) & 3];
				if ((xbegin + x + 1> start_x) && (xbegin + x + 1< end_x))
					vic3->bitmap->pix16(y + FIRSTLINE, xbegin + x + 1) = vic3->spritemulti[(code >> shift) & 3];
				break;
			}
		}
	}
}

static void vic3_sprite_collision( device_t *device, int nr, int y, int x, int mask )
{
	vic3_state *vic3 = get_safe_token(device);
	int i, value, xdiff;

	for (i = 7; i > nr; i--)
	{
		if (!SPRITEON(i) || !vic3->sprites[i].paintedline[y] || (SPRITE_COLLISION(i) && SPRITE_COLLISION(nr)))
			continue;

		if ((x + 7 < SPRITE_X_POS(i)) || (x >= SPRITE_X_POS(i) + SPRITE_X_SIZE(i)))
			continue;

		xdiff = x - SPRITE_X_POS(i);

		if ((x & 7) == (SPRITE_X_POS(i) & 7))
			value = vic3->sprites[i].bitmap[y][xdiff >> 3];
		else if (xdiff < 0)
			value = vic3->sprites[i].bitmap[y][0] >> (-xdiff);
		else {
			UINT8 *vp = vic3->sprites[i].bitmap[y]+(xdiff >> 3);
			value = ((vp[1] | (*vp << 8)) >> (8 - (xdiff & 7) )) & 0xff;
		}

		if (value & mask)
		{
			SPRITE_SET_COLLISION(i);
			SPRITE_SET_COLLISION(nr);
			vic3_set_interrupt(device->machine(), 4, vic3);
		}
	}
}

static void vic3_draw_sprite( device_t *device, int nr, int yoff, int ybegin, int yend, int start_x, int end_x )
{
	vic3_state *vic3 = get_safe_token(device);
	int y, i, addr, xbegin, color, prior, collision;
	int value, value3 = 0;

	xbegin = SPRITE_X_POS(nr);
	addr = vic3->dma_read(device->machine(), SPRITE_ADDR(nr)) << 6;
	color = SPRITE_COLOR(nr);
	prior = SPRITE_PRIORITY(nr);
	collision = SPRITE_BG_COLLISION(nr);

	if (SPRITE_X_EXPAND(nr))
	{
		for (y = ybegin; y <= yend; y++)
		{
			vic3->sprites[nr].paintedline[y] = 1;
			for (i = 0; i < 3; i++)
			{
				value = vic3->expandx[vic3->dma_read(device->machine(), addr + vic3->sprites[nr].line * 3 + i)];
				vic3->sprites[nr].bitmap[y][i * 2] = value >> 8;
				vic3->sprites[nr].bitmap[y][i * 2 + 1] = value & 0xff;
				vic3_sprite_collision(device, nr, y, xbegin + i * 16, value >> 8);
				vic3_sprite_collision(device, nr, y, xbegin + i * 16 + 8, value & 0xff);
				if (prior || !collision)
					value3 = vic3_getforeground16(device, yoff + y, xbegin + i * 16 - 7);
				if (!collision && (value & value3))
				{
					collision = 1;
					SPRITE_SET_BG_COLLISION(nr);
					vic3_set_interrupt(device->machine(), 2, vic3);
				}
				if (prior)
					value &= ~value3;
				vic3_draw_sprite_code(device, yoff + y, xbegin + i * 16, value >> 8, color, start_x, end_x);
				vic3_draw_sprite_code(device, yoff + y, xbegin + i * 16 + 8, value & 0xff, color, start_x, end_x);
			}
			vic3->sprites[nr].bitmap[y][i * 2]=0; //easier sprite collision detection
			if (SPRITE_Y_EXPAND(nr))
			{
				if (vic3->sprites[nr].repeat)
				{
					vic3->sprites[nr].line++;
					vic3->sprites[nr].repeat = 0;
				}
				else
					vic3->sprites[nr].repeat = 1;
			}
			else
			{
				vic3->sprites[nr].line++;
			}
		}
	}
	else
	{
		for (y = ybegin; y <= yend; y++)
		{
			vic3->sprites[nr].paintedline[y] = 1;
			for (i = 0; i < 3; i++)
			{
				value = vic3->dma_read(device->machine(), addr + vic3->sprites[nr].line * 3 + i);
				vic3->sprites[nr].bitmap[y][i] = value;
				vic3_sprite_collision(device, nr, y, xbegin + i * 8, value);
				if (prior || !collision)
					value3 = vic3_getforeground(device, yoff + y, xbegin + i * 8 - 7);
				if (!collision && (value & value3))
				{
					collision = 1;
					SPRITE_SET_BG_COLLISION(nr);
					vic3_set_interrupt(device->machine(), 2, vic3);
				}
				if (prior)
					value &= ~value3;
				vic3_draw_sprite_code(device, yoff + y, xbegin + i * 8, value, color, start_x, end_x);
			}
			vic3->sprites[nr].bitmap[y][i]=0; //easier sprite collision detection
			if (SPRITE_Y_EXPAND(nr))
			{
				if (vic3->sprites[nr].repeat)
				{
					vic3->sprites[nr].line++;
					vic3->sprites[nr].repeat = 0;
				}
				else
					vic3->sprites[nr].repeat = 1;
			}
			else
			{
				vic3->sprites[nr].line++;
			}
		}
	}
}

static void vic3_draw_sprite_multi( device_t *device, int nr, int yoff, int ybegin, int yend, int start_x, int end_x )
{
	vic3_state *vic3 = get_safe_token(device);
	int y, i, prior, addr, xbegin, collision;
	int value, value2, value3 = 0, bg/*, color[2]*/;

	xbegin = SPRITE_X_POS(nr);
	addr = vic3->dma_read(device->machine(), SPRITE_ADDR(nr)) << 6;
	vic3->spritemulti[2] = SPRITE_COLOR(nr);
	prior = SPRITE_PRIORITY(nr);
	collision = SPRITE_BG_COLLISION(nr);
	//color[0] = 0;
	//color[1] = 1;

	if (SPRITE_X_EXPAND(nr))
	{
		for (y = ybegin; y <= yend; y++)
		{
			vic3->sprites[nr].paintedline[y] = 1;
			for (i = 0; i < 3; i++)
			{
				value = vic3->expandx_multi[bg = vic3->dma_read(device->machine(), addr + vic3->sprites[nr].line * 3 + i)];
				value2 = vic3->expandx[vic3->multi_collision[bg]];
				vic3->sprites[nr].bitmap[y][i * 2] = value2 >> 8;
				vic3->sprites[nr].bitmap[y][i * 2 + 1] = value2 & 0xff;
				vic3_sprite_collision(device, nr, y, xbegin + i * 16, value2 >> 8);
				vic3_sprite_collision(device, nr, y, xbegin + i * 16 + 8, value2 & 0xff);
				if (prior || !collision)
				{
					value3 = vic3_getforeground16(device, yoff + y, xbegin + i * 16 - 7);
				}
				if (!collision && (value2 & value3))
				{
					collision = 1;
					SPRITE_SET_BG_COLLISION(nr);
					vic3_set_interrupt(device->machine(), 2, vic3);
				}
				if (prior)
				{
					vic3_draw_sprite_code_multi(device, yoff + y, xbegin + i * 16, value >> 8, (value3 >> 8) ^ 0xff, start_x, end_x);
					vic3_draw_sprite_code_multi(device, yoff + y, xbegin + i * 16 + 8, value & 0xff, (value3 & 0xff) ^ 0xff, start_x, end_x);
				}
				else
				{
					vic3_draw_sprite_code_multi(device, yoff + y, xbegin + i * 16, value >> 8, 0xff, start_x, end_x);
					vic3_draw_sprite_code_multi(device, yoff + y, xbegin + i * 16 + 8, value & 0xff, 0xff, start_x, end_x);
				}
			}
			vic3->sprites[nr].bitmap[y][i * 2]=0; //easier sprite collision detection
			if (SPRITE_Y_EXPAND(nr))
			{
				if (vic3->sprites[nr].repeat)
				{
					vic3->sprites[nr].line++;
					vic3->sprites[nr].repeat = 0;
				}
				else
					vic3->sprites[nr].repeat = 1;
			}
			else
			{
				vic3->sprites[nr].line++;
			}
		}
	}
	else
	{
		for (y = ybegin; y <= yend; y++)
		{
			vic3->sprites[nr].paintedline[y] = 1;
			for (i = 0; i < 3; i++)
			{
				value = vic3->dma_read(device->machine(), addr + vic3->sprites[nr].line * 3 + i);
				vic3->sprites[nr].bitmap[y][i] = value2 = vic3->multi_collision[value];
				vic3_sprite_collision(device, nr, y, xbegin + i * 8, value2);
				if (prior || !collision)
				{
					value3 = vic3_getforeground(device, yoff + y, xbegin + i * 8 - 7);
				}
				if (!collision && (value2 & value3))
				{
					collision = 1;
					SPRITE_SET_BG_COLLISION(nr);
					vic3_set_interrupt(device->machine(), 2, vic3);
				}
				if (prior)
				{
					vic3_draw_sprite_code_multi(device, yoff + y, xbegin + i * 8, value, value3 ^ 0xff, start_x, end_x);
				}
				else
				{
					vic3_draw_sprite_code_multi(device, yoff + y, xbegin + i * 8, value, 0xff, start_x, end_x);
				}
			}
			vic3->sprites[nr].bitmap[y][i] = 0; //easier sprite collision detection
			if (SPRITE_Y_EXPAND(nr))
			{
				if (vic3->sprites[nr].repeat)
				{
					vic3->sprites[nr].line++;
					vic3->sprites[nr].repeat = 0;
				}
				else
					vic3->sprites[nr].repeat = 1;
			}
			else
			{
				vic3->sprites[nr].line++;
			}
		}
	}
}

#ifndef memset16
static void *memset16 (void *dest, int value, size_t size)
{
	register int i;

	for (i = 0; i < size; i++)
		((short *) dest)[i] = value;
	return dest;
}
#endif

static void vic3_drawlines( device_t *device, int first, int last, int start_x, int end_x )
{
	vic3_state *vic3 = get_safe_token(device);
	int line, vline, end;
	int attr, ch, ecm;
	int syend;
	int offs, yoff, xoff, ybegin, yend, xbegin, xend;
	int x_end2;
	int i, j;

	if (first == last)
		return;
	vic3->lastline = last;

	/* top part of display not rastered */
	first -= VIC2_YPOS - YPOS;
	last -= VIC2_YPOS - YPOS;
	if ((first >= last) || (last <= 0))
	{
		for (i = 0; i < 8; i++)
			vic3->sprites[i].repeat = vic3->sprites[i].line = 0;
		return;
	}
	if (first < 0)
		first = 0;

	if (!SCREENON)
	{
		for (line = first; (line < last) && (line < vic3->bitmap->height()); line++)
		{
			memset16(&vic3->bitmap->pix16(line + FIRSTLINE), 0, vic3->bitmap->width());
		}
		return;
	}
	if (COLUMNS40)
		xbegin = XPOS, xend = xbegin + 640;
	else
		xbegin = XPOS + 7, xend = xbegin + 624;

	if (last < vic3->y_begin)
		end = last;
	else
		end = vic3->y_begin + YPOS;

	for (line = first; line < end; line++)
	{
		memset16(&vic3->bitmap->pix16(line + FIRSTLINE), FRAMECOLOR, vic3->bitmap->width());
	}

	if (LINES25)
	{
		vline = line - vic3->y_begin - YPOS;
	}
	else
	{
		vline = line - vic3->y_begin - YPOS + 8 - VERTICALPOS;
	}
	if (last < vic3->y_end + YPOS)
		end = last;
	else
		end = vic3->y_end + YPOS;
	x_end2 = vic3->x_end * 2;
	for (; line < end; vline = (vline + 8) & ~7, line = line + 1 + yend - ybegin)
	{
		offs = (vline >> 3) * 80;
		ybegin = vline & 7;
		yoff = line - ybegin;
		yend = (yoff + 7 < end) ? 7 : (end - yoff - 1);
		/* rendering 39 characters */
		/* left and right borders are overwritten later */
		vic3->shift[line] = HORIZONTALPOS;

		for (xoff = vic3->x_begin + XPOS; xoff < x_end2 + XPOS; xoff += 8, offs++)
		{
			ch = vic3->dma_read(device->machine(), vic3->videoaddr + offs);
			attr = vic3->dma_read_color(device->machine(), vic3->videoaddr + offs);
			if (HIRESON)
			{
				vic3->bitmapmulti[1] = vic3->c64_bitmap[1] = ch >> 4;
				vic3->bitmapmulti[2] = vic3->c64_bitmap[0] = ch & 0xf;
				if (MULTICOLORON)
				{
					vic3->bitmapmulti[3] = attr;
					vic3_draw_bitmap_multi(device, ybegin, yend, offs, yoff, xoff, start_x, end_x);
				}
				else
				{
					vic3_draw_bitmap(device, ybegin, yend, offs, yoff, xoff, start_x, end_x);
				}
			}
			else if (ECMON)
			{
				ecm = ch >> 6;
				vic3->ecmcolor[0] = vic3->colors[ecm];
				vic3->ecmcolor[1] = attr;
				vic3_draw_character(device, ybegin, yend, ch & ~0xC0, yoff, xoff, vic3->ecmcolor, start_x, end_x);
			}
			else if (MULTICOLORON && (attr & 8))
			{
				vic3->multi[3] = attr & 7;
				vic3_draw_character_multi(device, ybegin, yend, ch, yoff, xoff, start_x, end_x);
			}
			else
			{
				vic3->mono[1] = attr;
				vic3_draw_character(device, ybegin, yend, ch, yoff, xoff, vic3->mono, start_x, end_x);
			}
		}
		/* sprite priority, sprite overwrites lowerprior pixels */
		for (i = 7; i >= 0; i--)
		{
			if (vic3->sprites[i].line || vic3->sprites[i].repeat)
			{
				syend = yend;
				if (SPRITE_Y_EXPAND(i))
				{
					if ((21 - vic3->sprites[i].line) * 2 - vic3->sprites[i].repeat < yend - ybegin + 1)
						syend = ybegin + (21 - vic3->sprites[i].line) * 2 - vic3->sprites[i].repeat - 1;
				}
				else
				{
					if (vic3->sprites[i].line + yend - ybegin + 1 > 20)
						syend = ybegin + 20 - vic3->sprites[i].line;
				}
				if (yoff + syend > YPOS + 200)
					syend = YPOS + 200 - yoff - 1;
				if (SPRITE_MULTICOLOR(i))
					vic3_draw_sprite_multi(device, i, yoff, ybegin, syend, start_x, end_x);
				else
					vic3_draw_sprite(device, i, yoff, ybegin, syend, start_x, end_x);
				if ((syend != yend) || (vic3->sprites[i].line > 20))
				{
					vic3->sprites[i].line = vic3->sprites[i].repeat = 0;
					for (j = syend; j <= yend; j++)
						vic3->sprites[i].paintedline[j] = 0;
				}
			}
			// sprite wrap y at the top of the screen
			else if (SPRITEON(i) && (yoff == 1 + yend - ybegin) && (SPRITE_Y_POS(i) < 1 + yend - ybegin))
			{
				int wrapped = 1 + yend - ybegin - SPRITE_Y_POS(i);
				syend = yend;

				if (SPRITE_Y_EXPAND(i))
				{
					if (wrapped & 1) vic3->sprites[i].repeat = 1;
					wrapped >>= 1;
					syend = 21 * 2 - 1 - wrapped * 2;
					if (syend > (yend - ybegin)) syend = yend - ybegin;
				}
				else
				{
					syend = 21 - 1 - wrapped;
					if (syend > (yend - ybegin)) syend = yend - ybegin;
				}

				vic3->sprites[i].line = wrapped;

				if (SPRITE_MULTICOLOR(i))
					vic3_draw_sprite_multi(device, i, yoff, 0 , syend, start_x, end_x);
				else
					vic3_draw_sprite(device, i, yoff, 0 , syend, start_x, end_x);

				if ((syend != yend) || (vic3->sprites[i].line > 20))
				{
					for (j = syend; j <= yend; j++)
						vic3->sprites[i].paintedline[j] = 0;
					vic3->sprites[i].line = vic3->sprites[i].repeat = 0;
				}
			}
			else if (SPRITEON(i) && (yoff + ybegin <= SPRITE_Y_POS(i))
					 && (yoff + yend >= SPRITE_Y_POS(i)))
			{
				syend = yend;
				if (SPRITE_Y_EXPAND(i))
				{
					if (21 * 2 < yend - ybegin + 1)
						syend = ybegin + 21 * 2 - 1;
				}
				else
				{
					if (yend - ybegin + 1 > 21)
						syend = ybegin + 21 - 1;
				}
				if (yoff + syend >= YPOS + 200)
					syend = YPOS + 200 - yoff - 1;
				for (j = 0; j < SPRITE_Y_POS(i) - yoff; j++)
					vic3->sprites[i].paintedline[j] = 0;
				if (SPRITE_MULTICOLOR(i))
					vic3_draw_sprite_multi(device, i, yoff, SPRITE_Y_POS(i) - yoff, syend, start_x, end_x);
				else
					vic3_draw_sprite(device, i, yoff, SPRITE_Y_POS(i) - yoff, syend, start_x, end_x);
				if ((syend != yend) || (vic3->sprites[i].line > 20))
				{
					for (j = syend; j <= yend; j++)
						vic3->sprites[i].paintedline[j] = 0;
					vic3->sprites[i].line = vic3->sprites[i].repeat = 0;
				}
			}
			else
			{
				memset (vic3->sprites[i].paintedline, 0, sizeof (vic3->sprites[i].paintedline));
			}
		}

		for (i = ybegin; i <= yend; i++)
		{
			vic3->bitmap->plot_box(0, yoff + ybegin + FIRSTLINE, xbegin, yend-ybegin+1, FRAMECOLOR);
			vic3->bitmap->plot_box(xend, yoff + ybegin + FIRSTLINE, vic3->bitmap->width() - xend, yend-ybegin+1, FRAMECOLOR);
		}
	}
	if (last < vic3->bitmap->height())
		end = last;
	else
		end = vic3->bitmap->height();

	for (; line < end; line++)
	{
		memset16 (&vic3->bitmap->pix16(line + FIRSTLINE), FRAMECOLOR, vic3->bitmap->width());
	}
}

static void vic2_drawlines( device_t *device, int first, int last, int start_x, int end_x )
{
	vic3_state *vic3 = get_safe_token(device);
	int line, vline, end;
	int attr, ch, ecm;
	int syend;
	int offs, yoff, xoff, ybegin, yend, xbegin, xend;
	int i;

	if (VIC3_BITPLANES)
		return ;

	/* temporary allowing vic3 displaying 80 columns */
	if (vic3->reg[0x31] & 0x80)
	{
		vic3_drawlines(device, first, first + 1, start_x, end_x);
		return;
	}

	/* otherwise, draw VIC II output (currently using the old code, not the new one from vic6567.c) */

	/* top part of display not rastered */
	first -= VIC2_YPOS - YPOS;

	xbegin = VIC2_STARTVISIBLECOLUMNS;
	xend = xbegin + VIC2_VISIBLECOLUMNS;
	if (!SCREENON)
	{
		xbegin = VIC2_STARTVISIBLECOLUMNS;
		xend = xbegin + VIC2_VISIBLECOLUMNS;
		if ((start_x <= xbegin) && (end_x >= xend))
			vic3->bitmap->plot_box(xbegin, first + FIRSTLINE, xend - xbegin, 1, FRAMECOLOR);
		if ((start_x > xbegin) && (end_x >= xend))
			vic3->bitmap->plot_box(start_x - VIC2_STARTVISIBLECOLUMNS, first + FIRSTLINE, xend - start_x, 1, FRAMECOLOR);
		if ((start_x <= xbegin) && (end_x < xend))
			vic3->bitmap->plot_box(xbegin, first + FIRSTLINE, end_x - xbegin , 1, FRAMECOLOR);
		if ((start_x > xbegin) && (end_x < xend))
			vic3->bitmap->plot_box(start_x - VIC2_STARTVISIBLECOLUMNS, first + FIRSTLINE, end_x - start_x, 1, FRAMECOLOR);
		return;
	}

	if (COLUMNS40)
	{
		xbegin = XPOS;
		xend = xbegin + 320;
	}
	else
	{
		xbegin = XPOS + 7;
		xend = xbegin + 304;
	}

	if (first + 1 < vic3->y_begin)
		end = first + 1;
	else
		end = vic3->y_begin + YPOS;

	line = first;
	// top border
	if (line < end)
	{
		if ((start_x <= xbegin) && (end_x >= xend))
			vic3->bitmap->plot_box(xbegin, first + FIRSTLINE, xend - xbegin, 1, FRAMECOLOR);
		if ((start_x > xbegin) && (end_x >= xend))
			vic3->bitmap->plot_box(start_x - VIC2_STARTVISIBLECOLUMNS, first + FIRSTLINE, xend - start_x, 1, FRAMECOLOR);
		if ((start_x <= xbegin) && (end_x < xend))
			vic3->bitmap->plot_box(xbegin, first + FIRSTLINE, end_x - xbegin , 1, FRAMECOLOR);
		if ((start_x > xbegin) && (end_x < xend))
			vic3->bitmap->plot_box(start_x - VIC2_STARTVISIBLECOLUMNS, first + FIRSTLINE, end_x - start_x, 1, FRAMECOLOR);
		line = end;
	}

	vline = line - YPOS + 3 - VERTICALPOS;

	if (first + 1 < vic3->y_end + YPOS)
		end = first + 1;
	else
		end = vic3->y_end + YPOS;

	if (line < end)
	{
		offs = (vline >> 3) * 40;
		ybegin = vline & 7;
		yoff = line - ybegin;
		yend = (yoff + 7 < end) ? 7 : (end - yoff - 1);

		/* rendering 39 characters */
		/* left and right borders are overwritten later */

		vic3->shift[line] = HORIZONTALPOS;
		for (xoff = vic3->x_begin + XPOS; xoff < vic3->x_end + XPOS; xoff += 8, offs++)
		{
			ch = vic3->dma_read(device->machine(), vic3->videoaddr + offs);
#if 0
			attr = vic3->dma_read_color(device->machine(), vic3->videoaddr + offs);
#else
			/* temporaery until vic3 finished */
			attr = vic3->dma_read_color(device->machine(), (vic3->videoaddr + offs)&0x3ff)&0x0f;
#endif
			if (HIRESON)
			{
				vic3->bitmapmulti[1] = vic3->c64_bitmap[1] = ch >> 4;
				vic3->bitmapmulti[2] = vic3->c64_bitmap[0] = ch & 0xf;
				if (MULTICOLORON)
				{
					vic3->bitmapmulti[3] = attr;
					vic3_draw_bitmap_multi(device, ybegin, yend, offs, yoff, xoff, start_x, end_x);
				}
				else
				{
					vic3_draw_bitmap(device, ybegin, yend, offs, yoff, xoff, start_x, end_x);
				}
			}
			else if (ECMON)
			{
				ecm = ch >> 6;
				vic3->ecmcolor[0] = vic3->colors[ecm];
				vic3->ecmcolor[1] = attr;
				vic3_draw_character(device, ybegin, yend, ch & ~0xC0, yoff, xoff, vic3->ecmcolor, start_x, end_x);
			}
			else if (MULTICOLORON && (attr & 8))
			{
				vic3->multi[3] = attr & 7;
				vic3_draw_character_multi(device, ybegin, yend, ch, yoff, xoff, start_x, end_x);
			}
			else
			{
				vic3->mono[1] = attr;
				vic3_draw_character(device, ybegin, yend, ch, yoff, xoff, vic3->mono, start_x, end_x);
			}
		}

		/* sprite priority, sprite overwrites lowerprior pixels */
		for (i = 7; i >= 0; i--)
		{
			if (SPRITEON (i) &&
					(yoff + ybegin >= SPRITE_Y_POS (i)) &&
					(yoff + ybegin - SPRITE_Y_POS (i) < (SPRITE_Y_EXPAND (i)? 21 * 2 : 21 )) &&
					(SPRITE_Y_POS (i) < 0))
			{
				int wrapped = - SPRITE_Y_POS (i) + 6;

				syend = yend;

				if (SPRITE_Y_EXPAND (i))
				{
					if (wrapped & 1) vic3->sprites[i].repeat = 1;
					wrapped >>= 1;
					syend = 21 * 2 - 1 - wrapped * 2;
					if (syend > (yend - ybegin)) syend = yend - ybegin;
				}
				else
				{
					syend = 21 - 1 - wrapped;
					if (syend > (yend - ybegin)) syend = yend - ybegin;
				}

				vic3->sprites[i].line = wrapped;

				if (SPRITE_MULTICOLOR (i))
					vic3_draw_sprite_multi(device, i, 0, 0 , syend, start_x, end_x);
				else
					vic3_draw_sprite(device, i, 0, 0 , syend, start_x, end_x);
			}
			else if 	(SPRITEON (i) &&
					(yoff + ybegin >= SPRITE_Y_POS (i)) &&
					(yoff + ybegin - SPRITE_Y_POS (i) < (SPRITE_Y_EXPAND (i)? 21 * 2 : 21 )) &&
					(SPRITE_Y_POS (i) >= 0))
			{
				int wrapped = yoff + ybegin - SPRITE_Y_POS (i);

				syend = yend;

				if (SPRITE_Y_EXPAND (i))
				{
					if (wrapped & 1) vic3->sprites[i].repeat = 1;
					wrapped >>= 1;
					syend = 21 * 2 - 1 - wrapped * 2;
					if (syend > (yend - ybegin)) syend = yend - ybegin;
				}
				else
				{
					syend = 21 - 1 - wrapped;
					if (syend > (yend - ybegin)) syend = yend - ybegin;
				}

				vic3->sprites[i].line = wrapped;

				if (SPRITE_MULTICOLOR (i))
					vic3_draw_sprite_multi(device, i, yoff + ybegin, 0, 0, start_x, end_x);
				else
					vic3_draw_sprite(device, i, yoff + ybegin, 0, 0, start_x, end_x);
			}
			else
			{
				memset (vic3->sprites[i].paintedline, 0, sizeof (vic3->sprites[i].paintedline));
			}
		}
		line += 1 + yend - ybegin;
	}

	// left border
	if ((start_x <= VIC2_STARTVISIBLECOLUMNS) && (end_x >= xbegin))
		vic3->bitmap->plot_box(VIC2_STARTVISIBLECOLUMNS, first + FIRSTLINE, xbegin - VIC2_STARTVISIBLECOLUMNS, 1, FRAMECOLOR);
	else if ((start_x > VIC2_STARTVISIBLECOLUMNS) && (end_x >= xbegin))
		vic3->bitmap->plot_box(start_x, first + FIRSTLINE, xbegin - start_x, 1, FRAMECOLOR);
	else if ((start_x <= VIC2_STARTVISIBLECOLUMNS) && (end_x < xbegin))
		vic3->bitmap->plot_box(VIC2_STARTVISIBLECOLUMNS, first + FIRSTLINE, end_x, 1, FRAMECOLOR);
	else if ((start_x > VIC2_STARTVISIBLECOLUMNS) && (end_x < xbegin))
		vic3->bitmap->plot_box(start_x, first + FIRSTLINE, end_x - start_x, 1, FRAMECOLOR);

	// right border
	if ((start_x <= xend) && (end_x >= VIC2_STARTVISIBLECOLUMNS + VIC2_VISIBLECOLUMNS))
		vic3->bitmap->plot_box(xend, first + FIRSTLINE, VIC2_STARTVISIBLECOLUMNS + VIC2_VISIBLECOLUMNS - xend, 1, FRAMECOLOR);
	else if ((start_x > xend) && (end_x >= VIC2_STARTVISIBLECOLUMNS + VIC2_VISIBLECOLUMNS))
		vic3->bitmap->plot_box(start_x, first + FIRSTLINE, VIC2_STARTVISIBLECOLUMNS + VIC2_VISIBLECOLUMNS - start_x, 1, FRAMECOLOR);
	else if ((start_x <= xend) && (end_x < VIC2_STARTVISIBLECOLUMNS + VIC2_VISIBLECOLUMNS))
		vic3->bitmap->plot_box(xend, first + FIRSTLINE, end_x - xend, 1, FRAMECOLOR);
	else if ((start_x > VIC2_STARTVISIBLECOLUMNS) && (end_x < xbegin))
		vic3->bitmap->plot_box(start_x, first + FIRSTLINE, end_x - start_x, 1, FRAMECOLOR);

	if (first + 1 < vic3->bitmap->height())
		end = first + 1;
	else
		end = vic3->bitmap->height();

	// bottom border
	if (line < end)
	{
		if ((start_x <= xbegin) && (end_x >= xend))
			vic3->bitmap->plot_box(xbegin, first + FIRSTLINE, xend - xbegin, 1, FRAMECOLOR);
		if ((start_x > xbegin) && (end_x >= xend))
			vic3->bitmap->plot_box(start_x - VIC2_STARTVISIBLECOLUMNS, first + FIRSTLINE, xend - start_x, 1, FRAMECOLOR);
		if ((start_x <= xbegin) && (end_x < xend))
			vic3->bitmap->plot_box(xbegin, first + FIRSTLINE, end_x - xbegin , 1, FRAMECOLOR);
		if ((start_x > xbegin) && (end_x < xend))
			vic3->bitmap->plot_box(start_x - VIC2_STARTVISIBLECOLUMNS, first + FIRSTLINE, end_x - start_x, 1, FRAMECOLOR);
		line = end;
	}
}

/*****************************************************************************
    I/O HANDLERS
*****************************************************************************/

WRITE8_DEVICE_HANDLER( vic3_palette_w )
{
	vic3_state *vic3 = get_safe_token(device);

	if (offset < 0x100)
		vic3->palette_red[offset] = data;
	else if (offset < 0x200)
		vic3->palette_green[offset & 0xff] = data;
	else
		vic3->palette_blue[offset & 0xff] = data;

	vic3->palette_dirty = 1;
}


WRITE8_DEVICE_HANDLER( vic3_port_w )
{
	vic3_state *vic3 = get_safe_token(device);

	DBG_LOG(2, "vic write", ("%.2x:%.2x\n", offset, data));
	offset &= 0x7f;

	/* offsets 0x00 -> 0x2e coincide with VICII */
	switch (offset)
	{
	case 0x01:
	case 0x03:
	case 0x05:
	case 0x07:
	case 0x09:
	case 0x0b:
	case 0x0d:
	case 0x0f:
									/* sprite y positions */
		if (vic3->reg[offset] != data)
		{
			vic3->reg[offset] = data;
			vic3->sprites[offset / 2].y = SPRITE_Y_POS(offset / 2);
		}
		break;

	case 0x00:
	case 0x02:
	case 0x04:
	case 0x06:
	case 0x08:
	case 0x0a:
	case 0x0c:
	case 0x0e:
									/* sprite x positions */
		if (vic3->reg[offset] != data)
		{
			vic3->reg[offset] = data;
			vic3->sprites[offset / 2].x = SPRITE_X_POS(offset / 2);
		}
		break;

	case 0x10:							/* sprite x positions */
		if (vic3->reg[offset] != data)
		{
			vic3->reg[offset] = data;
			vic3->sprites[0].x = SPRITE_X_POS(0);
			vic3->sprites[1].x = SPRITE_X_POS(1);
			vic3->sprites[2].x = SPRITE_X_POS(2);
			vic3->sprites[3].x = SPRITE_X_POS(3);
			vic3->sprites[4].x = SPRITE_X_POS(4);
			vic3->sprites[5].x = SPRITE_X_POS(5);
			vic3->sprites[6].x = SPRITE_X_POS(6);
			vic3->sprites[7].x = SPRITE_X_POS(7);
		}
		break;

	case 0x17:							/* sprite y size */
		if (vic3->reg[offset] != data)
		{
			vic3->reg[offset] = data;
		}
		break;

	case 0x1d:							/* sprite x size */
		if (vic3->reg[offset] != data)
		{
			vic3->reg[offset] = data;
		}
		break;

	case 0x1b:							/* sprite background priority */
		if (vic3->reg[offset] != data)
		{
			vic3->reg[offset] = data;
		}
		break;

	case 0x1c:							/* sprite multicolor mode select */
		if (vic3->reg[offset] != data)
		{
			vic3->reg[offset] = data;
		}
		break;

	case 0x27:
	case 0x28:
	case 0x29:
	case 0x2a:
	case 0x2b:
	case 0x2c:
	case 0x2d:
	case 0x2e:
									/* sprite colors */
		if (vic3->reg[offset] != data)
		{
			vic3->reg[offset] = data;
		}
		break;

	case 0x25:							/* sprite multicolor */
		if (vic3->reg[offset] != data)
		{
			vic3->reg[offset] = data;
			vic3->spritemulti[1] = SPRITE_MULTICOLOR1;
		}
		break;

	case 0x26:							/* sprite multicolor */
		if (vic3->reg[offset] != data)
		{
			vic3->reg[offset] = data;
			vic3->spritemulti[3] = SPRITE_MULTICOLOR2;
		}
		break;

	case 0x19:
		vic3_clear_interrupt(device->machine(), data & 0x0f, vic3);
		break;

	case 0x1a:							/* irq mask */
		vic3->reg[offset] = data;
		vic3_set_interrupt(device->machine(), 0, vic3);	// beamrider needs this
		break;

	case 0x11:
		if (vic3->reg[offset] != data)
		{
			vic3->reg[offset] = data;
			if (LINES25)
			{
				vic3->y_begin = 0;
				vic3->y_end = vic3->y_begin + 200;
			}
			else
			{
				vic3->y_begin = 4;
				vic3->y_end = vic3->y_begin + 192;
			}
		}
		break;

	case 0x12:
		if (data != vic3->reg[offset])
		{
			vic3->reg[offset] = data;
		}
		break;

	case 0x16:
		if (vic3->reg[offset] != data)
		{
			vic3->reg[offset] = data;
			vic3->x_begin = HORIZONTALPOS;
			vic3->x_end = vic3->x_begin + 320;
		}
		break;

	case 0x18:
		if (vic3->reg[offset] != data)
		{
			vic3->reg[offset] = data;
			vic3->videoaddr = VIDEOADDR;
			vic3->chargenaddr = CHARGENADDR;
			vic3->bitmapaddr = BITMAPADDR;
		}
		break;

	case 0x21:							/* background color */
		if (vic3->reg[offset] != data)
		{
			vic3->reg[offset] = data;
			vic3->mono[0] = vic3->bitmapmulti[0] = vic3->multi[0] = vic3->colors[0] = BACKGROUNDCOLOR;
		}
		break;

	case 0x22:							/* background color 1 */
		if (vic3->reg[offset] != data)
		{
			vic3->reg[offset] = data;
			vic3->multi[1] = vic3->colors[1] = MULTICOLOR1;
		}
		break;

	case 0x23:							/* background color 2 */
		if (vic3->reg[offset] != data)
		{
			vic3->reg[offset] = data;
			vic3->multi[2] = vic3->colors[2] = MULTICOLOR2;
		}
		break;

	case 0x24:							/* background color 3 */
		if (vic3->reg[offset] != data)
		{
			vic3->reg[offset] = data;
			vic3->colors[3] = FOREGROUNDCOLOR;
		}
		break;

	case 0x20:							/* framecolor */
		if (vic3->reg[offset] != data)
		{
			vic3->reg[offset] = data;
		}
		break;

	case 0x2f:
		DBG_LOG(2, "vic write", ("%.2x:%.2x\n", offset, data));
		vic3->reg[offset] = data;
		break;
	case 0x30:
		vic3->reg[offset] = data;
		if (vic3->port_changed!=NULL) {
			DBG_LOG(2, "vic write", ("%.2x:%.2x\n", offset, data));
			vic3->reg[offset] = data;
			vic3->port_changed(device->machine(), data);
		}
		break;
	case 0x31:
		vic3->reg[offset] = data;
		if (data & 0x40)
			vic3->cpu->set_clock_scale(1.0);
		else
			vic3->cpu->set_clock_scale(1.0/3.5);
		break;
	case 0x32:
	case 0x33:
	case 0x34:
	case 0x35:
	case 0x36:
	case 0x37:
	case 0x38:
	case 0x39:
	case 0x3a:
	case 0x3b:
	case 0x3c:
	case 0x3d:
	case 0x3e:
	case 0x3f:
		vic3->reg[offset] = data;
		DBG_LOG(2, "vic write", ("%.2x:%.2x\n", offset, data));
		break;
	case 0x40:
	case 0x41:
	case 0x42:
	case 0x43:
	case 0x44:
	case 0x45:
	case 0x46:
	case 0x47:
		DBG_LOG(2, "vic plane write", ("%.2x:%.2x\n", offset, data));
		break;
	default:
		vic3->reg[offset] = data;
		break;
	}
}

READ8_DEVICE_HANDLER( vic3_port_r )
{
	vic3_state *vic3 = get_safe_token(device);
	int val = 0;
	offset &= 0x7f;

	/* offsets 0x00 -> 0x2e coincide with VICII */
	switch (offset)
	{
	case 0x11:
		val = (vic3->reg[offset] & ~0x80) | ((vic3->rasterline & 0x100) >> 1);
		break;

	case 0x12:
		val = vic3->rasterline & 0xff;
		break;

	case 0x16:
		val = vic3->reg[offset] | 0xc0;
		break;

	case 0x18:
		val = vic3->reg[offset] | 0x01;
		break;

	case 0x19:							/* interrupt flag register */
		/* vic2_clear_interrupt(0xf); */
		val = vic3->reg[offset] | 0x70;
		break;

	case 0x1a:
		val = vic3->reg[offset] | 0xf0;
		break;

	case 0x1e:							/* sprite to sprite collision detect */
		val = vic3->reg[offset];
		vic3->reg[offset] = 0;
		vic3_clear_interrupt(device->machine(), 4, vic3);
		break;

	case 0x1f:							/* sprite to background collision detect */
		val = vic3->reg[offset];
		vic3->reg[offset] = 0;
		vic3_clear_interrupt(device->machine(), 2, vic3);
		break;

	case 0x20:
	case 0x21:
	case 0x22:
	case 0x23:
	case 0x24:
		val = vic3->reg[offset];
		break;

	case 0x00:
	case 0x01:
	case 0x02:
	case 0x03:
	case 0x04:
	case 0x05:
	case 0x06:
	case 0x07:
	case 0x08:
	case 0x09:
	case 0x0a:
	case 0x0b:
	case 0x0c:
	case 0x0d:
	case 0x0e:
	case 0x0f:
	case 0x10:
	case 0x17:
	case 0x1b:
	case 0x1c:
	case 0x1d:
	case 0x25:
	case 0x26:
	case 0x27:
	case 0x28:
	case 0x29:
	case 0x2a:
	case 0x2b:
	case 0x2c:
	case 0x2d:
	case 0x2e:
		val = vic3->reg[offset];
		break;

	case 0x2f:
	case 0x30:
		val = vic3->reg[offset];
		DBG_LOG(2, "vic read", ("%.2x:%.2x\n", offset, val));
		break;
	case 0x31:
	case 0x32:
	case 0x33:
	case 0x34:
	case 0x35:
	case 0x36:
	case 0x37:
	case 0x38:
	case 0x39:
	case 0x3a:
	case 0x3b:
	case 0x3c:
	case 0x3d:
	case 0x3e:
	case 0x3f:						   /* not used */
		val = vic3->reg[offset];
		DBG_LOG(2, "vic read", ("%.2x:%.2x\n", offset, val));
		break;
	case 0x40:
	case 0x41:
	case 0x42:
	case 0x43:
	case 0x44:
	case 0x45:
	case 0x46:
	case 0x47:
		DBG_LOG(2, "vic3 plane read", ("%.2x:%.2x\n", offset, val));
		break;
	default:
		val = vic3->reg[offset];
	}
	return val;
}


#define VIC3_MASK(M)                                            \
    if (M)                                                      \
    {                                                           \
        if (M & 0x01)                                           \
            colors[0] = vic3->c64_mem_r(device->machine(), VIC3_ADDR(0) + offset);        \
        if (M & 0x02)                                           \
            colors[1] = vic3->c64_mem_r(device->machine(), VIC3_ADDR(1) + offset) << 1;     \
        if (M & 0x04)                                           \
            colors[2] = vic3->c64_mem_r(device->machine(), VIC3_ADDR(2) + offset) << 2;     \
        if (M & 0x08)                                           \
            colors[3] = vic3->c64_mem_r(device->machine(), VIC3_ADDR(3) + offset) << 3;     \
        if (M & 0x10)                                           \
            colors[4] = vic3->c64_mem_r(device->machine(), VIC3_ADDR(4) + offset) << 4;     \
        if (M & 0x20)                                           \
            colors[5] = vic3->c64_mem_r(device->machine(), VIC3_ADDR(5) + offset) << 5;     \
        if (M & 0x40)                                           \
            colors[6] = vic3->c64_mem_r(device->machine(), VIC3_ADDR(6) + offset) << 6;     \
        if (M & 0x80)                                           \
            colors[7] = vic3->c64_mem_r(device->machine(), VIC3_ADDR(7) + offset) << 7;     \
        for (i = 7; i >= 0; i--)                                \
        {                                                       \
            p = 0;                                              \
            if (M & 0x01)                                       \
            {                                                   \
                p = colors[0] & 0x01;                           \
                colors[0] >>= 1;                                \
            }                                                   \
            if (M & 0x02)                                       \
            {                                                   \
                p |= colors[1] & 0x02;                          \
                colors[1] >>= 1;                                \
            }                                                   \
            if (M & 0x04)                                       \
            {                                                   \
                p |= colors[2] & 0x04;                          \
                colors[2] >>= 1;                                \
            }                                                   \
            if (M & 0x08)                                       \
            {                                                   \
                p |= colors[3] & 0x08;                          \
                colors[3] >>= 1;                                \
            }                                                   \
            if (M & 0x10)                                       \
            {                                                   \
                p |= colors[4] & 0x10;                          \
                colors[4] >>= 1;                                \
            }                                                   \
            if (M & 0x20)                                       \
            {                                                   \
                p |= colors[5] & 0x20;                          \
                colors[5] >>= 1;                                \
            }                                                   \
            if (M & 0x40)                                       \
            {                                                   \
                p |= colors[6] & 0x40;                          \
                colors[6] >>= 1;                                \
            }                                                   \
            if (M & 0x80)                                       \
            {                                                   \
                p |= colors[7] & 0x80;                          \
                colors[7] >>= 1;                                \
            }                                                   \
            vic3->bitmap->pix16(YPOS + y, XPOS + x + i) = p; \
        }                                                       \
    }

#define VIC3_ADDR(a) VIC3_BITPLANE_IADDR(a)
static void vic3_interlace_draw_block( device_t *device, int x, int y, int offset )
{
	vic3_state *vic3 = get_safe_token(device);
	int colors[8] = {0};
	int i, p;

	switch (VIC3_BITPLANES_MASK)
	{
	case 0x05:
		VIC3_MASK(0x05)
		break;
	case 0x07:
		VIC3_MASK(0x07)
		break;
	case 0x0f:
		VIC3_MASK(0x0f)
		break;
	case 0x1f:
		VIC3_MASK(0x1f)
		break;
	case 0x7f:
		VIC3_MASK(0x7f)
		break;
	case 0xff:
		VIC3_MASK(0xff)
		break;
	default:
		if (VIC3_BITPLANES_MASK & 0x01)
			colors[0] = vic3->c64_mem_r(device->machine(), VIC3_BITPLANE_IADDR(0) + offset);

		if (VIC3_BITPLANES_MASK & 0x02)
			colors[1] = vic3->c64_mem_r(device->machine(), VIC3_BITPLANE_IADDR(1) + offset) << 1;

		if (VIC3_BITPLANES_MASK & 0x04)
			colors[2] = vic3->c64_mem_r(device->machine(), VIC3_BITPLANE_IADDR(2) + offset) << 2;

		if (VIC3_BITPLANES_MASK & 0x08)
			colors[3] = vic3->c64_mem_r(device->machine(), VIC3_BITPLANE_IADDR(3) + offset) << 3;

		if (VIC3_BITPLANES_MASK & 0x10)
			colors[4] = vic3->c64_mem_r(device->machine(), VIC3_BITPLANE_IADDR(4) + offset) << 4;

		if (VIC3_BITPLANES_MASK & 0x20)
			colors[5] = vic3->c64_mem_r(device->machine(), VIC3_BITPLANE_IADDR(5) + offset) << 5;

		if (VIC3_BITPLANES_MASK & 0x40)
			colors[6] = vic3->c64_mem_r(device->machine(), VIC3_BITPLANE_IADDR(6) + offset) << 6;

		if (VIC3_BITPLANES_MASK & 0x80)
			colors[7] = vic3->c64_mem_r(device->machine(), VIC3_BITPLANE_IADDR(7) + offset) << 7;

		for (i = 7; i >= 0; i--)
		{
			vic3->bitmap->pix16(YPOS + y, XPOS + x + i) =
				(colors[0] & 0x01) | (colors[1] & 0x02)
							 | (colors[2] & 0x04) | (colors[3] & 0x08)
							 | (colors[4] & 0x10) | (colors[5] & 0x20)
							 | (colors[6] & 0x40) | (colors[7] & 0x80);
			colors[0] >>= 1;
			colors[1] >>= 1;
			colors[2] >>= 1;
			colors[3] >>= 1;
			colors[4] >>= 1;
			colors[5] >>= 1;
			colors[6] >>= 1;
			colors[7] >>= 1;
		}
	}
}

#undef VIC3_ADDR
#define VIC3_ADDR(a) VIC3_BITPLANE_ADDR(a)
static void vic3_draw_block( device_t *device, int x, int y, int offset )
{
	vic3_state *vic3 = get_safe_token(device);
	int colors[8] = {0};
	int i, p;

	switch (VIC3_BITPLANES_MASK)
	{
	case 5:
		VIC3_MASK(0x05)
		break;
	case 7:
		VIC3_MASK(0x07)
		break;
	case 0xf:
		VIC3_MASK(0x0f)
		break;
	case 0x1f:
		VIC3_MASK(0x1f)
		break;
	case 0x7f:
		VIC3_MASK(0x7f)
		break;
	case 0xff:
		VIC3_MASK(0xff)
		break;
	default:
		if (VIC3_BITPLANES_MASK & 0x01)
			colors[0] = vic3->c64_mem_r(device->machine(), VIC3_BITPLANE_ADDR(0) + offset);

		if (VIC3_BITPLANES_MASK & 0x02)
			colors[1] = vic3->c64_mem_r(device->machine(), VIC3_BITPLANE_ADDR(1) + offset) << 1;

		if (VIC3_BITPLANES_MASK & 0x04)
			colors[2] = vic3->c64_mem_r(device->machine(), VIC3_BITPLANE_ADDR(2) + offset) << 2;

		if (VIC3_BITPLANES_MASK & 0x08)
			colors[3] = vic3->c64_mem_r(device->machine(), VIC3_BITPLANE_ADDR(3) + offset) << 3;

		if (VIC3_BITPLANES_MASK & 0x10)
			colors[4] = vic3->c64_mem_r(device->machine(), VIC3_BITPLANE_ADDR(4) + offset) << 4;

		if (VIC3_BITPLANES_MASK & 0x20)
			colors[5] = vic3->c64_mem_r(device->machine(), VIC3_BITPLANE_ADDR(5) + offset) << 5;

		if (VIC3_BITPLANES_MASK & 0x40)
			colors[6] = vic3->c64_mem_r(device->machine(), VIC3_BITPLANE_ADDR(6) + offset) << 6;

		if (VIC3_BITPLANES_MASK & 0x80)
			colors[7] = vic3->c64_mem_r(device->machine(), VIC3_BITPLANE_ADDR(7) + offset) << 7;

		for (i = 7; i >= 0; i--)
		{
			vic3->bitmap->pix16(YPOS + y, XPOS + x + i) =
				(colors[0] & 0x01) | (colors[1] & 0x02)
							 | (colors[2] & 0x04) | (colors[3] & 0x08)
							 | (colors[4] & 0x10) | (colors[5] & 0x20)
							 | (colors[6] & 0x40) | (colors[7] & 0x80);
			colors[0] >>= 1;
			colors[1] >>= 1;
			colors[2] >>= 1;
			colors[3] >>= 1;
			colors[4] >>= 1;
			colors[5] >>= 1;
			colors[6] >>= 1;
			colors[7] >>= 1;
		}
	}
}


static void vic3_draw_bitplanes( device_t *device )
{
	vic3_state *vic3 = get_safe_token(device);
	int x, y, y1s, offset;
	rectangle vis;
	const rectangle &visarea = vic3->main_screen->visible_area();

	if (VIC3_LINES == 400)
	{ /* interlaced! */
		for (y1s = 0, offset = 0; y1s < 400; y1s += 16)
		{
			for (x = 0; x < VIC3_BITPLANES_WIDTH; x += 8)
			{
				for (y = y1s; y < y1s + 16; y += 2, offset++)
				{
					if (vic3->interlace)
						vic3_draw_block(device, x, y, offset);
					else
						vic3_interlace_draw_block(device, x, y + 1, offset);
				}
			}
		}
		vic3->interlace ^= 1;
	}
	else
	{
		for (y1s = 0, offset = 0; y1s < 200; y1s += 8)
		{
			for (x = 0; x < VIC3_BITPLANES_WIDTH; x += 8)
			{
				for (y = y1s; y < y1s + 8; y++, offset++)
				{
					vic3_draw_block(device, x, y, offset);
				}
			}
		}
	}

	if (XPOS > 0)
	{
		vis.set(0, XPOS - 1, 0, visarea.max_y);
		vic3->bitmap->fill(FRAMECOLOR, vis);
	}

	if (XPOS + VIC3_BITPLANES_WIDTH < visarea.max_x)
	{
		vis.set(XPOS + VIC3_BITPLANES_WIDTH, visarea.max_x, 0, visarea.max_y);
		vic3->bitmap->fill(FRAMECOLOR, vis);
	}

	if (YPOS > 0)
	{
		vis.set(0, visarea.max_x, 0, YPOS - 1);
		vic3->bitmap->fill(FRAMECOLOR, vis);
	}

	if (YPOS + VIC3_LINES < visarea.max_y)
	{
		vis.set(0, visarea.max_x, YPOS + VIC3_LINES, visarea.max_y);
		vic3->bitmap->fill(FRAMECOLOR, vis);
	}
}

void vic3_raster_interrupt_gen( device_t *device )
{
	vic3_state *vic3 = get_safe_token(device);
	running_machine &machine = device->machine();
	int new_columns, new_rows;
	int i;

	vic3->rasterline++;
	if (vic3->rasterline >= vic3->lines)
	{
		vic3->rasterline = 0;
		if (vic3->palette_dirty)
			for (i = 0; i < 256; i++)
				palette_set_color_rgb(machine, i, vic3->palette_red[i] << 4, vic3->palette_green[i] << 4, vic3->palette_blue[i] << 4);

		if (vic3->palette_dirty)
		{
			vic3->spritemulti[1] = SPRITE_MULTICOLOR1;
			vic3->spritemulti[3] = SPRITE_MULTICOLOR2;
			vic3->mono[0] = vic3->bitmapmulti[0] = vic3->multi[0] = vic3->colors[0] = BACKGROUNDCOLOR;
			vic3->multi[1] = vic3->colors[1] = MULTICOLOR1;
			vic3->multi[2] = vic3->colors[2] = MULTICOLOR2;
			vic3->colors[3] = FOREGROUNDCOLOR;
			vic3->palette_dirty = 0;
		}

		new_rows = 200;

		if (VIC3_BITPLANES)
		{
			new_columns = VIC3_BITPLANES_WIDTH;
			if (new_columns < 320)
				new_columns = 320; /*sprites resolution about 320x200 */
			new_rows = VIC3_LINES;
		}
		else if (VIC3_80COLUMNS)
		{
			new_columns = 640;
		}
		else
		{
			new_columns = 320;
		}
		if ((new_columns != vic3->columns) || (new_rows != vic3->rows))
		{
			vic3->rows = new_rows;
			vic3->columns = new_columns;
			if (vic3->type == VIC4567_PAL)
				vic3->main_screen->set_visible_area(
									VIC2_STARTVISIBLECOLUMNS + 32,
									VIC2_STARTVISIBLECOLUMNS + 32 + vic3->columns + 16 - 1,
									VIC2_STARTVISIBLELINES + 34,
									VIC2_STARTVISIBLELINES + 34 + vic3->rows + 16 - 1);
			else
				vic3->main_screen->set_visible_area(
									VIC2_STARTVISIBLECOLUMNS + 34,
									VIC2_STARTVISIBLECOLUMNS + 34 + vic3->columns + 16 - 1,
									VIC2_STARTVISIBLELINES + 10,
									VIC2_STARTVISIBLELINES + 10 + vic3->rows + 16 - 1);
		}
		if (VIC3_BITPLANES)
		{
			vic3_draw_bitplanes(device);
		}
		else
		{
			if (vic3->type == VIC4567_PAL)
			{
				if (vic3->on)
					vic2_drawlines(device, vic3->lastline, vic3->lines, VIC2_STARTVISIBLECOLUMNS + 32, VIC2_STARTVISIBLECOLUMNS + 32 + vic3->columns + 16 - 1);
			}
			else
			{
				if (vic3->on)
					vic2_drawlines(device, vic3->lastline, vic3->lines, VIC2_STARTVISIBLECOLUMNS + 34, VIC2_STARTVISIBLECOLUMNS + 34 + vic3->columns + 16 - 1);
			}
		}

		for (i = 0; i < 8; i++)
			vic3->sprites[i].repeat = vic3->sprites[i].line = 0;

		vic3->lastline = 0;

		if (LIGHTPEN_BUTTON)
		{
			/* lightpen timer start */
			machine.scheduler().timer_set(attotime(0, 0), FUNC(vic3_timer_timeout), 1, vic3);
		}

	}

	if (vic3->rasterline == C64_2_RASTERLINE(RASTERLINE))
	{
		vic3_set_interrupt(machine, 1, vic3);
	}

	if (vic3->on)
		if ((vic3->rasterline >= VIC2_FIRSTRASTERLINE) && (vic3->rasterline < (VIC2_FIRSTRASTERLINE + VIC2_VISIBLELINES)))
		{
			if (vic3->type == VIC4567_PAL)
			{
				if (vic3->on)
					vic2_drawlines(device, vic3->rasterline - 1, vic3->rasterline, VIC2_STARTVISIBLECOLUMNS + 32, VIC2_STARTVISIBLECOLUMNS + 32 + vic3->columns + 16 - 1);
			}
			else
			{
				if (vic3->on)
					vic2_drawlines(device, vic3->rasterline - 1, vic3->rasterline, VIC2_STARTVISIBLECOLUMNS + 34, VIC2_STARTVISIBLECOLUMNS + 34 + vic3->columns + 16 - 1);
			}
		}
}

UINT32 vic3_video_update( device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	vic3_state *vic3 = get_safe_token(device);

	copybitmap(bitmap, *vic3->bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}

/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

static DEVICE_START( vic3 )
{
	vic3_state *vic3 = get_safe_token(device);
	const vic3_interface *intf = (vic3_interface *)device->static_config();
	int width, height;
	int i;

	vic3->cpu = device->machine().device(intf->cpu);

	vic3->main_screen = device->machine().device<screen_device>(intf->screen);
	width = vic3->main_screen->width();
	height = vic3->main_screen->height();

	vic3->bitmap = auto_bitmap_ind16_alloc(device->machine(), width, height);

	vic3->type = intf->type;

	vic3->dma_read = intf->dma_read;
	vic3->dma_read_color = intf->dma_read_color;
	vic3->interrupt = intf->irq;

	vic3->port_changed = intf->port_changed;

	vic3->c64_mem_r = intf->c64_mem_r;

	vic3->lightpen_button_cb = intf->button_cb;
	vic3->lightpen_x_cb = intf->x_cb;
	vic3->lightpen_y_cb = intf->y_cb;

	vic3->screen[0] = auto_alloc_array(device->machine(), UINT8, 216 * 656 / 8);

	for (i = 1; i < 216; i++)
		vic3->screen[i] = vic3->screen[i - 1] + 656 / 8;

	for (i = 0; i < 256; i++)
	{
		vic3->foreground[i] = 0;
		if ((i & 3) > 1)
			vic3->foreground[i] |= 0x3;
		if ((i & 0xc) > 0x4)
			vic3->foreground[i] |= 0xc;
		if ((i & 0x30) > 0x10)
			vic3->foreground[i] |= 0x30;
		if ((i & 0xc0) > 0x40)
			vic3->foreground[i] |= 0xc0;
	}

	for (i = 0; i < 256; i++)
	{
		vic3->expandx[i] = 0;
		if (i & 1)
			vic3->expandx[i] |= 3;
		if (i & 2)
			vic3->expandx[i] |= 0xc;
		if (i & 4)
			vic3->expandx[i] |= 0x30;
		if (i & 8)
			vic3->expandx[i] |= 0xc0;
		if (i & 0x10)
			vic3->expandx[i] |= 0x300;
		if (i & 0x20)
			vic3->expandx[i] |= 0xc00;
		if (i & 0x40)
			vic3->expandx[i] |= 0x3000;
		if (i & 0x80)
			vic3->expandx[i] |= 0xc000;
	}

	for (i = 0; i < 256; i++)
	{
		vic3->expandx_multi[i] = 0;
		if (i & 1)
			vic3->expandx_multi[i] |= 5;
		if (i & 2)
			vic3->expandx_multi[i] |= 0xa;
		if (i & 4)
			vic3->expandx_multi[i] |= 0x50;
		if (i & 8)
			vic3->expandx_multi[i] |= 0xa0;
		if (i & 0x10)
			vic3->expandx_multi[i] |= 0x500;
		if (i & 0x20)
			vic3->expandx_multi[i] |= 0xa00;
		if (i & 0x40)
			vic3->expandx_multi[i] |= 0x5000;
		if (i & 0x80)
			vic3->expandx_multi[i] |= 0xa000;
	}

	device->save_item(NAME(vic3->reg));

	device->save_item(NAME(vic3->on));

	//device->save_item(NAME(vic3->bitmap));

	device->save_item(NAME(vic3->lines));

	device->save_item(NAME(vic3->chargenaddr));
	device->save_item(NAME(vic3->videoaddr));
	device->save_item(NAME(vic3->bitmapaddr));

	device->save_item(NAME(vic3->x_begin));
	device->save_item(NAME(vic3->x_end));
	device->save_item(NAME(vic3->y_begin));
	device->save_item(NAME(vic3->y_end));

	device->save_item(NAME(vic3->c64_bitmap));
	device->save_item(NAME(vic3->bitmapmulti));
	device->save_item(NAME(vic3->mono));
	device->save_item(NAME(vic3->multi));
	device->save_item(NAME(vic3->ecmcolor));
	device->save_item(NAME(vic3->colors));
	device->save_item(NAME(vic3->spritemulti));

	device->save_item(NAME(vic3->lastline));
	device->save_item(NAME(vic3->rasterline));
	device->save_item(NAME(vic3->interlace));

	device->save_item(NAME(vic3->columns));
	device->save_item(NAME(vic3->rows));

	device->save_item(NAME(vic3->shift));
	device->save_item(NAME(vic3->foreground));
	device->save_item(NAME(vic3->multi_collision));

	device->save_item(NAME(vic3->palette_red));
	device->save_item(NAME(vic3->palette_green));
	device->save_item(NAME(vic3->palette_blue));
	device->save_item(NAME(vic3->palette_dirty));

	for (i = 0; i < 8; i++)
	{
		device->save_item(NAME(vic3->sprites[i].x), i);
		device->save_item(NAME(vic3->sprites[i].y), i);
		device->save_item(NAME(vic3->sprites[i].repeat), i);
		device->save_item(NAME(vic3->sprites[i].line), i);
		device->save_item(NAME(vic3->sprites[i].paintedline), i);
		device->save_item(NAME(vic3->sprites[i].bitmap[0]), i);
		device->save_item(NAME(vic3->sprites[i].bitmap[1]), i);
		device->save_item(NAME(vic3->sprites[i].bitmap[2]), i);
		device->save_item(NAME(vic3->sprites[i].bitmap[3]), i);
		device->save_item(NAME(vic3->sprites[i].bitmap[4]), i);
		device->save_item(NAME(vic3->sprites[i].bitmap[5]), i);
		device->save_item(NAME(vic3->sprites[i].bitmap[6]), i);
		device->save_item(NAME(vic3->sprites[i].bitmap[7]), i);
	}
}

static DEVICE_RESET( vic3 )
{
	vic3_state *vic3 = get_safe_token(device);
	int i;

	memset(vic3->reg, 0, ARRAY_LENGTH(vic3->reg));

	vic3->on = 1;

	vic3->interlace = 0;
	vic3->columns = 640;
	vic3->rows = 200;
	vic3->lines = VIC2_LINES;

	memset(&vic3->sprites, 0, sizeof(vic3->sprites));

	vic3->chargenaddr = 0;
	vic3->videoaddr = 0;
	vic3->bitmapaddr = 0;

	vic3->x_begin = 0;
	vic3->x_end = 0;
	vic3->y_begin = 0;
	vic3->y_end = 0;

	for (i = 0; i < 2; i++)
	{
		vic3->c64_bitmap[i] = 0;
		vic3->mono[i] = 0;
		vic3->ecmcolor[i] = 0;
	}

	for (i = 0; i < 4; i++)
	{
		vic3->bitmapmulti[i] = 0;
		vic3->multi[i] = 0;
		vic3->colors[i] = 0;
		vic3->spritemulti[i] = 0;
	}

	vic3->lastline = 0;
	vic3->rasterline = 0;

	memset(vic3->shift, 0, ARRAY_LENGTH(vic3->shift));
	memset(vic3->multi_collision, 0, ARRAY_LENGTH(vic3->multi_collision));
	memset(vic3->palette_red, 0, ARRAY_LENGTH(vic3->palette_red));
	memset(vic3->palette_green, 0, ARRAY_LENGTH(vic3->palette_green));
	memset(vic3->palette_blue, 0, ARRAY_LENGTH(vic3->palette_blue));

	vic3->palette_dirty = 0;
}


/*-------------------------------------------------
    device definition
-------------------------------------------------*/

DEVICE_GET_INFO(vic3)
{
 switch (state)
 {
  case DEVINFO_INT_TOKEN_BYTES: info->i = sizeof(vic3_state); break;

  case DEVINFO_FCT_START: info->start = DEVICE_START_NAME(vic3); break;

  case DEVINFO_FCT_RESET: info->reset = DEVICE_RESET_NAME(vic3); break;

  case DEVINFO_STR_NAME: strcpy(info->s, "4567 VIC III"); break;
 }
}

DEFINE_LEGACY_DEVICE(VIC3, vic3);
