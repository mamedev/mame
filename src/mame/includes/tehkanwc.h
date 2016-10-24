// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi, Roberto Fresca

#include "machine/gen_latch.h"
#include "sound/msm5205.h"

class tehkanwc_state : public driver_device
{
public:
	enum
	{
		TIMER_RESET
	};

	tehkanwc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_subcpu(*this, "sub"),
		m_msm(*this, "msm"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_soundlatch2(*this, "soundlatch2"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_videoram2(*this, "videoram2"),
		m_spriteram(*this, "spriteram") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_subcpu;
	required_device<msm5205_device> m_msm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<generic_latch_8_device> m_soundlatch2;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_videoram2;
	required_shared_ptr<uint8_t> m_spriteram;

	int m_track0[2];
	int m_track1[2];
	int m_msm_data_offs;
	int m_toggle;
	uint8_t m_scroll_x[2];
	uint8_t m_led0;
	uint8_t m_led1;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;

	void sub_cpu_halt_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t track_0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t track_1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void track_0_reset_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void track_1_reset_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sound_command_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sound_answer_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void colorram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void videoram2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void scroll_x_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void scroll_y_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void flipscreen_x_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void flipscreen_y_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void gridiron_led0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void gridiron_led1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t portA_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t portB_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void portA_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void portB_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void msm_reset_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void adpcm_int(int state);

	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	void init_teedoff();
	virtual void machine_start() override;
	virtual void video_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void gridiron_draw_led(bitmap_ind16 &bitmap, const rectangle &cliprect, uint8_t led,int player);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
