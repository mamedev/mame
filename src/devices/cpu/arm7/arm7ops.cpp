// license:BSD-3-Clause
// copyright-holders:Steve Ellenoff,R. Belmont,Ryan Holtz
#include "emu.h"
#include "arm7.h"
#include "arm7core.h"
#include "arm7help.h"

#include <bit>

#define LOG_OPS     (1U << 1)

#define VERBOSE     (0)
#include "logmacro.h"

int64_t arm7_cpu_device::saturate_qbit_overflow(int64_t res)
{
	if (res > 2147483647)   // INT32_MAX
	{   // overflow high? saturate and set Q
		res = 2147483647;
		set_cpsr(GET_CPSR | Q_MASK);
	}
	else if (res < (-2147483647-1)) // INT32_MIN
	{   // overflow low? saturate and set Q
		res = (-2147483647-1);
		set_cpsr(GET_CPSR | Q_MASK);
	}

	return res;
}


void arm7_cpu_device::SwitchMode(uint32_t cpsr_mode_val)
{
	uint32_t cspr = m_r[eCPSR] & ~MODE_FLAG;
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

// 26-bit modes: write the PSR bits of a value into R15 and the CPSR.  Used by the S-bit form of a data-processing
// instruction with Rd = R15 (MOVS pc, r14 etc.), the P form of the compares (TSTP/TEQP/CMPP/CMNP, which write only the
// PSR) and LDM ^ with R15 in the list.  In user mode the I, F and mode bits are protected and only N, Z, C and V are
// taken from the value (ARM2/ARM3 datasheets, "PSR protection"; the ARM6/ARM7 26-bit modes behave the same way).
void arm7_cpu_device::write_r15_psr26(uint32_t value, bool write_pc)
{
	uint32_t psr = value & 0xFC000003;
	if (GET_MODE == eARM7_MODE_USER)
		psr = (psr & 0xF0000000) | ((GET_CPSR & (I_MASK | F_MASK)) << (26 - 6)) | (GET_CPSR & 3);   // the CPSR, not R15: LDM has already loaded R15
	R15 = ((write_pc ? value : R15) & 0x03FFFFFC) | psr;
	uint32_t const cpsr = (GET_CPSR & 0x0FFFFF20) | (psr & 0xF0000000) /* N Z C V */ | ((psr & 0x0C000000) >> (26 - 6)) /* I F */ | (psr & 0x00000003) /* M1 M0 */;
	set_cpsr(cpsr);
	SwitchMode(cpsr & 3);
}

// LDM with writeback and the base register in the list: the loaded value wins ("A LDM will always overwrite the
// updated base if the base is in the list").  Not so for a user bank transfer (LDM ^ without R15) from a privileged
// mode when the base is one of the banked registers: the list loads the USER copy, the base is a different physical
// register and is written back as usual.  RISC OS's FPEmulator returns with LDMFD R13!,{R0-R14}^ / LDMFD R13!,{R15}^
// from SVC mode and needs the second load to come from the updated stack pointer.
bool arm7_cpu_device::ldm_loads_base(uint32_t insn, uint32_t rb) const
{
	if (!BIT(insn, rb))
		return false;
	if ((insn & INSN_BDT_S) && !(insn & 0x8000) && GET_MODE != eARM7_MODE_USER && GET_MODE != eARM7_MODE_SYS)
		return (GET_MODE == eARM7_MODE_FIQ) ? (rb < 8) : (rb < 13);
	return true;
}

uint32_t arm7_cpu_device::decodeShift(uint32_t insn, uint32_t *pCarry)
{
	uint32_t k  = (insn & INSN_OP2_SHIFT) >> INSN_OP2_SHIFT_SHIFT;  // Bits 11-7
	uint32_t rm = GetRegister(insn & INSN_OP2_RM);
	uint32_t t  = (insn & INSN_OP2_SHIFT_TYPE) >> INSN_OP2_SHIFT_TYPE_SHIFT;

	if ((insn & INSN_OP2_RM) == 0xf) {
		// "If a register is used to specify the shift amount the PC will be 12 bytes ahead." (instead of 8)
		rm += t & 1 ? 12 : 8;
	}

	/* All shift types ending in 1 are Rk, not #k */
	if (t & 1)
	{
//      LOGMASKED(LOG_OPS, "%08x:  RegShift %02x %02x\n", R15, k >> 1, GetRegister(k >> 1));
#if ARM7_DEBUG_CORE
			if ((insn & 0x80) == 0x80)
				LOGMASKED(LOG_OPS, "%08x:  RegShift ERROR (p36)\n", R15);
#endif
		// Keep only the bottom 8 bits for a Register Shift
		k = GetRegister(k >> 1) & 0xff;

		if (k == 0) /* Register shift by 0 is a no-op */
		{
//          LOGMASKED(LOG_OPS, "%08x:  NO-OP Regshift\n", R15);
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
			return k ? (rm << k) : rm;
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
			return rm >> k;
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
				return (rm >> k) | (0xffffffffu << (32 - k));
			else
				return rm >> k;
		}

	case 3:                     /* ROR and RRX */
		if (k)
		{
			k &= 31;
			if (k)
			{
				if (pCarry)
					*pCarry = rm & (1 << (k - 1));
				return std::rotr(rm, k);
			}
			else
			{
				if (pCarry)
					*pCarry = rm & SIGN_BIT;
				return rm;
			}
		}
		else
		{
			/* RRX */
			if (pCarry)
				*pCarry = (rm & 1);
			return (rm >> 1) | ((GET_CPSR & C_MASK) << 2);
		}
	}

	LOGMASKED(LOG_OPS, "%08x: Decodeshift error\n", R15);
	return 0;
} /* decodeShift */


int arm7_cpu_device::loadInc(uint32_t pat, uint32_t rbv, uint32_t s, int mode)
{
	int i, result;
	uint32_t data;

	result = 0;
	rbv &= ~3;
	for (i = 0; i < 16; i++)
	{
		if ((pat >> i) & 1)
		{
			data = READ32(rbv += 4);
			if (m_pendingAbtD) // "Overwriting of registers stops when the abort happens."
				return result;
			if (i == 15)
			{
				if (s) /* Pull full contents from stack */
					SetModeRegister(mode, 15, data);
				else if (MODE32) /* Pull only address, preserve mode & status flags */
					SetModeRegister(mode, 15, data);
				else
				{
					SetModeRegister(mode, 15, (GetModeRegister(mode, 15) & ~0x03FFFFFC) | (data & 0x03FFFFFC));
				}
			}
			else
			{
				SetModeRegister(mode, i, data);
			}
			result++;
		}
	}
	return result;
}


int arm7_cpu_device::loadDec(uint32_t pat, uint32_t rbv, uint32_t s, int mode)
{
	int i, result;
	uint32_t data;

	result = 0;
	rbv &= ~3;
	for (i = 15; i >= 0; i--)
	{
		if ((pat >> i) & 1)
		{
			data = READ32(rbv -= 4);
			if (m_pendingAbtD) // "Overwriting of registers stops when the abort happens."
				return result;
			if (i == 15)
			{
				if (s) /* Pull full contents from stack */
					SetModeRegister(mode, 15, data);
				else if (MODE32) /* Pull only address, preserve mode & status flags */
					SetModeRegister(mode, 15, data);
				else
				{
					SetModeRegister(mode, 15, (GetModeRegister(mode, 15) & ~0x03FFFFFC) | (data & 0x03FFFFFC));
				}
			}
			else
			{
				SetModeRegister(mode, i, data);
			}
			result++;
		}
	}
	return result;
}


int arm7_cpu_device::storeInc(uint32_t pat, uint32_t rbv, int mode)
{
	int i, result;

	result = 0;
	for (i = 0; i < 16; i++)
	{
		if ((pat >> i) & 1)
		{
#if ARM7_DEBUG_CORE
			if (i == 15) /* R15 is plus 12 from address of STM */
				LOGMASKED(LOG_OPS, "%08x: StoreInc on R15\n", R15);
#endif
			WRITE32(rbv += 4, GetModeRegister(mode, i));
			if (m_pendingAbtD)
				return result;
			result++;
		}
	}
	return result;
} /* storeInc */


int arm7_cpu_device::storeDec(uint32_t pat, uint32_t rbv, int mode)
{
	// pre-count the # of registers being stored
	int const result = std::popcount(pat & 0x0000ffff);
	int actual_result = 0;

	// adjust starting address
	rbv -= (result << 2);

	for (int i = 0; i <= 15; i++)
	{
		if ((pat >> i) & 1)
		{
#if ARM7_DEBUG_CORE
			if (i == 15) /* R15 is plus 12 from address of STM */
				LOGMASKED(LOG_OPS, "%08x: StoreDec on R15\n", R15);
#endif
			WRITE32(rbv, GetModeRegister(mode, i));
			if (m_pendingAbtD)
				return actual_result;
			rbv += 4;
			actual_result++;
		}
	}
	return result;
} /* storeDec */


/***************************************************************************
 *                            OPCODE HANDLING
 ***************************************************************************/

// Co-Processor Data Operation
void arm7_cpu_device::HandleCoProcDO(uint32_t insn)
{
	// This instruction simply instructs the co-processor to do something, no data is returned to ARM7 core
	arm7_do_callback(insn); // simply pass entire opcode to callback - since data format is actually dependent on co-proc implementation
}

// Co-Processor Register Transfer - To/From Arm to Co-Proc
void arm7_cpu_device::HandleCoProcRT(uint32_t insn)
{
	/* xxxx 1110 oooL nnnn dddd cccc ppp1 mmmm */

	// Load (MRC) data from Co-Proc to ARM7 register
	if (insn & 0x00100000)       // Bit 20 = Load or Store
	{
		uint32_t res = arm7_rt_r_callback(insn);   // RT Read handler must parse opcode & return appropriate result
		if (!m_pendingUnd)
		{
			uint32_t const rd = (insn >> 12) & 0xf;
			if (rd == eR15)
			{
				set_cpsr((GET_CPSR & ~(N_MASK | Z_MASK | C_MASK | V_MASK)) | (res & (N_MASK | Z_MASK | C_MASK | V_MASK)));
			}
			else
			{
				SetRegister(rd, res);
			}
		}
	}
	// Store (MCR) data from ARM7 to Co-Proc register
	else
	{
		arm7_rt_w_callback(insn, GetRegister((insn >> 12) & 0xf));
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
void arm7_cpu_device::HandleCoProcDT(uint32_t insn)
{
	uint32_t rn = (insn >> 16) & 0xf;
	uint32_t rnv = GetRegister(rn);    // Get Address Value stored from Rn
	uint32_t ornv = rnv;                // Keep value of Rn
	uint32_t off = (insn & 0xff) << 2;  // Offset is << 2 according to manual
	uint32_t *prn = &ARM7REG(rn);       // Pointer to our register, so it can be changed in the callback

#if ARM7_DEBUG_CORE
	if (((insn >> 16) & 0xf) == 15 && (insn & 0x200000))
		LOGMASKED(LOG_OPS, "%08x: Illegal use of R15 as base for write back value!\n", R15);
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
		SetRegister(rn, ornv);
}

void arm7_cpu_device::HandleBranch(uint32_t insn, bool h_bit)
{
	uint32_t off = (insn & INSN_BRANCH) << 2;
	if (h_bit)
	{
		// H goes to bit1
		off |= (insn & 0x01000000) >> 23;
	}

	/* Save PC into LR if this is a branch with link or a BLX */
	if ((insn & INSN_BL) || ((m_archRev >= 5) && ((insn & 0xfe000000) == 0xfa000000)))
	{
		SetRegister(14, R15 + 4);
	}

	/* Sign-extend the 24-bit offset in our calculations */
	if (MODE32)
		R15 += util::sext(off, 26) + 8;
	else
		R15 = ((R15 + (util::sext(off, 26) + 8)) & 0x03FFFFFC) | (R15 & ~0x03FFFFFC);
}

void arm7_cpu_device::HandleMemSingle(uint32_t insn)
{
	uint32_t rn, rnv, off, rd, rnv_old = 0;

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
	rd = (insn & INSN_RD) >> INSN_RD_SHIFT;
	bool const is_load = (insn & INSN_SDT_L) != 0;

	// A store reads its data before the base write-back, so STR Rn,[Rn,#off]! stores the original Rn.
	uint32_t const store_value = is_load ? 0 : ((rd == eR15) ? (R15 + 12) : GetRegister(rd));

	if (insn & INSN_SDT_P)
	{
		/* Pre-indexed addressing */
		if (insn & INSN_SDT_U)
		{
			if ((MODE32) || (rn != eR15))
				rnv = (GetRegister(rn) + off);
			else
				rnv = (GET_PC + off);
		}
		else
		{
			if ((MODE32) || (rn != eR15))
				rnv = (GetRegister(rn) - off);
			else
				rnv = (GET_PC - off);
		}

		if (insn & INSN_SDT_W)
		{
			rnv_old = GetRegister(rn);
			if (rn == eR15 && !MODE32)
				write_r15_psr26(rnv, true);   // R15 as the base: the write-back also sets the PSR (the A680 boot ROM does LDR R0,[R15],#0)
			else
				SetRegister(rn, rnv);

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
			rnv = GetRegister(rn);
		}
	}

	/* Do the transfer */
	if (is_load)
	{
		/* Load */
		if (insn & INSN_SDT_B)
		{
			uint32_t data = READ8(rnv);
			if (!m_pendingAbtD)
			{
				SetRegister(rd, data);
			}
		}
		else
		{
			uint32_t data = READ32(rnv);
			if (!m_pendingAbtD)
			{
				if (rd == eR15)
				{
					if (MODE32)
						R15 = data - 4;
					else
					{
						// Data East DE156: an unaligned target resumes at the next word boundary (World Cup Volleyball '95)
						if (m_ldr_pc_round_up && (data & 3))
							data += 4;
						R15 = (R15 & ~0x03FFFFFC) /* N Z C V I F M1 M0 */ | ((data - 4) & 0x03FFFFFC);
					}
					// LDR, PC takes 2S + 2N + 1I (5 total cycles)
					ARM7_ICOUNT -= 2;
					if ((data & 1) && m_archRev >= 5)
					{
						set_cpsr(GET_CPSR | T_MASK);
						R15--;
					}
				}
				else
				{
					SetRegister(rd, data);
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
				LOGMASKED(LOG_OPS, "Wrote R15 in byte mode\n");
#endif

			WRITE8(rnv, uint8_t(store_value));
		}
		else
		{
#if ARM7_DEBUG_CORE
			if (rd == eR15)
				LOGMASKED(LOG_OPS, "Wrote R15 in 32bit mode\n");
#endif

			WRITE32(rnv, store_value);
		}
		// Store takes only 2 N Cycles, so add + 1
		ARM7_ICOUNT += 1;
	}

	if (m_pendingAbtD)
	{
		if ((m_archRev < 4) && (COPRO_CTRL & COPRO_CTRL_LATE_ABORT))
		{
			// ARMv3 late abort: the base register write-back still takes place and the abort
			// handler has to undo it.
			if (!(insn & INSN_SDT_P))
			{
				if (rn == eR15 && !MODE32)
					write_r15_psr26((insn & INSN_SDT_U) ? (rnv + off) : (rnv - off), true);
				else
					SetRegister(rn, (insn & INSN_SDT_U) ? (rnv + off) : (rnv - off));
			}
		}
		else if ((insn & INSN_SDT_P) && (insn & INSN_SDT_W))
		{
			SetRegister(rn, rnv_old);
		}
	}
	else
	{
	/* Do post-indexing writeback */
	if (!(insn & INSN_SDT_P)/* && (insn & INSN_SDT_W)*/)
	{
		if (insn & INSN_SDT_U)
		{
			if (rd == rn && is_load) {
				SetRegister(rn, GetRegister(rd));
				// todo: check for offs... ?
			}
			else {
				if ((insn & INSN_SDT_W) != 0)
					LOGMASKED(LOG_OPS, "%08x:  RegisterWritebackIncrement %d %d %d\n", R15, (insn & INSN_SDT_P) != 0, (insn & INSN_SDT_W) != 0, (insn & INSN_SDT_U) != 0);

				if (rn == eR15 && !MODE32)
					write_r15_psr26(rnv + off, true);   // R15 as the base: the write-back also sets the PSR (the A680 boot ROM does LDR R0,[R15],#0)
				else
					SetRegister(rn, (rnv + off));
			}
		}
		else
		{
			if (rd == rn && is_load) {
				SetRegister(rn, GetRegister(rd));
			}
			else {
				if (rn == eR15 && !MODE32)
					write_r15_psr26(rnv - off, true);   // R15 as the base: the write-back also sets the PSR (the A680 boot ROM does LDR R0,[R15],#0)
				else
					SetRegister(rn, (rnv - off));

				if ((insn & INSN_SDT_W) != 0)
					LOGMASKED(LOG_OPS, "%08x:  RegisterWritebackDecrement %d %d %d\n", R15, (insn & INSN_SDT_P) != 0, (insn & INSN_SDT_W) != 0, (insn & INSN_SDT_U) != 0);
			}
		}
	}
}

//  arm7_check_irq_state();

} /* HandleMemSingle */

void arm7_cpu_device::HandleHalfWordDT(uint32_t insn)
{
	uint32_t rn, rnv, off, rd, rnv_old = 0;

	// Immediate or Register Offset?
	if (insn & 0x400000) {               // Bit 22 - 1 = immediate, 0 = register
		// imm. value in high nibble (bits 8-11) and lo nibble (bit 0-3)
		off = (((insn >> 8) & 0x0f) << 4) | (insn & 0x0f);
	}
	else {
		// register
		off = GetRegister(insn & 0x0f);
	}

	/* Calculate Rn, accounting for PC */
	rn = (insn & INSN_RN) >> INSN_RN_SHIFT;
	rd = (insn & INSN_RD) >> INSN_RD_SHIFT;
	bool const is_load = (insn & INSN_SDT_L) || ((insn & 0x60) == 0x40);   // L bit or LDRD

	const uint32_t store_value = is_load ? 0 : ((rd == eR15) ? (R15 + 12) : GetRegister(rd));
	const uint32_t store_value_hi = (is_load || (rd >= eR15)) ? 0 : GetRegister(rd + 1);   // STRD

	if (insn & INSN_SDT_P)
	{
		/* Pre-indexed addressing */
		if (insn & INSN_SDT_U)
		{
			rnv = (GetRegister(rn) + off);
		}
		else
		{
			rnv = (GetRegister(rn) - off);
		}

		if (insn & INSN_SDT_W)
		{
			rnv_old = GetRegister(rn);
			SetRegister(rn, rnv);

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
			rnv = GetRegister(rn);
		}
	}

	/* Do the transfer */
	/* Load */
	if (insn & INSN_SDT_L)
	{
		// Signed?
		if (insn & 0x40)
		{
			uint32_t newval;

			// Signed Half Word?
			if (insn & 0x20) {
				int32_t data = (int32_t)(int16_t)(uint16_t)READ16(rnv & ~1);
				if ((rnv & 1) && m_archRev < 5)
					data >>= 8;
				newval = (uint32_t)data;
			}
			// Signed Byte
			else {
				uint8_t databyte;
				uint32_t signbyte;
				databyte = READ8(rnv) & 0xff;
				signbyte = (databyte & 0x80) ? 0xffffff : 0;
				newval = (uint32_t)(signbyte << 8)|databyte;
			}

			if (!m_pendingAbtD)
			{
				// PC? (officially unpredictable on ARMv4 and later - the halfword path does not add 4 afterwards, so R15 takes the value as-is)
				if (rd == eR15)
				{
					R15 = newval;
					ARM7_ICOUNT -= 2;
				}
				else
				{
					SetRegister(rd, newval);
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
			uint32_t newval = READ16(rnv);

			if (!m_pendingAbtD)
			{
				if (rd == eR15)
				{
					R15 = newval;
					// extra cycles for LDR(H,SH,SB) PC (5 total cycles)
					ARM7_ICOUNT -= 2;
				}
				else
				{
					SetRegister(rd, newval);
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
		if (((insn & 0x60) == 0x40 || (insn & 0x60) == 0x60) && !(m_archFlags & ARCHFLAG_E))
		{
			// LDRD/STRD need the E variant; earlier cores have no instruction here
			LOGMASKED(LOG_OPS, "%08x: LDRD/STRD on a core without the DSP extensions\n", R15);
			if ((insn & INSN_SDT_P) && (insn & INSN_SDT_W))
			{
				SetRegister(rn, rnv_old);
			}
			R15 += 4;
			m_pendingUnd = true;
			update_irq_state();
			return;
		}
		else if ((insn & 0x60) == 0x40)  // LDRD - v5TE
		{
			if ((rd & 1) || (rd == eR14))
			{
				// odd Rd (including R15) and Rd = R14 is unpredictable: take an undefined instruction trap
				LOGMASKED(LOG_OPS, "%08x: LDRD with UNPREDICTABLE Rd = R%d\n", R15, rd);
				if ((insn & INSN_SDT_P) && (insn & INSN_SDT_W))
				{
					SetRegister(rn, rnv_old);
				}
				R15 += 4;
				m_pendingUnd = true;
				update_irq_state();
				return;
			}

			uint32_t const lo = READ32(rnv & ~3);
			uint32_t const hi = m_pendingAbtD ? 0 : READ32((rnv & ~3) + 4);
			if (!m_pendingAbtD)
			{
				SetRegister(rd, lo);
				SetRegister(rd+1, hi);
			}
			R15 += 4;
		}
		else if ((insn & 0x60) == 0x60) // STRD - v5TE
		{
			if ((rd & 1) || (rd == eR14))
			{
				LOGMASKED(LOG_OPS, "%08x: STRD with UNPREDICTABLE Rd = R%d\n", R15, rd);
				if ((insn & INSN_SDT_P) && (insn & INSN_SDT_W))
				{
					SetRegister(rn, rnv_old);
				}
				R15 += 4;
				m_pendingUnd = true;
				update_irq_state();
				return;
			}

			WRITE32(rnv & ~3, store_value);
			if (!m_pendingAbtD)
			{
				WRITE32((rnv & ~3) + 4, store_value_hi);
			}
			R15 += 4;
		}
		else
		{
			WRITE16(rnv, store_value);   // STRH R15 stores PC+12

			// if R15 is not increased then e.g. "STRH R10, [R15,#$10]" will be executed over and over again
			R15 += 4;

			// STRH takes 2 cycles, so we add + 1
			ARM7_ICOUNT += 1;
		}
	}

	if (m_pendingAbtD)
	{
		if ((insn & INSN_SDT_P) && (insn & INSN_SDT_W))
		{
			SetRegister(rn, rnv_old);
		}
	}
	else
	{
	/* Do post-indexing writeback */
	if (!(insn & INSN_SDT_P)/* && (insn & INSN_SDT_W)*/)
	{
		if (insn & INSN_SDT_U)
		{
			if (rd == rn && is_load) {
				SetRegister(rn, GetRegister(rd));
				// todo: check for offs... ?
			}
			else {
				if ((insn & INSN_SDT_W) != 0)
					LOGMASKED(LOG_OPS, "%08x:  RegisterWritebackIncrement %d %d %d\n", R15, (insn & INSN_SDT_P) != 0, (insn & INSN_SDT_W) != 0, (insn & INSN_SDT_U) != 0);

				SetRegister(rn, (rnv + off));
			}
		}
		else
		{
			if (rd == rn && is_load) {
				SetRegister(rn, GetRegister(rd));
			}
			else {
				SetRegister(rn, (rnv - off));

				if ((insn & INSN_SDT_W) != 0)
					LOGMASKED(LOG_OPS, "%08x:  RegisterWritebackDecrement %d %d %d\n", R15, (insn & INSN_SDT_P) != 0, (insn & INSN_SDT_W) != 0, (insn & INSN_SDT_U) != 0);
			}
		}
	}

	}

}

void arm7_cpu_device::HandleSwap(uint32_t insn)
{
	uint32_t rn, rm, rd, tmp;

	rn = GetRegister((insn >> 16) & 0xf);  // reg. w/read address
	rm = GetRegister(insn & 0xf);          // reg. w/write address
	rd = (insn >> 12) & 0xf;                // dest reg

#if ARM7_DEBUG_CORE
	if (rn == 15 || rm == 15 || rd == 15)
		LOGMASKED(LOG_OPS, "%08x: Illegal use of R15 in Swap Instruction\n", R15);
#endif

	// can be byte or word
	if (insn & 0x400000)
	{
		tmp = READ8(rn);
		if (!m_pendingAbtD)
		{
			WRITE8(rn, rm);
			SetRegister(rd, tmp);
		}
	}
	else
	{
		tmp = READ32(rn);
		if (!m_pendingAbtD)
		{
			WRITE32(rn, rm);
			SetRegister(rd, tmp);
		}
	}

	R15 += 4;
	// Instruction takes 1S+2N+1I cycles - so we subtract one more..
	ARM7_ICOUNT -= 1;
}

void arm7_cpu_device::HandlePSRTransfer(uint32_t insn)
{
	int reg = (insn & 0x400000) ? SPSR : eCPSR; // Either CPSR or SPSR
	uint32_t newval, val;
	int oldmode = GET_CPSR & MODE_FLAG;

	// get old value of CPSR/SPSR
	newval = GetRegister(reg);

	// MSR (bit 21 set) - Copy value to CPSR/SPSR
	if ((insn & 0x00200000))
	{
		// Immediate Value?
		if (insn & INSN_I) {
			// Value can be specified for a Right Rotate, 2x the value specified.
			int by = (insn & INSN_OP2_ROTATE) >> INSN_OP2_ROTATE_SHIFT;
			if (by)
				val = std::rotr<uint32_t>(insn & INSN_OP2_IMM, by << 1);
			else
				val = insn & INSN_OP2_IMM;
		}
		// Value from Register
		else
		{
			val = GetRegister(insn & 0x0f);
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
		{
			set_cpsr(newval);
		}
		else
			SetRegister(reg, newval);

		// Switch to new mode if changed
		if ((newval & MODE_FLAG) != oldmode)
			SwitchMode(GET_MODE);

	}
	// MRS (bit 21 clear) - Copy CPSR or SPSR to specified Register
	else
	{
		if ((reg == SPSR) && (oldmode == eARM7_MODE_USER || oldmode == eARM7_MODE_SYS))
		{
			LOGMASKED(LOG_OPS, "%08x: MRS from SPSR in a mode without an SPSR (UNPREDICTABLE, returns CPSR)\n", R15);
		}
		SetRegister((insn >> 12)& 0x0f, GetRegister(reg));
	}
}

void arm7_cpu_device::HandleALU(uint32_t insn)
{
	uint32_t op2, sc = 0, rd, rn, opcode;
	uint32_t by, rdn;

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
			op2 = std::rotr<uint32_t>(insn & INSN_OP2_IMM, by << 1);
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

		// extra cycle (register specified shift).  Only a shift amount taken from a register costs the I cycle
		// (ARM2 and ARM7TDMI datasheets alike); charging it for every register operand is a long-standing
		// over-count that is kept for the ARM7+ types for now, so that no ARM7 machine changes timing here
		if ((insn & 0x10) || m_archRev >= 3)
			ARM7_ICOUNT -= 1;
	}

	// LD TODO this comment is wrong
	/* Calculate Rn to account for pipelining */
	if ((opcode & 0xd) != 0xd) /* No Rn in MOV */
	{
		if ((rn = (insn & INSN_RN) >> INSN_RN_SHIFT) == eR15)
		{
#if ARM7_DEBUG_CORE
			LOGMASKED(LOG_OPS, "%08x:  Pipelined R15 (Shift %d)\n", R15, (insn & INSN_I ? 8 : insn & 0x10u ? 12 : 12));
#endif
			if (MODE32)
				rn = R15 + 8;
			else
				rn = GET_PC + 8;
		}
		else
		{
			rn = GetRegister(rn);
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
			if (rdn == eR15)
			{
				if (MODE32)
				{
				// When Rd is R15 and the S flag is set the result of the operation is placed in R15 and the SPSR corresponding to
				// the current mode is moved to the CPSR. This allows state changes which automatically restore both PC and
				// CPSR. --> This form of instruction should not be used in User mode. <--

					if (GET_MODE != eARM7_MODE_USER && GET_MODE != eARM7_MODE_SYS)
					{
						// Update CPSR from SPSR
						set_cpsr(GetRegister(SPSR));
						SwitchMode(GET_MODE);
					}
					else
					{
						LOGMASKED(LOG_OPS, "%08x: S-bit write to R15 in a mode without an SPSR (UNPREDICTABLE)\n", R15);
					}

					if (MODE32)
					{
						// On exception return, the instruction set state comes from the restored CPSR and the address is
						// aligned for it.  Fixes Nintendo DS boot.
						R15 = rd & (T_IS_SET(GET_CPSR) ? ~1 : ~3);
					}
					else
					{
						R15 = (R15 & ~0x03FFFFFC) | (rd & 0x03FFFFFC); // returned to a 26-bit mode: set_cpsr() has put the PSR in R15, take only the address
					}
				}
				else
				{
					// 26-bit mode: the result carries the new PSR in bits 31:26 and 1:0 (protected in user mode)
					write_r15_psr26(rd, true);
				}

				// extra cycles (PC written)
				ARM7_ICOUNT -= 2;

				/* IRQ masks may have changed in this instruction */
//              arm7_check_irq_state();
			}
			else
				/* S Flag is set - Write results to register & update CPSR (which was already handled using HandleALU flag macros) */
				SetRegister(rdn, rd);
		}
	}
	// SJE: Don't think this applies any more.. (see page 44 at bottom)
	/* TST & TEQ can affect R15 (the condition code register) with the S bit set */
	else if (rdn == eR15)
	{
		if (insn & INSN_S) {
#if ARM7_DEBUG_CORE
			LOGMASKED(LOG_OPS, "%08x: TST class on R15 s bit set\n", R15);
#endif
			if (MODE32)
			{
				// Rd is SBZ for the compare instructions in the 32-bit architectures (UNPREDICTABLE): the flags
				// have been set normally above, R15 is left alone
				LOGMASKED(LOG_OPS, "%08x: TST/TEQ/CMP/CMN with Rd = R15 in a 32-bit mode\n", R15);
			}
			else
			{
				// TSTP/TEQP/CMPP/CMNP: the result goes to the PSR only (protected in user mode)
				write_r15_psr26(rd, false);
			}

			/* IRQ masks may have changed in this instruction */
//          arm7_check_irq_state();
		}
		else
		{
#if ARM7_DEBUG_CORE
			LOGMASKED(LOG_OPS, "%08x: TST class on R15 no s bit set\n", R15);
#endif
		}
		// extra cycles (PC written)
		ARM7_ICOUNT -= 2;
	}

	// compensate for the -3 at the end
	ARM7_ICOUNT += 2;
}

void arm7_cpu_device::HandleMul(uint32_t insn)
{
	uint32_t r, rm, rs;

	// MUL takes 1S + mI and MLA 1S + (m+1)I cycles to execute, where S and I are as
	// defined in 6.2 Cycle Types on page 6-2.
	// m is the number of 8 bit multiplier array cycles required to complete the
	// multiply, which is controlled by the value of the multiplier operand
	// specified by Rs.

	rm = GetRegister(insn & INSN_MUL_RM);
	rs = GetRegister((insn & INSN_MUL_RS) >> INSN_MUL_RS_SHIFT);

	/* Do the basic multiply of Rm and Rs */
	r = rm * rs;

#if ARM7_DEBUG_CORE
	if ((insn & INSN_MUL_RM) == 0xf ||
		((insn & INSN_MUL_RS) >> INSN_MUL_RS_SHIFT) == 0xf ||
		((insn & INSN_MUL_RN) >> INSN_MUL_RN_SHIFT) == 0xf)
		LOGMASKED(LOG_OPS, "%08x:  R15 used in mult\n", R15);
#endif

	/* Add on Rn if this is a MLA */
	if (insn & INSN_MUL_A)
	{
		r += GetRegister((insn & INSN_MUL_RN) >> INSN_MUL_RN_SHIFT);
		// extra cycle for MLA
		if (m_archRev >= 3)
			ARM7_ICOUNT -= 1;
	}

	/* Write the result */
	SetRegister((insn & INSN_MUL_RD) >> INSN_MUL_RD_SHIFT, r);

	/* Set N and Z if asked */
	if (insn & INSN_S)
	{
		set_cpsr((GET_CPSR & ~(N_MASK | Z_MASK)) | HandleALUNZFlags(r));
	}

	if (m_archRev < 3)
	{
		// ARM2/ARM3: 1S + 1I for MUL and MLA alike, as the old cpu/arm core charged (the real parts take 1S + mI with m up
		// to 16); the chess computer wait-state models (tasc, risc2500) were tuned against that
		ARM7_ICOUNT -= 2;
	}
	else
	{
		if (rs & SIGN_BIT) rs = -rs;
		if (rs < 0x00000100) ARM7_ICOUNT -= 1 + 1;
		else if (rs < 0x00010000) ARM7_ICOUNT -= 1 + 2;
		else if (rs < 0x01000000) ARM7_ICOUNT -= 1 + 3;
		else ARM7_ICOUNT -= 1 + 4;
	}

	ARM7_ICOUNT += 3;
}

// todo: add proper cycle counts
void arm7_cpu_device::HandleSMulLong(uint32_t insn)
{
	// MULL takes 1S + (m+1)I and MLAL 1S + (m+2)I cycles to execute, where m is the
	// number of 8 bit multiplier array cycles required to complete the multiply, which is
	// controlled by the value of the multiplier operand specified by Rs.

	int32_t  rm  = (int32_t)GetRegister(insn & 0xf);
	int32_t  rs  = (int32_t)GetRegister(((insn >> 8) & 0xf));
	uint32_t rhi = (insn >> 16) & 0xf;
	uint32_t rlo = (insn >> 12) & 0xf;

#if ARM7_DEBUG_CORE
		if ((insn & 0xf) == 15 || ((insn >> 8) & 0xf) == 15 || ((insn >> 16) & 0xf) == 15 || ((insn >> 12) & 0xf) == 15)
			LOGMASKED(LOG_OPS, "%08x: Illegal use of PC as a register in SMULL opcode\n", R15);
#endif

	/* Perform the multiplication */
	int64_t res = mul_32x32(rm, rs);

	/* Add on Rn if this is a MLA */
	if (insn & INSN_MUL_A)
	{
		int64_t acum = (int64_t)((((int64_t)(GetRegister(rhi))) << 32) | GetRegister(rlo));
		res += acum;
		// extra cycle for MLA
		ARM7_ICOUNT -= 1;
	}

	/* Write the result (upper dword goes to RHi, lower to RLo) */
	SetRegister(rhi, res >> 32);
	SetRegister(rlo, res & 0xFFFFFFFF);

	/* Set N and Z if asked */
	if (insn & INSN_S)
	{
		set_cpsr((GET_CPSR & ~(N_MASK | Z_MASK)) | HandleLongALUNZFlags(res));
	}

	if (rs < 0) rs = -rs;
	if (rs < 0x00000100) ARM7_ICOUNT -= 1 + 1 + 1;
	else if (rs < 0x00010000) ARM7_ICOUNT -= 1 + 2 + 1;
	else if (rs < 0x01000000) ARM7_ICOUNT -= 1 + 3 + 1;
	else ARM7_ICOUNT -= 1 + 4 + 1;

	ARM7_ICOUNT += 3;
}

// todo: add proper cycle counts
void arm7_cpu_device::HandleUMulLong(uint32_t insn)
{
	// MULL takes 1S + (m+1)I and MLAL 1S + (m+2)I cycles to execute, where m is the
	// number of 8 bit multiplier array cycles required to complete the multiply, which is
	// controlled by the value of the multiplier operand specified by Rs.

	uint32_t rm  = GetRegister(insn & 0xf);
	uint32_t rs  = GetRegister(((insn >> 8) & 0xf));
	uint32_t rhi = (insn >> 16) & 0xf;
	uint32_t rlo = (insn >> 12) & 0xf;

#if ARM7_DEBUG_CORE
		if (((insn & 0xf) == 15) || (((insn >> 8) & 0xf) == 15) || (((insn >> 16) & 0xf) == 15) || (((insn >> 12) & 0xf) == 15))
			LOGMASKED(LOG_OPS, "%08x: Illegal use of PC as a register in SMULL opcode\n", R15);
#endif

	/* Perform the multiplication */
	uint64_t res = mulu_32x32(rm, rs);

	/* Add on Rn if this is a MLA */
	if (insn & INSN_MUL_A)
	{
		uint64_t acum = (uint64_t)((((uint64_t)(GetRegister(rhi))) << 32) | GetRegister(rlo));
		res += acum;
		// extra cycle for MLA
		ARM7_ICOUNT -= 1;
	}

	/* Write the result (upper dword goes to RHi, lower to RLo) */
	SetRegister(rhi, res >> 32);
	SetRegister(rlo, res & 0xFFFFFFFF);

	/* Set N and Z if asked */
	if (insn & INSN_S)
	{
		set_cpsr((GET_CPSR & ~(N_MASK | Z_MASK)) | HandleLongALUNZFlags(res));
	}

	if (rs < 0x00000100) ARM7_ICOUNT -= 1 + 1 + 1;
	else if (rs < 0x00010000) ARM7_ICOUNT -= 1 + 2 + 1;
	else if (rs < 0x01000000) ARM7_ICOUNT -= 1 + 3 + 1;
	else ARM7_ICOUNT -= 1 + 4 + 1;

	ARM7_ICOUNT += 3;
}

void arm7_cpu_device::HandleMemBlock(uint32_t insn)
{
	uint32_t rb = (insn & INSN_RN) >> INSN_RN_SHIFT;
	uint32_t rbp = GetRegister(rb);
	uint32_t const rb_orig = rbp;
	int result;

	if ((insn & 0xffff) == 0)
	{
		// Empty register list is unpredictable. Real cores write the base back by 0x40 as if all 16 registers
		// had been transferred, and ARMv4 cores (ARM7TDMI) also transfer R15 through the first slot.
		uint32_t const addr = (insn & INSN_BDT_U) ? (rbp + ((insn & INSN_BDT_P) ? 4 : 0)) : (rbp - ((insn & INSN_BDT_P) ? 0x40 : 0x3c));
		LOGMASKED(LOG_OPS, "%08x: %s with an empty register list\n", R15, (insn & INSN_BDT_L) ? "LDM" : "STM");
		if (m_archRev < 5)
		{
			if (insn & INSN_BDT_L)
			{
				uint32_t const data = READ32(addr & ~3);
				if (!m_pendingAbtD && MODE32)
					R15 = data - 4;     // arm7ops_89 adds 4
			}
			else
			{
				WRITE32(addr & ~3, R15 + 12);
			}
		}
		if ((insn & INSN_BDT_W) && !m_pendingAbtD)
			SetRegister(rb, (insn & INSN_BDT_U) ? (rbp + 0x40) : (rbp - 0x40));
		return;
	}

#if ARM7_DEBUG_CORE
	if (rbp & 3)
		LOGMASKED(LOG_OPS, "%08x: Unaligned Mem Transfer @ %08x\n", R15, rbp);
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
				LOGMASKED(LOG_OPS, "%08x: User Bank Transfer not fully tested - please check if working properly!\n", R15);
				result = loadInc(insn & 0xffff, rbp, insn & INSN_BDT_S, eARM7_MODE_USER);
				// todo - not sure if Writeback occurs on User registers also..
				//SwitchMode(curmode);
			}
			else
				result = loadInc(insn & 0xffff, rbp, insn & INSN_BDT_S, GET_MODE);

			if ((insn & INSN_BDT_W) && !m_pendingAbtD)
			{
#if ARM7_DEBUG_CORE
				if (rb == 15)
					LOGMASKED(LOG_OPS, "%08x:  Illegal LDRM writeback to r15\n", R15);
#endif
				// "A LDM will always overwrite the updated base if the base is in the list." (see ldm_loads_base())
				// GBA "V-Rally 3" expects R0 not to be overwritten with the updated base value [BP 8077B0C]
				if (!ldm_loads_base(insn, rb))
				{
					SetRegister(rb, GetRegister(rb) + result * 4);
				}
			}

			// R15 included? (NOTE: CPSR restore must occur LAST otherwise wrong registers restored!)
			if ((insn & 0x8000) && !m_pendingAbtD)
			{
				R15 -= 4;     // SJE: I forget why i did this?
				// S - Flag Set? Signals transfer of current mode SPSR->CPSR
				if (insn & INSN_BDT_S)
				{
					if (MODE32)
					{
						if (GET_MODE != eARM7_MODE_USER && GET_MODE != eARM7_MODE_SYS)
						{
							set_cpsr(GetRegister(SPSR));
							SwitchMode(GET_MODE);
						}
						else
						{
							LOGMASKED(LOG_OPS, "%08x: LDM with PC and S bit in a mode without an SPSR (UNPREDICTABLE)\n", R15);
						}

						// if the new mode we just SwitchMode'd to is 32-bit, mask R15 correctly for ARM vs Thumb
						if (MODE32)
						{
							R15 &= T_IS_SET(GET_CPSR) ? ~1 : ~3;
						}
					}
					else
					{
						// 26-bit mode: the loaded word carries the new PSR (protected in user mode)
						write_r15_psr26(R15, true);
					}
				}
				else
					if ((R15 & 1) && m_archRev >= 5)
					{
						set_cpsr(GET_CPSR | T_MASK);
						R15--;
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
				LOGMASKED(LOG_OPS, "%08x: User Bank Transfer not fully tested - please check if working properly!\n", R15);
				result = loadDec(insn & 0xffff, rbp, insn & INSN_BDT_S, eARM7_MODE_USER);
				// todo - not sure if Writeback occurs on User registers also..
				//SwitchMode(curmode);
			}
			else
				result = loadDec(insn & 0xffff, rbp, insn & INSN_BDT_S, GET_MODE);

			if ((insn & INSN_BDT_W) && !m_pendingAbtD)
			{
				if (rb == 0xf)
					LOGMASKED(LOG_OPS, "%08x:  Illegal LDRM writeback to r15\n", R15);
				// "A LDM will always overwrite the updated base if the base is in the list." (see ldm_loads_base())
				if (!ldm_loads_base(insn, rb))
				{
					SetRegister(rb, GetRegister(rb) - result * 4);
				}
			}

			// R15 included? (NOTE: CPSR restore must occur LAST otherwise wrong registers restored!)
			if ((insn & 0x8000) && !m_pendingAbtD) {
				R15 -= 4;     // SJE: I forget why i did this?
				// S - Flag Set? Signals transfer of current mode SPSR->CPSR
				if (insn & INSN_BDT_S)
				{
					if (MODE32)
					{
						if (GET_MODE != eARM7_MODE_USER && GET_MODE != eARM7_MODE_SYS)
						{
							set_cpsr(GetRegister(SPSR));
							SwitchMode(GET_MODE);
						}
						else
						{
							LOGMASKED(LOG_OPS, "%08x: LDM with PC and S bit in a mode without an SPSR (UNPREDICTABLE)\n", R15);
						}

						// if the new mode we just SwitchMode'd to is 32-bit, mask R15 correctly for ARM vs Thumb
						if (MODE32)
						{
							R15 &= T_IS_SET(GET_CPSR) ? ~1 : ~3;
						}
					}
					else
					{
						// 26-bit mode: the loaded word carries the new PSR (protected in user mode)
						write_r15_psr26(R15, true);
					}
				}
				else
					if ((R15 & 1) && m_archRev >= 5)
					{
						set_cpsr(GET_CPSR | T_MASK);
						R15--;
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
			LOGMASKED(LOG_OPS, "%08x: Writing R15 in strm\n", R15);
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
				LOGMASKED(LOG_OPS, "%08x: User Bank Transfer not fully tested - please check if working properly!\n", R15);
				result = storeInc(insn & 0xffff, rbp, eARM7_MODE_USER);
				// todo - not sure if Writeback occurs on User registers also..
				//SwitchMode(curmode);
			}
			else
				result = storeInc(insn & 0xffff, rbp, GET_MODE);

			if ((insn & INSN_BDT_W) && !m_pendingAbtD)
			{
				SetRegister(rb, GetRegister(rb) + result * 4);
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
				LOGMASKED(LOG_OPS, "%08x: User Bank Transfer not fully tested - please check if working properly!\n", R15);
				result = storeDec(insn & 0xffff, rbp, eARM7_MODE_USER);
				// todo - not sure if Writeback occurs on User registers also..
				//SwitchMode(curmode);
			}
			else
				result = storeDec(insn & 0xffff, rbp, GET_MODE);

			if ((insn & INSN_BDT_W) && !m_pendingAbtD)
			{
				SetRegister(rb, GetRegister(rb) - result * 4);
			}
		}
		if (insn & (1 << eR15))
			R15 -= 12;

		// STM takes (n-1)S + 2N cycles (n = # of register transfers)
		ARM7_ICOUNT -= (result - 1) + 2;
	}

	// On ARMv3 an aborted LDM/STM still writes back its base register as if the whole transfer had
	// completed, in both the early and late abort models, and the abort handler is expected
	// to undo that (Newton OS does so to work out the stack extension it needs). Loading the base
	// from the list is suppressed by the abort, so the written-back value is what remains.
	// ARMv4 and later restore the base instead.
	if (m_pendingAbtD && (insn & INSN_BDT_W) && (m_archRev < 4))
	{
		uint32_t const count = std::popcount(insn & 0xffffu);
		SetRegister(rb, (insn & INSN_BDT_U) ? (rb_orig + (count << 2)) : (rb_orig - (count << 2)));
	}

	// We will specify the cycle count for each case, so remove the -3 that occurs at the end
	ARM7_ICOUNT += 3;

} /* HandleMemBlock */


const arm7_cpu_device::arm7ops_ophandler arm7_cpu_device::ops_handler[0x20] =
{
	&arm7_cpu_device::arm7ops_0123, &arm7_cpu_device::arm7ops_0123, &arm7_cpu_device::arm7ops_0123, &arm7_cpu_device::arm7ops_0123,
	&arm7_cpu_device::arm7ops_4567, &arm7_cpu_device::arm7ops_4567, &arm7_cpu_device::arm7ops_4567, &arm7_cpu_device::arm7ops_4567,
	&arm7_cpu_device::arm7ops_89,   &arm7_cpu_device::arm7ops_89,   &arm7_cpu_device::arm7ops_ab,   &arm7_cpu_device::arm7ops_ab,
	&arm7_cpu_device::arm7ops_cd,   &arm7_cpu_device::arm7ops_cd,   &arm7_cpu_device::arm7ops_e,    &arm7_cpu_device::arm7ops_f,
	&arm7_cpu_device::arm9ops_undef,&arm7_cpu_device::arm9ops_1,    &arm7_cpu_device::arm9ops_undef,&arm7_cpu_device::arm9ops_undef,
	&arm7_cpu_device::arm9ops_undef,&arm7_cpu_device::arm9ops_57,   &arm7_cpu_device::arm9ops_undef,&arm7_cpu_device::arm9ops_57,
	&arm7_cpu_device::arm9ops_89,   &arm7_cpu_device::arm9ops_89,   &arm7_cpu_device::arm9ops_ab,   &arm7_cpu_device::arm9ops_ab,
	&arm7_cpu_device::arm9ops_c,    &arm7_cpu_device::arm9ops_undef,&arm7_cpu_device::arm9ops_e,    &arm7_cpu_device::arm9ops_undef,
};

void arm7_cpu_device::arm9ops_undef(uint32_t insn)
{
	// Undefined or unsupported instruction in the unconditional (cond = 1111) space.
	// Advance past it and take the undefined instruction exception (R14_und = next instruction).
	LOGMASKED(LOG_OPS, "%08x: ARM7: Instruction %08X unsupported\n", R15, insn);
	R15 += 4;
	m_pendingUnd = true;
	update_irq_state();
}

void arm7_cpu_device::arm9ops_1(uint32_t insn)
{
	/* Change processor state (CPS) */
	if ((insn & 0x00f10020) == 0x00000000)
	{
		// unsupported (armv6 onwards only)
		arm9ops_undef(insn);
	}
	else if ((insn & 0x00ff00f0) == 0x00010000) /* set endianness (SETEND) */
	{
		// Only for ARMv6 and later
		if (m_archRev < 6)
		{
			arm9ops_undef(insn);
		}
		else
		{
			uint32_t new_cpsr = GET_CPSR & ~(1 << 9);
			set_cpsr(new_cpsr | (insn & (1 << 9)));
			R15 += 4;
		}
	}
	else
	{
		arm9ops_undef(insn);
	}
}

void arm7_cpu_device::arm9ops_57(uint32_t insn)
{
	// Cache Preload (PLD) for ARMv5TE+.  We don't (yet?) emulate the cache.
	if ((insn & 0x0070f000) == 0x0050f000)
	{
		R15 += 4;
	}
	else
	{
		arm9ops_undef(insn);
	}
}

void arm7_cpu_device::arm9ops_89(uint32_t insn)
{
	/* Save Return State (SRS) */
	if ((insn & 0x005f0f00) == 0x004d0500)
	{
		// unsupported (armv6 onwards only)
		arm9ops_undef(insn);
	}
	else if ((insn & 0x00500f00) == 0x00100a00) /* Return From Exception (RFE) */
	{
		// unsupported (armv6 onwards only)
		arm9ops_undef(insn);
	}
	else
	{
		arm9ops_undef(insn);
	}
}

void arm7_cpu_device::arm9ops_ab(uint32_t insn)
{
	// BLX
	HandleBranch(insn, true);
	set_cpsr(GET_CPSR|T_MASK);
}

void arm7_cpu_device::arm9ops_c(uint32_t insn)
{
	/* Additional coprocessor double register transfer (MCRR2/MRRC2) */
	if ((insn & 0x00e00000) == 0x00400000)
	{
		// unsupported
		arm9ops_undef(insn);
	}
	else
	{
		arm9ops_undef(insn);
	}
}

void arm7_cpu_device::arm9ops_e(uint32_t insn)
{
	/* Additional coprocessor register transfer (CDP2/MCR2/MRC2) */
	// unsupported
	arm9ops_undef(insn);
}


void arm7_cpu_device::arm7ops_0123(uint32_t insn)
{
//case 0:
//case 1:
//case 2:
//case 3:
	/* Branch and Exchange (BX) */
	if (m_archRev >= 3 && (insn & 0x0ffffff0) == 0x012fff10)     // bits 27-4 == 000100101111111111110001 (on ARM2/ARM3 this is a TEQ without S: a no-op)
	{
		R15 = GetRegister(insn & 0x0f);
		// If new PC address has A0 set, switch to Thumb mode
		if (R15 & 1) {
			R15--;
			set_cpsr(GET_CPSR|T_MASK);
		}
	}
	else if (m_archRev < 5)
	{
		arm7ops_0123_v4(insn);
		return;
	}
	else if ((insn & 0x0ff000f0) == 0x01200030) // BLX Rn - v5
	{
		// read the target before writing LR, because BLX LR is legal
		uint32_t const target = GetRegister(insn & 0x0f);

		// save link address
		SetRegister(14, R15 + 4);

		R15 = target;
		// If new PC address has A0 set, switch to Thumb mode
		if (R15 & 1) {
			R15--;
			set_cpsr(GET_CPSR|T_MASK);
		}
	}
	else if ((insn & 0x0ff000f0) == 0x01200070) // BKPT - v5
	{
		LOGMASKED(LOG_OPS, "%08x: BKPT #%d\n", R15, ((insn >> 4) & 0xfff0) | (insn & 0xf));
		COPRO_FAULT_STATUS_D = COPRO_FAULT_DEBUG;
		m_pendingAbtP = true;
		update_irq_state();
	}
	else if ((insn & 0x0ff000f0) == 0x01600010) // CLZ - v5
	{
		uint32_t rm = insn&0xf;
		uint32_t rd = (insn>>12)&0xf;

		SetRegister(rd, std::countl_zero(GetRegister(rm)));

		R15 += 4;
	}
	else if (!(m_archFlags & ARCHFLAG_E) && (insn & 0x0f900090) == 0x01000080)
	{
		arm7ops_undef_conditional(insn);
	}
	else if (!(m_archFlags & ARCHFLAG_E) && (insn & 0x0f9000f0) == 0x01000050)
	{
		arm7ops_undef_conditional(insn);
	}
	else if ((insn & 0x0ff000f0) == 0x01000050) // QADD - v5
	{
		int32_t src1 = GetRegister(insn&0xf);
		int32_t src2 = GetRegister((insn>>16)&0xf);

		int64_t res = saturate_qbit_overflow((int64_t)src1 + (int64_t)src2);

		SetRegister((insn>>12)&0xf, (int32_t)res);
		R15 += 4;
	}
	else if ((insn & 0x0ff000f0) == 0x01400050) // QDADD - v5
	{
		int32_t src1 = GetRegister(insn&0xf);
		int32_t src2 = GetRegister((insn>>16)&0xf);

		// Rd = SignedSat(Rm + SignedSat(Rn * 2, 32), 32) - both saturations can set Q
		src2 = (int32_t)saturate_qbit_overflow((int64_t)src2 * 2);
		int64_t res = saturate_qbit_overflow((int64_t)src1 + (int64_t)src2);

		SetRegister((insn>>12)&0xf, (int32_t)res);
		R15 += 4;
	}
	else if ((insn & 0x0ff000f0) == 0x01200050) // QSUB - v5
	{
		int32_t src1 = GetRegister(insn&0xf);
		int32_t src2 = GetRegister((insn>>16)&0xf);

		int64_t res = saturate_qbit_overflow((int64_t)src1 - (int64_t)src2);

		SetRegister((insn>>12)&0xf, (int32_t)res);
		R15 += 4;
	}
	else if ((insn & 0x0ff000f0) == 0x01600050) // QDSUB - v5
	{
		int32_t src1 = GetRegister(insn&0xf);
		int32_t src2 = GetRegister((insn>>16)&0xf);

		// Rd = SignedSat(Rm - SignedSat(Rn * 2, 32), 32) - both saturations can set Q
		src2 = (int32_t)saturate_qbit_overflow((int64_t)src2 * 2);
		int64_t res = saturate_qbit_overflow((int64_t)src1 - (int64_t)src2);

		SetRegister((insn>>12)&0xf, (int32_t)res);
		R15 += 4;
	}
	else if ((insn & 0x0ff00090) == 0x01000080) // SMLAxy - v5
	{
		int32_t src1 = GetRegister(insn&0xf);
		int32_t src2 = GetRegister((insn>>8)&0xf);
		int32_t res1;

		// select top and bottom halves of src1/src2 and sign extend if necessary
		if (insn & 0x20)
			src1 >>= 16;
		else
			src1 = util::sext(src1, 16);

		if (insn & 0x40)
			src2 >>= 16;
		else
			src2 = util::sext(src2, 16);

		// do the signed multiply
		res1 = src1 * src2;
		// and the accumulate.  NOTE: only the accumulate can cause an overflow, which is why we do it this way.
		saturate_qbit_overflow((int64_t)res1 + (int64_t)GetRegister((insn>>12)&0xf));

		SetRegister((insn>>16)&0xf, res1 + GetRegister((insn>>12)&0xf));
		R15 += 4;
	}
	else if ((insn & 0x0ff00090) == 0x01400080) // SMLALxy - v5
	{
		int32_t src1 = GetRegister(insn&0xf);
		int32_t src2 = GetRegister((insn>>8)&0xf);

		// select top and bottom halves of src1/src2 and sign extend if necessary
		if (insn & 0x20)
		{
			src1 >>= 16;
		}
		else
		{
			src1 = util::sext(src1, 16);
		}

		if (insn & 0x40)
		{
			src2 >>= 16;
		}
		else
		{
			src2 = util::sext(src2, 16);
		}

		int64_t dst = (int64_t)GetRegister((insn>>12)&0xf);
		dst |= (int64_t)GetRegister((insn>>16)&0xf)<<32;

		// do the 16x16 multiply and the 64-bit accumulate (no saturation, no Q flag)
		dst += (int64_t)src1 * (int64_t)src2;

		// write back the result
		SetRegister((insn>>12)&0xf, (uint32_t)dst);
		SetRegister((insn>>16)&0xf, (uint32_t)(dst >> 32));
		R15 += 4;
	}
	else if ((insn & 0x0ff00090) == 0x01600080) // SMULxy - v5
	{
		int32_t src1 = GetRegister(insn&0xf);
		int32_t src2 = GetRegister((insn>>8)&0xf);

		// select top and bottom halves of src1/src2 and sign extend if necessary
		if (insn & 0x20)
			src1 >>= 16;
		else
			src1 = util::sext(src1, 16);

		if (insn & 0x40)
			src2 >>= 16;
		else
			src2 = util::sext(src2, 16);

		int32_t res = src1 * src2;
		SetRegister((insn>>16)&0xf, res);
		R15 += 4;
	}
	else if ((insn & 0x0ff000b0) == 0x012000a0) // SMULWy - v5
	{
		int32_t src1 = GetRegister(insn&0xf);
		int32_t src2 = GetRegister((insn>>8)&0xf);

		if (insn & 0x40)
			src2 >>= 16;
		else
			src2 = util::sext(src2, 16);

		int32_t res = mul_32x32_shift(src1, src2, 16);
		SetRegister((insn>>16)&0xf, (uint32_t)res);
		R15 += 4;
	}
	else if ((insn & 0x0ff000b0) == 0x01200080) // SMLAWy - v5
	{
		int32_t src1 = GetRegister(insn&0xf);
		int32_t src2 = GetRegister((insn>>8)&0xf);
		int32_t src3 = GetRegister((insn>>12)&0xf);

		if (insn & 0x40)
			src2 >>= 16;
		else
			src2 = util::sext(src2, 16);

		int32_t res = mul_32x32_shift(src1, src2, 16);

		// check for overflow and set the Q bit (the result itself is not saturated)
		saturate_qbit_overflow((int64_t)src3 + res);

		// do the real accumulate and write the result back
		SetRegister((insn>>16)&0xf, (uint32_t)src3 + (uint32_t)res);
		R15 += 4;
	}
	else
	{
		arm7ops_0123_v4(insn);
	}
}

void arm7_cpu_device::arm7ops_undef_conditional(uint32_t insn)
{
	// undefined instruction in the conditional space: advance past it and take the UND trap
	LOGMASKED(LOG_OPS, "%08x: undefined instruction %08X\n", R15, insn);
	R15 += 4;
	m_pendingUnd = true;
	update_irq_state();
}

void arm7_cpu_device::arm7ops_0123_v4(uint32_t insn)
{
	/* Multiply OR Swap OR Half Word Data Transfer */
	if ((insn & 0x0e000000) == 0 && (insn & 0x80) && (insn & 0x10))  // bits 27-25=000 bit 7=1 bit 4=1
	{
		/* Half Word Data Transfer */
		if (insn & 0x60)         // bits = 6-5 != 00
		{
			if (m_archRev < 3)
			{
				// ARM2/ARM3: the "multiply hole" with bits 6:5 set is undefined
				arm7ops_undef_conditional(insn);
				return;
			}
			HandleHalfWordDT(insn);
		}
		else
		/* Swap */
		if (insn & 0x01000000)   // bit 24 = 1
		{
			if (m_archRev < 3 && !(m_archFlags & ARCHFLAG_V2A))
			{
				// ARM1/ARM2: no SWP (ARMv2a)
				arm7ops_undef_conditional(insn);
				return;
			}
			HandleSwap(insn);
		}
		/* Multiply Or Multiply Long */
		else
		{
			/* multiply long */
			if (insn & 0x800000) // Bit 23 = 1 for Multiply Long
			{
				if (m_archRev < 3)
				{
					// ARMv3M and later
					arm7ops_undef_conditional(insn);
					return;
				}
				/* Signed? */
				if (insn & 0x00400000)
					HandleSMulLong(insn);
				else
					HandleUMulLong(insn);
			}
			/* multiply */
			else
			{
				if (m_archRev < 2)
				{
					// ARM1 has no multiplier
					arm7ops_undef_conditional(insn);
					return;
				}
				HandleMul(insn);
			}
			R15 += 4;
		}
	}
	/* Data Processing OR PSR Transfer */
	else if ((insn & 0x0c000000) == 0)   // bits 27-26 == 00 - This check can only exist properly after Multiplication check above
	{
		/* PSR Transfer (MRS & MSR) - ARMv3 and later; on ARM2/ARM3 these encodings are TST/TEQ/CMP/CMN without S, no-ops */
		if (m_archRev >= 3 && ((insn & 0x00100000) == 0) && ((insn & 0x01800000) == 0x01000000)) // S bit must be clear, and bit 24,23 = 10
		{
			if (!(insn & INSN_I) && (insn & 0xf0) != 0)
			{
				// cond 00010 xx0 .... with bits 7:4 != 0000 is the rest of the miscellaneous space: the ARMv5 additions
				// (already decoded above on v5 cores) and the remaining holes. Not MSR/MRS - trap it.
				arm7ops_undef_conditional(insn);
			}
			else
			{
				HandlePSRTransfer(insn);
				ARM7_ICOUNT += 2;       // PSR only takes 1 - S Cycle, so we add + 2, since at end, we -3..
				R15 += 4;
			}
		}
		/* Data Processing */
		else
		{
			HandleALU(insn);
		}
	}
//  break;
}

void arm7_cpu_device::arm7ops_4567(uint32_t insn) /* Data Transfer - Single Data Access */
{
	if ((insn & INSN_I) && (insn & 0x10))
	{
		// the ARMv6 media instructions don't exist on ARMv5 and earlier
		LOGMASKED(LOG_OPS, "%08x: undefined instruction %08X in the single data transfer space\n", R15, insn);
		R15 += 4;
		m_pendingUnd = true;
		update_irq_state();
		return;
	}

	HandleMemSingle(insn);
	R15 += 4;
}

void arm7_cpu_device::arm7ops_89(uint32_t insn) /* Block Data Transfer/Access */
{
	HandleMemBlock(insn);
	R15 += 4;
}

void arm7_cpu_device::arm7ops_ab(uint32_t insn) /* Branch or Branch & Link */
{
	HandleBranch(insn, false);
}

void arm7_cpu_device::arm7ops_cd(uint32_t insn) /* Co-Processor Data Transfer */
{
	if ((insn & 0x0fe00000) == 0x0c400000)  // MCRR/MRRC - v5TE: cond 1100 010L Rn Rd cp_num opcode CRm
	{
		// The only coprocessor in the tree with a double-register transfer is the XScale DSP (CP0: MAR/MRA),
		// which is not emulated yet - ignore those like the rest of its CP0 accesses; everything else traps.
		if ((m_archFlags & ARCHFLAG_XSCALE) && ((insn >> 8) & 0xf) == 0)
		{
			LOGMASKED(LOG_OPS, "%08x: XScale %s (DSP Coprocessor 0 not yet emulated)\n", R15, (insn & 0x00100000) ? "MRA" : "MAR");
			R15 += 4;
		}
		else
		{
			R15 += 4;
			m_pendingUnd = true;
			update_irq_state();
		}
		return;
	}

	HandleCoProcDT(insn);
	R15 += 4;
	if (m_archRev < 3)
		ARM7_ICOUNT += 2;   // 1S on ARM2/ARM3 (old cpu/arm core)
}

void arm7_cpu_device::arm7ops_e(uint32_t insn) /* Co-Processor Data Operation or Register Transfer */
{
	if (insn & 0x10)
		HandleCoProcRT(insn);
	else
		HandleCoProcDO(insn);
	R15 += 4;
	if (m_archRev < 3)
		ARM7_ICOUNT += 2;   // 1S on ARM2/ARM3 (old cpu/arm core)
}

void arm7_cpu_device::arm7ops_f(uint32_t insn) /* Software Interrupt */
{
	m_pendingSwi = true;
	update_irq_state();
	arm7_check_irq_state();
	//couldn't find any cycle counts for SWI
}
