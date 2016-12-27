// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#include "emu.h"

enum
{
	ADDR_MODE_PC32 = 0x10,
	ADDR_MODE_ABS32 = 0x30,
	ADDR_MODE_REL32 = 0x60,
	ADDR_MODE_PC16 = 0x90,
	ADDR_MODE_REL12 = 0xa0,
	ADDR_MODE_ABS16 = 0xb0,
	ADDR_MODE_PCX = 0xd0,
	ADDR_MODE_RELX = 0xe0
};

#define R1 ((insn[0] & 0x00f0) >> 4)
#define R2 (insn[0] & 0x000f)

#define I16 ((int16_t)insn[1])
#define I32 (*(int32_t *)&insn[1])
#define IMM_VALUE (insn[0] & 0x0080 ? I16 : I32)
#define IMM_SIZE (insn[0] & 0x0080 ? 2 : 4)

#define ADDR_MODE (insn[0] & 0x00f0)
#define ADDR_R2 ((insn[0] & 0x0050) == 0x0010 ? (insn[0] & 0x000f) : (insn[1] & 0x000f))
#define ADDR_SIZE (ADDR_MODE > ADDR_MODE_REL32 ? 2 : ADDR_MODE == ADDR_MODE_REL32 ? 6 : 4)
#define ADDR_RX ((insn[1] & 0xf0) >> 4)
#define ADDR_I12 (((int16_t)insn[1]) >> 4)

static const char *const cc[] =
{
	"",
	"clt",
	"cle",
	"ceq",
	"cgt",
	"cge",
	"cne",
	"cltu",
	"cleu",
	"cgtu",
	"cgeu",

	"v",
	"nv",
	"n",
	"nn",
	"fn"
};

char *address (offs_t pc, uint16_t *insn)
{
	static char buffer[32];

	switch (ADDR_MODE)
	{
	case ADDR_MODE_PC32: sprintf(buffer, "0x%x", pc + I32); break;
	case ADDR_MODE_ABS32: sprintf(buffer, "0x%x", I32); break;
	case ADDR_MODE_REL32: sprintf(buffer, "%d(r%d)", *(int32_t *)&insn[2], R2); break;
	case ADDR_MODE_PC16: sprintf(buffer, "0x%x", pc + I16); break;
	case ADDR_MODE_REL12: sprintf(buffer, "%d(r%d)", ADDR_I12, R2); break;
	case ADDR_MODE_ABS16: sprintf(buffer, "0x%x", I16); break;
	case ADDR_MODE_PCX: sprintf(buffer, "[r%d](pc)", ADDR_RX); break;
	case ADDR_MODE_RELX: sprintf(buffer, "[r%d](r%d)", ADDR_RX, R2); break;
	default: sprintf(buffer, "ERROR"); break;
	}

	return buffer;
}

CPU_DISASSEMBLE(clipper)
{
	uint16_t *insn = (uint16_t *)oprom;
	uint32_t flags = DASMFLAG_SUPPORTED;
	offs_t bytes;

	// TODO: substitute for 'fp' and 'sp' register names?

	switch (insn[0] >> 8)
	{
	case 0x00: 
		if (oprom[0] == 0)
			util::stream_format(stream, "noop");
		else
			util::stream_format(stream, "noop $%d", oprom[0]);
		bytes = 2; 
		break;

	case 0x10: util::stream_format(stream, "movwp   r%d,%s", R2, R1 == 0 ? "psw" : R1 == 1 ? "ssw" : "sswf"); bytes = 2; break;
	case 0x11: util::stream_format(stream, "movpw   %s,r%d", R1 == 0 ? "psw" : "ssw", R2); bytes = 2; break; 
	case 0x12: util::stream_format(stream, "calls   $%d", insn[0] & 0x7F); bytes = 2; flags |= DASMFLAG_STEP_OVER; break;
	case 0x13: util::stream_format(stream, "ret     r%d", R2); bytes = 2; flags |= DASMFLAG_STEP_OUT; break;
	case 0x14: util::stream_format(stream, "pushw   r%d,r%d", R2, R1); bytes = 2; break;

	case 0x16: util::stream_format(stream, "popw    r%d,r%d", R1, R2); bytes = 2; break;

	case 0x20: util::stream_format(stream, "adds    f%d,f%d", R1, R2); bytes = 2; break;
	case 0x21: util::stream_format(stream, "subs    f%d,f%d", R1, R2); bytes = 2; break;
	case 0x22: util::stream_format(stream, "addd    f%d,f%d", R1, R2); bytes = 2; break;
	case 0x23: util::stream_format(stream, "subd    f%d,f%d", R1, R2); bytes = 2; break;
	case 0x24: util::stream_format(stream, "movs    f%d,f%d", R1, R2); bytes = 2; break;
	case 0x25: util::stream_format(stream, "cmps    f%d,f%d", R1, R2); bytes = 2; break;
	case 0x26: util::stream_format(stream, "movd    f%d,f%d", R1, R2); bytes = 2; break;
	case 0x27: util::stream_format(stream, "cmpd    f%d,f%d", R1, R2); bytes = 2; break;
	case 0x28: util::stream_format(stream, "muls    f%d,f%d", R1, R2); bytes = 2; break;
	case 0x29: util::stream_format(stream, "divs    f%d,f%d", R1, R2); bytes = 2; break;
	case 0x2a: util::stream_format(stream, "muld    f%d,f%d", R1, R2); bytes = 2; break;
	case 0x2b: util::stream_format(stream, "divd    f%d,f%d", R1, R2); bytes = 2; break;
	case 0x2c: util::stream_format(stream, "movsw   f%d,r%d", R1, R2); bytes = 2; break;
	case 0x2d: util::stream_format(stream, "movws   r%d,f%d", R1, R2); bytes = 2; break;
	case 0x2e: util::stream_format(stream, "movdl   f%d,r%d:%d", R1, R2 + 0, R2 + 1); bytes = 2; break;
	case 0x2f: util::stream_format(stream, "movld   r%d:r%d,f%d", R1 + 0, R1 + 1, R2); bytes = 2; break;

	case 0x30: util::stream_format(stream, "shaw    r%d,r%d", R1, R2); bytes = 2; break;
	case 0x31: util::stream_format(stream, "shal    r%d,r%d:r%d", R1, R2 + 0, R2 + 1); bytes = 2; break;
	case 0x32: util::stream_format(stream, "shlw    r%d,r%d", R1, R2); bytes = 2; break;
	case 0x33: util::stream_format(stream, "shll    r%d,r%d:r%d", R1, R2 + 0, R2 + 1); bytes = 2; break;
	case 0x34: util::stream_format(stream, "rotw    r%d,r%d", R1, R2); bytes = 2; break;
	case 0x35: util::stream_format(stream, "rotl    r%d,r%d:r%d", R1, R2 + 0, R2 + 1); bytes = 2; break;

	case 0x38: util::stream_format(stream, "shai    $%d,r%d", I16, R2); bytes = 4; break;
	case 0x39: util::stream_format(stream, "shali   $%d,r%d:r%d", I16, R2 + 0, R2 + 1); bytes = 4; break;
	case 0x3a: util::stream_format(stream, "shli    $%d,r%d", I16, R2); bytes = 4; break;
	case 0x3b: util::stream_format(stream, "shlli   $%d,r%d:r%d", I16, R2 + 0, R2 + 1); bytes = 4; break;
	case 0x3c: util::stream_format(stream, "roti    $%d,r%d", I16, R2); bytes = 4; break;
	case 0x3d: util::stream_format(stream, "rotli   $%d,r%d:r%d", I16, R2 + 0, R2 + 1); bytes = 4; break;

	case 0x44: util::stream_format(stream, "call    r%d,(r%d)", R2, R1); bytes = 2; flags |= DASMFLAG_STEP_OVER; break;
	case 0x45: util::stream_format(stream, "call    r%d,%s", ADDR_R2, address(pc, insn)); bytes = 2 + ADDR_SIZE; flags |= DASMFLAG_STEP_OVER; break;

	case 0x48: util::stream_format(stream, "b%-4s   (r%d)", cc[R2], R1); bytes = 2; break;
	case 0x49: util::stream_format(stream, "b%-4s   %s", cc[ADDR_R2], address(pc, insn)); bytes = 2 + ADDR_SIZE; break;

	case 0x4c: util::stream_format(stream, "bf%s   (r%d)", R2 == 0 ? "any" : "bad", R1); bytes = 2; break;
	case 0x4d: util::stream_format(stream, "bf%s   %s", ADDR_R2 == 0 ? "any" : "bad", address(pc, insn)); bytes = 2 + ADDR_SIZE; break;

	case 0x60: util::stream_format(stream, "loadw   (r%d),r%d", R1, R2); bytes = 2; break;
	case 0x61: util::stream_format(stream, "loadw   %s,r%d", address(pc, insn), ADDR_R2); bytes = 2 + ADDR_SIZE; break;
	case 0x62: util::stream_format(stream, "loada   (r%d),r%d", R1, R2); bytes = 2; break;
	case 0x63: util::stream_format(stream, "loada   %s,r%d", address(pc, insn), ADDR_R2); bytes = 2 + ADDR_SIZE; break;
	case 0x64: util::stream_format(stream, "loads   (r%d),f%d", R1, R2); bytes = 2; break;
	case 0x65: util::stream_format(stream, "loads   %s,f%d", address(pc, insn), ADDR_R2); bytes = 2 + ADDR_SIZE; break;
	case 0x66: util::stream_format(stream, "loadd   (r%d),f%d", R1, R2); bytes = 2; break;
	case 0x67: util::stream_format(stream, "loadd   %s,f%d", address(pc, insn), ADDR_R2); bytes = 2 + ADDR_SIZE; break;
	case 0x68: util::stream_format(stream, "loadb   (r%d),r%d", R1, R2); bytes = 2; break;
	case 0x69: util::stream_format(stream, "loadb   %s,r%d", address(pc, insn), ADDR_R2); bytes = 2 + ADDR_SIZE; break;
	case 0x6a: util::stream_format(stream, "loadbu  (r%d),r%d", R1, R2); bytes = 2; break;
	case 0x6b: util::stream_format(stream, "loadbu  %s,r%d", address(pc, insn), ADDR_R2); bytes = 2 + ADDR_SIZE; break;
	case 0x6c: util::stream_format(stream, "loadh   (r%d),r%d", R1, R2); bytes = 2; break;
	case 0x6d: util::stream_format(stream, "loadh   %s,r%d", address(pc, insn), ADDR_R2); bytes = 2 + ADDR_SIZE; break;
	case 0x6e: util::stream_format(stream, "loadhu  (r%d),r%d", R1, R2); bytes = 2; break;
	case 0x6f: util::stream_format(stream, "loadhu  %s,r%d", address(pc, insn), ADDR_R2); bytes = 2 + ADDR_SIZE; break;

	case 0x70: util::stream_format(stream, "storw   r%d,(r%d)", R2, R1); bytes = 2; break;
	case 0x71: util::stream_format(stream, "storw   r%d,%s", ADDR_R2, address(pc, insn)); bytes = 2 + ADDR_SIZE; break;
	case 0x72: util::stream_format(stream, "tsts    (r%d),r%d", R1, R2); bytes = 2; break;
	case 0x73: util::stream_format(stream, "tsts    %s,r%d", address(pc, insn), ADDR_R2); bytes = 2 + ADDR_SIZE; break;
	case 0x74: util::stream_format(stream, "stors   f%d,(r%d)", R2, R1); bytes = 2; break;
	case 0x75: util::stream_format(stream, "stors   f%d,%s", ADDR_R2, address(pc, insn)); bytes = 2 + ADDR_SIZE; break;
	case 0x76: util::stream_format(stream, "stord   f%d,(r%d)", R2, R1); bytes = 2; break;
	case 0x77: util::stream_format(stream, "stord   f%d,%s", ADDR_R2, address(pc, insn)); bytes = 2 + ADDR_SIZE; break;
	case 0x78: util::stream_format(stream, "storb   r%d,(r%d)", R2, R1); bytes = 2; break;
	case 0x79: util::stream_format(stream, "storb   r%d,%s", ADDR_R2, address(pc, insn)); bytes = 2 + ADDR_SIZE; break;

	case 0x7c: util::stream_format(stream, "storh   r%d,(r%d)", R2, R1); bytes = 2; break;
	case 0x7d: util::stream_format(stream, "storh   r%d,%s", ADDR_R2, address(pc, insn)); bytes = 2 + ADDR_SIZE; break;

	case 0x80: util::stream_format(stream, "addw    r%d,r%d", R1, R2); bytes = 2; break;
		
	case 0x82: util::stream_format(stream, "addq    $%d,r%d", R1, R2); bytes = 2; break;
	case 0x83: util::stream_format(stream, "addi    $%d,r%d", IMM_VALUE, R2); bytes = 2 + IMM_SIZE; break;
	case 0x84: util::stream_format(stream, "movw    r%d,r%d", R1, R2); bytes = 2; break;

	case 0x86: util::stream_format(stream, "loadq   $%d,r%d", R1, R2); bytes = 2; break;
	case 0x87: util::stream_format(stream, "loadi   $%d,r%d", IMM_VALUE, R2); bytes = 2 + IMM_SIZE; break;
	case 0x88: util::stream_format(stream, "andw    r%d,r%d", R1, R2); bytes = 2; break;

	case 0x8b: util::stream_format(stream, "andi    $%d,r%d", IMM_VALUE, R2); bytes = 2 + IMM_SIZE; break;
	case 0x8c: util::stream_format(stream, "orw     r%d,r%d", R1, R2); bytes = 2; break;

	case 0x8f: util::stream_format(stream, "ori     $%d,r%d", IMM_VALUE, R2); bytes = 2 + IMM_SIZE; break;

	case 0x90: util::stream_format(stream, "addwc   r%d,r%d", R1, R2); bytes = 2; break;
	case 0x91: util::stream_format(stream, "subwc   r%d,r%d", R1, R2); bytes = 2; break;

	case 0x93: util::stream_format(stream, "negw    r%d,r%d", R1, R2); bytes = 2; break;

	case 0x98: util::stream_format(stream, "mulw    r%d,r%d", R1, R2); bytes = 2; break;
	case 0x99: util::stream_format(stream, "mulwx   r%d,r%d:r%d", R1, R2 + 0, R2 + 1); bytes = 2; break;
	case 0x9a: util::stream_format(stream, "mulwu   r%d,r%d", R1, R2); bytes = 2; break;
	case 0x9b: util::stream_format(stream, "mulwux  r%d,r%d:r%d", R1, R2 + 0, R2 + 1); bytes = 2; break;
	case 0x9c: util::stream_format(stream, "divw    r%d,r%d", R1, R2); bytes = 2; break;
	case 0x9d: util::stream_format(stream, "modw    r%d,r%d", R1, R2); bytes = 2; break;
	case 0x9e: util::stream_format(stream, "divwu   r%d,r%d", R1, R2); bytes = 2; break;
	case 0x9f: util::stream_format(stream, "modwu   r%d,r%d", R1, R2); bytes = 2; break;

	case 0xa0: util::stream_format(stream, "subw    r%d,r%d", R1, R2); bytes = 2; break;

	case 0xa2: util::stream_format(stream, "subq    $%d,r%d", R1, R2); bytes = 2; break;
	case 0xa3: util::stream_format(stream, "subi    $%d,r%d", IMM_VALUE, R2); bytes = 2 + IMM_SIZE; break;
	case 0xa4: util::stream_format(stream, "cmpw    r%d,r%d", R1, R2); bytes = 2; break;

	case 0xa6: util::stream_format(stream, "cmpq    $%d,r%d", R1, R2); bytes = 2; break;
	case 0xa7: util::stream_format(stream, "cmpi    $%d,r%d", IMM_VALUE, R2); bytes = 2 + IMM_SIZE; break;
	case 0xa8: util::stream_format(stream, "xorw    r%d,r%d", R1, R2); bytes = 2; break;

	case 0xab: util::stream_format(stream, "xori    $%d,r%d", IMM_VALUE, R2); bytes = 2 + IMM_SIZE; break;
	case 0xac: util::stream_format(stream, "notw    r%d,r%d", R1, R2); bytes = 2; break;

	case 0xae: util::stream_format(stream, "notq    $%d,r%d", R1, R2); bytes = 2; break;

	case 0xb4:
	case 0xb5:
		// unprivileged macro instructions
		switch (insn[0] & 0xff)
		{
		case 0x00: case 0x01: case 0x02: case 0x03:
		case 0x04: case 0x05: case 0x06: case 0x07:
		case 0x08: case 0x09: case 0x0a: case 0x0b:
		case 0x0c:
			util::stream_format(stream, "savew%d", R2);
			break;

		case 0x0d: util::stream_format(stream, "movc"); break;
		case 0x0e: util::stream_format(stream, "initc"); break;
		case 0x0f: util::stream_format(stream, "cmpc"); break;

		case 0x10: case 0x11: case 0x12: case 0x13:
		case 0x14: case 0x15: case 0x16: case 0x17:
		case 0x18: case 0x19: case 0x1a: case 0x1b:
		case 0x1c:
			util::stream_format(stream, "restw%d", R2);
			break;

		case 0x20: case 0x21: case 0x22: case 0x23:
		case 0x24: case 0x25: case 0x26: case 0x27:
			util::stream_format(stream, "saved%d", R2);
			break;

		case 0x28: case 0x29: case 0x2a: case 0x2b:
		case 0x2c: case 0x2d: case 0x2e: case 0x2f:
			util::stream_format(stream, "restd%d", R2);
			break;

		case 0x30: util::stream_format(stream, "cnvsw   f%d,r%d", (insn[1] & 0xf0) >> 4, insn[1] & 0xf); break;
		case 0x31: util::stream_format(stream, "cnvrsw  f%d,r%d", (insn[1] & 0xf0) >> 4, insn[1] & 0xf); break;
		case 0x32: util::stream_format(stream, "cnvtsw  f%d,r%d", (insn[1] & 0xf0) >> 4, insn[1] & 0xf); break;
		case 0x33: util::stream_format(stream, "cnvws   r%d,f%d", (insn[1] & 0xf0) >> 4, insn[1] & 0xf); break;
		case 0x34: util::stream_format(stream, "cnvdw   f%d,r%d", (insn[1] & 0xf0) >> 4, insn[1] & 0xf); break;
		case 0x35: util::stream_format(stream, "cnvrdw  f%d,r%d", (insn[1] & 0xf0) >> 4, insn[1] & 0xf); break;
		case 0x36: util::stream_format(stream, "cnvtdw  f%d,r%d", (insn[1] & 0xf0) >> 4, insn[1] & 0xf); break;
		case 0x37: util::stream_format(stream, "cnvwd   r%d,f%d", (insn[1] & 0xf0) >> 4, insn[1] & 0xf); break;
		case 0x38: util::stream_format(stream, "cnvsd   f%d,f%d", (insn[1] & 0xf0) >> 4, insn[1] & 0xf); break;
		case 0x39: util::stream_format(stream, "cnvds   f%d,f%d", (insn[1] & 0xf0) >> 4, insn[1] & 0xf); break;
		case 0x3a: util::stream_format(stream, "negs    f%d,f%d", (insn[1] & 0xf0) >> 4, insn[1] & 0xf); break;
		case 0x3b: util::stream_format(stream, "negd    f%d,f%d", (insn[1] & 0xf0) >> 4, insn[1] & 0xf); break;
		case 0x3c: util::stream_format(stream, "scalbs  r%d,f%d", (insn[1] & 0xf0) >> 4, insn[1] & 0xf); break;
		case 0x3d: util::stream_format(stream, "scalbd  r%d,f%d", (insn[1] & 0xf0) >> 4, insn[1] & 0xf); break;
		case 0x3e: util::stream_format(stream, "trapfn"); break;
		case 0x3f: util::stream_format(stream, "loadfs  r%d,f%d", (insn[1] & 0xf0) >> 4, insn[1] & 0xf); break;

		default:
			util::stream_format(stream, "macro   0x%04x 0x%04x ; invalid", insn[0], insn[1]);
			break;
		}
		bytes = 4;
		break;
	case 0xb6:
	case 0xb7:
		// privileged macro instructions
		switch (insn[0] & 0xff)
		{
		case 0x00: util::stream_format(stream, "movus   r%d,r%d", (insn[1] & 0xf0) >> 4, insn[1] & 0xf); break;
		case 0x01: util::stream_format(stream, "movsu   r%d,r%d", (insn[1] & 0xf0) >> 4, insn[1] & 0xf); break;
		case 0x02: util::stream_format(stream, "saveur  r%d", (insn[1] & 0xf0) >> 4); break;
		case 0x03: util::stream_format(stream, "restur  r%d", (insn[1] & 0xf0) >> 4); break;
		case 0x04: util::stream_format(stream, "reti    r%d", (insn[1] & 0xf0) >> 4); flags |= DASMFLAG_STEP_OUT; break;
		case 0x05: util::stream_format(stream, "wait"); break;

		default:
			util::stream_format(stream, "macro   0x%04x %04x", insn[0], insn[1]);
			break;
		}
		bytes = 4;
		break;

	default:
		util::stream_format(stream, ".word 0x%04x ; invalid", insn[0]);
		bytes = 2;
		break;
	}

	return bytes | flags;
}