// license:BSD-3-Clause
// copyright-holders:Karl Stenerud

#include "emu.h"
#include "mcf5206e.h"
#include "m68kdasm.h"

DEFINE_DEVICE_TYPE(MCF5206E,    mcf5206e_device,    "mcf5206e",     "Freescale MCF5206E")

std::unique_ptr<util::disasm_interface> mcf5206e_device::create_disassembler()
{
	return std::make_unique<m68k_disassembler>(m68k_disassembler::TYPE_COLDFIRE);
}

mcf5206e_device::mcf5206e_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: m68000_musashi_device(mconfig, tag, owner, clock, MCF5206E, 32,32)
{
}

void mcf5206e_device::device_start()
{
	m68000_musashi_device::device_start();
	init_cpu_coldfire();
}
