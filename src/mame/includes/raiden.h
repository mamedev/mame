class raiden_state : public driver_device
{
public:
	raiden_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 *videoram;
};


/*----------- defined in video/raiden.c -----------*/

extern UINT16 *raiden_back_data,*raiden_fore_data,*raiden_scroll_ram;

WRITE16_HANDLER( raiden_background_w );
WRITE16_HANDLER( raiden_foreground_w );
WRITE16_HANDLER( raiden_text_w );
VIDEO_START( raiden );
VIDEO_START( raidena );
WRITE16_HANDLER( raiden_control_w );
WRITE16_HANDLER( raidena_control_w );
VIDEO_UPDATE( raiden );
