// TMS32082 PP Disassembler

#include "emu.h"


static char *output;
static const UINT8 *opdata;
static int opbytes;

static void ATTR_PRINTF(1,2) print(const char *fmt, ...)
{
	va_list vl;

	va_start(vl, fmt);
	output += vsprintf(output, fmt, vl);
	va_end(vl);
}

static offs_t tms32082_disasm_pp(char *buffer, offs_t pc, const UINT8 *oprom)
{
	output = buffer;
	UINT32 flags = 0;
	opdata = oprom;
	opbytes = 8;

	print("???");

	return opbytes | flags | DASMFLAG_SUPPORTED;
}


CPU_DISASSEMBLE(tms32082_pp)
{
	return tms32082_disasm_pp(buffer, pc, oprom);
}