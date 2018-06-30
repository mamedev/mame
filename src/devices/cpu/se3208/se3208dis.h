// license:BSD-3-Clause
// copyright-holders:ElSemi

#ifndef MAME_CPU_SE3208_SE3208DIS_H
#define MAME_CPU_SE3208_SE3208DIS_H

#pragma once

class se3208_disassembler : public util::disasm_interface
{
public:
	se3208_disassembler() = default;
	virtual ~se3208_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	typedef uint32_t (se3208_disassembler::*_OP)(uint16_t Opcode, std::ostream &stream);

	uint32_t INVALIDOP(uint16_t Opcode, std::ostream &stream);
	uint32_t LDB(uint16_t Opcode, std::ostream &stream);
	uint32_t STB(uint16_t Opcode, std::ostream &stream);
	uint32_t LDS(uint16_t Opcode, std::ostream &stream);
	uint32_t STS(uint16_t Opcode, std::ostream &stream);
	uint32_t LD(uint16_t Opcode, std::ostream &stream);
	uint32_t ST(uint16_t Opcode, std::ostream &stream);
	uint32_t LDBU(uint16_t Opcode, std::ostream &stream);
	uint32_t LDSU(uint16_t Opcode, std::ostream &stream);
	uint32_t LERI(uint16_t Opcode, std::ostream &stream);
	uint32_t LDSP(uint16_t Opcode, std::ostream &stream);
	uint32_t STSP(uint16_t Opcode, std::ostream &stream);
	uint32_t PUSH(uint16_t Opcode, std::ostream &stream);
	uint32_t POP(uint16_t Opcode, std::ostream &stream);
	uint32_t LEATOSP(uint16_t Opcode, std::ostream &stream);
	uint32_t LEAFROMSP(uint16_t Opcode, std::ostream &stream);
	uint32_t LEASPTOSP(uint16_t Opcode, std::ostream &stream);
	uint32_t MOV(uint16_t Opcode, std::ostream &stream);
	uint32_t LDI(uint16_t Opcode, std::ostream &stream);
	uint32_t LDBSP(uint16_t Opcode, std::ostream &stream);
	uint32_t STBSP(uint16_t Opcode, std::ostream &stream);
	uint32_t LDSSP(uint16_t Opcode, std::ostream &stream);
	uint32_t STSSP(uint16_t Opcode, std::ostream &stream);
	uint32_t LDBUSP(uint16_t Opcode, std::ostream &stream);
	uint32_t LDSUSP(uint16_t Opcode, std::ostream &stream);
	uint32_t ADDI(uint16_t Opcode, std::ostream &stream);
	uint32_t SUBI(uint16_t Opcode, std::ostream &stream);
	uint32_t ADCI(uint16_t Opcode, std::ostream &stream);
	uint32_t SBCI(uint16_t Opcode, std::ostream &stream);
	uint32_t ANDI(uint16_t Opcode, std::ostream &stream);
	uint32_t ORI(uint16_t Opcode, std::ostream &stream);
	uint32_t XORI(uint16_t Opcode, std::ostream &stream);
	uint32_t CMPI(uint16_t Opcode, std::ostream &stream);
	uint32_t TSTI(uint16_t Opcode, std::ostream &stream);
	uint32_t ADD(uint16_t Opcode, std::ostream &stream);
	uint32_t SUB(uint16_t Opcode, std::ostream &stream);
	uint32_t ADC(uint16_t Opcode, std::ostream &stream);
	uint32_t SBC(uint16_t Opcode, std::ostream &stream);
	uint32_t AND(uint16_t Opcode, std::ostream &stream);
	uint32_t OR(uint16_t Opcode, std::ostream &stream);
	uint32_t XOR(uint16_t Opcode, std::ostream &stream);
	uint32_t CMP(uint16_t Opcode, std::ostream &stream);
	uint32_t TST(uint16_t Opcode, std::ostream &stream);
	uint32_t MULS(uint16_t Opcode, std::ostream &stream);
	uint32_t NEG(uint16_t Opcode, std::ostream &stream);
	uint32_t CALL(uint16_t Opcode, std::ostream &stream);
	uint32_t JV(uint16_t Opcode, std::ostream &stream);
	uint32_t JNV(uint16_t Opcode, std::ostream &stream);
	uint32_t JC(uint16_t Opcode, std::ostream &stream);
	uint32_t JNC(uint16_t Opcode, std::ostream &stream);
	uint32_t JP(uint16_t Opcode, std::ostream &stream);
	uint32_t JM(uint16_t Opcode, std::ostream &stream);
	uint32_t JNZ(uint16_t Opcode, std::ostream &stream);
	uint32_t JZ(uint16_t Opcode, std::ostream &stream);
	uint32_t JGE(uint16_t Opcode, std::ostream &stream);
	uint32_t JLE(uint16_t Opcode, std::ostream &stream);
	uint32_t JHI(uint16_t Opcode, std::ostream &stream);
	uint32_t JLS(uint16_t Opcode, std::ostream &stream);
	uint32_t JGT(uint16_t Opcode, std::ostream &stream);
	uint32_t JLT(uint16_t Opcode, std::ostream &stream);
	uint32_t JMP(uint16_t Opcode, std::ostream &stream);
	uint32_t JR(uint16_t Opcode, std::ostream &stream);
	uint32_t CALLR(uint16_t Opcode, std::ostream &stream);
	uint32_t ASR(uint16_t Opcode, std::ostream &stream);
	uint32_t LSR(uint16_t Opcode, std::ostream &stream);
	uint32_t ASL(uint16_t Opcode, std::ostream &stream);
	uint32_t EXTB(uint16_t Opcode, std::ostream &stream);
	uint32_t EXTS(uint16_t Opcode, std::ostream &stream);
	uint32_t SET(uint16_t Opcode, std::ostream &stream);
	uint32_t CLR(uint16_t Opcode, std::ostream &stream);
	uint32_t SWI(uint16_t Opcode, std::ostream &stream);
	uint32_t HALT(uint16_t Opcode, std::ostream &stream);
	uint32_t MVTC(uint16_t Opcode, std::ostream &stream);
	uint32_t MVFC(uint16_t Opcode, std::ostream &stream);

	_OP DecodeOp(uint16_t Opcode);

	uint32_t PC;
	uint32_t SR;
	uint32_t ER;
};

#endif
