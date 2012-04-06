class toypop_state : public driver_device
{
public:
	toypop_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_videoram;
	UINT8 *m_spriteram;
	UINT16 *m_bg_image;
	UINT8 *m_m68000_sharedram;
	tilemap_t *m_bg_tilemap;

	int m_bitmapflip;
	int m_palettebank;
	int m_interrupt_enable_68k;
	UINT8 m_main_irq_mask;
	UINT8 m_sound_irq_mask;
	DECLARE_READ16_MEMBER(toypop_m68000_sharedram_r);
	DECLARE_WRITE16_MEMBER(toypop_m68000_sharedram_w);
	DECLARE_READ8_MEMBER(toypop_main_interrupt_enable_r);
	DECLARE_WRITE8_MEMBER(toypop_main_interrupt_enable_w);
	DECLARE_WRITE8_MEMBER(toypop_main_interrupt_disable_w);
	DECLARE_WRITE8_MEMBER(toypop_sound_interrupt_enable_acknowledge_w);
	DECLARE_WRITE8_MEMBER(toypop_sound_interrupt_disable_w);
	DECLARE_WRITE8_MEMBER(toypop_sound_clear_w);
	DECLARE_WRITE8_MEMBER(toypop_sound_assert_w);
	DECLARE_WRITE8_MEMBER(toypop_m68000_clear_w);
	DECLARE_WRITE8_MEMBER(toypop_m68000_assert_w);
	DECLARE_WRITE16_MEMBER(toypop_m68000_interrupt_enable_w);
	DECLARE_WRITE16_MEMBER(toypop_m68000_interrupt_disable_w);
	DECLARE_WRITE8_MEMBER(toypop_videoram_w);
	DECLARE_WRITE8_MEMBER(toypop_palettebank_w);
	DECLARE_WRITE16_MEMBER(toypop_flipscreen_w);
	DECLARE_READ16_MEMBER(toypop_merged_background_r);
	DECLARE_WRITE16_MEMBER(toypop_merged_background_w);
};


/*----------- defined in video/toypop.c -----------*/

VIDEO_START( toypop );
SCREEN_UPDATE_IND16( toypop );
PALETTE_INIT( toypop );
