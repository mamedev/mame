class splash_state : public driver_device
{
public:
	splash_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 *vregs;
	UINT16 *videoram;
	UINT16 *spriteram;
	UINT16 *pixelram;
	UINT16 *bitmap_mode;
	int bitmap_type;
	int sprite_attr2_shift;
	tilemap_t *bg_tilemap[2];

	int adpcm_data;
	int ret;

	int vblank_irq, sound_irq;

	int msm_data1, msm_data2;
	int msm_toggle1, msm_toggle2;
	int msm_source;
	int snd_interrupt_enable1, snd_interrupt_enable2;

	UINT16 *protdata;
};


/*----------- defined in video/splash.c -----------*/

WRITE16_HANDLER( splash_vram_w );
VIDEO_START( splash );
SCREEN_UPDATE( splash );
SCREEN_UPDATE( funystrp );
