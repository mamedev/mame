// license:BSD-3-Clause
// copyright-holders:Paul Leaman

#include "machine/timer.h"
#include "video/bufsprite.h"
#include "emupal.h"
#include "tilemap.h"

class srumbler_state : public driver_device
{
public:
	srumbler_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_spriteram(*this,"spriteram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_backgroundram(*this, "backgroundram"),
		m_foregroundram(*this, "foregroundram") { }

	void srumbler(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<buffered_spriteram8_device> m_spriteram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_backgroundram;
	required_shared_ptr<uint8_t> m_foregroundram;

	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_fg_tilemap = nullptr;
	int m_scroll[4]{};

	void bankswitch_w(uint8_t data);
	void foreground_w(offs_t offset, uint8_t data);
	void background_w(offs_t offset, uint8_t data);
	void _4009_w(uint8_t data);
	void scroll_w(offs_t offset, uint8_t data);

	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	virtual void machine_start() override;
	virtual void video_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	TIMER_DEVICE_CALLBACK_MEMBER(interrupt);
	void srumbler_map(address_map &map);
	void srumbler_sound_map(address_map &map);
};
