/*************************************************************************

    Atari Cops'n Robbers hardware

*************************************************************************/

#include "sound/discrete.h"


class copsnrob_state : public driver_device
{
public:
	copsnrob_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *        m_videoram;
	UINT8 *        m_trucky;
	UINT8 *        m_truckram;
	UINT8 *        m_bulletsram;
	UINT8 *        m_cary;
	UINT8 *        m_carimage;
	size_t         m_videoram_size;

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
