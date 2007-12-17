/*****************************************************************************
 *
 *   8000dasm.c
 *   Portable Z8000(2) emulator
 *   Z8000 disassembler; requires the z8000_exec table to be initialized
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
 *
 *****************************************************************************/

#include "z8000.h"
#include "z8000cpu.h"
#include "cpuintrf.h"

static int n[12];	/* opcode nibbles */
static int b[6];	/* opcode bytes */
static int w[3];	/* opcode words */

static void GET_OP(const UINT8 *oprom, int i, unsigned offset)
{
	UINT16 opcode = (oprom[offset] << 8) | oprom[offset + 1];
	w[i] = opcode;
	b[i*2+0] = opcode >> 8;
	b[i*2+1] = opcode & 0xff;
	n[i*4+0] = (opcode >> 12) & 0x0f;
	n[i*4+1] = (opcode >> 8) & 0x0f;
	n[i*4+2] = (opcode >> 4) & 0x0f;
	n[i*4+3] = opcode & 0x0f;
}

static const char *cc[16] = {
	"n",   "lt",  "le",  "ule",  "pe/ov",   "mi",  "eq/z",   "c/ult",
	"a",   "ge",  "gt",  "ugt",  "po/nov",  "pl",  "ne/nz",  "nc/uge"
};

static const char *flg[16] = {
	"",    "p/v",  "s",   "p/v,s",   "z",   "p/v,z",  "s,z",  "p/v,s,z",
	"c",   "p/v,c","s,c", "p/v,s,c", "z,c", "p/v,z,c","s,z,c","p/v,s,z,c"
};

static const char *ints[4] = {
	"",    "vi",  "nvi",   "vi,nvi"
};

offs_t z8000_dasm(char *buff, offs_t pc, const UINT8 *oprom, const UINT8 *opram)
{
	int new_pc = pc, i, tmp;
	char *dst = buff;
	const char *src;
	Z8000_exec *o;
	UINT32 flags = 0;

    GET_OP(oprom, 0, new_pc - pc);
	new_pc += 2;
	switch (pc)
	{
		case 0x0000:
			dst += sprintf(dst, ".word   #$%04x ;RST", w[0]);
			break;
		case 0x0002:
			dst += sprintf(dst, ".word   #$%04x ;RST FCW", w[0]);
            break;
		case 0x0004:
			dst += sprintf(dst, ".word   #$%04x ;RST PC", w[0]);
            break;
		default:
			o = &z8000_exec[w[0]];
			if (o->size > 1) { GET_OP(oprom, 1, new_pc - pc); new_pc += 2; }
			if (o->size > 2) { GET_OP(oprom, 2, new_pc - pc); new_pc += 2; }
			src = o->dasm;
			flags = o->dasmflags;

			while (*src)
			{
				if (*src == '%')
				{
					src++;
					switch (*src) {
					case '0': case '1': case '2': case '3':
					case '4': case '5': case '6': case '7':
						/* nibble number */
						i = *src++ - '0';
						dst += sprintf(dst, "%d", n[i]);
						break;
					case '#':
						/* immediate */
						src++;
						switch (*src++) {
							case 'b': /* imm8 (byte) */
								i = *src++ - '0';
								dst += sprintf(dst, "#$%02x", b[i]);
								break;
							case 'w': /* imm16 (word) */
								i = *src++ - '0';
								dst += sprintf(dst, "#$%04x", w[i]);
								break;
							case 'l': /* imm32 (long) */
								i = *src++ - '0';
								dst += sprintf(dst, "#$%04x%04x", w[i], w[i+1]);
								break;
						}
						break;
					case '$':
						/* absolute immediate 8bit (rl/rr) */
						src++;
						i = *src++ - '0';
						dst += sprintf(dst, "#%d", ((INT8)b[i]<0) ? -(INT8)b[i] : b[i]);
						break;
					case '+':
						/* imm4m1 (inc/dec value) */
						src++;
						i = *src++ - '0';
						dst += sprintf(dst, "%i", n[i] + 1);
						break;
					case '*':
						/* left/right (rotate/shift) */
						src++;
						dst += sprintf(dst, "%c", b[2] ? 'r' : 'l');
						break;
					case '?':
						/* imm1or2 (shift/rotate once or twice) */
						src++;
						i = *src++ - '0';
						dst += sprintf(dst, "%c", (n[i] & 2) ? '2' : '1');
						break;
					case 'R':
						src++;
						tmp = ((n[1] & 0x01) << 16) + (n[3] << 8) + (n[7] & 0x08);
						switch (tmp)
						{
							case 0x000: dst += sprintf(dst, "inirb "); flags = DASMFLAG_STEP_OVER; break;
							case 0x008: dst += sprintf(dst, "inib  "); break;
							case 0x010: dst += sprintf(dst, "sinirb"); flags = DASMFLAG_STEP_OVER; break;
							case 0x018: dst += sprintf(dst, "sinib "); break;
							case 0x020: dst += sprintf(dst, "otirb "); flags = DASMFLAG_STEP_OVER; break;
							case 0x028: dst += sprintf(dst, "outib "); break;
							case 0x030: dst += sprintf(dst, "soutib"); break;
							case 0x038: dst += sprintf(dst, "sotirb"); flags = DASMFLAG_STEP_OVER; break;
							case 0x040: dst += sprintf(dst, "inb   "); break;
							case 0x048: dst += sprintf(dst, "inb   "); break;
							case 0x050: dst += sprintf(dst, "sinb  "); break;
							case 0x058: dst += sprintf(dst, "sinb  "); break;
							case 0x060: dst += sprintf(dst, "outb  "); break;
							case 0x068: dst += sprintf(dst, "outb  "); break;
							case 0x070: dst += sprintf(dst, "soutb "); break;
							case 0x078: dst += sprintf(dst, "soutb "); break;
							case 0x080: dst += sprintf(dst, "indrb "); flags = DASMFLAG_STEP_OVER; break;
							case 0x088: dst += sprintf(dst, "indb  "); break;
							case 0x090: dst += sprintf(dst, "sindrb"); flags = DASMFLAG_STEP_OVER; break;
							case 0x098: dst += sprintf(dst, "sindb "); break;
							case 0x0a0: dst += sprintf(dst, "otdrb "); flags = DASMFLAG_STEP_OVER; break;
							case 0x0a8: dst += sprintf(dst, "outdb "); break;
							case 0x0b0: dst += sprintf(dst, "soutdb"); break;
							case 0x0b8: dst += sprintf(dst, "sotdrb"); flags = DASMFLAG_STEP_OVER; break;
							case 0x100: dst += sprintf(dst, "inir  "); flags = DASMFLAG_STEP_OVER; break;
							case 0x108: dst += sprintf(dst, "ini   "); break;
							case 0x110: dst += sprintf(dst, "sinir "); flags = DASMFLAG_STEP_OVER; break;
							case 0x118: dst += sprintf(dst, "sini  "); break;
							case 0x120: dst += sprintf(dst, "otir  "); flags = DASMFLAG_STEP_OVER; break;
							case 0x128: dst += sprintf(dst, "outi  "); break;
							case 0x130: dst += sprintf(dst, "souti "); break;
							case 0x138: dst += sprintf(dst, "sotir "); flags = DASMFLAG_STEP_OVER; break;
							case 0x140: dst += sprintf(dst, "in    "); break;
							case 0x148: dst += sprintf(dst, "in    "); break;
							case 0x150: dst += sprintf(dst, "sin   "); break;
							case 0x158: dst += sprintf(dst, "sin   "); break;
							case 0x160: dst += sprintf(dst, "out   "); break;
							case 0x168: dst += sprintf(dst, "out   "); break;
							case 0x170: dst += sprintf(dst, "sout  "); break;
							case 0x178: dst += sprintf(dst, "sout  "); break;
							case 0x180: dst += sprintf(dst, "indr  "); flags = DASMFLAG_STEP_OVER; break;
							case 0x188: dst += sprintf(dst, "ind   "); break;
							case 0x190: dst += sprintf(dst, "sindr "); flags = DASMFLAG_STEP_OVER; break;
							case 0x198: dst += sprintf(dst, "sind  "); break;
							case 0x1a0: dst += sprintf(dst, "otdr  "); flags = DASMFLAG_STEP_OVER; break;
							case 0x1a8: dst += sprintf(dst, "outd  "); break;
							case 0x1b0: dst += sprintf(dst, "soutd "); break;
							case 0x1b8: dst += sprintf(dst, "sotdr "); flags = DASMFLAG_STEP_OVER; break;
                            default:
								dst += sprintf(dst, "??????");
						}
                        break;
                    case 'a':
						/* address */
						src++;
						i = *src++ - '0';
						dst += sprintf(dst, "$%04x", w[i]);
						break;
					case 'c':
						/* condition code */
						src++;
						i = *src++ - '0';
						if (n[i] == 8) {	/* always? */
							/* skip following comma */
							if (*src == ',')
								src++;
						}
						else dst += sprintf(dst, "%s", cc[n[i]]);
						break;
					case 'd':
						/* displacement */
						src++;
						i = *src++ - '0';
						switch (i) {
							case 0: /* disp7 */
								tmp = new_pc - 2 * (w[0] & 0x7f);
								dst += sprintf(dst, "#$%04x", tmp);
								break;
							case 1: /* disp8 */
								tmp = new_pc + 2 * (INT8)(w[0] & 0xff);
								dst += sprintf(dst, "#$%04x", tmp);
								break;
							case 2: /* disp12 */
								tmp = w[0] & 0x7ff;
								if (w[0] & 0x800)
									tmp = new_pc + 0x1000 -2 * tmp;
								else
									tmp = new_pc + -2 * tmp;
								dst += sprintf(dst, "#$%04x", tmp);
								break;
						}
						break;
					case 'f':
						/* flag (setflg/resflg/comflg) */
						src++;
						i = *src++ - '0';
						dst += sprintf(dst, "%s", flg[n[i]]);
						break;
					case 'i':
						/* interrupts */
						src++;
						i = *src++ - '0';
						dst += sprintf(dst, "%s", ints[n[i] & 3]);
						break;
					case 'p':
						/* disp16 (pc relative) */
						src++;
						i = *src++ - '0';
						dst += sprintf(dst, "$%04x", new_pc + w[i]);
						break;
					case 'r':
						/* register */
						src++;
						switch (*src++) {
							case 'b':
								/* byte */
								i = *src++ - '0';
								if (n[i] & 8)
									dst += sprintf(dst, "rl%d", n[i] & 7);
								else
									dst += sprintf(dst, "rh%d", n[i]);
								break;
							case 'w':
								/* word */
								i = *src++ - '0';
								dst += sprintf(dst, "r%d", n[i]);
								break;
							case 'l':
								/* long */
								i = *src++ - '0';
								dst += sprintf(dst, "rr%d", n[i]);
								break;
							case 'q':
								/* quad word (long long) */
								i = *src++ - '0';
								dst += sprintf(dst, "rq%d", n[i]);
								break;
						}
						break;
					default:
						*dst++ = '%';
						*dst++ = *src++;
						break;
					}
				} else *dst++ = *src++;
			}
			*dst = '\0';
			break;
	}
	return (new_pc - pc) | flags | DASMFLAG_SUPPORTED;
}

