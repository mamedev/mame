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
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	uint8_t m_bg_xscroll[2];
	int m_flipscreen;
	int m_video_enable;

	// common
	DECLARE_WRITE8_MEMBER(bg_ram_w);
	DECLARE_WRITE8_MEMBER(fg_ram_w);
	DECLARE_WRITE8_MEMBER(bg_yscroll_w);
	DECLARE_WRITE8_MEMBER(bg_xscroll_w);
	DECLARE_WRITE8_MEMBER(screen_ctrl_w);
	DECLARE_WRITE8_MEMBER(paletteram_xBGR_RRRR_GGGG_BBBB_w);
	DECLARE_WRITE8_MEMBER(rombank_w);

	// game specific
	DECLARE_WRITE8_MEMBER(gekiretu_rombank_w);

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
