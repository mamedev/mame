// license:BSD-3-Clause
// copyright-holders:superctr
/***************************************************************************

    Fujitsu MB8764 family DSP

    - 24-bit compound instructions: a transfer or jump with an ALU operation
    - two 128 x 16-bit data RAMs (ARAM 00-7f, BRAM 80-ff)
    - 16 x 16-bit two-stage pipelined multiplier, 26-bit accumulator D
    - 16-bit data bus: DMA input (D mode), output on request (REQ/BCT)
    - one instruction every two clocks

    Roland TVF16 (S-550, S-330, W-30): the MB87064's pinout, with 64 words
    of program RAM loaded over the data bus in place of the mask ROM.

    TODO:
    - encodings not known: MAB, MBA, LIB, LIV, LVP, ADY, GXY, SXY,
      register to register MOV, JOC to a RAM address, JOC on C1, F1, PL, MI
    - ABS, AND, ORA and DIV are placed by the data sheet's table alone;
      DIV isn't implemented
    - expansion RAM, P mode input, PCT

***************************************************************************/

#include "emu.h"
#include "mb8764.h"
#include "mb8764d.h"

#include <algorithm>
#include <cstdlib>
#include <iterator>

#define VERBOSE (0)
#include "logmacro.h"


DEFINE_DEVICE_TYPE(TVF16, tvf16_device, "tvf16", "Roland TVF16 DSP")

namespace {

// status bits, at their places in the CLR and SET masks
enum : u32
{
	ST_TRC = 1U << 2,
	ST_CLP = 1U << 3,
	ST_OV  = 1U << 4,
	ST_ESS = 1U << 5,
	ST_VS  = 1U << 6,
	ST_OF  = 1U << 7,
	ST_IF  = 1U << 8,
	ST_PCT = 1U << 9,
	ST_DMM = 1U << 10,
	ST_F1  = 1U << 16,
	ST_F0  = 1U << 17,
	ST_ZR  = 1U << 19,
	ST_MI  = 1U << 20
};

constexpr u32 ST_CLEARABLE = ST_F0 | ST_F1 | ST_DMM | ST_PCT | ST_IF | ST_OF | ST_VS | ST_ESS | ST_OV | ST_CLP | ST_TRC;
constexpr u32 ST_SETTABLE = ST_DMM | ST_PCT | ST_VS | ST_ESS | ST_CLP | ST_TRC;

} // anonymous namespace


mb8764_base_device::mb8764_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor program_map)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_BIG, 32, 10, -2, program_map)
	, m_data_config("data", ENDIANNESS_BIG, 16, 8, -1, address_map_constructor(FUNC(mb8764_base_device::data_map), this))
	, m_out_cb(*this)
{
}

device_memory_interface::space_config_vector mb8764_base_device::memory_space_config() const
{
	return space_config_vector{
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_DATA, &m_data_config)
	};
}

void mb8764_base_device::data_map(address_map &map)
{
	map(0x00, 0xff).ram();
}

std::unique_ptr<util::disasm_interface> mb8764_base_device::create_disassembler()
{
	return std::make_unique<mb8764_disassembler>();
}

void mb8764_base_device::device_start()
{
	space(AS_PROGRAM).cache(m_cache);
	space(AS_PROGRAM).specific(m_program);
	space(AS_DATA).specific(m_data);

	m_icount = 0;
	m_pcs = 0;
	m_a = 0;
	m_b = 0;
	m_d = 0;
	std::fill(std::begin(m_tr), std::end(m_tr), 0);
	m_x = 0;
	m_y = 0;
	m_c0 = 0;
	m_c1 = 0;
	m_vp = 0;
	m_pgm = 0;
	m_pgt = 0;
	m_exa = 0;
	m_ei = 0;
	m_eo = 0;

	set_icountptr(m_icount);

	state_add(STATE_GENPC, "GENPC", m_pc).noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_pc).noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS", m_st).formatstr("%11s").noshow();
	state_add(MB8764_PC, "PC", m_pc).mask(0x3ff);
	state_add(MB8764_PCS, "PCS", m_pcs).mask(0x3ff);
	state_add(MB8764_A, "A", m_a);
	state_add(MB8764_B, "B", m_b);
	state_add(MB8764_D, "D", m_d).mask(0x3ff'ffff).callimport();
	state_add(MB8764_X, "X", m_x).mask(0x7f);
	state_add(MB8764_Y, "Y", m_y);
	state_add(MB8764_C0, "C0", m_c0);
	state_add(MB8764_C1, "C1", m_c1).mask(0xf);
	state_add(MB8764_VP, "VP", m_vp).mask(0xf);
	state_add(MB8764_DMC, "DMC", m_dmc);
	state_add(MB8764_PGM, "PGM", m_pgm).mask(0x3);
	state_add(MB8764_PGT, "PGT", m_pgt).mask(0x7);
	state_add(MB8764_EXA, "EXA", m_exa).mask(0x3ff);
	state_add(MB8764_EI, "EI", m_ei);
	state_add(MB8764_EO, "EO", m_eo);

	save_item(NAME(m_pc));
	save_item(NAME(m_pcs));
	save_item(NAME(m_a));
	save_item(NAME(m_b));
	save_item(NAME(m_d));
	save_item(NAME(m_tr));
	save_item(NAME(m_x));
	save_item(NAME(m_y));
	save_item(NAME(m_c0));
	save_item(NAME(m_c1));
	save_item(NAME(m_vp));
	save_item(NAME(m_dmc));
	save_item(NAME(m_pgm));
	save_item(NAME(m_pgt));
	save_item(NAME(m_exa));
	save_item(NAME(m_ei));
	save_item(NAME(m_eo));
	save_item(NAME(m_st));
}

void mb8764_base_device::device_reset()
{
	m_pc = 0;
	m_dmc = 0;
	m_st = 0;
}

void mb8764_base_device::state_import(const device_state_entry &entry)
{
	switch (entry.index())
	{
	case MB8764_D:
		m_d = util::sext(m_d, 26);
		break;
	}
}

void mb8764_base_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
	case STATE_GENFLAGS:
		str = util::string_format("%c%c%c%c%c%c%c %c%c%c",
				(m_st & ST_F0) ? '0' : '.',
				(m_st & ST_F1) ? '1' : '.',
				(m_st & ST_IF) ? 'I' : '.',
				(m_st & ST_OF) ? 'O' : '.',
				(m_st & ST_OV) ? 'V' : '.',
				(m_st & ST_ZR) ? 'Z' : '.',
				(m_st & ST_MI) ? 'M' : '.',
				(m_st & ST_DMM) ? 'D' : '.',
				(m_st & ST_VS) ? 'S' : '.',
				(m_st & ST_CLP) ? 'C' : '.');
		break;
	}
}

void mb8764_base_device::fl0_w(int state)
{
	if (state)
		m_st |= ST_F0;
}

void mb8764_base_device::data_w(u16 data)
{
	m_ei = data;
	if (m_st & ST_DMM)
		m_data.write_word(m_dmc++, data);
	else
		m_st |= ST_IF;
}

s32 mb8764_base_device::product() const
{
	return (m_st & ST_TRC) ? (m_tr[1] >> 5) : ((m_tr[1] + 0x10) >> 5);
}

u16 mb8764_base_device::d_bus() const
{
	if ((m_st & ST_CLP) && (BIT(m_d, 25) != BIT(m_d, 24)))
		return BIT(m_d, 25) ? 0x8000 : 0x7fff;
	return (BIT(m_d, 25) << 15) | BIT(m_d, 9, 15);
}

void mb8764_base_device::alu(u8 code)
{
	s32 d;
	switch (code)
	{
	case 0x0: return;                                   // NOP
	case 0x1: d = std::abs(m_d); break;                 // ABS
	case 0x2: d = (s32(m_a) + m_b) * 512; break;        // ADD
	case 0x3: d = -m_d; break;                          // NEG
	case 0x4: d = product(); break;                     // MLT
	case 0x5: d = m_d >> 1; break;                      // SRA
	case 0x6: d = (s32(m_b) - m_a) * 512; break;        // SUB
	case 0x7: d = m_d * 2; break;                       // SLA
	case 0x8: d = m_d + product(); break;               // MSM
	case 0x9: d = m_d & (s32(m_a) * 512); break;        // AND
	case 0xa: d = m_d - product(); break;               // MRD
	case 0xb: d = m_d | (s32(m_a) * 512); break;        // ORA
	case 0xc: d = m_d + s32(m_a) * 512; break;          // SUM
	case 0xe: d = m_d - s32(m_a) * 512; break;          // RED
	case 0xf: d = ~m_d; break;                          // COM
	default:
		logerror("%03x: DIV unimplemented\n", (m_pc - 1) & 0x3ff);
		return;
	}

	m_d = util::sext(d, 26);
	m_st &= ~(ST_ZR | ST_MI);
	if (!m_d)
		m_st |= ST_ZR;
	if (m_d < 0)
		m_st |= ST_MI;
	if (BIT(m_d, 25) != BIT(m_d, 24))
		m_st |= ST_OV;
}

u8 mb8764_base_device::index_y(u8 address) const
{
	if (m_st & ST_VS)
		return (address & 0xf0) | ((address + m_y) & 0x0f);
	return address + m_y;
}

u16 mb8764_base_device::ram_r(bool internal, u8 address)
{
	if (!internal)
	{
		logerror("%03x: expansion RAM read %02x unimplemented\n", (m_pc - 1) & 0x3ff, address);
		return 0;
	}
	return m_data.read_word(address);
}

void mb8764_base_device::ram_w(bool internal, u8 address, u16 data)
{
	if (!internal)
	{
		logerror("%03x: expansion RAM write %02x = %04x unimplemented\n", (m_pc - 1) & 0x3ff, address, data);
		return;
	}
	m_data.write_word(address, data);
}

void mb8764_base_device::load_regs(u8 field, u16 data)
{
	if (BIT(field, 5))
		m_x = data & 0x7f;
	if (BIT(field, 4))
		m_y = (m_st & ST_VS) ? (data + m_vp) : data;
	if (BIT(field, 3))
		m_c0 = data;
	if (BIT(field, 2))
		m_c1 = data & 0xf;
	if (BIT(field, 1))
		m_pgm = BIT(data, 8, 2);
	if (BIT(field, 0))
		m_pgt = BIT(data, 7, 3);
}

void mb8764_base_device::clear(u32 mask)
{
	if (BIT(mask, 18))
		m_d = 0;
	if (BIT(mask, 15))
		m_dmc = 0;
	if (BIT(mask, 14))
		m_a = 0;
	if (BIT(mask, 13))
		m_b = 0;
	if (BIT(mask, 12))
		m_x = 0;
	if (BIT(mask, 11))
		m_y = 0;
	if (mask & 0x3)
		logerror("%03x: CLR of unknown bits %05x\n", (m_pc - 1) & 0x3ff, mask & 0x3);
	m_st &= ~(mask & ST_CLEARABLE);
}

void mb8764_base_device::set(u32 mask)
{
	if (mask & ~ST_SETTABLE)
		logerror("%03x: SET of unknown bits %05x\n", (m_pc - 1) & 0x3ff, mask & ~ST_SETTABLE);
	m_st |= mask & ST_SETTABLE;
}

bool mb8764_base_device::condition(u32 op)
{
	switch (op & 0xfc00)
	{
	case 0x1000:
		return !(m_st & ST_F0);
	case 0x4000:
		m_c0--;
		m_x = (m_x + 1) & 0x7f;
		m_y++;
		return m_c0 != 0;
	case 0x8400:
		return m_st & ST_OV;
	case 0x8800:
		return m_st & ST_ZR;
	case 0x9000:
		return !(m_st & ST_IF);
	case 0xa000:
		return m_st & ST_OF;
	default:
		logerror("%03x: unknown JOC condition %04x\n", (m_pc - 1) & 0x3ff, op & 0xfc00);
		return false;
	}
}

void mb8764_base_device::execute_one(u32 op)
{
	switch (BIT(op, 20, 4))
	{
	case 0x0: // LTB
	case 0x1: // LAB
	{
		const u8 a = (BIT(op, 9, 7) + (BIT(op, 16) ? m_x : 0)) & 0x7f;
		const u8 b = 0x80 | ((BIT(op, 8) ? index_y(op & 0x7f) : op) & 0x7f);
		const u16 adata = BIT(op, 20) ? m_data.read_word(a) : u16(m_program.read_dword((m_pgt << 7) + a));
		const u16 bdata = ram_r(BIT(op, 7), b);
		alu(BIT(op, 17, 3) << 1);
		m_a = adata;
		m_b = bdata;
		break;
	}

	case 0x4: // MOV $a,EO:A:B:D:EXA
	{
		const u16 data = ram_r(BIT(op, 9), BIT(op, 8) ? index_y(u8(op)) : u8(op));
		alu(BIT(op, 16, 4));
		if (BIT(op, 15))
			m_eo = data;
		if (BIT(op, 14))
			m_a = data;
		if (BIT(op, 13))
			m_b = data;
		if (BIT(op, 12))
			m_d = s32(s16(data)) * 512;
		if (BIT(op, 11))
			m_exa = data & 0x3ff;
		if (BIT(op, 10))
			logerror("%03x: MOV to unknown register %x\n", (m_pc - 1) & 0x3ff, BIT(op, 10, 6));
		break;
	}

	case 0x5: // NOP
		alu(BIT(op, 16, 4));
		break;

	case 0x6: // MOV $a,X:Y:C0:C1:PGM:PGT
	{
		const u16 data = ram_r(BIT(op, 9), BIT(op, 8) ? index_y(u8(op)) : u8(op));
		alu(BIT(op, 16, 4));
		load_regs(BIT(op, 10, 6), data);
		break;
	}

	case 0x7: // MOV #d,X:Y:C0:C1:PGM:PGT
		alu(BIT(op, 16, 4));
		load_regs(BIT(op, 10, 6), op & 0x3ff);
		break;

	case 0x8: // MOV D/A,EO:A:B:D:$a
	{
		static constexpr u8 codes[4] = { 0x0, 0x4, 0x8, 0xa };
		if (BIT(op, 17) || BIT(op, 10))
			logerror("%03x: unknown MOV form %06x\n", (m_pc - 1) & 0x3ff, op);
		const u16 data = BIT(op, 16) ? u16(m_a) : d_bus();
		alu(codes[BIT(op, 18, 2)]);
		if (BIT(op, 11))
			ram_w(BIT(op, 9), BIT(op, 8) ? index_y(u8(op)) : u8(op), data);
		if (BIT(op, 15))
			m_eo = data;
		if (BIT(op, 14))
			m_a = data;
		if (BIT(op, 13))
			m_b = data;
		if (BIT(op, 12))
			m_d = s32(s16(data)) * 512;
		break;
	}

	case 0xc: // LDI, MXY
		alu(BIT(op, 17, 3) << 1);
		if (BIT(op, 16))
		{
			m_x = (m_x + BIT(op, 9, 7)) & 0x7f;
			m_y += u8(op);
		}
		else
		{
			m_a = s16(op);
		}
		break;

	case 0xd: // JOC
	{
		const bool jump = condition(op);
		alu(BIT(op, 16, 4));
		if (jump)
			m_pc = op & 0x3ff;
		break;
	}

	case 0xe: // CLR, SET
		if (BIT(op, 19))
			set(op & 0x7'ffff);
		else
			clear(op & 0x7'ffff);
		break;

	case 0xf:
		if (BIT(op, 16)) // MOV #d,reg
		{
			switch (BIT(op, 17, 3))
			{
			case 0: m_eo = u16(op); break;
			case 1: m_out_cb(m_eo); break;
			case 2: m_d = s32(s16(op)) * 512; break;
			case 3: m_b = s16(op); break;
			case 5: m_exa = op & 0x3ff; break;
			default:
				logerror("%03x: unknown instruction %06x\n", (m_pc - 1) & 0x3ff, op);
				break;
			}
		}
		else
		{
			alu(BIT(op, 16, 4));
			switch (BIT(op, 12, 4))
			{
			case 0x0: // JSR
				m_pcs = m_pc;
				m_pc = op & 0x3ff;
				break;
			case 0x2: // RTS
				m_pc = m_pcs;
				break;
			case 0x4: // AVP
				m_vp = (m_vp + op) & 0xf;
				break;
			case 0xe: // JMP
				m_pc = op & 0x3ff;
				break;
			default:
				logerror("%03x: unknown instruction %06x\n", (m_pc - 1) & 0x3ff, op);
				break;
			}
		}
		break;

	default:
		logerror("%03x: unknown instruction %06x\n", (m_pc - 1) & 0x3ff, op);
		break;
	}

	m_tr[1] = m_tr[0];
	m_tr[0] = s32(m_a) * m_b;
}

u64 mb8764_base_device::execute_clocks_to_cycles(u64 clocks) const noexcept
{
	return (clocks + 2 - 1) / 2;
}

u64 mb8764_base_device::execute_cycles_to_clocks(u64 cycles) const noexcept
{
	return cycles * 2;
}

u32 mb8764_base_device::execute_min_cycles() const noexcept
{
	return 1;
}

u32 mb8764_base_device::execute_max_cycles() const noexcept
{
	return 1;
}

void mb8764_base_device::execute_run()
{
	while (m_icount > 0)
	{
		debugger_instruction_hook(m_pc);
		const u32 op = m_cache.read_dword(m_pc) & 0xff'ffff;
		m_pc = (m_pc + 1) & 0x3ff;
		execute_one(op);
		m_icount--;
	}
}


tvf16_device::tvf16_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: mb8764_base_device(mconfig, TVF16, tag, owner, clock, address_map_constructor(FUNC(tvf16_device::program_map), this))
{
}

void tvf16_device::program_map(address_map &map)
{
	map(0x000, 0x03f).mirror(0x3c0).ram();
}

void tvf16_device::device_start()
{
	mb8764_base_device::device_start();

	m_load_address = 0;
	m_load_high = 0;

	save_item(NAME(m_load_address));
	save_item(NAME(m_load_high));
}

void tvf16_device::program_w(u16 data)
{
	if (BIT(data, 15))
	{
		m_program.write_dword(m_load_address, (u32(m_load_high) << 15) | BIT(data, 0, 15));
	}
	else
	{
		m_load_address = (BIT(data, 9, 6) - 1) & 0x3f;
		m_load_high = BIT(data, 0, 9);
	}
}
