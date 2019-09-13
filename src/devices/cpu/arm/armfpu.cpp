// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/*
 ARM3 coprocessor FPU
 
 TODO:
 - Currently not hooked up to the main core: we rely on undefined instruction traps instead;
 - This should also be a subdevice of ARM3. ARM250 doesn't have a FPU at all unless an additional board 
   is provided which interfaces with a WE32206 MAU (which has its own instruction set, fun)
 - This is a good candidate for unit testing, and I mean it;
*/

inline float32_t arm_cpu_device::ReadCoProRegOrConst32(const uint32_t idx)
{
	if (idx & 8)
	{
		float32_t table[8] = { 0x00000000, 0x3f800000, 0x40000000, 0x40400000, 0x40800000, 0x40a00000, 0x3f000000, 0x41200000 };
		return table[idx & 7];
	}
	
	return ui32_to_f32(m_coproRegister[idx]);
}

inline float64_t arm_cpu_device::ReadCoProRegOrConst64(const uint32_t idx)
{
	if (idx & 8)
	{
		float64_t table[8] = { 0x0000000000000000, 0x3ff0000000000000, 0x4000000000000000, 0x4008000000000000, 
							   0x4010000000000000, 0x4014000000000000, 0x3fe0000000000000, 0x4024000000000000 };
		return table[idx & 7];
	}
	
	return ui32_to_f64(m_coproRegister[idx]);
}

void arm_cpu_device::HandleFPU( uint32_t insn )
{
	uint32_t rn = (insn>>12)&0xf;
	uint32_t crn = (insn>>16)&0x7;
	uint32_t crn2 = insn & 0xf;
	// nearest, Plus infinity, Minus infinity, Zero
	const uint_fast8_t arm_to_sf_rounding[4] = {
		softfloat_round_near_even,
		softfloat_round_max,
		softfloat_round_min,
		softfloat_round_near_maxMag
	};
	uint8_t round_mode = arm_to_sf_rounding[(insn>>5)&0x3];
	// Single, Double, double Extended, Packed decimal, Extended Packed decimal
	// TODO: we don't currently emulate this until we get the actual specs (i.e. registers actual parallelism, how they transfers etc.)
	uint8_t precision_mode = (BIT(insn,20)<<1)|(BIT(insn,7));

	
	//ee105f10 OPCODE
	//0381167c
	//ee105f10 OPCODE
	//038b1094
	//ee502110 OPCODE
	//038fe1bc
	//ee502110 OPCODE
	//038fe1bc
	//ee301110 OPCODE
	//03877ac4

	// bughunt
	// ee888100 - SINE
	// ee988100 - COSE
	// dropship, powrband, starf3k
	// 2e472029 - <not a COPRO inst>
	// karmaft
	// 3e694c3c - <not a COPRO inst, again ASCII?>
	// ee80b180 - SIND 
	// ee90a180 - COSE
	// ee108109 - MNFS
	// ee109109 /
	// mig29 (need to take off, terrain fill shapes?)
	// eea08180 - TAND
	// simosor
	// 7e007e00 - <not a COPRO inst>

	// FLT - integer to floating point reg
	if((insn & 0x0ff00f1f) == 0x0e000110)
	{
		m_coproRegister[crn] = GetRegister(rn);
	}
	// FIX - floating point to integer reg
	else if ((insn & 0x0fff0f98) == 0x0e100110)
	{
		uint32_t result;
		
		switch(precision_mode)
		{
			case 0:
			{
				float32_t src = ui32_to_f32(m_coproRegister[crn]);
				result = f32_to_ui32(src, round_mode, true);
				break;
			}
			case 1:
			{
				float64_t src = ui32_to_f64(m_coproRegister[crn]);
				result = f64_to_ui32(src, round_mode, true);
				break;
			}
			default:
				printf("%08x:  Unimplemented VL86C020 copro instruction FIX%d %08x %d %d\n", R15 & 0x3fffffc, precision_mode, insn,rn,crn);
				return;
		}
		
		SetRegister(rn, result);
	}
	// Basic Math
	else if ((insn&0x0f008f10)==0x0e000100)
	{
		uint32_t result;
		uint8_t optype = (insn & 0xf00000) >> 20;
		rn &= 7;
		// fast versions always use 32-bit precision
		switch(optype)
		{
			case 0x0: // ADF - Add
			{
				switch(precision_mode)
				{
					case 0:
					{
						float32_t op1 = ui32_to_f32(m_coproRegister[crn]);
						float32_t op2 = ReadCoProRegOrConst32(crn2);
						float32_t fres = f32_add(op1, op2);
						result = f32_to_ui32(fres, round_mode, true);
						break;
					}
					case 1:
					{
						float64_t op1 = ui32_to_f64(m_coproRegister[crn]);
						float64_t op2 = ReadCoProRegOrConst64(crn2);
						float64_t fres = f64_add(op1, op2);
						result = f64_to_ui32(fres, round_mode, true);
						break;
					}
					default:
						printf("%08x:  Unimplemented VL86C020 copro instruction ADF%d %08x %d %d\n", R15 & 0x3fffffc, precision_mode, insn,rn,crn);
						return;
				}
				break;
			}
			case 0x1: // MUF - Multiply
			case 0x9: // FML - Fast multiply
			{
				float32_t op1 = ui32_to_f32(m_coproRegister[crn]);
				float32_t op2 = ReadCoProRegOrConst32(crn2);
				float32_t fres = f32_mul(op1, op2);
				result = f32_to_ui32(fres, round_mode, true);
				break;
			}
			case 0x2: // SUF - Subtract
			{
				switch(precision_mode)
				{
					case 0:
					{
						float32_t op1 = ui32_to_f32(m_coproRegister[crn]);
						float32_t op2 = ReadCoProRegOrConst32(crn2);
						float32_t fres = f32_sub(op1, op2);
						result = f32_to_ui32(fres, round_mode, true);
						break;
					}
					case 1:
					{
						float64_t op1 = ui32_to_f64(m_coproRegister[crn]);
						float64_t op2 = ReadCoProRegOrConst64(crn2);
						float64_t fres = f64_sub(op1, op2);
						result = f64_to_ui32(fres, round_mode, true);
						break;
					}
					default:
						printf("%08x:  Unimplemented VL86C020 copro instruction SUF%d %08x %d %d\n", R15 & 0x3fffffc, precision_mode, insn,rn,crn);
						return;
				}
				break;
			}
			case 0x3: // RSF - Reverse Subtract
			{
				float32_t op1 = ui32_to_f32(m_coproRegister[crn]);
				float32_t op2 = ReadCoProRegOrConst32(crn2);
				float32_t fres = f32_sub(op2, op1);
				result = f32_to_ui32(fres, round_mode, true);
				break;
			}
			case 0x4: // DVF - Divide
			case 0xa: // FDV - Fast Divide
			{
				float32_t op1 = ui32_to_f32(m_coproRegister[crn]);
				float32_t op2 = ReadCoProRegOrConst32(crn2);
				float32_t fres = f32_div(op1, op2);
				result = f32_to_ui32(fres, round_mode, true);
				break;
			}
			case 0x5: // RDF - Reverse Divide
			case 0xb: // FRD - Fast Reverse Divide
			{
				float32_t op1 = ui32_to_f32(m_coproRegister[crn]);
				float32_t op2 = ReadCoProRegOrConst32(crn2);
				float32_t fres = f32_div(op2, op1);
				result = f32_to_ui32(fres, round_mode, true);
				break;
			}
			case 0x8: // RMF - Remainder 
			{
				float32_t op1 = ui32_to_f32(m_coproRegister[crn]);
				float32_t op2 = ReadCoProRegOrConst32(crn2);
				float32_t fres = f32_rem(op1, op2);
				result = f32_to_ui32(fres, round_mode, true);
				break;
			}
			case 0x6: // POW - Power
			case 0x7: // RPW - Reverse Power
				printf("%08x:  Unimplemented VL86C020 copro instruction POW/RPW %08x %d %d\n", R15 & 0x3fffffc, insn,rn,crn);
				return;
			case 0xc: // POL - Polar Angle
				printf("%08x:  Unimplemented VL86C020 copro instruction POL %08x %d %d\n", R15 & 0x3fffffc, insn,rn,crn);
				return;
			default:
				printf("%08x:  Illegal VL86C020 copro instruction %08x %d %d\n", R15 & 0x3fffffc, insn,rn,crn);
				return;
		}

		m_coproRegister[rn] = result;
	}
	// Advanced Math
	else if ((insn&0x0f008f10)==0x0e008100)
	{
		uint32_t result;
		uint8_t optype = (insn & 0xf00000) >> 20;
		rn &= 7;
//		float32_t op1 = ui32_to_f32(m_coproRegister[crn]);
//		float32_t op2 = ReadCoProRegOrConst32(crn2);
		
		switch(optype)
		{
			case 0x0: // MVF - Move
			case 0x1: // MNF - Move Negated
			{
				switch(precision_mode)
				{
					case 0:
					{
						float32_t fres = ReadCoProRegOrConst32(crn2);
						result = f32_to_ui32(fres, round_mode, true);
						if (optype & 1)
							result |= 1 << 31;
						break;
					}
					case 1:
					{
						float64_t fres = ReadCoProRegOrConst64(crn2);
						result = f64_to_ui32(fres, round_mode, true);
						if (optype & 1)
							result |= 1 << 31;
						break;
					}
					default:
						printf("%08x:  Unimplemented VL86C020 copro instruction MVF/MNF%d %08x %d %d\n", R15 & 0x3fffffc, precision_mode, insn,rn,crn);
						return;
				}
			}
			case 0x2: // ABS - Absolute
				printf("%08x:  Unimplemented VL86C020 copro instruction ABS %08x %d %d\n", R15 & 0x3fffffc, insn,rn,crn);
				return;
			case 0x3: // RND - Round to Integral
				printf("%08x:  Unimplemented VL86C020 copro instruction RND %08x %d %d\n", R15 & 0x3fffffc, insn,rn,crn);
				return;
			case 0x4: // SQT - Square Root
				printf("%08x:  Unimplemented VL86C020 copro instruction SQT %08x %d %d\n", R15 & 0x3fffffc, insn,rn,crn);
				return;
			case 0x5: // LOG - logarithm to base 10
			case 0x6: // LGN - logarithm to base e
				printf("%08x:  Unimplemented VL86C020 copro instruction LOG/LGN %08x %d %d\n", R15 & 0x3fffffc, insn,rn,crn);
				return;
			case 0x7: // EXP - Exponent
				printf("%08x:  Unimplemented VL86C020 copro instruction EXP %08x %d %d\n", R15 & 0x3fffffc, insn,rn,crn);
				return;
			case 0x8: // SIN - Sine
			case 0x9: // COS - Cosine
			case 0xa: // TAN - Tangent
				printf("%08x:  Unimplemented VL86C020 copro instruction SIN/COS/TAN %08x %d %d\n", R15 & 0x3fffffc, insn,rn,crn);
				return;
			case 0xb: // ASN - Arc Sine
				printf("%08x:  Unimplemented VL86C020 copro instruction ASN %08x %d %d\n", R15 & 0x3fffffc, insn,rn,crn);
				return;
			case 0xc: // ACS - Arc Cosine
				printf("%08x:  Unimplemented VL86C020 copro instruction ACS %08x %d %d\n", R15 & 0x3fffffc, insn,rn,crn);
				return;
			case 0xd: // ATN - Arc Tangent
				printf("%08x:  Unimplemented VL86C020 copro instruction ATN %08x %d %d\n", R15 & 0x3fffffc, insn,rn,crn);
				return;
			case 0xe: // URD - Unnormalized Round
				printf("%08x:  Unimplemented VL86C020 copro instruction URD %08x %d %d\n", R15 & 0x3fffffc, insn,rn,crn);
				return;
			case 0xf: // NRM - Normalise
				printf("%08x:  Unimplemented VL86C020 copro instruction NRM %08x %d %d\n", R15 & 0x3fffffc, insn,rn,crn);
				return;
		}
		
		m_coproRegister[rn] = result;
	}
	// CMF - compare
	else if ( (insn&0x0ff8fff0) == 0x0e90f110)
	{
		bool result;
		
		// TODO: according to "The Hacker" on AA, op2 ignores constants here, needs to be counterchecked.
		switch(precision_mode)
		{
			case 0:
			{
				float32_t op1 = ui32_to_f32(m_coproRegister[crn]);
				float32_t op2 = ui32_to_f32(m_coproRegister[crn2 & 7]);
				result = f32_eq(op1, op2);
				break;
			}
			case 1:
			{
				float64_t op1 = ui32_to_f64(m_coproRegister[crn]);
				float64_t op2 = ui32_to_f64(m_coproRegister[crn2 & 7]);
				result = f64_eq(op1, op2);
				break;
			}
			default:
				printf("%08x:  Unimplemented VL86C020 copro instruction CMF%d %08x %d %d\n", R15 & 0x3fffffc, precision_mode, insn,rn,crn);
				result = false;
				return;
		}
		
		// TODO: verify if it's gonna go to R15 or in its own FPSR
		if (result == true)
			R15 |= (Z_MASK);
		else
			R15 &= ~(Z_MASK);

		// TODO: V/C/N flags
		R15 &= ~(V_MASK);
	}
	else
	{
		printf("%08x:  Unimplemented VL86C020 copro instruction %08x %d %d\n", R15 & 0x3ffffff, insn,rn,crn);
		machine().debug_break();
	}
}
