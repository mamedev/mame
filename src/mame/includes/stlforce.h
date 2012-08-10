class stlforce_state : public driver_device
{
public:
	stlforce_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_bg_videoram(*this, "bg_videoram"),
		m_mlow_videoram(*this, "mlow_videoram"),
		m_mhigh_videoram(*this, "mhigh_videoram"),
		m_tx_videoram(*this, "tx_videoram"),
		m_bg_scrollram(*this, "bg_scrollram"),
		m_mlow_scrollram(*this, "mlow_scrollram"),
		m_mhigh_scrollram(*this, "mhigh_scrollram"),
		m_vidattrram(*this, "vidattrram"),
		m_spriteram(*this, "spriteram"){ }

	tilemap_t *m_bg_tilemap;
	tilemap_t *m_mlow_tilemap;
	tilemap_t *m_mhigh_tilemap;
	tilemap_t *m_tx_tilemap;

	required_shared_ptr<UINT16> m_bg_videoram;
	required_shared_ptr<UINT16> m_mlow_videoram;
	required_shared_ptr<UINT16> m_mhigh_videoram;
	required_shared_ptr<UINT16> m_tx_videoram;
	required_shared_ptr<UINT16> m_bg_scrollram;
	required_shared_ptr<UINT16> m_mlow_scrollram;
	required_shared_ptr<UINT16> m_mhigh_scrollram;
	required_shared_ptr<UINT16> m_vidattrram;

	required_shared_ptr<UINT16> m_spriteram;

	int m_sprxoffs;
	DECLARE_WRITE16_MEMBER(stlforce_bg_videoram_w);
	DECLARE_WRITE16_MEMBER(stlforce_mlow_videoram_w);
	DECLARE_WRITE16_MEMBER(stlforce_mhigh_videoram_w);
	DECLARE_WRITE16_MEMBER(stlforce_tx_videoram_w);
	DECLARE_WRITE16_MEMBER(eeprom_w);
	DECLARE_WRITE16_MEMBER(oki_bank_w);
	DECLARE_DRIVER_INIT(twinbrat);
	DECLARE_DRIVER_INIT(stlforce);
};


/*----------- defined in video/stlforce.c -----------*/

VIDEO_START( stlforce );
SCREEN_UPDATE_IND16( stlforce );
