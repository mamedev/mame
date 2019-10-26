// license:BSD-3-Clause
// copyright-holders:Peter Trauner,Antoine Mine
#ifndef MAME_CPU_SATURN_SATOPS_IPP
#define MAME_CPU_SATURN_SATOPS_IPP

#pragma once

#include "saturn.h"


#define saturn_assert(x) \
		do { if (!(x)) logerror("SATURN assertion failed: %s at %s:%i, pc=%05x\n", #x, __FILE__, __LINE__, m_pc); } while (false)

inline int saturn_device::READ_OP()
{
	m_icount-=3;
	const uint8_t data=m_cache->read_byte(m_pc);
	saturn_assert(data<0x10);
	m_pc=(m_pc+1)&0xfffff;
	return data;
}

inline int saturn_device::READ_OP_ARG()
{
	m_icount-=3;
	const uint8_t data=m_cache->read_byte(m_pc);
	saturn_assert(data<0x10);
	m_pc=(m_pc+1)&0xfffff;
	return data;
}

inline int saturn_device::READ_OP_ARG8()
{
	const int n0=READ_OP_ARG();
	const int n1=READ_OP_ARG();
	return n0|(n1<<4);
}

inline int8_t saturn_device::READ_OP_DIS8()
{
	return (int8_t)READ_OP_ARG8();
}

inline int saturn_device::READ_OP_ARG12()
{
	const int n0=READ_OP_ARG();
	const int n1=READ_OP_ARG();
	const int n2=READ_OP_ARG();
	return n0|(n1<<4)|(n2<<8);
}

inline int saturn_device::READ_OP_DIS12()
{
	int temp=READ_OP_ARG12();
	if (temp&0x800) temp-=0x1000;
	return temp;
}

inline int saturn_device::READ_OP_ARG16()
{
	const int n0=READ_OP_ARG();
	const int n1=READ_OP_ARG();
	const int n2=READ_OP_ARG();
	const int n3=READ_OP_ARG();
	return n0|(n1<<4)|(n2<<8)|(n3<<12);
}

inline int16_t saturn_device::READ_OP_DIS16()
{
	return (int16_t)READ_OP_ARG16();
}

inline int saturn_device::READ_OP_ARG20()
{
	const int n0=READ_OP_ARG();
	const int n1=READ_OP_ARG();
	const int n2=READ_OP_ARG();
	const int n3=READ_OP_ARG();
	const int n4=READ_OP_ARG();
	return n0|(n1<<4)|(n2<<8)|(n3<<12)|(n4<<16);
}

inline int saturn_device::READ_NIBBLE(uint32_t adr)
{
	m_icount-=3;
	const uint8_t data=m_program->read_byte(adr&0xfffff);
	saturn_assert(data<0x10);
	m_crc_func(adr&0xfffff, data, 0xffffffff);
	return data;
}

inline int saturn_device::READ_8(uint32_t adr)
{
	const int n0=READ_NIBBLE(adr);
	const int n1=READ_NIBBLE(adr+1);
	return n0|(n1<<4);
}

inline int saturn_device::READ_12(uint32_t adr)
{
	const int n0=READ_NIBBLE(adr);
	const int n1=READ_NIBBLE(adr+1);
	const int n2=READ_NIBBLE(adr+2);
	return n0|(n1<<4)|(n2<<8);
}

inline int saturn_device::READ_16(uint32_t adr)
{
	const int n0=READ_NIBBLE(adr);
	const int n1=READ_NIBBLE(adr+1);
	const int n2=READ_NIBBLE(adr+2);
	const int n3=READ_NIBBLE(adr+3);
	return n0|(n1<<4)|(n2<<8)|(n3<<12);
}

inline int saturn_device::READ_20(uint32_t adr)
{
	const int n0=READ_NIBBLE(adr);
	const int n1=READ_NIBBLE(adr+1);
	const int n2=READ_NIBBLE(adr+2);
	const int n3=READ_NIBBLE(adr+3);
	const int n4=READ_NIBBLE(adr+4);
	return n0|(n1<<4)|(n2<<8)|(n3<<12)|(n4<<16);
}

inline void saturn_device::WRITE_NIBBLE(uint32_t adr, uint8_t nib)
{
	m_icount-=3;
	saturn_assert(nib<0x10);
	m_program->write_byte(adr&0xfffff,nib);
}

inline int saturn_device::S64_READ_X(int r)
{
	return m_reg[r][0]|(m_reg[r][1]<<4)|(m_reg[r][2]<<8);
}

inline int saturn_device::S64_READ_WORD(int r)
{
	return m_reg[r][0]|(m_reg[r][1]<<4)|(m_reg[r][2]<<8)|(m_reg[r][3]<<12);
}

inline int saturn_device::S64_READ_A(int r)
{
	return m_reg[r][0]|(m_reg[r][1]<<4)|(m_reg[r][2]<<8)|(m_reg[r][3]<<12)|(m_reg[r][4]<<16);
}

inline void saturn_device::S64_WRITE_X(int r, int v)
{
	m_reg[r][0]=v&0xf;
	m_reg[r][1]=(v>>4)&0xf;
	m_reg[r][2]=(v>>8)&0xf;
}

inline void saturn_device::S64_WRITE_WORD(int r, int v)
{
	m_reg[r][0]=v&0xf;
	m_reg[r][1]=(v>>4)&0xf;
	m_reg[r][2]=(v>>8)&0xf;
	m_reg[r][3]=(v>>12)&0xf;
}

inline void saturn_device::S64_WRITE_A(int r, int v)
{
	m_reg[r][0]=v&0xf;
	m_reg[r][1]=(v>>4)&0xf;
	m_reg[r][2]=(v>>8)&0xf;
	m_reg[r][3]=(v>>12)&0xf;
	m_reg[r][4]=(v>>16)&0xf;
}



inline uint32_t saturn_device::saturn_pop()
{
	const uint32_t temp=m_rstk[0];
	memmove(m_rstk, m_rstk+1, sizeof(m_rstk)-sizeof(m_rstk[0]));
	m_rstk[7]=0;
	return temp;
}

inline void saturn_device::saturn_push(uint32_t adr)
{
	memmove(m_rstk+1, m_rstk, sizeof(m_rstk)-sizeof(m_rstk[0]));
	m_rstk[0]=adr;
}

inline void saturn_device::saturn_interrupt_on()
{
	LOG("SATURN at %05x: INTON\n", m_pc-4);
	m_irq_enable=1;
	if (m_irq_state) {
		LOG("SATURN set_irq_line(ASSERT)\n");
		m_pending_irq=1;
	}
}

inline void saturn_device::saturn_interrupt_off()
{
	LOG("SATURN at %05x: INTOFF\n", m_pc-4);
	m_irq_enable=0;
}

inline void saturn_device::saturn_reset_interrupt()
{
	LOG("SATURN at %05x: RSI\n", m_pc-5);
	m_rsi_func(ASSERT_LINE);
}

inline void saturn_device::saturn_mem_reset()
{
	m_reset_func(ASSERT_LINE);
}

inline void saturn_device::saturn_mem_config()
{
	m_config_func(S64_READ_A(C));
}

inline void saturn_device::saturn_mem_unconfig()
{
	m_unconfig_func(S64_READ_A(C));
}

inline void saturn_device::saturn_mem_id()
{
	const int id = m_id_func();
	S64_WRITE_A(C,id);
	m_monitor_id = id;
}

inline void saturn_device::saturn_shutdown()
{
	m_sleeping=1;
	m_irq_enable=1;
	LOG("SATURN at %05x: SHUTDN\n", m_pc-3);
}

inline void saturn_device::saturn_bus_command_b()
{
	logerror("SATURN at %05x: BUSCB opcode not handled\n", m_pc-4);
}

inline void saturn_device::saturn_bus_command_c()
{
	logerror("SATURN at %05x: BUSCC opcode not handled\n", m_pc-3);
}

inline void saturn_device::saturn_bus_command_d()
{
	logerror("SATURN at %05x: BUSCD opcode not handled\n", m_pc-4);
}

inline void saturn_device::saturn_serial_request()
{
	logerror("SATURN at %05x: SREQ? opcode not handled\n", m_pc-3);
}

inline void saturn_device::saturn_out_c()
{
	m_out=S64_READ_X(C);
	m_out_func(m_out);
}

inline void saturn_device::saturn_out_cs()
{
	m_out=(m_out&0xff0)|m_reg[C][0];
	m_out_func(m_out);
}

inline void saturn_device::saturn_in(int reg)
{
	saturn_assert(reg>=0 && reg<9);
	if (!(m_pc&1))
		logerror("SATURN at %05x: reg=IN opcode at odd addresse\n", m_pc-3);
	const int in = m_in_func();
	S64_WRITE_WORD(reg,in);
	m_monitor_in = in;
}


// st related
inline void saturn_device::saturn_clear_st()
{
	m_st&=0xf000;
}

inline void saturn_device::saturn_st_to_c()
{
	S64_WRITE_X(C,m_st);
}

inline void saturn_device::saturn_c_to_st()
{
	m_st=(m_st&0xf000)|(S64_READ_X(C));
}

inline void saturn_device::saturn_exchange_c_st()
{
	const int t=m_st;
	m_st=(t&0xf000)|(S64_READ_X(C));
	S64_WRITE_X(C,t);
}

inline void saturn_device::saturn_jump_after_test()
{
	const int adr=READ_OP_DIS8();
	if (m_carry) {
		if (adr==0) {
			m_pc=saturn_pop();
		} else {
			m_pc=(m_pc+adr-2)&0xfffff;
		}
	}
}

inline void saturn_device::saturn_st_clear_bit()
{
	m_st &= ~(1<<(READ_OP_ARG()));
}

inline void saturn_device::saturn_st_set_bit()
{
	m_st |= (1<<(READ_OP_ARG()));
}

inline void saturn_device::saturn_st_jump_bit_clear()
{
	m_carry=!((m_st>>(READ_OP_ARG()))&1);
	saturn_jump_after_test();
}

inline void saturn_device::saturn_st_jump_bit_set()
{
	m_carry=(m_st>>(READ_OP_ARG()))&1;
	saturn_jump_after_test();
}

inline void saturn_device::saturn_hst_clear_bits()
{
	m_hst&=~(READ_OP_ARG());
}

inline void saturn_device::saturn_hst_bits_cleared()
{
	m_carry=!(m_hst&(READ_OP_ARG()));
	saturn_jump_after_test();
}

// p related
inline void saturn_device::saturn_exchange_p()
{
	const int nr=READ_OP_ARG();
	const int t=m_p;
	m_p=m_reg[C][nr];
	m_reg[C][nr]=t;
}

inline void saturn_device::saturn_p_to_c()
{
	const int nr=READ_OP_ARG();
	m_reg[C][nr]=m_p;
}

inline void saturn_device::saturn_c_to_p()
{
	const int nr=READ_OP_ARG();
	m_p=m_reg[C][nr];
}

inline void saturn_device::saturn_dec_p()
{
	m_carry=m_p==0;
	m_p=(m_p-1)&0xf;
}

inline void saturn_device::saturn_inc_p()
{
	m_p=(m_p+1)&0xf;
	m_carry=m_p==0;
}

inline void saturn_device::saturn_load_p()
{
	m_p=READ_OP_ARG();
}

inline void saturn_device::saturn_p_equals()
{
	m_carry=m_p==(READ_OP_ARG());
	saturn_jump_after_test();
}

inline void saturn_device::saturn_p_not_equals()
{
	m_carry=m_p!=(READ_OP_ARG());
	saturn_jump_after_test();
}

inline void saturn_device::saturn_ca_p_1()
{
	const int a=(S64_READ_A(C))+1+m_p;
	m_carry=a>=0x100000;
	S64_WRITE_A(C,a&0xfffff);
}

inline void saturn_device::saturn_load_reg(int reg)
{
	int count=READ_OP_ARG();
	int pos=m_p;
	saturn_assert(reg>=0 && reg<9);
	for (; count>=0; count--, pos=(pos+1)&0xf) {
		m_reg[reg][pos]=READ_OP_ARG();
	}
}

inline void saturn_device::saturn_jump(int adr, int jump)
{
	saturn_assert(adr>=0 && adr<0x100000);
	if (jump) {
		m_pc=adr;
		m_icount-=10;
	}
}

inline void saturn_device::saturn_call(int adr)
{
	saturn_assert(adr>=0 && adr<0x100000);
	saturn_push(m_pc);
	m_pc=adr;
	//m_icount-=10;
}

inline void saturn_device::saturn_return(int yes)
{
	if (yes) {
		m_pc=saturn_pop();
		//m_icount-=10;
	}
}

inline void saturn_device::saturn_return_carry_set()
{
	m_pc=saturn_pop();
	//m_icount-=10;
	m_carry=1;
}

inline void saturn_device::saturn_return_carry_clear()
{
	m_pc=saturn_pop();
	//m_icount-=10;
	m_carry=0;
}

inline void saturn_device::saturn_return_interrupt()
{
	LOG("SATURN at %05x: RTI\n", m_pc-2);
	m_in_irq=0; // set to 1 when an IRQ is taken
	m_pc=saturn_pop();
	//m_icount-=10;
}

inline void saturn_device::saturn_return_xm_set()
{
	m_pc=saturn_pop();
	m_hst|=XM;
	//m_icount-=10;
}

inline void saturn_device::saturn_pop_c()
{
	S64_WRITE_A(C,saturn_pop());
}

inline void saturn_device::saturn_push_c()
{
	saturn_push(S64_READ_A(C));
}

inline void saturn_device::saturn_indirect_jump(int reg)
{
	saturn_assert(reg>=0 && reg<9);
	m_pc=READ_20(S64_READ_A(reg));
}

inline void saturn_device::saturn_equals_zero(int reg, int begin, int count)
{
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(begin>=0 && begin<16 && count>0 && begin+count<=16);
	m_carry=1;
	for (int i=0; i<count; i++) {
		const int t=m_reg[reg][begin+i];
		if (t!=0) { m_carry=0; break; }
		m_icount-=2;
	}
	saturn_jump_after_test();
}

inline void saturn_device::saturn_equals(int reg, int begin, int count, int right)
{
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(right>=0 && right<9);
	saturn_assert(begin>=0 && count>0 && begin+count<=16);
	m_carry=1;
	for (int i=0; i<count; i++) {
		const int t=m_reg[reg][begin+i];
		const int t2=m_reg[right][begin+i];
		if (t!=t2) { m_carry=0; break; }
		m_icount-=2;
	}
	saturn_jump_after_test();
}

inline void saturn_device::saturn_not_equals_zero(int reg, int begin, int count)
{
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(begin>=0 && count>0 && begin+count<=16);
	m_carry=0;
	for (int i=0; i<count; i++) {
		const int t=m_reg[reg][begin+i];
		if (t!=0) { m_carry=1; break; }
		m_icount-=2;
	}
	saturn_jump_after_test();
}

inline void saturn_device::saturn_not_equals(int reg, int begin, int count, int right)
{
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(right>=0 && right<9);
	saturn_assert(begin>=0 && count>0 && begin+count<=16);
	m_carry=0;
	for (int i=0; i<count; i++) {
		const int t=m_reg[reg][begin+i];
		const int t2=m_reg[right][begin+i];
		if (t!=t2) { m_carry=1; break; }
		m_icount-=2;
	}
	saturn_jump_after_test();
}

inline void saturn_device::saturn_greater(int reg, int begin, int count, int right)
{
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(right>=0 && right<9);
	saturn_assert(begin>=0 && count>0 && begin+count<=16);
	m_carry=0;
	for (int i=count-1; i>=0; i--) {
		const int t=m_reg[reg][begin+i];
		const int t2=m_reg[right][begin+i];
		if (t>t2) { m_carry=1; break; }
		if (t<t2) break;
		m_icount-=2;
	}
	saturn_jump_after_test();
}

inline void saturn_device::saturn_greater_equals(int reg, int begin, int count, int right)
{
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(right>=0 && right<9);
	saturn_assert(begin>=0 && count>0 && begin+count<=16);
	m_carry=1;
	for (int i=count-1; i>=0; i--) {
		const int t=m_reg[reg][begin+i];
		const int t2=m_reg[right][begin+i];
		if (t<t2) { m_carry=0; break; }
		if (t>t2) break;
		m_icount-=2;
	}
	saturn_jump_after_test();
}

inline void saturn_device::saturn_smaller_equals(int reg, int begin, int count, int right)
{
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(right>=0 && right<9);
	saturn_assert(begin>=0 && count>0 && begin+count<=16);
	m_carry=1;
	for (int i=count-1; i>=0; i--) {
		const int t=m_reg[reg][begin+i];
		const int t2=m_reg[right][begin+i];
		if (t>t2) { m_carry=0; break; }
		if (t<t2) break;
		m_icount-=2;
	}
	saturn_jump_after_test();
}

inline void saturn_device::saturn_smaller(int reg, int begin, int count, int right)
{
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(right>=0 && right<9);
	saturn_assert(begin>=0 && count>0 && begin+count<=16);
	m_carry=0;
	for (int i=count-1; i>=0; i--) {
		const int t=m_reg[reg][begin+i];
		const int t2=m_reg[right][begin+i];
		if (t<t2) { m_carry=1; break; }
		if (t>t2) break;
		m_icount-=2;
	}
	saturn_jump_after_test();
}

inline void saturn_device::saturn_jump_bit_clear(int reg)
{
	const int op=READ_OP_ARG();
	saturn_assert(reg>=0 && reg<9);
	m_carry=!((m_reg[reg][op>>2]>>(op&3))&1);
	saturn_jump_after_test();
}

inline void saturn_device::saturn_jump_bit_set(int reg)
{
	const int op=READ_OP_ARG();
	saturn_assert(reg>=0 && reg<9);
	m_carry=(m_reg[reg][op>>2]>>(op&3))&1;
	saturn_jump_after_test();
}

inline void saturn_device::saturn_load_pc(int reg)
{
	saturn_assert(reg>=0 && reg<9);
	m_pc=S64_READ_A(reg);
}

inline void saturn_device::saturn_store_pc(int reg)
{
	saturn_assert(reg>=0 && reg<9);
	S64_WRITE_A(reg,m_pc);
}

inline void saturn_device::saturn_exchange_pc(int reg)
{
	const int temp=m_pc;
	saturn_assert(reg>=0 && reg<9);
	m_pc=S64_READ_A(reg);
	S64_WRITE_A(reg, temp);
}

/*************************************************************************************
 address register related
*************************************************************************************/
inline void saturn_device::saturn_load_adr(int reg, int nibbles)
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

inline void saturn_device::saturn_add_adr(int reg)
{
	const int t=m_d[reg]+READ_OP_ARG()+1;
	saturn_assert(reg>=0 && reg<2);
	m_d[reg]=t&0xfffff;
	m_carry=t>=0x100000;
}

inline void saturn_device::saturn_sub_adr(int reg)
{
	const int t=m_d[reg]-READ_OP_ARG()-1;
	saturn_assert(reg>=0 && reg<2);
	m_d[reg]=t&0xfffff;
	m_carry=t<0;
}

inline void saturn_device::saturn_adr_to_reg(int adr, int reg)
{
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(adr>=0 && adr<2);
	S64_WRITE_A(reg,m_d[adr]);
}

inline void saturn_device::saturn_reg_to_adr(int reg, int adr)
{
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(adr>=0 && adr<2);
	m_d[adr]=S64_READ_A(reg);
}

inline void saturn_device::saturn_adr_to_reg_word(int adr, int reg)
{
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(adr>=0 && adr<2);
	S64_WRITE_WORD(reg,m_d[adr]&0xffff);
}

inline void saturn_device::saturn_reg_to_adr_word(int reg, int adr)
{
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(adr>=0 && adr<2);
	m_d[adr]=(m_d[adr]&0xf0000)|S64_READ_WORD(reg);
}

inline void saturn_device::saturn_exchange_adr_reg(int adr, int reg)
{
	const int temp=m_d[adr];
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(adr>=0 && adr<2);
	m_d[adr]=S64_READ_A(reg);
	S64_WRITE_A(reg,temp);
}

inline void saturn_device::saturn_exchange_adr_reg_word(int adr, int reg)
{
	const int temp=m_d[adr]&0xffff;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(adr>=0 && adr<2);
	m_d[adr]=(m_d[adr]&0xf0000)|S64_READ_WORD(reg);
	S64_WRITE_WORD(reg,temp);
}

inline void saturn_device::saturn_load_nibbles(int reg, int begin, int count, int adr)
{
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(adr>=0 && adr<2);
	saturn_assert(begin>=0 && count>0 && begin+count<=16);
	for (int i=0; i<count; i++) {
		m_reg[reg][begin+i]=READ_NIBBLE(m_d[adr]+i);
		m_icount-=2;
	}
}

inline void saturn_device::saturn_store_nibbles(int reg, int begin, int count, int adr)
{
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(adr>=0 && adr<2);
	saturn_assert(begin>=0 && count>0 && begin+count<=16);
	for (int i=0; i<count; i++) {
		WRITE_NIBBLE((m_d[adr]+i)&0xfffff,m_reg[reg][begin+i]);
		m_icount-=2;
	}
}

inline void saturn_device::saturn_clear_bit(int reg)
{
	const int arg=READ_OP_ARG();
	saturn_assert(reg>=0 && reg<9);
	m_reg[reg][arg>>2]&=~(1<<(arg&3));
}

inline void saturn_device::saturn_set_bit(int reg)
{
	const int arg=READ_OP_ARG();
	saturn_assert(reg>=0 && reg<9);
	m_reg[reg][arg>>2]|=1<<(arg&3);
}

/****************************************************************************
 clear opers
 ****************************************************************************/
inline void saturn_device::saturn_clear(int reg, int begin, int count)
{
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(begin>=0 && count>0 && begin+count<=16);
	for (int i=0; i<count; i++) {
		m_reg[reg][begin+i]=0;
		m_icount-=2;
	}
}

/****************************************************************************
 exchange opers
 ****************************************************************************/
inline void saturn_device::saturn_exchange(int left, int begin, int count, int right)
{
	saturn_assert(left>=0 && left<9);
	saturn_assert(right>=0 && right<9);
	saturn_assert(begin>=0 && count>0 && begin+count<=16);
	for (int i=0; i<count; i++) {
		const uint8_t temp=m_reg[left][begin+i];
		m_reg[left][begin+i]=m_reg[right][begin+i];
		m_reg[right][begin+i]=temp;
		m_icount-=2;
	}
}

/****************************************************************************
 copy opers
 ****************************************************************************/
inline void saturn_device::saturn_copy(int dest, int begin, int count, int src)
{
	saturn_assert(dest>=0 && dest<9);
	saturn_assert(src>=0 && src<9);
	saturn_assert(begin>=0 && count>0 && begin+count<=16);
	for (int i=0; i<count; i++) {
		m_reg[dest][begin+i]=m_reg[src][begin+i];
		m_icount-=2;
	}
}

/****************************************************************************
 add opers
 ****************************************************************************/
inline void saturn_device::saturn_add(int reg, int begin, int count, int right)
{
	const int base=m_decimal?10:16;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(right>=0 && right<9);
	saturn_assert(begin>=0 && count>0 && begin+count<=16);
	m_carry=0;
	for (int i=0; i<count; i++) {
		int t=m_reg[reg][begin+i];
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

inline void saturn_device::saturn_add_const(int reg, int begin, int count, uint8_t right)
{
	const int base=m_decimal?10:16;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(begin>=0 && count>0 && begin+count<=16);
	saturn_assert(count>1 || !m_decimal); /* SATURN bug */
	for (int i=0; i<count; i++) {
		int t=m_reg[reg][begin+i];
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
inline void saturn_device::saturn_sub(int reg, int begin, int count, int right)
{
	const int base=m_decimal?10:16;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(right>=0 && right<9);
	saturn_assert(begin>=0 && count>0 && begin+count<=16);
	m_carry=0;
	for (int i=0; i<count; i++) {
		int t=m_reg[reg][begin+i];
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

inline void saturn_device::saturn_sub_const(int reg, int begin, int count, int right)
{
	const int base=m_decimal?10:16;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(begin>=0 && count>0 && begin+count<=16);
	saturn_assert(count>1 || !m_decimal); // SATURN bug
	for (int i=0; i<count; i++) {
		int t=m_reg[reg][begin+i];
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
inline void saturn_device::saturn_sub2(int reg, int begin, int count, int right)
{
	const int base=m_decimal?10:16;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(right>=0 && right<9);
	saturn_assert(begin>=0 && count>0 && begin+count<=16);
	m_carry=0;
	for (int i=0; i<count; i++) {
		int t=m_reg[right][begin+i];
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
inline void saturn_device::saturn_increment(int reg, int begin, int count)
{
	int t=0;
	const int base=m_decimal?10:16;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(begin>=0 && count>0 && begin+count<=16);
	for (int i=0; i<count; i++) {
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
inline void saturn_device::saturn_decrement(int reg, int begin, int count)
{
	int t=0;
	const int base=m_decimal?10:16;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(begin>=0 && count>0 && begin+count<=16);
	for (int i=0; i<count; i++) {
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
inline void saturn_device::saturn_invert(int reg, int begin, int count)
{
	const int max=m_decimal?9:15;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(begin>=0 && count>0 && begin+count<=16);
	m_carry=0;
	for (int i=0; i<count; i++) {
		m_reg[reg][begin+i]=(max-m_reg[reg][begin+i])&0xf;
		m_icount-=2;
	}
}

/****************************************************************************
 negate (2 complement)  opers
 ****************************************************************************/
inline void saturn_device::saturn_negate(int reg, int begin, int count)
{
	const int max=m_decimal?9:15;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(begin>=0 && count>0 && begin+count<=16);
	int c=1;
	m_carry=0;
	for (int i=0; i<count; i++) {
		int n=m_reg[reg][begin+i];
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
inline void saturn_device::saturn_or(int dest, int begin, int count, int src)
{
	saturn_assert(dest>=0 && dest<9);
	saturn_assert(src>=0 && src<9);
	saturn_assert(begin>=0 && count>0 && begin+count<=16);
	for (int i=0; i<count; i++) {
		m_reg[dest][begin+i]|=m_reg[src][begin+i];
		m_icount-=2;
	}
}

/****************************************************************************
 and opers
 ****************************************************************************/
inline void saturn_device::saturn_and(int dest, int begin, int count, int src)
{
	saturn_assert(dest>=0 && dest<9);
	saturn_assert(src>=0 && src<9);
	saturn_assert(begin>=0 && count>0 && begin+count<=16);
	for (int i=0; i<count; i++) {
		m_reg[dest][begin+i]&=m_reg[src][begin+i];
		m_icount-=2;
	}
}

/****************************************************************************
 shift nibbles left opers
 ****************************************************************************/
inline void saturn_device::saturn_shift_nibble_left(int reg, int begin, int count)
{
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(begin>=0 && count>0 && begin+count<=16);
	if (m_reg[reg][begin+count-1]) m_hst|=SB;
	for (int i=count-1; i>=1; i--) {
		m_reg[reg][begin+i]=m_reg[reg][begin+i-1];
		m_icount-=2;
	}
	m_reg[reg][begin]=0;
	m_icount-=2;
}

/****************************************************************************
 shift nibbles right opers
 ****************************************************************************/
inline void saturn_device::saturn_shift_nibble_right(int reg, int begin, int count)
{
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(begin>=0 && count>0 && begin+count<=16);
	if (m_reg[reg][begin]) m_hst|=SB;
	for (int i=1; i<count; i++) {
		m_reg[reg][begin+i-1]=m_reg[reg][begin+i];
		m_icount-=2;
	}
	m_reg[reg][begin+count-1]=0;
	m_icount-=2;
}


/****************************************************************************
 rotate nibbles left opers
 ****************************************************************************/
inline void saturn_device::saturn_rotate_nibble_left_w(int reg)
{
	const int x=m_reg[reg][15];
	saturn_assert(reg>=0 && reg<9);
	for (int i=15; i>=1; i--) {
		m_reg[reg][i]=m_reg[reg][i-1];
		m_icount-=2;
	}
	m_reg[reg][0]=x;
	m_icount-=2;
}

/****************************************************************************
 rotate nibbles right opers
 ****************************************************************************/
inline void saturn_device::saturn_rotate_nibble_right_w(int reg)
{
	const int x=m_reg[reg][0];
	saturn_assert(reg>=0 && reg<9);
	for (int i=1; i<16; i++) {
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
inline void saturn_device::saturn_shift_right(int reg, int begin, int count)
{
	int c=0;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(begin>=0 && count>0 && begin+count<=16);
	for (int i=count-1; i>=0; i--) {
		int t=m_reg[reg][begin+i];
		t|=(c<<4);
		c=t&1;
		m_reg[reg][begin+i]=t>>1;
		m_icount-=2;
	}
	if (c) m_hst|=SB;
	m_icount-=2;
}

#endif // MAME_CPU_SATURN_SATOPS_IPP
