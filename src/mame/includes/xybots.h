/*************************************************************************

    Atari Xybots hardware

*************************************************************************/

#include "machine/atarigen.h"

class xybots_state : public atarigen_state
{
public:
	xybots_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag) { }

	UINT16			m_h256;
	DECLARE_READ16_MEMBER(special_port1_r);
	DECLARE_DRIVER_INIT(xybots);
};


/*----------- defined in video/xybots.c -----------*/

VIDEO_START( xybots );
SCREEN_UPDATE_IND16( xybots );
