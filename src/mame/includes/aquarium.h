


class aquarium_state : public driver_device
{
public:
	aquarium_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_mid_videoram(*this, "mid_videoram"),
		m_bak_videoram(*this, "bak_videoram"),
		m_txt_videoram(*this, "txt_videoram"),
		m_spriteram(*this, "spriteram"),
		m_scroll(*this, "scroll"){ }

	/* memory pointers */
	required_shared_ptr<UINT16> m_mid_videoram;
	required_shared_ptr<UINT16> m_bak_videoram;
	required_shared_ptr<UINT16> m_txt_videoram;
	required_shared_ptr<UINT16> m_spriteram;
	required_shared_ptr<UINT16> m_scroll;
//  UINT16 *  m_paletteram;   // currently this uses generic palette handling

	/* video-related */
	tilemap_t  *m_txt_tilemap;
	tilemap_t  *m_mid_tilemap;
	tilemap_t  *m_bak_tilemap;

	/* misc */
	int m_aquarium_snd_ack;

	/* devices */
	cpu_device *m_audiocpu;
	DECLARE_READ16_MEMBER(aquarium_coins_r);
	DECLARE_WRITE8_MEMBER(aquarium_snd_ack_w);
	DECLARE_WRITE16_MEMBER(aquarium_sound_w);
	DECLARE_WRITE8_MEMBER(aquarium_z80_bank_w);
	DECLARE_READ8_MEMBER(aquarium_oki_r);
	DECLARE_WRITE8_MEMBER(aquarium_oki_w);
	DECLARE_WRITE16_MEMBER(aquarium_txt_videoram_w);
	DECLARE_WRITE16_MEMBER(aquarium_mid_videoram_w);
	DECLARE_WRITE16_MEMBER(aquarium_bak_videoram_w);
	DECLARE_DRIVER_INIT(aquarium);
	TILE_GET_INFO_MEMBER(get_aquarium_txt_tile_info);
	TILE_GET_INFO_MEMBER(get_aquarium_mid_tile_info);
	TILE_GET_INFO_MEMBER(get_aquarium_bak_tile_info);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
};


/*----------- defined in video/aquarium.c -----------*/



SCREEN_UPDATE_IND16(aquarium);
