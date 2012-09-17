#include "emu.h"

#include "includes/comquest.h"

void comquest_state::video_start()
{
}

UINT32 comquest_state::screen_update_comquest(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x, y, j;

	for (y=0; y<128;y++) {
		for (x=0, j=0; j<8;j++,x+=8*4) {
#if 0
			drawgfx_opaque(bitmap, 0, machine.gfx[0], state->m_data[y][j],0,
					0,0,x,y);
#endif
		}
	}
	return 0;
}
