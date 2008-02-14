/*******************************************************************************

    mb88dasm.c
    Core implementation for the portable Fujitsu MB88xx series MCU disassembler.

    Written by Ernesto Corvi

*******************************************************************************/

#include "mb88xx.h"


offs_t mb88_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram)
{
	unsigned startpc = pc;
	UINT8 op = oprom[pc++ - startpc];
	UINT8 arg = oprom[pc - startpc];

	switch( op )
	{
		case 0x00: sprintf( buffer, "nop" ); break;
		case 0x01: sprintf( buffer, "outO (PortO<=A)" ); break;
		case 0x02: sprintf( buffer, "outP" ); break;
		case 0x03: sprintf( buffer, "outR (PortR[Y]<=A)" ); break;
		case 0x04: sprintf( buffer, "tay (Y<=A)" ); break;
		case 0x05: sprintf( buffer, "tath (TH<=A)" ); break;
		case 0x06: sprintf( buffer, "tatl (TL<=A)" ); break;
		case 0x07: sprintf( buffer, "tas (SB<=A)" ); break;
		case 0x08: sprintf( buffer, "icy (Y++)" ); break;
		case 0x09: sprintf( buffer, "icm (M[X,Y]++)" ); break;
		case 0x0A: sprintf( buffer, "stic (M[X,Y]<=A; Y++)" ); break;
		case 0x0B: sprintf( buffer, "x (A<=>M[X,Y])" ); break;
		case 0x0C: sprintf( buffer, "rol" ); break;
		case 0x0D: sprintf( buffer, "load (A<=M[X,Y])" ); break;
		case 0x0E: sprintf( buffer, "adc (A<=A+M[X,Y]+cf)" ); break;
		case 0x0F: sprintf( buffer, "and (A<=A & M[X,Y])" ); break;
		case 0x10: sprintf( buffer, "daa (A<=A+6 if (A>9 | cf=1)" ); break;
		case 0x11: sprintf( buffer, "das (A<=A+10 if (A>9 | cf=1)" ); break;
		case 0x12: sprintf( buffer, "inK (A<=PortK)" ); break;
		case 0x13: sprintf( buffer, "inR (A<=PortR[Y])" ); break;
		case 0x14: sprintf( buffer, "tya (A<=Y)" ); break;
		case 0x15: sprintf( buffer, "ttha (A<=TH)" ); break;
		case 0x16: sprintf( buffer, "ttla (A<=TL)" ); break;
		case 0x17: sprintf( buffer, "tsa (A<=S)" ); break;
		case 0x18: sprintf( buffer, "dcy (Y--)" ); break;
		case 0x19: sprintf( buffer, "dcm (M[X,Y]--)" ); break;
		case 0x1A: sprintf( buffer, "stdc (M[X,Y]<=A; Y--)" ); break;
		case 0x1B: sprintf( buffer, "xx (A<=>X)" ); break;
		case 0x1C: sprintf( buffer, "ror" ); break;
		case 0x1D: sprintf( buffer, "store (M[X,Y]<=A)" ); break;
		case 0x1E: sprintf( buffer, "sbc (A<=M[X,Y]-A-cf)" ); break;
		case 0x1F: sprintf( buffer, "or (A<=A | M[X,Y])" ); break;
		case 0x20: sprintf( buffer, "setR (PortR bit[Y]<=1)" ); break;
		case 0x21: sprintf( buffer, "setc (cf<=1)" ); break;
		case 0x22: sprintf( buffer, "rstR (PortR bit[Y]<=0)" ); break;
		case 0x23: sprintf( buffer, "rstc (cf<=0)" ); break;
		case 0x24: sprintf( buffer, "tstR (st<=PortR bit[Y])" ); break;
		case 0x25: sprintf( buffer, "tsti (st<=IRQ Line)" ); break;
		case 0x26: sprintf( buffer, "tstv (st<=vf)" ); break;
		case 0x27: sprintf( buffer, "tsts (st<=sf)" ); break;
		case 0x28: sprintf( buffer, "tstc (st<=cf)" ); break;
		case 0x29: sprintf( buffer, "tstz (st<=zf)" ); break;
		case 0x2A: sprintf( buffer, "sts (M[X,Y]<=SB)" ); break;
		case 0x2B: sprintf( buffer, "ls (SB<=M[X,Y])" ); break;
		case 0x2C: sprintf( buffer, "rts" ); break;
		case 0x2D: sprintf( buffer, "neg (A=-A)" ); break;
		case 0x2E: sprintf( buffer, "c (A==M[X,Y])" ); break;
		case 0x2F: sprintf( buffer, "eor (A ^ M[X,Y])" ); break;
		case 0x30:
		case 0x31:
		case 0x32:
		case 0x33: sprintf( buffer, "sbit%d (M[X,Y] bit%d=1)", op&3, op&3 ); break;
		case 0x34:
		case 0x35:
		case 0x36:
		case 0x37: sprintf( buffer, "rbit%d (M[X,Y] bit%d=0)", op&3, op&3 ); break;
		case 0x38:
		case 0x39:
		case 0x3A:
		case 0x3B: sprintf( buffer, "tbit%d (M[X,Y] bit%d == 1)", op&3, op&3 ); break;
		case 0x3C: sprintf( buffer, "rti" ); break;
		case 0x3D: sprintf( buffer, "jpa #$%02x (jump always)", arg ); pc++; break;
		case 0x3E: sprintf( buffer, "en #$%02x (enable bits)", arg ); pc++; break;
		case 0x3F: sprintf( buffer, "dis #$%02x (disable bits)", arg ); pc++; break;
		case 0x40:
		case 0x41:
		case 0x42:
		case 0x43: sprintf( buffer, "setd%d (PortR bit%d<=1)", op&3, op&3 ); break;
		case 0x44:
		case 0x45:
		case 0x46:
		case 0x47: sprintf( buffer, "rstd%d (PortR bit%d<=0)", op&3, op&3 ); break;
		case 0x48:
		case 0x49:
		case 0x4A:
		case 0x4B: sprintf( buffer, "tstd%d (PortR bit%d == 1)", (op&3)+8, (op&3)+8 ); break;
		case 0x4C:
		case 0x4D:
		case 0x4E:
		case 0x4F: sprintf( buffer, "tba%d (A bit%d == 1)", op&3, op&3 ); break;
		case 0x50:
		case 0x51:
		case 0x52:
		case 0x53: sprintf( buffer, "xd%d (A<=>M[0,%d])", op&3, op&3 ); break;
		case 0x54:
		case 0x55:
		case 0x56:
		case 0x57: sprintf( buffer, "xyd%d (Y<=>M[0,%d])", (op&3)+4, (op&3)+4 ); break;
		case 0x58:
		case 0x59:
		case 0x5A:
		case 0x5B:
		case 0x5C:
		case 0x5D:
		case 0x5E:
		case 0x5F: sprintf( buffer, "lxi #$%1x (X<=$%1x)", op&7, op&7 ); break;
		case 0x60:
		case 0x61:
		case 0x62:
		case 0x63:
		case 0x64:
		case 0x65:
		case 0x66:
		case 0x67: sprintf( buffer, "call %02x%02x (call if st=1)", op&7, arg ); pc++; break;
		case 0x68:
		case 0x69:
		case 0x6A:
		case 0x6B:
		case 0x6C:
		case 0x6D:
		case 0x6E:
		case 0x6F: sprintf( buffer, "jpl %02x%02x (jump if st=1)", op&7, arg ); pc++; break;
		case 0x70:
		case 0x71:
		case 0x72:
		case 0x73:
		case 0x74:
		case 0x75:
		case 0x76:
		case 0x77:
		case 0x78:
		case 0x79:
		case 0x7A:
		case 0x7B:
		case 0x7C:
		case 0x7D:
		case 0x7E:
		case 0x7F: sprintf( buffer, "ai #$%1x (A<=A+$%1x)", op&0xf, op&0xf ); break;
		case 0x80:
		case 0x81:
		case 0x82:
		case 0x83:
		case 0x84:
		case 0x85:
		case 0x86:
		case 0x87:
		case 0x88:
		case 0x89:
		case 0x8A:
		case 0x8B:
		case 0x8C:
		case 0x8D:
		case 0x8E:
		case 0x8F: sprintf( buffer, "lyi #$%1x (Y<=$%1x)", op&0xf, op&0xf ); break;
		case 0x90:
		case 0x91:
		case 0x92:
		case 0x93:
		case 0x94:
		case 0x95:
		case 0x96:
		case 0x97:
		case 0x98:
		case 0x99:
		case 0x9A:
		case 0x9B:
		case 0x9C:
		case 0x9D:
		case 0x9E:
		case 0x9F: sprintf( buffer, "li #$%1x (A<=$%1x)", op&0xf, op&0xf ); break;
		case 0xA0:
		case 0xA1:
		case 0xA2:
		case 0xA3:
		case 0xA4:
		case 0xA5:
		case 0xA6:
		case 0xA7:
		case 0xA8:
		case 0xA9:
		case 0xAA:
		case 0xAB:
		case 0xAC:
		case 0xAD:
		case 0xAE:
		case 0xAF: sprintf( buffer, "cyi #$%1x (Y==$%1x)", op&0xf, op&0xf ); break;
		case 0xB0:
		case 0xB1:
		case 0xB2:
		case 0xB3:
		case 0xB4:
		case 0xB5:
		case 0xB6:
		case 0xB7:
		case 0xB8:
		case 0xB9:
		case 0xBA:
		case 0xBB:
		case 0xBC:
		case 0xBD:
		case 0xBE:
		case 0xBF: sprintf( buffer, "ci #$%1x (A==$%1x)", op&0xf, op&0xf ); break;

		default:
			/* C0-FF */
			sprintf( buffer, "jmp $%04x (jump if st=1)", (pc&(~0x3f))+ op-0xC0 );
		break;
	}

	return (pc - startpc) | DASMFLAG_SUPPORTED;
}
