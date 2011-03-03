class dynduke_state : public driver_device
{
public:
	dynduke_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 *videoram;
	UINT16 *back_data;
	UINT16 *fore_data;
	UINT16 *scroll_ram;
	tilemap_t *bg_layer;
	tilemap_t *fg_layer;
	tilemap_t *tx_layer;
	int back_bankbase;
	int fore_bankbase;
	int back_enable;
	int fore_enable;
	int sprite_enable;
	int txt_enable;
	int old_back;
	int old_fore;
};


/*----------- defined in video/dynduke.c -----------*/

WRITE16_HANDLER( dynduke_background_w );
WRITE16_HANDLER( dynduke_foreground_w );
WRITE16_HANDLER( dynduke_text_w );
WRITE16_HANDLER( dynduke_gfxbank_w );
WRITE16_HANDLER( dynduke_control_w );
WRITE16_HANDLER( dynduke_paletteram_w );
VIDEO_START( dynduke );
SCREEN_UPDATE( dynduke );
SCREEN_EOF( dynduke );
