/***************************************************************************

    Bionic Commando

***************************************************************************/

class bionicc_state : public driver_device
{
public:
	bionicc_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT16 *  bgvideoram;
	UINT16 *  fgvideoram;
	UINT16 *  txvideoram;
	UINT16 *  paletteram;
//  UINT16 *  spriteram;  // needed for EOF, but currently handled through buffer_spriteram16

	/* video-related */
	tilemap_t   *tx_tilemap, *bg_tilemap, *fg_tilemap;
	UINT16    scroll[4];

	UINT16    inp[3];
	UINT16    soundcommand;
};


/*----------- defined in video/bionicc.c -----------*/

WRITE16_HANDLER( bionicc_fgvideoram_w );
WRITE16_HANDLER( bionicc_bgvideoram_w );
WRITE16_HANDLER( bionicc_txvideoram_w );
WRITE16_HANDLER( bionicc_paletteram_w );
WRITE16_HANDLER( bionicc_scroll_w );
WRITE16_HANDLER( bionicc_gfxctrl_w );

VIDEO_START( bionicc );
VIDEO_UPDATE( bionicc );
VIDEO_EOF( bionicc );
