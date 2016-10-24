// license:LGPL-2.1+
// copyright-holders:Tomasz Slanina
class ssrj_state : public driver_device
{
public:
	ssrj_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_vram1(*this, "vram1"),
		m_vram2(*this, "vram2"),
		m_vram3(*this, "vram3"),
		m_vram4(*this, "vram4"),
		m_scrollram(*this, "scrollram") { }

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_vram1;
	required_shared_ptr<uint8_t> m_vram2;
	required_shared_ptr<uint8_t> m_vram3;
	required_shared_ptr<uint8_t> m_vram4;
	required_shared_ptr<uint8_t> m_scrollram;

	int m_oldport;
	tilemap_t *m_tilemap1;
	tilemap_t *m_tilemap2;
	tilemap_t *m_tilemap4;
	std::unique_ptr<uint8_t[]> m_buffer_spriteram;

	uint8_t wheel_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void vram1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void vram2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void vram4_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void get_tile_info1(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info2(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info4(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void palette_init_ssrj(palette_device &palette);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof(screen_device &screen, bool state);
	void draw_objects(bitmap_ind16 &bitmap, const rectangle &cliprect );
};
