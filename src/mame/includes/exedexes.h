/*************************************************************************

    Exed Exes

*************************************************************************/


class exedexes_state : public driver_device
{
public:
	exedexes_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *        videoram;
	UINT8 *        colorram;
	UINT8 *        bg_scroll;
	UINT8 *        nbg_yscroll;
	UINT8 *        nbg_xscroll;
//  UINT8 *        spriteram;   // currently this uses generic buffered_spriteram

	/* video-related */
	tilemap_t        *bg_tilemap, *fg_tilemap, *tx_tilemap;
	int            chon, objon, sc1on, sc2on;
};



/*----------- defined in video/exedexes.c -----------*/

extern WRITE8_HANDLER( exedexes_videoram_w );
extern WRITE8_HANDLER( exedexes_colorram_w );
extern WRITE8_HANDLER( exedexes_c804_w );
extern WRITE8_HANDLER( exedexes_gfxctrl_w );

extern PALETTE_INIT( exedexes );
extern VIDEO_START( exedexes );
extern VIDEO_UPDATE( exedexes );
extern VIDEO_EOF( exedexes );
