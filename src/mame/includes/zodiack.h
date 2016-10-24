// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"

class zodiack_state : public driver_device
{
public:
	zodiack_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_audiocpu(*this, "audiocpu"),
			m_gfxdecode(*this, "gfxdecode"),
			m_palette(*this, "palette"),
			m_soundlatch(*this, "soundlatch"),
			m_videoram(*this, "videoram"),
			m_videoram_2(*this, "videoram_2"),
			m_attributeram(*this, "attributeram"),
			m_spriteram(*this, "spriteram"),
			m_bulletsram(*this, "bulletsram")
	{ }

	// in drivers/zodiack.c
	void nmi_mask_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sound_nmi_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void master_soundlatch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	// in video/zodiack.c
	void videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void videoram2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void attributes_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void flipscreen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void draw_bullets(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	// devices
	required_device<z80_device> m_maincpu;
	required_device<z80_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	// shared pointers
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_videoram_2;
	required_shared_ptr<uint8_t> m_attributeram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_bulletsram;

	// state
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	uint8_t m_main_nmi_enabled;
	uint8_t m_sound_nmi_enabled;
	bool m_percuss_hardware;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	void init_zodiack();
	void init_percuss();
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void palette_init_zodiack(palette_device &palette);
	void zodiack_sound_nmi_gen(device_t &device);
	void zodiack_main_nmi_gen(device_t &device);
};
