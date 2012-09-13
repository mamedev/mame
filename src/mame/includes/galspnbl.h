/*************************************************************************

    Hot Pinball
    Gals Pinball

*************************************************************************/

class galspnbl_state : public driver_device
{
public:
	galspnbl_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_spriteram(*this, "spriteram"),
		m_colorram(*this, "colorram"),
		m_videoram(*this, "videoram"),
		m_bgvideoram(*this, "bgvideoram"),
		m_scroll(*this, "scroll"){ }

	/* memory pointers */
	required_shared_ptr<UINT16> m_spriteram;
	required_shared_ptr<UINT16> m_colorram;
	required_shared_ptr<UINT16> m_videoram;
	required_shared_ptr<UINT16> m_bgvideoram;
	required_shared_ptr<UINT16> m_scroll;
//  UINT16 *    paletteram; // currently this uses generic palette handling

	/* devices */
	cpu_device *m_audiocpu;
	DECLARE_WRITE16_MEMBER(soundcommand_w);
	virtual void machine_start();
	virtual void palette_init();
};


/*----------- defined in video/galspnbl.c -----------*/



SCREEN_UPDATE_IND16( galspnbl );
