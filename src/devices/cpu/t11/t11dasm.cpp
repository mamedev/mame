// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*
 *   A T11 disassembler
 *
 *   Note: this is probably not the most efficient disassembler in the world :-)
 *
 *   This code written by Aaron Giles (agiles@sirius.com) for the MAME project
 *
 */

#include "emu.h"
#include "t11dasm.h"

#include <cassert>


const char *const t11_disassembler::regs[8] = { "R0", "R1", "R2", "R3", "R4", "R5", "SP", "PC" };

u16 t11_disassembler::r16p(offs_t &pc, const data_buffer &opcodes)
{
	u16 r = opcodes.r16(pc);
	pc += 2;
	return r;
}

template <bool Byte> std::string t11_disassembler::MakeEA (int lo, offs_t &pc, const data_buffer &opcodes)
{
	int reg, pm;

	reg = lo & 7;

	switch ((lo >> 3) & 7)
	{
		case 0:
			return util::string_format ("%s", regs[reg]);
			break;
		case 1:
			return util::string_format ("(%s)", regs[reg]);
			break;
		case 2:
			if (reg == 7) // immediate
			{
				pm = r16p(pc, opcodes);
				return util::string_format ("#%0*o", Byte ? 3 : 6, pm & (Byte ? 0377 : 0177777));
			}
			else
			{
				return util::string_format ("(%s)+", regs[reg]);
			}
			break;
		case 3:
			if (reg == 7) // absolute
			{
				pm = r16p(pc, opcodes);
				return util::string_format ("@#%06o", pm & 0177777);
			}
			else
			{
				return util::string_format ("@(%s)+", regs[reg]);
			}
			break;
		case 4:
			return util::string_format ("-(%s)", regs[reg]);
			break;
		case 5:
			return util::string_format ("@-(%s)", regs[reg]);
			break;
		case 6:
			pm = r16p(pc, opcodes);
			if (reg == 7) // relative
			{
				return util::string_format ("%06o", (pm + pc) & 0177777);
			}
			else
			{
				return util::string_format ("%s%o(%s)",
					BIT(pm,15)?"-":"",
					BIT(pm,15)?-(int16_t)pm:pm,
					regs[reg]);
			}
			break;
		case 7:
			pm = r16p(pc, opcodes);
			if (reg == 7) // relative deferred
			{
				return util::string_format ("@%06o", (pm + pc) & 0177777);
			}
			else
			{
				return util::string_format ("@%s%o(%s)",
					BIT(pm,15)?"-":"",
					BIT(pm,15)?-(int16_t)pm:pm,
					regs[reg]);
			}
			break;
	}
	return "";
}


offs_t t11_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	offs_t PC = pc;
	uint16_t op, lo, hi, addr;
	int16_t offset;
	uint32_t flags = 0;
	std::string ea1, ea2;

	op = r16p(pc, opcodes);
	lo = op & 077;
	hi = (op >> 6) & 077;

	switch (op & 0177700)
	{
		case 0000000:
			switch (lo)
			{
				case 000:  util::stream_format(stream, "HALT"); break;
				case 001:  util::stream_format(stream, "WAIT"); break;
				case 002:  util::stream_format(stream, "RTI"); flags = STEP_OUT; break;
				case 003:  util::stream_format(stream, "BPT"); flags = STEP_OVER; break;
				case 004:  util::stream_format(stream, "IOT"); flags = STEP_OVER; break;
				case 005:  util::stream_format(stream, "RESET"); break;
				case 006:  util::stream_format(stream, "RTT"); flags = STEP_OUT; break;
				case 007:  util::stream_format(stream, "MFPT"); break;
				default:   util::stream_format(stream, ".WORD %06o", op); break;
			}
			break;
		case 0000100:
			ea1 = MakeEA<false> (lo, pc, opcodes);
			util::stream_format(stream, "JMP   %s", ea1);
			break;
		case 0000200:
			switch (lo & 070)
			{
				case 000:
					if( (lo & 7) == 7 )
						util::stream_format(stream, "RETURN");
					else
						util::stream_format(stream, "RTS   %s", regs[lo & 7]);
					flags = STEP_OUT;
					break;
				// 00023x: SPL (not implemented on T-11)
				case 040:
				case 050:
					switch( lo & 15 )
					{
						case 000:  util::stream_format(stream, "NOP"); break;
						case 017:  util::stream_format(stream, "CCC"); break;
						case 001:  util::stream_format(stream, "CLC"); break;
						case 002:  util::stream_format(stream, "CLV"); break;
						case 004:  util::stream_format(stream, "CLZ"); break;
						case 010:  util::stream_format(stream, "CLN"); break;
						default:   util::stream_format(stream, "Ccc   #%02o", lo & 15); break;
					}
					break;
				case 060:
				case 070:
					switch( lo & 15 )
					{
						case 000:  util::stream_format(stream, "NOP"); break;
						case 017:  util::stream_format(stream, "SCC"); break;
						case 001:  util::stream_format(stream, "SEC"); break;
						case 002:  util::stream_format(stream, "SEV"); break;
						case 004:  util::stream_format(stream, "SEZ"); break;
						case 010:  util::stream_format(stream, "SEN"); break;
						default:   util::stream_format(stream, "Scc   #%02o", lo & 15); break;
					}
					break;
				default:
					util::stream_format(stream, ".WORD %06o", op);
					break;
			}
			break;
		case 0000300:
			ea1 = MakeEA<false> (lo, pc, opcodes);
			util::stream_format(stream, "SWAB  %s", ea1);
			break;
		case 0000400: case 0000500: case 0000600: case 0000700:
			offset = 2 * (int8_t)(op & 0377);
			util::stream_format(stream, "BR    %06o", (pc + offset) & 0177777);
			break;
		case 0001000: case 0001100: case 0001200: case 0001300:
			offset = 2 * (int8_t)(op & 0377);
			util::stream_format(stream, "BNE   %06o", (pc + offset) & 0177777);
			flags = STEP_COND;
			break;
		case 0001400: case 0001500: case 0001600: case 0001700:
			offset = 2 * (int8_t)(op & 0377);
			util::stream_format(stream, "BEQ   %06o", (pc + offset) & 0177777);
			flags = STEP_COND;
			break;
		case 0002000: case 0002100: case 0002200: case 0002300:
			offset = 2 * (int8_t)(op & 0377);
			util::stream_format(stream, "BGE   %06o", (pc + offset) & 0177777);
			flags = STEP_COND;
			break;
		case 0002400: case 0002500: case 0002600: case 0002700:
			offset = 2 * (int8_t)(op & 0377);
			util::stream_format(stream, "BLT   %06o", (pc + offset) & 0177777);
			flags = STEP_COND;
			break;
		case 0003000: case 0003100: case 0003200: case 0003300:
			offset = 2 * (int8_t)(op & 0377);
			util::stream_format(stream, "BGT   %06o", (pc + offset) & 0177777);
			flags = STEP_COND;
			break;
		case 0003400: case 0003500: case 0003600: case 0003700:
			offset = 2 * (int8_t)(op & 0377);
			util::stream_format(stream, "BLE   %06o", (pc + offset) & 0177777);
			flags = STEP_COND;
			break;
		case 0004000: case 0004100: case 0004200: case 0004300:
		case 0004400: case 0004500: case 0004600: case 0004700:
			ea1 = MakeEA<false> (lo, pc, opcodes);
			if ( (hi & 7) == 7 )
				util::stream_format(stream, "CALL  %s", ea1);
			else
				util::stream_format(stream, "JSR   %s,%s", regs[hi & 7], ea1);
			flags = STEP_OVER;
			break;
		case 0005000:
			ea1 = MakeEA<false> (lo, pc, opcodes);
			util::stream_format(stream, "CLR   %s", ea1);
			break;
		case 0005100:
			ea1 = MakeEA<false> (lo, pc, opcodes);
			util::stream_format(stream, "COM   %s", ea1);
			break;
		case 0005200:
			ea1 = MakeEA<false> (lo, pc, opcodes);
			util::stream_format(stream, "INC   %s", ea1);
			break;
		case 0005300:
			ea1 = MakeEA<false> (lo, pc, opcodes);
			util::stream_format(stream, "DEC   %s", ea1);
			break;
		case 0005400:
			ea1 = MakeEA<false> (lo, pc, opcodes);
			util::stream_format(stream, "NEG   %s", ea1);
			break;
		case 0005500:
			ea1 = MakeEA<false> (lo, pc, opcodes);
			util::stream_format(stream, "ADC   %s", ea1);
			break;
		case 0005600:
			ea1 = MakeEA<false> (lo, pc, opcodes);
			util::stream_format(stream, "SBC   %s", ea1);
			break;
		case 0005700:
			ea1 = MakeEA<false> (lo, pc, opcodes);
			util::stream_format(stream, "TST   %s", ea1);
			break;
		case 0006000:
			ea1 = MakeEA<false> (lo, pc, opcodes);
			util::stream_format(stream, "ROR   %s", ea1);
			break;
		case 0006100:
			ea1 = MakeEA<false> (lo, pc, opcodes);
			util::stream_format(stream, "ROL   %s", ea1);
			break;
		case 0006200:
			ea1 = MakeEA<false> (lo, pc, opcodes);
			util::stream_format(stream, "ASR   %s", ea1);
			break;
		case 0006300:
			ea1 = MakeEA<false> (lo, pc, opcodes);
			util::stream_format(stream, "ASL   %s", ea1);
			break;
		// 0064xx: MARK (not implemented on T-11)
		case 0006400:
			ea1 = MakeEA<false> (lo, pc, opcodes);
			util::stream_format(stream, "MARK  %s", ea1);
			break;
		// 0065xx: MFPI (not implemented on T-11)
		// 0066xx: MTPI (not implemented on T-11)
		case 0006700:
			ea1 = MakeEA<false> (lo, pc, opcodes);
			util::stream_format(stream, "SXT   %s", ea1);
			break;
		// 0070xx: CSM (not implemented on T-11)
		// 0072xx: TSTSET (not implemented on T-11)
		// 0073xx: WRTLCK (not implemented on T-11)

		case 0010000: case 0010100: case 0010200: case 0010300: case 0010400: case 0010500: case 0010600: case 0010700:
		case 0011000: case 0011100: case 0011200: case 0011300: case 0011400: case 0011500: case 0011600: case 0011700:
		case 0012000: case 0012100: case 0012200: case 0012300: case 0012400: case 0012500: case 0012600: case 0012700:
		case 0013000: case 0013100: case 0013200: case 0013300: case 0013400: case 0013500: case 0013600: case 0013700:
		case 0014000: case 0014100: case 0014200: case 0014300: case 0014400: case 0014500: case 0014600: case 0014700:
		case 0015000: case 0015100: case 0015200: case 0015300: case 0015400: case 0015500: case 0015600: case 0015700:
		case 0016000: case 0016100: case 0016200: case 0016300: case 0016400: case 0016500: case 0016600: case 0016700:
		case 0017000: case 0017100: case 0017200: case 0017300: case 0017400: case 0017500: case 0017600: case 0017700:
			ea1 = MakeEA<false> (hi, pc, opcodes);
			ea2 = MakeEA<false> (lo, pc, opcodes);
			util::stream_format(stream, "MOV   %s,%s", ea1, ea2);
			break;
		case 0020000: case 0020100: case 0020200: case 0020300: case 0020400: case 0020500: case 0020600: case 0020700:
		case 0021000: case 0021100: case 0021200: case 0021300: case 0021400: case 0021500: case 0021600: case 0021700:
		case 0022000: case 0022100: case 0022200: case 0022300: case 0022400: case 0022500: case 0022600: case 0022700:
		case 0023000: case 0023100: case 0023200: case 0023300: case 0023400: case 0023500: case 0023600: case 0023700:
		case 0024000: case 0024100: case 0024200: case 0024300: case 0024400: case 0024500: case 0024600: case 0024700:
		case 0025000: case 0025100: case 0025200: case 0025300: case 0025400: case 0025500: case 0025600: case 0025700:
		case 0026000: case 0026100: case 0026200: case 0026300: case 0026400: case 0026500: case 0026600: case 0026700:
		case 0027000: case 0027100: case 0027200: case 0027300: case 0027400: case 0027500: case 0027600: case 0027700:
			ea1 = MakeEA<false> (hi, pc, opcodes);
			ea2 = MakeEA<false> (lo, pc, opcodes);
			util::stream_format(stream, "CMP   %s,%s", ea1, ea2);
			break;
		case 0030000: case 0030100: case 0030200: case 0030300: case 0030400: case 0030500: case 0030600: case 0030700:
		case 0031000: case 0031100: case 0031200: case 0031300: case 0031400: case 0031500: case 0031600: case 0031700:
		case 0032000: case 0032100: case 0032200: case 0032300: case 0032400: case 0032500: case 0032600: case 0032700:
		case 0033000: case 0033100: case 0033200: case 0033300: case 0033400: case 0033500: case 0033600: case 0033700:
		case 0034000: case 0034100: case 0034200: case 0034300: case 0034400: case 0034500: case 0034600: case 0034700:
		case 0035000: case 0035100: case 0035200: case 0035300: case 0035400: case 0035500: case 0035600: case 0035700:
		case 0036000: case 0036100: case 0036200: case 0036300: case 0036400: case 0036500: case 0036600: case 0036700:
		case 0037000: case 0037100: case 0037200: case 0037300: case 0037400: case 0037500: case 0037600: case 0037700:
			ea1 = MakeEA<false> (hi, pc, opcodes);
			ea2 = MakeEA<false> (lo, pc, opcodes);
			util::stream_format(stream, "BIT   %s,%s", ea1, ea2);
			break;
		case 0040000: case 0040100: case 0040200: case 0040300: case 0040400: case 0040500: case 0040600: case 0040700:
		case 0041000: case 0041100: case 0041200: case 0041300: case 0041400: case 0041500: case 0041600: case 0041700:
		case 0042000: case 0042100: case 0042200: case 0042300: case 0042400: case 0042500: case 0042600: case 0042700:
		case 0043000: case 0043100: case 0043200: case 0043300: case 0043400: case 0043500: case 0043600: case 0043700:
		case 0044000: case 0044100: case 0044200: case 0044300: case 0044400: case 0044500: case 0044600: case 0044700:
		case 0045000: case 0045100: case 0045200: case 0045300: case 0045400: case 0045500: case 0045600: case 0045700:
		case 0046000: case 0046100: case 0046200: case 0046300: case 0046400: case 0046500: case 0046600: case 0046700:
		case 0047000: case 0047100: case 0047200: case 0047300: case 0047400: case 0047500: case 0047600: case 0047700:
			ea1 = MakeEA<false> (hi, pc, opcodes);
			ea2 = MakeEA<false> (lo, pc, opcodes);
			util::stream_format(stream, "BIC   %s,%s", ea1, ea2);
			break;
		case 0050000: case 0050100: case 0050200: case 0050300: case 0050400: case 0050500: case 0050600: case 0050700:
		case 0051000: case 0051100: case 0051200: case 0051300: case 0051400: case 0051500: case 0051600: case 0051700:
		case 0052000: case 0052100: case 0052200: case 0052300: case 0052400: case 0052500: case 0052600: case 0052700:
		case 0053000: case 0053100: case 0053200: case 0053300: case 0053400: case 0053500: case 0053600: case 0053700:
		case 0054000: case 0054100: case 0054200: case 0054300: case 0054400: case 0054500: case 0054600: case 0054700:
		case 0055000: case 0055100: case 0055200: case 0055300: case 0055400: case 0055500: case 0055600: case 0055700:
		case 0056000: case 0056100: case 0056200: case 0056300: case 0056400: case 0056500: case 0056600: case 0056700:
		case 0057000: case 0057100: case 0057200: case 0057300: case 0057400: case 0057500: case 0057600: case 0057700:
			ea1 = MakeEA<false> (hi, pc, opcodes);
			ea2 = MakeEA<false> (lo, pc, opcodes);
			util::stream_format(stream, "BIS   %s,%s", ea1, ea2);
			break;
		case 0060000: case 0060100: case 0060200: case 0060300: case 0060400: case 0060500: case 0060600: case 0060700:
		case 0061000: case 0061100: case 0061200: case 0061300: case 0061400: case 0061500: case 0061600: case 0061700:
		case 0062000: case 0062100: case 0062200: case 0062300: case 0062400: case 0062500: case 0062600: case 0062700:
		case 0063000: case 0063100: case 0063200: case 0063300: case 0063400: case 0063500: case 0063600: case 0063700:
		case 0064000: case 0064100: case 0064200: case 0064300: case 0064400: case 0064500: case 0064600: case 0064700:
		case 0065000: case 0065100: case 0065200: case 0065300: case 0065400: case 0065500: case 0065600: case 0065700:
		case 0066000: case 0066100: case 0066200: case 0066300: case 0066400: case 0066500: case 0066600: case 0066700:
		case 0067000: case 0067100: case 0067200: case 0067300: case 0067400: case 0067500: case 0067600: case 0067700:
			ea1 = MakeEA<false> (hi, pc, opcodes);
			ea2 = MakeEA<false> (lo, pc, opcodes);
			util::stream_format(stream, "ADD   %s,%s", ea1, ea2);
			break;

		// 070xxx: MUL (not implemented on T-11)
		case 0070000: case 0070100: case 0070200: case 0070300: case 0070400: case 0070500: case 0070600: case 0070700:
			ea1 = MakeEA<false> (lo, pc, opcodes);
			util::stream_format(stream, "MUL   %s,%s", ea1, regs[hi & 7]);
			break;
		// 071xxx: DIV (not implemented on T-11)
		case 0071000: case 0071100: case 0071200: case 0071300: case 0071400: case 0071500: case 0071600: case 0071700:
			ea1 = MakeEA<false> (lo, pc, opcodes);
			util::stream_format(stream, "DIV   %s,%s", ea1, regs[hi & 7]);
			break;
		// 072xxx: ASH (not implemented on T-11)
		case 0072000: case 0072100: case 0072200: case 0072300: case 0072400: case 0072500: case 0072600: case 0072700:
			ea1 = MakeEA<false> (lo, pc, opcodes);
			util::stream_format(stream, "ASH   %s,%s", ea1, regs[hi & 7]);
			break;
		// 073xxx: ASHC (not implemented on T-11)
		case 0073000: case 0073100: case 0073200: case 0073300: case 0073400: case 0073500: case 0073600: case 0073700:
			ea1 = MakeEA<false> (lo, pc, opcodes);
			util::stream_format(stream, "ASHC  %s,%s", ea1, regs[hi & 7]);
			break;
		case 0074000: case 0074100: case 0074200: case 0074300: case 0074400: case 0074500: case 0074600: case 0074700:
			ea1 = MakeEA<false> (lo, pc, opcodes);
			util::stream_format(stream, "XOR   %s,%s", regs[hi & 7], ea1);
			break;
		// 075xxx: Floating Instruction Set (not implemented on T-11)
		// 076xxx: Commercial Instruction Set (not implemented on T-11)
		case 0077000: case 0077100: case 0077200: case 0077300: case 0077400: case 0077500: case 0077600: case 0077700:
			addr = (pc - 2 * lo) & 0177777;
			util::stream_format(stream, "SOB   %s,%06o", regs[hi & 7], addr);
			flags = STEP_COND;
			break;

		case 0100000: case 0100100: case 0100200: case 0100300:
			offset = 2 * (int8_t)(op & 0377);
			util::stream_format(stream, "BPL   %06o", (pc + offset) & 0177777);
			flags = STEP_COND;
			break;
		case 0100400: case 0100500: case 0100600: case 0100700:
			offset = 2 * (int8_t)(op & 0377);
			util::stream_format(stream, "BMI   %06o", (pc + offset) & 0177777);
			flags = STEP_COND;
			break;
		case 0101000: case 0101100: case 0101200: case 0101300:
			offset = 2 * (int8_t)(op & 0377);
			util::stream_format(stream, "BHI   %06o", (pc + offset) & 0177777);
			flags = STEP_COND;
			break;
		case 0101400: case 0101500: case 0101600: case 0101700:
			offset = 2 * (int8_t)(op & 0377);
			util::stream_format(stream, "BLOS  %06o", (pc + offset) & 0177777);
			flags = STEP_COND;
			break;
		case 0102000: case 0102100: case 0102200: case 0102300:
			offset = 2 * (int8_t)(op & 0377);
			util::stream_format(stream, "BVC   %06o", (pc + offset) & 0177777);
			flags = STEP_COND;
			break;
		case 0102400: case 0102500: case 0102600: case 0102700:
			offset = 2 * (int8_t)(op & 0377);
			util::stream_format(stream, "BVS   %06o", (pc + offset) & 0177777);
			flags = STEP_COND;
			break;
		case 0103000: case 0103100: case 0103200: case 0103300:
			offset = 2 * (int8_t)(op & 0377);
			util::stream_format(stream, "BCC   %06o", (pc + offset) & 0177777);
			flags = STEP_COND;
			break;
		case 0103400: case 0103500: case 0103600: case 0103700:
			offset = 2 * (int8_t)(op & 0377);
			util::stream_format(stream, "BCS   %06o", (pc + offset) & 0177777);
			flags = STEP_COND;
			break;
		case 0104000: case 0104100: case 0104200: case 0104300:
			util::stream_format(stream, "EMT   %03o", op & 0377);
			flags = STEP_OVER;
			break;
		case 0104400: case 0104500: case 0104600: case 0104700:
			util::stream_format(stream, "TRAP  %03o", op & 0377);
			flags = STEP_OVER;
			break;

		case 0105000:
			ea1 = MakeEA<true> (lo, pc, opcodes);
			util::stream_format(stream, "CLRB  %s", ea1);
			break;
		case 0105100:
			ea1 = MakeEA<true> (lo, pc, opcodes);
			util::stream_format(stream, "COMB  %s", ea1);
			break;
		case 0105200:
			ea1 = MakeEA<true> (lo, pc, opcodes);
			util::stream_format(stream, "INCB  %s", ea1);
			break;
		case 0105300:
			ea1 = MakeEA<true> (lo, pc, opcodes);
			util::stream_format(stream, "DECB  %s", ea1);
			break;
		case 0105400:
			ea1 = MakeEA<true> (lo, pc, opcodes);
			util::stream_format(stream, "NEGB  %s", ea1);
			break;
		case 0105500:
			ea1 = MakeEA<true> (lo, pc, opcodes);
			util::stream_format(stream, "ADCB  %s", ea1);
			break;
		case 0105600:
			ea1 = MakeEA<true> (lo, pc, opcodes);
			util::stream_format(stream, "SBCB  %s", ea1);
			break;
		case 0105700:
			ea1 = MakeEA<true> (lo, pc, opcodes);
			util::stream_format(stream, "TSTB  %s", ea1);
			break;
		case 0106000:
			ea1 = MakeEA<true> (lo, pc, opcodes);
			util::stream_format(stream, "RORB  %s", ea1);
			break;
		case 0106100:
			ea1 = MakeEA<true> (lo, pc, opcodes);
			util::stream_format(stream, "ROLB  %s", ea1);
			break;
		case 0106200:
			ea1 = MakeEA<true> (lo, pc, opcodes);
			util::stream_format(stream, "ASRB  %s", ea1);
			break;
		case 0106300:
			ea1 = MakeEA<true> (lo, pc, opcodes);
			util::stream_format(stream, "ASLB  %s", ea1);
			break;
		case 0106400:
			ea1 = MakeEA<true> (lo, pc, opcodes);
			util::stream_format(stream, "MTPS  %s", ea1);
			break;
		// 1065xx: MFPD (not implemented on T-11)
		// 1066xx: MTPD (not implemented on T-11)
		case 0106700:
			ea1 = MakeEA<true> (lo, pc, opcodes);
			util::stream_format(stream, "MFPS  %s", ea1);
			break;

		case 0110000: case 0110100: case 0110200: case 0110300: case 0110400: case 0110500: case 0110600: case 0110700:
		case 0111000: case 0111100: case 0111200: case 0111300: case 0111400: case 0111500: case 0111600: case 0111700:
		case 0112000: case 0112100: case 0112200: case 0112300: case 0112400: case 0112500: case 0112600: case 0112700:
		case 0113000: case 0113100: case 0113200: case 0113300: case 0113400: case 0113500: case 0113600: case 0113700:
		case 0114000: case 0114100: case 0114200: case 0114300: case 0114400: case 0114500: case 0114600: case 0114700:
		case 0115000: case 0115100: case 0115200: case 0115300: case 0115400: case 0115500: case 0115600: case 0115700:
		case 0116000: case 0116100: case 0116200: case 0116300: case 0116400: case 0116500: case 0116600: case 0116700:
		case 0117000: case 0117100: case 0117200: case 0117300: case 0117400: case 0117500: case 0117600: case 0117700:
			ea1 = MakeEA<true> (hi, pc, opcodes);
			ea2 = MakeEA<true> (lo, pc, opcodes);
			util::stream_format(stream, "MOVB  %s,%s", ea1, ea2);
			break;
		case 0120000: case 0120100: case 0120200: case 0120300: case 0120400: case 0120500: case 0120600: case 0120700:
		case 0121000: case 0121100: case 0121200: case 0121300: case 0121400: case 0121500: case 0121600: case 0121700:
		case 0122000: case 0122100: case 0122200: case 0122300: case 0122400: case 0122500: case 0122600: case 0122700:
		case 0123000: case 0123100: case 0123200: case 0123300: case 0123400: case 0123500: case 0123600: case 0123700:
		case 0124000: case 0124100: case 0124200: case 0124300: case 0124400: case 0124500: case 0124600: case 0124700:
		case 0125000: case 0125100: case 0125200: case 0125300: case 0125400: case 0125500: case 0125600: case 0125700:
		case 0126000: case 0126100: case 0126200: case 0126300: case 0126400: case 0126500: case 0126600: case 0126700:
		case 0127000: case 0127100: case 0127200: case 0127300: case 0127400: case 0127500: case 0127600: case 0127700:
			ea1 = MakeEA<true> (hi, pc, opcodes);
			ea2 = MakeEA<true> (lo, pc, opcodes);
			util::stream_format(stream, "CMPB  %s,%s", ea1, ea2);
			break;
		case 0130000: case 0130100: case 0130200: case 0130300: case 0130400: case 0130500: case 0130600: case 0130700:
		case 0131000: case 0131100: case 0131200: case 0131300: case 0131400: case 0131500: case 0131600: case 0131700:
		case 0132000: case 0132100: case 0132200: case 0132300: case 0132400: case 0132500: case 0132600: case 0132700:
		case 0133000: case 0133100: case 0133200: case 0133300: case 0133400: case 0133500: case 0133600: case 0133700:
		case 0134000: case 0134100: case 0134200: case 0134300: case 0134400: case 0134500: case 0134600: case 0134700:
		case 0135000: case 0135100: case 0135200: case 0135300: case 0135400: case 0135500: case 0135600: case 0135700:
		case 0136000: case 0136100: case 0136200: case 0136300: case 0136400: case 0136500: case 0136600: case 0136700:
		case 0137000: case 0137100: case 0137200: case 0137300: case 0137400: case 0137500: case 0137600: case 0137700:
			ea1 = MakeEA<true> (hi, pc, opcodes);
			ea2 = MakeEA<true> (lo, pc, opcodes);
			util::stream_format(stream, "BITB  %s,%s", ea1, ea2);
			break;
		case 0140000: case 0140100: case 0140200: case 0140300: case 0140400: case 0140500: case 0140600: case 0140700:
		case 0141000: case 0141100: case 0141200: case 0141300: case 0141400: case 0141500: case 0141600: case 0141700:
		case 0142000: case 0142100: case 0142200: case 0142300: case 0142400: case 0142500: case 0142600: case 0142700:
		case 0143000: case 0143100: case 0143200: case 0143300: case 0143400: case 0143500: case 0143600: case 0143700:
		case 0144000: case 0144100: case 0144200: case 0144300: case 0144400: case 0144500: case 0144600: case 0144700:
		case 0145000: case 0145100: case 0145200: case 0145300: case 0145400: case 0145500: case 0145600: case 0145700:
		case 0146000: case 0146100: case 0146200: case 0146300: case 0146400: case 0146500: case 0146600: case 0146700:
		case 0147000: case 0147100: case 0147200: case 0147300: case 0147400: case 0147500: case 0147600: case 0147700:
			ea1 = MakeEA<true> (hi, pc, opcodes);
			ea2 = MakeEA<true> (lo, pc, opcodes);
			util::stream_format(stream, "BICB  %s,%s", ea1, ea2);
			break;
		case 0150000: case 0150100: case 0150200: case 0150300: case 0150400: case 0150500: case 0150600: case 0150700:
		case 0151000: case 0151100: case 0151200: case 0151300: case 0151400: case 0151500: case 0151600: case 0151700:
		case 0152000: case 0152100: case 0152200: case 0152300: case 0152400: case 0152500: case 0152600: case 0152700:
		case 0153000: case 0153100: case 0153200: case 0153300: case 0153400: case 0153500: case 0153600: case 0153700:
		case 0154000: case 0154100: case 0154200: case 0154300: case 0154400: case 0154500: case 0154600: case 0154700:
		case 0155000: case 0155100: case 0155200: case 0155300: case 0155400: case 0155500: case 0155600: case 0155700:
		case 0156000: case 0156100: case 0156200: case 0156300: case 0156400: case 0156500: case 0156600: case 0156700:
		case 0157000: case 0157100: case 0157200: case 0157300: case 0157400: case 0157500: case 0157600: case 0157700:
			ea1 = MakeEA<true> (hi, pc, opcodes);
			ea2 = MakeEA<true> (lo, pc, opcodes);
			util::stream_format(stream, "BISB  %s,%s", ea1, ea2);
			break;
		case 0160000: case 0160100: case 0160200: case 0160300: case 0160400: case 0160500: case 0160600: case 0160700:
		case 0161000: case 0161100: case 0161200: case 0161300: case 0161400: case 0161500: case 0161600: case 0161700:
		case 0162000: case 0162100: case 0162200: case 0162300: case 0162400: case 0162500: case 0162600: case 0162700:
		case 0163000: case 0163100: case 0163200: case 0163300: case 0163400: case 0163500: case 0163600: case 0163700:
		case 0164000: case 0164100: case 0164200: case 0164300: case 0164400: case 0164500: case 0164600: case 0164700:
		case 0165000: case 0165100: case 0165200: case 0165300: case 0165400: case 0165500: case 0165600: case 0165700:
		case 0166000: case 0166100: case 0166200: case 0166300: case 0166400: case 0166500: case 0166600: case 0166700:
		case 0167000: case 0167100: case 0167200: case 0167300: case 0167400: case 0167500: case 0167600: case 0167700:
			ea1 = MakeEA<false> (hi, pc, opcodes);
			ea2 = MakeEA<false> (lo, pc, opcodes);
			util::stream_format(stream, "SUB   %s,%s", ea1, ea2);
			break;

		// 17xxxx: floating point (not implemented on T-11)

		default:
			util::stream_format(stream, ".WORD %06o", op);
			break;
	}

	return (pc - PC) | flags | SUPPORTED;
}

u32 t11_disassembler::opcode_alignment() const
{
	return 2;
}
