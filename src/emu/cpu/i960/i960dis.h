#ifndef __I960DIS_H__
#define __I960DIS_H__

struct disassemble_t
{
	char        *buffer;    // output buffer
	unsigned long   IP;
	unsigned long   IPinc;
	const UINT8 *oprom;
	UINT32 disflags;
};

#endif /* __I960DIS_H__ */
