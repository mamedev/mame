/*************************************************************************

    Goal! '92

*************************************************************************/

class goal92_state : public driver_device
{
public:
	goal92_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT16 *    m_bg_data;
	UINT16 *    m_fg_data;
	UINT16 *    m_tx_data;
	UINT16 *    m_scrollram;
//  UINT16 *    m_paletteram; // this currently use generic palette handling
	UINT16 *    m_spriteram;
	UINT16 *    m_buffered_spriteram;

	/* video-related */
	tilemap_t     *m_bg_layer;
	tilemap_t     *m_fg_layer;
	tilemap_t     *m_tx_layer;
	UINT16      m_fg_bank;

	/* misc */
	int         m_msm5205next;
	int         m_adpcm_toggle;

	/* devices */
	device_t *m_audiocpu;
};





/*----------- defined in video/goal92.c -----------*/

WRITE16_HANDLER( goal92_background_w );
WRITE16_HANDLER( goal92_foreground_w );
WRITE16_HANDLER( goal92_text_w );
WRITE16_HANDLER( goal92_fg_bank_w );
READ16_HANDLER( goal92_fg_bank_r );

VIDEO_START( goal92 );
SCREEN_UPDATE( goal92 );
SCREEN_EOF( goal92 );
