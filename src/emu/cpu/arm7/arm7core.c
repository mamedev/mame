/*****************************************************************************
 *
 *   arm7core.c
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
      Additionally, you will need to include arm7exec.c in your cpu's execute routine.

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

#define ARM7_DEBUG_CORE 0

/* Prototypes */

// SJE: should these be inline? or are they too big to see any benefit?

static void HandleCoProcDO(arm_state *cpustate, UINT32 insn);
static void HandleCoProcRT(arm_state *cpustate, UINT32 insn);
static void HandleCoProcDT(arm_state *cpustate, UINT32 insn);
static void HandleHalfWordDT(arm_state *cpustate, UINT32 insn);
static void HandleSwap(arm_state *cpustate, UINT32 insn);
static void HandlePSRTransfer(arm_state *cpustate, UINT32 insn);
static void HandleALU(arm_state *cpustate, UINT32 insn);
static void HandleMul(arm_state *cpustate, UINT32 insn);
static void HandleUMulLong(arm_state *cpustate, UINT32 insn);
static void HandleSMulLong(arm_state *cpustate, UINT32 insn);
INLINE void HandleBranch(arm_state *cpustate, UINT32 insn);       // pretty short, so inline should be ok
static void HandleMemSingle(arm_state *cpustate, UINT32 insn);
static void HandleMemBlock(arm_state *cpustate, UINT32 insn);
static UINT32 decodeShift(arm_state *cpustate, UINT32 insn, UINT32 *pCarry);
INLINE void SwitchMode(arm_state *cpustate, int);
static void arm7_check_irq_state(arm_state *cpustate);

INLINE void arm7_cpu_write32(arm_state *cpustate, UINT32 addr, UINT32 data);
INLINE void arm7_cpu_write16(arm_state *cpustate, UINT32 addr, UINT16 data);
INLINE void arm7_cpu_write8(arm_state *cpustate, UINT32 addr, UINT8 data);
INLINE UINT32 arm7_cpu_read32(arm_state *cpustate, UINT32 addr);
INLINE UINT16 arm7_cpu_read16(arm_state *cpustate, UINT32 addr);
INLINE UINT8 arm7_cpu_read8(arm_state *cpustate, offs_t addr);

/* Static Vars */
// Note: for multi-cpu implementation, this approach won't work w/o modification
write32_device_func arm7_coproc_do_callback;    // holder for the co processor Data Operations Callback func.
read32_device_func arm7_coproc_rt_r_callback;   // holder for the co processor Register Transfer Read Callback func.
write32_device_func arm7_coproc_rt_w_callback;  // holder for the co processor Register Transfer Write Callback Callback func.
// holder for the co processor Data Transfer Read & Write Callback funcs
void (*arm7_coproc_dt_r_callback)(arm_state *cpustate, UINT32 insn, UINT32 *prn, UINT32 (*read32)(arm_state *cpustate, UINT32 addr));
void (*arm7_coproc_dt_w_callback)(arm_state *cpustate, UINT32 insn, UINT32 *prn, void (*write32)(arm_state *cpustate, UINT32 addr, UINT32 data));

#ifdef UNUSED_DEFINITION
// custom dasm callback handlers for co-processor instructions
char *(*arm7_dasm_cop_dt_callback)(arm_state *cpustate, char *pBuf, UINT32 opcode, char *pConditionCode, char *pBuf0);
char *(*arm7_dasm_cop_rt_callback)(arm_state *cpustate, char *pBuf, UINT32 opcode, char *pConditionCode, char *pBuf0);
char *(*arm7_dasm_cop_do_callback)(arm_state *cpustate, char *pBuf, UINT32 opcode, char *pConditionCode, char *pBuf0);
#endif


/***************************************************************************
 * Default Memory Handlers
 ***************************************************************************/
INLINE void arm7_cpu_write32(arm_state *cpustate, UINT32 addr, UINT32 data)
{
    if( COPRO_CTRL & COPRO_CTRL_MMU_EN )
    {
        addr = arm7_tlb_translate( cpustate, addr, 0 );
    }

    addr &= ~3;
	if ( cpustate->endian == ENDIANNESS_BIG )
		cpustate->program->write_dword(addr, data);
	else
	    cpustate->program->write_dword(addr, data);
}


INLINE void arm7_cpu_write16(arm_state *cpustate, UINT32 addr, UINT16 data)
{
    if( COPRO_CTRL & COPRO_CTRL_MMU_EN )
    {
        addr = arm7_tlb_translate( cpustate, addr, 0 );
    }

    addr &= ~1;
	if ( cpustate->endian == ENDIANNESS_BIG )
		cpustate->program->write_word(addr, data);
	else
		cpustate->program->write_word(addr, data);
}

INLINE void arm7_cpu_write8(arm_state *cpustate, UINT32 addr, UINT8 data)
{
    if( COPRO_CTRL & COPRO_CTRL_MMU_EN )
    {
        addr = arm7_tlb_translate( cpustate, addr, 0 );
    }

	if ( cpustate->endian == ENDIANNESS_BIG )
		cpustate->program->write_byte(addr, data);
	else
		cpustate->program->write_byte(addr, data);
}

INLINE UINT32 arm7_cpu_read32(arm_state *cpustate, offs_t addr)
{
    UINT32 result;

    if( COPRO_CTRL & COPRO_CTRL_MMU_EN )
    {
        addr = arm7_tlb_translate( cpustate, addr, 0 );
    }

    if (addr & 3)
    {
		if ( cpustate->endian == ENDIANNESS_BIG )
			result = cpustate->program->read_dword(addr & ~3);
		else
			result = cpustate->program->read_dword(addr & ~3);
        result = (result >> (8 * (addr & 3))) | (result << (32 - (8 * (addr & 3))));
    }
    else
    {
		if ( cpustate->endian == ENDIANNESS_BIG )
			result = cpustate->program->read_dword(addr);
		else
			result = cpustate->program->read_dword(addr);
    }

    return result;
}

INLINE UINT16 arm7_cpu_read16(arm_state *cpustate, offs_t addr)
{
    UINT16 result;

    if( COPRO_CTRL & COPRO_CTRL_MMU_EN )
    {
        addr = arm7_tlb_translate( cpustate, addr, 0 );
    }

	if ( cpustate->endian == ENDIANNESS_BIG )
		result = cpustate->program->read_word(addr & ~1);
	else
		result = cpustate->program->read_word(addr & ~1);

    if (addr & 1)
    {
        result = ((result >> 8) & 0xff) | ((result & 0xff) << 8);
    }

    return result;
}

INLINE UINT8 arm7_cpu_read8(arm_state *cpustate, offs_t addr)
{
    if( COPRO_CTRL & COPRO_CTRL_MMU_EN )
    {
        addr = arm7_tlb_translate( cpustate, addr, 0 );
    }

    // Handle through normal 8 bit handler (for 32 bit cpu)
	if ( cpustate->endian == ENDIANNESS_BIG )
		return cpustate->program->read_byte(addr);
	else
		return cpustate->program->read_byte(addr);
}

/***************
 * helper funcs
 ***************/

// TODO LD:
//  - SIGN_BITS_DIFFER = THUMB_SIGN_BITS_DIFFER
//  - do while (0)
//  - HandleALUAddFlags = HandleThumbALUAddFlags except for PC incr
//  - HandleALUSubFlags = HandleThumbALUSubFlags except for PC incr

/* Set NZCV flags for ADDS / SUBS */
#define HandleALUAddFlags(rd, rn, op2)                                                \
  if (insn & INSN_S)                                                                  \
    SET_CPSR(((GET_CPSR & ~(N_MASK | Z_MASK | V_MASK | C_MASK))                       \
              | (((!SIGN_BITS_DIFFER(rn, op2)) && SIGN_BITS_DIFFER(rn, rd)) << V_BIT) \
              | (((~(rn)) < (op2)) << C_BIT)                                          \
              | HandleALUNZFlags(rd)));                                               \
  R15 += 4;

#define HandleThumbALUAddFlags(rd, rn, op2)                                                       \
    SET_CPSR(((GET_CPSR & ~(N_MASK | Z_MASK | V_MASK | C_MASK))                                   \
              | (((!THUMB_SIGN_BITS_DIFFER(rn, op2)) && THUMB_SIGN_BITS_DIFFER(rn, rd)) << V_BIT) \
              | (((~(rn)) < (op2)) << C_BIT)                                                      \
              | HandleALUNZFlags(rd)));                                                           \
  R15 += 2;

#define IsNeg(i) ((i) >> 31)
#define IsPos(i) ((~(i)) >> 31)

#define HandleALUSubFlags(rd, rn, op2)                                                                         \
  if (insn & INSN_S)                                                                                           \
    SET_CPSR(((GET_CPSR & ~(N_MASK | Z_MASK | V_MASK | C_MASK))                                                \
              | ((SIGN_BITS_DIFFER(rn, op2) && SIGN_BITS_DIFFER(rn, rd)) << V_BIT)                             \
              | (((IsNeg(rn) & IsPos(op2)) | (IsNeg(rn) & IsPos(rd)) | (IsPos(op2) & IsPos(rd))) ? C_MASK : 0) \
              | HandleALUNZFlags(rd)));                                                                        \
  R15 += 4;

#define HandleThumbALUSubFlags(rd, rn, op2)                                                                    \
    SET_CPSR(((GET_CPSR & ~(N_MASK | Z_MASK | V_MASK | C_MASK))                                                \
              | ((THUMB_SIGN_BITS_DIFFER(rn, op2) && THUMB_SIGN_BITS_DIFFER(rn, rd)) << V_BIT)                 \
              | (((IsNeg(rn) & IsPos(op2)) | (IsNeg(rn) & IsPos(rd)) | (IsPos(op2) & IsPos(rd))) ? C_MASK : 0) \
              | HandleALUNZFlags(rd)));                                                                        \
  R15 += 2;

/* Set NZC flags for logical operations. */

// This macro (which I didn't write) - doesn't make it obvious that the SIGN BIT = 31, just as the N Bit does,
// therefore, N is set by default
#define HandleALUNZFlags(rd)               \
  (((rd) & SIGN_BIT) | ((!(rd)) << Z_BIT))


// Long ALU Functions use bit 63
#define HandleLongALUNZFlags(rd)                            \
  ((((rd) & ((UINT64)1 << 63)) >> 32) | ((!(rd)) << Z_BIT))

#define HandleALULogicalFlags(rd, sc)                  \
  if (insn & INSN_S)                                   \
    SET_CPSR(((GET_CPSR & ~(N_MASK | Z_MASK | C_MASK)) \
              | HandleALUNZFlags(rd)                   \
              | (((sc) != 0) << C_BIT)));              \
  R15 += 4;

// convert cpsr mode num into to text
static const char modetext[ARM7_NUM_MODES][5] = {
    "USER", "FIRQ", "IRQ",  "SVC", "ILL1", "ILL2", "ILL3", "ABT",
    "ILL4", "ILL5", "ILL6", "UND", "ILL7", "ILL8", "ILL9", "SYS"
};
static const char *GetModeText(int cpsr)
{
    return modetext[cpsr & MODE_FLAG];
}

// used to be functions, but no longer a need, so we'll use define for better speed.
#define GetRegister(cpustate, rIndex)        ARM7REG(sRegisterTable[GET_MODE][rIndex])
#define SetRegister(cpustate, rIndex, value) ARM7REG(sRegisterTable[GET_MODE][rIndex]) = value

// I could prob. convert to macro, but Switchmode shouldn't occur that often in emulated code..
INLINE void SwitchMode(arm_state *cpustate, int cpsr_mode_val)
{
    UINT32 cspr = GET_CPSR & ~MODE_FLAG;
    SET_CPSR(cspr | cpsr_mode_val);
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

static UINT32 decodeShift(arm_state *cpustate, UINT32 insn, UINT32 *pCarry)
{
    UINT32 k  = (insn & INSN_OP2_SHIFT) >> INSN_OP2_SHIFT_SHIFT;  // Bits 11-7
    UINT32 rm = GET_REGISTER(cpustate, insn & INSN_OP2_RM);
    UINT32 t  = (insn & INSN_OP2_SHIFT_TYPE) >> INSN_OP2_SHIFT_TYPE_SHIFT;

    if ((insn & INSN_OP2_RM) == 0xf) {
        rm += 8;
    }

    /* All shift types ending in 1 are Rk, not #k */
    if (t & 1)
    {
//      LOG(("%08x:  RegShift %02x %02x\n", R15, k >> 1, GET_REGISTER(cpustate, k >> 1)));
#if ARM7_DEBUG_CORE
            if ((insn & 0x80) == 0x80)
                LOG(("%08x:  RegShift ERROR (p36)\n", R15));
#endif

        // see p35 for check on this
        //k = GET_REGISTER(cpustate, k >> 1) & 0x1f;

        // Keep only the bottom 8 bits for a Register Shift
        k = GET_REGISTER(cpustate, k >> 1) & 0xff;

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
        break;

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
        break;

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
        break;

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
        break;
    }

    LOG(("%08x: Decodeshift error\n", R15));
    return 0;
} /* decodeShift */


static int loadInc(arm_state *cpustate, UINT32 pat, UINT32 rbv, UINT32 s)
{
    int i, result;
    UINT32 data;

    result = 0;
    rbv &= ~3;
    for (i = 0; i < 16; i++)
    {
        if ((pat >> i) & 1)
        {
            data = READ32(rbv += 4);
            if (cpustate->pendingAbtD != 0) break;
            if (i == 15) {
                if (s) /* Pull full contents from stack */
                    SET_REGISTER(cpustate, 15, data);
                else /* Pull only address, preserve mode & status flags */
                    SET_REGISTER(cpustate, 15, data);
            } else
                SET_REGISTER(cpustate, i, data);

            result++;
        }
    }
    return result;
}

static int loadDec(arm_state *cpustate, UINT32 pat, UINT32 rbv, UINT32 s)
{
    int i, result;
    UINT32 data;

    result = 0;
    rbv &= ~3;
    for (i = 15; i >= 0; i--)
    {
        if ((pat >> i) & 1)
        {
            data = READ32(rbv -= 4);
            if (cpustate->pendingAbtD != 0) break;
            if (i == 15) {
                if (s) /* Pull full contents from stack */
                    SET_REGISTER(cpustate, 15, data);
                else /* Pull only address, preserve mode & status flags */
                    SET_REGISTER(cpustate, 15, data);
            }
            else
                SET_REGISTER(cpustate, i, data);
            result++;
        }
    }
    return result;
}

static int storeInc(arm_state *cpustate, UINT32 pat, UINT32 rbv)
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
            WRITE32(rbv += 4, GET_REGISTER(cpustate, i));
            result++;
        }
    }
    return result;
} /* storeInc */

static int storeDec(arm_state *cpustate, UINT32 pat, UINT32 rbv)
{
    int i, result;

    result = 0;
    for (i = 15; i >= 0; i--)
    {
        if ((pat >> i) & 1)
        {
#if ARM7_DEBUG_CORE
            if (i == 15) /* R15 is plus 12 from address of STM */
                LOG(("%08x: StoreDec on R15\n", R15));
#endif
            WRITE32(rbv -= 4, GET_REGISTER(cpustate, i));
            result++;
        }
    }
    return result;
} /* storeDec */

/***************************************************************************
 *                            Main CPU Funcs
 ***************************************************************************/

// CPU INIT
static void arm7_core_init(running_device *device, const char *cpuname)
{
    arm_state *cpustate = get_safe_token(device);

    state_save_register_device_item_array(device, 0, cpustate->sArmRegister);
    state_save_register_device_item(device, 0, cpustate->pendingIrq);
    state_save_register_device_item(device, 0, cpustate->pendingFiq);
    state_save_register_device_item(device, 0, cpustate->pendingAbtD);
    state_save_register_device_item(device, 0, cpustate->pendingAbtP);
    state_save_register_device_item(device, 0, cpustate->pendingUnd);
    state_save_register_device_item(device, 0, cpustate->pendingSwi);
}

// CPU RESET
static void arm7_core_reset(legacy_cpu_device *device)
{
    arm_state *cpustate = get_safe_token(device);

    device_irq_callback save_irqcallback = cpustate->irq_callback;

    memset(cpustate, 0, sizeof(arm_state));
    cpustate->irq_callback = save_irqcallback;
    cpustate->device = device;
    cpustate->program = device->space(AS_PROGRAM);
	cpustate->endian = ENDIANNESS_LITTLE;
	cpustate->direct = &cpustate->program->direct();

    /* start up in SVC mode with interrupts disabled. */
    SwitchMode(cpustate, eARM7_MODE_SVC);
    SET_CPSR(GET_CPSR | I_MASK | F_MASK | 0x10);
    R15 = 0;
}

// Execute used to be here.. moved to separate file (arm7exec.c) to be included by cpu cores separately

// CPU CHECK IRQ STATE
// Note: couldn't find any exact cycle counts for most of these exceptions
static void arm7_check_irq_state(arm_state *cpustate)
{
    UINT32 cpsr = GET_CPSR;   /* save current CPSR */
    UINT32 pc = R15 + 4;      /* save old pc (already incremented in pipeline) */;

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
    if (cpustate->pendingAbtD) {
        SwitchMode(cpustate, eARM7_MODE_ABT);             /* Set ABT mode so PC is saved to correct R14 bank */
        SET_REGISTER(cpustate, 14, pc - 8 + 8);                   /* save PC to R14 */
        SET_REGISTER(cpustate, SPSR, cpsr);               /* Save current CPSR */
        SET_CPSR(GET_CPSR | I_MASK);            /* Mask IRQ */
        SET_CPSR(GET_CPSR & ~T_MASK);
        R15 = 0x10;                             /* IRQ Vector address */
        if ((COPRO_CTRL & COPRO_CTRL_MMU_EN) && (COPRO_CTRL & COPRO_CTRL_INTVEC_ADJUST)) R15 |= 0xFFFF0000;
        cpustate->pendingAbtD = 0;
        return;
    }

    // FIQ
    if (cpustate->pendingFiq && (cpsr & F_MASK) == 0) {
        SwitchMode(cpustate, eARM7_MODE_FIQ);             /* Set FIQ mode so PC is saved to correct R14 bank */
        SET_REGISTER(cpustate, 14, pc - 4 + 4);                   /* save PC to R14 */
        SET_REGISTER(cpustate, SPSR, cpsr);               /* Save current CPSR */
        SET_CPSR(GET_CPSR | I_MASK | F_MASK);   /* Mask both IRQ & FIQ */
        SET_CPSR(GET_CPSR & ~T_MASK);
        R15 = 0x1c;                             /* IRQ Vector address */
        if ((COPRO_CTRL & COPRO_CTRL_MMU_EN) && (COPRO_CTRL & COPRO_CTRL_INTVEC_ADJUST)) R15 |= 0xFFFF0000;
        return;
    }

    // IRQ
    if (cpustate->pendingIrq && (cpsr & I_MASK) == 0) {
        SwitchMode(cpustate, eARM7_MODE_IRQ);             /* Set IRQ mode so PC is saved to correct R14 bank */
        SET_REGISTER(cpustate, 14, pc - 4 + 4);                   /* save PC to R14 */
        SET_REGISTER(cpustate, SPSR, cpsr);               /* Save current CPSR */
        SET_CPSR(GET_CPSR | I_MASK);            /* Mask IRQ */
        SET_CPSR(GET_CPSR & ~T_MASK);
        R15 = 0x18;                             /* IRQ Vector address */
        if ((COPRO_CTRL & COPRO_CTRL_MMU_EN) && (COPRO_CTRL & COPRO_CTRL_INTVEC_ADJUST)) R15 |= 0xFFFF0000;
        return;
    }

    // Prefetch Abort
    if (cpustate->pendingAbtP) {
        SwitchMode(cpustate, eARM7_MODE_ABT);             /* Set ABT mode so PC is saved to correct R14 bank */
        SET_REGISTER(cpustate, 14, pc - 4 + 4);                   /* save PC to R14 */
        SET_REGISTER(cpustate, SPSR, cpsr);               /* Save current CPSR */
        SET_CPSR(GET_CPSR | I_MASK);            /* Mask IRQ */
        SET_CPSR(GET_CPSR & ~T_MASK);
        R15 = 0x0c;                             /* IRQ Vector address */
        if ((COPRO_CTRL & COPRO_CTRL_MMU_EN) && (COPRO_CTRL & COPRO_CTRL_INTVEC_ADJUST)) R15 |= 0xFFFF0000;
        cpustate->pendingAbtP = 0;
        return;
    }

    // Undefined instruction
    if (cpustate->pendingUnd) {
        SwitchMode(cpustate, eARM7_MODE_UND);             /* Set UND mode so PC is saved to correct R14 bank */
        if (T_IS_SET(GET_CPSR))
        {
                SET_REGISTER(cpustate, 14, pc - 4 + 2);         /* save PC to R14 */
        }
        else
        {
                SET_REGISTER(cpustate, 14, pc - 4 + 4 - 4);           /* save PC to R14 */
        }
        SET_REGISTER(cpustate, SPSR, cpsr);               /* Save current CPSR */
        SET_CPSR(GET_CPSR | I_MASK);            /* Mask IRQ */
        SET_CPSR(GET_CPSR & ~T_MASK);
        R15 = 0x04;                             /* IRQ Vector address */
        if ((COPRO_CTRL & COPRO_CTRL_MMU_EN) && (COPRO_CTRL & COPRO_CTRL_INTVEC_ADJUST)) R15 |= 0xFFFF0000;
        cpustate->pendingUnd = 0;
        return;
    }

    // Software Interrupt
    if (cpustate->pendingSwi) {
        SwitchMode(cpustate, eARM7_MODE_SVC);             /* Set SVC mode so PC is saved to correct R14 bank */
        // compensate for prefetch (should this also be done for normal IRQ?)
        if (T_IS_SET(GET_CPSR))
        {
                SET_REGISTER(cpustate, 14, pc - 4 + 2);         /* save PC to R14 */
        }
        else
        {
                SET_REGISTER(cpustate, 14, pc - 4 + 4);           /* save PC to R14 */
        }
        SET_REGISTER(cpustate, SPSR, cpsr);               /* Save current CPSR */
        SET_CPSR(GET_CPSR | I_MASK);            /* Mask IRQ */
        SET_CPSR(GET_CPSR & ~T_MASK);           /* Go to ARM mode */
        R15 = 0x08;                             /* Jump to the SWI vector */
        if ((COPRO_CTRL & COPRO_CTRL_MMU_EN) && (COPRO_CTRL & COPRO_CTRL_INTVEC_ADJUST)) R15 |= 0xFFFF0000;
        cpustate->pendingSwi = 0;
        return;
    }
}

// CPU - SET IRQ LINE
static void arm7_core_set_irq_line(arm_state *cpustate, int irqline, int state)
{
    switch (irqline) {

    case ARM7_IRQ_LINE: /* IRQ */
        cpustate->pendingIrq = state & 1;
        break;

    case ARM7_FIRQ_LINE: /* FIRQ */
        cpustate->pendingFiq = state & 1;
        break;

    case ARM7_ABORT_EXCEPTION:
        cpustate->pendingAbtD = state & 1;
        break;
    case ARM7_ABORT_PREFETCH_EXCEPTION:
        cpustate->pendingAbtP = state & 1;
        break;

    case ARM7_UNDEFINE_EXCEPTION:
        cpustate->pendingUnd = state & 1;
        break;
    }

    ARM7_CHECKIRQ;
}

/***************************************************************************
 *                            OPCODE HANDLING
 ***************************************************************************/

// Co-Processor Data Operation
static void HandleCoProcDO(arm_state *cpustate, UINT32 insn)
{
    // This instruction simply instructs the co-processor to do something, no data is returned to ARM7 core
    if (arm7_coproc_do_callback)
        arm7_coproc_do_callback(cpustate->device, insn, 0, 0);    // simply pass entire opcode to callback - since data format is actually dependent on co-proc implementation
    else
        LOG(("%08x: Co-Processor Data Operation executed, but no callback defined!\n", R15));
}

// Co-Processor Register Transfer - To/From Arm to Co-Proc
static void HandleCoProcRT(arm_state *cpustate, UINT32 insn)
{

    /* xxxx 1110 oooL nnnn dddd cccc ppp1 mmmm */

    // Load (MRC) data from Co-Proc to ARM7 register
    if (insn & 0x00100000)       // Bit 20 = Load or Store
    {
        if (arm7_coproc_rt_r_callback)
        {
            UINT32 res = arm7_coproc_rt_r_callback(cpustate->device, insn, 0);   // RT Read handler must parse opcode & return appropriate result
            SET_REGISTER(cpustate, (insn >> 12) & 0xf, res);
        }
        else
            LOG(("%08x: Co-Processor Register Transfer executed, but no RT Read callback defined!\n", R15));
    }
    // Store (MCR) data from ARM7 to Co-Proc register
    else
    {
        if (arm7_coproc_rt_w_callback)
            arm7_coproc_rt_w_callback(cpustate->device, insn, GET_REGISTER(cpustate, (insn >> 12) & 0xf), 0);
        else
            LOG(("%08x: Co-Processor Register Transfer executed, but no RT Write callback defined!\n", R15));
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
static void HandleCoProcDT(arm_state *cpustate, UINT32 insn)
{
    UINT32 rn = (insn >> 16) & 0xf;
    UINT32 rnv = GET_REGISTER(cpustate, rn);    // Get Address Value stored from Rn
    UINT32 ornv = rnv;                // Keep value of Rn
    UINT32 off = (insn & 0xff) << 2;  // Offset is << 2 according to manual
    UINT32 *prn = &ARM7REG(rn);       // Pointer to our register, so it can be changed in the callback

    // Pointers to read32/write32 functions
    void (*write32)(arm_state *cpustate, UINT32 addr, UINT32 data);
    UINT32 (*read32)(arm_state *cpustate, UINT32 addr);
    write32 = PTR_WRITE32;
    read32 = PTR_READ32;

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
        if (arm7_coproc_dt_r_callback)
            arm7_coproc_dt_r_callback(cpustate, insn, prn, read32);
        else
            LOG(("%08x: Co-Processer Data Transfer executed, but no READ callback defined!\n", R15));
    }
    // Store (STC) data from Co-Proc to ARM7 memory
    else
    {
        if (arm7_coproc_dt_w_callback)
            arm7_coproc_dt_w_callback(cpustate, insn, prn, write32);
        else
            LOG(("%08x: Co-Processer Data Transfer executed, but no WRITE callback defined!\n", R15));
    }

    // If writeback not used - ensure the original value of RN is restored in case co-proc callback changed value
    if ((insn & 0x200000) == 0)
        SET_REGISTER(cpustate, rn, ornv);
}

INLINE void HandleBranch(arm_state *cpustate, UINT32 insn)
{
    UINT32 off = (insn & INSN_BRANCH) << 2;

    /* Save PC into LR if this is a branch with link */
    if (insn & INSN_BL)
    {
        SET_REGISTER(cpustate, 14, R15 + 4);
    }

    /* Sign-extend the 24-bit offset in our calculations */
    if (off & 0x2000000u)
    {
        R15 -= ((~(off | 0xfc000000u)) + 1) - 8;
    }
    else
    {
        R15 += off + 8;
    }
}

static void HandleMemSingle(arm_state *cpustate, UINT32 insn)
{
    UINT32 rn, rnv, off, rd, rnv_old = 0;

    /* Fetch the offset */
    if (insn & INSN_I)
    {
        /* Register Shift */
        off = decodeShift(cpustate, insn, NULL);
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
            rnv = (GET_REGISTER(cpustate, rn) + off);
        }
        else
        {
            rnv = (GET_REGISTER(cpustate, rn) - off);
        }

        if (insn & INSN_SDT_W)
        {
            rnv_old = GET_REGISTER(cpustate, rn);
            SET_REGISTER(cpustate, rn, rnv);

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
            rnv = R15 + 8;
        }
        else
        {
            rnv = GET_REGISTER(cpustate, rn);
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
            if (cpustate->pendingAbtD == 0)
            {
                SET_REGISTER(cpustate, rd, data);
            }
        }
        else
        {
            UINT32 data = READ32(rnv);
            if (cpustate->pendingAbtD == 0)
            {
                if (rd == eR15)
                {
                    R15 = data - 4;
                    // LDR, PC takes 2S + 2N + 1I (5 total cycles)
                    ARM7_ICOUNT -= 2;
                }
                else
                {
                    SET_REGISTER(cpustate, rd, data);
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

            WRITE8(rnv, (UINT8) GET_REGISTER(cpustate, rd) & 0xffu);
        }
        else
        {
#if ARM7_DEBUG_CORE
                if (rd == eR15)
                    LOG(("Wrote R15 in 32bit mode\n"));
#endif

            //WRITE32(rnv, rd == eR15 ? R15 + 8 : GET_REGISTER(cpustate, rd));
            WRITE32(rnv, rd == eR15 ? R15 + 8 + 4 : GET_REGISTER(cpustate, rd)); // manual says STR rd = PC, +12
        }
        // Store takes only 2 N Cycles, so add + 1
        ARM7_ICOUNT += 1;
    }

	if (cpustate->pendingAbtD != 0)
	{
		if ((insn & INSN_SDT_P) && (insn & INSN_SDT_W))
		{
			SET_REGISTER(cpustate, rn, rnv_old);
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
                SET_REGISTER(cpustate, rn, GET_REGISTER(cpustate, rd));
                // todo: check for offs... ?
            }
            else {

                if ((insn & INSN_SDT_W) != 0)
                    LOG(("%08x:  RegisterWritebackIncrement %d %d %d\n", R15, (insn & INSN_SDT_P) != 0, (insn & INSN_SDT_W) != 0, (insn & INSN_SDT_U) != 0));

                SET_REGISTER(cpustate, rn, (rnv + off));
            }
        }
        else
        {
            /* Writeback is applied in pipeline, before value is read from mem,
                so writeback is effectively ignored */
            if (rd == rn) {
                SET_REGISTER(cpustate, rn, GET_REGISTER(cpustate, rd));
            }
            else {
                SET_REGISTER(cpustate, rn, (rnv - off));

                if ((insn & INSN_SDT_W) != 0)
                    LOG(("%08x:  RegisterWritebackDecrement %d %d %d\n", R15, (insn & INSN_SDT_P) != 0, (insn & INSN_SDT_W) != 0, (insn & INSN_SDT_U) != 0));
            }
        }
    }

	}

//  ARM7_CHECKIRQ

} /* HandleMemSingle */

static void HandleHalfWordDT(arm_state *cpustate, UINT32 insn)
{
    UINT32 rn, rnv, off, rd, rnv_old = 0;

    // Immediate or Register Offset?
    if (insn & 0x400000) {               // Bit 22 - 1 = immediate, 0 = register
        // imm. value in high nibble (bits 8-11) and lo nibble (bit 0-3)
        off = (((insn >> 8) & 0x0f) << 4) | (insn & 0x0f);
    }
    else {
        // register
        off = GET_REGISTER(cpustate, insn & 0x0f);
    }

    /* Calculate Rn, accounting for PC */
    rn = (insn & INSN_RN) >> INSN_RN_SHIFT;

    if (insn & INSN_SDT_P)
    {
        /* Pre-indexed addressing */
        if (insn & INSN_SDT_U)
        {
            rnv = (GET_REGISTER(cpustate, rn) + off);
        }
        else
        {
            rnv = (GET_REGISTER(cpustate, rn) - off);
        }

        if (insn & INSN_SDT_W)
        {
            rnv_old = GET_REGISTER(cpustate, rn);
            SET_REGISTER(cpustate, rn, rnv);

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
            rnv = GET_REGISTER(cpustate, rn);
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
            UINT32 newval = 0;

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

			if (cpustate->pendingAbtD == 0)
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
                SET_REGISTER(cpustate, rd, newval);
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

			if (cpustate->pendingAbtD == 0)
			{

            if (rd == eR15)
            {
                R15 = newval + 8;
            }
            else
            {
                SET_REGISTER(cpustate, rd, newval);
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
    	if ((insn & 0x60) == 0x40)	// LDRD
	{
		SET_REGISTER(cpustate, rd, READ32(rnv));
		SET_REGISTER(cpustate, rd+1, READ32(rnv+4));
                R15 += 4;
	}
    	else if ((insn & 0x60) == 0x60)	// STRD
	{
		WRITE32(rnv, GET_REGISTER(cpustate, rd));
		WRITE32(rnv+4, GET_REGISTER(cpustate, rd+1));
                R15 += 4;
	}
	else
	{
	        // WRITE16(rnv, rd == eR15 ? R15 + 8 : GET_REGISTER(cpustate, rd));
	        WRITE16(rnv, rd == eR15 ? R15 + 8 + 4 : GET_REGISTER(cpustate, rd)); // manual says STR RD=PC, +12 of address

			// if R15 is not increased then e.g. "STRH R10, [R15,#$10]" will be executed over and over again		        
			//if (rn != eR15)
	        R15 += 4;

	        // STRH takes 2 cycles, so we add + 1
	        ARM7_ICOUNT += 1;
	}
    }

	if (cpustate->pendingAbtD != 0)
	{
		if ((insn & INSN_SDT_P) && (insn & INSN_SDT_W))
		{
			SET_REGISTER(cpustate, rn, rnv_old);
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
                SET_REGISTER(cpustate, rn, GET_REGISTER(cpustate, rd));
                // todo: check for offs... ?
            }
            else {

                if ((insn & INSN_SDT_W) != 0)
                    LOG(("%08x:  RegisterWritebackIncrement %d %d %d\n", R15, (insn & INSN_SDT_P) != 0, (insn & INSN_SDT_W) != 0, (insn & INSN_SDT_U) != 0));

                SET_REGISTER(cpustate, rn, (rnv + off));
            }
        }
        else
        {
            /* Writeback is applied in pipeline, before value is read from mem,
                so writeback is effectively ignored */
            if (rd == rn) {
                SET_REGISTER(cpustate, rn, GET_REGISTER(cpustate, rd));
            }
            else {
                SET_REGISTER(cpustate, rn, (rnv - off));

                if ((insn & INSN_SDT_W) != 0)
                    LOG(("%08x:  RegisterWritebackDecrement %d %d %d\n", R15, (insn & INSN_SDT_P) != 0, (insn & INSN_SDT_W) != 0, (insn & INSN_SDT_U) != 0));
            }
        }
    }
    
    }

}

static void HandleSwap(arm_state *cpustate, UINT32 insn)
{
    UINT32 rn, rm, rd, tmp;

    rn = GET_REGISTER(cpustate, (insn >> 16) & 0xf);  // reg. w/read address
    rm = GET_REGISTER(cpustate, insn & 0xf);          // reg. w/write address
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
        SET_REGISTER(cpustate, rd, tmp);
    }
    else
    {
        tmp = READ32(rn);
        WRITE32(rn, rm);
        SET_REGISTER(cpustate, rd, tmp);
    }

    R15 += 4;
    // Instruction takes 1S+2N+1I cycles - so we subtract one more..
    ARM7_ICOUNT -= 1;
}

static void HandlePSRTransfer(arm_state *cpustate, UINT32 insn)
{
    int reg = (insn & 0x400000) ? SPSR : eCPSR; // Either CPSR or SPSR
    UINT32 newval, val = 0;
    int oldmode = GET_CPSR & MODE_FLAG;

    // get old value of CPSR/SPSR
    newval = GET_REGISTER(cpustate, reg);

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
            val = GET_REGISTER(cpustate, insn & 0x0f);
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

        // force valid mode
        newval |= 0x10;

        // Update the Register
        SET_REGISTER(cpustate, reg, newval);

        // Switch to new mode if changed
        if ((newval & MODE_FLAG) != oldmode)
            SwitchMode(cpustate, GET_MODE);

    }
    // MRS (bit 21 clear) - Copy CPSR or SPSR to specified Register
    else
    {
        SET_REGISTER(cpustate, (insn >> 12)& 0x0f, GET_REGISTER(cpustate, reg));
    }
}

static void HandleALU(arm_state *cpustate, UINT32 insn)
{
    UINT32 op2, sc = 0, rd, rn, opcode;
    UINT32 by, rdn;

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
        op2 = decodeShift(cpustate, insn, (insn & INSN_S) ? &sc : NULL);

        // LD TODO sc will always be 0 if this applies
        if (!(insn & INSN_S))
            sc = 0;
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
            rn = R15 + 8;
        }
        else
        {
            rn = GET_REGISTER(cpustate, rn);
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
            R15 = rd;
        }
        else
        {
            // Rd = 15 and S Flag IS set, Result is placed in R15, and current mode SPSR moved to CPSR
            if (rdn == eR15) 
			{
	            // When Rd is R15 and the S flag is set the result of the operation is placed in R15 and the SPSR corresponding to
	            // the current mode is moved to the CPSR. This allows state changes which automatically restore both PC and
	            // CPSR. --> This form of instruction should not be used in User mode. <--
				if (GET_MODE != eARM7_MODE_USER)
				{
					   // Update CPSR from SPSR
					   SET_CPSR(GET_REGISTER(cpustate, SPSR));
					   SwitchMode(cpustate, GET_MODE);
				}
                R15 = rd;

                /* IRQ masks may have changed in this instruction */
//              ARM7_CHECKIRQ;
            }
            else
                /* S Flag is set - Write results to register & update CPSR (which was already handled using HandleALU flag macros) */
                SET_REGISTER(cpustate, rdn, rd);
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
            R15 = rd;

            /* IRQ masks may have changed in this instruction */
//          ARM7_CHECKIRQ;
        }
        else
        {
#if ARM7_DEBUG_CORE
                LOG(("%08x: TST class on R15 no s bit set\n", R15));
#endif
        }
    }
}

static void HandleMul(arm_state *cpustate, UINT32 insn)
{
    UINT32 r;

    /* Do the basic multiply of Rm and Rs */
    r = GET_REGISTER(cpustate, insn & INSN_MUL_RM) *
        GET_REGISTER(cpustate, (insn & INSN_MUL_RS) >> INSN_MUL_RS_SHIFT);

#if ARM7_DEBUG_CORE
    if ((insn & INSN_MUL_RM) == 0xf ||
        ((insn & INSN_MUL_RS) >> INSN_MUL_RS_SHIFT) == 0xf ||
        ((insn & INSN_MUL_RN) >> INSN_MUL_RN_SHIFT) == 0xf)
        LOG(("%08x:  R15 used in mult\n", R15));
#endif

    /* Add on Rn if this is a MLA */
    if (insn & INSN_MUL_A)
    {
        r += GET_REGISTER(cpustate, (insn & INSN_MUL_RN) >> INSN_MUL_RN_SHIFT);
    }

    /* Write the result */
    SET_REGISTER(cpustate, (insn & INSN_MUL_RD) >> INSN_MUL_RD_SHIFT, r);

    /* Set N and Z if asked */
    if (insn & INSN_S)
    {
        SET_CPSR((GET_CPSR & ~(N_MASK | Z_MASK)) | HandleALUNZFlags(r));
    }
}

// todo: add proper cycle counts
static void HandleSMulLong(arm_state *cpustate, UINT32 insn)
{
    INT32 rm, rs;
    UINT32 rhi, rlo;
    INT64 res = 0;

    rm  = (INT32)GET_REGISTER(cpustate, insn & 0xf);
    rs  = (INT32)GET_REGISTER(cpustate, ((insn >> 8) & 0xf));
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
        INT64 acum = (INT64)((((INT64)(GET_REGISTER(cpustate, rhi))) << 32) | GET_REGISTER(cpustate, rlo));
        res += acum;
    }

    /* Write the result (upper dword goes to RHi, lower to RLo) */
    SET_REGISTER(cpustate, rhi, res >> 32);
    SET_REGISTER(cpustate, rlo, res & 0xFFFFFFFF);

    /* Set N and Z if asked */
    if (insn & INSN_S)
    {
        SET_CPSR((GET_CPSR & ~(N_MASK | Z_MASK)) | HandleLongALUNZFlags(res));
    }
}

// todo: add proper cycle counts
static void HandleUMulLong(arm_state *cpustate, UINT32 insn)
{
    UINT32 rm, rs;
    UINT32 rhi, rlo;
    UINT64 res = 0;

    rm  = (INT32)GET_REGISTER(cpustate, insn & 0xf);
    rs  = (INT32)GET_REGISTER(cpustate, ((insn >> 8) & 0xf));
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
        UINT64 acum = (UINT64)((((UINT64)(GET_REGISTER(cpustate, rhi))) << 32) | GET_REGISTER(cpustate, rlo));
        res += acum;
    }

    /* Write the result (upper dword goes to RHi, lower to RLo) */
    SET_REGISTER(cpustate, rhi, res >> 32);
    SET_REGISTER(cpustate, rlo, res & 0xFFFFFFFF);

    /* Set N and Z if asked */
    if (insn & INSN_S)
    {
        SET_CPSR((GET_CPSR & ~(N_MASK | Z_MASK)) | HandleLongALUNZFlags(res));
    }
}

static void HandleMemBlock(arm_state *cpustate, UINT32 insn)
{
    UINT32 rb = (insn & INSN_RN) >> INSN_RN_SHIFT;
    UINT32 rbp = GET_REGISTER(cpustate, rb);
    int result;

#if ARM7_DEBUG_CORE
    if (rbp & 3)
        LOG(("%08x: Unaligned Mem Transfer @ %08x\n", R15, rbp));
#endif

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
                // set to user mode - then do the transfer, and set back
                int curmode = GET_MODE;
                SwitchMode(cpustate, eARM7_MODE_USER);
                LOG(("%08x: User Bank Transfer not fully tested - please check if working properly!\n", R15));
                result = loadInc(cpustate, insn & 0xffff, rbp, insn & INSN_BDT_S);
                // todo - not sure if Writeback occurs on User registers also..
                SwitchMode(cpustate, curmode);
            }
            else
                result = loadInc(cpustate, insn & 0xffff, rbp, insn & INSN_BDT_S);

            if ((insn & INSN_BDT_W) && (cpustate->pendingAbtD == 0))
            {
#if ARM7_DEBUG_CORE
                    if (rb == 15)
                        LOG(("%08x:  Illegal LDRM writeback to r15\n", R15));
#endif
                SET_REGISTER(cpustate, rb, GET_REGISTER(cpustate, rb) + result * 4);
            }

            // R15 included? (NOTE: CPSR restore must occur LAST otherwise wrong registers restored!)
            if ((insn & 0x8000) && (cpustate->pendingAbtD == 0)) {
                R15 -= 4;     // SJE: I forget why i did this?
                // S - Flag Set? Signals transfer of current mode SPSR->CPSR
                if (insn & INSN_BDT_S)
				{
                    SET_CPSR(GET_REGISTER(cpustate, SPSR));
                    SwitchMode(cpustate, GET_MODE);
                }

				// LDM PC - takes 1 extra cycle
				ARM7_ICOUNT -= 1;
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
                int curmode = GET_MODE;
                SwitchMode(cpustate, eARM7_MODE_USER);
                LOG(("%08x: User Bank Transfer not fully tested - please check if working properly!\n", R15));
                result = loadDec(cpustate, insn & 0xffff, rbp, insn & INSN_BDT_S);
                // todo - not sure if Writeback occurs on User registers also..
                SwitchMode(cpustate, curmode);
            }
            else
                result = loadDec(cpustate, insn & 0xffff, rbp, insn & INSN_BDT_S);

            if ((insn & INSN_BDT_W) && (cpustate->pendingAbtD == 0))
            {
                if (rb == 0xf)
                    LOG(("%08x:  Illegal LDRM writeback to r15\n", R15));
                SET_REGISTER(cpustate, rb, GET_REGISTER(cpustate, rb)-result*4);
            }

            // R15 included? (NOTE: CPSR restore must occur LAST otherwise wrong registers restored!)
            if ((insn & 0x8000) && (cpustate->pendingAbtD == 0)) {
                R15 -= 4;     // SJE: I forget why i did this?
                // S - Flag Set? Signals transfer of current mode SPSR->CPSR
                if (insn & INSN_BDT_S)
				{
                    SET_CPSR(GET_REGISTER(cpustate, SPSR));
                    SwitchMode(cpustate, GET_MODE);
                }
                // LDM PC - takes 1 extra cycle
                ARM7_ICOUNT -= 1;
            }

            // LDM (NO PC) takes nS + 1n + 1I cycles (n = # of register transfers)
            ARM7_ICOUNT -= result + 1 + 1;
        }
    } /* Loading */
    else
    {
        /* Storing */
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
                int curmode = GET_MODE;
                SwitchMode(cpustate, eARM7_MODE_USER);
                LOG(("%08x: User Bank Transfer not fully tested - please check if working properly!\n", R15));
                result = storeInc(cpustate, insn & 0xffff, rbp);
                // todo - not sure if Writeback occurs on User registers also..
                SwitchMode(cpustate, curmode);
            }
            else
                result = storeInc(cpustate, insn & 0xffff, rbp);

            if ((insn & INSN_BDT_W) && (cpustate->pendingAbtD == 0))
            {
                SET_REGISTER(cpustate, rb, GET_REGISTER(cpustate, rb) + result * 4);
            }
        }
        else
        {
            /* Decrementing */
            if (!(insn & INSN_BDT_P))
            {
                rbp = rbp - (-4);
            }

            // S Flag Set = User Bank Transfer
            if (insn & INSN_BDT_S)
            {
                // set to user mode - then do the transfer, and set back
                int curmode = GET_MODE;
                SwitchMode(cpustate, eARM7_MODE_USER);
                LOG(("%08x: User Bank Transfer not fully tested - please check if working properly!\n", R15));
                result = storeDec(cpustate, insn & 0xffff, rbp);
                // todo - not sure if Writeback occurs on User registers also..
                SwitchMode(cpustate, curmode);
            }
            else
                result = storeDec(cpustate, insn & 0xffff, rbp);

            if ((insn & INSN_BDT_W) && (cpustate->pendingAbtD == 0))
            {
                SET_REGISTER(cpustate, rb, GET_REGISTER(cpustate, rb) - result * 4);
            }
        }
        if (insn & (1 << eR15))
            R15 -= 12;

        // STM takes (n+1)S+2N+1I cycles (n = # of register transfers)
        ARM7_ICOUNT -= (result + 1) + 2 + 1;
    }

    // We will specify the cycle count for each case, so remove the -3 that occurs at the end
    ARM7_ICOUNT += 3;

} /* HandleMemBlock */
