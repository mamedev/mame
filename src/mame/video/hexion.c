#include "emu.h"
#include "includes/hexion.h"




/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

INLINE void get_tile_info(running_machine &machine,tile_data &tileinfo,int tile_index,UINT8 *ram)
{
	tile_index *= 4;
	SET_TILE_INFO(
			0,
			ram[tile_index] + ((ram[tile_index+1] & 0x3f) << 8),
			ram[tile_index+2] & 0x0f,
			0);
}

static TILE_GET_INFO( get_tile_info0 )
{
	hexion_state *state = machine.driver_data<hexion_state>();
	get_tile_info(machine,tileinfo,tile_index,state->m_vram[0]);
}

static TILE_GET_INFO( get_tile_info1 )
{
	hexion_state *state = machine.driver_data<hexion_state>();
	get_tile_info(machine,tileinfo,tile_index,state->m_vram[1]);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( hexion )
{
	hexion_state *state = machine.driver_data<hexion_state>();
	state->m_bg_tilemap[0] = tilemap_create(machine, get_tile_info0,tilemap_scan_rows,8,8,64,32);
	state->m_bg_tilemap[1] = tilemap_create(machine, get_tile_info1,tilemap_scan_rows,     8,8,64,32);

	state->m_bg_tilemap[0]->set_transparent_pen(0);
	state->m_bg_tilemap[1]->set_scrollx(0,-4);
	state->m_bg_tilemap[1]->set_scrolly(0,4);

	state->m_vram[0] = machine.region("maincpu")->base() + 0x30000;
	state->m_vram[1] = state->m_vram[0] + 0x2000;
	state->m_unkram = state->m_vram[1] + 0x2000;
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_MEMBER(hexion_state::hexion_bankswitch_w)
{
	UINT8 *rom = machine().region("maincpu")->base() + 0x10000;

	/* bits 0-3 select ROM bank */
	memory_set_bankptr(machine(), "bank1",rom + 0x2000 * (data & 0x0f));

	/* does bit 6 trigger the 052591? */
	if (data & 0x40)
	{
		int bank = m_unkram[0]&1;
		memset(m_vram[bank],m_unkram[1],0x2000);
		m_bg_tilemap[bank]->mark_all_dirty();
	}
	/* bit 7 = PMC-BK */
	m_pmcbank = (data & 0x80) >> 7;

	/* other bits unknown */
if (data & 0x30)
	popmessage("bankswitch %02x",data&0xf0);

//logerror("%04x: bankswitch_w %02x\n",cpu_get_pc(&space.device()),data);
}

READ8_MEMBER(hexion_state::hexion_bankedram_r)
{
	if (m_gfxrom_select && offset < 0x1000)
	{
		return machine().region("gfx1")->base()[((m_gfxrom_select & 0x7f) << 12) + offset];
	}
	else if (m_bankctrl == 0)
	{
		return m_vram[m_rambank][offset];
	}
	else if (m_bankctrl == 2 && offset < 0x800)
	{
		return m_unkram[offset];
	}
	else
	{
//logerror("%04x: bankedram_r offset %04x, bankctrl = %02x\n",cpu_get_pc(&space.device()),offset,m_bankctrl);
		return 0;
	}
}

WRITE8_MEMBER(hexion_state::hexion_bankedram_w)
{
	if (m_bankctrl == 3 && offset == 0 && (data & 0xfe) == 0)
	{
//logerror("%04x: bankedram_w offset %04x, data %02x, bankctrl = %02x\n",cpu_get_pc(&space.device()),offset,data,m_bankctrl);
		m_rambank = data & 1;
	}
	else if (m_bankctrl == 0)
	{
		if (m_pmcbank)
		{
//logerror("%04x: bankedram_w offset %04x, data %02x, bankctrl = %02x\n",cpu_get_pc(&space.device()),offset,data,m_bankctrl);
			m_vram[m_rambank][offset] = data;
			m_bg_tilemap[m_rambank]->mark_tile_dirty(offset/4);
		}
		else
			logerror("%04x pmc internal ram %04x = %02x\n",cpu_get_pc(&space.device()),offset,data);
	}
	else if (m_bankctrl == 2 && offset < 0x800)
	{
		if (m_pmcbank)
		{
//logerror("%04x: unkram_w offset %04x, data %02x, bankctrl = %02x\n",cpu_get_pc(&space.device()),offset,data,m_bankctrl);
			m_unkram[offset] = data;
		}
		else
			logerror("%04x pmc internal ram %04x = %02x\n",cpu_get_pc(&space.device()),offset,data);
	}
	else
logerror("%04x: bankedram_w offset %04x, data %02x, bankctrl = %02x\n",cpu_get_pc(&space.device()),offset,data,m_bankctrl);
}

WRITE8_MEMBER(hexion_state::hexion_bankctrl_w)
{
//logerror("%04x: bankctrl_w %02x\n",cpu_get_pc(&space.device()),data);
	m_bankctrl = data;
}

WRITE8_MEMBER(hexion_state::hexion_gfxrom_select_w)
{
//logerror("%04x: gfxrom_select_w %02x\n",cpu_get_pc(&space.device()),data);
	m_gfxrom_select = data;
}



/***************************************************************************

  Display refresh

***************************************************************************/

SCREEN_UPDATE_IND16( hexion )
{
	hexion_state *state = screen.machine().driver_data<hexion_state>();
	state->m_bg_tilemap[1]->draw(bitmap, cliprect, 0,0);
	state->m_bg_tilemap[0]->draw(bitmap, cliprect, 0,0);
	return 0;
}
