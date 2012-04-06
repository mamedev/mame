class portrait_state : public driver_device
{
public:
	portrait_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_bgvideoram;
	UINT8 *m_fgvideoram;
	int m_scroll;
	tilemap_t *m_foreground;
	tilemap_t *m_background;
	UINT8 *m_spriteram;
	size_t m_spriteram_size;
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
