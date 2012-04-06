class silkroad_state : public driver_device
{
public:
	silkroad_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT32 *m_vidram;
	UINT32 *m_vidram2;
	UINT32 *m_vidram3;
	UINT32 *m_sprram;
	UINT32 *m_regs;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_fg2_tilemap;
	tilemap_t *m_fg3_tilemap;
	DECLARE_WRITE32_MEMBER(paletteram32_xRRRRRGGGGGBBBBB_dword_w);
	DECLARE_WRITE32_MEMBER(silk_coin_counter_w);
	DECLARE_WRITE32_MEMBER(silkroad_fgram_w);
	DECLARE_WRITE32_MEMBER(silkroad_fgram2_w);
	DECLARE_WRITE32_MEMBER(silkroad_fgram3_w);
};


/*----------- defined in video/silkroad.c -----------*/

VIDEO_START(silkroad);
SCREEN_UPDATE_IND16(silkroad);
