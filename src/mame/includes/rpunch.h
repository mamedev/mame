class rpunch_state : public driver_device
{
public:
	rpunch_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 *videoram;
};


/*----------- defined in video/rpunch.c -----------*/

extern UINT16 *rpunch_bitmapram;
extern size_t rpunch_bitmapram_size;
extern int rpunch_sprite_palette;

VIDEO_START( rpunch );
VIDEO_UPDATE( rpunch );

WRITE16_HANDLER( rpunch_videoram_w );
WRITE16_HANDLER( rpunch_videoreg_w );
WRITE16_HANDLER( rpunch_scrollreg_w );
WRITE16_HANDLER( rpunch_ins_w );
WRITE16_HANDLER( rpunch_crtc_data_w );
WRITE16_HANDLER( rpunch_crtc_register_w );
