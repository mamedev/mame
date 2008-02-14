#define IRQ_ADDRESS	0xf

INLINE int READ_OP(void)
{
	saturn_ICount-=3;
	return cpu_readop(saturn.pc++);
}

INLINE int READ_OP_ARG(void)
{
	saturn_ICount-=3;
	return cpu_readop_arg(saturn.pc++);
}

INLINE int READ_OP_ARG8(void)
{
	return READ_OP_ARG()|(READ_OP_ARG()<<4);
}

INLINE INT8 READ_OP_DIS8(void)
{
	return READ_OP_ARG()|(READ_OP_ARG()<<4);
}

INLINE int READ_OP_ARG12(void)
{
	return READ_OP_ARG()|(READ_OP_ARG()<<4)|(READ_OP_ARG()<<8);
}

INLINE int READ_OP_DIS12(void)
{
	int temp=READ_OP_ARG()|(READ_OP_ARG()<<4)|(READ_OP_ARG()<<8);
	if (temp&0x800) return -0x1000+temp;
	else return temp;
}

INLINE int READ_OP_ARG16(void)
{
	return READ_OP_ARG()|(READ_OP_ARG()<<4)|(READ_OP_ARG()<<8)|(READ_OP_ARG()<<12);
}

INLINE INT16 READ_OP_DIS16(void)
{
	return READ_OP_ARG()|(READ_OP_ARG()<<4)|(READ_OP_ARG()<<8)|(READ_OP_ARG()<<12);
}

INLINE int READ_OP_ARG20(void)
{
	return READ_OP_ARG()|(READ_OP_ARG()<<4)|(READ_OP_ARG()<<8)
		|(READ_OP_ARG()<<12)|(READ_OP_ARG()<<16);
}

INLINE int READ_NIBBLE(SaturnAdr adr)
{
	int data;
	saturn_ICount-=3;
	data = program_read_byte(adr);
	if (saturn.config&&saturn.config->crc) saturn.config->crc(adr, data);
	return data;
}

INLINE int READ_8(SaturnAdr adr)
{
	return READ_NIBBLE(adr)|(READ_NIBBLE(adr+1)<<4);
}

INLINE int READ_12(SaturnAdr adr)
{
	return READ_NIBBLE(adr)|(READ_NIBBLE(adr+1)<<4)|(READ_NIBBLE(adr+2)<<8);
}

INLINE int READ_16(SaturnAdr adr)
{
	return READ_NIBBLE(adr)|(READ_NIBBLE(adr+1)<<4)
		|(READ_NIBBLE(adr+2)<<8)|(READ_NIBBLE(adr+3)<<12);
}

INLINE int READ_20(SaturnAdr adr)
{
	return READ_NIBBLE(adr)|(READ_NIBBLE(adr+1)<<4)
		|(READ_NIBBLE(adr+2)<<8)|(READ_NIBBLE(adr+3)<<12)|(READ_NIBBLE(adr+4)<<16);
}

INLINE void WRITE_NIBBLE(SaturnAdr adr, SaturnNib nib)
{
	saturn_ICount -= 3;
	program_write_byte(adr, nib);
}

#ifdef LSB_FIRST
#define S64_BYTE(r, x) saturn.reg[r].b[x]
#define S64_WORD(r, x) saturn.reg[r].w[x]
#define S64_DOUBLE(r, x) saturn.reg[r].d[x]
#else
#define S64_BYTE(r, x) saturn.reg[r].b[7-x]
#define S64_WORD(r, x) saturn.reg[r].w[3-x]
#define S64_DOUBLE(r, x) saturn.reg[r].d[1-x]
#endif

#define S64_QUAD(r) saturn.reg[r].q

#define BEGIN_B 0
#define COUNT_B 2
#define BEGIN_X 0
#define COUNT_X 3
#define BEGIN_XS 2
#define COUNT_XS 1
#define BEGIN_A 0
#define COUNT_A 5
#define BEGIN_M 3
#define COUNT_M 12
#define BEGIN_S 15
#define COUNT_S 1
#define BEGIN_W 0
#define COUNT_W 16

#define S64_READ_NIBBLE(r, x) (((x)&1) ? (S64_BYTE(r, ((x)>>1))>>4) : (S64_BYTE(r, ((x)>>1))&0xf) )
#define S64_WRITE_NIBBLE(r, x, v) \
		(S64_BYTE(r, ((x)>>1)) = ((x)&1) \
		 ?(S64_BYTE(r, ((x)>>1))&0xf)|((v)<<4) \
		 :(S64_BYTE(r, ((x)>>1))&0xf0)|(v) )

#define S64_READ_B(r) S64_BYTE(r,0)
#define S64_WRITE_B(r,v) (S64_BYTE(r,0)=v)

#define S64_READ_XS(r) S64_READ_NIBBLE(r,2)
#define S64_WRITE_XS(r,v) S64_WRITE_NIBBLE(r,2,v)

#define S64_READ_S(r) S64_READ_NIBBLE(r,15)
#define S64_WRITE_S(r,v) S64_WRITE_NIBBLE(r,15,v)

#define S64_READ_P(r) S64_READ_NIBBLE(r,saturn.p)
#define S64_WRITE_P(r,v) S64_WRITE_NIBBLE(r,saturn.p,v)

#define S64_READ_X(r) (S64_WORD(r,0)&0xfff)
#define S64_WRITE_X(r,v) (S64_WORD(r,0)=(S64_WORD(r,0)&~0xfff)|(v))

// for address reg operations
#define S64_READ_WORD(r,nr) S64_WORD(r,nr)
#define S64_WRITE_WORD(r,nr,v) (S64_WORD(r,nr)=v)

#define S64_READ_A(r) (S64_DOUBLE(r,0)&0xfffff)
#define S64_WRITE_A(r,v) (S64_DOUBLE(r,0)=(S64_DOUBLE(r,0)&~0xfffff)|(v))

#define S64_READ_M(r) ((S64_QUAD(r)>>12)&0xffffffffffffULL)
#define S64_WRITE_M(r,v) (S64_QUAD(r)=(S64_QUAD(r)&~0xffffffffffff000ULL)|((v)<<12))

#define S64_READ_W(r) S64_QUAD(r)
#define S64_WRITE_W(r,v) (S64_QUAD(r)=(v))

INLINE SaturnAdr saturn_pop(void)
{
	SaturnAdr temp=saturn.rstk[0];
	memmove(saturn.rstk, saturn.rstk+1, sizeof(saturn.rstk)-sizeof(saturn.rstk[0]));
	saturn.rstk[7]=0;
	saturn.stackpointer--;
	return temp;
}

INLINE void saturn_push(SaturnAdr adr)
{
	memmove(saturn.rstk+1, saturn.rstk, sizeof(saturn.rstk)-sizeof(saturn.rstk[0]));
	saturn.rstk[0]=adr;
	saturn.stackpointer++;
}

INLINE void saturn_interrupt_on(void)
{

}

INLINE void saturn_interrupt_off(void)
{

}

INLINE void saturn_reset_interrupt(void)
{

}

INLINE void saturn_mem_reset(void)
{
	if (saturn.config->reset) saturn.config->reset();
}

INLINE void saturn_mem_config(void)
{
	if (saturn.config->config) saturn.config->config(S64_READ_A(C));
}

INLINE void saturn_mem_unconfig(void)
{
	if (saturn.config->unconfig) saturn.config->unconfig(S64_READ_A(C));
}

INLINE int saturn_mem_id(void)
{
	if (saturn.config->id) return saturn.config->id();
	return 0;
}

INLINE void saturn_shutdown(void)
{
}

INLINE void saturn_bus_command_b(void)
{
}

INLINE void saturn_bus_command_c(void)
{
}

INLINE void saturn_bus_command_d(void)
{
}

INLINE void saturn_serial_request(void)
{
}

INLINE void saturn_out_c(void)
{
	if (saturn.config&&saturn.config->out) saturn.config->out(S64_READ_X(C));
}

INLINE void saturn_out_cs(void)
{
	if (saturn.config&&saturn.config->out)
		saturn.config->out(S64_READ_NIBBLE(C,0)|(saturn.out&0xff0));
}

INLINE void saturn_in(int reg)
{
	if (saturn.config&&saturn.config->in) S64_WORD(reg,0)=saturn.config->in();
}

INLINE void saturn_sethex(void) { saturn.decimal=0; }
INLINE void saturn_setdec(void) { saturn.decimal=1; }

/* st related */
INLINE void saturn_clear_st(void)
{
	saturn.st=0;
}

INLINE void saturn_st_to_c(void)
{
	S64_WRITE_X(C, saturn.st&0xfff);
}

INLINE void saturn_c_to_st(void)
{
	saturn.st=(saturn.st&~0xfff)|S64_READ_X(C);
}

INLINE void saturn_exchange_c_st(void)
{
	int t=saturn.st&0xfff;
	saturn.st=(saturn.st&~0xfff)|S64_READ_X(C);
	S64_WRITE_X(C, t);
}

INLINE void saturn_st_clear_bit(void)
{
	switch(READ_OP_ARG()) {
	case 0: saturn.st&=~1;break;
	case 1: saturn.st&=~2;break;
	case 2: saturn.st&=~4;break;
	case 3: saturn.st&=~8;break;
	case 4: saturn.st&=~0x10;break;
	case 5: saturn.st&=~0x20;break;
	case 6: saturn.st&=~0x40;break;
	case 7: saturn.st&=~0x80;break;
	case 8: saturn.st&=~0x100;break;
	case 9: saturn.st&=~0x200;break;
	case 0xa: saturn.st&=~0x400;break;
	case 0xb: saturn.st&=~0x800;break;
	case 0xc: saturn.st&=~0x1000;break;
	case 0xd: saturn.st&=~0x2000;break;
	case 0xe: saturn.st&=~0x4000;break;
	case 0xf: saturn.st&=~0x8000;break;
	}
}

INLINE void saturn_st_set_bit(void)
{
	switch(READ_OP_ARG()) {
	case 0: saturn.st|=1;break;
	case 1: saturn.st|=2;break;
	case 2: saturn.st|=4;break;
	case 3: saturn.st|=8;break;
	case 4: saturn.st|=0x10;break;
	case 5: saturn.st|=0x20;break;
	case 6: saturn.st|=0x40;break;
	case 7: saturn.st|=0x80;break;
	case 8: saturn.st|=0x100;break;
	case 9: saturn.st|=0x200;break;
	case 0xa: saturn.st|=0x400;break;
	case 0xb: saturn.st|=0x800;break;
	case 0xc: saturn.st|=0x1000;break;
	case 0xd: saturn.st|=0x2000;break;
	case 0xe: saturn.st|=0x4000;break;
	case 0xf: saturn.st|=0x8000;break;
	}
}

INLINE void saturn_st_jump_bit_clear(void)
{
	int adr;
	switch(READ_OP_ARG()) {
	case 0: saturn.carry=!saturn.st&1;break;
	case 1: saturn.carry=!saturn.st&2;break;
	case 2: saturn.carry=!saturn.st&4;break;
	case 3: saturn.carry=!saturn.st&8;break;
	case 4: saturn.carry=!saturn.st&0x10;break;
	case 5: saturn.carry=!saturn.st&0x20;break;
	case 6: saturn.carry=!saturn.st&0x40;break;
	case 7: saturn.carry=!saturn.st&0x80;break;
	case 8: saturn.carry=!saturn.st&0x100;break;
	case 9: saturn.carry=!saturn.st&0x200;break;
	case 0xa: saturn.carry=!saturn.st&0x400;break;
	case 0xb: saturn.carry=!saturn.st&0x800;break;
	case 0xc: saturn.carry=!saturn.st&0x1000;break;
	case 0xd: saturn.carry=!saturn.st&0x2000;break;
	case 0xe: saturn.carry=!saturn.st&0x4000;break;
	case 0xf: saturn.carry=!saturn.st&0x8000;break;
	}
	adr=READ_OP_DIS8();
	if (saturn.carry) {
		if (adr==0) {
			saturn.pc=saturn_pop();
		} else {
			saturn.pc=(saturn.pc+adr-2)&0xfffff;
		}
		change_pc(saturn.pc);
	}
}

INLINE void saturn_st_jump_bit_set(void)
{
	int adr;
	switch(READ_OP_ARG()) {
	case 0: saturn.carry=saturn.st&1;break;
	case 1: saturn.carry=saturn.st&2;break;
	case 2: saturn.carry=saturn.st&4;break;
	case 3: saturn.carry=saturn.st&8;break;
	case 4: saturn.carry=saturn.st&0x10;break;
	case 5: saturn.carry=saturn.st&0x20;break;
	case 6: saturn.carry=saturn.st&0x40;break;
	case 7: saturn.carry=saturn.st&0x80;break;
	case 8: saturn.carry=saturn.st&0x100;break;
	case 9: saturn.carry=saturn.st&0x200;break;
	case 0xa: saturn.carry=saturn.st&0x400;break;
	case 0xb: saturn.carry=saturn.st&0x800;break;
	case 0xc: saturn.carry=saturn.st&0x1000;break;
	case 0xd: saturn.carry=saturn.st&0x2000;break;
	case 0xe: saturn.carry=saturn.st&0x4000;break;
	case 0xf: saturn.carry=saturn.st&0x8000;break;
	}
	adr=READ_OP_DIS8();
	if (saturn.carry) {
		if (adr==0) {
			saturn.pc=saturn_pop();
		} else {
			saturn.pc=(saturn.pc+adr-2)&0xfffff;
		}
		change_pc(saturn.pc);
	}
}

INLINE void saturn_hst_clear_bits(void)
{
	saturn.hst&=~READ_OP_ARG();
}

INLINE void saturn_hst_bits_cleared(void)
{
	saturn.carry=!(saturn.hst&READ_OP_ARG());
}

/* p related */
INLINE void saturn_exchange_p(void)
{
	int nr=READ_OP_ARG();
	int t=saturn.p;
	saturn.p=S64_READ_NIBBLE(C,nr);
	S64_WRITE_NIBBLE(C,nr,t);
}

INLINE void saturn_p_to_c(void)
{
	int nr=READ_OP_ARG();
	S64_WRITE_NIBBLE(C,nr,saturn.p);
}

INLINE void saturn_c_to_p(void)
{
	int nr=READ_OP_ARG();
	saturn.p=S64_READ_NIBBLE(C,nr);
}

INLINE void saturn_dec_p(void)
{
	saturn.carry=saturn.p==0;
	saturn.p=saturn.p-1;
}

INLINE void saturn_inc_p(void)
{
	saturn.p=saturn.p+1;
	saturn.carry=saturn.p==0;
}

INLINE void saturn_load_p(void)
{
	saturn.p=READ_OP_ARG();
}

INLINE void saturn_p_equals(void)
{
	int nr=READ_OP_ARG();
	int adr;
	saturn.carry=saturn.p==nr;
	adr=READ_OP_DIS8();
	if (saturn.carry) {
		if (adr==0) {
			saturn.pc=saturn_pop();
		} else {
			saturn.pc=(saturn.pc+adr-2)&0xfffff;
		}
		change_pc(saturn.pc);
	}
}

INLINE void saturn_p_not_equals(void)
{
	int nr=READ_OP_ARG();
	int adr;
	saturn.carry=saturn.p!=nr;
	adr=READ_OP_DIS8();
	if (saturn.carry) {
		if (adr==0) {
			saturn.pc=saturn_pop();
		} else {
			saturn.pc=(saturn.pc+adr-2)&0xfffff;
		}
		change_pc(saturn.pc);
	}
}

INLINE void saturn_ca_p_1(void)
{
	int a=S64_READ_A(C)+1+saturn.p;
	saturn.carry=a>=0x100000;
	S64_WRITE_A(C,a&0xfffff);
}

INLINE void saturn_load_reg(int reg)
{
	int count=READ_OP_ARG();
	int pos=saturn.p;
	for (; count>=0; count--, pos=(pos+1)&0xf ) {
		S64_WRITE_NIBBLE( reg, pos, READ_OP_ARG());
	}
}

INLINE void saturn_jump(int adr, int jump)
{
	if (jump) {
		saturn.pc=adr;
		saturn_ICount-=10;
		change_pc(saturn.pc);
	}
}

INLINE void saturn_call(int adr)
{
	saturn_push(saturn.pc);
	saturn.pc=adr;
//  saturn_ICount-=10;
	change_pc(saturn.pc);
}

INLINE void saturn_return(int yes)
{
	if (yes) {
		saturn.pc=saturn_pop();
//  saturn_ICount-=10;
		change_pc(saturn.pc);
	}
}

INLINE void saturn_return_carry_set(void)
{
	saturn.pc=saturn_pop();
//  saturn_ICount-=10;
	change_pc(saturn.pc);
	saturn.carry=1;
}

INLINE void saturn_return_carry_clear(void)
{
	saturn.pc=saturn_pop();
//  saturn_ICount-=10;
	change_pc(saturn.pc);
	saturn.carry=0;
}

INLINE void saturn_return_interrupt(void)
{
	saturn.pc=saturn_pop();
//  saturn_ICount-=10;
	change_pc(saturn.pc);
}

INLINE void saturn_return_xm_set(void)
{
	saturn.pc=saturn_pop();
	saturn.hst|=XM;
//  saturn_ICount-=10;
	change_pc(saturn.pc);
}

INLINE void saturn_pop_c(void)
{
	S64_WRITE_A(C,saturn_pop());
}

INLINE void saturn_push_c(void)
{
	saturn_push(S64_READ_A(C));
}

INLINE void saturn_indirect_jump(int reg)
{
	saturn.pc=READ_20(S64_READ_A(reg));
	change_pc(saturn.pc);
}

INLINE void saturn_equals_zero(int reg, int begin, int count)
{
	int i, t,adr;
	for (i=0; i<count; i++) {
		t=S64_READ_NIBBLE(reg, (begin+i)&0xf );
		if (t!=0) break;
		saturn_ICount-=2;
	}
	saturn.carry=i==count;
	adr=READ_OP_DIS8();
	if (saturn.carry) {
		if (adr==0) {
			saturn.pc=saturn_pop();
		} else {
			saturn.pc=(saturn.pc+adr-2)&0xfffff;
		}
		change_pc(saturn.pc);
	}
}

INLINE void saturn_equals(int reg, int begin, int count, int right)
{
	int i, t,t2,adr;
	for (i=0; i<count; i++) {
		t=S64_READ_NIBBLE(reg, (begin+i)&0xf );
		t2=S64_READ_NIBBLE(right, (begin+i)&0xf );
		if (t!=t2) break;
		saturn_ICount-=2;
	}
	saturn.carry=i==count;
	adr=READ_OP_DIS8();
	if (saturn.carry) {
		if (adr==0) {
			saturn.pc=saturn_pop();
		} else {
			saturn.pc=(saturn.pc+adr-2)&0xfffff;
		}
		change_pc(saturn.pc);
	}
}

INLINE void saturn_not_equals_zero(int reg, int begin, int count)
{
	int i, t,adr;
	for (i=0; i<count; i++) {
		t=S64_READ_NIBBLE(reg, (begin+i)&0xf );
		if (t==0) break;
		saturn_ICount-=2;
	}
	saturn.carry=i==count;
	adr=READ_OP_DIS8();
	if (saturn.carry) {
		if (adr==0) {
			saturn.pc=saturn_pop();
		} else {
			saturn.pc=(saturn.pc+adr-2)&0xfffff;
		}
		change_pc(saturn.pc);
	}
}

INLINE void saturn_not_equals(int reg, int begin, int count, int right)
{
	int i, t,t2,adr;
	for (i=0; i<count; i++) {
		t=S64_READ_NIBBLE(reg, (begin+i)&0xf );
		t2=S64_READ_NIBBLE(right, (begin+i)&0xf );
		if (t==t2) break;
		saturn_ICount-=2;
	}
	saturn.carry=i==count;
	adr=READ_OP_DIS8();
	if (saturn.carry) {
		if (adr==0) {
			saturn.pc=saturn_pop();
		} else {
			saturn.pc=(saturn.pc+adr-2)&0xfffff;
		}
		change_pc(saturn.pc);
	}
}

INLINE void saturn_greater(int reg, int begin, int count, int right)
{
	int i, t,t2,adr;
	for (i=count; i>=0; i--) {
		t=S64_READ_NIBBLE(reg, (begin+i)&0xf );
		t2=S64_READ_NIBBLE(right, (begin+i)&0xf );
		if (t<=t2) break;
		saturn_ICount-=2;
	}
	saturn.carry=i<0;
	adr=READ_OP_DIS8();
	if (saturn.carry) {
		if (adr==0) {
			saturn.pc=saturn_pop();
		} else {
			saturn.pc=(saturn.pc+adr-2)&0xfffff;
		}
		change_pc(saturn.pc);
	}
}

INLINE void saturn_greater_equals(int reg, int begin, int count, int right)
{
	int i, t,t2,adr;
	for (i=count; i>=0; i--) {
		t=S64_READ_NIBBLE(reg, (begin+i)&0xf );
		t2=S64_READ_NIBBLE(right, (begin+i)&0xf );
		if (t<t2) break;
		saturn_ICount-=2;
	}
	saturn.carry=i<0;
	adr=READ_OP_DIS8();
	if (saturn.carry) {
		if (adr==0) {
			saturn.pc=saturn_pop();
		} else {
			saturn.pc=(saturn.pc+adr-2)&0xfffff;
		}
		change_pc(saturn.pc);
	}
}

INLINE void saturn_smaller_equals(int reg, int begin, int count, int right)
{
	int i, t,t2,adr;
	for (i=count; i>=0; i--) {
		t=S64_READ_NIBBLE(reg, (begin+i)&0xf );
		t2=S64_READ_NIBBLE(right, (begin+i)&0xf );
		if (t>t2) break;
		saturn_ICount-=2;
	}
	saturn.carry=i<0;
	adr=READ_OP_DIS8();
	if (saturn.carry) {
		if (adr==0) {
			saturn.pc=saturn_pop();
		} else {
			saturn.pc=(saturn.pc+adr-2)&0xfffff;
		}
		change_pc(saturn.pc);
	}
}

INLINE void saturn_smaller(int reg, int begin, int count, int right)
{
	int i, t,t2,adr;
	for (i=count; i>=0; i--) {
		t=S64_READ_NIBBLE(reg, (begin+i)&0xf );
		t2=S64_READ_NIBBLE(right, (begin+i)&0xf );
		if (t>=t2) break;
		saturn_ICount-=2;
	}
	saturn.carry=i<0;
	adr=READ_OP_DIS8();
	if (saturn.carry) {
		if (adr==0) {
			saturn.pc=saturn_pop();
		} else {
			saturn.pc=(saturn.pc+adr-2)&0xfffff;
		}
		change_pc(saturn.pc);
	}
}

INLINE void saturn_jump_bit_clear(int reg)
{
	int adr;
	switch(READ_OP_ARG()) {
	case 0: saturn.carry=!(S64_BYTE( reg, 0)&1);break;
	case 1: saturn.carry=!(S64_BYTE( reg, 0)&2);break;
	case 2: saturn.carry=!(S64_BYTE( reg, 0)&4);break;
	case 3: saturn.carry=!(S64_BYTE( reg, 0)&8);break;
	case 4: saturn.carry=!(S64_BYTE( reg, 0)&0x10);break;
	case 5: saturn.carry=!(S64_BYTE( reg, 0)&0x20);break;
	case 6: saturn.carry=!(S64_BYTE( reg, 0)&0x40);break;
	case 7: saturn.carry=!(S64_BYTE( reg, 0)&0x80);break;
	case 8: saturn.carry=!(S64_BYTE( reg, 1)&1);break;
	case 9: saturn.carry=!(S64_BYTE( reg, 1)&2);break;
	case 0xa: saturn.carry=!(S64_BYTE( reg, 1)&4);break;
	case 0xb: saturn.carry=!(S64_BYTE( reg, 1)&8);break;
	case 0xc: saturn.carry=!(S64_BYTE( reg, 1)&0x10);break;
	case 0xd: saturn.carry=!(S64_BYTE( reg, 1)&0x20);break;
	case 0xe: saturn.carry=!(S64_BYTE( reg, 1)&0x40);break;
	case 0xf: saturn.carry=!(S64_BYTE( reg, 1)&0x80);break;
	}
	adr=READ_OP_DIS8();
	if (saturn.carry) {
		if (adr==0) {
			saturn.pc=saturn_pop();
		} else {
			saturn.pc=(saturn.pc+adr-2)&0xfffff;
		}
		change_pc(saturn.pc);
	}
}

INLINE void saturn_jump_bit_set(int reg)
{
	int adr;
	switch(READ_OP_ARG()) {
	case 0: saturn.carry=S64_BYTE( reg, 0)&1;break;
	case 1: saturn.carry=S64_BYTE( reg, 0)&2;break;
	case 2: saturn.carry=S64_BYTE( reg, 0)&4;break;
	case 3: saturn.carry=S64_BYTE( reg, 0)&8;break;
	case 4: saturn.carry=S64_BYTE( reg, 0)&0x10;break;
	case 5: saturn.carry=S64_BYTE( reg, 0)&0x20;break;
	case 6: saturn.carry=S64_BYTE( reg, 0)&0x40;break;
	case 7: saturn.carry=S64_BYTE( reg, 0)&0x80;break;
	case 8: saturn.carry=S64_BYTE( reg, 1)&1;break;
	case 9: saturn.carry=S64_BYTE( reg, 1)&2;break;
	case 0xa: saturn.carry=S64_BYTE( reg, 1)&4;break;
	case 0xb: saturn.carry=S64_BYTE( reg, 1)&8;break;
	case 0xc: saturn.carry=S64_BYTE( reg, 1)&0x10;break;
	case 0xd: saturn.carry=S64_BYTE( reg, 1)&0x20;break;
	case 0xe: saturn.carry=S64_BYTE( reg, 1)&0x40;break;
	case 0xf: saturn.carry=S64_BYTE( reg, 1)&0x80;break;
	}
	adr=READ_OP_DIS8();
	if (saturn.carry) {
		if (adr==0) {
			saturn.pc=saturn_pop();
		} else {
			saturn.pc=(saturn.pc+adr-2)&0xfffff;
		}
		change_pc(saturn.pc);
	}
}

INLINE void saturn_load_pc(int reg)
{
	saturn.pc=S64_READ_A(reg);
	change_pc(saturn.pc);
}

INLINE void saturn_store_pc(int reg)
{
	S64_WRITE_A(reg,saturn.pc);
}

INLINE void saturn_exchange_pc(int reg)
{
	int temp=saturn.pc;
	saturn.pc=S64_READ_A(reg);
	change_pc(saturn.pc);
	S64_WRITE_A(reg, temp);
}

/*************************************************************************************
 address register related
*************************************************************************************/
INLINE void saturn_load_adr(int reg, int nibbles)
{
	switch (nibbles) {
	case 5:
		saturn.d[reg]=READ_OP_ARG20();
		break;
	case 4:
		saturn.d[reg]=(saturn.d[reg]&0xf0000)|READ_OP_ARG16();
		break;
	case 2:
		saturn.d[reg]=(saturn.d[reg]&0xfff00)|READ_OP_ARG8();
		break;
	}
}

INLINE void saturn_add_adr(int reg)
{
	int t=saturn.d[reg]+READ_OP_ARG()+1;
	saturn.d[reg]=t&0xfffff;
	saturn.carry=t>=0x100000;
}

INLINE void saturn_sub_adr(int reg)
{
	int t=saturn.d[reg]-READ_OP_ARG()-1;
	saturn.d[reg]=t&0xfffff;
	saturn.carry=t<0;
}

INLINE void saturn_adr_to_reg(int adr, int reg)
{
	S64_WRITE_A(reg,saturn.d[adr]);
}

INLINE void saturn_reg_to_adr(int reg, int adr)
{
	saturn.d[adr]=S64_READ_A(reg);
}

INLINE void saturn_adr_to_reg_word(int adr, int reg)
{
	S64_WRITE_WORD(reg,0,saturn.d[adr]&0xffff);
}

INLINE void saturn_reg_to_adr_word(int reg, int adr)
{
	saturn.d[adr]=(saturn.d[adr]&0xf0000)|S64_READ_WORD(reg,0);
}

INLINE void saturn_exchange_adr_reg(int adr, int reg)
{
	int temp=saturn.d[adr];
	saturn.d[adr]=S64_READ_A(reg);
	S64_WRITE_A(reg,temp);
}

INLINE void saturn_exchange_adr_reg_word(int adr, int reg)
{
	int temp=saturn.d[adr]&0xffff;
	saturn.d[adr]=(saturn.d[adr]&0xf0000)|S64_READ_WORD(reg,0);
	S64_WRITE_WORD(reg,0,temp);
}

INLINE void saturn_load_nibbles(int reg, int begin, int count, int adr)
{
	int i;
	for (i=0; i<count; i++) {
		S64_WRITE_NIBBLE(reg,(begin+i)&0xf,READ_NIBBLE(saturn.d[adr]+i) );
		saturn_ICount-=2;
	}
}

INLINE void saturn_store_nibbles(int reg, int begin, int count, int adr)
{
	int i;
	for (i=0; i<count; i++) {
		WRITE_NIBBLE(saturn.d[adr]+i,S64_READ_NIBBLE(reg,(begin+i)&0xf) );
		saturn_ICount-=2;
	}
}

INLINE void saturn_clear_bit(int reg)
{
	switch(READ_OP_ARG()) {
	case 0: S64_BYTE( reg, 0)&=~1;break;
	case 1: S64_BYTE( reg, 0)&=~2;break;
	case 2: S64_BYTE( reg, 0)&=~4;break;
	case 3: S64_BYTE( reg, 0)&=~8;break;
	case 4: S64_BYTE( reg, 0)&=~0x10;break;
	case 5: S64_BYTE( reg, 0)&=~0x20;break;
	case 6: S64_BYTE( reg, 0)&=~0x40;break;
	case 7: S64_BYTE( reg, 0)&=~0x80;break;
	case 8: S64_BYTE( reg, 1)&=~1;break;
	case 9: S64_BYTE( reg, 1)&=~2;break;
	case 0xa: S64_BYTE( reg, 1)&=~4;break;
	case 0xb: S64_BYTE( reg, 1)&=~8;break;
	case 0xc: S64_BYTE( reg, 1)&=~0x10;break;
	case 0xd: S64_BYTE( reg, 1)&=~0x20;break;
	case 0xe: S64_BYTE( reg, 1)&=~0x40;break;
	case 0xf: S64_BYTE( reg, 1)&=~0x80;break;
	}
}

INLINE void saturn_set_bit(int reg)
{
	switch(READ_OP_ARG()) {
	case 0: S64_BYTE( reg, 0)|=1;break;
	case 1: S64_BYTE( reg, 0)|=2;break;
	case 2: S64_BYTE( reg, 0)|=4;break;
	case 3: S64_BYTE( reg, 0)|=8;break;
	case 4: S64_BYTE( reg, 0)|=0x10;break;
	case 5: S64_BYTE( reg, 0)|=0x20;break;
	case 6: S64_BYTE( reg, 0)|=0x40;break;
	case 7: S64_BYTE( reg, 0)|=0x80;break;
	case 8: S64_BYTE( reg, 1)|=1;break;
	case 9: S64_BYTE( reg, 1)|=2;break;
	case 0xa: S64_BYTE( reg, 1)|=4;break;
	case 0xb: S64_BYTE( reg, 1)|=8;break;
	case 0xc: S64_BYTE( reg, 1)|=0x10;break;
	case 0xd: S64_BYTE( reg, 1)|=0x20;break;
	case 0xe: S64_BYTE( reg, 1)|=0x40;break;
	case 0xf: S64_BYTE( reg, 1)|=0x80;break;
	}
}

/****************************************************************************
 clear opers
 ****************************************************************************/
INLINE void saturn_clear(int reg, int begin, int count)
{
	int i;
	for (i=0; i<count; i++) {
		S64_WRITE_NIBBLE(reg, (begin+i)&0xf, 0);
		saturn_ICount-=2;
	}
}

/****************************************************************************
 exchange opers
 ****************************************************************************/
INLINE void saturn_exchange(int left, int begin, int count, int right)
{
	int i;
	SaturnNib temp;
	for (i=0; i<count; i++) {
		temp=S64_READ_NIBBLE(left,(begin+i)&0xf);
		S64_WRITE_NIBBLE(left, (begin+i)&0xf, S64_READ_NIBBLE(right,(begin+i)&0xf));
		S64_WRITE_NIBBLE(right, (begin+i)&0xf, temp);
		saturn_ICount-=2;
	}
}

/****************************************************************************
 copy opers
 ****************************************************************************/
INLINE void saturn_copy(int dest, int begin, int count, int src)
{
	int i;
	for (i=0; i<count; i++) {
		S64_WRITE_NIBBLE(dest, (begin+i)&0xf, S64_READ_NIBBLE(src, (begin+i)&0xf));
		saturn_ICount-=2;
	}
}

/****************************************************************************
 add opers
 ****************************************************************************/
INLINE void saturn_add(int reg, int begin, int count, int right)
{
	int i, t=0;
	for (i=0; i<count; i++) {
		if (t>0x10) {
			t=S64_READ_NIBBLE(reg, (begin+i)&0xf)+1;
		} else {
			t=S64_READ_NIBBLE(reg, (begin+i)&0xf);
		}
		t+=S64_READ_NIBBLE(right, (begin+i)&0xf );
		S64_WRITE_NIBBLE(reg, (begin+i)&0xf , t&0x0f);
		saturn_ICount-=2;
	}
	saturn.carry=t==0x10;
}

INLINE void saturn_add_const(int reg, int begin, int count, SaturnNib right)
{
	int i, t=0;
	for (i=0; i<count; i++) {
		t=S64_READ_NIBBLE(reg, (begin+i)&0xf);
		t+=right;
		right=(right>>4)+1;
		S64_WRITE_NIBBLE(reg, (begin+i)&0xf, t&0x0f);
		saturn_ICount-=2;
		if (t<0x10) break;
	}
	saturn.carry=t>=0x10;
}

/****************************************************************************
 sub opers
 ****************************************************************************/
INLINE void saturn_sub(int reg, int begin, int count, int right)
{
	int i, t=0;
	for (i=0; i<count; i++) {
		if (t>0x10) {
			t=S64_READ_NIBBLE(reg, (begin+i)&0xf)-1;
		} else {
			t=S64_READ_NIBBLE(reg, (begin+i)&0xf );
		}
		t-=S64_READ_NIBBLE(right, (begin+i)&0xf );
		S64_WRITE_NIBBLE(reg, (begin+i)&0xf, t&0x0f);
		saturn_ICount-=2;
	}
	saturn.carry=t<0;
}

INLINE void saturn_sub_const(int reg, int begin, int count, int right)
{
	int i, t=0;
	for (i=0; i<count; i++) {
		t=S64_READ_NIBBLE(reg, (begin+i)&0xf );
		t-=right;
		right=(right>>4)+1;
		S64_WRITE_NIBBLE(reg, (begin+i)&0xf, t&0x0f);
		saturn_ICount-=2;
		if (t>=0) break;
	}
	saturn.carry=t<0;
}

/****************************************************************************
 sub2 opers (a=b-a)
 ****************************************************************************/
INLINE void saturn_sub2(int reg, int begin, int count, int right)
{
	int i, t=0;
	for (i=0; i<count; i++) {
		t=S64_READ_NIBBLE(reg, (begin+i)&0xf );
		t=S64_READ_NIBBLE(right, i)-t;
		S64_WRITE_NIBBLE(reg, (begin+i)&0xf, t&0x0f);
		saturn_ICount-=2;
		if (t>=0) break;
	}
	saturn.carry=t<0;
}

/****************************************************************************
 increment opers
 ****************************************************************************/
INLINE void saturn_increment(int reg, int begin, int count)
{
	int i, t=0;
	for (i=0; i<count; i++) {
		t=S64_READ_NIBBLE(reg, (begin+i)&0xf );
		t++;
		S64_WRITE_NIBBLE(reg, (begin+i)&0xf, t&0x0f);
		saturn_ICount-=2;
		if (t!=0x10) break;
	}
}

/****************************************************************************
 decrement opers
 ****************************************************************************/
INLINE void saturn_decrement(int reg, int begin, int count)
{
	int i, t=0;
	for (i=0; i<count; i++) {
		t=S64_READ_NIBBLE(reg, (begin+i)&0xf );
		t=(t-1)&0xf;
		S64_WRITE_NIBBLE(reg, (begin+i)&0xf, t);
		saturn_ICount-=2;
		if (t!=0) break;
	}
	saturn.carry=t==0;
}

/****************************************************************************
 invert (1 complement)  opers
 ****************************************************************************/
INLINE void saturn_invert(int reg, int begin, int count)
{
	int i;
	SaturnNib n;
	saturn.carry=1;
	for (i=0; i<count; i++) {
		S64_WRITE_NIBBLE(reg, (begin+i)&0xf, (n=S64_READ_NIBBLE(reg,(begin+i)&0xf)) ^ 0xf);
		saturn.carry=saturn.carry && (n==0);
		saturn_ICount-=2;
	}
}

/****************************************************************************
 negate (2 complement)  opers
 ****************************************************************************/
INLINE void	saturn_negate(int reg, int begin, int count)
{
	int i;
	SaturnNib n;
	saturn.carry=1;
	for (i=0; i<count; i++) {
		n=S64_READ_NIBBLE(reg, (begin+i)&0xf );
		saturn.carry=saturn.carry && (n==0);
		n=((n ^ 0xf)+1)&0xf;
		S64_WRITE_NIBBLE(reg, (begin+i)&0xf, n);
		saturn_ICount-=2;
	}
}

/****************************************************************************
 or opers
 ****************************************************************************/
INLINE void saturn_or(int dest, int begin, int count, int src)
{
	int i;
	for (i=0; i<count; i++) {
		S64_WRITE_NIBBLE(dest, (begin+i)&0xf,
						 S64_READ_NIBBLE(dest,(begin+i)&0xf)|S64_READ_NIBBLE(src,(begin+i)&0xf));
		saturn_ICount-=2;
	}
}

/****************************************************************************
 and opers
 ****************************************************************************/
INLINE void saturn_and(int dest, int begin, int count, int src)
{
	int i;
	for (i=0; i<count; i++) {
		S64_WRITE_NIBBLE(dest, (begin+i)&0xf,
						 S64_READ_NIBBLE(dest,(begin+i)&0xf)&S64_READ_NIBBLE(src,(begin+i)&0xf));
		saturn_ICount-=2;
	}
}

/****************************************************************************
 shift nibbles left opers
 ****************************************************************************/
INLINE void saturn_shift_nibble_left(int reg, int begin, int count)
{
	int i;
	for (i=count; i>1; i--) {
		S64_WRITE_NIBBLE(reg, (begin+i)&0xf, S64_READ_NIBBLE(reg,(begin+i-1)&0xf) );
		saturn_ICount-=2;
	}
	S64_WRITE_NIBBLE(reg, begin, 0);
	saturn_ICount-=2;
}

/****************************************************************************
 shift nibbles right opers
 ****************************************************************************/
INLINE void saturn_shift_nibble_right(int reg, int begin, int count)
{
	int i;
	for (i=1; i<count; i++) {
		S64_WRITE_NIBBLE(reg, (begin+i-1)&0xf, S64_READ_NIBBLE(reg,(begin+i)&0xf) );
		saturn_ICount-=2;
	}
	S64_WRITE_NIBBLE(reg, (begin+i-1)&0xf, 0);
	saturn_ICount-=2;
}

/****************************************************************************
 rotate nibble left opers
 ****************************************************************************/
INLINE void saturn_rotate_nibble_left_w(int reg)
{
	SaturnNib a=S64_READ_NIBBLE(reg, 15);
	S64_WRITE_W(reg, S64_READ_W(reg)<<4);
	S64_WRITE_NIBBLE(reg,0,a);
	saturn_ICount-=32;
}

INLINE void saturn_rotate_nibble_right_w(int reg)
{
	SaturnNib a=S64_READ_NIBBLE(reg,0);
	if (a) saturn.hst|=SB;
	S64_WRITE_W(reg, S64_READ_W(reg)>>4);
	S64_WRITE_NIBBLE(reg,15,a);
	saturn_ICount-=32;
}

/****************************************************************************
 shift right opers
 ****************************************************************************/
INLINE void saturn_shift_right(int reg, int begin, int count)
{
	int i, t, c=0;
	for (i=count; i>=count; i--) {
		t=S64_READ_NIBBLE(reg, (begin+i)&0xf );
		if (c) t|=0x10;
		c=t&1;
		S64_WRITE_NIBBLE(reg, (begin+i-1)&0xf, t>>1);
		saturn_ICount-=2;
	}
	if (c) saturn.hst|=SB;
	saturn_ICount-=2;
}


/****************************************************************************
 shift left opers, sets carry!
 ****************************************************************************/
INLINE void saturn_shift_left(int reg, int begin, int count)
{
	SaturnNib t;
	int i;
	saturn.carry=0;
	for (i=0; i<count; i++) {
		t=S64_READ_NIBBLE(reg,(begin+i)&0xf);
		S64_WRITE_NIBBLE(reg, (begin+i)&0xf, ((t<<1)&0xf)|saturn.carry);
		saturn.carry=t&8?1:0;
		saturn_ICount-=2;
	}
}
