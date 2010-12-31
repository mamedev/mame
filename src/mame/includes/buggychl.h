/*
    buggychl
*/

class buggychl_state : public driver_device
{
public:
	buggychl_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *     videoram;
	UINT8 *     spriteram;
	UINT8 *     scrollv;
	UINT8 *     scrollh;
	UINT8 *     charram;
	size_t      videoram_size;
	size_t      spriteram_size;

	/* video-related */
	bitmap_t    *tmp_bitmap1, *tmp_bitmap2;
	tilemap_t     *bg_tilemap;
	int         sl_bank, bg_on, sky_on, sprite_color_base, bg_scrollx;
	UINT8       sprite_lookup[0x2000];

	/* sound-related */
	int         sound_nmi_enable, pending_nmi;

	/* devices */
	device_t *audiocpu;
};


/*----------- defined in video/buggychl.c -----------*/

WRITE8_HANDLER( buggychl_chargen_w );
WRITE8_HANDLER( buggychl_sprite_lookup_bank_w );
WRITE8_HANDLER( buggychl_sprite_lookup_w );
WRITE8_HANDLER( buggychl_ctrl_w );
WRITE8_HANDLER( buggychl_bg_scrollx_w );

PALETTE_INIT( buggychl );
VIDEO_START( buggychl );
VIDEO_UPDATE( buggychl );
