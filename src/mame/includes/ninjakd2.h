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
	optional_shared_ptr<UINT8> m_bg_videoram;
	required_shared_ptr<UINT8> m_fg_videoram;
	required_shared_ptr<UINT8> m_spriteram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	optional_shared_ptr<UINT8> m_decrypted_opcodes;

	const INT16* m_sampledata;
	UINT8 m_omegaf_io_protection[3];
	UINT8 m_omegaf_io_protection_input;
	int m_omegaf_io_protection_tic;
	int m_next_sprite_overdraw_enabled;
	int (*m_stencil_compare_function) (UINT16 pal);
	int m_sprites_updated;
	bitmap_ind16 m_sprites_bitmap;
	int m_robokid_sprites;
	tilemap_t* m_fg_tilemap;
	tilemap_t* m_bg_tilemap;
	tilemap_t* m_bg0_tilemap;
	tilemap_t* m_bg1_tilemap;
	tilemap_t* m_bg2_tilemap;
	UINT8 m_vram_bank_mask;
	UINT8 m_robokid_bg0_bank;
	UINT8 m_robokid_bg1_bank;
	UINT8 m_robokid_bg2_bank;
	UINT8* m_robokid_bg0_videoram;
	UINT8* m_robokid_bg1_videoram;
	UINT8* m_robokid_bg2_videoram;
	UINT8 m_rom_bank_mask;

	void omegaf_io_protection_start();
	void omegaf_io_protection_reset();
	void robokid_motion_error_kludge(UINT16 offset);
	void video_init_common(UINT32 vram_alloc_size);

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
	DECLARE_DRIVER_INIT(mnight);
	DECLARE_DRIVER_INIT(ninjakd2);
	DECLARE_DRIVER_INIT(bootleg);
	DECLARE_DRIVER_INIT(robokid);
	DECLARE_DRIVER_INIT(robokidj);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(ninjakd2_get_bg_tile_info);
	TILE_GET_INFO_MEMBER(mnight_get_bg_tile_info);
	TILEMAP_MAPPER_MEMBER(robokid_bg_scan);
	TILEMAP_MAPPER_MEMBER(omegaf_bg_scan);
	TILE_GET_INFO_MEMBER(robokid_get_bg0_tile_info);
	TILE_GET_INFO_MEMBER(robokid_get_bg1_tile_info);
	TILE_GET_INFO_MEMBER(robokid_get_bg2_tile_info);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	DECLARE_VIDEO_START(mnight);
	DECLARE_VIDEO_START(arkarea);
	DECLARE_VIDEO_START(robokid);
	DECLARE_MACHINE_START(omegaf);
	DECLARE_MACHINE_RESET(omegaf);
	DECLARE_VIDEO_START(omegaf);
	UINT32 screen_update_ninjakd2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_robokid(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_omegaf(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_ninjakd2(screen_device &screen, bool state);
	INTERRUPT_GEN_MEMBER(ninjakd2_interrupt);
	void robokid_get_bg_tile_info( tile_data& tileinfo, tilemap_memory_index const tile_index, int const gfxnum, const UINT8* const videoram);
	void bg_ctrl(int offset, int data, tilemap_t* tilemap);
	void draw_sprites( bitmap_ind16 &bitmap);
	void erase_sprites( bitmap_ind16 &bitmap);
	void update_sprites();
	void lineswap_gfx_roms(const char *region, const int bit);
	void gfx_unscramble();
	DECLARE_WRITE_LINE_MEMBER(irqhandler);
};
