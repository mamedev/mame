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
	return memory_decrypted_read_byte(cpustate->program, cpustate->pc++);
}

INLINE UINT8 READ_OP_ARG(sc61860_state *cpustate)
{
	return memory_raw_read_byte(cpustate->program, cpustate->pc++);
}

INLINE UINT16 READ_OP_ARG_WORD(sc61860_state *cpustate)
{
	UINT16 t=memory_decrypted_read_byte(cpustate->program, cpustate->pc++)<<8;
	t|=memory_decrypted_read_byte(cpustate->program, cpustate->pc++);
	return t;
}

INLINE UINT8 READ_BYTE(sc61860_state *cpustate, UINT16 adr)
{
	return cpustate->program->read_byte(adr);
}

INLINE void WRITE_BYTE(sc61860_state *cpustate, UINT16 a,UINT8 v)
{
	cpustate->program->write_byte(a,v);
}

#define PUSH(v) cpustate->ram[--cpustate->r]=v
#define POP(cpustate) cpustate->ram[cpustate->r++]

INLINE void sc61860_load_imm(sc61860_state *cpustate, int r, UINT8 v)
{
	cpustate->ram[r]=v;
}

INLINE void sc61860_load(sc61860_state *cpustate)
{
	cpustate->ram[A]=cpustate->ram[cpustate->p];
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
	cpustate->r=cpustate->ram[A]&0x7f;
}

INLINE void sc61860_load_ext(sc61860_state *cpustate, int r)
{
	cpustate->ram[r]=READ_BYTE(cpustate, cpustate->dp);
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
	cpustate->ram[A]=cpustate->p;
}

INLINE void sc61860_store_q(sc61860_state *cpustate)
{
	cpustate->ram[A]=cpustate->q;
}

INLINE void sc61860_store_r(sc61860_state *cpustate)
{
	cpustate->ram[A]=cpustate->r;
}

INLINE void sc61860_store_ext(sc61860_state *cpustate, int r)
{
	WRITE_BYTE(cpustate, cpustate->dp, cpustate->ram[r]);
}

INLINE void sc61860_exam(sc61860_state *cpustate, int a, int b)
{
	UINT8 t=cpustate->ram[a];
	cpustate->ram[a]=cpustate->ram[b];
	cpustate->ram[b]=t;
}

INLINE void sc61860_test(sc61860_state *cpustate, int reg, UINT8 value)
{
	cpustate->zero=(cpustate->ram[reg]&value)==0;
}

INLINE void sc61860_test_ext(sc61860_state *cpustate)
{
	cpustate->zero=(READ_BYTE(cpustate, cpustate->dp)&READ_OP_ARG(cpustate))==0;
}

INLINE void sc61860_and(sc61860_state *cpustate, int reg, UINT8 value)
{
	cpustate->zero=(cpustate->ram[reg]&=value)==0;
}

INLINE void sc61860_and_ext(sc61860_state *cpustate)
{
	UINT8 t=READ_BYTE(cpustate, cpustate->dp)&READ_OP_ARG(cpustate);
	cpustate->zero=t==0;
    WRITE_BYTE(cpustate, cpustate->dp,t);
}

INLINE void sc61860_or(sc61860_state *cpustate, int reg, UINT8 value)
{
	cpustate->zero=(cpustate->ram[reg]|=value)==0;
}

INLINE void sc61860_or_ext(sc61860_state *cpustate)
{
	UINT8 t=READ_BYTE(cpustate, cpustate->dp)|READ_OP_ARG(cpustate);
	cpustate->zero=t==0;
    WRITE_BYTE(cpustate, cpustate->dp,t);
}

INLINE void sc61860_rotate_right(sc61860_state *cpustate)
{
	int t=cpustate->ram[A];
	if (cpustate->carry) t|=0x100;
	cpustate->carry=t&1;
	cpustate->ram[A]=t>>1;
}

INLINE void sc61860_rotate_left(sc61860_state *cpustate)
{
	int t=cpustate->ram[A]<<1;
	if (cpustate->carry) t|=1;
	cpustate->carry=t&0x100;
	cpustate->ram[A]=t;
}

INLINE void sc61860_swap(sc61860_state *cpustate)
{
	int t=cpustate->ram[A];
	cpustate->ram[A]=(t<<4)|((t>>4)&0xf);
}

// q=reg sideeffect
INLINE void sc61860_inc(sc61860_state *cpustate, int reg)
{
	cpustate->q=reg;
	cpustate->ram[reg]++;
	cpustate->zero=cpustate->carry=cpustate->ram[reg]==0;
}

INLINE void sc61860_inc_p(sc61860_state *cpustate)
{
	cpustate->p++;
}

// q=reg sideeffect
INLINE void sc61860_dec(sc61860_state *cpustate, int reg)
{
	cpustate->q=reg;
	cpustate->ram[reg]--;
	cpustate->zero=cpustate->ram[reg]==0;
	cpustate->carry=cpustate->ram[reg]==0xff;
}

INLINE void sc61860_dec_p(sc61860_state *cpustate)
{
	cpustate->p--;
}

INLINE void sc61860_add(sc61860_state *cpustate, int reg, UINT8 value)
{
	int t=cpustate->ram[reg]+value;
	cpustate->zero=(cpustate->ram[reg]=t)==0;
	cpustate->carry=t>=0x100;
}

INLINE void sc61860_add_carry(sc61860_state *cpustate)
{
	int t=cpustate->ram[cpustate->p]+cpustate->ram[A];
	if (cpustate->carry) t++;
	cpustate->zero=(cpustate->ram[cpustate->p]=t)==0;
	cpustate->carry=t>=0x100;
}

// p++ sideeffect
INLINE void sc61860_add_word(sc61860_state *cpustate)
{
	int t=cpustate->ram[cpustate->p]+cpustate->ram[A],t2;
	cpustate->ram[cpustate->p]=t;
	cpustate->p++;
	t2=cpustate->ram[cpustate->p]+cpustate->ram[B];
	if (t>=0x100) t2++;
	cpustate->ram[cpustate->p]=t2;
	cpustate->zero=(t2&0xff)==0 &&(t&0xff)==0;
	cpustate->carry=t2>=0x100;
}


INLINE void sc61860_sub(sc61860_state *cpustate, int reg, UINT8 value)
{
	int t=cpustate->ram[reg]-value;
	cpustate->zero=(cpustate->ram[reg]=t)==0;
	cpustate->carry=t<0;
}

INLINE void sc61860_sub_carry(sc61860_state *cpustate)
{
	int t=cpustate->ram[cpustate->p]-cpustate->ram[A];
	if (cpustate->carry) t--;
	cpustate->zero=(cpustate->ram[cpustate->p]=t)==0;
	cpustate->carry=t<0;
}


// p++ sideeffect
INLINE void sc61860_sub_word(sc61860_state *cpustate)
{
	int t=cpustate->ram[cpustate->p]-cpustate->ram[A],t2;
	cpustate->ram[cpustate->p]=t;
	cpustate->p++;
	t2=cpustate->ram[cpustate->p]-cpustate->ram[B];
	if (t<0) t2--;
	cpustate->ram[cpustate->p]=t2;
	cpustate->zero=(t2&0xff)==0 && (t&0xff)==0;
	cpustate->carry=t2<0;
}

INLINE void sc61860_cmp(sc61860_state *cpustate, int reg, UINT8 value)
{
	int t=cpustate->ram[reg]-value;
	cpustate->zero=t==0;
	cpustate->carry=t<0;
}

INLINE void sc61860_pop(sc61860_state *cpustate)
{
	cpustate->ram[A]=POP(cpustate);
}

INLINE void sc61860_push(sc61860_state *cpustate)
{
	PUSH(cpustate->ram[A]);
}

INLINE void sc61860_prepare_table_call(sc61860_state *cpustate)
{
	int adr;
	cpustate->h=READ_OP(cpustate);
	adr=READ_OP_ARG_WORD(cpustate);
	PUSH(adr>>8);
	PUSH(adr&0xff);
}

INLINE void sc61860_execute_table_call(sc61860_state *cpustate)
{
	int i, v, adr;
	for (i=0; i<cpustate->h; i++) {
		v=READ_OP(cpustate);
		adr=READ_OP_ARG_WORD(cpustate);
		cpustate->zero=v==cpustate->ram[A];
		if (cpustate->zero) {
			cpustate->pc=adr;
			return;
		}
	}
	cpustate->pc=READ_OP_ARG_WORD(cpustate);
}


INLINE void sc61860_call(sc61860_state *cpustate, UINT16 adr)
{
	PUSH(cpustate->pc>>8);
	PUSH(cpustate->pc&0xff);
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
	UINT16 adr=READ_OP_ARG_WORD(cpustate);
	if (yes) {
		cpustate->pc=adr;
	}
}

INLINE void sc61860_jump_rel_plus(sc61860_state *cpustate, int yes)
{
	UINT16 adr=cpustate->pc;
	adr+=READ_OP_ARG(cpustate);
	if (yes) {
		cpustate->pc=adr;
		cpustate->icount-=3;
	}
}

INLINE void sc61860_jump_rel_minus(sc61860_state *cpustate, int yes)
{
	UINT16 adr=cpustate->pc;
	adr-=READ_OP_ARG(cpustate);
	if (yes) {
		cpustate->pc=adr;
		cpustate->icount-=3;
	}
}

INLINE void sc61860_loop(sc61860_state *cpustate)
{
	UINT16 adr=cpustate->pc;
	adr-=READ_OP_ARG(cpustate);
	cpustate->ram[cpustate->r]--;
	cpustate->zero=cpustate->ram[cpustate->r]==0;
	cpustate->carry=cpustate->ram[cpustate->r]==0xff;
	if (!cpustate->carry) {
		cpustate->pc=adr;
		adr=POP(cpustate);
		cpustate->icount-=3;
	}
}

INLINE void sc61860_leave(sc61860_state *cpustate)
{
	cpustate->ram[cpustate->r]=0;
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
	    cpustate->config->outa(cpustate->device, cpustate->ram[IA]);
}

INLINE void sc61860_out_b(sc61860_state *cpustate)
{
	cpustate->q=IB;
	if (cpustate->config&&cpustate->config->outb)
	    cpustate->config->outb(cpustate->device, cpustate->ram[IB]);
}

INLINE void sc61860_out_f(sc61860_state *cpustate)
{
	cpustate->q=F0;
	/*cpustate->ram[F0]; */
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
	cpustate->config->outc(cpustate->device, cpustate->ram[C]);
    cpustate->c=cpustate->ram[C];
}

INLINE void sc61860_in_a(sc61860_state *cpustate)
{
	int data=0;
	if (cpustate->config&&cpustate->config->ina) data=cpustate->config->ina(cpustate->device);
	cpustate->ram[A]=data;
	cpustate->zero=data==0;
}

INLINE void sc61860_in_b(sc61860_state *cpustate)
{
	int data=0;
	if (cpustate->config&&cpustate->config->inb) data=cpustate->config->inb(cpustate->device);
	cpustate->ram[A]=data;
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
 UINT8 help = cpustate->ram[A];
 int i, hlp, hlp1 = 0; cpustate->zero=1;
 for ( i=0; i <= cpustate->ram[I]; i++)
 {
  hlp1 = (cpustate->ram[cpustate->p] & 0x0f) + (help & 0x0f ) + hlp1;
  if (hlp1 > 9) { hlp = hlp1 - 0x0a; hlp1 = 0x10; }
  else {hlp = hlp1; hlp1 = 0x00;}
  hlp1 = (cpustate->ram[cpustate->p] & 0xf0) + (help & 0xf0) + hlp1;
  if (hlp1 > 0x90) { cpustate->ram[cpustate->p] = hlp1 - 0xa0 + hlp; hlp1 = 1; }
  else {cpustate->ram[cpustate->p] = hlp1 + hlp; hlp1 = 0;}
  if ( cpustate->ram[cpustate->p--] != 0 ) cpustate->zero = 0;
  help = 0;
 }
 cpustate->carry= ( hlp1 ) ? 1 : 0;
 cpustate->icount-=3*(cpustate->ram[I]+1);
}


// p-=I+1, q-=I+2 sideeffect
INLINE void sc61860_add_bcd(sc61860_state *cpustate)
{
 int i, hlp, hlp1 = 0; cpustate->zero=1;
 for ( i=0; i <= cpustate->ram[I]; i++)
 {
  hlp1 = (cpustate->ram[cpustate->p] & 0x0f) + (cpustate->ram[cpustate->q] & 0x0f ) +
hlp1;
  if (hlp1 > 9) { hlp = hlp1 - 0x0a; hlp1 = 0x10; }
  else {hlp = hlp1; hlp1 = 0x00;}
  hlp1 = (cpustate->ram[cpustate->p] & 0xf0) + (cpustate->ram[cpustate->q--] & 0xf0) +
hlp1;
  if (hlp1 > 0x90) { cpustate->ram[cpustate->p] = hlp1 - 0xa0 + hlp; hlp1 = 1; }
  else {cpustate->ram[cpustate->p] = hlp1 + hlp; hlp1 = 0;}
  if ( cpustate->ram[cpustate->p--] != 0 ) cpustate->zero = 0;
 }
 cpustate->carry= ( hlp1 ) ? 1 : 0;
 cpustate->icount-=3*(cpustate->ram[I]+1);
 cpustate->q--;
}


// p-=I+1 sideeffect
INLINE void sc61860_sub_bcd_a(sc61860_state *cpustate)
{
 UINT8 help = cpustate->ram[A];
 int i, hlp, hlp1 = 0; cpustate->zero=1;
 for ( i=0; i <= cpustate->ram[I]; i++)
 {
  hlp1 = (cpustate->ram[cpustate->p]&0x0f) - (help&0x0f) - hlp1;
  if ( hlp1 < 0 ) { hlp = hlp1 + 0x0a; hlp1 = 0x10;}
  else { hlp = hlp1; hlp1 = 0x00;}
  hlp1 = (cpustate->ram[cpustate->p]&0xf0) - (help&0xf0) - hlp1;
  if ( hlp1 < 0 ) { cpustate->ram[cpustate->p] = hlp1 + 0xa0 + hlp; hlp1 = 1;}
  else {cpustate->ram[cpustate->p] = hlp1 + hlp; hlp1 = 0;}
  if ( cpustate->ram[cpustate->p--] != 0 ) cpustate->zero = 0;
  help = 0;
 }
 cpustate->carry= ( hlp1 ) ? 1 : 0;
 cpustate->icount-=3*(cpustate->ram[I]+1);
}


// p-=I+1, q-=I+2 sideeffect
INLINE void sc61860_sub_bcd(sc61860_state *cpustate)
{
 int i, hlp, hlp1 = 0; cpustate->zero=1;
 for ( i=0; i <= cpustate->ram[I]; i++)
 {
  hlp1 = (cpustate->ram[cpustate->p]&0x0f) - (cpustate->ram[cpustate->q]&0x0f) - hlp1;
  if ( hlp1 < 0 ) { hlp = hlp1 + 0x0a; hlp1 = 0x10;}
  else { hlp = hlp1; hlp1 = 0x00;}
  hlp1 = (cpustate->ram[cpustate->p]&0xf0) - (cpustate->ram[cpustate->q--]&0xf0) -
hlp1;
  if ( hlp1 < 0 ) { cpustate->ram[cpustate->p] = hlp1 + 0xa0 + hlp; hlp1 = 1;}
  else {cpustate->ram[cpustate->p] = hlp1 + hlp; hlp1 = 0;}
  if ( cpustate->ram[cpustate->p--] != 0 ) cpustate->zero = 0;
 }
 cpustate->carry= ( hlp1 ) ? 1 : 0;
 cpustate->icount-=3*(cpustate->ram[I]+1);
 cpustate->q--;
}

/* side effect p-i-1 -> p correct! */
INLINE void sc61860_shift_left_nibble(sc61860_state *cpustate)
{
	int i,t=0;
	for (i=0; i<=cpustate->ram[I]; i++) {
		t|=cpustate->ram[cpustate->p]<<4;
		cpustate->ram[cpustate->p--]=t;
		t>>=8;
		cpustate->icount--;
	}
}

/* side effect p+i+1 -> p correct! */
INLINE void sc61860_shift_right_nibble(sc61860_state *cpustate)
{
	int i,t=0;
	for (i=0; i<=cpustate->ram[I]; i++) {
		t|=cpustate->ram[cpustate->p];
		cpustate->ram[cpustate->p++]=t>>4;
		t=(t<<8)&0xf00;
		cpustate->icount--;
	}
}

// q=reg+1 sideeffect
INLINE void sc61860_inc_load_dp(sc61860_state *cpustate, int reg)
{
    if (++cpustate->ram[reg]==0) cpustate->ram[reg+1]++;
    cpustate->dp=cpustate->ram[reg]|(cpustate->ram[reg+1]<<8);
    cpustate->q=reg+1;
}

// q=reg+1 sideeffect
INLINE void sc61860_dec_load_dp(sc61860_state *cpustate, int reg)
{
    if (--cpustate->ram[reg]==0xff) cpustate->ram[reg+1]--;
    cpustate->dp=cpustate->ram[reg]|(cpustate->ram[reg+1]<<8);
    cpustate->q=reg+1;
}

// q=XH sideeffect
INLINE void sc61860_inc_load_dp_load(sc61860_state *cpustate)
{
    if (++cpustate->ram[XL]==0) cpustate->ram[XH]++;
    cpustate->dp=cpustate->ram[XL]|(cpustate->ram[XH]<<8);
    cpustate->q=XH; // hopefully correct before real read
    cpustate->ram[A]=READ_BYTE(cpustate, cpustate->dp);
}

// q=XH sideeffect
INLINE void sc61860_dec_load_dp_load(sc61860_state *cpustate)
{
    if (--cpustate->ram[XL]==0xff) cpustate->ram[XH]--;
    cpustate->dp=cpustate->ram[XL]|(cpustate->ram[XH]<<8);
    cpustate->q=XH; // hopefully correct before real read
    cpustate->ram[A]=READ_BYTE(cpustate, cpustate->dp);
}

// q=YH sideeffect
INLINE void sc61860_inc_load_dp_store(sc61860_state *cpustate)
{
    if (++cpustate->ram[YL]==0) cpustate->ram[YH]++;
    cpustate->dp=cpustate->ram[YL]|(cpustate->ram[YH]<<8);
    cpustate->q=YH; // hopefully correct before real write!
    WRITE_BYTE(cpustate, cpustate->dp,cpustate->ram[A]);
}

// q=YH sideeffect
INLINE void sc61860_dec_load_dp_store(sc61860_state *cpustate)
{
    if (--cpustate->ram[YL]==0xff) cpustate->ram[YH]--;
    cpustate->dp=cpustate->ram[YL]|(cpustate->ram[YH]<<8);
    cpustate->q=XH; // hopefully correct before real write!
    WRITE_BYTE(cpustate, cpustate->dp,cpustate->ram[A]);
}

INLINE void sc61860_fill(sc61860_state *cpustate)
{
	int i;
	for (i=0;i<=cpustate->ram[I];i++) {
		cpustate->ram[cpustate->p++]=cpustate->ram[A]; /* could be overwritten? */
		cpustate->icount--;
	}
}

INLINE void sc61860_fill_ext(sc61860_state *cpustate)
{
	int i;
	for (i=0;i<=cpustate->ram[I];i++) {
		WRITE_BYTE(cpustate, cpustate->dp, cpustate->ram[A]);
		if (i!=cpustate->ram[I]) cpustate->dp++;
		cpustate->icount-=3;
	}
}

// p+=count+1, q+=count+1 sideeffects
INLINE void sc61860_copy(sc61860_state *cpustate, int count)
{
	int i;
	for (i=0; i<=count; i++) {
		cpustate->ram[cpustate->p++]=cpustate->ram[cpustate->q++];
		cpustate->icount-=2;
	}

}

// p+=count+1, dp+=count sideeffects
INLINE void sc61860_copy_ext(sc61860_state *cpustate, int count)
{
	int i;
	for (i=0; i<=count; i++) {
		cpustate->ram[cpustate->p++]=READ_BYTE(cpustate, cpustate->dp);
		if (i!=count) cpustate->dp++;
		cpustate->icount-=4;
	}
}

INLINE void sc61860_copy_int(sc61860_state *cpustate, int count)
{
	int i;
	for (i=0; i<=count; i++) {
		cpustate->ram[cpustate->p++]=
			READ_BYTE(cpustate, (cpustate->ram[A]|(cpustate->ram[B]<<8)) ); /* internal rom! */
		if (i!=count) {
			if (++cpustate->ram[A]==0) cpustate->ram[B]++;
		}
		cpustate->icount-=4;
	}
}

INLINE void sc61860_exchange(sc61860_state *cpustate, int count)
{
	int i;
	UINT8 t;
	for (i=0; i<=count; i++) {
		t=cpustate->ram[cpustate->p];
		cpustate->ram[cpustate->p++]=cpustate->ram[cpustate->q];
		cpustate->ram[cpustate->q++]=t;
		cpustate->icount-=3;
	}
}

INLINE void sc61860_exchange_ext(sc61860_state *cpustate, int count)
{
	int i;
	UINT8 t;
	for (i=0; i<=count; i++) {
		t=cpustate->ram[cpustate->p];
		cpustate->ram[cpustate->p++]=READ_BYTE(cpustate, cpustate->dp);
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
	for (c=cpustate->ram[I]; c>=0; c--) {
//      cpustate->ram[cpustate->p]=(cpustate->ram[cpustate->p]+1)%0x60;
	    cpustate->ram[cpustate->p]=(cpustate->ram[cpustate->p]+1)&0x7f;
	    cpustate->zero=cpustate->config->x(cpustate->device);
	    cpustate->icount-=4;
	    if ( level != cpustate->zero) break;
	}
    }
}




