// license:BSD-3-Clause
// copyright-holders:R. Belmont,Ryan Holtz
/***************************************************************************

    gba_lcd.c

    File to handle emulation of the video hardware of the Game Boy Advance

    By R. Belmont, Ryan Holtz

***************************************************************************/

#include "emu.h"
#include "rendlay.h"

#include "gba_lcd.h"

#include "includes/gba.h" // this is a dependency from src/devices to src/mame which is very bad and should be fixed

/* LCD I/O Registers */
#define DISPCNT     HWLO(0x000)  /* 0x4000000  2  R/W   LCD Control */
#define GRNSWAP     HWHI(0x000)  /* 0x4000002  2  R/W   Undocumented - Green Swap */
#define DISPSTAT    HWLO(0x004)  /* 0x4000004  2  R/W   General LCD Status (STAT,LYC) */
#define VCOUNT      HWHI(0x004)  /* 0x4000006  2  R     Vertical Counter (LY) */
#define BG0CNT      HWLO(0x008)  /* 0x4000008  2  R/W   BG0 Control */
#define BG1CNT      HWHI(0x008)  /* 0x400000A  2  R/W   BG1 Control */
#define BG2CNT      HWLO(0x00C)  /* 0x400000C  2  R/W   BG2 Control */
#define BG3CNT      HWHI(0x00C)  /* 0x400000E  2  R/W   BG3 Control */
#define BG0HOFS     HWLO(0x010)  /* 0x4000010  2  W     BG0 X-Offset */
#define BG0VOFS     HWHI(0x010)  /* 0x4000012  2  W     BG0 Y-Offset */
#define BG1HOFS     HWLO(0x014)  /* 0x4000014  2  W     BG1 X-Offset */
#define BG1VOFS     HWHI(0x014)  /* 0x4000016  2  W     BG1 Y-Offset */
#define BG2HOFS     HWLO(0x018)  /* 0x4000018  2  W     BG2 X-Offset */
#define BG2VOFS     HWHI(0x018)  /* 0x400001A  2  W     BG2 Y-Offset */
#define BG3HOFS     HWLO(0x01C)  /* 0x400001C  2  W     BG3 X-Offset */
#define BG3VOFS     HWHI(0x01C)  /* 0x400001E  2  W     BG3 Y-Offset */
#define BG2PA       HWLO(0x020)  /* 0x4000020  2  W     BG2 Rotation/Scaling Parameter A (dx) */
#define BG2PB       HWHI(0x020)  /* 0x4000022  2  W     BG2 Rotation/Scaling Parameter B (dmx) */
#define BG2PC       HWLO(0x024)  /* 0x4000024  2  W     BG2 Rotation/Scaling Parameter C (dy) */
#define BG2PD       HWHI(0x024)  /* 0x4000026  2  W     BG2 Rotation/Scaling Parameter D (dmy) */
#define BG2X        WORD(0x028)  /* 0x4000028  4  W     BG2 Reference Point X-Coordinate */
#define BG2Y        WORD(0x02C)  /* 0x400002C  4  W     BG2 Reference Point Y-Coordinate */
#define BG3PA       HWLO(0x030)  /* 0x4000030  2  W     BG3 Rotation/Scaling Parameter A (dx) */
#define BG3PB       HWHI(0x030)  /* 0x4000032  2  W     BG3 Rotation/Scaling Parameter B (dmx) */
#define BG3PC       HWLO(0x034)  /* 0x4000034  2  W     BG3 Rotation/Scaling Parameter C (dy) */
#define BG3PD       HWHI(0x034)  /* 0x4000036  2  W     BG3 Rotation/Scaling Parameter D (dmy) */
#define BG3X        WORD(0x038)  /* 0x4000038  4  W     BG3 Reference Point X-Coordinate */
#define BG3Y        WORD(0x03C)  /* 0x400003C  4  W     BG3 Reference Point Y-Coordinate */
#define WIN0H       HWLO(0x040)  /* 0x4000040  2  W     Window 0 Horizontal Dimensions */
#define WIN1H       HWHI(0x040)  /* 0x4000042  2  W     Window 1 Horizontal Dimensions */
#define WIN0V       HWLO(0x044)  /* 0x4000044  2  W     Window 0 Vertical Dimensions */
#define WIN1V       HWHI(0x044)  /* 0x4000046  2  W     Window 1 Vertical Dimensions */
#define WININ       HWLO(0x048)  /* 0x4000048  2  R/W   Inside of Window 0 and 1 */
#define WINOUT      HWHI(0x048)  /* 0x400004A  2  R/W   Inside of OBJ Window & Outside of Windows */
#define MOSAIC      HWLO(0x04C)  /* 0x400004C  2  W     Mosaic Size */
                                 /* 0x400004E  2  -     Unused */
#define BLDCNT      HWLO(0x050)  /* 0x4000050  2  R/W   Color Special Effects Selection */
#define BLDALPHA    HWHI(0x050)  /* 0x4000052  2  W     Alpha Blending Coefficients */
#define BLDY        HWLO(0x054)  /* 0x4000054  2  W     Brightness (Fade-In/Out) Coefficient */
                                 /* 0x4000056  2  -     Unused */
                                                      
#define DISPSTAT_SET(val)       HWLO_SET(0x004, val)
#define DISPSTAT_RESET(val)     HWLO_RESET(0x004, val)

#define DISPSTAT_VBL            0x0001
#define DISPSTAT_HBL            0x0002
#define DISPSTAT_VCNT           0x0004
#define DISPSTAT_VBL_IRQ_EN     0x0008
#define DISPSTAT_HBL_IRQ_EN     0x0010
#define DISPSTAT_VCNT_IRQ_EN    0x0020
#define DISPSTAT_VCNT_VALUE     0xff00

#define DISPCNT_MODE            0x0007
#define DISPCNT_FRAMESEL        0x0010
#define DISPCNT_HBL_FREE        0x0020

#define DISPCNT_VRAM_MAP        0x0040
#define DISPCNT_VRAM_MAP_2D     0x0000
#define DISPCNT_VRAM_MAP_1D     0x0040

#define DISPCNT_BLANK           0x0080
#define DISPCNT_BG0_EN          0x0100
#define DISPCNT_BG1_EN          0x0200
#define DISPCNT_BG2_EN          0x0400
#define DISPCNT_BG3_EN          0x0800
#define DISPCNT_OBJ_EN          0x1000
#define DISPCNT_WIN0_EN         0x2000
#define DISPCNT_WIN1_EN         0x4000
#define DISPCNT_OBJWIN_EN       0x8000

#define OBJ_Y_COORD             0x00ff
#define OBJ_ROZMODE             0x0300
#define OBJ_ROZMODE_NONE        0x0000
#define OBJ_ROZMODE_ROZ         0x0100
#define OBJ_ROZMODE_DISABLE     0x0200
#define OBJ_ROZMODE_DBLROZ      0x0300

#define OBJ_MODE                0x0c00
#define OBJ_MODE_NORMAL         0x0000
#define OBJ_MODE_ALPHA          0x0400
#define OBJ_MODE_WINDOW         0x0800
#define OBJ_MODE_UNDEFINED      0x0c00

#define OBJ_MOSAIC              0x1000

#define OBJ_PALMODE             0x2000
#define OBJ_PALMODE_16          0x0000
#define OBJ_PALMODE_256         0x2000

#define OBJ_SHAPE               0xc000
#define OBJ_SHAPE_SQR           0x0000
#define OBJ_SHAPE_HORIZ         0x4000
#define OBJ_SHAPE_VERT          0x8000

#define OBJ_X_COORD             0x01ff
#define OBJ_SCALE_PARAM         0x3e00
#define OBJ_SCALE_PARAM_SHIFT   9
#define OBJ_HFLIP               0x1000
#define OBJ_VFLIP               0x2000
#define OBJ_SIZE                0xc000
#define OBJ_SIZE_8              0x0000
#define OBJ_SIZE_16             0x4000
#define OBJ_SIZE_32             0x8000
#define OBJ_SIZE_64             0xc000

#define OBJ_TILENUM             0x03ff
#define OBJ_PRIORITY            0x0c00
#define OBJ_PRIORITY_SHIFT      10
#define OBJ_PALNUM              0xf000
#define OBJ_PALNUM_SHIFT        12

#define BGCNT_SCREENSIZE        0xc000
#define BGCNT_SCREENSIZE_SHIFT  14
#define BGCNT_PALETTESET_WRAP   0x2000
#define BGCNT_SCREENBASE        0x1f00
#define BGCNT_SCREENBASE_SHIFT  8
#define BGCNT_PALETTE256        0x0080
#define BGCNT_MOSAIC            0x0040
#define BGCNT_CHARBASE          0x003c
#define BGCNT_CHARBASE_SHIFT    2
#define BGCNT_PRIORITY          0x0003

#define BLDCNT_BG0TP1           0x0001
#define BLDCNT_BG1TP1           0x0002
#define BLDCNT_BG2TP1           0x0004
#define BLDCNT_BG3TP1           0x0008
#define BLDCNT_OBJTP1           0x0010
#define BLDCNT_BDTP1            0x0020
#define BLDCNT_SFX              0x00c0
#define BLDCNT_SFX_NONE         0x0000
#define BLDCNT_SFX_ALPHA        0x0040
#define BLDCNT_SFX_LIGHTEN      0x0080
#define BLDCNT_SFX_DARKEN       0x00c0
#define BLDCNT_BG0TP2           0x0100
#define BLDCNT_BG1TP2           0x0200
#define BLDCNT_BG2TP2           0x0400
#define BLDCNT_BG3TP2           0x0800
#define BLDCNT_OBJTP2           0x1000
#define BLDCNT_BDTP2            0x2000
#define BLDCNT_TP2_SHIFT        8

#define TILEOBJ_TILE            0x03ff
#define TILEOBJ_HFLIP           0x0400
#define TILEOBJ_VFLIP           0x0800
#define TILEOBJ_PALETTE         0xf000

#define GBA_MODE0               0
#define GBA_MODE1               1
#define GBA_MODE2               2
#define GBA_MODE345             3

#define GBA_SUBMODE0            0
#define GBA_SUBMODE1            1
#define GBA_SUBMODE2            2

#define VERBOSE_LEVEL   (0)

static inline void ATTR_PRINTF(3,4) verboselog(device_t &device, int n_level, const char *s_fmt, ...)
{
	if( VERBOSE_LEVEL >= n_level )
	{
		va_list v;
		char buf[ 32768 ];
		va_start( v, s_fmt );
		vsprintf( buf, s_fmt, v );
		va_end( v );
		device.logerror( "%08x: %s", device.machine().describe_context(), buf );
	}
}

static const int coeff[32] = {
	0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16
};

static inline UINT32 alpha_blend_pixel(UINT32 color0, UINT32 color1, int ca, int cb)
{
	if(color0 < 0x80000000)
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

		if(r > 0x1f) r = 0x1f;
		if(g > 0x1f) g = 0x1f;
		if(b > 0x1f) b = 0x1f;

		return (color0 & 0xffff0000) | (b << 10) | (g << 5) | r;
	}
	return color0;
}

static inline UINT32 increase_brightness(UINT32 color, int coeff_)
{
	int r = (color >>  0) & 0x1f;
	int g = (color >>  5) & 0x1f;
	int b = (color >> 10) & 0x1f;

	r += ((0x1f - r) * coeff_) >> 4;
	g += ((0x1f - g) * coeff_) >> 4;
	b += ((0x1f - b) * coeff_) >> 4;

	if(r > 0x1f) r = 0x1f;
	if(g > 0x1f) g = 0x1f;
	if(b > 0x1f) b = 0x1f;

	return (color & 0xffff0000) | (b << 10) | (g << 5) | r;
}

static inline UINT32 decrease_brightness(UINT32 color, int coeff_)
{
	int r = (color >>  0) & 0x1f;
	int g = (color >>  5) & 0x1f;
	int b = (color >> 10) & 0x1f;

	r -= (r * coeff_) >> 4;
	g -= (g * coeff_) >> 4;
	b -= (b * coeff_) >> 4;

	if(r < 0) r = 0;
	if(g < 0) g = 0;
	if(b < 0) b = 0;

	return (color & 0xffff0000) | (b << 10) | (g << 5) | r;
}

const device_type GBA_LCD = &device_creator<gba_lcd_device>;

gba_lcd_device::gba_lcd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
				: device_t(mconfig, GBA_LCD, "GBA LCD", tag, owner, clock, "gba_lcd", __FILE__),
					device_video_interface(mconfig, *this)
{
}

inline void gba_lcd_device::update_mask(UINT8* mask, int mode, int submode, UINT32* obj_win, UINT8 inwin0, UINT8 inwin1, UINT8 in0_mask, UINT8 in1_mask, UINT8 out_mask)
{
	UINT8 mode_mask = 0;
	if (submode == GBA_SUBMODE2)
	{
		for (int x = 0; x < 240; x++)
		{
			mask[x] = out_mask;

			if ((obj_win[x] & 0x80000000) == 0)
				mask[x] = WINOUT >> 8;

			if (inwin1)
			{
				if (is_in_window(x, 1))
					mask[x] = in1_mask;
			}

			if (inwin0)
			{
				if (is_in_window(x, 0))
					mask[x] = in0_mask;
			}
		}
	}

	if (mode == GBA_MODE1)
	{
		// disable line3
		mode_mask = ~0x08;
	}
	else if (mode == GBA_MODE2)
	{
		// disable line0 & line1
		mode_mask = ~0x03;
	}
	else if (mode == GBA_MODE345)
	{
		// disable line0, line1 & line3
		mode_mask = ~0x0b;
	}


	if (mode_mask)
	{
		for (int x = 0; x < 240; x++)
			mask[x] &= mode_mask;
	}
}

void gba_lcd_device::draw_modes(int mode, int submode, int y, UINT32* line0, UINT32* line1, UINT32* line2, UINT32* line3, UINT32* lineOBJ, UINT32* lineOBJWin, UINT32* lineMix, int bpp)
{
	UINT32 backdrop = ((UINT16*)m_pram.get())[0] | 0x30000000;
	int inWindow0 = 0;
	int inWindow1 = 0;
	UINT8 inWin0Mask = WININ & 0x00ff;
	UINT8 inWin1Mask = WININ >> 8;
	UINT8 outMask = WINOUT & 0x00ff;
	UINT8 masks[240];   // this puts together WinMasks with the fact that some modes/submodes skip specific layers!

	if (submode == GBA_SUBMODE2)
	{
		if (DISPCNT & DISPCNT_WIN0_EN)
		{
			UINT8 v0 = WIN0V >> 8;
			UINT8 v1 = WIN0V & 0x00ff;
			inWindow0 = ((v0 == v1) && (v0 >= 0xe8)) ? 1 : 0;
			if (v1 >= v0)
				inWindow0 |= (y >= v0 && y < v1) ? 1 : 0;
			else
				inWindow0 |= (y >= v0 || y < v1) ? 1 : 0;
		}

		if (DISPCNT & DISPCNT_WIN1_EN)
		{
			UINT8 v0 = WIN1V >> 8;
			UINT8 v1 = WIN1V & 0x00ff;
			inWindow1 = ((v0 == v1) && (v0 >= 0xe8)) ? 1 : 0;
			if (v1 >= v0)
				inWindow1 |= (y >= v0 && y < v1) ? 1 : 0;
			else
				inWindow1 |= (y >= v0 || y < v1) ? 1 : 0;
		}
	}

	// Draw BG
	switch (mode)
	{
		case 0:
			draw_bg_scanline(line0, y, DISPCNT_BG0_EN, BG0CNT, BG0HOFS, BG0VOFS);
			draw_bg_scanline(line1, y, DISPCNT_BG1_EN, BG1CNT, BG1HOFS, BG1VOFS);
			draw_bg_scanline(line2, y, DISPCNT_BG2_EN, BG2CNT, BG2HOFS, BG2VOFS);
			draw_bg_scanline(line3, y, DISPCNT_BG3_EN, BG3CNT, BG3HOFS, BG3VOFS);
			break;
		case 1:
			draw_bg_scanline(line0, y, DISPCNT_BG0_EN, BG0CNT, BG0HOFS, BG0VOFS);
			draw_bg_scanline(line1, y, DISPCNT_BG1_EN, BG1CNT, BG1HOFS, BG1VOFS);
			draw_roz_scanline(line2, y, DISPCNT_BG2_EN, BG2CNT, BG2X, BG2Y, BG2PA, BG2PB, BG2PC, BG2PD, &m_gfxBG2X, &m_gfxBG2Y, m_gfxBG2Changed);
			break;
		case 2:
			draw_roz_scanline(line2, y, DISPCNT_BG2_EN, BG2CNT, BG2X, BG2Y, BG2PA, BG2PB, BG2PC, BG2PD, &m_gfxBG2X, &m_gfxBG2Y, m_gfxBG2Changed);
			draw_roz_scanline(line3, y, DISPCNT_BG3_EN, BG3CNT, BG3X, BG3Y, BG3PA, BG3PB, BG3PC, BG3PD, &m_gfxBG3X, &m_gfxBG3Y, m_gfxBG3Changed);
			break;
		case 3:
		case 4:
		case 5:
			draw_roz_bitmap_scanline(line2, y, DISPCNT_BG2_EN, BG2CNT, BG2X, BG2Y, BG2PA, BG2PB, BG2PC, BG2PD, &m_gfxBG2X, &m_gfxBG2Y, m_gfxBG2Changed, bpp);
			break;
	}

	// Draw OAM
	draw_gba_oam(lineOBJ, y);
	if (submode == GBA_SUBMODE2)
		draw_gba_oam_window(lineOBJWin, y);

	memset(masks, 0xff, sizeof(masks));
	update_mask(&masks[0], mode, submode, lineOBJWin, inWindow0, inWindow1, inWin0Mask, inWin1Mask, outMask);

	for (int x = 0; x < 240; x++)
	{
		UINT32 color = backdrop;
		UINT8 top = 0x20;

		if ((UINT8)(line0[x] >> 24) < (UINT8)(color >> 24) && masks[x] & 0x01)
		{
			color = line0[x];
			top = 0x01;
		}

		if ((UINT8)(line1[x] >> 24) < (UINT8)(color >> 24) && masks[x] & 0x02)
		{
			color = line1[x];
			top = 0x02;
		}

		if ((UINT8)(line2[x] >> 24) < (UINT8)(color >> 24) && masks[x] & 0x04)
		{
			color = line2[x];
			top = 0x04;
		}

		if ((UINT8)(line3[x] >> 24) < (UINT8)(color >> 24) && masks[x] & 0x08)
		{
			color = line3[x];
			top = 0x08;
		}

		if ((UINT8)(lineOBJ[x] >> 24) < (UINT8)(color >> 24) && masks[x] & 0x10)
		{
			color = lineOBJ[x];
			top = 0x10;
		}

		if (color & 0x00010000)
		{
			if (submode != GBA_SUBMODE0 || top == 0x10)
			{
				UINT32 back = backdrop;
				UINT8 top2 = 0x20;

				if ((UINT8)(line0[x] >> 24) < (UINT8)(back >> 24) && masks[x] & 0x01)
				{
					back = line0[x];
					top2 = 0x01;
				}

				if ((UINT8)(line1[x] >> 24) < (UINT8)(back >> 24) && masks[x] & 0x02)
				{
					back = line1[x];
					top2 = 0x02;
				}

				if ((UINT8)(line2[x] >> 24) < (UINT8)(back >> 24) && masks[x] & 0x04)
				{
					back = line2[x];
					top2 = 0x04;
				}

				if ((UINT8)(line3[x] >> 24) < (UINT8)(back >> 24) && masks[x] & 0x08)
				{
					back = line3[x];
					top2 = 0x08;
				}

				if (top2 & (BLDCNT >> BLDCNT_TP2_SHIFT))
					color = alpha_blend_pixel(color, back, coeff[BLDALPHA & 0x1f], coeff[(BLDALPHA >> 8) & 0x1f]);
				else
				{
					if (top & BLDCNT)
					{
						switch(BLDCNT & BLDCNT_SFX)
						{
							case BLDCNT_SFX_LIGHTEN:
								color = increase_brightness(color, coeff[BLDY & 0x1f]);
								break;
							case BLDCNT_SFX_DARKEN:
								color = decrease_brightness(color, coeff[BLDY & 0x1f]);
								break;
						}
					}
				}
			}
		}
		else if (submode == GBA_SUBMODE1 || (submode == GBA_SUBMODE2 && masks[x] & 0x20))
		{
			if (top & BLDCNT)
			{
				switch(BLDCNT & BLDCNT_SFX)
				{
					case BLDCNT_SFX_NONE:
						break;
					case BLDCNT_SFX_ALPHA:
					{
						UINT32 back = backdrop;
						UINT8 top2 = 0x20;

						if ((UINT8)(line0[x] >> 24) < (UINT8)(back >> 24) && masks[x] & 0x01)
						{
							if (top != 0x01)
							{
								back = line0[x];
								top2 = 0x01;
							}
						}

						if ((UINT8)(line1[x] >> 24) < (UINT8)(back >> 24) && masks[x] & 0x02)
						{
							if (top != 0x02)
							{
								back = line1[x];
								top2 = 0x02;
							}
						}

						if ((UINT8)(line2[x] >> 24) < (UINT8)(back >> 24) && masks[x] & 0x04)
						{
							if (top != 0x04)
							{
								back = line2[x];
								top2 = 0x04;
							}
						}

						if ((UINT8)(line3[x] >> 24) < (UINT8)(back >> 24) && masks[x] & 0x08)
						{
							if (top != 0x08)
							{
								back = line3[x];
								top2 = 0x08;
							}
						}

						if ((UINT8)(lineOBJ[x] >> 24) < (UINT8)(back >> 24) && masks[x] & 0x10)
						{
							if (top != 0x10)
							{
								back = lineOBJ[x];
								top2 = 0x10;
							}
						}

						if (top2 & (BLDCNT >> BLDCNT_TP2_SHIFT))
							color = alpha_blend_pixel(color, back, coeff[BLDALPHA & 0x1f], coeff[(BLDALPHA >> 8) & 0x1f]);
					}
						break;
					case BLDCNT_SFX_LIGHTEN:
						color = increase_brightness(color, coeff[BLDY & 0x1f]);
						break;
					case BLDCNT_SFX_DARKEN:
						color = decrease_brightness(color, coeff[BLDY & 0x1f]);
						break;
				}
			}
		}
		lineMix[x] = color;
	}
	if (mode == GBA_MODE1 || mode == GBA_MODE2 || mode == GBA_MODE345)
		m_gfxBG2Changed = 0;
	if (mode == GBA_MODE2)
		m_gfxBG3Changed = 0;
}

void gba_lcd_device::draw_roz_bitmap_scanline(UINT32 *scanline, int ypos, UINT32 enablemask, UINT32 ctrl, INT32 X, INT32 Y, INT32 PA, INT32 PB, INT32 PC, INT32 PD, INT32 *currentx, INT32 *currenty, int changed, int depth)
{
	UINT8 *src8 = (UINT8 *)m_vram.get();
	UINT16 *src16 = (UINT16 *)m_vram.get();
	UINT16 *palette = (UINT16 *)m_pram.get();
	INT32 sx = (depth == 4) ? 160 : 240;
	INT32 sy = (depth == 4) ? 128 : 160;
	UINT32 prio = ((ctrl & BGCNT_PRIORITY) << 25) + 0x1000000;
	INT32 cx, cy, pixx, pixy, x;

	if ((depth == 8) && (DISPCNT & DISPCNT_FRAMESEL))
		src8 += 0xa000;

	if ((depth == 4) && (DISPCNT & DISPCNT_FRAMESEL))
		src16 += 0xa000/2;

	// sign extend roz parameters
	if (X & 0x08000000) X |= 0xf0000000;
	if (Y & 0x08000000) Y |= 0xf0000000;
	if (PA & 0x8000) PA |= 0xffff0000;
	if (PB & 0x8000) PB |= 0xffff0000;
	if (PC & 0x8000) PC |= 0xffff0000;
	if (PD & 0x8000) PD |= 0xffff0000;

	if(ypos == 0)
		changed = 3;

	if(changed & 1)
		*currentx = X;
	else
		*currentx += PB;

	if(changed & 2)
		*currenty = Y;
	else
		*currenty += PD;

	cx = *currentx;
	cy = *currenty;

	if(ctrl & BGCNT_MOSAIC)
	{
		INT32 mosaic_line = ((MOSAIC & 0xf0) >> 4) + 1;
		INT32 tempy = (ypos / mosaic_line) * mosaic_line;
		cx = X + tempy*PB;
		cy = Y + tempy*PD;
	}

	pixx = cx >> 8;
	pixy = cy >> 8;

	for(x = 0; x < 240; x++)
	{
		if(pixx < 0 || pixy < 0 || pixx >= sx || pixy >= sy)
		{
			scanline[x] = 0x80000000;
		}
		else
		{
			if (depth == 8)
			{
				UINT8 color = src8[pixy*sx + pixx];
				scanline[x] = color ? (palette[color] | prio) : 0x80000000;
			}
			else
			{
				scanline[x] = src16[pixy*sx + pixx] | prio;
			}
		}

		cx += PA;
		cy += PC;

		pixx = cx >> 8;
		pixy = cy >> 8;
	}

	if(ctrl & BGCNT_MOSAIC)
	{
		INT32 mosaicx = (MOSAIC & 0x0f) + 1;
		if(mosaicx > 1)
		{
			INT32 m = 1;
			for(x = 0; x < 239; x++)
			{
				scanline[x+1] = scanline[x];
				m++;
				if(m == mosaicx)
				{
					m = 1;
					x++;
				}
			}
		}
	}
}

void gba_lcd_device::draw_roz_scanline(UINT32 *scanline, int ypos, UINT32 enablemask, UINT32 ctrl, INT32 X, INT32 Y, INT32 PA, INT32 PB, INT32 PC, INT32 PD, INT32 *currentx, INT32 *currenty, int changed)
{
	UINT32 base, mapbase, size;
	static const INT32 sizes[4] = { 128, 256, 512, 1024 };
	INT32 cx, cy, pixx, pixy;
	UINT8 *mgba_vram = (UINT8 *)m_vram.get();
	UINT32 tile;
	UINT16 *pgba_pram = (UINT16 *)m_pram.get();
	UINT16 pixel;
	UINT32 prio = ((ctrl & BGCNT_PRIORITY) << 25) + 0x1000000;
	int x = 0;

	for (x = 0; x < 240; x++)
		scanline[x] = 0x80000000;

	if (DISPCNT & enablemask)
	{
		base = ((ctrl & BGCNT_CHARBASE) >> BGCNT_CHARBASE_SHIFT) * 0x4000;          // VRAM base of tiles
		mapbase = ((ctrl & BGCNT_SCREENBASE) >> BGCNT_SCREENBASE_SHIFT) * 0x800;    // VRAM base of map
		size = (ctrl & BGCNT_SCREENSIZE) >> BGCNT_SCREENSIZE_SHIFT;                 // size of map in submaps

		// sign extend roz parameters
		if (X & 0x08000000) X |= 0xf0000000;
		if (Y & 0x08000000) Y |= 0xf0000000;
		if (PA & 0x8000) PA |= 0xffff0000;
		if (PB & 0x8000) PB |= 0xffff0000;
		if (PC & 0x8000) PC |= 0xffff0000;
		if (PD & 0x8000) PD |= 0xffff0000;

		if(ypos == 0)
			changed = 3;

		if(changed & 1)
			*currentx = X;
		else
			*currentx += PB;

		if(changed & 2)
			*currenty = Y;
		else
			*currenty += PD;

		cx = *currentx;
		cy = *currenty;

		if(ctrl & BGCNT_MOSAIC)
		{
			int mosaic_line = ((MOSAIC & 0xf0) >> 4) + 1;
			int y = ypos % mosaic_line;
			cx -= y*PB;
			cy -= y*PD;
		}

		pixx = cx >> 8;
		pixy = cy >> 8;

		if(ctrl & BGCNT_PALETTESET_WRAP)
		{
			pixx %= sizes[size];
			pixy %= sizes[size];
			if(pixx < 0)
			{
				pixx += sizes[size];
			}
			if(pixy < 0)
			{
				pixy += sizes[size];
			}
		}

		for(x = 0; x < 240; x++)
		{
			if(pixx < 0 || pixy < 0 || pixx >= sizes[size] || pixy >= sizes[size])
			{
				scanline[x] = 0x80000000;
			}
			else
			{
				int tilex = pixx & 7;
				int tiley = pixy & 7;

				// shall we shift for (ctrl & BGCNT_PALETTE256)?? or is not effective for ROZ?
				tile = mgba_vram[mapbase + (pixx >> 3) + (pixy >> 3) * (sizes[size] >> 3)];
				pixel = mgba_vram[base + (tile << 6) + (tiley << 3) + tilex];

				// plot it
				scanline[x] = pixel ? (pgba_pram[pixel] | prio) : 0x80000000;
			}

			cx += PA;
			cy += PC;

			pixx = cx >> 8;
			pixy = cy >> 8;

			if(ctrl & BGCNT_PALETTESET_WRAP)
			{
				pixx %= sizes[size];
				pixy %= sizes[size];
				if(pixx < 0)
				{
					pixx += sizes[size];
				}
				if(pixy < 0)
				{
					pixy += sizes[size];
				}
			}
		}

		if(ctrl & BGCNT_MOSAIC)
		{
			int mosaicx = (MOSAIC & 0x0f) + 1;
			if(mosaicx > 1)
			{
				int m = 1;
				for(x = 0; x < 239; x++)
				{
					scanline[x+1] = scanline[x];
					m++;
					if(m == mosaicx)
					{
						m = 1;
						x++;
					}
				}
			}
		}
	}
}

void gba_lcd_device::draw_bg_scanline(UINT32 *scanline, int ypos, UINT32 enablemask, UINT32 ctrl, UINT32 hofs, UINT32 vofs)
{
	UINT8 *vram = (UINT8*)m_vram.get();
	UINT16 *palette = (UINT16*)m_pram.get();
	UINT8 *chardata = &vram[((ctrl & BGCNT_CHARBASE) >> BGCNT_CHARBASE_SHIFT) * 0x4000];
	UINT16 *screendata = (UINT16*)&vram[((ctrl & BGCNT_SCREENBASE) >> BGCNT_SCREENBASE_SHIFT) * 0x800];
	UINT32 priority = ((ctrl & BGCNT_PRIORITY) << 25) + 0x1000000;
	INT32 width = 256;
	INT32 height = 256;
	INT32 maskx, masky, pixx, pixy;
	UINT8 use_mosaic = (ctrl & BGCNT_MOSAIC) ? 1 : 0;
	INT32 mosaicx = (MOSAIC & 0x000f) + 1;
	INT32 mosaicy = ((MOSAIC & 0x00f0) >> 4) + 1;
	INT32 stride;
	int x = 0;

	for (x = 0; x < 240; x++)
		scanline[x] = 0x80000000;

	if(DISPCNT & enablemask)
	{
		switch((ctrl & BGCNT_SCREENSIZE) >> BGCNT_SCREENSIZE_SHIFT)
		{
			case 1:
				width = 512;
				break;
			case 2:
				height = 512;
				break;
			case 3:
				width = 512;
				height = 512;
				break;
		}

		maskx = width - 1;
		masky = height - 1;

		pixx = hofs & maskx;
		pixy = (vofs + ypos) & masky;

		if(use_mosaic)
		{
			if((ypos % mosaicy) != 0)
			{
				mosaicy = (ypos / mosaicy) * mosaicy;
				pixy = (vofs + mosaicy) & masky;
			}
		}

		if(pixy > 255 && height > 256)
		{
			pixy &= 0x000000ff;
			screendata += 0x400;
			if(width > 256)
			{
				screendata += 0x400;
			}
		}

		stride = (pixy >> 3) << 5;

		UINT16 *src = screendata + 0x400 * (pixx >> 8) + ((pixx & 255) >> 3) + stride;
		for(x = 0; x < 240; x++)
		{
			UINT16 data = *src;
			INT32 tile = data & TILEOBJ_TILE;
			INT32 tilex = pixx & 7;
			INT32 tiley = pixy & 7;
			UINT8 color;
			UINT8 palindex;

			if(data & TILEOBJ_HFLIP)
			{
				tilex = 7 - tilex;
			}
			if(data & TILEOBJ_VFLIP)
			{
				tiley = 7 - tiley;
			}

			if (ctrl & BGCNT_PALETTE256)
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
			else
				scanline[x] = 0x80000000;

			if (data & TILEOBJ_HFLIP)
			{
				if (tilex == 0)
				{
					src++;
				}
			}
			else if (tilex == 7)
			{
				src++;
			}

			pixx++;
			if(pixx == 256)
			{
				if(width > 256)
				{
					src = screendata + 0x400 + stride;
				}
				else
				{
					src = screendata + stride;
					pixx = 0;
				}
			}
			else if(pixx >= width)
			{
				pixx = 0;
				src = screendata + stride;
			}
		}

		if(use_mosaic)
		{
			if(mosaicx > 1)
			{
				INT32 m = 1;
				for(x = 0; x < 239; x++)
				{
					scanline[x+1] = scanline[x];
					m++;
					if(m == mosaicx)
					{
						m = 1;
						x++;
					}
				}
			}
		}

	}
}

void gba_lcd_device::draw_gba_oam_window(UINT32 *scanline, int y)
{
	INT16 gba_oamindex;
	UINT32 tilebytebase, tileindex, tiledrawindex;
	UINT32 width, height;
	UINT16 *pgba_oam = (UINT16 *)m_oam.get();
	UINT8 *src = (UINT8*)m_vram.get();
	int x = 0;

	for (x = 0; x < 240; x++)
		scanline[x] = 0x80000000;

	if (DISPCNT & DISPCNT_OBJWIN_EN)
	{
		for( gba_oamindex = 127; gba_oamindex >= 0; gba_oamindex-- )
		{
			INT32 sx, sy;
			UINT16 attr0, attr1, attr2;
			INT32 cury;

			attr0 = pgba_oam[(4*gba_oamindex)+0];
			attr1 = pgba_oam[(4*gba_oamindex)+1];
			attr2 = pgba_oam[(4*gba_oamindex)+2];

			sx = (attr1 & OBJ_X_COORD);
			sy = (attr0 & OBJ_Y_COORD);

			if(sy > 160)
			{
				sy -= 256;
			}

			if ((attr0 & OBJ_MODE) != OBJ_MODE_WINDOW)
			{
				continue;
			}
			else
			{
				width = 0;
				height = 0;
				switch (attr0 & OBJ_SHAPE )
				{
					case OBJ_SHAPE_SQR:
						switch(attr1 & OBJ_SIZE )
						{
							case OBJ_SIZE_8:
								width = 1;
								height = 1;
								break;
							case OBJ_SIZE_16:
								width = 2;
								height = 2;
								break;
							case OBJ_SIZE_32:
								width = 4;
								height = 4;
								break;
							case OBJ_SIZE_64:
								width = 8;
								height = 8;
								break;
						}
						break;
					case OBJ_SHAPE_HORIZ:
						switch(attr1 & OBJ_SIZE )
						{
							case OBJ_SIZE_8:
								width = 2;
								height = 1;
								break;
							case OBJ_SIZE_16:
								width = 4;
								height = 1;
								break;
							case OBJ_SIZE_32:
								width = 4;
								height = 2;
								break;
							case OBJ_SIZE_64:
								width = 8;
								height = 4;
								break;
						}
						break;
					case OBJ_SHAPE_VERT:
						switch(attr1 & OBJ_SIZE )
						{
							case OBJ_SIZE_8:
								width = 1;
								height = 2;
								break;
							case OBJ_SIZE_16:
								width = 1;
								height = 4;
								break;
							case OBJ_SIZE_32:
								width = 2;
								height = 4;
								break;
							case OBJ_SIZE_64:
								width = 4;
								height = 8;
								break;
						}
						break;
					default:
						width = 0;
						height = 0;
						verboselog(*this, 0, "OAM error: Trying to draw OBJ with OBJ_SHAPE = 3!\n" );
						break;
				}

				tiledrawindex = tileindex = (attr2 & OBJ_TILENUM);
				tilebytebase = 0x10000; // the index doesn't change in the higher modes, we just ignore sprites that are out of range

				if (attr0 & OBJ_ROZMODE_ROZ)
				{
					INT32 fx, fy, ax, ay, rx, ry;
					INT16 dx, dy, dmx, dmy;
					UINT8 color;

					width *= 8;
					height *= 8;

					if ((attr0 & OBJ_ROZMODE) == OBJ_ROZMODE_DISABLE)
					{
						continue;
					}

					fx = width;
					fy = height;

					if((attr0 & OBJ_ROZMODE) == OBJ_ROZMODE_DBLROZ)
					{
						fx *= 2;
						fy *= 2;
					}

					cury = y - sy;

					if(cury >= 0 && cury < fy)
					{
						if(sx < 240 || ((sx + fx) & 0x1ff) < 240)
						{
							int rot = (attr1 & OBJ_SCALE_PARAM) >> OBJ_SCALE_PARAM_SHIFT;
							dx  = (INT16)pgba_oam[(rot << 4)+3];
							dmx = (INT16)pgba_oam[(rot << 4)+7];
							dy  = (INT16)pgba_oam[(rot << 4)+11];
							dmy = (INT16)pgba_oam[(rot << 4)+15];

							rx = (width << 7) - (fx >> 1)*dx - (fy >> 1)*dmx + cury * dmx;
							ry = (height << 7) - (fx >> 1)*dy - (fy >> 1)*dmy + cury * dmy;

							if((attr0 & OBJ_PALMODE) == OBJ_PALMODE_256)
							{
								int inc = 32;
								if((DISPCNT & DISPCNT_MODE) > 2 && tiledrawindex < 0x200)
								{
									continue;
								}
								if((DISPCNT & DISPCNT_VRAM_MAP) == DISPCNT_VRAM_MAP_1D)
								{
									inc = sx >> 2;
								}
								else
								{
									tiledrawindex &= 0x3fe;
								}
								for(x = 0; x < fx; x++)
								{
									ax = rx >> 8;
									ay = ry >> 8;

									if(ax < 0 || ax >= sx || ay < 0 || ay >= sy)
									{
									}
									else
									{
										color = src[tilebytebase + ((((tiledrawindex + (ay >> 3) * inc) << 5) + ((ay & 0x07) << 3) + ((ax >> 3) << 6) + (ax & 0x07)) & 0x7fff)];

										if(color)
										{
											scanline[sx] = 1;
										}
									}

									sx = (sx + 1) & 0x1ff;

									rx += dx;
									ry += dy;
								}
							}
							else
							{
								int inc = 32;
								if((DISPCNT & DISPCNT_MODE) > 2 && tiledrawindex < 0x200)
								{
									continue;
								}
								if((DISPCNT & DISPCNT_VRAM_MAP) == DISPCNT_VRAM_MAP_1D)
								{
									inc = sx >> 3;
								}
								for(x = 0; x < fx; x++)
								{
									ax = rx >> 8;
									ay = ry >> 8;

									if(ax < 0 || ax >= sx || ay < 0 || ay >= sy)
									{
									}
									else
									{
										color = src[tilebytebase + ((((tiledrawindex + (ay >> 3) * inc) << 5) + ((ay & 0x07) << 2) + ((ax >> 3) << 5) + ((ax & 0x07) >> 1)) & 0x7fff)];

										if(ax & 1)
										{
											color >>= 4;
										}
										else
										{
											color &= 0x0f;
										}

										if(color)
										{
											scanline[sx] = 1;
										}
									}

									sx = (sx + 1) & 0x1ff;

									rx += dx;
									ry += dy;
								}
							}
						}
					}
				}
				else
				{
					INT32 ax;
					int cury_ = y - sy;

					width *= 8;
					height *= 8;

					if(cury_ >= 0 && cury_ < height)
					{
						if( ( (sx < 240) || ( ( (sx + width) & 0x1ff ) < 240 ) ) && (attr0 & OBJ_ROZMODE_DISABLE) == 0)
						{
							if((attr0 & OBJ_PALMODE) == OBJ_PALMODE_256)
							{
								int inc = 32;
								int address = 0;
								if(attr1 & OBJ_VFLIP)
								{
									cury_ = height - cury_ - 1;
								}
								if((DISPCNT & DISPCNT_MODE) > 2 && tiledrawindex < 0x200)
								{
									continue;
								}

								if((DISPCNT & DISPCNT_VRAM_MAP) == DISPCNT_VRAM_MAP_1D)
								{
									inc = width >> 2;
								}
								else
								{
									tiledrawindex &= 0x3fe;
								}

								ax = 0;
								if(attr1 & OBJ_HFLIP)
								{
									ax = width - 1;
								}

								address = 0x10000 + ((((tiledrawindex + (cury_ >> 3) * inc) << 5) + ((cury_ & 7) << 3) + ((ax >> 3) << 6) + (ax & 7)) & 0x7fff);

								if(attr1 & OBJ_HFLIP)
								{
									ax = 7;
								}

								for(x = 0; x < width; x++)
								{
									if(sx < 240)
									{
										UINT8 color = src[address];

										if(color)
										{
											scanline[sx] = 1;
										}
									}

									sx = (sx + 1) & 0x1ff;

									if(attr1 & OBJ_HFLIP)
									{
										ax--;
										address--;
										if(ax == -1)
										{
											address -= 56;
											ax = 7;
										}
										if(address < 0x10000)
										{
											address += 0x8000;
										}
									}
									else
									{
										ax++;
										address++;
										if(ax == 8)
										{
											address += 56;
											ax = 0;
										}
										if(address > 0x17fff)
										{
											address -= 0x8000;
										}
									}
								}
							}
							else
							{
								int inc = 32;
								UINT32 address;
								if(attr1 & OBJ_VFLIP)
								{
									cury_ = height - cury_ - 1;
								}
								if((DISPCNT & DISPCNT_MODE) > 2 && tiledrawindex < 0x200)
								{
									continue;
								}

								if((DISPCNT & DISPCNT_VRAM_MAP) == DISPCNT_VRAM_MAP_1D)
								{
									inc = width >> 3;
								}

								ax = 0;
								if(attr1 & OBJ_HFLIP)
								{
									ax = width - 1;
								}

								address = 0x10000 + ((((tiledrawindex + (cury_ >> 3) * inc) << 5) + ((cury_ & 0x07) << 2) + ((ax >> 3) << 5) + ((ax & 0x07) >> 1)) & 0x7fff);
								if(attr1 & OBJ_HFLIP)
								{
									ax = 7;
									for(x = width - 1; x >= 0; x--)
									{
										if(sx < 240)
										{
											UINT8 color = src[address];
											if(x & 1)
											{
												color >>= 4;
											}
											else
											{
												color &= 0x0f;
											}

											if(color)
											{
												scanline[sx] = 1;
											}
										}

										sx = (sx + 1) & 0x1ff;
										ax--;
										if((x & 1) == 0)
										{
											address--;
										}
										if(ax == -1)
										{
											ax = 7;
											address -= 28;
										}
										if(address < 0x10000)
										{
											address += 0x8000;
										}
									}
								}
								else
								{
									for(x = 0; x < width; x++)
									{
										if(sx < 240)
										{
											UINT8 color = src[address];

											if(x & 1)
											{
												color >>= 4;
											}
											else
											{
												color &= 0x0f;
											}

											if(color)
											{
												scanline[sx] = 1;
											}
										}

										sx = (sx + 1) & 0x1ff;
										ax++;
										if(x & 1)
										{
											address++;
										}
										if(ax == 8)
										{
											address += 28;
											ax = 0;
										}
										if(address > 0x17fff)
										{
											address -= 0x8000;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

void gba_lcd_device::draw_gba_oam(UINT32 *scanline, int y)
{
	INT16 gba_oamindex;
	INT32 mosaiccnt = 0;
	INT32 mosaicy = ((MOSAIC & 0xf000) >> 12) + 1;
	INT32 mosaicx = ((MOSAIC & 0x0f00) >>  8) + 1;
	UINT32 tileindex, tiledrawindex; //, tilebytebase
	UINT8 width, height;
	UINT16 *pgba_oam = (UINT16 *)m_oam.get();
	UINT8 *src = (UINT8 *)m_vram.get();
	UINT16 *palette = (UINT16*)m_pram.get();
	int x = 0;

	for (x = 0; x < 240; x++)
		scanline[x] = 0x80000000;

	if( DISPCNT & DISPCNT_OBJ_EN )
	{
		for( gba_oamindex = 0; gba_oamindex < 128; gba_oamindex++ )
		{
			UINT16 attr0, attr1, attr2;
			INT32 cury;
			UINT32 prio;
			UINT32 priority;

			attr0 = pgba_oam[(4*gba_oamindex)+0];
			attr1 = pgba_oam[(4*gba_oamindex)+1];
			attr2 = pgba_oam[(4*gba_oamindex)+2];
			priority = (attr2 & OBJ_PRIORITY) >> OBJ_PRIORITY_SHIFT;
			prio = (priority << 25) | ((attr0 & OBJ_MODE) << 6);

			if ((attr0 & OBJ_MODE) != OBJ_MODE_WINDOW)
			{
				width = 0;
				height = 0;
				switch (attr0 & OBJ_SHAPE)
				{
					case OBJ_SHAPE_SQR:
						switch(attr1 & OBJ_SIZE )
						{
							case OBJ_SIZE_8:
								width = 1;
								height = 1;
								break;
							case OBJ_SIZE_16:
								width = 2;
								height = 2;
								break;
							case OBJ_SIZE_32:
								width = 4;
								height = 4;
								break;
							case OBJ_SIZE_64:
								width = 8;
								height = 8;
								break;
						}
						break;
					case OBJ_SHAPE_HORIZ:
						switch(attr1 & OBJ_SIZE )
						{
							case OBJ_SIZE_8:
								width = 2;
								height = 1;
								break;
							case OBJ_SIZE_16:
								width = 4;
								height = 1;
								break;
							case OBJ_SIZE_32:
								width = 4;
								height = 2;
								break;
							case OBJ_SIZE_64:
								width = 8;
								height = 4;
								break;
						}
						break;
					case OBJ_SHAPE_VERT:
						switch(attr1 & OBJ_SIZE )
						{
							case OBJ_SIZE_8:
								width = 1;
								height = 2;
								break;
							case OBJ_SIZE_16:
								width = 1;
								height = 4;
								break;
							case OBJ_SIZE_32:
								width = 2;
								height = 4;
								break;
							case OBJ_SIZE_64:
								width = 4;
								height = 8;
								break;
						}
						break;
					default:
						width = 0;
						height = 0;
						verboselog(*this, 0, "OAM error: Trying to draw OBJ with OBJ_SHAPE = 3!\n" );
						break;
				}

				tiledrawindex = tileindex = (attr2 & OBJ_TILENUM);
//              tilebytebase = 0x10000; // the index doesn't change in the higher modes, we just ignore sprites that are out of range

				if (attr0 & OBJ_ROZMODE_ROZ)
				{
					INT32 sx, sy;
					INT32 fx, fy, rx, ry;
					INT16 dx, dy, dmx, dmy;

					width *= 8;
					height *= 8;

					if ((attr0 & OBJ_ROZMODE) == OBJ_ROZMODE_DISABLE)
					{
						continue;
					}

					sx = (attr1 & OBJ_X_COORD);
					sy = (attr0 & OBJ_Y_COORD);

					if(sy > 160)
					{
						sy -= 256;
					}

					fx = width;
					fy = height;

					if((attr0 & OBJ_ROZMODE) == OBJ_ROZMODE_DBLROZ)
					{
						fx *= 2;
						fy *= 2;
					}

					cury = y - sy;

					if(cury >= 0 && cury < fy)
					{
						if(sx < 240 || ((sx + fx) & 0x1ff) < 240)
						{
							INT32 oamparam = (attr1 & OBJ_SCALE_PARAM) >> OBJ_SCALE_PARAM_SHIFT;

							dx  = (INT16)pgba_oam[(oamparam << 4)+3];
							dmx = (INT16)pgba_oam[(oamparam << 4)+7];
							dy  = (INT16)pgba_oam[(oamparam << 4)+11];
							dmy = (INT16)pgba_oam[(oamparam << 4)+15];

							if(attr0 & OBJ_MOSAIC)
							{
								cury -= (cury % mosaicy);
							}

							rx = (width << 7) - (fx >> 1)*dx - (fy >> 1)*dmx + cury*dmx;
							ry = (height << 7) - (fx >> 1)*dy - (fy >> 1)*dmy + cury*dmy;

							if((attr0 & OBJ_PALMODE) == OBJ_PALMODE_256)
							{
								INT32 inc = 32;

								if((DISPCNT & DISPCNT_MODE) > 2 && tiledrawindex < 0x200)
								{
									continue;
								}

								if((DISPCNT & DISPCNT_VRAM_MAP) == DISPCNT_VRAM_MAP_1D)
								{
									inc = width >> 2;
								}
								else
								{
									tiledrawindex &= 0x3fe;
								}

								for(x = 0; x < fx; x++)
								{
									INT32 pixx = rx >> 8;
									INT32 pixy = ry >> 8;

									if(!(pixx < 0 || pixx >= width || pixy < 0 || pixy >= height || sx >= 240))
									{
										UINT8 color = src[0x10000 + ((((tiledrawindex + (pixy >> 3) * inc) << 5) + ((pixy & 7) << 3) + ((pixx >> 3) << 6) + (pixx & 7)) & 0x7fff)];
										if(color == 0 && priority < ((scanline[sx] >> 25) & 3))
										{
											scanline[sx] = (scanline[sx] & 0xf9ffffff) | prio;
											if((attr0 & OBJ_MOSAIC) != 0 && mosaiccnt != 0)
											{
												scanline[sx] = (scanline[sx - 1] & 0xf9ffffff) | prio;
											}
										}
										else if(color != 0 && prio < (scanline[sx] & 0xff000000))
										{
											scanline[sx] = palette[256 + color] | prio;
											if((attr0 & OBJ_MOSAIC) != 0 && mosaiccnt != 0)
											{
												scanline[sx] = (scanline[sx - 1] & 0xf9ffffff) | prio;
											}
										}

										if(attr0 & OBJ_MOSAIC)
										{
											mosaiccnt++;
											if(mosaiccnt == mosaicx)
											{
												mosaiccnt = 0;
											}
										}
									}

									sx++;
									sx &= 0x1ff;

									rx += dx;
									ry += dy;
								}
							}
							else
							{
								INT32 inc = 32;
								INT32 palentry = (attr2 >> 8) & 0xf0;

								if((DISPCNT & DISPCNT_MODE) > 2 && tiledrawindex < 0x200)
								{
									continue;
								}

								if((DISPCNT & DISPCNT_VRAM_MAP) == DISPCNT_VRAM_MAP_1D)
								{
									inc = width >> 3;
								}

								for(x = 0; x < fx; x++)
								{
									INT32 pixx = rx >> 8;
									INT32 pixy = ry >> 8;

									if(!(pixx < 0 || pixx >= width || pixy < 0 || pixy >= height || sx >= 240))
									{
										UINT8 color = src[0x10000 + ((((tiledrawindex + (pixy >> 3) * inc) << 5) + ((pixy & 7) << 2) + ((pixx >> 3) << 5) + ((pixx & 7) >> 1)) & 0x7fff)];

										if(pixx & 1)
										{
											color >>= 4;
										}
										else
										{
											color &= 0x0f;
										}

										if(color == 0 && priority < ((scanline[sx] >> 25) & 3))
										{
											scanline[sx] = (scanline[sx] & 0xf9ffffff) | prio;
											if((attr0 & OBJ_MOSAIC) != 0 && mosaiccnt != 0)
											{
												scanline[sx] = (scanline[sx - 1] & 0xf9ffffff) | prio;
											}
										}
										else if(color != 0 && prio < (scanline[sx] & 0xff000000))
										{
											scanline[sx] = palette[256 + palentry + color] | prio;
											if((attr0 & OBJ_MOSAIC) != 0 && mosaiccnt != 0)
											{
												scanline[sx] = (scanline[sx - 1] & 0xf9ffffff) | prio;
											}
										}
									}

									if(attr0 & OBJ_MOSAIC)
									{
										mosaiccnt++;
										if(mosaiccnt == mosaicx)
										{
											mosaiccnt = 0;
										}
									}

									sx++;

									sx &= 0x1ff;

									rx += dx;
									ry += dy;
								}
							}
						}
					}
				}
				else
				{
					INT32 sx, sy;
					INT32 vflip = (attr1 & OBJ_VFLIP) ? 1 : 0;
					INT32 hflip = (attr1 & OBJ_HFLIP) ? 1 : 0;

					width *= 8;
					height *= 8;

					if ((attr0 & OBJ_ROZMODE) == OBJ_ROZMODE_DISABLE)
					{
						continue;
					}

					sx = (attr1 & OBJ_X_COORD);
					sy = (attr0 & OBJ_Y_COORD);

					if(sy > 160)
					{
						sy -= 256;
					}

					cury = y - sy;
					if(cury >= 0 && cury < height)
					{
						if(sx < 240 || ((sx + width) & 0x1ff) < 240)
						{
							if((attr0 & OBJ_PALMODE) == OBJ_PALMODE_256)
							{
								INT32 pixx;
								INT32 inc = 32;
								INT32 address = 0;

								if(vflip)
								{
									cury = height - cury - 1;
								}

								if((DISPCNT & DISPCNT_MODE) > 2 && tiledrawindex < 0x200)
								{
									continue;
								}

								if((DISPCNT & DISPCNT_VRAM_MAP) == DISPCNT_VRAM_MAP_1D)
								{
									inc = width >> 2;
								}
								else
								{
									tiledrawindex &= 0x3fe;
								}

								pixx = 0;
								if(hflip)
								{
									pixx = width - 1;
								}

								if(attr0 & OBJ_MOSAIC)
								{
									cury -= (cury % mosaicy);
								}

								address = 0x10000 + ((((tiledrawindex + (cury >> 3) * inc) << 5) + ((cury & 7) << 3) + ((pixx >> 3) << 6) + (pixx & 7)) & 0x7fff);

								if(hflip)
								{
									pixx = 7;
								}

								for(x = 0; x < width; x++)
								{
									if(sx < 240)
									{
										UINT8 color = src[address];
										if(color == 0 && priority < ((scanline[sx] >> 25) & 3))
										{
											scanline[sx] = (scanline[sx] & 0xf9ffffff) | prio;
											if((attr0 & OBJ_MOSAIC) != 0 && mosaiccnt != 0)
											{
												scanline[sx] = (scanline[sx - 1] & 0xf9ffffff) | prio;
											}
										}
										else if(color != 0 && prio < (scanline[sx] & 0xff000000))
										{
											scanline[sx] = palette[256 + color] | prio;
											if((attr0 & OBJ_MOSAIC) != 0 && mosaiccnt != 0)
											{
												scanline[sx] = (scanline[sx - 1] & 0xf9ffffff) | prio;
											}
										}
									}

									if(attr0 & OBJ_MOSAIC)
									{
										mosaiccnt++;
										if(mosaiccnt == mosaicx)
										{
											mosaiccnt = 0;
										}
									}

									sx++;
									sx &= 0x1ff;

									if(hflip)
									{
										pixx--;
										address--;
										if(pixx == -1)
										{
											address -= 56;
											pixx = 7;
										}
										if(address < 0x10000)
										{
											address += 0x8000;
										}
									}
									else
									{
										pixx++;
										address++;
										if(pixx == 8)
										{
											address += 56;
											pixx = 0;
										}
										if(address > 0x17fff)
										{
											address -= 0x8000;
										}
									}
								}
							}
							else
							{
								INT32 pixx;
								INT32 inc = 32;
								UINT32 address = 0;

								if(vflip)
								{
									cury = height - cury - 1;
								}

								if((DISPCNT & DISPCNT_MODE) > 2 && tiledrawindex < 0x200)
								{
									continue;
								}

								if((DISPCNT & DISPCNT_VRAM_MAP) == DISPCNT_VRAM_MAP_1D)
								{
									inc = width >> 3;
								}

								pixx = 0;
								if(hflip)
								{
									pixx = width - 1;
								}

								if(attr0 & OBJ_MOSAIC)
								{
									cury -= (cury % mosaicy);
								}

								address = 0x10000 + ((((tiledrawindex + (cury >> 3) * inc) << 5) + ((cury & 7) << 2) + ((pixx >> 3) << 5) + ((pixx & 7) >> 1)) & 0x7fff);

								if(hflip)
								{
									pixx = 7;
									for(x = width - 1; x >= 0; x--)
									{
										if(sx < 240)
										{
											UINT8 color = src[address];
											UINT8 palentry = (attr2 >> 8) & 0xf0;

											if(x & 1)
											{
												color >>= 4;
											}
											else
											{
												color &= 0x0f;
											}

											if(color == 0 && priority < ((scanline[sx] >> 25) & 3))
											{
												scanline[sx] = (scanline[sx] & 0xf9ffffff) | prio;
												if((attr0 & OBJ_MOSAIC) != 0 && mosaiccnt != 0)
												{
													scanline[sx] = (scanline[sx - 1] & 0xf9ffffff) | prio;
												}
											}
											else if(color != 0 && prio < (scanline[sx] & 0xff000000))
											{
												scanline[sx] = palette[256 + palentry + color] | prio;
												if((attr0 & OBJ_MOSAIC) != 0 && mosaiccnt != 0)
												{
													scanline[sx] = (scanline[sx - 1] & 0xf9ffffff) | prio;
												}
											}
										}

										sx++;
										sx &= 0x1ff;

										pixx--;
										if(!(x & 1))
										{
											address--;
										}
										if(pixx == -1)
										{
											address -= 28;
											pixx = 7;
										}
										if(address < 0x10000)
										{
											address += 0x8000;
										}
									}
								}
								else
								{
									for(x = 0; x < width; x++)
									{
										if(sx < 240)
										{
											UINT8 color = src[address];
											UINT8 palentry = (attr2 >> 8) & 0xf0;

											if(x & 1)
											{
												color >>= 4;
											}
											else
											{
												color &= 0x0f;
											}

											if(color == 0 && priority < ((scanline[sx] >> 25) & 3))
											{
												scanline[sx] = (scanline[sx] & 0xf9ffffff) | prio;
												if((attr0 & OBJ_MOSAIC) != 0 && mosaiccnt != 0)
												{
													scanline[sx] = (scanline[sx - 1] & 0xf9ffffff) | prio;
												}
											}
											else if(color != 0 && prio < (scanline[sx] & 0xff000000))
											{
												scanline[sx] = palette[256 + palentry + color] | prio;
												if((attr0 & OBJ_MOSAIC) != 0 && mosaiccnt != 0)
												{
													scanline[sx] = (scanline[sx - 1] & 0xf9ffffff) | prio;
												}
											}
										}

										if(attr0 & OBJ_MOSAIC)
										{
											mosaiccnt++;
											if(mosaiccnt == mosaicx)
											{
												mosaiccnt = 0;
											}
										}

										sx++;
										sx &= 0x1ff;

										pixx++;
										if(x & 1)
										{
											address++;
										}
										if(pixx == 8)
										{
											address += 28;
											pixx = 0;
										}
										if(address > 0x17fff)
										{
											address -= 0x8000;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

inline int gba_lcd_device::is_in_window(int x, int window)
{
	int x0 = WIN0H >> 8;
	int x1 = WIN0H & 0x00ff;

	if(window == 1)
	{
		x0 = WIN1H >> 8;
		x1 = WIN1H & 0x00ff;
	}

	if(x0 <= x1)
	{
		if(x >= x0 && x < x1)
		{
			return 1;
		}
	}
	else
	{
		if(x >= x0 || x < x1)
		{
			return 1;
		}
	}

	return 0;
}

void gba_lcd_device::draw_scanline(int y)
{
	bitmap_ind16 &bitmap = m_bitmap;
	UINT16 *scanline = &bitmap.pix16(y);
	UINT8 submode = 0;
	int depth = 0;

	// forced blank
	if (DISPCNT & DISPCNT_BLANK)
	{
		// forced blank is white
		for (int x = 0; x < 240; x++)
			scanline[x] = 0x7fff;
		return;
	}

	if(!m_fxOn && !m_windowOn && !(DISPCNT & DISPCNT_OBJWIN_EN))
		submode = GBA_SUBMODE0;
	else if(m_fxOn && !m_windowOn && !(DISPCNT & DISPCNT_OBJWIN_EN))
		submode = GBA_SUBMODE1;
	else
		submode = GBA_SUBMODE2;

	if ((DISPCNT & 7) == 3)
		depth = 16;
	else if ((DISPCNT & 7) == 4)
		depth = 8;
	else if ((DISPCNT & 7) == 5)
		depth = 4;

	//printf("mode = %d, %d\n", DISPCNT & 7, submode);

	switch(DISPCNT & 7)
	{
		case 0:
		case 1:
		case 2:
			draw_modes(DISPCNT & 7, submode, y, &m_xferscan[0][1024], &m_xferscan[1][1024], &m_xferscan[2][1024], &m_xferscan[3][1024], &m_xferscan[4][1024], &m_xferscan[5][1024], &m_xferscan[6][1024], depth);
			break;
		case 3:
		case 4:
		case 5:
			draw_modes(GBA_MODE345, submode, y, &m_xferscan[0][1024], &m_xferscan[1][1024], &m_xferscan[2][1024], &m_xferscan[3][1024], &m_xferscan[4][1024], &m_xferscan[5][1024], &m_xferscan[6][1024], depth);
			break;
		default:
			fatalerror("Invalid screen mode (6 or 7)!\n");
	}

	for (int x = 0; x < 240; x++)
	{
		scanline[x] = m_xferscan[6][1024 + x] & 0x7fff;
	}
}

static const char *reg_names[] = {
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
	UINT32 retval = 0;

	switch( offset )
	{
		case 0x0004/4:
			retval = (DISPSTAT & 0xffff) | (machine().first_screen()->vpos()<<16);
			break;
		default:
			if( ACCESSING_BITS_0_15 )
			{
				retval |= m_regs[offset] & 0x0000ffff;
			}
			if( ACCESSING_BITS_16_31 )
			{
				retval |= m_regs[offset] & 0xffff0000;
			}
			break;
	}

	assert_always(offset < ARRAY_LENGTH(reg_names) / 2, "Not enough register names in gba_lcd_device");

	if (ACCESSING_BITS_0_15)
	{
		verboselog(*this, 2, "GBA I/O Read: %s = %04x\n", reg_names[offset * 2], retval & 0x0000ffff);
	}
	if (ACCESSING_BITS_16_31)
	{
		verboselog(*this, 2, "GBA I/O Read: %s = %04x\n", reg_names[offset * 2 + 1], (retval & 0xffff0000) >> 16);
	}

	return retval;
}

WRITE32_MEMBER(gba_lcd_device::video_w)
{
	COMBINE_DATA(&m_regs[offset]);

	assert_always(offset < ARRAY_LENGTH(reg_names) / 2, "Not enough register names in gba_lcd_device");

	if (ACCESSING_BITS_0_15)
	{
		verboselog(*this, 2, "GBA I/O Write: %s = %04x\n", reg_names[offset * 2], data & 0x0000ffff);
	}
	if (ACCESSING_BITS_16_31)
	{
		verboselog(*this, 2, "GBA I/O Write: %s = %04x\n", reg_names[offset * 2 + 1], (data & 0xffff0000) >> 16);
	}

	switch( offset )
	{
		case 0x0000/4:
			if( ACCESSING_BITS_0_15 )
			{
				if(DISPCNT & (DISPCNT_WIN0_EN | DISPCNT_WIN1_EN))
				{
					m_windowOn = 1;
				}
				else
				{
					m_windowOn = 0;
				}
			}
			break;
		case 0x0028/4:
			m_gfxBG2Changed |= 1;
			break;
		case 0x002c/4:
			m_gfxBG2Changed |= 2;
			break;
		case 0x0038/4:
			m_gfxBG3Changed |= 1;
			break;
		case 0x003c/4:
			m_gfxBG3Changed |= 2;
			break;
		case 0x0050/4:
			if( ACCESSING_BITS_0_15 )
			{
				if(BLDCNT & BLDCNT_SFX)
				{
					m_fxOn = 1;
				}
				else
				{
					m_fxOn = 0;
				}
			}
			break;
	}
}

static inline UINT32 combine_data_32_16(UINT32 prev, UINT32 data, UINT32 mem_mask)
{
	COMBINE_DATA(&prev);
	switch(mem_mask)
	{
		case 0x000000ff:
			prev &= 0xffff00ff;
			prev |= data << 8;
			break;
		case 0x0000ff00:
			prev &= 0xffffff00;
			prev |= data >> 8;
			break;
		case 0x00ff0000:
			prev &= 0x00ffffff;
			prev |= data << 8;
			break;
		case 0xff000000:
			prev &= 0xff00ffff;
			prev |= data >> 8;
			break;
		default:
			break;
	}
	return prev;
}

READ32_MEMBER(gba_lcd_device::gba_pram_r)
{
	return m_pram[offset];
}

WRITE32_MEMBER(gba_lcd_device::gba_pram_w)
{
	m_pram[offset] = combine_data_32_16(m_pram[offset], data, mem_mask);
}

READ32_MEMBER(gba_lcd_device::gba_vram_r)
{
	return m_vram[offset];
}

WRITE32_MEMBER(gba_lcd_device::gba_vram_w)
{
	m_vram[offset] = combine_data_32_16(m_vram[offset], data, mem_mask);
}

READ32_MEMBER(gba_lcd_device::gba_oam_r)
{
	return m_oam[offset];
}

WRITE32_MEMBER(gba_lcd_device::gba_oam_w)
{
	m_oam[offset] = combine_data_32_16(m_oam[offset], data, mem_mask);
}

TIMER_CALLBACK_MEMBER(gba_lcd_device::perform_hbl)
{
	int scanline = machine().first_screen()->vpos();

	// draw only visible scanlines
	if (scanline < 160)
	{
		draw_scanline(scanline);

		machine().driver_data<gba_state>()->request_dma(gba_state::dma_start_timing::hblank);
	}

	if ((DISPSTAT & DISPSTAT_HBL_IRQ_EN ) != 0)
	{
		machine().driver_data<gba_state>()->request_irq(INT_HBL);
	}

	DISPSTAT_SET(DISPSTAT_HBL);

	m_hbl_timer->adjust(attotime::never);
}

TIMER_CALLBACK_MEMBER(gba_lcd_device::perform_scan)
{
	// clear hblank and raster IRQ flags
	DISPSTAT_RESET(DISPSTAT_HBL|DISPSTAT_VCNT);

	int scanline = machine().first_screen()->vpos();

	// VBL is set for scanlines 160 through 226 (but not 227, which is the last line)
	if (scanline >= 160 && scanline < 227)
	{
		DISPSTAT_SET(DISPSTAT_VBL);

		// VBL IRQ and DMA on line 160
		if (scanline == 160)
		{
			if (DISPSTAT & DISPSTAT_VBL_IRQ_EN)
			{
				machine().driver_data<gba_state>()->request_irq(INT_VBL);
			}

			machine().driver_data<gba_state>()->request_dma(gba_state::dma_start_timing::vblank);
		}
	}
	else
	{
		DISPSTAT_RESET(DISPSTAT_VBL);
	}

	// handle VCNT match interrupt/flag
	if (scanline == ((DISPSTAT >> 8) & 0xff))
	{
		DISPSTAT_SET(DISPSTAT_VCNT);

		if (DISPSTAT & DISPSTAT_VCNT_IRQ_EN)
		{
			machine().driver_data<gba_state>()->request_irq(INT_VCNT);
		}
	}

	m_hbl_timer->adjust(machine().first_screen()->time_until_pos(scanline, 240));
	m_scan_timer->adjust(machine().first_screen()->time_until_pos(( scanline + 1 ) % 228, 0));
}

PALETTE_INIT_MEMBER(gba_lcd_device, gba)
{
	for( UINT8 b = 0; b < 32; b++ )
	{
		for( UINT8 g = 0; g < 32; g++ )
		{
			for( UINT8 r = 0; r < 32; r++ )
			{
				palette.set_pen_color( ( b << 10 ) | ( g << 5 ) | r, pal5bit(r), pal5bit(g), pal5bit(b) );
			}
		}
	}
}

UINT32 gba_lcd_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}

void gba_lcd_device::device_start()
{
	m_pram = make_unique_clear<UINT32[]>(0x400 / 4);
	m_vram = make_unique_clear<UINT32[]>(0x18000 / 4);
	m_oam = make_unique_clear<UINT32[]>(0x400 / 4);

	save_pointer(NAME(m_pram.get()), 0x400 / 4);
	save_pointer(NAME(m_vram.get()), 0x18000 / 4);
	save_pointer(NAME(m_oam.get()), 0x400 / 4);

	m_screen->register_screen_bitmap(m_bitmap);

	/* create a timer to fire scanline functions */
	m_scan_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(gba_lcd_device::perform_scan),this));
	m_hbl_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(gba_lcd_device::perform_hbl),this));
	m_scan_timer->adjust(machine().first_screen()->time_until_pos(0, 0));

	save_item(NAME(m_windowOn));
	save_item(NAME(m_fxOn));
	save_item(NAME(m_gfxBG2Changed));
	save_item(NAME(m_gfxBG3Changed));
	save_item(NAME(m_gfxBG2X));
	save_item(NAME(m_gfxBG2Y));
	save_item(NAME(m_gfxBG3X));
	save_item(NAME(m_gfxBG3Y));
	save_item(NAME(m_xferscan));
}

void gba_lcd_device::device_reset()
{
	memset(m_regs, 0, sizeof(m_regs));

	m_gfxBG2Changed = 0;
	m_gfxBG3Changed = 0;
	m_gfxBG2X = 0;
	m_gfxBG2Y = 0;
	m_gfxBG3X = 0;
	m_gfxBG3Y = 0;
	m_windowOn = 0;
	m_fxOn = 0;

	m_scan_timer->adjust(machine().first_screen()->time_until_pos(0, 0));
	m_hbl_timer->adjust(attotime::never);
}

static MACHINE_CONFIG_FRAGMENT( gba_lcd )
	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_RAW_PARAMS(XTAL_16_777216MHz/4, 308, 0, 240, 228, 0, 160)
	MCFG_SCREEN_UPDATE_DEVICE(DEVICE_SELF, gba_lcd_device, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_DEFAULT_LAYOUT(layout_lcd)
	MCFG_PALETTE_ADD("palette", 32768)
	MCFG_PALETTE_INIT_OWNER(gba_lcd_device, gba)
MACHINE_CONFIG_END

machine_config_constructor gba_lcd_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( gba_lcd );
}
