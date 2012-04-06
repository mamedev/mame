class cabal_state : public driver_device
{
public:
	cabal_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT16 *m_spriteram;
	UINT16 *m_colorram;
	UINT16 *m_videoram;
	size_t m_spriteram_size;
	tilemap_t *m_background_layer;
	tilemap_t *m_text_layer;
	int m_sound_command1;
	int m_sound_command2;
	int m_last[4];
	DECLARE_WRITE16_MEMBER(cabalbl_sndcmd_w);
	DECLARE_WRITE16_MEMBER(track_reset_w);
	DECLARE_READ16_MEMBER(track_r);
	DECLARE_WRITE16_MEMBER(cabal_sound_irq_trigger_word_w);
	DECLARE_WRITE16_MEMBER(cabalbl_sound_irq_trigger_word_w);
	DECLARE_READ8_MEMBER(cabalbl_snd2_r);
	DECLARE_READ8_MEMBER(cabalbl_snd1_r);
	DECLARE_WRITE8_MEMBER(cabalbl_coin_w);
	DECLARE_WRITE16_MEMBER(cabal_flipscreen_w);
	DECLARE_WRITE16_MEMBER(cabal_background_videoram16_w);
	DECLARE_WRITE16_MEMBER(cabal_text_videoram16_w);
};


/*----------- defined in video/cabal.c -----------*/

extern VIDEO_START( cabal );
extern SCREEN_UPDATE_IND16( cabal );
