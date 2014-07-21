// license:BSD-3-Clause
// copyright-holders:Ville Linde, Barry Rodewald, Carl, Phil Bennett
// Pentium+ specific opcodes

extern flag float32_is_nan( float32 a ); // since its not defined in softfloat.h

void i386_device::MMXPROLOG()
{
	//m_x87_sw &= ~(X87_SW_TOP_MASK << X87_SW_TOP_SHIFT); // top = 0
	m_x87_tw = 0; // tag word = 0
}

void i386_device::READMMX(UINT32 ea,MMX_REG &r)
{
	r.q=READ64(ea);
}

void i386_device::WRITEMMX(UINT32 ea,MMX_REG &r)
{
	WRITE64(ea, r.q);
}

void i386_device::READXMM(UINT32 ea,XMM_REG &r)
{
	r.q[0]=READ64(ea);
	r.q[1]=READ64(ea+8);
}

void i386_device::WRITEXMM(UINT32 ea,i386_device::XMM_REG &r)
{
	WRITE64(ea, r.q[0]);
	WRITE64(ea+8, r.q[1]);
}

void i386_device::READXMM_LO64(UINT32 ea,i386_device::XMM_REG &r)
{
	r.q[0]=READ64(ea);
}

void i386_device::WRITEXMM_LO64(UINT32 ea,i386_device::XMM_REG &r)
{
	WRITE64(ea, r.q[0]);
}

void i386_device::READXMM_HI64(UINT32 ea,i386_device::XMM_REG &r)
{
	r.q[1]=READ64(ea);
}

void i386_device::WRITEXMM_HI64(UINT32 ea,i386_device::XMM_REG &r)
{
	WRITE64(ea, r.q[1]);
}

void i386_device::pentium_rdmsr()          // Opcode 0x0f 32
{
	UINT64 data;
	UINT8 valid_msr = 0;

	data = MSR_READ(REG32(ECX),&valid_msr);
	REG32(EDX) = data >> 32;
	REG32(EAX) = data & 0xffffffff;

	if(m_CPL != 0 || valid_msr == 0) // if current privilege level isn't 0 or the register isn't recognized ...
		FAULT(FAULT_GP,0) // ... throw a general exception fault

	CYCLES(CYCLES_RDMSR);
}

void i386_device::pentium_wrmsr()          // Opcode 0x0f 30
{
	UINT64 data;
	UINT8 valid_msr = 0;

	data = (UINT64)REG32(EAX);
	data |= (UINT64)(REG32(EDX)) << 32;

	MSR_WRITE(REG32(ECX),data,&valid_msr);

	if(m_CPL != 0 || valid_msr == 0) // if current privilege level isn't 0 or the register isn't recognized
		FAULT(FAULT_GP,0) // ... throw a general exception fault

	CYCLES(1);     // TODO: correct cycle count (~30-45)
}

void i386_device::pentium_rdtsc()          // Opcode 0x0f 31
{
	UINT64 ts = m_tsc + (m_base_cycles - m_cycles);
	REG32(EAX) = (UINT32)(ts);
	REG32(EDX) = (UINT32)(ts >> 32);

	CYCLES(CYCLES_RDTSC);
}

void i386_device::pentium_ud2()    // Opcode 0x0f 0b
{
	i386_trap(6, 0, 0);
}

void i386_device::pentium_rsm()
{
	UINT32 smram_state = m_smbase + 0xfe00;
	if(!m_smm)
	{
		logerror("i386: Invalid RSM outside SMM at %08X\n", m_pc - 1);
		i386_trap(6, 0, 0);
		return;
	}

	// load state, no sanity checks anywhere
	m_smbase = READ32(smram_state+SMRAM_SMBASE);
	m_cr[4] = READ32(smram_state+SMRAM_IP5_CR4);
	m_sreg[ES].limit = READ32(smram_state+SMRAM_IP5_ESLIM);
	m_sreg[ES].base = READ32(smram_state+SMRAM_IP5_ESBASE);
	m_sreg[ES].flags = READ32(smram_state+SMRAM_IP5_ESACC);
	m_sreg[CS].limit = READ32(smram_state+SMRAM_IP5_CSLIM);
	m_sreg[CS].base = READ32(smram_state+SMRAM_IP5_CSBASE);
	m_sreg[CS].flags = READ32(smram_state+SMRAM_IP5_CSACC);
	m_sreg[SS].limit = READ32(smram_state+SMRAM_IP5_SSLIM);
	m_sreg[SS].base = READ32(smram_state+SMRAM_IP5_SSBASE);
	m_sreg[SS].flags = READ32(smram_state+SMRAM_IP5_SSACC);
	m_sreg[DS].limit = READ32(smram_state+SMRAM_IP5_DSLIM);
	m_sreg[DS].base = READ32(smram_state+SMRAM_IP5_DSBASE);
	m_sreg[DS].flags = READ32(smram_state+SMRAM_IP5_DSACC);
	m_sreg[FS].limit = READ32(smram_state+SMRAM_IP5_FSLIM);
	m_sreg[FS].base = READ32(smram_state+SMRAM_IP5_FSBASE);
	m_sreg[FS].flags = READ32(smram_state+SMRAM_IP5_FSACC);
	m_sreg[GS].limit = READ32(smram_state+SMRAM_IP5_GSLIM);
	m_sreg[GS].base = READ32(smram_state+SMRAM_IP5_GSBASE);
	m_sreg[GS].flags = READ32(smram_state+SMRAM_IP5_GSACC);
	m_ldtr.flags = READ32(smram_state+SMRAM_IP5_LDTACC);
	m_ldtr.limit = READ32(smram_state+SMRAM_IP5_LDTLIM);
	m_ldtr.base = READ32(smram_state+SMRAM_IP5_LDTBASE);
	m_gdtr.limit = READ32(smram_state+SMRAM_IP5_GDTLIM);
	m_gdtr.base = READ32(smram_state+SMRAM_IP5_GDTBASE);
	m_idtr.limit = READ32(smram_state+SMRAM_IP5_IDTLIM);
	m_idtr.base = READ32(smram_state+SMRAM_IP5_IDTBASE);
	m_task.limit = READ32(smram_state+SMRAM_IP5_TRLIM);
	m_task.base = READ32(smram_state+SMRAM_IP5_TRBASE);
	m_task.flags = READ32(smram_state+SMRAM_IP5_TRACC);

	m_sreg[ES].selector = READ32(smram_state+SMRAM_ES);
	m_sreg[CS].selector = READ32(smram_state+SMRAM_CS);
	m_sreg[SS].selector = READ32(smram_state+SMRAM_SS);
	m_sreg[DS].selector = READ32(smram_state+SMRAM_DS);
	m_sreg[FS].selector = READ32(smram_state+SMRAM_FS);
	m_sreg[GS].selector = READ32(smram_state+SMRAM_GS);
	m_ldtr.segment = READ32(smram_state+SMRAM_LDTR);
	m_task.segment = READ32(smram_state+SMRAM_TR);

	m_dr[7] = READ32(smram_state+SMRAM_DR7);
	m_dr[6] = READ32(smram_state+SMRAM_DR6);
	REG32(EAX) = READ32(smram_state+SMRAM_EAX);
	REG32(ECX) = READ32(smram_state+SMRAM_ECX);
	REG32(EDX) = READ32(smram_state+SMRAM_EDX);
	REG32(EBX) = READ32(smram_state+SMRAM_EBX);
	REG32(ESP) = READ32(smram_state+SMRAM_ESP);
	REG32(EBP) = READ32(smram_state+SMRAM_EBP);
	REG32(ESI) = READ32(smram_state+SMRAM_ESI);
	REG32(EDI) = READ32(smram_state+SMRAM_EDI);
	m_eip = READ32(smram_state+SMRAM_EIP);
	m_eflags = READ32(smram_state+SMRAM_EAX);
	m_cr[3] = READ32(smram_state+SMRAM_CR3);
	m_cr[0] = READ32(smram_state+SMRAM_CR0);

	m_CPL = (m_sreg[SS].flags >> 13) & 3; // cpl == dpl of ss

	for(int i = 0; i < GS; i++)
	{
		if(PROTECTED_MODE && !V8086_MODE)
		{
			m_sreg[i].valid = m_sreg[i].selector ? true : false;
			m_sreg[i].d = (m_sreg[i].flags & 0x4000) ? 1 : 0;
		}
		else
			m_sreg[i].valid = true;
	}

	if(!m_smiact.isnull())
		m_smiact(false);
	m_smm = false;

	CHANGE_PC(m_eip);
	m_nmi_masked = false;
	if(m_smi_latched)
	{
		pentium_smi();
		return;
	}
	if(m_nmi_latched)
	{
		m_nmi_latched = false;
		i386_trap(2, 1, 0);
	}
}

void i386_device::pentium_prefetch_m8()    // Opcode 0x0f 18
{
	UINT8 modrm = FETCH();
	UINT32 ea = GetEA(modrm,0);
	CYCLES(1+(ea & 1)); // TODO: correct cycle count
}

void i386_device::pentium_cmovo_r16_rm16()    // Opcode 0x0f 40
{
	UINT16 src;
	UINT8 modrm = FETCH();

	if( modrm >= 0xc0 )
	{
		if (m_OF == 1)
		{
			src = LOAD_RM16(modrm);
			STORE_REG16(modrm, src);
		}
		CYCLES(1); // TODO: correct cycle count
	}
	else
	{
		UINT32 ea = GetEA(modrm,0);
		if (m_OF == 1)
		{
			src = READ16(ea);
			STORE_REG16(modrm, src);
		}
		CYCLES(1); // TODO: correct cycle count
	}
}

void i386_device::pentium_cmovo_r32_rm32()    // Opcode 0x0f 40
{
	UINT32 src;
	UINT8 modrm = FETCH();

	if( modrm >= 0xc0 )
	{
		if (m_OF == 1)
		{
			src = LOAD_RM32(modrm);
			STORE_REG32(modrm, src);
		}
		CYCLES(1); // TODO: correct cycle count
	}
	else
	{
		UINT32 ea = GetEA(modrm,0);
		if (m_OF == 1)
		{
			src = READ32(ea);
			STORE_REG32(modrm, src);
		}
		CYCLES(1); // TODO: correct cycle count
	}
}

void i386_device::pentium_cmovno_r16_rm16()    // Opcode 0x0f 41
{
	UINT16 src;
	UINT8 modrm = FETCH();

	if( modrm >= 0xc0 )
	{
		if (m_OF == 0)
		{
			src = LOAD_RM16(modrm);
			STORE_REG16(modrm, src);
		}
		CYCLES(1); // TODO: correct cycle count
	}
	else
	{
		UINT32 ea = GetEA(modrm,0);
		if (m_OF == 0)
		{
			src = READ16(ea);
			STORE_REG16(modrm, src);
		}
		CYCLES(1); // TODO: correct cycle count
	}
}

void i386_device::pentium_cmovno_r32_rm32()    // Opcode 0x0f 41
{
	UINT32 src;
	UINT8 modrm = FETCH();

	if( modrm >= 0xc0 )
	{
		if (m_OF == 0)
		{
			src = LOAD_RM32(modrm);
			STORE_REG32(modrm, src);
		}
		CYCLES(1); // TODO: correct cycle count
	}
	else
	{
		UINT32 ea = GetEA(modrm,0);
		if (m_OF == 0)
		{
			src = READ32(ea);
			STORE_REG32(modrm, src);
		}
		CYCLES(1); // TODO: correct cycle count
	}
}

void i386_device::pentium_cmovb_r16_rm16()    // Opcode 0x0f 42
{
	UINT16 src;
	UINT8 modrm = FETCH();

	if( modrm >= 0xc0 )
	{
		if (m_CF == 1)
		{
			src = LOAD_RM16(modrm);
			STORE_REG16(modrm, src);
		}
		CYCLES(1); // TODO: correct cycle count
	}
	else
	{
		UINT32 ea = GetEA(modrm,0);
		if (m_CF == 1)
		{
			src = READ16(ea);
			STORE_REG16(modrm, src);
		}
		CYCLES(1); // TODO: correct cycle count
	}
}

void i386_device::pentium_cmovb_r32_rm32()    // Opcode 0x0f 42
{
	UINT32 src;
	UINT8 modrm = FETCH();

	if( modrm >= 0xc0 )
	{
		if (m_CF == 1)
		{
			src = LOAD_RM32(modrm);
			STORE_REG32(modrm, src);
		}
		CYCLES(1); // TODO: correct cycle count
	}
	else
	{
		UINT32 ea = GetEA(modrm,0);
		if (m_CF == 1)
		{
			src = READ32(ea);
			STORE_REG32(modrm, src);
		}
		CYCLES(1); // TODO: correct cycle count
	}
}

void i386_device::pentium_cmovae_r16_rm16()    // Opcode 0x0f 43
{
	UINT16 src;
	UINT8 modrm = FETCH();

	if( modrm >= 0xc0 )
	{
		if (m_CF == 0)
		{
			src = LOAD_RM16(modrm);
			STORE_REG16(modrm, src);
		}
		CYCLES(1); // TODO: correct cycle count
	}
	else
	{
		UINT32 ea = GetEA(modrm,0);
		if (m_CF == 0)
		{
			src = READ16(ea);
			STORE_REG16(modrm, src);
		}
		CYCLES(1); // TODO: correct cycle count
	}
}

void i386_device::pentium_cmovae_r32_rm32()    // Opcode 0x0f 43
{
	UINT32 src;
	UINT8 modrm = FETCH();

	if( modrm >= 0xc0 )
	{
		if (m_CF == 0)
		{
			src = LOAD_RM32(modrm);
			STORE_REG32(modrm, src);
		}
		CYCLES(1); // TODO: correct cycle count
	}
	else
	{
		UINT32 ea = GetEA(modrm,0);
		if (m_CF == 0)
		{
			src = READ32(ea);
			STORE_REG32(modrm, src);
		}
		CYCLES(1); // TODO: correct cycle count
	}
}

void i386_device::pentium_cmove_r16_rm16()    // Opcode 0x0f 44
{
	UINT16 src;
	UINT8 modrm = FETCH();

	if( modrm >= 0xc0 )
	{
		if (m_ZF == 1)
		{
			src = LOAD_RM16(modrm);
			STORE_REG16(modrm, src);
		}
		CYCLES(1); // TODO: correct cycle count
	}
	else
	{
		UINT32 ea = GetEA(modrm,0);
		if (m_ZF == 1)
		{
			src = READ16(ea);
			STORE_REG16(modrm, src);
		}
		CYCLES(1); // TODO: correct cycle count
	}
}

void i386_device::pentium_cmove_r32_rm32()    // Opcode 0x0f 44
{
	UINT32 src;
	UINT8 modrm = FETCH();

	if( modrm >= 0xc0 )
	{
		if (m_ZF == 1)
		{
			src = LOAD_RM32(modrm);
			STORE_REG32(modrm, src);
		}
		CYCLES(1); // TODO: correct cycle count
	}
	else
	{
		UINT32 ea = GetEA(modrm,0);
		if (m_ZF == 1)
		{
			src = READ32(ea);
			STORE_REG32(modrm, src);
		}
		CYCLES(1); // TODO: correct cycle count
	}
}

void i386_device::pentium_cmovne_r16_rm16()    // Opcode 0x0f 45
{
	UINT16 src;
	UINT8 modrm = FETCH();

	if( modrm >= 0xc0 )
	{
		if (m_ZF == 0)
		{
			src = LOAD_RM16(modrm);
			STORE_REG16(modrm, src);
		}
		CYCLES(1); // TODO: correct cycle count
	}
	else
	{
		UINT32 ea = GetEA(modrm,0);
		if (m_ZF == 0)
		{
			src = READ16(ea);
			STORE_REG16(modrm, src);
		}
		CYCLES(1); // TODO: correct cycle count
	}
}

void i386_device::pentium_cmovne_r32_rm32()    // Opcode 0x0f 45
{
	UINT32 src;
	UINT8 modrm = FETCH();

	if( modrm >= 0xc0 )
	{
		if (m_ZF == 0)
		{
			src = LOAD_RM32(modrm);
			STORE_REG32(modrm, src);
		}
		CYCLES(1); // TODO: correct cycle count
	}
	else
	{
		UINT32 ea = GetEA(modrm,0);
		if (m_ZF == 0)
		{
			src = READ32(ea);
			STORE_REG32(modrm, src);
		}
		CYCLES(1); // TODO: correct cycle count
	}
}

void i386_device::pentium_cmovbe_r16_rm16()    // Opcode 0x0f 46
{
	UINT16 src;
	UINT8 modrm = FETCH();

	if( modrm >= 0xc0 )
	{
		if ((m_CF == 1) || (m_ZF == 1))
		{
			src = LOAD_RM16(modrm);
			STORE_REG16(modrm, src);
		}
		CYCLES(1); // TODO: correct cycle count
	}
	else
	{
		UINT32 ea = GetEA(modrm,0);
		if ((m_CF == 1) || (m_ZF == 1))
		{
			src = READ16(ea);
			STORE_REG16(modrm, src);
		}
		CYCLES(1); // TODO: correct cycle count
	}
}

void i386_device::pentium_cmovbe_r32_rm32()    // Opcode 0x0f 46
{
	UINT32 src;
	UINT8 modrm = FETCH();

	if( modrm >= 0xc0 )
	{
		if ((m_CF == 1) || (m_ZF == 1))
		{
			src = LOAD_RM32(modrm);
			STORE_REG32(modrm, src);
		}
		CYCLES(1); // TODO: correct cycle count
	}
	else
	{
		UINT32 ea = GetEA(modrm,0);
		if ((m_CF == 1) || (m_ZF == 1))
		{
			src = READ32(ea);
			STORE_REG32(modrm, src);
		}
		CYCLES(1); // TODO: correct cycle count
	}
}

void i386_device::pentium_cmova_r16_rm16()    // Opcode 0x0f 47
{
	UINT16 src;
	UINT8 modrm = FETCH();

	if( modrm >= 0xc0 )
	{
		if ((m_CF == 0) && (m_ZF == 0))
		{
			src = LOAD_RM16(modrm);
			STORE_REG16(modrm, src);
		}
		CYCLES(1); // TODO: correct cycle count
	}
	else
	{
		UINT32 ea = GetEA(modrm,0);
		if ((m_CF == 0) && (m_ZF == 0))
		{
			src = READ16(ea);
			STORE_REG16(modrm, src);
		}
		CYCLES(1); // TODO: correct cycle count
	}
}

void i386_device::pentium_cmova_r32_rm32()    // Opcode 0x0f 47
{
	UINT32 src;
	UINT8 modrm = FETCH();

	if( modrm >= 0xc0 )
	{
		if ((m_CF == 0) && (m_ZF == 0))
		{
			src = LOAD_RM32(modrm);
			STORE_REG32(modrm, src);
		}
		CYCLES(1); // TODO: correct cycle count
	}
	else
	{
		UINT32 ea = GetEA(modrm,0);
		if ((m_CF == 0) && (m_ZF == 0))
		{
			src = READ32(ea);
			STORE_REG32(modrm, src);
		}
		CYCLES(1); // TODO: correct cycle count
	}
}

void i386_device::pentium_cmovs_r16_rm16()    // Opcode 0x0f 48
{
	UINT16 src;
	UINT8 modrm = FETCH();

	if( modrm >= 0xc0 )
	{
		if (m_SF == 1)
		{
			src = LOAD_RM16(modrm);
			STORE_REG16(modrm, src);
		}
		CYCLES(1); // TODO: correct cycle count
	}
	else
	{
		UINT32 ea = GetEA(modrm,0);
		if (m_SF == 1)
		{
			src = READ16(ea);
			STORE_REG16(modrm, src);
		}
		CYCLES(1); // TODO: correct cycle count
	}
}

void i386_device::pentium_cmovs_r32_rm32()    // Opcode 0x0f 48
{
	UINT32 src;
	UINT8 modrm = FETCH();

	if( modrm >= 0xc0 )
	{
		if (m_SF == 1)
		{
			src = LOAD_RM32(modrm);
			STORE_REG32(modrm, src);
		}
		CYCLES(1); // TODO: correct cycle count
	}
	else
	{
		UINT32 ea = GetEA(modrm,0);
		if (m_SF == 1)
		{
			src = READ32(ea);
			STORE_REG32(modrm, src);
		}
		CYCLES(1); // TODO: correct cycle count
	}
}

void i386_device::pentium_cmovns_r16_rm16()    // Opcode 0x0f 49
{
	UINT16 src;
	UINT8 modrm = FETCH();

	if( modrm >= 0xc0 )
	{
		if (m_SF == 0)
		{
			src = LOAD_RM16(modrm);
			STORE_REG16(modrm, src);
		}
		CYCLES(1); // TODO: correct cycle count
	}
	else
	{
		UINT32 ea = GetEA(modrm,0);
		if (m_SF == 0)
		{
			src = READ16(ea);
			STORE_REG16(modrm, src);
		}
		CYCLES(1); // TODO: correct cycle count
	}
}

void i386_device::pentium_cmovns_r32_rm32()    // Opcode 0x0f 49
{
	UINT32 src;
	UINT8 modrm = FETCH();

	if( modrm >= 0xc0 )
	{
		if (m_SF == 0)
		{
			src = LOAD_RM32(modrm);
			STORE_REG32(modrm, src);
		}
		CYCLES(1); // TODO: correct cycle count
	}
	else
	{
		UINT32 ea = GetEA(modrm,0);
		if (m_SF == 0)
		{
			src = READ32(ea);
			STORE_REG32(modrm, src);
		}
		CYCLES(1); // TODO: correct cycle count
	}
}

void i386_device::pentium_cmovp_r16_rm16()    // Opcode 0x0f 4a
{
	UINT16 src;
	UINT8 modrm = FETCH();

	if( modrm >= 0xc0 )
	{
		if (m_PF == 1)
		{
			src = LOAD_RM16(modrm);
			STORE_REG16(modrm, src);
		}
		CYCLES(1); // TODO: correct cycle count
	}
	else
	{
		UINT32 ea = GetEA(modrm,0);
		if (m_PF == 1)
		{
			src = READ16(ea);
			STORE_REG16(modrm, src);
		}
		CYCLES(1); // TODO: correct cycle count
	}
}

void i386_device::pentium_cmovp_r32_rm32()    // Opcode 0x0f 4a
{
	UINT32 src;
	UINT8 modrm = FETCH();

	if( modrm >= 0xc0 )
	{
		if (m_PF == 1)
		{
			src = LOAD_RM32(modrm);
			STORE_REG32(modrm, src);
		}
		CYCLES(1); // TODO: correct cycle count
	}
	else
	{
		UINT32 ea = GetEA(modrm,0);
		if (m_PF == 1)
		{
			src = READ32(ea);
			STORE_REG32(modrm, src);
		}
		CYCLES(1); // TODO: correct cycle count
	}
}

void i386_device::pentium_cmovnp_r16_rm16()    // Opcode 0x0f 4b
{
	UINT16 src;
	UINT8 modrm = FETCH();

	if( modrm >= 0xc0 )
	{
		if (m_PF == 0)
		{
			src = LOAD_RM16(modrm);
			STORE_REG16(modrm, src);
		}
		CYCLES(1); // TODO: correct cycle count
	}
	else
	{
		UINT32 ea = GetEA(modrm,0);
		if (m_PF == 0)
		{
			src = READ16(ea);
			STORE_REG16(modrm, src);
		}
		CYCLES(1); // TODO: correct cycle count
	}
}

void i386_device::pentium_cmovnp_r32_rm32()    // Opcode 0x0f 4b
{
	UINT32 src;
	UINT8 modrm = FETCH();

	if( modrm >= 0xc0 )
	{
		if (m_PF == 0)
		{
			src = LOAD_RM32(modrm);
			STORE_REG32(modrm, src);
		}
		CYCLES(1); // TODO: correct cycle count
	}
	else
	{
		UINT32 ea = GetEA(modrm,0);
		if (m_PF == 0)
		{
			src = READ32(ea);
			STORE_REG32(modrm, src);
		}
		CYCLES(1); // TODO: correct cycle count
	}
}

void i386_device::pentium_cmovl_r16_rm16()    // Opcode 0x0f 4c
{
	UINT16 src;
	UINT8 modrm = FETCH();

	if( modrm >= 0xc0 )
	{
		if (m_SF != m_OF)
		{
			src = LOAD_RM16(modrm);
			STORE_REG16(modrm, src);
		}
		CYCLES(1); // TODO: correct cycle count
	}
	else
	{
		UINT32 ea = GetEA(modrm,0);
		if (m_SF != m_OF)
		{
			src = READ16(ea);
			STORE_REG16(modrm, src);
		}
		CYCLES(1); // TODO: correct cycle count
	}
}

void i386_device::pentium_cmovl_r32_rm32()    // Opcode 0x0f 4c
{
	UINT32 src;
	UINT8 modrm = FETCH();

	if( modrm >= 0xc0 )
	{
		if (m_SF != m_OF)
		{
			src = LOAD_RM32(modrm);
			STORE_REG32(modrm, src);
		}
		CYCLES(1); // TODO: correct cycle count
	}
	else
	{
		UINT32 ea = GetEA(modrm,0);
		if (m_SF != m_OF)
		{
			src = READ32(ea);
			STORE_REG32(modrm, src);
		}
		CYCLES(1); // TODO: correct cycle count
	}
}

void i386_device::pentium_cmovge_r16_rm16()    // Opcode 0x0f 4d
{
	UINT16 src;
	UINT8 modrm = FETCH();

	if( modrm >= 0xc0 )
	{
		if (m_SF == m_OF)
		{
			src = LOAD_RM16(modrm);
			STORE_REG16(modrm, src);
		}
		CYCLES(1); // TODO: correct cycle count
	}
	else
	{
		UINT32 ea = GetEA(modrm,0);
		if (m_SF == m_OF)
		{
			src = READ16(ea);
			STORE_REG16(modrm, src);
		}
		CYCLES(1); // TODO: correct cycle count
	}
}

void i386_device::pentium_cmovge_r32_rm32()    // Opcode 0x0f 4d
{
	UINT32 src;
	UINT8 modrm = FETCH();

	if( modrm >= 0xc0 )
	{
		if (m_SF == m_OF)
		{
			src = LOAD_RM32(modrm);
			STORE_REG32(modrm, src);
		}
		CYCLES(1); // TODO: correct cycle count
	}
	else
	{
		UINT32 ea = GetEA(modrm,0);
		if (m_SF == m_OF)
		{
			src = READ32(ea);
			STORE_REG32(modrm, src);
		}
		CYCLES(1); // TODO: correct cycle count
	}
}

void i386_device::pentium_cmovle_r16_rm16()    // Opcode 0x0f 4e
{
	UINT16 src;
	UINT8 modrm = FETCH();

	if( modrm >= 0xc0 )
	{
		if ((m_ZF == 1) || (m_SF != m_OF))
		{
			src = LOAD_RM16(modrm);
			STORE_REG16(modrm, src);
		}
		CYCLES(1); // TODO: correct cycle count
	}
	else
	{
		UINT32 ea = GetEA(modrm,0);
		if ((m_ZF == 1) || (m_SF != m_OF))
		{
			src = READ16(ea);
			STORE_REG16(modrm, src);
		}
		CYCLES(1); // TODO: correct cycle count
	}
}

void i386_device::pentium_cmovle_r32_rm32()    // Opcode 0x0f 4e
{
	UINT32 src;
	UINT8 modrm = FETCH();

	if( modrm >= 0xc0 )
	{
		if ((m_ZF == 1) || (m_SF != m_OF))
		{
			src = LOAD_RM32(modrm);
			STORE_REG32(modrm, src);
		}
		CYCLES(1); // TODO: correct cycle count
	}
	else
	{
		UINT32 ea = GetEA(modrm,0);
		if ((m_ZF == 1) || (m_SF != m_OF))
		{
			src = READ32(ea);
			STORE_REG32(modrm, src);
		}
		CYCLES(1); // TODO: correct cycle count
	}
}

void i386_device::pentium_cmovg_r16_rm16()    // Opcode 0x0f 4f
{
	UINT16 src;
	UINT8 modrm = FETCH();

	if( modrm >= 0xc0 )
	{
		if ((m_ZF == 0) && (m_SF == m_OF))
		{
			src = LOAD_RM16(modrm);
			STORE_REG16(modrm, src);
		}
		CYCLES(1); // TODO: correct cycle count
	}
	else
	{
		UINT32 ea = GetEA(modrm,0);
		if ((m_ZF == 0) && (m_SF == m_OF))
		{
			src = READ16(ea);
			STORE_REG16(modrm, src);
		}
		CYCLES(1); // TODO: correct cycle count
	}
}

void i386_device::pentium_cmovg_r32_rm32()    // Opcode 0x0f 4f
{
	UINT32 src;
	UINT8 modrm = FETCH();

	if( modrm >= 0xc0 )
	{
		if ((m_ZF == 0) && (m_SF == m_OF))
		{
			src = LOAD_RM32(modrm);
			STORE_REG32(modrm, src);
		}
		CYCLES(1); // TODO: correct cycle count
	}
	else
	{
		UINT32 ea = GetEA(modrm,0);
		if ((m_ZF == 0) && (m_SF == m_OF))
		{
			src = READ32(ea);
			STORE_REG32(modrm, src);
		}
		CYCLES(1); // TODO: correct cycle count
	}
}

void i386_device::pentium_movnti_m16_r16() // Opcode 0f c3
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		// unsupported by cpu
		CYCLES(1);     // TODO: correct cycle count
	} else {
		// since cache is not implemented
		UINT32 ea = GetEA(modrm, 0);
		WRITE16(ea,LOAD_RM16(modrm));
		CYCLES(1);     // TODO: correct cycle count
	}
}

void i386_device::pentium_movnti_m32_r32() // Opcode 0f c3
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		// unsupported by cpu
		CYCLES(1);     // TODO: correct cycle count
	} else {
		// since cache is not implemented
		UINT32 ea = GetEA(modrm, 0);
		WRITE32(ea,LOAD_RM32(modrm));
		CYCLES(1);     // TODO: correct cycle count
	}
}

void i386_device::i386_cyrix_unknown()     // Opcode 0x0f 74
{
	logerror("Unemulated 0x0f 0x74 opcode called\n");

	CYCLES(1);
}

void i386_device::pentium_cmpxchg8b_m64()  // Opcode 0x0f c7
{
	UINT8 modm = FETCH();
	if( modm >= 0xc0 ) {
		report_invalid_modrm("cmpxchg8b_m64", modm);
	} else {
		UINT32 ea = GetEA(modm, 0);
		UINT64 value = READ64(ea);
		UINT64 edx_eax = (((UINT64) REG32(EDX)) << 32) | REG32(EAX);
		UINT64 ecx_ebx = (((UINT64) REG32(ECX)) << 32) | REG32(EBX);

		if( value == edx_eax ) {
			WRITE64(ea, ecx_ebx);
			m_ZF = 1;
			CYCLES(CYCLES_CMPXCHG_REG_MEM_T);
		} else {
			REG32(EDX) = (UINT32) (value >> 32);
			REG32(EAX) = (UINT32) (value >>  0);
			m_ZF = 0;
			CYCLES(CYCLES_CMPXCHG_REG_MEM_F);
		}
	}
}

void i386_device::pentium_movntq_m64_r64() // Opcode 0f e7
{
	//MMXPROLOG(); // TODO: check if needed
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		CYCLES(1);     // unsupported
	} else {
		// since cache is not implemented
		UINT32 ea = GetEA(modrm, 0);
		WRITEMMX(ea, MMX((modrm >> 3) & 0x7));
		CYCLES(1);     // TODO: correct cycle count
	}
}

void i386_device::pentium_maskmovq_r64_r64()  // Opcode 0f f7
{
	int s,m,n;
	UINT8 modm = FETCH();
	UINT32 ea = GetEA(7, 0); // ds:di/edi/rdi register
	MMXPROLOG();
	s=(modm >> 3) & 7;
	m=modm & 7;
	for (n=0;n <= 7;n++)
		if (MMX(m).b[n] & 127)
			WRITE8(ea+n, MMX(s).b[n]);
}

void i386_device::pentium_popcnt_r16_rm16()    // Opcode f3 0f b8
{
	UINT16 src;
	UINT8 modrm = FETCH();
	int n,count;

	if( modrm >= 0xc0 ) {
		src = LOAD_RM16(modrm);
	} else {
		UINT32 ea = GetEA(modrm,0);
		src = READ16(ea);
	}
	count=0;
	for (n=0;n < 16;n++) {
		count=count+(src & 1);
		src=src >> 1;
	}
	STORE_REG16(modrm, count);
	CYCLES(1); // TODO: correct cycle count
}

void i386_device::pentium_popcnt_r32_rm32()    // Opcode f3 0f b8
{
	UINT32 src;
	UINT8 modrm = FETCH();
	int n,count;

	if( modrm >= 0xc0 ) {
		src = LOAD_RM32(modrm);
	} else {
		UINT32 ea = GetEA(modrm,0);
		src = READ32(ea);
	}
	count=0;
	for (n=0;n < 32;n++) {
		count=count+(src & 1);
		src=src >> 1;
	}
	STORE_REG32(modrm, count);
	CYCLES(1); // TODO: correct cycle count
}

void i386_device::pentium_tzcnt_r16_rm16()
{
	// for CPUs that don't support TZCNT, fall back to BSF
	i386_bsf_r16_rm16();
	// TODO: actually implement TZCNT
}

void i386_device::pentium_tzcnt_r32_rm32()
{
	// for CPUs that don't support TZCNT, fall back to BSF
	i386_bsf_r32_rm32();
	// TODO: actually implement TZCNT
}

INLINE INT8 SaturatedSignedWordToSignedByte(INT16 word)
{
	if (word > 127)
		return 127;
	if (word < -128)
		return -128;
	return (INT8)word;
}

INLINE UINT8 SaturatedSignedWordToUnsignedByte(INT16 word)
{
	if (word > 255)
		return 255;
	if (word < 0)
		return 0;
	return (UINT8)word;
}

INLINE INT16 SaturatedSignedDwordToSignedWord(INT32 dword)
{
	if (dword > 32767)
		return 32767;
	if (dword < -32768)
		return -32768;
	return (INT16)dword;
}

void i386_device::mmx_group_0f71()  // Opcode 0f 71
{
	UINT8 modm = FETCH();
	UINT8 imm8 = FETCH();
	MMXPROLOG();
	if( modm >= 0xc0 ) {
		switch ( (modm & 0x38) >> 3 )
		{
			case 2: // psrlw
				MMX(modm & 7).w[0]=MMX(modm & 7).w[0] >> imm8;
				MMX(modm & 7).w[1]=MMX(modm & 7).w[1] >> imm8;
				MMX(modm & 7).w[2]=MMX(modm & 7).w[2] >> imm8;
				MMX(modm & 7).w[3]=MMX(modm & 7).w[3] >> imm8;
				break;
			case 4: // psraw
				MMX(modm & 7).s[0]=MMX(modm & 7).s[0] >> imm8;
				MMX(modm & 7).s[1]=MMX(modm & 7).s[1] >> imm8;
				MMX(modm & 7).s[2]=MMX(modm & 7).s[2] >> imm8;
				MMX(modm & 7).s[3]=MMX(modm & 7).s[3] >> imm8;
				break;
			case 6: // psllw
				MMX(modm & 7).w[0]=MMX(modm & 7).w[0] << imm8;
				MMX(modm & 7).w[1]=MMX(modm & 7).w[1] << imm8;
				MMX(modm & 7).w[2]=MMX(modm & 7).w[2] << imm8;
				MMX(modm & 7).w[3]=MMX(modm & 7).w[3] << imm8;
				break;
			default:
				report_invalid_modrm("mmx_group0f71", modm);
		}
	}
}

void i386_device::mmx_group_0f72()  // Opcode 0f 72
{
	UINT8 modm = FETCH();
	UINT8 imm8 = FETCH();
	MMXPROLOG();
	if( modm >= 0xc0 ) {
		switch ( (modm & 0x38) >> 3 )
		{
			case 2: // psrld
				MMX(modm & 7).d[0]=MMX(modm & 7).d[0] >> imm8;
				MMX(modm & 7).d[1]=MMX(modm & 7).d[1] >> imm8;
				break;
			case 4: // psrad
				MMX(modm & 7).i[0]=MMX(modm & 7).i[0] >> imm8;
				MMX(modm & 7).i[1]=MMX(modm & 7).i[1] >> imm8;
				break;
			case 6: // pslld
				MMX(modm & 7).d[0]=MMX(modm & 7).d[0] << imm8;
				MMX(modm & 7).d[1]=MMX(modm & 7).d[1] << imm8;
				break;
			default:
				report_invalid_modrm("mmx_group0f72", modm);
		}
	}
}

void i386_device::mmx_group_0f73()  // Opcode 0f 73
{
	UINT8 modm = FETCH();
	UINT8 imm8 = FETCH();
	MMXPROLOG();
	if( modm >= 0xc0 ) {
		switch ( (modm & 0x38) >> 3 )
		{
			case 2: // psrlq
				MMX(modm & 7).q=MMX(modm & 7).q >> imm8;
				break;
			case 6: // psllq
				MMX(modm & 7).q=MMX(modm & 7).q << imm8;
				break;
			default:
				report_invalid_modrm("mmx_group0f73", modm);
		}
	}
}

void i386_device::mmx_psrlw_r64_rm64()  // Opcode 0f d1
{
	MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		int count=(int)MMX(modrm & 7).q;
		MMX((modrm >> 3) & 0x7).w[0]=MMX((modrm >> 3) & 0x7).w[0] >> count;
		MMX((modrm >> 3) & 0x7).w[1]=MMX((modrm >> 3) & 0x7).w[1] >> count;
		MMX((modrm >> 3) & 0x7).w[2]=MMX((modrm >> 3) & 0x7).w[2] >> count;
		MMX((modrm >> 3) & 0x7).w[3]=MMX((modrm >> 3) & 0x7).w[3] >> count;
	} else {
		MMX_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READMMX(ea, src);
		int count=(int)src.q;
		MMX((modrm >> 3) & 0x7).w[0]=MMX((modrm >> 3) & 0x7).w[0] >> count;
		MMX((modrm >> 3) & 0x7).w[1]=MMX((modrm >> 3) & 0x7).w[1] >> count;
		MMX((modrm >> 3) & 0x7).w[2]=MMX((modrm >> 3) & 0x7).w[2] >> count;
		MMX((modrm >> 3) & 0x7).w[3]=MMX((modrm >> 3) & 0x7).w[3] >> count;
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::mmx_psrld_r64_rm64()  // Opcode 0f d2
{
	MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		int count=(int)MMX(modrm & 7).q;
		MMX((modrm >> 3) & 0x7).d[0]=MMX((modrm >> 3) & 0x7).d[0] >> count;
		MMX((modrm >> 3) & 0x7).d[1]=MMX((modrm >> 3) & 0x7).d[1] >> count;
	} else {
		MMX_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READMMX(ea, src);
		int count=(int)src.q;
		MMX((modrm >> 3) & 0x7).d[0]=MMX((modrm >> 3) & 0x7).d[0] >> count;
		MMX((modrm >> 3) & 0x7).d[1]=MMX((modrm >> 3) & 0x7).d[1] >> count;
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::mmx_psrlq_r64_rm64()  // Opcode 0f d3
{
	MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		int count=(int)MMX(modrm & 7).q;
		MMX((modrm >> 3) & 0x7).q=MMX((modrm >> 3) & 0x7).q >> count;
	} else {
		MMX_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READMMX(ea, src);
		int count=(int)src.q;
		MMX((modrm >> 3) & 0x7).q=MMX((modrm >> 3) & 0x7).q >> count;
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::mmx_paddq_r64_rm64()  // Opcode 0f d4
{
	MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		MMX((modrm >> 3) & 0x7).q=MMX((modrm >> 3) & 0x7).q+MMX(modrm & 7).q;
	} else {
		MMX_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READMMX(ea, src);
		MMX((modrm >> 3) & 0x7).q=MMX((modrm >> 3) & 0x7).q+src.q;
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::mmx_pmullw_r64_rm64()  // Opcode 0f d5
{
	MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		MMX((modrm >> 3) & 0x7).w[0]=(UINT32)((INT32)MMX((modrm >> 3) & 0x7).s[0]*(INT32)MMX(modrm & 7).s[0]) & 0xffff;
		MMX((modrm >> 3) & 0x7).w[1]=(UINT32)((INT32)MMX((modrm >> 3) & 0x7).s[1]*(INT32)MMX(modrm & 7).s[1]) & 0xffff;
		MMX((modrm >> 3) & 0x7).w[2]=(UINT32)((INT32)MMX((modrm >> 3) & 0x7).s[2]*(INT32)MMX(modrm & 7).s[2]) & 0xffff;
		MMX((modrm >> 3) & 0x7).w[3]=(UINT32)((INT32)MMX((modrm >> 3) & 0x7).s[3]*(INT32)MMX(modrm & 7).s[3]) & 0xffff;
	} else {
		MMX_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READMMX(ea, src);
		MMX((modrm >> 3) & 0x7).w[0]=(UINT32)((INT32)MMX((modrm >> 3) & 0x7).s[0]*(INT32)src.s[0]) & 0xffff;
		MMX((modrm >> 3) & 0x7).w[1]=(UINT32)((INT32)MMX((modrm >> 3) & 0x7).s[1]*(INT32)src.s[1]) & 0xffff;
		MMX((modrm >> 3) & 0x7).w[2]=(UINT32)((INT32)MMX((modrm >> 3) & 0x7).s[2]*(INT32)src.s[2]) & 0xffff;
		MMX((modrm >> 3) & 0x7).w[3]=(UINT32)((INT32)MMX((modrm >> 3) & 0x7).s[3]*(INT32)src.s[3]) & 0xffff;
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::mmx_psubusb_r64_rm64()  // Opcode 0f d8
{
	int n;
	MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		for (n=0;n < 8;n++)
			MMX((modrm >> 3) & 0x7).b[n]=MMX((modrm >> 3) & 0x7).b[n] < MMX(modrm & 7).b[n] ? 0 : MMX((modrm >> 3) & 0x7).b[n]-MMX(modrm & 7).b[n];
	} else {
		MMX_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READMMX(ea, src);
		for (n=0;n < 8;n++)
			MMX((modrm >> 3) & 0x7).b[n]=MMX((modrm >> 3) & 0x7).b[n] < src.b[n] ? 0 : MMX((modrm >> 3) & 0x7).b[n]-src.b[n];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::mmx_psubusw_r64_rm64()  // Opcode 0f d9
{
	int n;
	MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		for (n=0;n < 4;n++)
			MMX((modrm >> 3) & 0x7).w[n]=MMX((modrm >> 3) & 0x7).w[n] < MMX(modrm & 7).w[n] ? 0 : MMX((modrm >> 3) & 0x7).w[n]-MMX(modrm & 7).w[n];
	} else {
		MMX_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READMMX(ea, src);
		for (n=0;n < 4;n++)
			MMX((modrm >> 3) & 0x7).w[n]=MMX((modrm >> 3) & 0x7).w[n] < src.w[n] ? 0 : MMX((modrm >> 3) & 0x7).w[n]-src.w[n];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::mmx_pand_r64_rm64()  // Opcode 0f db
{
	MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		MMX((modrm >> 3) & 0x7).q=MMX((modrm >> 3) & 0x7).q & MMX(modrm & 7).q;
	} else {
		MMX_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READMMX(ea, src);
		MMX((modrm >> 3) & 0x7).q=MMX((modrm >> 3) & 0x7).q & src.q;
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::mmx_paddusb_r64_rm64()  // Opcode 0f dc
{
	int n;
	MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		for (n=0;n < 8;n++)
			MMX((modrm >> 3) & 0x7).b[n]=MMX((modrm >> 3) & 0x7).b[n] > (0xff-MMX(modrm & 7).b[n]) ? 0xff : MMX((modrm >> 3) & 0x7).b[n]+MMX(modrm & 7).b[n];
	} else {
		MMX_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READMMX(ea, src);
		for (n=0;n < 8;n++)
			MMX((modrm >> 3) & 0x7).b[n]=MMX((modrm >> 3) & 0x7).b[n] > (0xff-src.b[n]) ? 0xff : MMX((modrm >> 3) & 0x7).b[n]+src.b[n];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::mmx_paddusw_r64_rm64()  // Opcode 0f dd
{
	int n;
	MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		for (n=0;n < 4;n++)
			MMX((modrm >> 3) & 0x7).w[n]=MMX((modrm >> 3) & 0x7).w[n] > (0xffff-MMX(modrm & 7).w[n]) ? 0xffff : MMX((modrm >> 3) & 0x7).w[n]+MMX(modrm & 7).w[n];
	} else {
		MMX_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READMMX(ea, src);
		for (n=0;n < 4;n++)
			MMX((modrm >> 3) & 0x7).w[n]=MMX((modrm >> 3) & 0x7).w[n] > (0xffff-src.w[n]) ? 0xffff : MMX((modrm >> 3) & 0x7).w[n]+src.w[n];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::mmx_pandn_r64_rm64()  // Opcode 0f df
{
	MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		MMX((modrm >> 3) & 0x7).q=(~MMX((modrm >> 3) & 0x7).q) & MMX(modrm & 7).q;
	} else {
		MMX_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READMMX(ea, src);
		MMX((modrm >> 3) & 0x7).q=(~MMX((modrm >> 3) & 0x7).q) & src.q;
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::mmx_psraw_r64_rm64()  // Opcode 0f e1
{
	MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		int count=(int)MMX(modrm & 7).q;
		MMX((modrm >> 3) & 0x7).s[0]=MMX((modrm >> 3) & 0x7).s[0] >> count;
		MMX((modrm >> 3) & 0x7).s[1]=MMX((modrm >> 3) & 0x7).s[1] >> count;
		MMX((modrm >> 3) & 0x7).s[2]=MMX((modrm >> 3) & 0x7).s[2] >> count;
		MMX((modrm >> 3) & 0x7).s[3]=MMX((modrm >> 3) & 0x7).s[3] >> count;
	} else {
		MMX_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READMMX(ea, src);
		int count=(int)src.q;
		MMX((modrm >> 3) & 0x7).s[0]=MMX((modrm >> 3) & 0x7).s[0] >> count;
		MMX((modrm >> 3) & 0x7).s[1]=MMX((modrm >> 3) & 0x7).s[1] >> count;
		MMX((modrm >> 3) & 0x7).s[2]=MMX((modrm >> 3) & 0x7).s[2] >> count;
		MMX((modrm >> 3) & 0x7).s[3]=MMX((modrm >> 3) & 0x7).s[3] >> count;
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::mmx_psrad_r64_rm64()  // Opcode 0f e2
{
	MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		int count=(int)MMX(modrm & 7).q;
		MMX((modrm >> 3) & 0x7).i[0]=MMX((modrm >> 3) & 0x7).i[0] >> count;
		MMX((modrm >> 3) & 0x7).i[1]=MMX((modrm >> 3) & 0x7).i[1] >> count;
	} else {
		MMX_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READMMX(ea, src);
		int count=(int)src.q;
		MMX((modrm >> 3) & 0x7).i[0]=MMX((modrm >> 3) & 0x7).i[0] >> count;
		MMX((modrm >> 3) & 0x7).i[1]=MMX((modrm >> 3) & 0x7).i[1] >> count;
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::mmx_pmulhw_r64_rm64()  // Opcode 0f e5
{
	MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		MMX((modrm >> 3) & 0x7).w[0]=(UINT32)((INT32)MMX((modrm >> 3) & 0x7).s[0]*(INT32)MMX(modrm & 7).s[0]) >> 16;
		MMX((modrm >> 3) & 0x7).w[1]=(UINT32)((INT32)MMX((modrm >> 3) & 0x7).s[1]*(INT32)MMX(modrm & 7).s[1]) >> 16;
		MMX((modrm >> 3) & 0x7).w[2]=(UINT32)((INT32)MMX((modrm >> 3) & 0x7).s[2]*(INT32)MMX(modrm & 7).s[2]) >> 16;
		MMX((modrm >> 3) & 0x7).w[3]=(UINT32)((INT32)MMX((modrm >> 3) & 0x7).s[3]*(INT32)MMX(modrm & 7).s[3]) >> 16;
	} else {
		MMX_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READMMX(ea, src);
		MMX((modrm >> 3) & 0x7).w[0]=(UINT32)((INT32)MMX((modrm >> 3) & 0x7).s[0]*(INT32)src.s[0]) >> 16;
		MMX((modrm >> 3) & 0x7).w[1]=(UINT32)((INT32)MMX((modrm >> 3) & 0x7).s[1]*(INT32)src.s[1]) >> 16;
		MMX((modrm >> 3) & 0x7).w[2]=(UINT32)((INT32)MMX((modrm >> 3) & 0x7).s[2]*(INT32)src.s[2]) >> 16;
		MMX((modrm >> 3) & 0x7).w[3]=(UINT32)((INT32)MMX((modrm >> 3) & 0x7).s[3]*(INT32)src.s[3]) >> 16;
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::mmx_psubsb_r64_rm64()  // Opcode 0f e8
{
	int n;
	MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		for (n=0;n < 8;n++)
			MMX((modrm >> 3) & 0x7).c[n]=SaturatedSignedWordToSignedByte((INT16)MMX((modrm >> 3) & 0x7).c[n] - (INT16)MMX(modrm & 7).c[n]);
	} else {
		MMX_REG s;
		UINT32 ea = GetEA(modrm, 0);
		READMMX(ea, s);
		for (n=0;n < 8;n++)
			MMX((modrm >> 3) & 0x7).c[n]=SaturatedSignedWordToSignedByte((INT16)MMX((modrm >> 3) & 0x7).c[n] - (INT16)s.c[n]);
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::mmx_psubsw_r64_rm64()  // Opcode 0f e9
{
	int n;
	MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		for (n=0;n < 4;n++)
			MMX((modrm >> 3) & 0x7).s[n]=SaturatedSignedDwordToSignedWord((INT32)MMX((modrm >> 3) & 0x7).s[n] - (INT32)MMX(modrm & 7).s[n]);
	} else {
		MMX_REG s;
		UINT32 ea = GetEA(modrm, 0);
		READMMX(ea, s);
		for (n=0;n < 4;n++)
			MMX((modrm >> 3) & 0x7).s[n]=SaturatedSignedDwordToSignedWord((INT32)MMX((modrm >> 3) & 0x7).s[n] - (INT32)s.s[n]);
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::mmx_por_r64_rm64()  // Opcode 0f eb
{
	MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		MMX((modrm >> 3) & 0x7).q=MMX((modrm >> 3) & 0x7).q | MMX(modrm & 7).q;
	} else {
		MMX_REG s;
		UINT32 ea = GetEA(modrm, 0);
		READMMX(ea, s);
		MMX((modrm >> 3) & 0x7).q=MMX((modrm >> 3) & 0x7).q | s.q;
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::mmx_paddsb_r64_rm64()  // Opcode 0f ec
{
	int n;
	MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		for (n=0;n < 8;n++)
			MMX((modrm >> 3) & 0x7).c[n]=SaturatedSignedWordToSignedByte((INT16)MMX((modrm >> 3) & 0x7).c[n] + (INT16)MMX(modrm & 7).c[n]);
	} else {
		MMX_REG s;
		UINT32 ea = GetEA(modrm, 0);
		READMMX(ea, s);
		for (n=0;n < 8;n++)
			MMX((modrm >> 3) & 0x7).c[n]=SaturatedSignedWordToSignedByte((INT16)MMX((modrm >> 3) & 0x7).c[n] + (INT16)s.c[n]);
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::mmx_paddsw_r64_rm64()  // Opcode 0f ed
{
	int n;
	MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		for (n=0;n < 4;n++)
			MMX((modrm >> 3) & 0x7).s[n]=SaturatedSignedDwordToSignedWord((INT32)MMX((modrm >> 3) & 0x7).s[n] + (INT32)MMX(modrm & 7).s[n]);
	} else {
		MMX_REG s;
		UINT32 ea = GetEA(modrm, 0);
		READMMX(ea, s);
		for (n=0;n < 4;n++)
			MMX((modrm >> 3) & 0x7).s[n]=SaturatedSignedDwordToSignedWord((INT32)MMX((modrm >> 3) & 0x7).s[n] + (INT32)s.s[n]);
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::mmx_pxor_r64_rm64()  // Opcode 0f ef
{
	MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		MMX((modrm >> 3) & 0x7).q=MMX((modrm >> 3) & 0x7).q ^ MMX(modrm & 7).q;
	} else {
		MMX_REG s;
		UINT32 ea = GetEA(modrm, 0);
		READMMX(ea, s);
		MMX((modrm >> 3) & 0x7).q=MMX((modrm >> 3) & 0x7).q ^ s.q;
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::mmx_psllw_r64_rm64()  // Opcode 0f f1
{
	MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		int count=(int)MMX(modrm & 7).q;
		MMX((modrm >> 3) & 0x7).w[0]=MMX((modrm >> 3) & 0x7).w[0] << count;
		MMX((modrm >> 3) & 0x7).w[1]=MMX((modrm >> 3) & 0x7).w[1] << count;
		MMX((modrm >> 3) & 0x7).w[2]=MMX((modrm >> 3) & 0x7).w[2] << count;
		MMX((modrm >> 3) & 0x7).w[3]=MMX((modrm >> 3) & 0x7).w[3] << count;
	} else {
		MMX_REG s;
		UINT32 ea = GetEA(modrm, 0);
		READMMX(ea, s);
		int count=(int)s.q;
		MMX((modrm >> 3) & 0x7).w[0]=MMX((modrm >> 3) & 0x7).w[0] << count;
		MMX((modrm >> 3) & 0x7).w[1]=MMX((modrm >> 3) & 0x7).w[1] << count;
		MMX((modrm >> 3) & 0x7).w[2]=MMX((modrm >> 3) & 0x7).w[2] << count;
		MMX((modrm >> 3) & 0x7).w[3]=MMX((modrm >> 3) & 0x7).w[3] << count;
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::mmx_pslld_r64_rm64()  // Opcode 0f f2
{
	MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		int count=(int)MMX(modrm & 7).q;
		MMX((modrm >> 3) & 0x7).d[0]=MMX((modrm >> 3) & 0x7).d[0] << count;
		MMX((modrm >> 3) & 0x7).d[1]=MMX((modrm >> 3) & 0x7).d[1] << count;
	} else {
		MMX_REG s;
		UINT32 ea = GetEA(modrm, 0);
		READMMX(ea, s);
		int count=(int)s.q;
		MMX((modrm >> 3) & 0x7).d[0]=MMX((modrm >> 3) & 0x7).d[0] << count;
		MMX((modrm >> 3) & 0x7).d[1]=MMX((modrm >> 3) & 0x7).d[1] << count;
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::mmx_psllq_r64_rm64()  // Opcode 0f f3
{
	MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		int count=(int)MMX(modrm & 7).q;
		MMX((modrm >> 3) & 0x7).q=MMX((modrm >> 3) & 0x7).q << count;
	} else {
		MMX_REG s;
		UINT32 ea = GetEA(modrm, 0);
		READMMX(ea, s);
		int count=(int)s.q;
		MMX((modrm >> 3) & 0x7).q=MMX((modrm >> 3) & 0x7).q << count;
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::mmx_pmaddwd_r64_rm64()  // Opcode 0f f5
{
	MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		MMX((modrm >> 3) & 0x7).i[0]=(INT32)MMX((modrm >> 3) & 0x7).s[0]*(INT32)MMX(modrm & 7).s[0]+
										(INT32)MMX((modrm >> 3) & 0x7).s[1]*(INT32)MMX(modrm & 7).s[1];
		MMX((modrm >> 3) & 0x7).i[1]=(INT32)MMX((modrm >> 3) & 0x7).s[2]*(INT32)MMX(modrm & 7).s[2]+
										(INT32)MMX((modrm >> 3) & 0x7).s[3]*(INT32)MMX(modrm & 7).s[3];
	} else {
		MMX_REG s;
		UINT32 ea = GetEA(modrm, 0);
		READMMX(ea, s);
		MMX((modrm >> 3) & 0x7).i[0]=(INT32)MMX((modrm >> 3) & 0x7).s[0]*(INT32)s.s[0]+
										(INT32)MMX((modrm >> 3) & 0x7).s[1]*(INT32)s.s[1];
		MMX((modrm >> 3) & 0x7).i[1]=(INT32)MMX((modrm >> 3) & 0x7).s[2]*(INT32)s.s[2]+
										(INT32)MMX((modrm >> 3) & 0x7).s[3]*(INT32)s.s[3];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::mmx_psubb_r64_rm64()  // Opcode 0f f8
{
	int n;
	MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		for (n=0;n < 8;n++)
			MMX((modrm >> 3) & 0x7).b[n]=MMX((modrm >> 3) & 0x7).b[n] - MMX(modrm & 7).b[n];
	} else {
		MMX_REG s;
		UINT32 ea = GetEA(modrm, 0);
		READMMX(ea, s);
		for (n=0;n < 8;n++)
			MMX((modrm >> 3) & 0x7).b[n]=MMX((modrm >> 3) & 0x7).b[n] - s.b[n];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::mmx_psubw_r64_rm64()  // Opcode 0f f9
{
	int n;
	MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		for (n=0;n < 4;n++)
			MMX((modrm >> 3) & 0x7).w[n]=MMX((modrm >> 3) & 0x7).w[n] - MMX(modrm & 7).w[n];
	} else {
		MMX_REG s;
		UINT32 ea = GetEA(modrm, 0);
		READMMX(ea, s);
		for (n=0;n < 4;n++)
			MMX((modrm >> 3) & 0x7).w[n]=MMX((modrm >> 3) & 0x7).w[n] - s.w[n];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::mmx_psubd_r64_rm64()  // Opcode 0f fa
{
	int n;
	MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		for (n=0;n < 2;n++)
			MMX((modrm >> 3) & 0x7).d[n]=MMX((modrm >> 3) & 0x7).d[n] - MMX(modrm & 7).d[n];
	} else {
		MMX_REG s;
		UINT32 ea = GetEA(modrm, 0);
		READMMX(ea, s);
		for (n=0;n < 2;n++)
			MMX((modrm >> 3) & 0x7).d[n]=MMX((modrm >> 3) & 0x7).d[n] - s.d[n];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::mmx_paddb_r64_rm64()  // Opcode 0f fc
{
	int n;
	MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		for (n=0;n < 8;n++)
			MMX((modrm >> 3) & 0x7).b[n]=MMX((modrm >> 3) & 0x7).b[n] + MMX(modrm & 7).b[n];
	} else {
		MMX_REG s;
		UINT32 ea = GetEA(modrm, 0);
		READMMX(ea, s);
		for (n=0;n < 8;n++)
			MMX((modrm >> 3) & 0x7).b[n]=MMX((modrm >> 3) & 0x7).b[n] + s.b[n];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::mmx_paddw_r64_rm64()  // Opcode 0f fd
{
	int n;
	MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		for (n=0;n < 4;n++)
			MMX((modrm >> 3) & 0x7).w[n]=MMX((modrm >> 3) & 0x7).w[n] + MMX(modrm & 7).w[n];
	} else {
		MMX_REG s;
		UINT32 ea = GetEA(modrm, 0);
		READMMX(ea, s);
		for (n=0;n < 4;n++)
			MMX((modrm >> 3) & 0x7).w[n]=MMX((modrm >> 3) & 0x7).w[n] + s.w[n];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::mmx_paddd_r64_rm64()  // Opcode 0f fe
{
	int n;
	MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		for (n=0;n < 2;n++)
			MMX((modrm >> 3) & 0x7).d[n]=MMX((modrm >> 3) & 0x7).d[n] + MMX(modrm & 7).d[n];
	} else {
		MMX_REG s;
		UINT32 ea = GetEA(modrm, 0);
		READMMX(ea, s);
		for (n=0;n < 2;n++)
			MMX((modrm >> 3) & 0x7).d[n]=MMX((modrm >> 3) & 0x7).d[n] + s.d[n];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::mmx_emms() // Opcode 0f 77
{
	m_x87_tw = 0xffff; // tag word = 0xffff
	// TODO
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::mmx_movd_r64_rm32() // Opcode 0f 6e
{
	MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		MMX((modrm >> 3) & 0x7).d[0]=LOAD_RM32(modrm);
	} else {
		UINT32 ea = GetEA(modrm, 0);
		MMX((modrm >> 3) & 0x7).d[0]=READ32(ea);
	}
	MMX((modrm >> 3) & 0x7).d[1]=0;
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::mmx_movq_r64_rm64() // Opcode 0f 6f
{
	MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		MMX((modrm >> 3) & 0x7).l=MMX(modrm & 0x7).l;
	} else {
		UINT32 ea = GetEA(modrm, 0);
		READMMX(ea, MMX((modrm >> 3) & 0x7));
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::mmx_movd_rm32_r64() // Opcode 0f 7e
{
	MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		STORE_RM32(modrm, MMX((modrm >> 3) & 0x7).d[0]);
	} else {
		UINT32 ea = GetEA(modrm, 0);
		WRITE32(ea, MMX((modrm >> 3) & 0x7).d[0]);
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::mmx_movq_rm64_r64() // Opcode 0f 7f
{
	MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		MMX(modrm & 0x7)=MMX((modrm >> 3) & 0x7);
	} else {
		UINT32 ea = GetEA(modrm, 0);
		WRITEMMX(ea, MMX((modrm >> 3) & 0x7));
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::mmx_pcmpeqb_r64_rm64() // Opcode 0f 74
{
	int c;
	MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		int s,d;
		s=modrm & 0x7;
		d=(modrm >> 3) & 0x7;
		for (c=0;c <= 7;c++)
			MMX(d).b[c]=(MMX(d).b[c] == MMX(s).b[c]) ? 0xff : 0;
	} else {
		MMX_REG s;
		int d=(modrm >> 3) & 0x7;
		UINT32 ea = GetEA(modrm, 0);
		READMMX(ea, s);
		for (c=0;c <= 7;c++)
			MMX(d).b[c]=(MMX(d).b[c] == s.b[c]) ? 0xff : 0;
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::mmx_pcmpeqw_r64_rm64() // Opcode 0f 75
{
	MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		int s,d;
		s=modrm & 0x7;
		d=(modrm >> 3) & 0x7;
		MMX(d).w[0]=(MMX(d).w[0] == MMX(s).w[0]) ? 0xffff : 0;
		MMX(d).w[1]=(MMX(d).w[1] == MMX(s).w[1]) ? 0xffff : 0;
		MMX(d).w[2]=(MMX(d).w[2] == MMX(s).w[2]) ? 0xffff : 0;
		MMX(d).w[3]=(MMX(d).w[3] == MMX(s).w[3]) ? 0xffff : 0;
	} else {
		MMX_REG s;
		int d=(modrm >> 3) & 0x7;
		UINT32 ea = GetEA(modrm, 0);
		READMMX(ea, s);
		MMX(d).w[0]=(MMX(d).w[0] == s.w[0]) ? 0xffff : 0;
		MMX(d).w[1]=(MMX(d).w[1] == s.w[1]) ? 0xffff : 0;
		MMX(d).w[2]=(MMX(d).w[2] == s.w[2]) ? 0xffff : 0;
		MMX(d).w[3]=(MMX(d).w[3] == s.w[3]) ? 0xffff : 0;
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::mmx_pcmpeqd_r64_rm64() // Opcode 0f 76
{
	MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		int s,d;
		s=modrm & 0x7;
		d=(modrm >> 3) & 0x7;
		MMX(d).d[0]=(MMX(d).d[0] == MMX(s).d[0]) ? 0xffffffff : 0;
		MMX(d).d[1]=(MMX(d).d[1] == MMX(s).d[1]) ? 0xffffffff : 0;
	} else {
		MMX_REG s;
		int d=(modrm >> 3) & 0x7;
		UINT32 ea = GetEA(modrm, 0);
		READMMX(ea, s);
		MMX(d).d[0]=(MMX(d).d[0] == s.d[0]) ? 0xffffffff : 0;
		MMX(d).d[1]=(MMX(d).d[1] == s.d[1]) ? 0xffffffff : 0;
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::mmx_pshufw_r64_rm64_i8() // Opcode 0f 70
{
	MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		MMX_REG t;
		int s,d;
		UINT8 imm8 = FETCH();
		s=modrm & 0x7;
		d=(modrm >> 3) & 0x7;
		t.q=MMX(s).q;
		MMX(d).w[0]=t.w[imm8 & 3];
		MMX(d).w[1]=t.w[(imm8 >> 2) & 3];
		MMX(d).w[2]=t.w[(imm8 >> 4) & 3];
		MMX(d).w[3]=t.w[(imm8 >> 6) & 3];
	} else {
		MMX_REG s;
		int d=(modrm >> 3) & 0x7;
		UINT32 ea = GetEA(modrm, 0);
		UINT8 imm8 = FETCH();
		READMMX(ea, s);
		MMX(d).w[0]=s.w[imm8 & 3];
		MMX(d).w[1]=s.w[(imm8 >> 2) & 3];
		MMX(d).w[2]=s.w[(imm8 >> 4) & 3];
		MMX(d).w[3]=s.w[(imm8 >> 6) & 3];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::mmx_punpcklbw_r64_r64m32() // Opcode 0f 60
{
	MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		UINT32 t;
		int s,d;
		s=modrm & 0x7;
		d=(modrm >> 3) & 0x7;
		t=MMX(d).d[0];
		MMX(d).b[0]=t & 0xff;
		MMX(d).b[1]=MMX(s).b[0];
		MMX(d).b[2]=(t >> 8) & 0xff;
		MMX(d).b[3]=MMX(s).b[1];
		MMX(d).b[4]=(t >> 16) & 0xff;
		MMX(d).b[5]=MMX(s).b[2];
		MMX(d).b[6]=(t >> 24) & 0xff;
		MMX(d).b[7]=MMX(s).b[3];
	} else {
		UINT32 s,t;
		int d=(modrm >> 3) & 0x7;
		UINT32 ea = GetEA(modrm, 0);
		s = READ32(ea);
		t=MMX(d).d[0];
		MMX(d).b[0]=t & 0xff;
		MMX(d).b[1]=s & 0xff;
		MMX(d).b[2]=(t >> 8) & 0xff;
		MMX(d).b[3]=(s >> 8) & 0xff;
		MMX(d).b[4]=(t >> 16) & 0xff;
		MMX(d).b[5]=(s >> 16) & 0xff;
		MMX(d).b[6]=(t >> 24) & 0xff;
		MMX(d).b[7]=(s >> 24) & 0xff;
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::mmx_punpcklwd_r64_r64m32() // Opcode 0f 61
{
	MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		UINT16 t;
		int s,d;
		s=modrm & 0x7;
		d=(modrm >> 3) & 0x7;
		t=MMX(d).w[1];
		MMX(d).w[0]=MMX(d).w[0];
		MMX(d).w[1]=MMX(s).w[0];
		MMX(d).w[2]=t;
		MMX(d).w[3]=MMX(s).w[1];
	} else {
		UINT32 s;
		UINT16 t;
		int d=(modrm >> 3) & 0x7;
		UINT32 ea = GetEA(modrm, 0);
		s = READ32(ea);
		t=MMX(d).w[1];
		MMX(d).w[0]=MMX(d).w[0];
		MMX(d).w[1]=s & 0xffff;
		MMX(d).w[2]=t;
		MMX(d).w[3]=(s >> 16) & 0xffff;
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::mmx_punpckldq_r64_r64m32() // Opcode 0f 62
{
	MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		int s,d;
		s=modrm & 0x7;
		d=(modrm >> 3) & 0x7;
		MMX(d).d[0]=MMX(d).d[0];
		MMX(d).d[1]=MMX(s).d[0];
	} else {
		UINT32 s;
		int d=(modrm >> 3) & 0x7;
		UINT32 ea = GetEA(modrm, 0);
		s = READ32(ea);
		MMX(d).d[0]=MMX(d).d[0];
		MMX(d).d[1]=s;
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::mmx_packsswb_r64_rm64() // Opcode 0f 63
{
	MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		int s,d;
		s=modrm & 0x7;
		d=(modrm >> 3) & 0x7;
		MMX(d).c[0]=SaturatedSignedWordToSignedByte(MMX(d).s[0]);
		MMX(d).c[1]=SaturatedSignedWordToSignedByte(MMX(d).s[1]);
		MMX(d).c[2]=SaturatedSignedWordToSignedByte(MMX(d).s[2]);
		MMX(d).c[3]=SaturatedSignedWordToSignedByte(MMX(d).s[3]);
		MMX(d).c[4]=SaturatedSignedWordToSignedByte(MMX(s).s[0]);
		MMX(d).c[5]=SaturatedSignedWordToSignedByte(MMX(s).s[1]);
		MMX(d).c[6]=SaturatedSignedWordToSignedByte(MMX(s).s[2]);
		MMX(d).c[7]=SaturatedSignedWordToSignedByte(MMX(s).s[3]);
	} else {
		MMX_REG s;
		int d=(modrm >> 3) & 0x7;
		UINT32 ea = GetEA(modrm, 0);
		READMMX(ea, s);
		MMX(d).c[0]=SaturatedSignedWordToSignedByte(MMX(d).s[0]);
		MMX(d).c[1]=SaturatedSignedWordToSignedByte(MMX(d).s[1]);
		MMX(d).c[2]=SaturatedSignedWordToSignedByte(MMX(d).s[2]);
		MMX(d).c[3]=SaturatedSignedWordToSignedByte(MMX(d).s[3]);
		MMX(d).c[4]=SaturatedSignedWordToSignedByte(s.s[0]);
		MMX(d).c[5]=SaturatedSignedWordToSignedByte(s.s[1]);
		MMX(d).c[6]=SaturatedSignedWordToSignedByte(s.s[2]);
		MMX(d).c[7]=SaturatedSignedWordToSignedByte(s.s[3]);
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::mmx_pcmpgtb_r64_rm64() // Opcode 0f 64
{
	int c;
	MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		int s,d;
		s=modrm & 0x7;
		d=(modrm >> 3) & 0x7;
		for (c=0;c <= 7;c++)
			MMX(d).b[c]=(MMX(d).c[c] > MMX(s).c[c]) ? 0xff : 0;
	} else {
		MMX_REG s;
		int d=(modrm >> 3) & 0x7;
		UINT32 ea = GetEA(modrm, 0);
		READMMX(ea, s);
		for (c=0;c <= 7;c++)
			MMX(d).b[c]=(MMX(d).c[c] > s.c[c]) ? 0xff : 0;
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::mmx_pcmpgtw_r64_rm64() // Opcode 0f 65
{
	int c;
	MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		int s,d;
		s=modrm & 0x7;
		d=(modrm >> 3) & 0x7;
		for (c=0;c <= 3;c++)
			MMX(d).w[c]=(MMX(d).s[c] > MMX(s).s[c]) ? 0xffff : 0;
	} else {
		MMX_REG s;
		int d=(modrm >> 3) & 0x7;
		UINT32 ea = GetEA(modrm, 0);
		READMMX(ea, s);
		for (c=0;c <= 3;c++)
			MMX(d).w[c]=(MMX(d).s[c] > s.s[c]) ? 0xffff : 0;
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::mmx_pcmpgtd_r64_rm64() // Opcode 0f 66
{
	int c;
	MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		int s,d;
		s=modrm & 0x7;
		d=(modrm >> 3) & 0x7;
		for (c=0;c <= 1;c++)
			MMX(d).d[c]=(MMX(d).i[c] > MMX(s).i[c]) ? 0xffffffff : 0;
	} else {
		MMX_REG s;
		int d=(modrm >> 3) & 0x7;
		UINT32 ea = GetEA(modrm, 0);
		READMMX(ea, s);
		for (c=0;c <= 1;c++)
			MMX(d).d[c]=(MMX(d).i[c] > s.i[c]) ? 0xffffffff : 0;
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::mmx_packuswb_r64_rm64() // Opcode 0f 67
{
	MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		int s,d;
		s=modrm & 0x7;
		d=(modrm >> 3) & 0x7;
		MMX(d).b[0]=SaturatedSignedWordToUnsignedByte(MMX(d).s[0]);
		MMX(d).b[1]=SaturatedSignedWordToUnsignedByte(MMX(d).s[1]);
		MMX(d).b[2]=SaturatedSignedWordToUnsignedByte(MMX(d).s[2]);
		MMX(d).b[3]=SaturatedSignedWordToUnsignedByte(MMX(d).s[3]);
		MMX(d).b[4]=SaturatedSignedWordToUnsignedByte(MMX(s).s[0]);
		MMX(d).b[5]=SaturatedSignedWordToUnsignedByte(MMX(s).s[1]);
		MMX(d).b[6]=SaturatedSignedWordToUnsignedByte(MMX(s).s[2]);
		MMX(d).b[7]=SaturatedSignedWordToUnsignedByte(MMX(s).s[3]);
	} else {
		MMX_REG s;
		int d=(modrm >> 3) & 0x7;
		UINT32 ea = GetEA(modrm, 0);
		READMMX(ea, s);
		MMX(d).b[0]=SaturatedSignedWordToUnsignedByte(MMX(d).s[0]);
		MMX(d).b[1]=SaturatedSignedWordToUnsignedByte(MMX(d).s[1]);
		MMX(d).b[2]=SaturatedSignedWordToUnsignedByte(MMX(d).s[2]);
		MMX(d).b[3]=SaturatedSignedWordToUnsignedByte(MMX(d).s[3]);
		MMX(d).b[4]=SaturatedSignedWordToUnsignedByte(s.s[0]);
		MMX(d).b[5]=SaturatedSignedWordToUnsignedByte(s.s[1]);
		MMX(d).b[6]=SaturatedSignedWordToUnsignedByte(s.s[2]);
		MMX(d).b[7]=SaturatedSignedWordToUnsignedByte(s.s[3]);
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::mmx_punpckhbw_r64_rm64() // Opcode 0f 68
{
	MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		int s,d;
		s=modrm & 0x7;
		d=(modrm >> 3) & 0x7;
		MMX(d).b[0]=MMX(d).b[4];
		MMX(d).b[1]=MMX(s).b[4];
		MMX(d).b[2]=MMX(d).b[5];
		MMX(d).b[3]=MMX(s).b[5];
		MMX(d).b[4]=MMX(d).b[6];
		MMX(d).b[5]=MMX(s).b[6];
		MMX(d).b[6]=MMX(d).b[7];
		MMX(d).b[7]=MMX(s).b[7];
	} else {
		MMX_REG s;
		int d=(modrm >> 3) & 0x7;
		UINT32 ea = GetEA(modrm, 0);
		READMMX(ea, s);
		MMX(d).b[0]=MMX(d).b[4];
		MMX(d).b[1]=s.b[4];
		MMX(d).b[2]=MMX(d).b[5];
		MMX(d).b[3]=s.b[5];
		MMX(d).b[4]=MMX(d).b[6];
		MMX(d).b[5]=s.b[6];
		MMX(d).b[6]=MMX(d).b[7];
		MMX(d).b[7]=s.b[7];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::mmx_punpckhwd_r64_rm64() // Opcode 0f 69
{
	MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		int s,d;
		s=modrm & 0x7;
		d=(modrm >> 3) & 0x7;
		MMX(d).w[0]=MMX(d).w[2];
		MMX(d).w[1]=MMX(s).w[2];
		MMX(d).w[2]=MMX(d).w[3];
		MMX(d).w[3]=MMX(s).w[3];
	} else {
		MMX_REG s;
		int d=(modrm >> 3) & 0x7;
		UINT32 ea = GetEA(modrm, 0);
		READMMX(ea, s);
		MMX(d).w[0]=MMX(d).w[2];
		MMX(d).w[1]=s.w[2];
		MMX(d).w[2]=MMX(d).w[3];
		MMX(d).w[3]=s.w[3];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::mmx_punpckhdq_r64_rm64() // Opcode 0f 6a
{
	MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		int s,d;
		s=modrm & 0x7;
		d=(modrm >> 3) & 0x7;
		MMX(d).d[0]=MMX(d).d[1];
		MMX(d).d[1]=MMX(s).d[1];
	} else {
		MMX_REG s;
		int d=(modrm >> 3) & 0x7;
		UINT32 ea = GetEA(modrm, 0);
		READMMX(ea, s);
		MMX(d).d[0]=MMX(d).d[1];
		MMX(d).d[1]=s.d[1];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::mmx_packssdw_r64_rm64() // Opcode 0f 6b
{
	MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		int s,d;
		s=modrm & 0x7;
		d=(modrm >> 3) & 0x7;
		MMX(d).s[0]=SaturatedSignedDwordToSignedWord(MMX(d).i[0]);
		MMX(d).s[1]=SaturatedSignedDwordToSignedWord(MMX(d).i[1]);
		MMX(d).s[2]=SaturatedSignedDwordToSignedWord(MMX(s).i[0]);
		MMX(d).s[3]=SaturatedSignedDwordToSignedWord(MMX(s).i[1]);
	} else {
		MMX_REG s;
		int d=(modrm >> 3) & 0x7;
		UINT32 ea = GetEA(modrm, 0);
		READMMX(ea, s);
		MMX(d).s[0]=SaturatedSignedDwordToSignedWord(MMX(d).i[0]);
		MMX(d).s[1]=SaturatedSignedDwordToSignedWord(MMX(d).i[1]);
		MMX(d).s[2]=SaturatedSignedDwordToSignedWord(s.i[0]);
		MMX(d).s[3]=SaturatedSignedDwordToSignedWord(s.i[1]);
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_sse_group0fae()  // Opcode 0f ae
{
	UINT8 modm = FETCH();
	if( modm == 0xf8 ) {
		logerror("Unemulated SFENCE opcode called\n");
		CYCLES(1); // sfence instruction
	} else if( modm == 0xf0 ) {
		CYCLES(1); // mfence instruction
	} else if( modm == 0xe8 ) {
		CYCLES(1); // lfence instruction
	} else if( modm < 0xc0 ) {
		UINT32 ea;
		switch ( (modm & 0x38) >> 3 )
		{
			case 2: // ldmxcsr m32
				ea = GetEA(modm, 0);
				m_mxcsr = READ32(ea);
				break;
			case 3: // stmxcsr m32
				ea = GetEA(modm, 0);
				WRITE32(ea, m_mxcsr);
				break;
			case 7: // clflush m8
				GetNonTranslatedEA(modm, NULL);
				break;
			default:
				report_invalid_modrm("sse_group0fae", modm);
		}
	} else {
		report_invalid_modrm("sse_group0fae", modm);
	}
}

void i386_device::sse_cvttps2dq_r128_rm128() // Opcode f3 0f 5b
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).i[0]=(INT32)XMM(modrm & 0x7).f[0];
		XMM((modrm >> 3) & 0x7).i[1]=(INT32)XMM(modrm & 0x7).f[1];
		XMM((modrm >> 3) & 0x7).i[2]=(INT32)XMM(modrm & 0x7).f[2];
		XMM((modrm >> 3) & 0x7).i[3]=(INT32)XMM(modrm & 0x7).f[3];
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, src);
		XMM((modrm >> 3) & 0x7).i[0]=(INT32)src.f[0];
		XMM((modrm >> 3) & 0x7).i[1]=(INT32)src.f[1];
		XMM((modrm >> 3) & 0x7).i[2]=(INT32)src.f[2];
		XMM((modrm >> 3) & 0x7).i[3]=(INT32)src.f[3];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_cvtss2sd_r128_r128m32() // Opcode f3 0f 5a
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).f64[0] = XMM(modrm & 0x7).f[0];
	} else {
		XMM_REG s;
		UINT32 ea = GetEA(modrm, 0);
		s.d[0] = READ32(ea);
		XMM((modrm >> 3) & 0x7).f64[0] = s.f[0];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_cvttss2si_r32_r128m32() // Opcode f3 0f 2c
{
	INT32 src;
	UINT8 modrm = FETCH(); // get mordm byte
	if( modrm >= 0xc0 ) { // if bits 7-6 are 11 the source is a xmm register (low doubleword)
		src = (INT32)XMM(modrm & 0x7).f[0^NATIVE_ENDIAN_VALUE_LE_BE(0,1)];
	} else { // otherwise is a memory address
		XMM_REG t;
		UINT32 ea = GetEA(modrm, 0);
		t.d[0] = READ32(ea);
		src = (INT32)t.f[0];
	}
	STORE_REG32(modrm, (UINT32)src);
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_cvtss2si_r32_r128m32() // Opcode f3 0f 2d
{
	INT32 src;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = (INT32)XMM(modrm & 0x7).f[0];
	} else {
		XMM_REG t;
		UINT32 ea = GetEA(modrm, 0);
		t.d[0] = READ32(ea);
		src = (INT32)t.f[0];
	}
	STORE_REG32(modrm, (UINT32)src);
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_cvtsi2ss_r128_rm32() // Opcode f3 0f 2a
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).f[0] = (INT32)LOAD_RM32(modrm);
	} else {
		UINT32 ea = GetEA(modrm, 0);
		XMM((modrm >> 3) & 0x7).f[0] = (INT32)READ32(ea);
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_cvtpi2ps_r128_rm64() // Opcode 0f 2a
{
	UINT8 modrm = FETCH();
	MMXPROLOG();
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).f[0] = MMX(modrm & 0x7).i[0];
		XMM((modrm >> 3) & 0x7).f[1] = MMX(modrm & 0x7).i[1];
	} else {
		MMX_REG r;
		UINT32 ea = GetEA(modrm, 0);
		READMMX(ea, r);
		XMM((modrm >> 3) & 0x7).f[0] = r.i[0];
		XMM((modrm >> 3) & 0x7).f[1] = r.i[1];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_cvttps2pi_r64_r128m64() // Opcode 0f 2c
{
	UINT8 modrm = FETCH();
	MMXPROLOG();
	if( modrm >= 0xc0 ) {
		MMX((modrm >> 3) & 0x7).i[0] = XMM(modrm & 0x7).f[0];
		MMX((modrm >> 3) & 0x7).i[1] = XMM(modrm & 0x7).f[1];
	} else {
		XMM_REG r;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, r);
		XMM((modrm >> 3) & 0x7).i[0] = r.f[0];
		XMM((modrm >> 3) & 0x7).i[1] = r.f[1];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_cvtps2pi_r64_r128m64() // Opcode 0f 2d
{
	UINT8 modrm = FETCH();
	MMXPROLOG();
	if( modrm >= 0xc0 ) {
		MMX((modrm >> 3) & 0x7).i[0] = XMM(modrm & 0x7).f[0];
		MMX((modrm >> 3) & 0x7).i[1] = XMM(modrm & 0x7).f[1];
	} else {
		XMM_REG r;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, r);
		XMM((modrm >> 3) & 0x7).i[0] = r.f[0];
		XMM((modrm >> 3) & 0x7).i[1] = r.f[1];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_cvtps2pd_r128_r128m64() // Opcode 0f 5a
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).f64[0] = (double)XMM(modrm & 0x7).f[0];
		XMM((modrm >> 3) & 0x7).f64[1] = (double)XMM(modrm & 0x7).f[1];
	} else {
		MMX_REG r;
		UINT32 ea = GetEA(modrm, 0);
		READMMX(ea, r);
		XMM((modrm >> 3) & 0x7).f64[0] = (double)r.f[0];
		XMM((modrm >> 3) & 0x7).f64[1] = (double)r.f[1];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_cvtdq2ps_r128_rm128() // Opcode 0f 5b
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).f[0] = (float)XMM(modrm & 0x7).i[0];
		XMM((modrm >> 3) & 0x7).f[1] = (float)XMM(modrm & 0x7).i[1];
		XMM((modrm >> 3) & 0x7).f[2] = (float)XMM(modrm & 0x7).i[2];
		XMM((modrm >> 3) & 0x7).f[3] = (float)XMM(modrm & 0x7).i[3];
	} else {
		XMM_REG r;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, r);
		XMM((modrm >> 3) & 0x7).f[0] = (float)r.i[0];
		XMM((modrm >> 3) & 0x7).f[1] = (float)r.i[1];
		XMM((modrm >> 3) & 0x7).f[2] = (float)r.i[2];
		XMM((modrm >> 3) & 0x7).f[3] = (float)r.i[3];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_cvtdq2pd_r128_r128m64() // Opcode f3 0f e6
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).f64[0] = (double)XMM(modrm & 0x7).i[0];
		XMM((modrm >> 3) & 0x7).f64[1] = (double)XMM(modrm & 0x7).i[1];
	} else {
		MMX_REG s;
		UINT32 ea = GetEA(modrm, 0);
		READMMX(ea, s);
		XMM((modrm >> 3) & 0x7).f64[0] = (double)s.i[0];
		XMM((modrm >> 3) & 0x7).f64[1] = (double)s.i[1];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_movss_r128_rm128() // Opcode f3 0f 10
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).d[0] = XMM(modrm & 0x7).d[0];
	} else {
		UINT32 ea = GetEA(modrm, 0);
		XMM((modrm >> 3) & 0x7).d[0] = READ32(ea);
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_movss_rm128_r128() // Opcode f3 0f 11
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM(modrm & 0x7).d[0] = XMM((modrm >> 3) & 0x7).d[0];
	} else {
		UINT32 ea = GetEA(modrm, 0);
		WRITE32(ea, XMM((modrm >> 3) & 0x7).d[0]);
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_movsldup_r128_rm128() // Opcode f3 0f 12
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).d[0] = XMM(modrm & 0x7).d[0];
		XMM((modrm >> 3) & 0x7).d[1] = XMM(modrm & 0x7).d[0];
		XMM((modrm >> 3) & 0x7).d[2] = XMM(modrm & 0x7).d[2];
		XMM((modrm >> 3) & 0x7).d[3] = XMM(modrm & 0x7).d[2];
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, src);
		XMM((modrm >> 3) & 0x7).d[0] = src.d[0];
		XMM((modrm >> 3) & 0x7).d[1] = src.d[0];
		XMM((modrm >> 3) & 0x7).d[2] = src.d[2];
		XMM((modrm >> 3) & 0x7).d[3] = src.d[2];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_movshdup_r128_rm128() // Opcode f3 0f 16
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).d[0] = XMM(modrm & 0x7).d[1];
		XMM((modrm >> 3) & 0x7).d[1] = XMM(modrm & 0x7).d[1];
		XMM((modrm >> 3) & 0x7).d[2] = XMM(modrm & 0x7).d[3];
		XMM((modrm >> 3) & 0x7).d[3] = XMM(modrm & 0x7).d[3];
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, src);
		XMM((modrm >> 3) & 0x7).d[0] = src.d[1];
		XMM((modrm >> 3) & 0x7).d[1] = src.d[1];
		XMM((modrm >> 3) & 0x7).d[2] = src.d[3];
		XMM((modrm >> 3) & 0x7).d[3] = src.d[3];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_movaps_r128_rm128() // Opcode 0f 28
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7) = XMM(modrm & 0x7);
	} else {
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, XMM((modrm >> 3) & 0x7));
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_movaps_rm128_r128() // Opcode 0f 29
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM(modrm & 0x7) = XMM((modrm >> 3) & 0x7);
	} else {
		UINT32 ea = GetEA(modrm, 0);
		WRITEXMM(ea, XMM((modrm >> 3) & 0x7));
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_movups_r128_rm128() // Opcode 0f 10
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7) = XMM(modrm & 0x7);
	} else {
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, XMM((modrm >> 3) & 0x7)); // address does not need to be 16-byte aligned
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_movups_rm128_r128() // Opcode 0f 11
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM(modrm & 0x7) = XMM((modrm >> 3) & 0x7);
	} else {
		UINT32 ea = GetEA(modrm, 0);
		WRITEXMM(ea, XMM((modrm >> 3) & 0x7)); // address does not need to be 16-byte aligned
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_movlps_r128_m64() // Opcode 0f 12
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		// unsupported by cpu
		CYCLES(1);     // TODO: correct cycle count
	} else {
		UINT32 ea = GetEA(modrm, 0);
		READXMM_LO64(ea, XMM((modrm >> 3) & 0x7));
		CYCLES(1);     // TODO: correct cycle count
	}
}

void i386_device::sse_movlps_m64_r128() // Opcode 0f 13
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		// unsupported by cpu
		CYCLES(1);     // TODO: correct cycle count
	} else {
		UINT32 ea = GetEA(modrm, 0);
		WRITEXMM_LO64(ea, XMM((modrm >> 3) & 0x7));
		CYCLES(1);     // TODO: correct cycle count
	}
}

void i386_device::sse_movhps_r128_m64() // Opcode 0f 16
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		// unsupported by cpu
		CYCLES(1);     // TODO: correct cycle count
	} else {
		UINT32 ea = GetEA(modrm, 0);
		READXMM_HI64(ea, XMM((modrm >> 3) & 0x7));
		CYCLES(1);     // TODO: correct cycle count
	}
}

void i386_device::sse_movhps_m64_r128() // Opcode 0f 17
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		// unsupported by cpu
		CYCLES(1);     // TODO: correct cycle count
	} else {
		UINT32 ea = GetEA(modrm, 0);
		WRITEXMM_HI64(ea, XMM((modrm >> 3) & 0x7));
		CYCLES(1);     // TODO: correct cycle count
	}
}

void i386_device::sse_movntps_m128_r128() // Opcode 0f 2b
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		// unsupported by cpu
		CYCLES(1);     // TODO: correct cycle count
	} else {
		// since cache is not implemented
		UINT32 ea = GetEA(modrm, 0);
		WRITEXMM(ea, XMM((modrm >> 3) & 0x7));
		CYCLES(1);     // TODO: correct cycle count
	}
}

void i386_device::sse_movmskps_r16_r128() // Opcode 0f 50
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		int b;
		b=(XMM(modrm & 0x7).d[0] >> 31) & 1;
		b=b | ((XMM(modrm & 0x7).d[1] >> 30) & 2);
		b=b | ((XMM(modrm & 0x7).d[2] >> 29) & 4);
		b=b | ((XMM(modrm & 0x7).d[3] >> 28) & 8);
		STORE_REG16(modrm, b);
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_movmskps_r32_r128() // Opcode 0f 50
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		int b;
		b=(XMM(modrm & 0x7).d[0] >> 31) & 1;
		b=b | ((XMM(modrm & 0x7).d[1] >> 30) & 2);
		b=b | ((XMM(modrm & 0x7).d[2] >> 29) & 4);
		b=b | ((XMM(modrm & 0x7).d[3] >> 28) & 8);
		STORE_REG32(modrm, b);
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_movq2dq_r128_r64() // Opcode f3 0f d6
{
	MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).q[0] = MMX(modrm & 7).q;
		XMM((modrm >> 3) & 0x7).q[1] = 0;
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_movdqu_r128_rm128() // Opcode f3 0f 6f
{
	MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).q[0] = XMM(modrm & 0x7).q[0];
		XMM((modrm >> 3) & 0x7).q[1] = XMM(modrm & 0x7).q[1];
	} else {
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, XMM((modrm >> 3) & 0x7));
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_movdqu_rm128_r128() // Opcode f3 0f 7f
{
	MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM(modrm & 0x7).q[0] = XMM((modrm >> 3) & 0x7).q[0];
		XMM(modrm & 0x7).q[1] = XMM((modrm >> 3) & 0x7).q[1];
	} else {
		UINT32 ea = GetEA(modrm, 0);
		WRITEXMM(ea, XMM((modrm >> 3) & 0x7));
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_movq_r128_r128m64() // Opcode f3 0f 7e
{
	MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).q[0] = XMM(modrm & 0x7).q[0];
		XMM((modrm >> 3) & 0x7).q[1] = 0;
	} else {
		UINT32 ea = GetEA(modrm, 0);
		XMM((modrm >> 3) & 0x7).q[0] = READ64(ea);
		XMM((modrm >> 3) & 0x7).q[1] = 0;
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_pmovmskb_r16_r64() // Opcode 0f d7
{
	//MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		int b;
		b=(MMX(modrm & 0x7).b[0] >> 7) & 1;
		b=b | ((MMX(modrm & 0x7).b[1] >> 6) & 2);
		b=b | ((MMX(modrm & 0x7).b[2] >> 5) & 4);
		b=b | ((MMX(modrm & 0x7).b[3] >> 4) & 8);
		b=b | ((MMX(modrm & 0x7).b[4] >> 3) & 16);
		b=b | ((MMX(modrm & 0x7).b[5] >> 2) & 32);
		b=b | ((MMX(modrm & 0x7).b[6] >> 1) & 64);
		b=b | ((MMX(modrm & 0x7).b[7] >> 0) & 128);
		STORE_REG16(modrm, b);
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_pmovmskb_r32_r64() // Opcode 0f d7
{
	//MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		int b;
		b=(MMX(modrm & 0x7).b[0] >> 7) & 1;
		b=b | ((MMX(modrm & 0x7).b[1] >> 6) & 2);
		b=b | ((MMX(modrm & 0x7).b[2] >> 5) & 4);
		b=b | ((MMX(modrm & 0x7).b[3] >> 4) & 8);
		b=b | ((MMX(modrm & 0x7).b[4] >> 3) & 16);
		b=b | ((MMX(modrm & 0x7).b[5] >> 2) & 32);
		b=b | ((MMX(modrm & 0x7).b[6] >> 1) & 64);
		b=b | ((MMX(modrm & 0x7).b[7] >> 0) & 128);
		STORE_REG32(modrm, b);
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_xorps() // Opcode 0f 57
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).d[0] = XMM((modrm >> 3) & 0x7).d[0] ^ XMM(modrm & 0x7).d[0];
		XMM((modrm >> 3) & 0x7).d[1] = XMM((modrm >> 3) & 0x7).d[1] ^ XMM(modrm & 0x7).d[1];
		XMM((modrm >> 3) & 0x7).d[2] = XMM((modrm >> 3) & 0x7).d[2] ^ XMM(modrm & 0x7).d[2];
		XMM((modrm >> 3) & 0x7).d[3] = XMM((modrm >> 3) & 0x7).d[3] ^ XMM(modrm & 0x7).d[3];
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, src);
		XMM((modrm >> 3) & 0x7).d[0] = XMM((modrm >> 3) & 0x7).d[0] ^ src.d[0];
		XMM((modrm >> 3) & 0x7).d[1] = XMM((modrm >> 3) & 0x7).d[1] ^ src.d[1];
		XMM((modrm >> 3) & 0x7).d[2] = XMM((modrm >> 3) & 0x7).d[2] ^ src.d[2];
		XMM((modrm >> 3) & 0x7).d[3] = XMM((modrm >> 3) & 0x7).d[3] ^ src.d[3];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_addps() // Opcode 0f 58
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).f[0] = XMM((modrm >> 3) & 0x7).f[0] + XMM(modrm & 0x7).f[0];
		XMM((modrm >> 3) & 0x7).f[1] = XMM((modrm >> 3) & 0x7).f[1] + XMM(modrm & 0x7).f[1];
		XMM((modrm >> 3) & 0x7).f[2] = XMM((modrm >> 3) & 0x7).f[2] + XMM(modrm & 0x7).f[2];
		XMM((modrm >> 3) & 0x7).f[3] = XMM((modrm >> 3) & 0x7).f[3] + XMM(modrm & 0x7).f[3];
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, src);
		XMM((modrm >> 3) & 0x7).f[0] = XMM((modrm >> 3) & 0x7).f[0] + src.f[0];
		XMM((modrm >> 3) & 0x7).f[1] = XMM((modrm >> 3) & 0x7).f[1] + src.f[1];
		XMM((modrm >> 3) & 0x7).f[2] = XMM((modrm >> 3) & 0x7).f[2] + src.f[2];
		XMM((modrm >> 3) & 0x7).f[3] = XMM((modrm >> 3) & 0x7).f[3] + src.f[3];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_sqrtps_r128_rm128() // Opcode 0f 51
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).f[0] = sqrt(XMM(modrm & 0x7).f[0]);
		XMM((modrm >> 3) & 0x7).f[1] = sqrt(XMM(modrm & 0x7).f[1]);
		XMM((modrm >> 3) & 0x7).f[2] = sqrt(XMM(modrm & 0x7).f[2]);
		XMM((modrm >> 3) & 0x7).f[3] = sqrt(XMM(modrm & 0x7).f[3]);
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, src);
		XMM((modrm >> 3) & 0x7).f[0] = sqrt(src.f[0]);
		XMM((modrm >> 3) & 0x7).f[1] = sqrt(src.f[1]);
		XMM((modrm >> 3) & 0x7).f[2] = sqrt(src.f[2]);
		XMM((modrm >> 3) & 0x7).f[3] = sqrt(src.f[3]);
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_rsqrtps_r128_rm128() // Opcode 0f 52
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).f[0] = 1.0 / sqrt(XMM(modrm & 0x7).f[0]);
		XMM((modrm >> 3) & 0x7).f[1] = 1.0 / sqrt(XMM(modrm & 0x7).f[1]);
		XMM((modrm >> 3) & 0x7).f[2] = 1.0 / sqrt(XMM(modrm & 0x7).f[2]);
		XMM((modrm >> 3) & 0x7).f[3] = 1.0 / sqrt(XMM(modrm & 0x7).f[3]);
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, src);
		XMM((modrm >> 3) & 0x7).f[0] = 1.0 / sqrt(src.f[0]);
		XMM((modrm >> 3) & 0x7).f[1] = 1.0 / sqrt(src.f[1]);
		XMM((modrm >> 3) & 0x7).f[2] = 1.0 / sqrt(src.f[2]);
		XMM((modrm >> 3) & 0x7).f[3] = 1.0 / sqrt(src.f[3]);
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_rcpps_r128_rm128() // Opcode 0f 53
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).f[0] = 1.0 / XMM(modrm & 0x7).f[0];
		XMM((modrm >> 3) & 0x7).f[1] = 1.0 / XMM(modrm & 0x7).f[1];
		XMM((modrm >> 3) & 0x7).f[2] = 1.0 / XMM(modrm & 0x7).f[2];
		XMM((modrm >> 3) & 0x7).f[3] = 1.0 / XMM(modrm & 0x7).f[3];
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, src);
		XMM((modrm >> 3) & 0x7).f[0] = 1.0 / src.f[0];
		XMM((modrm >> 3) & 0x7).f[1] = 1.0 / src.f[1];
		XMM((modrm >> 3) & 0x7).f[2] = 1.0 / src.f[2];
		XMM((modrm >> 3) & 0x7).f[3] = 1.0 / src.f[3];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_andps_r128_rm128() // Opcode 0f 54
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).q[0] = XMM((modrm >> 3) & 0x7).q[0] & XMM(modrm & 0x7).q[0];
		XMM((modrm >> 3) & 0x7).q[1] = XMM((modrm >> 3) & 0x7).q[1] & XMM(modrm & 0x7).q[1];
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, src);
		XMM((modrm >> 3) & 0x7).q[0] = XMM((modrm >> 3) & 0x7).q[0] & src.q[0];
		XMM((modrm >> 3) & 0x7).q[1] = XMM((modrm >> 3) & 0x7).q[1] & src.q[1];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_andnps_r128_rm128() // Opcode 0f 55
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).q[0] = ~(XMM((modrm >> 3) & 0x7).q[0]) & XMM(modrm & 0x7).q[0];
		XMM((modrm >> 3) & 0x7).q[1] = ~(XMM((modrm >> 3) & 0x7).q[1]) & XMM(modrm & 0x7).q[1];
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, src);
		XMM((modrm >> 3) & 0x7).q[0] = ~(XMM((modrm >> 3) & 0x7).q[0]) & src.q[0];
		XMM((modrm >> 3) & 0x7).q[1] = ~(XMM((modrm >> 3) & 0x7).q[1]) & src.q[1];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_orps_r128_rm128() // Opcode 0f 56
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).q[0] = XMM((modrm >> 3) & 0x7).q[0] | XMM(modrm & 0x7).q[0];
		XMM((modrm >> 3) & 0x7).q[1] = XMM((modrm >> 3) & 0x7).q[1] | XMM(modrm & 0x7).q[1];
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, src);
		XMM((modrm >> 3) & 0x7).q[0] = XMM((modrm >> 3) & 0x7).q[0] | src.q[0];
		XMM((modrm >> 3) & 0x7).q[1] = XMM((modrm >> 3) & 0x7).q[1] | src.q[1];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_mulps() // Opcode 0f 59 ????
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).f[0] = XMM((modrm >> 3) & 0x7).f[0] * XMM(modrm & 0x7).f[0];
		XMM((modrm >> 3) & 0x7).f[1] = XMM((modrm >> 3) & 0x7).f[1] * XMM(modrm & 0x7).f[1];
		XMM((modrm >> 3) & 0x7).f[2] = XMM((modrm >> 3) & 0x7).f[2] * XMM(modrm & 0x7).f[2];
		XMM((modrm >> 3) & 0x7).f[3] = XMM((modrm >> 3) & 0x7).f[3] * XMM(modrm & 0x7).f[3];
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, src);
		XMM((modrm >> 3) & 0x7).f[0] = XMM((modrm >> 3) & 0x7).f[0] * src.f[0];
		XMM((modrm >> 3) & 0x7).f[1] = XMM((modrm >> 3) & 0x7).f[1] * src.f[1];
		XMM((modrm >> 3) & 0x7).f[2] = XMM((modrm >> 3) & 0x7).f[2] * src.f[2];
		XMM((modrm >> 3) & 0x7).f[3] = XMM((modrm >> 3) & 0x7).f[3] * src.f[3];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_subps() // Opcode 0f 5c
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).f[0] = XMM((modrm >> 3) & 0x7).f[0] - XMM(modrm & 0x7).f[0];
		XMM((modrm >> 3) & 0x7).f[1] = XMM((modrm >> 3) & 0x7).f[1] - XMM(modrm & 0x7).f[1];
		XMM((modrm >> 3) & 0x7).f[2] = XMM((modrm >> 3) & 0x7).f[2] - XMM(modrm & 0x7).f[2];
		XMM((modrm >> 3) & 0x7).f[3] = XMM((modrm >> 3) & 0x7).f[3] - XMM(modrm & 0x7).f[3];
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, src);
		XMM((modrm >> 3) & 0x7).f[0] = XMM((modrm >> 3) & 0x7).f[0] - src.f[0];
		XMM((modrm >> 3) & 0x7).f[1] = XMM((modrm >> 3) & 0x7).f[1] - src.f[1];
		XMM((modrm >> 3) & 0x7).f[2] = XMM((modrm >> 3) & 0x7).f[2] - src.f[2];
		XMM((modrm >> 3) & 0x7).f[3] = XMM((modrm >> 3) & 0x7).f[3] - src.f[3];
	}
	CYCLES(1);     // TODO: correct cycle count
}

INLINE float sse_min_single(float src1, float src2)
{
	/*if ((src1 == 0) && (src2 == 0))
	    return src2;
	if (src1 = SNaN)
	    return src2;
	if (src2 = SNaN)
	    return src2;*/
	if (src1 < src2)
		return src1;
	return src2;
}

void i386_device::sse_minps() // Opcode 0f 5d
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).f[0] = sse_min_single(XMM((modrm >> 3) & 0x7).f[0], XMM(modrm & 0x7).f[0]);
		XMM((modrm >> 3) & 0x7).f[1] = sse_min_single(XMM((modrm >> 3) & 0x7).f[1], XMM(modrm & 0x7).f[1]);
		XMM((modrm >> 3) & 0x7).f[2] = sse_min_single(XMM((modrm >> 3) & 0x7).f[2], XMM(modrm & 0x7).f[2]);
		XMM((modrm >> 3) & 0x7).f[3] = sse_min_single(XMM((modrm >> 3) & 0x7).f[3], XMM(modrm & 0x7).f[3]);
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, src);
		XMM((modrm >> 3) & 0x7).f[0] = sse_min_single(XMM((modrm >> 3) & 0x7).f[0], src.f[0]);
		XMM((modrm >> 3) & 0x7).f[1] = sse_min_single(XMM((modrm >> 3) & 0x7).f[1], src.f[1]);
		XMM((modrm >> 3) & 0x7).f[2] = sse_min_single(XMM((modrm >> 3) & 0x7).f[2], src.f[2]);
		XMM((modrm >> 3) & 0x7).f[3] = sse_min_single(XMM((modrm >> 3) & 0x7).f[3], src.f[3]);
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_divps() // Opcode 0f 5e
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).f[0] = XMM((modrm >> 3) & 0x7).f[0] / XMM(modrm & 0x7).f[0];
		XMM((modrm >> 3) & 0x7).f[1] = XMM((modrm >> 3) & 0x7).f[1] / XMM(modrm & 0x7).f[1];
		XMM((modrm >> 3) & 0x7).f[2] = XMM((modrm >> 3) & 0x7).f[2] / XMM(modrm & 0x7).f[2];
		XMM((modrm >> 3) & 0x7).f[3] = XMM((modrm >> 3) & 0x7).f[3] / XMM(modrm & 0x7).f[3];
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, src);
		XMM((modrm >> 3) & 0x7).f[0] = XMM((modrm >> 3) & 0x7).f[0] / src.f[0];
		XMM((modrm >> 3) & 0x7).f[1] = XMM((modrm >> 3) & 0x7).f[1] / src.f[1];
		XMM((modrm >> 3) & 0x7).f[2] = XMM((modrm >> 3) & 0x7).f[2] / src.f[2];
		XMM((modrm >> 3) & 0x7).f[3] = XMM((modrm >> 3) & 0x7).f[3] / src.f[3];
	}
	CYCLES(1);     // TODO: correct cycle count
}

INLINE float sse_max_single(float src1, float src2)
{
	/*if ((src1 == 0) && (src2 == 0))
	    return src2;
	if (src1 = SNaN)
	    return src2;
	if (src2 = SNaN)
	    return src2;*/
	if (src1 > src2)
		return src1;
	return src2;
}

void i386_device::sse_maxps() // Opcode 0f 5f
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).f[0] = sse_max_single(XMM((modrm >> 3) & 0x7).f[0], XMM(modrm & 0x7).f[0]);
		XMM((modrm >> 3) & 0x7).f[1] = sse_max_single(XMM((modrm >> 3) & 0x7).f[1], XMM(modrm & 0x7).f[1]);
		XMM((modrm >> 3) & 0x7).f[2] = sse_max_single(XMM((modrm >> 3) & 0x7).f[2], XMM(modrm & 0x7).f[2]);
		XMM((modrm >> 3) & 0x7).f[3] = sse_max_single(XMM((modrm >> 3) & 0x7).f[3], XMM(modrm & 0x7).f[3]);
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, src);
		XMM((modrm >> 3) & 0x7).f[0] = sse_max_single(XMM((modrm >> 3) & 0x7).f[0], src.f[0]);
		XMM((modrm >> 3) & 0x7).f[1] = sse_max_single(XMM((modrm >> 3) & 0x7).f[1], src.f[1]);
		XMM((modrm >> 3) & 0x7).f[2] = sse_max_single(XMM((modrm >> 3) & 0x7).f[2], src.f[2]);
		XMM((modrm >> 3) & 0x7).f[3] = sse_max_single(XMM((modrm >> 3) & 0x7).f[3], src.f[3]);
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_maxss_r128_r128m32() // Opcode f3 0f 5f
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).f[0] = sse_max_single(XMM((modrm >> 3) & 0x7).f[0], XMM(modrm & 0x7).f[0]);
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(modrm, 0);
		src.d[0]=READ32(ea);
		XMM((modrm >> 3) & 0x7).f[0] = sse_max_single(XMM((modrm >> 3) & 0x7).f[0], src.f[0]);
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_addss() // Opcode f3 0f 58
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).f[0] = XMM((modrm >> 3) & 0x7).f[0] + XMM(modrm & 0x7).f[0];
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, src);
		XMM((modrm >> 3) & 0x7).f[0] = XMM((modrm >> 3) & 0x7).f[0] + src.f[0];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_subss() // Opcode f3 0f 5c
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).f[0] = XMM((modrm >> 3) & 0x7).f[0] - XMM(modrm & 0x7).f[0];
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, src);
		XMM((modrm >> 3) & 0x7).f[0] = XMM((modrm >> 3) & 0x7).f[0] - src.f[0];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_mulss() // Opcode f3 0f 5e
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).f[0] = XMM((modrm >> 3) & 0x7).f[0] * XMM(modrm & 0x7).f[0];
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, src);
		XMM((modrm >> 3) & 0x7).f[0] = XMM((modrm >> 3) & 0x7).f[0] * src.f[0];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_divss() // Opcode 0f 59
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).f[0] = XMM((modrm >> 3) & 0x7).f[0] / XMM(modrm & 0x7).f[0];
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, src);
		XMM((modrm >> 3) & 0x7).f[0] = XMM((modrm >> 3) & 0x7).f[0] / src.f[0];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_rcpss_r128_r128m32() // Opcode f3 0f 53
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).f[0] = 1.0 / XMM(modrm & 0x7).f[0];
	} else {
		XMM_REG s;
		UINT32 ea = GetEA(modrm, 0);
		s.d[0]=READ32(ea);
		XMM((modrm >> 3) & 0x7).f[0] = 1.0 / s.f[0];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_sqrtss_r128_r128m32() // Opcode f3 0f 51
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).f[0] = sqrt(XMM(modrm & 0x7).f[0]);
	} else {
		XMM_REG s;
		UINT32 ea = GetEA(modrm, 0);
		s.d[0]=READ32(ea);
		XMM((modrm >> 3) & 0x7).f[0] = sqrt(s.f[0]);
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_rsqrtss_r128_r128m32() // Opcode f3 0f 52
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).f[0] = 1.0 / sqrt(XMM(modrm & 0x7).f[0]);
	} else {
		XMM_REG s;
		UINT32 ea = GetEA(modrm, 0);
		s.d[0]=READ32(ea);
		XMM((modrm >> 3) & 0x7).f[0] = 1.0 / sqrt(s.f[0]);
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_minss_r128_r128m32() // Opcode f3 0f 5d
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).f[0] = XMM((modrm >> 3) & 0x7).f[0] < XMM(modrm & 0x7).f[0] ? XMM((modrm >> 3) & 0x7).f[0] : XMM(modrm & 0x7).f[0];
	} else {
		XMM_REG s;
		UINT32 ea = GetEA(modrm, 0);
		s.d[0] = READ32(ea);
		XMM((modrm >> 3) & 0x7).f[0] = XMM((modrm >> 3) & 0x7).f[0] < s.f[0] ? XMM((modrm >> 3) & 0x7).f[0] : s.f[0];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_comiss_r128_r128m32() // Opcode 0f 2f
{
	float32 a,b;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		a = XMM((modrm >> 3) & 0x7).d[0];
		b = XMM(modrm & 0x7).d[0];
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, src);
		a = XMM((modrm >> 3) & 0x7).d[0];
		b = src.d[0];
	}
	m_OF=0;
	m_SF=0;
	m_AF=0;
	if (float32_is_nan(a) || float32_is_nan(b))
	{
		m_ZF = 1;
		m_PF = 1;
		m_CF = 1;
	}
	else
	{
		m_ZF = 0;
		m_PF = 0;
		m_CF = 0;
		if (float32_eq(a, b))
			m_ZF = 1;
		if (float32_lt(a, b))
			m_CF = 1;
	}
	// should generate exception when at least one of the operands is either QNaN or SNaN
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_ucomiss_r128_r128m32() // Opcode 0f 2e
{
	float32 a,b;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		a = XMM((modrm >> 3) & 0x7).d[0];
		b = XMM(modrm & 0x7).d[0];
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, src);
		a = XMM((modrm >> 3) & 0x7).d[0];
		b = src.d[0];
	}
	m_OF=0;
	m_SF=0;
	m_AF=0;
	if (float32_is_nan(a) || float32_is_nan(b))
	{
		m_ZF = 1;
		m_PF = 1;
		m_CF = 1;
	}
	else
	{
		m_ZF = 0;
		m_PF = 0;
		m_CF = 0;
		if (float32_eq(a, b))
			m_ZF = 1;
		if (float32_lt(a, b))
			m_CF = 1;
	}
	// should generate exception when at least one of the operands is SNaN
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_shufps() // Opcode 0f 67
{
	UINT8 modrm = FETCH();
	UINT8 sel = FETCH();
	int m1,m2,m3,m4;
	int s,d;
	m1=sel & 3;
	m2=(sel >> 2) & 3;
	m3=(sel >> 4) & 3;
	m4=(sel >> 6) & 3;
	s=modrm & 0x7;
	d=(modrm >> 3) & 0x7;
	if( modrm >= 0xc0 ) {
		UINT32 t;
		t=XMM(d).d[m1];
		XMM(d).d[1]=XMM(d).d[m2];
		XMM(d).d[0]=t;
		XMM(d).d[2]=XMM(s).d[m3];
		XMM(d).d[3]=XMM(s).d[m4];
	} else {
		UINT32 t;
		XMM_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, src);
		t=XMM(d).d[m1];
		XMM(d).d[1]=XMM(d).d[m2];
		XMM(d).d[0]=t;
		XMM(d).d[2]=src.d[m3];
		XMM(d).d[3]=src.d[m4];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_unpcklps_r128_rm128() // Opcode 0f 14
{
	UINT8 modrm = FETCH();
	int s,d;
	s=modrm & 0x7;
	d=(modrm >> 3) & 0x7;
	if( modrm >= 0xc0 ) {
		XMM(d).d[3]=XMM(s).d[1];
		XMM(d).d[2]=XMM(d).d[1];
		XMM(d).d[1]=XMM(s).d[0];
		//XMM(d).d[0]=XMM(d).d[0];
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, src);
		XMM(d).d[3]=src.d[1];
		XMM(d).d[2]=XMM(d).d[1];
		XMM(d).d[1]=src.d[0];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_unpckhps_r128_rm128() // Opcode 0f 15
{
	UINT8 modrm = FETCH();
	int s,d;
	s=modrm & 0x7;
	d=(modrm >> 3) & 0x7;
	if( modrm >= 0xc0 ) {
		XMM(d).d[0]=XMM(d).d[2];
		XMM(d).d[1]=XMM(s).d[2];
		XMM(d).d[2]=XMM(d).d[3];
		XMM(d).d[3]=XMM(s).d[3];
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, src);
		XMM(d).d[0]=XMM(d).d[2];
		XMM(d).d[1]=src.d[2];
		XMM(d).d[2]=XMM(d).d[3];
		XMM(d).d[3]=src.d[3];
	}
	CYCLES(1);     // TODO: correct cycle count
}

INLINE bool sse_issingleordered(float op1, float op2)
{
	// TODO: true when at least one of the two source operands being compared is a NaN
	return (op1 != op1) || (op1 != op2);
}

INLINE bool sse_issingleunordered(float op1, float op2)
{
	// TODO: true when neither source operand is a NaN
	return !((op1 != op1) || (op1 != op2));
}

void i386_device::sse_predicate_compare_single(UINT8 imm8, XMM_REG d, XMM_REG s)
{
	switch (imm8 & 7)
	{
	case 0:
		s.d[0]=s.f[0] == s.f[0] ? 0xffffffff : 0;
		d.d[1]=d.f[1] == s.f[1] ? 0xffffffff : 0;
		d.d[2]=d.f[2] == s.f[2] ? 0xffffffff : 0;
		d.d[3]=d.f[3] == s.f[3] ? 0xffffffff : 0;
		break;
	case 1:
		d.d[0]=d.f[0] < s.f[0] ? 0xffffffff : 0;
		d.d[1]=d.f[1] < s.f[1] ? 0xffffffff : 0;
		d.d[2]=d.f[2] < s.f[2] ? 0xffffffff : 0;
		d.d[3]=d.f[3] < s.f[3] ? 0xffffffff : 0;
		break;
	case 2:
		d.d[0]=d.f[0] <= s.f[0] ? 0xffffffff : 0;
		d.d[1]=d.f[1] <= s.f[1] ? 0xffffffff : 0;
		d.d[2]=d.f[2] <= s.f[2] ? 0xffffffff : 0;
		d.d[3]=d.f[3] <= s.f[3] ? 0xffffffff : 0;
		break;
	case 3:
		d.d[0]=sse_issingleunordered(d.f[0], s.f[0]) ? 0xffffffff : 0;
		d.d[1]=sse_issingleunordered(d.f[1], s.f[1]) ? 0xffffffff : 0;
		d.d[2]=sse_issingleunordered(d.f[2], s.f[2]) ? 0xffffffff : 0;
		d.d[3]=sse_issingleunordered(d.f[3], s.f[3]) ? 0xffffffff : 0;
		break;
	case 4:
		d.d[0]=d.f[0] != s.f[0] ? 0xffffffff : 0;
		d.d[1]=d.f[1] != s.f[1] ? 0xffffffff : 0;
		d.d[2]=d.f[2] != s.f[2] ? 0xffffffff : 0;
		d.d[3]=d.f[3] != s.f[3] ? 0xffffffff : 0;
		break;
	case 5:
		d.d[0]=d.f[0] < s.f[0] ? 0 : 0xffffffff;
		d.d[1]=d.f[1] < s.f[1] ? 0 : 0xffffffff;
		d.d[2]=d.f[2] < s.f[2] ? 0 : 0xffffffff;
		d.d[3]=d.f[3] < s.f[3] ? 0 : 0xffffffff;
		break;
	case 6:
		d.d[0]=d.f[0] <= s.f[0] ? 0 : 0xffffffff;
		d.d[1]=d.f[1] <= s.f[1] ? 0 : 0xffffffff;
		d.d[2]=d.f[2] <= s.f[2] ? 0 : 0xffffffff;
		d.d[3]=d.f[3] <= s.f[3] ? 0 : 0xffffffff;
		break;
	case 7:
		d.d[0]=sse_issingleordered(d.f[0], s.f[0]) ? 0xffffffff : 0;
		d.d[1]=sse_issingleordered(d.f[1], s.f[1]) ? 0xffffffff : 0;
		d.d[2]=sse_issingleordered(d.f[2], s.f[2]) ? 0xffffffff : 0;
		d.d[3]=sse_issingleordered(d.f[3], s.f[3]) ? 0xffffffff : 0;
		break;
	}
}

void i386_device::sse_predicate_compare_single_scalar(UINT8 imm8, XMM_REG d, XMM_REG s)
{
	switch (imm8 & 7)
	{
	case 0:
		s.d[0]=s.f[0] == s.f[0] ? 0xffffffff : 0;
		break;
	case 1:
		d.d[0]=d.f[0] < s.f[0] ? 0xffffffff : 0;
		break;
	case 2:
		d.d[0]=d.f[0] <= s.f[0] ? 0xffffffff : 0;
		break;
	case 3:
		d.d[0]=sse_issingleunordered(d.f[0], s.f[0]) ? 0xffffffff : 0;
		break;
	case 4:
		d.d[0]=d.f[0] != s.f[0] ? 0xffffffff : 0;
		break;
	case 5:
		d.d[0]=d.f[0] < s.f[0] ? 0 : 0xffffffff;
		break;
	case 6:
		d.d[0]=d.f[0] <= s.f[0] ? 0 : 0xffffffff;
		break;
	case 7:
		d.d[0]=sse_issingleordered(d.f[0], s.f[0]) ? 0xffffffff : 0;
		break;
	}
}

void i386_device::sse_cmpps_r128_rm128_i8() // Opcode 0f c2
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		int s,d;
		UINT8 imm8 = FETCH();
		s=modrm & 0x7;
		d=(modrm >> 3) & 0x7;
		sse_predicate_compare_single(imm8, XMM(d), XMM(s));
	} else {
		int d;
		XMM_REG s;
		UINT32 ea = GetEA(modrm, 0);
		UINT8 imm8 = FETCH();
		READXMM(ea, s);
		d=(modrm >> 3) & 0x7;
		sse_predicate_compare_single(imm8, XMM(d), s);
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_cmpss_r128_r128m32_i8() // Opcode f3 0f c2
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		int s,d;
		UINT8 imm8 = FETCH();
		s=modrm & 0x7;
		d=(modrm >> 3) & 0x7;
		sse_predicate_compare_single_scalar(imm8, XMM(d), XMM(s));
	} else {
		int d;
		XMM_REG s;
		UINT32 ea = GetEA(modrm, 0);
		UINT8 imm8 = FETCH();
		s.d[0]=READ32(ea);
		d=(modrm >> 3) & 0x7;
		sse_predicate_compare_single_scalar(imm8, XMM(d), s);
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_pinsrw_r64_r16m16_i8() // Opcode 0f c4
{
	MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		UINT8 imm8 = FETCH();
		UINT16 v = LOAD_RM16(modrm);
		MMX((modrm >> 3) & 0x7).w[imm8 & 3] = v;
	} else {
		UINT32 ea = GetEA(modrm, 0);
		UINT8 imm8 = FETCH();
		UINT16 v = READ16(ea);
		MMX((modrm >> 3) & 0x7).w[imm8 & 3] = v;
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_pinsrw_r64_r32m16_i8() // Opcode 0f c4
{
	MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		UINT8 imm8 = FETCH();
		UINT16 v = (UINT16)LOAD_RM32(modrm);
		MMX((modrm >> 3) & 0x7).w[imm8 & 3] = v;
	} else {
		UINT32 ea = GetEA(modrm, 0);
		UINT8 imm8 = FETCH();
		UINT16 v = READ16(ea);
		MMX((modrm >> 3) & 0x7).w[imm8 & 3] = v;
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_pextrw_r16_r64_i8() // Opcode 0f c5
{
	//MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		UINT8 imm8 = FETCH();
		STORE_REG16(modrm, MMX(modrm & 0x7).w[imm8 & 3]);
	} else {
		//UINT8 imm8 = FETCH();
		report_invalid_modrm("pextrw_r16_r64_i8", modrm);
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_pextrw_r32_r64_i8() // Opcode 0f c5
{
	//MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		UINT8 imm8 = FETCH();
		STORE_REG32(modrm, MMX(modrm & 0x7).w[imm8 & 3]);
	} else {
		//UINT8 imm8 = FETCH();
		report_invalid_modrm("pextrw_r32_r64_i8", modrm);
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_pminub_r64_rm64() // Opcode 0f da
{
	int n;
	MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		for (n=0;n < 8;n++)
			MMX((modrm >> 3) & 0x7).b[n] = MMX((modrm >> 3) & 0x7).b[n] < MMX(modrm & 0x7).b[n] ? MMX((modrm >> 3) & 0x7).b[n] : MMX(modrm & 0x7).b[n];
	} else {
		MMX_REG s;
		UINT32 ea = GetEA(modrm, 0);
		READMMX(ea, s);
		for (n=0;n < 8;n++)
			MMX((modrm >> 3) & 0x7).b[n] = MMX((modrm >> 3) & 0x7).b[n] < s.b[n] ? MMX((modrm >> 3) & 0x7).b[n] : s.b[n];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_pmaxub_r64_rm64() // Opcode 0f de
{
	int n;
	MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		for (n=0;n < 8;n++)
			MMX((modrm >> 3) & 0x7).b[n] = MMX((modrm >> 3) & 0x7).b[n] > MMX(modrm & 0x7).b[n] ? MMX((modrm >> 3) & 0x7).b[n] : MMX(modrm & 0x7).b[n];
	} else {
		MMX_REG s;
		UINT32 ea = GetEA(modrm, 0);
		READMMX(ea, s);
		for (n=0;n < 8;n++)
			MMX((modrm >> 3) & 0x7).b[n] = MMX((modrm >> 3) & 0x7).b[n] > s.b[n] ? MMX((modrm >> 3) & 0x7).b[n] : s.b[n];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_pavgb_r64_rm64() // Opcode 0f e0
{
	int n;
	MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		for (n=0;n < 8;n++)
			MMX((modrm >> 3) & 0x7).b[n] = ((UINT16)MMX((modrm >> 3) & 0x7).b[n] + (UINT16)MMX(modrm & 0x7).b[n] + 1) >> 1;
	} else {
		MMX_REG s;
		UINT32 ea = GetEA(modrm, 0);
		READMMX(ea, s);
		for (n=0;n < 8;n++)
			MMX((modrm >> 3) & 0x7).b[n] = ((UINT16)MMX((modrm >> 3) & 0x7).b[n] + (UINT16)s.b[n] + 1) >> 1;
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_pavgw_r64_rm64() // Opcode 0f e3
{
	int n;
	MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		for (n=0;n < 4;n++)
			MMX((modrm >> 3) & 0x7).w[n] = ((UINT32)MMX((modrm >> 3) & 0x7).w[n] + (UINT32)MMX(modrm & 0x7).w[n] + 1) >> 1;
	} else {
		MMX_REG s;
		UINT32 ea = GetEA(modrm, 0);
		READMMX(ea, s);
		for (n=0;n < 4;n++)
			MMX((modrm >> 3) & 0x7).w[n] = ((UINT32)MMX((modrm >> 3) & 0x7).w[n] + (UINT32)s.w[n] + 1) >> 1;
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_pmulhuw_r64_rm64()  // Opcode 0f e4
{
	MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		MMX((modrm >> 3) & 0x7).w[0]=((UINT32)MMX((modrm >> 3) & 0x7).w[0]*(UINT32)MMX(modrm & 7).w[0]) >> 16;
		MMX((modrm >> 3) & 0x7).w[1]=((UINT32)MMX((modrm >> 3) & 0x7).w[1]*(UINT32)MMX(modrm & 7).w[1]) >> 16;
		MMX((modrm >> 3) & 0x7).w[2]=((UINT32)MMX((modrm >> 3) & 0x7).w[2]*(UINT32)MMX(modrm & 7).w[2]) >> 16;
		MMX((modrm >> 3) & 0x7).w[3]=((UINT32)MMX((modrm >> 3) & 0x7).w[3]*(UINT32)MMX(modrm & 7).w[3]) >> 16;
	} else {
		MMX_REG s;
		UINT32 ea = GetEA(modrm, 0);
		READMMX(ea, s);
		MMX((modrm >> 3) & 0x7).w[0]=((UINT32)MMX((modrm >> 3) & 0x7).w[0]*(UINT32)s.w[0]) >> 16;
		MMX((modrm >> 3) & 0x7).w[1]=((UINT32)MMX((modrm >> 3) & 0x7).w[1]*(UINT32)s.w[1]) >> 16;
		MMX((modrm >> 3) & 0x7).w[2]=((UINT32)MMX((modrm >> 3) & 0x7).w[2]*(UINT32)s.w[2]) >> 16;
		MMX((modrm >> 3) & 0x7).w[3]=((UINT32)MMX((modrm >> 3) & 0x7).w[3]*(UINT32)s.w[3]) >> 16;
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_pminsw_r64_rm64() // Opcode 0f ea
{
	int n;
	MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		for (n=0;n < 4;n++)
			MMX((modrm >> 3) & 0x7).s[n] = MMX((modrm >> 3) & 0x7).s[n] < MMX(modrm & 0x7).s[n] ? MMX((modrm >> 3) & 0x7).s[n] : MMX(modrm & 0x7).s[n];
	} else {
		MMX_REG s;
		UINT32 ea = GetEA(modrm, 0);
		READMMX(ea, s);
		for (n=0;n < 4;n++)
			MMX((modrm >> 3) & 0x7).s[n] = MMX((modrm >> 3) & 0x7).s[n] < s.s[n] ? MMX((modrm >> 3) & 0x7).s[n] : s.s[n];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_pmaxsw_r64_rm64() // Opcode 0f ee
{
	int n;
	MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		for (n=0;n < 4;n++)
			MMX((modrm >> 3) & 0x7).s[n] = MMX((modrm >> 3) & 0x7).s[n] > MMX(modrm & 0x7).s[n] ? MMX((modrm >> 3) & 0x7).s[n] : MMX(modrm & 0x7).s[n];
	} else {
		MMX_REG s;
		UINT32 ea = GetEA(modrm, 0);
		READMMX(ea, s);
		for (n=0;n < 4;n++)
			MMX((modrm >> 3) & 0x7).s[n] = MMX((modrm >> 3) & 0x7).s[n] > s.s[n] ? MMX((modrm >> 3) & 0x7).s[n] : s.s[n];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_pmuludq_r64_rm64() // Opcode 0f f4
{
	MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		MMX((modrm >> 3) & 0x7).q = (UINT64)MMX((modrm >> 3) & 0x7).d[0] * (UINT64)MMX(modrm & 0x7).d[0];
	} else {
		MMX_REG s;
		UINT32 ea = GetEA(modrm, 0);
		READMMX(ea, s);
		MMX((modrm >> 3) & 0x7).q = (UINT64)MMX((modrm >> 3) & 0x7).d[0] * (UINT64)s.d[0];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_psadbw_r64_rm64() // Opcode 0f f6
{
	int n;
	INT32 temp;
	MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		temp=0;
		for (n=0;n < 8;n++)
			temp += abs((INT32)MMX((modrm >> 3) & 0x7).b[n] - (INT32)MMX(modrm & 0x7).b[n]);
		MMX((modrm >> 3) & 0x7).l=(UINT64)temp & 0xffff;
	} else {
		MMX_REG s;
		UINT32 ea = GetEA(modrm, 0);
		READMMX(ea, s);
		temp=0;
		for (n=0;n < 8;n++)
			temp += abs((INT32)MMX((modrm >> 3) & 0x7).b[n] - (INT32)s.b[n]);
		MMX((modrm >> 3) & 0x7).l=(UINT64)temp & 0xffff;
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_psubq_r64_rm64()  // Opcode 0f fb
{
	MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		MMX((modrm >> 3) & 0x7).q=MMX((modrm >> 3) & 0x7).q - MMX(modrm & 7).q;
	} else {
		MMX_REG s;
		UINT32 ea = GetEA(modrm, 0);
		READMMX(ea, s);
		MMX((modrm >> 3) & 0x7).q=MMX((modrm >> 3) & 0x7).q - s.q;
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_pshufhw_r128_rm128_i8() // Opcode f3 0f 70
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM_REG t;
		int s,d;
		UINT8 imm8 = FETCH();
		s=modrm & 0x7;
		d=(modrm >> 3) & 0x7;
		t.q[0]=XMM(s).q[1];
		XMM(d).q[0]=XMM(s).q[0];
		XMM(d).w[4]=t.w[imm8 & 3];
		XMM(d).w[5]=t.w[(imm8 >> 2) & 3];
		XMM(d).w[6]=t.w[(imm8 >> 4) & 3];
		XMM(d).w[7]=t.w[(imm8 >> 6) & 3];
	} else {
		XMM_REG s;
		int d=(modrm >> 3) & 0x7;
		UINT32 ea = GetEA(modrm, 0);
		UINT8 imm8 = FETCH();
		READXMM(ea, s);
		XMM(d).q[0]=s.q[0];
		XMM(d).w[4]=s.w[4 + (imm8 & 3)];
		XMM(d).w[5]=s.w[4 + ((imm8 >> 2) & 3)];
		XMM(d).w[6]=s.w[4 + ((imm8 >> 4) & 3)];
		XMM(d).w[7]=s.w[4 + ((imm8 >> 6) & 3)];
	}
	CYCLES(1);     // TODO: correct cycle count
}
