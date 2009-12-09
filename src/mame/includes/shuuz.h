/*************************************************************************

    Atari Shuuz hardware

*************************************************************************/

#include "machine/atarigen.h"

typedef struct _shuuz_state shuuz_state;
struct _shuuz_state
{
	atarigen_state	atarigen;
};


/*----------- defined in video/shuuz.c -----------*/

VIDEO_START( shuuz );
VIDEO_UPDATE( shuuz );
