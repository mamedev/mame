// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*************************************************************************

Crazy Ballooon

*************************************************************************/

#include "sound/discrete.h"
#include "sound/sn76477.h"

#define CRBALOON_MASTER_XTAL    (XTAL_9_987MHz)


class crbaloon_state : public driver_device
{
public:
	crbaloon_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_spriteram(*this, "spriteram"),
		m_pc3092_data(*this, "pc3092_data"),
		m_maincpu(*this, "maincpu"),
		m_sn(*this, "snsnd"),
		m_discrete(*this, "discrete"),
		m_gfxdecode(*this, "gfxdecode") { }

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_pc3092_data;
	required_device<cpu_device> m_maincpu;
	required_device<sn76477_device> m_sn;
	required_device<discrete_device> m_discrete;
	required_device<gfxdecode_device> m_gfxdecode;

	uint16_t m_collision_address;
	uint8_t m_collision_address_clear;
	tilemap_t *m_bg_tilemap;
	uint8_t m_irq_mask;
	void pc3092_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t pc3259_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void port_sound_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void crbaloon_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void crbaloon_colorram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	ioport_value pc3092_r(ioport_field &field, void *param);
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_reset() override;
	virtual void video_start() override;
	void palette_init_crbaloon(palette_device &palette);
	uint32_t screen_update_crbaloon(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vblank_irq(device_t &device);
	void crbaloon_audio_set_music_freq(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void crbaloon_audio_set_music_enable(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void crbaloon_audio_set_laugh_enable(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint16_t crbaloon_get_collision_address();
	void crbaloon_set_clear_collision_address(int _crbaloon_collision_address_clear);
	void draw_sprite_and_check_collision(bitmap_ind16 &bitmap);
	void pc3092_reset(void);
	void pc3092_update();
	void pc3259_update(void);
	void crbaloon_audio_set_explosion_enable(int enabled);
	void crbaloon_audio_set_breath_enable(int enabled);
	void crbaloon_audio_set_appear_enable(int enabled);
};


/*----------- defined in audio/crbaloon.c -----------*/

MACHINE_CONFIG_EXTERN( crbaloon_audio );
