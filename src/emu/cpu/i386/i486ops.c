// Intel 486+ specific opcodes

static void I486OP(cpuid)(i386_state *cpustate)             // Opcode 0x0F A2
{
	if (cpustate->cpuid_id0 == 0)
	{
		// this 486 doesn't support the CPUID instruction
		logerror("CPUID not supported at %08x!\n", cpustate->eip);
		i386_trap(cpustate, 6, 0, 0);
	}
	else
	{
		switch (REG32(EAX))
		{
			case 0:
			{
				REG32(EAX) = cpustate->cpuid_max_input_value_eax;
				REG32(EBX) = cpustate->cpuid_id0;
				REG32(ECX) = cpustate->cpuid_id2;
				REG32(EDX) = cpustate->cpuid_id1;
				CYCLES(cpustate,CYCLES_CPUID);
				break;
			}

			case 1:
			{
				REG32(EAX) = cpustate->cpu_version;
				REG32(EDX) = cpustate->feature_flags;
				CYCLES(cpustate,CYCLES_CPUID_EAX1);
				break;
			}
		}
	}
}

static void I486OP(invd)(i386_state *cpustate)              // Opcode 0x0f 08
{
	// Nothing to do ?
	CYCLES(cpustate,CYCLES_INVD);
}

static void I486OP(wbinvd)(i386_state *cpustate)            // Opcode 0x0f 09
{
	// Nothing to do ?
}

static void I486OP(cmpxchg_rm8_r8)(i386_state *cpustate)    // Opcode 0x0f b0
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		UINT8 dst = LOAD_RM8(modrm);
		UINT8 src = LOAD_REG8(modrm);

		if( REG8(AL) == dst ) {
			STORE_RM8(modrm, src);
			cpustate->ZF = 1;
			CYCLES(cpustate,CYCLES_CMPXCHG_REG_REG_T);
		} else {
			REG8(AL) = dst;
			cpustate->ZF = 0;
			CYCLES(cpustate,CYCLES_CMPXCHG_REG_REG_F);
		}
	} else {
		// TODO: Check write if needed
		UINT32 ea = GetEA(cpustate,modrm,0);
		UINT8 dst = READ8(cpustate,ea);
		UINT8 src = LOAD_REG8(modrm);

		if( REG8(AL) == dst ) {
			WRITE8(cpustate,ea, src);
			cpustate->ZF = 1;
			CYCLES(cpustate,CYCLES_CMPXCHG_REG_MEM_T);
		} else {
			REG8(AL) = dst;
			cpustate->ZF = 0;
			CYCLES(cpustate,CYCLES_CMPXCHG_REG_MEM_F);
		}
	}
}

static void I486OP(cmpxchg_rm16_r16)(i386_state *cpustate)  // Opcode 0x0f b1
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		UINT16 dst = LOAD_RM16(modrm);
		UINT16 src = LOAD_REG16(modrm);

		if( REG16(AX) == dst ) {
			STORE_RM16(modrm, src);
			cpustate->ZF = 1;
			CYCLES(cpustate,CYCLES_CMPXCHG_REG_REG_T);
		} else {
			REG16(AX) = dst;
			cpustate->ZF = 0;
			CYCLES(cpustate,CYCLES_CMPXCHG_REG_REG_F);
		}
	} else {
		UINT32 ea = GetEA(cpustate,modrm,0);
		UINT16 dst = READ16(cpustate,ea);
		UINT16 src = LOAD_REG16(modrm);

		if( REG16(AX) == dst ) {
			WRITE16(cpustate,ea, src);
			cpustate->ZF = 1;
			CYCLES(cpustate,CYCLES_CMPXCHG_REG_MEM_T);
		} else {
			REG16(AX) = dst;
			cpustate->ZF = 0;
			CYCLES(cpustate,CYCLES_CMPXCHG_REG_MEM_F);
		}
	}
}

static void I486OP(cmpxchg_rm32_r32)(i386_state *cpustate)  // Opcode 0x0f b1
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		UINT32 dst = LOAD_RM32(modrm);
		UINT32 src = LOAD_REG32(modrm);

		if( REG32(EAX) == dst ) {
			STORE_RM32(modrm, src);
			cpustate->ZF = 1;
			CYCLES(cpustate,CYCLES_CMPXCHG_REG_REG_T);
		} else {
			REG32(EAX) = dst;
			cpustate->ZF = 0;
			CYCLES(cpustate,CYCLES_CMPXCHG_REG_REG_F);
		}
	} else {
		UINT32 ea = GetEA(cpustate,modrm,0);
		UINT32 dst = READ32(cpustate,ea);
		UINT32 src = LOAD_REG32(modrm);

		if( REG32(EAX) == dst ) {
			WRITE32(cpustate,ea, src);
			cpustate->ZF = 1;
			CYCLES(cpustate,CYCLES_CMPXCHG_REG_MEM_T);
		} else {
			REG32(EAX) = dst;
			cpustate->ZF = 0;
			CYCLES(cpustate,CYCLES_CMPXCHG_REG_MEM_F);
		}
	}
}

static void I486OP(xadd_rm8_r8)(i386_state *cpustate)   // Opcode 0x0f c0
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		UINT8 dst = LOAD_RM8(modrm);
		UINT8 src = LOAD_REG8(modrm);
		STORE_REG8(modrm, dst);
		STORE_RM8(modrm, dst + src);
		CYCLES(cpustate,CYCLES_XADD_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		UINT8 dst = READ8(cpustate,ea);
		UINT8 src = LOAD_REG8(modrm);
		WRITE8(cpustate,ea, dst + src);
		STORE_REG8(modrm, dst);
		CYCLES(cpustate,CYCLES_XADD_REG_MEM);
	}
}

static void I486OP(xadd_rm16_r16)(i386_state *cpustate) // Opcode 0x0f c1
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		UINT16 dst = LOAD_RM16(modrm);
		UINT16 src = LOAD_REG16(modrm);
		STORE_REG16(modrm, dst);
		STORE_RM16(modrm, dst + src);
		CYCLES(cpustate,CYCLES_XADD_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		UINT16 dst = READ16(cpustate,ea);
		UINT16 src = LOAD_REG16(modrm);
		WRITE16(cpustate,ea, dst + src);
		STORE_REG16(modrm, dst);
		CYCLES(cpustate,CYCLES_XADD_REG_MEM);
	}
}

static void I486OP(xadd_rm32_r32)(i386_state *cpustate) // Opcode 0x0f c1
{
	UINT8 modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		UINT32 dst = LOAD_RM32(modrm);
		UINT32 src = LOAD_REG32(modrm);
		STORE_REG32(modrm, dst);
		STORE_RM32(modrm, dst + src);
		CYCLES(cpustate,CYCLES_XADD_REG_REG);
	} else {
		UINT32 ea = GetEA(cpustate,modrm,1);
		UINT32 dst = READ32(cpustate,ea);
		UINT32 src = LOAD_REG32(modrm);
		WRITE32(cpustate,ea, dst + src);
		STORE_REG32(modrm, dst);
		CYCLES(cpustate,CYCLES_XADD_REG_MEM);
	}
}

static void I486OP(group0F01_16)(i386_state *cpustate)      // Opcode 0x0f 01
{
	UINT8 modrm = FETCH(cpustate);
	UINT16 address;
	UINT32 ea;

	switch( (modrm >> 3) & 0x7 )
	{
		case 0:         /* SGDT */
			{
				if( modrm >= 0xc0 ) {
					address = LOAD_RM16(modrm);
					ea = i386_translate( cpustate, CS, address, 1 );
				} else {
					ea = GetEA(cpustate,modrm,1);
				}
				WRITE16(cpustate,ea, cpustate->gdtr.limit);
				WRITE32(cpustate,ea + 2, cpustate->gdtr.base & 0xffffff);
				CYCLES(cpustate,CYCLES_SGDT);
				break;
			}
		case 1:         /* SIDT */
			{
				if (modrm >= 0xc0)
				{
					address = LOAD_RM16(modrm);
					ea = i386_translate( cpustate, CS, address, 1 );
				}
				else
				{
					ea = GetEA(cpustate,modrm,1);
				}
				WRITE16(cpustate,ea, cpustate->idtr.limit);
				WRITE32(cpustate,ea + 2, cpustate->idtr.base & 0xffffff);
				CYCLES(cpustate,CYCLES_SIDT);
				break;
			}
		case 2:         /* LGDT */
			{
				if(PROTECTED_MODE && cpustate->CPL)
					FAULT(FAULT_GP,0)
				if( modrm >= 0xc0 ) {
					address = LOAD_RM16(modrm);
					ea = i386_translate( cpustate, CS, address, 0 );
				} else {
					ea = GetEA(cpustate,modrm,0);
				}
				cpustate->gdtr.limit = READ16(cpustate,ea);
				cpustate->gdtr.base = READ32(cpustate,ea + 2) & 0xffffff;
				CYCLES(cpustate,CYCLES_LGDT);
				break;
			}
		case 3:         /* LIDT */
			{
				if(PROTECTED_MODE && cpustate->CPL)
					FAULT(FAULT_GP,0)
				if( modrm >= 0xc0 ) {
					address = LOAD_RM16(modrm);
					ea = i386_translate( cpustate, CS, address, 0 );
				} else {
					ea = GetEA(cpustate,modrm,0);
				}
				cpustate->idtr.limit = READ16(cpustate,ea);
				cpustate->idtr.base = READ32(cpustate,ea + 2) & 0xffffff;
				CYCLES(cpustate,CYCLES_LIDT);
				break;
			}
		case 4:         /* SMSW */
			{
				if( modrm >= 0xc0 ) {
					STORE_RM16(modrm, cpustate->cr[0]);
					CYCLES(cpustate,CYCLES_SMSW_REG);
				} else {
					ea = GetEA(cpustate,modrm,1);
					WRITE16(cpustate,ea, cpustate->cr[0]);
					CYCLES(cpustate,CYCLES_SMSW_MEM);
				}
				break;
			}
		case 6:         /* LMSW */
			{
				UINT16 b;
				if(PROTECTED_MODE && cpustate->CPL)
					FAULT(FAULT_GP,0)
				if( modrm >= 0xc0 ) {
					b = LOAD_RM16(modrm);
					CYCLES(cpustate,CYCLES_LMSW_REG);
				} else {
					ea = GetEA(cpustate,modrm,0);
					CYCLES(cpustate,CYCLES_LMSW_MEM);
					b = READ16(cpustate,ea);
				}
				if(PROTECTED_MODE)
					b |= 0x0001;  // cannot return to real mode using this instruction.
				cpustate->cr[0] &= ~0x0000000f;
				cpustate->cr[0] |= b & 0x0000000f;
				break;
			}
		case 7:         /* INVLPG */
			{
				// Nothing to do ?
				break;
			}
		default:
			fatalerror("i486: unimplemented opcode 0x0f 01 /%d at %08X\n", (modrm >> 3) & 0x7, cpustate->eip - 2);
			break;
	}
}

static void I486OP(group0F01_32)(i386_state *cpustate)      // Opcode 0x0f 01
{
	UINT8 modrm = FETCH(cpustate);
	UINT32 address, ea;

	switch( (modrm >> 3) & 0x7 )
	{
		case 0:         /* SGDT */
			{
				if( modrm >= 0xc0 ) {
					address = LOAD_RM32(modrm);
					ea = i386_translate( cpustate, CS, address, 1 );
				} else {
					ea = GetEA(cpustate,modrm,1);
				}
				WRITE16(cpustate,ea, cpustate->gdtr.limit);
				WRITE32(cpustate,ea + 2, cpustate->gdtr.base);
				CYCLES(cpustate,CYCLES_SGDT);
				break;
			}
		case 1:         /* SIDT */
			{
				if (modrm >= 0xc0)
				{
					address = LOAD_RM32(modrm);
					ea = i386_translate( cpustate, CS, address, 1 );
				}
				else
				{
					ea = GetEA(cpustate,modrm,1);
				}
				WRITE16(cpustate,ea, cpustate->idtr.limit);
				WRITE32(cpustate,ea + 2, cpustate->idtr.base);
				CYCLES(cpustate,CYCLES_SIDT);
				break;
			}
		case 2:         /* LGDT */
			{
				if(PROTECTED_MODE && cpustate->CPL)
					FAULT(FAULT_GP,0)
				if( modrm >= 0xc0 ) {
					address = LOAD_RM32(modrm);
					ea = i386_translate( cpustate, CS, address, 0 );
				} else {
					ea = GetEA(cpustate,modrm,0);
				}
				cpustate->gdtr.limit = READ16(cpustate,ea);
				cpustate->gdtr.base = READ32(cpustate,ea + 2);
				CYCLES(cpustate,CYCLES_LGDT);
				break;
			}
		case 3:         /* LIDT */
			{
				if(PROTECTED_MODE && cpustate->CPL)
					FAULT(FAULT_GP,0)
				if( modrm >= 0xc0 ) {
					address = LOAD_RM32(modrm);
					ea = i386_translate( cpustate, CS, address, 0 );
				} else {
					ea = GetEA(cpustate,modrm,0);
				}
				cpustate->idtr.limit = READ16(cpustate,ea);
				cpustate->idtr.base = READ32(cpustate,ea + 2);
				CYCLES(cpustate,CYCLES_LIDT);
				break;
			}
		case 4:         /* SMSW */
			{
				if( modrm >= 0xc0 ) {
					STORE_RM32(modrm, cpustate->cr[0] & 0xffff);
					CYCLES(cpustate,CYCLES_SMSW_REG);
				} else {
					/* always 16-bit memory operand */
					ea = GetEA(cpustate,modrm,1);
					WRITE16(cpustate,ea, cpustate->cr[0]);
					CYCLES(cpustate,CYCLES_SMSW_MEM);
				}
				break;
			}
		case 6:         /* LMSW */
			{
				if(PROTECTED_MODE && cpustate->CPL)
					FAULT(FAULT_GP,0)
				UINT16 b;
				if( modrm >= 0xc0 ) {
					b = LOAD_RM16(modrm);
					CYCLES(cpustate,CYCLES_LMSW_REG);
				} else {
					ea = GetEA(cpustate,modrm,0);
					CYCLES(cpustate,CYCLES_LMSW_MEM);
				b = READ16(cpustate,ea);
				}
				if(PROTECTED_MODE)
					b |= 0x0001;  // cannot return to real mode using this instruction.
				cpustate->cr[0] &= ~0x0000000f;
				cpustate->cr[0] |= b & 0x0000000f;
				break;
			}
		case 7:         /* INVLPG */
			{
				// Nothing to do ?
				break;
			}
		default:
			fatalerror("i486: unimplemented opcode 0x0f 01 /%d at %08X\n", (modrm >> 3) & 0x7, cpustate->eip - 2);
			break;
	}
}

static void I486OP(bswap_eax)(i386_state *cpustate)     // Opcode 0x0f 38
{
	REG32(EAX) = SWITCH_ENDIAN_32(REG32(EAX));
	CYCLES(cpustate,1);     // TODO
}

static void I486OP(bswap_ecx)(i386_state *cpustate)     // Opcode 0x0f 39
{
	REG32(ECX) = SWITCH_ENDIAN_32(REG32(ECX));
	CYCLES(cpustate,1);     // TODO
}

static void I486OP(bswap_edx)(i386_state *cpustate)     // Opcode 0x0f 3A
{
	REG32(EDX) = SWITCH_ENDIAN_32(REG32(EDX));
	CYCLES(cpustate,1);     // TODO
}

static void I486OP(bswap_ebx)(i386_state *cpustate)     // Opcode 0x0f 3B
{
	REG32(EBX) = SWITCH_ENDIAN_32(REG32(EBX));
	CYCLES(cpustate,1);     // TODO
}

static void I486OP(bswap_esp)(i386_state *cpustate)     // Opcode 0x0f 3C
{
	REG32(ESP) = SWITCH_ENDIAN_32(REG32(ESP));
	CYCLES(cpustate,1);     // TODO
}

static void I486OP(bswap_ebp)(i386_state *cpustate)     // Opcode 0x0f 3D
{
	REG32(EBP) = SWITCH_ENDIAN_32(REG32(EBP));
	CYCLES(cpustate,1);     // TODO
}

static void I486OP(bswap_esi)(i386_state *cpustate)     // Opcode 0x0f 3E
{
	REG32(ESI) = SWITCH_ENDIAN_32(REG32(ESI));
	CYCLES(cpustate,1);     // TODO
}

static void I486OP(bswap_edi)(i386_state *cpustate)     // Opcode 0x0f 3F
{
	REG32(EDI) = SWITCH_ENDIAN_32(REG32(EDI));
	CYCLES(cpustate,1);     // TODO
}

static void I486OP(mov_cr_r32)(i386_state *cpustate)        // Opcode 0x0f 22
{
	if(PROTECTED_MODE && cpustate->CPL)
		FAULT(FAULT_GP, 0);
	UINT8 modrm = FETCH(cpustate);
	UINT8 cr = (modrm >> 3) & 0x7;
	cpustate->cr[cr] = LOAD_RM32(modrm);
	switch(cr)
	{
		case 0: CYCLES(cpustate,CYCLES_MOV_REG_CR0); break;
		case 2: CYCLES(cpustate,CYCLES_MOV_REG_CR2); break;
		case 3: CYCLES(cpustate,CYCLES_MOV_REG_CR3); break;
		case 4: CYCLES(cpustate,1); break; // TODO
		default:
			fatalerror("i386: mov_cr_r32 CR%d !", cr);
			break;
	}
}
