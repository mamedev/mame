// Pentium+ specific opcodes

extern flag float32_is_nan( float32 a ); // since its not defined in softfloat.h

INLINE void MMXPROLOG(i386_state *cpustate)
{
	//cpustate->x87_sw &= ~(X87_SW_TOP_MASK << X87_SW_TOP_SHIFT); // top = 0
	cpustate->x87_tw = 0; // tag word = 0
}

INLINE void READMMX(i386_state *cpustate,UINT32 ea,MMX_REG &r)
{
	r.q=READ64(cpustate, ea);
}

INLINE void WRITEMMX(i386_state *cpustate,UINT32 ea,MMX_REG &r)
{
	WRITE64(cpustate, ea, r.q);
}

INLINE void READXMM(i386_state *cpustate,UINT32 ea,XMM_REG &r)
{
	r.q[0]=READ64(cpustate, ea);
	r.q[1]=READ64(cpustate, ea+8);
}

INLINE void WRITEXMM(i386_state *cpustate,UINT32 ea,XMM_REG &r)
{
	WRITE64(cpustate, ea, r.q[0]);
	WRITE64(cpustate, ea+8, r.q[1]);
}

INLINE void READXMM_LO64(i386_state *cpustate,UINT32 ea,XMM_REG &r)
{
	r.q[0]=READ64(cpustate, ea);
}

INLINE void WRITEXMM_LO64(i386_state *cpustate,UINT32 ea,XMM_REG &r)
{
	WRITE64(cpustate, ea, r.q[0]);
}

INLINE void READXMM_HI64(i386_state *cpustate,UINT32 ea,XMM_REG &r)
{
	r.q[1]=READ64(cpustate, ea);
}

INLINE void WRITEXMM_HI64(i386_state *cpustate,UINT32 ea,XMM_REG &r)
{
	WRITE64(cpustate, ea, r.q[1]);
}

static void PENTIUMOP(rdmsr)(i386_state *cpustate)          // Opcode 0x0f 32
{
	UINT64 data;
	UINT8 valid_msr = 0;

	data = MSR_READ(cpustate,REG32(ECX),&valid_msr);
	REG32(EDX) = data >> 32;
	REG32(EAX) = data & 0xffffffff;

	if(cpustate->CPL != 0 || valid_msr == 0) // if current privilege level isn't 0 or the register isn't recognized ...
		FAULT(FAULT_GP,0) // ... throw a general exception fault

	CYCLES(cpustate,CYCLES_RDMSR);
}

static void PENTIUMOP(wrmsr)(i386_state *cpustate)          // Opcode 0x0f 30
{
	UINT64 data;
	UINT8 valid_msr = 0;

	data = (UINT64)REG32(EAX);
	data |= (UINT64)(REG32(EDX)) << 32;

	MSR_WRITE(cpustate,REG32(ECX),data,&valid_msr);

	if(cpustate->CPL != 0 || valid_msr == 0) // if current privilege level isn't 0 or the register isn't recognized
		FAULT(FAULT_GP,0) // ... throw a general exception fault

	CYCLES(cpustate,1);     // TODO: correct cycle count (~30-45)
}

static void PENTIUMOP(rdtsc)(i386_state *cpustate)          // Opcode 0x0f 31
{
	UINT64 ts = cpustate->tsc + (cpustate->base_cycles - cpustate->cycles);
	REG32(EAX) = (UINT32)(ts);
	REG32(EDX) = (UINT32)(ts >> 32);

	CYCLES(cpustate,CYCLES_RDTSC);
}

static void PENTIUMOP(ud2)(i386_state *cpustate)    // Opcode 0x0f 0b
{
	i386_trap(cpustate, 6, 0, 0);
}

static void PENTIUMOP(rsm)(i386_state *cpustate)
{
	UINT32 smram_state = cpustate->smbase + 0xfe00;
	if(!cpustate->smm)
	{
		logerror("i386: Invalid RSM outside SMM at %08X\n", cpustate->pc - 1);
		i386_trap(cpustate, 6, 0, 0);
		return;
	}

	// load state, no sanity checks anywhere
	cpustate->smbase = READ32(cpustate, smram_state+SMRAM_SMBASE);
	cpustate->cr[4] = READ32(cpustate, smram_state+SMRAM_IP5_CR4);
	cpustate->sreg[ES].limit = READ32(cpustate, smram_state+SMRAM_IP5_ESLIM);
	cpustate->sreg[ES].base = READ32(cpustate, smram_state+SMRAM_IP5_ESBASE);
	cpustate->sreg[ES].flags = READ32(cpustate, smram_state+SMRAM_IP5_ESACC);
	cpustate->sreg[CS].limit = READ32(cpustate, smram_state+SMRAM_IP5_CSLIM);
	cpustate->sreg[CS].base = READ32(cpustate, smram_state+SMRAM_IP5_CSBASE);
	cpustate->sreg[CS].flags = READ32(cpustate, smram_state+SMRAM_IP5_CSACC);
	cpustate->sreg[SS].limit = READ32(cpustate, smram_state+SMRAM_IP5_SSLIM);
	cpustate->sreg[SS].base = READ32(cpustate, smram_state+SMRAM_IP5_SSBASE);
	cpustate->sreg[SS].flags = READ32(cpustate, smram_state+SMRAM_IP5_SSACC);
	cpustate->sreg[DS].limit = READ32(cpustate, smram_state+SMRAM_IP5_DSLIM);
	cpustate->sreg[DS].base = READ32(cpustate, smram_state+SMRAM_IP5_DSBASE);
	cpustate->sreg[DS].flags = READ32(cpustate, smram_state+SMRAM_IP5_DSACC);
	cpustate->sreg[FS].limit = READ32(cpustate, smram_state+SMRAM_IP5_FSLIM);
	cpustate->sreg[FS].base = READ32(cpustate, smram_state+SMRAM_IP5_FSBASE);
	cpustate->sreg[FS].flags = READ32(cpustate, smram_state+SMRAM_IP5_FSACC);
	cpustate->sreg[GS].limit = READ32(cpustate, smram_state+SMRAM_IP5_GSLIM);
	cpustate->sreg[GS].base = READ32(cpustate, smram_state+SMRAM_IP5_GSBASE);
	cpustate->sreg[GS].flags = READ32(cpustate, smram_state+SMRAM_IP5_GSACC);
	cpustate->ldtr.flags = READ32(cpustate, smram_state+SMRAM_IP5_LDTACC);
	cpustate->ldtr.limit = READ32(cpustate, smram_state+SMRAM_IP5_LDTLIM);
	cpustate->ldtr.base = READ32(cpustate, smram_state+SMRAM_IP5_LDTBASE);
	cpustate->gdtr.limit = READ32(cpustate, smram_state+SMRAM_IP5_GDTLIM);
	cpustate->gdtr.base = READ32(cpustate, smram_state+SMRAM_IP5_GDTBASE);
	cpustate->idtr.limit = READ32(cpustate, smram_state+SMRAM_IP5_IDTLIM);
	cpustate->idtr.base = READ32(cpustate, smram_state+SMRAM_IP5_IDTBASE);
	cpustate->task.limit = READ32(cpustate, smram_state+SMRAM_IP5_TRLIM);
	cpustate->task.base = READ32(cpustate, smram_state+SMRAM_IP5_TRBASE);
	cpustate->task.flags = READ32(cpustate, smram_state+SMRAM_IP5_TRACC);

	cpustate->sreg[ES].selector = READ32(cpustate, smram_state+SMRAM_ES);
	cpustate->sreg[CS].selector = READ32(cpustate, smram_state+SMRAM_CS);
	cpustate->sreg[SS].selector = READ32(cpustate, smram_state+SMRAM_SS);
	cpustate->sreg[DS].selector = READ32(cpustate, smram_state+SMRAM_DS);
	cpustate->sreg[FS].selector = READ32(cpustate, smram_state+SMRAM_FS);
	cpustate->sreg[GS].selector = READ32(cpustate, smram_state+SMRAM_GS);
	cpustate->ldtr.segment = READ32(cpustate, smram_state+SMRAM_LDTR);
	cpustate->task.segment = READ32(cpustate, smram_state+SMRAM_TR);

	cpustate->dr[7] = READ32(cpustate, smram_state+SMRAM_DR7);
	cpustate->dr[6] = READ32(cpustate, smram_state+SMRAM_DR6);
	REG32(EAX) = READ32(cpustate, smram_state+SMRAM_EAX);
	REG32(ECX) = READ32(cpustate, smram_state+SMRAM_ECX);
	REG32(EDX) = READ32(cpustate, smram_state+SMRAM_EDX);
	REG32(EBX) = READ32(cpustate, smram_state+SMRAM_EBX);
	REG32(ESP) = READ32(cpustate, smram_state+SMRAM_ESP);
	REG32(EBP) = READ32(cpustate, smram_state+SMRAM_EBP);
	REG32(ESI) = READ32(cpustate, smram_state+SMRAM_ESI);
	REG32(EDI) = READ32(cpustate, smram_state+SMRAM_EDI);
	cpustate->eip = READ32(cpustate, smram_state+SMRAM_EIP);
	cpustate->eflags = READ32(cpustate, smram_state+SMRAM_EAX);
	cpustate->cr[3] = READ32(cpustate, smram_state+SMRAM_CR3);
	cpustate->cr[0] = READ32(cpustate, smram_state+SMRAM_CR0);

	cpustate->CPL = (cpustate->sreg[SS].flags >> 13) & 3; // cpl == dpl of ss

	for(int i = 0; i < GS; i++)
	{
		if(PROTECTED_MODE && !V8086_MODE)
		{
			cpustate->sreg[i].valid = cpustate->sreg[i].selector ? true : false;
			cpustate->sreg[i].d = (cpustate->sreg[i].flags & 0x4000) ? 1 : 0;
		}
		else
			cpustate->sreg[i].valid = true;
	}

	if(!cpustate->smiact.isnull())
		cpustate->smiact(false);
	cpustate->smm = false;

	CHANGE_PC(cpustate,cpustate->eip);
	cpustate->nmi_masked = false;
	if(cpustate->smi_latched)
	{
		pentium_smi(cpustate);
		return;
	}
	if(cpustate->nmi_latched)
	{
		cpustate->nmi_latched = false;
		i386_trap(cpustate, 2, 1, 0);
	}
}

static void PENTIUMOP(prefetch_m8)(i386_state *cpustate)    // Opcode 0x0f 18
{
	UINT8 modrm = FETCH(cpustate);
	UINT32 ea = GetEA(cpustate,modrm,0);
	CYCLES(cpustate,1+(ea & 1)); // TODO: correct cycle count
}

static void PENTIUMOP(cmovo_r16_rm16)(i386_state *cpustate)    // Opcode 0x0f 40
{
	UINT16 src;
	UINT8 modrm = FETCH(cpustate);

	if( modrm >= 0xc0 )
	{
		if (cpustate->OF == 1)
		{
			src = LOAD_RM16(modrm);
			STORE_REG16(modrm, src);
		}
		CYCLES(cpustate,1); // TODO: correct cycle count
	}
	else
	{
		UINT32 ea = GetEA(cpustate,modrm,0);
		if (cpustate->OF == 1)
		{
			src = READ16(cpustate,ea);
			STORE_REG16(modrm, src);
		}
		CYCLES(cpustate,1); // TODO: correct cycle count
	}
}

static void PENTIUMOP(cmovo_r32_rm32)(i386_state *cpustate)    // Opcode 0x0f 40
{
	UINT32 src;
	UINT8 modrm = FETCH(cpustate);

	if( modrm >= 0xc0 )
	{
		if (cpustate->OF == 1)
		{
			src = LOAD_RM32(modrm);
			STORE_REG32(modrm, src);
		}
		CYCLES(cpustate,1); // TODO: correct cycle count
	}
	else
	{
		UINT32 ea = GetEA(cpustate,modrm,0);
		if (cpustate->OF == 1)
		{
			src = READ32(cpustate,ea);
			STORE_REG32(modrm, src);
		}
		CYCLES(cpustate,1); // TODO: correct cycle count
	}
}

static void PENTIUMOP(cmovno_r16_rm16)(i386_state *cpustate)    // Opcode 0x0f 41
{
	UINT16 src;
	UINT8 modrm = FETCH(cpustate);

	if( modrm >= 0xc0 )
	{
		if (cpustate->OF == 0)
		{
			src = LOAD_RM16(modrm);
			STORE_REG16(modrm, src);
		}
		CYCLES(cpustate,1); // TODO: correct cycle count
	}
	else
	{
		UINT32 ea = GetEA(cpustate,modrm,0);
		if (cpustate->OF == 0)
		{
			src = READ16(cpustate,ea);
			STORE_REG16(modrm, src);
		}
		CYCLES(cpustate,1); // TODO: correct cycle count
	}
}

static void PENTIUMOP(cmovno_r32_rm32)(i386_state *cpustate)    // Opcode 0x0f 41
{
	UINT32 src;
	UINT8 modrm = FETCH(cpustate);

	if( modrm >= 0xc0 )
	{
		if (cpustate->OF == 0)
		{
			src = LOAD_RM32(modrm);
			STORE_REG32(modrm, src);
		}
		CYCLES(cpustate,1); // TODO: correct cycle count
	}
	else
	{
		UINT32 ea = GetEA(cpustate,modrm,0);
		if (cpustate->OF == 0)
		{
			src = READ32(cpustate,ea);
			STORE_REG32(modrm, src);
		}
		CYCLES(cpustate,1); // TODO: correct cycle count
	}
}

static void PENTIUMOP(cmovb_r16_rm16)(i386_state *cpustate)    // Opcode 0x0f 42
{
	UINT16 src;
	UINT8 modrm = FETCH(cpustate);

	if( modrm >= 0xc0 )
	{
		if (cpustate->CF == 1)
		{
			src = LOAD_RM16(modrm);
			STORE_REG16(modrm, src);
		}
		CYCLES(cpustate,1); // TODO: correct cycle count
	}
	else
	{
		UINT32 ea = GetEA(cpustate,modrm,0);
		if (cpustate->CF == 1)
		{
			src = READ16(cpustate,ea);
			STORE_REG16(modrm, src);
		}
		CYCLES(cpustate,1); // TODO: correct cycle count
	}
}

static void PENTIUMOP(cmovb_r32_rm32)(i386_state *cpustate)    // Opcode 0x0f 42
{
	UINT32 src;
	UINT8 modrm = FETCH(cpustate);

	if( modrm >= 0xc0 )
	{
		if (cpustate->CF == 1)
		{
			src = LOAD_RM32(modrm);
			STORE_REG32(modrm, src);
		}
		CYCLES(cpustate,1); // TODO: correct cycle count
	}
	else
	{
		UINT32 ea = GetEA(cpustate,modrm,0);
		if (cpustate->CF == 1)
		{
			src = READ32(cpustate,ea);
			STORE_REG32(modrm, src);
		}
		CYCLES(cpustate,1); // TODO: correct cycle count
	}
}

static void PENTIUMOP(cmovae_r16_rm16)(i386_state *cpustate)    // Opcode 0x0f 43
{
	UINT16 src;
	UINT8 modrm = FETCH(cpustate);

	if( modrm >= 0xc0 )
	{
		if (cpustate->CF == 0)
		{
			src = LOAD_RM16(modrm);
			STORE_REG16(modrm, src);
		}
		CYCLES(cpustate,1); // TODO: correct cycle count
	}
	else
	{
		UINT32 ea = GetEA(cpustate,modrm,0);
		if (cpustate->CF == 0)
		{
			src = READ16(cpustate,ea);
			STORE_REG16(modrm, src);
		}
		CYCLES(cpustate,1); // TODO: correct cycle count
	}
}

static void PENTIUMOP(cmovae_r32_rm32)(i386_state *cpustate)    // Opcode 0x0f 43
{
	UINT32 src;
	UINT8 modrm = FETCH(cpustate);

	if( modrm >= 0xc0 )
	{
		if (cpustate->CF == 0)
		{
			src = LOAD_RM32(modrm);
			STORE_REG32(modrm, src);
		}
		CYCLES(cpustate,1); // TODO: correct cycle count
	}
	else
	{
		UINT32 ea = GetEA(cpustate,modrm,0);
		if (cpustate->CF == 0)
		{
			src = READ32(cpustate,ea);
			STORE_REG32(modrm, src);
		}
		CYCLES(cpustate,1); // TODO: correct cycle count
	}
}

static void PENTIUMOP(cmove_r16_rm16)(i386_state *cpustate)    // Opcode 0x0f 44
{
	UINT16 src;
	UINT8 modrm = FETCH(cpustate);

	if( modrm >= 0xc0 )
	{
		if (cpustate->ZF == 1)
		{
			src = LOAD_RM16(modrm);
			STORE_REG16(modrm, src);
		}
		CYCLES(cpustate,1); // TODO: correct cycle count
	}
	else
	{
		UINT32 ea = GetEA(cpustate,modrm,0);
		if (cpustate->ZF == 1)
		{
			src = READ16(cpustate,ea);
			STORE_REG16(modrm, src);
		}
		CYCLES(cpustate,1); // TODO: correct cycle count
	}
}

static void PENTIUMOP(cmove_r32_rm32)(i386_state *cpustate)    // Opcode 0x0f 44
{
	UINT32 src;
	UINT8 modrm = FETCH(cpustate);

	if( modrm >= 0xc0 )
	{
		if (cpustate->ZF == 1)
		{
			src = LOAD_RM32(modrm);
			STORE_REG32(modrm, src);
		}
		CYCLES(cpustate,1); // TODO: correct cycle count
	}
	else
	{
		UINT32 ea = GetEA(cpustate,modrm,0);
		if (cpustate->ZF == 1)
		{
			src = READ32(cpustate,ea);
			STORE_REG32(modrm, src);
		}
		CYCLES(cpustate,1); // TODO: correct cycle count
	}
}

static void PENTIUMOP(cmovne_r16_rm16)(i386_state *cpustate)    // Opcode 0x0f 45
{
	UINT16 src;
	UINT8 modrm = FETCH(cpustate);

	if( modrm >= 0xc0 )
	{
		if (cpustate->ZF == 0)
		{
			src = LOAD_RM16(modrm);
			STORE_REG16(modrm, src);
		}
		CYCLES(cpustate,1); // TODO: correct cycle count
	}
	else
	{
		UINT32 ea = GetEA(cpustate,modrm,0);
		if (cpustate->ZF == 0)
		{
			src = READ16(cpustate,ea);
			STORE_REG16(modrm, src);
		}
		CYCLES(cpustate,1); // TODO: correct cycle count
	}
}

static void PENTIUMOP(cmovne_r32_rm32)(i386_state *cpustate)    // Opcode 0x0f 45
{
	UINT32 src;
	UINT8 modrm = FETCH(cpustate);

	if( modrm >= 0xc0 )
	{
		if (cpustate->ZF == 0)
		{
			src = LOAD_RM32(modrm);
			STORE_REG32(modrm, src);
		}
		CYCLES(cpustate,1); // TODO: correct cycle count
	}
	else
	{
		UINT32 ea = GetEA(cpustate,modrm,0);
		if (cpustate->ZF == 0)
		{
			src = READ32(cpustate,ea);
			STORE_REG32(modrm, src);
		}
		CYCLES(cpustate,1); // TODO: correct cycle count
	}
}

static void PENTIUMOP(cmovbe_r16_rm16)(i386_state *cpustate)    // Opcode 0x0f 46
{
	UINT16 src;
	UINT8 modrm = FETCH(cpustate);

	if( modrm >= 0xc0 )
	{
		if ((cpustate->CF == 1) || (cpustate->ZF == 1))
		{
			src = LOAD_RM16(modrm);
			STORE_REG16(modrm, src);
		}
		CYCLES(cpustate,1); // TODO: correct cycle count
	}
	else
	{
		UINT32 ea = GetEA(cpustate,modrm,0);
		if ((cpustate->CF == 1) || (cpustate->ZF == 1))
		{
			src = READ16(cpustate,ea);
			STORE_REG16(modrm, src);
		}
		CYCLES(cpustate,1); // TODO: correct cycle count
	}
}

static void PENTIUMOP(cmovbe_r32_rm32)(i386_state *cpustate)    // Opcode 0x0f 46
{
	UINT32 src;
	UINT8 modrm = FETCH(cpustate);

	if( modrm >= 0xc0 )
	{
		if ((cpustate->CF == 1) || (cpustate->ZF == 1))
		{
			src = LOAD_RM32(modrm);
			STORE_REG32(modrm, src);
		}
		CYCLES(cpustate,1); // TODO: correct cycle count
	}
	else
	{
		UINT32 ea = GetEA(cpustate,modrm,0);
		if ((cpustate->CF == 1) || (cpustate->ZF == 1))
		{
			src = READ32(cpustate,ea);
			STORE_REG32(modrm, src);
		}
		CYCLES(cpustate,1); // TODO: correct cycle count
	}
}

static void PENTIUMOP(cmova_r16_rm16)(i386_state *cpustate)    // Opcode 0x0f 47
{
	UINT16 src;
	UINT8 modrm = FETCH(cpustate);

	if( modrm >= 0xc0 )
	{
		if ((cpustate->CF == 0) && (cpustate->ZF == 0))
		{
			src = LOAD_RM16(modrm);
			STORE_REG16(modrm, src);
		}
		CYCLES(cpustate,1); // TODO: correct cycle count
	}
	else
	{
		UINT32 ea = GetEA(cpustate,modrm,0);
		if ((cpustate->CF == 0) && (cpustate->ZF == 0))
		{
			src = READ16(cpustate,ea);
			STORE_REG16(modrm, src);
		}
		CYCLES(cpustate,1); // TODO: correct cycle count
	}
}

static void PENTIUMOP(cmova_r32_rm32)(i386_state *cpustate)    // Opcode 0x0f 47
{
	UINT32 src;
	UINT8 modrm = FETCH(cpustate);

	if( modrm >= 0xc0 )
	{
		if ((cpustate->CF == 0) && (cpustate->ZF == 0))
		{
			src = LOAD_RM32(modrm);
			STORE_REG32(modrm, src);
		}
		CYCLES(cpustate,1); // TODO: correct cycle count
	}
	else
	{
		UINT32 ea = GetEA(cpustate,modrm,0);
		if ((cpustate->CF == 0) && (cpustate->ZF == 0))
		{
			src = READ32(cpustate,ea);
			STORE_REG32(modrm, src);
		}
		CYCLES(cpustate,1); // TODO: correct cycle count
	}
}

static void PENTIUMOP(cmovs_r16_rm16)(i386_state *cpustate)    // Opcode 0x0f 48
{
	UINT16 src;
	UINT8 modrm = FETCH(cpustate);

	if( modrm >= 0xc0 )
	{
		if (cpustate->SF == 1)
		{
			src = LOAD_RM16(modrm);
			STORE_REG16(modrm, src);
		}
		CYCLES(cpustate,1); // TODO: correct cycle count
	}
	else
	{
		UINT32 ea = GetEA(cpustate,modrm,0);
		if (cpustate->SF == 1)
		{
			src = READ16(cpustate,ea);
			STORE_REG16(modrm, src);
		}
		CYCLES(cpustate,1); // TODO: correct cycle count
	}
}

static void PENTIUMOP(cmovs_r32_rm32)(i386_state *cpustate)    // Opcode 0x0f 48
{
	UINT32 src;
	UINT8 modrm = FETCH(cpustate);

	if( modrm >= 0xc0 )
	{
		if (cpustate->SF == 1)
		{
			src = LOAD_RM32(modrm);
			STORE_REG32(modrm, src);
		}
		CYCLES(cpustate,1); // TODO: correct cycle count
	}
	else
	{
		UINT32 ea = GetEA(cpustate,modrm,0);
		if (cpustate->SF == 1)
		{
			src = READ32(cpustate,ea);
			STORE_REG32(modrm, src);
		}
		CYCLES(cpustate,1); // TODO: correct cycle count
	}
}

static void PENTIUMOP(cmovns_r16_rm16)(i386_state *cpustate)    // Opcode 0x0f 49
{
	UINT16 src;
	UINT8 modrm = FETCH(cpustate);

	if( modrm >= 0xc0 )
	{
		if (cpustate->SF == 0)
		{
			src = LOAD_RM16(modrm);
			STORE_REG16(modrm, src);
		}
		CYCLES(cpustate,1); // TODO: correct cycle count
	}
	else
	{
		UINT32 ea = GetEA(cpustate,modrm,0);
		if (cpustate->SF == 0)
		{
			src = READ16(cpustate,ea);
			STORE_REG16(modrm, src);
		}
		CYCLES(cpustate,1); // TODO: correct cycle count
	}
}

static void PENTIUMOP(cmovns_r32_rm32)(i386_state *cpustate)    // Opcode 0x0f 49
{
	UINT32 src;
	UINT8 modrm = FETCH(cpustate);

	if( modrm >= 0xc0 )
	{
		if (cpustate->SF == 0)
		{
			src = LOAD_RM32(modrm);
			STORE_REG32(modrm, src);
		}
		CYCLES(cpustate,1); // TODO: correct cycle count
	}
	else
	{
		UINT32 ea = GetEA(cpustate,modrm,0);
		if (cpustate->SF == 0)
		{
			src = READ32(cpustate,ea);
			STORE_REG32(modrm, src);
		}
		CYCLES(cpustate,1); // TODO: correct cycle count
	}
}

static void PENTIUMOP(cmovp_r16_rm16)(i386_state *cpustate)    // Opcode 0x0f 4a
{
	UINT16 src;
	UINT8 modrm = FETCH(cpustate);

	if( modrm >= 0xc0 )
	{
		if (cpustate->PF == 1)
		{
			src = LOAD_RM16(modrm);
			STORE_REG16(modrm, src);
		}
		CYCLES(cpustate,1); // TODO: correct cycle count
	}
	else
	{
		UINT32 ea = GetEA(cpustate,modrm,0);
		if (cpustate->PF == 1)
		{
			src = READ16(cpustate,ea);
			STORE_REG16(modrm, src);
		}
		CYCLES(cpustate,1); // TODO: correct cycle count
	}
}

static void PENTIUMOP(cmovp_r32_rm32)(i386_state *cpustate)    // Opcode 0x0f 4a
{
	UINT32 src;
	UINT8 modrm = FETCH(cpustate);

	if( modrm >= 0xc0 )
	{
		if (cpustate->PF == 1)
		{
			src = LOAD_RM32(modrm);
			STORE_REG32(modrm, src);
		}
		CYCLES(cpustate,1); // TODO: correct cycle count
	}
	else
	{
		UINT32 ea = GetEA(cpustate,modrm,0);
		if (cpustate->PF == 1)
		{
			src = READ32(cpustate,ea);
			STORE_REG32(modrm, src);
		}
		CYCLES(cpustate,1); // TODO: correct cycle count
	}
}

static void PENTIUMOP(cmovnp_r16_rm16)(i386_state *cpustate)    // Opcode 0x0f 4b
{
	UINT16 src;
	UINT8 modrm = FETCH(cpustate);

	if( modrm >= 0xc0 )
	{
		if (cpustate->PF == 0)
		{
			src = LOAD_RM16(modrm);
			STORE_REG16(modrm, src);
		}
		CYCLES(cpustate,1); // TODO: correct cycle count
	}
	else
	{
		UINT32 ea = GetEA(cpustate,modrm,0);
		if (cpustate->PF == 0)
		{
			src = READ16(cpustate,ea);
			STORE_REG16(modrm, src);
		}
		CYCLES(cpustate,1); // TODO: correct cycle count
	}
}

static void PENTIUMOP(cmovnp_r32_rm32)(i386_state *cpustate)    // Opcode 0x0f 4b
{
	UINT32 src;
	UINT8 modrm = FETCH(cpustate);

	if( modrm >= 0xc0 )
	{
		if (cpustate->PF == 0)
		{
			src = LOAD_RM32(modrm);
			STORE_REG32(modrm, src);
		}
		CYCLES(cpustate,1); // TODO: correct cycle count
	}
	else
	{
		UINT32 ea = GetEA(cpustate,modrm,0);
		if (cpustate->PF == 0)
		{
			src = READ32(cpustate,ea);
			STORE_REG32(modrm, src);
		}
		CYCLES(cpustate,1); // TODO: correct cycle count
	}
}

static void PENTIUMOP(cmovl_r16_rm16)(i386_state *cpustate)    // Opcode 0x0f 4c
{
	UINT16 src;
	UINT8 modrm = FETCH(cpustate);

	if( modrm >= 0xc0 )
	{
		if (cpustate->SF != cpustate->OF)
		{
			src = LOAD_RM16(modrm);
			STORE_REG16(modrm, src);
		}
		CYCLES(cpustate,1); // TODO: correct cycle count
	}
	else
	{
		UINT32 ea = GetEA(cpustate,modrm,0);
		if (cpustate->SF != cpustate->OF)
		{
			src = READ16(cpustate,ea);
			STORE_REG16(modrm, src);
		}
		CYCLES(cpustate,1); // TODO: correct cycle count
	}
}

static void PENTIUMOP(cmovl_r32_rm32)(i386_state *cpustate)    // Opcode 0x0f 4c
{
	UINT32 src;
	UINT8 modrm = FETCH(cpustate);

	if( modrm >= 0xc0 )
	{
		if (cpustate->SF != cpustate->OF)
		{
			src = LOAD_RM32(modrm);
			STORE_REG32(modrm, src);
		}
		CYCLES(cpustate,1); // TODO: correct cycle count
	}
	else
	{
		UINT32 ea = GetEA(cpustate,modrm,0);
		if (cpustate->SF != cpustate->OF)
		{
			src = READ32(cpustate,ea);
			STORE_REG32(modrm, src);
		}
		CYCLES(cpustate,1); // TODO: correct cycle count
	}
}

static void PENTIUMOP(cmovge_r16_rm16)(i386_state *cpustate)    // Opcode 0x0f 4d
{
	UINT16 src;
	UINT8 modrm = FETCH(cpustate);

	if( modrm >= 0xc0 )
	{
		if (cpustate->SF == cpustate->OF)
		{
			src = LOAD_RM16(modrm);
			STORE_REG16(modrm, src);
		}
		CYCLES(cpustate,1); // TODO: correct cycle count
	}
	else
	{
		UINT32 ea = GetEA(cpustate,modrm,0);
		if (cpustate->SF == cpustate->OF)
		{
			src = READ16(cpustate,ea);
			STORE_REG16(modrm, src);
		}
		CYCLES(cpustate,1); // TODO: correct cycle count
	}
}

static void PENTIUMOP(cmovge_r32_rm32)(i386_state *cpustate)    // Opcode 0x0f 4d
{
	UINT32 src;
	UINT8 modrm = FETCH(cpustate);

	if( modrm >= 0xc0 )
	{
		if (cpustate->SF == cpustate->OF)
		{
			src = LOAD_RM32(modrm);
			STORE_REG32(modrm, src);
		}
		CYCLES(cpustate,1); // TODO: correct cycle count
	}
	else
	{
		UINT32 ea = GetEA(cpustate,modrm,0);
		if (cpustate->SF == cpustate->OF)
		{
			src = READ32(cpustate,ea);
			STORE_REG32(modrm, src);
		}
		CYCLES(cpustate,1); // TODO: correct cycle count
	}
}

static void PENTIUMOP(cmovle_r16_rm16)(i386_state *cpustate)    // Opcode 0x0f 4e
{
	UINT16 src;
	UINT8 modrm = FETCH(cpustate);

	if( modrm >= 0xc0 )
	{
		if ((cpustate->ZF == 1) || (cpustate->SF != cpustate->OF))
		{
			src = LOAD_RM16(modrm);
			STORE_REG16(modrm, src);
		}
		CYCLES(cpustate,1); // TODO: correct cycle count
	}
	else
	{
		UINT32 ea = GetEA(cpustate,modrm,0);
		if ((cpustate->ZF == 1) || (cpustate->SF != cpustate->OF))
		{
			src = READ16(cpustate,ea);
			STORE_REG16(modrm, src);
		}
		CYCLES(cpustate,1); // TODO: correct cycle count
	}
}

static void PENTIUMOP(cmovle_r32_rm32)(i386_state *cpustate)    // Opcode 0x0f 4e
{
	UINT32 src;
	UINT8 modrm = FETCH(cpustate);

	if( modrm >= 0xc0 )
	{
		if ((cpustate->ZF == 1) || (cpustate->SF != cpustate->OF))
		{
			src = LOAD_RM32(modrm);
			STORE_REG32(modrm, src);
		}
		CYCLES(cpustate,1); // TODO: correct cycle count
	}
	else
	{
		UINT32 ea = GetEA(cpustate,modrm,0);
		if ((cpustate->ZF == 1) || (cpustate->SF != cpustate->OF))
		{
			src = READ32(cpustate,ea);
			STORE_REG32(modrm, src);
		}
		CYCLES(cpustate,1); // TODO: correct cycle count
	}
}

static void PENTIUMOP(cmovg_r16_rm16)(i386_state *cpustate)    // Opcode 0x0f 4f
{
	UINT16 src;
	UINT8 modrm = FETCH(cpustate);

	if( modrm >= 0xc0 )
	{
		if ((cpustate->ZF == 0) && (cpustate->SF == cpustate->OF))
		{
			src = LOAD_RM16(modrm);
			STORE_REG16(modrm, src);
		}
		CYCLES(cpustate,1); // TODO: correct cycle count
	}
	else
	{
		UINT32 ea = GetEA(cpustate,modrm,0);
		if ((cpustate->ZF == 0) && (cpustate->SF == cpustate->OF))
		{
			src = READ16(cpustate,ea);
			STORE_REG16(modrm, src);
		}
		CYCLES(cpustate,1); // TODO: correct cycle count
	}
}

static void PENTIUMOP(cmovg_r32_rm32)(i386_state *cpustate)    // Opcode 0x0f 4f
{
	UINT32 src;
	UINT8 modrm = FETCH(cpustate);

	if( modrm >= 0xc0 )
	{
		if ((cpustate->ZF == 0) && (cpustate->SF == cpustate->OF))
		{
			src = LOAD_RM32(modrm);
			STORE_REG32(modrm, src);
		}
		CYCLES(cpustate,1); // TODO: correct cycle count
	}
	else
	{
		UINT32 ea = GetEA(cpustate,modrm,0);
		if ((cpustate->ZF == 0) && (cpustate->SF == cpustate->OF))
		{
			src = READ32(cpustate,ea);
			STORE_REG32(modrm, src);
		}
		CYCLES(cpustate,1); // TODO: correct cycle count
	}
}

static void PENTIUMOP(movnti_m16_r16)(i386_state *cpustate) // Opcode 0f c3
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		// unsupported by cpu
		CYCLES(cpustate,1);     // TODO: correct cycle count
	} else {
		// since cache is not implemented
		UINT32 ea = GetEA(cpustate, modrm, 0);
		WRITE16(cpustate,ea,LOAD_RM16(modrm));
		CYCLES(cpustate,1);     // TODO: correct cycle count
	}
}

static void PENTIUMOP(movnti_m32_r32)(i386_state *cpustate) // Opcode 0f c3
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		// unsupported by cpu
		CYCLES(cpustate,1);     // TODO: correct cycle count
	} else {
		// since cache is not implemented
		UINT32 ea = GetEA(cpustate, modrm, 0);
		WRITE32(cpustate,ea,LOAD_RM32(modrm));
		CYCLES(cpustate,1);     // TODO: correct cycle count
	}
}

static void I386OP(cyrix_unknown)(i386_state *cpustate)     // Opcode 0x0f 74
{
	logerror("Unemulated 0x0f 0x74 opcode called\n");

	CYCLES(cpustate,1);
}

static void PENTIUMOP(cmpxchg8b_m64)(i386_state *cpustate)  // Opcode 0x0f c7
{
	UINT8 modm = FETCH(cpustate);
	if( modm >= 0xc0 ) {
		report_invalid_modrm(cpustate, "cmpxchg8b_m64", modm);
	} else {
		UINT32 ea = GetEA(cpustate, modm, 0);
		UINT64 value = READ64(cpustate,ea);
		UINT64 edx_eax = (((UINT64) REG32(EDX)) << 32) | REG32(EAX);
		UINT64 ecx_ebx = (((UINT64) REG32(ECX)) << 32) | REG32(EBX);

		if( value == edx_eax ) {
			WRITE64(cpustate,ea, ecx_ebx);
			cpustate->ZF = 1;
			CYCLES(cpustate,CYCLES_CMPXCHG_REG_MEM_T);
		} else {
			REG32(EDX) = (UINT32) (value >> 32);
			REG32(EAX) = (UINT32) (value >>  0);
			cpustate->ZF = 0;
			CYCLES(cpustate,CYCLES_CMPXCHG_REG_MEM_F);
		}
	}
}

static void PENTIUMOP(movntq_m64_r64)(i386_state *cpustate) // Opcode 0f e7
{
	//MMXPROLOG(cpustate); // TODO: check if needed
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		CYCLES(cpustate,1);     // unsupported
	} else {
		// since cache is not implemented
		UINT32 ea = GetEA(cpustate, modrm, 0);
		WRITEMMX(cpustate, ea, MMX((modrm >> 3) & 0x7));
		CYCLES(cpustate,1);     // TODO: correct cycle count
	}
}

static void PENTIUMOP(maskmovq_r64_r64)(i386_state *cpustate)  // Opcode 0f f7
{
	int s,m,n;
	UINT8 modm = FETCH(cpustate);
	UINT32 ea = GetEA(cpustate, 7, 0); // ds:di/edi/rdi register
	MMXPROLOG(cpustate);
	s=(modm >> 3) & 7;
	m=modm & 7;
	for (n=0;n <= 7;n++)
		if (MMX(m).b[n] & 127)
			WRITE8(cpustate, ea+n, MMX(s).b[n]);
}

static void PENTIUMOP(popcnt_r16_rm16)(i386_state *cpustate)    // Opcode f3 0f b8
{
	UINT16 src;
	UINT8 modrm = FETCH(cpustate);
	int n,count;

	if( modrm >= 0xc0 ) {
		src = LOAD_RM16(modrm);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,0);
		src = READ16(cpustate,ea);
	}
	count=0;
	for (n=0;n < 16;n++) {
		count=count+(src & 1);
		src=src >> 1;
	}
	STORE_REG16(modrm, count);
	CYCLES(cpustate,1); // TODO: correct cycle count
}

static void PENTIUMOP(popcnt_r32_rm32)(i386_state *cpustate)    // Opcode f3 0f b8
{
	UINT32 src;
	UINT8 modrm = FETCH(cpustate);
	int n,count;

	if( modrm >= 0xc0 ) {
		src = LOAD_RM32(modrm);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,0);
		src = READ32(cpustate,ea);
	}
	count=0;
	for (n=0;n < 32;n++) {
		count=count+(src & 1);
		src=src >> 1;
	}
	STORE_REG32(modrm, count);
	CYCLES(cpustate,1); // TODO: correct cycle count
}

static void PENTIUMOP(tzcnt_r16_rm16)(i386_state *cpustate)
{
	// for CPUs that don't support TZCNT, fall back to BSF
	i386_bsf_r16_rm16(cpustate);
	// TODO: actually implement TZCNT
}

static void PENTIUMOP(tzcnt_r32_rm32)(i386_state *cpustate)
{
	// for CPUs that don't support TZCNT, fall back to BSF
	i386_bsf_r32_rm32(cpustate);
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

static void MMXOP(group_0f71)(i386_state *cpustate)  // Opcode 0f 71
{
	UINT8 modm = FETCH(cpustate);
	UINT8 imm8 = FETCH(cpustate);
	MMXPROLOG(cpustate);
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
				report_invalid_modrm(cpustate, "mmx_group0f71", modm);
		}
	}
}

static void MMXOP(group_0f72)(i386_state *cpustate)  // Opcode 0f 72
{
	UINT8 modm = FETCH(cpustate);
	UINT8 imm8 = FETCH(cpustate);
	MMXPROLOG(cpustate);
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
				report_invalid_modrm(cpustate, "mmx_group0f72", modm);
		}
	}
}

static void MMXOP(group_0f73)(i386_state *cpustate)  // Opcode 0f 73
{
	UINT8 modm = FETCH(cpustate);
	UINT8 imm8 = FETCH(cpustate);
	MMXPROLOG(cpustate);
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
				report_invalid_modrm(cpustate, "mmx_group0f73", modm);
		}
	}
}

static void MMXOP(psrlw_r64_rm64)(i386_state *cpustate)  // Opcode 0f d1
{
	MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		int count=(int)MMX(modrm & 7).q;
		MMX((modrm >> 3) & 0x7).w[0]=MMX((modrm >> 3) & 0x7).w[0] >> count;
		MMX((modrm >> 3) & 0x7).w[1]=MMX((modrm >> 3) & 0x7).w[1] >> count;
		MMX((modrm >> 3) & 0x7).w[2]=MMX((modrm >> 3) & 0x7).w[2] >> count;
		MMX((modrm >> 3) & 0x7).w[3]=MMX((modrm >> 3) & 0x7).w[3] >> count;
	} else {
		MMX_REG src;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READMMX(cpustate, ea, src);
		int count=(int)src.q;
		MMX((modrm >> 3) & 0x7).w[0]=MMX((modrm >> 3) & 0x7).w[0] >> count;
		MMX((modrm >> 3) & 0x7).w[1]=MMX((modrm >> 3) & 0x7).w[1] >> count;
		MMX((modrm >> 3) & 0x7).w[2]=MMX((modrm >> 3) & 0x7).w[2] >> count;
		MMX((modrm >> 3) & 0x7).w[3]=MMX((modrm >> 3) & 0x7).w[3] >> count;
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void MMXOP(psrld_r64_rm64)(i386_state *cpustate)  // Opcode 0f d2
{
	MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		int count=(int)MMX(modrm & 7).q;
		MMX((modrm >> 3) & 0x7).d[0]=MMX((modrm >> 3) & 0x7).d[0] >> count;
		MMX((modrm >> 3) & 0x7).d[1]=MMX((modrm >> 3) & 0x7).d[1] >> count;
	} else {
		MMX_REG src;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READMMX(cpustate, ea, src);
		int count=(int)src.q;
		MMX((modrm >> 3) & 0x7).d[0]=MMX((modrm >> 3) & 0x7).d[0] >> count;
		MMX((modrm >> 3) & 0x7).d[1]=MMX((modrm >> 3) & 0x7).d[1] >> count;
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void MMXOP(psrlq_r64_rm64)(i386_state *cpustate)  // Opcode 0f d3
{
	MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		int count=(int)MMX(modrm & 7).q;
		MMX((modrm >> 3) & 0x7).q=MMX((modrm >> 3) & 0x7).q >> count;
	} else {
		MMX_REG src;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READMMX(cpustate, ea, src);
		int count=(int)src.q;
		MMX((modrm >> 3) & 0x7).q=MMX((modrm >> 3) & 0x7).q >> count;
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void MMXOP(paddq_r64_rm64)(i386_state *cpustate)  // Opcode 0f d4
{
	MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		MMX((modrm >> 3) & 0x7).q=MMX((modrm >> 3) & 0x7).q+MMX(modrm & 7).q;
	} else {
		MMX_REG src;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READMMX(cpustate, ea, src);
		MMX((modrm >> 3) & 0x7).q=MMX((modrm >> 3) & 0x7).q+src.q;
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void MMXOP(pmullw_r64_rm64)(i386_state *cpustate)  // Opcode 0f d5
{
	MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		MMX((modrm >> 3) & 0x7).w[0]=(UINT32)((INT32)MMX((modrm >> 3) & 0x7).s[0]*(INT32)MMX(modrm & 7).s[0]) & 0xffff;
		MMX((modrm >> 3) & 0x7).w[1]=(UINT32)((INT32)MMX((modrm >> 3) & 0x7).s[1]*(INT32)MMX(modrm & 7).s[1]) & 0xffff;
		MMX((modrm >> 3) & 0x7).w[2]=(UINT32)((INT32)MMX((modrm >> 3) & 0x7).s[2]*(INT32)MMX(modrm & 7).s[2]) & 0xffff;
		MMX((modrm >> 3) & 0x7).w[3]=(UINT32)((INT32)MMX((modrm >> 3) & 0x7).s[3]*(INT32)MMX(modrm & 7).s[3]) & 0xffff;
	} else {
		MMX_REG src;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READMMX(cpustate, ea, src);
		MMX((modrm >> 3) & 0x7).w[0]=(UINT32)((INT32)MMX((modrm >> 3) & 0x7).s[0]*(INT32)src.s[0]) & 0xffff;
		MMX((modrm >> 3) & 0x7).w[1]=(UINT32)((INT32)MMX((modrm >> 3) & 0x7).s[1]*(INT32)src.s[1]) & 0xffff;
		MMX((modrm >> 3) & 0x7).w[2]=(UINT32)((INT32)MMX((modrm >> 3) & 0x7).s[2]*(INT32)src.s[2]) & 0xffff;
		MMX((modrm >> 3) & 0x7).w[3]=(UINT32)((INT32)MMX((modrm >> 3) & 0x7).s[3]*(INT32)src.s[3]) & 0xffff;
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void MMXOP(psubusb_r64_rm64)(i386_state *cpustate)  // Opcode 0f d8
{
	int n;
	MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		for (n=0;n < 8;n++)
			MMX((modrm >> 3) & 0x7).b[n]=MMX((modrm >> 3) & 0x7).b[n] < MMX(modrm & 7).b[n] ? 0 : MMX((modrm >> 3) & 0x7).b[n]-MMX(modrm & 7).b[n];
	} else {
		MMX_REG src;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READMMX(cpustate, ea, src);
		for (n=0;n < 8;n++)
			MMX((modrm >> 3) & 0x7).b[n]=MMX((modrm >> 3) & 0x7).b[n] < src.b[n] ? 0 : MMX((modrm >> 3) & 0x7).b[n]-src.b[n];
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void MMXOP(psubusw_r64_rm64)(i386_state *cpustate)  // Opcode 0f d9
{
	int n;
	MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		for (n=0;n < 4;n++)
			MMX((modrm >> 3) & 0x7).w[n]=MMX((modrm >> 3) & 0x7).w[n] < MMX(modrm & 7).w[n] ? 0 : MMX((modrm >> 3) & 0x7).w[n]-MMX(modrm & 7).w[n];
	} else {
		MMX_REG src;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READMMX(cpustate, ea, src);
		for (n=0;n < 4;n++)
			MMX((modrm >> 3) & 0x7).w[n]=MMX((modrm >> 3) & 0x7).w[n] < src.w[n] ? 0 : MMX((modrm >> 3) & 0x7).w[n]-src.w[n];
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void MMXOP(pand_r64_rm64)(i386_state *cpustate)  // Opcode 0f db
{
	MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		MMX((modrm >> 3) & 0x7).q=MMX((modrm >> 3) & 0x7).q & MMX(modrm & 7).q;
	} else {
		MMX_REG src;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READMMX(cpustate, ea, src);
		MMX((modrm >> 3) & 0x7).q=MMX((modrm >> 3) & 0x7).q & src.q;
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void MMXOP(paddusb_r64_rm64)(i386_state *cpustate)  // Opcode 0f dc
{
	int n;
	MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		for (n=0;n < 8;n++)
			MMX((modrm >> 3) & 0x7).b[n]=MMX((modrm >> 3) & 0x7).b[n] > (0xff-MMX(modrm & 7).b[n]) ? 0xff : MMX((modrm >> 3) & 0x7).b[n]+MMX(modrm & 7).b[n];
	} else {
		MMX_REG src;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READMMX(cpustate, ea, src);
		for (n=0;n < 8;n++)
			MMX((modrm >> 3) & 0x7).b[n]=MMX((modrm >> 3) & 0x7).b[n] > (0xff-src.b[n]) ? 0xff : MMX((modrm >> 3) & 0x7).b[n]+src.b[n];
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void MMXOP(paddusw_r64_rm64)(i386_state *cpustate)  // Opcode 0f dd
{
	int n;
	MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		for (n=0;n < 4;n++)
			MMX((modrm >> 3) & 0x7).w[n]=MMX((modrm >> 3) & 0x7).w[n] > (0xffff-MMX(modrm & 7).w[n]) ? 0xffff : MMX((modrm >> 3) & 0x7).w[n]+MMX(modrm & 7).w[n];
	} else {
		MMX_REG src;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READMMX(cpustate, ea, src);
		for (n=0;n < 4;n++)
			MMX((modrm >> 3) & 0x7).w[n]=MMX((modrm >> 3) & 0x7).w[n] > (0xffff-src.w[n]) ? 0xffff : MMX((modrm >> 3) & 0x7).w[n]+src.w[n];
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void MMXOP(pandn_r64_rm64)(i386_state *cpustate)  // Opcode 0f df
{
	MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		MMX((modrm >> 3) & 0x7).q=(~MMX((modrm >> 3) & 0x7).q) & MMX(modrm & 7).q;
	} else {
		MMX_REG src;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READMMX(cpustate, ea, src);
		MMX((modrm >> 3) & 0x7).q=(~MMX((modrm >> 3) & 0x7).q) & src.q;
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void MMXOP(psraw_r64_rm64)(i386_state *cpustate)  // Opcode 0f e1
{
	MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		int count=(int)MMX(modrm & 7).q;
		MMX((modrm >> 3) & 0x7).s[0]=MMX((modrm >> 3) & 0x7).s[0] >> count;
		MMX((modrm >> 3) & 0x7).s[1]=MMX((modrm >> 3) & 0x7).s[1] >> count;
		MMX((modrm >> 3) & 0x7).s[2]=MMX((modrm >> 3) & 0x7).s[2] >> count;
		MMX((modrm >> 3) & 0x7).s[3]=MMX((modrm >> 3) & 0x7).s[3] >> count;
	} else {
		MMX_REG src;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READMMX(cpustate, ea, src);
		int count=(int)src.q;
		MMX((modrm >> 3) & 0x7).s[0]=MMX((modrm >> 3) & 0x7).s[0] >> count;
		MMX((modrm >> 3) & 0x7).s[1]=MMX((modrm >> 3) & 0x7).s[1] >> count;
		MMX((modrm >> 3) & 0x7).s[2]=MMX((modrm >> 3) & 0x7).s[2] >> count;
		MMX((modrm >> 3) & 0x7).s[3]=MMX((modrm >> 3) & 0x7).s[3] >> count;
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void MMXOP(psrad_r64_rm64)(i386_state *cpustate)  // Opcode 0f e2
{
	MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		int count=(int)MMX(modrm & 7).q;
		MMX((modrm >> 3) & 0x7).i[0]=MMX((modrm >> 3) & 0x7).i[0] >> count;
		MMX((modrm >> 3) & 0x7).i[1]=MMX((modrm >> 3) & 0x7).i[1] >> count;
	} else {
		MMX_REG src;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READMMX(cpustate, ea, src);
		int count=(int)src.q;
		MMX((modrm >> 3) & 0x7).i[0]=MMX((modrm >> 3) & 0x7).i[0] >> count;
		MMX((modrm >> 3) & 0x7).i[1]=MMX((modrm >> 3) & 0x7).i[1] >> count;
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void MMXOP(pmulhw_r64_rm64)(i386_state *cpustate)  // Opcode 0f e5
{
	MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		MMX((modrm >> 3) & 0x7).w[0]=(UINT32)((INT32)MMX((modrm >> 3) & 0x7).s[0]*(INT32)MMX(modrm & 7).s[0]) >> 16;
		MMX((modrm >> 3) & 0x7).w[1]=(UINT32)((INT32)MMX((modrm >> 3) & 0x7).s[1]*(INT32)MMX(modrm & 7).s[1]) >> 16;
		MMX((modrm >> 3) & 0x7).w[2]=(UINT32)((INT32)MMX((modrm >> 3) & 0x7).s[2]*(INT32)MMX(modrm & 7).s[2]) >> 16;
		MMX((modrm >> 3) & 0x7).w[3]=(UINT32)((INT32)MMX((modrm >> 3) & 0x7).s[3]*(INT32)MMX(modrm & 7).s[3]) >> 16;
	} else {
		MMX_REG src;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READMMX(cpustate, ea, src);
		MMX((modrm >> 3) & 0x7).w[0]=(UINT32)((INT32)MMX((modrm >> 3) & 0x7).s[0]*(INT32)src.s[0]) >> 16;
		MMX((modrm >> 3) & 0x7).w[1]=(UINT32)((INT32)MMX((modrm >> 3) & 0x7).s[1]*(INT32)src.s[1]) >> 16;
		MMX((modrm >> 3) & 0x7).w[2]=(UINT32)((INT32)MMX((modrm >> 3) & 0x7).s[2]*(INT32)src.s[2]) >> 16;
		MMX((modrm >> 3) & 0x7).w[3]=(UINT32)((INT32)MMX((modrm >> 3) & 0x7).s[3]*(INT32)src.s[3]) >> 16;
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void MMXOP(psubsb_r64_rm64)(i386_state *cpustate)  // Opcode 0f e8
{
	int n;
	MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		for (n=0;n < 8;n++)
			MMX((modrm >> 3) & 0x7).c[n]=SaturatedSignedWordToSignedByte((INT16)MMX((modrm >> 3) & 0x7).c[n] - (INT16)MMX(modrm & 7).c[n]);
	} else {
		MMX_REG s;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READMMX(cpustate, ea, s);
		for (n=0;n < 8;n++)
			MMX((modrm >> 3) & 0x7).c[n]=SaturatedSignedWordToSignedByte((INT16)MMX((modrm >> 3) & 0x7).c[n] - (INT16)s.c[n]);
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void MMXOP(psubsw_r64_rm64)(i386_state *cpustate)  // Opcode 0f e9
{
	int n;
	MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		for (n=0;n < 4;n++)
			MMX((modrm >> 3) & 0x7).s[n]=SaturatedSignedDwordToSignedWord((INT32)MMX((modrm >> 3) & 0x7).s[n] - (INT32)MMX(modrm & 7).s[n]);
	} else {
		MMX_REG s;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READMMX(cpustate, ea, s);
		for (n=0;n < 4;n++)
			MMX((modrm >> 3) & 0x7).s[n]=SaturatedSignedDwordToSignedWord((INT32)MMX((modrm >> 3) & 0x7).s[n] - (INT32)s.s[n]);
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void MMXOP(por_r64_rm64)(i386_state *cpustate)  // Opcode 0f eb
{
	MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		MMX((modrm >> 3) & 0x7).q=MMX((modrm >> 3) & 0x7).q | MMX(modrm & 7).q;
	} else {
		MMX_REG s;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READMMX(cpustate, ea, s);
		MMX((modrm >> 3) & 0x7).q=MMX((modrm >> 3) & 0x7).q | s.q;
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void MMXOP(paddsb_r64_rm64)(i386_state *cpustate)  // Opcode 0f ec
{
	int n;
	MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		for (n=0;n < 8;n++)
			MMX((modrm >> 3) & 0x7).c[n]=SaturatedSignedWordToSignedByte((INT16)MMX((modrm >> 3) & 0x7).c[n] + (INT16)MMX(modrm & 7).c[n]);
	} else {
		MMX_REG s;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READMMX(cpustate, ea, s);
		for (n=0;n < 8;n++)
			MMX((modrm >> 3) & 0x7).c[n]=SaturatedSignedWordToSignedByte((INT16)MMX((modrm >> 3) & 0x7).c[n] + (INT16)s.c[n]);
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void MMXOP(paddsw_r64_rm64)(i386_state *cpustate)  // Opcode 0f ed
{
	int n;
	MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		for (n=0;n < 4;n++)
			MMX((modrm >> 3) & 0x7).s[n]=SaturatedSignedDwordToSignedWord((INT32)MMX((modrm >> 3) & 0x7).s[n] + (INT32)MMX(modrm & 7).s[n]);
	} else {
		MMX_REG s;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READMMX(cpustate, ea, s);
		for (n=0;n < 4;n++)
			MMX((modrm >> 3) & 0x7).s[n]=SaturatedSignedDwordToSignedWord((INT32)MMX((modrm >> 3) & 0x7).s[n] + (INT32)s.s[n]);
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void MMXOP(pxor_r64_rm64)(i386_state *cpustate)  // Opcode 0f ef
{
	MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		MMX((modrm >> 3) & 0x7).q=MMX((modrm >> 3) & 0x7).q ^ MMX(modrm & 7).q;
	} else {
		MMX_REG s;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READMMX(cpustate, ea, s);
		MMX((modrm >> 3) & 0x7).q=MMX((modrm >> 3) & 0x7).q ^ s.q;
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void MMXOP(psllw_r64_rm64)(i386_state *cpustate)  // Opcode 0f f1
{
	MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		int count=(int)MMX(modrm & 7).q;
		MMX((modrm >> 3) & 0x7).w[0]=MMX((modrm >> 3) & 0x7).w[0] << count;
		MMX((modrm >> 3) & 0x7).w[1]=MMX((modrm >> 3) & 0x7).w[1] << count;
		MMX((modrm >> 3) & 0x7).w[2]=MMX((modrm >> 3) & 0x7).w[2] << count;
		MMX((modrm >> 3) & 0x7).w[3]=MMX((modrm >> 3) & 0x7).w[3] << count;
	} else {
		MMX_REG s;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READMMX(cpustate, ea, s);
		int count=(int)s.q;
		MMX((modrm >> 3) & 0x7).w[0]=MMX((modrm >> 3) & 0x7).w[0] << count;
		MMX((modrm >> 3) & 0x7).w[1]=MMX((modrm >> 3) & 0x7).w[1] << count;
		MMX((modrm >> 3) & 0x7).w[2]=MMX((modrm >> 3) & 0x7).w[2] << count;
		MMX((modrm >> 3) & 0x7).w[3]=MMX((modrm >> 3) & 0x7).w[3] << count;
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void MMXOP(pslld_r64_rm64)(i386_state *cpustate)  // Opcode 0f f2
{
	MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		int count=(int)MMX(modrm & 7).q;
		MMX((modrm >> 3) & 0x7).d[0]=MMX((modrm >> 3) & 0x7).d[0] << count;
		MMX((modrm >> 3) & 0x7).d[1]=MMX((modrm >> 3) & 0x7).d[1] << count;
	} else {
		MMX_REG s;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READMMX(cpustate, ea, s);
		int count=(int)s.q;
		MMX((modrm >> 3) & 0x7).d[0]=MMX((modrm >> 3) & 0x7).d[0] << count;
		MMX((modrm >> 3) & 0x7).d[1]=MMX((modrm >> 3) & 0x7).d[1] << count;
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void MMXOP(psllq_r64_rm64)(i386_state *cpustate)  // Opcode 0f f3
{
	MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		int count=(int)MMX(modrm & 7).q;
		MMX((modrm >> 3) & 0x7).q=MMX((modrm >> 3) & 0x7).q << count;
	} else {
		MMX_REG s;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READMMX(cpustate, ea, s);
		int count=(int)s.q;
		MMX((modrm >> 3) & 0x7).q=MMX((modrm >> 3) & 0x7).q << count;
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void MMXOP(pmaddwd_r64_rm64)(i386_state *cpustate)  // Opcode 0f f5
{
	MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		MMX((modrm >> 3) & 0x7).i[0]=(INT32)MMX((modrm >> 3) & 0x7).s[0]*(INT32)MMX(modrm & 7).s[0]+
										(INT32)MMX((modrm >> 3) & 0x7).s[1]*(INT32)MMX(modrm & 7).s[1];
		MMX((modrm >> 3) & 0x7).i[1]=(INT32)MMX((modrm >> 3) & 0x7).s[2]*(INT32)MMX(modrm & 7).s[2]+
										(INT32)MMX((modrm >> 3) & 0x7).s[3]*(INT32)MMX(modrm & 7).s[3];
	} else {
		MMX_REG s;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READMMX(cpustate, ea, s);
		MMX((modrm >> 3) & 0x7).i[0]=(INT32)MMX((modrm >> 3) & 0x7).s[0]*(INT32)s.s[0]+
										(INT32)MMX((modrm >> 3) & 0x7).s[1]*(INT32)s.s[1];
		MMX((modrm >> 3) & 0x7).i[1]=(INT32)MMX((modrm >> 3) & 0x7).s[2]*(INT32)s.s[2]+
										(INT32)MMX((modrm >> 3) & 0x7).s[3]*(INT32)s.s[3];
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void MMXOP(psubb_r64_rm64)(i386_state *cpustate)  // Opcode 0f f8
{
	int n;
	MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		for (n=0;n < 8;n++)
			MMX((modrm >> 3) & 0x7).b[n]=MMX((modrm >> 3) & 0x7).b[n] - MMX(modrm & 7).b[n];
	} else {
		MMX_REG s;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READMMX(cpustate, ea, s);
		for (n=0;n < 8;n++)
			MMX((modrm >> 3) & 0x7).b[n]=MMX((modrm >> 3) & 0x7).b[n] - s.b[n];
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void MMXOP(psubw_r64_rm64)(i386_state *cpustate)  // Opcode 0f f9
{
	int n;
	MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		for (n=0;n < 4;n++)
			MMX((modrm >> 3) & 0x7).w[n]=MMX((modrm >> 3) & 0x7).w[n] - MMX(modrm & 7).w[n];
	} else {
		MMX_REG s;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READMMX(cpustate, ea, s);
		for (n=0;n < 4;n++)
			MMX((modrm >> 3) & 0x7).w[n]=MMX((modrm >> 3) & 0x7).w[n] - s.w[n];
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void MMXOP(psubd_r64_rm64)(i386_state *cpustate)  // Opcode 0f fa
{
	int n;
	MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		for (n=0;n < 2;n++)
			MMX((modrm >> 3) & 0x7).d[n]=MMX((modrm >> 3) & 0x7).d[n] - MMX(modrm & 7).d[n];
	} else {
		MMX_REG s;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READMMX(cpustate, ea, s);
		for (n=0;n < 2;n++)
			MMX((modrm >> 3) & 0x7).d[n]=MMX((modrm >> 3) & 0x7).d[n] - s.d[n];
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void MMXOP(paddb_r64_rm64)(i386_state *cpustate)  // Opcode 0f fc
{
	int n;
	MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		for (n=0;n < 8;n++)
			MMX((modrm >> 3) & 0x7).b[n]=MMX((modrm >> 3) & 0x7).b[n] + MMX(modrm & 7).b[n];
	} else {
		MMX_REG s;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READMMX(cpustate, ea, s);
		for (n=0;n < 8;n++)
			MMX((modrm >> 3) & 0x7).b[n]=MMX((modrm >> 3) & 0x7).b[n] + s.b[n];
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void MMXOP(paddw_r64_rm64)(i386_state *cpustate)  // Opcode 0f fd
{
	int n;
	MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		for (n=0;n < 4;n++)
			MMX((modrm >> 3) & 0x7).w[n]=MMX((modrm >> 3) & 0x7).w[n] + MMX(modrm & 7).w[n];
	} else {
		MMX_REG s;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READMMX(cpustate, ea, s);
		for (n=0;n < 4;n++)
			MMX((modrm >> 3) & 0x7).w[n]=MMX((modrm >> 3) & 0x7).w[n] + s.w[n];
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void MMXOP(paddd_r64_rm64)(i386_state *cpustate)  // Opcode 0f fe
{
	int n;
	MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		for (n=0;n < 2;n++)
			MMX((modrm >> 3) & 0x7).d[n]=MMX((modrm >> 3) & 0x7).d[n] + MMX(modrm & 7).d[n];
	} else {
		MMX_REG s;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READMMX(cpustate, ea, s);
		for (n=0;n < 2;n++)
			MMX((modrm >> 3) & 0x7).d[n]=MMX((modrm >> 3) & 0x7).d[n] + s.d[n];
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void MMXOP(emms)(i386_state *cpustate) // Opcode 0f 77
{
	cpustate->x87_tw = 0xffff; // tag word = 0xffff
	// TODO
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void MMXOP(movd_r64_rm32)(i386_state *cpustate) // Opcode 0f 6e
{
	MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		MMX((modrm >> 3) & 0x7).d[0]=LOAD_RM32(modrm);
	} else {
		UINT32 ea = GetEA(cpustate, modrm, 0);
		MMX((modrm >> 3) & 0x7).d[0]=READ32(cpustate, ea);
	}
	MMX((modrm >> 3) & 0x7).d[1]=0;
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void MMXOP(movq_r64_rm64)(i386_state *cpustate) // Opcode 0f 6f
{
	MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		MMX((modrm >> 3) & 0x7).l=MMX(modrm & 0x7).l;
	} else {
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READMMX(cpustate, ea, MMX((modrm >> 3) & 0x7));
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void MMXOP(movd_rm32_r64)(i386_state *cpustate) // Opcode 0f 7e
{
	MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		STORE_RM32(modrm, MMX((modrm >> 3) & 0x7).d[0]);
	} else {
		UINT32 ea = GetEA(cpustate, modrm, 0);
		WRITE32(cpustate, ea, MMX((modrm >> 3) & 0x7).d[0]);
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void MMXOP(movq_rm64_r64)(i386_state *cpustate) // Opcode 0f 7f
{
	MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		MMX(modrm & 0x7)=MMX((modrm >> 3) & 0x7);
	} else {
		UINT32 ea = GetEA(cpustate, modrm, 0);
		WRITEMMX(cpustate, ea, MMX((modrm >> 3) & 0x7));
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void MMXOP(pcmpeqb_r64_rm64)(i386_state *cpustate) // Opcode 0f 74
{
	int c;
	MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		int s,d;
		s=modrm & 0x7;
		d=(modrm >> 3) & 0x7;
		for (c=0;c <= 7;c++)
			MMX(d).b[c]=(MMX(d).b[c] == MMX(s).b[c]) ? 0xff : 0;
	} else {
		MMX_REG s;
		int d=(modrm >> 3) & 0x7;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READMMX(cpustate, ea, s);
		for (c=0;c <= 7;c++)
			MMX(d).b[c]=(MMX(d).b[c] == s.b[c]) ? 0xff : 0;
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void MMXOP(pcmpeqw_r64_rm64)(i386_state *cpustate) // Opcode 0f 75
{
	MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
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
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READMMX(cpustate, ea, s);
		MMX(d).w[0]=(MMX(d).w[0] == s.w[0]) ? 0xffff : 0;
		MMX(d).w[1]=(MMX(d).w[1] == s.w[1]) ? 0xffff : 0;
		MMX(d).w[2]=(MMX(d).w[2] == s.w[2]) ? 0xffff : 0;
		MMX(d).w[3]=(MMX(d).w[3] == s.w[3]) ? 0xffff : 0;
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void MMXOP(pcmpeqd_r64_rm64)(i386_state *cpustate) // Opcode 0f 76
{
	MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		int s,d;
		s=modrm & 0x7;
		d=(modrm >> 3) & 0x7;
		MMX(d).d[0]=(MMX(d).d[0] == MMX(s).d[0]) ? 0xffffffff : 0;
		MMX(d).d[1]=(MMX(d).d[1] == MMX(s).d[1]) ? 0xffffffff : 0;
	} else {
		MMX_REG s;
		int d=(modrm >> 3) & 0x7;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READMMX(cpustate, ea, s);
		MMX(d).d[0]=(MMX(d).d[0] == s.d[0]) ? 0xffffffff : 0;
		MMX(d).d[1]=(MMX(d).d[1] == s.d[1]) ? 0xffffffff : 0;
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void MMXOP(pshufw_r64_rm64_i8)(i386_state *cpustate) // Opcode 0f 70
{
	MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		MMX_REG t;
		int s,d;
		UINT8 imm8 = FETCH(cpustate);
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
		UINT32 ea = GetEA(cpustate, modrm, 0);
		UINT8 imm8 = FETCH(cpustate);
		READMMX(cpustate, ea, s);
		MMX(d).w[0]=s.w[imm8 & 3];
		MMX(d).w[1]=s.w[(imm8 >> 2) & 3];
		MMX(d).w[2]=s.w[(imm8 >> 4) & 3];
		MMX(d).w[3]=s.w[(imm8 >> 6) & 3];
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void MMXOP(punpcklbw_r64_r64m32)(i386_state *cpustate) // Opcode 0f 60
{
	MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
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
		UINT32 ea = GetEA(cpustate, modrm, 0);
		s = READ32(cpustate, ea);
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
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void MMXOP(punpcklwd_r64_r64m32)(i386_state *cpustate) // Opcode 0f 61
{
	MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
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
		UINT32 ea = GetEA(cpustate, modrm, 0);
		s = READ32(cpustate, ea);
		t=MMX(d).w[1];
		MMX(d).w[0]=MMX(d).w[0];
		MMX(d).w[1]=s & 0xffff;
		MMX(d).w[2]=t;
		MMX(d).w[3]=(s >> 16) & 0xffff;
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void MMXOP(punpckldq_r64_r64m32)(i386_state *cpustate) // Opcode 0f 62
{
	MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		int s,d;
		s=modrm & 0x7;
		d=(modrm >> 3) & 0x7;
		MMX(d).d[0]=MMX(d).d[0];
		MMX(d).d[1]=MMX(s).d[0];
	} else {
		UINT32 s;
		int d=(modrm >> 3) & 0x7;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		s = READ32(cpustate, ea);
		MMX(d).d[0]=MMX(d).d[0];
		MMX(d).d[1]=s;
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void MMXOP(packsswb_r64_rm64)(i386_state *cpustate) // Opcode 0f 63
{
	MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
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
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READMMX(cpustate, ea, s);
		MMX(d).c[0]=SaturatedSignedWordToSignedByte(MMX(d).s[0]);
		MMX(d).c[1]=SaturatedSignedWordToSignedByte(MMX(d).s[1]);
		MMX(d).c[2]=SaturatedSignedWordToSignedByte(MMX(d).s[2]);
		MMX(d).c[3]=SaturatedSignedWordToSignedByte(MMX(d).s[3]);
		MMX(d).c[4]=SaturatedSignedWordToSignedByte(s.s[0]);
		MMX(d).c[5]=SaturatedSignedWordToSignedByte(s.s[1]);
		MMX(d).c[6]=SaturatedSignedWordToSignedByte(s.s[2]);
		MMX(d).c[7]=SaturatedSignedWordToSignedByte(s.s[3]);
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void MMXOP(pcmpgtb_r64_rm64)(i386_state *cpustate) // Opcode 0f 64
{
	int c;
	MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		int s,d;
		s=modrm & 0x7;
		d=(modrm >> 3) & 0x7;
		for (c=0;c <= 7;c++)
			MMX(d).b[c]=(MMX(d).c[c] > MMX(s).c[c]) ? 0xff : 0;
	} else {
		MMX_REG s;
		int d=(modrm >> 3) & 0x7;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READMMX(cpustate, ea, s);
		for (c=0;c <= 7;c++)
			MMX(d).b[c]=(MMX(d).c[c] > s.c[c]) ? 0xff : 0;
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void MMXOP(pcmpgtw_r64_rm64)(i386_state *cpustate) // Opcode 0f 65
{
	int c;
	MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		int s,d;
		s=modrm & 0x7;
		d=(modrm >> 3) & 0x7;
		for (c=0;c <= 3;c++)
			MMX(d).w[c]=(MMX(d).s[c] > MMX(s).s[c]) ? 0xffff : 0;
	} else {
		MMX_REG s;
		int d=(modrm >> 3) & 0x7;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READMMX(cpustate, ea, s);
		for (c=0;c <= 3;c++)
			MMX(d).w[c]=(MMX(d).s[c] > s.s[c]) ? 0xffff : 0;
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void MMXOP(pcmpgtd_r64_rm64)(i386_state *cpustate) // Opcode 0f 66
{
	int c;
	MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		int s,d;
		s=modrm & 0x7;
		d=(modrm >> 3) & 0x7;
		for (c=0;c <= 1;c++)
			MMX(d).d[c]=(MMX(d).i[c] > MMX(s).i[c]) ? 0xffffffff : 0;
	} else {
		MMX_REG s;
		int d=(modrm >> 3) & 0x7;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READMMX(cpustate, ea, s);
		for (c=0;c <= 1;c++)
			MMX(d).d[c]=(MMX(d).i[c] > s.i[c]) ? 0xffffffff : 0;
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void MMXOP(packuswb_r64_rm64)(i386_state *cpustate) // Opcode 0f 67
{
	MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
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
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READMMX(cpustate, ea, s);
		MMX(d).b[0]=SaturatedSignedWordToUnsignedByte(MMX(d).s[0]);
		MMX(d).b[1]=SaturatedSignedWordToUnsignedByte(MMX(d).s[1]);
		MMX(d).b[2]=SaturatedSignedWordToUnsignedByte(MMX(d).s[2]);
		MMX(d).b[3]=SaturatedSignedWordToUnsignedByte(MMX(d).s[3]);
		MMX(d).b[4]=SaturatedSignedWordToUnsignedByte(s.s[0]);
		MMX(d).b[5]=SaturatedSignedWordToUnsignedByte(s.s[1]);
		MMX(d).b[6]=SaturatedSignedWordToUnsignedByte(s.s[2]);
		MMX(d).b[7]=SaturatedSignedWordToUnsignedByte(s.s[3]);
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void MMXOP(punpckhbw_r64_rm64)(i386_state *cpustate) // Opcode 0f 68
{
	MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
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
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READMMX(cpustate, ea, s);
		MMX(d).b[0]=MMX(d).b[4];
		MMX(d).b[1]=s.b[4];
		MMX(d).b[2]=MMX(d).b[5];
		MMX(d).b[3]=s.b[5];
		MMX(d).b[4]=MMX(d).b[6];
		MMX(d).b[5]=s.b[6];
		MMX(d).b[6]=MMX(d).b[7];
		MMX(d).b[7]=s.b[7];
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void MMXOP(punpckhwd_r64_rm64)(i386_state *cpustate) // Opcode 0f 69
{
	MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
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
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READMMX(cpustate, ea, s);
		MMX(d).w[0]=MMX(d).w[2];
		MMX(d).w[1]=s.w[2];
		MMX(d).w[2]=MMX(d).w[3];
		MMX(d).w[3]=s.w[3];
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void MMXOP(punpckhdq_r64_rm64)(i386_state *cpustate) // Opcode 0f 6a
{
	MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		int s,d;
		s=modrm & 0x7;
		d=(modrm >> 3) & 0x7;
		MMX(d).d[0]=MMX(d).d[1];
		MMX(d).d[1]=MMX(s).d[1];
	} else {
		MMX_REG s;
		int d=(modrm >> 3) & 0x7;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READMMX(cpustate, ea, s);
		MMX(d).d[0]=MMX(d).d[1];
		MMX(d).d[1]=s.d[1];
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void MMXOP(packssdw_r64_rm64)(i386_state *cpustate) // Opcode 0f 6b
{
	MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
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
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READMMX(cpustate, ea, s);
		MMX(d).s[0]=SaturatedSignedDwordToSignedWord(MMX(d).i[0]);
		MMX(d).s[1]=SaturatedSignedDwordToSignedWord(MMX(d).i[1]);
		MMX(d).s[2]=SaturatedSignedDwordToSignedWord(s.i[0]);
		MMX(d).s[3]=SaturatedSignedDwordToSignedWord(s.i[1]);
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(sse_group0fae)(i386_state *cpustate)  // Opcode 0f ae
{
	UINT8 modm = FETCH(cpustate);
	if( modm == 0xf8 ) {
		logerror("Unemulated SFENCE opcode called\n");
		CYCLES(cpustate,1); // sfence instruction
	} else if( modm == 0xf0 ) {
		CYCLES(cpustate,1); // mfence instruction
	} else if( modm == 0xe8 ) {
		CYCLES(cpustate,1); // lfence instruction
	} else if( modm < 0xc0 ) {
		UINT32 ea;
		switch ( (modm & 0x38) >> 3 )
		{
			case 2: // ldmxcsr m32
				ea = GetEA(cpustate, modm, 0);
				cpustate->mxcsr = READ32(cpustate, ea);
				break;
			case 3: // stmxcsr m32
				ea = GetEA(cpustate, modm, 0);
				WRITE32(cpustate, ea, cpustate->mxcsr);
				break;
			case 7: // clflush m8
				GetNonTranslatedEA(cpustate, modm, NULL);
				break;
			default:
				report_invalid_modrm(cpustate, "sse_group0fae", modm);
		}
	} else {
		report_invalid_modrm(cpustate, "sse_group0fae", modm);
	}
}

static void SSEOP(cvttps2dq_r128_rm128)(i386_state *cpustate) // Opcode f3 0f 5b
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).i[0]=(INT32)XMM(modrm & 0x7).f[0];
		XMM((modrm >> 3) & 0x7).i[1]=(INT32)XMM(modrm & 0x7).f[1];
		XMM((modrm >> 3) & 0x7).i[2]=(INT32)XMM(modrm & 0x7).f[2];
		XMM((modrm >> 3) & 0x7).i[3]=(INT32)XMM(modrm & 0x7).f[3];
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READXMM(cpustate, ea, src);
		XMM((modrm >> 3) & 0x7).i[0]=(INT32)src.f[0];
		XMM((modrm >> 3) & 0x7).i[1]=(INT32)src.f[1];
		XMM((modrm >> 3) & 0x7).i[2]=(INT32)src.f[2];
		XMM((modrm >> 3) & 0x7).i[3]=(INT32)src.f[3];
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(cvtss2sd_r128_r128m32)(i386_state *cpustate) // Opcode f3 0f 5a
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).f64[0] = XMM(modrm & 0x7).f[0];
	} else {
		XMM_REG s;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		s.d[0] = READ32(cpustate, ea);
		XMM((modrm >> 3) & 0x7).f64[0] = s.f[0];
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(cvttss2si_r32_r128m32)(i386_state *cpustate) // Opcode f3 0f 2c
{
	INT32 src;
	UINT8 modrm = FETCH(cpustate); // get mordm byte
	if( modrm >= 0xc0 ) { // if bits 7-6 are 11 the source is a xmm register (low doubleword)
		src = (INT32)XMM(modrm & 0x7).f[0^NATIVE_ENDIAN_VALUE_LE_BE(0,1)];
	} else { // otherwise is a memory address
		XMM_REG t;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		t.d[0] = READ32(cpustate, ea);
		src = (INT32)t.f[0];
	}
	STORE_REG32(modrm, (UINT32)src);
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(cvtss2si_r32_r128m32)(i386_state *cpustate) // Opcode f3 0f 2d
{
	INT32 src;
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = (INT32)XMM(modrm & 0x7).f[0];
	} else {
		XMM_REG t;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		t.d[0] = READ32(cpustate, ea);
		src = (INT32)t.f[0];
	}
	STORE_REG32(modrm, (UINT32)src);
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(cvtsi2ss_r128_rm32)(i386_state *cpustate) // Opcode f3 0f 2a
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).f[0] = (INT32)LOAD_RM32(modrm);
	} else {
		UINT32 ea = GetEA(cpustate, modrm, 0);
		XMM((modrm >> 3) & 0x7).f[0] = (INT32)READ32(cpustate, ea);
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(cvtpi2ps_r128_rm64)(i386_state *cpustate) // Opcode 0f 2a
{
	UINT8 modrm = FETCH(cpustate);
	MMXPROLOG(cpustate);
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).f[0] = MMX(modrm & 0x7).i[0];
		XMM((modrm >> 3) & 0x7).f[1] = MMX(modrm & 0x7).i[1];
	} else {
		MMX_REG r;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READMMX(cpustate, ea, r);
		XMM((modrm >> 3) & 0x7).f[0] = r.i[0];
		XMM((modrm >> 3) & 0x7).f[1] = r.i[1];
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(cvttps2pi_r64_r128m64)(i386_state *cpustate) // Opcode 0f 2c
{
	UINT8 modrm = FETCH(cpustate);
	MMXPROLOG(cpustate);
	if( modrm >= 0xc0 ) {
		MMX((modrm >> 3) & 0x7).i[0] = XMM(modrm & 0x7).f[0];
		MMX((modrm >> 3) & 0x7).i[1] = XMM(modrm & 0x7).f[1];
	} else {
		XMM_REG r;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READXMM(cpustate, ea, r);
		XMM((modrm >> 3) & 0x7).i[0] = r.f[0];
		XMM((modrm >> 3) & 0x7).i[1] = r.f[1];
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(cvtps2pi_r64_r128m64)(i386_state *cpustate) // Opcode 0f 2d
{
	UINT8 modrm = FETCH(cpustate);
	MMXPROLOG(cpustate);
	if( modrm >= 0xc0 ) {
		MMX((modrm >> 3) & 0x7).i[0] = XMM(modrm & 0x7).f[0];
		MMX((modrm >> 3) & 0x7).i[1] = XMM(modrm & 0x7).f[1];
	} else {
		XMM_REG r;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READXMM(cpustate, ea, r);
		XMM((modrm >> 3) & 0x7).i[0] = r.f[0];
		XMM((modrm >> 3) & 0x7).i[1] = r.f[1];
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(cvtps2pd_r128_r128m64)(i386_state *cpustate) // Opcode 0f 5a
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).f64[0] = (double)XMM(modrm & 0x7).f[0];
		XMM((modrm >> 3) & 0x7).f64[1] = (double)XMM(modrm & 0x7).f[1];
	} else {
		MMX_REG r;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READMMX(cpustate, ea, r);
		XMM((modrm >> 3) & 0x7).f64[0] = (double)r.f[0];
		XMM((modrm >> 3) & 0x7).f64[1] = (double)r.f[1];
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(cvtdq2ps_r128_rm128)(i386_state *cpustate) // Opcode 0f 5b
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).f[0] = (float)XMM(modrm & 0x7).i[0];
		XMM((modrm >> 3) & 0x7).f[1] = (float)XMM(modrm & 0x7).i[1];
		XMM((modrm >> 3) & 0x7).f[2] = (float)XMM(modrm & 0x7).i[2];
		XMM((modrm >> 3) & 0x7).f[3] = (float)XMM(modrm & 0x7).i[3];
	} else {
		XMM_REG r;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READXMM(cpustate, ea, r);
		XMM((modrm >> 3) & 0x7).f[0] = (float)r.i[0];
		XMM((modrm >> 3) & 0x7).f[1] = (float)r.i[1];
		XMM((modrm >> 3) & 0x7).f[2] = (float)r.i[2];
		XMM((modrm >> 3) & 0x7).f[3] = (float)r.i[3];
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(cvtdq2pd_r128_r128m64)(i386_state *cpustate) // Opcode f3 0f e6
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).f64[0] = (double)XMM(modrm & 0x7).i[0];
		XMM((modrm >> 3) & 0x7).f64[1] = (double)XMM(modrm & 0x7).i[1];
	} else {
		MMX_REG s;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READMMX(cpustate, ea, s);
		XMM((modrm >> 3) & 0x7).f64[0] = (double)s.i[0];
		XMM((modrm >> 3) & 0x7).f64[1] = (double)s.i[1];
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(movss_r128_rm128)(i386_state *cpustate) // Opcode f3 0f 10
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).d[0] = XMM(modrm & 0x7).d[0];
	} else {
		UINT32 ea = GetEA(cpustate, modrm, 0);
		XMM((modrm >> 3) & 0x7).d[0] = READ32(cpustate, ea);
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(movss_rm128_r128)(i386_state *cpustate) // Opcode f3 0f 11
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		XMM(modrm & 0x7).d[0] = XMM((modrm >> 3) & 0x7).d[0];
	} else {
		UINT32 ea = GetEA(cpustate, modrm, 0);
		WRITE32(cpustate, ea, XMM((modrm >> 3) & 0x7).d[0]);
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(movsldup_r128_rm128)(i386_state *cpustate) // Opcode f3 0f 12
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).d[0] = XMM(modrm & 0x7).d[0];
		XMM((modrm >> 3) & 0x7).d[1] = XMM(modrm & 0x7).d[0];
		XMM((modrm >> 3) & 0x7).d[2] = XMM(modrm & 0x7).d[2];
		XMM((modrm >> 3) & 0x7).d[3] = XMM(modrm & 0x7).d[2];
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READXMM(cpustate, ea, src);
		XMM((modrm >> 3) & 0x7).d[0] = src.d[0];
		XMM((modrm >> 3) & 0x7).d[1] = src.d[0];
		XMM((modrm >> 3) & 0x7).d[2] = src.d[2];
		XMM((modrm >> 3) & 0x7).d[3] = src.d[2];
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(movshdup_r128_rm128)(i386_state *cpustate) // Opcode f3 0f 16
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).d[0] = XMM(modrm & 0x7).d[1];
		XMM((modrm >> 3) & 0x7).d[1] = XMM(modrm & 0x7).d[1];
		XMM((modrm >> 3) & 0x7).d[2] = XMM(modrm & 0x7).d[3];
		XMM((modrm >> 3) & 0x7).d[3] = XMM(modrm & 0x7).d[3];
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READXMM(cpustate, ea, src);
		XMM((modrm >> 3) & 0x7).d[0] = src.d[1];
		XMM((modrm >> 3) & 0x7).d[1] = src.d[1];
		XMM((modrm >> 3) & 0x7).d[2] = src.d[3];
		XMM((modrm >> 3) & 0x7).d[3] = src.d[3];
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(movaps_r128_rm128)(i386_state *cpustate) // Opcode 0f 28
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7) = XMM(modrm & 0x7);
	} else {
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READXMM(cpustate, ea, XMM((modrm >> 3) & 0x7));
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(movaps_rm128_r128)(i386_state *cpustate) // Opcode 0f 29
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		XMM(modrm & 0x7) = XMM((modrm >> 3) & 0x7);
	} else {
		UINT32 ea = GetEA(cpustate, modrm, 0);
		WRITEXMM(cpustate, ea, XMM((modrm >> 3) & 0x7));
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(movups_r128_rm128)(i386_state *cpustate) // Opcode 0f 10
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7) = XMM(modrm & 0x7);
	} else {
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READXMM(cpustate, ea, XMM((modrm >> 3) & 0x7)); // address does not need to be 16-byte aligned
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(movups_rm128_r128)(i386_state *cpustate) // Opcode 0f 11
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		XMM(modrm & 0x7) = XMM((modrm >> 3) & 0x7);
	} else {
		UINT32 ea = GetEA(cpustate, modrm, 0);
		WRITEXMM(cpustate, ea, XMM((modrm >> 3) & 0x7)); // address does not need to be 16-byte aligned
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(movlps_r128_m64)(i386_state *cpustate) // Opcode 0f 12
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		// unsupported by cpu
		CYCLES(cpustate,1);     // TODO: correct cycle count
	} else {
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READXMM_LO64(cpustate, ea, XMM((modrm >> 3) & 0x7));
		CYCLES(cpustate,1);     // TODO: correct cycle count
	}
}

static void SSEOP(movlps_m64_r128)(i386_state *cpustate) // Opcode 0f 13
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		// unsupported by cpu
		CYCLES(cpustate,1);     // TODO: correct cycle count
	} else {
		UINT32 ea = GetEA(cpustate, modrm, 0);
		WRITEXMM_LO64(cpustate, ea, XMM((modrm >> 3) & 0x7));
		CYCLES(cpustate,1);     // TODO: correct cycle count
	}
}

static void SSEOP(movhps_r128_m64)(i386_state *cpustate) // Opcode 0f 16
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		// unsupported by cpu
		CYCLES(cpustate,1);     // TODO: correct cycle count
	} else {
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READXMM_HI64(cpustate, ea, XMM((modrm >> 3) & 0x7));
		CYCLES(cpustate,1);     // TODO: correct cycle count
	}
}

static void SSEOP(movhps_m64_r128)(i386_state *cpustate) // Opcode 0f 17
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		// unsupported by cpu
		CYCLES(cpustate,1);     // TODO: correct cycle count
	} else {
		UINT32 ea = GetEA(cpustate, modrm, 0);
		WRITEXMM_HI64(cpustate,ea, XMM((modrm >> 3) & 0x7));
		CYCLES(cpustate,1);     // TODO: correct cycle count
	}
}

static void SSEOP(movntps_m128_r128)(i386_state *cpustate) // Opcode 0f 2b
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		// unsupported by cpu
		CYCLES(cpustate,1);     // TODO: correct cycle count
	} else {
		// since cache is not implemented
		UINT32 ea = GetEA(cpustate, modrm, 0);
		WRITEXMM(cpustate, ea, XMM((modrm >> 3) & 0x7));
		CYCLES(cpustate,1);     // TODO: correct cycle count
	}
}

static void SSEOP(movmskps_r16_r128)(i386_state *cpustate) // Opcode 0f 50
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		int b;
		b=(XMM(modrm & 0x7).d[0] >> 31) & 1;
		b=b | ((XMM(modrm & 0x7).d[1] >> 30) & 2);
		b=b | ((XMM(modrm & 0x7).d[2] >> 29) & 4);
		b=b | ((XMM(modrm & 0x7).d[3] >> 28) & 8);
		STORE_REG16(modrm, b);
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(movmskps_r32_r128)(i386_state *cpustate) // Opcode 0f 50
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		int b;
		b=(XMM(modrm & 0x7).d[0] >> 31) & 1;
		b=b | ((XMM(modrm & 0x7).d[1] >> 30) & 2);
		b=b | ((XMM(modrm & 0x7).d[2] >> 29) & 4);
		b=b | ((XMM(modrm & 0x7).d[3] >> 28) & 8);
		STORE_REG32(modrm, b);
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(movq2dq_r128_r64)(i386_state *cpustate) // Opcode f3 0f d6
{
	MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).q[0] = MMX(modrm & 7).q;
		XMM((modrm >> 3) & 0x7).q[1] = 0;
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(movdqu_r128_rm128)(i386_state *cpustate) // Opcode f3 0f 6f
{
	MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).q[0] = XMM(modrm & 0x7).q[0];
		XMM((modrm >> 3) & 0x7).q[1] = XMM(modrm & 0x7).q[1];
	} else {
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READXMM(cpustate, ea, XMM((modrm >> 3) & 0x7));
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(movdqu_rm128_r128)(i386_state *cpustate) // Opcode f3 0f 7f
{
	MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		XMM(modrm & 0x7).q[0] = XMM((modrm >> 3) & 0x7).q[0];
		XMM(modrm & 0x7).q[1] = XMM((modrm >> 3) & 0x7).q[1];
	} else {
		UINT32 ea = GetEA(cpustate, modrm, 0);
		WRITEXMM(cpustate, ea, XMM((modrm >> 3) & 0x7));
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(movq_r128_r128m64)(i386_state *cpustate) // Opcode f3 0f 7e
{
	MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).q[0] = XMM(modrm & 0x7).q[0];
		XMM((modrm >> 3) & 0x7).q[1] = 0;
	} else {
		UINT32 ea = GetEA(cpustate, modrm, 0);
		XMM((modrm >> 3) & 0x7).q[0] = READ64(cpustate,ea);
		XMM((modrm >> 3) & 0x7).q[1] = 0;
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(pmovmskb_r16_r64)(i386_state *cpustate) // Opcode 0f d7
{
	//MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
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
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(pmovmskb_r32_r64)(i386_state *cpustate) // Opcode 0f d7
{
	//MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
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
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(xorps)(i386_state *cpustate) // Opcode 0f 57
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).d[0] = XMM((modrm >> 3) & 0x7).d[0] ^ XMM(modrm & 0x7).d[0];
		XMM((modrm >> 3) & 0x7).d[1] = XMM((modrm >> 3) & 0x7).d[1] ^ XMM(modrm & 0x7).d[1];
		XMM((modrm >> 3) & 0x7).d[2] = XMM((modrm >> 3) & 0x7).d[2] ^ XMM(modrm & 0x7).d[2];
		XMM((modrm >> 3) & 0x7).d[3] = XMM((modrm >> 3) & 0x7).d[3] ^ XMM(modrm & 0x7).d[3];
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READXMM(cpustate, ea, src);
		XMM((modrm >> 3) & 0x7).d[0] = XMM((modrm >> 3) & 0x7).d[0] ^ src.d[0];
		XMM((modrm >> 3) & 0x7).d[1] = XMM((modrm >> 3) & 0x7).d[1] ^ src.d[1];
		XMM((modrm >> 3) & 0x7).d[2] = XMM((modrm >> 3) & 0x7).d[2] ^ src.d[2];
		XMM((modrm >> 3) & 0x7).d[3] = XMM((modrm >> 3) & 0x7).d[3] ^ src.d[3];
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(addps)(i386_state *cpustate) // Opcode 0f 58
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).f[0] = XMM((modrm >> 3) & 0x7).f[0] + XMM(modrm & 0x7).f[0];
		XMM((modrm >> 3) & 0x7).f[1] = XMM((modrm >> 3) & 0x7).f[1] + XMM(modrm & 0x7).f[1];
		XMM((modrm >> 3) & 0x7).f[2] = XMM((modrm >> 3) & 0x7).f[2] + XMM(modrm & 0x7).f[2];
		XMM((modrm >> 3) & 0x7).f[3] = XMM((modrm >> 3) & 0x7).f[3] + XMM(modrm & 0x7).f[3];
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READXMM(cpustate, ea, src);
		XMM((modrm >> 3) & 0x7).f[0] = XMM((modrm >> 3) & 0x7).f[0] + src.f[0];
		XMM((modrm >> 3) & 0x7).f[1] = XMM((modrm >> 3) & 0x7).f[1] + src.f[1];
		XMM((modrm >> 3) & 0x7).f[2] = XMM((modrm >> 3) & 0x7).f[2] + src.f[2];
		XMM((modrm >> 3) & 0x7).f[3] = XMM((modrm >> 3) & 0x7).f[3] + src.f[3];
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(sqrtps_r128_rm128)(i386_state *cpustate) // Opcode 0f 51
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).f[0] = sqrt(XMM(modrm & 0x7).f[0]);
		XMM((modrm >> 3) & 0x7).f[1] = sqrt(XMM(modrm & 0x7).f[1]);
		XMM((modrm >> 3) & 0x7).f[2] = sqrt(XMM(modrm & 0x7).f[2]);
		XMM((modrm >> 3) & 0x7).f[3] = sqrt(XMM(modrm & 0x7).f[3]);
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READXMM(cpustate, ea, src);
		XMM((modrm >> 3) & 0x7).f[0] = sqrt(src.f[0]);
		XMM((modrm >> 3) & 0x7).f[1] = sqrt(src.f[1]);
		XMM((modrm >> 3) & 0x7).f[2] = sqrt(src.f[2]);
		XMM((modrm >> 3) & 0x7).f[3] = sqrt(src.f[3]);
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(rsqrtps_r128_rm128)(i386_state *cpustate) // Opcode 0f 52
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).f[0] = 1.0 / sqrt(XMM(modrm & 0x7).f[0]);
		XMM((modrm >> 3) & 0x7).f[1] = 1.0 / sqrt(XMM(modrm & 0x7).f[1]);
		XMM((modrm >> 3) & 0x7).f[2] = 1.0 / sqrt(XMM(modrm & 0x7).f[2]);
		XMM((modrm >> 3) & 0x7).f[3] = 1.0 / sqrt(XMM(modrm & 0x7).f[3]);
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READXMM(cpustate, ea, src);
		XMM((modrm >> 3) & 0x7).f[0] = 1.0 / sqrt(src.f[0]);
		XMM((modrm >> 3) & 0x7).f[1] = 1.0 / sqrt(src.f[1]);
		XMM((modrm >> 3) & 0x7).f[2] = 1.0 / sqrt(src.f[2]);
		XMM((modrm >> 3) & 0x7).f[3] = 1.0 / sqrt(src.f[3]);
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(rcpps_r128_rm128)(i386_state *cpustate) // Opcode 0f 53
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).f[0] = 1.0 / XMM(modrm & 0x7).f[0];
		XMM((modrm >> 3) & 0x7).f[1] = 1.0 / XMM(modrm & 0x7).f[1];
		XMM((modrm >> 3) & 0x7).f[2] = 1.0 / XMM(modrm & 0x7).f[2];
		XMM((modrm >> 3) & 0x7).f[3] = 1.0 / XMM(modrm & 0x7).f[3];
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READXMM(cpustate, ea, src);
		XMM((modrm >> 3) & 0x7).f[0] = 1.0 / src.f[0];
		XMM((modrm >> 3) & 0x7).f[1] = 1.0 / src.f[1];
		XMM((modrm >> 3) & 0x7).f[2] = 1.0 / src.f[2];
		XMM((modrm >> 3) & 0x7).f[3] = 1.0 / src.f[3];
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(andps_r128_rm128)(i386_state *cpustate) // Opcode 0f 54
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).q[0] = XMM((modrm >> 3) & 0x7).q[0] & XMM(modrm & 0x7).q[0];
		XMM((modrm >> 3) & 0x7).q[1] = XMM((modrm >> 3) & 0x7).q[1] & XMM(modrm & 0x7).q[1];
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READXMM(cpustate, ea, src);
		XMM((modrm >> 3) & 0x7).q[0] = XMM((modrm >> 3) & 0x7).q[0] & src.q[0];
		XMM((modrm >> 3) & 0x7).q[1] = XMM((modrm >> 3) & 0x7).q[1] & src.q[1];
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(andnps_r128_rm128)(i386_state *cpustate) // Opcode 0f 55
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).q[0] = ~(XMM((modrm >> 3) & 0x7).q[0]) & XMM(modrm & 0x7).q[0];
		XMM((modrm >> 3) & 0x7).q[1] = ~(XMM((modrm >> 3) & 0x7).q[1]) & XMM(modrm & 0x7).q[1];
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READXMM(cpustate, ea, src);
		XMM((modrm >> 3) & 0x7).q[0] = ~(XMM((modrm >> 3) & 0x7).q[0]) & src.q[0];
		XMM((modrm >> 3) & 0x7).q[1] = ~(XMM((modrm >> 3) & 0x7).q[1]) & src.q[1];
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(orps_r128_rm128)(i386_state *cpustate) // Opcode 0f 56
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).q[0] = XMM((modrm >> 3) & 0x7).q[0] | XMM(modrm & 0x7).q[0];
		XMM((modrm >> 3) & 0x7).q[1] = XMM((modrm >> 3) & 0x7).q[1] | XMM(modrm & 0x7).q[1];
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READXMM(cpustate, ea, src);
		XMM((modrm >> 3) & 0x7).q[0] = XMM((modrm >> 3) & 0x7).q[0] | src.q[0];
		XMM((modrm >> 3) & 0x7).q[1] = XMM((modrm >> 3) & 0x7).q[1] | src.q[1];
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(mulps)(i386_state *cpustate) // Opcode 0f 59 ????
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).f[0] = XMM((modrm >> 3) & 0x7).f[0] * XMM(modrm & 0x7).f[0];
		XMM((modrm >> 3) & 0x7).f[1] = XMM((modrm >> 3) & 0x7).f[1] * XMM(modrm & 0x7).f[1];
		XMM((modrm >> 3) & 0x7).f[2] = XMM((modrm >> 3) & 0x7).f[2] * XMM(modrm & 0x7).f[2];
		XMM((modrm >> 3) & 0x7).f[3] = XMM((modrm >> 3) & 0x7).f[3] * XMM(modrm & 0x7).f[3];
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READXMM(cpustate, ea, src);
		XMM((modrm >> 3) & 0x7).f[0] = XMM((modrm >> 3) & 0x7).f[0] * src.f[0];
		XMM((modrm >> 3) & 0x7).f[1] = XMM((modrm >> 3) & 0x7).f[1] * src.f[1];
		XMM((modrm >> 3) & 0x7).f[2] = XMM((modrm >> 3) & 0x7).f[2] * src.f[2];
		XMM((modrm >> 3) & 0x7).f[3] = XMM((modrm >> 3) & 0x7).f[3] * src.f[3];
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(subps)(i386_state *cpustate) // Opcode 0f 5c
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).f[0] = XMM((modrm >> 3) & 0x7).f[0] - XMM(modrm & 0x7).f[0];
		XMM((modrm >> 3) & 0x7).f[1] = XMM((modrm >> 3) & 0x7).f[1] - XMM(modrm & 0x7).f[1];
		XMM((modrm >> 3) & 0x7).f[2] = XMM((modrm >> 3) & 0x7).f[2] - XMM(modrm & 0x7).f[2];
		XMM((modrm >> 3) & 0x7).f[3] = XMM((modrm >> 3) & 0x7).f[3] - XMM(modrm & 0x7).f[3];
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READXMM(cpustate, ea, src);
		XMM((modrm >> 3) & 0x7).f[0] = XMM((modrm >> 3) & 0x7).f[0] - src.f[0];
		XMM((modrm >> 3) & 0x7).f[1] = XMM((modrm >> 3) & 0x7).f[1] - src.f[1];
		XMM((modrm >> 3) & 0x7).f[2] = XMM((modrm >> 3) & 0x7).f[2] - src.f[2];
		XMM((modrm >> 3) & 0x7).f[3] = XMM((modrm >> 3) & 0x7).f[3] - src.f[3];
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
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

static void SSEOP(minps)(i386_state *cpustate) // Opcode 0f 5d
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).f[0] = sse_min_single(XMM((modrm >> 3) & 0x7).f[0], XMM(modrm & 0x7).f[0]);
		XMM((modrm >> 3) & 0x7).f[1] = sse_min_single(XMM((modrm >> 3) & 0x7).f[1], XMM(modrm & 0x7).f[1]);
		XMM((modrm >> 3) & 0x7).f[2] = sse_min_single(XMM((modrm >> 3) & 0x7).f[2], XMM(modrm & 0x7).f[2]);
		XMM((modrm >> 3) & 0x7).f[3] = sse_min_single(XMM((modrm >> 3) & 0x7).f[3], XMM(modrm & 0x7).f[3]);
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READXMM(cpustate, ea, src);
		XMM((modrm >> 3) & 0x7).f[0] = sse_min_single(XMM((modrm >> 3) & 0x7).f[0], src.f[0]);
		XMM((modrm >> 3) & 0x7).f[1] = sse_min_single(XMM((modrm >> 3) & 0x7).f[1], src.f[1]);
		XMM((modrm >> 3) & 0x7).f[2] = sse_min_single(XMM((modrm >> 3) & 0x7).f[2], src.f[2]);
		XMM((modrm >> 3) & 0x7).f[3] = sse_min_single(XMM((modrm >> 3) & 0x7).f[3], src.f[3]);
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(divps)(i386_state *cpustate) // Opcode 0f 5e
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).f[0] = XMM((modrm >> 3) & 0x7).f[0] / XMM(modrm & 0x7).f[0];
		XMM((modrm >> 3) & 0x7).f[1] = XMM((modrm >> 3) & 0x7).f[1] / XMM(modrm & 0x7).f[1];
		XMM((modrm >> 3) & 0x7).f[2] = XMM((modrm >> 3) & 0x7).f[2] / XMM(modrm & 0x7).f[2];
		XMM((modrm >> 3) & 0x7).f[3] = XMM((modrm >> 3) & 0x7).f[3] / XMM(modrm & 0x7).f[3];
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READXMM(cpustate, ea, src);
		XMM((modrm >> 3) & 0x7).f[0] = XMM((modrm >> 3) & 0x7).f[0] / src.f[0];
		XMM((modrm >> 3) & 0x7).f[1] = XMM((modrm >> 3) & 0x7).f[1] / src.f[1];
		XMM((modrm >> 3) & 0x7).f[2] = XMM((modrm >> 3) & 0x7).f[2] / src.f[2];
		XMM((modrm >> 3) & 0x7).f[3] = XMM((modrm >> 3) & 0x7).f[3] / src.f[3];
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
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

static void SSEOP(maxps)(i386_state *cpustate) // Opcode 0f 5f
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).f[0] = sse_max_single(XMM((modrm >> 3) & 0x7).f[0], XMM(modrm & 0x7).f[0]);
		XMM((modrm >> 3) & 0x7).f[1] = sse_max_single(XMM((modrm >> 3) & 0x7).f[1], XMM(modrm & 0x7).f[1]);
		XMM((modrm >> 3) & 0x7).f[2] = sse_max_single(XMM((modrm >> 3) & 0x7).f[2], XMM(modrm & 0x7).f[2]);
		XMM((modrm >> 3) & 0x7).f[3] = sse_max_single(XMM((modrm >> 3) & 0x7).f[3], XMM(modrm & 0x7).f[3]);
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READXMM(cpustate, ea, src);
		XMM((modrm >> 3) & 0x7).f[0] = sse_max_single(XMM((modrm >> 3) & 0x7).f[0], src.f[0]);
		XMM((modrm >> 3) & 0x7).f[1] = sse_max_single(XMM((modrm >> 3) & 0x7).f[1], src.f[1]);
		XMM((modrm >> 3) & 0x7).f[2] = sse_max_single(XMM((modrm >> 3) & 0x7).f[2], src.f[2]);
		XMM((modrm >> 3) & 0x7).f[3] = sse_max_single(XMM((modrm >> 3) & 0x7).f[3], src.f[3]);
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(maxss_r128_r128m32)(i386_state *cpustate) // Opcode f3 0f 5f
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).f[0] = sse_max_single(XMM((modrm >> 3) & 0x7).f[0], XMM(modrm & 0x7).f[0]);
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		src.d[0]=READ32(cpustate, ea);
		XMM((modrm >> 3) & 0x7).f[0] = sse_max_single(XMM((modrm >> 3) & 0x7).f[0], src.f[0]);
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(addss)(i386_state *cpustate) // Opcode f3 0f 58
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).f[0] = XMM((modrm >> 3) & 0x7).f[0] + XMM(modrm & 0x7).f[0];
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READXMM(cpustate, ea, src);
		XMM((modrm >> 3) & 0x7).f[0] = XMM((modrm >> 3) & 0x7).f[0] + src.f[0];
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(subss)(i386_state *cpustate) // Opcode f3 0f 5c
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).f[0] = XMM((modrm >> 3) & 0x7).f[0] - XMM(modrm & 0x7).f[0];
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READXMM(cpustate, ea, src);
		XMM((modrm >> 3) & 0x7).f[0] = XMM((modrm >> 3) & 0x7).f[0] - src.f[0];
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(mulss)(i386_state *cpustate) // Opcode f3 0f 5e
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).f[0] = XMM((modrm >> 3) & 0x7).f[0] * XMM(modrm & 0x7).f[0];
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READXMM(cpustate, ea, src);
		XMM((modrm >> 3) & 0x7).f[0] = XMM((modrm >> 3) & 0x7).f[0] * src.f[0];
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(divss)(i386_state *cpustate) // Opcode 0f 59
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).f[0] = XMM((modrm >> 3) & 0x7).f[0] / XMM(modrm & 0x7).f[0];
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READXMM(cpustate, ea, src);
		XMM((modrm >> 3) & 0x7).f[0] = XMM((modrm >> 3) & 0x7).f[0] / src.f[0];
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(rcpss_r128_r128m32)(i386_state *cpustate) // Opcode f3 0f 53
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).f[0] = 1.0 / XMM(modrm & 0x7).f[0];
	} else {
		XMM_REG s;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		s.d[0]=READ32(cpustate, ea);
		XMM((modrm >> 3) & 0x7).f[0] = 1.0 / s.f[0];
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(sqrtss_r128_r128m32)(i386_state *cpustate) // Opcode f3 0f 51
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).f[0] = sqrt(XMM(modrm & 0x7).f[0]);
	} else {
		XMM_REG s;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		s.d[0]=READ32(cpustate, ea);
		XMM((modrm >> 3) & 0x7).f[0] = sqrt(s.f[0]);
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(rsqrtss_r128_r128m32)(i386_state *cpustate) // Opcode f3 0f 52
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).f[0] = 1.0 / sqrt(XMM(modrm & 0x7).f[0]);
	} else {
		XMM_REG s;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		s.d[0]=READ32(cpustate, ea);
		XMM((modrm >> 3) & 0x7).f[0] = 1.0 / sqrt(s.f[0]);
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(minss_r128_r128m32)(i386_state *cpustate) // Opcode f3 0f 5d
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		XMM((modrm >> 3) & 0x7).f[0] = XMM((modrm >> 3) & 0x7).f[0] < XMM(modrm & 0x7).f[0] ? XMM((modrm >> 3) & 0x7).f[0] : XMM(modrm & 0x7).f[0];
	} else {
		XMM_REG s;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		s.d[0] = READ32(cpustate, ea);
		XMM((modrm >> 3) & 0x7).f[0] = XMM((modrm >> 3) & 0x7).f[0] < s.f[0] ? XMM((modrm >> 3) & 0x7).f[0] : s.f[0];
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(comiss_r128_r128m32)(i386_state *cpustate) // Opcode 0f 2f
{
	float32 a,b;
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		a = XMM((modrm >> 3) & 0x7).d[0];
		b = XMM(modrm & 0x7).d[0];
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READXMM(cpustate, ea, src);
		a = XMM((modrm >> 3) & 0x7).d[0];
		b = src.d[0];
	}
	cpustate->OF=0;
	cpustate->SF=0;
	cpustate->AF=0;
	if (float32_is_nan(a) || float32_is_nan(b))
	{
		cpustate->ZF = 1;
		cpustate->PF = 1;
		cpustate->CF = 1;
	}
	else
	{
		cpustate->ZF = 0;
		cpustate->PF = 0;
		cpustate->CF = 0;
		if (float32_eq(a, b))
			cpustate->ZF = 1;
		if (float32_lt(a, b))
			cpustate->CF = 1;
	}
	// should generate exception when at least one of the operands is either QNaN or SNaN
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(ucomiss_r128_r128m32)(i386_state *cpustate) // Opcode 0f 2e
{
	float32 a,b;
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		a = XMM((modrm >> 3) & 0x7).d[0];
		b = XMM(modrm & 0x7).d[0];
	} else {
		XMM_REG src;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READXMM(cpustate, ea, src);
		a = XMM((modrm >> 3) & 0x7).d[0];
		b = src.d[0];
	}
	cpustate->OF=0;
	cpustate->SF=0;
	cpustate->AF=0;
	if (float32_is_nan(a) || float32_is_nan(b))
	{
		cpustate->ZF = 1;
		cpustate->PF = 1;
		cpustate->CF = 1;
	}
	else
	{
		cpustate->ZF = 0;
		cpustate->PF = 0;
		cpustate->CF = 0;
		if (float32_eq(a, b))
			cpustate->ZF = 1;
		if (float32_lt(a, b))
			cpustate->CF = 1;
	}
	// should generate exception when at least one of the operands is SNaN
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(shufps)(i386_state *cpustate) // Opcode 0f 67
{
	UINT8 modrm = FETCH(cpustate);
	UINT8 sel = FETCH(cpustate);
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
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READXMM(cpustate, ea, src);
		t=XMM(d).d[m1];
		XMM(d).d[1]=XMM(d).d[m2];
		XMM(d).d[0]=t;
		XMM(d).d[2]=src.d[m3];
		XMM(d).d[3]=src.d[m4];
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(unpcklps_r128_rm128)(i386_state *cpustate) // Opcode 0f 14
{
	UINT8 modrm = FETCH(cpustate);
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
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READXMM(cpustate, ea, src);
		XMM(d).d[3]=src.d[1];
		XMM(d).d[2]=XMM(d).d[1];
		XMM(d).d[1]=src.d[0];
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(unpckhps_r128_rm128)(i386_state *cpustate) // Opcode 0f 15
{
	UINT8 modrm = FETCH(cpustate);
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
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READXMM(cpustate, ea, src);
		XMM(d).d[0]=XMM(d).d[2];
		XMM(d).d[1]=src.d[2];
		XMM(d).d[2]=XMM(d).d[3];
		XMM(d).d[3]=src.d[3];
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
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

INLINE void sse_predicate_compare_single(UINT8 imm8, XMM_REG d, XMM_REG s)
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

INLINE void sse_predicate_compare_single_scalar(UINT8 imm8, XMM_REG d, XMM_REG s)
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

static void SSEOP(cmpps_r128_rm128_i8)(i386_state *cpustate) // Opcode 0f c2
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		int s,d;
		UINT8 imm8 = FETCH(cpustate);
		s=modrm & 0x7;
		d=(modrm >> 3) & 0x7;
		sse_predicate_compare_single(imm8, XMM(d), XMM(s));
	} else {
		int d;
		XMM_REG s;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		UINT8 imm8 = FETCH(cpustate);
		READXMM(cpustate, ea, s);
		d=(modrm >> 3) & 0x7;
		sse_predicate_compare_single(imm8, XMM(d), s);
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(cmpss_r128_r128m32_i8)(i386_state *cpustate) // Opcode f3 0f c2
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		int s,d;
		UINT8 imm8 = FETCH(cpustate);
		s=modrm & 0x7;
		d=(modrm >> 3) & 0x7;
		sse_predicate_compare_single_scalar(imm8, XMM(d), XMM(s));
	} else {
		int d;
		XMM_REG s;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		UINT8 imm8 = FETCH(cpustate);
		s.d[0]=READ32(cpustate, ea);
		d=(modrm >> 3) & 0x7;
		sse_predicate_compare_single_scalar(imm8, XMM(d), s);
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(pinsrw_r64_r16m16_i8)(i386_state *cpustate) // Opcode 0f c4
{
	MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		UINT8 imm8 = FETCH(cpustate);
		UINT16 v = LOAD_RM16(modrm);
		MMX((modrm >> 3) & 0x7).w[imm8 & 3] = v;
	} else {
		UINT32 ea = GetEA(cpustate, modrm, 0);
		UINT8 imm8 = FETCH(cpustate);
		UINT16 v = READ16(cpustate, ea);
		MMX((modrm >> 3) & 0x7).w[imm8 & 3] = v;
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(pinsrw_r64_r32m16_i8)(i386_state *cpustate) // Opcode 0f c4
{
	MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		UINT8 imm8 = FETCH(cpustate);
		UINT16 v = (UINT16)LOAD_RM32(modrm);
		MMX((modrm >> 3) & 0x7).w[imm8 & 3] = v;
	} else {
		UINT32 ea = GetEA(cpustate, modrm, 0);
		UINT8 imm8 = FETCH(cpustate);
		UINT16 v = READ16(cpustate, ea);
		MMX((modrm >> 3) & 0x7).w[imm8 & 3] = v;
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(pextrw_r16_r64_i8)(i386_state *cpustate) // Opcode 0f c5
{
	//MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		UINT8 imm8 = FETCH(cpustate);
		STORE_REG16(modrm, MMX(modrm & 0x7).w[imm8 & 3]);
	} else {
		//UINT8 imm8 = FETCH(cpustate);
		report_invalid_modrm(cpustate, "pextrw_r16_r64_i8", modrm);
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(pextrw_r32_r64_i8)(i386_state *cpustate) // Opcode 0f c5
{
	//MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		UINT8 imm8 = FETCH(cpustate);
		STORE_REG32(modrm, MMX(modrm & 0x7).w[imm8 & 3]);
	} else {
		//UINT8 imm8 = FETCH(cpustate);
		report_invalid_modrm(cpustate, "pextrw_r32_r64_i8", modrm);
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(pminub_r64_rm64)(i386_state *cpustate) // Opcode 0f da
{
	int n;
	MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		for (n=0;n < 8;n++)
			MMX((modrm >> 3) & 0x7).b[n] = MMX((modrm >> 3) & 0x7).b[n] < MMX(modrm & 0x7).b[n] ? MMX((modrm >> 3) & 0x7).b[n] : MMX(modrm & 0x7).b[n];
	} else {
		MMX_REG s;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READMMX(cpustate, ea, s);
		for (n=0;n < 8;n++)
			MMX((modrm >> 3) & 0x7).b[n] = MMX((modrm >> 3) & 0x7).b[n] < s.b[n] ? MMX((modrm >> 3) & 0x7).b[n] : s.b[n];
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(pmaxub_r64_rm64)(i386_state *cpustate) // Opcode 0f de
{
	int n;
	MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		for (n=0;n < 8;n++)
			MMX((modrm >> 3) & 0x7).b[n] = MMX((modrm >> 3) & 0x7).b[n] > MMX(modrm & 0x7).b[n] ? MMX((modrm >> 3) & 0x7).b[n] : MMX(modrm & 0x7).b[n];
	} else {
		MMX_REG s;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READMMX(cpustate, ea, s);
		for (n=0;n < 8;n++)
			MMX((modrm >> 3) & 0x7).b[n] = MMX((modrm >> 3) & 0x7).b[n] > s.b[n] ? MMX((modrm >> 3) & 0x7).b[n] : s.b[n];
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(pavgb_r64_rm64)(i386_state *cpustate) // Opcode 0f e0
{
	int n;
	MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		for (n=0;n < 8;n++)
			MMX((modrm >> 3) & 0x7).b[n] = ((UINT16)MMX((modrm >> 3) & 0x7).b[n] + (UINT16)MMX(modrm & 0x7).b[n] + 1) >> 1;
	} else {
		MMX_REG s;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READMMX(cpustate, ea, s);
		for (n=0;n < 8;n++)
			MMX((modrm >> 3) & 0x7).b[n] = ((UINT16)MMX((modrm >> 3) & 0x7).b[n] + (UINT16)s.b[n] + 1) >> 1;
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(pavgw_r64_rm64)(i386_state *cpustate) // Opcode 0f e3
{
	int n;
	MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		for (n=0;n < 4;n++)
			MMX((modrm >> 3) & 0x7).w[n] = ((UINT32)MMX((modrm >> 3) & 0x7).w[n] + (UINT32)MMX(modrm & 0x7).w[n] + 1) >> 1;
	} else {
		MMX_REG s;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READMMX(cpustate, ea, s);
		for (n=0;n < 4;n++)
			MMX((modrm >> 3) & 0x7).w[n] = ((UINT32)MMX((modrm >> 3) & 0x7).w[n] + (UINT32)s.w[n] + 1) >> 1;
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(pmulhuw_r64_rm64)(i386_state *cpustate)  // Opcode 0f e4
{
	MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		MMX((modrm >> 3) & 0x7).w[0]=((UINT32)MMX((modrm >> 3) & 0x7).w[0]*(UINT32)MMX(modrm & 7).w[0]) >> 16;
		MMX((modrm >> 3) & 0x7).w[1]=((UINT32)MMX((modrm >> 3) & 0x7).w[1]*(UINT32)MMX(modrm & 7).w[1]) >> 16;
		MMX((modrm >> 3) & 0x7).w[2]=((UINT32)MMX((modrm >> 3) & 0x7).w[2]*(UINT32)MMX(modrm & 7).w[2]) >> 16;
		MMX((modrm >> 3) & 0x7).w[3]=((UINT32)MMX((modrm >> 3) & 0x7).w[3]*(UINT32)MMX(modrm & 7).w[3]) >> 16;
	} else {
		MMX_REG s;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READMMX(cpustate, ea, s);
		MMX((modrm >> 3) & 0x7).w[0]=((UINT32)MMX((modrm >> 3) & 0x7).w[0]*(UINT32)s.w[0]) >> 16;
		MMX((modrm >> 3) & 0x7).w[1]=((UINT32)MMX((modrm >> 3) & 0x7).w[1]*(UINT32)s.w[1]) >> 16;
		MMX((modrm >> 3) & 0x7).w[2]=((UINT32)MMX((modrm >> 3) & 0x7).w[2]*(UINT32)s.w[2]) >> 16;
		MMX((modrm >> 3) & 0x7).w[3]=((UINT32)MMX((modrm >> 3) & 0x7).w[3]*(UINT32)s.w[3]) >> 16;
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(pminsw_r64_rm64)(i386_state *cpustate) // Opcode 0f ea
{
	int n;
	MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		for (n=0;n < 4;n++)
			MMX((modrm >> 3) & 0x7).s[n] = MMX((modrm >> 3) & 0x7).s[n] < MMX(modrm & 0x7).s[n] ? MMX((modrm >> 3) & 0x7).s[n] : MMX(modrm & 0x7).s[n];
	} else {
		MMX_REG s;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READMMX(cpustate, ea, s);
		for (n=0;n < 4;n++)
			MMX((modrm >> 3) & 0x7).s[n] = MMX((modrm >> 3) & 0x7).s[n] < s.s[n] ? MMX((modrm >> 3) & 0x7).s[n] : s.s[n];
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(pmaxsw_r64_rm64)(i386_state *cpustate) // Opcode 0f ee
{
	int n;
	MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		for (n=0;n < 4;n++)
			MMX((modrm >> 3) & 0x7).s[n] = MMX((modrm >> 3) & 0x7).s[n] > MMX(modrm & 0x7).s[n] ? MMX((modrm >> 3) & 0x7).s[n] : MMX(modrm & 0x7).s[n];
	} else {
		MMX_REG s;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READMMX(cpustate, ea, s);
		for (n=0;n < 4;n++)
			MMX((modrm >> 3) & 0x7).s[n] = MMX((modrm >> 3) & 0x7).s[n] > s.s[n] ? MMX((modrm >> 3) & 0x7).s[n] : s.s[n];
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(pmuludq_r64_rm64)(i386_state *cpustate) // Opcode 0f f4
{
	MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		MMX((modrm >> 3) & 0x7).q = (UINT64)MMX((modrm >> 3) & 0x7).d[0] * (UINT64)MMX(modrm & 0x7).d[0];
	} else {
		MMX_REG s;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READMMX(cpustate, ea, s);
		MMX((modrm >> 3) & 0x7).q = (UINT64)MMX((modrm >> 3) & 0x7).d[0] * (UINT64)s.d[0];
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(psadbw_r64_rm64)(i386_state *cpustate) // Opcode 0f f6
{
	int n;
	INT32 temp;
	MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		temp=0;
		for (n=0;n < 8;n++)
			temp += abs((INT32)MMX((modrm >> 3) & 0x7).b[n] - (INT32)MMX(modrm & 0x7).b[n]);
		MMX((modrm >> 3) & 0x7).l=(UINT64)temp & 0xffff;
	} else {
		MMX_REG s;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READMMX(cpustate, ea, s);
		temp=0;
		for (n=0;n < 8;n++)
			temp += abs((INT32)MMX((modrm >> 3) & 0x7).b[n] - (INT32)s.b[n]);
		MMX((modrm >> 3) & 0x7).l=(UINT64)temp & 0xffff;
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(psubq_r64_rm64)(i386_state *cpustate)  // Opcode 0f fb
{
	MMXPROLOG(cpustate);
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		MMX((modrm >> 3) & 0x7).q=MMX((modrm >> 3) & 0x7).q - MMX(modrm & 7).q;
	} else {
		MMX_REG s;
		UINT32 ea = GetEA(cpustate, modrm, 0);
		READMMX(cpustate, ea, s);
		MMX((modrm >> 3) & 0x7).q=MMX((modrm >> 3) & 0x7).q - s.q;
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}

static void SSEOP(pshufhw_r128_rm128_i8)(i386_state *cpustate) // Opcode f3 0f 70
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		XMM_REG t;
		int s,d;
		UINT8 imm8 = FETCH(cpustate);
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
		UINT32 ea = GetEA(cpustate, modrm, 0);
		UINT8 imm8 = FETCH(cpustate);
		READXMM(cpustate, ea, s);
		XMM(d).q[0]=s.q[0];
		XMM(d).w[4]=s.w[4 + (imm8 & 3)];
		XMM(d).w[5]=s.w[4 + ((imm8 >> 2) & 3)];
		XMM(d).w[6]=s.w[4 + ((imm8 >> 4) & 3)];
		XMM(d).w[7]=s.w[4 + ((imm8 >> 6) & 3)];
	}
	CYCLES(cpustate,1);     // TODO: correct cycle count
}
