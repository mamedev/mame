// Pentium+ specific opcodes

/* return the single precision floating point number represented by the 32 bit value */
INLINE float FPU_INT32_SINGLE(UINT32 value)
{
	float v;

	v=*((float *)&value);
	return v;
}

INLINE UINT32 FPU_SINGLE_INT32(X87_REG value)
{
	float fs=(float)value.f;
	UINT32 v;

	v=*((UINT32 *)(&fs));
	return v;
}

INLINE void MMXPROLOG(i386_state *cpustate)
{
	//cpustate->x87_sw &= ~(X87_SW_TOP_MASK << X87_SW_TOP_SHIFT); // top = 0
	cpustate->x87_tw = 0; // tag word = 0
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
	if(cpustate->nmi_latched)
	{
		cpustate->nmi_latched = false;
		i386_trap(cpustate, 2, 1, 0);
	}
}

static void SSEOP(cvttss2si)(i386_state *cpustate) // Opcode f3 0f 2c
{
	UINT32 src;
	UINT8 modrm = FETCH(cpustate); // get mordm byte
	if( modrm >= 0xc0 ) { // if bits 7-6 are 11 the source is a xmm register (low doubleword)
		src = XMM(modrm & 0x7).d[0^NATIVE_ENDIAN_VALUE_LE_BE(0,1)];
	} else { // otherwise is a memory address
		UINT32 ea = GetEA(cpustate, modrm, 0);
		src = READ32(cpustate, ea);
	}
	STORE_REG32(modrm, (INT32)FPU_INT32_SINGLE(src));
	// TODO
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
}

static void SSEOP(mulps)(i386_state *cpustate) // Opcode 0f 59
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
}

static void MMXOP(emms)(i386_state *cpustate) // Opcode 0f 77
{
	cpustate->x87_tw = 0xffff; // tag word = 0xffff
	// TODO
	CYCLES(cpustate,1);     // TODO: correct cycle count
}
