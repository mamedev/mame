class spcforce_state : public driver_device
{
public:
	spcforce_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *m_scrollram;
	UINT8 *m_videoram;
	UINT8 *m_colorram;

	int m_sn76496_latch;
	int m_sn76496_select;
};


/*----------- defined in video/spcforce.c -----------*/

WRITE8_HANDLER( spcforce_flip_screen_w );
SCREEN_UPDATE( spcforce );
