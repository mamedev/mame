class sprcros2_state : public driver_device
{
public:
	sprcros2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_fgvideoram(*this, "fgvideoram"),
		m_bgvideoram(*this, "bgvideoram"),
		m_spriteram(*this, "spriteram"),
		m_master(*this,"master"),
		m_slave(*this,"slave"){ }

	UINT8 m_s_port3;
	UINT8 m_port7;
	tilemap_t *m_bgtilemap;
	tilemap_t *m_fgtilemap;
	required_shared_ptr<UINT8> m_fgvideoram;
	required_shared_ptr<UINT8> m_bgvideoram;
	required_shared_ptr<UINT8> m_spriteram;

	required_device<cpu_device> m_master;
	required_device<cpu_device> m_slave;
	DECLARE_WRITE8_MEMBER(sprcros2_m_port7_w);
	DECLARE_WRITE8_MEMBER(sprcros2_s_port3_w);
	DECLARE_WRITE8_MEMBER(sprcros2_fgvideoram_w);
	DECLARE_WRITE8_MEMBER(sprcros2_bgvideoram_w);
	DECLARE_WRITE8_MEMBER(sprcros2_bgscrollx_w);
	DECLARE_WRITE8_MEMBER(sprcros2_bgscrolly_w);
	TILE_GET_INFO_MEMBER(get_sprcros2_bgtile_info);
	TILE_GET_INFO_MEMBER(get_sprcros2_fgtile_info);
	virtual void machine_start();
	virtual void video_start();
	virtual void palette_init();
};


/*----------- defined in video/sprcros2.c -----------*/




SCREEN_UPDATE_IND16( sprcros2 );
