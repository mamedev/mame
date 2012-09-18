class silkroad_state : public driver_device
{
public:
	silkroad_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_vidram(*this, "vidram"),
		m_vidram2(*this, "vidram2"),
		m_vidram3(*this, "vidram3"),
		m_sprram(*this, "sprram"),
		m_regs(*this, "regs"){ }

	required_shared_ptr<UINT32> m_vidram;
	required_shared_ptr<UINT32> m_vidram2;
	required_shared_ptr<UINT32> m_vidram3;
	required_shared_ptr<UINT32> m_sprram;
	required_shared_ptr<UINT32> m_regs;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_fg2_tilemap;
	tilemap_t *m_fg3_tilemap;
	DECLARE_WRITE32_MEMBER(paletteram32_xRRRRRGGGGGBBBBB_dword_w);
	DECLARE_WRITE32_MEMBER(silk_coin_counter_w);
	DECLARE_WRITE32_MEMBER(silkroad_fgram_w);
	DECLARE_WRITE32_MEMBER(silkroad_fgram2_w);
	DECLARE_WRITE32_MEMBER(silkroad_fgram3_w);
	DECLARE_WRITE32_MEMBER(silk_6295_bank_w);
	DECLARE_DRIVER_INIT(silkroad);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg2_tile_info);
	TILE_GET_INFO_MEMBER(get_fg3_tile_info);
	virtual void video_start();
	UINT32 screen_update_silkroad(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
