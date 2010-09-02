class bigstrkb_state : public driver_device
{
public:
	bigstrkb_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	tilemap_t *tilemap;
	tilemap_t *tilemap2;
	tilemap_t *tilemap3;

	UINT16 *videoram;
	UINT16 *videoram2;
	UINT16 *videoram3;

	UINT16 *vidreg1;
	UINT16 *vidreg2;
	UINT16 *spriteram;
};


/*----------- defined in video/bigstrkb.c -----------*/

WRITE16_HANDLER( bsb_videoram_w );
WRITE16_HANDLER( bsb_videoram2_w );
WRITE16_HANDLER( bsb_videoram3_w );
VIDEO_START(bigstrkb);
VIDEO_UPDATE(bigstrkb);
