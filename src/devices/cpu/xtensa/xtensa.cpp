// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Tensilica Xtensa

    Currently this device is just a stub with no actual execution core.

***************************************************************************/

#include "emu.h"
#include "xtensa.h"
#include "xtensad.h"

#include "xtensa_tables.h"

#define VERBOSE 1
#include "logmacro.h"

// device type definitions
DEFINE_DEVICE_TYPE(XTENSA, xtensa_device, "xtensa", "Tensilica Xtensa core")

xtensa_device::xtensa_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: cpu_device(mconfig, XTENSA, tag, owner, clock)
	, m_space_config("program", ENDIANNESS_LITTLE, 32, 32, 0)
	, m_pc(0)
{
}

std::unique_ptr<util::disasm_interface> xtensa_device::create_disassembler()
{
	return std::make_unique<xtensa_disassembler>();
}

device_memory_interface::space_config_vector xtensa_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_space_config),
	};
}

void xtensa_device::device_start()
{
	space(AS_PROGRAM).cache(m_cache);
	space(AS_PROGRAM).specific(m_space);

	std::fill(std::begin(m_a), std::end(m_a), 0);

	set_icountptr(m_icount);

	state_add(XTENSA_PC, "PC", m_pc);
	state_add(STATE_GENPC, "GENPC", m_pc);
	state_add(STATE_GENPCBASE, "CURPC", m_pc);
	for (int i = 0; i < 16; i++)
		state_add(XTENSA_A0 + i, string_format("a%d", i).c_str(), m_a[i]);

	save_item(NAME(m_pc));
	save_item(NAME(m_a));
}

void xtensa_device::device_reset()
{
	// TODO: Reset state
}

void xtensa_device::handle_reserved(u32 inst)
{
	LOG("%-8s0x%02X ; reserved\n", "db", inst & 0xff);
	m_nextpc = m_pc + 1;
}

void xtensa_device::getop_and_execute()
{
	m_nextpc = m_pc + 2;
	u32 inst = m_cache.read_byte(m_pc);
	inst |= m_cache.read_byte(m_pc+1)<<8;

	const u8 op0 = BIT(inst, 0, 4);
	if (op0 < 0b1000)
	{
		inst |= u32(m_cache.read_byte(m_pc+2)) << 16;
		m_nextpc = m_pc + 3;
	}

	switch (op0)
	{
	case 0b0000: // QRST
		switch (BIT(inst, 16, 4))
		{
		case 0b0000: // RST0
			switch (BIT(inst, 20, 4))
			{
			case 0b0000: // ST0
				switch (BIT(inst, 12, 4))
				{
				case 0b0000: // SNM0
					switch (BIT(inst, 4, 4))
					{
					case 0b0000: // ILL
						LOG("ill\n");
						break;

					case 0b1000: // RET
						LOG("ret\n");
						break;

					case 0b1001: // RETW (with Windowed Register Option)
						LOG("retw\n");
						break;

					case 0b1010: // JX
						LOG("%-8sa%d\n", "jx", BIT(inst, 8, 4));
						break;

					case 0b1100: // CALLX0
					case 0b1101: case 0b1110: case 0b1111: // CALLX4, CALLX8, CALLX12 (with Windowed Register Option)
						LOG("callx%-3da%d\n", BIT(inst, 4, 2) * 4, BIT(inst, 8, 4));
						break;

					default:
						handle_reserved(inst);
						break;
					}
					break;

				case 0b0001: // MOVSP (with Windowed Register Option)
					LOG("%-8sa%d, a%d", "movsp\n", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b0010: // SYNC
					switch (BIT(inst, 4, 8))
					{
					case 0b00000000: // ISYNC
						LOG("isync\n");
						break;

					case 0b00000001: // RSYNC
						LOG("rsync\n");
						break;

					case 0b00000010: // ESYNC
						LOG("esync\n");
						break;

					case 0b00000011: // DSYNC
						LOG("dsync\n");
						break;

					case 0b00001000: // EXCW (with Exception Option)
						LOG("excw\n");
						break;

					case 0b00001100: // MEMW
						LOG("memw\n");
						break;

					case 0b00001101: // EXTW (added in RA-2004.1)
						LOG("extw\n");
						break;

					case 0b00001111: // NOP (added in RA-2004.1; was assembly macro previously)
						LOG("nop\n");
						break;

					default:
						handle_reserved(inst);
						break;
					}
					break;

				case 0b0011: // RFEI
					switch (BIT(inst, 4, 4))
					{
					case 0b0000: // RFET
						switch (BIT(inst, 8, 4))
						{
						case 0b0000: // RFE (with Exception Option)
							LOG("rfe\n");
							break;

						case 0b0001: // RFUE (with Exception Option; XEA1 only)
							LOG("rfue\n");
							break;

						case 0b0010: // RFDE (with Exception Option)
							LOG("rfde\n");
							break;

						case 0b0100: // RFWO (with Windowed Register option)
							LOG("rfwo\n");
							break;

						case 0b0101: // RFWU (with Windowed Register option)
							LOG("rfwu\n");
							break;

						default:
							handle_reserved(inst);
							break;
						}
						break;

					case 0b0001: // RFI (with High-Priority Interrupt Option)
						LOG("%-8s%d\n", "rfi", BIT(inst, 8, 4));
						break;

					case 0b0010: // RFME (with Memory ECC/Parity Option)
						LOG("rfme\n");
						break;

					default:
						handle_reserved(inst);
						break;
					}
					break;

				case 0b0100: // BREAK (with Debug Option)
					LOG("%-8s%d, %d\n", "break", BIT(inst, 8, 4), BIT(inst, 4, 4));
					break;

				case 0b0101: // SYSCALL (with Exception Option)
					LOG("syscall\n");
					break;

				case 0b0110: // RSIL (with Interrupt Option)
					LOG("%-8sa%d, %d\n", "rsil", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b0111: // WAITI (with Interrupt Option)
					LOG("%-8s%d\n", "waiti", BIT(inst, 8, 4));
					break;

				case 0b1000: // ANY4 (with Boolean Option)
					LOG("%-8sb%d, b%d\n", "any4", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b1001: // ALL4 (with Boolean Option)
					LOG("%-8sb%d, b%d\n", "all4", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b1010: // ANY8 (with Boolean Option)
					LOG("%-8sb%d, b%d\n", "any8", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b1011: // ALL8 (with Boolean Option)
					LOG("%-8sb%d, b%d\n", "all8", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				default:
					handle_reserved(inst);
					break;
				}
				break;

			case 0b0001: // AND
				LOG("%-8sa%d, a%d, a%d\n", "and", BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b0010: // OR
				if (BIT(inst, 8, 4) == BIT(inst, 4, 4))
				{
					LOG("%-8sa%d, a%d\n", "mov", BIT(inst, 12, 4), BIT(inst, 8, 4));
				}
				else
				{
					LOG("%-8sa%d, a%d, a%d\n", "or", BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				}
				break;

			case 0b0011: // XOR
				LOG("%-8sa%d, a%d, a%d\n", "xor", BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b0100: // ST1
				switch (BIT(inst, 12, 4))
				{
				case 0b0000: case 0b0001: case 0b0010: case 0b0011: // SSR, SSL, SSA8L, SSA8B
					LOG("%-8sa%d\n", s_st1_ops[BIT(inst, 12, 4)], BIT(inst, 8, 4));
					break;

				case 0b0100: // SSAI
					LOG("%-8s%d\n", "ssai", BIT(inst, 8, 4) + (inst & 0x000010));
					break;

				case 0b0110: case 0b0111: // RER, WER
				case 0b1110: case 0b1111: // NSA, NSAU (with Miscellaneous Operations Option)
					LOG("%-8sa%d, a%d\n", s_st1_ops[BIT(inst, 12, 4)], BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b1000: // ROTW (with Windowed Register Option)
					LOG("%-8s%d\n", "rotw", util::sext(inst >> 4, 4));
					break;

				default:
					handle_reserved(inst);
					break;
				}
				break;

			case 0b0101: // TLB (with Region Translation Option or MMU Option)
				switch (BIT(inst, 12, 4))
				{
				case 0b0011: case 0b0101: case 0b0110: case 0b0111: // RITLB0, PITLB, WITLB, RITLB1
				case 0b1011: case 0b1101: case 0b1110: case 0b1111: // RDTLB0, PDTLB, WDTLB, RDTLB1
					LOG("%-8sa%d, a%d\n", s_tlb_ops[BIT(inst, 12, 4)], BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b0100: case 0b1100: // IITLB, IDTLB
					LOG("%-8sa%d\n", s_tlb_ops[BIT(inst, 12, 4)], BIT(inst, 8, 4));
					break;

				default:
					handle_reserved(inst);
					break;
				}
				break;

			case 0b0110: // RT0
				switch (BIT(inst, 8, 4))
				{
				case 0b0000: // NEG
					LOG("%-8sa%d, a%d\n", "neg", BIT(inst, 12, 4), BIT(inst, 4, 4));
					break;

				case 0b0001: // ABS
					LOG("%-8sa%d, a%d\n", "abs", BIT(inst, 12, 4), BIT(inst, 4, 4));
					break;

				default:
					handle_reserved(inst);
					break;
				}
				break;

			case 0b1000: case 0b1100: // ADD, SUB
				LOG("%-8sa%d, a%d, a%d\n", BIT(inst, 22) ? "sub" : "add", BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b1001: case 0b1010: case 0b1011: // ADDX2, ADDX4, ADDX8
			case 0b1101: case 0b1110: case 0b1111: // SUBX2, SUBX4, SUBX8
				LOG("%sx%-4da%d, a%d, a%d\n", BIT(inst, 22) ? "sub" : "add", 1 << BIT(inst, 20, 2), BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			default:
				handle_reserved(inst);
				break;
			}
			break;

		case 0b0001: // RST1
			switch (BIT(inst, 20, 4))
			{
			case 0b0000: case 0b0001: // SLLI (shift count is 0..31)
				LOG("%-8sa%d, a%d, %d\n", "slli", BIT(inst, 12, 4), BIT(inst, 4, 4), BIT(inst, 8, 4) + (BIT(inst, 20) ? 16 : 0));
				break;

			case 0b0010: case 0b0011: // SRAI (shift count is 0..31)
				LOG("%-8sa%d, a%d, %d\n", "srai", BIT(inst, 12, 4), BIT(inst, 4, 4), BIT(inst, 8, 4) + (BIT(inst, 20) ? 16 : 0));
				break;

			case 0b0100: // SRLI (shift count is 0..15)
				LOG("%-8sa%d, a%d, %d\n", "srli", BIT(inst, 12, 4), BIT(inst, 4, 4), BIT(inst, 8, 4));
				break;

			case 0b0110: // XSR (added in T1040)
				LOG("xsr.%-3s a%d\n", special_reg(BIT(inst, 8, 8), true), BIT(inst, 4, 4));
				break;

			case 0b0111: // ACCER (added in RC-2009.0)
				switch (BIT(inst, 20, 4))
				{
				case 0b0000: // RER
					LOG("%-8sa%d, a%d\n", "rer", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b1000: // WER
					LOG("%-8sa%d, a%d\n", "wer", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				default:
					handle_reserved(inst);
					break;
				}
				break;

			case 0b1000: // SRC
				LOG("%-8sa%d, a%d, a%d\n", "src", BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b1001: // SRL
				LOG("%-8sa%d, a%d\n", "srl", BIT(inst, 12, 4), BIT(inst, 4, 4));
				break;

			case 0b1010: // SLL
				LOG("%-8sa%d, a%d\n", "sll", BIT(inst, 12, 4), BIT(inst, 8, 4));
				break;

			case 0b1011: // SRA
				LOG("%-8sa%d, a%d\n", "sra", BIT(inst, 12, 4), BIT(inst, 4, 4));
				break;

			case 0b1100: // MUL16U (with 16-bit Integer Multiply Option)
				LOG("%-8sa%d, a%d, a%d\n", "mul16u", BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b1101: // MUL16S (with 16-bit Integer Multiply Option)
				LOG("%-8sa%d, a%d, a%d\n", "mul16s", BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b1111: // IMP (Implementation-Specific)
				switch (BIT(inst, 12, 4))
				{
				case 0b0000: // LICT (with Instruction Cache Test Option)
					LOG("%-8sa%d, a%d\n", "lict", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b0001: // SICT (with Instruction Cache Test Option)
					LOG("%-8sa%d, a%d\n", "sict", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b0010: // LICW (with Instruction Cache Test Option)
					LOG("%-8sa%d, a%d\n", "licw", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b0011: // SICW (with Instruction Cache Test Option)
					LOG("%-8sa%d, a%d\n", "sicw", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b1000: // LDCT (with Data Cache Test Option)
					LOG("%-8sa%d, a%d\n", "ldct", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b1001: // SDCT (with Data Cache Test Option)
					LOG("%-8sa%d, a%d\n", "sdct", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b1110: // RFDX (with On-Chip Debug)
					switch (BIT(inst, 4, 4))
					{
					case 0b0000: // RFDO
						LOG("rfdo\n");
						break;

					case 0b0001: // RFDD
						LOG("rfdd\n");
						break;

					default:
						handle_reserved(inst);
						break;
					}
					break;

				default:
					handle_reserved(inst);
					break;
				}
				break;

			default:
				handle_reserved(inst);
				break;
			}
			break;

		case 0b0010: // RST2
			switch (BIT(inst, 20, 4))
			{
			case 0b0000: case 0b0001: case 0b0010: case 0b0011: case 0b0100: // ANDB, ANDBC, ORB, ORBC, XORB (with Boolean Option)
				LOG("%-8sb%d, b%d, b%d\n", s_rst2_ops[BIT(inst, 20, 4)], BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b1000: case 0b1010: case 0b1011: // MULL, MULUH, MULSH (with 32-bit Integer Multiply Option)
			case 0b1100: case 0b1101: case 0b1110: case 0b1111: // QUOU, QUOS, REMU, REMS (with 32-bit Integer Divide Option)
				LOG("%-8sa%d, a%d, a%d\n", s_rst2_ops[BIT(inst, 20, 4)], BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			default:
				handle_reserved(inst);
				break;
			}
			break;

		case 0b0011: // RST3
			switch (BIT(inst, 20, 4))
			{
			case 0b0000: case 0b0001: // RSR, WSR
				LOG("%s.%-3d a%d\n", s_rst3_ops[BIT(inst, 20, 4)], special_reg(BIT(inst, 8, 8), BIT(inst, 20)), BIT(inst, 4, 4));
				break;

			case 0b0010: case 0b0011: // SEXT, CLAMPS (with Miscellaneous Operations Option)
				LOG("%-8sa%d, a%d, %d\n", s_rst3_ops[BIT(inst, 20, 4)], BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4) + 7);
				break;

			case 0b0100: case 0b0101: case 0b0110: case 0b0111: // MIN, MAX, MINU, MAXU (with Miscellaneous Operations Option)
			case 0b1000: case 0b1001: case 0b1010: case 0b1011: // MOVEQZ, MOVNEZ, MOVLTZ, MOVGEZ
				LOG("%-8sa%d, a%d, a%d\n", s_rst3_ops[BIT(inst, 20, 4)], BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b1100: case 0b1101: // MOVF, MOVT (with Boolean Option)
				LOG("%-8sa%d, a%d, b%d\n", s_rst3_ops[BIT(inst, 20, 4)], BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b1110: case 0b1111: // RUR, WUR (TODO: TIE user_register names)
				LOG("%s.u%-2d a%d\n", s_rst3_ops[BIT(inst, 20, 4)], BIT(inst, 4, 8), BIT(inst, 12, 4));
				break;
			}
			break;

		case 0b0100: case 0b0101: // EXTUI
			LOG("%-8sa%d, a%d, %d, %d\n", "extui", BIT(inst, 12, 4), BIT(inst, 4, 4), BIT(inst, 8, 4) + (BIT(inst, 16) ? 16 : 0), BIT(inst, 20, 4) + 1);
			break;

		case 0b0110: case 0b0111: // CUST0, CUST1
			LOG("%-8s0x%02X ; cust%d?\n", "db", inst & 0xff, BIT(inst, 16));
			m_nextpc = m_pc + 1;
			break;

		case 0b1000: // LSCX (with Floating-Point Coprocessor Option)
			switch (BIT(inst, 20, 4))
			{
			case 0b0000: // LSX
				LOG("%-8sf%d, a%d, a%d\n", "lsx", BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b0001: // LSXU
				LOG("%-8sf%d, a%d, a%d\n", "lsxu", BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b0100: // SSX
				LOG("%-8sf%d, a%d, a%d\n", "ssx", BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b0101: // SSXU
				LOG("%-8sf%d, a%d, a%d\n", "ssxu", BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			default:
				handle_reserved(inst);
				break;
			}
			break;

		case 0b1001: // LSC4 (with Windowed Register Option)
			switch (BIT(inst, 20, 4))
			{
			case 0b0000: // L32E
				LOG("%-8sa%d, a%d, %s\n", "l32e", BIT(inst, 4, 4), BIT(inst, 8, 4), format_imm(int(BIT(inst, 12, 4)) * 4 - 64));
				break;

			case 0b0100: // S32E
				LOG("%-8sa%d, a%d, %s\n", "s32e", BIT(inst, 4, 4), BIT(inst, 8, 4), format_imm(int(BIT(inst, 12, 4)) * 4 - 64));
				break;

			default:
				handle_reserved(inst);
				break;
			}
			break;

		case 0b1010: // FP0 (with Floating-Point Coprocessor Option)
			switch (BIT(inst, 20, 4))
			{
			case 0b0000: case 0b0001: case 0b0010: case 0b0100: case 0b0101: // ADD.S, SUB.S, MUL.S, MADD.S, MSUB.S
				LOG("%-8sf%d, f%d, f%d\n", s_fp0_ops[BIT(inst, 20, 4)], BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b1000: case 0b1001: case 0b1010: case 0b1011: case 0b1110: // ROUND.S, TRUNC.S, FLOOR.S, CEIL.S, UTRUNC.S
				LOG("%-7s a%d, f%d, %d\n", s_fp0_ops[BIT(inst, 20, 4)], BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b1100: case 0b1101: // FLOAT.S, UFLOAT.S
				LOG("%-7s f%d, a%d, %d\n", s_fp0_ops[BIT(inst, 20, 4)], BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b1111: // FP1OP
				switch (BIT(inst, 4, 4))
				{
				case 0b0000: // MOV.S
					LOG("%-8sf%d, f%d\n", "mov.s", BIT(inst, 12, 4), BIT(inst, 8, 4));
					break;

				case 0b0001: // ABS.S
					LOG("%-8sf%d, f%d\n", "abs.s", BIT(inst, 12, 4), BIT(inst, 8, 4));
					break;

				case 0b0100: // RFR
					LOG("%-8sa%d, f%d\n", "rfr", BIT(inst, 12, 4), BIT(inst, 8, 4));
					break;

				case 0b0101: // WFR
					LOG("%-8sf%d, a%d\n", "wfr", BIT(inst, 12, 4), BIT(inst, 8, 4));
					break;

				case 0b0110: // NEG.S
					LOG("%-8sf%d, f%d\n", "neg.s", BIT(inst, 12, 4), BIT(inst, 8, 4));
					break;

				default:
					handle_reserved(inst);
					break;
				}
				break;

			default:
				handle_reserved(inst);
				break;
			}
			break;

		case 0b1011: // FP1 (with Floating-Point Option)
			switch (BIT(inst, 20, 4))
			{
			case 0b0001: case 0b0010: case 0b0011: case 0b0100: case 0b0101: case 0b0110: case 0b0111: // UN.S, OEQ.S, UEQ.S, OLT.S, ULT.S, OLE.S, ULE.S
				LOG("%-8sb%d, f%d, f%d\n", s_fp1_ops[BIT(inst, 20, 4)], BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b1000: case 0b1001: case 0b1010: case 0b1011: // MOVEQZ.S, MOVNEZ.S, MOVLTZ.S, MOVGEZ.S
				LOG("%-8sf%d, f%d, a%d\n", s_fp1_ops[BIT(inst, 20, 4)], BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b1100: case 0b1101: // MOVF.S, MOVT.S
				LOG("%-8sf%d, f%d, b%d\n", s_fp1_ops[BIT(inst, 20, 4)], BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			default:
				handle_reserved(inst);
				break;
			}
			break;

		default:
			handle_reserved(inst);
			break;
		}
		break;

	case 0b0001: // L32R (virtual address is always aligned)
		LOG("%-8sa%d, 0x%08X\n", "l32r", BIT(inst, 4, 4), (m_pc + 3 - 0x40000 + (inst >> 8) * 4) & 0xfffffffc);
		break;

	case 0b0010: // LSAI
		switch (BIT(inst, 12, 4))
		{
		case 0b0000: case 0b0100: // L8UI, S8I
			LOG("%-8sa%d, a%d, %s\n", s_lsai_ops[BIT(inst, 12, 4)], BIT(inst, 4, 4), BIT(inst, 8, 4), format_imm(inst >> 16));
			break;

		case 0b0001: case 0b0101: case 0b1001: // L16UI, S16I, L16SI
			LOG("%-8sa%d, a%d, %s\n", s_lsai_ops[BIT(inst, 12, 4)], BIT(inst, 4, 4), BIT(inst, 8, 4), format_imm((inst >> 16) * 2));
			break;

		case 0b0010: case 0b0110: // L32I, S32I
		case 0b1011: case 0b1111: // L32AI, S32RI (with Multiprocessor Synchronization Option)
		case 0b1110: // S32C1I (with Conditional Store Option)
			LOG("%-8sa%d, a%d, %s\n", s_lsai_ops[BIT(inst, 12, 4)], BIT(inst, 4, 4), BIT(inst, 8, 4), format_imm((inst >> 16) * 4));
			break;

		case 0b0111: // CACHE
			switch (BIT(inst, 4, 4))
			{
			case 0b0000: case 0b0001: case 0b0010: case 0b0011: // DPFR, DPFW, DPFRO, DPFWO (with Data Cache Option)
			case 0b0100: case 0b0101: case 0b0110: case 0b0111: // DHWB, DHWBI, DHI, DII (with Data Cache Option)
			case 0b1100: case 0b1110: case 0b1111: // IPF, IHI, III (with Instruction Cache Option)
				LOG("%-8sa%d, %s\n", s_cache_ops[BIT(inst, 4, 4)], BIT(inst, 8, 4), format_imm((inst >> 16) * 4));
				break;

			case 0b1000: // DCE (with Data Cache Option)
				switch (BIT(inst, 16, 4))
				{
				case 0b0000: // DPFL (with Data Cache Index Lock Option)
					LOG("%-8sa%d, %s\n", "dpfl", BIT(inst, 8, 4), format_imm((inst >> 20) * 4));
					break;

				case 0b0010: // DHU (with Data Cache Index Lock Option)
					LOG("%-8sa%d, %s\n", "dhu", BIT(inst, 8, 4), format_imm((inst >> 20) * 4));
					break;

				case 0b0011: // DIU (with Data Cache Index Lock Option)
					LOG("%-8sa%d, %s\n", "diu", BIT(inst, 8, 4), format_imm((inst >> 20) * 4));
					break;

				case 0b0100: // DIWB (added in T1050)
					LOG("%-8sa%d, %s\n", "diwb", BIT(inst, 8, 4), format_imm((inst >> 20) * 4));
					break;

				case 0b0101: // DIWBI (added in T1050)
					LOG("%-8sa%d, %s\n", "diwbi", BIT(inst, 8, 4), format_imm((inst >> 20) * 4));
					break;
				}
				break;

			case 0b1101: // ICE (with Instruction Cache Index Lock Option)
				switch (BIT(inst, 16, 4))
				{
				case 0b0000: // IPFL
					LOG("%-8sa%d, %s\n", "ipfl", BIT(inst, 8, 4), format_imm((inst >> 20) * 4));
					break;

				case 0b0010: // IHU
					LOG("%-8sa%d, %s\n", "ihu", BIT(inst, 8, 4), format_imm((inst >> 20) * 4));
					break;

				case 0b0011: // IIU
					LOG("%-8sa%d, %s\n", "iiu", BIT(inst, 8, 4), format_imm((inst >> 20) * 4));
					break;

				default:
					handle_reserved(inst);
					break;
				}
				break;

			default:
				handle_reserved(inst);
				break;
			}
			break;

		case 0b1010: // MOVI
			LOG("%-8sa%d, %s\n", "movi", BIT(inst, 4, 4), format_imm(util::sext((inst & 0x000f00) + (inst >> 16), 12)));
			break;

		case 0b1100: case 0b1101: // ADDI, ADDMI
			LOG("%-8sa%d, a%d, %s\n", s_lsai_ops[BIT(inst, 12, 4)], BIT(inst, 8, 4), BIT(inst, 4, 4), format_imm(s8(u8(inst >> 16)) * (BIT(inst, 12) ? 256 : 1)));
			break;

		default:
			handle_reserved(inst);
			break;
		}
		break;

	case 0b0011: // LSCI (with Floating-Point Coprocessor Option)
		if (BIT(inst, 12, 2) == 0)
		{
			// LSI, SSI, LSIU, SSIU
			LOG("%-8sf%d, a%d, %s\n", s_lsci_ops[BIT(inst, 14, 2)], BIT(inst, 4, 4), BIT(inst, 8, 4), format_imm(BIT(inst, 16, 8) * 4));
			break;
		}
		else
		{
			handle_reserved(inst);
			break;
		}

	case 0b0100: // MAC16 (with MAC16 Option)
		switch (BIT(inst, 20, 4))
		{
		case 0b0000: case 0b0001: // MACID, MACCD
			if (BIT(inst, 18, 2) == 0b10)
			{
				LOG("%s.dd.%s.%s m%d, a%d, m%d, m%d\n", s_mac16_ops[BIT(inst, 18, 2)],
											s_mac16_half[BIT(inst, 16, 2)],
											BIT(inst, 20) ? "lddec" : "ldinc",
											BIT(inst, 12, 2), BIT(inst, 8, 4),
											BIT(inst, 14), BIT(inst, 6) + 2);
			}
			else
			{
				handle_reserved(inst);
				break;
			}
			break;

		case 0b0100: case 0b0101: // MACIA, MACCA
			if (BIT(inst, 18, 2) == 0b10)
			{
				LOG("%s.da.%s.%s m%d, a%d, m%d, a%d\n", s_mac16_ops[BIT(inst, 18, 2)],
											s_mac16_half[BIT(inst, 16, 2)],
											BIT(inst, 20) ? "lddec" : "ldinc",
											BIT(inst, 12, 2), BIT(inst, 8, 4),
											BIT(inst, 14), BIT(inst, 4, 4));
			}
			else
			{
				handle_reserved(inst);
				break;
			}
			break;

		case 0b0010: // MACDD
			if (BIT(inst, 18, 2) != 0b00)
			{
				LOG("%s.dd.%s m%d, m%d\n", s_mac16_ops[BIT(inst, 18, 2)], s_mac16_half[BIT(inst, 16, 2)], BIT(inst, 14), BIT(inst, 6) + 2);
			}
			else
			{
				handle_reserved(inst);
				break;
			}
			break;

		case 0b0011: // MACAD
			if (BIT(inst, 18, 2) != 0b00)
			{
				LOG("%s.ad.%s a%d, m%d\n", s_mac16_ops[BIT(inst, 18, 2)], s_mac16_half[BIT(inst, 16, 2)], BIT(inst, 8, 4), BIT(inst, 6) + 2);
			}
			else
			{
				handle_reserved(inst);
				break;
			}
			break;

		case 0b0110: // MACDA
			if (BIT(inst, 18, 2) != 0b00)
			{
				LOG("%s.da.%s m%d, a%d\n", s_mac16_ops[BIT(inst, 18, 2)], s_mac16_half[BIT(inst, 16, 2)], BIT(inst, 14), BIT(inst, 4, 4));
			}
			else
			{
				handle_reserved(inst);
				break;
			}
			break;

		case 0b0111: // MACAA
			LOG("%s.aa.%s a%d, a%d\n", s_mac16_ops[BIT(inst, 18, 2)], s_mac16_half[BIT(inst, 16, 2)], BIT(inst, 8, 4), BIT(inst, 4, 4));
			break;

		case 0b1000: case 0b1001: // MACI, MACC
			switch (BIT(inst, 16, 4))
			{
			case 0b0000: // LDINC, LDDEC
				LOG("%-8sm%d, a%d\n", BIT(inst, 20) ? "lddec" : "ldinc", BIT(inst, 12, 2), BIT(inst, 8, 4));
				break;

			default:
				handle_reserved(inst);
				break;
			}
			break;

		default:
			handle_reserved(inst);
			break;
		}
		break;

	case 0b0101: // CALLN (target address is always aligned)
		switch (BIT(inst, 4, 2))
		{
		case 0b00: // CALL0
		case 0b01: case 0b10: case 0b11: // CALL4, CALL8, CALL12 (with Windowed Register Option)
			LOG("call%-4d0x%08X\n", BIT(inst, 4, 2) * 4, (m_pc & 0xfffffffc) + 4 + util::sext(inst >> 6, 18) * 4);
			break;
		}
		break;

	case 0b0110: // SI
		switch (BIT(inst, 4, 2))
		{
		case 0b00: // J
		{
			u32 newpc = m_pc + 4 + util::sext(inst >> 6, 18);
			LOG("%-8s0x%08X\n", "j", newpc);
			m_nextpc = newpc;
			break;
		}

		case 0b01: // BZ
			LOG("%-8sa%d, 0x%08X\n", s_bz_ops[BIT(inst, 6, 2)], BIT(inst, 8, 4), m_pc + 4 + util::sext(inst >> 12, 12));
			break;

		case 0b10: // BI0
			LOG("%-8sa%d, %s, 0x%08X\n", s_bi0_ops[BIT(inst, 6, 2)], BIT(inst, 8, 4), format_imm(s_b4const[BIT(inst, 12, 4)]),  m_pc + 4 + s8(u8(inst >> 16)));
			break;

		case 0b11: // BI1
			switch (BIT(inst, 6, 2))
			{
			case 0b00: // ENTRY
				LOG("%-8sa%d, %s\n", "entry", BIT(inst, 8, 4), format_imm((inst >> 12) * 4));
				break;

			case 0b01: // B1
				switch (BIT(inst, 12, 4))
				{
				case 0b0000: case 0b0001: // BF, BT (with Boolean Option)
					LOG("%-8sb%d, 0x%08X\n", BIT(inst, 12) ? "bt" : "bf", BIT(inst, 8, 4), m_pc + 4 + s8(u8(inst >> 16)));
					break;

				case 0b1000: // LOOP (with Loop Option)
					LOG("%-8sa%d, 0x%08X\n", "loop", BIT(inst, 8, 4), m_pc + 4 + s8(u8(inst >> 16)));
					break;

				case 0b1001: // LOOPNEZ (with Loop Option)
					LOG("%-8sa%d, 0x%08X\n", "loopnez", BIT(inst, 8, 4), m_pc + 4 + s8(u8(inst >> 16)));
					break;

				case 0b1010: // LOOPGTZ (with Loop Option)
					LOG("%-8sa%d, 0x%08X\n", "loopgtz", BIT(inst, 8, 4), m_pc + 4 + s8(u8(inst >> 16)));
					break;

				default:
					handle_reserved(inst);
					break;
				}
				break;

			case 0b10: case 0b11: // BLTUI, BGEUI
				LOG("%-8sa%d, %s, 0x%08X\n", BIT(inst, 6) ? "bgeui" : "bltui", BIT(inst, 8, 4), format_imm(s_b4constu[BIT(inst, 4, 4)]), m_pc + 4 + s8(u8(inst >> 16)));
				break;
			}
			break;
		}
		break;

	case 0b0111: // B
		if (BIT(inst, 9, 2) == 0b11)
		{
			// BBCI, BBSI
			LOG("%-8sa%d, %d, 0x%08X\n", BIT(inst, 11) ? "bbsi" : "bbci", BIT(inst, 8, 4), BIT(inst, 4, 4) + (BIT(inst, 12) ? 4 : 0), m_pc + 4 + s8(u8(inst >> 16)));
		}
		else
		{
			// BNONE, BEQ, BLT, BLTU, BALL, BBC, BBCI, BANY, BNE, BGE, BGEU, BNALL, BBS
			LOG("%-8sa%d, a%d, 0x%08X\n", s_b_ops[BIT(inst, 12, 4)], BIT(inst, 8, 4), BIT(inst, 4, 4), m_pc + 4 + s8(u8(inst >> 16)));
		}
		break;

	case 0b1000: case 0b1001: // L32I.N (with Code Density Option)
		LOG("%-8sa%d, a%d, %s\n", BIT(inst, 0) ? "s32i.n" : "l32i.n", BIT(inst, 4, 4), BIT(inst, 8, 4), format_imm(BIT(inst, 12, 4) * 4));
		break;

	case 0b1010: // ADD.N (with Code Density Option)
		LOG("%-8sa%d, a%d, a%d\n", "add.n", BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
		break;

	case 0b1011: // ADDI.N (with Code Density Option)
		LOG("%-8sa%d, a%d, %d\n", "addi.n", BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4) == 0 ? -1 : int(BIT(inst, 4, 4)));
		break;

	case 0b1100: // ST2 (with Code Density Option)
		if (!BIT(inst, 7))
		{
			// 7-bit immediate field uses asymmetric sign extension (range is -32..95)
			LOG("%-8sa%d, %s\n", "movi.n", BIT(inst, 8, 4), format_imm(int((inst & 0x0070) + BIT(inst, 12, 4) - (BIT(inst, 5, 2) == 0b11 ? 128 : 0))));
			break;
		}
		else
		{
			// 6-bit immediate field is zero-extended (these forms can branch forward only)
			LOG("%-8sa%d, 0x%08X\n", BIT(inst, 6) ? "bnez.n" : "beqz.n", BIT(inst, 8, 4), m_pc + 4 + (inst & 0x0030) + BIT(inst, 12, 4));
			break;
		}

	case 0b1101: // ST3 (with Code Density Option)
		switch (BIT(inst, 12, 4))
		{
		case 0b0000: // MOV.N
			LOG("%-8sa%d, a%d\n", "mov.n", BIT(inst, 4, 4), BIT(inst, 8, 4));
			break;

		case 0b1111: // S3
			switch (BIT(inst, 4, 4))
			{
			case 0b0000: // RET.N
				LOG("ret.n\n");
				break;

			case 0b0001: // RETW.N (with Windowed Register Option)
				LOG("retw.n\n");
				break;

			case 0b0010: // BREAK.N (with Debug Option)
				LOG("%-8s%d\n", "break.n", BIT(inst, 8, 4));
				break;

			case 0b0011: // NOP.N
				LOG("nop.n\n");
				break;

			case 0b0110: // ILL.N
				LOG("ill.n\n");
				break;

			default:
				handle_reserved(inst);
				break;
			}
			break;

		default:
			handle_reserved(inst);
			break;
		}
		break;

	default:
		handle_reserved(inst);
		break;
	}

	m_pc = m_nextpc;
}


void xtensa_device::execute_run()
{
	while (m_icount > 0)
	{
		debugger_instruction_hook(m_pc);
		getop_and_execute();
		m_icount--;
	}
}

void xtensa_device::execute_set_input(int inputnum, int state)
{
	// TODO
}
