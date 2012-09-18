/***************************************************************************
  Functions to emulate video hardware on these Taito games:

  - operation wolf

***************************************************************************/

#include "emu.h"
#include "video/taitoic.h"
#include "includes/opwolf.h"


WRITE16_MEMBER(opwolf_state::opwolf_spritectrl_w)
{

	if (offset == 0)
	{
		/* bits 0 and 1 always set */
		/* bits 5-7 are the sprite palette bank */
		/* other bits unknown */

		pc090oj_set_sprite_ctrl(m_pc090oj, (data & 0xe0) >> 5);

		/* If data = 4, the Piston Motor is off, otherwise it's on. */
		if (data == 4)
		{
			output_set_value("Player1_Recoil_Piston", 0);
		}
		else
		{
			output_set_value("Player1_Recoil_Piston", 1);
		}
	}
}

/***************************************************************************/

UINT32 opwolf_state::screen_update_opwolf(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int layer[2];

	pc080sn_tilemap_update(m_pc080sn);

	layer[0] = 0;
	layer[1] = 1;

	machine().priority_bitmap.fill(0, cliprect);

	pc080sn_tilemap_draw(m_pc080sn, bitmap, cliprect, layer[0], TILEMAP_DRAW_OPAQUE, 1);
	pc080sn_tilemap_draw(m_pc080sn, bitmap, cliprect, layer[1], 0, 2);

	pc090oj_draw_sprites(m_pc090oj, bitmap, cliprect, 1);

//  if (ioport("P1X")->read())
//  popmessage("%d %d", machine(), "P1X"), ioport("P1Y")->read());

	return 0;
}

