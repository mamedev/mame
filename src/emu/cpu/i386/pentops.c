// Pentium+ specific opcodes

static void PENTIUMOP(rdmsr)(void)			// Opcode 0x0f 32
{
	// TODO
	CYCLES(CYCLES_RDMSR);
}

static void PENTIUMOP(wrmsr)(void)			// Opcode 0x0f 30
{
	// TODO
	CYCLES(1);		// TODO: correct cycle count
}

static void PENTIUMOP(rdtsc)(void)			// Opcode 0x0f 31
{
	UINT64 ts = I.tsc + (I.base_cycles - I.cycles);
	REG32(EAX) = (UINT32)(ts);
	REG32(EDX) = (UINT32)(ts >> 32);

	CYCLES(CYCLES_RDTSC);
}

static void I386OP(cyrix_unknown)(void)		// Opcode 0x0f 74
{
	CYCLES(1);
}

static void PENTIUMOP(cmpxchg8b_m64)(void)	// Opcode 0x0f c7
{
	UINT8 modm = FETCH();
	if( modm >= 0xc0 ) {
		fatalerror("invalid modm");
	} else {
		UINT32 ea = GetEA(modm);
		UINT64 value = READ64(ea);
		UINT64 edx_eax = (((UINT64) REG32(EDX)) << 32) | REG32(EAX);
		UINT64 ecx_ebx = (((UINT64) REG32(ECX)) << 32) | REG32(EBX);

		if( value == edx_eax ) {
			WRITE64(ea, ecx_ebx);
			I.ZF = 1;
			CYCLES(CYCLES_CMPXCHG_REG_MEM_T);
		} else {
			REG32(EDX) = (UINT32) (value >> 32);
			REG32(EAX) = (UINT32) (value >>  0);
			I.ZF = 0;
			CYCLES(CYCLES_CMPXCHG_REG_MEM_F);
		}
	}
}


