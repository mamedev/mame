// license:BSD-3-Clause
// copyright-holders:R. Belmont,Ryan Holtz
/***************************************************************************

    gba_lcd.c

    File to handle emulation of the video hardware of the Game Boy Advance

    By R. Belmont, Ryan Holtz

***************************************************************************/

#include "emu.h"
#include "gba_lcd.h"

#include "screen.h"


namespace {

// LCD I/O Registers
#define DISPCNT     HWLO(0x000)  // 0x4000000  2  R/W   LCD Control
#define GRNSWAP     HWHI(0x000)  // 0x4000002  2  R/W   Undocumented - Green Swap
#define DISPSTAT    HWLO(0x004)  // 0x4000004  2  R/W   General LCD Status (STAT,LYC)
#define VCOUNT      HWHI(0x004)  // 0x4000006  2  R     Vertical Counter (LY)
#define BG0CNT      HWLO(0x008)  // 0x4000008  2  R/W   BG0 Control
#define BG1CNT      HWHI(0x008)  // 0x400000a  2  R/W   BG1 Control
#define BG2CNT      HWLO(0x00c)  // 0x400000c  2  R/W   BG2 Control
#define BG3CNT      HWHI(0x00c)  // 0x400000e  2  R/W   BG3 Control
#define BG0HOFS     HWLO(0x010)  // 0x4000010  2  W     BG0 X-Offset
#define BG0VOFS     HWHI(0x010)  // 0x4000012  2  W     BG0 Y-Offset
#define BG1HOFS     HWLO(0x014)  // 0x4000014  2  W     BG1 X-Offset
#define BG1VOFS     HWHI(0x014)  // 0x4000016  2  W     BG1 Y-Offset
#define BG2HOFS     HWLO(0x018)  // 0x4000018  2  W     BG2 X-Offset
#define BG2VOFS     HWHI(0x018)  // 0x400001a  2  W     BG2 Y-Offset
#define BG3HOFS     HWLO(0x01c)  // 0x400001c  2  W     BG3 X-Offset
#define BG3VOFS     HWHI(0x01c)  // 0x400001e  2  W     BG3 Y-Offset
#define BG2PA       HWLO(0x020)  // 0x4000020  2  W     BG2 Rotation/Scaling Parameter A (dx)
#define BG2PB       HWHI(0x020)  // 0x4000022  2  W     BG2 Rotation/Scaling Parameter B (dmx)
#define BG2PC       HWLO(0x024)  // 0x4000024  2  W     BG2 Rotation/Scaling Parameter C (dy)
#define BG2PD       HWHI(0x024)  // 0x4000026  2  W     BG2 Rotation/Scaling Parameter D (dmy)
#define BG2X        WORD(0x028)  // 0x4000028  4  W     BG2 Reference Point X-Coordinate
#define BG2Y        WORD(0x02c)  // 0x400002c  4  W     BG2 Reference Point Y-Coordinate
#define BG3PA       HWLO(0x030)  // 0x4000030  2  W     BG3 Rotation/Scaling Parameter A (dx)
#define BG3PB       HWHI(0x030)  // 0x4000032  2  W     BG3 Rotation/Scaling Parameter B (dmx)
#define BG3PC       HWLO(0x034)  // 0x4000034  2  W     BG3 Rotation/Scaling Parameter C (dy)
#define BG3PD       HWHI(0x034)  // 0x4000036  2  W     BG3 Rotation/Scaling Parameter D (dmy)
#define BG3X        WORD(0x038)  // 0x4000038  4  W     BG3 Reference Point X-Coordinate
#define BG3Y        WORD(0x03c)  // 0x400003c  4  W     BG3 Reference Point Y-Coordinate
#define WIN0H       HWLO(0x040)  // 0x4000040  2  W     Window 0 Horizontal Dimensions
#define WIN1H       HWHI(0x040)  // 0x4000042  2  W     Window 1 Horizontal Dimensions
#define WIN0V       HWLO(0x044)  // 0x4000044  2  W     Window 0 Vertical Dimensions
#define WIN1V       HWHI(0x044)  // 0x4000046  2  W     Window 1 Vertical Dimensions
#define WININ       HWLO(0x048)  // 0x4000048  2  R/W   Inside of Window 0 and 1
#define WINOUT      HWHI(0x048)  // 0x400004a  2  R/W   Inside of OBJ Window & Outside of Windows
#define MOSAIC      HWLO(0x04c)  // 0x400004c  2  W     Mosaic Size
								 // 0x400004e  2  -     Unused
#define BLDCNT      HWLO(0x050)  // 0x4000050  2  R/W   Color Special Effects Selection
#define BLDALPHA    HWHI(0x050)  // 0x4000052  2  W     Alpha Blending Coefficients
#define BLDY        HWLO(0x054)  // 0x4000054  2  W     Brightness (Fade-In/Out) Coefficient
								 // 0x4000056  2  -     Unused

#define DISPSTAT_SET(val)       HWLO_SET(0x004, val)
#define DISPSTAT_RESET(val)     HWLO_RESET(0x004, val)

#define VERBOSE_LEVEL   (0)

inline void ATTR_PRINTF(3,4) verboselog(device_t &device, int n_level, const char *s_fmt, ...)
{
	if (VERBOSE_LEVEL >= n_level)
	{
		va_list v;
		char buf[32768];
		va_start(v, s_fmt);
		vsprintf(buf, s_fmt, v);
		va_end(v);
		device.logerror("%08x: %s", device.machine().describe_context(), buf);
	}
}

class object
{
public:
	object(device_t &device, uint16_t *oam, int index)
		: m_device(device)
	{
		m_attr0 = oam[(4 * index) + 0];
		m_attr1 = oam[(4 * index) + 1];
		m_attr2 = oam[(4 * index) + 2];
	}

	int    pos_y()       { return  m_attr0 & 0x00ff; }
	bool   roz()         { return  m_attr0 & 0x0100; }
	bool   roz_double()  { return  m_attr0 & 0x0200; }
	uint16_t mode_mask()   { return  m_attr0 & 0x0c00; }
	bool   mosaic()      { return  m_attr0 & 0x1000; }
	bool   palette_256() { return  m_attr0 & 0x2000; }

	int    pos_x()       { return  m_attr1 & 0x01ff; }
	int    roz_param()   { return (m_attr1 & 0x3e00) >> 9; }
	bool   hflip()       { return  m_attr1 & 0x1000; }
	bool   vflip()       { return  m_attr1 & 0x2000; }

	int    tile_number() { return  m_attr2 & 0x03ff; }
	int    priority()    { return (m_attr2 & 0x0c00) >> 10; }
	int    palette()     { return (m_attr2 & 0xf000) >> 8; }

	enum class mode : uint16_t
	{
		normal = 0x0000,
		alpha  = 0x0400,
		window = 0x0800
	};
	mode mode_enum() { return enum_value<mode>(m_attr0 & 0x0c00); }

	void size(int &width, int &height)
	{
		static int const size_table[4][4][2] =
		{
			{ { 8,   8 }, { 16, 16 }, { 32, 32 }, { 64, 64 } }, // square
			{ { 16,  8 }, { 32,  8 }, { 32, 16 }, { 64, 32 } }, // horizontal rect
			{ { 8,  16 }, { 8,  32 }, { 16, 32 }, { 32, 64 } }, // vertical rect
			{ { 0,   0 }, { 0,   0 }, { 0,   0 }, { 0,   0 } }  // invalid
		};

		int shape = (m_attr0 & 0xc000) >> 14;
		int size  = (m_attr1 & 0xc000) >> 14;

		width  = size_table[shape][size][0];
		height = size_table[shape][size][1];

		if (shape == 4)
			verboselog(m_device, 0, "WARNING: attempted to draw an object of invalid shape\n");
	}

private:
	device_t &m_device;

	uint16_t m_attr0;
	uint16_t m_attr1;
	uint16_t m_attr2;
};

} // anonymous namespace

DEFINE_DEVICE_TYPE(GBA_LCD, gba_lcd_device, "gba_lcd", "GBA LCD")

gba_lcd_device::gba_lcd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, GBA_LCD, tag, owner, clock)
	, device_video_interface(mconfig, *this)
	, m_int_hblank_cb(*this)
	, m_int_vblank_cb(*this)
	, m_int_vcount_cb(*this)
	, m_dma_hblank_cb(*this)
	, m_dma_vblank_cb(*this)
{
}

inline uint8_t gba_lcd_device::bg_video_mode()
{
	uint8_t mode = DISPCNT & 0x0007;

	if (mode > 5)
	{
		verboselog(*this, 0, "WARNING: attempted to set invalid BG video mode %d\n", mode);
		return 0;
	}

	return mode;
}

inline bool gba_lcd_device::is_set(dispcnt flag)
{
	return DISPCNT & underlying_value(flag);
}

inline void gba_lcd_device::set(dispstat flag)
{
	DISPSTAT_SET(underlying_value(flag));
}

inline void gba_lcd_device::clear(dispstat flag)
{
	DISPSTAT_RESET(underlying_value(flag));
}

inline bool gba_lcd_device::is_set(dispstat flag)
{
	return DISPSTAT & underlying_value(flag);
}

inline bool gba_lcd_device::is_set(uint16_t bgxcnt, bgcnt flag)
{
	return bgxcnt & underlying_value(flag);
}

inline uint8_t gba_lcd_device::bg_priority(uint16_t bgxcnt)
{
	return bgxcnt & 0x0003;
}

inline uint32_t gba_lcd_device::bg_char_base(uint16_t bgxcnt)
{
	return ((bgxcnt & 0x003c) >> 2) * 0x4000;
}

inline uint32_t gba_lcd_device::bg_screen_base(uint16_t bgxcnt)
{
	return ((bgxcnt & 0x1f00) >> 8) * 0x800;
}

inline void gba_lcd_device::bg_screen_size(uint16_t bgxcnt, bool text, int &width, int &height)
{
	static int const size_table[2][4][2] =
	{
		{ { 256, 256 }, { 512, 256 }, { 256, 512 }, { 512,   512 } }, // text mode
		{ { 128, 128 }, { 256, 256 }, { 512, 512 }, { 1024, 1024 } }  // rotation/scaling (roz) mode
	};

	int mode = text ? 0 : 1;
	int size = (bgxcnt & 0xc000) >> 14;

	width  = size_table[mode][size][0];
	height = size_table[mode][size][1];
}

inline uint16_t gba_lcd_device::mosaic_size(size_type type)
{
	return ((MOSAIC >> (4 * underlying_value(type))) & 0xf) + 1;
}

inline gba_lcd_device::sfx gba_lcd_device::color_sfx()
{
	return enum_value<sfx>(BLDCNT & 0x00c0);
}

inline uint8_t gba_lcd_device::color_sfx_target(target id)
{
	return (BLDCNT >> (8 * underlying_value(id))) & 0x3f;
}

inline void gba_lcd_device::update_mask(uint8_t* mask, int y)
{
	bool inwin0 = false;
	bool inwin1 = false;

	if (is_set(dispcnt::win0_en))
		inwin0 = is_in_window_v(y, 0);

	if (is_set(dispcnt::win1_en))
		inwin1 = is_in_window_v(y, 1);

	for (auto x = 0; x < 240; x++)
	{
		mask[x] = WINOUT & 0x00ff;

		if (m_scanline[5][x] != TRANSPARENT_PIXEL)
			mask[x] = WINOUT >> 8;

		if (inwin1 && is_in_window_h(x, 1))
			mask[x] = WININ >> 8;

		if (inwin0 && is_in_window_h(x, 0))
			mask[x] = WININ & 0x00ff;
	}
}

void gba_lcd_device::draw_scanline(int y)
{
	uint16_t *scanline = &m_bitmap.pix16(y);

	if (is_set(dispcnt::forced_blank))
	{
		// forced blank is white
		for (auto x = 0; x < 240; x++)
			scanline[x] = 0x7fff;

		return;
	}

	uint8_t mode = bg_video_mode();

	uint8_t submode;
	if (is_set(dispcnt::win0_en) || is_set(dispcnt::win1_en) || is_set(dispcnt::obj_win_en))
		submode = 2;
	else if (color_sfx() != sfx::none)
		submode = 1;
	else
		submode = 0;

	int depth = 0;
	if (mode == 3)
		depth = 16;
	else if (mode == 4)
		depth = 8;
	else if (mode == 5)
		depth = 4;

	// make all layers transparent at start
	for (auto l = 0; l < 6; l++)
	{
		for (auto x = 0; x < 240; x++)
		{
			m_scanline[l][x] = TRANSPARENT_PIXEL;
		}
	}

	// draw background
	switch (mode)
	{
	case 0:
		draw_bg_scanline(m_scanline[0], y, dispcnt::bg0_en, BG0CNT, BG0HOFS, BG0VOFS);
		draw_bg_scanline(m_scanline[1], y, dispcnt::bg1_en, BG1CNT, BG1HOFS, BG1VOFS);
		draw_bg_scanline(m_scanline[2], y, dispcnt::bg2_en, BG2CNT, BG2HOFS, BG2VOFS);
		draw_bg_scanline(m_scanline[3], y, dispcnt::bg3_en, BG3CNT, BG3HOFS, BG3VOFS);
		break;
	case 1:
		draw_bg_scanline(m_scanline[0], y, dispcnt::bg0_en, BG0CNT, BG0HOFS, BG0VOFS);
		draw_bg_scanline(m_scanline[1], y, dispcnt::bg1_en, BG1CNT, BG1HOFS, BG1VOFS);
		draw_roz_scanline(m_scanline[2], y, dispcnt::bg2_en, BG2CNT, BG2X, BG2Y, BG2PA, BG2PB, BG2PC, BG2PD, m_bg2x, m_bg2y);
		break;
	case 2:
		draw_roz_scanline(m_scanline[2], y, dispcnt::bg2_en, BG2CNT, BG2X, BG2Y, BG2PA, BG2PB, BG2PC, BG2PD, m_bg2x, m_bg2y);
		draw_roz_scanline(m_scanline[3], y, dispcnt::bg3_en, BG3CNT, BG3X, BG3Y, BG3PA, BG3PB, BG3PC, BG3PD, m_bg3x, m_bg3y);
		break;
	case 3:
	case 4:
	case 5:
		draw_roz_bitmap_scanline(m_scanline[2], y, dispcnt::bg2_en, BG2CNT, BG2X, BG2Y, BG2PA, BG2PB, BG2PC, BG2PD, m_bg2x, m_bg2y, depth);
		break;
	}

	uint8_t mask[240];

	// draw objects
	draw_oam(m_scanline[4], y);

	if (submode == 2)
	{
		draw_oam_window(m_scanline[5], y);
		update_mask(mask, y);
	}
	else
	{
		memset(mask, 0xff, sizeof(mask));
	}

	uint32_t backdrop = ((uint16_t *)m_pram.get())[0] | 0x30000000;

	for (auto x = 0; x < 240; x++)
	{
		uint32_t color = backdrop;
		uint8_t top = 0x20;

		for (auto l = 0; l < 5; l++)
		{
			if ((m_scanline[l][x] >> 24) < (color >> 24) && mask[x] & (0x01 << l))
			{
				color = m_scanline[l][x];
				top = (0x01 << l);
			}
		}

		if (color & 0x00010000)
		{
			if (submode != 0 || top == 0x10)
			{
				uint32_t back = backdrop;
				uint8_t top2 = 0x20;

				for (auto l = 0; l < 4; l++)
				{
					if ((m_scanline[l][x] >> 24) < (back >> 24) && mask[x] & (0x01 << l))
					{
						back = m_scanline[l][x];
						top2 = (0x01 << l);
					}
				}

				if (top2 & color_sfx_target(target::second))
				{
					color = alpha_blend(color, back);
				}
				else if (top & color_sfx_target(target::first))
				{
					switch (color_sfx())
					{
					case sfx::lighten:
						color = increase_brightness(color);
						break;
					case sfx::darken:
						color = decrease_brightness(color);
						break;
					default:
						break;
					}
				}
			}
		}
		else if (submode == 1 || (submode == 2 && mask[x] & 0x20))
		{
			if (top & color_sfx_target(target::first))
			{
				switch (color_sfx())
				{
				case sfx::none:
					break;
				case sfx::alpha:
				{
					uint32_t back = backdrop;
					uint8_t top2 = 0x20;

					for (auto l = 0; l < 5; l++)
					{
						if ((m_scanline[l][x] >> 24) < (back >> 24) && mask[x] & (0x01 << l))
						{
							if (top != (0x01 << l))
							{
								back = m_scanline[l][x];
								top2 = (0x01 << l);
							}
						}
					}

					if (top2 & color_sfx_target(target::second))
						color = alpha_blend(color, back);

					break;
				}
				case sfx::lighten:
					color = increase_brightness(color);
					break;
				case sfx::darken:
					color = decrease_brightness(color);
					break;
				}
			}
		}
		scanline[x] = color & 0x7fff;
	}
}

void gba_lcd_device::draw_roz_bitmap_scanline(uint32_t *scanline, int ypos, dispcnt bg_enable, uint32_t ctrl, int32_t X, int32_t Y, int32_t PA, int32_t PB, int32_t PC, int32_t PD, internal_reg &currentx, internal_reg &currenty, int depth)
{
	if (!is_set(bg_enable))
		return;

	uint8_t *src8 = (uint8_t *)m_vram.get();
	uint16_t *src16 = (uint16_t *)m_vram.get();
	uint16_t *palette = (uint16_t *)m_pram.get();
	int32_t sx = (depth == 4) ? 160 : 240;
	int32_t sy = (depth == 4) ? 128 : 160;
	uint32_t prio = (bg_priority(ctrl) << 25) + 0x1000000;

	if (is_set(dispcnt::alt_frame_sel))
	{
		if (depth == 8)
			src8 += 0xa000;

		if (depth == 4)
			src16 += 0xa000 / 2;
	}

	// sign extend roz parameters
	if (X & 0x08000000) X |= 0xf0000000;
	if (Y & 0x08000000) Y |= 0xf0000000;
	if (PA & 0x8000) PA |= 0xffff0000;
	if (PB & 0x8000) PB |= 0xffff0000;
	if (PC & 0x8000) PC |= 0xffff0000;
	if (PD & 0x8000) PD |= 0xffff0000;

	if (currentx.update)
	{
		currentx.status = X;
		currentx.update = false;
	}
	else
	{
		currentx.status += PB;
	}

	if (currenty.update)
	{
		currenty.status = Y;
		currenty.update = false;
	}
	else
	{
		currenty.status += PD;
	}

	int32_t cx = currentx.status;
	int32_t cy = currenty.status;

	if (is_set(ctrl, bgcnt::mosaic_en))
	{
		uint16_t mosaic_line = mosaic_size(size_type::bg_v);
		int32_t tempy = (ypos / mosaic_line) * mosaic_line;
		cx = X + tempy * PB;
		cy = Y + tempy * PD;
	}

	int32_t pixx = cx >> 8;
	int32_t pixy = cy >> 8;

	for (auto x = 0; x < 240; x++)
	{
		if (pixx >= 0 && pixy >= 0 && pixx < sx && pixy < sy)
		{
			if (depth == 8)
			{
				uint8_t color = src8[pixy * sx + pixx];
				if (color)
					scanline[x] = palette[color] | prio;
			}
			else
			{
				scanline[x] = src16[pixy * sx + pixx] | prio;
			}
		}

		cx += PA;
		cy += PC;

		pixx = cx >> 8;
		pixy = cy >> 8;
	}

	if (is_set(ctrl, bgcnt::mosaic_en))
	{
		uint16_t mosaicx = mosaic_size(size_type::bg_h);
		if (mosaicx > 1)
		{
			int32_t m = 1;
			for (auto x = 0; x < 239; x++)
			{
				scanline[x + 1] = scanline[x];
				m++;
				if (m == mosaicx)
				{
					m = 1;
					x++;
				}
			}
		}
	}
}

void gba_lcd_device::draw_roz_scanline(uint32_t *scanline, int ypos, dispcnt bg_enable, uint32_t ctrl, int32_t X, int32_t Y, int32_t PA, int32_t PB, int32_t PC, int32_t PD, internal_reg &currentx, internal_reg &currenty)
{
	if (!is_set(bg_enable))
		return;

	uint8_t *mgba_vram = (uint8_t *)m_vram.get();
	uint16_t *pgba_pram = (uint16_t *)m_pram.get();
	uint32_t priority = (bg_priority(ctrl) << 25) + 0x1000000;
	uint32_t base = bg_char_base(ctrl);
	uint32_t mapbase = bg_screen_base(ctrl);

	// size of map in submaps
	int width, height;
	bg_screen_size(ctrl, false, width, height);

	// sign extend roz parameters
	if (X & 0x08000000) X |= 0xf0000000;
	if (Y & 0x08000000) Y |= 0xf0000000;
	if (PA & 0x8000) PA |= 0xffff0000;
	if (PB & 0x8000) PB |= 0xffff0000;
	if (PC & 0x8000) PC |= 0xffff0000;
	if (PD & 0x8000) PD |= 0xffff0000;

	if (currentx.update)
	{
		currentx.status = X;
		currentx.update = false;
	}
	else
	{
		currentx.status += PB;
	}

	if (currenty.update)
	{
		currenty.status = Y;
		currenty.update = false;
	}
	else
	{
		currenty.status += PD;
	}

	int32_t cx = currentx.status;
	int32_t cy = currenty.status;

	if (is_set(ctrl, bgcnt::mosaic_en))
	{
		uint16_t mosaic_line = mosaic_size(size_type::bg_v);
		int y = ypos % mosaic_line;
		cx -= y * PB;
		cy -= y * PD;
	}

	int32_t pixx = cx >> 8;
	int32_t pixy = cy >> 8;

	if (is_set(ctrl, bgcnt::wraparound_en))
	{
		pixx %= width;
		pixy %= height;

		if (pixx < 0)
			pixx += width;

		if (pixy < 0)
			pixy += height;
	}

	for (auto x = 0; x < 240; x++)
	{
		if (pixx >= 0 && pixy >= 0 && pixx < width && pixy < height)
		{
			int tilex = pixx & 7;
			int tiley = pixy & 7;

			// shall we shift for is_set(ctrl, bgcnt::palette_256)? or is not effective for ROZ?
			uint32_t tile = mgba_vram[mapbase + (pixx >> 3) + (pixy >> 3) * (width >> 3)];
			uint16_t pixel = mgba_vram[base + (tile << 6) + (tiley << 3) + tilex];

			// plot it
			if (pixel)
				scanline[x] = pgba_pram[pixel] | priority;
		}

		cx += PA;
		cy += PC;

		pixx = cx >> 8;
		pixy = cy >> 8;

		if (is_set(ctrl, bgcnt::wraparound_en))
		{
			pixx %= width;
			pixy %= height;

			if (pixx < 0)
				pixx += width;

			if (pixy < 0)
				pixy += height;
		}
	}

	if (is_set(ctrl, bgcnt::mosaic_en))
	{
		uint16_t mosaicx = mosaic_size(size_type::bg_h);
		if (mosaicx > 1)
		{
			int m = 1;
			for (auto x = 0; x < 239; x++)
			{
				scanline[x + 1] = scanline[x];
				m++;
				if (m == mosaicx)
				{
					m = 1;
					x++;
				}
			}
		}
	}
}

void gba_lcd_device::draw_bg_scanline(uint32_t *scanline, int ypos, dispcnt bg_enable, uint32_t ctrl, uint32_t hofs, uint32_t vofs)
{
	if (!is_set(bg_enable))
		return;

	uint8_t *vram = (uint8_t*)m_vram.get();
	uint16_t *palette = (uint16_t *)m_pram.get();
	uint8_t *chardata = &vram[bg_char_base(ctrl)];
	uint16_t *screendata = (uint16_t *)&vram[bg_screen_base(ctrl)];
	uint32_t priority = (bg_priority(ctrl) << 25) + 0x1000000;
	uint16_t mosaicx = mosaic_size(size_type::bg_h);
	uint16_t mosaicy = mosaic_size(size_type::bg_v);

	int width, height;
	bg_screen_size(ctrl, true, width, height);

	int32_t pixx = hofs % width;
	int32_t pixy = (vofs + ypos) % height;

	if (is_set(ctrl, bgcnt::mosaic_en) && ypos % mosaicy)
	{
		mosaicy = (ypos / mosaicy) * mosaicy;
		pixy = (vofs + mosaicy) % height;
	}

	if (pixy > 255 && height > 256)
	{
		pixy &= 0x000000ff;
		screendata += 0x400;
		if (width > 256)
		{
			screendata += 0x400;
		}
	}

	int32_t stride = (pixy >> 3) << 5;

	uint16_t *src = screendata + 0x400 * (pixx >> 8) + ((pixx & 255) >> 3) + stride;

	for (auto x = 0; x < 240; x++)
	{
		uint16_t data = *src;
		int32_t tile = tile_number(data);
		int32_t tilex = pixx & 7;
		int32_t tiley = pixy & 7;
		uint8_t color;
		uint8_t palindex;

		if (tile_hflip(data))
			tilex = 7 - tilex;

		if (tile_vflip(data))
			tiley = 7 - tiley;

		if (is_set(ctrl, bgcnt::palette_256))
		{
			color = chardata[(tile << 6) + (tiley << 3) + tilex];
			palindex = 0;
		}
		else
		{
			color = chardata[(tile << 5) + (tiley << 2) + (tilex >> 1)];

			if (tilex & 1)
				color >>= 4;
			else
				color &= 0x0f;
			palindex = (data >> 8) & 0x00f0;
		}

		if (color)
			scanline[x] = palette[palindex + color] | priority;

		if (tile_hflip(data))
		{
			if (tilex == 0)
				src++;
		}
		else if (tilex == 7)
		{
			src++;
		}

		pixx++;
		if (pixx == 256)
		{
			if (width > 256)
			{
				src = screendata + 0x400 + stride;
			}
			else
			{
				src = screendata + stride;
				pixx = 0;
			}
		}
		else if (pixx >= width)
		{
			pixx = 0;
			src = screendata + stride;
		}
	}

	if (is_set(ctrl, bgcnt::mosaic_en) && mosaicx > 1)
	{
		int32_t m = 1;
		for (auto x = 0; x < 239; x++)
		{
			scanline[x+1] = scanline[x];
			m++;
			if (m == mosaicx)
			{
				m = 1;
				x++;
			}
		}
	}
}

void gba_lcd_device::draw_oam_window(uint32_t *scanline, int y)
{
	if (!is_set(dispcnt::obj_win_en))
		return;

	uint16_t *oam = (uint16_t *)m_oam.get();
	uint8_t *src = (uint8_t *)m_vram.get();

	for (auto obj_index = 127; obj_index >= 0; obj_index--)
	{
		object obj(*this, oam, obj_index);

		if (obj.mode_enum() != object::mode::window)
			continue;

		uint32_t tile_number = obj.tile_number();

		if (bg_video_mode() > 2 && tile_number < 0x200)
			continue;

		int32_t sx = obj.pos_x();
		int32_t sy = obj.pos_y();

		if (sy > 160)
			sy -= 256;

		int width, height;
		obj.size(width, height);

		if (obj.roz())
		{
			int32_t fx = width;
			int32_t fy = height;

			if (obj.roz_double())
			{
				fx *= 2;
				fy *= 2;
			}

			int32_t cury = y - sy;

			if (cury < 0 || cury >= fy)
				continue;

			if (sx >= 240 && ((sx + fx) % 512) >= 240)
				continue;

			int rot = obj.roz_param();
			int16_t dx  = (int16_t)oam[(rot << 4) + 3];
			int16_t dmx = (int16_t)oam[(rot << 4) + 7];
			int16_t dy  = (int16_t)oam[(rot << 4) + 11];
			int16_t dmy = (int16_t)oam[(rot << 4) + 15];

			int32_t rx = (width << 7) - (fx >> 1) * dx - (fy >> 1) * dmx + cury * dmx;
			int32_t ry = (height << 7) - (fx >> 1) * dy - (fy >> 1) * dmy + cury * dmy;

			int inc = 32;

			if (obj.palette_256())
			{
				if (is_set(dispcnt::vram_map_1d))
					inc = sx >> 2;
				else
					tile_number &= 0x3fe;

				for (auto x = 0; x < fx; x++)
				{
					int32_t ax = rx >> 8;
					int32_t ay = ry >> 8;

					if (ax >= 0 && ax < sx && ay >= 0 && ay < sy)
					{
						uint8_t color = src[0x10000 + ((((tile_number + (ay >> 3) * inc) << 5) + ((ay & 0x07) << 3) + ((ax >> 3) << 6) + (ax & 0x07)) & 0x7fff)];

						if (color)
							scanline[sx] = 1;
					}

					sx = (sx + 1) % 512;
					rx += dx;
					ry += dy;
				}
			}
			else
			{
				if (is_set(dispcnt::vram_map_1d))
					inc = sx >> 3;

				for (auto x = 0; x < fx; x++)
				{
					int32_t ax = rx >> 8;
					int32_t ay = ry >> 8;

					if (ax >= 0 && ax < sx && ay >= 0 && ay < sy)
					{
						uint8_t color = src[0x10000 + ((((tile_number + (ay >> 3) * inc) << 5) + ((ay & 0x07) << 2) + ((ax >> 3) << 5) + ((ax & 0x07) >> 1)) & 0x7fff)];

						if (ax & 1)
							color >>= 4;
						else
							color &= 0x0f;

						if (color)
							scanline[sx] = 1;
					}

					sx = (sx + 1) % 512;
					rx += dx;
					ry += dy;
				}
			}
		}
		else
		{
			// when roz bit is not set double roz bit means 'disable object'
			if (obj.roz_double())
				continue;

			int32_t cury = y - sy;

			if (cury < 0 || cury >= height)
				continue;

			if ((sx >= 240) && (((sx + width) % 512) >= 240))
				continue;

			int inc = 32;

			if (obj.vflip())
				cury = height - cury - 1;

			int32_t ax = obj.hflip() ? (width - 1) : 0;

			if (obj.palette_256())
			{
				if (is_set(dispcnt::vram_map_1d))
					inc = width >> 2;
				else
					tile_number &= 0x3fe;

				uint32_t address = 0x10000 + ((((tile_number + (cury >> 3) * inc) << 5) + ((cury & 7) << 3) + ((ax >> 3) << 6) + (ax & 7)) & 0x7fff);

				if (obj.hflip())
					ax = 7;

				for (auto x = 0; x < width; x++)
				{
					if (sx < 240)
					{
						uint8_t color = src[address];

						if (color)
							scanline[sx] = 1;
					}

					sx = (sx + 1) % 512;

					if (obj.hflip())
					{
						ax--;
						address--;

						if (ax == -1)
						{
							address -= 56;
							ax = 7;
						}

						if (address < 0x10000)
							address += 0x8000;
					}
					else
					{
						ax++;
						address++;

						if (ax == 8)
						{
							address += 56;
							ax = 0;
						}

						if (address > 0x17fff)
							address -= 0x8000;
					}
				}
			}
			else
			{
				if (is_set(dispcnt::vram_map_1d))
					inc = width >> 3;

				uint32_t address = 0x10000 + ((((tile_number + (cury >> 3) * inc) << 5) + ((cury & 0x07) << 2) + ((ax >> 3) << 5) + ((ax & 0x07) >> 1)) & 0x7fff);

				if (obj.hflip())
				{
					ax = 7;

					for (auto x = width - 1; x >= 0; x--)
					{
						if (sx < 240)
						{
							uint8_t color = src[address];
							if (x & 1)
								color >>= 4;
							else
								color &= 0x0f;

							if (color)
								scanline[sx] = 1;
						}

						sx = (sx + 1) % 512;
						ax--;

						if ((x & 1) == 0)
							address--;

						if (ax == -1)
						{
							ax = 7;
							address -= 28;
						}

						if (address < 0x10000)
							address += 0x8000;
					}
				}
				else
				{
					for (auto x = 0; x < width; x++)
					{
						if (sx < 240)
						{
							uint8_t color = src[address];

							if (x & 1)
								color >>= 4;
							else
								color &= 0x0f;

							if (color)
								scanline[sx] = 1;
						}

						sx = (sx + 1) % 512;
						ax++;

						if (x & 1)
							address++;

						if (ax == 8)
						{
							address += 28;
							ax = 0;
						}

						if (address > 0x17fff)
							address -= 0x8000;
					}
				}
			}
		}
	}
}

void gba_lcd_device::draw_oam(uint32_t *scanline, int y)
{
	if (!is_set(dispcnt::obj_en))
		return;

	int32_t mosaiccnt = 0;
	uint16_t mosaicx = mosaic_size(size_type::obj_h);
	uint16_t mosaicy = mosaic_size(size_type::obj_v);
	uint16_t *oam = (uint16_t *)m_oam.get();
	uint8_t *src = (uint8_t *)m_vram.get();
	uint16_t *palette = (uint16_t *)m_pram.get();

	for (auto obj_index = 0; obj_index < 128; obj_index++)
	{
		object obj(*this, oam, obj_index);

		if (obj.mode_enum() == object::mode::window)
			continue;

		uint32_t priority = obj.priority();
		uint32_t prio = (priority << 25) | (obj.mode_mask() << 6);

		int width, height;
		obj.size(width, height);

		uint32_t tile_number = obj.tile_number();

		if (bg_video_mode() > 2 && tile_number < 0x200)
			continue;

		if (obj.roz())
		{
			int32_t sx = obj.pos_x();
			int32_t sy = obj.pos_y();

			if (sy > 160)
				sy -= 256;

			int32_t fx = width;
			int32_t fy = height;

			if (obj.roz_double())
			{
				fx *= 2;
				fy *= 2;
			}

			int32_t cury = y - sy;

			if (cury < 0 || cury >= fy)
				continue;

			if (sx >= 240 && ((sx + fx) % 512) >= 240)
				continue;

			int32_t oamparam = obj.roz_param();

			int16_t dx  = (int16_t)oam[(oamparam << 4) + 3];
			int16_t dmx = (int16_t)oam[(oamparam << 4) + 7];
			int16_t dy  = (int16_t)oam[(oamparam << 4) + 11];
			int16_t dmy = (int16_t)oam[(oamparam << 4) + 15];

			if (obj.mosaic())
				cury -= (cury % mosaicy);

			int32_t rx = (width << 7) - (fx >> 1) * dx - (fy >> 1) * dmx + cury * dmx;
			int32_t ry = (height << 7) - (fx >> 1) * dy - (fy >> 1) * dmy + cury * dmy;

			int32_t inc = 32;

			if (obj.palette_256())
			{
				if (is_set(dispcnt::vram_map_1d))
					inc = width >> 2;
				else
					tile_number &= 0x3fe;

				for (auto x = 0; x < fx; x++)
				{
					int32_t pixx = rx >> 8;
					int32_t pixy = ry >> 8;

					if (!(pixx < 0 || pixx >= width || pixy < 0 || pixy >= height || sx >= 240))
					{
						uint8_t color = src[0x10000 + ((((tile_number + (pixy >> 3) * inc) << 5) + ((pixy & 7) << 3) + ((pixx >> 3) << 6) + (pixx & 7)) & 0x7fff)];

						if (color == 0 && priority < ((scanline[sx] >> 25) & 3))
						{
							scanline[sx] = (scanline[sx] & 0xf9ffffff) | prio;
							if (obj.mosaic() && mosaiccnt != 0)
							{
								scanline[sx] = (scanline[sx - 1] & 0xf9ffffff) | prio;
							}
						}
						else if (color != 0 && prio < (scanline[sx] & 0xff000000))
						{
							scanline[sx] = palette[256 + color] | prio;
							if (obj.mosaic() && mosaiccnt != 0)
							{
								scanline[sx] = (scanline[sx - 1] & 0xf9ffffff) | prio;
							}
						}

						if (obj.mosaic())
							mosaiccnt = (mosaiccnt + 1) % mosaicx;
					}

					sx = (sx + 1) % 512;
					rx += dx;
					ry += dy;
				}
			}
			else
			{
				if (is_set(dispcnt::vram_map_1d))
					inc = width >> 3;

				for (auto x = 0; x < fx; x++)
				{
					int32_t pixx = rx >> 8;
					int32_t pixy = ry >> 8;

					if (!(pixx < 0 || pixx >= width || pixy < 0 || pixy >= height || sx >= 240))
					{
						uint8_t color = src[0x10000 + ((((tile_number + (pixy >> 3) * inc) << 5) + ((pixy & 7) << 2) + ((pixx >> 3) << 5) + ((pixx & 7) >> 1)) & 0x7fff)];

						if (pixx & 1)
							color >>= 4;
						else
							color &= 0x0f;

						if (color == 0 && priority < ((scanline[sx] >> 25) & 3))
						{
							scanline[sx] = (scanline[sx] & 0xf9ffffff) | prio;
							if (obj.mosaic() && mosaiccnt != 0)
							{
								scanline[sx] = (scanline[sx - 1] & 0xf9ffffff) | prio;
							}
						}
						else if (color != 0 && prio < (scanline[sx] & 0xff000000))
						{
							scanline[sx] = palette[256 + obj.palette() + color] | prio;
							if (obj.mosaic() && mosaiccnt != 0)
							{
								scanline[sx] = (scanline[sx - 1] & 0xf9ffffff) | prio;
							}
						}
					}

					if (obj.mosaic())
						mosaiccnt = (mosaiccnt + 1) % mosaicx;

					sx = (sx + 1) % 512;
					rx += dx;
					ry += dy;
				}
			}
		}
		else
		{
			// when roz bit is not set double roz bit means 'disable object'
			if (obj.roz_double())
				continue;

			int32_t sx = obj.pos_x();
			int32_t sy = obj.pos_y();

			if (sy > 160)
				sy -= 256;

			int32_t cury = y - sy;

			if (cury < 0 || cury >= height)
				continue;

			if (sx >= 240 && ((sx + width) % 512) >= 240)
				continue;

			int32_t inc = 32;

			if (obj.vflip())
				cury = height - cury - 1;

			int32_t pixx = obj.hflip() ? (width - 1) : 0;

			if (obj.mosaic())
				cury -= (cury % mosaicy);

			if (obj.palette_256())
			{
				if (is_set(dispcnt::vram_map_1d))
					inc = width >> 2;
				else
					tile_number &= 0x3fe;

				uint32_t address = 0x10000 + ((((tile_number + (cury >> 3) * inc) << 5) + ((cury & 7) << 3) + ((pixx >> 3) << 6) + (pixx & 7)) & 0x7fff);

				if (obj.hflip())
					pixx = 7;

				for (auto x = 0; x < width; x++)
				{
					if (sx < 240)
					{
						uint8_t color = src[address];

						if (color == 0 && priority < ((scanline[sx] >> 25) & 3))
						{
							scanline[sx] = (scanline[sx] & 0xf9ffffff) | prio;
							if (obj.mosaic() && mosaiccnt != 0)
							{
								scanline[sx] = (scanline[sx - 1] & 0xf9ffffff) | prio;
							}
						}
						else if (color != 0 && prio < (scanline[sx] & 0xff000000))
						{
							scanline[sx] = palette[256 + color] | prio;
							if (obj.mosaic() && mosaiccnt != 0)
							{
								scanline[sx] = (scanline[sx - 1] & 0xf9ffffff) | prio;
							}
						}
					}

					if (obj.mosaic())
						mosaiccnt = (mosaiccnt + 1) % mosaicx;

					sx = (sx + 1) % 512;

					if (obj.hflip())
					{
						pixx--;
						address--;

						if (pixx == -1)
						{
							address -= 56;
							pixx = 7;
						}

						if (address < 0x10000)
							address += 0x8000;
					}
					else
					{
						pixx++;
						address++;

						if (pixx == 8)
						{
							address += 56;
							pixx = 0;
						}

						if (address > 0x17fff)
							address -= 0x8000;
					}
				}
			}
			else
			{
				if (is_set(dispcnt::vram_map_1d))
					inc = width >> 3;

				uint32_t address = 0x10000 + ((((tile_number + (cury >> 3) * inc) << 5) + ((cury & 7) << 2) + ((pixx >> 3) << 5) + ((pixx & 7) >> 1)) & 0x7fff);

				if (obj.hflip())
				{
					pixx = 7;
					for (auto x = width - 1; x >= 0; x--)
					{
						if (sx < 240)
						{
							uint8_t color = src[address];

							if (x & 1)
								color >>= 4;
							else
								color &= 0x0f;

							if (color == 0 && priority < ((scanline[sx] >> 25) & 3))
							{
								scanline[sx] = (scanline[sx] & 0xf9ffffff) | prio;
								if (obj.mosaic() && mosaiccnt != 0)
								{
									scanline[sx] = (scanline[sx - 1] & 0xf9ffffff) | prio;
								}
							}
							else if (color != 0 && prio < (scanline[sx] & 0xff000000))
							{
								scanline[sx] = palette[256 + obj.palette() + color] | prio;
								if (obj.mosaic() && mosaiccnt != 0)
								{
									scanline[sx] = (scanline[sx - 1] & 0xf9ffffff) | prio;
								}
							}
						}

						sx = (sx + 1) % 512;
						pixx--;

						if (!(x & 1))
							address--;

						if (pixx == -1)
						{
							address -= 28;
							pixx = 7;
						}

						if (address < 0x10000)
							address += 0x8000;
					}
				}
				else
				{
					for (auto x = 0; x < width; x++)
					{
						if (sx < 240)
						{
							uint8_t color = src[address];

							if (x & 1)
								color >>= 4;
							else
								color &= 0x0f;

							if (color == 0 && priority < ((scanline[sx] >> 25) & 3))
							{
								scanline[sx] = (scanline[sx] & 0xf9ffffff) | prio;
								if (obj.mosaic() && mosaiccnt != 0)
								{
									scanline[sx] = (scanline[sx - 1] & 0xf9ffffff) | prio;
								}
							}
							else if (color != 0 && prio < (scanline[sx] & 0xff000000))
							{
								scanline[sx] = palette[256 + obj.palette() + color] | prio;
								if (obj.mosaic() && mosaiccnt != 0)
								{
									scanline[sx] = (scanline[sx - 1] & 0xf9ffffff) | prio;
								}
							}
						}

						if (obj.mosaic())
							mosaiccnt = (mosaiccnt + 1) % mosaicx;

						sx = (sx + 1) % 512;
						pixx++;

						if (x & 1)
							address++;

						if (pixx == 8)
						{
							address += 28;
							pixx = 0;
						}

						if (address > 0x17fff)
							address -= 0x8000;
					}
				}
			}
		}
	}
}

inline bool gba_lcd_device::is_in_window_h(int x, int window)
{
	uint16_t reg = (window == 0) ? WIN0H : WIN1H;

	uint8_t x0 = reg >> 8;
	uint8_t x1 = reg & 0x00ff;

	if (x0 <= x1)
	{
		if (x >= x0 && x < x1)
			return true;
	}
	else
	{
		if (x >= x0 || x < x1)
			return true;
	}

	return false;
}

inline bool gba_lcd_device::is_in_window_v(int y, int window)
{
	uint16_t reg = (window == 0) ? WIN0V : WIN1V;

	uint8_t v0 = reg >> 8;
	uint8_t v1 = reg & 0x00ff;

	if ((v0 == v1) && (v0 >= 0xe8))
		return true;

	if (v1 >= v0)
	{
		if (y >= v0 && y < v1)
			return true;
	}
	else
	{
		if (y >= v0 || y < v1)
			return true;
	}

	return false;
}

static int const coeff[32] = {
	0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16
};

inline uint32_t gba_lcd_device::alpha_blend(uint32_t color0, uint32_t color1)
{
	int ca = coeff[BLDALPHA & 0x1f];
	int cb = coeff[(BLDALPHA >> 8) & 0x1f];

	if (color0 != TRANSPARENT_PIXEL)
	{
		int r0 = (color0 >>  0) & 0x1f;
		int g0 = (color0 >>  5) & 0x1f;
		int b0 = (color0 >> 10) & 0x1f;
		int r1 = (color1 >>  0) & 0x1f;
		int g1 = (color1 >>  5) & 0x1f;
		int b1 = (color1 >> 10) & 0x1f;
		int r = ((r0 * ca) >> 4) + ((r1 * cb) >> 4);
		int g = ((g0 * ca) >> 4) + ((g1 * cb) >> 4);
		int b = ((b0 * ca) >> 4) + ((b1 * cb) >> 4);

		if (r > 0x1f) r = 0x1f;
		if (g > 0x1f) g = 0x1f;
		if (b > 0x1f) b = 0x1f;

		return (color0 & 0xffff0000) | (b << 10) | (g << 5) | r;
	}

	return color0;
}

inline uint32_t gba_lcd_device::increase_brightness(uint32_t color)
{
	int cc = coeff[BLDY & 0x1f];

	int r = (color >>  0) & 0x1f;
	int g = (color >>  5) & 0x1f;
	int b = (color >> 10) & 0x1f;

	r += ((0x1f - r) * cc) >> 4;
	g += ((0x1f - g) * cc) >> 4;
	b += ((0x1f - b) * cc) >> 4;

	if (r > 0x1f) r = 0x1f;
	if (g > 0x1f) g = 0x1f;
	if (b > 0x1f) b = 0x1f;

	return (color & 0xffff0000) | (b << 10) | (g << 5) | r;
}

inline uint32_t gba_lcd_device::decrease_brightness(uint32_t color)
{
	int cc = coeff[BLDY & 0x1f];

	int r = (color >>  0) & 0x1f;
	int g = (color >>  5) & 0x1f;
	int b = (color >> 10) & 0x1f;

	r -= (r * cc) >> 4;
	g -= (g * cc) >> 4;
	b -= (b * cc) >> 4;

	if (r < 0) r = 0;
	if (g < 0) g = 0;
	if (b < 0) b = 0;

	return (color & 0xffff0000) | (b << 10) | (g << 5) | r;
}

static char const *const reg_names[] = {
	/* LCD I/O Registers */
	"DISPCNT", "GRNSWAP",  "DISPSTAT", "VCOUNT",
	"BG0CNT",  "BG1CNT",   "BG2CNT",   "BG3CNT",
	"BG0HOFS", "BG0VOFS",  "BG1HOFS",  "BG1VOFS",
	"BG2HOFS", "BG2VOFS",  "BG3HOFS",  "BG3VOFS",
	"BG2PA",   "BG2PB",    "BG2PC",    "BG2PD",
	"BG2X_L",  "BG2X_H",   "BG2Y_L",   "BG2Y_H",
	"BG3PA",   "BG3PB",    "BG3PC",    "BG3PD",
	"BG3X_L",  "BG3X_H",   "BG3Y_L",   "BG3Y_H",
	"WIN0H",   "WIN1H",    "WIN0V",    "WIN1V",
	"WININ",   "WINOUT",   "MOSAIC",   "Unused",
	"BLDCNT",  "BLDALPHA", "BLDY",     "Unused",
	"Unused",  "Unused",   "Unused",   "Unused",
};

READ32_MEMBER(gba_lcd_device::video_r)
{
	uint32_t retval = 0;

	switch (offset)
	{
	case 0x0004/4:
		retval = DISPSTAT | (screen().vpos() << 16);
		break;
	default:
		if (ACCESSING_BITS_0_15)
		{
			retval |= m_regs[offset] & 0x0000ffff;
		}
		if (ACCESSING_BITS_16_31)
		{
			retval |= m_regs[offset] & 0xffff0000;
		}
		break;
	}

	if (offset >= ARRAY_LENGTH(reg_names) / 2)
		throw emu_fatalerror("gba_lcd_device::video_r: Not enough register names in gba_lcd_device");

	if (ACCESSING_BITS_0_15)
		verboselog(*this, 2, "GBA I/O Read: %s = %04x\n", reg_names[offset * 2], retval & 0x0000ffff);

	if (ACCESSING_BITS_16_31)
		verboselog(*this, 2, "GBA I/O Read: %s = %04x\n", reg_names[offset * 2 + 1], (retval & 0xffff0000) >> 16);

	return retval;
}

WRITE32_MEMBER(gba_lcd_device::video_w)
{
	COMBINE_DATA(&m_regs[offset]);

	if (offset >= ARRAY_LENGTH(reg_names) / 2)
		throw emu_fatalerror("gba_lcd_device::video_w: Not enough register names in gba_lcd_device");

	if (ACCESSING_BITS_0_15)
		verboselog(*this, 2, "GBA I/O Write: %s = %04x\n", reg_names[offset * 2], data & 0x0000ffff);

	if (ACCESSING_BITS_16_31)
		verboselog(*this, 2, "GBA I/O Write: %s = %04x\n", reg_names[offset * 2 + 1], (data & 0xffff0000) >> 16);

	switch (offset)
	{
	case 0x0028/4:
		m_bg2x.update = true;
		break;
	case 0x002c/4:
		m_bg2y.update = true;
		break;
	case 0x0038/4:
		m_bg3x.update = true;
		break;
	case 0x003c/4:
		m_bg3y.update = true;
		break;
	}
}

READ32_MEMBER(gba_lcd_device::gba_pram_r)
{
	return m_pram[offset];
}

WRITE32_MEMBER(gba_lcd_device::gba_pram_w)
{
	COMBINE_DATA(&m_pram[offset]);
}

READ32_MEMBER(gba_lcd_device::gba_vram_r)
{
	return m_vram[offset];
}

WRITE32_MEMBER(gba_lcd_device::gba_vram_w)
{
	COMBINE_DATA(&m_vram[offset]);
}

READ32_MEMBER(gba_lcd_device::gba_oam_r)
{
	return m_oam[offset];
}

WRITE32_MEMBER(gba_lcd_device::gba_oam_w)
{
	COMBINE_DATA(&m_oam[offset]);
}

TIMER_CALLBACK_MEMBER(gba_lcd_device::perform_hbl)
{
	int scanline = screen().vpos();

	// reload LCD controller internal registers from I/O ones at vblank
	if (scanline == 0)
	{
		m_bg2x.update = true;
		m_bg2y.update = true;
		m_bg3x.update = true;
		m_bg3y.update = true;
	}

	// draw only visible scanlines
	if (scanline < 160)
	{
		draw_scanline(scanline);

		if (!m_dma_hblank_cb.isnull())
			m_dma_hblank_cb(ASSERT_LINE);
	}

	if (is_set(dispstat::hblank_irq_en))
	{
		if (!m_int_hblank_cb.isnull())
			m_int_hblank_cb(ASSERT_LINE);
	}

	set(dispstat::hblank);

	m_hbl_timer->adjust(attotime::never);
}

TIMER_CALLBACK_MEMBER(gba_lcd_device::perform_scan)
{
	clear(dispstat::hblank);
	clear(dispstat::vcount);

	int scanline = screen().vpos();

	// VBLANK is set for scanlines 160 through 226 (but not 227, which is the last line)
	if (scanline >= 160 && scanline < 227)
	{
		set(dispstat::vblank);

		// VBL IRQ and DMA on line 160
		if (scanline == 160)
		{
			if (is_set(dispstat::vblank_irq_en))
			{
				if (!m_int_vblank_cb.isnull())
					m_int_vblank_cb(ASSERT_LINE);
			}

			if (!m_dma_vblank_cb.isnull())
				m_dma_vblank_cb(ASSERT_LINE);
		}
	}
	else
	{
		clear(dispstat::vblank);
	}

	// handle VCOUNT match interrupt flag
	if (scanline == ((DISPSTAT >> 8) & 0xff))
	{
		set(dispstat::vcount);

		if (is_set(dispstat::vcount_irq_en))
		{
			if (!m_int_vcount_cb.isnull())
				m_int_vcount_cb(ASSERT_LINE);
		}
	}

	m_hbl_timer->adjust(screen().time_until_pos(scanline, 240));
	m_scan_timer->adjust(screen().time_until_pos((scanline + 1) % 228, 0));
}

void gba_lcd_device::gba_palette(palette_device &palette) const
{
	for (uint8_t b = 0; b < 32; b++)
	{
		for (uint8_t g = 0; g < 32; g++)
		{
			for (uint8_t r = 0; r < 32; r++)
			{
				palette.set_pen_color((b << 10) | (g << 5) | r, pal5bit(r), pal5bit(g), pal5bit(b));
			}
		}
	}
}

uint32_t gba_lcd_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);

	return 0;
}

void gba_lcd_device::device_start()
{
	/* resolve callbacks */
	m_int_hblank_cb.resolve();
	m_int_vblank_cb.resolve();
	m_int_vcount_cb.resolve();
	m_dma_hblank_cb.resolve();
	m_dma_vblank_cb.resolve();

	m_pram = make_unique_clear<uint32_t[]>(0x400 / 4);
	m_vram = make_unique_clear<uint32_t[]>(0x18000 / 4);
	m_oam = make_unique_clear<uint32_t[]>(0x400 / 4);

	screen().register_screen_bitmap(m_bitmap);

	/* create a timer to fire scanline functions */
	m_scan_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(gba_lcd_device::perform_scan),this));
	m_hbl_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(gba_lcd_device::perform_hbl),this));
	m_scan_timer->adjust(screen().time_until_pos(0, 0));

	save_item(NAME(m_regs));

	save_pointer(NAME(m_pram), 0x400 / 4);
	save_pointer(NAME(m_vram), 0x18000 / 4);
	save_pointer(NAME(m_oam), 0x400 / 4);

	save_item(NAME(m_bg2x.status));
	save_item(NAME(m_bg2x.update));

	save_item(NAME(m_bg2y.status));
	save_item(NAME(m_bg2y.update));

	save_item(NAME(m_bg3x.status));
	save_item(NAME(m_bg3x.update));

	save_item(NAME(m_bg3y.status));
	save_item(NAME(m_bg3y.update));

	save_item(NAME(m_scanline));
}

void gba_lcd_device::device_reset()
{
	memset(m_regs, 0, sizeof(m_regs));

	m_bg2x = { 0, false };
	m_bg2y = { 0, false };
	m_bg3x = { 0, false };
	m_bg3y = { 0, false };

	m_scan_timer->adjust(screen().time_until_pos(0, 0));
	m_hbl_timer->adjust(attotime::never);
}

void gba_lcd_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_raw(XTAL(16'777'216) / 4, 308, 0, 240, 228, 0, 160);
	screen.set_screen_update(FUNC(gba_lcd_device::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(gba_lcd_device::gba_palette), 32768);
}
