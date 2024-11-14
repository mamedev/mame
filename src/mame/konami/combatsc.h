// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino, Manuel Abadia
/*************************************************************************

    Combat School

*************************************************************************/
#ifndef MAME_KONAMI_COMBATSC_H
#define MAME_KONAMI_COMBATSC_H

#pragma once

#include "machine/gen_latch.h"
#include "k007452.h"
#include "sound/msm5205.h"
#include "sound/upd7759.h"
#include "k007121.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class combatsc_base_state : public driver_device
{
public:
	combatsc_base_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram%u", 0U),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_track_ports(*this, {"TRACK0_Y", "TRACK0_X", "TRACK1_Y", "TRACK1_X"}),
		m_mainbank(*this, "mainbank"),
		m_video_view(*this, "video_view")
	{
	}

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	// memory pointers
	required_shared_ptr_array<uint8_t, 2> m_videoram;
	std::unique_ptr<uint8_t[]> m_spriteram[2];

	// video-related
	tilemap_t *m_bg_tilemap[2]{};
	tilemap_t *m_textlayer = nullptr;
	uint8_t m_priority = 0U;

	uint8_t m_vreg = 0U;
	uint8_t m_video_circuit = 0U; // 0 or 1

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	optional_ioport_array<4> m_track_ports;

	required_memory_bank m_mainbank;
	memory_view m_video_view;

	void vreg_w(uint8_t data);
	void videoview0_w(offs_t offset, uint8_t data);
	void videoview1_w(offs_t offset, uint8_t data);
};

class combatsc_state : public combatsc_base_state
{
public:
	combatsc_state(const machine_config &mconfig, device_type type, const char *tag) :
		combatsc_base_state(mconfig, type, tag),
		m_k007121(*this, "k007121_%u", 1U),
		m_k007452(*this, "k007452"),
		m_upd7759(*this, "upd"),
		m_scrollram(*this, "scrollram%u", 0U),
		m_scroll_view(*this, "scrollview")
	{
	}

	void combatsc(machine_config &config);

	void init_combatsc();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device_array<k007121_device, 2> m_k007121;
	required_device<k007452_device> m_k007452;
	required_device<upd7759_device> m_upd7759;
	required_shared_ptr_array<uint8_t, 2> m_scrollram;
	memory_view m_scroll_view;

	bool m_textflip = false;

	// misc
	uint8_t m_pos[4]{};
	uint8_t m_sign[4]{};

	void bankselect_w(uint8_t data);
	void coin_counter_w(uint8_t data);
	uint8_t trackball_r(offs_t offset);
	uint8_t unk_r();
	void sh_irqtrigger_w(uint8_t data);
	void pf_control_w(offs_t offset, uint8_t data);
	uint8_t busy_r();
	void play_w(uint8_t data);
	void voice_reset_w(uint8_t data);
	void portA_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_tile_info0);
	TILE_GET_INFO_MEMBER(get_tile_info1);
	TILE_GET_INFO_MEMBER(get_text_info);
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, const uint8_t *source, int circuit, bitmap_ind8 &priority_bitmap, uint32_t pri_mask);
	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};

class combatscb_state : public combatsc_base_state
{
public:
	combatscb_state(const machine_config &mconfig, device_type type, const char *tag) :
		combatsc_base_state(mconfig, type, tag),
		m_gfxdecode(*this, "gfxdecode"),
		m_msm(*this, "msm"),
		m_soundbank(*this, "soundbank"),
		m_io_ram(*this, "io_ram", 0x4000, ENDIANNESS_BIG),
		m_bank_io_view(*this, "bank_io_view")
	{
	}

	void combatscb(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	optional_device<gfxdecode_device> m_gfxdecode;
	required_device<msm5205_device> m_msm;
	required_memory_bank m_soundbank;
	memory_share_creator<uint8_t> m_io_ram;
	memory_view m_bank_io_view;

	uint8_t m_bank_select = 0; // 0x00..0x1f

	void priority_w(uint8_t data);
	void io_w(offs_t offset, uint8_t data);
	void bankselect_w(uint8_t data);
	void msm_w(uint8_t data);
	void sound_irq_ack(uint8_t data);
	TILE_GET_INFO_MEMBER(get_tile_info0);
	TILE_GET_INFO_MEMBER(get_tile_info1);
	TILE_GET_INFO_MEMBER(get_text_info);
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, const uint8_t *source, int circuit);
	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};

#endif // MAME_KONAMI_COMBATSC_H
