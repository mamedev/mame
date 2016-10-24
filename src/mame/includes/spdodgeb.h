// license:BSD-3-Clause
// copyright-holders:Paul Hampson, Nicola Salmoria
/*************************************************************************

    Super Dodge Ball hardware

*************************************************************************/

#include "machine/gen_latch.h"
#include "sound/msm5205.h"

class spdodgeb_state : public driver_device
{
public:
	spdodgeb_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_msm1(*this, "msm1"),
		m_msm2(*this, "msm2"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<msm5205_device> m_msm1;
	required_device<msm5205_device> m_msm2;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;

	int m_toggle;
	int m_adpcm_pos[2];
	int m_adpcm_end[2];
	int m_adpcm_idle[2];
	int m_adpcm_data[2];
	int m_mcu63701_command;
	int m_inputs[4];
	uint8_t m_tapc[4];
	uint8_t m_last_port[2];
	uint8_t m_last_dash[2];
#if 0
	int m_running[2];
	int m_jumped[2];
	int m_prev[2][2];
	int m_countup[2][2];
	int m_countdown[2][2];
	int m_prev[2];
#endif
	int m_tile_palbank;
	int m_sprite_palbank;
	tilemap_t *m_bg_tilemap;
	int m_lastscroll;

	void sound_command_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void spd_adpcm_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t mcu63701_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mcu63701_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void scrollx_lo_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void spd_adpcm_int_1(int state);
	void spd_adpcm_int_2(int state);

	tilemap_memory_index background_scan(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void palette_init_spdodgeb(palette_device &palette);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect );

	void interrupt(timer_device &timer, void *ptr, int32_t param);
	ioport_value mcu63705_busy_r(ioport_field &field, void *param);

	void mcu63705_update_inputs();
	void spd_adpcm_int(msm5205_device *device, int chip);
};
