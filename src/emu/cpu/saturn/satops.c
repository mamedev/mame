#define IRQ_ADDRESS 0xf

#define saturn_assert(x) \
	do { if (!(x)) logerror("SATURN '%s' assertion failed: %s at %s:%i, pc=%05x\n", cpustate->device->tag(), #x, __FILE__, __LINE__, cpustate->pc); } while (0)

INLINE int READ_OP(saturn_state *cpustate)
{
	UINT8 data;
	cpustate->icount-=3;
		data=cpustate->direct->read_decrypted_byte(cpustate->pc);
	saturn_assert(data<0x10);
	cpustate->pc=(cpustate->pc+1)&0xfffff;
	return data;
}

INLINE int READ_OP_ARG(saturn_state *cpustate)
{
	UINT8 data;
	cpustate->icount-=3;
		data=cpustate->direct->read_raw_byte(cpustate->pc);
	saturn_assert(data<0x10);
	cpustate->pc=(cpustate->pc+1)&0xfffff;
	return data;
}

INLINE int READ_OP_ARG8(saturn_state *cpustate)
{
	int n0=READ_OP_ARG(cpustate);
	int n1=READ_OP_ARG(cpustate);
	return n0|(n1<<4);
}

INLINE INT8 READ_OP_DIS8(saturn_state *cpustate)
{
	return (INT8)READ_OP_ARG8(cpustate);
}

INLINE int READ_OP_ARG12(saturn_state *cpustate)
{
	int n0=READ_OP_ARG(cpustate);
	int n1=READ_OP_ARG(cpustate);
	int n2=READ_OP_ARG(cpustate);
	return n0|(n1<<4)|(n2<<8);
}

INLINE int READ_OP_DIS12(saturn_state *cpustate)
{
	int temp=READ_OP_ARG12(cpustate);
	if (temp&0x800) temp-=0x1000;
	return temp;
}

INLINE int READ_OP_ARG16(saturn_state *cpustate)
{
	int n0=READ_OP_ARG(cpustate);
	int n1=READ_OP_ARG(cpustate);
	int n2=READ_OP_ARG(cpustate);
	int n3=READ_OP_ARG(cpustate);
	return n0|(n1<<4)|(n2<<8)|(n3<<12);
}

INLINE INT16 READ_OP_DIS16(saturn_state *cpustate)
{
	return (INT16)READ_OP_ARG16(cpustate);
}

INLINE int READ_OP_ARG20(saturn_state *cpustate)
{
	int n0=READ_OP_ARG(cpustate);
	int n1=READ_OP_ARG(cpustate);
	int n2=READ_OP_ARG(cpustate);
	int n3=READ_OP_ARG(cpustate);
	int n4=READ_OP_ARG(cpustate);
	return n0|(n1<<4)|(n2<<8)|(n3<<12)|(n4<<16);
}

INLINE int READ_NIBBLE(saturn_state *cpustate, SaturnAdr adr)
{
	UINT8 data;
	cpustate->icount-=3;
	data=cpustate->program->read_byte(adr&0xfffff);
	saturn_assert(data<0x10);
	if (cpustate->config&&cpustate->config->crc) cpustate->config->crc(cpustate->device, adr&0xfffff, data);
	return data;
}

INLINE int READ_8(saturn_state *cpustate, SaturnAdr adr)
{
	return READ_NIBBLE(cpustate, adr)|(READ_NIBBLE(cpustate, adr+1)<<4);
}

INLINE int READ_12(saturn_state *cpustate, SaturnAdr adr)
{
	return READ_NIBBLE(cpustate, adr)|(READ_NIBBLE(cpustate, adr+1)<<4)|(READ_NIBBLE(cpustate, adr+2)<<8);
}

INLINE int READ_16(saturn_state *cpustate, SaturnAdr adr)
{
	return READ_NIBBLE(cpustate, adr)|(READ_NIBBLE(cpustate, adr+1)<<4)|(READ_NIBBLE(cpustate, adr+2)<<8)|(READ_NIBBLE(cpustate, adr+3)<<12);
}

INLINE int READ_20(saturn_state *cpustate, SaturnAdr adr)
{
	return READ_NIBBLE(cpustate, adr)|(READ_NIBBLE(cpustate, adr+1)<<4)|(READ_NIBBLE(cpustate, adr+2)<<8)|(READ_NIBBLE(cpustate, adr+3)<<12)|(READ_NIBBLE(cpustate, adr+4)<<16);
}

INLINE void WRITE_NIBBLE(saturn_state *cpustate, SaturnAdr adr, SaturnNib nib)
{
	cpustate->icount-=3;
	saturn_assert(nib<0x10);
	cpustate->program->write_byte(adr&0xfffff,nib);
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


INLINE int S64_READ_X(saturn_state *cpustate, int r)
{
	return cpustate->reg[r][0]|(cpustate->reg[r][1]<<4)|(cpustate->reg[r][2]<<8);
}

INLINE int S64_READ_WORD(saturn_state *cpustate, int r)
{
	return cpustate->reg[r][0]|(cpustate->reg[r][1]<<4)|(cpustate->reg[r][2]<<8)|(cpustate->reg[r][3]<<12);
}

INLINE int S64_READ_A(saturn_state *cpustate, int r)
{
	return cpustate->reg[r][0]|(cpustate->reg[r][1]<<4)|(cpustate->reg[r][2]<<8)|(cpustate->reg[r][3]<<12)|(cpustate->reg[r][4]<<16);
}

INLINE void S64_WRITE_X(saturn_state *cpustate, int r, int v)
{
	cpustate->reg[r][0]=v&0xf;
	cpustate->reg[r][1]=(v>>4)&0xf;
	cpustate->reg[r][2]=(v>>8)&0xf;
}

INLINE void S64_WRITE_WORD(saturn_state *cpustate, int r, int v)
{
	cpustate->reg[r][0]=v&0xf;
	cpustate->reg[r][1]=(v>>4)&0xf;
	cpustate->reg[r][2]=(v>>8)&0xf;
	cpustate->reg[r][3]=(v>>12)&0xf;
}

INLINE void S64_WRITE_A(saturn_state *cpustate, int r, int v)
{
	cpustate->reg[r][0]=v&0xf;
	cpustate->reg[r][1]=(v>>4)&0xf;
	cpustate->reg[r][2]=(v>>8)&0xf;
	cpustate->reg[r][3]=(v>>12)&0xf;
	cpustate->reg[r][4]=(v>>16)&0xf;
}





INLINE SaturnAdr saturn_pop(saturn_state *cpustate)
{
	SaturnAdr temp=cpustate->rstk[0];
	memmove(cpustate->rstk, cpustate->rstk+1, sizeof(cpustate->rstk)-sizeof(cpustate->rstk[0]));
	cpustate->rstk[7]=0;
	return temp;
}

INLINE void saturn_push(saturn_state *cpustate, SaturnAdr adr)
{
	memmove(cpustate->rstk+1, cpustate->rstk, sizeof(cpustate->rstk)-sizeof(cpustate->rstk[0]));
	cpustate->rstk[0]=adr;
}

INLINE void saturn_interrupt_on(saturn_state *cpustate)
{
	LOG(( "SATURN '%s' at %05x: INTON\n", cpustate->device->tag(), cpustate->pc-4 ));
	cpustate->irq_enable=1;
	if (cpustate->irq_state)
	{
		LOG(( "SATURN '%s' set_irq_line(ASSERT)\n", cpustate->device->tag()));
		cpustate->pending_irq=1;
	}
}

INLINE void saturn_interrupt_off(saturn_state *cpustate)
{
	LOG(( "SATURN '%s' at %05x: INTOFF\n", cpustate->device->tag(), cpustate->pc-4 ));
	cpustate->irq_enable=0;
}

INLINE void saturn_reset_interrupt(saturn_state *cpustate)
{
	LOG(( "SATURN '%s' at %05x: RSI\n", cpustate->device->tag(), cpustate->pc-5 ));
	if (cpustate->config&&cpustate->config->rsi) cpustate->config->rsi(cpustate->device);
}

INLINE void saturn_mem_reset(saturn_state *cpustate)
{
	if (cpustate->config&&cpustate->config->reset) cpustate->config->reset(cpustate->device);
}

INLINE void saturn_mem_config(saturn_state *cpustate)
{
	if (cpustate->config&&cpustate->config->config) cpustate->config->config(cpustate->device, S64_READ_A(cpustate, C));
}

INLINE void saturn_mem_unconfig(saturn_state *cpustate)
{
	if (cpustate->config&&cpustate->config->unconfig) cpustate->config->unconfig(cpustate->device, S64_READ_A(cpustate, C));
}

INLINE void saturn_mem_id(saturn_state *cpustate)
{
	int id=0;
	if (cpustate->config&&cpustate->config->id) id=cpustate->config->id(cpustate->device);
	S64_WRITE_A(cpustate, C,id);
	cpustate->monitor_id = id;
}

INLINE void saturn_shutdown(saturn_state *cpustate)
{
	cpustate->sleeping=1;
	cpustate->irq_enable=1;
	LOG(( "SATURN '%s' at %05x: SHUTDN\n", cpustate->device->tag(), cpustate->pc-3 ));
}

INLINE void saturn_bus_command_b(saturn_state *cpustate)
{
	logerror( "SATURN '%s' at %05x: BUSCB opcode not handled\n", cpustate->device->tag(), cpustate->pc-4 );
}

INLINE void saturn_bus_command_c(saturn_state *cpustate)
{
	logerror( "SATURN '%s' at %05x: BUSCC opcode not handled\n", cpustate->device->tag(), cpustate->pc-3 );
}

INLINE void saturn_bus_command_d(saturn_state *cpustate)
{
	logerror( "SATURN '%s' at %05x: BUSCD opcode not handled\n", cpustate->device->tag(), cpustate->pc-4 );
}

INLINE void saturn_serial_request(saturn_state *cpustate)
{
	logerror( "SATURN '%s' at %05x: SREQ? opcode not handled\n", cpustate->device->tag(), cpustate->pc-3 );
}

INLINE void saturn_out_c(saturn_state *cpustate)
{
	cpustate->out=S64_READ_X(cpustate, C);
	if (cpustate->config&&cpustate->config->out) cpustate->config->out(cpustate->device, cpustate->out);
}

INLINE void saturn_out_cs(saturn_state *cpustate)
{
	cpustate->out=(cpustate->out&0xff0)|cpustate->reg[C][0];
	if (cpustate->config&&cpustate->config->out) cpustate->config->out(cpustate->device, cpustate->out);
}

INLINE void saturn_in(saturn_state *cpustate, int reg)
{
	int in = 0;
	saturn_assert(reg>=0 && reg<9);
	if (!(cpustate->pc&1))
		logerror( "SATURN '%s' at %05x: reg=IN opcode at odd addresse\n",
				cpustate->device->tag(), cpustate->pc-3 );
	if (cpustate->config&&cpustate->config->in) in = cpustate->config->in(cpustate->device);
	S64_WRITE_WORD(cpustate, reg,in);
	cpustate->monitor_in = in;
}

INLINE void saturn_sethex(saturn_state *cpustate) { cpustate->decimal=0; }
INLINE void saturn_setdec(saturn_state *cpustate) { cpustate->decimal=1; }

/* st related */
INLINE void saturn_clear_st(saturn_state *cpustate)
{
	cpustate->st&=0xf000;
}

INLINE void saturn_st_to_c(saturn_state *cpustate)
{
	S64_WRITE_X(cpustate, C,cpustate->st);
}

INLINE void saturn_c_to_st(saturn_state *cpustate)
{
	cpustate->st=(cpustate->st&0xf000)|(S64_READ_X(cpustate, C));
}

INLINE void saturn_exchange_c_st(saturn_state *cpustate)
{
	int t=cpustate->st;
	cpustate->st=(t&0xf000)|(S64_READ_X(cpustate, C));
	S64_WRITE_X(cpustate, C,t);
}

INLINE void saturn_jump_after_test(saturn_state *cpustate)
{
	int adr=READ_OP_DIS8(cpustate);
	if (cpustate->carry) {
		if (adr==0) {
			cpustate->pc=saturn_pop(cpustate);
		} else {
			cpustate->pc=(cpustate->pc+adr-2)&0xfffff;
		}
	}
}
INLINE void saturn_st_clear_bit(saturn_state *cpustate)
{
	cpustate->st &= ~(1<<(READ_OP_ARG(cpustate)));
}

INLINE void saturn_st_set_bit(saturn_state *cpustate)
{
	cpustate->st |= (1<<(READ_OP_ARG(cpustate)));
}

INLINE void saturn_st_jump_bit_clear(saturn_state *cpustate)
{
	cpustate->carry=!((cpustate->st>>(READ_OP_ARG(cpustate)))&1);
	saturn_jump_after_test(cpustate);
}

INLINE void saturn_st_jump_bit_set(saturn_state *cpustate)
{
	cpustate->carry=(cpustate->st>>(READ_OP_ARG(cpustate)))&1;
	saturn_jump_after_test(cpustate);
}

INLINE void saturn_hst_clear_bits(saturn_state *cpustate)
{
	cpustate->hst&=~(READ_OP_ARG(cpustate));
}

INLINE void saturn_hst_bits_cleared(saturn_state *cpustate)
{
	cpustate->carry=!(cpustate->hst&(READ_OP_ARG(cpustate)));
	saturn_jump_after_test(cpustate);
}

/* p related */
INLINE void saturn_exchange_p(saturn_state *cpustate)
{
	int nr=READ_OP_ARG(cpustate);
	int t=cpustate->p;
	cpustate->p=cpustate->reg[C][nr];
	cpustate->reg[C][nr]=t;
}

INLINE void saturn_p_to_c(saturn_state *cpustate)
{
	int nr=READ_OP_ARG(cpustate);
	cpustate->reg[C][nr]=cpustate->p;
}

INLINE void saturn_c_to_p(saturn_state *cpustate)
{
	int nr=READ_OP_ARG(cpustate);
	cpustate->p=cpustate->reg[C][nr];
}

INLINE void saturn_dec_p(saturn_state *cpustate)
{
	cpustate->carry=cpustate->p==0;
	cpustate->p=(cpustate->p-1)&0xf;
}

INLINE void saturn_inc_p(saturn_state *cpustate)
{
	cpustate->p=(cpustate->p+1)&0xf;
	cpustate->carry=cpustate->p==0;
}

INLINE void saturn_load_p(saturn_state *cpustate)
{
	cpustate->p=READ_OP_ARG(cpustate);
}

INLINE void saturn_p_equals(saturn_state *cpustate)
{
	cpustate->carry=cpustate->p==(READ_OP_ARG(cpustate));
	saturn_jump_after_test(cpustate);
}

INLINE void saturn_p_not_equals(saturn_state *cpustate)
{
	cpustate->carry=cpustate->p!=(READ_OP_ARG(cpustate));
	saturn_jump_after_test(cpustate);
}

INLINE void saturn_ca_p_1(saturn_state *cpustate)
{
	int a=(S64_READ_A(cpustate, C))+1+cpustate->p;
	cpustate->carry=a>=0x100000;
	S64_WRITE_A(cpustate, C,a&0xfffff);
}

INLINE void saturn_load_reg(saturn_state *cpustate, int reg)
{
	int count=READ_OP_ARG(cpustate);
	int pos=cpustate->p;
	saturn_assert(reg>=0 && reg<9);
	for (; count>=0; count--, pos=(pos+1)&0xf ) {
		cpustate->reg[reg][pos]=READ_OP_ARG(cpustate);
	}
}

INLINE void saturn_jump(saturn_state *cpustate, int adr, int jump)
{
	saturn_assert(adr>=0 && adr<0x100000);
	if (jump) {
		cpustate->pc=adr;
		cpustate->icount-=10;
	}
}

INLINE void saturn_call(saturn_state *cpustate, int adr)
{
	saturn_assert(adr>=0 && adr<0x100000);
	saturn_push(cpustate, cpustate->pc);
	cpustate->pc=adr;
//  cpustate->icount-=10;
}

INLINE void saturn_return(saturn_state *cpustate, int yes)
{
	if (yes) {
		cpustate->pc=saturn_pop(cpustate);
//  cpustate->icount-=10;
	}
}

INLINE void saturn_return_carry_set(saturn_state *cpustate)
{
	cpustate->pc=saturn_pop(cpustate);
//  cpustate->icount-=10;
	cpustate->carry=1;
}

INLINE void saturn_return_carry_clear(saturn_state *cpustate)
{
	cpustate->pc=saturn_pop(cpustate);
//  cpustate->icount-=10;
	cpustate->carry=0;
}

INLINE void saturn_return_interrupt(saturn_state *cpustate)
{
	LOG(( "SATURN '%s' at %05x: RTI\n", cpustate->device->tag(), cpustate->pc-2 ));
	cpustate->in_irq=0; /* set to 1 when an IRQ is taken */
	cpustate->pc=saturn_pop(cpustate);
//  cpustate->icount-=10;
}

INLINE void saturn_return_xm_set(saturn_state *cpustate)
{
	cpustate->pc=saturn_pop(cpustate);
	cpustate->hst|=XM;
//  cpustate->icount-=10;
}

INLINE void saturn_pop_c(saturn_state *cpustate)
{
	S64_WRITE_A(cpustate, C,saturn_pop(cpustate));
}

INLINE void saturn_push_c(saturn_state *cpustate)
{
	saturn_push(cpustate, S64_READ_A(cpustate, C));
}

INLINE void saturn_indirect_jump(saturn_state *cpustate, int reg)
{
	saturn_assert(reg>=0 && reg<9);
	cpustate->pc=READ_20(cpustate, S64_READ_A(cpustate, reg));
}

INLINE void saturn_equals_zero(saturn_state *cpustate, int reg, int begin, int count)
{
	int i, t;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(begin>=0 && count>=0 && begin+count<=16);
	cpustate->carry=1;
	for (i=0; i<count; i++) {
		t=cpustate->reg[reg][begin+i];
		if (t!=0) { cpustate->carry=0; break; }
		cpustate->icount-=2;
	}
	saturn_jump_after_test(cpustate);
}

INLINE void saturn_equals(saturn_state *cpustate, int reg, int begin, int count, int right)
{
	int i, t,t2;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(right>=0 && right<9);
	saturn_assert(begin>=0 && count>=0 && begin+count<=16);
	cpustate->carry=1;
	for (i=0; i<count; i++) {
		t=cpustate->reg[reg][begin+i];
		t2=cpustate->reg[right][begin+i];
		if (t!=t2) { cpustate->carry=0; break; }
		cpustate->icount-=2;
	}
	saturn_jump_after_test(cpustate);
}

INLINE void saturn_not_equals_zero(saturn_state *cpustate, int reg, int begin, int count)
{
	int i, t;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(begin>=0 && count>=0 && begin+count<=16);
	cpustate->carry=0;
	for (i=0; i<count; i++) {
		t=cpustate->reg[reg][begin+i];
		if (t!=0) { cpustate->carry=1; break; }
		cpustate->icount-=2;
	}
	saturn_jump_after_test(cpustate);
}

INLINE void saturn_not_equals(saturn_state *cpustate, int reg, int begin, int count, int right)
{
	int i, t,t2;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(right>=0 && right<9);
	saturn_assert(begin>=0 && count>=0 && begin+count<=16);
	cpustate->carry=0;
	for (i=0; i<count; i++) {
		t=cpustate->reg[reg][begin+i];
		t2=cpustate->reg[right][begin+i];
		if (t!=t2) { cpustate->carry=1; break; }
		cpustate->icount-=2;
	}
	saturn_jump_after_test(cpustate);
}

INLINE void saturn_greater(saturn_state *cpustate, int reg, int begin, int count, int right)
{
	int i, t,t2;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(right>=0 && right<9);
	saturn_assert(begin>=0 && count>=0 && begin+count<=16);
	cpustate->carry=0;
	for (i=count-1; i>=0; i--) {
		t=cpustate->reg[reg][begin+i];
		t2=cpustate->reg[right][begin+i];
		if (t>t2) { cpustate->carry=1; break; }
		if (t<t2) break;
		cpustate->icount-=2;
	}
	saturn_jump_after_test(cpustate);
}

INLINE void saturn_greater_equals(saturn_state *cpustate, int reg, int begin, int count, int right)
{
	int i, t,t2;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(right>=0 && right<9);
	saturn_assert(begin>=0 && count>=0 && begin+count<=16);
	cpustate->carry=1;
	for (i=count-1; i>=0; i--) {
		t=cpustate->reg[reg][begin+i];
		t2=cpustate->reg[right][begin+i];
		if (t<t2) { cpustate->carry=0; break; }
		if (t>t2) break;
		cpustate->icount-=2;
	}
	saturn_jump_after_test(cpustate);
}

INLINE void saturn_smaller_equals(saturn_state *cpustate, int reg, int begin, int count, int right)
{
	int i, t,t2;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(right>=0 && right<9);
	saturn_assert(begin>=0 && count>=0 && begin+count<=16);
	cpustate->carry=1;
	for (i=count-1; i>=0; i--) {
		t=cpustate->reg[reg][begin+i];
		t2=cpustate->reg[right][begin+i];
		if (t>t2) { cpustate->carry=0; break; }
		if (t<t2) break;
		cpustate->icount-=2;
	}
	saturn_jump_after_test(cpustate);
}

INLINE void saturn_smaller(saturn_state *cpustate, int reg, int begin, int count, int right)
{
	int i, t,t2;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(right>=0 && right<9);
	saturn_assert(begin>=0 && count>=0 && begin+count<=16);
	cpustate->carry=0;
	for (i=count-1; i>=0; i--) {
		t=cpustate->reg[reg][begin+i];
		t2=cpustate->reg[right][begin+i];
		if (t<t2) { cpustate->carry=1; break; }
		if (t>t2) break;
		cpustate->icount-=2;
	}
	saturn_jump_after_test(cpustate);
}

INLINE void saturn_jump_bit_clear(saturn_state *cpustate, int reg)
{
	int op=READ_OP_ARG(cpustate);
	saturn_assert(reg>=0 && reg<9);
	cpustate->carry=!((cpustate->reg[reg][op>>2]>>(op&3))&1);
	saturn_jump_after_test(cpustate);
}

INLINE void saturn_jump_bit_set(saturn_state *cpustate, int reg)
{
	int op=READ_OP_ARG(cpustate);
	saturn_assert(reg>=0 && reg<9);
	cpustate->carry=(cpustate->reg[reg][op>>2]>>(op&3))&1;
	saturn_jump_after_test(cpustate);
}

INLINE void saturn_load_pc(saturn_state *cpustate, int reg)
{
	saturn_assert(reg>=0 && reg<9);
	cpustate->pc=S64_READ_A(cpustate, reg);
}

INLINE void saturn_store_pc(saturn_state *cpustate, int reg)
{
	saturn_assert(reg>=0 && reg<9);
	S64_WRITE_A(cpustate, reg,cpustate->pc);
}

INLINE void saturn_exchange_pc(saturn_state *cpustate, int reg)
{
	int temp=cpustate->pc;
	saturn_assert(reg>=0 && reg<9);
	cpustate->pc=S64_READ_A(cpustate, reg);
	S64_WRITE_A(cpustate, reg, temp);
}

/*************************************************************************************
 address register related
*************************************************************************************/
INLINE void saturn_load_adr(saturn_state *cpustate, int reg, int nibbles)
{
	saturn_assert(reg>=0 && reg<2);
	saturn_assert(nibbles==2 || nibbles==4 || nibbles==5);
	switch (nibbles) {
	case 5:
		cpustate->d[reg]=READ_OP_ARG20(cpustate);
		break;
	case 4:
		cpustate->d[reg]=(cpustate->d[reg]&0xf0000)|READ_OP_ARG16(cpustate);
		break;
	case 2:
		cpustate->d[reg]=(cpustate->d[reg]&0xfff00)|READ_OP_ARG8(cpustate);
		break;
	}
}

INLINE void saturn_add_adr(saturn_state *cpustate, int reg)
{
	int t=cpustate->d[reg]+READ_OP_ARG(cpustate)+1;
	saturn_assert(reg>=0 && reg<2);
	cpustate->d[reg]=t&0xfffff;
	cpustate->carry=t>=0x100000;
}

INLINE void saturn_sub_adr(saturn_state *cpustate, int reg)
{
	int t=cpustate->d[reg]-READ_OP_ARG(cpustate)-1;
	saturn_assert(reg>=0 && reg<2);
	cpustate->d[reg]=t&0xfffff;
	cpustate->carry=t<0;
}

INLINE void saturn_adr_to_reg(saturn_state *cpustate, int adr, int reg)
{
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(adr>=0 && adr<2);
	S64_WRITE_A(cpustate, reg,cpustate->d[adr]);
}

INLINE void saturn_reg_to_adr(saturn_state *cpustate, int reg, int adr)
{
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(adr>=0 && adr<2);
	cpustate->d[adr]=S64_READ_A(cpustate, reg);
}

INLINE void saturn_adr_to_reg_word(saturn_state *cpustate, int adr, int reg)
{
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(adr>=0 && adr<2);
	S64_WRITE_WORD(cpustate, reg,cpustate->d[adr]&0xffff);
}

INLINE void saturn_reg_to_adr_word(saturn_state *cpustate, int reg, int adr)
{
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(adr>=0 && adr<2);
	cpustate->d[adr]=(cpustate->d[adr]&0xf0000)|S64_READ_WORD(cpustate, reg);
}

INLINE void saturn_exchange_adr_reg(saturn_state *cpustate, int adr, int reg)
{
	int temp=cpustate->d[adr];
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(adr>=0 && adr<2);
	cpustate->d[adr]=S64_READ_A(cpustate, reg);
	S64_WRITE_A(cpustate, reg,temp);
}

INLINE void saturn_exchange_adr_reg_word(saturn_state *cpustate, int adr, int reg)
{
	int temp=cpustate->d[adr]&0xffff;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(adr>=0 && adr<2);
	cpustate->d[adr]=(cpustate->d[adr]&0xf0000)|S64_READ_WORD(cpustate, reg);
	S64_WRITE_WORD(cpustate, reg,temp);
}

INLINE void saturn_load_nibbles(saturn_state *cpustate, int reg, int begin, int count, int adr)
{
	int i;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(adr>=0 && adr<2);
	saturn_assert(begin>=0 && count>=0 && begin+count<=16);
	for (i=0; i<count; i++) {
		cpustate->reg[reg][begin+i]=READ_NIBBLE(cpustate, cpustate->d[adr]+i);
		cpustate->icount-=2;
	}
}

INLINE void saturn_store_nibbles(saturn_state *cpustate, int reg, int begin, int count, int adr)
{
	int i;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(adr>=0 && adr<2);
	saturn_assert(begin>=0 && count>=0 && begin+count<=16);
	for (i=0; i<count; i++) {
		WRITE_NIBBLE(cpustate,(cpustate->d[adr]+i)&0xfffff,cpustate->reg[reg][begin+i]);
		cpustate->icount-=2;
	}
}

INLINE void saturn_clear_bit(saturn_state *cpustate, int reg)
{
	int arg=READ_OP_ARG(cpustate);
	saturn_assert(reg>=0 && reg<9);
	cpustate->reg[reg][arg>>2]&=~(1<<(arg&3));
}

INLINE void saturn_set_bit(saturn_state *cpustate, int reg)
{
	int arg=READ_OP_ARG(cpustate);
	saturn_assert(reg>=0 && reg<9);
	cpustate->reg[reg][arg>>2]|=1<<(arg&3);
}

/****************************************************************************
 clear opers
 ****************************************************************************/
INLINE void saturn_clear(saturn_state *cpustate, int reg, int begin, int count)
{
	int i;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(begin>=0 && count>=0 && begin+count<=16);
	for (i=0; i<count; i++) {
		cpustate->reg[reg][begin+i]=0;
		cpustate->icount-=2;
	}
}

/****************************************************************************
 exchange opers
 ****************************************************************************/
INLINE void saturn_exchange(saturn_state *cpustate, int left, int begin, int count, int right)
{
	int i;
	SaturnNib temp;
	saturn_assert(left>=0 && left<9);
	saturn_assert(right>=0 && right<9);
	saturn_assert(begin>=0 && count>=0 && begin+count<=16);
	for (i=0; i<count; i++) {
		temp=cpustate->reg[left][begin+i];
		cpustate->reg[left][begin+i]=cpustate->reg[right][begin+i];
		cpustate->reg[right][begin+i]=temp;
		cpustate->icount-=2;
	}
}

/****************************************************************************
 copy opers
 ****************************************************************************/
INLINE void saturn_copy(saturn_state *cpustate, int dest, int begin, int count, int src)
{
	int i;
	saturn_assert(dest>=0 && dest<9);
	saturn_assert(src>=0 && src<9);
	saturn_assert(begin>=0 && count>=0 && begin+count<=16);
	for (i=0; i<count; i++) {
		cpustate->reg[dest][begin+i]=cpustate->reg[src][begin+i];
		cpustate->icount-=2;
	}
}

/****************************************************************************
 add opers
 ****************************************************************************/
INLINE void saturn_add(saturn_state *cpustate, int reg, int begin, int count, int right)
{
	int i, t;
	int base=cpustate->decimal?10:16;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(right>=0 && right<9);
	saturn_assert(begin>=0 && count>=0 && begin+count<=16);
	cpustate->carry=0;
	for (i=0; i<count; i++) {
		t=cpustate->reg[reg][begin+i];
		t+=cpustate->reg[right][begin+i];
		t+=cpustate->carry;
		if (t>=base) {
			cpustate->carry=1;
			t-=base;
		}
		else cpustate->carry=0;
		saturn_assert(t>=0); saturn_assert(t<base);
		cpustate->reg[reg][begin+i]=t&0xf;
		cpustate->icount-=2;
	}
}

INLINE void saturn_add_const(saturn_state *cpustate, int reg, int begin, int count, SaturnNib right)
{
	int i, t;
	int base=cpustate->decimal?10:16;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(begin>=0 && count>=0 && begin+count<=16);
	saturn_assert(count>1 || !cpustate->decimal); /* SATURN bug */
	for (i=0; i<count; i++) {
		t=cpustate->reg[reg][begin+i];
		t+=(right&0xf);
		right>>=4;
		if (t>=base) {
			right++;
			t-=base;
		}
		saturn_assert(t>=0); saturn_assert(t<base);
		cpustate->reg[reg][begin+i]=t&0xf;
		cpustate->icount-=2;
		if (!right) break;
	}
	cpustate->carry=right>0;
}

/****************************************************************************
 sub opers
 ****************************************************************************/
INLINE void saturn_sub(saturn_state *cpustate, int reg, int begin, int count, int right)
{
	int i, t;
	int base=cpustate->decimal?10:16;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(right>=0 && right<9);
	saturn_assert(begin>=0 && count>=0 && begin+count<=16);
	cpustate->carry=0;
	for (i=0; i<count; i++) {
		t=cpustate->reg[reg][begin+i];
		t-=cpustate->reg[right][begin+i];
		t-=cpustate->carry;
		if (t<0) {
			cpustate->carry=1;
			t+=base;
		}
		else cpustate->carry=0;
		saturn_assert(t>=0); saturn_assert(t<base);
		cpustate->reg[reg][begin+i]=t&0xf;
		cpustate->icount-=2;
	}
}

INLINE void saturn_sub_const(saturn_state *cpustate, int reg, int begin, int count, int right)
{
	int i, t;
	int base=cpustate->decimal?10:16;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(begin>=0 && count>=0 && begin+count<=16);
	saturn_assert(count>1 || !cpustate->decimal); /* SATURN bug */
	for (i=0; i<count; i++) {
		t=cpustate->reg[reg][begin+i];
		t-=(right&0xf);
		right>>=4;
		if (t<0) {
			right++;
			t+=base;
		}
		saturn_assert(t>=0); saturn_assert(t<base);
		cpustate->reg[reg][begin+i]=t&0xf;
		cpustate->icount-=2;
		if (!right) break;
	}
	cpustate->carry=right>0;
}

/****************************************************************************
 sub2 opers (a=b-a)
 ****************************************************************************/
INLINE void saturn_sub2(saturn_state *cpustate, int reg, int begin, int count, int right)
{
	int i, t;
	int base=cpustate->decimal?10:16;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(right>=0 && right<9);
	saturn_assert(begin>=0 && count>=0 && begin+count<=16);
	cpustate->carry=0;
	for (i=0; i<count; i++) {
		t=cpustate->reg[right][begin+i];
		t-=cpustate->reg[reg][begin+i];
		t-=cpustate->carry;
		if (t<0) {
			cpustate->carry=1;
			t+=base;
		}
		else cpustate->carry=0;
		saturn_assert(t>=0); saturn_assert(t<base);
		cpustate->reg[reg][begin+i]=t&0xf;
		cpustate->icount-=2;
	}
}

/****************************************************************************
 increment opers
 ****************************************************************************/
INLINE void saturn_increment(saturn_state *cpustate, int reg, int begin, int count)
{
	int i, t=0;
	int base=cpustate->decimal?10:16;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(begin>=0 && count>=0 && begin+count<=16);
	for (i=0; i<count; i++) {
		cpustate->icount-=2;
		t=cpustate->reg[reg][begin+i];
		t++;
		if (t>=base) cpustate->reg[reg][begin+i]=t-base;
		else { cpustate->reg[reg][begin+i]=t; break; }
	}
	cpustate->carry=t>=base;
}

/****************************************************************************
 decrement opers
 ****************************************************************************/
INLINE void saturn_decrement(saturn_state *cpustate, int reg, int begin, int count)
{
	int i, t=0;
	int base=cpustate->decimal?10:16;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(begin>=0 && count>=0 && begin+count<=16);
	for (i=0; i<count; i++) {
		cpustate->icount-=2;
		t=cpustate->reg[reg][begin+i];
		t--;
		if (t<0) cpustate->reg[reg][begin+i]=t+base;
		else { cpustate->reg[reg][begin+i]=t; break; }
	}
	cpustate->carry=t<0;
}

/****************************************************************************
 invert (1 complement)  opers
 ****************************************************************************/
INLINE void saturn_invert(saturn_state *cpustate, int reg, int begin, int count)
{
	int i;
	int max=cpustate->decimal?9:15;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(begin>=0 && count>=0 && begin+count<=16);
	cpustate->carry=0;
	for (i=0; i<count; i++) {
		cpustate->reg[reg][begin+i]=(max-cpustate->reg[reg][begin+i])&0xf;
		cpustate->icount-=2;
	}
}

/****************************************************************************
 negate (2 complement)  opers
 ****************************************************************************/
INLINE void saturn_negate(saturn_state *cpustate, int reg, int begin, int count)
{
	int i, n, c;
	int max=cpustate->decimal?9:15;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(begin>=0 && count>=0 && begin+count<=16);
	c=1;
	cpustate->carry=0;
	for (i=0; i<count; i++) {
		n=cpustate->reg[reg][begin+i];
		if (n) cpustate->carry=1;
		n=max+c-n;
		if (n>max) n-=max+1;
		else c=0;
		saturn_assert(n>=0); saturn_assert(n<=max);
		cpustate->reg[reg][begin+i]=n&0xf;
		cpustate->icount-=2;
	}
}

/****************************************************************************
 or opers
 ****************************************************************************/
INLINE void saturn_or(saturn_state *cpustate, int dest, int begin, int count, int src)
{
	int i;
	saturn_assert(dest>=0 && dest<9);
	saturn_assert(src>=0 && src<9);
	saturn_assert(begin>=0 && count>=0 && begin+count<=16);
	for (i=0; i<count; i++) {
		cpustate->reg[dest][begin+i]|=cpustate->reg[src][begin+i];
		cpustate->icount-=2;
	}
}

/****************************************************************************
 and opers
 ****************************************************************************/
INLINE void saturn_and(saturn_state *cpustate, int dest, int begin, int count, int src)
{
	int i;
	saturn_assert(dest>=0 && dest<9);
	saturn_assert(src>=0 && src<9);
	saturn_assert(begin>=0 && count>=0 && begin+count<=16);
	for (i=0; i<count; i++) {
		cpustate->reg[dest][begin+i]&=cpustate->reg[src][begin+i];
		cpustate->icount-=2;
	}
}

/****************************************************************************
 shift nibbles left opers
 ****************************************************************************/
INLINE void saturn_shift_nibble_left(saturn_state *cpustate, int reg, int begin, int count)
{
	int i;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(begin>=0 && count>=0 && begin+count<=16);
	if (cpustate->reg[reg][begin+count-1]) cpustate->hst|=SB;
	for (i=count-1; i>=1; i--) {
		cpustate->reg[reg][begin+i]=cpustate->reg[reg][begin+i-1];
		cpustate->icount-=2;
	}
	cpustate->reg[reg][begin]=0;
	cpustate->icount-=2;
}

/****************************************************************************
 shift nibbles right opers
 ****************************************************************************/
INLINE void saturn_shift_nibble_right(saturn_state *cpustate, int reg, int begin, int count)
{
	int i;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(begin>=0 && count>=0 && begin+count<=16);
	if (cpustate->reg[reg][begin]) cpustate->hst|=SB;
	for (i=1; i<count; i++) {
		cpustate->reg[reg][begin+i-1]=cpustate->reg[reg][begin+i];
		cpustate->icount-=2;
	}
	cpustate->reg[reg][begin+count-1]=0;
	cpustate->icount-=2;
}


/****************************************************************************
 rotate nibbles left opers
 ****************************************************************************/
INLINE void saturn_rotate_nibble_left_w(saturn_state *cpustate, int reg)
{
	int i, x=cpustate->reg[reg][15];
	saturn_assert(reg>=0 && reg<9);
	for (i=15; i>=1; i--) {
		cpustate->reg[reg][i]=cpustate->reg[reg][i-1];
		cpustate->icount-=2;
	}
	cpustate->reg[reg][0]=x;
	cpustate->icount-=2;
}

/****************************************************************************
 rotate nibbles right opers
 ****************************************************************************/
INLINE void saturn_rotate_nibble_right_w(saturn_state *cpustate, int reg)
{
	int i, x=cpustate->reg[reg][0];
	saturn_assert(reg>=0 && reg<9);
	for (i=1; i<16; i++) {
		cpustate->reg[reg][i-1]=cpustate->reg[reg][i];
		cpustate->icount-=2;
	}
	cpustate->reg[reg][15]=x;
	if (x) cpustate->hst|=SB;
	cpustate->icount-=2;
}


/****************************************************************************
 shift right opers
 ****************************************************************************/
INLINE void saturn_shift_right(saturn_state *cpustate, int reg, int begin, int count)
{
	int i, t, c=0;
	saturn_assert(reg>=0 && reg<9);
	saturn_assert(begin>=0 && count>=0 && begin+count<=16);
	for (i=count-1; i>=0; i--) {
		t=cpustate->reg[reg][begin+i];
		t|=(c<<4);
		c=t&1;
		cpustate->reg[reg][begin+i]=t>>1;
		cpustate->icount-=2;
	}
	if (c) cpustate->hst|=SB;
	cpustate->icount-=2;
}
