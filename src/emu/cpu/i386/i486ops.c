// Intel 486+ specific opcodes

static void I486OP(cpuid)(void)				// Opcode 0x0F A2
{
	switch (REG32(EAX))
	{
		case 0:
		{
			REG32(EAX) = I.cpuid_max_input_value_eax;
			REG32(EBX) = I.cpuid_id0;
			REG32(ECX) = I.cpuid_id2;
			REG32(EDX) = I.cpuid_id1;
			CYCLES(CYCLES_CPUID);
			break;
		}

		case 1:
		{
			REG32(EAX) = I.cpu_version;
			REG32(EDX) = I.feature_flags;
			CYCLES(CYCLES_CPUID_EAX1);
			break;
		}
	}
}

static void I486OP(invd)(void)				// Opcode 0x0f 08
{
	// Nothing to do ?
	CYCLES(CYCLES_INVD);
}

static void I486OP(wbinvd)(void)			// Opcode 0x0f 09
{
	// Nothing to do ?
}

static void I486OP(cmpxchg_rm8_r8)(void)	// Opcode 0x0f b0
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		UINT8 dst = LOAD_RM8(modrm);
		UINT8 src = LOAD_REG8(modrm);

		if( REG8(AL) == dst ) {
			STORE_RM8(modrm, src);
			I.ZF = 1;
			CYCLES(CYCLES_CMPXCHG_REG_REG_T);
		} else {
			REG8(AL) = dst;
			I.ZF = 0;
			CYCLES(CYCLES_CMPXCHG_REG_REG_F);
		}
	} else {
		UINT32 ea = GetEA(modrm);
		UINT8 dst = READ8(ea);
		UINT8 src = LOAD_REG8(modrm);

		if( REG8(AL) == dst ) {
			WRITE8(modrm, src);
			I.ZF = 1;
			CYCLES(CYCLES_CMPXCHG_REG_MEM_T);
		} else {
			REG8(AL) = dst;
			I.ZF = 0;
			CYCLES(CYCLES_CMPXCHG_REG_MEM_F);
		}
	}
}

static void I486OP(cmpxchg_rm16_r16)(void)	// Opcode 0x0f b1
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		UINT16 dst = LOAD_RM16(modrm);
		UINT16 src = LOAD_REG16(modrm);

		if( REG16(AX) == dst ) {
			STORE_RM16(modrm, src);
			I.ZF = 1;
			CYCLES(CYCLES_CMPXCHG_REG_REG_T);
		} else {
			REG16(AX) = dst;
			I.ZF = 0;
			CYCLES(CYCLES_CMPXCHG_REG_REG_F);
		}
	} else {
		UINT32 ea = GetEA(modrm);
		UINT16 dst = READ16(ea);
		UINT16 src = LOAD_REG16(modrm);

		if( REG16(AX) == dst ) {
			WRITE16(modrm, src);
			I.ZF = 1;
			CYCLES(CYCLES_CMPXCHG_REG_MEM_T);
		} else {
			REG16(AX) = dst;
			I.ZF = 0;
			CYCLES(CYCLES_CMPXCHG_REG_MEM_F);
		}
	}
}

static void I486OP(cmpxchg_rm32_r32)(void)	// Opcode 0x0f b1
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		UINT32 dst = LOAD_RM32(modrm);
		UINT32 src = LOAD_REG32(modrm);

		if( REG32(EAX) == dst ) {
			STORE_RM32(modrm, src);
			I.ZF = 1;
			CYCLES(CYCLES_CMPXCHG_REG_REG_T);
		} else {
			REG32(EAX) = dst;
			I.ZF = 0;
			CYCLES(CYCLES_CMPXCHG_REG_REG_F);
		}
	} else {
		UINT32 ea = GetEA(modrm);
		UINT32 dst = READ32(ea);
		UINT32 src = LOAD_REG32(modrm);

		if( REG32(EAX) == dst ) {
			WRITE32(ea, src);
			I.ZF = 1;
			CYCLES(CYCLES_CMPXCHG_REG_MEM_T);
		} else {
			REG32(EAX) = dst;
			I.ZF = 0;
			CYCLES(CYCLES_CMPXCHG_REG_MEM_F);
		}
	}
}

static void I486OP(xadd_rm8_r8)(void)	// Opcode 0x0f c0
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		UINT8 dst = LOAD_RM8(modrm);
		UINT8 src = LOAD_REG8(modrm);
		STORE_RM16(modrm, dst + src);
		STORE_REG16(modrm, dst);
		CYCLES(CYCLES_XADD_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		UINT8 dst = READ8(ea);
		UINT8 src = LOAD_REG8(modrm);
		WRITE8(ea, dst + src);
		STORE_REG8(modrm, dst);
		CYCLES(CYCLES_XADD_REG_MEM);
	}
}

static void I486OP(xadd_rm16_r16)(void)	// Opcode 0x0f c1
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		UINT16 dst = LOAD_RM16(modrm);
		UINT16 src = LOAD_REG16(modrm);
		STORE_RM16(modrm, dst + src);
		STORE_REG16(modrm, dst);
		CYCLES(CYCLES_XADD_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		UINT16 dst = READ16(ea);
		UINT16 src = LOAD_REG16(modrm);
		WRITE16(ea, dst + src);
		STORE_REG16(modrm, dst);
		CYCLES(CYCLES_XADD_REG_MEM);
	}
}

static void I486OP(xadd_rm32_r32)(void)	// Opcode 0x0f c1
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		UINT32 dst = LOAD_RM32(modrm);
		UINT32 src = LOAD_REG32(modrm);
		STORE_RM32(modrm, dst + src);
		STORE_REG32(modrm, dst);
		CYCLES(CYCLES_XADD_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm);
		UINT32 dst = READ32(ea);
		UINT32 src = LOAD_REG32(modrm);
		WRITE32(ea, dst + src);
		STORE_REG32(modrm, dst);
		CYCLES(CYCLES_XADD_REG_MEM);
	}
}

static void I486OP(group0F01_16)(void)		// Opcode 0x0f 01
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
		case 7:			/* INVLPG */
			{
				// Nothing to do ?
				break;
			}
		default:
			fatalerror("i486: unimplemented opcode 0x0f 01 /%d at %08X", (modrm >> 3) & 0x7, I.eip - 2);
			break;
	}
}

static void I486OP(group0F01_32)(void)		// Opcode 0x0f 01
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
		case 7:			/* INVLPG */
			{
				// Nothing to do ?
				break;
			}
		default:
			fatalerror("i486: unimplemented opcode 0x0f 01 /%d at %08X", (modrm >> 3) & 0x7, I.eip - 2);
			break;
	}
}

