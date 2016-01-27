// license:BSD-3-Clause
// copyright-holders:Peter Trauner,Antoine Mine
static const int adr_a_begin[]={
-1, -1, BEGIN_XS, BEGIN_X, BEGIN_S, BEGIN_M, BEGIN_B, BEGIN_W,
-1, -1, -1, -1,  -1, -1, -1, -1,
};

static const int adr_a_count[]={
-1, -1, COUNT_XS, COUNT_X, COUNT_S, COUNT_M, COUNT_B, COUNT_W,
-1, -1, -1, -1,  -1, -1, -1, -1,
};

static const int adr_b_begin[]={
-1, -1, -1, -1,  -1, -1, -1, -1,
-1, -1, BEGIN_XS, BEGIN_X, BEGIN_S, BEGIN_M, BEGIN_B, BEGIN_W,
};

static const int adr_b_count[]={
-1, -1, -1, -1,  -1, -1, -1, -1,
-1, -1, COUNT_XS, COUNT_X, COUNT_S, COUNT_M, COUNT_B, COUNT_W,
};

static const int adr_af_begin[]={
-1, -1, BEGIN_XS, BEGIN_X, BEGIN_S, BEGIN_M, BEGIN_B, BEGIN_W,
-1, -1, -1, -1, -1, -1, -1, BEGIN_A
};

static const int adr_af_count[]={
-1, -1, COUNT_XS, COUNT_X, COUNT_S, COUNT_M, COUNT_B, COUNT_W,
-1, -1, -1, -1, -1, -1, -1, COUNT_A
};

static const int reg_left[] ={A,B,C,D, B,C,A,C, I,I,I,I, I,I,I,I};
static const int reg_right[]={B,C,A,C, A,B,C,D, I,I,I,I, I,I,I,I};
static const int add_left[] ={A,B,C,D, I,I,I,I, B,C,A,C, I,I,I,I};
static const int add_right[]={B,C,A,C, I,I,I,I, A,B,C,D, I,I,I,I};
static const int sub_left[] ={A,B,C,D, I,I,I,I, B,C,A,C, A,B,C,D};
static const int sub_right[]={B,C,A,C, I,I,I,I, A,B,C,D, B,C,A,C};

void saturn_device::saturn_invalid3( int op1, int op2, int op3 )
{
	logerror( "SATURN '%s' invalid opcode %x%x%x at %05x\n",
			tag(), op1, op2, op3, m_pc-3 );
}

void saturn_device::saturn_invalid4( int op1, int op2, int op3, int op4 )
{
	logerror( "SATURN '%s' invalid opcode %x%x%x%x at %05x\n",
			tag(), op1, op2, op3, op4, m_pc-4 );
}

void saturn_device::saturn_invalid5( int op1, int op2, int op3, int op4, int op5 )
{
	logerror( "SATURN '%s' invalid opcode %x%x%x%x%x at %05x\n",
			tag(), op1, op2, op3, op4, op5, m_pc-5 );
}

void saturn_device::saturn_invalid6( int op1, int op2, int op3, int op4, int op5, int op6 )
{
	logerror( "SATURN '%s' invalid opcode %x%x%x%x%x%x at %05x\n",
			tag(), op1, op2, op3, op4, op5, op6, m_pc-6 );
}


void saturn_device::saturn_instruction_0e()
{
	int reg, adr;

	switch(adr=READ_OP()) {
	case 0:
		switch(reg=READ_OP()){
		case 0: case 1: case 2: case 3: case 4: case 5: case 6: case 7:
			saturn_and(reg_left[reg], m_p, 1, reg_right[reg]);
			break; //A=A&B p
		case 8: case 9: case 0xa: case 0xb: case 0xc: case 0xd: case 0xe: case 0xf:
			saturn_or(reg_left[reg&7], m_p, 1, reg_right[reg&7]);
			break; //A=A!B p
		}
		break;
	case 1:
		switch(reg=READ_OP()){
		case 0: case 1: case 2: case 3:case 4: case 5: case 6: case 7:
			saturn_and(reg_left[reg], 0, m_p+1, reg_right[reg]);
			break; //A=A&B wp
		case 8: case 9: case 0xa: case 0xb: case 0xc: case 0xd: case 0xe: case 0xf:
			saturn_or(reg_left[reg&7], 0, m_p+1, reg_right[reg&7]);
			break; //A=A!B wp
		}
		break;
	case 2: case 3: case 4: case 5: case 6: case 7: case 0xf:
		switch(reg=READ_OP()){
		case 0: case 1: case 2: case 3: case 4: case 5: case 6: case 7:
			saturn_and(reg_left[reg], adr_af_begin[adr], adr_af_count[adr], reg_right[reg]);
			break; //A=A&B xs
		case 8: case 9: case 0xa: case 0xb: case 0xc: case 0xd: case 0xe: case 0xf:
			saturn_or(reg_left[reg&7], adr_af_begin[adr], adr_af_count[adr], reg_right[reg&7]);
			break; //A=A!B xs
		}
		break;
	default:
		saturn_invalid3( 0, 0xe, adr );
		break;
	}
}

void saturn_device::saturn_instruction_1()
{
	int reg, adr, oper;

	switch (adr=READ_OP()) {
	case 0:
		switch (reg=READ_OP()) {
		case 0: case 1: case 2: case 3: case 4:
			saturn_copy(R0+reg, BEGIN_W, COUNT_W, A);
			break; // r0=a w
		case 8: case 9: case 0xa: case 0xb: case 0xc:
			saturn_copy(R0+(reg&7), BEGIN_W, COUNT_W, C);
			break; // r0=c w
		default:
			saturn_invalid3( 1, adr, reg );
			break;
		}
		break;
	case 1:
		switch (reg=READ_OP()) {
		case 0: case 1: case 2: case 3: case 4:
			saturn_copy(A, BEGIN_W, COUNT_W, R0+reg);
			break; // a=r0 w
		case 8: case 9: case 0xa: case 0xb: case 0xc:
			saturn_copy(C, BEGIN_W, COUNT_W, R0+(reg&7));
			break; // c=r0 w
		default:
			saturn_invalid3( 1, adr, reg );
			break;
		}
		break;
	case 2:
		switch (reg=READ_OP()) {
		case 0: case 1: case 2: case 3: case 4:
			saturn_exchange(A, BEGIN_W, COUNT_W, R0+reg);
			break; // ar0ex w
		case 8: case 9: case 0xa: case 0xb: case 0xc:
			saturn_exchange(C, BEGIN_W, COUNT_W, R0+(reg&7));
			break; // cr0ex w
		default:
			saturn_invalid3( 2, adr, reg );
			break;
		}
		break;
	case 3:
		switch (READ_OP()) {
		case 0: saturn_reg_to_adr(A,0);break;
		case 1: saturn_reg_to_adr(A,1);break;
		case 2: saturn_exchange_adr_reg(0,A);break;
		case 3: saturn_exchange_adr_reg(1,A);break;
		case 4: saturn_reg_to_adr(C,0);break;
		case 5: saturn_reg_to_adr(C,1);break;
		case 6: saturn_exchange_adr_reg(0,C);break;
		case 7: saturn_exchange_adr_reg(1,C);break;
		case 8: saturn_reg_to_adr_word(A,0);break;
		case 9: saturn_reg_to_adr_word(A,1);break;
		case 0xa: saturn_exchange_adr_reg_word(0,A);break;
		case 0xb: saturn_exchange_adr_reg_word(1,A);break;
		case 0xc: saturn_reg_to_adr_word(C,0);break;
		case 0xd: saturn_reg_to_adr_word(C,1);break;
		case 0xe: saturn_exchange_adr_reg_word(0,C);break;
		case 0xf: saturn_exchange_adr_reg_word(1,C);break;
		}
		break;
	case 4:
		switch (READ_OP()) {
		case 0: saturn_store_nibbles(A, BEGIN_A, COUNT_A, 0); break;
		case 1: saturn_store_nibbles(A, BEGIN_A, COUNT_A, 1); break;
		case 2: saturn_load_nibbles(A, BEGIN_A, COUNT_A, 0); break;
		case 3: saturn_load_nibbles(A, BEGIN_A, COUNT_A, 1); break;
		case 4: saturn_store_nibbles(C, BEGIN_A, COUNT_A, 0); break;
		case 5: saturn_store_nibbles(C, BEGIN_A, COUNT_A, 1); break;
		case 6: saturn_load_nibbles(C, BEGIN_A, COUNT_A, 0); break;
		case 7: saturn_load_nibbles(C, BEGIN_A, COUNT_A, 1); break;
		case 8: saturn_store_nibbles(A, BEGIN_B, COUNT_B, 0); break;
		case 9: saturn_store_nibbles(A, BEGIN_B, COUNT_B, 1); break;
		case 0xa: saturn_load_nibbles(A, BEGIN_B, COUNT_B, 0); break;
		case 0xb: saturn_load_nibbles(A, BEGIN_B, COUNT_B, 1); break;
		case 0xc: saturn_store_nibbles(C, BEGIN_B, COUNT_B, 0); break;
		case 0xd: saturn_store_nibbles(C, BEGIN_B, COUNT_B, 1); break;
		case 0xe: saturn_load_nibbles(C, BEGIN_B, COUNT_B, 0); break;
		case 0xf: saturn_load_nibbles(C, BEGIN_B, COUNT_B, 1); break;
		}
		break;
	case 5:
		switch (oper=READ_OP()) {
		case 0: case 1: case 4: case 5:
			switch (adr=READ_OP()) {
			case 0:
				saturn_store_nibbles(oper&4?C:A,m_p,1,oper&1);
				break;
			case 1:
				saturn_store_nibbles(oper&4?C:A,0,m_p+1,oper&1);
				break;
			case 2: case 3: case 4: case 5: case 6: case 7:
				saturn_store_nibbles(oper&4?C:A,adr_a_begin[adr],adr_a_count[adr],oper&1);
				break;
			default:
				saturn_invalid4( 1, 5, oper, adr );
				break;
			}
			break;
		case 2: case 3: case 6: case 7:
			switch (adr=READ_OP()) {
			case 0:
				saturn_load_nibbles(oper&4?C:A,m_p,1,oper&1);
				break;
			case 1:
				saturn_load_nibbles(oper&4?C:A,0,m_p+1,oper&1);
				break;
			case 2: case 3: case 4: case 5: case 6: case 7:
				saturn_load_nibbles(oper&4?C:A,adr_a_begin[adr],adr_a_count[adr],oper&1);
				break;
			default:
				saturn_invalid4( 1, 5, oper, adr );
				break;
			}
			break;
		case 8: saturn_store_nibbles(A, 0, READ_OP()+1, 0); break;
		case 9: saturn_store_nibbles(A, 0, READ_OP()+1, 1); break;
		case 0xa: saturn_load_nibbles(A, 0, READ_OP()+1, 0); break;
		case 0xb: saturn_load_nibbles(A, 0, READ_OP()+1, 1); break;
		case 0xc: saturn_store_nibbles(C, 0, READ_OP()+1, 0); break;
		case 0xd: saturn_store_nibbles(C, 0, READ_OP()+1, 1); break;
		case 0xe: saturn_load_nibbles(C, 0, READ_OP()+1, 0); break;
		case 0xf: saturn_load_nibbles(C, 0, READ_OP()+1, 1); break;
		}
		break;
	case 6: saturn_add_adr(0);break;
	case 7: saturn_add_adr(1);break;
	case 8: saturn_sub_adr(0);break;
	case 9: saturn_load_adr(0,2);break;
	case 0xa: saturn_load_adr(0,4);break;
	case 0xb: saturn_load_adr(0,5);break;
	case 0xc: saturn_sub_adr(1);break;
	case 0xd: saturn_load_adr(1,2);break;
	case 0xe: saturn_load_adr(1,4);break;
	case 0xf: saturn_load_adr(1,5);break;
	}
}

void saturn_device::saturn_instruction_80()
{
	int op;
	switch(READ_OP()) {
	case 0: saturn_out_cs();break;
	case 1: saturn_out_c();break;
	case 2: saturn_in(A);break;
	case 3: saturn_in(C);break;
	case 4: saturn_mem_unconfig();break;
	case 5: saturn_mem_config();break;
	case 6: saturn_mem_id();break;
	case 7: saturn_shutdown();break;
	case 8:
		switch(READ_OP()) {
		case 0: saturn_interrupt_on();break;
		case 1:
			switch(op=READ_OP()) {
			case 0: saturn_reset_interrupt();break;
			default: saturn_invalid5( 8, 0, 8, 1, op ); break;
			}
			break;
		case 2: saturn_load_reg(A);break; //la
		case 3: saturn_bus_command_b();break;
		case 4: saturn_clear_bit(A);break; // abit=0
		case 5: saturn_set_bit(A);break; // abit=1
		case 6: saturn_jump_bit_clear(A);break;
		case 7: saturn_jump_bit_set(A);break;
		case 8: saturn_clear_bit(C);break; // cbit=0
		case 9: saturn_set_bit(C);break; // cbit=1
		case 0xa: saturn_jump_bit_clear(C);break;
		case 0xb: saturn_jump_bit_set(C);break;
		case 0xc: saturn_indirect_jump(A);break;
		case 0xd: saturn_bus_command_d();break;
		case 0xe: saturn_indirect_jump(C);break;
		case 0xf: saturn_interrupt_off();break;
		}
		break;
	case 9: saturn_ca_p_1();break;//C+P+1
	case 0xa: saturn_mem_reset();break;
	case 0xb: saturn_bus_command_b();break;
	case 0xc: saturn_p_to_c();break;
	case 0xd: saturn_c_to_p();break;
	case 0xe: saturn_serial_request();break;
	case 0xf: saturn_exchange_p();break;
	}
}

void saturn_device::saturn_instruction_81a()
{
	int reg, adr,op;
	switch(adr=READ_OP()) {
	case 0:
		switch(op=READ_OP()) {
		case 0:
			switch(reg=READ_OP()) {
			case 0: case 1: case 2: case 3: case 4:
				saturn_copy(R0+reg,m_p,1,A);
				break; //r0=a p
			case 8: case 9: case 0xa: case 0xb: case 0xc:
				saturn_copy(R0+(reg&7),m_p,1,C);
				break; //r0=c p
			default:
				saturn_invalid6( 8, 1, 0xa, adr, op, reg);
				break;
			}
			break;
		case 1:
			switch(reg=READ_OP()) {
			case 0: case 1: case 2: case 3: case 4:
				saturn_copy(A,m_p,1,R0+reg);
				break; //a=r0 p
			case 8: case 9: case 0xa: case 0xb: case 0xc:
				saturn_copy(C,m_p,1,R0+(reg&7));
				break; //c=r0 p
			default:
				saturn_invalid6( 8, 1, 0xa, adr, op, reg);
				break;
			}
			break;
		case 2:
			switch (reg=READ_OP()) {
			case 0: case 1: case 2: case 3: case 4:
				saturn_exchange(A, m_p,1,R0+reg);
				break; // ar0ex p
			case 8: case 9: case 0xa: case 0xb: case 0xc:
				saturn_exchange(C, m_p,1,R0+(reg&7));
				break; // cr0ex p
			default:
				saturn_invalid6( 8, 1, 0xa, adr, op, reg);
				break;
			}
			break;
		default:
			saturn_invalid5( 8, 1, 0xa, adr, op );
			break;
		}
		break;
	case 1:
		switch(op=READ_OP()) {
		case 0:
			switch(reg=READ_OP()) {
			case 0: case 1: case 2: case 3: case 4:
				saturn_copy(R0+reg,0,m_p+1,A);
				break; //r0=a wp
			case 8: case 9: case 0xa: case 0xb: case 0xc:
				saturn_copy(R0+(reg&7),0,m_p+1,C);
				break; //r0=c wp
			default:
				saturn_invalid6( 8, 1, 0xa, adr, op, reg);
				break;
			}
			break;
		case 1:
			switch(reg=READ_OP()) {
			case 0: case 1: case 2: case 3: case 4:
				saturn_copy(A,0,m_p+1,R0+reg);
				break; //a=r0 wp
			case 8: case 9: case 0xa: case 0xb: case 0xc:
				saturn_copy(C,0,m_p+1,R0+(reg&7));
				break; //c=r0 wp
			default:
				saturn_invalid6( 8, 1, 0xa, adr, op, reg);
				break;
			}
			break;
		case 2:
			switch (reg=READ_OP()) {
			case 0: case 1: case 2: case 3: case 4:
				saturn_exchange(A, 0, m_p+1, R0+reg);
				break; // ar0ex wp
			case 8: case 9: case 0xa: case 0xb: case 0xc:
				saturn_exchange(C, 0, m_p+1, R0+(reg&7));
				break; // cr0ex wp
			default:
				saturn_invalid6( 8, 1, 0xa, adr, op, reg);
				break;
			}
			break;
		default:
			saturn_invalid5( 8, 1, 0xa, adr, op );
			break;
		}
		break;
	case 2: case 3: case 4: case 5: case 6: case 7: case 0xf:
		switch(op=READ_OP()) {
		case 0:
			switch(reg=READ_OP()) {
			case 0: case 1: case 2: case 3: case 4:
				saturn_copy(R0+reg,adr_af_begin[adr],adr_af_count[adr],A);
				break; //r0=a xs
			case 8: case 9: case 0xa: case 0xb: case 0xc:
				saturn_copy(R0+(reg&7),adr_af_begin[adr], adr_af_count[adr],C);
				break; //r0=c xs
			default:
				saturn_invalid6( 8, 1, 0xa, adr, op, reg);
				break;
			}
			break;
		case 1:
			switch(reg=READ_OP()) {
			case 0: case 1: case 2: case 3: case 4:
				saturn_copy(A,adr_af_begin[adr],adr_af_count[adr],R0+reg);
				break; //a=r0 xs
			case 8: case 9: case 0xa: case 0xb: case 0xc:
				saturn_copy(C,adr_af_begin[adr],adr_af_count[adr],R0+(reg&7));
				break; //c=r0 xs
			default:
				saturn_invalid6( 8, 1, 0xa, adr, op, reg);
				break;
			}
			break;
		case 2:
			switch (reg=READ_OP()) {
			case 0: case 1: case 2: case 3: case 4:
				saturn_exchange(A, adr_af_begin[adr], adr_af_count[adr], R0+reg);
				break; // ar0ex xs
			case 8: case 9: case 0xa: case 0xb: case 0xc:
				saturn_exchange(C, adr_af_begin[adr], adr_af_count[adr], R0+(reg&7));
				break; // cr0ex xs
			default:
				saturn_invalid6( 8, 1, 0xa, adr, op, reg);
				break;
			}
			break;
		default:
			saturn_invalid5( 8, 1, 0xa, adr, op );
			break;
		}
		break;
	default:
		saturn_invalid4( 8, 1, 0xa, adr );
		break;
	}
}

void saturn_device::saturn_instruction_81()
{
	int reg, adr;

	switch(reg=READ_OP()) {
	case 0: case 1: case 2: case 3:
		saturn_rotate_nibble_left_w(A+reg); break; // aslc w
	case 4: case 5: case 6: case 7:
		saturn_rotate_nibble_right_w(A+(reg&3)); break; // asrc w
	case 8:
		switch(adr=READ_OP()) {
		case 0:
			switch (reg=READ_OP()) {
			case 0: case 1: case 2: case 3:
				saturn_add_const(A+reg, m_p, 1, READ_OP()+1);
				break;
			case 8: case 9: case 0xa: case 0xb:
				saturn_sub_const(A+(reg&3), m_p, 1, READ_OP()+1);
				break;
			default:
				saturn_invalid5( 8, 1, 8, adr, reg );
				break;
			}
			break;
		case 1:
			switch (reg=READ_OP()) {
			case 0: case 1: case 2: case 3:
				saturn_add_const(A+reg, 0, m_p+1, READ_OP()+1);
				break;
			case 8: case 9: case 0xa: case 0xb:
				saturn_sub_const(A+(reg&3), 0, m_p+1, READ_OP()+1);
				break;
			default:
				saturn_invalid5( 8, 1, 8, adr, reg );
				break;
			}
			break;
		case 2: case 3: case 4: case 5: case 6: case 7: case 0xf:
			switch (reg=READ_OP()) {
			case 0: case 1: case 2: case 3:
				saturn_add_const(A+reg, adr_af_begin[adr], adr_af_count[adr], READ_OP()+1);
				break;
			case 8: case 9: case 0xa: case 0xb:
				saturn_sub_const(A+(reg&3), adr_af_begin[adr], adr_af_count[adr], READ_OP()+1);
				break;
			default:
				saturn_invalid5( 8, 1, 8, adr, reg );
				break;
			}
			break;
		default:
			saturn_invalid4( 8, 1, 8, adr );
			break;
		}
		break;
	case 9:
		switch(adr=READ_OP()) {
		case 0:
			switch(reg=READ_OP()){
			case 0: case 1: case 2: case 3:
				saturn_shift_right(A+reg,m_p,1);
				break; // asrb p
			default:
				saturn_invalid5( 8, 1, 9, adr, reg );
				break;
			}
			break;
		case 1:
			switch(reg=READ_OP()){
			case 0: case 1: case 2: case 3:
				saturn_shift_right(A+reg, 0,m_p+1);
				break; // asrb wp
			default:
				saturn_invalid5( 8, 1, 9, adr, reg );
				break;
			}
			break;
		case 2: case 3: case 4: case 5: case 6: case 7: case 0xf:
			switch(reg=READ_OP()){
			case 0: case 1: case 2: case 3:
				saturn_shift_right(A+reg, adr_af_begin[adr], adr_af_count[adr]);
				break; // asrb xs
			default:
				saturn_invalid5( 8, 1, 9, adr, reg );
				break;
			}
			break;
		default:
			saturn_invalid4( 8, 1, 9, adr );
			break;
		}
		break;
	case 0xa:
		saturn_instruction_81a();
		break;
	case 0xb:
		switch(adr=READ_OP()) {
		case 2: saturn_load_pc(A);break;
		case 3: saturn_load_pc(C);break;
		case 4: saturn_store_pc(A);break;
		case 5: saturn_store_pc(C);break;
		case 6: saturn_exchange_pc(A);break;
		case 7: saturn_exchange_pc(C);break;
		default: saturn_invalid4( 8, 1, reg, adr ); break;
		}
		break;
	case 0xc: case 0xd: case 0xe: case 0xf:
		saturn_shift_right(A+(reg&3), BEGIN_W, COUNT_W);
		break; // asrb w
	}
}

void saturn_device::saturn_instruction_8()
{
	int oper, adr;

	switch(READ_OP()) {
	case 0:
		saturn_instruction_80();
		break;
	case 1:
		saturn_instruction_81();
		break;
	case 2: saturn_hst_clear_bits();break;
	case 3: saturn_hst_bits_cleared();break;
	case 4: saturn_st_clear_bit();break;
	case 5: saturn_st_set_bit();break;
	case 6: saturn_st_jump_bit_clear();break;
	case 7: saturn_st_jump_bit_set();break;
	case 8: saturn_p_not_equals(); break;
	case 9: saturn_p_equals(); break;
	case 0xa:
		switch(oper=READ_OP()) {
		case 0: case 1: case 2: case 3:
			saturn_equals(reg_left[oper&3] , BEGIN_A, COUNT_A, reg_right[oper&3]);
			break;
		case 4: case 5: case 6: case 7:
			saturn_not_equals(reg_left[oper&3] , BEGIN_A, COUNT_A, reg_right[oper&3]);
			break;
		case 8: case 9: case 0xa: case 0xb:
			saturn_equals_zero(A+(oper&3), BEGIN_A, COUNT_A);
			break;
		case 0xc: case 0xd: case 0xe: case 0xf:
			saturn_not_equals_zero(A+(oper&3), BEGIN_A, COUNT_A);
			break;
		}
		break;
	case 0xb:
		switch(oper=READ_OP()) {
		case 0: case 1: case 2: case 3:
			saturn_greater(reg_left[oper&3] , BEGIN_A, COUNT_A, reg_right[oper&3]);
			break;
		case 4: case 5: case 6: case 7:
			saturn_smaller(reg_left[oper&3] , BEGIN_A, COUNT_A, reg_right[oper&3]);
			break;
		case 8: case 9: case 0xa: case 0xb:
			saturn_greater_equals(reg_left[oper&3], BEGIN_A, COUNT_A, reg_right[oper&3]);
			break;
		case 0xc: case 0xd: case 0xe: case 0xf:
			saturn_smaller_equals(reg_left[oper&3], BEGIN_A, COUNT_A, reg_right[oper&3]);
			break;
		}
		break;
	case 0xc:
		adr=READ_OP_DIS16();
		saturn_jump((adr+m_pc-4)&0xfffff,1);
		break;
	case 0xd:
		adr=READ_OP_ARG20();
		saturn_jump(adr,1);
		break;
	case 0xe:
		adr=READ_OP_DIS16();
		saturn_call((adr+m_pc)&0xfffff);
		break;
	case 0xf:
		adr=READ_OP_ARG20();
		saturn_call(adr);
		break;
	}
}

void saturn_device::saturn_instruction_9()
{
	int adr, oper;

	switch(adr=READ_OP()) {
	case 0:
		switch(oper=READ_OP()) {
		case 0: case 1: case 2: case 3:
			saturn_equals(reg_left[oper&3] , m_p, 1, reg_right[oper&3]);
			break;
		case 4: case 5: case 6: case 7:
			saturn_not_equals(reg_left[oper&3] ,m_p, 1, reg_right[oper&3]);
			break;
		case 8: case 9: case 0xa: case 0xb:
			saturn_equals_zero(A+(oper&3), m_p, 1);
			break;
		case 0xc: case 0xd: case 0xe: case 0xf:
			saturn_not_equals_zero(A+(oper&3), m_p, 1);
			break;
		}
		break;
	case 1:
		switch(oper=READ_OP()) {
		case 0: case 1: case 2: case 3:
			saturn_equals(reg_left[oper&3] , 0, m_p+1, reg_right[oper&3]);
			break;
		case 4: case 5: case 6: case 7:
			saturn_not_equals(reg_left[oper&3] , 0, m_p+1, reg_right[oper&3]);
			break;
		case 8: case 9: case 0xa: case 0xb:
			saturn_equals_zero(A+(oper&3), 0, m_p+1);
			break;
		case 0xc: case 0xd: case 0xe: case 0xf:
			saturn_not_equals_zero(A+(oper&3), 0, m_p+1);
			break;
		}
		break;
	case 2: case 3: case 4: case 5: case 6: case 7:
		switch(oper=READ_OP()) {
		case 0: case 1: case 2: case 3:
			saturn_equals(reg_left[oper&3] ,adr_a_begin[adr], adr_a_count[adr], reg_right[oper&3]);
			break;
		case 4: case 5: case 6: case 7:
			saturn_not_equals(reg_left[oper&3] ,adr_a_begin[adr], adr_a_count[adr], reg_right[oper&3]);
			break;
		case 8: case 9: case 0xa: case 0xb:
			saturn_equals_zero(A+(oper&3),adr_a_begin[adr], adr_a_count[adr]);
			break;
		case 0xc: case 0xd: case 0xe: case 0xf:
			saturn_not_equals_zero(A+(oper&3) ,adr_a_begin[adr], adr_a_count[adr]);
			break;
		}
		break;
	case 8:
		switch(oper=READ_OP()) {
		case 0: case 1: case 2: case 3:
			saturn_greater(reg_left[oper&3] ,m_p, 1, reg_right[oper&3]);
			break;
		case 4: case 5: case 6: case 7:
			saturn_smaller(reg_left[oper&3] ,m_p, 1, reg_right[oper&3]);
			break;
		case 8: case 9: case 0xa: case 0xb:
			saturn_greater_equals(reg_left[oper&3] ,m_p, 1, reg_right[oper&3]);
			break;
		case 0xc: case 0xd: case 0xe: case 0xf:
			saturn_smaller_equals(reg_left[oper&3] ,m_p, 1, reg_right[oper&3]);
			break;
		}
		break;
	case 9:
		switch(oper=READ_OP()) {
		case 0: case 1: case 2: case 3:
			saturn_greater(reg_left[oper&3] , 0, m_p+1, reg_right[oper&3]);
			break;
		case 4: case 5: case 6: case 7:
			saturn_smaller(reg_left[oper&3] , 0, m_p+1, reg_right[oper&3]);
			break;
		case 8: case 9: case 0xa: case 0xb:
			saturn_greater_equals(reg_left[oper&3], 0, m_p+1, reg_right[oper&3]);
			break;
		case 0xc: case 0xd: case 0xe: case 0xf:
			saturn_smaller_equals(reg_left[oper&3], 0, m_p+1, reg_right[oper&3]);
			break;
		}
		break;
	case 0xa: case 0xb: case 0xc: case 0xd: case 0xe: case 0xf:
		switch(oper=READ_OP()) {
		case 0: case 1: case 2: case 3:
			saturn_greater(reg_left[oper&3] ,adr_b_begin[adr], adr_b_count[adr], reg_right[oper&3]);
			break;
		case 4: case 5: case 6: case 7:
			saturn_smaller(reg_left[oper&3] ,adr_b_begin[adr], adr_b_count[adr], reg_right[oper&3]);
			break;
		case 8: case 9: case 0xa: case 0xb:
			saturn_greater_equals(reg_left[oper&3] ,adr_b_begin[adr], adr_b_count[adr], reg_right[oper&3]);
			break;
		case 0xc: case 0xd: case 0xe: case 0xf:
			saturn_smaller_equals(reg_left[oper&3] ,adr_b_begin[adr], adr_b_count[adr], reg_right[oper&3]);
			break;
		}
		break;
	}
}

void saturn_device::saturn_instruction_a()
{
	int reg, adr;

	switch(adr=READ_OP()) {
	case 0:
		switch (reg=READ_OP()) {
		case 0: case 1: case 2: case 3:
		case 8: case 9: case 0xa: case 0xb:
			saturn_add(add_left[reg], m_p, 1, add_right[reg]);
			break;
		case 4: case 5: case 6: case 7:
			saturn_add(A+(reg&3), m_p, 1, A+(reg&3));
			break;
		case 0xc: case 0xd: case 0xe: case 0xf:
			saturn_decrement(A+(reg&3), m_p, 1);
			break;
		}
		break;
	case 1:
		switch (reg=READ_OP()) {
		case 0: case 1: case 2: case 3:
		case 8: case 9: case 0xa: case 0xb:
			saturn_add(add_left[reg], 0, m_p+1, add_right[reg]);
			break;
		case 4: case 5: case 6: case 7:
			saturn_add(A+(reg&3), 0, m_p+1, A+(reg&3));
			break;
		case 0xc: case 0xd: case 0xe: case 0xf:
			saturn_decrement(A+(reg&3), 0, m_p+1);
			break;
		}
		break;
	case 2: case 3: case 4: case 5: case 6: case 7:
		switch (reg=READ_OP()) {
		case 0: case 1: case 2: case 3:
		case 8: case 9: case 0xa: case 0xb:
			saturn_add(add_left[reg], adr_a_begin[adr], adr_a_count[adr], add_right[reg]);
			break;
		case 4: case 5: case 6: case 7:
			saturn_add(A+(reg&3), adr_a_begin[adr], adr_a_count[adr], A+(reg&3));
			break;
		case 0xc: case 0xd: case 0xe: case 0xf:
			saturn_decrement(A+(reg&3), adr_a_begin[adr], adr_a_count[adr]);
			break;
		}
		break;
	case 8:
		switch(reg=READ_OP()) {
		case 0: case 1: case 2: case 3:
			saturn_clear(A+reg, m_p,1);
			break; // a=0 p
		case 4: case 5: case 6: case 7:
		case 8: case 9: case 0xa: case 0xb:
			saturn_copy(reg_right[reg&7], m_p,1,reg_left[reg&7]);
			break; // a=b p
		case 0xc: case 0xd: case 0xe: case 0xf:
			saturn_exchange(reg_left[reg&3], m_p,1,reg_right[reg&3]);
			break; // abex p
		}
		break;
	case 9:
		switch(reg=READ_OP()) {
		case 0: case 1: case 2: case 3:
			saturn_clear(A+reg,0,m_p+1);
			break; // a=0 wp
		case 4: case 5: case 6: case 7:
		case 8: case 9: case 0xa: case 0xb:
			saturn_copy(reg_right[reg&7], 0, m_p+1, reg_left[reg&7]);
			break; // a=b wp
		case 0xc: case 0xd: case 0xe: case 0xf:
			saturn_exchange(reg_left[reg&3], 0, m_p+1, reg_right[reg&3]);
			break; // abex wp
		}
		break;
	case 0xa: case 0xb: case 0xc: case 0xd: case 0xe: case 0xf:
		switch(reg=READ_OP()) {
		case 0: case 1: case 2: case 3:
			saturn_clear(A+reg, adr_b_begin[adr], adr_b_count[adr]);
			break; // a=0 xs
		case 4: case 5: case 6: case 7:
		case 8: case 9: case 0xa: case 0xb:
			saturn_copy(reg_right[reg&7], adr_b_begin[adr], adr_b_count[adr], reg_left[reg&7]);
			break; // a=b xs
		case 0xc: case 0xd: case 0xe: case 0xf:
			saturn_exchange(reg_left[reg&3], adr_b_begin[adr], adr_b_count[adr], reg_right[reg&3]);
			break; // abex xs
		}
		break;
	}
}

void saturn_device::saturn_instruction_b()
{
	int adr, reg;

	switch(adr=READ_OP()) {
	case 0:
		switch(reg=READ_OP()) {
		case 0: case 1: case 2: case 3:
		case 8: case 9: case 0xa: case 0xb:
			saturn_sub(sub_left[reg], m_p, 1, sub_right[reg]);
			break;
		case 4: case 5: case 6: case 7:
			saturn_increment(A+(reg&3), m_p, 1); break; // a=a+1 p
		case 0xc: case 0xd: case 0xe: case 0xf:
			saturn_sub2(sub_left[reg], m_p, 1, sub_right[reg]);
			break;
		}
		break;
	case 1:
		switch(reg=READ_OP()) {
		case 0: case 1: case 2: case 3:
		case 8: case 9: case 0xa: case 0xb:
			saturn_sub(sub_left[reg], 0, m_p+1, sub_right[reg]);
			break;
		case 4: case 5: case 6: case 7:
			saturn_increment(A+(reg&3), 0, m_p+1); break; // a=a+1 wp
		case 0xc: case 0xd: case 0xe: case 0xf:
			saturn_sub2(sub_left[reg], 0, m_p+1, sub_right[reg]);
			break;
		}
		break;
	case 2: case 3: case 4: case 5: case 6: case 7:
		switch(reg=READ_OP()) {
		case 0: case 1: case 2: case 3:
		case 8: case 9: case 0xa: case 0xb:
			saturn_sub(sub_left[reg], adr_a_begin[adr], adr_a_count[adr], sub_right[reg]);
			break;
		case 4: case 5: case 6: case 7:
			saturn_increment(A+(reg&3), adr_a_begin[adr], adr_a_count[adr]);
			break; // a=a+1 xs
		case 0xc: case 0xd: case 0xe: case 0xf:
			saturn_sub2(sub_left[reg], adr_a_begin[adr], adr_a_count[adr],
						sub_right[reg]);
			break;
		}
		break;
	case 8:
		switch(reg=READ_OP()) {
		case 0: case 1: case 2: case 3:
			saturn_shift_nibble_left(A+reg, m_p, 1); break; // asl p
		case 4: case 5: case 6: case 7:
			saturn_shift_nibble_right(A+(reg&3), m_p, 1); break; // asr p
		case 8: case 9: case 0xa: case 0xb:
			saturn_negate(A+(reg&3), m_p, 1); break; // A=-A p
		case 0xc: case 0xd: case 0xe: case 0xf:
			saturn_invert(A+(reg&3), m_p, 1); break; // A=-A-1 p
		}
		break;
	case 9:
		switch(reg=READ_OP()) {
		case 0: case 1: case 2: case 3:
			saturn_shift_nibble_left(A+reg,0,m_p+1); break; // asl wp
		case 4: case 5: case 6: case 7:
			saturn_shift_nibble_right(A+(reg&3),0,m_p+1); break; // asr wp
		case 8: case 9: case 0xa: case 0xb:
			saturn_negate(A+(reg&3),0,m_p+1); break; // A=-A wp
		case 0xc: case 0xd: case 0xe: case 0xf:
			saturn_invert(A+(reg&3),0,m_p+1); break; // A=-A-1 wp
		}
		break;
	case 0xa: case 0xb: case 0xc: case 0xd: case 0xe: case 0xf:
		switch(reg=READ_OP()) {
		case 0: case 1: case 2: case 3:
			saturn_shift_nibble_left(A+reg,adr_b_begin[adr], adr_b_count[adr]);
			break;
		case 4: case 5: case 6: case 7:
			saturn_shift_nibble_right(A+(reg&3), adr_b_begin[adr], adr_b_count[adr]);
			break;
		case 8: case 9: case 0xa: case 0xb:
			saturn_negate(A+(reg&3), adr_b_begin[adr], adr_b_count[adr]);
			break;
		case 0xc: case 0xd: case 0xe: case 0xf:
			saturn_invert(A+(reg&3), adr_b_begin[adr], adr_b_count[adr]);
			break;
		}
		break;
	}
}


void saturn_device::saturn_instruction()
{
	int reg, adr;

	switch(READ_OP()) {
	case 0:
		switch(READ_OP()) {
		case 0: saturn_return_xm_set();break;
		case 1: saturn_return(1);break;
		case 2: saturn_return_carry_set();break;
		case 3: saturn_return_carry_clear();break;
		case 4: saturn_sethex();break;
		case 5: saturn_setdec();break;
		case 6: saturn_push_c();break;
		case 7: saturn_pop_c();break;
		case 8: saturn_clear_st();break;
		case 9: saturn_st_to_c();break;
		case 0xa: saturn_c_to_st();break;
		case 0xb: saturn_exchange_c_st();break;
		case 0xc: saturn_inc_p();break;
		case 0xd: saturn_dec_p();break;
		case 0xe: saturn_instruction_0e();break;
		case 0xf: saturn_return_interrupt();break;
		}
		break;
	case 1:
		saturn_instruction_1();
		break;
	case 2:
		saturn_load_p();
		break;
	case 3:
		saturn_load_reg(C);
		break; // lc
	case 4:
		adr=READ_OP_DIS8();
		if (adr==0) {
			saturn_return(m_carry);
		}
		else {
			saturn_jump((m_pc+adr-2)&0xfffff, m_carry);
		}
		break;
	case 5:
		adr=READ_OP_DIS8();
		if (adr==0) {
			saturn_return(!m_carry);
		}
		else {
			saturn_jump((m_pc+adr-2)&0xfffff,!m_carry);
		}
		break;
	case 6:
		adr=READ_OP_DIS12();
		saturn_jump((m_pc+adr-3)&0xfffff,1); break;
	case 7:
		adr=READ_OP_DIS12();
		saturn_call((adr+m_pc)&0xfffff); break;
	case 8:
		saturn_instruction_8();
		break;
	case 9:
		saturn_instruction_9();
		break;
	case 0xa:
		saturn_instruction_a();
		break;
	case 0xb:
		saturn_instruction_b();
		break;
	case 0xc:
		switch (reg=READ_OP()) {
		case 0: case 1: case 2: case 3:
		case 8: case 9: case 0xa: case 0xb:
			saturn_add(add_left[reg], BEGIN_A, COUNT_A, add_right[reg]);
			break;
		case 4: case 5: case 6: case 7:
			saturn_add(A+(reg&3), BEGIN_A, COUNT_A, A+(reg&3));
			break;
		case 0xc: case 0xd: case 0xe: case 0xf:
			saturn_decrement(A+(reg&3), BEGIN_A, COUNT_A);
			break;
		}
		break;
	case 0xd:
		switch(reg=READ_OP()) {
		case 0: case 1: case 2: case 3:
			saturn_clear(A+reg, BEGIN_A, COUNT_A);
			break; // a=0 a
		case 4: case 5: case 6: case 7:
		case 8: case 9: case 0xa: case 0xb:
			saturn_copy(reg_right[reg&7], BEGIN_A, COUNT_A, reg_left[reg&7]);
			break; // a=b a
		case 0xc: case 0xd: case 0xe: case 0xf:
			saturn_exchange(reg_left[reg&3], BEGIN_A, COUNT_A, reg_right[reg&3]);
			break; // abex a
		}
		break;
	case 0xe:
		switch(reg=READ_OP()) {
		case 0: case 1: case 2: case 3:
		case 8: case 9: case 0xa: case 0xb:
			saturn_sub(sub_left[reg], BEGIN_A, COUNT_A, sub_right[reg]);
			break;
		case 4: case 5: case 6: case 7:
			saturn_increment(A+(reg&3), BEGIN_A, COUNT_A);
			break; // a=a+1 a
		case 0xc: case 0xd: case 0xe: case 0xf:
			saturn_sub2(sub_left[reg], BEGIN_A, COUNT_A, sub_right[reg]);
			break;
		}
		break;
	case 0xf:
		switch(reg=READ_OP()) {
		case 0: case 1: case 2: case 3:
			saturn_shift_nibble_left(A+reg,BEGIN_A, COUNT_A);
			break; // asl a
		case 4: case 5: case 6: case 7:
			saturn_shift_nibble_right(A+(reg&3),BEGIN_A, COUNT_A);
			break; // asr a
		case 8: case 9: case 0xa: case 0xb:
			saturn_negate(A+(reg&3),BEGIN_A, COUNT_A);
			break; // A=-A a
		case 0xc: case 0xd: case 0xe: case 0xf:
			saturn_invert(A+(reg&3),BEGIN_A, COUNT_A);
			break; // A=-A-1 a
		}
		break;
	}
}
