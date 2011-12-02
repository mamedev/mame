// Intel x87 FPU opcodes

#define ST(x)	(cpustate->fpu_reg[(cpustate->fpu_top + (x)) & 7])
#define FPU_INFINITY_DOUBLE		U64(0x7ff0000000000000)
#define FPU_INFINITY_SINGLE		(0x7f800000)
#define FPU_SIGN_BIT_DOUBLE		U64(0x8000000000000000)
#define FPU_SIGN_BIT_SINGLE		(0x80000000)

// FPU control word flags
#define FPU_MASK_INVALID_OP			0x0001
#define FPU_MASK_DENORMAL_OP		0x0002
#define FPU_MASK_ZERO_DIVIDE		0x0004
#define FPU_MASK_OVERFLOW			0x0008
#define FPU_MASK_UNDERFLOW			0x0010
#define FPU_MASK_PRECISION			0x0020

// FPU status word flags
#define FPU_BUSY					0x8000
#define FPU_C3						0x4000
#define FPU_STACK_TOP_MASK			0x3800
#define FPU_C2						0x0400
#define FPU_C1						0x0200
#define FPU_C0						0x0100
#define FPU_ERROR_SUMMARY			0x0080
#define FPU_STACK_FAULT				0x0040
#define FPU_EXCEPTION_PRECISION		0x0020
#define FPU_EXCEPTION_UNDERFLOW		0x0010
#define FPU_EXCEPTION_OVERFLOW		0x0008
#define FPU_EXCEPTION_ZERO_DIVIDE	0x0004
#define FPU_EXCEPTION_DENORMAL_OP	0x0002
#define FPU_EXCEPTION_INVALID_OP	0x0001


/* return the 32 bit representation of value seen as a single precision floating point number */
INLINE UINT32 FPU_SINGLE_INT32(X87_REG value)
{
	float fs=(float)value.f;
	UINT32 v;

	v=*((UINT32 *)(&fs));
	return v;
}

INLINE void FPU_PUSH(i386_state *cpustate, X87_REG value)
{
	cpustate->fpu_top--;
	if (cpustate->fpu_top < 0)
	{
		cpustate->fpu_top = 7;
	}

	cpustate->fpu_reg[cpustate->fpu_top] = value;
}

INLINE X87_REG FPU_POP(i386_state *cpustate)
{
	X87_REG value = cpustate->fpu_reg[cpustate->fpu_top];

	cpustate->fpu_tag_word |= 3 << (cpustate->fpu_top * 2);		// set FPU register tag to 3 (empty)

	cpustate->fpu_top++;
	if (cpustate->fpu_top > 7)
	{
		cpustate->fpu_top = 0;
	}

	return value;
}

static void I386OP(fpu_group_d8)(i386_state *cpustate)		// Opcode 0xd8
{
	UINT8 modrm = FETCH(cpustate);
	if (modrm < 0xc0)
	{
		UINT32 ea = GetEA(cpustate,modrm);

		switch ((modrm >> 3) & 0x7)
		{
		case 6:  // FDIV
			UINT32 src = READ32(cpustate,ea);
			if(src == 0)
				fatalerror("FPU: Unimplemented Divide-by-zero exception at %08X.\n", cpustate->pc-2);
			ST(0).f = ST(0).f / src;
			CYCLES(cpustate,1);		// TODO
			break;
		}
	}
	else
	{
		fatalerror("I386: FPU Op D8 %02X at %08X", modrm, cpustate->pc-2);
	}
}

static void I386OP(fpu_group_d9)(i386_state *cpustate)		// Opcode 0xd9
{
	UINT8 modrm = FETCH(cpustate);

	if (modrm < 0xc0)
	{
		UINT32 ea = GetEA(cpustate,modrm);

		switch ((modrm >> 3) & 0x7)
		{
			case 3:			// FSTP
			{
				// st(0) -> ea
				WRITE32(cpustate,ea,FPU_SINGLE_INT32(ST(0)));
				FPU_POP(cpustate);
				CYCLES(cpustate,1);		// TODO
				break;
			}

			case 5:			// FLDCW
			{
				cpustate->fpu_control_word = READ16(cpustate,ea);
				CYCLES(cpustate,1);		// TODO
				break;
			}

			case 6:			// FSTENV
			{  // TODO: 32-bit operand size
				WRITE16(cpustate,ea, cpustate->fpu_control_word);
				WRITE16(cpustate,ea+2, cpustate->fpu_status_word);
				WRITE16(cpustate,ea+4, cpustate->fpu_tag_word);
				WRITE16(cpustate,ea+6, cpustate->fpu_inst_ptr & 0xffff);
				WRITE16(cpustate,ea+8, (cpustate->fpu_opcode & 0x07ff) | ((cpustate->fpu_inst_ptr & 0x0f0000) >> 4));
				WRITE16(cpustate,ea+10, cpustate->fpu_data_ptr & 0xffff);
				WRITE16(cpustate,ea+12, ((cpustate->fpu_inst_ptr & 0x0f0000) >> 4));
				CYCLES(cpustate,1);		// TODO
				break;
			}

			case 7:			// FSTCW
			{
				WRITE16(cpustate,ea, cpustate->fpu_control_word);
				CYCLES(cpustate,1);		// TODO
				break;
			}

			default:
				fatalerror("I386: FPU Op D9 %02X at %08X", modrm, cpustate->pc-2);
		}
	}
	else
	{
		switch (modrm & 0x3f)
		{
			// FLD
			case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
			{
				X87_REG t = ST(modrm & 7);
				FPU_PUSH(cpustate,t);
				CYCLES(cpustate,1);		// TODO
				break;
			}

			case 0x20:		// FCHS
			{
				ST(0).i ^= FPU_SIGN_BIT_DOUBLE;
				CYCLES(cpustate,1);		// TODO
				break;
			}

			case 0x28:		// FLD1
			{
				X87_REG t;
				t.f = 1.0;
				FPU_PUSH(cpustate,t);
				CYCLES(cpustate,1);		// TODO
				break;
			}

			case 0x2e:		// FLDZ
			{
				X87_REG t;
				t.f = 0.0;
				FPU_PUSH(cpustate,t);
				CYCLES(cpustate,1);		// TODO
				break;
			}
			default:
				fatalerror("I386: FPU Op D9 %02X at %08X", modrm, cpustate->pc-2);
		}
	}
}

static void I386OP(fpu_group_da)(i386_state *cpustate)		// Opcode 0xda
{
	UINT8 modrm = FETCH(cpustate);
	fatalerror("I386: FPU Op DA %02X at %08X", modrm, cpustate->pc-2);
}

static void I386OP(fpu_group_db)(i386_state *cpustate)		// Opcode 0xdb
{
	UINT8 modrm = FETCH(cpustate);

	if (modrm < 0xc0)
	{
		UINT32 ea = GetEA(cpustate,modrm);

		switch ((modrm >> 3) & 0x7)
		{
			case 0:		// FILD
			{
				X87_REG t;

				t.f=(INT32)READ32(cpustate,ea);
				FPU_PUSH(cpustate,t);
				CYCLES(cpustate,1);		// TODO
				break;
			}

			default:
				fatalerror("I386: FPU Op DB %02X at %08X", modrm, cpustate->pc-2);
		}
	}
	else
	{
		switch (modrm & 0x3f)
		{
			case 0x22:		// FCLEX
			{
				// clears exception flags and busy bit.
				cpustate->fpu_status_word &= ~0x80ff;

				CYCLES(cpustate,1);		// TODO
				break;
			}
			case 0x23:		// FINIT
			{
				cpustate->fpu_control_word = 0x37f;
				cpustate->fpu_status_word = 0;
				cpustate->fpu_tag_word = 0xffff;
				cpustate->fpu_data_ptr = 0;
				cpustate->fpu_inst_ptr = 0;
				cpustate->fpu_opcode = 0;

				CYCLES(cpustate,1);		// TODO
				break;
			}

			case 0x24:		// FSETPM (treated as nop on 387+)
			{
				CYCLES(cpustate,1);
				break;
			}

			default:
				fatalerror("I386: FPU Op DB %02X at %08X", modrm, cpustate->pc-2);
		}
	}
}

static void I386OP(fpu_group_dc)(i386_state *cpustate)		// Opcode 0xdc
{
	UINT8 modrm = FETCH(cpustate);

	if (modrm < 0xc0)
	{
		//UINT32 ea = GetEA(cpustate,modrm);

		switch ((modrm >> 3) & 0x7)
		{
			default:
				fatalerror("I386: FPU Op DC %02X at %08X", modrm, cpustate->pc-2);
		}
	}
	else
	{
		switch (modrm & 0x3f)
		{
			case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
			{
				// FDIVR
				if ((ST(modrm & 7).i & U64(0x7fffffffffffffff)) == 0)
				{
					// set result as infinity if zero divide is masked
					if (cpustate->fpu_control_word & FPU_MASK_ZERO_DIVIDE)
					{
						ST(modrm & 7).i |= FPU_INFINITY_DOUBLE;
					}
				}
				else
				{
					ST(modrm & 7).f = ST(0).f / ST(modrm & 7).f;
				}
				CYCLES(cpustate,1);		// TODO
				break;
			}

			default:
				fatalerror("I386: FPU Op DC %02X at %08X", modrm, cpustate->pc-2);
		}
	}
}

static void I386OP(fpu_group_dd)(i386_state *cpustate)		// Opcode 0xdd
{
	UINT8 modrm = FETCH(cpustate);

	if (modrm < 0xc0)
	{
		UINT32 ea = GetEA(cpustate,modrm);

		switch ((modrm >> 3) & 0x7)
		{
			case 7:			// FSTSW
			{
				WRITE16(cpustate,ea, (cpustate->fpu_status_word & ~FPU_STACK_TOP_MASK) | (cpustate->fpu_top << 10));
				CYCLES(cpustate,1);		// TODO
				break;
			}

			default:
				fatalerror("I386: FPU Op DD %02X at %08X", modrm, cpustate->pc-2);
		}
	}
	else
	{
		switch (modrm & 0x3f)
		{
			default:
				fatalerror("I386: FPU Op DD %02X at %08X", modrm, cpustate->pc-2);
		}
	}
}

static void I386OP(fpu_group_de)(i386_state *cpustate)		// Opcode 0xde
{
	UINT8 modrm = FETCH(cpustate);

	if (modrm < 0xc0)
	{
	//  UINT32 ea = GetEA(cpustate,modrm);

		switch ((modrm >> 3) & 0x7)
		{
			default:
				fatalerror("I386: FPU Op DE %02X at %08X", modrm, cpustate->pc-2);
		}
	}
	else
	{
		switch (modrm & 0x3f)
		{
			case 0x19:			// FCOMPP
			{
				cpustate->fpu_status_word &= ~(FPU_C3 | FPU_C2 | FPU_C0);
				if (ST(0).f > ST(1).f)
				{
					// C3 = 0, C2 = 0, C0 = 0
				}
				else if (ST(0).f < ST(1).f)
				{
					cpustate->fpu_status_word |= FPU_C0;
				}
				else if (ST(0).f == ST(1).f)
				{
					cpustate->fpu_status_word |= FPU_C3;
				}
				else
				{
					// unordered
					cpustate->fpu_status_word |= (FPU_C3 | FPU_C2 | FPU_C0);
				}
				FPU_POP(cpustate);
				FPU_POP(cpustate);
				CYCLES(cpustate,1);		// TODO
				break;
			}

			// FDIVP
			case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
			{
				if ((ST(0).i & U64(0x7fffffffffffffff)) == 0)
				{
					// set result as infinity if zero divide is masked
					if (cpustate->fpu_control_word & FPU_MASK_ZERO_DIVIDE)
					{
						ST(modrm & 7).i |= FPU_INFINITY_DOUBLE;
					}
				}
				else
				{
					ST(modrm & 7).f = ST(modrm & 7).f / ST(0).f;
				}
				FPU_POP(cpustate);
				CYCLES(cpustate,1);		// TODO
				break;
			}

			default:
				fatalerror("I386: FPU Op DE %02X at %08X", modrm, cpustate->pc-2);
		}
	}
}

static void I386OP(fpu_group_df)(i386_state *cpustate)		// Opcode 0xdf
{
	UINT8 modrm = FETCH(cpustate);

	if (modrm < 0xc0)
	{
	//  UINT32 ea = GetEA(cpustate,modrm);

		switch ((modrm >> 3) & 0x7)
		{
			default:
				fatalerror("I386: FPU Op DF %02X at %08X", modrm, cpustate->pc-2);
		}
	}
	else
	{
		switch (modrm & 0x3f)
		{
			case 0x20:			// FSTSW AX
			{
				REG16(AX) = (cpustate->fpu_status_word & ~FPU_STACK_TOP_MASK) | (cpustate->fpu_top << 10);
				CYCLES(cpustate,1);		// TODO
				break;
			}

			default:
				fatalerror("I386: FPU Op DF %02X at %08X", modrm, cpustate->pc-2);
		}
	}
}
