// Pentium+ specific opcodes

static void PENTIUMOP(rdmsr)(i386_state *cpustate)			// Opcode 0x0f 32
{
	// TODO
	CYCLES(cpustate,CYCLES_RDMSR);
}

static void PENTIUMOP(wrmsr)(i386_state *cpustate)			// Opcode 0x0f 30
{
	// TODO
	CYCLES(cpustate,1);		// TODO: correct cycle count
}

static void PENTIUMOP(rdtsc)(i386_state *cpustate)			// Opcode 0x0f 31
{
	UINT64 ts = cpustate->tsc + (cpustate->base_cycles - cpustate->cycles);
	REG32(EAX) = (UINT32)(ts);
	REG32(EDX) = (UINT32)(ts >> 32);

	CYCLES(cpustate,CYCLES_RDTSC);
}

static void I386OP(cyrix_unknown)(i386_state *cpustate)		// Opcode 0x0f 74
{
	CYCLES(cpustate,1);
}

static void PENTIUMOP(cmpxchg8b_m64)(i386_state *cpustate)	// Opcode 0x0f c7
{
	UINT8 modm = FETCH(cpustate);
	if( modm >= 0xc0 ) {
		fatalerror("pentium: cmpxchg8b_m64 - invalid modm");
	} else {
		UINT32 ea = GetEA(cpustate,modm);
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

static void PENTIUMOP(sse_group0fae)(i386_state *cpustate)	// Opcode 0x0f ae
{
	UINT8 modm = FETCH(cpustate);
	if( modm == 0xf8 ) {
		CYCLES(cpustate,1); // sfence instruction
	} else {
		fatalerror("pentium: bad/unsupported 0f ae opcode");
	}
}

