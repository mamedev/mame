// license:BSD-3-Clause
// copyright-holders:Ville Linde, Barry Rodewald, Carl, Philip Bennett
UINT32 i386_device::i386_shift_rotate32(UINT8 modrm, UINT32 value, UINT8 shift)
{
	UINT32 dst, src;
	dst = value;
	src = value;

	if( shift == 0 ) {
		CYCLES_RM(modrm, 3, 7);
	} else if( shift == 1 ) {
		switch( (modrm >> 3) & 0x7 )
		{
			case 0:         /* ROL rm32, 1 */
				m_CF = (src & 0x80000000) ? 1 : 0;
				dst = (src << 1) + m_CF;
				m_OF = ((src ^ dst) & 0x80000000) ? 1 : 0;
				CYCLES_RM(modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 1:         /* ROR rm32, 1 */
				m_CF = (src & 0x1) ? 1 : 0;
				dst = (m_CF << 31) | (src >> 1);
				m_OF = ((src ^ dst) & 0x80000000) ? 1 : 0;
				CYCLES_RM(modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 2:         /* RCL rm32, 1 */
				dst = (src << 1) + m_CF;
				m_CF = (src & 0x80000000) ? 1 : 0;
				m_OF = ((src ^ dst) & 0x80000000) ? 1 : 0;
				CYCLES_RM(modrm, CYCLES_ROTATE_CARRY_REG, CYCLES_ROTATE_CARRY_MEM);
				break;
			case 3:         /* RCR rm32, 1 */
				dst = (m_CF << 31) | (src >> 1);
				m_CF = src & 0x1;
				m_OF = ((src ^ dst) & 0x80000000) ? 1 : 0;
				CYCLES_RM(modrm, CYCLES_ROTATE_CARRY_REG, CYCLES_ROTATE_CARRY_MEM);
				break;
			case 4:         /* SHL/SAL rm32, 1 */
			case 6:
				dst = src << 1;
				m_CF = (src & 0x80000000) ? 1 : 0;
				m_OF = (((m_CF << 31) ^ dst) & 0x80000000) ? 1 : 0;
				SetSZPF32(dst);
				CYCLES_RM(modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 5:         /* SHR rm32, 1 */
				dst = src >> 1;
				m_CF = src & 0x1;
				m_OF = (src & 0x80000000) ? 1 : 0;
				SetSZPF32(dst);
				CYCLES_RM(modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 7:         /* SAR rm32, 1 */
				dst = (INT32)(src) >> 1;
				m_CF = src & 0x1;
				m_OF = 0;
				SetSZPF32(dst);
				CYCLES_RM(modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
		}

	} else {
		shift &= 31;
		switch( (modrm >> 3) & 0x7 )
		{
			case 0:         /* ROL rm32, i8 */
				dst = ((src & ((UINT32)0xffffffff >> shift)) << shift) |
						((src & ((UINT32)0xffffffff << (32-shift))) >> (32-shift));
				m_CF = dst & 0x1;
				m_OF = (dst & 1) ^ (dst >> 31);
				CYCLES_RM(modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 1:         /* ROR rm32, i8 */
				dst = ((src & ((UINT32)0xffffffff << shift)) >> shift) |
						((src & ((UINT32)0xffffffff >> (32-shift))) << (32-shift));
				m_CF = (dst >> 31) & 0x1;
				m_OF = ((dst >> 31) ^ (dst >> 30)) & 1;
				CYCLES_RM(modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 2:         /* RCL rm32, i8 */
				dst = ((src & ((UINT32)0xffffffff >> shift)) << shift) |
						((src & ((UINT32)0xffffffff << (33-shift))) >> (33-shift)) |
						(m_CF << (shift-1));
				m_CF = (src >> (32-shift)) & 0x1;
				m_OF = m_CF ^ ((dst >> 31) & 1);
				CYCLES_RM(modrm, CYCLES_ROTATE_CARRY_REG, CYCLES_ROTATE_CARRY_MEM);
				break;
			case 3:         /* RCR rm32, i8 */
				dst = ((src & ((UINT32)0xffffffff << shift)) >> shift) |
						((src & ((UINT32)0xffffffff >> (32-shift))) << (33-shift)) |
						(m_CF << (32-shift));
				m_CF = (src >> (shift-1)) & 0x1;
				m_OF = ((dst >> 31) ^ (dst >> 30)) & 1;
				CYCLES_RM(modrm, CYCLES_ROTATE_CARRY_REG, CYCLES_ROTATE_CARRY_MEM);
				break;
			case 4:         /* SHL/SAL rm32, i8 */
			case 6:
				dst = src << shift;
				m_CF = (src & (1 << (32-shift))) ? 1 : 0;
				SetSZPF32(dst);
				CYCLES_RM(modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 5:         /* SHR rm32, i8 */
				dst = src >> shift;
				m_CF = (src & (1 << (shift-1))) ? 1 : 0;
				SetSZPF32(dst);
				CYCLES_RM(modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 7:         /* SAR rm32, i8 */
				dst = (INT32)src >> shift;
				m_CF = (src & (1 << (shift-1))) ? 1 : 0;
				SetSZPF32(dst);
				CYCLES_RM(modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
		}

	}
	return dst;
}



void i386_device::i386_adc_rm32_r32()      // Opcode 0x11
{
	UINT32 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_REG32(modrm);
		dst = LOAD_RM32(modrm);
		dst = ADC32(dst, src, m_CF);
		STORE_RM32(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm,1);
		src = LOAD_REG32(modrm);
		dst = READ32(ea);
		dst = ADC32(dst, src, m_CF);
		WRITE32(ea, dst);
		CYCLES(CYCLES_ALU_REG_MEM);
	}
}

void i386_device::i386_adc_r32_rm32()      // Opcode 0x13
{
	UINT32 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_RM32(modrm);
		dst = LOAD_REG32(modrm);
		dst = ADC32(dst, src, m_CF);
		STORE_REG32(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm,0);
		src = READ32(ea);
		dst = LOAD_REG32(modrm);
		dst = ADC32(dst, src, m_CF);
		STORE_REG32(modrm, dst);
		CYCLES(CYCLES_ALU_MEM_REG);
	}
}

void i386_device::i386_adc_eax_i32()       // Opcode 0x15
{
	UINT32 src, dst;
	src = FETCH32();
	dst = REG32(EAX);
	dst = ADC32(dst, src, m_CF);
	REG32(EAX) = dst;
	CYCLES(CYCLES_ALU_IMM_ACC);
}

void i386_device::i386_add_rm32_r32()      // Opcode 0x01
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
		UINT32 ea = GetEA(modrm,1);
		src = LOAD_REG32(modrm);
		dst = READ32(ea);
		dst = ADD32(dst, src);
		WRITE32(ea, dst);
		CYCLES(CYCLES_ALU_REG_MEM);
	}
}

void i386_device::i386_add_r32_rm32()      // Opcode 0x03
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
		UINT32 ea = GetEA(modrm,0);
		src = READ32(ea);
		dst = LOAD_REG32(modrm);
		dst = ADD32(dst, src);
		STORE_REG32(modrm, dst);
		CYCLES(CYCLES_ALU_MEM_REG);
	}
}

void i386_device::i386_add_eax_i32()       // Opcode 0x05
{
	UINT32 src, dst;
	src = FETCH32();
	dst = REG32(EAX);
	dst = ADD32(dst, src);
	REG32(EAX) = dst;
	CYCLES(CYCLES_ALU_IMM_ACC);
}

void i386_device::i386_and_rm32_r32()      // Opcode 0x21
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
		UINT32 ea = GetEA(modrm,1);
		src = LOAD_REG32(modrm);
		dst = READ32(ea);
		dst = AND32(dst, src);
		WRITE32(ea, dst);
		CYCLES(CYCLES_ALU_REG_MEM);
	}
}

void i386_device::i386_and_r32_rm32()      // Opcode 0x23
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
		UINT32 ea = GetEA(modrm,0);
		src = READ32(ea);
		dst = LOAD_REG32(modrm);
		dst = AND32(dst, src);
		STORE_REG32(modrm, dst);
		CYCLES(CYCLES_ALU_MEM_REG);
	}
}

void i386_device::i386_and_eax_i32()       // Opcode 0x25
{
	UINT32 src, dst;
	src = FETCH32();
	dst = REG32(EAX);
	dst = AND32(dst, src);
	REG32(EAX) = dst;
	CYCLES(CYCLES_ALU_IMM_ACC);
}

void i386_device::i386_bsf_r32_rm32()      // Opcode 0x0f bc
{
	UINT32 src, dst, temp;
	UINT8 modrm = FETCH();

	if( modrm >= 0xc0 ) {
		src = LOAD_RM32(modrm);
	} else {
		UINT32 ea = GetEA(modrm,0);
		src = READ32(ea);
	}

	dst = 0;

	if( src == 0 ) {
		m_ZF = 1;
	} else {
		m_ZF = 0;
		temp = 0;
		while( (src & (1 << temp)) == 0 ) {
			temp++;
			dst = temp;
			CYCLES(CYCLES_BSF);
		}
		STORE_REG32(modrm, dst);
	}
	CYCLES(CYCLES_BSF_BASE);
}

void i386_device::i386_bsr_r32_rm32()      // Opcode 0x0f bd
{
	UINT32 src, dst, temp;
	UINT8 modrm = FETCH();

	if( modrm >= 0xc0 ) {
		src = LOAD_RM32(modrm);
	} else {
		UINT32 ea = GetEA(modrm,0);
		src = READ32(ea);
	}

	dst = 0;

	if( src == 0 ) {
		m_ZF = 1;
	} else {
		m_ZF = 0;
		dst = temp = 31;
		while( (src & (1 << temp)) == 0 ) {
			temp--;
			dst = temp;
			CYCLES(CYCLES_BSR);
		}
		STORE_REG32(modrm, dst);
	}
	CYCLES(CYCLES_BSR_BASE);
}

void i386_device::i386_bt_rm32_r32()       // Opcode 0x0f a3
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		UINT32 dst = LOAD_RM32(modrm);
		UINT32 bit = LOAD_REG32(modrm);

		if( dst & (1 << bit) )
			m_CF = 1;
		else
			m_CF = 0;

		CYCLES(CYCLES_BT_REG_REG);
	} else {
		UINT8 segment;
		UINT32 ea = GetNonTranslatedEA(modrm,&segment);
		UINT32 bit = LOAD_REG32(modrm);
		ea += 4*(bit/32);
		ea = i386_translate(segment,(m_address_size)?ea:(ea&0xffff),0);
		bit %= 32;
		UINT32 dst = READ32(ea);

		if( dst & (1 << bit) )
			m_CF = 1;
		else
			m_CF = 0;

		CYCLES(CYCLES_BT_REG_MEM);
	}
}

void i386_device::i386_btc_rm32_r32()      // Opcode 0x0f bb
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		UINT32 dst = LOAD_RM32(modrm);
		UINT32 bit = LOAD_REG32(modrm);

		if( dst & (1 << bit) )
			m_CF = 1;
		else
			m_CF = 0;
		dst ^= (1 << bit);

		STORE_RM32(modrm, dst);
		CYCLES(CYCLES_BTC_REG_REG);
	} else {
		UINT8 segment;
		UINT32 ea = GetNonTranslatedEA(modrm,&segment);
		UINT32 bit = LOAD_REG32(modrm);
		ea += 4*(bit/32);
		ea = i386_translate(segment,(m_address_size)?ea:(ea&0xffff),1);
		bit %= 32;
		UINT32 dst = READ32(ea);

		if( dst & (1 << bit) )
			m_CF = 1;
		else
			m_CF = 0;
		dst ^= (1 << bit);

		WRITE32(ea, dst);
		CYCLES(CYCLES_BTC_REG_MEM);
	}
}

void i386_device::i386_btr_rm32_r32()      // Opcode 0x0f b3
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		UINT32 dst = LOAD_RM32(modrm);
		UINT32 bit = LOAD_REG32(modrm);

		if( dst & (1 << bit) )
			m_CF = 1;
		else
			m_CF = 0;
		dst &= ~(1 << bit);

		STORE_RM32(modrm, dst);
		CYCLES(CYCLES_BTR_REG_REG);
	} else {
		UINT8 segment;
		UINT32 ea = GetNonTranslatedEA(modrm,&segment);
		UINT32 bit = LOAD_REG32(modrm);
		ea += 4*(bit/32);
		ea = i386_translate(segment,(m_address_size)?ea:(ea&0xffff),1);
		bit %= 32;
		UINT32 dst = READ32(ea);

		if( dst & (1 << bit) )
			m_CF = 1;
		else
			m_CF = 0;
		dst &= ~(1 << bit);

		WRITE32(ea, dst);
		CYCLES(CYCLES_BTR_REG_MEM);
	}
}

void i386_device::i386_bts_rm32_r32()      // Opcode 0x0f ab
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		UINT32 dst = LOAD_RM32(modrm);
		UINT32 bit = LOAD_REG32(modrm);

		if( dst & (1 << bit) )
			m_CF = 1;
		else
			m_CF = 0;
		dst |= (1 << bit);

		STORE_RM32(modrm, dst);
		CYCLES(CYCLES_BTS_REG_REG);
	} else {
		UINT8 segment;
		UINT32 ea = GetNonTranslatedEA(modrm,&segment);
		UINT32 bit = LOAD_REG32(modrm);
		ea += 4*(bit/32);
		ea = i386_translate(segment,(m_address_size)?ea:(ea&0xffff),1);
		bit %= 32;
		UINT32 dst = READ32(ea);

		if( dst & (1 << bit) )
			m_CF = 1;
		else
			m_CF = 0;
		dst |= (1 << bit);

		WRITE32(ea, dst);
		CYCLES(CYCLES_BTS_REG_MEM);
	}
}

void i386_device::i386_call_abs32()        // Opcode 0x9a
{
	UINT32 offset = FETCH32();
	UINT16 ptr = FETCH16();

	if(PROTECTED_MODE && !V8086_MODE)
	{
		i386_protected_mode_call(ptr,offset,0,1);
	}
	else
	{
		PUSH32(m_sreg[CS].selector );
		PUSH32(m_eip );
		m_sreg[CS].selector = ptr;
		m_performed_intersegment_jump = 1;
		m_eip = offset;
		i386_load_segment_descriptor(CS);
	}
	CYCLES(CYCLES_CALL_INTERSEG);
	CHANGE_PC(m_eip);
}

void i386_device::i386_call_rel32()        // Opcode 0xe8
{
	INT32 disp = FETCH32();
	PUSH32(m_eip );
	m_eip += disp;
	CHANGE_PC(m_eip);
	CYCLES(CYCLES_CALL);       /* TODO: Timing = 7 + m */
}

void i386_device::i386_cdq()               // Opcode 0x99
{
	if( REG32(EAX) & 0x80000000 ) {
		REG32(EDX) = 0xffffffff;
	} else {
		REG32(EDX) = 0x00000000;
	}
	CYCLES(CYCLES_CWD);
}

void i386_device::i386_cmp_rm32_r32()      // Opcode 0x39
{
	UINT32 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_REG32(modrm);
		dst = LOAD_RM32(modrm);
		SUB32(dst, src);
		CYCLES(CYCLES_CMP_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm,0);
		src = LOAD_REG32(modrm);
		dst = READ32(ea);
		SUB32(dst, src);
		CYCLES(CYCLES_CMP_REG_MEM);
	}
}

void i386_device::i386_cmp_r32_rm32()      // Opcode 0x3b
{
	UINT32 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_RM32(modrm);
		dst = LOAD_REG32(modrm);
		SUB32(dst, src);
		CYCLES(CYCLES_CMP_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm,0);
		src = READ32(ea);
		dst = LOAD_REG32(modrm);
		SUB32(dst, src);
		CYCLES(CYCLES_CMP_MEM_REG);
	}
}

void i386_device::i386_cmp_eax_i32()       // Opcode 0x3d
{
	UINT32 src, dst;
	src = FETCH32();
	dst = REG32(EAX);
	SUB32(dst, src);
	CYCLES(CYCLES_CMP_IMM_ACC);
}

void i386_device::i386_cmpsd()             // Opcode 0xa7
{
	UINT32 eas, ead, src, dst;
	if( m_segment_prefix ) {
		eas = i386_translate(m_segment_override, m_address_size ? REG32(ESI) : REG16(SI), 0 );
	} else {
		eas = i386_translate(DS, m_address_size ? REG32(ESI) : REG16(SI), 0 );
	}
	ead = i386_translate(ES, m_address_size ? REG32(EDI) : REG16(DI), 0 );
	src = READ32(eas);
	dst = READ32(ead);
	SUB32(src,dst);
	BUMP_SI(4);
	BUMP_DI(4);
	CYCLES(CYCLES_CMPS);
}

void i386_device::i386_cwde()              // Opcode 0x98
{
	REG32(EAX) = (INT32)((INT16)REG16(AX));
	CYCLES(CYCLES_CBW);
}

void i386_device::i386_dec_eax()           // Opcode 0x48
{
	REG32(EAX) = DEC32(REG32(EAX) );
	CYCLES(CYCLES_DEC_REG);
}

void i386_device::i386_dec_ecx()           // Opcode 0x49
{
	REG32(ECX) = DEC32(REG32(ECX) );
	CYCLES(CYCLES_DEC_REG);
}

void i386_device::i386_dec_edx()           // Opcode 0x4a
{
	REG32(EDX) = DEC32(REG32(EDX) );
	CYCLES(CYCLES_DEC_REG);
}

void i386_device::i386_dec_ebx()           // Opcode 0x4b
{
	REG32(EBX) = DEC32(REG32(EBX) );
	CYCLES(CYCLES_DEC_REG);
}

void i386_device::i386_dec_esp()           // Opcode 0x4c
{
	REG32(ESP) = DEC32(REG32(ESP) );
	CYCLES(CYCLES_DEC_REG);
}

void i386_device::i386_dec_ebp()           // Opcode 0x4d
{
	REG32(EBP) = DEC32(REG32(EBP) );
	CYCLES(CYCLES_DEC_REG);
}

void i386_device::i386_dec_esi()           // Opcode 0x4e
{
	REG32(ESI) = DEC32(REG32(ESI) );
	CYCLES(CYCLES_DEC_REG);
}

void i386_device::i386_dec_edi()           // Opcode 0x4f
{
	REG32(EDI) = DEC32(REG32(EDI) );
	CYCLES(CYCLES_DEC_REG);
}

void i386_device::i386_imul_r32_rm32()     // Opcode 0x0f af
{
	UINT8 modrm = FETCH();
	INT64 result;
	INT64 src, dst;
	if( modrm >= 0xc0 ) {
		src = (INT64)(INT32)LOAD_RM32(modrm);
		CYCLES(CYCLES_IMUL32_REG_REG);     /* TODO: Correct multiply timing */
	} else {
		UINT32 ea = GetEA(modrm,0);
		src = (INT64)(INT32)READ32(ea);
		CYCLES(CYCLES_IMUL32_REG_REG);     /* TODO: Correct multiply timing */
	}

	dst = (INT64)(INT32)LOAD_REG32(modrm);
	result = src * dst;

	STORE_REG32(modrm, (UINT32)result);

	m_CF = m_OF = !(result == (INT64)(INT32)result);
}

void i386_device::i386_imul_r32_rm32_i32() // Opcode 0x69
{
	UINT8 modrm = FETCH();
	INT64 result;
	INT64 src, dst;
	if( modrm >= 0xc0 ) {
		dst = (INT64)(INT32)LOAD_RM32(modrm);
		CYCLES(CYCLES_IMUL32_REG_IMM_REG);     /* TODO: Correct multiply timing */
	} else {
		UINT32 ea = GetEA(modrm,0);
		dst = (INT64)(INT32)READ32(ea);
		CYCLES(CYCLES_IMUL32_MEM_IMM_REG);     /* TODO: Correct multiply timing */
	}

	src = (INT64)(INT32)FETCH32();
	result = src * dst;

	STORE_REG32(modrm, (UINT32)result);

	m_CF = m_OF = !(result == (INT64)(INT32)result);
}

void i386_device::i386_imul_r32_rm32_i8()  // Opcode 0x6b
{
	UINT8 modrm = FETCH();
	INT64 result;
	INT64 src, dst;
	if( modrm >= 0xc0 ) {
		dst = (INT64)(INT32)LOAD_RM32(modrm);
		CYCLES(CYCLES_IMUL32_REG_IMM_REG);     /* TODO: Correct multiply timing */
	} else {
		UINT32 ea = GetEA(modrm,0);
		dst = (INT64)(INT32)READ32(ea);
		CYCLES(CYCLES_IMUL32_MEM_IMM_REG);     /* TODO: Correct multiply timing */
	}

	src = (INT64)(INT8)FETCH();
	result = src * dst;

	STORE_REG32(modrm, (UINT32)result);

	m_CF = m_OF = !(result == (INT64)(INT32)result);
}

void i386_device::i386_in_eax_i8()         // Opcode 0xe5
{
	UINT16 port = FETCH();
	UINT32 data = READPORT32(port);
	REG32(EAX) = data;
	CYCLES(CYCLES_IN_VAR);
}

void i386_device::i386_in_eax_dx()         // Opcode 0xed
{
	UINT16 port = REG16(DX);
	UINT32 data = READPORT32(port);
	REG32(EAX) = data;
	CYCLES(CYCLES_IN);
}

void i386_device::i386_inc_eax()           // Opcode 0x40
{
	REG32(EAX) = INC32(REG32(EAX) );
	CYCLES(CYCLES_INC_REG);
}

void i386_device::i386_inc_ecx()           // Opcode 0x41
{
	REG32(ECX) = INC32(REG32(ECX) );
	CYCLES(CYCLES_INC_REG);
}

void i386_device::i386_inc_edx()           // Opcode 0x42
{
	REG32(EDX) = INC32(REG32(EDX) );
	CYCLES(CYCLES_INC_REG);
}

void i386_device::i386_inc_ebx()           // Opcode 0x43
{
	REG32(EBX) = INC32(REG32(EBX) );
	CYCLES(CYCLES_INC_REG);
}

void i386_device::i386_inc_esp()           // Opcode 0x44
{
	REG32(ESP) = INC32(REG32(ESP) );
	CYCLES(CYCLES_INC_REG);
}

void i386_device::i386_inc_ebp()           // Opcode 0x45
{
	REG32(EBP) = INC32(REG32(EBP) );
	CYCLES(CYCLES_INC_REG);
}

void i386_device::i386_inc_esi()           // Opcode 0x46
{
	REG32(ESI) = INC32(REG32(ESI) );
	CYCLES(CYCLES_INC_REG);
}

void i386_device::i386_inc_edi()           // Opcode 0x47
{
	REG32(EDI) = INC32(REG32(EDI) );
	CYCLES(CYCLES_INC_REG);
}

void i386_device::i386_iret32()            // Opcode 0xcf
{
	if( PROTECTED_MODE )
	{
		i386_protected_mode_iret(1);
	}
	else
	{
		/* TODO: #SS(0) exception */
		/* TODO: #GP(0) exception */
		m_eip = POP32();
		m_sreg[CS].selector = POP32() & 0xffff;
		set_flags(POP32() );
		i386_load_segment_descriptor(CS);
		CHANGE_PC(m_eip);
	}
	CYCLES(CYCLES_IRET);
}

void i386_device::i386_ja_rel32()          // Opcode 0x0f 87
{
	INT32 disp = FETCH32();
	if( m_CF == 0 && m_ZF == 0 ) {
		m_eip += disp;
		CHANGE_PC(m_eip);
		CYCLES(CYCLES_JCC_FULL_DISP);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

void i386_device::i386_jbe_rel32()         // Opcode 0x0f 86
{
	INT32 disp = FETCH32();
	if( m_CF != 0 || m_ZF != 0 ) {
		m_eip += disp;
		CHANGE_PC(m_eip);
		CYCLES(CYCLES_JCC_FULL_DISP);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

void i386_device::i386_jc_rel32()          // Opcode 0x0f 82
{
	INT32 disp = FETCH32();
	if( m_CF != 0 ) {
		m_eip += disp;
		CHANGE_PC(m_eip);
		CYCLES(CYCLES_JCC_FULL_DISP);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

void i386_device::i386_jg_rel32()          // Opcode 0x0f 8f
{
	INT32 disp = FETCH32();
	if( m_ZF == 0 && (m_SF == m_OF) ) {
		m_eip += disp;
		CHANGE_PC(m_eip);
		CYCLES(CYCLES_JCC_FULL_DISP);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

void i386_device::i386_jge_rel32()         // Opcode 0x0f 8d
{
	INT32 disp = FETCH32();
	if(m_SF == m_OF) {
		m_eip += disp;
		CHANGE_PC(m_eip);
		CYCLES(CYCLES_JCC_FULL_DISP);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

void i386_device::i386_jl_rel32()          // Opcode 0x0f 8c
{
	INT32 disp = FETCH32();
	if( (m_SF != m_OF) ) {
		m_eip += disp;
		CHANGE_PC(m_eip);
		CYCLES(CYCLES_JCC_FULL_DISP);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

void i386_device::i386_jle_rel32()         // Opcode 0x0f 8e
{
	INT32 disp = FETCH32();
	if( m_ZF != 0 || (m_SF != m_OF) ) {
		m_eip += disp;
		CHANGE_PC(m_eip);
		CYCLES(CYCLES_JCC_FULL_DISP);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

void i386_device::i386_jnc_rel32()         // Opcode 0x0f 83
{
	INT32 disp = FETCH32();
	if( m_CF == 0 ) {
		m_eip += disp;
		CHANGE_PC(m_eip);
		CYCLES(CYCLES_JCC_FULL_DISP);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

void i386_device::i386_jno_rel32()         // Opcode 0x0f 81
{
	INT32 disp = FETCH32();
	if( m_OF == 0 ) {
		m_eip += disp;
		CHANGE_PC(m_eip);
		CYCLES(CYCLES_JCC_FULL_DISP);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

void i386_device::i386_jnp_rel32()         // Opcode 0x0f 8b
{
	INT32 disp = FETCH32();
	if( m_PF == 0 ) {
		m_eip += disp;
		CHANGE_PC(m_eip);
		CYCLES(CYCLES_JCC_FULL_DISP);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

void i386_device::i386_jns_rel32()         // Opcode 0x0f 89
{
	INT32 disp = FETCH32();
	if( m_SF == 0 ) {
		m_eip += disp;
		CHANGE_PC(m_eip);
		CYCLES(CYCLES_JCC_FULL_DISP);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

void i386_device::i386_jnz_rel32()         // Opcode 0x0f 85
{
	INT32 disp = FETCH32();
	if( m_ZF == 0 ) {
		m_eip += disp;
		CHANGE_PC(m_eip);
		CYCLES(CYCLES_JCC_FULL_DISP);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

void i386_device::i386_jo_rel32()          // Opcode 0x0f 80
{
	INT32 disp = FETCH32();
	if( m_OF != 0 ) {
		m_eip += disp;
		CHANGE_PC(m_eip);
		CYCLES(CYCLES_JCC_FULL_DISP);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

void i386_device::i386_jp_rel32()          // Opcode 0x0f 8a
{
	INT32 disp = FETCH32();
	if( m_PF != 0 ) {
		m_eip += disp;
		CHANGE_PC(m_eip);
		CYCLES(CYCLES_JCC_FULL_DISP);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

void i386_device::i386_js_rel32()          // Opcode 0x0f 88
{
	INT32 disp = FETCH32();
	if( m_SF != 0 ) {
		m_eip += disp;
		CHANGE_PC(m_eip);
		CYCLES(CYCLES_JCC_FULL_DISP);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

void i386_device::i386_jz_rel32()          // Opcode 0x0f 84
{
	INT32 disp = FETCH32();
	if( m_ZF != 0 ) {
		m_eip += disp;
		CHANGE_PC(m_eip);
		CYCLES(CYCLES_JCC_FULL_DISP);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

void i386_device::i386_jcxz32()            // Opcode 0xe3
{
	INT8 disp = FETCH();
	int val = (m_address_size)?(REG32(ECX) == 0):(REG16(CX) == 0);
	if( val ) {
		m_eip += disp;
		CHANGE_PC(m_eip);
		CYCLES(CYCLES_JCXZ);       /* TODO: Timing = 9 + m */
	} else {
		CYCLES(CYCLES_JCXZ_NOBRANCH);
	}
}

void i386_device::i386_jmp_rel32()         // Opcode 0xe9
{
	UINT32 disp = FETCH32();
	/* TODO: Segment limit */
	m_eip += disp;
	CHANGE_PC(m_eip);
	CYCLES(CYCLES_JMP);        /* TODO: Timing = 7 + m */
}

void i386_device::i386_jmp_abs32()         // Opcode 0xea
{
	UINT32 address = FETCH32();
	UINT16 segment = FETCH16();

	if( PROTECTED_MODE && !V8086_MODE)
	{
		i386_protected_mode_jump(segment,address,0,1);
	}
	else
	{
		m_eip = address;
		m_sreg[CS].selector = segment;
		m_performed_intersegment_jump = 1;
		i386_load_segment_descriptor(CS);
		CHANGE_PC(m_eip);
	}
	CYCLES(CYCLES_JMP_INTERSEG);
}

void i386_device::i386_lea32()             // Opcode 0x8d
{
	UINT8 modrm = FETCH();
	UINT32 ea = GetNonTranslatedEA(modrm,nullptr);
	if (!m_address_size)
	{
		ea &= 0xffff;
	}
	STORE_REG32(modrm, ea);
	CYCLES(CYCLES_LEA);
}

void i386_device::i386_enter32()           // Opcode 0xc8
{
	UINT16 framesize = FETCH16();
	UINT8 level = FETCH() % 32;
	UINT8 x;
	UINT32 frameptr;
	PUSH32(REG32(EBP));
	if(!STACK_32BIT)
		frameptr = REG16(SP);
	else
		frameptr = REG32(ESP);

	if(level > 0)
	{
		for(x=1;x<level-1;x++)
		{
			REG32(EBP) -= 4;
			PUSH32(READ32(REG32(EBP)));
		}
		PUSH32(frameptr);
	}
	REG32(EBP) = frameptr;
	if(!STACK_32BIT)
		REG16(SP) -= framesize;
	else
		REG32(ESP) -= framesize;
	CYCLES(CYCLES_ENTER);
}

void i386_device::i386_leave32()           // Opcode 0xc9
{
	if(!STACK_32BIT)
		REG16(SP) = REG16(BP);
	else
		REG32(ESP) = REG32(EBP);
	REG32(EBP) = POP32();
	CYCLES(CYCLES_LEAVE);
}

void i386_device::i386_lodsd()             // Opcode 0xad
{
	UINT32 eas;
	if( m_segment_prefix ) {
		eas = i386_translate(m_segment_override, m_address_size ? REG32(ESI) : REG16(SI), 0 );
	} else {
		eas = i386_translate(DS, m_address_size ? REG32(ESI) : REG16(SI), 0 );
	}
	REG32(EAX) = READ32(eas);
	BUMP_SI(4);
	CYCLES(CYCLES_LODS);
}

void i386_device::i386_loop32()            // Opcode 0xe2
{
	INT8 disp = FETCH();
	INT32 reg = (m_address_size)?--REG32(ECX):--REG16(CX);
	if( reg != 0 ) {
		m_eip += disp;
		CHANGE_PC(m_eip);
	}
	CYCLES(CYCLES_LOOP);       /* TODO: Timing = 11 + m */
}

void i386_device::i386_loopne32()          // Opcode 0xe0
{
	INT8 disp = FETCH();
	INT32 reg = (m_address_size)?--REG32(ECX):--REG16(CX);
	if( reg != 0 && m_ZF == 0 ) {
		m_eip += disp;
		CHANGE_PC(m_eip);
	}
	CYCLES(CYCLES_LOOPNZ);     /* TODO: Timing = 11 + m */
}

void i386_device::i386_loopz32()           // Opcode 0xe1
{
	INT8 disp = FETCH();
	INT32 reg = (m_address_size)?--REG32(ECX):--REG16(CX);
	if( reg != 0 && m_ZF != 0 ) {
		m_eip += disp;
		CHANGE_PC(m_eip);
	}
	CYCLES(CYCLES_LOOPZ);      /* TODO: Timing = 11 + m */
}

void i386_device::i386_mov_rm32_r32()      // Opcode 0x89
{
	UINT32 src;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_REG32(modrm);
		STORE_RM32(modrm, src);
		CYCLES(CYCLES_MOV_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm,1);
		src = LOAD_REG32(modrm);
		WRITE32(ea, src);
		CYCLES(CYCLES_MOV_REG_MEM);
	}
}

void i386_device::i386_mov_r32_rm32()      // Opcode 0x8b
{
	UINT32 src;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_RM32(modrm);
		STORE_REG32(modrm, src);
		CYCLES(CYCLES_MOV_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm,0);
		src = READ32(ea);
		STORE_REG32(modrm, src);
		CYCLES(CYCLES_MOV_MEM_REG);
	}
}

void i386_device::i386_mov_rm32_i32()      // Opcode 0xc7
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		UINT32 value = FETCH32();
		STORE_RM32(modrm, value);
		CYCLES(CYCLES_MOV_IMM_REG);
	} else {
		UINT32 ea = GetEA(modrm,1);
		UINT32 value = FETCH32();
		WRITE32(ea, value);
		CYCLES(CYCLES_MOV_IMM_MEM);
	}
}

void i386_device::i386_mov_eax_m32()       // Opcode 0xa1
{
	UINT32 offset, ea;
	if( m_address_size ) {
		offset = FETCH32();
	} else {
		offset = FETCH16();
	}
	if( m_segment_prefix ) {
		ea = i386_translate(m_segment_override, offset, 0 );
	} else {
		ea = i386_translate(DS, offset, 0 );
	}
	REG32(EAX) = READ32(ea);
	CYCLES(CYCLES_MOV_MEM_ACC);
}

void i386_device::i386_mov_m32_eax()       // Opcode 0xa3
{
	UINT32 offset, ea;
	if( m_address_size ) {
		offset = FETCH32();
	} else {
		offset = FETCH16();
	}
	if( m_segment_prefix ) {
		ea = i386_translate(m_segment_override, offset, 1 );
	} else {
		ea = i386_translate(DS, offset, 1 );
	}
	WRITE32(ea, REG32(EAX) );
	CYCLES(CYCLES_MOV_ACC_MEM);
}

void i386_device::i386_mov_eax_i32()       // Opcode 0xb8
{
	REG32(EAX) = FETCH32();
	CYCLES(CYCLES_MOV_IMM_REG);
}

void i386_device::i386_mov_ecx_i32()       // Opcode 0xb9
{
	REG32(ECX) = FETCH32();
	CYCLES(CYCLES_MOV_IMM_REG);
}

void i386_device::i386_mov_edx_i32()       // Opcode 0xba
{
	REG32(EDX) = FETCH32();
	CYCLES(CYCLES_MOV_IMM_REG);
}

void i386_device::i386_mov_ebx_i32()       // Opcode 0xbb
{
	REG32(EBX) = FETCH32();
	CYCLES(CYCLES_MOV_IMM_REG);
}

void i386_device::i386_mov_esp_i32()       // Opcode 0xbc
{
	REG32(ESP) = FETCH32();
	CYCLES(CYCLES_MOV_IMM_REG);
}

void i386_device::i386_mov_ebp_i32()       // Opcode 0xbd
{
	REG32(EBP) = FETCH32();
	CYCLES(CYCLES_MOV_IMM_REG);
}

void i386_device::i386_mov_esi_i32()       // Opcode 0xbe
{
	REG32(ESI) = FETCH32();
	CYCLES(CYCLES_MOV_IMM_REG);
}

void i386_device::i386_mov_edi_i32()       // Opcode 0xbf
{
	REG32(EDI) = FETCH32();
	CYCLES(CYCLES_MOV_IMM_REG);
}

void i386_device::i386_movsd()             // Opcode 0xa5
{
	UINT32 eas, ead, v;
	if( m_segment_prefix ) {
		eas = i386_translate(m_segment_override, m_address_size ? REG32(ESI) : REG16(SI), 0 );
	} else {
		eas = i386_translate(DS, m_address_size ? REG32(ESI) : REG16(SI), 0 );
	}
	ead = i386_translate(ES, m_address_size ? REG32(EDI) : REG16(DI), 1 );
	v = READ32(eas);
	WRITE32(ead, v);
	BUMP_SI(4);
	BUMP_DI(4);
	CYCLES(CYCLES_MOVS);
}

void i386_device::i386_movsx_r32_rm8()     // Opcode 0x0f be
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		INT32 src = (INT8)LOAD_RM8(modrm);
		STORE_REG32(modrm, src);
		CYCLES(CYCLES_MOVSX_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm,0);
		INT32 src = (INT8)READ8(ea);
		STORE_REG32(modrm, src);
		CYCLES(CYCLES_MOVSX_MEM_REG);
	}
}

void i386_device::i386_movsx_r32_rm16()    // Opcode 0x0f bf
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		INT32 src = (INT16)LOAD_RM16(modrm);
		STORE_REG32(modrm, src);
		CYCLES(CYCLES_MOVSX_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm,0);
		INT32 src = (INT16)READ16(ea);
		STORE_REG32(modrm, src);
		CYCLES(CYCLES_MOVSX_MEM_REG);
	}
}

void i386_device::i386_movzx_r32_rm8()     // Opcode 0x0f b6
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		UINT32 src = (UINT8)LOAD_RM8(modrm);
		STORE_REG32(modrm, src);
		CYCLES(CYCLES_MOVZX_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm,0);
		UINT32 src = (UINT8)READ8(ea);
		STORE_REG32(modrm, src);
		CYCLES(CYCLES_MOVZX_MEM_REG);
	}
}

void i386_device::i386_movzx_r32_rm16()    // Opcode 0x0f b7
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		UINT32 src = (UINT16)LOAD_RM16(modrm);
		STORE_REG32(modrm, src);
		CYCLES(CYCLES_MOVZX_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm,0);
		UINT32 src = (UINT16)READ16(ea);
		STORE_REG32(modrm, src);
		CYCLES(CYCLES_MOVZX_MEM_REG);
	}
}

void i386_device::i386_or_rm32_r32()       // Opcode 0x09
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
		UINT32 ea = GetEA(modrm,1);
		src = LOAD_REG32(modrm);
		dst = READ32(ea);
		dst = OR32(dst, src);
		WRITE32(ea, dst);
		CYCLES(CYCLES_ALU_REG_MEM);
	}
}

void i386_device::i386_or_r32_rm32()       // Opcode 0x0b
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
		UINT32 ea = GetEA(modrm,0);
		src = READ32(ea);
		dst = LOAD_REG32(modrm);
		dst = OR32(dst, src);
		STORE_REG32(modrm, dst);
		CYCLES(CYCLES_ALU_MEM_REG);
	}
}

void i386_device::i386_or_eax_i32()        // Opcode 0x0d
{
	UINT32 src, dst;
	src = FETCH32();
	dst = REG32(EAX);
	dst = OR32(dst, src);
	REG32(EAX) = dst;
	CYCLES(CYCLES_ALU_IMM_ACC);
}

void i386_device::i386_out_eax_i8()        // Opcode 0xe7
{
	UINT16 port = FETCH();
	UINT32 data = REG32(EAX);
	WRITEPORT32(port, data);
	CYCLES(CYCLES_OUT_VAR);
}

void i386_device::i386_out_eax_dx()        // Opcode 0xef
{
	UINT16 port = REG16(DX);
	UINT32 data = REG32(EAX);
	WRITEPORT32(port, data);
	CYCLES(CYCLES_OUT);
}

void i386_device::i386_pop_eax()           // Opcode 0x58
{
	UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(SS,offset+3) == 0)
		REG32(EAX) = POP32();
	else
		FAULT(FAULT_SS,0)
	CYCLES(CYCLES_POP_REG_SHORT);
}

void i386_device::i386_pop_ecx()           // Opcode 0x59
{
	UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(SS,offset+3) == 0)
		REG32(ECX) = POP32();
	else
		FAULT(FAULT_SS,0)
	CYCLES(CYCLES_POP_REG_SHORT);
}

void i386_device::i386_pop_edx()           // Opcode 0x5a
{
	UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(SS,offset+3) == 0)
		REG32(EDX) = POP32();
	else
		FAULT(FAULT_SS,0)
	CYCLES(CYCLES_POP_REG_SHORT);
}

void i386_device::i386_pop_ebx()           // Opcode 0x5b
{
	UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(SS,offset+3) == 0)
		REG32(EBX) = POP32();
	else
		FAULT(FAULT_SS,0)
	CYCLES(CYCLES_POP_REG_SHORT);
}

void i386_device::i386_pop_esp()           // Opcode 0x5c
{
	UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(SS,offset+3) == 0)
		REG32(ESP) = POP32();
	else
		FAULT(FAULT_SS,0)
	CYCLES(CYCLES_POP_REG_SHORT);
}

void i386_device::i386_pop_ebp()           // Opcode 0x5d
{
	UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(SS,offset+3) == 0)
		REG32(EBP) = POP32();
	else
		FAULT(FAULT_SS,0)
	CYCLES(CYCLES_POP_REG_SHORT);
}

void i386_device::i386_pop_esi()           // Opcode 0x5e
{
	UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(SS,offset+3) == 0)
		REG32(ESI) = POP32();
	else
		FAULT(FAULT_SS,0)
	CYCLES(CYCLES_POP_REG_SHORT);
}

void i386_device::i386_pop_edi()           // Opcode 0x5f
{
	UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(SS,offset+3) == 0)
		REG32(EDI) = POP32();
	else
		FAULT(FAULT_SS,0)
	CYCLES(CYCLES_POP_REG_SHORT);
}

bool i386_device::i386_pop_seg32(int segment)
{
	UINT32 ea, offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	UINT32 value;
	bool fault;
	if(i386_limit_check(SS,offset+3) == 0)
	{
		ea = i386_translate(SS, offset, 0);
		value = READ32(ea);
		i386_sreg_load(value, segment, &fault);
		if(fault) return false;
		if(STACK_32BIT)
			REG32(ESP) = offset + 4;
		else
			REG16(SP) = offset + 4;
	}
	else
	{
		m_ext = 1;
		i386_trap_with_error(FAULT_SS,0,0,0);
		return false;
	}
	CYCLES(CYCLES_POP_SREG);
	return true;
}

void i386_device::i386_pop_ds32()          // Opcode 0x1f
{
	i386_pop_seg32(DS);
}

void i386_device::i386_pop_es32()          // Opcode 0x07
{
	i386_pop_seg32(ES);
}

void i386_device::i386_pop_fs32()          // Opcode 0x0f a1
{
	i386_pop_seg32(FS);
}

void i386_device::i386_pop_gs32()          // Opcode 0x0f a9
{
	i386_pop_seg32(GS);
}

void i386_device::i386_pop_ss32()          // Opcode 0x17
{
	if(!i386_pop_seg32(SS)) return;
	if(m_IF != 0) // if external interrupts are enabled
	{
		m_IF = 0;  // reset IF for the next instruction
		m_delayed_interrupt_enable = 1;
	}
}

void i386_device::i386_pop_rm32()          // Opcode 0x8f
{
	UINT8 modrm = FETCH();
	UINT32 value;
	UINT32 ea, offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(SS,offset+3) == 0)
	{
		// be careful here, if the write references the esp register
		// it expects the post-pop value but esp must be wound back
		// if the write faults
		UINT32 temp_sp = REG32(ESP);
		value = POP32();

		if( modrm >= 0xc0 ) {
			STORE_RM32(modrm, value);
		} else {
			ea = GetEA(modrm,1);
			try
			{
				WRITE32(ea, value);
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
	CYCLES(CYCLES_POP_RM);
}

void i386_device::i386_popad()             // Opcode 0x61
{
	UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(SS,offset+31) == 0)
	{
		REG32(EDI) = POP32();
		REG32(ESI) = POP32();
		REG32(EBP) = POP32();
		REG32(ESP) += 4;
		REG32(EBX) = POP32();
		REG32(EDX) = POP32();
		REG32(ECX) = POP32();
		REG32(EAX) = POP32();
	}
	else
		FAULT(FAULT_SS,0)
	CYCLES(CYCLES_POPA);
}

void i386_device::i386_popfd()             // Opcode 0x9d
{
	UINT32 value;
	UINT32 current = get_flags();
	UINT8 IOPL = (current >> 12) & 0x03;
	UINT32 mask = 0x00257fd5;  // VM, VIP and VIF cannot be set by POPF/POPFD
	UINT32 offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));

	// IOPL can only change if CPL is 0
	if(m_CPL != 0)
		mask &= ~0x00003000;

	// IF can only change if CPL is at least as privileged as IOPL
	if(m_CPL > IOPL)
		mask &= ~0x00000200;

	if(V8086_MODE)
	{
		if(IOPL < 3)
		{
			logerror("POPFD(%08x): IOPL < 3 while in V86 mode.\n",m_pc);
			FAULT(FAULT_GP,0)  // #GP(0)
		}
		mask &= ~0x00003000;  // IOPL cannot be changed while in V8086 mode
	}

	if(i386_limit_check(SS,offset+3) == 0)
	{
		value = POP32();
		value &= ~0x00010000;  // RF will always return zero
		set_flags((current & ~mask) | (value & mask));  // mask out reserved bits
	}
	else
		FAULT(FAULT_SS,0)
	CYCLES(CYCLES_POPF);
}

void i386_device::i386_push_eax()          // Opcode 0x50
{
	UINT32 offset;
	if(STACK_32BIT)
		offset = REG32(ESP) - 4;
	else
		offset = (REG16(SP) - 4) & 0xffff;
	if(i386_limit_check(SS,offset) == 0)
		PUSH32(REG32(EAX) );
	else
		FAULT(FAULT_SS,0)
	CYCLES(CYCLES_PUSH_REG_SHORT);
}

void i386_device::i386_push_ecx()          // Opcode 0x51
{
	UINT32 offset;
	if(STACK_32BIT)
		offset = REG32(ESP) - 4;
	else
		offset = (REG16(SP) - 4) & 0xffff;
	if(i386_limit_check(SS,offset) == 0)
		PUSH32(REG32(ECX) );
	else
		FAULT(FAULT_SS,0)
	CYCLES(CYCLES_PUSH_REG_SHORT);
}

void i386_device::i386_push_edx()          // Opcode 0x52
{
	UINT32 offset;
	if(STACK_32BIT)
		offset = REG32(ESP) - 4;
	else
		offset = (REG16(SP) - 4) & 0xffff;
	if(i386_limit_check(SS,offset) == 0)
		PUSH32(REG32(EDX) );
	else
		FAULT(FAULT_SS,0)
	CYCLES(CYCLES_PUSH_REG_SHORT);
}

void i386_device::i386_push_ebx()          // Opcode 0x53
{
	UINT32 offset;
	if(STACK_32BIT)
		offset = REG32(ESP) - 4;
	else
		offset = (REG16(SP) - 4) & 0xffff;
	if(i386_limit_check(SS,offset) == 0)
		PUSH32(REG32(EBX) );
	else
		FAULT(FAULT_SS,0)
	CYCLES(CYCLES_PUSH_REG_SHORT);
}

void i386_device::i386_push_esp()          // Opcode 0x54
{
	UINT32 offset;
	if(STACK_32BIT)
		offset = REG32(ESP) - 4;
	else
		offset = (REG16(SP) - 4) & 0xffff;
	if(i386_limit_check(SS,offset) == 0)
		PUSH32(REG32(ESP) );
	else
		FAULT(FAULT_SS,0)
	CYCLES(CYCLES_PUSH_REG_SHORT);
}

void i386_device::i386_push_ebp()          // Opcode 0x55
{
	UINT32 offset;
	if(STACK_32BIT)
		offset = REG32(ESP) - 4;
	else
		offset = (REG16(SP) - 4) & 0xffff;
	if(i386_limit_check(SS,offset) == 0)
		PUSH32(REG32(EBP) );
	else
		FAULT(FAULT_SS,0)
	CYCLES(CYCLES_PUSH_REG_SHORT);
}

void i386_device::i386_push_esi()          // Opcode 0x56
{
	UINT32 offset;
	if(STACK_32BIT)
		offset = REG32(ESP) - 4;
	else
		offset = (REG16(SP) - 4) & 0xffff;
	if(i386_limit_check(SS,offset) == 0)
		PUSH32(REG32(ESI) );
	else
		FAULT(FAULT_SS,0)
	CYCLES(CYCLES_PUSH_REG_SHORT);
}

void i386_device::i386_push_edi()          // Opcode 0x57
{
	UINT32 offset;
	if(STACK_32BIT)
		offset = REG32(ESP) - 4;
	else
		offset = (REG16(SP) - 4) & 0xffff;
	if(i386_limit_check(SS,offset) == 0)
		PUSH32(REG32(EDI) );
	else
		FAULT(FAULT_SS,0)
	CYCLES(CYCLES_PUSH_REG_SHORT);
}

void i386_device::i386_push_cs32()         // Opcode 0x0e
{
	UINT32 offset;
	if(STACK_32BIT)
		offset = REG32(ESP) - 4;
	else
		offset = (REG16(SP) - 4) & 0xffff;
	if(i386_limit_check(SS,offset) == 0)
		PUSH32(m_sreg[CS].selector );
	else
		FAULT(FAULT_SS,0)
	CYCLES(CYCLES_PUSH_SREG);
}

void i386_device::i386_push_ds32()         // Opcode 0x1e
{
	UINT32 offset;
	if(STACK_32BIT)
		offset = REG32(ESP) - 4;
	else
		offset = (REG16(SP) - 4) & 0xffff;
	if(i386_limit_check(SS,offset) == 0)
		PUSH32(m_sreg[DS].selector );
	else
		FAULT(FAULT_SS,0)
	CYCLES(CYCLES_PUSH_SREG);
}

void i386_device::i386_push_es32()         // Opcode 0x06
{
	UINT32 offset;
	if(STACK_32BIT)
		offset = REG32(ESP) - 4;
	else
		offset = (REG16(SP) - 4) & 0xffff;
	if(i386_limit_check(SS,offset) == 0)
		PUSH32(m_sreg[ES].selector );
	else
		FAULT(FAULT_SS,0)
	CYCLES(CYCLES_PUSH_SREG);
}

void i386_device::i386_push_fs32()         // Opcode 0x0f a0
{
	UINT32 offset;
	if(STACK_32BIT)
		offset = REG32(ESP) - 4;
	else
		offset = (REG16(SP) - 4) & 0xffff;
	if(i386_limit_check(SS,offset) == 0)
		PUSH32(m_sreg[FS].selector );
	else
		FAULT(FAULT_SS,0)
	CYCLES(CYCLES_PUSH_SREG);
}

void i386_device::i386_push_gs32()         // Opcode 0x0f a8
{
	UINT32 offset;
	if(STACK_32BIT)
		offset = REG32(ESP) - 4;
	else
		offset = (REG16(SP) - 4) & 0xffff;
	if(i386_limit_check(SS,offset) == 0)
		PUSH32(m_sreg[GS].selector );
	else
		FAULT(FAULT_SS,0)
	CYCLES(CYCLES_PUSH_SREG);
}

void i386_device::i386_push_ss32()         // Opcode 0x16
{
	UINT32 offset;
	if(STACK_32BIT)
		offset = REG32(ESP) - 4;
	else
		offset = (REG16(SP) - 4) & 0xffff;
	if(i386_limit_check(SS,offset) == 0)
		PUSH32(m_sreg[SS].selector );
	else
		FAULT(FAULT_SS,0)
	CYCLES(CYCLES_PUSH_SREG);
}

void i386_device::i386_push_i32()          // Opcode 0x68
{
	UINT32 value = FETCH32();
	UINT32 offset;
	if(STACK_32BIT)
		offset = REG32(ESP) - 4;
	else
		offset = (REG16(SP) - 4) & 0xffff;
	if(i386_limit_check(SS,offset) == 0)
		PUSH32(value);
	else
		FAULT(FAULT_SS,0)
	CYCLES(CYCLES_PUSH_IMM);
}

void i386_device::i386_pushad()            // Opcode 0x60
{
	UINT32 temp = REG32(ESP);
	UINT32 offset;
	if(STACK_32BIT)
		offset = REG32(ESP) - 32;
	else
		offset = (REG16(SP) - 32) & 0xffff;
	if(i386_limit_check(SS,offset) == 0)
	{
		PUSH32(REG32(EAX) );
		PUSH32(REG32(ECX) );
		PUSH32(REG32(EDX) );
		PUSH32(REG32(EBX) );
		PUSH32(temp );
		PUSH32(REG32(EBP) );
		PUSH32(REG32(ESI) );
		PUSH32(REG32(EDI) );
	}
	else
		FAULT(FAULT_SS,0)
	CYCLES(CYCLES_PUSHA);
}

void i386_device::i386_pushfd()            // Opcode 0x9c
{
	if(!m_IOP1 && !m_IOP2 && V8086_MODE)
		FAULT(FAULT_GP,0)
		UINT32 offset;
		if(STACK_32BIT)
			offset = REG32(ESP) - 4;
		else
			offset = (REG16(SP) - 4) & 0xffff;
	if(i386_limit_check(SS,offset) == 0)
		PUSH32(get_flags() & 0x00fcffff );
	else
		FAULT(FAULT_SS,0)
	CYCLES(CYCLES_PUSHF);
}

void i386_device::i386_ret_near32_i16()    // Opcode 0xc2
{
	INT16 disp = FETCH16();
	m_eip = POP32();
	REG32(ESP) += disp;
	CHANGE_PC(m_eip);
	CYCLES(CYCLES_RET_IMM);        /* TODO: Timing = 10 + m */
}

void i386_device::i386_ret_near32()        // Opcode 0xc3
{
	m_eip = POP32();
	CHANGE_PC(m_eip);
	CYCLES(CYCLES_RET);        /* TODO: Timing = 10 + m */
}

void i386_device::i386_sbb_rm32_r32()      // Opcode 0x19
{
	UINT32 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_REG32(modrm);
		dst = LOAD_RM32(modrm);
		dst = SBB32(dst, src, m_CF);
		STORE_RM32(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm,1);
		src = LOAD_REG32(modrm);
		dst = READ32(ea);
		dst = SBB32(dst, src, m_CF);
		WRITE32(ea, dst);
		CYCLES(CYCLES_ALU_REG_MEM);
	}
}

void i386_device::i386_sbb_r32_rm32()      // Opcode 0x1b
{
	UINT32 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_RM32(modrm);
		dst = LOAD_REG32(modrm);
		dst = SBB32(dst, src, m_CF);
		STORE_REG32(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm,0);
		src = READ32(ea);
		dst = LOAD_REG32(modrm);
		dst = SBB32(dst, src, m_CF);
		STORE_REG32(modrm, dst);
		CYCLES(CYCLES_ALU_MEM_REG);
	}
}

void i386_device::i386_sbb_eax_i32()       // Opcode 0x1d
{
	UINT32 src, dst;
	src = FETCH32();
	dst = REG32(EAX);
	dst = SBB32(dst, src, m_CF);
	REG32(EAX) = dst;
	CYCLES(CYCLES_ALU_IMM_ACC);
}

void i386_device::i386_scasd()             // Opcode 0xaf
{
	UINT32 eas, src, dst;
	eas = i386_translate(ES, m_address_size ? REG32(EDI) : REG16(DI), 0 );
	src = READ32(eas);
	dst = REG32(EAX);
	SUB32(dst, src);
	BUMP_DI(4);
	CYCLES(CYCLES_SCAS);
}

void i386_device::i386_shld32_i8()         // Opcode 0x0f a4
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		UINT32 dst = LOAD_RM32(modrm);
		UINT32 upper = LOAD_REG32(modrm);
		UINT8 shift = FETCH();
		shift &= 31;
		if( shift == 0 ) {
		} else {
			m_CF = (dst & (1 << (32-shift))) ? 1 : 0;
			dst = (dst << shift) | (upper >> (32-shift));
			m_OF = m_CF ^ (dst >> 31);
			SetSZPF32(dst);
		}
		STORE_RM32(modrm, dst);
		CYCLES(CYCLES_SHLD_REG);
	} else {
		UINT32 ea = GetEA(modrm,1);
		UINT32 dst = READ32(ea);
		UINT32 upper = LOAD_REG32(modrm);
		UINT8 shift = FETCH();
		shift &= 31;
		if( shift == 0 ) {
		} else {
			m_CF = (dst & (1 << (32-shift))) ? 1 : 0;
			dst = (dst << shift) | (upper >> (32-shift));
			m_OF = m_CF ^ (dst >> 31);
			SetSZPF32(dst);
		}
		WRITE32(ea, dst);
		CYCLES(CYCLES_SHLD_MEM);
	}
}

void i386_device::i386_shld32_cl()         // Opcode 0x0f a5
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		UINT32 dst = LOAD_RM32(modrm);
		UINT32 upper = LOAD_REG32(modrm);
		UINT8 shift = REG8(CL);
		shift &= 31;
		if( shift == 0 ) {
		} else {
			m_CF = (dst & (1 << (32-shift))) ? 1 : 0;
			dst = (dst << shift) | (upper >> (32-shift));
			m_OF = m_CF ^ (dst >> 31);
			SetSZPF32(dst);
		}
		STORE_RM32(modrm, dst);
		CYCLES(CYCLES_SHLD_REG);
	} else {
		UINT32 ea = GetEA(modrm,1);
		UINT32 dst = READ32(ea);
		UINT32 upper = LOAD_REG32(modrm);
		UINT8 shift = REG8(CL);
		shift &= 31;
		if( shift == 0 ) {
		} else {
			m_CF = (dst & (1 << (32-shift))) ? 1 : 0;
			dst = (dst << shift) | (upper >> (32-shift));
			m_OF = m_CF ^ (dst >> 31);
			SetSZPF32(dst);
		}
		WRITE32(ea, dst);
		CYCLES(CYCLES_SHLD_MEM);
	}
}

void i386_device::i386_shrd32_i8()         // Opcode 0x0f ac
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		UINT32 dst = LOAD_RM32(modrm);
		UINT32 upper = LOAD_REG32(modrm);
		UINT8 shift = FETCH();
		shift &= 31;
		if( shift == 0 ) {
		} else {
			m_CF = (dst & (1 << (shift-1))) ? 1 : 0;
			dst = (dst >> shift) | (upper << (32-shift));
			m_OF = ((dst >> 31) ^ (dst >> 30)) & 1;
			SetSZPF32(dst);
		}
		STORE_RM32(modrm, dst);
		CYCLES(CYCLES_SHRD_REG);
	} else {
		UINT32 ea = GetEA(modrm,1);
		UINT32 dst = READ32(ea);
		UINT32 upper = LOAD_REG32(modrm);
		UINT8 shift = FETCH();
		shift &= 31;
		if( shift == 0 ) {
		} else {
			m_CF = (dst & (1 << (shift-1))) ? 1 : 0;
			dst = (dst >> shift) | (upper << (32-shift));
			m_OF = ((dst >> 31) ^ (dst >> 30)) & 1;
			SetSZPF32(dst);
		}
		WRITE32(ea, dst);
		CYCLES(CYCLES_SHRD_MEM);
	}
}

void i386_device::i386_shrd32_cl()         // Opcode 0x0f ad
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		UINT32 dst = LOAD_RM32(modrm);
		UINT32 upper = LOAD_REG32(modrm);
		UINT8 shift = REG8(CL);
		shift &= 31;
		if( shift == 0 ) {
		} else {
			m_CF = (dst & (1 << (shift-1))) ? 1 : 0;
			dst = (dst >> shift) | (upper << (32-shift));
			m_OF = ((dst >> 31) ^ (dst >> 30)) & 1;
			SetSZPF32(dst);
		}
		STORE_RM32(modrm, dst);
		CYCLES(CYCLES_SHRD_REG);
	} else {
		UINT32 ea = GetEA(modrm,1);
		UINT32 dst = READ32(ea);
		UINT32 upper = LOAD_REG32(modrm);
		UINT8 shift = REG8(CL);
		shift &= 31;
		if( shift == 0 ) {
		} else {
			m_CF = (dst & (1 << (shift-1))) ? 1 : 0;
			dst = (dst >> shift) | (upper << (32-shift));
			m_OF = ((dst >> 31) ^ (dst >> 30)) & 1;
			SetSZPF32(dst);
		}
		WRITE32(ea, dst);
		CYCLES(CYCLES_SHRD_MEM);
	}
}

void i386_device::i386_stosd()             // Opcode 0xab
{
	UINT32 eas = i386_translate(ES, m_address_size ? REG32(EDI) : REG16(DI), 1 );
	WRITE32(eas, REG32(EAX));
	BUMP_DI(4);
	CYCLES(CYCLES_STOS);
}

void i386_device::i386_sub_rm32_r32()      // Opcode 0x29
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
		UINT32 ea = GetEA(modrm,1);
		src = LOAD_REG32(modrm);
		dst = READ32(ea);
		dst = SUB32(dst, src);
		WRITE32(ea, dst);
		CYCLES(CYCLES_ALU_REG_MEM);
	}
}

void i386_device::i386_sub_r32_rm32()      // Opcode 0x2b
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
		UINT32 ea = GetEA(modrm,1);
		src = READ32(ea);
		dst = LOAD_REG32(modrm);
		dst = SUB32(dst, src);
		STORE_REG32(modrm, dst);
		CYCLES(CYCLES_ALU_MEM_REG);
	}
}

void i386_device::i386_sub_eax_i32()       // Opcode 0x2d
{
	UINT32 src, dst;
	src = FETCH32();
	dst = REG32(EAX);
	dst = SUB32(dst, src);
	REG32(EAX) = dst;
	CYCLES(CYCLES_ALU_IMM_ACC);
}

void i386_device::i386_test_eax_i32()      // Opcode 0xa9
{
	UINT32 src = FETCH32();
	UINT32 dst = REG32(EAX);
	dst = src & dst;
	SetSZPF32(dst);
	m_CF = 0;
	m_OF = 0;
	CYCLES(CYCLES_TEST_IMM_ACC);
}

void i386_device::i386_test_rm32_r32()     // Opcode 0x85
{
	UINT32 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_REG32(modrm);
		dst = LOAD_RM32(modrm);
		dst = src & dst;
		SetSZPF32(dst);
		m_CF = 0;
		m_OF = 0;
		CYCLES(CYCLES_TEST_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm,0);
		src = LOAD_REG32(modrm);
		dst = READ32(ea);
		dst = src & dst;
		SetSZPF32(dst);
		m_CF = 0;
		m_OF = 0;
		CYCLES(CYCLES_TEST_REG_MEM);
	}
}

void i386_device::i386_xchg_eax_ecx()      // Opcode 0x91
{
	UINT32 temp;
	temp = REG32(EAX);
	REG32(EAX) = REG32(ECX);
	REG32(ECX) = temp;
	CYCLES(CYCLES_XCHG_REG_REG);
}

void i386_device::i386_xchg_eax_edx()      // Opcode 0x92
{
	UINT32 temp;
	temp = REG32(EAX);
	REG32(EAX) = REG32(EDX);
	REG32(EDX) = temp;
	CYCLES(CYCLES_XCHG_REG_REG);
}

void i386_device::i386_xchg_eax_ebx()      // Opcode 0x93
{
	UINT32 temp;
	temp = REG32(EAX);
	REG32(EAX) = REG32(EBX);
	REG32(EBX) = temp;
	CYCLES(CYCLES_XCHG_REG_REG);
}

void i386_device::i386_xchg_eax_esp()      // Opcode 0x94
{
	UINT32 temp;
	temp = REG32(EAX);
	REG32(EAX) = REG32(ESP);
	REG32(ESP) = temp;
	CYCLES(CYCLES_XCHG_REG_REG);
}

void i386_device::i386_xchg_eax_ebp()      // Opcode 0x95
{
	UINT32 temp;
	temp = REG32(EAX);
	REG32(EAX) = REG32(EBP);
	REG32(EBP) = temp;
	CYCLES(CYCLES_XCHG_REG_REG);
}

void i386_device::i386_xchg_eax_esi()      // Opcode 0x96
{
	UINT32 temp;
	temp = REG32(EAX);
	REG32(EAX) = REG32(ESI);
	REG32(ESI) = temp;
	CYCLES(CYCLES_XCHG_REG_REG);
}

void i386_device::i386_xchg_eax_edi()      // Opcode 0x97
{
	UINT32 temp;
	temp = REG32(EAX);
	REG32(EAX) = REG32(EDI);
	REG32(EDI) = temp;
	CYCLES(CYCLES_XCHG_REG_REG);
}

void i386_device::i386_xchg_r32_rm32()     // Opcode 0x87
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		UINT32 src = LOAD_RM32(modrm);
		UINT32 dst = LOAD_REG32(modrm);
		STORE_REG32(modrm, src);
		STORE_RM32(modrm, dst);
		CYCLES(CYCLES_XCHG_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm,1);
		UINT32 src = READ32(ea);
		UINT32 dst = LOAD_REG32(modrm);
		WRITE32(ea, dst);
		STORE_REG32(modrm, src);
		CYCLES(CYCLES_XCHG_REG_MEM);
	}
}

void i386_device::i386_xor_rm32_r32()      // Opcode 0x31
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
		UINT32 ea = GetEA(modrm,1);
		src = LOAD_REG32(modrm);
		dst = READ32(ea);
		dst = XOR32(dst, src);
		WRITE32(ea, dst);
		CYCLES(CYCLES_ALU_REG_MEM);
	}
}

void i386_device::i386_xor_r32_rm32()      // Opcode 0x33
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
		UINT32 ea = GetEA(modrm,0);
		src = READ32(ea);
		dst = LOAD_REG32(modrm);
		dst = XOR32(dst, src);
		STORE_REG32(modrm, dst);
		CYCLES(CYCLES_ALU_MEM_REG);
	}
}

void i386_device::i386_xor_eax_i32()       // Opcode 0x35
{
	UINT32 src, dst;
	src = FETCH32();
	dst = REG32(EAX);
	dst = XOR32(dst, src);
	REG32(EAX) = dst;
	CYCLES(CYCLES_ALU_IMM_ACC);
}



void i386_device::i386_group81_32()        // Opcode 0x81
{
	UINT32 ea;
	UINT32 src, dst;
	UINT8 modrm = FETCH();

	switch( (modrm >> 3) & 0x7 )
	{
		case 0:     // ADD Rm32, i32
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM32(modrm);
				src = FETCH32();
				dst = ADD32(dst, src);
				STORE_RM32(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm,1);
				dst = READ32(ea);
				src = FETCH32();
				dst = ADD32(dst, src);
				WRITE32(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 1:     // OR Rm32, i32
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM32(modrm);
				src = FETCH32();
				dst = OR32(dst, src);
				STORE_RM32(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm,1);
				dst = READ32(ea);
				src = FETCH32();
				dst = OR32(dst, src);
				WRITE32(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 2:     // ADC Rm32, i32
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM32(modrm);
				src = FETCH32();
				dst = ADC32(dst, src, m_CF);
				STORE_RM32(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm,1);
				dst = READ32(ea);
				src = FETCH32();
				dst = ADC32(dst, src, m_CF);
				WRITE32(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 3:     // SBB Rm32, i32
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM32(modrm);
				src = FETCH32();
				dst = SBB32(dst, src, m_CF);
				STORE_RM32(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm,1);
				dst = READ32(ea);
				src = FETCH32();
				dst = SBB32(dst, src, m_CF);
				WRITE32(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 4:     // AND Rm32, i32
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM32(modrm);
				src = FETCH32();
				dst = AND32(dst, src);
				STORE_RM32(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm,1);
				dst = READ32(ea);
				src = FETCH32();
				dst = AND32(dst, src);
				WRITE32(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 5:     // SUB Rm32, i32
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM32(modrm);
				src = FETCH32();
				dst = SUB32(dst, src);
				STORE_RM32(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm,1);
				dst = READ32(ea);
				src = FETCH32();
				dst = SUB32(dst, src);
				WRITE32(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 6:     // XOR Rm32, i32
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM32(modrm);
				src = FETCH32();
				dst = XOR32(dst, src);
				STORE_RM32(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm,1);
				dst = READ32(ea);
				src = FETCH32();
				dst = XOR32(dst, src);
				WRITE32(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 7:     // CMP Rm32, i32
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM32(modrm);
				src = FETCH32();
				SUB32(dst, src);
				CYCLES(CYCLES_CMP_REG_REG);
			} else {
				ea = GetEA(modrm,0);
				dst = READ32(ea);
				src = FETCH32();
				SUB32(dst, src);
				CYCLES(CYCLES_CMP_REG_MEM);
			}
			break;
	}
}

void i386_device::i386_group83_32()        // Opcode 0x83
{
	UINT32 ea;
	UINT32 src, dst;
	UINT8 modrm = FETCH();

	switch( (modrm >> 3) & 0x7 )
	{
		case 0:     // ADD Rm32, i32
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM32(modrm);
				src = (UINT32)(INT32)(INT8)FETCH();
				dst = ADD32(dst, src);
				STORE_RM32(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm,1);
				dst = READ32(ea);
				src = (UINT32)(INT32)(INT8)FETCH();
				dst = ADD32(dst, src);
				WRITE32(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 1:     // OR Rm32, i32
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM32(modrm);
				src = (UINT32)(INT32)(INT8)FETCH();
				dst = OR32(dst, src);
				STORE_RM32(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm,1);
				dst = READ32(ea);
				src = (UINT32)(INT32)(INT8)FETCH();
				dst = OR32(dst, src);
				WRITE32(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 2:     // ADC Rm32, i32
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM32(modrm);
				src = (UINT32)(INT32)(INT8)FETCH();
				dst = ADC32(dst, src, m_CF);
				STORE_RM32(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm,1);
				dst = READ32(ea);
				src = (UINT32)(INT32)(INT8)FETCH();
				dst = ADC32(dst, src, m_CF);
				WRITE32(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 3:     // SBB Rm32, i32
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM32(modrm);
				src = ((UINT32)(INT32)(INT8)FETCH());
				dst = SBB32(dst, src, m_CF);
				STORE_RM32(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm,1);
				dst = READ32(ea);
				src = ((UINT32)(INT32)(INT8)FETCH());
				dst = SBB32(dst, src, m_CF);
				WRITE32(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 4:     // AND Rm32, i32
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM32(modrm);
				src = (UINT32)(INT32)(INT8)FETCH();
				dst = AND32(dst, src);
				STORE_RM32(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm,1);
				dst = READ32(ea);
				src = (UINT32)(INT32)(INT8)FETCH();
				dst = AND32(dst, src);
				WRITE32(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 5:     // SUB Rm32, i32
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM32(modrm);
				src = (UINT32)(INT32)(INT8)FETCH();
				dst = SUB32(dst, src);
				STORE_RM32(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm,1);
				dst = READ32(ea);
				src = (UINT32)(INT32)(INT8)FETCH();
				dst = SUB32(dst, src);
				WRITE32(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 6:     // XOR Rm32, i32
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM32(modrm);
				src = (UINT32)(INT32)(INT8)FETCH();
				dst = XOR32(dst, src);
				STORE_RM32(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm,1);
				dst = READ32(ea);
				src = (UINT32)(INT32)(INT8)FETCH();
				dst = XOR32(dst, src);
				WRITE32(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 7:     // CMP Rm32, i32
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM32(modrm);
				src = (UINT32)(INT32)(INT8)FETCH();
				SUB32(dst, src);
				CYCLES(CYCLES_CMP_REG_REG);
			} else {
				ea = GetEA(modrm,0);
				dst = READ32(ea);
				src = (UINT32)(INT32)(INT8)FETCH();
				SUB32(dst, src);
				CYCLES(CYCLES_CMP_REG_MEM);
			}
			break;
	}
}

void i386_device::i386_groupC1_32()        // Opcode 0xc1
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
		UINT32 ea = GetEA(modrm,1);
		dst = READ32(ea);
		shift = FETCH() & 0x1f;
		dst = i386_shift_rotate32(modrm, dst, shift);
		WRITE32(ea, dst);
	}
}

void i386_device::i386_groupD1_32()        // Opcode 0xd1
{
	UINT32 dst;
	UINT8 modrm = FETCH();

	if( modrm >= 0xc0 ) {
		dst = LOAD_RM32(modrm);
		dst = i386_shift_rotate32(modrm, dst, 1);
		STORE_RM32(modrm, dst);
	} else {
		UINT32 ea = GetEA(modrm,1);
		dst = READ32(ea);
		dst = i386_shift_rotate32(modrm, dst, 1);
		WRITE32(ea, dst);
	}
}

void i386_device::i386_groupD3_32()        // Opcode 0xd3
{
	UINT32 dst;
	UINT8 modrm = FETCH();

	if( modrm >= 0xc0 ) {
		dst = LOAD_RM32(modrm);
		dst = i386_shift_rotate32(modrm, dst, REG8(CL));
		STORE_RM32(modrm, dst);
	} else {
		UINT32 ea = GetEA(modrm,1);
		dst = READ32(ea);
		dst = i386_shift_rotate32(modrm, dst, REG8(CL));
		WRITE32(ea, dst);
	}
}

void i386_device::i386_groupF7_32()        // Opcode 0xf7
{
	UINT8 modrm = FETCH();

	switch( (modrm >> 3) & 0x7 )
	{
		case 0:         /* TEST Rm32, i32 */
			if( modrm >= 0xc0 ) {
				UINT32 dst = LOAD_RM32(modrm);
				UINT32 src = FETCH32();
				dst &= src;
				m_CF = m_OF = m_AF = 0;
				SetSZPF32(dst);
				CYCLES(CYCLES_TEST_IMM_REG);
			} else {
				UINT32 ea = GetEA(modrm,0);
				UINT32 dst = READ32(ea);
				UINT32 src = FETCH32();
				dst &= src;
				m_CF = m_OF = m_AF = 0;
				SetSZPF32(dst);
				CYCLES(CYCLES_TEST_IMM_MEM);
			}
			break;
		case 2:         /* NOT Rm32 */
			if( modrm >= 0xc0 ) {
				UINT32 dst = LOAD_RM32(modrm);
				dst = ~dst;
				STORE_RM32(modrm, dst);
				CYCLES(CYCLES_NOT_REG);
			} else {
				UINT32 ea = GetEA(modrm,1);
				UINT32 dst = READ32(ea);
				dst = ~dst;
				WRITE32(ea, dst);
				CYCLES(CYCLES_NOT_MEM);
			}
			break;
		case 3:         /* NEG Rm32 */
			if( modrm >= 0xc0 ) {
				UINT32 dst = LOAD_RM32(modrm);
				dst = SUB32(0, dst );
				STORE_RM32(modrm, dst);
				CYCLES(CYCLES_NEG_REG);
			} else {
				UINT32 ea = GetEA(modrm,1);
				UINT32 dst = READ32(ea);
				dst = SUB32(0, dst );
				WRITE32(ea, dst);
				CYCLES(CYCLES_NEG_MEM);
			}
			break;
		case 4:         /* MUL EAX, Rm32 */
			{
				UINT64 result;
				UINT32 src, dst;
				if( modrm >= 0xc0 ) {
					src = LOAD_RM32(modrm);
					CYCLES(CYCLES_MUL32_ACC_REG);      /* TODO: Correct multiply timing */
				} else {
					UINT32 ea = GetEA(modrm,0);
					src = READ32(ea);
					CYCLES(CYCLES_MUL32_ACC_MEM);      /* TODO: Correct multiply timing */
				}

				dst = REG32(EAX);
				result = (UINT64)src * (UINT64)dst;
				REG32(EDX) = (UINT32)(result >> 32);
				REG32(EAX) = (UINT32)result;

				m_CF = m_OF = (REG32(EDX) != 0);
			}
			break;
		case 5:         /* IMUL EAX, Rm32 */
			{
				INT64 result;
				INT64 src, dst;
				if( modrm >= 0xc0 ) {
					src = (INT64)(INT32)LOAD_RM32(modrm);
					CYCLES(CYCLES_IMUL32_ACC_REG);     /* TODO: Correct multiply timing */
				} else {
					UINT32 ea = GetEA(modrm,0);
					src = (INT64)(INT32)READ32(ea);
					CYCLES(CYCLES_IMUL32_ACC_MEM);     /* TODO: Correct multiply timing */
				}

				dst = (INT64)(INT32)REG32(EAX);
				result = src * dst;

				REG32(EDX) = (UINT32)(result >> 32);
				REG32(EAX) = (UINT32)result;

				m_CF = m_OF = !(result == (INT64)(INT32)result);
			}
			break;
		case 6:         /* DIV EAX, Rm32 */
			{
				UINT64 quotient, remainder, result;
				UINT32 src;
				if( modrm >= 0xc0 ) {
					src = LOAD_RM32(modrm);
					CYCLES(CYCLES_DIV32_ACC_REG);
				} else {
					UINT32 ea = GetEA(modrm,0);
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
					i386_trap(0, 0, 0);
				}
			}
			break;
		case 7:         /* IDIV EAX, Rm32 */
			{
				INT64 quotient, remainder, result;
				UINT32 src;
				if( modrm >= 0xc0 ) {
					src = LOAD_RM32(modrm);
					CYCLES(CYCLES_IDIV32_ACC_REG);
				} else {
					UINT32 ea = GetEA(modrm,0);
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
					i386_trap(0, 0, 0);
				}
			}
			break;
	}
}

void i386_device::i386_groupFF_32()        // Opcode 0xff
{
	UINT8 modrm = FETCH();

	switch( (modrm >> 3) & 0x7 )
	{
		case 0:         /* INC Rm32 */
			if( modrm >= 0xc0 ) {
				UINT32 dst = LOAD_RM32(modrm);
				dst = INC32(dst);
				STORE_RM32(modrm, dst);
				CYCLES(CYCLES_INC_REG);
			} else {
				UINT32 ea = GetEA(modrm,1);
				UINT32 dst = READ32(ea);
				dst = INC32(dst);
				WRITE32(ea, dst);
				CYCLES(CYCLES_INC_MEM);
			}
			break;
		case 1:         /* DEC Rm32 */
			if( modrm >= 0xc0 ) {
				UINT32 dst = LOAD_RM32(modrm);
				dst = DEC32(dst);
				STORE_RM32(modrm, dst);
				CYCLES(CYCLES_DEC_REG);
			} else {
				UINT32 ea = GetEA(modrm,1);
				UINT32 dst = READ32(ea);
				dst = DEC32(dst);
				WRITE32(ea, dst);
				CYCLES(CYCLES_DEC_MEM);
			}
			break;
		case 2:         /* CALL Rm32 */
			{
				UINT32 address;
				if( modrm >= 0xc0 ) {
					address = LOAD_RM32(modrm);
					CYCLES(CYCLES_CALL_REG);       /* TODO: Timing = 7 + m */
				} else {
					UINT32 ea = GetEA(modrm,0);
					address = READ32(ea);
					CYCLES(CYCLES_CALL_MEM);       /* TODO: Timing = 10 + m */
				}
				PUSH32(m_eip );
				m_eip = address;
				CHANGE_PC(m_eip);
			}
			break;
		case 3:         /* CALL FAR Rm32 */
			{
				UINT16 selector;
				UINT32 address;

				if( modrm >= 0xc0 )
				{
					report_invalid_modrm("groupFF_32", modrm);
				}
				else
				{
					UINT32 ea = GetEA(modrm,0);
					address = READ32(ea + 0);
					selector = READ16(ea + 4);
					CYCLES(CYCLES_CALL_MEM_INTERSEG);      /* TODO: Timing = 10 + m */
					if(PROTECTED_MODE && !V8086_MODE)
					{
						i386_protected_mode_call(selector,address,1,1);
					}
					else
					{
						PUSH32(m_sreg[CS].selector );
						PUSH32(m_eip );
						m_sreg[CS].selector = selector;
						m_performed_intersegment_jump = 1;
						i386_load_segment_descriptor(CS );
						m_eip = address;
						CHANGE_PC(m_eip);
					}
				}
			}
			break;
		case 4:         /* JMP Rm32 */
			{
				UINT32 address;
				if( modrm >= 0xc0 ) {
					address = LOAD_RM32(modrm);
					CYCLES(CYCLES_JMP_REG);        /* TODO: Timing = 7 + m */
				} else {
					UINT32 ea = GetEA(modrm,0);
					address = READ32(ea);
					CYCLES(CYCLES_JMP_MEM);        /* TODO: Timing = 10 + m */
				}
				m_eip = address;
				CHANGE_PC(m_eip);
			}
			break;
		case 5:         /* JMP FAR Rm32 */
			{
				UINT16 selector;
				UINT32 address;

				if( modrm >= 0xc0 )
				{
					report_invalid_modrm("groupFF_32", modrm);
				}
				else
				{
					UINT32 ea = GetEA(modrm,0);
					address = READ32(ea + 0);
					selector = READ16(ea + 4);
					CYCLES(CYCLES_JMP_MEM_INTERSEG);       /* TODO: Timing = 10 + m */
					if(PROTECTED_MODE && !V8086_MODE)
					{
						i386_protected_mode_jump(selector,address,1,1);
					}
					else
					{
						m_sreg[CS].selector = selector;
						m_performed_intersegment_jump = 1;
						i386_load_segment_descriptor(CS );
						m_eip = address;
						CHANGE_PC(m_eip);
					}
				}
			}
			break;
		case 6:         /* PUSH Rm32 */
			{
				UINT32 value;
				if( modrm >= 0xc0 ) {
					value = LOAD_RM32(modrm);
				} else {
					UINT32 ea = GetEA(modrm,0);
					value = READ32(ea);
				}
				PUSH32(value);
				CYCLES(CYCLES_PUSH_RM);
			}
			break;
		default:
			report_invalid_modrm("groupFF_32", modrm);
			break;
	}
}

void i386_device::i386_group0F00_32()          // Opcode 0x0f 00
{
	UINT32 address, ea;
	UINT8 modrm = FETCH();
	I386_SREG seg;
	UINT8 result;

	switch( (modrm >> 3) & 0x7 )
	{
		case 0:         /* SLDT */
			if ( PROTECTED_MODE && !V8086_MODE )
			{
				if( modrm >= 0xc0 ) {
					STORE_RM32(modrm, m_ldtr.segment);
					CYCLES(CYCLES_SLDT_REG);
				} else {
					ea = GetEA(modrm,1);
					WRITE16(ea, m_ldtr.segment);
					CYCLES(CYCLES_SLDT_MEM);
				}
			}
			else
			{
				i386_trap(6, 0, 0);
			}
			break;
		case 1:         /* STR */
			if ( PROTECTED_MODE && !V8086_MODE )
			{
				if( modrm >= 0xc0 ) {
					STORE_RM32(modrm, m_task.segment);
					CYCLES(CYCLES_STR_REG);
				} else {
					ea = GetEA(modrm,1);
					WRITE16(ea, m_task.segment);
					CYCLES(CYCLES_STR_MEM);
				}
			}
			else
			{
				i386_trap(6, 0, 0);
			}
			break;
		case 2:         /* LLDT */
			if ( PROTECTED_MODE && !V8086_MODE )
			{
				if(m_CPL)
					FAULT(FAULT_GP,0)
				if( modrm >= 0xc0 ) {
					address = LOAD_RM32(modrm);
					m_ldtr.segment = address;
					CYCLES(CYCLES_LLDT_REG);
				} else {
					ea = GetEA(modrm,0);
					m_ldtr.segment = READ32(ea);
					CYCLES(CYCLES_LLDT_MEM);
				}
				memset(&seg, 0, sizeof(seg));
				seg.selector = m_ldtr.segment;
				i386_load_protected_mode_segment(&seg,nullptr);
				m_ldtr.limit = seg.limit;
				m_ldtr.base = seg.base;
				m_ldtr.flags = seg.flags;
			}
			else
			{
				i386_trap(6, 0, 0);
			}
			break;

		case 3:         /* LTR */
			if ( PROTECTED_MODE && !V8086_MODE )
			{
				if(m_CPL)
					FAULT(FAULT_GP,0)
				if( modrm >= 0xc0 ) {
					address = LOAD_RM32(modrm);
					m_task.segment = address;
					CYCLES(CYCLES_LTR_REG);
				} else {
					ea = GetEA(modrm,0);
					m_task.segment = READ32(ea);
					CYCLES(CYCLES_LTR_MEM);
				}
				memset(&seg, 0, sizeof(seg));
				seg.selector = m_task.segment;
				i386_load_protected_mode_segment(&seg,nullptr);

				UINT32 addr = ((seg.selector & 4) ? m_ldtr.base : m_gdtr.base) + (seg.selector & ~7) + 5;
				i386_translate_address(TRANSLATE_READ, &addr, nullptr);
				m_program->write_byte(addr, (seg.flags & 0xff) | 2);

				m_task.limit = seg.limit;
				m_task.base = seg.base;
				m_task.flags = seg.flags | 2;
			}
			else
			{
				i386_trap(6, 0, 0);
			}
			break;

		case 4:  /* VERR */
			if ( PROTECTED_MODE && !V8086_MODE )
			{
				if( modrm >= 0xc0 ) {
					address = LOAD_RM32(modrm);
					CYCLES(CYCLES_VERR_REG);
				} else {
					ea = GetEA(modrm,0);
					address = READ32(ea);
					CYCLES(CYCLES_VERR_MEM);
				}
				memset(&seg, 0, sizeof(seg));
				seg.selector = address;
				result = i386_load_protected_mode_segment(&seg,nullptr);
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
				i386_trap(6, 0, 0);
				logerror("i386: VERR: Exception - Running in real mode or virtual 8086 mode.\n");
			}
			break;

		case 5:  /* VERW */
			if ( PROTECTED_MODE && !V8086_MODE )
			{
				if( modrm >= 0xc0 ) {
					address = LOAD_RM16(modrm);
					CYCLES(CYCLES_VERW_REG);
				} else {
					ea = GetEA(modrm,0);
					address = READ16(ea);
					CYCLES(CYCLES_VERW_MEM);
				}
				memset(&seg, 0, sizeof(seg));
				seg.selector = address;
				result = i386_load_protected_mode_segment(&seg,nullptr);
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
				i386_trap(6, 0, 0);
				logerror("i386: VERW: Exception - Running in real mode or virtual 8086 mode.\n");
			}
			break;

		default:
			report_invalid_modrm("group0F00_32", modrm);
			break;
	}
}

void i386_device::i386_group0F01_32()      // Opcode 0x0f 01
{
	UINT8 modrm = FETCH();
	UINT32 address, ea;

	switch( (modrm >> 3) & 0x7 )
	{
		case 0:         /* SGDT */
			{
				if( modrm >= 0xc0 ) {
					address = LOAD_RM32(modrm);
					ea = i386_translate(CS, address, 1 );
				} else {
					ea = GetEA(modrm,1);
				}
				WRITE16(ea, m_gdtr.limit);
				WRITE32(ea + 2, m_gdtr.base);
				CYCLES(CYCLES_SGDT);
				break;
			}
		case 1:         /* SIDT */
			{
				if (modrm >= 0xc0)
				{
					address = LOAD_RM32(modrm);
					ea = i386_translate(CS, address, 1 );
				}
				else
				{
					ea = GetEA(modrm,1);
				}
				WRITE16(ea, m_idtr.limit);
				WRITE32(ea + 2, m_idtr.base);
				CYCLES(CYCLES_SIDT);
				break;
			}
		case 2:         /* LGDT */
			{
				if(PROTECTED_MODE && m_CPL)
					FAULT(FAULT_GP,0)
				if( modrm >= 0xc0 ) {
					address = LOAD_RM32(modrm);
					ea = i386_translate(CS, address, 0 );
				} else {
					ea = GetEA(modrm,0);
				}
				m_gdtr.limit = READ16(ea);
				m_gdtr.base = READ32(ea + 2);
				CYCLES(CYCLES_LGDT);
				break;
			}
		case 3:         /* LIDT */
			{
				if(PROTECTED_MODE && m_CPL)
					FAULT(FAULT_GP,0)
				if( modrm >= 0xc0 ) {
					address = LOAD_RM32(modrm);
					ea = i386_translate(CS, address, 0 );
				} else {
					ea = GetEA(modrm,0);
				}
				m_idtr.limit = READ16(ea);
				m_idtr.base = READ32(ea + 2);
				CYCLES(CYCLES_LIDT);
				break;
			}
		case 4:         /* SMSW */
			{
				if( modrm >= 0xc0 ) {
					// smsw stores all of cr0 into register
					STORE_RM32(modrm, m_cr[0]);
					CYCLES(CYCLES_SMSW_REG);
				} else {
					/* always 16-bit memory operand */
					ea = GetEA(modrm,1);
					WRITE16(ea, m_cr[0]);
					CYCLES(CYCLES_SMSW_MEM);
				}
				break;
			}
		case 6:         /* LMSW */
			{
				if(PROTECTED_MODE && m_CPL)
					FAULT(FAULT_GP,0)
				UINT16 b;
				if( modrm >= 0xc0 ) {
					b = LOAD_RM16(modrm);
					CYCLES(CYCLES_LMSW_REG);
				} else {
					ea = GetEA(modrm,0);
					CYCLES(CYCLES_LMSW_MEM);
				b = READ16(ea);
				}
				if(PROTECTED_MODE)
					b |= 0x0001;  // cannot return to real mode using this instruction.
				m_cr[0] &= ~0x0000000f;
				m_cr[0] |= b & 0x0000000f;
				break;
			}
		default:
			report_invalid_modrm("group0F01_32", modrm);
			break;
	}
}

void i386_device::i386_group0FBA_32()      // Opcode 0x0f ba
{
	UINT8 modrm = FETCH();

	switch( (modrm >> 3) & 0x7 )
	{
		case 4:         /* BT Rm32, i8 */
			if( modrm >= 0xc0 ) {
				UINT32 dst = LOAD_RM32(modrm);
				UINT8 bit = FETCH();

				if( dst & (1 << bit) )
					m_CF = 1;
				else
					m_CF = 0;

				CYCLES(CYCLES_BT_IMM_REG);
			} else {
				UINT32 ea = GetEA(modrm,0);
				UINT32 dst = READ32(ea);
				UINT8 bit = FETCH();

				if( dst & (1 << bit) )
					m_CF = 1;
				else
					m_CF = 0;

				CYCLES(CYCLES_BT_IMM_MEM);
			}
			break;
		case 5:         /* BTS Rm32, i8 */
			if( modrm >= 0xc0 ) {
				UINT32 dst = LOAD_RM32(modrm);
				UINT8 bit = FETCH();

				if( dst & (1 << bit) )
					m_CF = 1;
				else
					m_CF = 0;
				dst |= (1 << bit);

				STORE_RM32(modrm, dst);
				CYCLES(CYCLES_BTS_IMM_REG);
			} else {
				UINT32 ea = GetEA(modrm,1);
				UINT32 dst = READ32(ea);
				UINT8 bit = FETCH();

				if( dst & (1 << bit) )
					m_CF = 1;
				else
					m_CF = 0;
				dst |= (1 << bit);

				WRITE32(ea, dst);
				CYCLES(CYCLES_BTS_IMM_MEM);
			}
			break;
		case 6:         /* BTR Rm32, i8 */
			if( modrm >= 0xc0 ) {
				UINT32 dst = LOAD_RM32(modrm);
				UINT8 bit = FETCH();

				if( dst & (1 << bit) )
					m_CF = 1;
				else
					m_CF = 0;
				dst &= ~(1 << bit);

				STORE_RM32(modrm, dst);
				CYCLES(CYCLES_BTR_IMM_REG);
			} else {
				UINT32 ea = GetEA(modrm,1);
				UINT32 dst = READ32(ea);
				UINT8 bit = FETCH();

				if( dst & (1 << bit) )
					m_CF = 1;
				else
					m_CF = 0;
				dst &= ~(1 << bit);

				WRITE32(ea, dst);
				CYCLES(CYCLES_BTR_IMM_MEM);
			}
			break;
		case 7:         /* BTC Rm32, i8 */
			if( modrm >= 0xc0 ) {
				UINT32 dst = LOAD_RM32(modrm);
				UINT8 bit = FETCH();

				if( dst & (1 << bit) )
					m_CF = 1;
				else
					m_CF = 0;
				dst ^= (1 << bit);

				STORE_RM32(modrm, dst);
				CYCLES(CYCLES_BTC_IMM_REG);
			} else {
				UINT32 ea = GetEA(modrm,1);
				UINT32 dst = READ32(ea);
				UINT8 bit = FETCH();

				if( dst & (1 << bit) )
					m_CF = 1;
				else
					m_CF = 0;
				dst ^= (1 << bit);

				WRITE32(ea, dst);
				CYCLES(CYCLES_BTC_IMM_MEM);
			}
			break;
		default:
			report_invalid_modrm("group0FBA_32", modrm);
			break;
	}
}

void i386_device::i386_lar_r32_rm32()  // Opcode 0x0f 0x02
{
	UINT8 modrm = FETCH();
	I386_SREG seg;
	UINT8 type;

	if(PROTECTED_MODE && !V8086_MODE)
	{
		memset(&seg,0,sizeof(seg));
		if(modrm >= 0xc0)
		{
			seg.selector = LOAD_RM32(modrm);
			CYCLES(CYCLES_LAR_REG);
		}
		else
		{
			UINT32 ea = GetEA(modrm,0);
			seg.selector = READ32(ea);
			CYCLES(CYCLES_LAR_MEM);
		}
		if(seg.selector == 0)
		{
			SetZF(0);  // not a valid segment
		}
		else
		{
			UINT64 desc;
			if(!i386_load_protected_mode_segment(&seg,&desc))
			{
				SetZF(0);
				return;
			}
			UINT8 DPL = (seg.flags >> 5) & 3;
			if(((DPL < m_CPL) || (DPL < (seg.selector & 3))) && ((seg.flags & 0x1c) != 0x1c))
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
		i386_trap(6,0, 0);
		logerror("i386: LAR: Exception - running in real mode or virtual 8086 mode.\n");
	}
}

void i386_device::i386_lsl_r32_rm32()  // Opcode 0x0f 0x03
{
	UINT8 modrm = FETCH();
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
			UINT32 ea = GetEA(modrm,0);
			seg.selector = READ32(ea);
		}
		if(seg.selector == 0)
		{
			SetZF(0);  // not a valid segment
		}
		else
		{
			UINT8 type;
			if(!i386_load_protected_mode_segment(&seg,nullptr))
			{
				SetZF(0);
				return;
			}
			UINT8 DPL = (seg.flags >> 5) & 3;
			if(((DPL < m_CPL) || (DPL < (seg.selector & 3))) && ((seg.flags & 0x1c) != 0x1c))
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
		i386_trap(6, 0, 0);
}

void i386_device::i386_bound_r32_m32_m32() // Opcode 0x62
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
		UINT32 ea = GetEA(modrm,0);
		low = READ32(ea + 0);
		high = READ32(ea + 4);
	}
	val = LOAD_REG32(modrm);

	if ((val < low) || (val > high))
	{
		CYCLES(CYCLES_BOUND_OUT_RANGE);
		i386_trap(5, 0, 0);
	}
	else
	{
		CYCLES(CYCLES_BOUND_IN_RANGE);
	}
}

void i386_device::i386_retf32()            // Opcode 0xcb
{
	if(PROTECTED_MODE && !V8086_MODE)
	{
		i386_protected_mode_retf(0,1);
	}
	else
	{
		m_eip = POP32();
		m_sreg[CS].selector = POP32();
		i386_load_segment_descriptor(CS );
		CHANGE_PC(m_eip);
	}

	CYCLES(CYCLES_RET_INTERSEG);
}

void i386_device::i386_retf_i32()          // Opcode 0xca
{
	UINT16 count = FETCH16();

	if(PROTECTED_MODE && !V8086_MODE)
	{
		i386_protected_mode_retf(count,1);
	}
	else
	{
		m_eip = POP32();
		m_sreg[CS].selector = POP32();
		i386_load_segment_descriptor(CS );
		CHANGE_PC(m_eip);
		REG32(ESP) += count;
	}

	CYCLES(CYCLES_RET_IMM_INTERSEG);
}

void i386_device::i386_load_far_pointer32(int s)
{
	UINT8 modrm = FETCH();
	UINT16 selector;

	if( modrm >= 0xc0 ) {
		report_invalid_modrm("load_far_pointer32", modrm);
	} else {
		UINT32 ea = GetEA(modrm,0);
		STORE_REG32(modrm, READ32(ea + 0));
		selector = READ16(ea + 4);
		i386_sreg_load(selector,s,nullptr);
	}
}

void i386_device::i386_lds32()             // Opcode 0xc5
{
	i386_load_far_pointer32(DS);
	CYCLES(CYCLES_LDS);
}

void i386_device::i386_lss32()             // Opcode 0x0f 0xb2
{
	i386_load_far_pointer32(SS);
	CYCLES(CYCLES_LSS);
}

void i386_device::i386_les32()             // Opcode 0xc4
{
	i386_load_far_pointer32(ES);
	CYCLES(CYCLES_LES);
}

void i386_device::i386_lfs32()             // Opcode 0x0f 0xb4
{
	i386_load_far_pointer32(FS);
	CYCLES(CYCLES_LFS);
}

void i386_device::i386_lgs32()             // Opcode 0x0f 0xb5
{
	i386_load_far_pointer32(GS);
	CYCLES(CYCLES_LGS);
}
