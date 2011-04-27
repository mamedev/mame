class zodiack_state : public driver_device
{
public:
	zodiack_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *   m_videoram;
	UINT8 *   m_attributeram;
	UINT8 *   m_spriteram;
	UINT8 *   m_videoram_2;
	UINT8 *   m_bulletsram;
	size_t    m_videoram_size;
	size_t    m_spriteram_size;
	size_t    m_bulletsram_size;

	/* video-related */
	tilemap_t   *m_bg_tilemap;
	tilemap_t   *m_fg_tilemap;

	/* sound-related */
	UINT8     m_nmi_enable;
	UINT8     m_sound_nmi_enabled;

	/* misc */
	int       m_percuss_hardware;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
};

/*----------- defined in video/zodiack.c -----------*/

WRITE8_HANDLER( zodiack_videoram_w );
WRITE8_HANDLER( zodiack_videoram2_w );
WRITE8_HANDLER( zodiack_attributes_w );
WRITE8_HANDLER( zodiack_flipscreen_w );

PALETTE_INIT( zodiack );
VIDEO_START( zodiack );
SCREEN_UPDATE( zodiack );
