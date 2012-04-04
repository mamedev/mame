class lvcards_state : public driver_device
{
public:
	lvcards_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 m_payout;
	UINT8 m_pulse;
	UINT8 m_result;
	UINT8 *m_videoram;
	UINT8 *m_colorram;
	tilemap_t *m_bg_tilemap;
	DECLARE_WRITE8_MEMBER(control_port_2_w);
	DECLARE_WRITE8_MEMBER(control_port_2a_w);
	DECLARE_READ8_MEMBER(payout_r);
};


/*----------- defined in video/lvcards.c -----------*/

WRITE8_HANDLER( lvcards_videoram_w );
WRITE8_HANDLER( lvcards_colorram_w );

PALETTE_INIT( lvcards );
PALETTE_INIT( ponttehk );
VIDEO_START( lvcards );
SCREEN_UPDATE_IND16( lvcards );
