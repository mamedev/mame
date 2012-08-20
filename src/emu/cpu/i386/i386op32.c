static UINT32 I386OP(shift_rotate32)(i386_state *cpustate, UINT8 modrm, UINT32 value, UINT8 shift)
{
	UINT32 dst, src;
	dst = value;
	src = value;

	if( shift == 0 ) {
		CYCLES_RM(cpustate,modrm, 3, 7);
	} else if( shift == 1 ) {

		switch( (modrm >> 3) & 0x7 )
		{
			case 0:			/* ROL rm32, 1 */
				cpustate->CF = (src & 0x80000000) ? 1 : 0;
				dst = (src << 1) + cpustate->CF;
				cpustate->OF = ((src ^ dst) & 0x80000000) ? 1 : 0;
				CYCLES_RM(cpustate,modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 1:			/* ROR rm32, 1 */
				cpustate->CF = (src & 0x1) ? 1 : 0;
				dst = (cpustate->CF << 31) | (src >> 1);
				cpustate->OF = ((src ^ dst) & 0x80000000) ? 1 : 0;
				CYCLES_RM(cpustate,modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 2:			/* RCL rm32, 1 */
				dst = (src << 1) + cpustate->CF;
				cpustate->CF = (src & 0x80000000) ? 1 : 0;
				cpustate->OF = ((src ^ dst) & 0x80000000) ? 1 : 0;
				CYCLES_RM(cpustate,modrm, CYCLES_ROTATE_CARRY_REG, CYCLES_ROTATE_CARRY_MEM);
				break;
			case 3:			/* RCR rm32, 1 */
				dst = (cpustate->CF << 31) | (src >> 1);
				cpustate->CF = src & 0x1;
				cpustate->OF = ((src ^ dst) & 0x80000000) ? 1 : 0;
				CYCLES_RM(cpustate,modrm, CYCLES_ROTATE_CARRY_REG, CYCLES_ROTATE_CARRY_MEM);
				break;
			case 4:			/* SHL/SAL rm32, 1 */
			case 6:
				dst = src << 1;
				cpustate->CF = (src & 0x80000000) ? 1 : 0;
				cpustate->OF = (((cpustate->CF << 31) ^ dst) & 0x80000000) ? 1 : 0;
				SetSZPF32(dst);
				CYCLES_RM(cpustate,modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 5:			/* SHR rm32, 1 */
				dst = src >> 1;
				cpustate->CF = src & 0x1;
				cpustate->OF = (src & 0x80000000) ? 1 : 0;
				SetSZPF32(dst);
				CYCLES_RM(cpustate,modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 7:			/* SAR rm32, 1 */
				dst = (INT32)(src) >> 1;
				cpustate->CF = src & 0x1;
				cpustate->OF = 0;
				SetSZPF32(dst);
				CYCLES_RM(cpustate,modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
		}

	} else {
		shift &= 31;
		switch( (modrm >> 3) & 0x7 )
		{
			case 0:			/* ROL rm32, i8 */
				dst = ((src & ((UINT32)0xffffffff >> shift)) << shift) |
					  ((src & ((UINT32)0xffffffff << (32-shift))) >> (32-shift));
				cpustate->CF = dst & 0x1;
				cpustate->OF = (dst & 1) ^ (dst >> 31);
				CYCLES_RM(cpustate,modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 1:			/* ROR rm32, i8 */
				dst = ((src & ((UINT32)0xffffffff << shift)) >> shift) |
					  ((src & ((UINT32)0xffffffff >> (32-shift))) << (32-shift));
				cpustate->CF = (dst >> 31) & 0x1;
				cpustate->OF = ((dst >> 31) ^ (dst >> 30)) & 1;
				CYCLES_RM(cpustate,modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 2:			/* RCL rm32, i8 */
				dst = ((src & ((UINT32)0xffffffff >> shift)) << shift) |
					  ((src & ((UINT32)0xffffffff << (33-shift))) >> (33-shift)) |
					  (cpustate->CF << (shift-1));
				cpustate->CF = (src >> (32-shift)) & 0x1;
				cpustate->OF = cpustate->CF ^ ((dst >> 31) & 1);
				CYCLES_RM(cpustate,modrm, CYCLES_ROTATE_CARRY_REG, CYCLES_ROTATE_CARRY_MEM);
				break;
			case 3:			/* RCR rm32, i8 */
				dst = ((src & ((UINT32)0xffffffff << shift)) >> shift) |
					  ((src & ((UINT32)0xffffffff >> (32-shift))) << (33-shift)) |
					  (cpustate->CF << (32-shift));
				cpustate->CF = (src >> (shift-1)) & 0x1;
				cpustate->OF = ((dst >> 31) ^ (dst >> 30)) & 1;
				CYCLES_RM(cpustate,modrm, CYCLES_ROTATE_CARRY_REG, CYCLES_ROTATE_CARRY_MEM);
				break;
			case 4:			/* SHL/SAL rm32, i8 */
			case 6:
				dst = src << shift;
				cpustate->CF = (src & (1 << (32-shift))) ? 1 : 0;
				SetSZPF32(dst);
				CYCLES_RM(cpustate,modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 5:			/* SHR rm32, i8 */
				dst = src >> shift;
				cpustate->CF = (src & (1 << (shift-1))) ? 1 : 0;
				SetSZPF32(dst);
				CYCLES_RM(cpustate,modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 7:			/* SAR rm32, i8 */
				dst = (INT32)src >> shift;
				cpustate->CF = (src & (1 << (shift-1))) ? 1 : 0;
				SetSZPF32(dst);
				CYCLES_RM(cpustate,modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
		}

	}
	return dst;
}



static void I386OP(adc_rm32_r32)(i386_state *cpustate)		// Opcode 0x11
{
	UINT32 src, dst;
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_REG32(modrm);
		dst = LOAD_RM32(modrm);
		dst = ADC32(cpustate, dst, src, cpustate->CF);
		STORE_RM32(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		src = LOAD_REG32(modrm);
		dst = READ32(cpustate,ea);
		dst = ADC32(cpustate, dst, src, cpustate->CF);
		WRITE32(cpustate,ea, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_MEM);
	}
}

static void I386OP(adc_r32_rm32)(i386_state *cpustate)		// Opcode 0x13
{
	UINT32 src, dst;
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_RM32(modrm);
		dst = LOAD_REG32(modrm);
		dst = ADC32(cpustate, dst, src, cpustate->CF);
		STORE_REG32(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,0);
		src = READ32(cpustate,ea);
		dst = LOAD_REG32(modrm);
		dst = ADC32(cpustate, dst, src, cpustate->CF);
		STORE_REG32(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_MEM_REG);
	}
}

static void I386OP(adc_eax_i32)(i386_state *cpustate)		// Opcode 0x15
{
	UINT32 src, dst;
	src = FETCH32(cpustate);
	dst = REG32(EAX);
	dst = ADC32(cpustate, dst, src, cpustate->CF);
	REG32(EAX) = dst;
	CYCLES(cpustate,CYCLES_ALU_IMM_ACC);
}

static void I386OP(add_rm32_r32)(i386_state *cpustate)		// Opcode 0x01
{
	UINT32 src, dst;
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_REG32(modrm);
		dst = LOAD_RM32(modrm);
		dst = ADD32(cpustate,dst, src);
		STORE_RM32(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		src = LOAD_REG32(modrm);
		dst = READ32(cpustate,ea);
		dst = ADD32(cpustate,dst, src);
		WRITE32(cpustate,ea, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_MEM);
	}
}

static void I386OP(add_r32_rm32)(i386_state *cpustate)		// Opcode 0x03
{
	UINT32 src, dst;
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_RM32(modrm);
		dst = LOAD_REG32(modrm);
		dst = ADD32(cpustate,dst, src);
		STORE_REG32(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,0);
		src = READ32(cpustate,ea);
		dst = LOAD_REG32(modrm);
		dst = ADD32(cpustate,dst, src);
		STORE_REG32(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_MEM_REG);
	}
}

static void I386OP(add_eax_i32)(i386_state *cpustate)		// Opcode 0x05
{
	UINT32 src, dst;
	src = FETCH32(cpustate);
	dst = REG32(EAX);
	dst = ADD32(cpustate,dst, src);
	REG32(EAX) = dst;
	CYCLES(cpustate,CYCLES_ALU_IMM_ACC);
}

static void I386OP(and_rm32_r32)(i386_state *cpustate)		// Opcode 0x21
{
	UINT32 src, dst;
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_REG32(modrm);
		dst = LOAD_RM32(modrm);
		dst = AND32(cpustate,dst, src);
		STORE_RM32(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		src = LOAD_REG32(modrm);
		dst = READ32(cpustate,ea);
		dst = AND32(cpustate,dst, src);
		WRITE32(cpustate,ea, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_MEM);
	}
}

static void I386OP(and_r32_rm32)(i386_state *cpustate)		// Opcode 0x23
{
	UINT32 src, dst;
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_RM32(modrm);
		dst = LOAD_REG32(modrm);
		dst = AND32(cpustate,dst, src);
		STORE_REG32(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,0);
		src = READ32(cpustate,ea);
		dst = LOAD_REG32(modrm);
		dst = AND32(cpustate,dst, src);
		STORE_REG32(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_MEM_REG);
	}
}

static void I386OP(and_eax_i32)(i386_state *cpustate)		// Opcode 0x25
{
	UINT32 src, dst;
	src = FETCH32(cpustate);
	dst = REG32(EAX);
	dst = AND32(cpustate,dst, src);
	REG32(EAX) = dst;
	CYCLES(cpustate,CYCLES_ALU_IMM_ACC);
}

static void I386OP(bsf_r32_rm32)(i386_state *cpustate)		// Opcode 0x0f bc
{
	UINT32 src, dst, temp;
	UINT8 modrm = FETCH(cpustate);

	if( modrm >= 0xc0 ) {
		src = LOAD_RM32(modrm);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,0);
		src = READ32(cpustate,ea);
	}

	dst = 0;

	if( src == 0 ) {
		cpustate->ZF = 1;
	} else {
		cpustate->ZF = 0;
		temp = 0;
		while( (src & (1 << temp)) == 0 ) {
			temp++;
			dst = temp;
			CYCLES(cpustate,CYCLES_BSF);
		}
		STORE_REG32(modrm, dst);
	}
	CYCLES(cpustate,CYCLES_BSF_BASE);
}

static void I386OP(bsr_r32_rm32)(i386_state *cpustate)		// Opcode 0x0f bd
{
	UINT32 src, dst, temp;
	UINT8 modrm = FETCH(cpustate);

	if( modrm >= 0xc0 ) {
		src = LOAD_RM32(modrm);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,0);
		src = READ32(cpustate,ea);
	}

	dst = 0;

	if( src == 0 ) {
		cpustate->ZF = 1;
	} else {
		cpustate->ZF = 0;
		dst = temp = 31;
		while( (src & (1 << temp)) == 0 ) {
			temp--;
			dst = temp;
			CYCLES(cpustate,CYCLES_BSR);
		}
		STORE_REG32(modrm, dst);
	}
	CYCLES(cpustate,CYCLES_BSR_BASE);
}

static void I386OP(bt_rm32_r32)(i386_state *cpustate)		// Opcode 0x0f a3
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		UINT32 dst = LOAD_RM32(modrm);
		UINT32 bit = LOAD_REG32(modrm);

		if( dst & (1 << bit) )
			cpustate->CF = 1;
		else
			cpustate->CF = 0;

		CYCLES(cpustate,CYCLES_BT_REG_REG);
	} else {
		UINT8 segment;
		UINT32 ea = GetNonTranslatedEA(cpustate,modrm,&segment);
		UINT32 bit = LOAD_REG32(modrm);
		ea += 4*(bit/32);
		ea = i386_translate(cpustate,segment,(cpustate->address_size)?ea:(ea&0xffff),0);
		bit %= 32;
		UINT32 dst = READ32(cpustate,ea);

		if( dst & (1 << bit) )
			cpustate->CF = 1;
		else
			cpustate->CF = 0;

		CYCLES(cpustate,CYCLES_BT_REG_MEM);
	}
}

static void I386OP(btc_rm32_r32)(i386_state *cpustate)		// Opcode 0x0f bb
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		UINT32 dst = LOAD_RM32(modrm);
		UINT32 bit = LOAD_REG32(modrm);

		if( dst & (1 << bit) )
			cpustate->CF = 1;
		else
			cpustate->CF = 0;
		dst ^= (1 << bit);

		STORE_RM32(modrm, dst);
		CYCLES(cpustate,CYCLES_BTC_REG_REG);
	} else {
		UINT8 segment;
		UINT32 ea = GetNonTranslatedEA(cpustate,modrm,&segment);
		UINT32 bit = LOAD_REG32(modrm);
		ea += 4*(bit/32);
		ea = i386_translate(cpustate,segment,(cpustate->address_size)?ea:(ea&0xffff),1);
		bit %= 32;
		UINT32 dst = READ32(cpustate,ea);

		if( dst & (1 << bit) )
			cpustate->CF = 1;
		else
			cpustate->CF = 0;
		dst ^= (1 << bit);

		WRITE32(cpustate,ea, dst);
		CYCLES(cpustate,CYCLES_BTC_REG_MEM);
	}
}

static void I386OP(btr_rm32_r32)(i386_state *cpustate)		// Opcode 0x0f b3
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		UINT32 dst = LOAD_RM32(modrm);
		UINT32 bit = LOAD_REG32(modrm);

		if( dst & (1 << bit) )
			cpustate->CF = 1;
		else
			cpustate->CF = 0;
		dst &= ~(1 << bit);

		STORE_RM32(modrm, dst);
		CYCLES(cpustate,CYCLES_BTR_REG_REG);
	} else {
		UINT8 segment;
		UINT32 ea = GetNonTranslatedEA(cpustate,modrm,&segment);
		UINT32 bit = LOAD_REG32(modrm);
		ea += 4*(bit/32);
		ea = i386_translate(cpustate,segment,(cpustate->address_size)?ea:(ea&0xffff),1);
		bit %= 32;
		UINT32 dst = READ32(cpustate,ea);

		if( dst & (1 << bit) )
			cpustate->CF = 1;
		else
			cpustate->CF = 0;
		dst &= ~(1 << bit);

		WRITE32(cpustate,ea, dst);
		CYCLES(cpustate,CYCLES_BTR_REG_MEM);
	}
}

static void I386OP(bts_rm32_r32)(i386_state *cpustate)		// Opcode 0x0f ab
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		UINT32 dst = LOAD_RM32(modrm);
		UINT32 bit = LOAD_REG32(modrm);

		if( dst & (1 << bit) )
			cpustate->CF = 1;
		else
			cpustate->CF = 0;
		dst |= (1 << bit);

		STORE_RM32(modrm, dst);
		CYCLES(cpustate,CYCLES_BTS_REG_REG);
	} else {
		UINT8 segment;
		UINT32 ea = GetNonTranslatedEA(cpustate,modrm,&segment);
		UINT32 bit = LOAD_REG32(modrm);
		ea += 4*(bit/32);
		ea = i386_translate(cpustate,segment,(cpustate->address_size)?ea:(ea&0xffff),1);
		bit %= 32;
		UINT32 dst = READ32(cpustate,ea);

		if( dst & (1 << bit) )
			cpustate->CF = 1;
		else
			cpustate->CF = 0;
		dst |= (1 << bit);

		WRITE32(cpustate,ea, dst);
		CYCLES(cpustate,CYCLES_BTS_REG_MEM);
	}
}

static void I386OP(call_abs32)(i386_state *cpustate)		// Opcode 0x9a
{
	UINT32 offset = FETCH32(cpustate);
	UINT16 ptr = FETCH16(cpustate);

	if(PROTECTED_MODE && !V8086_MODE)
	{
		i386_protected_mode_call(cpustate,ptr,offset,0,1);
	}
	else
	{
		PUSH32(cpustate, cpustate->sreg[CS].selector );
		PUSH32(cpustate, cpustate->eip );
		cpustate->sreg[CS].selector = ptr;
		cpustate->performed_intersegment_jump = 1;
		cpustate->eip = offset;
		i386_load_segment_descriptor(cpustate,CS);
	}
	CYCLES(cpustate,CYCLES_CALL_INTERSEG);
	CHANGE_PC(cpustate,cpustate->eip);
}

static void I386OP(call_rel32)(i386_state *cpustate)		// Opcode 0xe8
{
	INT32 disp = FETCH32(cpustate);
	PUSH32(cpustate, cpustate->eip );
	cpustate->eip += disp;
	CHANGE_PC(cpustate,cpustate->eip);
	CYCLES(cpustate,CYCLES_CALL);		/* TODO: Timing = 7 + m */
}

static void I386OP(cdq)(i386_state *cpustate)				// Opcode 0x99
{
	if( REG32(EAX) & 0x80000000 ) {
		REG32(EDX) = 0xffffffff;
	} else {
		REG32(EDX) = 0x00000000;
	}
	CYCLES(cpustate,CYCLES_CWD);
}

static void I386OP(cmp_rm32_r32)(i386_state *cpustate)		// Opcode 0x39
{
	UINT32 src, dst;
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_REG32(modrm);
		dst = LOAD_RM32(modrm);
		SUB32(cpustate,dst, src);
		CYCLES(cpustate,CYCLES_CMP_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,0);
		src = LOAD_REG32(modrm);
		dst = READ32(cpustate,ea);
		SUB32(cpustate,dst, src);
		CYCLES(cpustate,CYCLES_CMP_REG_MEM);
	}
}

static void I386OP(cmp_r32_rm32)(i386_state *cpustate)		// Opcode 0x3b
{
	UINT32 src, dst;
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_RM32(modrm);
		dst = LOAD_REG32(modrm);
		SUB32(cpustate,dst, src);
		CYCLES(cpustate,CYCLES_CMP_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,0);
		src = READ32(cpustate,ea);
		dst = LOAD_REG32(modrm);
		SUB32(cpustate,dst, src);
		CYCLES(cpustate,CYCLES_CMP_MEM_REG);
	}
}

static void I386OP(cmp_eax_i32)(i386_state *cpustate)		// Opcode 0x3d
{
	UINT32 src, dst;
	src = FETCH32(cpustate);
	dst = REG32(EAX);
	SUB32(cpustate,dst, src);
	CYCLES(cpustate,CYCLES_CMP_IMM_ACC);
}

static void I386OP(cmpsd)(i386_state *cpustate)				// Opcode 0xa7
{
	UINT32 eas, ead, src, dst;
	if( cpustate->segment_prefix ) {
		eas = i386_translate(cpustate, cpustate->segment_override, cpustate->address_size ? REG32(ESI) : REG16(SI), 0 );
	} else {
		eas = i386_translate(cpustate, DS, cpustate->address_size ? REG32(ESI) : REG16(SI), 0 );
	}
	ead = i386_translate(cpustate, ES, cpustate->address_size ? REG32(EDI) : REG16(DI), 0 );
	src = READ32(cpustate,eas);
	dst = READ32(cpustate,ead);
	SUB32(cpustate,src,dst);
	BUMP_SI(cpustate,4);
	BUMP_DI(cpustate,4);
	CYCLES(cpustate,CYCLES_CMPS);
}

static void I386OP(cwde)(i386_state *cpustate)				// Opcode 0x98
{
	REG32(EAX) = (INT32)((INT16)REG16(AX));
	CYCLES(cpustate,CYCLES_CBW);
}

static void I386OP(dec_eax)(i386_state *cpustate)			// Opcode 0x48
{
	REG32(EAX) = DEC32(cpustate, REG32(EAX) );
	CYCLES(cpustate,CYCLES_DEC_REG);
}

static void I386OP(dec_ecx)(i386_state *cpustate)			// Opcode 0x49
{
	REG32(ECX) = DEC32(cpustate, REG32(ECX) );
	CYCLES(cpustate,CYCLES_DEC_REG);
}

static void I386OP(dec_edx)(i386_state *cpustate)			// Opcode 0x4a
{
	REG32(EDX) = DEC32(cpustate, REG32(EDX) );
	CYCLES(cpustate,CYCLES_DEC_REG);
}

static void I386OP(dec_ebx)(i386_state *cpustate)			// Opcode 0x4b
{
	REG32(EBX) = DEC32(cpustate, REG32(EBX) );
	CYCLES(cpustate,CYCLES_DEC_REG);
}

static void I386OP(dec_esp)(i386_state *cpustate)			// Opcode 0x4c
{
	REG32(ESP) = DEC32(cpustate, REG32(ESP) );
	CYCLES(cpustate,CYCLES_DEC_REG);
}

static void I386OP(dec_ebp)(i386_state *cpustate)			// Opcode 0x4d
{
	REG32(EBP) = DEC32(cpustate, REG32(EBP) );
	CYCLES(cpustate,CYCLES_DEC_REG);
}

static void I386OP(dec_esi)(i386_state *cpustate)			// Opcode 0x4e
{
	REG32(ESI) = DEC32(cpustate, REG32(ESI) );
	CYCLES(cpustate,CYCLES_DEC_REG);
}

static void I386OP(dec_edi)(i386_state *cpustate)			// Opcode 0x4f
{
	REG32(EDI) = DEC32(cpustate, REG32(EDI) );
	CYCLES(cpustate,CYCLES_DEC_REG);
}

static void I386OP(imul_r32_rm32)(i386_state *cpustate)		// Opcode 0x0f af
{
	UINT8 modrm = FETCH(cpustate);
	INT64 result;
	INT64 src, dst;
	if( modrm >= 0xc0 ) {
		src = (INT64)(INT32)LOAD_RM32(modrm);
		CYCLES(cpustate,CYCLES_IMUL32_REG_REG);		/* TODO: Correct multiply timing */
	} else {
		UINT32 ea = GetEA(cpustate,modrm,0);
		src = (INT64)(INT32)READ32(cpustate,ea);
		CYCLES(cpustate,CYCLES_IMUL32_REG_REG);		/* TODO: Correct multiply timing */
	}

	dst = (INT64)(INT32)LOAD_REG32(modrm);
	result = src * dst;

	STORE_REG32(modrm, (UINT32)result);

	cpustate->CF = cpustate->OF = !(result == (INT64)(INT32)result);
}

static void I386OP(imul_r32_rm32_i32)(i386_state *cpustate)	// Opcode 0x69
{
	UINT8 modrm = FETCH(cpustate);
	INT64 result;
	INT64 src, dst;
	if( modrm >= 0xc0 ) {
		dst = (INT64)(INT32)LOAD_RM32(modrm);
		CYCLES(cpustate,CYCLES_IMUL32_REG_IMM_REG);		/* TODO: Correct multiply timing */
	} else {
		UINT32 ea = GetEA(cpustate,modrm,0);
		dst = (INT64)(INT32)READ32(cpustate,ea);
		CYCLES(cpustate,CYCLES_IMUL32_MEM_IMM_REG);		/* TODO: Correct multiply timing */
	}

	src = (INT64)(INT32)FETCH32(cpustate);
	result = src * dst;

	STORE_REG32(modrm, (UINT32)result);

	cpustate->CF = cpustate->OF = !(result == (INT64)(INT32)result);
}

static void I386OP(imul_r32_rm32_i8)(i386_state *cpustate)	// Opcode 0x6b
{
	UINT8 modrm = FETCH(cpustate);
	INT64 result;
	INT64 src, dst;
	if( modrm >= 0xc0 ) {
		dst = (INT64)(INT32)LOAD_RM32(modrm);
		CYCLES(cpustate,CYCLES_IMUL32_REG_IMM_REG);		/* TODO: Correct multiply timing */
	} else {
		UINT32 ea = GetEA(cpustate,modrm,0);
		dst = (INT64)(INT32)READ32(cpustate,ea);
		CYCLES(cpustate,CYCLES_IMUL32_MEM_IMM_REG);		/* TODO: Correct multiply timing */
	}

	src = (INT64)(INT8)FETCH(cpustate);
	result = src * dst;

	STORE_REG32(modrm, (UINT32)result);

	cpustate->CF = cpustate->OF = !(result == (INT64)(INT32)result);
}

static void I386OP(in_eax_i8)(i386_state *cpustate)			// Opcode 0xe5
{
	UINT16 port = FETCH(cpustate);
	UINT32 data = READPORT32(cpustate, port);
	REG32(EAX) = data;
	CYCLES(cpustate,CYCLES_IN_VAR);
}

static void I386OP(in_eax_dx)(i386_state *cpustate)			// Opcode 0xed
{
	UINT16 port = REG16(DX);
	UINT32 data = READPORT32(cpustate, port);
	REG32(EAX) = data;
	CYCLES(cpustate,CYCLES_IN);
}

static void I386OP(inc_eax)(i386_state *cpustate)			// Opcode 0x40
{
	REG32(EAX) = INC32(cpustate, REG32(EAX) );
	CYCLES(cpustate,CYCLES_INC_REG);
}

static void I386OP(inc_ecx)(i386_state *cpustate)			// Opcode 0x41
{
	REG32(ECX) = INC32(cpustate, REG32(ECX) );
	CYCLES(cpustate,CYCLES_INC_REG);
}

static void I386OP(inc_edx)(i386_state *cpustate)			// Opcode 0x42
{
	REG32(EDX) = INC32(cpustate, REG32(EDX) );
	CYCLES(cpustate,CYCLES_INC_REG);
}

static void I386OP(inc_ebx)(i386_state *cpustate)			// Opcode 0x43
{
	REG32(EBX) = INC32(cpustate, REG32(EBX) );
	CYCLES(cpustate,CYCLES_INC_REG);
}

static void I386OP(inc_esp)(i386_state *cpustate)			// Opcode 0x44
{
	REG32(ESP) = INC32(cpustate, REG32(ESP) );
	CYCLES(cpustate,CYCLES_INC_REG);
}

static void I386OP(inc_ebp)(i386_state *cpustate)			// Opcode 0x45
{
	REG32(EBP) = INC32(cpustate, REG32(EBP) );
	CYCLES(cpustate,CYCLES_INC_REG);
}

static void I386OP(inc_esi)(i386_state *cpustate)			// Opcode 0x46
{
	REG32(ESI) = INC32(cpustate, REG32(ESI) );
	CYCLES(cpustate,CYCLES_INC_REG);
}

static void I386OP(inc_edi)(i386_state *cpustate)			// Opcode 0x47
{
	REG32(EDI) = INC32(cpustate, REG32(EDI) );
	CYCLES(cpustate,CYCLES_INC_REG);
}

static void I386OP(iret32)(i386_state *cpustate)			// Opcode 0xcf
{
	if( PROTECTED_MODE )
	{
		i386_protected_mode_iret(cpustate,1);
	}
	else
	{
		/* TODO: #SS(0) exception */
		/* TODO: #GP(0) exception */
		cpustate->eip = POP32(cpustate);
		cpustate->sreg[CS].selector = POP32(cpustate) & 0xffff;
		set_flags(cpustate, POP32(cpustate) );
		i386_load_segment_descriptor(cpustate,CS);
		CHANGE_PC(cpustate,cpustate->eip);
	}
	CYCLES(cpustate,CYCLES_IRET);
}

static void I386OP(ja_rel32)(i386_state *cpustate)			// Opcode 0x0f 87
{
	INT32 disp = FETCH32(cpustate);
	if( cpustate->CF == 0 && cpustate->ZF == 0 ) {
		cpustate->eip += disp;
		CHANGE_PC(cpustate,cpustate->eip);
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jbe_rel32)(i386_state *cpustate)			// Opcode 0x0f 86
{
	INT32 disp = FETCH32(cpustate);
	if( cpustate->CF != 0 || cpustate->ZF != 0 ) {
		cpustate->eip += disp;
		CHANGE_PC(cpustate,cpustate->eip);
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jc_rel32)(i386_state *cpustate)			// Opcode 0x0f 82
{
	INT32 disp = FETCH32(cpustate);
	if( cpustate->CF != 0 ) {
		cpustate->eip += disp;
		CHANGE_PC(cpustate,cpustate->eip);
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jg_rel32)(i386_state *cpustate)			// Opcode 0x0f 8f
{
	INT32 disp = FETCH32(cpustate);
	if( cpustate->ZF == 0 && (cpustate->SF == cpustate->OF) ) {
		cpustate->eip += disp;
		CHANGE_PC(cpustate,cpustate->eip);
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jge_rel32)(i386_state *cpustate)			// Opcode 0x0f 8d
{
	INT32 disp = FETCH32(cpustate);
	if(cpustate->SF == cpustate->OF) {
		cpustate->eip += disp;
		CHANGE_PC(cpustate,cpustate->eip);
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jl_rel32)(i386_state *cpustate)			// Opcode 0x0f 8c
{
	INT32 disp = FETCH32(cpustate);
	if( (cpustate->SF != cpustate->OF) ) {
		cpustate->eip += disp;
		CHANGE_PC(cpustate,cpustate->eip);
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jle_rel32)(i386_state *cpustate)			// Opcode 0x0f 8e
{
	INT32 disp = FETCH32(cpustate);
	if( cpustate->ZF != 0 || (cpustate->SF != cpustate->OF) ) {
		cpustate->eip += disp;
		CHANGE_PC(cpustate,cpustate->eip);
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jnc_rel32)(i386_state *cpustate)			// Opcode 0x0f 83
{
	INT32 disp = FETCH32(cpustate);
	if( cpustate->CF == 0 ) {
		cpustate->eip += disp;
		CHANGE_PC(cpustate,cpustate->eip);
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jno_rel32)(i386_state *cpustate)			// Opcode 0x0f 81
{
	INT32 disp = FETCH32(cpustate);
	if( cpustate->OF == 0 ) {
		cpustate->eip += disp;
		CHANGE_PC(cpustate,cpustate->eip);
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jnp_rel32)(i386_state *cpustate)			// Opcode 0x0f 8b
{
	INT32 disp = FETCH32(cpustate);
	if( cpustate->PF == 0 ) {
		cpustate->eip += disp;
		CHANGE_PC(cpustate,cpustate->eip);
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jns_rel32)(i386_state *cpustate)			// Opcode 0x0f 89
{
	INT32 disp = FETCH32(cpustate);
	if( cpustate->SF == 0 ) {
		cpustate->eip += disp;
		CHANGE_PC(cpustate,cpustate->eip);
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jnz_rel32)(i386_state *cpustate)			// Opcode 0x0f 85
{
	INT32 disp = FETCH32(cpustate);
	if( cpustate->ZF == 0 ) {
		cpustate->eip += disp;
		CHANGE_PC(cpustate,cpustate->eip);
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jo_rel32)(i386_state *cpustate)			// Opcode 0x0f 80
{
	INT32 disp = FETCH32(cpustate);
	if( cpustate->OF != 0 ) {
		cpustate->eip += disp;
		CHANGE_PC(cpustate,cpustate->eip);
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jp_rel32)(i386_state *cpustate)			// Opcode 0x0f 8a
{
	INT32 disp = FETCH32(cpustate);
	if( cpustate->PF != 0 ) {
		cpustate->eip += disp;
		CHANGE_PC(cpustate,cpustate->eip);
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(js_rel32)(i386_state *cpustate)			// Opcode 0x0f 88
{
	INT32 disp = FETCH32(cpustate);
	if( cpustate->SF != 0 ) {
		cpustate->eip += disp;
		CHANGE_PC(cpustate,cpustate->eip);
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jz_rel32)(i386_state *cpustate)			// Opcode 0x0f 84
{
	INT32 disp = FETCH32(cpustate);
	if( cpustate->ZF != 0 ) {
		cpustate->eip += disp;
		CHANGE_PC(cpustate,cpustate->eip);
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jcxz32)(i386_state *cpustate)			// Opcode 0xe3
{
	INT8 disp = FETCH(cpustate);
	int val = (cpustate->address_size)?(REG32(ECX) == 0):(REG16(CX) == 0);
	if( val ) {
		cpustate->eip += disp;
		CHANGE_PC(cpustate,cpustate->eip);
		CYCLES(cpustate,CYCLES_JCXZ);		/* TODO: Timing = 9 + m */
	} else {
		CYCLES(cpustate,CYCLES_JCXZ_NOBRANCH);
	}
}

static void I386OP(jmp_rel32)(i386_state *cpustate)			// Opcode 0xe9
{
	UINT32 disp = FETCH32(cpustate);
	/* TODO: Segment limit */
	cpustate->eip += disp;
	CHANGE_PC(cpustate,cpustate->eip);
	CYCLES(cpustate,CYCLES_JMP);		/* TODO: Timing = 7 + m */
}

static void I386OP(jmp_abs32)(i386_state *cpustate)			// Opcode 0xea
{
	UINT32 address = FETCH32(cpustate);
	UINT16 segment = FETCH16(cpustate);

	if( PROTECTED_MODE && !V8086_MODE)
	{
		i386_protected_mode_jump(cpustate,segment,address,0,1);
	}
	else
	{
		cpustate->eip = address;
		cpustate->sreg[CS].selector = segment;
		cpustate->performed_intersegment_jump = 1;
		i386_load_segment_descriptor(cpustate,CS);
		CHANGE_PC(cpustate,cpustate->eip);
	}
	CYCLES(cpustate,CYCLES_JMP_INTERSEG);
}

static void I386OP(lea32)(i386_state *cpustate)				// Opcode 0x8d
{
	UINT8 modrm = FETCH(cpustate);
	UINT32 ea = GetNonTranslatedEA(cpustate,modrm,NULL);
	if (!cpustate->address_size)
	{
		ea &= 0xffff;
	}
	STORE_REG32(modrm, ea);
	CYCLES(cpustate,CYCLES_LEA);
}

static void I386OP(enter32)(i386_state *cpustate)			// Opcode 0xc8
{
	UINT16 framesize = FETCH16(cpustate);
	UINT8 level = FETCH(cpustate) % 32;
	UINT8 x;
	UINT32 frameptr;
	PUSH32(cpustate,REG32(EBP));
	if(!STACK_32BIT)
		frameptr = REG16(SP);
	else
		frameptr = REG32(ESP);

	if(level > 0)
	{
		for(x=1;x<level-1;x++)
		{
			REG32(EBP) -= 4;
			PUSH32(cpustate,READ32(cpustate,REG32(EBP)));
		}
		PUSH32(cpustate,frameptr);
	}
	REG32(EBP) = frameptr;
	if(!STACK_32BIT)
		REG16(SP) -= framesize;
	else
		REG32(ESP) -= framesize;
	CYCLES(cpustate,CYCLES_ENTER);
}

static void I386OP(leave32)(i386_state *cpustate)			// Opcode 0xc9
{
	if(!STACK_32BIT)
		REG16(SP) = REG16(BP);
	else
		REG32(ESP) = REG32(EBP);
	REG32(EBP) = POP32(cpustate);
	CYCLES(cpustate,CYCLES_LEAVE);
}

static void I386OP(lodsd)(i386_state *cpustate)				// Opcode 0xad
{
	UINT32 eas;
	if( cpustate->segment_prefix ) {
		eas = i386_translate(cpustate, cpustate->segment_override, cpustate->address_size ? REG32(ESI) : REG16(SI), 0 );
	} else {
		eas = i386_translate(cpustate, DS, cpustate->address_size ? REG32(ESI) : REG16(SI), 0 );
	}
	REG32(EAX) = READ32(cpustate,eas);
	BUMP_SI(cpustate,4);
	CYCLES(cpustate,CYCLES_LODS);
}

static void I386OP(loop32)(i386_state *cpustate)			// Opcode 0xe2
{
	INT8 disp = FETCH(cpustate);
	INT32 reg = (cpustate->address_size)?--REG32(ECX):--REG16(CX);
	if( reg != 0 ) {
		cpustate->eip += disp;
		CHANGE_PC(cpustate,cpustate->eip);
	}
	CYCLES(cpustate,CYCLES_LOOP);		/* TODO: Timing = 11 + m */
}

static void I386OP(loopne32)(i386_state *cpustate)			// Opcode 0xe0
{
	INT8 disp = FETCH(cpustate);
	INT32 reg = (cpustate->address_size)?--REG32(ECX):--REG16(CX);
	if( reg != 0 && cpustate->ZF == 0 ) {
		cpustate->eip += disp;
		CHANGE_PC(cpustate,cpustate->eip);
	}
	CYCLES(cpustate,CYCLES_LOOPNZ);		/* TODO: Timing = 11 + m */
}

static void I386OP(loopz32)(i386_state *cpustate)			// Opcode 0xe1
{
	INT8 disp = FETCH(cpustate);
	INT32 reg = (cpustate->address_size)?--REG32(ECX):--REG16(CX);
	if( reg != 0 && cpustate->ZF != 0 ) {
		cpustate->eip += disp;
		CHANGE_PC(cpustate,cpustate->eip);
	}
	CYCLES(cpustate,CYCLES_LOOPZ);		/* TODO: Timing = 11 + m */
}

static void I386OP(mov_rm32_r32)(i386_state *cpustate)		// Opcode 0x89
{
	UINT32 src;
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_REG32(modrm);
		STORE_RM32(modrm, src);
		CYCLES(cpustate,CYCLES_MOV_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		src = LOAD_REG32(modrm);
		WRITE32(cpustate,ea, src);
		CYCLES(cpustate,CYCLES_MOV_REG_MEM);
	}
}

static void I386OP(mov_r32_rm32)(i386_state *cpustate)		// Opcode 0x8b
{
	UINT32 src;
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_RM32(modrm);
		STORE_REG32(modrm, src);
		CYCLES(cpustate,CYCLES_MOV_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,0);
		src = READ32(cpustate,ea);
		STORE_REG32(modrm, src);
		CYCLES(cpustate,CYCLES_MOV_MEM_REG);
	}
}

static void I386OP(mov_rm32_i32)(i386_state *cpustate)		// Opcode 0xc7
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		UINT32 value = FETCH32(cpustate);
		STORE_RM32(modrm, value);
		CYCLES(cpustate,CYCLES_MOV_IMM_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		UINT32 value = FETCH32(cpustate);
		WRITE32(cpustate,ea, value);
		CYCLES(cpustate,CYCLES_MOV_IMM_MEM);
	}
}

static void I386OP(mov_eax_m32)(i386_state *cpustate)		// Opcode 0xa1
{
	UINT32 offset, ea;
	if( cpustate->address_size ) {
		offset = FETCH32(cpustate);
	} else {
		offset = FETCH16(cpustate);
	}
	if( cpustate->segment_prefix ) {
		ea = i386_translate(cpustate, cpustate->segment_override, offset, 0 );
	} else {
		ea = i386_translate(cpustate, DS, offset, 0 );
	}
	REG32(EAX) = READ32(cpustate,ea);
	CYCLES(cpustate,CYCLES_MOV_MEM_ACC);
}

static void I386OP(mov_m32_eax)(i386_state *cpustate)		// Opcode 0xa3
{
	UINT32 offset, ea;
	if( cpustate->address_size ) {
		offset = FETCH32(cpustate);
	} else {
		offset = FETCH16(cpustate);
	}
	if( cpustate->segment_prefix ) {
		ea = i386_translate(cpustate, cpustate->segment_override, offset, 1 );
	} else {
		ea = i386_translate(cpustate, DS, offset, 1 );
	}
	WRITE32(cpustate, ea, REG32(EAX) );
	CYCLES(cpustate,CYCLES_MOV_ACC_MEM);
}

static void I386OP(mov_eax_i32)(i386_state *cpustate)		// Opcode 0xb8
{
	REG32(EAX) = FETCH32(cpustate);
	CYCLES(cpustate,CYCLES_MOV_IMM_REG);
}

static void I386OP(mov_ecx_i32)(i386_state *cpustate)		// Opcode 0xb9
{
	REG32(ECX) = FETCH32(cpustate);
	CYCLES(cpustate,CYCLES_MOV_IMM_REG);
}

static void I386OP(mov_edx_i32)(i386_state *cpustate)		// Opcode 0xba
{
	REG32(EDX) = FETCH32(cpustate);
	CYCLES(cpustate,CYCLES_MOV_IMM_REG);
}

static void I386OP(mov_ebx_i32)(i386_state *cpustate)		// Opcode 0xbb
{
	REG32(EBX) = FETCH32(cpustate);
	CYCLES(cpustate,CYCLES_MOV_IMM_REG);
}

static void I386OP(mov_esp_i32)(i386_state *cpustate)		// Opcode 0xbc
{
	REG32(ESP) = FETCH32(cpustate);
	CYCLES(cpustate,CYCLES_MOV_IMM_REG);
}

static void I386OP(mov_ebp_i32)(i386_state *cpustate)		// Opcode 0xbd
{
	REG32(EBP) = FETCH32(cpustate);
	CYCLES(cpustate,CYCLES_MOV_IMM_REG);
}

static void I386OP(mov_esi_i32)(i386_state *cpustate)		// Opcode 0xbe
{
	REG32(ESI) = FETCH32(cpustate);
	CYCLES(cpustate,CYCLES_MOV_IMM_REG);
}

static void I386OP(mov_edi_i32)(i386_state *cpustate)		// Opcode 0xbf
{
	REG32(EDI) = FETCH32(cpustate);
	CYCLES(cpustate,CYCLES_MOV_IMM_REG);
}

static void I386OP(movsd)(i386_state *cpustate)				// Opcode 0xa5
{
	UINT32 eas, ead, v;
	if( cpustate->segment_prefix ) {
		eas = i386_translate(cpustate, cpustate->segment_override, cpustate->address_size ? REG32(ESI) : REG16(SI), 0 );
	} else {
		eas = i386_translate(cpustate, DS, cpustate->address_size ? REG32(ESI) : REG16(SI), 0 );
	}
	ead = i386_translate(cpustate, ES, cpustate->address_size ? REG32(EDI) : REG16(DI), 1 );
	v = READ32(cpustate,eas);
	WRITE32(cpustate,ead, v);
	BUMP_SI(cpustate,4);
	BUMP_DI(cpustate,4);
	CYCLES(cpustate,CYCLES_MOVS);
}

static void I386OP(movsx_r32_rm8)(i386_state *cpustate)		// Opcode 0x0f be
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		INT32 src = (INT8)LOAD_RM8(modrm);
		STORE_REG32(modrm, src);
		CYCLES(cpustate,CYCLES_MOVSX_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,0);
		INT32 src = (INT8)READ8(cpustate,ea);
		STORE_REG32(modrm, src);
		CYCLES(cpustate,CYCLES_MOVSX_MEM_REG);
	}
}

static void I386OP(movsx_r32_rm16)(i386_state *cpustate)	// Opcode 0x0f bf
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		INT32 src = (INT16)LOAD_RM16(modrm);
		STORE_REG32(modrm, src);
		CYCLES(cpustate,CYCLES_MOVSX_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,0);
		INT32 src = (INT16)READ16(cpustate,ea);
		STORE_REG32(modrm, src);
		CYCLES(cpustate,CYCLES_MOVSX_MEM_REG);
	}
}

static void I386OP(movzx_r32_rm8)(i386_state *cpustate)		// Opcode 0x0f b6
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		UINT32 src = (UINT8)LOAD_RM8(modrm);
		STORE_REG32(modrm, src);
		CYCLES(cpustate,CYCLES_MOVZX_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,0);
		UINT32 src = (UINT8)READ8(cpustate,ea);
		STORE_REG32(modrm, src);
		CYCLES(cpustate,CYCLES_MOVZX_MEM_REG);
	}
}

static void I386OP(movzx_r32_rm16)(i386_state *cpustate)	// Opcode 0x0f b7
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		UINT32 src = (UINT16)LOAD_RM16(modrm);
		STORE_REG32(modrm, src);
		CYCLES(cpustate,CYCLES_MOVZX_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,0);
		UINT32 src = (UINT16)READ16(cpustate,ea);
		STORE_REG32(modrm, src);
		CYCLES(cpustate,CYCLES_MOVZX_MEM_REG);
	}
}

static void I386OP(or_rm32_r32)(i386_state *cpustate)		// Opcode 0x09
{
	UINT32 src, dst;
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_REG32(modrm);
		dst = LOAD_RM32(modrm);
		dst = OR32(cpustate,dst, src);
		STORE_RM32(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		src = LOAD_REG32(modrm);
		dst = READ32(cpustate,ea);
		dst = OR32(cpustate,dst, src);
		WRITE32(cpustate,ea, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_MEM);
	}
}

static void I386OP(or_r32_rm32)(i386_state *cpustate)		// Opcode 0x0b
{
	UINT32 src, dst;
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_RM32(modrm);
		dst = LOAD_REG32(modrm);
		dst = OR32(cpustate,dst, src);
		STORE_REG32(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,0);
		src = READ32(cpustate,ea);
		dst = LOAD_REG32(modrm);
		dst = OR32(cpustate,dst, src);
		STORE_REG32(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_MEM_REG);
	}
}

static void I386OP(or_eax_i32)(i386_state *cpustate)		// Opcode 0x0d
{
	UINT32 src, dst;
	src = FETCH32(cpustate);
	dst = REG32(EAX);
	dst = OR32(cpustate,dst, src);
	REG32(EAX) = dst;
	CYCLES(cpustate,CYCLES_ALU_IMM_ACC);
}

static void I386OP(out_eax_i8)(i386_state *cpustate)		// Opcode 0xe7
{
	UINT16 port = FETCH(cpustate);
	UINT32 data = REG32(EAX);
	WRITEPORT32(cpustate, port, data);
	CYCLES(cpustate,CYCLES_OUT_VAR);
}

static void I386OP(out_eax_dx)(i386_state *cpustate)		// Opcode 0xef
{
	UINT16 port = REG16(DX);
	UINT32 data = REG32(EAX);
	WRITEPORT32(cpustate, port, data);
	CYCLES(cpustate,CYCLES_OUT);
}

static void I386OP(pop_eax)(i386_state *cpustate)			// Opcode 0x58
{
	UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(cpustate,SS,offset+3) == 0)
		REG32(EAX) = POP32(cpustate);
	else
		FAULT(FAULT_SS,0)
	CYCLES(cpustate,CYCLES_POP_REG_SHORT);
}

static void I386OP(pop_ecx)(i386_state *cpustate)			// Opcode 0x59
{
	UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(cpustate,SS,offset+3) == 0)
		REG32(ECX) = POP32(cpustate);
	else
		FAULT(FAULT_SS,0)
	CYCLES(cpustate,CYCLES_POP_REG_SHORT);
}

static void I386OP(pop_edx)(i386_state *cpustate)			// Opcode 0x5a
{
	UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(cpustate,SS,offset+3) == 0)
		REG32(EDX) = POP32(cpustate);
	else
		FAULT(FAULT_SS,0)
	CYCLES(cpustate,CYCLES_POP_REG_SHORT);
}

static void I386OP(pop_ebx)(i386_state *cpustate)			// Opcode 0x5b
{
	UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(cpustate,SS,offset+3) == 0)
		REG32(EBX) = POP32(cpustate);
	else
		FAULT(FAULT_SS,0)
	CYCLES(cpustate,CYCLES_POP_REG_SHORT);
}

static void I386OP(pop_esp)(i386_state *cpustate)			// Opcode 0x5c
{
	UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(cpustate,SS,offset+3) == 0)
		REG32(ESP) = POP32(cpustate);
	else
		FAULT(FAULT_SS,0)
	CYCLES(cpustate,CYCLES_POP_REG_SHORT);
}

static void I386OP(pop_ebp)(i386_state *cpustate)			// Opcode 0x5d
{
	UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(cpustate,SS,offset+3) == 0)
		REG32(EBP) = POP32(cpustate);
	else
		FAULT(FAULT_SS,0)
	CYCLES(cpustate,CYCLES_POP_REG_SHORT);
}

static void I386OP(pop_esi)(i386_state *cpustate)			// Opcode 0x5e
{
	UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(cpustate,SS,offset+3) == 0)
		REG32(ESI) = POP32(cpustate);
	else
		FAULT(FAULT_SS,0)
	CYCLES(cpustate,CYCLES_POP_REG_SHORT);
}

static void I386OP(pop_edi)(i386_state *cpustate)			// Opcode 0x5f
{
	UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(cpustate,SS,offset+3) == 0)
		REG32(EDI) = POP32(cpustate);
	else
		FAULT(FAULT_SS,0)
	CYCLES(cpustate,CYCLES_POP_REG_SHORT);
}

static bool I386OP(pop_seg32)(i386_state *cpustate, int segment)
{
	UINT32 ea, offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	UINT32 value;
	bool fault;
	if(i386_limit_check(cpustate,SS,offset+3) == 0)
	{
		ea = i386_translate(cpustate, SS, offset, 0);
		value = READ32(cpustate, ea);
		i386_sreg_load(cpustate,value, segment, &fault);
		if(fault) return false;
		if(STACK_32BIT)
			REG32(ESP) = offset + 4;
		else
			REG16(SP) = offset + 4;
	}
	else
	{
		cpustate->ext = 1;
		i386_trap_with_error(cpustate,FAULT_SS,0,0,0);
		return false;
	}
	CYCLES(cpustate,CYCLES_POP_SREG);
	return true;
}

static void I386OP(pop_ds32)(i386_state *cpustate)			// Opcode 0x1f
{
	I386OP(pop_seg32)(cpustate, DS);
}

static void I386OP(pop_es32)(i386_state *cpustate)			// Opcode 0x07
{
	I386OP(pop_seg32)(cpustate, ES);
}

static void I386OP(pop_fs32)(i386_state *cpustate)			// Opcode 0x0f a1
{
	I386OP(pop_seg32)(cpustate, FS);
}

static void I386OP(pop_gs32)(i386_state *cpustate)			// Opcode 0x0f a9
{
	I386OP(pop_seg32)(cpustate, GS);
}

static void I386OP(pop_ss32)(i386_state *cpustate)			// Opcode 0x17
{
	if(!I386OP(pop_seg32)(cpustate, SS)) return;
	if(cpustate->IF != 0) // if external interrupts are enabled
	{
		cpustate->IF = 0;  // reset IF for the next instruction
		cpustate->delayed_interrupt_enable = 1;
	}
}

static void I386OP(pop_rm32)(i386_state *cpustate)			// Opcode 0x8f
{
	UINT8 modrm = FETCH(cpustate);
	UINT32 value;
	UINT32 ea, offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(cpustate,SS,offset+3) == 0)
	{
		// be careful here, if the write references the esp register
		// it expects the post-pop value but esp must be wound back
		// if the write faults
		UINT32 temp_sp = REG32(ESP);
		value = POP32(cpustate);

		if( modrm >= 0xc0 ) {
			STORE_RM32(modrm, value);
		} else {
			ea = GetEA(cpustate,modrm,1);
			try
			{
				WRITE32(cpustate,ea, value);
			}
			catch(UINT64 e)
			{
				REG32(ESP) = temp_sp;
				throw e;
			}
		}
	}
	else
		FAULT(FAULT_SS,0)
	CYCLES(cpustate,CYCLES_POP_RM);
}

static void I386OP(popad)(i386_state *cpustate)				// Opcode 0x61
{
	UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(cpustate,SS,offset+31) == 0)
	{
		REG32(EDI) = POP32(cpustate);
		REG32(ESI) = POP32(cpustate);
		REG32(EBP) = POP32(cpustate);
		REG32(ESP) += 4;
		REG32(EBX) = POP32(cpustate);
		REG32(EDX) = POP32(cpustate);
		REG32(ECX) = POP32(cpustate);
		REG32(EAX) = POP32(cpustate);
	}
	else
		FAULT(FAULT_SS,0)
	CYCLES(cpustate,CYCLES_POPA);
}

static void I386OP(popfd)(i386_state *cpustate)				// Opcode 0x9d
{
	UINT32 value;
	UINT32 current = get_flags(cpustate);
	UINT8 IOPL = (current >> 12) & 0x03;
	UINT32 mask = 0x00257fd5;  // VM, VIP and VIF cannot be set by POPF/POPFD
	UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));

	// IOPL can only change if CPL is 0
	if(cpustate->CPL != 0)
		mask &= ~0x00003000;

	// IF can only change if CPL is at least as privileged as IOPL
	if(cpustate->CPL > IOPL)
		mask &= ~0x00000200;

	if(V8086_MODE)
	{
		if(IOPL < 3)
		{
			logerror("POPFD(%08x): IOPL < 3 while in V86 mode.\n",cpustate->pc);
			FAULT(FAULT_GP,0)  // #GP(0)
		}
		mask &= ~0x00003000;  // IOPL cannot be changed while in V8086 mode
	}

	if(i386_limit_check(cpustate,SS,offset+3) == 0)
	{
		value = POP32(cpustate);
		value &= ~0x00010000;  // RF will always return zero
		set_flags(cpustate,(current & ~mask) | (value & mask));  // mask out reserved bits
	}
	else
		FAULT(FAULT_SS,0)
	CYCLES(cpustate,CYCLES_POPF);
}

static void I386OP(push_eax)(i386_state *cpustate)			// Opcode 0x50
{
	UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(cpustate,SS,offset-4) == 0)
		PUSH32(cpustate, REG32(EAX) );
	else
		FAULT(FAULT_SS,0)
	CYCLES(cpustate,CYCLES_PUSH_REG_SHORT);
}

static void I386OP(push_ecx)(i386_state *cpustate)			// Opcode 0x51
{
	UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(cpustate,SS,offset-4) == 0)
		PUSH32(cpustate, REG32(ECX) );
	else
		FAULT(FAULT_SS,0)
	CYCLES(cpustate,CYCLES_PUSH_REG_SHORT);
}

static void I386OP(push_edx)(i386_state *cpustate)			// Opcode 0x52
{
	UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(cpustate,SS,offset-4) == 0)
		PUSH32(cpustate, REG32(EDX) );
	else
		FAULT(FAULT_SS,0)
	CYCLES(cpustate,CYCLES_PUSH_REG_SHORT);
}

static void I386OP(push_ebx)(i386_state *cpustate)			// Opcode 0x53
{
	UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(cpustate,SS,offset-4) == 0)
		PUSH32(cpustate, REG32(EBX) );
	else
		FAULT(FAULT_SS,0)
	CYCLES(cpustate,CYCLES_PUSH_REG_SHORT);
}

static void I386OP(push_esp)(i386_state *cpustate)			// Opcode 0x54
{
	UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(cpustate,SS,offset-4) == 0)
		PUSH32(cpustate, REG32(ESP) );
	else
		FAULT(FAULT_SS,0)
	CYCLES(cpustate,CYCLES_PUSH_REG_SHORT);
}

static void I386OP(push_ebp)(i386_state *cpustate)			// Opcode 0x55
{
	UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(cpustate,SS,offset-4) == 0)
		PUSH32(cpustate, REG32(EBP) );
	else
		FAULT(FAULT_SS,0)
	CYCLES(cpustate,CYCLES_PUSH_REG_SHORT);
}

static void I386OP(push_esi)(i386_state *cpustate)			// Opcode 0x56
{
	UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(cpustate,SS,offset-4) == 0)
		PUSH32(cpustate, REG32(ESI) );
	else
		FAULT(FAULT_SS,0)
	CYCLES(cpustate,CYCLES_PUSH_REG_SHORT);
}

static void I386OP(push_edi)(i386_state *cpustate)			// Opcode 0x57
{
	UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(cpustate,SS,offset-4) == 0)
		PUSH32(cpustate, REG32(EDI) );
	else
		FAULT(FAULT_SS,0)
	CYCLES(cpustate,CYCLES_PUSH_REG_SHORT);
}

static void I386OP(push_cs32)(i386_state *cpustate)			// Opcode 0x0e
{
	UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(cpustate,SS,offset-4) == 0)
		PUSH32(cpustate, cpustate->sreg[CS].selector );
	else
		FAULT(FAULT_SS,0)
	CYCLES(cpustate,CYCLES_PUSH_SREG);
}

static void I386OP(push_ds32)(i386_state *cpustate)			// Opcode 0x1e
{
	UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(cpustate,SS,offset-4) == 0)
		PUSH32(cpustate, cpustate->sreg[DS].selector );
	else
		FAULT(FAULT_SS,0)
	CYCLES(cpustate,CYCLES_PUSH_SREG);
}

static void I386OP(push_es32)(i386_state *cpustate)			// Opcode 0x06
{
	UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(cpustate,SS,offset-4) == 0)
		PUSH32(cpustate, cpustate->sreg[ES].selector );
	else
		FAULT(FAULT_SS,0)
	CYCLES(cpustate,CYCLES_PUSH_SREG);
}

static void I386OP(push_fs32)(i386_state *cpustate)			// Opcode 0x0f a0
{
	UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(cpustate,SS,offset-4) == 0)
		PUSH32(cpustate, cpustate->sreg[FS].selector );
	else
		FAULT(FAULT_SS,0)
	CYCLES(cpustate,CYCLES_PUSH_SREG);
}

static void I386OP(push_gs32)(i386_state *cpustate)			// Opcode 0x0f a8
{
	UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(cpustate,SS,offset-4) == 0)
		PUSH32(cpustate, cpustate->sreg[GS].selector );
	else
		FAULT(FAULT_SS,0)
	CYCLES(cpustate,CYCLES_PUSH_SREG);
}

static void I386OP(push_ss32)(i386_state *cpustate)			// Opcode 0x16
{
	UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(cpustate,SS,offset-4) == 0)
		PUSH32(cpustate, cpustate->sreg[SS].selector );
	else
		FAULT(FAULT_SS,0)
	CYCLES(cpustate,CYCLES_PUSH_SREG);
}

static void I386OP(push_i32)(i386_state *cpustate)			// Opcode 0x68
{
	UINT32 value = FETCH32(cpustate);
	UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(cpustate,SS,offset-4) == 0)
		PUSH32(cpustate,value);
	else
		FAULT(FAULT_SS,0)
	CYCLES(cpustate,CYCLES_PUSH_IMM);
}

static void I386OP(pushad)(i386_state *cpustate)			// Opcode 0x60
{
	UINT32 temp = REG32(ESP);
	UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(cpustate,SS,offset-32) == 0)
	{
		PUSH32(cpustate, REG32(EAX) );
		PUSH32(cpustate, REG32(ECX) );
		PUSH32(cpustate, REG32(EDX) );
		PUSH32(cpustate, REG32(EBX) );
		PUSH32(cpustate, temp );
		PUSH32(cpustate, REG32(EBP) );
		PUSH32(cpustate, REG32(ESI) );
		PUSH32(cpustate, REG32(EDI) );
	}
	else
		FAULT(FAULT_SS,0)
	CYCLES(cpustate,CYCLES_PUSHA);
}

static void I386OP(pushfd)(i386_state *cpustate)			// Opcode 0x9c
{
	UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(cpustate,SS,offset-4) == 0)
		PUSH32(cpustate, get_flags(cpustate) & 0x00fcffff );
	else
		FAULT(FAULT_SS,0)
	CYCLES(cpustate,CYCLES_PUSHF);
}

static void I386OP(ret_near32_i16)(i386_state *cpustate)	// Opcode 0xc2
{
	INT16 disp = FETCH16(cpustate);
	cpustate->eip = POP32(cpustate);
	REG32(ESP) += disp;
	CHANGE_PC(cpustate,cpustate->eip);
	CYCLES(cpustate,CYCLES_RET_IMM);		/* TODO: Timing = 10 + m */
}

static void I386OP(ret_near32)(i386_state *cpustate)		// Opcode 0xc3
{
	cpustate->eip = POP32(cpustate);
	CHANGE_PC(cpustate,cpustate->eip);
	CYCLES(cpustate,CYCLES_RET);		/* TODO: Timing = 10 + m */
}

static void I386OP(sbb_rm32_r32)(i386_state *cpustate)		// Opcode 0x19
{
	UINT32 src, dst;
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_REG32(modrm);
		dst = LOAD_RM32(modrm);
		dst = SBB32(cpustate, dst, src, cpustate->CF);
		STORE_RM32(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		src = LOAD_REG32(modrm);
		dst = READ32(cpustate,ea);
		dst = SBB32(cpustate, dst, src, cpustate->CF);
		WRITE32(cpustate,ea, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_MEM);
	}
}

static void I386OP(sbb_r32_rm32)(i386_state *cpustate)		// Opcode 0x1b
{
	UINT32 src, dst;
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_RM32(modrm);
		dst = LOAD_REG32(modrm);
		dst = SBB32(cpustate, dst, src, cpustate->CF);
		STORE_REG32(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,0);
		src = READ32(cpustate,ea);
		dst = LOAD_REG32(modrm);
		dst = SBB32(cpustate, dst, src, cpustate->CF);
		STORE_REG32(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_MEM_REG);
	}
}

static void I386OP(sbb_eax_i32)(i386_state *cpustate)		// Opcode 0x1d
{
	UINT32 src, dst;
	src = FETCH32(cpustate);
	dst = REG32(EAX);
	dst = SBB32(cpustate, dst, src, cpustate->CF);
	REG32(EAX) = dst;
	CYCLES(cpustate,CYCLES_ALU_IMM_ACC);
}

static void I386OP(scasd)(i386_state *cpustate)				// Opcode 0xaf
{
	UINT32 eas, src, dst;
	eas = i386_translate(cpustate, ES, cpustate->address_size ? REG32(EDI) : REG16(DI), 0 );
	src = READ32(cpustate,eas);
	dst = REG32(EAX);
	SUB32(cpustate,dst, src);
	BUMP_DI(cpustate,4);
	CYCLES(cpustate,CYCLES_SCAS);
}

static void I386OP(shld32_i8)(i386_state *cpustate)			// Opcode 0x0f a4
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		UINT32 dst = LOAD_RM32(modrm);
		UINT32 upper = LOAD_REG32(modrm);
		UINT8 shift = FETCH(cpustate);
		shift &= 31;
		if( shift == 0 ) {

		} else {
			cpustate->CF = (dst & (1 << (32-shift))) ? 1 : 0;
			dst = (dst << shift) | (upper >> (32-shift));
			cpustate->OF = cpustate->CF ^ (dst >> 31);
			SetSZPF32(dst);
		}
		STORE_RM32(modrm, dst);
		CYCLES(cpustate,CYCLES_SHLD_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		UINT32 dst = READ32(cpustate,ea);
		UINT32 upper = LOAD_REG32(modrm);
		UINT8 shift = FETCH(cpustate);
		shift &= 31;
		if( shift == 0 ) {

		} else {
			cpustate->CF = (dst & (1 << (32-shift))) ? 1 : 0;
			dst = (dst << shift) | (upper >> (32-shift));
			cpustate->OF = cpustate->CF ^ (dst >> 31);
			SetSZPF32(dst);
		}
		WRITE32(cpustate,ea, dst);
		CYCLES(cpustate,CYCLES_SHLD_MEM);
	}
}

static void I386OP(shld32_cl)(i386_state *cpustate)			// Opcode 0x0f a5
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		UINT32 dst = LOAD_RM32(modrm);
		UINT32 upper = LOAD_REG32(modrm);
		UINT8 shift = REG8(CL);
		shift &= 31;
		if( shift == 0 ) {

		} else {
			cpustate->CF = (dst & (1 << (32-shift))) ? 1 : 0;
			dst = (dst << shift) | (upper >> (32-shift));
			cpustate->OF = cpustate->CF ^ (dst >> 31);
			SetSZPF32(dst);
		}
		STORE_RM32(modrm, dst);
		CYCLES(cpustate,CYCLES_SHLD_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		UINT32 dst = READ32(cpustate,ea);
		UINT32 upper = LOAD_REG32(modrm);
		UINT8 shift = REG8(CL);
		shift &= 31;
		if( shift == 0 ) {

		} else {
			cpustate->CF = (dst & (1 << (32-shift))) ? 1 : 0;
			dst = (dst << shift) | (upper >> (32-shift));
			cpustate->OF = cpustate->CF ^ (dst >> 31);
			SetSZPF32(dst);
		}
		WRITE32(cpustate,ea, dst);
		CYCLES(cpustate,CYCLES_SHLD_MEM);
	}
}

static void I386OP(shrd32_i8)(i386_state *cpustate)			// Opcode 0x0f ac
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		UINT32 dst = LOAD_RM32(modrm);
		UINT32 upper = LOAD_REG32(modrm);
		UINT8 shift = FETCH(cpustate);
		shift &= 31;
		if( shift == 0 ) {

		} else {
			cpustate->CF = (dst & (1 << (shift-1))) ? 1 : 0;
			dst = (dst >> shift) | (upper << (32-shift));
			cpustate->OF = ((dst >> 31) ^ (dst >> 30)) & 1;
			SetSZPF32(dst);
		}
		STORE_RM32(modrm, dst);
		CYCLES(cpustate,CYCLES_SHRD_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		UINT32 dst = READ32(cpustate,ea);
		UINT32 upper = LOAD_REG32(modrm);
		UINT8 shift = FETCH(cpustate);
		shift &= 31;
		if( shift == 0 ) {

		} else {
			cpustate->CF = (dst & (1 << (shift-1))) ? 1 : 0;
			dst = (dst >> shift) | (upper << (32-shift));
			cpustate->OF = ((dst >> 31) ^ (dst >> 30)) & 1;
			SetSZPF32(dst);
		}
		WRITE32(cpustate,ea, dst);
		CYCLES(cpustate,CYCLES_SHRD_MEM);
	}
}

static void I386OP(shrd32_cl)(i386_state *cpustate)			// Opcode 0x0f ad
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		UINT32 dst = LOAD_RM32(modrm);
		UINT32 upper = LOAD_REG32(modrm);
		UINT8 shift = REG8(CL);
		shift &= 31;
		if( shift == 0 ) {

		} else {
			cpustate->CF = (dst & (1 << (shift-1))) ? 1 : 0;
			dst = (dst >> shift) | (upper << (32-shift));
			cpustate->OF = ((dst >> 31) ^ (dst >> 30)) & 1;
			SetSZPF32(dst);
		}
		STORE_RM32(modrm, dst);
		CYCLES(cpustate,CYCLES_SHRD_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		UINT32 dst = READ32(cpustate,ea);
		UINT32 upper = LOAD_REG32(modrm);
		UINT8 shift = REG8(CL);
		shift &= 31;
		if( shift == 0 ) {

		} else {
			cpustate->CF = (dst & (1 << (shift-1))) ? 1 : 0;
			dst = (dst >> shift) | (upper << (32-shift));
			cpustate->OF = ((dst >> 31) ^ (dst >> 30)) & 1;
			SetSZPF32(dst);
		}
		WRITE32(cpustate,ea, dst);
		CYCLES(cpustate,CYCLES_SHRD_MEM);
	}
}

static void I386OP(stosd)(i386_state *cpustate)				// Opcode 0xab
{
	UINT32 eas = i386_translate(cpustate, ES, cpustate->address_size ? REG32(EDI) : REG16(DI), 1 );
	WRITE32(cpustate,eas, REG32(EAX));
	BUMP_DI(cpustate,4);
	CYCLES(cpustate,CYCLES_STOS);
}

static void I386OP(sub_rm32_r32)(i386_state *cpustate)		// Opcode 0x29
{
	UINT32 src, dst;
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_REG32(modrm);
		dst = LOAD_RM32(modrm);
		dst = SUB32(cpustate,dst, src);
		STORE_RM32(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		src = LOAD_REG32(modrm);
		dst = READ32(cpustate,ea);
		dst = SUB32(cpustate,dst, src);
		WRITE32(cpustate,ea, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_MEM);
	}
}

static void I386OP(sub_r32_rm32)(i386_state *cpustate)		// Opcode 0x2b
{
	UINT32 src, dst;
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_RM32(modrm);
		dst = LOAD_REG32(modrm);
		dst = SUB32(cpustate,dst, src);
		STORE_REG32(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		src = READ32(cpustate,ea);
		dst = LOAD_REG32(modrm);
		dst = SUB32(cpustate,dst, src);
		STORE_REG32(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_MEM_REG);
	}
}

static void I386OP(sub_eax_i32)(i386_state *cpustate)		// Opcode 0x2d
{
	UINT32 src, dst;
	src = FETCH32(cpustate);
	dst = REG32(EAX);
	dst = SUB32(cpustate,dst, src);
	REG32(EAX) = dst;
	CYCLES(cpustate,CYCLES_ALU_IMM_ACC);
}

static void I386OP(test_eax_i32)(i386_state *cpustate)		// Opcode 0xa9
{
	UINT32 src = FETCH32(cpustate);
	UINT32 dst = REG32(EAX);
	dst = src & dst;
	SetSZPF32(dst);
	cpustate->CF = 0;
	cpustate->OF = 0;
	CYCLES(cpustate,CYCLES_TEST_IMM_ACC);
}

static void I386OP(test_rm32_r32)(i386_state *cpustate)		// Opcode 0x85
{
	UINT32 src, dst;
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_REG32(modrm);
		dst = LOAD_RM32(modrm);
		dst = src & dst;
		SetSZPF32(dst);
		cpustate->CF = 0;
		cpustate->OF = 0;
		CYCLES(cpustate,CYCLES_TEST_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,0);
		src = LOAD_REG32(modrm);
		dst = READ32(cpustate,ea);
		dst = src & dst;
		SetSZPF32(dst);
		cpustate->CF = 0;
		cpustate->OF = 0;
		CYCLES(cpustate,CYCLES_TEST_REG_MEM);
	}
}

static void I386OP(xchg_eax_ecx)(i386_state *cpustate)		// Opcode 0x91
{
	UINT32 temp;
	temp = REG32(EAX);
	REG32(EAX) = REG32(ECX);
	REG32(ECX) = temp;
	CYCLES(cpustate,CYCLES_XCHG_REG_REG);
}

static void I386OP(xchg_eax_edx)(i386_state *cpustate)		// Opcode 0x92
{
	UINT32 temp;
	temp = REG32(EAX);
	REG32(EAX) = REG32(EDX);
	REG32(EDX) = temp;
	CYCLES(cpustate,CYCLES_XCHG_REG_REG);
}

static void I386OP(xchg_eax_ebx)(i386_state *cpustate)		// Opcode 0x93
{
	UINT32 temp;
	temp = REG32(EAX);
	REG32(EAX) = REG32(EBX);
	REG32(EBX) = temp;
	CYCLES(cpustate,CYCLES_XCHG_REG_REG);
}

static void I386OP(xchg_eax_esp)(i386_state *cpustate)		// Opcode 0x94
{
	UINT32 temp;
	temp = REG32(EAX);
	REG32(EAX) = REG32(ESP);
	REG32(ESP) = temp;
	CYCLES(cpustate,CYCLES_XCHG_REG_REG);
}

static void I386OP(xchg_eax_ebp)(i386_state *cpustate)		// Opcode 0x95
{
	UINT32 temp;
	temp = REG32(EAX);
	REG32(EAX) = REG32(EBP);
	REG32(EBP) = temp;
	CYCLES(cpustate,CYCLES_XCHG_REG_REG);
}

static void I386OP(xchg_eax_esi)(i386_state *cpustate)		// Opcode 0x96
{
	UINT32 temp;
	temp = REG32(EAX);
	REG32(EAX) = REG32(ESI);
	REG32(ESI) = temp;
	CYCLES(cpustate,CYCLES_XCHG_REG_REG);
}

static void I386OP(xchg_eax_edi)(i386_state *cpustate)		// Opcode 0x97
{
	UINT32 temp;
	temp = REG32(EAX);
	REG32(EAX) = REG32(EDI);
	REG32(EDI) = temp;
	CYCLES(cpustate,CYCLES_XCHG_REG_REG);
}

static void I386OP(xchg_r32_rm32)(i386_state *cpustate)		// Opcode 0x87
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		UINT32 src = LOAD_RM32(modrm);
		UINT32 dst = LOAD_REG32(modrm);
		STORE_REG32(modrm, src);
		STORE_RM32(modrm, dst);
		CYCLES(cpustate,CYCLES_XCHG_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		UINT32 src = READ32(cpustate,ea);
		UINT32 dst = LOAD_REG32(modrm);
		WRITE32(cpustate,ea, dst);
		STORE_REG32(modrm, src);
		CYCLES(cpustate,CYCLES_XCHG_REG_MEM);
	}
}

static void I386OP(xor_rm32_r32)(i386_state *cpustate)		// Opcode 0x31
{
	UINT32 src, dst;
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_REG32(modrm);
		dst = LOAD_RM32(modrm);
		dst = XOR32(cpustate,dst, src);
		STORE_RM32(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		src = LOAD_REG32(modrm);
		dst = READ32(cpustate,ea);
		dst = XOR32(cpustate,dst, src);
		WRITE32(cpustate,ea, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_MEM);
	}
}

static void I386OP(xor_r32_rm32)(i386_state *cpustate)		// Opcode 0x33
{
	UINT32 src, dst;
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_RM32(modrm);
		dst = LOAD_REG32(modrm);
		dst = XOR32(cpustate,dst, src);
		STORE_REG32(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,0);
		src = READ32(cpustate,ea);
		dst = LOAD_REG32(modrm);
		dst = XOR32(cpustate,dst, src);
		STORE_REG32(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_MEM_REG);
	}
}

static void I386OP(xor_eax_i32)(i386_state *cpustate)		// Opcode 0x35
{
	UINT32 src, dst;
	src = FETCH32(cpustate);
	dst = REG32(EAX);
	dst = XOR32(cpustate,dst, src);
	REG32(EAX) = dst;
	CYCLES(cpustate,CYCLES_ALU_IMM_ACC);
}



static void I386OP(group81_32)(i386_state *cpustate)		// Opcode 0x81
{
	UINT32 ea;
	UINT32 src, dst;
	UINT8 modrm = FETCH(cpustate);

	switch( (modrm >> 3) & 0x7 )
	{
		case 0:		// ADD Rm32, i32
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM32(modrm);
				src = FETCH32(cpustate);
				dst = ADD32(cpustate,dst, src);
				STORE_RM32(modrm, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(cpustate,modrm,1);
				dst = READ32(cpustate,ea);
				src = FETCH32(cpustate);
				dst = ADD32(cpustate,dst, src);
				WRITE32(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_MEM);
			}
			break;
		case 1:		// OR Rm32, i32
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM32(modrm);
				src = FETCH32(cpustate);
				dst = OR32(cpustate,dst, src);
				STORE_RM32(modrm, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(cpustate,modrm,1);
				dst = READ32(cpustate,ea);
				src = FETCH32(cpustate);
				dst = OR32(cpustate,dst, src);
				WRITE32(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_MEM);
			}
			break;
		case 2:		// ADC Rm32, i32
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM32(modrm);
				src = FETCH32(cpustate);
				dst = ADC32(cpustate, dst, src, cpustate->CF);
				STORE_RM32(modrm, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(cpustate,modrm,1);
				dst = READ32(cpustate,ea);
				src = FETCH32(cpustate);
				dst = ADC32(cpustate, dst, src, cpustate->CF);
				WRITE32(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_MEM);
			}
			break;
		case 3:		// SBB Rm32, i32
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM32(modrm);
				src = FETCH32(cpustate);
				dst = SBB32(cpustate, dst, src, cpustate->CF);
				STORE_RM32(modrm, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(cpustate,modrm,1);
				dst = READ32(cpustate,ea);
				src = FETCH32(cpustate);
				dst = SBB32(cpustate, dst, src, cpustate->CF);
				WRITE32(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_MEM);
			}
			break;
		case 4:		// AND Rm32, i32
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM32(modrm);
				src = FETCH32(cpustate);
				dst = AND32(cpustate,dst, src);
				STORE_RM32(modrm, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(cpustate,modrm,1);
				dst = READ32(cpustate,ea);
				src = FETCH32(cpustate);
				dst = AND32(cpustate,dst, src);
				WRITE32(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_MEM);
			}
			break;
		case 5:		// SUB Rm32, i32
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM32(modrm);
				src = FETCH32(cpustate);
				dst = SUB32(cpustate,dst, src);
				STORE_RM32(modrm, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(cpustate,modrm,1);
				dst = READ32(cpustate,ea);
				src = FETCH32(cpustate);
				dst = SUB32(cpustate,dst, src);
				WRITE32(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_MEM);
			}
			break;
		case 6:		// XOR Rm32, i32
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM32(modrm);
				src = FETCH32(cpustate);
				dst = XOR32(cpustate,dst, src);
				STORE_RM32(modrm, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(cpustate,modrm,1);
				dst = READ32(cpustate,ea);
				src = FETCH32(cpustate);
				dst = XOR32(cpustate,dst, src);
				WRITE32(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_MEM);
			}
			break;
		case 7:		// CMP Rm32, i32
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM32(modrm);
				src = FETCH32(cpustate);
				SUB32(cpustate,dst, src);
				CYCLES(cpustate,CYCLES_CMP_REG_REG);
			} else {
				ea = GetEA(cpustate,modrm,0);
				dst = READ32(cpustate,ea);
				src = FETCH32(cpustate);
				SUB32(cpustate,dst, src);
				CYCLES(cpustate,CYCLES_CMP_REG_MEM);
			}
			break;
	}
}

static void I386OP(group83_32)(i386_state *cpustate)		// Opcode 0x83
{
	UINT32 ea;
	UINT32 src, dst;
	UINT8 modrm = FETCH(cpustate);

	switch( (modrm >> 3) & 0x7 )
	{
		case 0:		// ADD Rm32, i32
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM32(modrm);
				src = (UINT32)(INT32)(INT8)FETCH(cpustate);
				dst = ADD32(cpustate,dst, src);
				STORE_RM32(modrm, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(cpustate,modrm,1);
				dst = READ32(cpustate,ea);
				src = (UINT32)(INT32)(INT8)FETCH(cpustate);
				dst = ADD32(cpustate,dst, src);
				WRITE32(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_MEM);
			}
			break;
		case 1:		// OR Rm32, i32
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM32(modrm);
				src = (UINT32)(INT32)(INT8)FETCH(cpustate);
				dst = OR32(cpustate,dst, src);
				STORE_RM32(modrm, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(cpustate,modrm,1);
				dst = READ32(cpustate,ea);
				src = (UINT32)(INT32)(INT8)FETCH(cpustate);
				dst = OR32(cpustate,dst, src);
				WRITE32(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_MEM);
			}
			break;
		case 2:		// ADC Rm32, i32
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM32(modrm);
				src = (UINT32)(INT32)(INT8)FETCH(cpustate);
				dst = ADC32(cpustate, dst, src, cpustate->CF);
				STORE_RM32(modrm, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(cpustate,modrm,1);
				dst = READ32(cpustate,ea);
				src = (UINT32)(INT32)(INT8)FETCH(cpustate);
				dst = ADC32(cpustate, dst, src, cpustate->CF);
				WRITE32(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_MEM);
			}
			break;
		case 3:		// SBB Rm32, i32
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM32(modrm);
				src = ((UINT32)(INT32)(INT8)FETCH(cpustate));
				dst = SBB32(cpustate, dst, src, cpustate->CF);
				STORE_RM32(modrm, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(cpustate,modrm,1);
				dst = READ32(cpustate,ea);
				src = ((UINT32)(INT32)(INT8)FETCH(cpustate));
				dst = SBB32(cpustate, dst, src, cpustate->CF);
				WRITE32(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_MEM);
			}
			break;
		case 4:		// AND Rm32, i32
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM32(modrm);
				src = (UINT32)(INT32)(INT8)FETCH(cpustate);
				dst = AND32(cpustate,dst, src);
				STORE_RM32(modrm, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(cpustate,modrm,1);
				dst = READ32(cpustate,ea);
				src = (UINT32)(INT32)(INT8)FETCH(cpustate);
				dst = AND32(cpustate,dst, src);
				WRITE32(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_MEM);
			}
			break;
		case 5:		// SUB Rm32, i32
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM32(modrm);
				src = (UINT32)(INT32)(INT8)FETCH(cpustate);
				dst = SUB32(cpustate,dst, src);
				STORE_RM32(modrm, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(cpustate,modrm,1);
				dst = READ32(cpustate,ea);
				src = (UINT32)(INT32)(INT8)FETCH(cpustate);
				dst = SUB32(cpustate,dst, src);
				WRITE32(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_MEM);
			}
			break;
		case 6:		// XOR Rm32, i32
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM32(modrm);
				src = (UINT32)(INT32)(INT8)FETCH(cpustate);
				dst = XOR32(cpustate,dst, src);
				STORE_RM32(modrm, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(cpustate,modrm,1);
				dst = READ32(cpustate,ea);
				src = (UINT32)(INT32)(INT8)FETCH(cpustate);
				dst = XOR32(cpustate,dst, src);
				WRITE32(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_MEM);
			}
			break;
		case 7:		// CMP Rm32, i32
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM32(modrm);
				src = (UINT32)(INT32)(INT8)FETCH(cpustate);
				SUB32(cpustate,dst, src);
				CYCLES(cpustate,CYCLES_CMP_REG_REG);
			} else {
				ea = GetEA(cpustate,modrm,0);
				dst = READ32(cpustate,ea);
				src = (UINT32)(INT32)(INT8)FETCH(cpustate);
				SUB32(cpustate,dst, src);
				CYCLES(cpustate,CYCLES_CMP_REG_MEM);
			}
			break;
	}
}

static void I386OP(groupC1_32)(i386_state *cpustate)		// Opcode 0xc1
{
	UINT32 dst;
	UINT8 modrm = FETCH(cpustate);
	UINT8 shift;

	if( modrm >= 0xc0 ) {
		dst = LOAD_RM32(modrm);
		shift = FETCH(cpustate) & 0x1f;
		dst = i386_shift_rotate32(cpustate, modrm, dst, shift);
		STORE_RM32(modrm, dst);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		dst = READ32(cpustate,ea);
		shift = FETCH(cpustate) & 0x1f;
		dst = i386_shift_rotate32(cpustate, modrm, dst, shift);
		WRITE32(cpustate,ea, dst);
	}
}

static void I386OP(groupD1_32)(i386_state *cpustate)		// Opcode 0xd1
{
	UINT32 dst;
	UINT8 modrm = FETCH(cpustate);

	if( modrm >= 0xc0 ) {
		dst = LOAD_RM32(modrm);
		dst = i386_shift_rotate32(cpustate, modrm, dst, 1);
		STORE_RM32(modrm, dst);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		dst = READ32(cpustate,ea);
		dst = i386_shift_rotate32(cpustate, modrm, dst, 1);
		WRITE32(cpustate,ea, dst);
	}
}

static void I386OP(groupD3_32)(i386_state *cpustate)		// Opcode 0xd3
{
	UINT32 dst;
	UINT8 modrm = FETCH(cpustate);

	if( modrm >= 0xc0 ) {
		dst = LOAD_RM32(modrm);
		dst = i386_shift_rotate32(cpustate, modrm, dst, REG8(CL));
		STORE_RM32(modrm, dst);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		dst = READ32(cpustate,ea);
		dst = i386_shift_rotate32(cpustate, modrm, dst, REG8(CL));
		WRITE32(cpustate,ea, dst);
	}
}

static void I386OP(groupF7_32)(i386_state *cpustate)		// Opcode 0xf7
{
	UINT8 modrm = FETCH(cpustate);

	switch( (modrm >> 3) & 0x7 )
	{
		case 0:			/* TEST Rm32, i32 */
			if( modrm >= 0xc0 ) {
				UINT32 dst = LOAD_RM32(modrm);
				UINT32 src = FETCH32(cpustate);
				dst &= src;
				cpustate->CF = cpustate->OF = cpustate->AF = 0;
				SetSZPF32(dst);
				CYCLES(cpustate,CYCLES_TEST_IMM_REG);
			} else {
				UINT32 ea = GetEA(cpustate,modrm,0);
				UINT32 dst = READ32(cpustate,ea);
				UINT32 src = FETCH32(cpustate);
				dst &= src;
				cpustate->CF = cpustate->OF = cpustate->AF = 0;
				SetSZPF32(dst);
				CYCLES(cpustate,CYCLES_TEST_IMM_MEM);
			}
			break;
		case 2:			/* NOT Rm32 */
			if( modrm >= 0xc0 ) {
				UINT32 dst = LOAD_RM32(modrm);
				dst = ~dst;
				STORE_RM32(modrm, dst);
				CYCLES(cpustate,CYCLES_NOT_REG);
			} else {
				UINT32 ea = GetEA(cpustate,modrm,1);
				UINT32 dst = READ32(cpustate,ea);
				dst = ~dst;
				WRITE32(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_NOT_MEM);
			}
			break;
		case 3:			/* NEG Rm32 */
			if( modrm >= 0xc0 ) {
				UINT32 dst = LOAD_RM32(modrm);
				dst = SUB32(cpustate, 0, dst );
				STORE_RM32(modrm, dst);
				CYCLES(cpustate,CYCLES_NEG_REG);
			} else {
				UINT32 ea = GetEA(cpustate,modrm,1);
				UINT32 dst = READ32(cpustate,ea);
				dst = SUB32(cpustate, 0, dst );
				WRITE32(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_NEG_MEM);
			}
			break;
		case 4:			/* MUL EAX, Rm32 */
			{
				UINT64 result;
				UINT32 src, dst;
				if( modrm >= 0xc0 ) {
					src = LOAD_RM32(modrm);
					CYCLES(cpustate,CYCLES_MUL32_ACC_REG);		/* TODO: Correct multiply timing */
				} else {
					UINT32 ea = GetEA(cpustate,modrm,0);
					src = READ32(cpustate,ea);
					CYCLES(cpustate,CYCLES_MUL32_ACC_MEM);		/* TODO: Correct multiply timing */
				}

				dst = REG32(EAX);
				result = (UINT64)src * (UINT64)dst;
				REG32(EDX) = (UINT32)(result >> 32);
				REG32(EAX) = (UINT32)result;

				cpustate->CF = cpustate->OF = (REG32(EDX) != 0);
			}
			break;
		case 5:			/* IMUL EAX, Rm32 */
			{
				INT64 result;
				INT64 src, dst;
				if( modrm >= 0xc0 ) {
					src = (INT64)(INT32)LOAD_RM32(modrm);
					CYCLES(cpustate,CYCLES_IMUL32_ACC_REG);		/* TODO: Correct multiply timing */
				} else {
					UINT32 ea = GetEA(cpustate,modrm,0);
					src = (INT64)(INT32)READ32(cpustate,ea);
					CYCLES(cpustate,CYCLES_IMUL32_ACC_MEM);		/* TODO: Correct multiply timing */
				}

				dst = (INT64)(INT32)REG32(EAX);
				result = src * dst;

				REG32(EDX) = (UINT32)(result >> 32);
				REG32(EAX) = (UINT32)result;

				cpustate->CF = cpustate->OF = !(result == (INT64)(INT32)result);
			}
			break;
		case 6:			/* DIV EAX, Rm32 */
			{
				UINT64 quotient, remainder, result;
				UINT32 src;
				if( modrm >= 0xc0 ) {
					src = LOAD_RM32(modrm);
					CYCLES(cpustate,CYCLES_DIV32_ACC_REG);
				} else {
					UINT32 ea = GetEA(cpustate,modrm,0);
					src = READ32(cpustate,ea);
					CYCLES(cpustate,CYCLES_DIV32_ACC_MEM);
				}

				quotient = ((UINT64)(REG32(EDX)) << 32) | (UINT64)(REG32(EAX));
				if( src ) {
					remainder = quotient % (UINT64)src;
					result = quotient / (UINT64)src;
					if( result > 0xffffffff ) {
						/* TODO: Divide error */
					} else {
						REG32(EDX) = (UINT32)remainder;
						REG32(EAX) = (UINT32)result;
					}
				} else {
					i386_trap(cpustate, 0, 0, 0);
				}
			}
			break;
		case 7:			/* IDIV EAX, Rm32 */
			{
				INT64 quotient, remainder, result;
				UINT32 src;
				if( modrm >= 0xc0 ) {
					src = LOAD_RM32(modrm);
					CYCLES(cpustate,CYCLES_IDIV32_ACC_REG);
				} else {
					UINT32 ea = GetEA(cpustate,modrm,0);
					src = READ32(cpustate,ea);
					CYCLES(cpustate,CYCLES_IDIV32_ACC_MEM);
				}

				quotient = (((INT64)REG32(EDX)) << 32) | ((UINT64)REG32(EAX));
				if( src ) {
					remainder = quotient % (INT64)(INT32)src;
					result = quotient / (INT64)(INT32)src;
					if( result > 0xffffffff ) {
						/* TODO: Divide error */
					} else {
						REG32(EDX) = (UINT32)remainder;
						REG32(EAX) = (UINT32)result;
					}
				} else {
					i386_trap(cpustate, 0, 0, 0);
				}
			}
			break;
	}
}

static void I386OP(groupFF_32)(i386_state *cpustate)		// Opcode 0xff
{
	UINT8 modrm = FETCH(cpustate);

	switch( (modrm >> 3) & 0x7 )
	{
		case 0:			/* INC Rm32 */
			if( modrm >= 0xc0 ) {
				UINT32 dst = LOAD_RM32(modrm);
				dst = INC32(cpustate,dst);
				STORE_RM32(modrm, dst);
				CYCLES(cpustate,CYCLES_INC_REG);
			} else {
				UINT32 ea = GetEA(cpustate,modrm,1);
				UINT32 dst = READ32(cpustate,ea);
				dst = INC32(cpustate,dst);
				WRITE32(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_INC_MEM);
			}
			break;
		case 1:			/* DEC Rm32 */
			if( modrm >= 0xc0 ) {
				UINT32 dst = LOAD_RM32(modrm);
				dst = DEC32(cpustate,dst);
				STORE_RM32(modrm, dst);
				CYCLES(cpustate,CYCLES_DEC_REG);
			} else {
				UINT32 ea = GetEA(cpustate,modrm,1);
				UINT32 dst = READ32(cpustate,ea);
				dst = DEC32(cpustate,dst);
				WRITE32(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_DEC_MEM);
			}
			break;
		case 2:			/* CALL Rm32 */
			{
				UINT32 address;
				if( modrm >= 0xc0 ) {
					address = LOAD_RM32(modrm);
					CYCLES(cpustate,CYCLES_CALL_REG);		/* TODO: Timing = 7 + m */
				} else {
					UINT32 ea = GetEA(cpustate,modrm,0);
					address = READ32(cpustate,ea);
					CYCLES(cpustate,CYCLES_CALL_MEM);		/* TODO: Timing = 10 + m */
				}
				PUSH32(cpustate, cpustate->eip );
				cpustate->eip = address;
				CHANGE_PC(cpustate,cpustate->eip);
			}
			break;
		case 3:			/* CALL FAR Rm32 */
			{
				UINT16 selector;
				UINT32 address;

				if( modrm >= 0xc0 )
				{
					fatalerror("i386: groupFF_32 /%d: NYI", (modrm >> 3) & 0x7);
				}
				else
				{
					UINT32 ea = GetEA(cpustate,modrm,0);
					address = READ32(cpustate,ea + 0);
					selector = READ16(cpustate,ea + 4);
					CYCLES(cpustate,CYCLES_CALL_MEM_INTERSEG);		/* TODO: Timing = 10 + m */
					if(PROTECTED_MODE && !V8086_MODE)
					{
						i386_protected_mode_call(cpustate,selector,address,1,1);
					}
					else
					{
						PUSH32(cpustate, cpustate->sreg[CS].selector );
						PUSH32(cpustate, cpustate->eip );
						cpustate->sreg[CS].selector = selector;
						cpustate->performed_intersegment_jump = 1;
						i386_load_segment_descriptor(cpustate, CS );
						cpustate->eip = address;
						CHANGE_PC(cpustate,cpustate->eip);
					}
				}
			}
			break;
		case 4:			/* JMP Rm32 */
			{
				UINT32 address;
				if( modrm >= 0xc0 ) {
					address = LOAD_RM32(modrm);
					CYCLES(cpustate,CYCLES_JMP_REG);		/* TODO: Timing = 7 + m */
				} else {
					UINT32 ea = GetEA(cpustate,modrm,0);
					address = READ32(cpustate,ea);
					CYCLES(cpustate,CYCLES_JMP_MEM);		/* TODO: Timing = 10 + m */
				}
				cpustate->eip = address;
				CHANGE_PC(cpustate,cpustate->eip);
			}
			break;
		case 5:			/* JMP FAR Rm32 */
			{
				UINT16 selector;
				UINT32 address;

				if( modrm >= 0xc0 )
				{
					fatalerror("i386: groupFF_32 /%d: NYI", (modrm >> 3) & 0x7);
				}
				else
				{
					UINT32 ea = GetEA(cpustate,modrm,0);
					address = READ32(cpustate,ea + 0);
					selector = READ16(cpustate,ea + 4);
					CYCLES(cpustate,CYCLES_JMP_MEM_INTERSEG);		/* TODO: Timing = 10 + m */
					if(PROTECTED_MODE && !V8086_MODE)
					{
						i386_protected_mode_jump(cpustate,selector,address,1,1);
					}
					else
					{
						cpustate->sreg[CS].selector = selector;
						cpustate->performed_intersegment_jump = 1;
						i386_load_segment_descriptor(cpustate, CS );
						cpustate->eip = address;
						CHANGE_PC(cpustate,cpustate->eip);
					}
				}
			}
			break;
		case 6:			/* PUSH Rm32 */
			{
				UINT32 value;
				if( modrm >= 0xc0 ) {
					value = LOAD_RM32(modrm);
				} else {
					UINT32 ea = GetEA(cpustate,modrm,0);
					value = READ32(cpustate,ea);
				}
				PUSH32(cpustate,value);
				CYCLES(cpustate,CYCLES_PUSH_RM);
			}
			break;
		default:
			fatalerror("i386: groupFF_32 /%d unimplemented at %08X", (modrm >> 3) & 0x7, cpustate->pc-2);
			break;
	}
}

static void I386OP(group0F00_32)(i386_state *cpustate)			// Opcode 0x0f 00
{
	UINT32 address, ea;
	UINT8 modrm = FETCH(cpustate);
	I386_SREG seg;
	UINT8 result;

	switch( (modrm >> 3) & 0x7 )
	{
		case 0:			/* SLDT */
			if ( PROTECTED_MODE && !V8086_MODE )
			{
				if( modrm >= 0xc0 ) {
					STORE_RM32(modrm, cpustate->ldtr.segment);
					CYCLES(cpustate,CYCLES_SLDT_REG);
				} else {
					ea = GetEA(cpustate,modrm,1);
					WRITE32(cpustate, ea, cpustate->ldtr.segment);
					CYCLES(cpustate,CYCLES_SLDT_MEM);
				}
			}
			else
			{
				i386_trap(cpustate,6, 0, 0);
			}
			break;
		case 1: 		/* STR */
			if ( PROTECTED_MODE && !V8086_MODE )
			{
				if( modrm >= 0xc0 ) {
					STORE_RM32(modrm, cpustate->task.segment);
					CYCLES(cpustate,CYCLES_STR_REG);
				} else {
					ea = GetEA(cpustate,modrm,1);
					WRITE32(cpustate, ea, cpustate->task.segment);
					CYCLES(cpustate,CYCLES_STR_MEM);
				}
			}
			else
			{
				i386_trap(cpustate,6, 0, 0);
			}
			break;
		case 2:			/* LLDT */
			if ( PROTECTED_MODE && !V8086_MODE )
			{
				if(cpustate->CPL)
					FAULT(FAULT_GP,0)
				if( modrm >= 0xc0 ) {
					address = LOAD_RM32(modrm);
					cpustate->ldtr.segment = address;
					CYCLES(cpustate,CYCLES_LLDT_REG);
				} else {
					ea = GetEA(cpustate,modrm,0);
					cpustate->ldtr.segment = READ32(cpustate,ea);
					CYCLES(cpustate,CYCLES_LLDT_MEM);
				}
				memset(&seg, 0, sizeof(seg));
				seg.selector = cpustate->ldtr.segment;
				i386_load_protected_mode_segment(cpustate,&seg,NULL);
				cpustate->ldtr.limit = seg.limit;
				cpustate->ldtr.base = seg.base;
				cpustate->ldtr.flags = seg.flags;
			}
			else
			{
				i386_trap(cpustate,6, 0, 0);
			}
			break;

		case 3:			/* LTR */
			if ( PROTECTED_MODE && !V8086_MODE )
			{
				if(cpustate->CPL)
					FAULT(FAULT_GP,0)
				if( modrm >= 0xc0 ) {
					address = LOAD_RM32(modrm);
					cpustate->task.segment = address;
					CYCLES(cpustate,CYCLES_LTR_REG);
				} else {
					ea = GetEA(cpustate,modrm,0);
					cpustate->task.segment = READ32(cpustate,ea);
					CYCLES(cpustate,CYCLES_LTR_MEM);
				}
				memset(&seg, 0, sizeof(seg));
				seg.selector = cpustate->task.segment;
				i386_load_protected_mode_segment(cpustate,&seg,NULL);
				cpustate->task.limit = seg.limit;
				cpustate->task.base = seg.base;
				cpustate->task.flags = seg.flags;
			}
			else
			{
				i386_trap(cpustate,6, 0, 0);
			}
			break;

		case 4:  /* VERR */
			if ( PROTECTED_MODE && !V8086_MODE )
			{
				if( modrm >= 0xc0 ) {
					address = LOAD_RM32(modrm);
					CYCLES(cpustate,CYCLES_VERR_REG);
				} else {
					ea = GetEA(cpustate,modrm,0);
					address = READ32(cpustate,ea);
					CYCLES(cpustate,CYCLES_VERR_MEM);
				}
				memset(&seg, 0, sizeof(seg));
				seg.selector = address;
				result = i386_load_protected_mode_segment(cpustate,&seg,NULL);
				// check if the segment is a code or data segment (not a special segment type, like a TSS, gate, LDT...)
				if(!(seg.flags & 0x10))
					result = 0;
				// check that the segment is readable
				if(seg.flags & 0x10)  // is code or data segment
				{
					if(seg.flags & 0x08)  // is code segment, so check if it's readable
					{
						if(!(seg.flags & 0x02))
						{
							result = 0;
						}
						else
						{  // check if conforming, these are always readable, regardless of privilege
							if(!(seg.flags & 0x04))
							{
								// if not conforming, then we must check privilege levels (TODO: current privilege level check)
								if(((seg.flags >> 5) & 0x03) < (address & 0x03))
									result = 0;
							}
						}
					}
				}
				// check that the descriptor privilege is greater or equal to the selector's privilege level and the current privilege (TODO)
				SetZF(result);
			}
			else
			{
				i386_trap(cpustate,6, 0, 0);
				logerror("i386: VERR: Exception - Running in real mode or virtual 8086 mode.\n");
			}
			break;

		case 5:  /* VERW */
			if ( PROTECTED_MODE && !V8086_MODE )
			{
				if( modrm >= 0xc0 ) {
					address = LOAD_RM16(modrm);
					CYCLES(cpustate,CYCLES_VERW_REG);
				} else {
					ea = GetEA(cpustate,modrm,0);
					address = READ16(cpustate,ea);
					CYCLES(cpustate,CYCLES_VERW_MEM);
				}
				memset(&seg, 0, sizeof(seg));
				seg.selector = address;
				result = i386_load_protected_mode_segment(cpustate,&seg,NULL);
				// check if the segment is a code or data segment (not a special segment type, like a TSS, gate, LDT...)
				if(!(seg.flags & 0x10))
					result = 0;
				// check that the segment is writable
				if(seg.flags & 0x10)  // is code or data segment
				{
					if(seg.flags & 0x08)  // is code segment (and thus, not writable)
					{
						result = 0;
					}
					else
					{  // is data segment
						if(!(seg.flags & 0x02))
							result = 0;
					}
				}
				// check that the descriptor privilege is greater or equal to the selector's privilege level and the current privilege (TODO)
				if(((seg.flags >> 5) & 0x03) < (address & 0x03))
					result = 0;
				SetZF(result);
			}
			else
			{
				i386_trap(cpustate,6, 0, 0);
				logerror("i386: VERW: Exception - Running in real mode or virtual 8086 mode.\n");
			}
			break;

		default:
			fatalerror("i386: group0F00_32 /%d unimplemented", (modrm >> 3) & 0x7);
			break;
	}
}

static void I386OP(group0F01_32)(i386_state *cpustate)		// Opcode 0x0f 01
{
	UINT8 modrm = FETCH(cpustate);
	UINT32 address, ea;

	switch( (modrm >> 3) & 0x7 )
	{
		case 0:			/* SGDT */
			{
				if( modrm >= 0xc0 ) {
					address = LOAD_RM32(modrm);
					ea = i386_translate(cpustate, CS, address, 1 );
				} else {
					ea = GetEA(cpustate,modrm,1);
				}
				WRITE16(cpustate,ea, cpustate->gdtr.limit);
				WRITE32(cpustate,ea + 2, cpustate->gdtr.base);
				CYCLES(cpustate,CYCLES_SGDT);
				break;
			}
		case 1:			/* SIDT */
			{
				if (modrm >= 0xc0)
				{
					address = LOAD_RM32(modrm);
					ea = i386_translate(cpustate, CS, address, 1 );
				}
				else
				{
					ea = GetEA(cpustate,modrm,1);
				}
				WRITE16(cpustate,ea, cpustate->idtr.limit);
				WRITE32(cpustate,ea + 2, cpustate->idtr.base);
				CYCLES(cpustate,CYCLES_SIDT);
				break;
			}
		case 2:			/* LGDT */
			{
				if(PROTECTED_MODE && cpustate->CPL)
					FAULT(FAULT_GP,0)
				if( modrm >= 0xc0 ) {
					address = LOAD_RM32(modrm);
					ea = i386_translate(cpustate, CS, address, 0 );
				} else {
					ea = GetEA(cpustate,modrm,0);
				}
				cpustate->gdtr.limit = READ16(cpustate,ea);
				cpustate->gdtr.base = READ32(cpustate,ea + 2);
				CYCLES(cpustate,CYCLES_LGDT);
				break;
			}
		case 3:			/* LIDT */
			{
				if(PROTECTED_MODE && cpustate->CPL)
					FAULT(FAULT_GP,0)
				if( modrm >= 0xc0 ) {
					address = LOAD_RM32(modrm);
					ea = i386_translate(cpustate, CS, address, 0 );
				} else {
					ea = GetEA(cpustate,modrm,0);
				}
				cpustate->idtr.limit = READ16(cpustate,ea);
				cpustate->idtr.base = READ32(cpustate,ea + 2);
				CYCLES(cpustate,CYCLES_LIDT);
				break;
			}
		case 4:			/* SMSW */
			{
				if( modrm >= 0xc0 ) {
					STORE_RM32(modrm, cpustate->cr[0] & 0xffff);
					CYCLES(cpustate,CYCLES_SMSW_REG);
				} else {
					/* always 16-bit memory operand */
					ea = GetEA(cpustate,modrm,1);
					WRITE16(cpustate,ea, cpustate->cr[0]);
					CYCLES(cpustate,CYCLES_SMSW_MEM);
				}
				break;
			}
		case 6:			/* LMSW */
			{
				if(PROTECTED_MODE && cpustate->CPL)
					FAULT(FAULT_GP,0)
				UINT16 b;
				if( modrm >= 0xc0 ) {
					b = LOAD_RM16(modrm);
					CYCLES(cpustate,CYCLES_LMSW_REG);
				} else {
					ea = GetEA(cpustate,modrm,0);
					CYCLES(cpustate,CYCLES_LMSW_MEM);
				b = READ16(cpustate,ea);
				}
				if(PROTECTED_MODE)
					b |= 0x0001;  // cannot return to real mode using this instruction.
				cpustate->cr[0] &= ~0x0000000f;
				cpustate->cr[0] |= b & 0x0000000f;
				break;
			}
		default:
			fatalerror("i386: unimplemented opcode 0x0f 01 /%d at %08X", (modrm >> 3) & 0x7, cpustate->eip - 2);
			break;
	}
}

static void I386OP(group0FBA_32)(i386_state *cpustate)		// Opcode 0x0f ba
{
	UINT8 modrm = FETCH(cpustate);

	switch( (modrm >> 3) & 0x7 )
	{
		case 4:			/* BT Rm32, i8 */
			if( modrm >= 0xc0 ) {
				UINT32 dst = LOAD_RM32(modrm);
				UINT8 bit = FETCH(cpustate);

				if( dst & (1 << bit) )
					cpustate->CF = 1;
				else
					cpustate->CF = 0;

				CYCLES(cpustate,CYCLES_BT_IMM_REG);
			} else {
				UINT32 ea = GetEA(cpustate,modrm,0);
				UINT32 dst = READ32(cpustate,ea);
				UINT8 bit = FETCH(cpustate);

				if( dst & (1 << bit) )
					cpustate->CF = 1;
				else
					cpustate->CF = 0;

				CYCLES(cpustate,CYCLES_BT_IMM_MEM);
			}
			break;
		case 5:			/* BTS Rm32, i8 */
			if( modrm >= 0xc0 ) {
				UINT32 dst = LOAD_RM32(modrm);
				UINT8 bit = FETCH(cpustate);

				if( dst & (1 << bit) )
					cpustate->CF = 1;
				else
					cpustate->CF = 0;
				dst |= (1 << bit);

				STORE_RM32(modrm, dst);
				CYCLES(cpustate,CYCLES_BTS_IMM_REG);
			} else {
				UINT32 ea = GetEA(cpustate,modrm,1);
				UINT32 dst = READ32(cpustate,ea);
				UINT8 bit = FETCH(cpustate);

				if( dst & (1 << bit) )
					cpustate->CF = 1;
				else
					cpustate->CF = 0;
				dst |= (1 << bit);

				WRITE32(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_BTS_IMM_MEM);
			}
			break;
		case 6:			/* BTR Rm32, i8 */
			if( modrm >= 0xc0 ) {
				UINT32 dst = LOAD_RM32(modrm);
				UINT8 bit = FETCH(cpustate);

				if( dst & (1 << bit) )
					cpustate->CF = 1;
				else
					cpustate->CF = 0;
				dst &= ~(1 << bit);

				STORE_RM32(modrm, dst);
				CYCLES(cpustate,CYCLES_BTR_IMM_REG);
			} else {
				UINT32 ea = GetEA(cpustate,modrm,1);
				UINT32 dst = READ32(cpustate,ea);
				UINT8 bit = FETCH(cpustate);

				if( dst & (1 << bit) )
					cpustate->CF = 1;
				else
					cpustate->CF = 0;
				dst &= ~(1 << bit);

				WRITE32(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_BTR_IMM_MEM);
			}
			break;
		case 7:			/* BTC Rm32, i8 */
			if( modrm >= 0xc0 ) {
				UINT32 dst = LOAD_RM32(modrm);
				UINT8 bit = FETCH(cpustate);

				if( dst & (1 << bit) )
					cpustate->CF = 1;
				else
					cpustate->CF = 0;
				dst ^= (1 << bit);

				STORE_RM32(modrm, dst);
				CYCLES(cpustate,CYCLES_BTC_IMM_REG);
			} else {
				UINT32 ea = GetEA(cpustate,modrm,1);
				UINT32 dst = READ32(cpustate,ea);
				UINT8 bit = FETCH(cpustate);

				if( dst & (1 << bit) )
					cpustate->CF = 1;
				else
					cpustate->CF = 0;
				dst ^= (1 << bit);

				WRITE32(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_BTC_IMM_MEM);
			}
			break;
		default:
			fatalerror("i386: group0FBA_32 /%d unknown", (modrm >> 3) & 0x7);
			break;
	}
}

static void I386OP(lar_r32_rm32)(i386_state *cpustate)  // Opcode 0x0f 0x02
{
	UINT8 modrm = FETCH(cpustate);
	I386_SREG seg;
	UINT8 type;

	if(PROTECTED_MODE && !V8086_MODE)
	{
		memset(&seg,0,sizeof(seg));
		if(modrm >= 0xc0)
		{
			seg.selector = LOAD_RM32(modrm);
			CYCLES(cpustate,CYCLES_LAR_REG);
		}
		else
		{
			UINT32 ea = GetEA(cpustate,modrm,0);
			seg.selector = READ32(cpustate,ea);
			CYCLES(cpustate,CYCLES_LAR_MEM);
		}
		if(seg.selector == 0)
		{
			SetZF(0);  // not a valid segment
		}
		else
		{
			UINT64 desc;
			if(!i386_load_protected_mode_segment(cpustate,&seg,&desc))
			{
				SetZF(0);
				return;
			}
			if((((seg.flags >> 5) & 3) < cpustate->CPL) && ((seg.flags & 0x1c) != 0x1c))
			{
				SetZF(0);
				return;
			}
			if(!(seg.flags & 0x10))  // special segment
			{
				// check for invalid segment types
				type = seg.flags & 0x000f;
				if(type == 0x00 || type == 0x08 || type == 0x0a || type == 0x0d)
				{
					SetZF(0);  // invalid segment type
				}
				else
				{
					STORE_REG32(modrm,(desc>>32) & 0x00ffff00);
					SetZF(1);
				}
			}
			else
			{
				STORE_REG32(modrm,(desc>>32) & 0x00ffff00);
				SetZF(1);
			}
		}
	}
	else
	{
		// illegal opcode
		i386_trap(cpustate,6,0, 0);
		logerror("i386: LAR: Exception - running in real mode or virtual 8086 mode.\n");
	}
}

static void I386OP(lsl_r32_rm32)(i386_state *cpustate)  // Opcode 0x0f 0x03
{
	UINT8 modrm = FETCH(cpustate);
	UINT32 limit;
	I386_SREG seg;

	if(PROTECTED_MODE && !V8086_MODE)
	{
		memset(&seg, 0, sizeof(seg));
		if(modrm >= 0xc0)
		{
			seg.selector = LOAD_RM32(modrm);
		}
		else
		{
			UINT32 ea = GetEA(cpustate,modrm,0);
			seg.selector = READ32(cpustate,ea);
		}
		if(seg.selector == 0)
		{
			SetZF(0);  // not a valid segment
		}
		else
		{
			UINT8 type;
			if(!i386_load_protected_mode_segment(cpustate,&seg,NULL))
			{
				SetZF(0);
				return;
			}
			if((((seg.flags >> 5) & 3) < cpustate->CPL) && ((seg.flags & 0x1c) != 0x1c))
			{
				SetZF(0);
				return;
			}
			type = seg.flags & 0x1f;
			switch(type)
			{
			case 0:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 10:
			case 12:
			case 13:
			case 14:
			case 15:
				SetZF(0);
				return;
			default:
				limit = seg.limit;
				STORE_REG32(modrm,limit);
				SetZF(1);
			}
		}
	}
	else
		i386_trap(cpustate,6, 0, 0);
}

static void I386OP(bound_r32_m32_m32)(i386_state *cpustate)	// Opcode 0x62
{
	UINT8 modrm;
	INT32 val, low, high;

	modrm = FETCH(cpustate);

	if (modrm >= 0xc0)
	{
		low = high = LOAD_RM32(modrm);
	}
	else
	{
		UINT32 ea = GetEA(cpustate,modrm,0);
		low = READ32(cpustate,ea + 0);
		high = READ32(cpustate,ea + 4);
	}
	val = LOAD_REG32(modrm);

	if ((val < low) || (val > high))
	{
		CYCLES(cpustate,CYCLES_BOUND_OUT_RANGE);
		i386_trap(cpustate,5, 0, 0);
	}
	else
	{
		CYCLES(cpustate,CYCLES_BOUND_IN_RANGE);
	}
}

static void I386OP(retf32)(i386_state *cpustate)			// Opcode 0xcb
{
	if(PROTECTED_MODE && !V8086_MODE)
	{
		i386_protected_mode_retf(cpustate,0,1);
	}
	else
	{
		cpustate->eip = POP32(cpustate);
		cpustate->sreg[CS].selector = POP32(cpustate);
		i386_load_segment_descriptor(cpustate, CS );
		CHANGE_PC(cpustate,cpustate->eip);
	}

	CYCLES(cpustate,CYCLES_RET_INTERSEG);
}

static void I386OP(retf_i32)(i386_state *cpustate)			// Opcode 0xca
{
	UINT16 count = FETCH16(cpustate);

	if(PROTECTED_MODE && !V8086_MODE)
	{
		i386_protected_mode_retf(cpustate,count,1);
	}
	else
	{
		cpustate->eip = POP32(cpustate);
		cpustate->sreg[CS].selector = POP32(cpustate);
		i386_load_segment_descriptor(cpustate, CS );
		CHANGE_PC(cpustate,cpustate->eip);
		REG32(ESP) += count;
	}

	CYCLES(cpustate,CYCLES_RET_IMM_INTERSEG);
}

static void I386OP(load_far_pointer32)(i386_state *cpustate, int s)
{
	UINT8 modrm = FETCH(cpustate);
	UINT16 selector;

	if( modrm >= 0xc0 ) {
		fatalerror("i386: load_far_pointer32 NYI");
	} else {
		UINT32 ea = GetEA(cpustate,modrm,0);
		STORE_REG32(modrm, READ32(cpustate,ea + 0));
		selector = READ16(cpustate,ea + 4);
		i386_sreg_load(cpustate,selector,s,NULL);
	}
}

static void I386OP(lds32)(i386_state *cpustate)				// Opcode 0xc5
{
	I386OP(load_far_pointer32)(cpustate, DS);
	CYCLES(cpustate,CYCLES_LDS);
}

static void I386OP(lss32)(i386_state *cpustate)				// Opcode 0x0f 0xb2
{
	I386OP(load_far_pointer32)(cpustate, SS);
	CYCLES(cpustate,CYCLES_LSS);
}

static void I386OP(les32)(i386_state *cpustate)				// Opcode 0xc4
{
	I386OP(load_far_pointer32)(cpustate, ES);
	CYCLES(cpustate,CYCLES_LES);
}

static void I386OP(lfs32)(i386_state *cpustate)				// Opcode 0x0f 0xb4
{
	I386OP(load_far_pointer32)(cpustate, FS);
	CYCLES(cpustate,CYCLES_LFS);
}

static void I386OP(lgs32)(i386_state *cpustate)				// Opcode 0x0f 0xb5
{
	I386OP(load_far_pointer32)(cpustate, GS);
	CYCLES(cpustate,CYCLES_LGS);
}
