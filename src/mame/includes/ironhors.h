// license:BSD-3-Clause
// copyright-holders:Mirko Buffoni, Couriersud
/*************************************************************************

    IronHorse

*************************************************************************/
#ifndef MAME_INCLUDES_IRONHORS_H
#define MAME_INCLUDES_IRONHORS_H

#pragma once

#include "machine/gen_latch.h"
#include "machine/timer.h"
#include "sound/discrete.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class ironhors_base_state : public driver_device
{
public:
	ironhors_base_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_screen(*this, "screen"),
		m_soundlatch(*this, "soundlatch"),
		m_disc_ih(*this, "disc_ih"),
		m_interrupt_enable(*this, "int_enable"),
		m_scroll(*this, "scroll"),
		m_colorram(*this, "colorram"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram%u", 1U)
	{ }

	void base(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	void sh_irqtrigger_w(uint8_t data);
	void videoram_w(offs_t offset, uint8_t data);
	void colorram_w(offs_t offset, uint8_t data);
	void charbank_w(uint8_t data);
	void palettebank_w(uint8_t data);
	void flipscreen_w(uint8_t data);
	void filter_w(uint8_t data);

	void palette(palette_device &palette) const;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<discrete_device> m_disc_ih;

	// memory pointers
	required_shared_ptr<uint8_t> m_interrupt_enable;
	required_shared_ptr<uint8_t> m_scroll;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr_array<uint8_t, 2> m_spriteram;

	// video-related
	tilemap_t *m_bg_tilemap = nullptr;
	uint8_t m_palettebank = 0U;
	uint8_t m_charbank = 0U;
	uint8_t m_spriterambank = 0U;
};

class ironhors_state : public ironhors_base_state
{
public:
	ironhors_state(const machine_config &mconfig, device_type type, const char *tag) :
		ironhors_base_state(mconfig, type, tag)
	{ }

	void ironhors(machine_config &config);

protected:
	virtual void video_start() override;

private:
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	TIMER_DEVICE_CALLBACK_MEMBER(scanline_tick);

	void master_map(address_map &map);
	void slave_map(address_map &map);
	void slave_io_map(address_map &map);

	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
};

class farwest_state : public ironhors_base_state
{
public:
	farwest_state(const machine_config &mconfig, device_type type, const char *tag) :
		ironhors_base_state(mconfig, type, tag)
	{ }

	void farwest(machine_config &config);

protected:
	virtual void video_start() override;

private:
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	TIMER_DEVICE_CALLBACK_MEMBER(scanline_tick);

	void master_map(address_map &map);
	void slave_map(address_map &map);

	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
};

#endif // MAME_INCLUDES_IRONHORS_H
