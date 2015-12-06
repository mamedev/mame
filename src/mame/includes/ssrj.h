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

	required_shared_ptr<UINT8> m_vram1;
	required_shared_ptr<UINT8> m_vram2;
	required_shared_ptr<UINT8> m_vram3;
	required_shared_ptr<UINT8> m_vram4;
	required_shared_ptr<UINT8> m_scrollram;

	int m_oldport;
	tilemap_t *m_tilemap1;
	tilemap_t *m_tilemap2;
	tilemap_t *m_tilemap4;
	UINT8 *m_buffer_spriteram;

	DECLARE_READ8_MEMBER(wheel_r);
	DECLARE_WRITE8_MEMBER(vram1_w);
	DECLARE_WRITE8_MEMBER(vram2_w);
	DECLARE_WRITE8_MEMBER(vram4_w);

	TILE_GET_INFO_MEMBER(get_tile_info1);
	TILE_GET_INFO_MEMBER(get_tile_info2);
	TILE_GET_INFO_MEMBER(get_tile_info4);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(ssrj);

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof(screen_device &screen, bool state);
	void draw_objects(bitmap_ind16 &bitmap, const rectangle &cliprect );
};
