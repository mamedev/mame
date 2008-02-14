// Intel x87 FPU opcodes

#define ST(x)	(I.fpu_reg[(I.fpu_top + (x)) & 7])
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

INLINE void FPU_PUSH(X87_REG value)
{
	I.fpu_top--;
	if (I.fpu_top < 0)
	{
		I.fpu_top = 7;
	}

	I.fpu_reg[I.fpu_top] = value;
}

INLINE X87_REG FPU_POP(void)
{
	X87_REG value = I.fpu_reg[I.fpu_top];

	I.fpu_tag_word |= 3 << (I.fpu_top * 2);		// set FPU register tag to 3 (empty)

	I.fpu_top++;
	if (I.fpu_top > 7)
	{
		I.fpu_top = 0;
	}

	return value;
}

static void I386OP(fpu_group_d8)(void)		// Opcode 0xd8
{
	UINT8 modrm = FETCH();
	fatalerror("I386: FPU Op D8 %02X at %08X", modrm, I.pc-2);
}

static void I386OP(fpu_group_d9)(void)		// Opcode 0xd9
{
	UINT8 modrm = FETCH();

	if (modrm < 0xc0)
	{
		UINT32 ea = GetEA(modrm);

		switch ((modrm >> 3) & 0x7)
		{
			case 5:			// FLDCW
			{
				I.fpu_control_word = READ16(ea);
				CYCLES(1);		// TODO
				break;
			}

			case 7:			// FSTCW
			{
				WRITE16(ea, I.fpu_control_word);
				CYCLES(1);		// TODO
				break;
			}

			default:
				fatalerror("I386: FPU Op D9 %02X at %08X", modrm, I.pc-2);
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
				FPU_PUSH(t);
				CYCLES(1);		// TODO
				break;
			}

			case 0x20:		// FCHS
			{
				ST(0).i ^= FPU_SIGN_BIT_DOUBLE;
				CYCLES(1);		// TODO
				break;
			}

			case 0x28:		// FLD1
			{
				X87_REG t;
				t.f = 1.0;
				FPU_PUSH(t);
				CYCLES(1);		// TODO
				break;
			}

			case 0x2e:		// FLDZ
			{
				X87_REG t;
				t.f = 0.0;
				FPU_PUSH(t);
				CYCLES(1);		// TODO
				break;
			}
			default:
				fatalerror("I386: FPU Op D9 %02X at %08X", modrm, I.pc-2);
		}
	}
}

static void I386OP(fpu_group_da)(void)		// Opcode 0xda
{
	UINT8 modrm = FETCH();
	fatalerror("I386: FPU Op DA %02X at %08X", modrm, I.pc-2);
}

static void I386OP(fpu_group_db)(void)		// Opcode 0xdb
{
	UINT8 modrm = FETCH();

	if (modrm < 0xc0)
	{
		fatalerror("I386: FPU Op DB %02X at %08X", modrm, I.pc-2);
	}
	else
	{
		switch (modrm & 0x3f)
		{
			case 0x23:		// FINIT
			{
				I.fpu_control_word = 0x37f;
				I.fpu_status_word = 0;
				I.fpu_tag_word = 0xffff;
				I.fpu_data_ptr = 0;
				I.fpu_inst_ptr = 0;
				I.fpu_opcode = 0;

				CYCLES(1);		// TODO
				break;
			}

			case 0x24:		// FSETPM (treated as nop on 387+)
			{
				CYCLES(1);
				break;
			}

			default:
				fatalerror("I386: FPU Op DB %02X at %08X", modrm, I.pc-2);
		}
	}
}

static void I386OP(fpu_group_dc)(void)		// Opcode 0xdc
{
	UINT8 modrm = FETCH();

	if (modrm < 0xc0)
	{
		//UINT32 ea = GetEA(modrm);

		switch ((modrm >> 3) & 0x7)
		{
			default:
				fatalerror("I386: FPU Op DC %02X at %08X", modrm, I.pc-2);
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
					if (I.fpu_control_word & FPU_MASK_ZERO_DIVIDE)
					{
						ST(modrm & 7).i |= FPU_INFINITY_DOUBLE;
					}
				}
				else
				{
					ST(modrm & 7).f = ST(0).f / ST(modrm & 7).f;
				}
				CYCLES(1);		// TODO
				break;
			}

			default:
				fatalerror("I386: FPU Op DC %02X at %08X", modrm, I.pc-2);
		}
	}
}

static void I386OP(fpu_group_dd)(void)		// Opcode 0xdd
{
	UINT8 modrm = FETCH();

	if (modrm < 0xc0)
	{
		UINT32 ea = GetEA(modrm);

		switch ((modrm >> 3) & 0x7)
		{
			case 7:			// FSTSW
			{
				WRITE16(ea, (I.fpu_status_word & ~FPU_STACK_TOP_MASK) | (I.fpu_top << 10));
				CYCLES(1);		// TODO
				break;
			}

			default:
				fatalerror("I386: FPU Op DD %02X at %08X", modrm, I.pc-2);
		}
	}
	else
	{
		switch (modrm & 0x3f)
		{
			default:
				fatalerror("I386: FPU Op DD %02X at %08X", modrm, I.pc-2);
		}
	}
}

static void I386OP(fpu_group_de)(void)		// Opcode 0xde
{
	UINT8 modrm = FETCH();

	if (modrm < 0xc0)
	{
	//  UINT32 ea = GetEA(modrm);

		switch ((modrm >> 3) & 0x7)
		{
			default:
				fatalerror("I386: FPU Op DE %02X at %08X", modrm, I.pc-2);
		}
	}
	else
	{
		switch (modrm & 0x3f)
		{
			case 0x19:			// FCOMPP
			{
				I.fpu_status_word &= ~(FPU_C3 | FPU_C2 | FPU_C0);
				if (ST(0).f > ST(1).f)
				{
					// C3 = 0, C2 = 0, C0 = 0
				}
				else if (ST(0).f < ST(1).f)
				{
					I.fpu_status_word |= FPU_C0;
				}
				else if (ST(0).f == ST(1).f)
				{
					I.fpu_status_word |= FPU_C3;
				}
				else
				{
					// unordered
					I.fpu_status_word |= (FPU_C3 | FPU_C2 | FPU_C0);
				}
				FPU_POP();
				FPU_POP();
				CYCLES(1);		// TODO
				break;
			}

			// FDIVP
			case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
			{
				if ((ST(0).i & U64(0x7fffffffffffffff)) == 0)
				{
					// set result as infinity if zero divide is masked
					if (I.fpu_control_word & FPU_MASK_ZERO_DIVIDE)
					{
						ST(modrm & 7).i |= FPU_INFINITY_DOUBLE;
					}
				}
				else
				{
					ST(modrm & 7).f = ST(modrm & 7).f / ST(0).f;
				}
				FPU_POP();
				CYCLES(1);		// TODO
				break;
			}

			default:
				fatalerror("I386: FPU Op DE %02X at %08X", modrm, I.pc-2);
		}
	}
}

static void I386OP(fpu_group_df)(void)		// Opcode 0xdf
{
	UINT8 modrm = FETCH();

	if (modrm < 0xc0)
	{
	//  UINT32 ea = GetEA(modrm);

		switch ((modrm >> 3) & 0x7)
		{
			default:
				fatalerror("I386: FPU Op DF %02X at %08X", modrm, I.pc-2);
		}
	}
	else
	{
		switch (modrm & 0x3f)
		{
			case 0x20:			// FSTSW AX
			{
				REG16(AX) = (I.fpu_status_word & ~FPU_STACK_TOP_MASK) | (I.fpu_top << 10);
				CYCLES(1);		// TODO
				break;
			}

			default:
				fatalerror("I386: FPU Op DF %02X at %08X", modrm, I.pc-2);
		}
	}
}
