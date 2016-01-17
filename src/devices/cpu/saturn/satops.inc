// license:BSD-3-Clause
// copyright-holders:Peter Trauner,Antoine Mine
#define IRQ_ADDRESS 0xf

#define saturn_assert(x) \
	do { if (!(x)) logerror("SATURN '%s' assertion failed: %s at %s:%i, pc=%05x\n", tag().c_str(), #x, __FILE__, __LINE__, m_pc); } while (0)

int saturn_device::READ_OP()
{
	UINT8 data;
	m_icount-=3;
		data=m_direct->read_byte(m_pc);
	saturn_assert(data<0x10);
	m_pc=(m_pc+1)&0xfffff;
	return data;
}

int saturn_device::READ_OP_ARG()
{
	UINT8 data;
	m_icount-=3;
		data=m_direct->read_byte(m_pc);
	saturn_assert(data<0x10);
	m_pc=(m_pc+1)&0xfffff;
	return data;
}

int saturn_device::READ_OP_ARG8()
{
	int n0=READ_OP_ARG();
	int n1=READ_OP_ARG();
	return n0|(n1<<4);
}

INT8 saturn_device::READ_OP_DIS8()
{
	return (INT8)READ_OP_ARG8();
}

int saturn_device::READ_OP_ARG12()
{
	int n0=READ_OP_ARG();
	int n1=READ_OP_ARG();
	int n2=READ_OP_ARG();
	return n0|(n1<<4)|(n2<<8);
}

int saturn_device::READ_OP_DIS12()
{
	int temp=READ_OP_ARG12();
	if (temp&0x800) temp-=0x1000;
	return temp;
}

int saturn_device::READ_OP_ARG16()
{
	int n0=READ_OP_ARG();
	int n1=READ_OP_ARG();
	int n2=READ_OP_ARG();
	int n3=READ_OP_ARG();
	return n0|(n1<<4)|(n2<<8)|(n3<<12);
}

INT16 saturn_device::READ_OP_DIS16()
{
	return (INT16)READ_OP_ARG16();
}

int saturn_device::READ_OP_ARG20()
{
	int n0=READ_OP_ARG();
	int n1=READ_OP_ARG();
	int n2=READ_OP_ARG();
	int n3=READ_OP_ARG();
	int n4=READ_OP_ARG();
	return n0|(n1<<4)|(n2<<8)|(n3<<12)|(n4<<16);
}

int saturn_device::READ_NIBBLE(UINT32 adr)
{
	UINT8 data;
	m_icount-=3;
	data=m_program->read_byte(adr&0xfffff);
	saturn_assert(data<0x10);
	m_crc_func(adr&0xfffff, data, 0xffffffff);
	return data;
}

int saturn_device::READ_8(UINT32 adr)
{
	int n0=READ_NIBBLE(adr);
	int n1=READ_NIBBLE(adr+1);
	return n0|(n1<<4);
}

int saturn_device::READ_12(UINT32 adr)
{
	int n0=READ_NIBBLE(adr);
	int n1=READ_NIBBLE(adr+1);
	int n2=READ_NIBBLE(adr+2);
	return n0|(n1<<4)|(n2<<8);
}

int saturn_device::READ_16(UINT32 adr)
{
	int n0=READ_NIBBLE(adr);
	int n1=READ_NIBBLE(adr+1);
	int n2=READ_NIBBLE(adr+2);
	int n3=READ_NIBBLE(adr+3);
	return n0|(n1<<4)|(n2<<8)|(n3<<12);
}

int saturn_device::READ_20(UINT32 adr)
{
	int n0=READ_NIBBLE(adr);
	int n1=READ_NIBBLE(adr+1);
	int n2=READ_NIBBLE(adr+2);
	int n3=READ_NIBBLE(adr+3);
	int n4=READ_NIBBLE(adr+4);
	return n0|(n1<<4)|(n2<<8)|(n3<<12)|(n4<<16);
}

void saturn_device::WRITE_NIBBLE(UINT32 adr, UINT8 nib)
{
	m_icount-=3;
	saturn_assert(nib<0x10);
	m_program->write_byte(adr&0xfffff,nib);
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


int saturn_device::S64_READ_X(int r)
{
	return m_reg[r][0]|(m_reg[r][1]<<4)|(m_reg[r][2]<<8);
}

int saturn_device::S64_READ_WORD(int r)
{
	return m_reg[r][0]|(m_reg[r][1]<<4)|(m_reg[r][2]<<8)|(m_reg[r][3]<<12);
}

int saturn_device::S64_READ_A(int r)
{
	return m_reg[r][0]|(m_reg[r][1]<<4)|(m_reg[r][2]<<8)|(m_reg[r][3]<<12)|(m_reg[r][4]<<16);
}

void saturn_device::S64_WRITE_X(int r, int v)
{
	m_reg[r][0]=v&0xf;
	m_reg[r][1]=(v>>4)&0xf;
	m_reg[r][2]=(v>>8)&0xf;
}

void saturn_device::S64_WRITE_WORD(int r, int v)
{
	m_reg[r][0]=v&0xf;
	m_reg[r][1]=(v>>4)&0xf;
	m_reg[r][2]=(v>>8)&0xf;
	m_reg[r][3]=(v>>12)&0xf;
}

void saturn_device::S64_WRITE_A(int r, int v)
{
	m_reg[r][0]=v&0xf;
	m_reg[r][1]=(v>>4)&0xf;
	m_reg[r][2]=(v>>8)&0xf;
	m_reg[r][3]=(v>>12)&0xf;
	m_reg[r][4]=(v>>16)&0xf;
}





UINT32 saturn_device::saturn_pop()
{
	UINT32 temp=m_rstk[0];
	memmove(m_rstk, m_rstk+1, sizeof(m_rstk)-sizeof(m_rstk[0]));
	m_rstk[7]=0;
	return temp;
}

void saturn_device::saturn_push(UINT32 adr)
{
	memmove(m_rstk+1, m_rstk, sizeof(m_rstk)-sizeof(m_rstk[0]));
	m_rstk[0]=adr;
}

void saturn_device::saturn_interrupt_on()
{
	LOG(( "SATURN '%s' at %05x: INTON\n", tag().c_str(), m_pc-4 ));
	m_irq_enable=1;
	if (m_irq_state)
	{
		LOG(( "SATURN '%s' set_irq_line(ASSERT)\n", tag().c_str()));
		m_pending_irq=1;
	}
}

void saturn_device::saturn_interrupt_off()
{
	LOG(( "SATURN '%s' at %05x: INTOFF\n", tag().c_str(), m_pc-4 ));
	m_irq_enable=0;
}

void saturn_device::saturn_reset_interrupt()
{
	LOG(( "SATURN '%s' at %05x: RSI\n", tag().c_str(), m_pc-5 ));
	m_rsi_func(ASSERT_LINE);
}

void saturn_device::saturn_mem_reset()
{
	m_reset_func(ASSERT_LINE);
}

void saturn_device::saturn_mem_config()
{
	m_config_func(S64_READ_A(C));
}

void saturn_device::saturn_mem_unconfig()
{
	m_unconfig_func(S64_READ_A(C));
}

void saturn_device::saturn_mem_id()
{
	int id=0;
	id = m_id_func();
	S64_WRITE_A(C,id);
	m_monitor_id = id;
}

void saturn_device::saturn_shutdown()
{
	m_sleeping=1;
	m_irq_enable=1;
	LOG(( "SATURN '%s' at %05x: SHUTDN\n", tag().c_str(), m_pc-3 ));
}

void saturn_device::saturn_bus_command_b()
{
	logerror( "SATURN '%s' at %05x: BUSCB opcode not handled\n", tag().c_str(), m_pc-4 );
}

void saturn_device::saturn_bus_command_c()
{
	logerror( "SATURN '%s' at %05x: BUSCC opcode not handled\n", tag().c_str(), m_pc-3 );
}

void saturn_device::saturn_bus_command_d()
{
	logerror( "SATURN '%s' at %05x: BUSCD opcode not handled\n", tag().c_str(), m_pc-4 );
}

void saturn_device::saturn_serial_request()
{
	logerror( "SATURN '%s' at %05x: SREQ? opcode not handled\n", tag().c_str(), m_pc-3 );
}

void saturn_device::saturn_out_c()
{
	m_out=S64_READ_X(C);
	m_out_func(m_out);
}

void saturn_device::saturn_out_cs()
{
	m_out=(m_out&0xff0)|m_reg[C][0];
	m_out_func(m_out);
}

void saturn_device::saturn_in(int reg)
{
	int in = 0;
	saturn_assert(reg>=0 && reg<9);
	if (!(m_pc&1))
		logerror( "SATURN '%s' at %05x: reg=IN opcode at odd addresse\n",
				tag().c_str(), m_pc-3 );
	in = m_in_func();
	S64_WRITE_WORD(reg,in);
	m_monitor_in = in;
}


/* st related */
void saturn_device::saturn_clear_st()
{
	m_st&=0xf000;
}

void saturn_device::saturn_st_to_c()
{
	S64_WRITE_X(C,m_st);
}

void saturn_device::saturn_c_to_st()
{
	m_st=(m_st&0xf000)|(S64_READ_X(C));
}

void saturn_device::saturn_exchange_c_st()
{
	int t=m_st;
	m_st=(t&0xf000)|(S64_READ_X(C));
	S64_WRITE_X(C,t);
}

void saturn_device::saturn_jump_after_test()
{
	int adr=READ_OP_DIS8();
	if (m_carry) {
		if (adr==0) {
			m_pc=saturn_pop();
		} else {
			m_pc=(m_pc+adr-2)&0xfffff;
		}
	}
}
void saturn_device::saturn_st_clear_bit()
{
	m_st &= ~(1<<(READ_OP_ARG()));
}

void saturn_device::saturn_st_set_bit()
{
	m_st |= (1<<(READ_OP_ARG()));
}

void saturn_device::saturn_st_jump_bit_clear()
{
	m_carry=!((m_st>>(READ_OP_ARG()))&1);
	saturn_jump_after_test();
}

void saturn_device::saturn_st_jump_bit_set()
{
	m_carry=(m_st>>(READ_OP_ARG()))&1;
	saturn_jump_after_test();
}

void saturn_device::saturn_hst_clear_bits()
{
	m_hst&=~(READ_OP_ARG());
}

void saturn_device::saturn_hst_bits_cleared()
{
	m_carry=!(m_hst&(READ_OP_ARG()));
	saturn_jump_after_test();
}

/* p related */
void saturn_device::saturn_exchange_p()
{
	int nr=READ_OP_ARG();
	int t=m_p;
	m_p=m_reg[C][nr];
	m_reg[C][nr]=t;
}

void saturn_device::saturn_p_to_c()
{
	int nr=READ_OP_ARG();
	m_reg[C][nr]=m_p;
}

void saturn_device::saturn_c_to_p()
{
	int nr=READ_OP_ARG();
	m_p=m_reg[C][nr];
}

void saturn_device::saturn_dec_p()
{
	m_carry=m_p==0;
	m_p=(m_p-1)&0xf;
}

void saturn_device::saturn_inc_p()
{
	m_p=(m_p+1)&0xf;
	m_carry=m_p==0;
}

void saturn_device::saturn_load_p()
{
	m_p=READ_OP_ARG();
}

void saturn_device::saturn_p_equals()
{
	m_carry=m_p==(READ_OP_ARG());
	saturn_jump_after_test();
}

void saturn_device::saturn_p_not_equals()
{
	m_carry=m_p!=(READ_OP_ARG());
	saturn_jump_after_test();
}

void saturn_device::saturn_ca_p_1()
{
	int a=(S64_READ_A(C))+1+m_p;
	m_carry=a>=0x100000;
	S64_WRITE_A(C,a&0xfffff);
}

void saturn_device::saturn_load_reg(int reg)
{
	int count=READ_OP_ARG();
	int pos=m_p;
	saturn_assert(reg>=0 && reg<9);
	for (; count>=0; count--, pos=(pos+1)&0xf ) {
		m_reg[reg][pos]=READ_OP_ARG();
	}
}

void saturn_device::saturn_jump(int adr, int jump)
{
	saturn_assert(adr>=0 && adr<0x100000);
	if (jump) {
		m_pc=adr;
		m_icount-=10;
	}
}

void saturn_device::saturn_call(int adr)
{
	saturn_assert(adr>=0 && adr<0x100000);
	saturn_push(m_pc);
	m_pc=adr;
//  m_icount-=10;
}

void saturn_device::saturn_return(int yes)
{
	if (yes) {
		m_pc=saturn_pop();
//  m_icount-=10;
	}
}

void saturn_device::saturn_return_carry_set()
{
	m_pc=saturn_pop();
//  m_icount-=10;
	m_carry=1;
}

void saturn_device::saturn_return_carry_clear()
{
	m_pc=saturn_pop();
//  m_icount-=10;
	m_carry=0;
}

void saturn_device::saturn_return_interrupt()
{
	LOG(( "SATURN '%s' at %05x: RTI\n", tag().c_str(), m_pc-2 ));
	m_in_irq=0; /* set to 1 when an IRQ is taken */
	m_pc=saturn_pop();
//  m_icount-=10;
}

void saturn_device::saturn_return_xm_set()
{
	m_pc=saturn_pop();
	m_hst|=XM;
//  m_icount-=10;
}

void saturn_device::saturn_pop_c()
{
	S64_WRITE_A(C,saturn_pop());
}

void saturn_device::saturn_push_c()
{
	saturn_push(S64_READ_A(C));
}

void saturn_device::saturn_indirect_jump(int reg)
{
	saturn_assert(reg>=0 && reg<9);
	m_pc=READ_20(S64_READ_A(reg));
}

void saturn_device::saturn_equals_zero(int reg, int begin, int count)
{
	int i, t;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(begin>=0 && count>0 && begin+count<=16);
	m_carry=1;
	for (i=0; i<count; i++) {
		t=m_reg[reg][begin+i];
		if (t!=0) { m_carry=0; break; }
		m_icount-=2;
	}
	saturn_jump_after_test();
}

void saturn_device::saturn_equals(int reg, int begin, int count, int right)
{
	int i, t,t2;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(right>=0 && right<9);
	saturn_assert(begin>=0 && count>0 && begin+count<=16);
	m_carry=1;
	for (i=0; i<count; i++) {
		t=m_reg[reg][begin+i];
		t2=m_reg[right][begin+i];
		if (t!=t2) { m_carry=0; break; }
		m_icount-=2;
	}
	saturn_jump_after_test();
}

void saturn_device::saturn_not_equals_zero(int reg, int begin, int count)
{
	int i, t;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(begin>=0 && count>0 && begin+count<=16);
	m_carry=0;
	for (i=0; i<count; i++) {
		t=m_reg[reg][begin+i];
		if (t!=0) { m_carry=1; break; }
		m_icount-=2;
	}
	saturn_jump_after_test();
}

void saturn_device::saturn_not_equals(int reg, int begin, int count, int right)
{
	int i, t,t2;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(right>=0 && right<9);
	saturn_assert(begin>=0 && count>0 && begin+count<=16);
	m_carry=0;
	for (i=0; i<count; i++) {
		t=m_reg[reg][begin+i];
		t2=m_reg[right][begin+i];
		if (t!=t2) { m_carry=1; break; }
		m_icount-=2;
	}
	saturn_jump_after_test();
}

void saturn_device::saturn_greater(int reg, int begin, int count, int right)
{
	int i, t,t2;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(right>=0 && right<9);
	saturn_assert(begin>=0 && count>0 && begin+count<=16);
	m_carry=0;
	for (i=count-1; i>=0; i--) {
		t=m_reg[reg][begin+i];
		t2=m_reg[right][begin+i];
		if (t>t2) { m_carry=1; break; }
		if (t<t2) break;
		m_icount-=2;
	}
	saturn_jump_after_test();
}

void saturn_device::saturn_greater_equals(int reg, int begin, int count, int right)
{
	int i, t,t2;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(right>=0 && right<9);
	saturn_assert(begin>=0 && count>0 && begin+count<=16);
	m_carry=1;
	for (i=count-1; i>=0; i--) {
		t=m_reg[reg][begin+i];
		t2=m_reg[right][begin+i];
		if (t<t2) { m_carry=0; break; }
		if (t>t2) break;
		m_icount-=2;
	}
	saturn_jump_after_test();
}

void saturn_device::saturn_smaller_equals(int reg, int begin, int count, int right)
{
	int i, t,t2;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(right>=0 && right<9);
	saturn_assert(begin>=0 && count>0 && begin+count<=16);
	m_carry=1;
	for (i=count-1; i>=0; i--) {
		t=m_reg[reg][begin+i];
		t2=m_reg[right][begin+i];
		if (t>t2) { m_carry=0; break; }
		if (t<t2) break;
		m_icount-=2;
	}
	saturn_jump_after_test();
}

void saturn_device::saturn_smaller(int reg, int begin, int count, int right)
{
	int i, t,t2;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(right>=0 && right<9);
	saturn_assert(begin>=0 && count>0 && begin+count<=16);
	m_carry=0;
	for (i=count-1; i>=0; i--) {
		t=m_reg[reg][begin+i];
		t2=m_reg[right][begin+i];
		if (t<t2) { m_carry=1; break; }
		if (t>t2) break;
		m_icount-=2;
	}
	saturn_jump_after_test();
}

void saturn_device::saturn_jump_bit_clear(int reg)
{
	int op=READ_OP_ARG();
	saturn_assert(reg>=0 && reg<9);
	m_carry=!((m_reg[reg][op>>2]>>(op&3))&1);
	saturn_jump_after_test();
}

void saturn_device::saturn_jump_bit_set(int reg)
{
	int op=READ_OP_ARG();
	saturn_assert(reg>=0 && reg<9);
	m_carry=(m_reg[reg][op>>2]>>(op&3))&1;
	saturn_jump_after_test();
}

void saturn_device::saturn_load_pc(int reg)
{
	saturn_assert(reg>=0 && reg<9);
	m_pc=S64_READ_A(reg);
}

void saturn_device::saturn_store_pc(int reg)
{
	saturn_assert(reg>=0 && reg<9);
	S64_WRITE_A(reg,m_pc);
}

void saturn_device::saturn_exchange_pc(int reg)
{
	int temp=m_pc;
	saturn_assert(reg>=0 && reg<9);
	m_pc=S64_READ_A(reg);
	S64_WRITE_A(reg, temp);
}

/*************************************************************************************
 address register related
*************************************************************************************/
void saturn_device::saturn_load_adr(int reg, int nibbles)
{
	saturn_assert(reg>=0 && reg<2);
	saturn_assert(nibbles==2 || nibbles==4 || nibbles==5);
	switch (nibbles) {
	case 5:
		m_d[reg]=READ_OP_ARG20();
		break;
	case 4:
		m_d[reg]=(m_d[reg]&0xf0000)|READ_OP_ARG16();
		break;
	case 2:
		m_d[reg]=(m_d[reg]&0xfff00)|READ_OP_ARG8();
		break;
	}
}

void saturn_device::saturn_add_adr(int reg)
{
	int t=m_d[reg]+READ_OP_ARG()+1;
	saturn_assert(reg>=0 && reg<2);
	m_d[reg]=t&0xfffff;
	m_carry=t>=0x100000;
}

void saturn_device::saturn_sub_adr(int reg)
{
	int t=m_d[reg]-READ_OP_ARG()-1;
	saturn_assert(reg>=0 && reg<2);
	m_d[reg]=t&0xfffff;
	m_carry=t<0;
}

void saturn_device::saturn_adr_to_reg(int adr, int reg)
{
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(adr>=0 && adr<2);
	S64_WRITE_A(reg,m_d[adr]);
}

void saturn_device::saturn_reg_to_adr(int reg, int adr)
{
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(adr>=0 && adr<2);
	m_d[adr]=S64_READ_A(reg);
}

void saturn_device::saturn_adr_to_reg_word(int adr, int reg)
{
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(adr>=0 && adr<2);
	S64_WRITE_WORD(reg,m_d[adr]&0xffff);
}

void saturn_device::saturn_reg_to_adr_word(int reg, int adr)
{
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(adr>=0 && adr<2);
	m_d[adr]=(m_d[adr]&0xf0000)|S64_READ_WORD(reg);
}

void saturn_device::saturn_exchange_adr_reg(int adr, int reg)
{
	int temp=m_d[adr];
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(adr>=0 && adr<2);
	m_d[adr]=S64_READ_A(reg);
	S64_WRITE_A(reg,temp);
}

void saturn_device::saturn_exchange_adr_reg_word(int adr, int reg)
{
	int temp=m_d[adr]&0xffff;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(adr>=0 && adr<2);
	m_d[adr]=(m_d[adr]&0xf0000)|S64_READ_WORD(reg);
	S64_WRITE_WORD(reg,temp);
}

void saturn_device::saturn_load_nibbles(int reg, int begin, int count, int adr)
{
	int i;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(adr>=0 && adr<2);
	saturn_assert(begin>=0 && count>0 && begin+count<=16);
	for (i=0; i<count; i++) {
		m_reg[reg][begin+i]=READ_NIBBLE(m_d[adr]+i);
		m_icount-=2;
	}
}

void saturn_device::saturn_store_nibbles(int reg, int begin, int count, int adr)
{
	int i;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(adr>=0 && adr<2);
	saturn_assert(begin>=0 && count>0 && begin+count<=16);
	for (i=0; i<count; i++) {
		WRITE_NIBBLE((m_d[adr]+i)&0xfffff,m_reg[reg][begin+i]);
		m_icount-=2;
	}
}

void saturn_device::saturn_clear_bit(int reg)
{
	int arg=READ_OP_ARG();
	saturn_assert(reg>=0 && reg<9);
	m_reg[reg][arg>>2]&=~(1<<(arg&3));
}

void saturn_device::saturn_set_bit(int reg)
{
	int arg=READ_OP_ARG();
	saturn_assert(reg>=0 && reg<9);
	m_reg[reg][arg>>2]|=1<<(arg&3);
}

/****************************************************************************
 clear opers
 ****************************************************************************/
void saturn_device::saturn_clear(int reg, int begin, int count)
{
	int i;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(begin>=0 && count>0 && begin+count<=16);
	for (i=0; i<count; i++) {
		m_reg[reg][begin+i]=0;
		m_icount-=2;
	}
}

/****************************************************************************
 exchange opers
 ****************************************************************************/
void saturn_device::saturn_exchange(int left, int begin, int count, int right)
{
	int i;
	UINT8 temp;
	saturn_assert(left>=0 && left<9);
	saturn_assert(right>=0 && right<9);
	saturn_assert(begin>=0 && count>0 && begin+count<=16);
	for (i=0; i<count; i++) {
		temp=m_reg[left][begin+i];
		m_reg[left][begin+i]=m_reg[right][begin+i];
		m_reg[right][begin+i]=temp;
		m_icount-=2;
	}
}

/****************************************************************************
 copy opers
 ****************************************************************************/
void saturn_device::saturn_copy(int dest, int begin, int count, int src)
{
	int i;
	saturn_assert(dest>=0 && dest<9);
	saturn_assert(src>=0 && src<9);
	saturn_assert(begin>=0 && count>0 && begin+count<=16);
	for (i=0; i<count; i++) {
		m_reg[dest][begin+i]=m_reg[src][begin+i];
		m_icount-=2;
	}
}

/****************************************************************************
 add opers
 ****************************************************************************/
void saturn_device::saturn_add(int reg, int begin, int count, int right)
{
	int i, t;
	int base=m_decimal?10:16;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(right>=0 && right<9);
	saturn_assert(begin>=0 && count>0 && begin+count<=16);
	m_carry=0;
	for (i=0; i<count; i++) {
		t=m_reg[reg][begin+i];
		t+=m_reg[right][begin+i];
		t+=m_carry;
		if (t>=base) {
			m_carry=1;
			t-=base;
		}
		else m_carry=0;
		saturn_assert(t>=0); saturn_assert(t<base);
		m_reg[reg][begin+i]=t&0xf;
		m_icount-=2;
	}
}

void saturn_device::saturn_add_const(int reg, int begin, int count, UINT8 right)
{
	int i, t;
	int base=m_decimal?10:16;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(begin>=0 && count>0 && begin+count<=16);
	saturn_assert(count>1 || !m_decimal); /* SATURN bug */
	for (i=0; i<count; i++) {
		t=m_reg[reg][begin+i];
		t+=(right&0xf);
		right>>=4;
		if (t>=base) {
			right++;
			t-=base;
		}
		saturn_assert(t>=0); saturn_assert(t<base);
		m_reg[reg][begin+i]=t&0xf;
		m_icount-=2;
		if (!right) break;
	}
	m_carry=right>0;
}

/****************************************************************************
 sub opers
 ****************************************************************************/
void saturn_device::saturn_sub(int reg, int begin, int count, int right)
{
	int i, t;
	int base=m_decimal?10:16;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(right>=0 && right<9);
	saturn_assert(begin>=0 && count>0 && begin+count<=16);
	m_carry=0;
	for (i=0; i<count; i++) {
		t=m_reg[reg][begin+i];
		t-=m_reg[right][begin+i];
		t-=m_carry;
		if (t<0) {
			m_carry=1;
			t+=base;
		}
		else m_carry=0;
		saturn_assert(t>=0); saturn_assert(t<base);
		m_reg[reg][begin+i]=t&0xf;
		m_icount-=2;
	}
}

void saturn_device::saturn_sub_const(int reg, int begin, int count, int right)
{
	int i, t;
	int base=m_decimal?10:16;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(begin>=0 && count>0 && begin+count<=16);
	saturn_assert(count>1 || !m_decimal); /* SATURN bug */
	for (i=0; i<count; i++) {
		t=m_reg[reg][begin+i];
		t-=(right&0xf);
		right>>=4;
		if (t<0) {
			right++;
			t+=base;
		}
		saturn_assert(t>=0); saturn_assert(t<base);
		m_reg[reg][begin+i]=t&0xf;
		m_icount-=2;
		if (!right) break;
	}
	m_carry=right>0;
}

/****************************************************************************
 sub2 opers (a=b-a)
 ****************************************************************************/
void saturn_device::saturn_sub2(int reg, int begin, int count, int right)
{
	int i, t;
	int base=m_decimal?10:16;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(right>=0 && right<9);
	saturn_assert(begin>=0 && count>0 && begin+count<=16);
	m_carry=0;
	for (i=0; i<count; i++) {
		t=m_reg[right][begin+i];
		t-=m_reg[reg][begin+i];
		t-=m_carry;
		if (t<0) {
			m_carry=1;
			t+=base;
		}
		else m_carry=0;
		saturn_assert(t>=0); saturn_assert(t<base);
		m_reg[reg][begin+i]=t&0xf;
		m_icount-=2;
	}
}

/****************************************************************************
 increment opers
 ****************************************************************************/
void saturn_device::saturn_increment(int reg, int begin, int count)
{
	int i, t=0;
	int base=m_decimal?10:16;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(begin>=0 && count>0 && begin+count<=16);
	for (i=0; i<count; i++) {
		m_icount-=2;
		t=m_reg[reg][begin+i];
		t++;
		if (t>=base) m_reg[reg][begin+i]=t-base;
		else { m_reg[reg][begin+i]=t; break; }
	}
	m_carry=t>=base;
}

/****************************************************************************
 decrement opers
 ****************************************************************************/
void saturn_device::saturn_decrement(int reg, int begin, int count)
{
	int i, t=0;
	int base=m_decimal?10:16;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(begin>=0 && count>0 && begin+count<=16);
	for (i=0; i<count; i++) {
		m_icount-=2;
		t=m_reg[reg][begin+i];
		t--;
		if (t<0) m_reg[reg][begin+i]=t+base;
		else { m_reg[reg][begin+i]=t; break; }
	}
	m_carry=t<0;
}

/****************************************************************************
 invert (1 complement)  opers
 ****************************************************************************/
void saturn_device::saturn_invert(int reg, int begin, int count)
{
	int i;
	int max=m_decimal?9:15;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(begin>=0 && count>0 && begin+count<=16);
	m_carry=0;
	for (i=0; i<count; i++) {
		m_reg[reg][begin+i]=(max-m_reg[reg][begin+i])&0xf;
		m_icount-=2;
	}
}

/****************************************************************************
 negate (2 complement)  opers
 ****************************************************************************/
void saturn_device::saturn_negate(int reg, int begin, int count)
{
	int i, n, c;
	int max=m_decimal?9:15;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(begin>=0 && count>0 && begin+count<=16);
	c=1;
	m_carry=0;
	for (i=0; i<count; i++) {
		n=m_reg[reg][begin+i];
		if (n) m_carry=1;
		n=max+c-n;
		if (n>max) n-=max+1;
		else c=0;
		saturn_assert(n>=0); saturn_assert(n<=max);
		m_reg[reg][begin+i]=n&0xf;
		m_icount-=2;
	}
}

/****************************************************************************
 or opers
 ****************************************************************************/
void saturn_device::saturn_or(int dest, int begin, int count, int src)
{
	int i;
	saturn_assert(dest>=0 && dest<9);
	saturn_assert(src>=0 && src<9);
	saturn_assert(begin>=0 && count>0 && begin+count<=16);
	for (i=0; i<count; i++) {
		m_reg[dest][begin+i]|=m_reg[src][begin+i];
		m_icount-=2;
	}
}

/****************************************************************************
 and opers
 ****************************************************************************/
void saturn_device::saturn_and(int dest, int begin, int count, int src)
{
	int i;
	saturn_assert(dest>=0 && dest<9);
	saturn_assert(src>=0 && src<9);
	saturn_assert(begin>=0 && count>0 && begin+count<=16);
	for (i=0; i<count; i++) {
		m_reg[dest][begin+i]&=m_reg[src][begin+i];
		m_icount-=2;
	}
}

/****************************************************************************
 shift nibbles left opers
 ****************************************************************************/
void saturn_device::saturn_shift_nibble_left(int reg, int begin, int count)
{
	int i;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(begin>=0 && count>0 && begin+count<=16);
	if (m_reg[reg][begin+count-1]) m_hst|=SB;
	for (i=count-1; i>=1; i--) {
		m_reg[reg][begin+i]=m_reg[reg][begin+i-1];
		m_icount-=2;
	}
	m_reg[reg][begin]=0;
	m_icount-=2;
}

/****************************************************************************
 shift nibbles right opers
 ****************************************************************************/
void saturn_device::saturn_shift_nibble_right(int reg, int begin, int count)
{
	int i;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(begin>=0 && count>0 && begin+count<=16);
	if (m_reg[reg][begin]) m_hst|=SB;
	for (i=1; i<count; i++) {
		m_reg[reg][begin+i-1]=m_reg[reg][begin+i];
		m_icount-=2;
	}
	m_reg[reg][begin+count-1]=0;
	m_icount-=2;
}


/****************************************************************************
 rotate nibbles left opers
 ****************************************************************************/
void saturn_device::saturn_rotate_nibble_left_w(int reg)
{
	int i, x=m_reg[reg][15];
	saturn_assert(reg>=0 && reg<9);
	for (i=15; i>=1; i--) {
		m_reg[reg][i]=m_reg[reg][i-1];
		m_icount-=2;
	}
	m_reg[reg][0]=x;
	m_icount-=2;
}

/****************************************************************************
 rotate nibbles right opers
 ****************************************************************************/
void saturn_device::saturn_rotate_nibble_right_w(int reg)
{
	int i, x=m_reg[reg][0];
	saturn_assert(reg>=0 && reg<9);
	for (i=1; i<16; i++) {
		m_reg[reg][i-1]=m_reg[reg][i];
		m_icount-=2;
	}
	m_reg[reg][15]=x;
	if (x) m_hst|=SB;
	m_icount-=2;
}


/****************************************************************************
 shift right opers
 ****************************************************************************/
void saturn_device::saturn_shift_right(int reg, int begin, int count)
{
	int i, t, c=0;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(begin>=0 && count>0 && begin+count<=16);
	for (i=count-1; i>=0; i--) {
		t=m_reg[reg][begin+i];
		t|=(c<<4);
		c=t&1;
		m_reg[reg][begin+i]=t>>1;
		m_icount-=2;
	}
	if (c) m_hst|=SB;
	m_icount-=2;
}
