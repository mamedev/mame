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
	ppu2c0x_set_vidaccess_callback(machine.device("ppu"), nes_ppu_vidaccess);

	if (state->m_four_screen_vram)
		set_nt_mirroring(machine, PPU_MIRROR_4SCREEN);
	else
	{
		switch (state->m_hard_mirroring)
		{
			case PPU_MIRROR_HORZ:
			case PPU_MIRROR_VERT:
			case PPU_MIRROR_HIGH:
			case PPU_MIRROR_LOW:
				set_nt_mirroring(machine, state->m_hard_mirroring);
				break;
			default:
				set_nt_mirroring(machine, PPU_MIRROR_NONE);
				break;
		}
	}
}

VIDEO_START( nes )
{
	nes_state *state = machine.driver_data<nes_state>();

	state->m_last_frame_flip =  0;

	machine.add_notifier(MACHINE_NOTIFY_RESET, machine_notify_delegate(FUNC(nes_vh_reset),&machine));
}

PALETTE_INIT( nes )
{
	ppu2c0x_init_palette(machine, 0);
}


/***************************************************************************

  Display refresh

***************************************************************************/

SCREEN_UPDATE( nes )
{
	nes_state *state = screen->machine().driver_data<nes_state>();

	/* render the ppu */
	ppu2c0x_render(state->m_ppu, bitmap, 0, 0, 0, 0);

	/* if this is a disk system game, check for the flip-disk key */
	if (state->m_disk_expansion && state->m_pcb_id == NO_BOARD)
	{
		// latch this input so it doesn't go at warp speed
		if ((input_port_read(screen->machine(), "FLIPDISK") & 0x01) && (!state->m_last_frame_flip))
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

		if (!(input_port_read(screen->machine(), "FLIPDISK") & 0x01))
			state->m_last_frame_flip = 0;
	}
	return 0;
}
