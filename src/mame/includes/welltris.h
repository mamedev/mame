class welltris_state : public driver_device
{
public:
	welltris_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_spriteram(*this, "spriteram"),
		m_pixelram(*this, "pixelram"),
		m_charvideoram(*this, "charvideoram"){ }

	int m_pending_command;

	required_shared_ptr<UINT16> m_spriteram;
	required_shared_ptr<UINT16> m_pixelram;
	required_shared_ptr<UINT16> m_charvideoram;

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
	DECLARE_READ16_MEMBER(welltris_spriteram_r);
	DECLARE_WRITE16_MEMBER(welltris_spriteram_w);
	DECLARE_WRITE16_MEMBER(welltris_palette_bank_w);
	DECLARE_WRITE16_MEMBER(welltris_gfxbank_w);
	DECLARE_WRITE16_MEMBER(welltris_scrollreg_w);
	DECLARE_WRITE16_MEMBER(welltris_charvideoram_w);
	void setbank(int num, int bank);
	DECLARE_CUSTOM_INPUT_MEMBER(pending_sound_r);
	DECLARE_DRIVER_INIT(quiz18k);
	DECLARE_DRIVER_INIT(welltris);
};


/*----------- defined in video/welltris.c -----------*/


VIDEO_START( welltris );
SCREEN_UPDATE_IND16( welltris );
