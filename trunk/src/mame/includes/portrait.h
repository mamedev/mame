class portrait_state : public driver_device
{
public:
	portrait_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_bgvideoram(*this, "bgvideoram"),
		m_fgvideoram(*this, "fgvideoram"),
		m_spriteram(*this, "spriteram"){ }

	required_shared_ptr<UINT8> m_bgvideoram;
	required_shared_ptr<UINT8> m_fgvideoram;
	int m_scroll;
	tilemap_t *m_foreground;
	tilemap_t *m_background;
	required_shared_ptr<UINT8> m_spriteram;
	DECLARE_WRITE8_MEMBER(portrait_ctrl_w);
	DECLARE_WRITE8_MEMBER(portrait_positive_scroll_w);
	DECLARE_WRITE8_MEMBER(portrait_negative_scroll_w);
	DECLARE_WRITE8_MEMBER(portrait_bgvideo_write);
	DECLARE_WRITE8_MEMBER(portrait_fgvideo_write);
};


/*----------- defined in video/portrait.c -----------*/

PALETTE_INIT( portrait );
VIDEO_START( portrait );
SCREEN_UPDATE_IND16( portrait );
