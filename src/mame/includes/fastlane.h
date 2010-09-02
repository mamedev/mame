/*************************************************************************

    Fast Lane

*************************************************************************/

class fastlane_state : public driver_device
{
public:
	fastlane_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

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
