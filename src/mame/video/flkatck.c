/***************************************************************************

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "video/konicdev.h"
#include "includes/flkatck.h"

/***************************************************************************

  Callbacks for the K007121

***************************************************************************/

static TILE_GET_INFO( get_tile_info_A )
{
	flkatck_state *state = machine.driver_data<flkatck_state>();
	UINT8 ctrl_0 = k007121_ctrlram_r(state->m_k007121, 0);
	UINT8 ctrl_2 = k007121_ctrlram_r(state->m_k007121, 2);
	UINT8 ctrl_3 = k007121_ctrlram_r(state->m_k007121, 3);
	UINT8 ctrl_4 = k007121_ctrlram_r(state->m_k007121, 4);
	UINT8 ctrl_5 = k007121_ctrlram_r(state->m_k007121, 5);
	int attr = state->m_k007121_ram[tile_index];
	int code = state->m_k007121_ram[tile_index + 0x400];
	int bit0 = (ctrl_5 >> 0) & 0x03;
	int bit1 = (ctrl_5 >> 2) & 0x03;
	int bit2 = (ctrl_5 >> 4) & 0x03;
	int bit3 = (ctrl_5 >> 6) & 0x03;
	int bank = ((attr & 0x80) >> 7) |
			((attr >> (bit0 + 2)) & 0x02) |
			((attr >> (bit1 + 1)) & 0x04) |
			((attr >> (bit2  )) & 0x08) |
			((attr >> (bit3 - 1)) & 0x10) |
			((ctrl_3 & 0x01) << 5);
	int mask = (ctrl_4 & 0xf0) >> 4;

	bank = (bank & ~(mask << 1)) | ((ctrl_4 & mask) << 1);

	if ((attr == 0x0d) && (!ctrl_0) && (!ctrl_2))
		bank = 0;	/*  this allows the game to print text
                    in all banks selected by the k007121 */

	SET_TILE_INFO(
			0,
			code + 256*bank,
			(attr & 0x0f) + 16,
			(attr & 0x20) ? TILE_FLIPY : 0);
}

static TILE_GET_INFO( get_tile_info_B )
{
	flkatck_state *state = machine.driver_data<flkatck_state>();
	int attr = state->m_k007121_ram[tile_index + 0x800];
	int code = state->m_k007121_ram[tile_index + 0xc00];

	SET_TILE_INFO(
			0,
			code,
			(attr & 0x0f) + 16,
			0);
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( flkatck )
{
	flkatck_state *state = machine.driver_data<flkatck_state>();
	state->m_k007121_tilemap[0] = tilemap_create(machine, get_tile_info_A, tilemap_scan_rows, 8, 8, 32, 32);
	state->m_k007121_tilemap[1] = tilemap_create(machine, get_tile_info_B, tilemap_scan_rows, 8, 8, 32, 32);
}


/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_MEMBER(flkatck_state::flkatck_k007121_w)
{

	m_k007121_ram[offset] = data;
	if (offset < 0x1000)	/* tiles */
	{
		if (offset & 0x800)	/* score */
			m_k007121_tilemap[1]->mark_tile_dirty(offset & 0x3ff);
		else
			m_k007121_tilemap[0]->mark_tile_dirty(offset & 0x3ff);
	}
}

WRITE8_MEMBER(flkatck_state::flkatck_k007121_regs_w)
{

	switch (offset)
	{
		case 0x04:	/* ROM bank select */
			if (data != k007121_ctrlram_r(m_k007121, 4))
				machine().tilemap().mark_all_dirty();
			break;

		case 0x07:	/* flip screen + IRQ control */
			m_flipscreen = data & 0x08;
			machine().tilemap().set_flip_all(m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
			m_irq_enabled = data & 0x02;
			break;
	}

	k007121_ctrl_w(m_k007121, offset, data);
}


/***************************************************************************

    Display Refresh

***************************************************************************/

/***************************************************************************

    Flack Attack sprites. Each sprite has 16 bytes!:


***************************************************************************/

SCREEN_UPDATE_IND16( flkatck )
{
	flkatck_state *state = screen.machine().driver_data<flkatck_state>();
	rectangle clip[2];
	const rectangle &visarea = screen.visible_area();

	if (state->m_flipscreen)
	{
		clip[0] = visarea;
		clip[0].max_x -= 40;

		clip[1] = visarea;
		clip[1].min_x = clip[1].max_x - 40;

		state->m_k007121_tilemap[0]->set_scrollx(0, k007121_ctrlram_r(state->m_k007121, 0) - 56 );
		state->m_k007121_tilemap[0]->set_scrolly(0, k007121_ctrlram_r(state->m_k007121, 2));
		state->m_k007121_tilemap[1]->set_scrollx(0, -16);
	}
	else
	{
		clip[0] = visarea;
		clip[0].min_x += 40;

		clip[1] = visarea;
		clip[1].max_x = 39;
		clip[1].min_x = 0;

		state->m_k007121_tilemap[0]->set_scrollx(0, k007121_ctrlram_r(state->m_k007121, 0) - 40 );
		state->m_k007121_tilemap[0]->set_scrolly(0, k007121_ctrlram_r(state->m_k007121, 2));
		state->m_k007121_tilemap[1]->set_scrollx(0, 0);
	}

	/* compute clipping */
	clip[0] &= cliprect;
	clip[1] &= cliprect;

	/* draw the graphics */
	state->m_k007121_tilemap[0]->draw(bitmap, clip[0], 0, 0);
	k007121_sprites_draw(state->m_k007121, bitmap, cliprect, screen.machine().gfx[0], NULL, &state->m_k007121_ram[0x1000], 0, 40, 0, (UINT32)-1);
	state->m_k007121_tilemap[1]->draw(bitmap, clip[1], 0, 0);
	return 0;
}
