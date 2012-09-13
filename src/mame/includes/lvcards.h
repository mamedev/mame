class lvcards_state : public driver_device
{
public:
	lvcards_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"){ }

	UINT8 m_payout;
	UINT8 m_pulse;
	UINT8 m_result;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;
	tilemap_t *m_bg_tilemap;
	DECLARE_WRITE8_MEMBER(control_port_2_w);
	DECLARE_WRITE8_MEMBER(control_port_2a_w);
	DECLARE_READ8_MEMBER(payout_r);
	DECLARE_WRITE8_MEMBER(lvcards_videoram_w);
	DECLARE_WRITE8_MEMBER(lvcards_colorram_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void video_start();
	virtual void palette_init();
	DECLARE_MACHINE_START(lvpoker);
	DECLARE_MACHINE_RESET(lvpoker);
	DECLARE_PALETTE_INIT(ponttehk);
};


/*----------- defined in video/lvcards.c -----------*/





SCREEN_UPDATE_IND16( lvcards );
