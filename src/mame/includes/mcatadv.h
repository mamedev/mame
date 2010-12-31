
class mcatadv_state : public driver_device
{
public:
	mcatadv_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT16 *     videoram1;
	UINT16 *     videoram2;
	UINT16 *     scroll1;
	UINT16 *     scroll2;
	UINT16 *     spriteram;
	UINT16 *     spriteram_old;
	UINT16 *     vidregs;
	UINT16 *     vidregs_old;
//  UINT16 *     paletteram;    // this currently uses generic palette handlers
	size_t       spriteram_size;

	/* video-related */
	tilemap_t    *tilemap1,  *tilemap2;
	int palette_bank1, palette_bank2;

	/* devices */
	device_t *maincpu;
	device_t *soundcpu;
};

/*----------- defined in video/mcatadv.c -----------*/

VIDEO_UPDATE( mcatadv );
VIDEO_START( mcatadv );
VIDEO_EOF( mcatadv );

WRITE16_HANDLER( mcatadv_videoram1_w );
WRITE16_HANDLER( mcatadv_videoram2_w );
