class legionna_state : public driver_device
{
public:
	legionna_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 *back_data;
	UINT16 *fore_data;
	UINT16 *mid_data;
	UINT16 *scrollram16;
	UINT16 *textram;
	UINT16 layer_disable;
	int sprite_xoffs;
	int sprite_yoffs;
	tilemap_t *background_layer;
	tilemap_t *foreground_layer;
	tilemap_t *midground_layer;
	tilemap_t *text_layer;
	int has_extended_banking;
	int has_extended_priority;
	UINT16 back_gfx_bank;
	UINT16 fore_gfx_bank;
	UINT16 mid_gfx_bank;
};


/*----------- defined in video/legionna.c -----------*/

void heatbrl_setgfxbank(running_machine &machine, UINT16 data);
void denjinmk_setgfxbank(running_machine &machine, UINT16 data);
WRITE16_HANDLER( legionna_background_w );
WRITE16_HANDLER( legionna_foreground_w );
WRITE16_HANDLER( legionna_midground_w );
WRITE16_HANDLER( legionna_text_w );

VIDEO_START( legionna );
VIDEO_START( cupsoc );
VIDEO_START( denjinmk );
VIDEO_START( grainbow );
VIDEO_START( godzilla );
SCREEN_UPDATE( legionna );
SCREEN_UPDATE( godzilla );
SCREEN_UPDATE( grainbow );
