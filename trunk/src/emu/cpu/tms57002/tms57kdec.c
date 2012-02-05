#include "emu.h"
#include "debugger.h"
#include "tms57002.h"
#include "tms57kpr.h"

#ifdef __GNUC__
#define noinline __attribute__((noinline))
#else
#define noinline /* */
#endif

static void tms57002_decode_error(tms57002_t *s, UINT32 opcode)
{
	char buf[256];
	UINT8 opr[3];
	if(s->unsupported_inst_warning)
		return;

	s->unsupported_inst_warning = 1;
	opr[0] = opcode;
	opr[1] = opcode >> 8;
	opr[2] = opcode >> 16;

	CPU_DISASSEMBLE_NAME(tms57002)(0, buf, s->pc, opr, opr, 0);
	popmessage("tms57002: %s - Contact Mamedev", buf);
}

void tms57002_decode_cat1(tms57002_t *s, UINT32 opcode, unsigned short *op, cstate *cs)
{
	switch(opcode >> 18) {
	case 0x00: // nop
		break;

#define CDEC1
#include "cpu/tms57002/tms57002.inc"
#undef CDEC1

	default:
		tms57002_decode_error(s, opcode);
		break;
	}
}

void tms57002_decode_cat2_pre(tms57002_t *s, UINT32 opcode, unsigned short *op, cstate *cs)
{
	switch((opcode >> 11) & 0x7f) {
	case 0x00: // nop
		break;

#define CDEC2A
#include "cpu/tms57002/tms57002.inc"
#undef CDEC2A

	default:
		tms57002_decode_error(s, opcode);
		break;
	}
}

void tms57002_decode_cat2_post(tms57002_t *s, UINT32 opcode, unsigned short *op, cstate *cs)
{
	switch((opcode >> 11) & 0x7f) {
	case 0x00: // nop
		break;

#define CDEC2B
#include "cpu/tms57002/tms57002.inc"
#undef CDEC2B

	default:
		tms57002_decode_error(s, opcode);
		break;
	}
}

void tms57002_decode_cat3(tms57002_t *s, UINT32 opcode, unsigned short *op, cstate *cs)
{
	switch((opcode >> 11) & 0x7f) {
	case 0x00: // nop
		break;

#define CDEC3
#include "cpu/tms57002/tms57002.inc"
#undef CDEC3

	default:
		tms57002_decode_error(s, opcode);
		break;
	}
}

