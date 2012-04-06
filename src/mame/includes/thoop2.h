class thoop2_state : public driver_device
{
public:
	thoop2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT16 *m_vregs;
	UINT16 *m_videoram;
	UINT16 *m_spriteram;
	int m_sprite_count[5];
	int *m_sprite_table[5];
	tilemap_t *m_pant[2];
	DECLARE_WRITE16_MEMBER(OKIM6295_bankswitch_w);
	DECLARE_WRITE16_MEMBER(thoop2_coin_w);
	DECLARE_READ16_MEMBER(DS5002FP_R);
	DECLARE_WRITE16_MEMBER(thoop2_vram_w);
};


/*----------- defined in video/thoop2.c -----------*/

VIDEO_START( thoop2 );
SCREEN_UPDATE_IND16( thoop2 );
