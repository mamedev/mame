// license:BSD-3-Clause
// copyright-holders:Allard van der Bas
#ifndef MAME_INCLUDES_VASTAR_H
#define MAME_INCLUDES_VASTAR_H

#pragma once

#include "emupal.h"
#include "tilemap.h"

class vastar_common_state : public driver_device
{
public:
	vastar_common_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_sharedram(*this, "sharedram")
	{ }

	void common(machine_config &config);

	INTERRUPT_GEN_MEMBER(vblank_irq);

protected:
	virtual void machine_start() override;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;

	required_shared_ptr<uint8_t> m_sharedram;

	uint8_t m_nmi_mask = 0;

	DECLARE_WRITE_LINE_MEMBER(nmi_mask_w);

	void cpu2_map(address_map &map);
	void cpu2_port_map(address_map &map);
	void main_port_map(address_map &map);
};

class vastar_state : public vastar_common_state
{
public:
	vastar_state(const machine_config &mconfig, device_type type, const char *tag) :
		vastar_common_state(mconfig, type, tag),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_bg1videoram(*this, "bg1videoram"),
		m_bg2videoram(*this, "bg2videoram"),
		m_fgvideoram(*this, "fgvideoram"),
		m_sprite_priority(*this, "sprite_priority")
	{ }

	void vastar(machine_config &config);

protected:
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_bg1videoram;
	required_shared_ptr<uint8_t> m_bg2videoram;
	required_shared_ptr<uint8_t> m_fgvideoram;
	required_shared_ptr<uint8_t> m_sprite_priority;

	// these are pointers into m_fgvideoram
	uint8_t* m_bg1_scroll = nullptr;
	uint8_t* m_bg2_scroll = nullptr;
	uint8_t* m_spriteram1 = nullptr;
	uint8_t* m_spriteram2 = nullptr;
	uint8_t* m_spriteram3 = nullptr;

	tilemap_t *m_fg_tilemap = nullptr;
	tilemap_t *m_bg1_tilemap = nullptr;
	tilemap_t *m_bg2_tilemap = nullptr;

	DECLARE_WRITE_LINE_MEMBER(flip_screen_w);
	void fgvideoram_w(offs_t offset, uint8_t data);
	void bg1videoram_w(offs_t offset, uint8_t data);
	void bg2videoram_w(offs_t offset, uint8_t data);

	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg1_tile_info);
	TILE_GET_INFO_MEMBER(get_bg2_tile_info);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);

	void main_map(address_map &map);
};

class dogfightp_state : public vastar_common_state
{
public:
	dogfightp_state(const machine_config &mconfig, device_type type, const char *tag) :
		vastar_common_state(mconfig, type, tag)
	{ }

	void dogfightp(machine_config &config);

private:
	void dogfightp_main_map(address_map &map);
};

#endif // MAME_INCLUDES_VASTAR_H
