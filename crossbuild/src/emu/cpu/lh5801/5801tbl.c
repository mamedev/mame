/* assumed
   res=left+right+c
   sbc is like adc with inverted carry and right side
   (decrement, compare the same)
   (like in the m6502 processors)
*/
INLINE UINT8 lh5801_add_generic(int left, int right, int carry)
{
	int res=left+right+carry;
	int v,c;

	lh5801.t&=~(H|V|Z|C);

	if (!(res&0xff)) lh5801.t|=Z;
	c=res&0x100;
	if (c) lh5801.t|=C;
	if (((left&0xf)+(right&0xf)+carry)&0x10) lh5801.t|=H;
	v=((left&0x7f)+(right&0x7f)+carry)&0x80;
	if ( (c&&!v)||(!c&&v) ) lh5801.t|=V;

	return res;
}

INLINE UINT16 lh5801_readop_word(void)
{
	UINT16 r;
	r=cpu_readop(P++)<<8;
	r|=cpu_readop(P++);
	return r;
}


INLINE void lh5801_adc(UINT8 data)
{
	lh5801.a=lh5801_add_generic(lh5801.a,data,lh5801.t&C);
}

INLINE void lh5801_add_mem(int addr, UINT8 data)
{
	int v=lh5801_add_generic(program_read_byte(addr),data,0);
	program_write_byte(addr,v);
}

INLINE void lh5801_adr(PAIR *reg)
{
	reg->b.l=lh5801_add_generic(reg->b.l,lh5801.a,0);
	if (lh5801.t&C) {
		reg->b.h++;
	}
}

INLINE void lh5801_sbc(UINT8 data)
{
	lh5801.a=lh5801_add_generic(lh5801.a,data^0xff,(lh5801.t&C)^1);
}

INLINE void lh5801_cpa(UINT8 a, UINT8 b)
{
	lh5801_add_generic(a, b^0xff, 1);
}

INLINE UINT8 lh5801_decimaladd_generic(int left, int right, int carry)
{
	int res=(left&0xf)+(right&0xf)+carry;
	lh5801.t&=~(H|V|Z|C);

	if (res>=10) {
		res+=6;
		lh5801.t|=H;
	}
	res+=(left&0xf0)+(right&0xf0);
	if (res>=0xa0) {
		res+=0x60;
		lh5801.t|=C;
	}
	if (!(res&0xff)) lh5801.t|=Z;
	//v???
	return res;
}

INLINE void lh5801_dca(UINT8 data)
{
	lh5801.a=lh5801_decimaladd_generic(lh5801.a, data, lh5801.t&C);
}

INLINE void lh5801_dcs(UINT8 data)
{
	lh5801.a=lh5801_decimaladd_generic(lh5801.a, data^0xff, (lh5801.t&C)^1);
}

INLINE void lh5801_and(UINT8 data)
{
	lh5801.a&=data;
	lh5801.t&=~Z;
	if (!lh5801.a) lh5801.t|=Z;
}

INLINE void lh5801_and_mem(int addr, UINT8 data)
{
	data&=program_read_byte(addr);
	lh5801.t&=~Z;
	if (!data) lh5801.t|=Z;
	program_write_byte(addr,data);
}

INLINE void lh5801_bit(UINT8 a, UINT8 b)
{
	lh5801.t&=~Z;
	if (!(a&b)) lh5801.t|=Z;
}

INLINE void lh5801_eor(UINT8 data)
{
	lh5801.a^=data;
	lh5801.t&=~Z;
	if (!lh5801.a) lh5801.t|=Z;
}

INLINE void lh5801_ora(UINT8 data)
{
	lh5801.a^=data;
	lh5801.t&=~Z;
	if (!lh5801.a) lh5801.t|=Z;
}

INLINE void lh5801_ora_mem(int addr, UINT8 data)
{
	data|=program_read_byte(addr);
	lh5801.t&=~Z;
	if (!data) lh5801.t|=Z;
	program_write_byte(addr,data);
}

INLINE void lh5801_lda(UINT8 data)
{
	lh5801.a=data;
	lh5801.t&=~Z;
	if (!lh5801.a) lh5801.t|=Z;
}

INLINE void lh5801_lde(PAIR *reg)
{
	// or z flag depends on reg
	lh5801.a=program_read_byte(reg->w.l--);
	lh5801.t&=~Z;
	if (!lh5801.a) lh5801.t|=Z;
}

INLINE void lh5801_sde(PAIR *reg)
{
	program_write_byte(reg->w.l--, lh5801.a);
}

INLINE void lh5801_lin(PAIR *reg)
{
	// or z flag depends on reg
	lh5801.a=program_read_byte(reg->w.l++);
	lh5801.t&=~Z;
	if (!lh5801.a) lh5801.t|=Z;
}

INLINE void lh5801_sin(PAIR *reg)
{
	program_write_byte(reg->w.l++, lh5801.a);
}

INLINE void lh5801_dec(UINT8 *adr)
{
	*adr=lh5801_add_generic(*adr,0xff,0);
}

INLINE void lh5801_inc(UINT8 *adr)
{
	*adr=lh5801_add_generic(*adr,1,0);
}

INLINE void lh5801_pop(void)
{
	lh5801.a=program_read_byte(++S);
	lh5801.t&=~Z;
	if (!lh5801.a) lh5801.t|=Z;
}

INLINE void lh5801_pop_word(PAIR *reg)
{
	reg->b.h=program_read_byte(++S);
	reg->b.l=program_read_byte(++S);
	// z flag?
}

INLINE void lh5801_rtn(void)
{
	P=program_read_byte(++S)<<8;
	P|=program_read_byte(++S);
	change_pc(P);
}

INLINE void lh5801_rti(void)
{
	P=program_read_byte(++S)<<8;
	P|=program_read_byte(++S);
	change_pc(P);
	lh5801.t=program_read_byte(++S);
}

INLINE void lh5801_push(UINT8 data)
{
	program_write_byte(S--, data);
}

INLINE void lh5801_push_word(UINT16 data)
{
	program_write_byte(S--, data&0xff);
	program_write_byte(S--, data>>8);
}

INLINE void lh5801_jmp(UINT16 adr)
{
	P=adr;
	change_pc(P);
}

INLINE void lh5801_branch_plus(int doit)
{
	UINT8 t=cpu_readop(P++);
	if (doit) {
		lh5801_icount-=3;
		P+=t;
		change_pc(P);
	}
}

INLINE void lh5801_branch_minus(int doit)
{
	UINT8 t=cpu_readop(P++);
	if (doit) {
		lh5801_icount-=3;
		P-=t;
		change_pc(P);
	}
}

INLINE void lh5801_lop(void)
{
	UINT8 t=cpu_readop(P++);
	lh5801_icount-=8;
	if (UL--) {
		lh5801_icount-=3;
		P-=t;
		change_pc(P);
	}
}

INLINE void lh5801_sjp(void)
{
	UINT16 n=lh5801_readop_word();
	lh5801_push_word(P);
	P=n;
	change_pc(n);
}

INLINE void lh5801_vector(int doit, int nr)
{
	if (doit) {
		lh5801_push_word(P);
		P=program_read_byte(0xff00+nr)<<8;
		P|=program_read_byte(0xff00+nr+1);
		change_pc(P);
		lh5801_icount-=21-8;
	}
	lh5801.t&=~Z; // after the jump!?
}

INLINE void lh5801_aex(void)
{
	UINT8 t=lh5801.a;
	lh5801.a=(t<<4)|(t>>4);
	// flags?
}

INLINE void lh5801_drl(int adr)
{
	UINT16 t=lh5801.a|(program_read_byte(adr)<<8);

	lh5801.a=t>>8;
	program_write_byte(adr,t>>4);
}

INLINE void lh5801_drr(int adr)
{
	UINT16 t=program_read_byte(adr)|(lh5801.a<<8);

	lh5801.a=t;
	program_write_byte(adr,t>>4);
}

INLINE void lh5801_rol(void)
{
	// maybe use of the adder
	int n=(lh5801.a<<1)|(lh5801.t&C);
	lh5801.a=n;
	// flags cvhz
	lh5801.t&=~(C&Z);
	if (n&0x100) lh5801.t|=C;
	if (!(n&0xff)) lh5801.t|=Z;
}

INLINE void lh5801_ror(void)
{
	int n=lh5801.a|((lh5801.t&C)<<8);
	lh5801.a=n>>1;
	// flags cvhz
	lh5801.t&=~(C&Z);
	lh5801.t|=(n&C);
	if (!(n&0x1fe)) lh5801.t|=Z;
}

INLINE void lh5801_shl(void)
{
	int nc=lh5801.a&0x80;
	lh5801.a<<=1;
	// flags cvhz
	lh5801.t&=~(C&Z);
	if (nc) lh5801.t|=C;
	if (!lh5801.a) lh5801.t|=Z;
}

INLINE void lh5801_shr(void)
{
	int nc=lh5801.a&1;
	lh5801.a>>=1;
	// flags cvhz
	lh5801.t&=~(C|Z);
	if (nc) lh5801.t|=C;
	if (!lh5801.a) lh5801.t|=Z;
}

INLINE void lh5801_am(int value)
{
	lh5801.tm=value;
	// jfkas?jfkl?jkd?
}

INLINE void lh5801_ita(void)
{
	if (lh5801.config&&lh5801.config->in) {
		lh5801.a=lh5801.config->in();
	} else {
		lh5801.a=0;
	}
	lh5801.t&=~Z;
	if (!lh5801.a) lh5801.t|=Z;
}

static void lh5801_instruction_fd(void)
{
	int oper;
	int adr;

	oper=cpu_readop(P++);
	switch (oper) {
	case 0x01: lh5801_sbc(program_read_byte(0x10000|X)); lh5801_icount-=11;break;
	case 0x03: lh5801_adc(program_read_byte(0x10000|X)); lh5801_icount-=11;break;
	case 0x05: lh5801_lda(program_read_byte(0x10000|X)); lh5801_icount-=10;break;
	case 0x07: lh5801_cpa(lh5801.a, program_read_byte(0x10000|X)); lh5801_icount-=11;break;
	case 0x08: X=X;lh5801_icount-=11;break; //!!!
	case 0x09: lh5801_and(program_read_byte(0x10000|X)); lh5801_icount-=11;break;
	case 0x0a: lh5801_pop_word(&lh5801.x); lh5801_icount-=15;break;
	case 0x0b: lh5801_ora(program_read_byte(0x10000|X)); lh5801_icount-=11;break;
	case 0x0c: lh5801_dcs(program_read_byte(0x10000|X)); lh5801_icount-=17; break;
	case 0x0d: lh5801_eor(program_read_byte(0x10000|X)); lh5801_icount-=11;break;
	case 0x0e: program_write_byte(0x10000|X,lh5801.a); lh5801_icount-=10;break;
	case 0x0f: lh5801_bit(program_read_byte(0x10000|X),lh5801.a); lh5801_icount-=11;break;
	case 0x11: lh5801_sbc(program_read_byte(0x10000|Y)); lh5801_icount-=11;break;
	case 0x13: lh5801_adc(program_read_byte(0x10000|Y)); lh5801_icount-=11;break;
	case 0x15: lh5801_lda(program_read_byte(0x10000|Y)); lh5801_icount-=10;break;
	case 0x17: lh5801_cpa(lh5801.a, program_read_byte(0x10000|Y)); lh5801_icount-=11;break;
	case 0x18: X=Y;lh5801_icount-=11;break;
	case 0x19: lh5801_and(program_read_byte(0x10000|Y)); lh5801_icount-=11;break;
	case 0x1a: lh5801_pop_word(&lh5801.y); lh5801_icount-=15;break;
	case 0x1b: lh5801_ora(program_read_byte(0x10000|Y)); lh5801_icount-=11;break;
	case 0x1c: lh5801_dcs(program_read_byte(0x10000|Y)); lh5801_icount-=17; break;
	case 0x1d: lh5801_eor(program_read_byte(0x10000|Y)); lh5801_icount-=11;break;
	case 0x1e: program_write_byte(0x10000|Y,lh5801.a); lh5801_icount-=10;break;
	case 0x1f: lh5801_bit(program_read_byte(0x10000|Y),lh5801.a); lh5801_icount-=11;break;
	case 0x21: lh5801_sbc(program_read_byte(0x10000|U)); lh5801_icount-=11;break;
	case 0x23: lh5801_adc(program_read_byte(0x10000|U)); lh5801_icount-=11;break;
	case 0x25: lh5801_lda(program_read_byte(0x10000|U)); lh5801_icount-=10;break;
	case 0x27: lh5801_cpa(lh5801.a, program_read_byte(0x10000|U)); lh5801_icount-=11;break;
	case 0x28: X=U;lh5801_icount-=11;break;
	case 0x29: lh5801_and(program_read_byte(0x10000|U)); lh5801_icount-=11;break;
	case 0x2a: lh5801_pop_word(&lh5801.u); lh5801_icount-=15;break;
	case 0x2b: lh5801_ora(program_read_byte(0x10000|U)); lh5801_icount-=11;break;
	case 0x2c: lh5801_dcs(program_read_byte(0x10000|U)); lh5801_icount-=17; break;
	case 0x2d: lh5801_eor(program_read_byte(0x10000|U)); lh5801_icount-=11;break;
	case 0x2e: program_write_byte(0x10000|U,lh5801.a); lh5801_icount-=10;break;
	case 0x2f: lh5801_bit(program_read_byte(0x10000|U),lh5801.a); lh5801_icount-=11;break;
	case 0x40: lh5801_inc(&XH);lh5801_icount-=9;break;
	case 0x42: lh5801_dec(&XH);lh5801_icount-=9;break;
	case 0x48: X=S;lh5801_icount-=11;break;
	case 0x49: lh5801_and_mem(0x10000|X, cpu_readop(P++)); lh5801_icount-=17;break;
	case 0x4a: X=X;lh5801_icount-=11;break; //!!!
	case 0x4b: lh5801_ora_mem(0x10000|X, cpu_readop(P++)); lh5801_icount-=17;break;
	case 0x4c: lh5801.bf=0;/*off !*/ lh5801_icount-=8;break;
	case 0x4d: lh5801_bit(program_read_byte(X|0x10000), cpu_readop(P++));lh5801_icount-=14;break;
	case 0x4e: S=X;lh5801_icount-=11;break;
	case 0x4f: lh5801_add_mem(0x10000|X, cpu_readop(P++)); lh5801_icount-=17;break;
	case 0x50: lh5801_inc(&YH);lh5801_icount-=9;break;
	case 0x52: lh5801_dec(&YH);lh5801_icount-=9;break;
	case 0x58: X=P;lh5801_icount-=11;break;
	case 0x59: lh5801_and_mem(0x10000|Y, cpu_readop(P++)); lh5801_icount-=17;break;
	case 0x5a: Y=X;lh5801_icount-=11;break;
	case 0x5b: lh5801_ora_mem(0x10000|Y, cpu_readop(P++)); lh5801_icount-=17;break;
	case 0x5d: lh5801_bit(program_read_byte(Y|0x10000), cpu_readop(P++));lh5801_icount-=14;break;
	case 0x5e: lh5801_jmp(X);lh5801_icount-=11;break; // P=X
	case 0x5f: lh5801_add_mem(0x10000|Y, cpu_readop(P++)); lh5801_icount-=17;break;
	case 0x60: lh5801_inc(&UH);lh5801_icount-=9;break;
	case 0x62: lh5801_dec(&UH);lh5801_icount-=9;break;
	case 0x69: lh5801_and_mem(0x10000|U, cpu_readop(P++)); lh5801_icount-=17;break;
	case 0x6a: U=X;lh5801_icount-=11;break;
	case 0x6b: lh5801_ora_mem(0x10000|U, cpu_readop(P++)); lh5801_icount-=17;break;
	case 0x6d: lh5801_bit(program_read_byte(X|0x10000), cpu_readop(P++));lh5801_icount-=14;break;
	case 0x6f: lh5801_add_mem(0x10000|U, cpu_readop(P++)); lh5801_icount-=17;break;
	case 0x81: lh5801.t|=IE; /*sie !*/lh5801_icount-=8;break;
	case 0x88: lh5801_push_word(X); lh5801_icount-=14;break;
	case 0x8a: lh5801_pop(); lh5801_icount-=12; break;
	case 0x8c: lh5801_dca(program_read_byte(0x10000|X)); lh5801_icount-=19; break;
	case 0x8e: /*cdv clears internal devider*/lh5801_icount-=8;break;
	case 0x98: lh5801_push_word(Y); lh5801_icount-=14;break;
	case 0x9c: lh5801_dca(program_read_byte(0x10000|Y)); lh5801_icount-=19; break;
	case 0xa1: lh5801_sbc(program_read_byte(0x10000|lh5801_readop_word())); lh5801_icount-=17;break;
	case 0xa3: lh5801_adc(program_read_byte(0x10000|lh5801_readop_word())); lh5801_icount-=17;break;
	case 0xa5: lh5801_lda(program_read_byte(0x10000|lh5801_readop_word())); lh5801_icount-=16;break;
	case 0xa7: lh5801_cpa(lh5801.a, program_read_byte(0x10000|lh5801_readop_word())); lh5801_icount-=17;break;
	case 0xa8: lh5801_push_word(U); lh5801_icount-=14;break;
	case 0xa9: lh5801_and(program_read_byte(0x10000|lh5801_readop_word())); lh5801_icount-=17;break;
	case 0xaa: lh5801_lda(lh5801.t); lh5801_icount-=9;break;
	case 0xab: lh5801_ora(program_read_byte(0x10000|lh5801_readop_word())); lh5801_icount-=17;break;
	case 0xac: lh5801_dca(program_read_byte(0x10000|U)); lh5801_icount-=19; break;
	case 0xad: lh5801_eor(program_read_byte(0x10000|lh5801_readop_word())); lh5801_icount-=17;break;
	case 0xae: program_write_byte(0x10000|lh5801_readop_word(),lh5801.a); lh5801_icount-=16;break;
	case 0xaf: lh5801_bit(program_read_byte(0x10000|lh5801_readop_word()),lh5801.a); lh5801_icount-=17;break;
	case 0xb1: /*hlt*/lh5801_icount-=8;break;
	case 0xba: lh5801_ita();lh5801_icount-=9;break;
	case 0xbe: lh5801.t&=~IE; /*rie !*/lh5801_icount-=8;break;
	case 0xc0: lh5801.dp=0; /*rdp !*/lh5801_icount-=8;break;
	case 0xc1: lh5801.dp=1; /*sdp !*/lh5801_icount-=8;break;
	case 0xc8: lh5801_push(lh5801.a); lh5801_icount-=11;break;
	case 0xca: lh5801_adr(&lh5801.x);lh5801_icount-=11;break;
	case 0xcc: /*atp sends a to data bus*/lh5801_icount-=9;break;
	case 0xce: lh5801_am(lh5801.a); lh5801_icount-=9; break;
	case 0xd3: lh5801_drr(0x10000|X); lh5801_icount-=16; break;
	case 0xd7: lh5801_drl(0x10000|X); lh5801_icount-=16; break;
	case 0xda: lh5801_adr(&lh5801.y);lh5801_icount-=11;break;
	case 0xde: lh5801_am(lh5801.a|0x100); lh5801_icount-=9; break;
	case 0xea: lh5801_adr(&lh5801.u);lh5801_icount-=11;break;
	case 0xe9:
		adr=lh5801_readop_word()|0x10000;
		lh5801_and_mem(adr, cpu_readop(P++)); lh5801_icount-=23;
		break;
	case 0xeb:
		adr=lh5801_readop_word()|0x10000;
		lh5801_ora_mem(adr, cpu_readop(P++)); lh5801_icount-=23;
		break;
	case 0xec: lh5801.t=lh5801.a; lh5801_icount-=9;break;
	case 0xed:
		adr=lh5801_readop_word()|0x10000;lh5801_bit(program_read_byte(adr), cpu_readop(P++));
		lh5801_icount-=20;break;
	case 0xef:
		adr=lh5801_readop_word()|0x10000;
		lh5801_add_mem(adr, cpu_readop(P++)); lh5801_icount-=23;
		break;

	default:
		logerror("lh5801 illegal opcode at %.4x fd%.2x\n",P-2, oper);
	}
}

static void lh5801_instruction(void)
{
	int oper;
	int adr;

	oper=cpu_readop(P++);
	switch (oper) {
	case 0x00: lh5801_sbc(XL); lh5801_icount-=6;break;
	case 0x01: lh5801_sbc(program_read_byte(X)); lh5801_icount-=7;break;
	case 0x02: lh5801_adc(XL); lh5801_icount-=6;break;
	case 0x03: lh5801_adc(program_read_byte(X)); lh5801_icount-=7;break;
	case 0x04: lh5801_lda(XL); lh5801_icount-=5;break;
	case 0x05: lh5801_lda(program_read_byte(X)); lh5801_icount-=6;break;
	case 0x06: lh5801_cpa(lh5801.a, XL); lh5801_icount-=6;break;
	case 0x07: lh5801_cpa(lh5801.a, program_read_byte(X)); lh5801_icount-=7;break;
	case 0x08: XH=lh5801.a; lh5801_icount-=5; break;
	case 0x09: lh5801_and(program_read_byte(X)); lh5801_icount-=7;break;
	case 0x0a: XL=lh5801.a; lh5801_icount-=5; break;
	case 0x0b: lh5801_ora(program_read_byte(X)); lh5801_icount-=7;break;
	case 0x0c: lh5801_dcs(program_read_byte(X)); lh5801_icount-=13; break;
	case 0x0d: lh5801_eor(program_read_byte(X)); lh5801_icount-=7;break;
	case 0x0e: program_write_byte(X,lh5801.a); lh5801_icount-=6;break;
	case 0x0f: lh5801_bit(program_read_byte(X),lh5801.a); lh5801_icount-=7;break;
	case 0x10: lh5801_sbc(YL); lh5801_icount-=6;break;
	case 0x11: lh5801_sbc(program_read_byte(Y)); lh5801_icount-=7;break;
	case 0x12: lh5801_adc(YL); lh5801_icount-=6;break;
	case 0x13: lh5801_adc(program_read_byte(Y)); lh5801_icount-=7;break;
	case 0x14: lh5801_lda(YL); lh5801_icount-=5;break;
	case 0x15: lh5801_lda(program_read_byte(Y)); lh5801_icount-=6;break;
	case 0x16: lh5801_cpa(lh5801.a, YL); lh5801_icount-=6;break;
	case 0x17: lh5801_cpa(lh5801.a, program_read_byte(Y)); lh5801_icount-=7;break;
	case 0x18: YH=lh5801.a; lh5801_icount-=5; break;
	case 0x19: lh5801_and(program_read_byte(Y)); lh5801_icount-=7;break;
	case 0x1a: YL=lh5801.a; lh5801_icount-=5; break;
	case 0x1b: lh5801_ora(program_read_byte(Y)); lh5801_icount-=7;break;
	case 0x1c: lh5801_dcs(program_read_byte(Y)); lh5801_icount-=13; break;
	case 0x1d: lh5801_eor(program_read_byte(Y)); lh5801_icount-=7;break;
	case 0x1e: program_write_byte(Y,lh5801.a); lh5801_icount-=6;break;
	case 0x1f: lh5801_bit(program_read_byte(Y),lh5801.a); lh5801_icount-=7;break;
	case 0x20: lh5801_sbc(UL); lh5801_icount-=6;break;
	case 0x21: lh5801_sbc(program_read_byte(U)); lh5801_icount-=7;break;
	case 0x22: lh5801_adc(UL); lh5801_icount-=6;break;
	case 0x23: lh5801_adc(program_read_byte(U)); lh5801_icount-=7;break;
	case 0x24: lh5801_lda(UL); lh5801_icount-=5;break;
	case 0x25: lh5801_lda(program_read_byte(U)); lh5801_icount-=6;break;
	case 0x26: lh5801_cpa(lh5801.a, UL); lh5801_icount-=6;break;
	case 0x27: lh5801_cpa(lh5801.a, program_read_byte(U)); lh5801_icount-=7;break;
	case 0x28: UH=lh5801.a; lh5801_icount-=5; break;
	case 0x29: lh5801_and(program_read_byte(U)); lh5801_icount-=7;break;
	case 0x2a: UL=lh5801.a; lh5801_icount-=5; break;
	case 0x2b: lh5801_ora(program_read_byte(U)); lh5801_icount-=7;break;
	case 0x2c: lh5801_dcs(program_read_byte(U)); lh5801_icount-=13; break;
	case 0x2d: lh5801_eor(program_read_byte(U)); lh5801_icount-=7;break;
	case 0x2e: program_write_byte(U,lh5801.a); lh5801_icount-=6;break;
	case 0x2f: lh5801_bit(program_read_byte(U),lh5801.a); lh5801_icount-=7;break;
	case 0x38: /*nop*/lh5801_icount-=5;break;
	case 0x40: lh5801_inc(&XL);lh5801_icount-=5;break;
	case 0x41: lh5801_sin(&lh5801.x); lh5801_icount-=6;break;
	case 0x42: lh5801_dec(&XL);lh5801_icount-=5;break;
	case 0x43: lh5801_sde(&lh5801.x); lh5801_icount-=6;break;
	case 0x44: X++;lh5801_icount-=5;break;
	case 0x45: lh5801_lin(&lh5801.x);lh5801_icount-=6;break;
	case 0x46: X--;lh5801_icount-=5;break;
	case 0x47: lh5801_lde(&lh5801.x);lh5801_icount-=6;break;
	case 0x48: XH=cpu_readop(P++);lh5801_icount-=6;break;
	case 0x49: lh5801_and_mem(X, cpu_readop(P++)); lh5801_icount-=13;break;
	case 0x4a: XL=cpu_readop(P++);lh5801_icount-=6;break;
	case 0x4b: lh5801_ora_mem(X, cpu_readop(P++)); lh5801_icount-=13;break;
	case 0x4c: lh5801_cpa(XH, cpu_readop(P++)); lh5801_icount-=7;break;
	case 0x4d: lh5801_bit(program_read_byte(X), cpu_readop(P++));lh5801_icount-=10;break;
	case 0x4e: lh5801_cpa(XL, cpu_readop(P++)); lh5801_icount-=7;break;
	case 0x4f: lh5801_add_mem(X, cpu_readop(P++)); lh5801_icount-=13;break;
	case 0x50: lh5801_inc(&YL);lh5801_icount-=5;break;
	case 0x51: lh5801_sin(&lh5801.y); lh5801_icount-=6;break;
	case 0x52: lh5801_dec(&YL);lh5801_icount-=5;break;
	case 0x53: lh5801_sde(&lh5801.y); lh5801_icount-=6;break;
	case 0x54: Y++;lh5801_icount-=5;break;
	case 0x55: lh5801_lin(&lh5801.y);lh5801_icount-=6;break;
	case 0x56: Y--;lh5801_icount-=5;break;
	case 0x57: lh5801_lde(&lh5801.y);lh5801_icount-=6;break;
	case 0x58: YH=cpu_readop(P++);lh5801_icount-=6;break;
	case 0x59: lh5801_and_mem(Y, cpu_readop(P++)); lh5801_icount-=13;break;
	case 0x5a: YL=cpu_readop(P++);lh5801_icount-=6;break;
	case 0x5b: lh5801_ora_mem(Y, cpu_readop(P++)); lh5801_icount-=13;break;
	case 0x5c: lh5801_cpa(YH, cpu_readop(P++)); lh5801_icount-=7;break;
	case 0x5d: lh5801_bit(program_read_byte(Y), cpu_readop(P++));lh5801_icount-=10;break;
	case 0x5e: lh5801_cpa(YL, cpu_readop(P++)); lh5801_icount-=7;break;
	case 0x5f: lh5801_add_mem(Y, cpu_readop(P++)); lh5801_icount-=13;break;
	case 0x60: lh5801_inc(&UL);lh5801_icount-=5;break;
	case 0x61: lh5801_sin(&lh5801.u); lh5801_icount-=6;break;
	case 0x62: lh5801_dec(&UL);lh5801_icount-=5;break;
	case 0x63: lh5801_sde(&lh5801.u); lh5801_icount-=6;break;
	case 0x64: U++;lh5801_icount-=5;break;
	case 0x65: lh5801_lin(&lh5801.u);lh5801_icount-=6;break;
	case 0x66: U--;lh5801_icount-=5;break;
	case 0x67: lh5801_lde(&lh5801.u);lh5801_icount-=6;break;
	case 0x68: UH=cpu_readop(P++);lh5801_icount-=6;break;
	case 0x69: lh5801_and_mem(U, cpu_readop(P++)); lh5801_icount-=13;break;
	case 0x6a: UL=cpu_readop(P++);lh5801_icount-=6;break;
	case 0x6b: lh5801_ora_mem(U, cpu_readop(P++)); lh5801_icount-=13;break;
	case 0x6c: lh5801_cpa(UH, cpu_readop(P++)); lh5801_icount-=7;break;
	case 0x6d: lh5801_bit(program_read_byte(U), cpu_readop(P++));lh5801_icount-=10;break;
	case 0x6e: lh5801_cpa(UL, cpu_readop(P++)); lh5801_icount-=7;break;
	case 0x6f: lh5801_add_mem(U, cpu_readop(P++)); lh5801_icount-=13;break;
	case 0x80: lh5801_sbc(XH); lh5801_icount-=6;break;
	case 0x81: lh5801_branch_plus(!(lh5801.t&C)); lh5801_icount-=8; break;
	case 0x82: lh5801_adc(XH); lh5801_icount-=6;break;
	case 0x83: lh5801_branch_plus(lh5801.t&C); lh5801_icount-=8; break;
	case 0x84: lh5801_lda(XH); lh5801_icount-=5;break;
	case 0x85: lh5801_branch_plus(!(lh5801.t&H)); lh5801_icount-=8; break;
	case 0x86: lh5801_cpa(lh5801.a, XH); lh5801_icount-=6;break;
	case 0x87: lh5801_branch_plus(lh5801.t&H); lh5801_icount-=8; break;
	case 0x88: lh5801_lop(); break;
	case 0x89: lh5801_branch_plus(!(lh5801.t&Z)); lh5801_icount-=8; break;
	case 0x8a: lh5801_rti(); lh5801_icount-=14; break;
	case 0x8b: lh5801_branch_plus(lh5801.t&Z); lh5801_icount-=8; break;
	case 0x8c: lh5801_dca(program_read_byte(X)); lh5801_icount-=15; break;
	case 0x8d: lh5801_branch_plus(!(lh5801.t&V)); lh5801_icount-=8; break;
	case 0x8e: lh5801_branch_plus(1); lh5801_icount-=5; break;
	case 0x8f: lh5801_branch_plus(lh5801.t&V); lh5801_icount-=8; break;
	case 0x90: lh5801_sbc(YH); lh5801_icount-=6;break;
	case 0x91: lh5801_branch_minus(!(lh5801.t&C)); lh5801_icount-=8; break;
	case 0x92: lh5801_adc(YH); lh5801_icount-=6;break;
	case 0x93: lh5801_branch_minus(lh5801.t&C); lh5801_icount-=8; break;
	case 0x94: lh5801_lda(YH); lh5801_icount-=5;break;
	case 0x95: lh5801_branch_minus(!(lh5801.t&H)); lh5801_icount-=8; break;
	case 0x96: lh5801_cpa(lh5801.a, YH); lh5801_icount-=6;break;
	case 0x97: lh5801_branch_minus(lh5801.t&H); lh5801_icount-=8; break;
	case 0x99: lh5801_branch_minus(!(lh5801.t&Z)); lh5801_icount-=8; break;
	case 0x9a: lh5801_rtn(); lh5801_icount-=11; break;
	case 0x9b: lh5801_branch_minus(lh5801.t&Z); lh5801_icount-=8; break;
	case 0x9c: lh5801_dca(program_read_byte(Y)); lh5801_icount-=15; break;
	case 0x9d: lh5801_branch_minus(!(lh5801.t&V)); lh5801_icount-=8; break;
	case 0x9e: lh5801_branch_minus(1); lh5801_icount-=6; break;
	case 0x9f: lh5801_branch_minus(lh5801.t&V); lh5801_icount-=8; break;
	case 0xa0: lh5801_sbc(UH); lh5801_icount-=6;break;
	case 0xa2: lh5801_adc(UH); lh5801_icount-=6;break;
	case 0xa1: lh5801_sbc(program_read_byte(lh5801_readop_word())); lh5801_icount-=13;break;
	case 0xa3: lh5801_adc(program_read_byte(lh5801_readop_word())); lh5801_icount-=13;break;
	case 0xa4: lh5801_lda(UH); lh5801_icount-=5;break;
	case 0xa5: lh5801_lda(program_read_byte(lh5801_readop_word())); lh5801_icount-=12;break;
	case 0xa6: lh5801_cpa(lh5801.a, UH); lh5801_icount-=6;break;
	case 0xa7: lh5801_cpa(lh5801.a, program_read_byte(lh5801_readop_word())); lh5801_icount-=13;break;
	case 0xa8: lh5801.pv=1;/*spv!*/ lh5801_icount-=4; break;
	case 0xa9: lh5801_and(program_read_byte(lh5801_readop_word())); lh5801_icount-=13;break;
	case 0xaa: S=lh5801_readop_word();lh5801_icount-=6;break;
	case 0xab: lh5801_ora(program_read_byte(lh5801_readop_word())); lh5801_icount-=13;break;
	case 0xac: lh5801_dca(program_read_byte(U)); lh5801_icount-=15; break;
	case 0xad: lh5801_eor(program_read_byte(lh5801_readop_word())); lh5801_icount-=13;break;
	case 0xae: program_write_byte(lh5801_readop_word(),lh5801.a); lh5801_icount-=12;break;
	case 0xaf: lh5801_bit(program_read_byte(lh5801_readop_word()),lh5801.a); lh5801_icount-=13;break;
	case 0xb1: lh5801_sbc(cpu_readop(P++)); lh5801_icount-=7;break;
	case 0xb3: lh5801_adc(cpu_readop(P++)); lh5801_icount-=7;break;
	case 0xb5: lh5801_lda(cpu_readop(P++)); lh5801_icount-=6;break;
	case 0xb7: lh5801_cpa(lh5801.a, cpu_readop(P++)); lh5801_icount-=7;break;
	case 0xb8: lh5801.pv=0;/*rpv!*/ lh5801_icount-=4; break;
	case 0xb9: lh5801_and(cpu_readop(P++)); lh5801_icount-=7;break;
	case 0xba: lh5801_jmp(lh5801_readop_word()); lh5801_icount-=12;break;
	case 0xbb: lh5801_ora(cpu_readop(P++)); lh5801_icount-=7;break;
	case 0xbd: lh5801_eor(cpu_readop(P++)); lh5801_icount-=7;break;
	case 0xbe: lh5801_sjp(); lh5801_icount-=19; break;
	case 0xbf: lh5801_bit(lh5801.a, cpu_readop(P++));lh5801_icount-=7;break;
	case 0xc1: lh5801_vector(!(lh5801.t&C), cpu_readop(P++)); lh5801_icount-=8;break;
	case 0xc3: lh5801_vector(lh5801.t&C, cpu_readop(P++)); lh5801_icount-=8;break;
	case 0xc5: lh5801_vector(!(lh5801.t&H), cpu_readop(P++)); lh5801_icount-=8;break;
	case 0xc7: lh5801_vector(lh5801.t&H, cpu_readop(P++)); lh5801_icount-=8;break;
	case 0xc9: lh5801_vector(!(lh5801.t&Z), cpu_readop(P++)); lh5801_icount-=8;break;
	case 0xcb: lh5801_vector(lh5801.t&Z, cpu_readop(P++)); lh5801_icount-=8;break;
	case 0xcd: lh5801_vector(1, cpu_readop(P++)); lh5801_icount-=7;break;
	case 0xcf: lh5801_vector(lh5801.t&V, cpu_readop(P++)); lh5801_icount-=8;break;
	case 0xd1: lh5801_ror(); lh5801_icount-=6; break;
	case 0xd3: lh5801_drr(X); lh5801_icount-=12; break;
	case 0xd5: lh5801_shr(); lh5801_icount-=6; break;
	case 0xd7: lh5801_drl(X); lh5801_icount-=12; break;
	case 0xd9: lh5801_shl(); lh5801_icount-=6; break;
	case 0xdb: lh5801_rol(); lh5801_icount-=6; break;
	case 0xdd: lh5801_inc(&lh5801.a);lh5801_icount-=5;break;
	case 0xdf: lh5801_dec(&lh5801.a);lh5801_icount-=5;break;
	case 0xe1: lh5801.pu=1;/*spu!*/ lh5801_icount-=4; break;
	case 0xe3: lh5801.pu=0;/*rpu!*/ lh5801_icount-=4; break;
	case 0xe9:
		adr=lh5801_readop_word();lh5801_and_mem(adr, cpu_readop(P++));
		lh5801_icount-=19;break;
	case 0xeb:
		adr=lh5801_readop_word();lh5801_ora_mem(adr, cpu_readop(P++));
		lh5801_icount-=19;break;
	case 0xed:
		adr=lh5801_readop_word();lh5801_bit(program_read_byte(adr), cpu_readop(P++));
		lh5801_icount-=16;break;
	case 0xef:
		adr=lh5801_readop_word();
		lh5801_add_mem(adr, cpu_readop(P++)); lh5801_icount-=19;
		break;
	case 0xf1: lh5801_aex(); lh5801_icount-=6; break;
	case 0xf5: program_write_byte(Y++, program_read_byte(X++)); lh5801_icount-=7; break; //tin
	case 0xf7: lh5801_cpa(lh5801.a, program_read_byte(X++));lh5801_icount-=7; break; //cin
	case 0xf9: lh5801.t&=~C;lh5801_icount-=4;break;
	case 0xfb: lh5801.t|=C;lh5801_icount-=4;break;
	case 0xfd: lh5801_instruction_fd();break;
	case 0xc0: case 0xc2: case 0xc4: case 0xc6:
	case 0xc8: case 0xca: case 0xcc: case 0xce:
	case 0xd0: case 0xd2: case 0xd4: case 0xd6:
	case 0xd8: case 0xda: case 0xdc: case 0xde:
	case 0xe0: case 0xe2: case 0xe4: case 0xe6:
	case 0xe8: case 0xea: case 0xec: case 0xee:
	case 0xf0: case 0xf2: case 0xf4: case 0xf6:
		lh5801_vector(1, oper);lh5801_icount-=4;break;
	default:
		logerror("lh5801 illegal opcode at %.4x %.2x\n",P-1, oper);
	}

}

