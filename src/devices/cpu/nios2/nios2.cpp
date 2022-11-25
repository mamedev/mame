// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Altera Nios II soft processor

    Currently only a rudimentary execution core is provided. No attempt
    has been made yet to emulate correct timings, pipelining, cache
    control, interrupts, trap instructions, etc.

***************************************************************************/

#include "emu.h"
#include "nios2.h"
#include "nios2dasm.h"

// device type definition
DEFINE_DEVICE_TYPE(NIOS2, nios2_device, "nios2", "Altera Nios II Processor")

nios2_device::nios2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: cpu_device(mconfig, NIOS2, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_LITTLE, 32, 32, 0)
{
	std::fill(std::begin(m_gpr), std::end(m_gpr), 0);
	std::fill(std::begin(m_ctl), std::end(m_ctl), 0);
}

std::unique_ptr<util::disasm_interface> nios2_device::create_disassembler()
{
	return std::make_unique<nios2_disassembler>();
}

device_memory_interface::space_config_vector nios2_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config)
	};
}

namespace {

static const char *const s_ctl_names[16] =
{
	"status", "estatus", "bstatus", "ienable", "ipending", "cpuid", "ctl6", "exception",
	"pteaddr", "tlbacc", "tlbmisc", "eccinj", "badaddr", "config", "mpubase", "mpuacc"
};

} // anonymous namespace

void nios2_device::device_start()
{
	space(AS_PROGRAM).specific(m_space);
	space(AS_PROGRAM).cache(m_cache);

	set_icountptr(m_icount);

	state_add(NIOS2_PC, "PC", m_pc);
	state_add(STATE_GENPC, "GENPC", m_pc).noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_pc).noshow();
	state_add<u32>(NIOS2_ZERO, "zero", []() { return 0; }).noshow();
	state_add(NIOS2_AT, "at", m_gpr[1]);
	for (int i = 2; i < 24; i++)
		state_add(NIOS2_R2 + i - 2, util::string_format("r%d", i).c_str(), m_gpr[i]);
	state_add(NIOS2_ET, "et", m_gpr[24]);
	state_add(NIOS2_BT, "bt", m_gpr[25]);
	state_add(NIOS2_GP, "gp", m_gpr[26]);
	state_add(NIOS2_SP, "sp", m_gpr[27]);
	state_add(NIOS2_FP, "fp", m_gpr[28]);
	state_add(NIOS2_EA, "ea", m_gpr[29]);
	state_add(NIOS2_BA, "ba", m_gpr[30]);
	state_add(NIOS2_RA, "ra", m_gpr[31]);
	for (int n = 0; n < 16; n++)
		state_add(NIOS2_STATUS + n, s_ctl_names[n], m_ctl[n]);

	save_item(NAME(m_gpr));
	save_item(NAME(m_ctl));
	save_item(NAME(m_pc));
}

void nios2_device::device_reset()
{
	m_pc = 0;
	m_ctl[0] = 0; // clear status
}

u32 nios2_device::read_ctl(unsigned r) const
{
	if (r < std::size(m_ctl))
		return m_ctl[r];
	else
		return 0;
}

void nios2_device::write_ctl(unsigned r, u32 val)
{
	if (r < std::size(m_ctl))
		m_ctl[r] = val;
}

void nios2_device::execute_run()
{
	do
	{
		debugger_instruction_hook(m_pc);
		u32 inst = m_cache.read_dword(m_pc);

		switch (BIT(inst, 0, 6))
		{
		case 0x00: // call
			m_gpr[31] = m_pc + 4;
			m_pc = (m_pc & 0xf0000000) | (inst >> 4);
			m_icount--;
			break;

		case 0x01: // jmpi
			m_pc = (m_pc & 0xf0000000) | (inst >> 4);
			m_icount--;
			break;

		case 0x03: case 0x23: // ldbu(io)
			m_gpr[BIT(inst, 22, 5)] = m_space.read_byte(get_reg(BIT(inst, 27, 5)) + s16(BIT(inst, 6, 16)));
			m_pc += 4;
			m_icount--;
			break;

		case 0x04: // addi
			m_gpr[BIT(inst, 22, 5)] = get_reg(BIT(inst, 27, 5)) + s16(BIT(inst, 6, 16));
			m_pc += 4;
			m_icount--;
			break;

		case 0x05: case 0x25: // stbio
			m_space.write_byte(get_reg(BIT(inst, 27, 5)) + s16(BIT(inst, 6, 16)), get_reg(BIT(inst, 22, 5)) & 0xff);
			m_pc += 4;
			m_icount--;
			break;

		case 0x06: // br
			m_pc += 4 + s32(s16(BIT(inst, 6, 16)));
			m_icount--;
			break;

		case 0x07: case 0x27: // ldb(io)
			m_gpr[BIT(inst, 22, 5)] = s32(s8(m_space.read_byte(get_reg(BIT(inst, 27, 5)) + s16(BIT(inst, 6, 16)))));
			m_pc += 4;
			m_icount--;
			break;

		case 0x08: // cmpgei
			m_gpr[BIT(inst, 22, 5)] = s32(get_reg(BIT(inst, 27, 5))) >= s16(BIT(inst, 6, 16)) ? 1 : 0;
			m_pc += 4;
			m_icount--;
			break;

		case 0x0b: case 0x2b: // ldhu(io)
			m_gpr[BIT(inst, 22, 5)] = m_space.read_word(get_reg(BIT(inst, 27, 5)) + s16(BIT(inst, 6, 16)));
			m_pc += 4;
			m_icount--;
			break;

		case 0x0c: // andi
			m_gpr[BIT(inst, 22, 5)] = get_reg(BIT(inst, 27, 5)) & BIT(inst, 6, 16);
			m_pc += 4;
			m_icount--;
			break;

		case 0x0d: case 0x2d: // sth(io)
			m_space.write_word(get_reg(BIT(inst, 27, 5)) + s16(BIT(inst, 6, 16)), get_reg(BIT(inst, 22, 5)) & 0xffff);
			m_pc += 4;
			m_icount--;
			break;

		case 0x0e: // bge
			if (s32(get_reg(BIT(inst, 27, 5))) >= s32(get_reg(BIT(inst, 22, 5))))
				m_pc += 4 + s32(s16(BIT(inst, 6, 16)));
			else
				m_pc += 4;
			m_icount--;
			break;

		case 0x0f: case 0x2f: // ldh(io)
			m_gpr[BIT(inst, 22, 5)] = s32(s16(m_space.read_word(get_reg(BIT(inst, 27, 5)) + s16(BIT(inst, 6, 16)))));
			m_pc += 4;
			m_icount--;
			break;

		case 0x10: // cmplti
			m_gpr[BIT(inst, 22, 5)] = s32(get_reg(BIT(inst, 27, 5))) < s16(BIT(inst, 6, 16)) ? 1 : 0;
			m_pc += 4;
			m_icount--;
			break;

		case 0x14: // ori
			m_gpr[BIT(inst, 22, 5)] = get_reg(BIT(inst, 27, 5)) | BIT(inst, 6, 16);
			m_pc += 4;
			m_icount--;
			break;

		case 0x15: case 0x35: // stw(io)
			m_space.write_dword(get_reg(BIT(inst, 27, 5)) + s16(BIT(inst, 6, 16)), get_reg(BIT(inst, 22, 5)));
			m_pc += 4;
			m_icount--;
			break;

		case 0x16: // blt
			if (s32(get_reg(BIT(inst, 27, 5))) < s32(get_reg(BIT(inst, 22, 5))))
				m_pc += 4 + s32(s16(BIT(inst, 6, 16)));
			else
				m_pc += 4;
			m_icount--;
			break;

		case 0x17: case 0x37: // ldw(io)
			m_gpr[BIT(inst, 22, 5)] = m_space.read_dword(get_reg(BIT(inst, 27, 5)) + s16(BIT(inst, 6, 16)));
			m_pc += 4;
			m_icount--;
			break;

		case 0x18: // cmpnei
			m_gpr[BIT(inst, 22, 5)] = get_reg(BIT(inst, 27, 5)) != s32(s16(BIT(inst, 6, 16))) ? 1 : 0;
			m_pc += 4;
			m_icount--;
			break;

		case 0x1c: // xori
			m_gpr[BIT(inst, 22, 5)] = get_reg(BIT(inst, 27, 5)) ^ BIT(inst, 6, 16);
			m_pc += 4;
			m_icount--;
			break;

		case 0x1e: // bne
			if (get_reg(BIT(inst, 27, 5)) != get_reg(BIT(inst, 22, 5)))
				m_pc += 4 + s32(s16(BIT(inst, 6, 16)));
			else
				m_pc += 4;
			m_icount--;
			break;

		case 0x20: // cmpeqi
			m_gpr[BIT(inst, 22, 5)] = get_reg(BIT(inst, 27, 5)) == s32(s16(BIT(inst, 6, 16))) ? 1 : 0;
			m_pc += 4;
			m_icount--;
			break;

		case 0x24: // muli
			m_gpr[BIT(inst, 22, 5)] = s32(get_reg(BIT(inst, 27, 5))) * s16(BIT(inst, 6, 16));
			m_pc += 4;
			m_icount--;
			break;

		case 0x26: // beq
			if (get_reg(BIT(inst, 27, 5)) == get_reg(BIT(inst, 22, 5)))
				m_pc += 4 + s32(s16(BIT(inst, 6, 16)));
			else
				m_pc += 4;
			m_icount--;
			break;

		case 0x28: // cmpgeui
			m_gpr[BIT(inst, 22, 5)] = get_reg(BIT(inst, 27, 5)) >= BIT(inst, 6, 16) ? 1 : 0;
			m_pc += 4;
			m_icount--;
			break;

		case 0x2c: // andhi
			m_gpr[BIT(inst, 22, 5)] = get_reg(BIT(inst, 27, 5)) & (BIT(inst, 6, 16) << 16);
			m_pc += 4;
			m_icount--;
			break;

		case 0x2e: // bgeu
			if (get_reg(BIT(inst, 27, 5)) >= get_reg(BIT(inst, 22, 5)))
				m_pc += 4 + s32(s16(BIT(inst, 6, 16)));
			else
				m_pc += 4;
			m_icount--;
			break;

		case 0x30: // cmpltui
			m_gpr[BIT(inst, 22, 5)] = get_reg(BIT(inst, 27, 5)) < BIT(inst, 6, 16) ? 1 : 0;
			m_pc += 4;
			m_icount--;
			break;

		case 0x33: // initd
			logerror("initd 0x%08X\n", get_reg(BIT(inst, 27, 5) + s16(BIT(inst, 6, 16))));
			m_pc += 4;
			m_icount--;
			break;

		case 0x34: // orhi
			m_gpr[BIT(inst, 22, 5)] = get_reg(BIT(inst, 27, 5)) | (BIT(inst, 6, 16) << 16);
			m_pc += 4;
			m_icount--;
			break;

		case 0x36: // bltu
			if (get_reg(BIT(inst, 27, 5)) < get_reg(BIT(inst, 22, 5)))
				m_pc += 4 + s32(s16(BIT(inst, 6, 16)));
			else
				m_pc += 4;
			m_icount--;
			break;

		case 0x3a:
			switch (BIT(inst, 11, 6))
			{
			case 0x01: // eret
			case 0x05: // ret
			case 0x09: // bret
			case 0x0d: // jmp
				m_pc = get_reg(BIT(inst, 27, 5));
				m_icount--;
				break;

			case 0x02: // roli
				m_gpr[BIT(inst, 17, 5)] = rotl_32(get_reg(BIT(inst, 27, 5)), BIT(inst, 6, 5));
				m_pc += 4;
				m_icount--;
				break;

			case 0x03: // rol
				m_gpr[BIT(inst, 17, 5)] = rotl_32(get_reg(BIT(inst, 27, 5)), get_reg(BIT(inst, 22, 5)));
				m_pc += 4;
				m_icount--;
				break;

			case 0x04: // flushp
				logerror("flushp instruction encountered (PC = 0x%08X)\n", m_pc);
				m_pc += 4;
				m_icount--;
				break;

			case 0x06: // nor
				m_gpr[BIT(inst, 17, 5)] = ~(get_reg(BIT(inst, 27, 5)) | get_reg(BIT(inst, 22, 5)));
				m_pc += 4;
				m_icount--;
				break;

			case 0x07: // mulxuu
				m_gpr[BIT(inst, 17, 5)] = mulu_32x32_hi(get_reg(BIT(inst, 27, 5)), get_reg(BIT(inst, 22, 5)));
				m_pc += 4;
				m_icount--;
				break;

			case 0x08: // cmpge
				m_gpr[BIT(inst, 17, 5)] = s32(get_reg(BIT(inst, 27, 5))) >= s32(get_reg(BIT(inst, 22, 5))) ? 1 : 0;
				m_pc += 4;
				m_icount--;
				break;

			case 0x0b: // ror
				m_gpr[BIT(inst, 17, 5)] = rotr_32(get_reg(BIT(inst, 27, 5)), get_reg(BIT(inst, 22, 5)));
				m_pc += 4;
				m_icount--;
				break;

			case 0x0c: // flushi
				logerror("flushi 0x%08X (PC = 0x%08X)\n", get_reg(BIT(inst, 27, 5)), m_pc);
				m_pc += 4;
				m_icount--;
				break;

			case 0x0e: // and
				m_gpr[BIT(inst, 17, 5)] = get_reg(BIT(inst, 27, 5)) & get_reg(BIT(inst, 22, 5));
				m_pc += 4;
				m_icount--;
				break;

			case 0x10: // cmplt
				m_gpr[BIT(inst, 17, 5)] = s32(get_reg(BIT(inst, 27, 5))) < s32(get_reg(BIT(inst, 22, 5))) ? 1 : 0;
				m_pc += 4;
				m_icount--;
				break;

			case 0x12: // slli
				m_gpr[BIT(inst, 17, 5)] = get_reg(BIT(inst, 27, 5)) << BIT(inst, 6, 5);
				m_pc += 4;
				m_icount--;
				break;

			case 0x13: // sll
				m_gpr[BIT(inst, 17, 5)] = get_reg(BIT(inst, 27, 5)) << (get_reg(BIT(inst, 22, 5)) & 0x1f);
				m_pc += 4;
				m_icount--;
				break;

			case 0x17: // mulxsu
				m_gpr[BIT(inst, 17, 5)] = (s64(s32(get_reg(BIT(inst, 27, 5)))) * u64(get_reg(BIT(inst, 22, 5)))) >> 32;
				m_pc += 4;
				m_icount--;
				break;

			case 0x18: // cmpne
				m_gpr[BIT(inst, 17, 5)] = get_reg(BIT(inst, 27, 5)) != get_reg(BIT(inst, 22, 5)) ? 1 : 0;
				m_pc += 4;
				m_icount--;
				break;

			case 0x16: // or
				m_gpr[BIT(inst, 17, 5)] = get_reg(BIT(inst, 27, 5)) & get_reg(BIT(inst, 22, 5));
				m_pc += 4;
				m_icount--;
				break;

			case 0x1a: // srli
				m_gpr[BIT(inst, 17, 5)] = get_reg(BIT(inst, 27, 5)) >> BIT(inst, 6, 5);
				m_pc += 4;
				m_icount--;
				break;

			case 0x1b: // srl
				m_gpr[BIT(inst, 17, 5)] = get_reg(BIT(inst, 27, 5)) >> (get_reg(BIT(inst, 22, 5)) & 0x1f);
				m_pc += 4;
				m_icount--;
				break;

			case 0x1c: // nextpc
				m_gpr[BIT(inst, 17, 5)] = m_pc += 4;
				m_icount--;
				break;

			case 0x1d: // callr
				m_gpr[BIT(inst, 17, 5)] = m_pc + 4;
				m_pc = get_reg(BIT(inst, 27, 5));
				m_icount--;
				break;

			case 0x1e: // xor
				m_gpr[BIT(inst, 17, 5)] = get_reg(BIT(inst, 27, 5)) ^ get_reg(BIT(inst, 22, 5));
				m_pc += 4;
				m_icount--;
				break;

			case 0x1f: // mulxss
				m_gpr[BIT(inst, 17, 5)] = mul_32x32_hi(get_reg(BIT(inst, 27, 5)), get_reg(BIT(inst, 22, 5)));
				m_pc += 4;
				m_icount--;
				break;

			case 0x20: // cmpeq
				m_gpr[BIT(inst, 17, 5)] = get_reg(BIT(inst, 27, 5)) == get_reg(BIT(inst, 22, 5)) ? 1 : 0;
				m_pc += 4;
				m_icount--;
				break;

			case 0x24: // divu
				if (u32 divisor = get_reg(BIT(inst, 22, 5)); divisor != 0)
					m_gpr[BIT(inst, 17, 5)] = get_reg(BIT(inst, 27, 5)) / divisor;
				else
				{
					logerror("Divide by 0 at PC = 0x%08X\n", m_pc);
					m_gpr[BIT(inst, 17, 5)] = 0xffffffff; // result is undefined
				}
				m_pc += 4;
				m_icount--;
				break;

			case 0x25: // div
				if (s32 divisor = get_reg(BIT(inst, 22, 5)); divisor != 0)
					m_gpr[BIT(inst, 17, 5)] = s32(get_reg(BIT(inst, 27, 5)) / divisor);
				else
				{
					logerror("Divide by 0 at PC = 0x%08X\n", m_pc);
					m_gpr[BIT(inst, 17, 5)] = 0xffffffff; // result is undefined
				}
				m_pc += 4;
				m_icount--;
				break;

			case 0x26: // rdctl
				m_gpr[BIT(inst, 17, 5)] = read_ctl(BIT(inst, 6, 5));
				m_pc += 4;
				m_icount--;
				break;

			case 0x27: // mul
				m_gpr[BIT(inst, 17, 5)] = get_reg(BIT(inst, 27, 5)) * get_reg(BIT(inst, 22, 5));
				m_pc += 4;
				m_icount--;
				break;

			case 0x28: // cmpgeu
				m_gpr[BIT(inst, 17, 5)] = get_reg(BIT(inst, 27, 5)) >= get_reg(BIT(inst, 22, 5)) ? 1 : 0;
				m_pc += 4;
				m_icount--;
				break;

			case 0x29: // initi
				logerror("initi 0x%08X (PC = 0x%08X)\n", get_reg(BIT(inst, 27, 5)), m_pc);
				m_pc += 4;
				m_icount--;
				break;

			case 0x2e: // wrctl
				write_ctl(BIT(inst, 6, 5), get_reg(BIT(inst, 27, 5)));
				m_pc += 4;
				m_icount--;
				break;

			case 0x30: // cmpltu
				m_gpr[BIT(inst, 17, 5)] = get_reg(BIT(inst, 27, 5)) < get_reg(BIT(inst, 22, 5)) ? 1 : 0;
				m_pc += 4;
				m_icount--;
				break;

			case 0x31: // add
				m_gpr[BIT(inst, 17, 5)] = get_reg(BIT(inst, 27, 5)) + get_reg(BIT(inst, 22, 5));
				m_pc += 4;
				m_icount--;
				break;

			case 0x36: // sync
				logerror("sync instruction encountered (PC = 0x%08X)\n", m_pc);
				m_pc += 4;
				m_icount--;
				break;

			case 0x39: // sub
				m_gpr[BIT(inst, 17, 5)] = get_reg(BIT(inst, 27, 5)) - get_reg(BIT(inst, 22, 5));
				m_pc += 4;
				m_icount--;
				break;

			case 0x3a: // srai
				m_gpr[BIT(inst, 17, 5)] = s32(get_reg(BIT(inst, 27, 5))) >> BIT(inst, 6, 5);
				m_pc += 4;
				m_icount--;
				break;

			case 0x3b: // sra
				m_gpr[BIT(inst, 17, 5)] = s32(get_reg(BIT(inst, 27, 5))) >> (get_reg(BIT(inst, 22, 5)) & 0x1f);
				m_pc += 4;
				m_icount--;
				break;

			default:
				logerror("Unknown/invalid R-type instruction word encountered (OPX = 0x%02X, PC = 0x%08X)\n", BIT(inst, 11, 6), m_pc);
				m_pc += 4;
				m_icount--;
				break;
			}
			break;

		case 0x3c: // xorhi
			m_gpr[BIT(inst, 22, 5)] = get_reg(BIT(inst, 27, 5)) ^ (BIT(inst, 6, 16) << 16);
			m_pc += 4;
			m_icount--;
			break;

		default:
			logerror("Unknown/invalid instruction word encountered (OP = 0x%02X, PC = 0x%08X)\n", BIT(inst, 0, 6), m_pc);
			m_pc += 4;
			m_icount--;
			break;
		}
	} while (m_icount > 0);
}
