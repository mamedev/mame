class welltris_state : public driver_device
{
public:
	welltris_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	int m_pending_command;

	UINT16 *m_spriteram;
	UINT16 *m_pixelram;
	UINT16 *m_charvideoram;

	tilemap_t *m_char_tilemap;
	UINT8 m_gfxbank[8];
	UINT16 m_charpalettebank;
	UINT16 m_spritepalettebank;
	UINT16 m_pixelpalettebank;
	int m_scrollx;
	int m_scrolly;
	DECLARE_WRITE8_MEMBER(welltris_sh_bankswitch_w);
	DECLARE_WRITE16_MEMBER(sound_command_w);
	DECLARE_WRITE8_MEMBER(pending_command_clear_w);
};


/*----------- defined in video/welltris.c -----------*/

//READ16_HANDLER( welltris_spriteram_r );
WRITE16_HANDLER( welltris_spriteram_w );
WRITE16_HANDLER( welltris_palette_bank_w );
WRITE16_HANDLER( welltris_gfxbank_w );
WRITE16_HANDLER( welltris_charvideoram_w );
WRITE16_HANDLER( welltris_scrollreg_w );

VIDEO_START( welltris );
SCREEN_UPDATE_IND16( welltris );
