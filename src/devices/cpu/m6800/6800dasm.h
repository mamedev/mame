// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*
 *   A quick-hack 6803/6808 disassembler
 *
 *   Note: this is not the good and proper way to disassemble anything, but it works
 *
 *   I'm afraid to put my name on it, but I feel obligated:
 *   This code written by Aaron Giles (agiles@sirius.com) for the MAME project
 */

#ifndef MAME_CPU_M6800_6800DASM_H
#define MAME_CPU_M6800_6800DASM_H

#pragma once

class m680x_disassembler : public util::disasm_interface
{
public:
	m680x_disassembler(int subtype);
	virtual ~m680x_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	enum addr_mode {
		inh,    /* inherent */
		rel,    /* relative */
		imb,    /* immediate (byte) */
		imw,    /* immediate (word) */
		dir,    /* direct address */
		imd,    /* HD63701YO: immediate, direct address */
		ext,    /* extended address */
		idx,    /* x + byte offset */
		imx,    /* HD63701YO: immediate, x + byte offset */
		sx1     /* HD63701YO: undocumented opcodes: byte from (s+1) */
	};

	enum op_names {
		aba=0,  abx,    adca,   adcb,   adda,   addb,   addd,   aim,
		anda,   andb,   asl,    asla,   aslb,   asld,   asr,    asra,
		asrb,   bcc,    bcs,    beq,    bge,    bgt,    bhi,    bita,
		bitb,   ble,    bls,    blt,    bmi,    bne,    bpl,    bra,
		brn,    bsr,    bvc,    bvs,    cba,    clc,    cli,    clr,
		clra,   clrb,   clv,    cmpa,   cmpb,   cmpx,   com,    coma,
		comb,   daa,    dec,    deca,   decb,   des,    dex,    eim,
		eora,   eorb,   ill,    inc,    inca,   incb,   ins,    inx,
		jmp,    jsr,    lda,    ldb,    ldd,    lds,    ldx,    lsr,
		lsra,   lsrb,   lsrd,   mul,    neg,    nega,   negb,   nop,
		oim,    ora,    orb,    psha,   pshb,   pshx,   pula,   pulb,
		pulx,   rol,    rola,   rolb,   ror,    rora,   rorb,   rti,
		rts,    sba,    sbca,   sbcb,   sec,    sev,    sta,    stb,
		_std,   sei,    sts,    stx,    suba,   subb,   subd,   swi,
		wai,    tab,    tap,    tba,    tim,    tpa,    tst,    tsta,
		tstb,   tsx,    txs,    asx1,   asx2,   xgdx,   addx,   adcx,
		bitx,   slp
	};

	static const char *const op_name_str[];
	static const u8 table[0x104][3];

	int m_subtype;
};

#endif
