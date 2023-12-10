// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Tensilica Xtensa disassembler

    All instructions are either 24 bits or 16 bits long, though data
    registers are 32 bits wide and subroutine entry points must be aligned
    on 4-byte boundaries. The 16-bit narrow instruction formats are
    redundant short forms which may or may not be supported by option.

    A big-endian version of this architecture (not supported here) was
    also defined, which reverses the position of all fields and also
    inverts the bit numbering for BBS(I) and BBC(I).

***************************************************************************/

#include "emu.h"
#include "xtensad.h"

#include "xtensa_helper.h"

xtensa_disassembler::xtensa_disassembler()
	: util::disasm_interface()
{
}

u32 xtensa_disassembler::opcode_alignment() const
{
	return 1;
}

offs_t xtensa_disassembler::disassemble(std::ostream &stream, offs_t pc, const xtensa_disassembler::data_buffer &opcodes, const xtensa_disassembler::data_buffer &params)
{
	u32 inst = opcodes.r16(pc);
	const u8 op0 = BIT(inst, 0, 4);
	if (op0 < 0b1000)
		inst |= u32(opcodes.r8(pc + 2)) << 16;

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
						stream << "ill";
						break;

					case 0b1000: // RET
						stream << "ret";
						return 3 | STEP_OUT | SUPPORTED;

					case 0b1001: // RETW (with Windowed Register Option)
						stream << "retw";
						return 3 | STEP_OUT | SUPPORTED;

					case 0b1010: // JX
						util::stream_format(stream, "%-8sa%d", "jx", BIT(inst, 8, 4));
						break;

					case 0b1100: // CALLX0
					case 0b1101: case 0b1110: case 0b1111: // CALLX4, CALLX8, CALLX12 (with Windowed Register Option)
						util::stream_format(stream, "callx%-3da%d", BIT(inst, 4, 2) * 4, BIT(inst, 8, 4));
						return 3 | STEP_OVER | SUPPORTED;

					default:
						util::stream_format(stream, "%-8s0x%02X ; reserved", "db", inst & 0xff);
						return 1 | SUPPORTED;
					}
					break;

				case 0b0001: // MOVSP (with Windowed Register Option)
					util::stream_format(stream, "%-8sa%d, a%d", "movsp", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b0010: // SYNC
					switch (BIT(inst, 4, 8))
					{
					case 0b00000000: // ISYNC
						stream << "isync";
						break;

					case 0b00000001: // RSYNC
						stream << "rsync";
						break;

					case 0b00000010: // ESYNC
						stream << "esync";
						break;

					case 0b00000011: // DSYNC
						stream << "dsync";
						break;

					case 0b00001000: // EXCW (with Exception Option)
						stream << "excw";
						break;

					case 0b00001100: // MEMW
						stream << "memw";
						break;

					case 0b00001101: // EXTW (added in RA-2004.1)
						stream << "extw";
						break;

					case 0b00001111: // NOP (added in RA-2004.1; was assembly macro previously)
						stream << "nop";
						break;

					default:
						util::stream_format(stream, "%-8s0x%02X ; reserved", "db", inst & 0xff);
						return 1 | SUPPORTED;
					}
					break;

				case 0b0011: // RFEI
					switch (BIT(inst, 4, 4))
					{
					case 0b0000: // RFET
						switch (BIT(inst, 8, 4))
						{
						case 0b0000: // RFE (with Exception Option)
							stream << "rfe";
							return 3 | STEP_OUT | SUPPORTED;

						case 0b0001: // RFUE (with Exception Option; XEA1 only)
							stream << "rfue";
							return 3 | STEP_OUT | SUPPORTED;

						case 0b0010: // RFDE (with Exception Option)
							stream << "rfde";
							return 3 | STEP_OUT | SUPPORTED;

						case 0b0100: // RFWO (with Windowed Register option)
							stream << "rfwo";
							return 3 | STEP_OUT | SUPPORTED;

						case 0b0101: // RFWU (with Windowed Register option)
							stream << "rfwu";
							return 3 | STEP_OUT | SUPPORTED;

						default:
							util::stream_format(stream, "%-8s0x%02X ; reserved", "db", inst & 0xff);
							return 1 | SUPPORTED;
						}
						break;

					case 0b0001: // RFI (with High-Priority Interrupt Option)
						util::stream_format(stream, "%-8s%d", "rfi", BIT(inst, 8, 4));
						return 3 | STEP_OUT | SUPPORTED;

					case 0b0010: // RFME (with Memory ECC/Parity Option)
						stream << "rfme";
						return 3 | STEP_OUT | SUPPORTED;

					default:
						util::stream_format(stream, "%-8s0x%02X ; reserved", "db", inst & 0xff);
						return 1 | SUPPORTED;
					}
					break;

				case 0b0100: // BREAK (with Debug Option)
					util::stream_format(stream, "%-8s%d, %d", "break", BIT(inst, 8, 4), BIT(inst, 4, 4));
					return 3 | STEP_OVER | SUPPORTED;

				case 0b0101: // SYSCALL (with Exception Option)
					stream << "syscall";
					return 3 | STEP_OVER | SUPPORTED;

				case 0b0110: // RSIL (with Interrupt Option)
					util::stream_format(stream, "%-8sa%d, %d", "rsil", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b0111: // WAITI (with Interrupt Option)
					util::stream_format(stream, "%-8s%d", "waiti", BIT(inst, 8, 4));
					break;

				case 0b1000: // ANY4 (with Boolean Option)
					util::stream_format(stream, "%-8sb%d, b%d", "any4", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b1001: // ALL4 (with Boolean Option)
					util::stream_format(stream, "%-8sb%d, b%d", "all4", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b1010: // ANY8 (with Boolean Option)
					util::stream_format(stream, "%-8sb%d, b%d", "any8", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b1011: // ALL8 (with Boolean Option)
					util::stream_format(stream, "%-8sb%d, b%d", "all8", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				default:
					util::stream_format(stream, "%-8s0x%02X ; reserved", "db", inst & 0xff);
					return 1 | SUPPORTED;
				}
				break;

			case 0b0001: // AND
				util::stream_format(stream, "%-8sa%d, a%d, a%d", "and", BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b0010: // OR
				if (BIT(inst, 8, 4) == BIT(inst, 4, 4))
					util::stream_format(stream, "%-8sa%d, a%d", "mov", BIT(inst, 12, 4), BIT(inst, 8, 4));
				else
					util::stream_format(stream, "%-8sa%d, a%d, a%d", "or", BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b0011: // XOR
				util::stream_format(stream, "%-8sa%d, a%d, a%d", "xor", BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b0100: // ST1
				switch (BIT(inst, 12, 4))
				{
				case 0b0000: case 0b0001: case 0b0010: case 0b0011: // SSR, SSL, SSA8L, SSA8B
					util::stream_format(stream, "%-8sa%d", xtensa_helper::s_st1_ops[BIT(inst, 12, 4)], BIT(inst, 8, 4));
					break;

				case 0b0100: // SSAI
					util::stream_format(stream, "%-8s%d", "ssai", BIT(inst, 8, 4) + (inst & 0x000010));
					break;

				case 0b0110: case 0b0111: // RER, WER
				case 0b1110: case 0b1111: // NSA, NSAU (with Miscellaneous Operations Option)
					util::stream_format(stream, "%-8sa%d, a%d", xtensa_helper::s_st1_ops[BIT(inst, 12, 4)], BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b1000: // ROTW (with Windowed Register Option)
					util::stream_format(stream, "%-8s%d", "rotw", util::sext(inst >> 4, 4));
					break;

				default:
					util::stream_format(stream, "%-8s0x%02X ; reserved", "db", inst & 0xff);
					return 1 | SUPPORTED;
				}
				break;

			case 0b0101: // TLB (with Region Translation Option or MMU Option)
				switch (BIT(inst, 12, 4))
				{
				case 0b0011: case 0b0101: case 0b0110: case 0b0111: // RITLB0, PITLB, WITLB, RITLB1
				case 0b1011: case 0b1101: case 0b1110: case 0b1111: // RDTLB0, PDTLB, WDTLB, RDTLB1
					util::stream_format(stream, "%-8sa%d, a%d", xtensa_helper::s_tlb_ops[BIT(inst, 12, 4)], BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b0100: case 0b1100: // IITLB, IDTLB
					util::stream_format(stream, "%-8sa%d", xtensa_helper::s_tlb_ops[BIT(inst, 12, 4)], BIT(inst, 8, 4));
					break;

				default:
					util::stream_format(stream, "%-8s0x%02X ; reserved", "db", inst & 0xff);
					return 1 | SUPPORTED;
				}
				break;

			case 0b0110: // RT0
				switch (BIT(inst, 8, 4))
				{
				case 0b0000: // NEG
					util::stream_format(stream, "%-8sa%d, a%d", "neg", BIT(inst, 12, 4), BIT(inst, 4, 4));
					break;

				case 0b0001: // ABS
					util::stream_format(stream, "%-8sa%d, a%d", "abs", BIT(inst, 12, 4), BIT(inst, 4, 4));
					break;

				default:
					util::stream_format(stream, "%-8s0x%02X ; reserved", "db", inst & 0xff);
					return 1 | SUPPORTED;
				}
				break;

			case 0b1000: case 0b1100: // ADD, SUB
				util::stream_format(stream, "%-8sa%d, a%d, a%d", BIT(inst, 22) ? "sub" : "add", BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b1001: case 0b1010: case 0b1011: // ADDX2, ADDX4, ADDX8
			case 0b1101: case 0b1110: case 0b1111: // SUBX2, SUBX4, SUBX8
				util::stream_format(stream, "%sx%-4da%d, a%d, a%d", BIT(inst, 22) ? "sub" : "add", 1 << BIT(inst, 20, 2), BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			default:
				util::stream_format(stream, "%-8s0x%02X ; reserved", "db", inst & 0xff);
				return 1 | SUPPORTED;
			}
			break;

		case 0b0001: // RST1
			switch (BIT(inst, 20, 4))
			{
			case 0b0000: case 0b0001: // SLLI (shift count is 1..31)
				util::stream_format(stream, "%-8sa%d, a%d, %d", "slli", BIT(inst, 12, 4), BIT(inst, 8, 4), 32 - (BIT(inst, 4, 4) + (BIT(inst, 20) ? 16 : 0)));
				break;

			case 0b0010: case 0b0011: // SRAI (shift count is 0..31)
				util::stream_format(stream, "%-8sa%d, a%d, %d", "srai", BIT(inst, 12, 4), BIT(inst, 4, 4), BIT(inst, 8, 4) + (BIT(inst, 20) ? 16 : 0));
				break;

			case 0b0100: // SRLI (shift count is 0..15)
				util::stream_format(stream, "%-8sa%d, a%d, %d", "srli", BIT(inst, 12, 4), BIT(inst, 4, 4), BIT(inst, 8, 4));
				break;

			case 0b0110: // XSR (added in T1040)
				util::stream_format(stream, "xsr.%-3s a%d", xtensa_helper::special_reg(BIT(inst, 8, 8), true), BIT(inst, 4, 4));
				break;

			case 0b0111: // ACCER (added in RC-2009.0)
				switch (BIT(inst, 20, 4))
				{
				case 0b0000: // RER
					util::stream_format(stream, "%-8sa%d, a%d", "rer", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b1000: // WER
					util::stream_format(stream, "%-8sa%d, a%d", "wer", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				default:
					util::stream_format(stream, "%-8s0x%02X ; reserved", "db", inst & 0xff);
					return 1 | SUPPORTED;
				}
				break;

			case 0b1000: // SRC
				util::stream_format(stream, "%-8sa%d, a%d, a%d", "src", BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b1001: // SRL
				util::stream_format(stream, "%-8sa%d, a%d", "srl", BIT(inst, 12, 4), BIT(inst, 4, 4));
				break;

			case 0b1010: // SLL
				util::stream_format(stream, "%-8sa%d, a%d", "sll", BIT(inst, 12, 4), BIT(inst, 8, 4));
				break;

			case 0b1011: // SRA
				util::stream_format(stream, "%-8sa%d, a%d", "sra", BIT(inst, 12, 4), BIT(inst, 4, 4));
				break;

			case 0b1100: // MUL16U (with 16-bit Integer Multiply Option)
				util::stream_format(stream, "%-8sa%d, a%d, a%d", "mul16u", BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b1101: // MUL16S (with 16-bit Integer Multiply Option)
				util::stream_format(stream, "%-8sa%d, a%d, a%d", "mul16s", BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b1111: // IMP (Implementation-Specific)
				switch (BIT(inst, 12, 4))
				{
				case 0b0000: // LICT (with Instruction Cache Test Option)
					util::stream_format(stream, "%-8sa%d, a%d", "lict", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b0001: // SICT (with Instruction Cache Test Option)
					util::stream_format(stream, "%-8sa%d, a%d", "sict", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b0010: // LICW (with Instruction Cache Test Option)
					util::stream_format(stream, "%-8sa%d, a%d", "licw", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b0011: // SICW (with Instruction Cache Test Option)
					util::stream_format(stream, "%-8sa%d, a%d", "sicw", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b1000: // LDCT (with Data Cache Test Option)
					util::stream_format(stream, "%-8sa%d, a%d", "ldct", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b1001: // SDCT (with Data Cache Test Option)
					util::stream_format(stream, "%-8sa%d, a%d", "sdct", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b1110: // RFDX (with On-Chip Debug)
					switch (BIT(inst, 4, 4))
					{
					case 0b0000: // RFDO
						stream << "rfdo";
						break;

					case 0b0001: // RFDD
						stream << "rfdd";
						break;

					default:
						util::stream_format(stream, "%-8s0x%02X ; reserved", "db", inst & 0xff);
						return 1 | SUPPORTED;
					}
					break;

				default:
					util::stream_format(stream, "%-8s0x%02X ; reserved", "db", inst & 0xff);
					return 1 | SUPPORTED;
				}
				break;

			default:
				util::stream_format(stream, "%-8s0x%02X ; reserved", "db", inst & 0xff);
				return 1 | SUPPORTED;
			}
			break;

		case 0b0010: // RST2
			switch (BIT(inst, 20, 4))
			{
			case 0b0000: case 0b0001: case 0b0010: case 0b0011: case 0b0100: // ANDB, ANDBC, ORB, ORBC, XORB (with Boolean Option)
				util::stream_format(stream, "%-8sb%d, b%d, b%d", xtensa_helper::s_rst2_ops[BIT(inst, 20, 4)], BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b1000: case 0b1010: case 0b1011: // MULL, MULUH, MULSH (with 32-bit Integer Multiply Option)
			case 0b1100: case 0b1101: case 0b1110: case 0b1111: // QUOU, QUOS, REMU, REMS (with 32-bit Integer Divide Option)
				util::stream_format(stream, "%-8sa%d, a%d, a%d", xtensa_helper::s_rst2_ops[BIT(inst, 20, 4)], BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			default:
				util::stream_format(stream, "%-8s0x%02X ; reserved", "db", inst & 0xff);
				return 1 | SUPPORTED;
			}
			break;

		case 0b0011: // RST3
			switch (BIT(inst, 20, 4))
			{
			case 0b0000: case 0b0001: // RSR, WSR
				util::stream_format(stream, "%s.%-3d a%d", xtensa_helper::s_rst3_ops[BIT(inst, 20, 4)], xtensa_helper::special_reg(BIT(inst, 8, 8), BIT(inst, 20)), BIT(inst, 4, 4));
				break;

			case 0b0010: case 0b0011: // SEXT, CLAMPS (with Miscellaneous Operations Option)
				util::stream_format(stream, "%-8sa%d, a%d, %d", xtensa_helper::s_rst3_ops[BIT(inst, 20, 4)], BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4) + 7);
				break;

			case 0b0100: case 0b0101: case 0b0110: case 0b0111: // MIN, MAX, MINU, MAXU (with Miscellaneous Operations Option)
			case 0b1000: case 0b1001: case 0b1010: case 0b1011: // MOVEQZ, MOVNEZ, MOVLTZ, MOVGEZ
				util::stream_format(stream, "%-8sa%d, a%d, a%d", xtensa_helper::s_rst3_ops[BIT(inst, 20, 4)], BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b1100: case 0b1101: // MOVF, MOVT (with Boolean Option)
				util::stream_format(stream, "%-8sa%d, a%d, b%d", xtensa_helper::s_rst3_ops[BIT(inst, 20, 4)], BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b1110: case 0b1111: // RUR, WUR (TODO: TIE user_register names)
				util::stream_format(stream, "%s.u%-2d a%d", xtensa_helper::s_rst3_ops[BIT(inst, 20, 4)], BIT(inst, 4, 8), BIT(inst, 12, 4));
				break;
			}
			break;

		case 0b0100: case 0b0101: // EXTUI
			util::stream_format(stream, "%-8sa%d, a%d, %d, %d", "extui", BIT(inst, 12, 4), BIT(inst, 4, 4), BIT(inst, 8, 4) + (BIT(inst, 16) ? 16 : 0), BIT(inst, 20, 4) + 1);
			break;

		case 0b0110: case 0b0111: // CUST0, CUST1
			util::stream_format(stream, "%-8s0x%02X ; cust%d?", "db", inst & 0xff, BIT(inst, 16));
			return 1 | SUPPORTED;

		case 0b1000: // LSCX (with Floating-Point Coprocessor Option)
			switch (BIT(inst, 20, 4))
			{
			case 0b0000: // LSX
				util::stream_format(stream, "%-8sf%d, a%d, a%d", "lsx", BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b0001: // LSXU
				util::stream_format(stream, "%-8sf%d, a%d, a%d", "lsxu", BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b0100: // SSX
				util::stream_format(stream, "%-8sf%d, a%d, a%d", "ssx", BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b0101: // SSXU
				util::stream_format(stream, "%-8sf%d, a%d, a%d", "ssxu", BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			default:
				util::stream_format(stream, "%-8s0x%02X ; reserved", "db", inst & 0xff);
				return 1 | SUPPORTED;
			}
			break;

		case 0b1001: // LSC4 (with Windowed Register Option)
			switch (BIT(inst, 20, 4))
			{
			case 0b0000: // L32E
				util::stream_format(stream, "%-8sa%d, a%d, %s", "l32e", BIT(inst, 4, 4), BIT(inst, 8, 4), xtensa_helper::format_imm(int(BIT(inst, 12, 4)) * 4 - 64));
				break;

			case 0b0100: // S32E
				util::stream_format(stream, "%-8sa%d, a%d, %s", "s32e", BIT(inst, 4, 4), BIT(inst, 8, 4), xtensa_helper::format_imm(int(BIT(inst, 12, 4)) * 4 - 64));
				break;

			default:
				util::stream_format(stream, "%-8s0x%02X ; reserved", "db", inst & 0xff);
				return 1 | SUPPORTED;
			}
			break;

		case 0b1010: // FP0 (with Floating-Point Coprocessor Option)
			switch (BIT(inst, 20, 4))
			{
			case 0b0000: case 0b0001: case 0b0010: case 0b0100: case 0b0101: // ADD.S, SUB.S, MUL.S, MADD.S, MSUB.S
				util::stream_format(stream, "%-8sf%d, f%d, f%d", xtensa_helper::s_fp0_ops[BIT(inst, 20, 4)], BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b1000: case 0b1001: case 0b1010: case 0b1011: case 0b1110: // ROUND.S, TRUNC.S, FLOOR.S, CEIL.S, UTRUNC.S
				util::stream_format(stream, "%-7s a%d, f%d, %d", xtensa_helper::s_fp0_ops[BIT(inst, 20, 4)], BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b1100: case 0b1101: // FLOAT.S, UFLOAT.S
				util::stream_format(stream, "%-7s f%d, a%d, %d", xtensa_helper::s_fp0_ops[BIT(inst, 20, 4)], BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b1111: // FP1OP
				switch (BIT(inst, 4, 4))
				{
				case 0b0000: // MOV.S
					util::stream_format(stream, "%-8sf%d, f%d", "mov.s", BIT(inst, 12, 4), BIT(inst, 8, 4));
					break;

				case 0b0001: // ABS.S
					util::stream_format(stream, "%-8sf%d, f%d", "abs.s", BIT(inst, 12, 4), BIT(inst, 8, 4));
					break;

				case 0b0100: // RFR
					util::stream_format(stream, "%-8sa%d, f%d", "rfr", BIT(inst, 12, 4), BIT(inst, 8, 4));
					break;

				case 0b0101: // WFR
					util::stream_format(stream, "%-8sf%d, a%d", "wfr", BIT(inst, 12, 4), BIT(inst, 8, 4));
					break;

				case 0b0110: // NEG.S
					util::stream_format(stream, "%-8sf%d, f%d", "neg.s", BIT(inst, 12, 4), BIT(inst, 8, 4));
					break;

				default:
					util::stream_format(stream, "%-8s0x%02X ; reserved", "db", inst & 0xff);
					return 1 | SUPPORTED;
				}
				break;

			default:
				util::stream_format(stream, "%-8s0x%02X ; reserved", "db", inst & 0xff);
				return 1 | SUPPORTED;
			}
			break;

		case 0b1011: // FP1 (with Floating-Point Option)
			switch (BIT(inst, 20, 4))
			{
			case 0b0001: case 0b0010: case 0b0011: case 0b0100: case 0b0101: case 0b0110: case 0b0111: // UN.S, OEQ.S, UEQ.S, OLT.S, ULT.S, OLE.S, ULE.S
				util::stream_format(stream, "%-8sb%d, f%d, f%d", xtensa_helper::s_fp1_ops[BIT(inst, 20, 4)], BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b1000: case 0b1001: case 0b1010: case 0b1011: // MOVEQZ.S, MOVNEZ.S, MOVLTZ.S, MOVGEZ.S
				util::stream_format(stream, "%-8sf%d, f%d, a%d", xtensa_helper::s_fp1_ops[BIT(inst, 20, 4)], BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b1100: case 0b1101: // MOVF.S, MOVT.S
				util::stream_format(stream, "%-8sf%d, f%d, b%d", xtensa_helper::s_fp1_ops[BIT(inst, 20, 4)], BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			default:
				util::stream_format(stream, "%-8s0x%02X ; reserved", "db", inst & 0xff);
				return 1 | SUPPORTED;
			}
			break;

		default:
			util::stream_format(stream, "%-8s0x%02X ; reserved", "db", inst & 0xff);
			return 1 | SUPPORTED;
		}
		return 3 | SUPPORTED;

	case 0b0001: // L32R (virtual address is always aligned)
		util::stream_format(stream, "%-8sa%d, 0x%08X", "l32r", BIT(inst, 4, 4), (pc + 3 - 0x40000 + (inst >> 8) * 4) & 0xfffffffc);
		return 3 | SUPPORTED;

	case 0b0010: // LSAI
		switch (BIT(inst, 12, 4))
		{
		case 0b0000: case 0b0100: // L8UI, S8I
			util::stream_format(stream, "%-8sa%d, a%d, %s", xtensa_helper::s_lsai_ops[BIT(inst, 12, 4)], BIT(inst, 4, 4), BIT(inst, 8, 4), xtensa_helper::format_imm(inst >> 16));
			break;

		case 0b0001: case 0b0101: case 0b1001: // L16UI, S16I, L16SI
			util::stream_format(stream, "%-8sa%d, a%d, %s", xtensa_helper::s_lsai_ops[BIT(inst, 12, 4)], BIT(inst, 4, 4), BIT(inst, 8, 4), xtensa_helper::format_imm((inst >> 16) * 2));
			break;

		case 0b0010: case 0b0110: // L32I, S32I
		case 0b1011: case 0b1111: // L32AI, S32RI (with Multiprocessor Synchronization Option)
		case 0b1110: // S32C1I (with Conditional Store Option)
			util::stream_format(stream, "%-8sa%d, a%d, %s", xtensa_helper::s_lsai_ops[BIT(inst, 12, 4)], BIT(inst, 4, 4), BIT(inst, 8, 4), xtensa_helper::format_imm((inst >> 16) * 4));
			break;

		case 0b0111: // CACHE
			switch (BIT(inst, 4, 4))
			{
			case 0b0000: case 0b0001: case 0b0010: case 0b0011: // DPFR, DPFW, DPFRO, DPFWO (with Data Cache Option)
			case 0b0100: case 0b0101: case 0b0110: case 0b0111: // DHWB, DHWBI, DHI, DII (with Data Cache Option)
			case 0b1100: case 0b1110: case 0b1111: // IPF, IHI, III (with Instruction Cache Option)
				util::stream_format(stream, "%-8sa%d, %s", xtensa_helper::s_cache_ops[BIT(inst, 4, 4)], BIT(inst, 8, 4), xtensa_helper::format_imm((inst >> 16) * 4));
				break;

			case 0b1000: // DCE (with Data Cache Option)
				switch (BIT(inst, 16, 4))
				{
				case 0b0000: // DPFL (with Data Cache Index Lock Option)
					util::stream_format(stream, "%-8sa%d, %s", "dpfl", BIT(inst, 8, 4), xtensa_helper::format_imm((inst >> 20) * 4));
					break;

				case 0b0010: // DHU (with Data Cache Index Lock Option)
					util::stream_format(stream, "%-8sa%d, %s", "dhu", BIT(inst, 8, 4), xtensa_helper::format_imm((inst >> 20) * 4));
					break;

				case 0b0011: // DIU (with Data Cache Index Lock Option)
					util::stream_format(stream, "%-8sa%d, %s", "diu", BIT(inst, 8, 4), xtensa_helper::format_imm((inst >> 20) * 4));
					break;

				case 0b0100: // DIWB (added in T1050)
					util::stream_format(stream, "%-8sa%d, %s", "diwb", BIT(inst, 8, 4), xtensa_helper::format_imm((inst >> 20) * 4));
					break;

				case 0b0101: // DIWBI (added in T1050)
					util::stream_format(stream, "%-8sa%d, %s", "diwbi", BIT(inst, 8, 4), xtensa_helper::format_imm((inst >> 20) * 4));
					break;
				}
				break;

			case 0b1101: // ICE (with Instruction Cache Index Lock Option)
				switch (BIT(inst, 16, 4))
				{
				case 0b0000: // IPFL
					util::stream_format(stream, "%-8sa%d, %s", "ipfl", BIT(inst, 8, 4), xtensa_helper::format_imm((inst >> 20) * 4));
					break;

				case 0b0010: // IHU
					util::stream_format(stream, "%-8sa%d, %s", "ihu", BIT(inst, 8, 4), xtensa_helper::format_imm((inst >> 20) * 4));
					break;

				case 0b0011: // IIU
					util::stream_format(stream, "%-8sa%d, %s", "iiu", BIT(inst, 8, 4), xtensa_helper::format_imm((inst >> 20) * 4));
					break;

				default:
					util::stream_format(stream, "%-8s0x%02X ; reserved", "db", inst & 0xff);
					return 1 | SUPPORTED;
				}
				break;

			default:
				util::stream_format(stream, "%-8s0x%02X ; reserved", "db", inst & 0xff);
				return 1 | SUPPORTED;
			}
			break;

		case 0b1010: // MOVI
			util::stream_format(stream, "%-8sa%d, %s", "movi", BIT(inst, 4, 4), xtensa_helper::format_imm(util::sext((inst & 0x000f00) + (inst >> 16), 12)));
			break;

		case 0b1100: case 0b1101: // ADDI, ADDMI
			util::stream_format(stream, "%-8sa%d, a%d, %s", xtensa_helper::s_lsai_ops[BIT(inst, 12, 4)], BIT(inst, 4, 4), BIT(inst, 8, 4), xtensa_helper::format_imm(s8(u8(inst >> 16)) * (BIT(inst, 12) ? 256 : 1)));
			break;

		default:
			util::stream_format(stream, "%-8s0x%02X ; reserved", "db", inst & 0xff);
			return 1 | SUPPORTED;
		}
		return 3 | SUPPORTED;

	case 0b0011: // LSCI (with Floating-Point Coprocessor Option)
		if (BIT(inst, 12, 2) == 0)
		{
			// LSI, SSI, LSIU, SSIU
			util::stream_format(stream, "%-8sf%d, a%d, %s", xtensa_helper::s_lsci_ops[BIT(inst, 14, 2)], BIT(inst, 4, 4), BIT(inst, 8, 4), xtensa_helper::format_imm(BIT(inst, 16, 8) * 4));
			return 3 | SUPPORTED;
		}
		else
		{
			util::stream_format(stream, "%-8s0x%02X ; reserved", "db", inst & 0xff);
			return 1 | SUPPORTED;
		}

	case 0b0100: // MAC16 (with MAC16 Option)
		switch (BIT(inst, 20, 4))
		{
		case 0b0000: case 0b0001: // MACID, MACCD
			if (BIT(inst, 18, 2) == 0b10)
				util::stream_format(stream, "%s.dd.%s.%s m%d, a%d, m%d, m%d", xtensa_helper::s_mac16_ops[BIT(inst, 18, 2)],
											xtensa_helper::s_mac16_half[BIT(inst, 16, 2)],
											BIT(inst, 20) ? "lddec" : "ldinc",
											BIT(inst, 12, 2), BIT(inst, 8, 4),
											BIT(inst, 14), BIT(inst, 6) + 2);
			else
			{
				util::stream_format(stream, "%-8s0x%02X ; reserved", "db", inst & 0xff);
				return 1 | SUPPORTED;
			}
			break;

		case 0b0100: case 0b0101: // MACIA, MACCA
			if (BIT(inst, 18, 2) == 0b10)
				util::stream_format(stream, "%s.da.%s.%s m%d, a%d, m%d, a%d", xtensa_helper::s_mac16_ops[BIT(inst, 18, 2)],
											xtensa_helper::s_mac16_half[BIT(inst, 16, 2)],
											BIT(inst, 20) ? "lddec" : "ldinc",
											BIT(inst, 12, 2), BIT(inst, 8, 4),
											BIT(inst, 14), BIT(inst, 4, 4));
			else
			{
				util::stream_format(stream, "%-8s0x%02X ; reserved", "db", inst & 0xff);
				return 1 | SUPPORTED;
			}
			break;

		case 0b0010: // MACDD
			if (BIT(inst, 18, 2) != 0b00)
				util::stream_format(stream, "%s.dd.%s m%d, m%d", xtensa_helper::s_mac16_ops[BIT(inst, 18, 2)], xtensa_helper::s_mac16_half[BIT(inst, 16, 2)], BIT(inst, 14), BIT(inst, 6) + 2);
			else
			{
				util::stream_format(stream, "%-8s0x%02X ; reserved", "db", inst & 0xff);
				return 1 | SUPPORTED;
			}
			break;

		case 0b0011: // MACAD
			if (BIT(inst, 18, 2) != 0b00)
				util::stream_format(stream, "%s.ad.%s a%d, m%d", xtensa_helper::s_mac16_ops[BIT(inst, 18, 2)], xtensa_helper::s_mac16_half[BIT(inst, 16, 2)], BIT(inst, 8, 4), BIT(inst, 6) + 2);
			else
			{
				util::stream_format(stream, "%-8s0x%02X ; reserved", "db", inst & 0xff);
				return 1 | SUPPORTED;
			}
			break;

		case 0b0110: // MACDA
			if (BIT(inst, 18, 2) != 0b00)
				util::stream_format(stream, "%s.da.%s m%d, a%d", xtensa_helper::s_mac16_ops[BIT(inst, 18, 2)], xtensa_helper::s_mac16_half[BIT(inst, 16, 2)], BIT(inst, 14), BIT(inst, 4, 4));
			else
			{
				util::stream_format(stream, "%-8s0x%02X ; reserved", "db", inst & 0xff);
				return 1 | SUPPORTED;
			}
			break;

		case 0b0111: // MACAA
			util::stream_format(stream, "%s.aa.%s a%d, a%d", xtensa_helper::s_mac16_ops[BIT(inst, 18, 2)], xtensa_helper::s_mac16_half[BIT(inst, 16, 2)], BIT(inst, 8, 4), BIT(inst, 4, 4));
			break;

		case 0b1000: case 0b1001: // MACI, MACC
			switch (BIT(inst, 16, 4))
			{
			case 0b0000: // LDINC, LDDEC
				util::stream_format(stream, "%-8sm%d, a%d", BIT(inst, 20) ? "lddec" : "ldinc", BIT(inst, 12, 2), BIT(inst, 8, 4));
				break;

			default:
				util::stream_format(stream, "%-8s0x%02X ; reserved", "db", inst & 0xff);
				return 1 | SUPPORTED;
			}
			break;

		default:
			util::stream_format(stream, "%-8s0x%02X ; reserved", "db", inst & 0xff);
			return 1 | SUPPORTED;
		}
		return 3 | SUPPORTED;

	case 0b0101: // CALLN (target address is always aligned)
		switch (BIT(inst, 4, 2))
		{
		case 0b00: // CALL0
		case 0b01: case 0b10: case 0b11: // CALL4, CALL8, CALL12 (with Windowed Register Option)
			util::stream_format(stream, "call%-4d0x%08X", BIT(inst, 4, 2) * 4, (pc & 0xfffffffc) + 4 + util::sext(inst >> 6, 18) * 4);
			break;
		}
		return 3 | STEP_OVER | SUPPORTED;

	case 0b0110: // SI
		switch (BIT(inst, 4, 2))
		{
		case 0b00: // J
			util::stream_format(stream, "%-8s0x%08X", "j", pc + 4 + util::sext(inst >> 6, 18));
			break;

		case 0b01: // BZ
			util::stream_format(stream, "%-8sa%d, 0x%08X", xtensa_helper::s_bz_ops[BIT(inst, 6, 2)], BIT(inst, 8, 4), pc + 4 + util::sext(inst >> 12, 12));
			return 3 | STEP_COND | SUPPORTED;

		case 0b10: // BI0
			util::stream_format(stream, "%-8sa%d, %s, 0x%08X", xtensa_helper::s_bi0_ops[BIT(inst, 6, 2)], BIT(inst, 8, 4), xtensa_helper::format_imm(xtensa_helper::s_b4const[BIT(inst, 12, 4)]), pc + 4 + s8(u8(inst >> 16)));
			return 3 | STEP_COND | SUPPORTED;

		case 0b11: // BI1
			switch (BIT(inst, 6, 2))
			{
			case 0b00: // ENTRY
				util::stream_format(stream, "%-8sa%d, %s", "entry", BIT(inst, 8, 4), xtensa_helper::format_imm((inst >> 12) * 4));
				break;

			case 0b01: // B1
				switch (BIT(inst, 12, 4))
				{
				case 0b0000: case 0b0001: // BF, BT (with Boolean Option)
					util::stream_format(stream, "%-8sb%d, 0x%08X", BIT(inst, 12) ? "bt" : "bf", BIT(inst, 8, 4), pc + 4 + s8(u8(inst >> 16)));
					return 3 | STEP_COND | SUPPORTED;

				case 0b1000: // LOOP (with Loop Option)
					util::stream_format(stream, "%-8sa%d, 0x%08X", "loop", BIT(inst, 8, 4), pc + 4 + s8(u8(inst >> 16)));
					break;

				case 0b1001: // LOOPNEZ (with Loop Option)
					util::stream_format(stream, "%-8sa%d, 0x%08X", "loopnez", BIT(inst, 8, 4), pc + 4 + s8(u8(inst >> 16)));
					return 3 | STEP_COND | SUPPORTED;

				case 0b1010: // LOOPGTZ (with Loop Option)
					util::stream_format(stream, "%-8sa%d, 0x%08X", "loopgtz", BIT(inst, 8, 4), pc + 4 + s8(u8(inst >> 16)));
					return 3 | STEP_COND | SUPPORTED;

				default:
					util::stream_format(stream, "%-8s0x%02X ; reserved", "db", inst & 0xff);
					return 1 | SUPPORTED;
				}
				break;

			case 0b10: case 0b11: // BLTUI, BGEUI
				util::stream_format(stream, "%-8sa%d, %s, 0x%08X", BIT(inst, 6) ? "bgeui" : "bltui", BIT(inst, 8, 4), xtensa_helper::format_imm(xtensa_helper::s_b4constu[BIT(inst, 12, 4)]), pc + 4 + s8(u8(inst >> 16)));
				return 3 | STEP_COND | SUPPORTED;
			}
			break;
		}
		return 3 | SUPPORTED;

	case 0b0111: // B
		if (BIT(inst, 13, 2) == 0b11)
		{
			// BBCI, BBSI
			util::stream_format(stream, "%-8sa%d, %d, 0x%08X", BIT(inst, 15) ? "bbsi" : "bbci", BIT(inst, 8, 4), BIT(inst, 4, 4) + (BIT(inst, 12) ? 16 : 0), pc + 4 + s8(u8(inst >> 16)));
		}
		else
		{
			// BNONE, BEQ, BLT, BLTU, BALL, BBC, BBCI, BANY, BNE, BGE, BGEU, BNALL, BBS
			util::stream_format(stream, "%-8sa%d, a%d, 0x%08X", xtensa_helper::s_b_ops[BIT(inst, 12, 4)], BIT(inst, 8, 4), BIT(inst, 4, 4), pc + 4 + s8(u8(inst >> 16)));
		}
		return 3 | STEP_COND | SUPPORTED;

	case 0b1000: case 0b1001: // L32I.N (with Code Density Option)
		util::stream_format(stream, "%-8sa%d, a%d, %s", BIT(inst, 0) ? "s32i.n" : "l32i.n", BIT(inst, 4, 4), BIT(inst, 8, 4), xtensa_helper::format_imm(BIT(inst, 12, 4) * 4));
		return 2 | SUPPORTED;

	case 0b1010: // ADD.N (with Code Density Option)
		util::stream_format(stream, "%-8sa%d, a%d, a%d", "add.n", BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
		return 2 | SUPPORTED;

	case 0b1011: // ADDI.N (with Code Density Option)
		util::stream_format(stream, "%-8sa%d, a%d, %d", "addi.n", BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4) == 0 ? -1 : int(BIT(inst, 4, 4)));
		return 2 | SUPPORTED;

	case 0b1100: // ST2 (with Code Density Option)
		if (!BIT(inst, 7))
		{
			// 7-bit immediate field uses asymmetric sign extension (range is -32..95)
			util::stream_format(stream, "%-8sa%d, %s", "movi.n", BIT(inst, 8, 4), xtensa_helper::format_imm(int((inst & 0x0070) + BIT(inst, 12, 4) - (BIT(inst, 5, 2) == 0b11 ? 128 : 0))));
			return 2 | SUPPORTED;
		}
		else
		{
			// 6-bit immediate field is zero-extended (these forms can branch forward only)
			util::stream_format(stream, "%-8sa%d, 0x%08X", BIT(inst, 6) ? "bnez.n" : "beqz.n", BIT(inst, 8, 4), pc + 4 + (inst & 0x0030) + BIT(inst, 12, 4));
			return 2 | STEP_COND | SUPPORTED;
		}

	case 0b1101: // ST3 (with Code Density Option)
		switch (BIT(inst, 12, 4))
		{
		case 0b0000: // MOV.N
			util::stream_format(stream, "%-8sa%d, a%d", "mov.n", BIT(inst, 4, 4), BIT(inst, 8, 4));
			break;

		case 0b1111: // S3
			switch (BIT(inst, 4, 4))
			{
			case 0b0000: // RET.N
				stream << "ret.n";
				return 2 | STEP_OUT | SUPPORTED;

			case 0b0001: // RETW.N (with Windowed Register Option)
				stream << "retw.n";
				return 2 | STEP_OUT | SUPPORTED;

			case 0b0010: // BREAK.N (with Debug Option)
				util::stream_format(stream, "%-8s%d", "break.n", BIT(inst, 8, 4));
				return 2 | STEP_OVER | SUPPORTED;

			case 0b0011: // NOP.N
				stream << "nop.n";
				break;

			case 0b0110: // ILL.N
				stream << "ill.n";
				break;

			default:
				util::stream_format(stream, "%-8s0x%02X ; reserved", "db", inst & 0xff);
				return 1 | SUPPORTED;
			}
			break;

		default:
			util::stream_format(stream, "%-8s0x%02X ; reserved", "db", inst & 0xff);
			return 1 | SUPPORTED;
		}
		return 2 | SUPPORTED;

	default:
		util::stream_format(stream, "%-8s0x%02X ; reserved", "db", inst & 0xff);
		return 1 | SUPPORTED;
	}
}
