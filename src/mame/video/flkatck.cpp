// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
/***************************************************************************

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/flkatck.h"

/***************************************************************************

  Callbacks for the K007121

***************************************************************************/

TILE_GET_INFO_MEMBER(flkatck_state::get_tile_info_A)
{
	UINT8 ctrl_0 = m_k007121->ctrlram_r(generic_space(), 0);
	UINT8 ctrl_2 = m_k007121->ctrlram_r(generic_space(), 2);
	UINT8 ctrl_3 = m_k007121->ctrlram_r(generic_space(), 3);
	UINT8 ctrl_4 = m_k007121->ctrlram_r(generic_space(), 4);
	UINT8 ctrl_5 = m_k007121->ctrlram_r(generic_space(), 5);
	int attr = m_k007121_ram[tile_index];
	int code = m_k007121_ram[tile_index + 0x400];
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
		bank = 0;   /*  this allows the game to print text
                    in all banks selected by the k007121 */

	SET_TILE_INFO_MEMBER(0,
			code + 256*bank,
			(attr & 0x0f) + 16,
			(attr & 0x20) ? TILE_FLIPY : 0);
}

TILE_GET_INFO_MEMBER(flkatck_state::get_tile_info_B)
{
	int attr = m_k007121_ram[tile_index + 0x800];
	int code = m_k007121_ram[tile_index + 0xc00];

	SET_TILE_INFO_MEMBER(0,
			code,
			(attr & 0x0f) + 16,
			0);
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void flkatck_state::video_start()
{
	m_k007121_tilemap[0] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(flkatck_state::get_tile_info_A),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_k007121_tilemap[1] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(flkatck_state::get_tile_info_B),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}


/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_MEMBER(flkatck_state::flkatck_k007121_w)
{
	m_k007121_ram[offset] = data;
	if (offset < 0x1000)    /* tiles */
	{
		if (offset & 0x800) /* score */
			m_k007121_tilemap[1]->mark_tile_dirty(offset & 0x3ff);
		else
			m_k007121_tilemap[0]->mark_tile_dirty(offset & 0x3ff);
	}
}

WRITE8_MEMBER(flkatck_state::flkatck_k007121_regs_w)
{
	switch (offset)
	{
		case 0x04:  /* ROM bank select */
			if (data != m_k007121->ctrlram_r(space, 4))
				machine().tilemap().mark_all_dirty();
			break;

		case 0x07:  /* flip screen + IRQ control */
			m_flipscreen = data & 0x08;
			machine().tilemap().set_flip_all(m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
			m_irq_enabled = data & 0x02;
			break;
	}

	m_k007121->ctrl_w(space, offset, data);
}


/***************************************************************************

    Display Refresh

***************************************************************************/

/***************************************************************************

    Flack Attack sprites. Each sprite has 16 bytes!:


***************************************************************************/

UINT32 flkatck_state::screen_update_flkatck(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	rectangle clip[2];
	const rectangle &visarea = screen.visible_area();

	address_space &space = machine().driver_data()->generic_space();
	if (m_flipscreen)
	{
		clip[0] = visarea;
		clip[0].max_x -= 40;

		clip[1] = visarea;
		clip[1].min_x = clip[1].max_x - 40;

		m_k007121_tilemap[0]->set_scrollx(0, m_k007121->ctrlram_r(space, 0) - 56 );
		m_k007121_tilemap[0]->set_scrolly(0, m_k007121->ctrlram_r(space, 2));
		m_k007121_tilemap[1]->set_scrollx(0, -16);
	}
	else
	{
		clip[0] = visarea;
		clip[0].min_x += 40;

		clip[1] = visarea;
		clip[1].max_x = 39;
		clip[1].min_x = 0;

		m_k007121_tilemap[0]->set_scrollx(0, m_k007121->ctrlram_r(space, 0) - 40 );
		m_k007121_tilemap[0]->set_scrolly(0, m_k007121->ctrlram_r(space, 2));
		m_k007121_tilemap[1]->set_scrollx(0, 0);
	}

	/* compute clipping */
	clip[0] &= cliprect;
	clip[1] &= cliprect;

	/* draw the graphics */
	m_k007121_tilemap[0]->draw(screen, bitmap, clip[0], 0, 0);
	m_k007121->sprites_draw(bitmap, cliprect, m_gfxdecode->gfx(0), nullptr, &m_k007121_ram[0x1000], 0, 40, 0, screen.priority(), (UINT32)-1);
	m_k007121_tilemap[1]->draw(screen, bitmap, clip[1], 0, 0);
	return 0;
}
