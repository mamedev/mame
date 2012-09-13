/***************************************************************************

  video/nes.c

  Routines to control the unique NES video hardware/PPU.

***************************************************************************/

#include "emu.h"
#include "video/ppu2c0x.h"
#include "includes/nes.h"
#include "machine/nes_mmc.h"

static void nes_vh_reset( running_machine &machine )
{
	nes_state *state = machine.driver_data<nes_state>();
	state->m_ppu->set_vidaccess_callback(nes_ppu_vidaccess);
}

void nes_state::video_start()
{

	m_last_frame_flip =  0;

	machine().add_notifier(MACHINE_NOTIFY_RESET, machine_notify_delegate(FUNC(nes_vh_reset),&machine()));
}

void nes_state::palette_init()
{
	m_ppu->init_palette(machine(), 0);
}


/***************************************************************************

  Display refresh

***************************************************************************/

SCREEN_UPDATE_IND16( nes )
{
	nes_state *state = screen.machine().driver_data<nes_state>();

	/* render the ppu */
	state->m_ppu->render(bitmap, 0, 0, 0, 0);

	/* if this is a disk system game, check for the flip-disk key */
	if (state->m_disk_expansion && state->m_pcb_id == NO_BOARD)
	{
		// latch this input so it doesn't go at warp speed
		if ((screen.machine().root_device().ioport("FLIPDISK")->read() & 0x01) && (!state->m_last_frame_flip))
		{
			state->m_last_frame_flip = 1;
			state->m_fds_current_side++;
			if (state->m_fds_current_side > state->m_fds_sides)
				state->m_fds_current_side = 0;

			if (state->m_fds_current_side == 0)
				popmessage("No disk inserted.");
			else
				popmessage("Disk set to side %d", state->m_fds_current_side);
		}

		if (!(screen.machine().root_device().ioport("FLIPDISK")->read() & 0x01))
			state->m_last_frame_flip = 0;
	}
	return 0;
}
