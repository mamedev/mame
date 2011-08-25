/*************************************************************************

    Moero Pro Yakyuu Homerun & Dynamic Shooting

*************************************************************************/

class homerun_state : public driver_device
{
public:
	homerun_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *    m_videoram;
	UINT8 *    m_spriteram;
	size_t     m_spriteram_size;

	/* video-related */
	tilemap_t    *m_tilemap;
	int        m_gfx_ctrl;

	/* misc */
	int        m_xpa;
	int        m_xpb;
	int        m_xpc;
	int        m_gc_up;
	int        m_gc_down;
};


/*----------- defined in video/homerun.c -----------*/

WRITE8_HANDLER( homerun_videoram_w );
WRITE8_HANDLER( homerun_color_w );
WRITE8_DEVICE_HANDLER( homerun_banking_w );

VIDEO_START(homerun);
SCREEN_UPDATE(homerun);
