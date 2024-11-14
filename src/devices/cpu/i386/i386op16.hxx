// license:BSD-3-Clause
// copyright-holders:Ville Linde, Barry Rodewald, Carl, Philip Bennett
uint16_t i386_device::i386_shift_rotate16(uint8_t modrm, uint32_t value, uint8_t shift)
{
	uint32_t src = value & 0xffff;
	uint16_t dst = value;

	if( shift == 0 ) {
		CYCLES_RM(modrm, 3, 7);
	} else if( shift == 1 ) {
		switch( (modrm >> 3) & 0x7 )
		{
			case 0:         /* ROL rm16, 1 */
				m_CF = (src & 0x8000) ? 1 : 0;
				dst = (src << 1) + m_CF;
				m_OF = ((src ^ dst) & 0x8000) ? 1 : 0;
				CYCLES_RM(modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 1:         /* ROR rm16, 1 */
				m_CF = (src & 0x1) ? 1 : 0;
				dst = (m_CF << 15) | (src >> 1);
				m_OF = ((src ^ dst) & 0x8000) ? 1 : 0;
				CYCLES_RM(modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 2:         /* RCL rm16, 1 */
				dst = (src << 1) + m_CF;
				m_CF = (src & 0x8000) ? 1 : 0;
				m_OF = ((src ^ dst) & 0x8000) ? 1 : 0;
				CYCLES_RM(modrm, CYCLES_ROTATE_CARRY_REG, CYCLES_ROTATE_CARRY_MEM);
				break;
			case 3:         /* RCR rm16, 1 */
				dst = (m_CF << 15) | (src >> 1);
				m_CF = src & 0x1;
				m_OF = ((src ^ dst) & 0x8000) ? 1 : 0;
				CYCLES_RM(modrm, CYCLES_ROTATE_CARRY_REG, CYCLES_ROTATE_CARRY_MEM);
				break;
			case 4:         /* SHL/SAL rm16, 1 */
			case 6:
				dst = src << 1;
				m_CF = (src & 0x8000) ? 1 : 0;
				m_OF = (((m_CF << 15) ^ dst) & 0x8000) ? 1 : 0;
				SetSZPF16(dst);
				CYCLES_RM(modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 5:         /* SHR rm16, 1 */
				dst = src >> 1;
				m_CF = src & 0x1;
				m_OF = (dst & 0x8000) ? 1 : 0;
				SetSZPF16(dst);
				CYCLES_RM(modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 7:         /* SAR rm16, 1 */
				dst = (int16_t)(src) >> 1;
				m_CF = src & 0x1;
				m_OF = 0;
				SetSZPF16(dst);
				CYCLES_RM(modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
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
						m_CF = src & 1;
						m_OF = (src & 1) ^ ((src >> 15) & 1);
					}
					break;
				}
				shift &= 15;
				dst = ((src & ((uint16_t)0xffff >> shift)) << shift) |
						((src & ((uint16_t)0xffff << (16-shift))) >> (16-shift));
				m_CF = dst & 0x1;
				m_OF = (dst & 1) ^ (dst >> 15);
				CYCLES_RM(modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 1:         /* ROR rm16, i8 */
				if(!(shift & 15))
				{
					if(shift & 16)
					{
						m_CF = (src >> 15) & 1;
						m_OF = ((src >> 15) & 1) ^ ((src >> 14) & 1);
					}
					break;
				}
				shift &= 15;
				dst = ((src & ((uint16_t)0xffff << shift)) >> shift) |
						((src & ((uint16_t)0xffff >> (16-shift))) << (16-shift));
				m_CF = (dst >> 15) & 1;
				m_OF = ((dst >> 15) ^ (dst >> 14)) & 1;
				CYCLES_RM(modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 2:         /* RCL rm16, i8 */
				shift %= 17;
				dst = ((src & ((uint16_t)0xffff >> shift)) << shift) |
						((src & ((uint16_t)0xffff << (17-shift))) >> (17-shift)) |
						(m_CF << (shift-1));
				if(shift) m_CF = (src >> (16-shift)) & 0x1;
				m_OF = m_CF ^ ((dst >> 15) & 1);
				CYCLES_RM(modrm, CYCLES_ROTATE_CARRY_REG, CYCLES_ROTATE_CARRY_MEM);
				break;
			case 3:         /* RCR rm16, i8 */
				shift %= 17;
				dst = ((src & ((uint16_t)0xffff << shift)) >> shift) |
						((src & ((uint16_t)0xffff >> (16-shift))) << (17-shift)) |
						(m_CF << (16-shift));
				if(shift) m_CF = (src >> (shift-1)) & 0x1;
				m_OF = ((dst >> 15) ^ (dst >> 14)) & 1;
				CYCLES_RM(modrm, CYCLES_ROTATE_CARRY_REG, CYCLES_ROTATE_CARRY_MEM);
				break;
			case 4:         /* SHL/SAL rm16, i8 */
			case 6:
				shift &= 31;
				dst = src << shift;
				m_CF = (shift <= 16) && (src & (1 << (16-shift)));
				SetSZPF16(dst);
				CYCLES_RM(modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 5:         /* SHR rm16, i8 */
				shift &= 31;
				dst = src >> shift;
				m_CF = (src & (1 << (shift-1))) ? 1 : 0;
				SetSZPF16(dst);
				CYCLES_RM(modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 7:         /* SAR rm16, i8 */
				shift &= 31;
				dst = (int16_t)src >> shift;
				m_CF = (src & (1 << (shift-1))) ? 1 : 0;
				SetSZPF16(dst);
				CYCLES_RM(modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
		}

	}
	return dst;
}



void i386_device::i386_adc_rm16_r16()      // Opcode 0x11
{
	uint16_t src, dst;
	uint8_t modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_REG16(modrm);
		dst = LOAD_RM16(modrm);
		dst = ADC16(dst, src, m_CF);
		STORE_RM16(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		uint32_t ea = GetEA(modrm,1);
		src = LOAD_REG16(modrm);
		dst = READ16(ea);
		dst = ADC16(dst, src, m_CF);
		WRITE16(ea, dst);
		CYCLES(CYCLES_ALU_REG_MEM);
	}
}

void i386_device::i386_adc_r16_rm16()      // Opcode 0x13
{
	uint16_t src, dst;
	uint8_t modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_RM16(modrm);
		dst = LOAD_REG16(modrm);
		dst = ADC16(dst, src, m_CF);
		STORE_REG16(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		uint32_t ea = GetEA(modrm,0);
		src = READ16(ea);
		dst = LOAD_REG16(modrm);
		dst = ADC16(dst, src, m_CF);
		STORE_REG16(modrm, dst);
		CYCLES(CYCLES_ALU_MEM_REG);
	}
}

void i386_device::i386_adc_ax_i16()        // Opcode 0x15
{
	uint16_t src, dst;
	src = FETCH16();
	dst = REG16(AX);
	dst = ADC16(dst, src, m_CF);
	REG16(AX) = dst;
	CYCLES(CYCLES_ALU_IMM_ACC);
}

void i386_device::i386_add_rm16_r16()      // Opcode 0x01
{
	uint16_t src, dst;
	uint8_t modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_REG16(modrm);
		dst = LOAD_RM16(modrm);
		dst = ADD16(dst, src);
		STORE_RM16(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		uint32_t ea = GetEA(modrm,1);
		src = LOAD_REG16(modrm);
		dst = READ16(ea);
		dst = ADD16(dst, src);
		WRITE16(ea, dst);
		CYCLES(CYCLES_ALU_REG_MEM);
	}
}

void i386_device::i386_add_r16_rm16()      // Opcode 0x03
{
	uint16_t src, dst;
	uint8_t modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_RM16(modrm);
		dst = LOAD_REG16(modrm);
		dst = ADD16(dst, src);
		STORE_REG16(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		uint32_t ea = GetEA(modrm,0);
		src = READ16(ea);
		dst = LOAD_REG16(modrm);
		dst = ADD16(dst, src);
		STORE_REG16(modrm, dst);
		CYCLES(CYCLES_ALU_MEM_REG);
	}
}

void i386_device::i386_add_ax_i16()        // Opcode 0x05
{
	uint16_t src, dst;
	src = FETCH16();
	dst = REG16(AX);
	dst = ADD16(dst, src);
	REG16(AX) = dst;
	CYCLES(CYCLES_ALU_IMM_ACC);
}

void i386_device::i386_and_rm16_r16()      // Opcode 0x21
{
	uint16_t src, dst;
	uint8_t modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_REG16(modrm);
		dst = LOAD_RM16(modrm);
		dst = AND16(dst, src);
		STORE_RM16(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		uint32_t ea = GetEA(modrm,1);
		src = LOAD_REG16(modrm);
		dst = READ16(ea);
		dst = AND16(dst, src);
		WRITE16(ea, dst);
		CYCLES(CYCLES_ALU_REG_MEM);
	}
}

void i386_device::i386_and_r16_rm16()      // Opcode 0x23
{
	uint16_t src, dst;
	uint8_t modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_RM16(modrm);
		dst = LOAD_REG16(modrm);
		dst = AND16(dst, src);
		STORE_REG16(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		uint32_t ea = GetEA(modrm,0);
		src = READ16(ea);
		dst = LOAD_REG16(modrm);
		dst = AND16(dst, src);
		STORE_REG16(modrm, dst);
		CYCLES(CYCLES_ALU_MEM_REG);
	}
}

void i386_device::i386_and_ax_i16()        // Opcode 0x25
{
	uint16_t src, dst;
	src = FETCH16();
	dst = REG16(AX);
	dst = AND16(dst, src);
	REG16(AX) = dst;
	CYCLES(CYCLES_ALU_IMM_ACC);
}

void i386_device::i386_bsf_r16_rm16()      // Opcode 0x0f bc
{
	uint16_t src, dst, temp;
	uint8_t modrm = FETCH();

	if( modrm >= 0xc0 ) {
		src = LOAD_RM16(modrm);
	} else {
		uint32_t ea = GetEA(modrm,0);
		src = READ16(ea);
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
		STORE_REG16(modrm, dst);
	}
	CYCLES(CYCLES_BSF_BASE);
}

void i386_device::i386_bsr_r16_rm16()      // Opcode 0x0f bd
{
	uint16_t src, dst, temp;
	uint8_t modrm = FETCH();

	if( modrm >= 0xc0 ) {
		src = LOAD_RM16(modrm);
	} else {
		uint32_t ea = GetEA(modrm,0);
		src = READ16(ea);
	}

	dst = 0;

	if( src == 0 ) {
		m_ZF = 1;
	} else {
		m_ZF = 0;
		dst = temp = 15;
		while( (src & (1 << temp)) == 0 ) {
			temp--;
			dst = temp;
			CYCLES(CYCLES_BSR);
		}
		STORE_REG16(modrm, dst);
	}
	CYCLES(CYCLES_BSR_BASE);
}


void i386_device::i386_bt_rm16_r16()       // Opcode 0x0f a3
{
	uint8_t modrm = FETCH();
	if( modrm >= 0xc0 ) {
		uint16_t dst = LOAD_RM16(modrm);
		uint16_t bit = LOAD_REG16(modrm);

		if( dst & (1 << (bit & 0xf)) )
			m_CF = 1;
		else
			m_CF = 0;

		CYCLES(CYCLES_BT_REG_REG);
	} else {
		uint8_t segment;
		uint32_t ea = GetNonTranslatedEA(modrm,&segment);
		uint16_t bit = LOAD_REG16(modrm);
		ea += 2*(bit/16);
		ea = i386_translate(segment,(m_address_size)?ea:(ea&0xffff),0);
		bit %= 16;
		uint16_t dst = READ16(ea);

		if( dst & (1 << bit) )
			m_CF = 1;
		else
			m_CF = 0;

		CYCLES(CYCLES_BT_REG_MEM);
	}
}

void i386_device::i386_btc_rm16_r16()      // Opcode 0x0f bb
{
	uint8_t modrm = FETCH();
	if( modrm >= 0xc0 ) {
		uint16_t dst = LOAD_RM16(modrm);
		uint16_t bit = LOAD_REG16(modrm);

		if( dst & (1 << (bit & 0xf)) )
			m_CF = 1;
		else
			m_CF = 0;
		dst ^= (1 << (bit & 0xf));

		STORE_RM16(modrm, dst);
		CYCLES(CYCLES_BTC_REG_REG);
	} else {
		uint8_t segment;
		uint32_t ea = GetNonTranslatedEA(modrm,&segment);
		uint16_t bit = LOAD_REG16(modrm);
		ea += 2*(bit/16);
		ea = i386_translate(segment,(m_address_size)?ea:(ea&0xffff),1);
		bit %= 16;
		uint16_t dst = READ16(ea);

		if( dst & (1 << bit) )
			m_CF = 1;
		else
			m_CF = 0;
		dst ^= (1 << bit);

		WRITE16(ea, dst);
		CYCLES(CYCLES_BTC_REG_MEM);
	}
}

void i386_device::i386_btr_rm16_r16()      // Opcode 0x0f b3
{
	uint8_t modrm = FETCH();
	if( modrm >= 0xc0 ) {
		uint16_t dst = LOAD_RM16(modrm);
		uint16_t bit = LOAD_REG16(modrm);

		if( dst & (1 << (bit & 0xf)) )
			m_CF = 1;
		else
			m_CF = 0;
		dst &= ~(1 << (bit & 0xf));

		STORE_RM16(modrm, dst);
		CYCLES(CYCLES_BTR_REG_REG);
	} else {
		uint8_t segment;
		uint32_t ea = GetNonTranslatedEA(modrm,&segment);
		uint16_t bit = LOAD_REG16(modrm);
		ea += 2*(bit/16);
		ea = i386_translate(segment,(m_address_size)?ea:(ea&0xffff),1);
		bit %= 16;
		uint16_t dst = READ16(ea);

		if( dst & (1 << bit) )
			m_CF = 1;
		else
			m_CF = 0;
		dst &= ~(1 << bit);

		WRITE16(ea, dst);
		CYCLES(CYCLES_BTR_REG_MEM);
	}
}

void i386_device::i386_bts_rm16_r16()      // Opcode 0x0f ab
{
	uint8_t modrm = FETCH();
	if( modrm >= 0xc0 ) {
		uint16_t dst = LOAD_RM16(modrm);
		uint16_t bit = LOAD_REG16(modrm);

		if( dst & (1 << (bit & 0xf)) )
			m_CF = 1;
		else
			m_CF = 0;
		dst |= (1 << (bit & 0xf));

		STORE_RM16(modrm, dst);
		CYCLES(CYCLES_BTS_REG_REG);
	} else {
		uint8_t segment;
		uint32_t ea = GetNonTranslatedEA(modrm,&segment);
		uint16_t bit = LOAD_REG16(modrm);
		ea += 2*(bit/16);
		ea = i386_translate(segment,(m_address_size)?ea:(ea&0xffff),1);
		bit %= 16;
		uint16_t dst = READ16(ea);

		if( dst & (1 << bit) )
			m_CF = 1;
		else
			m_CF = 0;
		dst |= (1 << bit);

		WRITE16(ea, dst);
		CYCLES(CYCLES_BTS_REG_MEM);
	}
}

void i386_device::i386_call_abs16()        // Opcode 0x9a
{
	uint16_t offset = FETCH16();
	uint16_t ptr = FETCH16();

	if( PROTECTED_MODE && !V8086_MODE)
	{
		i386_protected_mode_call(ptr,offset,0,0);
	}
	else
	{
		PUSH16(m_sreg[CS].selector );
		PUSH16(m_eip );
		m_sreg[CS].selector = ptr;
		m_performed_intersegment_jump = 1;
		m_eip = offset;
		i386_load_segment_descriptor(CS);
	}
	CYCLES(CYCLES_CALL_INTERSEG);      /* TODO: Timing = 17 + m */
	CHANGE_PC(m_eip);
}

void i386_device::i386_call_rel16()        // Opcode 0xe8
{
	int16_t disp = FETCH16();

	PUSH16(m_eip );
	if (m_sreg[CS].d)
	{
		m_eip += disp;
	}
	else
	{
		m_eip = (m_eip + disp) & 0xffff;
	}
	CHANGE_PC(m_eip);
	CYCLES(CYCLES_CALL);       /* TODO: Timing = 7 + m */
}

void i386_device::i386_cbw()               // Opcode 0x98
{
	REG16(AX) = (int16_t)((int8_t)REG8(AL));
	CYCLES(CYCLES_CBW);
}

void i386_device::i386_cmp_rm16_r16()      // Opcode 0x39
{
	uint16_t src, dst;
	uint8_t modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_REG16(modrm);
		dst = LOAD_RM16(modrm);
		SUB16(dst, src);
		CYCLES(CYCLES_CMP_REG_REG);
	} else {
		uint32_t ea = GetEA(modrm,0);
		src = LOAD_REG16(modrm);
		dst = READ16(ea);
		SUB16(dst, src);
		CYCLES(CYCLES_CMP_REG_MEM);
	}
}

void i386_device::i386_cmp_r16_rm16()      // Opcode 0x3b
{
	uint16_t src, dst;
	uint8_t modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_RM16(modrm);
		dst = LOAD_REG16(modrm);
		SUB16(dst, src);
		CYCLES(CYCLES_CMP_REG_REG);
	} else {
		uint32_t ea = GetEA(modrm,0);
		src = READ16(ea);
		dst = LOAD_REG16(modrm);
		SUB16(dst, src);
		CYCLES(CYCLES_CMP_MEM_REG);
	}
}

void i386_device::i386_cmp_ax_i16()        // Opcode 0x3d
{
	uint16_t src, dst;
	src = FETCH16();
	dst = REG16(AX);
	SUB16(dst, src);
	CYCLES(CYCLES_CMP_IMM_ACC);
}

void i386_device::i386_cmpsw()             // Opcode 0xa7
{
	uint32_t eas, ead;
	uint16_t src, dst;
	if( m_segment_prefix ) {
		eas = i386_translate(m_segment_override, m_address_size ? REG32(ESI) : REG16(SI), 0 );
	} else {
		eas = i386_translate(DS, m_address_size ? REG32(ESI) : REG16(SI), 0 );
	}
	ead = i386_translate(ES, m_address_size ? REG32(EDI) : REG16(DI), 0 );
	src = READ16(eas);
	dst = READ16(ead);
	SUB16(src,dst);
	BUMP_SI(2);
	BUMP_DI(2);
	CYCLES(CYCLES_CMPS);
}

void i386_device::i386_cwd()               // Opcode 0x99
{
	if( REG16(AX) & 0x8000 ) {
		REG16(DX) = 0xffff;
	} else {
		REG16(DX) = 0x0000;
	}
	CYCLES(CYCLES_CWD);
}

void i386_device::i386_dec_ax()            // Opcode 0x48
{
	REG16(AX) = DEC16(REG16(AX) );
	CYCLES(CYCLES_DEC_REG);
}

void i386_device::i386_dec_cx()            // Opcode 0x49
{
	REG16(CX) = DEC16(REG16(CX) );
	CYCLES(CYCLES_DEC_REG);
}

void i386_device::i386_dec_dx()            // Opcode 0x4a
{
	REG16(DX) = DEC16(REG16(DX) );
	CYCLES(CYCLES_DEC_REG);
}

void i386_device::i386_dec_bx()            // Opcode 0x4b
{
	REG16(BX) = DEC16(REG16(BX) );
	CYCLES(CYCLES_DEC_REG);
}

void i386_device::i386_dec_sp()            // Opcode 0x4c
{
	REG16(SP) = DEC16(REG16(SP) );
	CYCLES(CYCLES_DEC_REG);
}

void i386_device::i386_dec_bp()            // Opcode 0x4d
{
	REG16(BP) = DEC16(REG16(BP) );
	CYCLES(CYCLES_DEC_REG);
}

void i386_device::i386_dec_si()            // Opcode 0x4e
{
	REG16(SI) = DEC16(REG16(SI) );
	CYCLES(CYCLES_DEC_REG);
}

void i386_device::i386_dec_di()            // Opcode 0x4f
{
	REG16(DI) = DEC16(REG16(DI) );
	CYCLES(CYCLES_DEC_REG);
}

void i386_device::i386_imul_r16_rm16()     // Opcode 0x0f af
{
	uint8_t modrm = FETCH();
	int32_t result;
	int32_t src, dst;
	if( modrm >= 0xc0 ) {
		src = (int32_t)(int16_t)LOAD_RM16(modrm);
		CYCLES(CYCLES_IMUL16_REG_REG);     /* TODO: Correct multiply timing */
	} else {
		uint32_t ea = GetEA(modrm,0);
		src = (int32_t)(int16_t)READ16(ea);
		CYCLES(CYCLES_IMUL16_REG_MEM);     /* TODO: Correct multiply timing */
	}

	dst = (int32_t)(int16_t)LOAD_REG16(modrm);
	result = src * dst;

	STORE_REG16(modrm, (uint16_t)result);

	m_CF = m_OF = !(result == (int32_t)(int16_t)result);
}

void i386_device::i386_imul_r16_rm16_i16() // Opcode 0x69
{
	uint8_t modrm = FETCH();
	int32_t result;
	int32_t src, dst;
	if( modrm >= 0xc0 ) {
		dst = (int32_t)(int16_t)LOAD_RM16(modrm);
		CYCLES(CYCLES_IMUL16_REG_IMM_REG);     /* TODO: Correct multiply timing */
	} else {
		uint32_t ea = GetEA(modrm,0);
		dst = (int32_t)(int16_t)READ16(ea);
		CYCLES(CYCLES_IMUL16_MEM_IMM_REG);     /* TODO: Correct multiply timing */
	}

	src = (int32_t)(int16_t)FETCH16();
	result = src * dst;

	STORE_REG16(modrm, (uint16_t)result);

	m_CF = m_OF = !(result == (int32_t)(int16_t)result);
}

void i386_device::i386_imul_r16_rm16_i8()  // Opcode 0x6b
{
	uint8_t modrm = FETCH();
	int32_t result;
	int32_t src, dst;
	if( modrm >= 0xc0 ) {
		dst = (int32_t)(int16_t)LOAD_RM16(modrm);
		CYCLES(CYCLES_IMUL16_REG_IMM_REG);     /* TODO: Correct multiply timing */
	} else {
		uint32_t ea = GetEA(modrm,0);
		dst = (int32_t)(int16_t)READ16(ea);
		CYCLES(CYCLES_IMUL16_MEM_IMM_REG);     /* TODO: Correct multiply timing */
	}

	src = (int32_t)(int8_t)FETCH();
	result = src * dst;

	STORE_REG16(modrm, (uint16_t)result);

	m_CF = m_OF = !(result == (int32_t)(int16_t)result);
}

void i386_device::i386_in_ax_i8()          // Opcode 0xe5
{
	uint16_t port = FETCH();
	uint16_t data = READPORT16(port);
	REG16(AX) = data;
	CYCLES(CYCLES_IN_VAR);
}

void i386_device::i386_in_ax_dx()          // Opcode 0xed
{
	uint16_t port = REG16(DX);
	uint16_t data = READPORT16(port);
	REG16(AX) = data;
	CYCLES(CYCLES_IN);
}

void i386_device::i386_inc_ax()            // Opcode 0x40
{
	REG16(AX) = INC16(REG16(AX) );
	CYCLES(CYCLES_INC_REG);
}

void i386_device::i386_inc_cx()            // Opcode 0x41
{
	REG16(CX) = INC16(REG16(CX) );
	CYCLES(CYCLES_INC_REG);
}

void i386_device::i386_inc_dx()            // Opcode 0x42
{
	REG16(DX) = INC16(REG16(DX) );
	CYCLES(CYCLES_INC_REG);
}

void i386_device::i386_inc_bx()            // Opcode 0x43
{
	REG16(BX) = INC16(REG16(BX) );
	CYCLES(CYCLES_INC_REG);
}

void i386_device::i386_inc_sp()            // Opcode 0x44
{
	REG16(SP) = INC16(REG16(SP) );
	CYCLES(CYCLES_INC_REG);
}

void i386_device::i386_inc_bp()            // Opcode 0x45
{
	REG16(BP) = INC16(REG16(BP) );
	CYCLES(CYCLES_INC_REG);
}

void i386_device::i386_inc_si()            // Opcode 0x46
{
	REG16(SI) = INC16(REG16(SI) );
	CYCLES(CYCLES_INC_REG);
}

void i386_device::i386_inc_di()            // Opcode 0x47
{
	REG16(DI) = INC16(REG16(DI) );
	CYCLES(CYCLES_INC_REG);
}

void i386_device::i386_iret16()            // Opcode 0xcf
{
	if( PROTECTED_MODE )
	{
		i386_protected_mode_iret(0);
	}
	else
	{
		/* TODO: #SS(0) exception */
		/* TODO: #GP(0) exception */
		m_eip = POP16();
		m_sreg[CS].selector = POP16();
		set_flags(POP16() );
		i386_load_segment_descriptor(CS);
		CHANGE_PC(m_eip);
	}
	m_auto_clear_RF = false;
	CYCLES(CYCLES_IRET);
}

void i386_device::i386_ja_rel16()          // Opcode 0x0f 87
{
	int16_t disp = FETCH16();
	if( m_CF == 0 && m_ZF == 0 ) {
		if (m_sreg[CS].d)
		{
			m_eip += disp;
		}
		else
		{
			m_eip = (m_eip + disp) & 0xffff;
		}
		CHANGE_PC(m_eip);
		CYCLES(CYCLES_JCC_FULL_DISP);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

void i386_device::i386_jbe_rel16()         // Opcode 0x0f 86
{
	int16_t disp = FETCH16();
	if( m_CF != 0 || m_ZF != 0 ) {
		if (m_sreg[CS].d)
		{
			m_eip += disp;
		}
		else
		{
			m_eip = (m_eip + disp) & 0xffff;
		}
		CHANGE_PC(m_eip);
		CYCLES(CYCLES_JCC_FULL_DISP);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

void i386_device::i386_jc_rel16()          // Opcode 0x0f 82
{
	int16_t disp = FETCH16();
	if( m_CF != 0 ) {
		if (m_sreg[CS].d)
		{
			m_eip += disp;
		}
		else
		{
			m_eip = (m_eip + disp) & 0xffff;
		}
		CHANGE_PC(m_eip);
		CYCLES(CYCLES_JCC_FULL_DISP);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

void i386_device::i386_jg_rel16()          // Opcode 0x0f 8f
{
	int16_t disp = FETCH16();
	if( m_ZF == 0 && (m_SF == m_OF) ) {
		if (m_sreg[CS].d)
		{
			m_eip += disp;
		}
		else
		{
			m_eip = (m_eip + disp) & 0xffff;
		}
		CHANGE_PC(m_eip);
		CYCLES(CYCLES_JCC_FULL_DISP);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

void i386_device::i386_jge_rel16()         // Opcode 0x0f 8d
{
	int16_t disp = FETCH16();
	if(m_SF == m_OF) {
		if (m_sreg[CS].d)
		{
			m_eip += disp;
		}
		else
		{
			m_eip = (m_eip + disp) & 0xffff;
		}
		CHANGE_PC(m_eip);
		CYCLES(CYCLES_JCC_FULL_DISP);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

void i386_device::i386_jl_rel16()          // Opcode 0x0f 8c
{
	int16_t disp = FETCH16();
	if( (m_SF != m_OF) ) {
		if (m_sreg[CS].d)
		{
			m_eip += disp;
		}
		else
		{
			m_eip = (m_eip + disp) & 0xffff;
		}
		CHANGE_PC(m_eip);
		CYCLES(CYCLES_JCC_FULL_DISP);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

void i386_device::i386_jle_rel16()         // Opcode 0x0f 8e
{
	int16_t disp = FETCH16();
	if( m_ZF != 0 || (m_SF != m_OF) ) {
		if (m_sreg[CS].d)
		{
			m_eip += disp;
		}
		else
		{
			m_eip = (m_eip + disp) & 0xffff;
		}
		CHANGE_PC(m_eip);
		CYCLES(CYCLES_JCC_FULL_DISP);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

void i386_device::i386_jnc_rel16()         // Opcode 0x0f 83
{
	int16_t disp = FETCH16();
	if( m_CF == 0 ) {
		if (m_sreg[CS].d)
		{
			m_eip += disp;
		}
		else
		{
			m_eip = (m_eip + disp) & 0xffff;
		}
		CHANGE_PC(m_eip);
		CYCLES(CYCLES_JCC_FULL_DISP);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

void i386_device::i386_jno_rel16()         // Opcode 0x0f 81
{
	int16_t disp = FETCH16();
	if( m_OF == 0 ) {
		if (m_sreg[CS].d)
		{
			m_eip += disp;
		}
		else
		{
			m_eip = (m_eip + disp) & 0xffff;
		}
		CHANGE_PC(m_eip);
		CYCLES(CYCLES_JCC_FULL_DISP);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

void i386_device::i386_jnp_rel16()         // Opcode 0x0f 8b
{
	int16_t disp = FETCH16();
	if( m_PF == 0 ) {
		if (m_sreg[CS].d)
		{
			m_eip += disp;
		}
		else
		{
			m_eip = (m_eip + disp) & 0xffff;
		}
		CHANGE_PC(m_eip);
		CYCLES(CYCLES_JCC_FULL_DISP);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

void i386_device::i386_jns_rel16()         // Opcode 0x0f 89
{
	int16_t disp = FETCH16();
	if( m_SF == 0 ) {
		if (m_sreg[CS].d)
		{
			m_eip += disp;
		}
		else
		{
			m_eip = (m_eip + disp) & 0xffff;
		}
		CHANGE_PC(m_eip);
		CYCLES(CYCLES_JCC_FULL_DISP);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

void i386_device::i386_jnz_rel16()         // Opcode 0x0f 85
{
	int16_t disp = FETCH16();
	if( m_ZF == 0 ) {
		if (m_sreg[CS].d)
		{
			m_eip += disp;
		}
		else
		{
			m_eip = (m_eip + disp) & 0xffff;
		}
		CHANGE_PC(m_eip);
		CYCLES(CYCLES_JCC_FULL_DISP);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

void i386_device::i386_jo_rel16()          // Opcode 0x0f 80
{
	int16_t disp = FETCH16();
	if( m_OF != 0 ) {
		if (m_sreg[CS].d)
		{
			m_eip += disp;
		}
		else
		{
			m_eip = (m_eip + disp) & 0xffff;
		}
		CHANGE_PC(m_eip);
		CYCLES(CYCLES_JCC_FULL_DISP);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

void i386_device::i386_jp_rel16()          // Opcode 0x0f 8a
{
	int16_t disp = FETCH16();
	if( m_PF != 0 ) {
		if (m_sreg[CS].d)
		{
			m_eip += disp;
		}
		else
		{
			m_eip = (m_eip + disp) & 0xffff;
		}
		CHANGE_PC(m_eip);
		CYCLES(CYCLES_JCC_FULL_DISP);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

void i386_device::i386_js_rel16()          // Opcode 0x0f 88
{
	int16_t disp = FETCH16();
	if( m_SF != 0 ) {
		if (m_sreg[CS].d)
		{
			m_eip += disp;
		}
		else
		{
			m_eip = (m_eip + disp) & 0xffff;
		}
		CHANGE_PC(m_eip);
		CYCLES(CYCLES_JCC_FULL_DISP);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

void i386_device::i386_jz_rel16()          // Opcode 0x0f 84
{
	int16_t disp = FETCH16();
	if( m_ZF != 0 ) {
		if (m_sreg[CS].d)
		{
			m_eip += disp;
		}
		else
		{
			m_eip = (m_eip + disp) & 0xffff;
		}
		CHANGE_PC(m_eip);
		CYCLES(CYCLES_JCC_FULL_DISP);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

void i386_device::i386_jcxz16()            // Opcode 0xe3
{
	int8_t disp = FETCH();
	int val = (m_address_size)?(REG32(ECX) == 0):(REG16(CX) == 0);
	if( val ) {
		if (m_sreg[CS].d)
		{
			m_eip += disp;
		}
		else
		{
			m_eip = (m_eip + disp) & 0xffff;
		}
		CHANGE_PC(m_eip);
		CYCLES(CYCLES_JCXZ);       /* TODO: Timing = 9 + m */
	} else {
		CYCLES(CYCLES_JCXZ_NOBRANCH);
	}
}

void i386_device::i386_jmp_rel16()         // Opcode 0xe9
{
	int16_t disp = FETCH16();

	if (m_sreg[CS].d)
	{
		m_eip += disp;
	}
	else
	{
		m_eip = (m_eip + disp) & 0xffff;
	}
	CHANGE_PC(m_eip);
	CYCLES(CYCLES_JMP);        /* TODO: Timing = 7 + m */
}

void i386_device::i386_jmp_abs16()         // Opcode 0xea
{
	uint16_t address = FETCH16();
	uint16_t segment = FETCH16();

	if( PROTECTED_MODE && !V8086_MODE)
	{
		i386_protected_mode_jump(segment,address,0,0);
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

void i386_device::i386_lea16()             // Opcode 0x8d
{
	uint8_t modrm = FETCH();
	uint32_t ea = GetNonTranslatedEA(modrm,nullptr);
	STORE_REG16(modrm, ea);
	CYCLES(CYCLES_LEA);
}

void i386_device::i386_enter16()           // Opcode 0xc8
{
	uint16_t framesize = FETCH16();
	uint8_t level = FETCH() % 32;
	uint8_t x;
	uint16_t frameptr;
	PUSH16(REG16(BP));

	if(!STACK_32BIT)
		frameptr = REG16(SP);
	else
		frameptr = REG32(ESP);

	if(level > 0)
	{
		for(x=1;x<=level-1;x++)
		{
			uint32_t addr;
			if(!STACK_32BIT)
			{
				REG16(BP) -= 2;
				addr = REG16(BP);
			}
			else
			{
				REG32(EBP) -= 2;
				addr = REG32(EBP);
			}
			PUSH16(READ16(i386_translate(SS, addr, 0)));
		}
		PUSH16(frameptr);
	}
	REG16(BP) = frameptr;
	if(!STACK_32BIT)
		REG16(SP) -= framesize;
	else
		REG32(ESP) -= framesize;
	CYCLES(CYCLES_ENTER);
}

void i386_device::i386_leave16()           // Opcode 0xc9
{
	if(!STACK_32BIT)
		REG16(SP) = REG16(BP);
	else
		REG32(ESP) = REG32(EBP);
	REG16(BP) = POP16();
	CYCLES(CYCLES_LEAVE);
}

void i386_device::i386_lodsw()             // Opcode 0xad
{
	uint32_t eas;
	if( m_segment_prefix ) {
		eas = i386_translate(m_segment_override, m_address_size ? REG32(ESI) : REG16(SI), 0, 2);
	} else {
		eas = i386_translate(DS, m_address_size ? REG32(ESI) : REG16(SI), 0, 2);
	}
	REG16(AX) = READ16(eas);
	BUMP_SI(2);
	CYCLES(CYCLES_LODS);
}

void i386_device::i386_loop16()            // Opcode 0xe2
{
	int8_t disp = FETCH();
	int32_t val = (m_address_size)?(--REG32(ECX)):(--REG16(CX));
	if( val != 0 ) {
		if (m_sreg[CS].d)
		{
			m_eip += disp;
		}
		else
		{
			m_eip = (m_eip + disp) & 0xffff;
		}
		CHANGE_PC(m_eip);
	}
	CYCLES(CYCLES_LOOP);       /* TODO: Timing = 11 + m */
}

void i386_device::i386_loopne16()          // Opcode 0xe0
{
	int8_t disp = FETCH();
	int32_t val = (m_address_size)?(--REG32(ECX)):(--REG16(CX));
	if( val != 0 && m_ZF == 0 ) {
		if (m_sreg[CS].d)
		{
			m_eip += disp;
		}
		else
		{
			m_eip = (m_eip + disp) & 0xffff;
		}
		CHANGE_PC(m_eip);
	}
	CYCLES(CYCLES_LOOPNZ);     /* TODO: Timing = 11 + m */
}

void i386_device::i386_loopz16()           // Opcode 0xe1
{
	int8_t disp = FETCH();
	int32_t val = (m_address_size)?(--REG32(ECX)):(--REG16(CX));
	if( val != 0 && m_ZF != 0 ) {
		if (m_sreg[CS].d)
		{
			m_eip += disp;
		}
		else
		{
			m_eip = (m_eip + disp) & 0xffff;
		}
		CHANGE_PC(m_eip);
	}
	CYCLES(CYCLES_LOOPZ);      /* TODO: Timing = 11 + m */
}

void i386_device::i386_mov_rm16_r16()      // Opcode 0x89
{
	uint16_t src;
	uint8_t modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_REG16(modrm);
		STORE_RM16(modrm, src);
		CYCLES(CYCLES_MOV_REG_REG);
	} else {
		uint32_t ea = GetEA(modrm,1);
		src = LOAD_REG16(modrm);
		WRITE16(ea, src);
		CYCLES(CYCLES_MOV_REG_MEM);
	}
}

void i386_device::i386_mov_r16_rm16()      // Opcode 0x8b
{
	uint16_t src;
	uint8_t modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_RM16(modrm);
		STORE_REG16(modrm, src);
		CYCLES(CYCLES_MOV_REG_REG);
	} else {
		uint32_t ea = GetEA(modrm,0);
		src = READ16(ea);
		STORE_REG16(modrm, src);
		CYCLES(CYCLES_MOV_MEM_REG);
	}
}

void i386_device::i386_mov_rm16_i16()      // Opcode 0xc7
{
	uint8_t modrm = FETCH();
	if( modrm >= 0xc0 ) {
		uint16_t value = FETCH16();
		STORE_RM16(modrm, value);
		CYCLES(CYCLES_MOV_IMM_REG);
	} else {
		uint32_t ea = GetEA(modrm,1);
		uint16_t value = FETCH16();
		WRITE16(ea, value);
		CYCLES(CYCLES_MOV_IMM_MEM);
	}
}

void i386_device::i386_mov_ax_m16()        // Opcode 0xa1
{
	uint32_t offset, ea;
	if( m_address_size ) {
		offset = FETCH32();
	} else {
		offset = FETCH16();
	}
	/* TODO: Not sure if this is correct... */
	if( m_segment_prefix ) {
		ea = i386_translate(m_segment_override, offset, 0 );
	} else {
		ea = i386_translate(DS, offset, 0 );
	}
	REG16(AX) = READ16(ea);
	CYCLES(CYCLES_MOV_MEM_ACC);
}

void i386_device::i386_mov_m16_ax()        // Opcode 0xa3
{
	uint32_t offset, ea;
	if( m_address_size ) {
		offset = FETCH32();
	} else {
		offset = FETCH16();
	}
	/* TODO: Not sure if this is correct... */
	if( m_segment_prefix ) {
		ea = i386_translate(m_segment_override, offset, 1 );
	} else {
		ea = i386_translate(DS, offset, 1 );
	}
	WRITE16(ea, REG16(AX) );
	CYCLES(CYCLES_MOV_ACC_MEM);
}

void i386_device::i386_mov_ax_i16()        // Opcode 0xb8
{
	REG16(AX) = FETCH16();
	CYCLES(CYCLES_MOV_IMM_REG);
}

void i386_device::i386_mov_cx_i16()        // Opcode 0xb9
{
	REG16(CX) = FETCH16();
	CYCLES(CYCLES_MOV_IMM_REG);
}

void i386_device::i386_mov_dx_i16()        // Opcode 0xba
{
	REG16(DX) = FETCH16();
	CYCLES(CYCLES_MOV_IMM_REG);
}

void i386_device::i386_mov_bx_i16()        // Opcode 0xbb
{
	REG16(BX) = FETCH16();
	CYCLES(CYCLES_MOV_IMM_REG);
}

void i386_device::i386_mov_sp_i16()        // Opcode 0xbc
{
	REG16(SP) = FETCH16();
	CYCLES(CYCLES_MOV_IMM_REG);
}

void i386_device::i386_mov_bp_i16()        // Opcode 0xbd
{
	REG16(BP) = FETCH16();
	CYCLES(CYCLES_MOV_IMM_REG);
}

void i386_device::i386_mov_si_i16()        // Opcode 0xbe
{
	REG16(SI) = FETCH16();
	CYCLES(CYCLES_MOV_IMM_REG);
}

void i386_device::i386_mov_di_i16()        // Opcode 0xbf
{
	REG16(DI) = FETCH16();
	CYCLES(CYCLES_MOV_IMM_REG);
}

void i386_device::i386_movsw()             // Opcode 0xa5
{
	uint32_t eas, ead;
	uint16_t v;
	if( m_segment_prefix ) {
		eas = i386_translate(m_segment_override, m_address_size ? REG32(ESI) : REG16(SI), 0, 2);
	} else {
		eas = i386_translate(DS, m_address_size ? REG32(ESI) : REG16(SI), 0, 2);
	}
	ead = i386_translate(ES, m_address_size ? REG32(EDI) : REG16(DI), 1, 2);
	v = READ16(eas);
	WRITE16(ead, v);
	BUMP_SI(2);
	BUMP_DI(2);
	CYCLES(CYCLES_MOVS);
}

void i386_device::i386_movsx_r16_rm8()     // Opcode 0x0f be
{
	uint8_t modrm = FETCH();
	if( modrm >= 0xc0 ) {
		int16_t src = (int8_t)LOAD_RM8(modrm);
		STORE_REG16(modrm, src);
		CYCLES(CYCLES_MOVSX_REG_REG);
	} else {
		uint32_t ea = GetEA(modrm,0);
		int16_t src = (int8_t)READ8(ea);
		STORE_REG16(modrm, src);
		CYCLES(CYCLES_MOVSX_MEM_REG);
	}
}

void i386_device::i386_movzx_r16_rm8()     // Opcode 0x0f b6
{
	uint8_t modrm = FETCH();
	if( modrm >= 0xc0 ) {
		uint16_t src = (uint8_t)LOAD_RM8(modrm);
		STORE_REG16(modrm, src);
		CYCLES(CYCLES_MOVZX_REG_REG);
	} else {
		uint32_t ea = GetEA(modrm,0);
		uint16_t src = (uint8_t)READ8(ea);
		STORE_REG16(modrm, src);
		CYCLES(CYCLES_MOVZX_MEM_REG);
	}
}

void i386_device::i386_or_rm16_r16()       // Opcode 0x09
{
	uint16_t src, dst;
	uint8_t modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_REG16(modrm);
		dst = LOAD_RM16(modrm);
		dst = OR16(dst, src);
		STORE_RM16(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		uint32_t ea = GetEA(modrm,1);
		src = LOAD_REG16(modrm);
		dst = READ16(ea);
		dst = OR16(dst, src);
		WRITE16(ea, dst);
		CYCLES(CYCLES_ALU_REG_MEM);
	}
}

void i386_device::i386_or_r16_rm16()       // Opcode 0x0b
{
	uint16_t src, dst;
	uint8_t modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_RM16(modrm);
		dst = LOAD_REG16(modrm);
		dst = OR16(dst, src);
		STORE_REG16(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		uint32_t ea = GetEA(modrm,0);
		src = READ16(ea);
		dst = LOAD_REG16(modrm);
		dst = OR16(dst, src);
		STORE_REG16(modrm, dst);
		CYCLES(CYCLES_ALU_MEM_REG);
	}
}

void i386_device::i386_or_ax_i16()         // Opcode 0x0d
{
	uint16_t src, dst;
	src = FETCH16();
	dst = REG16(AX);
	dst = OR16(dst, src);
	REG16(AX) = dst;
	CYCLES(CYCLES_ALU_IMM_ACC);
}

void i386_device::i386_out_ax_i8()         // Opcode 0xe7
{
	uint16_t port = FETCH();
	uint16_t data = REG16(AX);
	WRITEPORT16(port, data);
	CYCLES(CYCLES_OUT_VAR);
}

void i386_device::i386_out_ax_dx()         // Opcode 0xef
{
	uint16_t port = REG16(DX);
	uint16_t data = REG16(AX);
	WRITEPORT16(port, data);
	CYCLES(CYCLES_OUT);
}

void i386_device::i386_pop_ax()            // Opcode 0x58
{
	uint32_t offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(SS,offset+1) == 0)
		REG16(AX) = POP16();
	else
		FAULT(FAULT_SS,0)
	CYCLES(CYCLES_POP_REG_SHORT);
}

void i386_device::i386_pop_cx()            // Opcode 0x59
{
	uint32_t offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(SS,offset+1) == 0)
		REG16(CX) = POP16();
	else
		FAULT(FAULT_SS,0)
	CYCLES(CYCLES_POP_REG_SHORT);
}

void i386_device::i386_pop_dx()            // Opcode 0x5a
{
	uint32_t offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(SS,offset+1) == 0)
		REG16(DX) = POP16();
	else
		FAULT(FAULT_SS,0)
	CYCLES(CYCLES_POP_REG_SHORT);
}

void i386_device::i386_pop_bx()            // Opcode 0x5b
{
	uint32_t offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(SS,offset+1) == 0)
		REG16(BX) = POP16();
	else
		FAULT(FAULT_SS,0)
	CYCLES(CYCLES_POP_REG_SHORT);
}

void i386_device::i386_pop_sp()            // Opcode 0x5c
{
	uint32_t offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(SS,offset+1) == 0)
		REG16(SP) = POP16();
	else
		FAULT(FAULT_SS,0)
	CYCLES(CYCLES_POP_REG_SHORT);
}

void i386_device::i386_pop_bp()            // Opcode 0x5d
{
	uint32_t offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(SS,offset+1) == 0)
		REG16(BP) = POP16();
	else
		FAULT(FAULT_SS,0)
	CYCLES(CYCLES_POP_REG_SHORT);
}

void i386_device::i386_pop_si()            // Opcode 0x5e
{
	uint32_t offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(SS,offset+1) == 0)
		REG16(SI) = POP16();
	else
		FAULT(FAULT_SS,0)
	CYCLES(CYCLES_POP_REG_SHORT);
}

void i386_device::i386_pop_di()            // Opcode 0x5f
{
	uint32_t offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	if(i386_limit_check(SS,offset+1) == 0)
		REG16(DI) = POP16();
	else
		FAULT(FAULT_SS,0)
	CYCLES(CYCLES_POP_REG_SHORT);
}

bool i386_device::i386_pop_seg16(int segment)
{
	uint32_t ea, offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
	uint16_t value;
	bool fault;
	if(i386_limit_check(SS,offset+1) == 0)
	{
		ea = i386_translate(SS, offset, 0);
		value = READ16(ea);
		i386_sreg_load(value, segment, &fault);
		if(fault) return false;
		if(STACK_32BIT)
			REG32(ESP) = offset + 2;
		else
			REG16(SP) = offset + 2;
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

void i386_device::i386_pop_ds16()          // Opcode 0x1f
{
	i386_pop_seg16(DS);
}

void i386_device::i386_pop_es16()          // Opcode 0x07
{
	i386_pop_seg16(ES);
}

void i386_device::i386_pop_fs16()          // Opcode 0x0f a1
{
	i386_pop_seg16(FS);
}

void i386_device::i386_pop_gs16()          // Opcode 0x0f a9
{
	i386_pop_seg16(GS);
}

void i386_device::i386_pop_ss16()          // Opcode 0x17
{
	if(!i386_pop_seg16(SS)) return;
	if(m_IF != 0) // if external interrupts are enabled
	{
		m_IF = 0;  // reset IF for the next instruction
		m_delayed_interrupt_enable = 1;
	}
}

void i386_device::i386_pop_rm16()          // Opcode 0x8f
{
	uint8_t modrm = FETCH();
	uint16_t value;
	uint32_t ea, offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));

	if(i386_limit_check(SS,offset+1) == 0)
	{
		uint32_t temp_sp = REG32(ESP);
		value = POP16();

		if( modrm >= 0xc0 ) {
			STORE_RM16(modrm, value);
		} else {
			try
			{
				ea = GetEA(modrm,1);
				WRITE16(ea, value);
			}
			catch(uint64_t e)
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

void i386_device::i386_popa()              // Opcode 0x61
{
	uint32_t offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));

	if(i386_limit_check(SS,offset+15) == 0)
	{
		REG16(DI) = POP16();
		REG16(SI) = POP16();
		REG16(BP) = POP16();
		REG16(SP) += 2;
		REG16(BX) = POP16();
		REG16(DX) = POP16();
		REG16(CX) = POP16();
		REG16(AX) = POP16();
	}
	else
		FAULT(FAULT_SS,0)
	CYCLES(CYCLES_POPA);
}

void i386_device::i386_popf()              // Opcode 0x9d
{
	uint32_t value;
	uint32_t current = get_flags();
	uint8_t IOPL = (current >> 12) & 0x03;
	uint32_t mask = 0x7fd5;
	uint32_t offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));

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
			LOGMASKED(LOG_PM_FAULT_GP, "POPFD(%08x): IOPL < 3 while in V86 mode.\n",m_pc);
			FAULT(FAULT_GP,0)  // #GP(0)
		}
		mask &= ~0x00003000;  // IOPL cannot be changed while in V8086 mode
	}

	if(i386_limit_check(SS,offset+1) == 0)
	{
		value = POP16();
		set_flags((current & ~mask) | (value & mask));  // mask out reserved bits
	}
	else
		FAULT(FAULT_SS,0)
	CYCLES(CYCLES_POPF);

	m_auto_clear_RF = false;
}

void i386_device::i386_push_ax()           // Opcode 0x50
{
	uint32_t offset;
	if(STACK_32BIT)
		offset = REG32(ESP) - 2;
	else
		offset = (REG16(SP) - 2) & 0xffff;
	if(i386_limit_check(SS,offset) == 0)
		PUSH16(REG16(AX) );
	else
		FAULT(FAULT_SS,0)
	CYCLES(CYCLES_PUSH_REG_SHORT);
}

void i386_device::i386_push_cx()           // Opcode 0x51
{
	uint32_t offset;
	if(STACK_32BIT)
		offset = REG32(ESP) - 2;
	else
		offset = (REG16(SP) - 2) & 0xffff;
	if(i386_limit_check(SS,offset) == 0)
		PUSH16(REG16(CX) );
	else
		FAULT(FAULT_SS,0)
	CYCLES(CYCLES_PUSH_REG_SHORT);
}

void i386_device::i386_push_dx()           // Opcode 0x52
{
	uint32_t offset;
	if(STACK_32BIT)
		offset = REG32(ESP) - 2;
	else
		offset = (REG16(SP) - 2) & 0xffff;
	if(i386_limit_check(SS,offset) == 0)
		PUSH16(REG16(DX) );
	else
		FAULT(FAULT_SS,0)
	CYCLES(CYCLES_PUSH_REG_SHORT);
}

void i386_device::i386_push_bx()           // Opcode 0x53
{
	uint32_t offset;
	if(STACK_32BIT)
		offset = REG32(ESP) - 2;
	else
		offset = (REG16(SP) - 2) & 0xffff;
	if(i386_limit_check(SS,offset) == 0)
		PUSH16(REG16(BX) );
	else
		FAULT(FAULT_SS,0)
	CYCLES(CYCLES_PUSH_REG_SHORT);
}

void i386_device::i386_push_sp()           // Opcode 0x54
{
	uint32_t offset;
	if(STACK_32BIT)
		offset = REG32(ESP) - 2;
	else
		offset = (REG16(SP) - 2) & 0xffff;
	if(i386_limit_check(SS,offset) == 0)
		PUSH16(REG16(SP) );
	else
		FAULT(FAULT_SS,0)
	CYCLES(CYCLES_PUSH_REG_SHORT);
}

void i386_device::i386_push_bp()           // Opcode 0x55
{
	uint32_t offset;
	if(STACK_32BIT)
		offset = REG32(ESP) - 2;
	else
		offset = (REG16(SP) - 2) & 0xffff;
	if(i386_limit_check(SS,offset) == 0)
		PUSH16(REG16(BP) );
	else
		FAULT(FAULT_SS,0)
	CYCLES(CYCLES_PUSH_REG_SHORT);
}

void i386_device::i386_push_si()           // Opcode 0x56
{
	uint32_t offset;
	if(STACK_32BIT)
		offset = REG32(ESP) - 2;
	else
		offset = (REG16(SP) - 2) & 0xffff;
	if(i386_limit_check(SS,offset) == 0)
		PUSH16(REG16(SI) );
	else
		FAULT(FAULT_SS,0)
	CYCLES(CYCLES_PUSH_REG_SHORT);
}

void i386_device::i386_push_di()           // Opcode 0x57
{
	uint32_t offset;
	if(STACK_32BIT)
		offset = REG32(ESP) - 2;
	else
		offset = (REG16(SP) - 2) & 0xffff;
	if(i386_limit_check(SS,offset) == 0)
		PUSH16(REG16(DI) );
	else
		FAULT(FAULT_SS,0)
	CYCLES(CYCLES_PUSH_REG_SHORT);
}

void i386_device::i386_push_cs16()         // Opcode 0x0e
{
	uint32_t offset;
	if(STACK_32BIT)
		offset = REG32(ESP) - 2;
	else
		offset = (REG16(SP) - 2) & 0xffff;
	if(i386_limit_check(SS,offset) == 0)
		PUSH16(m_sreg[CS].selector );
	else
		FAULT(FAULT_SS,0)
	CYCLES(CYCLES_PUSH_SREG);
}

void i386_device::i386_push_ds16()         // Opcode 0x1e
{
	uint32_t offset;
	if(STACK_32BIT)
		offset = REG32(ESP) - 2;
	else
		offset = (REG16(SP) - 2) & 0xffff;
	if(i386_limit_check(SS,offset) == 0)
		PUSH16(m_sreg[DS].selector );
	else
		FAULT(FAULT_SS,0)
	CYCLES(CYCLES_PUSH_SREG);
}

void i386_device::i386_push_es16()         // Opcode 0x06
{
	uint32_t offset;
	if(STACK_32BIT)
		offset = REG32(ESP) - 2;
	else
		offset = (REG16(SP) - 2) & 0xffff;
	if(i386_limit_check(SS,offset) == 0)
		PUSH16(m_sreg[ES].selector );
	else
		FAULT(FAULT_SS,0)
	CYCLES(CYCLES_PUSH_SREG);
}

void i386_device::i386_push_fs16()         // Opcode 0x0f a0
{
	uint32_t offset;
	if(STACK_32BIT)
		offset = REG32(ESP) - 2;
	else
		offset = (REG16(SP) - 2) & 0xffff;
	if(i386_limit_check(SS,offset) == 0)
		PUSH16(m_sreg[FS].selector );
	else
		FAULT(FAULT_SS,0)
	CYCLES(CYCLES_PUSH_SREG);
}

void i386_device::i386_push_gs16()         // Opcode 0x0f a8
{
	uint32_t offset;
	if(STACK_32BIT)
		offset = REG32(ESP) - 2;
	else
		offset = (REG16(SP) - 2) & 0xffff;
	if(i386_limit_check(SS,offset) == 0)
		PUSH16(m_sreg[GS].selector );
	else
		FAULT(FAULT_SS,0)
	CYCLES(CYCLES_PUSH_SREG);
}

void i386_device::i386_push_ss16()         // Opcode 0x16
{
	uint32_t offset;
	if(STACK_32BIT)
		offset = REG32(ESP) - 2;
	else
		offset = (REG16(SP) - 2) & 0xffff;
	if(i386_limit_check(SS,offset) == 0)
		PUSH16(m_sreg[SS].selector );
	else
		FAULT(FAULT_SS,0)
	CYCLES(CYCLES_PUSH_SREG);
}

void i386_device::i386_push_i16()          // Opcode 0x68
{
	uint16_t value = FETCH16();
	uint32_t offset;
	if(STACK_32BIT)
		offset = REG32(ESP) - 2;
	else
		offset = (REG16(SP) - 2) & 0xffff;
	if(i386_limit_check(SS,offset) == 0)
		PUSH16(value);
	else
		FAULT(FAULT_SS,0)
	CYCLES(CYCLES_PUSH_IMM);
}

void i386_device::i386_pusha()             // Opcode 0x60
{
	uint16_t temp = REG16(SP);
	uint32_t offset;
	if(STACK_32BIT)
		offset = REG32(ESP) - 16;
	else
		offset = (REG16(SP) - 16) & 0xffff;
	if(i386_limit_check(SS,offset) == 0)
	{
		PUSH16(REG16(AX) );
		PUSH16(REG16(CX) );
		PUSH16(REG16(DX) );
		PUSH16(REG16(BX) );
		PUSH16(temp );
		PUSH16(REG16(BP) );
		PUSH16(REG16(SI) );
		PUSH16(REG16(DI) );
	}
	else
		FAULT(FAULT_SS,0)
	CYCLES(CYCLES_PUSHA);
}

void i386_device::i386_pushf()             // Opcode 0x9c
{
	uint32_t offset;
	if(STACK_32BIT)
		offset = REG32(ESP) - 2;
	else
		offset = (REG16(SP) - 2) & 0xffff;
	if(i386_limit_check(SS,offset) == 0)
		PUSH16(get_flags() & 0xffff );
	else
		FAULT(FAULT_SS,0)
	CYCLES(CYCLES_PUSHF);
}

void i386_device::i386_ret_near16_i16()    // Opcode 0xc2
{
	int16_t disp = FETCH16();
	m_eip = POP16();
	REG16(SP) += disp;
	CHANGE_PC(m_eip);
	CYCLES(CYCLES_RET_IMM);        /* TODO: Timing = 10 + m */
}

void i386_device::i386_ret_near16()        // Opcode 0xc3
{
	m_eip = POP16();
	CHANGE_PC(m_eip);
	CYCLES(CYCLES_RET);        /* TODO: Timing = 10 + m */
}

void i386_device::i386_sbb_rm16_r16()      // Opcode 0x19
{
	uint16_t src, dst;
	uint8_t modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_REG16(modrm);
		dst = LOAD_RM16(modrm);
		dst = SBB16(dst, src, m_CF);
		STORE_RM16(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		uint32_t ea = GetEA(modrm,1);
		src = LOAD_REG16(modrm);
		dst = READ16(ea);
		dst = SBB16(dst, src, m_CF);
		WRITE16(ea, dst);
		CYCLES(CYCLES_ALU_REG_MEM);
	}
}

void i386_device::i386_sbb_r16_rm16()      // Opcode 0x1b
{
	uint16_t src, dst;
	uint8_t modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_RM16(modrm);
		dst = LOAD_REG16(modrm);
		dst = SBB16(dst, src, m_CF);
		STORE_REG16(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		uint32_t ea = GetEA(modrm,0);
		src = READ16(ea);
		dst = LOAD_REG16(modrm);
		dst = SBB16(dst, src, m_CF);
		STORE_REG16(modrm, dst);
		CYCLES(CYCLES_ALU_MEM_REG);
	}
}

void i386_device::i386_sbb_ax_i16()        // Opcode 0x1d
{
	uint16_t src, dst;
	src = FETCH16();
	dst = REG16(AX);
	dst = SBB16(dst, src, m_CF);
	REG16(AX) = dst;
	CYCLES(CYCLES_ALU_IMM_ACC);
}

void i386_device::i386_scasw()             // Opcode 0xaf
{
	uint32_t eas;
	uint16_t src, dst;
	eas = i386_translate(ES, m_address_size ? REG32(EDI) : REG16(DI), 0 );
	src = READ16(eas);
	dst = REG16(AX);
	SUB16(dst, src);
	BUMP_DI(2);
	CYCLES(CYCLES_SCAS);
}

void i386_device::i386_shld16_i8()         // Opcode 0x0f a4
{
	uint8_t modrm = FETCH();
	if( modrm >= 0xc0 ) {
		uint16_t dst = LOAD_RM16(modrm);
		uint16_t upper = LOAD_REG16(modrm);
		uint8_t shift = FETCH();
		shift &= 31;
		if( shift == 0 ) {
		} else if( shift > 15 ) {
			m_CF = (upper & (1 << (16-shift))) ? 1 : 0;
			// ppro and above should be (dst >> (32-shift))
			dst = (upper << (shift-16)) | (upper >> (32-shift));
			m_OF = m_CF ^ (dst >> 15);
			SetSZPF16(dst);
		} else {
			m_CF = (dst & (1 << (16-shift))) ? 1 : 0;
			dst = (dst << shift) | (upper >> (16-shift));
			m_OF = m_CF ^ (dst >> 15);
			SetSZPF16(dst);
		}
		STORE_RM16(modrm, dst);
		CYCLES(CYCLES_SHLD_REG);
	} else {
		uint32_t ea = GetEA(modrm,1);
		uint16_t dst = READ16(ea);
		uint16_t upper = LOAD_REG16(modrm);
		uint8_t shift = FETCH();
		shift &= 31;
		if( shift == 0 ) {
		} else if( shift > 15 ) {
			m_CF = (upper & (1 << (16-shift))) ? 1 : 0;
			dst = (upper << (shift-16)) | (upper >> (32-shift));
			m_OF = m_CF ^ (dst >> 15);
			SetSZPF16(dst);
		} else {
			m_CF = (dst & (1 << (16-shift))) ? 1 : 0;
			dst = (dst << shift) | (upper >> (16-shift));
			m_OF = m_CF ^ (dst >> 15);
			SetSZPF16(dst);
		}
		WRITE16(ea, dst);
		CYCLES(CYCLES_SHLD_MEM);
	}
}

void i386_device::i386_shld16_cl()         // Opcode 0x0f a5
{
	uint8_t modrm = FETCH();
	if( modrm >= 0xc0 ) {
		uint16_t dst = LOAD_RM16(modrm);
		uint16_t upper = LOAD_REG16(modrm);
		uint8_t shift = REG8(CL);
		shift &= 31;
		if( shift == 0 ) {
		} else if( shift > 15 ) {
			m_CF = (upper & (1 << (16-shift))) ? 1 : 0;
			dst = (upper << (shift-16)) | (upper >> (32-shift));
			m_OF = m_CF ^ (dst >> 15);
			SetSZPF16(dst);
		} else {
			m_CF = (dst & (1 << (16-shift))) ? 1 : 0;
			dst = (dst << shift) | (upper >> (16-shift));
			m_OF = m_CF ^ (dst >> 15);
			SetSZPF16(dst);
		}
		STORE_RM16(modrm, dst);
		CYCLES(CYCLES_SHLD_REG);
	} else {
		uint32_t ea = GetEA(modrm,1);
		uint16_t dst = READ16(ea);
		uint16_t upper = LOAD_REG16(modrm);
		uint8_t shift = REG8(CL);
		shift &= 31;
		if( shift == 0 ) {
		} else if( shift > 15 ) {
			m_CF = (upper & (1 << (16-shift))) ? 1 : 0;
			dst = (upper << (shift-16)) | (upper >> (32-shift));
			m_OF = m_CF ^ (dst >> 15);
			SetSZPF16(dst);
		} else {
			m_CF = (dst & (1 << (16-shift))) ? 1 : 0;
			dst = (dst << shift) | (upper >> (16-shift));
			m_OF = m_CF ^ (dst >> 15);
			SetSZPF16(dst);
		}
		WRITE16(ea, dst);
		CYCLES(CYCLES_SHLD_MEM);
	}
}

void i386_device::i386_shrd16_i8()         // Opcode 0x0f ac
{
	uint8_t modrm = FETCH();
	if( modrm >= 0xc0 ) {
		uint16_t dst = LOAD_RM16(modrm);
		uint16_t upper = LOAD_REG16(modrm);
		uint8_t shift = FETCH();
		shift &= 31;
		if( shift == 0) {
		} else if( shift > 15 ) {
			m_CF = (upper & (1 << (shift-1))) ? 1 : 0;
			dst = (upper >> (shift-16)) | (upper << (32-shift));
			m_OF = ((dst >> 15) ^ (dst >> 14)) & 1;
			SetSZPF16(dst);
		} else {
			m_CF = (dst & (1 << (shift-1))) ? 1 : 0;
			dst = (dst >> shift) | (upper << (16-shift));
			m_OF = ((dst >> 15) ^ (dst >> 14)) & 1;
			SetSZPF16(dst);
		}
		STORE_RM16(modrm, dst);
		CYCLES(CYCLES_SHRD_REG);
	} else {
		uint32_t ea = GetEA(modrm,1);
		uint16_t dst = READ16(ea);
		uint16_t upper = LOAD_REG16(modrm);
		uint8_t shift = FETCH();
		shift &= 31;
		if( shift == 0) {
		} else if( shift > 15 ) {
			m_CF = (upper & (1 << (shift-1))) ? 1 : 0;
			dst = (upper >> (shift-16)) | (upper << (32-shift));
			m_OF = ((dst >> 15) ^ (dst >> 14)) & 1;
			SetSZPF16(dst);
		} else {
			m_CF = (dst & (1 << (shift-1))) ? 1 : 0;
			dst = (dst >> shift) | (upper << (16-shift));
			m_OF = ((dst >> 15) ^ (dst >> 14)) & 1;
			SetSZPF16(dst);
		}
		WRITE16(ea, dst);
		CYCLES(CYCLES_SHRD_MEM);
	}
}

void i386_device::i386_shrd16_cl()         // Opcode 0x0f ad
{
	uint8_t modrm = FETCH();
	if( modrm >= 0xc0 ) {
		uint16_t dst = LOAD_RM16(modrm);
		uint16_t upper = LOAD_REG16(modrm);
		uint8_t shift = REG8(CL);
		shift &= 31;
		if( shift == 0) {
		} else if( shift > 15 ) {
			m_CF = (upper & (1 << (shift-1))) ? 1 : 0;
			dst = (upper >> (shift-16)) | (upper << (32-shift));
			m_OF = ((dst >> 15) ^ (dst >> 14)) & 1;
			SetSZPF16(dst);
		} else {
			m_CF = (dst & (1 << (shift-1))) ? 1 : 0;
			dst = (dst >> shift) | (upper << (16-shift));
			m_OF = ((dst >> 15) ^ (dst >> 14)) & 1;
			SetSZPF16(dst);
		}
		STORE_RM16(modrm, dst);
		CYCLES(CYCLES_SHRD_REG);
	} else {
		uint32_t ea = GetEA(modrm,1);
		uint16_t dst = READ16(ea);
		uint16_t upper = LOAD_REG16(modrm);
		uint8_t shift = REG8(CL);
		shift &= 31;
		if( shift == 0) {
		} else if( shift > 15 ) {
			m_CF = (upper & (1 << (shift-1))) ? 1 : 0;
			dst = (upper >> (shift-16)) | (upper << (32-shift));
			m_OF = ((dst >> 15) ^ (dst >> 14)) & 1;
			SetSZPF16(dst);
		} else {
			m_CF = (dst & (1 << (shift-1))) ? 1 : 0;
			dst = (dst >> shift) | (upper << (16-shift));
			m_OF = ((dst >> 15) ^ (dst >> 14)) & 1;
			SetSZPF16(dst);
		}
		WRITE16(ea, dst);
		CYCLES(CYCLES_SHRD_MEM);
	}
}

void i386_device::i386_stosw()             // Opcode 0xab
{
	uint32_t ead;
	ead = i386_translate(ES, m_address_size ? REG32(EDI) : REG16(DI), 1 );
	WRITE16(ead, REG16(AX));
	BUMP_DI(2);
	CYCLES(CYCLES_STOS);
}

void i386_device::i386_sub_rm16_r16()      // Opcode 0x29
{
	uint16_t src, dst;
	uint8_t modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_REG16(modrm);
		dst = LOAD_RM16(modrm);
		dst = SUB16(dst, src);
		STORE_RM16(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		uint32_t ea = GetEA(modrm,1);
		src = LOAD_REG16(modrm);
		dst = READ16(ea);
		dst = SUB16(dst, src);
		WRITE16(ea, dst);
		CYCLES(CYCLES_ALU_REG_MEM);
	}
}

void i386_device::i386_sub_r16_rm16()      // Opcode 0x2b
{
	uint16_t src, dst;
	uint8_t modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_RM16(modrm);
		dst = LOAD_REG16(modrm);
		dst = SUB16(dst, src);
		STORE_REG16(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		uint32_t ea = GetEA(modrm,0);
		src = READ16(ea);
		dst = LOAD_REG16(modrm);
		dst = SUB16(dst, src);
		STORE_REG16(modrm, dst);
		CYCLES(CYCLES_ALU_MEM_REG);
	}
}

void i386_device::i386_sub_ax_i16()        // Opcode 0x2d
{
	uint16_t src, dst;
	src = FETCH16();
	dst = REG16(AX);
	dst = SUB16(dst, src);
	REG16(AX) = dst;
	CYCLES(CYCLES_ALU_IMM_ACC);
}

void i386_device::i386_test_ax_i16()       // Opcode 0xa9
{
	uint16_t src = FETCH16();
	uint16_t dst = REG16(AX);
	dst = src & dst;
	SetSZPF16(dst);
	m_CF = 0;
	m_OF = 0;
	CYCLES(CYCLES_TEST_IMM_ACC);
}

void i386_device::i386_test_rm16_r16()     // Opcode 0x85
{
	uint16_t src, dst;
	uint8_t modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_REG16(modrm);
		dst = LOAD_RM16(modrm);
		dst = src & dst;
		SetSZPF16(dst);
		m_CF = 0;
		m_OF = 0;
		CYCLES(CYCLES_TEST_REG_REG);
	} else {
		uint32_t ea = GetEA(modrm,0);
		src = LOAD_REG16(modrm);
		dst = READ16(ea);
		dst = src & dst;
		SetSZPF16(dst);
		m_CF = 0;
		m_OF = 0;
		CYCLES(CYCLES_TEST_REG_MEM);
	}
}

void i386_device::i386_xchg_ax_cx()        // Opcode 0x91
{
	uint16_t temp;
	temp = REG16(AX);
	REG16(AX) = REG16(CX);
	REG16(CX) = temp;
	CYCLES(CYCLES_XCHG_REG_REG);
}

void i386_device::i386_xchg_ax_dx()        // Opcode 0x92
{
	uint16_t temp;
	temp = REG16(AX);
	REG16(AX) = REG16(DX);
	REG16(DX) = temp;
	CYCLES(CYCLES_XCHG_REG_REG);
}

void i386_device::i386_xchg_ax_bx()        // Opcode 0x93
{
	uint16_t temp;
	temp = REG16(AX);
	REG16(AX) = REG16(BX);
	REG16(BX) = temp;
	CYCLES(CYCLES_XCHG_REG_REG);
}

void i386_device::i386_xchg_ax_sp()        // Opcode 0x94
{
	uint16_t temp;
	temp = REG16(AX);
	REG16(AX) = REG16(SP);
	REG16(SP) = temp;
	CYCLES(CYCLES_XCHG_REG_REG);
}

void i386_device::i386_xchg_ax_bp()        // Opcode 0x95
{
	uint16_t temp;
	temp = REG16(AX);
	REG16(AX) = REG16(BP);
	REG16(BP) = temp;
	CYCLES(CYCLES_XCHG_REG_REG);
}

void i386_device::i386_xchg_ax_si()        // Opcode 0x96
{
	uint16_t temp;
	temp = REG16(AX);
	REG16(AX) = REG16(SI);
	REG16(SI) = temp;
	CYCLES(CYCLES_XCHG_REG_REG);
}

void i386_device::i386_xchg_ax_di()        // Opcode 0x97
{
	uint16_t temp;
	temp = REG16(AX);
	REG16(AX) = REG16(DI);
	REG16(DI) = temp;
	CYCLES(CYCLES_XCHG_REG_REG);
}

void i386_device::i386_xchg_r16_rm16()     // Opcode 0x87
{
	uint8_t modrm = FETCH();
	if( modrm >= 0xc0 ) {
		uint16_t src = LOAD_RM16(modrm);
		uint16_t dst = LOAD_REG16(modrm);
		STORE_REG16(modrm, src);
		STORE_RM16(modrm, dst);
		CYCLES(CYCLES_XCHG_REG_REG);
	} else {
		uint32_t ea = GetEA(modrm,1);
		uint16_t src = READ16(ea);
		uint16_t dst = LOAD_REG16(modrm);
		STORE_REG16(modrm, src);
		WRITE16(ea, dst);
		CYCLES(CYCLES_XCHG_REG_MEM);
	}
}

void i386_device::i386_xor_rm16_r16()      // Opcode 0x31
{
	uint16_t src, dst;
	uint8_t modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_REG16(modrm);
		dst = LOAD_RM16(modrm);
		dst = XOR16(dst, src);
		STORE_RM16(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		uint32_t ea = GetEA(modrm,1);
		src = LOAD_REG16(modrm);
		dst = READ16(ea);
		dst = XOR16(dst, src);
		WRITE16(ea, dst);
		CYCLES(CYCLES_ALU_REG_MEM);
	}
}

void i386_device::i386_xor_r16_rm16()      // Opcode 0x33
{
	uint16_t src, dst;
	uint8_t modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_RM16(modrm);
		dst = LOAD_REG16(modrm);
		dst = XOR16(dst, src);
		STORE_REG16(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		uint32_t ea = GetEA(modrm,0);
		src = READ16(ea);
		dst = LOAD_REG16(modrm);
		dst = XOR16(dst, src);
		STORE_REG16(modrm, dst);
		CYCLES(CYCLES_ALU_MEM_REG);
	}
}

void i386_device::i386_xor_ax_i16()        // Opcode 0x35
{
	uint16_t src, dst;
	src = FETCH16();
	dst = REG16(AX);
	dst = XOR16(dst, src);
	REG16(AX) = dst;
	CYCLES(CYCLES_ALU_IMM_ACC);
}



void i386_device::i386_group81_16()        // Opcode 0x81
{
	uint32_t ea;
	uint16_t src, dst;
	uint8_t modrm = FETCH();

	switch( (modrm >> 3) & 0x7 )
	{
		case 0:     // ADD Rm16, i16
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM16(modrm);
				src = FETCH16();
				dst = ADD16(dst, src);
				STORE_RM16(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm,1);
				dst = READ16(ea);
				src = FETCH16();
				dst = ADD16(dst, src);
				WRITE16(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 1:     // OR Rm16, i16
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM16(modrm);
				src = FETCH16();
				dst = OR16(dst, src);
				STORE_RM16(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm,1);
				dst = READ16(ea);
				src = FETCH16();
				dst = OR16(dst, src);
				WRITE16(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 2:     // ADC Rm16, i16
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM16(modrm);
				src = FETCH16();
				dst = ADC16(dst, src, m_CF);
				STORE_RM16(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm,1);
				dst = READ16(ea);
				src = FETCH16();
				dst = ADC16(dst, src, m_CF);
				WRITE16(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 3:     // SBB Rm16, i16
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM16(modrm);
				src = FETCH16();
				dst = SBB16(dst, src, m_CF);
				STORE_RM16(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm,1);
				dst = READ16(ea);
				src = FETCH16();
				dst = SBB16(dst, src, m_CF);
				WRITE16(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 4:     // AND Rm16, i16
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM16(modrm);
				src = FETCH16();
				dst = AND16(dst, src);
				STORE_RM16(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm,1);
				dst = READ16(ea);
				src = FETCH16();
				dst = AND16(dst, src);
				WRITE16(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 5:     // SUB Rm16, i16
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM16(modrm);
				src = FETCH16();
				dst = SUB16(dst, src);
				STORE_RM16(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm,1);
				dst = READ16(ea);
				src = FETCH16();
				dst = SUB16(dst, src);
				WRITE16(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 6:     // XOR Rm16, i16
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM16(modrm);
				src = FETCH16();
				dst = XOR16(dst, src);
				STORE_RM16(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm,1);
				dst = READ16(ea);
				src = FETCH16();
				dst = XOR16(dst, src);
				WRITE16(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 7:     // CMP Rm16, i16
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM16(modrm);
				src = FETCH16();
				SUB16(dst, src);
				CYCLES(CYCLES_CMP_REG_REG);
			} else {
				ea = GetEA(modrm,0);
				dst = READ16(ea);
				src = FETCH16();
				SUB16(dst, src);
				CYCLES(CYCLES_CMP_REG_MEM);
			}
			break;
	}
}

void i386_device::i386_group83_16()        // Opcode 0x83
{
	uint32_t ea;
	uint16_t src, dst;
	uint8_t modrm = FETCH();

	switch( (modrm >> 3) & 0x7 )
	{
		case 0:     // ADD Rm16, i16
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM16(modrm);
				src = (uint16_t)(int16_t)(int8_t)FETCH();
				dst = ADD16(dst, src);
				STORE_RM16(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm,1);
				dst = READ16(ea);
				src = (uint16_t)(int16_t)(int8_t)FETCH();
				dst = ADD16(dst, src);
				WRITE16(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 1:     // OR Rm16, i16
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM16(modrm);
				src = (uint16_t)(int16_t)(int8_t)FETCH();
				dst = OR16(dst, src);
				STORE_RM16(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm,1);
				dst = READ16(ea);
				src = (uint16_t)(int16_t)(int8_t)FETCH();
				dst = OR16(dst, src);
				WRITE16(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 2:     // ADC Rm16, i16
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM16(modrm);
				src = (uint16_t)(int16_t)(int8_t)FETCH();
				dst = ADC16(dst, src, m_CF);
				STORE_RM16(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm,1);
				dst = READ16(ea);
				src = (uint16_t)(int16_t)(int8_t)FETCH();
				dst = ADC16(dst, src, m_CF);
				WRITE16(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 3:     // SBB Rm16, i16
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM16(modrm);
				src = ((uint16_t)(int16_t)(int8_t)FETCH());
				dst = SBB16(dst, src, m_CF);
				STORE_RM16(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm,1);
				dst = READ16(ea);
				src = ((uint16_t)(int16_t)(int8_t)FETCH());
				dst = SBB16(dst, src, m_CF);
				WRITE16(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 4:     // AND Rm16, i16
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM16(modrm);
				src = (uint16_t)(int16_t)(int8_t)FETCH();
				dst = AND16(dst, src);
				STORE_RM16(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm,1);
				dst = READ16(ea);
				src = (uint16_t)(int16_t)(int8_t)FETCH();
				dst = AND16(dst, src);
				WRITE16(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 5:     // SUB Rm16, i16
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM16(modrm);
				src = (uint16_t)(int16_t)(int8_t)FETCH();
				dst = SUB16(dst, src);
				STORE_RM16(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm,1);
				dst = READ16(ea);
				src = (uint16_t)(int16_t)(int8_t)FETCH();
				dst = SUB16(dst, src);
				WRITE16(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 6:     // XOR Rm16, i16
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM16(modrm);
				src = (uint16_t)(int16_t)(int8_t)FETCH();
				dst = XOR16(dst, src);
				STORE_RM16(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm,1);
				dst = READ16(ea);
				src = (uint16_t)(int16_t)(int8_t)FETCH();
				dst = XOR16(dst, src);
				WRITE16(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 7:     // CMP Rm16, i16
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM16(modrm);
				src = (uint16_t)(int16_t)(int8_t)FETCH();
				SUB16(dst, src);
				CYCLES(CYCLES_CMP_REG_REG);
			} else {
				ea = GetEA(modrm,0);
				dst = READ16(ea);
				src = (uint16_t)(int16_t)(int8_t)FETCH();
				SUB16(dst, src);
				CYCLES(CYCLES_CMP_REG_MEM);
			}
			break;
	}
}

void i386_device::i386_groupC1_16()        // Opcode 0xc1
{
	uint16_t dst;
	uint8_t modrm = FETCH();
	uint8_t shift;

	if( modrm >= 0xc0 ) {
		dst = LOAD_RM16(modrm);
		shift = FETCH() & 0x1f;
		dst = i386_shift_rotate16(modrm, dst, shift);
		STORE_RM16(modrm, dst);
	} else {
		uint32_t ea = GetEA(modrm,1);
		dst = READ16(ea);
		shift = FETCH() & 0x1f;
		dst = i386_shift_rotate16(modrm, dst, shift);
		WRITE16(ea, dst);
	}
}

void i386_device::i386_groupD1_16()        // Opcode 0xd1
{
	uint16_t dst;
	uint8_t modrm = FETCH();

	if( modrm >= 0xc0 ) {
		dst = LOAD_RM16(modrm);
		dst = i386_shift_rotate16(modrm, dst, 1);
		STORE_RM16(modrm, dst);
	} else {
		uint32_t ea = GetEA(modrm,1);
		dst = READ16(ea);
		dst = i386_shift_rotate16(modrm, dst, 1);
		WRITE16(ea, dst);
	}
}

void i386_device::i386_groupD3_16()        // Opcode 0xd3
{
	uint16_t dst;
	uint8_t modrm = FETCH();

	if( modrm >= 0xc0 ) {
		dst = LOAD_RM16(modrm);
		dst = i386_shift_rotate16(modrm, dst, REG8(CL));
		STORE_RM16(modrm, dst);
	} else {
		uint32_t ea = GetEA(modrm,1);
		dst = READ16(ea);
		dst = i386_shift_rotate16(modrm, dst, REG8(CL));
		WRITE16(ea, dst);
	}
}

void i386_device::i386_groupF7_16()        // Opcode 0xf7
{
	uint8_t modrm = FETCH();

	switch( (modrm >> 3) & 0x7 )
	{
		case 0:         /* TEST Rm16, i16 */
			if( modrm >= 0xc0 ) {
				uint16_t dst = LOAD_RM16(modrm);
				uint16_t src = FETCH16();
				dst &= src;
				m_CF = m_OF = m_AF = 0;
				SetSZPF16(dst);
				CYCLES(CYCLES_TEST_IMM_REG);
			} else {
				uint32_t ea = GetEA(modrm,0);
				uint16_t dst = READ16(ea);
				uint16_t src = FETCH16();
				dst &= src;
				m_CF = m_OF = m_AF = 0;
				SetSZPF16(dst);
				CYCLES(CYCLES_TEST_IMM_MEM);
			}
			break;
		case 2:         /* NOT Rm16 */
			if( modrm >= 0xc0 ) {
				uint16_t dst = LOAD_RM16(modrm);
				dst = ~dst;
				STORE_RM16(modrm, dst);
				CYCLES(CYCLES_NOT_REG);
			} else {
				uint32_t ea = GetEA(modrm,1);
				uint16_t dst = READ16(ea);
				dst = ~dst;
				WRITE16(ea, dst);
				CYCLES(CYCLES_NOT_MEM);
			}
			break;
		case 3:         /* NEG Rm16 */
			if( modrm >= 0xc0 ) {
				uint16_t dst = LOAD_RM16(modrm);
				dst = SUB16(0, dst );
				STORE_RM16(modrm, dst);
				CYCLES(CYCLES_NEG_REG);
			} else {
				uint32_t ea = GetEA(modrm,1);
				uint16_t dst = READ16(ea);
				dst = SUB16(0, dst );
				WRITE16(ea, dst);
				CYCLES(CYCLES_NEG_MEM);
			}
			break;
		case 4:         /* MUL AX, Rm16 */
			{
				uint32_t result;
				uint16_t src, dst;
				if( modrm >= 0xc0 ) {
					src = LOAD_RM16(modrm);
					CYCLES(CYCLES_MUL16_ACC_REG);      /* TODO: Correct multiply timing */
				} else {
					uint32_t ea = GetEA(modrm,0);
					src = READ16(ea);
					CYCLES(CYCLES_MUL16_ACC_MEM);      /* TODO: Correct multiply timing */
				}

				dst = REG16(AX);
				result = (uint32_t)src * (uint32_t)dst;
				REG16(DX) = (uint16_t)(result >> 16);
				REG16(AX) = (uint16_t)result;

				m_CF = m_OF = (REG16(DX) != 0);
			}
			break;
		case 5:         /* IMUL AX, Rm16 */
			{
				int32_t result;
				int32_t src, dst;
				if( modrm >= 0xc0 ) {
					src = (int32_t)(int16_t)LOAD_RM16(modrm);
					CYCLES(CYCLES_IMUL16_ACC_REG);     /* TODO: Correct multiply timing */
				} else {
					uint32_t ea = GetEA(modrm,0);
					src = (int32_t)(int16_t)READ16(ea);
					CYCLES(CYCLES_IMUL16_ACC_MEM);     /* TODO: Correct multiply timing */
				}

				dst = (int32_t)(int16_t)REG16(AX);
				result = src * dst;

				REG16(DX) = (uint16_t)(result >> 16);
				REG16(AX) = (uint16_t)result;

				m_CF = m_OF = !(result == (int32_t)(int16_t)result);
			}
			break;
		case 6:         /* DIV AX, Rm16 */
			{
				uint32_t quotient, remainder, result;
				uint16_t src;
				if( modrm >= 0xc0 ) {
					src = LOAD_RM16(modrm);
					CYCLES(CYCLES_DIV16_ACC_REG);
				} else {
					uint32_t ea = GetEA(modrm,0);
					src = READ16(ea);
					CYCLES(CYCLES_DIV16_ACC_MEM);
				}

				quotient = ((uint32_t)(REG16(DX)) << 16) | (uint32_t)(REG16(AX));
				if( src ) {
					remainder = quotient % (uint32_t)src;
					result = quotient / (uint32_t)src;
					if( result > 0xffff ) {
						/* TODO: Divide error */
					} else {
						REG16(DX) = (uint16_t)remainder;
						REG16(AX) = (uint16_t)result;

						// this flag is actually undefined, enable on non-cyrix
						if (m_cpuid_id0 != 0x69727943)
							m_CF = 1;
					}
				} else {
					i386_trap(0, 0, 0);
				}
			}
			break;
		case 7:         /* IDIV AX, Rm16 */
			{
				int32_t quotient, remainder, result;
				uint16_t src;
				if( modrm >= 0xc0 ) {
					src = LOAD_RM16(modrm);
					CYCLES(CYCLES_IDIV16_ACC_REG);
				} else {
					uint32_t ea = GetEA(modrm,0);
					src = READ16(ea);
					CYCLES(CYCLES_IDIV16_ACC_MEM);
				}

				quotient = (((int32_t)REG16(DX)) << 16) | ((uint32_t)REG16(AX));
				if( src ) {
					remainder = quotient % (int32_t)(int16_t)src;
					result = quotient / (int32_t)(int16_t)src;
					if( result > 0xffff ) {
						/* TODO: Divide error */
					} else {
						REG16(DX) = (uint16_t)remainder;
						REG16(AX) = (uint16_t)result;

						// this flag is actually undefined, enable on non-cyrix
						if (m_cpuid_id0 != 0x69727943)
							m_CF = 1;
					}
				} else {
					i386_trap(0, 0, 0);
				}
			}
			break;
	}
}

void i386_device::i386_groupFF_16()        // Opcode 0xff
{
	uint8_t modrm = FETCH();

	switch( (modrm >> 3) & 0x7 )
	{
		case 0:         /* INC Rm16 */
			if( modrm >= 0xc0 ) {
				uint16_t dst = LOAD_RM16(modrm);
				dst = INC16(dst);
				STORE_RM16(modrm, dst);
				CYCLES(CYCLES_INC_REG);
			} else {
				uint32_t ea = GetEA(modrm,1);
				uint16_t dst = READ16(ea);
				dst = INC16(dst);
				WRITE16(ea, dst);
				CYCLES(CYCLES_INC_MEM);
			}
			break;
		case 1:         /* DEC Rm16 */
			if( modrm >= 0xc0 ) {
				uint16_t dst = LOAD_RM16(modrm);
				dst = DEC16(dst);
				STORE_RM16(modrm, dst);
				CYCLES(CYCLES_DEC_REG);
			} else {
				uint32_t ea = GetEA(modrm,1);
				uint16_t dst = READ16(ea);
				dst = DEC16(dst);
				WRITE16(ea, dst);
				CYCLES(CYCLES_DEC_MEM);
			}
			break;
		case 2:         /* CALL Rm16 */
			{
				uint16_t address;
				if( modrm >= 0xc0 ) {
					address = LOAD_RM16(modrm);
					CYCLES(CYCLES_CALL_REG);       /* TODO: Timing = 7 + m */
				} else {
					uint32_t ea = GetEA(modrm,0);
					address = READ16(ea);
					CYCLES(CYCLES_CALL_MEM);       /* TODO: Timing = 10 + m */
				}
				PUSH16(m_eip );
				m_eip = address;
				CHANGE_PC(m_eip);
			}
			break;
		case 3:         /* CALL FAR Rm16 */
			{
				uint16_t address, selector;
				if( modrm >= 0xc0 )
				{
					report_invalid_modrm("groupFF_16", modrm);
				}
				else
				{
					uint32_t ea = GetEA(modrm,0);
					address = READ16(ea + 0);
					selector = READ16(ea + 2);
					CYCLES(CYCLES_CALL_MEM_INTERSEG);      /* TODO: Timing = 10 + m */

					if(PROTECTED_MODE && !V8086_MODE)
					{
						i386_protected_mode_call(selector,address,1,0);
					}
					else
					{
						PUSH16(m_sreg[CS].selector );
						PUSH16(m_eip );
						m_sreg[CS].selector = selector;
						m_performed_intersegment_jump = 1;
						i386_load_segment_descriptor(CS );
						m_eip = address;
						CHANGE_PC(m_eip);
					}
				}
			}
			break;
		case 4:         /* JMP Rm16 */
			{
				uint16_t address;
				if( modrm >= 0xc0 ) {
					address = LOAD_RM16(modrm);
					CYCLES(CYCLES_JMP_REG);        /* TODO: Timing = 7 + m */
				} else {
					uint32_t ea = GetEA(modrm,0);
					address = READ16(ea);
					CYCLES(CYCLES_JMP_MEM);        /* TODO: Timing = 10 + m */
				}
				m_eip = address;
				CHANGE_PC(m_eip);
			}
			break;
		case 5:         /* JMP FAR Rm16 */
			{
				uint16_t address, selector;

				if( modrm >= 0xc0 )
				{
					report_invalid_modrm("groupFF_16", modrm);
				}
				else
				{
					uint32_t ea = GetEA(modrm,0);
					address = READ16(ea + 0);
					selector = READ16(ea + 2);
					CYCLES(CYCLES_JMP_MEM_INTERSEG);       /* TODO: Timing = 10 + m */
					if(PROTECTED_MODE && !V8086_MODE)
					{
						i386_protected_mode_jump(selector,address,1,0);
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
		case 6:         /* PUSH Rm16 */
			{
				uint16_t value;
				if( modrm >= 0xc0 ) {
					value = LOAD_RM16(modrm);
				} else {
					uint32_t ea = GetEA(modrm,0);
					value = READ16(ea);
				}
				PUSH16(value);
				CYCLES(CYCLES_PUSH_RM);
			}
			break;
		default:
			report_invalid_modrm("groupFF_16", modrm);
			break;
	}
}

void i386_device::i386_group0F00_16()          // Opcode 0x0f 00
{
	uint32_t address, ea;
	uint8_t modrm = FETCH();
	I386_SREG seg;
	uint8_t result;

	switch( (modrm >> 3) & 0x7 )
	{
		case 0:         /* SLDT */
			if ( PROTECTED_MODE && !V8086_MODE )
			{
				if( modrm >= 0xc0 ) {
					STORE_RM16(modrm, m_ldtr.segment);
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
					STORE_RM16(modrm, m_task.segment);
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
					address = LOAD_RM16(modrm);
					m_ldtr.segment = address;
					CYCLES(CYCLES_LLDT_REG);
				} else {
					ea = GetEA(modrm,0);
					m_ldtr.segment = READ16(ea);
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
					address = LOAD_RM16(modrm);
					m_task.segment = address;
					CYCLES(CYCLES_LTR_REG);
				} else {
					ea = GetEA(modrm,0);
					m_task.segment = READ16(ea);
					CYCLES(CYCLES_LTR_MEM);
				}
				memset(&seg, 0, sizeof(seg));
				seg.selector = m_task.segment;
				i386_load_protected_mode_segment(&seg,nullptr);

				uint32_t addr = ((seg.selector & 4) ? m_ldtr.base : m_gdtr.base) + (seg.selector & ~7) + 5;
				i386_translate_address(TR_READ, false, &addr, nullptr);
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
				result = 1;
				if( modrm >= 0xc0 ) {
					address = LOAD_RM16(modrm);
					CYCLES(CYCLES_VERR_REG);
				} else {
					ea = GetEA(modrm,0);
					address = READ16(ea);
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
				LOGMASKED(LOG_PM_EVENTS, "i386: VERR: Exception - Running in real mode or virtual 8086 mode.\n");
			}
			break;

		case 5:  /* VERW */
			if ( PROTECTED_MODE && !V8086_MODE )
			{
				result = 1;
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
				LOGMASKED(LOG_PM_EVENTS, "i386: VERW: Exception - Running in real mode or virtual 8086 mode.\n");
			}
			break;

		default:
			report_invalid_modrm("group0F00_16", modrm);
			break;
	}
}

void i386_device::i386_group0F01_16()      // Opcode 0x0f 01
{
	uint8_t modrm = FETCH();
	uint16_t address;
	uint32_t ea;

	switch( (modrm >> 3) & 0x7 )
	{
		case 0:         /* SGDT */
			{
				if( modrm >= 0xc0 ) {
					address = LOAD_RM16(modrm);
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
					address = LOAD_RM16(modrm);
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
					address = LOAD_RM16(modrm);
					ea = i386_translate(CS, address, 0 );
				} else {
					ea = GetEA(modrm,0);
				}
				m_gdtr.limit = READ16(ea);
				m_gdtr.base = READ32(ea + 2) & 0xffffff;
				CYCLES(CYCLES_LGDT);
				break;
			}
		case 3:         /* LIDT */
			{
				if(PROTECTED_MODE && m_CPL)
					FAULT(FAULT_GP,0)
				if( modrm >= 0xc0 ) {
					address = LOAD_RM16(modrm);
					ea = i386_translate(CS, address, 0 );
				} else {
					ea = GetEA(modrm,0);
				}
				m_idtr.limit = READ16(ea);
				m_idtr.base = READ32(ea + 2) & 0xffffff;
				CYCLES(CYCLES_LIDT);
				break;
			}
		case 4:         /* SMSW */
			{
				if( modrm >= 0xc0 ) {
					STORE_RM16(modrm, m_cr[0]);
					CYCLES(CYCLES_SMSW_REG);
				} else {
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
				uint16_t b;
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
			report_invalid_modrm("group0F01_16", modrm);
			break;
	}
}

void i386_device::i386_group0FBA_16()      // Opcode 0x0f ba
{
	uint8_t modrm = FETCH();

	switch( (modrm >> 3) & 0x7 )
	{
		case 4:         /* BT Rm16, i8 */
			if( modrm >= 0xc0 ) {
				uint16_t dst = LOAD_RM16(modrm);
				uint8_t bit = FETCH();

				if( dst & (1 << bit) )
					m_CF = 1;
				else
					m_CF = 0;

				CYCLES(CYCLES_BT_IMM_REG);
			} else {
				uint32_t ea = GetEA(modrm,0);
				uint16_t dst = READ16(ea);
				uint8_t bit = FETCH();

				if( dst & (1 << bit) )
					m_CF = 1;
				else
					m_CF = 0;

				CYCLES(CYCLES_BT_IMM_MEM);
			}
			break;
		case 5:         /* BTS Rm16, i8 */
			if( modrm >= 0xc0 ) {
				uint16_t dst = LOAD_RM16(modrm);
				uint8_t bit = FETCH();

				if( dst & (1 << bit) )
					m_CF = 1;
				else
					m_CF = 0;
				dst |= (1 << bit);

				STORE_RM16(modrm, dst);
				CYCLES(CYCLES_BTS_IMM_REG);
			} else {
				uint32_t ea = GetEA(modrm,1);
				uint16_t dst = READ16(ea);
				uint8_t bit = FETCH();

				if( dst & (1 << bit) )
					m_CF = 1;
				else
					m_CF = 0;
				dst |= (1 << bit);

				WRITE16(ea, dst);
				CYCLES(CYCLES_BTS_IMM_MEM);
			}
			break;
		case 6:         /* BTR Rm16, i8 */
			if( modrm >= 0xc0 ) {
				uint16_t dst = LOAD_RM16(modrm);
				uint8_t bit = FETCH();

				if( dst & (1 << bit) )
					m_CF = 1;
				else
					m_CF = 0;
				dst &= ~(1 << bit);

				STORE_RM16(modrm, dst);
				CYCLES(CYCLES_BTR_IMM_REG);
			} else {
				uint32_t ea = GetEA(modrm,1);
				uint16_t dst = READ16(ea);
				uint8_t bit = FETCH();

				if( dst & (1 << bit) )
					m_CF = 1;
				else
					m_CF = 0;
				dst &= ~(1 << bit);

				WRITE16(ea, dst);
				CYCLES(CYCLES_BTR_IMM_MEM);
			}
			break;
		case 7:         /* BTC Rm16, i8 */
			if( modrm >= 0xc0 ) {
				uint16_t dst = LOAD_RM16(modrm);
				uint8_t bit = FETCH();

				if( dst & (1 << bit) )
					m_CF = 1;
				else
					m_CF = 0;
				dst ^= (1 << bit);

				STORE_RM16(modrm, dst);
				CYCLES(CYCLES_BTC_IMM_REG);
			} else {
				uint32_t ea = GetEA(modrm,1);
				uint16_t dst = READ16(ea);
				uint8_t bit = FETCH();

				if( dst & (1 << bit) )
					m_CF = 1;
				else
					m_CF = 0;
				dst ^= (1 << bit);

				WRITE16(ea, dst);
				CYCLES(CYCLES_BTC_IMM_MEM);
			}
			break;
		default:
			report_invalid_modrm("group0FBA_16", modrm);
			break;
	}
}

void i386_device::i386_lar_r16_rm16()  // Opcode 0x0f 0x02
{
	uint8_t modrm = FETCH();
	I386_SREG seg;
	uint8_t type;

	if(PROTECTED_MODE && !V8086_MODE)
	{
		memset(&seg,0,sizeof(seg));
		if(modrm >= 0xc0)
		{
			seg.selector = LOAD_RM16(modrm);
			CYCLES(CYCLES_LAR_REG);
		}
		else
		{
			uint32_t ea = GetEA(modrm,0);
			seg.selector = READ16(ea);
			CYCLES(CYCLES_LAR_MEM);
		}
		if(seg.selector == 0)
		{
			SetZF(0);  // not a valid segment
		//  LOGMASKED(LOG_PM_EVENTS, "i386 (%08x): LAR: Selector %04x is invalid type.\n",m_pc,seg.selector);
		}
		else
		{
			if(!i386_load_protected_mode_segment(&seg,nullptr))
			{
				SetZF(0);
				return;
			}
			uint8_t DPL = (seg.flags >> 5) & 3;
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
		i386_trap(6,0, 0);
		LOGMASKED(LOG_PM_EVENTS, "i386: LAR: Exception - running in real mode or virtual 8086 mode.\n");
	}
}

void i386_device::i386_lsl_r16_rm16()  // Opcode 0x0f 0x03
{
	uint8_t modrm = FETCH();
	uint32_t limit;
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
			uint32_t ea = GetEA(modrm,0);
			seg.selector = READ16(ea);
		}
		if(seg.selector == 0)
		{
			SetZF(0);  // not a valid segment
		}
		else
		{
			uint8_t type;
			if(!i386_load_protected_mode_segment(&seg,nullptr))
			{
				SetZF(0);
				return;
			}
			uint8_t DPL = (seg.flags >> 5) & 3;
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
				STORE_REG16(modrm,limit & 0x0000ffff);
				SetZF(1);
			}
		}
	}
	else
		i386_trap(6, 0, 0);
}

void i386_device::i386_bound_r16_m16_m16() // Opcode 0x62
{
	uint8_t modrm;
	int16_t val, low, high;

	modrm = FETCH();

	if (modrm >= 0xc0)
	{
		low = high = LOAD_RM16(modrm);
	}
	else
	{
		uint32_t ea = GetEA(modrm,0);
		low = READ16(ea + 0);
		high = READ16(ea + 2);
	}
	val = LOAD_REG16(modrm);

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

void i386_device::i386_retf16()            // Opcode 0xcb
{
	if(PROTECTED_MODE && !V8086_MODE)
	{
		i386_protected_mode_retf(0,0);
	}
	else
	{
		m_eip = POP16();
		m_sreg[CS].selector = POP16();
		i386_load_segment_descriptor(CS );
		CHANGE_PC(m_eip);
	}

	CYCLES(CYCLES_RET_INTERSEG);
}

void i386_device::i386_retf_i16()          // Opcode 0xca
{
	uint16_t count = FETCH16();

	if(PROTECTED_MODE && !V8086_MODE)
	{
		i386_protected_mode_retf(count,0);
	}
	else
	{
		m_eip = POP16();
		m_sreg[CS].selector = POP16();
		i386_load_segment_descriptor(CS );
		CHANGE_PC(m_eip);
		REG16(SP) += count;
	}

	CYCLES(CYCLES_RET_IMM_INTERSEG);
}

bool i386_device::i386_load_far_pointer16(int s)
{
	uint8_t modrm = FETCH();
	uint16_t selector;

	if( modrm >= 0xc0 ) {
		//LOGMASKED(LOG_PM_EVENTS, "i386: load_far_pointer16 NYI\n"); // don't log, NT will use this a lot
		i386_trap(6, 0, 0);
		return false;
	} else {
		uint32_t ea = GetEA(modrm,0);
		STORE_REG16(modrm, READ16(ea + 0));
		selector = READ16(ea + 2);
		i386_sreg_load(selector,s,nullptr);
	}
	return true;
}

void i386_device::i386_lds16()             // Opcode 0xc5
{
	if(i386_load_far_pointer16(DS))
		CYCLES(CYCLES_LDS);
}

void i386_device::i386_lss16()             // Opcode 0x0f 0xb2
{
	if(i386_load_far_pointer16(SS))
		CYCLES(CYCLES_LSS);
}

void i386_device::i386_les16()             // Opcode 0xc4
{
	if(i386_load_far_pointer16(ES))
		CYCLES(CYCLES_LES);
}

void i386_device::i386_lfs16()             // Opcode 0x0f 0xb4
{
	if(i386_load_far_pointer16(FS))
		CYCLES(CYCLES_LFS);
}

void i386_device::i386_lgs16()             // Opcode 0x0f 0xb5
{
	if(i386_load_far_pointer16(GS))
		CYCLES(CYCLES_LGS);
}
