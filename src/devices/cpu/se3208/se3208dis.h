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
	typedef u32 (se3208_disassembler::*OP)(u16 opcode, std::ostream &stream);

	u32 INVALIDOP(u16 opcode, std::ostream &stream);
	u32 LDB(u16 opcode, std::ostream &stream);
	u32 STB(u16 opcode, std::ostream &stream);
	u32 LDS(u16 opcode, std::ostream &stream);
	u32 STS(u16 opcode, std::ostream &stream);
	u32 LD(u16 opcode, std::ostream &stream);
	u32 ST(u16 opcode, std::ostream &stream);
	u32 LDBU(u16 opcode, std::ostream &stream);
	u32 LDSU(u16 opcode, std::ostream &stream);
	u32 LERI(u16 opcode, std::ostream &stream);
	u32 LDSP(u16 opcode, std::ostream &stream);
	u32 STSP(u16 opcode, std::ostream &stream);
	u32 PUSH(u16 opcode, std::ostream &stream);
	u32 POP(u16 opcode, std::ostream &stream);
	u32 LEATOSP(u16 opcode, std::ostream &stream);
	u32 LEAFROMSP(u16 opcode, std::ostream &stream);
	u32 LEASPTOSP(u16 opcode, std::ostream &stream);
	u32 MOV(u16 opcode, std::ostream &stream);
	u32 LDI(u16 opcode, std::ostream &stream);
	u32 LDBSP(u16 opcode, std::ostream &stream);
	u32 STBSP(u16 opcode, std::ostream &stream);
	u32 LDSSP(u16 opcode, std::ostream &stream);
	u32 STSSP(u16 opcode, std::ostream &stream);
	u32 LDBUSP(u16 opcode, std::ostream &stream);
	u32 LDSUSP(u16 opcode, std::ostream &stream);
	u32 ADDI(u16 opcode, std::ostream &stream);
	u32 SUBI(u16 opcode, std::ostream &stream);
	u32 ADCI(u16 opcode, std::ostream &stream);
	u32 SBCI(u16 opcode, std::ostream &stream);
	u32 ANDI(u16 opcode, std::ostream &stream);
	u32 ORI(u16 opcode, std::ostream &stream);
	u32 XORI(u16 opcode, std::ostream &stream);
	u32 CMPI(u16 opcode, std::ostream &stream);
	u32 TSTI(u16 opcode, std::ostream &stream);
	u32 ADD(u16 opcode, std::ostream &stream);
	u32 SUB(u16 opcode, std::ostream &stream);
	u32 ADC(u16 opcode, std::ostream &stream);
	u32 SBC(u16 opcode, std::ostream &stream);
	u32 AND(u16 opcode, std::ostream &stream);
	u32 OR(u16 opcode, std::ostream &stream);
	u32 XOR(u16 opcode, std::ostream &stream);
	u32 CMP(u16 opcode, std::ostream &stream);
	u32 TST(u16 opcode, std::ostream &stream);
	u32 MULS(u16 opcode, std::ostream &stream);
	u32 NEG(u16 opcode, std::ostream &stream);
	u32 CALL(u16 opcode, std::ostream &stream);
	u32 JV(u16 opcode, std::ostream &stream);
	u32 JNV(u16 opcode, std::ostream &stream);
	u32 JC(u16 opcode, std::ostream &stream);
	u32 JNC(u16 opcode, std::ostream &stream);
	u32 JP(u16 opcode, std::ostream &stream);
	u32 JM(u16 opcode, std::ostream &stream);
	u32 JNZ(u16 opcode, std::ostream &stream);
	u32 JZ(u16 opcode, std::ostream &stream);
	u32 JGE(u16 opcode, std::ostream &stream);
	u32 JLE(u16 opcode, std::ostream &stream);
	u32 JHI(u16 opcode, std::ostream &stream);
	u32 JLS(u16 opcode, std::ostream &stream);
	u32 JGT(u16 opcode, std::ostream &stream);
	u32 JLT(u16 opcode, std::ostream &stream);
	u32 JMP(u16 opcode, std::ostream &stream);
	u32 JR(u16 opcode, std::ostream &stream);
	u32 CALLR(u16 opcode, std::ostream &stream);
	u32 ASR(u16 opcode, std::ostream &stream);
	u32 LSR(u16 opcode, std::ostream &stream);
	u32 ASL(u16 opcode, std::ostream &stream);
	u32 EXTB(u16 opcode, std::ostream &stream);
	u32 EXTS(u16 opcode, std::ostream &stream);
	u32 SET(u16 opcode, std::ostream &stream);
	u32 CLR(u16 opcode, std::ostream &stream);
	u32 SWI(u16 opcode, std::ostream &stream);
	u32 HALT(u16 opcode, std::ostream &stream);
	u32 MVTC(u16 opcode, std::ostream &stream);
	u32 MVFC(u16 opcode, std::ostream &stream);

	OP decode_op(u16 opcode);

	u32 PC;
	u32 SR;
	u32 ER;
};

#endif
