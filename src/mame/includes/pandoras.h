// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
/*************************************************************************

    Pandora's Palace

*************************************************************************/

class pandoras_state : public driver_device
{
public:
	pandoras_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_colorram(*this, "colorram"),
		m_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_audiocpu(*this, "audiocpu"),
		m_mcu(*this, "mcu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_videoram;

	/* video-related */
	tilemap_t     *m_layer0;
	int         m_flipscreen;

	int m_irq_enable_a;
	int m_irq_enable_b;
	int m_firq_old_data_a;
	int m_firq_old_data_b;
	int m_i8039_status;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_mcu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	void pandoras_int_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pandoras_cpua_irqtrigger_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pandoras_cpub_irqtrigger_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pandoras_i8039_irqtrigger_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void i8039_irqen_and_status_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pandoras_z80_irqtrigger_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pandoras_vram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pandoras_cram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pandoras_scrolly_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pandoras_flipscreen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t pandoras_portA_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t pandoras_portB_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void get_tile_info0(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void palette_init_pandoras(palette_device &palette);
	uint32_t screen_update_pandoras(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void pandoras_master_interrupt(device_t &device);
	void pandoras_slave_interrupt(device_t &device);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, uint8_t* sr );
};
