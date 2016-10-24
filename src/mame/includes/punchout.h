// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    Punch Out / Super Punch Out / Arm Wrestling

***************************************************************************/

#include "machine/rp5c01.h"
#include "machine/rp5h01.h"
#include "sound/vlm5030.h"

class punchout_state : public driver_device
{
public:
	punchout_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_rtc(*this, "rtc"),
		m_rp5h01(*this, "rp5h01"),
		m_vlm(*this, "vlm"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_bg_top_videoram(*this, "bg_top_videoram"),
		m_spr1_ctrlram(*this, "spr1_ctrlram"),
		m_spr2_ctrlram(*this, "spr2_ctrlram"),
		m_palettebank(*this, "palettebank"),
		m_spr1_videoram(*this, "spr1_videoram"),
		m_spr2_videoram(*this, "spr2_videoram"),
		m_bg_bot_videoram(*this, "bg_bot_videoram"),
		m_armwrest_fg_videoram(*this, "armwrest_fgram")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<rp5c01_device> m_rtc;
	optional_device<rp5h01_device> m_rp5h01;
	required_device<vlm5030_device> m_vlm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_bg_top_videoram;
	required_shared_ptr<uint8_t> m_spr1_ctrlram;
	required_shared_ptr<uint8_t> m_spr2_ctrlram;
	required_shared_ptr<uint8_t> m_palettebank;
	required_shared_ptr<uint8_t> m_spr1_videoram;
	required_shared_ptr<uint8_t> m_spr2_videoram;
	required_shared_ptr<uint8_t> m_bg_bot_videoram;
	optional_shared_ptr<uint8_t> m_armwrest_fg_videoram;

	tilemap_t *m_bg_top_tilemap;
	tilemap_t *m_bg_bot_tilemap;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_spr1_tilemap;
	tilemap_t *m_spr1_tilemap_flipx;
	tilemap_t *m_spr2_tilemap;

	uint8_t m_nmi_mask;
	void punchout_2a03_reset_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t spunchout_exp_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void spunchout_exp_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void spunchout_rp5h01_reset_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void spunchout_rp5h01_clock_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void nmi_mask_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void punchout_bg_top_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void punchout_bg_bot_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void armwrest_fg_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void punchout_spr1_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void punchout_spr2_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void punchout_speech_reset_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void punchout_speech_st_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void punchout_speech_vcu_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void top_get_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void armwrest_top_get_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void bot_get_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void armwrest_bot_get_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void bs1_get_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void bs2_get_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void armwrest_fg_get_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	tilemap_memory_index armwrest_bs1_scan(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	tilemap_memory_index armwrest_bs1_scan_flipx(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	virtual void video_start() override;
	void video_start_armwrest();
	void machine_reset_spnchout();
	uint32_t screen_update_punchout_top(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_punchout_bottom(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_armwrest_top(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_armwrest_bottom(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vblank_irq(device_t &device);
	void draw_big_sprite(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int palette);
	void armwrest_draw_big_sprite(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int palette);
	void drawbs2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void punchout_copy_top_palette(int bank);
	void punchout_copy_bot_palette(int bank);
};
