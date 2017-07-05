// license:BSD-3-Clause
// copyright-holders:Farfetch'd, R. Belmont
#ifndef __I960DIS_H__
#define __I960DIS_H__

struct disassemble_t
{
	disassemble_t(std::ostream &s, unsigned long ip, const device_disasm_interface::data_buffer &opc)
		: stream(s), IP(ip), opcodes(opc) { }

	std::ostream    &stream;    // output stream
	unsigned long   IP;
	unsigned long   IPinc;
	const device_disasm_interface::data_buffer &opcodes;
	uint32_t disflags;
};

#endif /* __I960DIS_H__ */
