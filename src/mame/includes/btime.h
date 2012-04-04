
class btime_state : public driver_device
{
public:
	btime_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *  m_videoram;
	UINT8 *  m_colorram;
//  UINT8 *  m_paletteram;    // currently this uses generic palette handling
	UINT8 *  m_lnc_charbank;
	UINT8 *  m_bnj_backgroundram;
	UINT8 *  m_zoar_scrollram;
	UINT8 *  m_deco_charram;
	UINT8 *  m_spriteram;	// used by disco
//  UINT8 *  m_decrypted;
	UINT8 *  m_rambase;
	UINT8 *  m_audio_rambase;
	size_t   m_videoram_size;
	size_t   m_spriteram_size;
	size_t   m_bnj_backgroundram_size;

	/* video-related */
	bitmap_ind16 *m_background_bitmap;
	UINT8    m_btime_palette;
	UINT8    m_bnj_scroll1;
	UINT8    m_bnj_scroll2;
	UINT8    m_btime_tilemap[4];

	/* audio-related */
	UINT8    m_audio_nmi_enable_type;
	UINT8    m_audio_nmi_enabled;
	UINT8    m_audio_nmi_state;

	/* protection-related (for mmonkey) */
	int      m_protection_command;
	int      m_protection_status;
	int      m_protection_value;
	int      m_protection_ret;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	DECLARE_WRITE8_MEMBER(audio_nmi_enable_w);
	DECLARE_WRITE8_MEMBER(lnc_w);
	DECLARE_WRITE8_MEMBER(mmonkey_w);
	DECLARE_WRITE8_MEMBER(btime_w);
	DECLARE_WRITE8_MEMBER(tisland_w);
	DECLARE_WRITE8_MEMBER(zoar_w);
	DECLARE_WRITE8_MEMBER(disco_w);
	DECLARE_WRITE8_MEMBER(audio_command_w);
	DECLARE_READ8_MEMBER(audio_command_r);
	DECLARE_READ8_MEMBER(zoar_dsw1_read);
	DECLARE_READ8_MEMBER(wtennis_reset_hack_r);
};


/*----------- defined in machine/btime.c -----------*/

READ8_HANDLER( mmonkey_protection_r );
WRITE8_HANDLER( mmonkey_protection_w );


/*----------- defined in video/btime.c -----------*/

PALETTE_INIT( btime );
PALETTE_INIT( lnc );

VIDEO_START( btime );
VIDEO_START( bnj );

SCREEN_UPDATE_IND16( btime );
SCREEN_UPDATE_IND16( cookrace );
SCREEN_UPDATE_IND16( bnj );
SCREEN_UPDATE_IND16( lnc );
SCREEN_UPDATE_IND16( zoar );
SCREEN_UPDATE_IND16( disco );
SCREEN_UPDATE_IND16( eggs );

WRITE8_HANDLER( btime_paletteram_w );
WRITE8_HANDLER( bnj_background_w );
WRITE8_HANDLER( bnj_scroll1_w );
WRITE8_HANDLER( bnj_scroll2_w );
READ8_HANDLER( btime_mirrorvideoram_r );
WRITE8_HANDLER( btime_mirrorvideoram_w );
READ8_HANDLER( btime_mirrorcolorram_r );
WRITE8_HANDLER( btime_mirrorcolorram_w );
WRITE8_HANDLER( lnc_videoram_w );
WRITE8_HANDLER( lnc_mirrorvideoram_w );
WRITE8_HANDLER( deco_charram_w );

WRITE8_HANDLER( zoar_video_control_w );
WRITE8_HANDLER( btime_video_control_w );
WRITE8_HANDLER( bnj_video_control_w );
WRITE8_HANDLER( disco_video_control_w );
