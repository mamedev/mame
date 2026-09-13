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

	uint32_t const cpsr = m_r[eCPSR];
	uint32_t const pc = m_r[eR15] + 4;

	/* Exception priorities:

	    Reset
	    Data abort
	    FIRQ
	    IRQ
	    Prefetch abort
	    Undefined instruction
	    Software Interrupt
	*/

	bool const prog32 = !(m_archFlags & ARCHFLAG_MODE26) || (COPRO_CTRL & COPRO_CTRL_PROG32);

	auto const enter_exception = [this, cpsr, prog32] (uint32_t mode32, uint32_t mode26, uint32_t link, uint32_t vector, bool mask_fiq)
	{
		if (prog32)
		{
			// switch banks; leaving a 26-bit mode strips the PSR bits from R15
			set_cpsr((cpsr & ~uint32_t(0x1f)) | SR_MODE32 | mode32);
			if (!(cpsr & SR_MODE32))
				link &= 0x03FFFFFC;         // R14 is a plain address in the 32-bit modes
			SetRegister(14, link);
			SetRegister(SPSR, cpsr);
			set_cpsr((GET_CPSR | I_MASK | (mask_fiq ? F_MASK : 0)) & ~T_MASK);
			R15 = vector | vector_base();    // CP15 c1 V bit selects the high vectors whether or not the MMU is enabled
		}
		else
		{
			// switch to 26-bit mode
			set_cpsr((cpsr & ~uint32_t(0x1f)) | mode26);
			// R14 gets the return address with the interrupted code's PSR in bits 31:26 and 1:0
			SetRegister(14, (link & 0x03FFFFFC) | (cpsr & 0xF0000000) | ((cpsr & (I_MASK | F_MASK)) << 20) | (cpsr & 3));
			// fetch the vector in the new mode
			R15 = (cpsr & 0xF0000000) | ((cpsr & F_MASK) << 20) | (mask_fiq ? 0x0C000000 : 0x08000000) | vector | mode26;
			set_cpsr((GET_CPSR & 0x0FFFFF3F) | (R15 & 0xF0000000) | ((R15 & 0x0C000000) >> 20));
		}
	};

	// Data Abort, or the 26-bit data space address exception (same priority and completion rules, but vector 0x14 and
	// always SVC mode - there is no ABT mode in the 26-bit world)
	if (m_pendingAbtD)
	{
		if (m_pendingAddrExc)
			enter_exception(eARM7_MODE_SVC, eARM7_MODE_SVC, T_IS_SET(cpsr) ? (pc + 2) : pc, 0x14, false);
		else
			enter_exception(eARM7_MODE_ABT, eARM7_MODE_SVC, T_IS_SET(cpsr) ? (pc + 2) : pc, 0x10, false);
		m_pendingAbtD = false;
		m_pendingAddrExc = false;
		update_irq_state();
		return;
	}

	// FIQ
	if (m_pendingFiq && (cpsr & F_MASK) == 0)
	{
		standard_irq_callback(ARM7_FIRQ_LINE, pc);
		enter_exception(eARM7_MODE_FIQ, eARM7_MODE_FIQ, pc, 0x1c, true);    // R14 = next instruction + 4
		return;
	}

	// IRQ
	if (m_pendingIrq && (cpsr & I_MASK) == 0)
	{
		standard_irq_callback(ARM7_IRQ_LINE, pc);
		enter_exception(eARM7_MODE_IRQ, eARM7_MODE_IRQ, pc, 0x18, false);   // R14 = next instruction + 4
		return;
	}

	// Prefetch Abort
	if (m_pendingAbtP)
	{
		enter_exception(eARM7_MODE_ABT, eARM7_MODE_SVC, pc, 0x0c, false);   // R14 = aborted instruction + 4
		m_pendingAbtP = false;
		update_irq_state();
		return;
	}

	// Undefined instruction
	if (m_pendingUnd)
	{
		// compensate for prefetch (should this also be done for normal IRQ?)
		uint32_t const link = T_IS_SET(cpsr) ? (pc - 4 + 2) : (pc - 4 + 4 - 4);
		enter_exception(eARM7_MODE_UND, eARM7_MODE_SVC, link, 0x04, false);
		m_pendingUnd = false;
		update_irq_state();
		return;
	}

	// Software Interrupt
	if (m_pendingSwi)
	{
		// compensate for prefetch (should this also be done for normal IRQ?)
		uint32_t const link = T_IS_SET(cpsr) ? (pc - 4 + 2) : (pc - 4 + 4);
		enter_exception(eARM7_MODE_SVC, eARM7_MODE_SVC, link, 0x08, false);
		m_pendingSwi = false;
		update_irq_state();
		return;
	}
}
