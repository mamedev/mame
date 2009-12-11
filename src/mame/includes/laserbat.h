/*************************************************************************

    Laser Battle / Lazarian - Cat and Mouse

*************************************************************************/

typedef struct _laserbat_state laserbat_state;
struct _laserbat_state
{
	/* video-related */
	tilemap    *bg_tilemap;
	UINT8      *videoram;
	UINT8      *colorram;
	int        video_page;

	/* misc */
	int        input_mux;
	int        active_8910, port0a, last_port0b;
	int        cb1_toggle;

	/* information for the single 32x32 sprite displayed */
	int        sprite_x;
	int        sprite_y;
	int        sprite_code;
	int        sprite_color;
	int        sprite_enable;

	/* sound-related */
	int        csound1;
	int        ksound1, ksound2, ksound3;
	int        degr, filt, a, us, bit14;

	/* device */
	const device_config *audiocpu;
	const device_config *s2636_1;
	const device_config *s2636_2;
	const device_config *s2636_3;
	const device_config *pia;
	const device_config *sn;
	const device_config *tms1;
	const device_config *tms2;
	const device_config *ay1;
	const device_config *ay2;
};


/*----------- defined in audio/laserbat.c -----------*/

WRITE8_HANDLER( laserbat_csound1_w );
WRITE8_HANDLER( laserbat_csound2_w );
