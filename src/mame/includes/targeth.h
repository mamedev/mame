// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
class targeth_state : public driver_device
{
public:
	targeth_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_vregs(*this, "vregs"),
		m_spriteram(*this, "spriteram") { }

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint16_t> m_videoram;
	required_shared_ptr<uint16_t> m_vregs;
	required_shared_ptr<uint16_t> m_spriteram;

	tilemap_t *m_pant[2];

	void OKIM6295_bankswitch_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void coin_counter_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void vram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	void get_tile_info_screen0(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info_screen1(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	virtual void video_start() override;

	void interrupt(timer_device &timer, void *ptr, int32_t param);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
};
