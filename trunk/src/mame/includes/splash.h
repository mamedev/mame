class splash_state : public driver_device
{
public:
	splash_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT16 *m_vregs;
	UINT16 *m_videoram;
	UINT16 *m_spriteram;
	UINT16 *m_pixelram;
	UINT16 *m_bitmap_mode;
	int m_bitmap_type;
	int m_sprite_attr2_shift;
	tilemap_t *m_bg_tilemap[2];

	int m_adpcm_data;
	int m_ret;

	int m_vblank_irq;
	int m_sound_irq;

	int m_msm_data1;
	int m_msm_data2;
	int m_msm_toggle1;
	int m_msm_toggle2;
	int m_msm_source;
	int m_snd_interrupt_enable1;
	int m_snd_interrupt_enable2;

	UINT16 *m_protdata;
};


/*----------- defined in video/splash.c -----------*/

WRITE16_HANDLER( splash_vram_w );
VIDEO_START( splash );
SCREEN_UPDATE( splash );
SCREEN_UPDATE( funystrp );
