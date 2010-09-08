class dynduke_state : public driver_device
{
public:
	dynduke_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 *videoram;
};


/*----------- defined in video/dynduke.c -----------*/

extern UINT16 *dynduke_back_data, *dynduke_fore_data, *dynduke_scroll_ram;

WRITE16_HANDLER( dynduke_background_w );
WRITE16_HANDLER( dynduke_foreground_w );
WRITE16_HANDLER( dynduke_text_w );
WRITE16_HANDLER( dynduke_gfxbank_w );
WRITE16_HANDLER( dynduke_control_w );
WRITE16_HANDLER( dynduke_paletteram_w );
VIDEO_START( dynduke );
VIDEO_UPDATE( dynduke );
VIDEO_EOF( dynduke );
