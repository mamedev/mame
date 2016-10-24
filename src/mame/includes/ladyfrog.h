// license:LGPL-2.1+
// copyright-holders:Tomasz Slanina
/*************************************************************************

    Lady Frog

*************************************************************************/

#include "machine/gen_latch.h"
#include "sound/msm5232.h"

class ladyfrog_state : public driver_device
{
public:
	ladyfrog_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_scrlram(*this, "scrlram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_msm(*this, "msm"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch") { }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	std::unique_ptr<uint8_t[]>    m_spriteram;
	required_shared_ptr<uint8_t> m_scrlram;
	std::vector<uint8_t> m_paletteram;
	std::vector<uint8_t> m_paletteram_ext;

	/* video-related */
	tilemap_t    *m_bg_tilemap;
	int        m_tilebank;
	int        m_palette_bank;
	int        m_spritetilebase;

	/* misc */
	int        m_sound_nmi_enable;
	int        m_pending_nmi;
	int        m_snd_flag;
	uint8_t      m_snd_data;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<msm5232_device> m_msm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	uint8_t from_snd_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void to_main_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sound_cpu_reset_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sound_command_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void nmi_disable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void nmi_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t snd_flag_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ladyfrog_spriteram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t ladyfrog_spriteram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ladyfrog_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t ladyfrog_videoram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ladyfrog_palette_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t ladyfrog_palette_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ladyfrog_gfxctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ladyfrog_gfxctrl2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t ladyfrog_scrlram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ladyfrog_scrlram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void unk_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void video_start_toucheme();
	void video_start_ladyfrog_common();
	uint32_t screen_update_ladyfrog(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void nmi_callback(void *ptr, int32_t param);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
};
