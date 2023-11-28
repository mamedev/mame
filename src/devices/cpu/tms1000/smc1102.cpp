// license:BSD-3-Clause
// copyright-holders:hap
/*

  Suwa Seikosha (now Seiko Epson) SMC1102, SMC1112

SMC1102 is a CMOS MCU based on TMS1100, keeping the same ALU and opcode mnemonics.
The stack(CALL/RETN) works a bit differently. They also added a timer, interrupts,
and a built-in LCD controller.

In the USA, it was marketed by S-MOS Systems, an affiliate of Seiko Group.

SMC1112 die notes (SMC1102 is assumed to be the same):
- 128x4 RAM array at top-left
- 256*64 8-bit ROM array at the bottom
- 30-term MPLA with 14 microinstructions, and 16 fixed opcodes next to it
  (assumed neither of them is supposed to be customized)
- 32x4 LCD RAM at the left
- no output PLA

TODO:
- add (micro)instructions PLA if it turns out it can be customized
- add halt opcode

*/

#include "emu.h"
#include "smc1102.h"
#include "tms1k_dasm.h"


// device definitions
DEFINE_DEVICE_TYPE(SMC1102, smc1102_cpu_device, "smc1102", "Suwa Seikosha SMC1102") // 60-pin QFP or 42-pin DIP
DEFINE_DEVICE_TYPE(SMC1112, smc1112_cpu_device, "smc1112", "Suwa Seikosha SMC1112") // low power version


smc1102_cpu_device::smc1102_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 o_pins, u8 r_pins, u8 pc_bits, u8 byte_bits, u8 x_bits, u8 stack_levels, int rom_width, address_map_constructor rom_map, int ram_width, address_map_constructor ram_map) :
	tms1100_cpu_device(mconfig, type, tag, owner, clock, o_pins, r_pins, pc_bits, byte_bits, x_bits, stack_levels, rom_width, rom_map, ram_width, ram_map),
	m_write_segs(*this)
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

	// zerofill
	memset(m_lcd_ram, 0, sizeof(m_lcd_ram));
	m_lcd_sr = 0;
	m_inten = false;
	m_selin = 0;
	m_k_line = false;

	m_div = 0;
	m_timer = 0;
	m_timeout = false;
	m_tmset = 0;

	memset(m_stack, 0, sizeof(m_stack));
	m_sp = 0;
	m_pb_stack = 0;
	m_cb_stack = 0;
	m_x_stack = 0;
	m_y_stack = 0;
	m_s_stack = 0;

	// register for savestates
	save_item(NAME(m_lcd_ram));
	save_item(NAME(m_lcd_sr));
	save_item(NAME(m_inten));
	save_item(NAME(m_selin));
	save_item(NAME(m_k_line));

	save_item(NAME(m_div));
	save_item(NAME(m_timer));
	save_item(NAME(m_timeout));
	save_item(NAME(m_tmset));

	save_item(NAME(m_stack));
	save_item(NAME(m_sp));
	save_item(NAME(m_pb_stack));
	save_item(NAME(m_cb_stack));
	save_item(NAME(m_x_stack));
	save_item(NAME(m_y_stack));
	save_item(NAME(m_s_stack));
}

void smc1102_cpu_device::device_reset()
{
	tms1100_cpu_device::device_reset();

	m_inten = false;
	m_selin = 0;
	m_timeout = false;

	// changed/added fixed instructions (mostly handled in op_extra)
	m_fixed_decode[0x0a] = F_EXTRA; // TASR
	m_fixed_decode[0x71] = F_EXTRA; // HALT
	m_fixed_decode[0x74] = F_EXTRA; // INTEN
	m_fixed_decode[0x75] = F_EXTRA; // INTDIS
	m_fixed_decode[0x76] = F_RETN; // INTRTN
	m_fixed_decode[0x78] = F_EXTRA; // SELIN
	m_fixed_decode[0x7b] = F_EXTRA; // TMSET

	m_fixed_decode[0x72] = m_fixed_decode[0x73] = F_EXTRA; // TSG
	m_fixed_decode[0x7c] = m_fixed_decode[0x7d] = F_EXTRA; // "
}

u32 smc1102_cpu_device::decode_micro(offs_t offset)
{
	// TCY, YNEC, TCMIY
	static const u16 micro1[3] = { 0x0402, 0x1204, 0x1032 };

	// 0x00, 0x20, 0x30, 0x70
	static const u16 micro2[0x40] =
	{
		0x0904, 0x0898, 0x1104, 0x2821, 0x104a, 0x101a, 0x0909, 0x0849,
		0x0401, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0404, 0x0000,

		0x0102, 0x0801, 0x0802, 0x1001, 0x306a, 0x303a, 0x2021, 0x2020,
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,

		0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0e04, 0x0e04, 0x0e04, 0x0e04, 0x0899, 0x0099, 0x0819, 0x0804,

		0x0519, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0519,
		0x0000, 0x0519, 0x0519, 0x0000, 0x0000, 0x0000, 0x0519, 0x0419
	};

	static const int micro2h[8] = { 0x00, -1, 0x10, 0x20, -1, -1, -1, 0x30 };

	u16 mask = 0;

	if (offset >= 0x40 && offset < 0x70)
		mask = micro1[offset >> 4 & 3];
	else if (offset < 0x80 && (offset & 0xf0) != 0x10)
		mask = micro2[micro2h[offset >> 4] | (offset & 0xf)];

	// does not have M_MTN or M_STSL
	const u32 md[14] = { M_AUTA, M_AUTY, M_NE, M_C8, M_CIN, M_CKM, M_15TN, M_NATN, M_ATN, M_CKN, M_CKP, M_MTP, M_YTP, M_STO };
	u32 decode = 0;

	for (int bit = 0; bit < 14; bit++)
		if (mask & (1 << bit))
			decode |= md[bit];

	return decode;
}


// interrupt/timer
void smc1102_cpu_device::execute_set_input(int line, int state)
{
	switch (line)
	{
		case SMC1102_INPUT_LINE_K:
			m_k_line = bool(state);
			break;

		default:
			break;
	}
}

void smc1102_cpu_device::read_opcode()
{
	// return from interrupt
	if (m_opcode == 0x76)
	{
		// restore registers
		m_pb = m_pb_stack;
		m_cb = m_cb_stack;
		m_x = m_x_stack;
		m_y = m_y_stack;
		m_status = m_s_stack;
	}

	// check interrupts (blocked after INTEN)
	if (m_opcode != 0x74)
	{
		const bool taken = (m_selin & 2) ? m_timeout : m_k_line;
		m_timeout = false;

		if (m_inten && taken)
		{
			interrupt();
			return;
		}
	}

	tms1100_cpu_device::read_opcode();
}

void smc1102_cpu_device::interrupt()
{
	standard_irq_callback(0, m_rom_address);

	// save registers
	m_pb_stack = m_pb;
	m_cb_stack = m_cb;
	m_x_stack = m_x;
	m_y_stack = m_y;
	m_s_stack = m_status;

	// insert CALL to 0 on page 14
	m_opcode = 0xc0;
	m_c4 = 0;
	m_fixed = m_fixed_decode[m_opcode];
	m_micro = m_micro_decode[m_opcode];

	m_pb = 0xe;
	m_cb = 0;
	m_status = 1;
	m_inten = false;
}

void smc1102_cpu_device::execute_run()
{
	while (m_icount > 0)
	{
		m_icount--;

		// decrement timer
		m_div = (m_div + 1) & 0x1fff;
		const u16 tmask = (m_selin & 1) ? 0x1ff : 0x1fff;
		if ((m_div & tmask) == 0)
		{
			m_timer = (m_timer - 1) & 0xf;
			if (m_timer == 0)
			{
				m_timer = m_tmset;
				m_timeout = true;
			}
		}

		// overall, LCD refresh rate is 64Hz
		if ((m_div & 0x1ff) == 0)
		{
			for (int i = 0; i < 4; i++)
				m_write_segs(i, m_lcd_ram[i]);
		}

		// 4 cycles per opcode instead of 6
		switch (m_subcycle)
		{
			case 2:
				execute_one(2);
				execute_one(3);
				break;

			case 3:
				execute_one(4);
				execute_one(5);
				break;

			default:
				execute_one(m_subcycle);
				break;
		}
		m_subcycle = (m_subcycle + 1) & 3;
	}
}


// opcode deviations
void smc1102_cpu_device::op_call()
{
	// CALL: call subroutine
	if (m_status)
	{
		m_stack[m_sp] = m_ca << 10 | m_pa << 6 | m_pc;
		m_sp = (m_sp + 1) % m_stack_levels;

		m_pc = m_opcode & m_pc_mask;
		m_pa = m_pb;
		m_ca = m_cb;
	}
}

void smc1102_cpu_device::op_retn()
{
	// RETN: return from subroutine
	m_sp = (m_stack_levels + m_sp - 1) % m_stack_levels;

	m_pc = m_stack[m_sp] & m_pc_mask;
	m_pa = m_pb = m_stack[m_sp] >> 6 & 0xf;
	m_ca = m_stack[m_sp] >> 10 & 1; // not CB
}

void smc1102_cpu_device::op_tasr()
{
	// TASR: transfer A to LCD S/R
	m_lcd_sr = m_lcd_sr << 4 | m_a;
}

void smc1102_cpu_device::op_tsg()
{
	// TSG: transfer LCD S/R to RAM
	m_lcd_ram[m_opcode & 3] = m_lcd_sr;
}

void smc1102_cpu_device::op_intdis()
{
	// INTDIS: disable interrupt
	m_inten = false;
}

void smc1102_cpu_device::op_inten()
{
	// INTEN: enable interrupt after next instruction
	m_inten = true;
}

void smc1102_cpu_device::op_selin()
{
	// SELIN: select interrupt
	m_selin = m_a & 3;
}

void smc1102_cpu_device::op_tmset()
{
	// TMSET: transfer A to timer latch
	m_tmset = m_a;
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
