// license:BSD-3-Clause
// copyright-holders:Karl Stenerud

#include "emu.h"
#include "fscpu32.h"
#include "m68kdasm.h"

DEFINE_DEVICE_TYPE(FSCPU32,     fscpu32_device,     "fscpu32",      "Freescale CPU32 Core")

std::unique_ptr<util::disasm_interface> fscpu32_device::create_disassembler()
{
	return std::make_unique<m68k_disassembler>(m68k_disassembler::TYPE_68340);
}

fscpu32_device::fscpu32_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: m68000_base_device(mconfig, tag, owner, clock, FSCPU32, 32,32)
{
}

fscpu32_device::fscpu32_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock,
										const device_type type, u32 prg_data_width, u32 prg_address_bits, address_map_constructor internal_map)
	: m68000_base_device(mconfig, tag, owner, clock, type, prg_data_width, prg_address_bits, internal_map)
{
}


void fscpu32_device::device_start()
{
	m68000_base_device::device_start();
	init_cpu_fscpu32();
}
