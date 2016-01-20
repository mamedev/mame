// license:???
// copyright-holders:insideoutboy
class sprcros2_state : public driver_device
{
public:
	sprcros2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_master(*this,"master"),
		m_slave(*this,"slave"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_fgvideoram(*this, "fgvideoram"),
		m_bgvideoram(*this, "bgvideoram"),
		m_spriteram(*this, "spriteram") { }

	required_device<cpu_device> m_master;
	required_device<cpu_device> m_slave;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<UINT8> m_fgvideoram;
	required_shared_ptr<UINT8> m_bgvideoram;
	required_shared_ptr<UINT8> m_spriteram;

	UINT8 m_s_port3;
	UINT8 m_port7;
	tilemap_t *m_bgtilemap;
	tilemap_t *m_fgtilemap;

	DECLARE_WRITE8_MEMBER(m_port7_w);
	DECLARE_WRITE8_MEMBER(s_port3_w);
	DECLARE_WRITE8_MEMBER(fgvideoram_w);
	DECLARE_WRITE8_MEMBER(bgvideoram_w);
	DECLARE_WRITE8_MEMBER(bgscrollx_w);
	DECLARE_WRITE8_MEMBER(bgscrolly_w);

	TILE_GET_INFO_MEMBER(get_bgtile_info);
	TILE_GET_INFO_MEMBER(get_fgtile_info);

	virtual void machine_start() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(sprcros2);

	INTERRUPT_GEN_MEMBER(s_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(m_interrupt);

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);
};
