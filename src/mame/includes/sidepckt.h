// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
/******************************************************************************

    Data East Side Pocket hardware

******************************************************************************/

#include "machine/gen_latch.h"

class sidepckt_state : public driver_device
{
public:
	sidepckt_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_spriteram(*this, "spriteram")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_spriteram;

	tilemap_t *m_bg_tilemap;
	const uint8_t* m_prot_table[3];
	uint8_t m_i8751_return;
	uint8_t m_current_ptr;
	uint8_t m_current_table;
	uint8_t m_in_math;
	uint8_t m_math_param;

	void sound_cpu_command_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t i8751_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void i8751_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void colorram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void flipscreen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void init_sidepckt();
	void init_sidepcktj();

	void get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	virtual void machine_reset() override;
	virtual void video_start() override;
	void palette_init_sidepckt(palette_device &palette);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);
};
