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

SCREEN_UPDATE_IND16( opwolf )
{
	opwolf_state *state = screen.machine().driver_data<opwolf_state>();
	int layer[2];

	pc080sn_tilemap_update(state->m_pc080sn);

	layer[0] = 0;
	layer[1] = 1;

	screen.machine().priority_bitmap.fill(0, cliprect);

	pc080sn_tilemap_draw(state->m_pc080sn, bitmap, cliprect, layer[0], TILEMAP_DRAW_OPAQUE, 1);
	pc080sn_tilemap_draw(state->m_pc080sn, bitmap, cliprect, layer[1], 0, 2);

	pc090oj_draw_sprites(state->m_pc090oj, bitmap, cliprect, 1);

//  if (input_port_read(machine, "P1X"))
//  popmessage("%d %d", input_port_read(machine, "P1X"), input_port_read(machine, "P1Y"));

	return 0;
}

