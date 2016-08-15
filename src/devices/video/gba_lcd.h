// license:BSD-3-Clause
// copyright-holders:R. Belmont,Ryan Holtz
/***************************************************************************

    gba_lcd.h

    File to handle emulation of the video hardware of the Game Boy Advance

    By R. Belmont, Ryan Holtz

***************************************************************************/

#pragma once

#ifndef __GBA_LCD_H__
#define __GBA_LCD_H__

#include "emu.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
extern const device_type GBA_LCD;


//**************************************************************************
//  DEVICE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_GBA_LCD_ADD(_tag) \
		MCFG_DEVICE_ADD(_tag, GBA_LCD, 0)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class gba_lcd_device :  public device_t,
						public device_video_interface
{
public:
	gba_lcd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_READ32_MEMBER(video_r);
	DECLARE_WRITE32_MEMBER(video_w);
	DECLARE_READ32_MEMBER(gba_pram_r);
	DECLARE_WRITE32_MEMBER(gba_pram_w);
	DECLARE_READ32_MEMBER(gba_vram_r);
	DECLARE_WRITE32_MEMBER(gba_vram_w);
	DECLARE_READ32_MEMBER(gba_oam_r);
	DECLARE_WRITE32_MEMBER(gba_oam_w);
	DECLARE_PALETTE_INIT(gba);
	TIMER_CALLBACK_MEMBER(perform_hbl);
	TIMER_CALLBACK_MEMBER(perform_scan);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

private:
	inline void update_mask(UINT8* mask, int mode, int submode, UINT32* obj_win, UINT8 inwin0, UINT8 inwin1, UINT8 in0_mask, UINT8 in1_mask, UINT8 out_mask);
	void draw_modes(int mode, int submode, int y, UINT32* line0, UINT32* line1, UINT32* line2, UINT32* line3, UINT32* lineOBJ, UINT32* lineOBJWin, UINT32* lineMix, int bpp);
	void draw_roz_bitmap_scanline(UINT32 *scanline, int ypos, UINT32 enablemask, UINT32 ctrl, INT32 X, INT32 Y, INT32 PA, INT32 PB, INT32 PC, INT32 PD, INT32 *currentx, INT32 *currenty, int changed, int depth);
	void draw_roz_scanline(UINT32 *scanline, int ypos, UINT32 enablemask, UINT32 ctrl, INT32 X, INT32 Y, INT32 PA, INT32 PB, INT32 PC, INT32 PD, INT32 *currentx, INT32 *currenty, int changed);
	void draw_bg_scanline(UINT32 *scanline, int ypos, UINT32 enablemask, UINT32 ctrl, UINT32 hofs, UINT32 vofs);
	void draw_gba_oam_window(UINT32 *scanline, int y);
	void draw_gba_oam(UINT32 *scanline, int y);
	inline int is_in_window(int x, int window);
	void draw_scanline(int y);

	std::unique_ptr<UINT32[]> m_pram;
	std::unique_ptr<UINT32[]> m_vram;
	std::unique_ptr<UINT32[]> m_oam;

	emu_timer *m_scan_timer, *m_hbl_timer;

	bitmap_ind16 m_bitmap;

	UINT32 m_regs[0x60 / 4];

	UINT8  m_windowOn;
	UINT8  m_fxOn;
	UINT8  m_gfxBG2Changed;
	UINT8  m_gfxBG3Changed;
	INT32  m_gfxBG2X;
	INT32  m_gfxBG2Y;
	INT32  m_gfxBG3X;
	INT32  m_gfxBG3Y;

	UINT32 m_xferscan[7][240+2048];
};

#endif /* GBA_LCD_H_ */
