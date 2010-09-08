class strnskil_state : public driver_device
{
public:
	strnskil_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *videoram;
};


/*----------- defined in video/strnskil.c -----------*/

extern UINT8 *strnskil_xscroll;

WRITE8_HANDLER( strnskil_videoram_w );
WRITE8_HANDLER( strnskil_scrl_ctrl_w );

PALETTE_INIT( strnskil );
VIDEO_START( strnskil );
VIDEO_UPDATE( strnskil );
