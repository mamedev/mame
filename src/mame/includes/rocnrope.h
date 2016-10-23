// license:BSD-3-Clause
// copyright-holders:Chris Hardy
class rocnrope_state : public driver_device
{
public:
	rocnrope_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_spriteram2(*this, "spriteram2"),
		m_spriteram(*this, "spriteram"),
		m_colorram(*this, "colorram"),
		m_videoram(*this, "videoram")
	{ }

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	// memory pointers
	required_shared_ptr<uint8_t> m_spriteram2;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_videoram;

	tilemap_t *m_bg_tilemap;
	uint8_t m_irq_mask;

	void rocnrope_interrupt_vector_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void irq_mask_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void rocnrope_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void rocnrope_colorram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void rocnrope_flipscreen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_rocnrope();

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(rocnrope);
	uint32_t screen_update_rocnrope(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vblank_irq);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect );
};
