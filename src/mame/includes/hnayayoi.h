/*************************************************************************

    Hana Yayoi & other Dynax games (using 1st version of their blitter)

*************************************************************************/

class hnayayoi_state : public driver_device
{
public:
	hnayayoi_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* video-related */
	UINT8      *m_pixmap[8];
	int        m_palbank;
	int        m_total_pixmaps;
	UINT8      m_blit_layer;
	UINT16     m_blit_dest;
	UINT32     m_blit_src;

	/* misc */
	int        m_keyb;
	DECLARE_READ8_MEMBER(keyboard_0_r);
	DECLARE_READ8_MEMBER(keyboard_1_r);
	DECLARE_WRITE8_MEMBER(keyboard_w);
	DECLARE_WRITE8_MEMBER(dynax_blitter_rev1_param_w);
	DECLARE_WRITE8_MEMBER(dynax_blitter_rev1_start_w);
	DECLARE_WRITE8_MEMBER(dynax_blitter_rev1_clear_w);
	DECLARE_WRITE8_MEMBER(hnayayoi_palbank_w);
};


/*----------- defined in video/hnayayoi.c -----------*/

VIDEO_START( hnayayoi );
VIDEO_START( untoucha );
SCREEN_UPDATE_IND16( hnayayoi );

