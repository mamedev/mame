#include "emu.h"
#include "debugger.h"
#include "scudsp.h"

CPU_DISASSEMBLE( scudsp )
{
	UINT32 op = oprom[0];
	unsigned size = 1;
//  const char *sym, *sym2;

	switch( op )
	{
		default:
			sprintf(buffer, "???");
			break;
	}

	return size;
}
