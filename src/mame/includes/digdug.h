// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria

class digdug_state : public galaga_state
{
public:
	digdug_state(const machine_config &mconfig, device_type type, const char *tag)
		: galaga_state(mconfig, type, tag),
		m_digdug_objram(*this, "digdug_objram"),
		m_digdug_posram(*this, "digdug_posram"),
		m_digdug_flpram(*this, "digdug_flpram")     { }

	required_shared_ptr<UINT8> m_digdug_objram;
	required_shared_ptr<UINT8> m_digdug_posram;
	required_shared_ptr<UINT8> m_digdug_flpram;

	UINT8 m_bg_select;
	UINT8 m_tx_color_mode;
	UINT8 m_bg_disable;
	UINT8 m_bg_color_bank;
	DECLARE_CUSTOM_INPUT_MEMBER(shifted_port_r);
	TILEMAP_MAPPER_MEMBER(tilemap_scan);
	TILE_GET_INFO_MEMBER(bg_get_tile_info);
	TILE_GET_INFO_MEMBER(tx_get_tile_info);
	DECLARE_VIDEO_START(digdug);
	DECLARE_PALETTE_INIT(digdug);
	UINT32 screen_update_digdug(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE8_MEMBER( digdug_videoram_w );
	DECLARE_WRITE8_MEMBER( digdug_PORT_w );
};
