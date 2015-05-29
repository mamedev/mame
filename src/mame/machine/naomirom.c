// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "naomirom.h"

const device_type NAOMI_ROM_BOARD = &device_creator<naomi_rom_board>;

naomi_rom_board::naomi_rom_board(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: naomi_board(mconfig, NAOMI_ROM_BOARD, "Sega NAOMI ROM Board", tag, owner, clock, "naomi_rom_board", __FILE__)
{
}

void naomi_rom_board::device_start()
{
	naomi_board::device_start();
	save_item(NAME(rom_cur_address));
}

void naomi_rom_board::device_reset()
{
	naomi_board::device_reset();
	rom_cur_address = 0;
}

void naomi_rom_board::board_setup_address(UINT32 address, bool is_dma)
{
	rom_cur_address = address & 0x1fffffff;
}

void naomi_rom_board::board_get_buffer(UINT8 *&base, UINT32 &limit)
{
	base = m_region->base() + rom_cur_address;
	limit = m_region->bytes() - rom_cur_address;
}

void naomi_rom_board::board_advance(UINT32 size)
{
	rom_cur_address += size;
}
