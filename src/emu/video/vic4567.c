// license:BSD-3-Clause
// copyright-holders:Peter Trauner
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

/*****************************************************************************
    CONSTANTS
*****************************************************************************/

#define VERBOSE_LEVEL 0
#define DBG_LOG(N,M,A) \
	do { \
		if(VERBOSE_LEVEL >= N) \
		{ \
			if( M ) \
				logerror("%11.6f: %-24s", machine().time().as_double(), (char*) M ); \
			logerror A; \
		} \
	} while (0)

#define VREFRESHINLINES         28

#define VIC2_YPOS               50
#define RASTERLINE_2_C64(a)     (a)
#define C64_2_RASTERLINE(a)     (a)
#define XPOS                (VIC2_STARTVISIBLECOLUMNS + (VIC2_VISIBLECOLUMNS - VIC2_HSIZE) / 2)
#define YPOS                (VIC2_STARTVISIBLELINES /* + (VIC2_VISIBLELINES - VIC2_VSIZE) / 2 */)
#define FIRSTLINE               10 /* 36 ((VIC2_VISIBLELINES - VIC2_VSIZE)/2) */
#define FIRSTCOLUMN         50

/* 2008-05 FP: lightpen code needs to read input port from c64.c and cbmb.c */

#define LIGHTPEN_BUTTON     (m_lightpen_button_cb(0))
#define LIGHTPEN_X_VALUE    (m_lightpen_x_cb(0))
#define LIGHTPEN_Y_VALUE    (m_lightpen_y_cb(0))

/* lightpen delivers values from internal counters; they do not start with the visual area or frame area */
#define VIC2_MAME_XPOS          0
#define VIC2_MAME_YPOS          0
#define VIC6567_X_BEGIN         38
#define VIC6567_Y_BEGIN         -6             /* first 6 lines after retrace not for lightpen! */
#define VIC6569_X_BEGIN         38
#define VIC6569_Y_BEGIN         -6
#define VIC2_X_BEGIN            ((m_type == VIC4567_PAL) ? VIC6569_X_BEGIN : VIC6567_X_BEGIN)
#define VIC2_Y_BEGIN            ((m_type == VIC4567_PAL) ? VIC6569_Y_BEGIN : VIC6567_Y_BEGIN)
#define VIC2_X_VALUE            ((LIGHTPEN_X_VALUE + VIC2_X_BEGIN + VIC2_MAME_XPOS) / 2)
#define VIC2_Y_VALUE            ((LIGHTPEN_Y_VALUE + VIC2_Y_BEGIN + VIC2_MAME_YPOS))

#define VIC2E_K0_LEVEL          (m_reg[0x2f] & 0x01)
#define VIC2E_K1_LEVEL          (m_reg[0x2f] & 0x02)
#define VIC2E_K2_LEVEL          (m_reg[0x2f] & 0x04)

/*#define VIC3_P5_LEVEL (m_reg[0x30] & 0x20) */
#define VIC3_BITPLANES          (m_reg[0x31] & 0x10)
#define VIC3_80COLUMNS          (m_reg[0x31] & 0x80)
#define VIC3_LINES              ((m_reg[0x31] & 0x19) == 0x19 ? 400 : 200)
#define VIC3_BITPLANES_WIDTH    (m_reg[0x31] & 0x80 ? 640 : 320)

/*#define VIC2E_TEST (vic2[0x30] & 2) */
#define DOUBLE_CLOCK            (m_reg[0x30] & 0x01)

/* sprites 0 .. 7 */
#define SPRITEON(nr)            (m_reg[0x15] & (1 << nr))
#define SPRITE_Y_EXPAND(nr)     (m_reg[0x17] & (1 << nr))
#define SPRITE_Y_SIZE(nr)       (SPRITE_Y_EXPAND(nr) ? 2 * 21 : 21)
#define SPRITE_X_EXPAND(nr)     (m_reg[0x1d] & (1 << nr))
#define SPRITE_X_SIZE(nr)       (SPRITE_X_EXPAND(nr) ? 2 * 24 : 24)
#define SPRITE_X_POS(nr)        ((m_reg[(nr) * 2] | (m_reg[0x10] & (1 <<(nr)) ? 0x100 : 0)) - 24 + XPOS)
#define SPRITE_X_POS2(nr)       (m_reg[(nr) * 2] | (m_reg[0x10] & (1 <<(nr)) ? 0x100 : 0))
#define SPRITE_Y_POS(nr)        (m_reg[1+2*(nr)] - 50 + YPOS)
#define SPRITE_Y_POS2(nr)       (m_reg[1 + 2 *(nr)])
#define SPRITE_MULTICOLOR(nr)   (m_reg[0x1c] & (1 << nr))
#define SPRITE_PRIORITY(nr)     (m_reg[0x1b] & (1 << nr))
#define SPRITE_MULTICOLOR1      (m_reg[0x25] & 0x0f)
#define SPRITE_MULTICOLOR2      (m_reg[0x26] & 0x0f)
#define SPRITE_COLOR(nr)        (m_reg[0x27+nr] & 0x0f)
#define SPRITE_ADDR(nr)         (m_videoaddr | 0x3f8 | nr)
#define SPRITE_BG_COLLISION(nr) (m_reg[0x1f] & (1 << nr))
#define SPRITE_COLLISION(nr)        (m_reg[0x1e] & (1 << nr))
#define SPRITE_SET_BG_COLLISION(nr) (m_reg[0x1f] |= (1 << nr))
#define SPRITE_SET_COLLISION(nr)    (m_reg[0x1e] |= (1 << nr))
#define SPRITE_COLL         (m_reg[0x1e])
#define SPRITE_BG_COLL          (m_reg[0x1f])

#define GFXMODE             ((m_reg[0x11] & 0x60) | (m_reg[0x16] & 0x10)) >> 4
#define SCREENON                (m_reg[0x11] & 0x10)
#define VERTICALPOS         (m_reg[0x11] & 0x07)
#define HORIZONTALPOS           (m_reg[0x16] & 0x07)
#define ECMON               (m_reg[0x11] & 0x40)
#define HIRESON             (m_reg[0x11] & 0x20)
#define MULTICOLORON            (m_reg[0x16] & 0x10)
#define LINES25             (m_reg[0x11] & 0x08)           /* else 24 Lines */
#define LINES               (LINES25 ? 25 : 24)
#define YSIZE               (LINES * 8)
#define COLUMNS40               (m_reg[0x16] & 0x08)           /* else 38 Columns */
#define COLUMNS             (COLUMNS40 ? 40 : 38)
#define XSIZE               (COLUMNS * 8)

#define VIDEOADDR               ((m_reg[0x18] & 0xf0) << (10 - 4))
#define CHARGENADDR         ((m_reg[0x18] & 0x0e) << 10)
#define BITMAPADDR          ((data & 0x08) << 10)

#define RASTERLINE          (((m_reg[0x11] & 0x80) << 1) | m_reg[0x12])

#define FRAMECOLOR          (m_reg[0x20] & 0x0f)
#define BACKGROUNDCOLOR         (m_reg[0x21] & 0x0f)
#define MULTICOLOR1         (m_reg[0x22] & 0x0f)
#define MULTICOLOR2         (m_reg[0x23] & 0x0f)
#define FOREGROUNDCOLOR         (m_reg[0x24] & 0x0f)


#define VIC2_LINES      (m_type == VIC4567_PAL ? VIC6569_LINES : VIC6567_LINES)
#define VIC2_VISIBLELINES   (m_type == VIC4567_PAL ? VIC6569_VISIBLELINES : VIC6567_VISIBLELINES)
#define VIC2_VISIBLECOLUMNS (m_type == VIC4567_PAL ? VIC6569_VISIBLECOLUMNS : VIC6567_VISIBLECOLUMNS)
#define VIC2_STARTVISIBLELINES ((VIC2_LINES - VIC2_VISIBLELINES)/2)
#define VIC2_FIRSTRASTERLINE  (m_type == VIC4567_PAL ? VIC6569_FIRSTRASTERLINE : VIC6567_FIRSTRASTERLINE)
#define VIC2_COLUMNS          (m_type == VIC4567_PAL ? VIC6569_COLUMNS : VIC6567_COLUMNS)
#define VIC2_STARTVISIBLECOLUMNS ((VIC2_COLUMNS - VIC2_VISIBLECOLUMNS)/2)

#define VIC3_BITPLANES_MASK (m_reg[0x32])
/* bit 0, 4 not used !?*/
/* I think hinibbles contains the banknumbers for interlaced modes */
/* if hinibble set then x&1==0 should be in bank1 (0x10000), x&1==1 in bank 0 */
#define VIC3_BITPLANE_ADDR_HELPER(x)  ((m_reg[0x33 + x] & 0x0f) << 12)
#define VIC3_BITPLANE_ADDR(x) (x & 1 ? VIC3_BITPLANE_ADDR_HELPER(x) + 0x10000 : VIC3_BITPLANE_ADDR_HELPER(x) )
#define VIC3_BITPLANE_IADDR_HELPER(x)  ((m_reg[0x33 + x] & 0xf0) << 8)
#define VIC3_BITPLANE_IADDR(x) (x & 1 ? VIC3_BITPLANE_IADDR_HELPER(x) + 0x10000 : VIC3_BITPLANE_IADDR_HELPER(x))


const device_type VIC3 = &device_creator<vic3_device>;

vic3_device::vic3_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
			: device_t(mconfig, VIC3, "4567 VIC III", tag, owner, clock, "vic3", __FILE__),
				device_video_interface(mconfig, *this),
				m_type(VIC4567_NTSC),
				m_cpu(*this),
				m_dma_read_cb(*this),
				m_dma_read_color_cb(*this),
				m_interrupt_cb(*this),
				m_port_changed_cb(*this),
				m_lightpen_button_cb(*this),
				m_lightpen_x_cb(*this),
				m_lightpen_y_cb(*this),
				m_c64_mem_r_cb(*this),
				m_palette(*this, "palette")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vic3_device::device_start()
{
	int width, height;

	width = m_screen->width();
	height = m_screen->height();

	m_bitmap = auto_bitmap_ind16_alloc(machine(), width, height);

	m_dma_read_cb.resolve_safe(0);
	m_dma_read_color_cb.resolve_safe(0);
	m_interrupt_cb.resolve_safe();

	m_port_changed_cb.resolve();

	m_c64_mem_r_cb.resolve_safe(0);

	m_lightpen_button_cb.resolve_safe(0);
	m_lightpen_x_cb.resolve_safe(0);
	m_lightpen_y_cb.resolve_safe(0);

	m_screenptr[0] = auto_alloc_array(machine(), UINT8, 216 * 656 / 8);

	for (int i = 1; i < 216; i++)
		m_screenptr[i] = m_screenptr[i - 1] + 656 / 8;

	for (int i = 0; i < 256; i++)
	{
		m_foreground[i] = 0;
		if ((i & 3) > 1)
			m_foreground[i] |= 0x3;
		if ((i & 0xc) > 0x4)
			m_foreground[i] |= 0xc;
		if ((i & 0x30) > 0x10)
			m_foreground[i] |= 0x30;
		if ((i & 0xc0) > 0x40)
			m_foreground[i] |= 0xc0;
	}

	for (int i = 0; i < 256; i++)
	{
		m_expandx[i] = 0;
		if (i & 1)
			m_expandx[i] |= 3;
		if (i & 2)
			m_expandx[i] |= 0xc;
		if (i & 4)
			m_expandx[i] |= 0x30;
		if (i & 8)
			m_expandx[i] |= 0xc0;
		if (i & 0x10)
			m_expandx[i] |= 0x300;
		if (i & 0x20)
			m_expandx[i] |= 0xc00;
		if (i & 0x40)
			m_expandx[i] |= 0x3000;
		if (i & 0x80)
			m_expandx[i] |= 0xc000;
	}

	for (int i = 0; i < 256; i++)
	{
		m_expandx_multi[i] = 0;
		if (i & 1)
			m_expandx_multi[i] |= 5;
		if (i & 2)
			m_expandx_multi[i] |= 0xa;
		if (i & 4)
			m_expandx_multi[i] |= 0x50;
		if (i & 8)
			m_expandx_multi[i] |= 0xa0;
		if (i & 0x10)
			m_expandx_multi[i] |= 0x500;
		if (i & 0x20)
			m_expandx_multi[i] |= 0xa00;
		if (i & 0x40)
			m_expandx_multi[i] |= 0x5000;
		if (i & 0x80)
			m_expandx_multi[i] |= 0xa000;
	}

	save_item(NAME(m_reg));

	save_item(NAME(m_on));

	//save_item(NAME(m_bitmap));

	save_item(NAME(m_lines));

	save_item(NAME(m_chargenaddr));
	save_item(NAME(m_videoaddr));
	save_item(NAME(m_bitmapaddr));

	save_item(NAME(m_x_begin));
	save_item(NAME(m_x_end));
	save_item(NAME(m_y_begin));
	save_item(NAME(m_y_end));

	save_item(NAME(m_c64_bitmap));
	save_item(NAME(m_bitmapmulti));
	save_item(NAME(m_mono));
	save_item(NAME(m_multi));
	save_item(NAME(m_ecmcolor));
	save_item(NAME(m_colors));
	save_item(NAME(m_spritemulti));

	save_item(NAME(m_lastline));
	save_item(NAME(m_rasterline));
	save_item(NAME(m_interlace));

	save_item(NAME(m_columns));
	save_item(NAME(m_rows));

	save_item(NAME(m_shift));
	save_item(NAME(m_foreground));
	save_item(NAME(m_multi_collision));

	save_item(NAME(m_palette_red));
	save_item(NAME(m_palette_green));
	save_item(NAME(m_palette_blue));
	save_item(NAME(m_palette_dirty));

	for (int i = 0; i < 8; i++)
	{
		save_item(NAME(m_sprites[i].x), i);
		save_item(NAME(m_sprites[i].y), i);
		save_item(NAME(m_sprites[i].repeat), i);
		save_item(NAME(m_sprites[i].line), i);
		save_item(NAME(m_sprites[i].paintedline), i);
		save_item(NAME(m_sprites[i].bitmap[0]), i);
		save_item(NAME(m_sprites[i].bitmap[1]), i);
		save_item(NAME(m_sprites[i].bitmap[2]), i);
		save_item(NAME(m_sprites[i].bitmap[3]), i);
		save_item(NAME(m_sprites[i].bitmap[4]), i);
		save_item(NAME(m_sprites[i].bitmap[5]), i);
		save_item(NAME(m_sprites[i].bitmap[6]), i);
		save_item(NAME(m_sprites[i].bitmap[7]), i);
	}
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void vic3_device::device_reset()
{
	memset(m_reg, 0, ARRAY_LENGTH(m_reg));

	m_on = 1;

	m_interlace = 0;
	m_columns = 640;
	m_rows = 200;
	m_lines = VIC2_LINES;

	memset(&m_sprites, 0, sizeof(m_sprites));

	m_chargenaddr = 0;
	m_videoaddr = 0;
	m_bitmapaddr = 0;

	m_x_begin = 0;
	m_x_end = 0;
	m_y_begin = 0;
	m_y_end = 0;

	for (int i = 0; i < 2; i++)
	{
		m_c64_bitmap[i] = 0;
		m_mono[i] = 0;
		m_ecmcolor[i] = 0;
	}

	for (int i = 0; i < 4; i++)
	{
		m_bitmapmulti[i] = 0;
		m_multi[i] = 0;
		m_colors[i] = 0;
		m_spritemulti[i] = 0;
	}

	m_lastline = 0;
	m_rasterline = 0;

	memset(m_shift, 0, ARRAY_LENGTH(m_shift));
	memset(m_multi_collision, 0, ARRAY_LENGTH(m_multi_collision));
	memset(m_palette_red, 0, ARRAY_LENGTH(m_palette_red));
	memset(m_palette_green, 0, ARRAY_LENGTH(m_palette_green));
	memset(m_palette_blue, 0, ARRAY_LENGTH(m_palette_blue));

	m_palette_dirty = 0;
}


/*****************************************************************************
    IMPLEMENTATION
*****************************************************************************/

inline int vic3_device::getforeground( int y, int x )
{
	return ((m_screenptr[y][x >> 3] << 8) | (m_screenptr[y][(x >> 3) + 1])) >> (8 - (x & 7));
}

inline int vic3_device::getforeground16( int y, int x )
{
	return ((m_screenptr[y][x >> 3] << 16) | (m_screenptr[y][(x >> 3) + 1] << 8) | (m_screenptr[y][(x >> 3) + 2])) >> (8 - (x & 7));
}

void vic3_device::set_interrupt( int mask )
{
	if (((m_reg[0x19] ^ mask) & m_reg[0x1a] & 0xf))
	{
		if (!(m_reg[0x19] & 0x80))
		{
			//DBG_LOG(2, "vic2", ("irq start %.2x\n", mask));
			m_reg[0x19] |= 0x80;
			m_interrupt_cb(1);
		}
	}
	m_reg[0x19] |= mask;
}

void vic3_device::clear_interrupt( int mask )
{
	m_reg[0x19] &= ~mask;
	if ((m_reg[0x19] & 0x80) && !(m_reg[0x19] & m_reg[0x1a] & 0xf))
	{
		//DBG_LOG(2, "vic2", ("irq end %.2x\n", mask));
		m_reg[0x19] &= ~0x80;
		m_interrupt_cb(0);
	}
}

TIMER_CALLBACK_MEMBER( vic3_device::timer_timeout )
{
	int which = param;
	//DBG_LOG(3, "vic3 ", ("timer %d timeout\n", which));
	switch (which)
	{
	case 1:                        /* light pen */
		/* and diode must recognize light */
		if (1)
		{
			m_reg[0x13] = VIC2_X_VALUE;
			m_reg[0x14] = VIC2_Y_VALUE;
		}
		set_interrupt(8);
		break;
	}
}

void vic3_device::draw_character( int ybegin, int yend, int ch, int yoff, int xoff, UINT16 *color, int start_x, int end_x )
{
	int code;

	for (int y = ybegin; y <= yend; y++)
	{
		code = m_dma_read_cb(m_chargenaddr + ch * 8 + y);
		m_screenptr[y + yoff][xoff >> 3] = code;
		if ((xoff + 0 > start_x) && (xoff + 0 < end_x)) m_bitmap->pix16(y + yoff + FIRSTLINE, xoff + 0) = color[code >> 7];
		if ((xoff + 1 > start_x) && (xoff + 0 < end_x)) m_bitmap->pix16(y + yoff + FIRSTLINE, xoff + 1) = color[(code >> 6) & 1];
		if ((xoff + 2 > start_x) && (xoff + 0 < end_x)) m_bitmap->pix16(y + yoff + FIRSTLINE, xoff + 2) = color[(code >> 5) & 1];
		if ((xoff + 3 > start_x) && (xoff + 0 < end_x)) m_bitmap->pix16(y + yoff + FIRSTLINE, xoff + 3) = color[(code >> 4) & 1];
		if ((xoff + 4 > start_x) && (xoff + 0 < end_x)) m_bitmap->pix16(y + yoff + FIRSTLINE, xoff + 4) = color[(code >> 3) & 1];
		if ((xoff + 5 > start_x) && (xoff + 0 < end_x)) m_bitmap->pix16(y + yoff + FIRSTLINE, xoff + 5) = color[(code >> 2) & 1];
		if ((xoff + 6 > start_x) && (xoff + 0 < end_x)) m_bitmap->pix16(y + yoff + FIRSTLINE, xoff + 6) = color[(code >> 1) & 1];
		if ((xoff + 7 > start_x) && (xoff + 0 < end_x)) m_bitmap->pix16(y + yoff + FIRSTLINE, xoff + 7) = color[code & 1];
	}
}

void vic3_device::draw_character_multi( int ybegin, int yend, int ch, int yoff, int xoff, int start_x, int end_x )
{
	int code;

	for (int y = ybegin; y <= yend; y++)
	{
		code = m_dma_read_cb(m_chargenaddr + ch * 8 + y);
		m_screenptr[y + yoff][xoff >> 3] = m_foreground[code];
		if ((xoff + 0 > start_x) && (xoff + 0 < end_x)) m_bitmap->pix16(y + yoff + FIRSTLINE, xoff + 0) = m_multi[code >> 6];
		if ((xoff + 1 > start_x) && (xoff + 1 < end_x)) m_bitmap->pix16(y + yoff + FIRSTLINE, xoff + 1) = m_multi[code >> 6];
		if ((xoff + 2 > start_x) && (xoff + 2 < end_x)) m_bitmap->pix16(y + yoff + FIRSTLINE, xoff + 2) = m_multi[(code >> 4) & 3];
		if ((xoff + 3 > start_x) && (xoff + 3 < end_x)) m_bitmap->pix16(y + yoff + FIRSTLINE, xoff + 3) = m_multi[(code >> 4) & 3];
		if ((xoff + 4 > start_x) && (xoff + 4 < end_x)) m_bitmap->pix16(y + yoff + FIRSTLINE, xoff + 4) = m_multi[(code >> 2) & 3];
		if ((xoff + 5 > start_x) && (xoff + 5 < end_x)) m_bitmap->pix16(y + yoff + FIRSTLINE, xoff + 5) = m_multi[(code >> 2) & 3];
		if ((xoff + 6 > start_x) && (xoff + 6 < end_x)) m_bitmap->pix16(y + yoff + FIRSTLINE, xoff + 6) = m_multi[code & 3];
		if ((xoff + 7 > start_x) && (xoff + 7 < end_x)) m_bitmap->pix16(y + yoff + FIRSTLINE, xoff + 7) = m_multi[code & 3];
	}
}

void vic3_device::draw_bitmap( int ybegin, int yend, int ch, int yoff, int xoff, int start_x, int end_x )
{
	int code;

	for (int y = ybegin; y <= yend; y++)
	{
		code = m_dma_read_cb((m_chargenaddr & 0x2000) + ch * 8 + y);
		m_screenptr[y + yoff][xoff >> 3] = code;
		if ((xoff + 0 > start_x) && (xoff + 0 < end_x)) m_bitmap->pix16(y + yoff + FIRSTLINE, xoff + 0) = m_c64_bitmap[code >> 7];
		if ((xoff + 1 > start_x) && (xoff + 1 < end_x)) m_bitmap->pix16(y + yoff + FIRSTLINE, xoff + 1) = m_c64_bitmap[(code >> 6) & 1];
		if ((xoff + 2 > start_x) && (xoff + 2 < end_x)) m_bitmap->pix16(y + yoff + FIRSTLINE, xoff + 2) = m_c64_bitmap[(code >> 5) & 1];
		if ((xoff + 3 > start_x) && (xoff + 3 < end_x)) m_bitmap->pix16(y + yoff + FIRSTLINE, xoff + 3) = m_c64_bitmap[(code >> 4) & 1];
		if ((xoff + 4 > start_x) && (xoff + 4 < end_x)) m_bitmap->pix16(y + yoff + FIRSTLINE, xoff + 4) = m_c64_bitmap[(code >> 3) & 1];
		if ((xoff + 5 > start_x) && (xoff + 5 < end_x)) m_bitmap->pix16(y + yoff + FIRSTLINE, xoff + 5) = m_c64_bitmap[(code >> 2) & 1];
		if ((xoff + 6 > start_x) && (xoff + 6 < end_x)) m_bitmap->pix16(y + yoff + FIRSTLINE, xoff + 6) = m_c64_bitmap[(code >> 1) & 1];
		if ((xoff + 7 > start_x) && (xoff + 7 < end_x)) m_bitmap->pix16(y + yoff + FIRSTLINE, xoff + 7) = m_c64_bitmap[code & 1];
	}
}

void vic3_device::draw_bitmap_multi( int ybegin, int yend, int ch, int yoff, int xoff, int start_x, int end_x )
{
	int code;

	for (int y = ybegin; y <= yend; y++)
	{
		code = m_dma_read_cb((m_chargenaddr & 0x2000) + ch * 8 + y);
		m_screenptr[y + yoff][xoff >> 3] = m_foreground[code];
		if ((xoff + 0 > start_x) && (xoff + 0 < end_x)) m_bitmap->pix16(y + yoff + FIRSTLINE, xoff + 0) = m_bitmapmulti[code >> 6];
		if ((xoff + 1 > start_x) && (xoff + 1 < end_x)) m_bitmap->pix16(y + yoff + FIRSTLINE, xoff + 1) = m_bitmapmulti[code >> 6];
		if ((xoff + 2 > start_x) && (xoff + 2 < end_x)) m_bitmap->pix16(y + yoff + FIRSTLINE, xoff + 2) = m_bitmapmulti[(code >> 4) & 3];
		if ((xoff + 3 > start_x) && (xoff + 3 < end_x)) m_bitmap->pix16(y + yoff + FIRSTLINE, xoff + 3) = m_bitmapmulti[(code >> 4) & 3];
		if ((xoff + 4 > start_x) && (xoff + 4 < end_x)) m_bitmap->pix16(y + yoff + FIRSTLINE, xoff + 4) = m_bitmapmulti[(code >> 2) & 3];
		if ((xoff + 5 > start_x) && (xoff + 5 < end_x)) m_bitmap->pix16(y + yoff + FIRSTLINE, xoff + 5) = m_bitmapmulti[(code >> 2) & 3];
		if ((xoff + 6 > start_x) && (xoff + 6 < end_x)) m_bitmap->pix16(y + yoff + FIRSTLINE, xoff + 6) = m_bitmapmulti[code & 3];
		if ((xoff + 7 > start_x) && (xoff + 7 < end_x)) m_bitmap->pix16(y + yoff + FIRSTLINE, xoff + 7) = m_bitmapmulti[code & 3];
	}
}

void vic3_device::draw_sprite_code( int y, int xbegin, int code, int color, int start_x, int end_x )
{
	register int mask, x;

	if ((y < YPOS) || (y >= (VIC2_STARTVISIBLELINES + VIC2_VISIBLELINES)) || (xbegin <= 1) || (xbegin >= (VIC2_STARTVISIBLECOLUMNS + VIC2_VISIBLECOLUMNS)))
		return;

	for (x = 0, mask = 0x80; x < 8; x++, mask >>= 1)
	{
		if (code & mask)
		{
			if ((xbegin + x > start_x) && (xbegin + x < end_x))
				m_bitmap->pix16(y + FIRSTLINE, xbegin + x) = color;
		}
	}
}

void vic3_device::draw_sprite_code_multi( int y, int xbegin, int code, int prior, int start_x, int end_x )
{
	register int x, mask, shift;

	if ((y < YPOS) || (y >= (VIC2_STARTVISIBLELINES + VIC2_VISIBLELINES)) || (xbegin <= 1) || (xbegin >= (VIC2_STARTVISIBLECOLUMNS + VIC2_VISIBLECOLUMNS)))
		return;

	for (x = 0, mask = 0xc0, shift = 6; x < 8; x += 2, mask >>= 2, shift -= 2)
	{
		if (code & mask)
		{
			switch ((prior & mask) >> shift)
			{
			case 1:
				if ((xbegin + x + 1 > start_x) && (xbegin + x + 1 < end_x))
					m_bitmap->pix16(y + FIRSTLINE, xbegin + x + 1) = m_spritemulti[(code >> shift) & 3];
				break;
			case 2:
				if ((xbegin + x > start_x) && (xbegin + x < end_x))
					m_bitmap->pix16(y + FIRSTLINE, xbegin + x) = m_spritemulti[(code >> shift) & 3];
				break;
			case 3:
				if ((xbegin + x > start_x) && (xbegin + x < end_x))
					m_bitmap->pix16(y + FIRSTLINE, xbegin + x) = m_spritemulti[(code >> shift) & 3];
				if ((xbegin + x + 1> start_x) && (xbegin + x + 1< end_x))
					m_bitmap->pix16(y + FIRSTLINE, xbegin + x + 1) = m_spritemulti[(code >> shift) & 3];
				break;
			}
		}
	}
}

void vic3_device::sprite_collision( int nr, int y, int x, int mask )
{
	int value, xdiff;

	for (int i = 7; i > nr; i--)
	{
		if (!SPRITEON(i) || !m_sprites[i].paintedline[y] || (SPRITE_COLLISION(i) && SPRITE_COLLISION(nr)))
			continue;

		if ((x + 7 < SPRITE_X_POS(i)) || (x >= SPRITE_X_POS(i) + SPRITE_X_SIZE(i)))
			continue;

		xdiff = x - SPRITE_X_POS(i);

		if ((x & 7) == (SPRITE_X_POS(i) & 7))
			value = m_sprites[i].bitmap[y][xdiff >> 3];
		else if (xdiff < 0)
			value = m_sprites[i].bitmap[y][0] >> (-xdiff);
		else {
			UINT8 *vp = m_sprites[i].bitmap[y]+(xdiff >> 3);
			value = ((vp[1] | (*vp << 8)) >> (8 - (xdiff & 7) )) & 0xff;
		}

		if (value & mask)
		{
			SPRITE_SET_COLLISION(i);
			SPRITE_SET_COLLISION(nr);
			set_interrupt(4);
		}
	}
}

void vic3_device::draw_sprite( int nr, int yoff, int ybegin, int yend, int start_x, int end_x )
{
	int y, i, addr, xbegin, color, prior, collision;
	int value, value3 = 0;

	xbegin = SPRITE_X_POS(nr);
	addr = m_dma_read_cb(SPRITE_ADDR(nr)) << 6;
	color = SPRITE_COLOR(nr);
	prior = SPRITE_PRIORITY(nr);
	collision = SPRITE_BG_COLLISION(nr);

	if (SPRITE_X_EXPAND(nr))
	{
		for (y = ybegin; y <= yend; y++)
		{
			m_sprites[nr].paintedline[y] = 1;
			for (i = 0; i < 3; i++)
			{
				value = m_expandx[m_dma_read_cb(addr + m_sprites[nr].line * 3 + i)];
				m_sprites[nr].bitmap[y][i * 2] = value >> 8;
				m_sprites[nr].bitmap[y][i * 2 + 1] = value & 0xff;
				sprite_collision(nr, y, xbegin + i * 16, value >> 8);
				sprite_collision(nr, y, xbegin + i * 16 + 8, value & 0xff);
				if (prior || !collision)
					value3 = getforeground16(yoff + y, xbegin + i * 16 - 7);
				if (!collision && (value & value3))
				{
					collision = 1;
					SPRITE_SET_BG_COLLISION(nr);
					set_interrupt(2);
				}
				if (prior)
					value &= ~value3;
				draw_sprite_code(yoff + y, xbegin + i * 16, value >> 8, color, start_x, end_x);
				draw_sprite_code(yoff + y, xbegin + i * 16 + 8, value & 0xff, color, start_x, end_x);
			}
			m_sprites[nr].bitmap[y][i * 2]=0; //easier sprite collision detection
			if (SPRITE_Y_EXPAND(nr))
			{
				if (m_sprites[nr].repeat)
				{
					m_sprites[nr].line++;
					m_sprites[nr].repeat = 0;
				}
				else
					m_sprites[nr].repeat = 1;
			}
			else
			{
				m_sprites[nr].line++;
			}
		}
	}
	else
	{
		for (y = ybegin; y <= yend; y++)
		{
			m_sprites[nr].paintedline[y] = 1;
			for (i = 0; i < 3; i++)
			{
				value = m_dma_read_cb(addr + m_sprites[nr].line * 3 + i);
				m_sprites[nr].bitmap[y][i] = value;
				sprite_collision(nr, y, xbegin + i * 8, value);
				if (prior || !collision)
					value3 = getforeground(yoff + y, xbegin + i * 8 - 7);
				if (!collision && (value & value3))
				{
					collision = 1;
					SPRITE_SET_BG_COLLISION(nr);
					set_interrupt(2);
				}
				if (prior)
					value &= ~value3;
				draw_sprite_code(yoff + y, xbegin + i * 8, value, color, start_x, end_x);
			}
			m_sprites[nr].bitmap[y][i]=0; //easier sprite collision detection
			if (SPRITE_Y_EXPAND(nr))
			{
				if (m_sprites[nr].repeat)
				{
					m_sprites[nr].line++;
					m_sprites[nr].repeat = 0;
				}
				else
					m_sprites[nr].repeat = 1;
			}
			else
			{
				m_sprites[nr].line++;
			}
		}
	}
}

void vic3_device::draw_sprite_multi( int nr, int yoff, int ybegin, int yend, int start_x, int end_x )
{
	int y, i, prior, addr, xbegin, collision;
	int value, value2, value3 = 0, bg/*, color[2]*/;

	xbegin = SPRITE_X_POS(nr);
	addr = m_dma_read_cb(SPRITE_ADDR(nr)) << 6;
	m_spritemulti[2] = SPRITE_COLOR(nr);
	prior = SPRITE_PRIORITY(nr);
	collision = SPRITE_BG_COLLISION(nr);
	//color[0] = 0;
	//color[1] = 1;

	if (SPRITE_X_EXPAND(nr))
	{
		for (y = ybegin; y <= yend; y++)
		{
			m_sprites[nr].paintedline[y] = 1;
			for (i = 0; i < 3; i++)
			{
				value = m_expandx_multi[bg = m_dma_read_cb(addr + m_sprites[nr].line * 3 + i)];
				value2 = m_expandx[m_multi_collision[bg]];
				m_sprites[nr].bitmap[y][i * 2] = value2 >> 8;
				m_sprites[nr].bitmap[y][i * 2 + 1] = value2 & 0xff;
				sprite_collision(nr, y, xbegin + i * 16, value2 >> 8);
				sprite_collision(nr, y, xbegin + i * 16 + 8, value2 & 0xff);
				if (prior || !collision)
				{
					value3 = getforeground16(yoff + y, xbegin + i * 16 - 7);
				}
				if (!collision && (value2 & value3))
				{
					collision = 1;
					SPRITE_SET_BG_COLLISION(nr);
					set_interrupt(2);
				}
				if (prior)
				{
					draw_sprite_code_multi(yoff + y, xbegin + i * 16, value >> 8, (value3 >> 8) ^ 0xff, start_x, end_x);
					draw_sprite_code_multi(yoff + y, xbegin + i * 16 + 8, value & 0xff, (value3 & 0xff) ^ 0xff, start_x, end_x);
				}
				else
				{
					draw_sprite_code_multi(yoff + y, xbegin + i * 16, value >> 8, 0xff, start_x, end_x);
					draw_sprite_code_multi(yoff + y, xbegin + i * 16 + 8, value & 0xff, 0xff, start_x, end_x);
				}
			}
			m_sprites[nr].bitmap[y][i * 2]=0; //easier sprite collision detection
			if (SPRITE_Y_EXPAND(nr))
			{
				if (m_sprites[nr].repeat)
				{
					m_sprites[nr].line++;
					m_sprites[nr].repeat = 0;
				}
				else
					m_sprites[nr].repeat = 1;
			}
			else
			{
				m_sprites[nr].line++;
			}
		}
	}
	else
	{
		for (y = ybegin; y <= yend; y++)
		{
			m_sprites[nr].paintedline[y] = 1;
			for (i = 0; i < 3; i++)
			{
				value = m_dma_read_cb(addr + m_sprites[nr].line * 3 + i);
				m_sprites[nr].bitmap[y][i] = value2 = m_multi_collision[value];
				sprite_collision(nr, y, xbegin + i * 8, value2);
				if (prior || !collision)
				{
					value3 = getforeground(yoff + y, xbegin + i * 8 - 7);
				}
				if (!collision && (value2 & value3))
				{
					collision = 1;
					SPRITE_SET_BG_COLLISION(nr);
					set_interrupt(2);
				}
				if (prior)
				{
					draw_sprite_code_multi(yoff + y, xbegin + i * 8, value, value3 ^ 0xff, start_x, end_x);
				}
				else
				{
					draw_sprite_code_multi(yoff + y, xbegin + i * 8, value, 0xff, start_x, end_x);
				}
			}
			m_sprites[nr].bitmap[y][i] = 0; //easier sprite collision detection
			if (SPRITE_Y_EXPAND(nr))
			{
				if (m_sprites[nr].repeat)
				{
					m_sprites[nr].line++;
					m_sprites[nr].repeat = 0;
				}
				else
					m_sprites[nr].repeat = 1;
			}
			else
			{
				m_sprites[nr].line++;
			}
		}
	}
}

#ifndef memset16
static void *memset16(void *dest, int value, size_t size)
{
	register int i;

	for (i = 0; i < size; i++)
		((short *) dest)[i] = value;
	return dest;
}
#endif

void vic3_device::drawlines( int first, int last, int start_x, int end_x )
{
	int line, vline, end;
	int attr, ch, ecm;
	int syend;
	int offs, yoff, xoff, ybegin, yend, xbegin, xend;
	int x_end2;
	int i, j;

	if (first == last)
		return;
	m_lastline = last;

	/* top part of display not rastered */
	first -= VIC2_YPOS - YPOS;
	last -= VIC2_YPOS - YPOS;
	if ((first >= last) || (last <= 0))
	{
		for (i = 0; i < 8; i++)
			m_sprites[i].repeat = m_sprites[i].line = 0;
		return;
	}
	if (first < 0)
		first = 0;

	if (!SCREENON)
	{
		for (line = first; (line < last) && (line < m_bitmap->height()); line++)
		{
			memset16(&m_bitmap->pix16(line + FIRSTLINE), 0, m_bitmap->width());
		}
		return;
	}
	if (COLUMNS40)
		xbegin = XPOS, xend = xbegin + 640;
	else
		xbegin = XPOS + 7, xend = xbegin + 624;

	if (last < m_y_begin)
		end = last;
	else
		end = m_y_begin + YPOS;

	for (line = first; line < end; line++)
	{
		memset16(&m_bitmap->pix16(line + FIRSTLINE), FRAMECOLOR, m_bitmap->width());
	}

	if (LINES25)
	{
		vline = line - m_y_begin - YPOS;
	}
	else
	{
		vline = line - m_y_begin - YPOS + 8 - VERTICALPOS;
	}
	if (last < m_y_end + YPOS)
		end = last;
	else
		end = m_y_end + YPOS;
	x_end2 = m_x_end * 2;
	for (; line < end; vline = (vline + 8) & ~7, line = line + 1 + yend - ybegin)
	{
		offs = (vline >> 3) * 80;
		ybegin = vline & 7;
		yoff = line - ybegin;
		yend = (yoff + 7 < end) ? 7 : (end - yoff - 1);
		/* rendering 39 characters */
		/* left and right borders are overwritten later */
		m_shift[line] = HORIZONTALPOS;

		for (xoff = m_x_begin + XPOS; xoff < x_end2 + XPOS; xoff += 8, offs++)
		{
			ch = m_dma_read_cb(m_videoaddr + offs);
			attr = m_dma_read_color_cb(m_videoaddr + offs);
			if (HIRESON)
			{
				m_bitmapmulti[1] = m_c64_bitmap[1] = ch >> 4;
				m_bitmapmulti[2] = m_c64_bitmap[0] = ch & 0xf;
				if (MULTICOLORON)
				{
					m_bitmapmulti[3] = attr;
					draw_bitmap_multi(ybegin, yend, offs, yoff, xoff, start_x, end_x);
				}
				else
				{
					draw_bitmap(ybegin, yend, offs, yoff, xoff, start_x, end_x);
				}
			}
			else if (ECMON)
			{
				ecm = ch >> 6;
				m_ecmcolor[0] = m_colors[ecm];
				m_ecmcolor[1] = attr;
				draw_character(ybegin, yend, ch & ~0xC0, yoff, xoff, m_ecmcolor, start_x, end_x);
			}
			else if (MULTICOLORON && (attr & 8))
			{
				m_multi[3] = attr & 7;
				draw_character_multi(ybegin, yend, ch, yoff, xoff, start_x, end_x);
			}
			else
			{
				m_mono[1] = attr;
				draw_character(ybegin, yend, ch, yoff, xoff, m_mono, start_x, end_x);
			}
		}
		/* sprite priority, sprite overwrites lowerprior pixels */
		for (i = 7; i >= 0; i--)
		{
			if (m_sprites[i].line || m_sprites[i].repeat)
			{
				syend = yend;
				if (SPRITE_Y_EXPAND(i))
				{
					if ((21 - m_sprites[i].line) * 2 - m_sprites[i].repeat < yend - ybegin + 1)
						syend = ybegin + (21 - m_sprites[i].line) * 2 - m_sprites[i].repeat - 1;
				}
				else
				{
					if (m_sprites[i].line + yend - ybegin + 1 > 20)
						syend = ybegin + 20 - m_sprites[i].line;
				}
				if (yoff + syend > YPOS + 200)
					syend = YPOS + 200 - yoff - 1;
				if (SPRITE_MULTICOLOR(i))
					draw_sprite_multi(i, yoff, ybegin, syend, start_x, end_x);
				else
					draw_sprite(i, yoff, ybegin, syend, start_x, end_x);
				if ((syend != yend) || (m_sprites[i].line > 20))
				{
					m_sprites[i].line = m_sprites[i].repeat = 0;
					for (j = syend; j <= yend; j++)
						m_sprites[i].paintedline[j] = 0;
				}
			}
			// sprite wrap y at the top of the screen
			else if (SPRITEON(i) && (yoff == 1 + yend - ybegin) && (SPRITE_Y_POS(i) < 1 + yend - ybegin))
			{
				int wrapped = 1 + yend - ybegin - SPRITE_Y_POS(i);
				syend = yend;

				if (SPRITE_Y_EXPAND(i))
				{
					if (wrapped & 1) m_sprites[i].repeat = 1;
					wrapped >>= 1;
					syend = 21 * 2 - 1 - wrapped * 2;
					if (syend > (yend - ybegin)) syend = yend - ybegin;
				}
				else
				{
					syend = 21 - 1 - wrapped;
					if (syend > (yend - ybegin)) syend = yend - ybegin;
				}

				m_sprites[i].line = wrapped;

				if (SPRITE_MULTICOLOR(i))
					draw_sprite_multi(i, yoff, 0 , syend, start_x, end_x);
				else
					draw_sprite(i, yoff, 0 , syend, start_x, end_x);

				if ((syend != yend) || (m_sprites[i].line > 20))
				{
					for (j = syend; j <= yend; j++)
						m_sprites[i].paintedline[j] = 0;
					m_sprites[i].line = m_sprites[i].repeat = 0;
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
					m_sprites[i].paintedline[j] = 0;
				if (SPRITE_MULTICOLOR(i))
					draw_sprite_multi(i, yoff, SPRITE_Y_POS(i) - yoff, syend, start_x, end_x);
				else
					draw_sprite(i, yoff, SPRITE_Y_POS(i) - yoff, syend, start_x, end_x);
				if ((syend != yend) || (m_sprites[i].line > 20))
				{
					for (j = syend; j <= yend; j++)
						m_sprites[i].paintedline[j] = 0;
					m_sprites[i].line = m_sprites[i].repeat = 0;
				}
			}
			else
			{
				memset (m_sprites[i].paintedline, 0, sizeof (m_sprites[i].paintedline));
			}
		}

		for (i = ybegin; i <= yend; i++)
		{
			m_bitmap->plot_box(0, yoff + ybegin + FIRSTLINE, xbegin, yend - ybegin + 1, FRAMECOLOR);
			m_bitmap->plot_box(xend, yoff + ybegin + FIRSTLINE, m_bitmap->width() - xend, yend - ybegin + 1, FRAMECOLOR);
		}
	}
	if (last < m_bitmap->height())
		end = last;
	else
		end = m_bitmap->height();

	for (; line < end; line++)
	{
		memset16(&m_bitmap->pix16(line + FIRSTLINE), FRAMECOLOR, m_bitmap->width());
	}
}

void vic3_device::vic2_drawlines( int first, int last, int start_x, int end_x )
{
	int line, vline, end;
	int attr, ch, ecm;
	int syend;
	int offs, yoff, xoff, ybegin, yend, xbegin, xend;
	int i;

	if (VIC3_BITPLANES)
		return ;

	/* temporary allowing vic3 displaying 80 columns */
	if (m_reg[0x31] & 0x80)
	{
		drawlines(first, first + 1, start_x, end_x);
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
			m_bitmap->plot_box(xbegin, first + FIRSTLINE, xend - xbegin, 1, FRAMECOLOR);
		if ((start_x > xbegin) && (end_x >= xend))
			m_bitmap->plot_box(start_x - VIC2_STARTVISIBLECOLUMNS, first + FIRSTLINE, xend - start_x, 1, FRAMECOLOR);
		if ((start_x <= xbegin) && (end_x < xend))
			m_bitmap->plot_box(xbegin, first + FIRSTLINE, end_x - xbegin , 1, FRAMECOLOR);
		if ((start_x > xbegin) && (end_x < xend))
			m_bitmap->plot_box(start_x - VIC2_STARTVISIBLECOLUMNS, first + FIRSTLINE, end_x - start_x, 1, FRAMECOLOR);
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

	if (first + 1 < m_y_begin)
		end = first + 1;
	else
		end = m_y_begin + YPOS;

	line = first;
	// top border
	if (line < end)
	{
		if ((start_x <= xbegin) && (end_x >= xend))
			m_bitmap->plot_box(xbegin, first + FIRSTLINE, xend - xbegin, 1, FRAMECOLOR);
		if ((start_x > xbegin) && (end_x >= xend))
			m_bitmap->plot_box(start_x - VIC2_STARTVISIBLECOLUMNS, first + FIRSTLINE, xend - start_x, 1, FRAMECOLOR);
		if ((start_x <= xbegin) && (end_x < xend))
			m_bitmap->plot_box(xbegin, first + FIRSTLINE, end_x - xbegin , 1, FRAMECOLOR);
		if ((start_x > xbegin) && (end_x < xend))
			m_bitmap->plot_box(start_x - VIC2_STARTVISIBLECOLUMNS, first + FIRSTLINE, end_x - start_x, 1, FRAMECOLOR);
		line = end;
	}

	vline = line - YPOS + 3 - VERTICALPOS;

	if (first + 1 < m_y_end + YPOS)
		end = first + 1;
	else
		end = m_y_end + YPOS;

	if (line < end)
	{
		offs = (vline >> 3) * 40;
		ybegin = vline & 7;
		yoff = line - ybegin;
		yend = (yoff + 7 < end) ? 7 : (end - yoff - 1);

		/* rendering 39 characters */
		/* left and right borders are overwritten later */

		m_shift[line] = HORIZONTALPOS;
		for (xoff = m_x_begin + XPOS; xoff < m_x_end + XPOS; xoff += 8, offs++)
		{
			ch = m_dma_read_cb(m_videoaddr + offs);
#if 0
			attr = m_dma_read_color_cb(m_videoaddr + offs);
#else
			/* temporary until vic3 finished */
			attr = m_dma_read_color_cb((m_videoaddr + offs)&0x3ff)&0x0f;
#endif
			if (HIRESON)
			{
				m_bitmapmulti[1] = m_c64_bitmap[1] = ch >> 4;
				m_bitmapmulti[2] = m_c64_bitmap[0] = ch & 0xf;
				if (MULTICOLORON)
				{
					m_bitmapmulti[3] = attr;
					draw_bitmap_multi(ybegin, yend, offs, yoff, xoff, start_x, end_x);
				}
				else
				{
					draw_bitmap(ybegin, yend, offs, yoff, xoff, start_x, end_x);
				}
			}
			else if (ECMON)
			{
				ecm = ch >> 6;
				m_ecmcolor[0] = m_colors[ecm];
				m_ecmcolor[1] = attr;
				draw_character(ybegin, yend, ch & ~0xC0, yoff, xoff, m_ecmcolor, start_x, end_x);
			}
			else if (MULTICOLORON && (attr & 8))
			{
				m_multi[3] = attr & 7;
				draw_character_multi(ybegin, yend, ch, yoff, xoff, start_x, end_x);
			}
			else
			{
				m_mono[1] = attr;
				draw_character(ybegin, yend, ch, yoff, xoff, m_mono, start_x, end_x);
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
					if (wrapped & 1) m_sprites[i].repeat = 1;
					wrapped >>= 1;
					syend = 21 * 2 - 1 - wrapped * 2;
					if (syend > (yend - ybegin)) syend = yend - ybegin;
				}
				else
				{
					syend = 21 - 1 - wrapped;
					if (syend > (yend - ybegin)) syend = yend - ybegin;
				}

				m_sprites[i].line = wrapped;

				if (SPRITE_MULTICOLOR (i))
					draw_sprite_multi(i, 0, 0 , syend, start_x, end_x);
				else
					draw_sprite(i, 0, 0 , syend, start_x, end_x);
			}
			else if     (SPRITEON (i) &&
					(yoff + ybegin >= SPRITE_Y_POS (i)) &&
					(yoff + ybegin - SPRITE_Y_POS (i) < (SPRITE_Y_EXPAND (i)? 21 * 2 : 21 )) &&
					(SPRITE_Y_POS (i) >= 0))
			{
				int wrapped = yoff + ybegin - SPRITE_Y_POS (i);

				syend = yend;

				if (SPRITE_Y_EXPAND (i))
				{
					if (wrapped & 1) m_sprites[i].repeat = 1;
					wrapped >>= 1;
					syend = 21 * 2 - 1 - wrapped * 2;
					if (syend > (yend - ybegin)) syend = yend - ybegin;
				}
				else
				{
					syend = 21 - 1 - wrapped;
					if (syend > (yend - ybegin)) syend = yend - ybegin;
				}

				m_sprites[i].line = wrapped;

				if (SPRITE_MULTICOLOR (i))
					draw_sprite_multi(i, yoff + ybegin, 0, 0, start_x, end_x);
				else
					draw_sprite(i, yoff + ybegin, 0, 0, start_x, end_x);
			}
			else
			{
				memset(m_sprites[i].paintedline, 0, sizeof (m_sprites[i].paintedline));
			}
		}
		line += 1 + yend - ybegin;
	}

	// left border
	if ((start_x <= VIC2_STARTVISIBLECOLUMNS) && (end_x >= xbegin))
		m_bitmap->plot_box(VIC2_STARTVISIBLECOLUMNS, first + FIRSTLINE, xbegin - VIC2_STARTVISIBLECOLUMNS, 1, FRAMECOLOR);
	else if ((start_x > VIC2_STARTVISIBLECOLUMNS) && (end_x >= xbegin))
		m_bitmap->plot_box(start_x, first + FIRSTLINE, xbegin - start_x, 1, FRAMECOLOR);
	else if ((start_x <= VIC2_STARTVISIBLECOLUMNS) && (end_x < xbegin))
		m_bitmap->plot_box(VIC2_STARTVISIBLECOLUMNS, first + FIRSTLINE, end_x, 1, FRAMECOLOR);
	else if ((start_x > VIC2_STARTVISIBLECOLUMNS) && (end_x < xbegin))
		m_bitmap->plot_box(start_x, first + FIRSTLINE, end_x - start_x, 1, FRAMECOLOR);

	// right border
	if ((start_x <= xend) && (end_x >= VIC2_STARTVISIBLECOLUMNS + VIC2_VISIBLECOLUMNS))
		m_bitmap->plot_box(xend, first + FIRSTLINE, VIC2_STARTVISIBLECOLUMNS + VIC2_VISIBLECOLUMNS - xend, 1, FRAMECOLOR);
	else if ((start_x > xend) && (end_x >= VIC2_STARTVISIBLECOLUMNS + VIC2_VISIBLECOLUMNS))
		m_bitmap->plot_box(start_x, first + FIRSTLINE, VIC2_STARTVISIBLECOLUMNS + VIC2_VISIBLECOLUMNS - start_x, 1, FRAMECOLOR);
	else if ((start_x <= xend) && (end_x < VIC2_STARTVISIBLECOLUMNS + VIC2_VISIBLECOLUMNS))
		m_bitmap->plot_box(xend, first + FIRSTLINE, end_x - xend, 1, FRAMECOLOR);
	else if ((start_x > VIC2_STARTVISIBLECOLUMNS) && (end_x < xbegin))
		m_bitmap->plot_box(start_x, first + FIRSTLINE, end_x - start_x, 1, FRAMECOLOR);

	if (first + 1 < m_bitmap->height())
		end = first + 1;
	else
		end = m_bitmap->height();

	// bottom border
	if (line < end)
	{
		if ((start_x <= xbegin) && (end_x >= xend))
			m_bitmap->plot_box(xbegin, first + FIRSTLINE, xend - xbegin, 1, FRAMECOLOR);
		if ((start_x > xbegin) && (end_x >= xend))
			m_bitmap->plot_box(start_x - VIC2_STARTVISIBLECOLUMNS, first + FIRSTLINE, xend - start_x, 1, FRAMECOLOR);
		if ((start_x <= xbegin) && (end_x < xend))
			m_bitmap->plot_box(xbegin, first + FIRSTLINE, end_x - xbegin , 1, FRAMECOLOR);
		if ((start_x > xbegin) && (end_x < xend))
			m_bitmap->plot_box(start_x - VIC2_STARTVISIBLECOLUMNS, first + FIRSTLINE, end_x - start_x, 1, FRAMECOLOR);
		line = end;
	}
}

/*****************************************************************************
    I/O HANDLERS
*****************************************************************************/

WRITE8_MEMBER( vic3_device::palette_w )
{
	if (offset < 0x100)
		m_palette_red[offset] = data;
	else if (offset < 0x200)
		m_palette_green[offset & 0xff] = data;
	else
		m_palette_blue[offset & 0xff] = data;

	m_palette_dirty = 1;
}


WRITE8_MEMBER( vic3_device::port_w )
{
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
		if (m_reg[offset] != data)
		{
			m_reg[offset] = data;
			m_sprites[offset / 2].y = SPRITE_Y_POS(offset / 2);
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
		if (m_reg[offset] != data)
		{
			m_reg[offset] = data;
			m_sprites[offset / 2].x = SPRITE_X_POS(offset / 2);
		}
		break;

	case 0x10:                          /* sprite x positions */
		if (m_reg[offset] != data)
		{
			m_reg[offset] = data;
			m_sprites[0].x = SPRITE_X_POS(0);
			m_sprites[1].x = SPRITE_X_POS(1);
			m_sprites[2].x = SPRITE_X_POS(2);
			m_sprites[3].x = SPRITE_X_POS(3);
			m_sprites[4].x = SPRITE_X_POS(4);
			m_sprites[5].x = SPRITE_X_POS(5);
			m_sprites[6].x = SPRITE_X_POS(6);
			m_sprites[7].x = SPRITE_X_POS(7);
		}
		break;

	case 0x17:                          /* sprite y size */
		if (m_reg[offset] != data)
		{
			m_reg[offset] = data;
		}
		break;

	case 0x1d:                          /* sprite x size */
		if (m_reg[offset] != data)
		{
			m_reg[offset] = data;
		}
		break;

	case 0x1b:                          /* sprite background priority */
		if (m_reg[offset] != data)
		{
			m_reg[offset] = data;
		}
		break;

	case 0x1c:                          /* sprite multicolor mode select */
		if (m_reg[offset] != data)
		{
			m_reg[offset] = data;
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
		if (m_reg[offset] != data)
		{
			m_reg[offset] = data;
		}
		break;

	case 0x25:                          /* sprite multicolor */
		if (m_reg[offset] != data)
		{
			m_reg[offset] = data;
			m_spritemulti[1] = SPRITE_MULTICOLOR1;
		}
		break;

	case 0x26:                          /* sprite multicolor */
		if (m_reg[offset] != data)
		{
			m_reg[offset] = data;
			m_spritemulti[3] = SPRITE_MULTICOLOR2;
		}
		break;

	case 0x19:
		clear_interrupt(data & 0x0f);
		break;

	case 0x1a:                          /* irq mask */
		m_reg[offset] = data;
		set_interrupt(0);   // beamrider needs this
		break;

	case 0x11:
		if (m_reg[offset] != data)
		{
			m_reg[offset] = data;
			if (LINES25)
			{
				m_y_begin = 0;
				m_y_end = m_y_begin + 200;
			}
			else
			{
				m_y_begin = 4;
				m_y_end = m_y_begin + 192;
			}
		}
		break;

	case 0x12:
		if (data != m_reg[offset])
		{
			m_reg[offset] = data;
		}
		break;

	case 0x16:
		if (m_reg[offset] != data)
		{
			m_reg[offset] = data;
			m_x_begin = HORIZONTALPOS;
			m_x_end = m_x_begin + 320;
		}
		break;

	case 0x18:
		if (m_reg[offset] != data)
		{
			m_reg[offset] = data;
			m_videoaddr = VIDEOADDR;
			m_chargenaddr = CHARGENADDR;
			m_bitmapaddr = BITMAPADDR;
		}
		break;

	case 0x21:                          /* background color */
		if (m_reg[offset] != data)
		{
			m_reg[offset] = data;
			m_mono[0] = m_bitmapmulti[0] = m_multi[0] = m_colors[0] = BACKGROUNDCOLOR;
		}
		break;

	case 0x22:                          /* background color 1 */
		if (m_reg[offset] != data)
		{
			m_reg[offset] = data;
			m_multi[1] = m_colors[1] = MULTICOLOR1;
		}
		break;

	case 0x23:                          /* background color 2 */
		if (m_reg[offset] != data)
		{
			m_reg[offset] = data;
			m_multi[2] = m_colors[2] = MULTICOLOR2;
		}
		break;

	case 0x24:                          /* background color 3 */
		if (m_reg[offset] != data)
		{
			m_reg[offset] = data;
			m_colors[3] = FOREGROUNDCOLOR;
		}
		break;

	case 0x20:                          /* framecolor */
		if (m_reg[offset] != data)
		{
			m_reg[offset] = data;
		}
		break;

	case 0x2f:
		DBG_LOG(2, "vic write", ("%.2x:%.2x\n", offset, data));
		m_reg[offset] = data;
		break;
	case 0x30:
		m_reg[offset] = data;
		if (!m_port_changed_cb.isnull())
		{
			DBG_LOG(2, "vic write", ("%.2x:%.2x\n", offset, data));
			m_reg[offset] = data;
			m_port_changed_cb((offs_t)0,data);
		}
		break;
	case 0x31:
		m_reg[offset] = data;
		if (data & 0x40)
			m_cpu->set_clock_scale(1.0);
		else
			m_cpu->set_clock_scale(1.0/3.5);
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
		m_reg[offset] = data;
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
		m_reg[offset] = data;
		break;
	}
}

READ8_MEMBER( vic3_device::port_r )
{
	int val = 0;
	offset &= 0x7f;

	/* offsets 0x00 -> 0x2e coincide with VICII */
	switch (offset)
	{
	case 0x11:
		val = (m_reg[offset] & ~0x80) | ((m_rasterline & 0x100) >> 1);
		break;

	case 0x12:
		val = m_rasterline & 0xff;
		break;

	case 0x16:
		val = m_reg[offset] | 0xc0;
		break;

	case 0x18:
		val = m_reg[offset] | 0x01;
		break;

	case 0x19:                          /* interrupt flag register */
		/* vic2_clear_interrupt(0xf); */
		val = m_reg[offset] | 0x70;
		break;

	case 0x1a:
		val = m_reg[offset] | 0xf0;
		break;

	case 0x1e:                          /* sprite to sprite collision detect */
		val = m_reg[offset];
		m_reg[offset] = 0;
		clear_interrupt(4);
		break;

	case 0x1f:                          /* sprite to background collision detect */
		val = m_reg[offset];
		m_reg[offset] = 0;
		clear_interrupt(2);
		break;

	case 0x20:
	case 0x21:
	case 0x22:
	case 0x23:
	case 0x24:
		val = m_reg[offset];
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
		val = m_reg[offset];
		break;

	case 0x2f:
	case 0x30:
		val = m_reg[offset];
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
	case 0x3f:                         /* not used */
		val = m_reg[offset];
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
		val = m_reg[offset];
	}
	return val;
}


#define VIC3_MASK(M)                                            \
	if (M)                                                      \
	{                                                           \
		if (M & 0x01)                                           \
			colors[0] = m_c64_mem_r_cb(VIC3_ADDR(0) + offset);        \
		if (M & 0x02)                                           \
			colors[1] = m_c64_mem_r_cb(VIC3_ADDR(1) + offset) << 1;     \
		if (M & 0x04)                                           \
			colors[2] = m_c64_mem_r_cb(VIC3_ADDR(2) + offset) << 2;     \
		if (M & 0x08)                                           \
			colors[3] = m_c64_mem_r_cb(VIC3_ADDR(3) + offset) << 3;     \
		if (M & 0x10)                                           \
			colors[4] = m_c64_mem_r_cb(VIC3_ADDR(4) + offset) << 4;     \
		if (M & 0x20)                                           \
			colors[5] = m_c64_mem_r_cb(VIC3_ADDR(5) + offset) << 5;     \
		if (M & 0x40)                        \
			colors[6] = m_c64_mem_r_cb(VIC3_ADDR(6) + offset) << 6;     \
		if (M & 0x80)                        \
			colors[7] = m_c64_mem_r_cb(VIC3_ADDR(7) + offset) << 7;     \
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
			m_bitmap->pix16(YPOS + y, XPOS + x + i) = p; \
		}                                                       \
	}

#define VIC3_ADDR(a) VIC3_BITPLANE_IADDR(a)
void vic3_device::interlace_draw_block( int x, int y, int offset )
{
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
			colors[0] = m_c64_mem_r_cb(VIC3_BITPLANE_IADDR(0) + offset);

		if (VIC3_BITPLANES_MASK & 0x02)
			colors[1] = m_c64_mem_r_cb(VIC3_BITPLANE_IADDR(1) + offset) << 1;

		if (VIC3_BITPLANES_MASK & 0x04)
			colors[2] = m_c64_mem_r_cb(VIC3_BITPLANE_IADDR(2) + offset) << 2;

		if (VIC3_BITPLANES_MASK & 0x08)
			colors[3] = m_c64_mem_r_cb(VIC3_BITPLANE_IADDR(3) + offset) << 3;

		if (VIC3_BITPLANES_MASK & 0x10)
			colors[4] = m_c64_mem_r_cb(VIC3_BITPLANE_IADDR(4) + offset) << 4;

		if (VIC3_BITPLANES_MASK & 0x20)
			colors[5] = m_c64_mem_r_cb(VIC3_BITPLANE_IADDR(5) + offset) << 5;

		if (VIC3_BITPLANES_MASK & 0x40)
			colors[6] = m_c64_mem_r_cb(VIC3_BITPLANE_IADDR(6) + offset) << 6;

		if (VIC3_BITPLANES_MASK & 0x80)
			colors[7] = m_c64_mem_r_cb(VIC3_BITPLANE_IADDR(7) + offset) << 7;

		for (i = 7; i >= 0; i--)
		{
			m_bitmap->pix16(YPOS + y, XPOS + x + i) =
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
void vic3_device::draw_block( int x, int y, int offset )
{
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
			colors[0] = m_c64_mem_r_cb(VIC3_BITPLANE_ADDR(0) + offset);

		if (VIC3_BITPLANES_MASK & 0x02)
			colors[1] = m_c64_mem_r_cb(VIC3_BITPLANE_ADDR(1) + offset) << 1;

		if (VIC3_BITPLANES_MASK & 0x04)
			colors[2] = m_c64_mem_r_cb(VIC3_BITPLANE_ADDR(2) + offset) << 2;

		if (VIC3_BITPLANES_MASK & 0x08)
			colors[3] = m_c64_mem_r_cb(VIC3_BITPLANE_ADDR(3) + offset) << 3;

		if (VIC3_BITPLANES_MASK & 0x10)
			colors[4] = m_c64_mem_r_cb(VIC3_BITPLANE_ADDR(4) + offset) << 4;

		if (VIC3_BITPLANES_MASK & 0x20)
			colors[5] = m_c64_mem_r_cb(VIC3_BITPLANE_ADDR(5) + offset) << 5;

		if (VIC3_BITPLANES_MASK & 0x40)
			colors[6] = m_c64_mem_r_cb(VIC3_BITPLANE_ADDR(6) + offset) << 6;

		if (VIC3_BITPLANES_MASK & 0x80)
			colors[7] = m_c64_mem_r_cb(VIC3_BITPLANE_ADDR(7) + offset) << 7;

		for (i = 7; i >= 0; i--)
		{
			m_bitmap->pix16(YPOS + y, XPOS + x + i) =
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


void vic3_device::draw_bitplanes()
{
	int x, y, y1s, offset;
	rectangle vis;
	const rectangle &visarea = m_screen->visible_area();

	if (VIC3_LINES == 400)
	{ /* interlaced! */
		for (y1s = 0, offset = 0; y1s < 400; y1s += 16)
		{
			for (x = 0; x < VIC3_BITPLANES_WIDTH; x += 8)
			{
				for (y = y1s; y < y1s + 16; y += 2, offset++)
				{
					if (m_interlace)
						draw_block(x, y, offset);
					else
						interlace_draw_block(x, y + 1, offset);
				}
			}
		}
		m_interlace ^= 1;
	}
	else
	{
		for (y1s = 0, offset = 0; y1s < 200; y1s += 8)
		{
			for (x = 0; x < VIC3_BITPLANES_WIDTH; x += 8)
			{
				for (y = y1s; y < y1s + 8; y++, offset++)
				{
					draw_block(x, y, offset);
				}
			}
		}
	}

	if (XPOS > 0)
	{
		vis.set(0, XPOS - 1, 0, visarea.max_y);
		m_bitmap->fill(FRAMECOLOR, vis);
	}

	if (XPOS + VIC3_BITPLANES_WIDTH < visarea.max_x)
	{
		vis.set(XPOS + VIC3_BITPLANES_WIDTH, visarea.max_x, 0, visarea.max_y);
		m_bitmap->fill(FRAMECOLOR, vis);
	}

	if (YPOS > 0)
	{
		vis.set(0, visarea.max_x, 0, YPOS - 1);
		m_bitmap->fill(FRAMECOLOR, vis);
	}

	if (YPOS + VIC3_LINES < visarea.max_y)
	{
		vis.set(0, visarea.max_x, YPOS + VIC3_LINES, visarea.max_y);
		m_bitmap->fill(FRAMECOLOR, vis);
	}
}

void vic3_device::raster_interrupt_gen()
{
	int new_columns, new_rows;
	int i;

	m_rasterline++;
	if (m_rasterline >= m_lines)
	{
		m_rasterline = 0;
		if (m_palette_dirty)
			for (i = 0; i < 256; i++)
				m_palette->set_pen_color(i, m_palette_red[i] << 4, m_palette_green[i] << 4, m_palette_blue[i] << 4);

		if (m_palette_dirty)
		{
			m_spritemulti[1] = SPRITE_MULTICOLOR1;
			m_spritemulti[3] = SPRITE_MULTICOLOR2;
			m_mono[0] = m_bitmapmulti[0] = m_multi[0] = m_colors[0] = BACKGROUNDCOLOR;
			m_multi[1] = m_colors[1] = MULTICOLOR1;
			m_multi[2] = m_colors[2] = MULTICOLOR2;
			m_colors[3] = FOREGROUNDCOLOR;
			m_palette_dirty = 0;
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
		if ((new_columns != m_columns) || (new_rows != m_rows))
		{
			m_rows = new_rows;
			m_columns = new_columns;
			if (m_type == VIC4567_PAL)
				m_screen->set_visible_area(
									VIC2_STARTVISIBLECOLUMNS + 32,
									VIC2_STARTVISIBLECOLUMNS + 32 + m_columns + 16 - 1,
									VIC2_STARTVISIBLELINES + 34,
									VIC2_STARTVISIBLELINES + 34 + m_rows + 16 - 1);
			else
				m_screen->set_visible_area(
									VIC2_STARTVISIBLECOLUMNS + 34,
									VIC2_STARTVISIBLECOLUMNS + 34 + m_columns + 16 - 1,
									VIC2_STARTVISIBLELINES + 10,
									VIC2_STARTVISIBLELINES + 10 + m_rows + 16 - 1);
		}
		if (VIC3_BITPLANES)
		{
			draw_bitplanes();
		}
		else
		{
			if (m_type == VIC4567_PAL)
			{
				if (m_on)
					vic2_drawlines(m_lastline, m_lines, VIC2_STARTVISIBLECOLUMNS + 32, VIC2_STARTVISIBLECOLUMNS + 32 + m_columns + 16 - 1);
			}
			else
			{
				if (m_on)
					vic2_drawlines(m_lastline, m_lines, VIC2_STARTVISIBLECOLUMNS + 34, VIC2_STARTVISIBLECOLUMNS + 34 + m_columns + 16 - 1);
			}
		}

		for (i = 0; i < 8; i++)
			m_sprites[i].repeat = m_sprites[i].line = 0;

		m_lastline = 0;

		if (LIGHTPEN_BUTTON)
		{
			/* lightpen timer start */
			machine().scheduler().timer_set(attotime(0, 0), timer_expired_delegate(FUNC(vic3_device::timer_timeout),this), 1);
		}

	}

	if (m_rasterline == C64_2_RASTERLINE(RASTERLINE))
	{
		set_interrupt(1);
	}

	if (m_on)
		if ((m_rasterline >= VIC2_FIRSTRASTERLINE) && (m_rasterline < (VIC2_FIRSTRASTERLINE + VIC2_VISIBLELINES)))
		{
			if (m_type == VIC4567_PAL)
			{
				if (m_on)
					vic2_drawlines(m_rasterline - 1, m_rasterline, VIC2_STARTVISIBLECOLUMNS + 32, VIC2_STARTVISIBLECOLUMNS + 32 + m_columns + 16 - 1);
			}
			else
			{
				if (m_on)
					vic2_drawlines(m_rasterline - 1, m_rasterline, VIC2_STARTVISIBLECOLUMNS + 34, VIC2_STARTVISIBLECOLUMNS + 34 + m_columns + 16 - 1);
			}
		}
}

UINT32 vic3_device::video_update( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	copybitmap(bitmap, *m_bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}


static MACHINE_CONFIG_FRAGMENT( vic3 )
	MCFG_PALETTE_ADD_INIT_BLACK("palette", 0x100)
MACHINE_CONFIG_END

//-------------------------------------------------
//  machine_config_additions - return a pointer to
//  the device's machine fragment
//-------------------------------------------------

machine_config_constructor vic3_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( vic3 );
}
