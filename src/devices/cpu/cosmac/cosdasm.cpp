// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    cosdasm.c

    Simple RCA COSMAC disassembler.
    Written by Curt Coder

***************************************************************************/

#include "emu.h"

enum
{
	TYPE_1801,
	TYPE_1802
};

#define CDP1801_OPCODE(...) \
	sprintf(buffer, __VA_ARGS__)

#define CDP1802_OPCODE(...) \
	if (variant < TYPE_1802) sprintf(buffer, "illegal"); else sprintf(buffer, __VA_ARGS__)

static offs_t implied(const UINT8 opcode)
{
	return opcode & 0x0f;
}

static offs_t immediate(const UINT8 **opram)
{
	return *(*opram)++;
}

static offs_t short_branch(offs_t pc, const UINT8 **opram)
{
	return (pc & 0xff00) | immediate(opram);
}

static offs_t long_branch(const UINT8 **opram)
{
	return (immediate(opram) << 8) | immediate(opram);
}

static offs_t short_skip(offs_t pc)
{
	return pc + 2;
}

static offs_t long_skip(offs_t pc)
{
	return pc + 3;
}

static UINT32 disassemble(device_t *device, char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 variant)
{
	const UINT8 *startram = opram;
	UINT32 flags = 0;

	opram++;
	UINT8 opcode = *oprom++;

	switch (opcode)
	{
	case 0x00: CDP1801_OPCODE("IDL"); break;
	case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
	case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
		CDP1801_OPCODE("LDN R%01X", implied(opcode)); break;
	case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
	case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
		CDP1801_OPCODE("INC R%01X", implied(opcode)); break;
	case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
	case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
		CDP1801_OPCODE("DEC R%01X", implied(opcode)); break;
	case 0x30: CDP1801_OPCODE("BR %04X", short_branch(pc, &opram)); break;
	case 0x32: CDP1801_OPCODE("BZ %04X", short_branch(pc, &opram)); break;
	case 0x33: CDP1801_OPCODE("BDF %04X", short_branch(pc, &opram)); break;
	case 0x34: CDP1801_OPCODE("B1 %04X", short_branch(pc, &opram)); break;
	case 0x35: CDP1801_OPCODE("B2 %04X", short_branch(pc, &opram)); break;
	case 0x36: CDP1801_OPCODE("B3 %04X", short_branch(pc, &opram)); break;
	case 0x37: CDP1801_OPCODE("B4 %04X", short_branch(pc, &opram)); break;
	case 0x38: CDP1801_OPCODE("SKP %04X", short_skip(pc)); break;
	case 0x3a: CDP1801_OPCODE("BNZ %04X", short_branch(pc, &opram)); break;
	case 0x3b: CDP1801_OPCODE("BNF %04X", short_branch(pc, &opram)); break;
	case 0x3c: CDP1801_OPCODE("BN1 %04X", short_branch(pc, &opram)); break;
	case 0x3d: CDP1801_OPCODE("BN2 %04X", short_branch(pc, &opram)); break;
	case 0x3e: CDP1801_OPCODE("BN3 %04X", short_branch(pc, &opram)); break;
	case 0x3f: CDP1801_OPCODE("BN4 %04X", short_branch(pc, &opram)); break;
	case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47:
	case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4e: case 0x4f:
		CDP1801_OPCODE("LDA R%01X", implied(opcode)); break;
	case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
	case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
		CDP1801_OPCODE("STR R%01X", implied(opcode)); break;
	case 0x61: CDP1801_OPCODE("OUT 1"); break;
	case 0x62: CDP1801_OPCODE("OUT 2"); break;
	case 0x63: CDP1801_OPCODE("OUT 3"); break;
	case 0x64: CDP1801_OPCODE("OUT 4"); break;
	case 0x65: CDP1801_OPCODE("OUT 5"); break;
	case 0x66: CDP1801_OPCODE("OUT 6"); break;
	case 0x67: CDP1801_OPCODE("OUT 7"); break;
	case 0x69: CDP1801_OPCODE("INP 1"); break;
	case 0x6a: CDP1801_OPCODE("INP 2"); break;
	case 0x6b: CDP1801_OPCODE("INP 3"); break;
	case 0x6c: CDP1801_OPCODE("INP 4"); break;
	case 0x6d: CDP1801_OPCODE("INP 5"); break;
	case 0x6e: CDP1801_OPCODE("INP 6"); break;
	case 0x6f: CDP1801_OPCODE("INP 7"); break;
	case 0x70: CDP1801_OPCODE("RET"); flags = DASMFLAG_STEP_OUT; break;
	case 0x71: CDP1801_OPCODE("DIS"); flags = DASMFLAG_STEP_OUT; break;
	case 0x78: CDP1801_OPCODE("SAV"); break;
	case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
	case 0x88: case 0x89: case 0x8a: case 0x8b: case 0x8c: case 0x8d: case 0x8e: case 0x8f:
		CDP1801_OPCODE("GLO R%01X", implied(opcode)); break;
	case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
	case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: case 0x9e: case 0x9f:
		CDP1801_OPCODE("GHI R%01X", implied(opcode)); break;
	case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6: case 0xa7:
	case 0xa8: case 0xa9: case 0xaa: case 0xab: case 0xac: case 0xad: case 0xae: case 0xaf:
		CDP1801_OPCODE("PLO R%01X", implied(opcode)); break;
	case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb5: case 0xb6: case 0xb7:
	case 0xb8: case 0xb9: case 0xba: case 0xbb: case 0xbc: case 0xbd: case 0xbe: case 0xbf:
		CDP1801_OPCODE("PHI R%01X", implied(opcode)); break;
	case 0xd0: case 0xd1: case 0xd2: case 0xd3: case 0xd4: case 0xd5: case 0xd6: case 0xd7:
	case 0xd8: case 0xd9: case 0xda: case 0xdb: case 0xdc: case 0xdd: case 0xde: case 0xdf:
		CDP1801_OPCODE("SEP R%01X", implied(opcode)); flags = DASMFLAG_STEP_OVER; break;
	case 0xe0: case 0xe1: case 0xe2: case 0xe3: case 0xe4: case 0xe5: case 0xe6: case 0xe7:
	case 0xe8: case 0xe9: case 0xea: case 0xeb: case 0xec: case 0xed: case 0xee: case 0xef:
		CDP1801_OPCODE("SEX R%01X", implied(opcode)); break;
	case 0xf0: CDP1801_OPCODE("LDX"); break;
	case 0xf1: CDP1801_OPCODE("OR"); break;
	case 0xf2: CDP1801_OPCODE("AND"); break;
	case 0xf3: CDP1801_OPCODE("XOR"); break;
	case 0xf4: CDP1801_OPCODE("ADD"); break;
	case 0xf5: CDP1801_OPCODE("SD"); break;
	case 0xf6: CDP1801_OPCODE("SHR"); break;
	case 0xf7: CDP1801_OPCODE("SM"); break;
	case 0xf8: CDP1801_OPCODE("LDI #%02X", immediate(&opram)); break;
	case 0xf9: CDP1801_OPCODE("ORI #%02X", immediate(&opram)); break;
	case 0xfa: CDP1801_OPCODE("ANI #%02X", immediate(&opram)); break;
	case 0xfb: CDP1801_OPCODE("XRI #%02X", immediate(&opram)); break;
	case 0xfc: CDP1801_OPCODE("ADI #%02X", immediate(&opram)); break;
	case 0xfd: CDP1801_OPCODE("SDI #%02X", immediate(&opram)); break;
	case 0xff: CDP1801_OPCODE("SMI #%02X", immediate(&opram)); break;
	// CDP1802
	case 0x31: CDP1802_OPCODE("BQ %04X", short_branch(pc, &opram)); break;
	case 0x39: CDP1802_OPCODE("BNQ %04X", short_branch(pc, &opram)); break;
	case 0x60: CDP1802_OPCODE("IRX"); break;
	case 0x72: CDP1802_OPCODE("LDXA"); break;
	case 0x73: CDP1802_OPCODE("STXD"); break;
	case 0x74: CDP1802_OPCODE("ADC"); break;
	case 0x75: CDP1802_OPCODE("SDB"); break;
	case 0x76: CDP1802_OPCODE("SHRC"); break;
	case 0x77: CDP1802_OPCODE("SMB"); break;
	case 0x79: CDP1802_OPCODE("MARK"); break;
	case 0x7a: CDP1802_OPCODE("REQ"); break;
	case 0x7b: CDP1802_OPCODE("SEQ"); break;
	case 0x7c: CDP1802_OPCODE("ADCI #%02X", immediate(&opram)); break;
	case 0x7d: CDP1802_OPCODE("SDBI #%02X", immediate(&opram)); break;
	case 0x7e: CDP1802_OPCODE("SHLC"); break;
	case 0x7f: CDP1802_OPCODE("SMBI #%02X", immediate(&opram)); break;
	case 0xc0: CDP1802_OPCODE("LBR %04X", long_branch(&opram)); break;
	case 0xc1: CDP1802_OPCODE("LBQ %04X", long_branch(&opram)); break;
	case 0xc2: CDP1802_OPCODE("LBZ %04X", long_branch(&opram)); break;
	case 0xc3: CDP1802_OPCODE("LBDF %04X", long_branch(&opram)); break;
	case 0xc4: CDP1802_OPCODE("NOP"); break;
	case 0xc5: CDP1802_OPCODE("LSNQ %04X", long_skip(pc)); break;
	case 0xc6: CDP1802_OPCODE("LSNZ %04X", long_skip(pc)); break;
	case 0xc7: CDP1802_OPCODE("LSNF %04X", long_skip(pc)); break;
	case 0xc8: CDP1802_OPCODE("LSKP %04X", long_skip(pc)); break;
	case 0xc9: CDP1802_OPCODE("LBNQ %04X", long_skip(pc)); break;
	case 0xca: CDP1802_OPCODE("LBNZ %04X", long_skip(pc)); break;
	case 0xcb: CDP1802_OPCODE("LBNF %04X", long_skip(pc)); break;
	case 0xcc: CDP1802_OPCODE("LSIE %04X", long_skip(pc)); break;
	case 0xcd: CDP1802_OPCODE("LSQ %04X", long_skip(pc)); break;
	case 0xce: CDP1802_OPCODE("LSZ %04X", long_skip(pc)); break;
	case 0xcf: CDP1802_OPCODE("LSDF %04X", long_skip(pc)); break;
	case 0xfe: CDP1802_OPCODE("SHL"); break;
	//
	default:   CDP1801_OPCODE("illegal"); break;
	}

	return (opram - startram) | flags | DASMFLAG_SUPPORTED;
}


CPU_DISASSEMBLE( cdp1801 )
{
	return disassemble(device, buffer, pc, oprom, opram, TYPE_1801);
}


CPU_DISASSEMBLE( cdp1802 )
{
	return disassemble(device, buffer, pc, oprom, opram, TYPE_1802);
}
