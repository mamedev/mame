/*************************************************************************

    D-Day

*************************************************************************/


class dday_state : public driver_device
{
public:
	dday_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *        m_bgvideoram;
	UINT8 *        m_fgvideoram;
	UINT8 *        m_textvideoram;
	UINT8 *        m_colorram;

	/* video-related */
	tilemap_t        *m_fg_tilemap;
	tilemap_t        *m_bg_tilemap;
	tilemap_t        *m_text_tilemap;
	tilemap_t        *m_sl_tilemap;
	bitmap_ind16 m_main_bitmap;
	int            m_control;
	int            m_sl_image;
	int            m_sl_enable;
	int            m_timer_value;

	/* devices */
	device_t *m_ay1;
	DECLARE_READ8_MEMBER(dday_countdown_timer_r);
	DECLARE_WRITE8_MEMBER(dday_bgvideoram_w);
	DECLARE_WRITE8_MEMBER(dday_fgvideoram_w);
	DECLARE_WRITE8_MEMBER(dday_textvideoram_w);
	DECLARE_WRITE8_MEMBER(dday_colorram_w);
	DECLARE_READ8_MEMBER(dday_colorram_r);
	DECLARE_WRITE8_MEMBER(dday_sl_control_w);
	DECLARE_WRITE8_MEMBER(dday_control_w);
};


/*----------- defined in video/dday.c -----------*/

PALETTE_INIT( dday );
VIDEO_START( dday );
SCREEN_UPDATE_IND16( dday );

