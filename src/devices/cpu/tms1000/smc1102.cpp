// license:BSD-3-Clause
// copyright-holders:hap
/*

  Suwa Seikosha (now Seiko Epson) SMC1102, SMC1112

SMC1102 is a CMOS MCU based on TMS1100, keeping the same ALU and opcode mnemonics.
They added a timer, interrupts, and a built-in LCD controller.

In the USA, it was marketed by S-MOS Systems, an affiliate of Seiko Group.

SMC1112 die notes (SMC1102 is assumed to be the same):
- 128x4 RAM array at top-left
- 256*64 8-bit ROM array at the bottom
- 30-term MPLA with 14 microinstructions, and 16 fixed opcodes next to it
  (assumed neither of them is supposed to be customized)
- 32x4 LCD RAM at the left
- no output PLA

TODO:
- row(pc) order is unknown
- each opcode is 4 cycles instead of 6
- does it have CL (call latch)
- everything else

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
	// TCY, YNEC, TCMIY
	static const u16 micro1[3] = { 0x0402, 0x1204, 0x1032 };

	// 0x20, 0x30, 0x00, 0x70
	static const u16 micro2[0x40] =
	{
		0x0102, 0x0801, 0x0802, 0x1001, 0x306a, 0x303a, 0x2021, 0x2020,
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,

		0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0e04, 0x0e04, 0x0e04, 0x0e04, 0x0899, 0x0099, 0x0819, 0x0804,

		0x0904, 0x0898, 0x1104, 0x2821, 0x104a, 0x101a, 0x0909, 0x0849,
		0x0401, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0404, 0x0000,

		0x0519, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0519,
		0x0000, 0x0519, 0x0519, 0x0000, 0x0000, 0x0000, 0x0519, 0x0419,
	};

	u16 mask = 0;

	if (offset >= 0x40 && offset < 0x70)
		mask = micro1[offset >> 4 & 3];
	else if (offset < 0x80 && (offset & 0xf0) != 0x10)
		mask = micro2[((offset ^ 0x20) | (offset >> 1 & 0x20)) & 0x3f];

	// does not have M_MTN or M_STSL
	const u32 md[14] = { M_AUTA, M_AUTY, M_NE, M_C8, M_CIN, M_CKM, M_15TN, M_NATN, M_ATN, M_CKN, M_CKP, M_MTP, M_YTP, M_STO };
	u32 decode = 0;

	for (int bit = 0; bit < 14; bit++)
		if (mask & (1 << bit))
			decode |= md[bit];

	return decode;
}

void smc1102_cpu_device::device_reset()
{
	tms1100_cpu_device::device_reset();

	// changed/added fixed instructions (mostly handled in op_extra)
	m_fixed_decode[0x0a] = F_EXTRA;
	m_fixed_decode[0x71] = F_EXTRA;
	m_fixed_decode[0x74] = F_EXTRA;
	m_fixed_decode[0x75] = F_EXTRA;
	m_fixed_decode[0x76] = F_RETN;
	m_fixed_decode[0x78] = F_EXTRA;
	m_fixed_decode[0x7b] = F_EXTRA;

	m_fixed_decode[0x72] = m_fixed_decode[0x73] = F_EXTRA;
	m_fixed_decode[0x7c] = m_fixed_decode[0x7d] = F_EXTRA;
}


// opcode deviations
void smc1102_cpu_device::op_tasr()
{
	// TASR: transfer A to LCD S/R
}

void smc1102_cpu_device::op_tsg()
{
	// TSG: transfer LCD S/R to RAM
}

void smc1102_cpu_device::op_intdis()
{
	// INTDIS: disable interrupt
}

void smc1102_cpu_device::op_inten()
{
	// INTEN: enable interrupt after next instruction
}

void smc1102_cpu_device::op_selin()
{
	// SELIN: select interrupt
}

void smc1102_cpu_device::op_tmset()
{
	// TMSET: transfer A to timer latch
}

void smc1102_cpu_device::op_halt()
{
	// HALT: stop CPU
}

void smc1102_cpu_device::op_extra()
{
	switch (m_opcode)
	{
		case 0x0a: op_tasr(); break;
		case 0x71: op_halt(); break;
		case 0x74: op_inten(); break;
		case 0x75: op_intdis(); break;
		case 0x78: op_selin(); break;
		case 0x7b: op_tmset(); break;

		case 0x72: case 0x73: case 0x7c: case 0x7d:
			op_tsg(); break;

		default: break;
	}
}
