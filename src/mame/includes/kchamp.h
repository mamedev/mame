// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
/*************************************************************************

    Karate Champ

*************************************************************************/

#include "machine/gen_latch.h"
#include "sound/msm5205.h"
#include "sound/dac.h"

class kchamp_state : public driver_device
{
public:
	kchamp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_spriteram(*this, "spriteram"),
		m_decrypted_opcodes(*this, "decrypted_opcodes"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_msm(*this, "msm"),
		m_dac(*this, "dac"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch") { }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_spriteram;
	optional_shared_ptr<uint8_t> m_decrypted_opcodes;

	/* video-related */
	tilemap_t    *m_bg_tilemap;

	/* misc */
	int        m_nmi_enable;
	int        m_sound_nmi_enable;
	int        m_msm_data;
	int        m_msm_play_lo_nibble;
	int        m_counter;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<msm5205_device> m_msm;
	optional_device<dac_8bit_r2r_device> m_dac;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	void control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sound_reset_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sound_command_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sound_msm_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t sound_reset_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void kc_sound_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void kchamp_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void kchamp_colorram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void kchamp_flipscreen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sound_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_kchampvs();
	void init_kchampvs2();
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_reset() override;
	virtual void video_start() override;
	void palette_init_kchamp(palette_device &palette);
	void machine_start_kchampvs();
	void machine_start_kchamp();
	uint32_t screen_update_kchampvs(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_kchamp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void kc_interrupt(device_t &device);
	void sound_int(device_t &device);
	void kchamp_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void kchampvs_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void decrypt_code();
	void msmint(int state);
};
