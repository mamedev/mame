// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli, David Haywood
/*************************************************************************

    Goal! '92

*************************************************************************/

#include "machine/gen_latch.h"
#include "sound/msm5205.h"

class goal92_state : public driver_device
{
public:
	goal92_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_bg_data(*this, "bg_data"),
		m_fg_data(*this, "fg_data"),
		m_tx_data(*this, "tx_data"),
		m_spriteram(*this, "spriteram"),
		m_scrollram(*this, "scrollram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_msm(*this, "msm"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch") { }

	/* memory pointers */
	required_shared_ptr<uint16_t> m_bg_data;
	required_shared_ptr<uint16_t> m_fg_data;
	required_shared_ptr<uint16_t> m_tx_data;
	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_scrollram;
	std::unique_ptr<uint16_t[]>    m_buffered_spriteram;

	/* video-related */
	tilemap_t     *m_bg_layer;
	tilemap_t     *m_fg_layer;
	tilemap_t     *m_tx_layer;
	uint16_t      m_fg_bank;

	/* misc */
	int         m_msm5205next;
	int         m_adpcm_toggle;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<msm5205_device> m_msm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	void goal92_sound_command_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t goal92_inputs_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void adpcm_data_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint16_t goal92_fg_bank_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void goal92_fg_bank_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void goal92_text_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void goal92_background_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void goal92_foreground_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void adpcm_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void get_text_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_back_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_fore_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_goal92(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_goal92(screen_device &screen, bool state);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, int pri );
	void irqhandler(int state);
	void goal92_adpcm_int(int state);
};
