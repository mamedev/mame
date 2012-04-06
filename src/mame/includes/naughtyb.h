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
	bitmap_ind16 m_tmpbitmap;
	DECLARE_READ8_MEMBER(in0_port_r);
	DECLARE_READ8_MEMBER(dsw0_port_r);
	DECLARE_READ8_MEMBER(popflame_protection_r);
	DECLARE_WRITE8_MEMBER(popflame_protection_w);
	DECLARE_READ8_MEMBER(trvmstr_questions_r);
	DECLARE_WRITE8_MEMBER(trvmstr_questions_w);
	DECLARE_WRITE8_MEMBER(naughtyb_videoreg_w);
	DECLARE_WRITE8_MEMBER(popflame_videoreg_w);
};


/*----------- defined in video/naughtyb.c -----------*/


VIDEO_START( naughtyb );
PALETTE_INIT( naughtyb );
SCREEN_UPDATE_IND16( naughtyb );
