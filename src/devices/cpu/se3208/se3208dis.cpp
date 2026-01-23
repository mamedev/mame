// license:BSD-3-Clause
// copyright-holders:ElSemi
#include "emu.h"
#include "se3208dis.h"

#define FLAG_E      0x0800

#define CLRFLAG(f)  SR &= ~(f);
#define SETFLAG(f)  SR |= (f);
#define TESTFLAG(f) (SR & (f))

#define EXTRACT(val, sbit, ebit)  (((val) >> sbit) & ((1 << (((ebit) - (sbit)) + 1)) - 1))
#define SEXT8(val)   (((val) & 0x80) ? ((val) | 0xffffff00) : ((val) & 0xff))
#define SEXT16(val)  (((val) & 0x8000) ? ((val) | 0xffff0000) : ((val) & 0xffff))
#define ZEXT8(val)   ((val) & 0xff)
#define ZEX1T6(val)  ((val) & 0xffff)
#define SEXT(bits,val)   ((val) & (1 << ((bits) - 1)) ? ((val) | (~((1 << (bits)) - 1))) : (val & ((1 << (bits)) - 1)))

#define INST(a) u32 se3208_disassembler::a(u16 opcode, std::ostream &stream)



INST(INVALIDOP)
{
	util::stream_format(stream, "INVALID");
	return 0;
}

INST(LDB)
{
	u32 offset = EXTRACT(opcode, 0, 4);
	const u32 index = EXTRACT(opcode, 5, 7);
	const u32 src_dst = EXTRACT(opcode, 8, 10);

	if (TESTFLAG(FLAG_E))
		offset = (EXTRACT(ER, 0, 27) << 4) | (offset & 0xf);

	if (index)
		util::stream_format(stream, "LDB   (%%R%d,0x%x),%%R%d", index, offset, src_dst);
	else
		util::stream_format(stream, "LDB   (0x%x),%%R%d", index + offset, src_dst);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(STB)
{
	u32 offset = EXTRACT(opcode, 0, 4);
	const u32 index = EXTRACT(opcode, 5, 7);
	const u32 src_dst = EXTRACT(opcode, 8, 10);

	if (TESTFLAG(FLAG_E))
		offset = (EXTRACT(ER, 0, 27) << 4) | (offset & 0xf);

	if (index)
		util::stream_format(stream, "STB   %%R%d,(%%R%d,0x%x)", src_dst, index, offset);
	else
		util::stream_format(stream, "STB   %%R%d,(0x%x)", src_dst, index + offset);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(LDS)
{
	u32 offset = EXTRACT(opcode, 0, 4);
	const u32 index = EXTRACT(opcode, 5, 7);
	const u32 src_dst = EXTRACT(opcode, 8, 10);

	offset <<= 1;

	if (TESTFLAG(FLAG_E))
		offset = (EXTRACT(ER, 0, 27) << 4) | (offset & 0xf);

	if (index)
		util::stream_format(stream, "LDS   (%%R%d,0x%x),%%R%d", index, offset, src_dst);
	else
		util::stream_format(stream, "LDS   (0x%x),%%R%d", index + offset, src_dst);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(STS)
{
	u32 offset = EXTRACT(opcode, 0, 4);
	const u32 index = EXTRACT(opcode, 5, 7);
	const u32 src_dst = EXTRACT(opcode, 8, 10);

	offset <<= 1;

	if (TESTFLAG(FLAG_E))
		offset = (EXTRACT(ER, 0, 27) << 4) | (offset & 0xf);

	if (index)
		util::stream_format(stream, "STS   %%R%d,(%%R%d,0x%x)", src_dst, index, offset);
	else
		util::stream_format(stream, "STS   %%R%d,(0x%x)", src_dst, index + offset);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(LD)
{
	u32 offset = EXTRACT(opcode, 0, 4);
	const u32 index = EXTRACT(opcode, 5, 7);
	const u32 src_dst = EXTRACT(opcode, 8, 10);

	offset <<= 2;

	if (TESTFLAG(FLAG_E))
		offset = (EXTRACT(ER, 0, 27) << 4) | (offset & 0xf);

	if (index)
		util::stream_format(stream, "LD    (%%R%d,0x%x),%%R%d", index, offset, src_dst);
	else
		util::stream_format(stream, "LD    (0x%x),%%R%d", index + offset, src_dst);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(ST)
{
	u32 offset = EXTRACT(opcode, 0, 4);
	const u32 index = EXTRACT(opcode, 5, 7);
	const u32 src_dst = EXTRACT(opcode, 8, 10);

	offset <<= 2;

	if (TESTFLAG(FLAG_E))
		offset = (EXTRACT(ER, 0, 27) << 4) | (offset & 0xf);

	if (index)
		util::stream_format(stream, "ST    %%R%d,(%%R%d,0x%x)", src_dst, index, offset);
	else
		util::stream_format(stream, "ST    %%R%d,(0x%x)", src_dst, index + offset);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(LDBU)
{
	u32 offset = EXTRACT(opcode, 0, 4);
	const u32 index = EXTRACT(opcode, 5, 7);
	const u32 src_dst = EXTRACT(opcode, 8, 10);

	if (TESTFLAG(FLAG_E))
		offset = (EXTRACT(ER, 0, 27) << 4) | (offset & 0xf);

	if (index)
		util::stream_format(stream, "LDBU  (%%R%d,0x%x),%%R%d", index, offset, src_dst);
	else
		util::stream_format(stream, "LDBU  (0x%x),%%R%d", index + offset, src_dst);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(LDSU)
{
	u32 offset = EXTRACT(opcode, 0, 4);
	const u32 index = EXTRACT(opcode, 5, 7);
	const u32 src_dst = EXTRACT(opcode, 8, 10);

	offset <<= 1;

	if (TESTFLAG(FLAG_E))
		offset = (EXTRACT(ER, 0, 27) << 4) | (offset & 0xf);

	if (index)
		util::stream_format(stream, "LDSU  (%%R%d,0x%x),%%R%d", index, offset, src_dst);
	else
		util::stream_format(stream, "LDSU  (0x%x),%%R%d", index + offset, src_dst);

	CLRFLAG(FLAG_E);
	return 0;
}


INST(LERI)
{
	const u32 imm = EXTRACT(opcode, 0, 13);

	if (TESTFLAG(FLAG_E))
		ER = (EXTRACT(ER, 0, 17) << 14) | imm;
	else
		ER = SEXT(14, imm);

	//util::stream_format(stream, "LERI  0x%x\t\tER=%08X", imm,ER);
	util::stream_format(stream, "LERI  0x%x", imm/*, ER*/);

	SETFLAG(FLAG_E);
	return 0;
}

INST(LDSP)
{
	u32 offset = EXTRACT(opcode, 0, 7);
	const u32 src_dst = EXTRACT(opcode, 8, 10);

	offset <<= 2;

	if (TESTFLAG(FLAG_E))
		offset = (EXTRACT(ER, 0, 27) << 4) | (offset & 0xf);

	util::stream_format(stream, "LD    (%%SP,0x%x),%%R%d", offset, src_dst);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(STSP)
{
	u32 offset = EXTRACT(opcode, 0, 7);
	const u32 src_dst = EXTRACT(opcode, 8, 10);

	offset <<= 2;

	if (TESTFLAG(FLAG_E))
		offset = (EXTRACT(ER, 0, 27) << 4) | (offset & 0xf);

	util::stream_format(stream, "ST    %%R%d,(%%SP,0x%x)", src_dst, offset);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(PUSH)
{
	u32 set = EXTRACT(opcode, 0, 10);
	stream << "PUSH  ";
	if (set & (1 << 10))
		stream << "%PC-";
	if (set & (1 << 9))
		stream << "%SR-";
	if (set & (1 << 8))
		stream << "%ER-";
	if (set & (1 << 7))
		stream << "%R7-";
	if (set & (1 << 6))
		stream << "%R6-";
	if (set & (1 << 5))
		stream << "%R5-";
	if (set & (1 << 4))
		stream << "%R4-";
	if (set & (1 << 3))
		stream << "%R3-";
	if (set & (1 << 2))
		stream << "%R2-";
	if (set & (1 << 1))
		stream << "%R1-";
	if (set & (1 << 0))
		stream << "%R0-";
	return 0;
}

INST(POP)
{
	u32 set = EXTRACT(opcode, 0, 10);
	int ret = 0;
	stream << "POP   ";
	if (set & (1 << 0))
		stream << "%R0-";
	if (set & (1 << 1))
		stream << "%R1-";
	if (set & (1 << 2))
		stream << "%R2-";
	if (set & (1 << 3))
		stream << "%R3-";
	if (set & (1 << 4))
		stream << "%R4-";
	if (set & (1 << 5))
		stream << "%R5-";
	if (set & (1 << 6))
		stream << "%R6-";
	if (set & (1 << 7))
		stream << "%R7-";
	if (set & (1 << 8))
		stream << "%ER-";
	if (set & (1 << 9))
		stream << "%SR-";
	if (set & (1 << 10))
	{
		stream << "%PC-";
		CLRFLAG(FLAG_E);    //Clear the flag, this is a ret so disassemble will start a new E block
		ret = 1;
	}
	return ret ? STEP_OUT : 0;
}

INST(LEATOSP)
{
	u32 offset = EXTRACT(opcode, 9, 12);
	const u32 index = EXTRACT(opcode, 3, 5);

	if (TESTFLAG(FLAG_E))
		offset = (EXTRACT(ER, 0, 27) << 4) | (offset & 0xf);
	else
		offset = SEXT(4, offset);

	if (index)
		util::stream_format(stream, "LEA   (%%R%d,0x%x),%%SP", index, offset);
	else
		util::stream_format(stream, "LEA   (0x%x),%%SP", index + offset);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(LEAFROMSP)
{
	u32 offset = EXTRACT(opcode, 9, 12);
	const u32 index = EXTRACT(opcode, 3, 5);

	if (TESTFLAG(FLAG_E))
		offset = (EXTRACT(ER, 0, 27) << 4) | (offset & 0xf);
	else
		offset = SEXT(4, offset);

	util::stream_format(stream, "LEA   (%%SP,0x%x),%%R%d", offset, index);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(LEASPTOSP)
{
	u32 offset = EXTRACT(opcode, 0, 7);

	offset <<= 2;

	if (TESTFLAG(FLAG_E))
		offset = (EXTRACT(ER, 0,23) << 8) | (offset&0xff);
	else
		offset = SEXT(10, offset);


	util::stream_format(stream, "LEA   (%%SP,0x%x),%%SP", offset);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(MOV)
{
	const u32 src = EXTRACT(opcode, 3, 5);
	const u32 dst = EXTRACT(opcode, 9, 11);

	if (src == 0 && dst == 0)
		util::stream_format(stream, "NOP");
	else
		util::stream_format(stream, "MOV   %%SR%d,%%DR%d", src, dst);
	return 0;
}

INST(LDI)
{
	const u32 dst = EXTRACT(opcode, 8, 10);
	u32 imm = EXTRACT(opcode, 0, 7);

	if (TESTFLAG(FLAG_E))
		imm = (EXTRACT(ER, 0, 27) << 4) | (imm & 0xf);
	else
		imm = SEXT8(imm);

	util::stream_format(stream, "LDI   0x%x,%%R%d", imm, dst);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(LDBSP)
{
	u32 offset = EXTRACT(opcode, 0, 3);
	const u32 src_dst = EXTRACT(opcode, 4, 6);

	if (TESTFLAG(FLAG_E))
		offset = (EXTRACT(ER, 0, 27) << 4) | (offset & 0xf);

	util::stream_format(stream, "LDB   (%%SP,0x%x),%%R%d", offset, src_dst);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(STBSP)
{
	u32 offset = EXTRACT(opcode, 0, 3);
	const u32 src_dst = EXTRACT(opcode, 4, 6);

	if (TESTFLAG(FLAG_E))
		offset = (EXTRACT(ER, 0, 27) << 4) | (offset & 0xf);

	util::stream_format(stream, "STB   %%R%d,(%%SP,0x%x)", src_dst, offset);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(LDSSP)
{
	u32 offset = EXTRACT(opcode, 0, 3);
	const u32 src_dst = EXTRACT(opcode, 4, 6);

	offset <<= 1;

	if (TESTFLAG(FLAG_E))
		offset = (EXTRACT(ER, 0, 27) << 4) | (offset & 0xf);

	util::stream_format(stream, "LDS   (%%SP,0x%x),%%R%d", offset, src_dst);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(STSSP)
{
	u32 offset = EXTRACT(opcode, 0, 3);
	const u32 src_dst = EXTRACT(opcode, 4, 6);

	offset <<= 1;

	if (TESTFLAG(FLAG_E))
		offset = (EXTRACT(ER, 0, 27) << 4) | (offset & 0xf);

	util::stream_format(stream, "STS   %%R%d,(%%SP,0x%x)", src_dst, offset);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(LDBUSP)
{
	u32 offset = EXTRACT(opcode, 0, 3);
	const u32 src_dst = EXTRACT(opcode, 4, 6);

	if (TESTFLAG(FLAG_E))
		offset = (EXTRACT(ER, 0, 27) << 4) | (offset & 0xf);

	util::stream_format(stream, "LDBU  (%%SP,0x%x),%%R%d", offset, src_dst);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(LDSUSP)
{
	u32 offset = EXTRACT(opcode, 0, 3);
	const u32 src_dst = EXTRACT(opcode, 4, 6);

	offset <<= 1;

	if (TESTFLAG(FLAG_E))
		offset = (EXTRACT(ER, 0, 27) << 4) | (offset & 0xf);

	util::stream_format(stream, "LDSU  (%%SP,0x%x),%%R%d", offset, src_dst);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(ADDI)
{
	const u32 imm = EXTRACT(opcode, 9, 12);
	const u32 src = EXTRACT(opcode, 3, 5);
	const u32 dst = EXTRACT(opcode, 0, 2);
	u32 imm2 = imm;

	if (TESTFLAG(FLAG_E))
		imm2 = (EXTRACT(ER, 0, 27) << 4) | (imm2 & 0xf);
	else
		imm2 = SEXT(4, imm2);

	util::stream_format(stream, "ADD   %%SR%d,0x%x,%%DR%d", src, imm2, dst/*, imm2*/);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(SUBI)
{
	const u32 imm = EXTRACT(opcode, 9, 12);
	const u32 src = EXTRACT(opcode, 3, 5);
	const u32 dst = EXTRACT(opcode, 0, 2);
	u32 imm2 = imm;

	if (TESTFLAG(FLAG_E))
		imm2 = (EXTRACT(ER, 0, 27) << 4) | (imm2 & 0xf);
	else
		imm2 = SEXT(4, imm2);

	util::stream_format(stream, "SUB   %%SR%d,0x%x,%%DR%d", src, imm2, dst/*, imm2*/);

	CLRFLAG(FLAG_E);

	return 0;
}

INST(ADCI)
{
	const u32 imm = EXTRACT(opcode, 9, 12);
	const u32 src = EXTRACT(opcode, 3, 5);
	const u32 dst = EXTRACT(opcode, 0, 2);
	u32 imm2 = imm;

	if (TESTFLAG(FLAG_E))
		imm2 = (EXTRACT(ER, 0, 27) << 4) | (imm2 & 0xf);
	else
		imm2 = SEXT(4, imm2);

	util::stream_format(stream, "ADC   %%SR%d,0x%x,%%DR%d", src, imm2, dst/*, imm2*/);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(SBCI)
{
	const u32 imm = EXTRACT(opcode, 9, 12);
	const u32 src = EXTRACT(opcode, 3, 5);
	const u32 dst = EXTRACT(opcode, 0, 2);
	u32 imm2 = imm;

	if (TESTFLAG(FLAG_E))
		imm2 = (EXTRACT(ER, 0, 27) << 4) | (imm2 & 0xf);
	else
		imm2 = SEXT(4, imm2);

	util::stream_format(stream, "SBC   %%SR%d,0x%x,%%DR%d", src, imm2, dst/*, imm2*/);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(ANDI)
{
	const u32 imm = EXTRACT(opcode, 9, 12);
	const u32 src = EXTRACT(opcode, 3, 5);
	const u32 dst = EXTRACT(opcode, 0, 2);
	u32 imm2 = imm;

	if (TESTFLAG(FLAG_E))
		imm2 = (EXTRACT(ER, 0, 27) << 4) | (imm2 & 0xf);
	else
		imm2 = SEXT(4, imm2);

	util::stream_format(stream, "AND   %%SR%d,0x%x,%%DR%d", src, imm2, dst/*, imm2*/);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(ORI)
{
	const u32 imm = EXTRACT(opcode, 9, 12);
	const u32 src = EXTRACT(opcode, 3, 5);
	const u32 dst = EXTRACT(opcode, 0, 2);
	u32 imm2 = imm;

	if (TESTFLAG(FLAG_E))
		imm2 = (EXTRACT(ER, 0, 27) << 4) | (imm2 & 0xf);
	else
		imm2 = SEXT(4, imm2);

	util::stream_format(stream, "OR    %%SR%d,0x%x,%%DR%d", src, imm2, dst/*, imm2*/);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(XORI)
{
	const u32 imm = EXTRACT(opcode, 9, 12);
	const u32 src = EXTRACT(opcode, 3, 5);
	const u32 dst = EXTRACT(opcode, 0, 2);
	u32 imm2 = imm;

	if (TESTFLAG(FLAG_E))
		imm2 = (EXTRACT(ER, 0, 27) << 4) | (imm2 & 0xf);
	else
		imm2 = SEXT(4, imm2);

	util::stream_format(stream, "XOR   %%SR%d,0x%x,%%DR%d", src, imm2, dst/*, imm2*/);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(CMPI)
{
	const u32 imm = EXTRACT(opcode, 9, 12);
	const u32 src = EXTRACT(opcode, 3, 5);
	u32 imm2 = imm;

	if (TESTFLAG(FLAG_E))
		imm2 = (EXTRACT(ER, 0, 27) << 4) | (imm2 & 0xf);
	else
		imm2 = SEXT(4, imm2);

	util::stream_format(stream, "CMP   %%SR%d,0x%x", src, imm2/*, imm2*/);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(TSTI)
{
	const u32 imm = EXTRACT(opcode, 9, 12);
	const u32 src = EXTRACT(opcode, 3, 5);
	u32 imm2 = imm;

	if (TESTFLAG(FLAG_E))
		imm2 = (EXTRACT(ER, 0, 27) << 4) | (imm2 & 0xf);
	else
		imm2 = SEXT(4, imm2);

	util::stream_format(stream, "TST   %%SR%d,0x%x", src, imm2/*, imm2*/);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(ADD)
{
	const u32 src2 = EXTRACT(opcode, 9, 11);
	const u32 src1 = EXTRACT(opcode, 3, 5);
	const u32 dst = EXTRACT(opcode, 0, 2);

	util::stream_format(stream, "ADD   %%SR%d,%%SR%d,%%DR%d", src1, src2, dst);
	return 0;
}

INST(SUB)
{
	const u32 src2 = EXTRACT(opcode, 9, 11);
	const u32 src1 = EXTRACT(opcode, 3, 5);
	const u32 dst = EXTRACT(opcode, 0, 2);

	util::stream_format(stream, "SUB   %%SR%d,%%SR%d,%%DR%d", src1, src2, dst);
	return 0;
}

INST(ADC)
{
	const u32 src2 = EXTRACT(opcode, 9, 11);
	const u32 src1 = EXTRACT(opcode, 3, 5);
	const u32 dst = EXTRACT(opcode, 0, 2);

	util::stream_format(stream, "ADC   %%SR%d,%%SR%d,%%DR%d", src1, src2, dst);
	return 0;
}

INST(SBC)
{
	const u32 src2 = EXTRACT(opcode, 9, 11);
	const u32 src1 = EXTRACT(opcode, 3, 5);
	const u32 dst = EXTRACT(opcode, 0, 2);

	util::stream_format(stream, "SBC   %%SR%d,%%SR%d,%%DR%d", src1, src2, dst);
	return 0;
}

INST(AND)
{
	const u32 src2 = EXTRACT(opcode, 9, 11);
	const u32 src1 = EXTRACT(opcode, 3, 5);
	const u32 dst = EXTRACT(opcode, 0, 2);

	util::stream_format(stream, "AND   %%SR%d,%%SR%d,%%DR%d", src1, src2, dst);
	return 0;
}

INST(OR)
{
	const u32 src2 = EXTRACT(opcode, 9, 11);
	const u32 src1 = EXTRACT(opcode, 3, 5);
	const u32 dst = EXTRACT(opcode, 0, 2);

	util::stream_format(stream, "OR    %%SR%d,%%SR%d,%%DR%d", src1, src2, dst);
	return 0;
}

INST(XOR)
{
	const u32 src2 = EXTRACT(opcode, 9, 11);
	const u32 src1 = EXTRACT(opcode, 3, 5);
	const u32 dst = EXTRACT(opcode, 0, 2);

	util::stream_format(stream, "XOR   %%SR%d,%%SR%d,%%DR%d", src1, src2, dst);
	return 0;
}

INST(CMP)
{
	const u32 src2 = EXTRACT(opcode, 9, 11);
	const u32 src1 = EXTRACT(opcode, 3, 5);

	util::stream_format(stream, "CMP   %%SR%d,%%SR%d", src1, src2);
	return 0;
}

INST(TST)
{
	const u32 src2 = EXTRACT(opcode, 9, 11);
	const u32 src1 = EXTRACT(opcode, 3, 5);

	util::stream_format(stream, "TST   %%SR%d,%%SR%d", src1, src2);
	return 0;
}

INST(MULS)
{
	const u32 src2 = EXTRACT(opcode, 6, 8);
	const u32 src1 = EXTRACT(opcode, 3, 5);
	const u32 dst = EXTRACT(opcode, 0, 2);

	util::stream_format(stream, "MUL   %%SR%d,%%SR%d,%%DR%d", src1, src2, dst);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(NEG)
{
	const u32 dst = EXTRACT(opcode, 9, 11);
	const u32 src = EXTRACT(opcode, 3, 5);

	util::stream_format(stream, "NEG   %%SR%d,%%DR%d", src, dst);
	return 0;
}

INST(CALL)
{
	const u32 offset = EXTRACT(opcode, 0, 7);
	u32 offset2;

	if (TESTFLAG(FLAG_E))
		offset2 = (EXTRACT(ER, 0, 22) << 8) | offset;
	else
		offset2 = SEXT(8, offset);
	offset2 <<= 1;
	util::stream_format(stream, "CALL  0x%x", PC + 2 + offset2);

	CLRFLAG(FLAG_E);
	return STEP_OVER;
}

INST(JV)
{
	const u32 offset = EXTRACT(opcode, 0, 7);
	u32 offset2;

	if (TESTFLAG(FLAG_E))
		offset2 = (EXTRACT(ER, 0, 22) << 8) | offset;
	else
		offset2 = SEXT(8, offset);
	offset2 <<= 1;
	util::stream_format(stream, "JV    0x%x", PC + 2 + offset2);

	CLRFLAG(FLAG_E);
	return STEP_COND;
}

INST(JNV)
{
	const u32 offset = EXTRACT(opcode, 0, 7);
	u32 offset2;

	if (TESTFLAG(FLAG_E))
		offset2 = (EXTRACT(ER, 0, 22) << 8) | offset;
	else
		offset2 = SEXT(8, offset);
	offset2 <<= 1;
	util::stream_format(stream, "JNV   0x%x", PC + 2 + offset2);

	CLRFLAG(FLAG_E);
	return STEP_COND;
}

INST(JC)
{
	const u32 offset = EXTRACT(opcode, 0, 7);
	u32 offset2;

	if (TESTFLAG(FLAG_E))
		offset2 = (EXTRACT(ER, 0, 22) << 8) | offset;
	else
		offset2 = SEXT(8, offset);
	offset2 <<= 1;
	util::stream_format(stream, "JC    0x%x", PC + 2 + offset2);

	CLRFLAG(FLAG_E);
	return STEP_COND;
}

INST(JNC)
{
	const u32 offset = EXTRACT(opcode, 0, 7);
	u32 offset2;

	if (TESTFLAG(FLAG_E))
		offset2 = (EXTRACT(ER, 0, 22) << 8) | offset;
	else
		offset2 = SEXT(8, offset);
	offset2 <<= 1;
	util::stream_format(stream, "JNC   0x%x", PC + 2 + offset2);

	CLRFLAG(FLAG_E);
	return STEP_COND;
}

INST(JP)
{
	const u32 offset = EXTRACT(opcode, 0, 7);
	u32 offset2;

	if (TESTFLAG(FLAG_E))
		offset2 = (EXTRACT(ER, 0, 22) << 8) | offset;
	else
		offset2 = SEXT(8, offset);
	offset2 <<= 1;
	util::stream_format(stream, "JP    0x%x", PC + 2 + offset2);

	CLRFLAG(FLAG_E);
	return STEP_COND;
}

INST(JM)
{
	const u32 offset = EXTRACT(opcode, 0, 7);
	u32 offset2;

	if (TESTFLAG(FLAG_E))
		offset2 = (EXTRACT(ER, 0, 22) << 8) | offset;
	else
		offset2 = SEXT(8, offset);
	offset2 <<= 1;
	util::stream_format(stream, "JM    0x%x", PC + 2 + offset2);

	CLRFLAG(FLAG_E);
	return STEP_COND;
}

INST(JNZ)
{
	const u32 offset = EXTRACT(opcode, 0, 7);
	u32 offset2;

	if (TESTFLAG(FLAG_E))
		offset2 = (EXTRACT(ER, 0, 22) << 8) | offset;
	else
		offset2 = SEXT(8, offset);
	offset2 <<= 1;
	util::stream_format(stream, "JNZ   0x%x", PC + 2 + offset2);

	CLRFLAG(FLAG_E);
	return STEP_COND;
}

INST(JZ)
{
	const u32 offset = EXTRACT(opcode, 0, 7);
	u32 offset2;

	if (TESTFLAG(FLAG_E))
		offset2 = (EXTRACT(ER, 0, 22) << 8) | offset;
	else
		offset2 = SEXT(8, offset);
	offset2 <<= 1;
	util::stream_format(stream, "JZ    0x%x", PC + 2 + offset2);

	CLRFLAG(FLAG_E);
	return STEP_COND;
}

INST(JGE)
{
	const u32 offset = EXTRACT(opcode, 0, 7);
	u32 offset2;

	if (TESTFLAG(FLAG_E))
		offset2 = (EXTRACT(ER, 0, 22) << 8) | offset;
	else
		offset2 = SEXT(8, offset);
	offset2 <<= 1;
	util::stream_format(stream, "JGE   0x%x", PC + 2 + offset2);

	CLRFLAG(FLAG_E);
	return STEP_COND;
}

INST(JLE)
{
	const u32 offset = EXTRACT(opcode, 0, 7);
	u32 offset2;

	if (TESTFLAG(FLAG_E))
		offset2 = (EXTRACT(ER, 0, 22) << 8) | offset;
	else
		offset2 = SEXT(8, offset);
	offset2 <<= 1;
	util::stream_format(stream, "JLE   0x%x", PC + 2 + offset2);

	CLRFLAG(FLAG_E);
	return STEP_COND;
}

INST(JHI)
{
	const u32 offset = EXTRACT(opcode, 0, 7);
	u32 offset2;

	if (TESTFLAG(FLAG_E))
		offset2 = (EXTRACT(ER, 0, 22) << 8) | offset;
	else
		offset2 = SEXT(8, offset);
	offset2 <<= 1;
	util::stream_format(stream, "JHI   0x%x", PC + 2 + offset2);

	CLRFLAG(FLAG_E);
	return STEP_COND;
}

INST(JLS)
{
	const u32 offset = EXTRACT(opcode, 0, 7);
	u32 offset2;

	if (TESTFLAG(FLAG_E))
		offset2 = (EXTRACT(ER, 0, 22) << 8) | offset;
	else
		offset2 = SEXT(8, offset);
	offset2 <<= 1;
	util::stream_format(stream, "JLS   0x%x", PC + 2 + offset2);

	CLRFLAG(FLAG_E);
	return STEP_COND;
}

INST(JGT)
{
	const u32 offset = EXTRACT(opcode, 0, 7);
	u32 offset2;

	if (TESTFLAG(FLAG_E))
		offset2 = (EXTRACT(ER, 0, 22) << 8) | offset;
	else
		offset2 = SEXT(8, offset);
	offset2 <<= 1;
	util::stream_format(stream, "JGT   0x%x", PC + 2 + offset2);

	CLRFLAG(FLAG_E);
	return STEP_COND;
}

INST(JLT)
{
	const u32 offset = EXTRACT(opcode, 0, 7);
	u32 offset2;

	if (TESTFLAG(FLAG_E))
		offset2 = (EXTRACT(ER, 0, 22) << 8) | offset;
	else
		offset2 = SEXT(8, offset);
	offset2 <<= 1;
	util::stream_format(stream, "JLT   0x%x", PC + 2 + offset2);

	CLRFLAG(FLAG_E);
	return STEP_COND;
}

INST(JMP)
{
	const u32 offset = EXTRACT(opcode, 0, 7);
	u32 offset2;

	if (TESTFLAG(FLAG_E))
		offset2 = (EXTRACT(ER, 0, 22) << 8) | offset;
	else
		offset2 = SEXT(8, offset);
	offset2 <<= 1;
	util::stream_format(stream, "JMP   0x%x", PC + 2 + offset2);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(JR)
{
	const u32 src = EXTRACT(opcode, 0, 3);

	util::stream_format(stream, "JR    %%R%d", src);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(CALLR)
{
	const u32 src = EXTRACT(opcode, 0, 3);

	util::stream_format(stream, "CALLR %%R%d", src);

	CLRFLAG(FLAG_E);
	return STEP_OVER;
}

INST(ASR)
{
	const u32 cs = opcode & (1 << 10);
	const u32 dst = EXTRACT(opcode, 0, 2);
	const u32 imm = EXTRACT(opcode, 5, 9);
	const u32 cnt = EXTRACT(opcode, 5, 7);

	if (cs)
		util::stream_format(stream, "ASR   %%R%d,%%R%d", cnt, dst);
	else
		util::stream_format(stream, "ASR   %x,%%R%d", imm, dst);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(LSR)
{
	const u32 cs = opcode & (1 << 10);
	const u32 dst = EXTRACT(opcode, 0, 2);
	const u32 imm = EXTRACT(opcode, 5, 9);
	const u32 cnt = EXTRACT(opcode, 5, 7);

	if (cs)
		util::stream_format(stream, "LSR   %%R%d,%%R%d", cnt, dst);
	else
		util::stream_format(stream, "LSR   %x,%%R%d", imm, dst);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(ASL)
{
	const u32 cs = opcode & (1 << 10);
	const u32 dst = EXTRACT(opcode, 0, 2);
	const u32 imm = EXTRACT(opcode, 5, 9);
	const u32 cnt = EXTRACT(opcode, 5, 7);

	if (cs)
		util::stream_format(stream, "ASL   %%R%d,%%R%d", cnt, dst);
	else
		util::stream_format(stream, "ASL   %x,%%R%d", imm, dst);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(EXTB)
{
	const u32 dst = EXTRACT(opcode, 0, 3);

	util::stream_format(stream, "EXTB  %%R%d", dst);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(EXTS)
{
	const u32 dst = EXTRACT(opcode, 0, 3);

	util::stream_format(stream, "EXTS  %%R%d", dst);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(SET)
{
	const u32 imm = EXTRACT(opcode, 0, 3);

	util::stream_format(stream, "SET   0x%x", imm);
	return 0;
}

INST(CLR)
{
	const u32 imm = EXTRACT(opcode, 0, 3);

	util::stream_format(stream, "CLR   0x%x", imm);
	return 0;
}

INST(SWI)
{
	const u32 imm = EXTRACT(opcode, 0, 3);

	util::stream_format(stream, "SWI   0x%x", imm);
	return 0;
}

INST(HALT)
{
	const u32 imm = EXTRACT(opcode, 0, 3);

	util::stream_format(stream, "HALT  0x%x", imm);
	return 0;
}

INST(MVTC)
{
	const u32 imm = EXTRACT(opcode, 0, 3);

	util::stream_format(stream, "MVTC  %%R0,%%CR%d", imm);
	return 0;
}

INST(MVFC)
{
	const u32 imm = EXTRACT(opcode, 0, 3);

	util::stream_format(stream, "MVFC  %%CR0%d,%%R0", imm);
	return 0;
}

se3208_disassembler::OP se3208_disassembler::decode_op(u16 opcode)
{
	switch (EXTRACT(opcode, 14, 15))
	{
		case 0x0:
			{
				const u8 op = EXTRACT(opcode, 11, 13);
				switch (op)
				{
					case 0x0:
						return &se3208_disassembler::LDB;
					case 0x1:
						return &se3208_disassembler::LDS;
					case 0x2:
						return &se3208_disassembler::LD;
					case 0x3:
						return &se3208_disassembler::LDBU;
					case 0x4:
						return &se3208_disassembler::STB;
					case 0x5:
						return &se3208_disassembler::STS;
					case 0x6:
						return &se3208_disassembler::ST;
					case 0x7:
						return &se3208_disassembler::LDSU;
				}
			}
			break;
		case 0x1:
			return &se3208_disassembler::LERI;
		case 0x2:
			{
				switch (EXTRACT(opcode, 11, 13))
				{
					case 0:
						return &se3208_disassembler::LDSP;
					case 1:
						return &se3208_disassembler::STSP;
					case 2:
						return &se3208_disassembler::PUSH;
					case 3:
						return &se3208_disassembler::POP;
					case 4:
					case 5:
					case 6:
					case 7:
					case 8: //arith
					case 9:
					case 10:
					case 11:
					case 12:
					case 13:
					case 14:
					case 15:
						switch (EXTRACT(opcode, 6, 8))
						{
							case 0:
								return &se3208_disassembler::ADDI;
							case 1:
								return &se3208_disassembler::ADCI;
							case 2:
								return &se3208_disassembler::SUBI;
							case 3:
								return &se3208_disassembler::SBCI;
							case 4:
								return &se3208_disassembler::ANDI;
							case 5:
								return &se3208_disassembler::ORI;
							case 6:
								return &se3208_disassembler::XORI;
							case 7:
								switch (EXTRACT(opcode, 0, 2))
								{
									case 0:
										return &se3208_disassembler::CMPI;
									case 1:
										return &se3208_disassembler::TSTI;
									case 2:
										return &se3208_disassembler::LEATOSP;
									case 3:
										return &se3208_disassembler::LEAFROMSP;
								}
								break;
						}
						break;
				}
			}
			break;
		case 3:
			switch (EXTRACT(opcode, 12, 13))
			{
				case 0:
					switch (EXTRACT(opcode, 6, 8))
					{
						case 0:
							return &se3208_disassembler::ADD;
						case 1:
							return &se3208_disassembler::ADC;
						case 2:
							return &se3208_disassembler::SUB;
						case 3:
							return &se3208_disassembler::SBC;
						case 4:
							return &se3208_disassembler::AND;
						case 5:
							return &se3208_disassembler::OR;
						case 6:
							return &se3208_disassembler::XOR;
						case 7:
							switch (EXTRACT(opcode, 0, 2))
							{
								case 0:
									return &se3208_disassembler::CMP;
								case 1:
									return &se3208_disassembler::TST;
								case 2:
									return &se3208_disassembler::MOV;
								case 3:
									return &se3208_disassembler::NEG;
							}
							break;
					}
					break;
				case 1:     //Jumps
					switch (EXTRACT(opcode, 8, 11))
					{
						case 0x0:
							return &se3208_disassembler::JNV;
						case 0x1:
							return &se3208_disassembler::JV;
						case 0x2:
							return &se3208_disassembler::JP;
						case 0x3:
							return &se3208_disassembler::JM;
						case 0x4:
							return &se3208_disassembler::JNZ;
						case 0x5:
							return &se3208_disassembler::JZ;
						case 0x6:
							return &se3208_disassembler::JNC;
						case 0x7:
							return &se3208_disassembler::JC;
						case 0x8:
							return &se3208_disassembler::JGT;
						case 0x9:
							return &se3208_disassembler::JLT;
						case 0xa:
							return &se3208_disassembler::JGE;
						case 0xb:
							return &se3208_disassembler::JLE;
						case 0xc:
							return &se3208_disassembler::JHI;
						case 0xd:
							return &se3208_disassembler::JLS;
						case 0xe:
							return &se3208_disassembler::JMP;
						case 0xf:
							return &se3208_disassembler::CALL;
					}
					break;
				case 2:
					if (opcode & (1 << 11))
						return &se3208_disassembler::LDI;
					else    //SP Ops
					{
						if (opcode & (1 << 10))
						{
							switch (EXTRACT(opcode, 7, 9))
							{
								case 0:
									return &se3208_disassembler::LDBSP;
								case 1:
									return &se3208_disassembler::LDSSP;
								case 3:
									return &se3208_disassembler::LDBUSP;
								case 4:
									return &se3208_disassembler::STBSP;
								case 5:
									return &se3208_disassembler::STSSP;
								case 7:
									return &se3208_disassembler::LDSUSP;
							}
						}
						else
						{
							if (opcode & (1 << 9))
							{
								return &se3208_disassembler::LEASPTOSP;
							}
							else
							{
								if (opcode & (1 << 8))
								{
								}
								else
								{
									switch (EXTRACT(opcode, 4, 7))
									{
										case 0:
											return &se3208_disassembler::EXTB;
										case 1:
											return &se3208_disassembler::EXTS;
										case 8:
											return &se3208_disassembler::JR;
										case 9:
											return &se3208_disassembler::CALLR;
										case 10:
											return &se3208_disassembler::SET;
										case 11:
											return &se3208_disassembler::CLR;
										case 12:
											return &se3208_disassembler::SWI;
										case 13:
											return &se3208_disassembler::HALT;
									}
								}
							}
						}
					}
					break;
				case 3:
					switch (EXTRACT(opcode, 9, 11))
					{
						case 0:
						case 1:
						case 2:
						case 3:
							switch (EXTRACT(opcode, 3, 4))
							{
								case 0:
									return &se3208_disassembler::ASR;
								case 1:
									return &se3208_disassembler::LSR;
								case 2:
									return &se3208_disassembler::ASL;
								//case 3:
								//  return &se3208_disassembler::LSL;
							}
							break;
						case 4:
							return &se3208_disassembler::MULS;
						case 6:
							if (opcode & (1 << 3))
								return &se3208_disassembler::MVFC;
							else
								return &se3208_disassembler::MVTC;
					}
					break;
			}
			break;

	}
	return &se3208_disassembler::INVALIDOP;
}


u32 se3208_disassembler::opcode_alignment() const
{
	return 2;
}

offs_t se3208_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	CLRFLAG(FLAG_E);
	ER = 0;

	PC = pc;
	const u16 opcode = opcodes.r16(pc);
	return 2 | ((this->*decode_op(opcode))(opcode, stream)) | SUPPORTED;
}
