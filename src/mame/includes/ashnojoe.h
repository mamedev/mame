/*************************************************************************

    Success Joe / Ashita no Joe

*************************************************************************/

class ashnojoe_state : public driver_device
{
public:
	ashnojoe_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT16 *    tileram;
	UINT16 *    tileram_1;
	UINT16 *    tileram_2;
	UINT16 *    tileram_3;
	UINT16 *    tileram_4;
	UINT16 *    tileram_5;
	UINT16 *    tileram_6;
	UINT16 *    tileram_7;
	UINT16 *    tilemap_reg;
//  UINT16 *    paletteram; // currently this uses generic palette handling

	/* video-related */
	tilemap_t     *joetilemap, *joetilemap2, *joetilemap3, *joetilemap4, *joetilemap5, *joetilemap6, *joetilemap7;

	/* sound-related */
	UINT8       adpcm_byte;
	int         soundlatch_status;
	int         msm5205_vclk_toggle;

	/* devices */
	device_t *audiocpu;
};


/*----------- defined in video/ashnojoe.c -----------*/

WRITE16_HANDLER( ashnojoe_tileram_w );
WRITE16_HANDLER( ashnojoe_tileram2_w );
WRITE16_HANDLER( ashnojoe_tileram3_w );
WRITE16_HANDLER( ashnojoe_tileram4_w );
WRITE16_HANDLER( ashnojoe_tileram5_w );
WRITE16_HANDLER( ashnojoe_tileram6_w );
WRITE16_HANDLER( ashnojoe_tileram7_w );
WRITE16_HANDLER( joe_tilemaps_xscroll_w );
WRITE16_HANDLER( joe_tilemaps_yscroll_w );

VIDEO_START( ashnojoe );
VIDEO_UPDATE( ashnojoe );
