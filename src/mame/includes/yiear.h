class yiear_state : public driver_device
{
public:
	yiear_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_spriteram(*this, "spriteram"),
		m_spriteram2(*this, "spriteram2"),
		m_videoram(*this, "videoram"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_spriteram2;
	required_shared_ptr<UINT8> m_videoram;

	/* video-related */
	tilemap_t  *m_bg_tilemap;

	UINT8      m_yiear_nmi_enable;
	UINT8      m_yiear_irq_enable;
	DECLARE_WRITE8_MEMBER(yiear_videoram_w);
	DECLARE_WRITE8_MEMBER(yiear_control_w);
};


/*----------- defined in video/yiear.c -----------*/


PALETTE_INIT( yiear );
VIDEO_START( yiear );
SCREEN_UPDATE_IND16( yiear );

READ8_DEVICE_HANDLER( yiear_speech_r );
WRITE8_DEVICE_HANDLER( yiear_VLM5030_control_w );
