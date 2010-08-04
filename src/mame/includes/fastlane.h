/*************************************************************************

    Fast Lane

*************************************************************************/

class fastlane_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, fastlane_state(machine)); }

	fastlane_state(running_machine &machine)
		: driver_data_t(machine) { }

	/* memory pointers */
	UINT8 *    videoram1;
	UINT8 *    videoram2;
	UINT8 *    paletteram;
	UINT8 *    spriteram;
	UINT8 *    k007121_regs;

	/* video-related */
	tilemap_t    *layer0, *layer1;
	rectangle  clip0, clip1;

	/* devices */
	running_device *konami2;
	running_device *k007121;
};




/*----------- defined in video/fastlane.c -----------*/

WRITE8_HANDLER( fastlane_vram1_w );
WRITE8_HANDLER( fastlane_vram2_w );

PALETTE_INIT( fastlane );
VIDEO_START( fastlane );
VIDEO_UPDATE( fastlane );
