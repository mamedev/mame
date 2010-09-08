class magmax_state : public driver_device
{
public:
	magmax_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 *videoram;
};


/*----------- defined in video/magmax.c -----------*/

extern UINT16 *magmax_vreg;
extern UINT16 *magmax_scroll_x;
extern UINT16 *magmax_scroll_y;

PALETTE_INIT( magmax );
VIDEO_UPDATE( magmax );
VIDEO_START( magmax );
