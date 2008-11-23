#define IRQ_ADDRESS	0xf

#define saturn_assert(x) \
	do { if (!(x)) logerror("SATURN%d assertion failed: %s at %s:%i, pc=%05x\n", cpunum_get_active(), #x, __FILE__, __LINE__, saturn.pc); } while (0)

INLINE int READ_OP(void)
{
	UINT8 data;
	saturn_ICount-=3;
        data=memory_decrypted_read_byte(saturn.program, saturn.pc);
	saturn_assert(data<0x10);
	saturn.pc=(saturn.pc+1)&0xfffff;
	return data;
}

INLINE int READ_OP_ARG(void)
{
	UINT8 data;
	saturn_ICount-=3;
        data=memory_raw_read_byte(saturn.program, saturn.pc);
	saturn_assert(data<0x10);
	saturn.pc=(saturn.pc+1)&0xfffff;
	return data;
}

INLINE int READ_OP_ARG8(void)
{
	int n0=READ_OP_ARG();
	int n1=READ_OP_ARG();
	return n0|(n1<<4);
}

INLINE INT8 READ_OP_DIS8(void)
{
	return (INT8)READ_OP_ARG8();
}

INLINE int READ_OP_ARG12(void)
{
	int n0=READ_OP_ARG();
	int n1=READ_OP_ARG();
	int n2=READ_OP_ARG();
	return n0|(n1<<4)|(n2<<8);
}

INLINE int READ_OP_DIS12(void)
{
	int temp=READ_OP_ARG12();
	if (temp&0x800) temp-=0x1000;
	return temp;
}

INLINE int READ_OP_ARG16(void)
{
	int n0=READ_OP_ARG();
	int n1=READ_OP_ARG();
	int n2=READ_OP_ARG();
	int n3=READ_OP_ARG();
	return n0|(n1<<4)|(n2<<8)|(n3<<12);
}

INLINE INT16 READ_OP_DIS16(void)
{
	return (INT16)READ_OP_ARG16();
}

INLINE int READ_OP_ARG20(void)
{
	int n0=READ_OP_ARG();
	int n1=READ_OP_ARG();
	int n2=READ_OP_ARG();
	int n3=READ_OP_ARG();
	int n4=READ_OP_ARG();
	return n0|(n1<<4)|(n2<<8)|(n3<<12)|(n4<<16);
}

INLINE int READ_NIBBLE(SaturnAdr adr)
{
	UINT8 data;
	saturn_ICount-=3;
	data=memory_read_byte(saturn.program, adr&0xfffff);
	saturn_assert(data<0x10);
	if (saturn.config&&saturn.config->crc) saturn.config->crc(Machine, adr&0xfffff, data);
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
	return READ_NIBBLE(adr)|(READ_NIBBLE(adr+1)<<4)|(READ_NIBBLE(adr+2)<<8)|(READ_NIBBLE(adr+3)<<12);
}

INLINE int READ_20(SaturnAdr adr)
{
	return READ_NIBBLE(adr)|(READ_NIBBLE(adr+1)<<4)|(READ_NIBBLE(adr+2)<<8)|(READ_NIBBLE(adr+3)<<12)|(READ_NIBBLE(adr+4)<<16);
}

INLINE void WRITE_NIBBLE(SaturnAdr adr, SaturnNib nib)
{
	saturn_ICount-=3;
	saturn_assert(nib<0x10);
	memory_write_byte(saturn.program, adr&0xfffff,nib);
}

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


INLINE int S64_READ_X(int r)
{
	return saturn.reg[r][0]|(saturn.reg[r][1]<<4)|(saturn.reg[r][2]<<8);
}

INLINE int S64_READ_WORD(int r)
{
	return saturn.reg[r][0]|(saturn.reg[r][1]<<4)|(saturn.reg[r][2]<<8)|(saturn.reg[r][3]<<12);
}

INLINE int S64_READ_A(int r)
{
	return saturn.reg[r][0]|(saturn.reg[r][1]<<4)|(saturn.reg[r][2]<<8)|(saturn.reg[r][3]<<12)|(saturn.reg[r][4]<<16);
}

INLINE void S64_WRITE_X(int r, int v)
{
	saturn.reg[r][0]=v&0xf;
	saturn.reg[r][1]=(v>>4)&0xf;
	saturn.reg[r][2]=(v>>8)&0xf;
}

INLINE void S64_WRITE_WORD(int r, int v)
{
	saturn.reg[r][0]=v&0xf;
	saturn.reg[r][1]=(v>>4)&0xf;
	saturn.reg[r][2]=(v>>8)&0xf;
	saturn.reg[r][3]=(v>>12)&0xf;
}

INLINE void S64_WRITE_A(int r, int v)
{
	saturn.reg[r][0]=v&0xf;
	saturn.reg[r][1]=(v>>4)&0xf;
	saturn.reg[r][2]=(v>>8)&0xf;
	saturn.reg[r][3]=(v>>12)&0xf;
	saturn.reg[r][4]=(v>>16)&0xf;
}





INLINE SaturnAdr saturn_pop(void)
{
	SaturnAdr temp=saturn.rstk[0];
	memmove(saturn.rstk, saturn.rstk+1, sizeof(saturn.rstk)-sizeof(saturn.rstk[0]));
	saturn.rstk[7]=0;
	return temp;
}

INLINE void saturn_push(SaturnAdr adr)
{
	memmove(saturn.rstk+1, saturn.rstk, sizeof(saturn.rstk)-sizeof(saturn.rstk[0]));
	saturn.rstk[0]=adr;
}

INLINE void saturn_interrupt_on(void)
{
	LOG(( "SATURN#%d at %05x: INTON\n", cpunum_get_active(), saturn.pc-4 ));
	saturn.irq_enable=1;
	if (saturn.irq_state)
	{
		LOG(( "SATURN#%d set_irq_line(ASSERT)\n", cpunum_get_active()));
		saturn.pending_irq=1;
	}
}

INLINE void saturn_interrupt_off(void)
{
	LOG(( "SATURN#%d at %05x: INTOFF\n", cpunum_get_active(), saturn.pc-4 ));
	saturn.irq_enable=0;
}

INLINE void saturn_reset_interrupt(void)
{
	LOG(( "SATURN#%d at %05x: RSI\n", cpunum_get_active(), saturn.pc-5 ));
	if (saturn.config&&saturn.config->rsi) saturn.config->rsi(Machine);
}

INLINE void saturn_mem_reset(void)
{
	if (saturn.config&&saturn.config->reset) saturn.config->reset(Machine);
}

INLINE void saturn_mem_config(void)
{
	if (saturn.config&&saturn.config->config) saturn.config->config(Machine, S64_READ_A(C));
}

INLINE void saturn_mem_unconfig(void)
{
	if (saturn.config&&saturn.config->unconfig) saturn.config->unconfig(Machine, S64_READ_A(C));
}

static int monitor_id;

INLINE void saturn_mem_id(void)
{
	int id=0;
	if (saturn.config&&saturn.config->id) id=saturn.config->id(Machine);
	S64_WRITE_A(C,id);
	monitor_id = id;
}

INLINE void saturn_shutdown(void)
{
	saturn.sleeping=1;
	saturn.irq_enable=1;
	LOG(( "SATURN#%d at %05x: SHUTDN\n", cpunum_get_active(), saturn.pc-3 ));
}

INLINE void saturn_bus_command_b(void)
{
	logerror( "SATURN#%d at %05x: BUSCB opcode not handled\n", cpunum_get_active(), saturn.pc-4 );
}

INLINE void saturn_bus_command_c(void)
{
	logerror( "SATURN#%d at %05x: BUSCC opcode not handled\n", cpunum_get_active(), saturn.pc-3 );
}

INLINE void saturn_bus_command_d(void)
{
	logerror( "SATURN#%d at %05x: BUSCD opcode not handled\n", cpunum_get_active(), saturn.pc-4 );
}

INLINE void saturn_serial_request(void)
{
	logerror( "SATURN#%d at %05x: SREQ? opcode not handled\n", cpunum_get_active(), saturn.pc-3 );
}

INLINE void saturn_out_c(void)
{
	saturn.out=S64_READ_X(C);
	if (saturn.config&&saturn.config->out) saturn.config->out(Machine, saturn.out);
}

INLINE void saturn_out_cs(void)
{
	saturn.out=(saturn.out&0xff0)|saturn.reg[C][0];
	if (saturn.config&&saturn.config->out) saturn.config->out(Machine, saturn.out);
}

static int monitor_in;

INLINE void saturn_in(int reg)
{
	int in = 0;
	saturn_assert(reg>=0 && reg<9);
	if (!(saturn.pc&1))
		logerror( "SATURN#%d at %05x: reg=IN opcode at odd addresse\n",
			  cpunum_get_active(), saturn.pc-3 );
	if (saturn.config&&saturn.config->in) in = saturn.config->in(Machine);
	S64_WRITE_WORD(reg,in);
	monitor_in = in;
}

INLINE void saturn_sethex(void) { saturn.decimal=0; }
INLINE void saturn_setdec(void) { saturn.decimal=1; }

/* st related */
INLINE void saturn_clear_st(void)
{
	saturn.st&=0xf000;
}

INLINE void saturn_st_to_c(void)
{
	S64_WRITE_X(C,saturn.st);
}

INLINE void saturn_c_to_st(void)
{
	saturn.st=(saturn.st&0xf000)|(S64_READ_X(C));
}

INLINE void saturn_exchange_c_st(void)
{
	int t=saturn.st;
	saturn.st=(t&0xf000)|(S64_READ_X(C));
	S64_WRITE_X(C,t);
}

INLINE void saturn_jump_after_test(void)
{
	int adr=READ_OP_DIS8();
	if (saturn.carry) {
		if (adr==0) {
			saturn.pc=saturn_pop();
		} else {
			saturn.pc=(saturn.pc+adr-2)&0xfffff;
		}
		change_pc(saturn.pc);
	}
}
INLINE void saturn_st_clear_bit(void)
{
	saturn.st &= ~(1<<(READ_OP_ARG()));
}

INLINE void saturn_st_set_bit(void)
{
	saturn.st |= (1<<(READ_OP_ARG()));
}

INLINE void saturn_st_jump_bit_clear(void)
{
	saturn.carry=!((saturn.st>>(READ_OP_ARG()))&1);
	saturn_jump_after_test();
}

INLINE void saturn_st_jump_bit_set(void)
{
	saturn.carry=(saturn.st>>(READ_OP_ARG()))&1;
	saturn_jump_after_test();
}

INLINE void saturn_hst_clear_bits(void)
{
	saturn.hst&=~(READ_OP_ARG());
}

INLINE void saturn_hst_bits_cleared(void)
{
	saturn.carry=!(saturn.hst&(READ_OP_ARG()));
	saturn_jump_after_test();
}

/* p related */
INLINE void saturn_exchange_p(void)
{
	int nr=READ_OP_ARG();
	int t=saturn.p;
	saturn.p=saturn.reg[C][nr];
	saturn.reg[C][nr]=t;
}

INLINE void saturn_p_to_c(void)
{
	int nr=READ_OP_ARG();
	saturn.reg[C][nr]=saturn.p;
}

INLINE void saturn_c_to_p(void)
{
	int nr=READ_OP_ARG();
	saturn.p=saturn.reg[C][nr];
}

INLINE void saturn_dec_p(void)
{
	saturn.carry=saturn.p==0;
	saturn.p=(saturn.p-1)&0xf;
}

INLINE void saturn_inc_p(void)
{
	saturn.p=(saturn.p+1)&0xf;
	saturn.carry=saturn.p==0;
}

INLINE void saturn_load_p(void)
{
	saturn.p=READ_OP_ARG();
}

INLINE void saturn_p_equals(void)
{
	saturn.carry=saturn.p==(READ_OP_ARG());
	saturn_jump_after_test();
}

INLINE void saturn_p_not_equals(void)
{
	saturn.carry=saturn.p!=(READ_OP_ARG());
	saturn_jump_after_test();
}

INLINE void saturn_ca_p_1(void)
{
	int a=(S64_READ_A(C))+1+saturn.p;
	saturn.carry=a>=0x100000;
	S64_WRITE_A(C,a&0xfffff);
}

INLINE void saturn_load_reg(int reg)
{
	int count=READ_OP_ARG();
	int pos=saturn.p;
	saturn_assert(reg>=0 && reg<9);
	for (; count>=0; count--, pos=(pos+1)&0xf ) {
		saturn.reg[reg][pos]=READ_OP_ARG();
	}
}

INLINE void saturn_jump(int adr, int jump)
{
	saturn_assert(adr>=0 && adr<0x100000);
	if (jump) {
		saturn.pc=adr;
		saturn_ICount-=10;
		change_pc(saturn.pc);
	}
}

INLINE void saturn_call(int adr)
{
	saturn_assert(adr>=0 && adr<0x100000);
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
	LOG(( "SATURN#%d at %05x: RTI\n", cpunum_get_active(), saturn.pc-2 ));
	saturn.in_irq=0; /* set to 1 when an IRQ is taken */
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
	saturn_assert(reg>=0 && reg<9);
	saturn.pc=READ_20(S64_READ_A(reg));
	change_pc(saturn.pc);
}

INLINE void saturn_equals_zero(int reg, int begin, int count)
{
	int i, t;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(begin>=0 && count>=0 && begin+count<=16);
	saturn.carry=1;
	for (i=0; i<count; i++) {
		t=saturn.reg[reg][begin+i];
		if (t!=0) { saturn.carry=0; break; }
		saturn_ICount-=2;
	}
	saturn_jump_after_test();
}

INLINE void saturn_equals(int reg, int begin, int count, int right)
{
	int i, t,t2;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(right>=0 && right<9);
	saturn_assert(begin>=0 && count>=0 && begin+count<=16);
	saturn.carry=1;
	for (i=0; i<count; i++) {
		t=saturn.reg[reg][begin+i];
		t2=saturn.reg[right][begin+i];
		if (t!=t2) { saturn.carry=0; break; }
		saturn_ICount-=2;
	}
	saturn_jump_after_test();
}

INLINE void saturn_not_equals_zero(int reg, int begin, int count)
{
	int i, t;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(begin>=0 && count>=0 && begin+count<=16);
	saturn.carry=0;
	for (i=0; i<count; i++) {
		t=saturn.reg[reg][begin+i];
		if (t!=0) { saturn.carry=1; break; }
		saturn_ICount-=2;
	}
	saturn_jump_after_test();
}

INLINE void saturn_not_equals(int reg, int begin, int count, int right)
{
	int i, t,t2;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(right>=0 && right<9);
	saturn_assert(begin>=0 && count>=0 && begin+count<=16);
	saturn.carry=0;
	for (i=0; i<count; i++) {
		t=saturn.reg[reg][begin+i];
		t2=saturn.reg[right][begin+i];
		if (t!=t2) { saturn.carry=1; break; }
		saturn_ICount-=2;
	}
	saturn_jump_after_test();
}

INLINE void saturn_greater(int reg, int begin, int count, int right)
{
	int i, t,t2;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(right>=0 && right<9);
	saturn_assert(begin>=0 && count>=0 && begin+count<=16);
	saturn.carry=0;
	for (i=count-1; i>=0; i--) {
		t=saturn.reg[reg][begin+i];
		t2=saturn.reg[right][begin+i];
		if (t>t2) { saturn.carry=1; break; }
		if (t<t2) break;
		saturn_ICount-=2;
	}
	saturn_jump_after_test();
}

INLINE void saturn_greater_equals(int reg, int begin, int count, int right)
{
	int i, t,t2;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(right>=0 && right<9);
	saturn_assert(begin>=0 && count>=0 && begin+count<=16);
	saturn.carry=1;
	for (i=count-1; i>=0; i--) {
		t=saturn.reg[reg][begin+i];
		t2=saturn.reg[right][begin+i];
		if (t<t2) { saturn.carry=0; break; }
		if (t>t2) break;
		saturn_ICount-=2;
	}
	saturn_jump_after_test();
}

INLINE void saturn_smaller_equals(int reg, int begin, int count, int right)
{
	int i, t,t2;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(right>=0 && right<9);
	saturn_assert(begin>=0 && count>=0 && begin+count<=16);
	saturn.carry=1;
	for (i=count-1; i>=0; i--) {
		t=saturn.reg[reg][begin+i];
		t2=saturn.reg[right][begin+i];
		if (t>t2) { saturn.carry=0; break; }
		if (t<t2) break;
		saturn_ICount-=2;
	}
	saturn_jump_after_test();
}

INLINE void saturn_smaller(int reg, int begin, int count, int right)
{
	int i, t,t2;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(right>=0 && right<9);
	saturn_assert(begin>=0 && count>=0 && begin+count<=16);
	saturn.carry=0;
	for (i=count-1; i>=0; i--) {
		t=saturn.reg[reg][begin+i];
		t2=saturn.reg[right][begin+i];
		if (t<t2) { saturn.carry=1; break; }
		if (t>t2) break;
		saturn_ICount-=2;
	}
	saturn_jump_after_test();
}

INLINE void saturn_jump_bit_clear(int reg)
{
	int op=READ_OP_ARG();
	saturn_assert(reg>=0 && reg<9);
	saturn.carry=!((saturn.reg[reg][op>>2]>>(op&3))&1);
	saturn_jump_after_test();
}

INLINE void saturn_jump_bit_set(int reg)
{
	int op=READ_OP_ARG();
	saturn_assert(reg>=0 && reg<9);
	saturn.carry=(saturn.reg[reg][op>>2]>>(op&3))&1;
	saturn_jump_after_test();
}

INLINE void saturn_load_pc(int reg)
{
	saturn_assert(reg>=0 && reg<9);
	saturn.pc=S64_READ_A(reg);
	change_pc(saturn.pc);
}

INLINE void saturn_store_pc(int reg)
{
	saturn_assert(reg>=0 && reg<9);
	S64_WRITE_A(reg,saturn.pc);
}

INLINE void saturn_exchange_pc(int reg)
{
	int temp=saturn.pc;
	saturn_assert(reg>=0 && reg<9);
	saturn.pc=S64_READ_A(reg);
	change_pc(saturn.pc);
	S64_WRITE_A(reg, temp);
}

/*************************************************************************************
 address register related
*************************************************************************************/
INLINE void saturn_load_adr(int reg, int nibbles)
{
	saturn_assert(reg>=0 && reg<2);
	saturn_assert(nibbles==2 || nibbles==4 || nibbles==5);
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
	saturn_assert(reg>=0 && reg<2);
	saturn.d[reg]=t&0xfffff;
	saturn.carry=t>=0x100000;
}

INLINE void saturn_sub_adr(int reg)
{
	int t=saturn.d[reg]-READ_OP_ARG()-1;
	saturn_assert(reg>=0 && reg<2);
	saturn.d[reg]=t&0xfffff;
	saturn.carry=t<0;
}

INLINE void saturn_adr_to_reg(int adr, int reg)
{
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(adr>=0 && adr<2);
	S64_WRITE_A(reg,saturn.d[adr]);
}

INLINE void saturn_reg_to_adr(int reg, int adr)
{
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(adr>=0 && adr<2);
	saturn.d[adr]=S64_READ_A(reg);
}

INLINE void saturn_adr_to_reg_word(int adr, int reg)
{
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(adr>=0 && adr<2);
	S64_WRITE_WORD(reg,saturn.d[adr]&0xffff);
}

INLINE void saturn_reg_to_adr_word(int reg, int adr)
{
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(adr>=0 && adr<2);
	saturn.d[adr]=(saturn.d[adr]&0xf0000)|S64_READ_WORD(reg);
}

INLINE void saturn_exchange_adr_reg(int adr, int reg)
{
	int temp=saturn.d[adr];
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(adr>=0 && adr<2);
	saturn.d[adr]=S64_READ_A(reg);
	S64_WRITE_A(reg,temp);
}

INLINE void saturn_exchange_adr_reg_word(int adr, int reg)
{
	int temp=saturn.d[adr]&0xffff;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(adr>=0 && adr<2);
	saturn.d[adr]=(saturn.d[adr]&0xf0000)|S64_READ_WORD(reg);
	S64_WRITE_WORD(reg,temp);
}

INLINE void saturn_load_nibbles(int reg, int begin, int count, int adr)
{
	int i;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(adr>=0 && adr<2);
	saturn_assert(begin>=0 && count>=0 && begin+count<=16);
	for (i=0; i<count; i++) {
		saturn.reg[reg][begin+i]=READ_NIBBLE(saturn.d[adr]+i);
		saturn_ICount-=2;
	}
}

INLINE void saturn_store_nibbles(int reg, int begin, int count, int adr)
{
	int i;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(adr>=0 && adr<2);
	saturn_assert(begin>=0 && count>=0 && begin+count<=16);
	for (i=0; i<count; i++) {
		WRITE_NIBBLE((saturn.d[adr]+i)&0xfffff,saturn.reg[reg][begin+i]);
		saturn_ICount-=2;
	}
}

INLINE void saturn_clear_bit(int reg)
{
	int arg=READ_OP_ARG();
	saturn_assert(reg>=0 && reg<9);
	saturn.reg[reg][arg>>2]&=~(1<<(arg&3));
}

INLINE void saturn_set_bit(int reg)
{
	int arg=READ_OP_ARG();
	saturn_assert(reg>=0 && reg<9);
	saturn.reg[reg][arg>>2]|=1<<(arg&3);
}

/****************************************************************************
 clear opers
 ****************************************************************************/
INLINE void saturn_clear(int reg, int begin, int count)
{
	int i;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(begin>=0 && count>=0 && begin+count<=16);
	for (i=0; i<count; i++) {
		saturn.reg[reg][begin+i]=0;
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
	saturn_assert(left>=0 && left<9);
	saturn_assert(right>=0 && right<9);
	saturn_assert(begin>=0 && count>=0 && begin+count<=16);
	for (i=0; i<count; i++) {
		temp=saturn.reg[left][begin+i];
		saturn.reg[left][begin+i]=saturn.reg[right][begin+i];
		saturn.reg[right][begin+i]=temp;
		saturn_ICount-=2;
	}
}

/****************************************************************************
 copy opers
 ****************************************************************************/
INLINE void saturn_copy(int dest, int begin, int count, int src)
{
	int i;
	saturn_assert(dest>=0 && dest<9);
	saturn_assert(src>=0 && src<9);
	saturn_assert(begin>=0 && count>=0 && begin+count<=16);
	for (i=0; i<count; i++) {
		saturn.reg[dest][begin+i]=saturn.reg[src][begin+i];
		saturn_ICount-=2;
	}
}

/****************************************************************************
 add opers
 ****************************************************************************/
INLINE void saturn_add(int reg, int begin, int count, int right)
{
	int i, t;
	int base=saturn.decimal?10:16;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(right>=0 && right<9);
	saturn_assert(begin>=0 && count>=0 && begin+count<=16);
	saturn.carry=0;
	for (i=0; i<count; i++) {
		t=saturn.reg[reg][begin+i];
		t+=saturn.reg[right][begin+i];
		t+=saturn.carry;
		if (t>=base) {
			saturn.carry=1;
			t-=base;
		}
		else saturn.carry=0;
		saturn_assert(t>=0); saturn_assert(t<base);
		saturn.reg[reg][begin+i]=t&0xf;
		saturn_ICount-=2;
	}
}

INLINE void saturn_add_const(int reg, int begin, int count, SaturnNib right)
{
	int i, t;
	int base=saturn.decimal?10:16;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(begin>=0 && count>=0 && begin+count<=16);
	saturn_assert(count>1 || !saturn.decimal); /* SATURN bug */
	for (i=0; i<count; i++) {
		t=saturn.reg[reg][begin+i];
		t+=(right&0xf);
		right>>=4;
		if (t>=base) {
			right++;
			t-=base;
		}
		saturn_assert(t>=0); saturn_assert(t<base);
		saturn.reg[reg][begin+i]=t&0xf;
		saturn_ICount-=2;
		if (!right) break;
	}
	saturn.carry=right>0;
}

/****************************************************************************
 sub opers
 ****************************************************************************/
INLINE void saturn_sub(int reg, int begin, int count, int right)
{
	int i, t;
	int base=saturn.decimal?10:16;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(right>=0 && right<9);
	saturn_assert(begin>=0 && count>=0 && begin+count<=16);
	saturn.carry=0;
	for (i=0; i<count; i++) {
		t=saturn.reg[reg][begin+i];
		t-=saturn.reg[right][begin+i];
		t-=saturn.carry;
		if (t<0) {
			saturn.carry=1;
			t+=base;
		}
		else saturn.carry=0;
		saturn_assert(t>=0); saturn_assert(t<base);
		saturn.reg[reg][begin+i]=t&0xf;
		saturn_ICount-=2;
	}
}

INLINE void saturn_sub_const(int reg, int begin, int count, int right)
{
	int i, t;
	int base=saturn.decimal?10:16;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(begin>=0 && count>=0 && begin+count<=16);
	saturn_assert(count>1 || !saturn.decimal); /* SATURN bug */
	for (i=0; i<count; i++) {
		t=saturn.reg[reg][begin+i];
		t-=(right&0xf);
		right>>=4;
		if (t<0) {
			right++;
			t+=base;
		}
		saturn_assert(t>=0); saturn_assert(t<base);
		saturn.reg[reg][begin+i]=t&0xf;
		saturn_ICount-=2;
		if (!right) break;
	}
	saturn.carry=right>0;
}

/****************************************************************************
 sub2 opers (a=b-a)
 ****************************************************************************/
INLINE void saturn_sub2(int reg, int begin, int count, int right)
{
	int i, t;
	int base=saturn.decimal?10:16;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(right>=0 && right<9);
	saturn_assert(begin>=0 && count>=0 && begin+count<=16);
	saturn.carry=0;
	for (i=0; i<count; i++) {
		t=saturn.reg[right][begin+i];
		t-=saturn.reg[reg][begin+i];
		t-=saturn.carry;
		if (t<0) {
			saturn.carry=1;
			t+=base;
		}
		else saturn.carry=0;
		saturn_assert(t>=0); saturn_assert(t<base);
		saturn.reg[reg][begin+i]=t&0xf;
		saturn_ICount-=2;
	}
}

/****************************************************************************
 increment opers
 ****************************************************************************/
INLINE void saturn_increment(int reg, int begin, int count)
{
	int i, t=0;
	int base=saturn.decimal?10:16;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(begin>=0 && count>=0 && begin+count<=16);
	for (i=0; i<count; i++) {
		saturn_ICount-=2;
		t=saturn.reg[reg][begin+i];
		t++;
		if (t>=base) saturn.reg[reg][begin+i]=t-base;
		else { saturn.reg[reg][begin+i]=t; break; }
	}
	saturn.carry=t>=base;
}

/****************************************************************************
 decrement opers
 ****************************************************************************/
INLINE void saturn_decrement(int reg, int begin, int count)
{
	int i, t=0;
	int base=saturn.decimal?10:16;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(begin>=0 && count>=0 && begin+count<=16);
	for (i=0; i<count; i++) {
		saturn_ICount-=2;
		t=saturn.reg[reg][begin+i];
		t--;
		if (t<0) saturn.reg[reg][begin+i]=t+base;
		else { saturn.reg[reg][begin+i]=t; break; }
	}
	saturn.carry=t<0;
}

/****************************************************************************
 invert (1 complement)  opers
 ****************************************************************************/
INLINE void saturn_invert(int reg, int begin, int count)
{
	int i;
	int max=saturn.decimal?9:15;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(begin>=0 && count>=0 && begin+count<=16);
	saturn.carry=0;
	for (i=0; i<count; i++) {
		saturn.reg[reg][begin+i]=(max-saturn.reg[reg][begin+i])&0xf;
		saturn_ICount-=2;
	}
}

/****************************************************************************
 negate (2 complement)  opers
 ****************************************************************************/
INLINE void saturn_negate(int reg, int begin, int count)
{
	int i, n, c;
	int max=saturn.decimal?9:15;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(begin>=0 && count>=0 && begin+count<=16);
	c=1;
	saturn.carry=0;
	for (i=0; i<count; i++) {
		n=saturn.reg[reg][begin+i];
		if (n) saturn.carry=1;
		n=max+c-n;
		if (n>max) n-=max+1;
		else c=0;
		saturn_assert(n>=0); saturn_assert(n<=max);
		saturn.reg[reg][begin+i]=n&0xf;
		saturn_ICount-=2;
	}
}

/****************************************************************************
 or opers
 ****************************************************************************/
INLINE void saturn_or(int dest, int begin, int count, int src)
{
	int i;
	saturn_assert(dest>=0 && dest<9);
	saturn_assert(src>=0 && src<9);
	saturn_assert(begin>=0 && count>=0 && begin+count<=16);
	for (i=0; i<count; i++) {
		saturn.reg[dest][begin+i]|=saturn.reg[src][begin+i];
		saturn_ICount-=2;
	}
}

/****************************************************************************
 and opers
 ****************************************************************************/
INLINE void saturn_and(int dest, int begin, int count, int src)
{
	int i;
	saturn_assert(dest>=0 && dest<9);
	saturn_assert(src>=0 && src<9);
	saturn_assert(begin>=0 && count>=0 && begin+count<=16);
	for (i=0; i<count; i++) {
		saturn.reg[dest][begin+i]&=saturn.reg[src][begin+i];
		saturn_ICount-=2;
	}
}

/****************************************************************************
 shift nibbles left opers
 ****************************************************************************/
INLINE void saturn_shift_nibble_left(int reg, int begin, int count)
{
	int i;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(begin>=0 && count>=0 && begin+count<=16);
	if (saturn.reg[reg][begin+count-1]) saturn.hst|=SB;
	for (i=count-1; i>=1; i--) {
		saturn.reg[reg][begin+i]=saturn.reg[reg][begin+i-1];
		saturn_ICount-=2;
	}
	saturn.reg[reg][begin]=0;
	saturn_ICount-=2;
}

/****************************************************************************
 shift nibbles right opers
 ****************************************************************************/
INLINE void saturn_shift_nibble_right(int reg, int begin, int count)
{
	int i;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(begin>=0 && count>=0 && begin+count<=16);
	if (saturn.reg[reg][begin]) saturn.hst|=SB;
	for (i=1; i<count; i++) {
		saturn.reg[reg][begin+i-1]=saturn.reg[reg][begin+i];
		saturn_ICount-=2;
	}
	saturn.reg[reg][begin+count-1]=0;
	saturn_ICount-=2;
}


/****************************************************************************
 rotate nibbles left opers
 ****************************************************************************/
INLINE void saturn_rotate_nibble_left_w(int reg)
{
	int i, x=saturn.reg[reg][15];
	saturn_assert(reg>=0 && reg<9);
	for (i=15; i>=1; i--) {
		saturn.reg[reg][i]=saturn.reg[reg][i-1];
		saturn_ICount-=2;
	}
	saturn.reg[reg][0]=x;
	saturn_ICount-=2;
}

/****************************************************************************
 rotate nibbles right opers
 ****************************************************************************/
INLINE void saturn_rotate_nibble_right_w(int reg)
{
	int i, x=saturn.reg[reg][0];
	saturn_assert(reg>=0 && reg<9);
	for (i=1; i<16; i++) {
		saturn.reg[reg][i-1]=saturn.reg[reg][i];
		saturn_ICount-=2;
	}
	saturn.reg[reg][15]=x;
	if (x) saturn.hst|=SB;
	saturn_ICount-=2;
}


/****************************************************************************
 shift right opers
 ****************************************************************************/
INLINE void saturn_shift_right(int reg, int begin, int count)
{
	int i, t, c=0;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(begin>=0 && count>=0 && begin+count<=16);
	for (i=count-1; i>=0; i--) {
		t=saturn.reg[reg][begin+i];
		t|=(c<<4);
		c=t&1;
		saturn.reg[reg][begin+i]=t>>1;
		saturn_ICount-=2;
	}
	if (c) saturn.hst|=SB;
	saturn_ICount-=2;
}
