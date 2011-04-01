class truco_state : public driver_device
{
public:
	truco_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *m_videoram;
	UINT8 *m_battery_ram;
	int m_trigger;
};


/*----------- defined in video/truco.c -----------*/

SCREEN_UPDATE( truco );
PALETTE_INIT( truco );
