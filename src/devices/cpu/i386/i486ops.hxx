// license:BSD-3-Clause
// copyright-holders:Ville Linde, Barry Rodewald, Carl, Philip Bennett
// Intel 486+ specific opcodes

void i386_device::i486_cpuid()             // Opcode 0x0F A2
{
	if (m_cpuid_id0 == 0)
	{
		// this 486 doesn't support the CPUID instruction
		logerror("CPUID not supported at %08x!\n", m_eip);
		i386_trap(6, 0, 0);
	}
	else
	{
		switch (REG32(EAX))
		{
			case 0:
			{
				REG32(EAX) = m_cpuid_max_input_value_eax;
				REG32(EBX) = m_cpuid_id0;
				REG32(ECX) = m_cpuid_id2;
				REG32(EDX) = m_cpuid_id1;
				CYCLES(CYCLES_CPUID);
				break;
			}

			case 1:
			{
				REG32(EAX) = m_cpu_version;
				REG32(EDX) = m_feature_flags;
				CYCLES(CYCLES_CPUID_EAX1);
				break;
			}
		}
	}
}

void i386_device::i486_invd()              // Opcode 0x0f 08
{
	// Nothing to do ?
	CYCLES(CYCLES_INVD);
}

void i386_device::i486_wbinvd()            // Opcode 0x0f 09
{
	// Nothing to do ?
}

void i386_device::i486_cmpxchg_rm8_r8()    // Opcode 0x0f b0
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		UINT8 dst = LOAD_RM8(modrm);
		UINT8 src = LOAD_REG8(modrm);

		if( REG8(AL) == dst ) {
			STORE_RM8(modrm, src);
			m_ZF = 1;
			CYCLES(CYCLES_CMPXCHG_REG_REG_T);
		} else {
			REG8(AL) = dst;
			m_ZF = 0;
			CYCLES(CYCLES_CMPXCHG_REG_REG_F);
		}
	} else {
		// TODO: Check write if needed
		UINT32 ea = GetEA(modrm,0);
		UINT8 dst = READ8(ea);
		UINT8 src = LOAD_REG8(modrm);

		if( REG8(AL) == dst ) {
			WRITE8(ea, src);
			m_ZF = 1;
			CYCLES(CYCLES_CMPXCHG_REG_MEM_T);
		} else {
			REG8(AL) = dst;
			m_ZF = 0;
			CYCLES(CYCLES_CMPXCHG_REG_MEM_F);
		}
	}
}

void i386_device::i486_cmpxchg_rm16_r16()  // Opcode 0x0f b1
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		UINT16 dst = LOAD_RM16(modrm);
		UINT16 src = LOAD_REG16(modrm);

		if( REG16(AX) == dst ) {
			STORE_RM16(modrm, src);
			m_ZF = 1;
			CYCLES(CYCLES_CMPXCHG_REG_REG_T);
		} else {
			REG16(AX) = dst;
			m_ZF = 0;
			CYCLES(CYCLES_CMPXCHG_REG_REG_F);
		}
	} else {
		UINT32 ea = GetEA(modrm,0);
		UINT16 dst = READ16(ea);
		UINT16 src = LOAD_REG16(modrm);

		if( REG16(AX) == dst ) {
			WRITE16(ea, src);
			m_ZF = 1;
			CYCLES(CYCLES_CMPXCHG_REG_MEM_T);
		} else {
			REG16(AX) = dst;
			m_ZF = 0;
			CYCLES(CYCLES_CMPXCHG_REG_MEM_F);
		}
	}
}

void i386_device::i486_cmpxchg_rm32_r32()  // Opcode 0x0f b1
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		UINT32 dst = LOAD_RM32(modrm);
		UINT32 src = LOAD_REG32(modrm);

		if( REG32(EAX) == dst ) {
			STORE_RM32(modrm, src);
			m_ZF = 1;
			CYCLES(CYCLES_CMPXCHG_REG_REG_T);
		} else {
			REG32(EAX) = dst;
			m_ZF = 0;
			CYCLES(CYCLES_CMPXCHG_REG_REG_F);
		}
	} else {
		UINT32 ea = GetEA(modrm,0);
		UINT32 dst = READ32(ea);
		UINT32 src = LOAD_REG32(modrm);

		if( REG32(EAX) == dst ) {
			WRITE32(ea, src);
			m_ZF = 1;
			CYCLES(CYCLES_CMPXCHG_REG_MEM_T);
		} else {
			REG32(EAX) = dst;
			m_ZF = 0;
			CYCLES(CYCLES_CMPXCHG_REG_MEM_F);
		}
	}
}

void i386_device::i486_xadd_rm8_r8()   // Opcode 0x0f c0
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		UINT8 dst = LOAD_RM8(modrm);
		UINT8 src = LOAD_REG8(modrm);
		STORE_REG8(modrm, dst);
		STORE_RM8(modrm, dst + src);
		CYCLES(CYCLES_XADD_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm,1);
		UINT8 dst = READ8(ea);
		UINT8 src = LOAD_REG8(modrm);
		WRITE8(ea, dst + src);
		STORE_REG8(modrm, dst);
		CYCLES(CYCLES_XADD_REG_MEM);
	}
}

void i386_device::i486_xadd_rm16_r16() // Opcode 0x0f c1
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		UINT16 dst = LOAD_RM16(modrm);
		UINT16 src = LOAD_REG16(modrm);
		STORE_REG16(modrm, dst);
		STORE_RM16(modrm, dst + src);
		CYCLES(CYCLES_XADD_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm,1);
		UINT16 dst = READ16(ea);
		UINT16 src = LOAD_REG16(modrm);
		WRITE16(ea, dst + src);
		STORE_REG16(modrm, dst);
		CYCLES(CYCLES_XADD_REG_MEM);
	}
}

void i386_device::i486_xadd_rm32_r32() // Opcode 0x0f c1
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		UINT32 dst = LOAD_RM32(modrm);
		UINT32 src = LOAD_REG32(modrm);
		STORE_REG32(modrm, dst);
		STORE_RM32(modrm, dst + src);
		CYCLES(CYCLES_XADD_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm,1);
		UINT32 dst = READ32(ea);
		UINT32 src = LOAD_REG32(modrm);
		WRITE32(ea, dst + src);
		STORE_REG32(modrm, dst);
		CYCLES(CYCLES_XADD_REG_MEM);
	}
}

void i386_device::i486_group0F01_16()      // Opcode 0x0f 01
{
	UINT8 modrm = FETCH();
	UINT16 address;
	UINT32 ea;

	switch( (modrm >> 3) & 0x7 )
	{
		case 0:         /* SGDT */
			{
				if( modrm >= 0xc0 ) {
					address = LOAD_RM16(modrm);
					ea = i386_translate( CS, address, 1 );
				} else {
					ea = GetEA(modrm,1);
				}
				WRITE16(ea, m_gdtr.limit);
				WRITE32(ea + 2, m_gdtr.base & 0xffffff);
				CYCLES(CYCLES_SGDT);
				break;
			}
		case 1:         /* SIDT */
			{
				if (modrm >= 0xc0)
				{
					address = LOAD_RM16(modrm);
					ea = i386_translate( CS, address, 1 );
				}
				else
				{
					ea = GetEA(modrm,1);
				}
				WRITE16(ea, m_idtr.limit);
				WRITE32(ea + 2, m_idtr.base & 0xffffff);
				CYCLES(CYCLES_SIDT);
				break;
			}
		case 2:         /* LGDT */
			{
				if(PROTECTED_MODE && m_CPL)
					FAULT(FAULT_GP,0)
				if( modrm >= 0xc0 ) {
					address = LOAD_RM16(modrm);
					ea = i386_translate( CS, address, 0 );
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
					ea = i386_translate( CS, address, 0 );
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
				UINT16 b;
				if(PROTECTED_MODE && m_CPL)
					FAULT(FAULT_GP,0)
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
		case 7:         /* INVLPG */
			{
				if(PROTECTED_MODE && m_CPL)
					FAULT(FAULT_GP,0)
				if(modrm >= 0xc0)
				{
					logerror("i486: invlpg with modrm %02X\n", modrm);
					FAULT(FAULT_UD,0)
				}
				ea = GetEA(modrm,-1);
				CYCLES(25); // TODO: add to cycles.h
				vtlb_flush_address(ea);
				break;
			}
		default:
			report_invalid_modrm("group0F01_16", modrm);
			break;
	}
}

void i386_device::i486_group0F01_32()      // Opcode 0x0f 01
{
	UINT8 modrm = FETCH();
	UINT32 address, ea;

	switch( (modrm >> 3) & 0x7 )
	{
		case 0:         /* SGDT */
			{
				if( modrm >= 0xc0 ) {
					address = LOAD_RM32(modrm);
					ea = i386_translate( CS, address, 1 );
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
					ea = i386_translate( CS, address, 1 );
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
					ea = i386_translate( CS, address, 0 );
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
					ea = i386_translate( CS, address, 0 );
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
					STORE_RM32(modrm, m_cr[0] & 0xffff);
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
		case 7:         /* INVLPG */
			{
				if(PROTECTED_MODE && m_CPL)
					FAULT(FAULT_GP,0)
				if(modrm >= 0xc0)
				{
					logerror("i486: invlpg with modrm %02X\n", modrm);
					FAULT(FAULT_UD,0)
				}
				ea = GetEA(modrm,-1);
				CYCLES(25); // TODO: add to cycles.h
				vtlb_flush_address(ea);
				break;
			}
		default:
			report_invalid_modrm("group0F01_32", modrm);
			break;
	}
}

void i386_device::i486_bswap_eax()     // Opcode 0x0f 38
{
	REG32(EAX) = SWITCH_ENDIAN_32(REG32(EAX));
	CYCLES(1);     // TODO
}

void i386_device::i486_bswap_ecx()     // Opcode 0x0f 39
{
	REG32(ECX) = SWITCH_ENDIAN_32(REG32(ECX));
	CYCLES(1);     // TODO
}

void i386_device::i486_bswap_edx()     // Opcode 0x0f 3A
{
	REG32(EDX) = SWITCH_ENDIAN_32(REG32(EDX));
	CYCLES(1);     // TODO
}

void i386_device::i486_bswap_ebx()     // Opcode 0x0f 3B
{
	REG32(EBX) = SWITCH_ENDIAN_32(REG32(EBX));
	CYCLES(1);     // TODO
}

void i386_device::i486_bswap_esp()     // Opcode 0x0f 3C
{
	REG32(ESP) = SWITCH_ENDIAN_32(REG32(ESP));
	CYCLES(1);     // TODO
}

void i386_device::i486_bswap_ebp()     // Opcode 0x0f 3D
{
	REG32(EBP) = SWITCH_ENDIAN_32(REG32(EBP));
	CYCLES(1);     // TODO
}

void i386_device::i486_bswap_esi()     // Opcode 0x0f 3E
{
	REG32(ESI) = SWITCH_ENDIAN_32(REG32(ESI));
	CYCLES(1);     // TODO
}

void i386_device::i486_bswap_edi()     // Opcode 0x0f 3F
{
	REG32(EDI) = SWITCH_ENDIAN_32(REG32(EDI));
	CYCLES(1);     // TODO
}

void i386_device::i486_mov_cr_r32()        // Opcode 0x0f 22
{
	if(PROTECTED_MODE && m_CPL)
		FAULT(FAULT_GP, 0);
	UINT8 modrm = FETCH();
	UINT8 cr = (modrm >> 3) & 0x7;
	UINT32 oldcr = m_cr[cr];
	UINT32 data = LOAD_RM32(modrm);
	switch(cr)
	{
		case 0:
			CYCLES(CYCLES_MOV_REG_CR0);
			if((oldcr ^ m_cr[cr]) & 0x80010000)
				vtlb_flush_dynamic();
			break;
		case 2: CYCLES(CYCLES_MOV_REG_CR2); break;
		case 3:
			CYCLES(CYCLES_MOV_REG_CR3);
			vtlb_flush_dynamic();
			break;
		case 4: CYCLES(1); break; // TODO
		default:
			logerror("i386: mov_cr_r32 CR%d!\n", cr);
			return;
	}
	m_cr[cr] = data;
}
