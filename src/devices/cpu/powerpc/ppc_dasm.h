// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*
 * disasm.c
 *
 * PowerPC 603e disassembler.
 *
 * When possible, invalid forms of instructions are checked for. To the best
 * of my knowledge, all appropriate load/store instructions are checked. I'm
 * not sure whether any other kinds of instructions need checking.
 */

/* Originally written by Bart Trzynadlowski for Supermodel project
 *
 * PowerPC 403 opcodes and MAME conversion by Ville Linde
 */

#ifndef MAME_CPU_POWERPC_PPC_DASM_H
#define MAME_CPU_POWERPC_PPC_DASM_H

#pragma once


class powerpc_disassembler : public util::disasm_interface
{
public:
	enum implementation : int
	{
		I_POWER   = 1 << 0,
		I_POWERPC = 1 << 1,
	};

	powerpc_disassembler(bool powerpc = true)
		: m_implementation(powerpc ? I_POWERPC : I_POWER)
	{
	};
	virtual ~powerpc_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;
	offs_t dasm_one(std::ostream &stream, uint32_t pc, uint32_t op);

private:
	/*
	 * Operand Formats
	 *
	 * These convey information on what operand fields are present and how they
	 * ought to be printed.
	 *
	 * I'm fairly certain all of these are used, but that is not guaranteed.
	 */

	enum
	{
		F_NONE,         // <no operands>
		F_LI,           // LI*4+PC if AA=0 else LI*4
		F_BCx,          // BO, BI, target_addr  used only by BCx
		F_RT_RA_0_SIMM, // rT, rA|0, SIMM       rA|0 means if rA == 0, print 0
		F_ADDIS,        // rT, rA, SIMM (printed as unsigned)   only used by ADDIS
		F_RT_RA_SIMM,   // rT, rA, SIMM
		F_RA_RT_UIMM,   // rA, rT, UIMM
		F_CMP_SIMM,     // crfD, L, A, SIMM
		F_CMP_UIMM,     // crfD, L, A, UIMM
		F_RT_RA_0_RB,   // rT, rA|0, rB
		F_RT_RA_RB,     // rT, rA, rB
		F_RT_D_RA_0,    // rT, d(rA|0)
		F_RT_D_RA,      // rT, d(rA)
		F_RA_RT_RB,     // rA, rT, rB
		F_FRT_D_RA_0,   // frT, d(RA|0)
		F_FRT_D_RA,     // frT, d(RA)
		F_FRT_RA_0_RB,  // frT, rA|0, rB
		F_FRT_RA_RB,    // frT, rA, rB
		F_TWI,          // TO, rA, SIMM         only used by TWI instruction
		F_CMP,          // crfD, L, rA, rB
		F_RA_RT,        // rA, rT
		F_RA_0_RB,      // rA|0, rB
		F_FRT_FRB,      // frT, frB
		F_FCMP,         // crfD, frA, frB
		F_CRFD_CRFS,    // crfD, crfS
		F_MCRXR,        // crfD                 only used by MCRXR
		F_RT,           // rT
		F_MFSR,         // rT, SR               only used by MFSR
		F_MTSR,         // SR, rT               only used by MTSR
		F_MFFSx,        // frT                  only used by MFFSx
		F_FCRBD,        // crbD                 FPSCR[crbD]
		F_MTFSFIx,      // crfD, IMM            only used by MTFSFIx
		F_RB,           // rB
		F_TW,           // TO, rA, rB           only used by TW
		F_RT_RA_0_NB,   // rT, rA|0, NB         print 32 if NB == 0
		F_SRAWIx,       // rA, rT, SH           only used by SRAWIx
		F_BO_BI,        // BO, BI
		F_CRBD_CRBA_CRBB,   // crbD, crbA, crbB
		F_RT_SPR,       // rT, SPR              and TBR
		F_MTSPR,        // SPR, rT              only used by MTSPR
		F_MTCRF,        // CRM, rT              only used by MTCRF
		F_MTFSFx,       // FM, frB              only used by MTFSFx
		F_RT_DCR,       // rT, DCR
		F_MTDCR,        // DCR, rT
		F_RT_RA,        // rT, rA
		F_FRT_FRA_FRC_FRB,  // frT, frA, frC, frB
		F_FRT_FRA_FRB,  // frT, frA, frB
		F_FRT_FRA_FRC,  // frT, frA, frC
		F_RA_RT_SH_MB_ME,   // rA, rT, SH, MB, ME
		F_RA_RT_RB_MB_ME,   // rA, rT, rB, MB, ME
		F_RT_RB         // rT, rB
	};

	/*
	 * Flags
	 */

	enum
	{
		FL_OE           = 1 << 0,    // if there is an OE field
		FL_RC           = 1 << 1,    // if there is an RC field
		FL_LK           = 1 << 2,    // if there is an LK field
		FL_AA           = 1 << 3,    // if there is an AA field
		FL_CHECK_RA_RT  = 1 << 4,    // assert rA!=0 and rA!=rT
		FL_CHECK_RA     = 1 << 5,    // assert rA!=0
		FL_CHECK_LSWI   = 1 << 6,    // specific check for LSWI validity
		FL_CHECK_LSWX   = 1 << 7,    // specific check for LSWX validity
		FL_SO           = 1 << 8     // use STEP_OUT
	};

	/*
	 * Instruction Descriptor
	 *
	 * Describes the layout of an instruction.
	 */

	struct IDESCR
	{
		char    mnem[32];   // mnemonic
		uint32_t  match;      // bit pattern of instruction after it has been masked
		uint32_t  mask;       // mask of variable fields (AND with ~mask to compare w/
						// bit pattern to determine a match)
		int format;         // operand format
		int flags;          // flags
		int implementation;
	};

	static const IDESCR itab[];
	static const char *const crbit[4];
	static const char *const crnbit[4];

	static std::string SPR(int spr_field);
	static std::string DCR(int dcr_field);
	static std::string DecodeSigned16(uint32_t op, int do_unsigned);
	static uint32_t Mask(unsigned const mb, unsigned const me);
	bool Simplified(uint32_t op, uint32_t vpc, std::string &signed16, std::string &mnem, std::string &oprs);

	implementation const m_implementation;
};

class power_disassembler : public powerpc_disassembler
{
public:
	power_disassembler()
		: powerpc_disassembler(false)
	{
	};
};

#endif
