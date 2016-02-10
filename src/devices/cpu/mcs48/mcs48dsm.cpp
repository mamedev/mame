// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    mcs48dsm.c

    Simple MCS-48/UPI-41 disassembler.
    Written by Aaron Giles

***************************************************************************/

#include "emu.h"


static UINT32 common_dasm(device_t *device, char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, int upi41)
{
	const UINT8 *startram = opram;
	UINT32 flags = 0;

	opram++;
	switch (*oprom++)
	{
		case 0x00:      sprintf(buffer, "nop");                                                 break;
		case 0x02:  if (!upi41)
						sprintf(buffer, "out  bus,a");
					else
						sprintf(buffer, "out  dbb,a");                                          break;
		case 0x03:      sprintf(buffer, "add  a,#$%02X", *opram++);                             break;
		case 0x04:      sprintf(buffer, "jmp  $0%02X", *opram++);                               break;
		case 0x05:      sprintf(buffer, "en   i");                                              break;
		case 0x07:      sprintf(buffer, "dec  a");                                              break;
		case 0x08:  if (!upi41)
						sprintf(buffer, "in   a,bus");
					else
						sprintf(buffer, "illegal");                                             break;
		case 0x09:      sprintf(buffer, "in   a,p1");                                           break;
		case 0x0a:      sprintf(buffer, "in   a,p2");                                           break;
		case 0x0c:      sprintf(buffer, "movd a,p4");                                           break;
		case 0x0d:      sprintf(buffer, "movd a,p5");                                           break;
		case 0x0e:      sprintf(buffer, "movd a,p6");                                           break;
		case 0x0f:      sprintf(buffer, "movd a,p7");                                           break;
		case 0x10:      sprintf(buffer, "inc  @r0");                                            break;
		case 0x11:      sprintf(buffer, "inc  @r1");                                            break;
		case 0x12:      sprintf(buffer, "jb0  $%03X", (pc & 0xf00) | *opram++);                 break;
		case 0x13:      sprintf(buffer, "addc a,#$%02X", *opram++);                             break;
		case 0x14:      sprintf(buffer, "call $0%02X", *opram++); flags = DASMFLAG_STEP_OVER;   break;
		case 0x15:      sprintf(buffer, "dis  i");                                              break;
		case 0x16:      sprintf(buffer, "jtf  $%03X", (pc & 0xf00) | *opram++);                 break;
		case 0x17:      sprintf(buffer, "inc  a");                                              break;
		case 0x18:      sprintf(buffer, "inc  r0");                                             break;
		case 0x19:      sprintf(buffer, "inc  r1");                                             break;
		case 0x1a:      sprintf(buffer, "inc  r2");                                             break;
		case 0x1b:      sprintf(buffer, "inc  r3");                                             break;
		case 0x1c:      sprintf(buffer, "inc  r4");                                             break;
		case 0x1d:      sprintf(buffer, "inc  r5");                                             break;
		case 0x1e:      sprintf(buffer, "inc  r6");                                             break;
		case 0x1f:      sprintf(buffer, "inc  r7");                                             break;
		case 0x20:      sprintf(buffer, "xch  a,@r0");                                          break;
		case 0x21:      sprintf(buffer, "xch  a,@r1");                                          break;
		case 0x22:  if (!upi41)
						sprintf(buffer, "illegal");
					else
						sprintf(buffer, "in   a,dbb");                                          break;
		case 0x23:      sprintf(buffer, "mov  a,#$%02X", *opram++);                             break;
		case 0x24:      sprintf(buffer, "jmp  $1%02X", *opram++);                               break;
		case 0x25:      sprintf(buffer, "en   tcnti");                                          break;
		case 0x26:      sprintf(buffer, "jnt0 $%03X", (pc & 0xf00) | *opram++);                 break;
		case 0x27:      sprintf(buffer, "clr  a");                                              break;
		case 0x28:      sprintf(buffer, "xch  a,r0");                                           break;
		case 0x29:      sprintf(buffer, "xch  a,r1");                                           break;
		case 0x2a:      sprintf(buffer, "xch  a,r2");                                           break;
		case 0x2b:      sprintf(buffer, "xch  a,r3");                                           break;
		case 0x2c:      sprintf(buffer, "xch  a,r4");                                           break;
		case 0x2d:      sprintf(buffer, "xch  a,r5");                                           break;
		case 0x2e:      sprintf(buffer, "xch  a,r6");                                           break;
		case 0x2f:      sprintf(buffer, "xch  a,r7");                                           break;
		case 0x30:      sprintf(buffer, "xchd a,@r0");                                          break;
		case 0x31:      sprintf(buffer, "xchd a,@r1");                                          break;
		case 0x32:      sprintf(buffer, "jb1  $%03X", (pc & 0xf00) | *opram++);                 break;
		case 0x34:      sprintf(buffer, "call $1%02X", *opram++); flags = DASMFLAG_STEP_OVER;   break;
		case 0x35:      sprintf(buffer, "dis  tcnti");                                          break;
		case 0x36:      sprintf(buffer, "jt0  $%03X", (pc & 0xf00) | *opram++);                 break;
		case 0x37:      sprintf(buffer, "cpl  a");                                              break;
		case 0x39:      sprintf(buffer, "outl p1,a");                                           break;
		case 0x3a:      sprintf(buffer, "outl p2,a");                                           break;
		case 0x3c:      sprintf(buffer, "movd p4,a");                                           break;
		case 0x3d:      sprintf(buffer, "movd p5,a");                                           break;
		case 0x3e:      sprintf(buffer, "movd p6,a");                                           break;
		case 0x3f:      sprintf(buffer, "movd p7,a");                                           break;
		case 0x40:      sprintf(buffer, "orl  a,@r0");                                          break;
		case 0x41:      sprintf(buffer, "orl  a,@r1");                                          break;
		case 0x42:      sprintf(buffer, "mov  a,t");                                            break;
		case 0x43:      sprintf(buffer, "orl  a,#$%02X", *opram++);                             break;
		case 0x44:      sprintf(buffer, "jmp  $2%02X", *opram++);                               break;
		case 0x45:      sprintf(buffer, "strt cnt");                                            break;
		case 0x46:      sprintf(buffer, "jnt1 $%03X", (pc & 0xf00) | *opram++);                 break;
		case 0x47:      sprintf(buffer, "swap a");                                              break;
		case 0x48:      sprintf(buffer, "orl  a,r0");                                           break;
		case 0x49:      sprintf(buffer, "orl  a,r1");                                           break;
		case 0x4a:      sprintf(buffer, "orl  a,r2");                                           break;
		case 0x4b:      sprintf(buffer, "orl  a,r3");                                           break;
		case 0x4c:      sprintf(buffer, "orl  a,r4");                                           break;
		case 0x4d:      sprintf(buffer, "orl  a,r5");                                           break;
		case 0x4e:      sprintf(buffer, "orl  a,r6");                                           break;
		case 0x4f:      sprintf(buffer, "orl  a,r7");                                           break;
		case 0x50:      sprintf(buffer, "anl  a,@r0");                                          break;
		case 0x51:      sprintf(buffer, "anl  a,@r1");                                          break;
		case 0x52:      sprintf(buffer, "jb2  $%03X", (pc & 0xf00) | *opram++);                 break;
		case 0x53:      sprintf(buffer, "anl  a,#$%02X", *opram++);                             break;
		case 0x54:      sprintf(buffer, "call $2%02X", *opram++); flags = DASMFLAG_STEP_OVER;   break;
		case 0x55:      sprintf(buffer, "strt t");                                              break;
		case 0x56:      sprintf(buffer, "jt1  $%03X", (pc & 0xf00) | *opram++);                 break;
		case 0x57:      sprintf(buffer, "da   a");                                              break;
		case 0x58:      sprintf(buffer, "anl  a,r0");                                           break;
		case 0x59:      sprintf(buffer, "anl  a,r1");                                           break;
		case 0x5a:      sprintf(buffer, "anl  a,r2");                                           break;
		case 0x5b:      sprintf(buffer, "anl  a,r3");                                           break;
		case 0x5c:      sprintf(buffer, "anl  a,r4");                                           break;
		case 0x5d:      sprintf(buffer, "anl  a,r5");                                           break;
		case 0x5e:      sprintf(buffer, "anl  a,r6");                                           break;
		case 0x5f:      sprintf(buffer, "anl  a,r7");                                           break;
		case 0x60:      sprintf(buffer, "add  a,@r0");                                          break;
		case 0x61:      sprintf(buffer, "add  a,@r1");                                          break;
		case 0x62:      sprintf(buffer, "mov  t,a");                                            break;
		case 0x64:      sprintf(buffer, "jmp  $3%02X", *opram++);                               break;
		case 0x65:      sprintf(buffer, "stop tcnt");                                           break;
		case 0x67:      sprintf(buffer, "rrc  a");                                              break;
		case 0x68:      sprintf(buffer, "add  a,r0");                                           break;
		case 0x69:      sprintf(buffer, "add  a,r1");                                           break;
		case 0x6a:      sprintf(buffer, "add  a,r2");                                           break;
		case 0x6b:      sprintf(buffer, "add  a,r3");                                           break;
		case 0x6c:      sprintf(buffer, "add  a,r4");                                           break;
		case 0x6d:      sprintf(buffer, "add  a,r5");                                           break;
		case 0x6e:      sprintf(buffer, "add  a,r6");                                           break;
		case 0x6f:      sprintf(buffer, "add  a,r7");                                           break;
		case 0x70:      sprintf(buffer, "addc a,@r0");                                          break;
		case 0x71:      sprintf(buffer, "addc a,@r1");                                          break;
		case 0x72:      sprintf(buffer, "jb3  $%03X", (pc & 0xf00) | *opram++);                 break;
		case 0x74:      sprintf(buffer, "call $3%02X", *opram++); flags = DASMFLAG_STEP_OVER;   break;
		case 0x75:  if (!upi41)
						sprintf(buffer, "ent0 clk");
					else
						sprintf(buffer, "illegal");                                             break;
		case 0x76:      sprintf(buffer, "jf1  $%03X", (pc & 0xf00) | *opram++);                 break;
		case 0x77:      sprintf(buffer, "rr   a");                                              break;
		case 0x78:      sprintf(buffer, "addc a,r0");                                           break;
		case 0x79:      sprintf(buffer, "addc a,r1");                                           break;
		case 0x7a:      sprintf(buffer, "addc a,r2");                                           break;
		case 0x7b:      sprintf(buffer, "addc a,r3");                                           break;
		case 0x7c:      sprintf(buffer, "addc a,r4");                                           break;
		case 0x7d:      sprintf(buffer, "addc a,r5");                                           break;
		case 0x7e:      sprintf(buffer, "addc a,r6");                                           break;
		case 0x7f:      sprintf(buffer, "addc a,r7");                                           break;
		case 0x80:  if (!upi41)
						sprintf(buffer, "movx a,@r0");
					else
						sprintf(buffer, "illegal");                                             break;
		case 0x81:  if (!upi41)
						sprintf(buffer, "movx a,@r1");
					else
						sprintf(buffer, "illegal");                                             break;
		case 0x83:      sprintf(buffer, "ret"); flags = DASMFLAG_STEP_OUT;                      break;
		case 0x84:      sprintf(buffer, "jmp  $4%02X", *opram++);                               break;
		case 0x85:      sprintf(buffer, "clr  f0");                                             break;
		case 0x86:  if (!upi41)
						sprintf(buffer, "jni  $%03X", (pc & 0xf00) | *opram++);
					else
						sprintf(buffer, "jobf $%03X", (pc & 0xf00) | *opram++);                 break;
		case 0x88:  if (!upi41)
						sprintf(buffer, "orl  bus,#$%02X", *opram++);
					else
						sprintf(buffer, "illegal");                                             break;
		case 0x89:      sprintf(buffer, "orl  p1,#$%02X", *opram++);                            break;
		case 0x8a:      sprintf(buffer, "orl  p2,#$%02X", *opram++);                            break;
		case 0x8c:      sprintf(buffer, "orld p4,a");                                           break;
		case 0x8d:      sprintf(buffer, "orld p5,a");                                           break;
		case 0x8e:      sprintf(buffer, "orld p6,a");                                           break;
		case 0x8f:      sprintf(buffer, "orld p7,a");                                           break;
		case 0x90:  if (!upi41)
						sprintf(buffer, "movx @r0,a");
					else
						sprintf(buffer, "mov  sts,a");                                          break;
		case 0x91:  if (!upi41)
						sprintf(buffer, "movx @r1,a");
					else
						sprintf(buffer, "illegal");                                             break;
		case 0x92:      sprintf(buffer, "jb4  $%03X", (pc & 0xf00) | *opram++);                 break;
		case 0x93:      sprintf(buffer, "retr"); flags = DASMFLAG_STEP_OUT;                     break;
		case 0x94:      sprintf(buffer, "call $4%02X", *opram++); flags = DASMFLAG_STEP_OVER;   break;
		case 0x95:      sprintf(buffer, "cpl  f0");                                             break;
		case 0x96:      sprintf(buffer, "jnz  $%03X", (pc & 0xf00) | *opram++);                 break;
		case 0x97:      sprintf(buffer, "clr  c");                                              break;
		case 0x98:  if (!upi41)
						sprintf(buffer, "anl  bus,#$%02X", *opram++);
					else
						sprintf(buffer, "illegal");                                             break;
		case 0x99:      sprintf(buffer, "anl  p1,#$%02X", *opram++);                            break;
		case 0x9a:      sprintf(buffer, "anl  p2,#$%02X", *opram++);                            break;
		case 0x9c:      sprintf(buffer, "anld p4,a");                                           break;
		case 0x9d:      sprintf(buffer, "anld p5,a");                                           break;
		case 0x9e:      sprintf(buffer, "anld p6,a");                                           break;
		case 0x9f:      sprintf(buffer, "anld p7,a");                                           break;
		case 0xa0:      sprintf(buffer, "mov  @r0,a");                                          break;
		case 0xa1:      sprintf(buffer, "mov  @r1,a");                                          break;
		case 0xa3:      sprintf(buffer, "movp a,@a");                                           break;
		case 0xa4:      sprintf(buffer, "jmp  $5%02X", *opram++);                               break;
		case 0xa5:      sprintf(buffer, "clr  f1");                                             break;
		case 0xa7:      sprintf(buffer, "cpl  c");                                              break;
		case 0xa8:      sprintf(buffer, "mov  r0,a");                                           break;
		case 0xa9:      sprintf(buffer, "mov  r1,a");                                           break;
		case 0xaa:      sprintf(buffer, "mov  r2,a");                                           break;
		case 0xab:      sprintf(buffer, "mov  r3,a");                                           break;
		case 0xac:      sprintf(buffer, "mov  r4,a");                                           break;
		case 0xad:      sprintf(buffer, "mov  r5,a");                                           break;
		case 0xae:      sprintf(buffer, "mov  r6,a");                                           break;
		case 0xaf:      sprintf(buffer, "mov  r7,a");                                           break;
		case 0xb0:      sprintf(buffer, "mov  @r0,#$%02X", *opram++);                           break;
		case 0xb1:      sprintf(buffer, "mov  @r1,#$%02X", *opram++);                           break;
		case 0xb2:      sprintf(buffer, "jb5  $%03X", (pc & 0xf00) | *opram++);                 break;
		case 0xb3:      sprintf(buffer, "jmpp @a");                                             break;
		case 0xb4:      sprintf(buffer, "call $5%02X", *opram++); flags = DASMFLAG_STEP_OVER;   break;
		case 0xb5:      sprintf(buffer, "cpl  f1");                                             break;
		case 0xb6:      sprintf(buffer, "jf0  $%03X", (pc & 0xf00) | *opram++);                 break;
		case 0xb8:      sprintf(buffer, "mov  r0,#$%02X", *opram++);                            break;
		case 0xb9:      sprintf(buffer, "mov  r1,#$%02X", *opram++);                            break;
		case 0xba:      sprintf(buffer, "mov  r2,#$%02X", *opram++);                            break;
		case 0xbb:      sprintf(buffer, "mov  r3,#$%02X", *opram++);                            break;
		case 0xbc:      sprintf(buffer, "mov  r4,#$%02X", *opram++);                            break;
		case 0xbd:      sprintf(buffer, "mov  r5,#$%02X", *opram++);                            break;
		case 0xbe:      sprintf(buffer, "mov  r6,#$%02X", *opram++);                            break;
		case 0xbf:      sprintf(buffer, "mov  r7,#$%02X", *opram++);                            break;
		case 0xc4:      sprintf(buffer, "jmp  $6%02X", *opram++);                               break;
		case 0xc5:      sprintf(buffer, "sel  rb0");                                            break;
		case 0xc6:      sprintf(buffer, "jz   $%03X", (pc & 0xf00) | *opram++);                 break;
		case 0xc7:      sprintf(buffer, "mov  a,psw");                                          break;
		case 0xc8:      sprintf(buffer, "dec  r0");                                             break;
		case 0xc9:      sprintf(buffer, "dec  r1");                                             break;
		case 0xca:      sprintf(buffer, "dec  r2");                                             break;
		case 0xcb:      sprintf(buffer, "dec  r3");                                             break;
		case 0xcc:      sprintf(buffer, "dec  r4");                                             break;
		case 0xcd:      sprintf(buffer, "dec  r5");                                             break;
		case 0xce:      sprintf(buffer, "dec  r6");                                             break;
		case 0xcf:      sprintf(buffer, "dec  r7");                                             break;
		case 0xd0:      sprintf(buffer, "xrl  a,@r0");                                          break;
		case 0xd1:      sprintf(buffer, "xrl  a,@r1");                                          break;
		case 0xd2:      sprintf(buffer, "jb6  $%03X", (pc & 0xf00) | *opram++);                 break;
		case 0xd3:      sprintf(buffer, "xrl  a,#$%02X", *opram++);                             break;
		case 0xd4:      sprintf(buffer, "call $6%02X", *opram++); flags = DASMFLAG_STEP_OVER;   break;
		case 0xd5:      sprintf(buffer, "sel  rb1");                                            break;
		case 0xd6:  if (!upi41)
						sprintf(buffer, "illegal");
					else
						sprintf(buffer, "jnibf $%03X", (pc & 0xf00) | *opram++);                break;
		case 0xd7:      sprintf(buffer, "mov  psw,a");                                          break;
		case 0xd8:      sprintf(buffer, "xrl  a,r0");                                           break;
		case 0xd9:      sprintf(buffer, "xrl  a,r1");                                           break;
		case 0xda:      sprintf(buffer, "xrl  a,r2");                                           break;
		case 0xdb:      sprintf(buffer, "xrl  a,r3");                                           break;
		case 0xdc:      sprintf(buffer, "xrl  a,r4");                                           break;
		case 0xdd:      sprintf(buffer, "xrl  a,r5");                                           break;
		case 0xde:      sprintf(buffer, "xrl  a,r6");                                           break;
		case 0xdf:      sprintf(buffer, "xrl  a,r7");                                           break;
		case 0xe3:      sprintf(buffer, "movp3 a,@a");                                          break;
		case 0xe4:      sprintf(buffer, "jmp  $7%02X", *opram++);                               break;
		case 0xe5:  if (!upi41)
						sprintf(buffer, "sel  mb0");
					else
						sprintf(buffer, "en   dma");                                            break;
		case 0xe6:      sprintf(buffer, "jnc  $%03X", (pc & 0xf00) | *opram++);                 break;
		case 0xe7:      sprintf(buffer, "rl   a");                                              break;
		case 0xe8:      sprintf(buffer, "djnz r0,$%03X", (pc & 0xf00) | *opram++); flags = DASMFLAG_STEP_OVER; break;
		case 0xe9:      sprintf(buffer, "djnz r1,$%03X", (pc & 0xf00) | *opram++); flags = DASMFLAG_STEP_OVER; break;
		case 0xea:      sprintf(buffer, "djnz r2,$%03X", (pc & 0xf00) | *opram++); flags = DASMFLAG_STEP_OVER; break;
		case 0xeb:      sprintf(buffer, "djnz r3,$%03X", (pc & 0xf00) | *opram++); flags = DASMFLAG_STEP_OVER; break;
		case 0xec:      sprintf(buffer, "djnz r4,$%03X", (pc & 0xf00) | *opram++); flags = DASMFLAG_STEP_OVER; break;
		case 0xed:      sprintf(buffer, "djnz r5,$%03X", (pc & 0xf00) | *opram++); flags = DASMFLAG_STEP_OVER; break;
		case 0xee:      sprintf(buffer, "djnz r6,$%03X", (pc & 0xf00) | *opram++); flags = DASMFLAG_STEP_OVER; break;
		case 0xef:      sprintf(buffer, "djnz r7,$%03X", (pc & 0xf00) | *opram++); flags = DASMFLAG_STEP_OVER; break;
		case 0xf0:      sprintf(buffer, "mov  a,@r0");                                          break;
		case 0xf1:      sprintf(buffer, "mov  a,@r1");                                          break;
		case 0xf2:      sprintf(buffer, "jb7  $%03X", (pc & 0xf00) | *opram++);                 break;
		case 0xf4:      sprintf(buffer, "call $7%02X", *opram++); flags = DASMFLAG_STEP_OVER;   break;
		case 0xf5:  if (!upi41)
						sprintf(buffer, "sel  mb1");
					else
						sprintf(buffer, "en   flags");                                          break;
		case 0xf6:      sprintf(buffer, "jc   $%03X", (pc & 0xf00) | *opram++);                 break;
		case 0xf7:      sprintf(buffer, "rlc  a");                                              break;
		case 0xf8:      sprintf(buffer, "mov  a,r0");                                           break;
		case 0xf9:      sprintf(buffer, "mov  a,r1");                                           break;
		case 0xfa:      sprintf(buffer, "mov  a,r2");                                           break;
		case 0xfb:      sprintf(buffer, "mov  a,r3");                                           break;
		case 0xfc:      sprintf(buffer, "mov  a,r4");                                           break;
		case 0xfd:      sprintf(buffer, "mov  a,r5");                                           break;
		case 0xfe:      sprintf(buffer, "mov  a,r6");                                           break;
		case 0xff:      sprintf(buffer, "mov  a,r7");                                           break;
		default:        sprintf(buffer, "illegal");                                             break;
	}

	return (opram - startram) | flags | DASMFLAG_SUPPORTED;
}


CPU_DISASSEMBLE( mcs48 )
{
	return common_dasm(device, buffer, pc, oprom, opram, FALSE);
}


CPU_DISASSEMBLE( upi41 )
{
	return common_dasm(device, buffer, pc, oprom, opram, TRUE);
}
