// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*************************************************************************

    Mr. Do

*************************************************************************/

class mrdo_state : public driver_device
{
public:
	mrdo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_bgvideoram(*this, "bgvideoram"),
		m_fgvideoram(*this, "fgvideoram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")  { }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_bgvideoram;
	required_shared_ptr<uint8_t> m_fgvideoram;
	required_shared_ptr<uint8_t> m_spriteram;

	/* video-related */
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	int       m_flipscreen;
	uint8_t mrdo_SECRE_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mrdo_bgvideoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mrdo_fgvideoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mrdo_scrollx_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mrdo_scrolly_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mrdo_flipscreen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(mrdo);
	uint32_t screen_update_mrdo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bitmap,const rectangle &cliprect );
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};
