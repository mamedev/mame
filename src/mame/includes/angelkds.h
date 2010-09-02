/*************************************************************************

    Angel Kids

*************************************************************************/

class angelkds_state : public driver_device
{
public:
	angelkds_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *    paletteram;
	UINT8 *    spriteram;
	UINT8 *    txvideoram;
	UINT8 *    bgtopvideoram;
	UINT8 *    bgbotvideoram;

	tilemap_t    *tx_tilemap, *bgbot_tilemap, *bgtop_tilemap;
	int        txbank;
	int        bgbotbank;
	int        bgtopbank;

	UINT8      sound[4];
	UINT8      sound2[4];
	UINT8      layer_ctrl;

	/* devices */
	running_device *subcpu;
};


/*----------- defined in video/angelkds.c -----------*/

WRITE8_HANDLER( angelkds_bgtopvideoram_w );
WRITE8_HANDLER( angelkds_bgbotvideoram_w );
WRITE8_HANDLER( angelkds_txvideoram_w );

WRITE8_HANDLER( angelkds_bgtopbank_write );
WRITE8_HANDLER( angelkds_bgtopscroll_write );
WRITE8_HANDLER( angelkds_bgbotbank_write );
WRITE8_HANDLER( angelkds_bgbotscroll_write );
WRITE8_HANDLER( angelkds_txbank_write );

WRITE8_HANDLER( angelkds_paletteram_w );
WRITE8_HANDLER( angelkds_layer_ctrl_write );

VIDEO_START( angelkds );
VIDEO_UPDATE( angelkds );
