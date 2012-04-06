class ms32_state : public driver_device
{
public:
	ms32_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_nvram_8;
	UINT32 *m_mahjong_input_select;
	UINT32 m_to_main;
	UINT16 m_irqreq;
	tilemap_t *m_tx_tilemap;
	tilemap_t *m_roz_tilemap;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_bg_tilemap_alt;
	UINT8* m_priram_8;
	UINT16* m_palram_16;
	UINT16* m_rozram_16;
	UINT16* m_lineram_16;
	UINT16* m_sprram_16;
	UINT16* m_txram_16;
	UINT16* m_bgram_16;
	UINT32 m_tilemaplayoutcontrol;
	UINT16* m_f1superb_extraram_16;
	tilemap_t* m_extra_tilemap;
	UINT32 *m_roz_ctrl;
	UINT32 *m_tx_scroll;
	UINT32 *m_bg_scroll;
	UINT32 *m_mainram;
	bitmap_ind16 m_temp_bitmap_tilemaps;
	bitmap_ind16 m_temp_bitmap_sprites;
	bitmap_ind8 m_temp_bitmap_sprites_pri;
	int m_reverse_sprite_order;
	int m_flipscreen;
	UINT32 m_brt[4];
	int m_brt_r;
	int m_brt_g;
	int m_brt_b;
	DECLARE_READ32_MEMBER(ms32_read_inputs3);
	DECLARE_WRITE32_MEMBER(ms32_sound_w);
	DECLARE_READ32_MEMBER(ms32_sound_r);
	DECLARE_WRITE32_MEMBER(reset_sub_w);
	DECLARE_READ8_MEMBER(ms32_nvram_r8);
	DECLARE_WRITE8_MEMBER(ms32_nvram_w8);
	DECLARE_READ8_MEMBER(ms32_priram_r8);
	DECLARE_WRITE8_MEMBER(ms32_priram_w8);
	DECLARE_READ16_MEMBER(ms32_palram_r16);
	DECLARE_WRITE16_MEMBER(ms32_palram_w16);
	DECLARE_READ16_MEMBER(ms32_rozram_r16);
	DECLARE_WRITE16_MEMBER(ms32_rozram_w16);
	DECLARE_READ16_MEMBER(ms32_lineram_r16);
	DECLARE_WRITE16_MEMBER(ms32_lineram_w16);
	DECLARE_READ16_MEMBER(ms32_sprram_r16);
	DECLARE_WRITE16_MEMBER(ms32_sprram_w16);
	DECLARE_READ16_MEMBER(ms32_txram_r16);
	DECLARE_WRITE16_MEMBER(ms32_txram_w16);
	DECLARE_READ16_MEMBER(ms32_bgram_r16);
	DECLARE_WRITE16_MEMBER(ms32_bgram_w16);
	DECLARE_WRITE32_MEMBER(pip_w);
	DECLARE_WRITE16_MEMBER(ms32_extra_w16);
	DECLARE_READ16_MEMBER(ms32_extra_r16);
	DECLARE_WRITE32_MEMBER(ms32_irq2_guess_w);
	DECLARE_WRITE32_MEMBER(ms32_irq5_guess_w);
	DECLARE_READ8_MEMBER(latch_r);
	DECLARE_WRITE8_MEMBER(ms32_snd_bank_w);
	DECLARE_WRITE8_MEMBER(to_main_w);
	DECLARE_WRITE32_MEMBER(ms32_brightness_w);
	DECLARE_WRITE32_MEMBER(ms32_gfxctrl_w);
};


/*----------- defined in video/ms32.c -----------*/

//extern UINT32 *ms32_fce00000;



VIDEO_START( ms32 );
VIDEO_START( f1superb );
SCREEN_UPDATE_RGB32( ms32 );
