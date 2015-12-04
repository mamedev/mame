// license:BSD-3-Clause
// copyright-holders:Ville Linde, Barry Rodewald, Carl, Philip Bennett, Samuele Zannoli
// Pentium+ specific opcodes

extern flag float32_is_nan( float32 a ); // since its not defined in softfloat.h
extern flag float64_is_nan( float64 a ); // since its not defined in softfloat.h

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

void i386_device::i386_cyrix_special()     // Opcode 0x0f 3a-3d
{
/*
0f 3a       BB0_RESET (set BB0 pointer = base)
0f 3b       BB1_RESET (set BB1 pointer = base)
0f 3c       CPU_WRITE (write special CPU memory-mapped register, [ebx] = eax)
0f 3d       CPU_READ (read special CPU memory-mapped register, eax, = [ebx])
*/

	CYCLES(1);
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

void i386_device::sse_maskmovdqu_r128_r128()  // Opcode 66 0f f7
{
	int s,m,n;
	UINT8 modm = FETCH();
	UINT32 ea = GetEA(7, 0); // ds:di/edi/rdi register
	s=(modm >> 3) & 7;
	m=modm & 7;
	for (n=0;n < 16;n++)
		if (XMM(m).b[n] & 127)
			WRITE8(ea+n, XMM(s).b[n]);
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

INLINE UINT16 SaturatedSignedDwordToUnsignedWord(INT32 dword)
{
	if (dword > 65535)
		return 65535;
	if (dword < 0)
		return 0;
	return (UINT16)dword;
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

void i386_device::sse_group_660f71()  // Opcode 66 0f 71
{
	UINT8 modm = FETCH();
	UINT8 imm8 = FETCH();
	if (modm >= 0xc0) {
		switch ((modm & 0x38) >> 3)
		{
			case 2: // psrlw
				for (int n = 0; n < 8;n++)
					XMM(modm & 7).w[n] = XMM(modm & 7).w[n] >> imm8;
				break;
			case 4: // psraw
				for (int n = 0; n < 8;n++)
					XMM(modm & 7).s[n] = XMM(modm & 7).s[n] >> imm8;
				break;
			case 6: // psllw
				for (int n = 0; n < 8;n++)
					XMM(modm & 7).w[n] = XMM(modm & 7).w[n] << imm8;
				break;
			default:
				report_invalid_modrm("mmx_group660f71", modm);
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

void i386_device::sse_group_660f72()  // Opcode 66 0f 72
{
	UINT8 modm = FETCH();
	UINT8 imm8 = FETCH();
	if (modm >= 0xc0) {
		switch ((modm & 0x38) >> 3)
		{
			case 2: // psrld
				for (int n = 0; n < 4;n++)
					XMM(modm & 7).d[n] = XMM(modm & 7).d[n] >> imm8;
				break;
			case 4: // psrad
				for (int n = 0; n < 4;n++)
					XMM(modm & 7).i[n] = XMM(modm & 7).i[n] >> imm8;
				break;
			case 6: // pslld
				for (int n = 0; n < 4;n++)
					XMM(modm & 7).d[n] = XMM(modm & 7).d[n] << imm8;
				break;
			default:
				report_invalid_modrm("mmx_group660f72", modm);
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
				MMX(modm & 7).q = imm8 > 63 ? 0 : MMX(modm & 7).q >> imm8;
				break;
			case 6: // psllq
				MMX(modm & 7).q = imm8 > 63 ? 0 : MMX(modm & 7).q << imm8;
				break;
			default:
				report_invalid_modrm("mmx_group0f73", modm);
		}
	}
}

void i386_device::sse_group_660f73()  // Opcode 66 0f 73
{
	UINT64 t0;
	UINT8 modm = FETCH();
	UINT8 imm8 = FETCH();
	if (modm >= 0xc0) {
		switch ((modm & 0x38) >> 3)
		{
		case 2: // psrlq
			XMM(modm & 7).q[0] = imm8 > 63 ? 0 : XMM(modm & 7).q[0] >> imm8;
			XMM(modm & 7).q[1] = imm8 > 63 ? 0 : XMM(modm & 7).q[1] >> imm8;
			break;
		case 3: // psrldq
			if (imm8 >= 16)
			{
				XMM(modm & 7).q[0] = 0;
				XMM(modm & 7).q[1] = 0;
			}
			else if (imm8 >= 8)
			{
				imm8 = (imm8 & 7) << 3;
				XMM(modm & 7).q[0] = XMM(modm & 7).q[1] >> imm8;
				XMM(modm & 7).q[1] = 0;
			}
			else if (imm8)
			{
				t0 = XMM(modm & 7).q[0];
				imm8 = imm8 << 3;
				XMM(modm & 7).q[0] = (XMM(modm & 7).q[1] << (64 - imm8)) | (t0 >> imm8);
				XMM(modm & 7).q[1] = t0 >> imm8;
			}
			break;
		case 6: // psllq
			XMM(modm & 7).q[0] = imm8 > 63 ? 0 : XMM(modm & 7).q[0] << imm8;
			XMM(modm & 7).q[1] = imm8 > 63 ? 0 : XMM(modm & 7).q[1] << imm8;
			break;
		case 7: // pslldq
			if (imm8 >= 16)
			{
				XMM(modm & 7).q[0] = 0;
				XMM(modm & 7).q[1] = 0;
			}
			else if (imm8 >= 8)
			{
				imm8 = (imm8 & 7) << 3;
				XMM(modm & 7).q[1] = XMM(modm & 7).q[0] << imm8;
				XMM(modm & 7).q[0] = 0;
			}
			else if (imm8)
			{
				imm8 = imm8 << 3;
				XMM(modm & 7).q[1] = (XMM(modm & 7).q[0] >> (64 - imm8)) | (XMM(modm & 7).q[1] << imm8);
				XMM(modm & 7).q[0] = XMM(modm & 7).q[0] << imm8;
			}
			break;
		default:
			report_invalid_modrm("sse_group660f73", modm);
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

void i386_device::i386_cyrix_svdc() // Opcode 0f 78
{
	UINT8 modrm = FETCH();

	if( modrm < 0xc0 ) {
		UINT32 ea = GetEA(modrm,0);
		int index = (modrm >> 3) & 7;
		int limit;
		switch (index)
		{
			case 0:
			{
				index = ES;
				break;
			}

			case 2:
			{
				index = SS;
				break;
			}

			case 3:
			{
				index = DS;
				break;
			}

			case 4:
			{
				index = FS;
				break;
			}

			case 5:
			{
				index = GS;
				break;
			}

			default:
			{
				i386_trap(6, 0, 0);
			}
		}

		limit = m_sreg[index].limit;

		if (m_sreg[index].flags & 0x8000) //G bit
		{
			limit >>= 12;
		}

		WRITE16(ea + 0, limit);
		WRITE32(ea + 2, m_sreg[index].base);
		WRITE16(ea + 5, m_sreg[index].flags); //replace top 8 bits of base
		WRITE8(ea + 7, m_sreg[index].base >> 24);
		WRITE16(ea + 8, m_sreg[index].selector);
	} else {
		i386_trap(6, 0, 0);
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::i386_cyrix_rsdc() // Opcode 0f 79
{
	UINT8 modrm = FETCH();

	if( modrm < 0xc0 ) {
		UINT32 ea = GetEA(modrm,0);
		int index = (modrm >> 3) & 7;
		UINT16 flags;
		UINT32 base;
		UINT32 limit;
		switch (index)
		{
			case 0:
			{
				index = ES;
				break;
			}

			case 2:
			{
				index = SS;
				break;
			}

			case 3:
			{
				index = DS;
				break;
			}

			case 4:
			{
				index = FS;
				break;
			}

			case 5:
			{
				index = GS;
				break;
			}

			default:
			{
				i386_trap(6, 0, 0);
			}
		}

		base = (READ32(ea + 2) & 0x00ffffff) | (READ8(ea + 7) << 24);
		flags = READ16(ea + 5);
		limit = READ16(ea + 0) | ((flags & 3) << 16);

		if (flags & 0x8000) //G bit
		{
			limit = (limit << 12) | 0xfff;
		}

		m_sreg[index].selector = READ16(ea + 8);
		m_sreg[index].flags = flags;
		m_sreg[index].base = base;
		m_sreg[index].limit = limit;
	} else {
		i386_trap(6, 0, 0);
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::i386_cyrix_svldt() // Opcode 0f 7a
{
	if ( PROTECTED_MODE && !V8086_MODE )
	{
		UINT8 modrm = FETCH();

		if( !(modrm & 0xf8) ) {
			UINT32 ea = GetEA(modrm,0);
			UINT32 limit = m_ldtr.limit;

			if (m_ldtr.flags & 0x8000) //G bit
			{
				limit >>= 12;
			}

			WRITE16(ea + 0, limit);
			WRITE32(ea + 2, m_ldtr.base);
			WRITE16(ea + 5, m_ldtr.flags); //replace top 8 bits of base
			WRITE8(ea + 7, m_ldtr.base >> 24);
			WRITE16(ea + 8, m_ldtr.segment);
		} else {
			i386_trap(6, 0, 0);
		}
	} else {
		i386_trap(6, 0, 0);
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::i386_cyrix_rsldt() // Opcode 0f 7b
{
	if ( PROTECTED_MODE && !V8086_MODE )
	{
		if(m_CPL)
			FAULT(FAULT_GP,0)

		UINT8 modrm = FETCH();

		if( !(modrm & 0xf8) ) {
			UINT32 ea = GetEA(modrm,0);
			UINT16 flags = READ16(ea + 5);
			UINT32 base = (READ32(ea + 2) | 0x00ffffff) | (READ8(ea + 7) << 24);
			UINT32 limit = READ16(ea + 0) | ((flags & 3) << 16);
			I386_SREG seg;

			if (flags & 0x8000) //G bit
			{
				limit = (limit << 12) | 0xfff;
			}

			memset(&seg, 0, sizeof(seg));
			seg.selector = READ16(ea + 8);
			i386_load_protected_mode_segment(&seg,nullptr);
			m_ldtr.limit = limit;
			m_ldtr.base = base;
			m_ldtr.flags = flags;
		} else {
			i386_trap(6, 0, 0);
		}
	} else {
		i386_trap(6, 0, 0);
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::i386_cyrix_svts() // Opcode 0f 7c
{
	if ( PROTECTED_MODE )
	{
		UINT8 modrm = FETCH();

		if( !(modrm & 0xf8) ) {
			UINT32 ea = GetEA(modrm,0);
			UINT32 limit = m_task.limit;

			if (m_task.flags & 0x8000) //G bit
			{
				limit >>= 12;
			}

			WRITE16(ea + 0, limit);
			WRITE32(ea + 2, m_task.base);
			WRITE16(ea + 5, m_task.flags); //replace top 8 bits of base
			WRITE8(ea + 7, m_task.base >> 24);
			WRITE16(ea + 8, m_task.segment);
		} else {
			i386_trap(6, 0, 0);
		}
	} else {
		i386_trap(6, 0, 0);
	}
}

void i386_device::i386_cyrix_rsts() // Opcode 0f 7d
{
	if ( PROTECTED_MODE )
	{
		if(m_CPL)
			FAULT(FAULT_GP,0)

		UINT8 modrm = FETCH();

		if( !(modrm & 0xf8) ) {
			UINT32 ea = GetEA(modrm,0);
			UINT16 flags = READ16(ea + 5);
			UINT32 base = (READ32(ea + 2) | 0x00ffffff) | (READ8(ea + 7) << 24);
			UINT32 limit = READ16(ea + 0) | ((flags & 3) << 16);

			if (flags & 0x8000) //G bit
			{
				limit = (limit << 12) | 0xfff;
			}
			m_task.segment = READ16(ea + 8);
			m_task.limit = limit;
			m_task.base = base;
			m_task.flags = flags;
		} else {
			i386_trap(6, 0, 0);
		}
	} else {
		i386_trap(6, 0, 0);
	}
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

void i386_device::sse_punpcklbw_r128_rm128() // Opcode 66 0f 60
{
	UINT8 modrm = FETCH();
	if (modrm >= 0xc0) {
		XMM_REG xd,xs;
		int s, d;
		s = modrm & 0x7;
		d = (modrm >> 3) & 0x7;
		xd.l[0] = XMM(d).l[0];
		xs.l[0] = XMM(s).l[0];
		XMM(d).b[0] = xd.b[0];
		XMM(d).b[1] = xs.b[0];
		XMM(d).b[2] = xd.b[1];
		XMM(d).b[3] = xs.b[1];
		XMM(d).b[4] = xd.b[2];
		XMM(d).b[5] = xs.b[2];
		XMM(d).b[6] = xd.b[3];
		XMM(d).b[7] = xs.b[3];
		XMM(d).b[8] = xd.b[4];
		XMM(d).b[9] = xs.b[4];
		XMM(d).b[10] = xd.b[5];
		XMM(d).b[11] = xs.b[5];
		XMM(d).b[12] = xd.b[6];
		XMM(d).b[13] = xs.b[6];
		XMM(d).b[14] = xd.b[7];
		XMM(d).b[15] = xs.b[7];
	}
	else {
		XMM_REG xd, xs;
		int d = (modrm >> 3) & 0x7;
		UINT32 ea = GetEA(modrm, 0);
		xd.l[0] = XMM(d).l[0];
		xs.q[0] = READ64(ea);
		for (int n = 0; n < 8; n++) {
			XMM(d).b[n << 1] = xd.b[n];
			XMM(d).b[(n << 1) | 1] = xs.b[n];
		}
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_punpcklwd_r128_rm128()
{
	UINT8 modrm = FETCH();
	if (modrm >= 0xc0) {
		XMM_REG xd, xs;
		int s, d;
		s = modrm & 0x7;
		d = (modrm >> 3) & 0x7;
		xd.l[0] = XMM(d).l[0];
		xs.l[0] = XMM(s).l[0];
		for (int n = 0; n < 4; n++) {
			XMM(d).w[n << 1] = xd.w[n];
			XMM(d).w[(n << 1) | 1] = xs.w[n];
		}
	}
	else {
		XMM_REG xd, xs;
		int d = (modrm >> 3) & 0x7;
		UINT32 ea = GetEA(modrm, 0);
		xd.l[0] = XMM(d).l[0];
		xs.q[0] = READ64(ea);
		for (int n = 0; n < 4; n++) {
			XMM(d).w[n << 1] = xd.w[n];
			XMM(d).w[(n << 1) | 1] = xs.w[n];
		}
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_punpckldq_r128_rm128()
{
	UINT8 modrm = FETCH();
	if (modrm >= 0xc0) {
		XMM_REG xd, xs;
		int s, d;
		s = modrm & 0x7;
		d = (modrm >> 3) & 0x7;
		xd.l[0] = XMM(d).l[0];
		xs.l[0] = XMM(s).l[0];
		for (int n = 0; n < 2; n++) {
			XMM(d).d[n << 1] = xd.d[n];
			XMM(d).d[(n << 1) | 1] = xs.d[n];
		}
	}
	else {
		XMM_REG xd, xs;
		int d = (modrm >> 3) & 0x7;
		UINT32 ea = GetEA(modrm, 0);
		xd.l[0] = XMM(d).l[0];
		xs.q[0] = READ64(ea);
		for (int n = 0; n < 2; n++) {
			XMM(d).d[n << 1] = xd.d[n];
			XMM(d).d[(n << 1) | 1] = xs.d[n];
		}
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_punpcklqdq_r128_rm128()
{
	UINT8 modrm = FETCH();
	if (modrm >= 0xc0) {
		XMM_REG xd, xs;
		int s, d;
		s = modrm & 0x7;
		d = (modrm >> 3) & 0x7;
		xd.l[0] = XMM(d).l[0];
		xs.l[0] = XMM(s).l[0];
		XMM(d).q[0] = xd.q[0];
		XMM(d).q[1] = xs.q[0];
	}
	else {
		XMM_REG xd, xs;
		int d = (modrm >> 3) & 0x7;
		UINT32 ea = GetEA(modrm, 0);
		xd.l[0] = XMM(d).l[0];
		xs.q[0] = READ64(ea);
		XMM(d).q[0] = xd.q[0];
		XMM(d).q[1] = xs.q[0];
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
		MMX_REG ds, sd;
		int s,d;
		s=modrm & 0x7;
		d=(modrm >> 3) & 0x7;
		ds.q = MMX(d).q;
		sd.q = MMX(s).q;
		MMX(d).b[0]=SaturatedSignedWordToUnsignedByte(ds.s[0]);
		MMX(d).b[1]=SaturatedSignedWordToUnsignedByte(ds.s[1]);
		MMX(d).b[2]=SaturatedSignedWordToUnsignedByte(ds.s[2]);
		MMX(d).b[3]=SaturatedSignedWordToUnsignedByte(ds.s[3]);
		MMX(d).b[4]=SaturatedSignedWordToUnsignedByte(sd.s[0]);
		MMX(d).b[5]=SaturatedSignedWordToUnsignedByte(sd.s[1]);
		MMX(d).b[6]=SaturatedSignedWordToUnsignedByte(sd.s[2]);
		MMX(d).b[7]=SaturatedSignedWordToUnsignedByte(sd.s[3]);
	} else {
		MMX_REG s,t;
		int d=(modrm >> 3) & 0x7;
		UINT32 ea = GetEA(modrm, 0);
		READMMX(ea, s);
		t.q = MMX(d).q;
		MMX(d).b[0]=SaturatedSignedWordToUnsignedByte(t.s[0]);
		MMX(d).b[1]=SaturatedSignedWordToUnsignedByte(t.s[1]);
		MMX(d).b[2]=SaturatedSignedWordToUnsignedByte(t.s[2]);
		MMX(d).b[3]=SaturatedSignedWordToUnsignedByte(t.s[3]);
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
		INT32 t1, t2, t3, t4;
		s=modrm & 0x7;
		d=(modrm >> 3) & 0x7;
		t1 = MMX(d).i[0];
		t2 = MMX(d).i[1];
		t3 = MMX(s).i[0];
		t4 = MMX(s).i[1];
		MMX(d).s[0] = SaturatedSignedDwordToSignedWord(t1);
		MMX(d).s[1] = SaturatedSignedDwordToSignedWord(t2);
		MMX(d).s[2] = SaturatedSignedDwordToSignedWord(t3);
		MMX(d).s[3] = SaturatedSignedDwordToSignedWord(t4);
	}
	else {
		MMX_REG s;
		INT32 t1, t2;
		int d=(modrm >> 3) & 0x7;
		UINT32 ea = GetEA(modrm, 0);
		READMMX(ea, s);
		t1 = MMX(d).i[0];
		t2 = MMX(d).i[1];
		MMX(d).s[0] = SaturatedSignedDwordToSignedWord(t1);
		MMX(d).s[1] = SaturatedSignedDwordToSignedWord(t2);
		MMX(d).s[2] = SaturatedSignedDwordToSignedWord(s.i[0]);
		MMX(d).s[3] = SaturatedSignedDwordToSignedWord(s.i[1]);
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_group_0fae()  // Opcode 0f ae
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
				GetNonTranslatedEA(modm, nullptr);
				break;
			default:
				report_invalid_modrm("sse_group_0fae", modm);
		}
	} else {
		report_invalid_modrm("sse_group_0fae", modm);
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
		XMM((modrm >> 3) & 0x7).f[0] = (float)MMX(modrm & 0x7).i[0];
		XMM((modrm >> 3) & 0x7).f[1] = (float)MMX(modrm & 0x7).i[1];
	} else {
		MMX_REG r;
		UINT32 ea = GetEA(modrm, 0);
		READMMX(ea, r);
		XMM((modrm >> 3) & 0x7).f[0] = (float)r.i[0];
		XMM((modrm >> 3) & 0x7).f[1] = (float)r.i[1];
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

void i386_device::sse_movupd_r128_rm128() // Opcode 66 0f 10
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

void i386_device::sse_movupd_rm128_r128() // Opcode 66 0f 11
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
		// MOVHLPS opcode
		XMM((modrm >> 3) & 0x7).q[0] = XMM(modrm & 0x7).q[1];
		CYCLES(1);     // TODO: correct cycle count
	} else {
		// MOVLPS opcode
		UINT32 ea = GetEA(modrm, 0);
		READXMM_LO64(ea, XMM((modrm >> 3) & 0x7));
		CYCLES(1);     // TODO: correct cycle count
	}
}

void i386_device::sse_movlpd_r128_m64() // Opcode 66 0f 12
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		CYCLES(1);     // TODO: correct cycle count
	} else {
		// MOVLPS opcode
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

void i386_device::sse_movlpd_m64_r128() // Opcode 66 0f 13
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
		// MOVLHPS opcode
		XMM((modrm >> 3) & 0x7).q[1] = XMM(modrm & 0x7).q[0];
		CYCLES(1);     // TODO: correct cycle count
	} else {
		// MOVHPS opcode
		UINT32 ea = GetEA(modrm, 0);
		READXMM_HI64(ea, XMM((modrm >> 3) & 0x7));
		CYCLES(1);     // TODO: correct cycle count
	}
}

void i386_device::sse_movhpd_r128_m64() // Opcode 66 0f 16
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		// unsupported by cpu
		CYCLES(1);     // TODO: correct cycle count
	} else {
		// MOVHPS opcode
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

void i386_device::sse_movhpd_m64_r128() // Opcode 66 0f 17
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

void i386_device::sse_movmskpd_r32_r128() // Opcode 66 0f 50
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		int b;
		b=(XMM(modrm & 0x7).q[0] >> 63) & 1;
		b=b | ((XMM(modrm & 0x7).q[1] >> 62) & 2);
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

void i386_device::sse_movd_m128_rm32() // Opcode 66 0f 6e
{
	UINT8 modrm = FETCH();
	if (modrm >= 0xc0) {
		XMM((modrm >> 3) & 0x7).d[0] = LOAD_RM32(modrm);
	}
	else {
		UINT32 ea = GetEA(modrm, 0);
		XMM((modrm >> 3) & 0x7).d[0] = READ32(ea);
	}
	XMM((modrm >> 3) & 0x7).d[1] = 0;
	XMM((modrm >> 3) & 0x7).q[1] = 0;
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_movdqa_m128_rm128() // Opcode 66 0f 6f
{
	UINT8 modrm = FETCH();
	if (modrm >= 0xc0) {
		XMM((modrm >> 3) & 0x7).q[0] = XMM(modrm & 0x7).q[0];
		XMM((modrm >> 3) & 0x7).q[1] = XMM(modrm & 0x7).q[1];
	}
	else {
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, XMM((modrm >> 3) & 0x7));
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

void i386_device::sse_movd_rm32_r128() // Opcode 66 0f 7e
{
	UINT8 modrm = FETCH();
	if (modrm >= 0xc0) {
		STORE_RM32(modrm, XMM((modrm >> 3) & 0x7).d[0]);
	}
	else {
		UINT32 ea = GetEA(modrm, 0);
		WRITE32(ea, XMM((modrm >> 3) & 0x7).d[0]);
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_movdqa_rm128_r128() // Opcode 66 0f 7f
{
	UINT8 modrm = FETCH();
	if (modrm >= 0xc0) {
		XMM(modrm & 0x7).q[0] = XMM((modrm >> 3) & 0x7).q[0];
		XMM(modrm & 0x7).q[1] = XMM((modrm >> 3) & 0x7).q[1];
	}
	else {
		UINT32 ea = GetEA(modrm, 0);
		WRITEXMM(ea, XMM((modrm >> 3) & 0x7));
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

void i386_device::sse_pmovmskb_r32_r128() // Opcode 66 0f d7
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		UINT32 b;
		b=(XMM(modrm & 0x7).b[0] >> 7) & 1;
		b=b | ((XMM(modrm & 0x7).b[1] >> 6) & 2);
		b=b | ((XMM(modrm & 0x7).b[2] >> 5) & 4);
		b=b | ((XMM(modrm & 0x7).b[3] >> 4) & 8);
		b=b | ((XMM(modrm & 0x7).b[4] >> 3) & 16);
		b=b | ((XMM(modrm & 0x7).b[5] >> 2) & 32);
		b=b | ((XMM(modrm & 0x7).b[6] >> 1) & 64);
		b=b | ((XMM(modrm & 0x7).b[7] >> 0) & 128);
		b=b | ((XMM(modrm & 0x7).b[8] << 1) & 256);
		b=b | ((XMM(modrm & 0x7).b[9] << 2) & 512);
		b=b | ((XMM(modrm & 0x7).b[10] << 3) & 1024);
		b=b | ((XMM(modrm & 0x7).b[11] << 4) & 2048);
		b=b | ((XMM(modrm & 0x7).b[12] << 5) & 4096);
		b=b | ((XMM(modrm & 0x7).b[13] << 6) & 8192);
		b=b | ((XMM(modrm & 0x7).b[14] << 7) & 16384);
		b=b | ((XMM(modrm & 0x7).b[15] << 8) & 32768);
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

void i386_device::sse_xorpd_r128_rm128() // Opcode 66 0f 57
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).q[0] = XMM((modrm >> 3) & 0x7).q[0] ^ XMM(modrm & 0x7).q[0];
		XMM((modrm >> 3) & 0x7).q[1] = XMM((modrm >> 3) & 0x7).q[1] ^ XMM(modrm & 0x7).q[1];
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, src);
		XMM((modrm >> 3) & 0x7).q[0] = XMM((modrm >> 3) & 0x7).q[0] ^ src.q[0];
		XMM((modrm >> 3) & 0x7).q[1] = XMM((modrm >> 3) & 0x7).q[1] ^ src.q[1];
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
		XMM((modrm >> 3) & 0x7).f[0] = 1.0f / XMM(modrm & 0x7).f[0];
		XMM((modrm >> 3) & 0x7).f[1] = 1.0f / XMM(modrm & 0x7).f[1];
		XMM((modrm >> 3) & 0x7).f[2] = 1.0f / XMM(modrm & 0x7).f[2];
		XMM((modrm >> 3) & 0x7).f[3] = 1.0f / XMM(modrm & 0x7).f[3];
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, src);
		XMM((modrm >> 3) & 0x7).f[0] = 1.0f / src.f[0];
		XMM((modrm >> 3) & 0x7).f[1] = 1.0f / src.f[1];
		XMM((modrm >> 3) & 0x7).f[2] = 1.0f / src.f[2];
		XMM((modrm >> 3) & 0x7).f[3] = 1.0f / src.f[3];
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

void i386_device::sse_andpd_r128_rm128() // Opcode 66 0f 54
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

void i386_device::sse_andnpd_r128_rm128() // Opcode 66 0f 55
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

void i386_device::sse_orpd_r128_rm128() // Opcode 66 0f 56
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

INLINE double sse_min_double(double src1, double src2)
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

INLINE double sse_max_double(double src1, double src2)
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
		XMM((modrm >> 3) & 0x7).f[0] = 1.0f / XMM(modrm & 0x7).f[0];
	} else {
		XMM_REG s;
		UINT32 ea = GetEA(modrm, 0);
		s.d[0]=READ32(ea);
		XMM((modrm >> 3) & 0x7).f[0] = 1.0f / s.f[0];
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

void i386_device::sse_comisd_r128_r128m64() // Opcode 66 0f 2f
{
	float64 a,b;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		a = XMM((modrm >> 3) & 0x7).q[0];
		b = XMM(modrm & 0x7).q[0];
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, src);
		a = XMM((modrm >> 3) & 0x7).q[0];
		b = src.q[0];
	}
	m_OF=0;
	m_SF=0;
	m_AF=0;
	if (float64_is_nan(a) || float64_is_nan(b))
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
		if (float64_eq(a, b))
			m_ZF = 1;
		if (float64_lt(a, b))
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

void i386_device::sse_ucomisd_r128_r128m64() // Opcode 66 0f 2e
{
	float64 a,b;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		a = XMM((modrm >> 3) & 0x7).q[0];
		b = XMM(modrm & 0x7).q[0];
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, src);
		a = XMM((modrm >> 3) & 0x7).q[0];
		b = src.q[0];
	}
	m_OF=0;
	m_SF=0;
	m_AF=0;
	if (float64_is_nan(a) || float64_is_nan(b))
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
		if (float64_eq(a, b))
			m_ZF = 1;
		if (float64_lt(a, b))
			m_CF = 1;
	}
	// should generate exception when at least one of the operands is SNaN
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_shufps() // Opcode 0f c6
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
		UINT32 t1,t2,t3,t4;
		t1=XMM(d).d[m1];
		t2=XMM(d).d[m2];
		t3=XMM(s).d[m3];
		t4=XMM(s).d[m4];
		XMM(d).d[0]=t1;
		XMM(d).d[1]=t2;
		XMM(d).d[2]=t3;
		XMM(d).d[3]=t4;
	} else {
		UINT32 t1,t2;
		XMM_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, src);
		t1=XMM(d).d[m1];
		t2=XMM(d).d[m2];
		XMM(d).d[0]=t1;
		XMM(d).d[1]=t2;
		XMM(d).d[2]=src.d[m3];
		XMM(d).d[3]=src.d[m4];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_shufpd_r128_rm128_i8() // Opcode 66 0f c6
{
	UINT8 modrm = FETCH();
	UINT8 sel = FETCH();
	int m1,m2;
	int s,d;
	m1=sel & 1;
	m2=(sel >> 1) & 1;
	s=modrm & 0x7;
	d=(modrm >> 3) & 0x7;
	if( modrm >= 0xc0 ) {
		UINT64 t1,t2;
		t1=XMM(d).q[m1];
		t2=XMM(s).q[m2];
		XMM(d).q[0]=t1;
		XMM(d).q[1]=t2;
	} else {
		UINT64 t1;
		XMM_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, src);
		t1=XMM(d).q[m1];
		XMM(d).q[0]=t1;
		XMM(d).q[1]=src.q[m2];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_unpcklps_r128_rm128() // Opcode 0f 14
{
	UINT8 modrm = FETCH();
	int s,d;
	UINT32 t1, t2, t3, t4;
	s=modrm & 0x7;
	d=(modrm >> 3) & 0x7;
	if( modrm >= 0xc0 ) {
		t1 = XMM(s).d[1];
		t2 = XMM(d).d[1];
		t3 = XMM(s).d[0];
		t4 = XMM(d).d[0];
		XMM(d).d[3]=t1;
		XMM(d).d[2]=t2;
		XMM(d).d[1]=t3;
		XMM(d).d[0]=t4;
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, src);
		t2 = XMM(d).d[1];
		XMM(d).d[3]=src.d[1];
		XMM(d).d[2]=t2;
		XMM(d).d[1]=src.d[0];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_unpcklpd_r128_rm128() // Opcode 66 0f 14
{
	UINT8 modrm = FETCH();
	int s,d;
	s=modrm & 0x7;
	d=(modrm >> 3) & 0x7;
	if( modrm >= 0xc0 ) {
		XMM(d).q[1]=XMM(s).q[0];
		XMM(d).q[0]=XMM(d).q[0];
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, src);
		XMM(d).q[1]=src.q[0];
		XMM(d).q[0]=XMM(d).q[0];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_unpckhps_r128_rm128() // Opcode 0f 15
{
	UINT8 modrm = FETCH();
	int s,d;
	UINT32 t1, t2, t3, t4;
	s=modrm & 0x7;
	d=(modrm >> 3) & 0x7;
	if( modrm >= 0xc0 ) {
		t1 = XMM(d).d[2];
		t2 = XMM(s).d[2];
		t3 = XMM(d).d[3];
		t4 = XMM(s).d[3];
		XMM(d).d[0]=t1;
		XMM(d).d[1]=t2;
		XMM(d).d[2]=t3;
		XMM(d).d[3]=t4;
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, src);
		t1 = XMM(d).d[2];
		t2 = XMM(d).d[3];
		XMM(d).d[0]=t1;
		XMM(d).d[1]=src.d[2];
		XMM(d).d[2]=t2;
		XMM(d).d[3]=src.d[3];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_unpckhpd_r128_rm128() // Opcode 66 0f 15
{
	UINT8 modrm = FETCH();
	int s,d;
	s=modrm & 0x7;
	d=(modrm >> 3) & 0x7;
	if( modrm >= 0xc0 ) {
		XMM(d).q[0]=XMM(d).q[1];
		XMM(d).q[1]=XMM(s).q[1];
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, src);
		XMM(d).q[0]=XMM(d).q[1];
		XMM(d).q[1]=src.q[1];
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

INLINE bool sse_isdoubleordered(double op1, double op2)
{
	// TODO: true when at least one of the two source operands being compared is a NaN
	return (op1 != op1) || (op1 != op2);
}

INLINE bool sse_isdoubleunordered(double op1, double op2)
{
	// TODO: true when neither source operand is a NaN
	return !((op1 != op1) || (op1 != op2));
}

void i386_device::sse_predicate_compare_single(UINT8 imm8, XMM_REG d, XMM_REG s)
{
	switch (imm8 & 7)
	{
	case 0:
		d.d[0]=d.f[0] == s.f[0] ? 0xffffffff : 0;
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

void i386_device::sse_predicate_compare_double(UINT8 imm8, XMM_REG d, XMM_REG s)
{
	switch (imm8 & 7)
	{
	case 0:
		d.q[0]=d.f64[0] == s.f64[0] ? U64(0xffffffffffffffff) : 0;
		d.q[1]=d.f64[1] == s.f64[1] ? U64(0xffffffffffffffff) : 0;
		break;
	case 1:
		d.q[0]=d.f64[0] < s.f64[0] ? U64(0xffffffffffffffff) : 0;
		d.q[1]=d.f64[1] < s.f64[1] ? U64(0xffffffffffffffff) : 0;
		break;
	case 2:
		d.q[0]=d.f64[0] <= s.f64[0] ? U64(0xffffffffffffffff) : 0;
		d.q[1]=d.f64[1] <= s.f64[1] ? U64(0xffffffffffffffff) : 0;
		break;
	case 3:
		d.q[0]=sse_isdoubleunordered(d.f64[0], s.f64[0]) ? U64(0xffffffffffffffff) : 0;
		d.q[1]=sse_isdoubleunordered(d.f64[1], s.f64[1]) ? U64(0xffffffffffffffff) : 0;
		break;
	case 4:
		d.q[0]=d.f64[0] != s.f64[0] ? U64(0xffffffffffffffff) : 0;
		d.q[1]=d.f64[1] != s.f64[1] ? U64(0xffffffffffffffff) : 0;
		break;
	case 5:
		d.q[0]=d.f64[0] < s.f64[0] ? 0 : U64(0xffffffffffffffff);
		d.q[1]=d.f64[1] < s.f64[1] ? 0 : U64(0xffffffffffffffff);
		break;
	case 6:
		d.q[0]=d.f64[0] <= s.f64[0] ? 0 : U64(0xffffffffffffffff);
		d.q[1]=d.f64[1] <= s.f64[1] ? 0 : U64(0xffffffffffffffff);
		break;
	case 7:
		d.q[0]=sse_isdoubleordered(d.f64[0], s.f64[0]) ? U64(0xffffffffffffffff) : 0;
		d.q[1]=sse_isdoubleordered(d.f64[1], s.f64[1]) ? U64(0xffffffffffffffff) : 0;
		break;
	}
}

void i386_device::sse_predicate_compare_single_scalar(UINT8 imm8, XMM_REG d, XMM_REG s)
{
	switch (imm8 & 7)
	{
	case 0:
		d.d[0]=d.f[0] == s.f[0] ? 0xffffffff : 0;
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

void i386_device::sse_predicate_compare_double_scalar(UINT8 imm8, XMM_REG d, XMM_REG s)
{
	switch (imm8 & 7)
	{
	case 0:
		d.q[0]=d.f64[0] == s.f64[0] ? U64(0xffffffffffffffff) : 0;
		break;
	case 1:
		d.q[0]=d.f64[0] < s.f64[0] ? U64(0xffffffffffffffff) : 0;
		break;
	case 2:
		d.q[0]=d.f64[0] <= s.f64[0] ? U64(0xffffffffffffffff) : 0;
		break;
	case 3:
		d.q[0]=sse_isdoubleunordered(d.f64[0], s.f64[0]) ? U64(0xffffffffffffffff) : 0;
		break;
	case 4:
		d.q[0]=d.f64[0] != s.f64[0] ? U64(0xffffffffffffffff) : 0;
		break;
	case 5:
		d.q[0]=d.f64[0] < s.f64[0] ? 0 : U64(0xffffffffffffffff);
		break;
	case 6:
		d.q[0]=d.f64[0] <= s.f64[0] ? 0 : U64(0xffffffffffffffff);
		break;
	case 7:
		d.q[0]=sse_isdoubleordered(d.f64[0], s.f64[0]) ? U64(0xffffffffffffffff) : 0;
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

void i386_device::sse_cmppd_r128_rm128_i8() // Opcode 66 0f c2
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		int s,d;
		UINT8 imm8 = FETCH();
		s=modrm & 0x7;
		d=(modrm >> 3) & 0x7;
		sse_predicate_compare_double(imm8, XMM(d), XMM(s));
	} else {
		int d;
		XMM_REG s;
		UINT32 ea = GetEA(modrm, 0);
		UINT8 imm8 = FETCH();
		READXMM(ea, s);
		d=(modrm >> 3) & 0x7;
		sse_predicate_compare_double(imm8, XMM(d), s);
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

void i386_device::sse_pinsrw_r64_r16m16_i8() // Opcode 0f c4, 16bit register
{
	MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		UINT8 imm8 = FETCH();
		UINT16 v = LOAD_RM16(modrm);
		if (m_xmm_operand_size)
			XMM((modrm >> 3) & 0x7).w[imm8 & 7] = v;
		else
			MMX((modrm >> 3) & 0x7).w[imm8 & 3] = v;
	} else {
		UINT32 ea = GetEA(modrm, 0);
		UINT8 imm8 = FETCH();
		UINT16 v = READ16(ea);
		if (m_xmm_operand_size)
			XMM((modrm >> 3) & 0x7).w[imm8 & 7] = v;
		else
			MMX((modrm >> 3) & 0x7).w[imm8 & 3] = v;
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_pinsrw_r64_r32m16_i8() // Opcode 0f c4, 32bit register
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

void i386_device::sse_pinsrw_r128_r32m16_i8() // Opcode 66 0f c4
{
	UINT8 modrm = FETCH();
	if (modrm >= 0xc0) {
		UINT8 imm8 = FETCH();
		UINT16 v = (UINT16)LOAD_RM32(modrm);
		XMM((modrm >> 3) & 0x7).w[imm8 & 7] = v;
	}
	else {
		UINT32 ea = GetEA(modrm, 0);
		UINT8 imm8 = FETCH();
		UINT16 v = READ16(ea);
		XMM((modrm >> 3) & 0x7).w[imm8 & 7] = v;
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_pextrw_r16_r64_i8() // Opcode 0f c5
{
	//MMXPROLOG();
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		UINT8 imm8 = FETCH();
		if (m_xmm_operand_size)
			STORE_REG16(modrm, XMM(modrm & 0x7).w[imm8 & 7]);
		else
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

void i386_device::sse_pextrw_reg_r128_i8() // Opcode 66 0f c5
{
	UINT8 modrm = FETCH();
	if (modrm >= 0xc0) {
		UINT8 imm8 = FETCH();
		STORE_REG32(modrm, XMM(modrm & 0x7).w[imm8 & 7]);
	}
	else {
		//UINT8 imm8 = FETCH();
		report_invalid_modrm("sse_pextrw_reg_r128_i8", modrm);
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

void i386_device::sse_pminub_r128_rm128() // Opcode 66 0f da
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		for (int n=0;n < 16;n++)
			XMM((modrm >> 3) & 0x7).b[n] = XMM((modrm >> 3) & 0x7).b[n] < XMM(modrm & 0x7).b[n] ? XMM((modrm >> 3) & 0x7).b[n] : XMM(modrm & 0x7).b[n];
	} else {
		XMM_REG s;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, s);
		for (int n=0;n < 16;n++)
			XMM((modrm >> 3) & 0x7).b[n] = XMM((modrm >> 3) & 0x7).b[n] < s.b[n] ? XMM((modrm >> 3) & 0x7).b[n] : s.b[n];
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

void i386_device::sse_pmuludq_r128_rm128() // Opcode 66 0f f4
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).q[0] = (UINT64)XMM((modrm >> 3) & 0x7).d[0] * (UINT64)XMM(modrm & 0x7).d[0];
		XMM((modrm >> 3) & 0x7).q[1] = (UINT64)XMM((modrm >> 3) & 0x7).d[2] * (UINT64)XMM(modrm & 0x7).d[2];
	} else {
		XMM_REG s;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, s);
		XMM((modrm >> 3) & 0x7).q[0] = (UINT64)XMM((modrm >> 3) & 0x7).d[0] * (UINT64)s.d[0];
		XMM((modrm >> 3) & 0x7).q[1] = (UINT64)XMM((modrm >> 3) & 0x7).d[2] * (UINT64)s.d[2];
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

void i386_device::sse_psubq_r128_rm128()  // Opcode 66 0f fb
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).q[0]=XMM((modrm >> 3) & 0x7).q[0] - XMM(modrm & 7).q[0];
		XMM((modrm >> 3) & 0x7).q[1]=XMM((modrm >> 3) & 0x7).q[1] - XMM(modrm & 7).q[1];
	} else {
		XMM_REG s;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, s);
		XMM((modrm >> 3) & 0x7).q[0]=XMM((modrm >> 3) & 0x7).q[0] - s.q[0];
		XMM((modrm >> 3) & 0x7).q[1]=XMM((modrm >> 3) & 0x7).q[1] - s.q[1];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_pshufd_r128_rm128_i8() // Opcode 66 0f 70
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM_REG t;
		int s,d;
		UINT8 imm8 = FETCH();
		s=modrm & 0x7;
		d=(modrm >> 3) & 0x7;
		t.q[0]=XMM(s).q[0];
		t.q[1]=XMM(s).q[1];
		XMM(d).d[0]=t.d[imm8 & 3];
		XMM(d).d[1]=t.d[(imm8 >> 2) & 3];
		XMM(d).d[2]=t.d[(imm8 >> 4) & 3];
		XMM(d).d[3]=t.d[(imm8 >> 6) & 3];
	} else {
		XMM_REG s;
		int d=(modrm >> 3) & 0x7;
		UINT32 ea = GetEA(modrm, 0);
		UINT8 imm8 = FETCH();
		READXMM(ea, s);
		XMM(d).d[0]=s.d[(imm8 & 3)];
		XMM(d).d[1]=s.d[((imm8 >> 2) & 3)];
		XMM(d).d[2]=s.d[((imm8 >> 4) & 3)];
		XMM(d).d[3]=s.d[((imm8 >> 6) & 3)];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_pshuflw_r128_rm128_i8() // Opcode f2 0f 70
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM_REG t;
		int s,d;
		UINT8 imm8 = FETCH();
		s=modrm & 0x7;
		d=(modrm >> 3) & 0x7;
		t.q[0]=XMM(s).q[0];
		XMM(d).q[1]=XMM(s).q[1];
		XMM(d).w[0]=t.w[imm8 & 3];
		XMM(d).w[1]=t.w[(imm8 >> 2) & 3];
		XMM(d).w[2]=t.w[(imm8 >> 4) & 3];
		XMM(d).w[3]=t.w[(imm8 >> 6) & 3];
	} else {
		XMM_REG s;
		int d=(modrm >> 3) & 0x7;
		UINT32 ea = GetEA(modrm, 0);
		UINT8 imm8 = FETCH();
		READXMM(ea, s);
		XMM(d).q[1]=s.q[1];
		XMM(d).w[0]=s.w[imm8 & 3];
		XMM(d).w[1]=s.w[(imm8 >> 2) & 3];
		XMM(d).w[2]=s.w[(imm8 >> 4) & 3];
		XMM(d).w[3]=s.w[(imm8 >> 6) & 3];
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

void i386_device::sse_packsswb_r128_rm128() // Opcode 66 0f 63
{
	UINT8 modrm = FETCH();
	if (modrm >= 0xc0) {
		XMM_REG t;
		int s, d;
		s = modrm & 0x7;
		d = (modrm >> 3) & 0x7;
		t.q[0] = XMM(s).q[0];
		t.q[1] = XMM(s).q[1];
		for (int n = 0; n < 8; n++)
			XMM(d).c[n] = SaturatedSignedWordToSignedByte(XMM(d).s[n]);
		for (int n = 0; n < 8; n++)
			XMM(d).c[n+8] = SaturatedSignedWordToSignedByte(t.s[n]);
	}
	else {
		XMM_REG s;
		int d = (modrm >> 3) & 0x7;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, s);
		for (int n = 0; n < 8; n++)
			XMM(d).c[n] = SaturatedSignedWordToSignedByte(XMM(d).s[n]);
		for (int n = 0; n < 8; n++)
			XMM(d).c[n + 8] = SaturatedSignedWordToSignedByte(s.s[n]);
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_packssdw_r128_rm128() // Opcode 66 0f 6b
{
	UINT8 modrm = FETCH();
	if (modrm >= 0xc0) {
		XMM_REG t;
		int s, d;
		s = modrm & 0x7;
		d = (modrm >> 3) & 0x7;
		t.q[0] = XMM(s).q[0];
		t.q[1] = XMM(s).q[1];
		XMM(d).s[0] = SaturatedSignedDwordToSignedWord(XMM(d).i[0]);
		XMM(d).s[1] = SaturatedSignedDwordToSignedWord(XMM(d).i[1]);
		XMM(d).s[2] = SaturatedSignedDwordToSignedWord(XMM(d).i[2]);
		XMM(d).s[3] = SaturatedSignedDwordToSignedWord(XMM(d).i[3]);
		XMM(d).s[4] = SaturatedSignedDwordToSignedWord(t.i[0]);
		XMM(d).s[5] = SaturatedSignedDwordToSignedWord(t.i[1]);
		XMM(d).s[6] = SaturatedSignedDwordToSignedWord(t.i[2]);
		XMM(d).s[7] = SaturatedSignedDwordToSignedWord(t.i[3]);
	}
	else {
		XMM_REG s;
		int d = (modrm >> 3) & 0x7;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, s);
		XMM(d).s[0] = SaturatedSignedDwordToSignedWord(XMM(d).i[0]);
		XMM(d).s[1] = SaturatedSignedDwordToSignedWord(XMM(d).i[1]);
		XMM(d).s[2] = SaturatedSignedDwordToSignedWord(XMM(d).i[2]);
		XMM(d).s[3] = SaturatedSignedDwordToSignedWord(XMM(d).i[3]);
		XMM(d).s[4] = SaturatedSignedDwordToSignedWord(s.i[0]);
		XMM(d).s[5] = SaturatedSignedDwordToSignedWord(s.i[1]);
		XMM(d).s[6] = SaturatedSignedDwordToSignedWord(s.i[2]);
		XMM(d).s[7] = SaturatedSignedDwordToSignedWord(s.i[3]);
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_pcmpgtb_r128_rm128() // Opcode 66 0f 64
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		int s,d;
		s=modrm & 0x7;
		d=(modrm >> 3) & 0x7;
		for (int c=0;c <= 15;c++)
			XMM(d).b[c]=(XMM(d).c[c] > XMM(s).c[c]) ? 0xff : 0;
	} else {
		XMM_REG s;
		int d=(modrm >> 3) & 0x7;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, s);
		for (int c=0;c <= 15;c++)
			XMM(d).b[c]=(XMM(d).c[c] > s.c[c]) ? 0xff : 0;
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_pcmpgtw_r128_rm128() // Opcode 66 0f 65
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		int s,d;
		s=modrm & 0x7;
		d=(modrm >> 3) & 0x7;
		for (int c=0;c <= 7;c++)
			XMM(d).w[c]=(XMM(d).s[c] > XMM(s).s[c]) ? 0xffff : 0;
	} else {
		XMM_REG s;
		int d=(modrm >> 3) & 0x7;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, s);
		for (int c=0;c <= 7;c++)
			XMM(d).w[c]=(XMM(d).s[c] > s.s[c]) ? 0xffff : 0;
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_pcmpgtd_r128_rm128() // Opcode 66 0f 66
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		int s,d;
		s=modrm & 0x7;
		d=(modrm >> 3) & 0x7;
		for (int c=0;c <= 3;c++)
			XMM(d).d[c]=(XMM(d).i[c] > XMM(s).i[c]) ? 0xffffffff : 0;
	} else {
		XMM_REG s;
		int d=(modrm >> 3) & 0x7;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, s);
		for (int c=0;c <= 3;c++)
			XMM(d).d[c]=(XMM(d).i[c] > s.i[c]) ? 0xffffffff : 0;
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_packuswb_r128_rm128() // Opcode 66 0f 67
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM_REG t;
		int s,d;
		s=modrm & 0x7;
		d=(modrm >> 3) & 0x7;
		t.q[0] = XMM(s).q[0];
		t.q[1] = XMM(s).q[1];
		for (int n = 0; n < 8;n++)
			XMM(d).b[n]=SaturatedSignedWordToUnsignedByte(XMM(d).s[n]);
		for (int n = 0; n < 8;n++)
			XMM(d).b[n+8]=SaturatedSignedWordToUnsignedByte(t.s[n]);
	} else {
		XMM_REG s;
		int d=(modrm >> 3) & 0x7;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, s);
		for (int n = 0; n < 8;n++)
			XMM(d).b[n]=SaturatedSignedWordToUnsignedByte(XMM(d).s[n]);
		for (int n = 0; n < 8;n++)
			XMM(d).b[n+8]=SaturatedSignedWordToUnsignedByte(s.s[n]);
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_punpckhbw_r128_rm128() // Opcode 66 0f 68
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM_REG t;
		int s,d;
		s=modrm & 0x7;
		d=(modrm >> 3) & 0x7;
		t.q[1] = XMM(s).q[1];
		for (int n = 0; n < 16; n += 2) {
			XMM(d).b[n]=XMM(d).b[8+(n >> 1)];
			XMM(d).b[n+1]=t.b[8+(n >> 1)];
		}
	} else {
		XMM_REG s;
		int d=(modrm >> 3) & 0x7;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, s);
		for (int n = 0; n < 16; n += 2) {
			XMM(d).b[n]=XMM(d).b[8+(n >> 1)];
			XMM(d).b[n+1]=s.b[8+(n >> 1)];
		}
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_punpckhwd_r128_rm128() // Opcode 66 0f 69
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM_REG t;
		int s,d;
		s=modrm & 0x7;
		d=(modrm >> 3) & 0x7;
		t.q[1] = XMM(s).q[1];
		for (int n = 0; n < 8; n += 2) {
			XMM(d).w[n]=XMM(d).w[4+(n >> 1)];
			XMM(d).w[n+1]=t.w[4+(n >> 1)];
		}
	} else {
		XMM_REG s;
		int d=(modrm >> 3) & 0x7;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, s);
		for (int n = 0; n < 8; n += 2) {
			XMM(d).w[n]=XMM(d).w[4+(n >> 1)];
			XMM(d).w[n+1]=s.w[4+(n >> 1)];
		}
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_unpckhdq_r128_rm128() // Opcode 66 0f 6a
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM_REG t;
		int s,d;
		s=modrm & 0x7;
		d=(modrm >> 3) & 0x7;
		t.q[1] = XMM(s).q[1];
		XMM(d).d[0]=XMM(d).d[2];
		XMM(d).d[1]=t.d[2];
		XMM(d).d[2]=XMM(d).d[3];
		XMM(d).d[3]=t.d[3];
	} else {
		XMM_REG s;
		int d=(modrm >> 3) & 0x7;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, s);
		XMM(d).d[0]=XMM(d).d[2];
		XMM(d).d[1]=s.d[2];
		XMM(d).d[2]=XMM(d).d[3];
		XMM(d).d[3]=s.d[3];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_punpckhqdq_r128_rm128() // Opcode 66 0f 6d
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM_REG t;
		int s,d;
		s=modrm & 0x7;
		d=(modrm >> 3) & 0x7;
		t.q[1] = XMM(s).q[1];
		XMM(d).q[0]=XMM(d).q[1];
		XMM(d).q[1]=t.q[1];
	} else {
		XMM_REG s;
		int d=(modrm >> 3) & 0x7;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, s);
		XMM(d).q[0]=XMM(d).q[1];
		XMM(d).q[1]=s.q[1];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_pcmpeqb_r128_rm128() // Opcode 66 0f 74
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		int s,d;
		s=modrm & 0x7;
		d=(modrm >> 3) & 0x7;
		for (int c=0;c <= 15;c++)
			XMM(d).b[c]=(XMM(d).c[c] == XMM(s).c[c]) ? 0xff : 0;
	} else {
		XMM_REG s;
		int d=(modrm >> 3) & 0x7;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, s);
		for (int c=0;c <= 15;c++)
			XMM(d).b[c]=(XMM(d).c[c] == s.c[c]) ? 0xff : 0;
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_pcmpeqw_r128_rm128() // Opcode 66 0f 75
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		int s,d;
		s=modrm & 0x7;
		d=(modrm >> 3) & 0x7;
		for (int c=0;c <= 7;c++)
			XMM(d).w[c]=(XMM(d).s[c] == XMM(s).s[c]) ? 0xffff : 0;
	} else {
		XMM_REG s;
		int d=(modrm >> 3) & 0x7;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, s);
		for (int c=0;c <= 7;c++)
			XMM(d).w[c]=(XMM(d).s[c] == s.s[c]) ? 0xffff : 0;
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_pcmpeqd_r128_rm128() // Opcode 66 0f 76
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		int s,d;
		s=modrm & 0x7;
		d=(modrm >> 3) & 0x7;
		for (int c=0;c <= 3;c++)
			XMM(d).d[c]=(XMM(d).i[c] == XMM(s).i[c]) ? 0xffffffff : 0;
	} else {
		XMM_REG s;
		int d=(modrm >> 3) & 0x7;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, s);
		for (int c=0;c <= 3;c++)
			XMM(d).d[c]=(XMM(d).i[c] == s.i[c]) ? 0xffffffff : 0;
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_paddq_r128_rm128()  // Opcode 66 0f d4
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		int s,d;
		s=modrm & 0x7;
		d=(modrm >> 3) & 0x7;
		XMM(d).q[0]=XMM(d).q[0]+XMM(s).q[0];
		XMM(d).q[1]=XMM(d).q[1]+XMM(s).q[1];
	} else {
		XMM_REG src;
		int d=(modrm >> 3) & 0x7;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, src);
		XMM(d).q[0]=XMM(d).q[0]+src.q[0];
		XMM(d).q[1]=XMM(d).q[1]+src.q[1];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_pmullw_r128_rm128()  // Opcode 66 0f d5
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		int s,d;
		s=modrm & 0x7;
		d=(modrm >> 3) & 0x7;
		for (int n = 0; n < 8;n++)
			XMM(d).w[n]=(UINT32)((INT32)XMM(d).s[n]*(INT32)XMM(s).s[n]) & 0xffff;
	} else {
		XMM_REG src;
		int d;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, src);
		d=(modrm >> 3) & 0x7;
		for (int n = 0; n < 8;n++)
			XMM(d).w[n]=(UINT32)((INT32)XMM(d).s[n]*(INT32)src.s[n]) & 0xffff;
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_paddb_r128_rm128()  // Opcode 66 0f fc
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		for (int n=0;n < 16;n++)
			XMM((modrm >> 3) & 0x7).b[n]=XMM((modrm >> 3) & 0x7).b[n] + XMM(modrm & 7).b[n];
	} else {
		XMM_REG s;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, s);
		for (int n=0;n < 16;n++)
			XMM((modrm >> 3) & 0x7).b[n]=XMM((modrm >> 3) & 0x7).b[n] + s.b[n];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_paddw_r128_rm128()  // Opcode 66 0f fd
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		for (int n=0;n < 8;n++)
			XMM((modrm >> 3) & 0x7).w[n]=XMM((modrm >> 3) & 0x7).w[n] + XMM(modrm & 7).w[n];
	} else {
		XMM_REG s;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, s);
		for (int n=0;n < 8;n++)
			XMM((modrm >> 3) & 0x7).w[n]=XMM((modrm >> 3) & 0x7).w[n] + s.w[n];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_paddd_r128_rm128()  // Opcode 66 0f fe
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		for (int n=0;n < 4;n++)
			XMM((modrm >> 3) & 0x7).d[n]=XMM((modrm >> 3) & 0x7).d[n] + XMM(modrm & 7).d[n];
	} else {
		XMM_REG s;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, s);
		for (int n=0;n < 4;n++)
			XMM((modrm >> 3) & 0x7).d[n]=XMM((modrm >> 3) & 0x7).d[n] + s.d[n];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_psubusb_r128_rm128()  // Opcode 66 0f d8
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		for (int n=0;n < 16;n++)
			XMM((modrm >> 3) & 0x7).b[n]=XMM((modrm >> 3) & 0x7).b[n] < XMM(modrm & 7).b[n] ? 0 : XMM((modrm >> 3) & 0x7).b[n]-XMM(modrm & 7).b[n];
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, src);
		for (int n=0;n < 16;n++)
			XMM((modrm >> 3) & 0x7).b[n]=XMM((modrm >> 3) & 0x7).b[n] < src.b[n] ? 0 : XMM((modrm >> 3) & 0x7).b[n]-src.b[n];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_psubusw_r128_rm128()  // Opcode 66 0f d9
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		for (int n=0;n < 8;n++)
			XMM((modrm >> 3) & 0x7).w[n]=XMM((modrm >> 3) & 0x7).w[n] < XMM(modrm & 7).w[n] ? 0 : XMM((modrm >> 3) & 0x7).w[n]-XMM(modrm & 7).w[n];
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, src);
		for (int n=0;n < 8;n++)
			XMM((modrm >> 3) & 0x7).w[n]=XMM((modrm >> 3) & 0x7).w[n] < src.w[n] ? 0 : XMM((modrm >> 3) & 0x7).w[n]-src.w[n];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_pand_r128_rm128()  // Opcode 66 0f db
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).q[0]=XMM((modrm >> 3) & 0x7).q[0] & XMM(modrm & 7).q[0];
		XMM((modrm >> 3) & 0x7).q[1]=XMM((modrm >> 3) & 0x7).q[1] & XMM(modrm & 7).q[1];
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, src);
		XMM((modrm >> 3) & 0x7).q[0]=XMM((modrm >> 3) & 0x7).q[0] & src.q[0];
		XMM((modrm >> 3) & 0x7).q[1]=XMM((modrm >> 3) & 0x7).q[1] & src.q[1];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_pandn_r128_rm128()  // Opcode 66 0f df
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).q[0]=(~XMM((modrm >> 3) & 0x7).q[0]) & XMM(modrm & 7).q[0];
		XMM((modrm >> 3) & 0x7).q[1]=(~XMM((modrm >> 3) & 0x7).q[1]) & XMM(modrm & 7).q[1];
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, src);
		XMM((modrm >> 3) & 0x7).q[0]=(~XMM((modrm >> 3) & 0x7).q[0]) & src.q[0];
		XMM((modrm >> 3) & 0x7).q[1]=(~XMM((modrm >> 3) & 0x7).q[1]) & src.q[1];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_paddusb_r128_rm128()  // Opcode 66 0f dc
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		for (int n=0;n < 16;n++)
			XMM((modrm >> 3) & 0x7).b[n]=XMM((modrm >> 3) & 0x7).b[n] > (0xff-XMM(modrm & 7).b[n]) ? 0xff : XMM((modrm >> 3) & 0x7).b[n]+XMM(modrm & 7).b[n];
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, src);
		for (int n=0;n < 16;n++)
			XMM((modrm >> 3) & 0x7).b[n]=XMM((modrm >> 3) & 0x7).b[n] > (0xff-src.b[n]) ? 0xff : XMM((modrm >> 3) & 0x7).b[n]+src.b[n];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_paddusw_r128_rm128()  // Opcode 66 0f dd
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		for (int n=0;n < 8;n++)
			XMM((modrm >> 3) & 0x7).w[n]=XMM((modrm >> 3) & 0x7).w[n] > (0xffff-XMM(modrm & 7).w[n]) ? 0xffff : XMM((modrm >> 3) & 0x7).w[n]+XMM(modrm & 7).w[n];
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, src);
		for (int n=0;n < 8;n++)
			XMM((modrm >> 3) & 0x7).w[n]=XMM((modrm >> 3) & 0x7).w[n] > (0xffff-src.w[n]) ? 0xffff : XMM((modrm >> 3) & 0x7).w[n]+src.w[n];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_pmaxub_r128_rm128() // Opcode 66 0f de
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		for (int n=0;n < 16;n++)
			XMM((modrm >> 3) & 0x7).b[n] = XMM((modrm >> 3) & 0x7).b[n] > XMM(modrm & 0x7).b[n] ? XMM((modrm >> 3) & 0x7).b[n] : XMM(modrm & 0x7).b[n];
	} else {
		XMM_REG s;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, s);
		for (int n=0;n < 16;n++)
			XMM((modrm >> 3) & 0x7).b[n] = XMM((modrm >> 3) & 0x7).b[n] > s.b[n] ? XMM((modrm >> 3) & 0x7).b[n] : s.b[n];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_pmulhuw_r128_rm128()  // Opcode 66 0f e4
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		for (int n=0;n < 8;n++)
			XMM((modrm >> 3) & 0x7).w[n]=((UINT32)XMM((modrm >> 3) & 0x7).w[n]*(UINT32)XMM(modrm & 7).w[n]) >> 16;
	} else {
		XMM_REG s;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, s);
		for (int n=0;n < 8;n++)
			XMM((modrm >> 3) & 0x7).w[n]=((UINT32)XMM((modrm >> 3) & 0x7).w[n]*(UINT32)s.w[n]) >> 16;
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_pmulhw_r128_rm128()  // Opcode 66 0f e5
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		for (int n=0;n < 8;n++)
			XMM((modrm >> 3) & 0x7).w[n]=(UINT32)((INT32)XMM((modrm >> 3) & 0x7).s[n]*(INT32)XMM(modrm & 7).s[n]) >> 16;
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, src);
		for (int n=0;n < 8;n++)
			XMM((modrm >> 3) & 0x7).w[n]=(UINT32)((INT32)XMM((modrm >> 3) & 0x7).s[n]*(INT32)src.s[n]) >> 16;
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_psubsb_r128_rm128()  // Opcode 66 0f e8
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		for (int n=0;n < 16;n++)
			XMM((modrm >> 3) & 0x7).c[n]=SaturatedSignedWordToSignedByte((INT16)XMM((modrm >> 3) & 0x7).c[n] - (INT16)XMM(modrm & 7).c[n]);
	} else {
		XMM_REG s;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, s);
		for (int n=0;n < 16;n++)
			XMM((modrm >> 3) & 0x7).c[n]=SaturatedSignedWordToSignedByte((INT16)XMM((modrm >> 3) & 0x7).c[n] - (INT16)s.c[n]);
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_psubsw_r128_rm128()  // Opcode 66 0f e9
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		for (int n=0;n < 8;n++)
			XMM((modrm >> 3) & 0x7).s[n]=SaturatedSignedDwordToSignedWord((INT32)XMM((modrm >> 3) & 0x7).s[n] - (INT32)XMM(modrm & 7).s[n]);
	} else {
		XMM_REG s;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, s);
		for (int n=0;n < 8;n++)
			XMM((modrm >> 3) & 0x7).s[n]=SaturatedSignedDwordToSignedWord((INT32)XMM((modrm >> 3) & 0x7).s[n] - (INT32)s.s[n]);
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_pminsw_r128_rm128() // Opcode 66 0f ea
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		for (int n=0;n < 8;n++)
			XMM((modrm >> 3) & 0x7).s[n] = XMM((modrm >> 3) & 0x7).s[n] < XMM(modrm & 0x7).s[n] ? XMM((modrm >> 3) & 0x7).s[n] : XMM(modrm & 0x7).s[n];
	} else {
		XMM_REG s;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, s);
		for (int n=0;n < 8;n++)
			XMM((modrm >> 3) & 0x7).s[n] = XMM((modrm >> 3) & 0x7).s[n] < s.s[n] ? XMM((modrm >> 3) & 0x7).s[n] : s.s[n];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_pmaxsw_r128_rm128() // Opcode 66 0f ee
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		for (int n=0;n < 8;n++)
			XMM((modrm >> 3) & 0x7).s[n] = XMM((modrm >> 3) & 0x7).s[n] > XMM(modrm & 0x7).s[n] ? XMM((modrm >> 3) & 0x7).s[n] : XMM(modrm & 0x7).s[n];
	} else {
		XMM_REG s;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, s);
		for (int n=0;n < 8;n++)
			XMM((modrm >> 3) & 0x7).s[n] = XMM((modrm >> 3) & 0x7).s[n] > s.s[n] ? XMM((modrm >> 3) & 0x7).s[n] : s.s[n];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_paddsb_r128_rm128()  // Opcode 66 0f ec
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		for (int n=0;n < 16;n++)
			XMM((modrm >> 3) & 0x7).c[n]=SaturatedSignedWordToSignedByte((INT16)XMM((modrm >> 3) & 0x7).c[n] + (INT16)XMM(modrm & 7).c[n]);
	} else {
		XMM_REG s;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, s);
		for (int n=0;n < 16;n++)
			XMM((modrm >> 3) & 0x7).c[n]=SaturatedSignedWordToSignedByte((INT16)XMM((modrm >> 3) & 0x7).c[n] + (INT16)s.c[n]);
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_paddsw_r128_rm128()  // Opcode 66 0f ed
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		for (int n=0;n < 8;n++)
			XMM((modrm >> 3) & 0x7).s[n]=SaturatedSignedDwordToSignedWord((INT32)XMM((modrm >> 3) & 0x7).s[n] + (INT32)XMM(modrm & 7).s[n]);
	} else {
		XMM_REG s;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, s);
		for (int n=0;n < 8;n++)
			XMM((modrm >> 3) & 0x7).s[n]=SaturatedSignedDwordToSignedWord((INT32)XMM((modrm >> 3) & 0x7).s[n] + (INT32)s.s[n]);
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_por_r128_rm128()  // Opcode 66 0f eb
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).q[0]=XMM((modrm >> 3) & 0x7).q[0] | XMM(modrm & 7).q[0];
		XMM((modrm >> 3) & 0x7).q[1]=XMM((modrm >> 3) & 0x7).q[1] | XMM(modrm & 7).q[1];
	} else {
		XMM_REG s;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, s);
		XMM((modrm >> 3) & 0x7).q[0]=XMM((modrm >> 3) & 0x7).q[0] | s.q[0];
		XMM((modrm >> 3) & 0x7).q[1]=XMM((modrm >> 3) & 0x7).q[1] | s.q[1];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_pxor_r128_rm128()  // Opcode 66 0f ef
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).q[0]=XMM((modrm >> 3) & 0x7).q[0] ^ XMM(modrm & 7).q[0];
		XMM((modrm >> 3) & 0x7).q[1]=XMM((modrm >> 3) & 0x7).q[1] ^ XMM(modrm & 7).q[1];
	} else {
		XMM_REG s;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, s);
		XMM((modrm >> 3) & 0x7).q[0]=XMM((modrm >> 3) & 0x7).q[0] ^ s.q[0];
		XMM((modrm >> 3) & 0x7).q[1]=XMM((modrm >> 3) & 0x7).q[1] ^ s.q[1];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_pmaddwd_r128_rm128()  // Opcode 66 0f f5
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		for (int n=0;n < 4;n++)
			XMM((modrm >> 3) & 0x7).i[n]=(INT32)XMM((modrm >> 3) & 0x7).s[n]*(INT32)XMM(modrm & 7).s[n]+
										(INT32)XMM((modrm >> 3) & 0x7).s[n]*(INT32)XMM(modrm & 7).s[n];
	} else {
		XMM_REG s;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, s);
		for (int n=0;n < 4;n++)
			XMM((modrm >> 3) & 0x7).i[n]=(INT32)XMM((modrm >> 3) & 0x7).s[n]*(INT32)s.s[n]+
										(INT32)XMM((modrm >> 3) & 0x7).s[n]*(INT32)s.s[n];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_psubb_r128_rm128()  // Opcode 66 0f f8
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		for (int n=0;n < 16;n++)
			XMM((modrm >> 3) & 0x7).b[n]=XMM((modrm >> 3) & 0x7).b[n] - XMM(modrm & 7).b[n];
	} else {
		XMM_REG s;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, s);
		for (int n=0;n < 16;n++)
			XMM((modrm >> 3) & 0x7).b[n]=XMM((modrm >> 3) & 0x7).b[n] - s.b[n];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_psubw_r128_rm128()  // Opcode 66 0f f9
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		for (int n=0;n < 8;n++)
			XMM((modrm >> 3) & 0x7).w[n]=XMM((modrm >> 3) & 0x7).w[n] - XMM(modrm & 7).w[n];
	} else {
		XMM_REG s;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, s);
		for (int n=0;n < 8;n++)
			XMM((modrm >> 3) & 0x7).w[n]=XMM((modrm >> 3) & 0x7).w[n] - s.w[n];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_psubd_r128_rm128()  // Opcode 66 0f fa
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		for (int n=0;n < 4;n++)
			XMM((modrm >> 3) & 0x7).d[n]=XMM((modrm >> 3) & 0x7).d[n] - XMM(modrm & 7).d[n];
	} else {
		XMM_REG s;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, s);
		for (int n=0;n < 4;n++)
			XMM((modrm >> 3) & 0x7).d[n]=XMM((modrm >> 3) & 0x7).d[n] - s.d[n];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_psadbw_r128_rm128() // Opcode 66 0f f6
{
	INT32 temp;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		temp=0;
		for (int n=0;n < 8;n++)
			temp += abs((INT32)XMM((modrm >> 3) & 0x7).b[n] - (INT32)XMM(modrm & 0x7).b[n]);
		XMM((modrm >> 3) & 0x7).l[0]=(UINT64)temp & 0xffff;
		temp=0;
		for (int n=8;n < 16;n++)
			temp += abs((INT32)XMM((modrm >> 3) & 0x7).b[n] - (INT32)XMM(modrm & 0x7).b[n]);
		XMM((modrm >> 3) & 0x7).l[1]=(UINT64)temp & 0xffff;
	} else {
		XMM_REG s;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, s);
		temp=0;
		for (int n=0;n < 8;n++)
			temp += abs((INT32)XMM((modrm >> 3) & 0x7).b[n] - (INT32)s.b[n]);
		XMM((modrm >> 3) & 0x7).l[0]=(UINT64)temp & 0xffff;
		temp=0;
		for (int n=8;n < 16;n++)
			temp += abs((INT32)XMM((modrm >> 3) & 0x7).b[n] - (INT32)s.b[n]);
		XMM((modrm >> 3) & 0x7).l[1]=(UINT64)temp & 0xffff;
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_pavgb_r128_rm128() // Opcode 66 0f e0
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		for (int n=0;n < 16;n++)
			XMM((modrm >> 3) & 0x7).b[n] = ((UINT16)XMM((modrm >> 3) & 0x7).b[n] + (UINT16)XMM(modrm & 0x7).b[n] + 1) >> 1;
	} else {
		XMM_REG s;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, s);
		for (int n=0;n < 16;n++)
			XMM((modrm >> 3) & 0x7).b[n] = ((UINT16)XMM((modrm >> 3) & 0x7).b[n] + (UINT16)s.b[n] + 1) >> 1;
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_pavgw_r128_rm128() // Opcode 66 0f e3
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		for (int n=0;n < 8;n++)
			XMM((modrm >> 3) & 0x7).w[n] = ((UINT32)XMM((modrm >> 3) & 0x7).w[n] + (UINT32)XMM(modrm & 0x7).w[n] + 1) >> 1;
	} else {
		XMM_REG s;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, s);
		for (int n=0;n < 8;n++)
			XMM((modrm >> 3) & 0x7).w[n] = ((UINT32)XMM((modrm >> 3) & 0x7).w[n] + (UINT32)s.w[n] + 1) >> 1;
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_psrlw_r128_rm128()  // Opcode 66 0f d1
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		int count=(int)XMM(modrm & 7).q[0];
		for (int n=0; n < 8;n++)
			XMM((modrm >> 3) & 0x7).w[n]=XMM((modrm >> 3) & 0x7).w[n] >> count;
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, src);
		int count=(int)src.q[0];
		for (int n=0; n < 8;n++)
			XMM((modrm >> 3) & 0x7).w[n]=XMM((modrm >> 3) & 0x7).w[n] >> count;
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_psrld_r128_rm128()  // Opcode 66 0f d2
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		int count=(int)XMM(modrm & 7).q[0];
		XMM((modrm >> 3) & 0x7).d[0]=XMM((modrm >> 3) & 0x7).d[0] >> count;
		XMM((modrm >> 3) & 0x7).d[1]=XMM((modrm >> 3) & 0x7).d[1] >> count;
		XMM((modrm >> 3) & 0x7).d[2]=XMM((modrm >> 3) & 0x7).d[2] >> count;
		XMM((modrm >> 3) & 0x7).d[3]=XMM((modrm >> 3) & 0x7).d[3] >> count;
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, src);
		int count=(int)src.q[0];
		XMM((modrm >> 3) & 0x7).d[0]=XMM((modrm >> 3) & 0x7).d[0] >> count;
		XMM((modrm >> 3) & 0x7).d[1]=XMM((modrm >> 3) & 0x7).d[1] >> count;
		XMM((modrm >> 3) & 0x7).d[2]=XMM((modrm >> 3) & 0x7).d[2] >> count;
		XMM((modrm >> 3) & 0x7).d[3]=XMM((modrm >> 3) & 0x7).d[3] >> count;
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_psrlq_r128_rm128()  // Opcode 66 0f d3
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		int count=(int)XMM(modrm & 7).q[0];
		XMM((modrm >> 3) & 0x7).q[0]=XMM((modrm >> 3) & 0x7).q[0] >> count;
		XMM((modrm >> 3) & 0x7).q[1]=XMM((modrm >> 3) & 0x7).q[1] >> count;
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, src);
		int count=(int)src.q[0];
		XMM((modrm >> 3) & 0x7).q[0]=XMM((modrm >> 3) & 0x7).q[0] >> count;
		XMM((modrm >> 3) & 0x7).q[1]=XMM((modrm >> 3) & 0x7).q[1] >> count;
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_psllw_r128_rm128()  // Opcode 66 0f f1
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		int count=(int)XMM(modrm & 7).q[0];
		for (int n=0; n < 8;n++)
			XMM((modrm >> 3) & 0x7).w[n]=XMM((modrm >> 3) & 0x7).w[n] << count;
	} else {
		XMM_REG s;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, s);
		int count=(int)s.q[0];
		for (int n=0; n < 8;n++)
			XMM((modrm >> 3) & 0x7).w[n]=XMM((modrm >> 3) & 0x7).w[n] << count;
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_pslld_r128_rm128()  // Opcode 66 0f f2
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		int count=(int)XMM(modrm & 7).q[0];
		XMM((modrm >> 3) & 0x7).d[0]=XMM((modrm >> 3) & 0x7).d[0] << count;
		XMM((modrm >> 3) & 0x7).d[1]=XMM((modrm >> 3) & 0x7).d[1] << count;
		XMM((modrm >> 3) & 0x7).d[2]=XMM((modrm >> 3) & 0x7).d[2] << count;
		XMM((modrm >> 3) & 0x7).d[3]=XMM((modrm >> 3) & 0x7).d[3] << count;
	} else {
		XMM_REG s;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, s);
		int count=(int)s.q[0];
		XMM((modrm >> 3) & 0x7).d[0]=XMM((modrm >> 3) & 0x7).d[0] << count;
		XMM((modrm >> 3) & 0x7).d[1]=XMM((modrm >> 3) & 0x7).d[1] << count;
		XMM((modrm >> 3) & 0x7).d[2]=XMM((modrm >> 3) & 0x7).d[2] << count;
		XMM((modrm >> 3) & 0x7).d[3]=XMM((modrm >> 3) & 0x7).d[3] << count;
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_psllq_r128_rm128()  // Opcode 66 0f f3
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		int count=(int)XMM(modrm & 7).q[0];
		XMM((modrm >> 3) & 0x7).q[0]=XMM((modrm >> 3) & 0x7).q[0] << count;
		XMM((modrm >> 3) & 0x7).q[1]=XMM((modrm >> 3) & 0x7).q[1] << count;
	} else {
		XMM_REG s;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, s);
		int count=(int)s.q[0];
		XMM((modrm >> 3) & 0x7).q[0]=XMM((modrm >> 3) & 0x7).q[0] << count;
		XMM((modrm >> 3) & 0x7).q[1]=XMM((modrm >> 3) & 0x7).q[1] << count;
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_psraw_r128_rm128()  // Opcode 66 0f e1
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		int count=(int)XMM(modrm & 7).q[0];
		for (int n=0; n < 8;n++)
			XMM((modrm >> 3) & 0x7).s[n]=XMM((modrm >> 3) & 0x7).s[n] >> count;
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, src);
		int count=(int)src.q[0];
		for (int n=0; n < 8;n++)
			XMM((modrm >> 3) & 0x7).s[n]=XMM((modrm >> 3) & 0x7).s[n] >> count;
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_psrad_r128_rm128()  // Opcode 66 0f e2
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		int count=(int)XMM(modrm & 7).q[0];
		XMM((modrm >> 3) & 0x7).i[0]=XMM((modrm >> 3) & 0x7).i[0] >> count;
		XMM((modrm >> 3) & 0x7).i[1]=XMM((modrm >> 3) & 0x7).i[1] >> count;
		XMM((modrm >> 3) & 0x7).i[2]=XMM((modrm >> 3) & 0x7).i[2] >> count;
		XMM((modrm >> 3) & 0x7).i[3]=XMM((modrm >> 3) & 0x7).i[3] >> count;
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, src);
		int count=(int)src.q[0];
		XMM((modrm >> 3) & 0x7).i[0]=XMM((modrm >> 3) & 0x7).i[0] >> count;
		XMM((modrm >> 3) & 0x7).i[1]=XMM((modrm >> 3) & 0x7).i[1] >> count;
		XMM((modrm >> 3) & 0x7).i[2]=XMM((modrm >> 3) & 0x7).i[2] >> count;
		XMM((modrm >> 3) & 0x7).i[3]=XMM((modrm >> 3) & 0x7).i[3] >> count;
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_movntdq_m128_r128()  // Opcode 66 0f e7
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		CYCLES(1);     // unsupported
	} else {
		// since cache is not implemented
		UINT32 ea = GetEA(modrm, 0);
		WRITEXMM(ea, XMM((modrm >> 3) & 0x7));
		CYCLES(1);     // TODO: correct cycle count
	}
}

void i386_device::sse_cvttpd2dq_r128_rm128()  // Opcode 66 0f e6
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).i[0]=(INT32)XMM((modrm >> 3) & 0x7).f64[0];
		XMM((modrm >> 3) & 0x7).i[1]=(INT32)XMM((modrm >> 3) & 0x7).f64[1];
		XMM((modrm >> 3) & 0x7).q[1] = 0;
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, src);
		XMM((modrm >> 3) & 0x7).i[0]=(INT32)src.f64[0];
		XMM((modrm >> 3) & 0x7).i[1]=(INT32)src.f64[1];
		XMM((modrm >> 3) & 0x7).q[1] = 0;
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_movq_r128m64_r128()  // Opcode 66 0f d6
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM(modrm & 0x7).q[0]=XMM((modrm >> 3) & 0x7).q[0];
		XMM(modrm & 0x7).q[1] = 0;
	} else {
		UINT32 ea = GetEA(modrm, 0);
		WRITE64(ea, XMM((modrm >> 3) & 0x7).q[0]);
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_addsubpd_r128_rm128()  // Opcode 66 0f d0
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		int s, d;
		s=modrm & 0x7;
		d=(modrm >> 3) & 0x7;
		XMM(d).f64[0]=XMM(d).f64[0]-XMM(s).f64[0];
		XMM(d).f64[1]=XMM(d).f64[1]+XMM(s).f64[1];
	} else {
		XMM_REG src;
		int d;
		UINT32 ea = GetEA(modrm, 0);
		d=(modrm >> 3) & 0x7;
		READXMM(ea, src);
		XMM(d).f64[0]=XMM(d).f64[0]-src.f64[0];
		XMM(d).f64[1]=XMM(d).f64[1]+src.f64[1];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_haddpd_r128_rm128()  // Opcode 66 0f 7c
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		int s, d;
		s=modrm & 0x7;
		d=(modrm >> 3) & 0x7;
		XMM(d).f64[0]=XMM(d).f64[0]+XMM(d).f64[1];
		XMM(d).f64[1]=XMM(s).f64[0]+XMM(s).f64[1];
	} else {
		XMM_REG src;
		int d;
		UINT32 ea = GetEA(modrm, 0);
		d=(modrm >> 3) & 0x7;
		READXMM(ea, src);
		XMM(d).f64[0]=XMM(d).f64[0]+XMM(d).f64[1];
		XMM(d).f64[1]=src.f64[0]+src.f64[1];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_hsubpd_r128_rm128()  // Opcode 66 0f 7d
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		int s, d;
		s=modrm & 0x7;
		d=(modrm >> 3) & 0x7;
		XMM(d).f64[0]=XMM(d).f64[0]-XMM(d).f64[1];
		XMM(d).f64[1]=XMM(s).f64[0]-XMM(s).f64[1];
	} else {
		XMM_REG src;
		int d;
		UINT32 ea = GetEA(modrm, 0);
		d=(modrm >> 3) & 0x7;
		READXMM(ea, src);
		XMM(d).f64[0]=XMM(d).f64[0]-XMM(d).f64[1];
		XMM(d).f64[1]=src.f64[0]-src.f64[1];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_sqrtpd_r128_rm128()  // Opcode 66 0f 51
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		int s, d;
		s=modrm & 0x7;
		d=(modrm >> 3) & 0x7;
		XMM(d).f64[0]=sqrt(XMM(s).f64[0]);
		XMM(d).f64[1]=sqrt(XMM(s).f64[1]);
	} else {
		XMM_REG src;
		int d;
		UINT32 ea = GetEA(modrm, 0);
		d=(modrm >> 3) & 0x7;
		READXMM(ea, src);
		XMM(d).f64[0]=sqrt(src.f64[0]);
		XMM(d).f64[1]=sqrt(src.f64[1]);
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_cvtpi2pd_r128_rm64()  // Opcode 66 0f 2a
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		MMXPROLOG();
		XMM((modrm >> 3) & 0x7).f64[0] = (double)MMX(modrm & 0x7).i[0];
		XMM((modrm >> 3) & 0x7).f64[1] = (double)MMX(modrm & 0x7).i[1];
	} else {
		MMX_REG r;
		UINT32 ea = GetEA(modrm, 0);
		READMMX(ea, r);
		XMM((modrm >> 3) & 0x7).f64[0] = (double)r.i[0];
		XMM((modrm >> 3) & 0x7).f64[1] = (double)r.i[1];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_cvttpd2pi_r64_rm128()  // Opcode 66 0f 2c
{
	UINT8 modrm = FETCH();
	MMXPROLOG();
	if( modrm >= 0xc0 ) {
		MMX((modrm >> 3) & 0x7).i[0] = XMM(modrm & 0x7).f64[0];
		MMX((modrm >> 3) & 0x7).i[1] = XMM(modrm & 0x7).f64[1];
	} else {
		XMM_REG r;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, r);
		MMX((modrm >> 3) & 0x7).i[0] = r.f64[0];
		MMX((modrm >> 3) & 0x7).i[1] = r.f64[1];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_cvtpd2pi_r64_rm128()  // Opcode 66 0f 2d
{
	UINT8 modrm = FETCH();
	MMXPROLOG();
	if( modrm >= 0xc0 ) {
		MMX((modrm >> 3) & 0x7).i[0] = XMM(modrm & 0x7).f64[0];
		MMX((modrm >> 3) & 0x7).i[1] = XMM(modrm & 0x7).f64[1];
	} else {
		XMM_REG r;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, r);
		MMX((modrm >> 3) & 0x7).i[0] = r.f64[0];
		MMX((modrm >> 3) & 0x7).i[1] = r.f64[1];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_cvtpd2ps_r128_rm128()  // Opcode 66 0f 5a
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).f[0] = (float)XMM(modrm & 0x7).f64[0];
		XMM((modrm >> 3) & 0x7).f[1] = (float)XMM(modrm & 0x7).f64[1];
		XMM((modrm >> 3) & 0x7).q[1] = 0;
	} else {
		XMM_REG r;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, r);
		XMM((modrm >> 3) & 0x7).f[0] = (float)r.f64[0];
		XMM((modrm >> 3) & 0x7).f[1] = (float)r.f64[1];
		XMM((modrm >> 3) & 0x7).q[1] = 0;
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_cvtps2dq_r128_rm128()  // Opcode 66 0f 5b
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).i[0] = XMM(modrm & 0x7).f[0];
		XMM((modrm >> 3) & 0x7).i[1] = XMM(modrm & 0x7).f[1];
		XMM((modrm >> 3) & 0x7).i[2] = XMM(modrm & 0x7).f[2];
		XMM((modrm >> 3) & 0x7).i[3] = XMM(modrm & 0x7).f[3];
	} else {
		XMM_REG r;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, r);
		XMM((modrm >> 3) & 0x7).i[0] = r.f[0];
		XMM((modrm >> 3) & 0x7).i[1] = r.f[1];
		XMM((modrm >> 3) & 0x7).i[2] = r.f[2];
		XMM((modrm >> 3) & 0x7).i[3] = r.f[3];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_addpd_r128_rm128()  // Opcode 66 0f 58
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).f64[0] = XMM((modrm >> 3) & 0x7).f64[0] + XMM(modrm & 0x7).f64[0];
		XMM((modrm >> 3) & 0x7).f64[1] = XMM((modrm >> 3) & 0x7).f64[1] + XMM(modrm & 0x7).f64[1];
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, src);
		XMM((modrm >> 3) & 0x7).f64[0] = XMM((modrm >> 3) & 0x7).f64[0] + src.f64[0];
		XMM((modrm >> 3) & 0x7).f64[1] = XMM((modrm >> 3) & 0x7).f64[1] + src.f64[1];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_mulpd_r128_rm128()  // Opcode 66 0f 59
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).f64[0] = XMM((modrm >> 3) & 0x7).f64[0] * XMM(modrm & 0x7).f64[0];
		XMM((modrm >> 3) & 0x7).f64[1] = XMM((modrm >> 3) & 0x7).f64[1] * XMM(modrm & 0x7).f64[1];
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, src);
		XMM((modrm >> 3) & 0x7).f64[0] = XMM((modrm >> 3) & 0x7).f64[0] * src.f64[0];
		XMM((modrm >> 3) & 0x7).f64[1] = XMM((modrm >> 3) & 0x7).f64[1] * src.f64[1];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_subpd_r128_rm128()  // Opcode 66 0f 5c
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).f64[0] = XMM((modrm >> 3) & 0x7).f64[0] - XMM(modrm & 0x7).f64[0];
		XMM((modrm >> 3) & 0x7).f64[1] = XMM((modrm >> 3) & 0x7).f64[1] - XMM(modrm & 0x7).f64[1];
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, src);
		XMM((modrm >> 3) & 0x7).f64[0] = XMM((modrm >> 3) & 0x7).f64[0] - src.f64[0];
		XMM((modrm >> 3) & 0x7).f64[1] = XMM((modrm >> 3) & 0x7).f64[1] - src.f64[1];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_minpd_r128_rm128()  // Opcode 66 0f 5d
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).f64[0] = sse_min_double(XMM((modrm >> 3) & 0x7).f64[0], XMM(modrm & 0x7).f64[0]);
		XMM((modrm >> 3) & 0x7).f64[1] = sse_min_double(XMM((modrm >> 3) & 0x7).f64[1], XMM(modrm & 0x7).f64[1]);
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, src);
		XMM((modrm >> 3) & 0x7).f64[0] = sse_min_double(XMM((modrm >> 3) & 0x7).f64[0], src.f64[0]);
		XMM((modrm >> 3) & 0x7).f64[1] = sse_min_double(XMM((modrm >> 3) & 0x7).f64[1], src.f64[1]);
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_divpd_r128_rm128()  // Opcode 66 0f 5e
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).f64[0] = XMM((modrm >> 3) & 0x7).f64[0] / XMM(modrm & 0x7).f64[0];
		XMM((modrm >> 3) & 0x7).f64[1] = XMM((modrm >> 3) & 0x7).f64[1] / XMM(modrm & 0x7).f64[1];
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, src);
		XMM((modrm >> 3) & 0x7).f64[0] = XMM((modrm >> 3) & 0x7).f64[0] / src.f64[0];
		XMM((modrm >> 3) & 0x7).f64[1] = XMM((modrm >> 3) & 0x7).f64[1] / src.f64[1];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_maxpd_r128_rm128()  // Opcode 66 0f 5f
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).f64[0] = sse_max_double(XMM((modrm >> 3) & 0x7).f64[0], XMM(modrm & 0x7).f64[0]);
		XMM((modrm >> 3) & 0x7).f64[1] = sse_max_double(XMM((modrm >> 3) & 0x7).f64[1], XMM(modrm & 0x7).f64[1]);
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, src);
		XMM((modrm >> 3) & 0x7).f64[0] = sse_max_double(XMM((modrm >> 3) & 0x7).f64[0], src.f64[0]);
		XMM((modrm >> 3) & 0x7).f64[1] = sse_max_double(XMM((modrm >> 3) & 0x7).f64[1], src.f64[1]);
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_movntpd_m128_r128()  // Opcode 66 0f 2b
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

void i386_device::sse_movapd_r128_rm128()  // Opcode 66 0f 28
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

void i386_device::sse_movapd_rm128_r128()  // Opcode 66 0f 29
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

void i386_device::sse_movsd_r128_r128m64() // Opcode f2 0f 10
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).q[0] = XMM(modrm & 0x7).q[0];
	} else {
		UINT32 ea = GetEA(modrm, 0);
		READXMM_LO64(ea, XMM((modrm >> 3) & 0x7));
		XMM((modrm >> 3) & 0x7).q[1] = 0;
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_movsd_r128m64_r128() // Opcode f2 0f 11
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM(modrm & 0x7).q[0] = XMM((modrm >> 3) & 0x7).q[0];
	} else {
		UINT32 ea = GetEA(modrm, 0);
		WRITEXMM_LO64(ea, XMM((modrm >> 3) & 0x7));
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_movddup_r128_r128m64() // Opcode f2 0f 12
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).q[0] = XMM(modrm & 0x7).q[0];
		XMM((modrm >> 3) & 0x7).q[1] = XMM((modrm >> 3) & 0x7).q[0];
	} else {
		UINT32 ea = GetEA(modrm, 0);
		READXMM_LO64(ea, XMM((modrm >> 3) & 0x7));
		XMM((modrm >> 3) & 0x7).q[1] = XMM((modrm >> 3) & 0x7).q[0];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_cvtsi2sd_r128_rm32() // Opcode f2 0f 2a
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).f64[0] = (INT32)LOAD_RM32(modrm);
	} else {
		UINT32 ea = GetEA(modrm, 0);
		XMM((modrm >> 3) & 0x7).f64[0] = (INT32)READ32(ea);
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_cvttsd2si_r32_r128m64() // Opcode f2 0f 2c
{
	INT32 src;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = (INT32)XMM(modrm & 0x7).f64[0];
	} else { // otherwise is a memory address
		XMM_REG t;
		UINT32 ea = GetEA(modrm, 0);
		READXMM_LO64(ea, t);
		src = (INT32)t.f64[0];
	}
	STORE_REG32(modrm, (UINT32)src);
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_cvtsd2si_r32_r128m64() // Opcode f2 0f 2d
{
	INT32 src;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = (INT32)XMM(modrm & 0x7).f64[0];
	} else { // otherwise is a memory address
		XMM_REG t;
		UINT32 ea = GetEA(modrm, 0);
		READXMM_LO64(ea, t);
		src = (INT32)t.f64[0];
	}
	STORE_REG32(modrm, (UINT32)src);
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_sqrtsd_r128_r128m64() // Opcode f2 0f 51
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		int s, d;
		s=modrm & 0x7;
		d=(modrm >> 3) & 0x7;
		XMM(d).f64[0]=sqrt(XMM(s).f64[0]);
	} else {
		XMM_REG src;
		int d;
		UINT32 ea = GetEA(modrm, 0);
		d=(modrm >> 3) & 0x7;
		READXMM(ea, src);
		XMM(d).f64[0]=sqrt(src.f64[0]);
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_addsd_r128_r128m64() // Opcode f2 0f 58
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).f64[0] = XMM((modrm >> 3) & 0x7).f64[0] + XMM(modrm & 0x7).f64[0];
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, src);
		XMM((modrm >> 3) & 0x7).f64[0] = XMM((modrm >> 3) & 0x7).f64[0] + src.f64[0];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_mulsd_r128_r128m64() // Opcode f2 0f 59
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).f64[0] = XMM((modrm >> 3) & 0x7).f64[0] * XMM(modrm & 0x7).f64[0];
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, src);
		XMM((modrm >> 3) & 0x7).f64[0] = XMM((modrm >> 3) & 0x7).f64[0] * src.f64[0];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_cvtsd2ss_r128_r128m64() // Opcode f2 0f 5a
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).f[0] = XMM(modrm & 0x7).f64[0];
	} else {
		XMM_REG s;
		UINT32 ea = GetEA(modrm, 0);
		READXMM_LO64(ea, s);
		XMM((modrm >> 3) & 0x7).f[0] = s.f64[0];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_subsd_r128_r128m64() // Opcode f2 0f 5c
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).f64[0] = XMM((modrm >> 3) & 0x7).f64[0] - XMM(modrm & 0x7).f64[0];
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, src);
		XMM((modrm >> 3) & 0x7).f64[0] = XMM((modrm >> 3) & 0x7).f64[0] - src.f64[0];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_minsd_r128_r128m64() // Opcode f2 0f 5d
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).f64[0] = sse_min_double(XMM((modrm >> 3) & 0x7).f64[0], XMM(modrm & 0x7).f64[0]);
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, src);
		XMM((modrm >> 3) & 0x7).f64[0] = sse_min_double(XMM((modrm >> 3) & 0x7).f64[0], src.f64[0]);
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_divsd_r128_r128m64() // Opcode f2 0f 5e
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).f64[0] = XMM((modrm >> 3) & 0x7).f64[0] / XMM(modrm & 0x7).f64[0];
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, src);
		XMM((modrm >> 3) & 0x7).f64[0] = XMM((modrm >> 3) & 0x7).f64[0] / src.f64[0];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_maxsd_r128_r128m64() // Opcode f2 0f 5f
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).f64[0] = sse_max_double(XMM((modrm >> 3) & 0x7).f64[0], XMM(modrm & 0x7).f64[0]);
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, src);
		XMM((modrm >> 3) & 0x7).f64[0] = sse_max_double(XMM((modrm >> 3) & 0x7).f64[0], src.f64[0]);
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_haddps_r128_rm128() // Opcode f2 0f 7c
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		int s, d;
		float f1, f2, f3, f4;
		s=modrm & 0x7;
		d=(modrm >> 3) & 0x7;
		f1=XMM(d).f[0]+XMM(d).f[1];
		f2=XMM(d).f[2]+XMM(d).f[3];
		f3=XMM(s).f[0]+XMM(s).f[1];
		f4=XMM(s).f[2]+XMM(s).f[3];
		XMM(d).f[0]=f1;
		XMM(d).f[1]=f2;
		XMM(d).f[2]=f3;
		XMM(d).f[3]=f4;
	} else {
		XMM_REG src;
		int d;
		float f1, f2;
		UINT32 ea = GetEA(modrm, 0);
		d=(modrm >> 3) & 0x7;
		READXMM(ea, src);
		f1=XMM(d).f[0]+XMM(d).f[1];
		f2=XMM(d).f[2]+XMM(d).f[3];
		XMM(d).f[0]=f1;
		XMM(d).f[1]=f2;
		XMM(d).f[2]=src.f[0]+src.f[1];
		XMM(d).f[3]=src.f[2]+src.f[3];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_hsubps_r128_rm128() // Opcode f2 0f 7d
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		int s, d;
		float f1, f2, f3, f4;
		s=modrm & 0x7;
		d=(modrm >> 3) & 0x7;
		f1=XMM(d).f[0]-XMM(d).f[1];
		f2=XMM(d).f[2]-XMM(d).f[3];
		f3=XMM(s).f[0]-XMM(s).f[1];
		f4=XMM(s).f[2]-XMM(s).f[3];
		XMM(d).f[0]=f1;
		XMM(d).f[1]=f2;
		XMM(d).f[2]=f3;
		XMM(d).f[3]=f4;
	} else {
		XMM_REG src;
		int d;
		float f1, f2;
		UINT32 ea = GetEA(modrm, 0);
		d=(modrm >> 3) & 0x7;
		READXMM(ea, src);
		f1=XMM(d).f[0]-XMM(d).f[1];
		f2=XMM(d).f[2]-XMM(d).f[3];
		XMM(d).f[0]=f1;
		XMM(d).f[1]=f2;
		XMM(d).f[2]=src.f[0]-src.f[1];
		XMM(d).f[3]=src.f[2]-src.f[3];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_cmpsd_r128_r128m64_i8() // Opcode f2 0f c2
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		int s,d;
		UINT8 imm8 = FETCH();
		s=modrm & 0x7;
		d=(modrm >> 3) & 0x7;
		sse_predicate_compare_double_scalar(imm8, XMM(d), XMM(s));
	} else {
		int d;
		XMM_REG s;
		UINT32 ea = GetEA(modrm, 0);
		UINT8 imm8 = FETCH();
		READXMM_LO64(ea, s);
		d=(modrm >> 3) & 0x7;
		sse_predicate_compare_double_scalar(imm8, XMM(d), s);
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_addsubps_r128_rm128() // Opcode f2 0f d0
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).f[0]=XMM((modrm >> 3) & 0x7).f[0] - XMM(modrm & 0x7).f[0];
		XMM((modrm >> 3) & 0x7).f[1]=XMM((modrm >> 3) & 0x7).f[1] + XMM(modrm & 0x7).f[1];
		XMM((modrm >> 3) & 0x7).f[2]=XMM((modrm >> 3) & 0x7).f[2] - XMM(modrm & 0x7).f[2];
		XMM((modrm >> 3) & 0x7).f[3]=XMM((modrm >> 3) & 0x7).f[3] + XMM(modrm & 0x7).f[3];
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, src);
		XMM((modrm >> 3) & 0x7).f[0]=XMM((modrm >> 3) & 0x7).f[0] - src.f[0];
		XMM((modrm >> 3) & 0x7).f[1]=XMM((modrm >> 3) & 0x7).f[1] + src.f[1];
		XMM((modrm >> 3) & 0x7).f[2]=XMM((modrm >> 3) & 0x7).f[2] - src.f[2];
		XMM((modrm >> 3) & 0x7).f[3]=XMM((modrm >> 3) & 0x7).f[3] + src.f[3];
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_movdq2q_r64_r128() // Opcode f2 0f d6
{
	UINT8 modrm = FETCH();
	MMXPROLOG();
	if( modrm >= 0xc0 ) {
		MMX((modrm >> 3) & 0x7).q = XMM(modrm & 0x7).q[0];
		CYCLES(1);     // TODO: correct cycle count
	} else {
		// unsupported by cpu
		CYCLES(1);     // TODO: correct cycle count
	}
}

void i386_device::sse_cvtpd2dq_r128_rm128() // Opcode f2 0f e6
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).i[0]=(INT32)XMM((modrm >> 3) & 0x7).f64[0];
		XMM((modrm >> 3) & 0x7).i[1]=(INT32)XMM((modrm >> 3) & 0x7).f64[1];
		XMM((modrm >> 3) & 0x7).q[1] = 0;
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, src);
		XMM((modrm >> 3) & 0x7).i[0]=(INT32)src.f64[0];
		XMM((modrm >> 3) & 0x7).i[1]=(INT32)src.f64[1];
		XMM((modrm >> 3) & 0x7).q[1] = 0;
	}
	CYCLES(1);     // TODO: correct cycle count
}

void i386_device::sse_lddqu_r128_m128() // Opcode f2 0f f0
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		// unsupported by cpu
		CYCLES(1);     // TODO: correct cycle count
	} else {
		UINT32 ea = GetEA(modrm, 0);
		READXMM(ea, XMM((modrm >> 3) & 0x7));
	}
}
