// license:BSD-3-Clause
// copyright-holders:Steve Ellenoff,R. Belmont,Ryan Holtz
/*****************************************************************************
 *
 *   arm7dasm.cpp
 *   Portable ARM7TDMI Core Emulator - Disassembler
 *
 *   Copyright Steve Ellenoff
 *
 *  This work is based on:
 *  #1) 'Atmel Corporation ARM7TDMI (Thumb) Datasheet - January 1999'
 *  #2) Arm 2/3/6 emulator By Bryan McPhail (bmcphail@tendril.co.uk) and Phil Stroffolino (MAME CORE 0.76)
 *
 *****************************************************************************/
/******************************************************************************
 *  Notes:
 *
 *  Because Co-Processor functions are highly specialized to the actual co-proc
 *  implementation being used, I've setup callback handlers to allow for custom
 *  dasm display of the co-proc functions so that the implementation specific
 *  commands/interpretation can be used. If not used, the default handlers which
 *  implement the ARM7TDMI guideline format is used
 ******************************************************************************/

#include "emu.h"
#include "arm7dasm.h"
#include "arm7core.h"

void arm7_disassembler::WritePadding(std::ostream &stream, std::streampos start_position)
{
	std::streamoff difference = stream.tellp() - start_position;
	if (difference >= 8)
		stream << ' ';
	for (std::streamoff i = difference; i < 8; i++)
		stream << ' ';
}

void arm7_disassembler::DasmCoProc_RT(std::ostream &stream, uint32_t opcode, const char *pConditionCode, std::streampos start_position)
{
	/* co processor register transfer */
	/* xxxx 1110 oooL nnnn dddd cccc ppp1 mmmm */
	if( opcode&0x00100000 )     //Bit 20 = Load or Store
		stream << "MRC";
	else
		stream << "MCR";
	stream << pConditionCode;
	WritePadding(stream, start_position);
	util::stream_format( stream, "p%d, %d, R%d, c%d, c%d",
					(opcode>>8)&0xf, (opcode>>21)&7, (opcode>>12)&0xf, (opcode>>16)&0xf, opcode&0xf );
	if((opcode>>5)&7) util::stream_format( stream, ", %d",(opcode>>5)&7);
}

void arm7_disassembler::DasmCoProc_DT(std::ostream &stream, uint32_t opcode, const char *pConditionCode, std::streampos start_position)
{
	/* co processor data transfer */
	/* xxxx 111P UNWL nnnn dddd pppp oooooooo */
	//todo: test this on valid instructions

	util::stream_format(stream, "%s%s",(opcode&0x00100000)?"LDC":"STC",pConditionCode);   //Bit 20 = 1 for Load, 0 for Store
	//Long Operation
	if(opcode & 0x400000)   util::stream_format(stream, "L");
	WritePadding(stream, start_position);

	//P# & CD #
	util::stream_format(stream, "p%d, c%d, ",(opcode>>8)&0x0f,(opcode>>12)&0x0f);

	//Base Register (Rn)
	util::stream_format(stream, "[R%d%s",(opcode>>16)&0x0f,(opcode&0x1000000)?"":"]");    //If Bit 24 = 1, Pre-increment, otherwise, Post increment so close brace

	//immediate value ( 8 bit value is << 2 according to manual )
	uint16_t imm = opcode & 0xff;
	if(imm != 0)
		util::stream_format(stream, ", #%s%s%X", (opcode&0x800000)?"":"-", (imm << 2) > 9 ? "0x" : "", imm << 2);

	//Pre-Inc brace & Write back
	if(opcode&0x1000000)
	{
		stream << ']';
		if(opcode&0x200000)
			stream << '!';
	}
}

void arm7_disassembler::DasmCoProc_DO(std::ostream &stream, uint32_t opcode, const char *pConditionCode, std::streampos start_position)
{
	/* co processor data operation */
	/* xxxx 1110 oooo nnnn dddd cccc ppp0 mmmm */
	util::stream_format( stream, "CDP%s", pConditionCode );
	WritePadding(stream, start_position);
	//p#,CPOpc,cd,cn,cm
	util::stream_format( stream, "p%d, %d, c%d, c%d, c%d",
		(opcode>>8)&0xf, (opcode>>20)&0xf, (opcode>>12)&0xf, (opcode>>16)&0xf, opcode&0xf );
	if((opcode>>5)&7) util::stream_format(stream, ", %d",(opcode>>5)&7);
}

uint32_t arm7_disassembler::ExtractImmediateOperand( uint32_t opcode )
{
	/* rrrrbbbbbbbb */
	uint32_t imm = opcode&0xff;
	int r = ((opcode>>8)&0xf)*2;
	return std::rotr(imm, r);
}

static const char *const pRegOp[4] = { "LSL","LSR","ASR","ROR" };

void arm7_disassembler::WriteShiftCount( std::ostream &stream, int type, int count, bool printType )
{
	if (count == 0)
	{
		if (type == 0)
		{
			// ignore LSL #0 for register shift
			if (printType)
				return;
		}
		else if (type == 3)
		{
			// RRX does not take a count
			if (printType)
				stream << ", RRX";
			return;
		}
		else
			count = 32;
	}

	if (printType)
		util::stream_format(stream, ", %s #%d", pRegOp[type], count);
	else
		util::stream_format(stream, ", #%d", count);
}

void arm7_disassembler::WriteDataProcessingOperand( std::ostream &stream, uint32_t opcode, bool printOp0, bool printOp1 )
{
	/* ccccctttmmmm */
	if (printOp0)
		util::stream_format(stream, "R%d, ", (opcode>>12)&0xf);
	if (printOp1)
		util::stream_format(stream, "R%d, ", (opcode>>16)&0xf);

	/* Immediate Op2 */
	if (opcode & 0x02000000)
	{
		uint32_t imm = ExtractImmediateOperand(opcode);
		util::stream_format( stream, "#%s%X", imm > 9 ? "0x" : "", imm );
		return;
	}

	/* Register Op2 */
	util::stream_format(stream, "R%d", (opcode>>0)&0xf);

	if( opcode&0x10 ) /* Shift amount specified in bottom bits of RS */
	{
		util::stream_format( stream, ", %s R%d", pRegOp[(opcode>>5)&3], (opcode>>8)&0xf );
	}
	else /* Shift amount immediate 5 bit unsigned integer */
	{
		WriteShiftCount(stream, (opcode>>5)&3, (opcode>>7)&0x1f, true);
	}
}

void arm7_disassembler::WriteRegisterOperand1( std::ostream &stream, uint32_t opcode )
{
	/* ccccctttmmmm */
	util::stream_format(
		stream,
		", %sR%d", /* Operand 1 register, (optional) sign, Operand 2 register, shift type */
		(opcode&0x800000)?"":"-",
		(opcode >> 0) & 0xf);

	if( opcode&0x10 ) /* Shift amount specified in bottom bits of RS */
	{
		util::stream_format( stream, ", %s R%d", pRegOp[(opcode>>5)&3], (opcode>>8)&0xf );
	}
	else /* Shift amount immediate 5 bit unsigned integer */
	{
		WriteShiftCount(stream, (opcode>>5)&3, (opcode>>7)&0x1f, true);
	}
} /* WriteRegisterOperand */

void arm7_disassembler::WriteRegisterList( std::ostream &stream, uint16_t operand )
{
	stream << '{';

	int j=0,last=0,found=0;
	for (j=0; j<16; j++) {
		if (operand&(1<<j) && found==0) {
			if (operand&((1<<j)-1))
				stream << ", ";
			found=1;
			last=j;
		}
		else if ((operand&(1<<j))==0 && found) {
			util::stream_format(stream, "R%d", last);
			if (last!=j-1)
				util::stream_format(stream, "-R%d", j-1);
			found=0;
		}
	}
	if (found) {
		if (last != 15)
			util::stream_format(stream, "R%d-", last);
		stream << "R15";
	}

	stream << '}';
}


void arm7_disassembler::WriteBranchAddress( std::ostream &stream, uint32_t pc, uint32_t opcode, bool h_bit )
{
	opcode <<= 2;
	if (h_bit && (opcode & 0x04000000))
	{
		opcode |= 2;
	}
	opcode = util::sext( opcode, 26 );
	pc += 8+opcode;
	util::stream_format( stream, "0x%08X", pc );
} /* WriteBranchAddress */

static const char *const pConditionCodeTable[16] =
{
	"EQ","NE","CS","CC",
	"MI","PL","VS","VC",
	"HI","LS","GE","LT",
	"GT","LE","","NV"
};

bool arm7_disassembler::dasm_armv6(std::ostream &stream, u32 opcode, const char *condition, std::streampos start_position, u32 &flags)
{
	if (opcode >= 0xf0000000)
	{
		if (opcode == 0xf57ff01f)
			stream << "CLREX";
		else if ((opcode & 0xfffffdff) == 0xf1010000)
		{
			stream << "SETEND";
			WritePadding(stream, start_position);
			stream << (BIT(opcode, 9) ? "BE" : "LE");
		}
		else if ((opcode & 0xfff1fe20) == 0xf1000000)
		{
			unsigned const imod = (opcode >> 18) & 3;
			bool const mode = BIT(opcode, 17);
			if (imod == 1 || (!mode && (opcode & 31)) || (imod ? !(opcode & 0x1c0) : (!mode || (opcode & 0x1c0))))
				return false;
			util::stream_format(stream, "CPS%s", imod == 3 ? "ID" : imod == 2 ? "IE" : "");
			WritePadding(stream, start_position);
			if (BIT(opcode, 8))
				stream << 'a';
			if (BIT(opcode, 7))
				stream << 'i';
			if (BIT(opcode, 6))
				stream << 'f';
			if (mode)
				util::stream_format(stream, "%s#0x%X", imod ? ", " : "", opcode & 31);
		}
		else if ((opcode & 0xfe5fffe0) == 0xf84d0500)
		{
			util::stream_format(stream, "SRS%c%c", BIT(opcode, 23) ? 'I' : 'D', BIT(opcode, 24) ? 'B' : 'A');
			WritePadding(stream, start_position);
			util::stream_format(stream, "SP%s, #0x%X", BIT(opcode, 21) ? "!" : "", opcode & 31);
		}
		else if ((opcode & 0xfe50ffff) == 0xf8100a00)
		{
			util::stream_format(stream, "RFE%c%c", BIT(opcode, 23) ? 'I' : 'D', BIT(opcode, 24) ? 'B' : 'A');
			WritePadding(stream, start_position);
			util::stream_format(stream, "R%d%s", (opcode >> 16) & 15, BIT(opcode, 21) ? "!" : "");
			flags = STEP_OUT;
		}
		else
			return false;
		return true;
	}

	unsigned const rd = (opcode >> 12) & 15, rn = (opcode >> 16) & 15, rm = opcode & 15;
	if ((opcode & 0x0f800ff0) == 0x01800f90)
	{
		bool const load = BIT(opcode, 20);
		unsigned const kind = (opcode >> 21) & 3;
		if (rn == 15 || rd == 15 || (load ? rm != 15 : rm == 15) || (kind == 1 && ((load ? rd : rm) & 1)))
			return false;
		static const char *const SUFFIXES[] = { "", "D", "B", "H" };
		util::stream_format(stream, "%s%s%s", load ? "LDREX" : "STREX", SUFFIXES[kind], condition);
		WritePadding(stream, start_position);
		util::stream_format(stream, "R%d, ", rd);
		if (!load)
			util::stream_format(stream, "R%d, ", rm);
		if (kind == 1)
			util::stream_format(stream, "R%d, ", (load ? rd : rm) + 1);
		util::stream_format(stream, "[R%d]", rn);
		return true;
	}
	if ((opcode & 0x0ff000f0) == 0x00400090)
	{
		util::stream_format(stream, "UMAAL%s", condition);
		WritePadding(stream, start_position);
		util::stream_format(stream, "R%d, R%d, R%d, R%d", rd, rn, rm, (opcode >> 8) & 15);
		return true;
	}
	return dasm_armv6_media(stream, opcode, condition, start_position);
}

bool arm7_disassembler::dasm_armv6_media(std::ostream &stream, u32 opcode, const char *condition, std::streampos start_position)
{
	unsigned const rd = (opcode >> 12) & 15, rn = (opcode >> 16) & 15, rm = opcode & 15, rs = (opcode >> 8) & 15;
	u32 const reverse = opcode & 0x0fff0ff0;
	if (reverse == 0x06bf0f30 || reverse == 0x06bf0fb0 || reverse == 0x06ff0fb0)
	{
		util::stream_format(stream, "%s%s", reverse == 0x06bf0f30 ? "REV" : reverse == 0x06bf0fb0 ? "REV16" : "REVSH", condition);
		WritePadding(stream, start_position);
		util::stream_format(stream, "R%d, R%d", rd, rm);
	}
	else if ((opcode & 0x0f8003f0) == 0x06800070 && ((opcode >> 20) & 3) != 1)
	{
		unsigned const kind = (opcode >> 20) & 3, rotate = ((opcode >> 10) & 3) * 8;
		util::stream_format(stream, "%cXT%s%s%s", BIT(opcode, 22) ? 'U' : 'S', rn == 15 ? "" : "A",
				kind == 0 ? "B16" : kind == 2 ? "B" : "H", condition);
		WritePadding(stream, start_position);
		util::stream_format(stream, "R%d, ", rd);
		if (rn != 15)
			util::stream_format(stream, "R%d, ", rn);
		util::stream_format(stream, "R%d", rm);
		if (rotate)
			util::stream_format(stream, ", ROR #%d", rotate);
	}
	else if ((opcode & 0x0ff00030) == 0x06800010)
	{
		util::stream_format(stream, "PKH%s%s", BIT(opcode, 6) ? "TB" : "BT", condition);
		WritePadding(stream, start_position);
		util::stream_format(stream, "R%d, R%d, R%d", rd, rn, rm);
		WriteShiftCount(stream, BIT(opcode, 6) ? 2 : 0, (opcode >> 7) & 31, true);
	}
	else if ((opcode & 0x0fa00030) == 0x06a00010 || (opcode & 0x0fb00ff0) == 0x06a00f30)
	{
		bool const half = (opcode & 0x0fb00ff0) == 0x06a00f30, uns = BIT(opcode, 22);
		util::stream_format(stream, "%cSAT%s%s", uns ? 'U' : 'S', half ? "16" : "", condition);
		WritePadding(stream, start_position);
		util::stream_format(stream, "R%d, #%d, R%d", rd, ((opcode >> 16) & (half ? 15 : 31)) + (uns ? 0 : 1), rm);
		if (!half)
			WriteShiftCount(stream, BIT(opcode, 6) ? 2 : 0, (opcode >> 7) & 31, true);
	}
	else if ((opcode & 0x0ff00ff0) == 0x06800fb0)
	{
		util::stream_format(stream, "SEL%s", condition);
		WritePadding(stream, start_position);
		util::stream_format(stream, "R%d, R%d, R%d", rd, rn, rm);
	}
	else if ((opcode & 0x0f800f10) == 0x06000f10)
	{
		unsigned const group = (opcode >> 20) & 7, op = (opcode >> 5) & 7;
		if (!(group & 3) || op == 5 || op == 6)
			return false;
		static const char *const PREFIXES[] = { "", "S", "Q", "SH", "", "U", "UQ", "UH" };
		static const char *const OPERATIONS[] = { "ADD16", "ASX", "SAX", "SUB16", "ADD8", "", "", "SUB8" };
		util::stream_format(stream, "%s%s%s", PREFIXES[group], OPERATIONS[op], condition);
		WritePadding(stream, start_position);
		util::stream_format(stream, "R%d, R%d, R%d", rd, rn, rm);
	}
	else if ((opcode & 0x0fb00090) == 0x07000010)
	{
		bool const wide = BIT(opcode, 22), sub = BIT(opcode, 6);
		util::stream_format(stream, "%s%s%s", wide ? (sub ? "SMLSLD" : "SMLALD")
				: rd == 15 ? (sub ? "SMUSD" : "SMUAD") : (sub ? "SMLSD" : "SMLAD"), BIT(opcode, 5) ? "X" : "", condition);
		WritePadding(stream, start_position);
		if (wide)
			util::stream_format(stream, "R%d, R%d, R%d, R%d", rd, rn, rm, rs);
		else
		{
			util::stream_format(stream, "R%d, R%d, R%d", rn, rm, rs);
			if (rd != 15)
				util::stream_format(stream, ", R%d", rd);
		}
	}
	else if ((opcode & 0x0ff000d0) == 0x07500010 || (opcode & 0x0ff000d0) == 0x075000d0)
	{
		if (BIT(opcode, 7) && rd == 15)
			return false;
		util::stream_format(stream, "%s%s%s", BIT(opcode, 7) ? "SMMLS" : rd == 15 ? "SMMUL" : "SMMLA", BIT(opcode, 5) ? "R" : "", condition);
		WritePadding(stream, start_position);
		util::stream_format(stream, "R%d, R%d, R%d", rn, rm, rs);
		if (rd != 15)
			util::stream_format(stream, ", R%d", rd);
	}
	else if ((opcode & 0x0ff000f0) == 0x07800010)
	{
		util::stream_format(stream, "%s%s", rd == 15 ? "USAD8" : "USADA8", condition);
		WritePadding(stream, start_position);
		util::stream_format(stream, "R%d, R%d, R%d", rn, rm, rs);
		if (rd != 15)
			util::stream_format(stream, ", R%d", rd);
	}
	else
		return false;
	return true;
}

bool arm7_disassembler::dasm_thumbv6(std::ostream &stream, u16 opcode, std::streampos start_position)
{
	if ((opcode & 0xff00) == 0xb200 || ((opcode & 0xff00) == 0xba00 && ((opcode >> 6) & 3) != 2))
	{
		static const char *const EXTEND[] = { "SXTH", "SXTB", "UXTH", "UXTB" };
		static const char *const REVERSE[] = { "REV", "REV16", "", "REVSH" };
		stream << (BIT(opcode, 11) ? REVERSE : EXTEND)[(opcode >> 6) & 3];
		WritePadding(stream, start_position);
		util::stream_format(stream, "R%d, R%d", opcode & 7, (opcode >> 3) & 7);
	}
	else if ((opcode & 0xfff7) == 0xb650)
	{
		stream << "SETEND";
		WritePadding(stream, start_position);
		stream << (BIT(opcode, 3) ? "BE" : "LE");
	}
	else if ((opcode & 0xffe8) == 0xb660 && (opcode & 7))
	{
		stream << (BIT(opcode, 4) ? "CPSID" : "CPSIE");
		WritePadding(stream, start_position);
		if (BIT(opcode, 2))
			stream << 'a';
		if (BIT(opcode, 1))
			stream << 'i';
		if (BIT(opcode, 0))
			stream << 'f';
	}
	else
		return false;
	return true;
}

bool arm7_disassembler::dasm_vfp_transfer(std::ostream &stream, u32 opcode, const char *condition, std::streampos start_position)
{
	unsigned const rd = (opcode >> 12) & 15, rn = (opcode >> 16) & 15, rm = opcode & 15;
	bool const load = BIT(opcode, 20), dp = BIT(opcode, 8);
	if ((opcode & 0x0f000010) == 0x0e000010 && !dp)
	{
		if ((opcode & 0x00e0007f) == 0x10 && rd != 15)
		{
			util::stream_format(stream, "VMOV%s", condition);
			WritePadding(stream, start_position);
			unsigned const sn = rn * 2 + BIT(opcode, 7);
			if (load)
				util::stream_format(stream, "R%d, S%d", rd, sn);
			else
				util::stream_format(stream, "S%d, R%d", sn, rd);
			return true;
		}
		if ((opcode & 0x00e000ff) == 0x00e00010)
		{
			static const char *const SYSTEM_REGISTERS[] = { "FPSID", "FPSCR", nullptr, nullptr, nullptr, nullptr,
					"MVFR1", "MVFR0", "FPEXC", "FPINST", "FPINST2" };
			if (rn >= std::size(SYSTEM_REGISTERS) || !SYSTEM_REGISTERS[rn] || (rd == 15 && !(load && rn == 1)))
				return false;
			util::stream_format(stream, "%s%s", load ? "VMRS" : "VMSR", condition);
			WritePadding(stream, start_position);
			if (!load)
				util::stream_format(stream, "%s, R%d", SYSTEM_REGISTERS[rn], rd);
			else if (rd == 15)
				stream << "APSR_nzcv, FPSCR";
			else
				util::stream_format(stream, "R%d, %s", rd, SYSTEM_REGISTERS[rn]);
			return true;
		}
	}
	else if ((opcode & 0x0fe000d0) == 0x0c400010)
	{
		unsigned const sm = rm * 2 + BIT(opcode, 5);
		if (rd == 15 || rn == 15 || (dp ? BIT(opcode, 5) : sm == 31))
			return false;
		util::stream_format(stream, "VMOV%s", condition);
		WritePadding(stream, start_position);
		if (load)
			util::stream_format(stream, "R%d, R%d, ", rd, rn);
		if (dp)
			util::stream_format(stream, "D%d", rm);
		else
			util::stream_format(stream, "S%d, S%d", sm, sm + 1);
		if (!load)
			util::stream_format(stream, ", R%d, R%d", rd, rn);
		return true;
	}
	return false;
}

bool arm7_disassembler::dasm_vfp_data(std::ostream &stream, u32 opcode, const char *condition, std::streampos start_position)
{
	bool const dp = BIT(opcode, 8);
	unsigned const rd = (opcode >> 12) & 15, rn = (opcode >> 16) & 15, rm = opcode & 15;
	unsigned const sd = rd * 2 + BIT(opcode, 22), sn = rn * 2 + BIT(opcode, 7), sm = rm * 2 + BIT(opcode, 5);
	unsigned const d = dp ? rd : sd, n = dp ? rn : sn, m = dp ? rm : sm;
	unsigned const operation = (opcode >> 20) & 0xb;
	char const reg = dp ? 'D' : 'S';
	unsigned const precision = dp ? 64 : 32;
	if (operation <= 3 || (operation == 8 && !BIT(opcode, 6)))
	{
		if (dp && (opcode & 0x004000a0))
			return false; // VFP2 has D0-D15, not the VFP3 D16-D31 bank.
		static const char *const OPERATIONS[] = { "VMLA", "VMLS", "VNMLS", "VNMLA", "VMUL", "VNMUL", "VADD", "VSUB" };
		util::stream_format(stream, "%s%s.F%d", operation == 8 ? "VDIV" : OPERATIONS[operation * 2 + BIT(opcode, 6)], condition, precision);
		WritePadding(stream, start_position);
		util::stream_format(stream, "%c%d, %c%d, %c%d", reg, d, reg, n, reg, m);
		return true;
	}
	if (operation != 11 || !BIT(opcode, 6))
		return false;
	if (rn <= 1)
	{
		if (dp && (opcode & 0x00400020))
			return false;
		static const char *const OPERATIONS[] = { "VMOV", "VABS", "VNEG", "VSQRT" };
		util::stream_format(stream, "%s%s.F%d", OPERATIONS[rn * 2 + BIT(opcode, 7)], condition, precision);
		WritePadding(stream, start_position);
		util::stream_format(stream, "%c%d, %c%d", reg, d, reg, m);
	}
	else if (rn == 4 || rn == 5)
	{
		if ((dp && (opcode & 0x00400020)) || (rn == 5 && (opcode & 0x2f)))
			return false;
		util::stream_format(stream, "VCMP%s%s.F%d", BIT(opcode, 7) ? "E" : "", condition, precision);
		WritePadding(stream, start_position);
		util::stream_format(stream, "%c%d, ", reg, d);
		if (rn == 5)
			stream << "#0.0";
		else
			util::stream_format(stream, "%c%d", reg, m);
	}
	else if (rn == 7 && BIT(opcode, 7))
	{
		if (BIT(opcode, dp ? 5 : 22))
			return false;
		util::stream_format(stream, "VCVT%s.F%d.F%d", condition, dp ? 32 : 64, precision);
		WritePadding(stream, start_position);
		if (dp)
			util::stream_format(stream, "S%d, D%d", sd, rm);
		else
			util::stream_format(stream, "D%d, S%d", rd, sm);
	}
	else if (rn == 8)
	{
		if (dp && BIT(opcode, 22))
			return false;
		util::stream_format(stream, "VCVT%s.F%d.%c32", condition, precision, BIT(opcode, 7) ? 'S' : 'U');
		WritePadding(stream, start_position);
		util::stream_format(stream, "%c%d, S%d", reg, d, sm);
	}
	else if (rn == 12 || rn == 13)
	{
		if (dp && BIT(opcode, 5))
			return false;
		util::stream_format(stream, "VCVT%s%s.%c32.F%d", BIT(opcode, 7) ? "" : "R", condition, rn == 13 ? 'S' : 'U', precision);
		WritePadding(stream, start_position);
		util::stream_format(stream, "S%d, %c%d", sd, reg, m);
	}
	else
		return false;
	return true;
}

bool arm7_disassembler::dasm_vfp(std::ostream &stream, u32 opcode, const char *condition, std::streampos start_position)
{
	if (((opcode >> 8) & 14) != 10)
		return false;
	if (dasm_vfp_transfer(stream, opcode, condition, start_position))
		return true;
	if ((opcode & 0x0f000010) == 0x0e000000)
		return dasm_vfp_data(stream, opcode, condition, start_position);
	// Unrecognized double-register transfers must not become VLDM/VSTM.
	if ((opcode & 0x0e000000) != 0x0c000000 || (opcode & 0x0fe00000) == 0x0c400000)
		return false;
	bool const dp = BIT(opcode, 8), load = BIT(opcode, 20), pre = BIT(opcode, 24), up = BIT(opcode, 23), wb = BIT(opcode, 21);
	unsigned const rd = (opcode >> 12) & 15, rn = (opcode >> 16) & 15;
	unsigned const first = dp ? rd : rd * 2 + BIT(opcode, 22), count = (opcode & 255) >> (dp ? 1 : 0);
	char const reg = dp ? 'D' : 'S';
	if (dp && BIT(opcode, 22))
		return false;
	if (pre && !wb)
	{
		util::stream_format(stream, "%s%s", load ? "VLDR" : "VSTR", condition);
		WritePadding(stream, start_position);
		util::stream_format(stream, "%c%d, [R%d", reg, first, rn);
		unsigned const offset = (opcode & 255) * 4;
		if (offset || !up)
			util::stream_format(stream, ", #%s%s%X", up ? "" : "-", offset > 9 ? "0x" : "", offset);
		stream << ']';
	}
	else
	{
		if (!count || first + count > (dp ? 16 : 32) || !(up ? !pre : (pre && wb)) || (wb && rn == 15))
			return false;
		// Odd doubleword counts encode the VFP2 extended register-save form.
		if (dp && BIT(opcode, 0))
			util::stream_format(stream, "%s%sX%s", load ? "FLDM" : "FSTM", up ? "IA" : "DB", condition);
		else
			util::stream_format(stream, "%s%s%s", load ? "VLDM" : "VSTM", up ? "IA" : "DB", condition);
		WritePadding(stream, start_position);
		util::stream_format(stream, "R%d%s, {%c%d", rn, wb ? "!" : "", reg, first);
		if (count > 1)
			util::stream_format(stream, "-%c%d", reg, first + count - 1);
		stream << '}';
	}
	return true;
}

u32 arm7_disassembler::arm7_disasm( std::ostream &stream, uint32_t pc, uint32_t opcode )
{
	static const char *const pOperation[16] =
	{
		"AND","EOR","SUB","RSB",
		"ADD","ADC","SBC","RSC",
		"TST","TEQ","CMP","CMN",
		"ORR","MOV","BIC","MVN"
	};
	const char *pConditionCode = pConditionCodeTable[opcode>>28];
	uint32_t dasmflags = 0;
	std::streampos start_position = stream.tellp();
	const u8 arch = m_config->get_arch_rev();
	if (arch >= 6 && dasm_armv6(stream, opcode, pConditionCode, start_position, dasmflags))
		return 4 | dasmflags | SUPPORTED;
	if (m_config->get_vfp_flag() && opcode < 0xf0000000 && dasm_vfp(stream, opcode, pConditionCode, start_position))
		return 4 | SUPPORTED;

	// Decode double-register transfers before the overlapping LDC/STC space.
	if (arch >= 5 && (opcode & 0x0fe00000) == 0x0c400000 && (opcode < 0xf0000000 || arch >= 6))
	{
		util::stream_format(stream, "%s%s", BIT(opcode, 20) ? "MRRC" : "MCRR", opcode >= 0xf0000000 ? "2" : pConditionCode);
		WritePadding(stream, start_position);
		util::stream_format(stream, "p%d, %d, R%d, R%d, c%d", (opcode >> 8) & 15, (opcode >> 4) & 15,
				(opcode >> 12) & 15, (opcode >> 16) & 15, opcode & 15);
		return 4 | SUPPORTED;
	}

	if( arch >= 3 && (opcode&0xfe000000)==0xfa000000 ) //bits 31-25 == 1111 101 (BLX - v5)
	{
		/* BLX(1) */
		util::stream_format( stream, "BLX" );
		dasmflags = STEP_OVER;

		WritePadding(stream, start_position);

		WriteBranchAddress( stream, pc, opcode, true );
	}
	else if( arch >= 3 && (opcode&0x0ff000f0)==0x01200030 )  // (BLX - v5)
	{
		/* BLX(2) */
		stream << "BLX";
		dasmflags = STEP_OVER;
		if (opcode < 0xe0000000)
			dasmflags |= STEP_COND;
		WritePadding(stream, start_position);
		util::stream_format( stream, "R%d",(opcode&0xf));
	}
	else if( arch >= 3 && (opcode&0x0ffffff0)==0x012fff10 ) //bits 27-4 == 000100101111111111110001
	{
		/* Branch and Exchange (BX) */
		util::stream_format( stream, "B%sX", pConditionCode );
		WritePadding(stream, start_position);
		util::stream_format( stream, "R%d",(opcode&0xf));
		if ((opcode & 0x0f) == 14)
			dasmflags = STEP_OUT;
		if (opcode < 0xe0000000)
			dasmflags |= STEP_COND;
	}
	else if (arch >= 3 && (opcode & 0x0ff000f0) == 0x01600010)   // CLZ - v5
	{
		stream << "CLZ";
		WritePadding(stream, start_position);
		util::stream_format(stream, "R%d, R%d", (opcode>>12)&0xf, opcode&0xf);
	}
	else if (arch >= 3 && (opcode & 0x0f9000f0) == 0x01000050)   // Q(D)ADD, Q(D)SUB - v5TE
	{
		util::stream_format(stream, "Q%s%s", (opcode & 0x00400000) != 0 ? "D" : "", (opcode & 0x00200000) != 0 ? "SUB" : "ADD");
		WritePadding(stream, start_position);
		if (((opcode>>12)&0xf) != (opcode&0xf))
			util::stream_format(stream, "R%d, ", (opcode>>12)&0xf);
		util::stream_format(stream, "R%d, R%d", opcode&0xf, (opcode>>16)&0xf);
	}
	else if (arch >= 3 && (opcode & 0x0ff00090) == 0x01000080)   // SMLAxy - v5TE
	{
		util::stream_format(stream, "SMLA%c%c", (opcode&0x20) ? 'T' : 'B', (opcode&0x40) ? 'T' : 'B');
		WritePadding(stream, start_position);
		util::stream_format(stream, "R%d, R%d, R%d, R%d", (opcode>>16)&0xf, opcode&0xf, (opcode>>8)&0xf, (opcode>>12)&0xf);   // Rd, Rm, Rs, Rn
	}
	else if (arch >= 3 && (opcode & 0x0ff00090) == 0x01400080)   // SMLALxy - v5TE
	{
		util::stream_format(stream, "SMLAL%c%c", (opcode&0x20) ? 'T' : 'B', (opcode&0x40) ? 'T' : 'B');
		WritePadding(stream, start_position);
		util::stream_format(stream, "R%d, R%d, R%d, R%d", (opcode>>12)&0xf, (opcode>>16)&0xf, opcode&0xf, (opcode>>8)&0xf);   // RdLo, RdHi, Rm, Rs
	}
	else if (arch >= 3 && (opcode & 0x0ff00090) == 0x01600080)   // SMULxy - v5TE
	{
		util::stream_format(stream, "SMUL%c%c", (opcode&0x20) ? 'T' : 'B', (opcode&0x40) ? 'T' : 'B');
		WritePadding(stream, start_position);
		util::stream_format(stream, "R%d, R%d, R%d", (opcode>>16)&0xf, opcode&0xf, (opcode>>8)&0xf);   // Rd, Rm, Rs
	}
	else if (arch >= 3 && (opcode & 0x0ff000b0) == 0x012000a0)   // SMULWy - v5TE
	{
		util::stream_format(stream, "SMULW%c", (opcode&0x40) ? 'T' : 'B');
		WritePadding(stream, start_position);
		util::stream_format(stream, "R%d, R%d, R%d", (opcode>>16)&0xf, opcode&0xf, (opcode>>8)&0xf);
	}
	else if (arch >= 3 && (opcode & 0x0ff000b0) == 0x01200080)   // SMLAWy - v5TE
	{
		util::stream_format(stream, "SMLAW%c", (opcode&0x40) ? 'T' : 'B');
		WritePadding(stream, start_position);
		util::stream_format(stream, "R%d, R%d, R%d, R%d", (opcode>>16)&0xf, opcode&0xf, (opcode>>8)&0xf, (opcode>>12)&0xf);
	}
	else if( arch < 3 && (opcode&0x0e000000)==0 && (opcode&0x90)==0x90 && (arch < 2 || (opcode&0x60) || (opcode&0x01800000)==0x00800000) )
	{
		// ARM1 has no multiply or swap; ARM2/ARM3 have only MUL/MLA (and SWP) in this space - the halfword transfers (v4)
		// and the long multiplies (v3M) are undefined
		stream << "Undefined";
	}
	else if( (opcode&0x0e000000)==0 && (opcode&0x80) && (opcode&0x10) ) //bits 27-25 == 000, bit 7=1, bit 4=1
	{
		/* multiply or swap or half word data transfer */
		if(opcode&0x60)
		{   //bits = 6-5 != 00
			/* half word data transfer */
			if (((opcode & 0x60) == 0x40) && !(opcode & 0x100000))  // bit 20 = 0, bits 5&6 = 10 is ARMv5TE LDRD
			{
				stream << "LDRD";
			}
			else if (((opcode & 0x60) == 0x60) && !(opcode & 0x100000)) // bit 20 = 0, bits 5&6 = 11 is ARMv5TE STRD
			{
				stream << "STRD";
			}
			else
			{
				//Bit 20 = 1 for Load, 0 for Store
				if (opcode & 0x00100000)
					stream << "LDR";
				else
					stream << "STR";

				//Signed? (if not, always unsigned half word)
				if(opcode&0x40)
					util::stream_format(stream, "S%c", (opcode & 0x20) ? 'H' : 'B');    //Bit 5 = 1 for Half Word, 0 for Byte
				else
					stream << 'H';
			}
			stream << pConditionCode;

			WritePadding(stream, start_position);

			//Dest Register
			util::stream_format(stream, "R%d, ",(opcode>>12)&0x0f);
			//Base Register
			int rn = (opcode>>16)&0x0f;
			if (rn == 15 && (opcode&0x1400000) == 0x1400000) {
				uint8_t imm = ((opcode>>4)&0xf0) | (opcode&0x0f);
				util::stream_format(stream, "0x%08X", uint32_t(pc + 8 + ((opcode&0x800000) ? imm : -imm)));
			}
			else {
				util::stream_format(stream, "[R%d%s", rn, (opcode&0x1000000)?"":"]");    //If Bit 24 = 1, Pre-increment, otherwise, Post increment so close brace

				//Immediate or Register Offset?
				if(opcode&0x400000) {           //Bit 22 - 1 = immediate, 0 = register
					//immediate         ( imm. value in high nibble (bits 8-11) and lo nibble (bit 0-3) )
					uint8_t imm = ((opcode>>4)&0xf0) | (opcode&0x0f);
					if(imm)
						util::stream_format(stream, ", #%s%s%X", (opcode&0x800000)?"":"-", imm > 9 ? "0x" : "", imm);
				}
				else {
					//register
					util::stream_format(stream, ", %sR%d", (opcode&0x800000)?"":"-", opcode & 0x0f);
				}
			}

			//Pre-Inc brace & Write back
			if(opcode&0x1000000)
			{
				stream << ']';
				if(opcode&0x200000)
					stream << '!';
			}
		}
		else {
			if(opcode&0x01000000) {     //bit 24 = 1
			/* swap */
			//todo: Test on valid instructions
				/* xxxx 0001 0B00 nnnn dddd 0000 1001 mmmm */
				util::stream_format( stream, "SWP%s%s", pConditionCode, (opcode & 0x400000)?"B":"" );    //Bit 22 = Byte/Word selection
				WritePadding(stream, start_position);

				//Rd, Rm, [Rn]
				util::stream_format( stream, "R%d, R%d, [R%d]",
								(opcode>>12)&0xf, opcode&0xf, (opcode>>16)&0xf );
			}
			else {
				/* multiply or multiply long */

				if( opcode&0x800000 )   //Bit 23 = 1 for Multiply Long
				{
					/* Multiply Long */
					/* xxxx0001 UAShhhhllllnnnn1001mmmm */

					/* Signed? */
					if( opcode&0x00400000 )
						stream << 'S';
					else
						stream << 'U';

					/* Multiply & Accumulate? */
					if( opcode&0x00200000 )
					{
						stream << "MLAL";
					}
					else
					{
						stream << "MULL";
					}
					stream << pConditionCode;

					/* Set Status Flags */
					if( opcode&0x00100000 )
					{
						stream << 'S';
					}
					WritePadding(stream, start_position);

					//Format is RLo,RHi,Rm,Rs
					util::stream_format(stream,
						"R%d, R%d, R%d, R%d",
						(opcode>>12)&0xf,
						(opcode>>16)&0xf,
						opcode&0xf,
						(opcode>>8)&0xf);
				}
				else
				{
					/* Multiply */
					/* xxxx0000 00ASdddd nnnnssss 1001mmmm */

					/* Multiply & Accumulate? */
					if( opcode&0x00200000 )
					{
						stream << "MLA";
					}
					/* Multiply */
					else
					{
						stream << "MUL";
					}
					stream << pConditionCode;
					if( opcode&0x00100000 )
					{
						stream << 'S';
					}
					WritePadding(stream, start_position);

					util::stream_format(stream,
						"R%d, R%d, R%d",
						(opcode>>16)&0xf,
						opcode&0xf,
						(opcode>>8)&0xf);

					if( opcode&0x00200000 )
					{
						util::stream_format( stream, ", R%d", (opcode>>12)&0xf );
					}
				}
			}
		}
	}
	else if( (opcode&0x0c000000)==0 )       //bits 27-26 == 00 - This check can only exist properly after Multiplication check above
	{
		/* Data Processing OR PSR Transfer */

		//SJE: check for MRS & MSR ( S bit must be clear, and bit 24,23 = 10 )
		if( arch >= 3 && ((opcode&0x00100000)==0) && ((opcode&0x01800000)==0x01000000) )
		{
			if ((opcode & 0xf26000f0) == 0xe0200070)
			{
				stream << "BKPT"; // v5
				WritePadding(stream, start_position);
				util::stream_format(stream, "0x%04X", (opcode & 0xfff00) >> 4 | (opcode & 0xf));
				dasmflags = STEP_OVER;
			}
			else
			{
				std::string strpsr = util::string_format("%cPSR", (opcode&0x400000) ? 'S' : 'C');

				//MSR ( bit 21 set )
				if( (opcode&0x00200000) ) {
					util::stream_format(stream, "MSR%s",pConditionCode );
					if( (opcode&0xf0000) != 0xf0000)
					{
						strpsr += '_';
						if (BIT(opcode, 19))
							strpsr += 'f';
						if (BIT(opcode, 18))
							strpsr += 'x';
						if (BIT(opcode, 17))
							strpsr += 's';
						if (BIT(opcode, 16))
							strpsr += 'c';
					}
					WritePadding(stream, start_position);
					util::stream_format(stream, "%s, ", strpsr);
					WriteDataProcessingOperand(stream, opcode, false, false);
				}
				//MRS ( bit 21 clear )
				else {
					util::stream_format(stream, "MRS%s", pConditionCode );
					WritePadding(stream, start_position);
					util::stream_format(stream, "R%d, %s", (opcode>>12)&0x0f, strpsr);
				}
			}
		}
		else {
			/* Data Processing */
			/* xxxx001a aaaSnnnn ddddrrrr bbbbbbbb */
			/* xxxx000a aaaSnnnn ddddcccc ctttmmmm */
			int op=(opcode>>21)&0xf;
			bool is_adr = (op == 0x02 || op == 0x04) && ((opcode >> 16) & 0x0f) == 15 && (opcode & 0x02000000) != 0;
			bool is_shift = (op == 0x0d) && (opcode & 0x02000000) == 0 && (opcode & 0x00000fe0) != 0;
			if( is_adr )
				stream << "ADR";
			else if( is_shift )
			{
				int type = (opcode>>5) & 3;
				if ( type == 3 && (opcode & 0x00000f90) == 0 )
					stream << "RRX";
				else
					stream << pRegOp[type];
			}
			else
				stream << pOperation[op];
			stream << pConditionCode;

			//SJE: corrected S-Bit bug here
			//if( (opcode&0x01000000) )
			if( (opcode&0x0100000) && (op & 0x0c) != 0x08 )
			{
				stream << 'S';
			}
			else if( arch < 3 && (opcode&0x0100000) && ((opcode>>12)&0xf) == 15 )
			{
				stream << 'P';      // ARM2/ARM3: TSTP/TEQP/CMPP/CMNP write the result to the PSR in R15
			}

			WritePadding(stream, start_position);

			switch (op) {
			case 0x02:
				// check for SUBS PC, LR, #imm
				if (((opcode >> 12) & 0x0f) == 15)
				{
					if (((opcode >> 0) & 0x0f) == 14 && (opcode & 0x02100000) == 0x02100000)
						dasmflags = STEP_OUT;
					if (opcode < 0xe0000000)
						dasmflags |= STEP_COND;
				}
				[[fallthrough]];
			case 0x04:
				if (is_adr)
				{
					uint32_t adr = pc + 8;
					if (op == 0x02)
						adr -= ExtractImmediateOperand(opcode);
					else
						adr += ExtractImmediateOperand(opcode);
					util::stream_format( stream, "R%d, 0x%08X", (opcode >> 12) & 0x0f, adr );
					break;
				}
				[[fallthrough]];
			case 0x00:
			case 0x01:
			case 0x03:
			case 0x05:
			case 0x06:
			case 0x07:
			case 0x0c:
			case 0x0e:
				WriteDataProcessingOperand(stream, opcode, ((opcode>>12)&0xf) != ((opcode>>16)&0xf), true);
				break;
			case 0x08:
			case 0x09:
			case 0x0a:
			case 0x0b:
				WriteDataProcessingOperand(stream, opcode, false, true);
				break;
			case 0x0d:
				if (is_shift)
				{
					uint8_t rd = (opcode >> 12) & 0x0f;
					uint8_t rs = (opcode >> 0) & 0x0f;
					if ( rd != rs )
						util::stream_format( stream, "R%d, ", rd );
					util::stream_format( stream, "R%d", rs );
					if( opcode&0x10 ) /* Shift amount specified in bottom bits of RS */
					{
						util::stream_format( stream, ", R%d", (opcode>>8)&0xf );
					}
					else /* Shift amount immediate 5 bit unsigned integer */
					{
						WriteShiftCount( stream, (opcode>>5)&3, (opcode>>7)&0x1f, false );
					}
					break;
				}
				/* look for mov pc,lr */
				if (((opcode >> 12) & 0x0f) == 15)
				{
					if (((opcode >> 0) & 0x0f) == 14 && (opcode & 0x02000000) == 0)
						dasmflags = STEP_OUT;
					if (opcode < 0xe0000000)
						dasmflags |= STEP_COND;
				}
				[[fallthrough]];
			case 0x0f:
				WriteDataProcessingOperand(stream, opcode, true, false);
				break;
			}
		}
	}
	else if( (opcode&0x0c000000)==0x04000000 )      //bits 27-26 == 01
	{
		/* Data Transfer */

		/* xxxx010P UBWLnnnn ddddoooo oooooooo  Immediate form */
		/* xxxx011P UBWLnnnn ddddcccc ctt0mmmm  Register form */
		if ((opcode & 0xf050f000) == 0xf050f000)
			stream << "PLD"; // preload hint - v5TE
		else
		{
			if( opcode&0x00100000 )
				stream << "LDR";
			else
				stream << "STR";
			stream << pConditionCode;

			if( opcode&0x00400000 )
			{
				stream << 'B';
			}
			if( (opcode&0x01200000) == 0x00200000 )
			{
				stream << 'T';
			}
		}

		WritePadding(stream, start_position);
		if ((opcode & 0xf050f000) != 0xf050f000)
			util::stream_format( stream, "R%d, ", (opcode>>12)&0xf);

		int rn = (opcode>>16)&0xf;
		if( opcode&0x02000000 )
		{
			/* register form */
			util::stream_format( stream, "[R%d%s", rn, (opcode&0x01000000)?"":"]" );
			WriteRegisterOperand1(stream, opcode);
			if( opcode&0x01000000 )
				stream << ']';
		}
		else if(rn==15)
		{
			uint32_t rnv = pc + 8;
			if( opcode&0x00800000 )
				rnv += opcode&0xfff;
			else
				rnv -= opcode&0xfff;
			util::stream_format( stream, "0x%08X", rnv);
		}
		else
		{
			/* immediate form */
			util::stream_format( stream, "[R%d%s", rn, (opcode&0x01000000)?"":"]" );

			//hide zero offsets
			if(opcode&0xfff) {
				stream << ", #";
				if( !(opcode&0x00800000 ))
					stream << '-';
				if( (opcode&0xfff) > 9)
					stream << "0x";
				util::stream_format( stream, "%X", opcode&0xfff );
			}
			if( opcode&0x01000000 )
				stream << ']';
		}

		if ( (opcode&0x01200000) == 0x01200000 )
		{
			/* writeback addr */
			stream << '!';
		}
	}
	else if( (opcode&0x0e000000) == 0x08000000 )        //bits 27-25 == 100
	{
		/* xxxx100P USWLnnnn llllllll llllllll */
		/* Block Data Transfer */

		int rn = (opcode>>16)&0xf;
		if( opcode&0x00100000 )
		{
			util::stream_format( stream, "LDM%s", pConditionCode );
			if (rn == 13)
				util::stream_format( stream, "%c%c", (opcode&0x01000000) ? 'E' : 'F', (opcode&0x00800000) ? 'D' : 'A');
			else
				util::stream_format( stream, "%c%c", (opcode&0x00800000) ? 'I' : 'D', (opcode&0x01000000) ? 'B' : 'A');
			if (opcode & 0x00008000)
			{
				dasmflags = STEP_OUT;
				if (opcode < 0xe0000000)
					dasmflags |= STEP_COND;
			}
		}
		else
		{
			util::stream_format( stream, "STM%s", pConditionCode );
			if (rn == 13)
				util::stream_format( stream, "%c%c", (opcode&0x01000000) ? 'F' : 'E', (opcode&0x00800000) ? 'A' : 'D');
			else
				util::stream_format( stream, "%c%c", (opcode&0x00800000) ? 'I' : 'D', (opcode&0x01000000) ? 'B' : 'A');
		}

		WritePadding(stream, start_position);
		util::stream_format( stream, "R%d", rn );
		if( opcode&0x00200000 )
			stream << '!';
		stream << ", ";

		WriteRegisterList(stream, opcode & 0x0000ffff);

		if( opcode&0x00400000 )
		{
			util::stream_format( stream, "^" );
		}
	}
	else if( (opcode&0x0e000000)==0x0a000000 )      //bits 27-25 == 101
	{
		/* branch instruction */
		/* xxxx101L oooooooo oooooooo oooooooo */
		if( opcode&0x01000000 )
		{
			util::stream_format( stream, "BL" );
			dasmflags = STEP_OVER;
		}
		else
		{
			util::stream_format( stream, "B" );
		}

		stream << pConditionCode;
		if (opcode < 0xe0000000)
			dasmflags |= STEP_COND;

		WritePadding(stream, start_position);

		WriteBranchAddress( stream, pc, opcode, false );
	}
	else if( (opcode&0x0e000000)==0x0c000000 )      //bits 27-25 == 110
	{
		/* co processor data transfer */
		if (opcode >= 0xf0000000)
			DasmCoProc_DT(stream, opcode, "2", start_position); // LDC2, STC2 - v5
		else
			DasmCoProc_DT(stream, opcode, pConditionCode, start_position);
	}
	else if( (opcode&0x0f000000)==0x0e000000 )      //bits 27-24 == 1110
	{
		/* co processor data operation or register transfer */

		//Register Transfer
		if(opcode&0x10)
		{
			DasmCoProc_RT(stream, opcode, arch >= 5 && opcode >= 0xf0000000 ? "2" : pConditionCode, start_position);
		}
		//Data Op
		else
		{
			DasmCoProc_DO(stream, opcode, arch >= 5 && opcode >= 0xf0000000 ? "2" : pConditionCode, start_position);
		}
	}
	else if( (opcode&0x0f000000) == 0x0f000000 )    //bits 27-24 == 1111
	{
		/* Software Interrupt */
		util::stream_format( stream, "SWI%s", pConditionCode );
		WritePadding(stream, start_position);
		util::stream_format( stream, "0x%X", opcode&0x00ffffff );
		dasmflags = STEP_OVER;
		if (opcode < 0xe0000000)
			dasmflags |= STEP_COND;
	}
	else
	{
		util::stream_format( stream, "Undefined" );
	}
	return 4 | dasmflags | SUPPORTED;
}

u32 arm7_disassembler::thumb_disasm(std::ostream &stream, uint32_t pc, uint16_t opcode)
{
	std::streampos start_position = stream.tellp();
	uint32_t dasmflags = 0;
	if (m_config->get_arch_rev() >= 6 && dasm_thumbv6(stream, opcode, start_position))
		return 2 | SUPPORTED;

//  uint32_t readword;
	uint32_t addr;
	uint16_t rm, rn, rs, rd, imm;//, rrs;
	int32_t offs;

	switch( ( opcode & THUMB_INSN_TYPE ) >> THUMB_INSN_TYPE_SHIFT )
	{
	case 0x0: /* Logical shifting */
		rs = ( opcode & THUMB_ADDSUB_RS ) >> THUMB_ADDSUB_RS_SHIFT;
		rd = ( opcode & THUMB_ADDSUB_RD ) >> THUMB_ADDSUB_RD_SHIFT;
		offs = ( opcode & THUMB_SHIFT_AMT ) >> THUMB_SHIFT_AMT_SHIFT;
		if( opcode & THUMB_SHIFT_R ) /* Shift right */
		{
			stream << "LSR";
			WritePadding(stream, start_position);
			if( rd != rs )
				util::stream_format( stream, "R%d, ", rd);
			util::stream_format( stream, "R%d, #%d", rs, (offs == 0) ? 32 : offs);
		}
		else /* Shift left */
		{
			stream << "LSL";
			WritePadding(stream, start_position);
			if( rd != rs )
				util::stream_format( stream, "R%d, ", rd);
			util::stream_format( stream, "R%d, #%d", rs, offs);
		}
		break;
	case 0x1: /* Arithmetic */
		rs = ( opcode & THUMB_ADDSUB_RS ) >> THUMB_ADDSUB_RS_SHIFT;
		rd = ( opcode & THUMB_ADDSUB_RD ) >> THUMB_ADDSUB_RD_SHIFT;
		if( opcode & THUMB_INSN_ADDSUB )
		{
			switch( ( opcode & THUMB_ADDSUB_TYPE ) >> THUMB_ADDSUB_TYPE_SHIFT )
			{
			case 0x0: /* ADD Rd, Rs, Rn */
				rn = ( opcode & THUMB_ADDSUB_RNIMM ) >> THUMB_ADDSUB_RNIMM_SHIFT;
				stream << "ADD";
				WritePadding(stream, start_position);
				if( rd != rs )
					util::stream_format( stream, "R%d, ", rd );
				util::stream_format( stream, "R%d, R%d", rs, rn );
				break;
			case 0x1: /* SUB Rd, Rs, Rn */
				rn = ( opcode & THUMB_ADDSUB_RNIMM ) >> THUMB_ADDSUB_RNIMM_SHIFT;
				stream << "SUB";
				WritePadding(stream, start_position);
				if( rd != rs )
					util::stream_format( stream, "R%d, ", rd );
				util::stream_format( stream, "R%d, R%d", rs, rn );
				break;
			case 0x2: /* ADD Rd, Rs, #imm */
				imm = ( opcode & THUMB_ADDSUB_RNIMM ) >> THUMB_ADDSUB_RNIMM_SHIFT;
				stream << "ADD";
				WritePadding(stream, start_position);
				if( rd != rs )
					util::stream_format( stream, "R%d, ", rd );
				util::stream_format( stream, "R%d, #%d", rs, imm );
				break;
			case 0x3: /* SUB Rd, Rs, #imm */
				imm = ( opcode & THUMB_ADDSUB_RNIMM ) >> THUMB_ADDSUB_RNIMM_SHIFT;
				stream << "SUB";
				WritePadding(stream, start_position);
				if( rd != rs )
					util::stream_format( stream, "R%d, ", rd );
				util::stream_format( stream, "R%d, #%d", rs, imm );
				break;
			default:
				util::stream_format(stream, "INVALID %04x", opcode);
				break;
			}
		}
		else
		{
			offs = ( opcode & THUMB_SHIFT_AMT ) >> THUMB_SHIFT_AMT_SHIFT;
			stream << "ASR";
			WritePadding(stream, start_position);
			if( rd != rs )
				util::stream_format( stream, "R%d, ", rd);
			util::stream_format( stream, "R%d, #%d", rs, offs);
		}
		break;
	case 0x2: /* CMP / MOV */
		rn = ( opcode & THUMB_INSN_IMM_RD ) >> THUMB_INSN_IMM_RD_SHIFT;
		imm = ( opcode & THUMB_INSN_IMM );
		if( opcode & THUMB_INSN_CMP )
			stream << "CMP";
		else
			stream << "MOV";
		WritePadding(stream, start_position);
		util::stream_format( stream, "R%d, #%s%X", rn, imm > 9 ? "0x" : "", imm );
		break;
	case 0x3: /* ADD/SUB immediate */
		rn = ( opcode & THUMB_INSN_IMM_RD ) >> THUMB_INSN_IMM_RD_SHIFT;
		imm = opcode & THUMB_INSN_IMM;
		if( opcode & THUMB_INSN_SUB ) /* SUB Rd, #Offset8 */
			stream << "SUB";
		else
			stream << "ADD";
		WritePadding(stream, start_position);
		util::stream_format( stream, "R%d, #%s%X", rn, imm > 9 ? "0x" : "", imm ); // fixed, rd -> rn
		break;
	case 0x4: /* Rd & Rm instructions */
		switch( ( opcode & THUMB_GROUP4_TYPE ) >> THUMB_GROUP4_TYPE_SHIFT )
		{
		case 0x0:
			rs = ( opcode & THUMB_ADDSUB_RS ) >> THUMB_ADDSUB_RS_SHIFT;
			rd = ( opcode & THUMB_ADDSUB_RD ) >> THUMB_ADDSUB_RD_SHIFT;
			switch( ( opcode & THUMB_ALUOP_TYPE ) >> THUMB_ALUOP_TYPE_SHIFT )
			{
				case 0x0: /* AND Rd, Rs */
					stream << "AND";
					WritePadding(stream, start_position);
					util::stream_format( stream, "R%d, R%d", rd, rs );
					break;
				case 0x1: /* EOR Rd, Rs */
					stream << "EOR";
					WritePadding(stream, start_position);
					util::stream_format( stream, "R%d, R%d", rd, rs );
					break;
				case 0x2: /* LSL Rd, Rs */
					stream << "LSL";
					WritePadding(stream, start_position);
					util::stream_format( stream, "R%d, R%d", rd, rs );
					break;
				case 0x3: /* LSR Rd, Rs */
					stream << "LSR";
					WritePadding(stream, start_position);
					util::stream_format( stream, "R%d, R%d", rd, rs );
					break;
				case 0x4: /* ASR Rd, Rs */
					stream << "ASR";
					WritePadding(stream, start_position);
					util::stream_format( stream, "R%d, R%d", rd, rs );
					break;
				case 0x5: /* ADC Rd, Rs */
					stream << "ADC";
					WritePadding(stream, start_position);
					util::stream_format( stream, "R%d, R%d", rd, rs );
					break;
				case 0x6: /* SBC Rd, Rs */
					stream << "SBC";
					WritePadding(stream, start_position);
					util::stream_format( stream, "R%d, R%d", rd, rs );
					break;
				case 0x7: /* ROR Rd, Rs */
					stream << "ROR";
					WritePadding(stream, start_position);
					util::stream_format( stream, "R%d, R%d", rd, rs );
					break;
				case 0x8: /* TST Rd, Rs */
					stream << "TST";
					WritePadding(stream, start_position);
					util::stream_format( stream, "R%d, R%d", rd, rs );
					break;
				case 0x9: /* NEG Rd, Rs */
					stream << "NEG";
					WritePadding(stream, start_position);
					util::stream_format( stream, "R%d, R%d", rd, rs );
					break;
				case 0xa: /* CMP Rd, Rs */
					stream << "CMP";
					WritePadding(stream, start_position);
					util::stream_format( stream, "R%d, R%d", rd, rs );
					break;
				case 0xb: /* CMN Rd, Rs - check flags, add dasm */
					stream << "CMN";
					WritePadding(stream, start_position);
					util::stream_format( stream, "R%d, R%d", rd, rs );
					break;
				case 0xc: /* ORR Rd, Rs */
					stream << "ORR";
					WritePadding(stream, start_position);
					util::stream_format( stream, "R%d, R%d", rd, rs );
					break;
				case 0xd: /* MUL Rd, Rs */
					stream << "MUL";
					WritePadding(stream, start_position);
					util::stream_format( stream, "R%d, R%d", rd, rs );
					break;
				case 0xe: /* BIC Rd, Rs */
					stream << "BIC";
					WritePadding(stream, start_position);
					util::stream_format( stream, "R%d, R%d", rd, rs );
					break;
				case 0xf: /* MVN Rd, Rs */
					stream << "MVN";
					WritePadding(stream, start_position);
					util::stream_format( stream, "R%d, R%d", rd, rs );
					break;
				default:
					util::stream_format(stream, "INVALID %04x", opcode);
					break;
			}
			break;
		case 0x1:
			switch( ( opcode & THUMB_HIREG_OP ) >> THUMB_HIREG_OP_SHIFT )
			{
			case 0x0: /* ADD Rd, Rs */
				rs = ( opcode & THUMB_HIREG_RS ) >> THUMB_HIREG_RS_SHIFT;
				rd = opcode & THUMB_HIREG_RD;
				switch( ( opcode & THUMB_HIREG_H ) >> THUMB_HIREG_H_SHIFT )
				{
				case 0x1: /* ADD Rd, HRs */
					stream << "ADD";
					WritePadding(stream, start_position);
					util::stream_format( stream, "R%d, R%d", rd, rs + 8 );
					break;
				case 0x2: /* ADD HRd, Rs */
					stream << "ADD";
					WritePadding(stream, start_position);
					util::stream_format( stream, "R%d, R%d", rd + 8, rs );
					break;
				case 0x3: /* ADD HRd, HRs */
					stream << "ADD";
					WritePadding(stream, start_position);
					util::stream_format( stream, "R%d, R%d", rd + 8, rs + 8 );
					break;
				default:
					util::stream_format(stream, "INVALID %04x", opcode);
					break;
				}
				break;
			case 0x1: /* CMP */
				rs = ( opcode & THUMB_HIREG_RS ) >> THUMB_HIREG_RS_SHIFT;
				rd = opcode & THUMB_HIREG_RD;
				switch( ( opcode & THUMB_HIREG_H ) >> THUMB_HIREG_H_SHIFT )
				{
				case 0x0: /* CMP Rd, Rs */
					stream << "CMP";
					WritePadding(stream, start_position);
					util::stream_format( stream, "R%d, R%d", rd, rs );
					break;
				case 0x1: /* CMP Rd, HRs */
					stream << "CMP";
					WritePadding(stream, start_position);
					util::stream_format( stream, "R%d, R%d", rd, rs + 8 );
					break;
				case 0x2: /* CMP Hd, Rs */
					stream << "CMP";
					WritePadding(stream, start_position);
					util::stream_format( stream, "R%d, R%d", rd + 8, rs );
					break;
				case 0x3: /* CMP Hd, Hs */
					stream << "CMP";
					WritePadding(stream, start_position);
					util::stream_format( stream, "R%d, R%d", rd + 8, rs + 8 );
					break;
				default:
					util::stream_format(stream, "INVALID %04x", opcode);
					break;
				}
				break;
			case 0x2: /* MOV */
				rs = ( opcode & THUMB_HIREG_RS ) >> THUMB_HIREG_RS_SHIFT;
				rd = opcode & THUMB_HIREG_RD;
				switch( ( opcode & THUMB_HIREG_H ) >> THUMB_HIREG_H_SHIFT )
				{
				case 0x0:
					stream << "MOV";
					WritePadding(stream, start_position);
					util::stream_format( stream, "R%d, R%d", rd, rs );
					break;
				case 0x1:
					stream << "MOV";
					WritePadding(stream, start_position);
					util::stream_format( stream, "R%d, R%d", rd, rs + 8 );
					break;
				case 0x2:
					stream << "MOV";
					WritePadding(stream, start_position);
					util::stream_format( stream, "R%d, R%d", rd + 8, rs );
					break;
				case 0x3:
					stream << "MOV";
					WritePadding(stream, start_position);
					util::stream_format( stream, "R%d, R%d", rd + 8, rs + 8 );
					break;
				default:
					util::stream_format(stream, "INVALID %04x", opcode);
					break;
				}
				break;
			case 0x3:
				switch( ( opcode & THUMB_HIREG_H ) >> THUMB_HIREG_H_SHIFT )
				{
				case 0x0:
					rd = ( opcode & THUMB_HIREG_RS ) >> THUMB_HIREG_RS_SHIFT;
					stream << "BX";
					WritePadding(stream, start_position);
					util::stream_format( stream, "R%d", rd );
					break;
				case 0x1:
					rd = ( ( opcode & THUMB_HIREG_RS ) >> THUMB_HIREG_RS_SHIFT ) + 8;
					stream << "BX";
					WritePadding(stream, start_position);
					util::stream_format( stream, "R%d", rd );
					if (rd == 14)
						dasmflags = STEP_OUT;
					break;
				case 0x2:
					rd = ( opcode & THUMB_HIREG_RS ) >> THUMB_HIREG_RS_SHIFT;
					stream << "BLX";
					WritePadding(stream, start_position);
					util::stream_format( stream, "R%d", rd );
					dasmflags = STEP_OVER;
					break;
				default:
					util::stream_format(stream, "INVALID %04x", opcode);
					break;
				}
				break;
			default:
				util::stream_format(stream, "INVALID %04x", opcode);
				break;
			}
			break;
		case 0x2:
		case 0x3:
			rd = ( opcode & THUMB_INSN_IMM_RD ) >> THUMB_INSN_IMM_RD_SHIFT;
			addr = uint32_t( opcode & THUMB_INSN_IMM ) << 2;
			stream << "LDR";
			WritePadding(stream, start_position);
			util::stream_format( stream, "R%d, 0x%08X", rd, (pc + 4 + addr) & 0xfffffffc );
			break;
		default:
			util::stream_format(stream, "INVALID %04x", opcode);
			break;
		}
		break;
	case 0x5: /* LDR* STR* */
		rm = ( opcode & THUMB_GROUP5_RM ) >> THUMB_GROUP5_RM_SHIFT;
		rn = ( opcode & THUMB_GROUP5_RN ) >> THUMB_GROUP5_RN_SHIFT;
		rd = ( opcode & THUMB_GROUP5_RD ) >> THUMB_GROUP5_RD_SHIFT;
		switch( ( opcode & THUMB_GROUP5_TYPE ) >> THUMB_GROUP5_TYPE_SHIFT )
		{
		case 0x0: /* STR Rd, [Rn, Rm] */
			stream << "STR";
			WritePadding(stream, start_position);
			util::stream_format( stream, "R%d, [R%d, R%d]", rd, rn, rm );
			break;
		case 0x1: /* STRH Rd, [Rn, Rm] */
			stream << "STRH";
			WritePadding(stream, start_position);
			util::stream_format( stream, "R%d, [R%d, R%d]", rd, rn, rm );
			break;
		case 0x2: /* STRB Rd, [Rn, Rm] */ /* check */
			stream << "STRB";
			WritePadding(stream, start_position);
			util::stream_format( stream, "R%d, [R%d, R%d]", rd, rn, rm );
			break;
		case 0x3: /* LDRSB Rd, [Rn, Rm] */
			stream << "LDRSB";
			WritePadding(stream, start_position);
			util::stream_format( stream, "R%d, [R%d, R%d]", rd, rn, rm );
			break;
		case 0x4: /* LDR Rd, [Rn, Rm] */ /* check */
			stream << "LDR";
			WritePadding(stream, start_position);
			util::stream_format( stream, "R%d, [R%d, R%d]", rd, rn, rm );
			break;
		case 0x5: /* LDRH Rd, [Rn, Rm] */
			stream << "LDRH";
			WritePadding(stream, start_position);
			util::stream_format( stream, "R%d, [R%d, R%d]", rd, rn, rm );
			break;
		case 0x6: /* LDRB Rd, [Rn, Rm] */
			stream << "LDRB";
			WritePadding(stream, start_position);
			util::stream_format( stream, "R%d, [R%d, R%d]", rd, rn, rm );
			break;
		case 0x7: /* LDRSH Rd, [Rn, Rm] */
			stream << "LDRSH";
			WritePadding(stream, start_position);
			util::stream_format( stream, "R%d, [R%d, R%d]", rd, rn, rm );
			break;
		default:
			util::stream_format(stream, "INVALID %04x", opcode);
			break;
		}
		break;
	case 0x6: /* Word Store w/ Immediate Offset */
		rn = ( opcode & THUMB_ADDSUB_RS ) >> THUMB_ADDSUB_RS_SHIFT;
		rd = opcode & THUMB_ADDSUB_RD;
		offs = ( ( opcode & THUMB_LSOP_OFFS ) >> THUMB_LSOP_OFFS_SHIFT ) << 2;
		if( opcode & THUMB_LSOP_L ) /* Load */
			stream << "LDR";
		else /* Store */
			stream << "STR";
		WritePadding(stream, start_position);
		util::stream_format( stream, "R%d, [R%d", rd, rn );
		if ( offs != 0 )
			util::stream_format( stream, ", #%s%X", offs > 9 ? "0x" : "", offs );
		stream << ']';
		break;
	case 0x7: /* Byte Store w/ Immeidate Offset */
		rn = ( opcode & THUMB_ADDSUB_RS ) >> THUMB_ADDSUB_RS_SHIFT;
		rd = opcode & THUMB_ADDSUB_RD;
		offs = ( opcode & THUMB_LSOP_OFFS ) >> THUMB_LSOP_OFFS_SHIFT;
		if( opcode & THUMB_LSOP_L ) /* Load */
			stream << "LDRB";
		else /* Store */
			stream << "STRB";
		WritePadding(stream, start_position);
		util::stream_format( stream, "R%d, [R%d", rd, rn );
		if ( offs != 0 )
			util::stream_format( stream, ", #%s%X", offs > 9 ? "0x" : "", offs );
		stream << ']';
		break;
	case 0x8: /* Load/Store Halfword */
		imm = ( opcode & THUMB_HALFOP_OFFS ) >> THUMB_HALFOP_OFFS_SHIFT;
		rs = ( opcode & THUMB_ADDSUB_RS ) >> THUMB_ADDSUB_RS_SHIFT;
		rd = ( opcode & THUMB_ADDSUB_RD ) >> THUMB_ADDSUB_RD_SHIFT;
		if( opcode & THUMB_HALFOP_L ) /* Load */
			stream << "LDRH";
		else /* Store */
			stream << "STRH";
		WritePadding(stream, start_position);
		util::stream_format( stream, "R%d, [R%d", rd, rs );
		if ( imm != 0 )
			util::stream_format( stream, ", #%s%X", (imm << 1) > 9 ? "0x" : "", imm << 1 );
		stream << ']';
		break;
	case 0x9: /* Stack-Relative Load/Store */
		rd = ( opcode & THUMB_STACKOP_RD ) >> THUMB_STACKOP_RD_SHIFT;
		offs = opcode & THUMB_INSN_IMM;
		if( opcode & THUMB_STACKOP_L )
			stream << "LDR";
		else
			stream << "STR";
		WritePadding(stream, start_position);
		util::stream_format( stream, "R%d, [SP", rd );
		if ( offs != 0 )
			util::stream_format( stream, ", #%s%X", (offs << 2) > 9 ? "0x" : "", offs << 2);
		stream << ']';
		break;
	case 0xa: /* Get relative address */
		rd = ( opcode & THUMB_RELADDR_RD ) >> THUMB_RELADDR_RD_SHIFT;
		offs = uint32_t( opcode & THUMB_INSN_IMM ) << 2;
		if( opcode & THUMB_RELADDR_SP ) /* ADD Rd, SP, #nn */
		{
			stream << "ADD";
			WritePadding(stream, start_position);
			util::stream_format( stream, "R%d, SP, #%s%X", rd, (offs << 2) > 9 ? "0x" : "", offs );
		}
		else /* ADD Rd, PC, #nn */
		{
			stream << "ADR";
			WritePadding(stream, start_position);
			util::stream_format( stream, "R%d, 0x%08X", rd, (pc + 4 + offs) & 0xfffffffc );
		}
		break;
	case 0xb: /* Stack-Related Opcodes */
		switch( ( opcode & THUMB_STACKOP_TYPE ) >> THUMB_STACKOP_TYPE_SHIFT )
		{
		case 0x0: /* ADD SP, #imm */
			addr = ( opcode & THUMB_INSN_IMM );
			addr &= ~THUMB_INSN_IMM_S;
			if( opcode & THUMB_INSN_IMM_S )
				stream << "SUB";
			else
				stream << "ADD";
			WritePadding(stream, start_position);
			util::stream_format( stream, "SP, #%s%X", (addr << 2) > 9 ? "0x" : "", addr << 2);
			break;
		case 0x4: /* PUSH {Rlist} */
		case 0x5: /* PUSH {Rlist}{LR} */
			stream << "PUSH";
			WritePadding(stream, start_position);
			WriteRegisterList(stream, (opcode & 0x100) << 6 | (opcode & 0xff));
			break;
		case 0xc: /* POP {Rlist} */
		case 0xd: /* POP {Rlist}{PC} */
			stream << "POP";
			WritePadding(stream, start_position);
			WriteRegisterList(stream, (opcode & 0x100) << 7 | (opcode & 0xff));
			if (opcode & 0x100)
				dasmflags = STEP_OUT;
			break;
		default:
			util::stream_format(stream, "INVALID %04x", opcode);
			break;
		}
		break;
	case 0xc: /* Multiple Load/Store */
		rd = ( opcode & THUMB_MULTLS_BASE ) >> THUMB_MULTLS_BASE_SHIFT;
		if( opcode & THUMB_MULTLS ) /* Load */
			stream << "LDMIA";
		else /* Store */
			stream << "STMIA";
		WritePadding(stream, start_position);
		util::stream_format( stream, "R%d!, {", rd);
		for( offs = 0; offs < 8; offs++ )
		{
			if( opcode & ( 1 << offs ) )
			{
				if( opcode & ((1 << offs) - 1) )
					stream << ", ";
				util::stream_format(stream, "R%d", offs);
			}
		}
		stream << '}';
		break;
	case 0xd: /* Conditional Branch */
		switch( ( opcode & THUMB_COND_TYPE ) >> THUMB_COND_TYPE_SHIFT )
		{
		case COND_AL:
			stream << "BKPT"; // v5
			WritePadding(stream, start_position);
			util::stream_format( stream, "0x%02X", opcode & 0xff);
			dasmflags = STEP_OVER;
			break;
		case COND_NV:
			stream << "SWI";
			WritePadding(stream, start_position);
			util::stream_format( stream, "0x%02X", opcode & 0xff);
			dasmflags = STEP_OVER;
			break;
		default:
			offs = (int8_t)( opcode & THUMB_INSN_IMM );
			util::stream_format( stream, "B%s", pConditionCodeTable[( opcode & THUMB_COND_TYPE ) >> THUMB_COND_TYPE_SHIFT]);
			WritePadding(stream, start_position);
			util::stream_format( stream, "0x%08X", pc + 4 + (offs << 1));
			dasmflags = STEP_COND;
			break;
		}
		break;
	case 0xe: /* B #offs */
		if( opcode & THUMB_BLOP_LO )
		{
			addr = ( ( opcode & THUMB_BLOP_OFFS ) << 1 ) & 0xfffc;
			util::stream_format( stream, "BLX (LO) 0x%X", addr );
			dasmflags = STEP_OVER;
		}
		else
		{
			offs = util::sext( ( opcode & THUMB_BRANCH_OFFS ) << 1, 12 );
			stream << "B";
			WritePadding(stream, start_position);
			util::stream_format( stream, "0x%08X", uint32_t(pc + 4 + offs));
		}
		break;
	case 0xf: /* BL */
		if( opcode & THUMB_BLOP_LO )
		{
			util::stream_format( stream, "BL (LO) 0x%X", ( opcode & THUMB_BLOP_OFFS ) << 1 );
			dasmflags = STEP_OVER;
		}
		else
		{
			addr = util::sext( ( opcode & THUMB_BLOP_OFFS ) << 12, 23 );
			util::stream_format( stream, "BL (HI) 0x%X", addr );
			dasmflags = STEP_OVER;
		}
		break;
	default:
		util::stream_format(stream, "INVALID %04x", opcode);
		break;
	}

	return 2 | dasmflags | SUPPORTED;
}

arm7_disassembler::arm7_disassembler(config *conf) : m_config(conf)
{
}

u32 arm7_disassembler::opcode_alignment() const
{
	return m_config->get_t_flag() ? 2 : 4;
}

offs_t arm7_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	if(m_config->get_t_flag())
		return thumb_disasm(stream, pc, opcodes.r16(pc));
	else
		return arm7_disasm(stream, pc, opcodes.r32(pc));
}
