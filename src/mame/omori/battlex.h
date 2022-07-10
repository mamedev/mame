// license:BSD-3-Clause
// copyright-holders:David Haywood
/***************************************************************************

    Battle Cross

***************************************************************************/

#include "emupal.h"
#include "tilemap.h"

class battlex_state : public driver_device
{
public:
	battlex_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	void init_battlex();
	void dodgeman(machine_config &config);
	void battlex(machine_config &config);
	DECLARE_CUSTOM_INPUT_MEMBER(battlex_in0_b4_r);

private:
	uint8_t m_in0_b4 = 0U;

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;

	/* video-related */
	tilemap_t *m_bg_tilemap = nullptr;
	uint8_t m_scroll_lsb = 0U;
	uint8_t m_scroll_msb = 0U;
	uint8_t m_starfield_enabled = 0U;
	void battlex_palette_w(offs_t offset, uint8_t data);
	void battlex_scroll_x_lsb_w(uint8_t data);
	void battlex_scroll_x_msb_w(uint8_t data);
	void battlex_scroll_starfield_w(uint8_t data);
	void battlex_videoram_w(offs_t offset, uint8_t data);
	void battlex_flipscreen_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_battlex(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(battlex_interrupt);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	DECLARE_VIDEO_START(dodgeman);
	TILE_GET_INFO_MEMBER(get_dodgeman_bg_tile_info);
	void battlex_map(address_map &map);
	void dodgeman_io_map(address_map &map);
	void io_map(address_map &map);
};
