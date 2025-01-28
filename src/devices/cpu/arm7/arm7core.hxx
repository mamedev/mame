// license:BSD-3-Clause
// copyright-holders:Steve Ellenoff,R. Belmont,Ryan Holtz
/*****************************************************************************
 *
 *   arm7core.hxx
 *   Portable ARM7TDMI Core Emulator
 *
 *   Copyright Steve Ellenoff
 *
 *  This work is based on:
 *  #1) 'Atmel Corporation ARM7TDMI (Thumb) Datasheet - January 1999'
 *  #2) Arm 2/3/6 emulator By Bryan McPhail (bmcphail@tendril.co.uk) and Phil Stroffolino (MAME CORE 0.76)
 *  #3) Thumb support by Ryan Holtz
 *  #4) Additional Thumb support and bugfixes by R. Belmont
 *
 *****************************************************************************/

/******************************************************************************
 *  Notes:

    **This core comes from my AT91 cpu core contributed to PinMAME,
      but with all the AT91 specific junk removed,
      which leaves just the ARM7TDMI core itself. I further removed the CPU specific MAME stuff
      so you just have the actual ARM7 core itself, since many cpu's incorporate an ARM7 core, but add on
      many cpu specific functionality.

      Therefore, to use the core, you simpy include this file along with the .h file into your own cpu specific
      implementation, and therefore, this file shouldn't be compiled as part of your project directly.

      For better or for worse, the code itself is very much intact from it's arm 2/3/6 origins from
      Bryan & Phil's work. I contemplated merging it in, but thought the fact that the CPSR is
      no longer part of the PC was enough of a change to make it annoying to merge.
    **

    Coprocessor functions are heavily implementation specific, so callback handlers are used to allow the
    implementation to handle the functionality. Custom DASM handlers are included as well to allow the DASM
    output to be tailored to the co-proc implementation details.

    Todo:
    26 bit compatibility mode not implemented.
    Data Processing opcodes need cycle count adjustments (see page 194 of ARM7TDMI manual for instruction timing summary)
    Multi-emulated cpu support untested, but probably will not work too well, as no effort was made to code for more than 1.
    Could not find info on what the TEQP opcode is from page 44..
    I have no idea if user bank switching is right, as I don't fully understand it's use.
    Search for Todo: tags for remaining items not done.


    Differences from Arm 2/3 (6 also?)
    -Thumb instruction support
    -Full 32 bit address support
    -PC no longer contains CPSR information, CPSR is own register now
    -New register SPSR to store previous contents of CPSR (this register is banked in many modes)
    -New opcodes for CPSR transfer, Long Multiplication, Co-Processor support, and some others
    -User Bank Mode transfer using certain flags which were previously unallowed (LDM/STM with S Bit & R15)
    -New operation modes? (unconfirmed)

    Based heavily on arm core from MAME 0.76:
    *****************************************
    ARM 2/3/6 Emulation

    Todo:
    Software interrupts unverified (nothing uses them so far, but they should be ok)
    Timing - Currently very approximated, nothing relies on proper timing so far.
    IRQ timing not yet correct (again, nothing is affected by this so far).

    By Bryan McPhail (bmcphail@tendril.co.uk) and Phil Stroffolino
*****************************************************************************/


// convert cpsr mode num into to text
static const char modetext[ARM7_NUM_MODES][5] = {
	"USER", "FIRQ", "IRQ",  "SVC", "ILL1", "ILL2", "ILL3", "ABT",
	"ILL4", "ILL5", "ILL6", "UND", "ILL7", "ILL8", "ILL9", "SYS"
};
static const char *GetModeText(int cpsr)
{
	return modetext[cpsr & MODE_FLAG];
}




/***************************************************************************
 *                            Main CPU Funcs
 ***************************************************************************/

void arm7_cpu_device::update_irq_state()
{
	m_pending_interrupt = m_pendingAbtD || m_pendingAbtP || m_pendingUnd || m_pendingSwi || m_pendingFiq || m_pendingIrq;
}

// CPU CHECK IRQ STATE
// Note: couldn't find any exact cycle counts for most of these exceptions
void arm7_cpu_device::arm7_check_irq_state()
{
	if (!m_pending_interrupt)
		return;

	uint32_t cpsr = m_r[eCPSR];   /* save current CPSR */
	uint32_t pc = m_r[eR15] + 4;      /* save old pc (already incremented in pipeline) */;

	/* Exception priorities:

	    Reset
	    Data abort
	    FIRQ
	    IRQ
	    Prefetch abort
	    Undefined instruction
	    Software Interrupt
	*/

	// Data Abort
	if (m_pendingAbtD)
	{
		if (MODE26) fatalerror("ARM7: pendingAbtD (todo)\n");
		SwitchMode(eARM7_MODE_ABT);             /* Set ABT mode so PC is saved to correct R14 bank */
		SetRegister(14, pc - 8 + 8);                   /* save PC to R14 */
		SetRegister(SPSR, cpsr);               /* Save current CPSR */
		set_cpsr(GET_CPSR | I_MASK);            /* Mask IRQ */
		set_cpsr(GET_CPSR & ~T_MASK);
		R15 = 0x10;                             /* IRQ Vector address */
		if ((COPRO_CTRL & COPRO_CTRL_MMU_EN) && (COPRO_CTRL & COPRO_CTRL_INTVEC_ADJUST)) R15 |= 0xFFFF0000;
		m_pendingAbtD = false;
		update_irq_state();
		return;
	}

	// FIQ
	if (m_pendingFiq && (cpsr & F_MASK) == 0)
	{
		standard_irq_callback(ARM7_FIRQ_LINE, pc);
		if (MODE26) fatalerror("pendingFiq (todo)\n");
		SwitchMode(eARM7_MODE_FIQ);             /* Set FIQ mode so PC is saved to correct R14 bank */
		SetRegister(14, pc - 4 + 4);                   /* save PC to R14 */
		SetRegister(SPSR, cpsr);               /* Save current CPSR */
		set_cpsr(GET_CPSR | I_MASK | F_MASK);   /* Mask both IRQ & FIQ */
		set_cpsr(GET_CPSR & ~T_MASK);
		R15 = 0x1c;                             /* IRQ Vector address */
		R15 |= m_vectorbase;
		if ((COPRO_CTRL & COPRO_CTRL_MMU_EN) && (COPRO_CTRL & COPRO_CTRL_INTVEC_ADJUST)) R15 |= 0xFFFF0000;
		return;
	}

	// IRQ
	if (m_pendingIrq && (cpsr & I_MASK) == 0)
	{
		standard_irq_callback(ARM7_IRQ_LINE, pc);
		SwitchMode(eARM7_MODE_IRQ);             /* Set IRQ mode so PC is saved to correct R14 bank */
		SetRegister(14, pc - 4 + 4);                   /* save PC to R14 */
		if (MODE32)
		{
			SetRegister(SPSR, cpsr);               /* Save current CPSR */
			set_cpsr(GET_CPSR | I_MASK);            /* Mask IRQ */
			set_cpsr(GET_CPSR & ~T_MASK);
			R15 = 0x18;                             /* IRQ Vector address */
		}
		else
		{
			uint32_t temp;
			R15 = (pc & 0xF4000000) /* N Z C V F */ | 0x18 | 0x00000002 /* IRQ */ | 0x08000000 /* I */;
			temp = (GET_CPSR & 0x0FFFFF3F) /* N Z C V I F */ | (R15 & 0xF0000000) /* N Z C V */ | ((R15 & 0x0C000000) >> (26 - 6)) /* I F */;
			set_cpsr(temp);            /* Mask IRQ */
		}
		R15 |= m_vectorbase;
		if ((COPRO_CTRL & COPRO_CTRL_MMU_EN) && (COPRO_CTRL & COPRO_CTRL_INTVEC_ADJUST)) R15 |= 0xFFFF0000;
		return;
	}

	// Prefetch Abort
	if (m_pendingAbtP)
	{
		if (MODE26) fatalerror("pendingAbtP (todo)\n");
		SwitchMode(eARM7_MODE_ABT);             /* Set ABT mode so PC is saved to correct R14 bank */
		SetRegister(14, pc - 4 + 4);                   /* save PC to R14 */
		SetRegister(SPSR, cpsr);               /* Save current CPSR */
		set_cpsr(GET_CPSR | I_MASK);            /* Mask IRQ */
		set_cpsr(GET_CPSR & ~T_MASK);
		R15 = 0x0c | m_vectorbase;                             /* IRQ Vector address */
		if ((COPRO_CTRL & COPRO_CTRL_MMU_EN) && (COPRO_CTRL & COPRO_CTRL_INTVEC_ADJUST)) R15 |= 0xFFFF0000;
		m_pendingAbtP = false;
		update_irq_state();
		return;
	}

	// Undefined instruction
	if (m_pendingUnd)
	{
		if (MODE26) printf("ARM7: pendingUnd (todo)\n");
		SwitchMode(eARM7_MODE_UND);             /* Set UND mode so PC is saved to correct R14 bank */
		// compensate for prefetch (should this also be done for normal IRQ?)
		if (T_IS_SET(GET_CPSR))
		{
			SetRegister(14, pc - 4 + 2);         /* save PC to R14 */
		}
		else
		{
			SetRegister(14, pc - 4 + 4 - 4);           /* save PC to R14 */
		}
		SetRegister(SPSR, cpsr);               /* Save current CPSR */
		set_cpsr(GET_CPSR | I_MASK);            /* Mask IRQ */
		set_cpsr(GET_CPSR & ~T_MASK);
		R15 = 0x04 | m_vectorbase;                             /* IRQ Vector address */
		if ((COPRO_CTRL & COPRO_CTRL_MMU_EN) && (COPRO_CTRL & COPRO_CTRL_INTVEC_ADJUST)) R15 |= 0xFFFF0000;
		m_pendingUnd = false;
		update_irq_state();
		return;
	}

	// Software Interrupt
	if (m_pendingSwi)
	{
		SwitchMode(eARM7_MODE_SVC);             /* Set SVC mode so PC is saved to correct R14 bank */
		// compensate for prefetch (should this also be done for normal IRQ?)
		if (T_IS_SET(GET_CPSR))
		{
			SetRegister(14, pc - 4 + 2);         /* save PC to R14 */
		}
		else
		{
			SetRegister(14, pc - 4 + 4);           /* save PC to R14 */
		}
		if (MODE32)
		{
			SetRegister(SPSR, cpsr);               /* Save current CPSR */
			set_cpsr(GET_CPSR | I_MASK);            /* Mask IRQ */
			set_cpsr(GET_CPSR & ~T_MASK);           /* Go to ARM mode */
			R15 = 0x08;                             /* Jump to the SWI vector */
		}
		else
		{
			uint32_t temp;
			R15 = (pc & 0xF4000000) /* N Z C V F */ | 0x08 | 0x00000003 /* SVC */ | 0x08000000 /* I */;
			temp = (GET_CPSR & 0x0FFFFF3F) /* N Z C V I F */ | (R15 & 0xF0000000) /* N Z C V */ | ((R15 & 0x0C000000) >> (26 - 6)) /* I F */;
			set_cpsr(temp);            /* Mask IRQ */
		}
		R15 |= m_vectorbase;
		if ((COPRO_CTRL & COPRO_CTRL_MMU_EN) && (COPRO_CTRL & COPRO_CTRL_INTVEC_ADJUST)) R15 |= 0xFFFF0000;
		m_pendingSwi = false;
		update_irq_state();
		return;
	}
}
