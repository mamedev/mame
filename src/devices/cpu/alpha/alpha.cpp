// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * An implementation of the Digital Alpha CPU family.
 *
 * Sources:
 *
 *   http://bitsavers.org/pdf/dec/alpha/21064-aa-RISC%20Microprocessor%20Preliminary%20Data%20Sheet-apr92.pdf
 *   http://bitsavers.org/pdf/dec/alpha/Sites_AlphaAXPArchitectureReferenceManual_2ed_1995.pdf
 *   https://sourceware.org/git/gitweb.cgi?p=binutils-gdb.git;a=blob_plain;f=opcodes/alpha-opc.c;hb=HEAD
 *   http://ftp.twaren.net/NetBSD/misc/dec-docs/
 *
 * TODO
 *   - interrupts and exceptions
 *   - address translation
 *   - ibox/abox registers
 *   - floating point instructions
 *   - primary caches
 *   - later cpu implementations
 *   - instruction set extensions
 *   - big-endian mode
 */

#include "emu.h"
#include "alpha.h"
#include "common.h"

#include "debugger.h"

#include "softfloat3/source/include/softfloat.h"

#define LOG_GENERAL   (1U << 0)
#define LOG_EXCEPTION (1U << 1)
#define LOG_SYSCALLS  (1U << 2)

//#define VERBOSE (LOG_GENERAL | LOG_EXCEPTION)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(DEC_21064, dec_21064_device, "21064", "DEC Alpha 21064")

dec_21064_device::dec_21064_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: alpha_ev4_device(mconfig, DEC_21064, tag, owner, clock)
{
}

alpha_ev4_device::alpha_ev4_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: alpha_device(mconfig, type, tag, owner, clock)
{
}

alpha_device::alpha_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_dasm_type(alpha_disassembler::dasm_type::TYPE_UNKNOWN)
	, m_as_config
		{
			address_space_config("0", ENDIANNESS_LITTLE, 64, 32, 0),
			address_space_config("1", ENDIANNESS_LITTLE, 64, 32, 0),
			address_space_config("2", ENDIANNESS_LITTLE, 64, 32, 0),
			address_space_config("3", ENDIANNESS_LITTLE, 64, 32, 0)
		}
	, m_srom_oe_cb(*this)
	, m_srom_data_cb(*this)
	, m_icount(0)
{
}

void alpha_device::device_start()
{
	set_icountptr(m_icount);

	save_item(NAME(m_pc));
	save_item(NAME(m_r));
	save_item(NAME(m_f));

	save_item(NAME(m_pal_mode));

	state_add(STATE_GENPC, "GENPC", m_pc).noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_pc).noshow();

	state_add(64, "PC", m_pc);

	// integer registers
	for (unsigned i = 0; i < 32; i++)
		state_add(i, util::string_format("R%d", i).c_str(), m_r[i]);

	// floating point registers
	for (unsigned i = 0; i < 32; i++)
		state_add(i + 32, util::string_format("F%d", i).c_str(), m_f[i]);

	m_srom_oe_cb.resolve_safe();
	m_srom_data_cb.resolve_safe(0);
}

void alpha_device::device_reset()
{
	m_pc = 0;
	m_pal_mode = true;
}

void alpha_device::execute_run()
{
	while (m_icount > 0)
	{
		debugger_instruction_hook(m_pc);

		fetch(m_pc,
			[this](u32 const op)
			{
				// update the program counter
				m_pc += 4;

				// execute an instruction
				cpu_execute(op);

				// reset always-zero registers
				m_r[31] = 0;
				m_f[31] = 0;
			});

		m_icount--;
	}
}

void alpha_device::execute_set_input(int inputnum, int state)
{
}

device_memory_interface::space_config_vector alpha_device::memory_space_config() const
{
	/*
	 * EV4 devices have a 34-bit physical address space. This is mapped using
	 * the top two bits to select one of four memory spaces with the other 32
	 * bits giving the offset within each space. This approach works out quite
	 * well for the jensen hardware, which uses the first space for memory, and
	 * the others for a variety of I/O memory mapping.
	 *
	 * Note: space numbers are multiplied by two to avoid the special handling
	 * applied to the decrypted opcode space (number 3).
	 */
	return space_config_vector {
		std::make_pair(0, &m_as_config[0]),
		std::make_pair(2, &m_as_config[1]),
		std::make_pair(4, &m_as_config[2]),
		std::make_pair(6, &m_as_config[3])
	};
}

bool alpha_device::memory_translate(int spacenum, int intention, offs_t &address)
{
	u64 placeholder = s64(s32(address));

	if (cpu_translate(placeholder, intention))
	{
		address = placeholder;

		return true;
	}

	return false;
}

std::unique_ptr<util::disasm_interface> alpha_device::create_disassembler()
{
	return std::make_unique<alpha_disassembler>(m_dasm_type);
}

void alpha_device::cpu_execute(u32 const op)
{
	switch ((op >> 26) & 0x3f)
	{
	case 0x08: m_r[Ra(op)] = m_r[Rb(op)] + Disp_M(op); break; // lda
	case 0x09: m_r[Ra(op)] = m_r[Rb(op)] + (Disp_M(op) << 16); break; // ldah
	case 0x0b: load<u64>((m_r[Rb(op)] + Disp_M(op)) & ~7, [this, op](u64 data) { m_r[Ra(op)] = data; }); break; // ldq_u
	case 0x0f: store<u64>((m_r[Rb(op)] + Disp_M(op)) & ~7, m_r[Ra(op)]); break; // stq_u

	case 0x10: // INTA* (integer arithmetic)
		switch ((op >> 5) & 0xff)
		{
			// register variants
		case 0x00: m_r[Rc(op)] = s64(s32(m_r[Ra(op)]) + s32(m_r[Rb(op)])); break; // addl
		case 0x02: m_r[Rc(op)] = s64(s32(m_r[Ra(op)] << 2) + s32(m_r[Rb(op)])); break; // s4addl
		case 0x09: m_r[Rc(op)] = s64(s32(m_r[Ra(op)]) - s32(m_r[Rb(op)])); break; // subl
		case 0x0b: m_r[Rc(op)] = s64(s32(m_r[Ra(op)] << 2) - s32(m_r[Rb(op)])); break; // s4subl
		case 0x0f: // cmpbge
			{
				u8 temp = 0;
				for (unsigned i = 0; i < 8; i++)
					if (u8(m_r[Ra(op)] >> (i * 8)) >= u8(m_r[Rb(op)] >> (i * 8)))
						temp |= (1U << i);

				m_r[Rc(op)] = u64(temp);
			}
			break;
		case 0x12: m_r[Rc(op)] = s64(s32(m_r[Ra(op)] << 3) + s32(m_r[Rb(op)])); break; // s8addl
		case 0x1b: m_r[Rc(op)] = s64(s32(m_r[Ra(op)] << 3) - s32(m_r[Rb(op)])); break; // s8subl
		case 0x1d: m_r[Rc(op)] = m_r[Ra(op)] < m_r[Rb(op)]; break; // cmpult
		case 0x20: m_r[Rc(op)] = m_r[Ra(op)] + m_r[Rb(op)]; break; // addq
		case 0x22: m_r[Rc(op)] = (m_r[Ra(op)] << 2) + m_r[Rb(op)]; break; // s4addq
		case 0x29: m_r[Rc(op)] = m_r[Ra(op)] - m_r[Rb(op)]; break; // subq
		case 0x2b: m_r[Rc(op)] = (m_r[Ra(op)] << 2) - m_r[Rb(op)]; break; // s4subq
		case 0x2d: m_r[Rc(op)] = m_r[Ra(op)] == m_r[Rb(op)]; break; // cmpeq
		case 0x32: m_r[Rc(op)] = (m_r[Ra(op)] << 3) + m_r[Rb(op)]; break; // s8addq
		case 0x3b: m_r[Rc(op)] = (m_r[Ra(op)] << 3) - m_r[Rb(op)]; break; // s8subq
		case 0x3d: m_r[Rc(op)] = m_r[Ra(op)] <= m_r[Rb(op)]; break; // cmpule
		case 0x40: m_r[Rc(op)] = s64(s32(m_r[Ra(op)]) + s32(m_r[Rb(op)])); break; // addl/v
		case 0x49: m_r[Rc(op)] = s64(s32(m_r[Ra(op)]) - s32(m_r[Rb(op)])); break; // subl/v
		case 0x4d: m_r[Rc(op)] = s64(m_r[Ra(op)]) < s64(m_r[Rb(op)]); break; // cmplt
		case 0x60: m_r[Rc(op)] = m_r[Ra(op)] + m_r[Rb(op)]; break; // addq/v
		case 0x69: m_r[Rc(op)] = m_r[Ra(op)] - m_r[Rb(op)]; break; // subq/v
		case 0x6d: m_r[Rc(op)] = s64(m_r[Ra(op)]) <= s64(m_r[Rb(op)]); break; // cmple

			// immediate variants
		case 0x80: m_r[Rc(op)] = s64(s32(m_r[Ra(op)]) + s32(Im(op))); break; // addl
		case 0x82: m_r[Rc(op)] = s64(s32(m_r[Ra(op)] << 2) + s32(Im(op))); break; // s4addl
		case 0x89: m_r[Rc(op)] = s64(s32(m_r[Ra(op)]) - s32(Im(op))); break; // subl
		case 0x8b: m_r[Rc(op)] = s64(s32(m_r[Ra(op)] << 2) - s32(Im(op))); break; // s4subl
		case 0x8f: // cmpbge
			{
				u8 temp = 0;
				for (unsigned i = 0; i < 8; i++)
					if (u8(m_r[Ra(op)] >> (i * 8)) >= u8(Im(op)))
						temp |= (1U << i);

				m_r[Rc(op)] = u64(temp);
			}
			break;
		case 0x92: m_r[Rc(op)] = s64(s32(m_r[Ra(op)] << 3) + s32(Im(op))); break; // s4addl
		case 0x9b: m_r[Rc(op)] = s64(s32(m_r[Ra(op)] << 3) - s32(Im(op))); break; // s8subl
		case 0x9d: m_r[Rc(op)] = m_r[Ra(op)] < Im(op); break; // cmpult
		case 0xa0: m_r[Rc(op)] = m_r[Ra(op)] + Im(op); break; // addq
		case 0xa2: m_r[Rc(op)] = (m_r[Ra(op)] << 2) + Im(op); break; // s4addq
		case 0xa9: m_r[Rc(op)] = m_r[Ra(op)] - Im(op); break; // subq
		case 0xab: m_r[Rc(op)] = (m_r[Ra(op)] << 2) - Im(op); break; // s4subq
		case 0xad: m_r[Rc(op)] = m_r[Ra(op)] == Im(op); break; // cmpeq
		case 0xb2: m_r[Rc(op)] = (m_r[Ra(op)] << 3) + Im(op); break; // s8addq
		case 0xbb: m_r[Rc(op)] = (m_r[Ra(op)] << 3) - Im(op); break; // s8subq
		case 0xbd: m_r[Rc(op)] = m_r[Ra(op)] <= Im(op); break; // cmpule
		case 0xc0: m_r[Rc(op)] = s64(s32(m_r[Ra(op)]) + s32(Im(op))); break; // addl/v
		case 0xc9: m_r[Rc(op)] = s64(s32(m_r[Ra(op)]) - s32(Im(op))); break; // subl/v
		case 0xcd: m_r[Rc(op)] = s64(m_r[Ra(op)]) < s64(Im(op)); break; // cmplt
		case 0xe0: m_r[Rc(op)] = m_r[Ra(op)] + Im(op); break; // addq/v
		case 0xe9: m_r[Rc(op)] = m_r[Ra(op)] - Im(op); break; // subq/v
		case 0xed: m_r[Rc(op)] = s64(m_r[Ra(op)]) <= s64(Im(op)); break; // cmple
		}
		break;
	case 0x11: // INTL* (integer logical)
		switch ((op >> 5) & 0xff)
		{
			// register variants
		case 0x00: m_r[Rc(op)] = m_r[Ra(op)] &  m_r[Rb(op)]; break; // and
		case 0x08: m_r[Rc(op)] = m_r[Ra(op)] & ~m_r[Rb(op)]; break; // bic
		case 0x14: // cmovlbs
			if (BIT(m_r[Ra(op)], 0))
				m_r[Rc(op)] = m_r[Rb(op)];
			break;
		case 0x16: // cmovlbc
			if (!BIT(m_r[Ra(op)], 0))
				m_r[Rc(op)] = m_r[Rb(op)];
			break;
		case 0x20: m_r[Rc(op)] = m_r[Ra(op)] |  m_r[Rb(op)]; break; // bis
		case 0x24: // cmoveq
			if (m_r[Ra(op)] == 0)
				m_r[Rc(op)] = m_r[Rb(op)];
			break;
		case 0x26: // cmovne
			if (m_r[Ra(op)] != 0)
				m_r[Rc(op)] = m_r[Rb(op)];
			break;
		case 0x28: m_r[Rc(op)] = m_r[Ra(op)] | ~m_r[Rb(op)]; break; // ornot
		case 0x40: m_r[Rc(op)] = m_r[Ra(op)] ^  m_r[Rb(op)]; break; // xor
		case 0x44: // cmovlt
			if (s64(m_r[Ra(op)]) < 0)
				m_r[Rc(op)] = m_r[Rb(op)];
			break;
		case 0x46: // cmovge
			if (s64(m_r[Ra(op)]) >= 0)
				m_r[Rc(op)] = m_r[Rb(op)];
			break;
		case 0x48: m_r[Rc(op)] = m_r[Ra(op)] ^ ~m_r[Rb(op)]; break; // eqv
		case 0x61: m_r[Rc(op)] = m_r[Rb(op)]; break; // amask
		case 0x64: // cmovle
			if (s64(m_r[Ra(op)]) <= 0)
				m_r[Rc(op)] = m_r[Rb(op)];
			break;
		case 0x66: // cmovgt
			if (s64(m_r[Ra(op)]) > 0)
				m_r[Rc(op)] = m_r[Rb(op)];
			break;

			// immediate variants
		case 0x80: m_r[Rc(op)] = m_r[Ra(op)] &  Im(op); break; // and
		case 0x88: m_r[Rc(op)] = m_r[Ra(op)] & ~Im(op); break; // bic
		case 0x94: // cmovlbs
			if (BIT(m_r[Ra(op)], 0))
				m_r[Rc(op)] = Im(op);
			break;
		case 0x96: // cmovlbc
			if (!BIT(m_r[Ra(op)], 0))
				m_r[Rc(op)] = Im(op);
			break;
		case 0xa0: m_r[Rc(op)] = m_r[Ra(op)] |  Im(op); break; // bis
		case 0xa4: // cmoveq
			if (m_r[Ra(op)] == 0)
				m_r[Rc(op)] = Im(op);
			break;
		case 0xa6: // cmovne
			if (m_r[Ra(op)] != 0)
				m_r[Rc(op)] = Im(op);
			break;
		case 0xa8: m_r[Rc(op)] = m_r[Ra(op)] | ~Im(op); break; // ornot
		case 0xc0: m_r[Rc(op)] = m_r[Ra(op)] ^  Im(op); break; // xor
		case 0xc4: // cmovlt
			if (s64(m_r[Ra(op)]) < 0)
				m_r[Rc(op)] = Im(op);
			break;
		case 0xc6: // cmovge
			if (s64(m_r[Ra(op)]) >= 0)
				m_r[Rc(op)] = Im(op);
			break;
		case 0xc8: m_r[Rc(op)] = m_r[Ra(op)] ^ ~Im(op); break; // eqv
		case 0xe1: m_r[Rc(op)] = Im(op); break; // amask
		case 0xe4: // cmovle
			if (s64(m_r[Ra(op)]) <= 0)
				m_r[Rc(op)] = Im(op);
			break;
		case 0xe6: // cmovgt
			if (s64(m_r[Ra(op)]) > 0)
				m_r[Rc(op)] = Im(op);
			break;
		case 0xec: m_r[Rc(op)] = 0; break; // implver
		}
		break;
	case 0x12: // INTS* (integer shift)
		switch ((op >> 5) & 0xff)
		{
			// register variants
		case 0x02: m_r[Rc(op)] = m_r[Ra(op)] & zap_mask(u8(0x01) << (m_r[Rb(op)] & 7)); break; // mskbl
		case 0x06: m_r[Rc(op)] = (m_r[Ra(op)] >> ((m_r[Rb(op)] & 7) * 8)) & zap_mask(~u8(0x01)); break; // extbl
		case 0x0b: m_r[Rc(op)] = (m_r[Ra(op)] << ((m_r[Rb(op)] & 7) * 8)) & zap_mask(~(u8(0x01) << (m_r[Rb(op)] & 7))); break; // insbl
		case 0x12: m_r[Rc(op)] = m_r[Ra(op)] & zap_mask(u8(0x03) << (m_r[Rb(op)] & 7)); break; // mskwl
		case 0x16: m_r[Rc(op)] = (m_r[Ra(op)] >> ((m_r[Rb(op)] & 7) * 8)) & zap_mask(~u8(0x03)); break; // extwl
		case 0x1b: m_r[Rc(op)] = (m_r[Ra(op)] << ((m_r[Rb(op)] & 7) * 8)) & zap_mask(~(u8(0x03) << (m_r[Rb(op)] & 7))); break; // inswl
		case 0x22: m_r[Rc(op)] = m_r[Ra(op)] & zap_mask(u8(0x0f) << (m_r[Rb(op)] & 7)); break; // mskll
		case 0x26: m_r[Rc(op)] = (m_r[Ra(op)] >> ((m_r[Rb(op)] & 7) * 8)) & zap_mask(~u8(0x0f)); break; // extll
		case 0x2b: m_r[Rc(op)] = (m_r[Ra(op)] << ((m_r[Rb(op)] & 7) * 8)) & zap_mask(~(u8(0x0f) << (m_r[Rb(op)] & 7))); break; // insll
		case 0x30: m_r[Rc(op)] = m_r[Ra(op)] & zap_mask(m_r[Rb(op)]); break; // zap
		case 0x31: m_r[Rc(op)] = m_r[Ra(op)] & ~zap_mask(m_r[Rb(op)]); break; // zapnot
		case 0x32: m_r[Rc(op)] = m_r[Ra(op)] & zap_mask(u8(0xff) << (m_r[Rb(op)] & 7)); break; // mskql
		case 0x34: m_r[Rc(op)] = m_r[Ra(op)] >> (m_r[Rb(op)] & 63); break; // srl
		case 0x36: m_r[Rc(op)] = (m_r[Ra(op)] >> ((m_r[Rb(op)] & 7) * 8)) & zap_mask(u8(~u8(0xff))); break; // extql
		case 0x39: m_r[Rc(op)] = m_r[Ra(op)] << (m_r[Rb(op)] & 63); break; // sll
		case 0x3b: m_r[Rc(op)] = (m_r[Ra(op)] << ((m_r[Rb(op)] & 7) * 8)) & zap_mask(~(u8(0xff) << (m_r[Rb(op)] & 7))); break; // insql
		case 0x3c: m_r[Rc(op)] = s64(m_r[Ra(op)]) >> (m_r[Rb(op)] & 63); break; // sra
		case 0x52: m_r[Rc(op)] = m_r[Ra(op)] & zap_mask(u8(0x03) >> (8 - (m_r[Rb(op)] & 7))); break; // mskwh
		case 0x57: m_r[Rc(op)] = (m_r[Ra(op)] >> (64 - ((m_r[Rb(op)] & 7) * 8))) & zap_mask(~(u8(0x03) >> (8 - (m_r[Rb(op)] & 7)))); break; // inswh
		case 0x5a: m_r[Rc(op)] = (m_r[Ra(op)] << (64 - ((m_r[Rb(op)] & 7) * 8))) & zap_mask(~u8(0x03)); break; // extwh
		case 0x62: m_r[Rc(op)] = m_r[Ra(op)] & zap_mask(u8(0x0f) >> (8 - (m_r[Rb(op)] & 7))); break; // msklh
		case 0x67: m_r[Rc(op)] = (m_r[Ra(op)] >> (64 - ((m_r[Rb(op)] & 7) * 8))) & zap_mask(~(u8(0x0f) >> (8 - (m_r[Rb(op)] & 7)))); break; // inslh
		case 0x6a: m_r[Rc(op)] = (m_r[Ra(op)] << (64 - ((m_r[Rb(op)] & 7) * 8))) & zap_mask(~u8(0x0f)); break; // extlh
		case 0x72: m_r[Rc(op)] = m_r[Ra(op)] & zap_mask(u8(0xff) >> (8 - (m_r[Rb(op)] & 7))); break; // mskqh
		case 0x77: m_r[Rc(op)] = (m_r[Ra(op)] >> (64 - ((m_r[Rb(op)] & 7) * 8))) & zap_mask(~(u8(0xff) >> (8 - (m_r[Rb(op)] & 7)))); break; // insqh
		case 0x7a: m_r[Rc(op)] = (m_r[Ra(op)] << (64 - ((m_r[Rb(op)] & 7) * 8))) & zap_mask(u8(~u8(0xff))); break; // extqh

			// immediate variants
		case 0x82: m_r[Rc(op)] = m_r[Ra(op)] & zap_mask(u8(0x01) << (Im(op) & 7)); break; // mskbl
		case 0x86: m_r[Rc(op)] = (m_r[Ra(op)] >> ((Im(op) & 7) * 8)) & zap_mask(~u8(0x01)); break; // extbl
		case 0x8b: m_r[Rc(op)] = (m_r[Ra(op)] << ((Im(op) & 7) * 8)) & zap_mask(~(u8(0x01) << (Im(op) & 7))); break; // insbl
		case 0x92: m_r[Rc(op)] = m_r[Ra(op)] & zap_mask(u8(0x03) << (Im(op) & 7)); break; // mskwl
		case 0x96: m_r[Rc(op)] = (m_r[Ra(op)] >> ((Im(op) & 7) * 8)) & zap_mask(~u8(0x03)); break; // extwl
		case 0x9b: m_r[Rc(op)] = (m_r[Ra(op)] << ((Im(op) & 7) * 8)) & zap_mask(~(u8(0x03) << (Im(op) & 7))); break; // inswl
		case 0xa2: m_r[Rc(op)] = m_r[Ra(op)] & zap_mask(u8(0x0f) << (Im(op) & 7)); break; // mskll
		case 0xa6: m_r[Rc(op)] = (m_r[Ra(op)] >> ((Im(op) & 7) * 8)) & zap_mask(~u8(0x0f)); break; // extll
		case 0xab: m_r[Rc(op)] = (m_r[Ra(op)] << ((Im(op) & 7) * 8)) & zap_mask(~(u8(0x0f) << (Im(op) & 7))); break; // insll
		case 0xb0: m_r[Rc(op)] = m_r[Ra(op)] & zap_mask(Im(op)); break; // zap
		case 0xb1: m_r[Rc(op)] = m_r[Ra(op)] & ~zap_mask(Im(op)); break; // zapnot
		case 0xb2: m_r[Rc(op)] = m_r[Ra(op)] & zap_mask(u8(0xff) << (Im(op) & 7)); break; // mskql
		case 0xb4: m_r[Rc(op)] = m_r[Ra(op)] >> (Im(op) & 63); break; // srl
		case 0xb6: m_r[Rc(op)] = (m_r[Ra(op)] >> ((Im(op) & 7) * 8)) & zap_mask(u8(~u8(0xff))); break; // extql
		case 0xb9: m_r[Rc(op)] = m_r[Ra(op)] << (Im(op) & 63); break; // sll
		case 0xbb: m_r[Rc(op)] = (m_r[Ra(op)] << ((Im(op) & 7) * 8)) & zap_mask(~(u8(0xff) << (Im(op) & 7))); break; // insql
		case 0xbc: m_r[Rc(op)] = s64(m_r[Ra(op)]) >> (Im(op) & 63); break; // sra
		case 0xd2: m_r[Rc(op)] = m_r[Ra(op)] & zap_mask(u8(0x03) >> (8 - (Im(op) & 7))); break; // mskwh
		case 0xd7: m_r[Rc(op)] = (m_r[Ra(op)] >> (64 - ((Im(op) & 7) * 8))) & zap_mask(~(u8(0x03) >> (8 - (Im(op) & 7)))); break; // inswh
		case 0xda: m_r[Rc(op)] = (m_r[Ra(op)] << (64 - ((Im(op) & 7) * 8))) & zap_mask(~u8(0x03)); break; // extwh
		case 0xe2: m_r[Rc(op)] = m_r[Ra(op)] & zap_mask(u8(0x0f) >> (8 - (Im(op) & 7))); break; // msklh
		case 0xe7: m_r[Rc(op)] = (m_r[Ra(op)] >> (64 - ((Im(op) & 7) * 8))) & zap_mask(~(u8(0x0f) >> (8 - (Im(op) & 7)))); break; // inslh
		case 0xea: m_r[Rc(op)] = (m_r[Ra(op)] << (64 - ((Im(op) & 7) * 8))) & zap_mask(~u8(0x0f)); break; // extlh
		case 0xf2: m_r[Rc(op)] = m_r[Ra(op)] & zap_mask(u8(0xff) >> (8 - (Im(op) & 7))); break; // mskqh
		case 0xf7: m_r[Rc(op)] = (m_r[Ra(op)] >> (64 - ((Im(op) & 7) * 8))) & zap_mask(~(u8(0xff) >> (8 - (Im(op) & 7)))); break; // insqh
		case 0xfa: m_r[Rc(op)] = (m_r[Ra(op)] << (64 - ((Im(op) & 7) * 8))) & zap_mask(u8(~u8(0xff))); break; // extqh
		}
		break;
	case 0x13: // INTM* (integer multiply)
		switch ((op >> 5) & 0xff)
		{
			// register variants
		case 0x00: m_r[Rc(op)] = s64(s32(u32(m_r[Ra(op)]) * u32(m_r[Rb(op)]))); break; // mull
		case 0x20: m_r[Rc(op)] = m_r[Ra(op)] * m_r[Rb(op)]; break; // mulq
		case 0x30: mulu_64x64(m_r[Ra(op)], m_r[Rb(op)], &m_r[Rc(op)]); break; // umulh
		case 0x40: m_r[Rc(op)] = s64(s32(u32(m_r[Ra(op)]) * u32(m_r[Rb(op)]))); break; // mull/v
		case 0x60: m_r[Rc(op)] = m_r[Ra(op)] * m_r[Rb(op)]; break; // mulq/v

			// immediate variants
		case 0x80: m_r[Rc(op)] = s64(s32(u32(m_r[Ra(op)]) * u32(Im(op)))); break; // mull
		case 0xa0: m_r[Rc(op)] = m_r[Ra(op)] * Im(op); break; // mulq
		case 0xb0: mulu_64x64(m_r[Ra(op)], Im(op), &m_r[Rc(op)]); break; // umulh
		case 0xc0: m_r[Rc(op)] = s64(s32(u32(m_r[Ra(op)]) * u32(Im(op)))); break; // mull/v
		case 0xe0: m_r[Rc(op)] = m_r[Ra(op)] * Im(op); break; // mulq/v
		}
		break;
	//case 0x14: // ITFP* (integer to floating)
	//case 0x15: // FLTV* (vax floating)
	//case 0x16: // FLTI* (ieee floating)
	//case 0x17: // FLTL* (floating)

	case 0x18: // MISC* (miscellaneous)
		// TODO: all of these are effectively no-ops for now
		switch (u16(op))
		{
		case 0x0000: break; // trapb
		case 0x0400: break; // excb
		case 0x4000: break; // mb
		case 0x4400: break; // wmb
		case 0x8000: break; // fetch
		case 0xa000: break; // fetch_m
		case 0xc000: break; // rpcc
		case 0xe000: break; // rc
		case 0xe800: break; // ecb
		case 0xf000: break; // rs
		case 0xf800: break; // wh64
		}
		break;

	case 0x1a: // JSR*
		m_r[Ra(op)] = m_pc;
		m_pc = m_r[Rb(op)] & ~3;
		break;

	case 0x20: load<u32>(m_r[Rb(op)] + Disp_M(op), [this, op](u32 data) { m_f[Ra(op)] = u32_to_f_floating(data); }); break; // ldf
	case 0x21: load<u64>(m_r[Rb(op)] + Disp_M(op), [this, op](u64 data) { m_f[Ra(op)] = u64_to_g_floating(data); }); break; // ldg
	case 0x22: load<u32>(m_r[Rb(op)] + Disp_M(op), [this, op](u32 data) { m_f[Ra(op)] = f32_to_f64(float32_t{ data }).v; }); break; // lds
	case 0x23: load<u64>(m_r[Rb(op)] + Disp_M(op), [this, op](u64 data) { m_f[Ra(op)] = data; }); break; // ldt
	case 0x24: store<u32>(m_r[Rb(op)] + Disp_M(op), f_floating_to_u32(m_f[Ra(op)])); break; // stf
	case 0x25: store<u64>(m_r[Rb(op)] + Disp_M(op), u64_to_g_floating(m_f[Ra(op)])); break; // stg
	case 0x26: store<u32>(m_r[Rb(op)] + Disp_M(op), f64_to_f32(float64_t{ m_f[Ra(op)] }).v); break; // sts
	case 0x27: store<u64>(m_r[Rb(op)] + Disp_M(op), m_f[Ra(op)]); break; // stt
	case 0x28: load<u32>(m_r[Rb(op)] + Disp_M(op), [this, op](s32 data) { m_r[Ra(op)] = s64(data); }); break; // ldl
	case 0x29: load<u64>(m_r[Rb(op)] + Disp_M(op), [this, op](u64 data) { m_r[Ra(op)] = data; }); break; // ldq
	case 0x2a: // ldl_l
		load_l<u32>(m_r[Rb(op)] + Disp_M(op),
			[this, op](address_space &space, u64 address, s32 data)
			{
				if (m_lock_watch)
					m_lock_watch->remove();

				m_r[Ra(op)] = s64(data);

				space.install_write_tap(offs_t(address & ~15), offs_t(address | 15), "ldl_l",
					[this](offs_t offset, u64 &data, u64 mem_mask)
					{
						m_lock_watch->remove();
						m_lock_watch = nullptr;
					});
			});
		break;
	case 0x2b: // ldq_l
		load_l<u64>(m_r[Rb(op)] + Disp_M(op),
			[this, op](address_space &space, u64 address, u64 data)
			{
				if (m_lock_watch)
					m_lock_watch->remove();

				m_r[Ra(op)] = data;

				space.install_write_tap(offs_t(address & ~15), offs_t(address | 15), "ldq_l",
					[this](offs_t offset, u64 &data, u64 mem_mask)
					{
						m_lock_watch->remove();
						m_lock_watch = nullptr;
					});
			});
		break;
	case 0x2c: store<u32>(m_r[Rb(op)] + Disp_M(op), u32(m_r[Ra(op)])); break; // stl
	case 0x2d: store<u64>(m_r[Rb(op)] + Disp_M(op), m_r[Ra(op)]); break; // stq
	case 0x2e: // stl_c
		if (m_lock_watch)
		{
			store<u32>(m_r[Rb(op)] + Disp_M(op), u32(m_r[Ra(op)]));
			m_r[Ra(op)] = 1;

			m_lock_watch->remove();
			m_lock_watch = nullptr;
		}
		else
			m_r[Ra(op)] = 0;
		break;
	case 0x2f: // stq_c
		if (m_lock_watch)
		{
			store<u64>(m_r[Rb(op)] + Disp_M(op), m_r[Ra(op)]);
			m_r[Ra(op)] = 1;

			m_lock_watch->remove();
			m_lock_watch = nullptr;
		}
		else
			m_r[Ra(op)] = 0;
		break;

		// branch format
	case 0x30: // br
		m_r[Ra(op)] = m_pc;
		m_pc += Disp_B(op);
		break;
	case 0x31: // fbeq
		if (!(m_f[Ra(op)] & 0x7fffffff'ffffffffULL))
			m_pc += Disp_B(op);
		break;
	case 0x32: // fblt
		if (BIT(m_f[Ra(op)], 63) && (m_f[Ra(op)] & 0x7fffffff'ffffffffULL))
			m_pc += Disp_B(op);
		break;
	case 0x33: // fble
		if (BIT(m_f[Ra(op)], 63) || !(m_f[Ra(op)] & 0x7fffffff'ffffffffULL))
			m_pc += Disp_B(op);
		break;
	case 0x34: // bsr
		m_r[Ra(op)] = m_pc;
		m_pc += Disp_B(op);
		break;
	case 0x35: // fbne
		if (m_f[Ra(op)] & 0x7fffffff'ffffffffULL)
			m_pc += Disp_B(op);
		break;
	case 0x36: // fbge
		if (!BIT(m_f[Ra(op)], 63) || !(m_f[Ra(op)] & 0x7fffffff'ffffffffULL))
			m_pc += Disp_B(op);
		break;
	case 0x37: // fbgt
		if (!BIT(m_f[Ra(op)], 63) && (m_f[Ra(op)] & 0x7fffffff'ffffffffULL))
			m_pc += Disp_B(op);
		break;
	case 0x38: // blbc
		if (!BIT(m_r[Ra(op)], 0))
			m_pc += Disp_B(op);
		break;
	case 0x39: // beq
		if (m_r[Ra(op)] == 0)
			m_pc += Disp_B(op);
		break;
	case 0x3a: // blt
		if (s64(m_r[Ra(op)]) < 0)
			m_pc += Disp_B(op);
		break;
	case 0x3b: // ble
		if (s64(m_r[Ra(op)]) <= 0)
			m_pc += Disp_B(op);
		break;
	case 0x3c: // blbs
		if (BIT(m_r[Ra(op)], 0))
			m_pc += Disp_B(op);
		break;
	case 0x3d: // bne
		if (m_r[Ra(op)] != 0)
			m_pc += Disp_B(op);
		break;
	case 0x3e: // bge
		if (s64(m_r[Ra(op)]) >= 0)
			m_pc += Disp_B(op);
		break;
	case 0x3f: // bgt
		if (s64(m_r[Ra(op)]) > 0)
			m_pc += Disp_B(op);
		break;
	}
}

u64 alpha_device::zap_mask(u8 const zap_bits)
{
	u64 mask = 0;

	for (unsigned i = 0; i < 8; i++)
		if (!BIT(zap_bits, i))
			mask |= (0xffULL << (i << 3));

	return mask;
}

// transform from f_floating memory to register format
u64 alpha_device::u32_to_f_floating(u32 const data)
{
	if (!BIT(data, 14) && (data & 0x00003f80UL))
		return
			(u64(data & 0x0000c000UL) << 48) |
			(u64(7) << 61) |
			(u64(data & 0x00003fffUL) << 45) |
			(u64(data & 0xffff0000UL) << 13);
	else
		return
			(u64(data & 0x0000c000UL) << 48) |
			(u64(data & 0x00003fffUL) << 45) |
			(u64(data & 0xffff0000UL) << 13);
}

// transform from f_floating register to memory format
u32 alpha_device::f_floating_to_u32(u64 const data)
{
	return
		(u32(data >> 48) & 0x0000c000UL) |
		(u32(data >> 45) & 0x00003fffUL) |
		(u32(data >> 13) & 0xffff0000UL);
}

// transform between g_floating register and memory format
u64 alpha_device::u64_to_g_floating(u64 const data)
{
	return
		((data & 0x00000000'0000ffffULL) << 48) |
		((data & 0x00000000'ffff0000ULL) << 16) |
		((data & 0x0000ffff'00000000ULL) >> 16) |
		((data & 0xffff0000'00000000ULL) >> 48);
}

bool alpha_ev4_device::cpu_translate(u64 &address, int intention)
{
	// trim virtual address to 43 bits
	address &= 0x7ff'ffffffff;

	if (intention & TRANSLATE_FETCH)
	{
		// instruction superpage mapping
		if ((m_ibx[IBX_ICCSR] & IBX_ICCSR_R_MAP) && !(m_ibx[IBX_PS] & IBX_PS_R_CM) && (address >> 41) == 2)
		{
			address &= 0x3'ffffffff;

			return true;
		}
	}
	else
	{
		// data superpage 1 mapping
		if ((m_abx[ABX_ABOX_CTL] & ABX_ABOX_CTL_SPE_1) && !(m_ibx[IBX_PS] & IBX_PS_R_CM) && (address >> 30) == 0x1ffe)
		{
			address &= 0x3fffffff;

			return true;
		}

		// data superpage 2 mapping
		if ((m_abx[ABX_ABOX_CTL] & ABX_ABOX_CTL_SPE_2) && !(m_ibx[IBX_PS] & IBX_PS_R_CM) && (address >> 41) == 2)
		{
			address &= 0x3'ffffffff;

			return true;
		}
	}

	return true;
}

template <typename T, typename U> std::enable_if_t<std::is_convertible<U, std::function<void(T)>>::value, void> alpha_device::load(u64 address, U &&apply)
{
	cpu_translate(address, TRANSLATE_READ);

	unsigned const s = (address >> 31) & 6;

	switch (sizeof(T))
	{
	case 1: apply(T(space(s).read_byte(address))); break;
	case 2: apply(T(space(s).read_word(address))); break;
	case 4: apply(T(space(s).read_dword(address))); break;
	case 8: apply(T(space(s).read_qword(address))); break;
	}
}

template <typename T, typename U> std::enable_if_t<std::is_convertible<U, std::function<void(address_space &, u64, T)>>::value, void> alpha_device::load_l(u64 address, U &&apply)
{
	cpu_translate(address, TRANSLATE_READ);

	unsigned const s = (address >> 31) & 6;

	switch (sizeof(T))
	{
	case 4: apply(space(s), address, T(space(s).read_dword(address))); break;
	case 8: apply(space(s), address, T(space(s).read_qword(address))); break;
	}
}

template <typename T, typename U> std::enable_if_t<std::is_convertible<U, T>::value, void> alpha_device::store(u64 address, U data, T mem_mask)
{
	cpu_translate(address, TRANSLATE_WRITE);

	unsigned const s = (address >> 31) & 6;

	switch (sizeof(T))
	{
	case 1: space(s).write_byte(address, T(data)); break;
	case 2: space(s).write_word(address, T(data), mem_mask); break;
	case 4: space(s).write_dword(address, T(data), mem_mask); break;
	case 8: space(s).write_qword(address, T(data), mem_mask); break;
	}
}

void alpha_device::fetch(u64 address, std::function<void(u32)> &&apply)
{
	cpu_translate(address, TRANSLATE_FETCH);

	apply(icache_fetch(address));
}

u32 alpha_device::read_srom(unsigned const bits)
{
	u32 data = 0;

	for (unsigned i = 0; i < bits; i++)
		if (m_srom_data_cb())
			data |= (1U << i);

	return data;
}

void alpha_ev4_device::device_start()
{
	alpha_device::device_start();

	save_item(NAME(m_ibx));
	save_item(NAME(m_abx));
	save_item(NAME(m_pt));
}

void alpha_ev4_device::device_reset()
{
	alpha_device::device_reset();

	m_ibx[IBX_ICCSR] = IBX_ICCSR_R_PC0 | IBX_ICCSR_R_PC1; // FIXME: ASN
	m_ibx[IBX_PAL_BASE] = 0;

	m_abx[ABX_ABOX_CTL] = 0;
	m_abx[ABX_BIU_CTL] = 0;
}

void alpha_ev4_device::cpu_execute(u32 const op)
{
	switch (op >> 26)
	{
	case 0x00: // call_pal
		{
			u16 offset = CALL_PAL | ((op & 0x3f) << 6);
			if (op & 0x80)
			{
				// unprivileged
				if (op & CALL_PAL_MASK)
					offset = OPCDEC;
				else
					offset |= 0x1000;
			}
			else
			{
				// privileged
				if ((op & CALL_PAL_MASK) || (m_ibx[IBX_PS] & IBX_PS_R_CM))
					offset = OPCDEC;
			}

			m_ibx[IBX_EXC_ADDR] = m_pc;
			if (m_pal_mode)
				m_ibx[IBX_EXC_ADDR] |= 1;

			m_pal_mode = true;
			m_pc = m_ibx[IBX_PAL_BASE] | offset;
		}
		break;

	case 0x19: // hw_mfpr
		if (op & 0x20)
			m_r[Ra(op)] = ibx_get(Rc(op));
		if (op & 0x40)
			m_r[Ra(op)] = abx_get(Rc(op));
		if (op & 0x80)
			m_r[Ra(op)] = m_pt[Rc(op)];
		break;

	case 0x1d: // hw_mtpr
		if (op & 0x20)
			ibx_set(Rc(op), m_r[Ra(op)]);
		if (op & 0x40)
			abx_set(Rc(op), m_r[Ra(op)]);
		if (op & 0x80)
			m_pt[Rc(op)] = m_r[Ra(op)];
		break;
	case 0x1e: // hw_rei
		m_pc = m_ibx[IBX_EXC_ADDR] & ~3;
		m_pal_mode = BIT(m_ibx[IBX_EXC_ADDR], 0);

		if (m_lock_watch)
		{
			m_lock_watch->remove();
			m_lock_watch = nullptr;
		}
		break;

	default:
		alpha_device::cpu_execute(op);
		break;
	}
}

u64 alpha_ev4_device::ibx_get(u8 reg)
{
	switch (ibx_reg(reg))
	{
		// PALmode only
	case IBX_ITB_PTE:
	case IBX_ITB_PTE_TEMP:
		if (m_pal_mode)
			return m_ibx[reg];
		else
			return 0;

	case IBX_ICCSR:
	case IBX_EXC_ADDR:
	case IBX_SL_RCV:
	case IBX_PS:
	case IBX_EXC_SUM:
	case IBX_PAL_BASE:
	case IBX_HIRR:
	case IBX_SIRR:
	case IBX_ASTRR:
	case IBX_HIER:
	case IBX_SIER:
	case IBX_ASTER:
		return m_ibx[reg];

	default:
		logerror("invalid mfpr/i register %d (%s)\n", reg, machine().describe_context());
		return 0;
	}
}

#define IBX_SET(Reg, Field) if (data & IBX_##Reg##_W_##Field) m_ibx[reg] |= IBX_##Reg##_R_##Field
#define IBX_SHL(Reg, Field, Shift) m_ibx[reg] |= (data & IBX_##Reg##_W_##Field) << Shift
#define IBX_SHR(Reg, Field, Shift) m_ibx[reg] |= (data & IBX_##Reg##_W_##Field) >> Shift

void alpha_ev4_device::ibx_set(u8 reg, u64 data)
{
	switch (ibx_reg(reg))
	{
		// PALmode only
	case IBX_TB_TAG:
	case IBX_ITB_PTE:
	case IBX_ITBZAP:
	case IBX_ITBASM:
	case IBX_ITBIS:
		if (m_pal_mode)
		{
			m_ibx[reg] = data;
			return;
		}
		break;

	case IBX_EXC_ADDR:
	case IBX_EXC_SUM:
	case IBX_SIRR:
	case IBX_ASTRR:
	case IBX_HIER:
	case IBX_SIER:
	case IBX_ASTER:
	case IBX_SL_CLR:
	case IBX_SL_XMIT:
		m_ibx[reg] = data;
		return;

	case IBX_ICCSR:
		m_ibx[reg] = data & IBX_ICCSR_R_PCE;
		IBX_SET(ICCSR, PC1);
		IBX_SET(ICCSR, PC0);
		IBX_SHL(ICCSR, PCMUX0, 1);
		IBX_SHR(ICCSR, GRP1, 19);
		IBX_SHR(ICCSR, ASN, 19);
		return;

	case IBX_PS:
		m_ibx[reg] = 0;
		IBX_SET(PS, CM0);
		IBX_SET(PS, CM1);
		return;

	case IBX_PAL_BASE:
		m_ibx[reg] = data & IBX_PAL_BASE_W;
		return;

	default:
		logerror("invalid mtpr/i register %d (%s)\n", reg, machine().describe_context());
		break;
	}
}

u64 alpha_ev4_device::abx_get(u8 reg)
{
	switch (abx_reg(reg))
	{
	case ABX_DTB_PTE:
	case ABX_DTB_PTE_TEMP:
	case ABX_MM_CSR:
	case ABX_VA:
	case ABX_BIU_ADDR:
	case ABX_BIU_STAT:
	case ABX_DC_STAT:
	case ABX_FILL_ADDR:
		return m_abx[reg];

	default:
		logerror("invalid mfpr/a register %d (%s)\n", reg, machine().describe_context());
		return 0;
	}
}

void alpha_ev4_device::abx_set(u8 reg, u64 data)
{
	switch (abx_reg(reg))
	{
	case ABX_TB_CTL:
	case ABX_DTB_PTE:
	case ABX_DTBZAP:
	case ABX_DTBASM:
	case ABX_DTBIS:
	case ABX_ABOX_CTL:
	case ABX_ALT_MODE:
	case ABX_CC:
	case ABX_CC_CTL:
	case ABX_BIU_CTL:
	case ABX_FILL_SYNDROME:
	case ABX_BC_TAG:
	case ABX_FLUSH_IC:
	case ABX_FLUSH_IC_ASM:
		m_abx[reg] = data;
		return;

	default:
		logerror("invalid mtpr/a register %d (%s)\n", reg, machine().describe_context());
		break;
	}
}

void dec_21064_device::device_reset()
{
	alpha_ev4_device::device_reset();

	m_srom_oe_cb(0);

	// load icache from srom
	for (icache_block &block : m_icache)
	{
		block.lw[0] = read_srom(32);
		block.lw[2] = read_srom(32);
		block.lw[4] = read_srom(32);
		block.lw[6] = read_srom(32);

		block.tag = read_srom(21);
		block.aav = read_srom(8);

		block.lw[1] = read_srom(32);
		block.lw[3] = read_srom(32);
		block.lw[5] = read_srom(32);
		block.lw[7] = read_srom(32);

		block.bht = read_srom(8);
	}

	m_srom_oe_cb(1);
}

u32 dec_21064_device::icache_fetch(u64 const address)
{
	icache_block &block = m_icache[(address >> 5) & 0xff];

	// check tag, valid, and asm or asn
	if ((block.tag != (address >> 13)) || !(block.aav & AAV_V) || (!(block.aav & AAV_ASM) && ((block.aav & AAV_ASN) != (((m_ibx[IBX_ICCSR] & IBX_ICCSR_R_ASN) >> 28)))))
	{
		// fetch a new block
		block.tag = address >> 13;
		block.aav = AAV_V | ((m_ibx[IBX_ICCSR] & IBX_ICCSR_R_ASN) >> 28); // TODO: set ASM depending on PTE

		// always set ASM if istream superpage mapping is enabled
		if (m_ibx[IBX_ICCSR] & IBX_ICCSR_R_MAP)
			block.aav |= AAV_ASM;

		address_space &s = space((address >> 31) & 6);
		for (unsigned i = 0; i < 8; i++)
			block.lw[i] = s.read_dword(address | (i << 2));
	}

	return block.lw[(address >> 2) & 7];
}
