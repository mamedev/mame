class magmax_state : public driver_device
{
public:
	magmax_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 *videoram;
	UINT8 sound_latch;
	UINT8 LS74_clr;
	UINT8 LS74_q;
	UINT8 gain_control;
	emu_timer *interrupt_timer;
	UINT16 *vreg;
	UINT16 *scroll_x;
	UINT16 *scroll_y;
	int flipscreen;
	UINT32 *prom_tab;
};


/*----------- defined in video/magmax.c -----------*/

PALETTE_INIT( magmax );
SCREEN_UPDATE( magmax );
VIDEO_START( magmax );
