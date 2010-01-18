/***************************************************************************

    Astro Fighter hardware

****************************************************************************/

typedef struct _astrof_state astrof_state;
struct _astrof_state
{
	/* video-related */
	UINT8 *    videoram;
	size_t     videoram_size;

	UINT8 *    colorram;
	UINT8 *    tomahawk_protection;

	UINT8 *    astrof_color;
	UINT8      astrof_palette_bank;
	UINT8      red_on;
	UINT8      flipscreen;
	UINT8      screen_off;
	UINT16     abattle_count;

	/* sound-related */
	UINT8      port_1_last;
	UINT8      port_2_last;
	UINT8      astrof_start_explosion;
	UINT8      astrof_death_playing;
	UINT8      astrof_bosskill_playing;

	/* devices */
	running_device *maincpu;
	running_device *samples;	// astrof & abattle
	running_device *sn;	// tomahawk
};

/*----------- defined in audio/astrof.c -----------*/

MACHINE_DRIVER_EXTERN( astrof_audio );
WRITE8_HANDLER( astrof_audio_1_w );
WRITE8_HANDLER( astrof_audio_2_w );

MACHINE_DRIVER_EXTERN( spfghmk2_audio );
WRITE8_HANDLER( spfghmk2_audio_w );

MACHINE_DRIVER_EXTERN( tomahawk_audio );
WRITE8_HANDLER( tomahawk_audio_w );
