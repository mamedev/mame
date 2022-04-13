// license:BSD-3-Clause
// copyright-holders:Uki

#include "emupal.h"
#include "tilemap.h"

class quizdna_state : public driver_device
{
public:
	quizdna_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_spriteram(*this, "spriteram"),
		m_generic_paletteram_8(*this, "paletteram") { }

	void gakupara(machine_config &config);
	void quizdna(machine_config &config);
	void gekiretu(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_generic_paletteram_8;

	std::unique_ptr<uint8_t[]> m_bg_ram;
	std::unique_ptr<uint8_t[]> m_fg_ram;
	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_fg_tilemap = nullptr;
	uint8_t m_bg_xscroll[2];
	int m_flipscreen = 0;
	int m_video_enable = 0;

	// common
	void bg_ram_w(offs_t offset, uint8_t data);
	void fg_ram_w(offs_t offset, uint8_t data);
	void bg_yscroll_w(uint8_t data);
	void bg_xscroll_w(offs_t offset, uint8_t data);
	void screen_ctrl_w(uint8_t data);
	void paletteram_xBGR_RRRR_GGGG_BBBB_w(offs_t offset, uint8_t data);
	void rombank_w(uint8_t data);

	// game specific
	void gekiretu_rombank_w(uint8_t data);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);

	virtual void machine_start() override;
	virtual void video_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void gakupara_io_map(address_map &map);
	void gekiretu_io_map(address_map &map);
	void gekiretu_map(address_map &map);
	void quizdna_io_map(address_map &map);
	void quizdna_map(address_map &map);
};
