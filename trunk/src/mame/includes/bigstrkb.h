class bigstrkb_state : public driver_device
{
public:
	bigstrkb_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram2(*this, "videoram2"),
		m_videoram3(*this, "videoram3"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_vidreg1(*this, "vidreg1"),
		m_vidreg2(*this, "vidreg2"){ }

	tilemap_t *m_tilemap;
	tilemap_t *m_tilemap2;
	tilemap_t *m_tilemap3;

	required_shared_ptr<UINT16> m_videoram2;
	required_shared_ptr<UINT16> m_videoram3;
	required_shared_ptr<UINT16> m_videoram;
	required_shared_ptr<UINT16> m_spriteram;

	required_shared_ptr<UINT16> m_vidreg1;
	required_shared_ptr<UINT16> m_vidreg2;

	DECLARE_WRITE16_MEMBER(bsb_videoram_w);
	DECLARE_WRITE16_MEMBER(bsb_videoram2_w);
	DECLARE_WRITE16_MEMBER(bsb_videoram3_w);
};


/*----------- defined in video/bigstrkb.c -----------*/

VIDEO_START(bigstrkb);
SCREEN_UPDATE_IND16(bigstrkb);
