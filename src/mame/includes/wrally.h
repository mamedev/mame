class wrally_state : public driver_device
{
public:
	wrally_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT16 *m_shareram;
	tilemap_t *m_pant[2];
	UINT16 *m_vregs;
	UINT16 *m_videoram;
	UINT16 *m_spriteram;
	DECLARE_READ8_MEMBER(dallas_share_r);
	DECLARE_WRITE8_MEMBER(dallas_share_w);
};


/*----------- defined in machine/wrally.c -----------*/

WRITE16_HANDLER( wrally_vram_w );
WRITE16_HANDLER( wrally_flipscreen_w );
WRITE16_HANDLER( OKIM6295_bankswitch_w );
WRITE16_HANDLER( wrally_coin_counter_w );
WRITE16_HANDLER( wrally_coin_lockout_w );

/*----------- defined in video/wrally.c -----------*/

VIDEO_START( wrally );
SCREEN_UPDATE_IND16( wrally );

