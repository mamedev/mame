// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
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

#define OP_HANDLER(name) void m6805_base_device::name()
#define OP_HANDLER_BIT(name) template <unsigned B> void m6805_base_device::name()
#define OP_HANDLER_BRA(name) template <bool C> void m6805_base_device::name()
#define OP_HANDLER_MODE(name) template <m6805_base_device::addr_mode M> void m6805_base_device::name()


OP_HANDLER( illegal )
{
	logerror("M6805: illegal opcode\n");
}

// $00/$02/$04/$06/$08/$0A/$0C/$0E BRSET direct,relative ---*
OP_HANDLER_BIT( brset )
{
	u8 t, r;
	DIRBYTE(r);
	immbyte(t);
	CLC;
	if (BIT(r, B)) { SEC; PC += SIGNED(t); }
}

// $01/$03/$05/$07/$09/$0B/$0D/$0F BRCLR direct,relative ---*
OP_HANDLER_BIT( brclr )
{
	u8 t, r;
	DIRBYTE(r);
	immbyte(t);
	SEC;
	if (!BIT(r, B)) { CLC; PC += SIGNED(t); }
}

// $10/$12/$14/$16/$18/$1A/$1C/$1E BSET direct ----
OP_HANDLER_BIT( bset )
{
	u8 t;
	DIRBYTE(t);
	wm(EAD, t | (u8(1) << B));
}

// $11/$13/$15/$17/$19/$1B/$1D/$1F BCLR direct ----
OP_HANDLER_BIT( bclr )
{
	u8 t;
	DIRBYTE(t);
	wm(EAD, t & ~(u8(1) << B));
}

// $20 BRA relative ----
// $21 BRN relative ----
OP_HANDLER_BRA( bra ) { BRANCH( true ); }

// $22 BHI relative ----
// $23 BLS relative ----
OP_HANDLER_BRA( bhi ) { BRANCH( !(CC & (CFLAG | ZFLAG)) ); }

// $24 BCC relative ----
// $25 BCS relative ----
OP_HANDLER_BRA( bcc ) { BRANCH( !(CC & CFLAG) ); }

// $26 BNE relative ----
// $27 BEQ relative ----
OP_HANDLER_BRA( bne ) { BRANCH( !(CC & ZFLAG) ); }

// $28 BHCC relative ----
// $29 BHCS relative ----
OP_HANDLER_BRA( bhcc ) { BRANCH( !(CC & HFLAG) ); }

// $2a BPL relative ----
// $2b BMI relative ----
OP_HANDLER_BRA( bpl ) { BRANCH( !(CC & NFLAG) ); }

// $2c BMC relative ----
// $2d BMS relative ----
OP_HANDLER_BRA( bmc ) { BRANCH( !(CC & IFLAG) ); }

// $2e BIL relative ----
// $2f BIH relative ----
OP_HANDLER_BRA( bil ) { BRANCH( test_il() ); }

// $30 NEG direct                   -***
// $60 NEG indexed, 1 byte offset   -***
// $70 NEG indexed                  -***
OP_HANDLER_MODE( neg )
{
	u8 t;
	ARGBYTE(t);
	u16 const r = -t;
	clr_nzc();
	set_nzc8(r);
	wm(EAD, r);
}

// $31 ILLEGAL
// $61 ILLEGAL
// $71 ILLEGAL

// $32 ILLEGAL
// $62 ILLEGAL
// $72 ILLEGAL

// $33 COM direct                   -**1
// $63 COM indexed, 1 byte offset   -**1
// $73 COM indexed                  -**1
OP_HANDLER_MODE( com )
{
	u8 t;
	ARGBYTE(t);
	t = ~t;
	clr_nz();
	set_nz8(t);
	SEC;
	wm(EAD, t);
}

// $34 LSR direct                   -0**
// $64 LSR indexed, 1 byte offset   -0**
// $74 LSR indexed                  -0**
OP_HANDLER_MODE( lsr )
{
	u8 t;
	ARGBYTE(t);
	clr_nzc();
	CC |= BIT(t, 0);
	t >>= 1;
	set_z8(t);
	wm(EAD, t);
}

// $35 ILLEGAL
// $65 ILLEGAL
// $75 ILLEGAL

// $36 ROR direct                   -***
// $66 ROR indexed, 1 byte offset   -***
// $76 ROR indexed                  -***
OP_HANDLER_MODE( ror )
{
	u8 t;
	ARGBYTE(t);
	u8 r = BIT(CC, 0) << 7;
	clr_nzc();
	CC |= BIT(t, 0);
	r |= t >> 1;
	set_nz8(r);
	wm(EAD, r);
}

// $37 ASR direct                   -***
// $67 ASR indexed, 1 byte offset   -***
// $77 ASR indexed                  -***
OP_HANDLER_MODE( asr )
{
	u8 t;
	ARGBYTE(t);
	clr_nzc();
	CC |= BIT(t, 0);
	t = (t >> 1) | (t & 0x80);
	set_nz8(t);
	wm(EAD, t);
}

// $38 LSL direct                   -***
// $68 LSL indexed, 1 byte offset   -***
// $78 LSL indexed                  -***
OP_HANDLER_MODE( lsl )
{
	u8 t;
	ARGBYTE(t);
	u16 const r = u16(t) << 1;
	clr_nzc();
	set_nzc8(r);
	wm(EAD, r);
}

// $39 ROL direct                   -***
// $69 ROL indexed, 1 byte offset   -***
// $79 ROL indexed                  -***
OP_HANDLER_MODE( rol )
{
	u16 t;
	ARGBYTE(t);
	u16 const r = BIT(CC, 0) | (t << 1);
	clr_nzc();
	set_nzc8(r);
	wm(EAD, r);
}

// $3a DEC direct                   -**-
// $6a DEC indexed, 1 byte offset   -**-
OP_HANDLER_MODE( dec )
{
	u8 t;
	ARGBYTE(t);
	--t;
	clr_nz();
	set_nz8(t);
	wm(EAD, t);
}

// $3b ILLEGAL
// $6b ILLEGAL
// $7b ILLEGAL

// $3c INC direct                   -**-
// $6c INC indexed, 1 byte offset   -**-
// $7c INC indexed                  -**-
OP_HANDLER_MODE( inc )
{
	u8 t;
	ARGBYTE(t);
	++t;
	clr_nz();
	set_nz8(t);
	wm(EAD, t);
}

// $3d TST direct                   -**-
// $6d TST indexed, 1 byte offset   -**-
// $7d TST indexed                  -**-
OP_HANDLER_MODE( tst )
{
	u8 t;
	ARGBYTE(t);
	clr_nz();
	set_nz8(t);
}

// $3e ILLEGAL
// $6e ILLEGAL
// $7e ILLEGAL

// $3f CLR direct                   -01-
// $6f CLR indexed, 1 byte offset   -01-
// $7f CLR indexed                  -01-
OP_HANDLER_MODE( clr )
{
	ARGADDR;
	clr_nz();
	SEZ;
	wm(EAD, 0);
}

// $40 NEGA inherent -***
OP_HANDLER( nega )
{
	u16 const r = -A;
	clr_nzc();
	set_nzc8(r);
	A = r;
}

// $41 ILLEGAL

// $42 MUL inherent 0--0
OP_HANDLER( mul )
{
	u16 const r = u16(A) * X;
	clr_hc();
	X = u8(r >> 8);
	A = u8(r);
}

// $43 COMA inherent -**1
OP_HANDLER( coma )
{
	A = ~A;
	clr_nz();
	set_nz8(A);
	SEC;
}

// $44 LSRA inherent -0**
OP_HANDLER( lsra )
{
	clr_nzc();
	CC |= BIT(A, 0);
	A >>= 1;
	set_z8(A);
}

// $45 ILLEGAL

// $46 RORA inherent -***
OP_HANDLER( rora )
{
	u8 r = BIT(CC, 0) << 7;
	clr_nzc();
	CC |= BIT(A, 0);
	r |= A >> 1;
	set_nz8(r);
	A = r;
}

// $47 ASRA inherent -***
OP_HANDLER( asra )
{
	clr_nzc();
	CC |= BIT(A, 0);
	A = (A & 0x80) | (A >> 1);
	set_nz8(A);
}

// $48 LSLA inherent -***
OP_HANDLER( lsla )
{
	u16 const r = u16(A) << 1;
	clr_nzc();
	set_nzc8(r);
	A = r;
}

// $49 ROLA inherent -***
OP_HANDLER( rola )
{
	u16 const t = A;
	u16 const r = BIT(CC, 0) | (t << 1);
	clr_nzc();
	set_nzc8(r);
	A = r;
}

// $4a DECA inherent -**-
OP_HANDLER( deca )
{
	--A;
	clr_nz();
	set_nz8(A);
}

// $4b ILLEGAL

// $4c INCA inherent -**-
OP_HANDLER( inca )
{
	++A;
	clr_nz();
	set_nz8(A);
}

// $4d TSTA inherent -**-
OP_HANDLER( tsta )
{
	clr_nz();
	set_nz8(A);
}

// $4e ILLEGAL

// $4f CLRA inherent -01-
OP_HANDLER( clra )
{
	A = 0;
	clr_nz();
	SEZ;
}

// $50 NEGX inherent -***
OP_HANDLER( negx )
{
	u16 const r = -X;
	clr_nzc();
	set_nzc8(r);
	X = r;
}

// $51 ILLEGAL

// $52 ILLEGAL

// $53 COMX inherent -**1
OP_HANDLER( comx )
{
	X = ~X;
	clr_nz();
	set_nz8(X);
	SEC;
}

// $54 LSRX inherent -0**
OP_HANDLER( lsrx )
{
	clr_nzc();
	CC |= BIT(X, 0);
	X >>= 1;
	set_z8(X);
}

// $55 ILLEGAL

// $56 RORX inherent -***
OP_HANDLER( rorx )
{
	u8 r = BIT(CC, 0) << 7;
	clr_nzc();
	CC |= BIT(X, 0);
	r |= X >> 1;
	set_nz8(r);
	X = r;
}

// $57 ASRX inherent -***
OP_HANDLER( asrx )
{
	clr_nzc();
	CC |= BIT(X, 0);
	X = (X & 0x80) | (X >> 1);
	set_nz8(X);
}

// $58 LSLX inherent -***
OP_HANDLER( lslx )
{
	u16 r = u16(X) << 1;
	clr_nzc();
	set_nzc8(r);
	X = r;
}

// $59 ROLX inherent -***
OP_HANDLER( rolx )
{
	u16 const t = X;
	u16 const r = BIT(CC, 0) | (t << 1);
	clr_nzc();
	set_nzc8(r);
	X = r;
}

// $5a DECX inherent -**-
OP_HANDLER( decx )
{
	--X;
	clr_nz();
	set_nz8(X);
}

// $5b ILLEGAL

// $5c INCX inherent -**-
OP_HANDLER( incx )
{
	++X;
	clr_nz();
	set_nz8(X);
}

// $5d TSTX inherent -**-
OP_HANDLER( tstx )
{
	clr_nz();
	set_nz8(X);
}

// $5e ILLEGAL

// $5f CLRX inherent -01-
OP_HANDLER( clrx )
{
	X = 0;
	clr_nz();
	SEZ;
}

// $80 RTI inherent ####
OP_HANDLER( rti )
{
	pullbyte(CC);
	pullbyte(A);
	pullbyte(X);
	pullword(m_pc);
}

// $81 RTS inherent ----
OP_HANDLER( rts )
{
	pullword(m_pc);
}

// $82 ILLEGAL

// $83 SWI absolute indirect ----
OP_HANDLER( swi )
{
	pushword(m_pc);
	pushbyte(m_x);
	pushbyte(m_a);
	pushbyte(m_cc);
	SEI;
	rm16(m_params.m_swi_vector & m_params.m_vector_mask, m_pc);
}

// $84 ILLEGAL

// $85 ILLEGAL

// $86 ILLEGAL

// $87 ILLEGAL

// $88 ILLEGAL

// $89 ILLEGAL

// $8A ILLEGAL

// $8B ILLEGAL

// $8C ILLEGAL

// $8D ILLEGAL

// $8E STOP inherent    ----
OP_HANDLER( stop )
{
	fatalerror("m6805[%s]: unimplemented STOP", tag());
}

// $8F WAIT inherent    ----
OP_HANDLER( wait )
{
	fatalerror("m6805[%s]: unimplemented WAIT", tag());
}


// $90 ILLEGAL

// $91 ILLEGAL

// $92 ILLEGAL

// $93 ILLEGAL

// $94 ILLEGAL

// $95 ILLEGAL

// $96 ILLEGAL

// $97 TAX inherent ----
OP_HANDLER( tax ) { X = A; }

// $98 CLC
OP_HANDLER( clc ) { CLC; }

// $99 SEC
OP_HANDLER( sec ) { SEC; }

// $9A CLI
OP_HANDLER( cli ) { CLI; }

// $9B SEI
OP_HANDLER( sei ) { SEI; } // TODO: check behaviour if edge-triggered interrupt was pending when this happens

// $9C RSP inherent ----
OP_HANDLER( rsp ) { S = SP_MASK; }

// $9D NOP inherent ----
OP_HANDLER( nop ) { }

// $9E ILLEGAL

// $9F TXA inherent ----
OP_HANDLER( txa )
{
	A = X;
}

// $a0 SUBA immediate               -***
// $b0 SUBA direct                  -***
// $c0 SUBA extended                -***
// $d0 SUBA indexed, 2 byte offset  -***
// $e0 SUBA indexed, 1 byte offset  -***
// $f0 SUBA indexed                 -***
OP_HANDLER_MODE( suba )
{
	u16 t;
	ARGBYTE(t);
	u16 const r = A - t;
	clr_nzc();
	set_nzc8(r);
	A = r;
}

// $a1 CMPA immediate               -***
// $b1 CMPA direct                  -***
// $c1 CMPA extended                -***
// $d1 CMPA indexed, 2 byte offset  -***
// $e1 CMPA indexed, 1 byte offset  -***
// $f1 CMPA indexed                 -***
OP_HANDLER_MODE( cmpa )
{
	u16 t;
	ARGBYTE(t);
	u16 const r = A - t;
	clr_nzc();
	set_nzc8(r);
}

// $a2 SBCA immediate               -***
// $b2 SBCA direct                  -***
// $c2 SBCA extended                -***
// $d2 SBCA indexed, 2 byte offset  -***
// $e2 SBCA indexed, 1 byte offset  -***
// $f2 SBCA indexed                 -***
OP_HANDLER_MODE( sbca )
{
	u16 t;
	ARGBYTE(t);
	u16 const r = A - t - BIT(CC, 0);
	clr_nzc();
	set_nzc8(r);
	A = r;
}

// $a3 CPX immediate                -***
// $b3 CPX direct                   -***
// $c3 CPX extended                 -***
// $d3 CPX indexed, 2 byte offset   -***
// $e3 CPX indexed, 1 byte offset   -***
// $f3 CPX indexed                  -***
OP_HANDLER_MODE( cpx )
{
	u16 t;
	ARGBYTE(t);
	u16 const r = X - t;
	clr_nzc();
	set_nzc8(r);
}

// $a4 ANDA immediate               -**-
// $b4 ANDA direct                  -**-
// $c4 ANDA extended                -**-
// $d4 ANDA indexed, 2 byte offset  -**-
// $e4 ANDA indexed, 1 byte offset  -**-
// $f4 ANDA indexed                 -**-
OP_HANDLER_MODE( anda )
{
	u8 t;
	ARGBYTE(t);
	A &= t;
	clr_nz();
	set_nz8(A);
}

// $a5 BITA immediate               -**-
// $b5 BITA direct                  -**-
// $c5 BITA extended                -**-
// $d5 BITA indexed, 2 byte offset  -**-
// $e5 BITA indexed, 1 byte offset  -**-
// $f5 BITA indexed                 -**-
OP_HANDLER_MODE( bita )
{
	u8 t;
	ARGBYTE(t);
	u8 const r = A & t;
	clr_nz();
	set_nz8(r);
}

// $a6 LDA immediate                -**-
// $b6 LDA direct                   -**-
// $c6 LDA extended                 -**-
// $d6 LDA indexed, 2 byte offset   -**-
// $e6 LDA indexed, 1 byte offset   -**-
// $f6 LDA indexed                  -**-
OP_HANDLER_MODE( lda )
{
	ARGBYTE(A);
	clr_nz();
	set_nz8(A);
}

// $a7 ILLEGAL
// $b7 STA direct                   -**-
// $c7 STA extended                 -**-
// $d7 STA indexed, 2 byte offset   -**-
// $e7 STA indexed, 1 byte offset   -**-
// $f7 STA indexed                  -**-
OP_HANDLER_MODE( sta )
{
	clr_nz();
	set_nz8(A);
	ARGADDR;
	wm(EAD, A);
}

// $a8 EORA immediate               -**-
// $b8 EORA direct                  -**-
// $c8 EORA extended                -**-
// $d8 EORA indexed, 2 byte offset  -**-
// $e8 EORA indexed, 1 byte offset  -**-
// $f8 EORA indexed                 -**-
OP_HANDLER_MODE( eora )
{
	u8 t;
	ARGBYTE(t);
	A ^= t;
	clr_nz();
	set_nz8(A);
}

// $a9 ADCA immediate               ****
// $b9 ADCA direct                  ****
// $c9 ADCA extended                ****
// $d9 ADCA indexed, 2 byte offset  ****
// $e9 ADCA indexed, 1 byte offset  ****
// $f9 ADCA indexed                 ****
OP_HANDLER_MODE( adca )
{
	u16 t;
	ARGBYTE(t);
	u16 const r = A + t + BIT(CC, 0);
	clr_hnzc();
	set_hnzc8(A, t, r);
	A = r;
}

// $aa ORA immediate                -**-
// $ba ORA direct                   -**-
// $ca ORA extended                 -**-
// $da ORA indexed, 2 byte offset   -**-
// $ea ORA indexed, 1 byte offset   -**-
// $fa ORA indexed                  -**-
OP_HANDLER_MODE( ora )
{
	u8 t;
	ARGBYTE(t);
	A |= t;
	clr_nz();
	set_nz8(A);
}

// $ab ADDA immediate               ****
// $bb ADDA direct                  ****
// $cb ADDA extended                ****
// $db ADDA indexed, 2 byte offset  ****
// $eb ADDA indexed, 1 byte offset  ****
// $fb ADDA indexed                 ****
OP_HANDLER_MODE( adda )
{
	u16 t;
	ARGBYTE(t);
	u16 const r = A + t;
	clr_hnzc();
	set_hnzc8(A, t, r);
	A = r;
}

// $ac ILLEGAL
// $bc JMP direct                   -***
// $cc JMP extended                 -***
// $dc JMP indexed, 2 byte offset   -***
// $ec JMP indexed, 1 byte offset   -***
// $fc JMP indexed                  -***
OP_HANDLER_MODE( jmp )
{
	ARGADDR;
	PC = EA;
}

// $ad BSR ----
OP_HANDLER( bsr )
{
	u8 t;
	immbyte(t);
	pushword(m_pc);
	PC += SIGNED(t);
}

// $bd JSR direct                   ----
// $cd JSR extended                 ----
// $dd JSR indexed, 2 byte offset   ----
// $ed JSR indexed, 1 byte offset   ----
// $fd JSR indexed                  ----
OP_HANDLER_MODE( jsr )
{
	ARGADDR;
	pushword(m_pc);
	PC = EA;
}

// $ae LDX immediate                -**-
// $be LDX direct                   -**-
// $ce LDX extended                 -**-
// $de LDX indexed, 2 byte offset   -**-
// $ee LDX indexed, 1 byte offset   -**-
// $fe LDX indexed                  -**-
OP_HANDLER_MODE( ldx )
{
	ARGBYTE(X);
	clr_nz();
	set_nz8(X);
}

// $af ILLEGAL
// $bf STX direct                   -**-
// $cf STX extended                 -**-
// $df STX indexed, 2 byte offset   -**-
// $ef STX indexed, 1 byte offset   -**-
// $ff STX indexed                  -**-
OP_HANDLER_MODE( stx )
{
	clr_nz();
	set_nz8(X);
	ARGADDR;
	wm(EAD, X);
}
