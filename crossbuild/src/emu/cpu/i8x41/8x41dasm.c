#include "debugger.h"
#include "i8x41.h"

offs_t i8x41_dasm(char *dst, offs_t pc, const UINT8 *oprom, const UINT8 *opram)
{
	UINT32 flags = 0;
	unsigned PC = pc;
	UINT8 op;
	UINT8 arg;

	op = oprom[PC++ - pc];
	switch( op )
	{
	case 0x00: /* 1: 0000 0000 */
		sprintf(dst, "nop");
		break;
	case 0x01: /* 1: 0000 0001 */
		sprintf(dst, "ill");
		break;
	case 0x02: /* 1: 0000 0010 */
		sprintf(dst, "out   dbb,a");
		break;
	case 0x03: /* 2: 0000 0011 */
		sprintf(dst, "add   a,#$%02X", opram[PC++ - pc]);
		break;
	case 0x04: /* 2: aaa0 0100 */
	case 0x24: /* 2: aaa0 0100 */
	case 0x44: /* 2: aaa0 0100 */
	case 0x64: /* 2: aaa0 0100 */
	case 0x84: /* 2: aaa0 0100 */
	case 0xa4: /* 2: aaa0 0100 */
	case 0xc4: /* 2: aaa0 0100 */
	case 0xe4: /* 2: aaa0 0100 */
		sprintf(dst, "jmp   $%04X", ((op<<3) & 0x700) | opram[PC++ - pc]);
		break;
	case 0x05: /* 1: 0000 0101 */
		sprintf(dst, "en    i");
		break;
	case 0x06: /* 1: 0000 0110 */
		sprintf(dst, "ill");
		break;
	case 0x07: /* 1: 0000 0111 */
		sprintf(dst, "dec   a");
		break;
	case 0x08: /* 2: 0000 10pp */
	case 0x09: /* 2: 0000 10pp */
	case 0x0a: /* 2: 0000 10pp */
	case 0x0b: /* 2: 0000 10pp */
		sprintf(dst, "in    a,p%d", op&3);
		break;
	case 0x0c: /* 2: 0000 11pp */
	case 0x0d: /* 2: 0000 11pp */
	case 0x0e: /* 2: 0000 11pp */
	case 0x0f: /* 2: 0000 11pp */
		sprintf(dst, "movd  a,p%d", op&3);
		break;
	case 0x10: /* 1: 0001 000r */
	case 0x11: /* 1: 0001 000r */
		sprintf(dst, "inc   @r%d", op&1);
		break;
	case 0x12: /* 2: bbb1 0010 */
	case 0x32: /* 2: bbb1 0010 */
	case 0x52: /* 2: bbb1 0010 */
	case 0x72: /* 2: bbb1 0010 */
	case 0x92: /* 2: bbb1 0010 */
	case 0xb2: /* 2: bbb1 0010 */
	case 0xd2: /* 2: bbb1 0010 */
	case 0xf2: /* 2: bbb1 0010 */
		arg = opram[PC++ - pc];
		sprintf(dst, "jb%d   $%04X", op >> 5, (PC & 0x700) | arg);
		break;
	case 0x13: /* 2: 0001 0011 */
		sprintf(dst, "addc  $%02X", opram[PC++ - pc]);
		break;
	case 0x14: /* 2: aaa1 0100 */
	case 0x34: /* 2: aaa1 0100 */
	case 0x54: /* 2: aaa1 0100 */
	case 0x74: /* 2: aaa1 0100 */
	case 0x94: /* 2: aaa1 0100 */
	case 0xb4: /* 2: aaa1 0100 */
	case 0xd4: /* 2: aaa1 0100 */
	case 0xf4: /* 2: aaa1 0100 */
		sprintf(dst, "call  $%04X", ((op<<3) & 0x700) | opram[PC++ - pc]);
		flags = DASMFLAG_STEP_OVER;
		break;
	case 0x15: /* 1: 0001 0101 */
		sprintf(dst, "dis   i");
		break;
	case 0x16: /* 2: 0001 0110 */
		arg = opram[PC++ - pc];
		sprintf(dst, "jtf   $%04X", (PC & 0x700) | arg);
		break;
	case 0x17: /* 1: 0001 0111 */
		sprintf(dst, "inc   a");
		break;
	case 0x18: /* 1: 0001 1rrr */
	case 0x19: /* 1: 0001 1rrr */
	case 0x1a: /* 1: 0001 1rrr */
	case 0x1b: /* 1: 0001 1rrr */
	case 0x1c: /* 1: 0001 1rrr */
	case 0x1d: /* 1: 0001 1rrr */
	case 0x1e: /* 1: 0001 1rrr */
	case 0x1f: /* 1: 0001 1rrr */
		sprintf(dst, "inc   r%d", op&7);
		break;
	case 0x20: /* 1: 0010 000r */
	case 0x21: /* 1: 0010 000r */
		sprintf(dst, "xch   a,@r%d", op&1);
		break;
	case 0x22: /* 1: 0010 0010 */
		sprintf(dst, "in    a,ddb");
		break;
	case 0x23: /* 2: 0010 0011 */
		sprintf(dst, "mov   a,#$%02X", opram[PC++ - pc]);
		break;
	case 0x25: /* 1: 0010 0101 */
		sprintf(dst, "en    tcnti");
		break;
	case 0x26: /* 2: 0010 0110 */
		arg = opram[PC++ - pc];
		sprintf(dst, "jnt0  $%04X", (PC & 0x700) | arg);
		break;
	case 0x27: /* 1: 0010 0111 */
		sprintf(dst, "clr   a");
		break;
	case 0x28: /* 1: 0010 1rrr */
	case 0x29: /* 1: 0010 1rrr */
	case 0x2a: /* 1: 0010 1rrr */
	case 0x2b: /* 1: 0010 1rrr */
	case 0x2c: /* 1: 0010 1rrr */
	case 0x2d: /* 1: 0010 1rrr */
	case 0x2e: /* 1: 0010 1rrr */
	case 0x2f: /* 1: 0010 1rrr */
		sprintf(dst, "xch   a,r%d", op&7);
		break;
	case 0x30: /* 1: 0011 000r */
	case 0x31: /* 1: 0011 000r */
		sprintf(dst, "xchd  a,@r%d", op&1);
		break;
	case 0x33: /* 1: 0011 0101 */
		sprintf(dst, "ill");
		break;
	case 0x35: /* 1: 0000 0101 */
		sprintf(dst, "dis   tcnti");
		break;
	case 0x36: /* 2: 0011 0110 */
		arg = opram[PC++ - pc];
		sprintf(dst, "jt0   $%04X", (PC & 0x700) | arg);
		break;
	case 0x37: /* 1: 0011 0111 */
		sprintf(dst, "cpl   a");
		break;
	case 0x38: /* 2: 0011 10pp */
	case 0x39: /* 2: 0011 10pp */
	case 0x3a: /* 2: 0011 10pp */
	case 0x3b: /* 2: 0011 10pp */
		sprintf(dst, "out   p%d,a", op&3);
		break;
	case 0x3c: /* 2: 0011 11pp */
	case 0x3d: /* 2: 0011 11pp */
	case 0x3e: /* 2: 0011 11pp */
	case 0x3f: /* 2: 0011 11pp */
		sprintf(dst, "movd  p%d,a", op&7);
		break;
	case 0x40: /* 1: 0100 000r */
	case 0x41: /* 1: 0100 000r */
		sprintf(dst, "orl   a,@r%d", op&1);
		break;
	case 0x42: /* 1: 0100 0010 */
		sprintf(dst, "mov   a,t");
		break;
	case 0x43: /* 2: 0100 0011 */
		sprintf(dst, "orl   a,#$%02X", opram[PC++ - pc]);
		break;
	case 0x45: /* 1: 0100 0101 */
		sprintf(dst, "strt  cnt");
		break;
	case 0x46: /* 2: 0100 0110 */
		arg = opram[PC++ - pc];
		sprintf(dst, "jnt1  $%04X", (PC & 0x700) | arg);
		break;
	case 0x47: /* 1: 0100 0111 */
		sprintf(dst, "swap  a");
		break;
	case 0x48: /* 1: 0100 1rrr */
	case 0x49: /* 1: 0100 1rrr */
	case 0x4a: /* 1: 0100 1rrr */
	case 0x4b: /* 1: 0100 1rrr */
	case 0x4c: /* 1: 0100 1rrr */
	case 0x4d: /* 1: 0100 1rrr */
	case 0x4e: /* 1: 0100 1rrr */
	case 0x4f: /* 1: 0100 1rrr */
		sprintf(dst, "orl   a,r%d", op&7);
		break;
	case 0x50: /* 1: 0101 000r */
	case 0x51: /* 1: 0101 000r */
		sprintf(dst, "anl   a,@r%d", op&1);
		break;
	case 0x53: /* 2: 0101 0011 */
		sprintf(dst, "anl   a,#$%02X", opram[PC++ - pc]);
		break;
	case 0x55: /* 1: 0101 0101 */
		sprintf(dst, "strt  t");
		break;
	case 0x56: /* 2: 0101 0110 */
		arg = opram[PC++ - pc];
		sprintf(dst, "jt1   $%04X", (PC & 0x700) | arg);
		break;
	case 0x57: /* 1: 0101 0111 */
		sprintf(dst, "da    a");
		break;
	case 0x58: /* 1: 0101 1rrr */
	case 0x59: /* 1: 0101 1rrr */
	case 0x5a: /* 1: 0101 1rrr */
	case 0x5b: /* 1: 0101 1rrr */
	case 0x5c: /* 1: 0101 1rrr */
	case 0x5d: /* 1: 0101 1rrr */
	case 0x5e: /* 1: 0101 1rrr */
	case 0x5f: /* 1: 0101 1rrr */
		sprintf(dst, "anl   a,r%d", op&7);
		break;
	case 0x60: /* 1: 0110 000r */
	case 0x61: /* 1: 0110 000r */
		sprintf(dst, "add   a,@r%d", op&1);
		break;
	case 0x62: /* 1: 0110 0010 */
		sprintf(dst, "mov   t,a");
		break;
	case 0x63: /* 1: 0110 0011 */
		sprintf(dst, "ill");
		break;
	case 0x65: /* 1: 0110 0101 */
		sprintf(dst, "stop  tcnt");
		break;
	case 0x66: /* 1: 0110 0110 */
		sprintf(dst, "ill");
		break;
	case 0x67: /* 1: 0110 0111 */
		sprintf(dst, "rlc   a");
		break;
	case 0x68: /* 1: 0110 1rrr */
	case 0x69: /* 1: 0110 1rrr */
	case 0x6a: /* 1: 0110 1rrr */
	case 0x6b: /* 1: 0110 1rrr */
	case 0x6c: /* 1: 0110 1rrr */
	case 0x6d: /* 1: 0110 1rrr */
	case 0x6e: /* 1: 0110 1rrr */
	case 0x6f: /* 1: 0110 1rrr */
		sprintf(dst, "add   a,r%d", op&7);
		break;
	case 0x70: /* 1: 0111 000r */
	case 0x71: /* 1: 0111 000r */
		sprintf(dst, "addc  a,@r%d", op&1);
		break;
	case 0x73: /* 1: 0111 0011 */
		sprintf(dst, "ill");
		break;
	case 0x75: /* 1: 0111 0101 */
		sprintf(dst, "ill");
		break;
	case 0x76: /* 2: 0111 0110 */
		arg = opram[PC++ - pc];
		sprintf(dst, "jf1   $%04X", (PC & 0x700) | arg);
		break;
	case 0x77: /* 1: 0111 0111 */
		sprintf(dst, "rl    a");
		break;
	case 0x78: /* 1: 0111 1rrr */
	case 0x79: /* 1: 0111 1rrr */
	case 0x7a: /* 1: 0111 1rrr */
	case 0x7b: /* 1: 0111 1rrr */
	case 0x7c: /* 1: 0111 1rrr */
	case 0x7d: /* 1: 0111 1rrr */
	case 0x7e: /* 1: 0111 1rrr */
	case 0x7f: /* 1: 0111 1rrr */
		sprintf(dst, "addc  a,r%d", op&7);
		break;
	case 0x80: /* 1: 1000 0000 */
		sprintf(dst, "ill ");
		break;
	case 0x81: /* 1: 1000 0001 */
		sprintf(dst, "ill ");
		break;
	case 0x82: /* 1: 1000 0010 */
		sprintf(dst, "ill ");
		break;
	case 0x83: /* 2: 1000 0011 */
		sprintf(dst, "ret");
		flags = DASMFLAG_STEP_OUT;
		break;
	case 0x85: /* 1: 1000 0101 */
		sprintf(dst, "clr   f0");
		break;
	case 0x86: /* 2: 1000 0110 */
		arg = opram[PC++ - pc];
		sprintf(dst, "jobf  $%04X", (PC & 0x700) | arg);
		break;
	case 0x87: /* 1: 1000 0111 */
		sprintf(dst, "ill");
		break;
	case 0x88: /* 2: 1000 10pp */
	case 0x89: /* 2: 1000 10pp */
	case 0x8a: /* 2: 1000 10pp */
	case 0x8b: /* 2: 1000 10pp */
		sprintf(dst, "orl   p%d,#$%02X", op&3, opram[PC++ - pc]);
		break;
	case 0x8c: /* 2: 1000 11pp */
	case 0x8d: /* 2: 1000 11pp */
	case 0x8e: /* 2: 1000 11pp */
	case 0x8f: /* 2: 1000 11pp */
		sprintf(dst, "orld  p%d,a", op&7);
		break;
	case 0x90: /* 1: 1001 0000 */
		sprintf(dst, "mov   sts,a");
		break;
	case 0x91: /* 1: 1001 0001 */
		sprintf(dst, "ill");
		break;
	case 0x93: /* 2: 1001 0011 */
		sprintf(dst, "retr");
		flags = DASMFLAG_STEP_OVER;
		break;
	case 0x95: /* 1: 1001 0101 */
		sprintf(dst, "cpl   f0");
		break;
	case 0x96: /* 2: 1001 0110 */
		arg = opram[PC++ - pc];
		sprintf(dst, "jnz   $%04X", (PC & 0x700) | arg);
		break;
	case 0x97: /* 1: 1001 0111 */
		sprintf(dst, "clr   c");
		break;
	case 0x98: /* 2: 1001 10pp */
	case 0x99: /* 2: 1001 10pp */
	case 0x9a: /* 2: 1001 10pp */
	case 0x9b: /* 2: 1001 10pp */
		sprintf(dst, "anl   p%d,#$%02X", op&3, opram[PC++ - pc]);
		break;
	case 0x9c: /* 2: 1001 11pp */
	case 0x9d: /* 2: 1001 11pp */
	case 0x9e: /* 2: 1001 11pp */
	case 0x9f: /* 2: 1001 11pp */
		sprintf(dst, "anld  p%d,a", op&7);
		break;
	case 0xa0: /* 1: 1010 000r */
	case 0xa1: /* 1: 1010 000r */
		sprintf(dst, "mov   @r%d,a", op&1);
		break;
	case 0xa2: /* 1: 1010 0010 */
		sprintf(dst, "ill");
		break;
	case 0xa3: /* 2: 1010 0011 */
		sprintf(dst, "movp  a,@a");
		break;
	case 0xa5: /* 1: 1010 0101 */
		sprintf(dst, "clr   f1");
		break;
	case 0xa6: /* 1: 1010 0110 */
		sprintf(dst, "ill");
		break;
	case 0xa7: /* 1: 1010 0111 */
		sprintf(dst, "cpl   c");
		break;
	case 0xa8: /* 1: 1010 1rrr */
	case 0xa9: /* 1: 1010 1rrr */
	case 0xaa: /* 1: 1010 1rrr */
	case 0xab: /* 1: 1010 1rrr */
	case 0xac: /* 1: 1010 1rrr */
	case 0xad: /* 1: 1010 1rrr */
	case 0xae: /* 1: 1010 1rrr */
	case 0xaf: /* 1: 1010 1rrr */
		sprintf(dst, "mov   r%d,a", op&7);
		break;
	case 0xb0: /* 2: 1011 000r */
	case 0xb1: /* 2: 1011 000r */
		sprintf(dst, "mov   @r%d,#$%02X", op&1, opram[PC++ - pc]);
		break;
	case 0xb3: /* 2: 1011 0011 */
		sprintf(dst, "jmpp  @a");
		break;
	case 0xb5: /* 1: 1011 0101 */
		sprintf(dst, "cpl   f1");
		break;
	case 0xb6: /* 2: 1011 0110 */
		arg = opram[PC++ - pc];
		sprintf(dst, "jf0   $%04X", (PC & 0x700) | arg);
		break;
	case 0xb7: /* 1: 1011 0111 */
		sprintf(dst, "ill");
		break;
	case 0xb8: /* 1: 1011 1rrr */
	case 0xb9: /* 1: 1011 1rrr */
	case 0xba: /* 1: 1011 1rrr */
	case 0xbb: /* 1: 1011 1rrr */
	case 0xbc: /* 1: 1011 1rrr */
	case 0xbd: /* 1: 1011 1rrr */
	case 0xbe: /* 1: 1011 1rrr */
	case 0xbf: /* 1: 1011 1rrr */
		sprintf(dst, "mov   r%d,#$%02X", op&7, opram[PC++ - pc]);
		break;
	case 0xc0: /* 1: 1100 0000 */
		sprintf(dst, "ill");
		break;
	case 0xc1: /* 1: 1100 0001 */
		sprintf(dst, "ill");
		break;
	case 0xc2: /* 1: 1100 0010 */
		sprintf(dst, "ill");
		break;
	case 0xc3: /* 1: 1100 0011 */
		sprintf(dst, "ill");
		break;
	case 0xc5: /* 1: 1100 0101 */
		sprintf(dst, "sel   rb0");
		break;
	case 0xc6: /* 2: 1100 0110 */
		arg = opram[PC++ - pc];
		sprintf(dst, "jz    $%04X", (PC & 0x700) | arg);
		break;
	case 0xc7: /* 1: 1100 0111 */
		sprintf(dst, "mov   a,psw");
		break;
	case 0xc8: /* 1: 1100 1rrr */
	case 0xc9: /* 1: 1100 1rrr */
	case 0xca: /* 1: 1100 1rrr */
	case 0xcb: /* 1: 1100 1rrr */
	case 0xcc: /* 1: 1100 1rrr */
	case 0xcd: /* 1: 1100 1rrr */
	case 0xce: /* 1: 1100 1rrr */
	case 0xcf: /* 1: 1100 1rrr */
		sprintf(dst, "dec   r%d", op&7);
		break;
	case 0xd0: /* 1: 1101 000r */
	case 0xd1: /* 1: 1101 000r */
		sprintf(dst, "xrl   a,@r%d", op&1);
		break;
	case 0xd3: /* 1: 1101 0011 */
		sprintf(dst, "xrl   a,#$%02X", opram[PC++ - pc]);
		break;
	case 0xd5: /* 1: 1101 0101 */
		sprintf(dst, "sel   rb1");
		break;
	case 0xd6: /* 2: 1101 0110 */
		arg = opram[PC++ - pc];
		sprintf(dst, "jnibf $%04X", (PC & 0x700) | arg);
		break;
	case 0xd7: /* 1: 1101 0111 */
		sprintf(dst, "mov   psw,a");
		break;
	case 0xd8: /* 1: 1101 1rrr */
	case 0xd9: /* 1: 1101 1rrr */
	case 0xda: /* 1: 1101 1rrr */
	case 0xdb: /* 1: 1101 1rrr */
	case 0xdc: /* 1: 1101 1rrr */
	case 0xdd: /* 1: 1101 1rrr */
	case 0xde: /* 1: 1101 1rrr */
	case 0xdf: /* 1: 1101 1rrr */
		sprintf(dst, "xrl   a,r%d", op&7);
		break;
	case 0xe0: /* 1: 1110 0000 */
		sprintf(dst, "ill");
		break;
	case 0xe1: /* 1: 1110 0001 */
		sprintf(dst, "ill");
		break;
	case 0xe2: /* 1: 1110 0010 */
		sprintf(dst, "ill");
		break;
	case 0xe3: /* 2: 1110 0011 */
		sprintf(dst, "movp3 a,@a");
		break;
	case 0xe5: /* 1: 1110 0101 */
		sprintf(dst, "en    dma");
		break;
	case 0xe6: /* 2: 1110 0110 */
		arg = opram[PC++ - pc];
		sprintf(dst, "jnc   $%04X", (PC & 0x700) | arg);
		break;
	case 0xe7: /* 1: 1110 0111 */
		sprintf(dst, "rl    a");
		break;
	case 0xe8: /* 2: 1110 1rrr */
	case 0xe9: /* 2: 1110 1rrr */
	case 0xea: /* 2: 1110 1rrr */
	case 0xeb: /* 2: 1110 1rrr */
	case 0xec: /* 2: 1110 1rrr */
	case 0xed: /* 2: 1110 1rrr */
	case 0xee: /* 2: 1110 1rrr */
	case 0xef: /* 2: 1110 1rrr */
		arg = opram[PC++ - pc];
		sprintf(dst, "djnz  r%d,$%04X", op&7, (PC & 0x700) | arg);
		flags = DASMFLAG_STEP_OVER;
		break;
	case 0xf0: /* 1: 1111 000r */
	case 0xf1: /* 1: 1111 000r */
		sprintf(dst, "mov   a,@r%d", op&1);
		break;
	case 0xf3: /* 1: 1111 0011 */
		sprintf(dst, "ill");
		break;
	case 0xf5: /* 1: 1111 0101 */
		sprintf(dst, "en    flags");
		break;
	case 0xf6: /* 2: 1111 0110 */
		arg = opram[PC++ - pc];
		sprintf(dst, "jc    $%04X", (PC & 0x700) | arg);
		break;
	case 0xf7: /* 1: 1111 0111 */
		sprintf(dst, "rlc   a");
		break;
	case 0xf8: /* 1: 1111 1rrr */
	case 0xf9: /* 1: 1111 1rrr */
	case 0xfa: /* 1: 1111 1rrr */
	case 0xfb: /* 1: 1111 1rrr */
	case 0xfc: /* 1: 1111 1rrr */
	case 0xfd: /* 1: 1111 1rrr */
	case 0xfe: /* 1: 1111 1rrr */
	case 0xff: /* 1: 1111 1rrr */
		sprintf(dst, "mov   a,r%d", op&7);
		break;
	}
	return (PC - pc) | flags | DASMFLAG_SUPPORTED;
}
