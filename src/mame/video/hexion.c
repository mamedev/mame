#include "emu.h"
#include "includes/hexion.h"




/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

INLINE void get_tile_info(running_machine &machine,tile_data *tileinfo,int tile_index,UINT8 *ram)
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

	tilemap_set_transparent_pen(state->m_bg_tilemap[0],0);
	tilemap_set_scrollx(state->m_bg_tilemap[1],0,-4);
	tilemap_set_scrolly(state->m_bg_tilemap[1],0,4);

	state->m_vram[0] = machine.region("maincpu")->base() + 0x30000;
	state->m_vram[1] = state->m_vram[0] + 0x2000;
	state->m_unkram = state->m_vram[1] + 0x2000;
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( hexion_bankswitch_w )
{
	hexion_state *state = space->machine().driver_data<hexion_state>();
	UINT8 *rom = space->machine().region("maincpu")->base() + 0x10000;

	/* bits 0-3 select ROM bank */
	memory_set_bankptr(space->machine(), "bank1",rom + 0x2000 * (data & 0x0f));

	/* does bit 6 trigger the 052591? */
	if (data & 0x40)
	{
		int bank = state->m_unkram[0]&1;
		memset(state->m_vram[bank],state->m_unkram[1],0x2000);
		tilemap_mark_all_tiles_dirty(state->m_bg_tilemap[bank]);
	}
	/* bit 7 = PMC-BK */
	state->m_pmcbank = (data & 0x80) >> 7;

	/* other bits unknown */
if (data & 0x30)
	popmessage("bankswitch %02x",data&0xf0);

//logerror("%04x: bankswitch_w %02x\n",cpu_get_pc(&space->device()),data);
}

READ8_HANDLER( hexion_bankedram_r )
{
	hexion_state *state = space->machine().driver_data<hexion_state>();
	if (state->m_gfxrom_select && offset < 0x1000)
	{
		return space->machine().region("gfx1")->base()[((state->m_gfxrom_select & 0x7f) << 12) + offset];
	}
	else if (state->m_bankctrl == 0)
	{
		return state->m_vram[state->m_rambank][offset];
	}
	else if (state->m_bankctrl == 2 && offset < 0x800)
	{
		return state->m_unkram[offset];
	}
	else
	{
//logerror("%04x: bankedram_r offset %04x, bankctrl = %02x\n",cpu_get_pc(&space->device()),offset,state->m_bankctrl);
		return 0;
	}
}

WRITE8_HANDLER( hexion_bankedram_w )
{
	hexion_state *state = space->machine().driver_data<hexion_state>();
	if (state->m_bankctrl == 3 && offset == 0 && (data & 0xfe) == 0)
	{
//logerror("%04x: bankedram_w offset %04x, data %02x, bankctrl = %02x\n",cpu_get_pc(&space->device()),offset,data,state->m_bankctrl);
		state->m_rambank = data & 1;
	}
	else if (state->m_bankctrl == 0)
	{
		if (state->m_pmcbank)
		{
//logerror("%04x: bankedram_w offset %04x, data %02x, bankctrl = %02x\n",cpu_get_pc(&space->device()),offset,data,state->m_bankctrl);
			state->m_vram[state->m_rambank][offset] = data;
			tilemap_mark_tile_dirty(state->m_bg_tilemap[state->m_rambank],offset/4);
		}
		else
			logerror("%04x pmc internal ram %04x = %02x\n",cpu_get_pc(&space->device()),offset,data);
	}
	else if (state->m_bankctrl == 2 && offset < 0x800)
	{
		if (state->m_pmcbank)
		{
//logerror("%04x: unkram_w offset %04x, data %02x, bankctrl = %02x\n",cpu_get_pc(&space->device()),offset,data,state->m_bankctrl);
			state->m_unkram[offset] = data;
		}
		else
			logerror("%04x pmc internal ram %04x = %02x\n",cpu_get_pc(&space->device()),offset,data);
	}
	else
logerror("%04x: bankedram_w offset %04x, data %02x, bankctrl = %02x\n",cpu_get_pc(&space->device()),offset,data,state->m_bankctrl);
}

WRITE8_HANDLER( hexion_bankctrl_w )
{
	hexion_state *state = space->machine().driver_data<hexion_state>();
//logerror("%04x: bankctrl_w %02x\n",cpu_get_pc(&space->device()),data);
	state->m_bankctrl = data;
}

WRITE8_HANDLER( hexion_gfxrom_select_w )
{
	hexion_state *state = space->machine().driver_data<hexion_state>();
//logerror("%04x: gfxrom_select_w %02x\n",cpu_get_pc(&space->device()),data);
	state->m_gfxrom_select = data;
}



/***************************************************************************

  Display refresh

***************************************************************************/

SCREEN_UPDATE( hexion )
{
	hexion_state *state = screen->machine().driver_data<hexion_state>();
	tilemap_draw(bitmap,cliprect,state->m_bg_tilemap[1],0,0);
	tilemap_draw(bitmap,cliprect,state->m_bg_tilemap[0],0,0);
	return 0;
}
