
class btime_state : public driver_device
{
public:
	btime_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *  videoram;
	UINT8 *  colorram;
//  UINT8 *  paletteram;    // currently this uses generic palette handling
	UINT8 *  lnc_charbank;
	UINT8 *  bnj_backgroundram;
	UINT8 *  zoar_scrollram;
	UINT8 *  deco_charram;
	UINT8 *  spriteram;	// used by disco
//  UINT8 *  decrypted;
	UINT8 *  rambase;
	UINT8 *  audio_rambase;
	size_t   videoram_size;
	size_t   spriteram_size;
	size_t   bnj_backgroundram_size;

	/* video-related */
	bitmap_t *background_bitmap;
	UINT8    btime_palette;
	UINT8    bnj_scroll1;
	UINT8    bnj_scroll2;
	UINT8    btime_tilemap[4];

	/* audio-related */
	UINT8    audio_nmi_enable_type;
	UINT8    audio_nmi_enabled;
	UINT8    audio_nmi_state;

	/* protection-related (for mmonkey) */
	int      protection_command;
	int      protection_status;
	int      protection_value;
	int      protection_ret;

	/* devices */
	device_t *maincpu;
	device_t *audiocpu;
};


/*----------- defined in machine/btime.c -----------*/

READ8_HANDLER( mmonkey_protection_r );
WRITE8_HANDLER( mmonkey_protection_w );


/*----------- defined in video/btime.c -----------*/

PALETTE_INIT( btime );
PALETTE_INIT( lnc );

VIDEO_START( btime );
VIDEO_START( bnj );

VIDEO_UPDATE( btime );
VIDEO_UPDATE( cookrace );
VIDEO_UPDATE( bnj );
VIDEO_UPDATE( lnc );
VIDEO_UPDATE( zoar );
VIDEO_UPDATE( disco );
VIDEO_UPDATE( eggs );

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
