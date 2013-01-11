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

static void saturn_invalid3( saturn_state *cpustate, int op1, int op2, int op3 )
{
	logerror( "SATURN '%s' invalid opcode %x%x%x at %05x\n",
			cpustate->device->tag(), op1, op2, op3, cpustate->pc-3 );
}

static void saturn_invalid4( saturn_state *cpustate, int op1, int op2, int op3, int op4 )
{
	logerror( "SATURN '%s' invalid opcode %x%x%x%x at %05x\n",
			cpustate->device->tag(), op1, op2, op3, op4, cpustate->pc-4 );
}

static void saturn_invalid5( saturn_state *cpustate, int op1, int op2, int op3, int op4, int op5 )
{
	logerror( "SATURN '%s' invalid opcode %x%x%x%x%x at %05x\n",
			cpustate->device->tag(), op1, op2, op3, op4, op5, cpustate->pc-5 );
}

static void saturn_invalid6( saturn_state *cpustate, int op1, int op2, int op3, int op4, int op5, int op6 )
{
	logerror( "SATURN '%s' invalid opcode %x%x%x%x%x%x at %05x\n",
			cpustate->device->tag(), op1, op2, op3, op4, op5, op6, cpustate->pc-6 );
}


INLINE void saturn_instruction_0e(saturn_state *cpustate)
{
	int reg, adr;

	switch(adr=READ_OP(cpustate)) {
	case 0:
		switch(reg=READ_OP(cpustate)){
		case 0: case 1: case 2: case 3: case 4: case 5: case 6: case 7:
			saturn_and(cpustate, reg_left[reg], cpustate->p, 1, reg_right[reg]);
			break; //A=A&B p
		case 8: case 9: case 0xa: case 0xb: case 0xc: case 0xd: case 0xe: case 0xf:
			saturn_or(cpustate, reg_left[reg&7], cpustate->p, 1, reg_right[reg&7]);
			break; //A=A!B p
		}
		break;
	case 1:
		switch(reg=READ_OP(cpustate)){
		case 0: case 1: case 2: case 3:case 4: case 5: case 6: case 7:
			saturn_and(cpustate, reg_left[reg], 0, cpustate->p+1, reg_right[reg]);
			break; //A=A&B wp
		case 8: case 9: case 0xa: case 0xb: case 0xc: case 0xd: case 0xe: case 0xf:
			saturn_or(cpustate, reg_left[reg&7], 0, cpustate->p+1, reg_right[reg&7]);
			break; //A=A!B wp
		}
		break;
	case 2: case 3: case 4: case 5: case 6: case 7: case 0xf:
		switch(reg=READ_OP(cpustate)){
		case 0: case 1: case 2: case 3: case 4: case 5: case 6: case 7:
			saturn_and(cpustate, reg_left[reg], adr_af_begin[adr], adr_af_count[adr], reg_right[reg]);
			break; //A=A&B xs
		case 8: case 9: case 0xa: case 0xb: case 0xc: case 0xd: case 0xe: case 0xf:
			saturn_or(cpustate, reg_left[reg&7], adr_af_begin[adr], adr_af_count[adr], reg_right[reg&7]);
			break; //A=A!B xs
		}
		break;
	default:
		saturn_invalid3( cpustate, 0, 0xe, adr );
		break;
	}
}

static void saturn_instruction_1(saturn_state *cpustate)
{
	int reg, adr, oper;

	switch (adr=READ_OP(cpustate)) {
	case 0:
		switch (reg=READ_OP(cpustate)) {
		case 0: case 1: case 2: case 3: case 4:
			saturn_copy(cpustate, R0+reg, BEGIN_W, COUNT_W, A);
			break; // r0=a w
		case 8: case 9: case 0xa: case 0xb: case 0xc:
			saturn_copy(cpustate, R0+(reg&7), BEGIN_W, COUNT_W, C);
			break; // r0=c w
		default:
			saturn_invalid3( cpustate, 1, adr, reg );
			break;
		}
		break;
	case 1:
		switch (reg=READ_OP(cpustate)) {
		case 0: case 1: case 2: case 3: case 4:
			saturn_copy(cpustate, A, BEGIN_W, COUNT_W, R0+reg);
			break; // a=r0 w
		case 8: case 9: case 0xa: case 0xb: case 0xc:
			saturn_copy(cpustate, C, BEGIN_W, COUNT_W, R0+(reg&7));
			break; // c=r0 w
		default:
			saturn_invalid3( cpustate, 1, adr, reg );
			break;
		}
		break;
	case 2:
		switch (reg=READ_OP(cpustate)) {
		case 0: case 1: case 2: case 3: case 4:
			saturn_exchange(cpustate, A, BEGIN_W, COUNT_W, R0+reg);
			break; // ar0ex w
		case 8: case 9: case 0xa: case 0xb: case 0xc:
			saturn_exchange(cpustate, C, BEGIN_W, COUNT_W, R0+(reg&7));
			break; // cr0ex w
		default:
			saturn_invalid3( cpustate, 2, adr, reg );
			break;
		}
		break;
	case 3:
		switch (READ_OP(cpustate)) {
		case 0: saturn_reg_to_adr(cpustate, A,0);break;
		case 1: saturn_reg_to_adr(cpustate, A,1);break;
		case 2: saturn_exchange_adr_reg(cpustate, 0,A);break;
		case 3: saturn_exchange_adr_reg(cpustate, 1,A);break;
		case 4: saturn_reg_to_adr(cpustate, C,0);break;
		case 5: saturn_reg_to_adr(cpustate, C,1);break;
		case 6: saturn_exchange_adr_reg(cpustate, 0,C);break;
		case 7: saturn_exchange_adr_reg(cpustate, 1,C);break;
		case 8: saturn_reg_to_adr_word(cpustate, A,0);break;
		case 9: saturn_reg_to_adr_word(cpustate, A,1);break;
		case 0xa: saturn_exchange_adr_reg_word(cpustate, 0,A);break;
		case 0xb: saturn_exchange_adr_reg_word(cpustate, 1,A);break;
		case 0xc: saturn_reg_to_adr_word(cpustate, C,0);break;
		case 0xd: saturn_reg_to_adr_word(cpustate, C,1);break;
		case 0xe: saturn_exchange_adr_reg_word(cpustate, 0,C);break;
		case 0xf: saturn_exchange_adr_reg_word(cpustate, 1,C);break;
		}
		break;
	case 4:
		switch (READ_OP(cpustate)) {
		case 0: saturn_store_nibbles(cpustate, A, BEGIN_A, COUNT_A, 0); break;
		case 1: saturn_store_nibbles(cpustate, A, BEGIN_A, COUNT_A, 1); break;
		case 2: saturn_load_nibbles(cpustate, A, BEGIN_A, COUNT_A, 0); break;
		case 3: saturn_load_nibbles(cpustate, A, BEGIN_A, COUNT_A, 1); break;
		case 4: saturn_store_nibbles(cpustate, C, BEGIN_A, COUNT_A, 0); break;
		case 5: saturn_store_nibbles(cpustate, C, BEGIN_A, COUNT_A, 1); break;
		case 6: saturn_load_nibbles(cpustate, C, BEGIN_A, COUNT_A, 0); break;
		case 7: saturn_load_nibbles(cpustate, C, BEGIN_A, COUNT_A, 1); break;
		case 8: saturn_store_nibbles(cpustate, A, BEGIN_B, COUNT_B, 0); break;
		case 9: saturn_store_nibbles(cpustate, A, BEGIN_B, COUNT_B, 1); break;
		case 0xa: saturn_load_nibbles(cpustate, A, BEGIN_B, COUNT_B, 0); break;
		case 0xb: saturn_load_nibbles(cpustate, A, BEGIN_B, COUNT_B, 1); break;
		case 0xc: saturn_store_nibbles(cpustate, C, BEGIN_B, COUNT_B, 0); break;
		case 0xd: saturn_store_nibbles(cpustate, C, BEGIN_B, COUNT_B, 1); break;
		case 0xe: saturn_load_nibbles(cpustate, C, BEGIN_B, COUNT_B, 0); break;
		case 0xf: saturn_load_nibbles(cpustate, C, BEGIN_B, COUNT_B, 1); break;
		}
		break;
	case 5:
		switch (oper=READ_OP(cpustate)) {
		case 0: case 1: case 4: case 5:
			switch (adr=READ_OP(cpustate)) {
			case 0:
				saturn_store_nibbles(cpustate, oper&4?C:A,cpustate->p,1,oper&1);
				break;
			case 1:
				saturn_store_nibbles(cpustate, oper&4?C:A,0,cpustate->p+1,oper&1);
				break;
			case 2: case 3: case 4: case 5: case 6: case 7:
				saturn_store_nibbles(cpustate, oper&4?C:A,adr_a_begin[adr],adr_a_count[adr],oper&1);
				break;
			default:
				saturn_invalid4( cpustate, 1, 5, oper, adr );
				break;
			}
			break;
		case 2: case 3: case 6: case 7:
			switch (adr=READ_OP(cpustate)) {
			case 0:
				saturn_load_nibbles(cpustate, oper&4?C:A,cpustate->p,1,oper&1);
				break;
			case 1:
				saturn_load_nibbles(cpustate, oper&4?C:A,0,cpustate->p+1,oper&1);
				break;
			case 2: case 3: case 4: case 5: case 6: case 7:
				saturn_load_nibbles(cpustate, oper&4?C:A,adr_a_begin[adr],adr_a_count[adr],oper&1);
				break;
			default:
				saturn_invalid4( cpustate, 1, 5, oper, adr );
				break;
			}
			break;
		case 8: saturn_store_nibbles(cpustate, A, 0, READ_OP(cpustate)+1, 0); break;
		case 9: saturn_store_nibbles(cpustate, A, 0, READ_OP(cpustate)+1, 1); break;
		case 0xa: saturn_load_nibbles(cpustate, A, 0, READ_OP(cpustate)+1, 0); break;
		case 0xb: saturn_load_nibbles(cpustate, A, 0, READ_OP(cpustate)+1, 1); break;
		case 0xc: saturn_store_nibbles(cpustate, C, 0, READ_OP(cpustate)+1, 0); break;
		case 0xd: saturn_store_nibbles(cpustate, C, 0, READ_OP(cpustate)+1, 1); break;
		case 0xe: saturn_load_nibbles(cpustate, C, 0, READ_OP(cpustate)+1, 0); break;
		case 0xf: saturn_load_nibbles(cpustate, C, 0, READ_OP(cpustate)+1, 1); break;
		}
		break;
	case 6: saturn_add_adr(cpustate, 0);break;
	case 7: saturn_add_adr(cpustate, 1);break;
	case 8: saturn_sub_adr(cpustate, 0);break;
	case 9: saturn_load_adr(cpustate, 0,2);break;
	case 0xa: saturn_load_adr(cpustate, 0,4);break;
	case 0xb: saturn_load_adr(cpustate, 0,5);break;
	case 0xc: saturn_sub_adr(cpustate, 1);break;
	case 0xd: saturn_load_adr(cpustate, 1,2);break;
	case 0xe: saturn_load_adr(cpustate, 1,4);break;
	case 0xf: saturn_load_adr(cpustate, 1,5);break;
	}
}

static void saturn_instruction_80(saturn_state *cpustate)
{
	int op;
	switch(READ_OP(cpustate)) {
	case 0: saturn_out_cs(cpustate);break;
	case 1: saturn_out_c(cpustate);break;
	case 2: saturn_in(cpustate, A);break;
	case 3: saturn_in(cpustate, C);break;
	case 4: saturn_mem_unconfig(cpustate);break;
	case 5: saturn_mem_config(cpustate);break;
	case 6: saturn_mem_id(cpustate);break;
	case 7: saturn_shutdown(cpustate);break;
	case 8:
		switch(READ_OP(cpustate)) {
		case 0: saturn_interrupt_on(cpustate);break;
		case 1:
			switch(op=READ_OP(cpustate)) {
			case 0: saturn_reset_interrupt(cpustate);break;
			default: saturn_invalid5( cpustate, 8, 0, 8, 1, op ); break;
			}
			break;
		case 2: saturn_load_reg(cpustate, A);break; //la
		case 3: saturn_bus_command_b(cpustate);break;
		case 4: saturn_clear_bit(cpustate, A);break; // abit=0
		case 5: saturn_set_bit(cpustate, A);break; // abit=1
		case 6: saturn_jump_bit_clear(cpustate, A);break;
		case 7: saturn_jump_bit_set(cpustate, A);break;
		case 8: saturn_clear_bit(cpustate, C);break; // cbit=0
		case 9: saturn_set_bit(cpustate, C);break; // cbit=1
		case 0xa: saturn_jump_bit_clear(cpustate, C);break;
		case 0xb: saturn_jump_bit_set(cpustate, C);break;
		case 0xc: saturn_indirect_jump(cpustate, A);break;
		case 0xd: saturn_bus_command_d(cpustate);break;
		case 0xe: saturn_indirect_jump(cpustate, C);break;
		case 0xf: saturn_interrupt_off(cpustate);break;
		}
		break;
	case 9: saturn_ca_p_1(cpustate);break;//C+P+1
	case 0xa: saturn_mem_reset(cpustate);break;
	case 0xb: saturn_bus_command_b(cpustate);break;
	case 0xc: saturn_p_to_c(cpustate);break;
	case 0xd: saturn_c_to_p(cpustate);break;
	case 0xe: saturn_serial_request(cpustate);break;
	case 0xf: saturn_exchange_p(cpustate);break;
	}
}

static void saturn_instruction_81a(saturn_state *cpustate)
{
	int reg, adr,op;
	switch(adr=READ_OP(cpustate)) {
	case 0:
		switch(op=READ_OP(cpustate)) {
		case 0:
			switch(reg=READ_OP(cpustate)) {
			case 0: case 1: case 2: case 3: case 4:
				saturn_copy(cpustate, R0+reg,cpustate->p,1,A);
				break; //r0=a p
			case 8: case 9: case 0xa: case 0xb: case 0xc:
				saturn_copy(cpustate, R0+(reg&7),cpustate->p,1,C);
				break; //r0=c p
			default:
				saturn_invalid6( cpustate, 8, 1, 0xa, adr, op, reg);
				break;
			}
			break;
		case 1:
			switch(reg=READ_OP(cpustate)) {
			case 0: case 1: case 2: case 3: case 4:
				saturn_copy(cpustate, A,cpustate->p,1,R0+reg);
				break; //a=r0 p
			case 8: case 9: case 0xa: case 0xb: case 0xc:
				saturn_copy(cpustate, C,cpustate->p,1,R0+(reg&7));
				break; //c=r0 p
			default:
				saturn_invalid6( cpustate, 8, 1, 0xa, adr, op, reg);
				break;
			}
			break;
		case 2:
			switch (reg=READ_OP(cpustate)) {
			case 0: case 1: case 2: case 3: case 4:
				saturn_exchange(cpustate, A, cpustate->p,1,R0+reg);
				break; // ar0ex p
			case 8: case 9: case 0xa: case 0xb: case 0xc:
				saturn_exchange(cpustate, C, cpustate->p,1,R0+(reg&7));
				break; // cr0ex p
			default:
				saturn_invalid6( cpustate, 8, 1, 0xa, adr, op, reg);
				break;
			}
			break;
		default:
			saturn_invalid5( cpustate, 8, 1, 0xa, adr, op );
			break;
		}
		break;
	case 1:
		switch(op=READ_OP(cpustate)) {
		case 0:
			switch(reg=READ_OP(cpustate)) {
			case 0: case 1: case 2: case 3: case 4:
				saturn_copy(cpustate, R0+reg,0,cpustate->p+1,A);
				break; //r0=a wp
			case 8: case 9: case 0xa: case 0xb: case 0xc:
				saturn_copy(cpustate, R0+(reg&7),0,cpustate->p+1,C);
				break; //r0=c wp
			default:
				saturn_invalid6( cpustate, 8, 1, 0xa, adr, op, reg);
				break;
			}
			break;
		case 1:
			switch(reg=READ_OP(cpustate)) {
			case 0: case 1: case 2: case 3: case 4:
				saturn_copy(cpustate, A,0,cpustate->p+1,R0+reg);
				break; //a=r0 wp
			case 8: case 9: case 0xa: case 0xb: case 0xc:
				saturn_copy(cpustate, C,0,cpustate->p+1,R0+(reg&7));
				break; //c=r0 wp
			default:
				saturn_invalid6( cpustate, 8, 1, 0xa, adr, op, reg);
				break;
			}
			break;
		case 2:
			switch (reg=READ_OP(cpustate)) {
			case 0: case 1: case 2: case 3: case 4:
				saturn_exchange(cpustate, A, 0, cpustate->p+1, R0+reg);
				break; // ar0ex wp
			case 8: case 9: case 0xa: case 0xb: case 0xc:
				saturn_exchange(cpustate, C, 0, cpustate->p+1, R0+(reg&7));
				break; // cr0ex wp
			default:
				saturn_invalid6( cpustate, 8, 1, 0xa, adr, op, reg);
				break;
			}
			break;
		default:
			saturn_invalid5( cpustate, 8, 1, 0xa, adr, op );
			break;
		}
		break;
	case 2: case 3: case 4: case 5: case 6: case 7: case 0xf:
		switch(op=READ_OP(cpustate)) {
		case 0:
			switch(reg=READ_OP(cpustate)) {
			case 0: case 1: case 2: case 3: case 4:
				saturn_copy(cpustate, R0+reg,adr_af_begin[adr],adr_af_count[adr],A);
				break; //r0=a xs
			case 8: case 9: case 0xa: case 0xb: case 0xc:
				saturn_copy(cpustate, R0+(reg&7),adr_af_begin[adr], adr_af_count[adr],C);
				break; //r0=c xs
			default:
				saturn_invalid6( cpustate, 8, 1, 0xa, adr, op, reg);
				break;
			}
			break;
		case 1:
			switch(reg=READ_OP(cpustate)) {
			case 0: case 1: case 2: case 3: case 4:
				saturn_copy(cpustate, A,adr_af_begin[adr],adr_af_count[adr],R0+reg);
				break; //a=r0 xs
			case 8: case 9: case 0xa: case 0xb: case 0xc:
				saturn_copy(cpustate, C,adr_af_begin[adr],adr_af_count[adr],R0+(reg&7));
				break; //c=r0 xs
			default:
				saturn_invalid6( cpustate, 8, 1, 0xa, adr, op, reg);
				break;
			}
			break;
		case 2:
			switch (reg=READ_OP(cpustate)) {
			case 0: case 1: case 2: case 3: case 4:
				saturn_exchange(cpustate, A, adr_af_begin[adr], adr_af_count[adr], R0+reg);
				break; // ar0ex xs
			case 8: case 9: case 0xa: case 0xb: case 0xc:
				saturn_exchange(cpustate, C, adr_af_begin[adr], adr_af_count[adr], R0+(reg&7));
				break; // cr0ex xs
			default:
				saturn_invalid6( cpustate, 8, 1, 0xa, adr, op, reg);
				break;
			}
			break;
		default:
			saturn_invalid5( cpustate, 8, 1, 0xa, adr, op );
			break;
		}
		break;
	default:
		saturn_invalid4( cpustate, 8, 1, 0xa, adr );
		break;
	}
}

static void saturn_instruction_81(saturn_state *cpustate)
{
	int reg, adr;

	switch(reg=READ_OP(cpustate)) {
	case 0: case 1: case 2: case 3:
		saturn_rotate_nibble_left_w(cpustate, A+reg); break; // aslc w
	case 4: case 5: case 6: case 7:
		saturn_rotate_nibble_right_w(cpustate, A+(reg&3)); break; // asrc w
	case 8:
		switch(adr=READ_OP(cpustate)) {
		case 0:
			switch (reg=READ_OP(cpustate)) {
			case 0: case 1: case 2: case 3:
				saturn_add_const(cpustate, A+reg, cpustate->p, 1, READ_OP(cpustate)+1);
				break;
			case 8: case 9: case 0xa: case 0xb:
				saturn_sub_const(cpustate, A+(reg&3), cpustate->p, 1, READ_OP(cpustate)+1);
				break;
			default:
				saturn_invalid5( cpustate, 8, 1, 8, adr, reg );
				break;
			}
			break;
		case 1:
			switch (reg=READ_OP(cpustate)) {
			case 0: case 1: case 2: case 3:
				saturn_add_const(cpustate, A+reg, 0, cpustate->p+1, READ_OP(cpustate)+1);
				break;
			case 8: case 9: case 0xa: case 0xb:
				saturn_sub_const(cpustate, A+(reg&3), 0, cpustate->p+1, READ_OP(cpustate)+1);
				break;
			default:
				saturn_invalid5( cpustate, 8, 1, 8, adr, reg );
				break;
			}
			break;
		case 2: case 3: case 4: case 5: case 6: case 7: case 0xf:
			switch (reg=READ_OP(cpustate)) {
			case 0: case 1: case 2: case 3:
				saturn_add_const(cpustate, A+reg, adr_af_begin[adr], adr_af_count[adr], READ_OP(cpustate)+1);
				break;
			case 8: case 9: case 0xa: case 0xb:
				saturn_sub_const(cpustate, A+(reg&3), adr_af_begin[adr], adr_af_count[adr], READ_OP(cpustate)+1);
				break;
			default:
				saturn_invalid5( cpustate, 8, 1, 8, adr, reg );
				break;
			}
			break;
		default:
			saturn_invalid4( cpustate, 8, 1, 8, adr );
			break;
		}
		break;
	case 9:
		switch(adr=READ_OP(cpustate)) {
		case 0:
			switch(reg=READ_OP(cpustate)){
			case 0: case 1: case 2: case 3:
				saturn_shift_right(cpustate, A+reg,cpustate->p,1);
				break; // asrb p
			default:
				saturn_invalid5( cpustate, 8, 1, 9, adr, reg );
				break;
			}
			break;
		case 1:
			switch(reg=READ_OP(cpustate)){
			case 0: case 1: case 2: case 3:
				saturn_shift_right(cpustate, A+reg, 0,cpustate->p+1);
				break; // asrb wp
			default:
				saturn_invalid5( cpustate, 8, 1, 9, adr, reg );
				break;
			}
			break;
		case 2: case 3: case 4: case 5: case 6: case 7: case 0xf:
			switch(reg=READ_OP(cpustate)){
			case 0: case 1: case 2: case 3:
				saturn_shift_right(cpustate, A+reg, adr_af_begin[adr], adr_af_count[adr]);
				break; // asrb xs
			default:
				saturn_invalid5( cpustate, 8, 1, 9, adr, reg );
				break;
			}
			break;
		default:
			saturn_invalid4( cpustate, 8, 1, 9, adr );
			break;
		}
		break;
	case 0xa:
		saturn_instruction_81a(cpustate);
		break;
	case 0xb:
		switch(adr=READ_OP(cpustate)) {
		case 2: saturn_load_pc(cpustate, A);break;
		case 3: saturn_load_pc(cpustate, C);break;
		case 4: saturn_store_pc(cpustate, A);break;
		case 5: saturn_store_pc(cpustate, C);break;
		case 6: saturn_exchange_pc(cpustate, A);break;
		case 7: saturn_exchange_pc(cpustate, C);break;
		default: saturn_invalid4( cpustate, 8, 1, reg, adr ); break;
		}
		break;
	case 0xc: case 0xd: case 0xe: case 0xf:
		saturn_shift_right(cpustate, A+(reg&3), BEGIN_W, COUNT_W);
		break; // asrb w
	}
}

static void saturn_instruction_8(saturn_state *cpustate)
{
	int oper, adr;

	switch(READ_OP(cpustate)) {
	case 0:
		saturn_instruction_80(cpustate);
		break;
	case 1:
		saturn_instruction_81(cpustate);
		break;
	case 2: saturn_hst_clear_bits(cpustate);break;
	case 3: saturn_hst_bits_cleared(cpustate);break;
	case 4: saturn_st_clear_bit(cpustate);break;
	case 5: saturn_st_set_bit(cpustate);break;
	case 6: saturn_st_jump_bit_clear(cpustate);break;
	case 7: saturn_st_jump_bit_set(cpustate);break;
	case 8: saturn_p_not_equals(cpustate); break;
	case 9: saturn_p_equals(cpustate); break;
	case 0xa:
		switch(oper=READ_OP(cpustate)) {
		case 0: case 1: case 2: case 3:
			saturn_equals(cpustate, reg_left[oper&3] , BEGIN_A, COUNT_A, reg_right[oper&3]);
			break;
		case 4: case 5: case 6: case 7:
			saturn_not_equals(cpustate, reg_left[oper&3] , BEGIN_A, COUNT_A, reg_right[oper&3]);
			break;
		case 8: case 9: case 0xa: case 0xb:
			saturn_equals_zero(cpustate, A+(oper&3), BEGIN_A, COUNT_A);
			break;
		case 0xc: case 0xd: case 0xe: case 0xf:
			saturn_not_equals_zero(cpustate, A+(oper&3), BEGIN_A, COUNT_A);
			break;
		}
		break;
	case 0xb:
		switch(oper=READ_OP(cpustate)) {
		case 0: case 1: case 2: case 3:
			saturn_greater(cpustate, reg_left[oper&3] , BEGIN_A, COUNT_A, reg_right[oper&3]);
			break;
		case 4: case 5: case 6: case 7:
			saturn_smaller(cpustate, reg_left[oper&3] , BEGIN_A, COUNT_A, reg_right[oper&3]);
			break;
		case 8: case 9: case 0xa: case 0xb:
			saturn_greater_equals(cpustate, reg_left[oper&3], BEGIN_A, COUNT_A, reg_right[oper&3]);
			break;
		case 0xc: case 0xd: case 0xe: case 0xf:
			saturn_smaller_equals(cpustate, reg_left[oper&3], BEGIN_A, COUNT_A, reg_right[oper&3]);
			break;
		}
		break;
	case 0xc:
		adr=READ_OP_DIS16(cpustate);
		saturn_jump(cpustate, (adr+cpustate->pc-4)&0xfffff,1);
		break;
	case 0xd:
		adr=READ_OP_ARG20(cpustate);
		saturn_jump(cpustate, adr,1);
		break;
	case 0xe:
		adr=READ_OP_DIS16(cpustate);
		saturn_call(cpustate, (adr+cpustate->pc)&0xfffff);
		break;
	case 0xf:
		adr=READ_OP_ARG20(cpustate);
		saturn_call(cpustate, adr);
		break;
	}
}

static void saturn_instruction_9(saturn_state *cpustate)
{
	int adr, oper;

	switch(adr=READ_OP(cpustate)) {
	case 0:
		switch(oper=READ_OP(cpustate)) {
		case 0: case 1: case 2: case 3:
			saturn_equals(cpustate, reg_left[oper&3] , cpustate->p, 1, reg_right[oper&3]);
			break;
		case 4: case 5: case 6: case 7:
			saturn_not_equals(cpustate, reg_left[oper&3] ,cpustate->p, 1, reg_right[oper&3]);
			break;
		case 8: case 9: case 0xa: case 0xb:
			saturn_equals_zero(cpustate, A+(oper&3), cpustate->p, 1);
			break;
		case 0xc: case 0xd: case 0xe: case 0xf:
			saturn_not_equals_zero(cpustate, A+(oper&3), cpustate->p, 1);
			break;
		}
		break;
	case 1:
		switch(oper=READ_OP(cpustate)) {
		case 0: case 1: case 2: case 3:
			saturn_equals(cpustate, reg_left[oper&3] , 0, cpustate->p+1, reg_right[oper&3]);
			break;
		case 4: case 5: case 6: case 7:
			saturn_not_equals(cpustate, reg_left[oper&3] , 0, cpustate->p+1, reg_right[oper&3]);
			break;
		case 8: case 9: case 0xa: case 0xb:
			saturn_equals_zero(cpustate, A+(oper&3), 0, cpustate->p+1);
			break;
		case 0xc: case 0xd: case 0xe: case 0xf:
			saturn_not_equals_zero(cpustate, A+(oper&3), 0, cpustate->p+1);
			break;
		}
		break;
	case 2: case 3: case 4: case 5: case 6: case 7:
		switch(oper=READ_OP(cpustate)) {
		case 0: case 1: case 2: case 3:
			saturn_equals(cpustate, reg_left[oper&3] ,adr_a_begin[adr], adr_a_count[adr], reg_right[oper&3]);
			break;
		case 4: case 5: case 6: case 7:
			saturn_not_equals(cpustate, reg_left[oper&3] ,adr_a_begin[adr], adr_a_count[adr], reg_right[oper&3]);
			break;
		case 8: case 9: case 0xa: case 0xb:
			saturn_equals_zero(cpustate, A+(oper&3),adr_a_begin[adr], adr_a_count[adr]);
			break;
		case 0xc: case 0xd: case 0xe: case 0xf:
			saturn_not_equals_zero(cpustate, A+(oper&3) ,adr_a_begin[adr], adr_a_count[adr]);
			break;
		}
		break;
	case 8:
		switch(oper=READ_OP(cpustate)) {
		case 0: case 1: case 2: case 3:
			saturn_greater(cpustate, reg_left[oper&3] ,cpustate->p, 1, reg_right[oper&3]);
			break;
		case 4: case 5: case 6: case 7:
			saturn_smaller(cpustate, reg_left[oper&3] ,cpustate->p, 1, reg_right[oper&3]);
			break;
		case 8: case 9: case 0xa: case 0xb:
			saturn_greater_equals(cpustate, reg_left[oper&3] ,cpustate->p, 1, reg_right[oper&3]);
			break;
		case 0xc: case 0xd: case 0xe: case 0xf:
			saturn_smaller_equals(cpustate, reg_left[oper&3] ,cpustate->p, 1, reg_right[oper&3]);
			break;
		}
		break;
	case 9:
		switch(oper=READ_OP(cpustate)) {
		case 0: case 1: case 2: case 3:
			saturn_greater(cpustate, reg_left[oper&3] , 0, cpustate->p+1, reg_right[oper&3]);
			break;
		case 4: case 5: case 6: case 7:
			saturn_smaller(cpustate, reg_left[oper&3] , 0, cpustate->p+1, reg_right[oper&3]);
			break;
		case 8: case 9: case 0xa: case 0xb:
			saturn_greater_equals(cpustate, reg_left[oper&3], 0, cpustate->p+1, reg_right[oper&3]);
			break;
		case 0xc: case 0xd: case 0xe: case 0xf:
			saturn_smaller_equals(cpustate, reg_left[oper&3], 0, cpustate->p+1, reg_right[oper&3]);
			break;
		}
		break;
	case 0xa: case 0xb: case 0xc: case 0xd: case 0xe: case 0xf:
		switch(oper=READ_OP(cpustate)) {
		case 0: case 1: case 2: case 3:
			saturn_greater(cpustate, reg_left[oper&3] ,adr_b_begin[adr], adr_b_count[adr], reg_right[oper&3]);
			break;
		case 4: case 5: case 6: case 7:
			saturn_smaller(cpustate, reg_left[oper&3] ,adr_b_begin[adr], adr_b_count[adr], reg_right[oper&3]);
			break;
		case 8: case 9: case 0xa: case 0xb:
			saturn_greater_equals(cpustate, reg_left[oper&3] ,adr_b_begin[adr], adr_b_count[adr], reg_right[oper&3]);
			break;
		case 0xc: case 0xd: case 0xe: case 0xf:
			saturn_smaller_equals(cpustate, reg_left[oper&3] ,adr_b_begin[adr], adr_b_count[adr], reg_right[oper&3]);
			break;
		}
		break;
	}
}

static void saturn_instruction_a(saturn_state *cpustate)
{
	int reg, adr;

	switch(adr=READ_OP(cpustate)) {
	case 0:
		switch (reg=READ_OP(cpustate)) {
		case 0: case 1: case 2: case 3:
		case 8: case 9: case 0xa: case 0xb:
			saturn_add(cpustate, add_left[reg], cpustate->p, 1, add_right[reg]);
			break;
		case 4: case 5: case 6: case 7:
			saturn_add(cpustate, A+(reg&3), cpustate->p, 1, A+(reg&3));
			break;
		case 0xc: case 0xd: case 0xe: case 0xf:
			saturn_decrement(cpustate, A+(reg&3), cpustate->p, 1);
			break;
		}
		break;
	case 1:
		switch (reg=READ_OP(cpustate)) {
		case 0: case 1: case 2: case 3:
		case 8: case 9: case 0xa: case 0xb:
			saturn_add(cpustate, add_left[reg], 0, cpustate->p+1, add_right[reg]);
			break;
		case 4: case 5: case 6: case 7:
			saturn_add(cpustate, A+(reg&3), 0, cpustate->p+1, A+(reg&3));
			break;
		case 0xc: case 0xd: case 0xe: case 0xf:
			saturn_decrement(cpustate, A+(reg&3), 0, cpustate->p+1);
			break;
		}
		break;
	case 2: case 3: case 4: case 5: case 6: case 7:
		switch (reg=READ_OP(cpustate)) {
		case 0: case 1: case 2: case 3:
		case 8: case 9: case 0xa: case 0xb:
			saturn_add(cpustate, add_left[reg], adr_a_begin[adr], adr_a_count[adr], add_right[reg]);
			break;
		case 4: case 5: case 6: case 7:
			saturn_add(cpustate, A+(reg&3), adr_a_begin[adr], adr_a_count[adr], A+(reg&3));
			break;
		case 0xc: case 0xd: case 0xe: case 0xf:
			saturn_decrement(cpustate, A+(reg&3), adr_a_begin[adr], adr_a_count[adr]);
			break;
		}
		break;
	case 8:
		switch(reg=READ_OP(cpustate)) {
		case 0: case 1: case 2: case 3:
			saturn_clear(cpustate, A+reg, cpustate->p,1);
			break; // a=0 p
		case 4: case 5: case 6: case 7:
		case 8: case 9: case 0xa: case 0xb:
			saturn_copy(cpustate, reg_right[reg&7], cpustate->p,1,reg_left[reg&7]);
			break; // a=b p
		case 0xc: case 0xd: case 0xe: case 0xf:
			saturn_exchange(cpustate, reg_left[reg&3], cpustate->p,1,reg_right[reg&3]);
			break; // abex p
		}
		break;
	case 9:
		switch(reg=READ_OP(cpustate)) {
		case 0: case 1: case 2: case 3:
			saturn_clear(cpustate, A+reg,0,cpustate->p+1);
			break; // a=0 wp
		case 4: case 5: case 6: case 7:
		case 8: case 9: case 0xa: case 0xb:
			saturn_copy(cpustate, reg_right[reg&7], 0, cpustate->p+1, reg_left[reg&7]);
			break; // a=b wp
		case 0xc: case 0xd: case 0xe: case 0xf:
			saturn_exchange(cpustate, reg_left[reg&3], 0, cpustate->p+1, reg_right[reg&3]);
			break; // abex wp
		}
		break;
	case 0xa: case 0xb: case 0xc: case 0xd: case 0xe: case 0xf:
		switch(reg=READ_OP(cpustate)) {
		case 0: case 1: case 2: case 3:
			saturn_clear(cpustate, A+reg, adr_b_begin[adr], adr_b_count[adr]);
			break; // a=0 xs
		case 4: case 5: case 6: case 7:
		case 8: case 9: case 0xa: case 0xb:
			saturn_copy(cpustate, reg_right[reg&7], adr_b_begin[adr], adr_b_count[adr], reg_left[reg&7]);
			break; // a=b xs
		case 0xc: case 0xd: case 0xe: case 0xf:
			saturn_exchange(cpustate, reg_left[reg&3], adr_b_begin[adr], adr_b_count[adr], reg_right[reg&3]);
			break; // abex xs
		}
		break;
	}
}

static void saturn_instruction_b(saturn_state *cpustate)
{
	int adr, reg;

	switch(adr=READ_OP(cpustate)) {
	case 0:
		switch(reg=READ_OP(cpustate)) {
		case 0: case 1: case 2: case 3:
		case 8: case 9: case 0xa: case 0xb:
			saturn_sub(cpustate, sub_left[reg], cpustate->p, 1, sub_right[reg]);
			break;
		case 4: case 5: case 6: case 7:
			saturn_increment(cpustate, A+(reg&3), cpustate->p, 1); break; // a=a+1 p
		case 0xc: case 0xd: case 0xe: case 0xf:
			saturn_sub2(cpustate, sub_left[reg], cpustate->p, 1, sub_right[reg]);
			break;
		}
		break;
	case 1:
		switch(reg=READ_OP(cpustate)) {
		case 0: case 1: case 2: case 3:
		case 8: case 9: case 0xa: case 0xb:
			saturn_sub(cpustate, sub_left[reg], 0, cpustate->p+1, sub_right[reg]);
			break;
		case 4: case 5: case 6: case 7:
			saturn_increment(cpustate, A+(reg&3), 0, cpustate->p+1); break; // a=a+1 wp
		case 0xc: case 0xd: case 0xe: case 0xf:
			saturn_sub2(cpustate, sub_left[reg], 0, cpustate->p+1, sub_right[reg]);
			break;
		}
		break;
	case 2: case 3: case 4: case 5: case 6: case 7:
		switch(reg=READ_OP(cpustate)) {
		case 0: case 1: case 2: case 3:
		case 8: case 9: case 0xa: case 0xb:
			saturn_sub(cpustate, sub_left[reg], adr_a_begin[adr], adr_a_count[adr], sub_right[reg]);
			break;
		case 4: case 5: case 6: case 7:
			saturn_increment(cpustate, A+(reg&3), adr_a_begin[adr], adr_a_count[adr]);
			break; // a=a+1 xs
		case 0xc: case 0xd: case 0xe: case 0xf:
			saturn_sub2(cpustate, sub_left[reg], adr_a_begin[adr], adr_a_count[adr],
						sub_right[reg]);
			break;
		}
		break;
	case 8:
		switch(reg=READ_OP(cpustate)) {
		case 0: case 1: case 2: case 3:
			saturn_shift_nibble_left(cpustate, A+reg, cpustate->p, 1); break; // asl p
		case 4: case 5: case 6: case 7:
			saturn_shift_nibble_right(cpustate, A+(reg&3), cpustate->p, 1); break; // asr p
		case 8: case 9: case 0xa: case 0xb:
			saturn_negate(cpustate, A+(reg&3), cpustate->p, 1); break; // A=-A p
		case 0xc: case 0xd: case 0xe: case 0xf:
			saturn_invert(cpustate, A+(reg&3), cpustate->p, 1); break; // A=-A-1 p
		}
		break;
	case 9:
		switch(reg=READ_OP(cpustate)) {
		case 0: case 1: case 2: case 3:
			saturn_shift_nibble_left(cpustate, A+reg,0,cpustate->p+1); break; // asl wp
		case 4: case 5: case 6: case 7:
			saturn_shift_nibble_right(cpustate, A+(reg&3),0,cpustate->p+1); break; // asr wp
		case 8: case 9: case 0xa: case 0xb:
			saturn_negate(cpustate, A+(reg&3),0,cpustate->p+1); break; // A=-A wp
		case 0xc: case 0xd: case 0xe: case 0xf:
			saturn_invert(cpustate, A+(reg&3),0,cpustate->p+1); break; // A=-A-1 wp
		}
		break;
	case 0xa: case 0xb: case 0xc: case 0xd: case 0xe: case 0xf:
		switch(reg=READ_OP(cpustate)) {
		case 0: case 1: case 2: case 3:
			saturn_shift_nibble_left(cpustate, A+reg,adr_b_begin[adr], adr_b_count[adr]);
			break;
		case 4: case 5: case 6: case 7:
			saturn_shift_nibble_right(cpustate, A+(reg&3), adr_b_begin[adr], adr_b_count[adr]);
			break;
		case 8: case 9: case 0xa: case 0xb:
			saturn_negate(cpustate, A+(reg&3), adr_b_begin[adr], adr_b_count[adr]);
			break;
		case 0xc: case 0xd: case 0xe: case 0xf:
			saturn_invert(cpustate, A+(reg&3), adr_b_begin[adr], adr_b_count[adr]);
			break;
		}
		break;
	}
}


static void saturn_instruction(saturn_state *cpustate)
{
	int reg, adr;

	switch(READ_OP(cpustate)) {
	case 0:
		switch(READ_OP(cpustate)) {
		case 0: saturn_return_xm_set(cpustate);break;
		case 1: saturn_return(cpustate, 1);break;
		case 2: saturn_return_carry_set(cpustate);break;
		case 3: saturn_return_carry_clear(cpustate);break;
		case 4: saturn_sethex(cpustate);break;
		case 5: saturn_setdec(cpustate);break;
		case 6: saturn_push_c(cpustate);break;
		case 7: saturn_pop_c(cpustate);break;
		case 8: saturn_clear_st(cpustate);break;
		case 9: saturn_st_to_c(cpustate);break;
		case 0xa: saturn_c_to_st(cpustate);break;
		case 0xb: saturn_exchange_c_st(cpustate);break;
		case 0xc: saturn_inc_p(cpustate);break;
		case 0xd: saturn_dec_p(cpustate);break;
		case 0xe: saturn_instruction_0e(cpustate);break;
		case 0xf: saturn_return_interrupt(cpustate);break;
		}
		break;
	case 1:
		saturn_instruction_1(cpustate);
		break;
	case 2:
		saturn_load_p(cpustate);
		break;
	case 3:
		saturn_load_reg(cpustate, C);
		break; // lc
	case 4:
		adr=READ_OP_DIS8(cpustate);
		if (adr==0) {
			saturn_return(cpustate, cpustate->carry);
		}
		else {
			saturn_jump(cpustate, (cpustate->pc+adr-2)&0xfffff, cpustate->carry);
		}
		break;
	case 5:
		adr=READ_OP_DIS8(cpustate);
		if (adr==0) {
			saturn_return(cpustate, !cpustate->carry);
		}
		else {
			saturn_jump(cpustate, (cpustate->pc+adr-2)&0xfffff,!cpustate->carry);
		}
		break;
	case 6:
		adr=READ_OP_DIS12(cpustate);
		saturn_jump(cpustate, (cpustate->pc+adr-3)&0xfffff,1); break;
	case 7:
		adr=READ_OP_DIS12(cpustate);
		saturn_call(cpustate, (adr+cpustate->pc)&0xfffff); break;
	case 8:
		saturn_instruction_8(cpustate);
		break;
	case 9:
		saturn_instruction_9(cpustate);
		break;
	case 0xa:
		saturn_instruction_a(cpustate);
		break;
	case 0xb:
		saturn_instruction_b(cpustate);
		break;
	case 0xc:
		switch (reg=READ_OP(cpustate)) {
		case 0: case 1: case 2: case 3:
		case 8: case 9: case 0xa: case 0xb:
			saturn_add(cpustate, add_left[reg], BEGIN_A, COUNT_A, add_right[reg]);
			break;
		case 4: case 5: case 6: case 7:
			saturn_add(cpustate, A+(reg&3), BEGIN_A, COUNT_A, A+(reg&3));
			break;
		case 0xc: case 0xd: case 0xe: case 0xf:
			saturn_decrement(cpustate, A+(reg&3), BEGIN_A, COUNT_A);
			break;
		}
		break;
	case 0xd:
		switch(reg=READ_OP(cpustate)) {
		case 0: case 1: case 2: case 3:
			saturn_clear(cpustate, A+reg, BEGIN_A, COUNT_A);
			break; // a=0 a
		case 4: case 5: case 6: case 7:
		case 8: case 9: case 0xa: case 0xb:
			saturn_copy(cpustate, reg_right[reg&7], BEGIN_A, COUNT_A, reg_left[reg&7]);
			break; // a=b a
		case 0xc: case 0xd: case 0xe: case 0xf:
			saturn_exchange(cpustate, reg_left[reg&3], BEGIN_A, COUNT_A, reg_right[reg&3]);
			break; // abex a
		}
		break;
	case 0xe:
		switch(reg=READ_OP(cpustate)) {
		case 0: case 1: case 2: case 3:
		case 8: case 9: case 0xa: case 0xb:
			saturn_sub(cpustate, sub_left[reg], BEGIN_A, COUNT_A, sub_right[reg]);
			break;
		case 4: case 5: case 6: case 7:
			saturn_increment(cpustate, A+(reg&3), BEGIN_A, COUNT_A);
			break; // a=a+1 a
		case 0xc: case 0xd: case 0xe: case 0xf:
			saturn_sub2(cpustate, sub_left[reg], BEGIN_A, COUNT_A, sub_right[reg]);
			break;
		}
		break;
	case 0xf:
		switch(reg=READ_OP(cpustate)) {
		case 0: case 1: case 2: case 3:
			saturn_shift_nibble_left(cpustate, A+reg,BEGIN_A, COUNT_A);
			break; // asl a
		case 4: case 5: case 6: case 7:
			saturn_shift_nibble_right(cpustate, A+(reg&3),BEGIN_A, COUNT_A);
			break; // asr a
		case 8: case 9: case 0xa: case 0xb:
			saturn_negate(cpustate, A+(reg&3),BEGIN_A, COUNT_A);
			break; // A=-A a
		case 0xc: case 0xd: case 0xe: case 0xf:
			saturn_invert(cpustate, A+(reg&3),BEGIN_A, COUNT_A);
			break; // A=-A-1 a
		}
		break;
	}
}
