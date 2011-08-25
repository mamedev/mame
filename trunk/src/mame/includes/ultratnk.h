/*************************************************************************

    Atari Ultra Tank hardware

*************************************************************************/


class ultratnk_state : public driver_device
{
public:
	ultratnk_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_videoram;
	int m_da_latch;
	int m_collision[4];
	tilemap_t* m_playfield;
	bitmap_t* m_helper;
};


/*----------- defined in video/ultratnk.c -----------*/

PALETTE_INIT( ultratnk );
VIDEO_START( ultratnk );
SCREEN_UPDATE( ultratnk );
SCREEN_EOF( ultratnk );

WRITE8_HANDLER( ultratnk_video_ram_w );
