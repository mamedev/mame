static UINT32 I386OP(shift_rotate32)(UINT8 modrm, UINT32 value, UINT8 shift)
{
	UINT32 dst, src;
	dst = value;
	src = value;

	if( shift == 0 ) {
		CYCLES_RM(modrm, 3, 7);
	} else if( shift == 1 ) {

		switch( (modrm >> 3) & 0x7 )
		{
			case 0:			/* ROL rm32, 1 */
				I.CF = (src & 0x80000000) ? 1 : 0;
				dst = (src << 1) + I.CF;
				I.OF = ((src ^ dst) & 0x80000000) ? 1 : 0;
				CYCLES_RM(modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 1:			/* ROR rm32, 1 */
				I.CF = (src & 0x1) ? 1 : 0;
				dst = (I.CF << 31) | (src >> 1);
				I.OF = ((src ^ dst) & 0x80000000) ? 1 : 0;
				CYCLES_RM(modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 2:			/* RCL rm32, 1 */
				dst = (src << 1) + I.CF;
				I.CF = (src & 0x80000000) ? 1 : 0;
				I.OF = ((src ^ dst) & 0x80000000) ? 1 : 0;
				CYCLES_RM(modrm, CYCLES_ROTATE_CARRY_REG, CYCLES_ROTATE_CARRY_MEM);
				break;
			case 3:			/* RCR rm32, 1 */
				dst = (I.CF << 31) | (src >> 1);
				I.CF = src & 0x1;
				I.OF = ((src ^ dst) & 0x80000000) ? 1 : 0;
				CYCLES_RM(modrm, CYCLES_ROTATE_CARRY_REG, CYCLES_ROTATE_CARRY_MEM);
				break;
			case 4:			/* SHL/SAL rm32, 1 */
			case 6:
				dst = src << 1;
				I.CF = (src & 0x80000000) ? 1 : 0;
				I.OF = (((I.CF << 31) ^ dst) & 0x80000000) ? 1 : 0;
				SetSZPF32(dst);
				CYCLES_RM(modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 5:			/* SHR rm32, 1 */
				dst = src >> 1;
				I.CF = src & 0x1;
				I.OF = (src & 0x80000000) ? 1 : 0;
				SetSZPF32(dst);
				CYCLES_RM(modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 7:			/* SAR rm32, 1 */
				dst = (INT32)(src) >> 1;
				I.CF = src & 0x1;
				I.OF = 0;
				SetSZPF32(dst);
				CYCLES_RM(modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
		}

	} else {

		switch( (modrm >> 3) & 0x7 )
		{
			case 0:			/* ROL rm32, i8 */
				dst = ((src & ((UINT32)0xffffffff >> shift)) << shift) |
					  ((src & ((UINT32)0xffffffff << (32-shift))) >> (32-shift));
				I.CF = (src >> (32-shift)) & 0x1;
				CYCLES_RM(modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 1:			/* ROR rm32, i8 */
				dst = ((src & ((UINT32)0xffffffff << shift)) >> shift) |
					  ((src & ((UINT32)0xffffffff >> (32-shift))) << (32-shift));
				I.CF = (src >> (shift-1)) & 0x1;
				CYCLES_RM(modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 2:			/* RCL rm32, i8 */
				dst = ((src & ((UINT32)0xffffffff >> shift)) << shift) |
					  ((src & ((UINT32)0xffffffff << (33-shift))) >> (33-shift)) |
					  (I.CF << (shift-1));
				I.CF = (src >> (32-shift)) & 0x1;
				CYCLES_RM(modrm, CYCLES_ROTATE_CARRY_REG, CYCLES_ROTATE_CARRY_MEM);
				break;
			case 3:			/* RCR rm32, i8 */
				dst = ((src & ((UINT32)0xffffffff << shift)) >> shift) |
					  ((src & ((UINT32)0xffffffff >> (32-shift))) << (33-shift)) |
					  (I.CF << (32-shift));
				I.CF = (src >> (shift-1)) & 0x1;
				CYCLES_RM(modrm, CYCLES_ROTATE_CARRY_REG, CYCLES_ROTATE_CARRY_MEM);
				break;
			case 4:			/* SHL/SAL rm32, i8 */
			case 6:
				dst = src << shift;
				I.CF = (src & (1 << (32-shift))) ? 1 : 0;
				SetSZPF32(dst);
				CYCLES_RM(modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 5:			/* SHR rm32, i8 */
				dst = src >> shift;
				I.CF = (src & (1 << (shift-1))) ? 1 : 0;
				SetSZPF32(dst);
				CYCLES_RM(modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 7:			/* SAR rm32, i8 */
				dst = (INT32)src >> shift;
				I.CF = (src & (1 << (shift-1))) ? 1 : 0;
				SetSZPF32(dst);
				CYCLES_RM(modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
		}

	}
	return dst;
}



static void I386OP(adc_rm32_r32)(void)		// Opcode 0x11
{
	UINT32 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_REG32(modrm);
		dst = LOAD_RM32(modrm);
		src = ADD32(src, I.CF);
		dst = ADD32(dst, src);
		STORE_RM32(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		src = LOAD_REG32(modrm);
		dst = READ32(ea);
		src = ADD32(src, I.CF);
		dst = ADD32(dst, src);
		WRITE32(ea, dst);
		CYCLES(CYCLES_ALU_REG_MEM);
	}
}

static void I386OP(adc_r32_rm32)(void)		// Opcode 0x13
{
	UINT32 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_RM32(modrm);
		dst = LOAD_REG32(modrm);
		src = ADD32(src, I.CF);
		dst = ADD32(dst, src);
		STORE_REG32(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		src = READ32(ea);
		dst = LOAD_REG32(modrm);
		src = ADD32(src, I.CF);
		dst = ADD32(dst, src);
		STORE_REG32(modrm, dst);
		CYCLES(CYCLES_ALU_MEM_REG);
	}
}

static void I386OP(adc_eax_i32)(void)		// Opcode 0x15
{
	UINT32 src, dst;
	src = FETCH32();
	dst = REG32(EAX);
	src = ADD32(src, I.CF);
	dst = ADD32(dst, src);
	REG32(EAX) = dst;
	CYCLES(CYCLES_ALU_IMM_ACC);
}

static void I386OP(add_rm32_r32)(void)		// Opcode 0x01
{
	UINT32 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_REG32(modrm);
		dst = LOAD_RM32(modrm);
		dst = ADD32(dst, src);
		STORE_RM32(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		src = LOAD_REG32(modrm);
		dst = READ32(ea);
		dst = ADD32(dst, src);
		WRITE32(ea, dst);
		CYCLES(CYCLES_ALU_REG_MEM);
	}
}

static void I386OP(add_r32_rm32)(void)		// Opcode 0x03
{
	UINT32 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_RM32(modrm);
		dst = LOAD_REG32(modrm);
		dst = ADD32(dst, src);
		STORE_REG32(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		src = READ32(ea);
		dst = LOAD_REG32(modrm);
		dst = ADD32(dst, src);
		STORE_REG32(modrm, dst);
		CYCLES(CYCLES_ALU_MEM_REG);
	}
}

static void I386OP(add_eax_i32)(void)		// Opcode 0x05
{
	UINT32 src, dst;
	src = FETCH32();
	dst = REG32(EAX);
	dst = ADD32(dst, src);
	REG32(EAX) = dst;
	CYCLES(CYCLES_ALU_IMM_ACC);
}

static void I386OP(and_rm32_r32)(void)		// Opcode 0x21
{
	UINT32 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_REG32(modrm);
		dst = LOAD_RM32(modrm);
		dst = AND32(dst, src);
		STORE_RM32(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		src = LOAD_REG32(modrm);
		dst = READ32(ea);
		dst = AND32(dst, src);
		WRITE32(ea, dst);
		CYCLES(CYCLES_ALU_REG_MEM);
	}
}

static void I386OP(and_r32_rm32)(void)		// Opcode 0x23
{
	UINT32 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_RM32(modrm);
		dst = LOAD_REG32(modrm);
		dst = AND32(dst, src);
		STORE_REG32(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		src = READ32(ea);
		dst = LOAD_REG32(modrm);
		dst = AND32(dst, src);
		STORE_REG32(modrm, dst);
		CYCLES(CYCLES_ALU_MEM_REG);
	}
}

static void I386OP(and_eax_i32)(void)		// Opcode 0x25
{
	UINT32 src, dst;
	src = FETCH32();
	dst = REG32(EAX);
	dst = AND32(dst, src);
	REG32(EAX) = dst;
	CYCLES(CYCLES_ALU_IMM_ACC);
}

static void I386OP(bsf_r32_rm32)(void)		// Opcode 0x0f bc
{
	UINT32 src, dst, temp;
	UINT8 modrm = FETCH();

	if( modrm >= 0xc0 ) {
		src = LOAD_RM32(modrm);
	} else {
		UINT32 ea = GetEA(modrm);
		src = READ32(ea);
	}

	dst = 0;

	if( src == 0 ) {
		I.ZF = 1;
	} else {
		I.ZF = 0;
		temp = 0;
		while( (src & (1 << temp)) == 0 ) {
			temp++;
			dst = temp;
			CYCLES(CYCLES_BSF);
		}
	}
	CYCLES(CYCLES_BSF_BASE);
	STORE_REG32(modrm, dst);
}

static void I386OP(bsr_r32_rm32)(void)		// Opcode 0x0f bd
{
	UINT32 src, dst, temp;
	UINT8 modrm = FETCH();

	if( modrm >= 0xc0 ) {
		src = LOAD_RM32(modrm);
	} else {
		UINT32 ea = GetEA(modrm);
		src = READ32(ea);
	}

	dst = 0;

	if( src == 0 ) {
		I.ZF = 1;
	} else {
		I.ZF = 0;
		temp = 31;
		while( (src & (1 << temp)) == 0 ) {
			temp--;
			dst = temp;
			CYCLES(CYCLES_BSR);
		}
	}
	CYCLES(CYCLES_BSR_BASE);
	STORE_REG32(modrm, dst);
}

static void I386OP(bt_rm32_r32)(void)		// Opcode 0x0f a3
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		UINT32 dst = LOAD_RM32(modrm);
		UINT32 bit = LOAD_REG32(modrm);

		if( dst & (1 << bit) )
			I.CF = 1;
		else
			I.CF = 0;

		CYCLES(CYCLES_BT_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		UINT32 dst = READ32(ea);
		UINT32 bit = LOAD_REG32(modrm);

		if( dst & (1 << bit) )
			I.CF = 1;
		else
			I.CF = 0;

		CYCLES(CYCLES_BT_REG_MEM);
	}
}

static void I386OP(btc_rm32_r32)(void)		// Opcode 0x0f bb
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		UINT32 dst = LOAD_RM32(modrm);
		UINT32 bit = LOAD_REG32(modrm);

		if( dst & (1 << bit) )
			I.CF = 1;
		else
			I.CF = 0;
		dst ^= (1 << bit);

		STORE_RM32(modrm, dst);
		CYCLES(CYCLES_BTC_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		UINT32 dst = READ32(ea);
		UINT32 bit = LOAD_REG32(modrm);

		if( dst & (1 << bit) )
			I.CF = 1;
		else
			I.CF = 0;
		dst ^= (1 << bit);

		WRITE32(ea, dst);
		CYCLES(CYCLES_BTC_REG_MEM);
	}
}

static void I386OP(btr_rm32_r32)(void)		// Opcode 0x0f b3
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		UINT32 dst = LOAD_RM32(modrm);
		UINT32 bit = LOAD_REG32(modrm);

		if( dst & (1 << bit) )
			I.CF = 1;
		else
			I.CF = 0;
		dst &= ~(1 << bit);

		STORE_RM32(modrm, dst);
		CYCLES(CYCLES_BTR_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		UINT32 dst = READ32(ea);
		UINT32 bit = LOAD_REG32(modrm);

		if( dst & (1 << bit) )
			I.CF = 1;
		else
			I.CF = 0;
		dst &= ~(1 << bit);

		WRITE32(ea, dst);
		CYCLES(CYCLES_BTR_REG_MEM);
	}
}

static void I386OP(bts_rm32_r32)(void)		// Opcode 0x0f ab
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		UINT32 dst = LOAD_RM32(modrm);
		UINT32 bit = LOAD_REG32(modrm);

		if( dst & (1 << bit) )
			I.CF = 1;
		else
			I.CF = 0;
		dst |= (1 << bit);

		STORE_RM32(modrm, dst);
		CYCLES(CYCLES_BTS_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		UINT32 dst = READ32(ea);
		UINT32 bit = LOAD_REG32(modrm);

		if( dst & (1 << bit) )
			I.CF = 1;
		else
			I.CF = 0;
		dst |= (1 << bit);

		WRITE32(ea, dst);
		CYCLES(CYCLES_BTS_REG_MEM);
	}
}

static void I386OP(call_abs32)(void)		// Opcode 0x9a
{
	UINT32 offset = FETCH32();
	UINT16 ptr = FETCH16();

	if( PROTECTED_MODE ) {
		/* TODO */
		fatalerror("i386: call_abs32 in protected mode unimplemented");
	} else {
		PUSH32( I.sreg[CS].selector );
		PUSH32( I.eip );
		I.sreg[CS].selector = ptr;
		I.eip = offset;
		i386_load_segment_descriptor(CS);
	}
	CYCLES(CYCLES_CALL_INTERSEG);
	CHANGE_PC(I.eip);
}

static void I386OP(call_rel32)(void)		// Opcode 0xe8
{
	INT32 disp = FETCH32();

	PUSH32( I.eip );
	I.eip += disp;
	CHANGE_PC(I.eip);
	CYCLES(CYCLES_CALL);		/* TODO: Timing = 7 + m */
}

static void I386OP(cdq)(void)				// Opcode 0x99
{
	if( REG32(EAX) & 0x80000000 ) {
		REG32(EDX) = 0xffffffff;
	} else {
		REG32(EDX) = 0x00000000;
	}
	CYCLES(CYCLES_CWD);
}

static void I386OP(cmp_rm32_r32)(void)		// Opcode 0x39
{
	UINT32 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_REG32(modrm);
		dst = LOAD_RM32(modrm);
		SUB32(dst, src);
		CYCLES(CYCLES_CMP_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		src = LOAD_REG32(modrm);
		dst = READ32(ea);
		SUB32(dst, src);
		CYCLES(CYCLES_CMP_REG_MEM);
	}
}

static void I386OP(cmp_r32_rm32)(void)		// Opcode 0x3b
{
	UINT32 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_RM32(modrm);
		dst = LOAD_REG32(modrm);
		SUB32(dst, src);
		CYCLES(CYCLES_CMP_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		src = READ32(ea);
		dst = LOAD_REG32(modrm);
		SUB32(dst, src);
		CYCLES(CYCLES_CMP_MEM_REG);
	}
}

static void I386OP(cmp_eax_i32)(void)		// Opcode 0x3d
{
	UINT32 src, dst;
	src = FETCH32();
	dst = REG32(EAX);
	SUB32(dst, src);
	CYCLES(CYCLES_CMP_IMM_ACC);
}

static void I386OP(cmpsd)(void)				// Opcode 0xa7
{
	UINT32 eas, ead, src, dst;
	if( I.segment_prefix ) {
		eas = i386_translate( I.segment_override, I.address_size ? REG32(ESI) : REG16(SI) );
	} else {
		eas = i386_translate( DS, I.address_size ? REG32(ESI) : REG16(SI) );
	}
	ead = i386_translate( ES, I.address_size ? REG32(EDI) : REG16(DI) );
	src = READ32(eas);
	dst = READ32(ead);
	SUB32(dst, src);
	BUMP_SI(4);
	BUMP_DI(4);
	CYCLES(CYCLES_CMPS);
}

static void I386OP(cwde)(void)				// Opcode 0x98
{
	REG32(EAX) = (INT32)((INT16)REG16(AX));
	CYCLES(CYCLES_CBW);
}

static void I386OP(dec_eax)(void)			// Opcode 0x48
{
	REG32(EAX) = DEC32( REG32(EAX) );
	CYCLES(CYCLES_DEC_REG);
}

static void I386OP(dec_ecx)(void)			// Opcode 0x49
{
	REG32(ECX) = DEC32( REG32(ECX) );
	CYCLES(CYCLES_DEC_REG);
}

static void I386OP(dec_edx)(void)			// Opcode 0x4a
{
	REG32(EDX) = DEC32( REG32(EDX) );
	CYCLES(CYCLES_DEC_REG);
}

static void I386OP(dec_ebx)(void)			// Opcode 0x4b
{
	REG32(EBX) = DEC32( REG32(EBX) );
	CYCLES(CYCLES_DEC_REG);
}

static void I386OP(dec_esp)(void)			// Opcode 0x4c
{
	REG32(ESP) = DEC32( REG32(ESP) );
	CYCLES(CYCLES_DEC_REG);
}

static void I386OP(dec_ebp)(void)			// Opcode 0x4d
{
	REG32(EBP) = DEC32( REG32(EBP) );
	CYCLES(CYCLES_DEC_REG);
}

static void I386OP(dec_esi)(void)			// Opcode 0x4e
{
	REG32(ESI) = DEC32( REG32(ESI) );
	CYCLES(CYCLES_DEC_REG);
}

static void I386OP(dec_edi)(void)			// Opcode 0x4f
{
	REG32(EDI) = DEC32( REG32(EDI) );
	CYCLES(CYCLES_DEC_REG);
}

static void I386OP(imul_r32_rm32)(void)		// Opcode 0x0f af
{
	UINT8 modrm = FETCH();
	INT64 result;
	INT64 src, dst;
	if( modrm >= 0xc0 ) {
		src = (INT64)(INT32)LOAD_RM32(modrm);
		CYCLES(CYCLES_IMUL32_REG_REG);		/* TODO: Correct multiply timing */
	} else {
		UINT32 ea = GetEA(modrm);
		src = (INT64)(INT32)READ32(ea);
		CYCLES(CYCLES_IMUL32_REG_REG);		/* TODO: Correct multiply timing */
	}

	dst = (INT64)(INT32)LOAD_REG32(modrm);
	result = src * dst;

	STORE_REG32(modrm, (UINT32)result);

	I.CF = I.OF = !(result == (INT64)(INT32)result);
}

static void I386OP(imul_r32_rm32_i32)(void)	// Opcode 0x69
{
	UINT8 modrm = FETCH();
	INT64 result;
	INT64 src, dst;
	if( modrm >= 0xc0 ) {
		dst = (INT64)(INT32)LOAD_RM32(modrm);
		CYCLES(CYCLES_IMUL32_REG_IMM_REG);		/* TODO: Correct multiply timing */
	} else {
		UINT32 ea = GetEA(modrm);
		dst = (INT64)(INT32)READ32(ea);
		CYCLES(CYCLES_IMUL32_MEM_IMM_REG);		/* TODO: Correct multiply timing */
	}

	src = (INT64)(INT32)FETCH32();
	result = src * dst;

	STORE_REG32(modrm, (UINT32)result);

	I.CF = I.OF = !(result == (INT64)(INT32)result);
}

static void I386OP(imul_r32_rm32_i8)(void)	// Opcode 0x6b
{
	UINT8 modrm = FETCH();
	INT64 result;
	INT64 src, dst;
	if( modrm >= 0xc0 ) {
		dst = (INT64)(INT32)LOAD_RM32(modrm);
		CYCLES(CYCLES_IMUL32_REG_IMM_REG);		/* TODO: Correct multiply timing */
	} else {
		UINT32 ea = GetEA(modrm);
		dst = (INT64)(INT32)READ32(ea);
		CYCLES(CYCLES_IMUL32_MEM_IMM_REG);		/* TODO: Correct multiply timing */
	}

	src = (INT64)(INT8)FETCH();
	result = src * dst;

	STORE_REG32(modrm, (UINT32)result);

	I.CF = I.OF = !(result == (INT64)(INT32)result);
}

static void I386OP(in_eax_i8)(void)			// Opcode 0xe5
{
	UINT16 port = FETCH();
	UINT32 data = READPORT32(port);
	REG32(EAX) = data;
	CYCLES(CYCLES_IN_VAR);
}

static void I386OP(in_eax_dx)(void)			// Opcode 0xed
{
	UINT16 port = REG16(DX);
	UINT32 data = READPORT32(port);
	REG32(EAX) = data;
	CYCLES(CYCLES_IN);
}

static void I386OP(inc_eax)(void)			// Opcode 0x40
{
	REG32(EAX) = INC32( REG32(EAX) );
	CYCLES(CYCLES_INC_REG);
}

static void I386OP(inc_ecx)(void)			// Opcode 0x41
{
	REG32(ECX) = INC32( REG32(ECX) );
	CYCLES(CYCLES_INC_REG);
}

static void I386OP(inc_edx)(void)			// Opcode 0x42
{
	REG32(EDX) = INC32( REG32(EDX) );
	CYCLES(CYCLES_INC_REG);
}

static void I386OP(inc_ebx)(void)			// Opcode 0x43
{
	REG32(EBX) = INC32( REG32(EBX) );
	CYCLES(CYCLES_INC_REG);
}

static void I386OP(inc_esp)(void)			// Opcode 0x44
{
	REG32(ESP) = INC32( REG32(ESP) );
	CYCLES(CYCLES_INC_REG);
}

static void I386OP(inc_ebp)(void)			// Opcode 0x45
{
	REG32(EBP) = INC32( REG32(EBP) );
	CYCLES(CYCLES_INC_REG);
}

static void I386OP(inc_esi)(void)			// Opcode 0x46
{
	REG32(ESI) = INC32( REG32(ESI) );
	CYCLES(CYCLES_INC_REG);
}

static void I386OP(inc_edi)(void)			// Opcode 0x47
{
	REG32(EDI) = INC32( REG32(EDI) );
	CYCLES(CYCLES_INC_REG);
}

static void I386OP(iret32)(void)			// Opcode 0xcf
{
	if( PROTECTED_MODE ) {
		/* TODO: Virtual 8086-mode */
		/* TODO: Nested task */
		/* TODO: #SS(0) exception */
		/* TODO: All the protection-related stuff... */
		I.eip = POP32();
		I.sreg[CS].selector = POP32() & 0xffff;
		set_flags( POP32() );
		i386_load_segment_descriptor(CS);
		CHANGE_PC(I.eip);
	} else {
		/* TODO: #SS(0) exception */
		/* TODO: #GP(0) exception */
		I.eip = POP32();
		I.sreg[CS].selector = POP32() & 0xffff;
		set_flags( POP32() );
		i386_load_segment_descriptor(CS);
		CHANGE_PC(I.eip);
	}
	CYCLES(CYCLES_IRET);
}

static void I386OP(ja_rel32)(void)			// Opcode 0x0f 87
{
	INT32 disp = FETCH32();
	if( I.CF == 0 && I.ZF == 0 ) {
		I.eip += disp;
		CHANGE_PC(I.eip);
		CYCLES(CYCLES_JCC_FULL_DISP);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jbe_rel32)(void)			// Opcode 0x0f 86
{
	INT32 disp = FETCH32();
	if( I.CF != 0 || I.ZF != 0 ) {
		I.eip += disp;
		CHANGE_PC(I.eip);
		CYCLES(CYCLES_JCC_FULL_DISP);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jc_rel32)(void)			// Opcode 0x0f 82
{
	INT32 disp = FETCH32();
	if( I.CF != 0 ) {
		I.eip += disp;
		CHANGE_PC(I.eip);
		CYCLES(CYCLES_JCC_FULL_DISP);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jg_rel32)(void)			// Opcode 0x0f 8f
{
	INT32 disp = FETCH32();
	if( I.ZF == 0 && (I.SF == I.OF) ) {
		I.eip += disp;
		CHANGE_PC(I.eip);
		CYCLES(CYCLES_JCC_FULL_DISP);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jge_rel32)(void)			// Opcode 0x0f 8d
{
	INT32 disp = FETCH32();
	if( (I.SF == I.OF) ) {
		I.eip += disp;
		CHANGE_PC(I.eip);
		CYCLES(CYCLES_JCC_FULL_DISP);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jl_rel32)(void)			// Opcode 0x0f 8c
{
	INT32 disp = FETCH32();
	if( (I.SF != I.OF) ) {
		I.eip += disp;
		CHANGE_PC(I.eip);
		CYCLES(CYCLES_JCC_FULL_DISP);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jle_rel32)(void)			// Opcode 0x0f 8e
{
	INT32 disp = FETCH32();
	if( I.ZF != 0 || (I.SF != I.OF) ) {
		I.eip += disp;
		CHANGE_PC(I.eip);
		CYCLES(CYCLES_JCC_FULL_DISP);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jnc_rel32)(void)			// Opcode 0x0f 83
{
	INT32 disp = FETCH32();
	if( I.CF == 0 ) {
		I.eip += disp;
		CHANGE_PC(I.eip);
		CYCLES(CYCLES_JCC_FULL_DISP);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jno_rel32)(void)			// Opcode 0x0f 81
{
	INT32 disp = FETCH32();
	if( I.OF == 0 ) {
		I.eip += disp;
		CHANGE_PC(I.eip);
		CYCLES(CYCLES_JCC_FULL_DISP);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jnp_rel32)(void)			// Opcode 0x0f 8b
{
	INT32 disp = FETCH32();
	if( I.PF == 0 ) {
		I.eip += disp;
		CHANGE_PC(I.eip);
		CYCLES(CYCLES_JCC_FULL_DISP);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jns_rel32)(void)			// Opcode 0x0f 89
{
	INT32 disp = FETCH32();
	if( I.SF == 0 ) {
		I.eip += disp;
		CHANGE_PC(I.eip);
		CYCLES(CYCLES_JCC_FULL_DISP);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jnz_rel32)(void)			// Opcode 0x0f 85
{
	INT32 disp = FETCH32();
	if( I.ZF == 0 ) {
		I.eip += disp;
		CHANGE_PC(I.eip);
		CYCLES(CYCLES_JCC_FULL_DISP);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jo_rel32)(void)			// Opcode 0x0f 80
{
	INT32 disp = FETCH32();
	if( I.OF != 0 ) {
		I.eip += disp;
		CHANGE_PC(I.eip);
		CYCLES(CYCLES_JCC_FULL_DISP);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jp_rel32)(void)			// Opcode 0x0f 8a
{
	INT32 disp = FETCH32();
	if( I.PF != 0 ) {
		I.eip += disp;
		CHANGE_PC(I.eip);
		CYCLES(CYCLES_JCC_FULL_DISP);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(js_rel32)(void)			// Opcode 0x0f 88
{
	INT32 disp = FETCH32();
	if( I.SF != 0 ) {
		I.eip += disp;
		CHANGE_PC(I.eip);
		CYCLES(CYCLES_JCC_FULL_DISP);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jz_rel32)(void)			// Opcode 0x0f 84
{
	INT32 disp = FETCH32();
	if( I.ZF != 0 ) {
		I.eip += disp;
		CHANGE_PC(I.eip);
		CYCLES(CYCLES_JCC_FULL_DISP);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jcxz32)(void)			// Opcode 0xe3
{
	INT8 disp = FETCH();
	if( REG32(ECX) == 0 ) {
		I.eip += disp;
		CHANGE_PC(I.eip);
		CYCLES(CYCLES_JCXZ);		/* TODO: Timing = 9 + m */
	} else {
		CYCLES(CYCLES_JCXZ_NOBRANCH);
	}
}

static void I386OP(jmp_rel32)(void)			// Opcode 0xe9
{
	UINT32 disp = FETCH32();
	/* TODO: Segment limit */
	I.eip += disp;
	CHANGE_PC(I.eip);
	CYCLES(CYCLES_JMP);		/* TODO: Timing = 7 + m */
}

static void I386OP(jmp_abs32)(void)			// Opcode 0xea
{
	UINT32 address = FETCH32();
	UINT16 segment = FETCH16();

	if( PROTECTED_MODE ) {
		/* TODO: #GP */
		/* TODO: access rights, etc. */
		I.eip = address;
		I.sreg[CS].selector = segment;
		i386_load_segment_descriptor(CS);
		CHANGE_PC(I.eip);
	} else {
		I.eip = address;
		I.sreg[CS].selector = segment;
		i386_load_segment_descriptor(CS);
		CHANGE_PC(I.eip);
	}
	CYCLES(CYCLES_JMP_INTERSEG);
}

static void I386OP(lea32)(void)				// Opcode 0x8d
{
	UINT8 modrm = FETCH();
	UINT32 ea = GetNonTranslatedEA(modrm);
	if (!I.address_size)
	{
		ea &= 0xffff;
	}
	STORE_REG32(modrm, ea);
	CYCLES(CYCLES_LEA);
}

static void I386OP(leave32)(void)			// Opcode 0xc9
{
	REG32(ESP) = REG32(EBP);
	REG32(EBP) = POP32();
	CYCLES(CYCLES_LEAVE);
}

static void I386OP(lodsd)(void)				// Opcode 0xad
{
	UINT32 eas;
	if( I.segment_prefix ) {
		eas = i386_translate( I.segment_override, I.address_size ? REG32(ESI) : REG16(SI) );
	} else {
		eas = i386_translate( DS, I.address_size ? REG32(ESI) : REG16(SI) );
	}
	REG32(EAX) = READ32(eas);
	BUMP_SI(4);
	CYCLES(CYCLES_LODS);
}

static void I386OP(loop32)(void)			// Opcode 0xe2
{
	INT8 disp = FETCH();
	REG32(ECX)--;
	if( REG32(ECX) != 0 ) {
		I.eip += disp;
		CHANGE_PC(I.eip);
	}
	CYCLES(CYCLES_LOOP);		/* TODO: Timing = 11 + m */
}

static void I386OP(loopne32)(void)			// Opcode 0xe0
{
	INT8 disp = FETCH();
	REG32(ECX)--;
	if( REG32(ECX) != 0 && I.ZF == 0 ) {
		I.eip += disp;
		CHANGE_PC(I.eip);
	}
	CYCLES(CYCLES_LOOPNZ);		/* TODO: Timing = 11 + m */
}

static void I386OP(loopz32)(void)			// Opcode 0xe1
{
	INT8 disp = FETCH();
	REG32(ECX)--;
	if( REG32(ECX) != 0 && I.ZF != 0 ) {
		I.eip += disp;
		CHANGE_PC(I.eip);
	}
	CYCLES(CYCLES_LOOPZ);		/* TODO: Timing = 11 + m */
}

static void I386OP(mov_rm32_r32)(void)		// Opcode 0x89
{
	UINT32 src;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_REG32(modrm);
		STORE_RM32(modrm, src);
		CYCLES(CYCLES_MOV_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		src = LOAD_REG32(modrm);
		WRITE32(ea, src);
		CYCLES(CYCLES_MOV_REG_MEM);
	}
}

static void I386OP(mov_r32_rm32)(void)		// Opcode 0x8b
{
	UINT32 src;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_RM32(modrm);
		STORE_REG32(modrm, src);
		CYCLES(CYCLES_MOV_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		src = READ32(ea);
		STORE_REG32(modrm, src);
		CYCLES(CYCLES_MOV_MEM_REG);
	}
}

static void I386OP(mov_rm32_i32)(void)		// Opcode 0xc7
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		UINT32 value = FETCH32();
		STORE_RM32(modrm, value);
		CYCLES(CYCLES_MOV_IMM_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		UINT32 value = FETCH32();
		WRITE32(ea, value);
		CYCLES(CYCLES_MOV_IMM_MEM);
	}
}

static void I386OP(mov_eax_m32)(void)		// Opcode 0xa1
{
	UINT32 offset, ea;
	if( I.address_size ) {
		offset = FETCH32();
	} else {
		offset = FETCH16();
	}
	if( I.segment_prefix ) {
		ea = i386_translate( I.segment_override, offset );
	} else {
		ea = i386_translate( DS, offset );
	}
	REG32(EAX) = READ32(ea);
	CYCLES(CYCLES_MOV_MEM_ACC);
}

static void I386OP(mov_m32_eax)(void)		// Opcode 0xa3
{
	UINT32 offset, ea;
	if( I.address_size ) {
		offset = FETCH32();
	} else {
		offset = FETCH16();
	}
	if( I.segment_prefix ) {
		ea = i386_translate( I.segment_override, offset );
	} else {
		ea = i386_translate( DS, offset );
	}
	WRITE32( ea, REG32(EAX) );
	CYCLES(CYCLES_MOV_ACC_MEM);
}

static void I386OP(mov_eax_i32)(void)		// Opcode 0xb8
{
	REG32(EAX) = FETCH32();
	CYCLES(CYCLES_MOV_IMM_REG);
}

static void I386OP(mov_ecx_i32)(void)		// Opcode 0xb9
{
	REG32(ECX) = FETCH32();
	CYCLES(CYCLES_MOV_IMM_REG);
}

static void I386OP(mov_edx_i32)(void)		// Opcode 0xba
{
	REG32(EDX) = FETCH32();
	CYCLES(CYCLES_MOV_IMM_REG);
}

static void I386OP(mov_ebx_i32)(void)		// Opcode 0xbb
{
	REG32(EBX) = FETCH32();
	CYCLES(CYCLES_MOV_IMM_REG);
}

static void I386OP(mov_esp_i32)(void)		// Opcode 0xbc
{
	REG32(ESP) = FETCH32();
	CYCLES(CYCLES_MOV_IMM_REG);
}

static void I386OP(mov_ebp_i32)(void)		// Opcode 0xbd
{
	REG32(EBP) = FETCH32();
	CYCLES(CYCLES_MOV_IMM_REG);
}

static void I386OP(mov_esi_i32)(void)		// Opcode 0xbe
{
	REG32(ESI) = FETCH32();
	CYCLES(CYCLES_MOV_IMM_REG);
}

static void I386OP(mov_edi_i32)(void)		// Opcode 0xbf
{
	REG32(EDI) = FETCH32();
	CYCLES(CYCLES_MOV_IMM_REG);
}

static void I386OP(movsd)(void)				// Opcode 0xa5
{
	UINT32 eas, ead, v;
	if( I.segment_prefix ) {
		eas = i386_translate( I.segment_override, I.address_size ? REG32(ESI) : REG16(SI) );
	} else {
		eas = i386_translate( DS, I.address_size ? REG32(ESI) : REG16(SI) );
	}
	ead = i386_translate( ES, I.address_size ? REG32(EDI) : REG16(DI) );
	v = READ32(eas);
	WRITE32(ead, v);
	BUMP_SI(4);
	BUMP_DI(4);
	CYCLES(CYCLES_MOVS);
}

static void I386OP(movsx_r32_rm8)(void)		// Opcode 0x0f be
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		INT32 src = (INT8)LOAD_RM8(modrm);
		STORE_REG32(modrm, src);
		CYCLES(CYCLES_MOVSX_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		INT32 src = (INT8)READ8(ea);
		STORE_REG32(modrm, src);
		CYCLES(CYCLES_MOVSX_MEM_REG);
	}
}

static void I386OP(movsx_r32_rm16)(void)	// Opcode 0x0f bf
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		INT32 src = (INT16)LOAD_RM16(modrm);
		STORE_REG32(modrm, src);
		CYCLES(CYCLES_MOVSX_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		INT32 src = (INT16)READ16(ea);
		STORE_REG32(modrm, src);
		CYCLES(CYCLES_MOVSX_MEM_REG);
	}
}

static void I386OP(movzx_r32_rm8)(void)		// Opcode 0x0f b6
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		UINT32 src = (UINT8)LOAD_RM8(modrm);
		STORE_REG32(modrm, src);
		CYCLES(CYCLES_MOVZX_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		UINT32 src = (UINT8)READ8(ea);
		STORE_REG32(modrm, src);
		CYCLES(CYCLES_MOVZX_MEM_REG);
	}
}

static void I386OP(movzx_r32_rm16)(void)	// Opcode 0x0f b7
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		UINT32 src = (UINT16)LOAD_RM16(modrm);
		STORE_REG32(modrm, src);
		CYCLES(CYCLES_MOVZX_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		UINT32 src = (UINT16)READ16(ea);
		STORE_REG32(modrm, src);
		CYCLES(CYCLES_MOVZX_MEM_REG);
	}
}

static void I386OP(or_rm32_r32)(void)		// Opcode 0x09
{
	UINT32 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_REG32(modrm);
		dst = LOAD_RM32(modrm);
		dst = OR32(dst, src);
		STORE_RM32(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		src = LOAD_REG32(modrm);
		dst = READ32(ea);
		dst = OR32(dst, src);
		WRITE32(ea, dst);
		CYCLES(CYCLES_ALU_REG_MEM);
	}
}

static void I386OP(or_r32_rm32)(void)		// Opcode 0x0b
{
	UINT32 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_RM32(modrm);
		dst = LOAD_REG32(modrm);
		dst = OR32(dst, src);
		STORE_REG32(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		src = READ32(ea);
		dst = LOAD_REG32(modrm);
		dst = OR32(dst, src);
		STORE_REG32(modrm, dst);
		CYCLES(CYCLES_ALU_MEM_REG);
	}
}

static void I386OP(or_eax_i32)(void)		// Opcode 0x0d
{
	UINT32 src, dst;
	src = FETCH32();
	dst = REG32(EAX);
	dst = OR32(dst, src);
	REG32(EAX) = dst;
	CYCLES(CYCLES_ALU_IMM_ACC);
}

static void I386OP(out_eax_i8)(void)		// Opcode 0xe7
{
	UINT16 port = FETCH();
	UINT32 data = REG32(EAX);
	WRITEPORT32(port, data);
	CYCLES(CYCLES_OUT_VAR);
}

static void I386OP(out_eax_dx)(void)		// Opcode 0xef
{
	UINT16 port = REG16(DX);
	UINT32 data = REG32(EAX);
	WRITEPORT32(port, data);
	CYCLES(CYCLES_OUT);
}

static void I386OP(pop_eax)(void)			// Opcode 0x58
{
	REG32(EAX) = POP32();
	CYCLES(CYCLES_POP_REG_SHORT);
}

static void I386OP(pop_ecx)(void)			// Opcode 0x59
{
	REG32(ECX) = POP32();
	CYCLES(CYCLES_POP_REG_SHORT);
}

static void I386OP(pop_edx)(void)			// Opcode 0x5a
{
	REG32(EDX) = POP32();
	CYCLES(CYCLES_POP_REG_SHORT);
}

static void I386OP(pop_ebx)(void)			// Opcode 0x5b
{
	REG32(EBX) = POP32();
	CYCLES(CYCLES_POP_REG_SHORT);
}

static void I386OP(pop_esp)(void)			// Opcode 0x5c
{
	REG32(ESP) = POP32();
	CYCLES(CYCLES_POP_REG_SHORT);
}

static void I386OP(pop_ebp)(void)			// Opcode 0x5d
{
	REG32(EBP) = POP32();
	CYCLES(CYCLES_POP_REG_SHORT);
}

static void I386OP(pop_esi)(void)			// Opcode 0x5e
{
	REG32(ESI) = POP32();
	CYCLES(CYCLES_POP_REG_SHORT);
}

static void I386OP(pop_edi)(void)			// Opcode 0x5f
{
	REG32(EDI) = POP32();
	CYCLES(CYCLES_POP_REG_SHORT);
}

static void I386OP(pop_ds32)(void)			// Opcode 0x1f
{
	I.sreg[DS].selector = POP32();
	if( PROTECTED_MODE ) {
		i386_load_segment_descriptor(DS);
	} else {
		i386_load_segment_descriptor(DS);
	}
	CYCLES(CYCLES_POP_SREG);
}

static void I386OP(pop_es32)(void)			// Opcode 0x07
{
	I.sreg[ES].selector = POP32();
	if( PROTECTED_MODE ) {
		i386_load_segment_descriptor(ES);
	} else {
		i386_load_segment_descriptor(ES);
	}
	CYCLES(CYCLES_POP_SREG);
}

static void I386OP(pop_fs32)(void)			// Opcode 0x0f a1
{
	I.sreg[FS].selector = POP32();
	if( PROTECTED_MODE ) {
		i386_load_segment_descriptor(FS);
	} else {
		i386_load_segment_descriptor(FS);
	}
	CYCLES(CYCLES_POP_SREG);
}

static void I386OP(pop_gs32)(void)			// Opcode 0x0f a9
{
	I.sreg[GS].selector = POP32();
	if( PROTECTED_MODE ) {
		i386_load_segment_descriptor(GS);
	} else {
		i386_load_segment_descriptor(GS);
	}
	CYCLES(CYCLES_POP_SREG);
}

static void I386OP(pop_ss32)(void)			// Opcode 0x17
{
	I.sreg[SS].selector = POP32();
	if( PROTECTED_MODE ) {
		i386_load_segment_descriptor(SS);
	} else {
		i386_load_segment_descriptor(SS);
	}
	CYCLES(CYCLES_POP_SREG);
}

static void I386OP(pop_rm32)(void)			// Opcode 0x8f
{
	UINT8 modrm = FETCH();
	UINT32 value = POP32();

	if( modrm >= 0xc0 ) {
		STORE_RM32(modrm, value);
	} else {
		UINT32 ea = GetEA(modrm);
		WRITE32(ea, value);
	}
	CYCLES(CYCLES_POP_RM);
}

static void I386OP(popad)(void)				// Opcode 0x61
{
	REG32(EDI) = POP32();
	REG32(ESI) = POP32();
	REG32(EBP) = POP32();
	REG32(ESP) += 4;
	REG32(EBX) = POP32();
	REG32(EDX) = POP32();
	REG32(ECX) = POP32();
	REG32(EAX) = POP32();
	CYCLES(CYCLES_POPA);
}

static void I386OP(popfd)(void)				// Opcode 0x9d
{
	UINT32 value = POP32();
	set_flags(value);
	CYCLES(CYCLES_POPF);
}

static void I386OP(push_eax)(void)			// Opcode 0x50
{
	PUSH32( REG32(EAX) );
	CYCLES(CYCLES_PUSH_REG_SHORT);
}

static void I386OP(push_ecx)(void)			// Opcode 0x51
{
	PUSH32( REG32(ECX) );
	CYCLES(CYCLES_PUSH_REG_SHORT);
}

static void I386OP(push_edx)(void)			// Opcode 0x52
{
	PUSH32( REG32(EDX) );
	CYCLES(CYCLES_PUSH_REG_SHORT);
}

static void I386OP(push_ebx)(void)			// Opcode 0x53
{
	PUSH32( REG32(EBX) );
	CYCLES(CYCLES_PUSH_REG_SHORT);
}

static void I386OP(push_esp)(void)			// Opcode 0x54
{
	PUSH32( REG32(ESP) );
	CYCLES(CYCLES_PUSH_REG_SHORT);
}

static void I386OP(push_ebp)(void)			// Opcode 0x55
{
	PUSH32( REG32(EBP) );
	CYCLES(CYCLES_PUSH_REG_SHORT);
}

static void I386OP(push_esi)(void)			// Opcode 0x56
{
	PUSH32( REG32(ESI) );
	CYCLES(CYCLES_PUSH_REG_SHORT);
}

static void I386OP(push_edi)(void)			// Opcode 0x57
{
	PUSH32( REG32(EDI) );
	CYCLES(CYCLES_PUSH_REG_SHORT);
}

static void I386OP(push_cs32)(void)			// Opcode 0x0e
{
	PUSH32( I.sreg[CS].selector );
	CYCLES(CYCLES_PUSH_SREG);
}

static void I386OP(push_ds32)(void)			// Opcode 0x1e
{
	PUSH32( I.sreg[DS].selector );
	CYCLES(CYCLES_PUSH_SREG);
}

static void I386OP(push_es32)(void)			// Opcode 0x06
{
	PUSH32( I.sreg[ES].selector );
	CYCLES(CYCLES_PUSH_SREG);
}

static void I386OP(push_fs32)(void)			// Opcode 0x0f a0
{
	PUSH32( I.sreg[FS].selector );
	CYCLES(CYCLES_PUSH_SREG);
}

static void I386OP(push_gs32)(void)			// Opcode 0x0f a8
{
	PUSH32( I.sreg[GS].selector );
	CYCLES(CYCLES_PUSH_SREG);
}

static void I386OP(push_ss32)(void)			// Opcode 0x16
{
	PUSH32( I.sreg[SS].selector );
	CYCLES(CYCLES_PUSH_SREG);
}

static void I386OP(push_i32)(void)			// Opcode 0x68
{
	UINT32 value = FETCH32();
	PUSH32(value);
	CYCLES(CYCLES_PUSH_IMM);
}

static void I386OP(pushad)(void)			// Opcode 0x60
{
	UINT32 temp = REG32(ESP);
	PUSH32( REG32(EAX) );
	PUSH32( REG32(ECX) );
	PUSH32( REG32(EDX) );
	PUSH32( REG32(EBX) );
	PUSH32( temp );
	PUSH32( REG32(EBP) );
	PUSH32( REG32(ESI) );
	PUSH32( REG32(EDI) );
	CYCLES(CYCLES_PUSHA);
}

static void I386OP(pushfd)(void)			// Opcode 0x9c
{
	PUSH32( get_flags() & 0x00fcffff );
	CYCLES(CYCLES_PUSHF);
}

static void I386OP(ret_near32_i16)(void)	// Opcode 0xc2
{
	INT16 disp = FETCH16();
	I.eip = POP32();
	REG32(ESP) += disp;
	CHANGE_PC(I.eip);
	CYCLES(CYCLES_RET_IMM);		/* TODO: Timing = 10 + m */
}

static void I386OP(ret_near32)(void)		// Opcode 0xc3
{
	I.eip = POP32();
	CHANGE_PC(I.eip);
	CYCLES(CYCLES_RET);		/* TODO: Timing = 10 + m */
}

static void I386OP(sbb_rm32_r32)(void)		// Opcode 0x19
{
	UINT32 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_REG32(modrm) + I.CF;
		dst = LOAD_RM32(modrm);
		dst = SUB32(dst, src);
		STORE_RM32(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		src = LOAD_REG32(modrm) + I.CF;
		dst = READ32(ea);
		dst = SUB32(dst, src);
		WRITE32(ea, dst);
		CYCLES(CYCLES_ALU_REG_MEM);
	}
}

static void I386OP(sbb_r32_rm32)(void)		// Opcode 0x1b
{
	UINT32 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_RM32(modrm) + I.CF;
		dst = LOAD_REG32(modrm);
		dst = SUB32(dst, src);
		STORE_REG32(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		src = READ32(ea) + I.CF;
		dst = LOAD_REG32(modrm);
		dst = SUB32(dst, src);
		STORE_REG32(modrm, dst);
		CYCLES(CYCLES_ALU_MEM_REG);
	}
}

static void I386OP(sbb_eax_i32)(void)		// Opcode 0x1d
{
	UINT32 src, dst;
	src = FETCH32() + I.CF;
	dst = REG32(EAX);
	dst = SUB32(dst, src);
	REG32(EAX) = dst;
	CYCLES(CYCLES_ALU_IMM_ACC);
}

static void I386OP(scasd)(void)				// Opcode 0xaf
{
	UINT32 eas, src, dst;
	eas = i386_translate( ES, I.address_size ? REG32(EDI) : REG16(DI) );
	src = READ32(eas);
	dst = REG32(EAX);
	SUB32(dst, src);
	BUMP_DI(4);
	CYCLES(CYCLES_SCAS);
}

static void I386OP(shld32_i8)(void)			// Opcode 0x0f a4
{
	/* TODO: Correct flags */
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		UINT32 dst = LOAD_RM32(modrm);
		UINT32 upper = LOAD_REG32(modrm);
		UINT8 shift = FETCH();
		if( shift > 31 || shift == 0 ) {

		} else {
			I.CF = (dst & (1 << (32-shift))) ? 1 : 0;
			dst = (dst << shift) | (upper >> (32-shift));
			SetSZPF32(dst);
		}
		STORE_RM32(modrm, dst);
		CYCLES(CYCLES_SHLD_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		UINT32 dst = READ32(ea);
		UINT32 upper = LOAD_REG32(modrm);
		UINT8 shift = FETCH();
		if( shift > 31 || shift == 0 ) {

		} else {
			I.CF = (dst & (1 << (32-shift))) ? 1 : 0;
			dst = (dst << shift) | (upper >> (32-shift));
			SetSZPF32(dst);
		}
		WRITE32(ea, dst);
		CYCLES(CYCLES_SHLD_MEM);
	}
}

static void I386OP(shld32_cl)(void)			// Opcode 0x0f a5
{
	/* TODO: Correct flags */
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		UINT32 dst = LOAD_RM32(modrm);
		UINT32 upper = LOAD_REG32(modrm);
		UINT8 shift = REG8(CL);
		if( shift > 31 || shift == 0 ) {

		} else {
			I.CF = (dst & (1 << (32-shift))) ? 1 : 0;
			dst = (dst << shift) | (upper >> (32-shift));
			SetSZPF32(dst);
		}
		STORE_RM32(modrm, dst);
		CYCLES(CYCLES_SHLD_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		UINT32 dst = READ32(ea);
		UINT32 upper = LOAD_REG32(modrm);
		UINT8 shift = REG8(CL);
		if( shift > 31 || shift == 0 ) {

		} else {
			I.CF = (dst & (1 << (32-shift))) ? 1 : 0;
			dst = (dst << shift) | (upper >> (32-shift));
			SetSZPF32(dst);
		}
		WRITE32(ea, dst);
		CYCLES(CYCLES_SHLD_MEM);
	}
}

static void I386OP(shrd32_i8)(void)			// Opcode 0x0f ac
{
	/* TODO: Correct flags */
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		UINT32 dst = LOAD_RM32(modrm);
		UINT32 upper = LOAD_REG32(modrm);
		UINT8 shift = FETCH();
		if( shift > 31 || shift == 0 ) {

		} else {
			I.CF = (dst & (1 << (shift-1))) ? 1 : 0;
			dst = (dst >> shift) | (upper << (32-shift));
			SetSZPF32(dst);
		}
		STORE_RM32(modrm, dst);
		CYCLES(CYCLES_SHRD_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		UINT32 dst = READ32(ea);
		UINT32 upper = LOAD_REG32(modrm);
		UINT8 shift = FETCH();
		if( shift > 31 || shift == 0 ) {

		} else {
			I.CF = (dst & (1 << (shift-1))) ? 1 : 0;
			dst = (dst >> shift) | (upper << (32-shift));
			SetSZPF32(dst);
		}
		WRITE32(ea, dst);
		CYCLES(CYCLES_SHRD_MEM);
	}
}

static void I386OP(shrd32_cl)(void)			// Opcode 0x0f ad
{
	/* TODO: Correct flags */
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		UINT32 dst = LOAD_RM32(modrm);
		UINT32 upper = LOAD_REG32(modrm);
		UINT8 shift = REG8(CL);
		if( shift > 31 || shift == 0 ) {

		} else {
			I.CF = (dst & (1 << (shift-1))) ? 1 : 0;
			dst = (dst >> shift) | (upper << (32-shift));
			SetSZPF32(dst);
		}
		STORE_RM32(modrm, dst);
		CYCLES(CYCLES_SHRD_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		UINT32 dst = READ32(ea);
		UINT32 upper = LOAD_REG32(modrm);
		UINT8 shift = REG8(CL);
		if( shift > 31 || shift == 0 ) {

		} else {
			I.CF = (dst & (1 << (shift-1))) ? 1 : 0;
			dst = (dst >> shift) | (upper << (32-shift));
			SetSZPF32(dst);
		}
		WRITE32(ea, dst);
		CYCLES(CYCLES_SHRD_MEM);
	}
}

static void I386OP(stosd)(void)				// Opcode 0xab
{
	UINT32 eas = i386_translate( ES, I.address_size ? REG32(EDI) : REG16(DI) );
	WRITE32(eas, REG32(EAX));
	BUMP_DI(4);
	CYCLES(CYCLES_STOS);
}

static void I386OP(sub_rm32_r32)(void)		// Opcode 0x29
{
	UINT32 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_REG32(modrm);
		dst = LOAD_RM32(modrm);
		dst = SUB32(dst, src);
		STORE_RM32(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		src = LOAD_REG32(modrm);
		dst = READ32(ea);
		dst = SUB32(dst, src);
		WRITE32(ea, dst);
		CYCLES(CYCLES_ALU_REG_MEM);
	}
}

static void I386OP(sub_r32_rm32)(void)		// Opcode 0x2b
{
	UINT32 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_RM32(modrm);
		dst = LOAD_REG32(modrm);
		dst = SUB32(dst, src);
		STORE_REG32(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		src = READ32(ea);
		dst = LOAD_REG32(modrm);
		dst = SUB32(dst, src);
		STORE_REG32(modrm, dst);
		CYCLES(CYCLES_ALU_MEM_REG);
	}
}

static void I386OP(sub_eax_i32)(void)		// Opcode 0x2d
{
	UINT32 src, dst;
	src = FETCH32();
	dst = REG32(EAX);
	dst = SUB32(dst, src);
	REG32(EAX) = dst;
	CYCLES(CYCLES_ALU_IMM_ACC);
}

static void I386OP(test_eax_i32)(void)		// Opcode 0xa9
{
	UINT32 src = FETCH32();
	UINT32 dst = REG32(EAX);
	dst = src & dst;
	SetSZPF32(dst);
	I.CF = 0;
	I.OF = 0;
	CYCLES(CYCLES_TEST_IMM_ACC);
}

static void I386OP(test_rm32_r32)(void)		// Opcode 0x85
{
	UINT32 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_REG32(modrm);
		dst = LOAD_RM32(modrm);
		dst = src & dst;
		SetSZPF32(dst);
		I.CF = 0;
		I.OF = 0;
		CYCLES(CYCLES_TEST_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		src = LOAD_REG32(modrm);
		dst = READ32(ea);
		dst = src & dst;
		SetSZPF32(dst);
		I.CF = 0;
		I.OF = 0;
		CYCLES(CYCLES_TEST_REG_MEM);
	}
}

static void I386OP(xchg_eax_ecx)(void)		// Opcode 0x91
{
	UINT32 temp;
	temp = REG32(EAX);
	REG32(EAX) = REG32(ECX);
	REG32(ECX) = temp;
	CYCLES(CYCLES_XCHG_REG_REG);
}

static void I386OP(xchg_eax_edx)(void)		// Opcode 0x92
{
	UINT32 temp;
	temp = REG32(EAX);
	REG32(EAX) = REG32(EDX);
	REG32(EDX) = temp;
	CYCLES(CYCLES_XCHG_REG_REG);
}

static void I386OP(xchg_eax_ebx)(void)		// Opcode 0x93
{
	UINT32 temp;
	temp = REG32(EAX);
	REG32(EAX) = REG32(EBX);
	REG32(EBX) = temp;
	CYCLES(CYCLES_XCHG_REG_REG);
}

static void I386OP(xchg_eax_esp)(void)		// Opcode 0x94
{
	UINT32 temp;
	temp = REG32(EAX);
	REG32(EAX) = REG32(ESP);
	REG32(ESP) = temp;
	CYCLES(CYCLES_XCHG_REG_REG);
}

static void I386OP(xchg_eax_ebp)(void)		// Opcode 0x95
{
	UINT32 temp;
	temp = REG32(EAX);
	REG32(EAX) = REG32(EBP);
	REG32(EBP) = temp;
	CYCLES(CYCLES_XCHG_REG_REG);
}

static void I386OP(xchg_eax_esi)(void)		// Opcode 0x96
{
	UINT32 temp;
	temp = REG32(EAX);
	REG32(EAX) = REG32(ESI);
	REG32(ESI) = temp;
	CYCLES(CYCLES_XCHG_REG_REG);
}

static void I386OP(xchg_eax_edi)(void)		// Opcode 0x97
{
	UINT32 temp;
	temp = REG32(EAX);
	REG32(EAX) = REG32(EDI);
	REG32(EDI) = temp;
	CYCLES(CYCLES_XCHG_REG_REG);
}

static void I386OP(xchg_r32_rm32)(void)		// Opcode 0x87
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		UINT32 src = LOAD_RM32(modrm);
		UINT32 dst = LOAD_REG32(modrm);
		STORE_REG32(modrm, src);
		STORE_RM32(modrm, dst);
		CYCLES(CYCLES_XCHG_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		UINT32 src = READ32(ea);
		UINT32 dst = LOAD_REG32(modrm);
		STORE_REG32(modrm, src);
		WRITE32(ea, dst);
		CYCLES(CYCLES_XCHG_REG_MEM);
	}
}

static void I386OP(xor_rm32_r32)(void)		// Opcode 0x31
{
	UINT32 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_REG32(modrm);
		dst = LOAD_RM32(modrm);
		dst = XOR32(dst, src);
		STORE_RM32(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		src = LOAD_REG32(modrm);
		dst = READ32(ea);
		dst = XOR32(dst, src);
		WRITE32(ea, dst);
		CYCLES(CYCLES_ALU_REG_MEM);
	}
}

static void I386OP(xor_r32_rm32)(void)		// Opcode 0x33
{
	UINT32 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_RM32(modrm);
		dst = LOAD_REG32(modrm);
		dst = XOR32(dst, src);
		STORE_REG32(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		src = READ32(ea);
		dst = LOAD_REG32(modrm);
		dst = XOR32(dst, src);
		STORE_REG32(modrm, dst);
		CYCLES(CYCLES_ALU_MEM_REG);
	}
}

static void I386OP(xor_eax_i32)(void)		// Opcode 0x35
{
	UINT32 src, dst;
	src = FETCH32();
	dst = REG32(EAX);
	dst = XOR32(dst, src);
	REG32(EAX) = dst;
	CYCLES(CYCLES_ALU_IMM_ACC);
}



static void I386OP(group81_32)(void)		// Opcode 0x81
{
	UINT32 ea;
	UINT32 src, dst;
	UINT8 modrm = FETCH();

	switch( (modrm >> 3) & 0x7 )
	{
		case 0:		// ADD Rm32, i32
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM32(modrm);
				src = FETCH32();
				dst = ADD32(dst, src);
				STORE_RM32(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm);
				dst = READ32(ea);
				src = FETCH32();
				dst = ADD32(dst, src);
				WRITE32(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 1:		// OR Rm32, i32
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM32(modrm);
				src = FETCH32();
				dst = OR32(dst, src);
				STORE_RM32(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm);
				dst = READ32(ea);
				src = FETCH32();
				dst = OR32(dst, src);
				WRITE32(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 2:		// ADC Rm32, i32
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM32(modrm);
				src = FETCH32();
				src = ADD32(src, I.CF);
				dst = ADD32(dst, src);
				STORE_RM32(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm);
				dst = READ32(ea);
				src = FETCH32();
				src = ADD32(src, I.CF);
				dst = ADD32(dst, src);
				WRITE32(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 3:		// SBB Rm32, i32
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM32(modrm);
				src = FETCH32() + I.CF;
				dst = SUB32(dst, src);
				STORE_RM32(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm);
				dst = READ32(ea);
				src = FETCH32() + I.CF;
				dst = SUB32(dst, src);
				WRITE32(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 4:		// AND Rm32, i32
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM32(modrm);
				src = FETCH32();
				dst = AND32(dst, src);
				STORE_RM32(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm);
				dst = READ32(ea);
				src = FETCH32();
				dst = AND32(dst, src);
				WRITE32(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 5:		// SUB Rm32, i32
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM32(modrm);
				src = FETCH32();
				dst = SUB32(dst, src);
				STORE_RM32(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm);
				dst = READ32(ea);
				src = FETCH32();
				dst = SUB32(dst, src);
				WRITE32(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 6:		// XOR Rm32, i32
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM32(modrm);
				src = FETCH32();
				dst = XOR32(dst, src);
				STORE_RM32(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm);
				dst = READ32(ea);
				src = FETCH32();
				dst = XOR32(dst, src);
				WRITE32(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 7:		// CMP Rm32, i32
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM32(modrm);
				src = FETCH32();
				SUB32(dst, src);
				CYCLES(CYCLES_CMP_REG_REG);
			} else {
				ea = GetEA(modrm);
				dst = READ32(ea);
				src = FETCH32();
				SUB32(dst, src);
				CYCLES(CYCLES_CMP_REG_MEM);
			}
			break;
	}
}

static void I386OP(group83_32)(void)		// Opcode 0x83
{
	UINT32 ea;
	UINT32 src, dst;
	UINT8 modrm = FETCH();

	switch( (modrm >> 3) & 0x7 )
	{
		case 0:		// ADD Rm32, i32
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM32(modrm);
				src = (UINT32)(INT32)(INT8)FETCH();
				dst = ADD32(dst, src);
				STORE_RM32(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm);
				dst = READ32(ea);
				src = (UINT32)(INT32)(INT8)FETCH();
				dst = ADD32(dst, src);
				WRITE32(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 1:		// OR Rm32, i32
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM32(modrm);
				src = (UINT32)(INT32)(INT8)FETCH();
				dst = OR32(dst, src);
				STORE_RM32(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm);
				dst = READ32(ea);
				src = (UINT32)(INT32)(INT8)FETCH();
				dst = OR32(dst, src);
				WRITE32(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 2:		// ADC Rm32, i32
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM32(modrm);
				src = (UINT32)(INT32)(INT8)FETCH();
				src = ADD32(src, I.CF);
				dst = ADD32(dst, src);
				STORE_RM32(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm);
				dst = READ32(ea);
				src = (UINT32)(INT32)(INT8)FETCH();
				src = ADD32(src, I.CF);
				dst = ADD32(dst, src);
				WRITE32(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 3:		// SBB Rm32, i32
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM32(modrm);
				src = ((UINT32)(INT32)(INT8)FETCH()) + I.CF;
				dst = SUB32(dst, src);
				STORE_RM32(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm);
				dst = READ32(ea);
				src = ((UINT32)(INT32)(INT8)FETCH()) + I.CF;
				dst = SUB32(dst, src);
				WRITE32(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 4:		// AND Rm32, i32
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM32(modrm);
				src = (UINT32)(INT32)(INT8)FETCH();
				dst = AND32(dst, src);
				STORE_RM32(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm);
				dst = READ32(ea);
				src = (UINT32)(INT32)(INT8)FETCH();
				dst = AND32(dst, src);
				WRITE32(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 5:		// SUB Rm32, i32
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM32(modrm);
				src = (UINT32)(INT32)(INT8)FETCH();
				dst = SUB32(dst, src);
				STORE_RM32(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm);
				dst = READ32(ea);
				src = (UINT32)(INT32)(INT8)FETCH();
				dst = SUB32(dst, src);
				WRITE32(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 6:		// XOR Rm32, i32
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM32(modrm);
				src = (UINT32)(INT32)(INT8)FETCH();
				dst = XOR32(dst, src);
				STORE_RM32(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm);
				dst = READ32(ea);
				src = (UINT32)(INT32)(INT8)FETCH();
				dst = XOR32(dst, src);
				WRITE32(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 7:		// CMP Rm32, i32
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM32(modrm);
				src = (UINT32)(INT32)(INT8)FETCH();
				SUB32(dst, src);
				CYCLES(CYCLES_CMP_REG_REG);
			} else {
				ea = GetEA(modrm);
				dst = READ32(ea);
				src = (UINT32)(INT32)(INT8)FETCH();
				SUB32(dst, src);
				CYCLES(CYCLES_CMP_REG_MEM);
			}
			break;
	}
}

static void I386OP(groupC1_32)(void)		// Opcode 0xc1
{
	UINT32 dst;
	UINT8 modrm = FETCH();
	UINT8 shift;

	if( modrm >= 0xc0 ) {
		dst = LOAD_RM32(modrm);
		shift = FETCH() & 0x1f;
		dst = i386_shift_rotate32(modrm, dst, shift);
		STORE_RM32(modrm, dst);
	} else {
		UINT32 ea = GetEA(modrm);
		dst = READ32(ea);
		shift = FETCH() & 0x1f;
		dst = i386_shift_rotate32(modrm, dst, shift);
		WRITE32(ea, dst);
	}
}

static void I386OP(groupD1_32)(void)		// Opcode 0xd1
{
	UINT32 dst;
	UINT8 modrm = FETCH();

	if( modrm >= 0xc0 ) {
		dst = LOAD_RM32(modrm);
		dst = i386_shift_rotate32(modrm, dst, 1);
		STORE_RM32(modrm, dst);
	} else {
		UINT32 ea = GetEA(modrm);
		dst = READ32(ea);
		dst = i386_shift_rotate32(modrm, dst, 1);
		WRITE32(ea, dst);
	}
}

static void I386OP(groupD3_32)(void)		// Opcode 0xd3
{
	UINT32 dst;
	UINT8 modrm = FETCH();

	if( modrm >= 0xc0 ) {
		dst = LOAD_RM32(modrm);
		dst = i386_shift_rotate32(modrm, dst, REG8(CL));
		STORE_RM32(modrm, dst);
	} else {
		UINT32 ea = GetEA(modrm);
		dst = READ32(ea);
		dst = i386_shift_rotate32(modrm, dst, REG8(CL));
		WRITE32(ea, dst);
	}
}

static void I386OP(groupF7_32)(void)		// Opcode 0xf7
{
	UINT8 modrm = FETCH();

	switch( (modrm >> 3) & 0x7 )
	{
		case 0:			/* TEST Rm32, i32 */
			if( modrm >= 0xc0 ) {
				UINT32 dst = LOAD_RM32(modrm);
				UINT32 src = FETCH32();
				dst &= src;
				I.CF = I.OF = I.AF = 0;
				SetSZPF32(dst);
				CYCLES(CYCLES_TEST_IMM_REG);
			} else {
				UINT32 ea = GetEA(modrm);
				UINT32 dst = READ32(ea);
				UINT32 src = FETCH32();
				dst &= src;
				I.CF = I.OF = I.AF = 0;
				SetSZPF32(dst);
				CYCLES(CYCLES_TEST_IMM_MEM);
			}
			break;
		case 2:			/* NOT Rm32 */
			if( modrm >= 0xc0 ) {
				UINT32 dst = LOAD_RM32(modrm);
				dst = ~dst;
				STORE_RM32(modrm, dst);
				CYCLES(CYCLES_NOT_REG);
			} else {
				UINT32 ea = GetEA(modrm);
				UINT32 dst = READ32(ea);
				dst = ~dst;
				WRITE32(ea, dst);
				CYCLES(CYCLES_NOT_MEM);
			}
			break;
		case 3:			/* NEG Rm32 */
			if( modrm >= 0xc0 ) {
				UINT32 dst = LOAD_RM32(modrm);
				dst = SUB32( 0, dst );
				STORE_RM32(modrm, dst);
				CYCLES(CYCLES_NEG_REG);
			} else {
				UINT32 ea = GetEA(modrm);
				UINT32 dst = READ32(ea);
				dst = SUB32( 0, dst );
				WRITE32(ea, dst);
				CYCLES(CYCLES_NEG_MEM);
			}
			break;
		case 4:			/* MUL EAX, Rm32 */
			{
				UINT64 result;
				UINT32 src, dst;
				if( modrm >= 0xc0 ) {
					src = LOAD_RM32(modrm);
					CYCLES(CYCLES_MUL32_ACC_REG);		/* TODO: Correct multiply timing */
				} else {
					UINT32 ea = GetEA(modrm);
					src = READ32(ea);
					CYCLES(CYCLES_MUL32_ACC_MEM);		/* TODO: Correct multiply timing */
				}

				dst = REG32(EAX);
				result = (UINT64)src * (UINT64)dst;
				REG32(EDX) = (UINT32)(result >> 32);
				REG32(EAX) = (UINT32)result;

				I.CF = I.OF = (REG32(EDX) != 0);
			}
			break;
		case 5:			/* IMUL EAX, Rm32 */
			{
				INT64 result;
				INT64 src, dst;
				if( modrm >= 0xc0 ) {
					src = (INT64)(INT32)LOAD_RM32(modrm);
					CYCLES(CYCLES_IMUL32_ACC_REG);		/* TODO: Correct multiply timing */
				} else {
					UINT32 ea = GetEA(modrm);
					src = (INT64)(INT32)READ32(ea);
					CYCLES(CYCLES_IMUL32_ACC_MEM);		/* TODO: Correct multiply timing */
				}

				dst = (INT64)(INT32)REG32(EAX);
				result = src * dst;

				REG32(EDX) = (UINT32)(result >> 32);
				REG32(EAX) = (UINT32)result;

				I.CF = I.OF = !(result == (INT64)(INT32)result);
			}
			break;
		case 6:			/* DIV EAX, Rm32 */
			{
				UINT64 quotient, remainder, result;
				UINT32 src;
				if( modrm >= 0xc0 ) {
					src = LOAD_RM32(modrm);
					CYCLES(CYCLES_DIV32_ACC_REG);
				} else {
					UINT32 ea = GetEA(modrm);
					src = READ32(ea);
					CYCLES(CYCLES_DIV32_ACC_MEM);
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
					/* TODO: Divide by zero */
				}
			}
			break;
		case 7:			/* IDIV EAX, Rm32 */
			{
				INT64 quotient, remainder, result;
				UINT32 src;
				if( modrm >= 0xc0 ) {
					src = LOAD_RM32(modrm);
					CYCLES(CYCLES_IDIV32_ACC_REG);
				} else {
					UINT32 ea = GetEA(modrm);
					src = READ32(ea);
					CYCLES(CYCLES_IDIV32_ACC_MEM);
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
					/* TODO: Divide by zero */
				}
			}
			break;
	}
}

static void I386OP(groupFF_32)(void)		// Opcode 0xff
{
	UINT8 modrm = FETCH();

	switch( (modrm >> 3) & 0x7 )
	{
		case 0:			/* INC Rm32 */
			if( modrm >= 0xc0 ) {
				UINT32 dst = LOAD_RM32(modrm);
				dst = INC32(dst);
				STORE_RM32(modrm, dst);
				CYCLES(CYCLES_INC_REG);
			} else {
				UINT32 ea = GetEA(modrm);
				UINT32 dst = READ32(ea);
				dst = INC32(dst);
				WRITE32(ea, dst);
				CYCLES(CYCLES_INC_MEM);
			}
			break;
		case 1:			/* DEC Rm32 */
			if( modrm >= 0xc0 ) {
				UINT32 dst = LOAD_RM32(modrm);
				dst = DEC32(dst);
				STORE_RM32(modrm, dst);
				CYCLES(CYCLES_DEC_REG);
			} else {
				UINT32 ea = GetEA(modrm);
				UINT32 dst = READ32(ea);
				dst = DEC32(dst);
				WRITE32(ea, dst);
				CYCLES(CYCLES_DEC_MEM);
			}
			break;
		case 2:			/* CALL Rm32 */
			{
				UINT32 address;
				if( modrm >= 0xc0 ) {
					address = LOAD_RM32(modrm);
					CYCLES(CYCLES_CALL_REG);		/* TODO: Timing = 7 + m */
				} else {
					UINT32 ea = GetEA(modrm);
					address = READ32(ea);
					CYCLES(CYCLES_CALL_MEM);		/* TODO: Timing = 10 + m */
				}
				PUSH32( I.eip );
				I.eip = address;
				CHANGE_PC(I.eip);
			}
			break;
		case 3:			/* CALL FAR Rm32 */
			{
				UINT16 selector;
				UINT32 address;
				if( modrm >= 0xc0 ) {
					fatalerror("NYI");
				} else {
					UINT32 ea = GetEA(modrm);
					address = READ32(ea + 0);
					selector = READ16(ea + 4);
					CYCLES(CYCLES_CALL_MEM_INTERSEG);		/* TODO: Timing = 10 + m */
				}
				PUSH32( I.sreg[CS].selector );
				PUSH32( I.eip );
				I.sreg[CS].selector = selector;
				I.performed_intersegment_jump = 1;
				i386_load_segment_descriptor( CS );
				I.eip = address;
				CHANGE_PC(I.eip);
			}
			break;
		case 4:			/* JMP Rm32 */
			{
				UINT32 address;
				if( modrm >= 0xc0 ) {
					address = LOAD_RM32(modrm);
					CYCLES(CYCLES_JMP_REG);		/* TODO: Timing = 7 + m */
				} else {
					UINT32 ea = GetEA(modrm);
					address = READ32(ea);
					CYCLES(CYCLES_JMP_MEM);		/* TODO: Timing = 10 + m */
				}
				I.eip = address;
				CHANGE_PC(I.eip);
			}
			break;
		case 5:			/* JMP FAR Rm32 */
			{
				UINT16 selector;
				UINT32 address;
				if( modrm >= 0xc0 ) {
					fatalerror("NYI");
				} else {
					UINT32 ea = GetEA(modrm);
					address = READ32(ea + 0);
					selector = READ16(ea + 4);
					CYCLES(CYCLES_JMP_MEM_INTERSEG);		/* TODO: Timing = 10 + m */
				}
				I.sreg[CS].selector = selector;
				I.performed_intersegment_jump = 1;
				i386_load_segment_descriptor( CS );
				I.eip = address;
				CHANGE_PC(I.eip);
			}
			break;
		case 6:			/* PUSH Rm32 */
			{
				UINT32 value;
				if( modrm >= 0xc0 ) {
					value = LOAD_RM32(modrm);
				} else {
					UINT32 ea = GetEA(modrm);
					value = READ32(ea);
				}
				PUSH32(value);
				CYCLES(CYCLES_PUSH_RM);
			}
			break;
		default:
			fatalerror("i386: groupFF_32 /%d unimplemented at %08X", (modrm >> 3) & 0x7, I.pc-2);
			break;
	}
}

static void I386OP(group0F00_32)(void)			// Opcode 0x0f 00
{
	UINT32 address, ea;
	UINT8 modrm = FETCH();

	switch( (modrm >> 3) & 0x7 )
	{
		case 2:			/* LLDT */
			if ( PROTECTED_MODE && !V8086_MODE )
			{
				if( modrm >= 0xc0 ) {
					address = LOAD_RM32(modrm);
					ea = i386_translate( CS, address );
					CYCLES(CYCLES_LLDT_REG);
				} else {
					ea = GetEA(modrm);
					CYCLES(CYCLES_LLDT_MEM);
				}
				I.ldtr.segment = READ32(ea);
			}
			else
			{
				i386_trap(6, 0);
			}
			break;

		case 3:			/* LTR */
			if ( PROTECTED_MODE && !V8086_MODE )
			{
				if( modrm >= 0xc0 ) {
					address = LOAD_RM32(modrm);
					ea = i386_translate( CS, address );
					CYCLES(CYCLES_LTR_REG);
				} else {
					ea = GetEA(modrm);
					CYCLES(CYCLES_LTR_MEM);
				}
				I.task.segment = READ32(ea);
			}
			else
			{
				i386_trap(6, 0);
			}
			break;

		default:
			fatalerror("i386: group0F00_32 /%d unimplemented", (modrm >> 3) & 0x7);
			break;
	}
}

static void I386OP(group0F01_32)(void)		// Opcode 0x0f 01
{
	UINT8 modrm = FETCH();
	UINT32 address, ea;

	switch( (modrm >> 3) & 0x7 )
	{
		case 0:			/* SGDT */
			{
				if( modrm >= 0xc0 ) {
					address = LOAD_RM32(modrm);
					ea = i386_translate( CS, address );
				} else {
					ea = GetEA(modrm);
				}
				WRITE16(ea, I.gdtr.limit);
				WRITE32(ea + 2, I.gdtr.base);
				CYCLES(CYCLES_SGDT);
				break;
			}
		case 1:			/* SIDT */
			{
				if (modrm >= 0xc0)
				{
					address = LOAD_RM32(modrm);
					ea = i386_translate( CS, address );
				}
				else
				{
					ea = GetEA(modrm);
				}
				WRITE16(ea, I.idtr.limit);
				WRITE32(ea + 2, I.idtr.base);
				CYCLES(CYCLES_SIDT);
				break;
			}
		case 2:			/* LGDT */
			{
				if( modrm >= 0xc0 ) {
					address = LOAD_RM32(modrm);
					ea = i386_translate( CS, address );
				} else {
					ea = GetEA(modrm);
				}
				I.gdtr.limit = READ16(ea);
				I.gdtr.base = READ32(ea + 2);
				CYCLES(CYCLES_LGDT);
				break;
			}
		case 3:			/* LIDT */
			{
				if( modrm >= 0xc0 ) {
					address = LOAD_RM32(modrm);
					ea = i386_translate( CS, address );
				} else {
					ea = GetEA(modrm);
				}
				I.idtr.limit = READ16(ea);
				I.idtr.base = READ32(ea + 2);
				CYCLES(CYCLES_LIDT);
				break;
			}
		default:
			fatalerror("i386: unimplemented opcode 0x0f 01 /%d at %08X", (modrm >> 3) & 0x7, I.eip - 2);
			break;
	}
}

static void I386OP(group0FBA_32)(void)		// Opcode 0x0f ba
{
	UINT8 modrm = FETCH();

	switch( (modrm >> 3) & 0x7 )
	{
		case 4:			/* BT Rm32, i8 */
			if( modrm >= 0xc0 ) {
				UINT32 dst = LOAD_RM32(modrm);
				UINT8 bit = FETCH();

				if( dst & (1 << bit) )
					I.CF = 1;
				else
					I.CF = 0;

				CYCLES(CYCLES_BT_IMM_REG);
			} else {
				UINT32 ea = GetEA(modrm);
				UINT32 dst = READ32(ea);
				UINT8 bit = FETCH();

				if( dst & (1 << bit) )
					I.CF = 1;
				else
					I.CF = 0;

				CYCLES(CYCLES_BT_IMM_MEM);
			}
			break;
		case 5:			/* BTS Rm32, i8 */
			if( modrm >= 0xc0 ) {
				UINT32 dst = LOAD_RM32(modrm);
				UINT8 bit = FETCH();

				if( dst & (1 << bit) )
					I.CF = 1;
				else
					I.CF = 0;
				dst |= (1 << bit);

				STORE_RM32(modrm, dst);
				CYCLES(CYCLES_BTS_IMM_REG);
			} else {
				UINT32 ea = GetEA(modrm);
				UINT32 dst = READ32(ea);
				UINT8 bit = FETCH();

				if( dst & (1 << bit) )
					I.CF = 1;
				else
					I.CF = 0;
				dst |= (1 << bit);

				WRITE32(ea, dst);
				CYCLES(CYCLES_BTS_IMM_MEM);
			}
			break;
		case 6:			/* BTR Rm32, i8 */
			if( modrm >= 0xc0 ) {
				UINT32 dst = LOAD_RM32(modrm);
				UINT8 bit = FETCH();

				if( dst & (1 << bit) )
					I.CF = 1;
				else
					I.CF = 0;
				dst &= ~(1 << bit);

				STORE_RM32(modrm, dst);
				CYCLES(CYCLES_BTR_IMM_REG);
			} else {
				UINT32 ea = GetEA(modrm);
				UINT32 dst = READ32(ea);
				UINT8 bit = FETCH();

				if( dst & (1 << bit) )
					I.CF = 1;
				else
					I.CF = 0;
				dst &= ~(1 << bit);

				WRITE32(ea, dst);
				CYCLES(CYCLES_BTR_IMM_MEM);
			}
			break;
		case 7:			/* BTC Rm32, i8 */
			if( modrm >= 0xc0 ) {
				UINT32 dst = LOAD_RM32(modrm);
				UINT8 bit = FETCH();

				if( dst & (1 << bit) )
					I.CF = 1;
				else
					I.CF = 0;
				dst ^= (1 << bit);

				STORE_RM32(modrm, dst);
				CYCLES(CYCLES_BTC_IMM_REG);
			} else {
				UINT32 ea = GetEA(modrm);
				UINT32 dst = READ32(ea);
				UINT8 bit = FETCH();

				if( dst & (1 << bit) )
					I.CF = 1;
				else
					I.CF = 0;
				dst ^= (1 << bit);

				WRITE32(ea, dst);
				CYCLES(CYCLES_BTC_IMM_MEM);
			}
			break;
		default:
			fatalerror("i386: group0FBA_32 /%d unknown", (modrm >> 3) & 0x7);
			break;
	}
}

static void I386OP(bound_r32_m32_m32)(void)	// Opcode 0x62
{
	UINT8 modrm;
	INT32 val, low, high;

	modrm = FETCH();

	if (modrm >= 0xc0)
	{
		low = high = LOAD_RM32(modrm);
	}
	else
	{
		UINT32 ea = GetEA(modrm);
		low = READ32(ea + 0);
		high = READ32(ea + 4);
	}
	val = LOAD_REG32(modrm);

	if ((val < low) || (val > high))
	{
		CYCLES(CYCLES_BOUND_OUT_RANGE);
		i386_trap(5, 0);
	}
	else
	{
		CYCLES(CYCLES_BOUND_IN_RANGE);
	}
}

static void I386OP(retf32)(void)			// Opcode 0xcb
{
	I.eip = POP32();
	I.sreg[CS].selector = POP32();
	i386_load_segment_descriptor( CS );
	CHANGE_PC(I.eip);

	CYCLES(CYCLES_RET_INTERSEG);
}

static void I386OP(retf_i32)(void)			// Opcode 0xca
{
	UINT16 count = FETCH16();

	I.eip = POP32();
	I.sreg[CS].selector = POP32();
	i386_load_segment_descriptor( CS );
	CHANGE_PC(I.eip);

	REG32(ESP) += count;
	CYCLES(CYCLES_RET_IMM_INTERSEG);
}

static void I386OP(xlat32)(void)			// Opcode 0xd7
{
	UINT32 ea;
	if( I.segment_prefix ) {
		ea = i386_translate( I.segment_override, REG32(EBX) + REG8(AL) );
	} else {
		ea = i386_translate( DS, REG32(EBX) + REG8(AL) );
	}
	REG8(AL) = READ8(ea);
	CYCLES(CYCLES_XLAT);
}

static void I386OP(load_far_pointer32)(int s)
{
	UINT8 modrm = FETCH();

	if( modrm >= 0xc0 ) {
		fatalerror("NYI");
	} else {
		UINT32 ea = GetEA(modrm);
		STORE_REG32(modrm, READ32(ea + 0));
		I.sreg[s].selector = READ16(ea + 4);
		i386_load_segment_descriptor( s );
	}
}

static void I386OP(lds32)(void)				// Opcode 0xc5
{
	I386OP(load_far_pointer32)(DS);
	CYCLES(CYCLES_LDS);
}

static void I386OP(lss32)(void)				// Opcode 0x0f 0xb2
{
	I386OP(load_far_pointer32)(SS);
	CYCLES(CYCLES_LSS);
}

static void I386OP(les32)(void)				// Opcode 0xc4
{
	I386OP(load_far_pointer16)(ES);
	CYCLES(CYCLES_LES);
}

static void I386OP(lfs32)(void)				// Opcode 0x0f 0xb4
{
	I386OP(load_far_pointer16)(FS);
	CYCLES(CYCLES_LFS);
}

static void I386OP(lgs32)(void)				// Opcode 0x0f 0xb5
{
	I386OP(load_far_pointer32)(GS);
	CYCLES(CYCLES_LGS);
}
