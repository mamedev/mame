class funworld_state : public driver_device
{
public:
	funworld_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8* m_videoram;
	UINT8* m_colorram;
	tilemap_t *m_bg_tilemap;
	DECLARE_READ8_MEMBER(questions_r);
	DECLARE_WRITE8_MEMBER(question_bank_w);
	DECLARE_WRITE8_MEMBER(funworld_videoram_w);
	DECLARE_WRITE8_MEMBER(funworld_colorram_w);
};


/*----------- defined in video/funworld.c -----------*/

PALETTE_INIT( funworld );
VIDEO_START( funworld );
VIDEO_START( magicrd2 );
SCREEN_UPDATE_IND16( funworld );
