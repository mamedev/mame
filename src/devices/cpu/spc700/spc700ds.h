// license:BSD-3-Clause
// copyright-holders:Karl Stenerud
#ifndef MAME_CPU_SPC700_SPC700DS_H
#define MAME_CPU_SPC700_SPC700DS_H

#pragma once

/* ======================================================================== */
/* =============================== COPYRIGHT ============================== */
/* ======================================================================== */
/*

Sony SPC700 CPU Emulator V1.0

Copyright Karl Stenerud

*/

class spc700_disassembler : public util::disasm_interface
{
public:
	spc700_disassembler() = default;
	virtual ~spc700_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	struct spc700_opcode_struct
	{
		unsigned char name;
		unsigned char args[2];
	};

	enum
	{
		IMP , A   , X   , Y   , YA  , SP  , PSW , C   , REL , UPAG, IMM , XI  ,
		XII , YI  , DP  , DPX , DPY , DPI , DXI , DIY , ABS , ABX , ABY , AXI , N0  ,
		N1  , N2  , N3  , N4  , N5  , N6  , N7  , N8  , N9  , N10 , N11 , N12 ,
		N13 , N14 , N15 , DP0 , DP1 , DP2 , DP3 , DP4 , DP5 , DP6 , DP7 , MEMN,
		MEMI
	};

	enum
	{
		ADC   ,  ADDW  ,  AND   ,  AND1  ,  ASL   ,  BBC   ,  BBS   ,  BCC   ,
		BCS   ,  BEQ   ,  BMI   ,  BNE   ,  BPL   ,  BRA   ,  BRK   ,  BVC   ,
		BVS   ,  CALL  ,  CBNE  ,  CLR1  ,  CLRC  ,  CLRP  ,  CLRV  ,  CMP   ,
		CMPW  ,  DAA   ,  DAS   ,  DBNZ  ,  DEC   ,  DECW  ,  DI    ,  DIV   ,
		EI    ,  EOR   ,  EOR1  ,  INC   ,  INCW  ,  JMP   ,  LSR   ,  MOV   ,
		MOV1  ,  MOVW  ,  MUL   ,  NOP   ,  NOT1  ,  NOTQ  ,  NOTC  ,  OR    ,
		OR1   ,  PCALL ,  POP   ,  PUSH  ,  RET   ,  RETI  ,  ROL   ,  ROR   ,
		SBC   ,  SET1  ,  SETC  ,  SETP  ,  SLEEP ,  STOP  ,  SUBW  ,  TCALL ,
		TCLR1 ,  TSET1 ,  XCN
	};

	static const char *const g_opnames[];
	static const spc700_opcode_struct g_opcodes[256];
	static inline unsigned int read_8_immediate(offs_t &pc, const data_buffer &opcodes);
	static inline unsigned int read_16_immediate(offs_t &pc, const data_buffer &opcodes);
};


#endif // MAME_CPU_SPC700_SPC700DS_H
