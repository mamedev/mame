// license:BSD-3-Clause
// copyright-holders:Steve Ellenoff,R. Belmont,Ryan Holtz
#include "emu.h"
#include "arm7.h"
#include "arm7core.h"
#include "arm7help.h"

INT64 arm7_cpu_device::saturate_qbit_overflow(INT64 res)
{
	if (res > 2147483647)   // INT32_MAX
	{   // overflow high? saturate and set Q
		res = 2147483647;
		SET_CPSR(GET_CPSR | Q_MASK);
	}
	else if (res < (-2147483647-1)) // INT32_MIN
	{   // overflow low? saturate and set Q
		res = (-2147483647-1);
		SET_CPSR(GET_CPSR | Q_MASK);
	}

	return res;
}


void arm7_cpu_device::SwitchMode(UINT32 cpsr_mode_val)
{
	UINT32 cspr = m_r[eCPSR] & ~MODE_FLAG;
	set_cpsr(cspr | cpsr_mode_val);
}


/* Decodes an Op2-style shifted-register form.  If @carry@ is non-zero the
 * shifter carry output will manifest itself as @*carry == 0@ for carry clear
 * and @*carry != 0@ for carry set.

   SJE: Rules:
   IF RC = 256, Result = no shift.
   LSL   0   = Result = RM, Carry = Old Contents of CPSR C Bit
   LSL(0,31) = Result shifted, least significant bit is in carry out
   LSL  32   = Result of 0, Carry = Bit 0 of RM
   LSL >32   = Result of 0, Carry out 0
   LSR   0   = LSR 32 (see below)
   LSR  32   = Result of 0, Carry = Bit 31 of RM
   LSR >32   = Result of 0, Carry out 0
   ASR >=32  = ENTIRE Result = bit 31 of RM
   ROR  32   = Result = RM, Carry = Bit 31 of RM
   ROR >32   = Same result as ROR n-32 until amount in range of 1-32 then follow rules
*/

UINT32 arm7_cpu_device::decodeShift(UINT32 insn, UINT32 *pCarry)
{
	UINT32 k  = (insn & INSN_OP2_SHIFT) >> INSN_OP2_SHIFT_SHIFT;  // Bits 11-7
	UINT32 rm = GET_REGISTER(insn & INSN_OP2_RM);
	UINT32 t  = (insn & INSN_OP2_SHIFT_TYPE) >> INSN_OP2_SHIFT_TYPE_SHIFT;

	if ((insn & INSN_OP2_RM) == 0xf) {
		// "If a register is used to specify the shift amount the PC will be 12 bytes ahead." (instead of 8)
		rm += t & 1 ? 12 : 8;
	}

	/* All shift types ending in 1 are Rk, not #k */
	if (t & 1)
	{
//      LOG(("%08x:  RegShift %02x %02x\n", R15, k >> 1, GET_REGISTER(k >> 1)));
#if ARM7_DEBUG_CORE
			if ((insn & 0x80) == 0x80)
				LOG(("%08x:  RegShift ERROR (p36)\n", R15));
#endif

		// see p35 for check on this
		//k = GET_REGISTER(k >> 1) & 0x1f;

		// Keep only the bottom 8 bits for a Register Shift
		k = GET_REGISTER(k >> 1) & 0xff;

		if (k == 0) /* Register shift by 0 is a no-op */
		{
//          LOG(("%08x:  NO-OP Regshift\n", R15));
			/* TODO this is wrong for at least ROR by reg with lower
			 *      5 bits 0 but lower 8 bits non zero */
			if (pCarry)
				*pCarry = GET_CPSR & C_MASK;
			return rm;
		}
	}
	/* Decode the shift type and perform the shift */
	switch (t >> 1)
	{
	case 0:                     /* LSL */
		// LSL  32   = Result of 0, Carry = Bit 0 of RM
		// LSL >32   = Result of 0, Carry out 0
		if (k >= 32)
		{
			if (pCarry)
				*pCarry = (k == 32) ? rm & 1 : 0;
			return 0;
		}
		else
		{
			if (pCarry)
			{
			// LSL      0   = Result = RM, Carry = Old Contents of CPSR C Bit
			// LSL (0,31)   = Result shifted, least significant bit is in carry out
			*pCarry = k ? (rm & (1 << (32 - k))) : (GET_CPSR & C_MASK);
			}
			return k ? LSL(rm, k) : rm;
		}

	case 1:                         /* LSR */
		if (k == 0 || k == 32)
		{
			if (pCarry)
				*pCarry = rm & SIGN_BIT;
			return 0;
		}
		else if (k > 32)
		{
			if (pCarry)
				*pCarry = 0;
			return 0;
		}
		else
		{
			if (pCarry)
				*pCarry = (rm & (1 << (k - 1)));
			return LSR(rm, k);
		}

	case 2:                     /* ASR */
		if (k == 0 || k > 32)
			k = 32;

		if (pCarry)
			*pCarry = (rm & (1 << (k - 1)));
		if (k >= 32)
			return rm & SIGN_BIT ? 0xffffffffu : 0;
		else
		{
			if (rm & SIGN_BIT)
				return LSR(rm, k) | (0xffffffffu << (32 - k));
			else
				return LSR(rm, k);
		}

	case 3:                     /* ROR and RRX */
		if (k)
		{
			while (k > 32)
				k -= 32;
			if (pCarry)
				*pCarry = rm & (1 << (k - 1));
			return ROR(rm, k);
		}
		else
		{
			/* RRX */
			if (pCarry)
				*pCarry = (rm & 1);
			return LSR(rm, 1) | ((GET_CPSR & C_MASK) << 2);
		}
	}

	LOG(("%08x: Decodeshift error\n", R15));
	return 0;
} /* decodeShift */


int arm7_cpu_device::loadInc(UINT32 pat, UINT32 rbv, UINT32 s, int mode)
{
	int i, result;
	UINT32 data;

	result = 0;
	rbv &= ~3;
	for (i = 0; i < 16; i++)
	{
		if ((pat >> i) & 1)
		{
			if (m_pendingAbtD == 0) // "Overwriting of registers stops when the abort happens."
			{
			data = READ32(rbv += 4);
			if (i == 15) {
				if (s) /* Pull full contents from stack */
					SET_MODE_REGISTER(mode, 15, data);
				else /* Pull only address, preserve mode & status flags */
					if (MODE32)
						SET_MODE_REGISTER(mode, 15, data);
					else
					{
						SET_MODE_REGISTER(mode, 15, (GET_MODE_REGISTER(mode, 15) & ~0x03FFFFFC) | (data & 0x03FFFFFC));
					}
			} else
				SET_MODE_REGISTER(mode, i, data);
			}
			result++;
		}
	}
	return result;
}


int arm7_cpu_device::loadDec(UINT32 pat, UINT32 rbv, UINT32 s, int mode)
{
	int i, result;
	UINT32 data;

	result = 0;
	rbv &= ~3;
	for (i = 15; i >= 0; i--)
	{
		if ((pat >> i) & 1)
		{
			if (m_pendingAbtD == 0) // "Overwriting of registers stops when the abort happens."
			{
			data = READ32(rbv -= 4);
			if (i == 15) {
				if (s) /* Pull full contents from stack */
					SET_MODE_REGISTER(mode, 15, data);
				else /* Pull only address, preserve mode & status flags */
					if (MODE32)
						SET_MODE_REGISTER(mode, 15, data);
					else
					{
						SET_MODE_REGISTER(mode, 15, (GET_MODE_REGISTER(mode, 15) & ~0x03FFFFFC) | (data & 0x03FFFFFC));
					}
			}
			else
				SET_MODE_REGISTER(mode, i, data);
			}
			result++;
		}
	}
	return result;
}


int arm7_cpu_device::storeInc(UINT32 pat, UINT32 rbv, int mode)
{
	int i, result;

	result = 0;
	for (i = 0; i < 16; i++)
	{
		if ((pat >> i) & 1)
		{
#if ARM7_DEBUG_CORE
			if (i == 15) /* R15 is plus 12 from address of STM */
				LOG(("%08x: StoreInc on R15\n", R15));
#endif
			WRITE32(rbv += 4, GET_MODE_REGISTER(mode, i));
			result++;
		}
	}
	return result;
} /* storeInc */


int arm7_cpu_device::storeDec(UINT32 pat, UINT32 rbv, int mode)
{
	int i, result = 0, cnt;

	// pre-count the # of registers being stored
	for (i = 15; i >= 0; i--)
	{
		if ((pat >> i) & 1)
		{
			result++;

			// starting address
			rbv -= 4;
		}
	}

	cnt = 0;
	for (i = 0; i <= 15; i++)
	{
		if ((pat >> i) & 1)
		{
#if ARM7_DEBUG_CORE
			if (i == 15) /* R15 is plus 12 from address of STM */
				LOG(("%08x: StoreDec on R15\n", R15));
#endif
			WRITE32(rbv + (cnt * 4), GET_MODE_REGISTER(mode, i));
			cnt++;
		}
	}
	return result;
} /* storeDec */


/***************************************************************************
 *                            OPCODE HANDLING
 ***************************************************************************/

// Co-Processor Data Operation
void arm7_cpu_device::HandleCoProcDO(UINT32 insn)
{
	// This instruction simply instructs the co-processor to do something, no data is returned to ARM7 core
	arm7_do_callback(*m_program, insn, 0, 0);    // simply pass entire opcode to callback - since data format is actually dependent on co-proc implementation
}

// Co-Processor Register Transfer - To/From Arm to Co-Proc
void arm7_cpu_device::HandleCoProcRT(UINT32 insn)
{
	/* xxxx 1110 oooL nnnn dddd cccc ppp1 mmmm */

	// Load (MRC) data from Co-Proc to ARM7 register
	if (insn & 0x00100000)       // Bit 20 = Load or Store
	{
		UINT32 res = arm7_rt_r_callback(*m_program, insn, 0);   // RT Read handler must parse opcode & return appropriate result
		if (m_pendingUnd == 0)
		{
			SET_REGISTER((insn >> 12) & 0xf, res);
		}
	}
	// Store (MCR) data from ARM7 to Co-Proc register
	else
	{
		arm7_rt_w_callback(*m_program, insn, GET_REGISTER((insn >> 12) & 0xf), 0);
	}
}

/* Data Transfer - To/From Arm to Co-Proc
   Loading or Storing, the co-proc function is responsible to read/write from the base register supplied + offset
   8 bit immediate value Base Offset address is << 2 to get the actual #

  issues - #1 - the co-proc function, needs direct access to memory reads or writes (ie, so we must send a pointer to a func)
         - #2 - the co-proc may adjust the base address (especially if it reads more than 1 word), so a pointer to the register must be used
                but the old value of the register must be restored if write back is not set..
         - #3 - when post incrementing is used, it's up to the co-proc func. to add the offset, since the transfer
                address supplied in that case, is simply the base. I suppose this is irrelevant if write back not set
                but if co-proc reads multiple address, it must handle the offset adjustment itself.
*/
// todo: test with valid instructions
void arm7_cpu_device::HandleCoProcDT(UINT32 insn)
{
	UINT32 rn = (insn >> 16) & 0xf;
	UINT32 rnv = GET_REGISTER(rn);    // Get Address Value stored from Rn
	UINT32 ornv = rnv;                // Keep value of Rn
	UINT32 off = (insn & 0xff) << 2;  // Offset is << 2 according to manual
	UINT32 *prn = &ARM7REG(rn);       // Pointer to our register, so it can be changed in the callback

#if ARM7_DEBUG_CORE
	if (((insn >> 16) & 0xf) == 15 && (insn & 0x200000))
		LOG(("%08x: Illegal use of R15 as base for write back value!\n", R15));
#endif

	// Pre-Increment base address (IF POST INCREMENT - CALL BACK FUNCTION MUST DO IT)
	if ((insn & 0x1000000) && off)
	{
		// Up - Down bit
		if (insn & 0x800000)
			rnv += off;
		else
			rnv -= off;
	}

	// Load (LDC) data from ARM7 memory to Co-Proc memory
	if (insn & 0x00100000)
	{
		arm7_dt_r_callback(insn, prn);
	}
	// Store (STC) data from Co-Proc to ARM7 memory
	else
	{
		arm7_dt_w_callback(insn, prn);
	}

	if (m_pendingUnd != 0) return;

	// If writeback not used - ensure the original value of RN is restored in case co-proc callback changed value
	if ((insn & 0x200000) == 0)
		SET_REGISTER(rn, ornv);
}

void arm7_cpu_device::HandleBranch(UINT32 insn)
{
	UINT32 off = (insn & INSN_BRANCH) << 2;

	/* Save PC into LR if this is a branch with link */
	if (insn & INSN_BL)
	{
		SET_REGISTER(14, R15 + 4);
	}

	/* Sign-extend the 24-bit offset in our calculations */
	if (off & 0x2000000u)
	{
		if (MODE32)
			R15 -= ((~(off | 0xfc000000u)) + 1) - 8;
		else
			R15 = ((R15 - (((~(off | 0xfc000000u)) + 1) - 8)) & 0x03FFFFFC) | (R15 & ~0x03FFFFFC);
	}
	else
	{
		if (MODE32)
			R15 += off + 8;
		else
			R15 = ((R15 + (off + 8)) & 0x03FFFFFC) | (R15 & ~0x03FFFFFC);
	}
}

void arm7_cpu_device::HandleMemSingle(UINT32 insn)
{
	UINT32 rn, rnv, off, rd, rnv_old = 0;

	/* Fetch the offset */
	if (insn & INSN_I)
	{
		/* Register Shift */
		off = decodeShift(insn, nullptr);
	}
	else
	{
		/* Immediate Value */
		off = insn & INSN_SDT_IMM;
	}

	/* Calculate Rn, accounting for PC */
	rn = (insn & INSN_RN) >> INSN_RN_SHIFT;

	if (insn & INSN_SDT_P)
	{
		/* Pre-indexed addressing */
		if (insn & INSN_SDT_U)
		{
			if ((MODE32) || (rn != eR15))
				rnv = (GET_REGISTER(rn) + off);
			else
				rnv = (GET_PC + off);
		}
		else
		{
			if ((MODE32) || (rn != eR15))
				rnv = (GET_REGISTER(rn) - off);
			else
				rnv = (GET_PC - off);
		}

		if (insn & INSN_SDT_W)
		{
			rnv_old = GET_REGISTER(rn);
			SET_REGISTER(rn, rnv);

	// check writeback???
		}
		else if (rn == eR15)
		{
			rnv = rnv + 8;
		}
	}
	else
	{
		/* Post-indexed addressing */
		if (rn == eR15)
		{
			if (MODE32)
				rnv = R15 + 8;
			else
				rnv = GET_PC + 8;
		}
		else
		{
			rnv = GET_REGISTER(rn);
		}
	}

	/* Do the transfer */
	rd = (insn & INSN_RD) >> INSN_RD_SHIFT;
	if (insn & INSN_SDT_L)
	{
		/* Load */
		if (insn & INSN_SDT_B)
		{
			UINT32 data = READ8(rnv);
			if (m_pendingAbtD == 0)
			{
				SET_REGISTER(rd, data);
			}
		}
		else
		{
			UINT32 data = READ32(rnv);
			if (m_pendingAbtD == 0)
			{
				if (rd == eR15)
				{
					if (MODE32)
						R15 = data - 4;
					else
						R15 = (R15 & ~0x03FFFFFC) /* N Z C V I F M1 M0 */ | ((data - 4) & 0x03FFFFFC);
				// LDR, PC takes 2S + 2N + 1I (5 total cycles)
					ARM7_ICOUNT -= 2;
				}
				else
				{
					SET_REGISTER(rd, data);
				}
			}
		}
	}
	else
	{
		/* Store */
		if (insn & INSN_SDT_B)
		{
#if ARM7_DEBUG_CORE
				if (rd == eR15)
					LOG(("Wrote R15 in byte mode\n"));
#endif

			WRITE8(rnv, (UINT8) GET_REGISTER(rd) & 0xffu);
		}
		else
		{
#if ARM7_DEBUG_CORE
				if (rd == eR15)
					LOG(("Wrote R15 in 32bit mode\n"));
#endif

			//WRITE32(rnv, rd == eR15 ? R15 + 8 : GET_REGISTER(rd));
			WRITE32(rnv, rd == eR15 ? R15 + 8 + 4 : GET_REGISTER(rd)); // manual says STR rd = PC, +12
		}
		// Store takes only 2 N Cycles, so add + 1
		ARM7_ICOUNT += 1;
	}

	if (m_pendingAbtD != 0)
	{
		if ((insn & INSN_SDT_P) && (insn & INSN_SDT_W))
		{
			SET_REGISTER(rn, rnv_old);
		}
	}
	else
	{
	/* Do post-indexing writeback */
	if (!(insn & INSN_SDT_P)/* && (insn & INSN_SDT_W)*/)
	{
		if (insn & INSN_SDT_U)
		{
			/* Writeback is applied in pipeline, before value is read from mem,
			    so writeback is effectively ignored */
			if (rd == rn) {
				SET_REGISTER(rn, GET_REGISTER(rd));
				// todo: check for offs... ?
			}
			else {
				if ((insn & INSN_SDT_W) != 0)
					LOG(("%08x:  RegisterWritebackIncrement %d %d %d\n", R15, (insn & INSN_SDT_P) != 0, (insn & INSN_SDT_W) != 0, (insn & INSN_SDT_U) != 0));

				SET_REGISTER(rn, (rnv + off));
			}
		}
		else
		{
			/* Writeback is applied in pipeline, before value is read from mem,
			    so writeback is effectively ignored */
			if (rd == rn) {
				SET_REGISTER(rn, GET_REGISTER(rd));
			}
			else {
				SET_REGISTER(rn, (rnv - off));

				if ((insn & INSN_SDT_W) != 0)
					LOG(("%08x:  RegisterWritebackDecrement %d %d %d\n", R15, (insn & INSN_SDT_P) != 0, (insn & INSN_SDT_W) != 0, (insn & INSN_SDT_U) != 0));
			}
		}
	}

	}

//  ARM7_CHECKIRQ

} /* HandleMemSingle */

void arm7_cpu_device::HandleHalfWordDT(UINT32 insn)
{
	UINT32 rn, rnv, off, rd, rnv_old = 0;

	// Immediate or Register Offset?
	if (insn & 0x400000) {               // Bit 22 - 1 = immediate, 0 = register
		// imm. value in high nibble (bits 8-11) and lo nibble (bit 0-3)
		off = (((insn >> 8) & 0x0f) << 4) | (insn & 0x0f);
	}
	else {
		// register
		off = GET_REGISTER(insn & 0x0f);
	}

	/* Calculate Rn, accounting for PC */
	rn = (insn & INSN_RN) >> INSN_RN_SHIFT;

	if (insn & INSN_SDT_P)
	{
		/* Pre-indexed addressing */
		if (insn & INSN_SDT_U)
		{
			rnv = (GET_REGISTER(rn) + off);
		}
		else
		{
			rnv = (GET_REGISTER(rn) - off);
		}

		if (insn & INSN_SDT_W)
		{
			rnv_old = GET_REGISTER(rn);
			SET_REGISTER(rn, rnv);

		// check writeback???
		}
		else if (rn == eR15)
		{
			rnv = (rnv) + 8;
		}
	}
	else
	{
		/* Post-indexed addressing */
		if (rn == eR15)
		{
			rnv = R15 + 8;
		}
		else
		{
			rnv = GET_REGISTER(rn);
		}
	}

	/* Do the transfer */
	rd = (insn & INSN_RD) >> INSN_RD_SHIFT;

	/* Load */
	if (insn & INSN_SDT_L)
	{
		// Signed?
		if (insn & 0x40)
		{
			UINT32 newval;

			// Signed Half Word?
			if (insn & 0x20) {
				UINT16 signbyte, databyte;
				databyte = READ16(rnv) & 0xFFFF;
				signbyte = (databyte & 0x8000) ? 0xffff : 0;
				newval = (UINT32)(signbyte << 16)|databyte;
			}
			// Signed Byte
			else {
				UINT8 databyte;
				UINT32 signbyte;
				databyte = READ8(rnv) & 0xff;
				signbyte = (databyte & 0x80) ? 0xffffff : 0;
				newval = (UINT32)(signbyte << 8)|databyte;
			}

			if (m_pendingAbtD == 0)
			{
			// PC?
			if (rd == eR15)
			{
				R15 = newval + 8;
				// LDR(H,SH,SB) PC takes 2S + 2N + 1I (5 total cycles)
				ARM7_ICOUNT -= 2;

			}
			else
			{
				SET_REGISTER(rd, newval);
				R15 += 4;
			}

			}
			else
			{
				R15 += 4;
			}

		}
		// Unsigned Half Word
		else
		{
			UINT32 newval = READ16(rnv);

			if (m_pendingAbtD == 0)
			{
			if (rd == eR15)
			{
				R15 = newval + 8;
				// extra cycles for LDR(H,SH,SB) PC (5 total cycles)
				ARM7_ICOUNT -= 2;
			}
			else
			{
				SET_REGISTER(rd, newval);
				R15 += 4;
			}

			}
			else
			{
				R15 += 4;
			}

		}


	}
	/* Store or ARMv5+ dword insns */
	else
	{
		if ((insn & 0x60) == 0x40)  // LDRD
	{
		SET_REGISTER(rd, READ32(rnv));
		SET_REGISTER(rd+1, READ32(rnv+4));
				R15 += 4;
	}
		else if ((insn & 0x60) == 0x60) // STRD
	{
		WRITE32(rnv, GET_REGISTER(rd));
		WRITE32(rnv+4, GET_REGISTER(rd+1));
				R15 += 4;
	}
	else
	{
			// WRITE16(rnv, rd == eR15 ? R15 + 8 : GET_REGISTER(rd));
			WRITE16(rnv, rd == eR15 ? R15 + 8 + 4 : GET_REGISTER(rd)); // manual says STR RD=PC, +12 of address

// if R15 is not increased then e.g. "STRH R10, [R15,#$10]" will be executed over and over again
#if 0
			if (rn != eR15)
#endif
				R15 += 4;

			// STRH takes 2 cycles, so we add + 1
			ARM7_ICOUNT += 1;
	}
	}

	if (m_pendingAbtD != 0)
	{
		if ((insn & INSN_SDT_P) && (insn & INSN_SDT_W))
		{
			SET_REGISTER(rn, rnv_old);
		}
	}
	else
	{
	// SJE: No idea if this writeback code works or makes sense here..

	/* Do post-indexing writeback */
	if (!(insn & INSN_SDT_P)/* && (insn & INSN_SDT_W)*/)
	{
		if (insn & INSN_SDT_U)
		{
			/* Writeback is applied in pipeline, before value is read from mem,
			    so writeback is effectively ignored */
			if (rd == rn) {
				SET_REGISTER(rn, GET_REGISTER(rd));
				// todo: check for offs... ?
			}
			else {
				if ((insn & INSN_SDT_W) != 0)
					LOG(("%08x:  RegisterWritebackIncrement %d %d %d\n", R15, (insn & INSN_SDT_P) != 0, (insn & INSN_SDT_W) != 0, (insn & INSN_SDT_U) != 0));

				SET_REGISTER(rn, (rnv + off));
			}
		}
		else
		{
			/* Writeback is applied in pipeline, before value is read from mem,
			    so writeback is effectively ignored */
			if (rd == rn) {
				SET_REGISTER(rn, GET_REGISTER(rd));
			}
			else {
				SET_REGISTER(rn, (rnv - off));

				if ((insn & INSN_SDT_W) != 0)
					LOG(("%08x:  RegisterWritebackDecrement %d %d %d\n", R15, (insn & INSN_SDT_P) != 0, (insn & INSN_SDT_W) != 0, (insn & INSN_SDT_U) != 0));
			}
		}
	}

	}

}

void arm7_cpu_device::HandleSwap(UINT32 insn)
{
	UINT32 rn, rm, rd, tmp;

	rn = GET_REGISTER((insn >> 16) & 0xf);  // reg. w/read address
	rm = GET_REGISTER(insn & 0xf);          // reg. w/write address
	rd = (insn >> 12) & 0xf;                // dest reg

#if ARM7_DEBUG_CORE
	if (rn == 15 || rm == 15 || rd == 15)
		LOG(("%08x: Illegal use of R15 in Swap Instruction\n", R15));
#endif

	// can be byte or word
	if (insn & 0x400000)
	{
		tmp = READ8(rn);
		WRITE8(rn, rm);
		SET_REGISTER(rd, tmp);
	}
	else
	{
		tmp = READ32(rn);
		WRITE32(rn, rm);
		SET_REGISTER(rd, tmp);
	}

	R15 += 4;
	// Instruction takes 1S+2N+1I cycles - so we subtract one more..
	ARM7_ICOUNT -= 1;
}

void arm7_cpu_device::HandlePSRTransfer(UINT32 insn)
{
	int reg = (insn & 0x400000) ? SPSR : eCPSR; // Either CPSR or SPSR
	UINT32 newval, val;
	int oldmode = GET_CPSR & MODE_FLAG;

	// get old value of CPSR/SPSR
	newval = GET_REGISTER(reg);

	// MSR (bit 21 set) - Copy value to CPSR/SPSR
	if ((insn & 0x00200000))
	{
		// Immediate Value?
		if (insn & INSN_I) {
			// Value can be specified for a Right Rotate, 2x the value specified.
			int by = (insn & INSN_OP2_ROTATE) >> INSN_OP2_ROTATE_SHIFT;
			if (by)
				val = ROR(insn & INSN_OP2_IMM, by << 1);
			else
				val = insn & INSN_OP2_IMM;
		}
		// Value from Register
		else
		{
			val = GET_REGISTER(insn & 0x0f);
		}

		// apply field code bits
		if (reg == eCPSR)
		{
			if (oldmode != eARM7_MODE_USER)
			{
				if (insn & 0x00010000)
				{
					newval = (newval & 0xffffff00) | (val & 0x000000ff);
				}
				if (insn & 0x00020000)
				{
					newval = (newval & 0xffff00ff) | (val & 0x0000ff00);
				}
				if (insn & 0x00040000)
				{
					newval = (newval & 0xff00ffff) | (val & 0x00ff0000);
				}
			}

			// status flags can be modified regardless of mode
			if (insn & 0x00080000)
			{
				// TODO for non ARMv5E mask should be 0xf0000000 (ie mask Q bit)
				newval = (newval & 0x00ffffff) | (val & 0xf8000000);
			}
		}
		else    // SPSR has stricter requirements
		{
			if (((GET_CPSR & 0x1f) > 0x10) && ((GET_CPSR & 0x1f) < 0x1f))
			{
				if (insn & 0x00010000)
				{
					newval = (newval & 0xffffff00) | (val & 0xff);
				}
				if (insn & 0x00020000)
				{
					newval = (newval & 0xffff00ff) | (val & 0xff00);
				}
				if (insn & 0x00040000)
				{
					newval = (newval & 0xff00ffff) | (val & 0xff0000);
				}
				if (insn & 0x00080000)
				{
					// TODO for non ARMv5E mask should be 0xf0000000 (ie mask Q bit)
					newval = (newval & 0x00ffffff) | (val & 0xf8000000);
				}
			}
		}

#if 0
		// force valid mode
		newval |= 0x10;
#endif

		// Update the Register
		if (reg == eCPSR)
			SET_CPSR(newval);
		else
			SET_REGISTER(reg, newval);

		// Switch to new mode if changed
		if ((newval & MODE_FLAG) != oldmode)
			SwitchMode(GET_MODE);

	}
	// MRS (bit 21 clear) - Copy CPSR or SPSR to specified Register
	else
	{
		SET_REGISTER((insn >> 12)& 0x0f, GET_REGISTER(reg));
	}
}

void arm7_cpu_device::HandleALU(UINT32 insn)
{
	UINT32 op2, sc = 0, rd, rn, opcode;
	UINT32 by, rdn;

	// Normal Data Processing : 1S
	// Data Processing with register specified shift : 1S + 1I
	// Data Processing with PC written : 2S + 1N
	// Data Processing with register specified shift and PC written : 2S + 1N + 1I

	opcode = (insn & INSN_OPCODE) >> INSN_OPCODE_SHIFT;

	rd = 0;
	rn = 0;

	/* --------------*/
	/* Construct Op2 */
	/* --------------*/

	/* Immediate constant */
	if (insn & INSN_I)
	{
		by = (insn & INSN_OP2_ROTATE) >> INSN_OP2_ROTATE_SHIFT;
		if (by)
		{
			op2 = ROR(insn & INSN_OP2_IMM, by << 1);
			sc = op2 & SIGN_BIT;
		}
		else
		{
			op2 = insn & INSN_OP2;      // SJE: Shouldn't this be INSN_OP2_IMM?
			sc = GET_CPSR & C_MASK;
		}
	}
	/* Op2 = Register Value */
	else
	{
		op2 = decodeShift(insn, (insn & INSN_S) ? &sc : nullptr);

		// LD TODO sc will always be 0 if this applies
		if (!(insn & INSN_S))
			sc = 0;

		// extra cycle (register specified shift)
		ARM7_ICOUNT -= 1;
	}

	// LD TODO this comment is wrong
	/* Calculate Rn to account for pipelining */
	if ((opcode & 0xd) != 0xd) /* No Rn in MOV */
	{
		if ((rn = (insn & INSN_RN) >> INSN_RN_SHIFT) == eR15)
		{
#if ARM7_DEBUG_CORE
			LOG(("%08x:  Pipelined R15 (Shift %d)\n", R15, (insn & INSN_I ? 8 : insn & 0x10u ? 12 : 12)));
#endif
			if (MODE32)
				rn = R15 + 8;
			else
				rn = GET_PC + 8;
		}
		else
		{
			rn = GET_REGISTER(rn);
		}
	}

	/* Perform the operation */

	switch (opcode)
	{
	/* Arithmetic operations */
	case OPCODE_SBC:
		rd = (rn - op2 - (GET_CPSR & C_MASK ? 0 : 1));
		HandleALUSubFlags(rd, rn, op2);
		break;
	case OPCODE_CMP:
	case OPCODE_SUB:
		rd = (rn - op2);
		HandleALUSubFlags(rd, rn, op2);
		break;
	case OPCODE_RSC:
		rd = (op2 - rn - (GET_CPSR & C_MASK ? 0 : 1));
		HandleALUSubFlags(rd, op2, rn);
		break;
	case OPCODE_RSB:
		rd = (op2 - rn);
		HandleALUSubFlags(rd, op2, rn);
		break;
	case OPCODE_ADC:
		rd = (rn + op2 + ((GET_CPSR & C_MASK) >> C_BIT));
		HandleALUAddFlags(rd, rn, op2);
		break;
	case OPCODE_CMN:
	case OPCODE_ADD:
		rd = (rn + op2);
		HandleALUAddFlags(rd, rn, op2);
		break;

	/* Logical operations */
	case OPCODE_AND:
	case OPCODE_TST:
		rd = rn & op2;
		HandleALULogicalFlags(rd, sc);
		break;
	case OPCODE_BIC:
		rd = rn & ~op2;
		HandleALULogicalFlags(rd, sc);
		break;
	case OPCODE_TEQ:
	case OPCODE_EOR:
		rd = rn ^ op2;
		HandleALULogicalFlags(rd, sc);
		break;
	case OPCODE_ORR:
		rd = rn | op2;
		HandleALULogicalFlags(rd, sc);
		break;
	case OPCODE_MOV:
		rd = op2;
		HandleALULogicalFlags(rd, sc);
		break;
	case OPCODE_MVN:
		rd = (~op2);
		HandleALULogicalFlags(rd, sc);
		break;
	}

	/* Put the result in its register if not one of the test only opcodes (TST,TEQ,CMP,CMN) */
	rdn = (insn & INSN_RD) >> INSN_RD_SHIFT;
	if ((opcode & 0xc) != 0x8)
	{
		// If Rd = R15, but S Flag not set, Result is placed in R15, but CPSR is not affected (page 44)
		if (rdn == eR15 && !(insn & INSN_S))
		{
			if (MODE32)
			{
				R15 = rd;
			}
			else
			{
				R15 = (R15 & ~0x03FFFFFC) | (rd & 0x03FFFFFC);
			}
			// extra cycles (PC written)
			ARM7_ICOUNT -= 2;
		}
		else
		{
			// Rd = 15 and S Flag IS set, Result is placed in R15, and current mode SPSR moved to CPSR
			if (rdn == eR15) {
				if (MODE32)
				{
				// When Rd is R15 and the S flag is set the result of the operation is placed in R15 and the SPSR corresponding to
				// the current mode is moved to the CPSR. This allows state changes which automatically restore both PC and
				// CPSR. --> This form of instruction should not be used in User mode. <--

				if (GET_MODE != eARM7_MODE_USER)
				{
					// Update CPSR from SPSR
					SET_CPSR(GET_REGISTER(SPSR));
					SwitchMode(GET_MODE);
				}

				R15 = rd;

				}
				else
				{
					UINT32 temp;
					R15 = rd; //(R15 & 0x03FFFFFC) | (rd & 0xFC000003);
					temp = (GET_CPSR & 0x0FFFFF20) | (rd & 0xF0000000) /* N Z C V */ | ((rd & 0x0C000000) >> (26 - 6)) /* I F */ | (rd & 0x00000003) /* M1 M0 */;
					SET_CPSR( temp);
					SwitchMode( temp & 3);
				}

				// extra cycles (PC written)
				ARM7_ICOUNT -= 2;

				/* IRQ masks may have changed in this instruction */
//              ARM7_CHECKIRQ;
			}
			else
				/* S Flag is set - Write results to register & update CPSR (which was already handled using HandleALU flag macros) */
				SET_REGISTER(rdn, rd);
		}
	}
	// SJE: Don't think this applies any more.. (see page 44 at bottom)
	/* TST & TEQ can affect R15 (the condition code register) with the S bit set */
	else if (rdn == eR15)
	{
		if (insn & INSN_S) {
#if ARM7_DEBUG_CORE
				LOG(("%08x: TST class on R15 s bit set\n", R15));
#endif
			if (MODE32)
				R15 = rd;
			else
			{
				UINT32 temp;
				R15 = (R15 & 0x03FFFFFC) | (rd & ~0x03FFFFFC);
				temp = (GET_CPSR & 0x0FFFFF20) | (rd & 0xF0000000) /* N Z C V */ | ((rd & 0x0C000000) >> (26 - 6)) /* I F */ | (rd & 0x00000003) /* M1 M0 */;
				SET_CPSR( temp);
				SwitchMode( temp & 3);
			}

			/* IRQ masks may have changed in this instruction */
//          ARM7_CHECKIRQ;
		}
		else
		{
#if ARM7_DEBUG_CORE
				LOG(("%08x: TST class on R15 no s bit set\n", R15));
#endif
		}
		// extra cycles (PC written)
		ARM7_ICOUNT -= 2;
	}

	// compensate for the -3 at the end
	ARM7_ICOUNT += 2;
}

void arm7_cpu_device::HandleMul(UINT32 insn)
{
	UINT32 r, rm, rs;

	// MUL takes 1S + mI and MLA 1S + (m+1)I cycles to execute, where S and I are as
	// defined in 6.2 Cycle Types on page 6-2.
	// m is the number of 8 bit multiplier array cycles required to complete the
	// multiply, which is controlled by the value of the multiplier operand
	// specified by Rs.

	rm = GET_REGISTER(insn & INSN_MUL_RM);
	rs = GET_REGISTER((insn & INSN_MUL_RS) >> INSN_MUL_RS_SHIFT);

	/* Do the basic multiply of Rm and Rs */
	r = rm * rs;

#if ARM7_DEBUG_CORE
	if ((insn & INSN_MUL_RM) == 0xf ||
		((insn & INSN_MUL_RS) >> INSN_MUL_RS_SHIFT) == 0xf ||
		((insn & INSN_MUL_RN) >> INSN_MUL_RN_SHIFT) == 0xf)
		LOG(("%08x:  R15 used in mult\n", R15));
#endif

	/* Add on Rn if this is a MLA */
	if (insn & INSN_MUL_A)
	{
		r += GET_REGISTER((insn & INSN_MUL_RN) >> INSN_MUL_RN_SHIFT);
		// extra cycle for MLA
		ARM7_ICOUNT -= 1;
	}

	/* Write the result */
	SET_REGISTER((insn & INSN_MUL_RD) >> INSN_MUL_RD_SHIFT, r);

	/* Set N and Z if asked */
	if (insn & INSN_S)
	{
		SET_CPSR((GET_CPSR & ~(N_MASK | Z_MASK)) | HandleALUNZFlags(r));
	}

	if (rs & SIGN_BIT) rs = -rs;
	if (rs < 0x00000100) ARM7_ICOUNT -= 1 + 1;
	else if (rs < 0x00010000) ARM7_ICOUNT -= 1 + 2;
	else if (rs < 0x01000000) ARM7_ICOUNT -= 1 + 3;
	else ARM7_ICOUNT -= 1 + 4;

	ARM7_ICOUNT += 3;
}

// todo: add proper cycle counts
void arm7_cpu_device::HandleSMulLong(UINT32 insn)
{
	INT32 rm, rs;
	UINT32 rhi, rlo;
	INT64 res;

	// MULL takes 1S + (m+1)I and MLAL 1S + (m+2)I cycles to execute, where m is the
	// number of 8 bit multiplier array cycles required to complete the multiply, which is
	// controlled by the value of the multiplier operand specified by Rs.

	rm  = (INT32)GET_REGISTER(insn & 0xf);
	rs  = (INT32)GET_REGISTER(((insn >> 8) & 0xf));
	rhi = (insn >> 16) & 0xf;
	rlo = (insn >> 12) & 0xf;

#if ARM7_DEBUG_CORE
		if ((insn & 0xf) == 15 || ((insn >> 8) & 0xf) == 15 || ((insn >> 16) & 0xf) == 15 || ((insn >> 12) & 0xf) == 15)
			LOG(("%08x: Illegal use of PC as a register in SMULL opcode\n", R15));
#endif

	/* Perform the multiplication */
	res = (INT64)rm * rs;

	/* Add on Rn if this is a MLA */
	if (insn & INSN_MUL_A)
	{
		INT64 acum = (INT64)((((INT64)(GET_REGISTER(rhi))) << 32) | GET_REGISTER(rlo));
		res += acum;
		// extra cycle for MLA
		ARM7_ICOUNT -= 1;
	}

	/* Write the result (upper dword goes to RHi, lower to RLo) */
	SET_REGISTER(rhi, res >> 32);
	SET_REGISTER(rlo, res & 0xFFFFFFFF);

	/* Set N and Z if asked */
	if (insn & INSN_S)
	{
		SET_CPSR((GET_CPSR & ~(N_MASK | Z_MASK)) | HandleLongALUNZFlags(res));
	}

	if (rs < 0) rs = -rs;
	if (rs < 0x00000100) ARM7_ICOUNT -= 1 + 1 + 1;
	else if (rs < 0x00010000) ARM7_ICOUNT -= 1 + 2 + 1;
	else if (rs < 0x01000000) ARM7_ICOUNT -= 1 + 3 + 1;
	else ARM7_ICOUNT -= 1 + 4 + 1;

	ARM7_ICOUNT += 3;
}

// todo: add proper cycle counts
void arm7_cpu_device::HandleUMulLong(UINT32 insn)
{
	UINT32 rm, rs;
	UINT32 rhi, rlo;
	UINT64 res;

	// MULL takes 1S + (m+1)I and MLAL 1S + (m+2)I cycles to execute, where m is the
	// number of 8 bit multiplier array cycles required to complete the multiply, which is
	// controlled by the value of the multiplier operand specified by Rs.

	rm  = (INT32)GET_REGISTER(insn & 0xf);
	rs  = (INT32)GET_REGISTER(((insn >> 8) & 0xf));
	rhi = (insn >> 16) & 0xf;
	rlo = (insn >> 12) & 0xf;

#if ARM7_DEBUG_CORE
		if (((insn & 0xf) == 15) || (((insn >> 8) & 0xf) == 15) || (((insn >> 16) & 0xf) == 15) || (((insn >> 12) & 0xf) == 15))
			LOG(("%08x: Illegal use of PC as a register in SMULL opcode\n", R15));
#endif

	/* Perform the multiplication */
	res = (UINT64)rm * rs;

	/* Add on Rn if this is a MLA */
	if (insn & INSN_MUL_A)
	{
		UINT64 acum = (UINT64)((((UINT64)(GET_REGISTER(rhi))) << 32) | GET_REGISTER(rlo));
		res += acum;
		// extra cycle for MLA
		ARM7_ICOUNT -= 1;
	}

	/* Write the result (upper dword goes to RHi, lower to RLo) */
	SET_REGISTER(rhi, res >> 32);
	SET_REGISTER(rlo, res & 0xFFFFFFFF);

	/* Set N and Z if asked */
	if (insn & INSN_S)
	{
		SET_CPSR((GET_CPSR & ~(N_MASK | Z_MASK)) | HandleLongALUNZFlags(res));
	}

	if (rs < 0x00000100) ARM7_ICOUNT -= 1 + 1 + 1;
	else if (rs < 0x00010000) ARM7_ICOUNT -= 1 + 2 + 1;
	else if (rs < 0x01000000) ARM7_ICOUNT -= 1 + 3 + 1;
	else ARM7_ICOUNT -= 1 + 4 + 1;

	ARM7_ICOUNT += 3;
}

void arm7_cpu_device::HandleMemBlock(UINT32 insn)
{
	UINT32 rb = (insn & INSN_RN) >> INSN_RN_SHIFT;
	UINT32 rbp = GET_REGISTER(rb);
	int result;

#if ARM7_DEBUG_CORE
	if (rbp & 3)
		LOG(("%08x: Unaligned Mem Transfer @ %08x\n", R15, rbp));
#endif

	// Normal LDM instructions take nS + 1N + 1I and LDM PC takes (n+1)S + 2N + 1I
	// incremental cycles, where S,N and I are as defined in 6.2 Cycle Types on page 6-2.
	// STM instructions take (n-1)S + 2N incremental cycles to execute, where n is the
	// number of words transferred.

	if (insn & INSN_BDT_L)
	{
		/* Loading */
		if (insn & INSN_BDT_U)
		{
			/* Incrementing */
			if (!(insn & INSN_BDT_P))
			{
				rbp = rbp + (- 4);
			}

			// S Flag Set, but R15 not in list = User Bank Transfer
			if (insn & INSN_BDT_S && (insn & 0x8000) == 0)
			{
				// !! actually switching to user mode triggers a section permission fault in Happy Fish 302-in-1 (BP C0030DF4, press F5 ~16 times) !!
				// set to user mode - then do the transfer, and set back
				//int curmode = GET_MODE;
				//SwitchMode(eARM7_MODE_USER);
				LOG(("%08x: User Bank Transfer not fully tested - please check if working properly!\n", R15));
				result = loadInc(insn & 0xffff, rbp, insn & INSN_BDT_S, eARM7_MODE_USER);
				// todo - not sure if Writeback occurs on User registers also..
				//SwitchMode(curmode);
			}
			else
				result = loadInc(insn & 0xffff, rbp, insn & INSN_BDT_S, GET_MODE);

			if ((insn & INSN_BDT_W) && (m_pendingAbtD == 0))
			{
#if ARM7_DEBUG_CORE
					if (rb == 15)
						LOG(("%08x:  Illegal LDRM writeback to r15\n", R15));
#endif
				// "A LDM will always overwrite the updated base if the base is in the list." (also for a user bank transfer?)
				// GBA "V-Rally 3" expects R0 not to be overwritten with the updated base value [BP 8077B0C]
				if (((insn >> rb) & 1) == 0)
				{
					SET_REGISTER(rb, GET_REGISTER(rb) + result * 4);
				}
			}

			// R15 included? (NOTE: CPSR restore must occur LAST otherwise wrong registers restored!)
			if ((insn & 0x8000) && (m_pendingAbtD == 0)) {
				R15 -= 4;     // SJE: I forget why i did this?
				// S - Flag Set? Signals transfer of current mode SPSR->CPSR
				if (insn & INSN_BDT_S)
				{
					if (MODE32)
					{
						SET_CPSR(GET_REGISTER(SPSR));
						SwitchMode(GET_MODE);
					}
					else
					{
						UINT32 temp;
//                      LOG(("LDM + S | R15 %08X CPSR %08X\n", R15, GET_CPSR));
						temp = (GET_CPSR & 0x0FFFFF20) | (R15 & 0xF0000000) /* N Z C V */ | ((R15 & 0x0C000000) >> (26 - 6)) /* I F */ | (R15 & 0x00000003) /* M1 M0 */;
						SET_CPSR( temp);
						SwitchMode(temp & 3);
					}
				}
				// LDM PC - takes 2 extra cycles
				ARM7_ICOUNT -= 2;
			}
		}
		else
		{
			/* Decrementing */
			if (!(insn & INSN_BDT_P))
			{
				rbp = rbp - (- 4);
			}

			// S Flag Set, but R15 not in list = User Bank Transfer
			if (insn & INSN_BDT_S && ((insn & 0x8000) == 0))
			{
				// set to user mode - then do the transfer, and set back
				//int curmode = GET_MODE;
				//SwitchMode(eARM7_MODE_USER);
				LOG(("%08x: User Bank Transfer not fully tested - please check if working properly!\n", R15));
				result = loadDec(insn & 0xffff, rbp, insn & INSN_BDT_S, eARM7_MODE_USER);
				// todo - not sure if Writeback occurs on User registers also..
				//SwitchMode(curmode);
			}
			else
				result = loadDec(insn & 0xffff, rbp, insn & INSN_BDT_S, GET_MODE);

			if ((insn & INSN_BDT_W) && (m_pendingAbtD == 0))
			{
				if (rb == 0xf)
					LOG(("%08x:  Illegal LDRM writeback to r15\n", R15));
				// "A LDM will always overwrite the updated base if the base is in the list." (also for a user bank transfer?)
				if (((insn >> rb) & 1) == 0)
				{
					SET_REGISTER(rb, GET_REGISTER(rb) - result * 4);
				}
			}

			// R15 included? (NOTE: CPSR restore must occur LAST otherwise wrong registers restored!)
			if ((insn & 0x8000) && (m_pendingAbtD == 0)) {
				R15 -= 4;     // SJE: I forget why i did this?
				// S - Flag Set? Signals transfer of current mode SPSR->CPSR
				if (insn & INSN_BDT_S)
				{
					if (MODE32)
					{
						SET_CPSR(GET_REGISTER(SPSR));
						SwitchMode(GET_MODE);
					}
					else
					{
						UINT32 temp;
//                      LOG(("LDM + S | R15 %08X CPSR %08X\n", R15, GET_CPSR));
						temp = (GET_CPSR & 0x0FFFFF20) /* N Z C V I F M4 M3 M2 M1 M0 */ | (R15 & 0xF0000000) /* N Z C V */ | ((R15 & 0x0C000000) >> (26 - 6)) /* I F */ | (R15 & 0x00000003) /* M1 M0 */;
						SET_CPSR( temp);
						SwitchMode(temp & 3);
					}
				}
				// LDM PC - takes 2 extra cycles
				ARM7_ICOUNT -= 2;
			}
		}
		// LDM (NO PC) takes (n)S + 1N + 1I cycles (n = # of register transfers)
		ARM7_ICOUNT -= result + 1 + 1;
	} /* Loading */
	else
	{
		/* Storing - STM */
		if (insn & (1 << eR15))
		{
#if ARM7_DEBUG_CORE
				LOG(("%08x: Writing R15 in strm\n", R15));
#endif
			/* special case handling if writing to PC */
			R15 += 12;
		}
		if (insn & INSN_BDT_U)
		{
			/* Incrementing */
			if (!(insn & INSN_BDT_P))
			{
				rbp = rbp + (- 4);
			}

			// S Flag Set = User Bank Transfer
			if (insn & INSN_BDT_S)
			{
				// todo: needs to be tested..

				// set to user mode - then do the transfer, and set back
				//int curmode = GET_MODE;
				//SwitchMode(eARM7_MODE_USER);
				LOG(("%08x: User Bank Transfer not fully tested - please check if working properly!\n", R15));
				result = storeInc(insn & 0xffff, rbp, eARM7_MODE_USER);
				// todo - not sure if Writeback occurs on User registers also..
				//SwitchMode(curmode);
			}
			else
				result = storeInc(insn & 0xffff, rbp, GET_MODE);

			if ((insn & INSN_BDT_W) && (m_pendingAbtD == 0))
			{
				SET_REGISTER(rb, GET_REGISTER(rb) + result * 4);
			}
		}
		else
		{
			/* Decrementing - but real CPU writes in incrementing order */
			if (!(insn & INSN_BDT_P))
			{
				rbp = rbp - (-4);
			}

			// S Flag Set = User Bank Transfer
			if (insn & INSN_BDT_S)
			{
				// set to user mode - then do the transfer, and set back
				//int curmode = GET_MODE;
				//SwitchMode(eARM7_MODE_USER);
				LOG(("%08x: User Bank Transfer not fully tested - please check if working properly!\n", R15));
				result = storeDec(insn & 0xffff, rbp, eARM7_MODE_USER);
				// todo - not sure if Writeback occurs on User registers also..
				//SwitchMode(curmode);
			}
			else
				result = storeDec(insn & 0xffff, rbp, GET_MODE);

			if ((insn & INSN_BDT_W) && (m_pendingAbtD == 0))
			{
				SET_REGISTER(rb, GET_REGISTER(rb) - result * 4);
			}
		}
		if (insn & (1 << eR15))
			R15 -= 12;

		// STM takes (n-1)S + 2N cycles (n = # of register transfers)
		ARM7_ICOUNT -= (result - 1) + 2;
	}

	// We will specify the cycle count for each case, so remove the -3 that occurs at the end
	ARM7_ICOUNT += 3;

} /* HandleMemBlock */


const arm7_cpu_device::arm7ops_ophandler arm7_cpu_device::ops_handler[0x10] =
{
	&arm7_cpu_device::arm7ops_0123, &arm7_cpu_device::arm7ops_0123, &arm7_cpu_device::arm7ops_0123, &arm7_cpu_device::arm7ops_0123,
	&arm7_cpu_device::arm7ops_4567, &arm7_cpu_device::arm7ops_4567, &arm7_cpu_device::arm7ops_4567, &arm7_cpu_device::arm7ops_4567,
	&arm7_cpu_device::arm7ops_89,   &arm7_cpu_device::arm7ops_89,   &arm7_cpu_device::arm7ops_ab,   &arm7_cpu_device::arm7ops_ab,
	&arm7_cpu_device::arm7ops_cd,   &arm7_cpu_device::arm7ops_cd,   &arm7_cpu_device::arm7ops_e,    &arm7_cpu_device::arm7ops_f,
};

void arm7_cpu_device::arm7ops_0123(UINT32 insn)
{
//case 0:
//case 1:
//case 2:
//case 3:
	/* Branch and Exchange (BX) */
	if ((insn & 0x0ffffff0) == 0x012fff10)     // bits 27-4 == 000100101111111111110001
	{
		R15 = GET_REGISTER(insn & 0x0f);
		// If new PC address has A0 set, switch to Thumb mode
		if (R15 & 1) {
			SET_CPSR(GET_CPSR|T_MASK);
			R15--;
		}
	}
	else if ((insn & 0x0ff000f0) == 0x01600010) // CLZ - v5
	{
		UINT32 rm = insn&0xf;
		UINT32 rd = (insn>>12)&0xf;

		SET_REGISTER(rd, count_leading_zeros(GET_REGISTER(rm)));

		R15 += 4;
	}
	else if ((insn & 0x0ff000f0) == 0x01000050) // QADD - v5
	{
		INT32 src1 = GET_REGISTER(insn&0xf);
		INT32 src2 = GET_REGISTER((insn>>16)&0xf);
		INT64 res;

		res = saturate_qbit_overflow((INT64)src1 + (INT64)src2);

		SET_REGISTER((insn>>12)&0xf, (INT32)res);
		R15 += 4;
	}
	else if ((insn & 0x0ff000f0) == 0x01400050) // QDADD - v5
	{
		INT32 src1 = GET_REGISTER(insn&0xf);
		INT32 src2 = GET_REGISTER((insn>>16)&0xf);
		INT64 res;

		// check if doubling operation will overflow
		res = (INT64)src2 * 2;
		saturate_qbit_overflow(res);

		src2 *= 2;
		res = saturate_qbit_overflow((INT64)src1 + (INT64)src2);

		SET_REGISTER((insn>>12)&0xf, (INT32)res);
		R15 += 4;
	}
	else if ((insn & 0x0ff000f0) == 0x01200050) // QSUB - v5
	{
		INT32 src1 = GET_REGISTER(insn&0xf);
		INT32 src2 = GET_REGISTER((insn>>16)&0xf);
		INT64 res;

		res = saturate_qbit_overflow((INT64)src1 - (INT64)src2);

		SET_REGISTER((insn>>12)&0xf, (INT32)res);
		R15 += 4;
	}
	else if ((insn & 0x0ff000f0) == 0x01600050) // QDSUB - v5
	{
		INT32 src1 = GET_REGISTER(insn&0xf);
		INT32 src2 = GET_REGISTER((insn>>16)&0xf);
		INT64 res;

		// check if doubling operation will overflow
		res = (INT64)src2 * 2;
		saturate_qbit_overflow(res);

		src2 *= 2;
		res = saturate_qbit_overflow((INT64)src1 - (INT64)src2);

		SET_REGISTER((insn>>12)&0xf, (INT32)res);
		R15 += 4;
	}
	else if ((insn & 0x0ff00090) == 0x01000080) // SMLAxy - v5
	{
		INT32 src1 = GET_REGISTER(insn&0xf);
		INT32 src2 = GET_REGISTER((insn>>8)&0xf);
		INT32 res1;

		// select top and bottom halves of src1/src2 and sign extend if necessary
		if (insn & 0x20)
		{
			src1 >>= 16;
		}

		src1 &= 0xffff;
		if (src1 & 0x8000)
		{
			src1 |= 0xffff0000;
		}

		if (insn & 0x40)
		{
			src2 >>= 16;
		}

		src2 &= 0xffff;
		if (src2 & 0x8000)
		{
			src2 |= 0xffff0000;
		}

		// do the signed multiply
		res1 = src1 * src2;
		// and the accumulate.  NOTE: only the accumulate can cause an overflow, which is why we do it this way.
		saturate_qbit_overflow((INT64)res1 + (INT64)GET_REGISTER((insn>>12)&0xf));

		SET_REGISTER((insn>>16)&0xf, res1 + GET_REGISTER((insn>>12)&0xf));
		R15 += 4;
	}
	else if ((insn & 0x0ff00090) == 0x01400080) // SMLALxy - v5
	{
		INT32 src1 = GET_REGISTER(insn&0xf);
		INT32 src2 = GET_REGISTER((insn>>8)&0xf);
		INT64 dst;

		dst = (INT64)GET_REGISTER((insn>>12)&0xf);
		dst |= (INT64)GET_REGISTER((insn>>16)&0xf)<<32;

		// do the multiply and accumulate
		dst += (INT64)src1 * (INT64)src2;

		// write back the result
		SET_REGISTER((insn>>12)&0xf, (UINT32)dst);
		SET_REGISTER((insn>>16)&0xf, (UINT32)(dst >> 32));
		R15 += 4;
	}
	else if ((insn & 0x0ff00090) == 0x01600080) // SMULxy - v5
	{
		INT32 src1 = GET_REGISTER(insn&0xf);
		INT32 src2 = GET_REGISTER((insn>>8)&0xf);
		INT32 res;

		// select top and bottom halves of src1/src2 and sign extend if necessary
		if (insn & 0x20)
		{
			src1 >>= 16;
		}

		src1 &= 0xffff;
		if (src1 & 0x8000)
		{
			src1 |= 0xffff0000;
		}

		if (insn & 0x40)
		{
			src2 >>= 16;
		}

		src2 &= 0xffff;
		if (src2 & 0x8000)
		{
			src2 |= 0xffff0000;
		}

		res = src1 * src2;
		SET_REGISTER((insn>>16)&0xf, res);
		R15 += 4;
	}
	else if ((insn & 0x0ff000b0) == 0x012000a0) // SMULWy - v5
	{
		INT32 src1 = GET_REGISTER(insn&0xf);
		INT32 src2 = GET_REGISTER((insn>>8)&0xf);
		INT64 res;

		if (insn & 0x40)
		{
			src2 >>= 16;
		}

		src2 &= 0xffff;
		if (src2 & 0x8000)
		{
			src2 |= 0xffff0000;
		}

		res = (INT64)src1 * (INT64)src2;
		res >>= 16;
		SET_REGISTER((insn>>16)&0xf, (UINT32)res);
		R15 += 4;
	}
	else if ((insn & 0x0ff000b0) == 0x01200080) // SMLAWy - v5
	{
		INT32 src1 = GET_REGISTER(insn&0xf);
		INT32 src2 = GET_REGISTER((insn>>8)&0xf);
		INT32 src3 = GET_REGISTER((insn>>12)&0xf);
		INT64 res;

		if (insn & 0x40)
		{
			src2 >>= 16;
		}

		src2 &= 0xffff;
		if (src2 & 0x8000)
		{
			src2 |= 0xffff0000;
		}

		res = (INT64)src1 * (INT64)src2;
		res >>= 16;

		// check for overflow and set the Q bit
		saturate_qbit_overflow((INT64)src3 + res);

		// do the real accumulate
		src3 += (INT32)res;

		// write the result back
		SET_REGISTER((insn>>16)&0xf, (UINT32)res);
		R15 += 4;
	}
	else
	/* Multiply OR Swap OR Half Word Data Transfer */
	if ((insn & 0x0e000000) == 0 && (insn & 0x80) && (insn & 0x10))  // bits 27-25=000 bit 7=1 bit 4=1
	{
		/* Half Word Data Transfer */
		if (insn & 0x60)         // bits = 6-5 != 00
		{
			HandleHalfWordDT(insn);
		}
		else
		/* Swap */
		if (insn & 0x01000000)   // bit 24 = 1
		{
			HandleSwap(insn);
		}
		/* Multiply Or Multiply Long */
		else
		{
			/* multiply long */
			if (insn & 0x800000) // Bit 23 = 1 for Multiply Long
			{
				/* Signed? */
				if (insn & 0x00400000)
					HandleSMulLong(insn);
				else
					HandleUMulLong(insn);
			}
			/* multiply */
			else
			{
				HandleMul(insn);
			}
			R15 += 4;
		}
	}
	/* Data Processing OR PSR Transfer */
	else if ((insn & 0x0c000000) == 0)   // bits 27-26 == 00 - This check can only exist properly after Multiplication check above
	{
		/* PSR Transfer (MRS & MSR) */
		if (((insn & 0x00100000) == 0) && ((insn & 0x01800000) == 0x01000000)) // S bit must be clear, and bit 24,23 = 10
		{
			HandlePSRTransfer(insn);
			ARM7_ICOUNT += 2;       // PSR only takes 1 - S Cycle, so we add + 2, since at end, we -3..
			R15 += 4;
		}
		/* Data Processing */
		else
		{
			HandleALU(insn);
		}
	}
//  break;
}

void arm7_cpu_device::arm7ops_4567(UINT32 insn) /* Data Transfer - Single Data Access */
{
//case 4:
//case 5:
//case 6:
//case 7:
	HandleMemSingle(insn);
	R15 += 4;
//  break;
}

void arm7_cpu_device::arm7ops_89(UINT32 insn) /* Block Data Transfer/Access */
{
//case 8:
//case 9:
	HandleMemBlock(insn);
	R15 += 4;
//  break;
}

void arm7_cpu_device::arm7ops_ab(UINT32 insn) /* Branch or Branch & Link */
{
//case 0xa:
//case 0xb:
	HandleBranch(insn);
//  break;
}

void arm7_cpu_device::arm7ops_cd(UINT32 insn) /* Co-Processor Data Transfer */
{
//case 0xc:
//case 0xd:
	HandleCoProcDT(insn);
	R15 += 4;
//  break;
}

void arm7_cpu_device::arm7ops_e(UINT32 insn) /* Co-Processor Data Operation or Register Transfer */
{
//case 0xe:
	if (insn & 0x10)
		HandleCoProcRT(insn);
	else
		HandleCoProcDO(insn);
	R15 += 4;
//  break;
}

void arm7_cpu_device::arm7ops_f(UINT32 insn) /* Software Interrupt */
{
	m_pendingSwi = 1;
	ARM7_CHECKIRQ;
	//couldn't find any cycle counts for SWI
//  break;
}
