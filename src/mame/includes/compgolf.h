/*************************************************************************

    Competition Golf Final Round

*************************************************************************/

class compgolf_state : public driver_device
{
public:
	compgolf_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *        m_videoram;
	UINT8 *        m_bg_ram;
	UINT8 *        m_spriteram;

	/* video-related */
	tilemap_t        *m_text_tilemap;
	tilemap_t        *m_bg_tilemap;
	int            m_scrollx_lo;
	int            m_scrollx_hi;
	int            m_scrolly_lo;
	int            m_scrolly_hi;

	/* misc */
	int            m_bank;
	DECLARE_WRITE8_MEMBER(compgolf_ctrl_w);
	DECLARE_WRITE8_MEMBER(compgolf_video_w);
	DECLARE_WRITE8_MEMBER(compgolf_back_w);
};


/*----------- defined in video/compgolf.c -----------*/

PALETTE_INIT ( compgolf );
VIDEO_START  ( compgolf );
SCREEN_UPDATE_IND16( compgolf );
