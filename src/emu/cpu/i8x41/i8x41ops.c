
/************************************************************************
 *  Emulate the Instructions
 ************************************************************************/

#define OP_HANDLER( _name )		INLINE void _name (upi41_state_t *upi41_state, UINT8 r)

/***********************************
 *  illegal opcodes
 ***********************************/
OP_HANDLER( illegal )
{
	logerror("i8x41 #%d: illegal opcode at 0x%03x: %02x\n", cpunum_get_active(), PC, ROP(upi41_state, PC));
}

/***********************************
 *  0110 1rrr *  ADD     A,Rr
 ***********************************/
OP_HANDLER( add_r )
{
	UINT8 res = A + GETR(upi41_state, r);
	if( res < A ) PSW |= FC;
	if( (res & 0x0f) < (A & 0x0f) ) PSW |= FA;
	A = res;
}

/***********************************
 *  0110 000r
 *  ADD     A,@Rr
 ***********************************/
OP_HANDLER( add_rm )
{
	UINT8 res = A + IRAM_R(upi41_state, GETR(upi41_state, r));
	if( res < A ) PSW |= FC;
	if( (res & 0x0f) < (A & 0x0f) ) PSW |= FA;
	A = res;
}

/***********************************
 *  0000 0011 7654 3210
 *  ADD     A,#n
 ***********************************/
OP_HANDLER( add_i )
{
	UINT8 res = A + ROP_ARG(upi41_state, PC);
	PC++;
	if( res < A ) PSW |= FC;
	if( (res & 0x0f) < (A & 0x0f) ) PSW |= FA;
	A = res;
}

/***********************************
 *  0111 1rrr
 *  ADDC    A,Rr
 ***********************************/
OP_HANDLER( addc_r )
{
	UINT8 res = A + GETR(upi41_state, r) + (PSW >> 7);
	if( res <= A ) PSW |= FC;
	if( (res & 0x0f) <= (A & 0x0f) ) PSW |= FA;
	A = res;
}

/***********************************
 *  0111 000r
 *  ADDC    A,@Rr
 ***********************************/
OP_HANDLER( addc_rm )
{
	UINT8 res = A + IRAM_R(upi41_state, GETR(upi41_state, r)) + (PSW >> 7);
	if( res <= A ) PSW |= FC;
	if( (res & 0x0f) <= (A & 0x0f) ) PSW |= FA;
	A = res;
}

/***********************************
 *  0001 0011 7654 3210
 *  ADDC    A,#n
 ***********************************/
OP_HANDLER( addc_i )
{
	UINT8 res = A + ROP_ARG(upi41_state, PC) + (PSW >> 7);
	PC++;
	if( res <= A ) PSW |= FC;
	if( (res & 0x0f) < (A & 0x0f) ) PSW |= FA;
	A = res;
}

/***********************************
 *  0101 1rrr
 *  ANL     A,Rr
 ***********************************/
OP_HANDLER( anl_r )
{
	A = A & GETR(upi41_state, r);
}

/***********************************
 *  0101 000r
 *  ANL     A,@Rr
 ***********************************/
OP_HANDLER( anl_rm )
{
	A = A & IRAM_R(upi41_state, GETR(upi41_state, r));
}

/***********************************
 *  0101 0011 7654 3210
 *  ANL     A,#n
 ***********************************/
OP_HANDLER( anl_i )
{
	A = A & ROP_ARG(upi41_state, PC);
	PC++;
}

/***********************************
 *  1001 10pp 7654 3210
 *  ANL     Pp,#n
 ***********************************/
OP_HANDLER( anl_p_i )
{
	UINT8 p = r;
	UINT8 val = ROP_ARG(upi41_state, PC);
	PC++;
	/* changed to latched port scheme */
	switch (p)
	{
		case 00:	break;	/* invalid port */
		case 01:	P1 &= val; WP(upi41_state, p, P1); break;
		case 02:	P2 &= val; WP(upi41_state, p, (P2 & P2_HS) ); break;
		case 03:	break;	/* invalid port */
		default:	break;
	}
}

/***********************************
 *  1001 11pp 7654 3210
 *  ANLD    Pp,A
 ***********************************/
OP_HANDLER( anld_p_a )
{
	UINT8 p = r;
	/* added proper expanded port setup */
	WP(upi41_state, 2, (P2 & 0xf0) | 0x0c | p); /* AND mode */
	WP(upi41_state, I8X41_ps, 0);	/* activate command strobe */
	WP(upi41_state, 2, (A & 0x0f)); 	/* Expander to take care of AND function */
	WP(upi41_state, I8X41_ps, 1);	/* release command strobe */
}

/***********************************
 *  aaa1 0100 7654 3210
 *  CALL    addr
 ***********************************/
OP_HANDLER( call_i )
{
	UINT16 page = (r & 0xe0) << 3;
	UINT8 adr = ROP_ARG(upi41_state, PC);
	PC++;
	PUSH_PC_TO_STACK();
	PC = page | adr;
}

/***********************************
 *  0010 0111
 *  CLR     A
 ***********************************/
OP_HANDLER( clr_a )
{
	A = 0;
}

/***********************************
 *  1001 0111
 *  CLR     C
 ***********************************/
OP_HANDLER( clr_c )
{
	PSW &= ~FC;
}

/***********************************
 *  1000 0101
 *  CLR     F0
 ***********************************/
OP_HANDLER( clr_f0 )
{
	PSW &= ~Ff0;
	STATE &= ~F0;
}

/***********************************
 *  1010 0101
 *  CLR     F1
 ***********************************/
OP_HANDLER( clr_f1 )
{
	STATE &= ~F1;
}

/***********************************
 *  0011 0111
 *  CPL     A
 ***********************************/
OP_HANDLER( cpl_a )
{
	A = ~A;
}

/***********************************
 *  1010 0111
 *  CPL     C
 ***********************************/
OP_HANDLER( cpl_c )
{
	PSW ^= FC;
}

/***********************************
 *  1001 0101
 *  CPL     F0
 ***********************************/
OP_HANDLER( cpl_f0 )
{
	PSW ^= Ff0;
	STATE ^= F0;
}

/***********************************
 *  1011 0101
 *  CPL     F1
 ***********************************/
OP_HANDLER( cpl_f1 )
{
	STATE ^= F1;
}

/***********************************
 *  0101 0111
 *  DA      A
 ***********************************/
OP_HANDLER( da_a )
{
	UINT8 res = A + ((PSW & FA) || ((A & 0x0f) > 0x09)) ? 0x06 : 0x00;
	if( (PSW & FC) || ((res & 0xf0) > 0x90) )
		res += 0x60;
	if( res < A )
		PSW |= FC;
	else
		PSW &= ~FC;
	A = res;
}

/***********************************
 *  0000 0111
 *  DEC     A
 ***********************************/
OP_HANDLER( dec_a )
{
	A -= 1;
}

/***********************************
 *  1100 1rrr
 *  DEC     Rr
 ***********************************/
OP_HANDLER( dec_r )
{
	SETR(upi41_state, r, GETR(upi41_state, r) - 1);
}

/***********************************
 *  0001 0101
 *  DIS     I
 ***********************************/
OP_HANDLER( dis_i )
{
	ENABLE &= ~IBFI;	/* disable input buffer full interrupt */
}

/***********************************
 *  0011 0101
 *  DIS     TCNTI
 ***********************************/
OP_HANDLER( dis_tcnti )
{
	ENABLE &= ~TCNTI;	/* disable timer/counter interrupt */
}

/***********************************
 *  0111 1rrr 7654 3210
 *  DJNZ    Rr,addr
 ***********************************/
OP_HANDLER( djnz_r_i )
{
	UINT8 adr = ROP_ARG(upi41_state, PC);
	PC++;
	SETR(upi41_state, r, GETR(upi41_state, r) - 1);
	if( GETR(upi41_state, r) )
		PC = (PC & 0x700) | adr;
}

/***********************************
 *  1110 0101
 *  EN      DMA
 ***********************************/
OP_HANDLER( en_dma )
{
	ENABLE |= DMA;		/* enable DMA handshake lines */
	P2_HS &= 0xbf;
	WP(upi41_state, 0x02, (P2 & P2_HS) );
}

/***********************************
 *  1111 0101
 *  EN      FLAGS
 ***********************************/
OP_HANDLER( en_flags )
{
	if( 0 == (ENABLE & FLAGS) )
	{
		/* Configure upper lines on Port 2 for IRQ handshaking (P24 and P25) */

		ENABLE |= FLAGS;
		if( STATE & OBF ) P2_HS |= 0x10;
		else P2_HS &= 0xef;
		if( STATE & IBF ) P2_HS |= 0x20;
		else P2_HS &= 0xdf;
		WP(upi41_state, 0x02, (P2 & P2_HS) );
	}
}

/***********************************
 *  0000 0101
 *  EN      I
 ***********************************/
OP_HANDLER( en_i )
{
	if( 0 == (ENABLE & IBFI) )
	{
		ENABLE |= IBFI;		/* enable input buffer full interrupt */
		CONTROL &= ~IBFI_IGNR;
		if( STATE & IBF )	/* already got data in the buffer? */
			set_irq_line(upi41_state, I8X41_INT_IBF, HOLD_LINE);
	}
}

/***********************************
 *  0010 0101
 *  EN      TCNTI
 ***********************************/
OP_HANDLER( en_tcnti )
{
	ENABLE |= TCNTI;	/* enable timer/counter interrupt */
	CONTROL &= ~TIRQ_IGNR;
}

/***********************************
 *  0010 0010
 *  IN      A,DBB
 ***********************************/
OP_HANDLER( in_a_dbb )
{
	if( upi41_state->irq_callback )
		(upi41_state->irq_callback)(upi41_state->device, I8X41_INT_IBF);

	STATE &= ~IBF;					/* clear input buffer full flag */
	if( ENABLE & FLAGS )
	{
		P2_HS &= 0xdf;
		if( STATE & OBF ) P2_HS |= 0x10;
		else P2_HS &= 0xef;
		WP(upi41_state, 0x02, (P2 & P2_HS) );	/* Clear the DBBI IRQ out on P25 */
	}
	A = DBBI;
}

/***********************************
 *  0000 10pp
 *  IN      A,Pp
 ***********************************/
OP_HANDLER( in_a_p )
{
	UINT8 p = r;
	/* changed to latched port scheme */
	switch( p )
	{
		case 00:	break;	/* invalid port */
		case 01:	A = (RP(upi41_state, p) & P1); break;
		case 02:	A = (RP(upi41_state, p) & P2); break;
		case 03:	break;	/* invalid port */
		default:	break;
	}
}

/***********************************
 *  0001 0111
 *  INC     A
 ***********************************/
OP_HANDLER( inc_a )
{
	A += 1;
}

/***********************************
 *  0001 1rrr
 *  INC     Rr
 ***********************************/
OP_HANDLER( inc_r )
{
	SETR(upi41_state, r, GETR(upi41_state, r) + 1);
}

/***********************************
 *  0001 000r
 *  INC  @  Rr
 ***********************************/
OP_HANDLER( inc_rm )
{
	UINT16 addr = GETR(upi41_state, r);
	IRAM_W(upi41_state,  addr, IRAM_R(upi41_state, addr) + 1 );
}

/***********************************
 *  bbb1 0010
 *  JBb     addr
 ***********************************/
OP_HANDLER( jbb_i )
{
	UINT8 bit = r;
	UINT8 adr = ROP_ARG(upi41_state, PC);
	PC += 1;
	if( A & (1 << bit) )
		PC = (PC & 0x700) | adr;
}

/***********************************
 *  1111 0110
 *  JC      addr
 ***********************************/
OP_HANDLER( jc_i )
{
	UINT8 adr = ROP_ARG(upi41_state, PC);
	PC += 1;
	if( PSW & FC )
		PC = (PC & 0x700) | adr;
}

/***********************************
 *  1011 0110
 *  JF0     addr
 ***********************************/
OP_HANDLER( jf0_i )
{
	UINT8 adr = ROP_ARG(upi41_state, PC);
	PC += 1;
	if( STATE & F0 )
		PC = (PC & 0x700) | adr;
}

/***********************************
 *  0111 0110
 *  JF1     addr
 ***********************************/
OP_HANDLER( jf1_i )
{
	UINT8 adr = ROP_ARG(upi41_state, PC);
	PC += 1;
	if( STATE & F1 )
		PC = (PC & 0x700) | adr;
}

/***********************************
 *  aaa0 0100
 *  JMP     addr
 ***********************************/
OP_HANDLER( jmp_i )
{
	/* err.. do we have 10 or 11 PC bits?
     * CALL is said to use 0aa1 (4 pages)
     * JMP is said to use aaa0 (8 pages)
     */
	UINT16 page = ((r & 0xe0) << 3);
	UINT8 adr = ROP_ARG(upi41_state, PC);
	PC = page | adr;
}

/***********************************
 *  1011 0011
 *  JMP  @  A
 ***********************************/
OP_HANDLER( jmpp_a )
{
	UINT16 adr = (PC & 0x700) | A;
	PC = (PC & 0x700) | RM(upi41_state, adr);
}

/***********************************
 *  1110 0110
 *  JNC     addr
 ***********************************/
OP_HANDLER( jnc_i )
{
	UINT8 adr = ROP_ARG(upi41_state, PC);
	PC += 1;
	if( !(PSW & FC) )
		PC = (PC & 0x700) | adr;
}

/***********************************
 *  1101 0110
 *  JNIBF   addr
 ***********************************/
OP_HANDLER( jnibf_i )
{
	UINT8 adr = ROP_ARG(upi41_state, PC);
	PC += 1;
	if( 0 == (STATE & IBF) )
		PC = (PC & 0x700) | adr;
}

/***********************************
 *  0010 0110
 *  JNT0    addr
 ***********************************/
OP_HANDLER( jnt0_i )
{
	UINT8 adr = ROP_ARG(upi41_state, PC);
	PC += 1;
	if( 0 == RP(upi41_state, I8X41_t0) )
		PC = (PC & 0x700) | adr;
}

/***********************************
 *  0100 0110
 *  JNT1    addr
 ***********************************/
OP_HANDLER( jnt1_i )
{
	UINT8 adr = ROP_ARG(upi41_state, PC);
	PC += 1;
	if( !(ENABLE & CNT) )
	{
		UINT8 level = RP(upi41_state, I8X41_t1);
		if( level ) CONTROL |= TEST1;
		else CONTROL &= ~TEST1;
	}
	if( !(CONTROL & TEST1) )
		PC = (PC & 0x700) | adr;
}

/***********************************
 *  1001 0110
 *  JNZ     addr
 ***********************************/
OP_HANDLER( jnz_i )
{
	UINT8 adr = ROP_ARG(upi41_state, PC);
	PC += 1;
	if( A )
		PC = (PC & 0x700) | adr;
}

/***********************************
 *  1000 0110
 *  JOBF    addr
 ***********************************/
OP_HANDLER( jobf_i )
{
	UINT8 adr = ROP_ARG(upi41_state, PC);
	PC += 1;
	if( STATE & OBF )
		PC = (PC & 0x700) | adr;
}

/***********************************
 *  0001 0110
 *  JTF     addr
 ***********************************/
OP_HANDLER( jtf_i )
{
	UINT8 adr = ROP_ARG(upi41_state, PC);
	PC += 1;
	if( CONTROL & TOVF )
		PC = (PC & 0x700) | adr;
	CONTROL &= ~TOVF;
}

/***********************************
 *  0011 0110
 *  JT0     addr
 ***********************************/
OP_HANDLER( jt0_i )
{
	UINT8 adr = ROP_ARG(upi41_state, PC);
	PC += 1;
	if( RP(upi41_state, I8X41_t0) )
		PC = (PC & 0x700) | adr;
}

/***********************************
 *  0101 0110
 *  JT1     addr
 ***********************************/
OP_HANDLER( jt1_i )
{
	UINT8 adr = ROP_ARG(upi41_state, PC);
	PC += 1;
	if( !(ENABLE & CNT) )
	{
		UINT8 level = RP(upi41_state, I8X41_t1);
		if( level ) CONTROL |= TEST1;
		else CONTROL &= ~TEST1;
	}
	if( (CONTROL & TEST1) )
		PC = (PC & 0x700) | adr;
}

/***********************************
 *  1100 0110
 *  JZ      addr
 ***********************************/
OP_HANDLER( jz_i )
{
	UINT8 adr = ROP_ARG(upi41_state, PC);
	PC += 1;
	if( !A )
		PC = (PC & 0x700) | adr;
}

/***********************************
 *  0010 0011
 *  MOV     A,#n
 ***********************************/
OP_HANDLER( mov_a_i )
{
	A = ROP(upi41_state, PC);
	PC += 1;
}

/***********************************
 *  1100 0111
 *  MOV     A,PSW
 ***********************************/
OP_HANDLER( mov_a_psw )
{
	A = PSW;
}

/***********************************
 *  1111 1rrr
 *  MOV     A,Rr
 ***********************************/
OP_HANDLER( mov_a_r )
{
	A = GETR(upi41_state, r);
}

/***********************************
 *  1111 000r
 *  MOV     A,Rr
 ***********************************/
OP_HANDLER( mov_a_rm )
{
	A = IRAM_R(upi41_state, GETR(upi41_state, r));
}

/***********************************
 *  0100 0010
 *  MOV     A,T
 ***********************************/
OP_HANDLER( mov_a_t )
{
	A = upi41_state->timer;
}

/***********************************
 *  1101 0111
 *  MOV     PSW,A
 ***********************************/
OP_HANDLER( mov_psw_a )
{
	PSW = A;
}

/***********************************
 *  1010 1rrr
 *  MOV     Rr,A
 ***********************************/
OP_HANDLER( mov_r_a )
{
	SETR(upi41_state, r, A);
}

/***********************************
 *  1011 1rrr
 *  MOV     Rr,#n
 ***********************************/
OP_HANDLER( mov_r_i )
{
	UINT8 val = ROP_ARG(upi41_state, PC);
	PC += 1;
	SETR(upi41_state, r, val);
}

/***********************************
 *  1010 000r
 *  MOV     @Rr,A
 ***********************************/
OP_HANDLER( mov_rm_a )
{
	IRAM_W(upi41_state, GETR(upi41_state, r), A );
}

/***********************************
 *  1011 000r
 *  MOV     @Rr,#n
 ***********************************/
OP_HANDLER( mov_rm_i )
{
	UINT8 val = ROP_ARG(upi41_state, PC);
	PC += 1;
	IRAM_W(upi41_state, GETR(upi41_state, r), val );
}

/***********************************
 *  1001 0000
 *  MOV     STS,A
 ***********************************/
OP_HANDLER( mov_sts_a )
{
	STATE = (STATE & 0x0f) | (A & 0xf0);
}

/***********************************
 *  0110 0010
 *  MOV     T,A
 ***********************************/
OP_HANDLER( mov_t_a )
{
	upi41_state->timer = A;
}

/***********************************
 *  0000 11pp
 *  MOVD    A,Pp
 ***********************************/
OP_HANDLER( movd_a_p )
{
	UINT8 p = r;
	/* added proper expanded port setup */
	WP(upi41_state, 2, (P2 & 0xf0) | 0x00 | p);	/* READ mode */
	WP(upi41_state, I8X41_ps, 0);		/* activate command strobe */
	A = RP(upi41_state, 2) & 0xf;
	WP(upi41_state, I8X41_ps, 1);		/* release command strobe */
}

/***********************************
 *  0011 11pp
 *  MOVD    Pp,A
 ***********************************/
OP_HANDLER( movd_p_a )
{
	UINT8 p = r;
	/* added proper expanded port setup */
	WP(upi41_state, 2, (P2 & 0xf0) | 0x04 | p);	/* WRITE mode */
	WP(upi41_state, I8X41_ps, 0);		/* activate command strobe */
	WP(upi41_state, 2, A & 0x0f);
	WP(upi41_state, I8X41_ps, 1);		/* release command strobe */
}

/***********************************
 *  1010 0011
 *  MOVP    A,@A
 ***********************************/
OP_HANDLER( movp_a_am )
{
	UINT16 addr = (PC & 0x700) | A;
	A = RM(upi41_state, addr);
}

/***********************************
 *  1110 0011
 *  MOVP3   A,@A
 ***********************************/
OP_HANDLER( movp3_a_am )
{
	UINT16 addr = 0x300 | A;
	A = RM(upi41_state, addr);
}

/***********************************
 *  0000 0000
 *  NOP
 ***********************************/
OP_HANDLER( nop )
{
}

/***********************************
 *  0100 1rrr
 *  ORL     A,Rr
 ***********************************/
OP_HANDLER( orl_r )
{
	A = A | GETR(upi41_state, r);
}

/***********************************
 *  0100 000r
 *  ORL     A,@Rr
 ***********************************/
OP_HANDLER( orl_rm )
{
	A = A | IRAM_R(upi41_state, GETR(upi41_state, r));
}

/***********************************
 *  0100 0011 7654 3210
 *  ORL     A,#n
 ***********************************/
OP_HANDLER( orl_i )
{
	UINT8 val = ROP_ARG(upi41_state, PC);
	PC++;
	A = A | val;
}

/***********************************
 *  1000 10pp 7654 3210
 *  ORL     Pp,#n
 ***********************************/
OP_HANDLER( orl_p_i )
{
	UINT8 p = r;
	UINT8 val = ROP_ARG(upi41_state, PC);
	PC++;
	/* changed to latched port scheme */
	switch (p)
	{
		case 00:	break;	/* invalid port */
		case 01:	P1 |= val; WP(upi41_state, p, P1); break;
		case 02:	P2 |= val; WP(upi41_state, p, P2); break;
		case 03:	break;	/* invalid port */
		default:	break;
	}
}

/***********************************
 *  1000 11pp 7654 3210
 *  ORLD    Pp,A
 ***********************************/
OP_HANDLER( orld_p_a )
{
	UINT8 p = r;
	/* added proper expanded port setup */
	WP(upi41_state, 2, (P2 & 0xf0) | 0x08 | p);	/* OR mode */
	WP(upi41_state, I8X41_ps, 0);	/* activate command strobe */
	WP(upi41_state, 2, A & 0x0f);	/* Expander to take care of OR function */
	WP(upi41_state, I8X41_ps, 1);	/* release command strobe */
}

/***********************************
 *  0000 0010
 *  OUT     DBB,A
 ***********************************/
OP_HANDLER( out_dbb_a )
{
	DBBO = A;			/* DBB output buffer */
	STATE |= OBF;		/* assert the output buffer full flag */
	if( ENABLE & FLAGS )
	{
		P2_HS |= 0x10;
		if( STATE & IBF ) P2_HS |= 0x20;
		else P2_HS &= 0xdf;
		WP(upi41_state, 0x02, (P2 & P2_HS) );	/* Assert the DBBO IRQ out on P24 */
	}
}

/***********************************
 *  0011 10pp
 *  OUT     Pp,A
 ***********************************/
OP_HANDLER( out_p_a )
{
	UINT8 p = r;
	/* changed to latched port scheme */
	switch (p)
	{
		case 00:	break;	/* invalid port */
		case 01:	WP(upi41_state, p, A); P1 = A; break;
		case 02:	WP(upi41_state, p, A); P2 = A; break;
		case 03:	break;	/* invalid port */
		default:	break;
	}
}

/***********************************
 *  1000 0011
 *  RET
 ***********************************/
OP_HANDLER( ret )
{
	UINT8 msb;
	PSW = (PSW & ~SP) | ((PSW - 1) & SP);
	msb = IRAM_R(upi41_state, M_STACK + (PSW&SP) * 2 + 1);
	PC = IRAM_R(upi41_state, M_STACK + (PSW&SP) * 2 + 0);
	PC |= (msb << 8) & 0x700;
}

/***********************************
 *  1001 0011
 *  RETR
 ***********************************/
OP_HANDLER( retr )
{
	UINT8 msb;
	PSW = (PSW & ~SP) | ((PSW - 1) & SP);
	msb = IRAM_R(upi41_state, M_STACK + (PSW&SP) * 2 + 1);
	PC = IRAM_R(upi41_state, M_STACK + (PSW&SP) * 2 + 0);
	PC |= (msb << 8) & 0x700;
	PSW = (PSW & 0x0f) | (msb & 0xf0);
	CONTROL &= ~IRQ_IGNR;
}

/***********************************
 *  1110 0111
 *  RL      A
 ***********************************/
OP_HANDLER( rl_a )
{
	A = (A << 1) | (A >> 7);
}

/***********************************
 *  1111 0111
 *  RLC     A
 ***********************************/
OP_HANDLER( rlc_a )
{
	UINT8 c = PSW >> 7;
	PSW = (PSW & ~FC) | (A & FC);
	A = (A << 1) | c;
}

/***********************************
 *  0111 0111
 *  RR      A
 ***********************************/
OP_HANDLER( rr_a )
{
	A = (A >> 1) | (A << 7);
}

/***********************************
 *  0110 0111
 *  RRC     A
 ***********************************/
OP_HANDLER( rrc_a )
{
	UINT8 c = PSW & 0x80;
	PSW = (PSW & ~FC) | (A << 7);
	A = (A >> 1) | c;
}

/***********************************
 *  1100 0101
 *  SEL     RB0
 ***********************************/
OP_HANDLER( sel_rb0 )
{
	PSW &= ~BS;
}

/***********************************
 *  1101 0101
 *  SEL     RB1
 ***********************************/
OP_HANDLER( sel_rb1 )
{
	PSW |= BS;
}

/***********************************
 *  0110 0101
 *  STOP    TCNT
 ***********************************/
OP_HANDLER( stop_tcnt )
{
	ENABLE &= ~(T|CNT);
}

/***********************************
 *  0100 0101
 *  STRT    CNT
 ***********************************/
OP_HANDLER( strt_cnt )
{
	ENABLE |= CNT;
	ENABLE &= ~T;
}

/***********************************
 *  0101 0101
 *  STRT    T
 ***********************************/
OP_HANDLER( strt_t )
{
	ENABLE |= T;
	ENABLE &= ~CNT;
}

/***********************************
 *  0100 0111
 *  SWAP    A
 ***********************************/
OP_HANDLER( swap_a )
{
	A = (A << 4) | (A >> 4);
}

/***********************************
 *  0010 1rrr
 *  XCH     A,Rr
 ***********************************/
OP_HANDLER( xch_a_r )
{
	UINT8 tmp = GETR(upi41_state, r);
	SETR(upi41_state, r, A);
	A = tmp;
}

/***********************************
 *  0010 000r
 *  XCH     A,@Rr
 ***********************************/
OP_HANDLER( xch_a_rm )
{
	UINT16 addr = GETR(upi41_state, r);
	UINT8 tmp = IRAM_R(upi41_state, addr);
	IRAM_W(upi41_state,  addr, A );
	A = tmp;
}

/***********************************
 *  0011 000r
 *  XCHD    A,@Rr
 ***********************************/
OP_HANDLER( xchd_a_rm )
{
	UINT16 addr = GETR(upi41_state, r);
	UINT8 tmp = IRAM_R(upi41_state, addr);
	IRAM_W(upi41_state,  addr, (tmp & 0xf0) | (A & 0x0f) );
	A = (A & 0xf0) | (tmp & 0x0f);
}

/***********************************
 *  1101 1rrr
 *  XRL     A,Rr
 ***********************************/
OP_HANDLER( xrl_r )
{
	A = A ^ GETR(upi41_state, r);
}

/***********************************
 *  1101 000r
 *  XRL     A,@Rr
 ***********************************/
OP_HANDLER( xrl_rm )
{
	A = A ^ IRAM_R(upi41_state, GETR(upi41_state, r));
}

/***********************************
 *  1101 0011 7654 3210
 *  XRL     A,#n
 ***********************************/
OP_HANDLER( xrl_i )
{
	UINT8 val = ROP_ARG(upi41_state, PC);
	PC++;
	A = A ^ val;
}

