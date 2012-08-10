class funworld_state : public driver_device
{
public:
	funworld_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"){ }

	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;
	tilemap_t *m_bg_tilemap;
	DECLARE_READ8_MEMBER(questions_r);
	DECLARE_WRITE8_MEMBER(question_bank_w);
	DECLARE_WRITE8_MEMBER(funworld_videoram_w);
	DECLARE_WRITE8_MEMBER(funworld_colorram_w);
	DECLARE_WRITE8_MEMBER(funworld_lamp_a_w);
	DECLARE_WRITE8_MEMBER(funworld_lamp_b_w);
	DECLARE_WRITE8_MEMBER(pia1_ca2_w);
	DECLARE_READ8_MEMBER(funquiz_ay8910_a_r);
	DECLARE_READ8_MEMBER(funquiz_ay8910_b_r);
	DECLARE_DRIVER_INIT(magicd2b);
	DECLARE_DRIVER_INIT(saloon);
	DECLARE_DRIVER_INIT(royalcdc);
	DECLARE_DRIVER_INIT(multiwin);
	DECLARE_DRIVER_INIT(soccernw);
	DECLARE_DRIVER_INIT(tabblue);
	DECLARE_DRIVER_INIT(magicd2a);
};


/*----------- defined in video/funworld.c -----------*/

PALETTE_INIT( funworld );
VIDEO_START( funworld );
VIDEO_START( magicrd2 );
SCREEN_UPDATE_IND16( funworld );
