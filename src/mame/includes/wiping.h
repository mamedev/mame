class wiping_state : public driver_device
{
public:
	wiping_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_videoram;
	UINT8 *m_colorram;
	int m_flipscreen;
	UINT8 *m_soundregs;
	UINT8 *m_spriteram;
	size_t m_spriteram_size;

	UINT8 m_main_irq_mask;
	UINT8 m_sound_irq_mask;
	DECLARE_READ8_MEMBER(ports_r);
	DECLARE_WRITE8_MEMBER(subcpu_reset_w);
	DECLARE_WRITE8_MEMBER(main_irq_mask_w);
	DECLARE_WRITE8_MEMBER(sound_irq_mask_w);
	DECLARE_WRITE8_MEMBER(wiping_flipscreen_w);
};


/*----------- defined in video/wiping.c -----------*/

PALETTE_INIT( wiping );
SCREEN_UPDATE_IND16( wiping );

