// license:BSD-3-Clause
// copyright-holders:hap
/*

  Suwa Seikosha (now Seiko Epson) SMC1102, SMC1112

SMC1102 is a CMOS MCU based on TMS1100, keeping the same ALU and opcode mnemonics.
They added a timer, interrupts, and a built-in LCD controller.

In the USA, it was marketed by S-MOS Systems, an affiliate of the Seiko Group.

SMC1112 die notes (SMC1102 is assumed to be the same):
- 128x4 RAM array at top-left
- 256*64 8-bit ROM array at the bottom
- 30-term MPLA with 14 microinstructions, and 16 fixed opcodes next to it
  (assumed neither of them is supposed to be customized)
- 32x4 LCD RAM at the left
- no output PLA

TODO:
- x

*/

#include "emu.h"
#include "smc1102.h"
#include "tms1k_dasm.h"


// device definitions
DEFINE_DEVICE_TYPE(SMC1102, smc1102_cpu_device, "smc1102", "Suwa Seikosha SMC1102") // 60-pin QFP or 42-pin DIP
DEFINE_DEVICE_TYPE(SMC1112, smc1112_cpu_device, "smc1112", "Suwa Seikosha SMC1112") // low power version


smc1102_cpu_device::smc1102_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 o_pins, u8 r_pins, u8 pc_bits, u8 byte_bits, u8 x_bits, u8 stack_levels, int rom_width, address_map_constructor rom_map, int ram_width, address_map_constructor ram_map) :
	tms1100_cpu_device(mconfig, type, tag, owner, clock, o_pins, r_pins, pc_bits, byte_bits, x_bits, stack_levels, rom_width, rom_map, ram_width, ram_map)
{ }

smc1102_cpu_device::smc1102_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	smc1102_cpu_device(mconfig, SMC1102, tag, owner, clock, 0 /* o pins */, 8 /* r pins */, 6 /* pc bits */, 8 /* byte width */, 3 /* x width */, 4 /* stack levels */, 11 /* rom width */, address_map_constructor(FUNC(smc1102_cpu_device::rom_11bit), this), 7 /* ram width */, address_map_constructor(FUNC(smc1102_cpu_device::ram_7bit), this))
{ }

smc1112_cpu_device::smc1112_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	smc1102_cpu_device(mconfig, SMC1112, tag, owner, clock, 0, 8, 6, 8, 3, 4, 11, address_map_constructor(FUNC(smc1112_cpu_device::rom_11bit), this), 7, address_map_constructor(FUNC(smc1112_cpu_device::ram_7bit), this))
{ }


// disasm
std::unique_ptr<util::disasm_interface> smc1102_cpu_device::create_disassembler()
{
	return std::make_unique<smc1102_disassembler>();
}


// device_start/reset
void smc1102_cpu_device::device_start()
{
	tms1100_cpu_device::device_start();
}

u32 smc1102_cpu_device::decode_micro(offs_t offset)
{
	return 0;
}

void smc1102_cpu_device::device_reset()
{
	tms1100_cpu_device::device_reset();
}
