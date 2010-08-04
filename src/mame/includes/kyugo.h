/***************************************************************************

    Kyugo hardware games

***************************************************************************/

class kyugo_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, kyugo_state(machine)); }

	kyugo_state(running_machine &machine)
		: driver_data_t(machine) { }

	/* memory pointers */
	UINT8 *     fgvideoram;
	UINT8 *     bgvideoram;
	UINT8 *     bgattribram;
	UINT8 *     spriteram_1;
	UINT8 *     spriteram_2;
	UINT8 *     shared_ram;

	/* video-related */
	tilemap_t     *bg_tilemap, *fg_tilemap;
	UINT8       scroll_x_lo, scroll_x_hi, scroll_y;
	int         bgpalbank, fgcolor;
	int         flipscreen;
	const UINT8 *color_codes;

	/* devices */
	running_device *maincpu;
	running_device *subcpu;
};


/*----------- defined in video/kyugo.c -----------*/

READ8_HANDLER( kyugo_spriteram_2_r );

WRITE8_HANDLER( kyugo_fgvideoram_w );
WRITE8_HANDLER( kyugo_bgvideoram_w );
WRITE8_HANDLER( kyugo_bgattribram_w );
WRITE8_HANDLER( kyugo_scroll_x_lo_w );
WRITE8_HANDLER( kyugo_gfxctrl_w );
WRITE8_HANDLER( kyugo_scroll_y_w );
WRITE8_HANDLER( kyugo_flipscreen_w );

VIDEO_START( kyugo );
VIDEO_UPDATE( kyugo );
