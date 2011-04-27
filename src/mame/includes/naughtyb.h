class naughtyb_state : public driver_device
{
public:
	naughtyb_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_videoram;
	UINT8 m_popflame_prot_seed;
	int m_r_index;
	int m_prot_count;
	int m_question_offset;
	UINT8 *m_videoram2;
	UINT8 *m_scrollreg;
	int m_cocktail;
	UINT8 m_palreg;
	int m_bankreg;
};


/*----------- defined in video/naughtyb.c -----------*/

WRITE8_HANDLER( naughtyb_videoreg_w );
WRITE8_HANDLER( popflame_videoreg_w );

VIDEO_START( naughtyb );
PALETTE_INIT( naughtyb );
SCREEN_UPDATE( naughtyb );
