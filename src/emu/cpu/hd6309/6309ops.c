/*

HNZVC

? = undefined
* = affected
- = unaffected
0 = cleared
1 = set
# = CCr directly affected by instruction
@ = special - carry set if bit 7 is set

*/

#define OP_HANDLER(_name) INLINE void _name (m68_state_t *m68_state)

OP_HANDLER( illegal )
{
	LOG(("HD6309: illegal opcode at %04x\nVectoring to [$fff0]\n",PC));

	CC |= CC_E;
	PUSHWORD(pPC);
	PUSHWORD(pU);
	PUSHWORD(pY);
	PUSHWORD(pX);
	PUSHBYTE(DP);

	if ( MD & MD_EM )
	{
		PUSHBYTE(F);
		PUSHBYTE(E);
		m68_state->icount -= 2;
	}

	PUSHBYTE(B);
	PUSHBYTE(A);
	PUSHBYTE(CC);

	PCD = RM16(m68_state, 0xfff0);
}

static void IIError(m68_state_t *m68_state)
{
	SEII;           // Set illegal Instruction Flag
	illegal(m68_state);     // Vector to Trap handler
}

static void DZError(m68_state_t *m68_state)
{
	SEDZ;           // Set Division by Zero Flag
	illegal(m68_state);     // Vector to Trap handler
}

/* $00 NEG direct ?**** */
OP_HANDLER( neg_di )
{
	UINT16 r,t;
	DIRBYTE(t);
	r = -t;
	CLR_NZVC;
	SET_FLAGS8(0,t,r);
	WM(EAD,r);
}

/* $01 OIM direct ?**** */
OP_HANDLER( oim_di )
{
	UINT8   r,t,im;
	IMMBYTE(im);
	DIRBYTE(t);
	r = im | t;
	CLR_NZV;
	SET_NZ8(r);
	WM(EAD,r);
}

/* $02 AIM direct */
OP_HANDLER( aim_di )
{
	UINT8   r,t,im;
	IMMBYTE(im);
	DIRBYTE(t);
	r = im & t;
	CLR_NZV;
	SET_NZ8(r);
	WM(EAD,r);
}

/* $03 COM direct -**01 */
OP_HANDLER( com_di )
{
	UINT8 t;
	DIRBYTE(t);
	t = ~t;
	CLR_NZV;
	SET_NZ8(t);
	SEC;
	WM(EAD,t);
}

/* $04 LSR direct -0*-* */
OP_HANDLER( lsr_di )
{
	UINT8 t;
	DIRBYTE(t);
	CLR_NZC;
	CC |= (t & CC_C);
	t >>= 1;
	SET_Z8(t);
	WM(EAD,t);
}

/* $05 EIM direct */
OP_HANDLER( eim_di )
{
	UINT8   r,t,im;
	IMMBYTE(im);
	DIRBYTE(t);
	r = im ^ t;
	CLR_NZV;
	SET_NZ8(r);
	WM(EAD,r);
}

/* $06 ROR direct -**-* */
OP_HANDLER( ror_di )
{
	UINT8 t,r;
	DIRBYTE(t);
	r= (CC & CC_C) << 7;
	CLR_NZC;
	CC |= (t & CC_C);
	r |= t>>1;
	SET_NZ8(r);
	WM(EAD,r);
}

/* $07 ASR direct ?**-* */
OP_HANDLER( asr_di )
{
	UINT8 t;
	DIRBYTE(t);
	CLR_NZC;
	CC |= (t & CC_C);
	t = (t & 0x80) | (t >> 1);
	SET_NZ8(t);
	WM(EAD,t);
}

/* $08 ASL direct ?**** */
OP_HANDLER( asl_di )
{
	UINT16 t,r;
	DIRBYTE(t);
	r = t << 1;
	CLR_NZVC;
	SET_FLAGS8(t,t,r);
	WM(EAD,r);
}

/* $09 ROL direct -**** */
OP_HANDLER( rol_di )
{
	UINT16 t,r;
	DIRBYTE(t);
	r = (CC & CC_C) | (t << 1);
	CLR_NZVC;
	SET_FLAGS8(t,t,r);
	WM(EAD,r);
}

/* $0A DEC direct -***- */
OP_HANDLER( dec_di )
{
	UINT8 t;
	DIRBYTE(t);
	--t;
	CLR_NZV;
	SET_FLAGS8D(t);
	WM(EAD,t);
}

/* $0B TIM direct */
OP_HANDLER( tim_di )
{
	UINT8   r,t,im;
	IMMBYTE(im);
	DIRBYTE(t);
	r = im & t;
	CLR_NZV;
	SET_NZ8(r);
}

/* $OC INC direct -***- */
OP_HANDLER( inc_di )
{
	UINT8 t;
	DIRBYTE(t);
	++t;
	CLR_NZV;
	SET_FLAGS8I(t);
	WM(EAD,t);
}

/* $OD TST direct -**0- */
OP_HANDLER( tst_di )
{
	UINT8 t;
	DIRBYTE(t);
	CLR_NZV;
	SET_NZ8(t);
}

/* $0E JMP direct ----- */
OP_HANDLER( jmp_di )
{
	DIRECT;
	PCD = EAD;
}

/* $0F CLR direct -0100 */
OP_HANDLER( clr_di )
{
	DIRECT;
	(void)RM(EAD);
	WM(EAD,0);
	CLR_NZVC;
	SEZ;
}

/* $10 FLAG */

/* $11 FLAG */

/* $12 NOP inherent ----- */
OP_HANDLER( nop )
{
	;
}

/* $13 SYNC inherent ----- */
OP_HANDLER( sync )
{
	/* SYNC stops processing instructions until an interrupt request happens. */
	/* This doesn't require the corresponding interrupt to be enabled: if it */
	/* is disabled, execution continues with the next instruction. */
	m68_state->int_state |= M6809_SYNC;  /* HJB 990227 */
	check_irq_lines(m68_state);
	/* if M6809_SYNC has not been cleared by check_irq_lines(m68_state),
	 * stop execution until the interrupt lines change. */
	if( m68_state->int_state & M6809_SYNC )
		if (m68_state->icount > 0) m68_state->icount = 0;
}

/* $14 sexw inherent */
OP_HANDLER( sexw )
{
	PAIR q;
	q.d = SIGNED_16(W);
	D = q.w.h;
	W = q.w.l;
	CLR_NZ;
	SET_N16(D);
	SET_Z(q.d);
}

/* $15 ILLEGAL */

/* $16 LBRA relative ----- */
OP_HANDLER( lbra )
{
	IMMWORD(EAP);
	PC += EA;
}

/* $17 LBSR relative ----- */
OP_HANDLER( lbsr )
{
	IMMWORD(EAP);
	PUSHWORD(pPC);
	PC += EA;
}

/* $18 ILLEGAL */

/* $19 DAA inherent (A) -**0* */
OP_HANDLER( daa )
{
	UINT8 msn, lsn;
	UINT16 t, cf = 0;
	msn = A & 0xf0; lsn = A & 0x0f;
	if( lsn>0x09 || CC & CC_H) cf |= 0x06;
	if( msn>0x80 && lsn>0x09 ) cf |= 0x60;
	if( msn>0x90 || CC & CC_C) cf |= 0x60;
	t = cf + A;
	CLR_NZV; /* keep carry from previous operation */
	SET_NZ8((UINT8)t); SET_C8(t);
	A = t;
}

/* $1A ORCC immediate ##### */
OP_HANDLER( orcc )
{
	UINT8 t;
	IMMBYTE(t);
	CC |= t;
	check_irq_lines(m68_state); /* HJB 990116 */
}

/* $1B ILLEGAL */

/* $1C ANDCC immediate ##### */
OP_HANDLER( andcc )
{
	UINT8 t;
	IMMBYTE(t);
	CC &= t;
	check_irq_lines(m68_state); /* HJB 990116 */
}

/* $1D SEX inherent -**-- */
OP_HANDLER( sex )
{
	UINT16 t;
	t = SIGNED(B);
	D = t;
	//  CLR_NZV;    Tim Lindner 20020905: verified that V flag is not affected
	CLR_NZ;
	SET_NZ16(t);
}

/* $1E EXG inherent ----- */
OP_HANDLER( exg )
{
	UINT16 t1,t2;
	UINT8 tb;
	int     promote = FALSE;

	IMMBYTE(tb);
	if( (tb^(tb>>4)) & 0x08 )   /* HJB 990225: mixed 8/16 bit case? */
	{
		promote = TRUE;
	}

	switch(tb>>4) {
		case  0: t1 = D;  break;
		case  1: t1 = X;  break;
		case  2: t1 = Y;  break;
		case  3: t1 = U;  break;
		case  4: t1 = S;  break;
		case  5: t1 = PC; break;
		case  6: t1 = W;  break;
		case  7: t1 = V;  break;
		case  8: t1 = (promote ? A + ((A) << 8) : A);  break;
		case  9: t1 = (promote ? B + ((B) << 8) : B);  break;
		case 10: t1 = (promote ? CC + ((CC) << 8) : CC); break;
		case 11: t1 = (promote ? DP + ((DP) << 8) : DP); break;
		case 12: t1 = 0;  break;
		case 13: t1 = 0;  break;
		case 14: t1 = (promote ? E + ((E) << 8) : E); break;
		default: t1 = (promote ? F + ((F) << 8) : F); break;
	}
	switch(tb&15) {
		case  0: t2 = D;  break;
		case  1: t2 = X;  break;
		case  2: t2 = Y;  break;
		case  3: t2 = U;  break;
		case  4: t2 = S;  break;
		case  5: t2 = PC; break;
		case  6: t2 = W;  break;
		case  7: t2 = V;  break;
		case  8: t2 = (promote ? A + ((A) << 8) : A);  break;
		case  9: t2 = (promote ? B + ((B) << 8) : B);  break;
		case 10: t2 = (promote ? CC + ((CC) << 8) : CC); break;
		case 11: t2 = (promote ? DP + ((DP) << 8) : DP); break;
		case 12: t2 = 0;  break;
		case 13: t2 = 0;  break;
		case 14: t2 = (promote ? E + ((E) << 8) : E); break;
		default: t2 = (promote ? F + ((F) << 8) : F); break;
	}

	switch(tb>>4) {
		case  0: D = t2;  break;
		case  1: X = t2;  break;
		case  2: Y = t2;  break;
		case  3: U = t2;  break;
		case  4: S = t2;  break;
		case  5: PC = t2; break;
		case  6: W = t2;  break;
		case  7: V = t2;  break;
		case  8: A = (promote ? t2 >> 8 : t2); break;
		case  9: B = (promote ? t2 & 0xff : t2); break;
		case 10: CC = (promote ? t2 & 0xff : t2); break;
		case 11: DP = (promote ? t2 >> 8 : t2); break;
		case 12: /* 0 = t2 */ break;
		case 13: /* 0 = t2 */ break;
		case 14: E = (promote ? t2 >> 8 : t2); break;
		case 15: F = (promote ? t2 & 0xff : t2); break;
	}
	switch(tb&15) {
		case  0: D = t1;  break;
		case  1: X = t1;  break;
		case  2: Y = t1;  break;
		case  3: U = t1;  break;
		case  4: S = t1;  break;
		case  5: PC = t1; break;
		case  6: W = t1;  break;
		case  7: V = t1;  break;
		case  8: A = (promote ? t1 >> 8 : t1); break;
		case  9: B = (promote ? t1 & 0xff : t1); break;
		case 10: CC = (promote ? t1 & 0xff : t1); break;
		case 11: DP = (promote ? t1 >> 8 : t1); break;
		case 12: /* 0 = t1 */ break;
		case 13: /* 0 = t1 */ break;
		case 14: E = (promote ? t1 >> 8 : t1); break;
		case 15: F = (promote ? t1 & 0xff : t1); break;
	}
}

/* $1F TFR inherent ----- */
OP_HANDLER( tfr )
{
	UINT8 tb;
	UINT16 t;
	int     promote = FALSE;

	IMMBYTE(tb);
	if( (tb^(tb>>4)) & 0x08 )
	{
		promote = TRUE;
	}

	switch(tb>>4) {
		case  0: t = D;  break;
		case  1: t = X;  break;
		case  2: t = Y;  break;
		case  3: t = U;  break;
		case  4: t = S;  break;
		case  5: t = PC; break;
		case  6: t = W;  break;
		case  7: t = V;  break;
		case  8: t = (promote ? A + ((A) << 8) : A);  break;
		case  9: t = (promote ? B + ((B) << 8) : B);  break;
		case 10: t = (promote ? CC + ((CC) << 8) : CC); break;
		case 11: t = (promote ? DP + ((DP) << 8) : DP); break;
		case 12: t = 0;  break;
		case 13: t = 0;  break;
		case 14: t = (promote ? E + ((E) << 8) : E); break;
		default: t = (promote ? F + ((F) << 8) : F); break;
	}

	switch(tb&15) {
		case  0: D = t;  break;
		case  1: X = t;  break;
		case  2: Y = t;  break;
		case  3: U = t;  break;
		case  4: S = t;  break;
		case  5: PC = t; break;
		case  6: W = t;  break;
		case  7: V = t;  break;
		case  8: A = (promote ? t >> 8 : t); break;
		case  9: B = (promote ? t & 0xff : t); break;
		case 10: CC = (promote ? t & 0xff : t); break;
		case 11: DP = (promote ? t >> 8 : t); break;
		case 12: /* 0 = t */ break;
		case 13: /* 0 = t */ break;
		case 14: E = (promote ? t >> 8 : t); break;
		case 15: F = (promote ? t & 0xff : t); break;
	}
}

/* $20 BRA relative ----- */
OP_HANDLER( bra )
{
	UINT8 t;
	IMMBYTE(t);
	PC += SIGNED(t);
}

/* $21 BRN relative ----- */
static UINT8 brn_temp;  // hack around GCC 4.6 error because we need the side effects of IMMBYTE
OP_HANDLER( brn )
{
	IMMBYTE(brn_temp);
}

/* $1021 LBRN relative ----- */
OP_HANDLER( lbrn )
{
	IMMWORD(EAP);
}

/* $22 BHI relative ----- */
OP_HANDLER( bhi )
{
	BRANCH( !(CC & (CC_Z|CC_C)) );
}

/* $1022 LBHI relative ----- */
OP_HANDLER( lbhi )
{
	LBRANCH( !(CC & (CC_Z|CC_C)) );
}

/* $23 BLS relative ----- */
OP_HANDLER( bls )
{
	BRANCH( (CC & (CC_Z|CC_C)) );
}

/* $1023 LBLS relative ----- */
OP_HANDLER( lbls )
{
	LBRANCH( (CC&(CC_Z|CC_C)) );
}

/* $24 BCC relative ----- */
OP_HANDLER( bcc )
{
	BRANCH( !(CC&CC_C) );
}

/* $1024 LBCC relative ----- */
OP_HANDLER( lbcc )
{
	LBRANCH( !(CC&CC_C) );
}

/* $25 BCS relative ----- */
OP_HANDLER( bcs )
{
	BRANCH( (CC&CC_C) );
}

/* $1025 LBCS relative ----- */
OP_HANDLER( lbcs )
{
	LBRANCH( (CC&CC_C) );
}

/* $26 BNE relative ----- */
OP_HANDLER( bne )
{
	BRANCH( !(CC&CC_Z) );
}

/* $1026 LBNE relative ----- */
OP_HANDLER( lbne )
{
	LBRANCH( !(CC&CC_Z) );
}

/* $27 BEQ relative ----- */
OP_HANDLER( beq )
{
	BRANCH( (CC&CC_Z) );
}

/* $1027 LBEQ relative ----- */
OP_HANDLER( lbeq )
{
	LBRANCH( (CC&CC_Z) );
}

/* $28 BVC relative ----- */
OP_HANDLER( bvc )
{
	BRANCH( !(CC&CC_V) );
}

/* $1028 LBVC relative ----- */
OP_HANDLER( lbvc )
{
	LBRANCH( !(CC&CC_V) );
}

/* $29 BVS relative ----- */
OP_HANDLER( bvs )
{
	BRANCH( (CC&CC_V) );
}

/* $1029 LBVS relative ----- */
OP_HANDLER( lbvs )
{
	LBRANCH( (CC&CC_V) );
}

/* $2A BPL relative ----- */
OP_HANDLER( bpl )
{
	BRANCH( !(CC&CC_N) );
}

/* $102A LBPL relative ----- */
OP_HANDLER( lbpl )
{
	LBRANCH( !(CC&CC_N) );
}

/* $2B BMI relative ----- */
OP_HANDLER( bmi )
{
	BRANCH( (CC&CC_N) );
}

/* $102B LBMI relative ----- */
OP_HANDLER( lbmi )
{
	LBRANCH( (CC&CC_N) );
}

/* $2C BGE relative ----- */
OP_HANDLER( bge )
{
	BRANCH( !NXORV );
}

/* $102C LBGE relative ----- */
OP_HANDLER( lbge )
{
	LBRANCH( !NXORV );
}

/* $2D BLT relative ----- */
OP_HANDLER( blt )
{
	BRANCH( NXORV );
}

/* $102D LBLT relative ----- */
OP_HANDLER( lblt )
{
	LBRANCH( NXORV );
}

/* $2E BGT relative ----- */
OP_HANDLER( bgt )
{
	BRANCH( !(NXORV || (CC&CC_Z)) );
}

/* $102E LBGT relative ----- */
OP_HANDLER( lbgt )
{
	LBRANCH( !(NXORV || (CC&CC_Z)) );
}

/* $2F BLE relative ----- */
OP_HANDLER( ble )
{
	BRANCH( (NXORV || (CC&CC_Z)) );
}

/* $102F LBLE relative ----- */
OP_HANDLER( lble )
{
	LBRANCH( (NXORV || (CC&CC_Z)) );
}

#define REGREG_PREAMBLE                                                     \
	IMMBYTE(tb);                                                            \
	if( (tb^(tb>>4)) & 0x08 )                                               \
		{promote = TRUE;}                                                   \
	switch(tb>>4) {                                                         \
		case  0: src16Reg = &D; large = TRUE;  break;                       \
		case  1: src16Reg = &X; large = TRUE;  break;                       \
		case  2: src16Reg = &Y; large = TRUE;  break;                       \
		case  3: src16Reg = &U; large = TRUE;  break;                       \
		case  4: src16Reg = &S; large = TRUE;  break;                       \
		case  5: src16Reg = &PC; large = TRUE; break;                       \
		case  6: src16Reg = &W; large = TRUE;  break;                       \
		case  7: src16Reg = &V; large = TRUE;  break;                       \
		case  8: if (promote) src16Reg = &D; else src8Reg = &A; break;      \
		case  9: if (promote) src16Reg = &D; else src8Reg = &B; break;      \
		case 10: if (promote) src16Reg = &z16; else src8Reg = &CC; break;   \
		case 11: if (promote) src16Reg = &z16; else src8Reg = &DP; break;   \
		case 12: if (promote) src16Reg = &z16; else src8Reg = &z8; break;   \
		case 13: if (promote) src16Reg = &z16; else src8Reg = &z8; break;   \
		case 14: if (promote) src16Reg = &W; else src8Reg = &E; break;      \
		default: if (promote) src16Reg = &W; else src8Reg = &F; break;      \
	}                                                                       \
	switch(tb&15) {                                                         \
		case  0: dst16Reg = &D; large = TRUE;  break;                       \
		case  1: dst16Reg = &X; large = TRUE;  break;                       \
		case  2: dst16Reg = &Y; large = TRUE;  break;                       \
		case  3: dst16Reg = &U; large = TRUE;  break;                       \
		case  4: dst16Reg = &S; large = TRUE;  break;                       \
		case  5: dst16Reg = &PC; large = TRUE; break;                       \
		case  6: dst16Reg = &W; large = TRUE;  break;                       \
		case  7: dst16Reg = &V; large = TRUE;  break;                       \
		case  8: if (promote) dst16Reg = &D; else dst8Reg = &A; break;      \
		case  9: if (promote) dst16Reg = &D; else dst8Reg = &B; break;      \
		case 10: if (promote) dst16Reg = &z16; else dst8Reg = &CC; break;   \
		case 11: if (promote) dst16Reg = &z16; else dst8Reg = &DP; break;   \
		case 12: if (promote) dst16Reg = &z16; else dst8Reg = &z8; break;   \
		case 13: if (promote) dst16Reg = &z16; else dst8Reg = &z8; break;   \
		case 14: if (promote) dst16Reg = &W; else dst8Reg = &E; break;      \
		default: if (promote) dst16Reg = &W; else dst8Reg = &F; break;      \
	}                                                                       \

/* $1030 addr_r r1 + r2 -> r2 */

OP_HANDLER( addr_r )
{
	UINT8   tb, z8 = 0;
	UINT16  z16 = 0, r8;
	UINT32  r16;
	UINT8   *src8Reg = NULL, *dst8Reg = NULL;
	UINT16  *src16Reg = NULL, *dst16Reg = NULL;
	int     promote = FALSE, large = FALSE;

	REGREG_PREAMBLE;

	if ( large )
	{
		r16 = *src16Reg + *dst16Reg;
		CLR_NZVC;
		*dst16Reg = r16;
		SET_FLAGS16(*src16Reg,*dst16Reg,r16);
	}
	else
	{
		r8 = *src8Reg + *dst8Reg;
		CLR_NZVC;
		/* SET_H(*src8Reg,*src8Reg,r8);*/ /*Experimentation prooved this not to be the case */
		*dst8Reg = r8;
		SET_FLAGS8(*src8Reg,*dst8Reg,r8);
	}
}

OP_HANDLER( adcr )
{
	UINT8   tb, z8 = 0;
	UINT16  z16 = 0, r8;
	UINT32  r16;
	UINT8   *src8Reg = NULL, *dst8Reg = NULL;
	UINT16  *src16Reg = NULL, *dst16Reg = NULL;
	int     promote = FALSE, large = FALSE;

	REGREG_PREAMBLE;

	if ( large )
	{
		r16 = *src16Reg + *dst16Reg + (CC & CC_C);
		CLR_NZVC;
		*dst16Reg = r16;
		SET_FLAGS16(*src16Reg,*dst16Reg,r16);
	}
	else
	{
		r8 = *src8Reg + *dst8Reg + (CC & CC_C);
		CLR_NZVC;
		/* SET_H(*src8Reg,*src8Reg,r8);*/ /*Experimentation prooved this not to be the case */
		*dst8Reg = r8;
		SET_FLAGS8(*src8Reg,*dst8Reg,r8);
	}
}

/* $1032 SUBR r1 - r2 -> r2 */
OP_HANDLER( subr )
{
	UINT8   tb, z8 = 0;
	UINT16  z16 = 0, r8;
	UINT32  r16;
	UINT8   *src8Reg = NULL, *dst8Reg = NULL;
	UINT16  *src16Reg = NULL, *dst16Reg = NULL;
	int     promote = FALSE, large = FALSE;

	REGREG_PREAMBLE;

	if ( large )
	{
		r16 = (UINT32)*dst16Reg - (UINT32)*src16Reg;
		CLR_NZVC;
		*dst16Reg = r16;
		SET_FLAGS16((UINT32)*dst16Reg,(UINT32)*src16Reg,r16);
	}
	else
	{
		r8 = *dst8Reg - *src8Reg;
		CLR_NZVC;
		*dst8Reg = r8;
		SET_FLAGS8(*dst8Reg,*src8Reg,r8);
	}
}

/* $1033 SBCR r1 - r2 - C -> r2 */
OP_HANDLER( sbcr )
{
	UINT8   tb, z8 = 0;
	UINT16  z16 = 0, r8;
	UINT32  r16;
	UINT8   *src8Reg = NULL, *dst8Reg = NULL;
	UINT16  *src16Reg = NULL, *dst16Reg = NULL;
	int     promote = FALSE, large = FALSE;

	REGREG_PREAMBLE;

	if ( large )
	{
		r16 = (UINT32)*dst16Reg - (UINT32)*src16Reg - (CC & CC_C);
		CLR_NZVC;
		*dst16Reg = r16;
		SET_FLAGS16((UINT32)*dst16Reg,(UINT32)*src16Reg,r16);
	}
	else
	{
		r8 = *dst8Reg - *src8Reg - (CC & CC_C);
		CLR_NZVC;
		*dst8Reg = r8;
		SET_FLAGS8(*dst8Reg,*src8Reg,r8);
	}
}

/* $1034 ANDR r1 & r2 -> r2 */
OP_HANDLER( andr )
{
	UINT8   tb, z8 = 0;
	UINT16  z16 = 0, r8;
	UINT32  r16;
	UINT8   *src8Reg = NULL, *dst8Reg = NULL;
	UINT16  *src16Reg = NULL, *dst16Reg = NULL;
	int     promote = FALSE, large = FALSE;

	REGREG_PREAMBLE;

	if ( large )
	{
		r16 = *src16Reg & *dst16Reg;
		CLR_NZV;
		*dst16Reg = r16;
		SET_NZ16(r16);
	}
	else
	{
		r8 = *src8Reg & *dst8Reg;
		CLR_NZV;
		*dst8Reg = r8;
		SET_NZ8(r8);
	}
}

/* $1035 ORR r1 | r2 -> r2 */
OP_HANDLER( orr )
{
	UINT8   tb, z8 = 0;
	UINT16  z16 = 0, r8;
	UINT32  r16;
	UINT8   *src8Reg = NULL, *dst8Reg = NULL;
	UINT16  *src16Reg = NULL, *dst16Reg = NULL;
	int     promote = FALSE, large = FALSE;

	REGREG_PREAMBLE;

	if ( large )
	{
		r16 = *src16Reg | *dst16Reg;
		CLR_NZV;
		*dst16Reg = r16;
		SET_NZ16(r16);
	}
	else
	{
		r8 = *src8Reg | *dst8Reg;
		CLR_NZV;
		*dst8Reg = r8;
		SET_NZ8(r8);
	}
}

/* $1036 EORR r1 ^ r2 -> r2 */
OP_HANDLER( eorr )
{
	UINT8   tb, z8 = 0;
	UINT16  z16 = 0, r8;
	UINT32  r16;
	UINT8   *src8Reg = NULL, *dst8Reg = NULL;
	UINT16  *src16Reg = NULL, *dst16Reg = NULL;
	int     promote = FALSE, large = FALSE;

	REGREG_PREAMBLE;

	if ( large )
	{
		r16 = *src16Reg ^ *dst16Reg;
		CLR_NZV;
		*dst16Reg = r16;
		SET_NZ16(r16);
	}
	else
	{
		r8 = *src8Reg ^ *dst8Reg;
		CLR_NZV;
		*dst8Reg = r8;
		SET_NZ8(r8);
	}
}

/* $1037 CMPR r1 - r2 */
OP_HANDLER( cmpr )
{
	UINT8   tb, z8 = 0;
	UINT16  z16 = 0, r8;
	UINT32  r16;
	UINT8   *src8Reg = NULL, *dst8Reg = NULL;
	UINT16  *src16Reg = NULL, *dst16Reg = NULL;
	int     promote = FALSE, large = FALSE;

	REGREG_PREAMBLE;

	if ( large )
	{
		r16 = (UINT32)*dst16Reg - (UINT32)*src16Reg;
		CLR_NZVC;
		SET_FLAGS16((UINT32)*dst16Reg,(UINT32)*src16Reg,r16);
	}
	else
	{
		r8 = *dst8Reg - *src8Reg;
		CLR_NZVC;
		SET_FLAGS8(*dst8Reg,*src8Reg,r8);
	}
}

/* $1138 TFM R0+,R1+ */
OP_HANDLER( tfmpp )
{
	UINT8   tb, srcValue = 0;

	IMMBYTE(tb);

	if ( W != 0 )
	{
		switch(tb>>4) {
			case  0: srcValue = RM(D++); break;
			case  1: srcValue = RM(X++); break;
			case  2: srcValue = RM(Y++); break;
			case  3: srcValue = RM(U++); break;
			case  4: srcValue = RM(S++); break;
			default: IIError(m68_state); return;        /* reg PC through F */
		}

		switch(tb&15) {
			case  0: WM(D++, srcValue); break;
			case  1: WM(X++, srcValue); break;
			case  2: WM(Y++, srcValue); break;
			case  3: WM(U++, srcValue); break;
			case  4: WM(S++, srcValue); break;
			default: IIError(m68_state); return;        /* reg PC through F */
		}

		PCD = PCD - 3;
		W--;
	}
	else
		m68_state->icount -= 6;   /* Needs six aditional cycles to get the 6+3n */
}

/* $1139 TFM R0-,R1- */
OP_HANDLER( tfmmm )
{
	UINT8   tb, srcValue = 0;

	IMMBYTE(tb);

	if ( W != 0 )
	{
		switch(tb>>4) {
			case  0: srcValue = RM(D--); break;
			case  1: srcValue = RM(X--); break;
			case  2: srcValue = RM(Y--); break;
			case  3: srcValue = RM(U--); break;
			case  4: srcValue = RM(S--); break;
			default: IIError(m68_state); return;        /* reg PC through F */
		}

		switch(tb&15) {
			case  0: WM(D--, srcValue); break;
			case  1: WM(X--, srcValue); break;
			case  2: WM(Y--, srcValue); break;
			case  3: WM(U--, srcValue); break;
			case  4: WM(S--, srcValue); break;
			default: IIError(m68_state); return;        /* reg PC through F */
		}

		PCD = PCD - 3;
		W--;
	}
	else
		m68_state->icount -= 6;   /* Needs six aditional cycles to get the 6+3n */
}

/* $113A TFM R0+,R1 */
OP_HANDLER( tfmpc )
{
	UINT8   tb, srcValue = 0;

	IMMBYTE(tb);

	if ( W != 0 )
	{
		switch(tb>>4) {
			case  0: srcValue = RM(D++); break;
			case  1: srcValue = RM(X++); break;
			case  2: srcValue = RM(Y++); break;
			case  3: srcValue = RM(U++); break;
			case  4: srcValue = RM(S++); break;
			default: IIError(m68_state); return;        /* reg PC through F */
		}

		switch(tb&15) {
			case  0: WM(D, srcValue); break;
			case  1: WM(X, srcValue); break;
			case  2: WM(Y, srcValue); break;
			case  3: WM(U, srcValue); break;
			case  4: WM(S, srcValue); break;
			default: IIError(m68_state); return;        /* reg PC through F */
		}

		PCD = PCD - 3;
		W--;
	}
	else
		m68_state->icount -= 6;   /* Needs six aditional cycles to get the 6+3n */
}

/* $113B TFM R0,R1+ */
OP_HANDLER( tfmcp )
{
	UINT8   tb, srcValue = 0;

	IMMBYTE(tb);

	if ( W != 0 )
	{
		switch(tb>>4) {
			case  0: srcValue = RM(D); break;
			case  1: srcValue = RM(X); break;
			case  2: srcValue = RM(Y); break;
			case  3: srcValue = RM(U); break;
			case  4: srcValue = RM(S); break;
			default: IIError(m68_state); return;        /* reg PC through F */
		}

		switch(tb&15) {
			case  0: WM(D++, srcValue); break;
			case  1: WM(X++, srcValue); break;
			case  2: WM(Y++, srcValue); break;
			case  3: WM(U++, srcValue); break;
			case  4: WM(S++, srcValue); break;
			default: IIError(m68_state); return;        /* reg PC through F */
		}

		PCD = PCD - 3;
		W--;
	}
	else
		m68_state->icount -= 6;   /* Needs six aditional cycles to get the 6+3n */
}

/* $30 LEAX indexed --*-- */
OP_HANDLER( leax )
{
	fetch_effective_address(m68_state);
	X = EA;
	CLR_Z;
	SET_Z(X);
}

/* $31 LEAY indexed --*-- */
OP_HANDLER( leay )
{
	fetch_effective_address(m68_state);
	Y = EA;
	CLR_Z;
	SET_Z(Y);
}

/* $32 LEAS indexed ----- */
OP_HANDLER( leas )
{
	fetch_effective_address(m68_state);
	S = EA;
	m68_state->int_state |= M6809_LDS;
}

/* $33 LEAU indexed ----- */
OP_HANDLER( leau )
{
	fetch_effective_address(m68_state);
	U = EA;
}

/* $34 PSHS inherent ----- */
OP_HANDLER( pshs )
{
	UINT8 t;
	IMMBYTE(t);
	if( t&0x80 ) { PUSHWORD(pPC); m68_state->icount -= 2; }
	if( t&0x40 ) { PUSHWORD(pU);  m68_state->icount -= 2; }
	if( t&0x20 ) { PUSHWORD(pY);  m68_state->icount -= 2; }
	if( t&0x10 ) { PUSHWORD(pX);  m68_state->icount -= 2; }
	if( t&0x08 ) { PUSHBYTE(DP);  m68_state->icount -= 1; }
	if( t&0x04 ) { PUSHBYTE(B);   m68_state->icount -= 1; }
	if( t&0x02 ) { PUSHBYTE(A);   m68_state->icount -= 1; }
	if( t&0x01 ) { PUSHBYTE(CC);  m68_state->icount -= 1; }
}

/* $1038 PSHSW inherent ----- */
OP_HANDLER( pshsw )
{
	PUSHWORD(pW);
}

/* $103a PSHUW inherent ----- */
OP_HANDLER( pshuw )
{
	PSHUWORD(pW);
}

/* $35 PULS inherent ----- */
OP_HANDLER( puls )
{
	UINT8 t;
	IMMBYTE(t);
	if( t&0x01 ) { PULLBYTE(CC); m68_state->icount -= 1; }
	if( t&0x02 ) { PULLBYTE(A);  m68_state->icount -= 1; }
	if( t&0x04 ) { PULLBYTE(B);  m68_state->icount -= 1; }
	if( t&0x08 ) { PULLBYTE(DP); m68_state->icount -= 1; }
	if( t&0x10 ) { PULLWORD(XD); m68_state->icount -= 2; }
	if( t&0x20 ) { PULLWORD(YD); m68_state->icount -= 2; }
	if( t&0x40 ) { PULLWORD(UD); m68_state->icount -= 2; }
	if( t&0x80 ) { PULLWORD(PCD); m68_state->icount -= 2; }

	/* HJB 990225: moved check after all PULLs */
	if( t&0x01 ) { check_irq_lines(m68_state); }
}

/* $1039 PULSW inherent ----- */
OP_HANDLER( pulsw )
{
	PULLWORD(W);
}

/* $103b PULUW inherent ----- */
OP_HANDLER( puluw )
{
	PULUWORD(W);
}

/* $36 PSHU inherent ----- */
OP_HANDLER( pshu )
{
	UINT8 t;
	IMMBYTE(t);
	if( t&0x80 ) { PSHUWORD(pPC); m68_state->icount -= 2; }
	if( t&0x40 ) { PSHUWORD(pS);  m68_state->icount -= 2; }
	if( t&0x20 ) { PSHUWORD(pY);  m68_state->icount -= 2; }
	if( t&0x10 ) { PSHUWORD(pX);  m68_state->icount -= 2; }
	if( t&0x08 ) { PSHUBYTE(DP);  m68_state->icount -= 1; }
	if( t&0x04 ) { PSHUBYTE(B);   m68_state->icount -= 1; }
	if( t&0x02 ) { PSHUBYTE(A);   m68_state->icount -= 1; }
	if( t&0x01 ) { PSHUBYTE(CC);  m68_state->icount -= 1; }
}

/* 37 PULU inherent ----- */
OP_HANDLER( pulu )
{
	UINT8 t;
	IMMBYTE(t);
	if( t&0x01 ) { PULUBYTE(CC); m68_state->icount -= 1; }
	if( t&0x02 ) { PULUBYTE(A);  m68_state->icount -= 1; }
	if( t&0x04 ) { PULUBYTE(B);  m68_state->icount -= 1; }
	if( t&0x08 ) { PULUBYTE(DP); m68_state->icount -= 1; }
	if( t&0x10 ) { PULUWORD(XD); m68_state->icount -= 2; }
	if( t&0x20 ) { PULUWORD(YD); m68_state->icount -= 2; }
	if( t&0x40 ) { PULUWORD(SD); m68_state->icount -= 2; }
	if( t&0x80 ) { PULUWORD(PCD); m68_state->icount -= 2; }

	/* HJB 990225: moved check after all PULLs */
	if( t&0x01 ) { check_irq_lines(m68_state); }
}

/* $38 ILLEGAL */

/* $39 RTS inherent ----- */
OP_HANDLER( rts )
{
	PULLWORD(PCD);
}

/* $3A ABX inherent ----- */
OP_HANDLER( abx )
{
	X += B;
}

/* $3B RTI inherent ##### */
OP_HANDLER( rti )
{
	UINT8 t;
	PULLBYTE(CC);
	t = CC & CC_E;      /* HJB 990225: entire state saved? */
	if(t)
	{
		m68_state->icount -= 9;
		PULLBYTE(A);
		PULLBYTE(B);
		if ( MD & MD_EM )
		{
			PULLBYTE(E);
			PULLBYTE(F);
			m68_state->icount -= 2;
		}
		PULLBYTE(DP);
		PULLWORD(XD);
		PULLWORD(YD);
		PULLWORD(UD);
	}
	PULLWORD(PCD);
	check_irq_lines(m68_state); /* HJB 990116 */
}

/* $3C CWAI inherent ----1 */
OP_HANDLER( cwai )
{
	UINT8 t;
	IMMBYTE(t);
	CC &= t;
	/*
	 * CWAI stacks the entire machine state on the hardware stack,
	 * then waits for an interrupt; when the interrupt is taken
	 * later, the state is *not* saved again after CWAI.
	 */
	CC |= CC_E;         /* HJB 990225: save entire state */
	PUSHWORD(pPC);
	PUSHWORD(pU);
	PUSHWORD(pY);
	PUSHWORD(pX);
	PUSHBYTE(DP);
	if ( MD & MD_EM )
	{
		PUSHBYTE(E);
		PUSHBYTE(F);
	}
	PUSHBYTE(B);
	PUSHBYTE(A);
	PUSHBYTE(CC);
	m68_state->int_state |= M6809_CWAI;  /* HJB 990228 */
	check_irq_lines(m68_state);   /* HJB 990116 */
	if( m68_state->int_state & M6809_CWAI )
		if( m68_state->icount > 0 )
			m68_state->icount = 0;
}

/* $3D MUL inherent --*-@ */
OP_HANDLER( mul )
{
	UINT16 t;
	t = A * B;
	CLR_ZC; SET_Z16(t); if(t&0x80) SEC;
	D = t;
}

/* $3E ILLEGAL */

/* $3F SWI (SWI2 SWI3) absolute indirect ----- */
OP_HANDLER( swi )
{
	CC |= CC_E;             /* HJB 980225: save entire state */
	PUSHWORD(pPC);
	PUSHWORD(pU);
	PUSHWORD(pY);
	PUSHWORD(pX);
	PUSHBYTE(DP);
	if ( MD & MD_EM )
	{
		PUSHBYTE(F);
		PUSHBYTE(E);
	}
	PUSHBYTE(B);
	PUSHBYTE(A);
	PUSHBYTE(CC);
	CC |= CC_IF | CC_II;    /* inhibit FIRQ and IRQ */
	PCD=RM16(m68_state, 0xfffa);
}

/* $1130 BAND */

#define decodePB_tReg(n)    (((n) >> 6) & 0x03)
#define decodePB_src(n)     (((n) >> 3) & 0x07)
#define decodePB_dst(n)     (((n) >> 0) & 0x07)

static const UINT8 bitTable[] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };

OP_HANDLER( band )
{
	UINT8       pb;
	UINT16      db;

	IMMBYTE(pb);

	DIRBYTE(db);

	if ( ( *(m68_state->regTable[decodePB_tReg(pb)]) & bitTable[decodePB_dst(pb)] ) && ( db & bitTable[decodePB_src(pb)] ))
		*(m68_state->regTable[decodePB_tReg(pb)]) |= bitTable[decodePB_dst(pb)];
	else
		*(m68_state->regTable[decodePB_tReg(pb)]) &= (~bitTable[decodePB_dst(pb)]);
}

/* $1131 BIAND */

OP_HANDLER( biand )
{
	UINT8       pb;
	UINT16      db;

	IMMBYTE(pb);

	DIRBYTE(db);

	if ( ( *(m68_state->regTable[decodePB_tReg(pb)]) & bitTable[decodePB_dst(pb)] ) && ( (~db) & bitTable[decodePB_src(pb)] ))
		*(m68_state->regTable[decodePB_tReg(pb)]) |= bitTable[decodePB_dst(pb)];
	else
		*(m68_state->regTable[decodePB_tReg(pb)]) &= (~bitTable[decodePB_dst(pb)]);
}

/* $1132 BOR */

OP_HANDLER( bor )
{
	UINT8       pb;
	UINT16      db;

	IMMBYTE(pb);

	DIRBYTE(db);

	if ( ( *(m68_state->regTable[decodePB_tReg(pb)]) & bitTable[decodePB_dst(pb)] ) || ( db & bitTable[decodePB_src(pb)] ))
		*(m68_state->regTable[decodePB_tReg(pb)]) |= bitTable[decodePB_dst(pb)];
	else
		*(m68_state->regTable[decodePB_tReg(pb)]) &= (~bitTable[decodePB_dst(pb)]);
}

/* $1133 BIOR */

OP_HANDLER( bior )
{
	UINT8       pb;
	UINT16      db;

	IMMBYTE(pb);

	DIRBYTE(db);

	if ( ( *(m68_state->regTable[decodePB_tReg(pb)]) & bitTable[decodePB_dst(pb)] ) || ( (~db) & bitTable[decodePB_src(pb)] ))
		*(m68_state->regTable[decodePB_tReg(pb)]) |= bitTable[decodePB_dst(pb)];
	else
		*(m68_state->regTable[decodePB_tReg(pb)]) &= (~bitTable[decodePB_dst(pb)]);
}

/* $1134 BEOR */

OP_HANDLER( beor )
{
	UINT8       pb;
	UINT16      db;
	UINT8       tReg, tMem;

	IMMBYTE(pb);

	DIRBYTE(db);

	tReg = *(m68_state->regTable[decodePB_tReg(pb)]) & bitTable[decodePB_dst(pb)];
	tMem = db & bitTable[decodePB_src(pb)];

	if ( (tReg || tMem ) && !(tReg && tMem) )
		*(m68_state->regTable[decodePB_tReg(pb)]) |= bitTable[decodePB_dst(pb)];
	else
		*(m68_state->regTable[decodePB_tReg(pb)]) &= (~bitTable[decodePB_dst(pb)]);
}

/* $1135 BIEOR */

OP_HANDLER( bieor )
{
	UINT8       pb;
	UINT16      db;
	UINT8       tReg, tMem;

	IMMBYTE(pb);

	DIRBYTE(db);

	tReg = *(m68_state->regTable[decodePB_tReg(pb)]) & bitTable[decodePB_dst(pb)];
	tMem = (~db) & bitTable[decodePB_src(pb)];

	if ( (tReg || tMem ) && !(tReg && tMem) )
		*(m68_state->regTable[decodePB_tReg(pb)]) |= bitTable[decodePB_dst(pb)];
	else
		*(m68_state->regTable[decodePB_tReg(pb)]) &= (~bitTable[decodePB_dst(pb)]);
}

/* $1133 LDBT */

OP_HANDLER( ldbt )
{
	UINT8       pb;
	UINT16      db;

	IMMBYTE(pb);

	DIRBYTE(db);

	if ( ( db & bitTable[decodePB_src(pb)] ) )
		*(m68_state->regTable[decodePB_tReg(pb)]) |= bitTable[decodePB_dst(pb)];
	else
		*(m68_state->regTable[decodePB_tReg(pb)]) &= (~bitTable[decodePB_dst(pb)]);
}

/* $1134 STBT */

OP_HANDLER( stbt )
{
	UINT8       pb;
	UINT16      db;

	IMMBYTE(pb);

	DIRBYTE(db);

	if ( ( *(m68_state->regTable[decodePB_tReg(pb)]) & bitTable[decodePB_dst(pb)] ) )
		WM( EAD, db | bitTable[decodePB_src(pb)] );
	else
		WM( EAD, db & (~bitTable[decodePB_src(pb)]) );
}

/* $103F SWI2 absolute indirect ----- */
OP_HANDLER( swi2 )
{
	CC |= CC_E;             /* HJB 980225: save entire state */
	PUSHWORD(pPC);
	PUSHWORD(pU);
	PUSHWORD(pY);
	PUSHWORD(pX);
	PUSHBYTE(DP);
	if ( MD & MD_EM )
	{
		PUSHBYTE(F);
		PUSHBYTE(E);
	}
	PUSHBYTE(B);
	PUSHBYTE(A);
	PUSHBYTE(CC);
	PCD = RM16(m68_state, 0xfff4);
}

/* $113F SWI3 absolute indirect ----- */
OP_HANDLER( swi3 )
{
	CC |= CC_E;             /* HJB 980225: save entire state */
	PUSHWORD(pPC);
	PUSHWORD(pU);
	PUSHWORD(pY);
	PUSHWORD(pX);
	PUSHBYTE(DP);
	if ( MD & MD_EM )
	{
		PUSHBYTE(F);
		PUSHBYTE(E);
	}
	PUSHBYTE(B);
	PUSHBYTE(A);
	PUSHBYTE(CC);
	PCD = RM16(m68_state, 0xfff2);
}

/* $40 NEGA inherent ?**** */
OP_HANDLER( nega )
{
	UINT16 r;
	r = -A;
	CLR_NZVC;
	SET_FLAGS8(0,A,r);
	A = r;
}

/* $41 ILLEGAL */

/* $42 ILLEGAL */

/* $43 COMA inherent -**01 */
OP_HANDLER( coma )
{
	A = ~A;
	CLR_NZV;
	SET_NZ8(A);
	SEC;
}

/* $44 LSRA inherent -0*-* */
OP_HANDLER( lsra )
{
	CLR_NZC;
	CC |= (A & CC_C);
	A >>= 1;
	SET_Z8(A);
}

/* $45 ILLEGAL */

/* $46 RORA inherent -**-* */
OP_HANDLER( rora )
{
	UINT8 r;
	r = (CC & CC_C) << 7;
	CLR_NZC;
	CC |= (A & CC_C);
	r |= A >> 1;
	SET_NZ8(r);
	A = r;
}

/* $47 ASRA inherent ?**-* */
OP_HANDLER( asra )
{
	CLR_NZC;
	CC |= (A & CC_C);
	A = (A & 0x80) | (A >> 1);
	SET_NZ8(A);
}

/* $48 ASLA inherent ?**** */
OP_HANDLER( asla )
{
	UINT16 r;
	r = A << 1;
	CLR_NZVC;
	SET_FLAGS8(A,A,r);
	A = r;
}

/* $49 ROLA inherent -**** */
OP_HANDLER( rola )
{
	UINT16 t,r;
	t = A;
	r = (CC & CC_C) | (t<<1);
	CLR_NZVC; SET_FLAGS8(t,t,r);
	A = r;
}

/* $4A DECA inherent -***- */
OP_HANDLER( deca )
{
	--A;
	CLR_NZV;
	SET_FLAGS8D(A);
}

/* $4B ILLEGAL */

/* $4C INCA inherent -***- */
OP_HANDLER( inca )
{
	++A;
	CLR_NZV;
	SET_FLAGS8I(A);
}

/* $4D TSTA inherent -**0- */
OP_HANDLER( tsta )
{
	CLR_NZV;
	SET_NZ8(A);
}

/* $4E ILLEGAL */

/* $4F CLRA inherent -0100 */
OP_HANDLER( clra )
{
	A = 0;
	CLR_NZVC; SEZ;
}

/* $50 NEGB inherent ?**** */
OP_HANDLER( negb )
{
	UINT16 r;
	r = -B;
	CLR_NZVC;
	SET_FLAGS8(0,B,r);
	B = r;
}

/* $1040 NEGD inherent ?**** */
OP_HANDLER( negd )
{
	UINT32 r;
	r = -D;
	CLR_NZVC;
	SET_FLAGS16(0,D,r);
	D = r;
}

/* $51 ILLEGAL */

/* $52 ILLEGAL */

/* $53 COMB inherent -**01 */
OP_HANDLER( comb )
{
	B = ~B;
	CLR_NZV;
	SET_NZ8(B);
	SEC;
}

/* $1143 COME inherent -**01 */
OP_HANDLER( come )
{
	E = ~E;
	CLR_NZV;
	SET_NZ8(E);
	SEC;
}

/* $1153 COMF inherent -**01 */
OP_HANDLER( comf )
{
	F = ~F;
	CLR_NZV;
	SET_NZ8(F);
	SEC;
}

/* $1043 COMD inherent -**01 */
OP_HANDLER( comd )
{
	D = ~D;
	CLR_NZV;
	SET_NZ16(D);
	SEC;
}

/* $1053 COMW inherent -**01 */
OP_HANDLER( comw )
{
	W = ~W;
	CLR_NZV;
	SET_NZ16(W);
	SEC;
}

/* $54 LSRB inherent -0*-* */
OP_HANDLER( lsrb )
{
	CLR_NZC;
	CC |= (B & CC_C);
	B >>= 1;
	SET_Z8(B);
}

/* $1044 LSRD inherent -0*-* */
OP_HANDLER( lsrd )
{
	CLR_NZC;
	CC |= (B & CC_C);
	D >>= 1;
	SET_Z16(D);
}

/* $1054 LSRW inherent -0*-* */
OP_HANDLER( lsrw )
{
	CLR_NZC;
	CC |= (F & CC_C);
	W >>= 1;
	SET_Z16(W);
}

/* $55 ILLEGAL */

/* $56 RORB inherent -**-* */
OP_HANDLER( rorb )
{
	UINT8 r;
	r = (CC & CC_C) << 7;
	CLR_NZC;
	CC |= (B & CC_C);
	r |= B >> 1;
	SET_NZ8(r);
	B = r;
}

/* $1046 RORD inherent -**-* */
OP_HANDLER( rord )
{
	UINT16 r;
	r = (CC & CC_C) << 15;
	CLR_NZC;
	CC |= (D & CC_C);
	r |= D >> 1;
	SET_NZ16(r);
	D = r;
}

/* $1056 RORW inherent -**-* */
OP_HANDLER( rorw )
{
	UINT16 r;
	r = (CC & CC_C) << 15;
	CLR_NZC;
	CC |= (W & CC_C);
	r |= W >> 1;
	SET_NZ16(r);
	W = r;
}

/* $57 ASRB inherent ?**-* */
OP_HANDLER( asrb )
{
	CLR_NZC;
	CC |= (B & CC_C);
	B= (B & 0x80) | (B >> 1);
	SET_NZ8(B);
}

/* $1047 ASRD inherent ?**-* */
OP_HANDLER( asrd )
{
	CLR_NZC;
	CC |= (D & CC_C);
	D= (D & 0x8000) | (D >> 1);
	SET_NZ16(D);
}

/* $58 ASLB inherent ?**** */
OP_HANDLER( aslb )
{
	UINT16 r;
	r = B << 1;
	CLR_NZVC;
	SET_FLAGS8(B,B,r);
	B = r;
}

/* $1048 ASLD inherent ?**** */
OP_HANDLER( asld )
{
	UINT32 r;
	r = D << 1;
	CLR_NZVC;
	SET_FLAGS16(D,D,r);
	D = r;
}

/* $59 ROLB inherent -**** */
OP_HANDLER( rolb )
{
	UINT16 t,r;
	t = B;
	r = CC & CC_C;
	r |= t << 1;
	CLR_NZVC;
	SET_FLAGS8(t,t,r);
	B = r;
}

/* $1049 ROLD inherent -**** */
OP_HANDLER( rold )
{
	UINT32 t,r;
	t = D;
	r = CC & CC_C;
	r |= t << 1;
	CLR_NZVC;
	SET_FLAGS16(t,t,r);
	D = r;
}

/* $1059 ROLW inherent -**** */
OP_HANDLER( rolw )
{
	UINT32 t,r;
	t = W;
	r = CC & CC_C;
	r |= t << 1;
	CLR_NZVC;
	SET_FLAGS16(t,t,r);
	W = r;
}

/* $5A DECB inherent -***- */
OP_HANDLER( decb )
{
	--B;
	CLR_NZV;
	SET_FLAGS8D(B);
}

/* $114a DECE inherent -***- */
OP_HANDLER( dece )
{
	--E;
	CLR_NZV;
	SET_FLAGS8D(E);
}

/* $115a DECF inherent -***- */
OP_HANDLER( decf )
{
	--F;
	CLR_NZV;
	SET_FLAGS8D(F);
}

/* $104a DECD inherent -***- */
OP_HANDLER( decd )
{
	UINT32 r;
	r = D - 1;
	CLR_NZVC;
	SET_FLAGS16(D,D,r)
	D = r;
}

/* $105a DECW inherent -***- */
OP_HANDLER( decw )
{
	UINT32 r;
	r = W - 1;
	CLR_NZVC;
	SET_FLAGS16(W,W,r)
	W = r;
}

/* $5B ILLEGAL */

/* $5C INCB inherent -***- */
OP_HANDLER( incb )
{
	++B;
	CLR_NZV;
	SET_FLAGS8I(B);
}

/* $114c INCE inherent -***- */
OP_HANDLER( ince )
{
	++E;
	CLR_NZV;
	SET_FLAGS8I(E);
}

/* $115c INCF inherent -***- */
OP_HANDLER( incf )
{
	++F;
	CLR_NZV;
	SET_FLAGS8I(F);
}

/* $104c INCD inherent -***- */
OP_HANDLER( incd )
{
	UINT32 r;
	r = D + 1;
	CLR_NZVC;
	SET_FLAGS16(D,D,r)
	D = r;
}

/* $105c INCW inherent -***- */
OP_HANDLER( incw )
{
	UINT32 r;
	r = W + 1;
	CLR_NZVC;
	SET_FLAGS16(W,W,r)
	W = r;
}

/* $5D TSTB inherent -**0- */
OP_HANDLER( tstb )
{
	CLR_NZV;
	SET_NZ8(B);
}

/* $104d TSTD inherent -**0- */
OP_HANDLER( tstd )
{
	CLR_NZV;
	SET_NZ16(D);
}

/* $105d TSTW inherent -**0- */
OP_HANDLER( tstw )
{
	CLR_NZV;
	SET_NZ16(W);
}

/* $114d TSTE inherent -**0- */
OP_HANDLER( tste )
{
	CLR_NZV;
	SET_NZ8(E);
}

/* $115d TSTF inherent -**0- */
OP_HANDLER( tstf )
{
	CLR_NZV;
	SET_NZ8(F);
}

/* $5E ILLEGAL */

/* $5F CLRB inherent -0100 */
OP_HANDLER( clrb )
{
	B = 0;
	CLR_NZVC; SEZ;
}

/* $104f CLRD inherent -0100 */
OP_HANDLER( clrd )
{
	D = 0;
	CLR_NZVC; SEZ;
}

/* $114f CLRE inherent -0100 */
OP_HANDLER( clre )
{
	E = 0;
	CLR_NZVC; SEZ;
}

/* $115f CLRF inherent -0100 */
OP_HANDLER( clrf )
{
	F = 0;
	CLR_NZVC; SEZ;
}

/* $105f CLRW inherent -0100 */
OP_HANDLER( clrw )
{
	W = 0;
	CLR_NZVC; SEZ;
}

/* $60 NEG indexed ?**** */
OP_HANDLER( neg_ix )
{
	UINT16 r,t;
	fetch_effective_address(m68_state);
	t = RM(EAD);
	r=-t;
	CLR_NZVC;
	SET_FLAGS8(0,t,r);
	WM(EAD,r);
}

/* $61 OIM indexed */
OP_HANDLER( oim_ix )
{
	UINT8   r,im;
	IMMBYTE(im);
	fetch_effective_address(m68_state);
	r = im | RM(EAD);
	CLR_NZV;
	SET_NZ8(r);
	WM(EAD,r);
}

/* $62 AIM indexed */
OP_HANDLER( aim_ix )
{
	UINT8   r,im;
	IMMBYTE(im);
	fetch_effective_address(m68_state);
	r = im & RM(EAD);
	CLR_NZV;
	SET_NZ8(r);
	WM(EAD,r);
}

/* $63 COM indexed -**01 */
OP_HANDLER( com_ix )
{
	UINT8 t;
	fetch_effective_address(m68_state);
	t = ~RM(EAD);
	CLR_NZV;
	SET_NZ8(t);
	SEC;
	WM(EAD,t);
}

/* $64 LSR indexed -0*-* */
OP_HANDLER( lsr_ix )
{
	UINT8 t;
	fetch_effective_address(m68_state);
	t=RM(EAD);
	CLR_NZC;
	CC |= (t & CC_C);
	t>>=1; SET_Z8(t);
	WM(EAD,t);
}

/* $65 EIM indexed */
OP_HANDLER( eim_ix )
{
	UINT8   r,im;
	IMMBYTE(im);
	fetch_effective_address(m68_state);
	r = im ^ RM(EAD);
	CLR_NZV;
	SET_NZ8(r);
	WM(EAD,r);
}
/* $66 ROR indexed -**-* */
OP_HANDLER( ror_ix )
{
	UINT8 t,r;
	fetch_effective_address(m68_state);
	t=RM(EAD);
	r = (CC & CC_C) << 7;
	CLR_NZC;
	CC |= (t & CC_C);
	r |= t>>1; SET_NZ8(r);
	WM(EAD,r);
}

/* $67 ASR indexed ?**-* */
OP_HANDLER( asr_ix )
{
	UINT8 t;
	fetch_effective_address(m68_state);
	t=RM(EAD);
	CLR_NZC;
	CC |= (t & CC_C);
	t=(t&0x80)|(t>>1);
	SET_NZ8(t);
	WM(EAD,t);
}

/* $68 ASL indexed ?**** */
OP_HANDLER( asl_ix )
{
	UINT16 t,r;
	fetch_effective_address(m68_state);
	t=RM(EAD);
	r = t << 1;
	CLR_NZVC;
	SET_FLAGS8(t,t,r);
	WM(EAD,r);
}

/* $69 ROL indexed -**** */
OP_HANDLER( rol_ix )
{
	UINT16 t,r;
	fetch_effective_address(m68_state);
	t=RM(EAD);
	r = CC & CC_C;
	r |= t << 1;
	CLR_NZVC;
	SET_FLAGS8(t,t,r);
	WM(EAD,r);
}

/* $6A DEC indexed -***- */
OP_HANDLER( dec_ix )
{
	UINT8 t;
	fetch_effective_address(m68_state);
	t = RM(EAD) - 1;
	CLR_NZV; SET_FLAGS8D(t);
	WM(EAD,t);
}

/* $6B TIM indexed */
OP_HANDLER( tim_ix )
{
	UINT8   r,im,m;
	IMMBYTE(im);
	fetch_effective_address(m68_state);
	m = RM(EAD);
	r = im & m;
	CLR_NZV;
	SET_NZ8(r);
}

/* $6C INC indexed -***- */
OP_HANDLER( inc_ix )
{
	UINT8 t;
	fetch_effective_address(m68_state);
	t = RM(EAD) + 1;
	CLR_NZV; SET_FLAGS8I(t);
	WM(EAD,t);
}

/* $6D TST indexed -**0- */
OP_HANDLER( tst_ix )
{
	UINT8 t;
	fetch_effective_address(m68_state);
	t = RM(EAD);
	CLR_NZV;
	SET_NZ8(t);
}

/* $6E JMP indexed ----- */
OP_HANDLER( jmp_ix )
{
	fetch_effective_address(m68_state);
	PCD = EAD;
}

/* $6F CLR indexed -0100 */
OP_HANDLER( clr_ix )
{
	fetch_effective_address(m68_state);
	(void)RM(EAD);
	WM(EAD,0);
	CLR_NZVC; SEZ;
}

/* $70 NEG extended ?**** */
OP_HANDLER( neg_ex )
{
	UINT16 r,t;
	EXTBYTE(t); r=-t;
	CLR_NZVC; SET_FLAGS8(0,t,r);
	WM(EAD,r);
}

/* $71 OIM extended */
OP_HANDLER( oim_ex )
{
	UINT8   r,t,im;
	IMMBYTE(im);
	EXTBYTE(t);
	r = im | t;
	CLR_NZV;
	SET_NZ8(r);
	WM(EAD,r);
}

/* $72 AIM extended */
OP_HANDLER( aim_ex )
{
	UINT8   r,t,im;
	IMMBYTE(im);
	EXTBYTE(t);
	r = im & t;
	CLR_NZV;
	SET_NZ8(r);
	WM(EAD,r);
}

/* $73 COM extended -**01 */
OP_HANDLER( com_ex )
{
	UINT8 t;
	EXTBYTE(t); t = ~t;
	CLR_NZV; SET_NZ8(t); SEC;
	WM(EAD,t);
}

/* $74 LSR extended -0*-* */
OP_HANDLER( lsr_ex )
{
	UINT8 t;
	EXTBYTE(t); CLR_NZC; CC |= (t & CC_C);
	t>>=1; SET_Z8(t);
	WM(EAD,t);
}

/* $75 EIM extended */
OP_HANDLER( eim_ex )
{
	UINT8   r,t,im;
	IMMBYTE(im);
	EXTBYTE(t);
	r = im ^ t;
	CLR_NZV;
	SET_NZ8(r);
	WM(EAD,r);
}

/* $76 ROR extended -**-* */
OP_HANDLER( ror_ex )
{
	UINT8 t,r;
	EXTBYTE(t); r=(CC & CC_C) << 7;
	CLR_NZC; CC |= (t & CC_C);
	r |= t>>1; SET_NZ8(r);
	WM(EAD,r);
}

/* $77 ASR extended ?**-* */
OP_HANDLER( asr_ex )
{
	UINT8 t;
	EXTBYTE(t); CLR_NZC; CC |= (t & CC_C);
	t=(t&0x80)|(t>>1);
	SET_NZ8(t);
	WM(EAD,t);
}

/* $78 ASL extended ?**** */
OP_HANDLER( asl_ex )
{
	UINT16 t,r;
	EXTBYTE(t); r=t<<1;
	CLR_NZVC; SET_FLAGS8(t,t,r);
	WM(EAD,r);
}

/* $79 ROL extended -**** */
OP_HANDLER( rol_ex )
{
	UINT16 t,r;
	EXTBYTE(t); r = (CC & CC_C) | (t << 1);
	CLR_NZVC; SET_FLAGS8(t,t,r);
	WM(EAD,r);
}

/* $7A DEC extended -***- */
OP_HANDLER( dec_ex )
{
	UINT8 t;
	EXTBYTE(t); --t;
	CLR_NZV; SET_FLAGS8D(t);
	WM(EAD,t);
}

/* $7B TIM extended */
OP_HANDLER( tim_ex )
{
	UINT8   r,t,im;
	IMMBYTE(im);
	EXTBYTE(t);
	r = im & t;
	CLR_NZV;
	SET_NZ8(r);
}

/* $7C INC extended -***- */
OP_HANDLER( inc_ex )
{
	UINT8 t;
	EXTBYTE(t); ++t;
	CLR_NZV; SET_FLAGS8I(t);
	WM(EAD,t);
}

/* $7D TST extended -**0- */
OP_HANDLER( tst_ex )
{
	UINT8 t;
	EXTBYTE(t); CLR_NZV; SET_NZ8(t);
}

/* $7E JMP extended ----- */
OP_HANDLER( jmp_ex )
{
	EXTENDED;
	PCD = EAD;
}

/* $7F CLR extended -0100 */
OP_HANDLER( clr_ex )
{
	EXTENDED;
	(void)RM(EAD);
	WM(EAD,0);
	CLR_NZVC; SEZ;
}

/* $80 SUBA immediate ?**** */
OP_HANDLER( suba_im )
{
	UINT16 t,r;
	IMMBYTE(t);
	r = A - t;
	CLR_NZVC;
	SET_FLAGS8(A,t,r);
	A = r;
}

/* $81 CMPA immediate ?**** */
OP_HANDLER( cmpa_im )
{
	UINT16    t,r;
	IMMBYTE(t);
	r = A - t;
	CLR_NZVC;
	SET_FLAGS8(A,t,r);
}

/* $82 SBCA immediate ?**** */
OP_HANDLER( sbca_im )
{
	UINT16    t,r;
	IMMBYTE(t);
	r = A - t - (CC & CC_C);
	CLR_NZVC;
	SET_FLAGS8(A,t,r);
	A = r;
}

/* $83 SUBD (CMPD CMPU) immediate -**** */
OP_HANDLER( subd_im )
{
	UINT32 r,d;
	PAIR b;
	IMMWORD(b);
	d = D;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
	D = r;
}

/* $1080 SUBW immediate -**** */
OP_HANDLER( subw_im )
{
	UINT32 r,d;
	PAIR b;
	IMMWORD(b);
	d = W;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
	W = r;
}

/* $1083 CMPD immediate -**** */
OP_HANDLER( cmpd_im )
{
	UINT32 r,d;
	PAIR b;
	IMMWORD(b);
	d = D;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
}

/* $1081 CMPW immediate -**** */
OP_HANDLER( cmpw_im )
{
	UINT32 r,d;
	PAIR b;
	IMMWORD(b);
	d = W;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
}

/* $1183 CMPU immediate -**** */
OP_HANDLER( cmpu_im )
{
	UINT32 r, d;
	PAIR b;
	IMMWORD(b);
	d = U;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
}

/* $84 ANDA immediate -**0- */
OP_HANDLER( anda_im )
{
	UINT8 t;
	IMMBYTE(t);
	A &= t;
	CLR_NZV;
	SET_NZ8(A);
}

/* $85 BITA immediate -**0- */
OP_HANDLER( bita_im )
{
	UINT8 t,r;
	IMMBYTE(t);
	r = A & t;
	CLR_NZV;
	SET_NZ8(r);
}

/* $86 LDA immediate -**0- */
OP_HANDLER( lda_im )
{
	IMMBYTE(A);
	CLR_NZV;
	SET_NZ8(A);
}

/* $88 EORA immediate -**0- */
OP_HANDLER( eora_im )
{
	UINT8 t;
	IMMBYTE(t);
	A ^= t;
	CLR_NZV;
	SET_NZ8(A);
}

/* $89 ADCA immediate ***** */
OP_HANDLER( adca_im )
{
	UINT16 t,r;
	IMMBYTE(t);
	r = A + t + (CC & CC_C);
	CLR_HNZVC;
	SET_FLAGS8(A,t,r);
	SET_H(A,t,r);
	A = r;
}

/* $8A ORA immediate -**0- */
OP_HANDLER( ora_im )
{
	UINT8 t;
	IMMBYTE(t);
	A |= t;
	CLR_NZV;
	SET_NZ8(A);
}

/* $8B ADDA immediate ***** */
OP_HANDLER( adda_im )
{
	UINT16 t,r;
	IMMBYTE(t);
	r = A + t;
	CLR_HNZVC;
	SET_FLAGS8(A,t,r);
	SET_H(A,t,r);
	A = r;
}

/* $8C CMPX (CMPY CMPS) immediate -**** */
OP_HANDLER( cmpx_im )
{
	UINT32 r,d;
	PAIR b;
	IMMWORD(b);
	d = X;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
}

/* $108C CMPY immediate -**** */
OP_HANDLER( cmpy_im )
{
	UINT32 r,d;
	PAIR b;
	IMMWORD(b);
	d = Y;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
}

/* $118C CMPS immediate -**** */
OP_HANDLER( cmps_im )
{
	UINT32 r,d;
	PAIR b;
	IMMWORD(b);
	d = S;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
}

/* $8D BSR ----- */
OP_HANDLER( bsr )
{
	UINT8 t;
	IMMBYTE(t);
	PUSHWORD(pPC);
	PC += SIGNED(t);
}

/* $8E LDX (LDY) immediate -**0- */
OP_HANDLER( ldx_im )
{
	IMMWORD(pX);
	CLR_NZV;
	SET_NZ16(X);
}

/* $CD LDQ immediate -**0- */
OP_HANDLER( ldq_im )
{
	PAIR    q;

	IMMLONG(q);
	D = q.w.h;
	W = q.w.l;
	CLR_NZV;
	SET_N8(A);
	SET_Z(q.d);
}

/* $108E LDY immediate -**0- */
OP_HANDLER( ldy_im )
{
	IMMWORD(pY);
	CLR_NZV;
	SET_NZ16(Y);
}

/* $118f MULD immediate */
OP_HANDLER( muld_im )
{
	PAIR t, q;

	IMMWORD( t );
	q.d = (INT16) D * (INT16)t.w.l;
	D = q.w.h;
	W = q.w.l;
	CLR_NZVC;
	SET_NZ16(D);
}

/* $118d DIVD immediate */
OP_HANDLER( divd_im )
{
	UINT8   t;
	INT16   v, oldD;

	IMMBYTE( t );

	if( t != 0 )
	{
		oldD = D;
		v = (INT16) D / (INT8) t;
		A = (INT16) D % (INT8) t;
		B = v;

		CLR_NZVC;
		SET_NZ8(B);

		if( B & 0x01 )
			SEC;

		if( (INT16)D < 0 )
			SEN;

		if ( (v > 127) || (v < -128) ) /* soft overflow */
		{
			SEV;

			if( (v > 255) || (v < -256) ) /* hard overflow - division is aborted */
			{
				SET_NZ16( oldD );
				D = abs( oldD );
			}
		}
	}
	else
	{
		m68_state->icount -= 8;
		DZError(m68_state);
	}
}

/* $118e DIVQ immediate */
OP_HANDLER( divq_im )
{
	PAIR    t,q, oldQ;
	INT32   v;

	IMMWORD( t );

	q.w.h = D;
	q.w.l = W;

	if( t.w.l != 0 )
	{
		oldQ = q;

		v = (INT32) q.d / (INT16) t.w.l;
		D = (INT32) q.d % (INT16) t.w.l;
		W = v;

		CLR_NZVC;
		SET_NZ16(W);

		if( W & 0x0001 )
			SEC;

		if ( (v > 32768) || (v < -32767) ) /* soft overflow */
		{
			SEV;

			if( (v > 65536 ) || (v < -65535 ) ) /* hard overflow - division is aborted */
			{
				if( (INT32)oldQ.d < 0 )
					SEN;
				else if( oldQ.d == 0 )
					SEZ;

				t.w.l = abs( t.w.l );
				D = oldQ.w.h;
				W = oldQ.w.l;
			}
		}
	}
	else
		DZError(m68_state);
}

/* $90 SUBA direct ?**** */
OP_HANDLER( suba_di )
{
	UINT16    t,r;
	DIRBYTE(t);
	r = A - t;
	CLR_NZVC;
	SET_FLAGS8(A,t,r);
	A = r;
}

/* $91 CMPA direct ?**** */
OP_HANDLER( cmpa_di )
{
	UINT16    t,r;
	DIRBYTE(t);
	r = A - t;
	CLR_NZVC;
	SET_FLAGS8(A,t,r);
}

/* $92 SBCA direct ?**** */
OP_HANDLER( sbca_di )
{
	UINT16    t,r;
	DIRBYTE(t);
	r = A - t - (CC & CC_C);
	CLR_NZVC;
	SET_FLAGS8(A,t,r);
	A = r;
}

/* $93 SUBD (CMPD CMPU) direct -**** */
OP_HANDLER( subd_di )
{
	UINT32 r,d;
	PAIR b;
	DIRWORD(b);
	d = D;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
	D = r;
}

/* $1090 SUBW direct -**** */
OP_HANDLER( subw_di )
{
	UINT32 r,d;
	PAIR b;
	DIRWORD(b);
	d = W;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
	W = r;
}

/* $1093 CMPD direct -**** */
OP_HANDLER( cmpd_di )
{
	UINT32 r,d;
	PAIR b;
	DIRWORD(b);
	d = D;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
}

/* $1091 CMPW direct -**** */
OP_HANDLER( cmpw_di )
{
	UINT32 r,d;
	PAIR b;
	DIRWORD(b);
	d = W;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
}

/* $1193 CMPU direct -**** */
OP_HANDLER( cmpu_di )
{
	UINT32 r,d;
	PAIR b;
	DIRWORD(b);
	d = U;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(U,b.d,r);
}

/* $94 ANDA direct -**0- */
OP_HANDLER( anda_di )
{
	UINT8 t;
	DIRBYTE(t);
	A &= t;
	CLR_NZV;
	SET_NZ8(A);
}

/* $95 BITA direct -**0- */
OP_HANDLER( bita_di )
{
	UINT8 t,r;
	DIRBYTE(t);
	r = A & t;
	CLR_NZV;
	SET_NZ8(r);
}

/* $96 LDA direct -**0- */
OP_HANDLER( lda_di )
{
	DIRBYTE(A);
	CLR_NZV;
	SET_NZ8(A);
}

/* $97 STA direct -**0- */
OP_HANDLER( sta_di )
{
	CLR_NZV;
	SET_NZ8(A);
	DIRECT;
	WM(EAD,A);
}

/* $98 EORA direct -**0- */
OP_HANDLER( eora_di )
{
	UINT8 t;
	DIRBYTE(t);
	A ^= t;
	CLR_NZV;
	SET_NZ8(A);
}

/* $99 ADCA direct ***** */
OP_HANDLER( adca_di )
{
	UINT16 t,r;
	DIRBYTE(t);
	r = A + t + (CC & CC_C);
	CLR_HNZVC;
	SET_FLAGS8(A,t,r);
	SET_H(A,t,r);
	A = r;
}

/* $9A ORA direct -**0- */
OP_HANDLER( ora_di )
{
	UINT8 t;
	DIRBYTE(t);
	A |= t;
	CLR_NZV;
	SET_NZ8(A);
}

/* $9B ADDA direct ***** */
OP_HANDLER( adda_di )
{
	UINT16 t,r;
	DIRBYTE(t);
	r = A + t;
	CLR_HNZVC;
	SET_FLAGS8(A,t,r);
	SET_H(A,t,r);
	A = r;
}

/* $9C CMPX (CMPY CMPS) direct -**** */
OP_HANDLER( cmpx_di )
{
	UINT32 r,d;
	PAIR b;
	DIRWORD(b);
	d = X;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
}

/* $109C CMPY direct -**** */
OP_HANDLER( cmpy_di )
{
	UINT32 r,d;
	PAIR b;
	DIRWORD(b);
	d = Y;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
}

/* $119C CMPS direct -**** */
OP_HANDLER( cmps_di )
{
	UINT32 r,d;
	PAIR b;
	DIRWORD(b);
	d = S;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
}

/* $9D JSR direct ----- */
OP_HANDLER( jsr_di )
{
	DIRECT;
	PUSHWORD(pPC);
	PCD = EAD;
}

/* $9E LDX (LDY) direct -**0- */
OP_HANDLER( ldx_di )
{
	DIRWORD(pX);
	CLR_NZV;
	SET_NZ16(X);
}

/* $119f MULD direct -**0- */
OP_HANDLER( muld_di )
{
	PAIR    t,q;

	DIRWORD(t);
	q.d = (INT16) D * (INT16)t.w.l;

	D = q.w.h;
	W = q.w.l;
	CLR_NZVC;
	SET_NZ16(D);
}

/* $119d DIVD direct -**0- */
OP_HANDLER( divd_di )
{
	UINT8   t;
	INT16   v, oldD;

	DIRBYTE(t);

	if( t != 0 )
	{
		oldD = D;
		v = (INT16) D / (INT8) t;
		A = (INT16) D % (INT8) t;
		B = v;

		CLR_NZVC;
		SET_NZ8(B);

		if( B & 0x01 )
			SEC;

		if( (INT16)D < 0 )
			SEN;

		if ( (v > 127) || (v < -128) ) /* soft overflow */
		{
			SEV;

			if( (v > 255) || (v < -256) ) /* hard overflow - division is aborted */
			{
				SET_NZ16( oldD );
				D = abs( oldD );
			}
		}
	}
	else
	{
		m68_state->icount -= 8;
		DZError(m68_state);
	}
}

/* $119e DIVQ direct -**0- */
OP_HANDLER( divq_di )
{
	PAIR    t,q, oldQ;
	INT32   v;

	DIRWORD(t);

	q.w.h = D;
	q.w.l = W;

	if( t.w.l != 0 )
	{
		oldQ = q;

		v = (INT32) q.d / (INT16) t.w.l;
		D = (INT32) q.d % (INT16) t.w.l;
		W = v;

		CLR_NZVC;
		SET_NZ16(W);

		if( W & 0x0001 )
			SEC;

		if ( (v > 32767) || (v < -32768) ) /* soft overflow */
		{
			SEV;

			if( (v > 65535 ) || (v < -65536 ) ) /* hard overflow - division is aborted */
			{
				if( (INT32)oldQ.d < 0 )
					SEN;
				else if( oldQ.d == 0 )
					SEZ;

				t.w.l = abs( t.w.l );
				D = oldQ.w.h;
				W = oldQ.w.l;
			}
		}
	}
	else
		DZError(m68_state);
}
/* $10dc LDQ direct -**0- */
OP_HANDLER( ldq_di )
{
	PAIR    q;

	DIRLONG(q);
	D = q.w.h;
	W = q.w.l;
	CLR_NZV;
	SET_N8(A);
	SET_Z(q.d);
}

/* $109E LDY direct -**0- */
OP_HANDLER( ldy_di )
{
	DIRWORD(pY);
	CLR_NZV;
	SET_NZ16(Y);
}

/* $9F STX (STY) direct -**0- */
OP_HANDLER( stx_di )
{
	CLR_NZV;
	SET_NZ16(X);
	DIRECT;
	WM16(m68_state, EAD,&pX);
}

/* $10dd STQ direct -**0- */
OP_HANDLER( stq_di )
{
	PAIR    q;

	q.w.h = D;
	q.w.l = W;
	DIRECT;
	WM32(m68_state, EAD,&q);
	CLR_NZV;
	SET_N8(A);
	SET_Z(q.d);
}

/* $109F STY direct -**0- */
OP_HANDLER( sty_di )
{
	CLR_NZV;
	SET_NZ16(Y);
	DIRECT;
	WM16(m68_state, EAD,&pY);
}

/* $a0 SUBA indexed ?**** */
OP_HANDLER( suba_ix )
{
	UINT16 t,r;
	fetch_effective_address(m68_state);
	t = RM(EAD);
	r = A - t;
	CLR_NZVC;
	SET_FLAGS8(A,t,r);
	A = r;
}

/* $a1 CMPA indexed ?**** */
OP_HANDLER( cmpa_ix )
{
	UINT16 t,r;
	fetch_effective_address(m68_state);
	t = RM(EAD);
	r = A - t;
	CLR_NZVC;
	SET_FLAGS8(A,t,r);
}

/* $a2 SBCA indexed ?**** */
OP_HANDLER( sbca_ix )
{
	UINT16    t,r;
	fetch_effective_address(m68_state);
	t = RM(EAD);
	r = A - t - (CC & CC_C);
	CLR_NZVC;
	SET_FLAGS8(A,t,r);
	A = r;
}

/* $a3 SUBD (CMPD CMPU) indexed -**** */
OP_HANDLER( subd_ix )
{
	UINT32 r,d;
	PAIR b;
	fetch_effective_address(m68_state);
	b.d=RM16(m68_state, EAD);
	d = D;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
	D = r;
}

/* $10a0 SUBW indexed -**** */
OP_HANDLER( subw_ix )
{
	UINT32 r,d;
	PAIR b;
	fetch_effective_address(m68_state);
	b.d=RM16(m68_state, EAD);
	d = W;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
	W = r;
}

/* $10a1 CMPW indexed -**** */
OP_HANDLER( cmpw_ix )
{
	UINT32 r,d;
	PAIR b;
	fetch_effective_address(m68_state);
	b.d=RM16(m68_state, EAD);
	d = W;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
}

/* $10a3 CMPD indexed -**** */
OP_HANDLER( cmpd_ix )
{
	UINT32 r,d;
	PAIR b;
	fetch_effective_address(m68_state);
	b.d=RM16(m68_state, EAD);
	d = D;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
}

/* $11a3 CMPU indexed -**** */
OP_HANDLER( cmpu_ix )
{
	UINT32 r;
	PAIR b;
	fetch_effective_address(m68_state);
	b.d=RM16(m68_state, EAD);
	r = U - b.d;
	CLR_NZVC;
	SET_FLAGS16(U,b.d,r);
}

/* $a4 ANDA indexed -**0- */
OP_HANDLER( anda_ix )
{
	fetch_effective_address(m68_state);
	A &= RM(EAD);
	CLR_NZV;
	SET_NZ8(A);
}

/* $a5 BITA indexed -**0- */
OP_HANDLER( bita_ix )
{
	UINT8 r;
	fetch_effective_address(m68_state);
	r = A & RM(EAD);
	CLR_NZV;
	SET_NZ8(r);
}

/* $a6 LDA indexed -**0- */
OP_HANDLER( lda_ix )
{
	fetch_effective_address(m68_state);
	A = RM(EAD);
	CLR_NZV;
	SET_NZ8(A);
}

/* $a7 STA indexed -**0- */
OP_HANDLER( sta_ix )
{
	fetch_effective_address(m68_state);
	CLR_NZV;
	SET_NZ8(A);
	WM(EAD,A);
}

/* $a8 EORA indexed -**0- */
OP_HANDLER( eora_ix )
{
	fetch_effective_address(m68_state);
	A ^= RM(EAD);
	CLR_NZV;
	SET_NZ8(A);
}

/* $a9 ADCA indexed ***** */
OP_HANDLER( adca_ix )
{
	UINT16 t,r;
	fetch_effective_address(m68_state);
	t = RM(EAD);
	r = A + t + (CC & CC_C);
	CLR_HNZVC;
	SET_FLAGS8(A,t,r);
	SET_H(A,t,r);
	A = r;
}

/* $aA ORA indexed -**0- */
OP_HANDLER( ora_ix )
{
	fetch_effective_address(m68_state);
	A |= RM(EAD);
	CLR_NZV;
	SET_NZ8(A);
}

/* $aB ADDA indexed ***** */
OP_HANDLER( adda_ix )
{
	UINT16 t,r;
	fetch_effective_address(m68_state);
	t = RM(EAD);
	r = A + t;
	CLR_HNZVC;
	SET_FLAGS8(A,t,r);
	SET_H(A,t,r);
	A = r;
}

/* $aC CMPX (CMPY CMPS) indexed -**** */
OP_HANDLER( cmpx_ix )
{
	UINT32 r,d;
	PAIR b;
	fetch_effective_address(m68_state);
	b.d=RM16(m68_state, EAD);
	d = X;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
}

/* $10aC CMPY indexed -**** */
OP_HANDLER( cmpy_ix )
{
	UINT32 r,d;
	PAIR b;
	fetch_effective_address(m68_state);
	b.d=RM16(m68_state, EAD);
	d = Y;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
}

/* $11aC CMPS indexed -**** */
OP_HANDLER( cmps_ix )
{
	UINT32 r,d;
	PAIR b;
	fetch_effective_address(m68_state);
	b.d=RM16(m68_state, EAD);
	d = S;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
}

/* $aD JSR indexed ----- */
OP_HANDLER( jsr_ix )
{
	fetch_effective_address(m68_state);
	PUSHWORD(pPC);
	PCD = EAD;
}

/* $aE LDX (LDY) indexed -**0- */
OP_HANDLER( ldx_ix )
{
	fetch_effective_address(m68_state);
	X=RM16(m68_state, EAD);
	CLR_NZV;
	SET_NZ16(X);
}

/* $11af MULD indexed -**0- */
OP_HANDLER( muld_ix )
{
	PAIR    q;
	UINT16  t;

	fetch_effective_address(m68_state);
	t=RM16(m68_state, EAD);
	q.d = (INT16) D * (INT16)t;

	D = q.w.h;
	W = q.w.l;
	CLR_NZVC;
	SET_NZ16(D);
}

/* $11ad DIVD indexed -**0- */
OP_HANDLER( divd_ix )
{
	UINT8   t;
	INT16   v, oldD;

	fetch_effective_address(m68_state);
	t=RM(EAD);

	if( t != 0 )
	{
		oldD = D;
		v = (INT16) D / (INT8) t;
		A = (INT16) D % (INT8) t;
		B = v;

		CLR_NZVC;
		SET_NZ8(B);

		if( B & 0x01 )
			SEC;

		if( (INT16)D < 0 )
			SEN;

		if ( (v > 127) || (v < -128) ) /* soft overflow */
		{
			SEV;

			if( (v > 255) || (v < -256) ) /* hard overflow - division is aborted */
			{
				SET_NZ16( oldD );
				D = abs( oldD );
			}
		}
	}
	else
	{
		m68_state->icount -= 8;
		DZError(m68_state);
	}
}

/* $11ae DIVQ indexed -**0- */
OP_HANDLER( divq_ix )
{
	PAIR    t,q, oldQ;
	INT32   v;

	fetch_effective_address(m68_state);
	t.w.l=RM16(m68_state, EAD);

	q.w.h = D;
	q.w.l = W;

	if( t.w.l != 0 )
	{
		oldQ = q;

		v = (INT32) q.d / (INT16) t.w.l;
		D = (INT32) q.d % (INT16) t.w.l;
		W = v;

		CLR_NZVC;
		SET_NZ16(W);

		if( W & 0x0001 )
			SEC;

		if ( (v > 32767) || (v < -32768) ) /* soft overflow */
		{
			SEV;

			if( (v > 65535 ) || (v < -65536 ) ) /* hard overflow - division is aborted */
			{
				if( (INT32)oldQ.d < 0 )
					SEN;
				else if( oldQ.d == 0 )
					SEZ;

				t.w.l = abs( t.w.l );
				D = oldQ.w.h;
				W = oldQ.w.l;
			}
		}
	}
	else
		DZError(m68_state);
}

/* $10ec LDQ indexed -**0- */
OP_HANDLER( ldq_ix )
{
	PAIR    q;

	fetch_effective_address(m68_state);
	q.d=RM32(m68_state, EAD);
	D = q.w.h;
	W = q.w.l;
	CLR_NZV;
	SET_N8(A);
	SET_Z(q.d);
}

/* $10aE LDY indexed -**0- */
OP_HANDLER( ldy_ix )
{
	fetch_effective_address(m68_state);
	Y=RM16(m68_state, EAD);
	CLR_NZV;
	SET_NZ16(Y);
}

/* $aF STX (STY) indexed -**0- */
OP_HANDLER( stx_ix )
{
	fetch_effective_address(m68_state);
	CLR_NZV;
	SET_NZ16(X);
	WM16(m68_state, EAD,&pX);
}

/* $10ed STQ indexed -**0- */
OP_HANDLER( stq_ix )
{
	PAIR    q;

	q.w.h = D;
	q.w.l = W;
	fetch_effective_address(m68_state);
	WM32(m68_state, EAD,&q);
	CLR_NZV;
	SET_N8(A);
	SET_Z(q.d);
}

/* $10aF STY indexed -**0- */
OP_HANDLER( sty_ix )
{
	fetch_effective_address(m68_state);
	CLR_NZV;
	SET_NZ16(Y);
	WM16(m68_state, EAD,&pY);
}

/* $b0 SUBA extended ?**** */
OP_HANDLER( suba_ex )
{
	UINT16    t,r;
	EXTBYTE(t);
	r = A - t;
	CLR_NZVC;
	SET_FLAGS8(A,t,r);
	A = r;
}

/* $b1 CMPA extended ?**** */
OP_HANDLER( cmpa_ex )
{
	UINT16    t,r;
	EXTBYTE(t);
	r = A - t;
	CLR_NZVC;
	SET_FLAGS8(A,t,r);
}

/* $b2 SBCA extended ?**** */
OP_HANDLER( sbca_ex )
{
	UINT16    t,r;
	EXTBYTE(t);
	r = A - t - (CC & CC_C);
	CLR_NZVC;
	SET_FLAGS8(A,t,r);
	A = r;
}

/* $b3 SUBD (CMPD CMPU) extended -**** */
OP_HANDLER( subd_ex )
{
	UINT32 r,d;
	PAIR b;
	EXTWORD(b);
	d = D;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
	D = r;
}

/* $10b0 SUBW extended -**** */
OP_HANDLER( subw_ex )
{
	UINT32 r,d;
	PAIR b = {{0,}};
	EXTWORD(b);
	d = W;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
	W = r;
}

/* $10b3 CMPD extended -**** */
OP_HANDLER( cmpd_ex )
{
	UINT32 r,d;
	PAIR b;
	EXTWORD(b);
	d = D;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
}

/* $10b1 CMPW extended -**** */
OP_HANDLER( cmpw_ex )
{
	UINT32 r,d;
	PAIR b = {{0,}};
	EXTWORD(b);
	d = W;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
}

/* $11b3 CMPU extended -**** */
OP_HANDLER( cmpu_ex )
{
	UINT32 r,d;
	PAIR b;
	EXTWORD(b);
	d = U;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
}

/* $b4 ANDA extended -**0- */
OP_HANDLER( anda_ex )
{
	UINT8 t;
	EXTBYTE(t);
	A &= t;
	CLR_NZV;
	SET_NZ8(A);
}

/* $b5 BITA extended -**0- */
OP_HANDLER( bita_ex )
{
	UINT8 t,r;
	EXTBYTE(t);
	r = A & t;
	CLR_NZV; SET_NZ8(r);
}

/* $b6 LDA extended -**0- */
OP_HANDLER( lda_ex )
{
	EXTBYTE(A);
	CLR_NZV;
	SET_NZ8(A);
}

/* $b7 STA extended -**0- */
OP_HANDLER( sta_ex )
{
	CLR_NZV;
	SET_NZ8(A);
	EXTENDED;
	WM(EAD,A);
}

/* $b8 EORA extended -**0- */
OP_HANDLER( eora_ex )
{
	UINT8 t;
	EXTBYTE(t);
	A ^= t;
	CLR_NZV;
	SET_NZ8(A);
}

/* $b9 ADCA extended ***** */
OP_HANDLER( adca_ex )
{
	UINT16 t,r;
	EXTBYTE(t);
	r = A + t + (CC & CC_C);
	CLR_HNZVC;
	SET_FLAGS8(A,t,r);
	SET_H(A,t,r);
	A = r;
}

/* $bA ORA extended -**0- */
OP_HANDLER( ora_ex )
{
	UINT8 t;
	EXTBYTE(t);
	A |= t;
	CLR_NZV;
	SET_NZ8(A);
}

/* $bB ADDA extended ***** */
OP_HANDLER( adda_ex )
{
	UINT16 t,r;
	EXTBYTE(t);
	r = A + t;
	CLR_HNZVC;
	SET_FLAGS8(A,t,r);
	SET_H(A,t,r);
	A = r;
}

/* $bC CMPX (CMPY CMPS) extended -**** */
OP_HANDLER( cmpx_ex )
{
	UINT32 r,d;
	PAIR b;
	EXTWORD(b);
	d = X;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
}

/* $10bC CMPY extended -**** */
OP_HANDLER( cmpy_ex )
{
	UINT32 r,d;
	PAIR b;
	EXTWORD(b);
	d = Y;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
}

/* $11bC CMPS extended -**** */
OP_HANDLER( cmps_ex )
{
	UINT32 r,d;
	PAIR b;
	EXTWORD(b);
	d = S;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
}

/* $bD JSR extended ----- */
OP_HANDLER( jsr_ex )
{
	EXTENDED;
	PUSHWORD(pPC);
	PCD = EAD;
}

/* $bE LDX (LDY) extended -**0- */
OP_HANDLER( ldx_ex )
{
	EXTWORD(pX);
	CLR_NZV;
	SET_NZ16(X);
}

/* $11bf MULD extended -**0- */
OP_HANDLER( muld_ex )
{
	PAIR    t, q;

	EXTWORD(t);
	q.d = (INT16) D * (INT16)t.w.l;

	D = q.w.h;
	W = q.w.l;
	CLR_NZVC;
	SET_NZ16(D);
}

/* $11bd DIVD extended -**0- */
OP_HANDLER( divd_ex )
{
	UINT8   t;
	INT16   v, oldD;

	EXTBYTE(t);

	if( t != 0 )
	{
		oldD = D;
		v = (INT16) D / (INT8) t;
		A = (INT16) D % (INT8) t;
		B = v;

		CLR_NZVC;
		SET_NZ8(B);

		if( B & 0x01 )
			SEC;

		if( (INT16)D < 0 )
			SEN;

		if ( (v > 127) || (v < -128) ) /* soft overflow */
		{
			SEV;

			if( (v > 255) || (v < -256) ) /* hard overflow - division is aborted */
			{
				SET_NZ16( oldD );
				D = abs( oldD );
			}
		}
	}
	else
	{
		m68_state->icount -= 8;
		DZError(m68_state);
	}
}

/* $11be DIVQ extended -**0- */
OP_HANDLER( divq_ex )
{
	PAIR    t,q, oldQ;
	INT32   v;

	EXTWORD(t);

	q.w.h = D;
	q.w.l = W;

	if( t.w.l != 0 )
	{
		oldQ = q;

		v = (INT32) q.d / (INT16) t.w.l;
		D = (INT32) q.d % (INT16) t.w.l;
		W = v;

		CLR_NZVC;
		SET_NZ16(W);

		if( W & 0x0001 )
			SEC;

		if ( (v > 32767) || (v < -32768) ) /* soft overflow */
		{
			SEV;

			if( (v > 65535 ) || (v < -65536 ) ) /* hard overflow - division is aborted */
			{
				if( (INT32)oldQ.d < 0 )
					SEN;
				else if( oldQ.d == 0 )
					SEZ;

				t.w.l = abs( t.w.l );
				D = oldQ.w.h;
				W = oldQ.w.l;
			}
		}
	}
	else
		DZError(m68_state);
}

/* $10fc LDQ extended -**0- */
OP_HANDLER( ldq_ex )
{
	PAIR    q;

	EXTLONG(q);
	D = q.w.h;
	W = q.w.l;
	CLR_NZV;
	SET_N8(A);
	SET_Z(q.d);
}

/* $10bE LDY extended -**0- */
OP_HANDLER( ldy_ex )
{
	EXTWORD(pY);
	CLR_NZV;
	SET_NZ16(Y);
}

/* $bF STX (STY) extended -**0- */
OP_HANDLER( stx_ex )
{
	CLR_NZV;
	SET_NZ16(X);
	EXTENDED;
	WM16(m68_state, EAD,&pX);
}

/* $10fd STQ extended -**0- */
OP_HANDLER( stq_ex )
{
	PAIR    q;

	q.w.h = D;
	q.w.l = W;
	EXTENDED;
	WM32(m68_state, EAD,&q);
	CLR_NZV;
	SET_N8(A);
	SET_Z(q.d);
}

/* $10bF STY extended -**0- */
OP_HANDLER( sty_ex )
{
	CLR_NZV;
	SET_NZ16(Y);
	EXTENDED;
	WM16(m68_state, EAD,&pY);
}

/* $c0 SUBB immediate ?**** */
OP_HANDLER( subb_im )
{
	UINT16    t,r;
	IMMBYTE(t);
	r = B - t;
	CLR_NZVC;
	SET_FLAGS8(B,t,r);
	B = r;
}

/* $1180 SUBE immediate ?**** */
OP_HANDLER( sube_im )
{
	UINT16    t,r;
	IMMBYTE(t);
	r = E - t;
	CLR_NZVC;
	SET_FLAGS8(E,t,r);
	E = r;
}

/* $11C0 SUBF immediate ?**** */
OP_HANDLER( subf_im )
{
	UINT16    t,r;
	IMMBYTE(t);
	r = F - t;
	CLR_NZVC;
	SET_FLAGS8(F,t,r);
	F = r;
}

/* $c1 CMPB immediate ?**** */
OP_HANDLER( cmpb_im )
{
	UINT16    t,r;
	IMMBYTE(t);
	r = B - t;
	CLR_NZVC; SET_FLAGS8(B,t,r);
}

/* $1181 CMPE immediate ?**** */
OP_HANDLER( cmpe_im )
{
	UINT16    t,r;
	IMMBYTE(t);
	r = E - t;
	CLR_NZVC; SET_FLAGS8(E,t,r);
}

/* $11C1 CMPF immediate ?**** */
OP_HANDLER( cmpf_im )
{
	UINT16    t,r;
	IMMBYTE(t);
	r = F - t;
	CLR_NZVC; SET_FLAGS8(F,t,r);
}

/* $c2 SBCB immediate ?**** */
OP_HANDLER( sbcb_im )
{
	UINT16    t,r;
	IMMBYTE(t);
	r = B - t - (CC & CC_C);
	CLR_NZVC;
	SET_FLAGS8(B,t,r);
	B = r;
}

/* $1082 SBCD immediate ?**** */
OP_HANDLER( sbcd_im )
{
	PAIR    t;
	UINT32   r;
	IMMWORD(t);
	r = D - t.w.l - (CC & CC_C);
	CLR_NZVC;
	SET_FLAGS16(D,t.w.l,r);
	D = r;
}

/* $c3 ADDD immediate -**** */
OP_HANDLER( addd_im )
{
	UINT32 r,d;
	PAIR b;
	IMMWORD(b);
	d = D;
	r = d + b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
	D = r;
}

/* $108b ADDW immediate -**** */
OP_HANDLER( addw_im )
{
	UINT32 r,d;
	PAIR b;
	IMMWORD(b);
	d = W;
	r = d + b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
	W = r;
}

/* $118b ADDE immediate -**** */
OP_HANDLER( adde_im )
{
	UINT16 t,r;
	IMMBYTE(t);
	r = E + t;
	CLR_HNZVC;
	SET_FLAGS8(E,t,r);
	SET_H(E,t,r);
	E = r;
}

/* $11Cb ADDF immediate -**** */
OP_HANDLER( addf_im )
{
	UINT16 t,r;
	IMMBYTE(t);
	r = F + t;
	CLR_HNZVC;
	SET_FLAGS8(F,t,r);
	SET_H(F,t,r);
	F = r;
}

/* $c4 ANDB immediate -**0- */
OP_HANDLER( andb_im )
{
	UINT8 t;
	IMMBYTE(t);
	B &= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $1084 ANDD immediate -**0- */
OP_HANDLER( andd_im )
{
	PAIR t;
	IMMWORD(t);
	D &= t.w.l;
	CLR_NZV;
	SET_NZ16(D);
}

/* $c5 BITB immediate -**0- */
OP_HANDLER( bitb_im )
{
	UINT8 t,r;
	IMMBYTE(t);
	r = B & t;
	CLR_NZV;
	SET_NZ8(r);
}

/* $1085 BITD immediate -**0- */
OP_HANDLER( bitd_im )
{
	PAIR    t;
	UINT16  r;
	IMMWORD(t);
	r = B & t.w.l;
	CLR_NZV;
	SET_NZ16(r);
}

/* $113c BITMD immediate -**0- */
OP_HANDLER( bitmd_im )
{
	/*
	The following is from Darren A.

	The Z flag is the only condition code that should be affected by BITMD.
	For example, when the "Divide-By-Zero" flag (bit 7) is set, BITMD should
	not set the N flag. It should also NOT clear the V flag (unlike the other
	BIT instructions).

	His comments come from experimentation and differ from Chris Burke
	*/

	UINT8 t,r;
	IMMBYTE(t);
	r = MD & t;
	CLR_Z;
	SET_Z8(r);

	MD &= ~(r & 0xc0); /* clear the tested high bits */

}

/* $c6 LDB immediate -**0- */
OP_HANDLER( ldb_im )
{
	IMMBYTE(B);
	CLR_NZV;
	SET_NZ8(B);
}

/* $113d LDMD immediate -**0- */
OP_HANDLER( ldmd_im )
{
	IMMBYTE(MD);
	UpdateState(m68_state);
}

/* $1186 LDE immediate -**0- */
OP_HANDLER( lde_im )
{
	IMMBYTE(E);
	CLR_NZV;
	SET_NZ8(E);
}

/* $11C6 LDF immediate -**0- */
OP_HANDLER( ldf_im )
{
	IMMBYTE(F);
	CLR_NZV;
	SET_NZ8(F);
}

/* $c8 EORB immediate -**0- */
OP_HANDLER( eorb_im )
{
	UINT8 t;
	IMMBYTE(t);
	B ^= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $1088 EORD immediate -**0- */
OP_HANDLER( eord_im )
{
	PAIR t;
	IMMWORD(t);
	D ^= t.w.l;
	CLR_NZV;
	SET_NZ16(D);
}

/* $c9 ADCB immediate ***** */
OP_HANDLER( adcb_im )
{
	UINT16 t,r;
	IMMBYTE(t);
	r = B + t + (CC & CC_C);
	CLR_HNZVC;
	SET_FLAGS8(B,t,r);
	SET_H(B,t,r);
	B = r;
}

/* $1089 ADCD immediate ***** */
OP_HANDLER( adcd_im )
{
	PAIR    t;
	UINT32  r;
	IMMWORD(t);
	r = D + t.w.l + (CC & CC_C);
	CLR_NZVC;
	SET_FLAGS16(D,t.w.l,r);
	D = r;
}

/* $cA ORB immediate -**0- */
OP_HANDLER( orb_im )
{
	UINT8 t;
	IMMBYTE(t);
	B |= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $108a ORD immediate -**0- */
OP_HANDLER( ord_im )
{
	PAIR t;
	IMMWORD(t);
	D |= t.w.l;
	CLR_NZV;
	SET_NZ16(D);
}

/* $cB ADDB immediate ***** */
OP_HANDLER( addb_im )
{
	UINT16 t,r;
	IMMBYTE(t);
	r = B + t;
	CLR_HNZVC;
	SET_FLAGS8(B,t,r);
	SET_H(B,t,r);
	B = r;
}

/* $cC LDD immediate -**0- */
OP_HANDLER( ldd_im )
{
	PAIR    t;

	IMMWORD(t);
	D=t.w.l;
	CLR_NZV;
	SET_NZ16(D);
}

/* $1086 LDW immediate -**0- */
OP_HANDLER( ldw_im )
{
	PAIR    t;
	IMMWORD(t);
	W=t.w.l;
	CLR_NZV;
	SET_NZ16(W);
}

/* $cE LDU (LDS) immediate -**0- */
OP_HANDLER( ldu_im )
{
	IMMWORD(pU);
	CLR_NZV;
	SET_NZ16(U);
}

/* $10cE LDS immediate -**0- */
OP_HANDLER( lds_im )
{
	IMMWORD(pS);
	CLR_NZV;
	SET_NZ16(S);
	m68_state->int_state |= M6809_LDS;
}

/* $d0 SUBB direct ?**** */
OP_HANDLER( subb_di )
{
	UINT16    t,r;
	DIRBYTE(t);
	r = B - t;
	CLR_NZVC;
	SET_FLAGS8(B,t,r);
	B = r;
}

/* $1190 SUBE direct ?**** */
OP_HANDLER( sube_di )
{
	UINT16    t,r;
	DIRBYTE(t);
	r = E - t;
	CLR_NZVC;
	SET_FLAGS8(E,t,r);
	E = r;
}

/* $11d0 SUBF direct ?**** */
OP_HANDLER( subf_di )
{
	UINT16    t,r;
	DIRBYTE(t);
	r = F - t;
	CLR_NZVC;
	SET_FLAGS8(F,t,r);
	F = r;
}

/* $d1 CMPB direct ?**** */
OP_HANDLER( cmpb_di )
{
	UINT16    t,r;
	DIRBYTE(t);
	r = B - t;
	CLR_NZVC;
	SET_FLAGS8(B,t,r);
}

/* $1191 CMPE direct ?**** */
OP_HANDLER( cmpe_di )
{
	UINT16    t,r;
	DIRBYTE(t);
	r = E - t;
	CLR_NZVC;
	SET_FLAGS8(E,t,r);
}

/* $11D1 CMPF direct ?**** */
OP_HANDLER( cmpf_di )
{
	UINT16    t,r;
	DIRBYTE(t);
	r = F - t;
	CLR_NZVC;
	SET_FLAGS8(F,t,r);
}

/* $d2 SBCB direct ?**** */
OP_HANDLER( sbcb_di )
{
	UINT16    t,r;
	DIRBYTE(t);
	r = B - t - (CC & CC_C);
	CLR_NZVC;
	SET_FLAGS8(B,t,r);
	B = r;
}

/* $1092 SBCD direct ?**** */
OP_HANDLER( sbcd_di )
{
	PAIR    t;
	UINT32  r;
	DIRWORD(t);
	r = D - t.w.l - (CC & CC_C);
	CLR_NZVC;
	SET_FLAGS16(D,t.w.l,r);
	D = r;
}

/* $d3 ADDD direct -**** */
OP_HANDLER( addd_di )
{
	UINT32 r,d;
	PAIR b;
	DIRWORD(b);
	d = D;
	r = d + b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
	D = r;
}

/* $109b ADDW direct -**** */
OP_HANDLER( addw_di )
{
	UINT32 r,d;
	PAIR b;
	DIRWORD(b);
	d = W;
	r = d + b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
	W = r;
}

/* $119b ADDE direct -**** */
OP_HANDLER( adde_di )
{
	UINT16 t,r;
	DIRBYTE(t);
	r = E + t;
	CLR_HNZVC;
	SET_FLAGS8(E,t,r);
	SET_H(E,t,r);
	E = r;
}

/* $11db ADDF direct -**** */
OP_HANDLER( addf_di )
{
	UINT16 t,r;
	DIRBYTE(t);
	r = F + t;
	CLR_HNZVC;
	SET_FLAGS8(F,t,r);
	SET_H(F,t,r);
	F = r;
}

/* $d4 ANDB direct -**0- */
OP_HANDLER( andb_di )
{
	UINT8 t;
	DIRBYTE(t);
	B &= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $1094 ANDD direct -**0- */
OP_HANDLER( andd_di )
{
	PAIR t;
	DIRWORD(t);
	D &= t.w.l;
	CLR_NZV;
	SET_NZ16(D);
}

/* $d5 BITB direct -**0- */
OP_HANDLER( bitb_di )
{
	UINT8 t,r;
	DIRBYTE(t);
	r = B & t;
	CLR_NZV;
	SET_NZ8(r);
}

/* $1095 BITD direct -**0- */
OP_HANDLER( bitd_di )
{
	PAIR    t;
	UINT16  r;
	DIRWORD(t);
	r = B & t.w.l;
	CLR_NZV;
	SET_NZ16(r);
}

/* $d6 LDB direct -**0- */
OP_HANDLER( ldb_di )
{
	DIRBYTE(B);
	CLR_NZV;
	SET_NZ8(B);
}

/* $1196 LDE direct -**0- */
OP_HANDLER( lde_di )
{
	DIRBYTE(E);
	CLR_NZV;
	SET_NZ8(E);
}

/* $11d6 LDF direct -**0- */
OP_HANDLER( ldf_di )
{
	DIRBYTE(F);
	CLR_NZV;
	SET_NZ8(F);
}

/* $d7 STB direct -**0- */
OP_HANDLER( stb_di )
{
	CLR_NZV;
	SET_NZ8(B);
	DIRECT;
	WM(EAD,B);
}

/* $1197 STE direct -**0- */
OP_HANDLER( ste_di )
{
	CLR_NZV;
	SET_NZ8(E);
	DIRECT;
	WM(EAD,E);
}

/* $11D7 STF direct -**0- */
OP_HANDLER( stf_di )
{
	CLR_NZV;
	SET_NZ8(F);
	DIRECT;
	WM(EAD,F);
}

/* $d8 EORB direct -**0- */
OP_HANDLER( eorb_di )
{
	UINT8 t;
	DIRBYTE(t);
	B ^= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $1098 EORD direct -**0- */
OP_HANDLER( eord_di )
{
	PAIR t;
	DIRWORD(t);
	D ^= t.w.l;
	CLR_NZV;
	SET_NZ16(D);
}

/* $d9 ADCB direct ***** */
OP_HANDLER( adcb_di )
{
	UINT16 t,r;
	DIRBYTE(t);
	r = B + t + (CC & CC_C);
	CLR_HNZVC;
	SET_FLAGS8(B,t,r);
	SET_H(B,t,r);
	B = r;
}

/* $1099 adcd direct ***** */
OP_HANDLER( adcd_di )
{
	UINT32  r;
	PAIR    t;

	DIRWORD(t);
	r = D + t.w.l + (CC & CC_C);
	CLR_NZVC;
	SET_FLAGS16(D,t.w.l,r);
	D = r;
}

/* $dA ORB direct -**0- */
OP_HANDLER( orb_di )
{
	UINT8 t;
	DIRBYTE(t);
	B |= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $109a ORD direct -**0- */
OP_HANDLER( ord_di )
{
	PAIR t;
	DIRWORD(t);
	D |= t.w.l;
	CLR_NZV;
	SET_NZ16(D);
}

/* $dB ADDB direct ***** */
OP_HANDLER( addb_di )
{
	UINT16 t,r;
	DIRBYTE(t);
	r = B + t;
	CLR_HNZVC;
	SET_FLAGS8(B,t,r);
	SET_H(B,t,r);
	B = r;
}

/* $dC LDD direct -**0- */
OP_HANDLER( ldd_di )
{
	PAIR t;
	DIRWORD(t);
	D=t.w.l;
	CLR_NZV;
	SET_NZ16(D);
}

/* $1096 LDW direct -**0- */
OP_HANDLER( ldw_di )
{
	PAIR t;
	DIRWORD(t);
	W=t.w.l;
	CLR_NZV;
	SET_NZ16(W);
}

/* $dD STD direct -**0- */
OP_HANDLER( std_di )
{
	CLR_NZV;
	SET_NZ16(D);
	DIRECT;
	WM16(m68_state, EAD,&pD);
}

/* $1097 STW direct -**0- */
OP_HANDLER( stw_di )
{
	CLR_NZV;
	SET_NZ16(W);
	DIRECT;
	WM16(m68_state, EAD,&pW);
}

/* $dE LDU (LDS) direct -**0- */
OP_HANDLER( ldu_di )
{
	DIRWORD(pU);
	CLR_NZV;
	SET_NZ16(U);
}

/* $10dE LDS direct -**0- */
OP_HANDLER( lds_di )
{
	DIRWORD(pS);
	CLR_NZV;
	SET_NZ16(S);
	m68_state->int_state |= M6809_LDS;
}

/* $dF STU (STS) direct -**0- */
OP_HANDLER( stu_di )
{
	CLR_NZV;
	SET_NZ16(U);
	DIRECT;
	WM16(m68_state, EAD,&pU);
}

/* $10dF STS direct -**0- */
OP_HANDLER( sts_di )
{
	CLR_NZV;
	SET_NZ16(S);
	DIRECT;
	WM16(m68_state, EAD,&pS);
}

/* $e0 SUBB indexed ?**** */
OP_HANDLER( subb_ix )
{
	UINT16    t,r;
	fetch_effective_address(m68_state);
	t = RM(EAD);
	r = B - t;
	CLR_NZVC;
	SET_FLAGS8(B,t,r);
	B = r;
}

/* $11a0 SUBE indexed ?**** */
OP_HANDLER( sube_ix )
{
	UINT16    t,r;
	fetch_effective_address(m68_state);
	t = RM(EAD);
	r = E - t;
	CLR_NZVC;
	SET_FLAGS8(E,t,r);
	E = r;
}

/* $11e0 SUBF indexed ?**** */
OP_HANDLER( subf_ix )
{
	UINT16    t,r;
	fetch_effective_address(m68_state);
	t = RM(EAD);
	r = F - t;
	CLR_NZVC;
	SET_FLAGS8(F,t,r);
	F = r;
}

/* $e1 CMPB indexed ?**** */
OP_HANDLER( cmpb_ix )
{
	UINT16    t,r;
	fetch_effective_address(m68_state);
	t = RM(EAD);
	r = B - t;
	CLR_NZVC;
	SET_FLAGS8(B,t,r);
}

/* $11a1 CMPE indexed ?**** */
OP_HANDLER( cmpe_ix )
{
	UINT16    t,r;
	fetch_effective_address(m68_state);
	t = RM(EAD);
	r = E - t;
	CLR_NZVC;
	SET_FLAGS8(E,t,r);
}

/* $11e1 CMPF indexed ?**** */
OP_HANDLER( cmpf_ix )
{
	UINT16    t,r;
	fetch_effective_address(m68_state);
	t = RM(EAD);
	r = F - t;
	CLR_NZVC;
	SET_FLAGS8(F,t,r);
}

/* $e2 SBCB indexed ?**** */
OP_HANDLER( sbcb_ix )
{
	UINT16    t,r;
	fetch_effective_address(m68_state);
	t = RM(EAD);
	r = B - t - (CC & CC_C);
	CLR_NZVC;
	SET_FLAGS8(B,t,r);
	B = r;
}

/* $10a2 SBCD indexed ?**** */
OP_HANDLER( sbcd_ix )
{
	UINT32    t,r;
	fetch_effective_address(m68_state);
	t = RM16(m68_state, EAD);
	r = D - t - (CC & CC_C);
	CLR_NZVC;
	SET_FLAGS16(D,t,r);
	D = r;
}

/* $e3 ADDD indexed -**** */
OP_HANDLER( addd_ix )
{
	UINT32 r,d;
	PAIR b;
	fetch_effective_address(m68_state);
	b.d=RM16(m68_state, EAD);
	d = D;
	r = d + b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
	D = r;
}

/* $10ab ADDW indexed -**** */
OP_HANDLER( addw_ix )
{
	UINT32 r,d;
	PAIR b;
	fetch_effective_address(m68_state);
	b.d=RM16(m68_state, EAD);
	d = W;
	r = d + b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
	W = r;
}

/* $11ab ADDE indexed -**** */
OP_HANDLER( adde_ix )
{
	UINT16 t,r;
	fetch_effective_address(m68_state);
	t = RM(EAD);
	r = E + t;
	CLR_HNZVC;
	SET_FLAGS8(E,t,r);
	SET_H(E,t,r);
	E = r;
}

/* $11eb ADDF indexed -**** */
OP_HANDLER( addf_ix )
{
	UINT16 t,r;
	fetch_effective_address(m68_state);
	t = RM(EAD);
	r = F + t;
	CLR_HNZVC;
	SET_FLAGS8(F,t,r);
	SET_H(F,t,r);
	F = r;
}

/* $e4 ANDB indexed -**0- */
OP_HANDLER( andb_ix )
{
	fetch_effective_address(m68_state);
	B &= RM(EAD);
	CLR_NZV;
	SET_NZ8(B);
}

/* $10a4 ANDD indexed -**0- */
OP_HANDLER( andd_ix )
{
	fetch_effective_address(m68_state);
	D &= RM16(m68_state, EAD);
	CLR_NZV;
	SET_NZ16(D);
}

/* $e5 BITB indexed -**0- */
OP_HANDLER( bitb_ix )
{
	UINT8 r;
	fetch_effective_address(m68_state);
	r = B & RM(EAD);
	CLR_NZV;
	SET_NZ8(r);
}

/* $10a5 BITD indexed -**0- */
OP_HANDLER( bitd_ix )
{
	UINT16 r;
	fetch_effective_address(m68_state);
	r = D & RM16(m68_state, EAD);
	CLR_NZV;
	SET_NZ16(r);
}

/* $e6 LDB indexed -**0- */
OP_HANDLER( ldb_ix )
{
	fetch_effective_address(m68_state);
	B = RM(EAD);
	CLR_NZV;
	SET_NZ8(B);
}

/* $11a6 LDE indexed -**0- */
OP_HANDLER( lde_ix )
{
	fetch_effective_address(m68_state);
	E = RM(EAD);
	CLR_NZV;
	SET_NZ8(E);
}

/* $11e6 LDF indexed -**0- */
OP_HANDLER( ldf_ix )
{
	fetch_effective_address(m68_state);
	F = RM(EAD);
	CLR_NZV;
	SET_NZ8(F);
}

/* $e7 STB indexed -**0- */
OP_HANDLER( stb_ix )
{
	fetch_effective_address(m68_state);
	CLR_NZV;
	SET_NZ8(B);
	WM(EAD,B);
}

/* $11a7 STE indexed -**0- */
OP_HANDLER( ste_ix )
{
	fetch_effective_address(m68_state);
	CLR_NZV;
	SET_NZ8(E);
	WM(EAD,E);
}

/* $11e7 STF indexed -**0- */
OP_HANDLER( stf_ix )
{
	fetch_effective_address(m68_state);
	CLR_NZV;
	SET_NZ8(F);
	WM(EAD,F);
}

/* $e8 EORB indexed -**0- */
OP_HANDLER( eorb_ix )
{
	fetch_effective_address(m68_state);
	B ^= RM(EAD);
	CLR_NZV;
	SET_NZ8(B);
}

/* $10a8 EORD indexed -**0- */
OP_HANDLER( eord_ix )
{
	fetch_effective_address(m68_state);
	D ^= RM16(m68_state, EAD);
	CLR_NZV;
	SET_NZ16(D);
}

/* $e9 ADCB indexed ***** */
OP_HANDLER( adcb_ix )
{
	UINT16 t,r;
	fetch_effective_address(m68_state);
	t = RM(EAD);
	r = B + t + (CC & CC_C);
	CLR_HNZVC;
	SET_FLAGS8(B,t,r);
	SET_H(B,t,r);
	B = r;
}

/* $10a9 ADCD indexed ***** */
OP_HANDLER( adcd_ix )
{
	UINT32  r;
	PAIR    t;
	fetch_effective_address(m68_state);
	t.d = RM16(m68_state, EAD);
	r = D + t.d + (CC & CC_C);
	CLR_NZVC;
	SET_FLAGS16(D,t.d,r);
	D = r;
}

/* $eA ORB indexed -**0- */
OP_HANDLER( orb_ix )
{
	fetch_effective_address(m68_state);
	B |= RM(EAD);
	CLR_NZV;
	SET_NZ8(B);
}

/* $10aa ORD indexed -**0- */
OP_HANDLER( ord_ix )
{
	fetch_effective_address(m68_state);
	D |= RM16(m68_state, EAD);
	CLR_NZV;
	SET_NZ16(D);
}

/* $eB ADDB indexed ***** */
OP_HANDLER( addb_ix )
{
	UINT16 t,r;
	fetch_effective_address(m68_state);
	t = RM(EAD);
	r = B + t;
	CLR_HNZVC;
	SET_FLAGS8(B,t,r);
	SET_H(B,t,r);
	B = r;
}

/* $eC LDD indexed -**0- */
OP_HANDLER( ldd_ix )
{
	fetch_effective_address(m68_state);
	D=RM16(m68_state, EAD);
	CLR_NZV; SET_NZ16(D);
}

/* $10a6 LDW indexed -**0- */
OP_HANDLER( ldw_ix )
{
	fetch_effective_address(m68_state);
	W=RM16(m68_state, EAD);
	CLR_NZV; SET_NZ16(W);
}

/* $eD STD indexed -**0- */
OP_HANDLER( std_ix )
{
	fetch_effective_address(m68_state);
	CLR_NZV;
	SET_NZ16(D);
	WM16(m68_state, EAD,&pD);
}

/* $10a7 STW indexed -**0- */
OP_HANDLER( stw_ix )
{
	fetch_effective_address(m68_state);
	CLR_NZV;
	SET_NZ16(W);
	WM16(m68_state, EAD,&pW);
}

/* $eE LDU (LDS) indexed -**0- */
OP_HANDLER( ldu_ix )
{
	fetch_effective_address(m68_state);
	U=RM16(m68_state, EAD);
	CLR_NZV;
	SET_NZ16(U);
}

/* $10eE LDS indexed -**0- */
OP_HANDLER( lds_ix )
{
	fetch_effective_address(m68_state);
	S=RM16(m68_state, EAD);
	CLR_NZV;
	SET_NZ16(S);
	m68_state->int_state |= M6809_LDS;
}

/* $eF STU (STS) indexed -**0- */
OP_HANDLER( stu_ix )
{
	fetch_effective_address(m68_state);
	CLR_NZV;
	SET_NZ16(U);
	WM16(m68_state, EAD,&pU);
}

/* $10eF STS indexed -**0- */
OP_HANDLER( sts_ix )
{
	fetch_effective_address(m68_state);
	CLR_NZV;
	SET_NZ16(S);
	WM16(m68_state, EAD,&pS);
}

/* $f0 SUBB extended ?**** */
OP_HANDLER( subb_ex )
{
	UINT16    t,r;
	EXTBYTE(t);
	r = B - t;
	CLR_NZVC;
	SET_FLAGS8(B,t,r);
	B = r;
}

/* $11b0 SUBE extended ?**** */
OP_HANDLER( sube_ex )
{
	UINT16    t,r;
	EXTBYTE(t);
	r = E - t;
	CLR_NZVC;
	SET_FLAGS8(E,t,r);
	E = r;
}

/* $11f0 SUBF extended ?**** */
OP_HANDLER( subf_ex )
{
	UINT16    t,r;
	EXTBYTE(t);
	r = F - t;
	CLR_NZVC;
	SET_FLAGS8(F,t,r);
	F = r;
}

/* $f1 CMPB extended ?**** */
OP_HANDLER( cmpb_ex )
{
	UINT16    t,r;
	EXTBYTE(t);
	r = B - t;
	CLR_NZVC;
	SET_FLAGS8(B,t,r);
}

/* $11b1 CMPE extended ?**** */
OP_HANDLER( cmpe_ex )
{
	UINT16    t,r;
	EXTBYTE(t);
	r = E - t;
	CLR_NZVC;
	SET_FLAGS8(E,t,r);
}

/* $11f1 CMPF extended ?**** */
OP_HANDLER( cmpf_ex )
{
	UINT16    t,r;
	EXTBYTE(t);
	r = F - t;
	CLR_NZVC;
	SET_FLAGS8(F,t,r);
}

/* $f2 SBCB extended ?**** */
OP_HANDLER( sbcb_ex )
{
	UINT16    t,r;
	EXTBYTE(t);
	r = B - t - (CC & CC_C);
	CLR_NZVC;
	SET_FLAGS8(B,t,r);
	B = r;
}

/* $10b2 SBCD extended ?**** */
OP_HANDLER( sbcd_ex )
{
	PAIR t = {{0,}};
	UINT32 r;

	EXTWORD(t);
	r = D - t.w.l - (CC & CC_C);
	CLR_NZVC;
	SET_FLAGS16(D,t.w.l,r);
	D = r;
}

/* $f3 ADDD extended -**** */
OP_HANDLER( addd_ex )
{
	UINT32 r,d;
	PAIR b;
	EXTWORD(b);
	d = D;
	r = d + b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
	D = r;
}

/* $10bb ADDW extended -**** */
OP_HANDLER( addw_ex )
{
	UINT32 r,d;
	PAIR b = {{0,}};
	EXTWORD(b);
	d = W;
	r = d + b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
	W = r;
}

/* $11bb ADDE extended -**** */
OP_HANDLER( adde_ex )
{
	UINT16 t,r;
	EXTBYTE(t);
	r = E + t;
	CLR_HNZVC;
	SET_FLAGS8(E,t,r);
	SET_H(E,t,r);
	E = r;
}

/* $11fb ADDF extended -**** */
OP_HANDLER( addf_ex )
{
	UINT16 t,r;
	EXTBYTE(t);
	r = F + t;
	CLR_HNZVC;
	SET_FLAGS8(F,t,r);
	SET_H(F,t,r);
	F = r;
}

/* $f4 ANDB extended -**0- */
OP_HANDLER( andb_ex )
{
	UINT8 t;
	EXTBYTE(t);
	B &= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $10b4 ANDD extended -**0- */
OP_HANDLER( andd_ex )
{
	PAIR t = {{0,}};
	EXTWORD(t);
	D &= t.w.l;
	CLR_NZV;
	SET_NZ16(D);
}

/* $f5 BITB extended -**0- */
OP_HANDLER( bitb_ex )
{
	UINT8 t,r;
	EXTBYTE(t);
	r = B & t;
	CLR_NZV;
	SET_NZ8(r);
}

/* $10b5 BITD extended -**0- */
OP_HANDLER( bitd_ex )
{
	PAIR t = {{0,}};
	UINT8 r;
	EXTWORD(t);
	r = B & t.w.l;
	CLR_NZV;
	SET_NZ16(r);
}

/* $f6 LDB extended -**0- */
OP_HANDLER( ldb_ex )
{
	EXTBYTE(B);
	CLR_NZV;
	SET_NZ8(B);
}

/* $11b6 LDE extended -**0- */
OP_HANDLER( lde_ex )
{
	EXTBYTE(E);
	CLR_NZV;
	SET_NZ8(E);
}

/* $11f6 LDF extended -**0- */
OP_HANDLER( ldf_ex )
{
	EXTBYTE(F);
	CLR_NZV;
	SET_NZ8(F);
}

/* $f7 STB extended -**0- */
OP_HANDLER( stb_ex )
{
	CLR_NZV;
	SET_NZ8(B);
	EXTENDED;
	WM(EAD,B);
}

/* $11b7 STE extended -**0- */
OP_HANDLER( ste_ex )
{
	CLR_NZV;
	SET_NZ8(E);
	EXTENDED;
	WM(EAD,E);
}

/* $11f7 STF extended -**0- */
OP_HANDLER( stf_ex )
{
	CLR_NZV;
	SET_NZ8(F);
	EXTENDED;
	WM(EAD,F);
}

/* $f8 EORB extended -**0- */
OP_HANDLER( eorb_ex )
{
	UINT8 t;
	EXTBYTE(t);
	B ^= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $10b8 EORD extended -**0- */
OP_HANDLER( eord_ex )
{
	PAIR t = {{0,}};
	EXTWORD(t);
	D ^= t.w.l;
	CLR_NZV;
	SET_NZ16(D);
}

/* $f9 ADCB extended ***** */
OP_HANDLER( adcb_ex )
{
	UINT16 t,r;
	EXTBYTE(t);
	r = B + t + (CC & CC_C);
	CLR_HNZVC;
	SET_FLAGS8(B,t,r);
	SET_H(B,t,r);
	B = r;
}

/* $10b9 ADCD extended ***** */
OP_HANDLER( adcd_ex )
{
	UINT32  r;
	PAIR    t;
	EXTWORD(t);
	r = D + t.w.l + (CC & CC_C);
	CLR_NZVC;
	SET_FLAGS16(D,t.w.l,r);
	D = r;
}

/* $fA ORB extended -**0- */
OP_HANDLER( orb_ex )
{
	UINT8 t;
	EXTBYTE(t);
	B |= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $10ba ORD extended -**0- */
OP_HANDLER( ord_ex )
{
	PAIR t = {{0,}};
	EXTWORD(t);
	D |= t.w.l;
	CLR_NZV;
	SET_NZ8(D);
}

/* $fB ADDB extended ***** */
OP_HANDLER( addb_ex )
{
	UINT16 t,r;
	EXTBYTE(t);
	r = B + t;
	CLR_HNZVC;
	SET_FLAGS8(B,t,r);
	SET_H(B,t,r);
	B = r;
}

/* $fC LDD extended -**0- */
OP_HANDLER( ldd_ex )
{
	EXTWORD(pD);
	CLR_NZV;
	SET_NZ16(D);
}

/* $10b6 LDW extended -**0- */
OP_HANDLER( ldw_ex )
{
	EXTWORD(pW);
	CLR_NZV;
	SET_NZ16(W);
}

/* $fD STD extended -**0- */
OP_HANDLER( std_ex )
{
	CLR_NZV;
	SET_NZ16(D);
	EXTENDED;
	WM16(m68_state, EAD,&pD);
}

/* $10b7 STW extended -**0- */
OP_HANDLER( stw_ex )
{
	CLR_NZV;
	SET_NZ16(W);
	EXTENDED;
	WM16(m68_state, EAD,&pW);
}

/* $fE LDU (LDS) extended -**0- */
OP_HANDLER( ldu_ex )
{
	EXTWORD(pU);
	CLR_NZV;
	SET_NZ16(U);
}

/* $10fE LDS extended -**0- */
OP_HANDLER( lds_ex )
{
	EXTWORD(pS);
	CLR_NZV;
	SET_NZ16(S);
	m68_state->int_state |= M6809_LDS;
}

/* $fF STU (STS) extended -**0- */
OP_HANDLER( stu_ex )
{
	CLR_NZV;
	SET_NZ16(U);
	EXTENDED;
	WM16(m68_state, EAD,&pU);
}

/* $10fF STS extended -**0- */
OP_HANDLER( sts_ex )
{
	CLR_NZV;
	SET_NZ16(S);
	EXTENDED;
	WM16(m68_state, EAD,&pS);
}

/* $10xx opcodes */
OP_HANDLER( pref10 )
{
	UINT8 ireg2 = ROP(PCD);
	PC++;

#if BIG_SWITCH
	switch( ireg2 )
	{
		case 0x21: lbrn(m68_state);         break;
		case 0x22: lbhi(m68_state);         break;
		case 0x23: lbls(m68_state);         break;
		case 0x24: lbcc(m68_state);         break;
		case 0x25: lbcs(m68_state);         break;
		case 0x26: lbne(m68_state);         break;
		case 0x27: lbeq(m68_state);         break;
		case 0x28: lbvc(m68_state);         break;
		case 0x29: lbvs(m68_state);         break;
		case 0x2a: lbpl(m68_state);         break;
		case 0x2b: lbmi(m68_state);         break;
		case 0x2c: lbge(m68_state);         break;
		case 0x2d: lblt(m68_state);         break;
		case 0x2e: lbgt(m68_state);         break;
		case 0x2f: lble(m68_state);         break;

		case 0x30: addr_r(m68_state);       break;
		case 0x31: adcr(m68_state);         break;
		case 0x32: subr(m68_state);         break;
		case 0x33: sbcr(m68_state);         break;
		case 0x34: andr(m68_state);         break;
		case 0x35: orr(m68_state);          break;
		case 0x36: eorr(m68_state);         break;
		case 0x37: cmpr(m68_state);         break;
		case 0x38: pshsw(m68_state);        break;
		case 0x39: pulsw(m68_state);        break;
		case 0x3a: pshuw(m68_state);        break;
		case 0x3b: puluw(m68_state);        break;
		case 0x3f: swi2(m68_state);         break;

		case 0x40: negd(m68_state);         break;
		case 0x43: comd(m68_state);         break;
		case 0x44: lsrd(m68_state);         break;
		case 0x46: rord(m68_state);         break;
		case 0x47: asrd(m68_state);         break;
		case 0x48: asld(m68_state);         break;
		case 0x49: rold(m68_state);         break;
		case 0x4a: decd(m68_state);         break;
		case 0x4c: incd(m68_state);         break;
		case 0x4d: tstd(m68_state);         break;
		case 0x4f: clrd(m68_state);         break;

		case 0x53: comw(m68_state);         break;
		case 0x54: lsrw(m68_state);         break;
		case 0x56: rorw(m68_state);         break;
		case 0x59: rolw(m68_state);         break;
		case 0x5a: decw(m68_state);         break;
		case 0x5c: incw(m68_state);         break;
		case 0x5d: tstw(m68_state);         break;
		case 0x5f: clrw(m68_state);         break;

		case 0x80: subw_im(m68_state);      break;
		case 0x81: cmpw_im(m68_state);      break;
		case 0x82: sbcd_im(m68_state);      break;
		case 0x83: cmpd_im(m68_state);      break;
		case 0x84: andd_im(m68_state);      break;
		case 0x85: bitd_im(m68_state);      break;
		case 0x86: ldw_im(m68_state);       break;
		case 0x88: eord_im(m68_state);      break;
		case 0x89: adcd_im(m68_state);      break;
		case 0x8a: ord_im(m68_state);       break;
		case 0x8b: addw_im(m68_state);      break;
		case 0x8c: cmpy_im(m68_state);      break;
		case 0x8e: ldy_im(m68_state);       break;

		case 0x90: subw_di(m68_state);      break;
		case 0x91: cmpw_di(m68_state);      break;
		case 0x92: sbcd_di(m68_state);      break;
		case 0x93: cmpd_di(m68_state);      break;
		case 0x94: andd_di(m68_state);      break;
		case 0x95: bitd_di(m68_state);      break;
		case 0x96: ldw_di(m68_state);       break;
		case 0x97: stw_di(m68_state);       break;
		case 0x98: eord_di(m68_state);      break;
		case 0x99: adcd_di(m68_state);      break;
		case 0x9a: ord_di(m68_state);       break;
		case 0x9b: addw_di(m68_state);      break;
		case 0x9c: cmpy_di(m68_state);      break;
		case 0x9e: ldy_di(m68_state);       break;
		case 0x9f: sty_di(m68_state);       break;

		case 0xa0: subw_ix(m68_state);      break;
		case 0xa1: cmpw_ix(m68_state);      break;
		case 0xa2: sbcd_ix(m68_state);      break;
		case 0xa3: cmpd_ix(m68_state);      break;
		case 0xa4: andd_ix(m68_state);      break;
		case 0xa5: bitd_ix(m68_state);      break;
		case 0xa6: ldw_ix(m68_state);       break;
		case 0xa7: stw_ix(m68_state);       break;
		case 0xa8: eord_ix(m68_state);      break;
		case 0xa9: adcd_ix(m68_state);      break;
		case 0xaa: ord_ix(m68_state);       break;
		case 0xab: addw_ix(m68_state);      break;
		case 0xac: cmpy_ix(m68_state);      break;
		case 0xae: ldy_ix(m68_state);       break;
		case 0xaf: sty_ix(m68_state);       break;

		case 0xb0: subw_ex(m68_state);      break;
		case 0xb1: cmpw_ex(m68_state);      break;
		case 0xb2: sbcd_ex(m68_state);      break;
		case 0xb3: cmpd_ex(m68_state);      break;
		case 0xb4: andd_ex(m68_state);      break;
		case 0xb5: bitd_ex(m68_state);      break;
		case 0xb6: ldw_ex(m68_state);       break;
		case 0xb7: stw_ex(m68_state);       break;
		case 0xb8: eord_ex(m68_state);      break;
		case 0xb9: adcd_ex(m68_state);      break;
		case 0xba: ord_ex(m68_state);       break;
		case 0xbb: addw_ex(m68_state);      break;
		case 0xbc: cmpy_ex(m68_state);      break;
		case 0xbe: ldy_ex(m68_state);       break;
		case 0xbf: sty_ex(m68_state);       break;

		case 0xce: lds_im(m68_state);       break;

		case 0xdc: ldq_di(m68_state);       break;
		case 0xdd: stq_di(m68_state);       break;
		case 0xde: lds_di(m68_state);       break;
		case 0xdf: sts_di(m68_state);       break;

		case 0xec: ldq_ix(m68_state);       break;
		case 0xed: stq_ix(m68_state);       break;
		case 0xee: lds_ix(m68_state);       break;
		case 0xef: sts_ix(m68_state);       break;

		case 0xfc: ldq_ex(m68_state);       break;
		case 0xfd: stq_ex(m68_state);       break;
		case 0xfe: lds_ex(m68_state);       break;
		case 0xff: sts_ex(m68_state);       break;

		default:  IIError(m68_state);        break;
	}
#else

	(*hd6309_page01[ireg2])(m68_state);

#endif /* BIG_SWITCH */

	m68_state->icount -= m68_state->cycle_counts_page01[ireg2];
}

/* $11xx opcodes */
OP_HANDLER( pref11 )
{
	UINT8 ireg2 = ROP(PCD);
	PC++;

#if BIG_SWITCH
	switch( ireg2 )
	{
		case 0x30: band(m68_state);         break;
		case 0x31: biand(m68_state);        break;
		case 0x32: bor(m68_state);          break;
		case 0x33: bior(m68_state);         break;
		case 0x34: beor(m68_state);         break;
		case 0x35: bieor(m68_state);        break;
		case 0x36: ldbt(m68_state);         break;
		case 0x37: stbt(m68_state);         break;
		case 0x38: tfmpp(m68_state);        break;  /* Timing for TFM is actually 6+3n.        */
		case 0x39: tfmmm(m68_state);        break;  /* To avoid saving the state, I decided    */
		case 0x3a: tfmpc(m68_state);        break;  /* to push the initial 6 cycles to the end */
		case 0x3b: tfmcp(m68_state);        break;  /* We will soon see how this fairs!        */
		case 0x3c: bitmd_im(m68_state);     break;
		case 0x3d: ldmd_im(m68_state);      break;
		case 0x3f: swi3(m68_state);         break;

		case 0x43: come(m68_state);         break;
		case 0x4a: dece(m68_state);         break;
		case 0x4c: ince(m68_state);         break;
		case 0x4d: tste(m68_state);         break;
		case 0x4f: clre(m68_state);         break;

		case 0x53: comf(m68_state);         break;
		case 0x5a: decf(m68_state);         break;
		case 0x5c: incf(m68_state);         break;
		case 0x5d: tstf(m68_state);         break;
		case 0x5f: clrf(m68_state);         break;

		case 0x80: sube_im(m68_state);      break;
		case 0x81: cmpe_im(m68_state);      break;
		case 0x83: cmpu_im(m68_state);      break;
		case 0x86: lde_im(m68_state);       break;
		case 0x8b: adde_im(m68_state);      break;
		case 0x8c: cmps_im(m68_state);      break;
		case 0x8d: divd_im(m68_state);      break;
		case 0x8e: divq_im(m68_state);      break;
		case 0x8f: muld_im(m68_state);      break;

		case 0x90: sube_di(m68_state);      break;
		case 0x91: cmpe_di(m68_state);      break;
		case 0x93: cmpu_di(m68_state);      break;
		case 0x96: lde_di(m68_state);       break;
		case 0x97: ste_di(m68_state);       break;
		case 0x9b: adde_di(m68_state);      break;
		case 0x9c: cmps_di(m68_state);      break;
		case 0x9d: divd_di(m68_state);      break;
		case 0x9e: divq_di(m68_state);      break;
		case 0x9f: muld_di(m68_state);      break;

		case 0xa0: sube_ix(m68_state);      break;
		case 0xa1: cmpe_ix(m68_state);      break;
		case 0xa3: cmpu_ix(m68_state);      break;
		case 0xa6: lde_ix(m68_state);       break;
		case 0xa7: ste_ix(m68_state);       break;
		case 0xab: adde_ix(m68_state);      break;
		case 0xac: cmps_ix(m68_state);      break;
		case 0xad: divd_ix(m68_state);      break;
		case 0xae: divq_ix(m68_state);      break;
		case 0xaf: muld_ix(m68_state);      break;

		case 0xb0: sube_ex(m68_state);      break;
		case 0xb1: cmpe_ex(m68_state);      break;
		case 0xb3: cmpu_ex(m68_state);      break;
		case 0xb6: lde_ex(m68_state);       break;
		case 0xb7: ste_ex(m68_state);       break;
		case 0xbb: adde_ex(m68_state);      break;
		case 0xbc: cmps_ex(m68_state);      break;
		case 0xbd: divd_ex(m68_state);      break;
		case 0xbe: divq_ex(m68_state);      break;
		case 0xbf: muld_ex(m68_state);      break;

		case 0xc0: subf_im(m68_state);      break;
		case 0xc1: cmpf_im(m68_state);      break;
		case 0xc6: ldf_im(m68_state);       break;
		case 0xcb: addf_im(m68_state);      break;

		case 0xd0: subf_di(m68_state);      break;
		case 0xd1: cmpf_di(m68_state);      break;
		case 0xd6: ldf_di(m68_state);       break;
		case 0xd7: stf_di(m68_state);       break;
		case 0xdb: addf_di(m68_state);      break;

		case 0xe0: subf_ix(m68_state);      break;
		case 0xe1: cmpf_ix(m68_state);      break;
		case 0xe6: ldf_ix(m68_state);       break;
		case 0xe7: stf_ix(m68_state);       break;
		case 0xeb: addf_ix(m68_state);      break;

		case 0xf0: subf_ex(m68_state);      break;
		case 0xf1: cmpf_ex(m68_state);      break;
		case 0xf6: ldf_ex(m68_state);       break;
		case 0xf7: stf_ex(m68_state);       break;
		case 0xfb: addf_ex(m68_state);      break;

		default:   IIError(m68_state);      break;
	}
#else

	(*hd6309_page11[ireg2])(m68_state);

#endif /* BIG_SWITCH */
	m68_state->icount -= m68_state->cycle_counts_page11[ireg2];
}
