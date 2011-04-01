class pcktgal_state : public driver_device
{
public:
	pcktgal_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	int m_msm5205next;
	int m_toggle;
	UINT8 *m_spriteram;
	size_t m_spriteram_size;
};


/*----------- defined in video/pcktgal.c -----------*/


PALETTE_INIT( pcktgal );
SCREEN_UPDATE( pcktgal );
SCREEN_UPDATE( pcktgalb );
