#ifndef _I960DISASSEMBLER_H
#define _I960DISASSEMBLER_H

typedef struct
{
	char		*buffer;	// output buffer
	unsigned long	IP;
	unsigned long	IPinc;
	const UINT8 *oprom;
	UINT32 disflags;
} disassemble_t;

char *i960_disassemble(disassemble_t *diss);

#endif // _I960DISASSEMBLER_H
