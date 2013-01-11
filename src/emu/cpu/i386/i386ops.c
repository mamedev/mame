static UINT8 I386OP(shift_rotate8)(i386_state *cpustate, UINT8 modrm, UINT32 value, UINT8 shift)
{
	UINT8 src = value;
	UINT8 dst = value;

	if( shift == 0 ) {
		CYCLES_RM(cpustate,modrm, 3, 7);
	} else if( shift == 1 ) {

		switch( (modrm >> 3) & 0x7 )
		{
			case 0:         /* ROL rm8, 1 */
				cpustate->CF = (src & 0x80) ? 1 : 0;
				dst = (src << 1) + cpustate->CF;
				cpustate->OF = ((src ^ dst) & 0x80) ? 1 : 0;
				CYCLES_RM(cpustate,modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 1:         /* ROR rm8, 1 */
				cpustate->CF = (src & 0x1) ? 1 : 0;
				dst = (cpustate->CF << 7) | (src >> 1);
				cpustate->OF = ((src ^ dst) & 0x80) ? 1 : 0;
				CYCLES_RM(cpustate,modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 2:         /* RCL rm8, 1 */
				dst = (src << 1) + cpustate->CF;
				cpustate->CF = (src & 0x80) ? 1 : 0;
				cpustate->OF = ((src ^ dst) & 0x80) ? 1 : 0;
				CYCLES_RM(cpustate,modrm, CYCLES_ROTATE_CARRY_REG, CYCLES_ROTATE_CARRY_MEM);
				break;
			case 3:         /* RCR rm8, 1 */
				dst = (cpustate->CF << 7) | (src >> 1);
				cpustate->CF = src & 0x1;
				cpustate->OF = ((src ^ dst) & 0x80) ? 1 : 0;
				CYCLES_RM(cpustate,modrm, CYCLES_ROTATE_CARRY_REG, CYCLES_ROTATE_CARRY_MEM);
				break;
			case 4:         /* SHL/SAL rm8, 1 */
			case 6:
				dst = src << 1;
				cpustate->CF = (src & 0x80) ? 1 : 0;
				cpustate->OF = (((cpustate->CF << 7) ^ dst) & 0x80) ? 1 : 0;
				SetSZPF8(dst);
				CYCLES_RM(cpustate,modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 5:         /* SHR rm8, 1 */
				dst = src >> 1;
				cpustate->CF = src & 0x1;
				cpustate->OF = (dst & 0x80) ? 1 : 0;
				SetSZPF8(dst);
				CYCLES_RM(cpustate,modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 7:         /* SAR rm8, 1 */
				dst = (INT8)(src) >> 1;
				cpustate->CF = src & 0x1;
				cpustate->OF = 0;
				SetSZPF8(dst);
				CYCLES_RM(cpustate,modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
		}

	} else {

		switch( (modrm >> 3) & 0x7 )
		{
			case 0:         /* ROL rm8, i8 */
				if(!(shift & 7))
				{
					if(shift & 0x18)
					{
						cpustate->CF = src & 1;
						cpustate->OF = (src & 1) ^ ((src >> 7) & 1);
					}
					break;
				}
				shift &= 7;
				dst = ((src & ((UINT8)0xff >> shift)) << shift) |
						((src & ((UINT8)0xff << (8-shift))) >> (8-shift));
				cpustate->CF = dst & 0x1;
				cpustate->OF = (dst & 1) ^ (dst >> 7);
				CYCLES_RM(cpustate,modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 1:         /* ROR rm8, i8 */
				if(!(shift & 7))
				{
					if(shift & 0x18)
					{
						cpustate->CF = (src >> 7) & 1;
						cpustate->OF = ((src >> 7) & 1) ^ ((src >> 6) & 1);
					}
					break;
				}
				shift &= 7;
				dst = ((src & ((UINT8)0xff << shift)) >> shift) |
						((src & ((UINT8)0xff >> (8-shift))) << (8-shift));
				cpustate->CF = (dst >> 7) & 1;
				cpustate->OF = ((dst >> 7) ^ (dst >> 6)) & 1;
				CYCLES_RM(cpustate,modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 2:         /* RCL rm8, i8 */
				shift %= 9;
				dst = ((src & ((UINT8)0xff >> shift)) << shift) |
						((src & ((UINT8)0xff << (9-shift))) >> (9-shift)) |
						(cpustate->CF << (shift-1));
				if(shift) cpustate->CF = (src >> (8-shift)) & 0x1;
				cpustate->OF = cpustate->CF ^ ((dst >> 7) & 1);
				CYCLES_RM(cpustate,modrm, CYCLES_ROTATE_CARRY_REG, CYCLES_ROTATE_CARRY_MEM);
				break;
			case 3:         /* RCR rm8, i8 */
				shift %= 9;
				dst = ((src & ((UINT8)0xff << shift)) >> shift) |
						((src & ((UINT8)0xff >> (8-shift))) << (9-shift)) |
						(cpustate->CF << (8-shift));
				if(shift) cpustate->CF = (src >> (shift-1)) & 0x1;
				cpustate->OF = ((dst >> 7) ^ (dst >> 6)) & 1;
				CYCLES_RM(cpustate,modrm, CYCLES_ROTATE_CARRY_REG, CYCLES_ROTATE_CARRY_MEM);
				break;
			case 4:         /* SHL/SAL rm8, i8 */
			case 6:
				shift &= 31;
				dst = src << shift;
				cpustate->CF = (src >> (8 - shift)) & 0x1;
				SetSZPF8(dst);
				CYCLES_RM(cpustate,modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 5:         /* SHR rm8, i8 */
				shift &= 31;
				dst = src >> shift;
				cpustate->CF = (src & (1 << (shift-1))) ? 1 : 0;
				SetSZPF8(dst);
				CYCLES_RM(cpustate,modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 7:         /* SAR rm8, i8 */
				shift &= 31;
				dst = (INT8)src >> shift;
				cpustate->CF = (src & (1 << (shift-1))) ? 1 : 0;
				SetSZPF8(dst);
				CYCLES_RM(cpustate,modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
		}
	}

	return dst;
}



static void I386OP(adc_rm8_r8)(i386_state *cpustate)        // Opcode 0x10
{
	UINT8 src, dst;
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_REG8(modrm);
		dst = LOAD_RM8(modrm);
		dst = ADC8(cpustate, dst, src, cpustate->CF);
		STORE_RM8(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		src = LOAD_REG8(modrm);
		dst = READ8(cpustate,ea);
		dst = ADC8(cpustate, dst, src, cpustate->CF);
		WRITE8(cpustate,ea, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_MEM);
	}
}

static void I386OP(adc_r8_rm8)(i386_state *cpustate)        // Opcode 0x12
{
	UINT8 src, dst;
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_RM8(modrm);
		dst = LOAD_REG8(modrm);
		dst = ADC8(cpustate, dst, src, cpustate->CF);
		STORE_REG8(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,0);
		src = READ8(cpustate,ea);
		dst = LOAD_REG8(modrm);
		dst = ADC8(cpustate, dst, src, cpustate->CF);
		STORE_REG8(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_MEM_REG);
	}
}

static void I386OP(adc_al_i8)(i386_state *cpustate)     // Opcode 0x14
{
	UINT8 src, dst;
	src = FETCH(cpustate);
	dst = REG8(AL);
	dst = ADC8(cpustate, dst, src, cpustate->CF);
	REG8(AL) = dst;
	CYCLES(cpustate,CYCLES_ALU_IMM_ACC);
}

static void I386OP(add_rm8_r8)(i386_state *cpustate)        // Opcode 0x00
{
	UINT8 src, dst;
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_REG8(modrm);
		dst = LOAD_RM8(modrm);
		dst = ADD8(cpustate,dst, src);
		STORE_RM8(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		src = LOAD_REG8(modrm);
		dst = READ8(cpustate,ea);
		dst = ADD8(cpustate,dst, src);
		WRITE8(cpustate,ea, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_MEM);
	}
}

static void I386OP(add_r8_rm8)(i386_state *cpustate)        // Opcode 0x02
{
	UINT8 src, dst;
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_RM8(modrm);
		dst = LOAD_REG8(modrm);
		dst = ADD8(cpustate,dst, src);
		STORE_REG8(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,0);
		src = READ8(cpustate,ea);
		dst = LOAD_REG8(modrm);
		dst = ADD8(cpustate,dst, src);
		STORE_REG8(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_MEM_REG);
	}
}

static void I386OP(add_al_i8)(i386_state *cpustate)     // Opcode 0x04
{
	UINT8 src, dst;
	src = FETCH(cpustate);
	dst = REG8(AL);
	dst = ADD8(cpustate,dst, src);
	REG8(AL) = dst;
	CYCLES(cpustate,CYCLES_ALU_IMM_ACC);
}

static void I386OP(and_rm8_r8)(i386_state *cpustate)        // Opcode 0x20
{
	UINT8 src, dst;
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_REG8(modrm);
		dst = LOAD_RM8(modrm);
		dst = AND8(cpustate,dst, src);
		STORE_RM8(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		src = LOAD_REG8(modrm);
		dst = READ8(cpustate,ea);
		dst = AND8(cpustate,dst, src);
		WRITE8(cpustate,ea, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_MEM);
	}
}

static void I386OP(and_r8_rm8)(i386_state *cpustate)        // Opcode 0x22
{
	UINT8 src, dst;
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_RM8(modrm);
		dst = LOAD_REG8(modrm);
		dst = AND8(cpustate,dst, src);
		STORE_REG8(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,0);
		src = READ8(cpustate,ea);
		dst = LOAD_REG8(modrm);
		dst = AND8(cpustate,dst, src);
		STORE_REG8(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_MEM_REG);
	}
}

static void I386OP(and_al_i8)(i386_state *cpustate)         // Opcode 0x24
{
	UINT8 src, dst;
	src = FETCH(cpustate);
	dst = REG8(AL);
	dst = AND8(cpustate,dst, src);
	REG8(AL) = dst;
	CYCLES(cpustate,CYCLES_ALU_IMM_ACC);
}

static void I386OP(clc)(i386_state *cpustate)               // Opcode 0xf8
{
	cpustate->CF = 0;
	CYCLES(cpustate,CYCLES_CLC);
}

static void I386OP(cld)(i386_state *cpustate)               // Opcode 0xfc
{
	cpustate->DF = 0;
	CYCLES(cpustate,CYCLES_CLD);
}

static void I386OP(cli)(i386_state *cpustate)               // Opcode 0xfa
{
	if(PROTECTED_MODE)
	{
		UINT8 IOPL = cpustate->IOP1 | (cpustate->IOP2 << 1);
		if(cpustate->CPL > IOPL)
			FAULT(FAULT_GP,0);
	}
	cpustate->IF = 0;
	CYCLES(cpustate,CYCLES_CLI);
}

static void I386OP(cmc)(i386_state *cpustate)               // Opcode 0xf5
{
	cpustate->CF ^= 1;
	CYCLES(cpustate,CYCLES_CMC);
}

static void I386OP(cmp_rm8_r8)(i386_state *cpustate)        // Opcode 0x38
{
	UINT8 src, dst;
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_REG8(modrm);
		dst = LOAD_RM8(modrm);
		SUB8(cpustate,dst, src);
		CYCLES(cpustate,CYCLES_CMP_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,0);
		src = LOAD_REG8(modrm);
		dst = READ8(cpustate,ea);
		SUB8(cpustate,dst, src);
		CYCLES(cpustate,CYCLES_CMP_REG_MEM);
	}
}

static void I386OP(cmp_r8_rm8)(i386_state *cpustate)        // Opcode 0x3a
{
	UINT8 src, dst;
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_RM8(modrm);
		dst = LOAD_REG8(modrm);
		SUB8(cpustate,dst, src);
		CYCLES(cpustate,CYCLES_CMP_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,0);
		src = READ8(cpustate,ea);
		dst = LOAD_REG8(modrm);
		SUB8(cpustate,dst, src);
		CYCLES(cpustate,CYCLES_CMP_MEM_REG);
	}
}

static void I386OP(cmp_al_i8)(i386_state *cpustate)         // Opcode 0x3c
{
	UINT8 src, dst;
	src = FETCH(cpustate);
	dst = REG8(AL);
	SUB8(cpustate,dst, src);
	CYCLES(cpustate,CYCLES_CMP_IMM_ACC);
}

static void I386OP(cmpsb)(i386_state *cpustate)             // Opcode 0xa6
{
	UINT32 eas, ead;
	UINT8 src, dst;
	if( cpustate->segment_prefix ) {
		eas = i386_translate(cpustate, cpustate->segment_override, cpustate->address_size ? REG32(ESI) : REG16(SI), 0 );
	} else {
		eas = i386_translate(cpustate, DS, cpustate->address_size ? REG32(ESI) : REG16(SI), 0 );
	}
	ead = i386_translate(cpustate, ES, cpustate->address_size ? REG32(EDI) : REG16(DI), 0 );
	src = READ8(cpustate,eas);
	dst = READ8(cpustate,ead);
	SUB8(cpustate,src, dst);
	BUMP_SI(cpustate,1);
	BUMP_DI(cpustate,1);
	CYCLES(cpustate,CYCLES_CMPS);
}

static void I386OP(in_al_i8)(i386_state *cpustate)          // Opcode 0xe4
{
	UINT16 port = FETCH(cpustate);
	UINT8 data = READPORT8(cpustate, port);
	REG8(AL) = data;
	CYCLES(cpustate,CYCLES_IN_VAR);
}

static void I386OP(in_al_dx)(i386_state *cpustate)          // Opcode 0xec
{
	UINT16 port = REG16(DX);
	UINT8 data = READPORT8(cpustate, port);
	REG8(AL) = data;
	CYCLES(cpustate,CYCLES_IN);
}

static void I386OP(ja_rel8)(i386_state *cpustate)           // Opcode 0x77
{
	INT8 disp = FETCH(cpustate);
	if( cpustate->CF == 0 && cpustate->ZF == 0 ) {
		NEAR_BRANCH(cpustate,disp);
		CYCLES(cpustate,CYCLES_JCC_DISP8);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(cpustate,CYCLES_JCC_DISP8_NOBRANCH);
	}
}

static void I386OP(jbe_rel8)(i386_state *cpustate)          // Opcode 0x76
{
	INT8 disp = FETCH(cpustate);
	if( cpustate->CF != 0 || cpustate->ZF != 0 ) {
		NEAR_BRANCH(cpustate,disp);
		CYCLES(cpustate,CYCLES_JCC_DISP8);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(cpustate,CYCLES_JCC_DISP8_NOBRANCH);
	}
}

static void I386OP(jc_rel8)(i386_state *cpustate)           // Opcode 0x72
{
	INT8 disp = FETCH(cpustate);
	if( cpustate->CF != 0 ) {
		NEAR_BRANCH(cpustate,disp);
		CYCLES(cpustate,CYCLES_JCC_DISP8);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(cpustate,CYCLES_JCC_DISP8_NOBRANCH);
	}
}

static void I386OP(jg_rel8)(i386_state *cpustate)           // Opcode 0x7f
{
	INT8 disp = FETCH(cpustate);
	if( cpustate->ZF == 0 && (cpustate->SF == cpustate->OF) ) {
		NEAR_BRANCH(cpustate,disp);
		CYCLES(cpustate,CYCLES_JCC_DISP8);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(cpustate,CYCLES_JCC_DISP8_NOBRANCH);
	}
}

static void I386OP(jge_rel8)(i386_state *cpustate)          // Opcode 0x7d
{
	INT8 disp = FETCH(cpustate);
	if(cpustate->SF == cpustate->OF) {
		NEAR_BRANCH(cpustate,disp);
		CYCLES(cpustate,CYCLES_JCC_DISP8);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(cpustate,CYCLES_JCC_DISP8_NOBRANCH);
	}
}

static void I386OP(jl_rel8)(i386_state *cpustate)           // Opcode 0x7c
{
	INT8 disp = FETCH(cpustate);
	if( (cpustate->SF != cpustate->OF) ) {
		NEAR_BRANCH(cpustate,disp);
		CYCLES(cpustate,CYCLES_JCC_DISP8);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(cpustate,CYCLES_JCC_DISP8_NOBRANCH);
	}
}

static void I386OP(jle_rel8)(i386_state *cpustate)      // Opcode 0x7e
{
	INT8 disp = FETCH(cpustate);
	if( cpustate->ZF != 0 || (cpustate->SF != cpustate->OF) ) {
		NEAR_BRANCH(cpustate,disp);
		CYCLES(cpustate,CYCLES_JCC_DISP8);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(cpustate,CYCLES_JCC_DISP8_NOBRANCH);
	}
}

static void I386OP(jnc_rel8)(i386_state *cpustate)          // Opcode 0x73
{
	INT8 disp = FETCH(cpustate);
	if( cpustate->CF == 0 ) {
		NEAR_BRANCH(cpustate,disp);
		CYCLES(cpustate,CYCLES_JCC_DISP8);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(cpustate,CYCLES_JCC_DISP8_NOBRANCH);
	}
}

static void I386OP(jno_rel8)(i386_state *cpustate)          // Opcode 0x71
{
	INT8 disp = FETCH(cpustate);
	if( cpustate->OF == 0 ) {
		NEAR_BRANCH(cpustate,disp);
		CYCLES(cpustate,CYCLES_JCC_DISP8);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(cpustate,CYCLES_JCC_DISP8_NOBRANCH);
	}
}

static void I386OP(jnp_rel8)(i386_state *cpustate)          // Opcode 0x7b
{
	INT8 disp = FETCH(cpustate);
	if( cpustate->PF == 0 ) {
		NEAR_BRANCH(cpustate,disp);
		CYCLES(cpustate,CYCLES_JCC_DISP8);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(cpustate,CYCLES_JCC_DISP8_NOBRANCH);
	}
}

static void I386OP(jns_rel8)(i386_state *cpustate)          // Opcode 0x79
{
	INT8 disp = FETCH(cpustate);
	if( cpustate->SF == 0 ) {
		NEAR_BRANCH(cpustate,disp);
		CYCLES(cpustate,CYCLES_JCC_DISP8);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(cpustate,CYCLES_JCC_DISP8_NOBRANCH);
	}
}

static void I386OP(jnz_rel8)(i386_state *cpustate)          // Opcode 0x75
{
	INT8 disp = FETCH(cpustate);
	if( cpustate->ZF == 0 ) {
		NEAR_BRANCH(cpustate,disp);
		CYCLES(cpustate,CYCLES_JCC_DISP8);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(cpustate,CYCLES_JCC_DISP8_NOBRANCH);
	}
}

static void I386OP(jo_rel8)(i386_state *cpustate)           // Opcode 0x70
{
	INT8 disp = FETCH(cpustate);
	if( cpustate->OF != 0 ) {
		NEAR_BRANCH(cpustate,disp);
		CYCLES(cpustate,CYCLES_JCC_DISP8);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(cpustate,CYCLES_JCC_DISP8_NOBRANCH);
	}
}

static void I386OP(jp_rel8)(i386_state *cpustate)           // Opcode 0x7a
{
	INT8 disp = FETCH(cpustate);
	if( cpustate->PF != 0 ) {
		NEAR_BRANCH(cpustate,disp);
		CYCLES(cpustate,CYCLES_JCC_DISP8);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(cpustate,CYCLES_JCC_DISP8_NOBRANCH);
	}
}

static void I386OP(js_rel8)(i386_state *cpustate)           // Opcode 0x78
{
	INT8 disp = FETCH(cpustate);
	if( cpustate->SF != 0 ) {
		NEAR_BRANCH(cpustate,disp);
		CYCLES(cpustate,CYCLES_JCC_DISP8);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(cpustate,CYCLES_JCC_DISP8_NOBRANCH);
	}
}

static void I386OP(jz_rel8)(i386_state *cpustate)           // Opcode 0x74
{
	INT8 disp = FETCH(cpustate);
	if( cpustate->ZF != 0 ) {
		NEAR_BRANCH(cpustate,disp);
		CYCLES(cpustate,CYCLES_JCC_DISP8);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(cpustate,CYCLES_JCC_DISP8_NOBRANCH);
	}
}

static void I386OP(jmp_rel8)(i386_state *cpustate)          // Opcode 0xeb
{
	INT8 disp = FETCH(cpustate);
	NEAR_BRANCH(cpustate,disp);
	CYCLES(cpustate,CYCLES_JMP_SHORT);      /* TODO: Timing = 7 + m */
}

static void I386OP(lahf)(i386_state *cpustate)              // Opcode 0x9f
{
	REG8(AH) = get_flags(cpustate) & 0xd7;
	CYCLES(cpustate,CYCLES_LAHF);
}

static void I386OP(lodsb)(i386_state *cpustate)             // Opcode 0xac
{
	UINT32 eas;
	if( cpustate->segment_prefix ) {
		eas = i386_translate(cpustate, cpustate->segment_override, cpustate->address_size ? REG32(ESI) : REG16(SI), 0 );
	} else {
		eas = i386_translate(cpustate, DS, cpustate->address_size ? REG32(ESI) : REG16(SI), 0 );
	}
	REG8(AL) = READ8(cpustate,eas);
	BUMP_SI(cpustate,1);
	CYCLES(cpustate,CYCLES_LODS);
}

static void I386OP(mov_rm8_r8)(i386_state *cpustate)        // Opcode 0x88
{
	UINT8 src;
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_REG8(modrm);
		STORE_RM8(modrm, src);
		CYCLES(cpustate,CYCLES_MOV_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		src = LOAD_REG8(modrm);
		WRITE8(cpustate,ea, src);
		CYCLES(cpustate,CYCLES_MOV_REG_MEM);
	}
}

static void I386OP(mov_r8_rm8)(i386_state *cpustate)        // Opcode 0x8a
{
	UINT8 src;
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_RM8(modrm);
		STORE_REG8(modrm, src);
		CYCLES(cpustate,CYCLES_MOV_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,0);
		src = READ8(cpustate,ea);
		STORE_REG8(modrm, src);
		CYCLES(cpustate,CYCLES_MOV_MEM_REG);
	}
}

static void I386OP(mov_rm8_i8)(i386_state *cpustate)        // Opcode 0xc6
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		UINT8 value = FETCH(cpustate);
		STORE_RM8(modrm, value);
		CYCLES(cpustate,CYCLES_MOV_IMM_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		UINT8 value = FETCH(cpustate);
		WRITE8(cpustate,ea, value);
		CYCLES(cpustate,CYCLES_MOV_IMM_MEM);
	}
}

static void I386OP(mov_r32_cr)(i386_state *cpustate)        // Opcode 0x0f 20
{
	if(PROTECTED_MODE && cpustate->CPL)
		FAULT(FAULT_GP, 0);
	UINT8 modrm = FETCH(cpustate);
	UINT8 cr = (modrm >> 3) & 0x7;

	STORE_RM32(modrm, cpustate->cr[cr]);
	CYCLES(cpustate,CYCLES_MOV_CR_REG);
}

static void I386OP(mov_r32_dr)(i386_state *cpustate)        // Opcode 0x0f 21
{
	if(PROTECTED_MODE && cpustate->CPL)
		FAULT(FAULT_GP, 0);
	UINT8 modrm = FETCH(cpustate);
	UINT8 dr = (modrm >> 3) & 0x7;

	STORE_RM32(modrm, cpustate->dr[dr]);
	switch(dr)
	{
		case 0:
		case 1:
		case 2:
		case 3:
			CYCLES(cpustate,CYCLES_MOV_REG_DR0_3);
			break;
		case 6:
		case 7:
			CYCLES(cpustate,CYCLES_MOV_REG_DR6_7);
			break;
	}
}

static void I386OP(mov_cr_r32)(i386_state *cpustate)        // Opcode 0x0f 22
{
	if(PROTECTED_MODE && cpustate->CPL)
		FAULT(FAULT_GP, 0);
	UINT8 modrm = FETCH(cpustate);
	UINT8 cr = (modrm >> 3) & 0x7;
	UINT32 data = LOAD_RM32(modrm);
	switch(cr)
	{
		case 0:
			data &= 0xfffeffff; // wp not supported on 386
			CYCLES(cpustate,CYCLES_MOV_REG_CR0);
			break;
		case 2: CYCLES(cpustate,CYCLES_MOV_REG_CR2); break;
		case 3: CYCLES(cpustate,CYCLES_MOV_REG_CR3); break;
		case 4: CYCLES(cpustate,1); break; // TODO
		default:
			fatalerror("i386: mov_cr_r32 CR%d!\n", cr);
			break;
	}
	cpustate->cr[cr] = data;
}

static void I386OP(mov_dr_r32)(i386_state *cpustate)        // Opcode 0x0f 23
{
	if(PROTECTED_MODE && cpustate->CPL)
		FAULT(FAULT_GP, 0);
	UINT8 modrm = FETCH(cpustate);
	UINT8 dr = (modrm >> 3) & 0x7;

	cpustate->dr[dr] = LOAD_RM32(modrm);
	switch(dr)
	{
		case 0:
		case 1:
		case 2:
		case 3:
			CYCLES(cpustate,CYCLES_MOV_DR0_3_REG);
			break;
		case 6:
		case 7:
			CYCLES(cpustate,CYCLES_MOV_DR6_7_REG);
			break;
		default:
			fatalerror("i386: mov_dr_r32 DR%d!\n", dr);
			break;
	}
}

static void I386OP(mov_al_m8)(i386_state *cpustate)         // Opcode 0xa0
{
	UINT32 offset, ea;
	if( cpustate->address_size ) {
		offset = FETCH32(cpustate);
	} else {
		offset = FETCH16(cpustate);
	}
	/* TODO: Not sure if this is correct... */
	if( cpustate->segment_prefix ) {
		ea = i386_translate(cpustate, cpustate->segment_override, offset, 0 );
	} else {
		ea = i386_translate(cpustate, DS, offset, 0 );
	}
	REG8(AL) = READ8(cpustate,ea);
	CYCLES(cpustate,CYCLES_MOV_IMM_MEM);
}

static void I386OP(mov_m8_al)(i386_state *cpustate)         // Opcode 0xa2
{
	UINT32 offset, ea;
	if( cpustate->address_size ) {
		offset = FETCH32(cpustate);
	} else {
		offset = FETCH16(cpustate);
	}
	/* TODO: Not sure if this is correct... */
	if( cpustate->segment_prefix ) {
		ea = i386_translate(cpustate, cpustate->segment_override, offset, 1 );
	} else {
		ea = i386_translate(cpustate, DS, offset, 1 );
	}
	WRITE8(cpustate, ea, REG8(AL) );
	CYCLES(cpustate,CYCLES_MOV_MEM_ACC);
}

static void I386OP(mov_rm16_sreg)(i386_state *cpustate)     // Opcode 0x8c
{
	UINT8 modrm = FETCH(cpustate);
	int s = (modrm >> 3) & 0x7;

	if( modrm >= 0xc0 ) {
		if(cpustate->operand_size)
			STORE_RM32(modrm, cpustate->sreg[s].selector);
		else
			STORE_RM16(modrm, cpustate->sreg[s].selector);
		CYCLES(cpustate,CYCLES_MOV_SREG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		WRITE16(cpustate,ea, cpustate->sreg[s].selector);
		CYCLES(cpustate,CYCLES_MOV_SREG_MEM);
	}
}

static void I386OP(mov_sreg_rm16)(i386_state *cpustate)     // Opcode 0x8e
{
	UINT16 selector;
	UINT8 modrm = FETCH(cpustate);
	bool fault;
	int s = (modrm >> 3) & 0x7;

	if( modrm >= 0xc0 ) {
		selector = LOAD_RM16(modrm);
		CYCLES(cpustate,CYCLES_MOV_REG_SREG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,0);
		selector = READ16(cpustate,ea);
		CYCLES(cpustate,CYCLES_MOV_MEM_SREG);
	}

	i386_sreg_load(cpustate,selector,s,&fault);
	if((s == SS) && !fault)
	{
		if(cpustate->IF != 0) // if external interrupts are enabled
		{
			cpustate->IF = 0;  // reset IF for the next instruction
			cpustate->delayed_interrupt_enable = 1;
		}
	}
}

static void I386OP(mov_al_i8)(i386_state *cpustate)         // Opcode 0xb0
{
	REG8(AL) = FETCH(cpustate);
	CYCLES(cpustate,CYCLES_MOV_IMM_REG);
}

static void I386OP(mov_cl_i8)(i386_state *cpustate)         // Opcode 0xb1
{
	REG8(CL) = FETCH(cpustate);
	CYCLES(cpustate,CYCLES_MOV_IMM_REG);
}

static void I386OP(mov_dl_i8)(i386_state *cpustate)         // Opcode 0xb2
{
	REG8(DL) = FETCH(cpustate);
	CYCLES(cpustate,CYCLES_MOV_IMM_REG);
}

static void I386OP(mov_bl_i8)(i386_state *cpustate)         // Opcode 0xb3
{
	REG8(BL) = FETCH(cpustate);
	CYCLES(cpustate,CYCLES_MOV_IMM_REG);
}

static void I386OP(mov_ah_i8)(i386_state *cpustate)         // Opcode 0xb4
{
	REG8(AH) = FETCH(cpustate);
	CYCLES(cpustate,CYCLES_MOV_IMM_REG);
}

static void I386OP(mov_ch_i8)(i386_state *cpustate)         // Opcode 0xb5
{
	REG8(CH) = FETCH(cpustate);
	CYCLES(cpustate,CYCLES_MOV_IMM_REG);
}

static void I386OP(mov_dh_i8)(i386_state *cpustate)         // Opcode 0xb6
{
	REG8(DH) = FETCH(cpustate);
	CYCLES(cpustate,CYCLES_MOV_IMM_REG);
}

static void I386OP(mov_bh_i8)(i386_state *cpustate)         // Opcode 0xb7
{
	REG8(BH) = FETCH(cpustate);
	CYCLES(cpustate,CYCLES_MOV_IMM_REG);
}

static void I386OP(movsb)(i386_state *cpustate)             // Opcode 0xa4
{
	UINT32 eas, ead;
	UINT8 v;
	if( cpustate->segment_prefix ) {
		eas = i386_translate(cpustate, cpustate->segment_override, cpustate->address_size ? REG32(ESI) : REG16(SI), 0 );
	} else {
		eas = i386_translate(cpustate, DS, cpustate->address_size ? REG32(ESI) : REG16(SI), 0 );
	}
	ead = i386_translate(cpustate, ES, cpustate->address_size ? REG32(EDI) : REG16(DI), 1 );
	v = READ8(cpustate,eas);
	WRITE8(cpustate,ead, v);
	BUMP_SI(cpustate,1);
	BUMP_DI(cpustate,1);
	CYCLES(cpustate,CYCLES_MOVS);
}

static void I386OP(or_rm8_r8)(i386_state *cpustate)         // Opcode 0x08
{
	UINT8 src, dst;
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_REG8(modrm);
		dst = LOAD_RM8(modrm);
		dst = OR8(cpustate,dst, src);
		STORE_RM8(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		src = LOAD_REG8(modrm);
		dst = READ8(cpustate,ea);
		dst = OR8(cpustate,dst, src);
		WRITE8(cpustate,ea, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_MEM);
	}
}

static void I386OP(or_r8_rm8)(i386_state *cpustate)         // Opcode 0x0a
{
	UINT8 src, dst;
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_RM8(modrm);
		dst = LOAD_REG8(modrm);
		dst = OR8(cpustate,dst, src);
		STORE_REG8(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,0);
		src = READ8(cpustate,ea);
		dst = LOAD_REG8(modrm);
		dst = OR8(cpustate,dst, src);
		STORE_REG8(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_MEM_REG);
	}
}

static void I386OP(or_al_i8)(i386_state *cpustate)          // Opcode 0x0c
{
	UINT8 src, dst;
	src = FETCH(cpustate);
	dst = REG8(AL);
	dst = OR8(cpustate,dst, src);
	REG8(EAX) = dst;
	CYCLES(cpustate,CYCLES_ALU_IMM_ACC);
}

static void I386OP(out_al_i8)(i386_state *cpustate)         // Opcode 0xe6
{
	UINT16 port = FETCH(cpustate);
	UINT8 data = REG8(AL);
	WRITEPORT8(cpustate, port, data);
	CYCLES(cpustate,CYCLES_OUT_VAR);
}

static void I386OP(out_al_dx)(i386_state *cpustate)         // Opcode 0xee
{
	UINT16 port = REG16(DX);
	UINT8 data = REG8(AL);
	WRITEPORT8(cpustate, port, data);
	CYCLES(cpustate,CYCLES_OUT);
}


static void I386OP(arpl)(i386_state *cpustate)           // Opcode 0x63
{
	UINT16 src, dst;
	UINT8 modrm = FETCH(cpustate);
	UINT8 flag = 0;

	if(PROTECTED_MODE && !V8086_MODE)
	{
			if( modrm >= 0xc0 ) {
			src = LOAD_REG16(modrm);
			dst = LOAD_RM16(modrm);
			if( (dst&0x3) < (src&0x3) ) {
				dst = (dst&0xfffc) | (src&0x3);
				flag = 1;
				STORE_RM16(modrm, dst);
			}
		} else {
			UINT32 ea = GetEA(cpustate, modrm,1);
			src = LOAD_REG16(modrm);
			dst = READ16(cpustate, ea);
			if( (dst&0x3) < (src&0x3) ) {
				dst = (dst&0xfffc) | (src&0x3);
				flag = 1;
				WRITE16(cpustate, ea, dst);
			}
		}
		SetZF(flag);
	}
	else
		i386_trap(cpustate, 6, 0, 0);  // invalid opcode in real mode or v8086 mode
}

static void I386OP(push_i8)(i386_state *cpustate)           // Opcode 0x6a
{
	UINT8 value = FETCH(cpustate);
	PUSH8(cpustate,value);
	CYCLES(cpustate,CYCLES_PUSH_IMM);
}

static void I386OP(ins_generic)(i386_state *cpustate, int size)
{
	UINT32 ead;
	UINT8 vb;
	UINT16 vw;
	UINT32 vd;

	ead = i386_translate(cpustate, ES, cpustate->address_size ? REG32(EDI) : REG16(DI), 1 );

	switch(size) {
	case 1:
		vb = READPORT8(cpustate, REG16(DX));
		WRITE8(cpustate,ead, vb);
		break;
	case 2:
		vw = READPORT16(cpustate, REG16(DX));
		WRITE16(cpustate,ead, vw);
		break;
	case 4:
		vd = READPORT32(cpustate, REG16(DX));
		WRITE32(cpustate,ead, vd);
		break;
	}

	if(cpustate->address_size)
		REG32(EDI) += ((cpustate->DF) ? -1 : 1) * size;
	else
		REG16(DI) += ((cpustate->DF) ? -1 : 1) * size;
	CYCLES(cpustate,CYCLES_INS);    // TODO: Confirm this value
}

static void I386OP(insb)(i386_state *cpustate)              // Opcode 0x6c
{
	I386OP(ins_generic)(cpustate, 1);
}

static void I386OP(insw)(i386_state *cpustate)              // Opcode 0x6d
{
	I386OP(ins_generic)(cpustate, 2);
}

static void I386OP(insd)(i386_state *cpustate)              // Opcode 0x6d
{
	I386OP(ins_generic)(cpustate, 4);
}

static void I386OP(outs_generic)(i386_state *cpustate, int size)
{
	UINT32 eas;
	UINT8 vb;
	UINT16 vw;
	UINT32 vd;

	if( cpustate->segment_prefix ) {
		eas = i386_translate(cpustate, cpustate->segment_override, cpustate->address_size ? REG32(ESI) : REG16(SI), 0 );
	} else {
		eas = i386_translate(cpustate, DS, cpustate->address_size ? REG32(ESI) : REG16(SI), 0 );
	}

	switch(size) {
	case 1:
		vb = READ8(cpustate,eas);
		WRITEPORT8(cpustate, REG16(DX), vb);
		break;
	case 2:
		vw = READ16(cpustate,eas);
		WRITEPORT16(cpustate, REG16(DX), vw);
		break;
	case 4:
		vd = READ32(cpustate,eas);
		WRITEPORT32(cpustate, REG16(DX), vd);
		break;
	}

	if(cpustate->address_size)
		REG32(ESI) += ((cpustate->DF) ? -1 : 1) * size;
	else
		REG16(SI) += ((cpustate->DF) ? -1 : 1) * size;
	CYCLES(cpustate,CYCLES_OUTS);   // TODO: Confirm this value
}

static void I386OP(outsb)(i386_state *cpustate)             // Opcode 0x6e
{
	I386OP(outs_generic)(cpustate, 1);
}

static void I386OP(outsw)(i386_state *cpustate)             // Opcode 0x6f
{
	I386OP(outs_generic)(cpustate, 2);
}

static void I386OP(outsd)(i386_state *cpustate)             // Opcode 0x6f
{
	I386OP(outs_generic)(cpustate, 4);
}

static void I386OP(repeat)(i386_state *cpustate, int invert_flag)
{
	UINT32 repeated_eip = cpustate->eip;
	UINT32 repeated_pc = cpustate->pc;
	UINT8 opcode; // = FETCH(cpustate);
//  UINT32 eas, ead;
	UINT32 count;
	INT32 cycle_base = 0, cycle_adjustment = 0;
	UINT8 prefix_flag=1;
	UINT8 *flag = NULL;


	do {
	repeated_eip = cpustate->eip;
	repeated_pc = cpustate->pc;
	opcode = FETCH(cpustate);
	switch(opcode) {
		case 0x0f:
		if (invert_flag == 0)
			I386OP(decode_three_bytef3)(cpustate); // sse f3 0f
		else
			I386OP(decode_three_bytef2)(cpustate); // sse f2 0f
		return;
		case 0x26:
		cpustate->segment_override=ES;
		cpustate->segment_prefix=1;
		break;
		case 0x2e:
		cpustate->segment_override=CS;
		cpustate->segment_prefix=1;
		break;
		case 0x36:
		cpustate->segment_override=SS;
		cpustate->segment_prefix=1;
		break;
		case 0x3e:
		cpustate->segment_override=DS;
		cpustate->segment_prefix=1;
		break;
		case 0x64:
		cpustate->segment_override=FS;
		cpustate->segment_prefix=1;
		break;
		case 0x65:
		cpustate->segment_override=GS;
		cpustate->segment_prefix=1;
		break;
		case 0x66:
		cpustate->operand_size ^= 1;
		break;
		case 0x67:
		cpustate->address_size ^= 1;
		break;
		default:
		prefix_flag=0;
	}
	} while (prefix_flag);


	if( cpustate->segment_prefix ) {
		// FIXME: the following does not work if both address override and segment override are used
		i386_translate(cpustate, cpustate->segment_override, cpustate->sreg[cpustate->segment_prefix].d ? REG32(ESI) : REG16(SI), -1 );
	} else {
		//eas =
		i386_translate(cpustate, DS, cpustate->address_size ? REG32(ESI) : REG16(SI), -1 );
	}
	i386_translate(cpustate, ES, cpustate->address_size ? REG32(EDI) : REG16(DI), -1 );

	switch(opcode)
	{
		case 0x6c:
		case 0x6d:
			/* INSB, INSW, INSD */
			// TODO: cycle count
			cycle_base = 8;
			cycle_adjustment = -4;
			flag = NULL;
			break;

		case 0x6e:
		case 0x6f:
			/* OUTSB, OUTSW, OUTSD */
			// TODO: cycle count
			cycle_base = 8;
			cycle_adjustment = -4;
			flag = NULL;
			break;

		case 0xa4:
		case 0xa5:
			/* MOVSB, MOVSW, MOVSD */
			cycle_base = 8;
			cycle_adjustment = -4;
			flag = NULL;
			break;

		case 0xa6:
		case 0xa7:
			/* CMPSB, CMPSW, CMPSD */
			cycle_base = 5;
			cycle_adjustment = -1;
			flag = &cpustate->ZF;
			break;

		case 0xac:
		case 0xad:
			/* LODSB, LODSW, LODSD */
			cycle_base = 5;
			cycle_adjustment = 1;
			flag = NULL;
			break;

		case 0xaa:
		case 0xab:
			/* STOSB, STOSW, STOSD */
			cycle_base = 5;
			cycle_adjustment = 0;
			flag = NULL;
			break;

		case 0xae:
		case 0xaf:
			/* SCASB, SCASW, SCASD */
			cycle_base = 5;
			cycle_adjustment = 0;
			flag = &cpustate->ZF;
			break;

		case 0x90:
			CYCLES(cpustate,CYCLES_NOP);
			return;

		default:
			fatalerror("i386: Invalid REP/opcode %02X combination\n",opcode);
			break;
	}

	if( cpustate->address_size ) {
		if( REG32(ECX) == 0 )
			return;
	} else {
		if( REG16(CX) == 0 )
			return;
	}

	/* now actually perform the repeat */
	CYCLES_NUM(cycle_base);
	do
	{
		cpustate->eip = repeated_eip;
		cpustate->pc = repeated_pc;
		try
		{
			I386OP(decode_opcode)(cpustate);
		}
		catch (UINT64 e)
		{
			cpustate->eip = cpustate->prev_eip;
			throw e;
		}

		CYCLES_NUM(cycle_adjustment);

		if (cpustate->address_size)
			count = --REG32(ECX);
		else
			count = --REG16(CX);
		if (cpustate->cycles <= 0)
			goto outofcycles;
	}
	while( count && (!flag || (invert_flag ? !*flag : *flag)) );
	return;

outofcycles:
	/* if we run out of cycles to execute, and we are still in the repeat, we need
	 * to exit this instruction in such a way to go right back into it when we have
	 * time to execute cycles */
	if(flag && (invert_flag ? *flag : !*flag))
		return;
	cpustate->eip = cpustate->prev_eip;
	CHANGE_PC(cpustate,cpustate->eip);
	CYCLES_NUM(-cycle_base);
}

static void I386OP(rep)(i386_state *cpustate)               // Opcode 0xf3
{
	I386OP(repeat)(cpustate, 0);
}

static void I386OP(repne)(i386_state *cpustate)             // Opcode 0xf2
{
	I386OP(repeat)(cpustate, 1);
}

static void I386OP(sahf)(i386_state *cpustate)              // Opcode 0x9e
{
	set_flags(cpustate, (get_flags(cpustate) & 0xffffff00) | (REG8(AH) & 0xd7) );
	CYCLES(cpustate,CYCLES_SAHF);
}

static void I386OP(sbb_rm8_r8)(i386_state *cpustate)        // Opcode 0x18
{
	UINT8 src, dst;
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_REG8(modrm);
		dst = LOAD_RM8(modrm);
		dst = SBB8(cpustate, dst, src, cpustate->CF);
		STORE_RM8(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		src = LOAD_REG8(modrm);
		dst = READ8(cpustate,ea);
		dst = SBB8(cpustate, dst, src, cpustate->CF);
		WRITE8(cpustate,ea, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_MEM);
	}
}

static void I386OP(sbb_r8_rm8)(i386_state *cpustate)        // Opcode 0x1a
{
	UINT8 src, dst;
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_RM8(modrm);
		dst = LOAD_REG8(modrm);
		dst = SBB8(cpustate, dst, src, cpustate->CF);
		STORE_REG8(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,0);
		src = READ8(cpustate,ea);
		dst = LOAD_REG8(modrm);
		dst = SBB8(cpustate, dst, src, cpustate->CF);
		STORE_REG8(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_MEM_REG);
	}
}

static void I386OP(sbb_al_i8)(i386_state *cpustate)         // Opcode 0x1c
{
	UINT8 src, dst;
	src = FETCH(cpustate);
	dst = REG8(AL);
	dst = SBB8(cpustate, dst, src, cpustate->CF);
	REG8(EAX) = dst;
	CYCLES(cpustate,CYCLES_ALU_IMM_ACC);
}

static void I386OP(scasb)(i386_state *cpustate)             // Opcode 0xae
{
	UINT32 eas;
	UINT8 src, dst;
	eas = i386_translate(cpustate, ES, cpustate->address_size ? REG32(EDI) : REG16(DI), 0 );
	src = READ8(cpustate,eas);
	dst = REG8(AL);
	SUB8(cpustate,dst, src);
	BUMP_DI(cpustate,1);
	CYCLES(cpustate,CYCLES_SCAS);
}

static void I386OP(setalc)(i386_state *cpustate)            // Opcode 0xd6 (undocumented)
{
	if( cpustate->CF ) {
		REG8(AL) = 0xff;
	} else {
		REG8(AL) = 0;
	}
	CYCLES(cpustate,3);
}

static void I386OP(seta_rm8)(i386_state *cpustate)          // Opcode 0x0f 97
{
	UINT8 modrm = FETCH(cpustate);
	UINT8 value = 0;
	if( cpustate->CF == 0 && cpustate->ZF == 0 ) {
		value = 1;
	}
	if( modrm >= 0xc0 ) {
		STORE_RM8(modrm, value);
		CYCLES(cpustate,CYCLES_SETCC_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		WRITE8(cpustate,ea, value);
		CYCLES(cpustate,CYCLES_SETCC_MEM);
	}
}

static void I386OP(setbe_rm8)(i386_state *cpustate)         // Opcode 0x0f 96
{
	UINT8 modrm = FETCH(cpustate);
	UINT8 value = 0;
	if( cpustate->CF != 0 || cpustate->ZF != 0 ) {
		value = 1;
	}
	if( modrm >= 0xc0 ) {
		STORE_RM8(modrm, value);
		CYCLES(cpustate,CYCLES_SETCC_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		WRITE8(cpustate,ea, value);
		CYCLES(cpustate,CYCLES_SETCC_MEM);
	}
}

static void I386OP(setc_rm8)(i386_state *cpustate)          // Opcode 0x0f 92
{
	UINT8 modrm = FETCH(cpustate);
	UINT8 value = 0;
	if( cpustate->CF != 0 ) {
		value = 1;
	}
	if( modrm >= 0xc0 ) {
		STORE_RM8(modrm, value);
		CYCLES(cpustate,CYCLES_SETCC_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		WRITE8(cpustate,ea, value);
		CYCLES(cpustate,CYCLES_SETCC_MEM);
	}
}

static void I386OP(setg_rm8)(i386_state *cpustate)          // Opcode 0x0f 9f
{
	UINT8 modrm = FETCH(cpustate);
	UINT8 value = 0;
	if( cpustate->ZF == 0 && (cpustate->SF == cpustate->OF) ) {
		value = 1;
	}
	if( modrm >= 0xc0 ) {
		STORE_RM8(modrm, value);
		CYCLES(cpustate,CYCLES_SETCC_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		WRITE8(cpustate,ea, value);
		CYCLES(cpustate,CYCLES_SETCC_MEM);
	}
}

static void I386OP(setge_rm8)(i386_state *cpustate)         // Opcode 0x0f 9d
{
	UINT8 modrm = FETCH(cpustate);
	UINT8 value = 0;
	if(cpustate->SF == cpustate->OF) {
		value = 1;
	}
	if( modrm >= 0xc0 ) {
		STORE_RM8(modrm, value);
		CYCLES(cpustate,CYCLES_SETCC_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		WRITE8(cpustate,ea, value);
		CYCLES(cpustate,CYCLES_SETCC_MEM);
	}
}

static void I386OP(setl_rm8)(i386_state *cpustate)          // Opcode 0x0f 9c
{
	UINT8 modrm = FETCH(cpustate);
	UINT8 value = 0;
	if( cpustate->SF != cpustate->OF ) {
		value = 1;
	}
	if( modrm >= 0xc0 ) {
		STORE_RM8(modrm, value);
		CYCLES(cpustate,CYCLES_SETCC_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		WRITE8(cpustate,ea, value);
		CYCLES(cpustate,CYCLES_SETCC_MEM);
	}
}

static void I386OP(setle_rm8)(i386_state *cpustate)         // Opcode 0x0f 9e
{
	UINT8 modrm = FETCH(cpustate);
	UINT8 value = 0;
	if( cpustate->ZF != 0 || (cpustate->SF != cpustate->OF) ) {
		value = 1;
	}
	if( modrm >= 0xc0 ) {
		STORE_RM8(modrm, value);
		CYCLES(cpustate,CYCLES_SETCC_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		WRITE8(cpustate,ea, value);
		CYCLES(cpustate,CYCLES_SETCC_MEM);
	}
}

static void I386OP(setnc_rm8)(i386_state *cpustate)         // Opcode 0x0f 93
{
	UINT8 modrm = FETCH(cpustate);
	UINT8 value = 0;
	if( cpustate->CF == 0 ) {
		value = 1;
	}
	if( modrm >= 0xc0 ) {
		STORE_RM8(modrm, value);
		CYCLES(cpustate,CYCLES_SETCC_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		WRITE8(cpustate,ea, value);
		CYCLES(cpustate,CYCLES_SETCC_MEM);
	}
}

static void I386OP(setno_rm8)(i386_state *cpustate)         // Opcode 0x0f 91
{
	UINT8 modrm = FETCH(cpustate);
	UINT8 value = 0;
	if( cpustate->OF == 0 ) {
		value = 1;
	}
	if( modrm >= 0xc0 ) {
		STORE_RM8(modrm, value);
		CYCLES(cpustate,CYCLES_SETCC_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		WRITE8(cpustate,ea, value);
		CYCLES(cpustate,CYCLES_SETCC_MEM);
	}
}

static void I386OP(setnp_rm8)(i386_state *cpustate)         // Opcode 0x0f 9b
{
	UINT8 modrm = FETCH(cpustate);
	UINT8 value = 0;
	if( cpustate->PF == 0 ) {
		value = 1;
	}
	if( modrm >= 0xc0 ) {
		STORE_RM8(modrm, value);
		CYCLES(cpustate,CYCLES_SETCC_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		WRITE8(cpustate,ea, value);
		CYCLES(cpustate,CYCLES_SETCC_MEM);
	}
}

static void I386OP(setns_rm8)(i386_state *cpustate)         // Opcode 0x0f 99
{
	UINT8 modrm = FETCH(cpustate);
	UINT8 value = 0;
	if( cpustate->SF == 0 ) {
		value = 1;
	}
	if( modrm >= 0xc0 ) {
		STORE_RM8(modrm, value);
		CYCLES(cpustate,CYCLES_SETCC_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		WRITE8(cpustate,ea, value);
		CYCLES(cpustate,CYCLES_SETCC_MEM);
	}
}

static void I386OP(setnz_rm8)(i386_state *cpustate)         // Opcode 0x0f 95
{
	UINT8 modrm = FETCH(cpustate);
	UINT8 value = 0;
	if( cpustate->ZF == 0 ) {
		value = 1;
	}
	if( modrm >= 0xc0 ) {
		STORE_RM8(modrm, value);
		CYCLES(cpustate,CYCLES_SETCC_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		WRITE8(cpustate,ea, value);
		CYCLES(cpustate,CYCLES_SETCC_MEM);
	}
}

static void I386OP(seto_rm8)(i386_state *cpustate)          // Opcode 0x0f 90
{
	UINT8 modrm = FETCH(cpustate);
	UINT8 value = 0;
	if( cpustate->OF != 0 ) {
		value = 1;
	}
	if( modrm >= 0xc0 ) {
		STORE_RM8(modrm, value);
		CYCLES(cpustate,CYCLES_SETCC_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		WRITE8(cpustate,ea, value);
		CYCLES(cpustate,CYCLES_SETCC_MEM);
	}
}

static void I386OP(setp_rm8)(i386_state *cpustate)          // Opcode 0x0f 9a
{
	UINT8 modrm = FETCH(cpustate);
	UINT8 value = 0;
	if( cpustate->PF != 0 ) {
		value = 1;
	}
	if( modrm >= 0xc0 ) {
		STORE_RM8(modrm, value);
		CYCLES(cpustate,CYCLES_SETCC_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		WRITE8(cpustate,ea, value);
		CYCLES(cpustate,CYCLES_SETCC_MEM);
	}
}

static void I386OP(sets_rm8)(i386_state *cpustate)          // Opcode 0x0f 98
{
	UINT8 modrm = FETCH(cpustate);
	UINT8 value = 0;
	if( cpustate->SF != 0 ) {
		value = 1;
	}
	if( modrm >= 0xc0 ) {
		STORE_RM8(modrm, value);
		CYCLES(cpustate,CYCLES_SETCC_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		WRITE8(cpustate,ea, value);
		CYCLES(cpustate,CYCLES_SETCC_MEM);
	}
}

static void I386OP(setz_rm8)(i386_state *cpustate)          // Opcode 0x0f 94
{
	UINT8 modrm = FETCH(cpustate);
	UINT8 value = 0;
	if( cpustate->ZF != 0 ) {
		value = 1;
	}
	if( modrm >= 0xc0 ) {
		STORE_RM8(modrm, value);
		CYCLES(cpustate,CYCLES_SETCC_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		WRITE8(cpustate,ea, value);
		CYCLES(cpustate,CYCLES_SETCC_MEM);
	}
}

static void I386OP(stc)(i386_state *cpustate)               // Opcode 0xf9
{
	cpustate->CF = 1;
	CYCLES(cpustate,CYCLES_STC);
}

static void I386OP(std)(i386_state *cpustate)               // Opcode 0xfd
{
	cpustate->DF = 1;
	CYCLES(cpustate,CYCLES_STD);
}

static void I386OP(sti)(i386_state *cpustate)               // Opcode 0xfb
{
	if(PROTECTED_MODE)
	{
		UINT8 IOPL = cpustate->IOP1 | (cpustate->IOP2 << 1);
		if(cpustate->CPL > IOPL)
			FAULT(FAULT_GP,0);
	}
	cpustate->delayed_interrupt_enable = 1;  // IF is set after the next instruction.
	CYCLES(cpustate,CYCLES_STI);
}

static void I386OP(stosb)(i386_state *cpustate)             // Opcode 0xaa
{
	UINT32 ead;
	ead = i386_translate(cpustate, ES, cpustate->address_size ? REG32(EDI) : REG16(DI), 1 );
	WRITE8(cpustate,ead, REG8(AL));
	BUMP_DI(cpustate,1);
	CYCLES(cpustate,CYCLES_STOS);
}

static void I386OP(sub_rm8_r8)(i386_state *cpustate)        // Opcode 0x28
{
	UINT8 src, dst;
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_REG8(modrm);
		dst = LOAD_RM8(modrm);
		dst = SUB8(cpustate,dst, src);
		STORE_RM8(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		src = LOAD_REG8(modrm);
		dst = READ8(cpustate,ea);
		dst = SUB8(cpustate,dst, src);
		WRITE8(cpustate,ea, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_MEM);
	}
}

static void I386OP(sub_r8_rm8)(i386_state *cpustate)        // Opcode 0x2a
{
	UINT8 src, dst;
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_RM8(modrm);
		dst = LOAD_REG8(modrm);
		dst = SUB8(cpustate,dst, src);
		STORE_REG8(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,0);
		src = READ8(cpustate,ea);
		dst = LOAD_REG8(modrm);
		dst = SUB8(cpustate,dst, src);
		STORE_REG8(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_MEM_REG);
	}
}

static void I386OP(sub_al_i8)(i386_state *cpustate)         // Opcode 0x2c
{
	UINT8 src, dst;
	src = FETCH(cpustate);
	dst = REG8(EAX);
	dst = SUB8(cpustate,dst, src);
	REG8(EAX) = dst;
	CYCLES(cpustate,CYCLES_ALU_IMM_ACC);
}

static void I386OP(test_al_i8)(i386_state *cpustate)        // Opcode 0xa8
{
	UINT8 src = FETCH(cpustate);
	UINT8 dst = REG8(AL);
	dst = src & dst;
	SetSZPF8(dst);
	cpustate->CF = 0;
	cpustate->OF = 0;
	CYCLES(cpustate,CYCLES_ALU_IMM_ACC);
}

static void I386OP(test_rm8_r8)(i386_state *cpustate)       // Opcode 0x84
{
	UINT8 src, dst;
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_REG8(modrm);
		dst = LOAD_RM8(modrm);
		dst = src & dst;
		SetSZPF8(dst);
		cpustate->CF = 0;
		cpustate->OF = 0;
		CYCLES(cpustate,CYCLES_TEST_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,0);
		src = LOAD_REG8(modrm);
		dst = READ8(cpustate,ea);
		dst = src & dst;
		SetSZPF8(dst);
		cpustate->CF = 0;
		cpustate->OF = 0;
		CYCLES(cpustate,CYCLES_TEST_REG_MEM);
	}
}

static void I386OP(xchg_r8_rm8)(i386_state *cpustate)       // Opcode 0x86
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		UINT8 src = LOAD_RM8(modrm);
		UINT8 dst = LOAD_REG8(modrm);
		STORE_REG8(modrm, src);
		STORE_RM8(modrm, dst);
		CYCLES(cpustate,CYCLES_XCHG_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		UINT8 src = READ8(cpustate,ea);
		UINT8 dst = LOAD_REG8(modrm);
		WRITE8(cpustate,ea, dst);
		STORE_REG8(modrm, src);
		CYCLES(cpustate,CYCLES_XCHG_REG_MEM);
	}
}

static void I386OP(xor_rm8_r8)(i386_state *cpustate)        // Opcode 0x30
{
	UINT8 src, dst;
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_REG8(modrm);
		dst = LOAD_RM8(modrm);
		dst = XOR8(cpustate,dst, src);
		STORE_RM8(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		src = LOAD_REG8(modrm);
		dst = READ8(cpustate,ea);
		dst = XOR8(cpustate,dst, src);
		WRITE8(cpustate,ea, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_MEM);
	}
}

static void I386OP(xor_r8_rm8)(i386_state *cpustate)        // Opcode 0x32
{
	UINT32 src, dst;
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_RM8(modrm);
		dst = LOAD_REG8(modrm);
		dst = XOR8(cpustate,dst, src);
		STORE_REG8(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,0);
		src = READ8(cpustate,ea);
		dst = LOAD_REG8(modrm);
		dst = XOR8(cpustate,dst, src);
		STORE_REG8(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_MEM_REG);
	}
}

static void I386OP(xor_al_i8)(i386_state *cpustate)         // Opcode 0x34
{
	UINT8 src, dst;
	src = FETCH(cpustate);
	dst = REG8(AL);
	dst = XOR8(cpustate,dst, src);
	REG8(AL) = dst;
	CYCLES(cpustate,CYCLES_ALU_IMM_ACC);
}



static void I386OP(group80_8)(i386_state *cpustate)         // Opcode 0x80
{
	UINT32 ea;
	UINT8 src, dst;
	UINT8 modrm = FETCH(cpustate);

	switch( (modrm >> 3) & 0x7 )
	{
		case 0:     // ADD Rm8, i8
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM8(modrm);
				src = FETCH(cpustate);
				dst = ADD8(cpustate,dst, src);
				STORE_RM8(modrm, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(cpustate,modrm,0);
				dst = READ8(cpustate,ea);
				src = FETCH(cpustate);
				dst = ADD8(cpustate,dst, src);
				WRITE8(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_MEM);
			}
			break;
		case 1:     // OR Rm8, i8
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM8(modrm);
				src = FETCH(cpustate);
				dst = OR8(cpustate,dst, src);
				STORE_RM8(modrm, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(cpustate,modrm,1);
				dst = READ8(cpustate,ea);
				src = FETCH(cpustate);
				dst = OR8(cpustate,dst, src);
				WRITE8(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_MEM);
			}
			break;
		case 2:     // ADC Rm8, i8
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM8(modrm);
				src = FETCH(cpustate);
				dst = ADC8(cpustate, dst, src, cpustate->CF);
				STORE_RM8(modrm, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(cpustate,modrm,1);
				dst = READ8(cpustate,ea);
				src = FETCH(cpustate);
				dst = ADC8(cpustate, dst, src, cpustate->CF);
				WRITE8(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_MEM);
			}
			break;
		case 3:     // SBB Rm8, i8
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM8(modrm);
				src = FETCH(cpustate);
				dst = SBB8(cpustate, dst, src, cpustate->CF);
				STORE_RM8(modrm, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(cpustate,modrm,1);
				dst = READ8(cpustate,ea);
				src = FETCH(cpustate);
				dst = SBB8(cpustate, dst, src, cpustate->CF);
				WRITE8(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_MEM);
			}
			break;
		case 4:     // AND Rm8, i8
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM8(modrm);
				src = FETCH(cpustate);
				dst = AND8(cpustate,dst, src);
				STORE_RM8(modrm, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(cpustate,modrm,1);
				dst = READ8(cpustate,ea);
				src = FETCH(cpustate);
				dst = AND8(cpustate,dst, src);
				WRITE8(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_MEM);
			}
			break;
		case 5:     // SUB Rm8, i8
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM8(modrm);
				src = FETCH(cpustate);
				dst = SUB8(cpustate,dst, src);
				STORE_RM8(modrm, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(cpustate,modrm,1);
				dst = READ8(cpustate,ea);
				src = FETCH(cpustate);
				dst = SUB8(cpustate,dst, src);
				WRITE8(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_MEM);
			}
			break;
		case 6:     // XOR Rm8, i8
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM8(modrm);
				src = FETCH(cpustate);
				dst = XOR8(cpustate,dst, src);
				STORE_RM8(modrm, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(cpustate,modrm,1);
				dst = READ8(cpustate,ea);
				src = FETCH(cpustate);
				dst = XOR8(cpustate,dst, src);
				WRITE8(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_MEM);
			}
			break;
		case 7:     // CMP Rm8, i8
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM8(modrm);
				src = FETCH(cpustate);
				SUB8(cpustate,dst, src);
				CYCLES(cpustate,CYCLES_CMP_REG_REG);
			} else {
				ea = GetEA(cpustate,modrm,0);
				dst = READ8(cpustate,ea);
				src = FETCH(cpustate);
				SUB8(cpustate,dst, src);
				CYCLES(cpustate,CYCLES_CMP_REG_MEM);
			}
			break;
	}
}

static void I386OP(groupC0_8)(i386_state *cpustate)         // Opcode 0xc0
{
	UINT8 dst;
	UINT8 modrm = FETCH(cpustate);
	UINT8 shift;

	if( modrm >= 0xc0 ) {
		dst = LOAD_RM8(modrm);
		shift = FETCH(cpustate) & 0x1f;
		dst = i386_shift_rotate8(cpustate, modrm, dst, shift);
		STORE_RM8(modrm, dst);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		dst = READ8(cpustate,ea);
		shift = FETCH(cpustate) & 0x1f;
		dst = i386_shift_rotate8(cpustate, modrm, dst, shift);
		WRITE8(cpustate,ea, dst);
	}
}

static void I386OP(groupD0_8)(i386_state *cpustate)         // Opcode 0xd0
{
	UINT8 dst;
	UINT8 modrm = FETCH(cpustate);

	if( modrm >= 0xc0 ) {
		dst = LOAD_RM8(modrm);
		dst = i386_shift_rotate8(cpustate, modrm, dst, 1);
		STORE_RM8(modrm, dst);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		dst = READ8(cpustate,ea);
		dst = i386_shift_rotate8(cpustate, modrm, dst, 1);
		WRITE8(cpustate,ea, dst);
	}
}

static void I386OP(groupD2_8)(i386_state *cpustate)         // Opcode 0xd2
{
	UINT8 dst;
	UINT8 modrm = FETCH(cpustate);

	if( modrm >= 0xc0 ) {
		dst = LOAD_RM8(modrm);
		dst = i386_shift_rotate8(cpustate, modrm, dst, REG8(CL));
		STORE_RM8(modrm, dst);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		dst = READ8(cpustate,ea);
		dst = i386_shift_rotate8(cpustate, modrm, dst, REG8(CL));
		WRITE8(cpustate,ea, dst);
	}
}

static void I386OP(groupF6_8)(i386_state *cpustate)         // Opcode 0xf6
{
	UINT8 modrm = FETCH(cpustate);

	switch( (modrm >> 3) & 0x7 )
	{
		case 0:         /* TEST Rm8, i8 */
			if( modrm >= 0xc0 ) {
				UINT8 dst = LOAD_RM8(modrm);
				UINT8 src = FETCH(cpustate);
				dst &= src;
				cpustate->CF = cpustate->OF = cpustate->AF = 0;
				SetSZPF8(dst);
				CYCLES(cpustate,CYCLES_TEST_IMM_REG);
			} else {
				UINT32 ea = GetEA(cpustate,modrm,0);
				UINT8 dst = READ8(cpustate,ea);
				UINT8 src = FETCH(cpustate);
				dst &= src;
				cpustate->CF = cpustate->OF = cpustate->AF = 0;
				SetSZPF8(dst);
				CYCLES(cpustate,CYCLES_TEST_IMM_MEM);
			}
			break;
		case 2:         /* NOT Rm8 */
			if( modrm >= 0xc0 ) {
				UINT8 dst = LOAD_RM8(modrm);
				dst = ~dst;
				STORE_RM8(modrm, dst);
				CYCLES(cpustate,CYCLES_NOT_REG);
			} else {
				UINT32 ea = GetEA(cpustate,modrm,1);
				UINT8 dst = READ8(cpustate,ea);
				dst = ~dst;
				WRITE8(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_NOT_MEM);
			}
			break;
		case 3:         /* NEG Rm8 */
			if( modrm >= 0xc0 ) {
				UINT8 dst = LOAD_RM8(modrm);
				dst = SUB8(cpustate, 0, dst );
				STORE_RM8(modrm, dst);
				CYCLES(cpustate,CYCLES_NEG_REG);
			} else {
				UINT32 ea = GetEA(cpustate,modrm,1);
				UINT8 dst = READ8(cpustate,ea);
				dst = SUB8(cpustate, 0, dst );
				WRITE8(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_NEG_MEM);
			}
			break;
		case 4:         /* MUL AL, Rm8 */
			{
				UINT16 result;
				UINT8 src, dst;
				if( modrm >= 0xc0 ) {
					src = LOAD_RM8(modrm);
					CYCLES(cpustate,CYCLES_MUL8_ACC_REG);       /* TODO: Correct multiply timing */
				} else {
					UINT32 ea = GetEA(cpustate,modrm,0);
					src = READ8(cpustate,ea);
					CYCLES(cpustate,CYCLES_MUL8_ACC_MEM);       /* TODO: Correct multiply timing */
				}

				dst = REG8(AL);
				result = (UINT16)src * (UINT16)dst;
				REG16(AX) = (UINT16)result;

				cpustate->CF = cpustate->OF = (REG16(AX) > 0xff);
			}
			break;
		case 5:         /* IMUL AL, Rm8 */
			{
				INT16 result;
				INT16 src, dst;
				if( modrm >= 0xc0 ) {
					src = (INT16)(INT8)LOAD_RM8(modrm);
					CYCLES(cpustate,CYCLES_IMUL8_ACC_REG);      /* TODO: Correct multiply timing */
				} else {
					UINT32 ea = GetEA(cpustate,modrm,0);
					src = (INT16)(INT8)READ8(cpustate,ea);
					CYCLES(cpustate,CYCLES_IMUL8_ACC_MEM);      /* TODO: Correct multiply timing */
				}

				dst = (INT16)(INT8)REG8(AL);
				result = src * dst;

				REG16(AX) = (UINT16)result;

				cpustate->CF = cpustate->OF = !(result == (INT16)(INT8)result);
			}
			break;
		case 6:         /* DIV AL, Rm8 */
			{
				UINT16 quotient, remainder, result;
				UINT8 src;
				if( modrm >= 0xc0 ) {
					src = LOAD_RM8(modrm);
					CYCLES(cpustate,CYCLES_DIV8_ACC_REG);
				} else {
					UINT32 ea = GetEA(cpustate,modrm,0);
					src = READ8(cpustate,ea);
					CYCLES(cpustate,CYCLES_DIV8_ACC_MEM);
				}

				quotient = (UINT16)REG16(AX);
				if( src ) {
					remainder = quotient % (UINT16)src;
					result = quotient / (UINT16)src;
					if( result > 0xff ) {
						/* TODO: Divide error */
					} else {
						REG8(AH) = (UINT8)remainder & 0xff;
						REG8(AL) = (UINT8)result & 0xff;

						// this flag is actually undefined, enable on non-cyrix
						if (cpustate->cpuid_id0 != 0x69727943)
							cpustate->CF = 1;
					}
				} else {
					i386_trap(cpustate, 0, 0, 0);
				}
			}
			break;
		case 7:         /* IDIV AL, Rm8 */
			{
				INT16 quotient, remainder, result;
				UINT8 src;
				if( modrm >= 0xc0 ) {
					src = LOAD_RM8(modrm);
					CYCLES(cpustate,CYCLES_IDIV8_ACC_REG);
				} else {
					UINT32 ea = GetEA(cpustate,modrm,0);
					src = READ8(cpustate,ea);
					CYCLES(cpustate,CYCLES_IDIV8_ACC_MEM);
				}

				quotient = (INT16)REG16(AX);
				if( src ) {
					remainder = quotient % (INT16)(INT8)src;
					result = quotient / (INT16)(INT8)src;
					if( result > 0xff ) {
						/* TODO: Divide error */
					} else {
						REG8(AH) = (UINT8)remainder & 0xff;
						REG8(AL) = (UINT8)result & 0xff;

						// this flag is actually undefined, enable on non-cyrix
						if (cpustate->cpuid_id0 != 0x69727943)
							cpustate->CF = 1;
					}
				} else {
					i386_trap(cpustate, 0, 0, 0);
				}
			}
			break;
	}
}

static void I386OP(groupFE_8)(i386_state *cpustate)         // Opcode 0xfe
{
	UINT8 modrm = FETCH(cpustate);

	switch( (modrm >> 3) & 0x7 )
	{
		case 0:         /* INC Rm8 */
			if( modrm >= 0xc0 ) {
				UINT8 dst = LOAD_RM8(modrm);
				dst = INC8(cpustate,dst);
				STORE_RM8(modrm, dst);
				CYCLES(cpustate,CYCLES_INC_REG);
			} else {
				UINT32 ea = GetEA(cpustate,modrm,1);
				UINT8 dst = READ8(cpustate,ea);
				dst = INC8(cpustate,dst);
				WRITE8(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_INC_MEM);
			}
			break;
		case 1:         /* DEC Rm8 */
			if( modrm >= 0xc0 ) {
				UINT8 dst = LOAD_RM8(modrm);
				dst = DEC8(cpustate,dst);
				STORE_RM8(modrm, dst);
				CYCLES(cpustate,CYCLES_DEC_REG);
			} else {
				UINT32 ea = GetEA(cpustate,modrm,1);
				UINT8 dst = READ8(cpustate,ea);
				dst = DEC8(cpustate,dst);
				WRITE8(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_DEC_MEM);
			}
			break;
		case 6:         /* PUSH Rm8 */
			{
				UINT8 value;
				if( modrm >= 0xc0 ) {
					value = LOAD_RM8(modrm);
				} else {
					UINT32 ea = GetEA(cpustate,modrm,0);
					value = READ8(cpustate,ea);
				}
				if( cpustate->operand_size ) {
					PUSH32(cpustate,value);
				} else {
					PUSH16(cpustate,value);
				}
				CYCLES(cpustate,CYCLES_PUSH_RM);
			}
			break;
		default:
			fatalerror("i386: groupFE_8 /%d unimplemented\n", (modrm >> 3) & 0x7);
			break;
	}
}



static void I386OP(segment_CS)(i386_state *cpustate)        // Opcode 0x2e
{
	cpustate->segment_prefix = 1;
	cpustate->segment_override = CS;

	I386OP(decode_opcode)(cpustate);
}

static void I386OP(segment_DS)(i386_state *cpustate)        // Opcode 0x3e
{
	cpustate->segment_prefix = 1;
	cpustate->segment_override = DS;
	CYCLES(cpustate,0); // TODO: Specify cycle count
	I386OP(decode_opcode)(cpustate);
}

static void I386OP(segment_ES)(i386_state *cpustate)        // Opcode 0x26
{
	cpustate->segment_prefix = 1;
	cpustate->segment_override = ES;
	CYCLES(cpustate,0); // TODO: Specify cycle count
	I386OP(decode_opcode)(cpustate);
}

static void I386OP(segment_FS)(i386_state *cpustate)        // Opcode 0x64
{
	cpustate->segment_prefix = 1;
	cpustate->segment_override = FS;
	CYCLES(cpustate,1); // TODO: Specify cycle count
	I386OP(decode_opcode)(cpustate);
}

static void I386OP(segment_GS)(i386_state *cpustate)        // Opcode 0x65
{
	cpustate->segment_prefix = 1;
	cpustate->segment_override = GS;
	CYCLES(cpustate,1); // TODO: Specify cycle count
	I386OP(decode_opcode)(cpustate);
}

static void I386OP(segment_SS)(i386_state *cpustate)        // Opcode 0x36
{
	cpustate->segment_prefix = 1;
	cpustate->segment_override = SS;
	CYCLES(cpustate,0); // TODO: Specify cycle count
	I386OP(decode_opcode)(cpustate);
}

static void I386OP(operand_size)(i386_state *cpustate)      // Opcode prefix 0x66
{
	if(cpustate->operand_prefix == 0)
	{
		cpustate->operand_size ^= 1;
		cpustate->operand_prefix = 1;
	}
	cpustate->opcode = FETCH(cpustate);
	if (cpustate->opcode == 0x0f)
		I386OP(decode_three_byte66)(cpustate);
	else
	{
		if( cpustate->operand_size )
			cpustate->opcode_table1_32[cpustate->opcode](cpustate);
		else
			cpustate->opcode_table1_16[cpustate->opcode](cpustate);
	}
}

static void I386OP(address_size)(i386_state *cpustate)      // Opcode 0x67
{
	if(cpustate->address_prefix == 0)
	{
		cpustate->address_size ^= 1;
		cpustate->address_prefix = 1;
	}
	I386OP(decode_opcode)(cpustate);
}

static void I386OP(nop)(i386_state *cpustate)               // Opcode 0x90
{
	CYCLES(cpustate,CYCLES_NOP);
}

static void I386OP(int3)(i386_state *cpustate)              // Opcode 0xcc
{
	CYCLES(cpustate,CYCLES_INT3);
	cpustate->ext = 0; // not an external interrupt
	i386_trap(cpustate,3, 1, 0);
	cpustate->ext = 1;
}

static void I386OP(int)(i386_state *cpustate)               // Opcode 0xcd
{
	int interrupt = FETCH(cpustate);
	CYCLES(cpustate,CYCLES_INT);
	cpustate->ext = 0; // not an external interrupt
	i386_trap(cpustate,interrupt, 1, 0);
	cpustate->ext = 1;
}

static void I386OP(into)(i386_state *cpustate)              // Opcode 0xce
{
	if( cpustate->OF ) {
		cpustate->ext = 0;
		i386_trap(cpustate,4, 1, 0);
		cpustate->ext = 1;
		CYCLES(cpustate,CYCLES_INTO_OF1);
	}
	else
	{
		CYCLES(cpustate,CYCLES_INTO_OF0);
	}
}

static UINT32 i386_escape_ea;   // hack around GCC 4.6 error because we need the side effects of GetEA()
static void I386OP(escape)(i386_state *cpustate)            // Opcodes 0xd8 - 0xdf
{
	UINT8 modrm = FETCH(cpustate);
	if(modrm < 0xc0)
	{
		i386_escape_ea = GetEA(cpustate,modrm,0);
	}
	CYCLES(cpustate,3); // TODO: confirm this
	(void) LOAD_RM8(modrm);
}

static void I386OP(hlt)(i386_state *cpustate)               // Opcode 0xf4
{
	if(PROTECTED_MODE && cpustate->CPL != 0)
		FAULT(FAULT_GP,0);
	cpustate->halted = 1;
	CYCLES(cpustate,CYCLES_HLT);
	if (cpustate->cycles > 0)
		cpustate->cycles = 0;
}

static void I386OP(decimal_adjust)(i386_state *cpustate, int direction)
{
	UINT8 tmpAL = REG8(AL);
	UINT8 tmpCF = cpustate->CF;

	if (cpustate->AF || ((REG8(AL) & 0xf) > 9))
	{
		UINT16 t= (UINT16)REG8(AL) + (direction * 0x06);
		REG8(AL) = (UINT8)t&0xff;
		cpustate->AF = 1;
		if (t & 0x100)
			cpustate->CF = 1;
		if (direction > 0)
			tmpAL = REG8(AL);
	}

	if (tmpCF || (tmpAL > 0x99))
	{
		REG8(AL) += (direction * 0x60);
		cpustate->CF = 1;
	}

	SetSZPF8(REG8(AL));
}

static void I386OP(daa)(i386_state *cpustate)               // Opcode 0x27
{
	I386OP(decimal_adjust)(cpustate, +1);
	CYCLES(cpustate,CYCLES_DAA);
}

static void I386OP(das)(i386_state *cpustate)               // Opcode 0x2f
{
	I386OP(decimal_adjust)(cpustate, -1);
	CYCLES(cpustate,CYCLES_DAS);
}

static void I386OP(aaa)(i386_state *cpustate)               // Opcode 0x37
{
	if( ( (REG8(AL) & 0x0f) > 9) || (cpustate->AF != 0) ) {
		REG16(AX) = REG16(AX) + 6;
		REG8(AH) = REG8(AH) + 1;
		cpustate->AF = 1;
		cpustate->CF = 1;
	} else {
		cpustate->AF = 0;
		cpustate->CF = 0;
	}
	REG8(AL) = REG8(AL) & 0x0f;
	CYCLES(cpustate,CYCLES_AAA);
}

static void I386OP(aas)(i386_state *cpustate)               // Opcode 0x3f
{
	if (cpustate->AF || ((REG8(AL) & 0xf) > 9))
	{
		REG16(AX) -= 6;
		REG8(AH) -= 1;
		cpustate->AF = 1;
		cpustate->CF = 1;
	}
	else
	{
		cpustate->AF = 0;
		cpustate->CF = 0;
	}
	REG8(AL) &= 0x0f;
	CYCLES(cpustate,CYCLES_AAS);
}

static void I386OP(aad)(i386_state *cpustate)               // Opcode 0xd5
{
	UINT8 tempAL = REG8(AL);
	UINT8 tempAH = REG8(AH);
	UINT8 i = FETCH(cpustate);

	REG8(AL) = (tempAL + (tempAH * i)) & 0xff;
	REG8(AH) = 0;
	SetSZPF8( REG8(AL) );
	CYCLES(cpustate,CYCLES_AAD);
}

static void I386OP(aam)(i386_state *cpustate)               // Opcode 0xd4
{
	UINT8 tempAL = REG8(AL);
	UINT8 i = FETCH(cpustate);

	REG8(AH) = tempAL / i;
	REG8(AL) = tempAL % i;
	SetSZPF8( REG8(AL) );
	CYCLES(cpustate,CYCLES_AAM);
}

static void I386OP(clts)(i386_state *cpustate)              // Opcode 0x0f 0x06
{
	// Privileged instruction, CPL must be zero.  Can be used in real or v86 mode.
	if(PROTECTED_MODE && cpustate->CPL != 0)
		FAULT(FAULT_GP,0)
	cpustate->cr[0] &= ~0x08;   /* clear TS bit */
	CYCLES(cpustate,CYCLES_CLTS);
}

static void I386OP(wait)(i386_state *cpustate)              // Opcode 0x9B
{
	// TODO
}

static void I386OP(lock)(i386_state *cpustate)              // Opcode 0xf0
{
	// lock doesn't depend on iopl on 386
	// TODO: lock causes UD on unlockable opcodes
	CYCLES(cpustate,CYCLES_LOCK);       // TODO: Determine correct cycle count
	I386OP(decode_opcode)(cpustate);
}

static void I386OP(mov_r32_tr)(i386_state *cpustate)        // Opcode 0x0f 24
{
	FETCH(cpustate);
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void I386OP(mov_tr_r32)(i386_state *cpustate)        // Opcode 0x0f 26
{
	FETCH(cpustate);
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void I386OP(loadall)(i386_state *cpustate)       // Opcode 0x0f 0x07 (0x0f 0x05 on 80286), undocumented
{
	popmessage("LOADALL instruction hit!");
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void I386OP(unimplemented)(i386_state *cpustate)
{
	report_unimplemented_opcode(cpustate);
}

static void I386OP(invalid)(i386_state *cpustate)
{
	report_invalid_opcode(cpustate);
	i386_trap(cpustate, 6, 0, 0);
}

static void I386OP(xlat)(i386_state *cpustate)          // Opcode 0xd7
{
	UINT32 ea;
	if( cpustate->segment_prefix ) {
		if(!cpustate->address_size)
		{
			ea = i386_translate(cpustate, cpustate->segment_override, REG16(BX) + REG8(AL), 0 );
		}
		else
		{
			ea = i386_translate(cpustate, cpustate->segment_override, REG32(EBX) + REG8(AL), 0 );
		}
	} else {
		if(!cpustate->address_size)
		{
			ea = i386_translate(cpustate, DS, REG16(BX) + REG8(AL), 0 );
		}
		else
		{
			ea = i386_translate(cpustate, DS, REG32(EBX) + REG8(AL), 0 );
		}
	}
	REG8(AL) = READ8(cpustate,ea);
	CYCLES(cpustate,CYCLES_XLAT);
}
