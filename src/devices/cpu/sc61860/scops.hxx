// license:BSD-3-Clause
// copyright-holders:Peter Trauner
/*****************************************************************************
 *
 *   scops.inc
 *   portable sharp 61860 emulator interface
 *   (sharp pocket computers)
 *
 *   Copyright Peter Trauner
 *
 * History of changes:
 * 21.07.2001 Several changes listed below were made by Mario Konegger
 *            (konegger@itp.tu-graz.ac.at)
 *        replaced buggy BCD-commands add_bcd, sub_bcd, add_bcd_a,
 *            sub_bcd_a and changed out_c, to implement HLT-mode of the CPU.
 *
 *****************************************************************************/

uint8_t sc61860_device::READ_OP()
{
	return m_cache.read_byte(m_pc++);
}

uint8_t sc61860_device::READ_OP_ARG()
{
	return m_cache.read_byte(m_pc++);
}

uint16_t sc61860_device::READ_OP_ARG_WORD()
{
	uint16_t t=m_cache.read_byte(m_pc++)<<8;
	t|=m_cache.read_byte(m_pc++);
	return t;
}

uint8_t sc61860_device::READ_BYTE(uint16_t adr)
{
	return m_program.read_byte(adr);
}

void sc61860_device::WRITE_BYTE(uint16_t a, uint8_t v)
{
	m_program.write_byte(a, v);
}

uint8_t sc61860_device::READ_RAM(int r)
{
	return m_ram[r];
}

void sc61860_device::WRITE_RAM(int r, uint8_t v)
{
	m_ram[r] = v;
}

void sc61860_device::PUSH(uint8_t v)
{
	m_r--;
	WRITE_RAM(m_r, v);
}

uint8_t sc61860_device::POP()
{
	uint8_t t = READ_RAM(m_r);
	m_r++;
	return t;
}

void sc61860_device::sc61860_load_imm(int r, uint8_t v)
{
	WRITE_RAM(r, v);
}

void sc61860_device::sc61860_load()
{
	WRITE_RAM(A, READ_RAM(m_p));
}

void sc61860_device::sc61860_load_imm_p(uint8_t v)
{
	m_p=v&0x7f;
}

void sc61860_device::sc61860_load_imm_q(uint8_t v)
{
	m_q=v&0x7f;
}

void sc61860_device::sc61860_load_r()
{
	m_r = READ_RAM(A) & 0x7f;
}

void sc61860_device::sc61860_load_ext(int r)
{
	WRITE_RAM(r, READ_BYTE(m_dp));
}

void sc61860_device::sc61860_load_dp()
{
	m_dp=READ_OP_ARG_WORD();
}

void sc61860_device::sc61860_load_dl()
{
	m_dp=(m_dp&~0xff)|READ_OP_ARG();
}

void sc61860_device::sc61860_store_p()
{
	WRITE_RAM(A, m_p);
}

void sc61860_device::sc61860_store_q()
{
	WRITE_RAM(A, m_q);
}

void sc61860_device::sc61860_store_r()
{
	WRITE_RAM(A, m_r);
}

void sc61860_device::sc61860_store_ext(int r)
{
	WRITE_BYTE(m_dp, READ_RAM(r));
}

void sc61860_device::sc61860_exam(int a, int b)
{
	uint8_t t = READ_RAM(a);
	WRITE_RAM(a, READ_RAM(b));
	WRITE_RAM(b, t);
}

void sc61860_device::sc61860_test(int reg, uint8_t value)
{
	m_zero=(READ_RAM(reg) & value)==0;
}

void sc61860_device::sc61860_test_ext()
{
	m_zero=(READ_BYTE(m_dp)&READ_OP_ARG())==0;
}

void sc61860_device::sc61860_and(int reg, uint8_t value)
{
	uint8_t t = READ_RAM(reg) & value;
	WRITE_RAM(reg,  t);
	m_zero=t==0;
}

void sc61860_device::sc61860_and_ext()
{
	uint8_t t = READ_BYTE(m_dp) & READ_OP_ARG();
	m_zero=t==0;
	WRITE_BYTE(m_dp, t);
}

void sc61860_device::sc61860_or(int reg, uint8_t value)
{
	uint8_t t = READ_RAM(reg) | value;
	WRITE_RAM(reg, t);
	m_zero=t==0;
}

void sc61860_device::sc61860_or_ext()
{
	uint8_t t=READ_BYTE(m_dp)|READ_OP_ARG();
	m_zero=t==0;
	WRITE_BYTE(m_dp, t);
}

void sc61860_device::sc61860_rotate_right()
{
	int t = READ_RAM(A);
	if (m_carry) t|=0x100;
	m_carry=t&1;
	WRITE_RAM(A, t>>1);
}

void sc61860_device::sc61860_rotate_left()
{
	int t = READ_RAM(A) << 1;
	if (m_carry) t|=1;
	m_carry=t&0x100;
	WRITE_RAM(A, t);
}

void sc61860_device::sc61860_swap()
{
	int t = READ_RAM(A);
	WRITE_RAM(A, (t<<4)|((t>>4)&0xf));
}

// q=reg sideeffect
void sc61860_device::sc61860_inc(int reg)
{
	uint8_t t = READ_RAM(reg) + 1;
	m_q=reg;
	WRITE_RAM(reg, t);
	m_zero=t==0;
	m_carry=t==0;
}

void sc61860_device::sc61860_inc_p()
{
	m_p++;
}

// q=reg sideeffect
void sc61860_device::sc61860_dec(int reg)
{
	uint8_t t = READ_RAM(reg) - 1;
	m_q=reg;
	WRITE_RAM(reg, t);
	m_zero=t==0;
	m_carry=t==0xff;
}

void sc61860_device::sc61860_dec_p()
{
	m_p--;
}

void sc61860_device::sc61860_add(int reg, uint8_t value)
{
	int t = READ_RAM(reg) + value;
	WRITE_RAM(reg, t);
	m_zero=(t&0xff)==0;
	m_carry=t>=0x100;
}

void sc61860_device::sc61860_add_carry()
{
	int t = READ_RAM(m_p) + READ_RAM(A);
	if (m_carry) t++;
	WRITE_RAM(m_p, t);
	m_zero=(t&0xff)==0;
	m_carry=t>=0x100;
}

// p++ sideeffect
void sc61860_device::sc61860_add_word()
{
	int t = READ_RAM(m_p) + READ_RAM(A), t2;
	WRITE_RAM(m_p, t);
	m_p++;
	t2 = READ_RAM(m_p) + READ_RAM(B);
	if (t>=0x100) t2++;
	WRITE_RAM(m_p, t2);
	m_zero=(t2&0xff)==0 &&(t&0xff)==0;
	m_carry=t2>=0x100;
}


void sc61860_device::sc61860_sub(int reg, uint8_t value)
{
	int t = READ_RAM(reg) - value;
	WRITE_RAM(reg, t);
	m_zero=(t&0xff)==0;
	m_carry=t<0;
}

void sc61860_device::sc61860_sub_carry()
{
	int t = READ_RAM(m_p) - READ_RAM(A);
	if (m_carry) t--;
	WRITE_RAM(m_p, t);
	m_zero=(t&0xff)==0;
	m_carry=t<0;
}


// p++ sideeffect
void sc61860_device::sc61860_sub_word()
{
	int t = READ_RAM(m_p) - READ_RAM(A), t2;
	WRITE_RAM(m_p, t);
	m_p++;
	t2 = READ_RAM(m_p) - READ_RAM(B);
	if (t<0) t2--;
	WRITE_RAM(m_p, t2);
	m_zero=(t2&0xff)==0 && (t&0xff)==0;
	m_carry=t2<0;
}

void sc61860_device::sc61860_cmp(int reg, uint8_t value)
{
	int t = READ_RAM(reg) - value;
	m_zero=t==0;
	m_carry=t<0;
}

void sc61860_device::sc61860_pop()
{
	WRITE_RAM(A, POP());
}

void sc61860_device::sc61860_push()
{
	PUSH(READ_RAM(A));
}

void sc61860_device::sc61860_prepare_table_call()
{
	int adr;
	m_h=READ_OP();
	adr=READ_OP_ARG_WORD();
	PUSH(adr>>8);
	PUSH(adr&0xff);
}

void sc61860_device::sc61860_execute_table_call()
{
	int i, v, adr;
	for (i=0; i<m_h; i++) {
		v=READ_OP();
		adr=READ_OP_ARG_WORD();
		m_zero=v==READ_RAM(A);
		if (m_zero) {
			m_pc=adr;
			return;
		}
	}
	m_pc=READ_OP_ARG_WORD();
}


void sc61860_device::sc61860_call(uint16_t adr)
{
	PUSH(m_pc>>8);
	PUSH(m_pc&0xff);
	m_pc=adr;
}

void sc61860_device::sc61860_return()
{
	uint16_t t=POP();
	t|=POP()<<8;
	m_pc=t;
}

void sc61860_device::sc61860_jump(int yes)
{
	uint16_t adr = READ_OP_ARG_WORD();
	if (yes) {
		m_pc=adr;
	}
}

void sc61860_device::sc61860_jump_rel_plus(int yes)
{
	uint16_t adr = m_pc + READ_OP_ARG();
	if (yes) {
		m_pc=adr;
		m_icount-=3;
	}
}

void sc61860_device::sc61860_jump_rel_minus(int yes)
{
	uint16_t adr = m_pc - READ_OP_ARG();
	if (yes) {
		m_pc=adr;
		m_icount-=3;
	}
}

void sc61860_device::sc61860_loop()
{
	uint16_t adr = m_pc - READ_OP_ARG();
	uint8_t t = READ_RAM(m_r) - 1;
	WRITE_RAM(m_r, t);
	m_zero=t==0;
	m_carry=t==0xff;
	if (!m_carry) {
		m_pc=adr;
		adr=POP();
		m_icount-=3;
	}
}

void sc61860_device::sc61860_leave()
{
	WRITE_RAM(m_r, 0);
}

void sc61860_device::sc61860_wait()
{
	int t=READ_OP();
	m_icount-=t;
	m_icount-=t;
	m_icount-=3;
}

void sc61860_device::sc61860_set_carry()
{
	m_carry=1;
	m_zero=1;
}

void sc61860_device::sc61860_reset_carry()
{
	m_carry=0;
	m_zero=1;
}

void sc61860_device::sc61860_out_a()
{
	m_q=IA;
	m_outa(READ_RAM(IA));
}

void sc61860_device::sc61860_out_b()
{
	m_q=IB;
	m_outb( READ_RAM(IB));
}

void sc61860_device::sc61860_out_f()
{
	m_q=F0;
	/*READ_RAM(F0); */
}


/*   c0 display on
   c1 counter reset
   c2 cpu halt
   c3 computer off
   c4 beeper frequency (1 4khz, 0 2khz), or (c5=0) membran pos1/pos2
   c5 beeper on
   c6 beeper steuerung*/
void sc61860_device::sc61860_out_c()
{
	m_q=C;
	m_outc( READ_RAM(C));
	m_c = READ_RAM(C);
}

void sc61860_device::sc61860_in_a()
{
	int data=0;
	data=m_ina();
	WRITE_RAM(A, data);
	m_zero=data==0;
}

void sc61860_device::sc61860_in_b()
{
	int data=0;
	data=m_inb();
	WRITE_RAM(A, data);
	m_zero=data==0;
}

/* 0 systemclock 512ms
   1 systemclock 2ms
   2 ?
   3 brk/on key
   4 ?
   5 ?
   6 reset
   7 cassette input */
void sc61860_device::sc61860_test_special()
{
	int t=0;
	if (m_timer.t512ms) t|=1;
	if (m_timer.t2ms) t|=2;
	if (!m_brk.isunset()&&m_brk()) t|=8;
	if (!m_reset.isunset()&&m_reset()) t|=0x40;
	if (!m_x.isunset()&&m_x()) t|=0x80;

	m_zero=(t&READ_OP())==0;
}

/************************************************************************************
 "string" operations
***********************************************************************************/

// p-=I+1 sideeffect
void sc61860_device::sc61860_add_bcd_a()
{
	uint8_t help = READ_RAM(A);
	int i, hlp, hlp1 = 0;
	m_zero=1;
	for (i=0; i <= READ_RAM(I); i++) {
		int t = READ_RAM(m_p);
		hlp1 = (t & 0x0f) + (help & 0x0f) + hlp1;
		if (hlp1 > 9) { hlp = hlp1 - 0x0a; hlp1 = 0x10; }
		else { hlp = hlp1; hlp1 = 0x00; }
		hlp1 = (t & 0xf0) + (help & 0xf0) + hlp1;
		if (hlp1 > 0x90) { WRITE_RAM(m_p, hlp1 - 0xa0 + hlp); hlp1 = 1; }
		else { WRITE_RAM(m_p, hlp1 + hlp); hlp1 = 0; }
		if ( READ_RAM(m_p) != 0 ) m_zero = 0;
		m_p--;
		help = 0;
	}
	m_carry= ( hlp1 ) ? 1 : 0;
	m_icount-=3*(READ_RAM(I)+1);
}


// p-=I+1, q-=I+2 sideeffect
void sc61860_device::sc61860_add_bcd()
{
	int i, hlp, hlp1 = 0;
	m_zero=1;
	for (i=0; i <= READ_RAM(I); i++) {
		int t = READ_RAM(m_p);
		int t2 = READ_RAM(m_q);
		hlp1 = (t & 0x0f) + (t2 & 0x0f) + hlp1;
		if (hlp1 > 9) { hlp = hlp1 - 0x0a; hlp1 = 0x10; }
		else { hlp = hlp1; hlp1 = 0x00; }
		hlp1 = (t & 0xf0) + (t2 & 0xf0) + hlp1;
		m_q--;
		if (hlp1 > 0x90) { WRITE_RAM(m_p, hlp1 - 0xa0 + hlp); hlp1 = 1; }
		else { WRITE_RAM(m_p, hlp1 + hlp); hlp1 = 0; }
		if ( READ_RAM(m_p) != 0 ) m_zero = 0;
		m_p--;
	}
	m_carry= ( hlp1 ) ? 1 : 0;
	m_icount-=3*(READ_RAM(I)+1);
	m_q--;
}


// p-=I+1 sideeffect
void sc61860_device::sc61860_sub_bcd_a()
{
	uint8_t help = READ_RAM(A);
	int i, hlp, hlp1 = 0;
	m_zero=1;
	for (i=0; i <= READ_RAM(I); i++) {
		int t = READ_RAM(m_p);
		hlp1 = (t & 0x0f) - (help & 0x0f) - hlp1;
		if ( hlp1 < 0 ) { hlp = hlp1 + 0x0a; hlp1 = 0x10; }
		else { hlp = hlp1; hlp1 = 0x00; }
		hlp1 = (t & 0xf0) - (help & 0xf0) - hlp1;
		if ( hlp1 < 0 ) { WRITE_RAM(m_p, hlp1 + 0xa0 + hlp); hlp1 = 1; }
		else { WRITE_RAM(m_p, hlp1 + hlp); hlp1 = 0; }
		if ( READ_RAM(m_p) != 0 ) m_zero = 0;
		m_p--;
		help = 0;
	}
	m_carry= ( hlp1 ) ? 1 : 0;
	m_icount-=3*(READ_RAM(I)+1);
}


// p-=I+1, q-=I+2 sideeffect
void sc61860_device::sc61860_sub_bcd()
{
	int i, hlp, hlp1 = 0;
	m_zero=1;
	for (i=0; i <= READ_RAM(I); i++) {
		int t = READ_RAM(m_p);
		int t2 = READ_RAM(m_q);
		hlp1 = (t & 0x0f) - (t2 & 0x0f) - hlp1;
		if ( hlp1 < 0 ) { hlp = hlp1 + 0x0a; hlp1 = 0x10; }
		else { hlp = hlp1; hlp1 = 0x00; }
		hlp1 = (t & 0xf0) - (t2 & 0xf0) - hlp1;
		m_q--;
		if ( hlp1 < 0 ) { WRITE_RAM(m_p, hlp1 + 0xa0 + hlp); hlp1 = 1; }
		else { WRITE_RAM(m_p, hlp1 + hlp); hlp1 = 0; }
		if ( READ_RAM(m_p) != 0 ) m_zero = 0;
		m_p--;
	}
	m_carry= ( hlp1 ) ? 1 : 0;
	m_icount-=3*(READ_RAM(I)+1);
	m_q--;
}

/* side effect p-i-1 -> p correct! */
void sc61860_device::sc61860_shift_left_nibble()
{
	int i,t=0;
	for (i=0; i<=READ_RAM(I); i++) {
		t |= READ_RAM(m_p)<<4;
		WRITE_RAM(m_p, t);
		m_p--;
		t>>=8;
		m_icount--;
	}
}

/* side effect p+i+1 -> p correct! */
void sc61860_device::sc61860_shift_right_nibble()
{
	int i,t=0;
	for (i=0; i<=READ_RAM(I); i++) {
		t |= READ_RAM(m_p);
		WRITE_RAM(m_p, t>>4);
		m_p++;
		t=(t<<8)&0xf00;
		m_icount--;
	}
}

// q=reg+1 sideeffect
void sc61860_device::sc61860_inc_load_dp(int reg)
{
	uint8_t t = READ_RAM(reg) + 1;
	uint8_t t2 = READ_RAM(reg + 1);
	WRITE_RAM(reg, t);
	if (t == 0) { t2++; WRITE_RAM(reg + 1, t2); }
	m_dp=t|(t2<<8);
	m_q=reg+1;
}

// q=reg+1 sideeffect
void sc61860_device::sc61860_dec_load_dp(int reg)
{
	uint8_t t = READ_RAM(reg) - 1;
	uint8_t t2 = READ_RAM(reg + 1);
	WRITE_RAM(reg, t);
	if (t == 0xff) { t2--; WRITE_RAM(reg + 1, t2); }
	m_dp=t|(t2<<8);
	m_q=reg+1;
}

// q=XH sideeffect
void sc61860_device::sc61860_inc_load_dp_load()
{
	sc61860_inc_load_dp(XL);
	WRITE_RAM(A, READ_BYTE(m_dp));
}

// q=XH sideeffect
void sc61860_device::sc61860_dec_load_dp_load()
{
	sc61860_dec_load_dp(XL);
	WRITE_RAM(A, READ_BYTE(m_dp));
}

// q=YH sideeffect
void sc61860_device::sc61860_inc_load_dp_store()
{
	sc61860_inc_load_dp(YL);
	WRITE_BYTE(m_dp, READ_RAM(A));
}

// q=YH sideeffect
void sc61860_device::sc61860_dec_load_dp_store()
{
	sc61860_dec_load_dp(YL);
	WRITE_BYTE(m_dp, READ_RAM(A));
}

void sc61860_device::sc61860_fill()
{
	int i;
	for (i=0;i<=READ_RAM(I);i++) {
		WRITE_RAM(m_p, READ_RAM(A)); /* could be overwritten? */
		m_p++;
		m_icount--;
	}
}

void sc61860_device::sc61860_fill_ext()
{
	int i;
	for (i=0;i<=READ_RAM(I);i++) {
		WRITE_BYTE(m_dp, READ_RAM(A));
		if (i!=READ_RAM(I)) m_dp++;
		m_icount-=3;
	}
}

// p+=count+1, q+=count+1 sideeffects
void sc61860_device::sc61860_copy(int count)
{
	int i;
	for (i=0; i<=count; i++) {
		WRITE_RAM(m_p, READ_RAM(m_q));
		m_p++;
		m_q++;
		m_icount-=2;
	}

}

// p+=count+1, dp+=count sideeffects
void sc61860_device::sc61860_copy_ext(int count)
{
	int i;
	for (i=0; i<=count; i++) {
		WRITE_RAM(m_p, READ_BYTE(m_dp));
		m_p++;
		if (i!=count) m_dp++;
		m_icount-=4;
	}
}

void sc61860_device::sc61860_copy_int(int count)
{
	int i;
	for (i=0; i<=count; i++) {
		uint8_t t = READ_BYTE((READ_RAM(A)|(READ_RAM(B)<<8))); /* internal rom! */
		WRITE_RAM(m_p, t);
		m_p++;
		if (i!=count) {
			t = READ_RAM(A) + 1;
			WRITE_RAM(A, t);
			if (t==0) {
				t = READ_RAM(B) + 1;
				WRITE_RAM(B, t);
			}
		}
		m_icount-=4;
	}
}

void sc61860_device::sc61860_exchange(int count)
{
	int i;
	uint8_t t;
	for (i=0; i<=count; i++) {
		t = READ_RAM(m_p);
		WRITE_RAM(m_p, READ_RAM(m_q));
		WRITE_RAM(m_q, t);
		m_p++;
		m_q++;
		m_icount-=3;
	}
}

void sc61860_device::sc61860_exchange_ext(int count)
{
	int i;
	uint8_t t;
	for (i=0; i<=count; i++) {
		t = READ_RAM(m_p);
		WRITE_RAM(m_p, READ_BYTE(m_dp));
		m_p++;
		WRITE_BYTE(m_dp, t);
		if (i!=count) m_dp++;
		m_icount-=6;
	}
}

// undocumented
// only 1 opcode working in pc1403
// both opcodes working in pc1350
void sc61860_device::sc61860_wait_x(int level)
{
	int c;
	m_zero=level;

	if (!m_x.isunset()) {
		for (c=READ_RAM(I); c>=0; c--) {
			uint8_t t = (READ_RAM(m_p)+1)&0x7f;
			WRITE_RAM(m_p, t);
			m_zero=m_x();
			m_icount-=4;
			if (level != m_zero) break;
		}
	}
}
