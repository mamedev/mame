// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
#include "emu.h"
#include "superfx.h"

static char *output;

static void ATTR_PRINTF(1,2) print(const char *fmt, ...)
{
	va_list vl;

	va_start(vl, fmt);
	output += vsprintf(output, fmt, vl);
	va_end(vl);
}

offs_t superfx_dasm_one(char *buffer, offs_t pc, UINT8 op, UINT8 param0, UINT8 param1, UINT16 alt)
{
	UINT8 bytes_consumed = 1;
	output = buffer;

	switch(op)
	{
		case 0x00: // STOP
			print("STOP");
			break;
		case 0x01: // NOP
			print("NOP");
			break;
		case 0x02: // CACHE
			print("CACHE");
			break;
		case 0x03: // LSR
			print("LSR");
			break;
		case 0x04: // ROL
			print("ROL");
			break;
		case 0x05: // BRA
			print("BRA     %d", (INT8)param0);
			bytes_consumed = 2;
			break;
		case 0x06: // BLT
			print("BLT     %d", (INT8)param0);
			bytes_consumed = 2;
			break;
		case 0x07: // BGE
			print("BGE     %d", (INT8)param0);
			bytes_consumed = 2;
			break;
		case 0x08: // BNE
			print("BNE     %d", (INT8)param0);
			bytes_consumed = 2;
			break;
		case 0x09: // BEQ
			print("BEQ     %d", (INT8)param0);
			bytes_consumed = 2;
			break;
		case 0x0a: // BPL
			print("BPL     %d", (INT8)param0);
			bytes_consumed = 2;
			break;
		case 0x0b: // BMI
			print("BMI     %d", (INT8)param0);
			bytes_consumed = 2;
			break;
		case 0x0c: // BCC
			print("BCC     %d", (INT8)param0);
			bytes_consumed = 2;
			break;
		case 0x0d: // BCS
			print("BCS     %d", (INT8)param0);
			bytes_consumed = 2;
			break;
		case 0x0e: // BVC
			print("BVC     %d", (INT8)param0);
			bytes_consumed = 2;
			break;
		case 0x0f: // BVS
			print("BVS     %d", (INT8)param0);
			bytes_consumed = 2;
			break;

		case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
		case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f: // TO
			print("TO      R%d", op & 0xf);
			break;

		case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
		case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f: // WITH
			print("WITH    R%d", op & 0xf);
			break;

		case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35:
		case 0x36: case 0x37: case 0x38: case 0x39: case 0x3a: case 0x3b:   // STW_IR / STB_IR
			switch(alt)
			{
				case SUPERFX_SFR_ALT0:
				case SUPERFX_SFR_ALT2:
					print("STW     (R%d)", op & 0xf);
					break;
				case SUPERFX_SFR_ALT1:
				case SUPERFX_SFR_ALT3:
					print("STB     (R%d)", op & 0xf);
					break;
			}
			break;

		case 0x3c: // LOOP
			print("LOOP");
			break;
		case 0x3d: // ALT1
			print("ALT1");
			break;
		case 0x3e: // ALT2
			print("ALT2");
			break;
		case 0x3f: // ALT3
			print("ALT3");
			break;

		case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45:
		case 0x46: case 0x47: case 0x48: case 0x49: case 0x4a: case 0x4b:   // LDW_IR / LDB_IR
			switch(alt)
			{
				case SUPERFX_SFR_ALT0:
				case SUPERFX_SFR_ALT2:
					print("LDW     (R%d)", op & 0xf);
					break;
				case SUPERFX_SFR_ALT1:
				case SUPERFX_SFR_ALT3:
					print("LDB     (R%d)", op & 0xf);
					break;
			}
			break;

		case 0x4c: // PLOT / RPIX
			switch(alt)
			{
				case SUPERFX_SFR_ALT0:
				case SUPERFX_SFR_ALT2:
					print("PLOT");
					break;
				case SUPERFX_SFR_ALT1:
				case SUPERFX_SFR_ALT3:
					print("RPIX");
					break;
			}
			break;

		case 0x4d: // SWAP
			print("SWAP");
			break;

		case 0x4e: // COLOR / CMODE
			switch(alt)
			{
				case SUPERFX_SFR_ALT0:
				case SUPERFX_SFR_ALT2:
					print("COLOR");
					break;
				case SUPERFX_SFR_ALT1:
				case SUPERFX_SFR_ALT3:
					print("CMODE");
					break;
			}
			break;

		case 0x4f: // NOT
			print("NOT");
			break;

		case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
		case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f: // ADD / ADC / ADDI / ADCI
			switch(alt)
			{
				case SUPERFX_SFR_ALT0:
					print("ADD     R%d", op &0xf);
					break;
				case SUPERFX_SFR_ALT1:
					print("ADC     R%d", op &0xf);
					break;
				case SUPERFX_SFR_ALT2:
					print("ADDI    R%d", op &0xf);
					break;
				case SUPERFX_SFR_ALT3:
					print("ADCI    R%d", op &0xf);
					break;
			}
			break;

		case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67:
		case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6e: case 0x6f: // SUB / SBC / SUBI / CMP
			switch(alt)
			{
				case SUPERFX_SFR_ALT0:
					print("SUB     R%d", op &0xf);
					break;
				case SUPERFX_SFR_ALT1:
					print("SBC     R%d", op &0xf);
					break;
				case SUPERFX_SFR_ALT2:
					print("SUBI    R%d", op &0xf);
					break;
				case SUPERFX_SFR_ALT3:
					print("CMP     R%d", op &0xf);
					break;
			}
			break;

		case 0x70: // MERGE
			print("MERGE");
			break;

					case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
		case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f: // AND / BIC / ANDI / BICI
			switch(alt)
			{
				case SUPERFX_SFR_ALT0:
					print("AND     R%d", op &0xf);
					break;
				case SUPERFX_SFR_ALT1:
					print("BIC     R%d", op &0xf);
					break;
				case SUPERFX_SFR_ALT2:
					print("ANDI    R%d", op &0xf);
					break;
				case SUPERFX_SFR_ALT3:
					print("BICI    R%d", op &0xf);
					break;
			}
			break;

		case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
		case 0x88: case 0x89: case 0x8a: case 0x8b: case 0x8c: case 0x8d: case 0x8e: case 0x8f: // MULT / UMULT / MULTI / UMULTI
			switch(alt)
			{
				case SUPERFX_SFR_ALT0:
					print("MULT    R%d", op &0xf);
					break;
				case SUPERFX_SFR_ALT1:
					print("UMULT   R%d", op &0xf);
					break;
				case SUPERFX_SFR_ALT2:
					print("MULTI   R%d", op &0xf);
					break;
				case SUPERFX_SFR_ALT3:
					print("UMULTI  R%d", op &0xf);
					break;
			}
			break;

		case 0x90: // SBK
			print("SBK");
			break;

		case 0x91: case 0x92: case 0x93: case 0x94: // LINK
			print("LINK    %d", op & 0xf);
			break;

		case 0x95: // SEX
			print("SEX");
			break;

		case 0x96: // ASR / DIV2
			switch(alt)
			{
				case SUPERFX_SFR_ALT0:
				case SUPERFX_SFR_ALT2:
					print("ASR");
					break;
				case SUPERFX_SFR_ALT1:
				case SUPERFX_SFR_ALT3:
					print("DIV2");
					break;
			}
			break;

		case 0x97: // ROR
			print("ROR");
			break;

		case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: // JMP / LJMP
			switch(alt)
			{
				case SUPERFX_SFR_ALT0:
				case SUPERFX_SFR_ALT2:
					print("JMP     R%d", op & 0xf);
					break;
				case SUPERFX_SFR_ALT1:
				case SUPERFX_SFR_ALT3:
					print("LJMP    R%d", op & 0xf);
					break;
			}
			break;

		case 0x9e: // LOB
			print("LOB");
			break;

		case 0x9f: // FMULT / LMULT
			switch(alt)
			{
				case SUPERFX_SFR_ALT0:
				case SUPERFX_SFR_ALT2:
					print("FMULT");
					break;
				case SUPERFX_SFR_ALT1:
				case SUPERFX_SFR_ALT3:
					print("LMULT");
					break;
			}
			break;

		case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6: case 0xa7:
		case 0xa8: case 0xa9: case 0xaa: case 0xab: case 0xac: case 0xad: case 0xae: case 0xaf: // IBT / LMS / SMS / LML
			switch(alt)
			{
				case SUPERFX_SFR_ALT0:
					print("IBT     R%d,0x%02x", op & 0xf, param0);
					break;
				case SUPERFX_SFR_ALT2:
					print("SMS     R%d,(0x%04x)", op & 0xf, param0 << 1);
					break;
				case SUPERFX_SFR_ALT1:
				case SUPERFX_SFR_ALT3:
					print("LMS     R%d,(0x%04x)", op & 0xf, param0 << 1);
					break;
			}
			bytes_consumed = 2;
			break;

		case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb5: case 0xb6: case 0xb7:
		case 0xb8: case 0xb9: case 0xba: case 0xbb: case 0xbc: case 0xbd: case 0xbe: case 0xbf: // FROM
			print("FROM    R%d", op & 0xf);
			break;

		case 0xc0: // HIB
			print("HIB");
			break;

					case 0xc1: case 0xc2: case 0xc3: case 0xc4: case 0xc5: case 0xc6: case 0xc7:
		case 0xc8: case 0xc9: case 0xca: case 0xcb: case 0xcc: case 0xcd: case 0xce: case 0xcf: // OR / XOR / ORI / XORI
			switch(alt)
			{
				case SUPERFX_SFR_ALT0:
					print("OR      R%d", op &0xf);
					break;
				case SUPERFX_SFR_ALT1:
					print("XOR     R%d", op &0xf);
					break;
				case SUPERFX_SFR_ALT2:
					print("ORI     R%d", op &0xf);
					break;
				case SUPERFX_SFR_ALT3:
					print("XORI    R%d", op &0xf);
					break;
			}
			break;

		case 0xd0: case 0xd1: case 0xd2: case 0xd3: case 0xd4: case 0xd5: case 0xd6: case 0xd7:
		case 0xd8: case 0xd9: case 0xda: case 0xdb: case 0xdc: case 0xdd: case 0xde:            // INC
			print("INC     R%d", op & 0xf);
			break;

		case 0xdf: // GETC / RAMB / ROMB
			switch(alt)
			{
				case SUPERFX_SFR_ALT0:
				case SUPERFX_SFR_ALT1:
					print("GETC");
					break;
				case SUPERFX_SFR_ALT2:
					print("RAMB");
					break;
				case SUPERFX_SFR_ALT3:
					print("ROMB");
					break;
			}
			break;

		case 0xe0: case 0xe1: case 0xe2: case 0xe3: case 0xe4: case 0xe5: case 0xe6: case 0xe7:
		case 0xe8: case 0xe9: case 0xea: case 0xeb: case 0xec: case 0xed: case 0xee:            // DEC
			print("DEC     R%d", op & 0xf);
			break;

		case 0xef: // GETB / GETBH / GETBL / GETBS
			switch(alt)
			{
				case SUPERFX_SFR_ALT0:
					print("GETB");
					break;
				case SUPERFX_SFR_ALT1:
					print("GETBH");
					break;
				case SUPERFX_SFR_ALT2:
					print("GETBL");
					break;
				case SUPERFX_SFR_ALT3:
					print("GETBS");
					break;
			}
			break;

		case 0xf0: case 0xf1: case 0xf2: case 0xf3: case 0xf4: case 0xf5: case 0xf6: case 0xf7:
		case 0xf8: case 0xf9: case 0xfa: case 0xfb: case 0xfc: case 0xfd: case 0xfe: case 0xff: // IWT / LM / SM / LM
			switch(alt)
			{
				case SUPERFX_SFR_ALT0:
					print("IWT     R%d,#%02x%02x", op & 0xf, param1, param0);
					bytes_consumed = 3;
					break;
				case SUPERFX_SFR_ALT2:
					print("SM      R%d", op & 0xf);
					break;
				case SUPERFX_SFR_ALT1:
				case SUPERFX_SFR_ALT3:
					print("LM      R%d", op & 0xf);
					break;
			}
			break;
	}

	return bytes_consumed | DASMFLAG_SUPPORTED;
}
