class pingpong_state : public driver_device
{
public:
	pingpong_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu")
		{ }

	int m_intenable;
	int m_question_addr_high;
	UINT8 *m_videoram;
	UINT8 *m_colorram;
	tilemap_t *m_bg_tilemap;
	UINT8 *m_spriteram;
	size_t m_spriteram_size;

	required_device<cpu_device> m_maincpu;
};


/*----------- defined in video/pingpong.c -----------*/

WRITE8_HANDLER( pingpong_videoram_w );
WRITE8_HANDLER( pingpong_colorram_w );

PALETTE_INIT( pingpong );
VIDEO_START( pingpong );
SCREEN_UPDATE( pingpong );
