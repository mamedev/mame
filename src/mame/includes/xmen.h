// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef MAME_INCLUDES_XMEN_H
#define MAME_INCLUDES_XMEN_H

#pragma once

#include "machine/gen_latch.h"
#include "machine/timer.h"
#include "sound/k054539.h"
#include "video/k053246_k053247_k055673.h"
#include "video/k053251.h"
#include "video/k052109.h"
#include "video/konami_helper.h"
#include "machine/k054321.h"
#include "screen.h"

class xmen_state : public driver_device
{
public:
	xmen_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_xmen6p_spriteram(*this, "spriteram%u", 0),
		m_xmen6p_tilemap(*this, "tilemap%u", 0),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_k054539(*this, "k054539"),
		m_k052109(*this, "k052109"),
		m_k053246(*this, "k053246"),
		m_k053251(*this, "k053251"),
		m_screen(*this, "screen"),
		m_k054321(*this, "k054321"),
		m_z80bank(*this, "z80bank")
	{ }

	void xmen(machine_config &config);
	void xmen6p(machine_config &config);

	DECLARE_READ_LINE_MEMBER(xmen_frame_r);

private:
	/* video-related */
	int        m_layer_colorbase[3]{};
	int        m_sprite_colorbase = 0;
	int        m_layerpri[3]{};

	/* for xmen6p */
	std::unique_ptr<bitmap_ind16> m_screen_right;
	std::unique_ptr<bitmap_ind16> m_screen_left;
	optional_shared_ptr_array<uint16_t, 2> m_xmen6p_spriteram;
	optional_shared_ptr_array<uint16_t, 4> m_xmen6p_tilemap;
	uint16_t * m_k053247_ram;
	bool       m_xmen6p_tilemap_select;

	/* misc */
	uint8_t    m_vblank_irq_mask = 0;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<k054539_device> m_k054539;
	required_device<k052109_device> m_k052109;
	required_device<k053247_device> m_k053246;
	required_device<k053251_device> m_k053251;
	required_device<screen_device> m_screen;
	required_device<k054321_device> m_k054321;

	required_memory_bank m_z80bank;
	void eeprom_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void xmen_18fa00_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void sound_bankswitch_w(uint8_t data);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	DECLARE_VIDEO_START(xmen6p);
	uint32_t screen_update_xmen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_xmen6p_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_xmen6p_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank_xmen6p);
	TIMER_DEVICE_CALLBACK_MEMBER(xmen_scanline);
	K052109_CB_MEMBER(tile_callback);
	K053246_CB_MEMBER(sprite_callback);

	void _6p_main_map(address_map &map);
	void main_map(address_map &map);
	void sound_map(address_map &map);
};

#endif // MAME_INCLUDES_XMEN_H
