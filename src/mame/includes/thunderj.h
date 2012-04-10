/*************************************************************************

    Atari ThunderJaws hardware

*************************************************************************/

#include "machine/atarigen.h"

class thunderj_state : public atarigen_state
{
public:
	thunderj_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag) { }

	UINT8			m_alpha_tile_bank;
	DECLARE_READ16_MEMBER(special_port2_r);
	DECLARE_WRITE16_MEMBER(latch_w);
	DECLARE_READ16_MEMBER(thunderj_atarivc_r);
	DECLARE_WRITE16_MEMBER(thunderj_atarivc_w);
};


/*----------- defined in video/thunderj.c -----------*/

VIDEO_START( thunderj );
SCREEN_UPDATE_IND16( thunderj );
