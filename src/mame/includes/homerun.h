/*************************************************************************

    Moero Pro Yakyuu Homerun & Dynamic Shooting

*************************************************************************/

class homerun_state : public driver_device
{
public:
	homerun_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_spriteram;

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
	DECLARE_CUSTOM_INPUT_MEMBER(homerun_40_r);
	DECLARE_WRITE8_MEMBER(pa_w);
	DECLARE_WRITE8_MEMBER(pb_w);
	DECLARE_WRITE8_MEMBER(pc_w);
};


/*----------- defined in video/homerun.c -----------*/

WRITE8_DEVICE_HANDLER( homerun_banking_w );

VIDEO_START(homerun);
SCREEN_UPDATE_IND16(homerun);
