class naughtyb_state : public driver_device
{
public:
	naughtyb_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *videoram;
};


/*----------- defined in video/naughtyb.c -----------*/

extern UINT8 *naughtyb_videoram2;
extern UINT8 *naughtyb_scrollreg;
extern int naughtyb_cocktail;

WRITE8_HANDLER( naughtyb_videoreg_w );
WRITE8_HANDLER( popflame_videoreg_w );

VIDEO_START( naughtyb );
PALETTE_INIT( naughtyb );
VIDEO_UPDATE( naughtyb );
