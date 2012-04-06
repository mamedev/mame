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
	DECLARE_WRITE16_MEMBER(splash_sh_irqtrigger_w);
	DECLARE_WRITE16_MEMBER(roldf_sh_irqtrigger_w);
	DECLARE_WRITE16_MEMBER(splash_coin_w);
	DECLARE_WRITE8_MEMBER(splash_adpcm_data_w);
	DECLARE_READ16_MEMBER(roldfrog_bombs_r);
	DECLARE_WRITE8_MEMBER(sound_bank_w);
	DECLARE_WRITE8_MEMBER(roldfrog_vblank_ack_w);
	DECLARE_READ8_MEMBER(roldfrog_unk_r);
	DECLARE_READ16_MEMBER(spr_read);
	DECLARE_WRITE16_MEMBER(spr_write);
	DECLARE_WRITE16_MEMBER(funystrp_sh_irqtrigger_w);
	DECLARE_READ8_MEMBER(int_source_r);
	DECLARE_WRITE8_MEMBER(msm1_data_w);
	DECLARE_WRITE8_MEMBER(msm1_interrupt_w);
	DECLARE_WRITE8_MEMBER(msm2_interrupt_w);
	DECLARE_WRITE8_MEMBER(msm2_data_w);
	DECLARE_WRITE16_MEMBER(splash_vram_w);
};


/*----------- defined in video/splash.c -----------*/

VIDEO_START( splash );
SCREEN_UPDATE_IND16( splash );
SCREEN_UPDATE_IND16( funystrp );
