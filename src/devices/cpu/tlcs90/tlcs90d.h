// license:BSD-3-Clause
// copyright-holders:Luca Elia
/*************************************************************************************************************

    Toshiba TLCS-90 Series MCU's

    emulation by Luca Elia, based on the Z80 core by Juergen Buchmueller

    ChangeLog:

    20150517 Fixed TRUN bit masking (timers start/stop handling) [Rainer Keuchel]

*************************************************************************************************************/

#ifndef MAME_CPU_TLCS90_TLCS90D_H
#define MAME_CPU_TLCS90_TLCS90D_H

#pragma once

class tlcs90_disassembler : public util::disasm_interface
{
public:
	tlcs90_disassembler() = default;
	virtual ~tlcs90_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	enum _e_op {    UNKNOWN,    NOP,    EX,     EXX,    LD,     LDW,    LDA,    LDI,    LDIR,   LDD,    LDDR,   CPI,    CPIR,   CPD,    CPDR,   PUSH,   POP,    JP,     JR,     CALL,   CALLR,      RET,    RETI,   HALT,   DI,     EI,     SWI,    DAA,    CPL,    NEG,    LDAR,   RCF,    SCF,    CCF,    TSET,   BIT,    SET,    RES,    INC,    DEC,    INCX,   DECX,   INCW,   DECW,   ADD,    ADC,    SUB,    SBC,    AND,    XOR,    OR,     CP,     RLC,    RRC,    RL,     RR,     SLA,    SRA,    SLL,    SRL,    RLD,    RRD,    DJNZ,   MUL,    DIV     };

	enum class e_mode : u8 {
		NONE,   BIT8,   CC,
		I8,     D8,     R8,
		I16,    D16,    R16,
		MI16,   MR16,   MR16D8, MR16R8,
		R16D8,  R16R8
	};

	static const char *const op_names[];
	static const char *const r8_names[];
	static const char *const r16_names[];
	static const char *const cc_names[];
	static const char *const ir_names[];

	uint8_t        m_op;

	e_mode  m_mode1;
	uint16_t  m_r1,m_r1b;

	e_mode  m_mode2;
	uint16_t  m_r2,m_r2b;

	offs_t m_addr;
	const data_buffer *m_opcodes;

	inline uint8_t  READ8();
	inline uint16_t READ16();
	void decode();

	bool stream_arg(std::ostream &stream, uint32_t pc, const char *pre, const e_mode mode, const uint16_t r, const uint16_t rb);
	const char *internal_registers_names(uint16_t x);
};

#endif
