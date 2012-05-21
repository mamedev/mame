class solomon_state : public driver_device
{
public:
	solomon_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_videoram2(*this, "videoram2"),
		m_colorram2(*this, "colorram2"){ }

	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_videoram2;
	required_shared_ptr<UINT8> m_colorram2;

	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;

	UINT8 m_nmi_mask;
	DECLARE_WRITE8_MEMBER(solomon_sh_command_w);
	DECLARE_READ8_MEMBER(solomon_0xe603_r);
	DECLARE_WRITE8_MEMBER(nmi_mask_w);
	DECLARE_WRITE8_MEMBER(solomon_videoram_w);
	DECLARE_WRITE8_MEMBER(solomon_colorram_w);
	DECLARE_WRITE8_MEMBER(solomon_videoram2_w);
	DECLARE_WRITE8_MEMBER(solomon_colorram2_w);
	DECLARE_WRITE8_MEMBER(solomon_flipscreen_w);
};


/*----------- defined in video/solomon.c -----------*/


VIDEO_START( solomon );
SCREEN_UPDATE_IND16( solomon );
