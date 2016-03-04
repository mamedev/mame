// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*

HNZC

? = undefined
* = affected
- = unaffected
0 = cleared
1 = set
# = ccr directly affected by instruction
@ = special - carry set if bit 7 is set

*/

#define OP_HANDLER(_name) void m6805_base_device::_name()
#define DERIVED_OP_HANDLER(_arch,_name) void _arch##_device::_name()

#define OP_HANDLER_BIT(_name) void m6805_base_device::_name(UINT8 bit)

OP_HANDLER( illegal )
{
	logerror("M6805: illegal opcode\n");
}

/* $00/$02/$04/$06/$08/$0A/$0C/$0E BRSET direct,relative ---- */
OP_HANDLER_BIT( brset )
{
	UINT8 t,r;
	DIRBYTE(r);
	IMMBYTE(t);

	CLC;

	if (r&bit) {
		SEC;
		PC+=SIGNED(t);
	}
}

/* $01/$03/$05/$07/$09/$0B/$0D/$0F BRCLR direct,relative ---- */
OP_HANDLER_BIT( brclr )
{
	UINT8 t,r;
	DIRBYTE(r);
	IMMBYTE(t);

	SEC;

	if (!(r&bit)) {
		CLC;
		PC+=SIGNED(t);
	}
}

/* $10/$12/$14/$16/$18/$1A/$1C/$1E BSET direct ---- */
OP_HANDLER_BIT( bset )
{
	UINT8 t,r;
	DIRBYTE(t); r=t|bit;
	WM(EAD,r);
}

/* $11/$13/$15/$17/$19/$1B/$1D/$1F BCLR direct ---- */
OP_HANDLER_BIT( bclr)
{
	UINT8 t,r;
	DIRBYTE(t); r=t&(~bit);
	WM(EAD,r);
}

/* $20 BRA relative ---- */
OP_HANDLER( bra )
{
	UINT8 t;
	IMMBYTE(t);
	PC+=SIGNED(t);
}

/* $21 BRN relative ---- */
OP_HANDLER( brn )
{
	SKIPBYTE();
}

/* $22 BHI relative ---- */
OP_HANDLER( bhi )
{
	BRANCH( !(CC&(CFLAG|ZFLAG)) );
}

/* $23 BLS relative ---- */
OP_HANDLER( bls )
{
	BRANCH( CC&(CFLAG|ZFLAG) );
}

/* $24 BCC relative ---- */
OP_HANDLER( bcc )
{
	BRANCH( !(CC&CFLAG) );
}

/* $25 BCS relative ---- */
OP_HANDLER( bcs )
{
	BRANCH( CC&CFLAG );
}

/* $26 BNE relative ---- */
OP_HANDLER( bne )
{
	BRANCH( !(CC&ZFLAG) );
}

/* $27 BEQ relative ---- */
OP_HANDLER( beq )
{
	BRANCH( CC&ZFLAG );
}

/* $28 BHCC relative ---- */
OP_HANDLER( bhcc )
{
	BRANCH( !(CC&HFLAG) );
}

/* $29 BHCS relative ---- */
OP_HANDLER( bhcs )
{
	BRANCH( CC&HFLAG );
}

/* $2a BPL relative ---- */
OP_HANDLER( bpl )
{
	BRANCH( !(CC&NFLAG) );
}

/* $2b BMI relative ---- */
OP_HANDLER( bmi )
{
	BRANCH( CC&NFLAG );
}

/* $2c BMC relative ---- */
OP_HANDLER( bmc )
{
	BRANCH( !(CC&IFLAG) );
}

/* $2d BMS relative ---- */
OP_HANDLER( bms )
{
	BRANCH( CC&IFLAG );
}

/* $2e BIL relative ---- */
OP_HANDLER( bil )
{
	BRANCH(m_irq_state[0] != CLEAR_LINE);
}

DERIVED_OP_HANDLER( hd63705, bil )
{
	BRANCH(m_nmi_state != CLEAR_LINE);
}

/* $2f BIH relative ---- */
OP_HANDLER( bih )
{
	BRANCH(m_irq_state[0] == CLEAR_LINE);
}

DERIVED_OP_HANDLER( hd63705, bih )
{
	BRANCH(m_nmi_state == CLEAR_LINE);
}

/* $30 NEG direct -*** */
OP_HANDLER( neg_di )
{
	UINT8 t;
	UINT16 r;
	DIRBYTE(t);
	r=-t;
	CLR_NZC;
	SET_FLAGS8(0,t,r);
	WM(EAD,r);
}

/* $31 ILLEGAL */

/* $32 ILLEGAL */

/* $33 COM direct -**1 */
OP_HANDLER( com_di )
{
	UINT8 t;
	DIRBYTE(t);
	t = ~t;
	CLR_NZ;
	SET_NZ8(t);
	SEC;
	WM(EAD,t);
}

/* $34 LSR direct -0** */
OP_HANDLER( lsr_di )
{
	UINT8 t;
	DIRBYTE(t);
	CLR_NZC;
	CC |= (t&0x01);
	t >>= 1;
	SET_Z8(t);
	WM(EAD,t);
}

/* $35 ILLEGAL */

/* $36 ROR direct -*** */
OP_HANDLER( ror_di )
{
	UINT8 t,r;
	DIRBYTE(t);
	r = (CC & 0x01) << 7;
	CLR_NZC;
	CC |= (t & 0x01);
	r |= t>>1;
	SET_NZ8(r);
	WM(EAD,r);
}

/* $37 ASR direct ?*** */
OP_HANDLER( asr_di )
{
	UINT8 t;
	DIRBYTE(t);
	CLR_NZC; CC|=(t&0x01);
	t>>=1; t|=((t&0x40)<<1);
	SET_NZ8(t);
	WM(EAD,t);
}

/* $38 LSL direct ?*** */
OP_HANDLER( lsl_di )
{
	UINT8 t;
	UINT16 r;
	DIRBYTE(t);
	r = t << 1;
	CLR_NZC;
	SET_FLAGS8(t,t,r);
	WM(EAD,r);
}

/* $39 ROL direct -*** */
OP_HANDLER( rol_di )
{
	UINT16 t,r;
	DIRBYTE(t);
	r = CC & 0x01;
	r |= t << 1;
	CLR_NZC;
	SET_FLAGS8(t,t,r);
	WM(EAD,r);
}

/* $3a DEC direct -**- */
OP_HANDLER( dec_di )
{
	UINT8 t;
	DIRBYTE(t);
	--t;
	CLR_NZ; SET_FLAGS8D(t);
	WM(EAD,t);
}

/* $3b ILLEGAL */

/* $3c INC direct -**- */
OP_HANDLER( inc_di )
{
	UINT8 t;
	DIRBYTE(t);
	++t;
	CLR_NZ; SET_FLAGS8I(t);
	WM(EAD,t);
}

/* $3d TST direct -**- */
OP_HANDLER( tst_di )
{
	UINT8 t;
	DIRBYTE(t);
	CLR_NZ; SET_NZ8(t);
}

/* $3e ILLEGAL */

/* $3f CLR direct -0100 */
OP_HANDLER( clr_di )
{
	DIRECT;
	CLR_NZ; SEZ;
	WM(EAD,0);
}

/* $40 NEGA inherent ?*** */
OP_HANDLER( nega )
{
	UINT16 r;
	r = -A;
	CLR_NZC; SET_FLAGS8(0,A,r);
	A = r;
}

/* $41 ILLEGAL */

/* $42 ILLEGAL */

/* $43 COMA inherent -**1 */
OP_HANDLER( coma )
{
	A = ~A;
	CLR_NZ;
	SET_NZ8(A);
	SEC;
}

/* $44 LSRA inherent -0** */
OP_HANDLER( lsra )
{
	CLR_NZC;
	CC |= (A & 0x01);
	A >>= 1;
	SET_Z8(A);
}

/* $45 ILLEGAL */

/* $46 RORA inherent -*** */
OP_HANDLER( rora )
{
	UINT8 r;
	r = (CC & 0x01) << 7;
	CLR_NZC;
	CC |= (A & 0x01);
	r |= A >> 1;
	SET_NZ8(r);
	A = r;
}

/* $47 ASRA inherent ?*** */
OP_HANDLER( asra )
{
	CLR_NZC;
	CC |= (A & 0x01);
	A = (A & 0x80) | (A >> 1);
	SET_NZ8(A);
}

/* $48 LSLA inherent ?*** */
OP_HANDLER( lsla )
{
	UINT16 r;
	r = A << 1;
	CLR_NZC;
	SET_FLAGS8(A,A,r);
	A = r;
}

/* $49 ROLA inherent -*** */
OP_HANDLER( rola )
{
	UINT16 t,r;
	t = A;
	r = CC & 0x01;
	r |= t << 1;
	CLR_NZC;
	SET_FLAGS8(t,t,r);
	A = r;
}

/* $4a DECA inherent -**- */
OP_HANDLER( deca )
{
	--A;
	CLR_NZ;
	SET_FLAGS8D(A);
}

/* $4b ILLEGAL */

/* $4c INCA inherent -**- */
OP_HANDLER( inca )
{
	++A;
	CLR_NZ;
	SET_FLAGS8I(A);
}

/* $4d TSTA inherent -**- */
OP_HANDLER( tsta )
{
	CLR_NZ;
	SET_NZ8(A);
}

/* $4e ILLEGAL */

/* $4f CLRA inherent -010 */
OP_HANDLER( clra )
{
	A = 0;
	CLR_NZ;
	SEZ;
}

/* $50 NEGX inherent ?*** */
OP_HANDLER( negx )
{
	UINT16 r;
	r = -X;
	CLR_NZC;
	SET_FLAGS8(0,X,r);
	X = r;
}

/* $51 ILLEGAL */

/* $52 ILLEGAL */

/* $53 COMX inherent -**1 */
OP_HANDLER( comx )
{
	X = ~X;
	CLR_NZ;
	SET_NZ8(X);
	SEC;
}

/* $54 LSRX inherent -0** */
OP_HANDLER( lsrx )
{
	CLR_NZC;
	CC |= (X & 0x01);
	X >>= 1;
	SET_Z8(X);
}

/* $55 ILLEGAL */

/* $56 RORX inherent -*** */
OP_HANDLER( rorx )
{
	UINT8 r;
	r = (CC & 0x01) << 7;
	CLR_NZC;
	CC |= (X & 0x01);
	r |= X>>1;
	SET_NZ8(r);
	X = r;
}

/* $57 ASRX inherent ?*** */
OP_HANDLER( asrx )
{
	CLR_NZC;
	CC |= (X & 0x01);
	X = (X & 0x80) | (X >> 1);
	SET_NZ8(X);
}

/* $58 ASLX inherent ?*** */
OP_HANDLER( aslx )
{
	UINT16 r;
	r = X << 1;
	CLR_NZC;
	SET_FLAGS8(X,X,r);
	X = r;
}

/* $59 ROLX inherent -*** */
OP_HANDLER( rolx )
{
	UINT16 t,r;
	t = X;
	r = CC & 0x01;
	r |= t<<1;
	CLR_NZC;
	SET_FLAGS8(t,t,r);
	X = r;
}

/* $5a DECX inherent -**- */
OP_HANDLER( decx )
{
	--X;
	CLR_NZ;
	SET_FLAGS8D(X);
}

/* $5b ILLEGAL */

/* $5c INCX inherent -**- */
OP_HANDLER( incx )
{
	++X;
	CLR_NZ;
	SET_FLAGS8I(X);
}

/* $5d TSTX inherent -**- */
OP_HANDLER( tstx )
{
	CLR_NZ;
	SET_NZ8(X);
}

/* $5e ILLEGAL */

/* $5f CLRX inherent -010 */
OP_HANDLER( clrx )
{
	X = 0;
	CLR_NZC;
	SEZ;
}

/* $60 NEG indexed, 1 byte offset -*** */
OP_HANDLER( neg_ix1 )
{
	UINT8 t;
	UINT16 r;
	IDX1BYTE(t); r=-t;
	CLR_NZC; SET_FLAGS8(0,t,r);
	WM(EAD,r);
}

/* $61 ILLEGAL */

/* $62 ILLEGAL */

/* $63 COM indexed, 1 byte offset -**1 */
OP_HANDLER( com_ix1 )
{
	UINT8 t;
	IDX1BYTE(t); t = ~t;
	CLR_NZ; SET_NZ8(t); SEC;
	WM(EAD,t);
}

/* $64 LSR indexed, 1 byte offset -0** */
OP_HANDLER( lsr_ix1 )
{
	UINT8 t;
	IDX1BYTE(t);
	CLR_NZC;
	CC |= (t & 0x01);
	t >>= 1;
	SET_Z8(t);
	WM(EAD,t);
}

/* $65 ILLEGAL */

/* $66 ROR indexed, 1 byte offset -*** */
OP_HANDLER( ror_ix1 )
{
	UINT8 t,r;
	IDX1BYTE(t);
	r = (CC & 0x01) << 7;
	CLR_NZC;
	CC |= (t & 0x01);
	r |= t>>1;
	SET_NZ8(r);
	WM(EAD,r);
}

/* $67 ASR indexed, 1 byte offset ?*** */
OP_HANDLER( asr_ix1 )
{
	UINT8 t;
	IDX1BYTE(t);
	CLR_NZC; CC|=(t&0x01);
	t>>=1; t|=((t&0x40)<<1);
	SET_NZ8(t);
	WM(EAD,t);
}

/* $68 LSL indexed, 1 byte offset ?*** */
OP_HANDLER( lsl_ix1 )
{
	UINT8 t;
	UINT16 r;
	IDX1BYTE(t);
	r = t << 1;
	CLR_NZC;
	SET_FLAGS8(t,t,r);
	WM(EAD,r);
}

/* $69 ROL indexed, 1 byte offset -*** */
OP_HANDLER( rol_ix1 )
{
	UINT16 t,r;
	IDX1BYTE(t);
	r = CC & 0x01;
	r |= t << 1;
	CLR_NZC;
	SET_FLAGS8(t,t,r);
	WM(EAD,r);
}

/* $6a DEC indexed, 1 byte offset -**- */
OP_HANDLER( dec_ix1 )
{
	UINT8 t;
	IDX1BYTE(t);
	--t;
	CLR_NZ; SET_FLAGS8D(t);
	WM(EAD,t);
}

/* $6b ILLEGAL */

/* $6c INC indexed, 1 byte offset -**- */
OP_HANDLER( inc_ix1 )
{
	UINT8 t;
	IDX1BYTE(t);
	++t;
	CLR_NZ; SET_FLAGS8I(t);
	WM(EAD,t);
}

/* $6d TST indexed, 1 byte offset -**- */
OP_HANDLER( tst_ix1 )
{
	UINT8 t;
	IDX1BYTE(t);
	CLR_NZ; SET_NZ8(t);
}

/* $6e ILLEGAL */

/* $6f CLR indexed, 1 byte offset -0100 */
OP_HANDLER( clr_ix1 )
{
	INDEXED1;
	CLR_NZC; SEZ;
	WM(EAD,0);
}

/* $70 NEG indexed -*** */
OP_HANDLER( neg_ix )
{
	UINT8 t;
	UINT16 r;
	IDXBYTE(t); r=-t;
	CLR_NZC; SET_FLAGS8(0,t,r);
	WM(EAD,r);
}

/* $71 ILLEGAL */

/* $72 ILLEGAL */

/* $73 COM indexed -**1 */
OP_HANDLER( com_ix )
{
	UINT8 t;
	IDXBYTE(t); t = ~t;
	CLR_NZ; SET_NZ8(t); SEC;
	WM(EAD,t);
}

/* $74 LSR indexed -0** */
OP_HANDLER( lsr_ix )
{
	UINT8 t;
	IDXBYTE(t);
	CLR_NZC;
	CC |= (t & 0x01);
	t >>= 1;
	SET_Z8(t);
	WM(EAD,t);
}

/* $75 ILLEGAL */

/* $76 ROR indexed -*** */
OP_HANDLER( ror_ix )
{
	UINT8 t,r;
	IDXBYTE(t);
	r = (CC & 0x01) << 7;
	CLR_NZC;
	CC |= (t & 0x01);
	r |= t >> 1;
	SET_NZ8(r);
	WM(EAD,r);
}

/* $77 ASR indexed ?*** */
OP_HANDLER( asr_ix )
{
	UINT8 t;
	IDXBYTE(t);
	CLR_NZC;
	CC |= (t & 0x01);
	t = (t & 0x80) | (t >> 1);
	SET_NZ8(t);
	WM(EAD,t);
}

/* $78 LSL indexed ?*** */
OP_HANDLER( lsl_ix )
{
	UINT8 t;
	UINT16 r;
	IDXBYTE(t); r=t<<1;
	CLR_NZC; SET_FLAGS8(t,t,r);
	WM(EAD,r);
}

/* $79 ROL indexed -*** */
OP_HANDLER( rol_ix )
{
	UINT16 t,r;
	IDXBYTE(t);
	r = CC & 0x01;
	r |= t << 1;
	CLR_NZC;
	SET_FLAGS8(t,t,r);
	WM(EAD,r);
}

/* $7a DEC indexed -**- */
OP_HANDLER( dec_ix )
{
	UINT8 t;
	IDXBYTE(t);
	--t;
	CLR_NZ; SET_FLAGS8D(t);
	WM(EAD,t);
}

/* $7b ILLEGAL */

/* $7c INC indexed -**- */
OP_HANDLER( inc_ix )
{
	UINT8 t;
	IDXBYTE(t);
	++t;
	CLR_NZ; SET_FLAGS8I(t);
	WM(EAD,t);
}

/* $7d TST indexed -**- */
OP_HANDLER( tst_ix )
{
	UINT8 t;
	IDXBYTE(t);
	CLR_NZ; SET_NZ8(t);
}

/* $7e ILLEGAL */

/* $7f CLR indexed -0100 */
OP_HANDLER( clr_ix )
{
	INDEXED;
	CLR_NZC; SEZ;
	WM(EAD,0);
}

/* $80 RTI inherent #### */
OP_HANDLER( rti )
{
	PULLBYTE(CC);
	PULLBYTE(A);
	PULLBYTE(X);
	PULLWORD(m_pc);
#if IRQ_LEVEL_DETECT
	if( m_irq_state != CLEAR_LINE && (CC & IFLAG) == 0 )
	{
		m_pending_interrupts |= M6805_INT_IRQ;
	}
#endif
}

/* $81 RTS inherent ---- */
OP_HANDLER( rts )
{
	PULLWORD(m_pc);
}

/* $82 ILLEGAL */

/* $83 SWI absolute indirect ---- */
OP_HANDLER( swi )
{
	PUSHWORD(m_pc);
	PUSHBYTE(m_x);
	PUSHBYTE(m_a);
	PUSHBYTE(m_cc);
	SEI;
	RM16(0xfffc, &m_pc);
}

DERIVED_OP_HANDLER( hd63705, swi )
{
	PUSHWORD(m_pc);
	PUSHBYTE(m_x);
	PUSHBYTE(m_a);
	PUSHBYTE(m_cc);
	SEI;
	RM16(0x1ffa, &m_pc);
}
/* $84 ILLEGAL */

/* $85 ILLEGAL */

/* $86 ILLEGAL */

/* $87 ILLEGAL */

/* $88 ILLEGAL */

/* $89 ILLEGAL */

/* $8A ILLEGAL */

/* $8B ILLEGAL */

/* $8C ILLEGAL */

/* $8D ILLEGAL */

/* $8E ILLEGAL */

/* $8F ILLEGAL */

/* $90 ILLEGAL */

/* $91 ILLEGAL */

/* $92 ILLEGAL */

/* $93 ILLEGAL */

/* $94 ILLEGAL */

/* $95 ILLEGAL */

/* $96 ILLEGAL */

/* $97 TAX inherent ---- */
OP_HANDLER( tax )
{
	X = A;
}

/* $98 CLC */

/* $99 SEC */

/* $9A CLI */

/* $9B SEI */

/* $9C RSP inherent ---- */
OP_HANDLER( rsp )
{
	S = SP_MASK;
}

/* $9D NOP inherent ---- */
OP_HANDLER( nop )
{
}

/* $9E ILLEGAL */

/* $9F TXA inherent ---- */
OP_HANDLER( txa )
{
	A = X;
}

/* $a0 SUBA immediate ?*** */
OP_HANDLER( suba_im )
{
	UINT16    t,r;
	IMMBYTE(t);
	r = A - t;
	CLR_NZC;
	SET_FLAGS8(A,t,r);
	A = r;
}

/* $a1 CMPA immediate ?*** */
OP_HANDLER( cmpa_im )
{
	UINT16    t,r;
	IMMBYTE(t);
	r = A - t;
	CLR_NZC;
	SET_FLAGS8(A,t,r);
}

/* $a2 SBCA immediate ?*** */
OP_HANDLER( sbca_im )
{
	UINT16    t,r;
	IMMBYTE(t);
	r = A - t - (CC & 0x01);
	CLR_NZC;
	SET_FLAGS8(A,t,r);
	A = r;
}

/* $a3 CPX immediate -*** */
OP_HANDLER( cpx_im )
{
	UINT16    t,r;
	IMMBYTE(t);
	r = X - t;
	CLR_NZC;
	SET_FLAGS8(X,t,r);
}

/* $a4 ANDA immediate -**- */
OP_HANDLER( anda_im )
{
	UINT8 t;
	IMMBYTE(t);
	A &= t;
	CLR_NZ;
	SET_NZ8(A);
}

/* $a5 BITA immediate -**- */
OP_HANDLER( bita_im )
{
	UINT8 t,r;
	IMMBYTE(t);
	r = A & t;
	CLR_NZ;
	SET_NZ8(r);
}

/* $a6 LDA immediate -**- */
OP_HANDLER( lda_im )
{
	IMMBYTE(A);
	CLR_NZ;
	SET_NZ8(A);
}

/* $a7 ILLEGAL */

/* $a8 EORA immediate -**- */
OP_HANDLER( eora_im )
{
	UINT8 t;
	IMMBYTE(t);
	A ^= t;
	CLR_NZ;
	SET_NZ8(A);
}

/* $a9 ADCA immediate **** */
OP_HANDLER( adca_im )
{
	UINT16 t,r;
	IMMBYTE(t);
	r = A + t + (CC & 0x01);
	CLR_HNZC;
	SET_FLAGS8(A,t,r);
	SET_H(A,t,r);
	A = r;
}

/* $aa ORA immediate -**- */
OP_HANDLER( ora_im )
{
	UINT8 t;
	IMMBYTE(t);
	A |= t;
	CLR_NZ;
	SET_NZ8(A);
}

/* $ab ADDA immediate **** */
OP_HANDLER( adda_im )
{
	UINT16 t,r;
	IMMBYTE(t);
	r = A + t;
	CLR_HNZC;
	SET_FLAGS8(A,t,r);
	SET_H(A,t,r);
	A = r;
}

/* $ac ILLEGAL */

/* $ad BSR ---- */
OP_HANDLER( bsr )
{
	UINT8 t;
	IMMBYTE(t);
	PUSHWORD(m_pc);
	PC += SIGNED(t);
}

/* $ae LDX immediate -**- */
OP_HANDLER( ldx_im )
{
	IMMBYTE(X);
	CLR_NZ;
	SET_NZ8(X);
}

/* $af ILLEGAL */

/* $b0 SUBA direct ?*** */
OP_HANDLER( suba_di )
{
	UINT16    t,r;
	DIRBYTE(t);
	r = A - t;
	CLR_NZC;
	SET_FLAGS8(A,t,r);
	A = r;
}

/* $b1 CMPA direct ?*** */
OP_HANDLER( cmpa_di )
{
	UINT16    t,r;
	DIRBYTE(t);
	r = A - t;
	CLR_NZC;
	SET_FLAGS8(A,t,r);
}

/* $b2 SBCA direct ?*** */
OP_HANDLER( sbca_di )
{
	UINT16    t,r;
	DIRBYTE(t);
	r = A - t - (CC & 0x01);
	CLR_NZC;
	SET_FLAGS8(A,t,r);
	A = r;
}

/* $b3 CPX direct -*** */
OP_HANDLER( cpx_di )
{
	UINT16    t,r;
	DIRBYTE(t);
	r = X - t;
	CLR_NZC;
	SET_FLAGS8(X,t,r);
}

/* $b4 ANDA direct -**- */
OP_HANDLER( anda_di )
{
	UINT8 t;
	DIRBYTE(t);
	A &= t;
	CLR_NZ;
	SET_NZ8(A);
}

/* $b5 BITA direct -**- */
OP_HANDLER( bita_di )
{
	UINT8 t,r;
	DIRBYTE(t);
	r = A & t;
	CLR_NZ;
	SET_NZ8(r);
}

/* $b6 LDA direct -**- */
OP_HANDLER( lda_di )
{
	DIRBYTE(A);
	CLR_NZ;
	SET_NZ8(A);
}

/* $b7 STA direct -**- */
OP_HANDLER( sta_di )
{
	CLR_NZ;
	SET_NZ8(A);
	DIRECT;
	WM(EAD,A);
}

/* $b8 EORA direct -**- */
OP_HANDLER( eora_di )
{
	UINT8 t;
	DIRBYTE(t);
	A ^= t;
	CLR_NZ;
	SET_NZ8(A);
}

/* $b9 ADCA direct **** */
OP_HANDLER( adca_di )
{
	UINT16 t,r;
	DIRBYTE(t);
	r = A + t + (CC & 0x01);
	CLR_HNZC;
	SET_FLAGS8(A,t,r);
	SET_H(A,t,r);
	A = r;
}

/* $ba ORA direct -**- */
OP_HANDLER( ora_di )
{
	UINT8 t;
	DIRBYTE(t);
	A |= t;
	CLR_NZ;
	SET_NZ8(A);
}

/* $bb ADDA direct **** */
OP_HANDLER( adda_di )
{
	UINT16 t,r;
	DIRBYTE(t);
	r = A + t;
	CLR_HNZC;
	SET_FLAGS8(A,t,r);
	SET_H(A,t,r);
	A = r;
}

/* $bc JMP direct -*** */
OP_HANDLER( jmp_di )
{
	DIRECT;
	PC = EA;
}

/* $bd JSR direct ---- */
OP_HANDLER( jsr_di )
{
	DIRECT;
	PUSHWORD(m_pc);
	PC = EA;
}

/* $be LDX direct -**- */
OP_HANDLER( ldx_di )
{
	DIRBYTE(X);
	CLR_NZ;
	SET_NZ8(X);
}

/* $bf STX direct -**- */
OP_HANDLER( stx_di )
{
	CLR_NZ;
	SET_NZ8(X);
	DIRECT;
	WM(EAD,X);
}

/* $c0 SUBA extended ?*** */
OP_HANDLER( suba_ex )
{
	UINT16    t,r;
	EXTBYTE(t);
	r = A - t;
	CLR_NZC;
	SET_FLAGS8(A,t,r);
	A = r;
}

/* $c1 CMPA extended ?*** */
OP_HANDLER( cmpa_ex )
{
	UINT16    t,r;
	EXTBYTE(t);
	r = A - t;
	CLR_NZC;
	SET_FLAGS8(A,t,r);
}

/* $c2 SBCA extended ?*** */
OP_HANDLER( sbca_ex )
{
	UINT16    t,r;
	EXTBYTE(t);
	r = A - t - (CC & 0x01);
	CLR_NZC;
	SET_FLAGS8(A,t,r);
	A = r;
}

/* $c3 CPX extended -*** */
OP_HANDLER( cpx_ex )
{
	UINT16    t,r;
	EXTBYTE(t);
	r = X - t;
	CLR_NZC;
	SET_FLAGS8(X,t,r);
}

/* $c4 ANDA extended -**- */
OP_HANDLER( anda_ex )
{
	UINT8 t;
	EXTBYTE(t);
	A &= t;
	CLR_NZ;
	SET_NZ8(A);
}

/* $c5 BITA extended -**- */
OP_HANDLER( bita_ex )
{
	UINT8 t,r;
	EXTBYTE(t);
	r = A & t;
	CLR_NZ;
	SET_NZ8(r);
}

/* $c6 LDA extended -**- */
OP_HANDLER( lda_ex )
{
	EXTBYTE(A);
	CLR_NZ;
	SET_NZ8(A);
}

/* $c7 STA extended -**- */
OP_HANDLER( sta_ex )
{
	CLR_NZ;
	SET_NZ8(A);
	EXTENDED;
	WM(EAD,A);
}

/* $c8 EORA extended -**- */
OP_HANDLER( eora_ex )
{
	UINT8 t;
	EXTBYTE(t);
	A ^= t;
	CLR_NZ;
	SET_NZ8(A);
}

/* $c9 ADCA extended **** */
OP_HANDLER( adca_ex )
{
	UINT16 t,r;
	EXTBYTE(t);
	r = A + t + (CC & 0x01);
	CLR_HNZC;
	SET_FLAGS8(A,t,r);
	SET_H(A,t,r);
	A = r;
}

/* $ca ORA extended -**- */
OP_HANDLER( ora_ex )
{
	UINT8 t;
	EXTBYTE(t);
	A |= t;
	CLR_NZ;
	SET_NZ8(A);
}

/* $cb ADDA extended **** */
OP_HANDLER( adda_ex )
{
	UINT16 t,r;
	EXTBYTE(t);
	r = A + t;
	CLR_HNZC;
	SET_FLAGS8(A,t,r);
	SET_H(A,t,r);
	A = r;
}

/* $cc JMP extended -*** */
OP_HANDLER( jmp_ex )
{
	EXTENDED;
	PC = EA;
}

/* $cd JSR extended ---- */
OP_HANDLER( jsr_ex )
{
	EXTENDED;
	PUSHWORD(m_pc);
	PC = EA;
}

/* $ce LDX extended -**- */
OP_HANDLER( ldx_ex )
{
	EXTBYTE(X);
	CLR_NZ;
	SET_NZ8(X);
}

/* $cf STX extended -**- */
OP_HANDLER( stx_ex )
{
	CLR_NZ;
	SET_NZ8(X);
	EXTENDED;
	WM(EAD,X);
}

/* $d0 SUBA indexed, 2 byte offset ?*** */
OP_HANDLER( suba_ix2 )
{
	UINT16    t,r;
	IDX2BYTE(t);
	r = A - t;
	CLR_NZC;
	SET_FLAGS8(A,t,r);
	A = r;
}

/* $d1 CMPA indexed, 2 byte offset ?*** */
OP_HANDLER( cmpa_ix2 )
{
	UINT16    t,r;
	IDX2BYTE(t);
	r = A - t;
	CLR_NZC;
	SET_FLAGS8(A,t,r);
}

/* $d2 SBCA indexed, 2 byte offset ?*** */
OP_HANDLER( sbca_ix2 )
{
	UINT16    t,r;
	IDX2BYTE(t);
	r = A - t - (CC & 0x01);
	CLR_NZC;
	SET_FLAGS8(A,t,r);
	A = r;
}

/* $d3 CPX indexed, 2 byte offset -*** */
OP_HANDLER( cpx_ix2 )
{
	UINT16    t,r;
	IDX2BYTE(t);
	r = X - t;
	CLR_NZC;
	SET_FLAGS8(X,t,r);
}

/* $d4 ANDA indexed, 2 byte offset -**- */
OP_HANDLER( anda_ix2 )
{
	UINT8 t;
	IDX2BYTE(t);
	A &= t;
	CLR_NZ;
	SET_NZ8(A);
}

/* $d5 BITA indexed, 2 byte offset -**- */
OP_HANDLER( bita_ix2 )
{
	UINT8 t,r;
	IDX2BYTE(t);
	r = A & t;
	CLR_NZ;
	SET_NZ8(r);
}

/* $d6 LDA indexed, 2 byte offset -**- */
OP_HANDLER( lda_ix2 )
{
	IDX2BYTE(A);
	CLR_NZ;
	SET_NZ8(A);
}

/* $d7 STA indexed, 2 byte offset -**- */
OP_HANDLER( sta_ix2 )
{
	CLR_NZ;
	SET_NZ8(A);
	INDEXED2;
	WM(EAD,A);
}

/* $d8 EORA indexed, 2 byte offset -**- */
OP_HANDLER( eora_ix2 )
{
	UINT8 t;
	IDX2BYTE(t);
	A ^= t;
	CLR_NZ;
	SET_NZ8(A);
}

/* $d9 ADCA indexed, 2 byte offset **** */
OP_HANDLER( adca_ix2 )
{
	UINT16 t,r;
	IDX2BYTE(t);
	r = A + t + (CC & 0x01);
	CLR_HNZC;
	SET_FLAGS8(A,t,r);
	SET_H(A,t,r);
	A = r;
}

/* $da ORA indexed, 2 byte offset -**- */
OP_HANDLER( ora_ix2 )
{
	UINT8 t;
	IDX2BYTE(t);
	A |= t;
	CLR_NZ;
	SET_NZ8(A);
}

/* $db ADDA indexed, 2 byte offset **** */
OP_HANDLER( adda_ix2 )
{
	UINT16 t,r;
	IDX2BYTE(t);
	r = A + t;
	CLR_HNZC;
	SET_FLAGS8(A,t,r);
	SET_H(A,t,r);
	A = r;
}

/* $dc JMP indexed, 2 byte offset -*** */
OP_HANDLER( jmp_ix2 )
{
	INDEXED2;
	PC = EA;
}

/* $dd JSR indexed, 2 byte offset ---- */
OP_HANDLER( jsr_ix2 )
{
	INDEXED2;
	PUSHWORD(m_pc);
	PC = EA;
}

/* $de LDX indexed, 2 byte offset -**- */
OP_HANDLER( ldx_ix2 )
{
	IDX2BYTE(X);
	CLR_NZ;
	SET_NZ8(X);
}

/* $df STX indexed, 2 byte offset -**- */
OP_HANDLER( stx_ix2 )
{
	CLR_NZ;
	SET_NZ8(X);
	INDEXED2;
	WM(EAD,X);
}

/* $e0 SUBA indexed, 1 byte offset ?*** */
OP_HANDLER( suba_ix1 )
{
	UINT16    t,r;
	IDX1BYTE(t);
	r = A - t;
	CLR_NZC;
	SET_FLAGS8(A,t,r);
	A = r;
}

/* $e1 CMPA indexed, 1 byte offset ?*** */
OP_HANDLER( cmpa_ix1 )
{
	UINT16    t,r;
	IDX1BYTE(t);
	r = A - t;
	CLR_NZC;
	SET_FLAGS8(A,t,r);
}

/* $e2 SBCA indexed, 1 byte offset ?*** */
OP_HANDLER( sbca_ix1 )
{
	UINT16    t,r;
	IDX1BYTE(t);
	r = A - t - (CC & 0x01);
	CLR_NZC;
	SET_FLAGS8(A,t,r);
	A = r;
}

/* $e3 CPX indexed, 1 byte offset -*** */
OP_HANDLER( cpx_ix1 )
{
	UINT16    t,r;
	IDX1BYTE(t);
	r = X - t;
	CLR_NZC;
	SET_FLAGS8(X,t,r);
}

/* $e4 ANDA indexed, 1 byte offset -**- */
OP_HANDLER( anda_ix1 )
{
	UINT8 t;
	IDX1BYTE(t);
	A &= t;
	CLR_NZ;
	SET_NZ8(A);
}

/* $e5 BITA indexed, 1 byte offset -**- */
OP_HANDLER( bita_ix1 )
{
	UINT8 t,r;
	IDX1BYTE(t);
	r = A & t;
	CLR_NZ;
	SET_NZ8(r);
}

/* $e6 LDA indexed, 1 byte offset -**- */
OP_HANDLER( lda_ix1 )
{
	IDX1BYTE(A);
	CLR_NZ;
	SET_NZ8(A);
}

/* $e7 STA indexed, 1 byte offset -**- */
OP_HANDLER( sta_ix1 )
{
	CLR_NZ;
	SET_NZ8(A);
	INDEXED1;
	WM(EAD,A);
}

/* $e8 EORA indexed, 1 byte offset -**- */
OP_HANDLER( eora_ix1 )
{
	UINT8 t;
	IDX1BYTE(t);
	A ^= t;
	CLR_NZ;
	SET_NZ8(A);
}

/* $e9 ADCA indexed, 1 byte offset **** */
OP_HANDLER( adca_ix1 )
{
	UINT16 t,r;
	IDX1BYTE(t);
	r = A + t + (CC & 0x01);
	CLR_HNZC;
	SET_FLAGS8(A,t,r);
	SET_H(A,t,r);
	A = r;
}

/* $ea ORA indexed, 1 byte offset -**- */
OP_HANDLER( ora_ix1 )
{
	UINT8 t;
	IDX1BYTE(t);
	A |= t;
	CLR_NZ;
	SET_NZ8(A);
}

/* $eb ADDA indexed, 1 byte offset **** */
OP_HANDLER( adda_ix1 )
{
	UINT16 t,r;
	IDX1BYTE(t);
	r = A + t;
	CLR_HNZC;
	SET_FLAGS8(A,t,r);
	SET_H(A,t,r);
	A = r;
}

/* $ec JMP indexed, 1 byte offset -*** */
OP_HANDLER( jmp_ix1 )
{
	INDEXED1;
	PC = EA;
}

/* $ed JSR indexed, 1 byte offset ---- */
OP_HANDLER( jsr_ix1 )
{
	INDEXED1;
	PUSHWORD(m_pc);
	PC = EA;
}

/* $ee LDX indexed, 1 byte offset -**- */
OP_HANDLER( ldx_ix1 )
{
	IDX1BYTE(X);
	CLR_NZ;
	SET_NZ8(X);
}

/* $ef STX indexed, 1 byte offset -**- */
OP_HANDLER( stx_ix1 )
{
	CLR_NZ;
	SET_NZ8(X);
	INDEXED1;
	WM(EAD,X);
}

/* $f0 SUBA indexed ?*** */
OP_HANDLER( suba_ix )
{
	UINT16    t,r;
	IDXBYTE(t);
	r = A - t;
	CLR_NZC;
	SET_FLAGS8(A,t,r);
	A = r;
}

/* $f1 CMPA indexed ?*** */
OP_HANDLER( cmpa_ix )
{
	UINT16    t,r;
	IDXBYTE(t);
	r = A - t;
	CLR_NZC;
	SET_FLAGS8(A,t,r);
}

/* $f2 SBCA indexed ?*** */
OP_HANDLER( sbca_ix )
{
	UINT16    t,r;
	IDXBYTE(t);
	r = A - t - (CC & 0x01);
	CLR_NZC;
	SET_FLAGS8(A,t,r);
	A = r;
}

/* $f3 CPX indexed -*** */
OP_HANDLER( cpx_ix )
{
	UINT16    t,r;
	IDXBYTE(t);
	r = X - t;
	CLR_NZC;
	SET_FLAGS8(X,t,r);
}

/* $f4 ANDA indexed -**- */
OP_HANDLER( anda_ix )
{
	UINT8 t;
	IDXBYTE(t);
	A &= t;
	CLR_NZ;
	SET_NZ8(A);
}

/* $f5 BITA indexed -**- */
OP_HANDLER( bita_ix )
{
	UINT8 t,r;
	IDXBYTE(t);
	r = A & t;
	CLR_NZ;
	SET_NZ8(r);
}

/* $f6 LDA indexed -**- */
OP_HANDLER( lda_ix )
{
	IDXBYTE(A);
	CLR_NZ;
	SET_NZ8(A);
}

/* $f7 STA indexed -**- */
OP_HANDLER( sta_ix )
{
	CLR_NZ;
	SET_NZ8(A);
	INDEXED;
	WM(EAD,A);
}

/* $f8 EORA indexed -**- */
OP_HANDLER( eora_ix )
{
	UINT8 t;
	IDXBYTE(t);
	A ^= t;
	CLR_NZ;
	SET_NZ8(A);
}

/* $f9 ADCA indexed **** */
OP_HANDLER( adca_ix )
{
	UINT16 t,r;
	IDXBYTE(t);
	r = A + t + (CC & 0x01);
	CLR_HNZC;
	SET_FLAGS8(A,t,r);
	SET_H(A,t,r);
	A = r;
}

/* $fa ORA indexed -**- */
OP_HANDLER( ora_ix )
{
	UINT8 t;
	IDXBYTE(t);
	A |= t;
	CLR_NZ;
	SET_NZ8(A);
}

/* $fb ADDA indexed **** */
OP_HANDLER( adda_ix )
{
	UINT16 t,r;
	IDXBYTE(t);
	r = A + t;
	CLR_HNZC;
	SET_FLAGS8(A,t,r);
	SET_H(A,t,r);
	A = r;
}

/* $fc JMP indexed -*** */
OP_HANDLER( jmp_ix )
{
	INDEXED;
	PC = EA;
}

/* $fd JSR indexed ---- */
OP_HANDLER( jsr_ix )
{
	INDEXED;
	PUSHWORD(m_pc);
	PC = EA;
}

/* $fe LDX indexed -**- */
OP_HANDLER( ldx_ix )
{
	IDXBYTE(X);
	CLR_NZ;
	SET_NZ8(X);
}

/* $ff STX indexed -**- */
OP_HANDLER( stx_ix )
{
	CLR_NZ;
	SET_NZ8(X);
	INDEXED;
	WM(EAD,X);
}
