// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/*****************************************************************************
 *
 *   Portable uPD7810/11, 7810H/11H, 78C10/C11/C14 disassembler
 *
 *   NS20030112: added 7807.
 *
 *****************************************************************************/

#ifndef MAME_CPU_UPD7810_UPD7810_DASM_H
#define MAME_CPU_UPD7810_UPD7810_DASM_H

#pragma once

class upd7810_base_disassembler : public util::disasm_interface
{
public:
	struct dasm_s {
	public:
		dasm_s();
		dasm_s(uint8_t t, const char *a);
		dasm_s(const dasm_s (&a)[256]);

		const char *name() const;
		const char *args() const;

		bool is_prefix() const;
		bool is_call() const;
		bool is_return() const;

		const dasm_s &prefix_get(uint8_t op) const;

		uint8_t m_token;
		const void *m_args;

		static const char *const token_names[];
	};

	enum
	{
		prefix = 0,
		illegal,
		ACI,
		ADC,
		ADCW,
		ADCX,
		ADD,
		ADDNC,
		ADDNCW,
		ADDNCX,
		ADDW,
		ADDX,
		ADI,
		ADINC,
		ANA,
		ANAW,
		ANAX,
		AND,
		ANI,
		ANIW,
		BIT,
		BLOCK,
		CALB,
		CALF,
		CALL,
		CALT,
		CLC,
		CLR,    /* 7807 */
		CMC,    /* 7807 */
		DAA,
		DADC,
		DADD,
		DADDNC,
		DAN,
		DCR,
		DCRW,
		DCX,
		DEQ,
		DGT,
		DI,
		DIV,
		DLT,
		DMOV,
		DNE,
		DOFF,
		DON,
		DOR,
		DRLL,
		DRLR,
		DSBB,
		DSLL,
		DSLR,
		DSUB,
		DSUBNB,
		DXR,
		EADD,
		EI,
		EQA,
		EQAW,
		EQAX,
		EQI,
		EQIW,
		ESUB,
		EX,     /* 7801 */
		EXA,
		EXH,
		EXX,
		EXR,    /* 7807 */
		GTA,
		GTAW,
		GTAX,
		GTI,
		GTIW,
		HALT,
		IN,     /* 7801 */
		INR,
		INRW,
		INX,
		JB,
		JEA,
		JMP,
		JR,
		JRE,
		LBCD,
		LDAW,
		LDAX,
		LDEAX,
		LDED,
		LHLD,
		LSPD,
		LTA,
		LTAW,
		LTAX,
		LTI,
		LTIW,
		LXI,
		MOV,
		MUL,
		MVI,
		MVIW,
		MVIX,
		NEA,
		NEAW,
		NEAX,
		NEGA,
		NEI,
		NEIW,
		NOP,
		NOT,    /* 7807 */
		OFFA,
		OFFAW,
		OFFAX,
		OFFI,
		OFFIW,
		ONA,
		ONAW,
		ONAX,
		ONI,
		ONIW,
		OR, /* 7807 */
		ORA,
		ORAW,
		ORAX,
		ORI,
		ORIW,
		OUT,    /* 7801 */
		PER,    /* 7801 */
		PEX,    /* 7801 */
		POP,
		PUSH,
		RET,
		RETI,
		RETS,
		RLD,
		RLL,
		RLR,
		RRD,
		SBB,
		SBBW,
		SBBX,
		SBCD,
		SBI,
		SDED,
		SETB,   /* 7807 */
		SHLD,
		SIO,    /* 7801 */
		SK,
		SKIT,
		SKN,
		SKNIT,
		SLL,
		SLLC,
		SLR,
		SLRC,
		SOFTI,
		SSPD,
		STAW,
		STAX,
		STC,
		STEAX,
		STM,    /* 7801 */
		STOP,
		SUB,
		SUBNB,
		SUBNBW,
		SUBNBX,
		SUBW,
		SUBX,
		SUI,
		SUINB,
		TABLE,
		XOR,    /* 7807 */
		XRA,
		XRAW,
		XRAX,
		XRI
	};

	upd7810_base_disassembler(const dasm_s *table, bool is_7810);
	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

	static const char *const regname[32];
	static const dasm_s d60[256];
	static const dasm_s d70[256];
	static const dasm_s d74[256];

	bool m_is_7810;
	const dasm_s *m_dasmXX;
};

class upd7810_disassembler : public upd7810_base_disassembler
{
public:
	static const dasm_s XX_7810[256];
	static const dasm_s d48_7810[256];
	static const dasm_s d4C_7810[256];
	static const dasm_s d4D_7810[256];
	static const dasm_s d64_7810[256];

	upd7810_disassembler();
	virtual ~upd7810_disassembler() = default;
};

class upd7807_disassembler : public upd7810_base_disassembler
{
public:
	static const dasm_s XX_7807[256];
	static const dasm_s d48_7807[256];
	static const dasm_s d4C_7807[256];
	static const dasm_s d4D_7807[256];
	static const dasm_s d64_7807[256];

	upd7807_disassembler();
	virtual ~upd7807_disassembler() = default;
};

class upd7801_disassembler : public upd7810_base_disassembler
{
public:
	static const dasm_s XX_7801[256];
	static const dasm_s d48_7801[256];
	static const dasm_s d4C_7801[256];
	static const dasm_s d4D_7801[256];
	static const dasm_s d60_7801[256];
	static const dasm_s d64_7801[256];
	static const dasm_s d70_7801[256];
	static const dasm_s d74_7801[256];

	upd7801_disassembler();
	virtual ~upd7801_disassembler() = default;
};

class upd78c05_disassembler : public upd7810_base_disassembler
{
public:
	static const dasm_s XX_78c05[256];
	static const dasm_s d48_78c05[256];
	static const dasm_s d4C_78c05[256];
	static const dasm_s d4D_78c05[256];
	static const dasm_s d60_78c05[256];
	static const dasm_s d64_78c05[256];
	static const dasm_s d70_78c05[256];
	static const dasm_s d74_78c05[256];

	upd78c05_disassembler();
	virtual ~upd78c05_disassembler() = default;
};

#endif // MAME_CPU_UPD7810_UPD7810_DASM_H
