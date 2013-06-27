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

#ifdef NEW
static void illegal( konami_state *cpustate )
#else
INLINE void illegal( konami_state *cpustate )
#endif
{
	logerror("KONAMI: illegal opcode at %04x\n",PC);
}

/* $00 NEG direct ?**** */
INLINE void neg_di( konami_state *cpustate )
{
	UINT16 r,t;
	DIRBYTE(cpustate, t);
	r = -t;
	CLR_NZVC;
	SET_FLAGS8(0,t,r);
	WM(cpustate, EAD,r);
}

/* $01 ILLEGAL */

/* $02 ILLEGAL */

/* $03 COM direct -**01 */
INLINE void com_di( konami_state *cpustate )
{
	UINT8 t;
	DIRBYTE(cpustate, t);
	t = ~t;
	CLR_NZV;
	SET_NZ8(t);
	SEC;
	WM(cpustate, EAD,t);
}

/* $04 LSR direct -0*-* */
INLINE void lsr_di( konami_state *cpustate )
{
	UINT8 t;
	DIRBYTE(cpustate, t);
	CLR_NZC;
	CC |= (t & CC_C);
	t >>= 1;
	SET_Z8(t);
	WM(cpustate, EAD,t);
}

/* $05 ILLEGAL */

/* $06 ROR direct -**-* */
INLINE void ror_di( konami_state *cpustate )
{
	UINT8 t,r;
	DIRBYTE(cpustate, t);
	r= (CC & CC_C) << 7;
	CLR_NZC;
	CC |= (t & CC_C);
	r |= t>>1;
	SET_NZ8(r);
	WM(cpustate, EAD,r);
}

/* $07 ASR direct ?**-* */
INLINE void asr_di( konami_state *cpustate )
{
	UINT8 t;
	DIRBYTE(cpustate, t);
	CLR_NZC;
	CC |= (t & CC_C);
	t = (t & 0x80) | (t >> 1);
	SET_NZ8(t);
	WM(cpustate, EAD,t);
}

/* $08 ASL direct ?**** */
INLINE void asl_di( konami_state *cpustate )
{
	UINT16 t,r;
	DIRBYTE(cpustate, t);
	r = t << 1;
	CLR_NZVC;
	SET_FLAGS8(t,t,r);
	WM(cpustate, EAD,r);
}

/* $09 ROL direct -**** */
INLINE void rol_di( konami_state *cpustate )
{
	UINT16 t,r;
	DIRBYTE(cpustate, t);
	r = (CC & CC_C) | (t << 1);
	CLR_NZVC;
	SET_FLAGS8(t,t,r);
	WM(cpustate, EAD,r);
}

/* $0A DEC direct -***- */
INLINE void dec_di( konami_state *cpustate )
{
	UINT8 t;
	DIRBYTE(cpustate, t);
	--t;
	CLR_NZV;
	SET_FLAGS8D(t);
	WM(cpustate, EAD,t);
}

/* $0B ILLEGAL */

/* $OC INC direct -***- */
INLINE void inc_di( konami_state *cpustate )
{
	UINT8 t;
	DIRBYTE(cpustate, t);
	++t;
	CLR_NZV;
	SET_FLAGS8I(t);
	WM(cpustate, EAD,t);
}

/* $OD TST direct -**0- */
INLINE void tst_di( konami_state *cpustate )
{
	UINT8 t;
	DIRBYTE(cpustate, t);
	CLR_NZV;
	SET_NZ8(t);
}

/* $0E JMP direct ----- */
INLINE void jmp_di( konami_state *cpustate )
{
	DIRECT(cpustate);
	PCD=EAD;
}

/* $0F CLR direct -0100 */
INLINE void clr_di( konami_state *cpustate )
{
	DIRECT(cpustate);
	WM(cpustate, EAD,0);
	CLR_NZVC;
	SEZ;
}

/* $10 FLAG */

/* $11 FLAG */

/* $12 NOP inherent ----- */
INLINE void nop( konami_state *cpustate )
{
	;
}

/* $13 SYNC inherent ----- */
INLINE void sync( konami_state *cpustate )
{
	/* SYNC stops processing instructions until an interrupt request happens. */
	/* This doesn't require the corresponding interrupt to be enabled: if it */
	/* is disabled, execution continues with the next instruction. */
	cpustate->int_state |= KONAMI_SYNC;
	check_irq_lines(cpustate);
	/* if KONAMI_SYNC has not been cleared by check_irq_lines,
	 * stop execution until the interrupt lines change. */
	if( (cpustate->int_state & KONAMI_SYNC) && cpustate->icount > 0 )
		cpustate->icount = 0;
}

/* $14 ILLEGAL */

/* $15 ILLEGAL */

/* $16 LBRA relative ----- */
INLINE void lbra( konami_state *cpustate )
{
	IMMWORD(cpustate, cpustate->ea);
	PC += EA;
}

/* $17 LBSR relative ----- */
INLINE void lbsr( konami_state *cpustate )
{
	IMMWORD(cpustate, cpustate->ea);
	PUSHWORD(cpustate, pPC);
	PC += EA;
}

/* $18 ILLEGAL */

#if 1
/* $19 DAA inherent (A) -**0* */
INLINE void daa( konami_state *cpustate )
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
#else
/* $19 DAA inherent (A) -**0* */
INLINE void daa( konami_state *cpustate )
{
	UINT16 t;
	t = A;
	if (CC & CC_H) t+=0x06;
	if ((t&0x0f)>9) t+=0x06;        /* ASG -- this code is broken! $66+$99=$FF -> DAA should = $65, we get $05! */
	if (CC & CC_C) t+=0x60;
	if ((t&0xf0)>0x90) t+=0x60;
	if (t&0x100) SEC;
	A = t;
}
#endif

/* $1A ORCC immediate ##### */
INLINE void orcc( konami_state *cpustate )
{
	UINT8 t;
	IMMBYTE(cpustate, t);
	CC |= t;
	check_irq_lines(cpustate);
}

/* $1B ILLEGAL */

/* $1C ANDCC immediate ##### */
INLINE void andcc( konami_state *cpustate )
{
	UINT8 t;
	IMMBYTE(cpustate, t);
	CC &= t;
	check_irq_lines(cpustate);
}

/* $1D SEX inherent -**0- */
INLINE void sex( konami_state *cpustate )
{
	UINT16 t;
	t = SIGNED(B);
	D = t;
//  CLR_NZV;    NS 20020905: applying the same fix that was applied to 6809 and 6309
	CLR_NZ;
	SET_NZ16(t);
}

/* $1E EXG inherent ----- */
INLINE void exg( konami_state *cpustate )
{
	UINT16 t1 = 0, t2 = 0;
	UINT8 tb;

	IMMBYTE(cpustate, tb);

	GETREG( t1, tb >> 4 );
	GETREG( t2, tb & 0x0f );

	SETREG( t2, tb >> 4 );
	SETREG( t1, tb & 0x0f );
}

/* $1F TFR inherent ----- */
INLINE void tfr( konami_state *cpustate )
{
	UINT8 tb;
	UINT16 t = 0;

	IMMBYTE(cpustate, tb);

	GETREG( t, tb & 0x0f );
	SETREG( t, ( tb >> 4 ) & 0x07 );
}

/* $20 BRA relative ----- */
INLINE void bra( konami_state *cpustate )
{
	UINT8 t;
	IMMBYTE(cpustate, t);
	PC += SIGNED(t);
}

/* $21 BRN relative ----- */
static UINT8 konami_brn_t;  // hack around GCC 4.6 error because we need the side effects of IMMBYTE
INLINE void brn( konami_state *cpustate )
{
	IMMBYTE(cpustate, konami_brn_t);
}

/* $1021 LBRN relative ----- */
INLINE void lbrn( konami_state *cpustate )
{
	IMMWORD(cpustate, cpustate->ea);
}

/* $22 BHI relative ----- */
INLINE void bhi( konami_state *cpustate )
{
	BRANCH(cpustate, !(CC & (CC_Z|CC_C)) );
}

/* $1022 LBHI relative ----- */
INLINE void lbhi( konami_state *cpustate )
{
	LBRANCH(cpustate, !(CC & (CC_Z|CC_C)) );
}

/* $23 BLS relative ----- */
INLINE void bls( konami_state *cpustate )
{
	BRANCH(cpustate, (CC & (CC_Z|CC_C)) );
}

/* $1023 LBLS relative ----- */
INLINE void lbls( konami_state *cpustate )
{
	LBRANCH(cpustate, (CC&(CC_Z|CC_C)) );
}

/* $24 BCC relative ----- */
INLINE void bcc( konami_state *cpustate )
{
	BRANCH(cpustate, !(CC&CC_C) );
}

/* $1024 LBCC relative ----- */
INLINE void lbcc( konami_state *cpustate )
{
	LBRANCH(cpustate, !(CC&CC_C) );
}

/* $25 BCS relative ----- */
INLINE void bcs( konami_state *cpustate )
{
	BRANCH(cpustate, (CC&CC_C) );
}

/* $1025 LBCS relative ----- */
INLINE void lbcs( konami_state *cpustate )
{
	LBRANCH(cpustate, (CC&CC_C) );
}

/* $26 BNE relative ----- */
INLINE void bne( konami_state *cpustate )
{
	BRANCH(cpustate, !(CC&CC_Z) );
}

/* $1026 LBNE relative ----- */
INLINE void lbne( konami_state *cpustate )
{
	LBRANCH(cpustate, !(CC&CC_Z) );
}

/* $27 BEQ relative ----- */
INLINE void beq( konami_state *cpustate )
{
	BRANCH(cpustate, (CC&CC_Z) );
}

/* $1027 LBEQ relative ----- */
INLINE void lbeq( konami_state *cpustate )
{
	LBRANCH(cpustate, (CC&CC_Z) );
}

/* $28 BVC relative ----- */
INLINE void bvc( konami_state *cpustate )
{
	BRANCH(cpustate, !(CC&CC_V) );
}

/* $1028 LBVC relative ----- */
INLINE void lbvc( konami_state *cpustate )
{
	LBRANCH(cpustate, !(CC&CC_V) );
}

/* $29 BVS relative ----- */
INLINE void bvs( konami_state *cpustate )
{
	BRANCH(cpustate, (CC&CC_V) );
}

/* $1029 LBVS relative ----- */
INLINE void lbvs( konami_state *cpustate )
{
	LBRANCH(cpustate, (CC&CC_V) );
}

/* $2A BPL relative ----- */
INLINE void bpl( konami_state *cpustate )
{
	BRANCH(cpustate, !(CC&CC_N) );
}

/* $102A LBPL relative ----- */
INLINE void lbpl( konami_state *cpustate )
{
	LBRANCH(cpustate, !(CC&CC_N) );
}

/* $2B BMI relative ----- */
INLINE void bmi( konami_state *cpustate )
{
	BRANCH(cpustate, (CC&CC_N) );
}

/* $102B LBMI relative ----- */
INLINE void lbmi( konami_state *cpustate )
{
	LBRANCH(cpustate, (CC&CC_N) );
}

/* $2C BGE relative ----- */
INLINE void bge( konami_state *cpustate )
{
	BRANCH(cpustate, !NXORV );
}

/* $102C LBGE relative ----- */
INLINE void lbge( konami_state *cpustate )
{
	LBRANCH(cpustate, !NXORV );
}

/* $2D BLT relative ----- */
INLINE void blt( konami_state *cpustate )
{
	BRANCH(cpustate, NXORV );
}

/* $102D LBLT relative ----- */
INLINE void lblt( konami_state *cpustate )
{
	LBRANCH(cpustate, NXORV );
}

/* $2E BGT relative ----- */
INLINE void bgt( konami_state *cpustate )
{
	BRANCH(cpustate, !(NXORV || (CC&CC_Z)) );
}

/* $102E LBGT relative ----- */
INLINE void lbgt( konami_state *cpustate )
{
	LBRANCH(cpustate, !(NXORV || (CC&CC_Z)) );
}

/* $2F BLE relative ----- */
INLINE void ble( konami_state *cpustate )
{
	BRANCH(cpustate, (NXORV || (CC&CC_Z)) );
}

/* $102F LBLE relative ----- */
INLINE void lble( konami_state *cpustate )
{
	LBRANCH(cpustate, (NXORV || (CC&CC_Z)) );
}

/* $30 LEAX indexed --*-- */
INLINE void leax( konami_state *cpustate )
{
	X = EA;
	CLR_Z;
	SET_Z(X);
}

/* $31 LEAY indexed --*-- */
INLINE void leay( konami_state *cpustate )
{
	Y = EA;
	CLR_Z;
	SET_Z(Y);
}

/* $32 LEAS indexed ----- */
INLINE void leas( konami_state *cpustate )
{
	S = EA;
	cpustate->int_state |= KONAMI_LDS;
}

/* $33 LEAU indexed ----- */
INLINE void leau( konami_state *cpustate )
{
	U = EA;
}

/* $34 PSHS inherent ----- */
INLINE void pshs( konami_state *cpustate )
{
	UINT8 t;
	IMMBYTE(cpustate, t);
	if( t&0x80 ) { PUSHWORD(cpustate, pPC); cpustate->icount -= 2; }
	if( t&0x40 ) { PUSHWORD(cpustate, pU);  cpustate->icount -= 2; }
	if( t&0x20 ) { PUSHWORD(cpustate, pY);  cpustate->icount -= 2; }
	if( t&0x10 ) { PUSHWORD(cpustate, pX);  cpustate->icount -= 2; }
	if( t&0x08 ) { PUSHBYTE(cpustate, DP);  cpustate->icount -= 1; }
	if( t&0x04 ) { PUSHBYTE(cpustate, B);   cpustate->icount -= 1; }
	if( t&0x02 ) { PUSHBYTE(cpustate, A);   cpustate->icount -= 1; }
	if( t&0x01 ) { PUSHBYTE(cpustate, CC);  cpustate->icount -= 1; }
}

/* 35 PULS inherent ----- */
INLINE void puls( konami_state *cpustate )
{
	UINT8 t;
	IMMBYTE(cpustate, t);
	if( t&0x01 ) { PULLBYTE(cpustate, CC); cpustate->icount -= 1; }
	if( t&0x02 ) { PULLBYTE(cpustate, A);  cpustate->icount -= 1; }
	if( t&0x04 ) { PULLBYTE(cpustate, B);  cpustate->icount -= 1; }
	if( t&0x08 ) { PULLBYTE(cpustate, DP); cpustate->icount -= 1; }
	if( t&0x10 ) { PULLWORD(cpustate, XD); cpustate->icount -= 2; }
	if( t&0x20 ) { PULLWORD(cpustate, YD); cpustate->icount -= 2; }
	if( t&0x40 ) { PULLWORD(cpustate, UD); cpustate->icount -= 2; }
	if( t&0x80 ) { PULLWORD(cpustate, PCD); cpustate->icount -= 2; }

	/* check after all PULLs */
	if( t&0x01 ) { check_irq_lines(cpustate); }
}

/* $36 PSHU inherent ----- */
INLINE void pshu( konami_state *cpustate )
{
	UINT8 t;
	IMMBYTE(cpustate, t);
	if( t&0x80 ) { PSHUWORD(cpustate, pPC); cpustate->icount -= 2; }
	if( t&0x40 ) { PSHUWORD(cpustate, pS);  cpustate->icount -= 2; }
	if( t&0x20 ) { PSHUWORD(cpustate, pY);  cpustate->icount -= 2; }
	if( t&0x10 ) { PSHUWORD(cpustate, pX);  cpustate->icount -= 2; }
	if( t&0x08 ) { PSHUBYTE(cpustate, DP);  cpustate->icount -= 1; }
	if( t&0x04 ) { PSHUBYTE(cpustate, B);   cpustate->icount -= 1; }
	if( t&0x02 ) { PSHUBYTE(cpustate, A);   cpustate->icount -= 1; }
	if( t&0x01 ) { PSHUBYTE(cpustate, CC);  cpustate->icount -= 1; }
}

/* 37 PULU inherent ----- */
INLINE void pulu( konami_state *cpustate )
{
	UINT8 t;
	IMMBYTE(cpustate, t);
	if( t&0x01 ) { PULUBYTE(cpustate, CC); cpustate->icount -= 1; }
	if( t&0x02 ) { PULUBYTE(cpustate, A);  cpustate->icount -= 1; }
	if( t&0x04 ) { PULUBYTE(cpustate, B);  cpustate->icount -= 1; }
	if( t&0x08 ) { PULUBYTE(cpustate, DP); cpustate->icount -= 1; }
	if( t&0x10 ) { PULUWORD(cpustate, XD); cpustate->icount -= 2; }
	if( t&0x20 ) { PULUWORD(cpustate, YD); cpustate->icount -= 2; }
	if( t&0x40 ) { PULUWORD(cpustate, SD); cpustate->icount -= 2; }
	if( t&0x80 ) { PULUWORD(cpustate, PCD); cpustate->icount -= 2; }

	/* check after all PULLs */
	if( t&0x01 ) { check_irq_lines(cpustate); }
}

/* $38 ILLEGAL */

/* $39 RTS inherent ----- */
INLINE void rts( konami_state *cpustate )
{
	PULLWORD(cpustate, PCD);
}

/* $3A ABX inherent ----- */
INLINE void abx( konami_state *cpustate )
{
	X += B;
}

/* $3B RTI inherent ##### */
INLINE void rti( konami_state *cpustate )
{
	PULLBYTE(cpustate, CC);
	if( CC & CC_E ) /* entire state saved? */
	{
		cpustate->icount -= 9;
		PULLBYTE(cpustate, A);
		PULLBYTE(cpustate, B);
		PULLBYTE(cpustate, DP);
		PULLWORD(cpustate, XD);
		PULLWORD(cpustate, YD);
		PULLWORD(cpustate, UD);
	}
	PULLWORD(cpustate, PCD);
	check_irq_lines(cpustate);
}

/* $3C CWAI inherent ----1 */
INLINE void cwai( konami_state *cpustate )
{
	UINT8 t;
	IMMBYTE(cpustate, t);
	CC &= t;
	/*
	 * CWAI stacks the entire machine state on the hardware stack,
	 * then waits for an interrupt; when the interrupt is taken
	 * later, the state is *not* saved again after CWAI.
	 */
	CC |= CC_E;         /* HJB 990225: save entire state */
	PUSHWORD(cpustate, pPC);
	PUSHWORD(cpustate, pU);
	PUSHWORD(cpustate, pY);
	PUSHWORD(cpustate, pX);
	PUSHBYTE(cpustate, DP);
	PUSHBYTE(cpustate, B);
	PUSHBYTE(cpustate, A);
	PUSHBYTE(cpustate, CC);
	cpustate->int_state |= KONAMI_CWAI;
	check_irq_lines(cpustate);
	if( (cpustate->int_state & KONAMI_CWAI) && cpustate->icount > 0 )
		cpustate->icount = 0;
}

/* $3D MUL inherent --*-@ */
INLINE void mul( konami_state *cpustate )
{
	UINT16 t;
	t = A * B;
	CLR_ZC; SET_Z16(t); if(t&0x80) SEC;
	D = t;
}

/* $3E ILLEGAL */

/* $3F SWI (SWI2 SWI3) absolute indirect ----- */
INLINE void swi( konami_state *cpustate )
{
	CC |= CC_E;             /* HJB 980225: save entire state */
	PUSHWORD(cpustate, pPC);
	PUSHWORD(cpustate, pU);
	PUSHWORD(cpustate, pY);
	PUSHWORD(cpustate, pX);
	PUSHBYTE(cpustate, DP);
	PUSHBYTE(cpustate, B);
	PUSHBYTE(cpustate, A);
	PUSHBYTE(cpustate, CC);
	CC |= CC_IF | CC_II;    /* inhibit FIRQ and IRQ */
	PCD=RM16(cpustate, 0xfffa);
}

/* $103F SWI2 absolute indirect ----- */
INLINE void swi2( konami_state *cpustate )
{
	CC |= CC_E;             /* HJB 980225: save entire state */
	PUSHWORD(cpustate, pPC);
	PUSHWORD(cpustate, pU);
	PUSHWORD(cpustate, pY);
	PUSHWORD(cpustate, pX);
	PUSHBYTE(cpustate, DP);
	PUSHBYTE(cpustate, B);
	PUSHBYTE(cpustate, A);
	PUSHBYTE(cpustate, CC);
	PCD=RM16(cpustate, 0xfff4);
}

/* $113F SWI3 absolute indirect ----- */
INLINE void swi3( konami_state *cpustate )
{
	CC |= CC_E;             /* HJB 980225: save entire state */
	PUSHWORD(cpustate, pPC);
	PUSHWORD(cpustate, pU);
	PUSHWORD(cpustate, pY);
	PUSHWORD(cpustate, pX);
	PUSHBYTE(cpustate, DP);
	PUSHBYTE(cpustate, B);
	PUSHBYTE(cpustate, A);
	PUSHBYTE(cpustate, CC);
	PCD=RM16(cpustate, 0xfff2);
}

/* $40 NEGA inherent ?**** */
INLINE void nega( konami_state *cpustate )
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
INLINE void coma( konami_state *cpustate )
{
	A = ~A;
	CLR_NZV;
	SET_NZ8(A);
	SEC;
}

/* $44 LSRA inherent -0*-* */
INLINE void lsra( konami_state *cpustate )
{
	CLR_NZC;
	CC |= (A & CC_C);
	A >>= 1;
	SET_Z8(A);
}

/* $45 ILLEGAL */

/* $46 RORA inherent -**-* */
INLINE void rora( konami_state *cpustate )
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
INLINE void asra( konami_state *cpustate )
{
	CLR_NZC;
	CC |= (A & CC_C);
	A = (A & 0x80) | (A >> 1);
	SET_NZ8(A);
}

/* $48 ASLA inherent ?**** */
INLINE void asla( konami_state *cpustate )
{
	UINT16 r;
	r = A << 1;
	CLR_NZVC;
	SET_FLAGS8(A,A,r);
	A = r;
}

/* $49 ROLA inherent -**** */
INLINE void rola( konami_state *cpustate )
{
	UINT16 t,r;
	t = A;
	r = (CC & CC_C) | (t<<1);
	CLR_NZVC; SET_FLAGS8(t,t,r);
	A = r;
}

/* $4A DECA inherent -***- */
INLINE void deca( konami_state *cpustate )
{
	--A;
	CLR_NZV;
	SET_FLAGS8D(A);
}

/* $4B ILLEGAL */

/* $4C INCA inherent -***- */
INLINE void inca( konami_state *cpustate )
{
	++A;
	CLR_NZV;
	SET_FLAGS8I(A);
}

/* $4D TSTA inherent -**0- */
INLINE void tsta( konami_state *cpustate )
{
	CLR_NZV;
	SET_NZ8(A);
}

/* $4E ILLEGAL */

/* $4F CLRA inherent -0100 */
INLINE void clra( konami_state *cpustate )
{
	A = 0;
	CLR_NZVC; SEZ;
}

/* $50 NEGB inherent ?**** */
INLINE void negb( konami_state *cpustate )
{
	UINT16 r;
	r = -B;
	CLR_NZVC;
	SET_FLAGS8(0,B,r);
	B = r;
}

/* $51 ILLEGAL */

/* $52 ILLEGAL */

/* $53 COMB inherent -**01 */
INLINE void comb( konami_state *cpustate )
{
	B = ~B;
	CLR_NZV;
	SET_NZ8(B);
	SEC;
}

/* $54 LSRB inherent -0*-* */
INLINE void lsrb( konami_state *cpustate )
{
	CLR_NZC;
	CC |= (B & CC_C);
	B >>= 1;
	SET_Z8(B);
}

/* $55 ILLEGAL */

/* $56 RORB inherent -**-* */
INLINE void rorb( konami_state *cpustate )
{
	UINT8 r;
	r = (CC & CC_C) << 7;
	CLR_NZC;
	CC |= (B & CC_C);
	r |= B >> 1;
	SET_NZ8(r);
	B = r;
}

/* $57 ASRB inherent ?**-* */
INLINE void asrb( konami_state *cpustate )
{
	CLR_NZC;
	CC |= (B & CC_C);
	B= (B & 0x80) | (B >> 1);
	SET_NZ8(B);
}

/* $58 ASLB inherent ?**** */
INLINE void aslb( konami_state *cpustate )
{
	UINT16 r;
	r = B << 1;
	CLR_NZVC;
	SET_FLAGS8(B,B,r);
	B = r;
}

/* $59 ROLB inherent -**** */
INLINE void rolb( konami_state *cpustate )
{
	UINT16 t,r;
	t = B;
	r = CC & CC_C;
	r |= t << 1;
	CLR_NZVC;
	SET_FLAGS8(t,t,r);
	B = r;
}

/* $5A DECB inherent -***- */
INLINE void decb( konami_state *cpustate )
{
	--B;
	CLR_NZV;
	SET_FLAGS8D(B);
}

/* $5B ILLEGAL */

/* $5C INCB inherent -***- */
INLINE void incb( konami_state *cpustate )
{
	++B;
	CLR_NZV;
	SET_FLAGS8I(B);
}

/* $5D TSTB inherent -**0- */
INLINE void tstb( konami_state *cpustate )
{
	CLR_NZV;
	SET_NZ8(B);
}

/* $5E ILLEGAL */

/* $5F CLRB inherent -0100 */
INLINE void clrb( konami_state *cpustate )
{
	B = 0;
	CLR_NZVC; SEZ;
}

/* $60 NEG indexed ?**** */
INLINE void neg_ix( konami_state *cpustate )
{
	UINT16 r,t;
	t = RM(cpustate, EAD);
	r = -t;
	CLR_NZVC;
	SET_FLAGS8(0,t,r);
	WM(cpustate, EAD,r);
}

/* $61 ILLEGAL */

/* $62 ILLEGAL */

/* $63 COM indexed -**01 */
INLINE void com_ix( konami_state *cpustate )
{
	UINT8 t;
	t = ~RM(cpustate, EAD);
	CLR_NZV;
	SET_NZ8(t);
	SEC;
	WM(cpustate, EAD,t);
}

/* $64 LSR indexed -0*-* */
INLINE void lsr_ix( konami_state *cpustate )
{
	UINT8 t;
	t = RM(cpustate, EAD);
	CLR_NZC;
	CC |= (t & CC_C);
	t>>=1; SET_Z8(t);
	WM(cpustate, EAD,t);
}

/* $65 ILLEGAL */

/* $66 ROR indexed -**-* */
INLINE void ror_ix( konami_state *cpustate )
{
	UINT8 t,r;
	t = RM(cpustate, EAD);
	r = (CC & CC_C) << 7;
	CLR_NZC;
	CC |= (t & CC_C);
	r |= t>>1; SET_NZ8(r);
	WM(cpustate, EAD,r);
}

/* $67 ASR indexed ?**-* */
INLINE void asr_ix( konami_state *cpustate )
{
	UINT8 t;
	t = RM(cpustate, EAD);
	CLR_NZC;
	CC |= (t & CC_C);
	t=(t&0x80)|(t>>1);
	SET_NZ8(t);
	WM(cpustate, EAD,t);
}

/* $68 ASL indexed ?**** */
INLINE void asl_ix( konami_state *cpustate )
{
	UINT16 t,r;
	t = RM(cpustate, EAD);
	r = t << 1;
	CLR_NZVC;
	SET_FLAGS8(t,t,r);
	WM(cpustate, EAD,r);
}

/* $69 ROL indexed -**** */
INLINE void rol_ix( konami_state *cpustate )
{
	UINT16 t,r;
	t = RM(cpustate, EAD);
	r = CC & CC_C;
	r |= t << 1;
	CLR_NZVC;
	SET_FLAGS8(t,t,r);
	WM(cpustate, EAD,r);
}

/* $6A DEC indexed -***- */
INLINE void dec_ix( konami_state *cpustate )
{
	UINT8 t;
	t = RM(cpustate, EAD) - 1;
	CLR_NZV; SET_FLAGS8D(t);
	WM(cpustate, EAD,t);
}

/* $6B ILLEGAL */

/* $6C INC indexed -***- */
INLINE void inc_ix( konami_state *cpustate )
{
	UINT8 t;
	t = RM(cpustate, EAD) + 1;
	CLR_NZV; SET_FLAGS8I(t);
	WM(cpustate, EAD,t);
}

/* $6D TST indexed -**0- */
INLINE void tst_ix( konami_state *cpustate )
{
	UINT8 t;
	t = RM(cpustate, EAD);
	CLR_NZV;
	SET_NZ8(t);
}

/* $6E JMP indexed ----- */
INLINE void jmp_ix( konami_state *cpustate )
{
	PCD=EAD;
}

/* $6F CLR indexed -0100 */
INLINE void clr_ix( konami_state *cpustate )
{
	WM(cpustate, EAD,0);
	CLR_NZVC; SEZ;
}

/* $70 NEG extended ?**** */
INLINE void neg_ex( konami_state *cpustate )
{
	UINT16 r,t;
	EXTBYTE(cpustate, t); r=-t;
	CLR_NZVC; SET_FLAGS8(0,t,r);
	WM(cpustate, EAD,r);
}

/* $71 ILLEGAL */

/* $72 ILLEGAL */

/* $73 COM extended -**01 */
INLINE void com_ex( konami_state *cpustate )
{
	UINT8 t;
	EXTBYTE(cpustate, t); t = ~t;
	CLR_NZV; SET_NZ8(t); SEC;
	WM(cpustate, EAD,t);
}

/* $74 LSR extended -0*-* */
INLINE void lsr_ex( konami_state *cpustate )
{
	UINT8 t;
	EXTBYTE(cpustate, t); CLR_NZC; CC |= (t & CC_C);
	t>>=1; SET_Z8(t);
	WM(cpustate, EAD,t);
}

/* $75 ILLEGAL */

/* $76 ROR extended -**-* */
INLINE void ror_ex( konami_state *cpustate )
{
	UINT8 t,r;
	EXTBYTE(cpustate, t); r=(CC & CC_C) << 7;
	CLR_NZC; CC |= (t & CC_C);
	r |= t>>1; SET_NZ8(r);
	WM(cpustate, EAD,r);
}

/* $77 ASR extended ?**-* */
INLINE void asr_ex( konami_state *cpustate )
{
	UINT8 t;
	EXTBYTE(cpustate, t); CLR_NZC; CC |= (t & CC_C);
	t=(t&0x80)|(t>>1);
	SET_NZ8(t);
	WM(cpustate, EAD,t);
}

/* $78 ASL extended ?**** */
INLINE void asl_ex( konami_state *cpustate )
{
	UINT16 t,r;
	EXTBYTE(cpustate, t); r=t<<1;
	CLR_NZVC; SET_FLAGS8(t,t,r);
	WM(cpustate, EAD,r);
}

/* $79 ROL extended -**** */
INLINE void rol_ex( konami_state *cpustate )
{
	UINT16 t,r;
	EXTBYTE(cpustate, t); r = (CC & CC_C) | (t << 1);
	CLR_NZVC; SET_FLAGS8(t,t,r);
	WM(cpustate, EAD,r);
}

/* $7A DEC extended -***- */
INLINE void dec_ex( konami_state *cpustate )
{
	UINT8 t;
	EXTBYTE(cpustate, t); --t;
	CLR_NZV; SET_FLAGS8D(t);
	WM(cpustate, EAD,t);
}

/* $7B ILLEGAL */

/* $7C INC extended -***- */
INLINE void inc_ex( konami_state *cpustate )
{
	UINT8 t;
	EXTBYTE(cpustate, t); ++t;
	CLR_NZV; SET_FLAGS8I(t);
	WM(cpustate, EAD,t);
}

/* $7D TST extended -**0- */
INLINE void tst_ex( konami_state *cpustate )
{
	UINT8 t;
	EXTBYTE(cpustate, t); CLR_NZV; SET_NZ8(t);
}

/* $7E JMP extended ----- */
INLINE void jmp_ex( konami_state *cpustate )
{
	EXTENDED(cpustate);
	PCD=EAD;
}

/* $7F CLR extended -0100 */
INLINE void clr_ex( konami_state *cpustate )
{
	EXTENDED(cpustate);
	WM(cpustate, EAD,0);
	CLR_NZVC; SEZ;
}

/* $80 SUBA immediate ?**** */
INLINE void suba_im( konami_state *cpustate )
{
	UINT16 t,r;
	IMMBYTE(cpustate, t);
	r = A - t;
	CLR_NZVC;
	SET_FLAGS8(A,t,r);
	A = r;
}

/* $81 CMPA immediate ?**** */
INLINE void cmpa_im( konami_state *cpustate )
{
	UINT16    t,r;
	IMMBYTE(cpustate, t);
	r = A - t;
	CLR_NZVC;
	SET_FLAGS8(A,t,r);
}

/* $82 SBCA immediate ?**** */
INLINE void sbca_im( konami_state *cpustate )
{
	UINT16    t,r;
	IMMBYTE(cpustate, t);
	r = A - t - (CC & CC_C);
	CLR_NZVC;
	SET_FLAGS8(A,t,r);
	A = r;
}

/* $83 SUBD (CMPD CMPU) immediate -**** */
INLINE void subd_im( konami_state *cpustate )
{
	UINT32 r,d;
	PAIR b;
	IMMWORD(cpustate, b);
	d = D;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
	D = r;
}

/* $1083 CMPD immediate -**** */
INLINE void cmpd_im( konami_state *cpustate )
{
	UINT32 r,d;
	PAIR b;
	IMMWORD(cpustate, b);
	d = D;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
}

/* $1183 CMPU immediate -**** */
INLINE void cmpu_im( konami_state *cpustate )
{
	UINT32 r, d;
	PAIR b;
	IMMWORD(cpustate, b);
	d = U;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
}

/* $84 ANDA immediate -**0- */
INLINE void anda_im( konami_state *cpustate )
{
	UINT8 t;
	IMMBYTE(cpustate, t);
	A &= t;
	CLR_NZV;
	SET_NZ8(A);
}

/* $85 BITA immediate -**0- */
INLINE void bita_im( konami_state *cpustate )
{
	UINT8 t,r;
	IMMBYTE(cpustate, t);
	r = A & t;
	CLR_NZV;
	SET_NZ8(r);
}

/* $86 LDA immediate -**0- */
INLINE void lda_im( konami_state *cpustate )
{
	IMMBYTE(cpustate, A);
	CLR_NZV;
	SET_NZ8(A);
}

/* is this a legal instruction? */
/* $87 STA immediate -**0- */
INLINE void sta_im( konami_state *cpustate )
{
	CLR_NZV;
	SET_NZ8(A);
	IMM8(cpustate);
	WM(cpustate, EAD,A);
}

/* $88 EORA immediate -**0- */
INLINE void eora_im( konami_state *cpustate )
{
	UINT8 t;
	IMMBYTE(cpustate, t);
	A ^= t;
	CLR_NZV;
	SET_NZ8(A);
}

/* $89 ADCA immediate ***** */
INLINE void adca_im( konami_state *cpustate )
{
	UINT16 t,r;
	IMMBYTE(cpustate, t);
	r = A + t + (CC & CC_C);
	CLR_HNZVC;
	SET_FLAGS8(A,t,r);
	SET_H(A,t,r);
	A = r;
}

/* $8A ORA immediate -**0- */
INLINE void ora_im( konami_state *cpustate )
{
	UINT8 t;
	IMMBYTE(cpustate, t);
	A |= t;
	CLR_NZV;
	SET_NZ8(A);
}

/* $8B ADDA immediate ***** */
INLINE void adda_im( konami_state *cpustate )
{
	UINT16 t,r;
	IMMBYTE(cpustate, t);
	r = A + t;
	CLR_HNZVC;
	SET_FLAGS8(A,t,r);
	SET_H(A,t,r);
	A = r;
}

/* $8C CMPX (CMPY CMPS) immediate -**** */
INLINE void cmpx_im( konami_state *cpustate )
{
	UINT32 r,d;
	PAIR b;
	IMMWORD(cpustate, b);
	d = X;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
}

/* $108C CMPY immediate -**** */
INLINE void cmpy_im( konami_state *cpustate )
{
	UINT32 r,d;
	PAIR b;
	IMMWORD(cpustate, b);
	d = Y;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
}

/* $118C CMPS immediate -**** */
INLINE void cmps_im( konami_state *cpustate )
{
	UINT32 r,d;
	PAIR b;
	IMMWORD(cpustate, b);
	d = S;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
}

/* $8D BSR ----- */
INLINE void bsr( konami_state *cpustate )
{
	UINT8 t;
	IMMBYTE(cpustate, t);
	PUSHWORD(cpustate, pPC);
	PC += SIGNED(t);
}

/* $8E LDX (LDY) immediate -**0- */
INLINE void ldx_im( konami_state *cpustate )
{
	IMMWORD(cpustate, pX);
	CLR_NZV;
	SET_NZ16(X);
}

/* $108E LDY immediate -**0- */
INLINE void ldy_im( konami_state *cpustate )
{
	IMMWORD(cpustate, pY);
	CLR_NZV;
	SET_NZ16(Y);
}

/* is this a legal instruction? */
/* $8F STX (STY) immediate -**0- */
INLINE void stx_im( konami_state *cpustate )
{
	CLR_NZV;
	SET_NZ16(X);
	IMM16(cpustate);
	WM16(cpustate, EAD,&pX);
}

/* is this a legal instruction? */
/* $108F STY immediate -**0- */
INLINE void sty_im( konami_state *cpustate )
{
	CLR_NZV;
	SET_NZ16(Y);
	IMM16(cpustate);
	WM16(cpustate, EAD,&pY);
}

/* $90 SUBA direct ?**** */
INLINE void suba_di( konami_state *cpustate )
{
	UINT16    t,r;
	DIRBYTE(cpustate, t);
	r = A - t;
	CLR_NZVC;
	SET_FLAGS8(A,t,r);
	A = r;
}

/* $91 CMPA direct ?**** */
INLINE void cmpa_di( konami_state *cpustate )
{
	UINT16    t,r;
	DIRBYTE(cpustate, t);
	r = A - t;
	CLR_NZVC;
	SET_FLAGS8(A,t,r);
}

/* $92 SBCA direct ?**** */
INLINE void sbca_di( konami_state *cpustate )
{
	UINT16    t,r;
	DIRBYTE(cpustate, t);
	r = A - t - (CC & CC_C);
	CLR_NZVC;
	SET_FLAGS8(A,t,r);
	A = r;
}

/* $93 SUBD (CMPD CMPU) direct -**** */
INLINE void subd_di( konami_state *cpustate )
{
	UINT32 r,d;
	PAIR b;
	DIRWORD(cpustate, b);
	d = D;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
	D = r;
}

/* $1093 CMPD direct -**** */
INLINE void cmpd_di( konami_state *cpustate )
{
	UINT32 r,d;
	PAIR b;
	DIRWORD(cpustate, b);
	d = D;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
}

/* $1193 CMPU direct -**** */
INLINE void cmpu_di( konami_state *cpustate )
{
	UINT32 r,d;
	PAIR b;
	DIRWORD(cpustate, b);
	d = U;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(U,b.d,r);
}

/* $94 ANDA direct -**0- */
INLINE void anda_di( konami_state *cpustate )
{
	UINT8 t;
	DIRBYTE(cpustate, t);
	A &= t;
	CLR_NZV;
	SET_NZ8(A);
}

/* $95 BITA direct -**0- */
INLINE void bita_di( konami_state *cpustate )
{
	UINT8 t,r;
	DIRBYTE(cpustate, t);
	r = A & t;
	CLR_NZV;
	SET_NZ8(r);
}

/* $96 LDA direct -**0- */
INLINE void lda_di( konami_state *cpustate )
{
	DIRBYTE(cpustate, A);
	CLR_NZV;
	SET_NZ8(A);
}

/* $97 STA direct -**0- */
INLINE void sta_di( konami_state *cpustate )
{
	CLR_NZV;
	SET_NZ8(A);
	DIRECT(cpustate);
	WM(cpustate, EAD,A);
}

/* $98 EORA direct -**0- */
INLINE void eora_di( konami_state *cpustate )
{
	UINT8 t;
	DIRBYTE(cpustate, t);
	A ^= t;
	CLR_NZV;
	SET_NZ8(A);
}

/* $99 ADCA direct ***** */
INLINE void adca_di( konami_state *cpustate )
{
	UINT16 t,r;
	DIRBYTE(cpustate, t);
	r = A + t + (CC & CC_C);
	CLR_HNZVC;
	SET_FLAGS8(A,t,r);
	SET_H(A,t,r);
	A = r;
}

/* $9A ORA direct -**0- */
INLINE void ora_di( konami_state *cpustate )
{
	UINT8 t;
	DIRBYTE(cpustate, t);
	A |= t;
	CLR_NZV;
	SET_NZ8(A);
}

/* $9B ADDA direct ***** */
INLINE void adda_di( konami_state *cpustate )
{
	UINT16 t,r;
	DIRBYTE(cpustate, t);
	r = A + t;
	CLR_HNZVC;
	SET_FLAGS8(A,t,r);
	SET_H(A,t,r);
	A = r;
}

/* $9C CMPX (CMPY CMPS) direct -**** */
INLINE void cmpx_di( konami_state *cpustate )
{
	UINT32 r,d;
	PAIR b;
	DIRWORD(cpustate, b);
	d = X;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
}

/* $109C CMPY direct -**** */
INLINE void cmpy_di( konami_state *cpustate )
{
	UINT32 r,d;
	PAIR b;
	DIRWORD(cpustate, b);
	d = Y;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
}

/* $119C CMPS direct -**** */
INLINE void cmps_di( konami_state *cpustate )
{
	UINT32 r,d;
	PAIR b;
	DIRWORD(cpustate, b);
	d = S;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
}

/* $9D JSR direct ----- */
INLINE void jsr_di( konami_state *cpustate )
{
	DIRECT(cpustate);
	PUSHWORD(cpustate, pPC);
	PCD=EAD;
}

/* $9E LDX (LDY) direct -**0- */
INLINE void ldx_di( konami_state *cpustate )
{
	DIRWORD(cpustate, pX);
	CLR_NZV;
	SET_NZ16(X);
}

/* $109E LDY direct -**0- */
INLINE void ldy_di( konami_state *cpustate )
{
	DIRWORD(cpustate, pY);
	CLR_NZV;
	SET_NZ16(Y);
}

/* $9F STX (STY) direct -**0- */
INLINE void stx_di( konami_state *cpustate )
{
	CLR_NZV;
	SET_NZ16(X);
	DIRECT(cpustate);
	WM16(cpustate, EAD,&pX);
}

/* $109F STY direct -**0- */
INLINE void sty_di( konami_state *cpustate )
{
	CLR_NZV;
	SET_NZ16(Y);
	DIRECT(cpustate);
	WM16(cpustate, EAD,&pY);
}

/* $a0 SUBA indexed ?**** */
INLINE void suba_ix( konami_state *cpustate )
{
	UINT16 t,r;
	t = RM(cpustate, EAD);
	r = A - t;
	CLR_NZVC;
	SET_FLAGS8(A,t,r);
	A = r;
}

/* $a1 CMPA indexed ?**** */
INLINE void cmpa_ix( konami_state *cpustate )
{
	UINT16 t,r;
	t = RM(cpustate, EAD);
	r = A - t;
	CLR_NZVC;
	SET_FLAGS8(A,t,r);
}

/* $a2 SBCA indexed ?**** */
INLINE void sbca_ix( konami_state *cpustate )
{
	UINT16    t,r;
	t = RM(cpustate, EAD);
	r = A - t - (CC & CC_C);
	CLR_NZVC;
	SET_FLAGS8(A,t,r);
	A = r;
}

/* $a3 SUBD (CMPD CMPU) indexed -**** */
INLINE void subd_ix( konami_state *cpustate )
{
	UINT32 r,d;
	PAIR b;
	b.d=RM16(cpustate, EAD);
	d = D;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
	D = r;
}

/* $10a3 CMPD indexed -**** */
INLINE void cmpd_ix( konami_state *cpustate )
{
	UINT32 r,d;
	PAIR b;
	b.d=RM16(cpustate, EAD);
	d = D;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
}

/* $11a3 CMPU indexed -**** */
INLINE void cmpu_ix( konami_state *cpustate )
{
	UINT32 r;
	PAIR b;
	b.d=RM16(cpustate, EAD);
	r = U - b.d;
	CLR_NZVC;
	SET_FLAGS16(U,b.d,r);
}

/* $a4 ANDA indexed -**0- */
INLINE void anda_ix( konami_state *cpustate )
{
	A &= RM(cpustate, EAD);
	CLR_NZV;
	SET_NZ8(A);
}

/* $a5 BITA indexed -**0- */
INLINE void bita_ix( konami_state *cpustate )
{
	UINT8 r;
	r = A & RM(cpustate, EAD);
	CLR_NZV;
	SET_NZ8(r);
}

/* $a6 LDA indexed -**0- */
INLINE void lda_ix( konami_state *cpustate )
{
	A = RM(cpustate, EAD);
	CLR_NZV;
	SET_NZ8(A);
}

/* $a7 STA indexed -**0- */
INLINE void sta_ix( konami_state *cpustate )
{
	CLR_NZV;
	SET_NZ8(A);
	WM(cpustate, EAD,A);
}

/* $a8 EORA indexed -**0- */
INLINE void eora_ix( konami_state *cpustate )
{
	A ^= RM(cpustate, EAD);
	CLR_NZV;
	SET_NZ8(A);
}

/* $a9 ADCA indexed ***** */
INLINE void adca_ix( konami_state *cpustate )
{
	UINT16 t,r;
	t = RM(cpustate, EAD);
	r = A + t + (CC & CC_C);
	CLR_HNZVC;
	SET_FLAGS8(A,t,r);
	SET_H(A,t,r);
	A = r;
}

/* $aA ORA indexed -**0- */
INLINE void ora_ix( konami_state *cpustate )
{
	A |= RM(cpustate, EAD);
	CLR_NZV;
	SET_NZ8(A);
}

/* $aB ADDA indexed ***** */
INLINE void adda_ix( konami_state *cpustate )
{
	UINT16 t,r;
	t = RM(cpustate, EAD);
	r = A + t;
	CLR_HNZVC;
	SET_FLAGS8(A,t,r);
	SET_H(A,t,r);
	A = r;
}

/* $aC CMPX (CMPY CMPS) indexed -**** */
INLINE void cmpx_ix( konami_state *cpustate )
{
	UINT32 r,d;
	PAIR b;
	b.d=RM16(cpustate, EAD);
	d = X;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
}

/* $10aC CMPY indexed -**** */
INLINE void cmpy_ix( konami_state *cpustate )
{
	UINT32 r,d;
	PAIR b;
	b.d=RM16(cpustate, EAD);
	d = Y;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
}

/* $11aC CMPS indexed -**** */
INLINE void cmps_ix( konami_state *cpustate )
{
	UINT32 r,d;
	PAIR b;
	b.d=RM16(cpustate, EAD);
	d = S;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
}

/* $aD JSR indexed ----- */
INLINE void jsr_ix( konami_state *cpustate )
{
	PUSHWORD(cpustate, pPC);
	PCD=EAD;
}

/* $aE LDX (LDY) indexed -**0- */
INLINE void ldx_ix( konami_state *cpustate )
{
	X=RM16(cpustate, EAD);
	CLR_NZV;
	SET_NZ16(X);
}

/* $10aE LDY indexed -**0- */
INLINE void ldy_ix( konami_state *cpustate )
{
	Y=RM16(cpustate, EAD);
	CLR_NZV;
	SET_NZ16(Y);
}

/* $aF STX (STY) indexed -**0- */
INLINE void stx_ix( konami_state *cpustate )
{
	CLR_NZV;
	SET_NZ16(X);
	WM16(cpustate, EAD,&pX);
}

/* $10aF STY indexed -**0- */
INLINE void sty_ix( konami_state *cpustate )
{
	CLR_NZV;
	SET_NZ16(Y);
	WM16(cpustate, EAD,&pY);
}

/* $b0 SUBA extended ?**** */
INLINE void suba_ex( konami_state *cpustate )
{
	UINT16    t,r;
	EXTBYTE(cpustate, t);
	r = A - t;
	CLR_NZVC;
	SET_FLAGS8(A,t,r);
	A = r;
}

/* $b1 CMPA extended ?**** */
INLINE void cmpa_ex( konami_state *cpustate )
{
	UINT16    t,r;
	EXTBYTE(cpustate, t);
	r = A - t;
	CLR_NZVC;
	SET_FLAGS8(A,t,r);
}

/* $b2 SBCA extended ?**** */
INLINE void sbca_ex( konami_state *cpustate )
{
	UINT16    t,r;
	EXTBYTE(cpustate, t);
	r = A - t - (CC & CC_C);
	CLR_NZVC;
	SET_FLAGS8(A,t,r);
	A = r;
}

/* $b3 SUBD (CMPD CMPU) extended -**** */
INLINE void subd_ex( konami_state *cpustate )
{
	UINT32 r,d;
	PAIR b;
	EXTWORD(cpustate, b);
	d = D;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
	D = r;
}

/* $10b3 CMPD extended -**** */
INLINE void cmpd_ex( konami_state *cpustate )
{
	UINT32 r,d;
	PAIR b;
	EXTWORD(cpustate, b);
	d = D;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
}

/* $11b3 CMPU extended -**** */
INLINE void cmpu_ex( konami_state *cpustate )
{
	UINT32 r,d;
	PAIR b;
	EXTWORD(cpustate, b);
	d = U;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
}

/* $b4 ANDA extended -**0- */
INLINE void anda_ex( konami_state *cpustate )
{
	UINT8 t;
	EXTBYTE(cpustate, t);
	A &= t;
	CLR_NZV;
	SET_NZ8(A);
}

/* $b5 BITA extended -**0- */
INLINE void bita_ex( konami_state *cpustate )
{
	UINT8 t,r;
	EXTBYTE(cpustate, t);
	r = A & t;
	CLR_NZV; SET_NZ8(r);
}

/* $b6 LDA extended -**0- */
INLINE void lda_ex( konami_state *cpustate )
{
	EXTBYTE(cpustate, A);
	CLR_NZV;
	SET_NZ8(A);
}

/* $b7 STA extended -**0- */
INLINE void sta_ex( konami_state *cpustate )
{
	CLR_NZV;
	SET_NZ8(A);
	EXTENDED(cpustate);
	WM(cpustate, EAD,A);
}

/* $b8 EORA extended -**0- */
INLINE void eora_ex( konami_state *cpustate )
{
	UINT8 t;
	EXTBYTE(cpustate, t);
	A ^= t;
	CLR_NZV;
	SET_NZ8(A);
}

/* $b9 ADCA extended ***** */
INLINE void adca_ex( konami_state *cpustate )
{
	UINT16 t,r;
	EXTBYTE(cpustate, t);
	r = A + t + (CC & CC_C);
	CLR_HNZVC;
	SET_FLAGS8(A,t,r);
	SET_H(A,t,r);
	A = r;
}

/* $bA ORA extended -**0- */
INLINE void ora_ex( konami_state *cpustate )
{
	UINT8 t;
	EXTBYTE(cpustate, t);
	A |= t;
	CLR_NZV;
	SET_NZ8(A);
}

/* $bB ADDA extended ***** */
INLINE void adda_ex( konami_state *cpustate )
{
	UINT16 t,r;
	EXTBYTE(cpustate, t);
	r = A + t;
	CLR_HNZVC;
	SET_FLAGS8(A,t,r);
	SET_H(A,t,r);
	A = r;
}

/* $bC CMPX (CMPY CMPS) extended -**** */
INLINE void cmpx_ex( konami_state *cpustate )
{
	UINT32 r,d;
	PAIR b;
	EXTWORD(cpustate, b);
	d = X;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
}

/* $10bC CMPY extended -**** */
INLINE void cmpy_ex( konami_state *cpustate )
{
	UINT32 r,d;
	PAIR b;
	EXTWORD(cpustate, b);
	d = Y;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
}

/* $11bC CMPS extended -**** */
INLINE void cmps_ex( konami_state *cpustate )
{
	UINT32 r,d;
	PAIR b;
	EXTWORD(cpustate, b);
	d = S;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
}

/* $bD JSR extended ----- */
INLINE void jsr_ex( konami_state *cpustate )
{
	EXTENDED(cpustate);
	PUSHWORD(cpustate, pPC);
	PCD=EAD;
}

/* $bE LDX (LDY) extended -**0- */
INLINE void ldx_ex( konami_state *cpustate )
{
	EXTWORD(cpustate, pX);
	CLR_NZV;
	SET_NZ16(X);
}

/* $10bE LDY extended -**0- */
INLINE void ldy_ex( konami_state *cpustate )
{
	EXTWORD(cpustate, pY);
	CLR_NZV;
	SET_NZ16(Y);
}

/* $bF STX (STY) extended -**0- */
INLINE void stx_ex( konami_state *cpustate )
{
	CLR_NZV;
	SET_NZ16(X);
	EXTENDED(cpustate);
	WM16(cpustate, EAD,&pX);
}

/* $10bF STY extended -**0- */
INLINE void sty_ex( konami_state *cpustate )
{
	CLR_NZV;
	SET_NZ16(Y);
	EXTENDED(cpustate);
	WM16(cpustate, EAD,&pY);
}

/* $c0 SUBB immediate ?**** */
INLINE void subb_im( konami_state *cpustate )
{
	UINT16    t,r;
	IMMBYTE(cpustate, t);
	r = B - t;
	CLR_NZVC;
	SET_FLAGS8(B,t,r);
	B = r;
}

/* $c1 CMPB immediate ?**** */
INLINE void cmpb_im( konami_state *cpustate )
{
	UINT16    t,r;
	IMMBYTE(cpustate, t);
	r = B - t;
	CLR_NZVC; SET_FLAGS8(B,t,r);
}

/* $c2 SBCB immediate ?**** */
INLINE void sbcb_im( konami_state *cpustate )
{
	UINT16    t,r;
	IMMBYTE(cpustate, t);
	r = B - t - (CC & CC_C);
	CLR_NZVC;
	SET_FLAGS8(B,t,r);
	B = r;
}

/* $c3 ADDD immediate -**** */
INLINE void addd_im( konami_state *cpustate )
{
	UINT32 r,d;
	PAIR b;
	IMMWORD(cpustate, b);
	d = D;
	r = d + b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
	D = r;
}

/* $c4 ANDB immediate -**0- */
INLINE void andb_im( konami_state *cpustate )
{
	UINT8 t;
	IMMBYTE(cpustate, t);
	B &= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $c5 BITB immediate -**0- */
INLINE void bitb_im( konami_state *cpustate )
{
	UINT8 t,r;
	IMMBYTE(cpustate, t);
	r = B & t;
	CLR_NZV;
	SET_NZ8(r);
}

/* $c6 LDB immediate -**0- */
INLINE void ldb_im( konami_state *cpustate )
{
	IMMBYTE(cpustate, B);
	CLR_NZV;
	SET_NZ8(B);
}

/* is this a legal instruction? */
/* $c7 STB immediate -**0- */
INLINE void stb_im( konami_state *cpustate )
{
	CLR_NZV;
	SET_NZ8(B);
	IMM8(cpustate);
	WM(cpustate, EAD,B);
}

/* $c8 EORB immediate -**0- */
INLINE void eorb_im( konami_state *cpustate )
{
	UINT8 t;
	IMMBYTE(cpustate, t);
	B ^= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $c9 ADCB immediate ***** */
INLINE void adcb_im( konami_state *cpustate )
{
	UINT16 t,r;
	IMMBYTE(cpustate, t);
	r = B + t + (CC & CC_C);
	CLR_HNZVC;
	SET_FLAGS8(B,t,r);
	SET_H(B,t,r);
	B = r;
}

/* $cA ORB immediate -**0- */
INLINE void orb_im( konami_state *cpustate )
{
	UINT8 t;
	IMMBYTE(cpustate, t);
	B |= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $cB ADDB immediate ***** */
INLINE void addb_im( konami_state *cpustate )
{
	UINT16 t,r;
	IMMBYTE(cpustate, t);
	r = B + t;
	CLR_HNZVC;
	SET_FLAGS8(B,t,r);
	SET_H(B,t,r);
	B = r;
}

/* $cC LDD immediate -**0- */
INLINE void ldd_im( konami_state *cpustate )
{
	IMMWORD(cpustate, pD);
	CLR_NZV;
	SET_NZ16(D);
}

/* is this a legal instruction? */
/* $cD STD immediate -**0- */
INLINE void std_im( konami_state *cpustate )
{
	CLR_NZV;
	SET_NZ16(D);
	IMM16(cpustate);
	WM16(cpustate, EAD,&pD);
}

/* $cE LDU (LDS) immediate -**0- */
INLINE void ldu_im( konami_state *cpustate )
{
	IMMWORD(cpustate, pU);
	CLR_NZV;
	SET_NZ16(U);
}

/* $10cE LDS immediate -**0- */
INLINE void lds_im( konami_state *cpustate )
{
	IMMWORD(cpustate, pS);
	CLR_NZV;
	SET_NZ16(S);
	cpustate->int_state |= KONAMI_LDS;
}

/* is this a legal instruction? */
/* $cF STU (STS) immediate -**0- */
INLINE void stu_im( konami_state *cpustate )
{
	CLR_NZV;
	SET_NZ16(U);
	IMM16(cpustate);
	WM16(cpustate, EAD,&pU);
}

/* is this a legal instruction? */
/* $10cF STS immediate -**0- */
INLINE void sts_im( konami_state *cpustate )
{
	CLR_NZV;
	SET_NZ16(S);
	IMM16(cpustate);
	WM16(cpustate, EAD,&pS);
}

/* $d0 SUBB direct ?**** */
INLINE void subb_di( konami_state *cpustate )
{
	UINT16    t,r;
	DIRBYTE(cpustate, t);
	r = B - t;
	CLR_NZVC;
	SET_FLAGS8(B,t,r);
	B = r;
}

/* $d1 CMPB direct ?**** */
INLINE void cmpb_di( konami_state *cpustate )
{
	UINT16    t,r;
	DIRBYTE(cpustate, t);
	r = B - t;
	CLR_NZVC;
	SET_FLAGS8(B,t,r);
}

/* $d2 SBCB direct ?**** */
INLINE void sbcb_di( konami_state *cpustate )
{
	UINT16    t,r;
	DIRBYTE(cpustate, t);
	r = B - t - (CC & CC_C);
	CLR_NZVC;
	SET_FLAGS8(B,t,r);
	B = r;
}

/* $d3 ADDD direct -**** */
INLINE void addd_di( konami_state *cpustate )
{
	UINT32 r,d;
	PAIR b;
	DIRWORD(cpustate, b);
	d = D;
	r = d + b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
	D = r;
}

/* $d4 ANDB direct -**0- */
INLINE void andb_di( konami_state *cpustate )
{
	UINT8 t;
	DIRBYTE(cpustate, t);
	B &= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $d5 BITB direct -**0- */
INLINE void bitb_di( konami_state *cpustate )
{
	UINT8 t,r;
	DIRBYTE(cpustate, t);
	r = B & t;
	CLR_NZV;
	SET_NZ8(r);
}

/* $d6 LDB direct -**0- */
INLINE void ldb_di( konami_state *cpustate )
{
	DIRBYTE(cpustate, B);
	CLR_NZV;
	SET_NZ8(B);
}

/* $d7 STB direct -**0- */
INLINE void stb_di( konami_state *cpustate )
{
	CLR_NZV;
	SET_NZ8(B);
	DIRECT(cpustate);
	WM(cpustate, EAD,B);
}

/* $d8 EORB direct -**0- */
INLINE void eorb_di( konami_state *cpustate )
{
	UINT8 t;
	DIRBYTE(cpustate, t);
	B ^= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $d9 ADCB direct ***** */
INLINE void adcb_di( konami_state *cpustate )
{
	UINT16 t,r;
	DIRBYTE(cpustate, t);
	r = B + t + (CC & CC_C);
	CLR_HNZVC;
	SET_FLAGS8(B,t,r);
	SET_H(B,t,r);
	B = r;
}

/* $dA ORB direct -**0- */
INLINE void orb_di( konami_state *cpustate )
{
	UINT8 t;
	DIRBYTE(cpustate, t);
	B |= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $dB ADDB direct ***** */
INLINE void addb_di( konami_state *cpustate )
{
	UINT16 t,r;
	DIRBYTE(cpustate, t);
	r = B + t;
	CLR_HNZVC;
	SET_FLAGS8(B,t,r);
	SET_H(B,t,r);
	B = r;
}

/* $dC LDD direct -**0- */
INLINE void ldd_di( konami_state *cpustate )
{
	DIRWORD(cpustate, pD);
	CLR_NZV;
	SET_NZ16(D);
}

/* $dD STD direct -**0- */
INLINE void std_di( konami_state *cpustate )
{
	CLR_NZV;
	SET_NZ16(D);
	DIRECT(cpustate);
	WM16(cpustate, EAD,&pD);
}

/* $dE LDU (LDS) direct -**0- */
INLINE void ldu_di( konami_state *cpustate )
{
	DIRWORD(cpustate, pU);
	CLR_NZV;
	SET_NZ16(U);
}

/* $10dE LDS direct -**0- */
INLINE void lds_di( konami_state *cpustate )
{
	DIRWORD(cpustate, pS);
	CLR_NZV;
	SET_NZ16(S);
	cpustate->int_state |= KONAMI_LDS;
}

/* $dF STU (STS) direct -**0- */
INLINE void stu_di( konami_state *cpustate )
{
	CLR_NZV;
	SET_NZ16(U);
	DIRECT(cpustate);
	WM16(cpustate, EAD,&pU);
}

/* $10dF STS direct -**0- */
INLINE void sts_di( konami_state *cpustate )
{
	CLR_NZV;
	SET_NZ16(S);
	DIRECT(cpustate);
	WM16(cpustate, EAD,&pS);
}

/* $e0 SUBB indexed ?**** */
INLINE void subb_ix( konami_state *cpustate )
{
	UINT16    t,r;
	t = RM(cpustate, EAD);
	r = B - t;
	CLR_NZVC;
	SET_FLAGS8(B,t,r);
	B = r;
}

/* $e1 CMPB indexed ?**** */
INLINE void cmpb_ix( konami_state *cpustate )
{
	UINT16    t,r;
	t = RM(cpustate, EAD);
	r = B - t;
	CLR_NZVC;
	SET_FLAGS8(B,t,r);
}

/* $e2 SBCB indexed ?**** */
INLINE void sbcb_ix( konami_state *cpustate )
{
	UINT16    t,r;
	t = RM(cpustate, EAD);
	r = B - t - (CC & CC_C);
	CLR_NZVC;
	SET_FLAGS8(B,t,r);
	B = r;
}

/* $e3 ADDD indexed -**** */
INLINE void addd_ix( konami_state *cpustate )
{
	UINT32 r,d;
	PAIR b;
	b.d=RM16(cpustate, EAD);
	d = D;
	r = d + b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
	D = r;
}

/* $e4 ANDB indexed -**0- */
INLINE void andb_ix( konami_state *cpustate )
{
	B &= RM(cpustate, EAD);
	CLR_NZV;
	SET_NZ8(B);
}

/* $e5 BITB indexed -**0- */
INLINE void bitb_ix( konami_state *cpustate )
{
	UINT8 r;
	r = B & RM(cpustate, EAD);
	CLR_NZV;
	SET_NZ8(r);
}

/* $e6 LDB indexed -**0- */
INLINE void ldb_ix( konami_state *cpustate )
{
	B = RM(cpustate, EAD);
	CLR_NZV;
	SET_NZ8(B);
}

/* $e7 STB indexed -**0- */
INLINE void stb_ix( konami_state *cpustate )
{
	CLR_NZV;
	SET_NZ8(B);
	WM(cpustate, EAD,B);
}

/* $e8 EORB indexed -**0- */
INLINE void eorb_ix( konami_state *cpustate )
{
	B ^= RM(cpustate, EAD);
	CLR_NZV;
	SET_NZ8(B);
}

/* $e9 ADCB indexed ***** */
INLINE void adcb_ix( konami_state *cpustate )
{
	UINT16 t,r;
	t = RM(cpustate, EAD);
	r = B + t + (CC & CC_C);
	CLR_HNZVC;
	SET_FLAGS8(B,t,r);
	SET_H(B,t,r);
	B = r;
}

/* $eA ORB indexed -**0- */
INLINE void orb_ix( konami_state *cpustate )
{
	B |= RM(cpustate, EAD);
	CLR_NZV;
	SET_NZ8(B);
}

/* $eb ADDB indexed ***** */
INLINE void addb_ix( konami_state *cpustate )
{
	UINT16 t,r;
	t = RM(cpustate, EAD);
	r = B + t;
	CLR_HNZVC;
	SET_FLAGS8(B,t,r);
	SET_H(B,t,r);
	B = r;
}

/* $ec LDD indexed -**0- */
INLINE void ldd_ix( konami_state *cpustate )
{
	D=RM16(cpustate, EAD);
	CLR_NZV; SET_NZ16(D);
}

/* $eD STD indexed -**0- */
INLINE void std_ix( konami_state *cpustate )
{
	CLR_NZV;
	SET_NZ16(D);
	WM16(cpustate, EAD,&pD);
}

/* $eE LDU (LDS) indexed -**0- */
INLINE void ldu_ix( konami_state *cpustate )
{
	U=RM16(cpustate, EAD);
	CLR_NZV;
	SET_NZ16(U);
}

/* $10eE LDS indexed -**0- */
INLINE void lds_ix( konami_state *cpustate )
{
	S=RM16(cpustate, EAD);
	CLR_NZV;
	SET_NZ16(S);
	cpustate->int_state |= KONAMI_LDS;
}

/* $eF STU (STS) indexed -**0- */
INLINE void stu_ix( konami_state *cpustate )
{
	CLR_NZV;
	SET_NZ16(U);
	WM16(cpustate, EAD,&pU);
}

/* $10eF STS indexed -**0- */
INLINE void sts_ix( konami_state *cpustate )
{
	CLR_NZV;
	SET_NZ16(S);
	WM16(cpustate, EAD,&pS);
}

/* $f0 SUBB extended ?**** */
INLINE void subb_ex( konami_state *cpustate )
{
	UINT16    t,r;
	EXTBYTE(cpustate, t);
	r = B - t;
	CLR_NZVC;
	SET_FLAGS8(B,t,r);
	B = r;
}

/* $f1 CMPB extended ?**** */
INLINE void cmpb_ex( konami_state *cpustate )
{
	UINT16    t,r;
	EXTBYTE(cpustate, t);
	r = B - t;
	CLR_NZVC;
	SET_FLAGS8(B,t,r);
}

/* $f2 SBCB extended ?**** */
INLINE void sbcb_ex( konami_state *cpustate )
{
	UINT16    t,r;
	EXTBYTE(cpustate, t);
	r = B - t - (CC & CC_C);
	CLR_NZVC;
	SET_FLAGS8(B,t,r);
	B = r;
}

/* $f3 ADDD extended -**** */
INLINE void addd_ex( konami_state *cpustate )
{
	UINT32 r,d;
	PAIR b;
	EXTWORD(cpustate, b);
	d = D;
	r = d + b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
	D = r;
}

/* $f4 ANDB extended -**0- */
INLINE void andb_ex( konami_state *cpustate )
{
	UINT8 t;
	EXTBYTE(cpustate, t);
	B &= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $f5 BITB extended -**0- */
INLINE void bitb_ex( konami_state *cpustate )
{
	UINT8 t,r;
	EXTBYTE(cpustate, t);
	r = B & t;
	CLR_NZV;
	SET_NZ8(r);
}

/* $f6 LDB extended -**0- */
INLINE void ldb_ex( konami_state *cpustate )
{
	EXTBYTE(cpustate, B);
	CLR_NZV;
	SET_NZ8(B);
}

/* $f7 STB extended -**0- */
INLINE void stb_ex( konami_state *cpustate )
{
	CLR_NZV;
	SET_NZ8(B);
	EXTENDED(cpustate);
	WM(cpustate, EAD,B);
}

/* $f8 EORB extended -**0- */
INLINE void eorb_ex( konami_state *cpustate )
{
	UINT8 t;
	EXTBYTE(cpustate, t);
	B ^= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $f9 ADCB extended ***** */
INLINE void adcb_ex( konami_state *cpustate )
{
	UINT16 t,r;
	EXTBYTE(cpustate, t);
	r = B + t + (CC & CC_C);
	CLR_HNZVC;
	SET_FLAGS8(B,t,r);
	SET_H(B,t,r);
	B = r;
}

/* $fA ORB extended -**0- */
INLINE void orb_ex( konami_state *cpustate )
{
	UINT8 t;
	EXTBYTE(cpustate, t);
	B |= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $fB ADDB extended ***** */
INLINE void addb_ex( konami_state *cpustate )
{
	UINT16 t,r;
	EXTBYTE(cpustate, t);
	r = B + t;
	CLR_HNZVC;
	SET_FLAGS8(B,t,r);
	SET_H(B,t,r);
	B = r;
}

/* $fC LDD extended -**0- */
INLINE void ldd_ex( konami_state *cpustate )
{
	EXTWORD(cpustate, pD);
	CLR_NZV;
	SET_NZ16(D);
}

/* $fD STD extended -**0- */
INLINE void std_ex( konami_state *cpustate )
{
	CLR_NZV;
	SET_NZ16(D);
	EXTENDED(cpustate);
	WM16(cpustate, EAD,&pD);
}

/* $fE LDU (LDS) extended -**0- */
INLINE void ldu_ex( konami_state *cpustate )
{
	EXTWORD(cpustate, pU);
	CLR_NZV;
	SET_NZ16(U);
}

/* $10fE LDS extended -**0- */
INLINE void lds_ex( konami_state *cpustate )
{
	EXTWORD(cpustate, pS);
	CLR_NZV;
	SET_NZ16(S);
	cpustate->int_state |= KONAMI_LDS;
}

/* $fF STU (STS) extended -**0- */
INLINE void stu_ex( konami_state *cpustate )
{
	CLR_NZV;
	SET_NZ16(U);
	EXTENDED(cpustate);
	WM16(cpustate, EAD,&pU);
}

/* $10fF STS extended -**0- */
INLINE void sts_ex( konami_state *cpustate )
{
	CLR_NZV;
	SET_NZ16(S);
	EXTENDED(cpustate);
	WM16(cpustate, EAD,&pS);
}

INLINE void setline_im( konami_state *cpustate )
{
	UINT8 t;
	IMMBYTE(cpustate, t);

	if ( cpustate->setlines_callback )
		(*cpustate->setlines_callback)( cpustate->device, t );
}

INLINE void setline_ix( konami_state *cpustate )
{
	UINT8 t;
	t = RM(cpustate, EA);

	if ( cpustate->setlines_callback )
		(*cpustate->setlines_callback)( cpustate->device, t );
}

INLINE void setline_di( konami_state *cpustate )
{
	UINT8 t;
	DIRBYTE(cpustate, t);

	if ( cpustate->setlines_callback )
		(*cpustate->setlines_callback)( cpustate->device, t );
}

INLINE void setline_ex( konami_state *cpustate )
{
	UINT8 t;
	EXTBYTE(cpustate, t);

	if ( cpustate->setlines_callback )
		(*cpustate->setlines_callback)( cpustate->device, t );
}

INLINE void bmove( konami_state *cpustate )
{
	UINT8   t;

	while( U != 0 ) {
		t = RM(cpustate, Y);
		WM(cpustate, X,t);
		Y++;
		X++;
		U--;
		cpustate->icount -= 2;
	}
}

INLINE void move( konami_state *cpustate )
{
	UINT8   t;

	t = RM(cpustate, Y);
	WM(cpustate, X,t);
	Y++;
	X++;
	U--;
}

/* CLRD inherent -0100 */
INLINE void clrd( konami_state *cpustate )
{
	D = 0;
	CLR_NZVC; SEZ;
}

/* CLRW indexed -0100 */
INLINE void clrw_ix( konami_state *cpustate )
{
	PAIR t;
	t.d = 0;
	WM16(cpustate, EAD,&t);
	CLR_NZVC; SEZ;
}

/* CLRW direct -0100 */
INLINE void clrw_di( konami_state *cpustate )
{
	PAIR t;
	t.d = 0;
	DIRECT(cpustate);
	WM16(cpustate, EAD,&t);
	CLR_NZVC;
	SEZ;
}

/* CLRW extended -0100 */
INLINE void clrw_ex( konami_state *cpustate )
{
	PAIR t;
	t.d = 0;
	EXTENDED(cpustate);
	WM16(cpustate, EAD,&t);
	CLR_NZVC; SEZ;
}

/* LSRD immediate -0*-* */
INLINE void lsrd( konami_state *cpustate )
{
	UINT8 t;

	IMMBYTE(cpustate,  t );

	while ( t-- ) {
		CLR_NZC;
		CC |= (D & CC_C);
		D >>= 1;
		SET_Z16(D);
	}
}

/* RORD immediate -**-* */
INLINE void rord( konami_state *cpustate )
{
	UINT16 r;
	UINT8  t;

	IMMBYTE(cpustate, t);

	while ( t-- ) {
		r = (CC & CC_C) << 15;
		CLR_NZC;
		CC |= (D & CC_C);
		r |= D >> 1;
		SET_NZ16(r);
		D = r;
	}
}

/* ASRD immediate ?**-* */
INLINE void asrd( konami_state *cpustate )
{
	UINT8 t;

	IMMBYTE(cpustate, t);

	while ( t-- ) {
		CLR_NZC;
		CC |= (D & CC_C);
		D = (D & 0x8000) | (D >> 1);
		SET_NZ16(D);
	}
}

/* ASLD immediate ?**** */
INLINE void asld( konami_state *cpustate )
{
	UINT32  r;
	UINT8   t;

	IMMBYTE(cpustate,  t );

	while ( t-- ) {
		r = D << 1;
		CLR_NZVC;
		SET_FLAGS16(D,D,r);
		D = r;
	}
}

/* ROLD immediate -**-* */
INLINE void rold( konami_state *cpustate )
{
	UINT16 r;
	UINT8  t;

	IMMBYTE(cpustate, t);

	while ( t-- ) {
		CLR_NZC;
		if ( D & 0x8000 ) SEC;
		r = CC & CC_C;
		r |= D << 1;
		SET_NZ16(r);
		D = r;
	}
}

/* DECB,JNZ relative ----- */
INLINE void decbjnz( konami_state *cpustate )
{
	--B;
	CLR_NZV;
	SET_FLAGS8D(B);
	BRANCH(cpustate, !(CC&CC_Z) );
}

/* DECX,JNZ relative ----- */
INLINE void decxjnz( konami_state *cpustate )
{
	--X;
	CLR_NZV;
	SET_NZ16(X);    /* should affect V as well? */
	BRANCH(cpustate, !(CC&CC_Z) );
}

INLINE void bset( konami_state *cpustate )
{
	UINT8   t;

	while( U != 0 ) {
		t = A;
		WM(cpustate, XD,t);
		X++;
		U--;
		cpustate->icount -= 2;
	}
}

INLINE void bset2( konami_state *cpustate )
{
	while( U != 0 ) {
		WM16(cpustate, XD,&pD);
		X += 2;
		U--;
		cpustate->icount -= 3;
	}
}

/* LMUL inherent --*-@ */
INLINE void lmul( konami_state *cpustate )
{
	UINT32 t;
	t = X * Y;
	X = (t >> 16);
	Y = (t & 0xffff);
	CLR_ZC; SET_Z(t); if( t & 0x8000 ) SEC;
}

/* DIVX inherent --*-@ */
INLINE void divx( konami_state *cpustate )
{
	UINT16 t;
	UINT8 r;
	if ( B != 0 )
	{
		t = X / B;
		r = X % B;
	}
	else
	{
		/* ?? */
		t = 0;
		r = 0;
	}
	CLR_ZC; SET_Z16(t); if ( t & 0x80 ) SEC;
	X = t;
	B = r;
}

/* INCD inherent -***- */
INLINE void incd( konami_state *cpustate )
{
	UINT32 r;
	r = D + 1;
	CLR_NZV;
	SET_FLAGS16(D,D,r);
	D = r;
}

/* INCW direct -***- */
INLINE void incw_di( konami_state *cpustate )
{
	PAIR t,r;
	DIRWORD(cpustate, t);
	r = t;
	++r.d;
	CLR_NZV;
	SET_FLAGS16(t.d, t.d, r.d);
	WM16(cpustate, EAD,&r);
}

/* INCW indexed -***- */
INLINE void incw_ix( konami_state *cpustate )
{
	PAIR t,r;
	t.d=RM16(cpustate, EAD);
	r = t;
	++r.d;
	CLR_NZV;
	SET_FLAGS16(t.d, t.d, r.d);
	WM16(cpustate, EAD,&r);
}

/* INCW extended -***- */
INLINE void incw_ex( konami_state *cpustate )
{
	PAIR t, r;
	EXTWORD(cpustate, t);
	r = t;
	++r.d;
	CLR_NZV; SET_FLAGS16(t.d,t.d,r.d);
	WM16(cpustate, EAD,&r);
}

/* DECD inherent -***- */
INLINE void decd( konami_state *cpustate )
{
	UINT32 r;
	r = D - 1;
	CLR_NZV;
	SET_FLAGS16(D,D,r);
	D = r;
}

/* DECW direct -***- */
INLINE void decw_di( konami_state *cpustate )
{
	PAIR t,r;
	DIRWORD(cpustate, t);
	r = t;
	--r.d;
	CLR_NZV;
	SET_FLAGS16(t.d, t.d, r.d);
	WM16(cpustate, EAD,&r);
}

/* DECW indexed -***- */
INLINE void decw_ix( konami_state *cpustate )
{
	PAIR t, r;
	t.d=RM16(cpustate, EAD);
	r = t;
	--r.d;
	CLR_NZV; SET_FLAGS16(t.d, t.d, r.d);
	WM16(cpustate, EAD,&r);
}

/* DECW extended -***- */
INLINE void decw_ex( konami_state *cpustate )
{
	PAIR t, r;
	EXTWORD(cpustate, t);
	r = t;
	--r.d;
	CLR_NZV; SET_FLAGS16(t.d,t.d,r.d);
	WM16(cpustate, EAD,&r);
}

/* TSTD inherent -**0- */
INLINE void tstd( konami_state *cpustate )
{
	CLR_NZV;
	SET_NZ16(D);
}

/* TSTW direct -**0- */
INLINE void tstw_di( konami_state *cpustate )
{
	PAIR t;
	CLR_NZV;
	DIRWORD(cpustate, t);
	SET_NZ16(t.d);
}

/* TSTW indexed -**0- */
INLINE void tstw_ix( konami_state *cpustate )
{
	PAIR t;
	CLR_NZV;
	t.d=RM16(cpustate, EAD);
	SET_NZ16(t.d);
}

/* TSTW extended -**0- */
INLINE void tstw_ex( konami_state *cpustate )
{
	PAIR t;
	CLR_NZV;
	EXTWORD(cpustate, t);
	SET_NZ16(t.d);
}

/* LSRW direct -0*-* */
INLINE void lsrw_di( konami_state *cpustate )
{
	PAIR t;
	DIRWORD(cpustate, t);
	CLR_NZC;
	CC |= (t.d & CC_C);
	t.d >>= 1;
	SET_Z16(t.d);
	WM16(cpustate, EAD,&t);
}

/* LSRW indexed -0*-* */
INLINE void lsrw_ix( konami_state *cpustate )
{
	PAIR t;
	t.d=RM16(cpustate, EAD);
	CLR_NZC;
	CC |= (t.d & CC_C);
	t.d >>= 1;
	SET_Z16(t.d);
	WM16(cpustate, EAD,&t);
}

/* LSRW extended -0*-* */
INLINE void lsrw_ex( konami_state *cpustate )
{
	PAIR t;
	EXTWORD(cpustate, t);
	CLR_NZC;
	CC |= (t.d & CC_C);
	t.d >>= 1;
	SET_Z16(t.d);
	WM16(cpustate, EAD,&t);
}

/* RORW direct -**-* */
INLINE void rorw_di( konami_state *cpustate )
{
	PAIR t,r;
	DIRWORD(cpustate, t);
	r.d = (CC & CC_C) << 15;
	CLR_NZC;
	CC |= (t.d & CC_C);
	r.d |= t.d>>1;
	SET_NZ16(r.d);
	WM16(cpustate, EAD,&r);
}

/* RORW indexed -**-* */
INLINE void rorw_ix( konami_state *cpustate )
{
	PAIR t,r;
	t.d=RM16(cpustate, EAD);
	r.d = (CC & CC_C) << 15;
	CLR_NZC;
	CC |= (t.d & CC_C);
	r.d |= t.d>>1;
	SET_NZ16(r.d);
	WM16(cpustate, EAD,&r);
}

/* RORW extended -**-* */
INLINE void rorw_ex( konami_state *cpustate )
{
	PAIR t,r;
	EXTWORD(cpustate, t);
	r.d = (CC & CC_C) << 15;
	CLR_NZC;
	CC |= (t.d & CC_C);
	r.d |= t.d>>1;
	SET_NZ16(r.d);
	WM16(cpustate, EAD,&r);
}

/* ASRW direct ?**-* */
INLINE void asrw_di( konami_state *cpustate )
{
	PAIR t;
	DIRWORD(cpustate, t);
	CLR_NZC;
	CC |= (t.d & CC_C);
	t.d = (t.d & 0x8000) | (t.d >> 1);
	SET_NZ16(t.d);
	WM16(cpustate, EAD,&t);
}

/* ASRW indexed ?**-* */
INLINE void asrw_ix( konami_state *cpustate )
{
	PAIR t;
	t.d=RM16(cpustate, EAD);
	CLR_NZC;
	CC |= (t.d & CC_C);
	t.d = (t.d & 0x8000) | (t.d >> 1);
	SET_NZ16(t.d);
	WM16(cpustate, EAD,&t);
}

/* ASRW extended ?**-* */
INLINE void asrw_ex( konami_state *cpustate )
{
	PAIR t;
	EXTWORD(cpustate, t);
	CLR_NZC;
	CC |= (t.d & CC_C);
	t.d = (t.d & 0x8000) | (t.d >> 1);
	SET_NZ16(t.d);
	WM16(cpustate, EAD,&t);
}

/* ASLW direct ?**** */
INLINE void aslw_di( konami_state *cpustate )
{
	PAIR t,r;
	DIRWORD(cpustate, t);
	r.d = t.d << 1;
	CLR_NZVC;
	SET_FLAGS16(t.d,t.d,r.d);
	WM16(cpustate, EAD,&r);
}

/* ASLW indexed ?**** */
INLINE void aslw_ix( konami_state *cpustate )
{
	PAIR t,r;
	t.d=RM16(cpustate, EAD);
	r.d = t.d << 1;
	CLR_NZVC;
	SET_FLAGS16(t.d,t.d,r.d);
	WM16(cpustate, EAD,&r);
}

/* ASLW extended ?**** */
INLINE void aslw_ex( konami_state *cpustate )
{
	PAIR t,r;
	EXTWORD(cpustate, t);
	r.d = t.d << 1;
	CLR_NZVC;
	SET_FLAGS16(t.d,t.d,r.d);
	WM16(cpustate, EAD,&r);
}

/* ROLW direct -**** */
INLINE void rolw_di( konami_state *cpustate )
{
	PAIR t,r;
	DIRWORD(cpustate, t);
	r.d = (CC & CC_C) | (t.d << 1);
	CLR_NZVC;
	SET_FLAGS16(t.d,t.d,r.d);
	WM16(cpustate, EAD,&r);
}

/* ROLW indexed -**** */
INLINE void rolw_ix( konami_state *cpustate )
{
	PAIR t,r;
	t.d=RM16(cpustate, EAD);
	r.d = (CC & CC_C) | (t.d << 1);
	CLR_NZVC;
	SET_FLAGS16(t.d,t.d,r.d);
	WM16(cpustate, EAD,&r);
}

/* ROLW extended -**** */
INLINE void rolw_ex( konami_state *cpustate )
{
	PAIR t,r;
	EXTWORD(cpustate, t);
	r.d = (CC & CC_C) | (t.d << 1);
	CLR_NZVC;
	SET_FLAGS16(t.d,t.d,r.d);
	WM16(cpustate, EAD,&r);
}

/* NEGD inherent ?**** */
INLINE void negd( konami_state *cpustate )
{
	UINT32 r;
	r = -D;
	CLR_NZVC;
	SET_FLAGS16(0,D,r);
	D = r;
}

/* NEGW direct ?**** */
INLINE void negw_di( konami_state *cpustate )
{
	PAIR r,t;
	DIRWORD(cpustate, t);
	r.d = -t.d;
	CLR_NZVC;
	SET_FLAGS16(0,t.d,r.d);
	WM16(cpustate, EAD,&r);
}

/* NEGW indexed ?**** */
INLINE void negw_ix( konami_state *cpustate )
{
	PAIR r,t;
	t.d=RM16(cpustate, EAD);
	r.d = -t.d;
	CLR_NZVC;
	SET_FLAGS16(0,t.d,r.d);
	WM16(cpustate, EAD,&r);
}

/* NEGW extended ?**** */
INLINE void negw_ex( konami_state *cpustate )
{
	PAIR r,t;
	EXTWORD(cpustate, t);
	r.d = -t.d;
	CLR_NZVC;
	SET_FLAGS16(0,t.d,r.d);
	WM16(cpustate, EAD,&r);
}

/* ABSA inherent ?**** */
INLINE void absa( konami_state *cpustate )
{
	UINT16 r;
	if (A & 0x80)
		r = -A;
	else
		r = A;
	CLR_NZVC;
	SET_FLAGS8(0,A,r);
	A = r;
}

/* ABSB inherent ?**** */
INLINE void absb( konami_state *cpustate )
{
	UINT16 r;
	if (B & 0x80)
		r = -B;
	else
		r = B;
	CLR_NZVC;
	SET_FLAGS8(0,B,r);
	B = r;
}

/* ABSD inherent ?**** */
INLINE void absd( konami_state *cpustate )
{
	UINT32 r;
	if (D & 0x8000)
		r = -D;
	else
		r = D;
	CLR_NZVC;
	SET_FLAGS16(0,D,r);
	D = r;
}

/* LSRD direct -0*-* */
INLINE void lsrd_di( konami_state *cpustate )
{
	UINT8 t;

	DIRBYTE(cpustate,  t );

	while ( t-- ) {
		CLR_NZC;
		CC |= (D & CC_C);
		D >>= 1;
		SET_Z16(D);
	}
}

/* RORD direct -**-* */
INLINE void rord_di( konami_state *cpustate )
{
	UINT16 r;
	UINT8  t;

	DIRBYTE(cpustate, t);

	while ( t-- ) {
		r = (CC & CC_C) << 15;
		CLR_NZC;
		CC |= (D & CC_C);
		r |= D >> 1;
		SET_NZ16(r);
		D = r;
	}
}

/* ASRD direct ?**-* */
INLINE void asrd_di( konami_state *cpustate )
{
	UINT8 t;

	DIRBYTE(cpustate, t);

	while ( t-- ) {
		CLR_NZC;
		CC |= (D & CC_C);
		D = (D & 0x8000) | (D >> 1);
		SET_NZ16(D);
	}
}

/* ASLD direct ?**** */
INLINE void asld_di( konami_state *cpustate )
{
	UINT32  r;
	UINT8   t;

	DIRBYTE(cpustate,  t );

	while ( t-- ) {
		r = D << 1;
		CLR_NZVC;
		SET_FLAGS16(D,D,r);
		D = r;
	}
}

/* ROLD direct -**-* */
INLINE void rold_di( konami_state *cpustate )
{
	UINT16 r;
	UINT8  t;

	DIRBYTE(cpustate, t);

	while ( t-- ) {
		CLR_NZC;
		if ( D & 0x8000 ) SEC;
		r = CC & CC_C;
		r |= D << 1;
		SET_NZ16(r);
		D = r;
	}
}

/* LSRD indexed -0*-* */
INLINE void lsrd_ix( konami_state *cpustate )
{
	UINT8 t;

	t=RM(cpustate, EA);

	while ( t-- ) {
		CLR_NZC;
		CC |= (D & CC_C);
		D >>= 1;
		SET_Z16(D);
	}
}

/* RORD indexed -**-* */
INLINE void rord_ix( konami_state *cpustate )
{
	UINT16 r;
	UINT8  t;

	t=RM(cpustate, EA);

	while ( t-- ) {
		r = (CC & CC_C) << 15;
		CLR_NZC;
		CC |= (D & CC_C);
		r |= D >> 1;
		SET_NZ16(r);
		D = r;
	}
}

/* ASRD indexed ?**-* */
INLINE void asrd_ix( konami_state *cpustate )
{
	UINT8 t;

	t=RM(cpustate, EA);

	while ( t-- ) {
		CLR_NZC;
		CC |= (D & CC_C);
		D = (D & 0x8000) | (D >> 1);
		SET_NZ16(D);
	}
}

/* ASLD indexed ?**** */
INLINE void asld_ix( konami_state *cpustate )
{
	UINT32  r;
	UINT8   t;

	t=RM(cpustate, EA);

	while ( t-- ) {
		r = D << 1;
		CLR_NZVC;
		SET_FLAGS16(D,D,r);
		D = r;
	}
}

/* ROLD indexed -**-* */
INLINE void rold_ix( konami_state *cpustate )
{
	UINT16 r;
	UINT8  t;

	t=RM(cpustate, EA);

	while ( t-- ) {
		CLR_NZC;
		if ( D & 0x8000 ) SEC;
		r = CC & CC_C;
		r |= D << 1;
		SET_NZ16(r);
		D = r;
	}
}

/* LSRD extended -0*-* */
INLINE void lsrd_ex( konami_state *cpustate )
{
	UINT8 t;

	EXTBYTE(cpustate, t);

	while ( t-- ) {
		CLR_NZC;
		CC |= (D & CC_C);
		D >>= 1;
		SET_Z16(D);
	}
}

/* RORD extended -**-* */
INLINE void rord_ex( konami_state *cpustate )
{
	UINT16 r;
	UINT8  t;

	EXTBYTE(cpustate, t);

	while ( t-- ) {
		r = (CC & CC_C) << 15;
		CLR_NZC;
		CC |= (D & CC_C);
		r |= D >> 1;
		SET_NZ16(r);
		D = r;
	}
}

/* ASRD extended ?**-* */
INLINE void asrd_ex( konami_state *cpustate )
{
	UINT8 t;

	EXTBYTE(cpustate, t);

	while ( t-- ) {
		CLR_NZC;
		CC |= (D & CC_C);
		D = (D & 0x8000) | (D >> 1);
		SET_NZ16(D);
	}
}

/* ASLD extended ?**** */
INLINE void asld_ex( konami_state *cpustate )
{
	UINT32  r;
	UINT8   t;

	EXTBYTE(cpustate, t);

	while ( t-- ) {
		r = D << 1;
		CLR_NZVC;
		SET_FLAGS16(D,D,r);
		D = r;
	}
}

/* ROLD extended -**-* */
INLINE void rold_ex( konami_state *cpustate )
{
	UINT16 r;
	UINT8  t;

	EXTBYTE(cpustate, t);

	while ( t-- ) {
		CLR_NZC;
		if ( D & 0x8000 ) SEC;
		r = CC & CC_C;
		r |= D << 1;
		SET_NZ16(r);
		D = r;
	}
}

INLINE void opcode2( konami_state *cpustate )
{
	UINT8 ireg2 = ROP_ARG(cpustate, PCD);
	PC++;

	switch ( ireg2 ) {
//  case 0x00: EA=0; break; /* auto increment */
//  case 0x01: EA=0; break; /* double auto increment */
//  case 0x02: EA=0; break; /* auto decrement */
//  case 0x03: EA=0; break; /* double auto decrement */
//  case 0x04: EA=0; break; /* postbyte offs */
//  case 0x05: EA=0; break; /* postword offs */
//  case 0x06: EA=0; break; /* normal */
	case 0x07:
		EAD=0;
		(*konami_extended[cpustate->ireg])(cpustate);
		cpustate->icount -= 2;
		return;
//  case 0x08: EA=0; break; /* indirect - auto increment */
//  case 0x09: EA=0; break; /* indirect - double auto increment */
//  case 0x0a: EA=0; break; /* indirect - auto decrement */
//  case 0x0b: EA=0; break; /* indirect - double auto decrement */
//  case 0x0c: EA=0; break; /* indirect - postbyte offs */
//  case 0x0d: EA=0; break; /* indirect - postword offs */
//  case 0x0e: EA=0; break; /* indirect - normal */
	case 0x0f:              /* indirect - extended */
		IMMWORD(cpustate, cpustate->ea);
		EA=RM16(cpustate, EAD);
		cpustate->icount-=4;
		break;
//  case 0x10: EA=0; break; /* auto increment */
//  case 0x11: EA=0; break; /* double auto increment */
//  case 0x12: EA=0; break; /* auto decrement */
//  case 0x13: EA=0; break; /* double auto decrement */
//  case 0x14: EA=0; break; /* postbyte offs */
//  case 0x15: EA=0; break; /* postword offs */
//  case 0x16: EA=0; break; /* normal */
//  case 0x17: EA=0; break; /* extended */
//  case 0x18: EA=0; break; /* indirect - auto increment */
//  case 0x19: EA=0; break; /* indirect - double auto increment */
//  case 0x1a: EA=0; break; /* indirect - auto decrement */
//  case 0x1b: EA=0; break; /* indirect - double auto decrement */
//  case 0x1c: EA=0; break; /* indirect - postbyte offs */
//  case 0x1d: EA=0; break; /* indirect - postword offs */
//  case 0x1e: EA=0; break; /* indirect - normal */
//  case 0x1f: EA=0; break; /* indirect - extended */

/* base X */
	case 0x20:              /* auto increment */
		EA=X;
		X++;
		cpustate->icount-=2;
		break;
	case 0x21:              /* double auto increment */
		EA=X;
		X+=2;
		cpustate->icount-=3;
		break;
	case 0x22:              /* auto decrement */
		X--;
		EA=X;
		cpustate->icount-=2;
		break;
	case 0x23:              /* double auto decrement */
		X-=2;
		EA=X;
		cpustate->icount-=3;
		break;
	case 0x24:              /* postbyte offs */
		IMMBYTE(cpustate, EA);
		EA=X+SIGNED(EA);
		cpustate->icount-=2;
		break;
	case 0x25:              /* postword offs */
		IMMWORD(cpustate, cpustate->ea);
		EA+=X;
		cpustate->icount-=4;
		break;
	case 0x26:              /* normal */
		EA=X;
		break;
//  case 0x27: EA=0; break; /* extended */
	case 0x28:              /* indirect - auto increment */
		EA=X;
		X++;
		EA=RM16(cpustate, EAD);
		cpustate->icount-=5;
		break;
	case 0x29:              /* indirect - double auto increment */
		EA=X;
		X+=2;
		EA=RM16(cpustate, EAD);
		cpustate->icount-=6;
		break;
	case 0x2a:              /* indirect - auto decrement */
		X--;
		EA=X;
		EA=RM16(cpustate, EAD);
		cpustate->icount-=5;
		break;
	case 0x2b:              /* indirect - double auto decrement */
		X-=2;
		EA=X;
		EA=RM16(cpustate, EAD);
		cpustate->icount-=6;
		break;
	case 0x2c:              /* indirect - postbyte offs */
		IMMBYTE(cpustate, EA);
		EA=X+SIGNED(EA);
		EA=RM16(cpustate, EAD);
		cpustate->icount-=4;
		break;
	case 0x2d:              /* indirect - postword offs */
		IMMWORD(cpustate, cpustate->ea);
		EA+=X;
		EA=RM16(cpustate, EAD);
		cpustate->icount-=7;
		break;
	case 0x2e:              /* indirect - normal */
		EA=X;
		EA=RM16(cpustate, EAD);
		cpustate->icount-=3;
		break;
//  case 0x2f: EA=0; break; /* indirect - extended */

/* base Y */
	case 0x30:              /* auto increment */
		EA=Y;
		Y++;
		cpustate->icount-=2;
		break;
	case 0x31:              /* double auto increment */
		EA=Y;
		Y+=2;
		cpustate->icount-=3;
		break;
	case 0x32:              /* auto decrement */
		Y--;
		EA=Y;
		cpustate->icount-=2;
		break;
	case 0x33:              /* double auto decrement */
		Y-=2;
		EA=Y;
		cpustate->icount-=3;
		break;
	case 0x34:              /* postbyte offs */
		IMMBYTE(cpustate, EA);
		EA=Y+SIGNED(EA);
		cpustate->icount-=2;
		break;
	case 0x35:              /* postword offs */
		IMMWORD(cpustate, cpustate->ea);
		EA+=Y;
		cpustate->icount-=4;
		break;
	case 0x36:              /* normal */
		EA=Y;
		break;
//  case 0x37: EA=0; break; /* extended */
	case 0x38:              /* indirect - auto increment */
		EA=Y;
		Y++;
		EA=RM16(cpustate, EAD);
		cpustate->icount-=5;
		break;
	case 0x39:              /* indirect - double auto increment */
		EA=Y;
		Y+=2;
		EA=RM16(cpustate, EAD);
		cpustate->icount-=6;
		break;
	case 0x3a:              /* indirect - auto decrement */
		Y--;
		EA=Y;
		EA=RM16(cpustate, EAD);
		cpustate->icount-=5;
		break;
	case 0x3b:              /* indirect - double auto decrement */
		Y-=2;
		EA=Y;
		EA=RM16(cpustate, EAD);
		cpustate->icount-=6;
		break;
	case 0x3c:              /* indirect - postbyte offs */
		IMMBYTE(cpustate, EA);
		EA=Y+SIGNED(EA);
		EA=RM16(cpustate, EAD);
		cpustate->icount-=4;
		break;
	case 0x3d:              /* indirect - postword offs */
		IMMWORD(cpustate, cpustate->ea);
		EA+=Y;
		EA=RM16(cpustate, EAD);
		cpustate->icount-=7;
		break;
	case 0x3e:              /* indirect - normal */
		EA=Y;
		EA=RM16(cpustate, EAD);
		cpustate->icount-=3;
		break;
//  case 0x3f: EA=0; break; /* indirect - extended */

//  case 0x40: EA=0; break; /* auto increment */
//  case 0x41: EA=0; break; /* double auto increment */
//  case 0x42: EA=0; break; /* auto decrement */
//  case 0x43: EA=0; break; /* double auto decrement */
//  case 0x44: EA=0; break; /* postbyte offs */
//  case 0x45: EA=0; break; /* postword offs */
//  case 0x46: EA=0; break; /* normal */
//  case 0x47: EA=0; break; /* extended */
//  case 0x48: EA=0; break; /* indirect - auto increment */
//  case 0x49: EA=0; break; /* indirect - double auto increment */
//  case 0x4a: EA=0; break; /* indirect - auto decrement */
//  case 0x4b: EA=0; break; /* indirect - double auto decrement */
//  case 0x4c: EA=0; break; /* indirect - postbyte offs */
//  case 0x4d: EA=0; break; /* indirect - postword offs */
//  case 0x4e: EA=0; break; /* indirect - normal */
//  case 0x4f: EA=0; break; /* indirect - extended */

/* base U */
	case 0x50:              /* auto increment */
		EA=U;
		U++;
		cpustate->icount-=2;
		break;
	case 0x51:              /* double auto increment */
		EA=U;
		U+=2;
		cpustate->icount-=3;
		break;
	case 0x52:              /* auto decrement */
		U--;
		EA=U;
		cpustate->icount-=2;
		break;
	case 0x53:              /* double auto decrement */
		U-=2;
		EA=U;
		cpustate->icount-=3;
		break;
	case 0x54:              /* postbyte offs */
		IMMBYTE(cpustate, EA);
		EA=U+SIGNED(EA);
		cpustate->icount-=2;
		break;
	case 0x55:              /* postword offs */
		IMMWORD(cpustate, cpustate->ea);
		EA+=U;
		cpustate->icount-=4;
		break;
	case 0x56:              /* normal */
		EA=U;
		break;
//  case 0x57: EA=0; break; /* extended */
	case 0x58:              /* indirect - auto increment */
		EA=U;
		U++;
		EA=RM16(cpustate, EAD);
		cpustate->icount-=5;
		break;
	case 0x59:              /* indirect - double auto increment */
		EA=U;
		U+=2;
		EA=RM16(cpustate, EAD);
		cpustate->icount-=6;
		break;
	case 0x5a:              /* indirect - auto decrement */
		U--;
		EA=U;
		EA=RM16(cpustate, EAD);
		cpustate->icount-=5;
		break;
	case 0x5b:              /* indirect - double auto decrement */
		U-=2;
		EA=U;
		EA=RM16(cpustate, EAD);
		cpustate->icount-=6;
		break;
	case 0x5c:              /* indirect - postbyte offs */
		IMMBYTE(cpustate, EA);
		EA=U+SIGNED(EA);
		EA=RM16(cpustate, EAD);
		cpustate->icount-=4;
		break;
	case 0x5d:              /* indirect - postword offs */
		IMMWORD(cpustate, cpustate->ea);
		EA+=U;
		EA=RM16(cpustate, EAD);
		cpustate->icount-=7;
		break;
	case 0x5e:              /* indirect - normal */
		EA=U;
		EA=RM16(cpustate, EAD);
		cpustate->icount-=3;
		break;
//  case 0x5f: EA=0; break; /* indirect - extended */

/* base S */
	case 0x60:              /* auto increment */
		EAD=SD;
		S++;
		cpustate->icount-=2;
		break;
	case 0x61:              /* double auto increment */
		EAD=SD;
		S+=2;
		cpustate->icount-=3;
		break;
	case 0x62:              /* auto decrement */
		S--;
		EAD=SD;
		cpustate->icount-=2;
		break;
	case 0x63:              /* double auto decrement */
		S-=2;
		EAD=SD;
		cpustate->icount-=3;
		break;
	case 0x64:              /* postbyte offs */
		IMMBYTE(cpustate, EA);
		EA=S+SIGNED(EA);
		cpustate->icount-=2;
		break;
	case 0x65:              /* postword offs */
		IMMWORD(cpustate, cpustate->ea);
		EA+=S;
		cpustate->icount-=4;
		break;
	case 0x66:              /* normal */
		EAD=SD;
		break;
//  case 0x67: EA=0; break; /* extended */
	case 0x68:              /* indirect - auto increment */
		EAD=SD;
		S++;
		EA=RM16(cpustate, EAD);
		cpustate->icount-=5;
		break;
	case 0x69:              /* indirect - double auto increment */
		EAD=SD;
		S+=2;
		EA=RM16(cpustate, EAD);
		cpustate->icount-=6;
		break;
	case 0x6a:              /* indirect - auto decrement */
		S--;
		EAD=SD;
		EA=RM16(cpustate, EAD);
		cpustate->icount-=5;
		break;
	case 0x6b:              /* indirect - double auto decrement */
		S-=2;
		EAD=SD;
		EA=RM16(cpustate, EAD);
		cpustate->icount-=6;
		break;
	case 0x6c:              /* indirect - postbyte offs */
		IMMBYTE(cpustate, EA);
		EA=S+SIGNED(EA);
		EA=RM16(cpustate, EAD);
		cpustate->icount-=4;
		break;
	case 0x6d:              /* indirect - postword offs */
		IMMWORD(cpustate, cpustate->ea);
		EA+=S;
		EA=RM16(cpustate, EAD);
		cpustate->icount-=7;
		break;
	case 0x6e:              /* indirect - normal */
		EAD=SD;
		EA=RM16(cpustate, EAD);
		cpustate->icount-=3;
		break;
//  case 0x6f: EA=0; break; /* indirect - extended */

/* base PC */
	case 0x70:              /* auto increment */
		EAD=PCD;
		PC++;
		cpustate->icount-=2;
		break;
	case 0x71:              /* double auto increment */
		EAD=PCD;
		PC+=2;
		cpustate->icount-=3;
		break;
	case 0x72:              /* auto decrement */
		PC--;
		EAD=PCD;
		cpustate->icount-=2;
		break;
	case 0x73:              /* double auto decrement */
		PC-=2;
		EAD=PCD;
		cpustate->icount-=3;
		break;
	case 0x74:              /* postbyte offs */
		IMMBYTE(cpustate, EA);
		EA=PC-1+SIGNED(EA);
		cpustate->icount-=2;
		break;
	case 0x75:              /* postword offs */
		IMMWORD(cpustate, cpustate->ea);
		EA+=PC-2;
		cpustate->icount-=4;
		break;
	case 0x76:              /* normal */
		EAD=PCD;
		break;
//  case 0x77: EA=0; break; /* extended */
	case 0x78:              /* indirect - auto increment */
		EAD=PCD;
		PC++;
		EA=RM16(cpustate, EAD);
		cpustate->icount-=5;
		break;
	case 0x79:              /* indirect - double auto increment */
		EAD=PCD;
		PC+=2;
		EA=RM16(cpustate, EAD);
		cpustate->icount-=6;
		break;
	case 0x7a:              /* indirect - auto decrement */
		PC--;
		EAD=PCD;
		EA=RM16(cpustate, EAD);
		cpustate->icount-=5;
		break;
	case 0x7b:              /* indirect - double auto decrement */
		PC-=2;
		EAD=PCD;
		EA=RM16(cpustate, EAD);
		cpustate->icount-=6;
		break;
	case 0x7c:              /* indirect - postbyte offs */
		IMMBYTE(cpustate, EA);
		EA=PC-1+SIGNED(EA);
		EA=RM16(cpustate, EAD);
		cpustate->icount-=4;
		break;
	case 0x7d:              /* indirect - postword offs */
		IMMWORD(cpustate, cpustate->ea);
		EA+=PC-2;
		EA=RM16(cpustate, EAD);
		cpustate->icount-=7;
		break;
	case 0x7e:              /* indirect - normal */
		EAD=PCD;
		EA=RM16(cpustate, EAD);
		cpustate->icount-=3;
		break;
//  case 0x7f: EA=0; break; /* indirect - extended */

//  case 0x80: EA=0; break; /* register a */
//  case 0x81: EA=0; break; /* register b */
//  case 0x82: EA=0; break; /* ???? */
//  case 0x83: EA=0; break; /* ???? */
//  case 0x84: EA=0; break; /* ???? */
//  case 0x85: EA=0; break; /* ???? */
//  case 0x86: EA=0; break; /* ???? */
//  case 0x87: EA=0; break; /* register d */
//  case 0x88: EA=0; break; /* indirect - register a */
//  case 0x89: EA=0; break; /* indirect - register b */
//  case 0x8a: EA=0; break; /* indirect - ???? */
//  case 0x8b: EA=0; break; /* indirect - ???? */
//  case 0x8c: EA=0; break; /* indirect - ???? */
//  case 0x8d: EA=0; break; /* indirect - ???? */
//  case 0x8e: EA=0; break; /* indirect - register d */
//  case 0x8f: EA=0; break; /* indirect - ???? */
//  case 0x90: EA=0; break; /* register a */
//  case 0x91: EA=0; break; /* register b */
//  case 0x92: EA=0; break; /* ???? */
//  case 0x93: EA=0; break; /* ???? */
//  case 0x94: EA=0; break; /* ???? */
//  case 0x95: EA=0; break; /* ???? */
//  case 0x96: EA=0; break; /* ???? */
//  case 0x97: EA=0; break; /* register d */
//  case 0x98: EA=0; break; /* indirect - register a */
//  case 0x99: EA=0; break; /* indirect - register b */
//  case 0x9a: EA=0; break; /* indirect - ???? */
//  case 0x9b: EA=0; break; /* indirect - ???? */
//  case 0x9c: EA=0; break; /* indirect - ???? */
//  case 0x9d: EA=0; break; /* indirect - ???? */
//  case 0x9e: EA=0; break; /* indirect - register d */
//  case 0x9f: EA=0; break; /* indirect - ???? */
	case 0xa0:              /* register a */
		EA=X+SIGNED(A);
		cpustate->icount-=1;
		break;
	case 0xa1:              /* register b */
		EA=X+SIGNED(B);
		cpustate->icount-=1;
		break;
//  case 0xa2: EA=0; break; /* ???? */
//  case 0xa3: EA=0; break; /* ???? */
//  case 0xa4: EA=0; break; /* ???? */
//  case 0xa5: EA=0; break; /* ???? */
//  case 0xa6: EA=0; break; /* ???? */
	case 0xa7:              /* register d */
		EA=X+D;
		cpustate->icount-=4;
		break;
	case 0xa8:              /* indirect - register a */
		EA=X+SIGNED(A);
		EA=RM16(cpustate, EAD);
		cpustate->icount-=4;
		break;
	case 0xa9:              /* indirect - register b */
		EA=X+SIGNED(B);
		EA=RM16(cpustate, EAD);
		cpustate->icount-=4;
		break;
//  case 0xaa: EA=0; break; /* indirect - ???? */
//  case 0xab: EA=0; break; /* indirect - ???? */
//  case 0xac: EA=0; break; /* indirect - ???? */
//  case 0xad: EA=0; break; /* indirect - ???? */
//  case 0xae: EA=0; break; /* indirect - ???? */
	case 0xaf:              /* indirect - register d */
		EA=X+D;
		EA=RM16(cpustate, EAD);
		cpustate->icount-=7;
		break;
	case 0xb0:              /* register a */
		EA=Y+SIGNED(A);
		cpustate->icount-=1;
		break;
	case 0xb1:              /* register b */
		EA=Y+SIGNED(B);
		cpustate->icount-=1;
		break;
//  case 0xb2: EA=0; break; /* ???? */
//  case 0xb3: EA=0; break; /* ???? */
//  case 0xb4: EA=0; break; /* ???? */
//  case 0xb5: EA=0; break; /* ???? */
//  case 0xb6: EA=0; break; /* ???? */
	case 0xb7:              /* register d */
		EA=Y+D;
		cpustate->icount-=4;
		break;
	case 0xb8:              /* indirect - register a */
		EA=Y+SIGNED(A);
		EA=RM16(cpustate, EAD);
		cpustate->icount-=4;
		break;
	case 0xb9:              /* indirect - register b */
		EA=Y+SIGNED(B);
		EA=RM16(cpustate, EAD);
		cpustate->icount-=4;
		break;
//  case 0xba: EA=0; break; /* indirect - ???? */
//  case 0xbb: EA=0; break; /* indirect - ???? */
//  case 0xbc: EA=0; break; /* indirect - ???? */
//  case 0xbd: EA=0; break; /* indirect - ???? */
//  case 0xbe: EA=0; break; /* indirect - ???? */
	case 0xbf:              /* indirect - register d */
		EA=Y+D;
		EA=RM16(cpustate, EAD);
		cpustate->icount-=7;
		break;
//  case 0xc0: EA=0; break; /* register a */
//  case 0xc1: EA=0; break; /* register b */
//  case 0xc2: EA=0; break; /* ???? */
//  case 0xc3: EA=0; break; /* ???? */
	case 0xc4:
		EAD=0;
		(*konami_direct[cpustate->ireg])(cpustate);
		cpustate->icount -= 1;
		return;
//  case 0xc5: EA=0; break; /* ???? */
//  case 0xc6: EA=0; break; /* ???? */
//  case 0xc7: EA=0; break; /* register d */
//  case 0xc8: EA=0; break; /* indirect - register a */
//  case 0xc9: EA=0; break; /* indirect - register b */
//  case 0xca: EA=0; break; /* indirect - ???? */
//  case 0xcb: EA=0; break; /* indirect - ???? */
	case 0xcc:              /* indirect - direct */
		DIRWORD(cpustate, cpustate->ea);
		cpustate->icount-=4;
		break;
//  case 0xcd: EA=0; break; /* indirect - ???? */
//  case 0xce: EA=0; break; /* indirect - register d */
//  case 0xcf: EA=0; break; /* indirect - ???? */
	case 0xd0:              /* register a */
		EA=U+SIGNED(A);
		cpustate->icount-=1;
		break;
	case 0xd1:              /* register b */
		EA=U+SIGNED(B);
		cpustate->icount-=1;
		break;
//  case 0xd2: EA=0; break; /* ???? */
//  case 0xd3: EA=0; break; /* ???? */
//  case 0xd4: EA=0; break; /* ???? */
//  case 0xd5: EA=0; break; /* ???? */
//  case 0xd6: EA=0; break; /* ???? */
	case 0xd7:              /* register d */
		EA=U+D;
		cpustate->icount-=4;
		break;
	case 0xd8:              /* indirect - register a */
		EA=U+SIGNED(A);
		EA=RM16(cpustate, EAD);
		cpustate->icount-=4;
		break;
	case 0xd9:              /* indirect - register b */
		EA=U+SIGNED(B);
		EA=RM16(cpustate, EAD);
		cpustate->icount-=4;
		break;
//  case 0xda: EA=0; break; /* indirect - ???? */
//  case 0xdb: EA=0; break; /* indirect - ???? */
//  case 0xdc: EA=0; break; /* indirect - ???? */
//  case 0xdd: EA=0; break; /* indirect - ???? */
//  case 0xde: EA=0; break; /* indirect - ???? */
	case 0xdf:              /* indirect - register d */
		EA=U+D;
		EA=RM16(cpustate, EAD);
		cpustate->icount-=7;
		break;
	case 0xe0:              /* register a */
		EA=S+SIGNED(A);
		cpustate->icount-=1;
		break;
	case 0xe1:              /* register b */
		EA=S+SIGNED(B);
		cpustate->icount-=1;
		break;
//  case 0xe2: EA=0; break; /* ???? */
//  case 0xe3: EA=0; break; /* ???? */
//  case 0xe4: EA=0; break; /* ???? */
//  case 0xe5: EA=0; break; /* ???? */
//  case 0xe6: EA=0; break; /* ???? */
	case 0xe7:              /* register d */
		EA=S+D;
		cpustate->icount-=4;
		break;
	case 0xe8:              /* indirect - register a */
		EA=S+SIGNED(A);
		EA=RM16(cpustate, EAD);
		cpustate->icount-=4;
		break;
	case 0xe9:              /* indirect - register b */
		EA=S+SIGNED(B);
		EA=RM16(cpustate, EAD);
		cpustate->icount-=4;
		break;
//  case 0xea: EA=0; break; /* indirect - ???? */
//  case 0xeb: EA=0; break; /* indirect - ???? */
//  case 0xec: EA=0; break; /* indirect - ???? */
//  case 0xed: EA=0; break; /* indirect - ???? */
//  case 0xee: EA=0; break; /* indirect - ???? */
	case 0xef:              /* indirect - register d */
		EA=S+D;
		EA=RM16(cpustate, EAD);
		cpustate->icount-=7;
		break;
	case 0xf0:              /* register a */
		EA=PC+SIGNED(A);
		cpustate->icount-=1;
		break;
	case 0xf1:              /* register b */
		EA=PC+SIGNED(B);
		cpustate->icount-=1;
		break;
//  case 0xf2: EA=0; break; /* ???? */
//  case 0xf3: EA=0; break; /* ???? */
//  case 0xf4: EA=0; break; /* ???? */
//  case 0xf5: EA=0; break; /* ???? */
//  case 0xf6: EA=0; break; /* ???? */
	case 0xf7:              /* register d */
		EA=PC+D;
		cpustate->icount-=4;
		break;
	case 0xf8:              /* indirect - register a */
		EA=PC+SIGNED(A);
		EA=RM16(cpustate, EAD);
		cpustate->icount-=4;
		break;
	case 0xf9:              /* indirect - register b */
		EA=PC+SIGNED(B);
		EA=RM16(cpustate, EAD);
		cpustate->icount-=4;
		break;
//  case 0xfa: EA=0; break; /* indirect - ???? */
//  case 0xfb: EA=0; break; /* indirect - ???? */
//  case 0xfc: EA=0; break; /* indirect - ???? */
//  case 0xfd: EA=0; break; /* indirect - ???? */
//  case 0xfe: EA=0; break; /* indirect - ???? */
	case 0xff:              /* indirect - register d */
		EA=PC+D;
		EA=RM16(cpustate, EAD);
		cpustate->icount-=7;
		break;
	default:
		logerror("KONAMI: Unknown/Invalid postbyte at PC = %04x\n", PC -1 );
		EAD = 0;
	}
	(*konami_indexed[cpustate->ireg])(cpustate);
}
