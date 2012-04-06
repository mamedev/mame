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
	DECLARE_WRITE8_MEMBER(homerun_videoram_w);
	DECLARE_WRITE8_MEMBER(homerun_color_w);
};


/*----------- defined in video/homerun.c -----------*/

WRITE8_DEVICE_HANDLER( homerun_banking_w );

VIDEO_START(homerun);
SCREEN_UPDATE_IND16(homerun);
