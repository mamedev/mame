/*************************************************************************

    Bogey Manor

*************************************************************************/

class bogeyman_state : public driver_device
{
public:
	bogeyman_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *    m_videoram;
	UINT8 *    m_videoram2;
	UINT8 *    m_colorram;
	UINT8 *    m_colorram2;
	UINT8 *    m_spriteram;
//  UINT8 *    m_paletteram;  // currently this uses generic palette handling
	size_t     m_spriteram_size;

	/* video-related */
	tilemap_t    *m_bg_tilemap;
	tilemap_t    *m_fg_tilemap;

	/* misc */
	int        m_psg_latch;
	int        m_last_write;
	int        m_colbank;
	DECLARE_WRITE8_MEMBER(bogeyman_8910_latch_w);
	DECLARE_WRITE8_MEMBER(bogeyman_8910_control_w);
	DECLARE_WRITE8_MEMBER(bogeyman_videoram_w);
	DECLARE_WRITE8_MEMBER(bogeyman_colorram_w);
	DECLARE_WRITE8_MEMBER(bogeyman_videoram2_w);
	DECLARE_WRITE8_MEMBER(bogeyman_colorram2_w);
	DECLARE_WRITE8_MEMBER(bogeyman_paletteram_w);
};


/*----------- defined in video/bogeyman.c -----------*/


PALETTE_INIT( bogeyman );
VIDEO_START( bogeyman );
SCREEN_UPDATE_IND16( bogeyman );
