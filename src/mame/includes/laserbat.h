/*************************************************************************

    Laser Battle / Lazarian - Cat and Mouse

*************************************************************************/

class laserbat_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, laserbat_state(machine)); }

	laserbat_state(running_machine &machine) { }

	/* video-related */
	tilemap_t    *bg_tilemap;
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
	running_device *audiocpu;
	running_device *s2636_1;
	running_device *s2636_2;
	running_device *s2636_3;
	running_device *pia;
	running_device *sn;
	running_device *tms1;
	running_device *tms2;
	running_device *ay1;
	running_device *ay2;
};


/*----------- defined in audio/laserbat.c -----------*/

WRITE8_HANDLER( laserbat_csound1_w );
WRITE8_HANDLER( laserbat_csound2_w );
