// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    Green Beret

***************************************************************************/
#ifndef MAME_INCLUDES_GBERET_H
#define MAME_INCLUDES_GBERET_H

#pragma once

#include "machine/timer.h"
#include "sound/sn76496.h"
#include "emupal.h"
#include "tilemap.h"

class gberet_base_state : public driver_device
{
public:
	gberet_base_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_colorram(*this, "colorram"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_sn(*this, "snsnd")
	{ }

protected:
	virtual void video_start() override;

	// memory pointers
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<sn76489a_device> m_sn;

	// video-related
	tilemap_t * m_bg_tilemap;

	// misc
	void videoram_w(offs_t offset, uint8_t data);
	void colorram_w(offs_t offset, uint8_t data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	void palette(palette_device &palette) const; // TODO: move down in gberet_state once the bootleg PROMs decoding is done
};

class gberet_state : public gberet_base_state
{
public:
	gberet_state(const machine_config &mconfig, device_type type, const char *tag) :
		gberet_base_state(mconfig, type, tag),
		m_spriteram2(*this, "spriteram2"),
		m_scrollram(*this, "scrollram"),
		m_soundlatch(*this, "soundlatch")
	{ }

	void gberet(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	void prg_map(address_map &map);

private:
	// memory pointers
	required_shared_ptr<uint8_t> m_spriteram2;
	required_shared_ptr<uint8_t> m_scrollram;
	required_shared_ptr<uint8_t> m_soundlatch;

	// video-related
	uint8_t m_spritebank;

	// misc
	uint8_t m_interrupt_mask;
	uint8_t m_interrupt_ticks;
	void coin_counter_w(uint8_t data);
	void flipscreen_w(uint8_t data);
	void sound_w(uint8_t data);
	void scroll_w(offs_t offset, uint8_t data);
	void sprite_bank_w(uint8_t data);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(interrupt_tick);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
};

class mrgoemon_state : public gberet_state
{
public:
	mrgoemon_state(const machine_config &mconfig, device_type type, const char *tag) :
		gberet_state(mconfig, type, tag),
		m_mainbank(*this, "mainbank")
	{ }

	void mrgoemon(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	// memory pointers
	required_memory_bank m_mainbank;

	void coin_counter_w(uint8_t data);
	void prg_map(address_map &map);
};

class gberetb_state : public gberet_base_state
{
public:
	gberetb_state(const machine_config &mconfig, device_type type, const char *tag) :
		gberet_base_state(mconfig, type, tag)
	{ }

	void gberetb(machine_config &config);

private:
	void flipscreen_w(uint8_t data);
	uint8_t irq_ack_r();
	void nmi_ack_w(uint8_t data);
	void scroll_w(offs_t offset, uint8_t data);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void prg_map(address_map &map);
};

#endif // MAME_INCLUDES_GBERET_H
