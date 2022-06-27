// license:BSD-3-Clause
// copyright-holders:Jarek Parchanski

#include "machine/bankdev.h"
#include "machine/timer.h"
#include "video/jalblend.h"
#include "emupal.h"
#include "tilemap.h"

class psychic5_state : public driver_device
{
public:
	psychic5_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_vrambank(*this, "vrambank"),
		m_blend(*this, "blend"),
		m_spriteram(*this, "spriteram"),
		m_fg_videoram(*this, "fg_videoram"),
		m_bg_videoram(*this, "bg_videoram"),
		m_bg_control(*this, "bg_control"),
		m_ps5_palette_ram_bg(*this, "palette_ram_bg"),
		m_ps5_palette_ram_sp(*this, "palette_ram_sp"),
		m_ps5_palette_ram_tx(*this, "palette_ram_tx")

	{ }

	void psychic5(machine_config &config);
	void bombsa(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_device<address_map_bank_device> m_vrambank;
	optional_device<jaleco_blend_device> m_blend;

	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_fg_videoram;
	required_shared_ptr<uint8_t> m_bg_videoram;
	required_shared_ptr<uint8_t> m_bg_control;
	required_shared_ptr<uint8_t> m_ps5_palette_ram_bg;
	required_shared_ptr<uint8_t> m_ps5_palette_ram_sp;
	required_shared_ptr<uint8_t> m_ps5_palette_ram_tx;

	uint8_t m_bank_latch = 0;
	uint8_t m_ps5_vram_page = 0;
	uint8_t m_bg_clip_mode = 0;
	uint8_t m_title_screen = 0;
	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_fg_tilemap = nullptr;
	uint16_t m_palette_intensity = 0;
	uint8_t m_bombsa_unknown = 0;
	int m_sx1 = 0;
	int m_sy1 = 0;
	int m_sy2 = 0;

	uint8_t bankselect_r();
	uint8_t vram_page_select_r();
	void vram_page_select_w(uint8_t data);
	void fg_videoram_w(offs_t offset, uint8_t data);
	void bg_videoram_w(offs_t offset, uint8_t data);
	void sprite_col_w(offs_t offset, uint8_t data);
	void bg_col_w(offs_t offset, uint8_t data);
	void tx_col_w(offs_t offset, uint8_t data);

	/* psychic5 specific */
	void psychic5_coin_counter_w(uint8_t data);
	void psychic5_bankselect_w(uint8_t data);
	void psychic5_title_screen_w(uint8_t data);

	/* bombsa specific */
	void bombsa_bankselect_w(uint8_t data);
	void bombsa_flipscreen_w(uint8_t data);
	void bombsa_unknown_w(uint8_t data);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);

	virtual void machine_reset() override;
	DECLARE_MACHINE_START(psychic5);
	DECLARE_MACHINE_START(bombsa);
	virtual void video_start() override;
	DECLARE_VIDEO_START(psychic5);
	DECLARE_VIDEO_START(bombsa);
	virtual void video_reset() override;

	TIMER_DEVICE_CALLBACK_MEMBER(scanline);

	uint32_t screen_update_psychic5(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_bombsa(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void change_palette(int offset, uint8_t* palram, int palbase);
	void change_bg_palette(int color, int lo_offs, int hi_offs);
	void set_background_palette_intensity();
	void draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void bombsa_main_map(address_map &map);
	void bombsa_sound_map(address_map &map);
	void bombsa_soundport_map(address_map &map);
	void bombsa_vrambank_map(address_map &map);
	void psychic5_main_map(address_map &map);
	void psychic5_sound_map(address_map &map);
	void psychic5_soundport_map(address_map &map);
	void psychic5_vrambank_map(address_map &map);
	void draw_background(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect); //only used by psychic5
};
