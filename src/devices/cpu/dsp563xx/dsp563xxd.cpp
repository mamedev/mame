// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// DSP 563xx disassembler

#include "emu.h"
#include "dsp563xxd.h"

dsp563xx_disassembler::dsp563xx_disassembler()
{
}

u32 dsp563xx_disassembler::opcode_alignment() const
{
	return 1;
}

offs_t dsp563xx_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	u32 opcode = opcodes.r32(pc) & 0xffffff;
	u8 kmove = t_move[opcode >> 8];
	u8 knpar = kmove || opcode >= 0x100000 ? 0 : t_npar[opcode];
	u8 kipar = knpar ? 0 : t_ipar[opcode & 0xff];
	bool ex = BIT(t_move_ex, kmove) || BIT(t_npar_ex[knpar >> 6], knpar & 0x3f);
	u32 exv = ex ? opcodes.r32(pc+1) : 0;

	std::string smove = disasm_move(kmove, opcode, exv, pc);
	std::string snpar = disasm_npar(knpar, opcode, exv, pc);
	std::string sipar = disasm_ipar(kipar, opcode, exv, pc);

	std::string s = "unknown";

	if(opcode == 0x000000)
	s = "nop";
	else if(knpar)
	s = snpar;
	else if(!sipar.empty()) {
	if(!smove.empty())
		s = sipar + ' ' + smove;
	else if(kmove)
		s = sipar;
	} else if(!smove.empty() && kipar)
	s = "move " + smove;
	stream << s;
	return (ex ? 2 : 1) | t_npar_flags[knpar] | SUPPORTED;
}
