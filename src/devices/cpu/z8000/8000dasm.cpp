// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller,Ernesto Corvi
/*****************************************************************************
 *
 *   8000dasm.c
 *   Portable Z8000(2) emulator
 *   Z8000 disassembler; requires the z8000_exec table to be initialized
 *
 *****************************************************************************/

#include "emu.h"
#include "z8000.h"
#include "z8000cpu.h"

#include "debugger.h"
#include "debug/debugvw.h"
#include "debug/debugcon.h"


static int n[16];   /* opcode nibbles */
static int b[8];    /* opcode bytes */
static int w[4];    /* opcode words */

static void GET_OP(const uint8_t *oprom, int i, unsigned offset)
{
	uint16_t opcode = (oprom[offset] << 8) | oprom[offset + 1];
	w[i] = opcode;
	b[i*2+0] = opcode >> 8;
	b[i*2+1] = opcode & 0xff;
	n[i*4+0] = (opcode >> 12) & 0x0f;
	n[i*4+1] = (opcode >> 8) & 0x0f;
	n[i*4+2] = (opcode >> 4) & 0x0f;
	n[i*4+3] = opcode & 0x0f;
}

static const char *const cc[16] = {
	"n",   "lt",  "le",  "ule",  "pe/ov",   "mi",  "eq/z",   "c/ult",
	"a",   "ge",  "gt",  "ugt",  "po/nov",  "pl",  "ne/nz",  "nc/uge"
};

static const char *const flg[16] = {
	"",    "p/v",  "s",   "p/v,s",   "z",   "p/v,z",  "s,z",  "p/v,s,z",
	"c",   "p/v,c","s,c", "p/v,s,c", "z,c", "p/v,z,c","s,z,c","p/v,s,z,c"
};

static const char *const ints[4] = {
	"",    "vi",  "nvi",   "vi,nvi"
};

int z8k_segm;                               /* Current disassembler mode: 0 - non-segmented, 1 - segmented */
int z8k_segm_mode = Z8K_SEGM_MODE_AUTO;     /* User disassembler mode setting: segmented, non-segmented, auto */

CPU_DISASSEMBLE(z8000)
{
	int new_pc = pc, i, j, tmp;
	const char *src;
	z8002_device::Z8000_dasm o;
	uint32_t flags = 0;
	uint32_t old_w;

	z8002_device::init_tables();

	GET_OP(oprom, 0, new_pc - pc);
	new_pc += 2;
	switch (pc)
	{
		case 0x0000:
			util::stream_format(stream, ".word   #%%%04x ;RST", w[0]);
			break;
		case 0x0002:
			util::stream_format(stream, ".word   #%%%04x ;RST FCW", w[0]);
			break;
		case 0x0004:
			util::stream_format(stream, ".word   #%%%04x ;RST PC", w[0]);
			break;
		default:
			o = z8002_device::dasm(w[0]);
			if (o.size > 1) { GET_OP(oprom, 1, new_pc - pc); new_pc += 2; }
			if (o.size > 2) { GET_OP(oprom, 2, new_pc - pc); new_pc += 2; }
			src = o.dasm;
			flags = o.flags;

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
						util::stream_format(stream, "%d", n[i]);
						break;
					case '#':
						/* immediate */
						src++;
						switch (*src++) {
							case 'b': /* imm8 (byte) */
								i = *src++ - '0';
								util::stream_format(stream, "#%%%02x", b[i]);
								break;
							case 'w': /* imm16 (word) */
								i = *src++ - '0';
								util::stream_format(stream, "#%%%04x", w[i]);
								break;
							case 'l': /* imm32 (long) */
								i = *src++ - '0';
								util::stream_format(stream, "#%%%04x%04x", w[i], w[i+1]);
								break;
						}
						break;
					case '$':
						/* absolute immediate 8bit (rl/rr) */
						src++;
						i = *src++ - '0';
						util::stream_format(stream, "#%d", ((int8_t)b[i]<0) ? -(int8_t)b[i] : b[i]);
						break;
					case '+':
						/* imm4m1 (inc/dec value) */
						src++;
						i = *src++ - '0';
						util::stream_format(stream, "%i", n[i] + 1);
						break;
					case '*':
						/* left/right (rotate/shift) */
						src++;
						util::stream_format(stream, "%c", b[2] ? 'r' : 'l');
						break;
					case '?':
						/* imm1or2 (shift/rotate once or twice) */
						src++;
						i = *src++ - '0';
						util::stream_format(stream, "%c", (n[i] & 2) ? '2' : '1');
						break;
					case 'R':
						src++;
						//tmp = ((n[1] & 0x01) << 16) + (n[3] << 8) + (n[7] & 0x08);
						tmp = ((n[1] & 0x01) << 8) + (n[3] << 4) + (n[7] & 0x08);
						switch (tmp)
						{
							case 0x000: util::stream_format(stream, "inirb "); flags = DASMFLAG_STEP_OVER; break;
							case 0x008: util::stream_format(stream, "inib  "); break;
							case 0x010: util::stream_format(stream, "sinirb"); flags = DASMFLAG_STEP_OVER; break;
							case 0x018: util::stream_format(stream, "sinib "); break;
							case 0x020: util::stream_format(stream, "otirb "); flags = DASMFLAG_STEP_OVER; break;
							case 0x028: util::stream_format(stream, "outib "); break;
							case 0x030: util::stream_format(stream, "soutib"); break;
							case 0x038: util::stream_format(stream, "sotirb"); flags = DASMFLAG_STEP_OVER; break;
							case 0x040: util::stream_format(stream, "inb   "); break;
							case 0x048: util::stream_format(stream, "inb   "); break;
							case 0x050: util::stream_format(stream, "sinb  "); break;
							case 0x058: util::stream_format(stream, "sinb  "); break;
							case 0x060: util::stream_format(stream, "outb  "); break;
							case 0x068: util::stream_format(stream, "outb  "); break;
							case 0x070: util::stream_format(stream, "soutb "); break;
							case 0x078: util::stream_format(stream, "soutb "); break;
							case 0x080: util::stream_format(stream, "indrb "); flags = DASMFLAG_STEP_OVER; break;
							case 0x088: util::stream_format(stream, "indb  "); break;
							case 0x090: util::stream_format(stream, "sindrb"); flags = DASMFLAG_STEP_OVER; break;
							case 0x098: util::stream_format(stream, "sindb "); break;
							case 0x0a0: util::stream_format(stream, "otdrb "); flags = DASMFLAG_STEP_OVER; break;
							case 0x0a8: util::stream_format(stream, "outdb "); break;
							case 0x0b0: util::stream_format(stream, "soutdb"); break;
							case 0x0b8: util::stream_format(stream, "sotdrb"); flags = DASMFLAG_STEP_OVER; break;
							case 0x100: util::stream_format(stream, "inir  "); flags = DASMFLAG_STEP_OVER; break;
							case 0x108: util::stream_format(stream, "ini   "); break;
							case 0x110: util::stream_format(stream, "sinir "); flags = DASMFLAG_STEP_OVER; break;
							case 0x118: util::stream_format(stream, "sini  "); break;
							case 0x120: util::stream_format(stream, "otir  "); flags = DASMFLAG_STEP_OVER; break;
							case 0x128: util::stream_format(stream, "outi  "); break;
							case 0x130: util::stream_format(stream, "souti "); break;
							case 0x138: util::stream_format(stream, "sotir "); flags = DASMFLAG_STEP_OVER; break;
							case 0x140: util::stream_format(stream, "in    "); break;
							case 0x148: util::stream_format(stream, "in    "); break;
							case 0x150: util::stream_format(stream, "sin   "); break;
							case 0x158: util::stream_format(stream, "sin   "); break;
							case 0x160: util::stream_format(stream, "out   "); break;
							case 0x168: util::stream_format(stream, "out   "); break;
							case 0x170: util::stream_format(stream, "sout  "); break;
							case 0x178: util::stream_format(stream, "sout  "); break;
							case 0x180: util::stream_format(stream, "indr  "); flags = DASMFLAG_STEP_OVER; break;
							case 0x188: util::stream_format(stream, "ind   "); break;
							case 0x190: util::stream_format(stream, "sindr "); flags = DASMFLAG_STEP_OVER; break;
							case 0x198: util::stream_format(stream, "sind  "); break;
							case 0x1a0: util::stream_format(stream, "otdr  "); flags = DASMFLAG_STEP_OVER; break;
							case 0x1a8: util::stream_format(stream, "outd  "); break;
							case 0x1b0: util::stream_format(stream, "soutd "); break;
							case 0x1b8: util::stream_format(stream, "sotdr "); flags = DASMFLAG_STEP_OVER; break;
							default:
								util::stream_format(stream, "unk(0x%x)", tmp);
						}
						break;
					case 'a':
						/* address */
						src++;
						i = *src++ - '0';
						if (z8k_segm) {
							if (w[i] & 0x8000) {
								old_w = w[i];
								for (j = i; j < o.size; j++)
									w[j] = w[j + 1];
								GET_OP(oprom, o.size - 1, new_pc - pc);
								new_pc += 2;
								w[i] = ((old_w & 0x7f00) << 16) | (w[i] & 0xffff);
							}
							else {
								w[i] = ((w[i] & 0x7f00) << 16) | (w[i] & 0xff);
							}
							util::stream_format(stream, "<%%%02X>%%%04X", (w[i] >> 24) & 0xff, w[i] & 0xffff);
						}
						else util::stream_format(stream, "%%%04x", w[i]);
						break;
					case 'c':
						/* condition code */
						src++;
						i = *src++ - '0';
						if (n[i] == 8) {    /* always? */
							/* skip following comma */
							if (*src == ',')
								src++;
						}
						else util::stream_format(stream, "%s", cc[n[i]]);
						break;
					case 'd':
						/* displacement */
						src++;
						i = *src++ - '0';
						switch (i) {
							case 0: /* disp7 */
								tmp = new_pc - 2 * (w[0] & 0x7f);
								break;
							case 1: /* disp8 */
								tmp = new_pc + 2 * (int8_t)(w[0] & 0xff);
								break;
							case 2: /* disp12 */
								tmp = w[0] & 0x7ff;
								if (w[0] & 0x800)
									tmp = new_pc + 0x1000 -2 * tmp;
								else
									tmp = new_pc + -2 * tmp;
								break;
							default:
								tmp = 0;
								abort();
						}
						if (z8k_segm)
							util::stream_format(stream, "<%%%02X>%%%04X", (tmp >> 16) & 0xff, tmp & 0xffff);
						else
							util::stream_format(stream, "%%%04x", tmp);
						break;
					case 'f':
						/* flag (setflg/resflg/comflg) */
						src++;
						i = *src++ - '0';
						util::stream_format(stream, "%s", flg[n[i]]);
						break;
					case 'i':
						/* interrupts */
						src++;
						i = *src++ - '0';
						util::stream_format(stream, "%s", ints[n[i] & 3]);
						break;
					case 'n':
						/* register count for ldm */
						src++;
						util::stream_format(stream, "%d", n[7] + 1);
						break;
					case 'p':
						/* disp16 (pc relative) */
						src++;
						i = *src++ - '0';
						util::stream_format(stream, "%%%04x", new_pc + w[i]);
						break;
					case 'r':
						/* register */
						src++;
						switch (*src++) {
							case 'b':
								/* byte */
								i = *src++ - '0';
								if (n[i] & 8)
									util::stream_format(stream, "rl%d", n[i] & 7);
								else
									util::stream_format(stream, "rh%d", n[i]);
								break;
							case 'w':
								/* word */
								i = *src++ - '0';
								util::stream_format(stream, "r%d", n[i]);
								break;
							case 'l':
								/* long */
								i = *src++ - '0';
								util::stream_format(stream, "rr%d", n[i]);
								break;
							case 'q':
								/* quad word (long long) */
								i = *src++ - '0';
								util::stream_format(stream, "rq%d", n[i]);
								break;
						}
						break;
					default:
						stream << '%' << *src++;
						break;
					}
				} else stream << *src++;
			}
			break;
	}
	return (new_pc - pc) | flags | DASMFLAG_SUPPORTED;
}
