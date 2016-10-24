// license:BSD-3-Clause
// copyright-holders:Roberto Ventura, Leandro Dardini, Yochizo, Nicola Salmoria
/******************************************************************************

    UPL "sprite framebuffer" hardware

******************************************************************************/

#include "sound/samples.h"

class ninjakd2_state : public driver_device
{
public:
	ninjakd2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_pcm(*this, "pcm"),
		m_pcm_region(*this, "pcm"),
		m_bg_videoram(*this, "bg_videoram"),
		m_fg_videoram(*this, "fg_videoram"),
		m_spriteram(*this, "spriteram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_decrypted_opcodes(*this, "decrypted_opcodes")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	optional_device<samples_device> m_pcm;
	optional_memory_region m_pcm_region;
	optional_shared_ptr<uint8_t> m_bg_videoram;
	required_shared_ptr<uint8_t> m_fg_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	optional_shared_ptr<uint8_t> m_decrypted_opcodes;

	const int16_t* m_sampledata;
	uint8_t m_omegaf_io_protection[3];
	uint8_t m_omegaf_io_protection_input;
	int m_omegaf_io_protection_tic;
	int m_next_sprite_overdraw_enabled;
	bool (*m_stencil_compare_function) (uint16_t pal);
	int m_sprites_updated;
	bitmap_ind16 m_sprites_bitmap;
	int m_robokid_sprites;
	tilemap_t* m_fg_tilemap;
	tilemap_t* m_bg_tilemap;
	tilemap_t* m_bg0_tilemap;
	tilemap_t* m_bg1_tilemap;
	tilemap_t* m_bg2_tilemap;
	uint8_t m_vram_bank_mask;
	uint8_t m_robokid_bg0_bank;
	uint8_t m_robokid_bg1_bank;
	uint8_t m_robokid_bg2_bank;
	std::unique_ptr<uint8_t[]> m_robokid_bg0_videoram;
	std::unique_ptr<uint8_t[]> m_robokid_bg1_videoram;
	std::unique_ptr<uint8_t[]> m_robokid_bg2_videoram;
	uint8_t m_rom_bank_mask;

	void omegaf_io_protection_start();
	void omegaf_io_protection_reset();
	void robokid_motion_error_kludge(uint16_t offset);
	void video_init_common(uint32_t vram_alloc_size);

	void ninjakd2_bankselect_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ninjakd2_soundreset_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ninjakd2_pcm_play_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	SAMPLES_START_CB_MEMBER(ninjakd2_init_samples);
	uint8_t omegaf_io_protection_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t robokid_motion_error_verbose_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void omegaf_io_protection_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ninjakd2_bgvideoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ninjakd2_fgvideoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void robokid_bg0_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void robokid_bg1_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void robokid_bg2_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t robokid_bg0_videoram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t robokid_bg1_videoram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t robokid_bg2_videoram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void robokid_bg0_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void robokid_bg1_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void robokid_bg2_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ninjakd2_bg_ctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void robokid_bg0_ctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void robokid_bg1_ctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void robokid_bg2_ctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ninjakd2_sprite_overdraw_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_mnight();
	void init_ninjakd2();
	void init_bootleg();
	void init_robokid();
	void init_robokidj();
	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void ninjakd2_get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void mnight_get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	tilemap_memory_index robokid_bg_scan(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	tilemap_memory_index omegaf_bg_scan(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	void robokid_get_bg0_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void robokid_get_bg1_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void robokid_get_bg2_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void video_start_mnight();
	void video_start_arkarea();
	void video_start_robokid();
	void machine_start_omegaf();
	void machine_reset_omegaf();
	void video_start_omegaf();
	uint32_t screen_update_ninjakd2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_robokid(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_omegaf(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_ninjakd2(screen_device &screen, bool state);
	void ninjakd2_interrupt(device_t &device);
	void robokid_get_bg_tile_info( tile_data& tileinfo, tilemap_memory_index const tile_index, int const gfxnum, const uint8_t* const videoram);
	void bg_ctrl(int offset, int data, tilemap_t* tilemap);
	void draw_sprites( bitmap_ind16 &bitmap);
	void erase_sprites( bitmap_ind16 &bitmap);
	void update_sprites();
	void lineswap_gfx_roms(const char *region, const int bit);
	void gfx_unscramble();
};
