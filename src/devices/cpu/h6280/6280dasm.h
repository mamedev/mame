// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/*****************************************************************************

    6280dasm.c Hudsonsoft Hu6280 (HuC6280/Hu6280a) disassembler

    Copyright Bryan McPhail, mish@tendril.co.uk

    This source code is based (with permission!) on the 6502 emulator by
    Juergen Buchmueller.  It is released as part of the Mame emulator project.
    Let me know if you intend to use this code in any other project.


    Notes relating to Mame:

    The dasm window shows 'real' memory, as executed by the cpu
    The data windows show 'physical' memory, as defined in the memory map

******************************************************************************/

#ifndef MAME_CPU_H6280_6280DASM_H
#define MAME_CPU_H6280_6280DASM_H

#pragma once

class h6280_disassembler : public util::disasm_interface
{
public:
	h6280_disassembler() = default;
	virtual ~h6280_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	enum addr_mode {
		_non=0,      /* no additional arguments */
		_acc,        /* accumulator */
		_imp,        /* implicit */
		_imm,        /* immediate */
		_abs,        /* absolute */
		_zpg,        /* zero page */
		_zpx,        /* zero page + X */
		_zpy,        /* zero page + Y */
		_zpi,        /* zero page indirect */
		_abx,        /* absolute + X */
		_aby,        /* absolute + Y */
		_rel,        /* relative */
		_idx,        /* zero page pre indexed */
		_idy,        /* zero page post indexed */
		_ind,        /* indirect */
		_iax,        /* indirect + X */
		_blk,        /* block */
		_zrl,        /* zero page relative */
		_imz,        /* immediate, zero page */
		_izx,        /* immediate, zero page + X */
		_ima,        /* immediate, absolute */
		_imx         /* immediate, absolute + X */
	};

	enum opcodes {
		/* 6502 opcodes */
		_adc=0,_and,  _asl,  _bcc,  _bcs,  _beq,  _bit,  _bmi,
		_bne,  _bpl,  _brk,  _bvc,  _bvs,  _clc,  _cld,  _cli,
		_clv,  _cmp,  _cpx,  _cpy,  _dec,  _dex,  _dey,  _eor,
		_inc,  _inx,  _iny,  _jmp,  _jsr,  _lda,  _ldx,  _ldy,
		_lsr,  _nop,  _ora,  _pha,  _php,  _pla,  _plp,  _rol,
		_ror,  _rti,  _rts,  _sbc,  _sec,  _sed,  _sei,  _sta,
		_stx,  _sty,  _tax,  _tay,  _tsx,  _txa,  _txs,  _tya,
		_ill,

		/* Hu6280 extensions */
		_bra,  _stz,  _trb,  _tsb,  _dea,  _ina,  _sax,  _bsr,
		_phx,  _phy,  _plx,  _ply,  _csh,  _csl,  _tam,  _tma,
		_cla,  _cly,  _clx,  _st0,  _st1,  _st2,  _tst,  _set,
		_tdd,  _tia,  _tii,  _tin,  _tai,  _say,  _sxy,

		_sm0,  _sm1,  _sm2,  _sm3,  _sm4,  _sm5,  _sm6,  _sm7,
		_rm0,  _rm1,  _rm2,  _rm3,  _rm4,  _rm5,  _rm6,  _rm7,

		_bs0,  _bs1,  _bs2,  _bs3,  _bs4,  _bs5,  _bs6,  _bs7,
		_br0,  _br1,  _br2,  _br3,  _br4,  _br5,  _br6,  _br7
	};

	static const char *const token[];
	static const unsigned char op6280[512];

};

#endif
