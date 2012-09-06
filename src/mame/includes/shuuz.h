/*************************************************************************

    Atari Shuuz hardware

*************************************************************************/

#include "machine/atarigen.h"

class shuuz_state : public atarigen_state
{
public:
	shuuz_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag) { }

	int m_cur[2];
	DECLARE_READ16_MEMBER(shuuz_atarivc_r);
	DECLARE_WRITE16_MEMBER(shuuz_atarivc_w);
	DECLARE_WRITE16_MEMBER(latch_w);
	DECLARE_READ16_MEMBER(leta_r);
	DECLARE_READ16_MEMBER(special_port0_r);
	TILE_GET_INFO_MEMBER(get_playfield_tile_info);
};


/*----------- defined in video/shuuz.c -----------*/

VIDEO_START( shuuz );
SCREEN_UPDATE_IND16( shuuz );
