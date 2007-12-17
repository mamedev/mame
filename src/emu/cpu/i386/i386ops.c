static UINT8 I386OP(shift_rotate8)(UINT8 modrm, UINT32 value, UINT8 shift)
{
	UINT8 src = value;
	UINT8 dst = value;

	if( shift == 0 ) {
		CYCLES_RM(modrm, 3, 7);
	} else if( shift == 1 ) {

		switch( (modrm >> 3) & 0x7 )
		{
			case 0:			/* ROL rm8, 1 */
				I.CF = (src & 0x80) ? 1 : 0;
				dst = (src << 1) + I.CF;
				I.OF = ((src ^ dst) & 0x80) ? 1 : 0;
				CYCLES_RM(modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 1:			/* ROR rm8, 1 */
				I.CF = (src & 0x1) ? 1 : 0;
				dst = (I.CF << 7) | (src >> 1);
				I.OF = ((src ^ dst) & 0x80) ? 1 : 0;
				CYCLES_RM(modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 2:			/* RCL rm8, 1 */
				dst = (src << 1) + I.CF;
				I.CF = (src & 0x80) ? 1 : 0;
				I.OF = ((src ^ dst) & 0x80) ? 1 : 0;
				CYCLES_RM(modrm, CYCLES_ROTATE_CARRY_REG, CYCLES_ROTATE_CARRY_MEM);
				break;
			case 3:			/* RCR rm8, 1 */
				dst = (I.CF << 7) | (src >> 1);
				I.CF = src & 0x1;
				I.OF = ((src ^ dst) & 0x80) ? 1 : 0;
				CYCLES_RM(modrm, CYCLES_ROTATE_CARRY_REG, CYCLES_ROTATE_CARRY_MEM);
				break;
			case 4:			/* SHL/SAL rm8, 1 */
			case 6:
				dst = src << 1;
				I.CF = (src & 0x80) ? 1 : 0;
				I.OF = (((I.CF << 7) ^ dst) & 0x80) ? 1 : 0;
				SetSZPF8(dst);
				CYCLES_RM(modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 5:			/* SHR rm8, 1 */
				dst = src >> 1;
				I.CF = src & 0x1;
				I.OF = (dst & 0x80) ? 1 : 0;
				SetSZPF8(dst);
				CYCLES_RM(modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 7:			/* SAR rm8, 1 */
				dst = (INT8)(src) >> 1;
				I.CF = src & 0x1;
				I.OF = 0;
				SetSZPF8(dst);
				CYCLES_RM(modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
		}

	} else {

		switch( (modrm >> 3) & 0x7 )
		{
			case 0:			/* ROL rm8, i8 */
				dst = ((src & ((UINT8)0xff >> shift)) << shift) |
					  ((src & ((UINT8)0xff << (8-shift))) >> (8-shift));
				I.CF = (src >> (8-shift)) & 0x1;
				CYCLES_RM(modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 1:			/* ROR rm8, i8 */
				dst = ((src & ((UINT8)0xff << shift)) >> shift) |
					  ((src & ((UINT8)0xff >> (8-shift))) << (8-shift));
				I.CF = (src >> (shift-1)) & 0x1;
				CYCLES_RM(modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 2:			/* RCL rm8, i8 */
				dst = ((src & ((UINT8)0xff >> shift)) << shift) |
					  ((src & ((UINT8)0xff << (9-shift))) >> (9-shift)) |
					  (I.CF << (shift-1));
				I.CF = (src >> (8-shift)) & 0x1;
				CYCLES_RM(modrm, CYCLES_ROTATE_CARRY_REG, CYCLES_ROTATE_CARRY_MEM);
				break;
			case 3:			/* RCR rm8, i8 */
				dst = ((src & ((UINT8)0xff << shift)) >> shift) |
					  ((src & ((UINT8)0xff >> (8-shift))) << (9-shift)) |
					  (I.CF << (8-shift));
				I.CF = (src >> (shift-1)) & 0x1;
				CYCLES_RM(modrm, CYCLES_ROTATE_CARRY_REG, CYCLES_ROTATE_CARRY_MEM);
				break;
			case 4:			/* SHL/SAL rm8, i8 */
			case 6:
				dst = src << shift;
				I.CF = (src & (1 << (8-shift))) ? 1 : 0;
				SetSZPF8(dst);
				CYCLES_RM(modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 5:			/* SHR rm8, i8 */
				dst = src >> shift;
				I.CF = (src & (1 << (shift-1))) ? 1 : 0;
				SetSZPF8(dst);
				CYCLES_RM(modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 7:			/* SAR rm8, i8 */
				dst = (INT8)src >> shift;
				I.CF = (src & (1 << (shift-1))) ? 1 : 0;
				SetSZPF8(dst);
				CYCLES_RM(modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
		}
	}

	return dst;
}



static void I386OP(adc_rm8_r8)(void)		// Opcode 0x10
{
	UINT8 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_REG8(modrm);
		dst = LOAD_RM8(modrm);
		src = ADD8(src, I.CF);
		dst = ADD8(dst, src);
		STORE_RM8(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		src = LOAD_REG8(modrm);
		dst = READ8(ea);
		src = ADD8(src, I.CF);
		dst = ADD8(dst, src);
		WRITE8(ea, dst);
		CYCLES(CYCLES_ALU_REG_MEM);
	}
}

static void I386OP(adc_r8_rm8)(void)		// Opcode 0x12
{
	UINT8 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_RM8(modrm);
		dst = LOAD_REG8(modrm);
		src = ADD8(src, I.CF);
		dst = ADD8(dst, src);
		STORE_REG8(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		src = READ8(ea);
		dst = LOAD_REG8(modrm);
		src = ADD8(src, I.CF);
		dst = ADD8(dst, src);
		STORE_REG8(modrm, dst);
		CYCLES(CYCLES_ALU_MEM_REG);
	}
}

static void I386OP(adc_al_i8)(void)		// Opcode 0x14
{
	UINT8 src, dst;
	src = FETCH();
	dst = REG8(AL);
	src = ADD8(src, I.CF);
	dst = ADD8(dst, src);
	REG8(AL) = dst;
	CYCLES(CYCLES_ALU_IMM_ACC);
}

static void I386OP(add_rm8_r8)(void)		// Opcode 0x00
{
	UINT8 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_REG8(modrm);
		dst = LOAD_RM8(modrm);
		dst = ADD8(dst, src);
		STORE_RM8(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		src = LOAD_REG8(modrm);
		dst = READ8(ea);
		dst = ADD8(dst, src);
		WRITE8(ea, dst);
		CYCLES(CYCLES_ALU_REG_MEM);
	}
}

static void I386OP(add_r8_rm8)(void)		// Opcode 0x02
{
	UINT8 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_RM8(modrm);
		dst = LOAD_REG8(modrm);
		dst = ADD8(dst, src);
		STORE_REG8(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		src = READ8(ea);
		dst = LOAD_REG8(modrm);
		dst = ADD8(dst, src);
		STORE_REG8(modrm, dst);
		CYCLES(CYCLES_ALU_MEM_REG);
	}
}

static void I386OP(add_al_i8)(void)		// Opcode 0x04
{
	UINT8 src, dst;
	src = FETCH();
	dst = REG8(AL);
	dst = ADD8(dst, src);
	REG8(AL) = dst;
	CYCLES(CYCLES_ALU_IMM_ACC);
}

static void I386OP(and_rm8_r8)(void)		// Opcode 0x20
{
	UINT8 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_REG8(modrm);
		dst = LOAD_RM8(modrm);
		dst = AND8(dst, src);
		STORE_RM8(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		src = LOAD_REG8(modrm);
		dst = READ8(ea);
		dst = AND8(dst, src);
		WRITE8(ea, dst);
		CYCLES(CYCLES_ALU_REG_MEM);
	}
}

static void I386OP(and_r8_rm8)(void)		// Opcode 0x22
{
	UINT8 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_RM8(modrm);
		dst = LOAD_REG8(modrm);
		dst = AND8(dst, src);
		STORE_REG8(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		src = READ8(ea);
		dst = LOAD_REG8(modrm);
		dst = AND8(dst, src);
		STORE_REG8(modrm, dst);
		CYCLES(CYCLES_ALU_MEM_REG);
	}
}

static void I386OP(and_al_i8)(void)			// Opcode 0x24
{
	UINT8 src, dst;
	src = FETCH();
	dst = REG8(AL);
	dst = AND8(dst, src);
	REG8(AL) = dst;
	CYCLES(CYCLES_ALU_IMM_ACC);
}

static void I386OP(clc)(void)				// Opcode 0xf8
{
	I.CF = 0;
	CYCLES(CYCLES_CLC);
}

static void I386OP(cld)(void)				// Opcode 0xfc
{
	I.DF = 0;
	CYCLES(CYCLES_CLD);
}

static void I386OP(cli)(void)				// Opcode 0xfa
{
	I.IF = 0;
	CYCLES(CYCLES_CLI);
}

static void I386OP(cmc)(void)				// Opcode 0xf5
{
	I.CF ^= 1;
	CYCLES(CYCLES_CMC);
}

static void I386OP(cmp_rm8_r8)(void)		// Opcode 0x38
{
	UINT8 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_REG8(modrm);
		dst = LOAD_RM8(modrm);
		SUB8(dst, src);
		CYCLES(CYCLES_CMP_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		src = LOAD_REG8(modrm);
		dst = READ8(ea);
		SUB8(dst, src);
		CYCLES(CYCLES_CMP_REG_MEM);
	}
}

static void I386OP(cmp_r8_rm8)(void)		// Opcode 0x3a
{
	UINT8 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_RM8(modrm);
		dst = LOAD_REG8(modrm);
		SUB8(dst, src);
		CYCLES(CYCLES_CMP_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		src = READ8(ea);
		dst = LOAD_REG8(modrm);
		SUB8(dst, src);
		CYCLES(CYCLES_CMP_MEM_REG);
	}
}

static void I386OP(cmp_al_i8)(void)			// Opcode 0x3c
{
	UINT8 src, dst;
	src = FETCH();
	dst = REG8(AL);
	SUB8(dst, src);
	CYCLES(CYCLES_CMP_IMM_ACC);
}

static void I386OP(cmpsb)(void)				// Opcode 0xa6
{
	UINT32 eas, ead;
	UINT8 src, dst;
	if( I.segment_prefix ) {
		eas = i386_translate( I.segment_override, I.address_size ? REG32(ESI) : REG16(SI) );
	} else {
		eas = i386_translate( DS, I.address_size ? REG32(ESI) : REG16(SI) );
	}
	ead = i386_translate( ES, I.address_size ? REG32(EDI) : REG16(DI) );
	src = READ8(eas);
	dst = READ8(ead);
	SUB8(dst, src);
	BUMP_SI(1);
	BUMP_DI(1);
	CYCLES(CYCLES_CMPS);
}

static void I386OP(in_al_i8)(void)			// Opcode 0xe4
{
	UINT16 port = FETCH();
	UINT8 data = READPORT8(port);
	REG8(AL) = data;
	CYCLES(CYCLES_IN_VAR);
}

static void I386OP(in_al_dx)(void)			// Opcode 0xec
{
	UINT16 port = REG16(DX);
	UINT8 data = READPORT8(port);
	REG8(AL) = data;
	CYCLES(CYCLES_IN);
}

static void I386OP(ja_rel8)(void)			// Opcode 0x77
{
	INT8 disp = FETCH();
	if( I.CF == 0 && I.ZF == 0 ) {
		NEAR_BRANCH(disp);
		CYCLES(CYCLES_JCC_DISP8);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_DISP8_NOBRANCH);
	}
}

static void I386OP(jbe_rel8)(void)			// Opcode 0x76
{
	INT8 disp = FETCH();
	if( I.CF != 0 || I.ZF != 0 ) {
		NEAR_BRANCH(disp);
		CYCLES(CYCLES_JCC_DISP8);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_DISP8_NOBRANCH);
	}
}

static void I386OP(jc_rel8)(void)			// Opcode 0x72
{
	INT8 disp = FETCH();
	if( I.CF != 0 ) {
		NEAR_BRANCH(disp);
		CYCLES(CYCLES_JCC_DISP8);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_DISP8_NOBRANCH);
	}
}

static void I386OP(jg_rel8)(void)			// Opcode 0x7f
{
	INT8 disp = FETCH();
	if( I.ZF == 0 && (I.SF == I.OF) ) {
		NEAR_BRANCH(disp);
		CYCLES(CYCLES_JCC_DISP8);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_DISP8_NOBRANCH);
	}
}

static void I386OP(jge_rel8)(void)			// Opcode 0x7d
{
	INT8 disp = FETCH();
	if( (I.SF == I.OF) ) {
		NEAR_BRANCH(disp);
		CYCLES(CYCLES_JCC_DISP8);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_DISP8_NOBRANCH);
	}
}

static void I386OP(jl_rel8)(void)			// Opcode 0x7c
{
	INT8 disp = FETCH();
	if( (I.SF != I.OF) ) {
		NEAR_BRANCH(disp);
		CYCLES(CYCLES_JCC_DISP8);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_DISP8_NOBRANCH);
	}
}

static void I386OP(jle_rel8)(void)		// Opcode 0x7e
{
	INT8 disp = FETCH();
	if( I.ZF != 0 || (I.SF != I.OF) ) {
		NEAR_BRANCH(disp);
		CYCLES(CYCLES_JCC_DISP8);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_DISP8_NOBRANCH);
	}
}

static void I386OP(jnc_rel8)(void)			// Opcode 0x73
{
	INT8 disp = FETCH();
	if( I.CF == 0 ) {
		NEAR_BRANCH(disp);
		CYCLES(CYCLES_JCC_DISP8);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_DISP8_NOBRANCH);
	}
}

static void I386OP(jno_rel8)(void)			// Opcode 0x71
{
	INT8 disp = FETCH();
	if( I.OF == 0 ) {
		NEAR_BRANCH(disp);
		CYCLES(CYCLES_JCC_DISP8);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_DISP8_NOBRANCH);
	}
}

static void I386OP(jnp_rel8)(void)			// Opcode 0x7b
{
	INT8 disp = FETCH();
	if( I.PF == 0 ) {
		NEAR_BRANCH(disp);
		CYCLES(CYCLES_JCC_DISP8);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_DISP8_NOBRANCH);
	}
}

static void I386OP(jns_rel8)(void)			// Opcode 0x79
{
	INT8 disp = FETCH();
	if( I.SF == 0 ) {
		NEAR_BRANCH(disp);
		CYCLES(CYCLES_JCC_DISP8);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_DISP8_NOBRANCH);
	}
}

static void I386OP(jnz_rel8)(void)			// Opcode 0x75
{
	INT8 disp = FETCH();
	if( I.ZF == 0 ) {
		NEAR_BRANCH(disp);
		CYCLES(CYCLES_JCC_DISP8);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_DISP8_NOBRANCH);
	}
}

static void I386OP(jo_rel8)(void)			// Opcode 0x70
{
	INT8 disp = FETCH();
	if( I.OF != 0 ) {
		NEAR_BRANCH(disp);
		CYCLES(CYCLES_JCC_DISP8);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_DISP8_NOBRANCH);
	}
}

static void I386OP(jp_rel8)(void)			// Opcode 0x7a
{
	INT8 disp = FETCH();
	if( I.PF != 0 ) {
		NEAR_BRANCH(disp);
		CYCLES(CYCLES_JCC_DISP8);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_DISP8_NOBRANCH);
	}
}

static void I386OP(js_rel8)(void)			// Opcode 0x78
{
	INT8 disp = FETCH();
	if( I.SF != 0 ) {
		NEAR_BRANCH(disp);
		CYCLES(CYCLES_JCC_DISP8);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_DISP8_NOBRANCH);
	}
}

static void I386OP(jz_rel8)(void)			// Opcode 0x74
{
	INT8 disp = FETCH();
	if( I.ZF != 0 ) {
		NEAR_BRANCH(disp);
		CYCLES(CYCLES_JCC_DISP8);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_DISP8_NOBRANCH);
	}
}

static void I386OP(jmp_rel8)(void)			// Opcode 0xeb
{
	INT8 disp = FETCH();
	NEAR_BRANCH(disp);
	CYCLES(CYCLES_JMP_SHORT);		/* TODO: Timing = 7 + m */
}

static void I386OP(lahf)(void)				// Opcode 0x9f
{
	REG8(AH) = get_flags() & 0xd7;
	CYCLES(CYCLES_LAHF);
}

static void I386OP(lodsb)(void)				// Opcode 0xac
{
	UINT32 eas;
	if( I.segment_prefix ) {
		eas = i386_translate( I.segment_override, I.address_size ? REG32(ESI) : REG16(SI) );
	} else {
		eas = i386_translate( DS, I.address_size ? REG32(ESI) : REG16(SI) );
	}
	REG8(AL) = READ8(eas);
	BUMP_SI(1);
	CYCLES(CYCLES_LODS);
}

static void I386OP(mov_rm8_r8)(void)		// Opcode 0x88
{
	UINT8 src;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_REG8(modrm);
		STORE_RM8(modrm, src);
		CYCLES(CYCLES_MOV_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		src = LOAD_REG8(modrm);
		WRITE8(ea, src);
		CYCLES(CYCLES_MOV_REG_MEM);
	}
}

static void I386OP(mov_r8_rm8)(void)		// Opcode 0x8a
{
	UINT8 src;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_RM8(modrm);
		STORE_REG8(modrm, src);
		CYCLES(CYCLES_MOV_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		src = READ8(ea);
		STORE_REG8(modrm, src);
		CYCLES(CYCLES_MOV_MEM_REG);
	}
}

static void I386OP(mov_rm8_i8)(void)		// Opcode 0xc6
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		UINT8 value = FETCH();
		STORE_RM8(modrm, value);
		CYCLES(CYCLES_MOV_IMM_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		UINT8 value = FETCH();
		WRITE8(ea, value);
		CYCLES(CYCLES_MOV_IMM_MEM);
	}
}

static void I386OP(mov_r32_cr)(void)		// Opcode 0x0f 20
{
	UINT8 modrm = FETCH();
	UINT8 cr = (modrm >> 3) & 0x7;

	STORE_RM32(modrm, I.cr[cr]);
	CYCLES(CYCLES_MOV_CR_REG);
}

static void I386OP(mov_r32_dr)(void)		// Opcode 0x0f 21
{
	UINT8 modrm = FETCH();
	UINT8 dr = (modrm >> 3) & 0x7;

	STORE_RM32(modrm, I.dr[dr]);
	switch(dr)
	{
		case 0:
		case 1:
		case 2:
		case 3:
			CYCLES(CYCLES_MOV_REG_DR0_3);
			break;
		case 6:
		case 7:
			CYCLES(CYCLES_MOV_REG_DR6_7);
			break;
	}
}

static void I386OP(mov_cr_r32)(void)		// Opcode 0x0f 22
{
	UINT8 modrm = FETCH();
	UINT8 cr = (modrm >> 3) & 0x7;

	I.cr[cr] = LOAD_RM32(modrm);
	switch(cr)
	{
		case 0: CYCLES(CYCLES_MOV_REG_CR0); break;
		case 2: CYCLES(CYCLES_MOV_REG_CR2); break;
		case 3: CYCLES(CYCLES_MOV_REG_CR3); break;
		default:
			fatalerror("i386: mov_cr_r32 CR%d !", cr);
			break;
	}
}

static void I386OP(mov_dr_r32)(void)		// Opcode 0x0f 23
{
	UINT8 modrm = FETCH();
	UINT8 dr = (modrm >> 3) & 0x7;

	I.dr[dr] = LOAD_RM32(modrm);
	switch(dr)
	{
		case 0:
		case 1:
		case 2:
		case 3:
			CYCLES(CYCLES_MOV_DR0_3_REG);
			break;
		case 6:
		case 7:
			CYCLES(CYCLES_MOV_DR6_7_REG);
			break;
		default:
			fatalerror("i386: mov_dr_r32 DR%d !", dr);
			break;
	}
}

static void I386OP(mov_al_m8)(void)			// Opcode 0xa0
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
	REG8(AL) = READ8(ea);
	CYCLES(CYCLES_MOV_IMM_MEM);
}

static void I386OP(mov_m8_al)(void)			// Opcode 0xa2
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
	WRITE8( ea, REG8(AL) );
	CYCLES(CYCLES_MOV_MEM_ACC);
}

static void I386OP(mov_rm16_sreg)(void)		// Opcode 0x8c
{
	UINT8 modrm = FETCH();
	int s = (modrm >> 3) & 0x7;

	if( modrm >= 0xc0 ) {
		STORE_RM16(modrm, I.sreg[s].selector);
		CYCLES(CYCLES_MOV_SREG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		WRITE16(ea, I.sreg[s].selector);
		CYCLES(CYCLES_MOV_SREG_MEM);
	}
}

static void I386OP(mov_sreg_rm16)(void)		// Opcode 0x8e
{
	UINT16 selector;
	UINT8 modrm = FETCH();
	int s = (modrm >> 3) & 0x7;

	if( modrm >= 0xc0 ) {
		selector = LOAD_RM16(modrm);
		CYCLES(CYCLES_MOV_REG_SREG);
	} else {
		UINT32 ea = GetEA(modrm);
		selector = READ16(ea);
		CYCLES(CYCLES_MOV_MEM_SREG);
	}
	I.sreg[s].selector = selector;
	i386_load_segment_descriptor( s );
}

static void I386OP(mov_al_i8)(void)			// Opcode 0xb0
{
	REG8(AL) = FETCH();
	CYCLES(CYCLES_MOV_IMM_REG);
}

static void I386OP(mov_cl_i8)(void)			// Opcode 0xb1
{
	REG8(CL) = FETCH();
	CYCLES(CYCLES_MOV_IMM_REG);
}

static void I386OP(mov_dl_i8)(void)			// Opcode 0xb2
{
	REG8(DL) = FETCH();
	CYCLES(CYCLES_MOV_IMM_REG);
}

static void I386OP(mov_bl_i8)(void)			// Opcode 0xb3
{
	REG8(BL) = FETCH();
	CYCLES(CYCLES_MOV_IMM_REG);
}

static void I386OP(mov_ah_i8)(void)			// Opcode 0xb4
{
	REG8(AH) = FETCH();
	CYCLES(CYCLES_MOV_IMM_REG);
}

static void I386OP(mov_ch_i8)(void)			// Opcode 0xb5
{
	REG8(CH) = FETCH();
	CYCLES(CYCLES_MOV_IMM_REG);
}

static void I386OP(mov_dh_i8)(void)			// Opcode 0xb6
{
	REG8(DH) = FETCH();
	CYCLES(CYCLES_MOV_IMM_REG);
}

static void I386OP(mov_bh_i8)(void)			// Opcode 0xb7
{
	REG8(BH) = FETCH();
	CYCLES(CYCLES_MOV_IMM_REG);
}

static void I386OP(movsb)(void)				// Opcode 0xa4
{
	UINT32 eas, ead;
	UINT8 v;
	if( I.segment_prefix ) {
		eas = i386_translate( I.segment_override, I.address_size ? REG32(ESI) : REG16(SI) );
	} else {
		eas = i386_translate( DS, I.address_size ? REG32(ESI) : REG16(SI) );
	}
	ead = i386_translate( ES, I.address_size ? REG32(EDI) : REG16(DI) );
	v = READ8(eas);
	WRITE8(ead, v);
	BUMP_SI(1);
	BUMP_DI(1);
	CYCLES(CYCLES_MOVS);
}

static void I386OP(or_rm8_r8)(void)			// Opcode 0x08
{
	UINT8 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_REG8(modrm);
		dst = LOAD_RM8(modrm);
		dst = OR8(dst, src);
		STORE_RM8(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		src = LOAD_REG8(modrm);
		dst = READ8(ea);
		dst = OR8(dst, src);
		WRITE8(ea, dst);
		CYCLES(CYCLES_ALU_REG_MEM);
	}
}

static void I386OP(or_r8_rm8)(void)			// Opcode 0x0a
{
	UINT8 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_RM8(modrm);
		dst = LOAD_REG8(modrm);
		dst = OR8(dst, src);
		STORE_REG8(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		src = READ8(ea);
		dst = LOAD_REG8(modrm);
		dst = OR8(dst, src);
		STORE_REG8(modrm, dst);
		CYCLES(CYCLES_ALU_MEM_REG);
	}
}

static void I386OP(or_al_i8)(void)			// Opcode 0x0c
{
	UINT8 src, dst;
	src = FETCH();
	dst = REG8(AL);
	dst = OR8(dst, src);
	REG8(EAX) = dst;
	CYCLES(CYCLES_ALU_IMM_ACC);
}

static void I386OP(out_al_i8)(void)			// Opcode 0xe6
{
	UINT16 port = FETCH();
	UINT8 data = REG8(AL);
	WRITEPORT8(port, data);
	CYCLES(CYCLES_OUT_VAR);
}

static void I386OP(out_al_dx)(void)			// Opcode 0xee
{
	UINT16 port = REG16(DX);
	UINT8 data = REG8(AL);
	WRITEPORT8(port, data);
	CYCLES(CYCLES_OUT);
}

static void I386OP(push_i8)(void)			// Opcode 0x6a
{
	UINT8 value = FETCH();
	PUSH8(value);
	CYCLES(CYCLES_PUSH_IMM);
}

static void I386OP(ins_generic)(int size)
{
	UINT32 ead;
	UINT8 vb;
	UINT16 vw;
	UINT32 vd;

	ead = i386_translate( ES, I.address_size ? REG32(EDI) : REG16(DI) );

	switch(size) {
	case 1:
		vb = READPORT8(REG16(DX));
		WRITE8(ead, vb);
		break;
	case 2:
		vw = READPORT16(REG16(DX));
		WRITE16(ead, vw);
		break;
	case 4:
		vd = READPORT32(REG16(DX));
		WRITE32(ead, vd);
		break;
	}

	REG32(EDI) += ((I.DF) ? -1 : 1) * size;
	CYCLES(CYCLES_INS);	// TODO: Confirm this value
}

static void I386OP(insb)(void)				// Opcode 0x6c
{
	I386OP(ins_generic)(1);
}

static void I386OP(insw)(void)				// Opcode 0x6d
{
	I386OP(ins_generic)(2);
}

static void I386OP(insd)(void)				// Opcode 0x6d
{
	I386OP(ins_generic)(4);
}

static void I386OP(outs_generic)(int size)
{
	UINT32 eas;
	UINT8 vb;
	UINT16 vw;
	UINT32 vd;

	if( I.segment_prefix ) {
		eas = i386_translate( I.segment_override, REG32(ESI) );
	} else {
		eas = i386_translate( DS, REG32(ESI) );
	}

	switch(size) {
	case 1:
		vb = READ8(eas);
		WRITEPORT8(REG16(DX), vb);
		break;
	case 2:
		vw = READ16(eas);
		WRITEPORT16(REG16(DX), vw);
		break;
	case 4:
		vd = READ32(eas);
		WRITEPORT32(REG16(DX), vd);
		break;
	}

	REG32(ESI) += ((I.DF) ? -1 : 1) * size;
	CYCLES(CYCLES_OUTS);	// TODO: Confirm this value
}

static void I386OP(outsb)(void)				// Opcode 0x6e
{
	I386OP(outs_generic)(1);
}

static void I386OP(outsw)(void)				// Opcode 0x6f
{
	I386OP(outs_generic)(2);
}

static void I386OP(outsd)(void)				// Opcode 0x6f
{
	I386OP(outs_generic)(4);
}

static void I386OP(repeat)(int invert_flag)
{
	UINT32 repeated_eip = I.eip;
	UINT32 repeated_pc = I.pc;
	UINT8 opcode = FETCH();
	UINT32 eas, ead;
	UINT32 count;
	INT32 cycle_base = 0, cycle_adjustment = 0;
	UINT8 *flag = NULL;

	if( I.segment_prefix ) {
		eas = i386_translate( I.segment_override, I.sreg[CS].d ? REG32(ESI) : REG16(SI) );
	} else {
		eas = i386_translate( DS, I.sreg[CS].d ? REG32(ESI) : REG16(SI) );
	}
	ead = i386_translate( ES, I.sreg[CS].d ? REG32(EDI) : REG16(DI) );

	if( opcode == 0x66 ) {
		I.operand_size ^= 1;
		repeated_eip = I.eip;
		repeated_pc = I.pc;
		opcode = FETCH();
	}
	if( opcode == 0x67 ) {
		I.address_size ^= 1;
		repeated_eip = I.eip;
		repeated_pc = I.pc;
		opcode = FETCH();
	}

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
			flag = &I.ZF;
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
			flag = &I.ZF;
			break;

		default:
			fatalerror("i386: Invalid REP/opcode %02X combination",opcode);
			break;
	}

	if( I.sreg[CS].d ) {
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
		I.eip = repeated_eip;
		I.pc = repeated_pc;
		I386OP(decode_opcode)();
		CYCLES_NUM(cycle_adjustment);

		if (I.sreg[CS].d)
			count = --REG32(ECX);
		else
			count = --REG16(CX);
		if (I.cycles <= 0)
			goto outofcycles;
	}
	while( count && (!flag || (invert_flag ? !*flag : *flag)) );
	return;

outofcycles:
	/* if we run out of cycles to execute, and we are still in the repeat, we need
     * to exit this instruction in such a way to go right back into it when we have
     * time to execute cycles */
	I.eip = I.prev_eip;
	CHANGE_PC(I.eip);
	CYCLES_NUM(-cycle_base);
}

static void I386OP(rep)(void)				// Opcode 0xf3
{
	I386OP(repeat)(0);
	}

static void I386OP(repne)(void)				// Opcode 0xf2
	{
	I386OP(repeat)(1);
}

static void I386OP(sahf)(void)				// Opcode 0x9e
{
	set_flags( (get_flags() & 0xffffff00) | (REG8(AH) & 0xd7) );
	CYCLES(CYCLES_SAHF);
}

static void I386OP(sbb_rm8_r8)(void)		// Opcode 0x18
{
	UINT8 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_REG8(modrm) + I.CF;
		dst = LOAD_RM8(modrm);
		dst = SUB8(dst, src);
		STORE_RM8(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		src = LOAD_REG8(modrm) + I.CF;
		dst = READ8(ea);
		dst = SUB8(dst, src);
		WRITE8(ea, dst);
		CYCLES(CYCLES_ALU_REG_MEM);
	}
}

static void I386OP(sbb_r8_rm8)(void)		// Opcode 0x1a
{
	UINT8 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_RM8(modrm) + I.CF;
		dst = LOAD_REG8(modrm);
		dst = SUB8(dst, src);
		STORE_REG8(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		src = READ8(ea) + I.CF;
		dst = LOAD_REG8(modrm);
		dst = SUB8(dst, src);
		STORE_REG8(modrm, dst);
		CYCLES(CYCLES_ALU_MEM_REG);
	}
}

static void I386OP(sbb_al_i8)(void)			// Opcode 0x1c
{
	UINT8 src, dst;
	src = FETCH() + I.CF;
	dst = REG8(AL);
	dst = SUB8(dst, src);
	REG8(EAX) = dst;
	CYCLES(CYCLES_ALU_IMM_ACC);
}

static void I386OP(scasb)(void)				// Opcode 0xae
{
	UINT32 eas;
	UINT8 src, dst;
	eas = i386_translate( ES, I.address_size ? REG32(EDI) : REG16(DI) );
	src = READ8(eas);
	dst = REG8(AL);
	SUB8(dst, src);
	BUMP_DI(1);
	CYCLES(CYCLES_SCAS);
}

static void I386OP(setalc)(void)			// Opcode 0xd6 (undocumented)
{
	if( I.CF ) {
		REG8(AL) = 0xff;
	} else {
		REG8(AL) = 0;
	}
	CYCLES(3);
}

static void I386OP(seta_rm8)(void)			// Opcode 0x0f 97
{
	UINT8 modrm = FETCH();
	UINT8 value = 0;
	if( I.CF == 0 && I.ZF == 0 ) {
		value = 1;
	}
	if( modrm >= 0xc0 ) {
		STORE_RM8(modrm, value);
		CYCLES(CYCLES_SETCC_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		WRITE8(ea, value);
		CYCLES(CYCLES_SETCC_MEM);
	}
}

static void I386OP(setbe_rm8)(void)			// Opcode 0x0f 96
{
	UINT8 modrm = FETCH();
	UINT8 value = 0;
	if( I.CF != 0 || I.ZF != 0 ) {
		value = 1;
	}
	if( modrm >= 0xc0 ) {
		STORE_RM8(modrm, value);
		CYCLES(CYCLES_SETCC_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		WRITE8(ea, value);
		CYCLES(CYCLES_SETCC_MEM);
	}
}

static void I386OP(setc_rm8)(void)			// Opcode 0x0f 92
{
	UINT8 modrm = FETCH();
	UINT8 value = 0;
	if( I.CF != 0 ) {
		value = 1;
	}
	if( modrm >= 0xc0 ) {
		STORE_RM8(modrm, value);
		CYCLES(CYCLES_SETCC_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		WRITE8(ea, value);
		CYCLES(CYCLES_SETCC_MEM);
	}
}

static void I386OP(setg_rm8)(void)			// Opcode 0x0f 9f
{
	UINT8 modrm = FETCH();
	UINT8 value = 0;
	if( I.ZF == 0 && (I.SF == I.OF) ) {
		value = 1;
	}
	if( modrm >= 0xc0 ) {
		STORE_RM8(modrm, value);
		CYCLES(CYCLES_SETCC_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		WRITE8(ea, value);
		CYCLES(CYCLES_SETCC_MEM);
	}
}

static void I386OP(setge_rm8)(void)			// Opcode 0x0f 9d
{
	UINT8 modrm = FETCH();
	UINT8 value = 0;
	if( (I.SF == I.OF) ) {
		value = 1;
	}
	if( modrm >= 0xc0 ) {
		STORE_RM8(modrm, value);
		CYCLES(CYCLES_SETCC_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		WRITE8(ea, value);
		CYCLES(CYCLES_SETCC_MEM);
	}
}

static void I386OP(setl_rm8)(void)			// Opcode 0x0f 9c
{
	UINT8 modrm = FETCH();
	UINT8 value = 0;
	if( I.SF != I.OF ) {
		value = 1;
	}
	if( modrm >= 0xc0 ) {
		STORE_RM8(modrm, value);
		CYCLES(CYCLES_SETCC_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		WRITE8(ea, value);
		CYCLES(CYCLES_SETCC_MEM);
	}
}

static void I386OP(setle_rm8)(void)			// Opcode 0x0f 9e
{
	UINT8 modrm = FETCH();
	UINT8 value = 0;
	if( I.ZF != 0 || (I.SF != I.OF) ) {
		value = 1;
	}
	if( modrm >= 0xc0 ) {
		STORE_RM8(modrm, value);
		CYCLES(CYCLES_SETCC_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		WRITE8(ea, value);
		CYCLES(CYCLES_SETCC_MEM);
	}
}

static void I386OP(setnc_rm8)(void)			// Opcode 0x0f 93
{
	UINT8 modrm = FETCH();
	UINT8 value = 0;
	if( I.CF == 0 ) {
		value = 1;
	}
	if( modrm >= 0xc0 ) {
		STORE_RM8(modrm, value);
		CYCLES(CYCLES_SETCC_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		WRITE8(ea, value);
		CYCLES(CYCLES_SETCC_MEM);
	}
}

static void I386OP(setno_rm8)(void)			// Opcode 0x0f 91
{
	UINT8 modrm = FETCH();
	UINT8 value = 0;
	if( I.OF == 0 ) {
		value = 1;
	}
	if( modrm >= 0xc0 ) {
		STORE_RM8(modrm, value);
		CYCLES(CYCLES_SETCC_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		WRITE8(ea, value);
		CYCLES(CYCLES_SETCC_MEM);
	}
}

static void I386OP(setnp_rm8)(void)			// Opcode 0x0f 9b
{
	UINT8 modrm = FETCH();
	UINT8 value = 0;
	if( I.PF == 0 ) {
		value = 1;
	}
	if( modrm >= 0xc0 ) {
		STORE_RM8(modrm, value);
		CYCLES(CYCLES_SETCC_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		WRITE8(ea, value);
		CYCLES(CYCLES_SETCC_MEM);
	}
}

static void I386OP(setns_rm8)(void)			// Opcode 0x0f 99
{
	UINT8 modrm = FETCH();
	UINT8 value = 0;
	if( I.SF == 0 ) {
		value = 1;
	}
	if( modrm >= 0xc0 ) {
		STORE_RM8(modrm, value);
		CYCLES(CYCLES_SETCC_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		WRITE8(ea, value);
		CYCLES(CYCLES_SETCC_MEM);
	}
}

static void I386OP(setnz_rm8)(void)			// Opcode 0x0f 95
{
	UINT8 modrm = FETCH();
	UINT8 value = 0;
	if( I.ZF == 0 ) {
		value = 1;
	}
	if( modrm >= 0xc0 ) {
		STORE_RM8(modrm, value);
		CYCLES(CYCLES_SETCC_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		WRITE8(ea, value);
		CYCLES(CYCLES_SETCC_MEM);
	}
}

static void I386OP(seto_rm8)(void)			// Opcode 0x0f 90
{
	UINT8 modrm = FETCH();
	UINT8 value = 0;
	if( I.OF != 0 ) {
		value = 1;
	}
	if( modrm >= 0xc0 ) {
		STORE_RM8(modrm, value);
		CYCLES(CYCLES_SETCC_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		WRITE8(ea, value);
		CYCLES(CYCLES_SETCC_MEM);
	}
}

static void I386OP(setp_rm8)(void)			// Opcode 0x0f 9a
{
	UINT8 modrm = FETCH();
	UINT8 value = 0;
	if( I.PF != 0 ) {
		value = 1;
	}
	if( modrm >= 0xc0 ) {
		STORE_RM8(modrm, value);
		CYCLES(CYCLES_SETCC_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		WRITE8(ea, value);
		CYCLES(CYCLES_SETCC_MEM);
	}
}

static void I386OP(sets_rm8)(void)			// Opcode 0x0f 98
{
	UINT8 modrm = FETCH();
	UINT8 value = 0;
	if( I.SF != 0 ) {
		value = 1;
	}
	if( modrm >= 0xc0 ) {
		STORE_RM8(modrm, value);
		CYCLES(CYCLES_SETCC_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		WRITE8(ea, value);
		CYCLES(CYCLES_SETCC_MEM);
	}
}

static void I386OP(setz_rm8)(void)			// Opcode 0x0f 94
{
	UINT8 modrm = FETCH();
	UINT8 value = 0;
	if( I.ZF != 0 ) {
		value = 1;
	}
	if( modrm >= 0xc0 ) {
		STORE_RM8(modrm, value);
		CYCLES(CYCLES_SETCC_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		WRITE8(ea, value);
		CYCLES(CYCLES_SETCC_MEM);
	}
}

static void I386OP(stc)(void)				// Opcode 0xf9
{
	I.CF = 1;
	CYCLES(CYCLES_STC);
}

static void I386OP(std)(void)				// Opcode 0xfd
{
	I.DF = 1;
	CYCLES(CYCLES_STD);
}

static void I386OP(sti)(void)				// Opcode 0xfb
{
	I.IF = 1;
	CYCLES(CYCLES_STI);
}

static void I386OP(stosb)(void)				// Opcode 0xaa
{
	UINT32 ead;
	ead = i386_translate( ES, I.address_size ? REG32(EDI) : REG16(DI) );
	WRITE8(ead, REG8(AL));
	BUMP_DI(1);
	CYCLES(CYCLES_STOS);
}

static void I386OP(sub_rm8_r8)(void)		// Opcode 0x28
{
	UINT8 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_REG8(modrm);
		dst = LOAD_RM8(modrm);
		dst = SUB8(dst, src);
		STORE_RM8(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		src = LOAD_REG8(modrm);
		dst = READ8(ea);
		dst = SUB8(dst, src);
		WRITE8(ea, dst);
		CYCLES(CYCLES_ALU_REG_MEM);
	}
}

static void I386OP(sub_r8_rm8)(void)		// Opcode 0x2a
{
	UINT8 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_RM8(modrm);
		dst = LOAD_REG8(modrm);
		dst = SUB8(dst, src);
		STORE_REG8(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		src = READ8(ea);
		dst = LOAD_REG8(modrm);
		dst = SUB8(dst, src);
		STORE_REG8(modrm, dst);
		CYCLES(CYCLES_ALU_MEM_REG);
	}
}

static void I386OP(sub_al_i8)(void)			// Opcode 0x2c
{
	UINT8 src, dst;
	src = FETCH();
	dst = REG8(EAX);
	dst = SUB8(dst, src);
	REG8(EAX) = dst;
	CYCLES(CYCLES_ALU_IMM_ACC);
}

static void I386OP(test_al_i8)(void)		// Opcode 0xa8
{
	UINT8 src = FETCH();
	UINT8 dst = REG8(AL);
	dst = src & dst;
	SetSZPF8(dst);
	I.CF = 0;
	I.OF = 0;
	CYCLES(CYCLES_ALU_IMM_ACC);
}

static void I386OP(test_rm8_r8)(void)		// Opcode 0x84
{
	UINT8 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_REG8(modrm);
		dst = LOAD_RM8(modrm);
		dst = src & dst;
		SetSZPF8(dst);
		I.CF = 0;
		I.OF = 0;
		CYCLES(CYCLES_TEST_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		src = LOAD_REG8(modrm);
		dst = READ8(ea);
		dst = src & dst;
		SetSZPF8(dst);
		I.CF = 0;
		I.OF = 0;
		CYCLES(CYCLES_TEST_REG_MEM);
	}
}

static void I386OP(xchg_r8_rm8)(void)		// Opcode 0x86
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		UINT8 src = LOAD_RM8(modrm);
		UINT8 dst = LOAD_REG8(modrm);
		STORE_REG8(modrm, src);
		STORE_RM8(modrm, dst);
		CYCLES(CYCLES_XCHG_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		UINT8 src = READ8(ea);
		UINT8 dst = LOAD_REG8(modrm);
		STORE_REG8(modrm, src);
		WRITE8(ea, dst);
		CYCLES(CYCLES_XCHG_REG_MEM);
	}
}

static void I386OP(xor_rm8_r8)(void)		// Opcode 0x30
{
	UINT8 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_REG8(modrm);
		dst = LOAD_RM8(modrm);
		dst = XOR8(dst, src);
		STORE_RM8(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		src = LOAD_REG8(modrm);
		dst = READ8(ea);
		dst = XOR8(dst, src);
		WRITE8(ea, dst);
		CYCLES(CYCLES_ALU_REG_MEM);
	}
}

static void I386OP(xor_r8_rm8)(void)		// Opcode 0x32
{
	UINT32 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_RM8(modrm);
		dst = LOAD_REG8(modrm);
		dst = XOR8(dst, src);
		STORE_REG8(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		src = READ8(ea);
		dst = LOAD_REG8(modrm);
		dst = XOR8(dst, src);
		STORE_REG8(modrm, dst);
		CYCLES(CYCLES_ALU_MEM_REG);
	}
}

static void I386OP(xor_al_i8)(void)			// Opcode 0x34
{
	UINT8 src, dst;
	src = FETCH();
	dst = REG8(AL);
	dst = XOR8(dst, src);
	REG8(AL) = dst;
	CYCLES(CYCLES_ALU_IMM_ACC);
}



static void I386OP(group80_8)(void)			// Opcode 0x80
{
	UINT32 ea;
	UINT8 src, dst;
	UINT8 modrm = FETCH();

	switch( (modrm >> 3) & 0x7 )
	{
		case 0:		// ADD Rm8, i8
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM8(modrm);
				src = FETCH();
				dst = ADD8(dst, src);
				STORE_RM8(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm);
				dst = READ8(ea);
				src = FETCH();
				dst = ADD8(dst, src);
				WRITE8(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 1:		// OR Rm8, i8
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM8(modrm);
				src = FETCH();
				dst = OR8(dst, src);
				STORE_RM8(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm);
				dst = READ8(ea);
				src = FETCH();
				dst = OR8(dst, src);
				WRITE8(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 2:		// ADC Rm8, i8
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM8(modrm);
				src = FETCH();
				src = ADD8(src, I.CF);
				dst = ADD8(dst, src);
				STORE_RM8(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm);
				dst = READ8(ea);
				src = FETCH();
				src = ADD8(src, I.CF);
				dst = ADD8(dst, src);
				WRITE8(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 3:		// SBB Rm8, i8
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM8(modrm);
				src = FETCH() + I.CF;
				dst = SUB8(dst, src);
				STORE_RM8(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm);
				dst = READ8(ea);
				src = FETCH() + I.CF;
				dst = SUB8(dst, src);
				WRITE8(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 4:		// AND Rm8, i8
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM8(modrm);
				src = FETCH();
				dst = AND8(dst, src);
				STORE_RM8(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm);
				dst = READ8(ea);
				src = FETCH();
				dst = AND8(dst, src);
				WRITE8(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 5:		// SUB Rm8, i8
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM8(modrm);
				src = FETCH();
				dst = SUB8(dst, src);
				STORE_RM8(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm);
				dst = READ8(ea);
				src = FETCH();
				dst = SUB8(dst, src);
				WRITE8(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 6:		// XOR Rm8, i8
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM8(modrm);
				src = FETCH();
				dst = XOR8(dst, src);
				STORE_RM8(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm);
				dst = READ8(ea);
				src = FETCH();
				dst = XOR8(dst, src);
				WRITE8(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 7:		// CMP Rm8, i8
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM8(modrm);
				src = FETCH();
				SUB8(dst, src);
				CYCLES(CYCLES_CMP_REG_REG);
			} else {
				ea = GetEA(modrm);
				dst = READ8(ea);
				src = FETCH();
				SUB8(dst, src);
				CYCLES(CYCLES_CMP_REG_MEM);
			}
			break;
	}
}

static void I386OP(groupC0_8)(void)			// Opcode 0xc0
{
	UINT8 dst;
	UINT8 modrm = FETCH();
	UINT8 shift;

	if( modrm >= 0xc0 ) {
		dst = LOAD_RM8(modrm);
		shift = FETCH() & 0x1f;
		dst = i386_shift_rotate8(modrm, dst, shift);
		STORE_RM8(modrm, dst);
	} else {
		UINT32 ea = GetEA(modrm);
		dst = READ8(ea);
		shift = FETCH() & 0x1f;
		dst = i386_shift_rotate8(modrm, dst, shift);
		WRITE8(ea, dst);
	}
}

static void I386OP(groupD0_8)(void)			// Opcode 0xd0
{
	UINT8 dst;
	UINT8 modrm = FETCH();

	if( modrm >= 0xc0 ) {
		dst = LOAD_RM8(modrm);
		dst = i386_shift_rotate8(modrm, dst, 1);
		STORE_RM8(modrm, dst);
	} else {
		UINT32 ea = GetEA(modrm);
		dst = READ8(ea);
		dst = i386_shift_rotate8(modrm, dst, 1);
		WRITE8(ea, dst);
	}
}

static void I386OP(groupD2_8)(void)			// Opcode 0xd2
{
	UINT8 dst;
	UINT8 modrm = FETCH();

	if( modrm >= 0xc0 ) {
		dst = LOAD_RM8(modrm);
		dst = i386_shift_rotate8(modrm, dst, REG8(CL));
		STORE_RM8(modrm, dst);
	} else {
		UINT32 ea = GetEA(modrm);
		dst = READ8(ea);
		dst = i386_shift_rotate8(modrm, dst, REG8(CL));
		WRITE8(ea, dst);
	}
}

static void I386OP(groupF6_8)(void)			// Opcode 0xf6
{
	UINT8 modrm = FETCH();

	switch( (modrm >> 3) & 0x7 )
	{
		case 0:			/* TEST Rm8, i8 */
			if( modrm >= 0xc0 ) {
				UINT8 dst = LOAD_RM8(modrm);
				UINT8 src = FETCH();
				dst &= src;
				I.CF = I.OF = I.AF = 0;
				SetSZPF8(dst);
				CYCLES(CYCLES_TEST_IMM_REG);
			} else {
				UINT32 ea = GetEA(modrm);
				UINT8 dst = READ8(ea);
				UINT8 src = FETCH();
				dst &= src;
				I.CF = I.OF = I.AF = 0;
				SetSZPF8(dst);
				CYCLES(CYCLES_TEST_IMM_MEM);
			}
			break;
		case 2:			/* NOT Rm8 */
			if( modrm >= 0xc0 ) {
				UINT8 dst = LOAD_RM8(modrm);
				dst = ~dst;
				STORE_RM8(modrm, dst);
				CYCLES(CYCLES_NOT_REG);
			} else {
				UINT32 ea = GetEA(modrm);
				UINT8 dst = READ8(ea);
				dst = ~dst;
				WRITE8(ea, dst);
				CYCLES(CYCLES_NOT_MEM);
			}
			break;
		case 3:			/* NEG Rm8 */
			if( modrm >= 0xc0 ) {
				UINT8 dst = LOAD_RM8(modrm);
				dst = SUB8( 0, dst );
				STORE_RM8(modrm, dst);
				CYCLES(CYCLES_NEG_REG);
			} else {
				UINT32 ea = GetEA(modrm);
				UINT8 dst = READ8(ea);
				dst = SUB8( 0, dst );
				WRITE8(ea, dst);
				CYCLES(CYCLES_NEG_MEM);
			}
			break;
		case 4:			/* MUL AL, Rm8 */
			{
				UINT16 result;
				UINT8 src, dst;
				if( modrm >= 0xc0 ) {
					src = LOAD_RM8(modrm);
					CYCLES(CYCLES_MUL8_ACC_REG);		/* TODO: Correct multiply timing */
				} else {
					UINT32 ea = GetEA(modrm);
					src = READ8(ea);
					CYCLES(CYCLES_MUL8_ACC_MEM);		/* TODO: Correct multiply timing */
				}

				dst = REG8(AL);
				result = (UINT16)src * (UINT16)dst;
				REG16(AX) = (UINT16)result;

				I.CF = I.OF = (REG16(AX) > 0xff);
			}
			break;
		case 5:			/* IMUL AL, Rm8 */
			{
				INT16 result;
				INT16 src, dst;
				if( modrm >= 0xc0 ) {
					src = (INT16)(INT8)LOAD_RM8(modrm);
					CYCLES(CYCLES_IMUL8_ACC_REG);		/* TODO: Correct multiply timing */
				} else {
					UINT32 ea = GetEA(modrm);
					src = (INT16)(INT8)READ8(ea);
					CYCLES(CYCLES_IMUL8_ACC_MEM);		/* TODO: Correct multiply timing */
				}

				dst = (INT16)(INT8)REG8(AL);
				result = src * dst;

				REG16(AX) = (UINT16)result;

				I.CF = I.OF = !(result == (INT16)(INT8)result);
			}
			break;
		case 6:			/* DIV AL, Rm8 */
			{
				UINT16 quotient, remainder, result;
				UINT8 src;
				if( modrm >= 0xc0 ) {
					src = LOAD_RM8(modrm);
					CYCLES(CYCLES_DIV8_ACC_REG);
				} else {
					UINT32 ea = GetEA(modrm);
					src = READ8(ea);
					CYCLES(CYCLES_DIV8_ACC_MEM);
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
					}
				} else {
					/* TODO: Divide by zero */
				}
			}
			break;
		case 7:			/* IDIV AL, Rm8 */
			{
				INT16 quotient, remainder, result;
				UINT8 src;
				if( modrm >= 0xc0 ) {
					src = LOAD_RM8(modrm);
					CYCLES(CYCLES_IDIV8_ACC_REG);
				} else {
					UINT32 ea = GetEA(modrm);
					src = READ8(ea);
					CYCLES(CYCLES_IDIV8_ACC_MEM);
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
					}
				} else {
					/* TODO: Divide by zero */
				}
			}
			break;
	}
}

static void I386OP(groupFE_8)(void)			// Opcode 0xfe
{
	UINT8 modrm = FETCH();

	switch( (modrm >> 3) & 0x7 )
	{
		case 0:			/* INC Rm8 */
			if( modrm >= 0xc0 ) {
				UINT8 dst = LOAD_RM8(modrm);
				dst = INC8(dst);
				STORE_RM8(modrm, dst);
				CYCLES(CYCLES_INC_REG);
			} else {
				UINT32 ea = GetEA(modrm);
				UINT8 dst = READ8(ea);
				dst = INC8(dst);
				WRITE8(ea, dst);
				CYCLES(CYCLES_INC_MEM);
			}
			break;
		case 1:			/* DEC Rm8 */
			if( modrm >= 0xc0 ) {
				UINT8 dst = LOAD_RM8(modrm);
				dst = DEC8(dst);
				STORE_RM8(modrm, dst);
				CYCLES(CYCLES_DEC_REG);
			} else {
				UINT32 ea = GetEA(modrm);
				UINT8 dst = READ8(ea);
				dst = DEC8(dst);
				WRITE8(ea, dst);
				CYCLES(CYCLES_DEC_MEM);
			}
			break;
		case 6:			/* PUSH Rm8 */
			{
				UINT8 value;
				if( modrm >= 0xc0 ) {
					value = LOAD_RM8(modrm);
				} else {
					UINT32 ea = GetEA(modrm);
					value = READ8(ea);
				}
				if( I.operand_size ) {
					PUSH32(value);
				} else {
					PUSH16(value);
				}
				CYCLES(CYCLES_PUSH_RM);
			}
			break;
		default:
			fatalerror("i386: groupFE_8 /%d unimplemented", (modrm >> 3) & 0x7);
			break;
	}
}



static void I386OP(segment_CS)(void)		// Opcode 0x2e
{
	I.segment_prefix = 1;
	I.segment_override = CS;

	I386OP(decode_opcode)();
}

static void I386OP(segment_DS)(void)		// Opcode 0x3e
{
	I.segment_prefix = 1;
	I.segment_override = DS;
	CYCLES(0);	// TODO: Specify cycle count
	I386OP(decode_opcode)();
}

static void I386OP(segment_ES)(void)		// Opcode 0x26
{
	I.segment_prefix = 1;
	I.segment_override = ES;
	CYCLES(0);	// TODO: Specify cycle count
	I386OP(decode_opcode)();
}

static void I386OP(segment_FS)(void)		// Opcode 0x64
{
	I.segment_prefix = 1;
	I.segment_override = FS;
	CYCLES(1);	// TODO: Specify cycle count
	I386OP(decode_opcode)();
}

static void I386OP(segment_GS)(void)		// Opcode 0x65
{
	I.segment_prefix = 1;
	I.segment_override = GS;
	CYCLES(1);	// TODO: Specify cycle count
	I386OP(decode_opcode)();
}

static void I386OP(segment_SS)(void)		// Opcode 0x36
{
	I.segment_prefix = 1;
	I.segment_override = SS;
	CYCLES(0);	// TODO: Specify cycle count
	I386OP(decode_opcode)();
}

static void I386OP(operand_size)(void)		// Opcode 0x66
{
	I.operand_size ^= 1;
	I386OP(decode_opcode)();
}

static void I386OP(address_size)(void)		// Opcode 0x67
{
	I.address_size ^= 1;
	I386OP(decode_opcode)();
}

static void I386OP(nop)(void)				// Opcode 0x90
{
	CYCLES(CYCLES_NOP);
}

static void I386OP(int3)(void)				// Opcode 0xcc
{
	CYCLES(CYCLES_INT3);
	i386_trap(3, 1);
}

static void I386OP(int)(void)				// Opcode 0xcd
{
	int interrupt = FETCH();
	CYCLES(CYCLES_INT);
	i386_trap(interrupt, 1);
}

static void I386OP(into)(void)				// Opcode 0xce
{
	if( I.OF ) {
		i386_trap(4, 1);
		CYCLES(CYCLES_INTO_OF1);
	}
	else
	{
		CYCLES(CYCLES_INTO_OF0);
	}
}

static void I386OP(escape)(void)			// Opcodes 0xd8 - 0xdf
{
	UINT8 modrm = FETCH();
	CYCLES(3);	// TODO: confirm this
	(void) LOAD_RM8(modrm);
}

static void I386OP(hlt)(void)				// Opcode 0xf4
{
	// TODO: We need to raise an exception in protected mode and when
	// the current privilege level is not zero
	I.halted = 1;
	I.cycles = 0;
	CYCLES(CYCLES_HLT);
}

static void I386OP(decimal_adjust)(int direction)
{
	UINT8 tmpAL = REG8(AL);

	if (I.AF || ((REG8(AL) & 0xf) > 9))
	{
		REG8(AL) = REG8(AL) + (direction * 0x06);
		I.AF = 1;
		if (REG8(AL) & 0x100)
			I.CF = 1;
		if (direction > 0)
			tmpAL = REG8(AL);
	}

	if (I.CF || (tmpAL > 0x9f))
	{
		REG8(AL) += (direction * 0x60);
		I.CF = 1;
	}

	SetSZPF8(REG8(AL));
}

static void I386OP(daa)(void)				// Opcode 0x27
{
	I386OP(decimal_adjust)(+1);
	CYCLES(CYCLES_DAA);
}

static void I386OP(das)(void)				// Opcode 0x2f
{
	I386OP(decimal_adjust)(-1);
	CYCLES(CYCLES_DAS);
}

static void I386OP(aaa)(void)				// Opcode 0x37
{
	if( (REG8(AL) & 0x0f) || I.AF != 0 ) {
		REG16(AX) = REG16(AX) + 6;
		REG8(AH) = REG8(AH) + 1;
		I.AF = 1;
		I.CF = 1;
	} else {
		I.AF = 0;
		I.CF = 0;
	}
	REG8(AL) = REG8(AL) & 0x0f;
	CYCLES(CYCLES_AAA);
}

static void I386OP(aas)(void)				// Opcode 0x3f
{
	if (I.AF || ((REG8(AL) & 0xf) > 9))
    {
		REG16(AX) -= 6;
		REG8(AH) -= 1;
		I.AF = 1;
		I.CF = 1;
    }
	else
	{
		I.AF = 0;
		I.CF = 0;
    }
	REG8(AL) &= 0x0f;
	CYCLES(CYCLES_AAS);
}

static void I386OP(aad)(void)				// Opcode 0xd5
{
	UINT8 tempAL = REG8(AL);
	UINT8 tempAH = REG8(AH);
	UINT8 i = FETCH();

	REG8(AL) = (tempAL + (tempAH * i)) & 0xff;
	REG8(AH) = 0;
	SetSZPF8( REG8(AL) );
	CYCLES(CYCLES_AAD);
}

static void I386OP(aam)(void)				// Opcode 0xd4
{
	UINT8 tempAL = REG8(AL);
	UINT8 i = FETCH();

	REG8(AH) = tempAL / i;
	REG8(AL) = tempAL % i;
	SetSZPF8( REG8(AL) );
	CYCLES(CYCLES_AAM);
}

static void I386OP(clts)(void)				// Opcode 0x0f 0x06
{
	// TODO: #GP(0) is executed
	I.cr[0] &= ~0x08;	/* clear TS bit */
	CYCLES(CYCLES_CLTS);
}

static void I386OP(wait)(void)				// Opcode 0x9B
{
	// TODO
}

static void I386OP(lock)(void)				// Opcode 0xf0
{
	CYCLES(CYCLES_LOCK);		// TODO: Determine correct cycle count
	I386OP(decode_opcode)();
}

static void I386OP(mov_r32_tr)(void)		// Opcode 0x0f 24
{
	FETCH();
	CYCLES(1);		// TODO: correct cycle count
}

static void I386OP(mov_tr_r32)(void)		// Opcode 0x0f 26
{
	FETCH();
	CYCLES(1);		// TODO: correct cycle count
}

static void I386OP(unimplemented)(void)
{
	fatalerror("i386: Unimplemented opcode %02X at %08X", I.opcode, I.pc - 1 );
}

static void I386OP(invalid)(void)
{
	//i386_trap(6);
	fatalerror("i386: Invalid opcode %02X at %08X", I.opcode, I.pc - 1);
}
