class truco_state : public driver_device
{
public:
	truco_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_videoram;
	UINT8 *m_battery_ram;
	int m_trigger;
};


/*----------- defined in video/truco.c -----------*/

SCREEN_UPDATE( truco );
PALETTE_INIT( truco );
