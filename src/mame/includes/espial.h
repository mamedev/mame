/***************************************************************************

 Espial hardware games (drivers: espial.c)

***************************************************************************/

class espial_state : public driver_device
{
public:
	espial_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *   videoram;
	UINT8 *   colorram;
	UINT8 *   attributeram;
	UINT8 *   scrollram;
	UINT8 *   spriteram_1;
	UINT8 *   spriteram_2;
	UINT8 *   spriteram_3;

	/* video-related */
	tilemap_t   *bg_tilemap, *fg_tilemap;
	int       flipscreen;

	/* sound-related */
	UINT8     sound_nmi_enabled;

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
};

/*----------- defined in video/espial.c -----------*/

PALETTE_INIT( espial );
VIDEO_START( espial );
VIDEO_START( netwars );
WRITE8_HANDLER( espial_videoram_w );
WRITE8_HANDLER( espial_colorram_w );
WRITE8_HANDLER( espial_attributeram_w );
WRITE8_HANDLER( espial_scrollram_w );
WRITE8_HANDLER( espial_flipscreen_w );
VIDEO_UPDATE( espial );
