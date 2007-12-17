/********************************************
    NEC V810 (upd70732) disassembler
  Tomasz Slanina - analog[at]op.pl
*******************************************/

#include <math.h>
#include "debugger.h"
#include "v810.h"

const static char *dRegs[]={
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

offs_t v810_dasm(char *buffer, offs_t oldpc, const UINT8 *oprom, const UINT8 *opram)
{
	UINT32 flags = 0;
	UINT32 opc,opc2;
	UINT32 pc=oldpc;
	unsigned size;
	opc = R_OP(pc);
	opc2 = R_OP(pc+2);

	switch(opc>>10)
	{
		case 0x00: sprintf(buffer,"MOV %s,%s",GET1s(opc),GET2s(opc)); size=2; break;
		case 0x01: sprintf(buffer,"ADD %s,%s",GET1s(opc),GET2s(opc)); size=2; break;
		case 0x02: sprintf(buffer,"SUB %s,%s",GET1s(opc),GET2s(opc)); size=2; break;
		case 0x03: sprintf(buffer,"CMP %s,%s",GET1s(opc),GET2s(opc)); size=2; break;
		case 0x04: sprintf(buffer,"SHL %s,%s",GET1s(opc),GET2s(opc)); size=2; break;
		case 0x05: sprintf(buffer,"SHR %s,%s",GET1s(opc),GET2s(opc)); size=2; break;
		case 0x06: sprintf(buffer,"JMP [%s]",GET1s(opc)); size=2; if ((opc&0x1f) == 31) flags = DASMFLAG_STEP_OUT; break;
		case 0x07: sprintf(buffer,"SAR %s,%s",GET1s(opc),GET2s(opc)); size=2; break;
		case 0x08: sprintf(buffer,"MUL %s,%s",GET1s(opc),GET2s(opc)); size=2; break;
		case 0x09: sprintf(buffer,"DIV %s,%s",GET1s(opc),GET2s(opc)); size=2; break;
		case 0x0a: sprintf(buffer,"MULU %s,%s",GET1s(opc),GET2s(opc)); size=2; break;
		case 0x0b: sprintf(buffer,"DIVU %s,%s",GET1s(opc),GET2s(opc)); size=2; break;
		case 0x0c: sprintf(buffer,"OR %s,%s",GET1s(opc),GET2s(opc)); 	size=2; break;
		case 0x0d: sprintf(buffer,"AND %s,%s",GET1s(opc),GET2s(opc)); size=2; break;
		case 0x0e: sprintf(buffer,"XOR %s,%s",GET1s(opc),GET2s(opc)); size=2; break;
		case 0x0f: sprintf(buffer,"NOT %s,%s",GET1s(opc),GET2s(opc)); size=2; break;
		case 0x10: sprintf(buffer,"MOV %X,%s",I5(opc),GET2s(opc)); size=2; break;
		case 0x11: sprintf(buffer,"ADD %X,%s",I5(opc),GET2s(opc)); size=2; break;
		case 0x12: sprintf(buffer,"SETF %X,%s",I5(opc),GET2s(opc)); size=2; break;
		case 0x13: sprintf(buffer,"CMP %X,%s",I5(opc),GET2s(opc)); size=2; break;
		case 0x14: sprintf(buffer,"SHL %X,%s",UI5(opc),GET2s(opc)); size=2; break;
		case 0x15: sprintf(buffer,"SHR %X,%s",UI5(opc),GET2s(opc)); size=2; break;
		case 0x16: sprintf(buffer,"EI"); size=2; break;
		case 0x17: sprintf(buffer,"SAR %X,%s",UI5(opc),GET2s(opc)); size=2; break;
		case 0x18: sprintf(buffer,"TRAP %X",I5(opc)); size=2; break;
		case 0x19: sprintf(buffer,"RETI"); size=2; flags = DASMFLAG_STEP_OUT; break;
		case 0x1a: sprintf(buffer,"HALT"); size=2; break;
		case 0x1b: sprintf(buffer,"Unk 0x1B"); size=2; break;
		case 0x1c: sprintf(buffer,"LDSR %s,%s",GET2s(opc),GETRs(opc));size=2; break;
		case 0x1d: sprintf(buffer,"STSR %s,%s",GETRs(opc),GET2s(opc));size=2; break;
		case 0x1e: sprintf(buffer,"DI"); size=2; break;
		case 0x1f:
				   	switch(opc&0x1f)
						{
							case 0x00:	sprintf(buffer,"SCH0BSU"); break;
							case 0x01:	sprintf(buffer,"SCH0BSD"); break;
							case 0x02:	sprintf(buffer,"SCH1BSU"); break;
							case 0x03:	sprintf(buffer,"SCH1BSD"); break;
							case 0x04:	sprintf(buffer,"UnkS 4"); break;
							case 0x05:	sprintf(buffer,"UnkS 5"); break;
							case 0x06:	sprintf(buffer,"UnkS 6"); break;
							case 0x08:	sprintf(buffer,"ORBSU"); break;
							case 0x09:	sprintf(buffer,"ANDBSU"); break;
							case 0x0a:	sprintf(buffer,"XORBSU"); break;
							case 0x0b:	sprintf(buffer,"MOVBSU"); break;
							case 0x0c:	sprintf(buffer,"ORNBSU"); break;
							case 0x0d:	sprintf(buffer,"ANDNBSU"); break;
							case 0x0e:	sprintf(buffer,"XORNBSU"); break;
							case 0x0f:	sprintf(buffer,"NOTBSU"); break;
							default: 		sprintf(buffer,"UnkBS 0x%X",opc&0x1f); break;
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
							 		case 0x0: sprintf(buffer,"BV %X",pc+D9(opc));  break;
							 		case 0x1: sprintf(buffer,"BL %X",pc+D9(opc));  break;
							 		case 0x2: sprintf(buffer,"BE %X",pc+D9(opc));  break;
							 		case 0x3: sprintf(buffer,"BNH %X",pc+D9(opc)); break;
							 		case 0x4: sprintf(buffer,"BN %X",pc+D9(opc));  break;
							 		case 0x5: sprintf(buffer,"BR %X",pc+D9(opc));  break;
							 		case 0x6: sprintf(buffer,"BLT %X",pc+D9(opc)); break;
							 		case 0x7: sprintf(buffer,"BLE %X",pc+D9(opc)); break;
							 		case 0x8: sprintf(buffer,"BNV %X",pc+D9(opc)); break;
							 		case 0x9: sprintf(buffer,"BNL %X",pc+D9(opc)); break;
							 		case 0xa: sprintf(buffer,"BNE %X",pc+D9(opc)); break;
							 		case 0xb: sprintf(buffer,"BH %X",pc+D9(opc));  break;
							 		case 0xc: sprintf(buffer,"BP %X",pc+D9(opc));  break;
							 		case 0xd: sprintf(buffer,"NOP"); break;
							 		case 0xe: sprintf(buffer,"BGE %X",pc+D9(opc)); break;
							 		case 0xf: sprintf(buffer,"BGT %X",pc+D9(opc)); break;
						 	 }
						 	 size=2;
						 	 break;

		case 0x28:	sprintf(buffer,"MOVEA %X, %s, %s",I16(opc2),GET1s(opc),GET2s(opc));size=4; break;
		case 0x29:	sprintf(buffer,"ADDI %X, %s, %s",I16(opc2),GET1s(opc),GET2s(opc));size=4; break;
		case 0x2a:	sprintf(buffer,"JR %X",pc+D26(opc,opc2));size=4; break;
		case 0x2b:	sprintf(buffer,"JAL %X",pc+D26(opc,opc2));size=4; flags = DASMFLAG_STEP_OVER; break;
		case 0x2c:	sprintf(buffer,"ORI %X, %s, %s",UI16(opc2),GET1s(opc),GET2s(opc));size=4; break;
		case 0x2d:	sprintf(buffer,"ANDI %X, %s, %s",UI16(opc2),GET1s(opc),GET2s(opc));size=4; break;
		case 0x2e:	sprintf(buffer,"XORI %X, %s, %s",UI16(opc2),GET1s(opc),GET2s(opc));size=4; break;
		case 0x2f:	sprintf(buffer,"MOVHI %X, %s, %s",UI16(opc2),GET1s(opc),GET2s(opc));size=4; break;
		case 0x30:	sprintf(buffer,"LDB %X[%s], %s",D16(opc2),GET1s(opc),GET2s(opc));size=4; break;
		case 0x31:	sprintf(buffer,"LDH %X[%s], %s",D16(opc2),GET1s(opc),GET2s(opc));size=4; break;
		case 0x32:  sprintf(buffer,"Unk 0x32"); size=2; break;
		case 0x33:	sprintf(buffer,"LDW %X[%s], %s",D16(opc2),GET1s(opc),GET2s(opc));size=4; break;
		case 0x34:	sprintf(buffer,"STB %s, %X[%s]",GET2s(opc),D16(opc2),GET1s(opc));size=4; break;
		case 0x35:	sprintf(buffer,"STH %s, %X[%s]",GET2s(opc),D16(opc2),GET1s(opc));size=4; break;
		case 0x36:  sprintf(buffer,"Unk 0x36"); size=2; break;
		case 0x37:	sprintf(buffer,"STW %s, %X[%s]",GET2s(opc),D16(opc2),GET1s(opc));size=4; break;
		case 0x38:	sprintf(buffer,"INB %X[%s], %s",D16(opc2),GET1s(opc),GET2s(opc));size=4; break;
		case 0x39:	sprintf(buffer,"INH %X[%s], %s",D16(opc2),GET1s(opc),GET2s(opc));size=4; break;
		case 0x3a:	sprintf(buffer,"CAXI %X[%s], %s",D16(opc2),GET1s(opc),GET2s(opc));size=4; break;
		case 0x3b:	sprintf(buffer,"INW %X[%s], %s",D16(opc2),GET1s(opc),GET2s(opc));size=4; break;

		case 0x3c:	sprintf(buffer,"OUTB %s, %X[%s]",GET2s(opc),D16(opc2),GET1s(opc));size=4; break;
		case 0x3d:	sprintf(buffer,"OUTH %s, %X[%s]",GET2s(opc),D16(opc2),GET1s(opc));size=4; break;
		case 0x3e:
								switch((opc2&0xfc00)>>10)
								{
									case 0x0: sprintf(buffer,"CMPF.S %s, %s",GET1s(opc),GET2s(opc)); break;
									case 0x2: sprintf(buffer,"CVT.WS %s, %s",GET1s(opc),GET2s(opc)); break;
									case 0x3: sprintf(buffer,"CVT.SW %s, %s",GET1s(opc),GET2s(opc)); break;
									case 0x4: sprintf(buffer,"ADDF.S %s, %s",GET1s(opc),GET2s(opc)); break;
									case 0x5: sprintf(buffer,"SUBF.S %s, %s",GET1s(opc),GET2s(opc)); break;
									case 0x6: sprintf(buffer,"MULF.S %s, %s",GET1s(opc),GET2s(opc)); break;
									case 0x7: sprintf(buffer,"DIVF.S %s, %s",GET1s(opc),GET2s(opc)); break;
									case 0xb: sprintf(buffer,"TRNC.SW %s, %s",GET1s(opc),GET2s(opc)); break;
									default : sprintf(buffer,"Unkf 0x%X",(opc2&0xfc00)>>10); break;
							}
							size=4;
							break;
		case 0x3f:	sprintf(buffer,"OUTW %s, %X[%s]",GET2s(opc),D16(opc2),GET1s(opc));size=4; break;

		default : size=2;
	}
	return size | flags | DASMFLAG_SUPPORTED;
}

