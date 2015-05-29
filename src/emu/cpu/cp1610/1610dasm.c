// license:BSD-3-Clause
// copyright-holders:Frank Palazzolo
#include "emu.h"
#include "debugger.h"
#include "cp1610.h"

CPU_DISASSEMBLE( cp1610 )
{
	UINT16 oprom16[4]={ (oprom[0] << 8) | oprom[1], (oprom[2] << 8) | oprom[3], (oprom[4] << 8) | oprom[5], (oprom[6] << 8) | oprom[7] };
	UINT16 op = oprom16[0]; UINT16 subop;
	UINT16 ea, ea1, ea2;
	unsigned size = 1;
//  const char *sym, *sym2;

	switch( op )
	{
	/* opcode  bitmask */
	case 0x00: /* 0 000 000 000 */
//      sym = set_ea_info( 0, 12, EA_UINT8, EA_ZPG_RD );
		sprintf(buffer, "HLT");
		break;
	case 0x001: /* 0 000 000 001 */
		subop = oprom16[1];
		switch (subop & 0x3f8)
		{
			case 0x280: /* 1 010 000 xxx */
			case 0x288: /* 1 010 001 xxx */
			case 0x290: /* 1 010 010 xxx */
			case 0x298: /* 1 010 011 xxx */
			case 0x2a0: /* 1 010 100 xxx */
			case 0x2a8: /* 1 010 101 xxx */
			case 0x2b0: /* 1 010 110 xxx */
				sprintf(buffer,"MVI@ (R%01d), R%01d",(subop&0x38)>>3,subop&7);
				size += 1;
				break;
			case 0x2b8: /* 1 010 111 xxx */
				ea1 = oprom16[2];
				ea2 = oprom16[3];
				ea = ((ea2&0xff)<<8) | (ea1&0xff);
				sprintf(buffer,"MVII #%04X, R%01d",ea,subop&7);
				size += 3;
				break;
			case 0x2c0: /* 1 011 000 xxx */
			case 0x2c8: /* 1 011 001 xxx */
			case 0x2d0: /* 1 011 010 xxx */
			case 0x2d8: /* 1 011 011 xxx */
			case 0x2e0: /* 1 011 100 xxx */
			case 0x2e8: /* 1 011 101 xxx */
			case 0x2f0: /* 1 011 110 xxx */
				sprintf(buffer,"ADD@ (R%01d), R%01d",(subop&0x38)>>3,subop&7);
				size += 1;
				break;
			case 0x2f8: /* 1 011 111 xxx */
				ea1 = oprom16[2];
				ea2 = oprom16[3];
				ea = ((ea2&0xff)<<8) | (ea1&0xff);
				sprintf(buffer,"ADDI #%04X, R%01d",ea,subop&7);
				size += 3;
				break;
			case 0x300: /* 1 100 000 xxx */
			case 0x308: /* 1 100 001 xxx */
			case 0x310: /* 1 100 010 xxx */
			case 0x318: /* 1 100 011 xxx */
			case 0x320: /* 1 100 100 xxx */
			case 0x328: /* 1 100 101 xxx */
			case 0x330: /* 1 100 110 xxx */
				sprintf(buffer,"SUB@ (R%01d), R%01d",(subop&0x38)>>3,subop&7);
				size += 1;
				break;
			case 0x338: /* 1 100 111 xxx */
				ea1 = oprom16[2];
				ea2 = oprom16[3];
				ea = ((ea2&0xff)<<8) | (ea1&0xff);
				sprintf(buffer,"SUBI #%04X, R%01d",ea,subop&7);
				size += 3;
				break;
			case 0x340: /* 1 101 000 xxx */
			case 0x348: /* 1 101 001 xxx */
			case 0x350: /* 1 101 010 xxx */
			case 0x358: /* 1 101 011 xxx */
			case 0x360: /* 1 101 100 xxx */
			case 0x368: /* 1 101 101 xxx */
			case 0x370: /* 1 101 110 xxx */
				sprintf(buffer,"CMP@ (R%01d), R%01d",(subop&0x38)>>3,subop&7);
				size += 1;
				break;
			case 0x378: /* 1 101 111 xxx */
				ea1 = oprom16[2];
				ea2 = oprom16[3];
				ea = ((ea2&0xff)<<8) | (ea1&0xff);
				sprintf(buffer,"CMPI #%04X, R%01d",ea,subop&7);
				size += 3;
				break;
			case 0x380: /* 1 110 000 xxx */
			case 0x388: /* 1 110 001 xxx */
			case 0x390: /* 1 110 010 xxx */
			case 0x398: /* 1 110 011 xxx */
			case 0x3a0: /* 1 110 100 xxx */
			case 0x3a8: /* 1 110 101 xxx */
			case 0x3b0: /* 1 110 110 xxx */
				sprintf(buffer,"AND@ (R%01d), R%01d",(subop&0x38)>>3,subop&7);
				size += 1;
				break;
			case 0x3b8: /* 1 110 111 xxx */
				ea1 = oprom16[2];
				ea2 = oprom16[3];
				ea = ((ea2&0xff)<<8) | (ea1&0xff);
				sprintf(buffer,"ANDI #%04X, R%01d",ea,subop&7);
				size += 3;
				break;
			case 0x3c0: /* 1 111 000 xxx */
			case 0x3c8: /* 1 111 001 xxx */
			case 0x3d0: /* 1 111 010 xxx */
			case 0x3d8: /* 1 111 011 xxx */
			case 0x3e0: /* 1 111 100 xxx */
			case 0x3e8: /* 1 111 101 xxx */
			case 0x3f0: /* 1 111 110 xxx */
				sprintf(buffer,"XOR@ (R%01d), R%01d",(subop&0x38)>>3,subop&7);
				size += 1;
				break;
			case 0x3f8: /* 1 111 111 xxx */
				ea1 = oprom16[2];
				ea2 = oprom16[3];
				ea = ((ea2&0xff)<<8) | (ea1&0xff);
				sprintf(buffer,"XORI #%04X, R%01d",ea,subop&7);
				size += 3;
				break;
			default:
				size += 1;
				sprintf(buffer, "SDBD ????");
		}
		break;
	case 0x002: /* 0 000 000 010 */
		sprintf(buffer, "EIS");
		break;
	case 0x003: /* 0 000 000 011 */
		sprintf(buffer, "DIS");
		break;
	case 0x004: /* 0 000 000 100 */
		size += 2;
		ea1 = oprom16[1];
		ea2 = oprom16[2];
		ea = ((ea1<<8)&0xfc00) + (ea2&0x3ff);
		if ((ea1&0x300) == 0x300)
		{
			//?? set direct address
			switch(ea1 & 0x03)
			{
				case 0:
					sprintf(buffer, "J    %04X",ea);
					break;
				case 1:
					sprintf(buffer, "JE   %04X",ea);
					break;
				case 2:
					sprintf(buffer, "JD   %04X",ea);
					break;
				case 3:
					sprintf(buffer, "????");
					break;
			}
		}
		else
		{
			//?? set R?, return address
			switch(ea1 & 0x03)
			{
				case 0:
					sprintf(buffer, "JSR  R%01d,%04X",((ea1&0x300)>>8)+4,ea);
					break;
				case 1:
					sprintf(buffer, "JSRE R%01d,%04X",((ea1&0x300)>>8)+4,ea);
					break;
				case 2:
					sprintf(buffer, "JSRD R%01d,%04X",((ea1&0x300)>>8)+4,ea);
					break;
				case 3:
					sprintf(buffer, "????");
					break;
			}
		}
		break;
	case 0x005: /* 0 000 000 101 */
		sprintf(buffer, "TCI");
		break;
	case 0x006: /* 0 000 000 110 */
		sprintf(buffer, "CLRC");
		break;
	case 0x007: /* 0 000 000 111 */
		sprintf(buffer, "SETC");
		break;
	case 0x008: /* 0 000 001 000 */
	case 0x009: /* 0 000 001 001 */
	case 0x00a: /* 0 000 001 010 */
	case 0x00b: /* 0 000 001 011 */
	case 0x00c: /* 0 000 001 100 */
	case 0x00d: /* 0 000 001 101 */
	case 0x00e: /* 0 000 001 110 */
	case 0x00f: /* 0 000 001 111 */
		sprintf(buffer, "INCR R%01d",op&0x7);
		break;
	case 0x010: /* 0 000 010 000 */
	case 0x011: /* 0 000 010 001 */
	case 0x012: /* 0 000 010 010 */
	case 0x013: /* 0 000 010 011 */
	case 0x014: /* 0 000 010 100 */
	case 0x015: /* 0 000 010 101 */
	case 0x016: /* 0 000 010 110 */
	case 0x017: /* 0 000 010 111 */
		sprintf(buffer, "DECR R%01d",op&0x7);
		break;
	case 0x018: /* 0 000 011 000 */
	case 0x019: /* 0 000 011 001 */
	case 0x01a: /* 0 000 011 010 */
	case 0x01b: /* 0 000 011 011 */
	case 0x01c: /* 0 000 011 100 */
	case 0x01d: /* 0 000 011 101 */
	case 0x01e: /* 0 000 011 110 */
	case 0x01f: /* 0 000 011 111 */
		sprintf(buffer, "COMR R%01d",op&0x7);
		break;
	case 0x020: /* 0 000 100 000 */
	case 0x021: /* 0 000 100 001 */
	case 0x022: /* 0 000 100 010 */
	case 0x023: /* 0 000 100 011 */
	case 0x024: /* 0 000 100 100 */
	case 0x025: /* 0 000 100 101 */
	case 0x026: /* 0 000 100 110 */
	case 0x027: /* 0 000 100 111 */
		sprintf(buffer, "NEGR R%01d",op&0x7);
		break;
	case 0x028: /* 0 000 101 000 */
	case 0x029: /* 0 000 101 001 */
	case 0x02a: /* 0 000 101 010 */
	case 0x02b: /* 0 000 101 011 */
	case 0x02c: /* 0 000 101 100 */
	case 0x02d: /* 0 000 101 101 */
	case 0x02e: /* 0 000 101 110 */
	case 0x02f: /* 0 000 101 111 */
		sprintf(buffer, "ADCR R%01d",op&0x7);
		break;
	case 0x030: /* 0 000 110 000 */
	case 0x031: /* 0 000 110 001 */
	case 0x032: /* 0 000 110 010 */
	case 0x033: /* 0 000 110 011 */
		sprintf(buffer, "GSWD R%01d",op&0x3);
		break;
	case 0x034: /* 0 000 110 100 */
	case 0x035: /* 0 000 110 101 */
		sprintf(buffer, "NOP (%01d)",op&0x1); //???
		break;
	case 0x036: /* 0 000 110 110 */
	case 0x037: /* 0 000 110 111 */
		sprintf(buffer, "SIN");
		break;
	case 0x038: /* 0 000 111 000 */
	case 0x039: /* 0 000 111 001 */
	case 0x03a: /* 0 000 111 010 */
	case 0x03b: /* 0 000 111 011 */
	case 0x03c: /* 0 000 111 100 */
	case 0x03d: /* 0 000 111 101 */
	case 0x03e: /* 0 000 111 110 */
	case 0x03f: /* 0 000 111 111 */
		sprintf(buffer, "RSWD R%01d",op&0x7);
		break;
	case 0x040: /* 0 001 000 000 */
	case 0x041: /* 0 001 000 001 */
	case 0x042: /* 0 001 000 010 */
	case 0x043: /* 0 001 000 011 */
		sprintf(buffer, "SWAP R%01d,1",op&0x3);
		break;
	case 0x044: /* 0 001 000 100 */
	case 0x045: /* 0 001 000 101 */
	case 0x046: /* 0 001 000 110 */
	case 0x047: /* 0 001 000 111 */
		sprintf(buffer, "SWAP R%01d,2",op&0x3);
		break;
	case 0x048: /* 0 001 001 000 */
	case 0x049: /* 0 001 001 001 */
	case 0x04a: /* 0 001 001 010 */
	case 0x04b: /* 0 001 001 011 */
		sprintf(buffer, "SLL  R%01d,1",op&0x3);
		break;
	case 0x04c: /* 0 001 001 100 */
	case 0x04d: /* 0 001 001 101 */
	case 0x04e: /* 0 001 001 110 */
	case 0x04f: /* 0 001 001 111 */
		sprintf(buffer, "SLL  R%01d,2",op&0x3);
		break;
	case 0x050: /* 0 001 010 000 */
	case 0x051: /* 0 001 010 001 */
	case 0x052: /* 0 001 010 010 */
	case 0x053: /* 0 001 010 011 */
		sprintf(buffer, "RLC  R%01d,1",op&0x3);
		break;
	case 0x054: /* 0 001 010 100 */
	case 0x055: /* 0 001 010 101 */
	case 0x056: /* 0 001 010 110 */
	case 0x057: /* 0 001 010 111 */
		sprintf(buffer, "RLC  R%01d,2",op&0x3);
		break;
	case 0x058: /* 0 001 011 000 */
	case 0x059: /* 0 001 011 001 */
	case 0x05a: /* 0 001 011 010 */
	case 0x05b: /* 0 001 011 011 */
		sprintf(buffer, "SLLC R%01d,1",op&0x3);
		break;
	case 0x05c: /* 0 001 011 100 */
	case 0x05d: /* 0 001 011 101 */
	case 0x05e: /* 0 001 011 110 */
	case 0x05f: /* 0 001 011 111 */
		sprintf(buffer, "SLLC R%01d,2",op&0x3);
		break;
	case 0x060: /* 0 001 100 000 */
	case 0x061: /* 0 001 100 001 */
	case 0x062: /* 0 001 100 010 */
	case 0x063: /* 0 001 100 011 */
		sprintf(buffer, "SLR  R%01d,1",op&0x3);
		break;
	case 0x064: /* 0 001 100 100 */
	case 0x065: /* 0 001 100 101 */
	case 0x066: /* 0 001 100 110 */
	case 0x067: /* 0 001 100 111 */
		sprintf(buffer, "SLR  R%01d,2",op&0x3);
		break;
	case 0x068: /* 0 001 101 000 */
	case 0x069: /* 0 001 101 001 */
	case 0x06a: /* 0 001 101 010 */
	case 0x06b: /* 0 001 101 011 */
		sprintf(buffer, "SAR  R%01d,1",op&0x3);
		break;
	case 0x06c: /* 0 001 101 100 */
	case 0x06d: /* 0 001 101 101 */
	case 0x06e: /* 0 001 101 110 */
	case 0x06f: /* 0 001 101 111 */
		sprintf(buffer, "SAR  R%01d,2",op&0x3);
		break;
	case 0x070: /* 0 001 110 000 */
	case 0x071: /* 0 001 110 001 */
	case 0x072: /* 0 001 110 010 */
	case 0x073: /* 0 001 110 011 */
		sprintf(buffer, "RRC  R%01d,1",op&0x3);
		break;
	case 0x074: /* 0 001 110 100 */
	case 0x075: /* 0 001 110 101 */
	case 0x076: /* 0 001 110 110 */
	case 0x077: /* 0 001 110 111 */
		sprintf(buffer, "RRC  R%01d,2",op&0x3);
		break;
	case 0x078: /* 0 001 111 000 */
	case 0x079: /* 0 001 111 001 */
	case 0x07a: /* 0 001 111 010 */
	case 0x07b: /* 0 001 111 011 */
		sprintf(buffer, "SARC R%01d,1",op&0x3);
		break;
	case 0x07c: /* 0 001 111 100 */
	case 0x07d: /* 0 001 111 101 */
	case 0x07e: /* 0 001 111 110 */
	case 0x07f: /* 0 001 111 111 */
		sprintf(buffer, "SARC R%01d,2",op&0x3);
		break;
	case 0x080: /* 0 010 000 000 */
	case 0x089: /* 0 010 001 001 */
	case 0x092: /* 0 010 010 010 */
	case 0x09b: /* 0 010 011 011 */
	case 0x0a4: /* 0 010 100 100 */
	case 0x0ad: /* 0 010 101 101 */
	case 0x0b6: /* 0 010 110 110 */
	case 0x0bf: /* 0 010 111 111 */
		sprintf(buffer, "TSTR R%01d",op&0x7);
		break;
	case 0x087: /* 0 010 000 111 */
	case 0x08f: /* 0 010 001 111 */
	case 0x097: /* 0 010 010 111 */
	case 0x09f: /* 0 010 011 111 */
	case 0x0a7: /* 0 010 100 111 */
	case 0x0af: /* 0 010 101 111 */
	case 0x0b7: /* 0 010 110 111 */
		sprintf(buffer, "JR   R%01d",(op&0x38)>>3);
		break;
	case 0x081: /* 0 010 000 001 */
	case 0x082: /* 0 010 000 010 */
	case 0x083: /* 0 010 000 011 */
	case 0x084: /* 0 010 000 100 */
	case 0x085: /* 0 010 000 101 */
	case 0x086: /* 0 010 000 110 */
	case 0x088: /* 0 010 001 000 */
	case 0x08a: /* 0 010 001 010 */
	case 0x08b: /* 0 010 001 011 */
	case 0x08c: /* 0 010 001 100 */
	case 0x08d: /* 0 010 001 101 */
	case 0x08e: /* 0 010 001 110 */
	case 0x090: /* 0 010 010 000 */
	case 0x091: /* 0 010 010 001 */
	case 0x093: /* 0 010 010 011 */
	case 0x094: /* 0 010 010 100 */
	case 0x095: /* 0 010 010 101 */
	case 0x096: /* 0 010 010 110 */
	case 0x098: /* 0 010 011 000 */
	case 0x099: /* 0 010 011 001 */
	case 0x09a: /* 0 010 011 010 */
	case 0x09c: /* 0 010 011 100 */
	case 0x09d: /* 0 010 011 101 */
	case 0x09e: /* 0 010 011 110 */
	case 0x0a0: /* 0 010 100 000 */
	case 0x0a1: /* 0 010 100 001 */
	case 0x0a2: /* 0 010 100 010 */
	case 0x0a3: /* 0 010 100 011 */
	case 0x0a5: /* 0 010 100 101 */
	case 0x0a6: /* 0 010 100 110 */
	case 0x0a8: /* 0 010 101 000 */
	case 0x0a9: /* 0 010 101 001 */
	case 0x0aa: /* 0 010 101 010 */
	case 0x0ab: /* 0 010 101 011 */
	case 0x0ac: /* 0 010 101 100 */
	case 0x0ae: /* 0 010 101 110 */
	case 0x0b0: /* 0 010 110 000 */
	case 0x0b1: /* 0 010 110 001 */
	case 0x0b2: /* 0 010 110 010 */
	case 0x0b3: /* 0 010 110 011 */
	case 0x0b4: /* 0 010 110 100 */
	case 0x0b5: /* 0 010 110 101 */
	case 0x0b8: /* 0 010 111 000 */
	case 0x0b9: /* 0 010 111 001 */
	case 0x0ba: /* 0 010 111 010 */
	case 0x0bb: /* 0 010 111 011 */
	case 0x0bc: /* 0 010 111 100 */
	case 0x0bd: /* 0 010 111 101 */
	case 0x0be: /* 0 010 111 110 */
		sprintf(buffer, "MOVR R%01d,R%01d",(op&0x38)>>3,op&0x7);
		break;
	case 0x0c0: /* 0 011 000 000 */
	case 0x0c1: /* 0 011 000 001 */
	case 0x0c2: /* 0 011 000 010 */
	case 0x0c3: /* 0 011 000 011 */
	case 0x0c4: /* 0 011 000 100 */
	case 0x0c5: /* 0 011 000 101 */
	case 0x0c6: /* 0 011 000 110 */
	case 0x0c7: /* 0 011 000 111 */
	case 0x0c8: /* 0 011 001 000 */
	case 0x0c9: /* 0 011 001 001 */
	case 0x0ca: /* 0 011 001 010 */
	case 0x0cb: /* 0 011 001 011 */
	case 0x0cc: /* 0 011 001 100 */
	case 0x0cd: /* 0 011 001 101 */
	case 0x0ce: /* 0 011 001 110 */
	case 0x0cf: /* 0 011 001 111 */
	case 0x0d0: /* 0 011 010 000 */
	case 0x0d1: /* 0 011 010 001 */
	case 0x0d2: /* 0 011 010 010 */
	case 0x0d3: /* 0 011 010 011 */
	case 0x0d4: /* 0 011 010 100 */
	case 0x0d5: /* 0 011 010 101 */
	case 0x0d6: /* 0 011 010 110 */
	case 0x0d7: /* 0 011 010 111 */
	case 0x0d8: /* 0 011 011 000 */
	case 0x0d9: /* 0 011 011 001 */
	case 0x0da: /* 0 011 011 010 */
	case 0x0db: /* 0 011 011 011 */
	case 0x0dc: /* 0 011 011 100 */
	case 0x0dd: /* 0 011 011 101 */
	case 0x0de: /* 0 011 011 110 */
	case 0x0df: /* 0 011 011 111 */
	case 0x0e0: /* 0 011 100 000 */
	case 0x0e1: /* 0 011 100 001 */
	case 0x0e2: /* 0 011 100 010 */
	case 0x0e3: /* 0 011 100 011 */
	case 0x0e4: /* 0 011 100 100 */
	case 0x0e5: /* 0 011 100 101 */
	case 0x0e6: /* 0 011 100 110 */
	case 0x0e7: /* 0 011 100 111 */
	case 0x0e8: /* 0 011 101 000 */
	case 0x0e9: /* 0 011 101 001 */
	case 0x0ea: /* 0 011 101 010 */
	case 0x0eb: /* 0 011 101 011 */
	case 0x0ec: /* 0 011 101 100 */
	case 0x0ed: /* 0 011 101 101 */
	case 0x0ee: /* 0 011 101 110 */
	case 0x0ef: /* 0 011 101 111 */
	case 0x0f0: /* 0 011 110 000 */
	case 0x0f1: /* 0 011 110 001 */
	case 0x0f2: /* 0 011 110 010 */
	case 0x0f3: /* 0 011 110 011 */
	case 0x0f4: /* 0 011 110 100 */
	case 0x0f5: /* 0 011 110 101 */
	case 0x0f6: /* 0 011 110 110 */
	case 0x0f7: /* 0 011 110 111 */
	case 0x0f8: /* 0 011 111 000 */
	case 0x0f9: /* 0 011 111 001 */
	case 0x0fa: /* 0 011 111 010 */
	case 0x0fb: /* 0 011 111 011 */
	case 0x0fc: /* 0 011 111 100 */
	case 0x0fd: /* 0 011 111 101 */
	case 0x0fe: /* 0 011 111 110 */
	case 0x0ff: /* 0 011 111 111 */
		sprintf(buffer, "ADDR R%01d,R%01d",(op&0x38)>>3,op&7);
		break;
	case 0x100: /* 0 100 000 000 */
	case 0x101: /* 0 100 000 001 */
	case 0x102: /* 0 100 000 010 */
	case 0x103: /* 0 100 000 011 */
	case 0x104: /* 0 100 000 100 */
	case 0x105: /* 0 100 000 101 */
	case 0x106: /* 0 100 000 110 */
	case 0x107: /* 0 100 000 111 */
	case 0x108: /* 0 100 001 000 */
	case 0x109: /* 0 100 001 001 */
	case 0x10a: /* 0 100 001 010 */
	case 0x10b: /* 0 100 001 011 */
	case 0x10c: /* 0 100 001 100 */
	case 0x10d: /* 0 100 001 101 */
	case 0x10e: /* 0 100 001 110 */
	case 0x10f: /* 0 100 001 111 */
	case 0x110: /* 0 100 010 000 */
	case 0x111: /* 0 100 010 001 */
	case 0x112: /* 0 100 010 010 */
	case 0x113: /* 0 100 010 011 */
	case 0x114: /* 0 100 010 100 */
	case 0x115: /* 0 100 010 101 */
	case 0x116: /* 0 100 010 110 */
	case 0x117: /* 0 100 010 111 */
	case 0x118: /* 0 100 011 000 */
	case 0x119: /* 0 100 011 001 */
	case 0x11a: /* 0 100 011 010 */
	case 0x11b: /* 0 100 011 011 */
	case 0x11c: /* 0 100 011 100 */
	case 0x11d: /* 0 100 011 101 */
	case 0x11e: /* 0 100 011 110 */
	case 0x11f: /* 0 100 011 111 */
	case 0x120: /* 0 100 100 000 */
	case 0x121: /* 0 100 100 001 */
	case 0x122: /* 0 100 100 010 */
	case 0x123: /* 0 100 100 011 */
	case 0x124: /* 0 100 100 100 */
	case 0x125: /* 0 100 100 101 */
	case 0x126: /* 0 100 100 110 */
	case 0x127: /* 0 100 100 111 */
	case 0x128: /* 0 100 101 000 */
	case 0x129: /* 0 100 101 001 */
	case 0x12a: /* 0 100 101 010 */
	case 0x12b: /* 0 100 101 011 */
	case 0x12c: /* 0 100 101 100 */
	case 0x12d: /* 0 100 101 101 */
	case 0x12e: /* 0 100 101 110 */
	case 0x12f: /* 0 100 101 111 */
	case 0x130: /* 0 100 110 000 */
	case 0x131: /* 0 100 110 001 */
	case 0x132: /* 0 100 110 010 */
	case 0x133: /* 0 100 110 011 */
	case 0x134: /* 0 100 110 100 */
	case 0x135: /* 0 100 110 101 */
	case 0x136: /* 0 100 110 110 */
	case 0x137: /* 0 100 110 111 */
	case 0x138: /* 0 100 111 000 */
	case 0x139: /* 0 100 111 001 */
	case 0x13a: /* 0 100 111 010 */
	case 0x13b: /* 0 100 111 011 */
	case 0x13c: /* 0 100 111 100 */
	case 0x13d: /* 0 100 111 101 */
	case 0x13e: /* 0 100 111 110 */
	case 0x13f: /* 0 100 111 111 */
		sprintf(buffer, "SUBR R%01d,R%01d",(op&0x38)>>3,op&7);
		break;
	case 0x140: /* 0 101 000 000 */
	case 0x141: /* 0 101 000 001 */
	case 0x142: /* 0 101 000 010 */
	case 0x143: /* 0 101 000 011 */
	case 0x144: /* 0 101 000 100 */
	case 0x145: /* 0 101 000 101 */
	case 0x146: /* 0 101 000 110 */
	case 0x147: /* 0 101 000 111 */
	case 0x148: /* 0 101 001 000 */
	case 0x149: /* 0 101 001 001 */
	case 0x14a: /* 0 101 001 010 */
	case 0x14b: /* 0 101 001 011 */
	case 0x14c: /* 0 101 001 100 */
	case 0x14d: /* 0 101 001 101 */
	case 0x14e: /* 0 101 001 110 */
	case 0x14f: /* 0 101 001 111 */
	case 0x150: /* 0 101 010 000 */
	case 0x151: /* 0 101 010 001 */
	case 0x152: /* 0 101 010 010 */
	case 0x153: /* 0 101 010 011 */
	case 0x154: /* 0 101 010 100 */
	case 0x155: /* 0 101 010 101 */
	case 0x156: /* 0 101 010 110 */
	case 0x157: /* 0 101 010 111 */
	case 0x158: /* 0 101 011 000 */
	case 0x159: /* 0 101 011 001 */
	case 0x15a: /* 0 101 011 010 */
	case 0x15b: /* 0 101 011 011 */
	case 0x15c: /* 0 101 011 100 */
	case 0x15d: /* 0 101 011 101 */
	case 0x15e: /* 0 101 011 110 */
	case 0x15f: /* 0 101 011 111 */
	case 0x160: /* 0 101 100 000 */
	case 0x161: /* 0 101 100 001 */
	case 0x162: /* 0 101 100 010 */
	case 0x163: /* 0 101 100 011 */
	case 0x164: /* 0 101 100 100 */
	case 0x165: /* 0 101 100 101 */
	case 0x166: /* 0 101 100 110 */
	case 0x167: /* 0 101 100 111 */
	case 0x168: /* 0 101 101 000 */
	case 0x169: /* 0 101 101 001 */
	case 0x16a: /* 0 101 101 010 */
	case 0x16b: /* 0 101 101 011 */
	case 0x16c: /* 0 101 101 100 */
	case 0x16d: /* 0 101 101 101 */
	case 0x16e: /* 0 101 101 110 */
	case 0x16f: /* 0 101 101 111 */
	case 0x170: /* 0 101 110 000 */
	case 0x171: /* 0 101 110 001 */
	case 0x172: /* 0 101 110 010 */
	case 0x173: /* 0 101 110 011 */
	case 0x174: /* 0 101 110 100 */
	case 0x175: /* 0 101 110 101 */
	case 0x176: /* 0 101 110 110 */
	case 0x177: /* 0 101 110 111 */
	case 0x178: /* 0 101 111 000 */
	case 0x179: /* 0 101 111 001 */
	case 0x17a: /* 0 101 111 010 */
	case 0x17b: /* 0 101 111 011 */
	case 0x17c: /* 0 101 111 100 */
	case 0x17d: /* 0 101 111 101 */
	case 0x17e: /* 0 101 111 110 */
	case 0x17f: /* 0 101 111 111 */
		sprintf(buffer, "CMPR R%01d,R%01d",(op&0x38)>>3,op&7);
		break;
	case 0x180: /* 0 110 000 000 */
	case 0x181: /* 0 110 000 001 */
	case 0x182: /* 0 110 000 010 */
	case 0x183: /* 0 110 000 011 */
	case 0x184: /* 0 110 000 100 */
	case 0x185: /* 0 110 000 101 */
	case 0x186: /* 0 110 000 110 */
	case 0x187: /* 0 110 000 111 */
	case 0x188: /* 0 110 001 000 */
	case 0x189: /* 0 110 001 001 */
	case 0x18a: /* 0 110 001 010 */
	case 0x18b: /* 0 110 001 011 */
	case 0x18c: /* 0 110 001 100 */
	case 0x18d: /* 0 110 001 101 */
	case 0x18e: /* 0 110 001 110 */
	case 0x18f: /* 0 110 001 111 */
	case 0x190: /* 0 110 010 000 */
	case 0x191: /* 0 110 010 001 */
	case 0x192: /* 0 110 010 010 */
	case 0x193: /* 0 110 010 011 */
	case 0x194: /* 0 110 010 100 */
	case 0x195: /* 0 110 010 101 */
	case 0x196: /* 0 110 010 110 */
	case 0x197: /* 0 110 010 111 */
	case 0x198: /* 0 110 011 000 */
	case 0x199: /* 0 110 011 001 */
	case 0x19a: /* 0 110 011 010 */
	case 0x19b: /* 0 110 011 011 */
	case 0x19c: /* 0 110 011 100 */
	case 0x19d: /* 0 110 011 101 */
	case 0x19e: /* 0 110 011 110 */
	case 0x19f: /* 0 110 011 111 */
	case 0x1a0: /* 0 110 100 000 */
	case 0x1a1: /* 0 110 100 001 */
	case 0x1a2: /* 0 110 100 010 */
	case 0x1a3: /* 0 110 100 011 */
	case 0x1a4: /* 0 110 100 100 */
	case 0x1a5: /* 0 110 100 101 */
	case 0x1a6: /* 0 110 100 110 */
	case 0x1a7: /* 0 110 100 111 */
	case 0x1a8: /* 0 110 101 000 */
	case 0x1a9: /* 0 110 101 001 */
	case 0x1aa: /* 0 110 101 010 */
	case 0x1ab: /* 0 110 101 011 */
	case 0x1ac: /* 0 110 101 100 */
	case 0x1ad: /* 0 110 101 101 */
	case 0x1ae: /* 0 110 101 110 */
	case 0x1af: /* 0 110 101 111 */
	case 0x1b0: /* 0 110 110 000 */
	case 0x1b1: /* 0 110 110 001 */
	case 0x1b2: /* 0 110 110 010 */
	case 0x1b3: /* 0 110 110 011 */
	case 0x1b4: /* 0 110 110 100 */
	case 0x1b5: /* 0 110 110 101 */
	case 0x1b6: /* 0 110 110 110 */
	case 0x1b7: /* 0 110 110 111 */
	case 0x1b8: /* 0 110 111 000 */
	case 0x1b9: /* 0 110 111 001 */
	case 0x1ba: /* 0 110 111 010 */
	case 0x1bb: /* 0 110 111 011 */
	case 0x1bc: /* 0 110 111 100 */
	case 0x1bd: /* 0 110 111 101 */
	case 0x1be: /* 0 110 111 110 */
	case 0x1bf: /* 0 110 111 111 */
		sprintf(buffer, "ANDR R%01d,R%01d",(op&0x38)>>3,op&7);
		break;
	case 0x1c0: /* 0 111 000 000 */
	case 0x1c9: /* 0 111 001 001 */
	case 0x1d2: /* 0 111 010 010 */
	case 0x1db: /* 0 111 011 011 */
	case 0x1e4: /* 0 111 100 100 */
	case 0x1ed: /* 0 111 101 101 */
	case 0x1f6: /* 0 111 110 110 */
	case 0x1ff: /* 0 111 111 111 */
		sprintf(buffer, "CLRR R%01d",op&7);
		break;
	case 0x1c1: /* 0 111 000 001 */
	case 0x1c2: /* 0 111 000 010 */
	case 0x1c3: /* 0 111 000 011 */
	case 0x1c4: /* 0 111 000 100 */
	case 0x1c5: /* 0 111 000 101 */
	case 0x1c6: /* 0 111 000 110 */
	case 0x1c7: /* 0 111 000 111 */
	case 0x1c8: /* 0 111 001 000 */
	case 0x1ca: /* 0 111 001 010 */
	case 0x1cb: /* 0 111 001 011 */
	case 0x1cc: /* 0 111 001 100 */
	case 0x1cd: /* 0 111 001 101 */
	case 0x1ce: /* 0 111 001 110 */
	case 0x1cf: /* 0 111 001 111 */
	case 0x1d0: /* 0 111 010 000 */
	case 0x1d1: /* 0 111 010 001 */
	case 0x1d3: /* 0 111 010 011 */
	case 0x1d4: /* 0 111 010 100 */
	case 0x1d5: /* 0 111 010 101 */
	case 0x1d6: /* 0 111 010 110 */
	case 0x1d7: /* 0 111 010 111 */
	case 0x1d8: /* 0 111 011 000 */
	case 0x1d9: /* 0 111 011 001 */
	case 0x1da: /* 0 111 011 010 */
	case 0x1dc: /* 0 111 011 100 */
	case 0x1dd: /* 0 111 011 101 */
	case 0x1de: /* 0 111 011 110 */
	case 0x1df: /* 0 111 011 111 */
	case 0x1e0: /* 0 111 100 000 */
	case 0x1e1: /* 0 111 100 001 */
	case 0x1e2: /* 0 111 100 010 */
	case 0x1e3: /* 0 111 100 011 */
	case 0x1e5: /* 0 111 100 101 */
	case 0x1e6: /* 0 111 100 110 */
	case 0x1e7: /* 0 111 100 111 */
	case 0x1e8: /* 0 111 101 000 */
	case 0x1e9: /* 0 111 101 001 */
	case 0x1ea: /* 0 111 101 010 */
	case 0x1eb: /* 0 111 101 011 */
	case 0x1ec: /* 0 111 101 100 */
	case 0x1ee: /* 0 111 101 110 */
	case 0x1ef: /* 0 111 101 111 */
	case 0x1f0: /* 0 111 110 000 */
	case 0x1f1: /* 0 111 110 001 */
	case 0x1f2: /* 0 111 110 010 */
	case 0x1f3: /* 0 111 110 011 */
	case 0x1f4: /* 0 111 110 100 */
	case 0x1f5: /* 0 111 110 101 */
	case 0x1f7: /* 0 111 110 111 */
	case 0x1f8: /* 0 111 111 000 */
	case 0x1f9: /* 0 111 111 001 */
	case 0x1fa: /* 0 111 111 010 */
	case 0x1fb: /* 0 111 111 011 */
	case 0x1fc: /* 0 111 111 100 */
	case 0x1fd: /* 0 111 111 101 */
	case 0x1fe: /* 0 111 111 110 */
		sprintf(buffer, "XORR R%01d,R%01d",(op&0x38)>>3,op&7);
		break;
	case 0x200: /* 1 000 000 000 */
		size += 1;
		ea = oprom16[1];
		sprintf(buffer, "B    %04X",pc+2+ea);
		break;
	case 0x201: /* 1 000 000 001 */
		size += 1;
		ea = oprom16[1];
		sprintf(buffer, "BC   %04X",pc+2+ea);
		break;
	case 0x202: /* 1 000 000 010 */
		size += 1;
		ea = oprom16[1];
		sprintf(buffer, "BOV  %04X",pc+2+ea);
		break;
	case 0x203: /* 1 000 000 011 */
		size += 1;
		ea = oprom16[1];
		sprintf(buffer, "BPL  %04X",pc+2+ea);
		break;
	case 0x204: /* 1 000 000 100 */
		size += 1;
		ea = oprom16[1];
		sprintf(buffer, "BZE  %04X",pc+2+ea);
		break;
	case 0x205: /* 1 000 000 101 */
		size += 1;
		ea = oprom16[1];
		sprintf(buffer, "BLT  %04X",pc+2+ea);
		break;
	case 0x206: /* 1 000 000 110 */
		size += 1;
		ea = oprom16[1];
		sprintf(buffer, "BLE  %04X",pc+2+ea);
		break;
	case 0x207: /* 1 000 000 111 */
		size += 1;
		ea = oprom16[1];
		sprintf(buffer, "BUSC %04X",pc+2+ea);
		break;
	case 0x208: /* 1 000 001 000 */
		size += 1;
		ea = oprom16[1];
		sprintf(buffer, "NOPP %04X",pc+2+ea);
		break;
	case 0x209: /* 1 000 001 001 */
		size += 1;
		ea = oprom16[1];
		sprintf(buffer, "BNC  %04X",pc+2+ea);
		break;
	case 0x20a: /* 1 000 001 010 */
		size += 1;
		ea = oprom16[1];
		sprintf(buffer, "BNOV %04X",pc+2+ea);
		break;
	case 0x20b: /* 1 000 001 011 */
		size += 1;
		ea = oprom16[1];
		sprintf(buffer, "BMI  %04X",pc+2+ea);
		break;
	case 0x20c: /* 1 000 001 100 */
		size += 1;
		ea = oprom16[1];
		sprintf(buffer, "BNZE %04X",pc+2+ea);
		break;
	case 0x20d: /* 1 000 001 101 */
		size += 1;
		ea = oprom16[1];
		sprintf(buffer, "BGE  %04X",pc+2+ea);
		break;
	case 0x20e: /* 1 000 001 110 */
		size += 1;
		ea = oprom16[1];
		sprintf(buffer, "BGT  %04X",pc+2+ea);
		break;
	case 0x20f: /* 1 000 001 111 */
		size += 1;
		ea = oprom16[1];
		sprintf(buffer, "BESC %04X",pc+2+ea);
		break;
	case 0x210: /* 1 000 010 000 */
	case 0x211: /* 1 000 010 001 */
	case 0x212: /* 1 000 010 010 */
	case 0x213: /* 1 000 010 011 */
	case 0x214: /* 1 000 010 100 */
	case 0x215: /* 1 000 010 101 */
	case 0x216: /* 1 000 010 110 */
	case 0x217: /* 1 000 010 111 */
	case 0x218: /* 1 000 011 000 */
	case 0x219: /* 1 000 011 001 */
	case 0x21a: /* 1 000 011 010 */
	case 0x21b: /* 1 000 011 011 */
	case 0x21c: /* 1 000 011 100 */
	case 0x21d: /* 1 000 011 101 */
	case 0x21e: /* 1 000 011 110 */
	case 0x21f: /* 1 000 011 111 */
		size += 1;
		ea = oprom16[1];
		sprintf(buffer, "BEXT %04X,%01X",pc+2+ea,op&0x0f);
		break;
	case 0x220: /* 1 000 100 000 */
		size += 1;
		ea = oprom16[1];
		sprintf(buffer, "B    %04X",pc+1-ea);
		break;
	case 0x221: /* 1 000 100 001 */
		size += 1;
		ea = oprom16[1];
		sprintf(buffer, "BC   %04X",pc+1-ea);
		break;
	case 0x222: /* 1 000 100 010 */
		size += 1;
		ea = oprom16[1];
		sprintf(buffer, "BOV  %04X",pc+1-ea);
		break;
	case 0x223: /* 1 000 100 011 */
		size += 1;
		ea = oprom16[1];
		sprintf(buffer, "BPL  %04X",pc+1-ea);
		break;
	case 0x224: /* 1 000 100 100 */
		size += 1;
		ea = oprom16[1];
		sprintf(buffer, "BZE  %04X",pc+1-ea);
		break;
	case 0x225: /* 1 000 100 101 */
		size += 1;
		ea = oprom16[1];
		sprintf(buffer, "BLT  %04X",pc+1-ea);
		break;
	case 0x226: /* 1 000 100 110 */
		size += 1;
		ea = oprom16[1];
		sprintf(buffer, "BLE  %04X",pc+1-ea);
		break;
	case 0x227: /* 1 000 100 111 */
		size += 1;
		ea = oprom16[1];
		sprintf(buffer, "BUSC %04X",pc+1-ea);
		break;
	case 0x228: /* 1 000 101 000 */
		size += 1;
		ea = oprom16[1];
		sprintf(buffer, "NOPP %04X",pc+1-ea);
		break;
	case 0x229: /* 1 000 101 001 */
		size += 1;
		ea = oprom16[1];
		sprintf(buffer, "BNC  %04X",pc+1-ea);
		break;
	case 0x22a: /* 1 000 101 010 */
		size += 1;
		ea = oprom16[1];
		sprintf(buffer, "BNOV %04X",pc+1-ea);
		break;
	case 0x22b: /* 1 000 101 011 */
		size += 1;
		ea = oprom16[1];
		sprintf(buffer, "BMI  %04X",pc+1-ea);
		break;
	case 0x22c: /* 1 000 101 100 */
		size += 1;
		ea = oprom16[1];
		sprintf(buffer, "BNZE %04X",pc+1-ea);
		break;
	case 0x22d: /* 1 000 101 101 */
		size += 1;
		ea = oprom16[1];
		sprintf(buffer, "BGE  %04X",pc+1-ea);
		break;
	case 0x22e: /* 1 000 101 110 */
		size += 1;
		ea = oprom16[1];
		sprintf(buffer, "BGT  %04X",pc+1-ea);
		break;
	case 0x22f: /* 1 000 101 111 */
		size += 1;
		ea = oprom16[1];
		sprintf(buffer, "BESC %04X",pc+1-ea);
		break;
	case 0x230: /* 1 000 110 000 */
	case 0x231: /* 1 000 110 001 */
	case 0x232: /* 1 000 110 010 */
	case 0x233: /* 1 000 110 011 */
	case 0x234: /* 1 000 110 100 */
	case 0x235: /* 1 000 110 101 */
	case 0x236: /* 1 000 110 110 */
	case 0x237: /* 1 000 110 111 */
	case 0x238: /* 1 000 111 000 */
	case 0x239: /* 1 000 111 001 */
	case 0x23a: /* 1 000 111 010 */
	case 0x23b: /* 1 000 111 011 */
	case 0x23c: /* 1 000 111 100 */
	case 0x23d: /* 1 000 111 101 */
	case 0x23e: /* 1 000 111 110 */
	case 0x23f: /* 1 000 111 111 */
		size += 1;
		ea = oprom16[1];
		sprintf(buffer, "BEXT %04X,%01X",pc+1-ea,op&0x0f);
		break;
	case 0x240: /* 1 001 000 000 */
	case 0x241: /* 1 001 000 001 */
	case 0x242: /* 1 001 000 010 */
	case 0x243: /* 1 001 000 011 */
	case 0x244: /* 1 001 000 100 */
	case 0x245: /* 1 001 000 101 */
	case 0x246: /* 1 001 000 110 */
	case 0x247: /* 1 001 000 111 */
		size += 1;
		ea = oprom16[1];
		sprintf(buffer, "MVO  R%01d,(%04X)",op&0x7,ea);
		break;
	case 0x248: /* 1 001 001 000 */
	case 0x249: /* 1 001 001 001 */
	case 0x24a: /* 1 001 001 010 */
	case 0x24b: /* 1 001 001 011 */
	case 0x24c: /* 1 001 001 100 */
	case 0x24d: /* 1 001 001 101 */
	case 0x24e: /* 1 001 001 110 */
	case 0x24f: /* 1 001 001 111 */
	case 0x250: /* 1 001 010 000 */
	case 0x251: /* 1 001 010 001 */
	case 0x252: /* 1 001 010 010 */
	case 0x253: /* 1 001 010 011 */
	case 0x254: /* 1 001 010 100 */
	case 0x255: /* 1 001 010 101 */
	case 0x256: /* 1 001 010 110 */
	case 0x257: /* 1 001 010 111 */
	case 0x258: /* 1 001 011 000 */
	case 0x259: /* 1 001 011 001 */
	case 0x25a: /* 1 001 011 010 */
	case 0x25b: /* 1 001 011 011 */
	case 0x25c: /* 1 001 011 100 */
	case 0x25d: /* 1 001 011 101 */
	case 0x25e: /* 1 001 011 110 */
	case 0x25f: /* 1 001 011 111 */
	case 0x260: /* 1 001 100 000 */
	case 0x261: /* 1 001 100 001 */
	case 0x262: /* 1 001 100 010 */
	case 0x263: /* 1 001 100 011 */
	case 0x264: /* 1 001 100 100 */
	case 0x265: /* 1 001 100 101 */
	case 0x266: /* 1 001 100 110 */
	case 0x267: /* 1 001 100 111 */
	case 0x268: /* 1 001 101 000 */
	case 0x269: /* 1 001 101 001 */
	case 0x26a: /* 1 001 101 010 */
	case 0x26b: /* 1 001 101 011 */
	case 0x26c: /* 1 001 101 100 */
	case 0x26d: /* 1 001 101 101 */
	case 0x26e: /* 1 001 101 110 */
	case 0x26f: /* 1 001 101 111 */
		sprintf(buffer, "MVO@ R%01d,(R%01d)",op&0x7,(op&0x38)>>3);
		break;
	case 0x270: /* 1 001 110 000 */
	case 0x271: /* 1 001 110 001 */
	case 0x272: /* 1 001 110 010 */
	case 0x273: /* 1 001 110 011 */
	case 0x274: /* 1 001 110 100 */
	case 0x275: /* 1 001 110 101 */
	case 0x276: /* 1 001 110 110 */
	case 0x277: /* 1 001 110 111 */
		sprintf(buffer, "PSHR R%01d",op&0x7);
		break;
	case 0x278: /* 1 001 111 000 */
	case 0x279: /* 1 001 111 001 */
	case 0x27a: /* 1 001 111 010 */
	case 0x27b: /* 1 001 111 011 */
	case 0x27c: /* 1 001 111 100 */
	case 0x27d: /* 1 001 111 101 */
	case 0x27e: /* 1 001 111 110 */
	case 0x27f: /* 1 001 111 111 */
		size += 1;
		sprintf(buffer, "MVOI R%01d,(%04X)",op&0x7,pc+1);
		break;
	case 0x280: /* 1 010 000 000 */
	case 0x281: /* 1 010 000 001 */
	case 0x282: /* 1 010 000 010 */
	case 0x283: /* 1 010 000 011 */
	case 0x284: /* 1 010 000 100 */
	case 0x285: /* 1 010 000 101 */
	case 0x286: /* 1 010 000 110 */
	case 0x287: /* 1 010 000 111 */
		size += 1;
		ea = oprom16[1];
		sprintf(buffer, "MVI  (%04X),R%01d",ea,op&0x7);
		break;
	case 0x288: /* 1 010 001 000 */
	case 0x289: /* 1 010 001 001 */
	case 0x28a: /* 1 010 001 010 */
	case 0x28b: /* 1 010 001 011 */
	case 0x28c: /* 1 010 001 100 */
	case 0x28d: /* 1 010 001 101 */
	case 0x28e: /* 1 010 001 110 */
	case 0x28f: /* 1 010 001 111 */
	case 0x290: /* 1 010 010 000 */
	case 0x291: /* 1 010 010 001 */
	case 0x292: /* 1 010 010 010 */
	case 0x293: /* 1 010 010 011 */
	case 0x294: /* 1 010 010 100 */
	case 0x295: /* 1 010 010 101 */
	case 0x296: /* 1 010 010 110 */
	case 0x297: /* 1 010 010 111 */
	case 0x298: /* 1 010 011 000 */
	case 0x299: /* 1 010 011 001 */
	case 0x29a: /* 1 010 011 010 */
	case 0x29b: /* 1 010 011 011 */
	case 0x29c: /* 1 010 011 100 */
	case 0x29d: /* 1 010 011 101 */
	case 0x29e: /* 1 010 011 110 */
	case 0x29f: /* 1 010 011 111 */
	case 0x2a0: /* 1 010 100 000 */
	case 0x2a1: /* 1 010 100 001 */
	case 0x2a2: /* 1 010 100 010 */
	case 0x2a3: /* 1 010 100 011 */
	case 0x2a4: /* 1 010 100 100 */
	case 0x2a5: /* 1 010 100 101 */
	case 0x2a6: /* 1 010 100 110 */
	case 0x2a7: /* 1 010 100 111 */
	case 0x2a8: /* 1 010 101 000 */
	case 0x2a9: /* 1 010 101 001 */
	case 0x2aa: /* 1 010 101 010 */
	case 0x2ab: /* 1 010 101 011 */
	case 0x2ac: /* 1 010 101 100 */
	case 0x2ad: /* 1 010 101 101 */
	case 0x2ae: /* 1 010 101 110 */
	case 0x2af: /* 1 010 101 111 */
		sprintf(buffer, "MVI@ (R%01d),R%01d",(op&0x38)>>3,op&0x7);
		break;
	case 0x2b0: /* 1 010 110 000 */
	case 0x2b1: /* 1 010 110 001 */
	case 0x2b2: /* 1 010 110 010 */
	case 0x2b3: /* 1 010 110 011 */
	case 0x2b4: /* 1 010 110 100 */
	case 0x2b5: /* 1 010 110 101 */
	case 0x2b6: /* 1 010 110 110 */
	case 0x2b7: /* 1 010 110 111 */
		sprintf(buffer, "PULR R%01d",op&0x7);
		break;
	case 0x2b8: /* 1 010 111 000 */
	case 0x2b9: /* 1 010 111 001 */
	case 0x2ba: /* 1 010 111 010 */
	case 0x2bb: /* 1 010 111 011 */
	case 0x2bc: /* 1 010 111 100 */
	case 0x2bd: /* 1 010 111 101 */
	case 0x2be: /* 1 010 111 110 */
	case 0x2bf: /* 1 010 111 111 */
		size += 1;
		ea = oprom16[1];
		sprintf(buffer, "MVII #%04X,R%01d",ea,op&0x7);
		break;
	case 0x2c0: /* 1 011 010 000 */
	case 0x2c1: /* 1 011 010 001 */
	case 0x2c2: /* 1 011 010 010 */
	case 0x2c3: /* 1 011 010 011 */
	case 0x2c4: /* 1 011 010 100 */
	case 0x2c5: /* 1 011 010 101 */
	case 0x2c6: /* 1 011 010 110 */
	case 0x2c7: /* 1 011 010 111 */
		size += 1;
		ea = oprom16[1];
		sprintf(buffer, "ADD  (%04X),R%01d",ea,op&0x7);
		break;
	case 0x2c8: /* 1 011 001 000 */
	case 0x2c9: /* 1 011 001 001 */
	case 0x2ca: /* 1 011 001 010 */
	case 0x2cb: /* 1 011 001 011 */
	case 0x2cc: /* 1 011 001 100 */
	case 0x2cd: /* 1 011 001 101 */
	case 0x2ce: /* 1 011 001 110 */
	case 0x2cf: /* 1 011 001 111 */
	case 0x2d0: /* 1 011 010 000 */
	case 0x2d1: /* 1 011 010 001 */
	case 0x2d2: /* 1 011 010 010 */
	case 0x2d3: /* 1 011 010 011 */
	case 0x2d4: /* 1 011 010 100 */
	case 0x2d5: /* 1 011 010 101 */
	case 0x2d6: /* 1 011 010 110 */
	case 0x2d7: /* 1 011 010 111 */
	case 0x2d8: /* 1 011 011 000 */
	case 0x2d9: /* 1 011 011 001 */
	case 0x2da: /* 1 011 011 010 */
	case 0x2db: /* 1 011 011 011 */
	case 0x2dc: /* 1 011 011 100 */
	case 0x2dd: /* 1 011 011 101 */
	case 0x2de: /* 1 011 011 110 */
	case 0x2df: /* 1 011 011 111 */
	case 0x2e0: /* 1 011 100 000 */
	case 0x2e1: /* 1 011 100 001 */
	case 0x2e2: /* 1 011 100 010 */
	case 0x2e3: /* 1 011 100 011 */
	case 0x2e4: /* 1 011 100 100 */
	case 0x2e5: /* 1 011 100 101 */
	case 0x2e6: /* 1 011 100 110 */
	case 0x2e7: /* 1 011 100 111 */
	case 0x2e8: /* 1 011 101 000 */
	case 0x2e9: /* 1 011 101 001 */
	case 0x2ea: /* 1 011 101 010 */
	case 0x2eb: /* 1 011 101 011 */
	case 0x2ec: /* 1 011 101 100 */
	case 0x2ed: /* 1 011 101 101 */
	case 0x2ee: /* 1 011 101 110 */
	case 0x2ef: /* 1 011 101 111 */
	case 0x2f0: /* 1 011 110 000 */
	case 0x2f1: /* 1 011 110 001 */
	case 0x2f2: /* 1 011 110 010 */
	case 0x2f3: /* 1 011 110 011 */
	case 0x2f4: /* 1 011 110 100 */
	case 0x2f5: /* 1 011 110 101 */
	case 0x2f6: /* 1 011 110 110 */
	case 0x2f7: /* 1 011 110 111 */
		sprintf(buffer, "ADD@ (R%01d),R%01d",(op&0x38)>>3,op&0x7);
		break;
	case 0x2f8: /* 1 011 111 000 */
	case 0x2f9: /* 1 011 111 001 */
	case 0x2fa: /* 1 011 111 010 */
	case 0x2fb: /* 1 011 111 011 */
	case 0x2fc: /* 1 011 111 100 */
	case 0x2fd: /* 1 011 111 101 */
	case 0x2fe: /* 1 011 111 110 */
	case 0x2ff: /* 1 011 111 111 */
		size += 1;
		ea = oprom16[1];
		sprintf(buffer, "ADDI #%04X,R%01d",ea,op&0x7);
		break;
	case 0x300: /* 1 100 000 000 */
	case 0x301: /* 1 100 000 001 */
	case 0x302: /* 1 100 000 010 */
	case 0x303: /* 1 100 000 011 */
	case 0x304: /* 1 100 000 100 */
	case 0x305: /* 1 100 000 101 */
	case 0x306: /* 1 100 000 110 */
	case 0x307: /* 1 100 000 111 */
		size += 1;
		ea = oprom16[1];
		sprintf(buffer, "SUB  (%04X),R%01d",ea,op&0x7);
		break;
	case 0x308: /* 1 100 001 000 */
	case 0x309: /* 1 100 001 001 */
	case 0x30a: /* 1 100 001 010 */
	case 0x30b: /* 1 100 001 011 */
	case 0x30c: /* 1 100 001 100 */
	case 0x30d: /* 1 100 001 101 */
	case 0x30e: /* 1 100 001 110 */
	case 0x30f: /* 1 100 001 111 */
	case 0x310: /* 1 100 010 000 */
	case 0x311: /* 1 100 010 001 */
	case 0x312: /* 1 100 010 010 */
	case 0x313: /* 1 100 010 011 */
	case 0x314: /* 1 100 010 100 */
	case 0x315: /* 1 100 010 101 */
	case 0x316: /* 1 100 010 110 */
	case 0x317: /* 1 100 010 111 */
	case 0x318: /* 1 100 011 000 */
	case 0x319: /* 1 100 011 001 */
	case 0x31a: /* 1 100 011 010 */
	case 0x31b: /* 1 100 011 011 */
	case 0x31c: /* 1 100 011 100 */
	case 0x31d: /* 1 100 011 101 */
	case 0x31e: /* 1 100 011 110 */
	case 0x31f: /* 1 100 011 111 */
	case 0x320: /* 1 100 100 000 */
	case 0x321: /* 1 100 100 001 */
	case 0x322: /* 1 100 100 010 */
	case 0x323: /* 1 100 100 011 */
	case 0x324: /* 1 100 100 100 */
	case 0x325: /* 1 100 100 101 */
	case 0x326: /* 1 100 100 110 */
	case 0x327: /* 1 100 100 111 */
	case 0x328: /* 1 100 101 000 */
	case 0x329: /* 1 100 101 001 */
	case 0x32a: /* 1 100 101 010 */
	case 0x32b: /* 1 100 101 011 */
	case 0x32c: /* 1 100 101 100 */
	case 0x32d: /* 1 100 101 101 */
	case 0x32e: /* 1 100 101 110 */
	case 0x32f: /* 1 100 101 111 */
	case 0x330: /* 1 100 110 000 */
	case 0x331: /* 1 100 110 001 */
	case 0x332: /* 1 100 110 010 */
	case 0x333: /* 1 100 110 011 */
	case 0x334: /* 1 100 110 100 */
	case 0x335: /* 1 100 110 101 */
	case 0x336: /* 1 100 110 110 */
	case 0x337: /* 1 100 110 111 */
		sprintf(buffer, "SUB@ (R%01d),R%01d",(op&0x38)>>3,op&0x7);
		break;
	case 0x338: /* 1 100 111 000 */
	case 0x339: /* 1 100 111 001 */
	case 0x33a: /* 1 100 111 010 */
	case 0x33b: /* 1 100 111 011 */
	case 0x33c: /* 1 100 111 100 */
	case 0x33d: /* 1 100 111 101 */
	case 0x33e: /* 1 100 111 110 */
	case 0x33f: /* 1 100 111 111 */
		size += 1;
		ea = oprom16[1];
		sprintf(buffer, "SUBI #%04X,R%01d",ea,op&0x7);
		break;
	case 0x340: /* 1 101 000 000 */
	case 0x341: /* 1 101 000 001 */
	case 0x342: /* 1 101 000 010 */
	case 0x343: /* 1 101 000 011 */
	case 0x344: /* 1 101 000 100 */
	case 0x345: /* 1 101 000 101 */
	case 0x346: /* 1 101 000 110 */
	case 0x347: /* 1 101 000 111 */
		size += 1;
		ea = oprom16[1];
		sprintf(buffer, "CMP  (%04X),R%01d",ea,op&0x7);
		break;
	case 0x348: /* 1 101 001 000 */
	case 0x349: /* 1 101 001 001 */
	case 0x34a: /* 1 101 001 010 */
	case 0x34b: /* 1 101 001 011 */
	case 0x34c: /* 1 101 001 100 */
	case 0x34d: /* 1 101 001 101 */
	case 0x34e: /* 1 101 001 110 */
	case 0x34f: /* 1 101 001 111 */
	case 0x350: /* 1 101 010 000 */
	case 0x351: /* 1 101 010 001 */
	case 0x352: /* 1 101 010 010 */
	case 0x353: /* 1 101 010 011 */
	case 0x354: /* 1 101 010 100 */
	case 0x355: /* 1 101 010 101 */
	case 0x356: /* 1 101 010 110 */
	case 0x357: /* 1 101 010 111 */
	case 0x358: /* 1 101 011 000 */
	case 0x359: /* 1 101 011 001 */
	case 0x35a: /* 1 101 011 010 */
	case 0x35b: /* 1 101 011 011 */
	case 0x35c: /* 1 101 011 100 */
	case 0x35d: /* 1 101 011 101 */
	case 0x35e: /* 1 101 011 110 */
	case 0x35f: /* 1 101 011 111 */
	case 0x360: /* 1 101 100 000 */
	case 0x361: /* 1 101 100 001 */
	case 0x362: /* 1 101 100 010 */
	case 0x363: /* 1 101 100 011 */
	case 0x364: /* 1 101 100 100 */
	case 0x365: /* 1 101 100 101 */
	case 0x366: /* 1 101 100 110 */
	case 0x367: /* 1 101 100 111 */
	case 0x368: /* 1 101 101 000 */
	case 0x369: /* 1 101 101 001 */
	case 0x36a: /* 1 101 101 010 */
	case 0x36b: /* 1 101 101 011 */
	case 0x36c: /* 1 101 101 100 */
	case 0x36d: /* 1 101 101 101 */
	case 0x36e: /* 1 101 101 110 */
	case 0x36f: /* 1 101 101 111 */
	case 0x370: /* 1 101 110 000 */
	case 0x371: /* 1 101 110 001 */
	case 0x372: /* 1 101 110 010 */
	case 0x373: /* 1 101 110 011 */
	case 0x374: /* 1 101 110 100 */
	case 0x375: /* 1 101 110 101 */
	case 0x376: /* 1 101 110 110 */
	case 0x377: /* 1 101 110 111 */
		sprintf(buffer, "CMP@ (R%01d),R%01d",(op&0x38)>>3,op&0x7);
		break;
	case 0x378: /* 1 101 111 000 */
	case 0x379: /* 1 101 111 001 */
	case 0x37a: /* 1 101 111 010 */
	case 0x37b: /* 1 101 111 011 */
	case 0x37c: /* 1 101 111 100 */
	case 0x37d: /* 1 101 111 101 */
	case 0x37e: /* 1 101 111 110 */
	case 0x37f: /* 1 101 111 111 */
		size += 1;
		ea = oprom16[1];
		sprintf(buffer, "CMPI #%04X,R%01d",ea,op&0x7);
		break;
	case 0x380: /* 1 110 000 000 */
	case 0x381: /* 1 110 000 001 */
	case 0x382: /* 1 110 000 010 */
	case 0x383: /* 1 110 000 011 */
	case 0x384: /* 1 110 000 100 */
	case 0x385: /* 1 110 000 101 */
	case 0x386: /* 1 110 000 110 */
	case 0x387: /* 1 110 000 111 */
		size += 1;
		ea = oprom16[1];
		sprintf(buffer, "AND  (%04X),R%01d",ea,op&0x7);
		break;
	case 0x388: /* 1 110 001 000 */
	case 0x389: /* 1 110 001 001 */
	case 0x38a: /* 1 110 001 010 */
	case 0x38b: /* 1 110 001 011 */
	case 0x38c: /* 1 110 001 100 */
	case 0x38d: /* 1 110 001 101 */
	case 0x38e: /* 1 110 001 110 */
	case 0x38f: /* 1 110 001 111 */
	case 0x390: /* 1 110 010 000 */
	case 0x391: /* 1 110 010 001 */
	case 0x392: /* 1 110 010 010 */
	case 0x393: /* 1 110 010 011 */
	case 0x394: /* 1 110 010 100 */
	case 0x395: /* 1 110 010 101 */
	case 0x396: /* 1 110 010 110 */
	case 0x397: /* 1 110 010 111 */
	case 0x398: /* 1 110 011 000 */
	case 0x399: /* 1 110 011 001 */
	case 0x39a: /* 1 110 011 010 */
	case 0x39b: /* 1 110 011 011 */
	case 0x39c: /* 1 110 011 100 */
	case 0x39d: /* 1 110 011 101 */
	case 0x39e: /* 1 110 011 110 */
	case 0x39f: /* 1 110 011 111 */
	case 0x3a0: /* 1 110 100 000 */
	case 0x3a1: /* 1 110 100 001 */
	case 0x3a2: /* 1 110 100 010 */
	case 0x3a3: /* 1 110 100 011 */
	case 0x3a4: /* 1 110 100 100 */
	case 0x3a5: /* 1 110 100 101 */
	case 0x3a6: /* 1 110 100 110 */
	case 0x3a7: /* 1 110 100 111 */
	case 0x3a8: /* 1 110 101 000 */
	case 0x3a9: /* 1 110 101 001 */
	case 0x3aa: /* 1 110 101 010 */
	case 0x3ab: /* 1 110 101 011 */
	case 0x3ac: /* 1 110 101 100 */
	case 0x3ad: /* 1 110 101 101 */
	case 0x3ae: /* 1 110 101 110 */
	case 0x3af: /* 1 110 101 111 */
	case 0x3b0: /* 1 110 110 000 */
	case 0x3b1: /* 1 110 110 001 */
	case 0x3b2: /* 1 110 110 010 */
	case 0x3b3: /* 1 110 110 011 */
	case 0x3b4: /* 1 110 110 100 */
	case 0x3b5: /* 1 110 110 101 */
	case 0x3b6: /* 1 110 110 110 */
	case 0x3b7: /* 1 110 110 111 */
		sprintf(buffer, "AND@ (R%01d),R%01d",(op&0x38)>>3,op&0x7);
		break;
	case 0x3b8: /* 1 110 111 000 */
	case 0x3b9: /* 1 110 111 001 */
	case 0x3ba: /* 1 110 111 010 */
	case 0x3bb: /* 1 110 111 011 */
	case 0x3bc: /* 1 110 111 100 */
	case 0x3bd: /* 1 110 111 101 */
	case 0x3be: /* 1 110 111 110 */
	case 0x3bf: /* 1 110 111 111 */
		size += 1;
		ea = oprom16[1];
		sprintf(buffer, "ANDI #%04X,R%01d",ea,op&0x7);
		break;
	case 0x3c0: /* 1 111 000 000 */
	case 0x3c1: /* 1 111 000 001 */
	case 0x3c2: /* 1 111 000 010 */
	case 0x3c3: /* 1 111 000 011 */
	case 0x3c4: /* 1 111 000 100 */
	case 0x3c5: /* 1 111 000 101 */
	case 0x3c6: /* 1 111 000 110 */
	case 0x3c7: /* 1 111 000 111 */
		size += 1;
		ea = oprom16[1];
		sprintf(buffer, "XOR  (%04X),R%01d",ea,op&0x7);
		break;
	case 0x3c8: /* 1 111 001 000 */
	case 0x3c9: /* 1 111 001 001 */
	case 0x3ca: /* 1 111 001 010 */
	case 0x3cb: /* 1 111 001 011 */
	case 0x3cc: /* 1 111 001 100 */
	case 0x3cd: /* 1 111 001 101 */
	case 0x3ce: /* 1 111 001 110 */
	case 0x3cf: /* 1 111 001 111 */
	case 0x3d0: /* 1 111 010 000 */
	case 0x3d1: /* 1 111 010 001 */
	case 0x3d2: /* 1 111 010 010 */
	case 0x3d3: /* 1 111 010 011 */
	case 0x3d4: /* 1 111 010 100 */
	case 0x3d5: /* 1 111 010 101 */
	case 0x3d6: /* 1 111 010 110 */
	case 0x3d7: /* 1 111 010 111 */
	case 0x3d8: /* 1 111 011 000 */
	case 0x3d9: /* 1 111 011 001 */
	case 0x3da: /* 1 111 011 010 */
	case 0x3db: /* 1 111 011 011 */
	case 0x3dc: /* 1 111 011 100 */
	case 0x3dd: /* 1 111 011 101 */
	case 0x3de: /* 1 111 011 110 */
	case 0x3df: /* 1 111 011 111 */
	case 0x3e0: /* 1 111 100 000 */
	case 0x3e1: /* 1 111 100 001 */
	case 0x3e2: /* 1 111 100 010 */
	case 0x3e3: /* 1 111 100 011 */
	case 0x3e4: /* 1 111 100 100 */
	case 0x3e5: /* 1 111 100 101 */
	case 0x3e6: /* 1 111 100 110 */
	case 0x3e7: /* 1 111 100 111 */
	case 0x3e8: /* 1 111 101 000 */
	case 0x3e9: /* 1 111 101 001 */
	case 0x3ea: /* 1 111 101 010 */
	case 0x3eb: /* 1 111 101 011 */
	case 0x3ec: /* 1 111 101 100 */
	case 0x3ed: /* 1 111 101 101 */
	case 0x3ee: /* 1 111 101 110 */
	case 0x3ef: /* 1 111 101 111 */
	case 0x3f0: /* 1 111 110 000 */
	case 0x3f1: /* 1 111 110 001 */
	case 0x3f2: /* 1 111 110 010 */
	case 0x3f3: /* 1 111 110 011 */
	case 0x3f4: /* 1 111 110 100 */
	case 0x3f5: /* 1 111 110 101 */
	case 0x3f6: /* 1 111 110 110 */
	case 0x3f7: /* 1 111 110 111 */
		sprintf(buffer, "XOR@ (R%01d),R%01d",(op&0x38)>>3,op&0x7);
		break;
	case 0x3f8: /* 1 111 111 000 */
	case 0x3f9: /* 1 111 111 001 */
	case 0x3fa: /* 1 111 111 010 */
	case 0x3fb: /* 1 111 111 011 */
	case 0x3fc: /* 1 111 111 100 */
	case 0x3fd: /* 1 111 111 101 */
	case 0x3fe: /* 1 111 111 110 */
	case 0x3ff: /* 1 111 111 111 */
		size += 1;
		ea = oprom16[1];
		sprintf(buffer, "XORI #%04X,R%01d",ea,op&0x7);
		break;
	default:
		sprintf(buffer, "????");
	}

	return size;
}
