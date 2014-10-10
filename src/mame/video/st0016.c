/************************************
      Seta custom ST-0016 chip
    driver by Tomasz Slanina
************************************/

#include "emu.h"
#include "includes/st0016.h"

UINT32 st0016_state::screen_update_st0016(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return m_maincpu->update(screen,bitmap,cliprect);
}

VIDEO_START_MEMBER(st0016_state, st0016)
{
}

