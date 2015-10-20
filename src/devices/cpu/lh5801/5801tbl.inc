// license:BSD-3-Clause
// copyright-holders:Peter Trauner
/* assumed
   res=left+right+c
   sbc is like adc with inverted carry and right side
   (decrement, compare the same)
   (like in the m6502 processors)
*/
UINT8 lh5801_cpu_device::lh5801_add_generic(int left, int right, int carry)
{
	int res=left+right+carry;
	int v,c;

	m_t&=~(H|V|Z|C);

	if (!(res&0xff)) m_t|=Z;
	c=res&0x100;
	if (c) m_t|=C;
	if (((left&0xf)+(right&0xf)+carry)&0x10) m_t|=H;
	v=((left&0x7f)+(right&0x7f)+carry)&0x80;
	if ( (c&&!v)||(!c&&v) ) m_t|=V;

	return res;
}

UINT16 lh5801_cpu_device::lh5801_readop_word()
{
	UINT16 r;
	r=m_direct->read_byte(P++)<<8;
	r|=m_direct->read_byte(P++);
	return r;
}


void lh5801_cpu_device::lh5801_adc(UINT8 data)
{
	m_a=lh5801_add_generic(m_a,data,m_t&C);
}

void lh5801_cpu_device::lh5801_add_mem(address_space &space, int addr, UINT8 data)
{
	int v=lh5801_add_generic(space.read_byte(addr),data,0);
	space.write_byte(addr,v);
}

void lh5801_cpu_device::lh5801_adr(PAIR *reg)
{
	reg->b.l=lh5801_add_generic(reg->b.l,m_a,0);
	if (m_t&C) {
		reg->b.h++;
	}
}

void lh5801_cpu_device::lh5801_sbc(UINT8 data)
{
	m_a=lh5801_add_generic(m_a,data^0xff,m_t&C);
}

void lh5801_cpu_device::lh5801_cpa(UINT8 a, UINT8 b)
{
	lh5801_add_generic(a, b^0xff, 1);
}

UINT8 lh5801_cpu_device::lh5801_decimaladd_generic(int left, int right, int carry)
{
	int res=lh5801_add_generic(left, right, carry);
	UINT8 da;

	//DA values taken from official documentation
	if (!(m_t&C) && !(m_t&H))
		da = 0x9a;
	else if (!(m_t&C) &&  (m_t&H))
		da = 0xa0;
	else if ((m_t&C) && !(m_t&H))
		da = 0xfa;
	else    //if ((m_t&C) && (m_t&H))
		da = 0x00;

	return res + da;
}

void lh5801_cpu_device::lh5801_dca(UINT8 data)
{
	m_a += 0x66;    //taken from official documentation
	m_a=lh5801_decimaladd_generic(m_a, data, m_t&C);
}

void lh5801_cpu_device::lh5801_dcs(UINT8 data)
{
	m_a=lh5801_decimaladd_generic(m_a, data^0xff, m_t&C);
}

void lh5801_cpu_device::lh5801_and(UINT8 data)
{
	m_a&=data;
	m_t&=~Z;
	if (!m_a) m_t|=Z;
}

void lh5801_cpu_device::lh5801_and_mem(address_space &space, int addr, UINT8 data)
{
	data&=space.read_byte(addr);
	m_t&=~Z;
	if (!data) m_t|=Z;
	space.write_byte(addr,data);
}

void lh5801_cpu_device::lh5801_bit(UINT8 a, UINT8 b)
{
	m_t&=~Z;
	if (!(a&b)) m_t|=Z;
}

void lh5801_cpu_device::lh5801_eor(UINT8 data)
{
	m_a^=data;
	m_t&=~Z;
	if (!m_a) m_t|=Z;
}

void lh5801_cpu_device::lh5801_ora(UINT8 data)
{
	m_a|=data;
	m_t&=~Z;
	if (!m_a) m_t|=Z;
}

void lh5801_cpu_device::lh5801_ora_mem(address_space &space, int addr, UINT8 data)
{
	data|=space.read_byte(addr);
	m_t&=~Z;
	if (!data) m_t|=Z;
	space.write_byte(addr,data);
}

void lh5801_cpu_device::lh5801_lda(UINT8 data)
{
	m_a=data;
	m_t&=~Z;
	if (!m_a) m_t|=Z;
}

void lh5801_cpu_device::lh5801_lde(PAIR *reg)
{
	// or z flag depends on reg
	m_a=m_program->read_byte(reg->w.l--);
	m_t&=~Z;
	if (!m_a) m_t|=Z;
}

void lh5801_cpu_device::lh5801_sde(PAIR *reg)
{
	m_program->write_byte(reg->w.l--, m_a);
}

void lh5801_cpu_device::lh5801_lin(PAIR *reg)
{
	// or z flag depends on reg
	m_a=m_program->read_byte(reg->w.l++);
	m_t&=~Z;
	if (!m_a) m_t|=Z;
}

void lh5801_cpu_device::lh5801_sin(PAIR *reg)
{
	m_program->write_byte(reg->w.l++, m_a);
}

void lh5801_cpu_device::lh5801_dec(UINT8 *adr)
{
	*adr=lh5801_add_generic(*adr,0xff,0);
}

void lh5801_cpu_device::lh5801_inc(UINT8 *adr)
{
	*adr=lh5801_add_generic(*adr,1,0);
}

void lh5801_cpu_device::lh5801_pop()
{
	m_a=m_program->read_byte(++S);
	m_t&=~Z;
	if (!m_a) m_t|=Z;
}

void lh5801_cpu_device::lh5801_pop_word(PAIR *reg)
{
	reg->b.h=m_program->read_byte(++S);
	reg->b.l=m_program->read_byte(++S);
	// z flag?
}

void lh5801_cpu_device::lh5801_rtn()
{
	P=m_program->read_byte(++S)<<8;
	P|=m_program->read_byte(++S);
}

void lh5801_cpu_device::lh5801_rti()
{
	P=m_program->read_byte(++S)<<8;
	P|=m_program->read_byte(++S);
	m_t=m_program->read_byte(++S);
}

void lh5801_cpu_device::lh5801_push(UINT8 data)
{
	m_program->write_byte(S--, data);
}

void lh5801_cpu_device::lh5801_push_word(UINT16 data)
{
	m_program->write_byte(S--, data&0xff);
	m_program->write_byte(S--, data>>8);
}

void lh5801_cpu_device::lh5801_jmp(UINT16 adr)
{
	P=adr;
}

void lh5801_cpu_device::lh5801_branch_plus(int doit)
{
	UINT8 t=m_direct->read_byte(P++);
	if (doit) {
		m_icount-=3;
		P+=t;
	}
}

void lh5801_cpu_device::lh5801_branch_minus(int doit)
{
	UINT8 t=m_direct->read_byte(P++);
	if (doit) {
		m_icount-=3;
		P-=t;
	}
}

void lh5801_cpu_device::lh5801_lop()
{
	UINT8 t=m_direct->read_byte(P++);
	m_icount-=8;
	if (UL--) {
		m_icount-=3;
		P-=t;
	}
}

void lh5801_cpu_device::lh5801_sjp()
{
	UINT16 n=lh5801_readop_word();
	lh5801_push_word(P);
	P=n;
}

void lh5801_cpu_device::lh5801_vector(int doit, int nr)
{
	if (doit) {
		lh5801_push_word(P);
		P=m_program->read_byte(0xff00+nr)<<8;
		P|=m_program->read_byte(0xff00+nr+1);
		m_icount-=21-8;
	}
	m_t&=~Z; // after the jump!?
}

void lh5801_cpu_device::lh5801_aex()
{
	UINT8 t=m_a;
	m_a=(t<<4)|(t>>4);
	// flags?
}

void lh5801_cpu_device::lh5801_drl(address_space &space, int adr)
{
	UINT16 t=m_a|(space.read_byte(adr)<<8);

	m_a=t>>8;
	space.write_byte(adr,t>>4);
}

void lh5801_cpu_device::lh5801_drr(address_space &space, int adr)
{
	UINT16 t=space.read_byte(adr)|(m_a<<8);

	m_a=t;
	space.write_byte(adr,t>>4);
}

void lh5801_cpu_device::lh5801_rol()
{
	// maybe use of the adder
	int n = m_a;
	m_a = (n<<1) | (m_t&C);
	// flags cvhz
	m_t&=~(H|V|Z|C);
	if (n&0x80) m_t|=C;
	if (!m_a) m_t|=Z;
	if (m_a & 0x10) m_t|=H;
	if ((BIT(n,6) && !BIT(n,7)) || (!BIT(n,6) && BIT(n,7))) m_t|=V;
}

void lh5801_cpu_device::lh5801_ror()
{
	int n = m_a;
	m_a=(n | ((m_t&C)<<8))>>1;
	// flags cvhz
	m_t&=~(H|V|Z|C);
	if (n&0x01) m_t|=C;
	if (!m_a) m_t|=Z;
	if (m_a & 0x08) m_t|=H;
	if ((BIT(n,0) && BIT(m_a,1)) || (BIT(m_a,0) && BIT(n,1))) m_t|=V;
}

void lh5801_cpu_device::lh5801_shl()
{
	int n = m_a;
	m_a<<=1;
	// flags cvhz
	m_t&=~(H|V|Z|C);
	if (n&0x80) m_t|=C;
	if (!m_a) m_t|=Z;
	if (m_a & 0x10) m_t|=H;
	if ((BIT(n,6) && !BIT(n,7)) || (!BIT(n,6) && BIT(n,7))) m_t|=V;
}

void lh5801_cpu_device::lh5801_shr()
{
	int n = m_a;
	m_a>>=1;
	// flags cvhz
	m_t&=~(H|V|Z|C);
	if (n & 0x01) m_t|=C;
	if (!m_a) m_t|=Z;
	if (m_a & 0x08) m_t|=H;
	if ((BIT(n,0) && BIT(m_a,1)) || (BIT(m_a,0) && BIT(n,1))) m_t|=V;
}

void lh5801_cpu_device::lh5801_am(int value)
{
	m_tm=value;
	// jfkas?jfkl?jkd?
}

void lh5801_cpu_device::lh5801_ita()
{
	m_a=m_in_func();
	m_t&=~Z;
	if (!m_a) m_t|=Z;
}

void lh5801_cpu_device::lh5801_instruction_fd()
{
	int oper;
	int adr;

	oper=m_direct->read_byte(P++);
	switch (oper) {
	case 0x01: lh5801_sbc(m_io->read_byte(X)); m_icount-=11;break;
	case 0x03: lh5801_adc(m_io->read_byte(X)); m_icount-=11;break;
	case 0x05: lh5801_lda(m_io->read_byte(X)); m_icount-=10;break;
	case 0x07: lh5801_cpa(m_a, m_io->read_byte(X)); m_icount-=11;break;
	case 0x08: X=X;m_icount-=11;break; //!!!
	case 0x09: lh5801_and(m_io->read_byte(X)); m_icount-=11;break;
	case 0x0a: lh5801_pop_word(&m_x); m_icount-=15;break;
	case 0x0b: lh5801_ora(m_io->read_byte(X)); m_icount-=11;break;
	case 0x0c: lh5801_dcs(m_io->read_byte(X)); m_icount-=17; break;
	case 0x0d: lh5801_eor(m_io->read_byte(X)); m_icount-=11;break;
	case 0x0e: m_io->write_byte(X,m_a); m_icount-=10;break;
	case 0x0f: lh5801_bit(m_io->read_byte(X),m_a); m_icount-=11;break;
	case 0x11: lh5801_sbc(m_io->read_byte(Y)); m_icount-=11;break;
	case 0x13: lh5801_adc(m_io->read_byte(Y)); m_icount-=11;break;
	case 0x15: lh5801_lda(m_io->read_byte(Y)); m_icount-=10;break;
	case 0x17: lh5801_cpa(m_a, m_io->read_byte(Y)); m_icount-=11;break;
	case 0x18: X=Y;m_icount-=11;break;
	case 0x19: lh5801_and(m_io->read_byte(Y)); m_icount-=11;break;
	case 0x1a: lh5801_pop_word(&m_y); m_icount-=15;break;
	case 0x1b: lh5801_ora(m_io->read_byte(Y)); m_icount-=11;break;
	case 0x1c: lh5801_dcs(m_io->read_byte(Y)); m_icount-=17; break;
	case 0x1d: lh5801_eor(m_io->read_byte(Y)); m_icount-=11;break;
	case 0x1e: m_io->write_byte(Y,m_a); m_icount-=10;break;
	case 0x1f: lh5801_bit(m_io->read_byte(Y),m_a); m_icount-=11;break;
	case 0x21: lh5801_sbc(m_io->read_byte(U)); m_icount-=11;break;
	case 0x23: lh5801_adc(m_io->read_byte(U)); m_icount-=11;break;
	case 0x25: lh5801_lda(m_io->read_byte(U)); m_icount-=10;break;
	case 0x27: lh5801_cpa(m_a, m_io->read_byte(U)); m_icount-=11;break;
	case 0x28: X=U;m_icount-=11;break;
	case 0x29: lh5801_and(m_io->read_byte(U)); m_icount-=11;break;
	case 0x2a: lh5801_pop_word(&m_u); m_icount-=15;break;
	case 0x2b: lh5801_ora(m_io->read_byte(U)); m_icount-=11;break;
	case 0x2c: lh5801_dcs(m_io->read_byte(U)); m_icount-=17; break;
	case 0x2d: lh5801_eor(m_io->read_byte(U)); m_icount-=11;break;
	case 0x2e: m_io->write_byte(U,m_a); m_icount-=10;break;
	case 0x2f: lh5801_bit(m_io->read_byte(U),m_a); m_icount-=11;break;
	case 0x40: lh5801_inc(&XH);m_icount-=9;break;
	case 0x42: lh5801_dec(&XH);m_icount-=9;break;
	case 0x48: X=S;m_icount-=11;break;
	case 0x49: lh5801_and_mem(*m_io, X, m_direct->read_byte(P++)); m_icount-=17;break;
	case 0x4a: X=X;m_icount-=11;break; //!!!
	case 0x4b: lh5801_ora_mem(*m_io, X, m_direct->read_byte(P++)); m_icount-=17;break;
	case 0x4c: m_bf=0;/*off !*/ m_icount-=8;break;
	case 0x4d: lh5801_bit(m_io->read_byte(X), m_direct->read_byte(P++));m_icount-=14;break;
	case 0x4e: S=X;m_icount-=11;break;
	case 0x4f: lh5801_add_mem(*m_io, X, m_direct->read_byte(P++)); m_icount-=17;break;
	case 0x50: lh5801_inc(&YH);m_icount-=9;break;
	case 0x52: lh5801_dec(&YH);m_icount-=9;break;
	case 0x58: X=P;m_icount-=11;break;
	case 0x59: lh5801_and_mem(*m_io, Y, m_direct->read_byte(P++)); m_icount-=17;break;
	case 0x5a: Y=X;m_icount-=11;break;
	case 0x5b: lh5801_ora_mem(*m_io, Y, m_direct->read_byte(P++)); m_icount-=17;break;
	case 0x5d: lh5801_bit(m_io->read_byte(Y), m_direct->read_byte(P++));m_icount-=14;break;
	case 0x5e: lh5801_jmp(X);m_icount-=11;break; // P=X
	case 0x5f: lh5801_add_mem(*m_io, Y, m_direct->read_byte(P++)); m_icount-=17;break;
	case 0x60: lh5801_inc(&UH);m_icount-=9;break;
	case 0x62: lh5801_dec(&UH);m_icount-=9;break;
	case 0x69: lh5801_and_mem(*m_io, U, m_direct->read_byte(P++)); m_icount-=17;break;
	case 0x6a: U=X;m_icount-=11;break;
	case 0x6b: lh5801_ora_mem(*m_io, U, m_direct->read_byte(P++)); m_icount-=17;break;
	case 0x6d: lh5801_bit(m_io->read_byte(X), m_direct->read_byte(P++));m_icount-=14;break;
	case 0x6f: lh5801_add_mem(*m_io, U, m_direct->read_byte(P++)); m_icount-=17;break;
	case 0x81: m_t|=IE; /*sie !*/m_icount-=8;break;
	case 0x88: lh5801_push_word(X); m_icount-=14;break;
	case 0x8a: lh5801_pop(); m_icount-=12; break;
	case 0x8c: lh5801_dca(m_io->read_byte(X)); m_icount-=19; break;
	case 0x8e: /*cdv clears internal devider*/m_icount-=8;break;
	case 0x98: lh5801_push_word(Y); m_icount-=14;break;
	case 0x9c: lh5801_dca(m_io->read_byte(Y)); m_icount-=19; break;
	case 0xa1: lh5801_sbc(m_io->read_byte(lh5801_readop_word())); m_icount-=17;break;
	case 0xa3: lh5801_adc(m_io->read_byte(lh5801_readop_word())); m_icount-=17;break;
	case 0xa5: lh5801_lda(m_io->read_byte(lh5801_readop_word())); m_icount-=16;break;
	case 0xa7: lh5801_cpa(m_a, m_io->read_byte(lh5801_readop_word())); m_icount-=17;break;
	case 0xa8: lh5801_push_word(U); m_icount-=14;break;
	case 0xa9: lh5801_and(m_io->read_byte(lh5801_readop_word())); m_icount-=17;break;
	case 0xaa: lh5801_lda(m_t); m_icount-=9;break;
	case 0xab: lh5801_ora(m_io->read_byte(lh5801_readop_word())); m_icount-=17;break;
	case 0xac: lh5801_dca(m_io->read_byte(U)); m_icount-=19; break;
	case 0xad: lh5801_eor(m_io->read_byte(lh5801_readop_word())); m_icount-=17;break;
	case 0xae: m_io->write_byte(lh5801_readop_word(),m_a); m_icount-=16;break;
	case 0xaf: lh5801_bit(m_io->read_byte(lh5801_readop_word()),m_a); m_icount-=17;break;
	case 0xb1: /*hlt*/m_icount-=8;break;
	case 0xba: lh5801_ita();m_icount-=9;break;
	case 0xbe: m_t&=~IE; /*rie !*/m_icount-=8;break;
	case 0xc0: m_dp=0; /*rdp !*/m_icount-=8;break;
	case 0xc1: m_dp=1; /*sdp !*/m_icount-=8;break;
	case 0xc8: lh5801_push(m_a); m_icount-=11;break;
	case 0xca: lh5801_adr(&m_x);m_icount-=11;break;
	case 0xcc: /*atp sends a to data bus*/m_icount-=9;break;
	case 0xce: lh5801_am(m_a); m_icount-=9; break;
	case 0xd3: lh5801_drr(*m_io, X); m_icount-=16; break;
	case 0xd7: lh5801_drl(*m_io, X); m_icount-=16; break;
	case 0xda: lh5801_adr(&m_y);m_icount-=11;break;
	case 0xde: lh5801_am(m_a|0x100); m_icount-=9; break;
	case 0xea: lh5801_adr(&m_u);m_icount-=11;break;
	case 0xe9:
		adr=lh5801_readop_word();
		lh5801_and_mem(*m_io, adr, m_direct->read_byte(P++)); m_icount-=23;
		break;
	case 0xeb:
		adr=lh5801_readop_word();
		lh5801_ora_mem(*m_io, adr, m_direct->read_byte(P++)); m_icount-=23;
		break;
	case 0xec: m_t=m_a; m_icount-=9;break;
	case 0xed:
		adr=lh5801_readop_word();
		lh5801_bit(m_io->read_byte(adr), m_direct->read_byte(P++));
		m_icount-=20;break;
	case 0xef:
		adr=lh5801_readop_word();
		lh5801_add_mem(*m_io, adr, m_direct->read_byte(P++)); m_icount-=23;
		break;

	default:
		logerror("lh5801 illegal opcode at %.4x fd%.2x\n",P-2, oper);
	}
}

void lh5801_cpu_device::lh5801_instruction()
{
	int oper;
	int adr;

	oper=m_direct->read_byte(P++);
	switch (oper) {
	case 0x00: lh5801_sbc(XL); m_icount-=6;break;
	case 0x01: lh5801_sbc(m_program->read_byte(X)); m_icount-=7;break;
	case 0x02: lh5801_adc(XL); m_icount-=6;break;
	case 0x03: lh5801_adc(m_program->read_byte(X)); m_icount-=7;break;
	case 0x04: lh5801_lda(XL); m_icount-=5;break;
	case 0x05: lh5801_lda(m_program->read_byte(X)); m_icount-=6;break;
	case 0x06: lh5801_cpa(m_a, XL); m_icount-=6;break;
	case 0x07: lh5801_cpa(m_a, m_program->read_byte(X)); m_icount-=7;break;
	case 0x08: XH=m_a; m_icount-=5; break;
	case 0x09: lh5801_and(m_program->read_byte(X)); m_icount-=7;break;
	case 0x0a: XL=m_a; m_icount-=5; break;
	case 0x0b: lh5801_ora(m_program->read_byte(X)); m_icount-=7;break;
	case 0x0c: lh5801_dcs(m_program->read_byte(X)); m_icount-=13; break;
	case 0x0d: lh5801_eor(m_program->read_byte(X)); m_icount-=7;break;
	case 0x0e: m_program->write_byte(X,m_a); m_icount-=6;break;
	case 0x0f: lh5801_bit(m_program->read_byte(X),m_a); m_icount-=7;break;
	case 0x10: lh5801_sbc(YL); m_icount-=6;break;
	case 0x11: lh5801_sbc(m_program->read_byte(Y)); m_icount-=7;break;
	case 0x12: lh5801_adc(YL); m_icount-=6;break;
	case 0x13: lh5801_adc(m_program->read_byte(Y)); m_icount-=7;break;
	case 0x14: lh5801_lda(YL); m_icount-=5;break;
	case 0x15: lh5801_lda(m_program->read_byte(Y)); m_icount-=6;break;
	case 0x16: lh5801_cpa(m_a, YL); m_icount-=6;break;
	case 0x17: lh5801_cpa(m_a, m_program->read_byte(Y)); m_icount-=7;break;
	case 0x18: YH=m_a; m_icount-=5; break;
	case 0x19: lh5801_and(m_program->read_byte(Y)); m_icount-=7;break;
	case 0x1a: YL=m_a; m_icount-=5; break;
	case 0x1b: lh5801_ora(m_program->read_byte(Y)); m_icount-=7;break;
	case 0x1c: lh5801_dcs(m_program->read_byte(Y)); m_icount-=13; break;
	case 0x1d: lh5801_eor(m_program->read_byte(Y)); m_icount-=7;break;
	case 0x1e: m_program->write_byte(Y,m_a); m_icount-=6;break;
	case 0x1f: lh5801_bit(m_program->read_byte(Y),m_a); m_icount-=7;break;
	case 0x20: lh5801_sbc(UL); m_icount-=6;break;
	case 0x21: lh5801_sbc(m_program->read_byte(U)); m_icount-=7;break;
	case 0x22: lh5801_adc(UL); m_icount-=6;break;
	case 0x23: lh5801_adc(m_program->read_byte(U)); m_icount-=7;break;
	case 0x24: lh5801_lda(UL); m_icount-=5;break;
	case 0x25: lh5801_lda(m_program->read_byte(U)); m_icount-=6;break;
	case 0x26: lh5801_cpa(m_a, UL); m_icount-=6;break;
	case 0x27: lh5801_cpa(m_a, m_program->read_byte(U)); m_icount-=7;break;
	case 0x28: UH=m_a; m_icount-=5; break;
	case 0x29: lh5801_and(m_program->read_byte(U)); m_icount-=7;break;
	case 0x2a: UL=m_a; m_icount-=5; break;
	case 0x2b: lh5801_ora(m_program->read_byte(U)); m_icount-=7;break;
	case 0x2c: lh5801_dcs(m_program->read_byte(U)); m_icount-=13; break;
	case 0x2d: lh5801_eor(m_program->read_byte(U)); m_icount-=7;break;
	case 0x2e: m_program->write_byte(U,m_a); m_icount-=6;break;
	case 0x2f: lh5801_bit(m_program->read_byte(U),m_a); m_icount-=7;break;
	case 0x38: /*nop*/m_icount-=5;break;
	case 0x40: lh5801_inc(&XL);m_icount-=5;break;
	case 0x41: lh5801_sin(&m_x); m_icount-=6;break;
	case 0x42: lh5801_dec(&XL);m_icount-=5;break;
	case 0x43: lh5801_sde(&m_x); m_icount-=6;break;
	case 0x44: X++;m_icount-=5;break;
	case 0x45: lh5801_lin(&m_x);m_icount-=6;break;
	case 0x46: X--;m_icount-=5;break;
	case 0x47: lh5801_lde(&m_x);m_icount-=6;break;
	case 0x48: XH=m_direct->read_byte(P++);m_icount-=6;break;
	case 0x49: lh5801_and_mem(*m_program, X, m_direct->read_byte(P++)); m_icount-=13;break;
	case 0x4a: XL=m_direct->read_byte(P++);m_icount-=6;break;
	case 0x4b: lh5801_ora_mem(*m_program, X, m_direct->read_byte(P++)); m_icount-=13;break;
	case 0x4c: lh5801_cpa(XH, m_direct->read_byte(P++)); m_icount-=7;break;
	case 0x4d: lh5801_bit(m_program->read_byte(X), m_direct->read_byte(P++));m_icount-=10;break;
	case 0x4e: lh5801_cpa(XL, m_direct->read_byte(P++)); m_icount-=7;break;
	case 0x4f: lh5801_add_mem(*m_program, X, m_direct->read_byte(P++)); m_icount-=13;break;
	case 0x50: lh5801_inc(&YL);m_icount-=5;break;
	case 0x51: lh5801_sin(&m_y); m_icount-=6;break;
	case 0x52: lh5801_dec(&YL);m_icount-=5;break;
	case 0x53: lh5801_sde(&m_y); m_icount-=6;break;
	case 0x54: Y++;m_icount-=5;break;
	case 0x55: lh5801_lin(&m_y);m_icount-=6;break;
	case 0x56: Y--;m_icount-=5;break;
	case 0x57: lh5801_lde(&m_y);m_icount-=6;break;
	case 0x58: YH=m_direct->read_byte(P++);m_icount-=6;break;
	case 0x59: lh5801_and_mem(*m_program, Y, m_direct->read_byte(P++)); m_icount-=13;break;
	case 0x5a: YL=m_direct->read_byte(P++);m_icount-=6;break;
	case 0x5b: lh5801_ora_mem(*m_program, Y, m_direct->read_byte(P++)); m_icount-=13;break;
	case 0x5c: lh5801_cpa(YH, m_direct->read_byte(P++)); m_icount-=7;break;
	case 0x5d: lh5801_bit(m_program->read_byte(Y), m_direct->read_byte(P++));m_icount-=10;break;
	case 0x5e: lh5801_cpa(YL, m_direct->read_byte(P++)); m_icount-=7;break;
	case 0x5f: lh5801_add_mem(*m_program, Y, m_direct->read_byte(P++)); m_icount-=13;break;
	case 0x60: lh5801_inc(&UL);m_icount-=5;break;
	case 0x61: lh5801_sin(&m_u); m_icount-=6;break;
	case 0x62: lh5801_dec(&UL);m_icount-=5;break;
	case 0x63: lh5801_sde(&m_u); m_icount-=6;break;
	case 0x64: U++;m_icount-=5;break;
	case 0x65: lh5801_lin(&m_u);m_icount-=6;break;
	case 0x66: U--;m_icount-=5;break;
	case 0x67: lh5801_lde(&m_u);m_icount-=6;break;
	case 0x68: UH=m_direct->read_byte(P++);m_icount-=6;break;
	case 0x69: lh5801_and_mem(*m_program, U, m_direct->read_byte(P++)); m_icount-=13;break;
	case 0x6a: UL=m_direct->read_byte(P++);m_icount-=6;break;
	case 0x6b: lh5801_ora_mem(*m_program, U, m_direct->read_byte(P++)); m_icount-=13;break;
	case 0x6c: lh5801_cpa(UH, m_direct->read_byte(P++)); m_icount-=7;break;
	case 0x6d: lh5801_bit(m_program->read_byte(U), m_direct->read_byte(P++));m_icount-=10;break;
	case 0x6e: lh5801_cpa(UL, m_direct->read_byte(P++)); m_icount-=7;break;
	case 0x6f: lh5801_add_mem(*m_program, U, m_direct->read_byte(P++)); m_icount-=13;break;
	case 0x80: lh5801_sbc(XH); m_icount-=6;break;
	case 0x81: lh5801_branch_plus(!(m_t&C)); m_icount-=8; break;
	case 0x82: lh5801_adc(XH); m_icount-=6;break;
	case 0x83: lh5801_branch_plus(m_t&C); m_icount-=8; break;
	case 0x84: lh5801_lda(XH); m_icount-=5;break;
	case 0x85: lh5801_branch_plus(!(m_t&H)); m_icount-=8; break;
	case 0x86: lh5801_cpa(m_a, XH); m_icount-=6;break;
	case 0x87: lh5801_branch_plus(m_t&H); m_icount-=8; break;
	case 0x88: lh5801_lop(); break;
	case 0x89: lh5801_branch_plus(!(m_t&Z)); m_icount-=8; break;
	case 0x8a: lh5801_rti(); m_icount-=14; break;
	case 0x8b: lh5801_branch_plus(m_t&Z); m_icount-=8; break;
	case 0x8c: lh5801_dca(m_program->read_byte(X)); m_icount-=15; break;
	case 0x8d: lh5801_branch_plus(!(m_t&V)); m_icount-=8; break;
	case 0x8e: lh5801_branch_plus(1); m_icount-=5; break;
	case 0x8f: lh5801_branch_plus(m_t&V); m_icount-=8; break;
	case 0x90: lh5801_sbc(YH); m_icount-=6;break;
	case 0x91: lh5801_branch_minus(!(m_t&C)); m_icount-=8; break;
	case 0x92: lh5801_adc(YH); m_icount-=6;break;
	case 0x93: lh5801_branch_minus(m_t&C); m_icount-=8; break;
	case 0x94: lh5801_lda(YH); m_icount-=5;break;
	case 0x95: lh5801_branch_minus(!(m_t&H)); m_icount-=8; break;
	case 0x96: lh5801_cpa(m_a, YH); m_icount-=6;break;
	case 0x97: lh5801_branch_minus(m_t&H); m_icount-=8; break;
	case 0x99: lh5801_branch_minus(!(m_t&Z)); m_icount-=8; break;
	case 0x9a: lh5801_rtn(); m_icount-=11; break;
	case 0x9b: lh5801_branch_minus(m_t&Z); m_icount-=8; break;
	case 0x9c: lh5801_dca(m_program->read_byte(Y)); m_icount-=15; break;
	case 0x9d: lh5801_branch_minus(!(m_t&V)); m_icount-=8; break;
	case 0x9e: lh5801_branch_minus(1); m_icount-=6; break;
	case 0x9f: lh5801_branch_minus(m_t&V); m_icount-=8; break;
	case 0xa0: lh5801_sbc(UH); m_icount-=6;break;
	case 0xa2: lh5801_adc(UH); m_icount-=6;break;
	case 0xa1: lh5801_sbc(m_program->read_byte(lh5801_readop_word())); m_icount-=13;break;
	case 0xa3: lh5801_adc(m_program->read_byte(lh5801_readop_word())); m_icount-=13;break;
	case 0xa4: lh5801_lda(UH); m_icount-=5;break;
	case 0xa5: lh5801_lda(m_program->read_byte(lh5801_readop_word())); m_icount-=12;break;
	case 0xa6: lh5801_cpa(m_a, UH); m_icount-=6;break;
	case 0xa7: lh5801_cpa(m_a, m_program->read_byte(lh5801_readop_word())); m_icount-=13;break;
	case 0xa8: m_pv=1;/*spv!*/ m_icount-=4; break;
	case 0xa9: lh5801_and(m_program->read_byte(lh5801_readop_word())); m_icount-=13;break;
	case 0xaa: S=lh5801_readop_word();m_icount-=6;break;
	case 0xab: lh5801_ora(m_program->read_byte(lh5801_readop_word())); m_icount-=13;break;
	case 0xac: lh5801_dca(m_program->read_byte(U)); m_icount-=15; break;
	case 0xad: lh5801_eor(m_program->read_byte(lh5801_readop_word())); m_icount-=13;break;
	case 0xae: m_program->write_byte(lh5801_readop_word(),m_a); m_icount-=12;break;
	case 0xaf: lh5801_bit(m_program->read_byte(lh5801_readop_word()),m_a); m_icount-=13;break;
	case 0xb1: lh5801_sbc(m_direct->read_byte(P++)); m_icount-=7;break;
	case 0xb3: lh5801_adc(m_direct->read_byte(P++)); m_icount-=7;break;
	case 0xb5: lh5801_lda(m_direct->read_byte(P++)); m_icount-=6;break;
	case 0xb7: lh5801_cpa(m_a, m_direct->read_byte(P++)); m_icount-=7;break;
	case 0xb8: m_pv=0;/*rpv!*/ m_icount-=4; break;
	case 0xb9: lh5801_and(m_direct->read_byte(P++)); m_icount-=7;break;
	case 0xba: lh5801_jmp(lh5801_readop_word()); m_icount-=12;break;
	case 0xbb: lh5801_ora(m_direct->read_byte(P++)); m_icount-=7;break;
	case 0xbd: lh5801_eor(m_direct->read_byte(P++)); m_icount-=7;break;
	case 0xbe: lh5801_sjp(); m_icount-=19; break;
	case 0xbf: lh5801_bit(m_a, m_direct->read_byte(P++));m_icount-=7;break;
	case 0xc1: lh5801_vector(!(m_t&C), m_direct->read_byte(P++)); m_icount-=8;break;
	case 0xc3: lh5801_vector(m_t&C, m_direct->read_byte(P++)); m_icount-=8;break;
	case 0xc5: lh5801_vector(!(m_t&H), m_direct->read_byte(P++)); m_icount-=8;break;
	case 0xc7: lh5801_vector(m_t&H, m_direct->read_byte(P++)); m_icount-=8;break;
	case 0xc9: lh5801_vector(!(m_t&Z), m_direct->read_byte(P++)); m_icount-=8;break;
	case 0xcb: lh5801_vector(m_t&Z, m_direct->read_byte(P++)); m_icount-=8;break;
	case 0xcd: lh5801_vector(1, m_direct->read_byte(P++)); m_icount-=7;break;
	case 0xcf: lh5801_vector(m_t&V, m_direct->read_byte(P++)); m_icount-=8;break;
	case 0xd1: lh5801_ror(); m_icount-=6; break;
	case 0xd3: lh5801_drr(*m_program, X); m_icount-=12; break;
	case 0xd5: lh5801_shr(); m_icount-=6; break;
	case 0xd7: lh5801_drl(*m_program, X); m_icount-=12; break;
	case 0xd9: lh5801_shl(); m_icount-=6; break;
	case 0xdb: lh5801_rol(); m_icount-=6; break;
	case 0xdd: lh5801_inc(&m_a);m_icount-=5;break;
	case 0xdf: lh5801_dec(&m_a);m_icount-=5;break;
	case 0xe1: m_pu=1;/*spu!*/ m_icount-=4; break;
	case 0xe3: m_pu=0;/*rpu!*/ m_icount-=4; break;
	case 0xe9:
		adr=lh5801_readop_word();lh5801_and_mem(*m_program, adr, m_direct->read_byte(P++));
		m_icount-=19;break;
	case 0xeb:
		adr=lh5801_readop_word();lh5801_ora_mem(*m_program, adr, m_direct->read_byte(P++));
		m_icount-=19;break;
	case 0xed:
		adr=lh5801_readop_word();lh5801_bit(m_program->read_byte(adr), m_direct->read_byte(P++));
		m_icount-=16;break;
	case 0xef:
		adr=lh5801_readop_word();
		lh5801_add_mem(*m_program, adr, m_direct->read_byte(P++)); m_icount-=19;
		break;
	case 0xf1: lh5801_aex(); m_icount-=6; break;
	case 0xf5: m_program->write_byte(Y++, m_program->read_byte(X++)); m_icount-=7; break; //tin
	case 0xf7: lh5801_cpa(m_a, m_program->read_byte(X++));m_icount-=7; break; //cin
	case 0xf9: m_t&=~C;m_icount-=4;break;
	case 0xfb: m_t|=C;m_icount-=4;break;
	case 0xfd: lh5801_instruction_fd();break;
	case 0xc0: case 0xc2: case 0xc4: case 0xc6:
	case 0xc8: case 0xca: case 0xcc: case 0xce:
	case 0xd0: case 0xd2: case 0xd4: case 0xd6:
	case 0xd8: case 0xda: case 0xdc: case 0xde:
	case 0xe0: case 0xe2: case 0xe4: case 0xe6:
	case 0xe8: case 0xea: case 0xec: case 0xee:
	case 0xf0: case 0xf2: case 0xf4: case 0xf6:
		lh5801_vector(1, oper);m_icount-=4;break;
	default:
		logerror("lh5801 illegal opcode at %.4x %.2x\n",P-1, oper);
	}

}
