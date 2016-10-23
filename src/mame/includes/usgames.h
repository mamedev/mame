// license:BSD-3-Clause
// copyright-holders:David Haywood, Nicola Salmoria
class usgames_state : public driver_device
{
public:
	usgames_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_videoram(*this, "videoram"),
		m_charram(*this, "charram") { }

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_charram;

	tilemap_t *m_tilemap;

	void rombank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void lamps1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void lamps2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void charram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	TILE_GET_INFO_MEMBER(get_tile_info);

	virtual void machine_start() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(usgames);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
