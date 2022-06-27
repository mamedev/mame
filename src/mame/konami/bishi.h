// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*************************************************************************

    Bishi Bashi Champ Mini Game Senshuken

*************************************************************************/
#ifndef MAME_INCLUDES_BISHI_H
#define MAME_INCLUDES_BISHI_H

#pragma once

#include "machine/timer.h"
#include "k054156_k054157_k056832.h"
#include "k055555.h"
#include "k054338.h"
#include "konami_helper.h"
#include "emupal.h"
#include "screen.h"

#define CPU_CLOCK       (XTAL(24'000'000) / 2)        /* 68000 clock */
#define SOUND_CLOCK     XTAL(16'934'400)     /* YMZ280 clock */

class bishi_state : public driver_device
{
public:
	bishi_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_k056832(*this, "k056832"),
		m_k054338(*this, "k054338"),
		m_k055555(*this, "k055555"),
		m_palette(*this, "palette"),
		m_screen(*this, "screen")
	{ }

	void bishi(machine_config &config);

private:
	uint16_t control_r();
	void control_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void control2_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t bishi_mirror_r(offs_t offset);
	uint16_t bishi_K056832_rom_r(offs_t offset);
	uint32_t screen_update_bishi(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(bishi_scanline);
	K056832_CB_MEMBER(tile_callback);

	void main_map(address_map &map);
protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
private:
	/* misc */
	uint16_t     m_cur_control = 0U;
	uint16_t     m_cur_control2 = 0U;

	/* video-related */
	int        m_layer_colorbase[4]{};

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<k056832_device> m_k056832;
	required_device<k054338_device> m_k054338;
	required_device<k055555_device> m_k055555;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
};

#endif // MAME_INCLUDES_BISHI_H
