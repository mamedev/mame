// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    Time Pilot

***************************************************************************/

#include "sound/tc8830f.h"

class timeplt_state : public driver_device
{
public:
	timeplt_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_tc8830f(*this, "tc8830f"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_colorram(*this, "colorram"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_spriteram2(*this, "spriteram2")
	{ }

	required_device<cpu_device> m_maincpu;
	optional_device<tc8830f_device> m_tc8830f;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	/* memory pointers */
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_spriteram2;

	/* video-related */
	tilemap_t  *m_bg_tilemap;

	/* misc */
	uint8_t    m_nmi_enable;
	bool    m_video_enable;

	/* common */
	void coincounter_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void colorram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void flipscreen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t scanline_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	/* all but psurge */
	void nmi_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void video_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	/* psurge */
	uint8_t psurge_protection_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	/* chkun */
	ioport_value chkun_hopper_status_r(ioport_field &field, void *param);
	void chkun_sound_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_chkun_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void palette_init_timeplt(palette_device &palette);
	void video_start_chkun();
	void video_start_psurge();

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );

	void interrupt(device_t &device);
};
