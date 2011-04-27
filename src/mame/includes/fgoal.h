

class fgoal_state : public driver_device
{
public:
	fgoal_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *    m_video_ram;

	/* video-related */
	bitmap_t   *m_bgbitmap;
	bitmap_t   *m_fgbitmap;
	UINT8      m_xpos;
	UINT8      m_ypos;
	int        m_current_color;

	/* misc */
	int        m_fgoal_player;
	UINT8      m_row;
	UINT8      m_col;
	int        m_prev_coin;

	/* devices */
	device_t *m_maincpu;
	device_t *m_mb14241;
};


/*----------- defined in video/fgoal.c -----------*/

VIDEO_START( fgoal );
SCREEN_UPDATE( fgoal );

WRITE8_HANDLER( fgoal_color_w );
WRITE8_HANDLER( fgoal_xpos_w );
WRITE8_HANDLER( fgoal_ypos_w );

