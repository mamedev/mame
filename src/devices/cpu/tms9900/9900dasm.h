// license:BSD-3-Clause
// copyright-holders:Raphael Nabet
/*****************************************************************************
 *
 *   9900dasm.c
 *   TMS 9900 family disassembler
 *
 *****************************************************************************/

#ifndef MAME_CPU_TMS9900_9900DASM_H
#define MAME_CPU_TMS9900_9900DASM_H

#pragma once

class tms9900_disassembler : public util::disasm_interface
{
public:
	tms9900_disassembler(int model);
	virtual ~tms9900_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	enum format_t
	{
		format_1,   /* 2 address instructions */
		format_2a,  /* jump instructions */
		format_2b,  /* bit I/O instructions */
		format_3_9, /* logical, multiply, and divide instructions */
		format_4,   /* CRU instructions */
		format_5,   /* register shift instructions */
		format_6,   /* single address instructions */
		format_7,   /* instructions without operands */
		format_8a,  /* immediate instructions (destination register) */
		format_8b,  /* immediate instructions (no destination register) */
		format_9,   /* extended operation instruction */
		format_10,  /* memory map file instruction */
		format_11,  /* multiple precision instructions */
		format_12,  /* string instructions */
		format_13,  /* multiple precision shift instructions */
		format_14,  /* bit testing instructions */
		format_15,  /* invert order of field instruction */
		format_16,  /* field instructions */
		format_17,  /* alter register and jump instructions */
		format_18,  /* single register operand instructions */
		format_liim,/* format for liim (looks like format 18) */
		format_19,  /* move address instruction */
		format_20,  /* list search instructions */
		format_21,  /* extend precision instruction */

		illegal
	};

	/* definitions for flags */
	enum
	{
		/* processor set on which opcodes are available */
		ps_any      = 0x01,     /* every processor in the tms9900/ti990 family */
		ps_mapper   = 0x02,     /* processors with memory mapper (ti990/10, ti990/12,
		                           and tms99000 with mapper coprocessor) */
		ps_tms9995  = 0x04,     /* ti990/12, tms9995, and later */
		ps_tms99000 = 0x08,     /* ti990/12, tms99000, and later */
		ps_ti990_12 = 0x10,     /* ti990/12 only */

		/* additional flags for special decoding */
		sd_11       = 0x100,    /* bit 11 should be cleared in li, ai, andi, ori, ci, stwp, stst */
		sd_11_15    = 0x200     /* bits 11-15 should be cleared in lwpi, limi, idle, rset, rtwp, ckon, ckof, lrex */
	};

	struct description_t
	{
		const char *mnemonic;
		format_t format;
		int flags;
	};


	enum opcodes {
		/* basic instruction set */
		_a=0,   _ab,    _c,     _cb,    _s,     _sb,    _soc,   _socb,  _szc,   _szcb,
		_mov,   _movb,  _coc,   _czc,   _xor,   _mpy,   _div,   _xop,   _b,     _bl,
		_blwp,  _clr,   _seto,  _inv,   _neg,   _abs,   _swpb,  _inc,   _inct,  _dec,
		_dect,  _x,     _ldcr,  _stcr,  _sbo,   _sbz,   _tb,    _jeq,   _jgt,   _jh,
		_jhe,   _jl,    _jle,   _jlt,   _jmp,   _jnc,   _jne,   _jno,   _joc,   _jop,
		_sla,   _sra,   _src,   _srl,   _ai,    _andi,  _ci,    _li,    _ori,   _lwpi,
		_limi,  _stst,  _stwp,  _rtwp,  _idle,  _rset,  _ckof,  _ckon,  _lrex,

		/* mapper instruction set */
		_lds,   _ldd,   _lmf,

		/* tms9995 instruction set */
		_divs,  _mpys,  _lst,   _lwp,

		/* tms99000 instruction set */
		_bind,

		/* ti990/12 instruction set */
		_sram,  _slam,  _rto,   _lto,   _cnto,  _slsl,  _slsp,  _bdc,   _dbc,   _swpm,
		_xorm,  _orm,   _andm,  _sm,    _am,    _mova,  _emd,   _eint,  _dint,  _stpc,
		_cs,    _seqb,  _movs,  _lim,   _lcs,   _blsk,  _mvsr,  _mvsk,  _pops,  _pshs,

		_cri,   _cdi,   _negr,  _negd,  _cre,   _cde,   _cer,   _ced,   _nrm,   _tmb,
		_tcmb,  _tsmb,  _srj,   _arj,   _xit,   _insf,  _xv,    _xf,    _ar,    _cir,
		_sr,    _mr,    _dr,    _lr,    _str,   _iof,   _sneb,  _crc,   _ts,    _ad,
		_cid,   _sd,    _md,    _dd,    _ld,    _std,   _ep,

		/* tms9940-only instruction set */
		_liim,  _dca,   _dcs,

		_ill
	};

	static const description_t descriptions[144+3+1];
	static const enum opcodes ops_4000_ffff_s12[12];
	static const enum opcodes ops_2000_3fff_s10[8];
	static const enum opcodes ops_1000_1fff_s8[16];
	static const enum opcodes ops_0e40_0fff_s6[7];
	static const enum opcodes ops_0e00_0e3f_s4[4];
	static const enum opcodes ops_0c40_0dff_s6[7];
	static const enum opcodes ops_0c10_0c3f_s4[3];
	static const enum opcodes ops_0c00_0c0f_s0[16];
	static const enum opcodes ops_0800_0bff_s8[4];
	static const enum opcodes ops_0400_07ff_s6[16];
	static const enum opcodes ops_0200_03ff_s5[16];
	static const enum opcodes ops_0100_01ff_s6[4];
	static const enum opcodes ops_0030_00ff_s4[13];
	static const enum opcodes ops_001c_002f_s0[20];

	int m_model_id;

	inline uint16_t readop_arg(const data_buffer &params, offs_t &PC);
	void print_arg (std::ostream &stream, int mode, int arg, const data_buffer &params, offs_t &PC);
};

#endif // MAME_CPU_TMS9900_9900DASM_H
