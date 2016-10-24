// license:BSD-3-Clause
// copyright-holders:Brad Oliver
#include "sound/samples.h"
class tankbatt_state : public driver_device
{
public:
	tankbatt_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_samples(*this, "samples"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_bulletsram(*this, "bulletsram"),
		m_videoram(*this, "videoram") { }

	required_device<cpu_device> m_maincpu;
	required_device<samples_device> m_samples;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_bulletsram;
	required_shared_ptr<uint8_t> m_videoram;

	int m_nmi_enable;
	int m_sound_enable;
	tilemap_t *m_bg_tilemap;

	void led_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t in0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t in1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t dsw_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void interrupt_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void demo_interrupt_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sh_expl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sh_engine_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sh_fire_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void irq_ack_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void coincounter_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void coinlockout_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);


	void interrupt(device_t &device);
	void coin_inserted(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);

	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	virtual void machine_start() override;
	virtual void video_start() override;
	void palette_init_tankbatt(palette_device &palette);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_bullets(bitmap_ind16 &bitmap, const rectangle &cliprect);
};
