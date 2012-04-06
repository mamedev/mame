class targeth_state : public driver_device
{
public:
	targeth_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu")
		{ }

	UINT16 *m_vregs;
	UINT16 *m_videoram;
	UINT16 *m_spriteram;
	tilemap_t *m_pant[2];

	required_device<cpu_device> m_maincpu;
	DECLARE_WRITE16_MEMBER(OKIM6295_bankswitch_w);
	DECLARE_WRITE16_MEMBER(targeth_coin_counter_w);
	DECLARE_WRITE16_MEMBER(targeth_vram_w);
};


/*----------- defined in video/targeth.c -----------*/

VIDEO_START( targeth );
SCREEN_UPDATE_IND16( targeth );
