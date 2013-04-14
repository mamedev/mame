/***************************************************************************

  video/nes.c

  Routines to control the unique NES video hardware/PPU.

***************************************************************************/

#include "emu.h"
#include "video/ppu2c0x.h"
#include "includes/nes.h"
//#include "includes/nes_mmc.h"

void nes_state::video_reset()
{
	m_ppu->set_vidaccess_callback(ppu2c0x_vidaccess_delegate(FUNC(nes_state::nes_ppu_vidaccess),this));
}

void nes_state::video_start()
{
	m_last_frame_flip =  0;
}

void nes_state::palette_init()
{
	m_ppu->init_palette(machine(), 0);
}


/***************************************************************************

  Display refresh

***************************************************************************/

UINT32 nes_state::screen_update_nes(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* render the ppu */
	m_ppu->render(bitmap, 0, 0, 0, 0);

	/* if this is a disk system game, check for the flip-disk key */
	if (m_disk_expansion && m_pcb_id == NO_BOARD)
	{
		// latch this input so it doesn't go at warp speed
		if ((ioport("FLIPDISK")->read() & 0x01) && (!m_last_frame_flip))
		{
			m_last_frame_flip = 1;
			m_fds_current_side++;
			if (m_fds_current_side > m_fds_sides)
				m_fds_current_side = 0;

			if (m_fds_current_side == 0)
				popmessage("No disk inserted.");
			else
				popmessage("Disk set to side %d", m_fds_current_side);
		}

		if (!(ioport("FLIPDISK")->read() & 0x01))
			m_last_frame_flip = 0;
	}
	return 0;
}
