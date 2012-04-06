class suna16_state : public driver_device
{
public:
	suna16_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu")
		{ }

	UINT16 m_prot;
	UINT16 *m_paletteram;
	UINT16 *m_spriteram;
	UINT16 *m_spriteram2;
	int m_color_bank;

	required_device<cpu_device> m_maincpu;
	DECLARE_WRITE16_MEMBER(suna16_soundlatch_w);
	DECLARE_WRITE16_MEMBER(bssoccer_leds_w);
	DECLARE_WRITE16_MEMBER(uballoon_leds_w);
	DECLARE_WRITE16_MEMBER(bestbest_coin_w);
	DECLARE_READ16_MEMBER(bestbest_prot_r);
	DECLARE_WRITE16_MEMBER(bestbest_prot_w);
	DECLARE_WRITE8_MEMBER(bssoccer_pcm_1_bankswitch_w);
	DECLARE_WRITE8_MEMBER(bssoccer_pcm_2_bankswitch_w);
	DECLARE_WRITE8_MEMBER(uballoon_pcm_1_bankswitch_w);
	DECLARE_WRITE16_MEMBER(suna16_flipscreen_w);
	DECLARE_WRITE16_MEMBER(bestbest_flipscreen_w);
	DECLARE_READ16_MEMBER(suna16_paletteram16_r);
	DECLARE_WRITE16_MEMBER(suna16_paletteram16_w);
};


/*----------- defined in video/suna16.c -----------*/



VIDEO_START( suna16 );
SCREEN_UPDATE_IND16( suna16 );
SCREEN_UPDATE_IND16( bestbest );
