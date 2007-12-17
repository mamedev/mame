/*****************************************************************************
 *
 *   8085dasm.c
 *   Portable I8085A disassembler
 *
 *   Copyright (C) 1998,1999,2000 Juergen Buchmueller, all rights reserved.
 *   You can contact me at juergen@mame.net or pullmoll@stop1984.com
 *
 *   - This source code is released as freeware for non-commercial purposes
 *     as part of the M.A.M.E. (Multiple Arcade Machine Emulator) project.
 *     The licensing terms of MAME apply to this piece of code for the MAME
 *     project and derviative works, as defined by the MAME license. You
 *     may opt to make modifications, improvements or derivative works under
 *     that same conditions, and the MAME project may opt to keep
 *     modifications, improvements or derivatives under their terms exclusively.
 *
 *   - Alternatively you can choose to apply the terms of the "GPL" (see
 *     below) to this - and only this - piece of code or your derivative works.
 *     Note that in no case your choice can have any impact on any other
 *     source code of the MAME project, or binary, or executable, be it closely
 *     or losely related to this piece of code.
 *
 *  -  At your choice you are also free to remove either licensing terms from
 *     this file and continue to use it under only one of the two licenses. Do this
 *     if you think that licenses are not compatible (enough) for you, or if you
 *     consider either license 'too restrictive' or 'too free'.
 *
 *  -  GPL (GNU General Public License)
 *     This program is free software; you can redistribute it and/or
 *     modify it under the terms of the GNU General Public License
 *     as published by the Free Software Foundation; either version 2
 *     of the License, or (at your option) any later version.
 *
 *     This program is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public License
 *     along with this program; if not, write to the Free Software
 *     Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 *****************************************************************************/

#include "cpuintrf.h"

/* 8080/8085A mnemonics were more irritation than information
   What would you guess "CP $3456" to mean? It's not compare,
   but call if plus ... therefore: */
//#define Z80_MNEMONICS

#define OP(A)   oprom[(A) - PC]
#define ARG(A)  opram[(A) - PC]
#define ARGW(A) (opram[(A) - PC] | (opram[(A) + 1 - PC] << 8))

offs_t i8085_dasm(char *buff, offs_t pc, const UINT8 *oprom, const UINT8 *opram)
{
	UINT32 flags = 0;
	UINT8 op;
	unsigned PC = pc;
	switch (op = OP(pc++))
	{
#ifdef  Z80_MNEMONICS
		case 0x00: sprintf (buff,"nop");                             break;
		case 0x01: sprintf (buff,"ld   bc,$%04x", ARGW(pc)); pc+=2;  break;
		case 0x02: sprintf (buff,"ld   (bc),a");                     break;
		case 0x03: sprintf (buff,"inc  bc");                         break;
		case 0x04: sprintf (buff,"inc  b");                          break;
		case 0x05: sprintf (buff,"dec  b");                          break;
		case 0x06: sprintf (buff,"ld   b,$%02x", ARG(pc)); pc++;     break;
		case 0x07: sprintf (buff,"rlca");                            break;
		case 0x08: sprintf (buff,"sub  hl,bc (*)");                  break;
		case 0x09: sprintf (buff,"add  hl,bc");                      break;
		case 0x0a: sprintf (buff,"ld   a,(bc)");                     break;
		case 0x0b: sprintf (buff,"dec  bc");                         break;
		case 0x0c: sprintf (buff,"inc  c");                          break;
		case 0x0d: sprintf (buff,"dec  c");                          break;
		case 0x0e: sprintf (buff,"ld   c,$%02x", ARG(pc)); pc++;     break;
		case 0x0f: sprintf (buff,"rrca");                            break;
		case 0x10: sprintf (buff,"sra  hl (*)");                     break;
		case 0x11: sprintf (buff,"ld   de,$%04x", ARGW(pc)); pc+=2;  break;
		case 0x12: sprintf (buff,"ld   (de),a");                     break;
		case 0x13: sprintf (buff,"inc  de");                         break;
		case 0x14: sprintf (buff,"inc  d");                          break;
		case 0x15: sprintf (buff,"dec  d");                          break;
		case 0x16: sprintf (buff,"ld   d,$%02x", ARG(pc)); pc++;     break;
		case 0x17: sprintf (buff,"rla");                             break;
		case 0x18: sprintf (buff,"rl   de (*)");                     break;
		case 0x19: sprintf (buff,"add  hl,de");                      break;
		case 0x1a: sprintf (buff,"ld   a,(de)");                     break;
		case 0x1b: sprintf (buff,"dec  de");                         break;
		case 0x1c: sprintf (buff,"inc  e");                          break;
		case 0x1d: sprintf (buff,"dec  e");                          break;
		case 0x1e: sprintf (buff,"ld   e,$%02x", ARG(pc)); pc++;     break;
		case 0x1f: sprintf (buff,"rra");                             break;
		case 0x20: sprintf (buff,"rim");                             break;
		case 0x21: sprintf (buff,"ld   hl,$%04x", ARGW(pc)); pc+=2;  break;
		case 0x22: sprintf (buff,"ld   ($%04x),hl", ARGW(pc)); pc+=2;break;
		case 0x23: sprintf (buff,"inc  hl");                         break;
		case 0x24: sprintf (buff,"inc  h");                          break;
		case 0x25: sprintf (buff,"dec  h");                          break;
		case 0x26: sprintf (buff,"ld   h,$%02x", ARG(pc)); pc++;     break;
		case 0x27: sprintf (buff,"daa");                             break;
		case 0x28: sprintf (buff,"ld   de,hl+$%02x (*)",ARG(pc));pc++;break;
		case 0x29: sprintf (buff,"add  hl,hl");                      break;
		case 0x2a: sprintf (buff,"ld   hl,($%04x)", ARGW(pc)); pc+=2;break;
		case 0x2b: sprintf (buff,"dec  hl");                         break;
		case 0x2c: sprintf (buff,"inc  l");                          break;
		case 0x2d: sprintf (buff,"dec  l");                          break;
		case 0x2e: sprintf (buff,"ld   l,$%02x", ARG(pc)); pc++;     break;
		case 0x2f: sprintf (buff,"cpl");                             break;
		case 0x30: sprintf (buff,"sim");                             break;
		case 0x31: sprintf (buff,"ld   sp,$%04x", ARGW(pc)); pc+=2;  break;
		case 0x32: sprintf (buff,"ld   ($%04x),a", ARGW(pc)); pc+=2; break;
		case 0x33: sprintf (buff,"inc  sp");                         break;
		case 0x34: sprintf (buff,"inc  (hl)");                       break;
		case 0x35: sprintf (buff,"dec  (hl)");                       break;
		case 0x36: sprintf (buff,"ld   (hl),$%02x", ARG(pc)); pc++;  break;
		case 0x37: sprintf (buff,"scf");                             break;
		case 0x38: sprintf (buff,"ld   de,sp+$%02x (*)",ARG(pc));pc++;break;
		case 0x39: sprintf (buff,"add  hl,sp");                      break;
		case 0x3a: sprintf (buff,"ld   a,($%04x)", ARGW(pc)); pc+=2; break;
		case 0x3b: sprintf (buff,"dec  sp");                         break;
		case 0x3c: sprintf (buff,"inc  a");                          break;
		case 0x3d: sprintf (buff,"dec  a");                          break;
		case 0x3e: sprintf (buff,"ld   a,$%02x", ARG(pc)); pc++;     break;
		case 0x3f: sprintf (buff,"ccf");                             break;
		case 0x40: sprintf (buff,"ld   b,b");                        break;
		case 0x41: sprintf (buff,"ld   b,c");                        break;
		case 0x42: sprintf (buff,"ld   b,d");                        break;
		case 0x43: sprintf (buff,"ld   b,e");                        break;
		case 0x44: sprintf (buff,"ld   b,h");                        break;
		case 0x45: sprintf (buff,"ld   b,l");                        break;
		case 0x46: sprintf (buff,"ld   b,(hl)");                     break;
		case 0x47: sprintf (buff,"ld   b,a");                        break;
		case 0x48: sprintf (buff,"ld   c,b");                        break;
		case 0x49: sprintf (buff,"ld   c,c");                        break;
		case 0x4a: sprintf (buff,"ld   c,d");                        break;
		case 0x4b: sprintf (buff,"ld   c,e");                        break;
		case 0x4c: sprintf (buff,"ld   c,h");                        break;
		case 0x4d: sprintf (buff,"ld   c,l");                        break;
		case 0x4e: sprintf (buff,"ld   c,(hl)");                     break;
		case 0x4f: sprintf (buff,"ld   c,a");                        break;
		case 0x50: sprintf (buff,"ld   d,b");                        break;
		case 0x51: sprintf (buff,"ld   d,c");                        break;
		case 0x52: sprintf (buff,"ld   d,d");                        break;
		case 0x53: sprintf (buff,"ld   d,e");                        break;
		case 0x54: sprintf (buff,"ld   d,h");                        break;
		case 0x55: sprintf (buff,"ld   d,l");                        break;
		case 0x56: sprintf (buff,"ld   d,(hl)");                     break;
		case 0x57: sprintf (buff,"ld   d,a");                        break;
		case 0x58: sprintf (buff,"ld   e,b");                        break;
		case 0x59: sprintf (buff,"ld   e,c");                        break;
		case 0x5a: sprintf (buff,"ld   e,d");                        break;
		case 0x5b: sprintf (buff,"ld   e,e");                        break;
		case 0x5c: sprintf (buff,"ld   e,h");                        break;
		case 0x5d: sprintf (buff,"ld   e,l");                        break;
		case 0x5e: sprintf (buff,"ld   e,(hl)");                     break;
		case 0x5f: sprintf (buff,"ld   e,a");                        break;
		case 0x60: sprintf (buff,"ld   h,b");                        break;
		case 0x61: sprintf (buff,"ld   h,c");                        break;
		case 0x62: sprintf (buff,"ld   h,d");                        break;
		case 0x63: sprintf (buff,"ld   h,e");                        break;
		case 0x64: sprintf (buff,"ld   h,h");                        break;
		case 0x65: sprintf (buff,"ld   h,l");                        break;
		case 0x66: sprintf (buff,"ld   h,(hl)");                     break;
		case 0x67: sprintf (buff,"ld   h,a");                        break;
		case 0x68: sprintf (buff,"ld   l,b");                        break;
		case 0x69: sprintf (buff,"ld   l,c");                        break;
		case 0x6a: sprintf (buff,"ld   l,d");                        break;
		case 0x6b: sprintf (buff,"ld   l,e");                        break;
		case 0x6c: sprintf (buff,"ld   l,h");                        break;
		case 0x6d: sprintf (buff,"ld   l,l");                        break;
		case 0x6e: sprintf (buff,"ld   l,(hl)");                     break;
		case 0x6f: sprintf (buff,"ld   l,a");                        break;
		case 0x70: sprintf (buff,"ld   (hl),b");                     break;
		case 0x71: sprintf (buff,"ld   (hl),c");                     break;
		case 0x72: sprintf (buff,"ld   (hl),d");                     break;
		case 0x73: sprintf (buff,"ld   (hl),e");                     break;
		case 0x74: sprintf (buff,"ld   (hl),h");                     break;
		case 0x75: sprintf (buff,"ld   (hl),l");                     break;
		case 0x76: sprintf (buff,"halt");                            break;
		case 0x77: sprintf (buff,"ld   (hl),a");                     break;
		case 0x78: sprintf (buff,"ld   a,b");                        break;
		case 0x79: sprintf (buff,"ld   a,c");                        break;
		case 0x7a: sprintf (buff,"ld   a,d");                        break;
		case 0x7b: sprintf (buff,"ld   a,e");                        break;
		case 0x7c: sprintf (buff,"ld   a,h");                        break;
		case 0x7d: sprintf (buff,"ld   a,l");                        break;
		case 0x7e: sprintf (buff,"ld   a,(hl)");                     break;
		case 0x7f: sprintf (buff,"ld   a,a");                        break;
		case 0x80: sprintf (buff,"add  a,b");                        break;
		case 0x81: sprintf (buff,"add  a,c");                        break;
		case 0x82: sprintf (buff,"add  a,d");                        break;
		case 0x83: sprintf (buff,"add  a,e");                        break;
		case 0x84: sprintf (buff,"add  a,h");                        break;
		case 0x85: sprintf (buff,"add  a,l");                        break;
		case 0x86: sprintf (buff,"add  a,(hl)");                     break;
		case 0x87: sprintf (buff,"add  a,a");                        break;
		case 0x88: sprintf (buff,"adc  a,b");                        break;
		case 0x89: sprintf (buff,"adc  a,c");                        break;
		case 0x8a: sprintf (buff,"adc  a,d");                        break;
		case 0x8b: sprintf (buff,"adc  a,e");                        break;
		case 0x8c: sprintf (buff,"adc  a,h");                        break;
		case 0x8d: sprintf (buff,"adc  a,l");                        break;
		case 0x8e: sprintf (buff,"adc  a,(hl)");                     break;
		case 0x8f: sprintf (buff,"adc  a,a");                        break;
		case 0x90: sprintf (buff,"sub  b");                          break;
		case 0x91: sprintf (buff,"sub  c");                          break;
		case 0x92: sprintf (buff,"sub  d");                          break;
		case 0x93: sprintf (buff,"sub  e");                          break;
		case 0x94: sprintf (buff,"sub  h");                          break;
		case 0x95: sprintf (buff,"sub  l");                          break;
		case 0x96: sprintf (buff,"sub  (hl)");                       break;
		case 0x97: sprintf (buff,"sub  a");                          break;
		case 0x98: sprintf (buff,"sbc  a,b");                        break;
		case 0x99: sprintf (buff,"sbc  a,c");                        break;
		case 0x9a: sprintf (buff,"sbc  a,d");                        break;
		case 0x9b: sprintf (buff,"sbc  a,e");                        break;
		case 0x9c: sprintf (buff,"sbc  a,h");                        break;
		case 0x9d: sprintf (buff,"sbc  a,l");                        break;
		case 0x9e: sprintf (buff,"sbc  a,(hl)");                     break;
		case 0x9f: sprintf (buff,"sbc  a,a");                        break;
		case 0xa0: sprintf (buff,"and  b");                          break;
		case 0xa1: sprintf (buff,"and  c");                          break;
		case 0xa2: sprintf (buff,"and  d");                          break;
		case 0xa3: sprintf (buff,"and  e");                          break;
		case 0xa4: sprintf (buff,"and  h");                          break;
		case 0xa5: sprintf (buff,"and  l");                          break;
		case 0xa6: sprintf (buff,"and  (hl)");                       break;
		case 0xa7: sprintf (buff,"and  a");                          break;
		case 0xa8: sprintf (buff,"xor  b");                          break;
		case 0xa9: sprintf (buff,"xor  c");                          break;
		case 0xaa: sprintf (buff,"xor  d");                          break;
		case 0xab: sprintf (buff,"xor  e");                          break;
		case 0xac: sprintf (buff,"xor  h");                          break;
		case 0xad: sprintf (buff,"xor  l");                          break;
		case 0xae: sprintf (buff,"xor  (hl)");                       break;
		case 0xaf: sprintf (buff,"xor  a");                          break;
		case 0xb0: sprintf (buff,"or   b");                          break;
		case 0xb1: sprintf (buff,"or   c");                          break;
		case 0xb2: sprintf (buff,"or   d");                          break;
		case 0xb3: sprintf (buff,"or   e");                          break;
		case 0xb4: sprintf (buff,"or   h");                          break;
		case 0xb5: sprintf (buff,"or   l");                          break;
		case 0xb6: sprintf (buff,"or   (hl)");                       break;
		case 0xb7: sprintf (buff,"or   a");                          break;
		case 0xb8: sprintf (buff,"cp   b");                          break;
		case 0xb9: sprintf (buff,"cp   c");                          break;
		case 0xba: sprintf (buff,"cp   d");                          break;
		case 0xbb: sprintf (buff,"cp   e");                          break;
		case 0xbc: sprintf (buff,"cp   h");                          break;
		case 0xbd: sprintf (buff,"cp   l");                          break;
		case 0xbe: sprintf (buff,"cp   (hl)");                       break;
		case 0xbf: sprintf (buff,"cp   a");                          break;
		case 0xc0: sprintf (buff,"ret  nz"); flags = DASMFLAG_STEP_OUT; break;
		case 0xc1: sprintf (buff,"pop  bc");                         break;
		case 0xc2: sprintf (buff,"jp   nz,$%04x", ARGW(pc)); pc+=2;  break;
		case 0xc3: sprintf (buff,"jp   $%04x", ARGW(pc)); pc+=2;     break;
		case 0xc4: sprintf (buff,"call nz,$%04x", ARGW(pc)); pc+=2; flags = DASMFLAG_STEP_OVER; break;
		case 0xc5: sprintf (buff,"push bc");                         break;
		case 0xc6: sprintf (buff,"add  a,$%02x", ARG(pc)); pc++;     break;
		case 0xc7: sprintf (buff,"rst  $00"); flags = DASMFLAG_STEP_OVER; break;
		case 0xc8: sprintf (buff,"ret  z"); flags = DASMFLAG_STEP_OUT; break;
		case 0xc9: sprintf (buff,"ret"); flags = DASMFLAG_STEP_OUT;  break;
		case 0xca: sprintf (buff,"jp   z,$%04x", ARGW(pc)); pc+=2;   break;
		case 0xcb: sprintf (buff,"rst  v,$40 (*)"); flags = DASMFLAG_STEP_OVER; break;
		case 0xcc: sprintf (buff,"call z,$%04x", ARGW(pc)); pc+=2; flags = DASMFLAG_STEP_OVER; break;
		case 0xcd: sprintf (buff,"call $%04x", ARGW(pc)); pc+=2; flags = DASMFLAG_STEP_OVER; break;
		case 0xce: sprintf (buff,"adc  a,$%02x", ARG(pc)); pc++;     break;
		case 0xcf: sprintf (buff,"rst  $08"); flags = DASMFLAG_STEP_OVER; break;
		case 0xd0: sprintf (buff,"ret  nc"); flags = DASMFLAG_STEP_OUT; break;
		case 0xd1: sprintf (buff,"pop  de");                         break;
		case 0xd2: sprintf (buff,"jp   nc,$%04x", ARGW(pc)); pc+=2;  break;
		case 0xd3: sprintf (buff,"out  ($%02x),a", ARG(pc)); pc++;   break;
		case 0xd4: sprintf (buff,"call nc,$%04x", ARGW(pc)); pc+=2; flags = DASMFLAG_STEP_OVER; break;
		case 0xd5: sprintf (buff,"push de");                         break;
		case 0xd6: sprintf (buff,"sub  $%02x", ARG(pc)); pc++;       break;
		case 0xd7: sprintf (buff,"rst  $10"); flags = DASMFLAG_STEP_OVER; break;
		case 0xd8: sprintf (buff,"ret  c");                          break;
		case 0xd9: sprintf (buff,"ld   (de),hl (*)");                break;
		case 0xda: sprintf (buff,"jp   c,$%04x", ARGW(pc)); pc+=2;   break;
		case 0xdb: sprintf (buff,"in   a,($%02x)", ARG(pc)); pc++;   break;
		case 0xdc: sprintf (buff,"call c,$%04x", ARGW(pc)); pc+=2; flags = DASMFLAG_STEP_OVER; break;
		case 0xdd: sprintf (buff,"jp   nx,$%04x (*)",ARGW(pc));pc+=2;break;
		case 0xde: sprintf (buff,"sub  $%02x", ARG(pc)); pc++;       break;
		case 0xdf: sprintf (buff,"rst  $18"); flags = DASMFLAG_STEP_OVER; break;
		case 0xe0: sprintf (buff,"ret  pe");                         break;
		case 0xe1: sprintf (buff,"pop  hl");                         break;
		case 0xe2: sprintf (buff,"jp   pe,$%04x", ARGW(pc)); pc+=2;  break;
		case 0xe3: sprintf (buff,"ex   (sp),hl");                    break;
		case 0xe4: sprintf (buff,"call pe,$%04x", ARGW(pc)); pc+=2; flags = DASMFLAG_STEP_OVER; break;
		case 0xe5: sprintf (buff,"push hl");                         break;
		case 0xe6: sprintf (buff,"and  $%02x", ARG(pc)); pc++;       break;
		case 0xe7: sprintf (buff,"rst  $20"); flags = DASMFLAG_STEP_OVER; break;
		case 0xe8: sprintf (buff,"ret  po");                         break;
		case 0xe9: sprintf (buff,"jp   (hl)");                       break;
		case 0xea: sprintf (buff,"jp   po,$%04x", ARGW(pc)); pc+=2;  break;
		case 0xeb: sprintf (buff,"ex   de,hl");                      break;
		case 0xec: sprintf (buff,"call po,$%04x", ARGW(pc)); pc+=2; flags = DASMFLAG_STEP_OVER; break;
		case 0xed: sprintf (buff,"ld   hl,(de) (*)");                break;
		case 0xee: sprintf (buff,"xor  $%02x", ARG(pc)); pc++;       break;
		case 0xef: sprintf (buff,"rst  $28"); flags = DASMFLAG_STEP_OVER; break;
		case 0xf0: sprintf (buff,"ret  p");                          break;
		case 0xf1: sprintf (buff,"pop  af");                         break;
		case 0xf2: sprintf (buff,"jp   p,$%04x", ARGW(pc)); pc+=2;   break;
		case 0xf3: sprintf (buff,"di");                              break;
		case 0xf4: sprintf (buff,"cp   $%04x", ARGW(pc)); pc+=2;     break;
		case 0xf5: sprintf (buff,"push af");                         break;
		case 0xf6: sprintf (buff,"or   $%02x", ARG(pc)); pc++;       break;
		case 0xf7: sprintf (buff,"rst  $30"); flags = DASMFLAG_STEP_OVER; break;
		case 0xf8: sprintf (buff,"ret  m");                          break;
		case 0xf9: sprintf (buff,"ld   sp,hl");                      break;
		case 0xfa: sprintf (buff,"jp   m,$%04x", ARGW(pc)); pc+=2;   break;
		case 0xfb: sprintf (buff,"ei");                              break;
		case 0xfc: sprintf (buff,"call m,$%04x", ARGW(pc)); pc+=2; flags = DASMFLAG_STEP_OVER; break;
		case 0xfd: sprintf (buff,"jp   x,$%04x (*)",ARGW(pc));pc+=2; break;
		case 0xfe: sprintf (buff,"cp   $%02x", ARG(pc)); pc++;       break;
		case 0xff: sprintf (buff,"rst  $38"); flags = DASMFLAG_STEP_OVER; break;
#else
		case 0x00: sprintf (buff,"nop");                             break;
		case 0x01: sprintf (buff,"lxi  b,$%04x", ARGW(pc)); pc+=2;   break;
		case 0x02: sprintf (buff,"stax b");                          break;
		case 0x03: sprintf (buff,"inx  b");                          break;
		case 0x04: sprintf (buff,"inr  b");                          break;
		case 0x05: sprintf (buff,"dcr  b");                          break;
		case 0x06: sprintf (buff,"mvi  b,$%02x", ARG(pc)); pc++;     break;
		case 0x07: sprintf (buff,"rlc");                             break;
		case 0x08: sprintf (buff,"dsub (*)");                        break;
		case 0x09: sprintf (buff,"dad  b");                          break;
		case 0x0a: sprintf (buff,"ldax b");                          break;
		case 0x0b: sprintf (buff,"dcx  b");                          break;
		case 0x0c: sprintf (buff,"inr  c");                          break;
		case 0x0d: sprintf (buff,"dcr  c");                          break;
		case 0x0e: sprintf (buff,"mvi  c,$%02x", ARG(pc)); pc++;     break;
		case 0x0f: sprintf (buff,"rrc");                             break;
		case 0x10: sprintf (buff,"asrh (*)");                        break;
		case 0x11: sprintf (buff,"lxi  d,$%04x", ARGW(pc)); pc+=2;   break;
		case 0x12: sprintf (buff,"stax d");                          break;
		case 0x13: sprintf (buff,"inx  d");                          break;
		case 0x14: sprintf (buff,"inr  d");                          break;
		case 0x15: sprintf (buff,"dcr  d");                          break;
		case 0x16: sprintf (buff,"mvi  d,$%02x", ARG(pc)); pc++;     break;
		case 0x17: sprintf (buff,"ral");                             break;
		case 0x18: sprintf (buff,"rlde (*)");                        break;
		case 0x19: sprintf (buff,"dad  d");                          break;
		case 0x1a: sprintf (buff,"ldax d");                          break;
		case 0x1b: sprintf (buff,"dcx  d");                          break;
		case 0x1c: sprintf (buff,"inr  e");                          break;
		case 0x1d: sprintf (buff,"dcr  e");                          break;
		case 0x1e: sprintf (buff,"mvi  e,$%02x", ARG(pc)); pc++;     break;
		case 0x1f: sprintf (buff,"rar");                             break;
		case 0x20: sprintf (buff,"rim");                             break;
		case 0x21: sprintf (buff,"lxi  h,$%04x", ARGW(pc)); pc+=2;   break;
		case 0x22: sprintf (buff,"shld $%04x", ARGW(pc)); pc+=2;     break;
		case 0x23: sprintf (buff,"inx  h");                          break;
		case 0x24: sprintf (buff,"inr  h");                          break;
		case 0x25: sprintf (buff,"dcr  h");                          break;
		case 0x26: sprintf (buff,"mvi  h,$%02x", ARG(pc)); pc++;     break;
		case 0x27: sprintf (buff,"daa");                             break;
		case 0x28: sprintf (buff,"ldeh $%02x (*)", ARG(pc)); pc++;   break;
		case 0x29: sprintf (buff,"dad  h");                          break;
		case 0x2a: sprintf (buff,"lhld $%04x", ARGW(pc)); pc+=2;     break;
		case 0x2b: sprintf (buff,"dcx  h");                          break;
		case 0x2c: sprintf (buff,"inr  l");                          break;
		case 0x2d: sprintf (buff,"dcr  l");                          break;
		case 0x2e: sprintf (buff,"mvi  l,$%02x", ARG(pc)); pc++;     break;
		case 0x2f: sprintf (buff,"cma");                             break;
		case 0x30: sprintf (buff,"sim");                             break;
		case 0x31: sprintf (buff,"lxi  sp,$%04x", ARGW(pc)); pc+=2;  break;
		case 0x32: sprintf (buff,"stax $%04x", ARGW(pc)); pc+=2;     break;
		case 0x33: sprintf (buff,"inx  sp");                         break;
		case 0x34: sprintf (buff,"inr  m");                          break;
		case 0x35: sprintf (buff,"dcr  m");                          break;
		case 0x36: sprintf (buff,"mvi  m,$%02x", ARG(pc)); pc++;     break;
		case 0x37: sprintf (buff,"stc");                             break;
		case 0x38: sprintf (buff,"ldes $%02x", ARG(pc)); pc++;       break;
		case 0x39: sprintf (buff,"dad sp");                          break;
		case 0x3a: sprintf (buff,"ldax $%04x", ARGW(pc)); pc+=2;     break;
		case 0x3b: sprintf (buff,"dcx  sp");                         break;
		case 0x3c: sprintf (buff,"inr  a");                          break;
		case 0x3d: sprintf (buff,"dcr  a");                          break;
		case 0x3e: sprintf (buff,"mvi  a,$%02x", ARG(pc)); pc++;     break;
		case 0x3f: sprintf (buff,"cmf");                             break;
		case 0x40: sprintf (buff,"mov  b,b");                        break;
		case 0x41: sprintf (buff,"mov  b,c");                        break;
		case 0x42: sprintf (buff,"mov  b,d");                        break;
		case 0x43: sprintf (buff,"mov  b,e");                        break;
		case 0x44: sprintf (buff,"mov  b,h");                        break;
		case 0x45: sprintf (buff,"mov  b,l");                        break;
		case 0x46: sprintf (buff,"mov  b,m");                        break;
		case 0x47: sprintf (buff,"mov  b,a");                        break;
		case 0x48: sprintf (buff,"mov  c,b");                        break;
		case 0x49: sprintf (buff,"mov  c,c");                        break;
		case 0x4a: sprintf (buff,"mov  c,d");                        break;
		case 0x4b: sprintf (buff,"mov  c,e");                        break;
		case 0x4c: sprintf (buff,"mov  c,h");                        break;
		case 0x4d: sprintf (buff,"mov  c,l");                        break;
		case 0x4e: sprintf (buff,"mov  c,m");                        break;
		case 0x4f: sprintf (buff,"mov  c,a");                        break;
		case 0x50: sprintf (buff,"mov  d,b");                        break;
		case 0x51: sprintf (buff,"mov  d,c");                        break;
		case 0x52: sprintf (buff,"mov  d,d");                        break;
		case 0x53: sprintf (buff,"mov  d,e");                        break;
		case 0x54: sprintf (buff,"mov  d,h");                        break;
		case 0x55: sprintf (buff,"mov  d,l");                        break;
		case 0x56: sprintf (buff,"mov  d,m");                        break;
		case 0x57: sprintf (buff,"mov  d,a");                        break;
		case 0x58: sprintf (buff,"mov  e,b");                        break;
		case 0x59: sprintf (buff,"mov  e,c");                        break;
		case 0x5a: sprintf (buff,"mov  e,d");                        break;
		case 0x5b: sprintf (buff,"mov  e,e");                        break;
		case 0x5c: sprintf (buff,"mov  e,h");                        break;
		case 0x5d: sprintf (buff,"mov  e,l");                        break;
		case 0x5e: sprintf (buff,"mov  e,m");                        break;
		case 0x5f: sprintf (buff,"mov  e,a");                        break;
		case 0x60: sprintf (buff,"mov  h,b");                        break;
		case 0x61: sprintf (buff,"mov  h,c");                        break;
		case 0x62: sprintf (buff,"mov  h,d");                        break;
		case 0x63: sprintf (buff,"mov  h,e");                        break;
		case 0x64: sprintf (buff,"mov  h,h");                        break;
		case 0x65: sprintf (buff,"mov  h,l");                        break;
		case 0x66: sprintf (buff,"mov  h,m");                        break;
		case 0x67: sprintf (buff,"mov  h,a");                        break;
		case 0x68: sprintf (buff,"mov  l,b");                        break;
		case 0x69: sprintf (buff,"mov  l,c");                        break;
		case 0x6a: sprintf (buff,"mov  l,d");                        break;
		case 0x6b: sprintf (buff,"mov  l,e");                        break;
		case 0x6c: sprintf (buff,"mov  l,h");                        break;
		case 0x6d: sprintf (buff,"mov  l,l");                        break;
		case 0x6e: sprintf (buff,"mov  l,m");                        break;
		case 0x6f: sprintf (buff,"mov  l,a");                        break;
		case 0x70: sprintf (buff,"mov  m,b");                        break;
		case 0x71: sprintf (buff,"mov  m,c");                        break;
		case 0x72: sprintf (buff,"mov  m,d");                        break;
		case 0x73: sprintf (buff,"mov  m,e");                        break;
		case 0x74: sprintf (buff,"mov  m,h");                        break;
		case 0x75: sprintf (buff,"mov  m,l");                        break;
		case 0x76: sprintf (buff,"mov  m,m");                        break;
		case 0x77: sprintf (buff,"mov  m,a");                        break;
		case 0x78: sprintf (buff,"mov  a,b");                        break;
		case 0x79: sprintf (buff,"mov  a,c");                        break;
		case 0x7a: sprintf (buff,"mov  a,d");                        break;
		case 0x7b: sprintf (buff,"mov  a,e");                        break;
		case 0x7c: sprintf (buff,"mov  a,h");                        break;
		case 0x7d: sprintf (buff,"mov  a,l");                        break;
		case 0x7e: sprintf (buff,"mov  a,m");                        break;
		case 0x7f: sprintf (buff,"mov  a,a");                        break;
		case 0x80: sprintf (buff,"add  b");                          break;
		case 0x81: sprintf (buff,"add  c");                          break;
		case 0x82: sprintf (buff,"add  d");                          break;
		case 0x83: sprintf (buff,"add  e");                          break;
		case 0x84: sprintf (buff,"add  h");                          break;
		case 0x85: sprintf (buff,"add  l");                          break;
		case 0x86: sprintf (buff,"add  m");                          break;
		case 0x87: sprintf (buff,"add  a");                          break;
		case 0x88: sprintf (buff,"adc  b");                          break;
		case 0x89: sprintf (buff,"adc  c");                          break;
		case 0x8a: sprintf (buff,"adc  d");                          break;
		case 0x8b: sprintf (buff,"adc  e");                          break;
		case 0x8c: sprintf (buff,"adc  h");                          break;
		case 0x8d: sprintf (buff,"adc  l");                          break;
		case 0x8e: sprintf (buff,"adc  m");                          break;
		case 0x8f: sprintf (buff,"adc  a");                          break;
		case 0x90: sprintf (buff,"sub  b");                          break;
		case 0x91: sprintf (buff,"sub  c");                          break;
		case 0x92: sprintf (buff,"sub  d");                          break;
		case 0x93: sprintf (buff,"sub  e");                          break;
		case 0x94: sprintf (buff,"sub  h");                          break;
		case 0x95: sprintf (buff,"sub  l");                          break;
		case 0x96: sprintf (buff,"sub  m");                          break;
		case 0x97: sprintf (buff,"sub  a");                          break;
		case 0x98: sprintf (buff,"sbb  b");                          break;
		case 0x99: sprintf (buff,"sbb  c");                          break;
		case 0x9a: sprintf (buff,"sbb  d");                          break;
		case 0x9b: sprintf (buff,"sbb  e");                          break;
		case 0x9c: sprintf (buff,"sbb  h");                          break;
		case 0x9d: sprintf (buff,"sbb  l");                          break;
		case 0x9e: sprintf (buff,"sbb  m");                          break;
		case 0x9f: sprintf (buff,"sbb  a");                          break;
		case 0xa0: sprintf (buff,"ana  b");                          break;
		case 0xa1: sprintf (buff,"ana  c");                          break;
		case 0xa2: sprintf (buff,"ana  d");                          break;
		case 0xa3: sprintf (buff,"ana  e");                          break;
		case 0xa4: sprintf (buff,"ana  h");                          break;
		case 0xa5: sprintf (buff,"ana  l");                          break;
		case 0xa6: sprintf (buff,"ana  m");                          break;
		case 0xa7: sprintf (buff,"ana  a");                          break;
		case 0xa8: sprintf (buff,"xra  b");                          break;
		case 0xa9: sprintf (buff,"xra  c");                          break;
		case 0xaa: sprintf (buff,"xra  d");                          break;
		case 0xab: sprintf (buff,"xra  e");                          break;
		case 0xac: sprintf (buff,"xra  h");                          break;
		case 0xad: sprintf (buff,"xra  l");                          break;
		case 0xae: sprintf (buff,"xra  m");                          break;
		case 0xaf: sprintf (buff,"xra  a");                          break;
		case 0xb0: sprintf (buff,"ora  b");                          break;
		case 0xb1: sprintf (buff,"ora  c");                          break;
		case 0xb2: sprintf (buff,"ora  d");                          break;
		case 0xb3: sprintf (buff,"ora  e");                          break;
		case 0xb4: sprintf (buff,"ora  h");                          break;
		case 0xb5: sprintf (buff,"ora  l");                          break;
		case 0xb6: sprintf (buff,"ora  m");                          break;
		case 0xb7: sprintf (buff,"ora  a");                          break;
		case 0xb8: sprintf (buff,"cmp  b");                          break;
		case 0xb9: sprintf (buff,"cmp  c");                          break;
		case 0xba: sprintf (buff,"cmp  d");                          break;
		case 0xbb: sprintf (buff,"cmp  e");                          break;
		case 0xbc: sprintf (buff,"cmp  h");                          break;
		case 0xbd: sprintf (buff,"cmp  l");                          break;
		case 0xbe: sprintf (buff,"cmp  m");                          break;
		case 0xbf: sprintf (buff,"cmp  a");                          break;
		case 0xc0: sprintf (buff,"rnz"); flags = DASMFLAG_STEP_OUT;  break;
		case 0xc1: sprintf (buff,"pop  b");                          break;
		case 0xc2: sprintf (buff,"jnz  $%04x", ARGW(pc)); pc+=2;     break;
		case 0xc3: sprintf (buff,"jmp  $%04x", ARGW(pc)); pc+=2;     break;
		case 0xc4: sprintf (buff,"cnz  $%04x", ARGW(pc)); pc+=2; flags = DASMFLAG_STEP_OVER; break;
		case 0xc5: sprintf (buff,"push b");                          break;
		case 0xc6: sprintf (buff,"adi  $%02x", ARG(pc)); pc++;       break;
		case 0xc7: sprintf (buff,"rst  0"); flags = DASMFLAG_STEP_OVER; break;
		case 0xc8: sprintf (buff,"rz"); flags = DASMFLAG_STEP_OUT;   break;
		case 0xc9: sprintf (buff,"ret"); flags = DASMFLAG_STEP_OUT;  break;
		case 0xca: sprintf (buff,"jz   $%04x", ARGW(pc)); pc+=2;     break;
		case 0xcb: sprintf (buff,"rstv 8 (*)"); flags = DASMFLAG_STEP_OVER; break;
		case 0xcc: sprintf (buff,"cz   $%04x", ARGW(pc)); pc+=2; flags = DASMFLAG_STEP_OVER; break;
		case 0xcd: sprintf (buff,"call $%04x", ARGW(pc)); pc+=2; flags = DASMFLAG_STEP_OVER; break;
		case 0xce: sprintf (buff,"aci  $%02x", ARG(pc)); pc++;       break;
		case 0xcf: sprintf (buff,"rst  1"); flags = DASMFLAG_STEP_OVER; break;
		case 0xd0: sprintf (buff,"rnc"); flags = DASMFLAG_STEP_OUT;  break;
		case 0xd1: sprintf (buff,"pop  d");                          break;
		case 0xd2: sprintf (buff,"jnc  $%04x", ARGW(pc)); pc+=2;     break;
		case 0xd3: sprintf (buff,"out  $%02x", ARG(pc)); pc++;       break;
		case 0xd4: sprintf (buff,"cnc  $%04x", ARGW(pc)); pc+=2; flags = DASMFLAG_STEP_OVER; break;
		case 0xd5: sprintf (buff,"push d");                          break;
		case 0xd6: sprintf (buff,"sui  $%02x", ARG(pc)); pc++;       break;
		case 0xd7: sprintf (buff,"rst  2"); flags = DASMFLAG_STEP_OVER; break;
		case 0xd8: sprintf (buff,"rc"); flags = DASMFLAG_STEP_OUT;   break;
		case 0xd9: sprintf (buff,"shlx d (*)");                      break;
		case 0xda: sprintf (buff,"jc   $%04x", ARGW(pc)); pc+=2;     break;
		case 0xdb: sprintf (buff,"in   $%02x", ARG(pc)); pc++;       break;
		case 0xdc: sprintf (buff,"cc   $%04x", ARGW(pc)); pc+=2; flags = DASMFLAG_STEP_OVER; break;
		case 0xdd: sprintf (buff,"jnx  $%04x (*)", ARGW(pc)); pc+=2; break;
		case 0xde: sprintf (buff,"sbi  $%02x", ARG(pc)); pc++;       break;
		case 0xdf: sprintf (buff,"rst  3"); flags = DASMFLAG_STEP_OVER; break;
		case 0xe0: sprintf (buff,"rpo"); flags = DASMFLAG_STEP_OUT;  break;
		case 0xe1: sprintf (buff,"pop  h");                          break;
		case 0xe2: sprintf (buff,"jpo  $%04x", ARGW(pc)); pc+=2;     break;
		case 0xe3: sprintf (buff,"xthl");                            break;
		case 0xe4: sprintf (buff,"cpo  $%04x", ARGW(pc)); pc+=2; flags = DASMFLAG_STEP_OVER; break;
		case 0xe5: sprintf (buff,"push h");                          break;
		case 0xe6: sprintf (buff,"ani  $%02x", ARG(pc)); pc++;       break;
		case 0xe7: sprintf (buff,"rst  4"); flags = DASMFLAG_STEP_OVER; break;
		case 0xe8: sprintf (buff,"rpe"); flags = DASMFLAG_STEP_OUT;  break;
		case 0xe9: sprintf (buff,"pchl");                            break;
		case 0xea: sprintf (buff,"jpe  $%04x", ARGW(pc)); pc+=2;     break;
		case 0xeb: sprintf (buff,"xchg");                            break;
		case 0xec: sprintf (buff,"cpe  $%04x", ARGW(pc)); pc+=2; flags = DASMFLAG_STEP_OVER; break;
		case 0xed: sprintf (buff,"lhlx d (*)");                      break;
		case 0xee: sprintf (buff,"xri  $%02x", ARG(pc)); pc++;       break;
		case 0xef: sprintf (buff,"rst  5"); flags = DASMFLAG_STEP_OVER; break;
		case 0xf0: sprintf (buff,"rp"); flags = DASMFLAG_STEP_OUT;   break;
		case 0xf1: sprintf (buff,"pop  a");                          break;
		case 0xf2: sprintf (buff,"jp   $%04x", ARGW(pc)); pc+=2;     break;
		case 0xf3: sprintf (buff,"di");                              break;
		case 0xf4: sprintf (buff,"cp   $%04x", ARGW(pc)); pc+=2;     break;
		case 0xf5: sprintf (buff,"push a");                          break;
		case 0xf6: sprintf (buff,"ori  $%02x", ARG(pc)); pc++;       break;
		case 0xf7: sprintf (buff,"rst  6"); flags = DASMFLAG_STEP_OVER; break;
		case 0xf8: sprintf (buff,"rm"); flags = DASMFLAG_STEP_OUT;   break;
		case 0xf9: sprintf (buff,"sphl");                            break;
		case 0xfa: sprintf (buff,"jm   $%04x", ARGW(pc)); pc+=2;     break;
		case 0xfb: sprintf (buff,"ei");                              break;
		case 0xfc: sprintf (buff,"cm   $%04x", ARGW(pc)); pc+=2; flags = DASMFLAG_STEP_OVER; break;
		case 0xfd: sprintf (buff,"jx   $%04x (*)", ARGW(pc)); pc+=2; break;
		case 0xfe: sprintf (buff,"cpi  $%02x", ARG(pc)); pc++;       break;
		case 0xff: sprintf (buff,"rst  7"); flags = DASMFLAG_STEP_OVER; break;
#endif
	}
	return (pc - PC) | flags | DASMFLAG_SUPPORTED;
}

