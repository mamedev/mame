class dcon_state : public driver_device
{
public:
	dcon_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 *back_data;
	UINT16 *fore_data;
	UINT16 *mid_data;
	UINT16 *scroll_ram;
	UINT16 *textram;
	tilemap_t *background_layer;
	tilemap_t *foreground_layer;
	tilemap_t *midground_layer;
	tilemap_t *text_layer;
	UINT16 enable;
	int gfx_bank_select;
	int last_gfx_bank;
};


/*----------- defined in video/dcon.c -----------*/

WRITE16_HANDLER( dcon_gfxbank_w );
WRITE16_HANDLER( dcon_background_w );
WRITE16_HANDLER( dcon_foreground_w );
WRITE16_HANDLER( dcon_midground_w );
WRITE16_HANDLER( dcon_text_w );
WRITE16_HANDLER( dcon_control_w );
READ16_HANDLER( dcon_control_r );

VIDEO_START( dcon );
SCREEN_UPDATE( dcon );
SCREEN_UPDATE( sdgndmps );
