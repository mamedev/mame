class cabal_state : public driver_device
{
public:
	cabal_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_spriteram(*this, "spriteram"),
		m_colorram(*this, "colorram"),
		m_videoram(*this, "videoram"){ }

	required_shared_ptr<UINT16> m_spriteram;
	required_shared_ptr<UINT16> m_colorram;
	required_shared_ptr<UINT16> m_videoram;
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
	DECLARE_WRITE8_MEMBER(cabalbl_1_adpcm_w);
	DECLARE_WRITE8_MEMBER(cabalbl_2_adpcm_w);
	DECLARE_DRIVER_INIT(cabal);
	DECLARE_DRIVER_INIT(cabalbl2);
	TILE_GET_INFO_MEMBER(get_back_tile_info);
	TILE_GET_INFO_MEMBER(get_text_tile_info);
	virtual void video_start();
	DECLARE_MACHINE_RESET(cabalbl);
};


/*----------- defined in video/cabal.c -----------*/

extern VIDEO_START( cabal );
extern SCREEN_UPDATE_IND16( cabal );
