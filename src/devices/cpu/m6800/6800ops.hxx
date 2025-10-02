// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*

HNZVC

? = undefined
* = affected
- = unaffected
0 = cleared
1 = set
# = ccr directly affected by instruction
@ = special - carry set if bit 7 is set

*/

#define OP_HANDLER(_name) void m6800_cpu_device::_name ()

//OP_HANDLER( illegl1 )
OP_HANDLER( illegl1 )
{
	logerror("m6800: illegal 1-byte opcode: address %04X, op %02X\n",PC-1,(int) M_RDOP_ARG(PC-1)&0xFF);
}

//OP_HANDLER( illegl2 )
OP_HANDLER( illegl2 )
{
	logerror("m6800: illegal 2-byte opcode: address %04X, op %02X\n",PC-1,(int) M_RDOP_ARG(PC-1)&0xFF);
	PC++;
}

//OP_HANDLER( illegl3 )
OP_HANDLER( illegl3 )
{
	logerror("m6800: illegal 3-byte opcode: address %04X, op %02X\n",PC-1,(int) M_RDOP_ARG(PC-1)&0xFF);
	PC += 2;
}

/* HD63701 only */
//OP_HANDLER( trap )
OP_HANDLER( trap )
{
	logerror("m6800: illegal opcode: address %04X, op %02X\n",PC-1,(int) M_RDOP_ARG(PC-1)&0xFF);
	PC--;
	take_trap();
}

/* $00 ILLEGAL */

/* $01 NOP */
OP_HANDLER( nop )
{
}

/* $02 ILLEGAL */

/* $03 ILLEGAL */

/* $04 LSRD inherent -0*-* */
OP_HANDLER( lsrd )
{
	u16 t;
	CLR_NZVC; t = D; CC|=(t&0x0001);
	t>>=1; SET_Z16(t);
	if (NXORC) SEV;
	D=t;
}

/* $05 ASLD inherent ?**** */
OP_HANDLER( asld )
{
	int r;
	u16 t;
	t = D; r=t<<1;
	CLR_NZVC; SET_FLAGS16(t,t,r);
	D=r;
}

/* $06 TAP inherent ##### */
OP_HANDLER( tap )
{
	CC=A;

	// TAP temporarily sets the I flag and blocks IRQ until the next opcode
	// (if the next opcode is TAP, IRQ is blocked again)
	execute_one();
	check_irq_lines();
}

/* $07 TPA inherent ----- */
OP_HANDLER( tpa )
{
	A=CC;
}

/* $08 INX inherent --*-- */
OP_HANDLER( inx )
{
	++X;
	CLR_Z; SET_Z16(X);
}

/* $09 DEX inherent --*-- */
OP_HANDLER( dex )
{
	--X;
	CLR_Z; SET_Z16(X);
}

/* $0a CLV */
OP_HANDLER( clv )
{
	CLV;
}

/* $0b SEV */
OP_HANDLER( sev )
{
	SEV;
}

/* $0c CLC */
OP_HANDLER( clc )
{
	CLC;
}

/* $0d SEC */
OP_HANDLER( sec )
{
	SEC;
}

/* $0e CLI */
OP_HANDLER( cli )
{
	u8 i = CC & 0x10;
	CLI;

	// pending IRQ won't be triggered until next machine cycle
	if (i) execute_one();
	check_irq_lines();
}

/* $0f SEI */
OP_HANDLER( sei )
{
	SEI;
}

/* $10 SBA inherent -**** */
OP_HANDLER( sba )
{
	u16 t;
	t=A-B;
	CLR_NZVC; SET_FLAGS8(A,B,t);
	A=t;
}

/* $11 CBA inherent -**** */
OP_HANDLER( cba )
{
	u16 t;
	t=A-B;
	CLR_NZVC; SET_FLAGS8(A,B,t);
}

/* $12 ILLEGAL */
OP_HANDLER( undoc1 )
{
	X += RM( S + 1 );
}

/* $13 ILLEGAL */
OP_HANDLER( undoc2 )
{
	X += RM( S + 1 );
}


/* $14 ILLEGAL */

/* $15 ILLEGAL */

/* $16 TAB inherent -**0- */
OP_HANDLER( tab )
{
	B=A;
	CLR_NZV; SET_NZ8(B);
}

/* $17 TBA inherent -**0- */
OP_HANDLER( tba )
{
	A=B;
	CLR_NZV; SET_NZ8(A);
}

/* $18 XGDX inherent ----- */ /* HD63701Y0 only */
OP_HANDLER( xgdx )
{
	u16 t = X;
	X = D;
	D=t;
}

/* $19 DAA inherent (A) -**0* */
OP_HANDLER( daa )
{
	u8 msn, lsn;
	u16 t, cf = 0;
	msn=A & 0xf0; lsn=A & 0x0f;
	if (lsn>0x09 || CC&0x20) cf |= 0x06;
	if (msn>0x80 && lsn>0x09) cf |= 0x60;
	if (msn>0x90 || CC&0x01) cf |= 0x60;
	t = cf + A;
	CLR_NZV; // keep carry from previous operation
	SET_NZ8((u8)t); SET_C8(t);
	A = t;
}

/* $1a ILLEGAL */

/* $1a SLP */ /* HD63701Y0 only */
OP_HANDLER( slp )
{
	// wait for next IRQ (same as waiting of WAI)
	m_wai_state |= M6800_SLP;

	check_irq_lines();
	if (m_wai_state & M6800_SLP)
		eat_cycles();
}

/* $1b ABA inherent ***** */
OP_HANDLER( aba )
{
	u16 t;
	t=A+B;
	CLR_HNZVC; SET_FLAGS8(A,B,t); SET_H(A,B,t);
	A=t;
}

/* $1c ILLEGAL */

/* $1d ILLEGAL */

/* $1e ILLEGAL */

/* $1f ILLEGAL */

/* $20 BRA relative ----- */
OP_HANDLER( bra )
{
	u8 t;
	BRANCH(1);
}

/* $21 BRN relative ----- */
OP_HANDLER( brn )
{
	u8 t;
	BRANCH(0);
}

/* $22 BHI relative ----- */
OP_HANDLER( bhi )
{
	u8 t;
	BRANCH(!(CC&0x05));
}

/* $23 BLS relative ----- */
OP_HANDLER( bls )
{
	u8 t;
	BRANCH(CC&0x05);
}

/* $24 BCC relative ----- */
OP_HANDLER( bcc )
{
	u8 t;
	BRANCH(!(CC&0x01));
}

/* $25 BCS relative ----- */
OP_HANDLER( bcs )
{
	u8 t;
	BRANCH(CC&0x01);
}

/* $26 BNE relative ----- */
OP_HANDLER( bne )
{
	u8 t;
	BRANCH(!(CC&0x04));
}

/* $27 BEQ relative ----- */
OP_HANDLER( beq )
{
	u8 t;
	BRANCH(CC&0x04);
}

/* $28 BVC relative ----- */
OP_HANDLER( bvc )
{
	u8 t;
	BRANCH(!(CC&0x02));
}

/* $29 BVS relative ----- */
OP_HANDLER( bvs )
{
	u8 t;
	BRANCH(CC&0x02);
}

/* $2a BPL relative ----- */
OP_HANDLER( bpl )
{
	u8 t;
	BRANCH(!(CC&0x08));
}

/* $2b BMI relative ----- */
OP_HANDLER( bmi )
{
	u8 t;
	BRANCH(CC&0x08);
}

/* $2c BGE relative ----- */
OP_HANDLER( bge )
{
	u8 t;
	BRANCH(!NXORV);
}

/* $2d BLT relative ----- */
OP_HANDLER( blt )
{
	u8 t;
	BRANCH(NXORV);
}

/* $2e BGT relative ----- */
OP_HANDLER( bgt )
{
	u8 t;
	BRANCH(!(NXORV||CC&0x04));
}

/* $2f BLE relative ----- */
OP_HANDLER( ble )
{
	u8 t;
	BRANCH(NXORV||CC&0x04);
}

/* $30 TSX inherent ----- */
OP_HANDLER( tsx )
{
	X = ( S + 1 );
}

/* $31 INS inherent ----- */
OP_HANDLER( ins )
{
	++S;
}

/* $32 PULA inherent ----- */
OP_HANDLER( pula )
{
	PULLBYTE(m_d.b.h);
}

/* $33 PULB inherent ----- */
OP_HANDLER( pulb )
{
	PULLBYTE(m_d.b.l);
}

/* $34 DES inherent ----- */
OP_HANDLER( des )
{
	--S;
}

/* $35 TXS inherent ----- */
OP_HANDLER( txs )
{
	S = ( X - 1 );
}

/* $36 PSHA inherent ----- */
OP_HANDLER( psha )
{
	PUSHBYTE(m_d.b.h);
}

/* $37 PSHB inherent ----- */
OP_HANDLER( pshb )
{
	PUSHBYTE(m_d.b.l);
}

/* $38 PULX inherent ----- */
OP_HANDLER( pulx )
{
	PULLWORD(pX);
}

/* $39 RTS inherent ----- */
OP_HANDLER( rts )
{
	PULLWORD(pPC);
}

/* $3a ABX inherent ----- */
OP_HANDLER( abx )
{
	X += B;
}

/* $3b RTI inherent ##### */
OP_HANDLER( rti )
{
	PULLBYTE(CC);
	PULLBYTE(B);
	PULLBYTE(A);
	PULLWORD(pX);
	PULLWORD(pPC);

	check_irq_lines();
}

/* $3c PSHX inherent ----- */
OP_HANDLER( pshx )
{
	PUSHWORD(pX);
}

/* $3d MUL inherent --*-@ */
OP_HANDLER( mul )
{
	u16 t;
	t=A*B;
	CLR_C;
	if (t & 0x80) SEC;
	D=t;
}

/* $3e WAI inherent ----- */
OP_HANDLER( wai )
{
	// WAI stacks the entire machine state on the hardware stack,
	// then waits for an interrupt.
	m_wai_state |= M6800_WAI;

	PUSHWORD(pPC);
	PUSHWORD(pX);
	PUSHBYTE(A);
	PUSHBYTE(B);
	PUSHBYTE(CC);

	check_irq_lines();
	if (m_wai_state & M6800_WAI)
		eat_cycles();
}

/* $3f SWI absolute indirect ----- */
OP_HANDLER( swi )
{
	PUSHWORD(pPC);
	PUSHWORD(pX);
	PUSHBYTE(A);
	PUSHBYTE(B);
	PUSHBYTE(CC);

	SEI;
	PCD = RM16(0xfffa);
}

/* $40 NEGA inherent ?**** */
OP_HANDLER( nega )
{
	u16 r;
	r=-A;
	CLR_NZVC; SET_FLAGS8(0,A,r);
	A=r;
}

/* $41 ILLEGAL */

/* $42 ILLEGAL */

/* $43 COMA inherent -**01 */
OP_HANDLER( coma )
{
	A = ~A;
	CLR_NZV; SET_NZ8(A); SEC;
}

/* $44 LSRA inherent -0*-* */
OP_HANDLER( lsra )
{
	CLR_NZVC; CC|=(A&0x01);
	A>>=1; SET_Z8(A);
	if (NXORC) SEV;
}

/* $45 ILLEGAL */

/* $46 RORA inherent -**-* */
OP_HANDLER( rora )
{
	u8 r;
	r=(CC&0x01)<<7;
	CLR_NZVC; CC|=(A&0x01);
	r |= A>>1; SET_NZ8(r);
	if (NXORC) SEV;
	A=r;
}

/* $47 ASRA inherent ?**-* */
OP_HANDLER( asra )
{
	CLR_NZVC; CC|=(A&0x01);
	A>>=1; A|=((A&0x40)<<1);
	SET_NZ8(A);
	if (NXORC) SEV;
}

/* $48 ASLA inherent ?**** */
OP_HANDLER( asla )
{
	u16 r;
	r=A<<1;
	CLR_NZVC; SET_FLAGS8(A,A,r);
	A=r;
}

/* $49 ROLA inherent -**** */
OP_HANDLER( rola )
{
	u16 t,r;
	t = A; r = CC&0x01; r |= t<<1;
	CLR_NZVC; SET_FLAGS8(t,t,r);
	A=r;
}

/* $4a DECA inherent -***- */
OP_HANDLER( deca )
{
	--A;
	CLR_NZV; SET_FLAGS8D(A);
}

/* $4b ILLEGAL */

/* $4c INCA inherent -***- */
OP_HANDLER( inca )
{
	++A;
	CLR_NZV; SET_FLAGS8I(A);
}

/* $4d TSTA inherent -**0- */
OP_HANDLER( tsta )
{
	CLR_NZVC; SET_NZ8(A);
}

/* $4e ILLEGAL */

/* $4f CLRA inherent -0100 */
OP_HANDLER( clra )
{
	A=0;
	CLR_NZVC; SEZ;
}

/* $50 NEGB inherent ?**** */
OP_HANDLER( negb )
{
	u16 r;
	r=-B;
	CLR_NZVC; SET_FLAGS8(0,B,r);
	B=r;
}

/* $51 ILLEGAL */

/* $52 ILLEGAL */

/* $53 COMB inherent -**01 */
OP_HANDLER( comb )
{
	B = ~B;
	CLR_NZV; SET_NZ8(B); SEC;
}

/* $54 LSRB inherent -0*-* */
OP_HANDLER( lsrb )
{
	CLR_NZVC; CC|=(B&0x01);
	B>>=1; SET_Z8(B);
	if (NXORC) SEV;
}

/* $55 ILLEGAL */

/* $56 RORB inherent -**-* */
OP_HANDLER( rorb )
{
	u8 r;
	r=(CC&0x01)<<7;
	CLR_NZVC; CC|=(B&0x01);
	r |= B>>1; SET_NZ8(r);
	if (NXORC) SEV;
	B=r;
}

/* $57 ASRB inherent ?**-* */
OP_HANDLER( asrb )
{
	CLR_NZVC; CC|=(B&0x01);
	B>>=1; B|=((B&0x40)<<1);
	SET_NZ8(B);
	if (NXORC) SEV;
}

/* $58 ASLB inherent ?**** */
OP_HANDLER( aslb )
{
	u16 r;
	r=B<<1;
	CLR_NZVC; SET_FLAGS8(B,B,r);
	B=r;
}

/* $59 ROLB inherent -**** */
OP_HANDLER( rolb )
{
	u16 t,r;
	t = B; r = CC&0x01; r |= t<<1;
	CLR_NZVC; SET_FLAGS8(t,t,r);
	B=r;
}

/* $5a DECB inherent -***- */
OP_HANDLER( decb )
{
	--B;
	CLR_NZV; SET_FLAGS8D(B);
}

/* $5b ILLEGAL */

/* $5c INCB inherent -***- */
OP_HANDLER( incb )
{
	++B;
	CLR_NZV; SET_FLAGS8I(B);
}

/* $5d TSTB inherent -**0- */
OP_HANDLER( tstb )
{
	CLR_NZVC; SET_NZ8(B);
}

/* $5e ILLEGAL */

/* $5f CLRB inherent -0100 */
OP_HANDLER( clrb )
{
	B=0;
	CLR_NZVC; SEZ;
}

/* $60 NEG indexed ?**** */
OP_HANDLER( neg_ix )
{
	u16 r,t;
	IDXBYTE(t); r=-t;
	CLR_NZVC; SET_FLAGS8(0,t,r);
	WM(EAD,r);
}

/* $61 AIM --**0- */ /* HD63701Y0 only */
OP_HANDLER( aim_ix )
{
	u8 t, r;
	IMMBYTE(t);
	IDXBYTE(r);
	r &= t;
	CLR_NZV; SET_NZ8(r);
	WM(EAD,r);
}

/* $62 OIM --**0- */ /* HD63701Y0 only */
OP_HANDLER( oim_ix )
{
	u8 t, r;
	IMMBYTE(t);
	IDXBYTE(r);
	r |= t;
	CLR_NZV; SET_NZ8(r);
	WM(EAD,r);
}

/* $63 COM indexed -**01 */
OP_HANDLER( com_ix )
{
	u8 t;
	IDXBYTE(t); t = ~t;
	CLR_NZV; SET_NZ8(t); SEC;
	WM(EAD,t);
}

/* $64 LSR indexed -0*-* */
OP_HANDLER( lsr_ix )
{
	u8 t;
	IDXBYTE(t); CLR_NZVC; CC|=(t&0x01);
	t>>=1; SET_Z8(t);
	if (NXORC) SEV;
	WM(EAD,t);
}

/* $65 EIM --**0- */ /* HD63701Y0 only */
OP_HANDLER( eim_ix )
{
	u8 t, r;
	IMMBYTE(t);
	IDXBYTE(r);
	r ^= t;
	CLR_NZV; SET_NZ8(r);
	WM(EAD,r);
}

/* $66 ROR indexed -**-* */
OP_HANDLER( ror_ix )
{
	u8 t,r;
	IDXBYTE(t); r=(CC&0x01)<<7;
	CLR_NZVC; CC|=(t&0x01);
	r |= t>>1; SET_NZ8(r);
	if (NXORC) SEV;
	WM(EAD,r);
}

/* $67 ASR indexed ?**-* */
OP_HANDLER( asr_ix )
{
	u8 t;
	IDXBYTE(t); CLR_NZVC; CC|=(t&0x01);
	t>>=1; t|=((t&0x40)<<1);
	SET_NZ8(t);
	if (NXORC) SEV;
	WM(EAD,t);
}

/* $68 ASL indexed ?**** */
OP_HANDLER( asl_ix )
{
	u16 t,r;
	IDXBYTE(t); r=t<<1;
	CLR_NZVC; SET_FLAGS8(t,t,r);
	WM(EAD,r);
}

/* $69 ROL indexed -**** */
OP_HANDLER( rol_ix )
{
	u16 t,r;
	IDXBYTE(t); r = CC&0x01; r |= t<<1;
	CLR_NZVC; SET_FLAGS8(t,t,r);
	WM(EAD,r);
}

/* $6a DEC indexed -***- */
OP_HANDLER( dec_ix )
{
	u8 t;
	IDXBYTE(t); --t;
	CLR_NZV; SET_FLAGS8D(t);
	WM(EAD,t);
}

/* $6b TIM --**0- */ /* HD63701Y0 only */
OP_HANDLER( tim_ix )
{
	u8 t, r;
	IMMBYTE(t);
	IDXBYTE(r);
	r &= t;
	CLR_NZV; SET_NZ8(r);
}

/* $6c INC indexed -***- */
OP_HANDLER( inc_ix )
{
	u8 t;
	IDXBYTE(t); ++t;
	CLR_NZV; SET_FLAGS8I(t);
	WM(EAD,t);
}

/* $6d TST indexed -**0- */
OP_HANDLER( tst_ix )
{
	u8 t;
	IDXBYTE(t); CLR_NZVC; SET_NZ8(t);
}

/* $6e JMP indexed ----- */
OP_HANDLER( jmp_ix )
{
	INDEXED; PC=EA;
}

/* $6f CLR indexed -0100 */
OP_HANDLER( clr_ix )
{
	INDEXED; RM(EAD); WM(EAD,0);
	CLR_NZVC; SEZ;
}

/* $70 NEG extended ?**** */
OP_HANDLER( neg_ex )
{
	u16 r,t;
	EXTBYTE(t); r=-t;
	CLR_NZVC; SET_FLAGS8(0,t,r);
	WM(EAD,r);
}

/* $71 AIM --**0- */ /* HD63701Y0 only */
OP_HANDLER( aim_di )
{
	u8 t, r;
	IMMBYTE(t);
	DIRBYTE(r);
	r &= t;
	CLR_NZV; SET_NZ8(r);
	WM(EAD,r);
}

/* $72 OIM --**0- */ /* HD63701Y0 only */
OP_HANDLER( oim_di )
{
	u8 t, r;
	IMMBYTE(t);
	DIRBYTE(r);
	r |= t;
	CLR_NZV; SET_NZ8(r);
	WM(EAD,r);
}

/* $73 COM extended -**01 */
OP_HANDLER( com_ex )
{
	u8 t;
	EXTBYTE(t); t = ~t;
	CLR_NZV; SET_NZ8(t); SEC;
	WM(EAD,t);
}

/* $74 LSR extended -0*-* */
OP_HANDLER( lsr_ex )
{
	u8 t;
	EXTBYTE(t);
	CLR_NZVC;
	CC|=(t&0x01);
	t>>=1;
	SET_Z8(t);
	if (NXORC) SEV;
	WM(EAD,t);
}

/* $75 EIM --**0- */ /* HD63701Y0 only */
OP_HANDLER( eim_di )
{
	u8 t, r;
	IMMBYTE(t);
	DIRBYTE(r);
	r ^= t;
	CLR_NZV; SET_NZ8(r);
	WM(EAD,r);
}

/* $76 ROR extended -**-* */
OP_HANDLER( ror_ex )
{
	u8 t,r;
	EXTBYTE(t); r=(CC&0x01)<<7;
	CLR_NZVC; CC|=(t&0x01);
	r |= t>>1; SET_NZ8(r);
	if (NXORC) SEV;
	WM(EAD,r);
}

/* $77 ASR extended ?**-* */
OP_HANDLER( asr_ex )
{
	u8 t;
	EXTBYTE(t); CLR_NZVC; CC|=(t&0x01);
	t>>=1; t|=((t&0x40)<<1);
	SET_NZ8(t);
	if (NXORC) SEV;
	WM(EAD,t);
}

/* $78 ASL extended ?**** */
OP_HANDLER( asl_ex )
{
	u16 t,r;
	EXTBYTE(t); r=t<<1;
	CLR_NZVC; SET_FLAGS8(t,t,r);
	WM(EAD,r);
}

/* $79 ROL extended -**** */
OP_HANDLER( rol_ex )
{
	u16 t,r;
	EXTBYTE(t); r = CC&0x01; r |= t<<1;
	CLR_NZVC; SET_FLAGS8(t,t,r);
	WM(EAD,r);
}

/* $7a DEC extended -***- */
OP_HANDLER( dec_ex )
{
	u8 t;
	EXTBYTE(t); --t;
	CLR_NZV; SET_FLAGS8D(t);
	WM(EAD,t);
}

/* $7b TIM --**0- */ /* HD63701Y0 only */
OP_HANDLER( tim_di )
{
	u8 t, r;
	IMMBYTE(t);
	DIRBYTE(r);
	r &= t;
	CLR_NZV; SET_NZ8(r);
}

/* $7c INC extended -***- */
OP_HANDLER( inc_ex )
{
	u8 t;
	EXTBYTE(t); ++t;
	CLR_NZV; SET_FLAGS8I(t);
	WM(EAD,t);
}

/* $7d TST extended -**0- */
OP_HANDLER( tst_ex )
{
	u8 t;
	EXTBYTE(t); CLR_NZVC; SET_NZ8(t);
}

/* $7e JMP extended ----- */
OP_HANDLER( jmp_ex )
{
	EXTENDED; PC=EA;
}

/* $7f CLR extended -0100 */
OP_HANDLER( clr_ex )
{
	EXTENDED; RM(EAD); WM(EAD,0);
	CLR_NZVC; SEZ;
}

/* $80 SUBA immediate ?**** */
OP_HANDLER( suba_im )
{
	u16    t,r;
	IMMBYTE(t); r = A-t;
	CLR_NZVC; SET_FLAGS8(A,t,r);
	A = r;
}

/* $81 CMPA immediate ?**** */
OP_HANDLER( cmpa_im )
{
	u16    t,r;
	IMMBYTE(t); r = A-t;
	CLR_NZVC; SET_FLAGS8(A,t,r);
}

/* $82 SBCA immediate ?**** */
OP_HANDLER( sbca_im )
{
	u16    t,r;
	IMMBYTE(t); r = A-t-(CC&0x01);
	CLR_NZVC; SET_FLAGS8(A,t,r);
	A = r;
}

/* $83 SUBD immediate -**** */
OP_HANDLER( subd_im )
{
	u32 r,d;
	PAIR b;
	IMMWORD(b);
	d = D;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
	D = r;
}

/* $84 ANDA immediate -**0- */
OP_HANDLER( anda_im )
{
	u8 t;
	IMMBYTE(t); A &= t;
	CLR_NZV; SET_NZ8(A);
}

/* $85 BITA immediate -**0- */
OP_HANDLER( bita_im )
{
	u8 t,r;
	IMMBYTE(t); r = A&t;
	CLR_NZV; SET_NZ8(r);
}

/* $86 LDA immediate -**0- */
OP_HANDLER( lda_im )
{
	IMMBYTE(A);
	CLR_NZV; SET_NZ8(A);
}

/* is this a legal instruction? */
/* $87 STA immediate -**0- */
OP_HANDLER( sta_im )
{
	CLR_NZV; SET_NZ8(A);
	IMM8; WM(EAD,A);
}

/* $88 EORA immediate -**0- */
OP_HANDLER( eora_im )
{
	u8 t;
	IMMBYTE(t); A ^= t;
	CLR_NZV; SET_NZ8(A);
}

/* $89 ADCA immediate ***** */
OP_HANDLER( adca_im )
{
	u16 t,r;
	IMMBYTE(t); r = A+t+(CC&0x01);
	CLR_HNZVC; SET_FLAGS8(A,t,r); SET_H(A,t,r);
	A = r;
}

/* $8a ORA immediate -**0- */
OP_HANDLER( ora_im )
{
	u8 t;
	IMMBYTE(t); A |= t;
	CLR_NZV; SET_NZ8(A);
}

/* $8b ADDA immediate ***** */
OP_HANDLER( adda_im )
{
	u16 t,r;
	IMMBYTE(t); r = A+t;
	CLR_HNZVC; SET_FLAGS8(A,t,r); SET_H(A,t,r);
	A = r;
}

/* $8c CMPX immediate -***- */
OP_HANDLER( cmpx_im )
{
	PAIR r,d,b;
	IMMWORD(b);
	d.d = X;
	r.w.l = d.b.h - b.b.h;
	CLR_NZV;
	SET_N8(r.b.l);
	SET_V8(d.b.h, b.b.h, r.w.l);
	r.d = d.d - b.d;
	SET_Z16(r.d);
}

/* $8c CPX immediate -**** (6803) */
OP_HANDLER( cpx_im )
{
	u32 r,d;
	PAIR b;
	IMMWORD(b);
	d = X;
	r = d - b.d;
	CLR_NZVC; SET_FLAGS16(d,b.d,r);
}


/* $8d BSR ----- */
OP_HANDLER( bsr )
{
	u8 t;
	IMMBYTE(t);
	PUSHWORD(pPC);
	PC += SIGNED(t);
}

/* $8e LDS immediate -**0- */
OP_HANDLER( lds_im )
{
	IMMWORD(m_s);
	CLR_NZV;
	SET_NZ16(S);
}

/* $8f STS immediate -**0- */
OP_HANDLER( sts_im )
{
	CLR_NZV;
	SET_NZ16(S);
	IMM16;
	WM16(EAD,&m_s);
}

/* $90 SUBA direct ?**** */
OP_HANDLER( suba_di )
{
	u16    t,r;
	DIRBYTE(t); r = A-t;
	CLR_NZVC; SET_FLAGS8(A,t,r);
	A = r;
}

/* $91 CMPA direct ?**** */
OP_HANDLER( cmpa_di )
{
	u16    t,r;
	DIRBYTE(t); r = A-t;
	CLR_NZVC; SET_FLAGS8(A,t,r);
}

/* $92 SBCA direct ?**** */
OP_HANDLER( sbca_di )
{
	u16    t,r;
	DIRBYTE(t); r = A-t-(CC&0x01);
	CLR_NZVC; SET_FLAGS8(A,t,r);
	A = r;
}

/* $93 SUBD direct -**** */
OP_HANDLER( subd_di )
{
	u32 r,d;
	PAIR b;
	DIRWORD(b);
	d = D;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
	D=r;
}

/* $94 ANDA direct -**0- */
OP_HANDLER( anda_di )
{
	u8 t;
	DIRBYTE(t); A &= t;
	CLR_NZV; SET_NZ8(A);
}

/* $95 BITA direct -**0- */
OP_HANDLER( bita_di )
{
	u8 t,r;
	DIRBYTE(t); r = A&t;
	CLR_NZV; SET_NZ8(r);
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
	u8 t;
	DIRBYTE(t);
	A ^= t;
	CLR_NZV;
	SET_NZ8(A);
}

/* $99 ADCA direct ***** */
OP_HANDLER( adca_di )
{
	u16 t,r;
	DIRBYTE(t);
	r = A+t+(CC&0x01);
	CLR_HNZVC;
	SET_FLAGS8(A,t,r);
	SET_H(A,t,r);
	A = r;
}

/* $9a ORA direct -**0- */
OP_HANDLER( ora_di )
{
	u8 t;
	DIRBYTE(t);
	A |= t;
	CLR_NZV;
	SET_NZ8(A);
}

/* $9b ADDA direct ***** */
OP_HANDLER( adda_di )
{
	u16 t,r;
	DIRBYTE(t);
	r = A + t;
	CLR_HNZVC;
	SET_FLAGS8(A,t,r);
	SET_H(A,t,r);
	A = r;
}

/* $9c CMPX direct -***- */
OP_HANDLER( cmpx_di )
{
	PAIR r,d,b;
	DIRWORD(b);
	d.d = X;
	r.w.l = d.b.h - b.b.h;
	CLR_NZV;
	SET_N8(r.b.l);
	SET_V8(d.b.h, b.b.h, r.w.l);
	r.d = d.d - b.d;
	SET_Z16(r.d);
}

/* $9c CPX direct -**** (6803) */
OP_HANDLER( cpx_di )
{
	u32 r,d;
	PAIR b;
	DIRWORD(b);
	d = X;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
}

/* $9d JSR direct ----- */
OP_HANDLER( jsr_di )
{
	DIRECT;
	PUSHWORD(pPC);
	PC = EA;
}

/* $9e LDS direct -**0- */
OP_HANDLER( lds_di )
{
	DIRWORD(m_s);
	CLR_NZV;
	SET_NZ16(S);
}

/* $9f STS direct -**0- */
OP_HANDLER( sts_di )
{
	CLR_NZV;
	SET_NZ16(S);
	DIRECT;
	WM16(EAD,&m_s);
}

/* $a0 SUBA indexed ?**** */
OP_HANDLER( suba_ix )
{
	u16    t,r;
	IDXBYTE(t);
	r = A - t;
	CLR_NZVC;
	SET_FLAGS8(A,t,r);
	A = r;
}

/* $a1 CMPA indexed ?**** */
OP_HANDLER( cmpa_ix )
{
	u16    t,r;
	IDXBYTE(t);
	r = A - t;
	CLR_NZVC;
	SET_FLAGS8(A,t,r);
}

/* $a2 SBCA indexed ?**** */
OP_HANDLER( sbca_ix )
{
	u16    t,r;
	IDXBYTE(t);
	r = A - t - (CC&0x01);
	CLR_NZVC;
	SET_FLAGS8(A,t,r);
	A = r;
}

/* $a3 SUBD indexed -**** */
OP_HANDLER( subd_ix )
{
	u32 r,d;
	PAIR b;
	IDXWORD(b);
	d = D;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
	D = r;
}

/* $a4 ANDA indexed -**0- */
OP_HANDLER( anda_ix )
{
	u8 t;
	IDXBYTE(t); A &= t;
	CLR_NZV;
	SET_NZ8(A);
}

/* $a5 BITA indexed -**0- */
OP_HANDLER( bita_ix )
{
	u8 t,r;
	IDXBYTE(t); r = A&t;
	CLR_NZV;
	SET_NZ8(r);
}

/* $a6 LDA indexed -**0- */
OP_HANDLER( lda_ix )
{
	IDXBYTE(A);
	CLR_NZV;
	SET_NZ8(A);
}

/* $a7 STA indexed -**0- */
OP_HANDLER( sta_ix )
{
	CLR_NZV;
	SET_NZ8(A);
	INDEXED;
	WM(EAD,A);
}

/* $a8 EORA indexed -**0- */
OP_HANDLER( eora_ix )
{
	u8 t;
	IDXBYTE(t);
	A ^= t;
	CLR_NZV;
	SET_NZ8(A);
}

/* $a9 ADCA indexed ***** */
OP_HANDLER( adca_ix )
{
	u16 t,r;
	IDXBYTE(t);
	r = A + t + (CC&0x01);
	CLR_HNZVC;
	SET_FLAGS8(A,t,r);
	SET_H(A,t,r);
	A = r;
}

/* $aa ORA indexed -**0- */
OP_HANDLER( ora_ix )
{
	u8 t;
	IDXBYTE(t);
	A |= t;
	CLR_NZV;
	SET_NZ8(A);
}

/* $ab ADDA indexed ***** */
OP_HANDLER( adda_ix )
{
	u16 t,r;
	IDXBYTE(t);
	r = A+t;
	CLR_HNZVC;
	SET_FLAGS8(A,t,r);
	SET_H(A,t,r);
	A = r;
}

/* $ac CMPX indexed -***- */
OP_HANDLER( cmpx_ix )
{
	PAIR r,d,b;
	IDXWORD(b);
	d.d = X;
	r.w.l = d.b.h - b.b.h;
	CLR_NZV;
	SET_N8(r.b.l);
	SET_V8(d.b.h, b.b.h, r.w.l);
	r.d = d.d - b.d;
	SET_Z16(r.d);
}

/* $ac CPX indexed -**** (6803)*/
OP_HANDLER( cpx_ix )
{
	u32 r,d;
	PAIR b;
	IDXWORD(b);
	d = X;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
}

/* $ad JSR indexed ----- */
OP_HANDLER( jsr_ix )
{
	INDEXED;
	PUSHWORD(pPC);
	PC = EA;
}

/* $ae LDS indexed -**0- */
OP_HANDLER( lds_ix )
{
	IDXWORD(m_s);
	CLR_NZV;
	SET_NZ16(S);
}

/* $af STS indexed -**0- */
OP_HANDLER( sts_ix )
{
	CLR_NZV;
	SET_NZ16(S);
	INDEXED;
	WM16(EAD,&m_s);
}

/* $b0 SUBA extended ?**** */
OP_HANDLER( suba_ex )
{
	u16    t,r;
	EXTBYTE(t);
	r = A - t;
	CLR_NZVC;
	SET_FLAGS8(A,t,r);
	A = r;
}

/* $b1 CMPA extended ?**** */
OP_HANDLER( cmpa_ex )
{
	u16    t,r;
	EXTBYTE(t);
	r = A-t;
	CLR_NZVC;
	SET_FLAGS8(A,t,r);
}

/* $b2 SBCA extended ?**** */
OP_HANDLER( sbca_ex )
{
	u16    t,r;
	EXTBYTE(t);
	r = A-t-(CC&0x01);
	CLR_NZVC;
	SET_FLAGS8(A,t,r);
	A = r;
}

/* $b3 SUBD extended -**** */
OP_HANDLER( subd_ex )
{
	u32 r,d;
	PAIR b;
	EXTWORD(b);
	d = D;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
	D=r;
}

/* $b4 ANDA extended -**0- */
OP_HANDLER( anda_ex )
{
	u8 t;
	EXTBYTE(t);
	A &= t;
	CLR_NZV;
	SET_NZ8(A);
}

/* $b5 BITA extended -**0- */
OP_HANDLER( bita_ex )
{
	u8 t,r;
	EXTBYTE(t);
	r = A&t;
	CLR_NZV;
	SET_NZ8(r);
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
	u8 t;
	EXTBYTE(t);
	A ^= t;
	CLR_NZV;
	SET_NZ8(A);
}

/* $b9 ADCA extended ***** */
OP_HANDLER( adca_ex )
{
	u16 t,r;
	EXTBYTE(t);
	r = A+t+(CC&0x01);
	CLR_HNZVC;
	SET_FLAGS8(A,t,r);
	SET_H(A,t,r);
	A = r;
}

/* $ba ORA extended -**0- */
OP_HANDLER( ora_ex )
{
	u8 t;
	EXTBYTE(t);
	A |= t;
	CLR_NZV;
	SET_NZ8(A);
}

/* $bb ADDA extended ***** */
OP_HANDLER( adda_ex )
{
	u16 t,r;
	EXTBYTE(t);
	r = A+t;
	CLR_HNZVC;
	SET_FLAGS8(A,t,r);
	SET_H(A,t,r);
	A = r;
}

/* $bc CMPX extended -***- */
OP_HANDLER( cmpx_ex )
{
	PAIR r,d,b;
	EXTWORD(b);
	d.d = X;
	r.w.l = d.b.h - b.b.h;
	CLR_NZV;
	SET_N8(r.b.l);
	SET_V8(d.b.h, b.b.h, r.w.l);
	r.d = d.d - b.d;
	SET_Z16(r.d);
}

/* $bc CPX extended -**** (6803) */
OP_HANDLER( cpx_ex )
{
	u32 r,d;
	PAIR b;
	EXTWORD(b);
	d = X;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
}

/* $bd JSR extended ----- */
OP_HANDLER( jsr_ex )
{
	EXTENDED;
	PUSHWORD(pPC);
	PC = EA;
}

/* $be LDS extended -**0- */
OP_HANDLER( lds_ex )
{
	EXTWORD(m_s);
	CLR_NZV;
	SET_NZ16(S);
}

/* $bf STS extended -**0- */
OP_HANDLER( sts_ex )
{
	CLR_NZV;
	SET_NZ16(S);
	EXTENDED;
	WM16(EAD,&m_s);
}

/* $c0 SUBB immediate ?**** */
OP_HANDLER( subb_im )
{
	u16    t,r;
	IMMBYTE(t);
	r = B-t;
	CLR_NZVC;
	SET_FLAGS8(B,t,r);
	B = r;
}

/* $c1 CMPB immediate ?**** */
OP_HANDLER( cmpb_im )
{
	u16    t,r;
	IMMBYTE(t);
	r = B-t;
	CLR_NZVC;
	SET_FLAGS8(B,t,r);
}

/* $c2 SBCB immediate ?**** */
OP_HANDLER( sbcb_im )
{
	u16    t,r;
	IMMBYTE(t);
	r = B-t-(CC&0x01);
	CLR_NZVC;
	SET_FLAGS8(B,t,r);
	B = r;
}

/* $c3 ADDD immediate -**** */
OP_HANDLER( addd_im )
{
	u32 r,d;
	PAIR b;
	IMMWORD(b);
	d = D;
	r = d + b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
	D = r;
}

/* $c4 ANDB immediate -**0- */
OP_HANDLER( andb_im )
{
	u8 t;
	IMMBYTE(t);
	B &= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $c5 BITB immediate -**0- */
OP_HANDLER( bitb_im )
{
	u8 t,r;
	IMMBYTE(t);
	r = B&t;
	CLR_NZV;
	SET_NZ8(r);
}

/* $c6 LDB immediate -**0- */
OP_HANDLER( ldb_im )
{
	IMMBYTE(B);
	CLR_NZV;
	SET_NZ8(B);
}

/* is this a legal instruction? */
/* $c7 STB immediate -**0- */
OP_HANDLER( stb_im )
{
	CLR_NZV;
	SET_NZ8(B);
	IMM8;
	WM(EAD,B);
}

/* $c8 EORB immediate -**0- */
OP_HANDLER( eorb_im )
{
	u8 t;
	IMMBYTE(t);
	B ^= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $c9 ADCB immediate ***** */
OP_HANDLER( adcb_im )
{
	u16 t,r;
	IMMBYTE(t);
	r = B+t+(CC&0x01);
	CLR_HNZVC;
	SET_FLAGS8(B,t,r);
	SET_H(B,t,r);
	B = r;
}

/* $ca ORB immediate -**0- */
OP_HANDLER( orb_im )
{
	u8 t;
	IMMBYTE(t);
	B |= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $cb ADDB immediate ***** */
OP_HANDLER( addb_im )
{
	u16 t,r;
	IMMBYTE(t);
	r = B+t;
	CLR_HNZVC;
	SET_FLAGS8(B,t,r);
	SET_H(B,t,r);
	B = r;
}

/* $CC LDD immediate -**0- */
OP_HANDLER( ldd_im )
{
	IMMWORD(m_d);
	CLR_NZV;
	SET_NZ16(D);
}

/* is this a legal instruction? */
/* $cd STD immediate -**0- */
OP_HANDLER( std_im )
{
	IMM16;
	CLR_NZV;
	SET_NZ16(D);
	WM16(EAD,&m_d);
}

/* $ce LDX immediate -**0- */
OP_HANDLER( ldx_im )
{
	IMMWORD(m_x);
	CLR_NZV;
	SET_NZ16(X);
}

/* $cf STX immediate -**0- */
OP_HANDLER( stx_im )
{
	CLR_NZV;
	SET_NZ16(X);
	IMM16;
	WM16(EAD,&m_x);
}

/* $d0 SUBB direct ?**** */
OP_HANDLER( subb_di )
{
	u16    t,r;
	DIRBYTE(t);
	r = B-t;
	CLR_NZVC;
	SET_FLAGS8(B,t,r);
	B = r;
}

/* $d1 CMPB direct ?**** */
OP_HANDLER( cmpb_di )
{
	u16    t,r;
	DIRBYTE(t);
	r = B-t;
	CLR_NZVC;
	SET_FLAGS8(B,t,r);
}

/* $d2 SBCB direct ?**** */
OP_HANDLER( sbcb_di )
{
	u16    t,r;
	DIRBYTE(t);
	r = B-t-(CC&0x01);
	CLR_NZVC;
	SET_FLAGS8(B,t,r);
	B = r;
}

/* $d3 ADDD direct -**** */
OP_HANDLER( addd_di )
{
	u32 r,d;
	PAIR b;
	DIRWORD(b);
	d = D;
	r = d + b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
	D = r;
}

/* $d4 ANDB direct -**0- */
OP_HANDLER( andb_di )
{
	u8 t;
	DIRBYTE(t);
	B &= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $d5 BITB direct -**0- */
OP_HANDLER( bitb_di )
{
	u8 t,r;
	DIRBYTE(t);
	r = B&t;
	CLR_NZV;
	SET_NZ8(r);
}

/* $d6 LDB direct -**0- */
OP_HANDLER( ldb_di )
{
	DIRBYTE(B);
	CLR_NZV;
	SET_NZ8(B);
}

/* $d7 STB direct -**0- */
OP_HANDLER( stb_di )
{
	CLR_NZV;
	SET_NZ8(B);
	DIRECT;
	WM(EAD,B);
}

/* $d8 EORB direct -**0- */
OP_HANDLER( eorb_di )
{
	u8 t;
	DIRBYTE(t);
	B ^= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $d9 ADCB direct ***** */
OP_HANDLER( adcb_di )
{
	u16 t,r;
	DIRBYTE(t);
	r = B+t+(CC&0x01);
	CLR_HNZVC;
	SET_FLAGS8(B,t,r);
	SET_H(B,t,r);
	B = r;
}

/* $da ORB direct -**0- */
OP_HANDLER( orb_di )
{
	u8 t;
	DIRBYTE(t);
	B |= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $db ADDB direct ***** */
OP_HANDLER( addb_di )
{
	u16 t,r;
	DIRBYTE(t);
	r = B+t;
	CLR_HNZVC;
	SET_FLAGS8(B,t,r);
	SET_H(B,t,r);
	B = r;
}

/* $dc LDD direct -**0- */
OP_HANDLER( ldd_di )
{
	DIRWORD(m_d);
	CLR_NZV;
	SET_NZ16(D);
}

/* $dd STD direct -**0- */
OP_HANDLER( std_di )
{
	DIRECT;
	CLR_NZV;
	SET_NZ16(D);
	WM16(EAD,&m_d);
}

/* $de LDX direct -**0- */
OP_HANDLER( ldx_di )
{
	DIRWORD(m_x);
	CLR_NZV;
	SET_NZ16(X);
}

/* $dF STX direct -**0- */
OP_HANDLER( stx_di )
{
	CLR_NZV;
	SET_NZ16(X);
	DIRECT;
	WM16(EAD,&m_x);
}

/* $e0 SUBB indexed ?**** */
OP_HANDLER( subb_ix )
{
	u16    t,r;
	IDXBYTE(t);
	r = B-t;
	CLR_NZVC;
	SET_FLAGS8(B,t,r);
	B = r;
}

/* $e1 CMPB indexed ?**** */
OP_HANDLER( cmpb_ix )
{
	u16    t,r;
	IDXBYTE(t);
	r = B-t;
	CLR_NZVC;
	SET_FLAGS8(B,t,r);
}

/* $e2 SBCB indexed ?**** */
OP_HANDLER( sbcb_ix )
{
	u16    t,r;
	IDXBYTE(t);
	r = B-t-(CC&0x01);
	CLR_NZVC;
	SET_FLAGS8(B,t,r);
	B = r;
}

/* $e3 ADDD indexed -**** */
OP_HANDLER( addd_ix )
{
	u32 r,d;
	PAIR b;
	IDXWORD(b);
	d = D;
	r = d + b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
	D = r;
}

/* $e4 ANDB indexed -**0- */
OP_HANDLER( andb_ix )
{
	u8 t;
	IDXBYTE(t);
	B &= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $e5 BITB indexed -**0- */
OP_HANDLER( bitb_ix )
{
	u8 t,r;
	IDXBYTE(t);
	r = B&t;
	CLR_NZV;
	SET_NZ8(r);
}

/* $e6 LDB indexed -**0- */
OP_HANDLER( ldb_ix )
{
	IDXBYTE(B);
	CLR_NZV;
	SET_NZ8(B);
}

/* $e7 STB indexed -**0- */
OP_HANDLER( stb_ix )
{
	CLR_NZV;
	SET_NZ8(B);
	INDEXED;
	WM(EAD,B);
}

/* $e8 EORB indexed -**0- */
OP_HANDLER( eorb_ix )
{
	u8 t;
	IDXBYTE(t);
	B ^= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $e9 ADCB indexed ***** */
OP_HANDLER( adcb_ix )
{
	u16 t,r;
	IDXBYTE(t);
	r = B+t+(CC&0x01);
	CLR_HNZVC;
	SET_FLAGS8(B,t,r);
	SET_H(B,t,r);
	B = r;
}

/* $ea ORB indexed -**0- */
OP_HANDLER( orb_ix )
{
	u8 t;
	IDXBYTE(t);
	B |= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $eb ADDB indexed ***** */
OP_HANDLER( addb_ix )
{
	u16 t,r;
	IDXBYTE(t);
	r = B+t;
	CLR_HNZVC;
	SET_FLAGS8(B,t,r);
	SET_H(B,t,r);
	B = r;
}

/* $ec LDD indexed -**0- */
OP_HANDLER( ldd_ix )
{
	IDXWORD(m_d);
	CLR_NZV;
	SET_NZ16(D);
}

/* $ec ADCX immediate -****    NSC8105 only.  Flags are a guess - copied from addb_im() */
// actually this is ADDX, causes garbage in nightgal.cpp otherwise
OP_HANDLER( adcx_im )
{
	u16 t,r;
	IMMBYTE(t);
	r = X+t;
	CLR_HNZVC;
	SET_FLAGS8(X,t,r);
	SET_H(X,t,r);
	X = r;
}

/* $ed STD indexed -**0- */
OP_HANDLER( std_ix )
{
	INDEXED;
	CLR_NZV;
	SET_NZ16(D);
	WM16(EAD,&m_d);
}

/* $ee LDX indexed -**0- */
OP_HANDLER( ldx_ix )
{
	IDXWORD(m_x);
	CLR_NZV;
	SET_NZ16(X);
}

/* $ef STX indexed -**0- */
OP_HANDLER( stx_ix )
{
	CLR_NZV;
	SET_NZ16(X);
	INDEXED;
	WM16(EAD,&m_x);
}

/* $f0 SUBB extended ?**** */
OP_HANDLER( subb_ex )
{
	u16    t,r;
	EXTBYTE(t);
	r = B-t;
	CLR_NZVC;
	SET_FLAGS8(B,t,r);
	B = r;
}

/* $f1 CMPB extended ?**** */
OP_HANDLER( cmpb_ex )
{
	u16    t,r;
	EXTBYTE(t);
	r = B-t;
	CLR_NZVC;
	SET_FLAGS8(B,t,r);
}

/* $f2 SBCB extended ?**** */
OP_HANDLER( sbcb_ex )
{
	u16    t,r;
	EXTBYTE(t);
	r = B-t-(CC&0x01);
	CLR_NZVC;
	SET_FLAGS8(B,t,r);
	B = r;
}

/* $f3 ADDD extended -**** */
OP_HANDLER( addd_ex )
{
	u32 r,d;
	PAIR b;
	EXTWORD(b);
	d = D;
	r = d + b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
	D = r;
}

/* $f4 ANDB extended -**0- */
OP_HANDLER( andb_ex )
{
	u8 t;
	EXTBYTE(t);
	B &= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $f5 BITB extended -**0- */
OP_HANDLER( bitb_ex )
{
	u8 t,r;
	EXTBYTE(t);
	r = B & t;
	CLR_NZV;
	SET_NZ8(r);
}

/* $f6 LDB extended -**0- */
OP_HANDLER( ldb_ex )
{
	EXTBYTE(B);
	CLR_NZV;
	SET_NZ8(B);
}

/* $f7 STB extended -**0- */
OP_HANDLER( stb_ex )
{
	CLR_NZV;
	SET_NZ8(B);
	EXTENDED;
	WM(EAD,B);
}

/* $f8 EORB extended -**0- */
OP_HANDLER( eorb_ex )
{
	u8 t;
	EXTBYTE(t);
	B ^= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $f9 ADCB extended ***** */
OP_HANDLER( adcb_ex )
{
	u16 t,r;
	EXTBYTE(t);
	r = B+t+(CC&0x01);
	CLR_HNZVC;
	SET_FLAGS8(B,t,r);
	SET_H(B,t,r);
	B = r;
}

/* $fa ORB extended -**0- */
OP_HANDLER( orb_ex )
{
	u8 t;
	EXTBYTE(t);
	B |= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $fb ADDB extended ***** */
OP_HANDLER( addb_ex )
{
	u16 t,r;
	EXTBYTE(t);
	r = B+t;
	CLR_HNZVC;
	SET_FLAGS8(B,t,r);
	SET_H(B,t,r);
	B = r;
}

/* $fc LDD extended -**0- */
OP_HANDLER( ldd_ex )
{
	EXTWORD(m_d);
	CLR_NZV;
	SET_NZ16(D);
}

/* $fc ADDX extended -****    NSC8105 only.  Flags are a guess */
OP_HANDLER( addx_ex )
{
	u32 r,d;
	PAIR b;
	EXTWORD(b);
	d = X;
	r = d + b.d;
	CLR_NZVC;
	SET_FLAGS16(d,b.d,r);
	X = r;
}

/* $fd STD extended -**0- */
OP_HANDLER( std_ex )
{
	EXTENDED;
	CLR_NZV;
	SET_NZ16(D);
	WM16(EAD,&m_d);
}

/* $fe LDX extended -**0- */
OP_HANDLER( ldx_ex )
{
	EXTWORD(m_x);
	CLR_NZV;
	SET_NZ16(X);
}

/* $ff STX extended -**0- */
OP_HANDLER( stx_ex )
{
	CLR_NZV;
	SET_NZ16(X);
	EXTENDED;
	WM16(EAD,&m_x);
}

/* NSC8105 specific, guessed opcodes (tested by Night Gal Summer) */
// $bb - $mask & [X + $disp8]
OP_HANDLER( btst_ix )
{
	u8 val;
	u8 mask = M_RDOP_ARG(PCD);
	{EA=X+(M_RDOP_ARG(PCD+1));PC+=2;}
	val = RM(EAD) & mask;
	CLR_NZVC; SET_NZ8(val);
}

// $b2 - assuming correct, store first byte to (X + $disp8)
OP_HANDLER( stx_nsc )
{
	IMM8;
	u8 val = RM(EAD);
	IMM8;
	EA = X + RM(EAD);
	CLR_NZV;
	SET_NZ8(val);
	WM(EAD,val);
}
