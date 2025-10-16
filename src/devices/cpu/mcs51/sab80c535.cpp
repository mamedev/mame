// license:BSD-3-Clause
// copyright-holders:Steve Ellenoff, Manuel Abadia, Couriersud

#include "emu.h"
#include "sab80c535.h"
#include "mcs51dasm.h"

DEFINE_DEVICE_TYPE(SAB80C535, sab80c535_device, "sab80c535", "Siemens SAB80C535")

sab80c535_device::sab80c535_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: i80c52_device(mconfig, SAB80C535, tag, owner, clock, 0)
{
}

std::unique_ptr<util::disasm_interface> sab80c535_device::create_disassembler()
{
	return std::make_unique<sab80c515_disassembler>();
}
