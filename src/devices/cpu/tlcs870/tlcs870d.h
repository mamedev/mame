// license:BSD-3-Clause
// copyright-holders:David Haywood
/*************************************************************************************************************

    Toshiba TLCS-870 Series MCUs

    The TLCS-870/X expands on this instruction set using the same base encoding.

    The TLCS-870/C appears to have a completely different encoding.

    loosely baesd on the tlcs90 core by Luca Elia

*************************************************************************************************************/

#ifndef MAME_CPU_TLCS870_TLCS870D_H
#define MAME_CPU_TLCS870_TLCS870D_H

#pragma once

class tlcs870_disassembler : public util::disasm_interface
{
public:
	tlcs870_disassembler() = default;
	virtual ~tlcs870_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	enum _e_op {
		UNKNOWN = 0x00,
		CALL, CALLP, CALLV, CLR, CPL,
		DAA, DAS, DEC, /*DI,*/ DIV,
		/*EI,*/
		INC,
		/*J,*/ JP, JR, JRS,
		LD, LDW,
		MCMP, MUL,
		NOP,
		POP, PUSH,
		RET, RETI, RETN, ROLC, ROLD, RORC, RORD,
		SET, SHLC, SHRC, SWAP, SWI,
		/*TEST,*/ XCH,

		ALU_ADDC,
		ALU_ADD,
		ALU_SUBB,
		ALU_SUB,
		ALU_AND,
		ALU_XOR,
		ALU_OR,
		ALU_CMP
	};

	static const char *const op_names[];
	static const char *const reg8[];
	static const char *const type_x[];
	static const char *const conditions[];
	static const char *const reg16[];
	static const char *const reg16p[];

	uint16_t m_op;
	int m_param2_type;
	uint16_t m_param2;

	int m_param1_type;
	uint16_t m_param1;

	uint8_t m_bitpos;
	uint8_t m_flagsaffected;
	uint8_t m_cycles;

	uint32_t  m_addr;

	const data_buffer *m_opcodes;

	inline uint8_t  READ8();
	inline uint16_t READ16();

	void decode();
	void decode_register_prefix(uint8_t b0);
	void decode_source(int type, uint16_t val);
	void decode_dest(uint8_t b0);

	void disassemble_param(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params, int type, uint16_t val);
};

#endif
