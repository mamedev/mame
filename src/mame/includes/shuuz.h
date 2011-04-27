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
};


/*----------- defined in video/shuuz.c -----------*/

VIDEO_START( shuuz );
SCREEN_UPDATE( shuuz );
