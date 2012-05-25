class tunhunt_state : public driver_device
{
public:
	tunhunt_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_workram(*this, "workram"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"){ }

	UINT8 m_control;
	required_shared_ptr<UINT8> m_workram;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_spriteram;
	tilemap_t *m_fg_tilemap;
	bitmap_ind16 m_tmpbitmap;
	DECLARE_WRITE8_MEMBER(tunhunt_control_w);
	DECLARE_READ8_MEMBER(tunhunt_button_r);
	DECLARE_WRITE8_MEMBER(tunhunt_videoram_w);
	DECLARE_READ8_MEMBER(dsw2_0r);
	DECLARE_READ8_MEMBER(dsw2_1r);
	DECLARE_READ8_MEMBER(dsw2_2r);
	DECLARE_READ8_MEMBER(dsw2_3r);
	DECLARE_READ8_MEMBER(dsw2_4r);
};


/*----------- defined in video/tunhunt.c -----------*/


PALETTE_INIT( tunhunt );
VIDEO_START( tunhunt );
SCREEN_UPDATE_IND16( tunhunt );
