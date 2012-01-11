/*****************************************************************************
 *
 *   arm7exec.c
 *   Portable ARM7TDMI Core Emulator
 *
 *   Copyright Steve Ellenoff, all rights reserved.
 *
 *   - This source code is released as freeware for non-commercial purposes.
 *   - You are free to use and redistribute this code in modified or
 *     unmodified form, provided you list me in the credits.
 *   - If you modify this source code, you must add a notice to each modified
 *     source file that it has been changed.  If you're a nice person, you
 *     will clearly mark each change too.  :)
 *   - If you wish to use this for commercial purposes, please contact me at
 *     sellenoff@hotmail.com
 *   - The author of this copywritten work reserves the right to change the
 *     terms of its usage and license at any time, including retroactively
 *   - This entire notice must remain in the source code.
 *
 *  This work is based on:
 *  #1) 'Atmel Corporation ARM7TDMI (Thumb) Datasheet - January 1999'
 *  #2) Arm 2/3/6 emulator By Bryan McPhail (bmcphail@tendril.co.uk) and Phil Stroffolino (MAME CORE 0.76)
 *
 *****************************************************************************/

/******************************************************************************
 *  Notes:
 *         This file contains the code to run during the CPU EXECUTE METHOD.
 *         It has been split into it's own file (from the arm7core.c) so it can be
 *         directly compiled into any cpu core that wishes to use it.
 *
 *         It should be included as follows in your cpu core:
 *
 *         CPU_EXECUTE( arm7 )
 *         {
 *         #include "arm7exec.c"
 *         }
 *
*****************************************************************************/


/* This implementation uses an improved switch() for hopefully faster opcode fetches compared to my last version
.. though there's still room for improvement. */
{
    UINT32 pc;
    UINT32 insn;
    arm_state *cpustate = get_safe_token(device);

    do
    {
        debugger_instruction_hook(cpustate->device, GET_PC);

        /* handle Thumb instructions if active */
        if (T_IS_SET(GET_CPSR))
        {
			UINT32 raddr;

            pc = R15;

			// "In Thumb state, bit [0] is undefined and must be ignored. Bits [31:1] contain the PC."
			raddr = pc & (~1);

			if ( COPRO_CTRL & COPRO_CTRL_MMU_EN )
			{
	    		if (!arm7_tlb_translate(cpustate, &raddr, ARM7_TLB_ABORT_P | ARM7_TLB_READ))
	    		{
	    			goto skip_exec;
	    		}
			}

			insn = cpustate->direct->read_decrypted_word(raddr);
			thumb_handler[(insn & THUMB_INSN_TYPE) >> THUMB_INSN_TYPE_SHIFT](cpustate, pc, insn);
          
        }
        else
        {
			UINT32 raddr;

            /* load 32 bit instruction */
            pc = GET_PC;

			// "In ARM state, bits [1:0] of r15 are undefined and must be ignored. Bits [31:2] contain the PC."
			raddr = pc & (~3);

	    if ( COPRO_CTRL & COPRO_CTRL_MMU_EN )
	    {
	    	if (!arm7_tlb_translate(cpustate, &raddr, ARM7_TLB_ABORT_P | ARM7_TLB_READ))
	    	{
	    		goto skip_exec;
	    	}
	    }

#if 0
			if (MODE26)
			{
				UINT32 temp1, temp2;
				temp1 = GET_CPSR & 0xF00000C3;
				temp2 = (R15 & 0xF0000000) | ((R15 & 0x0C000000) >> (26 - 6)) | (R15 & 0x00000003);
				if (temp1 != temp2) fatalerror( "%08X: 32-bit and 26-bit modes are out of sync (%08X %08X)", pc, temp1, temp2);
			}
#endif

            insn = cpustate->direct->read_decrypted_dword(raddr);

            /* process condition codes for this instruction */
            switch (insn >> INSN_COND_SHIFT)
            {
            case COND_EQ:
                if (Z_IS_CLEAR(GET_CPSR))
                    goto L_Next;
                break;
            case COND_NE:
                if (Z_IS_SET(GET_CPSR))
                    goto L_Next;
                break;
            case COND_CS:
                if (C_IS_CLEAR(GET_CPSR))
                    goto L_Next;
                break;
            case COND_CC:
                if (C_IS_SET(GET_CPSR))
                    goto L_Next;
                break;
            case COND_MI:
                if (N_IS_CLEAR(GET_CPSR))
                    goto L_Next;
                break;
            case COND_PL:
                if (N_IS_SET(GET_CPSR))
                    goto L_Next;
                break;
            case COND_VS:
                if (V_IS_CLEAR(GET_CPSR))
                    goto L_Next;
                break;
            case COND_VC:
                if (V_IS_SET(GET_CPSR))
                    goto L_Next;
                break;
            case COND_HI:
                if (C_IS_CLEAR(GET_CPSR) || Z_IS_SET(GET_CPSR))
                    goto L_Next;
                break;
            case COND_LS:
                if (C_IS_SET(GET_CPSR) && Z_IS_CLEAR(GET_CPSR))
                    goto L_Next;
                break;
            case COND_GE:
                if (!(GET_CPSR & N_MASK) != !(GET_CPSR & V_MASK)) /* Use x ^ (x >> ...) method */
                    goto L_Next;
                break;
            case COND_LT:
                if (!(GET_CPSR & N_MASK) == !(GET_CPSR & V_MASK))
                    goto L_Next;
                break;
            case COND_GT:
                if (Z_IS_SET(GET_CPSR) || (!(GET_CPSR & N_MASK) != !(GET_CPSR & V_MASK)))
                    goto L_Next;
                break;
            case COND_LE:
                if (Z_IS_CLEAR(GET_CPSR) && (!(GET_CPSR & N_MASK) == !(GET_CPSR & V_MASK)))
                  goto L_Next;
                break;
            case COND_NV:
                goto L_Next;
            }
            /*******************************************************************/
            /* If we got here - condition satisfied, so decode the instruction */
            /*******************************************************************/
            switch ((insn & 0xF000000) >> 24)
            {
                case 0:
                case 1:
                case 2:
                case 3:
                    /* Branch and Exchange (BX) */
                    if ((insn & 0x0ffffff0) == 0x012fff10)     // bits 27-4 == 000100101111111111110001
                    {
                        R15 = GET_REGISTER(cpustate, insn & 0x0f);
                        // If new PC address has A0 set, switch to Thumb mode
                        if (R15 & 1) {
                            SET_CPSR(GET_CPSR|T_MASK);
                            R15--;
                        }
                    }
		    else if ((insn & 0x0ff000f0) == 0x01600010)	// CLZ - v5
		    {
		    	UINT32 rm = insn&0xf;
			UINT32 rd = (insn>>12)&0xf;

			SET_REGISTER(cpustate, rd, count_leading_zeros(GET_REGISTER(cpustate, rm)));

			R15 += 4;
		    }
		    else if ((insn & 0x0ff000f0) == 0x01000050)	// QADD - v5
		    {
		    	INT32 src1 = GET_REGISTER(cpustate, insn&0xf);
			INT32 src2 = GET_REGISTER(cpustate, (insn>>16)&0xf);
			INT64 res;

			res = saturate_qbit_overflow(cpustate, (INT64)src1 + (INT64)src2);

			SET_REGISTER(cpustate, (insn>>12)&0xf, (INT32)res);
			R15 += 4;
		    }
		    else if ((insn & 0x0ff000f0) == 0x01400050)	// QDADD - v5
		    {
		    	INT32 src1 = GET_REGISTER(cpustate, insn&0xf);
			INT32 src2 = GET_REGISTER(cpustate, (insn>>16)&0xf);
			INT64 res;

			// check if doubling operation will overflow
			res = (INT64)src2 * 2;
			saturate_qbit_overflow(cpustate, res);

			src2 *= 2;
			res = saturate_qbit_overflow(cpustate, (INT64)src1 + (INT64)src2);

			SET_REGISTER(cpustate, (insn>>12)&0xf, (INT32)res);
			R15 += 4;
		    }
		    else if ((insn & 0x0ff000f0) == 0x01200050)	// QSUB - v5
		    {
		    	INT32 src1 = GET_REGISTER(cpustate, insn&0xf);
			INT32 src2 = GET_REGISTER(cpustate, (insn>>16)&0xf);
			INT64 res;

			res = saturate_qbit_overflow(cpustate, (INT64)src1 - (INT64)src2);

			SET_REGISTER(cpustate, (insn>>12)&0xf, (INT32)res);
			R15 += 4;
		    }
		    else if ((insn & 0x0ff000f0) == 0x01600050)	// QDSUB - v5
		    {
		    	INT32 src1 = GET_REGISTER(cpustate, insn&0xf);
			INT32 src2 = GET_REGISTER(cpustate, (insn>>16)&0xf);
			INT64 res;

			// check if doubling operation will overflow
			res = (INT64)src2 * 2;
			saturate_qbit_overflow(cpustate, res);

			src2 *= 2;
			res = saturate_qbit_overflow(cpustate, (INT64)src1 - (INT64)src2);

			SET_REGISTER(cpustate, (insn>>12)&0xf, (INT32)res);
			R15 += 4;
		    }
		    else if ((insn & 0x0ff00090) == 0x01000080)	// SMLAxy - v5
		    {
		    	INT32 src1 = GET_REGISTER(cpustate, insn&0xf);
			INT32 src2 = GET_REGISTER(cpustate, (insn>>8)&0xf);
			INT32 res1;

			// select top and bottom halves of src1/src2 and sign extend if necessary
			if (insn & 0x20)
			{
				src1 >>= 16;
			}
			else
			{
				src1 &= 0xffff;
				if (src1 & 0x8000)
				{
					src1 |= 0xffff;
				}
			}

			if (insn & 0x40)
			{
				src2 >>= 16;
			}
			else
			{
				src2 &= 0xffff;
				if (src2 & 0x8000)
				{
					src2 |= 0xffff;
				}
			}

			// do the signed multiply
			res1 = src1 * src2;
			// and the accumulate.  NOTE: only the accumulate can cause an overflow, which is why we do it this way.
			saturate_qbit_overflow(cpustate, (INT64)res1 + (INT64)GET_REGISTER(cpustate, (insn>>12)&0xf));

			SET_REGISTER(cpustate, (insn>>16)&0xf, res1 + GET_REGISTER(cpustate, (insn>>12)&0xf));
			R15 += 4;
		    }
		    else if ((insn & 0x0ff00090) == 0x01400080)	// SMLALxy - v5
		    {
		    	INT32 src1 = GET_REGISTER(cpustate, insn&0xf);
			INT32 src2 = GET_REGISTER(cpustate, (insn>>8)&0xf);
			INT64 dst;

			// select top and bottom halves of src1/src2 and sign extend if necessary
			if (insn & 0x20)
			{
				src1 >>= 16;
			}
			else
			{
				src1 &= 0xffff;
				if (src1 & 0x8000)
				{
					src1 |= 0xffff;
				}
			}

			if (insn & 0x40)
			{
				src2 >>= 16;
			}
			else
			{
				src2 &= 0xffff;
				if (src2 & 0x8000)
				{
					src2 |= 0xffff;
				}
			}

			dst = (INT64)GET_REGISTER(cpustate, (insn>>12)&0xf);
			dst |= (INT64)GET_REGISTER(cpustate, (insn>>16)&0xf)<<32;

			// do the multiply and accumulate
			dst += (INT64)src1 * (INT64)src2;

			// write back the result
			SET_REGISTER(cpustart, (insn>>12)&0xf, (UINT32)(dst&0xffffffff));
			SET_REGISTER(cpustart, (insn>>16)&0xf, (UINT32)(dst>>32));
		    }
		    else if ((insn & 0x0ff00090) == 0x01600080)	// SMULxy - v5
		    {
		    	INT32 src1 = GET_REGISTER(cpustate, insn&0xf);
			INT32 src2 = GET_REGISTER(cpustate, (insn>>8)&0xf);
			INT32 res;

			// select top and bottom halves of src1/src2 and sign extend if necessary
			if (insn & 0x20)
			{
				src1 >>= 16;
			}
			else
			{
				src1 &= 0xffff;
				if (src1 & 0x8000)
				{
					src1 |= 0xffff;
				}
			}

			if (insn & 0x40)
			{
				src2 >>= 16;
			}
			else
			{
				src2 &= 0xffff;
				if (src2 & 0x8000)
				{
					src2 |= 0xffff;
				}
			}

			res = src1 * src2;
			SET_REGISTER(cpustart, (insn>>16)&0xf, res);
		    }
		    else if ((insn & 0x0ff000b0) == 0x012000a0)	// SMULWy - v5
		    {
		    	INT32 src1 = GET_REGISTER(cpustate, insn&0xf);
			INT32 src2 = GET_REGISTER(cpustate, (insn>>8)&0xf);
			INT64 res;

			if (insn & 0x40)
			{
				src2 >>= 16;
			}
			else
			{
				src2 &= 0xffff;
				if (src2 & 0x8000)
				{
					src2 |= 0xffff;
				}
			}

			res = (INT64)src1 * (INT64)src2;
			res >>= 16;
			SET_REGISTER(cpustart, (insn>>16)&0xf, (UINT32)res);
		    }
		    else if ((insn & 0x0ff000b0) == 0x01200080)	// SMLAWy - v5
		    {
		    	INT32 src1 = GET_REGISTER(cpustate, insn&0xf);
			INT32 src2 = GET_REGISTER(cpustate, (insn>>8)&0xf);
			INT32 src3 = GET_REGISTER(cpustate, (insn>>12)&0xf);
			INT64 res;

			if (insn & 0x40)
			{
				src2 >>= 16;
			}
			else
			{
				src2 &= 0xffff;
				if (src2 & 0x8000)
				{
					src2 |= 0xffff;
				}
			}

			res = (INT64)src1 * (INT64)src2;
			res >>= 16;

			// check for overflow and set the Q bit
			saturate_qbit_overflow(cpustate, (INT64)src3 + res);

			// do the real accumulate
			src3 += (INT32)res;

			// write the result back
			SET_REGISTER(cpustart, (insn>>16)&0xf, (UINT32)res);
		    }
                    else
                    /* Multiply OR Swap OR Half Word Data Transfer */
                    if ((insn & 0x0e000000) == 0 && (insn & 0x80) && (insn & 0x10))  // bits 27-25=000 bit 7=1 bit 4=1
                    {
                        /* Half Word Data Transfer */
                        if (insn & 0x60)         // bits = 6-5 != 00
                        {
                            HandleHalfWordDT(cpustate, insn);
                        }
                        else
                        /* Swap */
                        if (insn & 0x01000000)   // bit 24 = 1
                        {
                            HandleSwap(cpustate, insn);
                        }
                        /* Multiply Or Multiply Long */
                        else
                        {
                            /* multiply long */
                            if (insn & 0x800000) // Bit 23 = 1 for Multiply Long
                            {
                                /* Signed? */
                                if (insn & 0x00400000)
                                    HandleSMulLong(cpustate, insn);
                                else
                                    HandleUMulLong(cpustate, insn);
                            }
                            /* multiply */
                            else
                            {
                                HandleMul(cpustate, insn);
                            }
                            R15 += 4;
                        }
                    }
                    else
                    /* Data Processing OR PSR Transfer */
                    if ((insn & 0x0c000000) == 0)   // bits 27-26 == 00 - This check can only exist properly after Multiplication check above
                    {
                        /* PSR Transfer (MRS & MSR) */
                        if (((insn & 0x00100000) == 0) && ((insn & 0x01800000) == 0x01000000)) // S bit must be clear, and bit 24,23 = 10
                        {
                            HandlePSRTransfer(cpustate, insn);
                            ARM7_ICOUNT += 2;       // PSR only takes 1 - S Cycle, so we add + 2, since at end, we -3..
                            R15 += 4;
                        }
                        /* Data Processing */
                        else
                        {
                            HandleALU(cpustate, insn);
                        }
                    }
                    break;
                /* Data Transfer - Single Data Access */
                case 4:
                case 5:
                case 6:
                case 7:
                    HandleMemSingle(cpustate, insn);
                    R15 += 4;
                    break;
                /* Block Data Transfer/Access */
                case 8:
                case 9:
                    HandleMemBlock(cpustate, insn);
                    R15 += 4;
                    break;
                /* Branch or Branch & Link */
                case 0xa:
                case 0xb:
                    HandleBranch(cpustate, insn);
                    break;
                /* Co-Processor Data Transfer */
                case 0xc:
                case 0xd:
                    HandleCoProcDT(cpustate, insn);
                    R15 += 4;
                    break;
                /* Co-Processor Data Operation or Register Transfer */
                case 0xe:
                    if (insn & 0x10)
                        HandleCoProcRT(cpustate, insn);
                    else
                        HandleCoProcDO(cpustate, insn);
                    R15 += 4;
                    break;
                /* Software Interrupt */
                case 0x0f:
                    cpustate->pendingSwi = 1;
                    ARM7_CHECKIRQ;
                    //couldn't find any cycle counts for SWI
                    break;
                /* Undefined */
                default:
                    cpustate->pendingSwi = 1;
                    ARM7_CHECKIRQ;
                    ARM7_ICOUNT -= 1;               //undefined takes 4 cycles (page 77)
                    LOG(("%08x:  Undefined instruction\n",pc-4));
                    L_Next:
                        R15 += 4;
                        ARM7_ICOUNT +=2;    //Any unexecuted instruction only takes 1 cycle (page 193)
            }
        }

skip_exec:

        ARM7_CHECKIRQ;

        /* All instructions remove 3 cycles.. Others taking less / more will have adjusted this # prior to here */
        ARM7_ICOUNT -= 3;
    } while (ARM7_ICOUNT > 0);
}
