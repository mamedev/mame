class deadang_state : public driver_device
{
public:
	deadang_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 *videoram;
	UINT16 *video_data;
	UINT16 *scroll_ram;
	tilemap_t *pf3_layer;
	tilemap_t *pf2_layer;
	tilemap_t *pf1_layer;
	tilemap_t *text_layer;
	int deadangle_tilebank;
	int deadangle_oldtilebank;
};


/*----------- defined in video/deadang.c -----------*/

WRITE16_HANDLER( deadang_foreground_w );
WRITE16_HANDLER( deadang_text_w );
WRITE16_HANDLER( deadang_bank_w );

VIDEO_START( deadang );
SCREEN_UPDATE( deadang );
