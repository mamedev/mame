static UINT16 I386OP(shift_rotate16)(i386_state *cpustate, UINT8 modrm, UINT32 value, UINT8 shift)
{
	UINT16 src = value;
	UINT16 dst = value;

	if( shift == 0 ) {
		CYCLES_RM(cpustate,modrm, 3, 7);
	} else if( shift == 1 ) {

		switch( (modrm >> 3) & 0x7 )
		{
			case 0:         /* ROL rm16, 1 */
				cpustate->CF = (src & 0x8000) ? 1 : 0;
				dst = (src << 1) + cpustate->CF;
				cpustate->OF = ((src ^ dst) & 0x8000) ? 1 : 0;
				CYCLES_RM(cpustate,modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 1:         /* ROR rm16, 1 */
				cpustate->CF = (src & 0x1) ? 1 : 0;
				dst = (cpustate->CF << 15) | (src >> 1);
				cpustate->OF = ((src ^ dst) & 0x8000) ? 1 : 0;
				CYCLES_RM(cpustate,modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 2:         /* RCL rm16, 1 */
				dst = (src << 1) + cpustate->CF;
				cpustate->CF = (src & 0x8000) ? 1 : 0;
				cpustate->OF = ((src ^ dst) & 0x8000) ? 1 : 0;
				CYCLES_RM(cpustate,modrm, CYCLES_ROTATE_CARRY_REG, CYCLES_ROTATE_CARRY_MEM);
				break;
			case 3:         /* RCR rm16, 1 */
				dst = (cpustate->CF << 15) | (src >> 1);
				cpustate->CF = src & 0x1;
				cpustate->OF = ((src ^ dst) & 0x8000) ? 1 : 0;
				CYCLES_RM(cpustate,modrm, CYCLES_ROTATE_CARRY_REG, CYCLES_ROTATE_CARRY_MEM);
				break;
			case 4:         /* SHL/SAL rm16, 1 */
			case 6:
				dst = src << 1;
				cpustate->CF = (src & 0x8000) ? 1 : 0;
				cpustate->OF = (((cpustate->CF << 15) ^ dst) & 0x8000) ? 1 : 0;
				SetSZPF16(dst);
				CYCLES_RM(cpustate,modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 5:         /* SHR rm16, 1 */
				dst = src >> 1;
				cpustate->CF = src & 0x1;
				cpustate->OF = (dst & 0x8000) ? 1 : 0;
				SetSZPF16(dst);
				CYCLES_RM(cpustate,modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 7:         /* SAR rm16, 1 */
				dst = (INT16)(src) >> 1;
				cpustate->CF = src & 0x1;
				cpustate->OF = 0;
				SetSZPF16(dst);
				CYCLES_RM(cpustate,modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
		}
	} else {

		switch( (modrm >> 3) & 0x7 )
		{
			case 0:         /* ROL rm16, i8 */
				if(!(shift & 15))
				{
					if(shift & 16)
					{
						cpustate->CF = src & 1;
						cpustate->OF = (src & 1) ^ ((src >> 15) & 1);
					}
					break;
				}
				shift &= 15;
				dst = ((src & ((UINT16)0xffff >> shift)) << shift) |
						((src & ((UINT16)0xffff << (16-shift))) >> (16-shift));
				cpustate->CF = dst & 0x1;
				cpustate->OF = (dst & 1) ^ (dst >> 15);
				CYCLES_RM(cpustate,modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 1:         /* ROR rm16, i8 */
				if(!(shift & 15))
				{
					if(shift & 16)
					{
						cpustate->CF = (src >> 15) & 1;
						cpustate->OF = ((src >> 15) & 1) ^ ((src >> 14) & 1);
					}
					break;
				}
				shift &= 15;
				dst = ((src & ((UINT16)0xffff << shift)) >> shift) |
						((src & ((UINT16)0xffff >> (16-shift))) << (16-shift));
				cpustate->CF = (dst >> 15) & 1;
				cpustate->OF = ((dst >> 15) ^ (dst >> 14)) & 1;
				CYCLES_RM(cpustate,modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 2:         /* RCL rm16, i8 */
				shift %= 17;
				dst = ((src & ((UINT16)0xffff >> shift)) << shift) |
						((src & ((UINT16)0xffff << (17-shift))) >> (17-shift)) |
						(cpustate->CF << (shift-1));
				if(shift) cpustate->CF = (src >> (16-shift)) & 0x1;
				cpustate->OF = cpustate->CF ^ ((dst >> 15) & 1);
				CYCLES_RM(cpustate,modrm, CYCLES_ROTATE_CARRY_REG, CYCLES_ROTATE_CARRY_MEM);
				break;
			case 3:         /* RCR rm16, i8 */
				shift %= 17;
				dst = ((src & ((UINT16)0xffff << shift)) >> shift) |
						((src & ((UINT16)0xffff >> (16-shift))) << (17-shift)) |
						(cpustate->CF << (16-shift));
				if(shift) cpustate->CF = (src >> (shift-1)) & 0x1;
				cpustate->OF = ((dst >> 15) ^ (dst >> 14)) & 1;
				CYCLES_RM(cpustate,modrm, CYCLES_ROTATE_CARRY_REG, CYCLES_ROTATE_CARRY_MEM);
				break;
			case 4:         /* SHL/SAL rm16, i8 */
			case 6:
				shift &= 31;
				dst = src << shift;
				cpustate->CF = (src & (1 << (16-shift))) ? 1 : 0;
				SetSZPF16(dst);
				CYCLES_RM(cpustate,modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 5:         /* SHR rm16, i8 */
				shift &= 31;
				dst = src >> shift;
				cpustate->CF = (src & (1 << (shift-1))) ? 1 : 0;
				SetSZPF16(dst);
				CYCLES_RM(cpustate,modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 7:         /* SAR rm16, i8 */
				shift &= 31;
				dst = (INT16)src >> shift;
				cpustate->CF = (src & (1 << (shift-1))) ? 1 : 0;
				SetSZPF16(dst);
				CYCLES_RM(cpustate,modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
		}

	}
	return dst;
}



static void I386OP(adc_rm16_r16)(i386_state *cpustate)      // Opcode 0x11
{
	UINT16 src, dst;
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_REG16(modrm);
		dst = LOAD_RM16(modrm);
		dst = ADC16(cpustate, dst, src, cpustate->CF);
		STORE_RM16(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		src = LOAD_REG16(modrm);
		dst = READ16(cpustate,ea);
		dst = ADC16(cpustate, dst, src, cpustate->CF);
		WRITE16(cpustate,ea, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_MEM);
	}
}

static void I386OP(adc_r16_rm16)(i386_state *cpustate)      // Opcode 0x13
{
	UINT16 src, dst;
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_RM16(modrm);
		dst = LOAD_REG16(modrm);
		dst = ADC16(cpustate, dst, src, cpustate->CF);
		STORE_REG16(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,0);
		src = READ16(cpustate,ea);
		dst = LOAD_REG16(modrm);
		dst = ADC16(cpustate, dst, src, cpustate->CF);
		STORE_REG16(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_MEM_REG);
	}
}

static void I386OP(adc_ax_i16)(i386_state *cpustate)        // Opcode 0x15
{
	UINT16 src, dst;
	src = FETCH16(cpustate);
	dst = REG16(AX);
	dst = ADC16(cpustate, dst, src, cpustate->CF);
	REG16(AX) = dst;
	CYCLES(cpustate,CYCLES_ALU_IMM_ACC);
}

static void I386OP(add_rm16_r16)(i386_state *cpustate)      // Opcode 0x01
{
	UINT16 src, dst;
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_REG16(modrm);
		dst = LOAD_RM16(modrm);
		dst = ADD16(cpustate,dst, src);
		STORE_RM16(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		src = LOAD_REG16(modrm);
		dst = READ16(cpustate,ea);
		dst = ADD16(cpustate,dst, src);
		WRITE16(cpustate,ea, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_MEM);
	}
}

static void I386OP(add_r16_rm16)(i386_state *cpustate)      // Opcode 0x03
{
	UINT16 src, dst;
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_RM16(modrm);
		dst = LOAD_REG16(modrm);
		dst = ADD16(cpustate,dst, src);
		STORE_REG16(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,0);
		src = READ16(cpustate,ea);
		dst = LOAD_REG16(modrm);
		dst = ADD16(cpustate,dst, src);
		STORE_REG16(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_MEM_REG);
	}
}

static void I386OP(add_ax_i16)(i386_state *cpustate)        // Opcode 0x05
{
	UINT16 src, dst;
	src = FETCH16(cpustate);
	dst = REG16(AX);
	dst = ADD16(cpustate,dst, src);
	REG16(AX) = dst;
	CYCLES(cpustate,CYCLES_ALU_IMM_ACC);
}

static void I386OP(and_rm16_r16)(i386_state *cpustate)      // Opcode 0x21
{
	UINT16 src, dst;
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_REG16(modrm);
		dst = LOAD_RM16(modrm);
		dst = AND16(cpustate,dst, src);
		STORE_RM16(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		src = LOAD_REG16(modrm);
		dst = READ16(cpustate,ea);
		dst = AND16(cpustate,dst, src);
		WRITE16(cpustate,ea, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_MEM);
	}
}

static void I386OP(and_r16_rm16)(i386_state *cpustate)      // Opcode 0x23
{
	UINT16 src, dst;
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_RM16(modrm);
		dst = LOAD_REG16(modrm);
		dst = AND16(cpustate,dst, src);
		STORE_REG16(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,0);
		src = READ16(cpustate,ea);
		dst = LOAD_REG16(modrm);
		dst = AND16(cpustate,dst, src);
		STORE_REG16(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_MEM_REG);
	}
}

static void I386OP(and_ax_i16)(i386_state *cpustate)        // Opcode 0x25
{
	UINT16 src, dst;
	src = FETCH16(cpustate);
	dst = REG16(AX);
	dst = AND16(cpustate,dst, src);
	REG16(AX) = dst;
	CYCLES(cpustate,CYCLES_ALU_IMM_ACC);
}

static void I386OP(bsf_r16_rm16)(i386_state *cpustate)      // Opcode 0x0f bc
{
	UINT16 src, dst, temp;
	UINT8 modrm = FETCH(cpustate);

	if( modrm >= 0xc0 ) {
		src = LOAD_RM16(modrm);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,0);
		src = READ16(cpustate,ea);
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
		STORE_REG16(modrm, dst);
	}
	CYCLES(cpustate,CYCLES_BSF_BASE);
}

static void I386OP(bsr_r16_rm16)(i386_state *cpustate)      // Opcode 0x0f bd
{
	UINT16 src, dst, temp;
	UINT8 modrm = FETCH(cpustate);

	if( modrm >= 0xc0 ) {
		src = LOAD_RM16(modrm);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,0);
		src = READ16(cpustate,ea);
	}

	dst = 0;

	if( src == 0 ) {
		cpustate->ZF = 1;
	} else {
		cpustate->ZF = 0;
		dst = temp = 15;
		while( (src & (1 << temp)) == 0 ) {
			temp--;
			dst = temp;
			CYCLES(cpustate,CYCLES_BSR);
		}
		STORE_REG16(modrm, dst);
	}
	CYCLES(cpustate,CYCLES_BSR_BASE);
}


static void I386OP(bt_rm16_r16)(i386_state *cpustate)       // Opcode 0x0f a3
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		UINT16 dst = LOAD_RM16(modrm);
		UINT16 bit = LOAD_REG16(modrm);

		if( dst & (1 << (bit & 0xf)) )
			cpustate->CF = 1;
		else
			cpustate->CF = 0;

		CYCLES(cpustate,CYCLES_BT_REG_REG);
	} else {
		UINT8 segment;
		UINT32 ea = GetNonTranslatedEA(cpustate,modrm,&segment);
		UINT16 bit = LOAD_REG16(modrm);
		ea += 2*(bit/16);
		ea = i386_translate(cpustate,segment,(cpustate->address_size)?ea:(ea&0xffff),0);
		bit %= 16;
		UINT16 dst = READ16(cpustate,ea);

		if( dst & (1 << bit) )
			cpustate->CF = 1;
		else
			cpustate->CF = 0;

		CYCLES(cpustate,CYCLES_BT_REG_MEM);
	}
}

static void I386OP(btc_rm16_r16)(i386_state *cpustate)      // Opcode 0x0f bb
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		UINT16 dst = LOAD_RM16(modrm);
		UINT16 bit = LOAD_REG16(modrm);

		if( dst & (1 << (bit & 0xf)) )
			cpustate->CF = 1;
		else
			cpustate->CF = 0;
		dst ^= (1 << (bit & 0xf));

		STORE_RM16(modrm, dst);
		CYCLES(cpustate,CYCLES_BTC_REG_REG);
	} else {
		UINT8 segment;
		UINT32 ea = GetNonTranslatedEA(cpustate,modrm,&segment);
		UINT16 bit = LOAD_REG16(modrm);
		ea += 2*(bit/16);
		ea = i386_translate(cpustate,segment,(cpustate->address_size)?ea:(ea&0xffff),1);
		bit %= 16;
		UINT16 dst = READ16(cpustate,ea);

		if( dst & (1 << bit) )
			cpustate->CF = 1;
		else
			cpustate->CF = 0;
		dst ^= (1 << bit);

		WRITE16(cpustate,ea, dst);
		CYCLES(cpustate,CYCLES_BTC_REG_MEM);
	}
}

static void I386OP(btr_rm16_r16)(i386_state *cpustate)      // Opcode 0x0f b3
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		UINT16 dst = LOAD_RM16(modrm);
		UINT16 bit = LOAD_REG16(modrm);

		if( dst & (1 << (bit & 0xf)) )
			cpustate->CF = 1;
		else
			cpustate->CF = 0;
		dst &= ~(1 << (bit & 0xf));

		STORE_RM16(modrm, dst);
		CYCLES(cpustate,CYCLES_BTR_REG_REG);
	} else {
		UINT8 segment;
		UINT32 ea = GetNonTranslatedEA(cpustate,modrm,&segment);
		UINT16 bit = LOAD_REG16(modrm);
		ea += 2*(bit/16);
		ea = i386_translate(cpustate,segment,(cpustate->address_size)?ea:(ea&0xffff),1);
		bit %= 16;
		UINT16 dst = READ16(cpustate,ea);

		if( dst & (1 << bit) )
			cpustate->CF = 1;
		else
			cpustate->CF = 0;
		dst &= ~(1 << bit);

		WRITE16(cpustate,ea, dst);
		CYCLES(cpustate,CYCLES_BTR_REG_MEM);
	}
}

static void I386OP(bts_rm16_r16)(i386_state *cpustate)      // Opcode 0x0f ab
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		UINT16 dst = LOAD_RM16(modrm);
		UINT16 bit = LOAD_REG16(modrm);

		if( dst & (1 << (bit & 0xf)) )
			cpustate->CF = 1;
		else
			cpustate->CF = 0;
		dst |= (1 << (bit & 0xf));

		STORE_RM16(modrm, dst);
		CYCLES(cpustate,CYCLES_BTS_REG_REG);
	} else {
		UINT8 segment;
		UINT32 ea = GetNonTranslatedEA(cpustate,modrm,&segment);
		UINT16 bit = LOAD_REG16(modrm);
		ea += 2*(bit/16);
		ea = i386_translate(cpustate,segment,(cpustate->address_size)?ea:(ea&0xffff),1);
		bit %= 16;
		UINT16 dst = READ16(cpustate,ea);

		if( dst & (1 << bit) )
			cpustate->CF = 1;
		else
			cpustate->CF = 0;
		dst |= (1 << bit);

		WRITE16(cpustate,ea, dst);
		CYCLES(cpustate,CYCLES_BTS_REG_MEM);
	}
}

static void I386OP(call_abs16)(i386_state *cpustate)        // Opcode 0x9a
{
	UINT16 offset = FETCH16(cpustate);
	UINT16 ptr = FETCH16(cpustate);

	if( PROTECTED_MODE && !V8086_MODE)
	{
		i386_protected_mode_call(cpustate,ptr,offset,0,0);
	}
	else
	{
		PUSH16(cpustate, cpustate->sreg[CS].selector );
		PUSH16(cpustate, cpustate->eip );
		cpustate->sreg[CS].selector = ptr;
		cpustate->performed_intersegment_jump = 1;
		cpustate->eip = offset;
		i386_load_segment_descriptor(cpustate,CS);
	}
	CYCLES(cpustate,CYCLES_CALL_INTERSEG);      /* TODO: Timing = 17 + m */
	CHANGE_PC(cpustate,cpustate->eip);
}

static void I386OP(call_rel16)(i386_state *cpustate)        // Opcode 0xe8
{
	INT16 disp = FETCH16(cpustate);

	PUSH16(cpustate, cpustate->eip );
	if (cpustate->sreg[CS].d)
	{
		cpustate->eip += disp;
	}
	else
	{
		cpustate->eip = (cpustate->eip + disp) & 0xffff;
	}
	CHANGE_PC(cpustate,cpustate->eip);
	CYCLES(cpustate,CYCLES_CALL);       /* TODO: Timing = 7 + m */
}

static void I386OP(cbw)(i386_state *cpustate)               // Opcode 0x98
{
	REG16(AX) = (INT16)((INT8)REG8(AL));
	CYCLES(cpustate,CYCLES_CBW);
}

static void I386OP(cmp_rm16_r16)(i386_state *cpustate)      // Opcode 0x39
{
	UINT16 src, dst;
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_REG16(modrm);
		dst = LOAD_RM16(modrm);
		SUB16(cpustate,dst, src);
		CYCLES(cpustate,CYCLES_CMP_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,0);
		src = LOAD_REG16(modrm);
		dst = READ16(cpustate,ea);
		SUB16(cpustate,dst, src);
		CYCLES(cpustate,CYCLES_CMP_REG_MEM);
	}
}

static void I386OP(cmp_r16_rm16)(i386_state *cpustate)      // Opcode 0x3b
{
	UINT16 src, dst;
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_RM16(modrm);
		dst = LOAD_REG16(modrm);
		SUB16(cpustate,dst, src);
		CYCLES(cpustate,CYCLES_CMP_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,0);
		src = READ16(cpustate,ea);
		dst = LOAD_REG16(modrm);
		SUB16(cpustate,dst, src);
		CYCLES(cpustate,CYCLES_CMP_MEM_REG);
	}
}

static void I386OP(cmp_ax_i16)(i386_state *cpustate)        // Opcode 0x3d
{
	UINT16 src, dst;
	src = FETCH16(cpustate);
	dst = REG16(AX);
	SUB16(cpustate,dst, src);
	CYCLES(cpustate,CYCLES_CMP_IMM_ACC);
}

static void I386OP(cmpsw)(i386_state *cpustate)             // Opcode 0xa7
{
	UINT32 eas, ead;
	UINT16 src, dst;
	if( cpustate->segment_prefix ) {
		eas = i386_translate(cpustate, cpustate->segment_override, cpustate->address_size ? REG32(ESI) : REG16(SI), 0 );
	} else {
		eas = i386_translate(cpustate, DS, cpustate->address_size ? REG32(ESI) : REG16(SI), 0 );
	}
	ead = i386_translate(cpustate, ES, cpustate->address_size ? REG32(EDI) : REG16(DI), 0 );
	src = READ16(cpustate,eas);
	dst = READ16(cpustate,ead);
	SUB16(cpustate,src,dst);
	BUMP_SI(cpustate,2);
	BUMP_DI(cpustate,2);
	CYCLES(cpustate,CYCLES_CMPS);
}

static void I386OP(cwd)(i386_state *cpustate)               // Opcode 0x99
{
	if( REG16(AX) & 0x8000 ) {
		REG16(DX) = 0xffff;
	} else {
		REG16(DX) = 0x0000;
	}
	CYCLES(cpustate,CYCLES_CWD);
}

static void I386OP(dec_ax)(i386_state *cpustate)            // Opcode 0x48
{
	REG16(AX) = DEC16(cpustate, REG16(AX) );
	CYCLES(cpustate,CYCLES_DEC_REG);
}

static void I386OP(dec_cx)(i386_state *cpustate)            // Opcode 0x49
{
	REG16(CX) = DEC16(cpustate, REG16(CX) );
	CYCLES(cpustate,CYCLES_DEC_REG);
}

static void I386OP(dec_dx)(i386_state *cpustate)            // Opcode 0x4a
{
	REG16(DX) = DEC16(cpustate, REG16(DX) );
	CYCLES(cpustate,CYCLES_DEC_REG);
}

static void I386OP(dec_bx)(i386_state *cpustate)            // Opcode 0x4b
{
	REG16(BX) = DEC16(cpustate, REG16(BX) );
	CYCLES(cpustate,CYCLES_DEC_REG);
}

static void I386OP(dec_sp)(i386_state *cpustate)            // Opcode 0x4c
{
	REG16(SP) = DEC16(cpustate, REG16(SP) );
	CYCLES(cpustate,CYCLES_DEC_REG);
}

static void I386OP(dec_bp)(i386_state *cpustate)            // Opcode 0x4d
{
	REG16(BP) = DEC16(cpustate, REG16(BP) );
	CYCLES(cpustate,CYCLES_DEC_REG);
}

static void I386OP(dec_si)(i386_state *cpustate)            // Opcode 0x4e
{
	REG16(SI) = DEC16(cpustate, REG16(SI) );
	CYCLES(cpustate,CYCLES_DEC_REG);
}

static void I386OP(dec_di)(i386_state *cpustate)            // Opcode 0x4f
{
	REG16(DI) = DEC16(cpustate, REG16(DI) );
	CYCLES(cpustate,CYCLES_DEC_REG);
}

static void I386OP(imul_r16_rm16)(i386_state *cpustate)     // Opcode 0x0f af
{
	UINT8 modrm = FETCH(cpustate);
	INT32 result;
	INT32 src, dst;
	if( modrm >= 0xc0 ) {
		src = (INT32)(INT16)LOAD_RM16(modrm);
		CYCLES(cpustate,CYCLES_IMUL16_REG_REG);     /* TODO: Correct multiply timing */
	} else {
		UINT32 ea = GetEA(cpustate,modrm,0);
		src = (INT32)(INT16)READ16(cpustate,ea);
		CYCLES(cpustate,CYCLES_IMUL16_REG_MEM);     /* TODO: Correct multiply timing */
	}

	dst = (INT32)(INT16)LOAD_REG16(modrm);
	result = src * dst;

	STORE_REG16(modrm, (UINT16)result);

	cpustate->CF = cpustate->OF = !(result == (INT32)(INT16)result);
}

static void I386OP(imul_r16_rm16_i16)(i386_state *cpustate) // Opcode 0x69
{
	UINT8 modrm = FETCH(cpustate);
	INT32 result;
	INT32 src, dst;
	if( modrm >= 0xc0 ) {
		dst = (INT32)(INT16)LOAD_RM16(modrm);
		CYCLES(cpustate,CYCLES_IMUL16_REG_IMM_REG);     /* TODO: Correct multiply timing */
	} else {
		UINT32 ea = GetEA(cpustate,modrm,0);
		dst = (INT32)(INT16)READ16(cpustate,ea);
		CYCLES(cpustate,CYCLES_IMUL16_MEM_IMM_REG);     /* TODO: Correct multiply timing */
	}

	src = (INT32)(INT16)FETCH16(cpustate);
	result = src * dst;

	STORE_REG16(modrm, (UINT16)result);

	cpustate->CF = cpustate->OF = !(result == (INT32)(INT16)result);
}

static void I386OP(imul_r16_rm16_i8)(i386_state *cpustate)  // Opcode 0x6b
{
	UINT8 modrm = FETCH(cpustate);
	INT32 result;
	INT32 src, dst;
	if( modrm >= 0xc0 ) {
		dst = (INT32)(INT16)LOAD_RM16(modrm);
		CYCLES(cpustate,CYCLES_IMUL16_REG_IMM_REG);     /* TODO: Correct multiply timing */
	} else {
		UINT32 ea = GetEA(cpustate,modrm,0);
		dst = (INT32)(INT16)READ16(cpustate,ea);
		CYCLES(cpustate,CYCLES_IMUL16_MEM_IMM_REG);     /* TODO: Correct multiply timing */
	}

	src = (INT32)(INT8)FETCH(cpustate);
	result = src * dst;

	STORE_REG16(modrm, (UINT16)result);

	cpustate->CF = cpustate->OF = !(result == (INT32)(INT16)result);
}

static void I386OP(in_ax_i8)(i386_state *cpustate)          // Opcode 0xe5
{
	UINT16 port = FETCH(cpustate);
	UINT16 data = READPORT16(cpustate, port);
	REG16(AX) = data;
	CYCLES(cpustate,CYCLES_IN_VAR);
}

static void I386OP(in_ax_dx)(i386_state *cpustate)          // Opcode 0xed
{
	UINT16 port = REG16(DX);
	UINT16 data = READPORT16(cpustate, port);
	REG16(AX) = data;
	CYCLES(cpustate,CYCLES_IN);
}

static void I386OP(inc_ax)(i386_state *cpustate)            // Opcode 0x40
{
	REG16(AX) = INC16(cpustate, REG16(AX) );
	CYCLES(cpustate,CYCLES_INC_REG);
}

static void I386OP(inc_cx)(i386_state *cpustate)            // Opcode 0x41
{
	REG16(CX) = INC16(cpustate, REG16(CX) );
	CYCLES(cpustate,CYCLES_INC_REG);
}

static void I386OP(inc_dx)(i386_state *cpustate)            // Opcode 0x42
{
	REG16(DX) = INC16(cpustate, REG16(DX) );
	CYCLES(cpustate,CYCLES_INC_REG);
}

static void I386OP(inc_bx)(i386_state *cpustate)            // Opcode 0x43
{
	REG16(BX) = INC16(cpustate, REG16(BX) );
	CYCLES(cpustate,CYCLES_INC_REG);
}

static void I386OP(inc_sp)(i386_state *cpustate)            // Opcode 0x44
{
	REG16(SP) = INC16(cpustate, REG16(SP) );
	CYCLES(cpustate,CYCLES_INC_REG);
}

static void I386OP(inc_bp)(i386_state *cpustate)            // Opcode 0x45
{
	REG16(BP) = INC16(cpustate, REG16(BP) );
	CYCLES(cpustate,CYCLES_INC_REG);
}

static void I386OP(inc_si)(i386_state *cpustate)            // Opcode 0x46
{
	REG16(SI) = INC16(cpustate, REG16(SI) );
	CYCLES(cpustate,CYCLES_INC_REG);
}

static void I386OP(inc_di)(i386_state *cpustate)            // Opcode 0x47
{
	REG16(DI) = INC16(cpustate, REG16(DI) );
	CYCLES(cpustate,CYCLES_INC_REG);
}

static void I386OP(iret16)(i386_state *cpustate)            // Opcode 0xcf
{
	if( PROTECTED_MODE )
	{
		i386_protected_mode_iret(cpustate,0);
	}
	else
	{
		/* TODO: #SS(0) exception */
		/* TODO: #GP(0) exception */
		cpustate->eip = POP16(cpustate);
		cpustate->sreg[CS].selector = POP16(cpustate);
		set_flags(cpustate, POP16(cpustate) );
		i386_load_segment_descriptor(cpustate,CS);
		CHANGE_PC(cpustate,cpustate->eip);
	}
	CYCLES(cpustate,CYCLES_IRET);
}

static void I386OP(ja_rel16)(i386_state *cpustate)          // Opcode 0x0f 87
{
	INT16 disp = FETCH16(cpustate);
	if( cpustate->CF == 0 && cpustate->ZF == 0 ) {
		if (cpustate->sreg[CS].d)
		{
			cpustate->eip += disp;
		}
		else
		{
			cpustate->eip = (cpustate->eip + disp) & 0xffff;
		}
		CHANGE_PC(cpustate,cpustate->eip);
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jbe_rel16)(i386_state *cpustate)         // Opcode 0x0f 86
{
	INT16 disp = FETCH16(cpustate);
	if( cpustate->CF != 0 || cpustate->ZF != 0 ) {
		if (cpustate->sreg[CS].d)
		{
			cpustate->eip += disp;
		}
		else
		{
			cpustate->eip = (cpustate->eip + disp) & 0xffff;
		}
		CHANGE_PC(cpustate,cpustate->eip);
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jc_rel16)(i386_state *cpustate)          // Opcode 0x0f 82
{
	INT16 disp = FETCH16(cpustate);
	if( cpustate->CF != 0 ) {
		if (cpustate->sreg[CS].d)
		{
			cpustate->eip += disp;
		}
		else
		{
			cpustate->eip = (cpustate->eip + disp) & 0xffff;
		}
		CHANGE_PC(cpustate,cpustate->eip);
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jg_rel16)(i386_state *cpustate)          // Opcode 0x0f 8f
{
	INT16 disp = FETCH16(cpustate);
	if( cpustate->ZF == 0 && (cpustate->SF == cpustate->OF) ) {
		if (cpustate->sreg[CS].d)
		{
			cpustate->eip += disp;
		}
		else
		{
			cpustate->eip = (cpustate->eip + disp) & 0xffff;
		}
		CHANGE_PC(cpustate,cpustate->eip);
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jge_rel16)(i386_state *cpustate)         // Opcode 0x0f 8d
{
	INT16 disp = FETCH16(cpustate);
	if(cpustate->SF == cpustate->OF) {
		if (cpustate->sreg[CS].d)
		{
			cpustate->eip += disp;
		}
		else
		{
			cpustate->eip = (cpustate->eip + disp) & 0xffff;
		}
		CHANGE_PC(cpustate,cpustate->eip);
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jl_rel16)(i386_state *cpustate)          // Opcode 0x0f 8c
{
	INT16 disp = FETCH16(cpustate);
	if( (cpustate->SF != cpustate->OF) ) {
		if (cpustate->sreg[CS].d)
		{
			cpustate->eip += disp;
		}
		else
		{
			cpustate->eip = (cpustate->eip + disp) & 0xffff;
		}
		CHANGE_PC(cpustate,cpustate->eip);
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jle_rel16)(i386_state *cpustate)         // Opcode 0x0f 8e
{
	INT16 disp = FETCH16(cpustate);
	if( cpustate->ZF != 0 || (cpustate->SF != cpustate->OF) ) {
		if (cpustate->sreg[CS].d)
		{
			cpustate->eip += disp;
		}
		else
		{
			cpustate->eip = (cpustate->eip + disp) & 0xffff;
		}
		CHANGE_PC(cpustate,cpustate->eip);
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jnc_rel16)(i386_state *cpustate)         // Opcode 0x0f 83
{
	INT16 disp = FETCH16(cpustate);
	if( cpustate->CF == 0 ) {
		if (cpustate->sreg[CS].d)
		{
			cpustate->eip += disp;
		}
		else
		{
			cpustate->eip = (cpustate->eip + disp) & 0xffff;
		}
		CHANGE_PC(cpustate,cpustate->eip);
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jno_rel16)(i386_state *cpustate)         // Opcode 0x0f 81
{
	INT16 disp = FETCH16(cpustate);
	if( cpustate->OF == 0 ) {
		if (cpustate->sreg[CS].d)
		{
			cpustate->eip += disp;
		}
		else
		{
			cpustate->eip = (cpustate->eip + disp) & 0xffff;
		}
		CHANGE_PC(cpustate,cpustate->eip);
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jnp_rel16)(i386_state *cpustate)         // Opcode 0x0f 8b
{
	INT16 disp = FETCH16(cpustate);
	if( cpustate->PF == 0 ) {
		if (cpustate->sreg[CS].d)
		{
			cpustate->eip += disp;
		}
		else
		{
			cpustate->eip = (cpustate->eip + disp) & 0xffff;
		}
		CHANGE_PC(cpustate,cpustate->eip);
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jns_rel16)(i386_state *cpustate)         // Opcode 0x0f 89
{
	INT16 disp = FETCH16(cpustate);
	if( cpustate->SF == 0 ) {
		if (cpustate->sreg[CS].d)
		{
			cpustate->eip += disp;
		}
		else
		{
			cpustate->eip = (cpustate->eip + disp) & 0xffff;
		}
		CHANGE_PC(cpustate,cpustate->eip);
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jnz_rel16)(i386_state *cpustate)         // Opcode 0x0f 85
{
	INT16 disp = FETCH16(cpustate);
	if( cpustate->ZF == 0 ) {
		if (cpustate->sreg[CS].d)
		{
			cpustate->eip += disp;
		}
		else
		{
			cpustate->eip = (cpustate->eip + disp) & 0xffff;
		}
		CHANGE_PC(cpustate,cpustate->eip);
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jo_rel16)(i386_state *cpustate)          // Opcode 0x0f 80
{
	INT16 disp = FETCH16(cpustate);
	if( cpustate->OF != 0 ) {
		if (cpustate->sreg[CS].d)
		{
			cpustate->eip += disp;
		}
		else
		{
			cpustate->eip = (cpustate->eip + disp) & 0xffff;
		}
		CHANGE_PC(cpustate,cpustate->eip);
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jp_rel16)(i386_state *cpustate)          // Opcode 0x0f 8a
{
	INT16 disp = FETCH16(cpustate);
	if( cpustate->PF != 0 ) {
		if (cpustate->sreg[CS].d)
		{
			cpustate->eip += disp;
		}
		else
		{
			cpustate->eip = (cpustate->eip + disp) & 0xffff;
		}
		CHANGE_PC(cpustate,cpustate->eip);
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(js_rel16)(i386_state *cpustate)          // Opcode 0x0f 88
{
	INT16 disp = FETCH16(cpustate);
	if( cpustate->SF != 0 ) {
		if (cpustate->sreg[CS].d)
		{
			cpustate->eip += disp;
		}
		else
		{
			cpustate->eip = (cpustate->eip + disp) & 0xffff;
		}
		CHANGE_PC(cpustate,cpustate->eip);
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jz_rel16)(i386_state *cpustate)          // Opcode 0x0f 84
{
	INT16 disp = FETCH16(cpustate);
	if( cpustate->ZF != 0 ) {
		if (cpustate->sreg[CS].d)
		{
			cpustate->eip += disp;
		}
		else
		{
			cpustate->eip = (cpustate->eip + disp) & 0xffff;
		}
		CHANGE_PC(cpustate,cpustate->eip);
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jcxz16)(i386_state *cpustate)            // Opcode 0xe3
{
	INT8 disp = FETCH(cpustate);
	int val = (cpustate->address_size)?(REG32(ECX) == 0):(REG16(CX) == 0);
	if( val ) {
		if (cpustate->sreg[CS].d)
		{
			cpustate->eip += disp;
		}
		else
		{
			cpustate->eip = (cpustate->eip + disp) & 0xffff;
		}
		CHANGE_PC(cpustate,cpustate->eip);
		CYCLES(cpustate,CYCLES_JCXZ);       /* TODO: Timing = 9 + m */
	} else {
		CYCLES(cpustate,CYCLES_JCXZ_NOBRANCH);
	}
}

static void I386OP(jmp_rel16)(i386_state *cpustate)         // Opcode 0xe9
{
	INT16 disp = FETCH16(cpustate);

	if (cpustate->sreg[CS].d)
	{
		cpustate->eip += disp;
	}
	else
	{
		cpustate->eip = (cpustate->eip + disp) & 0xffff;
	}
	CHANGE_PC(cpustate,cpustate->eip);
	CYCLES(cpustate,CYCLES_JMP);        /* TODO: Timing = 7 + m */
}

static void I386OP(jmp_abs16)(i386_state *cpustate)         // Opcode 0xea
{
	UINT16 address = FETCH16(cpustate);
	UINT16 segment = FETCH16(cpustate);

	if( PROTECTED_MODE && !V8086_MODE)
	{
		i386_protected_mode_jump(cpustate,segment,address,0,0);
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

static void I386OP(lea16)(i386_state *cpustate)             // Opcode 0x8d
{
	UINT8 modrm = FETCH(cpustate);
	UINT32 ea = GetNonTranslatedEA(cpustate,modrm,NULL);
	STORE_REG16(modrm, ea);
	CYCLES(cpustate,CYCLES_LEA);
}

static void I386OP(enter16)(i386_state *cpustate)           // Opcode 0xc8
{
	UINT16 framesize = FETCH16(cpustate);
	UINT8 level = FETCH(cpustate) % 32;
	UINT8 x;
	UINT16 frameptr;
	PUSH16(cpustate,REG16(BP));

	if(!STACK_32BIT)
		frameptr = REG16(SP);
	else
		frameptr = REG32(ESP);

	if(level > 0)
	{
		for(x=1;x<level-1;x++)
		{
			REG16(BP) -= 2;
			PUSH16(cpustate,READ16(cpustate,REG16(BP)));
		}
		PUSH16(cpustate,frameptr);
	}
	REG16(BP) = frameptr;
	if(!STACK_32BIT)
		REG16(SP) -= framesize;
	else
		REG32(ESP) -= framesize;
	CYCLES(cpustate,CYCLES_ENTER);
}

static void I386OP(leave16)(i386_state *cpustate)           // Opcode 0xc9
{
	if(!STACK_32BIT)
		REG16(SP) = REG16(BP);
	else
		REG32(ESP) = REG32(EBP);
	REG16(BP) = POP16(cpustate);
	CYCLES(cpustate,CYCLES_LEAVE);
}

static void I386OP(lodsw)(i386_state *cpustate)             // Opcode 0xad
{
	UINT32 eas;
	if( cpustate->segment_prefix ) {
		eas = i386_translate(cpustate, cpustate->segment_override, cpustate->address_size ? REG32(ESI) : REG16(SI), 0 );
	} else {
		eas = i386_translate(cpustate, DS, cpustate->address_size ? REG32(ESI) : REG16(SI), 0 );
	}
	REG16(AX) = READ16(cpustate,eas);
	BUMP_SI(cpustate,2);
	CYCLES(cpustate,CYCLES_LODS);
}

static void I386OP(loop16)(i386_state *cpustate)            // Opcode 0xe2
{
	INT8 disp = FETCH(cpustate);
	INT32 val = (cpustate->address_size)?(--REG32(ECX)):(--REG16(CX));
	if( val != 0 ) {
		if (cpustate->sreg[CS].d)
		{
			cpustate->eip += disp;
		}
		else
		{
			cpustate->eip = (cpustate->eip + disp) & 0xffff;
		}
		CHANGE_PC(cpustate,cpustate->eip);
	}
	CYCLES(cpustate,CYCLES_LOOP);       /* TODO: Timing = 11 + m */
}

static void I386OP(loopne16)(i386_state *cpustate)          // Opcode 0xe0
{
	INT8 disp = FETCH(cpustate);
	INT32 val = (cpustate->address_size)?(--REG32(ECX)):(--REG16(CX));
	if( val != 0 && cpustate->ZF == 0 ) {
		if (cpustate->sreg[CS].d)
		{
			cpustate->eip += disp;
		}
		else
		{
			cpustate->eip = (cpustate->eip + disp) & 0xffff;
		}
		CHANGE_PC(cpustate,cpustate->eip);
	}
	CYCLES(cpustate,CYCLES_LOOPNZ);     /* TODO: Timing = 11 + m */
}

static void I386OP(loopz16)(i386_state *cpustate)           // Opcode 0xe1
{
	INT8 disp = FETCH(cpustate);
	INT32 val = (cpustate->address_size)?(--REG32(ECX)):(--REG16(CX));
	if( val != 0 && cpustate->ZF != 0 ) {
		if (cpustate->sreg[CS].d)
		{
			cpustate->eip += disp;
		}
		else
		{
			cpustate->eip = (cpustate->eip + disp) & 0xffff;
		}
		CHANGE_PC(cpustate,cpustate->eip);
	}
	CYCLES(cpustate,CYCLES_LOOPZ);      /* TODO: Timing = 11 + m */
}

static void I386OP(mov_rm16_r16)(i386_state *cpustate)      // Opcode 0x89
{
	UINT16 src;
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_REG16(modrm);
		STORE_RM16(modrm, src);
		CYCLES(cpustate,CYCLES_MOV_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		src = LOAD_REG16(modrm);
		WRITE16(cpustate,ea, src);
		CYCLES(cpustate,CYCLES_MOV_REG_MEM);
	}
}

static void I386OP(mov_r16_rm16)(i386_state *cpustate)      // Opcode 0x8b
{
	UINT16 src;
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_RM16(modrm);
		STORE_REG16(modrm, src);
		CYCLES(cpustate,CYCLES_MOV_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,0);
		src = READ16(cpustate,ea);
		STORE_REG16(modrm, src);
		CYCLES(cpustate,CYCLES_MOV_MEM_REG);
	}
}

static void I386OP(mov_rm16_i16)(i386_state *cpustate)      // Opcode 0xc7
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		UINT16 value = FETCH16(cpustate);
		STORE_RM16(modrm, value);
		CYCLES(cpustate,CYCLES_MOV_IMM_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		UINT16 value = FETCH16(cpustate);
		WRITE16(cpustate,ea, value);
		CYCLES(cpustate,CYCLES_MOV_IMM_MEM);
	}
}

static void I386OP(mov_ax_m16)(i386_state *cpustate)        // Opcode 0xa1
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
	REG16(AX) = READ16(cpustate,ea);
	CYCLES(cpustate,CYCLES_MOV_MEM_ACC);
}

static void I386OP(mov_m16_ax)(i386_state *cpustate)        // Opcode 0xa3
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
	WRITE16(cpustate, ea, REG16(AX) );
	CYCLES(cpustate,CYCLES_MOV_ACC_MEM);
}

static void I386OP(mov_ax_i16)(i386_state *cpustate)        // Opcode 0xb8
{
	REG16(AX) = FETCH16(cpustate);
	CYCLES(cpustate,CYCLES_MOV_IMM_REG);
}

static void I386OP(mov_cx_i16)(i386_state *cpustate)        // Opcode 0xb9
{
	REG16(CX) = FETCH16(cpustate);
	CYCLES(cpustate,CYCLES_MOV_IMM_REG);
}

static void I386OP(mov_dx_i16)(i386_state *cpustate)        // Opcode 0xba
{
	REG16(DX) = FETCH16(cpustate);
	CYCLES(cpustate,CYCLES_MOV_IMM_REG);
}

static void I386OP(mov_bx_i16)(i386_state *cpustate)        // Opcode 0xbb
{
	REG16(BX) = FETCH16(cpustate);
	CYCLES(cpustate,CYCLES_MOV_IMM_REG);
}

static void I386OP(mov_sp_i16)(i386_state *cpustate)        // Opcode 0xbc
{
	REG16(SP) = FETCH16(cpustate);
	CYCLES(cpustate,CYCLES_MOV_IMM_REG);
}

static void I386OP(mov_bp_i16)(i386_state *cpustate)        // Opcode 0xbd
{
	REG16(BP) = FETCH16(cpustate);
	CYCLES(cpustate,CYCLES_MOV_IMM_REG);
}

static void I386OP(mov_si_i16)(i386_state *cpustate)        // Opcode 0xbe
{
	REG16(SI) = FETCH16(cpustate);
	CYCLES(cpustate,CYCLES_MOV_IMM_REG);
}

static void I386OP(mov_di_i16)(i386_state *cpustate)        // Opcode 0xbf
{
	REG16(DI) = FETCH16(cpustate);
	CYCLES(cpustate,CYCLES_MOV_IMM_REG);
}

static void I386OP(movsw)(i386_state *cpustate)             // Opcode 0xa5
{
	UINT32 eas, ead;
	UINT16 v;
	if( cpustate->segment_prefix ) {
		eas = i386_translate(cpustate, cpustate->segment_override, cpustate->address_size ? REG32(ESI) : REG16(SI), 0 );
	} else {
		eas = i386_translate(cpustate, DS, cpustate->address_size ? REG32(ESI) : REG16(SI), 0 );
	}
	ead = i386_translate(cpustate, ES, cpustate->address_size ? REG32(EDI) : REG16(DI), 1 );
	v = READ16(cpustate,eas);
	WRITE16(cpustate,ead, v);
	BUMP_SI(cpustate,2);
	BUMP_DI(cpustate,2);
	CYCLES(cpustate,CYCLES_MOVS);
}

static void I386OP(movsx_r16_rm8)(i386_state *cpustate)     // Opcode 0x0f be
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		INT16 src = (INT8)LOAD_RM8(modrm);
		STORE_REG16(modrm, src);
		CYCLES(cpustate,CYCLES_MOVSX_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,0);
		INT16 src = (INT8)READ8(cpustate,ea);
		STORE_REG16(modrm, src);
		CYCLES(cpustate,CYCLES_MOVSX_MEM_REG);
	}
}

static void I386OP(movzx_r16_rm8)(i386_state *cpustate)     // Opcode 0x0f b6
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		UINT16 src = (UINT8)LOAD_RM8(modrm);
		STORE_REG16(modrm, src);
		CYCLES(cpustate,CYCLES_MOVZX_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,0);
		UINT16 src = (UINT8)READ8(cpustate,ea);
		STORE_REG16(modrm, src);
		CYCLES(cpustate,CYCLES_MOVZX_MEM_REG);
	}
}

static void I386OP(or_rm16_r16)(i386_state *cpustate)       // Opcode 0x09
{
	UINT16 src, dst;
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_REG16(modrm);
		dst = LOAD_RM16(modrm);
		dst = OR16(cpustate,dst, src);
		STORE_RM16(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		src = LOAD_REG16(modrm);
		dst = READ16(cpustate,ea);
		dst = OR16(cpustate,dst, src);
		WRITE16(cpustate,ea, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_MEM);
	}
}

static void I386OP(or_r16_rm16)(i386_state *cpustate)       // Opcode 0x0b
{
	UINT16 src, dst;
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_RM16(modrm);
		dst = LOAD_REG16(modrm);
		dst = OR16(cpustate,dst, src);
		STORE_REG16(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,0);
		src = READ16(cpustate,ea);
		dst = LOAD_REG16(modrm);
		dst = OR16(cpustate,dst, src);
		STORE_REG16(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_MEM_REG);
	}
}

static void I386OP(or_ax_i16)(i386_state *cpustate)         // Opcode 0x0d
{
	UINT16 src, dst;
	src = FETCH16(cpustate);
	dst = REG16(AX);
	dst = OR16(cpustate,dst, src);
	REG16(AX) = dst;
	CYCLES(cpustate,CYCLES_ALU_IMM_ACC);
}

static void I386OP(out_ax_i8)(i386_state *cpustate)         // Opcode 0xe7
{
	UINT16 port = FETCH(cpustate);
	UINT16 data = REG16(AX);
	WRITEPORT16(cpustate, port, data);
	CYCLES(cpustate,CYCLES_OUT_VAR);
}

static void I386OP(out_ax_dx)(i386_state *cpustate)         // Opcode 0xef
{
	UINT16 port = REG16(DX);
	UINT16 data = REG16(AX);
	WRITEPORT16(cpustate, port, data);
	CYCLES(cpustate,CYCLES_OUT);
}

static void I386OP(pop_ax)(i386_state *cpustate)            // Opcode 0x58
{
	UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(cpustate,SS,offset+1) == 0)
		REG16(AX) = POP16(cpustate);
	else
		FAULT(FAULT_SS,0)
	CYCLES(cpustate,CYCLES_POP_REG_SHORT);
}

static void I386OP(pop_cx)(i386_state *cpustate)            // Opcode 0x59
{
	UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(cpustate,SS,offset+1) == 0)
		REG16(CX) = POP16(cpustate);
	else
		FAULT(FAULT_SS,0)
	CYCLES(cpustate,CYCLES_POP_REG_SHORT);
}

static void I386OP(pop_dx)(i386_state *cpustate)            // Opcode 0x5a
{
	UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(cpustate,SS,offset+1) == 0)
		REG16(DX) = POP16(cpustate);
	else
		FAULT(FAULT_SS,0)
	CYCLES(cpustate,CYCLES_POP_REG_SHORT);
}

static void I386OP(pop_bx)(i386_state *cpustate)            // Opcode 0x5b
{
	UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(cpustate,SS,offset+1) == 0)
		REG16(BX) = POP16(cpustate);
	else
		FAULT(FAULT_SS,0)
	CYCLES(cpustate,CYCLES_POP_REG_SHORT);
}

static void I386OP(pop_sp)(i386_state *cpustate)            // Opcode 0x5c
{
	UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(cpustate,SS,offset+1) == 0)
		REG16(SP) = POP16(cpustate);
	else
		FAULT(FAULT_SS,0)
	CYCLES(cpustate,CYCLES_POP_REG_SHORT);
}

static void I386OP(pop_bp)(i386_state *cpustate)            // Opcode 0x5d
{
	UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(cpustate,SS,offset+1) == 0)
		REG16(BP) = POP16(cpustate);
	else
		FAULT(FAULT_SS,0)
	CYCLES(cpustate,CYCLES_POP_REG_SHORT);
}

static void I386OP(pop_si)(i386_state *cpustate)            // Opcode 0x5e
{
	UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(cpustate,SS,offset+1) == 0)
		REG16(SI) = POP16(cpustate);
	else
		FAULT(FAULT_SS,0)
	CYCLES(cpustate,CYCLES_POP_REG_SHORT);
}

static void I386OP(pop_di)(i386_state *cpustate)            // Opcode 0x5f
{
	UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(cpustate,SS,offset+1) == 0)
		REG16(DI) = POP16(cpustate);
	else
		FAULT(FAULT_SS,0)
	CYCLES(cpustate,CYCLES_POP_REG_SHORT);
}

static bool I386OP(pop_seg16)(i386_state *cpustate, int segment)
{
	UINT32 ea, offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	UINT16 value;
	bool fault;
	if(i386_limit_check(cpustate,SS,offset+1) == 0)
	{
		ea = i386_translate(cpustate, SS, offset, 0);
		value = READ16(cpustate, ea);
		i386_sreg_load(cpustate,value, segment, &fault);
		if(fault) return false;
		if(STACK_32BIT)
			REG32(ESP) = offset + 2;
		else
			REG16(SP) = offset + 2;
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

static void I386OP(pop_ds16)(i386_state *cpustate)          // Opcode 0x1f
{
	I386OP(pop_seg16)(cpustate, DS);
}

static void I386OP(pop_es16)(i386_state *cpustate)          // Opcode 0x07
{
	I386OP(pop_seg16)(cpustate, ES);
}

static void I386OP(pop_fs16)(i386_state *cpustate)          // Opcode 0x0f a1
{
	I386OP(pop_seg16)(cpustate, FS);
}

static void I386OP(pop_gs16)(i386_state *cpustate)          // Opcode 0x0f a9
{
	I386OP(pop_seg16)(cpustate, GS);
}

static void I386OP(pop_ss16)(i386_state *cpustate)          // Opcode 0x17
{
	if(!I386OP(pop_seg16)(cpustate, SS)) return;
	if(cpustate->IF != 0) // if external interrupts are enabled
	{
		cpustate->IF = 0;  // reset IF for the next instruction
		cpustate->delayed_interrupt_enable = 1;
	}
}

static void I386OP(pop_rm16)(i386_state *cpustate)          // Opcode 0x8f
{
	UINT8 modrm = FETCH(cpustate);
	UINT16 value;
	UINT32 ea, offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));

	if(i386_limit_check(cpustate,SS,offset+1) == 0)
	{
		UINT32 temp_sp = REG32(ESP);
		value = POP16(cpustate);

		if( modrm >= 0xc0 ) {
			STORE_RM16(modrm, value);
		} else {
			ea = GetEA(cpustate,modrm,1);
			try
			{
				WRITE16(cpustate,ea, value);
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

static void I386OP(popa)(i386_state *cpustate)              // Opcode 0x61
{
	UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));

	if(i386_limit_check(cpustate,SS,offset+15) == 0)
	{
		REG16(DI) = POP16(cpustate);
		REG16(SI) = POP16(cpustate);
		REG16(BP) = POP16(cpustate);
		REG16(SP) += 2;
		REG16(BX) = POP16(cpustate);
		REG16(DX) = POP16(cpustate);
		REG16(CX) = POP16(cpustate);
		REG16(AX) = POP16(cpustate);
	}
	else
		FAULT(FAULT_SS,0)
	CYCLES(cpustate,CYCLES_POPA);
}

static void I386OP(popf)(i386_state *cpustate)              // Opcode 0x9d
{
	UINT32 value;
	UINT32 current = get_flags(cpustate);
	UINT8 IOPL = (current >> 12) & 0x03;
	UINT32 mask = 0x7fd5;
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

	if(i386_limit_check(cpustate,SS,offset+1) == 0)
	{
		value = POP16(cpustate);
		set_flags(cpustate,(current & ~mask) | (value & mask));  // mask out reserved bits
	}
	else
		FAULT(FAULT_SS,0)
	CYCLES(cpustate,CYCLES_POPF);
}

static void I386OP(push_ax)(i386_state *cpustate)           // Opcode 0x50
{
	UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(cpustate,SS,offset-2) == 0)
		PUSH16(cpustate, REG16(AX) );
	else
		FAULT(FAULT_SS,0)
	CYCLES(cpustate,CYCLES_PUSH_REG_SHORT);
}

static void I386OP(push_cx)(i386_state *cpustate)           // Opcode 0x51
{
	UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(cpustate,SS,offset-2) == 0)
		PUSH16(cpustate, REG16(CX) );
	else
		FAULT(FAULT_SS,0)
	CYCLES(cpustate,CYCLES_PUSH_REG_SHORT);
}

static void I386OP(push_dx)(i386_state *cpustate)           // Opcode 0x52
{
	UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(cpustate,SS,offset-2) == 0)
		PUSH16(cpustate, REG16(DX) );
	else
		FAULT(FAULT_SS,0)
	CYCLES(cpustate,CYCLES_PUSH_REG_SHORT);
}

static void I386OP(push_bx)(i386_state *cpustate)           // Opcode 0x53
{
	UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(cpustate,SS,offset-2) == 0)
		PUSH16(cpustate, REG16(BX) );
	else
		FAULT(FAULT_SS,0)
	CYCLES(cpustate,CYCLES_PUSH_REG_SHORT);
}

static void I386OP(push_sp)(i386_state *cpustate)           // Opcode 0x54
{
	UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(cpustate,SS,offset-2) == 0)
		PUSH16(cpustate, REG16(SP) );
	else
		FAULT(FAULT_SS,0)
	CYCLES(cpustate,CYCLES_PUSH_REG_SHORT);
}

static void I386OP(push_bp)(i386_state *cpustate)           // Opcode 0x55
{
	UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(cpustate,SS,offset-2) == 0)
		PUSH16(cpustate, REG16(BP) );
	else
		FAULT(FAULT_SS,0)
	CYCLES(cpustate,CYCLES_PUSH_REG_SHORT);
}

static void I386OP(push_si)(i386_state *cpustate)           // Opcode 0x56
{
	UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(cpustate,SS,offset-2) == 0)
		PUSH16(cpustate, REG16(SI) );
	else
		FAULT(FAULT_SS,0)
	CYCLES(cpustate,CYCLES_PUSH_REG_SHORT);
}

static void I386OP(push_di)(i386_state *cpustate)           // Opcode 0x57
{
	UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(cpustate,SS,offset-2) == 0)
		PUSH16(cpustate, REG16(DI) );
	else
		FAULT(FAULT_SS,0)
	CYCLES(cpustate,CYCLES_PUSH_REG_SHORT);
}

static void I386OP(push_cs16)(i386_state *cpustate)         // Opcode 0x0e
{
	UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(cpustate,SS,offset-2) == 0)
		PUSH16(cpustate, cpustate->sreg[CS].selector );
	else
		FAULT(FAULT_SS,0)
	CYCLES(cpustate,CYCLES_PUSH_SREG);
}

static void I386OP(push_ds16)(i386_state *cpustate)         // Opcode 0x1e
{
	UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(cpustate,SS,offset-2) == 0)
		PUSH16(cpustate, cpustate->sreg[DS].selector );
	else
		FAULT(FAULT_SS,0)
	CYCLES(cpustate,CYCLES_PUSH_SREG);
}

static void I386OP(push_es16)(i386_state *cpustate)         // Opcode 0x06
{
	UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(cpustate,SS,offset-2) == 0)
		PUSH16(cpustate, cpustate->sreg[ES].selector );
	else
		FAULT(FAULT_SS,0)
	CYCLES(cpustate,CYCLES_PUSH_SREG);
}

static void I386OP(push_fs16)(i386_state *cpustate)         // Opcode 0x0f a0
{
	UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(cpustate,SS,offset-2) == 0)
		PUSH16(cpustate, cpustate->sreg[FS].selector );
	else
		FAULT(FAULT_SS,0)
	CYCLES(cpustate,CYCLES_PUSH_SREG);
}

static void I386OP(push_gs16)(i386_state *cpustate)         // Opcode 0x0f a8
{
	UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(cpustate,SS,offset-2) == 0)
		PUSH16(cpustate, cpustate->sreg[GS].selector );
	else
		FAULT(FAULT_SS,0)
	CYCLES(cpustate,CYCLES_PUSH_SREG);
}

static void I386OP(push_ss16)(i386_state *cpustate)         // Opcode 0x16
{
	UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(cpustate,SS,offset-2) == 0)
		PUSH16(cpustate, cpustate->sreg[SS].selector );
	else
		FAULT(FAULT_SS,0)
	CYCLES(cpustate,CYCLES_PUSH_SREG);
}

static void I386OP(push_i16)(i386_state *cpustate)          // Opcode 0x68
{
	UINT16 value = FETCH16(cpustate);
	UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(cpustate,SS,offset-2) == 0)
		PUSH16(cpustate,value);
	else
		FAULT(FAULT_SS,0)
	CYCLES(cpustate,CYCLES_PUSH_IMM);
}

static void I386OP(pusha)(i386_state *cpustate)             // Opcode 0x60
{
	UINT16 temp = REG16(SP);
	UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(cpustate,SS,offset-16) == 0)
	{
		PUSH16(cpustate, REG16(AX) );
		PUSH16(cpustate, REG16(CX) );
		PUSH16(cpustate, REG16(DX) );
		PUSH16(cpustate, REG16(BX) );
		PUSH16(cpustate, temp );
		PUSH16(cpustate, REG16(BP) );
		PUSH16(cpustate, REG16(SI) );
		PUSH16(cpustate, REG16(DI) );
	}
	else
		FAULT(FAULT_SS,0)
	CYCLES(cpustate,CYCLES_PUSHA);
}

static void I386OP(pushf)(i386_state *cpustate)             // Opcode 0x9c
{
	UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(cpustate,SS,offset-2) == 0)
		PUSH16(cpustate, get_flags(cpustate) & 0xffff );
	else
		FAULT(FAULT_SS,0)
	CYCLES(cpustate,CYCLES_PUSHF);
}

static void I386OP(ret_near16_i16)(i386_state *cpustate)    // Opcode 0xc2
{
	INT16 disp = FETCH16(cpustate);
	cpustate->eip = POP16(cpustate);
	REG16(SP) += disp;
	CHANGE_PC(cpustate,cpustate->eip);
	CYCLES(cpustate,CYCLES_RET_IMM);        /* TODO: Timing = 10 + m */
}

static void I386OP(ret_near16)(i386_state *cpustate)        // Opcode 0xc3
{
	cpustate->eip = POP16(cpustate);
	CHANGE_PC(cpustate,cpustate->eip);
	CYCLES(cpustate,CYCLES_RET);        /* TODO: Timing = 10 + m */
}

static void I386OP(sbb_rm16_r16)(i386_state *cpustate)      // Opcode 0x19
{
	UINT16 src, dst;
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_REG16(modrm);
		dst = LOAD_RM16(modrm);
		dst = SBB16(cpustate, dst, src, cpustate->CF);
		STORE_RM16(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		src = LOAD_REG16(modrm);
		dst = READ16(cpustate,ea);
		dst = SBB16(cpustate, dst, src, cpustate->CF);
		WRITE16(cpustate,ea, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_MEM);
	}
}

static void I386OP(sbb_r16_rm16)(i386_state *cpustate)      // Opcode 0x1b
{
	UINT16 src, dst;
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_RM16(modrm);
		dst = LOAD_REG16(modrm);
		dst = SBB16(cpustate, dst, src, cpustate->CF);
		STORE_REG16(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,0);
		src = READ16(cpustate,ea);
		dst = LOAD_REG16(modrm);
		dst = SBB16(cpustate, dst, src, cpustate->CF);
		STORE_REG16(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_MEM_REG);
	}
}

static void I386OP(sbb_ax_i16)(i386_state *cpustate)        // Opcode 0x1d
{
	UINT16 src, dst;
	src = FETCH16(cpustate);
	dst = REG16(AX);
	dst = SBB16(cpustate, dst, src, cpustate->CF);
	REG16(AX) = dst;
	CYCLES(cpustate,CYCLES_ALU_IMM_ACC);
}

static void I386OP(scasw)(i386_state *cpustate)             // Opcode 0xaf
{
	UINT32 eas;
	UINT16 src, dst;
	eas = i386_translate(cpustate, ES, cpustate->address_size ? REG32(EDI) : REG16(DI), 0 );
	src = READ16(cpustate,eas);
	dst = REG16(AX);
	SUB16(cpustate,dst, src);
	BUMP_DI(cpustate,2);
	CYCLES(cpustate,CYCLES_SCAS);
}

static void I386OP(shld16_i8)(i386_state *cpustate)         // Opcode 0x0f a4
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		UINT16 dst = LOAD_RM16(modrm);
		UINT16 upper = LOAD_REG16(modrm);
		UINT8 shift = FETCH(cpustate);
		shift &= 31;
		if( shift == 0 ) {

		} else if( shift > 15 ) {
			cpustate->CF = (upper & (1 << (16-shift))) ? 1 : 0;
			// ppro and above should be (dst >> (32-shift))
			dst = (upper << (shift-16)) | (upper >> (32-shift));
			cpustate->OF = cpustate->CF ^ (dst >> 15);
			SetSZPF16(dst);
		} else {
			cpustate->CF = (dst & (1 << (16-shift))) ? 1 : 0;
			dst = (dst << shift) | (upper >> (16-shift));
			cpustate->OF = cpustate->CF ^ (dst >> 15);
			SetSZPF16(dst);
		}
		STORE_RM16(modrm, dst);
		CYCLES(cpustate,CYCLES_SHLD_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		UINT16 dst = READ16(cpustate,ea);
		UINT16 upper = LOAD_REG16(modrm);
		UINT8 shift = FETCH(cpustate);
		shift &= 31;
		if( shift == 0 ) {

		} else if( shift > 15 ) {
			cpustate->CF = (upper & (1 << (16-shift))) ? 1 : 0;
			dst = (upper << (shift-16)) | (upper >> (32-shift));
			cpustate->OF = cpustate->CF ^ (dst >> 15);
			SetSZPF16(dst);
		} else {
			cpustate->CF = (dst & (1 << (16-shift))) ? 1 : 0;
			dst = (dst << shift) | (upper >> (16-shift));
			cpustate->OF = cpustate->CF ^ (dst >> 15);
			SetSZPF16(dst);
		}
		WRITE16(cpustate,ea, dst);
		CYCLES(cpustate,CYCLES_SHLD_MEM);
	}
}

static void I386OP(shld16_cl)(i386_state *cpustate)         // Opcode 0x0f a5
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		UINT16 dst = LOAD_RM16(modrm);
		UINT16 upper = LOAD_REG16(modrm);
		UINT8 shift = REG8(CL);
		shift &= 31;
		if( shift == 0 ) {

		} else if( shift > 15 ) {
			cpustate->CF = (upper & (1 << (16-shift))) ? 1 : 0;
			dst = (upper << (shift-16)) | (upper >> (32-shift));
			cpustate->OF = cpustate->CF ^ (dst >> 15);
			SetSZPF16(dst);
		} else {
			cpustate->CF = (dst & (1 << (16-shift))) ? 1 : 0;
			dst = (dst << shift) | (upper >> (16-shift));
			cpustate->OF = cpustate->CF ^ (dst >> 15);
			SetSZPF16(dst);
		}
		STORE_RM16(modrm, dst);
		CYCLES(cpustate,CYCLES_SHLD_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		UINT16 dst = READ16(cpustate,ea);
		UINT16 upper = LOAD_REG16(modrm);
		UINT8 shift = REG8(CL);
		shift &= 31;
		if( shift == 0 ) {

		} else if( shift > 15 ) {
			cpustate->CF = (upper & (1 << (16-shift))) ? 1 : 0;
			dst = (upper << (shift-16)) | (upper >> (32-shift));
			cpustate->OF = cpustate->CF ^ (dst >> 15);
			SetSZPF16(dst);
		} else {
			cpustate->CF = (dst & (1 << (16-shift))) ? 1 : 0;
			dst = (dst << shift) | (upper >> (16-shift));
			cpustate->OF = cpustate->CF ^ (dst >> 15);
			SetSZPF16(dst);
		}
		WRITE16(cpustate,ea, dst);
		CYCLES(cpustate,CYCLES_SHLD_MEM);
	}
}

static void I386OP(shrd16_i8)(i386_state *cpustate)         // Opcode 0x0f ac
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		UINT16 dst = LOAD_RM16(modrm);
		UINT16 upper = LOAD_REG16(modrm);
		UINT8 shift = FETCH(cpustate);
		shift &= 31;
		if( shift == 0) {

		} else if( shift > 15 ) {
			cpustate->CF = (upper & (1 << (shift-1))) ? 1 : 0;
			dst = (upper >> (shift-16)) | (upper << (32-shift));
			cpustate->OF = ((dst >> 15) ^ (dst >> 14)) & 1;
			SetSZPF16(dst);
		} else {
			cpustate->CF = (dst & (1 << (shift-1))) ? 1 : 0;
			dst = (dst >> shift) | (upper << (16-shift));
			cpustate->OF = ((dst >> 15) ^ (dst >> 14)) & 1;
			SetSZPF16(dst);
		}
		STORE_RM16(modrm, dst);
		CYCLES(cpustate,CYCLES_SHRD_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		UINT16 dst = READ16(cpustate,ea);
		UINT16 upper = LOAD_REG16(modrm);
		UINT8 shift = FETCH(cpustate);
		shift &= 31;
		if( shift == 0) {

		} else if( shift > 15 ) {
			cpustate->CF = (upper & (1 << (shift-1))) ? 1 : 0;
			dst = (upper >> (shift-16)) | (upper << (32-shift));
			cpustate->OF = ((dst >> 15) ^ (dst >> 14)) & 1;
			SetSZPF16(dst);
		} else {
			cpustate->CF = (dst & (1 << (shift-1))) ? 1 : 0;
			dst = (dst >> shift) | (upper << (16-shift));
			cpustate->OF = ((dst >> 15) ^ (dst >> 14)) & 1;
			SetSZPF16(dst);
		}
		WRITE16(cpustate,ea, dst);
		CYCLES(cpustate,CYCLES_SHRD_MEM);
	}
}

static void I386OP(shrd16_cl)(i386_state *cpustate)         // Opcode 0x0f ad
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		UINT16 dst = LOAD_RM16(modrm);
		UINT16 upper = LOAD_REG16(modrm);
		UINT8 shift = REG8(CL);
		shift &= 31;
		if( shift == 0) {

		} else if( shift > 15 ) {
			cpustate->CF = (upper & (1 << (shift-1))) ? 1 : 0;
			dst = (upper >> (shift-16)) | (upper << (32-shift));
			cpustate->OF = ((dst >> 15) ^ (dst >> 14)) & 1;
			SetSZPF16(dst);
		} else {
			cpustate->CF = (dst & (1 << (shift-1))) ? 1 : 0;
			dst = (dst >> shift) | (upper << (16-shift));
			cpustate->OF = ((dst >> 15) ^ (dst >> 14)) & 1;
			SetSZPF16(dst);
		}
		STORE_RM16(modrm, dst);
		CYCLES(cpustate,CYCLES_SHRD_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		UINT16 dst = READ16(cpustate,ea);
		UINT16 upper = LOAD_REG16(modrm);
		UINT8 shift = REG8(CL);
		shift &= 31;
		if( shift == 0) {

		} else if( shift > 15 ) {
			cpustate->CF = (upper & (1 << (shift-1))) ? 1 : 0;
			dst = (upper >> (shift-16)) | (upper << (32-shift));
			cpustate->OF = ((dst >> 15) ^ (dst >> 14)) & 1;
			SetSZPF16(dst);
		} else {
			cpustate->CF = (dst & (1 << (shift-1))) ? 1 : 0;
			dst = (dst >> shift) | (upper << (16-shift));
			cpustate->OF = ((dst >> 15) ^ (dst >> 14)) & 1;
			SetSZPF16(dst);
		}
		WRITE16(cpustate,ea, dst);
		CYCLES(cpustate,CYCLES_SHRD_MEM);
	}
}

static void I386OP(stosw)(i386_state *cpustate)             // Opcode 0xab
{
	UINT32 ead;
	ead = i386_translate(cpustate, ES, cpustate->address_size ? REG32(EDI) : REG16(DI), 1 );
	WRITE16(cpustate,ead, REG16(AX));
	BUMP_DI(cpustate,2);
	CYCLES(cpustate,CYCLES_STOS);
}

static void I386OP(sub_rm16_r16)(i386_state *cpustate)      // Opcode 0x29
{
	UINT16 src, dst;
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_REG16(modrm);
		dst = LOAD_RM16(modrm);
		dst = SUB16(cpustate,dst, src);
		STORE_RM16(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		src = LOAD_REG16(modrm);
		dst = READ16(cpustate,ea);
		dst = SUB16(cpustate,dst, src);
		WRITE16(cpustate,ea, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_MEM);
	}
}

static void I386OP(sub_r16_rm16)(i386_state *cpustate)      // Opcode 0x2b
{
	UINT16 src, dst;
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_RM16(modrm);
		dst = LOAD_REG16(modrm);
		dst = SUB16(cpustate,dst, src);
		STORE_REG16(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,0);
		src = READ16(cpustate,ea);
		dst = LOAD_REG16(modrm);
		dst = SUB16(cpustate,dst, src);
		STORE_REG16(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_MEM_REG);
	}
}

static void I386OP(sub_ax_i16)(i386_state *cpustate)        // Opcode 0x2d
{
	UINT16 src, dst;
	src = FETCH16(cpustate);
	dst = REG16(AX);
	dst = SUB16(cpustate,dst, src);
	REG16(AX) = dst;
	CYCLES(cpustate,CYCLES_ALU_IMM_ACC);
}

static void I386OP(test_ax_i16)(i386_state *cpustate)       // Opcode 0xa9
{
	UINT16 src = FETCH16(cpustate);
	UINT16 dst = REG16(AX);
	dst = src & dst;
	SetSZPF16(dst);
	cpustate->CF = 0;
	cpustate->OF = 0;
	CYCLES(cpustate,CYCLES_TEST_IMM_ACC);
}

static void I386OP(test_rm16_r16)(i386_state *cpustate)     // Opcode 0x85
{
	UINT16 src, dst;
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_REG16(modrm);
		dst = LOAD_RM16(modrm);
		dst = src & dst;
		SetSZPF16(dst);
		cpustate->CF = 0;
		cpustate->OF = 0;
		CYCLES(cpustate,CYCLES_TEST_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,0);
		src = LOAD_REG16(modrm);
		dst = READ16(cpustate,ea);
		dst = src & dst;
		SetSZPF16(dst);
		cpustate->CF = 0;
		cpustate->OF = 0;
		CYCLES(cpustate,CYCLES_TEST_REG_MEM);
	}
}

static void I386OP(xchg_ax_cx)(i386_state *cpustate)        // Opcode 0x91
{
	UINT16 temp;
	temp = REG16(AX);
	REG16(AX) = REG16(CX);
	REG16(CX) = temp;
	CYCLES(cpustate,CYCLES_XCHG_REG_REG);
}

static void I386OP(xchg_ax_dx)(i386_state *cpustate)        // Opcode 0x92
{
	UINT16 temp;
	temp = REG16(AX);
	REG16(AX) = REG16(DX);
	REG16(DX) = temp;
	CYCLES(cpustate,CYCLES_XCHG_REG_REG);
}

static void I386OP(xchg_ax_bx)(i386_state *cpustate)        // Opcode 0x93
{
	UINT16 temp;
	temp = REG16(AX);
	REG16(AX) = REG16(BX);
	REG16(BX) = temp;
	CYCLES(cpustate,CYCLES_XCHG_REG_REG);
}

static void I386OP(xchg_ax_sp)(i386_state *cpustate)        // Opcode 0x94
{
	UINT16 temp;
	temp = REG16(AX);
	REG16(AX) = REG16(SP);
	REG16(SP) = temp;
	CYCLES(cpustate,CYCLES_XCHG_REG_REG);
}

static void I386OP(xchg_ax_bp)(i386_state *cpustate)        // Opcode 0x95
{
	UINT16 temp;
	temp = REG16(AX);
	REG16(AX) = REG16(BP);
	REG16(BP) = temp;
	CYCLES(cpustate,CYCLES_XCHG_REG_REG);
}

static void I386OP(xchg_ax_si)(i386_state *cpustate)        // Opcode 0x96
{
	UINT16 temp;
	temp = REG16(AX);
	REG16(AX) = REG16(SI);
	REG16(SI) = temp;
	CYCLES(cpustate,CYCLES_XCHG_REG_REG);
}

static void I386OP(xchg_ax_di)(i386_state *cpustate)        // Opcode 0x97
{
	UINT16 temp;
	temp = REG16(AX);
	REG16(AX) = REG16(DI);
	REG16(DI) = temp;
	CYCLES(cpustate,CYCLES_XCHG_REG_REG);
}

static void I386OP(xchg_r16_rm16)(i386_state *cpustate)     // Opcode 0x87
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		UINT16 src = LOAD_RM16(modrm);
		UINT16 dst = LOAD_REG16(modrm);
		STORE_REG16(modrm, src);
		STORE_RM16(modrm, dst);
		CYCLES(cpustate,CYCLES_XCHG_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		UINT16 src = READ16(cpustate,ea);
		UINT16 dst = LOAD_REG16(modrm);
		STORE_REG16(modrm, src);
		WRITE16(cpustate,ea, dst);
		CYCLES(cpustate,CYCLES_XCHG_REG_MEM);
	}
}

static void I386OP(xor_rm16_r16)(i386_state *cpustate)      // Opcode 0x31
{
	UINT16 src, dst;
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_REG16(modrm);
		dst = LOAD_RM16(modrm);
		dst = XOR16(cpustate,dst, src);
		STORE_RM16(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		src = LOAD_REG16(modrm);
		dst = READ16(cpustate,ea);
		dst = XOR16(cpustate,dst, src);
		WRITE16(cpustate,ea, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_MEM);
	}
}

static void I386OP(xor_r16_rm16)(i386_state *cpustate)      // Opcode 0x33
{
	UINT16 src, dst;
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_RM16(modrm);
		dst = LOAD_REG16(modrm);
		dst = XOR16(cpustate,dst, src);
		STORE_REG16(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,0);
		src = READ16(cpustate,ea);
		dst = LOAD_REG16(modrm);
		dst = XOR16(cpustate,dst, src);
		STORE_REG16(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_MEM_REG);
	}
}

static void I386OP(xor_ax_i16)(i386_state *cpustate)        // Opcode 0x35
{
	UINT16 src, dst;
	src = FETCH16(cpustate);
	dst = REG16(AX);
	dst = XOR16(cpustate,dst, src);
	REG16(AX) = dst;
	CYCLES(cpustate,CYCLES_ALU_IMM_ACC);
}



static void I386OP(group81_16)(i386_state *cpustate)        // Opcode 0x81
{
	UINT32 ea;
	UINT16 src, dst;
	UINT8 modrm = FETCH(cpustate);

	switch( (modrm >> 3) & 0x7 )
	{
		case 0:     // ADD Rm16, i16
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM16(modrm);
				src = FETCH16(cpustate);
				dst = ADD16(cpustate,dst, src);
				STORE_RM16(modrm, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(cpustate,modrm,1);
				dst = READ16(cpustate,ea);
				src = FETCH16(cpustate);
				dst = ADD16(cpustate,dst, src);
				WRITE16(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_MEM);
			}
			break;
		case 1:     // OR Rm16, i16
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM16(modrm);
				src = FETCH16(cpustate);
				dst = OR16(cpustate,dst, src);
				STORE_RM16(modrm, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(cpustate,modrm,1);
				dst = READ16(cpustate,ea);
				src = FETCH16(cpustate);
				dst = OR16(cpustate,dst, src);
				WRITE16(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_MEM);
			}
			break;
		case 2:     // ADC Rm16, i16
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM16(modrm);
				src = FETCH16(cpustate);
				dst = ADC16(cpustate, dst, src, cpustate->CF);
				STORE_RM16(modrm, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(cpustate,modrm,1);
				dst = READ16(cpustate,ea);
				src = FETCH16(cpustate);
				dst = ADC16(cpustate, dst, src, cpustate->CF);
				WRITE16(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_MEM);
			}
			break;
		case 3:     // SBB Rm16, i16
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM16(modrm);
				src = FETCH16(cpustate);
				dst = SBB16(cpustate, dst, src, cpustate->CF);
				STORE_RM16(modrm, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(cpustate,modrm,1);
				dst = READ16(cpustate,ea);
				src = FETCH16(cpustate);
				dst = SBB16(cpustate, dst, src, cpustate->CF);
				WRITE16(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_MEM);
			}
			break;
		case 4:     // AND Rm16, i16
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM16(modrm);
				src = FETCH16(cpustate);
				dst = AND16(cpustate,dst, src);
				STORE_RM16(modrm, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(cpustate,modrm,1);
				dst = READ16(cpustate,ea);
				src = FETCH16(cpustate);
				dst = AND16(cpustate,dst, src);
				WRITE16(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_MEM);
			}
			break;
		case 5:     // SUB Rm16, i16
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM16(modrm);
				src = FETCH16(cpustate);
				dst = SUB16(cpustate,dst, src);
				STORE_RM16(modrm, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(cpustate,modrm,1);
				dst = READ16(cpustate,ea);
				src = FETCH16(cpustate);
				dst = SUB16(cpustate,dst, src);
				WRITE16(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_MEM);
			}
			break;
		case 6:     // XOR Rm16, i16
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM16(modrm);
				src = FETCH16(cpustate);
				dst = XOR16(cpustate,dst, src);
				STORE_RM16(modrm, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(cpustate,modrm,1);
				dst = READ16(cpustate,ea);
				src = FETCH16(cpustate);
				dst = XOR16(cpustate,dst, src);
				WRITE16(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_MEM);
			}
			break;
		case 7:     // CMP Rm16, i16
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM16(modrm);
				src = FETCH16(cpustate);
				SUB16(cpustate,dst, src);
				CYCLES(cpustate,CYCLES_CMP_REG_REG);
			} else {
				ea = GetEA(cpustate,modrm,0);
				dst = READ16(cpustate,ea);
				src = FETCH16(cpustate);
				SUB16(cpustate,dst, src);
				CYCLES(cpustate,CYCLES_CMP_REG_MEM);
			}
			break;
	}
}

static void I386OP(group83_16)(i386_state *cpustate)        // Opcode 0x83
{
	UINT32 ea;
	UINT16 src, dst;
	UINT8 modrm = FETCH(cpustate);

	switch( (modrm >> 3) & 0x7 )
	{
		case 0:     // ADD Rm16, i16
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM16(modrm);
				src = (UINT16)(INT16)(INT8)FETCH(cpustate);
				dst = ADD16(cpustate,dst, src);
				STORE_RM16(modrm, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(cpustate,modrm,1);
				dst = READ16(cpustate,ea);
				src = (UINT16)(INT16)(INT8)FETCH(cpustate);
				dst = ADD16(cpustate,dst, src);
				WRITE16(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_MEM);
			}
			break;
		case 1:     // OR Rm16, i16
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM16(modrm);
				src = (UINT16)(INT16)(INT8)FETCH(cpustate);
				dst = OR16(cpustate,dst, src);
				STORE_RM16(modrm, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(cpustate,modrm,1);
				dst = READ16(cpustate,ea);
				src = (UINT16)(INT16)(INT8)FETCH(cpustate);
				dst = OR16(cpustate,dst, src);
				WRITE16(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_MEM);
			}
			break;
		case 2:     // ADC Rm16, i16
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM16(modrm);
				src = (UINT16)(INT16)(INT8)FETCH(cpustate);
				dst = ADC16(cpustate, dst, src, cpustate->CF);
				STORE_RM16(modrm, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(cpustate,modrm,1);
				dst = READ16(cpustate,ea);
				src = (UINT16)(INT16)(INT8)FETCH(cpustate);
				dst = ADC16(cpustate, dst, src, cpustate->CF);
				WRITE16(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_MEM);
			}
			break;
		case 3:     // SBB Rm16, i16
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM16(modrm);
				src = ((UINT16)(INT16)(INT8)FETCH(cpustate));
				dst = SBB16(cpustate, dst, src, cpustate->CF);
				STORE_RM16(modrm, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(cpustate,modrm,1);
				dst = READ16(cpustate,ea);
				src = ((UINT16)(INT16)(INT8)FETCH(cpustate));
				dst = SBB16(cpustate, dst, src, cpustate->CF);
				WRITE16(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_MEM);
			}
			break;
		case 4:     // AND Rm16, i16
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM16(modrm);
				src = (UINT16)(INT16)(INT8)FETCH(cpustate);
				dst = AND16(cpustate,dst, src);
				STORE_RM16(modrm, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(cpustate,modrm,1);
				dst = READ16(cpustate,ea);
				src = (UINT16)(INT16)(INT8)FETCH(cpustate);
				dst = AND16(cpustate,dst, src);
				WRITE16(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_MEM);
			}
			break;
		case 5:     // SUB Rm16, i16
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM16(modrm);
				src = (UINT16)(INT16)(INT8)FETCH(cpustate);
				dst = SUB16(cpustate,dst, src);
				STORE_RM16(modrm, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(cpustate,modrm,1);
				dst = READ16(cpustate,ea);
				src = (UINT16)(INT16)(INT8)FETCH(cpustate);
				dst = SUB16(cpustate,dst, src);
				WRITE16(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_MEM);
			}
			break;
		case 6:     // XOR Rm16, i16
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM16(modrm);
				src = (UINT16)(INT16)(INT8)FETCH(cpustate);
				dst = XOR16(cpustate,dst, src);
				STORE_RM16(modrm, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(cpustate,modrm,1);
				dst = READ16(cpustate,ea);
				src = (UINT16)(INT16)(INT8)FETCH(cpustate);
				dst = XOR16(cpustate,dst, src);
				WRITE16(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_MEM);
			}
			break;
		case 7:     // CMP Rm16, i16
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM16(modrm);
				src = (UINT16)(INT16)(INT8)FETCH(cpustate);
				SUB16(cpustate,dst, src);
				CYCLES(cpustate,CYCLES_CMP_REG_REG);
			} else {
				ea = GetEA(cpustate,modrm,0);
				dst = READ16(cpustate,ea);
				src = (UINT16)(INT16)(INT8)FETCH(cpustate);
				SUB16(cpustate,dst, src);
				CYCLES(cpustate,CYCLES_CMP_REG_MEM);
			}
			break;
	}
}

static void I386OP(groupC1_16)(i386_state *cpustate)        // Opcode 0xc1
{
	UINT16 dst;
	UINT8 modrm = FETCH(cpustate);
	UINT8 shift;

	if( modrm >= 0xc0 ) {
		dst = LOAD_RM16(modrm);
		shift = FETCH(cpustate) & 0x1f;
		dst = i386_shift_rotate16(cpustate, modrm, dst, shift);
		STORE_RM16(modrm, dst);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		dst = READ16(cpustate,ea);
		shift = FETCH(cpustate) & 0x1f;
		dst = i386_shift_rotate16(cpustate, modrm, dst, shift);
		WRITE16(cpustate,ea, dst);
	}
}

static void I386OP(groupD1_16)(i386_state *cpustate)        // Opcode 0xd1
{
	UINT16 dst;
	UINT8 modrm = FETCH(cpustate);

	if( modrm >= 0xc0 ) {
		dst = LOAD_RM16(modrm);
		dst = i386_shift_rotate16(cpustate, modrm, dst, 1);
		STORE_RM16(modrm, dst);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		dst = READ16(cpustate,ea);
		dst = i386_shift_rotate16(cpustate, modrm, dst, 1);
		WRITE16(cpustate,ea, dst);
	}
}

static void I386OP(groupD3_16)(i386_state *cpustate)        // Opcode 0xd3
{
	UINT16 dst;
	UINT8 modrm = FETCH(cpustate);

	if( modrm >= 0xc0 ) {
		dst = LOAD_RM16(modrm);
		dst = i386_shift_rotate16(cpustate, modrm, dst, REG8(CL));
		STORE_RM16(modrm, dst);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		dst = READ16(cpustate,ea);
		dst = i386_shift_rotate16(cpustate, modrm, dst, REG8(CL));
		WRITE16(cpustate,ea, dst);
	}
}

static void I386OP(groupF7_16)(i386_state *cpustate)        // Opcode 0xf7
{
	UINT8 modrm = FETCH(cpustate);

	switch( (modrm >> 3) & 0x7 )
	{
		case 0:         /* TEST Rm16, i16 */
			if( modrm >= 0xc0 ) {
				UINT16 dst = LOAD_RM16(modrm);
				UINT16 src = FETCH16(cpustate);
				dst &= src;
				cpustate->CF = cpustate->OF = cpustate->AF = 0;
				SetSZPF16(dst);
				CYCLES(cpustate,CYCLES_TEST_IMM_REG);
			} else {
				UINT32 ea = GetEA(cpustate,modrm,0);
				UINT16 dst = READ16(cpustate,ea);
				UINT16 src = FETCH16(cpustate);
				dst &= src;
				cpustate->CF = cpustate->OF = cpustate->AF = 0;
				SetSZPF16(dst);
				CYCLES(cpustate,CYCLES_TEST_IMM_MEM);
			}
			break;
		case 2:         /* NOT Rm16 */
			if( modrm >= 0xc0 ) {
				UINT16 dst = LOAD_RM16(modrm);
				dst = ~dst;
				STORE_RM16(modrm, dst);
				CYCLES(cpustate,CYCLES_NOT_REG);
			} else {
				UINT32 ea = GetEA(cpustate,modrm,1);
				UINT16 dst = READ16(cpustate,ea);
				dst = ~dst;
				WRITE16(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_NOT_MEM);
			}
			break;
		case 3:         /* NEG Rm16 */
			if( modrm >= 0xc0 ) {
				UINT16 dst = LOAD_RM16(modrm);
				dst = SUB16(cpustate, 0, dst );
				STORE_RM16(modrm, dst);
				CYCLES(cpustate,CYCLES_NEG_REG);
			} else {
				UINT32 ea = GetEA(cpustate,modrm,1);
				UINT16 dst = READ16(cpustate,ea);
				dst = SUB16(cpustate, 0, dst );
				WRITE16(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_NEG_MEM);
			}
			break;
		case 4:         /* MUL AX, Rm16 */
			{
				UINT32 result;
				UINT16 src, dst;
				if( modrm >= 0xc0 ) {
					src = LOAD_RM16(modrm);
					CYCLES(cpustate,CYCLES_MUL16_ACC_REG);      /* TODO: Correct multiply timing */
				} else {
					UINT32 ea = GetEA(cpustate,modrm,0);
					src = READ16(cpustate,ea);
					CYCLES(cpustate,CYCLES_MUL16_ACC_MEM);      /* TODO: Correct multiply timing */
				}

				dst = REG16(AX);
				result = (UINT32)src * (UINT32)dst;
				REG16(DX) = (UINT16)(result >> 16);
				REG16(AX) = (UINT16)result;

				cpustate->CF = cpustate->OF = (REG16(DX) != 0);
			}
			break;
		case 5:         /* IMUL AX, Rm16 */
			{
				INT32 result;
				INT32 src, dst;
				if( modrm >= 0xc0 ) {
					src = (INT32)(INT16)LOAD_RM16(modrm);
					CYCLES(cpustate,CYCLES_IMUL16_ACC_REG);     /* TODO: Correct multiply timing */
				} else {
					UINT32 ea = GetEA(cpustate,modrm,0);
					src = (INT32)(INT16)READ16(cpustate,ea);
					CYCLES(cpustate,CYCLES_IMUL16_ACC_MEM);     /* TODO: Correct multiply timing */
				}

				dst = (INT32)(INT16)REG16(AX);
				result = src * dst;

				REG16(DX) = (UINT16)(result >> 16);
				REG16(AX) = (UINT16)result;

				cpustate->CF = cpustate->OF = !(result == (INT32)(INT16)result);
			}
			break;
		case 6:         /* DIV AX, Rm16 */
			{
				UINT32 quotient, remainder, result;
				UINT16 src;
				if( modrm >= 0xc0 ) {
					src = LOAD_RM16(modrm);
					CYCLES(cpustate,CYCLES_DIV16_ACC_REG);
				} else {
					UINT32 ea = GetEA(cpustate,modrm,0);
					src = READ16(cpustate,ea);
					CYCLES(cpustate,CYCLES_DIV16_ACC_MEM);
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

						// this flag is actually undefined, enable on non-cyrix
						if (cpustate->cpuid_id0 != 0x69727943)
							cpustate->CF = 1;
					}
				} else {
					i386_trap(cpustate, 0, 0, 0);
				}
			}
			break;
		case 7:         /* IDIV AX, Rm16 */
			{
				INT32 quotient, remainder, result;
				UINT16 src;
				if( modrm >= 0xc0 ) {
					src = LOAD_RM16(modrm);
					CYCLES(cpustate,CYCLES_IDIV16_ACC_REG);
				} else {
					UINT32 ea = GetEA(cpustate,modrm,0);
					src = READ16(cpustate,ea);
					CYCLES(cpustate,CYCLES_IDIV16_ACC_MEM);
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

static void I386OP(groupFF_16)(i386_state *cpustate)        // Opcode 0xff
{
	UINT8 modrm = FETCH(cpustate);

	switch( (modrm >> 3) & 0x7 )
	{
		case 0:         /* INC Rm16 */
			if( modrm >= 0xc0 ) {
				UINT16 dst = LOAD_RM16(modrm);
				dst = INC16(cpustate,dst);
				STORE_RM16(modrm, dst);
				CYCLES(cpustate,CYCLES_INC_REG);
			} else {
				UINT32 ea = GetEA(cpustate,modrm,1);
				UINT16 dst = READ16(cpustate,ea);
				dst = INC16(cpustate,dst);
				WRITE16(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_INC_MEM);
			}
			break;
		case 1:         /* DEC Rm16 */
			if( modrm >= 0xc0 ) {
				UINT16 dst = LOAD_RM16(modrm);
				dst = DEC16(cpustate,dst);
				STORE_RM16(modrm, dst);
				CYCLES(cpustate,CYCLES_DEC_REG);
			} else {
				UINT32 ea = GetEA(cpustate,modrm,1);
				UINT16 dst = READ16(cpustate,ea);
				dst = DEC16(cpustate,dst);
				WRITE16(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_DEC_MEM);
			}
			break;
		case 2:         /* CALL Rm16 */
			{
				UINT16 address;
				if( modrm >= 0xc0 ) {
					address = LOAD_RM16(modrm);
					CYCLES(cpustate,CYCLES_CALL_REG);       /* TODO: Timing = 7 + m */
				} else {
					UINT32 ea = GetEA(cpustate,modrm,0);
					address = READ16(cpustate,ea);
					CYCLES(cpustate,CYCLES_CALL_MEM);       /* TODO: Timing = 10 + m */
				}
				PUSH16(cpustate, cpustate->eip );
				cpustate->eip = address;
				CHANGE_PC(cpustate,cpustate->eip);
			}
			break;
		case 3:         /* CALL FAR Rm16 */
			{
				UINT16 address, selector;
				if( modrm >= 0xc0 )
				{
					fatalerror("i386: groupFF_16 /%d NYI\n", (modrm >> 3) & 0x7);
				}
				else
				{
					UINT32 ea = GetEA(cpustate,modrm,0);
					address = READ16(cpustate,ea + 0);
					selector = READ16(cpustate,ea + 2);
					CYCLES(cpustate,CYCLES_CALL_MEM_INTERSEG);      /* TODO: Timing = 10 + m */

					if(PROTECTED_MODE && !V8086_MODE)
					{
						i386_protected_mode_call(cpustate,selector,address,1,0);
					}
					else
					{
						PUSH16(cpustate, cpustate->sreg[CS].selector );
						PUSH16(cpustate, cpustate->eip );
						cpustate->sreg[CS].selector = selector;
						cpustate->performed_intersegment_jump = 1;
						i386_load_segment_descriptor(cpustate, CS );
						cpustate->eip = address;
						CHANGE_PC(cpustate,cpustate->eip);
					}
				}
			}
			break;
		case 4:         /* JMP Rm16 */
			{
				UINT16 address;
				if( modrm >= 0xc0 ) {
					address = LOAD_RM16(modrm);
					CYCLES(cpustate,CYCLES_JMP_REG);        /* TODO: Timing = 7 + m */
				} else {
					UINT32 ea = GetEA(cpustate,modrm,0);
					address = READ16(cpustate,ea);
					CYCLES(cpustate,CYCLES_JMP_MEM);        /* TODO: Timing = 10 + m */
				}
				cpustate->eip = address;
				CHANGE_PC(cpustate,cpustate->eip);
			}
			break;
		case 5:         /* JMP FAR Rm16 */
			{
				UINT16 address, selector;

				if( modrm >= 0xc0 )
				{
					fatalerror("i386: groupFF_16 /%d NYI\n", (modrm >> 3) & 0x7);
				}
				else
				{
					UINT32 ea = GetEA(cpustate,modrm,0);
					address = READ16(cpustate,ea + 0);
					selector = READ16(cpustate,ea + 2);
					CYCLES(cpustate,CYCLES_JMP_MEM_INTERSEG);       /* TODO: Timing = 10 + m */
					if(PROTECTED_MODE && !V8086_MODE)
					{
						i386_protected_mode_jump(cpustate,selector,address,1,0);
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
		case 6:         /* PUSH Rm16 */
			{
				UINT16 value;
				if( modrm >= 0xc0 ) {
					value = LOAD_RM16(modrm);
				} else {
					UINT32 ea = GetEA(cpustate,modrm,0);
					value = READ16(cpustate,ea);
				}
				PUSH16(cpustate,value);
				CYCLES(cpustate,CYCLES_PUSH_RM);
			}
			break;
		case 7:
			I386OP(invalid)(cpustate);
			break;
		default:
			fatalerror("i386: groupFF_16 /%d unimplemented\n", (modrm >> 3) & 0x7);
			break;
	}
}

static void I386OP(group0F00_16)(i386_state *cpustate)          // Opcode 0x0f 00
{
	UINT32 address, ea;
	UINT8 modrm = FETCH(cpustate);
	I386_SREG seg;
	UINT8 result;

	switch( (modrm >> 3) & 0x7 )
	{
		case 0:         /* SLDT */
			if ( PROTECTED_MODE && !V8086_MODE )
			{
				if( modrm >= 0xc0 ) {
					STORE_RM16(modrm, cpustate->ldtr.segment);
					CYCLES(cpustate,CYCLES_SLDT_REG);
				} else {
					ea = GetEA(cpustate,modrm,1);
					WRITE16(cpustate, ea, cpustate->ldtr.segment);
					CYCLES(cpustate,CYCLES_SLDT_MEM);
				}
			}
			else
			{
				i386_trap(cpustate,6, 0, 0);
			}
			break;
		case 1:         /* STR */
			if ( PROTECTED_MODE && !V8086_MODE )
			{
				if( modrm >= 0xc0 ) {
					STORE_RM16(modrm, cpustate->task.segment);
					CYCLES(cpustate,CYCLES_STR_REG);
				} else {
					ea = GetEA(cpustate,modrm,1);
					WRITE16(cpustate, ea, cpustate->task.segment);
					CYCLES(cpustate,CYCLES_STR_MEM);
				}
			}
			else
			{
				i386_trap(cpustate,6, 0, 0);
			}
			break;
		case 2:         /* LLDT */
			if ( PROTECTED_MODE && !V8086_MODE )
			{
				if(cpustate->CPL)
					FAULT(FAULT_GP,0)
				if( modrm >= 0xc0 ) {
					address = LOAD_RM16(modrm);
					cpustate->ldtr.segment = address;
					CYCLES(cpustate,CYCLES_LLDT_REG);
				} else {
					ea = GetEA(cpustate,modrm,0);
					cpustate->ldtr.segment = READ16(cpustate,ea);
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

		case 3:         /* LTR */
			if ( PROTECTED_MODE && !V8086_MODE )
			{
				if(cpustate->CPL)
					FAULT(FAULT_GP,0)
				if( modrm >= 0xc0 ) {
					address = LOAD_RM16(modrm);
					cpustate->task.segment = address;
					CYCLES(cpustate,CYCLES_LTR_REG);
				} else {
					ea = GetEA(cpustate,modrm,0);
					cpustate->task.segment = READ16(cpustate,ea);
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
				result = 1;
				if( modrm >= 0xc0 ) {
					address = LOAD_RM16(modrm);
					CYCLES(cpustate,CYCLES_VERR_REG);
				} else {
					ea = GetEA(cpustate,modrm,0);
					address = READ16(cpustate,ea);
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
				result = 1;
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
			fatalerror("i386: group0F00_16 /%d unimplemented\n", (modrm >> 3) & 0x7);
			break;
	}
}

static void I386OP(group0F01_16)(i386_state *cpustate)      // Opcode 0x0f 01
{
	UINT8 modrm = FETCH(cpustate);
	UINT16 address;
	UINT32 ea;

	switch( (modrm >> 3) & 0x7 )
	{
		case 0:         /* SGDT */
			{
				if( modrm >= 0xc0 ) {
					address = LOAD_RM16(modrm);
					ea = i386_translate(cpustate, CS, address, 1 );
				} else {
					ea = GetEA(cpustate,modrm,1);
				}
				WRITE16(cpustate,ea, cpustate->gdtr.limit);
				WRITE32(cpustate,ea + 2, cpustate->gdtr.base & 0xffffff);
				CYCLES(cpustate,CYCLES_SGDT);
				break;
			}
		case 1:         /* SIDT */
			{
				if (modrm >= 0xc0)
				{
					address = LOAD_RM16(modrm);
					ea = i386_translate(cpustate, CS, address, 1 );
				}
				else
				{
					ea = GetEA(cpustate,modrm,1);
				}
				WRITE16(cpustate,ea, cpustate->idtr.limit);
				WRITE32(cpustate,ea + 2, cpustate->idtr.base & 0xffffff);
				CYCLES(cpustate,CYCLES_SIDT);
				break;
			}
		case 2:         /* LGDT */
			{
				if(PROTECTED_MODE && cpustate->CPL)
					FAULT(FAULT_GP,0)
				if( modrm >= 0xc0 ) {
					address = LOAD_RM16(modrm);
					ea = i386_translate(cpustate, CS, address, 0 );
				} else {
					ea = GetEA(cpustate,modrm,0);
				}
				cpustate->gdtr.limit = READ16(cpustate,ea);
				cpustate->gdtr.base = READ32(cpustate,ea + 2) & 0xffffff;
				CYCLES(cpustate,CYCLES_LGDT);
				break;
			}
		case 3:         /* LIDT */
			{
				if(PROTECTED_MODE && cpustate->CPL)
					FAULT(FAULT_GP,0)
				if( modrm >= 0xc0 ) {
					address = LOAD_RM16(modrm);
					ea = i386_translate(cpustate, CS, address, 0 );
				} else {
					ea = GetEA(cpustate,modrm,0);
				}
				cpustate->idtr.limit = READ16(cpustate,ea);
				cpustate->idtr.base = READ32(cpustate,ea + 2) & 0xffffff;
				CYCLES(cpustate,CYCLES_LIDT);
				break;
			}
		case 4:         /* SMSW */
			{
				if( modrm >= 0xc0 ) {
					STORE_RM16(modrm, cpustate->cr[0]);
					CYCLES(cpustate,CYCLES_SMSW_REG);
				} else {
					ea = GetEA(cpustate,modrm,1);
					WRITE16(cpustate,ea, cpustate->cr[0]);
					CYCLES(cpustate,CYCLES_SMSW_MEM);
				}
				break;
			}
		case 6:         /* LMSW */
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
			fatalerror("i386: unimplemented opcode 0x0f 01 /%d at %08X\n", (modrm >> 3) & 0x7, cpustate->eip - 2);
			break;
	}
}

static void I386OP(group0FBA_16)(i386_state *cpustate)      // Opcode 0x0f ba
{
	UINT8 modrm = FETCH(cpustate);

	switch( (modrm >> 3) & 0x7 )
	{
		case 4:         /* BT Rm16, i8 */
			if( modrm >= 0xc0 ) {
				UINT16 dst = LOAD_RM16(modrm);
				UINT8 bit = FETCH(cpustate);

				if( dst & (1 << bit) )
					cpustate->CF = 1;
				else
					cpustate->CF = 0;

				CYCLES(cpustate,CYCLES_BT_IMM_REG);
			} else {
				UINT32 ea = GetEA(cpustate,modrm,0);
				UINT16 dst = READ16(cpustate,ea);
				UINT8 bit = FETCH(cpustate);

				if( dst & (1 << bit) )
					cpustate->CF = 1;
				else
					cpustate->CF = 0;

				CYCLES(cpustate,CYCLES_BT_IMM_MEM);
			}
			break;
		case 5:         /* BTS Rm16, i8 */
			if( modrm >= 0xc0 ) {
				UINT16 dst = LOAD_RM16(modrm);
				UINT8 bit = FETCH(cpustate);

				if( dst & (1 << bit) )
					cpustate->CF = 1;
				else
					cpustate->CF = 0;
				dst |= (1 << bit);

				STORE_RM16(modrm, dst);
				CYCLES(cpustate,CYCLES_BTS_IMM_REG);
			} else {
				UINT32 ea = GetEA(cpustate,modrm,1);
				UINT16 dst = READ16(cpustate,ea);
				UINT8 bit = FETCH(cpustate);

				if( dst & (1 << bit) )
					cpustate->CF = 1;
				else
					cpustate->CF = 0;
				dst |= (1 << bit);

				WRITE16(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_BTS_IMM_MEM);
			}
			break;
		case 6:         /* BTR Rm16, i8 */
			if( modrm >= 0xc0 ) {
				UINT16 dst = LOAD_RM16(modrm);
				UINT8 bit = FETCH(cpustate);

				if( dst & (1 << bit) )
					cpustate->CF = 1;
				else
					cpustate->CF = 0;
				dst &= ~(1 << bit);

				STORE_RM16(modrm, dst);
				CYCLES(cpustate,CYCLES_BTR_IMM_REG);
			} else {
				UINT32 ea = GetEA(cpustate,modrm,1);
				UINT16 dst = READ16(cpustate,ea);
				UINT8 bit = FETCH(cpustate);

				if( dst & (1 << bit) )
					cpustate->CF = 1;
				else
					cpustate->CF = 0;
				dst &= ~(1 << bit);

				WRITE16(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_BTR_IMM_MEM);
			}
			break;
		case 7:         /* BTC Rm16, i8 */
			if( modrm >= 0xc0 ) {
				UINT16 dst = LOAD_RM16(modrm);
				UINT8 bit = FETCH(cpustate);

				if( dst & (1 << bit) )
					cpustate->CF = 1;
				else
					cpustate->CF = 0;
				dst ^= (1 << bit);

				STORE_RM16(modrm, dst);
				CYCLES(cpustate,CYCLES_BTC_IMM_REG);
			} else {
				UINT32 ea = GetEA(cpustate,modrm,1);
				UINT16 dst = READ16(cpustate,ea);
				UINT8 bit = FETCH(cpustate);

				if( dst & (1 << bit) )
					cpustate->CF = 1;
				else
					cpustate->CF = 0;
				dst ^= (1 << bit);

				WRITE16(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_BTC_IMM_MEM);
			}
			break;
		default:
			fatalerror("i386: group0FBA_16 /%d unknown\n", (modrm >> 3) & 0x7);
			break;
	}
}

static void I386OP(lar_r16_rm16)(i386_state *cpustate)  // Opcode 0x0f 0x02
{
	UINT8 modrm = FETCH(cpustate);
	I386_SREG seg;
	UINT8 type;

	if(PROTECTED_MODE && !V8086_MODE)
	{
		memset(&seg,0,sizeof(seg));
		if(modrm >= 0xc0)
		{
			seg.selector = LOAD_RM16(modrm);
			CYCLES(cpustate,CYCLES_LAR_REG);
		}
		else
		{
			UINT32 ea = GetEA(cpustate,modrm,0);
			seg.selector = READ16(cpustate,ea);
			CYCLES(cpustate,CYCLES_LAR_MEM);
		}
		if(seg.selector == 0)
		{
			SetZF(0);  // not a valid segment
		//  logerror("i386 (%08x): LAR: Selector %04x is invalid type.\n",cpustate->pc,seg.selector);
		}
		else
		{
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
					STORE_REG16(modrm,(seg.flags << 8) & 0xff00);
					SetZF(1);
				}
			}
			else
			{  // data or code segment (both are valid for LAR)
				STORE_REG16(modrm,(seg.flags << 8) & 0xff00);
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

static void I386OP(lsl_r16_rm16)(i386_state *cpustate)  // Opcode 0x0f 0x03
{
	UINT8 modrm = FETCH(cpustate);
	UINT32 limit;
	I386_SREG seg;

	if(PROTECTED_MODE && !V8086_MODE)
	{
		memset(&seg, 0, sizeof(seg));
		if(modrm >= 0xc0)
		{
			seg.selector = LOAD_RM16(modrm);
		}
		else
		{
			UINT32 ea = GetEA(cpustate,modrm,0);
			seg.selector = READ16(cpustate,ea);
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
				STORE_REG16(modrm,limit & 0x0000ffff);
				SetZF(1);
			}
		}
	}
	else
		i386_trap(cpustate,6, 0, 0);
}

static void I386OP(bound_r16_m16_m16)(i386_state *cpustate) // Opcode 0x62
{
	UINT8 modrm;
	INT16 val, low, high;

	modrm = FETCH(cpustate);

	if (modrm >= 0xc0)
	{
		low = high = LOAD_RM16(modrm);
	}
	else
	{
		UINT32 ea = GetEA(cpustate,modrm,0);
		low = READ16(cpustate,ea + 0);
		high = READ16(cpustate,ea + 2);
	}
	val = LOAD_REG16(modrm);

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

static void I386OP(retf16)(i386_state *cpustate)            // Opcode 0xcb
{
	if(PROTECTED_MODE && !V8086_MODE)
	{
		i386_protected_mode_retf(cpustate,0,0);
	}
	else
	{
		cpustate->eip = POP16(cpustate);
		cpustate->sreg[CS].selector = POP16(cpustate);
		i386_load_segment_descriptor(cpustate, CS );
		CHANGE_PC(cpustate,cpustate->eip);
	}

	CYCLES(cpustate,CYCLES_RET_INTERSEG);
}

static void I386OP(retf_i16)(i386_state *cpustate)          // Opcode 0xca
{
	UINT16 count = FETCH16(cpustate);

	if(PROTECTED_MODE && !V8086_MODE)
	{
		i386_protected_mode_retf(cpustate,count,0);
	}
	else
	{
		cpustate->eip = POP16(cpustate);
		cpustate->sreg[CS].selector = POP16(cpustate);
		i386_load_segment_descriptor(cpustate, CS );
		CHANGE_PC(cpustate,cpustate->eip);
		REG16(SP) += count;
	}

	CYCLES(cpustate,CYCLES_RET_IMM_INTERSEG);
}

static void I386OP(load_far_pointer16)(i386_state *cpustate, int s)
{
	UINT8 modrm = FETCH(cpustate);
	UINT16 selector;

	if( modrm >= 0xc0 ) {
		fatalerror("i386: load_far_pointer16 NYI\n");
	} else {
		UINT32 ea = GetEA(cpustate,modrm,0);
		STORE_REG16(modrm, READ16(cpustate,ea + 0));
		selector = READ16(cpustate,ea + 2);
		i386_sreg_load(cpustate,selector,s,NULL);
	}
}

static void I386OP(lds16)(i386_state *cpustate)             // Opcode 0xc5
{
	I386OP(load_far_pointer16)(cpustate, DS);
	CYCLES(cpustate,CYCLES_LDS);
}

static void I386OP(lss16)(i386_state *cpustate)             // Opcode 0x0f 0xb2
{
	I386OP(load_far_pointer16)(cpustate, SS);
	CYCLES(cpustate,CYCLES_LSS);
}

static void I386OP(les16)(i386_state *cpustate)             // Opcode 0xc4
{
	I386OP(load_far_pointer16)(cpustate, ES);
	CYCLES(cpustate,CYCLES_LES);
}

static void I386OP(lfs16)(i386_state *cpustate)             // Opcode 0x0f 0xb4
{
	I386OP(load_far_pointer16)(cpustate, FS);
	CYCLES(cpustate,CYCLES_LFS);
}

static void I386OP(lgs16)(i386_state *cpustate)             // Opcode 0x0f 0xb5
{
	I386OP(load_far_pointer16)(cpustate, GS);
	CYCLES(cpustate,CYCLES_LGS);
}
