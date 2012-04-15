/*************************************************************************

    Kusayakyuu

*************************************************************************/

class ksayakyu_state : public driver_device
{
public:
	ksayakyu_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_spriteram;

	/* video-related */
	tilemap_t    *m_tilemap;
	tilemap_t    *m_textmap;
	int        m_video_ctrl;
	int        m_flipscreen;

	/* misc */
	int        m_sound_status;
	DECLARE_WRITE8_MEMBER(bank_select_w);
	DECLARE_WRITE8_MEMBER(latch_w);
	DECLARE_READ8_MEMBER(sound_status_r);
	DECLARE_WRITE8_MEMBER(tomaincpu_w);
	DECLARE_WRITE8_MEMBER(ksayakyu_videoram_w);
	DECLARE_WRITE8_MEMBER(ksayakyu_videoctrl_w);
};


/*----------- defined in video/ksayakyu.c -----------*/

PALETTE_INIT( ksayakyu );
VIDEO_START( ksayakyu );
SCREEN_UPDATE_IND16( ksayakyu );
