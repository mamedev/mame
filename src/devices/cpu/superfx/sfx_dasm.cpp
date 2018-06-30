// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
#include "emu.h"
#include "sfx_dasm.h"

superfx_disassembler::superfx_disassembler(config *conf) : m_config(conf)
{
}

u32 superfx_disassembler::opcode_alignment() const
{
	return 1;
}

offs_t superfx_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	uint8_t  op = opcodes.r8(pc);
	uint8_t  param0 = opcodes.r8(pc+1);
	uint8_t  param1 = opcodes.r8(pc+2);
	uint16_t alt = m_config->get_alt();

	uint8_t bytes_consumed = 1;

	switch(op)
	{
		case 0x00: // STOP
			util::stream_format(stream, "STOP");
			break;
		case 0x01: // NOP
			util::stream_format(stream, "NOP");
			break;
		case 0x02: // CACHE
			util::stream_format(stream, "CACHE");
			break;
		case 0x03: // LSR
			util::stream_format(stream, "LSR");
			break;
		case 0x04: // ROL
			util::stream_format(stream, "ROL");
			break;
		case 0x05: // BRA
			util::stream_format(stream, "BRA     %d", (int)(int8_t)param0);
			bytes_consumed = 2;
			break;
		case 0x06: // BLT
			util::stream_format(stream, "BLT     %d", (int)(int8_t)param0);
			bytes_consumed = 2;
			break;
		case 0x07: // BGE
			util::stream_format(stream, "BGE     %d", (int)(int8_t)param0);
			bytes_consumed = 2;
			break;
		case 0x08: // BNE
			util::stream_format(stream, "BNE     %d", (int)(int8_t)param0);
			bytes_consumed = 2;
			break;
		case 0x09: // BEQ
			util::stream_format(stream, "BEQ     %d", (int)(int8_t)param0);
			bytes_consumed = 2;
			break;
		case 0x0a: // BPL
			util::stream_format(stream, "BPL     %d", (int)(int8_t)param0);
			bytes_consumed = 2;
			break;
		case 0x0b: // BMI
			util::stream_format(stream, "BMI     %d", (int)(int8_t)param0);
			bytes_consumed = 2;
			break;
		case 0x0c: // BCC
			util::stream_format(stream, "BCC     %d", (int)(int8_t)param0);
			bytes_consumed = 2;
			break;
		case 0x0d: // BCS
			util::stream_format(stream, "BCS     %d", (int)(int8_t)param0);
			bytes_consumed = 2;
			break;
		case 0x0e: // BVC
			util::stream_format(stream, "BVC     %d", (int)(int8_t)param0);
			bytes_consumed = 2;
			break;
		case 0x0f: // BVS
			util::stream_format(stream, "BVS     %d", (int)(int8_t)param0);
			bytes_consumed = 2;
			break;

		case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
		case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f: // TO
			util::stream_format(stream, "TO      R%d", op & 0xf);
			break;

		case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
		case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f: // WITH
			util::stream_format(stream, "WITH    R%d", op & 0xf);
			break;

		case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35:
		case 0x36: case 0x37: case 0x38: case 0x39: case 0x3a: case 0x3b:   // STW_IR / STB_IR
			switch(alt)
			{
				case SUPERFX_SFR_ALT0:
				case SUPERFX_SFR_ALT2:
					util::stream_format(stream, "STW     (R%d)", op & 0xf);
					break;
				case SUPERFX_SFR_ALT1:
				case SUPERFX_SFR_ALT3:
					util::stream_format(stream, "STB     (R%d)", op & 0xf);
					break;
			}
			break;

		case 0x3c: // LOOP
			util::stream_format(stream, "LOOP");
			break;
		case 0x3d: // ALT1
			util::stream_format(stream, "ALT1");
			break;
		case 0x3e: // ALT2
			util::stream_format(stream, "ALT2");
			break;
		case 0x3f: // ALT3
			util::stream_format(stream, "ALT3");
			break;

		case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45:
		case 0x46: case 0x47: case 0x48: case 0x49: case 0x4a: case 0x4b:   // LDW_IR / LDB_IR
			switch(alt)
			{
				case SUPERFX_SFR_ALT0:
				case SUPERFX_SFR_ALT2:
					util::stream_format(stream, "LDW     (R%d)", op & 0xf);
					break;
				case SUPERFX_SFR_ALT1:
				case SUPERFX_SFR_ALT3:
					util::stream_format(stream, "LDB     (R%d)", op & 0xf);
					break;
			}
			break;

		case 0x4c: // PLOT / RPIX
			switch(alt)
			{
				case SUPERFX_SFR_ALT0:
				case SUPERFX_SFR_ALT2:
					util::stream_format(stream, "PLOT");
					break;
				case SUPERFX_SFR_ALT1:
				case SUPERFX_SFR_ALT3:
					util::stream_format(stream, "RPIX");
					break;
			}
			break;

		case 0x4d: // SWAP
			util::stream_format(stream, "SWAP");
			break;

		case 0x4e: // COLOR / CMODE
			switch(alt)
			{
				case SUPERFX_SFR_ALT0:
				case SUPERFX_SFR_ALT2:
					util::stream_format(stream, "COLOR");
					break;
				case SUPERFX_SFR_ALT1:
				case SUPERFX_SFR_ALT3:
					util::stream_format(stream, "CMODE");
					break;
			}
			break;

		case 0x4f: // NOT
			util::stream_format(stream, "NOT");
			break;

		case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
		case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f: // ADD / ADC / ADDI / ADCI
			switch(alt)
			{
				case SUPERFX_SFR_ALT0:
					util::stream_format(stream, "ADD     R%d", op &0xf);
					break;
				case SUPERFX_SFR_ALT1:
					util::stream_format(stream, "ADC     R%d", op &0xf);
					break;
				case SUPERFX_SFR_ALT2:
					util::stream_format(stream, "ADDI    R%d", op &0xf);
					break;
				case SUPERFX_SFR_ALT3:
					util::stream_format(stream, "ADCI    R%d", op &0xf);
					break;
			}
			break;

		case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67:
		case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6e: case 0x6f: // SUB / SBC / SUBI / CMP
			switch(alt)
			{
				case SUPERFX_SFR_ALT0:
					util::stream_format(stream, "SUB     R%d", op &0xf);
					break;
				case SUPERFX_SFR_ALT1:
					util::stream_format(stream, "SBC     R%d", op &0xf);
					break;
				case SUPERFX_SFR_ALT2:
					util::stream_format(stream, "SUBI    R%d", op &0xf);
					break;
				case SUPERFX_SFR_ALT3:
					util::stream_format(stream, "CMP     R%d", op &0xf);
					break;
			}
			break;

		case 0x70: // MERGE
			util::stream_format(stream, "MERGE");
			break;

					case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
		case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f: // AND / BIC / ANDI / BICI
			switch(alt)
			{
				case SUPERFX_SFR_ALT0:
					util::stream_format(stream, "AND     R%d", op &0xf);
					break;
				case SUPERFX_SFR_ALT1:
					util::stream_format(stream, "BIC     R%d", op &0xf);
					break;
				case SUPERFX_SFR_ALT2:
					util::stream_format(stream, "ANDI    R%d", op &0xf);
					break;
				case SUPERFX_SFR_ALT3:
					util::stream_format(stream, "BICI    R%d", op &0xf);
					break;
			}
			break;

		case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
		case 0x88: case 0x89: case 0x8a: case 0x8b: case 0x8c: case 0x8d: case 0x8e: case 0x8f: // MULT / UMULT / MULTI / UMULTI
			switch(alt)
			{
				case SUPERFX_SFR_ALT0:
					util::stream_format(stream, "MULT    R%d", op &0xf);
					break;
				case SUPERFX_SFR_ALT1:
					util::stream_format(stream, "UMULT   R%d", op &0xf);
					break;
				case SUPERFX_SFR_ALT2:
					util::stream_format(stream, "MULTI   R%d", op &0xf);
					break;
				case SUPERFX_SFR_ALT3:
					util::stream_format(stream, "UMULTI  R%d", op &0xf);
					break;
			}
			break;

		case 0x90: // SBK
			util::stream_format(stream, "SBK");
			break;

		case 0x91: case 0x92: case 0x93: case 0x94: // LINK
			util::stream_format(stream, "LINK    %d", op & 0xf);
			break;

		case 0x95: // SEX
			util::stream_format(stream, "SEX");
			break;

		case 0x96: // ASR / DIV2
			switch(alt)
			{
				case SUPERFX_SFR_ALT0:
				case SUPERFX_SFR_ALT2:
					util::stream_format(stream, "ASR");
					break;
				case SUPERFX_SFR_ALT1:
				case SUPERFX_SFR_ALT3:
					util::stream_format(stream, "DIV2");
					break;
			}
			break;

		case 0x97: // ROR
			util::stream_format(stream, "ROR");
			break;

		case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: // JMP / LJMP
			switch(alt)
			{
				case SUPERFX_SFR_ALT0:
				case SUPERFX_SFR_ALT2:
					util::stream_format(stream, "JMP     R%d", op & 0xf);
					break;
				case SUPERFX_SFR_ALT1:
				case SUPERFX_SFR_ALT3:
					util::stream_format(stream, "LJMP    R%d", op & 0xf);
					break;
			}
			break;

		case 0x9e: // LOB
			util::stream_format(stream, "LOB");
			break;

		case 0x9f: // FMULT / LMULT
			switch(alt)
			{
				case SUPERFX_SFR_ALT0:
				case SUPERFX_SFR_ALT2:
					util::stream_format(stream, "FMULT");
					break;
				case SUPERFX_SFR_ALT1:
				case SUPERFX_SFR_ALT3:
					util::stream_format(stream, "LMULT");
					break;
			}
			break;

		case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6: case 0xa7:
		case 0xa8: case 0xa9: case 0xaa: case 0xab: case 0xac: case 0xad: case 0xae: case 0xaf: // IBT / LMS / SMS / LML
			switch(alt)
			{
				case SUPERFX_SFR_ALT0:
					util::stream_format(stream, "IBT     R%d,0x%02x", op & 0xf, param0);
					break;
				case SUPERFX_SFR_ALT2:
					util::stream_format(stream, "SMS     R%d,(0x%04x)", op & 0xf, param0 << 1);
					break;
				case SUPERFX_SFR_ALT1:
				case SUPERFX_SFR_ALT3:
					util::stream_format(stream, "LMS     R%d,(0x%04x)", op & 0xf, param0 << 1);
					break;
			}
			bytes_consumed = 2;
			break;

		case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb5: case 0xb6: case 0xb7:
		case 0xb8: case 0xb9: case 0xba: case 0xbb: case 0xbc: case 0xbd: case 0xbe: case 0xbf: // FROM
			util::stream_format(stream, "FROM    R%d", op & 0xf);
			break;

		case 0xc0: // HIB
			util::stream_format(stream, "HIB");
			break;

					case 0xc1: case 0xc2: case 0xc3: case 0xc4: case 0xc5: case 0xc6: case 0xc7:
		case 0xc8: case 0xc9: case 0xca: case 0xcb: case 0xcc: case 0xcd: case 0xce: case 0xcf: // OR / XOR / ORI / XORI
			switch(alt)
			{
				case SUPERFX_SFR_ALT0:
					util::stream_format(stream, "OR      R%d", op &0xf);
					break;
				case SUPERFX_SFR_ALT1:
					util::stream_format(stream, "XOR     R%d", op &0xf);
					break;
				case SUPERFX_SFR_ALT2:
					util::stream_format(stream, "ORI     R%d", op &0xf);
					break;
				case SUPERFX_SFR_ALT3:
					util::stream_format(stream, "XORI    R%d", op &0xf);
					break;
			}
			break;

		case 0xd0: case 0xd1: case 0xd2: case 0xd3: case 0xd4: case 0xd5: case 0xd6: case 0xd7:
		case 0xd8: case 0xd9: case 0xda: case 0xdb: case 0xdc: case 0xdd: case 0xde:            // INC
			util::stream_format(stream, "INC     R%d", op & 0xf);
			break;

		case 0xdf: // GETC / RAMB / ROMB
			switch(alt)
			{
				case SUPERFX_SFR_ALT0:
				case SUPERFX_SFR_ALT1:
					util::stream_format(stream, "GETC");
					break;
				case SUPERFX_SFR_ALT2:
					util::stream_format(stream, "RAMB");
					break;
				case SUPERFX_SFR_ALT3:
					util::stream_format(stream, "ROMB");
					break;
			}
			break;

		case 0xe0: case 0xe1: case 0xe2: case 0xe3: case 0xe4: case 0xe5: case 0xe6: case 0xe7:
		case 0xe8: case 0xe9: case 0xea: case 0xeb: case 0xec: case 0xed: case 0xee:            // DEC
			util::stream_format(stream, "DEC     R%d", op & 0xf);
			break;

		case 0xef: // GETB / GETBH / GETBL / GETBS
			switch(alt)
			{
				case SUPERFX_SFR_ALT0:
					util::stream_format(stream, "GETB");
					break;
				case SUPERFX_SFR_ALT1:
					util::stream_format(stream, "GETBH");
					break;
				case SUPERFX_SFR_ALT2:
					util::stream_format(stream, "GETBL");
					break;
				case SUPERFX_SFR_ALT3:
					util::stream_format(stream, "GETBS");
					break;
			}
			break;

		case 0xf0: case 0xf1: case 0xf2: case 0xf3: case 0xf4: case 0xf5: case 0xf6: case 0xf7:
		case 0xf8: case 0xf9: case 0xfa: case 0xfb: case 0xfc: case 0xfd: case 0xfe: case 0xff: // IWT / LM / SM / LM
			switch(alt)
			{
				case SUPERFX_SFR_ALT0:
					util::stream_format(stream, "IWT     R%d,#%02x%02x", op & 0xf, param1, param0);
					bytes_consumed = 3;
					break;
				case SUPERFX_SFR_ALT2:
					util::stream_format(stream, "SM      R%d", op & 0xf);
					break;
				case SUPERFX_SFR_ALT1:
				case SUPERFX_SFR_ALT3:
					util::stream_format(stream, "LM      R%d", op & 0xf);
					break;
			}
			break;
	}

	return bytes_consumed | SUPPORTED;
}
