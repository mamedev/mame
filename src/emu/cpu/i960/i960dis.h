#ifndef __I960DIS_H__
#define __I960DIS_H__

typedef struct _disassemble_t disassemble_t;
struct _disassemble_t
{
	char		*buffer;	// output buffer
	unsigned long	IP;
	unsigned long	IPinc;
	const UINT8 *oprom;
	UINT32 disflags;
};

char *i960_disassemble(disassemble_t *diss);

#endif /* __I960DIS_H__ */
