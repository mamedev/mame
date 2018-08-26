// license:GPL-2.0+
// copyright-holders:David Graves, Jarek Burczynski
/***************************************************************************
  Functions to emulate video hardware on these Taito games:

  - operation wolf

***************************************************************************/

#include "emu.h"
#include "includes/opwolf.h"
#include "screen.h"


WRITE16_MEMBER(opwolf_state::opwolf_spritectrl_w)
{
	// popmessage("opwolf_spritectrl_w ctrl = %4x", data);
	if (offset == 0)
	{
		/* bit 0 -> MOTOR1 transistor */
		/* bit 1 -> MOTOR2 transistor */
		/* bit 2 -> Reset c-chip and coin custom PC050CM (active low) */
		/* bit 3 -> Not connected */
		/* bit 4 -> LATCH - used to signal light gun position can be latched to inputs on v-blank */
		/* bits 5-7 are the sprite palette bank */

		m_pc090oj->set_sprite_ctrl((data & 0xe0) >> 5);

		/* If data & 3, the Piston Motor is activated via M-1/M-2 connector */
		if (data & 3)
		{
			output().set_value("Player1_Recoil_Piston", 1);
		}
		else
		{
			output().set_value("Player1_Recoil_Piston", 0);
		}
	}
}

/***************************************************************************/

uint32_t opwolf_state::screen_update_opwolf(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int layer[2];

	m_pc080sn->tilemap_update();

	layer[0] = 0;
	layer[1] = 1;

	screen.priority().fill(0, cliprect);

	// Sprite/tilemap priority is hardwired by the PAL16L8 at location 19

	m_pc080sn->tilemap_draw(screen, bitmap, cliprect, layer[0], TILEMAP_DRAW_OPAQUE, 1);
	m_pc080sn->tilemap_draw(screen, bitmap, cliprect, layer[1], 0, 2);

	m_pc090oj->draw_sprites(bitmap, cliprect, screen.priority(), 1);

//  if (ioport("P1X")->read())
//  popmessage("%d %d", machine(), "P1X"), ioport("P1Y")->read());

	return 0;
}
