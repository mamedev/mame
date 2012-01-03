// Intel x87 FPU opcodes

// TODO:
// - double check single precision opcodes;
// - opcode cycles (every single CPU has different counting on it);
// - clean-ups;

#define ST(x)	(cpustate->fpu_reg[(cpustate->fpu_top + (x)) & 7])
#define FPU_INFINITY_DOUBLE		U64(0x7ff0000000000000)
#define FPU_INFINITY_SINGLE		(0x7f800000)
#define FPU_SIGN_BIT_DOUBLE		U64(0x8000000000000000)
#define FPU_SIGN_BIT_SINGLE		(0x80000000)
#define FPU_MANTISSA_DOUBLE		U64(0x000fffffffffffff)


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

INLINE X87_REG X87_FROUND(i386_state *cpustate, X87_REG t)
{
	switch((cpustate->fpu_control_word >> 10) & 3)
	{
		case 0: t.f = (INT64)t.f + 0.5; break; /* Nearest */
		case 1: t.f = (INT64)floor(t.f); break; /* Down */
		case 2: t.f = (INT64)ceil(t.f); break; /* Up */
		case 3: t.f = (INT64)t.f; break; /* Chop */
	}
	return t;
}

INLINE X87_REG READ80(i386_state *cpustate,UINT32 ea)
{
	X87_REG t;
	UINT16 begin;
	INT64 exp;
	INT64 mantissa;
	UINT8 sign;

	t.i = READ64(cpustate,ea);
	begin = READ16(cpustate,ea+8);

	exp = (begin&0x7fff) - 16383;
	exp = (exp > 0) ? (exp&0x3ff) : (-exp&0x3ff);
	exp += 1023;

	mantissa = (t.i >> 11) & (FPU_MANTISSA_DOUBLE);
	sign = (begin&0x8000) >> 15;

	if(t.i & 0x400)
		mantissa++;

	t.i = ((UINT64)sign << 63)|((UINT64)exp << 52)|((UINT64)mantissa);

	return t;
}

INLINE void WRITE80(i386_state *cpustate,UINT32 ea, X87_REG t)
{
	UINT8 sign = (t.i & FPU_SIGN_BIT_DOUBLE) ? 1 : 0;
	INT64 exp = (t.i & FPU_INFINITY_DOUBLE) >> 52;
	INT64 mantissa = (t.i & FPU_MANTISSA_DOUBLE) << 11;
	INT16 begin;

	if(t.f != 0)
	{
		logerror("Check WRITE80 with t.f != 0\n");
		//mantissa |= FPU_SIGN_BIT_DOUBLE;
		//exp += (16383 - 1023);
	}

	begin = (sign<<15)|(INT16)(exp);
	t.i = mantissa;
	WRITE64(cpustate,ea,t.i);
	WRITE16(cpustate,ea+8,begin);
}

static void I386OP(fpu_group_d8)(i386_state *cpustate)		// Opcode 0xd8
{
	UINT8 modrm = FETCH(cpustate);
	if (modrm < 0xc0)
	{
		UINT32 ea = GetEA(cpustate,modrm);
		UINT32 src = READ32(cpustate,ea);

		switch ((modrm >> 3) & 0x7)
		{
			case 0:	 // FADD short
			{
				ST(0).f += src;
				CYCLES(cpustate,8);
				break;
			}

			case 1:	 // FMUL short
			{
				ST(0).f *= src;
				CYCLES(cpustate,8);
				break;
			}

			case 2:	 // FCOM short
			{
				cpustate->fpu_status_word &= ~(FPU_C3 | FPU_C2 | FPU_C0);
				if(ST(0).f == src)
					cpustate->fpu_status_word |= FPU_C3;
				if(ST(0).f < src)
					cpustate->fpu_status_word |= FPU_C0;
				CYCLES(cpustate,4);
				break;
			}

			case 3:	 // FCOMP short
			{
				cpustate->fpu_status_word &= ~(FPU_C3 | FPU_C2 | FPU_C0);
				if(ST(0).f == src)
					cpustate->fpu_status_word |= FPU_C3;
				if(ST(0).f < src)
					cpustate->fpu_status_word |= FPU_C0;
				FPU_POP(cpustate);
				CYCLES(cpustate,4);
				break;
			}

			case 4:	 // FSUB short
			{
				ST(0).f -= src;
				CYCLES(cpustate,8);
				break;
			}

			case 5:	 // FSUBR short
			{
				ST(0).f = src - ST(0).f;
				CYCLES(cpustate,8);
				break;
			}

			case 6:  // FDIV
			{
				if(src == 0)
					fatalerror("FPU: Unimplemented Divide-by-zero exception at %08X.\n", cpustate->pc-2);
				else
					ST(0).f = ST(0).f / src;
				CYCLES(cpustate,73);
				break;
			}

			case 7:  // FDIVR
			{
				if(src == 0)
					fatalerror("FPU: Unimplemented Divide-by-zero exception at %08X.\n", cpustate->pc-2);
				else
					ST(0).f = src / ST(0).f;
				CYCLES(cpustate,73);
				break;
			}
		}
	}
	else
	{
		switch (modrm & 0x3f)
		{
			case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07: // FADD
			{
				ST(0).f+=ST(modrm & 7).f;
				CYCLES(cpustate,8);
				break;
			}
			case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f: // FMUL
			{
				ST(0).f*=ST(modrm & 7).f;
				CYCLES(cpustate,16);
				break;
			}
			case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17: // FCOM
			{
				cpustate->fpu_status_word &= ~(FPU_C3 | FPU_C2 | FPU_C0);
				if(ST(0).f == ST(modrm & 7).f)
					cpustate->fpu_status_word |= FPU_C3;
				if(ST(0).f < ST(modrm & 7).f)
					cpustate->fpu_status_word |= FPU_C0;
				CYCLES(cpustate,4);
				break;
			}

			case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f: // FCOMP
			{
				cpustate->fpu_status_word &= ~(FPU_C3 | FPU_C2 | FPU_C0);
				if(ST(0).f == ST(modrm & 7).f)
					cpustate->fpu_status_word |= FPU_C3;
				if(ST(0).f < ST(modrm & 7).f)
					cpustate->fpu_status_word |= FPU_C0;
				FPU_POP(cpustate);
				CYCLES(cpustate,4);
				break;
			}

			case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27: // FSUB
			{
				ST(0).f-=ST(modrm & 7).f;
				CYCLES(cpustate,8);
				break;
			}

			case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f: // FSUBR
			{
				ST(0).f = ST(modrm & 7).f - ST(0).f;
				CYCLES(cpustate,8);
				break;
			}

			case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37: // FDIV
			{
				if(ST(modrm & 7).f != 0)
					ST(0).f/=ST(modrm & 7).f;
				else
					fatalerror("Divide by zero on FDIV 0xd8 0x30");
				CYCLES(cpustate,73);
				break;
			}

			case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f: // FDIVR
			{
				if(ST(0).f != 0)
					ST(0).f = ST(modrm & 7).f / ST(0).f;
				else
					fatalerror("Divide by zero on FDIVR 0xd8 0x38");
				CYCLES(cpustate,73);
				break;
			}

			default:
				fatalerror("I386: FPU Op D8 %02X at %08X", modrm, cpustate->pc-2);
		}

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
			case 0:			// FLD single-precision
			{
				X87_REG t;
				t.i = READ32(cpustate,ea);
				FPU_PUSH(cpustate,t);
				CYCLES(cpustate,3);
				break;
			}

			case 2:			// FST single-precision
			{
				WRITE32(cpustate,ea,FPU_SINGLE_INT32(ST(0)));
				CYCLES(cpustate,7);
				break;
			}

			case 3:			// FSTP
			{
				// st(0) -> ea
				WRITE32(cpustate,ea,FPU_SINGLE_INT32(ST(0)));
				FPU_POP(cpustate);
				CYCLES(cpustate,7);
				break;
			}

			case 4:			// FLDENV
			{
				if( cpustate->operand_size )  // 32-bit real/protected mode
				{
					cpustate->fpu_control_word = READ16(cpustate,ea);
					cpustate->fpu_status_word = READ16(cpustate,ea+4);
					cpustate->fpu_tag_word = READ16(cpustate,ea+8);
				}
				else // 16-bit real/protected mode
				{
					cpustate->fpu_control_word = READ16(cpustate,ea);
					cpustate->fpu_status_word = READ16(cpustate,ea+2);
					cpustate->fpu_tag_word = READ16(cpustate,ea+4);
				}

				cpustate->fpu_top = (cpustate->fpu_status_word>>11) & 7;

				CYCLES(cpustate,(cpustate->cr[0] & 1) ? 34 : 44);
				break;
			}

			case 5:			// FLDCW
			{
				cpustate->fpu_control_word = READ16(cpustate,ea);
				CYCLES(cpustate,4);
				break;
			}

			case 6:			// FSTENV
			{
				logerror("x87 FSTENV triggered with mode = %d %02x %02x\n",(cpustate->cr[0] & 1)|(cpustate->operand_size & 1)<<1,cpustate->cr[0],cpustate->operand_size);
				switch((cpustate->cr[0] & 1)|(cpustate->operand_size & 1)<<1)
				{
					case 0:	// 16-bit real mode
						WRITE16(cpustate,ea, cpustate->fpu_control_word);
						WRITE16(cpustate,ea+2, cpustate->fpu_status_word);
						WRITE16(cpustate,ea+4, cpustate->fpu_tag_word);
						WRITE16(cpustate,ea+6, cpustate->fpu_inst_ptr & 0xffff);
						//WRITE16(cpustate,ea+8, (cpustate->fpu_opcode & 0x07ff) | ((cpustate->fpu_inst_ptr & 0x0f0000) >> 4));
						WRITE16(cpustate,ea+10, cpustate->fpu_data_ptr & 0xffff);
						//WRITE16(cpustate,ea+12, ((cpustate->fpu_inst_ptr & 0x0f0000) >> 4));
						break;
					case 1: // 16-bit protected mode
						WRITE16(cpustate,ea, cpustate->fpu_control_word);
						WRITE16(cpustate,ea+2, cpustate->fpu_status_word);
						WRITE16(cpustate,ea+4, cpustate->fpu_tag_word);
						WRITE16(cpustate,ea+6, cpustate->fpu_inst_ptr & 0xffff);
						WRITE16(cpustate,ea+8, (cpustate->fpu_opcode & 0x07ff) | ((cpustate->fpu_inst_ptr & 0x0f0000) >> 4));
						WRITE16(cpustate,ea+10, cpustate->fpu_data_ptr & 0xffff);
						WRITE16(cpustate,ea+12, ((cpustate->fpu_inst_ptr & 0x0f0000) >> 4));
						break;
					case 2: // 32-bit real mode
						WRITE16(cpustate,ea, cpustate->fpu_control_word);
						WRITE16(cpustate,ea+4, cpustate->fpu_status_word);
						WRITE16(cpustate,ea+8, cpustate->fpu_tag_word);
						WRITE16(cpustate,ea+12, cpustate->fpu_inst_ptr & 0xffff);
						//WRITE16(cpustate,ea+8, (cpustate->fpu_opcode & 0x07ff) | ((cpustate->fpu_inst_ptr & 0x0f0000) >> 4));
						WRITE16(cpustate,ea+20, cpustate->fpu_data_ptr & 0xffff);
						//WRITE16(cpustate,ea+12, ((cpustate->fpu_inst_ptr & 0x0f0000) >> 4));
						WRITE32(cpustate,ea+24, (cpustate->fpu_data_ptr>>16)<<12);
						break;
					case 3: // 32-bit protected mode
						WRITE16(cpustate,ea, cpustate->fpu_control_word);
						WRITE16(cpustate,ea+4, cpustate->fpu_status_word);
						WRITE16(cpustate,ea+8, cpustate->fpu_tag_word);
						WRITE32(cpustate,ea+12, cpustate->fpu_inst_ptr);
						WRITE32(cpustate,ea+16, cpustate->fpu_opcode);
						WRITE32(cpustate,ea+20, cpustate->fpu_data_ptr);
						WRITE32(cpustate,ea+24, cpustate->fpu_inst_ptr);
						break;
				}

				CYCLES(cpustate,(cpustate->cr[0] & 1) ? 56 : 67);
				break;
			}

			case 7:			// FSTCW
			{
				WRITE16(cpustate,ea, cpustate->fpu_control_word);
				CYCLES(cpustate,3);
				break;
			}

			default:
				fatalerror("I386: FPU Op D9 %02X at %08X", (modrm >> 3) & 0x7, cpustate->pc-2);
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
				CYCLES(cpustate,4);
				break;
			}

			// FXCH
			case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
			{
				X87_REG t = ST(0);
				ST(0) = ST(modrm & 7);
				ST(modrm & 7) = t;
				CYCLES(cpustate,4);
				break;
			}

			case 0x10:		// FNOP
			{
				CYCLES(cpustate,3);
				break;
			}

			case 0x20:		// FCHS
			{
				ST(0).i ^= FPU_SIGN_BIT_DOUBLE;
				CYCLES(cpustate,6);
				break;
			}

			case 0x21:		// FABS
			{
				ST(0).f = fabs(ST(0).f);
				CYCLES(cpustate,3);
				break;
			}


			case 0x24:		// FTST
			{
				cpustate->fpu_status_word &= ~(FPU_C3 | FPU_C2 | FPU_C0);
				if(ST(0).f == 0.0)
					cpustate->fpu_status_word |= FPU_C3;
				if(ST(0).f < 0.0)
					cpustate->fpu_status_word |= FPU_C0;
				CYCLES(cpustate,4);
				break;
			}

			case 0x25:		// FXAM
			{
				cpustate->fpu_status_word &= ~(FPU_C3 | FPU_C2 | FPU_C0);
				if(((cpustate->fpu_tag_word>>((cpustate->fpu_top&1)<<1))&3)==3)
					cpustate->fpu_status_word |= FPU_C3 | FPU_C0;
				else if(ST(0).f == 0.0)
					cpustate->fpu_status_word |= FPU_C3;
				else
					cpustate->fpu_status_word |= FPU_C2;
				if(ST(0).f < 0.0)
					cpustate->fpu_status_word |= FPU_C1;
				CYCLES(cpustate,8);
				break;
			}

			case 0x28:		// FLD1
			{
				X87_REG t;
				t.f = 1.0;
				FPU_PUSH(cpustate,t);
				CYCLES(cpustate,4);
				break;
			}

			case 0x29:		// FLDL2T
			{
				X87_REG t;
				t.f = 3.3219280948873623;
				FPU_PUSH(cpustate,t);
				CYCLES(cpustate,8);
			}

			case 0x2a:		// FLDL2E
			{
				X87_REG t;
				t.f = 1.4426950408889634;
				FPU_PUSH(cpustate,t);
				CYCLES(cpustate,8);
			}

			case 0x2b:		// FLDPI
			{
				X87_REG t;
				t.f = 3.141592653589793;
				FPU_PUSH(cpustate,t);
				CYCLES(cpustate,8);
			}

			case 0x2c:		// FLDEG2
			{
				X87_REG t;
				t.f = 0.3010299956639812;
				FPU_PUSH(cpustate,t);
				CYCLES(cpustate,8);
			}

			case 0x2d:		// FLDLN2
			{
				X87_REG t;
				t.f = 0.693147180559945;
				FPU_PUSH(cpustate,t);
				CYCLES(cpustate,8);
			}

			case 0x2e:		// FLDZ
			{
				X87_REG t;
				t.f = 0.0;
				FPU_PUSH(cpustate,t);
				/* TODO: tag moves? */
				CYCLES(cpustate,4);
				break;
			}

			case 0x30:		// F2XM1
			{
				ST(0).f = pow(2.0,ST(0).f)-1.0;
				CYCLES(cpustate,200);
				break;
			}

			case 0x31:		// FYL2X
			{
				ST(1).f *= (log(ST(0).f)/log(2.0));
				FPU_POP(cpustate);
				CYCLES(cpustate,250);
				break;
			}

			case 0x32:		// FPTAN
			{
				X87_REG t;
				t.f = 1.0;
				ST(0).f = tan(ST(0).f);
				FPU_PUSH(cpustate,t);
				cpustate->fpu_status_word &= ~(FPU_C2);
				/* TODO: source operand out-of-range */
				CYCLES(cpustate,235);
				break;
			}

			case 0x33:		// FPATAN
			{
				ST(1).f = atan2(ST(1).f,ST(0).f);
				FPU_POP(cpustate);
				CYCLES(cpustate,250);
				break;
			}

			case 0x36:		// FPDECSTP
			{
				cpustate->fpu_top -= 1;
				cpustate->fpu_top &= 7;
				CYCLES(cpustate,1); // TODO
				break;
			}

			case 0x37:		// FPINCSTP
			{
				cpustate->fpu_top += 1;
				cpustate->fpu_top &= 7;
				CYCLES(cpustate,1); // TODO
				break;
			}

			case 0x38:		// FPREM
			{
				X87_REG t;

				if(ST(1).i == 0)
					fatalerror("Divide by zero on x87 FPREM opcode");

				t.i = ST(0).i / ST(1).i;
				ST(0).f = ST(0).f-(ST(1).i*t.f);
				cpustate->fpu_status_word &= ~(FPU_C0 | FPU_C1 | FPU_C2 | FPU_C3);

				if(t.i & 4)
					cpustate->fpu_status_word |= FPU_C0;

				if(t.i & 2)
					cpustate->fpu_status_word |= FPU_C3;

				if(t.i & 1)
					cpustate->fpu_status_word |= FPU_C1;

				CYCLES(cpustate,100);
				break;
			}

			case 0x3a:		// FSQRT
			{
				ST(0).f = sqrt(ST(0).f);
				CYCLES(cpustate,83);
				break;
			}

			case 0x3b:		// FSINCOS
			{
				X87_REG t;
				t.f = cos(ST(0).f);
				ST(0).f = sin(ST(0).f);
				FPU_PUSH(cpustate,t);
				cpustate->fpu_status_word &= ~(FPU_C2);
				/* TODO: source operand out-of-range */
				CYCLES(cpustate,330);
				break;
			}

			case 0x3c:		// FRNDINT
			{
				ST(0) = X87_FROUND(cpustate,ST(0));
				CYCLES(cpustate,21);
				break;
			}

			case 0x3d:		// FSCALE
			{
				X87_REG t;
				t = ST(1);
				ST(0).f = ST(0).f*pow(2.0,t.f);
				CYCLES(cpustate,30);
			}

			case 0x3e:		// FSIN
			{
				ST(0).f = sin(ST(0).f);
				cpustate->fpu_status_word &= ~(FPU_C2);
				/* TODO: source operand out-of-range */
				CYCLES(cpustate,300);
				break;
			}

			case 0x3f:		// FCOS
			{
				ST(0).f = cos(ST(0).f);
				cpustate->fpu_status_word &= ~(FPU_C2);
				/* TODO: source operand out-of-range */
				CYCLES(cpustate,300);
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

	if (modrm < 0xc0)
	{
		UINT32 ea = GetEA(cpustate,modrm);
		X87_REG t;
		t.i = READ32(cpustate,ea);

		switch ((modrm >> 3) & 0x7)
		{
			case 0:	// FIADD
				ST(0).i+=t.i;
				CYCLES(cpustate,20);
				break;
			case 1:	// FIMUL
				ST(0).i*=t.i;
				CYCLES(cpustate,22);
				break;
			case 2:	// FICOM
				cpustate->fpu_status_word &= ~(FPU_C3 | FPU_C2 | FPU_C0);
				if(ST(0).i == t.i)
					cpustate->fpu_status_word |= FPU_C3;
				if(ST(0).i < t.i)
					cpustate->fpu_status_word |= FPU_C0;

				CYCLES(cpustate,15);
				break;
			case 3:	// FICOMP
				cpustate->fpu_status_word &= ~(FPU_C3 | FPU_C2 | FPU_C0);
				if(ST(0).i == t.i)
					cpustate->fpu_status_word |= FPU_C3;
				if(ST(0).i < t.i)
					cpustate->fpu_status_word |= FPU_C0;

				FPU_POP(cpustate);
				CYCLES(cpustate,15);
				break;

			case 4:	// FISUB
				ST(0).i-=t.i;
				CYCLES(cpustate,15);
				break;

			case 5:	// FISUBR
				ST(0).i = t.i - ST(0).i;
				CYCLES(cpustate,15);
				break;

			case 6:	// FIDIV
				if(t.i)
					ST(0).i/=t.i;
				else
					fatalerror("Divide-by-zero on FIDIV 0xda 0x06 opcode");
				CYCLES(cpustate,84);
				break;

			case 7:	// FIDIVR
				if(ST(0).i)
					ST(0).i = t.i / ST(0).i;
				else
					fatalerror("Divide-by-zero on FIDIV 0xda 0x07 opcode");
				CYCLES(cpustate,84);
				break;

			default:
				fatalerror("I386: FPU Op DA %02X at %08X", modrm, cpustate->pc-2);
		}
	}
	else
	{
		switch(modrm & 0x3f)
		{
			case 0x29: // FUCOMPP
				cpustate->fpu_status_word &= ~(FPU_C3 | FPU_C2 | FPU_C0);
				if(ST(0).f == ST(1).f)
					cpustate->fpu_status_word |= FPU_C3;
				if(ST(0).f < ST(1).f)
					cpustate->fpu_status_word |= FPU_C0;

				FPU_POP(cpustate);
				FPU_POP(cpustate);
				CYCLES(cpustate,5);
				break;
			default:
				fatalerror("I386: FPU Op DA %02X at %08X", modrm, cpustate->pc-2);
		}
	}
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
				CYCLES(cpustate,9);
				break;
			}

			case 2:		// FIST
			{
				X87_REG t;

				t = X87_FROUND(cpustate,ST(0));
				WRITE32(cpustate,ea,(INT32)t.i);
				CYCLES(cpustate,28);
				break;
			}

			case 3:		// FISTP
			{
				X87_REG t;

				t = X87_FROUND(cpustate,ST(0));
				WRITE32(cpustate,ea,(INT32)t.i);
				FPU_POP(cpustate);
				CYCLES(cpustate,28);
				break;
			}

			case 5:		// FLD extended
			{
				X87_REG t;

				t = READ80(cpustate,ea);
				FPU_PUSH(cpustate,t);
				CYCLES(cpustate,6);
				break;
			}

			case 7:		// FSTP extended
			{
				WRITE80(cpustate,ea,ST(0));
				FPU_POP(cpustate);
				CYCLES(cpustate,6);
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

				CYCLES(cpustate,17);
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
		UINT32 ea = GetEA(cpustate,modrm);
		X87_REG t;

		t.i = READ64(cpustate,ea);

		switch ((modrm >> 3) & 0x7)
		{
			case 0: /* FADD double */
			{
				ST(0).f += t.f;
				CYCLES(cpustate,8);
				break;
			}

			case 1: /* FMUL double */
			{
				ST(0).f *= t.f;
				CYCLES(cpustate,14);
				break;
			}

			case 2: /* FCOM double */
			{
				cpustate->fpu_status_word &= ~(FPU_C3 | FPU_C2 | FPU_C0);
				if(ST(0).f == t.f)
					cpustate->fpu_status_word |= FPU_C3;
				if(ST(0).f < t.f)
					cpustate->fpu_status_word |= FPU_C0;

				CYCLES(cpustate,4);
				break;
			}

			case 3: /* FCOMP double */
			{
				cpustate->fpu_status_word &= ~(FPU_C3 | FPU_C2 | FPU_C0);
				if(ST(0).f == t.f)
					cpustate->fpu_status_word |= FPU_C3;
				if(ST(0).f < t.f)
					cpustate->fpu_status_word |= FPU_C0;

				FPU_POP(cpustate);
				CYCLES(cpustate,4);
				break;
			}

			case 4: /* FSUB double */
			{
				ST(0).f -= t.f;
				CYCLES(cpustate,8);
				break;
			}

			case 5: /* FSUBR double */
			{
				ST(0).f = t.f - ST(0).f;
				CYCLES(cpustate,8);
				break;
			}

			case 6: /* FDIV double */
			{
				if(t.f)
					ST(0).f /= t.f;
				else
					fatalerror("FPU Op DC 6 Divide by zero unhandled exception");

				CYCLES(cpustate,73);
				break;
			}

			case 7: /* FDIV double */
			{
				if(ST(0).f)
					ST(0).f = t.f / ST(0).f;
				else
					fatalerror("FPU Op DC 6 Divide by zero unhandled exception");

				CYCLES(cpustate,73);
				break;
			}

			default:
				fatalerror("I386: FPU Op DC %02X at %08X", (modrm >> 3) & 0x7, cpustate->pc-2);
		}
	}
	else
	{
		switch (modrm & 0x3f)
		{
			case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07: // FADD
			{
				ST(modrm & 7).f += ST(0).f;
				CYCLES(cpustate,8);
				break;
			}

			case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f: // FMUL
			{
				ST(modrm & 7).f *= ST(0).f;
				CYCLES(cpustate,8);
				break;
			}

			case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17: // FSUBR
			{
				ST(modrm & 7).f = ST(0).f - ST(modrm & 7).f;
				CYCLES(cpustate,8);
				break;
			}

			case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f: // FSUB
			{
				ST(modrm & 7).f -= ST(0).f;
				CYCLES(cpustate,8);
				break;
			}

			case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27: // FSUBR
			{
				ST(modrm & 7).f = ST(0).f - ST(modrm & 7).f;
				CYCLES(cpustate,8);
				break;
			}

			case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f: // FSUB
			{
				ST(modrm & 7).f -= ST(0).f;
				CYCLES(cpustate,8);
				break;
			}

			// FDIVR
			case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
			{
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
				CYCLES(cpustate,73);
				break;
			}

			 // FDIV
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
					ST(modrm & 7).f /= ST(0).f;
				}
				CYCLES(cpustate,73);
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
			case 0: // FLD
			{
				X87_REG t;
				t.i = READ64(cpustate,ea);
				FPU_PUSH(cpustate,t);
				CYCLES(cpustate,3);
				break;
			}

			case 2: // FST
			{
				X87_REG t;
				t.f = ST(0).f;
				WRITE64(cpustate,ea, t.i);
				//FPU_POP(cpustate);
				CYCLES(cpustate,8);
				break;
			}


			case 3: // FSTP
			{
				X87_REG t;
				t.f = ST(0).f;
				WRITE64(cpustate,ea, t.i);
				FPU_POP(cpustate);
				CYCLES(cpustate,8);
				break;
			}

			case 4:	// FRSTOR
			{
				int i;
				if( cpustate->operand_size )  // 32-bit real/protected mode
				{
					cpustate->fpu_control_word = READ16(cpustate,ea);
					cpustate->fpu_status_word = READ16(cpustate,ea+4);
					cpustate->fpu_tag_word = READ16(cpustate,ea+8);
					ea+=28;
				}
				else // 16-bit real/protected mode
				{
					cpustate->fpu_control_word = READ16(cpustate,ea);
					cpustate->fpu_status_word = READ16(cpustate,ea+2);
					cpustate->fpu_tag_word = READ16(cpustate,ea+4);
					ea+=14;
				}

				cpustate->fpu_top = (cpustate->fpu_status_word>>11) & 7;

				for(i=0;i<8;i++)
					ST(i) = READ80(cpustate,ea+i*10);

				CYCLES(cpustate,(cpustate->cr[0] & 1) ? 34 : 44);
				break;
			}

			case 6: // FSAVE
			{
				int i;
				logerror("x87 FSAVE triggered with mode = %d %02x %02x\n",(cpustate->cr[0] & 1)|(cpustate->operand_size & 1)<<1,cpustate->cr[0],cpustate->operand_size);
				switch((cpustate->cr[0] & 1)|(cpustate->operand_size & 1)<<1)
				{
					case 0:	// 16-bit real mode
						WRITE16(cpustate,ea, cpustate->fpu_control_word);
						WRITE16(cpustate,ea+2, cpustate->fpu_status_word);
						WRITE16(cpustate,ea+4, cpustate->fpu_tag_word);
						WRITE16(cpustate,ea+6, cpustate->fpu_inst_ptr & 0xffff);
						//WRITE16(cpustate,ea+8, (cpustate->fpu_opcode & 0x07ff) | ((cpustate->fpu_inst_ptr & 0x0f0000) >> 4));
						WRITE16(cpustate,ea+10, cpustate->fpu_data_ptr & 0xffff);
						//WRITE16(cpustate,ea+12, ((cpustate->fpu_inst_ptr & 0x0f0000) >> 4));
						ea+=14;
						break;
					case 1: // 16-bit protected mode
						WRITE16(cpustate,ea, cpustate->fpu_control_word);
						WRITE16(cpustate,ea+2, cpustate->fpu_status_word);
						WRITE16(cpustate,ea+4, cpustate->fpu_tag_word);
						WRITE16(cpustate,ea+6, cpustate->fpu_inst_ptr & 0xffff);
						WRITE16(cpustate,ea+8, (cpustate->fpu_opcode & 0x07ff) | ((cpustate->fpu_inst_ptr & 0x0f0000) >> 4));
						WRITE16(cpustate,ea+10, cpustate->fpu_data_ptr & 0xffff);
						WRITE16(cpustate,ea+12, ((cpustate->fpu_inst_ptr & 0x0f0000) >> 4));
						ea+=14;
						break;
					case 2: // 32-bit real mode
						WRITE16(cpustate,ea, cpustate->fpu_control_word);
						WRITE16(cpustate,ea+4, cpustate->fpu_status_word);
						WRITE16(cpustate,ea+8, cpustate->fpu_tag_word);
						WRITE16(cpustate,ea+12, cpustate->fpu_inst_ptr & 0xffff);
						//WRITE16(cpustate,ea+8, (cpustate->fpu_opcode & 0x07ff) | ((cpustate->fpu_inst_ptr & 0x0f0000) >> 4));
						WRITE16(cpustate,ea+20, cpustate->fpu_data_ptr & 0xffff);
						//WRITE16(cpustate,ea+12, ((cpustate->fpu_inst_ptr & 0x0f0000) >> 4));
						WRITE32(cpustate,ea+24, (cpustate->fpu_data_ptr>>16)<<12);
						ea+=28;
						break;
					case 3: // 32-bit protected mode
						WRITE16(cpustate,ea, cpustate->fpu_control_word);
						WRITE16(cpustate,ea+4, cpustate->fpu_status_word);
						WRITE16(cpustate,ea+8, cpustate->fpu_tag_word);
						WRITE32(cpustate,ea+12, cpustate->fpu_inst_ptr);
						WRITE32(cpustate,ea+16, cpustate->fpu_opcode);
						WRITE32(cpustate,ea+20, cpustate->fpu_data_ptr);
						WRITE32(cpustate,ea+24, cpustate->fpu_inst_ptr);
						ea+=28;
						break;
				}

				for(i=0;i<8;i++)
					ST(i) = READ80(cpustate,ea+i*10);

				CYCLES(cpustate,(cpustate->cr[0] & 1) ? 56 : 67);
				break;
			}

			case 7:			// FSTSW
			{
				WRITE16(cpustate,ea, (cpustate->fpu_status_word & ~FPU_STACK_TOP_MASK) | (cpustate->fpu_top << 10));
				CYCLES(cpustate,1);		// TODO
				break;
			}

			default:
				fatalerror("I386: FPU Op DD %02X (%02X) at %08X", modrm, (modrm >> 3) & 7, cpustate->pc-2);
		}
	}
	else
	{
		switch (modrm & 0x3f)
		{
			case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07: // FFREE
			{
				cpustate->fpu_tag_word |= (3<<((modrm & 7)<< 1));
				CYCLES(cpustate,3);
				break;
			}

			case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17: // FST
			{
				UINT16 tmp;
				ST(modrm & 7) = ST(0);
				tmp = (cpustate->fpu_tag_word>>((cpustate->fpu_top&7)<<1))&3;
				cpustate->fpu_tag_word &= ~(3<<((modrm & 7)<< 1));
				cpustate->fpu_tag_word |= (tmp<<((modrm & 7)<< 1));
				CYCLES(cpustate,3);
				break;
			}

			case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f: // FSTP
			{
				UINT16 tmp;
				ST(modrm & 7) = ST(0);
				tmp = (cpustate->fpu_tag_word>>((cpustate->fpu_top&7)<<1))&3;
				cpustate->fpu_tag_word &= ~(3<<((modrm & 7)<< 1));
				cpustate->fpu_tag_word |= (tmp<<((modrm & 7)<< 1));
				FPU_POP(cpustate);
				CYCLES(cpustate,3);
				break;
			}

			case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27: // FUCOM
			{
				cpustate->fpu_status_word &= ~(FPU_C3 | FPU_C2 | FPU_C0);
				if (ST(0).f > ST(modrm & 7).f)
				{
					// C3 = 0, C2 = 0, C0 = 0
				}
				else if (ST(0).f < ST(modrm & 7).f)
				{
					cpustate->fpu_status_word |= FPU_C0;
				}
				else if (ST(0).f == ST(modrm & 7).f)
				{
					cpustate->fpu_status_word |= FPU_C3;
				}
				else
				{
					// unordered
					cpustate->fpu_status_word |= (FPU_C3 | FPU_C2 | FPU_C0);
				}

				//FPU_POP(cpustate);
				CYCLES(cpustate,4);
				break;
			}

			case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f: // FUCOMP
			{
				cpustate->fpu_status_word &= ~(FPU_C3 | FPU_C2 | FPU_C0);
				if (ST(0).f > ST(modrm & 7).f)
				{
					// C3 = 0, C2 = 0, C0 = 0
				}
				else if (ST(0).f < ST(modrm & 7).f)
				{
					cpustate->fpu_status_word |= FPU_C0;
				}
				else if (ST(0).f == ST(modrm & 7).f)
				{
					cpustate->fpu_status_word |= FPU_C3;
				}
				else
				{
					// unordered
					cpustate->fpu_status_word |= (FPU_C3 | FPU_C2 | FPU_C0);
				}

				FPU_POP(cpustate);
				CYCLES(cpustate,4);
				break;
			}

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
		UINT32 ea = GetEA(cpustate,modrm);
		INT32 t;

		t = (INT32)READ16(cpustate,ea);

		switch ((modrm >> 3) & 0x7)
		{
			case 0:	// FIADD single precision
			{
				ST(0).f+=(double)t;
				CYCLES(cpustate,20);
				break;
			}

			case 1:	// FIMUL single precision
			{
				ST(0).f*=(double)t;
				CYCLES(cpustate,22);
				break;
			}

			case 2: // FICOM
			{
				cpustate->fpu_status_word &= ~(FPU_C3 | FPU_C2 | FPU_C0);
				if (ST(0).f > (double)t)
				{
					// C3 = 0, C2 = 0, C0 = 0
				}
				else if (ST(0).f < (double)t)
				{
					cpustate->fpu_status_word |= FPU_C0;
				}
				else if (ST(0).f == (double)t)
				{
					cpustate->fpu_status_word |= FPU_C3;
				}
				else
				{
					// unordered
					cpustate->fpu_status_word |= (FPU_C3 | FPU_C2 | FPU_C0);
				}
				CYCLES(cpustate,15);
			}

			case 3: // FICOMP
			{
				cpustate->fpu_status_word &= ~(FPU_C3 | FPU_C2 | FPU_C0);
				if (ST(0).f > (double)t)
				{
					// C3 = 0, C2 = 0, C0 = 0
				}
				else if (ST(0).f < (double)t)
				{
					cpustate->fpu_status_word |= FPU_C0;
				}
				else if (ST(0).f == (double)t)
				{
					cpustate->fpu_status_word |= FPU_C3;
				}
				else
				{
					// unordered
					cpustate->fpu_status_word |= (FPU_C3 | FPU_C2 | FPU_C0);
				}
				FPU_POP(cpustate);
				CYCLES(cpustate,15);
			}

			case 4:	// FISUB single precision
			{
				ST(0).f-=(double)t;
				CYCLES(cpustate,20);
				break;
			}

			case 5:	// FISUBR single precision
			{
				ST(0).f = (double)t - ST(0).f;
				CYCLES(cpustate,20);
				break;
			}

			case 6:	// FIDIV single precision
			{
				if((double)t == 0)
					fatalerror("Divide by zero on x87 opcode 0xde 0x06");
				else
					ST(0).f/=(double)t;
				CYCLES(cpustate,84);
				break;
			}

			case 7:	// FIDIVR single precision
			{
				if(ST(0).f == 0)
					fatalerror("Divide by zero on x87 opcode 0xde 0x07");
				else
					ST(0).f = (double)t / ST(0).f;
				CYCLES(cpustate,84);
				break;
			}


			default:
				fatalerror("I386: FPU Op DE %02X at %08X", modrm, cpustate->pc-2);
		}
	}
	else
	{
		switch (modrm & 0x3f)
		{
			case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07: // FADDP
			{
				ST(modrm & 7).f += ST(cpustate->fpu_top).f;
				FPU_POP(cpustate);
				CYCLES(cpustate,8);
				break;
			}

			case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f: // FMULP
			{
				ST(modrm & 7).f *= ST(cpustate->fpu_top).f;
				FPU_POP(cpustate);
				CYCLES(cpustate,16);
				break;
			}

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
				CYCLES(cpustate,5);
				break;
			}

			case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27: // FSUBRP
			{
				ST(modrm & 7).f = ST(0).f - ST(modrm & 7).f;
				FPU_POP(cpustate);
				CYCLES(cpustate,8);
				break;
			}

			case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f: // FSUBP
			{
				ST(modrm & 7).f -= ST(cpustate->fpu_top).f;
				FPU_POP(cpustate);
				CYCLES(cpustate,8);
				break;
			}

			// FDIVRP
			case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
			{
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
				FPU_POP(cpustate);
				CYCLES(cpustate,73);
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
				CYCLES(cpustate,73);
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
		UINT32 ea = GetEA(cpustate,modrm);

		switch ((modrm >> 3) & 0x7)
		{
			case 0:		// FILD short
			{
				X87_REG t;

				t.f=(INT16)READ16(cpustate,ea);
				FPU_PUSH(cpustate,t);
				CYCLES(cpustate,13);
				break;
			}

			case 3:		// FISTP short
			{
				X87_REG t;

				t = X87_FROUND(cpustate,ST(0));
				WRITE16(cpustate,ea,(INT16)t.i);
				FPU_POP(cpustate);
				CYCLES(cpustate,29);
				break;
			}

			case 5:		// FILD long
			{
				X87_REG t;

				t.f=(INT64)READ64(cpustate,ea);
				FPU_PUSH(cpustate,t);
				CYCLES(cpustate,10);
				break;
			}

			case 6:		// FBSTP
			{
				UINT8 res;
				double bcd_data;
				int i;

				bcd_data = ST(0).f;

				for(i=0;i<9;i++)
				{
					res = (UINT8)floor(fmod(bcd_data,10.0));
					bcd_data -= floor(fmod(bcd_data,10.0));
					bcd_data /= 10.0;
					res = (UINT8)floor(fmod(bcd_data,10.0))<<4;
					bcd_data -= floor(fmod(bcd_data,10.0));
					bcd_data /= 10.0;
					WRITE8(cpustate,ea+i,res);
				}
				res = (UINT8)floor(fmod(bcd_data,10.0));
				if(bcd_data < 0)
					res |= 0x80;
				WRITE8(cpustate,ea+9,res);
				FPU_POP(cpustate);
				CYCLES(cpustate,1); // TODO
				break;
			}

			case 7:		// FISTP long
			{
				X87_REG t;

				t = X87_FROUND(cpustate,ST(0));
				WRITE64(cpustate,ea,t.i);
				FPU_POP(cpustate);
				CYCLES(cpustate,29);
				break;
			}

			default:
				fatalerror("I386: FPU Op DF %02X at %08X", (modrm >> 3) & 7, cpustate->pc-2);
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
