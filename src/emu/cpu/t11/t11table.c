/*** t11: Portable DEC T-11 emulator ******************************************

    Copyright (C) Aaron Giles 1998

    Opcode table plus function prototypes

*****************************************************************************/

/*

modes:
  rg = register
  rgd = register deferred
  in = increment
  ind = increment deferred
  de = decrement
  ded = decrement deferred
  ix = index
  ixd = index deferred

*/

static void op_0000(void);
static void illegal(void);

static void jmp_rgd(void);
static void jmp_in(void);
static void jmp_ind(void);
static void jmp_de(void);
static void jmp_ded(void);
static void jmp_ix(void);
static void jmp_ixd(void);

static void rts(void);
static void ccc(void);
static void scc(void);

static void swab_rg(void);
static void swab_rgd(void);
static void swab_in(void);
static void swab_ind(void);
static void swab_de(void);
static void swab_ded(void);
static void swab_ix(void);
static void swab_ixd(void);

static void br(void);
static void bne(void);
static void beq(void);
static void bge(void);
static void blt(void);
static void bgt(void);
static void ble(void);

static void jsr_rgd(void);
static void jsr_in(void);
static void jsr_ind(void);
static void jsr_de(void);
static void jsr_ded(void);
static void jsr_ix(void);
static void jsr_ixd(void);

static void clr_rg(void);
static void clr_rgd(void);
static void clr_in(void);
static void clr_ind(void);
static void clr_de(void);
static void clr_ded(void);
static void clr_ix(void);
static void clr_ixd(void);

static void com_rg(void);
static void com_rgd(void);
static void com_in(void);
static void com_ind(void);
static void com_de(void);
static void com_ded(void);
static void com_ix(void);
static void com_ixd(void);

static void inc_rg(void);
static void inc_rgd(void);
static void inc_in(void);
static void inc_ind(void);
static void inc_de(void);
static void inc_ded(void);
static void inc_ix(void);
static void inc_ixd(void);

static void dec_rg(void);
static void dec_rgd(void);
static void dec_in(void);
static void dec_ind(void);
static void dec_de(void);
static void dec_ded(void);
static void dec_ix(void);
static void dec_ixd(void);

static void neg_rg(void);
static void neg_rgd(void);
static void neg_in(void);
static void neg_ind(void);
static void neg_de(void);
static void neg_ded(void);
static void neg_ix(void);
static void neg_ixd(void);

static void adc_rg(void);
static void adc_rgd(void);
static void adc_in(void);
static void adc_ind(void);
static void adc_de(void);
static void adc_ded(void);
static void adc_ix(void);
static void adc_ixd(void);

static void sbc_rg(void);
static void sbc_rgd(void);
static void sbc_in(void);
static void sbc_ind(void);
static void sbc_de(void);
static void sbc_ded(void);
static void sbc_ix(void);
static void sbc_ixd(void);

static void tst_rg(void);
static void tst_rgd(void);
static void tst_in(void);
static void tst_ind(void);
static void tst_de(void);
static void tst_ded(void);
static void tst_ix(void);
static void tst_ixd(void);

static void ror_rg(void);
static void ror_rgd(void);
static void ror_in(void);
static void ror_ind(void);
static void ror_de(void);
static void ror_ded(void);
static void ror_ix(void);
static void ror_ixd(void);

static void rol_rg(void);
static void rol_rgd(void);
static void rol_in(void);
static void rol_ind(void);
static void rol_de(void);
static void rol_ded(void);
static void rol_ix(void);
static void rol_ixd(void);

static void asr_rg(void);
static void asr_rgd(void);
static void asr_in(void);
static void asr_ind(void);
static void asr_de(void);
static void asr_ded(void);
static void asr_ix(void);
static void asr_ixd(void);

static void asl_rg(void);
static void asl_rgd(void);
static void asl_in(void);
static void asl_ind(void);
static void asl_de(void);
static void asl_ded(void);
static void asl_ix(void);
static void asl_ixd(void);

/*static void mark(void);*/

static void sxt_rg(void);
static void sxt_rgd(void);
static void sxt_in(void);
static void sxt_ind(void);
static void sxt_de(void);
static void sxt_ded(void);
static void sxt_ix(void);
static void sxt_ixd(void);

static void mov_rg_rg(void);
static void mov_rg_rgd(void);
static void mov_rg_in(void);
static void mov_rg_ind(void);
static void mov_rg_de(void);
static void mov_rg_ded(void);
static void mov_rg_ix(void);
static void mov_rg_ixd(void);
static void mov_rgd_rg(void);
static void mov_rgd_rgd(void);
static void mov_rgd_in(void);
static void mov_rgd_ind(void);
static void mov_rgd_de(void);
static void mov_rgd_ded(void);
static void mov_rgd_ix(void);
static void mov_rgd_ixd(void);
static void mov_in_rg(void);
static void mov_in_rgd(void);
static void mov_in_in(void);
static void mov_in_ind(void);
static void mov_in_de(void);
static void mov_in_ded(void);
static void mov_in_ix(void);
static void mov_in_ixd(void);
static void mov_ind_rg(void);
static void mov_ind_rgd(void);
static void mov_ind_in(void);
static void mov_ind_ind(void);
static void mov_ind_de(void);
static void mov_ind_ded(void);
static void mov_ind_ix(void);
static void mov_ind_ixd(void);
static void mov_de_rg(void);
static void mov_de_rgd(void);
static void mov_de_in(void);
static void mov_de_ind(void);
static void mov_de_de(void);
static void mov_de_ded(void);
static void mov_de_ix(void);
static void mov_de_ixd(void);
static void mov_ded_rg(void);
static void mov_ded_rgd(void);
static void mov_ded_in(void);
static void mov_ded_ind(void);
static void mov_ded_de(void);
static void mov_ded_ded(void);
static void mov_ded_ix(void);
static void mov_ded_ixd(void);
static void mov_ix_rg(void);
static void mov_ix_rgd(void);
static void mov_ix_in(void);
static void mov_ix_ind(void);
static void mov_ix_de(void);
static void mov_ix_ded(void);
static void mov_ix_ix(void);
static void mov_ix_ixd(void);
static void mov_ixd_rg(void);
static void mov_ixd_rgd(void);
static void mov_ixd_in(void);
static void mov_ixd_ind(void);
static void mov_ixd_de(void);
static void mov_ixd_ded(void);
static void mov_ixd_ix(void);
static void mov_ixd_ixd(void);

static void cmp_rg_rg(void);
static void cmp_rg_rgd(void);
static void cmp_rg_in(void);
static void cmp_rg_ind(void);
static void cmp_rg_de(void);
static void cmp_rg_ded(void);
static void cmp_rg_ix(void);
static void cmp_rg_ixd(void);
static void cmp_rgd_rg(void);
static void cmp_rgd_rgd(void);
static void cmp_rgd_in(void);
static void cmp_rgd_ind(void);
static void cmp_rgd_de(void);
static void cmp_rgd_ded(void);
static void cmp_rgd_ix(void);
static void cmp_rgd_ixd(void);
static void cmp_in_rg(void);
static void cmp_in_rgd(void);
static void cmp_in_in(void);
static void cmp_in_ind(void);
static void cmp_in_de(void);
static void cmp_in_ded(void);
static void cmp_in_ix(void);
static void cmp_in_ixd(void);
static void cmp_ind_rg(void);
static void cmp_ind_rgd(void);
static void cmp_ind_in(void);
static void cmp_ind_ind(void);
static void cmp_ind_de(void);
static void cmp_ind_ded(void);
static void cmp_ind_ix(void);
static void cmp_ind_ixd(void);
static void cmp_de_rg(void);
static void cmp_de_rgd(void);
static void cmp_de_in(void);
static void cmp_de_ind(void);
static void cmp_de_de(void);
static void cmp_de_ded(void);
static void cmp_de_ix(void);
static void cmp_de_ixd(void);
static void cmp_ded_rg(void);
static void cmp_ded_rgd(void);
static void cmp_ded_in(void);
static void cmp_ded_ind(void);
static void cmp_ded_de(void);
static void cmp_ded_ded(void);
static void cmp_ded_ix(void);
static void cmp_ded_ixd(void);
static void cmp_ix_rg(void);
static void cmp_ix_rgd(void);
static void cmp_ix_in(void);
static void cmp_ix_ind(void);
static void cmp_ix_de(void);
static void cmp_ix_ded(void);
static void cmp_ix_ix(void);
static void cmp_ix_ixd(void);
static void cmp_ixd_rg(void);
static void cmp_ixd_rgd(void);
static void cmp_ixd_in(void);
static void cmp_ixd_ind(void);
static void cmp_ixd_de(void);
static void cmp_ixd_ded(void);
static void cmp_ixd_ix(void);
static void cmp_ixd_ixd(void);

static void bit_rg_rg(void);
static void bit_rg_rgd(void);
static void bit_rg_in(void);
static void bit_rg_ind(void);
static void bit_rg_de(void);
static void bit_rg_ded(void);
static void bit_rg_ix(void);
static void bit_rg_ixd(void);
static void bit_rgd_rg(void);
static void bit_rgd_rgd(void);
static void bit_rgd_in(void);
static void bit_rgd_ind(void);
static void bit_rgd_de(void);
static void bit_rgd_ded(void);
static void bit_rgd_ix(void);
static void bit_rgd_ixd(void);
static void bit_in_rg(void);
static void bit_in_rgd(void);
static void bit_in_in(void);
static void bit_in_ind(void);
static void bit_in_de(void);
static void bit_in_ded(void);
static void bit_in_ix(void);
static void bit_in_ixd(void);
static void bit_ind_rg(void);
static void bit_ind_rgd(void);
static void bit_ind_in(void);
static void bit_ind_ind(void);
static void bit_ind_de(void);
static void bit_ind_ded(void);
static void bit_ind_ix(void);
static void bit_ind_ixd(void);
static void bit_de_rg(void);
static void bit_de_rgd(void);
static void bit_de_in(void);
static void bit_de_ind(void);
static void bit_de_de(void);
static void bit_de_ded(void);
static void bit_de_ix(void);
static void bit_de_ixd(void);
static void bit_ded_rg(void);
static void bit_ded_rgd(void);
static void bit_ded_in(void);
static void bit_ded_ind(void);
static void bit_ded_de(void);
static void bit_ded_ded(void);
static void bit_ded_ix(void);
static void bit_ded_ixd(void);
static void bit_ix_rg(void);
static void bit_ix_rgd(void);
static void bit_ix_in(void);
static void bit_ix_ind(void);
static void bit_ix_de(void);
static void bit_ix_ded(void);
static void bit_ix_ix(void);
static void bit_ix_ixd(void);
static void bit_ixd_rg(void);
static void bit_ixd_rgd(void);
static void bit_ixd_in(void);
static void bit_ixd_ind(void);
static void bit_ixd_de(void);
static void bit_ixd_ded(void);
static void bit_ixd_ix(void);
static void bit_ixd_ixd(void);

static void bic_rg_rg(void);
static void bic_rg_rgd(void);
static void bic_rg_in(void);
static void bic_rg_ind(void);
static void bic_rg_de(void);
static void bic_rg_ded(void);
static void bic_rg_ix(void);
static void bic_rg_ixd(void);
static void bic_rgd_rg(void);
static void bic_rgd_rgd(void);
static void bic_rgd_in(void);
static void bic_rgd_ind(void);
static void bic_rgd_de(void);
static void bic_rgd_ded(void);
static void bic_rgd_ix(void);
static void bic_rgd_ixd(void);
static void bic_in_rg(void);
static void bic_in_rgd(void);
static void bic_in_in(void);
static void bic_in_ind(void);
static void bic_in_de(void);
static void bic_in_ded(void);
static void bic_in_ix(void);
static void bic_in_ixd(void);
static void bic_ind_rg(void);
static void bic_ind_rgd(void);
static void bic_ind_in(void);
static void bic_ind_ind(void);
static void bic_ind_de(void);
static void bic_ind_ded(void);
static void bic_ind_ix(void);
static void bic_ind_ixd(void);
static void bic_de_rg(void);
static void bic_de_rgd(void);
static void bic_de_in(void);
static void bic_de_ind(void);
static void bic_de_de(void);
static void bic_de_ded(void);
static void bic_de_ix(void);
static void bic_de_ixd(void);
static void bic_ded_rg(void);
static void bic_ded_rgd(void);
static void bic_ded_in(void);
static void bic_ded_ind(void);
static void bic_ded_de(void);
static void bic_ded_ded(void);
static void bic_ded_ix(void);
static void bic_ded_ixd(void);
static void bic_ix_rg(void);
static void bic_ix_rgd(void);
static void bic_ix_in(void);
static void bic_ix_ind(void);
static void bic_ix_de(void);
static void bic_ix_ded(void);
static void bic_ix_ix(void);
static void bic_ix_ixd(void);
static void bic_ixd_rg(void);
static void bic_ixd_rgd(void);
static void bic_ixd_in(void);
static void bic_ixd_ind(void);
static void bic_ixd_de(void);
static void bic_ixd_ded(void);
static void bic_ixd_ix(void);
static void bic_ixd_ixd(void);

static void bis_rg_rg(void);
static void bis_rg_rgd(void);
static void bis_rg_in(void);
static void bis_rg_ind(void);
static void bis_rg_de(void);
static void bis_rg_ded(void);
static void bis_rg_ix(void);
static void bis_rg_ixd(void);
static void bis_rgd_rg(void);
static void bis_rgd_rgd(void);
static void bis_rgd_in(void);
static void bis_rgd_ind(void);
static void bis_rgd_de(void);
static void bis_rgd_ded(void);
static void bis_rgd_ix(void);
static void bis_rgd_ixd(void);
static void bis_in_rg(void);
static void bis_in_rgd(void);
static void bis_in_in(void);
static void bis_in_ind(void);
static void bis_in_de(void);
static void bis_in_ded(void);
static void bis_in_ix(void);
static void bis_in_ixd(void);
static void bis_ind_rg(void);
static void bis_ind_rgd(void);
static void bis_ind_in(void);
static void bis_ind_ind(void);
static void bis_ind_de(void);
static void bis_ind_ded(void);
static void bis_ind_ix(void);
static void bis_ind_ixd(void);
static void bis_de_rg(void);
static void bis_de_rgd(void);
static void bis_de_in(void);
static void bis_de_ind(void);
static void bis_de_de(void);
static void bis_de_ded(void);
static void bis_de_ix(void);
static void bis_de_ixd(void);
static void bis_ded_rg(void);
static void bis_ded_rgd(void);
static void bis_ded_in(void);
static void bis_ded_ind(void);
static void bis_ded_de(void);
static void bis_ded_ded(void);
static void bis_ded_ix(void);
static void bis_ded_ixd(void);
static void bis_ix_rg(void);
static void bis_ix_rgd(void);
static void bis_ix_in(void);
static void bis_ix_ind(void);
static void bis_ix_de(void);
static void bis_ix_ded(void);
static void bis_ix_ix(void);
static void bis_ix_ixd(void);
static void bis_ixd_rg(void);
static void bis_ixd_rgd(void);
static void bis_ixd_in(void);
static void bis_ixd_ind(void);
static void bis_ixd_de(void);
static void bis_ixd_ded(void);
static void bis_ixd_ix(void);
static void bis_ixd_ixd(void);

static void add_rg_rg(void);
static void add_rg_rgd(void);
static void add_rg_in(void);
static void add_rg_ind(void);
static void add_rg_de(void);
static void add_rg_ded(void);
static void add_rg_ix(void);
static void add_rg_ixd(void);
static void add_rgd_rg(void);
static void add_rgd_rgd(void);
static void add_rgd_in(void);
static void add_rgd_ind(void);
static void add_rgd_de(void);
static void add_rgd_ded(void);
static void add_rgd_ix(void);
static void add_rgd_ixd(void);
static void add_in_rg(void);
static void add_in_rgd(void);
static void add_in_in(void);
static void add_in_ind(void);
static void add_in_de(void);
static void add_in_ded(void);
static void add_in_ix(void);
static void add_in_ixd(void);
static void add_ind_rg(void);
static void add_ind_rgd(void);
static void add_ind_in(void);
static void add_ind_ind(void);
static void add_ind_de(void);
static void add_ind_ded(void);
static void add_ind_ix(void);
static void add_ind_ixd(void);
static void add_de_rg(void);
static void add_de_rgd(void);
static void add_de_in(void);
static void add_de_ind(void);
static void add_de_de(void);
static void add_de_ded(void);
static void add_de_ix(void);
static void add_de_ixd(void);
static void add_ded_rg(void);
static void add_ded_rgd(void);
static void add_ded_in(void);
static void add_ded_ind(void);
static void add_ded_de(void);
static void add_ded_ded(void);
static void add_ded_ix(void);
static void add_ded_ixd(void);
static void add_ix_rg(void);
static void add_ix_rgd(void);
static void add_ix_in(void);
static void add_ix_ind(void);
static void add_ix_de(void);
static void add_ix_ded(void);
static void add_ix_ix(void);
static void add_ix_ixd(void);
static void add_ixd_rg(void);
static void add_ixd_rgd(void);
static void add_ixd_in(void);
static void add_ixd_ind(void);
static void add_ixd_de(void);
static void add_ixd_ded(void);
static void add_ixd_ix(void);
static void add_ixd_ixd(void);

static void xor_rg(void);
static void xor_rgd(void);
static void xor_in(void);
static void xor_ind(void);
static void xor_de(void);
static void xor_ded(void);
static void xor_ix(void);
static void xor_ixd(void);

static void sob(void);

static void bpl(void);
static void bmi(void);
static void bhi(void);
static void blos(void);
static void bvc(void);
static void bvs(void);
static void bcc(void);
static void bcs(void);
static void emt(void);
static void trap(void);

static void clrb_rg(void);
static void clrb_rgd(void);
static void clrb_in(void);
static void clrb_ind(void);
static void clrb_de(void);
static void clrb_ded(void);
static void clrb_ix(void);
static void clrb_ixd(void);

static void comb_rg(void);
static void comb_rgd(void);
static void comb_in(void);
static void comb_ind(void);
static void comb_de(void);
static void comb_ded(void);
static void comb_ix(void);
static void comb_ixd(void);

static void incb_rg(void);
static void incb_rgd(void);
static void incb_in(void);
static void incb_ind(void);
static void incb_de(void);
static void incb_ded(void);
static void incb_ix(void);
static void incb_ixd(void);

static void decb_rg(void);
static void decb_rgd(void);
static void decb_in(void);
static void decb_ind(void);
static void decb_de(void);
static void decb_ded(void);
static void decb_ix(void);
static void decb_ixd(void);

static void negb_rg(void);
static void negb_rgd(void);
static void negb_in(void);
static void negb_ind(void);
static void negb_de(void);
static void negb_ded(void);
static void negb_ix(void);
static void negb_ixd(void);

static void adcb_rg(void);
static void adcb_rgd(void);
static void adcb_in(void);
static void adcb_ind(void);
static void adcb_de(void);
static void adcb_ded(void);
static void adcb_ix(void);
static void adcb_ixd(void);

static void sbcb_rg(void);
static void sbcb_rgd(void);
static void sbcb_in(void);
static void sbcb_ind(void);
static void sbcb_de(void);
static void sbcb_ded(void);
static void sbcb_ix(void);
static void sbcb_ixd(void);

static void tstb_rg(void);
static void tstb_rgd(void);
static void tstb_in(void);
static void tstb_ind(void);
static void tstb_de(void);
static void tstb_ded(void);
static void tstb_ix(void);
static void tstb_ixd(void);

static void rorb_rg(void);
static void rorb_rgd(void);
static void rorb_in(void);
static void rorb_ind(void);
static void rorb_de(void);
static void rorb_ded(void);
static void rorb_ix(void);
static void rorb_ixd(void);

static void rolb_rg(void);
static void rolb_rgd(void);
static void rolb_in(void);
static void rolb_ind(void);
static void rolb_de(void);
static void rolb_ded(void);
static void rolb_ix(void);
static void rolb_ixd(void);

static void asrb_rg(void);
static void asrb_rgd(void);
static void asrb_in(void);
static void asrb_ind(void);
static void asrb_de(void);
static void asrb_ded(void);
static void asrb_ix(void);
static void asrb_ixd(void);

static void aslb_rg(void);
static void aslb_rgd(void);
static void aslb_in(void);
static void aslb_ind(void);
static void aslb_de(void);
static void aslb_ded(void);
static void aslb_ix(void);
static void aslb_ixd(void);

static void mtps_rg(void);
static void mtps_rgd(void);
static void mtps_in(void);
static void mtps_ind(void);
static void mtps_de(void);
static void mtps_ded(void);
static void mtps_ix(void);
static void mtps_ixd(void);

static void mfps_rg(void);
static void mfps_rgd(void);
static void mfps_in(void);
static void mfps_ind(void);
static void mfps_de(void);
static void mfps_ded(void);
static void mfps_ix(void);
static void mfps_ixd(void);

static void movb_rg_rg(void);
static void movb_rg_rgd(void);
static void movb_rg_in(void);
static void movb_rg_ind(void);
static void movb_rg_de(void);
static void movb_rg_ded(void);
static void movb_rg_ix(void);
static void movb_rg_ixd(void);
static void movb_rgd_rg(void);
static void movb_rgd_rgd(void);
static void movb_rgd_in(void);
static void movb_rgd_ind(void);
static void movb_rgd_de(void);
static void movb_rgd_ded(void);
static void movb_rgd_ix(void);
static void movb_rgd_ixd(void);
static void movb_in_rg(void);
static void movb_in_rgd(void);
static void movb_in_in(void);
static void movb_in_ind(void);
static void movb_in_de(void);
static void movb_in_ded(void);
static void movb_in_ix(void);
static void movb_in_ixd(void);
static void movb_ind_rg(void);
static void movb_ind_rgd(void);
static void movb_ind_in(void);
static void movb_ind_ind(void);
static void movb_ind_de(void);
static void movb_ind_ded(void);
static void movb_ind_ix(void);
static void movb_ind_ixd(void);
static void movb_de_rg(void);
static void movb_de_rgd(void);
static void movb_de_in(void);
static void movb_de_ind(void);
static void movb_de_de(void);
static void movb_de_ded(void);
static void movb_de_ix(void);
static void movb_de_ixd(void);
static void movb_ded_rg(void);
static void movb_ded_rgd(void);
static void movb_ded_in(void);
static void movb_ded_ind(void);
static void movb_ded_de(void);
static void movb_ded_ded(void);
static void movb_ded_ix(void);
static void movb_ded_ixd(void);
static void movb_ix_rg(void);
static void movb_ix_rgd(void);
static void movb_ix_in(void);
static void movb_ix_ind(void);
static void movb_ix_de(void);
static void movb_ix_ded(void);
static void movb_ix_ix(void);
static void movb_ix_ixd(void);
static void movb_ixd_rg(void);
static void movb_ixd_rgd(void);
static void movb_ixd_in(void);
static void movb_ixd_ind(void);
static void movb_ixd_de(void);
static void movb_ixd_ded(void);
static void movb_ixd_ix(void);
static void movb_ixd_ixd(void);

static void cmpb_rg_rg(void);
static void cmpb_rg_rgd(void);
static void cmpb_rg_in(void);
static void cmpb_rg_ind(void);
static void cmpb_rg_de(void);
static void cmpb_rg_ded(void);
static void cmpb_rg_ix(void);
static void cmpb_rg_ixd(void);
static void cmpb_rgd_rg(void);
static void cmpb_rgd_rgd(void);
static void cmpb_rgd_in(void);
static void cmpb_rgd_ind(void);
static void cmpb_rgd_de(void);
static void cmpb_rgd_ded(void);
static void cmpb_rgd_ix(void);
static void cmpb_rgd_ixd(void);
static void cmpb_in_rg(void);
static void cmpb_in_rgd(void);
static void cmpb_in_in(void);
static void cmpb_in_ind(void);
static void cmpb_in_de(void);
static void cmpb_in_ded(void);
static void cmpb_in_ix(void);
static void cmpb_in_ixd(void);
static void cmpb_ind_rg(void);
static void cmpb_ind_rgd(void);
static void cmpb_ind_in(void);
static void cmpb_ind_ind(void);
static void cmpb_ind_de(void);
static void cmpb_ind_ded(void);
static void cmpb_ind_ix(void);
static void cmpb_ind_ixd(void);
static void cmpb_de_rg(void);
static void cmpb_de_rgd(void);
static void cmpb_de_in(void);
static void cmpb_de_ind(void);
static void cmpb_de_de(void);
static void cmpb_de_ded(void);
static void cmpb_de_ix(void);
static void cmpb_de_ixd(void);
static void cmpb_ded_rg(void);
static void cmpb_ded_rgd(void);
static void cmpb_ded_in(void);
static void cmpb_ded_ind(void);
static void cmpb_ded_de(void);
static void cmpb_ded_ded(void);
static void cmpb_ded_ix(void);
static void cmpb_ded_ixd(void);
static void cmpb_ix_rg(void);
static void cmpb_ix_rgd(void);
static void cmpb_ix_in(void);
static void cmpb_ix_ind(void);
static void cmpb_ix_de(void);
static void cmpb_ix_ded(void);
static void cmpb_ix_ix(void);
static void cmpb_ix_ixd(void);
static void cmpb_ixd_rg(void);
static void cmpb_ixd_rgd(void);
static void cmpb_ixd_in(void);
static void cmpb_ixd_ind(void);
static void cmpb_ixd_de(void);
static void cmpb_ixd_ded(void);
static void cmpb_ixd_ix(void);
static void cmpb_ixd_ixd(void);

static void bitb_rg_rg(void);
static void bitb_rg_rgd(void);
static void bitb_rg_in(void);
static void bitb_rg_ind(void);
static void bitb_rg_de(void);
static void bitb_rg_ded(void);
static void bitb_rg_ix(void);
static void bitb_rg_ixd(void);
static void bitb_rgd_rg(void);
static void bitb_rgd_rgd(void);
static void bitb_rgd_in(void);
static void bitb_rgd_ind(void);
static void bitb_rgd_de(void);
static void bitb_rgd_ded(void);
static void bitb_rgd_ix(void);
static void bitb_rgd_ixd(void);
static void bitb_in_rg(void);
static void bitb_in_rgd(void);
static void bitb_in_in(void);
static void bitb_in_ind(void);
static void bitb_in_de(void);
static void bitb_in_ded(void);
static void bitb_in_ix(void);
static void bitb_in_ixd(void);
static void bitb_ind_rg(void);
static void bitb_ind_rgd(void);
static void bitb_ind_in(void);
static void bitb_ind_ind(void);
static void bitb_ind_de(void);
static void bitb_ind_ded(void);
static void bitb_ind_ix(void);
static void bitb_ind_ixd(void);
static void bitb_de_rg(void);
static void bitb_de_rgd(void);
static void bitb_de_in(void);
static void bitb_de_ind(void);
static void bitb_de_de(void);
static void bitb_de_ded(void);
static void bitb_de_ix(void);
static void bitb_de_ixd(void);
static void bitb_ded_rg(void);
static void bitb_ded_rgd(void);
static void bitb_ded_in(void);
static void bitb_ded_ind(void);
static void bitb_ded_de(void);
static void bitb_ded_ded(void);
static void bitb_ded_ix(void);
static void bitb_ded_ixd(void);
static void bitb_ix_rg(void);
static void bitb_ix_rgd(void);
static void bitb_ix_in(void);
static void bitb_ix_ind(void);
static void bitb_ix_de(void);
static void bitb_ix_ded(void);
static void bitb_ix_ix(void);
static void bitb_ix_ixd(void);
static void bitb_ixd_rg(void);
static void bitb_ixd_rgd(void);
static void bitb_ixd_in(void);
static void bitb_ixd_ind(void);
static void bitb_ixd_de(void);
static void bitb_ixd_ded(void);
static void bitb_ixd_ix(void);
static void bitb_ixd_ixd(void);

static void bicb_rg_rg(void);
static void bicb_rg_rgd(void);
static void bicb_rg_in(void);
static void bicb_rg_ind(void);
static void bicb_rg_de(void);
static void bicb_rg_ded(void);
static void bicb_rg_ix(void);
static void bicb_rg_ixd(void);
static void bicb_rgd_rg(void);
static void bicb_rgd_rgd(void);
static void bicb_rgd_in(void);
static void bicb_rgd_ind(void);
static void bicb_rgd_de(void);
static void bicb_rgd_ded(void);
static void bicb_rgd_ix(void);
static void bicb_rgd_ixd(void);
static void bicb_in_rg(void);
static void bicb_in_rgd(void);
static void bicb_in_in(void);
static void bicb_in_ind(void);
static void bicb_in_de(void);
static void bicb_in_ded(void);
static void bicb_in_ix(void);
static void bicb_in_ixd(void);
static void bicb_ind_rg(void);
static void bicb_ind_rgd(void);
static void bicb_ind_in(void);
static void bicb_ind_ind(void);
static void bicb_ind_de(void);
static void bicb_ind_ded(void);
static void bicb_ind_ix(void);
static void bicb_ind_ixd(void);
static void bicb_de_rg(void);
static void bicb_de_rgd(void);
static void bicb_de_in(void);
static void bicb_de_ind(void);
static void bicb_de_de(void);
static void bicb_de_ded(void);
static void bicb_de_ix(void);
static void bicb_de_ixd(void);
static void bicb_ded_rg(void);
static void bicb_ded_rgd(void);
static void bicb_ded_in(void);
static void bicb_ded_ind(void);
static void bicb_ded_de(void);
static void bicb_ded_ded(void);
static void bicb_ded_ix(void);
static void bicb_ded_ixd(void);
static void bicb_ix_rg(void);
static void bicb_ix_rgd(void);
static void bicb_ix_in(void);
static void bicb_ix_ind(void);
static void bicb_ix_de(void);
static void bicb_ix_ded(void);
static void bicb_ix_ix(void);
static void bicb_ix_ixd(void);
static void bicb_ixd_rg(void);
static void bicb_ixd_rgd(void);
static void bicb_ixd_in(void);
static void bicb_ixd_ind(void);
static void bicb_ixd_de(void);
static void bicb_ixd_ded(void);
static void bicb_ixd_ix(void);
static void bicb_ixd_ixd(void);

static void bisb_rg_rg(void);
static void bisb_rg_rgd(void);
static void bisb_rg_in(void);
static void bisb_rg_ind(void);
static void bisb_rg_de(void);
static void bisb_rg_ded(void);
static void bisb_rg_ix(void);
static void bisb_rg_ixd(void);
static void bisb_rgd_rg(void);
static void bisb_rgd_rgd(void);
static void bisb_rgd_in(void);
static void bisb_rgd_ind(void);
static void bisb_rgd_de(void);
static void bisb_rgd_ded(void);
static void bisb_rgd_ix(void);
static void bisb_rgd_ixd(void);
static void bisb_in_rg(void);
static void bisb_in_rgd(void);
static void bisb_in_in(void);
static void bisb_in_ind(void);
static void bisb_in_de(void);
static void bisb_in_ded(void);
static void bisb_in_ix(void);
static void bisb_in_ixd(void);
static void bisb_ind_rg(void);
static void bisb_ind_rgd(void);
static void bisb_ind_in(void);
static void bisb_ind_ind(void);
static void bisb_ind_de(void);
static void bisb_ind_ded(void);
static void bisb_ind_ix(void);
static void bisb_ind_ixd(void);
static void bisb_de_rg(void);
static void bisb_de_rgd(void);
static void bisb_de_in(void);
static void bisb_de_ind(void);
static void bisb_de_de(void);
static void bisb_de_ded(void);
static void bisb_de_ix(void);
static void bisb_de_ixd(void);
static void bisb_ded_rg(void);
static void bisb_ded_rgd(void);
static void bisb_ded_in(void);
static void bisb_ded_ind(void);
static void bisb_ded_de(void);
static void bisb_ded_ded(void);
static void bisb_ded_ix(void);
static void bisb_ded_ixd(void);
static void bisb_ix_rg(void);
static void bisb_ix_rgd(void);
static void bisb_ix_in(void);
static void bisb_ix_ind(void);
static void bisb_ix_de(void);
static void bisb_ix_ded(void);
static void bisb_ix_ix(void);
static void bisb_ix_ixd(void);
static void bisb_ixd_rg(void);
static void bisb_ixd_rgd(void);
static void bisb_ixd_in(void);
static void bisb_ixd_ind(void);
static void bisb_ixd_de(void);
static void bisb_ixd_ded(void);
static void bisb_ixd_ix(void);
static void bisb_ixd_ixd(void);

static void sub_rg_rg(void);
static void sub_rg_rgd(void);
static void sub_rg_in(void);
static void sub_rg_ind(void);
static void sub_rg_de(void);
static void sub_rg_ded(void);
static void sub_rg_ix(void);
static void sub_rg_ixd(void);
static void sub_rgd_rg(void);
static void sub_rgd_rgd(void);
static void sub_rgd_in(void);
static void sub_rgd_ind(void);
static void sub_rgd_de(void);
static void sub_rgd_ded(void);
static void sub_rgd_ix(void);
static void sub_rgd_ixd(void);
static void sub_in_rg(void);
static void sub_in_rgd(void);
static void sub_in_in(void);
static void sub_in_ind(void);
static void sub_in_de(void);
static void sub_in_ded(void);
static void sub_in_ix(void);
static void sub_in_ixd(void);
static void sub_ind_rg(void);
static void sub_ind_rgd(void);
static void sub_ind_in(void);
static void sub_ind_ind(void);
static void sub_ind_de(void);
static void sub_ind_ded(void);
static void sub_ind_ix(void);
static void sub_ind_ixd(void);
static void sub_de_rg(void);
static void sub_de_rgd(void);
static void sub_de_in(void);
static void sub_de_ind(void);
static void sub_de_de(void);
static void sub_de_ded(void);
static void sub_de_ix(void);
static void sub_de_ixd(void);
static void sub_ded_rg(void);
static void sub_ded_rgd(void);
static void sub_ded_in(void);
static void sub_ded_ind(void);
static void sub_ded_de(void);
static void sub_ded_ded(void);
static void sub_ded_ix(void);
static void sub_ded_ixd(void);
static void sub_ix_rg(void);
static void sub_ix_rgd(void);
static void sub_ix_in(void);
static void sub_ix_ind(void);
static void sub_ix_de(void);
static void sub_ix_ded(void);
static void sub_ix_ix(void);
static void sub_ix_ixd(void);
static void sub_ixd_rg(void);
static void sub_ixd_rgd(void);
static void sub_ixd_in(void);
static void sub_ixd_ind(void);
static void sub_ixd_de(void);
static void sub_ixd_ded(void);
static void sub_ixd_ix(void);
static void sub_ixd_ixd(void);



static void (*opcode_table[65536 >> 3])(void) =
{
	/* 0x0000 */
	op_0000,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	jmp_rgd,	jmp_in,		jmp_ind,	jmp_de,		jmp_ded,	jmp_ix,		jmp_ixd,
	rts,		illegal,	illegal,	illegal,	ccc,		ccc,		scc,		scc,
	swab_rg,	swab_rgd,	swab_in,	swab_ind,	swab_de,	swab_ded,	swab_ix,	swab_ixd,
	/* 0x0100 */
	br,			br,			br,			br,			br,			br,			br,			br,
	br,			br,			br,			br,			br,			br,			br,			br,
	br,			br,			br,			br,			br,			br,			br,			br,
	br,			br,			br,			br,			br,			br,			br,			br,
	/* 0x0200 */
	bne,		bne,		bne,		bne,		bne,		bne,		bne,		bne,
	bne,		bne,		bne,		bne,		bne,		bne,		bne,		bne,
	bne,		bne,		bne,		bne,		bne,		bne,		bne,		bne,
	bne,		bne,		bne,		bne,		bne,		bne,		bne,		bne,
	/* 0x0300 */
	beq,		beq,		beq,		beq,		beq,		beq,		beq,		beq,
	beq,		beq,		beq,		beq,		beq,		beq,		beq,		beq,
	beq,		beq,		beq,		beq,		beq,		beq,		beq,		beq,
	beq,		beq,		beq,		beq,		beq,		beq,		beq,		beq,
	/* 0x0400 */
	bge,		bge,		bge,		bge,		bge,		bge,		bge,		bge,
	bge,		bge,		bge,		bge,		bge,		bge,		bge,		bge,
	bge,		bge,		bge,		bge,		bge,		bge,		bge,		bge,
	bge,		bge,		bge,		bge,		bge,		bge,		bge,		bge,
	/* 0x0500 */
	blt,		blt,		blt,		blt,		blt,		blt,		blt,		blt,
	blt,		blt,		blt,		blt,		blt,		blt,		blt,		blt,
	blt,		blt,		blt,		blt,		blt,		blt,		blt,		blt,
	blt,		blt,		blt,		blt,		blt,		blt,		blt,		blt,
	/* 0x0600 */
	bgt,		bgt,		bgt,		bgt,		bgt,		bgt,		bgt,		bgt,
	bgt,		bgt,		bgt,		bgt,		bgt,		bgt,		bgt,		bgt,
	bgt,		bgt,		bgt,		bgt,		bgt,		bgt,		bgt,		bgt,
	bgt,		bgt,		bgt,		bgt,		bgt,		bgt,		bgt,		bgt,
	/* 0x0700 */
	ble,		ble,		ble,		ble,		ble,		ble,		ble,		ble,
	ble,		ble,		ble,		ble,		ble,		ble,		ble,		ble,
	ble,		ble,		ble,		ble,		ble,		ble,		ble,		ble,
	ble,		ble,		ble,		ble,		ble,		ble,		ble,		ble,
	/* 0x0800 */
	illegal,	jsr_rgd,	jsr_in,		jsr_ind,	jsr_de,		jsr_ded,	jsr_ix,		jsr_ixd,
	illegal,	jsr_rgd,	jsr_in,		jsr_ind,	jsr_de,		jsr_ded,	jsr_ix,		jsr_ixd,
	illegal,	jsr_rgd,	jsr_in,		jsr_ind,	jsr_de,		jsr_ded,	jsr_ix,		jsr_ixd,
	illegal,	jsr_rgd,	jsr_in,		jsr_ind,	jsr_de,		jsr_ded,	jsr_ix,		jsr_ixd,
	/* 0x0900 */
	illegal,	jsr_rgd,	jsr_in,		jsr_ind,	jsr_de,		jsr_ded,	jsr_ix,		jsr_ixd,
	illegal,	jsr_rgd,	jsr_in,		jsr_ind,	jsr_de,		jsr_ded,	jsr_ix,		jsr_ixd,
	illegal,	jsr_rgd,	jsr_in,		jsr_ind,	jsr_de,		jsr_ded,	jsr_ix,		jsr_ixd,
	illegal,	jsr_rgd,	jsr_in,		jsr_ind,	jsr_de,		jsr_ded,	jsr_ix,		jsr_ixd,
	/* 0x0a00 */
	clr_rg,		clr_rgd,	clr_in,		clr_ind,	clr_de,		clr_ded,	clr_ix,		clr_ixd,
	com_rg,		com_rgd,	com_in,		com_ind,	com_de,		com_ded,	com_ix,		com_ixd,
	inc_rg,		inc_rgd,	inc_in,		inc_ind,	inc_de,		inc_ded,	inc_ix,		inc_ixd,
	dec_rg,		dec_rgd,	dec_in,		dec_ind,	dec_de,		dec_ded,	dec_ix,		dec_ixd,
	/* 0x0b00 */
	neg_rg,		neg_rgd,	neg_in,		neg_ind,	neg_de,		neg_ded,	neg_ix,		neg_ixd,
	adc_rg,		adc_rgd,	adc_in,		adc_ind,	adc_de,		adc_ded,	adc_ix,		adc_ixd,
	sbc_rg,		sbc_rgd,	sbc_in,		sbc_ind,	sbc_de,		sbc_ded,	sbc_ix,		sbc_ixd,
	tst_rg,		tst_rgd,	tst_in,		tst_ind,	tst_de,		tst_ded,	tst_ix,		tst_ixd,
	/* 0x0c00 */
	ror_rg,		ror_rgd,	ror_in,		ror_ind,	ror_de,		ror_ded,	ror_ix,		ror_ixd,
	rol_rg,		rol_rgd,	rol_in,		rol_ind,	rol_de,		rol_ded,	rol_ix,		rol_ixd,
	asr_rg,		asr_rgd,	asr_in,		asr_ind,	asr_de,		asr_ded,	asr_ix,		asr_ixd,
	asl_rg,		asl_rgd,	asl_in,		asl_ind,	asl_de,		asl_ded,	asl_ix,		asl_ixd,
	/* 0x0d00 */
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	sxt_rg,		sxt_rgd,	sxt_in,		sxt_ind,	sxt_de,		sxt_ded,	sxt_ix,		sxt_ixd,
	/* 0x0e00 */
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	/* 0x0f00 */
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,

	/* 0x1000 */
	mov_rg_rg,	mov_rg_rgd,	mov_rg_in,	mov_rg_ind,	mov_rg_de,	mov_rg_ded,	mov_rg_ix,	mov_rg_ixd,
	mov_rg_rg,	mov_rg_rgd,	mov_rg_in,	mov_rg_ind,	mov_rg_de,	mov_rg_ded,	mov_rg_ix,	mov_rg_ixd,
	mov_rg_rg,	mov_rg_rgd,	mov_rg_in,	mov_rg_ind,	mov_rg_de,	mov_rg_ded,	mov_rg_ix,	mov_rg_ixd,
	mov_rg_rg,	mov_rg_rgd,	mov_rg_in,	mov_rg_ind,	mov_rg_de,	mov_rg_ded,	mov_rg_ix,	mov_rg_ixd,
	/* 0x1100 */
	mov_rg_rg,	mov_rg_rgd,	mov_rg_in,	mov_rg_ind,	mov_rg_de,	mov_rg_ded,	mov_rg_ix,	mov_rg_ixd,
	mov_rg_rg,	mov_rg_rgd,	mov_rg_in,	mov_rg_ind,	mov_rg_de,	mov_rg_ded,	mov_rg_ix,	mov_rg_ixd,
	mov_rg_rg,	mov_rg_rgd,	mov_rg_in,	mov_rg_ind,	mov_rg_de,	mov_rg_ded,	mov_rg_ix,	mov_rg_ixd,
	mov_rg_rg,	mov_rg_rgd,	mov_rg_in,	mov_rg_ind,	mov_rg_de,	mov_rg_ded,	mov_rg_ix,	mov_rg_ixd,
	/* 0x1200 */
	mov_rgd_rg,	mov_rgd_rgd,mov_rgd_in,	mov_rgd_ind,mov_rgd_de,	mov_rgd_ded,mov_rgd_ix,	mov_rgd_ixd,
	mov_rgd_rg,	mov_rgd_rgd,mov_rgd_in,	mov_rgd_ind,mov_rgd_de,	mov_rgd_ded,mov_rgd_ix,	mov_rgd_ixd,
	mov_rgd_rg,	mov_rgd_rgd,mov_rgd_in,	mov_rgd_ind,mov_rgd_de,	mov_rgd_ded,mov_rgd_ix,	mov_rgd_ixd,
	mov_rgd_rg,	mov_rgd_rgd,mov_rgd_in,	mov_rgd_ind,mov_rgd_de,	mov_rgd_ded,mov_rgd_ix,	mov_rgd_ixd,
	/* 0x1300 */
	mov_rgd_rg,	mov_rgd_rgd,mov_rgd_in,	mov_rgd_ind,mov_rgd_de,	mov_rgd_ded,mov_rgd_ix,	mov_rgd_ixd,
	mov_rgd_rg,	mov_rgd_rgd,mov_rgd_in,	mov_rgd_ind,mov_rgd_de,	mov_rgd_ded,mov_rgd_ix,	mov_rgd_ixd,
	mov_rgd_rg,	mov_rgd_rgd,mov_rgd_in,	mov_rgd_ind,mov_rgd_de,	mov_rgd_ded,mov_rgd_ix,	mov_rgd_ixd,
	mov_rgd_rg,	mov_rgd_rgd,mov_rgd_in,	mov_rgd_ind,mov_rgd_de,	mov_rgd_ded,mov_rgd_ix,	mov_rgd_ixd,
	/* 0x1400 */
	mov_in_rg,	mov_in_rgd,	mov_in_in,	mov_in_ind,	mov_in_de,	mov_in_ded,	mov_in_ix,	mov_in_ixd,
	mov_in_rg,	mov_in_rgd,	mov_in_in,	mov_in_ind,	mov_in_de,	mov_in_ded,	mov_in_ix,	mov_in_ixd,
	mov_in_rg,	mov_in_rgd,	mov_in_in,	mov_in_ind,	mov_in_de,	mov_in_ded,	mov_in_ix,	mov_in_ixd,
	mov_in_rg,	mov_in_rgd,	mov_in_in,	mov_in_ind,	mov_in_de,	mov_in_ded,	mov_in_ix,	mov_in_ixd,
	/* 0x1500 */
	mov_in_rg,	mov_in_rgd,	mov_in_in,	mov_in_ind,	mov_in_de,	mov_in_ded,	mov_in_ix,	mov_in_ixd,
	mov_in_rg,	mov_in_rgd,	mov_in_in,	mov_in_ind,	mov_in_de,	mov_in_ded,	mov_in_ix,	mov_in_ixd,
	mov_in_rg,	mov_in_rgd,	mov_in_in,	mov_in_ind,	mov_in_de,	mov_in_ded,	mov_in_ix,	mov_in_ixd,
	mov_in_rg,	mov_in_rgd,	mov_in_in,	mov_in_ind,	mov_in_de,	mov_in_ded,	mov_in_ix,	mov_in_ixd,
	/* 0x1600 */
	mov_ind_rg,	mov_ind_rgd,mov_ind_in,	mov_ind_ind,mov_ind_de,	mov_ind_ded,mov_ind_ix,	mov_ind_ixd,
	mov_ind_rg,	mov_ind_rgd,mov_ind_in,	mov_ind_ind,mov_ind_de,	mov_ind_ded,mov_ind_ix,	mov_ind_ixd,
	mov_ind_rg,	mov_ind_rgd,mov_ind_in,	mov_ind_ind,mov_ind_de,	mov_ind_ded,mov_ind_ix,	mov_ind_ixd,
	mov_ind_rg,	mov_ind_rgd,mov_ind_in,	mov_ind_ind,mov_ind_de,	mov_ind_ded,mov_ind_ix,	mov_ind_ixd,
	/* 0x1700 */
	mov_ind_rg,	mov_ind_rgd,mov_ind_in,	mov_ind_ind,mov_ind_de,	mov_ind_ded,mov_ind_ix,	mov_ind_ixd,
	mov_ind_rg,	mov_ind_rgd,mov_ind_in,	mov_ind_ind,mov_ind_de,	mov_ind_ded,mov_ind_ix,	mov_ind_ixd,
	mov_ind_rg,	mov_ind_rgd,mov_ind_in,	mov_ind_ind,mov_ind_de,	mov_ind_ded,mov_ind_ix,	mov_ind_ixd,
	mov_ind_rg,	mov_ind_rgd,mov_ind_in,	mov_ind_ind,mov_ind_de,	mov_ind_ded,mov_ind_ix,	mov_ind_ixd,
	/* 0x1800 */
	mov_de_rg,	mov_de_rgd,	mov_de_in,	mov_de_ind,	mov_de_de,	mov_de_ded,	mov_de_ix,	mov_de_ixd,
	mov_de_rg,	mov_de_rgd,	mov_de_in,	mov_de_ind,	mov_de_de,	mov_de_ded,	mov_de_ix,	mov_de_ixd,
	mov_de_rg,	mov_de_rgd,	mov_de_in,	mov_de_ind,	mov_de_de,	mov_de_ded,	mov_de_ix,	mov_de_ixd,
	mov_de_rg,	mov_de_rgd,	mov_de_in,	mov_de_ind,	mov_de_de,	mov_de_ded,	mov_de_ix,	mov_de_ixd,
	/* 0x1900 */
	mov_de_rg,	mov_de_rgd,	mov_de_in,	mov_de_ind,	mov_de_de,	mov_de_ded,	mov_de_ix,	mov_de_ixd,
	mov_de_rg,	mov_de_rgd,	mov_de_in,	mov_de_ind,	mov_de_de,	mov_de_ded,	mov_de_ix,	mov_de_ixd,
	mov_de_rg,	mov_de_rgd,	mov_de_in,	mov_de_ind,	mov_de_de,	mov_de_ded,	mov_de_ix,	mov_de_ixd,
	mov_de_rg,	mov_de_rgd,	mov_de_in,	mov_de_ind,	mov_de_de,	mov_de_ded,	mov_de_ix,	mov_de_ixd,
	/* 0x1a00 */
	mov_ded_rg,	mov_ded_rgd,mov_ded_in,	mov_ded_ind,mov_ded_de,	mov_ded_ded,mov_ded_ix,	mov_ded_ixd,
	mov_ded_rg,	mov_ded_rgd,mov_ded_in,	mov_ded_ind,mov_ded_de,	mov_ded_ded,mov_ded_ix,	mov_ded_ixd,
	mov_ded_rg,	mov_ded_rgd,mov_ded_in,	mov_ded_ind,mov_ded_de,	mov_ded_ded,mov_ded_ix,	mov_ded_ixd,
	mov_ded_rg,	mov_ded_rgd,mov_ded_in,	mov_ded_ind,mov_ded_de,	mov_ded_ded,mov_ded_ix,	mov_ded_ixd,
	/* 0x1b00 */
	mov_ded_rg,	mov_ded_rgd,mov_ded_in,	mov_ded_ind,mov_ded_de,	mov_ded_ded,mov_ded_ix,	mov_ded_ixd,
	mov_ded_rg,	mov_ded_rgd,mov_ded_in,	mov_ded_ind,mov_ded_de,	mov_ded_ded,mov_ded_ix,	mov_ded_ixd,
	mov_ded_rg,	mov_ded_rgd,mov_ded_in,	mov_ded_ind,mov_ded_de,	mov_ded_ded,mov_ded_ix,	mov_ded_ixd,
	mov_ded_rg,	mov_ded_rgd,mov_ded_in,	mov_ded_ind,mov_ded_de,	mov_ded_ded,mov_ded_ix,	mov_ded_ixd,
	/* 0x1c00 */
	mov_ix_rg,	mov_ix_rgd,	mov_ix_in,	mov_ix_ind,	mov_ix_de,	mov_ix_ded,	mov_ix_ix,	mov_ix_ixd,
	mov_ix_rg,	mov_ix_rgd,	mov_ix_in,	mov_ix_ind,	mov_ix_de,	mov_ix_ded,	mov_ix_ix,	mov_ix_ixd,
	mov_ix_rg,	mov_ix_rgd,	mov_ix_in,	mov_ix_ind,	mov_ix_de,	mov_ix_ded,	mov_ix_ix,	mov_ix_ixd,
	mov_ix_rg,	mov_ix_rgd,	mov_ix_in,	mov_ix_ind,	mov_ix_de,	mov_ix_ded,	mov_ix_ix,	mov_ix_ixd,
	/* 0x1d00 */
	mov_ix_rg,	mov_ix_rgd,	mov_ix_in,	mov_ix_ind,	mov_ix_de,	mov_ix_ded,	mov_ix_ix,	mov_ix_ixd,
	mov_ix_rg,	mov_ix_rgd,	mov_ix_in,	mov_ix_ind,	mov_ix_de,	mov_ix_ded,	mov_ix_ix,	mov_ix_ixd,
	mov_ix_rg,	mov_ix_rgd,	mov_ix_in,	mov_ix_ind,	mov_ix_de,	mov_ix_ded,	mov_ix_ix,	mov_ix_ixd,
	mov_ix_rg,	mov_ix_rgd,	mov_ix_in,	mov_ix_ind,	mov_ix_de,	mov_ix_ded,	mov_ix_ix,	mov_ix_ixd,
	/* 0x1e00 */
	mov_ixd_rg,	mov_ixd_rgd,mov_ixd_in,	mov_ixd_ind,mov_ixd_de,	mov_ixd_ded,mov_ixd_ix,	mov_ixd_ixd,
	mov_ixd_rg,	mov_ixd_rgd,mov_ixd_in,	mov_ixd_ind,mov_ixd_de,	mov_ixd_ded,mov_ixd_ix,	mov_ixd_ixd,
	mov_ixd_rg,	mov_ixd_rgd,mov_ixd_in,	mov_ixd_ind,mov_ixd_de,	mov_ixd_ded,mov_ixd_ix,	mov_ixd_ixd,
	mov_ixd_rg,	mov_ixd_rgd,mov_ixd_in,	mov_ixd_ind,mov_ixd_de,	mov_ixd_ded,mov_ixd_ix,	mov_ixd_ixd,
	/* 0x1f00 */
	mov_ixd_rg,	mov_ixd_rgd,mov_ixd_in,	mov_ixd_ind,mov_ixd_de,	mov_ixd_ded,mov_ixd_ix,	mov_ixd_ixd,
	mov_ixd_rg,	mov_ixd_rgd,mov_ixd_in,	mov_ixd_ind,mov_ixd_de,	mov_ixd_ded,mov_ixd_ix,	mov_ixd_ixd,
	mov_ixd_rg,	mov_ixd_rgd,mov_ixd_in,	mov_ixd_ind,mov_ixd_de,	mov_ixd_ded,mov_ixd_ix,	mov_ixd_ixd,
	mov_ixd_rg,	mov_ixd_rgd,mov_ixd_in,	mov_ixd_ind,mov_ixd_de,	mov_ixd_ded,mov_ixd_ix,	mov_ixd_ixd,

	/* 0x2000 */
	cmp_rg_rg,	cmp_rg_rgd,	cmp_rg_in,	cmp_rg_ind,	cmp_rg_de,	cmp_rg_ded,	cmp_rg_ix,	cmp_rg_ixd,
	cmp_rg_rg,	cmp_rg_rgd,	cmp_rg_in,	cmp_rg_ind,	cmp_rg_de,	cmp_rg_ded,	cmp_rg_ix,	cmp_rg_ixd,
	cmp_rg_rg,	cmp_rg_rgd,	cmp_rg_in,	cmp_rg_ind,	cmp_rg_de,	cmp_rg_ded,	cmp_rg_ix,	cmp_rg_ixd,
	cmp_rg_rg,	cmp_rg_rgd,	cmp_rg_in,	cmp_rg_ind,	cmp_rg_de,	cmp_rg_ded,	cmp_rg_ix,	cmp_rg_ixd,
	/* 0x2100 */
	cmp_rg_rg,	cmp_rg_rgd,	cmp_rg_in,	cmp_rg_ind,	cmp_rg_de,	cmp_rg_ded,	cmp_rg_ix,	cmp_rg_ixd,
	cmp_rg_rg,	cmp_rg_rgd,	cmp_rg_in,	cmp_rg_ind,	cmp_rg_de,	cmp_rg_ded,	cmp_rg_ix,	cmp_rg_ixd,
	cmp_rg_rg,	cmp_rg_rgd,	cmp_rg_in,	cmp_rg_ind,	cmp_rg_de,	cmp_rg_ded,	cmp_rg_ix,	cmp_rg_ixd,
	cmp_rg_rg,	cmp_rg_rgd,	cmp_rg_in,	cmp_rg_ind,	cmp_rg_de,	cmp_rg_ded,	cmp_rg_ix,	cmp_rg_ixd,
	/* 0x2200 */
	cmp_rgd_rg,	cmp_rgd_rgd,cmp_rgd_in,	cmp_rgd_ind,cmp_rgd_de,	cmp_rgd_ded,cmp_rgd_ix,	cmp_rgd_ixd,
	cmp_rgd_rg,	cmp_rgd_rgd,cmp_rgd_in,	cmp_rgd_ind,cmp_rgd_de,	cmp_rgd_ded,cmp_rgd_ix,	cmp_rgd_ixd,
	cmp_rgd_rg,	cmp_rgd_rgd,cmp_rgd_in,	cmp_rgd_ind,cmp_rgd_de,	cmp_rgd_ded,cmp_rgd_ix,	cmp_rgd_ixd,
	cmp_rgd_rg,	cmp_rgd_rgd,cmp_rgd_in,	cmp_rgd_ind,cmp_rgd_de,	cmp_rgd_ded,cmp_rgd_ix,	cmp_rgd_ixd,
	/* 0x2300 */
	cmp_rgd_rg,	cmp_rgd_rgd,cmp_rgd_in,	cmp_rgd_ind,cmp_rgd_de,	cmp_rgd_ded,cmp_rgd_ix,	cmp_rgd_ixd,
	cmp_rgd_rg,	cmp_rgd_rgd,cmp_rgd_in,	cmp_rgd_ind,cmp_rgd_de,	cmp_rgd_ded,cmp_rgd_ix,	cmp_rgd_ixd,
	cmp_rgd_rg,	cmp_rgd_rgd,cmp_rgd_in,	cmp_rgd_ind,cmp_rgd_de,	cmp_rgd_ded,cmp_rgd_ix,	cmp_rgd_ixd,
	cmp_rgd_rg,	cmp_rgd_rgd,cmp_rgd_in,	cmp_rgd_ind,cmp_rgd_de,	cmp_rgd_ded,cmp_rgd_ix,	cmp_rgd_ixd,
	/* 0x2400 */
	cmp_in_rg,	cmp_in_rgd,	cmp_in_in,	cmp_in_ind,	cmp_in_de,	cmp_in_ded,	cmp_in_ix,	cmp_in_ixd,
	cmp_in_rg,	cmp_in_rgd,	cmp_in_in,	cmp_in_ind,	cmp_in_de,	cmp_in_ded,	cmp_in_ix,	cmp_in_ixd,
	cmp_in_rg,	cmp_in_rgd,	cmp_in_in,	cmp_in_ind,	cmp_in_de,	cmp_in_ded,	cmp_in_ix,	cmp_in_ixd,
	cmp_in_rg,	cmp_in_rgd,	cmp_in_in,	cmp_in_ind,	cmp_in_de,	cmp_in_ded,	cmp_in_ix,	cmp_in_ixd,
	/* 0x2500 */
	cmp_in_rg,	cmp_in_rgd,	cmp_in_in,	cmp_in_ind,	cmp_in_de,	cmp_in_ded,	cmp_in_ix,	cmp_in_ixd,
	cmp_in_rg,	cmp_in_rgd,	cmp_in_in,	cmp_in_ind,	cmp_in_de,	cmp_in_ded,	cmp_in_ix,	cmp_in_ixd,
	cmp_in_rg,	cmp_in_rgd,	cmp_in_in,	cmp_in_ind,	cmp_in_de,	cmp_in_ded,	cmp_in_ix,	cmp_in_ixd,
	cmp_in_rg,	cmp_in_rgd,	cmp_in_in,	cmp_in_ind,	cmp_in_de,	cmp_in_ded,	cmp_in_ix,	cmp_in_ixd,
	/* 0x2600 */
	cmp_ind_rg,	cmp_ind_rgd,cmp_ind_in,	cmp_ind_ind,cmp_ind_de,	cmp_ind_ded,cmp_ind_ix,	cmp_ind_ixd,
	cmp_ind_rg,	cmp_ind_rgd,cmp_ind_in,	cmp_ind_ind,cmp_ind_de,	cmp_ind_ded,cmp_ind_ix,	cmp_ind_ixd,
	cmp_ind_rg,	cmp_ind_rgd,cmp_ind_in,	cmp_ind_ind,cmp_ind_de,	cmp_ind_ded,cmp_ind_ix,	cmp_ind_ixd,
	cmp_ind_rg,	cmp_ind_rgd,cmp_ind_in,	cmp_ind_ind,cmp_ind_de,	cmp_ind_ded,cmp_ind_ix,	cmp_ind_ixd,
	/* 0x2700 */
	cmp_ind_rg,	cmp_ind_rgd,cmp_ind_in,	cmp_ind_ind,cmp_ind_de,	cmp_ind_ded,cmp_ind_ix,	cmp_ind_ixd,
	cmp_ind_rg,	cmp_ind_rgd,cmp_ind_in,	cmp_ind_ind,cmp_ind_de,	cmp_ind_ded,cmp_ind_ix,	cmp_ind_ixd,
	cmp_ind_rg,	cmp_ind_rgd,cmp_ind_in,	cmp_ind_ind,cmp_ind_de,	cmp_ind_ded,cmp_ind_ix,	cmp_ind_ixd,
	cmp_ind_rg,	cmp_ind_rgd,cmp_ind_in,	cmp_ind_ind,cmp_ind_de,	cmp_ind_ded,cmp_ind_ix,	cmp_ind_ixd,
	/* 0x2800 */
	cmp_de_rg,	cmp_de_rgd,	cmp_de_in,	cmp_de_ind,	cmp_de_de,	cmp_de_ded,	cmp_de_ix,	cmp_de_ixd,
	cmp_de_rg,	cmp_de_rgd,	cmp_de_in,	cmp_de_ind,	cmp_de_de,	cmp_de_ded,	cmp_de_ix,	cmp_de_ixd,
	cmp_de_rg,	cmp_de_rgd,	cmp_de_in,	cmp_de_ind,	cmp_de_de,	cmp_de_ded,	cmp_de_ix,	cmp_de_ixd,
	cmp_de_rg,	cmp_de_rgd,	cmp_de_in,	cmp_de_ind,	cmp_de_de,	cmp_de_ded,	cmp_de_ix,	cmp_de_ixd,
	/* 0x2900 */
	cmp_de_rg,	cmp_de_rgd,	cmp_de_in,	cmp_de_ind,	cmp_de_de,	cmp_de_ded,	cmp_de_ix,	cmp_de_ixd,
	cmp_de_rg,	cmp_de_rgd,	cmp_de_in,	cmp_de_ind,	cmp_de_de,	cmp_de_ded,	cmp_de_ix,	cmp_de_ixd,
	cmp_de_rg,	cmp_de_rgd,	cmp_de_in,	cmp_de_ind,	cmp_de_de,	cmp_de_ded,	cmp_de_ix,	cmp_de_ixd,
	cmp_de_rg,	cmp_de_rgd,	cmp_de_in,	cmp_de_ind,	cmp_de_de,	cmp_de_ded,	cmp_de_ix,	cmp_de_ixd,
	/* 0x2a00 */
	cmp_ded_rg,	cmp_ded_rgd,cmp_ded_in,	cmp_ded_ind,cmp_ded_de,	cmp_ded_ded,cmp_ded_ix,	cmp_ded_ixd,
	cmp_ded_rg,	cmp_ded_rgd,cmp_ded_in,	cmp_ded_ind,cmp_ded_de,	cmp_ded_ded,cmp_ded_ix,	cmp_ded_ixd,
	cmp_ded_rg,	cmp_ded_rgd,cmp_ded_in,	cmp_ded_ind,cmp_ded_de,	cmp_ded_ded,cmp_ded_ix,	cmp_ded_ixd,
	cmp_ded_rg,	cmp_ded_rgd,cmp_ded_in,	cmp_ded_ind,cmp_ded_de,	cmp_ded_ded,cmp_ded_ix,	cmp_ded_ixd,
	/* 0x2b00 */
	cmp_ded_rg,	cmp_ded_rgd,cmp_ded_in,	cmp_ded_ind,cmp_ded_de,	cmp_ded_ded,cmp_ded_ix,	cmp_ded_ixd,
	cmp_ded_rg,	cmp_ded_rgd,cmp_ded_in,	cmp_ded_ind,cmp_ded_de,	cmp_ded_ded,cmp_ded_ix,	cmp_ded_ixd,
	cmp_ded_rg,	cmp_ded_rgd,cmp_ded_in,	cmp_ded_ind,cmp_ded_de,	cmp_ded_ded,cmp_ded_ix,	cmp_ded_ixd,
	cmp_ded_rg,	cmp_ded_rgd,cmp_ded_in,	cmp_ded_ind,cmp_ded_de,	cmp_ded_ded,cmp_ded_ix,	cmp_ded_ixd,
	/* 0x2c00 */
	cmp_ix_rg,	cmp_ix_rgd,	cmp_ix_in,	cmp_ix_ind,	cmp_ix_de,	cmp_ix_ded,	cmp_ix_ix,	cmp_ix_ixd,
	cmp_ix_rg,	cmp_ix_rgd,	cmp_ix_in,	cmp_ix_ind,	cmp_ix_de,	cmp_ix_ded,	cmp_ix_ix,	cmp_ix_ixd,
	cmp_ix_rg,	cmp_ix_rgd,	cmp_ix_in,	cmp_ix_ind,	cmp_ix_de,	cmp_ix_ded,	cmp_ix_ix,	cmp_ix_ixd,
	cmp_ix_rg,	cmp_ix_rgd,	cmp_ix_in,	cmp_ix_ind,	cmp_ix_de,	cmp_ix_ded,	cmp_ix_ix,	cmp_ix_ixd,
	/* 0x2d00 */
	cmp_ix_rg,	cmp_ix_rgd,	cmp_ix_in,	cmp_ix_ind,	cmp_ix_de,	cmp_ix_ded,	cmp_ix_ix,	cmp_ix_ixd,
	cmp_ix_rg,	cmp_ix_rgd,	cmp_ix_in,	cmp_ix_ind,	cmp_ix_de,	cmp_ix_ded,	cmp_ix_ix,	cmp_ix_ixd,
	cmp_ix_rg,	cmp_ix_rgd,	cmp_ix_in,	cmp_ix_ind,	cmp_ix_de,	cmp_ix_ded,	cmp_ix_ix,	cmp_ix_ixd,
	cmp_ix_rg,	cmp_ix_rgd,	cmp_ix_in,	cmp_ix_ind,	cmp_ix_de,	cmp_ix_ded,	cmp_ix_ix,	cmp_ix_ixd,
	/* 0x2e00 */
	cmp_ixd_rg,	cmp_ixd_rgd,cmp_ixd_in,	cmp_ixd_ind,cmp_ixd_de,	cmp_ixd_ded,cmp_ixd_ix,	cmp_ixd_ixd,
	cmp_ixd_rg,	cmp_ixd_rgd,cmp_ixd_in,	cmp_ixd_ind,cmp_ixd_de,	cmp_ixd_ded,cmp_ixd_ix,	cmp_ixd_ixd,
	cmp_ixd_rg,	cmp_ixd_rgd,cmp_ixd_in,	cmp_ixd_ind,cmp_ixd_de,	cmp_ixd_ded,cmp_ixd_ix,	cmp_ixd_ixd,
	cmp_ixd_rg,	cmp_ixd_rgd,cmp_ixd_in,	cmp_ixd_ind,cmp_ixd_de,	cmp_ixd_ded,cmp_ixd_ix,	cmp_ixd_ixd,
	/* 0x2f00 */
	cmp_ixd_rg,	cmp_ixd_rgd,cmp_ixd_in,	cmp_ixd_ind,cmp_ixd_de,	cmp_ixd_ded,cmp_ixd_ix,	cmp_ixd_ixd,
	cmp_ixd_rg,	cmp_ixd_rgd,cmp_ixd_in,	cmp_ixd_ind,cmp_ixd_de,	cmp_ixd_ded,cmp_ixd_ix,	cmp_ixd_ixd,
	cmp_ixd_rg,	cmp_ixd_rgd,cmp_ixd_in,	cmp_ixd_ind,cmp_ixd_de,	cmp_ixd_ded,cmp_ixd_ix,	cmp_ixd_ixd,
	cmp_ixd_rg,	cmp_ixd_rgd,cmp_ixd_in,	cmp_ixd_ind,cmp_ixd_de,	cmp_ixd_ded,cmp_ixd_ix,	cmp_ixd_ixd,

	/* 0x3000 */
	bit_rg_rg,	bit_rg_rgd,	bit_rg_in,	bit_rg_ind,	bit_rg_de,	bit_rg_ded,	bit_rg_ix,	bit_rg_ixd,
	bit_rg_rg,	bit_rg_rgd,	bit_rg_in,	bit_rg_ind,	bit_rg_de,	bit_rg_ded,	bit_rg_ix,	bit_rg_ixd,
	bit_rg_rg,	bit_rg_rgd,	bit_rg_in,	bit_rg_ind,	bit_rg_de,	bit_rg_ded,	bit_rg_ix,	bit_rg_ixd,
	bit_rg_rg,	bit_rg_rgd,	bit_rg_in,	bit_rg_ind,	bit_rg_de,	bit_rg_ded,	bit_rg_ix,	bit_rg_ixd,
	/* 0x3100 */
	bit_rg_rg,	bit_rg_rgd,	bit_rg_in,	bit_rg_ind,	bit_rg_de,	bit_rg_ded,	bit_rg_ix,	bit_rg_ixd,
	bit_rg_rg,	bit_rg_rgd,	bit_rg_in,	bit_rg_ind,	bit_rg_de,	bit_rg_ded,	bit_rg_ix,	bit_rg_ixd,
	bit_rg_rg,	bit_rg_rgd,	bit_rg_in,	bit_rg_ind,	bit_rg_de,	bit_rg_ded,	bit_rg_ix,	bit_rg_ixd,
	bit_rg_rg,	bit_rg_rgd,	bit_rg_in,	bit_rg_ind,	bit_rg_de,	bit_rg_ded,	bit_rg_ix,	bit_rg_ixd,
	/* 0x3200 */
	bit_rgd_rg,	bit_rgd_rgd,bit_rgd_in,	bit_rgd_ind,bit_rgd_de,	bit_rgd_ded,bit_rgd_ix,	bit_rgd_ixd,
	bit_rgd_rg,	bit_rgd_rgd,bit_rgd_in,	bit_rgd_ind,bit_rgd_de,	bit_rgd_ded,bit_rgd_ix,	bit_rgd_ixd,
	bit_rgd_rg,	bit_rgd_rgd,bit_rgd_in,	bit_rgd_ind,bit_rgd_de,	bit_rgd_ded,bit_rgd_ix,	bit_rgd_ixd,
	bit_rgd_rg,	bit_rgd_rgd,bit_rgd_in,	bit_rgd_ind,bit_rgd_de,	bit_rgd_ded,bit_rgd_ix,	bit_rgd_ixd,
	/* 0x3300 */
	bit_rgd_rg,	bit_rgd_rgd,bit_rgd_in,	bit_rgd_ind,bit_rgd_de,	bit_rgd_ded,bit_rgd_ix,	bit_rgd_ixd,
	bit_rgd_rg,	bit_rgd_rgd,bit_rgd_in,	bit_rgd_ind,bit_rgd_de,	bit_rgd_ded,bit_rgd_ix,	bit_rgd_ixd,
	bit_rgd_rg,	bit_rgd_rgd,bit_rgd_in,	bit_rgd_ind,bit_rgd_de,	bit_rgd_ded,bit_rgd_ix,	bit_rgd_ixd,
	bit_rgd_rg,	bit_rgd_rgd,bit_rgd_in,	bit_rgd_ind,bit_rgd_de,	bit_rgd_ded,bit_rgd_ix,	bit_rgd_ixd,
	/* 0x3400 */
	bit_in_rg,	bit_in_rgd,	bit_in_in,	bit_in_ind,	bit_in_de,	bit_in_ded,	bit_in_ix,	bit_in_ixd,
	bit_in_rg,	bit_in_rgd,	bit_in_in,	bit_in_ind,	bit_in_de,	bit_in_ded,	bit_in_ix,	bit_in_ixd,
	bit_in_rg,	bit_in_rgd,	bit_in_in,	bit_in_ind,	bit_in_de,	bit_in_ded,	bit_in_ix,	bit_in_ixd,
	bit_in_rg,	bit_in_rgd,	bit_in_in,	bit_in_ind,	bit_in_de,	bit_in_ded,	bit_in_ix,	bit_in_ixd,
	/* 0x3500 */
	bit_in_rg,	bit_in_rgd,	bit_in_in,	bit_in_ind,	bit_in_de,	bit_in_ded,	bit_in_ix,	bit_in_ixd,
	bit_in_rg,	bit_in_rgd,	bit_in_in,	bit_in_ind,	bit_in_de,	bit_in_ded,	bit_in_ix,	bit_in_ixd,
	bit_in_rg,	bit_in_rgd,	bit_in_in,	bit_in_ind,	bit_in_de,	bit_in_ded,	bit_in_ix,	bit_in_ixd,
	bit_in_rg,	bit_in_rgd,	bit_in_in,	bit_in_ind,	bit_in_de,	bit_in_ded,	bit_in_ix,	bit_in_ixd,
	/* 0x3600 */
	bit_ind_rg,	bit_ind_rgd,bit_ind_in,	bit_ind_ind,bit_ind_de,	bit_ind_ded,bit_ind_ix,	bit_ind_ixd,
	bit_ind_rg,	bit_ind_rgd,bit_ind_in,	bit_ind_ind,bit_ind_de,	bit_ind_ded,bit_ind_ix,	bit_ind_ixd,
	bit_ind_rg,	bit_ind_rgd,bit_ind_in,	bit_ind_ind,bit_ind_de,	bit_ind_ded,bit_ind_ix,	bit_ind_ixd,
	bit_ind_rg,	bit_ind_rgd,bit_ind_in,	bit_ind_ind,bit_ind_de,	bit_ind_ded,bit_ind_ix,	bit_ind_ixd,
	/* 0x3700 */
	bit_ind_rg,	bit_ind_rgd,bit_ind_in,	bit_ind_ind,bit_ind_de,	bit_ind_ded,bit_ind_ix,	bit_ind_ixd,
	bit_ind_rg,	bit_ind_rgd,bit_ind_in,	bit_ind_ind,bit_ind_de,	bit_ind_ded,bit_ind_ix,	bit_ind_ixd,
	bit_ind_rg,	bit_ind_rgd,bit_ind_in,	bit_ind_ind,bit_ind_de,	bit_ind_ded,bit_ind_ix,	bit_ind_ixd,
	bit_ind_rg,	bit_ind_rgd,bit_ind_in,	bit_ind_ind,bit_ind_de,	bit_ind_ded,bit_ind_ix,	bit_ind_ixd,
	/* 0x3800 */
	bit_de_rg,	bit_de_rgd,	bit_de_in,	bit_de_ind,	bit_de_de,	bit_de_ded,	bit_de_ix,	bit_de_ixd,
	bit_de_rg,	bit_de_rgd,	bit_de_in,	bit_de_ind,	bit_de_de,	bit_de_ded,	bit_de_ix,	bit_de_ixd,
	bit_de_rg,	bit_de_rgd,	bit_de_in,	bit_de_ind,	bit_de_de,	bit_de_ded,	bit_de_ix,	bit_de_ixd,
	bit_de_rg,	bit_de_rgd,	bit_de_in,	bit_de_ind,	bit_de_de,	bit_de_ded,	bit_de_ix,	bit_de_ixd,
	/* 0x3900 */
	bit_de_rg,	bit_de_rgd,	bit_de_in,	bit_de_ind,	bit_de_de,	bit_de_ded,	bit_de_ix,	bit_de_ixd,
	bit_de_rg,	bit_de_rgd,	bit_de_in,	bit_de_ind,	bit_de_de,	bit_de_ded,	bit_de_ix,	bit_de_ixd,
	bit_de_rg,	bit_de_rgd,	bit_de_in,	bit_de_ind,	bit_de_de,	bit_de_ded,	bit_de_ix,	bit_de_ixd,
	bit_de_rg,	bit_de_rgd,	bit_de_in,	bit_de_ind,	bit_de_de,	bit_de_ded,	bit_de_ix,	bit_de_ixd,
	/* 0x3a00 */
	bit_ded_rg,	bit_ded_rgd,bit_ded_in,	bit_ded_ind,bit_ded_de,	bit_ded_ded,bit_ded_ix,	bit_ded_ixd,
	bit_ded_rg,	bit_ded_rgd,bit_ded_in,	bit_ded_ind,bit_ded_de,	bit_ded_ded,bit_ded_ix,	bit_ded_ixd,
	bit_ded_rg,	bit_ded_rgd,bit_ded_in,	bit_ded_ind,bit_ded_de,	bit_ded_ded,bit_ded_ix,	bit_ded_ixd,
	bit_ded_rg,	bit_ded_rgd,bit_ded_in,	bit_ded_ind,bit_ded_de,	bit_ded_ded,bit_ded_ix,	bit_ded_ixd,
	/* 0x3b00 */
	bit_ded_rg,	bit_ded_rgd,bit_ded_in,	bit_ded_ind,bit_ded_de,	bit_ded_ded,bit_ded_ix,	bit_ded_ixd,
	bit_ded_rg,	bit_ded_rgd,bit_ded_in,	bit_ded_ind,bit_ded_de,	bit_ded_ded,bit_ded_ix,	bit_ded_ixd,
	bit_ded_rg,	bit_ded_rgd,bit_ded_in,	bit_ded_ind,bit_ded_de,	bit_ded_ded,bit_ded_ix,	bit_ded_ixd,
	bit_ded_rg,	bit_ded_rgd,bit_ded_in,	bit_ded_ind,bit_ded_de,	bit_ded_ded,bit_ded_ix,	bit_ded_ixd,
	/* 0x3c00 */
	bit_ix_rg,	bit_ix_rgd,	bit_ix_in,	bit_ix_ind,	bit_ix_de,	bit_ix_ded,	bit_ix_ix,	bit_ix_ixd,
	bit_ix_rg,	bit_ix_rgd,	bit_ix_in,	bit_ix_ind,	bit_ix_de,	bit_ix_ded,	bit_ix_ix,	bit_ix_ixd,
	bit_ix_rg,	bit_ix_rgd,	bit_ix_in,	bit_ix_ind,	bit_ix_de,	bit_ix_ded,	bit_ix_ix,	bit_ix_ixd,
	bit_ix_rg,	bit_ix_rgd,	bit_ix_in,	bit_ix_ind,	bit_ix_de,	bit_ix_ded,	bit_ix_ix,	bit_ix_ixd,
	/* 0x3d00 */
	bit_ix_rg,	bit_ix_rgd,	bit_ix_in,	bit_ix_ind,	bit_ix_de,	bit_ix_ded,	bit_ix_ix,	bit_ix_ixd,
	bit_ix_rg,	bit_ix_rgd,	bit_ix_in,	bit_ix_ind,	bit_ix_de,	bit_ix_ded,	bit_ix_ix,	bit_ix_ixd,
	bit_ix_rg,	bit_ix_rgd,	bit_ix_in,	bit_ix_ind,	bit_ix_de,	bit_ix_ded,	bit_ix_ix,	bit_ix_ixd,
	bit_ix_rg,	bit_ix_rgd,	bit_ix_in,	bit_ix_ind,	bit_ix_de,	bit_ix_ded,	bit_ix_ix,	bit_ix_ixd,
	/* 0x3e00 */
	bit_ixd_rg,	bit_ixd_rgd,bit_ixd_in,	bit_ixd_ind,bit_ixd_de,	bit_ixd_ded,bit_ixd_ix,	bit_ixd_ixd,
	bit_ixd_rg,	bit_ixd_rgd,bit_ixd_in,	bit_ixd_ind,bit_ixd_de,	bit_ixd_ded,bit_ixd_ix,	bit_ixd_ixd,
	bit_ixd_rg,	bit_ixd_rgd,bit_ixd_in,	bit_ixd_ind,bit_ixd_de,	bit_ixd_ded,bit_ixd_ix,	bit_ixd_ixd,
	bit_ixd_rg,	bit_ixd_rgd,bit_ixd_in,	bit_ixd_ind,bit_ixd_de,	bit_ixd_ded,bit_ixd_ix,	bit_ixd_ixd,
	/* 0x3f00 */
	bit_ixd_rg,	bit_ixd_rgd,bit_ixd_in,	bit_ixd_ind,bit_ixd_de,	bit_ixd_ded,bit_ixd_ix,	bit_ixd_ixd,
	bit_ixd_rg,	bit_ixd_rgd,bit_ixd_in,	bit_ixd_ind,bit_ixd_de,	bit_ixd_ded,bit_ixd_ix,	bit_ixd_ixd,
	bit_ixd_rg,	bit_ixd_rgd,bit_ixd_in,	bit_ixd_ind,bit_ixd_de,	bit_ixd_ded,bit_ixd_ix,	bit_ixd_ixd,
	bit_ixd_rg,	bit_ixd_rgd,bit_ixd_in,	bit_ixd_ind,bit_ixd_de,	bit_ixd_ded,bit_ixd_ix,	bit_ixd_ixd,

	/* 0x4000 */
	bic_rg_rg,	bic_rg_rgd,	bic_rg_in,	bic_rg_ind,	bic_rg_de,	bic_rg_ded,	bic_rg_ix,	bic_rg_ixd,
	bic_rg_rg,	bic_rg_rgd,	bic_rg_in,	bic_rg_ind,	bic_rg_de,	bic_rg_ded,	bic_rg_ix,	bic_rg_ixd,
	bic_rg_rg,	bic_rg_rgd,	bic_rg_in,	bic_rg_ind,	bic_rg_de,	bic_rg_ded,	bic_rg_ix,	bic_rg_ixd,
	bic_rg_rg,	bic_rg_rgd,	bic_rg_in,	bic_rg_ind,	bic_rg_de,	bic_rg_ded,	bic_rg_ix,	bic_rg_ixd,
	/* 0x4100 */
	bic_rg_rg,	bic_rg_rgd,	bic_rg_in,	bic_rg_ind,	bic_rg_de,	bic_rg_ded,	bic_rg_ix,	bic_rg_ixd,
	bic_rg_rg,	bic_rg_rgd,	bic_rg_in,	bic_rg_ind,	bic_rg_de,	bic_rg_ded,	bic_rg_ix,	bic_rg_ixd,
	bic_rg_rg,	bic_rg_rgd,	bic_rg_in,	bic_rg_ind,	bic_rg_de,	bic_rg_ded,	bic_rg_ix,	bic_rg_ixd,
	bic_rg_rg,	bic_rg_rgd,	bic_rg_in,	bic_rg_ind,	bic_rg_de,	bic_rg_ded,	bic_rg_ix,	bic_rg_ixd,
	/* 0x4200 */
	bic_rgd_rg,	bic_rgd_rgd,bic_rgd_in,	bic_rgd_ind,bic_rgd_de,	bic_rgd_ded,bic_rgd_ix,	bic_rgd_ixd,
	bic_rgd_rg,	bic_rgd_rgd,bic_rgd_in,	bic_rgd_ind,bic_rgd_de,	bic_rgd_ded,bic_rgd_ix,	bic_rgd_ixd,
	bic_rgd_rg,	bic_rgd_rgd,bic_rgd_in,	bic_rgd_ind,bic_rgd_de,	bic_rgd_ded,bic_rgd_ix,	bic_rgd_ixd,
	bic_rgd_rg,	bic_rgd_rgd,bic_rgd_in,	bic_rgd_ind,bic_rgd_de,	bic_rgd_ded,bic_rgd_ix,	bic_rgd_ixd,
	/* 0x4300 */
	bic_rgd_rg,	bic_rgd_rgd,bic_rgd_in,	bic_rgd_ind,bic_rgd_de,	bic_rgd_ded,bic_rgd_ix,	bic_rgd_ixd,
	bic_rgd_rg,	bic_rgd_rgd,bic_rgd_in,	bic_rgd_ind,bic_rgd_de,	bic_rgd_ded,bic_rgd_ix,	bic_rgd_ixd,
	bic_rgd_rg,	bic_rgd_rgd,bic_rgd_in,	bic_rgd_ind,bic_rgd_de,	bic_rgd_ded,bic_rgd_ix,	bic_rgd_ixd,
	bic_rgd_rg,	bic_rgd_rgd,bic_rgd_in,	bic_rgd_ind,bic_rgd_de,	bic_rgd_ded,bic_rgd_ix,	bic_rgd_ixd,
	/* 0x4400 */
	bic_in_rg,	bic_in_rgd,	bic_in_in,	bic_in_ind,	bic_in_de,	bic_in_ded,	bic_in_ix,	bic_in_ixd,
	bic_in_rg,	bic_in_rgd,	bic_in_in,	bic_in_ind,	bic_in_de,	bic_in_ded,	bic_in_ix,	bic_in_ixd,
	bic_in_rg,	bic_in_rgd,	bic_in_in,	bic_in_ind,	bic_in_de,	bic_in_ded,	bic_in_ix,	bic_in_ixd,
	bic_in_rg,	bic_in_rgd,	bic_in_in,	bic_in_ind,	bic_in_de,	bic_in_ded,	bic_in_ix,	bic_in_ixd,
	/* 0x4500 */
	bic_in_rg,	bic_in_rgd,	bic_in_in,	bic_in_ind,	bic_in_de,	bic_in_ded,	bic_in_ix,	bic_in_ixd,
	bic_in_rg,	bic_in_rgd,	bic_in_in,	bic_in_ind,	bic_in_de,	bic_in_ded,	bic_in_ix,	bic_in_ixd,
	bic_in_rg,	bic_in_rgd,	bic_in_in,	bic_in_ind,	bic_in_de,	bic_in_ded,	bic_in_ix,	bic_in_ixd,
	bic_in_rg,	bic_in_rgd,	bic_in_in,	bic_in_ind,	bic_in_de,	bic_in_ded,	bic_in_ix,	bic_in_ixd,
	/* 0x4600 */
	bic_ind_rg,	bic_ind_rgd,bic_ind_in,	bic_ind_ind,bic_ind_de,	bic_ind_ded,bic_ind_ix,	bic_ind_ixd,
	bic_ind_rg,	bic_ind_rgd,bic_ind_in,	bic_ind_ind,bic_ind_de,	bic_ind_ded,bic_ind_ix,	bic_ind_ixd,
	bic_ind_rg,	bic_ind_rgd,bic_ind_in,	bic_ind_ind,bic_ind_de,	bic_ind_ded,bic_ind_ix,	bic_ind_ixd,
	bic_ind_rg,	bic_ind_rgd,bic_ind_in,	bic_ind_ind,bic_ind_de,	bic_ind_ded,bic_ind_ix,	bic_ind_ixd,
	/* 0x4700 */
	bic_ind_rg,	bic_ind_rgd,bic_ind_in,	bic_ind_ind,bic_ind_de,	bic_ind_ded,bic_ind_ix,	bic_ind_ixd,
	bic_ind_rg,	bic_ind_rgd,bic_ind_in,	bic_ind_ind,bic_ind_de,	bic_ind_ded,bic_ind_ix,	bic_ind_ixd,
	bic_ind_rg,	bic_ind_rgd,bic_ind_in,	bic_ind_ind,bic_ind_de,	bic_ind_ded,bic_ind_ix,	bic_ind_ixd,
	bic_ind_rg,	bic_ind_rgd,bic_ind_in,	bic_ind_ind,bic_ind_de,	bic_ind_ded,bic_ind_ix,	bic_ind_ixd,
	/* 0x4800 */
	bic_de_rg,	bic_de_rgd,	bic_de_in,	bic_de_ind,	bic_de_de,	bic_de_ded,	bic_de_ix,	bic_de_ixd,
	bic_de_rg,	bic_de_rgd,	bic_de_in,	bic_de_ind,	bic_de_de,	bic_de_ded,	bic_de_ix,	bic_de_ixd,
	bic_de_rg,	bic_de_rgd,	bic_de_in,	bic_de_ind,	bic_de_de,	bic_de_ded,	bic_de_ix,	bic_de_ixd,
	bic_de_rg,	bic_de_rgd,	bic_de_in,	bic_de_ind,	bic_de_de,	bic_de_ded,	bic_de_ix,	bic_de_ixd,
	/* 0x4900 */
	bic_de_rg,	bic_de_rgd,	bic_de_in,	bic_de_ind,	bic_de_de,	bic_de_ded,	bic_de_ix,	bic_de_ixd,
	bic_de_rg,	bic_de_rgd,	bic_de_in,	bic_de_ind,	bic_de_de,	bic_de_ded,	bic_de_ix,	bic_de_ixd,
	bic_de_rg,	bic_de_rgd,	bic_de_in,	bic_de_ind,	bic_de_de,	bic_de_ded,	bic_de_ix,	bic_de_ixd,
	bic_de_rg,	bic_de_rgd,	bic_de_in,	bic_de_ind,	bic_de_de,	bic_de_ded,	bic_de_ix,	bic_de_ixd,
	/* 0x4a00 */
	bic_ded_rg,	bic_ded_rgd,bic_ded_in,	bic_ded_ind,bic_ded_de,	bic_ded_ded,bic_ded_ix,	bic_ded_ixd,
	bic_ded_rg,	bic_ded_rgd,bic_ded_in,	bic_ded_ind,bic_ded_de,	bic_ded_ded,bic_ded_ix,	bic_ded_ixd,
	bic_ded_rg,	bic_ded_rgd,bic_ded_in,	bic_ded_ind,bic_ded_de,	bic_ded_ded,bic_ded_ix,	bic_ded_ixd,
	bic_ded_rg,	bic_ded_rgd,bic_ded_in,	bic_ded_ind,bic_ded_de,	bic_ded_ded,bic_ded_ix,	bic_ded_ixd,
	/* 0x4b00 */
	bic_ded_rg,	bic_ded_rgd,bic_ded_in,	bic_ded_ind,bic_ded_de,	bic_ded_ded,bic_ded_ix,	bic_ded_ixd,
	bic_ded_rg,	bic_ded_rgd,bic_ded_in,	bic_ded_ind,bic_ded_de,	bic_ded_ded,bic_ded_ix,	bic_ded_ixd,
	bic_ded_rg,	bic_ded_rgd,bic_ded_in,	bic_ded_ind,bic_ded_de,	bic_ded_ded,bic_ded_ix,	bic_ded_ixd,
	bic_ded_rg,	bic_ded_rgd,bic_ded_in,	bic_ded_ind,bic_ded_de,	bic_ded_ded,bic_ded_ix,	bic_ded_ixd,
	/* 0x4c00 */
	bic_ix_rg,	bic_ix_rgd,	bic_ix_in,	bic_ix_ind,	bic_ix_de,	bic_ix_ded,	bic_ix_ix,	bic_ix_ixd,
	bic_ix_rg,	bic_ix_rgd,	bic_ix_in,	bic_ix_ind,	bic_ix_de,	bic_ix_ded,	bic_ix_ix,	bic_ix_ixd,
	bic_ix_rg,	bic_ix_rgd,	bic_ix_in,	bic_ix_ind,	bic_ix_de,	bic_ix_ded,	bic_ix_ix,	bic_ix_ixd,
	bic_ix_rg,	bic_ix_rgd,	bic_ix_in,	bic_ix_ind,	bic_ix_de,	bic_ix_ded,	bic_ix_ix,	bic_ix_ixd,
	/* 0x4d00 */
	bic_ix_rg,	bic_ix_rgd,	bic_ix_in,	bic_ix_ind,	bic_ix_de,	bic_ix_ded,	bic_ix_ix,	bic_ix_ixd,
	bic_ix_rg,	bic_ix_rgd,	bic_ix_in,	bic_ix_ind,	bic_ix_de,	bic_ix_ded,	bic_ix_ix,	bic_ix_ixd,
	bic_ix_rg,	bic_ix_rgd,	bic_ix_in,	bic_ix_ind,	bic_ix_de,	bic_ix_ded,	bic_ix_ix,	bic_ix_ixd,
	bic_ix_rg,	bic_ix_rgd,	bic_ix_in,	bic_ix_ind,	bic_ix_de,	bic_ix_ded,	bic_ix_ix,	bic_ix_ixd,
	/* 0x4e00 */
	bic_ixd_rg,	bic_ixd_rgd,bic_ixd_in,	bic_ixd_ind,bic_ixd_de,	bic_ixd_ded,bic_ixd_ix,	bic_ixd_ixd,
	bic_ixd_rg,	bic_ixd_rgd,bic_ixd_in,	bic_ixd_ind,bic_ixd_de,	bic_ixd_ded,bic_ixd_ix,	bic_ixd_ixd,
	bic_ixd_rg,	bic_ixd_rgd,bic_ixd_in,	bic_ixd_ind,bic_ixd_de,	bic_ixd_ded,bic_ixd_ix,	bic_ixd_ixd,
	bic_ixd_rg,	bic_ixd_rgd,bic_ixd_in,	bic_ixd_ind,bic_ixd_de,	bic_ixd_ded,bic_ixd_ix,	bic_ixd_ixd,
	/* 0x4f00 */
	bic_ixd_rg,	bic_ixd_rgd,bic_ixd_in,	bic_ixd_ind,bic_ixd_de,	bic_ixd_ded,bic_ixd_ix,	bic_ixd_ixd,
	bic_ixd_rg,	bic_ixd_rgd,bic_ixd_in,	bic_ixd_ind,bic_ixd_de,	bic_ixd_ded,bic_ixd_ix,	bic_ixd_ixd,
	bic_ixd_rg,	bic_ixd_rgd,bic_ixd_in,	bic_ixd_ind,bic_ixd_de,	bic_ixd_ded,bic_ixd_ix,	bic_ixd_ixd,
	bic_ixd_rg,	bic_ixd_rgd,bic_ixd_in,	bic_ixd_ind,bic_ixd_de,	bic_ixd_ded,bic_ixd_ix,	bic_ixd_ixd,

	/* 0x5000 */
	bis_rg_rg,	bis_rg_rgd,	bis_rg_in,	bis_rg_ind,	bis_rg_de,	bis_rg_ded,	bis_rg_ix,	bis_rg_ixd,
	bis_rg_rg,	bis_rg_rgd,	bis_rg_in,	bis_rg_ind,	bis_rg_de,	bis_rg_ded,	bis_rg_ix,	bis_rg_ixd,
	bis_rg_rg,	bis_rg_rgd,	bis_rg_in,	bis_rg_ind,	bis_rg_de,	bis_rg_ded,	bis_rg_ix,	bis_rg_ixd,
	bis_rg_rg,	bis_rg_rgd,	bis_rg_in,	bis_rg_ind,	bis_rg_de,	bis_rg_ded,	bis_rg_ix,	bis_rg_ixd,
	/* 0x5100 */
	bis_rg_rg,	bis_rg_rgd,	bis_rg_in,	bis_rg_ind,	bis_rg_de,	bis_rg_ded,	bis_rg_ix,	bis_rg_ixd,
	bis_rg_rg,	bis_rg_rgd,	bis_rg_in,	bis_rg_ind,	bis_rg_de,	bis_rg_ded,	bis_rg_ix,	bis_rg_ixd,
	bis_rg_rg,	bis_rg_rgd,	bis_rg_in,	bis_rg_ind,	bis_rg_de,	bis_rg_ded,	bis_rg_ix,	bis_rg_ixd,
	bis_rg_rg,	bis_rg_rgd,	bis_rg_in,	bis_rg_ind,	bis_rg_de,	bis_rg_ded,	bis_rg_ix,	bis_rg_ixd,
	/* 0x5200 */
	bis_rgd_rg,	bis_rgd_rgd,bis_rgd_in,	bis_rgd_ind,bis_rgd_de,	bis_rgd_ded,bis_rgd_ix,	bis_rgd_ixd,
	bis_rgd_rg,	bis_rgd_rgd,bis_rgd_in,	bis_rgd_ind,bis_rgd_de,	bis_rgd_ded,bis_rgd_ix,	bis_rgd_ixd,
	bis_rgd_rg,	bis_rgd_rgd,bis_rgd_in,	bis_rgd_ind,bis_rgd_de,	bis_rgd_ded,bis_rgd_ix,	bis_rgd_ixd,
	bis_rgd_rg,	bis_rgd_rgd,bis_rgd_in,	bis_rgd_ind,bis_rgd_de,	bis_rgd_ded,bis_rgd_ix,	bis_rgd_ixd,
	/* 0x5300 */
	bis_rgd_rg,	bis_rgd_rgd,bis_rgd_in,	bis_rgd_ind,bis_rgd_de,	bis_rgd_ded,bis_rgd_ix,	bis_rgd_ixd,
	bis_rgd_rg,	bis_rgd_rgd,bis_rgd_in,	bis_rgd_ind,bis_rgd_de,	bis_rgd_ded,bis_rgd_ix,	bis_rgd_ixd,
	bis_rgd_rg,	bis_rgd_rgd,bis_rgd_in,	bis_rgd_ind,bis_rgd_de,	bis_rgd_ded,bis_rgd_ix,	bis_rgd_ixd,
	bis_rgd_rg,	bis_rgd_rgd,bis_rgd_in,	bis_rgd_ind,bis_rgd_de,	bis_rgd_ded,bis_rgd_ix,	bis_rgd_ixd,
	/* 0x5400 */
	bis_in_rg,	bis_in_rgd,	bis_in_in,	bis_in_ind,	bis_in_de,	bis_in_ded,	bis_in_ix,	bis_in_ixd,
	bis_in_rg,	bis_in_rgd,	bis_in_in,	bis_in_ind,	bis_in_de,	bis_in_ded,	bis_in_ix,	bis_in_ixd,
	bis_in_rg,	bis_in_rgd,	bis_in_in,	bis_in_ind,	bis_in_de,	bis_in_ded,	bis_in_ix,	bis_in_ixd,
	bis_in_rg,	bis_in_rgd,	bis_in_in,	bis_in_ind,	bis_in_de,	bis_in_ded,	bis_in_ix,	bis_in_ixd,
	/* 0x5500 */
	bis_in_rg,	bis_in_rgd,	bis_in_in,	bis_in_ind,	bis_in_de,	bis_in_ded,	bis_in_ix,	bis_in_ixd,
	bis_in_rg,	bis_in_rgd,	bis_in_in,	bis_in_ind,	bis_in_de,	bis_in_ded,	bis_in_ix,	bis_in_ixd,
	bis_in_rg,	bis_in_rgd,	bis_in_in,	bis_in_ind,	bis_in_de,	bis_in_ded,	bis_in_ix,	bis_in_ixd,
	bis_in_rg,	bis_in_rgd,	bis_in_in,	bis_in_ind,	bis_in_de,	bis_in_ded,	bis_in_ix,	bis_in_ixd,
	/* 0x5600 */
	bis_ind_rg,	bis_ind_rgd,bis_ind_in,	bis_ind_ind,bis_ind_de,	bis_ind_ded,bis_ind_ix,	bis_ind_ixd,
	bis_ind_rg,	bis_ind_rgd,bis_ind_in,	bis_ind_ind,bis_ind_de,	bis_ind_ded,bis_ind_ix,	bis_ind_ixd,
	bis_ind_rg,	bis_ind_rgd,bis_ind_in,	bis_ind_ind,bis_ind_de,	bis_ind_ded,bis_ind_ix,	bis_ind_ixd,
	bis_ind_rg,	bis_ind_rgd,bis_ind_in,	bis_ind_ind,bis_ind_de,	bis_ind_ded,bis_ind_ix,	bis_ind_ixd,
	/* 0x5700 */
	bis_ind_rg,	bis_ind_rgd,bis_ind_in,	bis_ind_ind,bis_ind_de,	bis_ind_ded,bis_ind_ix,	bis_ind_ixd,
	bis_ind_rg,	bis_ind_rgd,bis_ind_in,	bis_ind_ind,bis_ind_de,	bis_ind_ded,bis_ind_ix,	bis_ind_ixd,
	bis_ind_rg,	bis_ind_rgd,bis_ind_in,	bis_ind_ind,bis_ind_de,	bis_ind_ded,bis_ind_ix,	bis_ind_ixd,
	bis_ind_rg,	bis_ind_rgd,bis_ind_in,	bis_ind_ind,bis_ind_de,	bis_ind_ded,bis_ind_ix,	bis_ind_ixd,
	/* 0x5800 */
	bis_de_rg,	bis_de_rgd,	bis_de_in,	bis_de_ind,	bis_de_de,	bis_de_ded,	bis_de_ix,	bis_de_ixd,
	bis_de_rg,	bis_de_rgd,	bis_de_in,	bis_de_ind,	bis_de_de,	bis_de_ded,	bis_de_ix,	bis_de_ixd,
	bis_de_rg,	bis_de_rgd,	bis_de_in,	bis_de_ind,	bis_de_de,	bis_de_ded,	bis_de_ix,	bis_de_ixd,
	bis_de_rg,	bis_de_rgd,	bis_de_in,	bis_de_ind,	bis_de_de,	bis_de_ded,	bis_de_ix,	bis_de_ixd,
	/* 0x5900 */
	bis_de_rg,	bis_de_rgd,	bis_de_in,	bis_de_ind,	bis_de_de,	bis_de_ded,	bis_de_ix,	bis_de_ixd,
	bis_de_rg,	bis_de_rgd,	bis_de_in,	bis_de_ind,	bis_de_de,	bis_de_ded,	bis_de_ix,	bis_de_ixd,
	bis_de_rg,	bis_de_rgd,	bis_de_in,	bis_de_ind,	bis_de_de,	bis_de_ded,	bis_de_ix,	bis_de_ixd,
	bis_de_rg,	bis_de_rgd,	bis_de_in,	bis_de_ind,	bis_de_de,	bis_de_ded,	bis_de_ix,	bis_de_ixd,
	/* 0x5a00 */
	bis_ded_rg,	bis_ded_rgd,bis_ded_in,	bis_ded_ind,bis_ded_de,	bis_ded_ded,bis_ded_ix,	bis_ded_ixd,
	bis_ded_rg,	bis_ded_rgd,bis_ded_in,	bis_ded_ind,bis_ded_de,	bis_ded_ded,bis_ded_ix,	bis_ded_ixd,
	bis_ded_rg,	bis_ded_rgd,bis_ded_in,	bis_ded_ind,bis_ded_de,	bis_ded_ded,bis_ded_ix,	bis_ded_ixd,
	bis_ded_rg,	bis_ded_rgd,bis_ded_in,	bis_ded_ind,bis_ded_de,	bis_ded_ded,bis_ded_ix,	bis_ded_ixd,
	/* 0x5b00 */
	bis_ded_rg,	bis_ded_rgd,bis_ded_in,	bis_ded_ind,bis_ded_de,	bis_ded_ded,bis_ded_ix,	bis_ded_ixd,
	bis_ded_rg,	bis_ded_rgd,bis_ded_in,	bis_ded_ind,bis_ded_de,	bis_ded_ded,bis_ded_ix,	bis_ded_ixd,
	bis_ded_rg,	bis_ded_rgd,bis_ded_in,	bis_ded_ind,bis_ded_de,	bis_ded_ded,bis_ded_ix,	bis_ded_ixd,
	bis_ded_rg,	bis_ded_rgd,bis_ded_in,	bis_ded_ind,bis_ded_de,	bis_ded_ded,bis_ded_ix,	bis_ded_ixd,
	/* 0x5c00 */
	bis_ix_rg,	bis_ix_rgd,	bis_ix_in,	bis_ix_ind,	bis_ix_de,	bis_ix_ded,	bis_ix_ix,	bis_ix_ixd,
	bis_ix_rg,	bis_ix_rgd,	bis_ix_in,	bis_ix_ind,	bis_ix_de,	bis_ix_ded,	bis_ix_ix,	bis_ix_ixd,
	bis_ix_rg,	bis_ix_rgd,	bis_ix_in,	bis_ix_ind,	bis_ix_de,	bis_ix_ded,	bis_ix_ix,	bis_ix_ixd,
	bis_ix_rg,	bis_ix_rgd,	bis_ix_in,	bis_ix_ind,	bis_ix_de,	bis_ix_ded,	bis_ix_ix,	bis_ix_ixd,
	/* 0x5d00 */
	bis_ix_rg,	bis_ix_rgd,	bis_ix_in,	bis_ix_ind,	bis_ix_de,	bis_ix_ded,	bis_ix_ix,	bis_ix_ixd,
	bis_ix_rg,	bis_ix_rgd,	bis_ix_in,	bis_ix_ind,	bis_ix_de,	bis_ix_ded,	bis_ix_ix,	bis_ix_ixd,
	bis_ix_rg,	bis_ix_rgd,	bis_ix_in,	bis_ix_ind,	bis_ix_de,	bis_ix_ded,	bis_ix_ix,	bis_ix_ixd,
	bis_ix_rg,	bis_ix_rgd,	bis_ix_in,	bis_ix_ind,	bis_ix_de,	bis_ix_ded,	bis_ix_ix,	bis_ix_ixd,
	/* 0x5e00 */
	bis_ixd_rg,	bis_ixd_rgd,bis_ixd_in,	bis_ixd_ind,bis_ixd_de,	bis_ixd_ded,bis_ixd_ix,	bis_ixd_ixd,
	bis_ixd_rg,	bis_ixd_rgd,bis_ixd_in,	bis_ixd_ind,bis_ixd_de,	bis_ixd_ded,bis_ixd_ix,	bis_ixd_ixd,
	bis_ixd_rg,	bis_ixd_rgd,bis_ixd_in,	bis_ixd_ind,bis_ixd_de,	bis_ixd_ded,bis_ixd_ix,	bis_ixd_ixd,
	bis_ixd_rg,	bis_ixd_rgd,bis_ixd_in,	bis_ixd_ind,bis_ixd_de,	bis_ixd_ded,bis_ixd_ix,	bis_ixd_ixd,
	/* 0x5f00 */
	bis_ixd_rg,	bis_ixd_rgd,bis_ixd_in,	bis_ixd_ind,bis_ixd_de,	bis_ixd_ded,bis_ixd_ix,	bis_ixd_ixd,
	bis_ixd_rg,	bis_ixd_rgd,bis_ixd_in,	bis_ixd_ind,bis_ixd_de,	bis_ixd_ded,bis_ixd_ix,	bis_ixd_ixd,
	bis_ixd_rg,	bis_ixd_rgd,bis_ixd_in,	bis_ixd_ind,bis_ixd_de,	bis_ixd_ded,bis_ixd_ix,	bis_ixd_ixd,
	bis_ixd_rg,	bis_ixd_rgd,bis_ixd_in,	bis_ixd_ind,bis_ixd_de,	bis_ixd_ded,bis_ixd_ix,	bis_ixd_ixd,

	/* 0x6000 */
	add_rg_rg,	add_rg_rgd,	add_rg_in,	add_rg_ind,	add_rg_de,	add_rg_ded,	add_rg_ix,	add_rg_ixd,
	add_rg_rg,	add_rg_rgd,	add_rg_in,	add_rg_ind,	add_rg_de,	add_rg_ded,	add_rg_ix,	add_rg_ixd,
	add_rg_rg,	add_rg_rgd,	add_rg_in,	add_rg_ind,	add_rg_de,	add_rg_ded,	add_rg_ix,	add_rg_ixd,
	add_rg_rg,	add_rg_rgd,	add_rg_in,	add_rg_ind,	add_rg_de,	add_rg_ded,	add_rg_ix,	add_rg_ixd,
	/* 0x6100 */
	add_rg_rg,	add_rg_rgd,	add_rg_in,	add_rg_ind,	add_rg_de,	add_rg_ded,	add_rg_ix,	add_rg_ixd,
	add_rg_rg,	add_rg_rgd,	add_rg_in,	add_rg_ind,	add_rg_de,	add_rg_ded,	add_rg_ix,	add_rg_ixd,
	add_rg_rg,	add_rg_rgd,	add_rg_in,	add_rg_ind,	add_rg_de,	add_rg_ded,	add_rg_ix,	add_rg_ixd,
	add_rg_rg,	add_rg_rgd,	add_rg_in,	add_rg_ind,	add_rg_de,	add_rg_ded,	add_rg_ix,	add_rg_ixd,
	/* 0x6200 */
	add_rgd_rg,	add_rgd_rgd,add_rgd_in,	add_rgd_ind,add_rgd_de,	add_rgd_ded,add_rgd_ix,	add_rgd_ixd,
	add_rgd_rg,	add_rgd_rgd,add_rgd_in,	add_rgd_ind,add_rgd_de,	add_rgd_ded,add_rgd_ix,	add_rgd_ixd,
	add_rgd_rg,	add_rgd_rgd,add_rgd_in,	add_rgd_ind,add_rgd_de,	add_rgd_ded,add_rgd_ix,	add_rgd_ixd,
	add_rgd_rg,	add_rgd_rgd,add_rgd_in,	add_rgd_ind,add_rgd_de,	add_rgd_ded,add_rgd_ix,	add_rgd_ixd,
	/* 0x6300 */
	add_rgd_rg,	add_rgd_rgd,add_rgd_in,	add_rgd_ind,add_rgd_de,	add_rgd_ded,add_rgd_ix,	add_rgd_ixd,
	add_rgd_rg,	add_rgd_rgd,add_rgd_in,	add_rgd_ind,add_rgd_de,	add_rgd_ded,add_rgd_ix,	add_rgd_ixd,
	add_rgd_rg,	add_rgd_rgd,add_rgd_in,	add_rgd_ind,add_rgd_de,	add_rgd_ded,add_rgd_ix,	add_rgd_ixd,
	add_rgd_rg,	add_rgd_rgd,add_rgd_in,	add_rgd_ind,add_rgd_de,	add_rgd_ded,add_rgd_ix,	add_rgd_ixd,
	/* 0x6400 */
	add_in_rg,	add_in_rgd,	add_in_in,	add_in_ind,	add_in_de,	add_in_ded,	add_in_ix,	add_in_ixd,
	add_in_rg,	add_in_rgd,	add_in_in,	add_in_ind,	add_in_de,	add_in_ded,	add_in_ix,	add_in_ixd,
	add_in_rg,	add_in_rgd,	add_in_in,	add_in_ind,	add_in_de,	add_in_ded,	add_in_ix,	add_in_ixd,
	add_in_rg,	add_in_rgd,	add_in_in,	add_in_ind,	add_in_de,	add_in_ded,	add_in_ix,	add_in_ixd,
	/* 0x6500 */
	add_in_rg,	add_in_rgd,	add_in_in,	add_in_ind,	add_in_de,	add_in_ded,	add_in_ix,	add_in_ixd,
	add_in_rg,	add_in_rgd,	add_in_in,	add_in_ind,	add_in_de,	add_in_ded,	add_in_ix,	add_in_ixd,
	add_in_rg,	add_in_rgd,	add_in_in,	add_in_ind,	add_in_de,	add_in_ded,	add_in_ix,	add_in_ixd,
	add_in_rg,	add_in_rgd,	add_in_in,	add_in_ind,	add_in_de,	add_in_ded,	add_in_ix,	add_in_ixd,
	/* 0x6600 */
	add_ind_rg,	add_ind_rgd,add_ind_in,	add_ind_ind,add_ind_de,	add_ind_ded,add_ind_ix,	add_ind_ixd,
	add_ind_rg,	add_ind_rgd,add_ind_in,	add_ind_ind,add_ind_de,	add_ind_ded,add_ind_ix,	add_ind_ixd,
	add_ind_rg,	add_ind_rgd,add_ind_in,	add_ind_ind,add_ind_de,	add_ind_ded,add_ind_ix,	add_ind_ixd,
	add_ind_rg,	add_ind_rgd,add_ind_in,	add_ind_ind,add_ind_de,	add_ind_ded,add_ind_ix,	add_ind_ixd,
	/* 0x6700 */
	add_ind_rg,	add_ind_rgd,add_ind_in,	add_ind_ind,add_ind_de,	add_ind_ded,add_ind_ix,	add_ind_ixd,
	add_ind_rg,	add_ind_rgd,add_ind_in,	add_ind_ind,add_ind_de,	add_ind_ded,add_ind_ix,	add_ind_ixd,
	add_ind_rg,	add_ind_rgd,add_ind_in,	add_ind_ind,add_ind_de,	add_ind_ded,add_ind_ix,	add_ind_ixd,
	add_ind_rg,	add_ind_rgd,add_ind_in,	add_ind_ind,add_ind_de,	add_ind_ded,add_ind_ix,	add_ind_ixd,
	/* 0x6800 */
	add_de_rg,	add_de_rgd,	add_de_in,	add_de_ind,	add_de_de,	add_de_ded,	add_de_ix,	add_de_ixd,
	add_de_rg,	add_de_rgd,	add_de_in,	add_de_ind,	add_de_de,	add_de_ded,	add_de_ix,	add_de_ixd,
	add_de_rg,	add_de_rgd,	add_de_in,	add_de_ind,	add_de_de,	add_de_ded,	add_de_ix,	add_de_ixd,
	add_de_rg,	add_de_rgd,	add_de_in,	add_de_ind,	add_de_de,	add_de_ded,	add_de_ix,	add_de_ixd,
	/* 0x6900 */
	add_de_rg,	add_de_rgd,	add_de_in,	add_de_ind,	add_de_de,	add_de_ded,	add_de_ix,	add_de_ixd,
	add_de_rg,	add_de_rgd,	add_de_in,	add_de_ind,	add_de_de,	add_de_ded,	add_de_ix,	add_de_ixd,
	add_de_rg,	add_de_rgd,	add_de_in,	add_de_ind,	add_de_de,	add_de_ded,	add_de_ix,	add_de_ixd,
	add_de_rg,	add_de_rgd,	add_de_in,	add_de_ind,	add_de_de,	add_de_ded,	add_de_ix,	add_de_ixd,
	/* 0x6a00 */
	add_ded_rg,	add_ded_rgd,add_ded_in,	add_ded_ind,add_ded_de,	add_ded_ded,add_ded_ix,	add_ded_ixd,
	add_ded_rg,	add_ded_rgd,add_ded_in,	add_ded_ind,add_ded_de,	add_ded_ded,add_ded_ix,	add_ded_ixd,
	add_ded_rg,	add_ded_rgd,add_ded_in,	add_ded_ind,add_ded_de,	add_ded_ded,add_ded_ix,	add_ded_ixd,
	add_ded_rg,	add_ded_rgd,add_ded_in,	add_ded_ind,add_ded_de,	add_ded_ded,add_ded_ix,	add_ded_ixd,
	/* 0x6b00 */
	add_ded_rg,	add_ded_rgd,add_ded_in,	add_ded_ind,add_ded_de,	add_ded_ded,add_ded_ix,	add_ded_ixd,
	add_ded_rg,	add_ded_rgd,add_ded_in,	add_ded_ind,add_ded_de,	add_ded_ded,add_ded_ix,	add_ded_ixd,
	add_ded_rg,	add_ded_rgd,add_ded_in,	add_ded_ind,add_ded_de,	add_ded_ded,add_ded_ix,	add_ded_ixd,
	add_ded_rg,	add_ded_rgd,add_ded_in,	add_ded_ind,add_ded_de,	add_ded_ded,add_ded_ix,	add_ded_ixd,
	/* 0x6c00 */
	add_ix_rg,	add_ix_rgd,	add_ix_in,	add_ix_ind,	add_ix_de,	add_ix_ded,	add_ix_ix,	add_ix_ixd,
	add_ix_rg,	add_ix_rgd,	add_ix_in,	add_ix_ind,	add_ix_de,	add_ix_ded,	add_ix_ix,	add_ix_ixd,
	add_ix_rg,	add_ix_rgd,	add_ix_in,	add_ix_ind,	add_ix_de,	add_ix_ded,	add_ix_ix,	add_ix_ixd,
	add_ix_rg,	add_ix_rgd,	add_ix_in,	add_ix_ind,	add_ix_de,	add_ix_ded,	add_ix_ix,	add_ix_ixd,
	/* 0x6d00 */
	add_ix_rg,	add_ix_rgd,	add_ix_in,	add_ix_ind,	add_ix_de,	add_ix_ded,	add_ix_ix,	add_ix_ixd,
	add_ix_rg,	add_ix_rgd,	add_ix_in,	add_ix_ind,	add_ix_de,	add_ix_ded,	add_ix_ix,	add_ix_ixd,
	add_ix_rg,	add_ix_rgd,	add_ix_in,	add_ix_ind,	add_ix_de,	add_ix_ded,	add_ix_ix,	add_ix_ixd,
	add_ix_rg,	add_ix_rgd,	add_ix_in,	add_ix_ind,	add_ix_de,	add_ix_ded,	add_ix_ix,	add_ix_ixd,
	/* 0x6e00 */
	add_ixd_rg,	add_ixd_rgd,add_ixd_in,	add_ixd_ind,add_ixd_de,	add_ixd_ded,add_ixd_ix,	add_ixd_ixd,
	add_ixd_rg,	add_ixd_rgd,add_ixd_in,	add_ixd_ind,add_ixd_de,	add_ixd_ded,add_ixd_ix,	add_ixd_ixd,
	add_ixd_rg,	add_ixd_rgd,add_ixd_in,	add_ixd_ind,add_ixd_de,	add_ixd_ded,add_ixd_ix,	add_ixd_ixd,
	add_ixd_rg,	add_ixd_rgd,add_ixd_in,	add_ixd_ind,add_ixd_de,	add_ixd_ded,add_ixd_ix,	add_ixd_ixd,
	/* 0x6f00 */
	add_ixd_rg,	add_ixd_rgd,add_ixd_in,	add_ixd_ind,add_ixd_de,	add_ixd_ded,add_ixd_ix,	add_ixd_ixd,
	add_ixd_rg,	add_ixd_rgd,add_ixd_in,	add_ixd_ind,add_ixd_de,	add_ixd_ded,add_ixd_ix,	add_ixd_ixd,
	add_ixd_rg,	add_ixd_rgd,add_ixd_in,	add_ixd_ind,add_ixd_de,	add_ixd_ded,add_ixd_ix,	add_ixd_ixd,
	add_ixd_rg,	add_ixd_rgd,add_ixd_in,	add_ixd_ind,add_ixd_de,	add_ixd_ded,add_ixd_ix,	add_ixd_ixd,

	/* 0x7000 */
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	/* 0x7100 */
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	/* 0x7200 */
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	/* 0x7300 */
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	/* 0x7400 */
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	/* 0x7500 */
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	/* 0x7600 */
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	/* 0x7700 */
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	/* 0x7800 */
	xor_rg,		xor_rgd,	xor_in,		xor_ind,	xor_de,		xor_ded,	xor_ix,		xor_ixd,
	xor_rg,		xor_rgd,	xor_in,		xor_ind,	xor_de,		xor_ded,	xor_ix,		xor_ixd,
	xor_rg,		xor_rgd,	xor_in,		xor_ind,	xor_de,		xor_ded,	xor_ix,		xor_ixd,
	xor_rg,		xor_rgd,	xor_in,		xor_ind,	xor_de,		xor_ded,	xor_ix,		xor_ixd,
	/* 0x7900 */
	xor_rg,		xor_rgd,	xor_in,		xor_ind,	xor_de,		xor_ded,	xor_ix,		xor_ixd,
	xor_rg,		xor_rgd,	xor_in,		xor_ind,	xor_de,		xor_ded,	xor_ix,		xor_ixd,
	xor_rg,		xor_rgd,	xor_in,		xor_ind,	xor_de,		xor_ded,	xor_ix,		xor_ixd,
	xor_rg,		xor_rgd,	xor_in,		xor_ind,	xor_de,		xor_ded,	xor_ix,		xor_ixd,
	/* 0x7a00 */
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	/* 0x7b00 */
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	/* 0x7c00 */
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	/* 0x7d00 */
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	/* 0x7e00 */
	sob,		sob,		sob,		sob,		sob,		sob,		sob,		sob,
	sob,		sob,		sob,		sob,		sob,		sob,		sob,		sob,
	sob,		sob,		sob,		sob,		sob,		sob,		sob,		sob,
	sob,		sob,		sob,		sob,		sob,		sob,		sob,		sob,
	/* 0x7f00 */
	sob,		sob,		sob,		sob,		sob,		sob,		sob,		sob,
	sob,		sob,		sob,		sob,		sob,		sob,		sob,		sob,
	sob,		sob,		sob,		sob,		sob,		sob,		sob,		sob,
	sob,		sob,		sob,		sob,		sob,		sob,		sob,		sob,

	/* 0x8000 */
	bpl,		bpl,		bpl,		bpl,		bpl,		bpl,		bpl,		bpl,
	bpl,		bpl,		bpl,		bpl,		bpl,		bpl,		bpl,		bpl,
	bpl,		bpl,		bpl,		bpl,		bpl,		bpl,		bpl,		bpl,
	bpl,		bpl,		bpl,		bpl,		bpl,		bpl,		bpl,		bpl,
	/* 0x8100 */
	bmi,		bmi,		bmi,		bmi,		bmi,		bmi,		bmi,		bmi,
	bmi,		bmi,		bmi,		bmi,		bmi,		bmi,		bmi,		bmi,
	bmi,		bmi,		bmi,		bmi,		bmi,		bmi,		bmi,		bmi,
	bmi,		bmi,		bmi,		bmi,		bmi,		bmi,		bmi,		bmi,
	/* 0x8200 */
	bhi,		bhi,		bhi,		bhi,		bhi,		bhi,		bhi,		bhi,
	bhi,		bhi,		bhi,		bhi,		bhi,		bhi,		bhi,		bhi,
	bhi,		bhi,		bhi,		bhi,		bhi,		bhi,		bhi,		bhi,
	bhi,		bhi,		bhi,		bhi,		bhi,		bhi,		bhi,		bhi,
	/* 0x8300 */
	blos,		blos,		blos,		blos,		blos,		blos,		blos,		blos,
	blos,		blos,		blos,		blos,		blos,		blos,		blos,		blos,
	blos,		blos,		blos,		blos,		blos,		blos,		blos,		blos,
	blos,		blos,		blos,		blos,		blos,		blos,		blos,		blos,
	/* 0x8400 */
	bvc,		bvc,		bvc,		bvc,		bvc,		bvc,		bvc,		bvc,
	bvc,		bvc,		bvc,		bvc,		bvc,		bvc,		bvc,		bvc,
	bvc,		bvc,		bvc,		bvc,		bvc,		bvc,		bvc,		bvc,
	bvc,		bvc,		bvc,		bvc,		bvc,		bvc,		bvc,		bvc,
	/* 0x8500 */
	bvs,		bvs,		bvs,		bvs,		bvs,		bvs,		bvs,		bvs,
	bvs,		bvs,		bvs,		bvs,		bvs,		bvs,		bvs,		bvs,
	bvs,		bvs,		bvs,		bvs,		bvs,		bvs,		bvs,		bvs,
	bvs,		bvs,		bvs,		bvs,		bvs,		bvs,		bvs,		bvs,
	/* 0x8600 */
	bcc,		bcc,		bcc,		bcc,		bcc,		bcc,		bcc,		bcc,
	bcc,		bcc,		bcc,		bcc,		bcc,		bcc,		bcc,		bcc,
	bcc,		bcc,		bcc,		bcc,		bcc,		bcc,		bcc,		bcc,
	bcc,		bcc,		bcc,		bcc,		bcc,		bcc,		bcc,		bcc,
	/* 0x8700 */
	bcs,		bcs,		bcs,		bcs,		bcs,		bcs,		bcs,		bcs,
	bcs,		bcs,		bcs,		bcs,		bcs,		bcs,		bcs,		bcs,
	bcs,		bcs,		bcs,		bcs,		bcs,		bcs,		bcs,		bcs,
	bcs,		bcs,		bcs,		bcs,		bcs,		bcs,		bcs,		bcs,
	/* 0x8800 */
	emt,		emt,		emt,		emt,		emt,		emt,		emt,		emt,
	emt,		emt,		emt,		emt,		emt,		emt,		emt,		emt,
	emt,		emt,		emt,		emt,		emt,		emt,		emt,		emt,
	emt,		emt,		emt,		emt,		emt,		emt,		emt,		emt,
	/* 0x8900 */
	trap,		trap,		trap,		trap,		trap,		trap,		trap,		trap,
	trap,		trap,		trap,		trap,		trap,		trap,		trap,		trap,
	trap,		trap,		trap,		trap,		trap,		trap,		trap,		trap,
	trap,		trap,		trap,		trap,		trap,		trap,		trap,		trap,
	/* 0x8a00 */
	clrb_rg,	clrb_rgd,	clrb_in,	clrb_ind,	clrb_de,	clrb_ded,	clrb_ix,	clrb_ixd,
	comb_rg,	comb_rgd,	comb_in,	comb_ind,	comb_de,	comb_ded,	comb_ix,	comb_ixd,
	incb_rg,	incb_rgd,	incb_in,	incb_ind,	incb_de,	incb_ded,	incb_ix,	incb_ixd,
	decb_rg,	decb_rgd,	decb_in,	decb_ind,	decb_de,	decb_ded,	decb_ix,	decb_ixd,
	/* 0x8b00 */
	negb_rg,	negb_rgd,	negb_in,	negb_ind,	negb_de,	negb_ded,	negb_ix,	negb_ixd,
	adcb_rg,	adcb_rgd,	adcb_in,	adcb_ind,	adcb_de,	adcb_ded,	adcb_ix,	adcb_ixd,
	sbcb_rg,	sbcb_rgd,	sbcb_in,	sbcb_ind,	sbcb_de,	sbcb_ded,	sbcb_ix,	sbcb_ixd,
	tstb_rg,	tstb_rgd,	tstb_in,	tstb_ind,	tstb_de,	tstb_ded,	tstb_ix,	tstb_ixd,
	/* 0x8c00 */
	rorb_rg,	rorb_rgd,	rorb_in,	rorb_ind,	rorb_de,	rorb_ded,	rorb_ix,	rorb_ixd,
	rolb_rg,	rolb_rgd,	rolb_in,	rolb_ind,	rolb_de,	rolb_ded,	rolb_ix,	rolb_ixd,
	asrb_rg,	asrb_rgd,	asrb_in,	asrb_ind,	asrb_de,	asrb_ded,	asrb_ix,	asrb_ixd,
	aslb_rg,	aslb_rgd,	aslb_in,	aslb_ind,	aslb_de,	aslb_ded,	aslb_ix,	aslb_ixd,
	/* 0x8d00 */
	mtps_rg,	mtps_rgd,	mtps_in,	mtps_ind,	mtps_de,	mtps_ded,	mtps_ix,	mtps_ixd,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	mfps_rg,	mfps_rgd,	mfps_in,	mfps_ind,	mfps_de,	mfps_ded,	mfps_ix,	mfps_ixd,
	/* 0x8e00 */
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	/* 0x8f00 */
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,

	/* 0x9000 */
	movb_rg_rg,	movb_rg_rgd,movb_rg_in,	movb_rg_ind,movb_rg_de,	movb_rg_ded,movb_rg_ix,	movb_rg_ixd,
	movb_rg_rg,	movb_rg_rgd,movb_rg_in,	movb_rg_ind,movb_rg_de,	movb_rg_ded,movb_rg_ix,	movb_rg_ixd,
	movb_rg_rg,	movb_rg_rgd,movb_rg_in,	movb_rg_ind,movb_rg_de,	movb_rg_ded,movb_rg_ix,	movb_rg_ixd,
	movb_rg_rg,	movb_rg_rgd,movb_rg_in,	movb_rg_ind,movb_rg_de,	movb_rg_ded,movb_rg_ix,	movb_rg_ixd,
	/* 0x9100 */
	movb_rg_rg,	movb_rg_rgd,movb_rg_in,	movb_rg_ind,movb_rg_de,	movb_rg_ded,movb_rg_ix,	movb_rg_ixd,
	movb_rg_rg,	movb_rg_rgd,movb_rg_in,	movb_rg_ind,movb_rg_de,	movb_rg_ded,movb_rg_ix,	movb_rg_ixd,
	movb_rg_rg,	movb_rg_rgd,movb_rg_in,	movb_rg_ind,movb_rg_de,	movb_rg_ded,movb_rg_ix,	movb_rg_ixd,
	movb_rg_rg,	movb_rg_rgd,movb_rg_in,	movb_rg_ind,movb_rg_de,	movb_rg_ded,movb_rg_ix,	movb_rg_ixd,
	/* 0x9200 */
	movb_rgd_rg,movb_rgd_rgd,movb_rgd_in,movb_rgd_ind,movb_rgd_de,movb_rgd_ded,movb_rgd_ix,movb_rgd_ixd,
	movb_rgd_rg,movb_rgd_rgd,movb_rgd_in,movb_rgd_ind,movb_rgd_de,movb_rgd_ded,movb_rgd_ix,movb_rgd_ixd,
	movb_rgd_rg,movb_rgd_rgd,movb_rgd_in,movb_rgd_ind,movb_rgd_de,movb_rgd_ded,movb_rgd_ix,movb_rgd_ixd,
	movb_rgd_rg,movb_rgd_rgd,movb_rgd_in,movb_rgd_ind,movb_rgd_de,movb_rgd_ded,movb_rgd_ix,movb_rgd_ixd,
	/* 0x9300 */
	movb_rgd_rg,movb_rgd_rgd,movb_rgd_in,movb_rgd_ind,movb_rgd_de,movb_rgd_ded,movb_rgd_ix,movb_rgd_ixd,
	movb_rgd_rg,movb_rgd_rgd,movb_rgd_in,movb_rgd_ind,movb_rgd_de,movb_rgd_ded,movb_rgd_ix,movb_rgd_ixd,
	movb_rgd_rg,movb_rgd_rgd,movb_rgd_in,movb_rgd_ind,movb_rgd_de,movb_rgd_ded,movb_rgd_ix,movb_rgd_ixd,
	movb_rgd_rg,movb_rgd_rgd,movb_rgd_in,movb_rgd_ind,movb_rgd_de,movb_rgd_ded,movb_rgd_ix,movb_rgd_ixd,
	/* 0x9400 */
	movb_in_rg,	movb_in_rgd,movb_in_in,	movb_in_ind,movb_in_de,	movb_in_ded,movb_in_ix,	movb_in_ixd,
	movb_in_rg,	movb_in_rgd,movb_in_in,	movb_in_ind,movb_in_de,	movb_in_ded,movb_in_ix,	movb_in_ixd,
	movb_in_rg,	movb_in_rgd,movb_in_in,	movb_in_ind,movb_in_de,	movb_in_ded,movb_in_ix,	movb_in_ixd,
	movb_in_rg,	movb_in_rgd,movb_in_in,	movb_in_ind,movb_in_de,	movb_in_ded,movb_in_ix,	movb_in_ixd,
	/* 0x9500 */
	movb_in_rg,	movb_in_rgd,movb_in_in,	movb_in_ind,movb_in_de,	movb_in_ded,movb_in_ix,	movb_in_ixd,
	movb_in_rg,	movb_in_rgd,movb_in_in,	movb_in_ind,movb_in_de,	movb_in_ded,movb_in_ix,	movb_in_ixd,
	movb_in_rg,	movb_in_rgd,movb_in_in,	movb_in_ind,movb_in_de,	movb_in_ded,movb_in_ix,	movb_in_ixd,
	movb_in_rg,	movb_in_rgd,movb_in_in,	movb_in_ind,movb_in_de,	movb_in_ded,movb_in_ix,	movb_in_ixd,
	/* 0x9600 */
	movb_ind_rg,movb_ind_rgd,movb_ind_in,movb_ind_ind,movb_ind_de,movb_ind_ded,movb_ind_ix,movb_ind_ixd,
	movb_ind_rg,movb_ind_rgd,movb_ind_in,movb_ind_ind,movb_ind_de,movb_ind_ded,movb_ind_ix,movb_ind_ixd,
	movb_ind_rg,movb_ind_rgd,movb_ind_in,movb_ind_ind,movb_ind_de,movb_ind_ded,movb_ind_ix,movb_ind_ixd,
	movb_ind_rg,movb_ind_rgd,movb_ind_in,movb_ind_ind,movb_ind_de,movb_ind_ded,movb_ind_ix,movb_ind_ixd,
	/* 0x9700 */
	movb_ind_rg,movb_ind_rgd,movb_ind_in,movb_ind_ind,movb_ind_de,movb_ind_ded,movb_ind_ix,movb_ind_ixd,
	movb_ind_rg,movb_ind_rgd,movb_ind_in,movb_ind_ind,movb_ind_de,movb_ind_ded,movb_ind_ix,movb_ind_ixd,
	movb_ind_rg,movb_ind_rgd,movb_ind_in,movb_ind_ind,movb_ind_de,movb_ind_ded,movb_ind_ix,movb_ind_ixd,
	movb_ind_rg,movb_ind_rgd,movb_ind_in,movb_ind_ind,movb_ind_de,movb_ind_ded,movb_ind_ix,movb_ind_ixd,
	/* 0x9800 */
	movb_de_rg,	movb_de_rgd,movb_de_in,	movb_de_ind,movb_de_de,	movb_de_ded,movb_de_ix,	movb_de_ixd,
	movb_de_rg,	movb_de_rgd,movb_de_in,	movb_de_ind,movb_de_de,	movb_de_ded,movb_de_ix,	movb_de_ixd,
	movb_de_rg,	movb_de_rgd,movb_de_in,	movb_de_ind,movb_de_de,	movb_de_ded,movb_de_ix,	movb_de_ixd,
	movb_de_rg,	movb_de_rgd,movb_de_in,	movb_de_ind,movb_de_de,	movb_de_ded,movb_de_ix,	movb_de_ixd,
	/* 0x9900 */
	movb_de_rg,	movb_de_rgd,movb_de_in,	movb_de_ind,movb_de_de,	movb_de_ded,movb_de_ix,	movb_de_ixd,
	movb_de_rg,	movb_de_rgd,movb_de_in,	movb_de_ind,movb_de_de,	movb_de_ded,movb_de_ix,	movb_de_ixd,
	movb_de_rg,	movb_de_rgd,movb_de_in,	movb_de_ind,movb_de_de,	movb_de_ded,movb_de_ix,	movb_de_ixd,
	movb_de_rg,	movb_de_rgd,movb_de_in,	movb_de_ind,movb_de_de,	movb_de_ded,movb_de_ix,	movb_de_ixd,
	/* 0x9a00 */
	movb_ded_rg,movb_ded_rgd,movb_ded_in,movb_ded_ind,movb_ded_de,movb_ded_ded,movb_ded_ix,movb_ded_ixd,
	movb_ded_rg,movb_ded_rgd,movb_ded_in,movb_ded_ind,movb_ded_de,movb_ded_ded,movb_ded_ix,movb_ded_ixd,
	movb_ded_rg,movb_ded_rgd,movb_ded_in,movb_ded_ind,movb_ded_de,movb_ded_ded,movb_ded_ix,movb_ded_ixd,
	movb_ded_rg,movb_ded_rgd,movb_ded_in,movb_ded_ind,movb_ded_de,movb_ded_ded,movb_ded_ix,movb_ded_ixd,
	/* 0x9b00 */
	movb_ded_rg,movb_ded_rgd,movb_ded_in,movb_ded_ind,movb_ded_de,movb_ded_ded,movb_ded_ix,movb_ded_ixd,
	movb_ded_rg,movb_ded_rgd,movb_ded_in,movb_ded_ind,movb_ded_de,movb_ded_ded,movb_ded_ix,movb_ded_ixd,
	movb_ded_rg,movb_ded_rgd,movb_ded_in,movb_ded_ind,movb_ded_de,movb_ded_ded,movb_ded_ix,movb_ded_ixd,
	movb_ded_rg,movb_ded_rgd,movb_ded_in,movb_ded_ind,movb_ded_de,movb_ded_ded,movb_ded_ix,movb_ded_ixd,
	/* 0x9c00 */
	movb_ix_rg,	movb_ix_rgd,movb_ix_in,	movb_ix_ind,movb_ix_de,	movb_ix_ded,movb_ix_ix,	movb_ix_ixd,
	movb_ix_rg,	movb_ix_rgd,movb_ix_in,	movb_ix_ind,movb_ix_de,	movb_ix_ded,movb_ix_ix,	movb_ix_ixd,
	movb_ix_rg,	movb_ix_rgd,movb_ix_in,	movb_ix_ind,movb_ix_de,	movb_ix_ded,movb_ix_ix,	movb_ix_ixd,
	movb_ix_rg,	movb_ix_rgd,movb_ix_in,	movb_ix_ind,movb_ix_de,	movb_ix_ded,movb_ix_ix,	movb_ix_ixd,
	/* 0x9d00 */
	movb_ix_rg,	movb_ix_rgd,movb_ix_in,	movb_ix_ind,movb_ix_de,	movb_ix_ded,movb_ix_ix,	movb_ix_ixd,
	movb_ix_rg,	movb_ix_rgd,movb_ix_in,	movb_ix_ind,movb_ix_de,	movb_ix_ded,movb_ix_ix,	movb_ix_ixd,
	movb_ix_rg,	movb_ix_rgd,movb_ix_in,	movb_ix_ind,movb_ix_de,	movb_ix_ded,movb_ix_ix,	movb_ix_ixd,
	movb_ix_rg,	movb_ix_rgd,movb_ix_in,	movb_ix_ind,movb_ix_de,	movb_ix_ded,movb_ix_ix,	movb_ix_ixd,
	/* 0x9e00 */
	movb_ixd_rg,movb_ixd_rgd,movb_ixd_in,movb_ixd_ind,movb_ixd_de,movb_ixd_ded,movb_ixd_ix,movb_ixd_ixd,
	movb_ixd_rg,movb_ixd_rgd,movb_ixd_in,movb_ixd_ind,movb_ixd_de,movb_ixd_ded,movb_ixd_ix,movb_ixd_ixd,
	movb_ixd_rg,movb_ixd_rgd,movb_ixd_in,movb_ixd_ind,movb_ixd_de,movb_ixd_ded,movb_ixd_ix,movb_ixd_ixd,
	movb_ixd_rg,movb_ixd_rgd,movb_ixd_in,movb_ixd_ind,movb_ixd_de,movb_ixd_ded,movb_ixd_ix,movb_ixd_ixd,
	/* 0x9f00 */
	movb_ixd_rg,movb_ixd_rgd,movb_ixd_in,movb_ixd_ind,movb_ixd_de,movb_ixd_ded,movb_ixd_ix,movb_ixd_ixd,
	movb_ixd_rg,movb_ixd_rgd,movb_ixd_in,movb_ixd_ind,movb_ixd_de,movb_ixd_ded,movb_ixd_ix,movb_ixd_ixd,
	movb_ixd_rg,movb_ixd_rgd,movb_ixd_in,movb_ixd_ind,movb_ixd_de,movb_ixd_ded,movb_ixd_ix,movb_ixd_ixd,
	movb_ixd_rg,movb_ixd_rgd,movb_ixd_in,movb_ixd_ind,movb_ixd_de,movb_ixd_ded,movb_ixd_ix,movb_ixd_ixd,

	/* 0xa000 */
	cmpb_rg_rg,	cmpb_rg_rgd,cmpb_rg_in,	cmpb_rg_ind,cmpb_rg_de,	cmpb_rg_ded,cmpb_rg_ix,	cmpb_rg_ixd,
	cmpb_rg_rg,	cmpb_rg_rgd,cmpb_rg_in,	cmpb_rg_ind,cmpb_rg_de,	cmpb_rg_ded,cmpb_rg_ix,	cmpb_rg_ixd,
	cmpb_rg_rg,	cmpb_rg_rgd,cmpb_rg_in,	cmpb_rg_ind,cmpb_rg_de,	cmpb_rg_ded,cmpb_rg_ix,	cmpb_rg_ixd,
	cmpb_rg_rg,	cmpb_rg_rgd,cmpb_rg_in,	cmpb_rg_ind,cmpb_rg_de,	cmpb_rg_ded,cmpb_rg_ix,	cmpb_rg_ixd,
	/* 0xa100 */
	cmpb_rg_rg,	cmpb_rg_rgd,cmpb_rg_in,	cmpb_rg_ind,cmpb_rg_de,	cmpb_rg_ded,cmpb_rg_ix,	cmpb_rg_ixd,
	cmpb_rg_rg,	cmpb_rg_rgd,cmpb_rg_in,	cmpb_rg_ind,cmpb_rg_de,	cmpb_rg_ded,cmpb_rg_ix,	cmpb_rg_ixd,
	cmpb_rg_rg,	cmpb_rg_rgd,cmpb_rg_in,	cmpb_rg_ind,cmpb_rg_de,	cmpb_rg_ded,cmpb_rg_ix,	cmpb_rg_ixd,
	cmpb_rg_rg,	cmpb_rg_rgd,cmpb_rg_in,	cmpb_rg_ind,cmpb_rg_de,	cmpb_rg_ded,cmpb_rg_ix,	cmpb_rg_ixd,
	/* 0xa200 */
	cmpb_rgd_rg,cmpb_rgd_rgd,cmpb_rgd_in,cmpb_rgd_ind,cmpb_rgd_de,cmpb_rgd_ded,cmpb_rgd_ix,cmpb_rgd_ixd,
	cmpb_rgd_rg,cmpb_rgd_rgd,cmpb_rgd_in,cmpb_rgd_ind,cmpb_rgd_de,cmpb_rgd_ded,cmpb_rgd_ix,cmpb_rgd_ixd,
	cmpb_rgd_rg,cmpb_rgd_rgd,cmpb_rgd_in,cmpb_rgd_ind,cmpb_rgd_de,cmpb_rgd_ded,cmpb_rgd_ix,cmpb_rgd_ixd,
	cmpb_rgd_rg,cmpb_rgd_rgd,cmpb_rgd_in,cmpb_rgd_ind,cmpb_rgd_de,cmpb_rgd_ded,cmpb_rgd_ix,cmpb_rgd_ixd,
	/* 0xa300 */
	cmpb_rgd_rg,cmpb_rgd_rgd,cmpb_rgd_in,cmpb_rgd_ind,cmpb_rgd_de,cmpb_rgd_ded,cmpb_rgd_ix,cmpb_rgd_ixd,
	cmpb_rgd_rg,cmpb_rgd_rgd,cmpb_rgd_in,cmpb_rgd_ind,cmpb_rgd_de,cmpb_rgd_ded,cmpb_rgd_ix,cmpb_rgd_ixd,
	cmpb_rgd_rg,cmpb_rgd_rgd,cmpb_rgd_in,cmpb_rgd_ind,cmpb_rgd_de,cmpb_rgd_ded,cmpb_rgd_ix,cmpb_rgd_ixd,
	cmpb_rgd_rg,cmpb_rgd_rgd,cmpb_rgd_in,cmpb_rgd_ind,cmpb_rgd_de,cmpb_rgd_ded,cmpb_rgd_ix,cmpb_rgd_ixd,
	/* 0xa400 */
	cmpb_in_rg,	cmpb_in_rgd,cmpb_in_in,	cmpb_in_ind,cmpb_in_de,	cmpb_in_ded,cmpb_in_ix,	cmpb_in_ixd,
	cmpb_in_rg,	cmpb_in_rgd,cmpb_in_in,	cmpb_in_ind,cmpb_in_de,	cmpb_in_ded,cmpb_in_ix,	cmpb_in_ixd,
	cmpb_in_rg,	cmpb_in_rgd,cmpb_in_in,	cmpb_in_ind,cmpb_in_de,	cmpb_in_ded,cmpb_in_ix,	cmpb_in_ixd,
	cmpb_in_rg,	cmpb_in_rgd,cmpb_in_in,	cmpb_in_ind,cmpb_in_de,	cmpb_in_ded,cmpb_in_ix,	cmpb_in_ixd,
	/* 0xa500 */
	cmpb_in_rg,	cmpb_in_rgd,cmpb_in_in,	cmpb_in_ind,cmpb_in_de,	cmpb_in_ded,cmpb_in_ix,	cmpb_in_ixd,
	cmpb_in_rg,	cmpb_in_rgd,cmpb_in_in,	cmpb_in_ind,cmpb_in_de,	cmpb_in_ded,cmpb_in_ix,	cmpb_in_ixd,
	cmpb_in_rg,	cmpb_in_rgd,cmpb_in_in,	cmpb_in_ind,cmpb_in_de,	cmpb_in_ded,cmpb_in_ix,	cmpb_in_ixd,
	cmpb_in_rg,	cmpb_in_rgd,cmpb_in_in,	cmpb_in_ind,cmpb_in_de,	cmpb_in_ded,cmpb_in_ix,	cmpb_in_ixd,
	/* 0xa600 */
	cmpb_ind_rg,cmpb_ind_rgd,cmpb_ind_in,cmpb_ind_ind,cmpb_ind_de,cmpb_ind_ded,cmpb_ind_ix,cmpb_ind_ixd,
	cmpb_ind_rg,cmpb_ind_rgd,cmpb_ind_in,cmpb_ind_ind,cmpb_ind_de,cmpb_ind_ded,cmpb_ind_ix,cmpb_ind_ixd,
	cmpb_ind_rg,cmpb_ind_rgd,cmpb_ind_in,cmpb_ind_ind,cmpb_ind_de,cmpb_ind_ded,cmpb_ind_ix,cmpb_ind_ixd,
	cmpb_ind_rg,cmpb_ind_rgd,cmpb_ind_in,cmpb_ind_ind,cmpb_ind_de,cmpb_ind_ded,cmpb_ind_ix,cmpb_ind_ixd,
	/* 0xa700 */
	cmpb_ind_rg,cmpb_ind_rgd,cmpb_ind_in,cmpb_ind_ind,cmpb_ind_de,cmpb_ind_ded,cmpb_ind_ix,cmpb_ind_ixd,
	cmpb_ind_rg,cmpb_ind_rgd,cmpb_ind_in,cmpb_ind_ind,cmpb_ind_de,cmpb_ind_ded,cmpb_ind_ix,cmpb_ind_ixd,
	cmpb_ind_rg,cmpb_ind_rgd,cmpb_ind_in,cmpb_ind_ind,cmpb_ind_de,cmpb_ind_ded,cmpb_ind_ix,cmpb_ind_ixd,
	cmpb_ind_rg,cmpb_ind_rgd,cmpb_ind_in,cmpb_ind_ind,cmpb_ind_de,cmpb_ind_ded,cmpb_ind_ix,cmpb_ind_ixd,
	/* 0xa800 */
	cmpb_de_rg,	cmpb_de_rgd,cmpb_de_in,	cmpb_de_ind,cmpb_de_de,	cmpb_de_ded,cmpb_de_ix,	cmpb_de_ixd,
	cmpb_de_rg,	cmpb_de_rgd,cmpb_de_in,	cmpb_de_ind,cmpb_de_de,	cmpb_de_ded,cmpb_de_ix,	cmpb_de_ixd,
	cmpb_de_rg,	cmpb_de_rgd,cmpb_de_in,	cmpb_de_ind,cmpb_de_de,	cmpb_de_ded,cmpb_de_ix,	cmpb_de_ixd,
	cmpb_de_rg,	cmpb_de_rgd,cmpb_de_in,	cmpb_de_ind,cmpb_de_de,	cmpb_de_ded,cmpb_de_ix,	cmpb_de_ixd,
	/* 0xa900 */
	cmpb_de_rg,	cmpb_de_rgd,cmpb_de_in,	cmpb_de_ind,cmpb_de_de,	cmpb_de_ded,cmpb_de_ix,	cmpb_de_ixd,
	cmpb_de_rg,	cmpb_de_rgd,cmpb_de_in,	cmpb_de_ind,cmpb_de_de,	cmpb_de_ded,cmpb_de_ix,	cmpb_de_ixd,
	cmpb_de_rg,	cmpb_de_rgd,cmpb_de_in,	cmpb_de_ind,cmpb_de_de,	cmpb_de_ded,cmpb_de_ix,	cmpb_de_ixd,
	cmpb_de_rg,	cmpb_de_rgd,cmpb_de_in,	cmpb_de_ind,cmpb_de_de,	cmpb_de_ded,cmpb_de_ix,	cmpb_de_ixd,
	/* 0xaa00 */
	cmpb_ded_rg,cmpb_ded_rgd,cmpb_ded_in,cmpb_ded_ind,cmpb_ded_de,cmpb_ded_ded,cmpb_ded_ix,cmpb_ded_ixd,
	cmpb_ded_rg,cmpb_ded_rgd,cmpb_ded_in,cmpb_ded_ind,cmpb_ded_de,cmpb_ded_ded,cmpb_ded_ix,cmpb_ded_ixd,
	cmpb_ded_rg,cmpb_ded_rgd,cmpb_ded_in,cmpb_ded_ind,cmpb_ded_de,cmpb_ded_ded,cmpb_ded_ix,cmpb_ded_ixd,
	cmpb_ded_rg,cmpb_ded_rgd,cmpb_ded_in,cmpb_ded_ind,cmpb_ded_de,cmpb_ded_ded,cmpb_ded_ix,cmpb_ded_ixd,
	/* 0xab00 */
	cmpb_ded_rg,cmpb_ded_rgd,cmpb_ded_in,cmpb_ded_ind,cmpb_ded_de,cmpb_ded_ded,cmpb_ded_ix,cmpb_ded_ixd,
	cmpb_ded_rg,cmpb_ded_rgd,cmpb_ded_in,cmpb_ded_ind,cmpb_ded_de,cmpb_ded_ded,cmpb_ded_ix,cmpb_ded_ixd,
	cmpb_ded_rg,cmpb_ded_rgd,cmpb_ded_in,cmpb_ded_ind,cmpb_ded_de,cmpb_ded_ded,cmpb_ded_ix,cmpb_ded_ixd,
	cmpb_ded_rg,cmpb_ded_rgd,cmpb_ded_in,cmpb_ded_ind,cmpb_ded_de,cmpb_ded_ded,cmpb_ded_ix,cmpb_ded_ixd,
	/* 0xac00 */
	cmpb_ix_rg,	cmpb_ix_rgd,cmpb_ix_in,	cmpb_ix_ind,cmpb_ix_de,	cmpb_ix_ded,cmpb_ix_ix,	cmpb_ix_ixd,
	cmpb_ix_rg,	cmpb_ix_rgd,cmpb_ix_in,	cmpb_ix_ind,cmpb_ix_de,	cmpb_ix_ded,cmpb_ix_ix,	cmpb_ix_ixd,
	cmpb_ix_rg,	cmpb_ix_rgd,cmpb_ix_in,	cmpb_ix_ind,cmpb_ix_de,	cmpb_ix_ded,cmpb_ix_ix,	cmpb_ix_ixd,
	cmpb_ix_rg,	cmpb_ix_rgd,cmpb_ix_in,	cmpb_ix_ind,cmpb_ix_de,	cmpb_ix_ded,cmpb_ix_ix,	cmpb_ix_ixd,
	/* 0xad00 */
	cmpb_ix_rg,	cmpb_ix_rgd,cmpb_ix_in,	cmpb_ix_ind,cmpb_ix_de,	cmpb_ix_ded,cmpb_ix_ix,	cmpb_ix_ixd,
	cmpb_ix_rg,	cmpb_ix_rgd,cmpb_ix_in,	cmpb_ix_ind,cmpb_ix_de,	cmpb_ix_ded,cmpb_ix_ix,	cmpb_ix_ixd,
	cmpb_ix_rg,	cmpb_ix_rgd,cmpb_ix_in,	cmpb_ix_ind,cmpb_ix_de,	cmpb_ix_ded,cmpb_ix_ix,	cmpb_ix_ixd,
	cmpb_ix_rg,	cmpb_ix_rgd,cmpb_ix_in,	cmpb_ix_ind,cmpb_ix_de,	cmpb_ix_ded,cmpb_ix_ix,	cmpb_ix_ixd,
	/* 0xae00 */
	cmpb_ixd_rg,cmpb_ixd_rgd,cmpb_ixd_in,cmpb_ixd_ind,cmpb_ixd_de,cmpb_ixd_ded,cmpb_ixd_ix,cmpb_ixd_ixd,
	cmpb_ixd_rg,cmpb_ixd_rgd,cmpb_ixd_in,cmpb_ixd_ind,cmpb_ixd_de,cmpb_ixd_ded,cmpb_ixd_ix,cmpb_ixd_ixd,
	cmpb_ixd_rg,cmpb_ixd_rgd,cmpb_ixd_in,cmpb_ixd_ind,cmpb_ixd_de,cmpb_ixd_ded,cmpb_ixd_ix,cmpb_ixd_ixd,
	cmpb_ixd_rg,cmpb_ixd_rgd,cmpb_ixd_in,cmpb_ixd_ind,cmpb_ixd_de,cmpb_ixd_ded,cmpb_ixd_ix,cmpb_ixd_ixd,
	/* 0xaf00 */
	cmpb_ixd_rg,cmpb_ixd_rgd,cmpb_ixd_in,cmpb_ixd_ind,cmpb_ixd_de,cmpb_ixd_ded,cmpb_ixd_ix,cmpb_ixd_ixd,
	cmpb_ixd_rg,cmpb_ixd_rgd,cmpb_ixd_in,cmpb_ixd_ind,cmpb_ixd_de,cmpb_ixd_ded,cmpb_ixd_ix,cmpb_ixd_ixd,
	cmpb_ixd_rg,cmpb_ixd_rgd,cmpb_ixd_in,cmpb_ixd_ind,cmpb_ixd_de,cmpb_ixd_ded,cmpb_ixd_ix,cmpb_ixd_ixd,
	cmpb_ixd_rg,cmpb_ixd_rgd,cmpb_ixd_in,cmpb_ixd_ind,cmpb_ixd_de,cmpb_ixd_ded,cmpb_ixd_ix,cmpb_ixd_ixd,

	/* 0xb000 */
	bitb_rg_rg,	bitb_rg_rgd,bitb_rg_in,	bitb_rg_ind,bitb_rg_de,	bitb_rg_ded,bitb_rg_ix,	bitb_rg_ixd,
	bitb_rg_rg,	bitb_rg_rgd,bitb_rg_in,	bitb_rg_ind,bitb_rg_de,	bitb_rg_ded,bitb_rg_ix,	bitb_rg_ixd,
	bitb_rg_rg,	bitb_rg_rgd,bitb_rg_in,	bitb_rg_ind,bitb_rg_de,	bitb_rg_ded,bitb_rg_ix,	bitb_rg_ixd,
	bitb_rg_rg,	bitb_rg_rgd,bitb_rg_in,	bitb_rg_ind,bitb_rg_de,	bitb_rg_ded,bitb_rg_ix,	bitb_rg_ixd,
	/* 0xb100 */
	bitb_rg_rg,	bitb_rg_rgd,bitb_rg_in,	bitb_rg_ind,bitb_rg_de,	bitb_rg_ded,bitb_rg_ix,	bitb_rg_ixd,
	bitb_rg_rg,	bitb_rg_rgd,bitb_rg_in,	bitb_rg_ind,bitb_rg_de,	bitb_rg_ded,bitb_rg_ix,	bitb_rg_ixd,
	bitb_rg_rg,	bitb_rg_rgd,bitb_rg_in,	bitb_rg_ind,bitb_rg_de,	bitb_rg_ded,bitb_rg_ix,	bitb_rg_ixd,
	bitb_rg_rg,	bitb_rg_rgd,bitb_rg_in,	bitb_rg_ind,bitb_rg_de,	bitb_rg_ded,bitb_rg_ix,	bitb_rg_ixd,
	/* 0xb200 */
	bitb_rgd_rg,bitb_rgd_rgd,bitb_rgd_in,bitb_rgd_ind,bitb_rgd_de,bitb_rgd_ded,bitb_rgd_ix,bitb_rgd_ixd,
	bitb_rgd_rg,bitb_rgd_rgd,bitb_rgd_in,bitb_rgd_ind,bitb_rgd_de,bitb_rgd_ded,bitb_rgd_ix,bitb_rgd_ixd,
	bitb_rgd_rg,bitb_rgd_rgd,bitb_rgd_in,bitb_rgd_ind,bitb_rgd_de,bitb_rgd_ded,bitb_rgd_ix,bitb_rgd_ixd,
	bitb_rgd_rg,bitb_rgd_rgd,bitb_rgd_in,bitb_rgd_ind,bitb_rgd_de,bitb_rgd_ded,bitb_rgd_ix,bitb_rgd_ixd,
	/* 0xb300 */
	bitb_rgd_rg,bitb_rgd_rgd,bitb_rgd_in,bitb_rgd_ind,bitb_rgd_de,bitb_rgd_ded,bitb_rgd_ix,bitb_rgd_ixd,
	bitb_rgd_rg,bitb_rgd_rgd,bitb_rgd_in,bitb_rgd_ind,bitb_rgd_de,bitb_rgd_ded,bitb_rgd_ix,bitb_rgd_ixd,
	bitb_rgd_rg,bitb_rgd_rgd,bitb_rgd_in,bitb_rgd_ind,bitb_rgd_de,bitb_rgd_ded,bitb_rgd_ix,bitb_rgd_ixd,
	bitb_rgd_rg,bitb_rgd_rgd,bitb_rgd_in,bitb_rgd_ind,bitb_rgd_de,bitb_rgd_ded,bitb_rgd_ix,bitb_rgd_ixd,
	/* 0xb400 */
	bitb_in_rg,	bitb_in_rgd,bitb_in_in,	bitb_in_ind,bitb_in_de,	bitb_in_ded,bitb_in_ix,	bitb_in_ixd,
	bitb_in_rg,	bitb_in_rgd,bitb_in_in,	bitb_in_ind,bitb_in_de,	bitb_in_ded,bitb_in_ix,	bitb_in_ixd,
	bitb_in_rg,	bitb_in_rgd,bitb_in_in,	bitb_in_ind,bitb_in_de,	bitb_in_ded,bitb_in_ix,	bitb_in_ixd,
	bitb_in_rg,	bitb_in_rgd,bitb_in_in,	bitb_in_ind,bitb_in_de,	bitb_in_ded,bitb_in_ix,	bitb_in_ixd,
	/* 0xb500 */
	bitb_in_rg,	bitb_in_rgd,bitb_in_in,	bitb_in_ind,bitb_in_de,	bitb_in_ded,bitb_in_ix,	bitb_in_ixd,
	bitb_in_rg,	bitb_in_rgd,bitb_in_in,	bitb_in_ind,bitb_in_de,	bitb_in_ded,bitb_in_ix,	bitb_in_ixd,
	bitb_in_rg,	bitb_in_rgd,bitb_in_in,	bitb_in_ind,bitb_in_de,	bitb_in_ded,bitb_in_ix,	bitb_in_ixd,
	bitb_in_rg,	bitb_in_rgd,bitb_in_in,	bitb_in_ind,bitb_in_de,	bitb_in_ded,bitb_in_ix,	bitb_in_ixd,
	/* 0xb600 */
	bitb_ind_rg,bitb_ind_rgd,bitb_ind_in,bitb_ind_ind,bitb_ind_de,bitb_ind_ded,bitb_ind_ix,bitb_ind_ixd,
	bitb_ind_rg,bitb_ind_rgd,bitb_ind_in,bitb_ind_ind,bitb_ind_de,bitb_ind_ded,bitb_ind_ix,bitb_ind_ixd,
	bitb_ind_rg,bitb_ind_rgd,bitb_ind_in,bitb_ind_ind,bitb_ind_de,bitb_ind_ded,bitb_ind_ix,bitb_ind_ixd,
	bitb_ind_rg,bitb_ind_rgd,bitb_ind_in,bitb_ind_ind,bitb_ind_de,bitb_ind_ded,bitb_ind_ix,bitb_ind_ixd,
	/* 0xb700 */
	bitb_ind_rg,bitb_ind_rgd,bitb_ind_in,bitb_ind_ind,bitb_ind_de,bitb_ind_ded,bitb_ind_ix,bitb_ind_ixd,
	bitb_ind_rg,bitb_ind_rgd,bitb_ind_in,bitb_ind_ind,bitb_ind_de,bitb_ind_ded,bitb_ind_ix,bitb_ind_ixd,
	bitb_ind_rg,bitb_ind_rgd,bitb_ind_in,bitb_ind_ind,bitb_ind_de,bitb_ind_ded,bitb_ind_ix,bitb_ind_ixd,
	bitb_ind_rg,bitb_ind_rgd,bitb_ind_in,bitb_ind_ind,bitb_ind_de,bitb_ind_ded,bitb_ind_ix,bitb_ind_ixd,
	/* 0xb800 */
	bitb_de_rg,	bitb_de_rgd,bitb_de_in,	bitb_de_ind,bitb_de_de,	bitb_de_ded,bitb_de_ix,	bitb_de_ixd,
	bitb_de_rg,	bitb_de_rgd,bitb_de_in,	bitb_de_ind,bitb_de_de,	bitb_de_ded,bitb_de_ix,	bitb_de_ixd,
	bitb_de_rg,	bitb_de_rgd,bitb_de_in,	bitb_de_ind,bitb_de_de,	bitb_de_ded,bitb_de_ix,	bitb_de_ixd,
	bitb_de_rg,	bitb_de_rgd,bitb_de_in,	bitb_de_ind,bitb_de_de,	bitb_de_ded,bitb_de_ix,	bitb_de_ixd,
	/* 0xb900 */
	bitb_de_rg,	bitb_de_rgd,bitb_de_in,	bitb_de_ind,bitb_de_de,	bitb_de_ded,bitb_de_ix,	bitb_de_ixd,
	bitb_de_rg,	bitb_de_rgd,bitb_de_in,	bitb_de_ind,bitb_de_de,	bitb_de_ded,bitb_de_ix,	bitb_de_ixd,
	bitb_de_rg,	bitb_de_rgd,bitb_de_in,	bitb_de_ind,bitb_de_de,	bitb_de_ded,bitb_de_ix,	bitb_de_ixd,
	bitb_de_rg,	bitb_de_rgd,bitb_de_in,	bitb_de_ind,bitb_de_de,	bitb_de_ded,bitb_de_ix,	bitb_de_ixd,
	/* 0xba00 */
	bitb_ded_rg,bitb_ded_rgd,bitb_ded_in,bitb_ded_ind,bitb_ded_de,bitb_ded_ded,bitb_ded_ix,bitb_ded_ixd,
	bitb_ded_rg,bitb_ded_rgd,bitb_ded_in,bitb_ded_ind,bitb_ded_de,bitb_ded_ded,bitb_ded_ix,bitb_ded_ixd,
	bitb_ded_rg,bitb_ded_rgd,bitb_ded_in,bitb_ded_ind,bitb_ded_de,bitb_ded_ded,bitb_ded_ix,bitb_ded_ixd,
	bitb_ded_rg,bitb_ded_rgd,bitb_ded_in,bitb_ded_ind,bitb_ded_de,bitb_ded_ded,bitb_ded_ix,bitb_ded_ixd,
	/* 0xbb00 */
	bitb_ded_rg,bitb_ded_rgd,bitb_ded_in,bitb_ded_ind,bitb_ded_de,bitb_ded_ded,bitb_ded_ix,bitb_ded_ixd,
	bitb_ded_rg,bitb_ded_rgd,bitb_ded_in,bitb_ded_ind,bitb_ded_de,bitb_ded_ded,bitb_ded_ix,bitb_ded_ixd,
	bitb_ded_rg,bitb_ded_rgd,bitb_ded_in,bitb_ded_ind,bitb_ded_de,bitb_ded_ded,bitb_ded_ix,bitb_ded_ixd,
	bitb_ded_rg,bitb_ded_rgd,bitb_ded_in,bitb_ded_ind,bitb_ded_de,bitb_ded_ded,bitb_ded_ix,bitb_ded_ixd,
	/* 0xbc00 */
	bitb_ix_rg,	bitb_ix_rgd,bitb_ix_in,	bitb_ix_ind,bitb_ix_de,	bitb_ix_ded,bitb_ix_ix,	bitb_ix_ixd,
	bitb_ix_rg,	bitb_ix_rgd,bitb_ix_in,	bitb_ix_ind,bitb_ix_de,	bitb_ix_ded,bitb_ix_ix,	bitb_ix_ixd,
	bitb_ix_rg,	bitb_ix_rgd,bitb_ix_in,	bitb_ix_ind,bitb_ix_de,	bitb_ix_ded,bitb_ix_ix,	bitb_ix_ixd,
	bitb_ix_rg,	bitb_ix_rgd,bitb_ix_in,	bitb_ix_ind,bitb_ix_de,	bitb_ix_ded,bitb_ix_ix,	bitb_ix_ixd,
	/* 0xbd00 */
	bitb_ix_rg,	bitb_ix_rgd,bitb_ix_in,	bitb_ix_ind,bitb_ix_de,	bitb_ix_ded,bitb_ix_ix,	bitb_ix_ixd,
	bitb_ix_rg,	bitb_ix_rgd,bitb_ix_in,	bitb_ix_ind,bitb_ix_de,	bitb_ix_ded,bitb_ix_ix,	bitb_ix_ixd,
	bitb_ix_rg,	bitb_ix_rgd,bitb_ix_in,	bitb_ix_ind,bitb_ix_de,	bitb_ix_ded,bitb_ix_ix,	bitb_ix_ixd,
	bitb_ix_rg,	bitb_ix_rgd,bitb_ix_in,	bitb_ix_ind,bitb_ix_de,	bitb_ix_ded,bitb_ix_ix,	bitb_ix_ixd,
	/* 0xbe00 */
	bitb_ixd_rg,bitb_ixd_rgd,bitb_ixd_in,bitb_ixd_ind,bitb_ixd_de,bitb_ixd_ded,bitb_ixd_ix,bitb_ixd_ixd,
	bitb_ixd_rg,bitb_ixd_rgd,bitb_ixd_in,bitb_ixd_ind,bitb_ixd_de,bitb_ixd_ded,bitb_ixd_ix,bitb_ixd_ixd,
	bitb_ixd_rg,bitb_ixd_rgd,bitb_ixd_in,bitb_ixd_ind,bitb_ixd_de,bitb_ixd_ded,bitb_ixd_ix,bitb_ixd_ixd,
	bitb_ixd_rg,bitb_ixd_rgd,bitb_ixd_in,bitb_ixd_ind,bitb_ixd_de,bitb_ixd_ded,bitb_ixd_ix,bitb_ixd_ixd,
	/* 0xbf00 */
	bitb_ixd_rg,bitb_ixd_rgd,bitb_ixd_in,bitb_ixd_ind,bitb_ixd_de,bitb_ixd_ded,bitb_ixd_ix,bitb_ixd_ixd,
	bitb_ixd_rg,bitb_ixd_rgd,bitb_ixd_in,bitb_ixd_ind,bitb_ixd_de,bitb_ixd_ded,bitb_ixd_ix,bitb_ixd_ixd,
	bitb_ixd_rg,bitb_ixd_rgd,bitb_ixd_in,bitb_ixd_ind,bitb_ixd_de,bitb_ixd_ded,bitb_ixd_ix,bitb_ixd_ixd,
	bitb_ixd_rg,bitb_ixd_rgd,bitb_ixd_in,bitb_ixd_ind,bitb_ixd_de,bitb_ixd_ded,bitb_ixd_ix,bitb_ixd_ixd,

	/* 0xc000 */
	bicb_rg_rg,	bicb_rg_rgd,bicb_rg_in,	bicb_rg_ind,bicb_rg_de,	bicb_rg_ded,bicb_rg_ix,	bicb_rg_ixd,
	bicb_rg_rg,	bicb_rg_rgd,bicb_rg_in,	bicb_rg_ind,bicb_rg_de,	bicb_rg_ded,bicb_rg_ix,	bicb_rg_ixd,
	bicb_rg_rg,	bicb_rg_rgd,bicb_rg_in,	bicb_rg_ind,bicb_rg_de,	bicb_rg_ded,bicb_rg_ix,	bicb_rg_ixd,
	bicb_rg_rg,	bicb_rg_rgd,bicb_rg_in,	bicb_rg_ind,bicb_rg_de,	bicb_rg_ded,bicb_rg_ix,	bicb_rg_ixd,
	/* 0xc100 */
	bicb_rg_rg,	bicb_rg_rgd,bicb_rg_in,	bicb_rg_ind,bicb_rg_de,	bicb_rg_ded,bicb_rg_ix,	bicb_rg_ixd,
	bicb_rg_rg,	bicb_rg_rgd,bicb_rg_in,	bicb_rg_ind,bicb_rg_de,	bicb_rg_ded,bicb_rg_ix,	bicb_rg_ixd,
	bicb_rg_rg,	bicb_rg_rgd,bicb_rg_in,	bicb_rg_ind,bicb_rg_de,	bicb_rg_ded,bicb_rg_ix,	bicb_rg_ixd,
	bicb_rg_rg,	bicb_rg_rgd,bicb_rg_in,	bicb_rg_ind,bicb_rg_de,	bicb_rg_ded,bicb_rg_ix,	bicb_rg_ixd,
	/* 0xc200 */
	bicb_rgd_rg,bicb_rgd_rgd,bicb_rgd_in,bicb_rgd_ind,bicb_rgd_de,bicb_rgd_ded,bicb_rgd_ix,bicb_rgd_ixd,
	bicb_rgd_rg,bicb_rgd_rgd,bicb_rgd_in,bicb_rgd_ind,bicb_rgd_de,bicb_rgd_ded,bicb_rgd_ix,bicb_rgd_ixd,
	bicb_rgd_rg,bicb_rgd_rgd,bicb_rgd_in,bicb_rgd_ind,bicb_rgd_de,bicb_rgd_ded,bicb_rgd_ix,bicb_rgd_ixd,
	bicb_rgd_rg,bicb_rgd_rgd,bicb_rgd_in,bicb_rgd_ind,bicb_rgd_de,bicb_rgd_ded,bicb_rgd_ix,bicb_rgd_ixd,
	/* 0xc300 */
	bicb_rgd_rg,bicb_rgd_rgd,bicb_rgd_in,bicb_rgd_ind,bicb_rgd_de,bicb_rgd_ded,bicb_rgd_ix,bicb_rgd_ixd,
	bicb_rgd_rg,bicb_rgd_rgd,bicb_rgd_in,bicb_rgd_ind,bicb_rgd_de,bicb_rgd_ded,bicb_rgd_ix,bicb_rgd_ixd,
	bicb_rgd_rg,bicb_rgd_rgd,bicb_rgd_in,bicb_rgd_ind,bicb_rgd_de,bicb_rgd_ded,bicb_rgd_ix,bicb_rgd_ixd,
	bicb_rgd_rg,bicb_rgd_rgd,bicb_rgd_in,bicb_rgd_ind,bicb_rgd_de,bicb_rgd_ded,bicb_rgd_ix,bicb_rgd_ixd,
	/* 0xc400 */
	bicb_in_rg,	bicb_in_rgd,bicb_in_in,	bicb_in_ind,bicb_in_de,	bicb_in_ded,bicb_in_ix,	bicb_in_ixd,
	bicb_in_rg,	bicb_in_rgd,bicb_in_in,	bicb_in_ind,bicb_in_de,	bicb_in_ded,bicb_in_ix,	bicb_in_ixd,
	bicb_in_rg,	bicb_in_rgd,bicb_in_in,	bicb_in_ind,bicb_in_de,	bicb_in_ded,bicb_in_ix,	bicb_in_ixd,
	bicb_in_rg,	bicb_in_rgd,bicb_in_in,	bicb_in_ind,bicb_in_de,	bicb_in_ded,bicb_in_ix,	bicb_in_ixd,
	/* 0xc500 */
	bicb_in_rg,	bicb_in_rgd,bicb_in_in,	bicb_in_ind,bicb_in_de,	bicb_in_ded,bicb_in_ix,	bicb_in_ixd,
	bicb_in_rg,	bicb_in_rgd,bicb_in_in,	bicb_in_ind,bicb_in_de,	bicb_in_ded,bicb_in_ix,	bicb_in_ixd,
	bicb_in_rg,	bicb_in_rgd,bicb_in_in,	bicb_in_ind,bicb_in_de,	bicb_in_ded,bicb_in_ix,	bicb_in_ixd,
	bicb_in_rg,	bicb_in_rgd,bicb_in_in,	bicb_in_ind,bicb_in_de,	bicb_in_ded,bicb_in_ix,	bicb_in_ixd,
	/* 0xc600 */
	bicb_ind_rg,bicb_ind_rgd,bicb_ind_in,bicb_ind_ind,bicb_ind_de,bicb_ind_ded,bicb_ind_ix,bicb_ind_ixd,
	bicb_ind_rg,bicb_ind_rgd,bicb_ind_in,bicb_ind_ind,bicb_ind_de,bicb_ind_ded,bicb_ind_ix,bicb_ind_ixd,
	bicb_ind_rg,bicb_ind_rgd,bicb_ind_in,bicb_ind_ind,bicb_ind_de,bicb_ind_ded,bicb_ind_ix,bicb_ind_ixd,
	bicb_ind_rg,bicb_ind_rgd,bicb_ind_in,bicb_ind_ind,bicb_ind_de,bicb_ind_ded,bicb_ind_ix,bicb_ind_ixd,
	/* 0xc700 */
	bicb_ind_rg,bicb_ind_rgd,bicb_ind_in,bicb_ind_ind,bicb_ind_de,bicb_ind_ded,bicb_ind_ix,bicb_ind_ixd,
	bicb_ind_rg,bicb_ind_rgd,bicb_ind_in,bicb_ind_ind,bicb_ind_de,bicb_ind_ded,bicb_ind_ix,bicb_ind_ixd,
	bicb_ind_rg,bicb_ind_rgd,bicb_ind_in,bicb_ind_ind,bicb_ind_de,bicb_ind_ded,bicb_ind_ix,bicb_ind_ixd,
	bicb_ind_rg,bicb_ind_rgd,bicb_ind_in,bicb_ind_ind,bicb_ind_de,bicb_ind_ded,bicb_ind_ix,bicb_ind_ixd,
	/* 0xc800 */
	bicb_de_rg,	bicb_de_rgd,bicb_de_in,	bicb_de_ind,bicb_de_de,	bicb_de_ded,bicb_de_ix,	bicb_de_ixd,
	bicb_de_rg,	bicb_de_rgd,bicb_de_in,	bicb_de_ind,bicb_de_de,	bicb_de_ded,bicb_de_ix,	bicb_de_ixd,
	bicb_de_rg,	bicb_de_rgd,bicb_de_in,	bicb_de_ind,bicb_de_de,	bicb_de_ded,bicb_de_ix,	bicb_de_ixd,
	bicb_de_rg,	bicb_de_rgd,bicb_de_in,	bicb_de_ind,bicb_de_de,	bicb_de_ded,bicb_de_ix,	bicb_de_ixd,
	/* 0xc900 */
	bicb_de_rg,	bicb_de_rgd,bicb_de_in,	bicb_de_ind,bicb_de_de,	bicb_de_ded,bicb_de_ix,	bicb_de_ixd,
	bicb_de_rg,	bicb_de_rgd,bicb_de_in,	bicb_de_ind,bicb_de_de,	bicb_de_ded,bicb_de_ix,	bicb_de_ixd,
	bicb_de_rg,	bicb_de_rgd,bicb_de_in,	bicb_de_ind,bicb_de_de,	bicb_de_ded,bicb_de_ix,	bicb_de_ixd,
	bicb_de_rg,	bicb_de_rgd,bicb_de_in,	bicb_de_ind,bicb_de_de,	bicb_de_ded,bicb_de_ix,	bicb_de_ixd,
	/* 0xca00 */
	bicb_ded_rg,bicb_ded_rgd,bicb_ded_in,bicb_ded_ind,bicb_ded_de,bicb_ded_ded,bicb_ded_ix,bicb_ded_ixd,
	bicb_ded_rg,bicb_ded_rgd,bicb_ded_in,bicb_ded_ind,bicb_ded_de,bicb_ded_ded,bicb_ded_ix,bicb_ded_ixd,
	bicb_ded_rg,bicb_ded_rgd,bicb_ded_in,bicb_ded_ind,bicb_ded_de,bicb_ded_ded,bicb_ded_ix,bicb_ded_ixd,
	bicb_ded_rg,bicb_ded_rgd,bicb_ded_in,bicb_ded_ind,bicb_ded_de,bicb_ded_ded,bicb_ded_ix,bicb_ded_ixd,
	/* 0xcb00 */
	bicb_ded_rg,bicb_ded_rgd,bicb_ded_in,bicb_ded_ind,bicb_ded_de,bicb_ded_ded,bicb_ded_ix,bicb_ded_ixd,
	bicb_ded_rg,bicb_ded_rgd,bicb_ded_in,bicb_ded_ind,bicb_ded_de,bicb_ded_ded,bicb_ded_ix,bicb_ded_ixd,
	bicb_ded_rg,bicb_ded_rgd,bicb_ded_in,bicb_ded_ind,bicb_ded_de,bicb_ded_ded,bicb_ded_ix,bicb_ded_ixd,
	bicb_ded_rg,bicb_ded_rgd,bicb_ded_in,bicb_ded_ind,bicb_ded_de,bicb_ded_ded,bicb_ded_ix,bicb_ded_ixd,
	/* 0xcc00 */
	bicb_ix_rg,	bicb_ix_rgd,bicb_ix_in,	bicb_ix_ind,bicb_ix_de,	bicb_ix_ded,bicb_ix_ix,	bicb_ix_ixd,
	bicb_ix_rg,	bicb_ix_rgd,bicb_ix_in,	bicb_ix_ind,bicb_ix_de,	bicb_ix_ded,bicb_ix_ix,	bicb_ix_ixd,
	bicb_ix_rg,	bicb_ix_rgd,bicb_ix_in,	bicb_ix_ind,bicb_ix_de,	bicb_ix_ded,bicb_ix_ix,	bicb_ix_ixd,
	bicb_ix_rg,	bicb_ix_rgd,bicb_ix_in,	bicb_ix_ind,bicb_ix_de,	bicb_ix_ded,bicb_ix_ix,	bicb_ix_ixd,
	/* 0xcd00 */
	bicb_ix_rg,	bicb_ix_rgd,bicb_ix_in,	bicb_ix_ind,bicb_ix_de,	bicb_ix_ded,bicb_ix_ix,	bicb_ix_ixd,
	bicb_ix_rg,	bicb_ix_rgd,bicb_ix_in,	bicb_ix_ind,bicb_ix_de,	bicb_ix_ded,bicb_ix_ix,	bicb_ix_ixd,
	bicb_ix_rg,	bicb_ix_rgd,bicb_ix_in,	bicb_ix_ind,bicb_ix_de,	bicb_ix_ded,bicb_ix_ix,	bicb_ix_ixd,
	bicb_ix_rg,	bicb_ix_rgd,bicb_ix_in,	bicb_ix_ind,bicb_ix_de,	bicb_ix_ded,bicb_ix_ix,	bicb_ix_ixd,
	/* 0xce00 */
	bicb_ixd_rg,bicb_ixd_rgd,bicb_ixd_in,bicb_ixd_ind,bicb_ixd_de,bicb_ixd_ded,bicb_ixd_ix,bicb_ixd_ixd,
	bicb_ixd_rg,bicb_ixd_rgd,bicb_ixd_in,bicb_ixd_ind,bicb_ixd_de,bicb_ixd_ded,bicb_ixd_ix,bicb_ixd_ixd,
	bicb_ixd_rg,bicb_ixd_rgd,bicb_ixd_in,bicb_ixd_ind,bicb_ixd_de,bicb_ixd_ded,bicb_ixd_ix,bicb_ixd_ixd,
	bicb_ixd_rg,bicb_ixd_rgd,bicb_ixd_in,bicb_ixd_ind,bicb_ixd_de,bicb_ixd_ded,bicb_ixd_ix,bicb_ixd_ixd,
	/* 0xcf00 */
	bicb_ixd_rg,bicb_ixd_rgd,bicb_ixd_in,bicb_ixd_ind,bicb_ixd_de,bicb_ixd_ded,bicb_ixd_ix,bicb_ixd_ixd,
	bicb_ixd_rg,bicb_ixd_rgd,bicb_ixd_in,bicb_ixd_ind,bicb_ixd_de,bicb_ixd_ded,bicb_ixd_ix,bicb_ixd_ixd,
	bicb_ixd_rg,bicb_ixd_rgd,bicb_ixd_in,bicb_ixd_ind,bicb_ixd_de,bicb_ixd_ded,bicb_ixd_ix,bicb_ixd_ixd,
	bicb_ixd_rg,bicb_ixd_rgd,bicb_ixd_in,bicb_ixd_ind,bicb_ixd_de,bicb_ixd_ded,bicb_ixd_ix,bicb_ixd_ixd,

	/* 0xd000 */
	bisb_rg_rg,	bisb_rg_rgd,bisb_rg_in,	bisb_rg_ind,bisb_rg_de,	bisb_rg_ded,bisb_rg_ix,	bisb_rg_ixd,
	bisb_rg_rg,	bisb_rg_rgd,bisb_rg_in,	bisb_rg_ind,bisb_rg_de,	bisb_rg_ded,bisb_rg_ix,	bisb_rg_ixd,
	bisb_rg_rg,	bisb_rg_rgd,bisb_rg_in,	bisb_rg_ind,bisb_rg_de,	bisb_rg_ded,bisb_rg_ix,	bisb_rg_ixd,
	bisb_rg_rg,	bisb_rg_rgd,bisb_rg_in,	bisb_rg_ind,bisb_rg_de,	bisb_rg_ded,bisb_rg_ix,	bisb_rg_ixd,
	/* 0xd100 */
	bisb_rg_rg,	bisb_rg_rgd,bisb_rg_in,	bisb_rg_ind,bisb_rg_de,	bisb_rg_ded,bisb_rg_ix,	bisb_rg_ixd,
	bisb_rg_rg,	bisb_rg_rgd,bisb_rg_in,	bisb_rg_ind,bisb_rg_de,	bisb_rg_ded,bisb_rg_ix,	bisb_rg_ixd,
	bisb_rg_rg,	bisb_rg_rgd,bisb_rg_in,	bisb_rg_ind,bisb_rg_de,	bisb_rg_ded,bisb_rg_ix,	bisb_rg_ixd,
	bisb_rg_rg,	bisb_rg_rgd,bisb_rg_in,	bisb_rg_ind,bisb_rg_de,	bisb_rg_ded,bisb_rg_ix,	bisb_rg_ixd,
	/* 0xd200 */
	bisb_rgd_rg,bisb_rgd_rgd,bisb_rgd_in,bisb_rgd_ind,bisb_rgd_de,bisb_rgd_ded,bisb_rgd_ix,bisb_rgd_ixd,
	bisb_rgd_rg,bisb_rgd_rgd,bisb_rgd_in,bisb_rgd_ind,bisb_rgd_de,bisb_rgd_ded,bisb_rgd_ix,bisb_rgd_ixd,
	bisb_rgd_rg,bisb_rgd_rgd,bisb_rgd_in,bisb_rgd_ind,bisb_rgd_de,bisb_rgd_ded,bisb_rgd_ix,bisb_rgd_ixd,
	bisb_rgd_rg,bisb_rgd_rgd,bisb_rgd_in,bisb_rgd_ind,bisb_rgd_de,bisb_rgd_ded,bisb_rgd_ix,bisb_rgd_ixd,
	/* 0xd300 */
	bisb_rgd_rg,bisb_rgd_rgd,bisb_rgd_in,bisb_rgd_ind,bisb_rgd_de,bisb_rgd_ded,bisb_rgd_ix,bisb_rgd_ixd,
	bisb_rgd_rg,bisb_rgd_rgd,bisb_rgd_in,bisb_rgd_ind,bisb_rgd_de,bisb_rgd_ded,bisb_rgd_ix,bisb_rgd_ixd,
	bisb_rgd_rg,bisb_rgd_rgd,bisb_rgd_in,bisb_rgd_ind,bisb_rgd_de,bisb_rgd_ded,bisb_rgd_ix,bisb_rgd_ixd,
	bisb_rgd_rg,bisb_rgd_rgd,bisb_rgd_in,bisb_rgd_ind,bisb_rgd_de,bisb_rgd_ded,bisb_rgd_ix,bisb_rgd_ixd,
	/* 0xd400 */
	bisb_in_rg,	bisb_in_rgd,bisb_in_in,	bisb_in_ind,bisb_in_de,	bisb_in_ded,bisb_in_ix,	bisb_in_ixd,
	bisb_in_rg,	bisb_in_rgd,bisb_in_in,	bisb_in_ind,bisb_in_de,	bisb_in_ded,bisb_in_ix,	bisb_in_ixd,
	bisb_in_rg,	bisb_in_rgd,bisb_in_in,	bisb_in_ind,bisb_in_de,	bisb_in_ded,bisb_in_ix,	bisb_in_ixd,
	bisb_in_rg,	bisb_in_rgd,bisb_in_in,	bisb_in_ind,bisb_in_de,	bisb_in_ded,bisb_in_ix,	bisb_in_ixd,
	/* 0xd500 */
	bisb_in_rg,	bisb_in_rgd,bisb_in_in,	bisb_in_ind,bisb_in_de,	bisb_in_ded,bisb_in_ix,	bisb_in_ixd,
	bisb_in_rg,	bisb_in_rgd,bisb_in_in,	bisb_in_ind,bisb_in_de,	bisb_in_ded,bisb_in_ix,	bisb_in_ixd,
	bisb_in_rg,	bisb_in_rgd,bisb_in_in,	bisb_in_ind,bisb_in_de,	bisb_in_ded,bisb_in_ix,	bisb_in_ixd,
	bisb_in_rg,	bisb_in_rgd,bisb_in_in,	bisb_in_ind,bisb_in_de,	bisb_in_ded,bisb_in_ix,	bisb_in_ixd,
	/* 0xd600 */
	bisb_ind_rg,bisb_ind_rgd,bisb_ind_in,bisb_ind_ind,bisb_ind_de,bisb_ind_ded,bisb_ind_ix,bisb_ind_ixd,
	bisb_ind_rg,bisb_ind_rgd,bisb_ind_in,bisb_ind_ind,bisb_ind_de,bisb_ind_ded,bisb_ind_ix,bisb_ind_ixd,
	bisb_ind_rg,bisb_ind_rgd,bisb_ind_in,bisb_ind_ind,bisb_ind_de,bisb_ind_ded,bisb_ind_ix,bisb_ind_ixd,
	bisb_ind_rg,bisb_ind_rgd,bisb_ind_in,bisb_ind_ind,bisb_ind_de,bisb_ind_ded,bisb_ind_ix,bisb_ind_ixd,
	/* 0xd700 */
	bisb_ind_rg,bisb_ind_rgd,bisb_ind_in,bisb_ind_ind,bisb_ind_de,bisb_ind_ded,bisb_ind_ix,bisb_ind_ixd,
	bisb_ind_rg,bisb_ind_rgd,bisb_ind_in,bisb_ind_ind,bisb_ind_de,bisb_ind_ded,bisb_ind_ix,bisb_ind_ixd,
	bisb_ind_rg,bisb_ind_rgd,bisb_ind_in,bisb_ind_ind,bisb_ind_de,bisb_ind_ded,bisb_ind_ix,bisb_ind_ixd,
	bisb_ind_rg,bisb_ind_rgd,bisb_ind_in,bisb_ind_ind,bisb_ind_de,bisb_ind_ded,bisb_ind_ix,bisb_ind_ixd,
	/* 0xd800 */
	bisb_de_rg,	bisb_de_rgd,bisb_de_in,	bisb_de_ind,bisb_de_de,	bisb_de_ded,bisb_de_ix,	bisb_de_ixd,
	bisb_de_rg,	bisb_de_rgd,bisb_de_in,	bisb_de_ind,bisb_de_de,	bisb_de_ded,bisb_de_ix,	bisb_de_ixd,
	bisb_de_rg,	bisb_de_rgd,bisb_de_in,	bisb_de_ind,bisb_de_de,	bisb_de_ded,bisb_de_ix,	bisb_de_ixd,
	bisb_de_rg,	bisb_de_rgd,bisb_de_in,	bisb_de_ind,bisb_de_de,	bisb_de_ded,bisb_de_ix,	bisb_de_ixd,
	/* 0xd900 */
	bisb_de_rg,	bisb_de_rgd,bisb_de_in,	bisb_de_ind,bisb_de_de,	bisb_de_ded,bisb_de_ix,	bisb_de_ixd,
	bisb_de_rg,	bisb_de_rgd,bisb_de_in,	bisb_de_ind,bisb_de_de,	bisb_de_ded,bisb_de_ix,	bisb_de_ixd,
	bisb_de_rg,	bisb_de_rgd,bisb_de_in,	bisb_de_ind,bisb_de_de,	bisb_de_ded,bisb_de_ix,	bisb_de_ixd,
	bisb_de_rg,	bisb_de_rgd,bisb_de_in,	bisb_de_ind,bisb_de_de,	bisb_de_ded,bisb_de_ix,	bisb_de_ixd,
	/* 0xda00 */
	bisb_ded_rg,bisb_ded_rgd,bisb_ded_in,bisb_ded_ind,bisb_ded_de,bisb_ded_ded,bisb_ded_ix,bisb_ded_ixd,
	bisb_ded_rg,bisb_ded_rgd,bisb_ded_in,bisb_ded_ind,bisb_ded_de,bisb_ded_ded,bisb_ded_ix,bisb_ded_ixd,
	bisb_ded_rg,bisb_ded_rgd,bisb_ded_in,bisb_ded_ind,bisb_ded_de,bisb_ded_ded,bisb_ded_ix,bisb_ded_ixd,
	bisb_ded_rg,bisb_ded_rgd,bisb_ded_in,bisb_ded_ind,bisb_ded_de,bisb_ded_ded,bisb_ded_ix,bisb_ded_ixd,
	/* 0xdb00 */
	bisb_ded_rg,bisb_ded_rgd,bisb_ded_in,bisb_ded_ind,bisb_ded_de,bisb_ded_ded,bisb_ded_ix,bisb_ded_ixd,
	bisb_ded_rg,bisb_ded_rgd,bisb_ded_in,bisb_ded_ind,bisb_ded_de,bisb_ded_ded,bisb_ded_ix,bisb_ded_ixd,
	bisb_ded_rg,bisb_ded_rgd,bisb_ded_in,bisb_ded_ind,bisb_ded_de,bisb_ded_ded,bisb_ded_ix,bisb_ded_ixd,
	bisb_ded_rg,bisb_ded_rgd,bisb_ded_in,bisb_ded_ind,bisb_ded_de,bisb_ded_ded,bisb_ded_ix,bisb_ded_ixd,
	/* 0xdc00 */
	bisb_ix_rg,	bisb_ix_rgd,bisb_ix_in,	bisb_ix_ind,bisb_ix_de,	bisb_ix_ded,bisb_ix_ix,	bisb_ix_ixd,
	bisb_ix_rg,	bisb_ix_rgd,bisb_ix_in,	bisb_ix_ind,bisb_ix_de,	bisb_ix_ded,bisb_ix_ix,	bisb_ix_ixd,
	bisb_ix_rg,	bisb_ix_rgd,bisb_ix_in,	bisb_ix_ind,bisb_ix_de,	bisb_ix_ded,bisb_ix_ix,	bisb_ix_ixd,
	bisb_ix_rg,	bisb_ix_rgd,bisb_ix_in,	bisb_ix_ind,bisb_ix_de,	bisb_ix_ded,bisb_ix_ix,	bisb_ix_ixd,
	/* 0xdd00 */
	bisb_ix_rg,	bisb_ix_rgd,bisb_ix_in,	bisb_ix_ind,bisb_ix_de,	bisb_ix_ded,bisb_ix_ix,	bisb_ix_ixd,
	bisb_ix_rg,	bisb_ix_rgd,bisb_ix_in,	bisb_ix_ind,bisb_ix_de,	bisb_ix_ded,bisb_ix_ix,	bisb_ix_ixd,
	bisb_ix_rg,	bisb_ix_rgd,bisb_ix_in,	bisb_ix_ind,bisb_ix_de,	bisb_ix_ded,bisb_ix_ix,	bisb_ix_ixd,
	bisb_ix_rg,	bisb_ix_rgd,bisb_ix_in,	bisb_ix_ind,bisb_ix_de,	bisb_ix_ded,bisb_ix_ix,	bisb_ix_ixd,
	/* 0xde00 */
	bisb_ixd_rg,bisb_ixd_rgd,bisb_ixd_in,bisb_ixd_ind,bisb_ixd_de,bisb_ixd_ded,bisb_ixd_ix,bisb_ixd_ixd,
	bisb_ixd_rg,bisb_ixd_rgd,bisb_ixd_in,bisb_ixd_ind,bisb_ixd_de,bisb_ixd_ded,bisb_ixd_ix,bisb_ixd_ixd,
	bisb_ixd_rg,bisb_ixd_rgd,bisb_ixd_in,bisb_ixd_ind,bisb_ixd_de,bisb_ixd_ded,bisb_ixd_ix,bisb_ixd_ixd,
	bisb_ixd_rg,bisb_ixd_rgd,bisb_ixd_in,bisb_ixd_ind,bisb_ixd_de,bisb_ixd_ded,bisb_ixd_ix,bisb_ixd_ixd,
	/* 0xdf00 */
	bisb_ixd_rg,bisb_ixd_rgd,bisb_ixd_in,bisb_ixd_ind,bisb_ixd_de,bisb_ixd_ded,bisb_ixd_ix,bisb_ixd_ixd,
	bisb_ixd_rg,bisb_ixd_rgd,bisb_ixd_in,bisb_ixd_ind,bisb_ixd_de,bisb_ixd_ded,bisb_ixd_ix,bisb_ixd_ixd,
	bisb_ixd_rg,bisb_ixd_rgd,bisb_ixd_in,bisb_ixd_ind,bisb_ixd_de,bisb_ixd_ded,bisb_ixd_ix,bisb_ixd_ixd,
	bisb_ixd_rg,bisb_ixd_rgd,bisb_ixd_in,bisb_ixd_ind,bisb_ixd_de,bisb_ixd_ded,bisb_ixd_ix,bisb_ixd_ixd,

	/* 0xe000 */
	sub_rg_rg,	sub_rg_rgd,	sub_rg_in,	sub_rg_ind,	sub_rg_de,	sub_rg_ded,	sub_rg_ix,	sub_rg_ixd,
	sub_rg_rg,	sub_rg_rgd,	sub_rg_in,	sub_rg_ind,	sub_rg_de,	sub_rg_ded,	sub_rg_ix,	sub_rg_ixd,
	sub_rg_rg,	sub_rg_rgd,	sub_rg_in,	sub_rg_ind,	sub_rg_de,	sub_rg_ded,	sub_rg_ix,	sub_rg_ixd,
	sub_rg_rg,	sub_rg_rgd,	sub_rg_in,	sub_rg_ind,	sub_rg_de,	sub_rg_ded,	sub_rg_ix,	sub_rg_ixd,
	/* 0xe100 */
	sub_rg_rg,	sub_rg_rgd,	sub_rg_in,	sub_rg_ind,	sub_rg_de,	sub_rg_ded,	sub_rg_ix,	sub_rg_ixd,
	sub_rg_rg,	sub_rg_rgd,	sub_rg_in,	sub_rg_ind,	sub_rg_de,	sub_rg_ded,	sub_rg_ix,	sub_rg_ixd,
	sub_rg_rg,	sub_rg_rgd,	sub_rg_in,	sub_rg_ind,	sub_rg_de,	sub_rg_ded,	sub_rg_ix,	sub_rg_ixd,
	sub_rg_rg,	sub_rg_rgd,	sub_rg_in,	sub_rg_ind,	sub_rg_de,	sub_rg_ded,	sub_rg_ix,	sub_rg_ixd,
	/* 0xe200 */
	sub_rgd_rg,	sub_rgd_rgd,sub_rgd_in,	sub_rgd_ind,sub_rgd_de,	sub_rgd_ded,sub_rgd_ix,	sub_rgd_ixd,
	sub_rgd_rg,	sub_rgd_rgd,sub_rgd_in,	sub_rgd_ind,sub_rgd_de,	sub_rgd_ded,sub_rgd_ix,	sub_rgd_ixd,
	sub_rgd_rg,	sub_rgd_rgd,sub_rgd_in,	sub_rgd_ind,sub_rgd_de,	sub_rgd_ded,sub_rgd_ix,	sub_rgd_ixd,
	sub_rgd_rg,	sub_rgd_rgd,sub_rgd_in,	sub_rgd_ind,sub_rgd_de,	sub_rgd_ded,sub_rgd_ix,	sub_rgd_ixd,
	/* 0xe300 */
	sub_rgd_rg,	sub_rgd_rgd,sub_rgd_in,	sub_rgd_ind,sub_rgd_de,	sub_rgd_ded,sub_rgd_ix,	sub_rgd_ixd,
	sub_rgd_rg,	sub_rgd_rgd,sub_rgd_in,	sub_rgd_ind,sub_rgd_de,	sub_rgd_ded,sub_rgd_ix,	sub_rgd_ixd,
	sub_rgd_rg,	sub_rgd_rgd,sub_rgd_in,	sub_rgd_ind,sub_rgd_de,	sub_rgd_ded,sub_rgd_ix,	sub_rgd_ixd,
	sub_rgd_rg,	sub_rgd_rgd,sub_rgd_in,	sub_rgd_ind,sub_rgd_de,	sub_rgd_ded,sub_rgd_ix,	sub_rgd_ixd,
	/* 0xe400 */
	sub_in_rg,	sub_in_rgd,	sub_in_in,	sub_in_ind,	sub_in_de,	sub_in_ded,	sub_in_ix,	sub_in_ixd,
	sub_in_rg,	sub_in_rgd,	sub_in_in,	sub_in_ind,	sub_in_de,	sub_in_ded,	sub_in_ix,	sub_in_ixd,
	sub_in_rg,	sub_in_rgd,	sub_in_in,	sub_in_ind,	sub_in_de,	sub_in_ded,	sub_in_ix,	sub_in_ixd,
	sub_in_rg,	sub_in_rgd,	sub_in_in,	sub_in_ind,	sub_in_de,	sub_in_ded,	sub_in_ix,	sub_in_ixd,
	/* 0xe500 */
	sub_in_rg,	sub_in_rgd,	sub_in_in,	sub_in_ind,	sub_in_de,	sub_in_ded,	sub_in_ix,	sub_in_ixd,
	sub_in_rg,	sub_in_rgd,	sub_in_in,	sub_in_ind,	sub_in_de,	sub_in_ded,	sub_in_ix,	sub_in_ixd,
	sub_in_rg,	sub_in_rgd,	sub_in_in,	sub_in_ind,	sub_in_de,	sub_in_ded,	sub_in_ix,	sub_in_ixd,
	sub_in_rg,	sub_in_rgd,	sub_in_in,	sub_in_ind,	sub_in_de,	sub_in_ded,	sub_in_ix,	sub_in_ixd,
	/* 0xe600 */
	sub_ind_rg,	sub_ind_rgd,sub_ind_in,	sub_ind_ind,sub_ind_de,	sub_ind_ded,sub_ind_ix,	sub_ind_ixd,
	sub_ind_rg,	sub_ind_rgd,sub_ind_in,	sub_ind_ind,sub_ind_de,	sub_ind_ded,sub_ind_ix,	sub_ind_ixd,
	sub_ind_rg,	sub_ind_rgd,sub_ind_in,	sub_ind_ind,sub_ind_de,	sub_ind_ded,sub_ind_ix,	sub_ind_ixd,
	sub_ind_rg,	sub_ind_rgd,sub_ind_in,	sub_ind_ind,sub_ind_de,	sub_ind_ded,sub_ind_ix,	sub_ind_ixd,
	/* 0xe700 */
	sub_ind_rg,	sub_ind_rgd,sub_ind_in,	sub_ind_ind,sub_ind_de,	sub_ind_ded,sub_ind_ix,	sub_ind_ixd,
	sub_ind_rg,	sub_ind_rgd,sub_ind_in,	sub_ind_ind,sub_ind_de,	sub_ind_ded,sub_ind_ix,	sub_ind_ixd,
	sub_ind_rg,	sub_ind_rgd,sub_ind_in,	sub_ind_ind,sub_ind_de,	sub_ind_ded,sub_ind_ix,	sub_ind_ixd,
	sub_ind_rg,	sub_ind_rgd,sub_ind_in,	sub_ind_ind,sub_ind_de,	sub_ind_ded,sub_ind_ix,	sub_ind_ixd,
	/* 0xe800 */
	sub_de_rg,	sub_de_rgd,	sub_de_in,	sub_de_ind,	sub_de_de,	sub_de_ded,	sub_de_ix,	sub_de_ixd,
	sub_de_rg,	sub_de_rgd,	sub_de_in,	sub_de_ind,	sub_de_de,	sub_de_ded,	sub_de_ix,	sub_de_ixd,
	sub_de_rg,	sub_de_rgd,	sub_de_in,	sub_de_ind,	sub_de_de,	sub_de_ded,	sub_de_ix,	sub_de_ixd,
	sub_de_rg,	sub_de_rgd,	sub_de_in,	sub_de_ind,	sub_de_de,	sub_de_ded,	sub_de_ix,	sub_de_ixd,
	/* 0xe900 */
	sub_de_rg,	sub_de_rgd,	sub_de_in,	sub_de_ind,	sub_de_de,	sub_de_ded,	sub_de_ix,	sub_de_ixd,
	sub_de_rg,	sub_de_rgd,	sub_de_in,	sub_de_ind,	sub_de_de,	sub_de_ded,	sub_de_ix,	sub_de_ixd,
	sub_de_rg,	sub_de_rgd,	sub_de_in,	sub_de_ind,	sub_de_de,	sub_de_ded,	sub_de_ix,	sub_de_ixd,
	sub_de_rg,	sub_de_rgd,	sub_de_in,	sub_de_ind,	sub_de_de,	sub_de_ded,	sub_de_ix,	sub_de_ixd,
	/* 0xea00 */
	sub_ded_rg,	sub_ded_rgd,sub_ded_in,	sub_ded_ind,sub_ded_de,	sub_ded_ded,sub_ded_ix,	sub_ded_ixd,
	sub_ded_rg,	sub_ded_rgd,sub_ded_in,	sub_ded_ind,sub_ded_de,	sub_ded_ded,sub_ded_ix,	sub_ded_ixd,
	sub_ded_rg,	sub_ded_rgd,sub_ded_in,	sub_ded_ind,sub_ded_de,	sub_ded_ded,sub_ded_ix,	sub_ded_ixd,
	sub_ded_rg,	sub_ded_rgd,sub_ded_in,	sub_ded_ind,sub_ded_de,	sub_ded_ded,sub_ded_ix,	sub_ded_ixd,
	/* 0xeb00 */
	sub_ded_rg,	sub_ded_rgd,sub_ded_in,	sub_ded_ind,sub_ded_de,	sub_ded_ded,sub_ded_ix,	sub_ded_ixd,
	sub_ded_rg,	sub_ded_rgd,sub_ded_in,	sub_ded_ind,sub_ded_de,	sub_ded_ded,sub_ded_ix,	sub_ded_ixd,
	sub_ded_rg,	sub_ded_rgd,sub_ded_in,	sub_ded_ind,sub_ded_de,	sub_ded_ded,sub_ded_ix,	sub_ded_ixd,
	sub_ded_rg,	sub_ded_rgd,sub_ded_in,	sub_ded_ind,sub_ded_de,	sub_ded_ded,sub_ded_ix,	sub_ded_ixd,
	/* 0xec00 */
	sub_ix_rg,	sub_ix_rgd,	sub_ix_in,	sub_ix_ind,	sub_ix_de,	sub_ix_ded,	sub_ix_ix,	sub_ix_ixd,
	sub_ix_rg,	sub_ix_rgd,	sub_ix_in,	sub_ix_ind,	sub_ix_de,	sub_ix_ded,	sub_ix_ix,	sub_ix_ixd,
	sub_ix_rg,	sub_ix_rgd,	sub_ix_in,	sub_ix_ind,	sub_ix_de,	sub_ix_ded,	sub_ix_ix,	sub_ix_ixd,
	sub_ix_rg,	sub_ix_rgd,	sub_ix_in,	sub_ix_ind,	sub_ix_de,	sub_ix_ded,	sub_ix_ix,	sub_ix_ixd,
	/* 0xed00 */
	sub_ix_rg,	sub_ix_rgd,	sub_ix_in,	sub_ix_ind,	sub_ix_de,	sub_ix_ded,	sub_ix_ix,	sub_ix_ixd,
	sub_ix_rg,	sub_ix_rgd,	sub_ix_in,	sub_ix_ind,	sub_ix_de,	sub_ix_ded,	sub_ix_ix,	sub_ix_ixd,
	sub_ix_rg,	sub_ix_rgd,	sub_ix_in,	sub_ix_ind,	sub_ix_de,	sub_ix_ded,	sub_ix_ix,	sub_ix_ixd,
	sub_ix_rg,	sub_ix_rgd,	sub_ix_in,	sub_ix_ind,	sub_ix_de,	sub_ix_ded,	sub_ix_ix,	sub_ix_ixd,
	/* 0xee00 */
	sub_ixd_rg,	sub_ixd_rgd,sub_ixd_in,	sub_ixd_ind,sub_ixd_de,	sub_ixd_ded,sub_ixd_ix,	sub_ixd_ixd,
	sub_ixd_rg,	sub_ixd_rgd,sub_ixd_in,	sub_ixd_ind,sub_ixd_de,	sub_ixd_ded,sub_ixd_ix,	sub_ixd_ixd,
	sub_ixd_rg,	sub_ixd_rgd,sub_ixd_in,	sub_ixd_ind,sub_ixd_de,	sub_ixd_ded,sub_ixd_ix,	sub_ixd_ixd,
	sub_ixd_rg,	sub_ixd_rgd,sub_ixd_in,	sub_ixd_ind,sub_ixd_de,	sub_ixd_ded,sub_ixd_ix,	sub_ixd_ixd,
	/* 0xef00 */
	sub_ixd_rg,	sub_ixd_rgd,sub_ixd_in,	sub_ixd_ind,sub_ixd_de,	sub_ixd_ded,sub_ixd_ix,	sub_ixd_ixd,
	sub_ixd_rg,	sub_ixd_rgd,sub_ixd_in,	sub_ixd_ind,sub_ixd_de,	sub_ixd_ded,sub_ixd_ix,	sub_ixd_ixd,
	sub_ixd_rg,	sub_ixd_rgd,sub_ixd_in,	sub_ixd_ind,sub_ixd_de,	sub_ixd_ded,sub_ixd_ix,	sub_ixd_ixd,
	sub_ixd_rg,	sub_ixd_rgd,sub_ixd_in,	sub_ixd_ind,sub_ixd_de,	sub_ixd_ded,sub_ixd_ix,	sub_ixd_ixd,

	/* 0xf000 */
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	/* 0xf100 */
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	/* 0xf200 */
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	/* 0xf300 */
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	/* 0xf400 */
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	/* 0xf500 */
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	/* 0xf600 */
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	/* 0xf700 */
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	/* 0xf800 */
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	/* 0xf900 */
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	/* 0xfa00 */
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	/* 0xfb00 */
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	/* 0xfc00 */
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	/* 0xfd00 */
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	/* 0xfe00 */
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	/* 0xff00 */
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal
};
