// license:BSD-3-Clause
// copyright-holders:Mike Balfour
class runaway_state : public driver_device
{
public:
	runaway_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_video_ram(*this, "video_ram"),
		m_sprite_ram(*this, "sprite_ram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	emu_timer *m_interrupt_timer;
	required_shared_ptr<uint8_t> m_video_ram;
	required_shared_ptr<uint8_t> m_sprite_ram;
	tilemap_t *m_bg_tilemap;
	int m_tile_bank;
	uint8_t runaway_input_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void runaway_led_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void runaway_irq_ack_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void runaway_paletteram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void runaway_video_ram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void runaway_tile_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t runaway_pot_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void runaway_get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void qwak_get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void video_start_qwak();
	uint32_t screen_update_runaway(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_qwak(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void interrupt_callback(void *ptr, int32_t param);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
};
