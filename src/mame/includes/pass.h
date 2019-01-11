// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_INCLUDES_PASS_H
#define MAME_INCLUDES_PASS_H

#pragma once

class pass_state : public driver_device
{
public:
	pass_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_bg_tilemap(nullptr),
		m_fg_tilemap(nullptr),
		m_bg_videoram(*this, "bg_videoram"),
		m_fg_videoram(*this, "fg_videoram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode")
	{
	}

	void pass(machine_config &config);

protected:
	virtual void video_start() override;

private:
	DECLARE_WRITE16_MEMBER(pass_bg_videoram_w);
	DECLARE_WRITE16_MEMBER(pass_fg_videoram_w);
	TILE_GET_INFO_MEMBER(get_pass_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_pass_fg_tile_info);

	uint32_t screen_update_pass(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void pass_map(address_map &map);
	void pass_sound_io_map(address_map &map);
	void pass_sound_map(address_map &map);

	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;

	required_shared_ptr<uint16_t> m_bg_videoram;
	required_shared_ptr<uint16_t> m_fg_videoram;
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
};

#endif // MAME_INCLUDES_PASS_H
