// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, David Carne

// KS0164 core

#include "emu.h"
#include "ks0164.h"
#include "ks0164d.h"

DEFINE_DEVICE_TYPE(KS0164CPU, ks0164_cpu_device, "ks0164cpu", "Samsung KS0164 audio processor")

ks0164_cpu_device::ks0164_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cpu_device(mconfig, KS0164CPU, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_BIG, 16, 16)
{
}

void ks0164_cpu_device::device_start()
{
	space(AS_PROGRAM).cache(m_program_cache);
	space(AS_PROGRAM).specific(m_program);

	state_add(STATE_GENPC,     "GENPC",     m_r[R_PC]).callexport().noshow();
	state_add(STATE_GENPCBASE, "CURPC",     m_r[R_PC]).callexport().noshow();
	state_add(STATE_GENFLAGS,  "GENFLAGS",  m_r[R_PSW]).callimport().formatstr("%8s").noshow();
	state_add(KS0164_PC,       "PC",        m_r[R_PC]).callimport();
	state_add(KS0164_PSW,      "PSW",       m_r[R_PSW]).callimport();

	state_add(KS0164_SP,       "SP",        m_r[R_SP]);

	state_add(KS0164_R0,       "R0",        m_r[0]);
	state_add(KS0164_R1,       "R1",        m_r[1]);
	state_add(KS0164_R2,       "R2",        m_r[2]);
	state_add(KS0164_R3,       "R3",        m_r[3]);

	save_item(NAME(m_r));

	set_icountptr(m_icount);

	m_irq = 0x10000;
	memset(m_r, 0, sizeof(m_r));
}

void ks0164_cpu_device::device_reset()
{
	m_irq = 1;
	memset(m_r, 0, sizeof(m_r));
	m_r[R_PSW] = F_I;
}

uint32_t ks0164_cpu_device::execute_min_cycles() const noexcept
{
	return 1;
}

uint32_t ks0164_cpu_device::execute_max_cycles() const noexcept
{
	return 5;
}

void ks0164_cpu_device::execute_set_input(int inputnum, int state)
{
	if(state)
		m_irq |= 1 << inputnum;
	else
		m_irq &= ~(1 << inputnum);
}

device_memory_interface::space_config_vector ks0164_cpu_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config)
	};
}

std::unique_ptr<util::disasm_interface> ks0164_cpu_device::create_disassembler()
{
	return std::make_unique<ks0164_disassembler>();
}

void ks0164_cpu_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch(entry.index()) {
	case STATE_GENFLAGS:
		str = util::string_format("%c %c%c%c%c %x",
								  m_r[R_PSW] & F_I ? 'I' : '-',
								  m_r[R_PSW] & F_V ? 'V' : '-',
								  m_r[R_PSW] & F_C ? 'C' : '-',
								  m_r[R_PSW] & F_N ? 'N' : '-',
								  m_r[R_PSW] & F_Z ? 'Z' : '-',
								  m_r[R_PSW] & 0xf);
		break;
	}
}

void ks0164_cpu_device::handle_irq()
{
	u16 mask = m_irq & util::make_bitmask<u16>((m_r[R_PSW] & 15) + 1);
	if(mask) {
		int index;
		for(index = 0; !(mask & (1 << index)); index ++);
		if(index) {
			// Normal irq (not reset), save pc and psw
			if(m_r[R_PSW] & F_I)
				return;
			standard_irq_callback(0, m_r[R_PC]);
			m_program.write_word(m_r[R_SP] - 2, m_r[R_PC]);
			m_program.write_word(m_r[R_SP] - 4, m_r[R_PSW]);
			m_r[R_SP] -= 4;
			m_icount -= 2;
		} else
			m_irq &= 0xfffe;

		m_r[R_PSW] = (m_r[R_PSW] & 0xfff0) | (index ? index - 1 : 0);
		m_r[R_PC] = m_program_cache.read_word(index*2);
		m_icount --;
	}
}

u16 ks0164_cpu_device::snz(u16 r)
{
	u16 f = 0;
	if(!r)
		f |= F_Z;
	if(r & 0x8000)
		f |= F_N;
	m_r[R_PSW] = (m_r[R_PSW] & ~F_MASK) | f;
	return r;
}

void ks0164_cpu_device::do_alu(u16 opcode, u16 v2)
{
	int r = (opcode >> 8) & 7;
	switch((opcode >> 11) & 7) {
	case 0: { // add
		u16 v1 = m_r[r];
		u16 res = v1 + v2;
		u16 f = 0;
		if(!res)
			f |= F_Z;
		if(res & 0x8000)
			f |= F_N;
		if(((v1 & v2) | ((~res) & (v1 | v2))) & 0x8000)
			f |= F_C;
		if(((v1 ^ res) & (v2 ^ res)) & 0x8000)
			f |= F_V;
		m_r[r] = res;
		m_r[R_PSW] = (m_r[R_PSW] & ~F_MASK) | f;
		break;
	}
	case 1: { // sub
		u16 v1 = m_r[r];
		u16 res = v1 - v2;
		u16 f = 0;
		if(!res)
			f |= F_Z;
		if(res & 0x8000)
			f |= F_N;
		if(((v2 & res) | ((~v1) & (v2 | res))) & 0x8000)
			f |= F_C;
		if(((v1 ^ v2) & (v1 ^ res)) & 0x8000)
			f |= F_V;
		m_r[r] = res;
		m_r[R_PSW] = (m_r[R_PSW] & ~F_MASK) | f;
		break;
	}
	case 2: { // cmp
		u16 v1 = m_r[r];
		u16 res = v1 - v2;
		u16 f = 0;
		if(!res)
			f |= F_Z;
		if(res & 0x8000)
			f |= F_N;
		if(((v2 & res) | ((~v1) & (v2 | res))) & 0x8000)
			f |= F_C;
		if(((v1 ^ v2) & (v1 ^ res)) & 0x8000)
			f |= F_V;
		m_r[R_PSW] = (m_r[R_PSW] & ~F_MASK) | f;
		break;
	}
	case 3:   // and
		m_r[r] = snz(m_r[r] & v2);
		break;
	case 4:   // or
		m_r[r] = snz(m_r[r] | v2);
		break;
	case 5:   // xor
		m_r[r] = snz(m_r[r] ^ v2);
		break;
	case 6:   // set
		m_r[r] = snz(v2);
		break;
	case 7: { // mul
		u32 res;
		if(opcode & 0x0080)
			res = s16(m_r[r]) * s16(v2);
		else
			res = u16(m_r[r]) * u16(v2);

		u16 f = 0;
		if(!res)
			f |= F_Z;
		if(res & 0x8000)
			f |= F_N;
		if(res & 0xffff0000)
			f |= F_C;
		if(res >= 0x00008000 && res < 0xffff8000)
			f |= F_V;
		m_r[r] = res;
		m_r[R_PSW] = (m_r[R_PSW] & ~F_MASK) | f;
		break;
	}
	}
}

void ks0164_cpu_device::unk(u16 opcode)
{
	logerror("Unknown opcode %04x at address %04x\n", opcode, m_r[R_PC]-2);
}

void ks0164_cpu_device::execute_run()
{
	while(m_icount > 0) {
		if(m_irq)
			handle_irq();
		debugger_instruction_hook(m_r[R_PC]);
		u16 opcode = m_program_cache.read_word(m_r[R_PC]);
		m_r[R_PC] += 2;

		// First switch level on bits 15-14 and 2-0
		switch(((opcode >> 11) & 0x18) | (opcode & 0x7)) {

		case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07: {
			// Conditional branches
			// 00cc ccoo oooo oooo
			bool cond;
			switch((opcode >> 10) & 0xf) {
			case 0x0: cond = !(m_r[R_PSW] & F_Z); break;
			case 0x1: cond =   m_r[R_PSW] & F_Z ; break;
			case 0x2: cond = !(m_r[R_PSW] & F_C); break;
			case 0x3: cond =   m_r[R_PSW] & F_C ; break;
			case 0x4: cond = !(m_r[R_PSW] & F_N); break;
			case 0x5: cond =   m_r[R_PSW] & F_N ; break;
			case 0x6: cond = !(m_r[R_PSW] & F_V); break;
			case 0x7: cond =   m_r[R_PSW] & F_V ; break;
			case 0x8: cond = false;               break;
			case 0x9: cond = !(m_r[R_PSW] & F_Z) && !(m_r[R_PSW] & F_C); break;
			case 0xa: cond = (m_r[R_PSW] & F_Z) || (m_r[R_PSW] & F_C); break;
			case 0xb: cond = (!(m_r[R_PSW] & F_Z) && (m_r[R_PSW] & F_N) && (m_r[R_PSW] & F_V)) || (!(m_r[R_PSW] & F_Z) && !(m_r[R_PSW] & F_V) && !(m_r[R_PSW] & F_N)); break;
			case 0xc: cond = ((m_r[R_PSW] & F_N) && (m_r[R_PSW] & F_V)) || (!(m_r[R_PSW] & F_V) && !(m_r[R_PSW] & F_N)); break;
			case 0xd: cond = (m_r[R_PSW] & F_Z) || ((m_r[R_PSW] & F_N) && !(m_r[R_PSW] & F_V)) || ((m_r[R_PSW] & F_V) && !(m_r[R_PSW] & F_N)); break;
			case 0xe: cond = ((m_r[R_PSW] & F_N) && !(m_r[R_PSW] & F_V)) || ((m_r[R_PSW] & F_V) && !(m_r[R_PSW] & F_N)); break;
			case 0xf: default: cond = true; break;
			}
			if(cond) {
				m_r[R_PC] += util::sext(opcode, 10);
			}
			break;
		}

		case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f: {
			// ALU functions with 8-bit immediate
			// 01ff frrr iiii iiii

			do_alu(opcode, s8(opcode));
			break;
		}

		case 0x14: {
			// ALU functions with other register
			// 10ff frrr Ssss w100
			u16 rv = m_r[(opcode >> 4) & 7];
			u16 v = opcode & 0x0008 ? rv : opcode & 0x0080 ? s8(rv) : u8(rv);

			do_alu(opcode, v);
			break;
		}

		case 0x15: {
			// ALU functions with immediate
			// 10ff frrr S... w101
			u16 rv = m_program_cache.read_word(m_r[R_PC]);
			m_r[R_PC] += 2;
			u16 v = opcode & 0x0008 ? rv : opcode & 0x0080 ? s8(rv) : u8(rv);

			do_alu(opcode, v);
			break;
		}

		case 0x16: {
			// ALU functions from memory indexed
			// 10ff frrr Ssss w110
			u16 a = m_r[(opcode >> 4) & 7];
			u16 v = opcode & 0x0008 ? m_program.read_word(a) : opcode & 0x0080 ? s8(m_program.read_byte(a)) : u8(m_program.read_byte(a));
			m_icount -= 2;

			do_alu(opcode, v);
			break;
		}

		case 0x17: {
			// ALU functions from memory indexed and offset
			// 10ff frrr Ssss w111
			u16 a = m_r[(opcode >> 4) & 7] + m_program_cache.read_word(m_r[R_PC]);
			m_r[R_PC] += 2;
			u16 v = opcode & 0x0008 ? m_program.read_word(a) : opcode & 0x0080 ? s8(m_program.read_byte(a)) : u8(m_program.read_byte(a));
			m_icount -= 2;

			do_alu(opcode, v);
			break;
		}

		case 0x18: {
			switch((opcode >> 12) & 3) {
			case 0: {
				// Data move with pre/post incrementation
				// 1100 arrr bsss c000

				int r1 = (opcode >> 8) & 7;
				int r2 = (opcode >> 4) & 7;
				switch(bitswap<3>(opcode, 11, 7, 3)) {
				case 1: m_program.write_word(m_r[r1], m_r[r2]); m_r[r1] += 2; break;
				case 4: m_r[r1] -= 2; m_program.write_word(m_r[r1], m_r[r2]); break;
				default: unk(opcode); break;
				}
				m_icount --;
				break;
			}

			case 1: {
				// Min/max with immediate
				// 1101 Mrrr S000 1000
				u16 v1 = m_r[(opcode >> 8) & 7];
				u16 v2 = m_program_cache.read_word(m_r[R_PC]);
				m_r[R_PC] += 2;
				u16 res;
				switch(bitswap<2>(opcode, 11, 7)) {
				case 0: res = std::max<u16>(v1, v2); break;
				case 1: res = std::max<s16>(v1, v2); break;
				case 2: res = std::min<u16>(v1, v2); break;
				case 3: default: res = std::min<s16>(v1, v2); break;
				}
				m_r[(opcode >> 8) & 7] = res;
				break;
			}

			case 2: {
				// Bit test in register
				// 1110 .rrr bbbb .000
				if(m_r[(opcode >> 8) & 7] & (1 << ((opcode >> 4) & 0xf)))
					m_r[R_PSW] &= ~F_Z;
				else
					m_r[R_PSW] |=  F_Z;
				break;
			}

			case 3: {
				unk(opcode);
				break;
			}
			}
			break;
		}

		case 0x19: {
			switch((opcode >> 12) & 3) {
			case 0: {
				// Push all registers
				// 1100 .... .... .001

				m_program.write_word(m_r[R_SP] - 2, m_r[0]);
				m_program.write_word(m_r[R_SP] - 4, m_r[1]);
				m_program.write_word(m_r[R_SP] - 6, m_r[2]);
				m_program.write_word(m_r[R_SP] - 8, m_r[3]);
				m_r[R_SP] -= 8;
				break;
			}

			case 1: {
				// Absolute jump
				// 1101 .... .... .001
				m_r[R_PC] = m_program.read_word(m_r[R_PC]);
				break;
			}

			case 2: {
				// Bit set in register
				// 1110 .rrr bbbb .001
				if(m_r[(opcode >> 8) & 7] & (1 << ((opcode >> 4) & 0xf)))
					m_r[R_PSW] &= ~F_Z;
				else {
					m_r[R_PSW] |=  F_Z;
					m_r[(opcode >> 8) & 7] |= 1 << ((opcode >> 4) & 0xf);
				}
				break;
			}

			case 3: {
				unk(opcode);
				break;
			}
			}
			break;
		}

		case 0x1a: {
			switch((opcode >> 12) & 3) {
			case 0: {
				// Data move with pre/post incrementation
				// 1100 arrr bsss c010

				int r1 = (opcode >> 8) & 7;
				int r2 = (opcode >> 4) & 7;
				switch(bitswap<3>(opcode, 11, 7, 3)) {
				case 1: m_r[r1] = m_program.read_word(m_r[r2]); m_r[r2] += 2; break;
				default: unk(opcode); break;
				}
				m_icount --;
				break;
			}

			case 1: {
				// Absolute subroutine call
				// 1101 .... .... .010

				u16 a = m_program.read_word(m_r[R_PC]);
				m_program.write_word(m_r[R_SP] - 2, m_r[R_PC] + 2);
				m_r[R_SP] -= 2;
				m_r[R_PC] = a;
				m_icount -= 2;
				break;
			}

			case 2: {
				// Bit test from memory indexed
				// 1110 .rrr bbbb w010

				u16 a = m_r[(opcode >> 8) & 7];
				u16 v = opcode & 0x0008 ? m_program.read_word(a) : m_program.read_byte(a);
				if(v & (1 << ((opcode >> 4) & 0xf)))
					m_r[R_PSW] &= ~F_Z;
				else
					m_r[R_PSW] |=  F_Z;
				m_icount -= 2;
				break;
			}

			case 3: {
				unk(opcode);
				break;
			}
			}
			break;
		}

		case 0x1b: {
			switch((opcode >> 12) & 3) {
			case 0: {
				// Pop all registers
				// 1100 .... .... .011

				m_r[0] = m_program.read_word(m_r[R_SP] + 6);
				m_r[1] = m_program.read_word(m_r[R_SP] + 4);
				m_r[2] = m_program.read_word(m_r[R_SP] + 2);
				m_r[3] = m_program.read_word(m_r[R_SP] + 0);
				m_r[R_SP] += 8;
				break;
			}

			case 1: {
				// Return from subroutine
				// 1101 .... .... .011

				m_r[R_PC] = m_program.read_word(m_r[R_SP]);
				m_r[R_SP] += 2;
				m_icount --;
				break;
			}

			case 2: {
				// Bit test from memory indexed and offset
				// 1110 .rrr bbbb w011

				u16 a = m_r[(opcode >> 8) & 7] + m_program_cache.read_word(m_r[R_PC]);
				m_r[R_PC] += 2;
				u16 v = opcode & 0x0008 ? m_program.read_word(a) : m_program.read_byte(a);
				if(v & (1 << ((opcode >> 4) & 0xf)))
					m_r[R_PSW] &= ~F_Z;
				else
					m_r[R_PSW] |=  F_Z;
				m_icount -= 2;
				break;
			}

			case 3: {
				unk(opcode);
				break;
			}
			}
			break;
		}

		case 0x1c: {
			switch((opcode >> 12) & 3) {
			case 0: {
				// Unsigned shifts
				// 1100 drrr nnnn .100

				int r = (opcode >> 8) & 7;
				int shift = (opcode >> 4) & 0xf;
				u16 v1 = m_r[r];
				u16 res;

				if(!shift) {
					m_r[R_PSW] = (m_r[R_PSW] & ~F_MASK) | (v1 ? v1 & 0x8000 ? F_N : 0 : F_Z);
					res = v1;
				} else if(opcode & 0x0800) {
					res = v1 >> shift;
					u16 f = res ? 0 : F_Z;
					if(v1 & (1 << (shift - 1)))
						f |= F_C;
					m_r[R_PSW] = (m_r[R_PSW] & ~F_MASK) | f;
				} else {
					res = v1 << shift;
					u16 f = res ? res & 0x8000 ? F_N : 0 : F_Z;
					if(v1 & (1 << (16-shift)))
						f |= F_C;
					m_r[R_PSW] = (m_r[R_PSW] & ~F_MASK) | f;
				}
				m_r[r] = res;
				break;
			}

			case 1: {
				// Return from interrupt
				// 1101 .... .... .100

				m_r[R_PSW] = m_program.read_word(m_r[R_SP] + 0);
				m_r[R_PC]  = m_program.read_word(m_r[R_SP] + 2);
				m_r[R_SP] += 4;
				m_icount -= 2;
				break;
			}

			case 2: {
				// Bit clear in register
				// 1110 .rrr bbbb .100
				if(m_r[(opcode >> 8) & 7] & (1 << ((opcode >> 4) & 0xf))) {
					m_r[(opcode >> 8) & 7] &= ~(1 << ((opcode >> 4) & 0xf));
					m_r[R_PSW] &= ~F_Z;
				} else
					m_r[R_PSW] |=  F_Z;
				break;
			}

			case 3: {
				// Decrement and branch
				// 1111 .rrr .... .100
				int r = (opcode >> 8) & 7;
				u16 a = m_program_cache.read_word(m_r[R_PC]);
				m_r[R_PC] += 2;

				m_r[r] --;
				if(m_r[r] != 0)
					m_r[R_PC] = a;
				break;
			}
			}
			break;
		}

		case 0x1d: {
			switch((opcode >> 12) & 3) {
			case 0: {
				// Signed shifts
				// 1100 drrr nnnn .101

				int r = (opcode >> 8) & 7;
				int shift = (opcode >> 4) & 0xf;
				u16 v1 = m_r[r];
				u16 res;

				if(!shift) {
					m_r[R_PSW] = (m_r[R_PSW] & ~F_MASK) | (v1 ? v1 & 0x8000 ? F_N : 0 : F_Z);
					res = v1;
				} else if(opcode & 0x0800) {
					res = s16(v1) >> shift;
					u16 f = res ? res & 0x8000 ? F_N : 0 : F_Z;
					if(v1 & (1 << (shift - 1)))
						f |= F_C;
					m_r[R_PSW] = (m_r[R_PSW] & ~F_MASK) | f;
				} else {
					res = v1 << shift;
					u16 f = res ? res & 0x8000 ? F_N : 0 : F_Z;
					if(v1 & (1 << (16-shift)))
						f |= F_C;
					m_r[R_PSW] = (m_r[R_PSW] & ~F_MASK) | f;
				}
				m_r[r] = res;
				break;
			}

			case 1: {
				// Neg/not
				// 1101 .rrr S... .101

				u16 v = m_r[(opcode >> 8) & 7];
				if(opcode & 0x0080)
					v = -s16(v);
				else
					v = ~v;
				m_r[(opcode >> 8) & 7] = v;
				m_r[R_PSW] = (m_r[R_PSW] & ~F_MASK) | (v ? v & 0x8000 ? F_N : 0 : F_Z);
				break;
			}

			case 2: {
				unk(opcode);
				break;
			}

			case 3: {
				// Compare with immediate and branch if equal
				// 1111 .rrr .... .101

				u16 v = m_program_cache.read_word(m_r[R_PC]);
				u16 a = m_program_cache.read_word(m_r[R_PC] + 2);
				m_r[R_PC] += 4;

				do_alu((opcode & 0x07ff) | 0x1000, v);
				if(m_r[R_PSW] & F_Z)
					m_r[R_PC] = a;
				break;
			}
			}
			break;
		}

		case 0x1e: {
			switch((opcode >> 12) & 3) {
			case 0: {
				// Rotate without carry?
				// 1100 drrr nnnn .110
				/*
				From btplay2k flash.u22:
				Surrounding code gives some context to usage here? + opcode is sandwiched between other shift/rotate commands
				6E2C: 7010           r0 = 10
				6E2E: 8817 0001      r0 -= (r1 + 1).bu
				6E32: 0406           beq 6e3a
				6E34: CA1C           r2 >>= 1
				6E36: F004 6E34      dbra r0, 6e34
				...
				6E52: B017 0001      r0 = (r1 + 1).bu
				6E56: CA1E           ?ca1e
				6E58: F004 6E56      dbra r0, 6e56
				*/

				int r = (opcode >> 8) & 7;
				int shift = (opcode >> 4) & 0xf;
				u16 v1 = m_r[r];
				u16 res;

				if(!shift) {
					m_r[R_PSW] = (m_r[R_PSW] & ~F_MASK) | (v1 ? v1 & 0x8000 ? F_N : 0 : F_Z);
					res = v1;
				} else if(opcode & 0x0800) {
					res = (v1 >> shift) | (v1 << (16 - shift));
					u16 f = res ? 0 : F_Z;
					if(v1 & (1 << (shift - 1)))
						f |= F_C;
					m_r[R_PSW] = (m_r[R_PSW] & ~F_MASK) | f;
				} else {
					res = (v1 << shift) | (v1 >> (16 - shift));
					u16 f = res ? res & 0x8000 ? F_N : 0 : F_Z;
					if(v1 & (1 << (16-shift)))
						f |= F_C;
					m_r[R_PSW] = (m_r[R_PSW] & ~F_MASK) | f;
				}
				m_r[r] = res;
				break;
			}

			case 1: {
				unk(opcode);
				break;
			}

			case 2: {
				// Write memory indexed
				// 1110 .rrr .sss w110
				u16 a = m_r[(opcode >> 8) & 7];
				if(opcode & 0x0008)
					m_program.write_word(a, m_r[(opcode >> 4) & 7]);
				else
					m_program.write_byte(a, m_r[(opcode >> 4) & 7]);
				break;
			}

			case 3: {
				unk(opcode);
				break;
			}
			}
			break;
		}

		case 0x1f: {
			switch((opcode >> 12) & 3) {
			case 0: {
				// Rotate through carry
				// 1100 drrr nnnn .111

				int r = (opcode >> 8) & 7;
				int shift = (opcode >> 4) & 0xf;
				u32 v1 = m_r[r] | (m_r[R_PSW] & F_C ? 0x10000 : 0);
				u16 res;

				if(!shift) {
					m_r[R_PSW] = (m_r[R_PSW] & ~(F_N|F_Z|F_V)) | (v1 & 0xffff ? v1 & 0x8000 ? F_N : 0 : F_Z);
					res = v1;
				} else if(opcode & 0x0800) {
					res = (v1 >> shift) | (v1 << (17-shift));
					u16 f = res ? res & 0x8000 ? F_N : 0 : F_Z;
					if(v1 & (1 << (shift - 1)))
						f |= F_C;
					m_r[R_PSW] = (m_r[R_PSW] & ~F_MASK) | f;
				} else {
					res = (v1 << shift) | (v1 >> (17-shift));
					u16 f = res ? res & 0x8000 ? F_N : 0 : F_Z;
					if(v1 & (1 << (16-shift)))
						f |= F_C;
					m_r[R_PSW] = (m_r[R_PSW] & ~F_MASK) | f;
				}
				m_r[r] = res;
				break;
			}

			case 1: {
				unk(opcode);
				break;
			}

			case 2: {
				// Write memory indexed and offset
				// 1110 .rrr .sss w111
				u16 a = m_r[(opcode >> 8) & 7] + m_program_cache.read_word(m_r[R_PC]);
				m_r[R_PC] += 2;
				if(opcode & 0x0008)
					m_program.write_word(a, m_r[(opcode >> 4) & 7]);
				else
					m_program.write_byte(a, m_r[(opcode >> 4) & 7]);
				break;
			}

			case 3: {
				unk(opcode);
				break;
			}
			}
			break;
		}

		default:
			unk(opcode);
			break;
		}

		m_r[R_ZERO] = 0;
	}
}
