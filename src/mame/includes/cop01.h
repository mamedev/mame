// license:BSD-3-Clause
// copyright-holders:Carlos A. Lozano
/*************************************************************************

    Cops 01

*************************************************************************/

#include "machine/gen_latch.h"

class cop01_state : public driver_device
{
public:
	cop01_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_bgvideoram(*this, "bgvideoram"),
		m_spriteram(*this, "spriteram"),
		m_fgvideoram(*this, "fgvideoram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch") { }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_bgvideoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_fgvideoram;

	/* video-related */
	tilemap_t        *m_bg_tilemap;
	tilemap_t        *m_fg_tilemap;
	uint8_t          m_vreg[4];

	/* sound-related */
	int            m_pulse;
	int            m_timer; // kludge for ym3526 in mightguy

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	void cop01_sound_command_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t cop01_sound_command_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void cop01_irq_ack_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t cop01_sound_irq_ack_w(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t kludge(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void cop01_background_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void cop01_foreground_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void cop01_vreg_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	ioport_value mightguy_area_r(ioport_field &field, void *param);
	void init_mightguy();
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void palette_init_cop01(palette_device &palette);
	uint32_t screen_update_cop01(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
};
