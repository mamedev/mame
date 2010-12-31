/*************************************************************************

    Knuckle Joe

*************************************************************************/

class kncljoe_state : public driver_device
{
public:
	kncljoe_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *    videoram;
	UINT8 *    spriteram;
	UINT8 *    scrollregs;
	size_t     spriteram_size;

	/* video-related */
	tilemap_t    *bg_tilemap;
	int        tile_bank, sprite_bank;
	int        flipscreen;

	/* misc */
	UINT8      port1, port2;

	/* devices */
	device_t *soundcpu;
};



/*----------- defined in video/kncljoe.c -----------*/

WRITE8_HANDLER(kncljoe_videoram_w);
WRITE8_HANDLER(kncljoe_control_w);
WRITE8_HANDLER(kncljoe_scroll_w);

PALETTE_INIT( kncljoe );
VIDEO_START( kncljoe );
VIDEO_UPDATE( kncljoe );
