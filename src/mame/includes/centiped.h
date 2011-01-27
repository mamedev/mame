/*************************************************************************

    Atari Centipede hardware

*************************************************************************/

class centiped_state : public driver_device
{
public:
	centiped_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *videoram;
	UINT8 oldpos[4];
	UINT8 sign[4];
	UINT8 dsw_select;
	UINT8 control_select;
	UINT8 *rambase;
	UINT8 flipscreen;
	UINT8 *bullsdrt_tiles_bankram;
	tilemap_t *bg_tilemap;
	UINT8 bullsdrt_sprites_bank;
	UINT8 penmask[64];
};


/*----------- defined in video/centiped.c -----------*/

PALETTE_INIT( warlords );

VIDEO_START( centiped );
VIDEO_START( milliped );
VIDEO_START( warlords );
VIDEO_START( bullsdrt );

VIDEO_UPDATE( centiped );
VIDEO_UPDATE( milliped );
VIDEO_UPDATE( warlords );
VIDEO_UPDATE( bullsdrt );

WRITE8_HANDLER( centiped_paletteram_w );
WRITE8_HANDLER( milliped_paletteram_w );

WRITE8_HANDLER( centiped_videoram_w );
WRITE8_HANDLER( centiped_flip_screen_w );
WRITE8_HANDLER( bullsdrt_tilesbank_w );
WRITE8_HANDLER( bullsdrt_sprites_bank_w );

WRITE8_HANDLER( mazeinv_paletteram_w );
