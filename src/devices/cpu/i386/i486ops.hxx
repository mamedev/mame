// license:BSD-3-Clause
// copyright-holders:Ville Linde, Barry Rodewald, Carl, Philip Bennett
// Intel 486+ specific opcodes

void i386_device::i486_cpuid()             // Opcode 0x0F A2
{
	if (m_cpuid_id0 == 0)
	{
		// this 486 doesn't support the CPUID instruction
		LOGMASKED(LOG_MSR, "CPUID not supported at %08x!\n", m_eip);
		i386_trap(6, 0);
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

			default:
			{
				// call the model specific implementation
				opcode_cpuid();
				break;
			}
		}
	}
}

void i386_device::i486_invd()              // Opcode 0x0f 08
{
	// TODO: manage the cache if present
	opcode_invd();
	CYCLES(CYCLES_INVD);
}

void i386_device::i486_wbinvd()            // Opcode 0x0f 09
{
	// TODO: manage the cache if present
	opcode_wbinvd();
}

void i386_device::i486_cmpxchg_rm8_r8()    // Opcode 0x0f b0
{
	uint8_t modrm = FETCH();
	if( modrm >= 0xc0 ) {
		uint8_t dst = LOAD_RM8(modrm);
		uint8_t src = LOAD_REG8(modrm);

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
		uint32_t ea = GetEA(modrm,0);
		uint8_t dst = READ8(ea);
		uint8_t src = LOAD_REG8(modrm);

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
	uint8_t modrm = FETCH();
	if( modrm >= 0xc0 ) {
		uint16_t dst = LOAD_RM16(modrm);
		uint16_t src = LOAD_REG16(modrm);

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
		uint32_t ea = GetEA(modrm,0);
		uint16_t dst = READ16(ea);
		uint16_t src = LOAD_REG16(modrm);

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
	uint8_t modrm = FETCH();
	if( modrm >= 0xc0 ) {
		uint32_t dst = LOAD_RM32(modrm);
		uint32_t src = LOAD_REG32(modrm);

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
		uint32_t ea = GetEA(modrm,0);
		uint32_t dst = READ32(ea);
		uint32_t src = LOAD_REG32(modrm);

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
	uint8_t modrm = FETCH();
	if( modrm >= 0xc0 ) {
		uint8_t dst = LOAD_RM8(modrm);
		uint8_t src = LOAD_REG8(modrm);
		uint8_t sum = ADD8(dst, src);
		STORE_REG8(modrm, dst);
		STORE_RM8(modrm, sum);
		CYCLES(CYCLES_XADD_REG_REG);
	} else {
		uint32_t ea = GetEA(modrm,1);
		uint8_t dst = READ8(ea);
		uint8_t src = LOAD_REG8(modrm);
		uint8_t sum = ADD8(dst, src);
		WRITE8(ea, sum);
		STORE_REG8(modrm, dst);
		CYCLES(CYCLES_XADD_REG_MEM);
	}
}

void i386_device::i486_xadd_rm16_r16() // Opcode 0x0f c1
{
	uint8_t modrm = FETCH();
	if( modrm >= 0xc0 ) {
		uint16_t dst = LOAD_RM16(modrm);
		uint16_t src = LOAD_REG16(modrm);
		uint16_t sum = ADD16(dst, src);
		STORE_REG16(modrm, dst);
		STORE_RM16(modrm, sum);
		CYCLES(CYCLES_XADD_REG_REG);
	} else {
		uint32_t ea = GetEA(modrm,1);
		uint16_t dst = READ16(ea);
		uint16_t src = LOAD_REG16(modrm);
		uint16_t sum = ADD16(dst, src);
		WRITE16(ea, sum);
		STORE_REG16(modrm, dst);
		CYCLES(CYCLES_XADD_REG_MEM);
	}
}

void i386_device::i486_xadd_rm32_r32() // Opcode 0x0f c1
{
	uint8_t modrm = FETCH();
	if( modrm >= 0xc0 ) {
		uint32_t dst = LOAD_RM32(modrm);
		uint32_t src = LOAD_REG32(modrm);
		uint32_t sum = ADD32(dst, src);
		STORE_REG32(modrm, dst);
		STORE_RM32(modrm, sum);
		CYCLES(CYCLES_XADD_REG_REG);
	} else {
		uint32_t ea = GetEA(modrm,1);
		uint32_t dst = READ32(ea);
		uint32_t src = LOAD_REG32(modrm);
		uint32_t sum = ADD32(dst, src);
		WRITE32(ea, sum);
		STORE_REG32(modrm, dst);
		CYCLES(CYCLES_XADD_REG_MEM);
	}
}

void i386_device::i486_group0F01_16()      // Opcode 0x0f 01
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
					ea = i386_translate( CS, address, 1 );
				} else {
					ea = GetEA(modrm,1);
				}
				WRITE16(ea, m_gdtr.limit);
				// Win32s requires all 32 bits to be stored here, despite various Intel docs
				// claiming that the upper 8 bits are either zeroed or undefined in 16-bit mode
				WRITE32(ea + 2, m_gdtr.base);
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
		case 7:         /* INVLPG */
			{
				if(PROTECTED_MODE && m_CPL)
					FAULT(FAULT_GP,0)
				if(modrm >= 0xc0)
				{
					LOGMASKED(LOG_PM_FAULT_UD, "i486: invlpg with modrm %02X\n", modrm);
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
	uint8_t modrm = FETCH();
	uint32_t address, ea;

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
		case 7:         /* INVLPG */
			{
				if(PROTECTED_MODE && m_CPL)
					FAULT(FAULT_GP,0)
				if(modrm >= 0xc0)
				{
					LOGMASKED(LOG_PM_FAULT_UD, "i486: invlpg with modrm %02X\n", modrm);
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
	REG32(EAX) = swapendian_int32(REG32(EAX));
	CYCLES(1);     // TODO
}

void i386_device::i486_bswap_ecx()     // Opcode 0x0f 39
{
	REG32(ECX) = swapendian_int32(REG32(ECX));
	CYCLES(1);     // TODO
}

void i386_device::i486_bswap_edx()     // Opcode 0x0f 3A
{
	REG32(EDX) = swapendian_int32(REG32(EDX));
	CYCLES(1);     // TODO
}

void i386_device::i486_bswap_ebx()     // Opcode 0x0f 3B
{
	REG32(EBX) = swapendian_int32(REG32(EBX));
	CYCLES(1);     // TODO
}

void i386_device::i486_bswap_esp()     // Opcode 0x0f 3C
{
	REG32(ESP) = swapendian_int32(REG32(ESP));
	CYCLES(1);     // TODO
}

void i386_device::i486_bswap_ebp()     // Opcode 0x0f 3D
{
	REG32(EBP) = swapendian_int32(REG32(EBP));
	CYCLES(1);     // TODO
}

void i386_device::i486_bswap_esi()     // Opcode 0x0f 3E
{
	REG32(ESI) = swapendian_int32(REG32(ESI));
	CYCLES(1);     // TODO
}

void i386_device::i486_bswap_edi()     // Opcode 0x0f 3F
{
	REG32(EDI) = swapendian_int32(REG32(EDI));
	CYCLES(1);     // TODO
}

void i386_device::i486_mov_cr_r32()        // Opcode 0x0f 22
{
	if(PROTECTED_MODE && m_CPL)
		FAULT(FAULT_GP, 0);
	uint8_t modrm = FETCH();
	uint8_t cr = (modrm >> 3) & 0x7;
	uint32_t oldcr = m_cr[cr];
	uint32_t data = LOAD_RM32(modrm);
	switch(cr)
	{
		case 0:
			CYCLES(CYCLES_MOV_REG_CR0);
			if((oldcr ^ m_cr[cr]) & (CR0_PG | CR0_WP))
				vtlb_flush_dynamic();
			if (PROTECTED_MODE != BIT(data, 0))
				debugger_privilege_hook();
			break;
		case 2: CYCLES(CYCLES_MOV_REG_CR2); break;
		case 3:
			CYCLES(CYCLES_MOV_REG_CR3);
			vtlb_flush_dynamic();
			break;
		case 4: CYCLES(1); break; // TODO
		default:
			LOGMASKED(LOG_INVALID_OPCODE, "i386: mov_cr_r32 CR%d!\n", cr);
			return;
	}
	m_cr[cr] = data;
}

void i386_device::i486_wait()
{
	if ((m_cr[0] & (CR0_TS | CR0_MP)) == (CR0_TS | CR0_MP))
	{
		i386_trap(FAULT_NM, 0);
		return;
	}
	x87_mf_fault();
}
