/***************************************************************************

    Track'n'Field

***************************************************************************/


class trackfld_state : public driver_device
{
public:
	trackfld_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *  videoram;
	UINT8 *  colorram;
	UINT8 *  scroll;
	UINT8 *  scroll2;
	UINT8 *  spriteram;
	UINT8 *  spriteram2;
	size_t   spriteram_size;

	/* video-related */
	tilemap_t  *bg_tilemap;
	int      bg_bank, sprite_bank1, sprite_bank2;
	int      old_gfx_bank;					// needed by atlantol
	int		 sprites_gfx_banked;
};


/*----------- defined in video/trackfld.c -----------*/

WRITE8_HANDLER( trackfld_videoram_w );
WRITE8_HANDLER( trackfld_colorram_w );
WRITE8_HANDLER( trackfld_flipscreen_w );
WRITE8_HANDLER( atlantol_gfxbank_w );

PALETTE_INIT( trackfld );
VIDEO_START( trackfld );
VIDEO_UPDATE( trackfld );
VIDEO_START( atlantol );

