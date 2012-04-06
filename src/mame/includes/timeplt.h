/***************************************************************************

    Time Pilot

***************************************************************************/

class timeplt_state : public driver_device
{
public:
	timeplt_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *  m_videoram;
	UINT8 *  m_colorram;
	UINT8 *  m_spriteram;
	UINT8 *  m_spriteram2;

	/* video-related */
	tilemap_t  *m_bg_tilemap;

	/* misc */
	UINT8    m_nmi_enable;

	/* devices */
	cpu_device *m_maincpu;
	DECLARE_WRITE8_MEMBER(timeplt_nmi_enable_w);
	DECLARE_WRITE8_MEMBER(timeplt_coin_counter_w);
	DECLARE_READ8_MEMBER(psurge_protection_r);
	DECLARE_WRITE8_MEMBER(timeplt_videoram_w);
	DECLARE_WRITE8_MEMBER(timeplt_colorram_w);
	DECLARE_WRITE8_MEMBER(timeplt_flipscreen_w);
	DECLARE_READ8_MEMBER(timeplt_scanline_r);
};


/*----------- defined in video/timeplt.c -----------*/


PALETTE_INIT( timeplt );
VIDEO_START( timeplt );
VIDEO_START( chkun );
SCREEN_UPDATE_IND16( timeplt );
