// license:BSD-3-Clause
// copyright-holders:Roberto Ventura, Leandro Dardini, Yochizo, Nicola Salmoria
/******************************************************************************

    UPL "sprite framebuffer" hardware

******************************************************************************/

#include "sound/samples.h"
#include "screen.h"

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

	DECLARE_WRITE8_MEMBER(ninjakd2_bankselect_w);
	DECLARE_WRITE8_MEMBER(ninjakd2_soundreset_w);
	DECLARE_WRITE8_MEMBER(ninjakd2_pcm_play_w);
	SAMPLES_START_CB_MEMBER(ninjakd2_init_samples);
	DECLARE_READ8_MEMBER(omegaf_io_protection_r);
	DECLARE_READ8_MEMBER(robokid_motion_error_verbose_r);
	DECLARE_WRITE8_MEMBER(omegaf_io_protection_w);
	DECLARE_WRITE8_MEMBER(ninjakd2_bgvideoram_w);
	DECLARE_WRITE8_MEMBER(ninjakd2_fgvideoram_w);
	DECLARE_WRITE8_MEMBER(robokid_bg0_bank_w);
	DECLARE_WRITE8_MEMBER(robokid_bg1_bank_w);
	DECLARE_WRITE8_MEMBER(robokid_bg2_bank_w);
	DECLARE_READ8_MEMBER(robokid_bg0_videoram_r);
	DECLARE_READ8_MEMBER(robokid_bg1_videoram_r);
	DECLARE_READ8_MEMBER(robokid_bg2_videoram_r);
	DECLARE_WRITE8_MEMBER(robokid_bg0_videoram_w);
	DECLARE_WRITE8_MEMBER(robokid_bg1_videoram_w);
	DECLARE_WRITE8_MEMBER(robokid_bg2_videoram_w);
	DECLARE_WRITE8_MEMBER(ninjakd2_bg_ctrl_w);
	DECLARE_WRITE8_MEMBER(robokid_bg0_ctrl_w);
	DECLARE_WRITE8_MEMBER(robokid_bg1_ctrl_w);
	DECLARE_WRITE8_MEMBER(robokid_bg2_ctrl_w);
	DECLARE_WRITE8_MEMBER(ninjakd2_sprite_overdraw_w);
	void init_mnight();
	void init_ninjakd2();
	void init_bootleg();
	void init_robokid();
	void init_robokidj();
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(ninjakd2_get_bg_tile_info);
	TILE_GET_INFO_MEMBER(mnight_get_bg_tile_info);
	TILEMAP_MAPPER_MEMBER(robokid_bg_scan);
	TILEMAP_MAPPER_MEMBER(omegaf_bg_scan);
	TILE_GET_INFO_MEMBER(robokid_get_bg0_tile_info);
	TILE_GET_INFO_MEMBER(robokid_get_bg1_tile_info);
	TILE_GET_INFO_MEMBER(robokid_get_bg2_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_VIDEO_START(mnight);
	DECLARE_VIDEO_START(arkarea);
	DECLARE_VIDEO_START(robokid);
	DECLARE_MACHINE_START(omegaf);
	DECLARE_MACHINE_RESET(omegaf);
	DECLARE_VIDEO_START(omegaf);
	uint32_t screen_update_ninjakd2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_robokid(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_omegaf(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank_ninjakd2);
	void robokid_get_bg_tile_info( tile_data& tileinfo, tilemap_memory_index const tile_index, int const gfxnum, const uint8_t* const videoram);
	void bg_ctrl(int offset, int data, tilemap_t* tilemap);
	void draw_sprites( bitmap_ind16 &bitmap);
	void erase_sprites( bitmap_ind16 &bitmap);
	void update_sprites();
	void lineswap_gfx_roms(const char *region, const int bit);
	void gfx_unscramble();
	void omegaf(machine_config &config);
	void ninjakd2b(machine_config &config);
	void robokid(machine_config &config);
	void arkarea(machine_config &config);
	void mnight(machine_config &config);
	void ninjakd2(machine_config &config);
	void ninjakd2_core(machine_config &config);
	void decrypted_opcodes_map(address_map &map);
	void mnight_main_cpu(address_map &map);
	void ninjakd2_main_cpu(address_map &map);
	void ninjakd2_sound_cpu(address_map &map);
	void ninjakd2_sound_io(address_map &map);
	void ninjakid_nopcm_sound_cpu(address_map &map);
	void omegaf_main_cpu(address_map &map);
	void robokid_main_cpu(address_map &map);
};
