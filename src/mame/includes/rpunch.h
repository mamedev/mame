class rpunch_state : public driver_device
{
public:
	rpunch_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 *videoram;
	UINT8 sound_data;
	UINT8 sound_busy;
	UINT8 ym2151_irq;
	UINT8 upd_rom_bank;
	UINT16 *bitmapram;
	size_t bitmapram_size;
	int sprite_palette;
	tilemap_t *background[2];
	UINT16 videoflags;
	UINT8 crtc_register;
	emu_timer *crtc_timer;
	UINT8 bins;
	UINT8 gins;
	UINT16 *spriteram;
};


/*----------- defined in video/rpunch.c -----------*/

VIDEO_START( rpunch );
SCREEN_UPDATE( rpunch );

WRITE16_HANDLER( rpunch_videoram_w );
WRITE16_HANDLER( rpunch_videoreg_w );
WRITE16_HANDLER( rpunch_scrollreg_w );
WRITE16_HANDLER( rpunch_ins_w );
WRITE16_HANDLER( rpunch_crtc_data_w );
WRITE16_HANDLER( rpunch_crtc_register_w );
