/*****************************************************************************
 *
 *   scops.c
 *   portable sharp 61860 emulator interface
 *   (sharp pocket computers)
 *
 *   Copyright Peter Trauner, all rights reserved.
 *
 *   - This source code is released as freeware for non-commercial purposes.
 *   - You are free to use and redistribute this code in modified or
 *     unmodified form, provided you list me in the credits.
 *   - If you modify this source code, you must add a notice to each modified
 *     source file that it has been changed.  If you're a nice person, you
 *     will clearly mark each change too.  :)
 *   - If you wish to use this for commercial purposes, please contact me at
 *     peter.trauner@jk.uni-linz.ac.at
 *   - The author of this copywritten work reserves the right to change the
 *     terms of its usage and license at any time, including retroactively
 *   - This entire notice must remain in the source code.
 *
 * History of changes:
 * 21.07.2001 Several changes listed below were made by Mario Konegger
 *            (konegger@itp.tu-graz.ac.at)
 *        replaced buggy BCD-commands add_bcd, sub_bcd, add_bcd_a,
 *            sub_bcd_a and changed out_c, to implement HLT-mode of the CPU.
 *
 *****************************************************************************/

INLINE UINT8 READ_OP(sc61860_state *cpustate)
{
	return cpustate->direct->read_decrypted_byte(cpustate->pc++);
}

INLINE UINT8 READ_OP_ARG(sc61860_state *cpustate)
{
	return cpustate->direct->read_raw_byte(cpustate->pc++);
}

INLINE UINT16 READ_OP_ARG_WORD(sc61860_state *cpustate)
{
	UINT16 t=cpustate->direct->read_decrypted_byte(cpustate->pc++)<<8;
	t|=cpustate->direct->read_decrypted_byte(cpustate->pc++);
	return t;
}

INLINE UINT8 READ_BYTE(sc61860_state *cpustate, UINT16 adr)
{
	return cpustate->program->read_byte(adr);
}

INLINE void WRITE_BYTE(sc61860_state *cpustate, UINT16 a, UINT8 v)
{
	cpustate->program->write_byte(a, v);
}

INLINE UINT8 READ_RAM(sc61860_state *cpustate, int r)
{
	return cpustate->ram[r];
}

INLINE void WRITE_RAM(sc61860_state *cpustate, int r, UINT8 v)
{
	cpustate->ram[r] = v;
}

INLINE void PUSH(sc61860_state *cpustate, UINT8 v)
{
	cpustate->r--;
	WRITE_RAM(cpustate, cpustate->r, v);
}

INLINE UINT8 POP(sc61860_state *cpustate)
{
	UINT8 t = READ_RAM(cpustate, cpustate->r);
	cpustate->r++;
	return t;
}

INLINE void sc61860_load_imm(sc61860_state *cpustate, int r, UINT8 v)
{
	WRITE_RAM(cpustate, r, v);
}

INLINE void sc61860_load(sc61860_state *cpustate)
{
	WRITE_RAM(cpustate, A, READ_RAM(cpustate, cpustate->p));
}

INLINE void sc61860_load_imm_p(sc61860_state *cpustate, UINT8 v)
{
	cpustate->p=v&0x7f;
}

INLINE void sc61860_load_imm_q(sc61860_state *cpustate, UINT8 v)
{
	cpustate->q=v&0x7f;
}

INLINE void sc61860_load_r(sc61860_state *cpustate)
{
	cpustate->r = READ_RAM(cpustate, A) & 0x7f;
}

INLINE void sc61860_load_ext(sc61860_state *cpustate, int r)
{
	WRITE_RAM(cpustate, r, READ_BYTE(cpustate, cpustate->dp));
}

INLINE void sc61860_load_dp(sc61860_state *cpustate)
{
	cpustate->dp=READ_OP_ARG_WORD(cpustate);
}

INLINE void sc61860_load_dl(sc61860_state *cpustate)
{
	cpustate->dp=(cpustate->dp&~0xff)|READ_OP_ARG(cpustate);
}

INLINE void sc61860_store_p(sc61860_state *cpustate)
{
	WRITE_RAM(cpustate, A, cpustate->p);
}

INLINE void sc61860_store_q(sc61860_state *cpustate)
{
	WRITE_RAM(cpustate, A, cpustate->q);
}

INLINE void sc61860_store_r(sc61860_state *cpustate)
{
	WRITE_RAM(cpustate, A, cpustate->r);
}

INLINE void sc61860_store_ext(sc61860_state *cpustate, int r)
{
	WRITE_BYTE(cpustate, cpustate->dp, READ_RAM(cpustate, r));
}

INLINE void sc61860_exam(sc61860_state *cpustate, int a, int b)
{
	UINT8 t = READ_RAM(cpustate, a);
	WRITE_RAM(cpustate, a, READ_RAM(cpustate, b));
	WRITE_RAM(cpustate, b, t);
}

INLINE void sc61860_test(sc61860_state *cpustate, int reg, UINT8 value)
{
	cpustate->zero=(READ_RAM(cpustate, reg) & value)==0;
}

INLINE void sc61860_test_ext(sc61860_state *cpustate)
{
	cpustate->zero=(READ_BYTE(cpustate, cpustate->dp)&READ_OP_ARG(cpustate))==0;
}

INLINE void sc61860_and(sc61860_state *cpustate, int reg, UINT8 value)
{
	UINT8 t = READ_RAM(cpustate, reg) & value;
	WRITE_RAM(cpustate, reg,  t);
	cpustate->zero=t==0;
}

INLINE void sc61860_and_ext(sc61860_state *cpustate)
{
	UINT8 t = READ_BYTE(cpustate, cpustate->dp) & READ_OP_ARG(cpustate);
	cpustate->zero=t==0;
	WRITE_BYTE(cpustate, cpustate->dp, t);
}

INLINE void sc61860_or(sc61860_state *cpustate, int reg, UINT8 value)
{
	UINT8 t = READ_RAM(cpustate, reg) | value;
	WRITE_RAM(cpustate, reg, t);
	cpustate->zero=t==0;
}

INLINE void sc61860_or_ext(sc61860_state *cpustate)
{
	UINT8 t=READ_BYTE(cpustate, cpustate->dp)|READ_OP_ARG(cpustate);
	cpustate->zero=t==0;
	WRITE_BYTE(cpustate, cpustate->dp, t);
}

INLINE void sc61860_rotate_right(sc61860_state *cpustate)
{
	int t = READ_RAM(cpustate, A);
	if (cpustate->carry) t|=0x100;
	cpustate->carry=t&1;
	WRITE_RAM(cpustate, A, t>>1);
}

INLINE void sc61860_rotate_left(sc61860_state *cpustate)
{
	int t = READ_RAM(cpustate, A) << 1;
	if (cpustate->carry) t|=1;
	cpustate->carry=t&0x100;
	WRITE_RAM(cpustate, A, t);
}

INLINE void sc61860_swap(sc61860_state *cpustate)
{
	int t = READ_RAM(cpustate, A);
	WRITE_RAM(cpustate, A, (t<<4)|((t>>4)&0xf));
}

// q=reg sideeffect
INLINE void sc61860_inc(sc61860_state *cpustate, int reg)
{
	UINT8 t = READ_RAM(cpustate, reg) + 1;
	cpustate->q=reg;
	WRITE_RAM(cpustate, reg, t);
	cpustate->zero=t==0;
	cpustate->carry=t==0;
}

INLINE void sc61860_inc_p(sc61860_state *cpustate)
{
	cpustate->p++;
}

// q=reg sideeffect
INLINE void sc61860_dec(sc61860_state *cpustate, int reg)
{
	UINT8 t = READ_RAM(cpustate, reg) - 1;
	cpustate->q=reg;
	WRITE_RAM(cpustate, reg, t);
	cpustate->zero=t==0;
	cpustate->carry=t==0xff;
}

INLINE void sc61860_dec_p(sc61860_state *cpustate)
{
	cpustate->p--;
}

INLINE void sc61860_add(sc61860_state *cpustate, int reg, UINT8 value)
{
	int t = READ_RAM(cpustate, reg) + value;
	WRITE_RAM(cpustate, reg, t);
	cpustate->zero=(t&0xff)==0;
	cpustate->carry=t>=0x100;
}

INLINE void sc61860_add_carry(sc61860_state *cpustate)
{
	int t = READ_RAM(cpustate, cpustate->p) + READ_RAM(cpustate, A);
	if (cpustate->carry) t++;
	WRITE_RAM(cpustate, cpustate->p, t);
	cpustate->zero=(t&0xff)==0;
	cpustate->carry=t>=0x100;
}

// p++ sideeffect
INLINE void sc61860_add_word(sc61860_state *cpustate)
{
	int t = READ_RAM(cpustate, cpustate->p) + READ_RAM(cpustate, A), t2;
	WRITE_RAM(cpustate, cpustate->p, t);
	cpustate->p++;
	t2 = READ_RAM(cpustate, cpustate->p) + READ_RAM(cpustate, B);
	if (t>=0x100) t2++;
	WRITE_RAM(cpustate, cpustate->p, t2);
	cpustate->zero=(t2&0xff)==0 &&(t&0xff)==0;
	cpustate->carry=t2>=0x100;
}


INLINE void sc61860_sub(sc61860_state *cpustate, int reg, UINT8 value)
{
	int t = READ_RAM(cpustate, reg) - value;
	WRITE_RAM(cpustate, reg, t);
	cpustate->zero=(t&0xff)==0;
	cpustate->carry=t<0;
}

INLINE void sc61860_sub_carry(sc61860_state *cpustate)
{
	int t = READ_RAM(cpustate, cpustate->p) - READ_RAM(cpustate, A);
	if (cpustate->carry) t--;
	WRITE_RAM(cpustate, cpustate->p, t);
	cpustate->zero=(t&0xff)==0;
	cpustate->carry=t<0;
}


// p++ sideeffect
INLINE void sc61860_sub_word(sc61860_state *cpustate)
{
	int t = READ_RAM(cpustate, cpustate->p) - READ_RAM(cpustate, A), t2;
	WRITE_RAM(cpustate, cpustate->p, t);
	cpustate->p++;
	t2 = READ_RAM(cpustate, cpustate->p) - READ_RAM(cpustate, B);
	if (t<0) t2--;
	WRITE_RAM(cpustate, cpustate->p, t2);
	cpustate->zero=(t2&0xff)==0 && (t&0xff)==0;
	cpustate->carry=t2<0;
}

INLINE void sc61860_cmp(sc61860_state *cpustate, int reg, UINT8 value)
{
	int t = READ_RAM(cpustate, reg) - value;
	cpustate->zero=t==0;
	cpustate->carry=t<0;
}

INLINE void sc61860_pop(sc61860_state *cpustate)
{
	WRITE_RAM(cpustate, A, POP(cpustate));
}

INLINE void sc61860_push(sc61860_state *cpustate)
{
	PUSH(cpustate, READ_RAM(cpustate, A));
}

INLINE void sc61860_prepare_table_call(sc61860_state *cpustate)
{
	int adr;
	cpustate->h=READ_OP(cpustate);
	adr=READ_OP_ARG_WORD(cpustate);
	PUSH(cpustate, adr>>8);
	PUSH(cpustate, adr&0xff);
}

INLINE void sc61860_execute_table_call(sc61860_state *cpustate)
{
	int i, v, adr;
	for (i=0; i<cpustate->h; i++) {
		v=READ_OP(cpustate);
		adr=READ_OP_ARG_WORD(cpustate);
		cpustate->zero=v==READ_RAM(cpustate, A);
		if (cpustate->zero) {
			cpustate->pc=adr;
			return;
		}
	}
	cpustate->pc=READ_OP_ARG_WORD(cpustate);
}


INLINE void sc61860_call(sc61860_state *cpustate, UINT16 adr)
{
	PUSH(cpustate, cpustate->pc>>8);
	PUSH(cpustate, cpustate->pc&0xff);
	cpustate->pc=adr;
}

INLINE void sc61860_return(sc61860_state *cpustate)
{
	UINT16 t=POP(cpustate);
	t|=POP(cpustate)<<8;
	cpustate->pc=t;
}

INLINE void sc61860_jump(sc61860_state *cpustate, int yes)
{
	UINT16 adr = READ_OP_ARG_WORD(cpustate);
	if (yes) {
		cpustate->pc=adr;
	}
}

INLINE void sc61860_jump_rel_plus(sc61860_state *cpustate, int yes)
{
	UINT16 adr = cpustate->pc + READ_OP_ARG(cpustate);
	if (yes) {
		cpustate->pc=adr;
		cpustate->icount-=3;
	}
}

INLINE void sc61860_jump_rel_minus(sc61860_state *cpustate, int yes)
{
	UINT16 adr = cpustate->pc - READ_OP_ARG(cpustate);
	if (yes) {
		cpustate->pc=adr;
		cpustate->icount-=3;
	}
}

INLINE void sc61860_loop(sc61860_state *cpustate)
{
	UINT16 adr = cpustate->pc - READ_OP_ARG(cpustate);
	UINT8 t = READ_RAM(cpustate, cpustate->r) - 1;
	WRITE_RAM(cpustate, cpustate->r, t);
	cpustate->zero=t==0;
	cpustate->carry=t==0xff;
	if (!cpustate->carry) {
		cpustate->pc=adr;
		adr=POP(cpustate);
		cpustate->icount-=3;
	}
}

INLINE void sc61860_leave(sc61860_state *cpustate)
{
	WRITE_RAM(cpustate, cpustate->r, 0);
}

INLINE void sc61860_wait(sc61860_state *cpustate)
{
	int t=READ_OP(cpustate);
	cpustate->icount-=t;
	cpustate->icount-=t;
	cpustate->icount-=3;
}

INLINE void sc61860_set_carry(sc61860_state *cpustate)
{
	cpustate->carry=1;
	cpustate->zero=1;
}

INLINE void sc61860_reset_carry(sc61860_state *cpustate)
{
	cpustate->carry=0;
	cpustate->zero=1;
}

INLINE void sc61860_out_a(sc61860_state *cpustate)
{
	cpustate->q=IA;
	if (cpustate->config&&cpustate->config->outa)
		cpustate->config->outa(cpustate->device, READ_RAM(cpustate, IA));
}

INLINE void sc61860_out_b(sc61860_state *cpustate)
{
	cpustate->q=IB;
	if (cpustate->config&&cpustate->config->outb)
		cpustate->config->outb(cpustate->device, READ_RAM(cpustate, IB));
}

INLINE void sc61860_out_f(sc61860_state *cpustate)
{
	cpustate->q=F0;
	/*READ_RAM(cpustate, F0); */
}


/*   c0 display on
   c1 counter reset
   c2 cpu halt
   c3 computer off
   c4 beeper frequency (1 4khz, 0 2khz), or (c5=0) membran pos1/pos2
   c5 beeper on
   c6 beeper steuerung*/
INLINE void sc61860_out_c(sc61860_state *cpustate)
{
	cpustate->q=C;
	if (cpustate->config&&cpustate->config->outc)
		cpustate->config->outc(cpustate->device, READ_RAM(cpustate, C));
	cpustate->c = READ_RAM(cpustate, C);
}

INLINE void sc61860_in_a(sc61860_state *cpustate)
{
	int data=0;
	if (cpustate->config&&cpustate->config->ina) data=cpustate->config->ina(cpustate->device);
	WRITE_RAM(cpustate, A, data);
	cpustate->zero=data==0;
}

INLINE void sc61860_in_b(sc61860_state *cpustate)
{
	int data=0;
	if (cpustate->config&&cpustate->config->inb) data=cpustate->config->inb(cpustate->device);
	WRITE_RAM(cpustate, A, data);
	cpustate->zero=data==0;
}

/* 0 systemclock 512ms
   1 systemclock 2ms
   2 ?
   3 brk/on key
   4 ?
   5 ?
   6 reset
   7 cassette input */
INLINE void sc61860_test_special(sc61860_state *cpustate)
{
	int t=0;
	if (cpustate->timer.t512ms) t|=1;
	if (cpustate->timer.t2ms) t|=2;
	if (cpustate->config&&cpustate->config->brk&&cpustate->config->brk(cpustate->device)) t|=8;
	if (cpustate->config&&cpustate->config->reset&&cpustate->config->reset(cpustate->device)) t|=0x40;
	if (cpustate->config&&cpustate->config->x&&cpustate->config->x(cpustate->device)) t|=0x80;

	cpustate->zero=(t&READ_OP(cpustate))==0;
}

/************************************************************************************
 "string" operations
***********************************************************************************/

// p-=I+1 sideeffect
INLINE void sc61860_add_bcd_a(sc61860_state *cpustate)
{
	UINT8 help = READ_RAM(cpustate, A);
	int i, hlp, hlp1 = 0;
	cpustate->zero=1;
	for (i=0; i <= READ_RAM(cpustate, I); i++) {
		int t = READ_RAM(cpustate, cpustate->p);
		hlp1 = (t & 0x0f) + (help & 0x0f) + hlp1;
		if (hlp1 > 9) { hlp = hlp1 - 0x0a; hlp1 = 0x10; }
		else { hlp = hlp1; hlp1 = 0x00; }
		hlp1 = (t & 0xf0) + (help & 0xf0) + hlp1;
		if (hlp1 > 0x90) { WRITE_RAM(cpustate, cpustate->p, hlp1 - 0xa0 + hlp); hlp1 = 1; }
		else { WRITE_RAM(cpustate, cpustate->p, hlp1 + hlp); hlp1 = 0; }
		if ( READ_RAM(cpustate, cpustate->p) != 0 ) cpustate->zero = 0;
		cpustate->p--;
		help = 0;
	}
	cpustate->carry= ( hlp1 ) ? 1 : 0;
	cpustate->icount-=3*(READ_RAM(cpustate, I)+1);
}


// p-=I+1, q-=I+2 sideeffect
INLINE void sc61860_add_bcd(sc61860_state *cpustate)
{
	int i, hlp, hlp1 = 0;
	cpustate->zero=1;
	for (i=0; i <= READ_RAM(cpustate, I); i++) {
		int t = READ_RAM(cpustate, cpustate->p);
		int t2 = READ_RAM(cpustate, cpustate->q);
		hlp1 = (t & 0x0f) + (t2 & 0x0f) + hlp1;
		if (hlp1 > 9) { hlp = hlp1 - 0x0a; hlp1 = 0x10; }
		else { hlp = hlp1; hlp1 = 0x00; }
		hlp1 = (t & 0xf0) + (t2 & 0xf0) + hlp1;
		cpustate->q--;
		if (hlp1 > 0x90) { WRITE_RAM(cpustate, cpustate->p, hlp1 - 0xa0 + hlp); hlp1 = 1; }
		else { WRITE_RAM(cpustate, cpustate->p, hlp1 + hlp); hlp1 = 0; }
		if ( READ_RAM(cpustate, cpustate->p) != 0 ) cpustate->zero = 0;
		cpustate->p--;
	}
	cpustate->carry= ( hlp1 ) ? 1 : 0;
	cpustate->icount-=3*(READ_RAM(cpustate, I)+1);
	cpustate->q--;
}


// p-=I+1 sideeffect
INLINE void sc61860_sub_bcd_a(sc61860_state *cpustate)
{
	UINT8 help = READ_RAM(cpustate, A);
	int i, hlp, hlp1 = 0;
	cpustate->zero=1;
	for (i=0; i <= READ_RAM(cpustate, I); i++) {
		int t = READ_RAM(cpustate, cpustate->p);
		hlp1 = (t & 0x0f) - (help & 0x0f) - hlp1;
		if ( hlp1 < 0 ) { hlp = hlp1 + 0x0a; hlp1 = 0x10; }
		else { hlp = hlp1; hlp1 = 0x00; }
		hlp1 = (t & 0xf0) - (help & 0xf0) - hlp1;
		if ( hlp1 < 0 ) { WRITE_RAM(cpustate, cpustate->p, hlp1 + 0xa0 + hlp); hlp1 = 1; }
		else { WRITE_RAM(cpustate, cpustate->p, hlp1 + hlp); hlp1 = 0; }
		if ( READ_RAM(cpustate, cpustate->p) != 0 ) cpustate->zero = 0;
		cpustate->p--;
		help = 0;
	}
	cpustate->carry= ( hlp1 ) ? 1 : 0;
	cpustate->icount-=3*(READ_RAM(cpustate, I)+1);
}


// p-=I+1, q-=I+2 sideeffect
INLINE void sc61860_sub_bcd(sc61860_state *cpustate)
{
	int i, hlp, hlp1 = 0;
	cpustate->zero=1;
	for (i=0; i <= READ_RAM(cpustate, I); i++) {
		int t = READ_RAM(cpustate, cpustate->p);
		int t2 = READ_RAM(cpustate, cpustate->q);
		hlp1 = (t & 0x0f) - (t2 & 0x0f) - hlp1;
		if ( hlp1 < 0 ) { hlp = hlp1 + 0x0a; hlp1 = 0x10; }
		else { hlp = hlp1; hlp1 = 0x00; }
		hlp1 = (t & 0xf0) - (t2 & 0xf0) - hlp1;
		cpustate->q--;
		if ( hlp1 < 0 ) { WRITE_RAM(cpustate, cpustate->p, hlp1 + 0xa0 + hlp); hlp1 = 1; }
		else { WRITE_RAM(cpustate, cpustate->p, hlp1 + hlp); hlp1 = 0; }
		if ( READ_RAM(cpustate, cpustate->p) != 0 ) cpustate->zero = 0;
		cpustate->p--;
	}
	cpustate->carry= ( hlp1 ) ? 1 : 0;
	cpustate->icount-=3*(READ_RAM(cpustate, I)+1);
	cpustate->q--;
}

/* side effect p-i-1 -> p correct! */
INLINE void sc61860_shift_left_nibble(sc61860_state *cpustate)
{
	int i,t=0;
	for (i=0; i<=READ_RAM(cpustate, I); i++) {
		t |= READ_RAM(cpustate, cpustate->p)<<4;
		WRITE_RAM(cpustate, cpustate->p, t);
		cpustate->p--;
		t>>=8;
		cpustate->icount--;
	}
}

/* side effect p+i+1 -> p correct! */
INLINE void sc61860_shift_right_nibble(sc61860_state *cpustate)
{
	int i,t=0;
	for (i=0; i<=READ_RAM(cpustate, I); i++) {
		t |= READ_RAM(cpustate, cpustate->p);
		WRITE_RAM(cpustate, cpustate->p, t>>4);
		cpustate->p++;
		t=(t<<8)&0xf00;
		cpustate->icount--;
	}
}

// q=reg+1 sideeffect
INLINE void sc61860_inc_load_dp(sc61860_state *cpustate, int reg)
{
	UINT8 t = READ_RAM(cpustate, reg) + 1;
	UINT8 t2 = READ_RAM(cpustate, reg + 1);
	WRITE_RAM(cpustate, reg, t);
	if (t == 0) { t2++; WRITE_RAM(cpustate, reg + 1, t2); }
	cpustate->dp=t|(t2<<8);
	cpustate->q=reg+1;
}

// q=reg+1 sideeffect
INLINE void sc61860_dec_load_dp(sc61860_state *cpustate, int reg)
{
	UINT8 t = READ_RAM(cpustate, reg) - 1;
	UINT8 t2 = READ_RAM(cpustate, reg + 1);
	WRITE_RAM(cpustate, reg, t);
	if (t == 0xff) { t2--; WRITE_RAM(cpustate, reg + 1, t2); }
	cpustate->dp=t|(t2<<8);
	cpustate->q=reg+1;
}

// q=XH sideeffect
INLINE void sc61860_inc_load_dp_load(sc61860_state *cpustate)
{
	sc61860_inc_load_dp(cpustate, XL);
	WRITE_RAM(cpustate, A, READ_BYTE(cpustate, cpustate->dp));
}

// q=XH sideeffect
INLINE void sc61860_dec_load_dp_load(sc61860_state *cpustate)
{
	sc61860_dec_load_dp(cpustate, XL);
	WRITE_RAM(cpustate, A, READ_BYTE(cpustate, cpustate->dp));
}

// q=YH sideeffect
INLINE void sc61860_inc_load_dp_store(sc61860_state *cpustate)
{
	sc61860_inc_load_dp(cpustate, YL);
	WRITE_BYTE(cpustate, cpustate->dp, READ_RAM(cpustate, A));
}

// q=YH sideeffect
INLINE void sc61860_dec_load_dp_store(sc61860_state *cpustate)
{
	sc61860_dec_load_dp(cpustate, YL);
	WRITE_BYTE(cpustate, cpustate->dp, READ_RAM(cpustate, A));
}

INLINE void sc61860_fill(sc61860_state *cpustate)
{
	int i;
	for (i=0;i<=READ_RAM(cpustate, I);i++) {
		WRITE_RAM(cpustate, cpustate->p, READ_RAM(cpustate, A)); /* could be overwritten? */
		cpustate->p++;
		cpustate->icount--;
	}
}

INLINE void sc61860_fill_ext(sc61860_state *cpustate)
{
	int i;
	for (i=0;i<=READ_RAM(cpustate, I);i++) {
		WRITE_BYTE(cpustate, cpustate->dp, READ_RAM(cpustate, A));
		if (i!=READ_RAM(cpustate, I)) cpustate->dp++;
		cpustate->icount-=3;
	}
}

// p+=count+1, q+=count+1 sideeffects
INLINE void sc61860_copy(sc61860_state *cpustate, int count)
{
	int i;
	for (i=0; i<=count; i++) {
		WRITE_RAM(cpustate, cpustate->p, READ_RAM(cpustate, cpustate->q));
		cpustate->p++;
		cpustate->q++;
		cpustate->icount-=2;
	}

}

// p+=count+1, dp+=count sideeffects
INLINE void sc61860_copy_ext(sc61860_state *cpustate, int count)
{
	int i;
	for (i=0; i<=count; i++) {
		WRITE_RAM(cpustate, cpustate->p, READ_BYTE(cpustate, cpustate->dp));
		cpustate->p++;
		if (i!=count) cpustate->dp++;
		cpustate->icount-=4;
	}
}

INLINE void sc61860_copy_int(sc61860_state *cpustate, int count)
{
	int i;
	for (i=0; i<=count; i++) {
		UINT8 t = READ_BYTE(cpustate, (READ_RAM(cpustate, A)|(READ_RAM(cpustate, B)<<8))); /* internal rom! */
		WRITE_RAM(cpustate, cpustate->p, t);
		cpustate->p++;
		if (i!=count) {
			t = READ_RAM(cpustate, A) + 1;
			WRITE_RAM(cpustate, A, t);
			if (t==0) {
				t = READ_RAM(cpustate, B) + 1;
				WRITE_RAM(cpustate, B, t);
			}
		}
		cpustate->icount-=4;
	}
}

INLINE void sc61860_exchange(sc61860_state *cpustate, int count)
{
	int i;
	UINT8 t;
	for (i=0; i<=count; i++) {
		t = READ_RAM(cpustate, cpustate->p);
		WRITE_RAM(cpustate, cpustate->p, READ_RAM(cpustate, cpustate->q));
		WRITE_RAM(cpustate, cpustate->q, t);
		cpustate->p++;
		cpustate->q++;
		cpustate->icount-=3;
	}
}

INLINE void sc61860_exchange_ext(sc61860_state *cpustate, int count)
{
	int i;
	UINT8 t;
	for (i=0; i<=count; i++) {
		t = READ_RAM(cpustate, cpustate->p);
		WRITE_RAM(cpustate, cpustate->p, READ_BYTE(cpustate, cpustate->dp));
		cpustate->p++;
		WRITE_BYTE(cpustate, cpustate->dp, t);
		if (i!=count) cpustate->dp++;
		cpustate->icount-=6;
	}
}

// undocumented
// only 1 opcode working in pc1403
// both opcodes working in pc1350
INLINE void sc61860_wait_x(sc61860_state *cpustate, int level)
{
	int c;
	cpustate->zero=level;

	if (cpustate->config&&cpustate->config->x) {
		for (c=READ_RAM(cpustate, I); c>=0; c--) {
			UINT8 t = (READ_RAM(cpustate, cpustate->p)+1)&0x7f;
			WRITE_RAM(cpustate, cpustate->p, t);
			cpustate->zero=cpustate->config->x(cpustate->device);
			cpustate->icount-=4;
			if (level != cpustate->zero) break;
		}
	}
}
