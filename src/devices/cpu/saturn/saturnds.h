// license:BSD-3-Clause
// copyright-holders:Peter Trauner,Antoine Mine
/*****************************************************************************
 *
 *   saturnds.c
 *   portable saturn emulator interface
 *   (hp calculators)
 *
 *****************************************************************************/

#ifndef MAME_CPU_SATURN_SATURNDS_H
#define MAME_CPU_SATURN_SATURNDS_H

#pragma once

class saturn_disassembler : public util::disasm_interface
{
public:
	struct config {
		virtual ~config() = default;
		virtual bool get_nonstandard_mnemonics_mode() const = 0;
	};

	saturn_disassembler(config *conf);
	virtual ~saturn_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	// don't split branch and return, source relies on this ordering
	enum MNEMONICS
	{
		Return, ReturnSetXM, ReturnSetCarry, ReturnClearCarry, ReturnFromInterrupt,
		jump3,jump4,jump,
		call3,call4,call,
		branchCarrySet, returnCarrySet,
		branchCarryClear, returnCarryClear,

		outCS, outC, inA, inC,
		unconfig, xconfig, Cid, shutdown, cp1, reset, buscc,
		CcopyP, PcopyC, sreq, CswapP,

		inton, AloadImm, buscb,
		clearAbit, setAbit,
		branchAbitclear, returnAbitclear,
		branchAbitset, returnAbitset,
		clearCbit, setCbit,
		branchCbitclear, returnCbitclear,
		branchCbitset, returnCbitset,
		PCloadA, buscd, PCloadC, intoff, rsi,

		jumpA, jumpC, PCcopyA, PCcopyC, AcopyPC, CcopyPC,

		clearHST,
		branchHSTclear, returnHSTclear,

		clearBitST, setBitST,
		branchSTclear, returnSTclear,
		branchSTset, returnSTset,

		branchPdiffers, returnPdiffers,
		branchPequals, returnPequals,

		branchAequalsB, returnAequalsB,
		branchBequalsC, returnBequalsC,
		branchAequalsC, returnAequalsC,
		branchCequalsD, returnCequalsD,
		branchAdiffersB, returnAdiffersB,
		branchBdiffersC, returnBdiffersC,
		branchAdiffersC, returnAdiffersC,
		branchCdiffersD, returnCdiffersD,
		branchAzero, returnAzero,
		branchBzero, returnBzero,
		branchCzero, returnCzero,
		branchDzero, returnDzero,
		branchAnotzero, returnAnotzero,
		branchBnotzero, returnBnotzero,
		branchCnotzero, returnCnotzero,
		branchDnotzero, returnDnotzero,

		branchAgreaterB, returnAgreaterB,
		branchBgreaterC, returnBgreaterC,
		branchCgreaterA, returnCgreaterA,
		branchDgreaterC, returnDgreaterC,
		branchAlowerB, returnAlowerB,
		branchBlowerC, returnBlowerC,
		branchClowerA, returnClowerA,
		branchDlowerC, returnDlowerC,
		branchAnotlowerB, returnAnotlowerB,
		branchBnotlowerC, returnBnotlowerC,
		branchCnotlowerA, returnCnotlowerA,
		branchDnotlowerC, returnDnotlowerC,
		branchAnotgreaterB, returnAnotgreaterB,
		branchBnotgreaterC, returnBnotgreaterC,
		branchCnotgreaterA, returnCnotgreaterA,
		branchDnotgreaterC, returnDnotgreaterC,

		SetHexMode, SetDecMode,
		PushC, PopC,

		D0loadImm2, D0loadImm4, D0loadImm5,
		D1loadImm2, D1loadImm4, D1loadImm5,
		PloadImm, CloadImm,

		clearST,
		CcopyST, STcopyC,
		swapCST,

		incP, decP,

		R0copyA, R1copyA, R2copyA, R3copyA, R4copyA,
		R0copyC, R1copyC, R2copyC, R3copyC, R4copyC,

		AcopyR0, AcopyR1, AcopyR2, AcopyR3, AcopyR4,
		CcopyR0, CcopyR1, CcopyR2, CcopyR3, CcopyR4,

		D0copyA, D1copyA, D0copyC, D1copyC,
		D0copyAShort, D1copyAShort, D0copyCShort, D1copyCShort, // other class mnemonic

		SwapAR0, SwapAR1, SwapAR2, SwapAR3, SwapAR4,
		SwapCR0, SwapCR1, SwapCR2, SwapCR3, SwapCR4,

		SwapAD0, SwapAD1, SwapCD0, SwapCD1,
		SwapAD0Short, SwapAD1Short, SwapCD0Short, SwapCD1Short, // other class mnemonic

		D0storeA, D1storeA, D0storeC, D1storeC,
		AloadD0, AloadD1, CloadD0, CloadD1,

		D0addImm, D1addImm, D0subImm, D1subImm,
		AaddImm, BaddImm, CaddImm, DaddImm,
		AsubImm, BsubImm, CsubImm, DsubImm,

		AandB, BandC, CandA, DandC, BandA, CandB, AandC, CandD,
		AorB, BorC, CorA, DorC, BorA, CorB, AorC, CorD,

		Ashiftrightbit, Bshiftrightbit, Cshiftrightbit, Dshiftrightbit,

		AshiftleftCarry, BshiftleftCarry, CshiftleftCarry, DshiftleftCarry,
		AshiftrightCarry, BshiftrightCarry, CshiftrightCarry, DshiftrightCarry,

		AaddB, BaddC, CaddA, DaddC, AaddA, BaddB, CaddC, DaddD,
		BaddA, CaddB, AaddC, CaddD, decA, decB, decC, decD,

		AsubB, BsubC, CsubA, DsubC, incA, incB, incC, incD,
		BsubA, CsubB, AsubC, CsubD, AsubnB, BsubnC, CsubnA, DsubnC,

		clearA, clearB, clearC, clearD,
		AcopyB, BcopyC, CcopyA, DcopyC, BcopyA, CcopyB, AcopyC, CcopyD,
		AswapB, BswapC, CswapA, DswapC,

		Ashiftleft, Bshiftleft, Cshiftleft, Dshiftleft,
		Ashiftright, Bshiftright, Cshiftright, Dshiftright,
		negateA, negateB, negateC, negateD,
		notA, notB, notC, notD
	};

	enum opcode_sel
	{
		Complete=-1,
		Illegal,
		Opcode0, Opcode0E, Opcode0Ea,
		Opcode1, Opcode10, Opcode11, Opcode12, Opcode13, Opcode14, Opcode15,
		Opcode8, Opcode80, Opcode808, Opcode8081,
		Opcode81, Opcode818, Opcode818a, Opcode819, Opcode819a,
		Opcode81A, Opcode81Aa, Opcode81Aa0,Opcode81Aa1, Opcode81Aa2, Opcode81B,
		Opcode8A, Opcode8B,
		Opcode9, Opcode9a, Opcode9b,
		OpcodeA, OpcodeAa, OpcodeAb,
		OpcodeB, OpcodeBa, OpcodeBb,
		OpcodeC,
		OpcodeD,
		OpcodeE,
		OpcodeF
	};

	enum opcode_adr
	{
		AdrNone,
		AdrAF, AdrA, AdrB, AdrCount,
		BranchReturn, TestBranchRet, ImmBranch,
		ABranchReturn, // address field A
		xBranchReturn, // address field specified in previous opcode entry
		Imm, ImmCount, ImmCload, Imm2, Imm4, Imm5,
		Dis3, Dis3Call, Dis4, Dis4Call, Abs,
		FieldP, FieldWP, FieldXS, FieldX, FieldS, FieldM, FieldB, FieldW, FieldA,
		AdrImmCount
	};

	struct OPCODE
	{
		opcode_sel sel;
		opcode_adr adr;
		MNEMONICS mnemonic;
	};

	config *m_config;

	static const char *const adr_b[];
	static const char *const adr_af[];
	static const char *const adr_a[];
	static const char number_2_hex[];

	static const char *const mnemonics[][2];
	static const OPCODE opcs[][0x10];

	static const int field_adr_af[];
	static const int field_adr_a[];
	static const int field_adr_b[];

	const char *field_2_string(int adr_enum);

};

#endif
