// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "naomirom.h"

DEFINE_DEVICE_TYPE(NAOMI_ROM_BOARD, naomi_rom_board, "naomi_rom_board", "Sega NAOMI ROM Board")

naomi_rom_board::naomi_rom_board(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: naomi_board(mconfig, NAOMI_ROM_BOARD, tag, owner, clock)
	, m_region(*this, DEVICE_SELF)
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

void naomi_rom_board::board_setup_address(uint32_t address, bool is_dma)
{
	rom_cur_address = address & 0x1fffffff;
}

void naomi_rom_board::board_get_buffer(uint8_t *&base, uint32_t &limit)
{
	base = m_region->base() + rom_cur_address;
	limit = m_region->bytes() - rom_cur_address;
}

void naomi_rom_board::board_advance(uint32_t size)
{
	rom_cur_address += size;
}
