// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#include "emu.h"

#include "ns32000.h"
#include "ns32000dasm.h"
#include "debugger.h"

DEFINE_DEVICE_TYPE(NS32008, ns32008_device, "ns32008", "National Semiconductor NS32008")
DEFINE_DEVICE_TYPE(NS32016, ns32016_device, "ns32016", "National Semiconductor NS32016")
DEFINE_DEVICE_TYPE(NS32032, ns32032_device, "ns32032", "National Semiconductor NS32032")

/*
 * TODO:
 *  - prefetch queue
 *  - fetch/ea/data/rmw bus cycles
 *  - address translation/abort
 *  - unimplemented instructions
 *      - format 6: subp,addp
 *      - format 8: movus/movsu
 *      - format 14: rdval,wrval,lmr,smr
 *  - cascaded interrupts
 *  - opcode/operand/memory clock cycles
 *  - 32332, 32532
 */

enum psr_mask : u16
{
	// accessible in user mode
	PSR_C = 0x0001, // carry/borrow condition
	PSR_T = 0x0002, // trace trap enable
	PSR_L = 0x0004, // less than condition
					// unused
					// unused
	PSR_F = 0x0020, // general condition
	PSR_Z = 0x0040, // zero condition
	PSR_N = 0x0080, // negative condition

	// accessible in supervisor mode
	PSR_U = 0x0100, // user mode
	PSR_S = 0x0200, // stack pointer select
	PSR_P = 0x0400, // prevent multiple trace trap
	PSR_I = 0x0800, // interrupt enable
					// unused
					// unused
					// unused
					// unused
};

enum cfg_mask : u32
{
	CFG_I = 0x01, // vectored interrupts
	CFG_F = 0x02, // fpu present
	CFG_M = 0x04, // mmu present
	CFG_C = 0x08, // custom coprocessor present
};

enum trap_type : unsigned
{
	NVI   =  0, // non-vectored interrupt
	NMI   =  1, // non-maskable interrupt
	ABT   =  2, // abort
	FPU   =  3, // floating point unit
	ILL   =  4, // illegal operation
	SVC   =  5, // supervisor call
	DVZ   =  6, // integer divide by zero
	FLG   =  7, // flag instruction
	BPT   =  8, // breakpoint instruction
	TRC   =  9, // instruction trace
	UND   = 10, // undefined opcode
};

static const u32 size_mask[] = { 0x000000ffU, 0x0000ffffU, 0x00000000U, 0xffffffffU };

#define SP ((m_psr & PSR_S) ? m_sp1 : m_sp0)

template <int Width>ns32000_device<Width>::ns32000_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int databits, int addrbits)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_LITTLE, databits, addrbits, 0)
	, m_interrupt_config("interrupt", ENDIANNESS_LITTLE, databits, addrbits, 0)
	, m_fpu(*this, finder_base::DUMMY_TAG)
	, m_icount(0)
	, m_pc(0)
	, m_sb(0)
	, m_fp(0)
	, m_sp1(0)
	, m_sp0(0)
	, m_intbase(0)
	, m_psr(0)
	, m_mod(0)
	, m_cfg(0)
	, m_r{0}
	, m_f{0}
	, m_nmi_line(false)
	, m_int_line(false)
	, m_wait(false)
	, m_sequential(false)
{
}

ns32008_device::ns32008_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: ns32000_device(mconfig, NS32008, tag, owner, clock, 8, 24)
{
}

ns32016_device::ns32016_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: ns32000_device(mconfig, NS32016, tag, owner, clock, 16, 24)
{
}

ns32032_device::ns32032_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: ns32000_device(mconfig, NS32032, tag, owner, clock, 32, 24)
{
}

template <int Width> void ns32000_device<Width>::device_start()
{
	set_icountptr(m_icount);

	save_item(NAME(m_pc));
	save_item(NAME(m_sb));
	save_item(NAME(m_fp));
	save_item(NAME(m_sp1));
	save_item(NAME(m_sp0));
	save_item(NAME(m_intbase));
	save_item(NAME(m_psr));
	save_item(NAME(m_mod));
	save_item(NAME(m_cfg));

	save_item(NAME(m_r));
	save_item(NAME(m_f));

	save_item(NAME(m_nmi_line));
	save_item(NAME(m_int_line));
	save_item(NAME(m_wait));

	state_add(STATE_GENPC, "GENPC", m_pc).noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_pc).noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS", m_psr).mask(0xfe7).formatstr("%10s").noshow();

	// dedicated registers
	int index = 0;
	state_add(index++, "PC", m_pc);
	state_add(index++, "SB", m_sb);
	state_add(index++, "FP", m_fp);
	state_add(index++, "SP1", m_sp1);
	state_add(index++, "SP0", m_sp0);
	state_add(index++, "INTBASE", m_intbase);
	state_add(index++, "PSR", m_psr);
	state_add(index++, "MOD", m_mod);
	state_add(index++, "CFG", m_cfg);

	// general registers
	for (unsigned i = 0; i < 8; i++)
		state_add(index++, util::string_format("R%d", i).c_str(), m_r[i]);

	// floating point registers
	if (m_fpu)
		m_fpu->state_add(*this, index);
}

template <int Width> void ns32000_device<Width>::device_reset()
{
	for (std::pair<int, address_space_config const *> s : memory_space_config())
		space(has_configured_map(s.first) ? s.first : 0).cache(m_bus[s.first]);

	m_pc = 0;
	m_psr = 0;
	m_cfg = 0;

	m_nmi_line = false;
	m_int_line = false;
	m_wait = false;
}

template <int Width> void ns32000_device<Width>::state_string_export(device_state_entry const &entry, std::string &str) const
{
	switch (entry.index())
	{
	case STATE_GENFLAGS:
		str = string_format("%c%c%c%c%c%c%c%c%c%c",
			(m_psr & PSR_I) ? 'I' : '.',
			(m_psr & PSR_P) ? 'P' : '.',
			(m_psr & PSR_S) ? 'S' : '.',
			(m_psr & PSR_U) ? 'U' : '.',
			(m_psr & PSR_N) ? 'N' : '.',
			(m_psr & PSR_Z) ? 'Z' : '.',
			(m_psr & PSR_F) ? 'F' : '.',
			(m_psr & PSR_L) ? 'L' : '.',
			(m_psr & PSR_T) ? 'T' : '.',
			(m_psr & PSR_C) ? 'C' : '.');
		break;
	}
}

template <int Width> s32 ns32000_device<Width>::displacement(unsigned &bytes)
{
	s32 disp = space(0).read_byte(m_pc + bytes);
	if (BIT(disp, 7))
	{
		if (BIT(disp, 6))
		{
			// double word displacement
			disp = s32(swapendian_int32(space(0).read_dword_unaligned(m_pc + bytes)) << 2) >> 2;
			bytes += 4;
		}
		else
		{
			// word displacement
			disp = s16(swapendian_int16(space(0).read_word_unaligned(m_pc + bytes)) << 2) >> 2;
			bytes += 2;
		}
	}
	else
	{
		// byte displacement
		disp = s8(disp << 1) >> 1;
		bytes += 1;
	}

	return disp;
}

template <int Width> void ns32000_device<Width>::decode(addr_mode *mode, unsigned &bytes)
{
	bool scaled[] = { false, false };

	// scaled mode
	for (unsigned i = 0; i < 2; i++)
	{
		if (mode[i].gen > 0x1b)
		{
			u8 const index = space(0).read_byte(m_pc + bytes);
			bytes += 1;

			mode[i].disp = m_r[index & 7] << (mode[i].gen & 3);
			mode[i].gen = index >> 3;
			scaled[i] = true;
		}
	}

	// base mode
	for (unsigned i = 0; i < 2; i++)
	{
		switch (mode[i].gen)
		{
		case 0x00: case 0x01: case 0x02: case 0x03:
		case 0x04: case 0x05: case 0x06: case 0x07:
			// register
			if (scaled[i])
			{
				mode[i].base = m_r[mode[i].gen];
				mode[i].type = MEM;
			}
			else
				mode[i].type = REG;
			break;
		case 0x08: case 0x09: case 0x0a: case 0x0b:
		case 0x0c: case 0x0d: case 0x0e: case 0x0f:
			// register relative
			mode[i].base = m_r[mode[i].gen & 7] + displacement(bytes);
			mode[i].type = MEM;
			break;
		case 0x10:
			// frame memory relative disp2(disp1(FP))
			mode[i].base = m_fp + displacement(bytes);
			mode[i].disp += displacement(bytes);
			mode[i].type = IND;
			break;
		case 0x11:
			// stack memory relative disp2(disp1(SP))
			mode[i].base = SP + displacement(bytes);
			mode[i].disp += displacement(bytes);
			mode[i].type = IND;
			break;
		case 0x12:
			// static memory relative disp2(disp1(SB))
			mode[i].base = m_sb + displacement(bytes);
			mode[i].disp += displacement(bytes);
			mode[i].type = IND;
			break;
		case 0x13:
			// reserved
			break;
		case 0x14:
			// immediate
			switch (mode[i].size)
			{
			case SIZE_B: mode[i].imm = space(0).read_byte(m_pc + bytes); break;
			case SIZE_W: mode[i].imm = swapendian_int16(space(0).read_word_unaligned(m_pc + bytes)); break;
			case SIZE_D: mode[i].imm = swapendian_int32(space(0).read_dword_unaligned(m_pc + bytes)); break;
			case SIZE_Q: mode[i].imm = swapendian_int64(space(0).read_qword_unaligned(m_pc + bytes)); break;
			}
			bytes += mode[i].size + 1;
			mode[i].type = IMM;
			break;
		case 0x15:
			// absolute @disp
			mode[i].base = displacement(bytes);
			mode[i].type = MEM;
			break;
		case 0x16:
			// external EXT(disp1) + disp2
			mode[i].base = displacement(bytes) * 4;
			mode[i].disp += displacement(bytes);
			mode[i].type = EXT;
			break;
		case 0x17:
			// top of stack TOS
			mode[i].base = SP;
			mode[i].type = scaled[i] ? MEM : TOS;
			break;
		case 0x18:
			// frame memory disp(FP)
			mode[i].base = m_fp + displacement(bytes);
			mode[i].type = MEM;
			break;
		case 0x19:
			// stack memory disp(SP)
			mode[i].base = SP + displacement(bytes);
			mode[i].type = MEM;
			break;
		case 0x1a:
			// static memory disp(SB)
			mode[i].base = m_sb + displacement(bytes);
			mode[i].type = MEM;
			break;
		case 0x1b:
			// program memory *+disp
			mode[i].base = m_pc + displacement(bytes);
			mode[i].type = MEM;
			break;
		}
	}
}

template <int Width> u32 ns32000_device<Width>::ea(addr_mode const mode)
{
	u32 base;

	switch (mode.type)
	{
	case REG:
		base = m_r[mode.gen];
		break;

	case IND:
		base = m_bus[12].read_dword_unaligned(mode.base);
		break;

	case EXT:
		base = m_bus[12].read_dword_unaligned(m_mod + 4);
		base = m_bus[12].read_dword_unaligned(base + mode.base);
		break;

	default:
		base = mode.base;
		break;
	}

	return base + mode.disp;
}

template <int Width> u64 ns32000_device<Width>::gen_read(addr_mode mode)
{
	u64 data = 0;

	switch (mode.type)
	{
	case IMM:
		data = mode.imm;
		break;

	case REG:
		data = m_r[mode.gen] & size_mask[mode.size];
		break;

	case TOS:
		switch (mode.size)
		{
		case SIZE_B: data = space(0).read_byte(SP); break;
		case SIZE_W: data = space(0).read_word_unaligned(SP); break;
		case SIZE_D: data = space(0).read_dword_unaligned(SP); break;
		case SIZE_Q: data = space(0).read_qword_unaligned(SP); break;
		}

		// post-increment stack pointer
		if (mode.access == READ)
			SP += mode.size + 1;
		break;

	default:
		switch (mode.size)
		{
		case SIZE_B: data = space(0).read_byte(ea(mode)); break;
		case SIZE_W: data = space(0).read_word_unaligned(ea(mode)); break;
		case SIZE_D: data = space(0).read_dword_unaligned(ea(mode)); break;
		case SIZE_Q: data = space(0).read_qword_unaligned(ea(mode)); break;
		}
		break;
	}

	return data;
}

template <int Width> s64 ns32000_device<Width>::gen_read_sx(addr_mode mode)
{
	u64 data = gen_read(mode);

	switch (mode.size)
	{
	case SIZE_B: data = s8(data); break;
	case SIZE_W: data = s16(data); break;
	case SIZE_D: data = s32(data); break;
	case SIZE_Q: data = s64(data); break;
	}

	return data;
}

template <int Width> void ns32000_device<Width>::gen_write(addr_mode mode, u64 data)
{
	switch (mode.type)
	{
	case REG:
		m_r[mode.gen] = (m_r[mode.gen] & ~size_mask[mode.size]) | (data & size_mask[mode.size]);
		break;

	case TOS:
		// pre-decrement stack pointer
		if (mode.access == WRITE)
			SP -= mode.size + 1;

		switch (mode.size)
		{
		case SIZE_B: space(0).write_byte(SP, data); break;
		case SIZE_W: space(0).write_word_unaligned(SP, data); break;
		case SIZE_D: space(0).write_dword_unaligned(SP, data); break;
		case SIZE_Q: space(0).write_qword_unaligned(SP, data); break;
		}
		break;

	default:
		switch (mode.size)
		{
		case SIZE_B: space(0).write_byte(ea(mode), data); break;
		case SIZE_W: space(0).write_word_unaligned(ea(mode), data); break;
		case SIZE_D: space(0).write_dword_unaligned(ea(mode), data); break;
		case SIZE_Q: space(0).write_qword_unaligned(ea(mode), data); break;
		}
		break;
	}
}

template <int Width> bool ns32000_device<Width>::condition(unsigned const cc)
{
	switch (cc & 15)
	{
	case 0x0: // equal
		return (m_psr & PSR_Z);
	case 0x1: // not equal
		return !(m_psr & PSR_Z);
	case 0x2: // carry set
		return (m_psr & PSR_C);
	case 0x3: // carry clear
		return !(m_psr & PSR_C);
	case 0x4: // higher
		return (m_psr & PSR_L);
	case 0x5: // lower or same
		return !(m_psr & PSR_L);
	case 0x6: // greater than
		return (m_psr & PSR_N);
	case 0x7: // less or equal
		return !(m_psr & PSR_N);
	case 0x8: // flag set
		return (m_psr & PSR_F);
	case 0x9: // flag clear
		return !(m_psr & PSR_F);
	case 0xa: // lower
		return !(m_psr & PSR_L) && !(m_psr & PSR_Z);
	case 0xb: // higher or same
		return (m_psr & PSR_L) || (m_psr & PSR_Z);
	case 0xc: // less than
		return !(m_psr & PSR_N) && !(m_psr & PSR_Z);
	case 0xd: // greater or equal
		return (m_psr & PSR_N) || (m_psr & PSR_Z);
	case 0xe: // unconditionally true
		return true;
	case 0xf: // unconditionally false
		return false;
	}

	// can't happen
	return false;
}

template <int Width> void ns32000_device<Width>::flags(u32 const src1, u32 const src2, u32 const dest, unsigned const size, bool const subtraction)
{
	unsigned const sign_bit = (size + 1) * 8 - 1;

	bool const src1_s = BIT(src1, sign_bit);
	bool const src2_s = subtraction ? !BIT(src2, sign_bit) : BIT(src2, sign_bit);
	bool const dest_s = subtraction ? !BIT(dest, sign_bit) : BIT(dest, sign_bit);

	m_psr &= ~(PSR_F | PSR_C);
	if ((src2_s && src1_s) || (!dest_s && (src2_s || src1_s)))
		m_psr |= PSR_C;

	if ((src2_s == src1_s) && (dest_s != src2_s))
		m_psr |= PSR_F;
}

template <int Width> void ns32000_device<Width>::interrupt(unsigned const vector, u32 const return_address, bool const trap)
{
	// clear trace pending flag
	if (vector == TRC)
		m_psr &= ~PSR_P;

	// push psr
	m_sp0 -= 2;
	space(0).write_word_unaligned(m_sp0, m_psr);

	// update psr
	if (trap)
		m_psr &= ~(PSR_P | PSR_S | PSR_U | PSR_T);
	else
		m_psr &= ~(PSR_I | PSR_P | PSR_S | PSR_U | PSR_T);

	// fetch external procedure descriptor
	u32 const descriptor = space(0).read_dword_unaligned(m_intbase + vector * 4);

	// push mod
	m_sp0 -= 2;
	space(0).write_word_unaligned(m_sp0, m_mod);

	// push return address
	m_sp0 -= 4;
	space(0).write_dword_unaligned(m_sp0, return_address);

	// update mod, sb, pc
	m_mod = u16(descriptor);
	m_sb = space(0).read_dword_unaligned(m_mod + 0);
	m_pc = space(0).read_dword_unaligned(m_mod + 8) + u16(descriptor >> 16);

	// TODO: flush queue
	m_sequential = false;

	if (trap && (machine().debug_flags & DEBUG_FLAG_ENABLED))
		debug()->exception_hook(vector);
}

template <int Width> void ns32000_device<Width>::execute_run()
{
	while (m_icount > 0)
	{
		if (m_wait)
		{
			m_icount = 0;
			continue;
		}

		if (m_nmi_line)
		{
			// acknowledge interrupt and discard vector
			m_bus[4].read_byte(0xffff00);
			m_nmi_line = false;

			// service interrupt
			interrupt(NMI, m_pc, false);

			// notify the debugger
			if (machine().debug_flags & DEBUG_FLAG_ENABLED)
				debug()->interrupt_hook(INPUT_LINE_NMI);
		}
		else if (m_int_line && (m_psr & PSR_I))
		{
			// acknowledge interrupt and read vector
			s8 vector = m_bus[4].read_byte(0xfffe00);

			// check for non-vectored mode
			if (!(m_cfg & CFG_I))
				vector = NVI;
			else if (vector < 0)
			{
				// TODO: cascaded
			}

			// service interrupt
			interrupt(vector, m_pc, false);

			// notify the debugger
			if (machine().debug_flags & DEBUG_FLAG_ENABLED)
				debug()->interrupt_hook(INPUT_LINE_IRQ0);
		}

		// update trace pending
		if (m_psr & PSR_T)
			m_psr |= PSR_P;
		else
			m_psr &= ~PSR_P;

		debugger_instruction_hook(m_pc);

		u8 const opbyte = space(0).read_byte(m_pc);
		unsigned bytes = 1;
		unsigned tcy = 1;
		m_sequential = true;

		if ((opbyte & 15) == 10)
		{
			// format 0: cccc 1010
			// Bcond dst
			//       disp
			s32 const dst = displacement(bytes);

			if (condition(opbyte >> 4))
			{
				m_pc += dst;
				m_sequential = false;
				tcy = 6;
			}
			else
				tcy = 7;
		}
		else if ((opbyte & 15) == 2)
		{
			// format 1: oooo 0010
			switch (opbyte >> 4)
			{
			case 0x0:
				// BSR dst
				//     disp
				{
					s32 const dst = displacement(bytes);

					SP -= 4;
					space(0).write_dword_unaligned(SP, m_pc + bytes);

					m_pc += dst;
					m_sequential = false;
					tcy = 6;
				}
				break;
			case 0x1:
				// RET constant
				//     disp
				{
					s32 const constant = displacement(bytes);

					u32 const addr = space(0).read_dword_unaligned(SP);
					SP += 4 + constant;

					m_pc = addr;
					m_sequential = false;
					tcy = 2;
				}
				break;
			case 0x2:
				// CXP index
				//     disp
				{
					s32 const index = displacement(bytes);

					u32 const link_base = space(0).read_dword_unaligned(m_mod + 4);
					u32 const descriptor = space(0).read_dword_unaligned(link_base + index * 4);

					SP -= 4;
					space(0).write_dword_unaligned(SP, m_mod);
					SP -= 4;
					space(0).write_dword_unaligned(SP, m_pc + bytes);

					m_mod = u16(descriptor);
					m_sb = space(0).read_dword_unaligned(m_mod + 0);
					m_pc = space(0).read_dword_unaligned(m_mod + 8) + u16(descriptor >> 16);
					m_sequential = false;
					tcy = 16;
				}
				break;
			case 0x3:
				// RXP constant
				//     disp
				{
					s32 const constant = displacement(bytes);

					m_pc = space(0).read_dword_unaligned(SP);
					SP += 4;
					m_mod = space(0).read_dword_unaligned(SP);
					SP += 4;

					m_sb = space(0).read_dword_unaligned(m_mod + 0);
					SP += constant;
					m_sequential = false;
					tcy = 2;
				}
				break;
			case 0x4:
				// RETT constant
				//      disp
				if (!(m_psr & PSR_U))
				{
					s32 const constant = displacement(bytes);

					u32 &sp(SP);
					m_pc = space(0).read_dword_unaligned(sp);
					sp += 4;
					m_mod = space(0).read_word_unaligned(sp);
					sp += 2;
					m_psr = space(0).read_word_unaligned(sp);
					sp += 2;

					m_sb = space(0).read_dword_unaligned(m_mod);

					SP += constant;
					m_sequential = false;
					tcy = 35;
				}
				else
					interrupt(ILL, m_pc);
				break;
			case 0x5:
				// RETI
				if (!(m_psr & PSR_U))
				{
					// interrupt return bus cycle
					m_bus[6].read_byte(0xfffe00);

					u32 &sp(SP);
					m_pc = space(0).read_dword_unaligned(sp);
					sp += 4;
					m_mod = space(0).read_word_unaligned(sp);
					sp += 2;
					m_psr = space(0).read_word_unaligned(sp);
					sp += 2;

					m_sb = space(0).read_dword_unaligned(m_mod);
					m_sequential = false;
					tcy = 39;
				}
				else
					interrupt(ILL, m_pc);
				break;
			case 0x6:
				// SAVE reglist
				//      imm
				{
					u8 const reglist = space(0).read_byte(m_pc + bytes++);

					tcy = 13;
					for (unsigned i = 0; i < 8; i++)
					{
						if (BIT(reglist, i))
						{
							SP -= 4;
							space(0).write_dword_unaligned(SP, m_r[i]);
							tcy += 4;
						}
					}
				}
				break;
			case 0x7:
				// RESTORE reglist
				//         imm
				{
					u8 const reglist = space(0).read_byte(m_pc + bytes++);

					tcy = 12;
					for (unsigned i = 0; i < 8; i++)
					{
						if (BIT(reglist, i))
						{
							m_r[7 - i] = space(0).read_dword_unaligned(SP);
							SP += 4;
							tcy += 5;
						}
					}
				}
				break;
			case 0x8:
				// ENTER reglist,constant
				//       imm,disp
				{
					u8 const reglist = space(0).read_byte(m_pc + bytes++);
					s32 const constant = displacement(bytes);

					SP -= 4;
					space(0).write_dword_unaligned(SP, m_fp);
					m_fp = SP;
					SP -= constant;

					tcy = 18;
					for (unsigned i = 0; i < 8; i++)
					{
						if (BIT(reglist, i))
						{
							SP -= 4;
							space(0).write_dword_unaligned(SP, m_r[i]);
							tcy += 4;
						}
					}
				}
				break;
			case 0x9:
				// EXIT reglist
				//      imm
				{
					u8 const reglist = space(0).read_byte(m_pc + bytes++);

					tcy = 17;
					for (unsigned i = 0; i < 8; i++)
					{
						if (BIT(reglist, i))
						{
							m_r[7 - i] = space(0).read_dword_unaligned(SP);
							SP += 4;
							tcy += 5;
						}
					}
					SP = m_fp;
					m_fp = space(0).read_dword_unaligned(SP);
					SP += 4;
				}
				break;
			case 0xa:
				// NOP
				tcy = 3;
				break;
			case 0xb:
				// WAIT
				m_wait = true;
				tcy = 6;
				break;
			case 0xc:
				// DIA
				m_wait = true;
				tcy = 3;
				break;
			case 0xd:
				// FLAG
				if (m_psr & PSR_F)
				{
					interrupt(FLG, m_pc);
					tcy = 44;
				}
				else
					tcy = 6;
				break;
			case 0xe:
				// SVC
				interrupt(SVC, m_pc);
				tcy = 40;
				break;
			case 0xf:
				// BPT
				interrupt(BPT, m_pc);
				tcy = 40;
				break;
			}
		}
		else if ((opbyte & 15) == 12 || (opbyte & 15) == 13 || (opbyte & 15) == 15)
		{
			// format 2: gggg gsss sooo 11ii
			u16 const opword = space(0).read_word_unaligned(m_pc);
			bytes = 2;

			// HACK: use reserved mode for second unused type
			addr_mode mode[] = { addr_mode((opword >> 11) & 31), addr_mode(0x13) };

			unsigned const quick = (opword >> 7) & 15;
			size_code const size = size_code(opbyte & 3);

			switch ((opbyte >> 4) & 7)
			{
			case 0:
				// ADDQi src,dst
				//       quick,gen
				//             rmw.i
				{
					mode[0].rmw_i(size);
					decode(mode, bytes);

					u32 const src1 = s32(quick << 28) >> 28;
					u32 const src2 = gen_read(mode[0]);

					u32 const dst = src1 + src2;
					flags(src1, src2, dst, size, false);

					gen_write(mode[0], dst);

					tcy = (mode[0].type == REG) ? 4 : 6;
				}
				break;
			case 1:
				// CMPQi src1,src2
				//       quick,gen
				//             read.i
				{
					mode[0].read_i(size);
					decode(mode, bytes);

					u32 const src1 = (s32(quick << 28) >> 28) & size_mask[size];
					u32 const src2 = gen_read(mode[0]);

					m_psr &= ~(PSR_N | PSR_Z | PSR_L);

					if ((size == SIZE_D && s32(src1) > s32(src2))
					|| ((size == SIZE_W && s16(src1) > s16(src2))
					|| ((size == SIZE_B && s8(src1) > s8(src2)))))
						m_psr |= PSR_N;

					if (src1 == src2)
						m_psr |= PSR_Z;

					if ((size == SIZE_D && u32(src1) > u32(src2))
					|| ((size == SIZE_W && u16(src1) > u16(src2))
					|| ((size == SIZE_B && u8(src1) > u8(src2)))))
						m_psr |= PSR_L;

					tcy = 3;
				}
				break;
			case 2:
				// SPRi procreg,dst
				//      short,gen
				//            write.i
				mode[0].write_i(size);
				decode(mode, bytes);

				switch (quick)
				{
				case 0x0: // US
					// TODO: user stack pointer?
					gen_write(mode[0], m_sp1);
					break;
				case 0x8: // FP
					gen_write(mode[0], m_fp);
					break;
				case 0x9: // SP
					gen_write(mode[0], SP);
					break;
				case 0xa: // SB
					gen_write(mode[0], m_sb);
					break;
				case 0xd: // PSR
					if (!(m_psr & PSR_U))
						gen_write(mode[0], m_psr);
					else
						interrupt(ILL, m_pc);
					break;
				case 0xe: // INTBASE
					if (!(m_psr & PSR_U))
						gen_write(mode[0], m_intbase);
					else
						interrupt(ILL, m_pc);
					break;
				case 0xf: // MOD
					gen_write(mode[0], m_mod);
					break;
				}

				// TODO: tcy 21-27
				tcy = 21;
				break;
			case 3:
				// Scondi dst
				//        gen
				//        write.i
				{
					mode[0].write_i(size);
					decode(mode, bytes);

					bool const dst = condition(quick);
					gen_write(mode[0], dst);

					tcy = dst ? 10 : 9;
				}
				break;
			case 4:
				// ACBi inc,index,dst
				//      quick,gen,disp
				//            rmw.i
				{
					mode[0].rmw_i(size);
					decode(mode, bytes);

					s32 const inc = s32(quick << 28) >> 28;
					u32 index = gen_read(mode[0]);
					s32 const dst = displacement(bytes);

					index += inc;
					gen_write(mode[0], index);

					if (index & size_mask[size])
					{
						m_pc += dst;
						m_sequential = false;

						tcy = (mode[0].type == REG) ? 17 : 15;
					}
					else
						tcy = (mode[0].type == REG) ? 18 : 16;
				}
				break;
			case 5:
				// MOVQi src,dst
				//       quick,gen
				//             write.i
				mode[0].write_i(size);
				decode(mode, bytes);

				gen_write(mode[0], s32(quick << 28) >> 28);

				tcy = (mode[0].type == REG) ? 3 : 2;
				break;
			case 6:
				// LPRi procreg,src
				//      short,gen
				//            read.i
				mode[0].read_i(size);
				decode(mode, bytes);

				switch (quick)
				{
				case 0x0: // UPSR
					m_psr = (m_psr & 0xff00) | u8(gen_read(mode[0]));
					break;
				case 0x8: // FP
					m_fp = gen_read(mode[0]);
					break;
				case 0x9: // SP
					SP = gen_read(mode[0]);
					break;
				case 0xa: // SB
					m_sb = gen_read(mode[0]);
					break;
				case 0xd: // PSR
					if (!(m_psr & PSR_U))
					{
						u32 const src = gen_read(mode[0]);

						if (size == SIZE_B)
							m_psr = (m_psr & 0xff00) | u8(src);
						else
							m_psr = src;
					}
					else
						interrupt(ILL, m_pc);
					break;
				case 0xe: // INTBASE
					if (!(m_psr & PSR_U))
						m_intbase = gen_read(mode[0]);
					else
						interrupt(ILL, m_pc);
					break;
				case 0xf: // MOD
					m_mod = gen_read(mode[0]);
					break;
				default:
					interrupt(UND, m_pc);
					break;
				}

				// TODO: tcy 19-33
				tcy = 19;
				break;
			case 7:
				// format 3: gggg gooo o111 11ii
				switch ((opword >> 7) & 15)
				{
				case 0x0:
					// CXPD desc
					//      gen
					//      addr
					if (size == SIZE_D)
					{
						mode[0].addr();
						decode(mode, bytes);

						u32 const descriptor = gen_read(mode[0]);

						SP -= 4;
						space(0).write_dword_unaligned(SP, m_mod);
						SP -= 4;
						space(0).write_dword_unaligned(SP, m_pc + bytes);

						m_mod = u16(descriptor);
						m_sb = space(0).read_dword_unaligned(m_mod + 0);
						m_pc = space(0).read_dword_unaligned(m_mod + 8) + u16(descriptor >> 16);
						m_sequential = false;

						tcy = 13;
					}
					else
						interrupt(UND, m_pc);
					break;
				case 0x2:
					// BICPSRi src
					//         gen
					//         read.[BW]
					if (size == SIZE_B || size == SIZE_W)
					{
						mode[0].read_i(size);
						decode(mode, bytes);

						if (size == SIZE_B || !(m_psr & PSR_U))
						{
							u16 const src = gen_read(mode[0]);

							m_psr &= ~src;

							tcy = (size == SIZE_B) ? 18 : 30;
						}
						else
							interrupt(ILL, m_pc);
					}
					else
						interrupt(UND, m_pc);
					break;
				case 0x4:
					// JUMP dst
					//      gen
					//      addr
					if (size == SIZE_D)
					{
						mode[0].addr();
						decode(mode, bytes);

						m_pc = ea(mode[0]);
						m_sequential = false;

						tcy = 2;
					}
					else
						interrupt(UND, m_pc);
					break;
				case 0x6:
					// BISPSRi src
					//         gen
					//         read.[BW]
					if (size == SIZE_B || size == SIZE_W)
					{
						mode[0].read_i(size);
						decode(mode, bytes);

						if (size == SIZE_B || !(m_psr & PSR_U))
						{
							u16 const src = gen_read(mode[0]);

							m_psr |= src;

							tcy = (size == SIZE_B) ? 18 : 30;
						}
						else
							interrupt(ILL, m_pc);
					}
					else
						interrupt(UND, m_pc);
					break;
				case 0xa:
					// ADJSPi src
					//        gen
					//        read.i
					{
						mode[0].read_i(size);
						decode(mode, bytes);

						s32 const src = gen_read_sx(mode[0]);

						SP -= src;

						tcy = 6;
					}
					break;
				case 0xc:
					// JSR dst
					//     gen
					//     addr
					if (size == SIZE_D)
					{
						mode[0].addr();
						decode(mode, bytes);

						SP -= 4;
						space(0).write_dword_unaligned(SP, m_pc + bytes);

						m_pc = ea(mode[0]);
						m_sequential = false;

						tcy = 5;
					}
					else
						interrupt(UND, m_pc);
					break;
				case 0xe:
					// CASEi src
					//       gen
					//       read.i
					{
						mode[0].read_i(size);
						decode(mode, bytes);

						s32 const src = gen_read_sx(mode[0]);

						m_pc += src;
						m_sequential = false;

						tcy = 4;
					}
					break;
				default:
					interrupt(UND, m_pc);
					break;
				}
				break;
			}
		}
		else if ((opbyte & 3) != 2)
		{
			// format 4: xxxx xyyy yyoo ooii
			u16 const opword = space(0).read_word_unaligned(m_pc);
			bytes = 2;

			addr_mode mode[2] = { addr_mode((opword >> 11) & 31), addr_mode((opword >> 6) & 31) };
			size_code const size = size_code(opbyte & 3);

			switch ((opbyte >> 2) & 15)
			{
			case 0x0:
				// ADDi src,dst
				//      gen,gen
				//      read.i,rmw.i
				{
					mode[0].read_i(size);
					mode[1].rmw_i(size);
					decode(mode, bytes);

					u32 const src1 = gen_read(mode[0]);
					u32 const src2 = gen_read(mode[1]);

					u32 const dst = src1 + src2;
					flags(src1, src2, dst, size, false);

					gen_write(mode[1], dst);

					tcy = (mode[1].type == REG) ? 4 : 3;
				}
				break;
			case 0x1:
				// CMPi src1,src2
				//      gen,gen
				//      read.i,read.i
				{
					mode[0].read_i(size);
					mode[1].read_i(size);
					decode(mode, bytes);

					u32 const src1 = gen_read(mode[0]);
					u32 const src2 = gen_read(mode[1]);

					m_psr &= ~(PSR_N | PSR_Z | PSR_L);

					if ((size == SIZE_D && s32(src1) > s32(src2))
					|| ((size == SIZE_W && s16(src1) > s16(src2))
					|| ((size == SIZE_B && s8(src1) > s8(src2)))))
						m_psr |= PSR_N;

					if (src1 == src2)
						m_psr |= PSR_Z;

					if ((size == SIZE_D && u32(src1) > u32(src2))
					|| ((size == SIZE_W && u16(src1) > u16(src2))
					|| ((size == SIZE_B && u8(src1) > u8(src2)))))
						m_psr |= PSR_L;

					tcy = 3;
				}
				break;
			case 0x2:
				// BICi src,dst
				//      gen,gen
				//      read.i,rmw.i
				{
					mode[0].read_i(size);
					mode[1].rmw_i(size);
					decode(mode, bytes);

					u32 const src = gen_read(mode[0]);
					u32 const dst = gen_read(mode[1]);

					gen_write(mode[1], dst & ~src);

					tcy = (mode[1].type == REG) ? 4 : 3;
				}
				break;
			case 0x4:
				// ADDCi src,dst
				//      gen,gen
				//      read.i,rmw.i
				{
					mode[0].read_i(size);
					mode[1].rmw_i(size);
					decode(mode, bytes);

					u32 const src1 = gen_read(mode[0]);
					u32 const src2 = gen_read(mode[1]);

					u32 const dst = src1 + src2 + (m_psr & PSR_C);
					flags(src1, src2, dst, size, false);

					gen_write(mode[1], dst);

					tcy = (mode[1].type == REG) ? 4 : 3;
				}
				break;
			case 0x5:
				// MOVi src,dst
				//      gen,gen
				//      read.i,write.i
				{
					mode[0].read_i(size);
					mode[1].write_i(size);
					decode(mode, bytes);

					u32 const src = gen_read(mode[0]);

					gen_write(mode[1], src);

					tcy = (mode[1].type == REG) ? 3 : 1;
				}
				break;
			case 0x6:
				// ORi src,dst
				//      gen,gen
				//      read.i,rmw.i
				{
					mode[0].read_i(size);
					mode[1].rmw_i(size);
					decode(mode, bytes);

					u32 const src = gen_read(mode[0]);
					u32 const dst = gen_read(mode[1]);

					gen_write(mode[1], src | dst);

					tcy = (mode[1].type == REG) ? 4 : 3;
				}
				break;
			case 0x8:
				// SUBi src,dst
				//      gen,gen
				//      read.i,rmw.i
				{
					mode[0].read_i(size);
					mode[1].rmw_i(size);
					decode(mode, bytes);

					u32 const src1 = gen_read(mode[0]);
					u32 const src2 = gen_read(mode[1]);

					u32 const dst = src2 - src1;
					flags(src1, src2, dst, size, true);

					gen_write(mode[1], dst);

					tcy = (mode[1].type == REG) ? 4 : 3;
				}
				break;
			case 0x9:
				// ADDR src,dst
				//      gen,gen
				//      addr,write.D
				if (size == SIZE_D)
				{
					mode[0].addr();
					mode[1].write_i(size);
					decode(mode, bytes);

					gen_write(mode[1], ea(mode[0]));

					tcy = (mode[1].type == REG) ? 3 : 2;
				}
				else
					interrupt(UND, m_pc);
				break;
			case 0xa:
				// ANDi src,dst
				//      gen,gen
				//      read.i,rmw.i
				{
					mode[0].read_i(size);
					mode[1].rmw_i(size);
					decode(mode, bytes);

					u32 const src = gen_read(mode[0]);
					u32 const dst = gen_read(mode[1]);

					gen_write(mode[1], src & dst);

					tcy = (mode[1].type == REG) ? 4 : 3;
				}
				break;
			case 0xc:
				// SUBCi src,dst
				//      gen,gen
				//      read.i,rmw.i
				{
					mode[0].read_i(size);
					mode[1].rmw_i(size);
					decode(mode, bytes);

					u32 const src1 = gen_read(mode[0]);
					u32 const src2 = gen_read(mode[1]);

					u32 const dst = src2 - src1 - (m_psr & PSR_C);
					flags(src1, src2, dst, size, true);

					gen_write(mode[1], dst);

					tcy = (mode[1].type == REG) ? 4 : 3;
				}
				break;
			case 0xd:
				// TBITi offset,base
				//       gen,gen
				//       read.i,regaddr
				{
					mode[0].read_i(size);
					mode[1].regaddr();
					decode(mode, bytes);

					s32 const offset = gen_read_sx(mode[0]);

					if (mode[1].type == REG)
					{
						if (BIT(m_r[mode[1].gen], offset & 31))
							m_psr |= PSR_F;
						else
							m_psr &= ~PSR_F;

						tcy = 4;
					}
					else
					{
						u8 const byte = space(0).read_byte(ea(mode[1]) + (offset >> 3));

						if (BIT(byte, offset & 7))
							m_psr |= PSR_F;
						else
							m_psr &= ~PSR_F;

						tcy = 14;
					}
				}
				break;
			case 0xe:
				// XORi src,dst
				//      gen,gen
				//      read.i,rmw.i
				{
					mode[0].read_i(size);
					mode[1].rmw_i(size);
					decode(mode, bytes);

					u32 const src = gen_read(mode[0]);
					u32 const dst = gen_read(mode[1]);

					gen_write(mode[1], src ^ dst);

					tcy = (mode[1].type == REG) ? 4 : 3;
				}
				break;
			}
		}
		else switch (opbyte)
		{
		case 0x0e:
			// format 5: 0000 0sss s0oo ooii 0000 1110
			{
				u16 const opword = space(0).read_word_unaligned(m_pc + bytes);
				bytes += 2;

				size_code const size = size_code(opword & 3);

				// string instruction options
				bool const translate = BIT(opword, 7);
				bool const backward = BIT(opword, 8);
				unsigned const uw = (opword >> 9) & 3;

				switch ((opword >> 2) & 15)
				{
				case 0:
					// MOVSi options
					tcy = (translate || backward || uw) ? 54 : 18;

					m_psr &= ~PSR_F;
					while (m_r[0])
					{
						u32 data =
							(size == SIZE_D) ? space(0).read_dword_unaligned(m_r[1]) :
							(size == SIZE_W) ? space(0).read_word_unaligned(m_r[1]) :
							space(0).read_byte(m_r[1]);

						if (translate)
							data = space(0).read_byte(m_r[3] + u8(data));

						tcy += translate ? 27 : (backward || uw) ? 24 : 13;

						bool const match = !((m_r[4] ^ data) & size_mask[size]);
						if ((uw == 1 && !match) || (uw == 3 && match))
						{
							m_psr |= PSR_F;
							break;
						}

						if (size == SIZE_D)
							space(0).write_dword_unaligned(m_r[2], data);
						else if (size == SIZE_W)
							space(0).write_word_unaligned(m_r[2], data);
						else
							space(0).write_byte(m_r[2], data);

						if (backward)
						{
							m_r[1] -= size + 1;
							m_r[2] -= size + 1;
						}
						else
						{
							m_r[1] += size + 1;
							m_r[2] += size + 1;
						}

						m_r[0]--;
					}
					break;
				case 1:
					// CMPSi options
					tcy = 53;

					m_psr |= PSR_Z;
					m_psr &= ~(PSR_N | PSR_F | PSR_L);
					while (m_r[0])
					{
						u32 src1 =
							(size == SIZE_D) ? space(0).read_dword_unaligned(m_r[1]) :
							(size == SIZE_W) ? space(0).read_word_unaligned(m_r[1]) :
							space(0).read_byte(m_r[1]);
						u32 src2 =
							(size == SIZE_D) ? space(0).read_dword_unaligned(m_r[2]) :
							(size == SIZE_W) ? space(0).read_word_unaligned(m_r[2]) :
							space(0).read_byte(m_r[2]);

						if (translate)
							src1 = space(0).read_byte(m_r[3] + u8(src1));

						tcy += translate ? 38 : 35;

						bool const match = !((m_r[4] ^ src1) & size_mask[size]);
						if ((uw == 1 && !match) || (uw == 3 && match))
						{
							m_psr |= PSR_F;
							break;
						}

						if (src1 != src2)
						{
							m_psr &= ~PSR_Z;

							if ((size == SIZE_D && s32(src1) > s32(src2))
							|| ((size == SIZE_W && s16(src1) > s16(src2))
							|| ((size == SIZE_B && s8(src1) > s8(src2)))))
								m_psr |= PSR_N;

							if ((size == SIZE_D && u32(src1) > u32(src2))
							|| ((size == SIZE_W && u16(src1) > u16(src2))
							|| ((size == SIZE_B && u8(src1) > u8(src2)))))
								m_psr |= PSR_L;

							break;
						}

						if (backward)
						{
							m_r[1] -= size + 1;
							m_r[2] -= size + 1;
						}
						else
						{
							m_r[1] += size + 1;
							m_r[2] += size + 1;
						}

						m_r[0]--;
					}
					break;
				case 2:
					// SETCFG cfglist
					//        short
					if (!(m_psr & PSR_U))
					{
						m_cfg = (opword >> 7) & 15;

						tcy = 15;
					}
					else
						interrupt(ILL, m_pc);
					break;
				case 3:
					// SKPSi options
					tcy = 51;

					m_psr &= ~PSR_F;
					while (m_r[0])
					{
						u32 data =
							(size == SIZE_D) ? space(0).read_dword_unaligned(m_r[1]) :
							(size == SIZE_W) ? space(0).read_word_unaligned(m_r[1]) :
							space(0).read_byte(m_r[1]);

						if (translate)
							data = space(0).read_byte(m_r[3] + u8(data));

						tcy += translate ? 30 : 27;

						bool const match = !((m_r[4] ^ data) & size_mask[size]);
						if ((uw == 1 && !match) || (uw == 3 && match))
						{
							m_psr |= PSR_F;
							break;
						}

						if (backward)
							m_r[1] -= size + 1;
						else
							m_r[1] += size + 1;

						m_r[0]--;
					}
					break;
				default:
					interrupt(UND, m_pc);
					break;
				}
			}
			break;
		case 0x4e:
			// format 6: xxxx xyyy yyoo ooii 0100 1110
			{
				u16 const opword = space(0).read_word_unaligned(m_pc + bytes);
				bytes += 2;

				addr_mode mode[] = { addr_mode((opword >> 11) & 31), addr_mode((opword >> 6) & 31) };
				size_code const size = size_code(opword & 3);

				switch ((opword >> 2) & 15)
				{
				case 0x0:
					// ROTi count,dst
					//      gen,gen
					//      read.B,rmw.i
					{
						mode[0].read_i(SIZE_B);
						mode[1].rmw_i(size);
						decode(mode, bytes);

						s32 const count = gen_read_sx(mode[0]);
						u32 const src = gen_read(mode[1]);

						unsigned const limit = (size + 1) * 8 - 1;
						u32 const dst = ((src << (count & limit)) & size_mask[size]) | ((src & size_mask[size]) >> (limit - (count & limit) + 1));

						gen_write(mode[1], dst);

						tcy = 14 + (count & limit);
					}
					break;
				case 0x1:
					// ASHi count,dst
					//      gen,gen
					//      read.B,rmw.i
					{
						mode[0].read_i(SIZE_B);
						mode[1].rmw_i(size);
						decode(mode, bytes);

						s32 const count = gen_read_sx(mode[0]);
						s32 const src = gen_read_sx(mode[1]);

						u32 const dst = (count < 0) ? (src >> -count) : (src << count);

						gen_write(mode[1], dst);

						tcy = 14 + std::abs(count);
					}
					break;
				case 0x2:
					// CBITi offset,base
					//       gen,gen
					//       read.i,regaddr
				case 0x3:
					// CBITIi offset,base
					//        gen,gen
					//       read.i,regaddr
					{
						mode[0].read_i(size);
						mode[1].regaddr();
						decode(mode, bytes);

						s32 const offset = gen_read_sx(mode[0]);

						if (mode[1].type == REG)
						{
							if (BIT(m_r[mode[1].gen], offset & 31))
								m_psr |= PSR_F;
							else
								m_psr &= ~PSR_F;

							m_r[mode[1].gen] &= ~(1U << (offset & 31));

							tcy = 7;
						}
						else
						{
							u32 const byte_ea = ea(mode[1]) + (offset >> 3);
							u8 const byte = space(0).read_byte(byte_ea);

							if (BIT(byte, offset & 7))
								m_psr |= PSR_F;
							else
								m_psr &= ~PSR_F;

							space(0).write_byte(byte_ea, byte & ~(1U << (offset & 7)));

							tcy = 15;
						}
					}
					break;
				case 0x4:
					interrupt(UND, m_pc);
					break;
				case 0x5:
					// LSHi count,dst
					//      gen,gen
					//      read.B,rmw.i
					{
						mode[0].read_i(SIZE_B);
						mode[1].rmw_i(size);
						decode(mode, bytes);

						s32 const count = gen_read_sx(mode[0]);
						u32 const src = gen_read(mode[1]);

						u32 const dst = (count < 0) ? (src >> -count) : (src << count);

						gen_write(mode[1], dst);

						tcy = 14 + std::abs(count);
					}
					break;
				case 0x6:
					// SBITi offset,base
					//       gen,gen
					//       read.i,regaddr
				case 0x7:
					// SBITI offset,base
					//       gen,gen
					//       read.i,regaddr
					{
						mode[0].read_i(size);
						mode[1].regaddr();
						decode(mode, bytes);

						s32 const offset = gen_read_sx(mode[0]);

						if (mode[1].type == REG)
						{
							if (BIT(m_r[mode[1].gen], offset & 31))
								m_psr |= PSR_F;
							else
								m_psr &= ~PSR_F;

							m_r[mode[1].gen] |= (1U << (offset & 31));

							tcy = 7;
						}
						else
						{
							u32 const byte_ea = ea(mode[1]) + (offset >> 3);
							u8 const byte = space(0).read_byte(byte_ea);

							if (BIT(byte, offset & 7))
								m_psr |= PSR_F;
							else
								m_psr &= ~PSR_F;

							space(0).write_byte(byte_ea, byte | (1U << (offset & 7)));

							tcy = 15;
						}
					}
					break;
				case 0x8:
					// NEGi src,dst
					//      gen,gen
					//      read.i,write.i
					{
						mode[0].read_i(size);
						mode[1].write_i(size);
						decode(mode, bytes);

						u32 const src = gen_read(mode[0]);

						if (src)
							m_psr |= PSR_C;
						else
							m_psr &= ~PSR_C;

						if ((src ^ ~(size_mask[size] >> 1)) & size_mask[size])
						{
							m_psr &= ~PSR_F;
							gen_write(mode[1], -src);
						}
						else
						{
							m_psr |= PSR_F;
							gen_write(mode[1], src);
						}

						tcy = 5;
					}
					break;
				case 0x9:
					// NOTi src,dst
					//      gen,gen
					//      read.i,write.i
					{
						mode[0].read_i(size);
						mode[1].write_i(size);
						decode(mode, bytes);

						u32 const src = gen_read(mode[0]);

						gen_write(mode[1], src ^ 1U);

						tcy = 5;
					}
					break;
				case 0xa:
					interrupt(UND, m_pc);
					break;
				case 0xb:
					// SUBPi src,dst
					//      gen,gen
					//      read.i,rmw.i
					fatalerror("unimplemented: subp (%s)\n", machine().describe_context());

					// TODO: tcy 16/18
					break;
				case 0xc:
					// ABSi src,dst
					//      gen,gen
					//      read.i,write.i
					{
						mode[0].read_i(size);
						mode[1].write_i(size);
						decode(mode, bytes);

						s32 const src = gen_read_sx(mode[0]);

						m_psr &= ~PSR_F;
						if (src == s32(0x80000000) >> (32 - (size + 1) * 8))
						{
							m_psr |= PSR_F;
							gen_write(mode[1], src);
						}
						else
							gen_write(mode[1], std::abs(src));

						tcy = (src < 0) ? 9 : 8;
					}
					break;
				case 0xd:
					// COMi src,dst
					//      gen,gen
					//      read.i,write.i
					{
						mode[0].read_i(size);
						mode[1].write_i(size);
						decode(mode, bytes);

						u32 const src = gen_read(mode[0]);

						gen_write(mode[1], ~src);

						tcy = 7;
					}
					break;
				case 0xe:
					// IBITi offset,base
					//       gen,gen
					//       read.i,regaddr
					{
						mode[0].read_i(size);
						mode[1].regaddr();
						decode(mode, bytes);

						s32 const offset = gen_read_sx(mode[0]);

						if (mode[1].type == REG)
						{
							if (BIT(m_r[mode[1].gen], offset & 31))
								m_psr |= PSR_F;
							else
								m_psr &= ~PSR_F;

							m_r[mode[1].gen] ^= (1U << (offset & 31));

							tcy = 9;
						}
						else
						{
							u32 const byte_ea = ea(mode[1]) + (offset >> 3);
							u8 const byte = space(0).read_byte(byte_ea);

							if (BIT(byte, offset & 7))
								m_psr |= PSR_F;
							else
								m_psr &= ~PSR_F;

							space(0).write_byte(byte_ea, byte ^ (1U << (offset & 7)));

							tcy = 17;
						}
					}
					break;
				case 0xf:
					// ADDPi src,dst
					//       gen,gen
					//       read.i,rmw.i
					fatalerror("unimplemented: addp (%s)\n", machine().describe_context());

					// TODO: tcy 16/18
					break;
				}
			}
			break;
		case 0xce:
			// format 7: xxxx xyyy yyoo ooii 1100 1110
			{
				u16 const opword = space(0).read_word_unaligned(m_pc + bytes);
				bytes += 2;

				addr_mode mode[2] = { addr_mode((opword >> 11) & 31), addr_mode((opword >> 6) & 31) };
				size_code const size = size_code(opword & 3);

				switch ((opword >> 2) & 15)
				{
				case 0x0:
					// MOVMi block1,block2,length
					//       gen,gen,disp
					//       addr,addr
					{
						mode[0].addr();
						mode[1].addr();
						decode(mode, bytes);

						u32 block1 = ea(mode[0]);
						u32 block2 = ea(mode[1]);
						unsigned const num = displacement(bytes) / (size + 1) + 1;

						// TODO: aligned/unaligned transfers?
						for (unsigned i = 0; i < num; i++)
						{
							switch (size)
							{
							case SIZE_B: space(0).write_byte(block2, space(0).read_byte(block1)); break;
							case SIZE_W: space(0).write_word_unaligned(block2, space(0).read_word_unaligned(block1)); break;
							case SIZE_D: space(0).write_dword_unaligned(block2, space(0).read_dword_unaligned(block1)); break;
							default:
								// can't happen
								break;
							}

							block1 += (size + 1);
							block2 += (size + 1);
						}

						tcy = 20 + 3 * num;
					}
					break;
				case 0x1:
					// CMPMi block1,block2,length
					//       gen,gen,disp
					//       addr,addr
					{
						mode[0].addr();
						mode[1].addr();
						decode(mode, bytes);

						u32 block1 = ea(mode[0]);
						u32 block2 = ea(mode[1]);
						unsigned const num = displacement(bytes) / (size + 1) + 1;

						tcy = 24;

						m_psr |= PSR_Z;
						m_psr &= ~(PSR_N | PSR_L);

						// TODO: aligned/unaligned transfers?
						for (unsigned i = 0; i < num; i++)
						{
							s32 int1 = 0;
							s32 int2 = 0;

							switch (size)
							{
							case SIZE_B:
								int1 = s8(space(0).read_byte(block1));
								int2 = s8(space(0).read_byte(block2));
								break;
							case SIZE_W:
								int1 = s16(space(0).read_word_unaligned(block1));
								int2 = s16(space(0).read_word_unaligned(block2));
								break;
							case SIZE_D:
								int1 = s32(space(0).read_dword_unaligned(block1));
								int2 = s32(space(0).read_dword_unaligned(block2));
								break;
							default:
								// can't happen
								break;
							}

							tcy += 9;

							if (int1 != int2)
							{
								m_psr &= ~PSR_Z;
								if (int1 > int2)
									m_psr |= PSR_N;
								if (u32(int1) > u32(int2))
									m_psr |= PSR_L;

								break;
							}

							block1 += (size + 1);
							block2 += (size + 1);
						}
					}
					break;
				case 0x2:
					// INSSi src,base,offset,length
					//       gen,gen,imm
					//       read.i,regaddr
					{
						mode[0].read_i(size);
						mode[1].regaddr();
						decode(mode, bytes);

						u8 const imm = space(0).read_byte(m_pc + bytes++);
						unsigned const offset = imm >> 5;
						u32 const mask = ((2ULL << (imm & 31)) - 1) << offset;

						u32 const src = gen_read(mode[0]);
						u32 const base = gen_read(mode[1]);

						gen_write(mode[1], (base & ~mask) | ((src << offset) & mask));

						// TODO: tcy 39-49
						tcy = 39;
					}
					break;
				case 0x3:
					// EXTSi base,dst,offset,length
					//       gen,gen,imm
					//       regaddr,write.i
					{
						mode[0].regaddr();
						mode[1].write_i(size);
						decode(mode, bytes);

						u8 const imm = space(0).read_byte(m_pc + bytes++);
						unsigned const offset = imm >> 5;
						u32 const mask = (2ULL << (imm & 31)) - 1;

						u32 const base = gen_read(mode[0]);
						u32 const dst = (base >> offset) & mask;

						gen_write(mode[1], dst);

						// TODO: tcy 26-36
						tcy = 26;
					}
					break;
				case 0x4:
					// MOVXBW src,dst
					//        gen,gen
					//        read.B,write.W
					if (size == SIZE_B)
					{
						mode[0].read_i(size);
						mode[1].write_i(SIZE_W);
						decode(mode, bytes);

						u8 const src = gen_read(mode[0]);
						s16 const dst = s8(src);

						gen_write(mode[1], dst);

						tcy = 6;
					}
					else
						interrupt(UND, m_pc);
					break;
				case 0x5:
					// MOVZBW src,dst
					//        gen,gen
					//        read.B,write.W
					if (size == SIZE_B)
					{
						mode[0].read_i(size);
						mode[1].write_i(SIZE_W);
						decode(mode, bytes);

						u8 const src = gen_read(mode[0]);
						u16 const dst = src;

						gen_write(mode[1], dst);

						tcy = 5;
					}
					else
						interrupt(UND, m_pc);
					break;
				case 0x6:
					// MOVZiD src,dst
					//        gen,gen
					//        read.[BW],write.D
					if (size == SIZE_B || size == SIZE_W)
					{
						mode[0].read_i(size);
						mode[1].write_i(SIZE_D);
						decode(mode, bytes);

						u32 const src = gen_read(mode[0]);

						gen_write(mode[1], src);

						tcy = 5;
					}
					else
						interrupt(UND, m_pc);
					break;
				case 0x7:
					// MOVXiD src,dst
					//        gen,gen
					//        read.[BW],write.D
					if (size == SIZE_B || size == SIZE_W)
					{
						mode[0].read_i(size);
						mode[1].write_i(SIZE_D);
						decode(mode, bytes);

						u32 const src = gen_read(mode[0]);
						s32 const dst = (size == SIZE_W) ? s16(src) : s8(src);

						gen_write(mode[1], dst);

						tcy = 6;
					}
					else
						interrupt(UND, m_pc);
					break;
				case 0x8:
					// MULi src,dst
					//      gen,gen
					//      read.i,rmw.i
					{
						mode[0].read_i(size);
						mode[1].rmw_i(size);
						decode(mode, bytes);

						u32 const src1 = gen_read(mode[0]);
						u32 const src2 = gen_read(mode[1]);

						u32 const dst = src1 * src2;

						gen_write(mode[1], dst);

						tcy = 15;
					}
					break;
				case 0x9:
					// MEIi src,dst
					//      gen,gen
					//      read.i,rmw.2i
					{
						mode[0].read_i(size);
						mode[1].rmw_i(size_code(size * 2 + 1));
						decode(mode, bytes);

						u32 const src1 = gen_read(mode[0]);
						u32 const src2 = ((mode[1].type == REG)
							? m_r[mode[1].gen ^ 0]
							: gen_read(mode[1])) & size_mask[size];

						u64 const dst = mulu_32x32(src1, src2);

						if (mode[1].type == REG)
						{
							m_r[mode[1].gen ^ 0] = (m_r[mode[1].gen ^ 0] & ~size_mask[size]) | ((dst >> 0) & size_mask[size]);
							m_r[mode[1].gen ^ 1] = (m_r[mode[1].gen ^ 1] & ~size_mask[size]) | ((dst >> ((size + 1) * 8)) & size_mask[size]);
						}
						else
							gen_write(mode[1], dst);

						tcy = 23;
					}
					break;
				case 0xa:
					interrupt(UND, m_pc);
					break;
				case 0xb:
					// DEIi src,dst
					//      gen,gen
					//      read.i,rmw.2i
					{
						mode[0].read_i(size);
						mode[1].rmw_i(size_code(size * 2 + 1));
						decode(mode, bytes);

						u32 const src1 = gen_read(mode[0]);
						if (src1)
						{
							u64 const src2 = (mode[1].type == REG)
								? (u64(m_r[mode[1].gen ^ 1] & size_mask[size]) << ((size + 1) * 8)) | (m_r[mode[1].gen ^ 0] & size_mask[size])
								: gen_read(mode[1]);

							u32 const quotient = src2 / src1;
							u32 const remainder = src2 % src1;

							if (mode[1].type == REG)
							{
								m_r[mode[1].gen ^ 0] = (m_r[mode[1].gen ^ 0] & ~size_mask[size]) | (remainder & size_mask[size]);
								m_r[mode[1].gen ^ 1] = (m_r[mode[1].gen ^ 1] & ~size_mask[size]) | (quotient & size_mask[size]);

								tcy = 31;
							}
							else
							{
								gen_write(mode[1], (u64(quotient) << ((size + 1) * 8)) | remainder);

								tcy = 38;
							}
						}
						else
						{
							// restore stack pointer
							if (mode[0].type == TOS)
								SP -= size + 1;

							interrupt(DVZ, m_pc);
						}
					}
					break;
				case 0xc:
					// QUOi src,dst
					//      gen,gen
					//      read.i,rmw.i
					{
						mode[0].read_i(size);
						mode[1].rmw_i(size);
						decode(mode, bytes);

						s32 const src1 = gen_read_sx(mode[0]);
						if (src1)
						{
							s32 const src2 = gen_read_sx(mode[1]);

							s32 const dst = src2 / src1;

							gen_write(mode[1], dst);

							// TODO: tcy 49-55
							tcy = 49;
						}
						else
						{
							// restore stack pointer
							if (mode[0].type == TOS)
								SP -= size + 1;

							interrupt(DVZ, m_pc);
						}
					}
					break;
				case 0xd:
					// REMi src,dst
					//      gen,gen
					//      read.i,rmw.i
					{
						mode[0].read_i(size);
						mode[1].rmw_i(size);
						decode(mode, bytes);

						s32 const src1 = gen_read_sx(mode[0]);
						if (src1)
						{
							s32 const src2 = gen_read_sx(mode[1]);

							s32 const dst = src2 % src1;

							gen_write(mode[1], dst);

							// TODO: tcy 57-62
							tcy = 57;
						}
						else
						{
							// restore stack pointer
							if (mode[0].type == TOS)
								SP -= size + 1;

							interrupt(DVZ, m_pc);
						}
					}
					break;
				case 0xe:
					// MODi src,dst
					//      gen,gen
					//      read.i,rmw.i
					{
						mode[0].read_i(size);
						mode[1].rmw_i(size);
						decode(mode, bytes);

						s32 const src1 = gen_read_sx(mode[0]);
						if (src1)
						{
							s32 const src2 = gen_read_sx(mode[1]);

							s32 const dst = (src1 + (src2 % src1)) % src1;

							gen_write(mode[1], dst);

							// TODO: tcy 54-73
							tcy = 54;
						}
						else
						{
							// restore stack pointer
							if (mode[0].type == TOS)
								SP -= size + 1;

							interrupt(DVZ, m_pc);
						}
					}
					break;
				case 0xf:
					// DIVi src,dst
					//      gen,gen
					//      read.i,rmw.i
					{
						mode[0].read_i(size);
						mode[1].rmw_i(size);
						decode(mode, bytes);

						s32 const src1 = gen_read_sx(mode[0]);
						if (src1)
						{
							s32 const src2 = gen_read_sx(mode[1]);

							s32 const quotient = src2 / src1;
							s32 const remainder = src2 % src1;

							if ((quotient < 0) && remainder)
								gen_write(mode[1], quotient - 1);
							else
								gen_write(mode[1], quotient);

							// TODO: tcy 58-68
							tcy = 58;
						}
						else
						{
							// restore stack pointer
							if (mode[0].type == TOS)
								SP -= size + 1;

							interrupt(DVZ, m_pc);
						}
					}
					break;
				}
			}
			break;
		case 0x2e:
		case 0x6e:
		case 0xae:
		case 0xee:
			// format 8: xxxx xyyy yyrr roii oo10 1110
			{
				u16 const opword = space(0).read_word_unaligned(m_pc + bytes);
				bytes += 2;

				addr_mode mode[2] = { addr_mode((opword >> 11) & 31), addr_mode((opword >> 6) & 31) };
				unsigned const reg = (opword >> 3) & 7;
				size_code const size = size_code(opword & 3);

				switch ((opword & 4) | (opbyte >> 6))
				{
				case 0:
					// EXTi offset,base,dst,length
					//      reg,gen,gen,disp
					//          regaddr,write.i
					{
						mode[0].regaddr();
						mode[1].write_i(size);
						decode(mode, bytes);

						s32 const offset = m_r[reg];
						s32 const length = displacement(bytes);
						u32 const mask = (1U << length) - 1;

						u32 dst;
						if (mode[0].type != REG)
						{
							u32 const base_ea = ea(mode[0]) + (offset >> 3);
							u32 const base = space(0).read_dword_unaligned(base_ea);

							dst = (base >> (offset & 7));

							// TODO: tcy 17-51
							tcy = 17;
						}
						else
						{
							dst = (m_r[mode[0].gen] >> (offset & 31));

							// TODO: tcy 19-29
							tcy = 19;
						}

						gen_write(mode[1], dst & mask);
					}
					break;
				case 1:
					// CVTP offset,base,dst
					//      reg,gen,gen
					//          addr,write.D
					if (size == SIZE_D)
					{
						mode[0].addr();
						mode[1].write_i(size);
						decode(mode, bytes);

						s32 const offset = s32(m_r[reg]);
						u32 const base = ea(mode[0]);

						gen_write(mode[1], base * 8 + offset);

						tcy = 7;
					}
					else
						interrupt(UND, m_pc);
					break;
				case 2:
					// INSi offset,src,base,length
					//      reg,gen,gen,disp
					//          read.i,regaddr
					{
						mode[0].read_i(size);
						mode[1].regaddr();
						decode(mode, bytes);

						s32 const offset = m_r[reg];
						s32 const length = displacement(bytes);
						u32 const src = gen_read(mode[0]);

						u32 dst;
						if (mode[1].type == REG)
						{
							u32 const base = m_r[mode[1].gen];
							u32 const mask = ((1U << length) - 1) << (offset & 31);

							dst = (base & ~mask) | ((src << (offset & 31)) & mask);

							// TODO: tcy 29-39
							tcy = 29;
						}
						else
						{
							u32 const base_ea = ea(mode[1]) + (offset >> 3);
							u32 const base = space(0).read_dword_unaligned(base_ea);
							u32 const mask = ((1U << length) - 1) << (offset & 7);

							dst = (base & ~mask) | ((src << (offset & 7)) & mask);

							// TODO: tcy 28-96
							tcy = 28;
						}

						gen_write(mode[1], dst);
					}
					break;
				case 3:
					// CHECKi dst,bounds,src
					//        reg,gen,gen
					//            addr,read.i
					{
						mode[0].addr();
						mode[1].read_i(size);
						decode(mode, bytes);

						u32 const bounds = ea(mode[0]);
						s32 const src = gen_read_sx(mode[1]);

						s32 lower = 0;
						s32 upper = 0;
						switch (size)
						{
						case SIZE_B:
							upper = s8(space(0).read_byte(bounds + 0));
							lower = s8(space(0).read_byte(bounds + 1));
							break;
						case SIZE_W:
							upper = s16(space(0).read_word_unaligned(bounds + 0));
							lower = s16(space(0).read_word_unaligned(bounds + 2));
							break;
						case SIZE_D:
							upper = s32(space(0).read_dword_unaligned(bounds + 0));
							lower = s32(space(0).read_dword_unaligned(bounds + 4));
							break;
						default:
							// can't happen
							break;
						}

						if (src >= lower && src <= upper)
						{
							m_psr &= ~PSR_F;
							m_r[reg] = src - lower;

							tcy = 11;
						}
						else
						{
							m_psr |= PSR_F;

							tcy = (src >= lower) ? 7 : 10;
						}
					}
					break;
				case 4:
					// INDEXi accum,length,index
					//        reg,gen,gen
					//            read.i,read.i
					{
						mode[0].read_i(size);
						mode[1].read_i(size);
						decode(mode, bytes);

						u32 const length = gen_read(mode[0]);
						u32 const index = gen_read(mode[1]);

						m_r[reg] = m_r[reg] * (length + 1) + index;

						tcy = 25;
					}
					break;
				case 5:
					// FFSi base,offset
					//      gen,gen
					//      read.i,rmw.B
					{
						mode[0].read_i(size);
						mode[1].rmw_i(SIZE_B);
						decode(mode, bytes);

						u32 const base = gen_read(mode[0]);
						u32 offset = gen_read(mode[1]);
						unsigned const limit = (size + 1) * 8;

						m_psr |= PSR_F;
						while (offset < limit)
							if (BIT(base, offset))
							{
								m_psr &= ~PSR_F;
								break;
							}
							else
								offset++;

						gen_write(mode[1], offset & (limit - 1));

						// TODO: tcy 24-28
						tcy = 24;
					}
					break;
				case 6:
					// MOVSU/MOVUS src,dst
					//             gen,gen
					//             addr,addr
					{
						mode[0].addr();
						mode[1].addr();
						decode(mode, bytes);

						fatalerror("unimplemented: movsu/movus (%s)\n", machine().describe_context());

						tcy = 33;
					}
					break;
				}
			}
			break;
		case 0x3e:
			// format 9: xxxx xyyy yyoo ofii 0011 1110
			if (m_cfg & CFG_F)
			{
				if (!m_fpu)
					fatalerror("floating point unit not configured (%s)\n", machine().describe_context());

				u16 const opword = space(0).read_word_unaligned(m_pc + bytes);
				bytes += 2;

				addr_mode mode[2] = { addr_mode((opword >> 11) & 31), addr_mode((opword >> 6) & 31) };
				size_code const size_f = BIT(opword, 0) ? SIZE_D : SIZE_Q;
				size_code const size = size_code(opword & 3);

				m_fpu->write_id(opbyte);
				m_fpu->write_op(swapendian_int16(opword));

				switch ((opword >> 3) & 7)
				{
				case 0:
					// MOVif src,dst
					//       gen,gen
					//       read.i,write.f
					mode[0].read_i(size);
					mode[1].write_f(size_f);
					decode(mode, bytes);

					if (slave(mode[0], mode[1]))
						interrupt(FPU, m_pc);
					break;
				case 1:
					// LFSR src
					//      gen
					//      read.D
					mode[0].read_i(size);
					decode(mode, bytes);

					if (slave(mode[0], mode[1]))
						interrupt(FPU, m_pc);
					break;
				case 2:
					// MOVLF src,dst
					//       gen,gen
					//       read.L,write.F
					mode[0].read_f(SIZE_Q);
					mode[1].write_f(size_f);
					decode(mode, bytes);

					if (slave(mode[0], mode[1]))
						interrupt(FPU, m_pc);
					break;
				case 3:
					// MOVFL src,dst
					//       gen,gen
					//       read.F,write.L
					mode[0].read_f(SIZE_D);
					mode[1].write_f(size_f);
					decode(mode, bytes);

					if (slave(mode[0], mode[1]))
						interrupt(FPU, m_pc);
					break;
				case 4:
					// ROUNDfi src,dst
					//         gen,gen
					//         read.f,write.i
					mode[0].read_f(size_f);
					mode[1].write_i(size);
					decode(mode, bytes);

					if (slave(mode[0], mode[1]))
						interrupt(FPU, m_pc);
					break;
				case 5:
					// TRUNCfi src,dst
					//         gen,gen
					//         read.f,write.i
					mode[0].read_f(size_f);
					mode[1].write_i(size);
					decode(mode, bytes);

					if (slave(mode[0], mode[1]))
						interrupt(FPU, m_pc);
					break;
				case 6:
					// SFSR dst
					//      gen
					//      write.D
					mode[0].write_i(size);
					decode(mode, bytes);

					if (slave(mode[1], mode[0]))
						interrupt(FPU, m_pc);
					break;
				case 7:
					// FLOORfi src,dst
					//         gen,gen
					//         read.f,write.i
					mode[0].read_f(size_f);
					mode[1].write_i(size);
					decode(mode, bytes);

					if (slave(mode[0], mode[1]))
						interrupt(FPU, m_pc);
					break;
				}
			}
			else
				interrupt(UND, m_pc);
			break;
		case 0x7e: // format 10
			interrupt(UND, m_pc);
			break;
		case 0xbe:
			// format 11: xxxx xyyy yyoo oo0f 1011 1110
			if (m_cfg & CFG_F)
			{
				if (!m_fpu)
					fatalerror("floating point unit not configured (%s)\n", machine().describe_context());

				u16 const opword = space(0).read_word_unaligned(m_pc + bytes);
				bytes += 2;

				addr_mode mode[2] = { addr_mode((opword >> 11) & 31), addr_mode((opword >> 6) & 31) };
				size_code const size_f = BIT(opword, 0) ? SIZE_D : SIZE_Q;

				m_fpu->write_id(opbyte);
				m_fpu->write_op(swapendian_int16(opword));

				switch ((opword >> 2) & 15)
				{
				case 0x0:
					// ADDf src,dst
					//      gen,gen
					//      read.f,rmw.f
					mode[0].read_f(size_f);
					mode[1].rmw_f(size_f);
					decode(mode, bytes);

					if (slave(mode[0], mode[1]))
						interrupt(FPU, m_pc);
					break;
				case 0x1:
					// MOVf src,dst
					//      gen,gen
					//      read.f,write.f
					mode[0].read_f(size_f);
					mode[1].write_f(size_f);
					decode(mode, bytes);

					if (slave(mode[0], mode[1]))
						interrupt(FPU, m_pc);
					break;
				case 0x2:
					// CMPf src1,src2
					//      gen,gen
					//      read.f,read.f
					{
						mode[0].read_f(size_f);
						mode[1].read_f(size_f);
						decode(mode, bytes);

						u16 const status = slave(mode[0], mode[1]);
						if (!(status & ns32000_slave_interface::SLAVE_Q))
						{
							m_psr &= ~(PSR_N | PSR_Z | PSR_L);
							m_psr |= status & (ns32000_slave_interface::SLAVE_N | ns32000_slave_interface::SLAVE_Z | ns32000_slave_interface::SLAVE_L);
						}
						else
							interrupt(FPU, m_pc);
					}
					break;
				case 0x3:
					// Trap(SLAVE)
					// operands from ns32532 datasheet
					mode[0].read_f(size_f);
					mode[1].read_f(size_f);
					decode(mode, bytes);

					if (slave(mode[0], mode[1]))
						interrupt(FPU, m_pc);
					break;
				case 0x4:
					// SUBf src,dst
					//      gen,gen
					//      read.f,rmw.f
					mode[0].read_f(size_f);
					mode[1].rmw_f(size_f);
					decode(mode, bytes);

					if (slave(mode[0], mode[1]))
						interrupt(FPU, m_pc);
					break;
				case 0x5:
					// NEGf src,dst
					//      gen,gen
					//      read.f,write.f
					mode[0].read_f(size_f);
					mode[1].write_f(size_f);
					decode(mode, bytes);

					if (slave(mode[0], mode[1]))
						interrupt(FPU, m_pc);
					break;
				case 0x8:
					// DIVf src,dst
					//      gen,gen
					//      read.f,rmw.f
					mode[0].read_f(size_f);
					mode[1].rmw_f(size_f);
					decode(mode, bytes);

					if (slave(mode[0], mode[1]))
						interrupt(FPU, m_pc);
					break;
				case 0x9:
					// Trap(SLAVE)
					// operands from ns32532 datasheet
					mode[0].read_f(size_f);
					mode[1].write_f(size_f);
					decode(mode, bytes);

					if (slave(mode[0], mode[1]))
						interrupt(FPU, m_pc);
					break;
				case 0xc:
					// MULf src,dst
					//      gen,gen
					//      read.f,rmw.f
					mode[0].read_f(size_f);
					mode[1].rmw_f(size_f);
					decode(mode, bytes);

					if (slave(mode[0], mode[1]))
						interrupt(FPU, m_pc);
					break;
				case 0xd:
					// ABSf src,dst
					//      gen,gen
					//      read.f,write.f
					mode[0].read_f(size_f);
					mode[1].write_f(size_f);
					decode(mode, bytes);

					if (slave(mode[0], mode[1]))
						interrupt(FPU, m_pc);
					break;
				}
			}
			else
				interrupt(UND, m_pc);
			break;
		case 0xfe: // format 12
		case 0x9e: // format 13
			interrupt(UND, m_pc);
			break;
		case 0x1e:
			// format 14: xxxx xsss s0oo ooii 0001 1110
			if (!(m_psr & PSR_U))
			{
				if (m_cfg & CFG_M)
				{
					u16 const opword = space(0).read_word_unaligned(m_pc + bytes);
					bytes += 2;

					addr_mode mode[] = { addr_mode((opword >> 11) & 31), addr_mode(0x13) };

					//unsigned const quick = (opword >> 7) & 15;
					size_code const size = size_code(opword & 3);

					// TODO: mmu instructions
					switch ((opword >> 2) & 15)
					{
					case 0:
						// RDVAL loc
						//       gen
						//       addr
						mode[0].addr();
						decode(mode, bytes);

						tcy = 21;
						break;
					case 1:
						// WRVAL loc
						//       gen
						//       addr
						mode[0].addr();
						decode(mode, bytes);

						tcy = 21;
						break;
					case 2:
						// LMR mmureg,src
						//     short,gen
						//           read.D
						mode[0].read_i(size);
						decode(mode, bytes);

						tcy = 30;
						break;
					case 3:
						// SMR mmureg,dst
						//     short,gen
						//           write.D
						mode[0].write_i(size);
						decode(mode, bytes);

						tcy = 25;
						break;
					default:
						interrupt(UND, m_pc);
						break;
					}
				}
				else
					interrupt(UND, m_pc);
			}
			else
				interrupt(ILL, m_pc);
			break;
		case 0x16: // format 15.0
		case 0x36: // format 15.1
		case 0xb6: // format 15.5
			// TODO: custom coprocessor
			break;
		case 0x5e: // format 16
		case 0xde: // format 17
		case 0x8e: // format 18
		case 0x06:
		case 0x26:
		case 0x46:
		case 0x66:
		case 0x86:
		case 0xa6:
		case 0xc6:
		case 0xe6:
			// format 19
			interrupt(UND, m_pc);
			break;
		}

		if (m_sequential)
			m_pc += bytes;

		// trace trap
		if (m_psr & PSR_P)
			interrupt(TRC, m_pc);

		m_icount -= tcy;
	}
}

template <int Width> void ns32000_device<Width>::execute_set_input(int inputnum, int state)
{
	if (state)
		m_wait = false;

	switch (inputnum)
	{
	case INPUT_LINE_NMI:
		// NMI is edge triggered
		m_nmi_line = m_nmi_line || (state == ASSERT_LINE);
		break;

	case INPUT_LINE_IRQ0:
		// INT is level triggered
		m_int_line = state == ASSERT_LINE;
		break;
	}
}

template <int Width> device_memory_interface::space_config_vector ns32000_device<Width>::memory_space_config() const
{
	return space_config_vector{
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(4, &m_interrupt_config), // interrupt acknowledge, master
		std::make_pair(5, &m_interrupt_config), // interrupt acknowledge, cascaded
		std::make_pair(6, &m_interrupt_config), // end of interrupt, master
		std::make_pair(7, &m_interrupt_config), // end of interrupt, cascaded
		std::make_pair(10, &m_program_config), // data transfer
		std::make_pair(11, &m_program_config), // read read-modify-write operand
		std::make_pair(12, &m_program_config), // read for effective address
	};
}

template <int Width> bool ns32000_device<Width>::memory_translate(int spacenum, int intention, offs_t &address)
{
	return true;
}

template <int Width> std::unique_ptr<util::disasm_interface> ns32000_device<Width>::create_disassembler()
{
	return std::make_unique<ns32000_disassembler>();
}

template <int Width> u16 ns32000_device<Width>::slave(addr_mode op1, addr_mode op2)
{
	if ((op1.access == READ || op1.access == RMW) && !(op1.type == REG && op1.slave))
	{
		u64 const data = gen_read(op1);

		switch (op1.size)
		{
		case SIZE_B:
			m_fpu->write_op(u8(data));
			break;
		case SIZE_W:
			m_fpu->write_op(u16(data));
			break;
		case SIZE_D:
			m_fpu->write_op(u16(data >> 0));
			m_fpu->write_op(u16(data >> 16));
			break;
		case SIZE_Q:
			m_fpu->write_op(u16(data >> 0));
			m_fpu->write_op(u16(data >> 16));
			m_fpu->write_op(u16(data >> 32));
			m_fpu->write_op(u16(data >> 48));
			break;
		}
	}

	if ((op2.access == READ || op2.access == RMW) && !(op2.type == REG && op2.slave))
	{
		u64 const data = gen_read(op2);

		switch (op2.size)
		{
		case SIZE_B:
			m_fpu->write_op(u8(data));
			break;
		case SIZE_W:
			m_fpu->write_op(u16(data));
			break;
		case SIZE_D:
			m_fpu->write_op(u16(data >> 0));
			m_fpu->write_op(u16(data >> 16));
			break;
		case SIZE_Q:
			m_fpu->write_op(u16(data >> 0));
			m_fpu->write_op(u16(data >> 16));
			m_fpu->write_op(u16(data >> 32));
			m_fpu->write_op(u16(data >> 48));
			break;
		}
	}

	u16 const status = m_fpu->read_st(&m_icount);

	if (!(status & ns32000_slave_interface::SLAVE_Q))
	{
		if ((op2.access == WRITE || op2.access == RMW) && !(op2.type == REG && op2.slave))
		{
			u64 data = m_fpu->read_op();

			switch (op2.size)
			{
			case SIZE_D:
				data |= u64(m_fpu->read_op()) << 16;
				break;
			case SIZE_Q:
				data |= u64(m_fpu->read_op()) << 16;
				data |= u64(m_fpu->read_op()) << 32;
				data |= u64(m_fpu->read_op()) << 48;
				break;
			default:
				break;
			}

			gen_write(op2, data);
		}
	}
	else
	{
		// restore stack pointer
		if (op1.type == TOS && op1.access == READ)
			SP -= op1.size + 1;

		if (op2.type == TOS && op2.access == READ)
			SP -= op2.size + 1;
	}

	return status;
}
