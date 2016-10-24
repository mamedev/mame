// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Ernesto Corvi
class iqblock_state : public driver_device
{
public:
	iqblock_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_rambase(*this, "rambase"),
		m_bgvideoram(*this, "bgvideoram"),
		m_fgvideoram(*this, "fgvideoram") { }

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;

	required_shared_ptr<uint8_t> m_rambase;
	required_shared_ptr<uint8_t> m_bgvideoram;
	required_shared_ptr<uint8_t> m_fgvideoram;

	int m_videoenable;
	int m_video_type;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;

	void iqblock_prot_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void grndtour_prot_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void irqack_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void fgvideoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bgvideoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void fgscroll_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void port_C_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void irq(timer_device &timer, void *ptr, int32_t param);

	void init_grndtour();
	void init_iqblock();
	virtual void video_start() override;

	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
