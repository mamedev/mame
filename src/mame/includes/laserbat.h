/*************************************************************************

    Laser Battle / Lazarian - Cat and Mouse

*************************************************************************/

class laserbat_state : public driver_device
{
public:
	laserbat_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

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
	device_t *audiocpu;
	device_t *s2636_1;
	device_t *s2636_2;
	device_t *s2636_3;
	device_t *pia;
	device_t *sn;
	device_t *tms1;
	device_t *tms2;
	device_t *ay1;
	device_t *ay2;
};


/*----------- defined in audio/laserbat.c -----------*/

WRITE8_HANDLER( laserbat_csound1_w );
WRITE8_HANDLER( laserbat_csound2_w );
