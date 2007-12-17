static UINT16 I386OP(shift_rotate16)(UINT8 modrm, UINT32 value, UINT8 shift)
{
	UINT16 src = value;
	UINT16 dst = value;

	if( shift == 0 ) {
		CYCLES_RM(modrm, 3, 7);
	} else if( shift == 1 ) {

		switch( (modrm >> 3) & 0x7 )
		{
			case 0:			/* ROL rm16, 1 */
				I.CF = (src & 0x8000) ? 1 : 0;
				dst = (src << 1) + I.CF;
				I.OF = ((src ^ dst) & 0x8000) ? 1 : 0;
				CYCLES_RM(modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 1:			/* ROR rm16, 1 */
				I.CF = (src & 0x1) ? 1 : 0;
				dst = (I.CF << 15) | (src >> 1);
				I.OF = ((src ^ dst) & 0x8000) ? 1 : 0;
				CYCLES_RM(modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 2:			/* RCL rm16, 1 */
				dst = (src << 1) + I.CF;
				I.CF = (src & 0x8000) ? 1 : 0;
				I.OF = ((src ^ dst) & 0x8000) ? 1 : 0;
				CYCLES_RM(modrm, CYCLES_ROTATE_CARRY_REG, CYCLES_ROTATE_CARRY_MEM);
				break;
			case 3:			/* RCR rm16, 1 */
				dst = (I.CF << 15) | (src >> 1);
				I.CF = src & 0x1;
				I.OF = ((src ^ dst) & 0x8000) ? 1 : 0;
				CYCLES_RM(modrm, CYCLES_ROTATE_CARRY_REG, CYCLES_ROTATE_CARRY_MEM);
				break;
			case 4:			/* SHL/SAL rm16, 1 */
			case 6:
				dst = src << 1;
				I.CF = (src & 0x8000) ? 1 : 0;
				I.OF = (((I.CF << 15) ^ dst) & 0x8000) ? 1 : 0;
				SetSZPF16(dst);
				CYCLES_RM(modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 5:			/* SHR rm16, 1 */
				dst = src >> 1;
				I.CF = src & 0x1;
				I.OF = (dst & 0x8000) ? 1 : 0;
				SetSZPF16(dst);
				CYCLES_RM(modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 7:			/* SAR rm16, 1 */
				dst = (INT16)(src) >> 1;
				I.CF = src & 0x1;
				I.OF = 0;
				SetSZPF16(dst);
				CYCLES_RM(modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
		}
	} else {

		switch( (modrm >> 3) & 0x7 )
		{
			case 0:			/* ROL rm16, i8 */
				dst = ((src & ((UINT16)0xffff >> shift)) << shift) |
					  ((src & ((UINT16)0xffff << (16-shift))) >> (16-shift));
				I.CF = (src >> (16-shift)) & 0x1;
				CYCLES_RM(modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 1:			/* ROR rm16, i8 */
				dst = ((src & ((UINT16)0xffff << shift)) >> shift) |
					  ((src & ((UINT16)0xffff >> (16-shift))) << (16-shift));
				I.CF = (src >> (shift-1)) & 0x1;
				CYCLES_RM(modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 2:			/* RCL rm16, i8 */
				dst = ((src & ((UINT16)0xffff >> shift)) << shift) |
					  ((src & ((UINT16)0xffff << (17-shift))) >> (17-shift)) |
					  (I.CF << (shift-1));
				I.CF = (src >> (16-shift)) & 0x1;
				CYCLES_RM(modrm, CYCLES_ROTATE_CARRY_REG, CYCLES_ROTATE_CARRY_MEM);
				break;
			case 3:			/* RCR rm16, i8 */
				dst = ((src & ((UINT16)0xffff << shift)) >> shift) |
					  ((src & ((UINT16)0xffff >> (16-shift))) << (17-shift)) |
					  (I.CF << (16-shift));
				I.CF = (src >> (shift-1)) & 0x1;
				CYCLES_RM(modrm, CYCLES_ROTATE_CARRY_REG, CYCLES_ROTATE_CARRY_MEM);
				break;
			case 4:			/* SHL/SAL rm16, i8 */
			case 6:
				dst = src << shift;
				I.CF = (src & (1 << (16-shift))) ? 1 : 0;
				SetSZPF16(dst);
				CYCLES_RM(modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 5:			/* SHR rm16, i8 */
				dst = src >> shift;
				I.CF = (src & (1 << (shift-1))) ? 1 : 0;
				SetSZPF16(dst);
				CYCLES_RM(modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 7:			/* SAR rm16, i8 */
				dst = (INT16)src >> shift;
				I.CF = (src & (1 << (shift-1))) ? 1 : 0;
				SetSZPF16(dst);
				CYCLES_RM(modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
		}

	}
	return dst;
}



static void I386OP(adc_rm16_r16)(void)		// Opcode 0x11
{
	UINT16 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_REG16(modrm);
		dst = LOAD_RM16(modrm);
		src = ADD16(src, I.CF);
		dst = ADD16(dst, src);
		STORE_RM16(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		src = LOAD_REG16(modrm);
		dst = READ16(ea);
		src = ADD16(src, I.CF);
		dst = ADD16(dst, src);
		WRITE16(ea, dst);
		CYCLES(CYCLES_ALU_REG_MEM);
	}
}

static void I386OP(adc_r16_rm16)(void)		// Opcode 0x13
{
	UINT16 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_RM16(modrm);
		dst = LOAD_REG16(modrm);
		src = ADD16(src, I.CF);
		dst = ADD16(dst, src);
		STORE_REG16(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		src = READ16(ea);
		dst = LOAD_REG16(modrm);
		src = ADD16(src, I.CF);
		dst = ADD16(dst, src);
		STORE_REG16(modrm, dst);
		CYCLES(CYCLES_ALU_MEM_REG);
	}
}

static void I386OP(adc_ax_i16)(void)		// Opcode 0x15
{
	UINT16 src, dst;
	src = FETCH16();
	dst = REG16(AX);
	src = ADD16(src, I.CF);
	dst = ADD16(dst, src);
	REG16(AX) = dst;
	CYCLES(CYCLES_ALU_IMM_ACC);
}

static void I386OP(add_rm16_r16)(void)		// Opcode 0x01
{
	UINT16 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_REG16(modrm);
		dst = LOAD_RM16(modrm);
		dst = ADD16(dst, src);
		STORE_RM16(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		src = LOAD_REG16(modrm);
		dst = READ16(ea);
		dst = ADD16(dst, src);
		WRITE16(ea, dst);
		CYCLES(CYCLES_ALU_REG_MEM);
	}
}

static void I386OP(add_r16_rm16)(void)		// Opcode 0x03
{
	UINT16 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_RM16(modrm);
		dst = LOAD_REG16(modrm);
		dst = ADD16(dst, src);
		STORE_REG16(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		src = READ16(ea);
		dst = LOAD_REG16(modrm);
		dst = ADD16(dst, src);
		STORE_REG16(modrm, dst);
		CYCLES(CYCLES_ALU_MEM_REG);
	}
}

static void I386OP(add_ax_i16)(void)		// Opcode 0x05
{
	UINT16 src, dst;
	src = FETCH16();
	dst = REG16(AX);
	dst = ADD16(dst, src);
	REG16(AX) = dst;
	CYCLES(CYCLES_ALU_IMM_ACC);
}

static void I386OP(and_rm16_r16)(void)		// Opcode 0x21
{
	UINT16 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_REG16(modrm);
		dst = LOAD_RM16(modrm);
		dst = AND16(dst, src);
		STORE_RM16(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		src = LOAD_REG16(modrm);
		dst = READ16(ea);
		dst = AND16(dst, src);
		WRITE16(ea, dst);
		CYCLES(CYCLES_ALU_REG_MEM);
	}
}

static void I386OP(and_r16_rm16)(void)		// Opcode 0x23
{
	UINT16 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_RM16(modrm);
		dst = LOAD_REG16(modrm);
		dst = AND16(dst, src);
		STORE_REG16(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		src = READ16(ea);
		dst = LOAD_REG16(modrm);
		dst = AND16(dst, src);
		STORE_REG16(modrm, dst);
		CYCLES(CYCLES_ALU_MEM_REG);
	}
}

static void I386OP(and_ax_i16)(void)		// Opcode 0x25
{
	UINT16 src, dst;
	src = FETCH16();
	dst = REG16(AX);
	dst = AND16(dst, src);
	REG16(AX) = dst;
	CYCLES(CYCLES_ALU_IMM_ACC);
}

static void I386OP(bsf_r16_rm16)(void)		// Opcode 0x0f bc
{
	UINT16 src, dst, temp;
	UINT8 modrm = FETCH();

	if( modrm >= 0xc0 ) {
		src = LOAD_RM16(modrm);
	} else {
		UINT32 ea = GetEA(modrm);
		src = READ16(ea);
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
	STORE_REG16(modrm, dst);
}

static void I386OP(bsr_r16_rm16)(void)		// Opcode 0x0f bd
{
	UINT16 src, dst, temp;
	UINT8 modrm = FETCH();

	if( modrm >= 0xc0 ) {
		src = LOAD_RM16(modrm);
	} else {
		UINT32 ea = GetEA(modrm);
		src = READ16(ea);
	}

	dst = 0;

	if( src == 0 ) {
		I.ZF = 1;
	} else {
		I.ZF = 0;
		temp = 15;
		while( (src & (1 << temp)) == 0 ) {
			temp--;
			dst = temp;
			CYCLES(CYCLES_BSR);
		}
	}
	CYCLES(CYCLES_BSR_BASE);
	STORE_REG16(modrm, dst);
}


static void I386OP(bt_rm16_r16)(void)		// Opcode 0x0f a3
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		UINT16 dst = LOAD_RM16(modrm);
		UINT16 bit = LOAD_REG16(modrm);

		if( dst & (1 << bit) )
			I.CF = 1;
		else
			I.CF = 0;

		CYCLES(CYCLES_BT_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		UINT16 dst = READ16(ea);
		UINT16 bit = LOAD_REG16(modrm);

		if( dst & (1 << bit) )
			I.CF = 1;
		else
			I.CF = 0;

		CYCLES(CYCLES_BT_REG_MEM);
	}
}

static void I386OP(btc_rm16_r16)(void)		// Opcode 0x0f bb
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		UINT16 dst = LOAD_RM16(modrm);
		UINT16 bit = LOAD_REG16(modrm);

		if( dst & (1 << bit) )
			I.CF = 1;
		else
			I.CF = 0;
		dst ^= (1 << bit);

		STORE_RM16(modrm, dst);
		CYCLES(CYCLES_BTC_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		UINT16 dst = READ16(ea);
		UINT16 bit = LOAD_REG16(modrm);

		if( dst & (1 << bit) )
			I.CF = 1;
		else
			I.CF = 0;
		dst ^= (1 << bit);

		WRITE16(ea, dst);
		CYCLES(CYCLES_BTC_REG_MEM);
	}
}

static void I386OP(btr_rm16_r16)(void)		// Opcode 0x0f b3
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		UINT16 dst = LOAD_RM16(modrm);
		UINT16 bit = LOAD_REG16(modrm);

		if( dst & (1 << bit) )
			I.CF = 1;
		else
			I.CF = 0;
		dst &= ~(1 << bit);

		STORE_RM16(modrm, dst);
		CYCLES(CYCLES_BTR_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		UINT16 dst = READ16(ea);
		UINT16 bit = LOAD_REG16(modrm);

		if( dst & (1 << bit) )
			I.CF = 1;
		else
			I.CF = 0;
		dst &= ~(1 << bit);

		WRITE16(ea, dst);
		CYCLES(CYCLES_BTR_REG_MEM);
	}
}

static void I386OP(bts_rm16_r16)(void)		// Opcode 0x0f ab
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		UINT16 dst = LOAD_RM16(modrm);
		UINT16 bit = LOAD_REG16(modrm);

		if( dst & (1 << bit) )
			I.CF = 1;
		else
			I.CF = 0;
		dst |= (1 << bit);

		STORE_RM16(modrm, dst);
		CYCLES(CYCLES_BTS_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		UINT16 dst = READ16(ea);
		UINT16 bit = LOAD_REG16(modrm);

		if( dst & (1 << bit) )
			I.CF = 1;
		else
			I.CF = 0;
		dst |= (1 << bit);

		WRITE16(ea, dst);
		CYCLES(CYCLES_BTS_REG_MEM);
	}
}

static void I386OP(call_abs16)(void)		// Opcode 0x9a
{
	UINT16 offset = FETCH16();
	UINT16 ptr = FETCH16();

	if( PROTECTED_MODE ) {
		/* TODO */
		fatalerror("i386: call_abs16 in protected mode unimplemented");
	} else {
		if (I.sreg[CS].d)
		{
			PUSH32( I.sreg[CS].selector );
			PUSH32( I.eip );
		}
		else
		{
			PUSH16( I.sreg[CS].selector );
			PUSH16( I.eip );
		}
		I.sreg[CS].selector = ptr;
		I.eip = offset;
		i386_load_segment_descriptor(CS);
	}
	CYCLES(CYCLES_CALL_INTERSEG);		/* TODO: Timing = 17 + m */
	CHANGE_PC(I.eip);
}

static void I386OP(call_rel16)(void)		// Opcode 0xe8
{
	INT16 disp = FETCH16();

	PUSH16( I.eip );
	if (I.sreg[CS].d)
	{
		I.eip += disp;
	}
	else
	{
		I.eip = (I.eip + disp) & 0xffff;
	}
	CHANGE_PC(I.eip);
	CYCLES(CYCLES_CALL);		/* TODO: Timing = 7 + m */
}

static void I386OP(cbw)(void)				// Opcode 0x98
{
	REG16(AX) = (INT16)((INT8)REG8(AL));
	CYCLES(CYCLES_CBW);
}

static void I386OP(cmp_rm16_r16)(void)		// Opcode 0x39
{
	UINT16 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_REG16(modrm);
		dst = LOAD_RM16(modrm);
		SUB16(dst, src);
		CYCLES(CYCLES_CMP_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		src = LOAD_REG16(modrm);
		dst = READ16(ea);
		SUB16(dst, src);
		CYCLES(CYCLES_CMP_REG_MEM);
	}
}

static void I386OP(cmp_r16_rm16)(void)		// Opcode 0x3b
{
	UINT16 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_RM16(modrm);
		dst = LOAD_REG16(modrm);
		SUB16(dst, src);
		CYCLES(CYCLES_CMP_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		src = READ16(ea);
		dst = LOAD_REG16(modrm);
		SUB16(dst, src);
		CYCLES(CYCLES_CMP_MEM_REG);
	}
}

static void I386OP(cmp_ax_i16)(void)		// Opcode 0x3d
{
	UINT16 src, dst;
	src = FETCH16();
	dst = REG16(AX);
	SUB16(dst, src);
	CYCLES(CYCLES_CMP_IMM_ACC);
}

static void I386OP(cmpsw)(void)				// Opcode 0xa7
{
	UINT32 eas, ead;
	UINT16 src, dst;
	if( I.segment_prefix ) {
		eas = i386_translate( I.segment_override, I.address_size ? REG32(ESI) : REG16(SI) );
	} else {
		eas = i386_translate( DS, I.address_size ? REG32(ESI) : REG16(SI) );
	}
	ead = i386_translate( ES, I.address_size ? REG32(EDI) : REG16(DI) );
	src = READ16(eas);
	dst = READ16(ead);
	SUB16(dst, src);
	BUMP_SI(2);
	BUMP_DI(2);
	CYCLES(CYCLES_CMPS);
}

static void I386OP(cwd)(void)				// Opcode 0x99
{
	if( REG16(AX) & 0x8000 ) {
		REG16(DX) = 0xffff;
	} else {
		REG16(DX) = 0x0000;
	}
	CYCLES(CYCLES_CWD);
}

static void I386OP(dec_ax)(void)			// Opcode 0x48
{
	REG16(AX) = DEC16( REG16(AX) );
	CYCLES(CYCLES_DEC_REG);
}

static void I386OP(dec_cx)(void)			// Opcode 0x49
{
	REG16(CX) = DEC16( REG16(CX) );
	CYCLES(CYCLES_DEC_REG);
}

static void I386OP(dec_dx)(void)			// Opcode 0x4a
{
	REG16(DX) = DEC16( REG16(DX) );
	CYCLES(CYCLES_DEC_REG);
}

static void I386OP(dec_bx)(void)			// Opcode 0x4b
{
	REG16(BX) = DEC16( REG16(BX) );
	CYCLES(CYCLES_DEC_REG);
}

static void I386OP(dec_sp)(void)			// Opcode 0x4c
{
	REG16(SP) = DEC16( REG16(SP) );
	CYCLES(CYCLES_DEC_REG);
}

static void I386OP(dec_bp)(void)			// Opcode 0x4d
{
	REG16(BP) = DEC16( REG16(BP) );
	CYCLES(CYCLES_DEC_REG);
}

static void I386OP(dec_si)(void)			// Opcode 0x4e
{
	REG16(SI) = DEC16( REG16(SI) );
	CYCLES(CYCLES_DEC_REG);
}

static void I386OP(dec_di)(void)			// Opcode 0x4f
{
	REG16(DI) = DEC16( REG16(DI) );
	CYCLES(CYCLES_DEC_REG);
}

static void I386OP(imul_r16_rm16)(void)		// Opcode 0x0f af
{
	UINT8 modrm = FETCH();
	INT32 result;
	INT32 src, dst;
	if( modrm >= 0xc0 ) {
		src = (INT32)(INT16)LOAD_RM16(modrm);
		CYCLES(CYCLES_IMUL16_REG_REG);		/* TODO: Correct multiply timing */
	} else {
		UINT32 ea = GetEA(modrm);
		src = (INT32)(INT16)READ16(ea);
		CYCLES(CYCLES_IMUL16_REG_MEM);		/* TODO: Correct multiply timing */
	}

	dst = (INT32)(INT16)LOAD_REG16(modrm);
	result = src * dst;

	STORE_REG16(modrm, (UINT16)result);

	I.CF = I.OF = !(result == (INT32)(INT16)result);
}

static void I386OP(imul_r16_rm16_i16)(void)	// Opcode 0x69
{
	UINT8 modrm = FETCH();
	INT32 result;
	INT32 src, dst;
	if( modrm >= 0xc0 ) {
		dst = (INT32)(INT16)LOAD_RM16(modrm);
		CYCLES(CYCLES_IMUL16_REG_IMM_REG);		/* TODO: Correct multiply timing */
	} else {
		UINT32 ea = GetEA(modrm);
		dst = (INT32)(INT16)READ16(ea);
		CYCLES(CYCLES_IMUL16_MEM_IMM_REG);		/* TODO: Correct multiply timing */
	}

	src = (INT32)(INT16)FETCH16();
	result = src * dst;

	STORE_REG16(modrm, (UINT16)result);

	I.CF = I.OF = !(result == (INT32)(INT16)result);
}

static void I386OP(imul_r16_rm16_i8)(void)	// Opcode 0x6b
{
	UINT8 modrm = FETCH();
	INT32 result;
	INT32 src, dst;
	if( modrm >= 0xc0 ) {
		dst = (INT32)(INT16)LOAD_RM16(modrm);
		CYCLES(CYCLES_IMUL16_REG_IMM_REG);		/* TODO: Correct multiply timing */
	} else {
		UINT32 ea = GetEA(modrm);
		dst = (INT32)(INT16)READ16(ea);
		CYCLES(CYCLES_IMUL16_MEM_IMM_REG);		/* TODO: Correct multiply timing */
	}

	src = (INT32)(INT8)FETCH();
	result = src * dst;

	STORE_REG16(modrm, (UINT16)result);

	I.CF = I.OF = !(result == (INT32)(INT16)result);
}

static void I386OP(in_ax_i8)(void)			// Opcode 0xe5
{
	UINT16 port = FETCH();
	UINT16 data = READPORT16(port);
	REG16(AX) = data;
	CYCLES(CYCLES_IN_VAR);
}

static void I386OP(in_ax_dx)(void)			// Opcode 0xed
{
	UINT16 port = REG16(DX);
	UINT16 data = READPORT16(port);
	REG16(AX) = data;
	CYCLES(CYCLES_IN);
}

static void I386OP(inc_ax)(void)			// Opcode 0x40
{
	REG16(AX) = INC16( REG16(AX) );
	CYCLES(CYCLES_INC_REG);
}

static void I386OP(inc_cx)(void)			// Opcode 0x41
{
	REG16(CX) = INC16( REG16(CX) );
	CYCLES(CYCLES_INC_REG);
}

static void I386OP(inc_dx)(void)			// Opcode 0x42
{
	REG16(DX) = INC16( REG16(DX) );
	CYCLES(CYCLES_INC_REG);
}

static void I386OP(inc_bx)(void)			// Opcode 0x43
{
	REG16(BX) = INC16( REG16(BX) );
	CYCLES(CYCLES_INC_REG);
}

static void I386OP(inc_sp)(void)			// Opcode 0x44
{
	REG16(SP) = INC16( REG16(SP) );
	CYCLES(CYCLES_INC_REG);
}

static void I386OP(inc_bp)(void)			// Opcode 0x45
{
	REG16(BP) = INC16( REG16(BP) );
	CYCLES(CYCLES_INC_REG);
}

static void I386OP(inc_si)(void)			// Opcode 0x46
{
	REG16(SI) = INC16( REG16(SI) );
	CYCLES(CYCLES_INC_REG);
}

static void I386OP(inc_di)(void)			// Opcode 0x47
{
	REG16(DI) = INC16( REG16(DI) );
	CYCLES(CYCLES_INC_REG);
}

static void I386OP(iret16)(void)			// Opcode 0xcf
{
	if( PROTECTED_MODE ) {
		/* TODO: Virtual 8086-mode */
		/* TODO: Nested task */
		/* TODO: #SS(0) exception */
		/* TODO: All the protection-related stuff... */
		I.eip = POP16();
		I.sreg[CS].selector = POP16();
		set_flags( POP16() );
		i386_load_segment_descriptor(CS);
		CHANGE_PC(I.eip);
	} else {
		/* TODO: #SS(0) exception */
		/* TODO: #GP(0) exception */
		I.eip = POP16();
		I.sreg[CS].selector = POP16();
		set_flags( POP16() );
		i386_load_segment_descriptor(CS);
		CHANGE_PC(I.eip);
	}
	CYCLES(CYCLES_IRET);
}

static void I386OP(ja_rel16)(void)			// Opcode 0x0f 87
{
	INT16 disp = FETCH16();
	if( I.CF == 0 && I.ZF == 0 ) {
		if (I.sreg[CS].d)
		{
			I.eip += disp;
		}
		else
		{
			I.eip = (I.eip + disp) & 0xffff;
		}
		CHANGE_PC(I.eip);
		CYCLES(CYCLES_JCC_FULL_DISP);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jbe_rel16)(void)			// Opcode 0x0f 86
{
	INT16 disp = FETCH16();
	if( I.CF != 0 || I.ZF != 0 ) {
		if (I.sreg[CS].d)
		{
			I.eip += disp;
		}
		else
		{
			I.eip = (I.eip + disp) & 0xffff;
		}
		CHANGE_PC(I.eip);
		CYCLES(CYCLES_JCC_FULL_DISP);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jc_rel16)(void)			// Opcode 0x0f 82
{
	INT16 disp = FETCH16();
	if( I.CF != 0 ) {
		if (I.sreg[CS].d)
		{
			I.eip += disp;
		}
		else
		{
			I.eip = (I.eip + disp) & 0xffff;
		}
		CHANGE_PC(I.eip);
		CYCLES(CYCLES_JCC_FULL_DISP);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jg_rel16)(void)			// Opcode 0x0f 8f
{
	INT16 disp = FETCH16();
	if( I.ZF == 0 && (I.SF == I.OF) ) {
		if (I.sreg[CS].d)
		{
			I.eip += disp;
		}
		else
		{
			I.eip = (I.eip + disp) & 0xffff;
		}
		CHANGE_PC(I.eip);
		CYCLES(CYCLES_JCC_FULL_DISP);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jge_rel16)(void)			// Opcode 0x0f 8d
{
	INT16 disp = FETCH16();
	if( (I.SF == I.OF) ) {
		if (I.sreg[CS].d)
		{
			I.eip += disp;
		}
		else
		{
			I.eip = (I.eip + disp) & 0xffff;
		}
		CHANGE_PC(I.eip);
		CYCLES(CYCLES_JCC_FULL_DISP);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jl_rel16)(void)			// Opcode 0x0f 8c
{
	INT16 disp = FETCH16();
	if( (I.SF != I.OF) ) {
		if (I.sreg[CS].d)
		{
			I.eip += disp;
		}
		else
		{
			I.eip = (I.eip + disp) & 0xffff;
		}
		CHANGE_PC(I.eip);
		CYCLES(CYCLES_JCC_FULL_DISP);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jle_rel16)(void)			// Opcode 0x0f 8e
{
	INT16 disp = FETCH16();
	if( I.ZF != 0 || (I.SF != I.OF) ) {
		if (I.sreg[CS].d)
		{
			I.eip += disp;
		}
		else
		{
			I.eip = (I.eip + disp) & 0xffff;
		}
		CHANGE_PC(I.eip);
		CYCLES(CYCLES_JCC_FULL_DISP);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jnc_rel16)(void)			// Opcode 0x0f 83
{
	INT16 disp = FETCH16();
	if( I.CF == 0 ) {
		if (I.sreg[CS].d)
		{
			I.eip += disp;
		}
		else
		{
			I.eip = (I.eip + disp) & 0xffff;
		}
		CHANGE_PC(I.eip);
		CYCLES(CYCLES_JCC_FULL_DISP);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jno_rel16)(void)			// Opcode 0x0f 81
{
	INT16 disp = FETCH16();
	if( I.OF == 0 ) {
		if (I.sreg[CS].d)
		{
			I.eip += disp;
		}
		else
		{
			I.eip = (I.eip + disp) & 0xffff;
		}
		CHANGE_PC(I.eip);
		CYCLES(CYCLES_JCC_FULL_DISP);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jnp_rel16)(void)			// Opcode 0x0f 8b
{
	INT16 disp = FETCH16();
	if( I.PF == 0 ) {
		if (I.sreg[CS].d)
		{
			I.eip += disp;
		}
		else
		{
			I.eip = (I.eip + disp) & 0xffff;
		}
		CHANGE_PC(I.eip);
		CYCLES(CYCLES_JCC_FULL_DISP);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jns_rel16)(void)			// Opcode 0x0f 89
{
	INT16 disp = FETCH16();
	if( I.SF == 0 ) {
		if (I.sreg[CS].d)
		{
			I.eip += disp;
		}
		else
		{
			I.eip = (I.eip + disp) & 0xffff;
		}
		CHANGE_PC(I.eip);
		CYCLES(CYCLES_JCC_FULL_DISP);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jnz_rel16)(void)			// Opcode 0x0f 85
{
	INT16 disp = FETCH16();
	if( I.ZF == 0 ) {
		if (I.sreg[CS].d)
		{
			I.eip += disp;
		}
		else
		{
			I.eip = (I.eip + disp) & 0xffff;
		}
		CHANGE_PC(I.eip);
		CYCLES(CYCLES_JCC_FULL_DISP);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jo_rel16)(void)			// Opcode 0x0f 80
{
	INT16 disp = FETCH16();
	if( I.OF != 0 ) {
		if (I.sreg[CS].d)
		{
			I.eip += disp;
		}
		else
		{
			I.eip = (I.eip + disp) & 0xffff;
		}
		CHANGE_PC(I.eip);
		CYCLES(CYCLES_JCC_FULL_DISP);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jp_rel16)(void)			// Opcode 0x0f 8a
{
	INT16 disp = FETCH16();
	if( I.PF != 0 ) {
		if (I.sreg[CS].d)
		{
			I.eip += disp;
		}
		else
		{
			I.eip = (I.eip + disp) & 0xffff;
		}
		CHANGE_PC(I.eip);
		CYCLES(CYCLES_JCC_FULL_DISP);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(js_rel16)(void)			// Opcode 0x0f 88
{
	INT16 disp = FETCH16();
	if( I.SF != 0 ) {
		if (I.sreg[CS].d)
		{
			I.eip += disp;
		}
		else
		{
			I.eip = (I.eip + disp) & 0xffff;
		}
		CHANGE_PC(I.eip);
		CYCLES(CYCLES_JCC_FULL_DISP);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jz_rel16)(void)			// Opcode 0x0f 84
{
	INT16 disp = FETCH16();
	if( I.ZF != 0 ) {
		if (I.sreg[CS].d)
		{
			I.eip += disp;
		}
		else
		{
			I.eip = (I.eip + disp) & 0xffff;
		}
		CHANGE_PC(I.eip);
		CYCLES(CYCLES_JCC_FULL_DISP);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jcxz16)(void)			// Opcode 0xe3
{
	INT8 disp = FETCH();
	if( REG16(CX) == 0 ) {
		if (I.sreg[CS].d)
		{
			I.eip += disp;
		}
		else
		{
			I.eip = (I.eip + disp) & 0xffff;
		}
		CHANGE_PC(I.eip);
		CYCLES(CYCLES_JCXZ);		/* TODO: Timing = 9 + m */
	} else {
		CYCLES(CYCLES_JCXZ_NOBRANCH);
	}
}

static void I386OP(jmp_rel16)(void)			// Opcode 0xe9
{
	INT16 disp = FETCH16();

	if (I.sreg[CS].d)
	{
		I.eip += disp;
	}
	else
	{
		I.eip = (I.eip + disp) & 0xffff;
	}
	CHANGE_PC(I.eip);
	CYCLES(CYCLES_JMP);		/* TODO: Timing = 7 + m */
}

static void I386OP(jmp_abs16)(void)			// Opcode 0xea
{
	UINT16 address = FETCH16();
	UINT16 segment = FETCH16();

	if( PROTECTED_MODE ) {
		/* TODO: #GP */
		/* TODO: access rights, etc. */
		I.eip = address;
		I.sreg[CS].selector = segment;
		I.performed_intersegment_jump = 1;
		i386_load_segment_descriptor(CS);
		CHANGE_PC(I.eip);
	} else {
		I.eip = address;
		I.sreg[CS].selector = segment;
		I.performed_intersegment_jump = 1;
		i386_load_segment_descriptor(CS);
		CHANGE_PC(I.eip);
	}
	CYCLES(CYCLES_JMP_INTERSEG);
}

static void I386OP(lea16)(void)				// Opcode 0x8d
{
	UINT8 modrm = FETCH();
	UINT32 ea = GetNonTranslatedEA(modrm);
	STORE_REG16(modrm, ea);
	CYCLES(CYCLES_LEA);
}

static void I386OP(leave16)(void)			// Opcode 0xc9
{
	REG16(SP) = REG16(BP);
	REG16(BP) = POP16();
	CYCLES(CYCLES_LEAVE);
}

static void I386OP(lodsw)(void)				// Opcode 0xad
{
	UINT32 eas;
	if( I.segment_prefix ) {
		eas = i386_translate( I.segment_override, I.address_size ? REG32(ESI) : REG16(SI) );
	} else {
		eas = i386_translate( DS, I.address_size ? REG32(ESI) : REG16(SI) );
	}
	REG16(AX) = READ16(eas);
	BUMP_SI(2);
	CYCLES(CYCLES_LODS);
}

static void I386OP(loop16)(void)			// Opcode 0xe2
{
	INT8 disp = FETCH();
	REG16(CX)--;
	if( REG16(CX) != 0 ) {
		if (I.sreg[CS].d)
		{
			I.eip += disp;
		}
		else
		{
			I.eip = (I.eip + disp) & 0xffff;
		}
		CHANGE_PC(I.eip);
	}
	CYCLES(CYCLES_LOOP);		/* TODO: Timing = 11 + m */
}

static void I386OP(loopne16)(void)			// Opcode 0xe0
{
	INT8 disp = FETCH();
	REG16(CX)--;
	if( REG16(CX) != 0 && I.ZF == 0 ) {
		if (I.sreg[CS].d)
		{
			I.eip += disp;
		}
		else
		{
			I.eip = (I.eip + disp) & 0xffff;
		}
		CHANGE_PC(I.eip);
	}
	CYCLES(CYCLES_LOOPNZ);		/* TODO: Timing = 11 + m */
}

static void I386OP(loopz16)(void)			// Opcode 0xe1
{
	INT8 disp = FETCH();
	REG16(CX)--;
	if( REG16(CX) != 0 && I.ZF != 0 ) {
		if (I.sreg[CS].d)
		{
			I.eip += disp;
		}
		else
		{
			I.eip = (I.eip + disp) & 0xffff;
		}
		CHANGE_PC(I.eip);
	}
	CYCLES(CYCLES_LOOPZ);		/* TODO: Timing = 11 + m */
}

static void I386OP(mov_rm16_r16)(void)		// Opcode 0x89
{
	UINT16 src;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_REG16(modrm);
		STORE_RM16(modrm, src);
		CYCLES(CYCLES_MOV_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		src = LOAD_REG16(modrm);
		WRITE16(ea, src);
		CYCLES(CYCLES_MOV_REG_MEM);
	}
}

static void I386OP(mov_r16_rm16)(void)		// Opcode 0x8b
{
	UINT16 src;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_RM16(modrm);
		STORE_REG16(modrm, src);
		CYCLES(CYCLES_MOV_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		src = READ16(ea);
		STORE_REG16(modrm, src);
		CYCLES(CYCLES_MOV_MEM_REG);
	}
}

static void I386OP(mov_rm16_i16)(void)		// Opcode 0xc7
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		UINT16 value = FETCH16();
		STORE_RM16(modrm, value);
		CYCLES(CYCLES_MOV_IMM_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		UINT16 value = FETCH16();
		WRITE16(ea, value);
		CYCLES(CYCLES_MOV_IMM_MEM);
	}
}

static void I386OP(mov_ax_m16)(void)		// Opcode 0xa1
{
	UINT32 offset, ea;
	if( I.address_size ) {
		offset = FETCH32();
	} else {
		offset = FETCH16();
	}
	/* TODO: Not sure if this is correct... */
	if( I.segment_prefix ) {
		ea = i386_translate( I.segment_override, offset );
	} else {
		ea = i386_translate( DS, offset );
	}
	REG16(AX) = READ16(ea);
	CYCLES(CYCLES_MOV_MEM_ACC);
}

static void I386OP(mov_m16_ax)(void)		// Opcode 0xa3
{
	UINT32 offset, ea;
	if( I.address_size ) {
		offset = FETCH32();
	} else {
		offset = FETCH16();
	}
	/* TODO: Not sure if this is correct... */
	if( I.segment_prefix ) {
		ea = i386_translate( I.segment_override, offset );
	} else {
		ea = i386_translate( DS, offset );
	}
	WRITE16( ea, REG16(AX) );
	CYCLES(CYCLES_MOV_ACC_MEM);
}

static void I386OP(mov_ax_i16)(void)		// Opcode 0xb8
{
	REG16(AX) = FETCH16();
	CYCLES(CYCLES_MOV_IMM_REG);
}

static void I386OP(mov_cx_i16)(void)		// Opcode 0xb9
{
	REG16(CX) = FETCH16();
	CYCLES(CYCLES_MOV_IMM_REG);
}

static void I386OP(mov_dx_i16)(void)		// Opcode 0xba
{
	REG16(DX) = FETCH16();
	CYCLES(CYCLES_MOV_IMM_REG);
}

static void I386OP(mov_bx_i16)(void)		// Opcode 0xbb
{
	REG16(BX) = FETCH16();
	CYCLES(CYCLES_MOV_IMM_REG);
}

static void I386OP(mov_sp_i16)(void)		// Opcode 0xbc
{
	REG16(SP) = FETCH16();
	CYCLES(CYCLES_MOV_IMM_REG);
}

static void I386OP(mov_bp_i16)(void)		// Opcode 0xbd
{
	REG16(BP) = FETCH16();
	CYCLES(CYCLES_MOV_IMM_REG);
}

static void I386OP(mov_si_i16)(void)		// Opcode 0xbe
{
	REG16(SI) = FETCH16();
	CYCLES(CYCLES_MOV_IMM_REG);
}

static void I386OP(mov_di_i16)(void)		// Opcode 0xbf
{
	REG16(DI) = FETCH16();
	CYCLES(CYCLES_MOV_IMM_REG);
}

static void I386OP(movsw)(void)				// Opcode 0xa5
{
	UINT32 eas, ead;
	UINT16 v;
	if( I.segment_prefix ) {
		eas = i386_translate( I.segment_override, I.address_size ? REG32(ESI) : REG16(SI) );
	} else {
		eas = i386_translate( DS, I.address_size ? REG32(ESI) : REG16(SI) );
	}
	ead = i386_translate( ES, I.address_size ? REG32(EDI) : REG16(DI) );
	v = READ16(eas);
	WRITE16(ead, v);
	BUMP_SI(2);
	BUMP_DI(2);
	CYCLES(CYCLES_MOVS);
}

static void I386OP(movsx_r16_rm8)(void)		// Opcode 0x0f be
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		INT16 src = (INT8)LOAD_RM8(modrm);
		STORE_REG16(modrm, src);
		CYCLES(CYCLES_MOVSX_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		INT16 src = (INT8)READ8(ea);
		STORE_REG16(modrm, src);
		CYCLES(CYCLES_MOVSX_MEM_REG);
	}
}

static void I386OP(movzx_r16_rm8)(void)		// Opcode 0x0f b6
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		UINT16 src = (UINT8)LOAD_RM8(modrm);
		STORE_REG16(modrm, src);
		CYCLES(CYCLES_MOVZX_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		UINT16 src = (UINT8)READ8(ea);
		STORE_REG16(modrm, src);
		CYCLES(CYCLES_MOVZX_MEM_REG);
	}
}

static void I386OP(or_rm16_r16)(void)		// Opcode 0x09
{
	UINT16 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_REG16(modrm);
		dst = LOAD_RM16(modrm);
		dst = OR16(dst, src);
		STORE_RM16(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		src = LOAD_REG16(modrm);
		dst = READ16(ea);
		dst = OR16(dst, src);
		WRITE16(ea, dst);
		CYCLES(CYCLES_ALU_REG_MEM);
	}
}

static void I386OP(or_r16_rm16)(void)		// Opcode 0x0b
{
	UINT16 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_RM16(modrm);
		dst = LOAD_REG16(modrm);
		dst = OR16(dst, src);
		STORE_REG16(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		src = READ16(ea);
		dst = LOAD_REG16(modrm);
		dst = OR16(dst, src);
		STORE_REG16(modrm, dst);
		CYCLES(CYCLES_ALU_MEM_REG);
	}
}

static void I386OP(or_ax_i16)(void)			// Opcode 0x0d
{
	UINT16 src, dst;
	src = FETCH16();
	dst = REG16(AX);
	dst = OR16(dst, src);
	REG16(AX) = dst;
	CYCLES(CYCLES_ALU_IMM_ACC);
}

static void I386OP(out_ax_i8)(void)			// Opcode 0xe7
{
	UINT16 port = FETCH();
	UINT16 data = REG16(AX);
	WRITEPORT16(port, data);
	CYCLES(CYCLES_OUT_VAR);
}

static void I386OP(out_ax_dx)(void)			// Opcode 0xef
{
	UINT16 port = REG16(DX);
	UINT16 data = REG16(AX);
	WRITEPORT16(port, data);
	CYCLES(CYCLES_OUT);
}

static void I386OP(pop_ax)(void)			// Opcode 0x58
{
	REG16(AX) = POP16();
	CYCLES(CYCLES_POP_REG_SHORT);
}

static void I386OP(pop_cx)(void)			// Opcode 0x59
{
	REG16(CX) = POP16();
	CYCLES(CYCLES_POP_REG_SHORT);
}

static void I386OP(pop_dx)(void)			// Opcode 0x5a
{
	REG16(DX) = POP16();
	CYCLES(CYCLES_POP_REG_SHORT);
}

static void I386OP(pop_bx)(void)			// Opcode 0x5b
{
	REG16(BX) = POP16();
	CYCLES(CYCLES_POP_REG_SHORT);
}

static void I386OP(pop_sp)(void)			// Opcode 0x5c
{
	REG16(SP) = POP16();
	CYCLES(CYCLES_POP_REG_SHORT);
}

static void I386OP(pop_bp)(void)			// Opcode 0x5d
{
	REG16(BP) = POP16();
	CYCLES(CYCLES_POP_REG_SHORT);
}

static void I386OP(pop_si)(void)			// Opcode 0x5e
{
	REG16(SI) = POP16();
	CYCLES(CYCLES_POP_REG_SHORT);
}

static void I386OP(pop_di)(void)			// Opcode 0x5f
{
	REG16(DI) = POP16();
	CYCLES(CYCLES_POP_REG_SHORT);
}

static void I386OP(pop_ds16)(void)			// Opcode 0x1f
{
	I.sreg[DS].selector = POP16();
	if( PROTECTED_MODE ) {
		i386_load_segment_descriptor(DS);
	} else {
		i386_load_segment_descriptor(DS);
	}
	CYCLES(CYCLES_POP_SREG);
}

static void I386OP(pop_es16)(void)			// Opcode 0x07
{
	I.sreg[ES].selector = POP16();
	if( PROTECTED_MODE ) {
		i386_load_segment_descriptor(ES);
	} else {
		i386_load_segment_descriptor(ES);
	}
	CYCLES(CYCLES_POP_SREG);
}

static void I386OP(pop_fs16)(void)			// Opcode 0x0f a1
{
	I.sreg[FS].selector = POP16();
	if( PROTECTED_MODE ) {
		i386_load_segment_descriptor(FS);
	} else {
		i386_load_segment_descriptor(FS);
	}
	CYCLES(CYCLES_POP_SREG);
}

static void I386OP(pop_gs16)(void)			// Opcode 0x0f a9
{
	I.sreg[GS].selector = POP16();
	if( PROTECTED_MODE ) {
		i386_load_segment_descriptor(GS);
	} else {
		i386_load_segment_descriptor(GS);
	}
	CYCLES(CYCLES_POP_SREG);
}

static void I386OP(pop_ss16)(void)			// Opcode 0x17
{
	I.sreg[SS].selector = POP16();
	if( PROTECTED_MODE ) {
		i386_load_segment_descriptor(SS);
	} else {
		i386_load_segment_descriptor(SS);
	}
	CYCLES(CYCLES_POP_SREG);
}

static void I386OP(pop_rm16)(void)			// Opcode 0x8f
{
	UINT8 modrm = FETCH();
	UINT16 value = POP16();

	if( modrm >= 0xc0 ) {
		STORE_RM16(modrm, value);
	} else {
		UINT32 ea = GetEA(modrm);
		WRITE16(ea, value);
	}
	CYCLES(CYCLES_POP_RM);
}

static void I386OP(popa)(void)				// Opcode 0x61
{
	REG16(DI) = POP16();
	REG16(SI) = POP16();
	REG16(BP) = POP16();
	REG16(SP) += 2;
	REG16(BX) = POP16();
	REG16(DX) = POP16();
	REG16(CX) = POP16();
	REG16(AX) = POP16();
	CYCLES(CYCLES_POPA);
}

static void I386OP(popf)(void)				// Opcode 0x9d
{
	UINT16 value = POP16();
	set_flags(value);
	CYCLES(CYCLES_POPF);
}

static void I386OP(push_ax)(void)			// Opcode 0x50
{
	PUSH16( REG16(AX) );
	CYCLES(CYCLES_PUSH_REG_SHORT);
}

static void I386OP(push_cx)(void)			// Opcode 0x51
{
	PUSH16( REG16(CX) );
	CYCLES(CYCLES_PUSH_REG_SHORT);
}

static void I386OP(push_dx)(void)			// Opcode 0x52
{
	PUSH16( REG16(DX) );
	CYCLES(CYCLES_PUSH_REG_SHORT);
}

static void I386OP(push_bx)(void)			// Opcode 0x53
{
	PUSH16( REG16(BX) );
	CYCLES(CYCLES_PUSH_REG_SHORT);
}

static void I386OP(push_sp)(void)			// Opcode 0x54
{
	PUSH16( REG16(SP) );
	CYCLES(CYCLES_PUSH_REG_SHORT);
}

static void I386OP(push_bp)(void)			// Opcode 0x55
{
	PUSH16( REG16(BP) );
	CYCLES(CYCLES_PUSH_REG_SHORT);
}

static void I386OP(push_si)(void)			// Opcode 0x56
{
	PUSH16( REG16(SI) );
	CYCLES(CYCLES_PUSH_REG_SHORT);
}

static void I386OP(push_di)(void)			// Opcode 0x57
{
	PUSH16( REG16(DI) );
	CYCLES(CYCLES_PUSH_REG_SHORT);
}

static void I386OP(push_cs16)(void)			// Opcode 0x0e
{
	PUSH16( I.sreg[CS].selector );
	CYCLES(CYCLES_PUSH_SREG);
}

static void I386OP(push_ds16)(void)			// Opcode 0x1e
{
	PUSH16( I.sreg[DS].selector );
	CYCLES(CYCLES_PUSH_SREG);
}

static void I386OP(push_es16)(void)			// Opcode 0x06
{
	PUSH16( I.sreg[ES].selector );
	CYCLES(CYCLES_PUSH_SREG);
}

static void I386OP(push_fs16)(void)			// Opcode 0x0f a0
{
	PUSH16( I.sreg[FS].selector );
	CYCLES(CYCLES_PUSH_SREG);
}

static void I386OP(push_gs16)(void)			// Opcode 0x0f a8
{
	PUSH16( I.sreg[GS].selector );
	CYCLES(CYCLES_PUSH_SREG);
}

static void I386OP(push_ss16)(void)			// Opcode 0x16
{
	PUSH16( I.sreg[SS].selector );
	CYCLES(CYCLES_PUSH_SREG);
}

static void I386OP(push_i16)(void)			// Opcode 0x68
{
	UINT16 value = FETCH16();
	PUSH16(value);
	CYCLES(CYCLES_PUSH_IMM);
}

static void I386OP(pusha)(void)				// Opcode 0x60
{
	UINT16 temp = REG16(SP);
	PUSH16( REG16(AX) );
	PUSH16( REG16(CX) );
	PUSH16( REG16(DX) );
	PUSH16( REG16(BX) );
	PUSH16( temp );
	PUSH16( REG16(BP) );
	PUSH16( REG16(SI) );
	PUSH16( REG16(DI) );
	CYCLES(CYCLES_PUSHA);
}

static void I386OP(pushf)(void)				// Opcode 0x9c
{
	PUSH16( get_flags() & 0xffff );
	CYCLES(CYCLES_PUSHF);
}

static void I386OP(ret_near16_i16)(void)	// Opcode 0xc2
{
	INT16 disp = FETCH16();
	I.eip = POP16();
	REG16(SP) += disp;
	CHANGE_PC(I.eip);
	CYCLES(CYCLES_RET_IMM);		/* TODO: Timing = 10 + m */
}

static void I386OP(ret_near16)(void)		// Opcode 0xc3
{
	I.eip = POP16();
	CHANGE_PC(I.eip);
	CYCLES(CYCLES_RET);		/* TODO: Timing = 10 + m */
}

static void I386OP(sbb_rm16_r16)(void)		// Opcode 0x19
{
	UINT16 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_REG16(modrm) + I.CF;
		dst = LOAD_RM16(modrm);
		dst = SUB16(dst, src);
		STORE_RM16(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		src = LOAD_REG16(modrm) + I.CF;
		dst = READ16(ea);
		dst = SUB16(dst, src);
		WRITE16(ea, dst);
		CYCLES(CYCLES_ALU_REG_MEM);
	}
}

static void I386OP(sbb_r16_rm16)(void)		// Opcode 0x1b
{
	UINT16 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_RM16(modrm) + I.CF;
		dst = LOAD_REG16(modrm);
		dst = SUB16(dst, src);
		STORE_REG16(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		src = READ16(ea) + I.CF;
		dst = LOAD_REG16(modrm);
		dst = SUB16(dst, src);
		STORE_REG16(modrm, dst);
		CYCLES(CYCLES_ALU_MEM_REG);
	}
}

static void I386OP(sbb_ax_i16)(void)		// Opcode 0x1d
{
	UINT16 src, dst;
	src = FETCH16() + I.CF;
	dst = REG16(AX);
	dst = SUB16(dst, src);
	REG16(AX) = dst;
	CYCLES(CYCLES_ALU_IMM_ACC);
}

static void I386OP(scasw)(void)				// Opcode 0xaf
{
	UINT32 eas;
	UINT16 src, dst;
	eas = i386_translate( ES, I.address_size ? REG32(EDI) : REG16(DI) );
	src = READ16(eas);
	dst = REG16(AX);
	SUB16(dst, src);
	BUMP_DI(2);
	CYCLES(CYCLES_SCAS);
}

static void I386OP(shld16_i8)(void)			// Opcode 0x0f a4
{
	/* TODO: Correct flags */
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		UINT16 dst = LOAD_RM16(modrm);
		UINT16 upper = LOAD_REG16(modrm);
		UINT8 shift = FETCH();
		if( shift > 31 || shift == 0 ) {

		} else if( shift > 15 ) {
			I.CF = (dst & (1 << (16-shift))) ? 1 : 0;
			dst = (upper << (shift-16)) | (upper >> (32-shift));
			SetSZPF16(dst);
		} else {
			I.CF = (dst & (1 << (16-shift))) ? 1 : 0;
			dst = (dst << shift) | (upper >> (16-shift));
			SetSZPF16(dst);
		}
		STORE_RM16(modrm, dst);
		CYCLES(CYCLES_SHLD_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		UINT16 dst = READ16(ea);
		UINT16 upper = LOAD_REG16(modrm);
		UINT8 shift = FETCH();
		if( shift > 31 || shift == 0 ) {

		} else if( shift > 15 ) {
			I.CF = (dst & (1 << (16-shift))) ? 1 : 0;
			dst = (upper << (shift-16)) | (upper >> (32-shift));
			SetSZPF16(dst);
		} else {
			I.CF = (dst & (1 << (16-shift))) ? 1 : 0;
			dst = (dst << shift) | (upper >> (16-shift));
			SetSZPF16(dst);
		}
		WRITE16(ea, dst);
		CYCLES(CYCLES_SHLD_MEM);
	}
}

static void I386OP(shld16_cl)(void)			// Opcode 0x0f a5
{
	/* TODO: Correct flags */
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		UINT16 dst = LOAD_RM16(modrm);
		UINT16 upper = LOAD_REG16(modrm);
		UINT8 shift = REG8(CL);
		if( shift > 31 || shift == 0 ) {

		} else if( shift > 15 ) {
			I.CF = (dst & (1 << (16-shift))) ? 1 : 0;
			dst = (upper << (shift-16)) | (upper >> (32-shift));
			SetSZPF16(dst);
		} else {
			I.CF = (dst & (1 << (16-shift))) ? 1 : 0;
			dst = (dst << shift) | (upper >> (16-shift));
			SetSZPF16(dst);
		}
		STORE_RM16(modrm, dst);
		CYCLES(CYCLES_SHLD_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		UINT16 dst = READ16(ea);
		UINT16 upper = LOAD_REG16(modrm);
		UINT8 shift = REG8(CL);
		if( shift > 31 || shift == 0 ) {

		} else if( shift > 15 ) {
			I.CF = (dst & (1 << (16-shift))) ? 1 : 0;
			dst = (upper << (shift-16)) | (upper >> (32-shift));
			SetSZPF16(dst);
		} else {
			I.CF = (dst & (1 << (16-shift))) ? 1 : 0;
			dst = (dst << shift) | (upper >> (16-shift));
			SetSZPF16(dst);
		}
		WRITE16(ea, dst);
		CYCLES(CYCLES_SHLD_MEM);
	}
}

static void I386OP(shrd16_i8)(void)			// Opcode 0x0f ac
{
	/* TODO: Correct flags */
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		UINT16 dst = LOAD_RM16(modrm);
		UINT16 upper = LOAD_REG16(modrm);
		UINT8 shift = FETCH();
		if( shift > 15 || shift == 0) {

		} else {
			I.CF = (dst & (1 << (shift-1))) ? 1 : 0;
			dst = (dst >> shift) | (upper << (16-shift));
			SetSZPF16(dst);
		}
		STORE_RM16(modrm, dst);
		CYCLES(CYCLES_SHRD_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		UINT16 dst = READ16(ea);
		UINT16 upper = LOAD_REG16(modrm);
		UINT8 shift = FETCH();
		if( shift > 15 || shift == 0) {

		} else {
			I.CF = (dst & (1 << (shift-1))) ? 1 : 0;
			dst = (dst >> shift) | (upper << (16-shift));
			SetSZPF16(dst);
		}
		WRITE16(ea, dst);
		CYCLES(CYCLES_SHRD_MEM);
	}
}

static void I386OP(shrd16_cl)(void)			// Opcode 0x0f ad
{
	/* TODO: Correct flags */
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		UINT16 dst = LOAD_RM16(modrm);
		UINT16 upper = LOAD_REG16(modrm);
		UINT8 shift = REG8(CL);
		if( shift > 15 || shift == 0) {

		} else {
			I.CF = (dst & (1 << (shift-1))) ? 1 : 0;
			dst = (dst >> shift) | (upper << (16-shift));
			SetSZPF16(dst);
		}
		STORE_RM16(modrm, dst);
		CYCLES(CYCLES_SHRD_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		UINT16 dst = READ16(ea);
		UINT16 upper = LOAD_REG16(modrm);
		UINT8 shift = REG8(CL);
		if( shift > 15 || shift == 0) {

		} else {
			I.CF = (dst & (1 << (shift-1))) ? 1 : 0;
			dst = (dst >> shift) | (upper << (16-shift));
			SetSZPF16(dst);
		}
		WRITE16(ea, dst);
		CYCLES(CYCLES_SHRD_MEM);
	}
}

static void I386OP(stosw)(void)				// Opcode 0xab
{
	UINT32 ead;
	ead = i386_translate( ES, I.address_size ? REG32(EDI) : REG16(DI) );
	WRITE16(ead, REG16(AX));
	BUMP_DI(2);
	CYCLES(CYCLES_STOS);
}

static void I386OP(sub_rm16_r16)(void)		// Opcode 0x29
{
	UINT16 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_REG16(modrm);
		dst = LOAD_RM16(modrm);
		dst = SUB16(dst, src);
		STORE_RM16(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		src = LOAD_REG16(modrm);
		dst = READ16(ea);
		dst = SUB16(dst, src);
		WRITE16(ea, dst);
		CYCLES(CYCLES_ALU_REG_MEM);
	}
}

static void I386OP(sub_r16_rm16)(void)		// Opcode 0x2b
{
	UINT16 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_RM16(modrm);
		dst = LOAD_REG16(modrm);
		dst = SUB16(dst, src);
		STORE_REG16(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		src = READ16(ea);
		dst = LOAD_REG16(modrm);
		dst = SUB16(dst, src);
		STORE_REG16(modrm, dst);
		CYCLES(CYCLES_ALU_MEM_REG);
	}
}

static void I386OP(sub_ax_i16)(void)		// Opcode 0x2d
{
	UINT16 src, dst;
	src = FETCH16();
	dst = REG16(AX);
	dst = SUB16(dst, src);
	REG16(AX) = dst;
	CYCLES(CYCLES_ALU_IMM_ACC);
}

static void I386OP(test_ax_i16)(void)		// Opcode 0xa9
{
	UINT16 src = FETCH16();
	UINT16 dst = REG16(AX);
	dst = src & dst;
	SetSZPF16(dst);
	I.CF = 0;
	I.OF = 0;
	CYCLES(CYCLES_TEST_IMM_ACC);
}

static void I386OP(test_rm16_r16)(void)		// Opcode 0x85
{
	UINT16 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_REG16(modrm);
		dst = LOAD_RM16(modrm);
		dst = src & dst;
		SetSZPF16(dst);
		I.CF = 0;
		I.OF = 0;
		CYCLES(CYCLES_TEST_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		src = LOAD_REG16(modrm);
		dst = READ16(ea);
		dst = src & dst;
		SetSZPF16(dst);
		I.CF = 0;
		I.OF = 0;
		CYCLES(CYCLES_TEST_REG_MEM);
	}
}

static void I386OP(xchg_ax_cx)(void)		// Opcode 0x91
{
	UINT16 temp;
	temp = REG16(AX);
	REG16(AX) = REG16(CX);
	REG16(CX) = temp;
	CYCLES(CYCLES_XCHG_REG_REG);
}

static void I386OP(xchg_ax_dx)(void)		// Opcode 0x92
{
	UINT16 temp;
	temp = REG16(AX);
	REG16(AX) = REG16(DX);
	REG16(DX) = temp;
	CYCLES(CYCLES_XCHG_REG_REG);
}

static void I386OP(xchg_ax_bx)(void)		// Opcode 0x93
{
	UINT16 temp;
	temp = REG16(AX);
	REG16(AX) = REG16(BX);
	REG16(BX) = temp;
	CYCLES(CYCLES_XCHG_REG_REG);
}

static void I386OP(xchg_ax_sp)(void)		// Opcode 0x94
{
	UINT16 temp;
	temp = REG16(AX);
	REG16(AX) = REG16(SP);
	REG16(SP) = temp;
	CYCLES(CYCLES_XCHG_REG_REG);
}

static void I386OP(xchg_ax_bp)(void)		// Opcode 0x95
{
	UINT16 temp;
	temp = REG16(AX);
	REG16(AX) = REG16(BP);
	REG16(BP) = temp;
	CYCLES(CYCLES_XCHG_REG_REG);
}

static void I386OP(xchg_ax_si)(void)		// Opcode 0x96
{
	UINT16 temp;
	temp = REG16(AX);
	REG16(AX) = REG16(SI);
	REG16(SI) = temp;
	CYCLES(CYCLES_XCHG_REG_REG);
}

static void I386OP(xchg_ax_di)(void)		// Opcode 0x97
{
	UINT16 temp;
	temp = REG16(AX);
	REG16(AX) = REG16(DI);
	REG16(DI) = temp;
	CYCLES(CYCLES_XCHG_REG_REG);
}

static void I386OP(xchg_r16_rm16)(void)		// Opcode 0x87
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		UINT16 src = LOAD_RM16(modrm);
		UINT16 dst = LOAD_REG16(modrm);
		STORE_REG16(modrm, src);
		STORE_RM16(modrm, dst);
		CYCLES(CYCLES_XCHG_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		UINT16 src = READ16(ea);
		UINT16 dst = LOAD_REG16(modrm);
		STORE_REG16(modrm, src);
		WRITE16(ea, dst);
		CYCLES(CYCLES_XCHG_REG_MEM);
	}
}

static void I386OP(xor_rm16_r16)(void)		// Opcode 0x31
{
	UINT16 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_REG16(modrm);
		dst = LOAD_RM16(modrm);
		dst = XOR16(dst, src);
		STORE_RM16(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		src = LOAD_REG16(modrm);
		dst = READ16(ea);
		dst = XOR16(dst, src);
		WRITE16(ea, dst);
		CYCLES(CYCLES_ALU_REG_MEM);
	}
}

static void I386OP(xor_r16_rm16)(void)		// Opcode 0x33
{
	UINT16 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_RM16(modrm);
		dst = LOAD_REG16(modrm);
		dst = XOR16(dst, src);
		STORE_REG16(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		src = READ16(ea);
		dst = LOAD_REG16(modrm);
		dst = XOR16(dst, src);
		STORE_REG16(modrm, dst);
		CYCLES(CYCLES_ALU_MEM_REG);
	}
}

static void I386OP(xor_ax_i16)(void)		// Opcode 0x35
{
	UINT16 src, dst;
	src = FETCH16();
	dst = REG16(AX);
	dst = XOR16(dst, src);
	REG16(AX) = dst;
	CYCLES(CYCLES_ALU_IMM_ACC);
}



static void I386OP(group81_16)(void)		// Opcode 0x81
{
	UINT32 ea;
	UINT16 src, dst;
	UINT8 modrm = FETCH();

	switch( (modrm >> 3) & 0x7 )
	{
		case 0:		// ADD Rm16, i16
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM16(modrm);
				src = FETCH16();
				dst = ADD16(dst, src);
				STORE_RM16(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm);
				dst = READ16(ea);
				src = FETCH16();
				dst = ADD16(dst, src);
				WRITE16(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 1:		// OR Rm16, i16
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM16(modrm);
				src = FETCH16();
				dst = OR16(dst, src);
				STORE_RM16(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm);
				dst = READ16(ea);
				src = FETCH16();
				dst = OR16(dst, src);
				WRITE16(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 2:		// ADC Rm16, i16
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM16(modrm);
				src = FETCH16();
				src = ADD16(src, I.CF);
				dst = ADD16(dst, src);
				STORE_RM16(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm);
				dst = READ16(ea);
				src = FETCH16();
				src = ADD16(src, I.CF);
				dst = ADD16(dst, src);
				WRITE16(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 3:		// SBB Rm16, i16
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM16(modrm);
				src = FETCH16() + I.CF;
				dst = SUB16(dst, src);
				STORE_RM16(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm);
				dst = READ16(ea);
				src = FETCH16() + I.CF;
				dst = SUB16(dst, src);
				WRITE16(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 4:		// AND Rm16, i16
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM16(modrm);
				src = FETCH16();
				dst = AND16(dst, src);
				STORE_RM16(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm);
				dst = READ16(ea);
				src = FETCH16();
				dst = AND16(dst, src);
				WRITE16(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 5:		// SUB Rm16, i16
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM16(modrm);
				src = FETCH16();
				dst = SUB16(dst, src);
				STORE_RM16(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm);
				dst = READ16(ea);
				src = FETCH16();
				dst = SUB16(dst, src);
				WRITE16(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 6:		// XOR Rm16, i16
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM16(modrm);
				src = FETCH16();
				dst = XOR16(dst, src);
				STORE_RM16(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm);
				dst = READ16(ea);
				src = FETCH16();
				dst = XOR16(dst, src);
				WRITE16(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 7:		// CMP Rm16, i16
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM16(modrm);
				src = FETCH16();
				SUB16(dst, src);
				CYCLES(CYCLES_CMP_REG_REG);
			} else {
				ea = GetEA(modrm);
				dst = READ16(ea);
				src = FETCH16();
				SUB16(dst, src);
				CYCLES(CYCLES_CMP_REG_MEM);
			}
			break;
	}
}

static void I386OP(group83_16)(void)		// Opcode 0x83
{
	UINT32 ea;
	UINT16 src, dst;
	UINT8 modrm = FETCH();

	switch( (modrm >> 3) & 0x7 )
	{
		case 0:		// ADD Rm16, i16
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM16(modrm);
				src = (UINT16)(INT16)(INT8)FETCH();
				dst = ADD16(dst, src);
				STORE_RM16(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm);
				dst = READ16(ea);
				src = (UINT16)(INT16)(INT8)FETCH();
				dst = ADD16(dst, src);
				WRITE16(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 1:		// OR Rm16, i16
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM16(modrm);
				src = (UINT16)(INT16)(INT8)FETCH();
				dst = OR16(dst, src);
				STORE_RM16(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm);
				dst = READ16(ea);
				src = (UINT16)(INT16)(INT8)FETCH();
				dst = OR16(dst, src);
				WRITE16(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 2:		// ADC Rm16, i16
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM16(modrm);
				src = (UINT16)(INT16)(INT8)FETCH();
				src = ADD16(src, I.CF);
				dst = ADD16(dst, src);
				STORE_RM16(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm);
				dst = READ16(ea);
				src = (UINT16)(INT16)(INT8)FETCH();
				src = ADD16(src, I.CF);
				dst = ADD16(dst, src);
				WRITE16(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 3:		// SBB Rm16, i16
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM16(modrm);
				src = ((UINT16)(INT16)(INT8)FETCH()) + I.CF;
				dst = SUB16(dst, src);
				STORE_RM16(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm);
				dst = READ16(ea);
				src = ((UINT16)(INT16)(INT8)FETCH()) + I.CF;
				dst = SUB16(dst, src);
				WRITE16(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 4:		// AND Rm16, i16
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM16(modrm);
				src = (UINT16)(INT16)(INT8)FETCH();
				dst = AND16(dst, src);
				STORE_RM16(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm);
				dst = READ16(ea);
				src = (UINT16)(INT16)(INT8)FETCH();
				dst = AND16(dst, src);
				WRITE16(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 5:		// SUB Rm16, i16
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM16(modrm);
				src = (UINT16)(INT16)(INT8)FETCH();
				dst = SUB16(dst, src);
				STORE_RM16(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm);
				dst = READ16(ea);
				src = (UINT16)(INT16)(INT8)FETCH();
				dst = SUB16(dst, src);
				WRITE16(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 6:		// XOR Rm16, i16
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM16(modrm);
				src = (UINT16)(INT16)(INT8)FETCH();
				dst = XOR16(dst, src);
				STORE_RM16(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm);
				dst = READ16(ea);
				src = (UINT16)(INT16)(INT8)FETCH();
				dst = XOR16(dst, src);
				WRITE16(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 7:		// CMP Rm16, i16
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM16(modrm);
				src = (UINT16)(INT16)(INT8)FETCH();
				SUB16(dst, src);
				CYCLES(CYCLES_CMP_REG_REG);
			} else {
				ea = GetEA(modrm);
				dst = READ16(ea);
				src = (UINT16)(INT16)(INT8)FETCH();
				SUB16(dst, src);
				CYCLES(CYCLES_CMP_REG_MEM);
			}
			break;
	}
}

static void I386OP(groupC1_16)(void)		// Opcode 0xc1
{
	UINT16 dst;
	UINT8 modrm = FETCH();
	UINT8 shift;

	if( modrm >= 0xc0 ) {
		dst = LOAD_RM16(modrm);
		shift = FETCH() & 0x1f;
		dst = i386_shift_rotate16(modrm, dst, shift);
		STORE_RM16(modrm, dst);
	} else {
		UINT32 ea = GetEA(modrm);
		dst = READ16(ea);
		shift = FETCH() & 0x1f;
		dst = i386_shift_rotate16(modrm, dst, shift);
		WRITE16(ea, dst);
	}
}

static void I386OP(groupD1_16)(void)		// Opcode 0xd1
{
	UINT16 dst;
	UINT8 modrm = FETCH();

	if( modrm >= 0xc0 ) {
		dst = LOAD_RM16(modrm);
		dst = i386_shift_rotate16(modrm, dst, 1);
		STORE_RM16(modrm, dst);
	} else {
		UINT32 ea = GetEA(modrm);
		dst = READ16(ea);
		dst = i386_shift_rotate16(modrm, dst, 1);
		WRITE16(ea, dst);
	}
}

static void I386OP(groupD3_16)(void)		// Opcode 0xd3
{
	UINT16 dst;
	UINT8 modrm = FETCH();

	if( modrm >= 0xc0 ) {
		dst = LOAD_RM16(modrm);
		dst = i386_shift_rotate16(modrm, dst, REG8(CL));
		STORE_RM16(modrm, dst);
	} else {
		UINT32 ea = GetEA(modrm);
		dst = READ16(ea);
		dst = i386_shift_rotate16(modrm, dst, REG8(CL));
		WRITE16(ea, dst);
	}
}

static void I386OP(groupF7_16)(void)		// Opcode 0xf7
{
	UINT8 modrm = FETCH();

	switch( (modrm >> 3) & 0x7 )
	{
		case 0:			/* TEST Rm16, i16 */
			if( modrm >= 0xc0 ) {
				UINT16 dst = LOAD_RM16(modrm);
				UINT16 src = FETCH16();
				dst &= src;
				I.CF = I.OF = I.AF = 0;
				SetSZPF16(dst);
				CYCLES(CYCLES_TEST_IMM_REG);
			} else {
				UINT32 ea = GetEA(modrm);
				UINT16 dst = READ16(ea);
				UINT16 src = FETCH16();
				dst &= src;
				I.CF = I.OF = I.AF = 0;
				SetSZPF16(dst);
				CYCLES(CYCLES_TEST_IMM_MEM);
			}
			break;
		case 2:			/* NOT Rm16 */
			if( modrm >= 0xc0 ) {
				UINT16 dst = LOAD_RM16(modrm);
				dst = ~dst;
				STORE_RM16(modrm, dst);
				CYCLES(CYCLES_NOT_REG);
			} else {
				UINT32 ea = GetEA(modrm);
				UINT16 dst = READ16(ea);
				dst = ~dst;
				WRITE16(ea, dst);
				CYCLES(CYCLES_NOT_MEM);
			}
			break;
		case 3:			/* NEG Rm16 */
			if( modrm >= 0xc0 ) {
				UINT16 dst = LOAD_RM16(modrm);
				dst = SUB16( 0, dst );
				STORE_RM16(modrm, dst);
				CYCLES(CYCLES_NEG_REG);
			} else {
				UINT32 ea = GetEA(modrm);
				UINT16 dst = READ16(ea);
				dst = SUB16( 0, dst );
				WRITE16(ea, dst);
				CYCLES(CYCLES_NEG_MEM);
			}
			break;
		case 4:			/* MUL AX, Rm16 */
			{
				UINT32 result;
				UINT16 src, dst;
				if( modrm >= 0xc0 ) {
					src = LOAD_RM16(modrm);
					CYCLES(CYCLES_MUL16_ACC_REG);		/* TODO: Correct multiply timing */
				} else {
					UINT32 ea = GetEA(modrm);
					src = READ16(ea);
					CYCLES(CYCLES_MUL16_ACC_MEM);		/* TODO: Correct multiply timing */
				}

				dst = REG16(AX);
				result = (UINT32)src * (UINT32)dst;
				REG16(DX) = (UINT16)(result >> 16);
				REG16(AX) = (UINT16)result;

				I.CF = I.OF = (REG16(DX) != 0);
			}
			break;
		case 5:			/* IMUL AX, Rm16 */
			{
				INT32 result;
				INT32 src, dst;
				if( modrm >= 0xc0 ) {
					src = (INT32)(INT16)LOAD_RM16(modrm);
					CYCLES(CYCLES_IMUL16_ACC_REG);		/* TODO: Correct multiply timing */
				} else {
					UINT32 ea = GetEA(modrm);
					src = (INT32)(INT16)READ16(ea);
					CYCLES(CYCLES_IMUL16_ACC_MEM);		/* TODO: Correct multiply timing */
				}

				dst = (INT32)(INT16)REG16(AX);
				result = src * dst;

				REG16(DX) = (UINT16)(result >> 16);
				REG16(AX) = (UINT16)result;

				I.CF = I.OF = !(result == (INT32)(INT16)result);
			}
			break;
		case 6:			/* DIV AX, Rm16 */
			{
				UINT32 quotient, remainder, result;
				UINT16 src;
				if( modrm >= 0xc0 ) {
					src = LOAD_RM16(modrm);
					CYCLES(CYCLES_DIV16_ACC_REG);
				} else {
					UINT32 ea = GetEA(modrm);
					src = READ16(ea);
					CYCLES(CYCLES_DIV16_ACC_MEM);
				}

				quotient = ((UINT32)(REG16(DX)) << 16) | (UINT32)(REG16(AX));
				if( src ) {
					remainder = quotient % (UINT32)src;
					result = quotient / (UINT32)src;
					if( result > 0xffff ) {
						/* TODO: Divide error */
					} else {
						REG16(DX) = (UINT16)remainder;
						REG16(AX) = (UINT16)result;
					}
				} else {
					/* TODO: Divide by zero */
				}
			}
			break;
		case 7:			/* IDIV AX, Rm16 */
			{
				INT32 quotient, remainder, result;
				UINT16 src;
				if( modrm >= 0xc0 ) {
					src = LOAD_RM16(modrm);
					CYCLES(CYCLES_IDIV16_ACC_REG);
				} else {
					UINT32 ea = GetEA(modrm);
					src = READ16(ea);
					CYCLES(CYCLES_IDIV16_ACC_MEM);
				}

				quotient = (((INT32)REG16(DX)) << 16) | ((UINT32)REG16(AX));
				if( src ) {
					remainder = quotient % (INT32)(INT16)src;
					result = quotient / (INT32)(INT16)src;
					if( result > 0xffff ) {
						/* TODO: Divide error */
					} else {
						REG16(DX) = (UINT16)remainder;
						REG16(AX) = (UINT16)result;
					}
				} else {
					/* TODO: Divide by zero */
				}
			}
			break;
	}
}

static void I386OP(groupFF_16)(void)		// Opcode 0xff
{
	UINT8 modrm = FETCH();

	switch( (modrm >> 3) & 0x7 )
	{
		case 0:			/* INC Rm16 */
			if( modrm >= 0xc0 ) {
				UINT16 dst = LOAD_RM16(modrm);
				dst = INC16(dst);
				STORE_RM16(modrm, dst);
				CYCLES(CYCLES_INC_REG);
			} else {
				UINT32 ea = GetEA(modrm);
				UINT16 dst = READ16(ea);
				dst = INC16(dst);
				WRITE16(ea, dst);
				CYCLES(CYCLES_INC_MEM);
			}
			break;
		case 1:			/* DEC Rm16 */
			if( modrm >= 0xc0 ) {
				UINT16 dst = LOAD_RM16(modrm);
				dst = DEC16(dst);
				STORE_RM16(modrm, dst);
				CYCLES(CYCLES_DEC_REG);
			} else {
				UINT32 ea = GetEA(modrm);
				UINT16 dst = READ16(ea);
				dst = DEC16(dst);
				WRITE16(ea, dst);
				CYCLES(CYCLES_DEC_MEM);
			}
			break;
		case 2:			/* CALL Rm16 */
			{
				UINT16 address;
				if( modrm >= 0xc0 ) {
					address = LOAD_RM16(modrm);
					CYCLES(CYCLES_CALL_REG);		/* TODO: Timing = 7 + m */
				} else {
					UINT32 ea = GetEA(modrm);
					address = READ16(ea);
					CYCLES(CYCLES_CALL_MEM);		/* TODO: Timing = 10 + m */
				}
				PUSH16( I.eip );
				I.eip = address;
				CHANGE_PC(I.eip);
			}
			break;
		case 3:			/* CALL FAR Rm16 */
			{
				UINT16 address, selector;
				if( modrm >= 0xc0 ) {
					fatalerror("NYI");
				} else {
					UINT32 ea = GetEA(modrm);
					address = READ16(ea + 0);
					selector = READ16(ea + 2);
					CYCLES(CYCLES_CALL_MEM_INTERSEG);		/* TODO: Timing = 10 + m */
				}
				PUSH16( I.sreg[CS].selector );
				PUSH16( I.eip );
				I.sreg[CS].selector = selector;
				I.performed_intersegment_jump = 1;
				i386_load_segment_descriptor( CS );
				I.eip = address;
				CHANGE_PC(I.eip);
			}
			break;
		case 4:			/* JMP Rm16 */
			{
				UINT16 address;
				if( modrm >= 0xc0 ) {
					address = LOAD_RM16(modrm);
					CYCLES(CYCLES_JMP_REG);		/* TODO: Timing = 7 + m */
				} else {
					UINT32 ea = GetEA(modrm);
					address = READ16(ea);
					CYCLES(CYCLES_JMP_MEM);		/* TODO: Timing = 10 + m */
				}
				I.eip = address;
				CHANGE_PC(I.eip);
			}
			break;
		case 5:			/* JMP FAR Rm16 */
			{
				UINT16 address, selector;
				if( modrm >= 0xc0 ) {
					fatalerror("NYI");
				} else {
					UINT32 ea = GetEA(modrm);
					address = READ16(ea + 0);
					selector = READ16(ea + 2);
					CYCLES(CYCLES_JMP_MEM_INTERSEG);		/* TODO: Timing = 10 + m */
				}
				I.sreg[CS].selector = selector;
				I.performed_intersegment_jump = 1;
				i386_load_segment_descriptor( CS );
				I.eip = address;
				CHANGE_PC(I.eip);
			}
			break;
		case 6:			/* PUSH Rm16 */
			{
				UINT16 value;
				if( modrm >= 0xc0 ) {
					value = LOAD_RM16(modrm);
				} else {
					UINT32 ea = GetEA(modrm);
					value = READ16(ea);
				}
				PUSH16(value);
				CYCLES(CYCLES_PUSH_RM);
			}
			break;
		case 7:
			I386OP(invalid)();
			break;
		default:
			fatalerror("i386: groupFF_16 /%d unimplemented", (modrm >> 3) & 0x7);
			break;
	}
}

static void I386OP(group0F00_16)(void)			// Opcode 0x0f 00
{
	UINT32 address, ea;
	UINT8 modrm = FETCH();

	switch( (modrm >> 3) & 0x7 )
	{
		case 2:			/* LLDT */
			if ( PROTECTED_MODE && !V8086_MODE )
			{
				if( modrm >= 0xc0 ) {
					address = LOAD_RM16(modrm);
					ea = i386_translate( CS, address );
					CYCLES(CYCLES_LLDT_REG);
				} else {
					ea = GetEA(modrm);
					CYCLES(CYCLES_LLDT_MEM);
				}
				I.ldtr.segment = READ16(ea);
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
					address = LOAD_RM16(modrm);
					ea = i386_translate( CS, address );
					CYCLES(CYCLES_LTR_REG);
				} else {
					ea = GetEA(modrm);
					CYCLES(CYCLES_LTR_MEM);
				}
				I.task.segment = READ16(ea);
			}
			else
			{
				i386_trap(6, 0);
			}
			break;

		default:
			fatalerror("i386: group0F00_16 /%d unimplemented", (modrm >> 3) & 0x7);
			break;
	}
}

static void I386OP(group0F01_16)(void)		// Opcode 0x0f 01
{
	UINT8 modrm = FETCH();
	UINT16 address;
	UINT32 ea;

	switch( (modrm >> 3) & 0x7 )
	{
		case 0:			/* SGDT */
			{
				if( modrm >= 0xc0 ) {
					address = LOAD_RM16(modrm);
					ea = i386_translate( CS, address );
				} else {
					ea = GetEA(modrm);
				}
				WRITE16(ea, I.gdtr.limit);
				WRITE32(ea + 2, I.gdtr.base & 0xffffff);
				CYCLES(CYCLES_SGDT);
				break;
			}
		case 1:			/* SIDT */
			{
				if (modrm >= 0xc0)
				{
					address = LOAD_RM16(modrm);
					ea = i386_translate( CS, address );
				}
				else
				{
					ea = GetEA(modrm);
				}
				WRITE16(ea, I.idtr.limit);
				WRITE32(ea + 2, I.idtr.base & 0xffffff);
				CYCLES(CYCLES_SIDT);
				break;
			}
		case 2:			/* LGDT */
			{
				if( modrm >= 0xc0 ) {
					address = LOAD_RM16(modrm);
					ea = i386_translate( CS, address );
				} else {
					ea = GetEA(modrm);
				}
				I.gdtr.limit = READ16(ea);
				I.gdtr.base = READ32(ea + 2) & 0xffffff;
				CYCLES(CYCLES_LGDT);
				break;
			}
		case 3:			/* LIDT */
			{
				if( modrm >= 0xc0 ) {
					address = LOAD_RM16(modrm);
					ea = i386_translate( CS, address );
				} else {
					ea = GetEA(modrm);
				}
				I.idtr.limit = READ16(ea);
				I.idtr.base = READ32(ea + 2) & 0xffffff;
				CYCLES(CYCLES_LIDT);
				break;
			}
		case 4:			/* SMSW */
			{
				if( modrm >= 0xc0 ) {
					STORE_RM16(modrm, I.cr[0]);
					CYCLES(CYCLES_SMSW_REG);
				} else {
					UINT32 ea = GetEA(modrm);
					WRITE16(ea, I.cr[0]);
					CYCLES(CYCLES_SMSW_MEM);
				}
				break;
			}
		case 6:			/* LMSW */
			{
				// TODO: Check for protection fault
				UINT8 b;
				if( modrm >= 0xc0 ) {
					b = LOAD_RM8(modrm);
					CYCLES(CYCLES_LMSW_REG);
				} else {
					ea = GetEA(modrm);
					CYCLES(CYCLES_LMSW_MEM);
				b = READ8(ea);
				}
				I.cr[0] &= ~0x03;
				I.cr[0] |= b & 0x03;
				break;
			}
		default:
			fatalerror("i386: unimplemented opcode 0x0f 01 /%d at %08X", (modrm >> 3) & 0x7, I.eip - 2);
			break;
	}
}

static void I386OP(group0FBA_16)(void)		// Opcode 0x0f ba
{
	UINT8 modrm = FETCH();

	switch( (modrm >> 3) & 0x7 )
	{
		case 4:			/* BT Rm16, i8 */
			if( modrm >= 0xc0 ) {
				UINT16 dst = LOAD_RM16(modrm);
				UINT8 bit = FETCH();

				if( dst & (1 << bit) )
					I.CF = 1;
				else
					I.CF = 0;

				CYCLES(CYCLES_BT_IMM_REG);
			} else {
				UINT32 ea = GetEA(modrm);
				UINT16 dst = READ16(ea);
				UINT8 bit = FETCH();

				if( dst & (1 << bit) )
					I.CF = 1;
				else
					I.CF = 0;

				CYCLES(CYCLES_BT_IMM_MEM);
			}
			break;
		case 5:			/* BTS Rm16, i8 */
			if( modrm >= 0xc0 ) {
				UINT16 dst = LOAD_RM16(modrm);
				UINT8 bit = FETCH();

				if( dst & (1 << bit) )
					I.CF = 1;
				else
					I.CF = 0;
				dst |= (1 << bit);

				STORE_RM16(modrm, dst);
				CYCLES(CYCLES_BTS_IMM_REG);
			} else {
				UINT32 ea = GetEA(modrm);
				UINT16 dst = READ16(ea);
				UINT8 bit = FETCH();

				if( dst & (1 << bit) )
					I.CF = 1;
				else
					I.CF = 0;
				dst |= (1 << bit);

				WRITE16(ea, dst);
				CYCLES(CYCLES_BTS_IMM_MEM);
			}
			break;
		case 6:			/* BTR Rm16, i8 */
			if( modrm >= 0xc0 ) {
				UINT16 dst = LOAD_RM16(modrm);
				UINT8 bit = FETCH();

				if( dst & (1 << bit) )
					I.CF = 1;
				else
					I.CF = 0;
				dst &= ~(1 << bit);

				STORE_RM16(modrm, dst);
				CYCLES(CYCLES_BTR_IMM_REG);
			} else {
				UINT32 ea = GetEA(modrm);
				UINT16 dst = READ16(ea);
				UINT8 bit = FETCH();

				if( dst & (1 << bit) )
					I.CF = 1;
				else
					I.CF = 0;
				dst &= ~(1 << bit);

				WRITE16(ea, dst);
				CYCLES(CYCLES_BTR_IMM_MEM);
			}
			break;
		case 7:			/* BTC Rm16, i8 */
			if( modrm >= 0xc0 ) {
				UINT16 dst = LOAD_RM16(modrm);
				UINT8 bit = FETCH();

				if( dst & (1 << bit) )
					I.CF = 1;
				else
					I.CF = 0;
				dst ^= (1 << bit);

				STORE_RM16(modrm, dst);
				CYCLES(CYCLES_BTC_IMM_REG);
			} else {
				UINT32 ea = GetEA(modrm);
				UINT16 dst = READ16(ea);
				UINT8 bit = FETCH();

				if( dst & (1 << bit) )
					I.CF = 1;
				else
					I.CF = 0;
				dst ^= (1 << bit);

				WRITE16(ea, dst);
				CYCLES(CYCLES_BTC_IMM_MEM);
			}
			break;
		default:
			fatalerror("i386: group0FBA_16 /%d unknown", (modrm >> 3) & 0x7);
			break;
	}
}

static void I386OP(bound_r16_m16_m16)(void)	// Opcode 0x62
{
	UINT8 modrm;
	INT16 val, low, high;

	modrm = FETCH();

	if (modrm >= 0xc0)
	{
		low = high = LOAD_RM16(modrm);
	}
	else
	{
		UINT32 ea = GetEA(modrm);
		low = READ16(ea + 0);
		high = READ16(ea + 2);
	}
	val = LOAD_REG16(modrm);

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

static void I386OP(retf16)(void)			// Opcode 0xcb
{
	I.eip = POP16();
	I.sreg[CS].selector = POP16();
	i386_load_segment_descriptor( CS );
	CHANGE_PC(I.eip);

	CYCLES(CYCLES_RET_INTERSEG);
}

static void I386OP(retf_i16)(void)			// Opcode 0xca
{
	UINT16 count = FETCH16();

	I.eip = POP16();
	I.sreg[CS].selector = POP16();
	i386_load_segment_descriptor( CS );
	CHANGE_PC(I.eip);

	REG16(SP) += count;
	CYCLES(CYCLES_RET_IMM_INTERSEG);
}

static void I386OP(xlat16)(void)			// Opcode 0xd7
{
	UINT32 ea;
	if( I.segment_prefix ) {
		ea = i386_translate( I.segment_override, REG16(BX) + REG8(AL) );
	} else {
		ea = i386_translate( DS, REG16(BX) + REG8(AL) );
	}
	REG8(AL) = READ8(ea);
	CYCLES(CYCLES_XLAT);
}

static void I386OP(load_far_pointer16)(int s)
{
	UINT8 modrm = FETCH();

	if( modrm >= 0xc0 ) {
		fatalerror("NYI");
	} else {
		UINT32 ea = GetEA(modrm);
		STORE_REG16(modrm, READ16(ea + 0));
		I.sreg[s].selector = READ16(ea + 2);
		i386_load_segment_descriptor( s );
	}
}

static void I386OP(lds16)(void)				// Opcode 0xc5
{
	I386OP(load_far_pointer16)(DS);
	CYCLES(CYCLES_LDS);
}

static void I386OP(lss16)(void)				// Opcode 0x0f 0xb2
{
	I386OP(load_far_pointer16)(SS);
	CYCLES(CYCLES_LSS);
}

static void I386OP(les16)(void)				// Opcode 0xc4
{
	I386OP(load_far_pointer16)(ES);
	CYCLES(CYCLES_LES);
}

static void I386OP(lfs16)(void)				// Opcode 0x0f 0xb4
{
	I386OP(load_far_pointer16)(FS);
	CYCLES(CYCLES_LFS);
}

static void I386OP(lgs16)(void)				// Opcode 0x0f 0xb5
{
	I386OP(load_far_pointer16)(GS);
	CYCLES(CYCLES_LGS);
}

