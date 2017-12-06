// license:BSD-3-Clause
// copyright-holders:Peter Trauner
/*****************************************************************************
 *
 *   disasm.c
 *   portable lh5801 emulator interface
 *
 *
 *****************************************************************************/

#ifndef MAME_CPU_LH5801_5801DASM_H
#define MAME_CPU_LH5801_5801DASM_H

#pragma once

class lh5801_disassembler : public util::disasm_interface
{
public:
	lh5801_disassembler() = default;
	virtual ~lh5801_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	enum Ins
	{
		ILL, ILL2, PREFD, NOP,

		LDA, STA, LDI, LDX, STX,
		LDE, SDE, LIN, SIN,
		TIN, // (x++)->(y++)
		ADC, ADI, ADR, SBC, SBI,
		DCA, DCS, // bcd add and sub
		CPA, CPI, CIN, // A compared with (x++)
		AND, ANI, ORA, ORI, EOR, EAI, BIT, BII,
		INC, DEC,
		DRL, DRR, // digit rotates
		ROL, ROR,
		SHL, SHR,
		AEX, // A nibble swap

		BCR, BCS, BHR, BHS, BZR, BZS, BVR, BVS,
		BCH, LOP, // loop with ul
		JMP, SJP, RTN, RTI, HLT,
		VCR, VCS, VHR, VHS, VVS, VZR, VZS,
		VMJ, VEJ,
		PSH, POP, ATT, TTA,
		REC, SEC, RIE, SIE,

		AM0, AM1, // load timer reg
		ITA, // reads input port
		ATP, // akku send to data bus
		CDV, // clears internal divider
		OFF, // clears bf flip flop
		RDP, SDP,// reset display flip flop
		RPU, SPU,// flip flop pu off
		RPV, SPV // flip flop pv off
	};

	enum Adr
	{
		Imp,
		Reg,
		Vec, // imm byte (vector at 0xffxx)
		Vej,
		Imm,
		RegImm,
		Imm16,
		RegImm16,
		ME0,
		ME0Imm,
		Abs,
		AbsImm,
		ME1,
		ME1Imm,
		ME1Abs,
		ME1AbsImm,
		RelP,
		RelM
	};

	enum Regs
	{
		RegNone,
		A,
		XL, XH, X,
		YL, YH, Y,
		UL, UH, U,
		P, S
	};

	struct Entry {
		Ins ins;
		Adr adr;
		Regs reg;

		const char *ins_name() const { return ins_names[ins]; }
		const char *reg_name() const { return reg_names[reg]; }
		Entry(Ins i, Adr a = Imp, Regs r = RegNone) : ins(i), adr(a), reg(r) { }
	};

	static const char *const ins_names[];
	static const char *const reg_names[];

	static const Entry table[0x100];
	static const Entry table_fd[0x100];
};

#endif
