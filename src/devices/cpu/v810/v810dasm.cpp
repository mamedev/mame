// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina
/********************************************
    NEC V810 (upd70732) disassembler
  Tomasz Slanina - analog[at]op.pl
*******************************************/

#include "emu.h"
#include "v810dasm.h"

#define I5(x) (((x)&0x1f)|(((x)&0x10)?0xffffffe0:0))
#define UI5(x) ((x)&0x1f)
#define I16(x) (((x)&0xffff)|(((x)&0x8000)?0xffff0000:0))
#define UI16(x) ((x)&0xffff)
#define D16(x) (((x)&0xffff)|(((x)&0x8000)?0xffff0000:0))
#define D26(x,y) ((y)|((x&0x3ff)<<16 )|((x&0x200)?0xfc000000:0))
#define D9(x) ((x&0x1ff)|((x&0x100)?0xfffffe00:0))

const char *const v810_disassembler::dRegs[]=
{
"R0","R1","R2","SP","R4",
"R5","R6","R7","R8","R9",
"R10","R11","R12","R13",
"R14","R15","R16","R17",
"R18","R19","R20","R21",
"R22","R23","R24","R25",
"R26","R27","R28","R29",
"R30","R31",
"EIPC","EIPSW","FEPC","FEPSW","ECR",
"PSW","PIR","TKCW","<Unk>","<Unk>",
"<Unk>","<Unk>","<Unk>","<Unk>","<Unk>",
"<Unk>","<Unk>","<Unk>","<Unk>","<Unk>",
"<Unk>","<Unk>","<Unk>","<Unk>","CHCW","ADTRE",
"<Unk>","<Unk>","<Unk>","<Unk>","<Unk>","<Unk>"
};

#define GET1s(opcode) dRegs[(opcode)&0x1f]
#define GET2s(opcode) dRegs[((opcode)>>5)&0x1f]
#define GETRs(opcode) dRegs[32+((opcode)&0x1f)]

u32 v810_disassembler::opcode_alignment() const
{
	return 2;
}

offs_t v810_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	uint32_t flags = 0;
	uint32_t opc,opc2;
	unsigned size;
	opc = opcodes.r16(pc);
	opc2 = opcodes.r16(pc+2);

	switch(opc>>10)
	{
		case 0x00: util::stream_format(stream,"MOV %s,%s",GET1s(opc),GET2s(opc)); size=2; break;
		case 0x01: util::stream_format(stream,"ADD %s,%s",GET1s(opc),GET2s(opc)); size=2; break;
		case 0x02: util::stream_format(stream,"SUB %s,%s",GET1s(opc),GET2s(opc)); size=2; break;
		case 0x03: util::stream_format(stream,"CMP %s,%s",GET1s(opc),GET2s(opc)); size=2; break;
		case 0x04: util::stream_format(stream,"SHL %s,%s",GET1s(opc),GET2s(opc)); size=2; break;
		case 0x05: util::stream_format(stream,"SHR %s,%s",GET1s(opc),GET2s(opc)); size=2; break;
		case 0x06: util::stream_format(stream,"JMP [%s]",GET1s(opc)); size=2; if ((opc&0x1f) == 31) flags = STEP_OUT; break;
		case 0x07: util::stream_format(stream,"SAR %s,%s",GET1s(opc),GET2s(opc)); size=2; break;
		case 0x08: util::stream_format(stream,"MUL %s,%s",GET1s(opc),GET2s(opc)); size=2; break;
		case 0x09: util::stream_format(stream,"DIV %s,%s",GET1s(opc),GET2s(opc)); size=2; break;
		case 0x0a: util::stream_format(stream,"MULU %s,%s",GET1s(opc),GET2s(opc)); size=2; break;
		case 0x0b: util::stream_format(stream,"DIVU %s,%s",GET1s(opc),GET2s(opc)); size=2; break;
		case 0x0c: util::stream_format(stream,"OR %s,%s",GET1s(opc),GET2s(opc));    size=2; break;
		case 0x0d: util::stream_format(stream,"AND %s,%s",GET1s(opc),GET2s(opc)); size=2; break;
		case 0x0e: util::stream_format(stream,"XOR %s,%s",GET1s(opc),GET2s(opc)); size=2; break;
		case 0x0f: util::stream_format(stream,"NOT %s,%s",GET1s(opc),GET2s(opc)); size=2; break;
		case 0x10: util::stream_format(stream,"MOV %X,%s",I5(opc),GET2s(opc)); size=2; break;
		case 0x11: util::stream_format(stream,"ADD %X,%s",I5(opc),GET2s(opc)); size=2; break;
		case 0x12: util::stream_format(stream,"SETF %X,%s",I5(opc),GET2s(opc)); size=2; break;
		case 0x13: util::stream_format(stream,"CMP %X,%s",I5(opc),GET2s(opc)); size=2; break;
		case 0x14: util::stream_format(stream,"SHL %X,%s",UI5(opc),GET2s(opc)); size=2; break;
		case 0x15: util::stream_format(stream,"SHR %X,%s",UI5(opc),GET2s(opc)); size=2; break;
		case 0x16: util::stream_format(stream,"EI"); size=2; break;
		case 0x17: util::stream_format(stream,"SAR %X,%s",UI5(opc),GET2s(opc)); size=2; break;
		case 0x18: util::stream_format(stream,"TRAP %X",I5(opc)); size=2; break;
		case 0x19: util::stream_format(stream,"RETI"); size=2; flags = STEP_OUT; break;
		case 0x1a: util::stream_format(stream,"HALT"); size=2; break;
		case 0x1b: util::stream_format(stream,"Unk 0x1B"); size=2; break;
		case 0x1c: util::stream_format(stream,"LDSR %s,%s",GET2s(opc),GETRs(opc));size=2; break;
		case 0x1d: util::stream_format(stream,"STSR %s,%s",GETRs(opc),GET2s(opc));size=2; break;
		case 0x1e: util::stream_format(stream,"DI"); size=2; break;
		case 0x1f:
					switch(opc&0x1f)
						{
							case 0x00:  util::stream_format(stream,"SCH0BSU"); break;
							case 0x01:  util::stream_format(stream,"SCH0BSD"); break;
							case 0x02:  util::stream_format(stream,"SCH1BSU"); break;
							case 0x03:  util::stream_format(stream,"SCH1BSD"); break;
							case 0x04:  util::stream_format(stream,"UnkS 4"); break;
							case 0x05:  util::stream_format(stream,"UnkS 5"); break;
							case 0x06:  util::stream_format(stream,"UnkS 6"); break;
							case 0x08:  util::stream_format(stream,"ORBSU"); break;
							case 0x09:  util::stream_format(stream,"ANDBSU"); break;
							case 0x0a:  util::stream_format(stream,"XORBSU"); break;
							case 0x0b:  util::stream_format(stream,"MOVBSU"); break;
							case 0x0c:  util::stream_format(stream,"ORNBSU"); break;
							case 0x0d:  util::stream_format(stream,"ANDNBSU"); break;
							case 0x0e:  util::stream_format(stream,"XORNBSU"); break;
							case 0x0f:  util::stream_format(stream,"NOTBSU"); break;
							default:        util::stream_format(stream,"UnkBS 0x%X",opc&0x1f); break;
						}
					size=2;
					break;
		case 0x20:
		case 0x21:
		case 0x22:
		case 0x23:
		case 0x24:
		case 0x25:
		case 0x26:
		case 0x27: switch( (opc>>9) &0xf)
								{
									case 0x0: util::stream_format(stream,"BV %X",pc+D9(opc));  break;
									case 0x1: util::stream_format(stream,"BL %X",pc+D9(opc));  break;
									case 0x2: util::stream_format(stream,"BE %X",pc+D9(opc));  break;
									case 0x3: util::stream_format(stream,"BNH %X",pc+D9(opc)); break;
									case 0x4: util::stream_format(stream,"BN %X",pc+D9(opc));  break;
									case 0x5: util::stream_format(stream,"BR %X",pc+D9(opc));  break;
									case 0x6: util::stream_format(stream,"BLT %X",pc+D9(opc)); break;
									case 0x7: util::stream_format(stream,"BLE %X",pc+D9(opc)); break;
									case 0x8: util::stream_format(stream,"BNV %X",pc+D9(opc)); break;
									case 0x9: util::stream_format(stream,"BNL %X",pc+D9(opc)); break;
									case 0xa: util::stream_format(stream,"BNE %X",pc+D9(opc)); break;
									case 0xb: util::stream_format(stream,"BH %X",pc+D9(opc));  break;
									case 0xc: util::stream_format(stream,"BP %X",pc+D9(opc));  break;
									case 0xd: util::stream_format(stream,"NOP"); break;
									case 0xe: util::stream_format(stream,"BGE %X",pc+D9(opc)); break;
									case 0xf: util::stream_format(stream,"BGT %X",pc+D9(opc)); break;
								}
								size=2;
								break;

		case 0x28:  util::stream_format(stream,"MOVEA %X, %s, %s",I16(opc2),GET1s(opc),GET2s(opc));size=4; break;
		case 0x29:  util::stream_format(stream,"ADDI %X, %s, %s",I16(opc2),GET1s(opc),GET2s(opc));size=4; break;
		case 0x2a:  util::stream_format(stream,"JR %X",pc+D26(opc,opc2));size=4; break;
		case 0x2b:  util::stream_format(stream,"JAL %X",pc+D26(opc,opc2));size=4; flags = STEP_OVER; break;
		case 0x2c:  util::stream_format(stream,"ORI %X, %s, %s",UI16(opc2),GET1s(opc),GET2s(opc));size=4; break;
		case 0x2d:  util::stream_format(stream,"ANDI %X, %s, %s",UI16(opc2),GET1s(opc),GET2s(opc));size=4; break;
		case 0x2e:  util::stream_format(stream,"XORI %X, %s, %s",UI16(opc2),GET1s(opc),GET2s(opc));size=4; break;
		case 0x2f:  util::stream_format(stream,"MOVHI %X, %s, %s",UI16(opc2),GET1s(opc),GET2s(opc));size=4; break;
		case 0x30:  util::stream_format(stream,"LDB %X[%s], %s",D16(opc2),GET1s(opc),GET2s(opc));size=4; break;
		case 0x31:  util::stream_format(stream,"LDH %X[%s], %s",D16(opc2),GET1s(opc),GET2s(opc));size=4; break;
		case 0x32:  util::stream_format(stream,"Unk 0x32"); size=2; break;
		case 0x33:  util::stream_format(stream,"LDW %X[%s], %s",D16(opc2),GET1s(opc),GET2s(opc));size=4; break;
		case 0x34:  util::stream_format(stream,"STB %s, %X[%s]",GET2s(opc),D16(opc2),GET1s(opc));size=4; break;
		case 0x35:  util::stream_format(stream,"STH %s, %X[%s]",GET2s(opc),D16(opc2),GET1s(opc));size=4; break;
		case 0x36:  util::stream_format(stream,"Unk 0x36"); size=2; break;
		case 0x37:  util::stream_format(stream,"STW %s, %X[%s]",GET2s(opc),D16(opc2),GET1s(opc));size=4; break;
		case 0x38:  util::stream_format(stream,"INB %X[%s], %s",D16(opc2),GET1s(opc),GET2s(opc));size=4; break;
		case 0x39:  util::stream_format(stream,"INH %X[%s], %s",D16(opc2),GET1s(opc),GET2s(opc));size=4; break;
		case 0x3a:  util::stream_format(stream,"CAXI %X[%s], %s",D16(opc2),GET1s(opc),GET2s(opc));size=4; break;
		case 0x3b:  util::stream_format(stream,"INW %X[%s], %s",D16(opc2),GET1s(opc),GET2s(opc));size=4; break;

		case 0x3c:  util::stream_format(stream,"OUTB %s, %X[%s]",GET2s(opc),D16(opc2),GET1s(opc));size=4; break;
		case 0x3d:  util::stream_format(stream,"OUTH %s, %X[%s]",GET2s(opc),D16(opc2),GET1s(opc));size=4; break;
		case 0x3e:
								switch((opc2&0xfc00)>>10)
								{
									case 0x0: util::stream_format(stream,"CMPF.S %s, %s",GET1s(opc),GET2s(opc)); break;
									case 0x2: util::stream_format(stream,"CVT.WS %s, %s",GET1s(opc),GET2s(opc)); break;
									case 0x3: util::stream_format(stream,"CVT.SW %s, %s",GET1s(opc),GET2s(opc)); break;
									case 0x4: util::stream_format(stream,"ADDF.S %s, %s",GET1s(opc),GET2s(opc)); break;
									case 0x5: util::stream_format(stream,"SUBF.S %s, %s",GET1s(opc),GET2s(opc)); break;
									case 0x6: util::stream_format(stream,"MULF.S %s, %s",GET1s(opc),GET2s(opc)); break;
									case 0x7: util::stream_format(stream,"DIVF.S %s, %s",GET1s(opc),GET2s(opc)); break;
									case 0xb: util::stream_format(stream,"TRNC.SW %s, %s",GET1s(opc),GET2s(opc)); break;
									default : util::stream_format(stream,"Unkf 0x%X",(opc2&0xfc00)>>10); break;
								}
							size=4;
							break;
		case 0x3f:  util::stream_format(stream,"OUTW %s, %X[%s]",GET2s(opc),D16(opc2),GET1s(opc));size=4; break;

		default : size=2;
	}
	return size | flags | SUPPORTED;
}
