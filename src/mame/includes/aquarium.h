


class aquarium_state : public driver_device
{
public:
	aquarium_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT16 *  m_scroll;
	UINT16 *  m_txt_videoram;
	UINT16 *  m_mid_videoram;
	UINT16 *  m_bak_videoram;
	UINT16 *  m_spriteram;
//  UINT16 *  m_paletteram;   // currently this uses generic palette handling
	size_t    m_spriteram_size;

	/* video-related */
	tilemap_t  *m_txt_tilemap;
	tilemap_t  *m_mid_tilemap;
	tilemap_t  *m_bak_tilemap;

	/* misc */
	int m_aquarium_snd_ack;

	/* devices */
	device_t *m_audiocpu;
	DECLARE_READ16_MEMBER(aquarium_coins_r);
	DECLARE_WRITE8_MEMBER(aquarium_snd_ack_w);
	DECLARE_WRITE16_MEMBER(aquarium_sound_w);
	DECLARE_WRITE8_MEMBER(aquarium_z80_bank_w);
	DECLARE_READ8_MEMBER(aquarium_oki_r);
	DECLARE_WRITE8_MEMBER(aquarium_oki_w);
	DECLARE_WRITE16_MEMBER(aquarium_txt_videoram_w);
	DECLARE_WRITE16_MEMBER(aquarium_mid_videoram_w);
	DECLARE_WRITE16_MEMBER(aquarium_bak_videoram_w);
};


/*----------- defined in video/aquarium.c -----------*/


VIDEO_START(aquarium);
SCREEN_UPDATE_IND16(aquarium);
