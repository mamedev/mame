/*********************************\

 ARCompact disassembler

\*********************************/

#include "emu.h"
#include <stdarg.h>

static char *output;

static void ATTR_PRINTF(1,2) print(const char *fmt, ...)
{
	va_list vl;

	va_start(vl, fmt);
	vsprintf(output, fmt, vl);
	va_end(vl);
}

/*****************************************************************************/



/*****************************************************************************/



CPU_DISASSEMBLE(arcompact)
{
	UINT32 op = oprom[0] | (oprom[1] << 8) | (oprom[2] << 16) | (oprom[3] << 24);
	op = BIG_ENDIANIZE_INT32(op);

	output = buffer;

	print("<undefined>");

	return 4 | DASMFLAG_SUPPORTED;
}
