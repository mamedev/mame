// license:BSD-3-Clause
// copyright-holders:Andrea Bogazzi
/*
    Jaleco "FPU" math coprocessor (F-1 Super Battle)

    Unidentified part, programmed by the host at boot through shared RAM.
    Instruction set reverse-engineered by Andrea Bogazzi from the program
    the V70 uploads, validated against its built-in self-test.

    20-bit instructions: 4-bit opcode, 16-bit argument.
    Register operations: fn[15:10] a[9:6] m[5:4] b[3:0], 16 registers s0-sf,
    two loop counters c6/c7, flags Z N C V.

    TODO:
    - operations the F-1 program never uses are logged, not implemented
      (unknown fn values, register group ops other than clear/latch/saturate,
      control words other than halt)
    - memory addressing selector 0, base of selector 5 unverified
    - clock, second interrupt output (host ROM names "fpu 0-1"/"fpu 1-0"),
      host registers other than s0-sf/pc/ctrl
*/

#include "emu.h"
#include "jalfpu.h"
#include "jalfpu_dasm.h"

#define LOG_UNIMPL (1U << 1)
#define LOG_HOST   (1U << 2)

#define VERBOSE (0)
#include "logmacro.h"

#define LOGUNIMPL(...) LOGMASKED(LOG_UNIMPL, __VA_ARGS__)
#define LOGHOST(...)   LOGMASKED(LOG_HOST, __VA_ARGS__)


namespace {

constexpr u8 LOAD_SEL[6] = { 0x3, 0x7, 0xb, 0xd, 0xe, 0xf };
constexpr u8 GROUP_REG[6] = { 0x6, 0x5, 0x4, 0x2, 0x1, 0x0 };
constexpr u16 MODE_BASE[8] = { 0x000, 0x000, 0x100, 0x200, 0x000, 0x300, 0x300, 0x600 };

// control words seen in the uploaded program
constexpr u16 CTL_HALT = 0x4080;    // end of routine, interrupts the host
constexpr u16 CTL_ENTRY_A = 0x4130; // routine entry marker (000 1c6 20d 237 338 38b), no effect
constexpr u16 CTL_ENTRY_B = 0x4100; // routine entry marker (14c 167 1b6), no effect

} // anonymous namespace


DEFINE_DEVICE_TYPE(JALECO_FPU, jaleco_fpu_device, "jalfpu", "Jaleco FPU math coprocessor")


jaleco_fpu_device::jaleco_fpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: cpu_device(mconfig, JALECO_FPU, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_LITTLE, 32, 10, -2, address_map_constructor(FUNC(jaleco_fpu_device::program_map), this))
	, m_data_config("data", ENDIANNESS_LITTLE, 16, 12, -1, address_map_constructor(FUNC(jaleco_fpu_device::data_map), this))
	, m_prg_ram(*this, "prg")
	, m_data_ram(*this, "data")
	, m_irq_cb(*this)
{
}

void jaleco_fpu_device::program_map(address_map &map)
{
	map(0x000, 0x3ff).ram().share(m_prg_ram);
}

void jaleco_fpu_device::data_map(address_map &map)
{
	map(0x000, 0x8ff).ram().share(m_data_ram);
}

void jaleco_fpu_device::host_map(address_map &map)
{
	map(0x0000, 0x23ff).umask32(0x0000ffff).rw(FUNC(jaleco_fpu_device::host_data_r), FUNC(jaleco_fpu_device::host_data_w));
	map(0x2400, 0x24ff).umask32(0x0000ffff).rw(FUNC(jaleco_fpu_device::host_r), FUNC(jaleco_fpu_device::host_w));
	map(0x4000, 0x5fff).umask32(0x0000ffff).rw(FUNC(jaleco_fpu_device::host_prg_r), FUNC(jaleco_fpu_device::host_prg_w));
}

device_memory_interface::space_config_vector jaleco_fpu_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_DATA,    &m_data_config)
	};
}

std::unique_ptr<util::disasm_interface> jaleco_fpu_device::create_disassembler()
{
	return std::make_unique<jaleco_fpu_disassembler>();
}

void jaleco_fpu_device::device_start()
{
	space(AS_PROGRAM).cache(m_program);
	space(AS_DATA).specific(m_data);

	m_pc = m_ppc = 0;
	std::fill(std::begin(m_s), std::end(m_s), 0);
	std::fill(std::begin(m_stack), std::end(m_stack), 0);
	std::fill(std::begin(m_hostreg), std::end(m_hostreg), 0);
	m_c6 = m_c7 = 0;
	m_flags = 0;
	m_sign = 0;
	m_sp = 0;
	m_ctrl = 0;
	m_running = false;
	m_delay = false;
	m_delay_target = 0;

	state_add(STATE_GENPC,     "GENPC", m_pc).noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_ppc).noshow();
	state_add(STATE_GENFLAGS,  "GENFLAGS", m_flags).formatstr("%4s").noshow();
	state_add(JALFPU_PC,       "PC", m_pc).mask(0x3ff);
	for (int i = 0; i < 16; i++)
		state_add(JALFPU_S0 + i, util::string_format("S%X", i), m_s[i]);
	state_add(JALFPU_C6,       "C6", m_c6);
	state_add(JALFPU_C7,       "C7", m_c7);
	state_add(JALFPU_SP,       "SP", m_sp);
	state_add(JALFPU_CTRL,     "CTRL", m_ctrl);
	state_add(JALFPU_SIGN,     "SIGN", m_sign);
	for (int i = 0; i < 4; i++)
		state_add(JALFPU_STK0 + i, util::string_format("STK%d", i), m_stack[i]).mask(0x3ff);
	state_add(JALFPU_DELAY,    "DELAY", m_delay);
	state_add(JALFPU_DTGT,     "DTGT", m_delay_target).mask(0x3ff);

	save_item(NAME(m_pc));
	save_item(NAME(m_ppc));
	save_item(NAME(m_s));
	save_item(NAME(m_c6));
	save_item(NAME(m_c7));
	save_item(NAME(m_flags));
	save_item(NAME(m_sign));
	save_item(NAME(m_stack));
	save_item(NAME(m_sp));
	save_item(NAME(m_ctrl));
	save_item(NAME(m_hostreg));
	save_item(NAME(m_running));
	save_item(NAME(m_delay));
	save_item(NAME(m_delay_target));

	set_icountptr(m_icount);
}

void jaleco_fpu_device::device_reset()
{
	m_pc = m_ppc = 0;
	m_sp = 0;
	m_ctrl = 0;
	m_running = false;
	m_delay = false;
	m_irq_cb(CLEAR_LINE);
}

void jaleco_fpu_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	if (entry.index() == STATE_GENFLAGS)
	{
		str = util::string_format("%c%c%c%c",
				(m_flags & F_Z) ? 'Z' : '.',
				(m_flags & F_N) ? 'N' : '.',
				(m_flags & F_C) ? 'C' : '.',
				(m_flags & F_V) ? 'V' : '.');
	}
}


u16 jaleco_fpu_device::host_data_r(offs_t offset)
{
	return m_data_ram[offset];
}

void jaleco_fpu_device::host_data_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_data_ram[offset]);
}

u16 jaleco_fpu_device::host_prg_r(offs_t offset)
{
	u32 const word = m_prg_ram[offset >> 1];
	return BIT(offset, 0) ? (word & 0xffff) : (word >> 16);
}

void jaleco_fpu_device::host_prg_w(offs_t offset, u16 data, u16 mem_mask)
{
	u32 &word = m_prg_ram[offset >> 1];
	if (BIT(offset, 0))
		word = (word & 0xf0000) | (data & mem_mask) | (word & ~mem_mask & 0xffff);
	else
		word = (word & 0x0ffff) | u32((((word >> 16) & ~mem_mask) | (data & mem_mask)) & 0xf) << 16;
}

u16 jaleco_fpu_device::host_r(offs_t offset)
{
	if (offset < 0x10)
		return m_s[offset];

	switch (offset)
	{
	case 0x30: return m_pc;
	case 0x32: return m_ctrl;
	}

	if (!machine().side_effects_disabled())
		LOGHOST("%s: host_r %04x\n", machine().describe_context(), 0x2400 + offset * 4);
	return m_hostreg[offset & 0x3f];
}

void jaleco_fpu_device::host_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (offset < 0x10)
	{
		COMBINE_DATA(&m_s[offset]);
		return;
	}

	switch (offset)
	{
	case 0x30:
		COMBINE_DATA(&m_pc);
		m_pc &= 0x3ff;
		LOGHOST("%s: start at %03x\n", machine().describe_context(), m_pc);
		m_delay = false;
		m_running = true;
		break;

	case 0x32:
		COMBINE_DATA(&m_ctrl);
		if (data & mem_mask & 0x0006)
			m_irq_cb(CLEAR_LINE);
		break;

	default:
		LOGHOST("%s: host_w %04x = %04x\n", machine().describe_context(), 0x2400 + offset * 4, data);
		COMBINE_DATA(&m_hostreg[offset & 0x3f]);
		break;
	}
}


void jaleco_fpu_device::set_nz(u16 v)
{
	m_flags &= ~(F_Z | F_N);
	if (!v)
		m_flags |= F_Z;
	if (BIT(v, 15))
		m_flags |= F_N;
}

bool jaleco_fpu_device::condition(u8 code)
{
	bool const n = m_flags & F_N;
	bool const v = m_flags & F_V;
	switch (code)
	{
	case 0x0: return true;
	case 0x2: return m_c6 != 0;
	case 0x3: return m_c6-- != 0;
	case 0x4: return m_c7 != 0;
	case 0x5: return m_c7-- != 0;
	case 0x8: return m_flags & F_Z;
	case 0x9: return n != v; // signed <
	case 0xa: return (n == v) && !(m_flags & F_Z);
	case 0xd: return m_flags & F_C;
	case 0xe: return n;
	case 0xf: return v;
	default:
		LOGUNIMPL("%03x: unknown condition %x\n", m_ppc, code);
		return false;
	}
}

u16 jaleco_fpu_device::mem_addr(u8 mode, u8 base)
{
	u8 const sel = (mode >> 2) & 7;
	if (!sel)
		LOGUNIMPL("%03x: memory mode %02x\n", m_ppc, mode);
	u16 const addr = (m_s[base] + MODE_BASE[sel]) & 0xfff;
	if (BIT(mode, 1))
		m_s[base] += BIT(mode, 0) ? -1 : 1;
	return addr;
}

void jaleco_fpu_device::unimplemented(u32 op)
{
	LOGUNIMPL("%03x: unimplemented %x %04x\n", m_ppc, op >> 16, op & 0xffff);
}

void jaleco_fpu_device::op_alu(u32 op)
{
	unsigned const fn = (op >> 10) & 0x3f;
	unsigned const a = (op >> 6) & 0xf;
	unsigned const b = op & 0xf;

	u32 const d = m_s[b];
	u32 const s = m_s[a];
	u32 const cin = (m_flags & F_C) ? 1 : 0;
	u32 r;
	bool store = true;
	switch (fn)
	{
	case 0x07: case 0x0f:
		r = d + s + (fn == 0x0f ? cin : 0);
		m_flags &= ~(F_C | F_V);
		if (r & 0x10000)
			m_flags |= F_C;
		if ((d ^ r) & (s ^ r) & 0x8000)
			m_flags |= F_V;
		break;
	case 0x17: case 0x1f: case 0x27:
		r = d - s - (fn == 0x1f ? cin : 0);
		m_flags &= ~(F_C | F_V);
		if (r & 0x10000)
			m_flags |= F_C;
		if ((d ^ s) & (d ^ r) & 0x8000)
			m_flags |= F_V;
		store = fn != 0x27;
		break;
	case 0x2f:
		r = d & s;
		m_flags &= ~(F_C | F_V);
		break;
	case 0x37:
		r = d | s;
		m_flags &= ~(F_C | F_V);
		break;
	case 0x3f:
		r = d ^ s;
		m_flags &= ~(F_C | F_V);
		break;
	default:
		unimplemented(op);
		return;
	}
	set_nz(r);
	if (store)
		m_s[b] = r;
}

void jaleco_fpu_device::op_muldiv(u32 op)
{
	unsigned const fn = (op >> 10) & 0x3f;
	unsigned const a = (op >> 6) & 0xf;
	unsigned const b = op & 0xf;

	switch (fn)
	{
	case 0x1c: case 0x1d: case 0x1e:
	{
		s64 r = s64(s16(m_s[b])) * s64(m_s[a]);
		if (!BIT(m_sign, a))
			r = -r;
		m_s[b] = u64(r) >> 16;
		m_s[0xd] = r;
		set_nz(m_s[b]);
		break;
	}
	case 0x17:
	{
		s32 const r = s32(s16(m_s[b])) * s32(s16(m_s[a]));
		m_s[b] = u32(r) >> 16;
		m_s[0xd] = r;
		set_nz(m_s[b]);
		break;
	}
	case 0x07:
		set_nz(m_s[b]);
		break;
	case 0x27:
	{
		// on overflow and divide by zero the quotient saturates: the program
		// consumes results with no V check and clamps only one side (22c-231),
		// and the perspective divide at 2df overflows routinely in-game.
		u32 const dividend = (u32(m_s[b]) << 16) | m_s[0xd];
		u16 const divisor = m_s[a];
		if (!divisor || (dividend / divisor) > 0xffff)
		{
			LOGUNIMPL("%03x: unsigned divide %s\n", m_ppc, divisor ? "overflow" : "by zero");
			m_flags |= F_V;
			m_s[0xd] = 0xffff;
			set_nz(m_s[0xd]);
			break;
		}
		m_s[0xd] = dividend / divisor;
		m_s[b] = dividend % divisor;
		set_nz(m_s[0xd]);
		break;
	}
	case 0x2f:
	{
		s64 const dividend = s32((u32(m_s[b]) << 16) | m_s[0xd]);
		s16 const divisor = s16(m_s[a]);
		s64 const quotient = divisor ? dividend / divisor : (dividend < 0 ? -0x8000 : 0x7fff);
		if (!divisor || (quotient < -0x8000) || (quotient > 0x7fff))
		{
			LOGUNIMPL("%03x: signed divide %s\n", m_ppc, divisor ? "overflow" : "by zero");
			m_flags |= F_V;
			m_s[0xd] = (quotient < 0) ? 0x8000 : 0x7fff;
			set_nz(m_s[0xd]);
			break;
		}
		m_s[0xd] = quotient;
		m_s[b] = dividend % divisor;
		set_nz(m_s[0xd]);
		break;
	}
	default:
		unimplemented(op);
		break;
	}
}

void jaleco_fpu_device::op_move(u32 op)
{
	unsigned const fn = (op >> 10) & 0x3f;
	unsigned const a = (op >> 6) & 0xf;
	unsigned const b = op & 0xf;

	switch (fn)
	{
	case 0x3f: case 0x3c: case 0x3d: case 0x3e:
		m_s[b] = m_s[a];
		set_nz(m_s[b]);
		break;
	case 0x07:
	{
		u16 const v = m_s[a];
		m_s[b] = v + 1;
		set_nz(m_s[b]);
		m_flags &= ~(F_C | F_V);
		if (v == 0xffff)
			m_flags |= F_C;
		if (v == 0x7fff)
			m_flags |= F_V;
		break;
	}
	case 0x0f:
	{
		u16 const v = m_s[a];
		m_s[b] = v - 1;
		set_nz(m_s[b]);
		m_flags &= ~(F_C | F_V);
		if (v == 0)
			m_flags |= F_C;
		if (v == 0x8000)
			m_flags |= F_V;
		break;
	}
	case 0x17:
		m_s[b] = -m_s[a];
		set_nz(m_s[b]);
		break;
	case 0x1f:
	{
		u16 const v = m_s[a];
		m_s[b] = BIT(v, 15) ? -v : v;
		set_nz(m_s[b]);
		// N reflects the input being positive; matches the host ROM's projected Y table
		m_flags = (m_flags & ~F_N) | ((s16(v) > 0) ? F_N : 0);
		break;
	}
	case 0x27:
		m_s[b] = ~m_s[a];
		set_nz(m_s[b]);
		break;
	default:
		unimplemented(op);
		break;
	}
}

void jaleco_fpu_device::op_shift(u32 op)
{
	unsigned const fn = (op >> 10) & 0x3f;
	unsigned const a = (op >> 6) & 0xf;
	unsigned const b = op & 0xf;

	u16 const v = m_s[a];
	bool const cin = m_flags & F_C;
	bool cout;
	u16 r;
	switch (fn)
	{
	case 0x07: cout = BIT(v, 15); r = v << 1; break;
	case 0x0f: cout = BIT(v, 0); r = v >> 1; break;
	case 0x1f: cout = BIT(v, 0); r = u16(s16(v) >> 1); break;
	case 0x37: cout = BIT(v, 15); r = (v << 1) | (cin ? 1 : 0); break;
	case 0x3f: cout = BIT(v, 0); r = (v >> 1) | (cin ? 0x8000 : 0); break;
	default:
		unimplemented(op);
		return;
	}
	m_s[b] = r;
	m_flags &= ~F_C;
	if (cout)
		m_flags |= F_C;
	set_nz(r);
}

void jaleco_fpu_device::op_group(u32 op)
{
	unsigned const fn = (op >> 10) & 0x3f;
	u8 const mask = (op >> 4) & 0x3f;

	for (int i = 0; i < 6; i++)
	{
		if (!BIT(mask, i))
			continue;
		u8 const r = GROUP_REG[i];
		switch (fn)
		{
		case 0x1e: case 0x1f:
			m_sign &= ~(1 << r);
			break;
		case 0x2d:
			m_sign = (m_sign & ~(1 << r)) | ((m_flags & F_N) ? (1 << r) : 0);
			break;
		case 0x3a:
			if (m_flags & F_C)
				m_s[r] = 0xffff;
			break;
		default:
			unimplemented(op);
			return;
		}
	}
}

void jaleco_fpu_device::op_branch(u32 op)
{
	unsigned const fn = (op >> 10) & 0x3f;
	unsigned const code = fn & 0xf;
	bool const sense = BIT(fn, 4);
	bool const delay = BIT(fn, 5);
	u16 target = op & 0x3ff;
	bool taken;

	if (code == 0x7)
	{
		if (sense)
		{
			if (m_sp >= std::size(m_stack))
				LOGUNIMPL("%03x: stack overflow\n", m_ppc);
			else
				m_stack[m_sp++] = (m_ppc + (delay ? 2 : 1)) & 0x3ff;
		}
		else
		{
			if (!m_sp)
				LOGUNIMPL("%03x: stack underflow\n", m_ppc);
			else
				target = m_stack[--m_sp];
		}
		taken = true;
	}
	else
	{
		taken = condition(code) == sense;
	}

	if (taken)
	{
		if (delay)
		{
			m_delay = true;
			m_delay_target = target;
		}
		else
		{
			m_pc = target;
		}
	}
}

void jaleco_fpu_device::execute_one(u32 op)
{
	unsigned const opc = op >> 16;
	u16 const arg = op & 0xffff;
	unsigned const fn = arg >> 10;
	unsigned const a = (arg >> 6) & 0xf;
	unsigned const b = arg & 0xf;

	switch (opc)
	{
	case 0x0: case 0x1: case 0x2: case 0x3: case 0x4: case 0x5:
		m_s[LOAD_SEL[opc]] = arg;
		break;

	case 0x6:
		m_c6 = arg;
		break;

	case 0x7:
		m_c7 = arg;
		break;

	case 0x8:
		op_alu(op);
		break;

	case 0x9:
		op_muldiv(op);
		break;

	case 0xa:
		op_move(op);
		break;

	case 0xb:
		op_shift(op);
		break;

	case 0xc:
	{
		u16 const addr = mem_addr(fn & 0x1f, a);
		if (BIT(fn, 5))
			m_s[b] = m_data.read_word(addr);
		else
			m_data.write_word(addr, m_s[b]);
		break;
	}

	case 0xd:
		op_group(op);
		break;

	case 0xe:
		if (arg == CTL_HALT)
		{
			m_running = false;
			m_irq_cb(ASSERT_LINE);
		}
		else if (arg != CTL_ENTRY_A && arg != CTL_ENTRY_B)
			unimplemented(op);
		break;

	case 0xf:
		op_branch(op);
		break;
	}
}

void jaleco_fpu_device::execute_run()
{
	while (m_icount > 0)
	{
		if (!m_running)
		{
			debugger_wait_hook();
			m_icount = 0;
			return;
		}

		m_ppc = m_pc;
		debugger_instruction_hook(m_pc);

		u32 const op = m_program.read_dword(m_pc) & 0xfffff;
		bool const in_delay = m_delay;
		u16 const delay_target = m_delay_target;
		m_delay = false;
		m_pc = (m_pc + 1) & 0x3ff;

		execute_one(op);

		if (in_delay)
		{
			if (m_delay)
				LOGUNIMPL("%03x: branch in delay slot\n", m_ppc);
			m_pc = delay_target;
		}

		m_icount--;
	}
}
