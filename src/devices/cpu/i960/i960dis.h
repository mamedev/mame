// license:BSD-3-Clause
// copyright-holders:Farfetch'd, R. Belmont
#ifndef __I960DIS_H__
#define __I960DIS_H__

struct disassemble_t
{
	std::ostream    *stream;    // output stream
	unsigned long   IP;
	unsigned long   IPinc;
	const uint8_t *oprom;
	uint32_t disflags;
};

#endif /* __I960DIS_H__ */
