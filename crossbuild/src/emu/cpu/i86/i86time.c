/****************************************************************************
*             real mode i286 emulator v1.4 by Fabrice Frances               *
*               (initial work based on David Hedley's pcemu)                *
****************************************************************************/

struct i80x86_timing
{
	int		id;

	UINT8	exception, iret;								/* exception, IRET */
	UINT8	int3, int_imm, into_nt, into_t;					/* INTs */
	UINT8	override;										/* segment overrides */
	UINT8	flag_ops, lahf, sahf;							/* flag operations */
	UINT8	aaa, aas, aam, aad;								/* arithmetic adjusts */
	UINT8	daa, das;										/* decimal adjusts */
	UINT8	cbw, cwd;										/* sign extension */
	UINT8	hlt, load_ptr, lea, nop, wait, xlat;			/* misc */

	UINT8	jmp_short, jmp_near, jmp_far;					/* direct JMPs */
	UINT8	jmp_r16, jmp_m16, jmp_m32;						/* indirect JMPs */
	UINT8	call_near, call_far;							/* direct CALLs */
	UINT8	call_r16, call_m16, call_m32;					/* indirect CALLs */
	UINT8	ret_near, ret_far, ret_near_imm, ret_far_imm;	/* returns */
	UINT8	jcc_nt, jcc_t, jcxz_nt, jcxz_t;					/* conditional JMPs */
	UINT8	loop_nt, loop_t, loope_nt, loope_t;				/* loops */

	UINT8	in_imm8, in_imm16, in_dx8, in_dx16;				/* port reads */
	UINT8	out_imm8, out_imm16, out_dx8, out_dx16;			/* port writes */

	UINT8	mov_rr8, mov_rm8, mov_mr8;						/* move, 8-bit */
	UINT8	mov_ri8, mov_mi8;								/* move, 8-bit immediate */
	UINT8	mov_rr16, mov_rm16, mov_mr16;					/* move, 16-bit */
	UINT8	mov_ri16, mov_mi16;								/* move, 16-bit immediate */
	UINT8	mov_am8, mov_am16, mov_ma8, mov_ma16;			/* move, AL/AX memory */
	UINT8	mov_sr, mov_sm, mov_rs, mov_ms;					/* move, segment registers */
	UINT8	xchg_rr8, xchg_rm8;								/* exchange, 8-bit */
	UINT8	xchg_rr16, xchg_rm16, xchg_ar16;				/* exchange, 16-bit */

	UINT8	push_r16, push_m16, push_seg, pushf;			/* pushes */
	UINT8	pop_r16, pop_m16, pop_seg, popf;				/* pops */

	UINT8	alu_rr8, alu_rm8, alu_mr8;						/* ALU ops, 8-bit */
	UINT8	alu_ri8, alu_mi8, alu_mi8_ro;					/* ALU ops, 8-bit immediate */
	UINT8	alu_rr16, alu_rm16, alu_mr16;					/* ALU ops, 16-bit */
	UINT8	alu_ri16, alu_mi16, alu_mi16_ro;				/* ALU ops, 16-bit immediate */
	UINT8	alu_r16i8, alu_m16i8, alu_m16i8_ro;				/* ALU ops, 16-bit w/8-bit immediate */
	UINT8	mul_r8, mul_r16, mul_m8, mul_m16;				/* MUL */
	UINT8	imul_r8, imul_r16, imul_m8, imul_m16;			/* IMUL */
	UINT8	div_r8, div_r16, div_m8, div_m16;				/* DIV */
	UINT8	idiv_r8, idiv_r16, idiv_m8, idiv_m16;			/* IDIV */
	UINT8	incdec_r8, incdec_r16, incdec_m8, incdec_m16;	/* INC/DEC */
	UINT8	negnot_r8, negnot_r16, negnot_m8, negnot_m16;	/* NEG/NOT */

	UINT8	rot_reg_1, rot_reg_base, rot_reg_bit;			/* reg shift/rotate */
	UINT8	rot_m8_1, rot_m8_base, rot_m8_bit;				/* m8 shift/rotate */
	UINT8	rot_m16_1, rot_m16_base, rot_m16_bit;			/* m16 shift/rotate */

	UINT8	cmps8, rep_cmps8_base, rep_cmps8_count;			/* CMPS 8-bit */
	UINT8	cmps16, rep_cmps16_base, rep_cmps16_count;		/* CMPS 16-bit */
	UINT8	scas8, rep_scas8_base, rep_scas8_count;			/* SCAS 8-bit */
	UINT8	scas16, rep_scas16_base, rep_scas16_count;		/* SCAS 16-bit */
	UINT8	lods8, rep_lods8_base, rep_lods8_count;			/* LODS 8-bit */
	UINT8	lods16, rep_lods16_base, rep_lods16_count;		/* LODS 16-bit */
	UINT8	stos8, rep_stos8_base, rep_stos8_count;			/* STOS 8-bit */
	UINT8	stos16, rep_stos16_base, rep_stos16_count;		/* STOS 16-bit */
	UINT8	movs8, rep_movs8_base, rep_movs8_count;			/* MOVS 8-bit */
	UINT8	movs16, rep_movs16_base, rep_movs16_count;		/* MOVS 16-bit */

	void *	check1;											/* marker to make sure we line up */

	UINT8	ins8, rep_ins8_base, rep_ins8_count;			/* (80186) INS 8-bit */
	UINT8	ins16, rep_ins16_base, rep_ins16_count;			/* (80186) INS 16-bit */
	UINT8	outs8, rep_outs8_base, rep_outs8_count;			/* (80186) OUTS 8-bit */
	UINT8	outs16, rep_outs16_base, rep_outs16_count;		/* (80186) OUTS 16-bit */
	UINT8	push_imm, pusha, popa;							/* (80186) PUSH immediate, PUSHA/POPA */
	UINT8	imul_rri8, imul_rmi8;							/* (80186) IMUL immediate 8-bit */
	UINT8	imul_rri16, imul_rmi16;							/* (80186) IMUL immediate 16-bit */
	UINT8	enter0, enter1, enter_base, enter_count, leave;	/* (80186) ENTER/LEAVE */
	UINT8	bound;											/* (80186) BOUND */

	void *	check2;											/* marker to make sure we line up */
};


/* these come from the 8088 timings in OPCODE.LST, but with the
   penalty for 16-bit memory accesses removed wherever possible */
static const struct i80x86_timing i8086_cycles =
{
	8086,

	51,32,			/* exception, IRET */
	 2, 0, 4, 2,	/* INTs */
	 2,				/* segment overrides */
	 2, 4, 4,		/* flag operations */
	 4, 4,83,60,	/* arithmetic adjusts */
	 4, 4,			/* decimal adjusts */
	 2, 5,			/* sign extension */
	 2,24, 2, 2, 3,11,	/* misc */

	15,15,15,		/* direct JMPs */
	11,18,24,		/* indirect JMPs */
	19,28,			/* direct CALLs */
	16,21,37,		/* indirect CALLs */
	20,32,24,31,	/* returns */
	 4,16, 6,18,	/* conditional JMPs */
	 5,17, 6,18,	/* loops */

	10,14, 8,12,	/* port reads */
	10,14, 8,12,	/* port writes */

	 2, 8, 9,		/* move, 8-bit */
	 4,10,			/* move, 8-bit immediate */
	 2, 8, 9,		/* move, 16-bit */
	 4,10,			/* move, 16-bit immediate */
	10,10,10,10,	/* move, AL/AX memory */
	 2, 8, 2, 9,	/* move, segment registers */
	 4,17,			/* exchange, 8-bit */
	 4,17, 3,		/* exchange, 16-bit */

	15,24,14,14,	/* pushes */
	12,25,12,12,	/* pops */

	 3, 9,16,		/* ALU ops, 8-bit */
	 4,17,10,		/* ALU ops, 8-bit immediate */
	 3, 9,16,		/* ALU ops, 16-bit */
	 4,17,10,		/* ALU ops, 16-bit immediate */
	 4,17,10,		/* ALU ops, 16-bit w/8-bit immediate */
	70,118,76,128,	/* MUL */
	80,128,86,138,	/* IMUL */
	80,144,86,154,	/* DIV */
	101,165,107,175,/* IDIV */
	 3, 2,15,15,	/* INC/DEC */
	 3, 3,16,16,	/* NEG/NOT */

	 2, 8, 4,		/* reg shift/rotate */
	15,20, 4,		/* m8 shift/rotate */
	15,20, 4,		/* m16 shift/rotate */

	22, 9,21,		/* CMPS 8-bit */
	22, 9,21,		/* CMPS 16-bit */
	15, 9,14,		/* SCAS 8-bit */
	15, 9,14,		/* SCAS 16-bit */
	12, 9,11,		/* LODS 8-bit */
	12, 9,11,		/* LODS 16-bit */
	11, 9,10,		/* STOS 8-bit */
	11, 9,10,		/* STOS 16-bit */
	18, 9,17,		/* MOVS 8-bit */
	18, 9,17,		/* MOVS 16-bit */

	(void *)-1		/* marker to make sure we line up */
};


/* these come from the Intel 80186 datasheet */
static const struct i80x86_timing i80186_cycles =
{
	80186,

	45,28,			/* exception, IRET */
	 0, 2, 4, 3,	/* INTs */
	 2,				/* segment overrides */
	 2, 2, 3,		/* flag operations */
	 8, 7,19,15,	/* arithmetic adjusts */
	 4, 4,			/* decimal adjusts */
	 2, 4,			/* sign extension */
	 2,18, 6, 2, 6,11,	/* misc */

	14,14,14,		/* direct JMPs */
	11,17,26,		/* indirect JMPs */
	15,23,			/* direct CALLs */
	13,19,38,		/* indirect CALLs */
	16,22,18,25,	/* returns */
	 4,13, 5,15,	/* conditional JMPs */
	 6,16, 6,16,	/* loops */

	10,10, 8, 8,	/* port reads */
	 9, 9, 7, 7,	/* port writes */

	 2, 9,12,		/* move, 8-bit */
	 3,12,			/* move, 8-bit immediate */
	 2, 9,12,		/* move, 16-bit */
	 4,13,			/* move, 16-bit immediate */
	 8, 8, 9, 9,	/* move, AL/AX memory */
	 2,11, 2,11,	/* move, segment registers */
	 4,17,			/* exchange, 8-bit */
	 4,17, 3,		/* exchange, 16-bit */

	10,16, 9, 9,	/* pushes */
	10,20, 8, 8,	/* pops */

	 3,10,10,		/* ALU ops, 8-bit */
	 4,16,10,		/* ALU ops, 8-bit immediate */
	 3,10,10,		/* ALU ops, 16-bit */
	 4,16,10,		/* ALU ops, 16-bit immediate */
	 4,16,10,		/* ALU ops, 16-bit w/8-bit immediate */
	26,35,32,41,	/* MUL */
	25,34,31,40,	/* IMUL */
	29,38,35,44,	/* DIV */
	44,53,50,59,	/* IDIV */
	 3, 3,15,15,	/* INC/DEC */
	 3, 3,10,10,	/* NEG/NOT */

	 2, 5, 1,		/* reg shift/rotate */
	15,17, 1,		/* m8 shift/rotate */
	15,17, 1,		/* m16 shift/rotate */

	22, 5,22,		/* CMPS 8-bit */
	22, 5,22,		/* CMPS 16-bit */
	15, 5,15,		/* SCAS 8-bit */
	15, 5,15,		/* SCAS 16-bit */
	12, 6,11,		/* LODS 8-bit */
	12, 6,11,		/* LODS 16-bit */
	10, 6, 9,		/* STOS 8-bit */
	10, 6, 9,		/* STOS 16-bit */
	14, 8, 8,		/* MOVS 8-bit */
	14, 8, 8,		/* MOVS 16-bit */

	(void *)-1,		/* marker to make sure we line up */

	14, 8, 8,		/* (80186) INS 8-bit */
	14, 8, 8,		/* (80186) INS 16-bit */
	14, 8, 8,		/* (80186) OUTS 8-bit */
	14, 8, 8,		/* (80186) OUTS 16-bit */
	14,68,83,		/* (80186) PUSH immediate, PUSHA/POPA */
	22,29,			/* (80186) IMUL immediate 8-bit */
	25,32,			/* (80186) IMUL immediate 16-bit */
	15,25,4,16, 8,	/* (80186) ENTER/LEAVE */
	33,				/* (80186) BOUND */

	(void *)-1		/* marker to make sure we line up */
};


/* these come from the 80286 timings in OPCODE.LST */
/* many of these numbers are suspect */
static const struct i80x86_timing i80286_cycles =
{
	80286,

	23,17,			/* exception, IRET */
	 0, 2, 3, 1,	/* INTs */
	 2,				/* segment overrides */
	 2, 2, 2,		/* flag operations */
	 3, 3,16,14,	/* arithmetic adjusts */
	 3, 3,			/* decimal adjusts */
	 2, 2,			/* sign extension */
	 2, 7, 3, 3, 3, 5,	/* misc */

	 7, 7,11,		/* direct JMPs */
	 7,11,26,		/* indirect JMPs */
	 7,13,			/* direct CALLs */
	 7,11,29,		/* indirect CALLs */
	11,15,11,15,	/* returns */
	 3, 7, 4, 8,	/* conditional JMPs */
	 4, 8, 4, 8,	/* loops */

	 5, 5, 5, 5,	/* port reads */
	 3, 3, 3, 3,	/* port writes */

	 2, 3, 3,		/* move, 8-bit */
	 2, 3,			/* move, 8-bit immediate */
	 2, 3, 3,		/* move, 16-bit */
	 2, 3,			/* move, 16-bit immediate */
	 5, 5, 3, 3,	/* move, AL/AX memory */
	 2, 5, 2, 3,	/* move, segment registers */
	 3, 5,			/* exchange, 8-bit */
	 3, 5, 3,		/* exchange, 16-bit */

	 5, 5, 3, 3,	/* pushes */
	 5, 5, 5, 5,	/* pops */

	 2, 7, 7,		/* ALU ops, 8-bit */
	 3, 7, 7,		/* ALU ops, 8-bit immediate */
	 2, 7, 7,		/* ALU ops, 16-bit */
	 3, 7, 7,		/* ALU ops, 16-bit immediate */
	 3, 7, 7,		/* ALU ops, 16-bit w/8-bit immediate */
	13,21,16,24,	/* MUL */
	13,21,16,24,	/* IMUL */
	14,22,17,25,	/* DIV */
	17,25,20,28,	/* IDIV */
	 2, 2, 7, 7,	/* INC/DEC */
	 2, 2, 7, 7,	/* NEG/NOT */

	 2, 5, 0,		/* reg shift/rotate */
	 7, 8, 1,		/* m8 shift/rotate */
	 7, 8, 1,		/* m16 shift/rotate */

	13, 5,12,		/* CMPS 8-bit */
	13, 5,12,		/* CMPS 16-bit */
	 9, 5, 8,		/* SCAS 8-bit */
	 9, 5, 8,		/* SCAS 16-bit */
	 5, 5, 4,		/* LODS 8-bit */
	 5, 5, 4,		/* LODS 16-bit */
	 4, 4, 3,		/* STOS 8-bit */
	 4, 4, 3,		/* STOS 16-bit */
	 5, 5, 4,		/* MOVS 8-bit */
	 5, 5, 4,		/* MOVS 16-bit */

	(void *)-1,		/* marker to make sure we line up */

	 5, 5, 4,		/* (80186) INS 8-bit */
	 5, 5, 4,		/* (80186) INS 16-bit */
	 5, 5, 4,		/* (80186) OUTS 8-bit */
	 5, 5, 4,		/* (80186) OUTS 16-bit */
	 3,17,19,		/* (80186) PUSH immediate, PUSHA/POPA */
	21,24,			/* (80186) IMUL immediate 8-bit */
	21,24,			/* (80186) IMUL immediate 16-bit */
	11,15,12, 4, 5,	/* (80186) ENTER/LEAVE */
	13,				/* (80186) BOUND */

	(void *)-1		/* marker to make sure we line up */
};
