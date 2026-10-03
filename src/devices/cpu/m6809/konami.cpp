// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/*********************************************************************

    Portable Konami cpu emulator
    Custom HD6309 in a gate array with ROM blocks for instruction decoding.

    Based on M6809 cpu core copyright John Butler

    TODO:
    - measure EA timings for memory-operand instructions beyond the measured
      extended (postbyte 07) forms; TST (92) and selected LEA/PC forms have
      separate measured coverage. PC predecrement and plain indirect PC are
      also unmeasured.

    References:

        6809 Simulator V09, By L.C. Benschop, Eindhoven The Netherlands.

        m6809: Portable 6809 emulator, DS (6809 code in MAME, derived from
            the 6809 Simulator V09)

        6809 Microcomputer Programming & Interfacing with Experiments"
            by Andrew C. Staugaard, Jr.; Howard W. Sams & Co., Inc.

    System dependencies:    uint16_t must be 16 bit unsigned int
                            uint8_t must be 8 bit unsigned int
                            uint32_t must be more than 16 bits
                            arrays up to 65536 bytes must be supported
                            machine must be twos complement

    History:

October 2026 Jose Tejada (aka JOTEGO):
    Added measured instruction timings, documented Konami CPU instructions,
    and measured condition-code behavior.

March 2013 NPW:
    Rewrite of 6809/6309/Konami CPU; overall core is now unified and
    supports mid-instruction timings.

    Some of the instruction timings have changed with the new core; the
    old core had some nonsensical timings.  For example (from scontra):

        819A    3A 07 1F 8C        STA $1f8C

    Under the old core, this took four clock cycles, which is dubious
    because this instruction would have to do four opcode reads and one
    write.  OGalibert says that the current timings are just a guess and
    nobody has done precise readings, so I'm replacing the old guesses
    with new guesses.

991022 HJB:
    Tried to improve speed: Using bit7 of cycles1 as flag for multi
    byte opcodes is gone, those opcodes now instead go through opcode2().
    Inlined fetch_effective_address() into that function as well.
    Got rid of the slow/fast flags for stack (S and U) memory accesses.
    Minor changes to use 32 bit values as arguments to memory functions
    and added defines for that purpose (e.g. X = 16bit XD = 32bit).

990720 EHC:
    Created this file

*****************************************************************************/

#include "emu.h"
#include "konami.h"
#include "m6809inl.h"
#include "6x09dasm.h"


//**************************************************************************
//  PARAMETERS
//**************************************************************************

// turn off 'unreferenced label' errors
#if defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wunused-label"
#endif
#ifdef _MSC_VER
#pragma warning( disable : 4102 )
#endif


//**************************************************************************
//  DEVICE INTERFACE
//**************************************************************************

DEFINE_DEVICE_TYPE(KONAMI, konami_cpu_device, "konami_cpu", "KONAMI CPU")


//-------------------------------------------------
//  konami_cpu_device - constructor
//-------------------------------------------------

// The Konami part is clocked at its measured input rate.  Its instruction
// timings are accounted in those raw clock periods, unlike the E-cycle based
// M6809/HD6309 devices in this file.

konami_cpu_device::konami_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	m6809_base_device(mconfig, tag, owner, clock, KONAMI, 1),
	m_set_lines(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void konami_cpu_device::device_start()
{
	super::device_start();

	// initialize variables
	m_temp_im = 0;
	m_bcount = 0;
	m_dp_exgtfr_low = 0;
	m_pc_transfer_opcode = 0;
	m_pc_transfer_opcode_valid = false;
	m_timing_active = false;
	m_timing_opcode = 0;
	m_timing_postbyte = 0;
	m_timing_operand1 = 0;
	m_timing_cc = 0;
	m_timing_a = 0;
	m_timing_b = 0;
	m_timing_u = 0;
	m_timing_x = 0;
	m_timing_elapsed = 0;

	// setup regtable
	save_item(NAME(m_temp_im));
	save_item(NAME(m_bcount));
	save_item(NAME(m_dp_exgtfr_low));
	save_item(NAME(m_pc_transfer_opcode));
	save_item(NAME(m_pc_transfer_opcode_valid));
	save_item(NAME(m_timing_active));
	save_item(NAME(m_timing_opcode));
	save_item(NAME(m_timing_postbyte));
	save_item(NAME(m_timing_operand1));
	save_item(NAME(m_timing_cc));
	save_item(NAME(m_timing_a));
	save_item(NAME(m_timing_b));
	save_item(NAME(m_timing_u));
	save_item(NAME(m_timing_x));
	save_item(NAME(m_timing_elapsed));
}


//-------------------------------------------------
//  device_reset
//-------------------------------------------------

void konami_cpu_device::device_reset()
{
	super::device_reset();
	m_dp_exgtfr_low = 0;
	m_pc_transfer_opcode_valid = false;
	m_timing_active = false;
	m_timing_elapsed = 0;
}


//-------------------------------------------------
//  disassemble - call the disassembly
//  helper function
//-------------------------------------------------

std::unique_ptr<util::disasm_interface> konami_cpu_device::create_disassembler()
{
	return std::make_unique<konami_disassembler>();
}


//-------------------------------------------------
//  read_operand
//-------------------------------------------------

inline uint8_t konami_cpu_device::read_operand()
{
	return super::read_operand();
}


//-------------------------------------------------
//  read_operand
//-------------------------------------------------

inline uint8_t konami_cpu_device::read_operand(int ordinal)
{
	switch(m_addressing_mode)
	{
		case ADDRESSING_MODE_EA:            return read_memory(m_ea.w + ordinal);
		case ADDRESSING_MODE_IMMEDIATE:     return read_opcode_arg();
		case ADDRESSING_MODE_REGISTER_D:    return (ordinal & 1) ? m_q.r.b : m_q.r.a;
		default:                            fatalerror("Unexpected");
	}
}


//-------------------------------------------------
//  write_operand
//-------------------------------------------------

inline void konami_cpu_device::write_operand(uint8_t data)
{
	super::write_operand(data);
}



//-------------------------------------------------
//  write_operand
//-------------------------------------------------

inline void konami_cpu_device::write_operand(int ordinal, uint8_t data)
{
	switch(m_addressing_mode)
	{
		case ADDRESSING_MODE_IMMEDIATE:     /* do nothing */                                break;
		case ADDRESSING_MODE_EA:            write_memory(m_ea.w + ordinal, data);           break;
		case ADDRESSING_MODE_REGISTER_D:    *((ordinal & 1) ? &m_q.r.b : &m_q.r.a) = data;  break;
		default:                            fatalerror("Unexpected");
	}
}


//-------------------------------------------------
//  ireg
//-------------------------------------------------

inline uint16_t &konami_cpu_device::ireg()
{
	switch(m_opcode & 0x70)
	{
		case 0x20: return m_x.w;  // X
		case 0x30: return m_y.w;  // Y
		case 0x50: return m_u.w;  // U
		case 0x60: return m_s.w;  // S
		case 0x70: return m_pc.w; // PC

		default:
			fatalerror("Should not get here");
			// never executed
	}
}


//-------------------------------------------------
//  read_exgtfr_register
//-------------------------------------------------

inline uint16_t konami_cpu_device::read_exgtfr_register(uint8_t reg)
{
	uint16_t result = 0;

	switch(reg & 0x07)
	{
		case 0: result = m_q.r.a | 0x1000; break; // A (high byte is always 0x10)
		case 1: result = m_q.r.d;   break; // D!
		case 2: result = m_x.w;     break; // X
		case 3: result = m_y.w;     break; // Y
		case 4: result = (uint16_t(m_dp) << 8) | m_dp_exgtfr_low; break; // DP-related word
		case 5: result = m_u.w;     break; // U
		case 6: result = m_s.w;     break; // S
		case 7: result = m_pc.w;    break; // PC
	}

	return result;
}


//-------------------------------------------------
//  write_exgtfr_register
//-------------------------------------------------

inline void konami_cpu_device::write_exgtfr_register(uint8_t reg, uint16_t value)
{
	switch(reg & 0x07)
	{
		case 0: m_q.r.a = value;    break; // A
		case 1: m_q.r.b = value;    break; // B
		case 2: m_x.w   = value;    break; // X
		case 3: m_y.w   = value;    break; // Y
		case 4:
			m_dp = value >> 8;
			m_dp_exgtfr_low = value;
			break; // DP-related word; stack operations only update visible DP
		case 5: m_u.w   = value;    break; // U
		case 6: m_s.w   = value;    break; // S
		case 7: m_pc.w  = value;    break; // PC
	}
}


//-------------------------------------------------
//  read_konami_opcode
//-------------------------------------------------

inline uint8_t konami_cpu_device::read_konami_opcode()
{
	if (!m_pc_transfer_opcode_valid)
		return read_opcode();

	m_pc_transfer_opcode_valid = false;
	m_pc.w++;
	eat(1);
	return m_pc_transfer_opcode;
}


//-------------------------------------------------
//  lmul
//-------------------------------------------------

inline void konami_cpu_device::lmul()
{
	PAIR result;

	// do the multiply
	result.d = (uint32_t)m_x.w * m_y.w;

	// set the result registers
	m_x.w = result.w.h;
	m_y.w = result.w.l;

	// set Z flag
	set_flags<uint32_t>(CC_Z, result.d);

}


//-------------------------------------------------
//  divx
//-------------------------------------------------

inline void konami_cpu_device::divx()
{
	uint16_t result;
	uint8_t remainder;

	if (m_q.r.b != 0)
	{
		result = m_x.w / m_q.r.b;
		remainder = m_x.w % m_q.r.b;
	}
	else
	{
		// The measured zero-divisor result saturates the quotient and keeps
		// the original low byte in B.
		result = 0xffff;
		remainder = m_x.b.l;
	}

	// N and Z describe the quotient; all other flags are preserved.
	m_x.w = set_flags<uint16_t>(CC_NZ, result);
	m_q.r.b = remainder;
}


//-------------------------------------------------
//  daa_konami
//-------------------------------------------------

inline void konami_cpu_device::daa_konami()
{
	uint8_t const old_a = m_q.r.a;
	uint8_t const low = ((m_cc & CC_H) || ((old_a & 0x0f) > 9)) ? 0x06 : 0;
	uint8_t const high = ((m_cc & CC_C) || (old_a > 0x99)) ? 0x60 : 0;
	uint16_t const sum = old_a + low + high;
	uint8_t const adjusted = sum;
	bool const carry = (sum > 0xff);
	uint8_t const old_carry = m_cc & CC_C;

	m_q.r.a = adjusted;
	m_cc &= ~(CC_H | CC_N | CC_Z | CC_V | CC_C);
	if (((old_a & 0x0f) + low) > 0x0f)
		m_cc |= CC_H;
	if (high == 0)
	{
		if (adjusted & 0x80)
			m_cc |= CC_N;
		if (adjusted == 0)
			m_cc |= CC_Z;
	}
	if (old_carry ^ (carry ? CC_C : 0))
		m_cc |= CC_C;
}


//-------------------------------------------------
//  set_lines
//-------------------------------------------------

void konami_cpu_device::set_lines(uint8_t data)
{
	m_set_lines(offs_t(0), data);
}


//-------------------------------------------------
//  execute_one - try to execute a single instruction
//-------------------------------------------------

inline void konami_cpu_device::execute_one()
{
	timing_scope timing{ *this };
	switch(pop_state())
	{
#include "cpu/m6809/konami.hxx"
	}
}


//-------------------------------------------------
//  begin_instruction_timing
//-------------------------------------------------

void konami_cpu_device::begin_instruction_timing(uint8_t opcode)
{
	m_timing_opcode = opcode;
	m_timing_cc = m_cc;
	m_timing_a = m_q.r.a;
	m_timing_b = m_q.r.b;
	m_timing_u = m_u.w;
	m_timing_x = m_x.w;
	m_timing_postbyte = m_mintf->read_opcode_arg(m_pc.w);
	m_timing_operand1 = m_mintf->read_opcode_arg(m_pc.w + 1);
	m_timing_active = true;
}


//-------------------------------------------------
//  finish_instruction_timing
//-------------------------------------------------

void konami_cpu_device::finish_instruction_timing(int cycle_count)
{
	// Generated microcode can suspend in the middle of an instruction when a
	// timeslice expires.  Keep the initial cycle count until its state stack is
	// empty, then add only the measured internal cycles that the bus activity
	// and existing micro-operations did not account for.
	if (!m_timing_active)
		return;

	m_timing_elapsed += cycle_count - m_icount;
	if (execution_state_pending())
		return;

	int const target = documented_instruction_cycles();
	if (target >= 0)
	{
		if (m_timing_elapsed < target)
			eat(target - m_timing_elapsed);
	}

	m_timing_active = false;
	m_timing_elapsed = 0;
}


//-------------------------------------------------
//  documented_instruction_cycles
//-------------------------------------------------

int konami_cpu_device::documented_instruction_cycles() const
{
	uint8_t const op = m_timing_opcode;
	uint8_t const pb = m_timing_postbyte;
	uint8_t const cc = m_timing_cc;

	// The measured effective-address rows are exact for TST.  Other EA
	// instructions are timed here only for the documented extended form.
	if (op == 0x92)
	{
		uint8_t const base_register = pb & 0xf0;
		if (base_register == 0x20 || base_register == 0x30 || base_register == 0x50 || base_register == 0x60)
		{
			int clocks = -1;
			switch (pb & 0x07)
			{
			case 0x0: case 0x2: case 0x4: case 0x6: clocks = 16; break;
			case 0x1: case 0x3: clocks = 17; break;
			case 0x5: clocks = 22; break;
			default: break;
			}
			if (clocks >= 0)
				return clocks + ((pb & 0x08) ? 7 : 0);
		}
		switch (pb)
		{
		case 0x07: return 21;
		case 0xc4: return 19;
		case 0xcc: return 26;
		case 0x0f: return 28;
		case 0xa0: case 0xb0: case 0xe0: return 18;
		case 0xd0: return 20;
		case 0xa8: case 0xb8: case 0xe8: return 25;
		case 0xd8: return 27;
		case 0xe7: return 20;
		case 0xef: return 27;
		case 0x76: case 0x70: return 16;
		case 0x71: return 17;
		case 0x78: return 23;
		case 0x79: return 24;
		case 0x74: return 21;
		case 0x75: return 22;
		case 0x7c: return 28;
		case 0x7d: return 29;
		case 0xf0: case 0xf1: return 18;
		case 0xf7: return 20;
		case 0xf8: case 0xf9: return 25;
		case 0xff: return 27;
		default: return -1;
		}
	}

	// Inherent and immediate instructions with measured fixed timings.
	switch (op)
	{
	case 0x0c: case 0x0d:
		{
			unsigned const mask = pb;
			unsigned const bytes = unsigned(BIT(mask, 0)) + unsigned(BIT(mask, 1)) + unsigned(BIT(mask, 2)) + unsigned(BIT(mask, 3));
			unsigned const words = unsigned(BIT(mask, 4)) + unsigned(BIT(mask, 5)) + unsigned(BIT(mask, 6)) + unsigned(BIT(mask, 7));
			return 21 + 3 * bytes + 7 * words;
		}
	case 0x0e: case 0x0f:
		{
			unsigned const mask = pb;
			unsigned const bytes = unsigned(BIT(mask, 0)) + unsigned(BIT(mask, 1)) + unsigned(BIT(mask, 2)) + unsigned(BIT(mask, 3));
			unsigned const words = unsigned(BIT(mask, 4)) + unsigned(BIT(mask, 5)) + unsigned(BIT(mask, 6));
			return 20 + 4 * bytes + 8 * words + (BIT(mask, 7) ? 7 : 0);
		}
	case 0x14: case 0x15: case 0x18: case 0x19: case 0x1c: case 0x1d:
	case 0x20: case 0x21: case 0x34: case 0x35:
		return 8;
	case 0x10: case 0x11: case 0x24: case 0x25: case 0x28: case 0x29:
	case 0x2c: case 0x2d: case 0x30: case 0x31: case 0x38: case 0x3c: case 0x3d:
		return 9;
	case 0x3e: case 0x3f:
		{
			unsigned const destination = (pb >> 4) & 7;
			unsigned const source = pb & 7;
			if (!BIT(pb, 7))
			{
				if (destination <= 5 && source <= 5)
					return 8;
				if (op == 0x3e && pb == 0x26)
					return 8;
			}
			else if (pb == 0xa3 || (op == 0x3e && (pb == 0xa6 || pb == 0xa7)))
				return 8;
			return -1;
		}
	case 0x40: case 0x42: case 0x44: case 0x46: case 0x48: case 0x4c: case 0x4e: case 0x50: case 0x52:
		return 12;
	case 0x4a: case 0x54: case 0x56:
		return 13;
	case 0x60: return 11;
	case 0x68: return 13;
	case 0x70: return 6;
	case 0x78: return 7;
	case 0x02: case 0x04: case 0x5d: return 65;
	case 0x03: return 29;
	case 0x05: return pb == 0x07 ? 26 : pb == 0xc4 ? 24 : pb == 0x26 ? 21 : -1;
	case 0x06: return pb == 0x07 ? 24 : pb == 0xc4 ? 22 : pb == 0x26 ? 19 : -1;
	case 0x07: return pb == 0x07 ? 22 : pb == 0xc4 ? 20 : pb == 0x26 ? 17 : -1;
	case 0x08: case 0x09: case 0x0a: case 0x0b: return pb == 0x07 ? 20 : -1;
	case 0xaa: return 19;
	case 0xab: return 21;
	case 0xac: return m_timing_postbyte == 0 ? (m_timing_b == 1 ? 9 : 13) : -1;
	case 0xad: return m_timing_postbyte == 0 ? (m_timing_x == 1 ? 9 : 13) : -1;
	case 0x80: case 0x81: case 0x83: case 0x84: case 0x89: case 0x8a: case 0x8c: case 0x8d:
	case 0x90: case 0x91: case 0x93: case 0x94: case 0x96: case 0x97: case 0x99: case 0x9a:
	case 0x9c: case 0x9d: case 0xa0: case 0xa1: case 0xae: case 0xb0: case 0xb2: case 0xc2:
		return 4;
	case 0x86: case 0x87: case 0xca: case 0xcc: case 0xcd:
		return 5;
	case 0xc4: return 7;
	case 0xc6: case 0xc8: return 6;
	case 0xce: return 8;
	case 0xb1:
		{
			if (m_timing_a != 0x00 && m_timing_a != 0x0a && m_timing_a != 0xa0 && m_timing_a != 0xff)
				return -1;
			unsigned const low = (m_timing_a & 0x0f) > 9;
			unsigned const high = m_timing_a > 0x99;
			unsigned const low_cost = BIT(cc, 5) ? 3 : low;
			unsigned const high_cost = BIT(cc, 0) ? 3 : high;
			return 14 - low_cost - high_cost;
		}
	case 0xb3: return 16;
	case 0xb4: return 23;
	case 0xb5:
		return m_timing_b ? 25 + (m_x.w & 1) : 26 - BIT(m_timing_x, 15);
	case 0xb6: return m_timing_u ? 7 + 8 * m_timing_u : -1;
	case 0xb7: return 14;
	case 0xcf: return m_timing_u ? 7 + 4 * m_timing_u : -1;
	case 0xd0: return m_timing_u ? 7 + 8 * m_timing_u : -1;
	case 0x8f: return 14;
	case 0x9f: return BIT(m_cc, 7) ? 63 : 21;
	case 0x39: return pb == 0x07 ? 22 : -1;
	case 0x3a: case 0x3b: return pb == 0x07 ? 22 : -1;
	case 0xa8: return pb == 0x07 ? 17 : -1;
	case 0xa9: return pb == 0x07 ? 27 : -1;
	case 0x82: return pb == 0x07 ? 22 : -1;
	case 0x85: case 0x88: case 0x8b: case 0x8e: case 0x95: case 0x98: case 0x9b: case 0x9e:
		return pb == 0x07 ? (op == 0x82 ? 22 : (op == 0x95 || op == 0x98 || op == 0x9b || op == 0x9e ? 25 : 25)) : -1;
	case 0xa2: return pb == 0x07 ? 26 : -1;
	case 0xa3: case 0xa4: case 0xa5: return pb == 0x07 ? 35 : -1;
	case 0xa6: return pb == 0x07 ? 34 : -1;
	case 0xa7: return pb == 0x07 ? 33 : -1;
	case 0xc3: return pb == 0x07 ? 26 : -1;
	case 0xc5: case 0xc7: case 0xc9:
		return pb == 0x07 ? 33 : -1;
	case 0xcb: return pb == 0x07 ? 25 : -1;
	case 0xb8: case 0xba: case 0xbc: case 0xbe: case 0xc0:
		{
			unsigned const n = m_timing_postbyte & 0x0f;
			if (op == 0xb8 || op == 0xba) return 15 + n;
			if (op == 0xbc) return 17 + n;
			return 30 - n;
		}
	case 0xb9: case 0xbb: case 0xbd: case 0xbf: case 0xc1:
		{
			if (pb != 0x07) return -1;
			unsigned const n = m_timing_a & 0x0f;
			if (op == 0xb9 || op == 0xbb) return 40 + n;
			if (op == 0xbd) return 42 + n;
			return 55 - n;
		}
	case 0xd1: return pb == 0x07 ? 22 : pb == 0xc4 ? 20 : pb == 0x26 ? 17 : -1;
	default: break;
	}

	// Single-byte extended-addressed arithmetic, logic, loads, compares and
	// stores.  The reference explicitly limits these EA counts to postbyte 07.
	if (pb == 0x07)
	{
		if (op >= 0x12 && op <= 0x23) return 21;
		if ((op >= 0x26 && op <= 0x2f) || (op >= 0x32 && op <= 0x33)) return 22;
		if (op >= 0x36 && op <= 0x3b) return (op == 0x36 || op == 0x37) ? 21 : 22;
		if (op >= 0x41 && op <= 0x53) return (op & 1) ? 25 : -1;
		if (op == 0x55 || op == 0x57) return 25;
		if (op >= 0x58 && op <= 0x5c) return 26;
	}

	// Conditional relative branches have measured false/taken timings.
	if ((op >= 0x61 && op <= 0x67) || (op >= 0x71 && op <= 0x77) || (op >= 0x69 && op <= 0x6f) || (op >= 0x79 && op <= 0x7f))
	{
		if ((op & 0x08) ? (m_timing_postbyte || m_timing_operand1) : m_timing_postbyte)
			return -1;
		bool taken = false;
		bool const c = BIT(cc, 0), v = BIT(cc, 1), z = BIT(cc, 2), n = BIT(cc, 3);
		unsigned const condition = op & 0x07;
		switch (condition)
		{
		case 1: taken = !c && !z; break;
		case 2: taken = !c; break;
		case 3: taken = !z; break;
		case 4: taken = !v; break;
		case 5: taken = !n; break;
		case 6: taken = n == v; break;
		case 7: taken = !z && (n == v); break;
		}
		bool const long_branch = op & 0x08;
		if (op & 0x10)
			taken = !taken;
		if (long_branch)
		{
			if (op >= 0x79)
				return taken ? (op == 0x79 ? 13 : op == 0x7e ? 14 : op == 0x7f ? 16 : 13) : (op == 0x79 ? 10 : op == 0x7e ? 10 : op == 0x7f ? 12 : 8);
			return taken ? (op == 0x69 ? 15 : op == 0x6e ? 15 : op == 0x6f ? 17 : 13) : (op == 0x69 ? 7 : op == 0x6e ? 9 : op == 0x6f ? 11 : 7);
		}
		if (op >= 0x71)
			return taken ? (op == 0x71 ? 11 : op == 0x76 ? 12 : op == 0x77 ? 14 : 11) : (op == 0x71 ? 9 : op == 0x76 ? 9 : op == 0x77 ? 11 : 7);
		return taken ? (op == 0x61 ? 13 : op == 0x66 ? 13 : op == 0x67 ? 15 : 11) : (op == 0x61 ? 7 : op == 0x66 ? 9 : op == 0x67 ? 11 : 7);
	}

	return -1;
}


//-------------------------------------------------
//  execute_run - execute a timeslice's worth of
//  opcodes
//-------------------------------------------------

void konami_cpu_device::execute_run()
{
	do
	{
		execute_one();
	} while(m_icount > 0);
}
