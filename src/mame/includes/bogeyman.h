/*************************************************************************

    Bogey Manor

*************************************************************************/

class bogeyman_state : public driver_device
{
public:
	bogeyman_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"),
		m_videoram2(*this, "videoram2"),
		m_colorram(*this, "colorram"),
		m_colorram2(*this, "colorram2"),
		m_spriteram(*this, "spriteram"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_videoram2;
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_colorram2;
	required_shared_ptr<UINT8> m_spriteram;
//  UINT8 *    m_paletteram;  // currently this uses generic palette handling

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
	DECLARE_WRITE8_MEMBER(bogeyman_colbank_w);
};


/*----------- defined in video/bogeyman.c -----------*/


PALETTE_INIT( bogeyman );
VIDEO_START( bogeyman );
SCREEN_UPDATE_IND16( bogeyman );
