// license:BSD-3-Clause
// copyright-holders:Paul Priest, R. Belmont

/***************************************************************************

    Konami 056734 (ESC) security coprocessor
    Reference: generated-gems/konami_056734_manual.md in https://github.com/rb6502/mame-ai-tips

    A 32-bit core with 32 general and 32 special registers, word-addressed
    local SRAM, and bus-master access to the host's 24-bit big-endian bus.
    At reset an internal boot ROM copies a boot object from the host into
    local memory; the boot object decrypts a command interpreter (kernel)
    from the host ROM, which in turn loads and runs encrypted programs the
    host uploads.  The host starts each transaction by writing a pointer to
    the mailbox.

    Each chip holds a secret (s10, and a nibble of s11) and fetches
    instructions through a descrambler: a permutation of the 2-bit lanes
    of the word plus an XOR constant.  Data reads are not descrambled.

    The firmware's ciphers, for reference (this file runs the firmware and
    implements none of them):

    Image header, big-endian.  The kernel is at s11 + 0x96c; a program resides
    wherever the host's LOAD packet points.
      +0x00  magic 0xfef724fb
      +0x04  code words A ^ 0x3c96        +0x06  data words B ^ 0xb9e5
      +0x08  workspace words C ^ 0x67a5   +0x0a  LOAD entry D ^ 0x3616
      +0x0c  RUN entry E ^ 0x7d8d         +0x0e  third entry F ^ 0x57bb
      +0x10  flags G ^ 0x796c, used only for programs (the kernel's reads
             0x1000 and the boot code ignores it): bit 12 plain text, bits
             11:0 must equal ((A + B) ^ D ^ E ^ F) & 0xfff, bits 15:13 select
             the rounds
      +0x12  16-bit sum of the halfwords of the decrypted code and data
      +0x14  salt
      +0x18  code (stored scrambled for the fetch unit, padded to an even
             word count), then data

    Kernel (decrypted by the boot code): a stream cipher over the code,
    restarted for the data.
      key = (s11[27:24] * 0x01010101) ^ 0x36ae2592 ^ (s10[15:0] * 0x00010001)
      a = key ^ 0x6e8ef8fc, b = key
      each word: plain = word ^ a; a += b; b ^= rotl(a, 16)

    Programs (decrypted by the kernel): a FEAL-8 variant on 64-bit blocks of
    two big-endian words (L, R), restarted for the data.
      k0 = s11[27:24] << 28 | (G & 0xfff) << 16 | s10[15:0]
      k1 = k0 ^ salt
      k2 = ((G >> 13) << 3 | ((E ^ s10) & 3)) & 0xff
    S0(x, y) = rotl8((x + y) & 0xff, 2), S1(x, y) = rotl8((x + y + 1) & 0xff, 2).
    Key schedule (FEAL's): A = k0, B = k1, D = 0; eight times
      B' = fk(A, B ^ D); D = A; A = B; B = B'
    and each B' gives two 16-bit subkeys, high half first: K0 to K15.
    fk(a, b), bytes a0-a3 and b0-b3 most significant first:
      f1 = S1(a0^a1, a2^a3^b0), f2 = S0(a2^a3, f1^b1),
      f0 = S0(a0, f1^b2), f3 = S1(a3, f2^b3); result f0 f1 f2 f3
    Block: n = (k2 >> 2) & 6, or 8 when that is 0 (8 in every shipped image).
      R ^= K1:K0; L ^= K3:K2; R ^= L
      n times: (L, R) = (R, L ^ f(R, K[4 + round]))
      L ^= R; L ^= K[n+5]:K[n+4]; R ^= K[n+7]:K[n+6]; output (R, L)
    Round function f(a, k) by k2 & 3, with k's high and low bytes b0 and b1:
      0: x = a0^a1^b0, y = a2^a3^b1; f1 = S1(x, y), f2 = S0(y, f1), f0 = S0(a0, f1), f3 = S1(a3, f2)
      1: x = a0^a2^b0, y = a1^a3^b1; f2 = S1(x, y), f1 = S0(y, f2), f0 = S0(a0, f2), f3 = S1(a3, f1)
      2: x = a2^a3^b0, y = a0^a1^b1; f3 = S1(x, y), f0 = S0(y, f3), f2 = S0(a2, f3), f1 = S1(a1, f0)
      3: x = a0^a1^b1, y = a2^a3^b0; f0 = S1(x, y), f3 = S0(y, f0), f1 = S0(a1, f0), f2 = S1(a2, f3)
    After each block, with output words o1, o2, two 16-bit subkey updates run,
    m1 = ((k2 & 3) ^ k0) & 3 and then m2 = ((k1 & 3) | 2) ^ m1:
      0: K[o1 & 15] ^= K[(o2 >> 16) & 15]
      1: K[(o1 >> 16) & 15] += 1
      2: K[(o2 >> 16) & 15] += K[o1 & 15]
      3: K[(o2 >> 16) & 15] -= 1

    The boot code is the same on every chip: its runs of no-ops hold the XOR
    constant, and comparing it between chips gives the lane map.

    TODO:
    - opcode 11022 (boot word 278, kernel word 321) is unknown


***************************************************************************/

#include "emu.h"
#include "k056734.h"

#include <algorithm>
#include <bit>

DEFINE_DEVICE_TYPE(K056734, k056734_device, "k056734", "Konami 056734 ESC")

namespace {

constexpr u32 BOOT_OBJECT = 0x200100;
constexpr u32 LOCAL_MASK = 0x1fff;

using dasm = k056734_disassembler;

// operation size from lane 7: word, byte, halfword
constexpr u32 SIZE_MASK[4] = { 0xffffffff, 0x000000ff, 0x0000ffff, 0xffffffff };

enum : unsigned
{
	ALU_ADD,
	ALU_SUB,
	ALU_AND,
	ALU_XOR,
	ALU_OR,
	ALU_NOR
};

constexpr unsigned MEMORY_OP_ALU[4] = { ALU_ADD, ALU_SUB, ALU_AND, ALU_XOR };

} // anonymous namespace


k056734_device::k056734_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: cpu_device(mconfig, K056734, tag, owner, clock)
	, m_local_config("local", ENDIANNESS_BIG, 32, 13, -2, address_map_constructor(FUNC(k056734_device::local_map), this))
	, m_host(*this, finder_base::DUMMY_TAG, -1)
	, m_irq_cb(*this)
	, m_key(0)
	, m_config(0)
	, m_fetch_xor(0)
	, m_fetch_lanes{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 }
	, m_icount(0)
	, m_pc(0)
	, m_reg{}
	, m_flags(0)
	, m_boot(false)
{
}

void k056734_device::local_map(address_map &map)
{
	map(0x0000, 0x1fff).ram();
}

device_memory_interface::space_config_vector k056734_device::memory_space_config() const
{
	return space_config_vector{ std::make_pair(AS_PROGRAM, &m_local_config) };
}

std::unique_ptr<util::disasm_interface> k056734_device::create_disassembler()
{
	return std::make_unique<k056734_disassembler>(k056734_disassembler::descrambler(m_fetch_xor, m_fetch_lanes));
}

void k056734_device::device_validity_check(validity_checker &valid) const
{
	u16 seen = 0;
	for (u8 l : m_fetch_lanes)
	{
		if (l < 16)
			seen |= 1 << l;
	}
	if (seen != 0xffff)
		osd_printf_error("Fetch lane map is not a permutation of 0-15\n");
	if (m_config > 15)
		osd_printf_error("Configuration nibble %u out of range\n", m_config);
}

void k056734_device::device_start()
{
	m_fetch = k056734_disassembler::descrambler(m_fetch_xor, m_fetch_lanes);
	space(AS_PROGRAM).specific(m_local);

	state_add(STATE_GENPC, "GENPC", m_pc).noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_pc).noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS", m_flags).formatstr("%4s").noshow();
	state_add(K056734_PC, "PC", m_pc).mask(LOCAL_MASK);
	state_add(K056734_FLAGS, "FLAGS", m_flags).mask(0x0f);
	for (unsigned i = 1; i < 32; i++)
		state_add(K056734_R1 + i - 1, util::string_format("R%u", i).c_str(), m_reg[i]);
	for (unsigned i = 0; i < 32; i++)
	{
		if (i != S_PC - 32)
			state_add(K056734_S0 + i, util::string_format("S%u", i).c_str(), m_reg[32 + i]);
	}

	save_item(NAME(m_pc));
	save_item(NAME(m_reg));
	save_item(NAME(m_flags));
	save_item(NAME(m_boot));

	set_icountptr(m_icount);
}

void k056734_device::device_reset()
{
	m_reg[S_KEY] = m_key;
	m_reg[S_BOOT] = (u32(m_config) << 24) | BOOT_OBJECT;
	m_reg[S_HOST] &= ~u32(2);
	m_pc = 0;
	m_boot = true;
	m_irq_cb(0);
}

u32 k056734_device::execute_min_cycles() const noexcept
{
	return 1;
}

u32 k056734_device::execute_max_cycles() const noexcept
{
	return 1;
}

void k056734_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
	case STATE_GENFLAGS:
		str = util::string_format("%c%c%c%c",
				(m_flags & F_V) ? 'V' : '.',
				(m_flags & F_C) ? 'C' : '.',
				(m_flags & F_N) ? 'N' : '.',
				(m_flags & F_Z) ? 'Z' : '.');
		break;
	}
}


void k056734_device::mailbox_w(u32 data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(k056734_device::mailbox_sync), this), s32(data));
}

TIMER_CALLBACK_MEMBER(k056734_device::mailbox_sync)
{
	m_reg[S_MAILBOX] = u32(param);
}


u32 k056734_device::get_reg(unsigned r)
{
	switch (r)
	{
	case 0:
		return 0;
	case S_PC:
		return m_pc + 2;
	case S_HOST:
		// bit 1 drives the host interrupt; the firmware pulses it with ori, so it can't read back
		return m_reg[S_HOST] & ~u32(2);
	case S_MAILBOX:
		// idle skip
		if (!m_reg[S_MAILBOX])
			m_icount = 0;
		return m_reg[S_MAILBOX];
	default:
		return m_reg[r];
	}
}

void k056734_device::set_reg(unsigned r, u32 data)
{
	switch (r)
	{
	case 0:
		break;
	case S_PC:
		logerror("%04x: write %08x to s2\n", m_pc, data);
		break;
	case S_HOST:
		if (BIT(data ^ m_reg[S_HOST], 1))
			m_irq_cb(BIT(data, 1));
		m_reg[S_HOST] = data;
		break;
	default:
		m_reg[r] = data;
		break;
	}
}

void k056734_device::push(u32 data)
{
	m_reg[S_SP]--;
	m_local.write_dword(m_reg[S_SP] & LOCAL_MASK, data);
}

u32 k056734_device::pop()
{
	u32 const data = m_local.read_dword(m_reg[S_SP] & LOCAL_MASK);
	m_reg[S_SP]++;
	return data;
}

u32 k056734_device::alu(unsigned op, u32 a, u32 b, unsigned size)
{
	u32 const mask = SIZE_MASK[size];
	u32 const sign = (mask >> 1) + 1;
	a &= mask;
	b &= mask;

	u32 r;
	u8 flags = m_flags & (F_C | F_V);
	switch (op)
	{
	case ALU_ADD:
		r = (a + b) & mask;
		flags = ((r < a) ? F_C : 0) | ((~(a ^ b) & (a ^ r) & sign) ? F_V : 0);
		break;
	case ALU_SUB:
		r = (a - b) & mask;
		flags = ((a < b) ? F_C : 0) | ((((a ^ b) & (a ^ r)) & sign) ? F_V : 0);
		break;
	case ALU_AND:
		r = a & b;
		break;
	case ALU_XOR:
		r = a ^ b;
		break;
	case ALU_OR:
		r = a | b;
		break;
	default:
		r = ~(a | b) & mask;
		break;
	}
	if (!r)
		flags |= F_Z;
	if (r & sign)
		flags |= F_N;
	m_flags = flags;
	return r;
}

u32 k056734_device::host_read(u32 addr, unsigned size)
{
	addr &= 0xffffff;
	switch (size)
	{
	case 1:
		return m_host->read_byte(addr);
	case 2:
		return m_host->read_word_unaligned(addr);
	default:
		return m_host->read_dword_unaligned(addr);
	}
}

void k056734_device::host_write(u32 addr, u32 data, unsigned size)
{
	addr &= 0xffffff;
	switch (size)
	{
	case 1:
		m_host->write_byte(addr, data);
		break;
	case 2:
		m_host->write_word_unaligned(addr, data);
		break;
	default:
		m_host->write_dword_unaligned(addr, data);
		break;
	}
}

void k056734_device::boot()
{
	// the boot ROM copies the boot object (a word count, then the words) to local 0 and starts it
	u32 const base = m_reg[S_BOOT] & 0xffffff;
	u32 const count = std::min<u32>(host_read(base, 0), LOCAL_MASK + 1);
	for (u32 i = 0; i < count; i++)
		m_local.write_dword(i, host_read(base + 4 + (i * 4), 0));
	m_icount -= count;
}

void k056734_device::execute_run()
{
	if (m_boot)
	{
		m_boot = false;
		boot();
	}

	while (m_icount > 0)
	{
		debugger_instruction_hook(m_pc);
		execute_one(m_fetch(m_local.read_dword(m_pc)));
		m_icount--;
	}
}

void k056734_device::execute_one(u32 op)
{
	unsigned const size = dasm::lane(op, 7);
	unsigned const ra = dasm::reg_a(op);
	unsigned const rb = dasm::reg_b(op);
	unsigned const off = dasm::off10(op);
	s32 const disp = util::sext(off, 10);
	u32 const imm = dasm::imm16(op);
	u32 const mask = SIZE_MASK[size];
	u32 npc = m_pc + 1;

	if (!op)
	{
		m_pc = npc & LOCAL_MASK;
		return;
	}

	switch (dasm::lane(op, 0))
	{
	case 0:
		if (dasm::is_branch(op))
		{
			// branch, displacement in words from the next instruction
			bool const z = m_flags & F_Z;
			bool const n = m_flags & F_N;
			bool const c = m_flags & F_C;
			bool const v = m_flags & F_V;
			bool taken = false;
			switch (dasm::op4(op))
			{
			case dasm::op4(1, 0, 0, 0): taken = true; break;                     // jmp
			case dasm::op4(1, 1, 0, 0): taken = true; push(npc); break;          // call
			case dasm::op4(2, 0, 0, 0): taken = z; break;                        // beq
			case dasm::op4(3, 0, 0, 0): taken = !z; break;                       // bne
			case dasm::op4(0, 0, 1, 0): taken = n; break;                        // bmi
			case dasm::op4(1, 0, 1, 0): taken = !n; break;                       // bpl
			case dasm::op4(2, 0, 1, 0): taken = c; break;                        // bcs
			case dasm::op4(3, 0, 1, 0): taken = !c; break;                       // bcc
			case dasm::op4(1, 0, 2, 0): taken = !z && (n == v); break;           // bgt
			case dasm::op4(3, 0, 2, 0): taken = !c && !z; break;                 // bhi
			case dasm::op4(0, 0, 3, 0): taken = n != v; break;                   // blt
			default:
				logerror("%04x: unknown branch condition %d%d%d\n", m_pc, dasm::lane(op, 10), dasm::lane(op, 8), dasm::lane(op, 3));
				break;
			}
			if (taken)
				npc = m_pc + 1 + s16(imm);
		}
		else if (dasm::op5(op) == dasm::op5(1, 0, 2, 0, 0))
		{
			// jr
			npc = get_reg(ra) + disp;
		}
		else if (dasm::op5(op) == dasm::op5(1, 1, 2, 0, 0))
		{
			// jsr
			push(npc);
			npc = get_reg(ra);
		}
		else
		{
			// li: 24-bit immediate, destination encoded in lanes 8, 3 and 10
			set_reg(dasm::reg_d(op), dasm::imm24(op));
		}
		break;

	case 1:
		// absolute host access, 24-bit address
		if (BIT(dasm::lane(op, 8), 1))
			host_write(dasm::imm24(op), get_reg(dasm::reg_d(op)), 0);
		else
			set_reg(dasm::reg_d(op), host_read(dasm::imm24(op), 0));
		break;

	case 2:
		switch (dasm::op5(op))
		{
		case dasm::op5(1, 1, 0, 0, 2): // push
			push(get_reg(ra));
			break;
		case dasm::op5(3, 1, 0, 0, 2): // pop
			set_reg(ra, pop());
			break;
		case dasm::op5(1, 1, 0, 1, 2): // ret
		case dasm::op5(3, 1, 0, 1, 2):
			npc = pop();
			break;
		case dasm::op5(1, 0, 0, 3, 2): // lih
			set_reg(ra, (get_reg(ra) & 0x0000ffff) | (imm << 16));
			break;
		case dasm::op5(3, 0, 0, 1, 2): // swap16
			set_reg(ra, std::rotl(get_reg(rb), 16));
			break;
		case dasm::op5(1, 0, 2, 1, 2): // ext.h
			set_reg(ra, s16(get_reg(rb)));
			break;
		case dasm::op5(1, 0, 1, 1, 2): // ext.b
			set_reg(ra, s8(get_reg(rb)));
			break;
		case dasm::op5(1, 0, 0, 2, 2): // mul
			m_reg[S_MULDIV] = get_reg(ra) * get_reg(rb);
			break;
		case dasm::op5(1, 0, 2, 2, 2): // mul.h
			m_reg[S_MULDIV] = s32(s16(get_reg(ra))) * s32(s16(get_reg(rb)));
			break;
		case dasm::op5(3, 0, 2, 2, 2): // div.h
			if (u16(get_reg(rb)))
				m_reg[S_MULDIV] = u16(get_reg(ra)) / u16(get_reg(rb));
			break;
		case dasm::op5(3, 0, 0, 0, 2): // taken as neg; only the boot RAM test uses it, where any bijection passes
			set_reg(ra, -get_reg(rb));
			break;
		default:
			{
				// shifts and rotates by one bit within the size; upper bits come from rB
				u32 const sign = (mask >> 1) + 1;
				u32 const a = get_reg(rb);
				u32 const v = a & mask;
				u32 r;
				switch (dasm::op4(op))
				{
				case dasm::op4(1, 2, 0, 2): // shl
				case dasm::op4(1, 2, 1, 2): // asl
					r = (v << 1) & mask;
					break;
				case dasm::op4(1, 2, 2, 2): // rol
					r = ((v << 1) & mask) | ((v & sign) ? 1 : 0);
					break;
				case dasm::op4(3, 2, 0, 2): // shr
					r = v >> 1;
					break;
				case dasm::op4(3, 2, 1, 2): // asr
					r = (v >> 1) | (v & sign);
					break;
				case dasm::op4(3, 2, 2, 2): // ror
					r = (v >> 1) | ((v & 1) ? sign : 0);
					break;
				default:
					logerror("%04x: unknown opcode %08x\n", m_pc, op);
					m_pc = npc & LOCAL_MASK;
					return;
				}
				m_flags = (m_flags & (F_C | F_V)) | (r ? 0 : F_Z) | ((r & sign) ? F_N : 0);
				set_reg(ra, (a & ~mask) | r);
			}
			break;
		}
		break;

	case 3:
		{
			u32 const a = get_reg(ra);
			auto const host = [this, rb, disp] () { return get_reg(rb) + disp; };
			auto const local = [this, rb, disp] () { return (get_reg(rb) + disp) & LOCAL_MASK; };
			switch (dasm::op4(op))
			{
			// register-register: upper bits come from rB
			case dasm::op4(1, 2, 0, 3): set_reg(ra, (get_reg(rb) & ~mask) | alu(ALU_ADD, get_reg(rb), get_reg(BIT(off, 3, 6)), size)); break;
			case dasm::op4(1, 2, 1, 3): set_reg(ra, (get_reg(rb) & ~mask) | alu(ALU_SUB, get_reg(rb), get_reg(BIT(off, 3, 6)), size)); break;
			case dasm::op4(1, 2, 2, 3): set_reg(ra, (get_reg(rb) & ~mask) | alu(ALU_AND, get_reg(rb), get_reg(BIT(off, 3, 6)), size)); break;
			case dasm::op4(1, 2, 3, 3): set_reg(ra, (get_reg(rb) & ~mask) | alu(ALU_XOR, get_reg(rb), get_reg(BIT(off, 3, 6)), size)); break;
			case dasm::op4(3, 2, 1, 3): set_reg(ra, (get_reg(rb) & ~mask) | alu(ALU_OR, get_reg(rb), get_reg(BIT(off, 3, 6)), size)); break;
			case dasm::op4(3, 2, 3, 3): set_reg(ra, (get_reg(rb) & ~mask) | alu(ALU_NOR, get_reg(rb), get_reg(BIT(off, 3, 6)), size)); break;

			// immediate, zero-extended: upper bits come from rA
			case dasm::op4(1, 3, 0, 3): set_reg(ra, (a & ~mask) | alu(ALU_ADD, a, imm, size)); break;
			case dasm::op4(1, 3, 1, 3): set_reg(ra, (a & ~mask) | alu(ALU_SUB, a, imm, size)); break;
			case dasm::op4(1, 3, 2, 3): set_reg(ra, (a & ~mask) | alu(ALU_AND, a, imm, size)); break;
			case dasm::op4(1, 3, 3, 3): set_reg(ra, (a & ~mask) | alu(ALU_XOR, a, imm, size)); break;
			case dasm::op4(3, 3, 1, 3): set_reg(ra, (a & ~mask) | alu(ALU_OR, a, imm, size)); break;

			// loads zero-extend; local stores below a word keep the upper bits of the word
			case dasm::op4(2, 0, 3, 3): set_reg(ra, host_read(host(), size)); break;
			case dasm::op4(3, 0, 3, 3): host_write(host(), a, size); break;
			case dasm::op4(2, 1, 3, 3): set_reg(ra, m_local.read_dword(local()) & mask); break;
			case dasm::op4(3, 1, 3, 3): m_local.write_dword(local(), (m_local.read_dword(local()) & ~mask) | (a & mask)); break;

			// register op local memory
			case dasm::op4(0, 1, 0, 3): set_reg(ra, (a & ~mask) | alu(ALU_ADD, a, m_local.read_dword(local()), size)); break;
			case dasm::op4(0, 1, 1, 3): alu(ALU_SUB, a, m_local.read_dword(local()), size); break;
			case dasm::op4(0, 1, 2, 3): set_reg(ra, (a & ~mask) | alu(ALU_AND, a, m_local.read_dword(local()), size)); break;
			case dasm::op4(0, 1, 3, 3): set_reg(ra, (a & ~mask) | alu(ALU_XOR, a, m_local.read_dword(local()), size)); break;
			case dasm::op4(2, 1, 1, 3): set_reg(ra, (a & ~mask) | alu(ALU_OR, a, m_local.read_dword(local()), size)); break;

			// local memory op register
			case dasm::op4(1, 1, 0, 3):
			case dasm::op4(1, 1, 1, 3):
			case dasm::op4(1, 1, 2, 3):
			case dasm::op4(1, 1, 3, 3):
				{
					u32 const l = local();
					u32 const m = m_local.read_dword(l);
					m_local.write_dword(l, (m & ~mask) | alu(MEMORY_OP_ALU[dasm::lane(op, 3)], m, a, size));
				}
				break;

			// compare with host memory
			case dasm::op4(0, 0, 1, 3): alu(ALU_SUB, a, host_read(host(), size), size); break;
			case dasm::op4(1, 0, 1, 3): alu(ALU_SUB, host_read(host(), size), a, size); break;

			default:
				logerror("%04x: unknown opcode %08x\n", m_pc, op);
				break;
			}
		}
		break;
	}

	m_pc = npc & LOCAL_MASK;
}
