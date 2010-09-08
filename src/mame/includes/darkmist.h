class darkmist_state : public driver_device
{
public:
	darkmist_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *videoram;
};


/*----------- defined in drivers/darkmist.c -----------*/

extern int darkmist_hw;


/*----------- defined in video/darkmist.c -----------*/

VIDEO_START( darkmist );
VIDEO_UPDATE( darkmist );
PALETTE_INIT( darkmist );

extern UINT8 *darkmist_scroll;
extern UINT8 *darkmist_spritebank;
