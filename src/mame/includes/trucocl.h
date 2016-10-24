// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
#include "sound/dac.h"

class trucocl_state : public driver_device
{
public:
	enum
	{
		TIMER_DAC_IRQ
	};

	trucocl_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_maincpu(*this, "maincpu"),
		m_dac(*this, "dac"),
		m_gfxdecode(*this, "gfxdecode") { }

	int m_cur_dac_address;
	int m_cur_dac_address_index;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	tilemap_t *m_bg_tilemap;

	uint8_t m_irq_mask;
	void irq_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void trucocl_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void trucocl_colorram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void audio_dac_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_trucocl();
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void video_start() override;
	void palette_init_trucocl(palette_device &palette);
	uint32_t screen_update_trucocl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void trucocl_interrupt(device_t &device);
	required_device<cpu_device> m_maincpu;
	required_device<dac_byte_interface> m_dac;
	required_device<gfxdecode_device> m_gfxdecode;

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
