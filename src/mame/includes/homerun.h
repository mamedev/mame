// license:LGPL-2.1+
// copyright-holders:Tomasz Slanina
/*************************************************************************

    Jaleco Moero Pro Yakyuu Homerun hardware

*************************************************************************/

#include "sound/upd7759.h"
#include "sound/samples.h"

class homerun_state : public driver_device
{
public:
	homerun_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_colorram(*this, "colorram"),
		m_d7756(*this, "d7756"),
		m_samples(*this, "samples"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette")
	{ }

	required_device<cpu_device> m_maincpu;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_colorram;
	optional_device<upd7756_device> m_d7756;
	optional_device<samples_device> m_samples;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	uint8_t m_control;
	uint8_t m_sample;

	tilemap_t *m_tilemap;
	int m_gfx_ctrl;
	int m_scrollx;
	int m_scrolly;

	void homerun_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void homerun_d7756_sample_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void homerun_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void homerun_color_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void homerun_scrollhi_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void homerun_scrolly_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void homerun_scrollx_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	ioport_value homerun_sprite0_r(ioport_field &field, void *param);
	ioport_value homerun_d7756_busy_r(ioport_field &field, void *param);
	ioport_value ganjaja_d7756_busy_r(ioport_field &field, void *param);
	ioport_value ganjaja_hopper_status_r(ioport_field &field, void *param);

	void get_homerun_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_homerun(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void homerun_banking_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
};
