class wiz_state : public driver_device
{
public:
	wiz_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_videoram;
	int m_dsc0;
	int m_dsc1;
	UINT8 *m_videoram2;
	UINT8 *m_colorram2;
	UINT8 *m_attributesram;
	UINT8 *m_attributesram2;
	UINT8 *m_sprite_bank;
	INT32 m_flipx;
	INT32 m_flipy;
	INT32 m_bgpen;
	UINT8 m_char_bank[2];
	UINT8 m_palbank[2];
	int m_palette_bank;
	UINT8 *m_spriteram;
	UINT8 *m_spriteram2;
	size_t m_spriteram_size;
	UINT8 m_main_nmi_mask;
	UINT8 m_sound_nmi_mask;
	DECLARE_WRITE8_MEMBER(sound_command_w);
	DECLARE_READ8_MEMBER(wiz_protection_r);
	DECLARE_WRITE8_MEMBER(wiz_coin_counter_w);
	DECLARE_WRITE8_MEMBER(wiz_main_nmi_mask_w);
	DECLARE_WRITE8_MEMBER(wiz_sound_nmi_mask_w);
	DECLARE_WRITE8_MEMBER(wiz_palettebank_w);
	DECLARE_WRITE8_MEMBER(wiz_bgcolor_w);
	DECLARE_WRITE8_MEMBER(wiz_char_bank_select_w);
	DECLARE_WRITE8_MEMBER(wiz_flipx_w);
	DECLARE_WRITE8_MEMBER(wiz_flipy_w);
};


/*----------- defined in video/wiz.c -----------*/


VIDEO_START( wiz );
PALETTE_INIT( wiz );
SCREEN_UPDATE_IND16( wiz );
SCREEN_UPDATE_IND16( stinger );
SCREEN_UPDATE_IND16( kungfut );
