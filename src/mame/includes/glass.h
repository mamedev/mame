// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
/*************************************************************************

    Glass

*************************************************************************/

class glass_state : public driver_device
{
public:
	glass_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_vregs(*this, "vregs"),
		m_spriteram(*this, "spriteram"),
		m_mainram(*this, "mainram") { }


	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	/* memory pointers */
	required_shared_ptr<uint16_t> m_videoram;
	required_shared_ptr<uint16_t> m_vregs;
	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_mainram;

	/* video-related */
	tilemap_t     *m_pant[2];
	std::unique_ptr<bitmap_ind16> m_screen_bitmap;

	/* misc */
	int         m_current_bit;
	int         m_current_command;
	int         m_cause_interrupt;
	int         m_blitter_serial_buffer[5];

	void clr_int_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void OKIM6295_bankswitch_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void coin_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void blitter_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void vram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t mainram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void mainram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	void init_glass();
	void init_glassp();
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	void get_tile_info_screen0(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info_screen1(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void interrupt(device_t &device);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void ROM16_split_gfx( const char *src_reg, const char *dst_reg, int start, int length, int dest1, int dest2 );
};
