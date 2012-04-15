/*************************************************************************

    Atari Cops'n Robbers hardware

*************************************************************************/

#include "sound/discrete.h"


class copsnrob_state : public driver_device
{
public:
	copsnrob_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_trucky(*this, "trucky"),
		m_truckram(*this, "truckram"),
		m_bulletsram(*this, "bulletsram"),
		m_carimage(*this, "carimage"),
		m_cary(*this, "cary"),
		m_videoram(*this, "videoram"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_trucky;
	required_shared_ptr<UINT8> m_truckram;
	required_shared_ptr<UINT8> m_bulletsram;
	required_shared_ptr<UINT8> m_carimage;
	required_shared_ptr<UINT8> m_cary;
	required_shared_ptr<UINT8> m_videoram;

	/* misc */
	UINT8          m_misc;
	UINT8          m_ic_h3_data;
	DECLARE_READ8_MEMBER(copsnrob_misc_r);
	DECLARE_WRITE8_MEMBER(copsnrob_misc2_w);
	DECLARE_WRITE8_MEMBER(copsnrob_misc_w);
};


/*----------- defined in machine/copsnrob.c -----------*/

READ8_HANDLER( copsnrob_gun_position_r );


/*----------- defined in video/copsnrob.c -----------*/

SCREEN_UPDATE_IND16( copsnrob );


/*----------- defined in audio/copsnrob.c -----------*/

DISCRETE_SOUND_EXTERN( copsnrob );
