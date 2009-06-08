/*****************************************************************************
 *
 *   arm7.c
 *   Portable ARM7TDMI CPU Emulator
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
 *
 *****************************************************************************/

/******************************************************************************
 *  Notes:

    ** This is a plain vanilla implementation of an ARM7 cpu which incorporates my ARM7 core.
       It can be used as is, or used to demonstrate how to utilize the arm7 core to create a cpu
       that uses the core, since there are numerous different mcu packages that incorporate an arm7 core.

       See the notes in the arm7core.c file itself regarding issues/limitations of the arm7 core.
    **
*****************************************************************************/
#include "arm7.h"
#include "debugger.h"
#include "arm7core.h"   //include arm7 core

/* Example for showing how Co-Proc functions work */
#define TEST_COPROC_FUNCS 1

/* prototypes */
#if TEST_COPROC_FUNCS
static WRITE32_HANDLER(test_do_callback);
static READ32_HANDLER(test_rt_r_callback);
static WRITE32_HANDLER(test_rt_w_callback);
static void test_dt_r_callback(arm_state *cpustate, UINT32 insn, UINT32 *prn, UINT32 (*read32)(arm_state *cpustate, UINT32 addr));
static void test_dt_w_callback(arm_state *cpustate, UINT32 insn, UINT32 *prn, void (*write32)(arm_state *cpustate, UINT32 addr, UINT32 data));
#endif

/* Macros that can be re-defined for custom cpu implementations - The core expects these to be defined */
/* In this case, we are using the default arm7 handlers (supplied by the core)
   - but simply changes these and define your own if needed for cpu implementation specific needs */
#define READ8(addr)         arm7_cpu_read8(cpustate, addr)
#define WRITE8(addr,data)   arm7_cpu_write8(cpustate, addr,data)
#define READ16(addr)        arm7_cpu_read16(cpustate, addr)
#define WRITE16(addr,data)  arm7_cpu_write16(cpustate, addr,data)
#define READ32(addr)        arm7_cpu_read32(cpustate, addr)
#define WRITE32(addr,data)  arm7_cpu_write32(cpustate, addr,data)
#define PTR_READ32          &arm7_cpu_read32
#define PTR_WRITE32         &arm7_cpu_write32

/* Macros that need to be defined according to the cpu implementation specific need */
#define ARM7REG(reg)        cpustate->sArmRegister[reg]
#define ARM7_ICOUNT         cpustate->iCount

INLINE arm_state *get_safe_token(const device_config *device)
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == CPU);
	assert(cpu_get_type(device) == CPU_ARM7);
	return (arm_state *)device->token;
}

/* include the arm7 core */
#include "arm7core.c"

/***************************************************************************
 * CPU SPECIFIC IMPLEMENTATIONS
 **************************************************************************/
static CPU_INIT( arm7 )
{
	arm_state *cpustate = get_safe_token(device);

	// must call core
	arm7_core_init(device, "arm7");

	cpustate->irq_callback = irqcallback;
	cpustate->device = device;
	cpustate->program = memory_find_address_space(device, ADDRESS_SPACE_PROGRAM);

#if TEST_COPROC_FUNCS
	// setup co-proc callbacks example
	arm7_coproc_do_callback = test_do_callback;
	arm7_coproc_rt_r_callback = test_rt_r_callback;
	arm7_coproc_rt_w_callback = test_rt_w_callback;
	arm7_coproc_dt_r_callback = test_dt_r_callback;
	arm7_coproc_dt_w_callback = test_dt_w_callback;
#endif
}

static CPU_RESET( arm7 )
{
	// must call core reset
	arm7_core_reset(device);
}

static CPU_EXIT( arm7 )
{
	/* nothing to do here */
}

static CPU_EXECUTE( arm7 )
{
/* include the arm7 core execute code */
#include "arm7exec.c"
}


static void set_irq_line(arm_state *cpustate, int irqline, int state)
{
    // must call core
    arm7_core_set_irq_line(cpustate, irqline, state);
}

static CPU_DISASSEMBLE( arm7 )
{
    arm_state *cpustate = get_safe_token(device);

    if (T_IS_SET(GET_CPSR))
    {
        return thumb_disasm(buffer, pc, oprom[0] | (oprom[1] << 8)) | 2;
    }
    else
    {
        return arm7_disasm(buffer, pc, oprom[0] | (oprom[1] << 8) | (oprom[2] << 16) | (oprom[3] << 24)) | 4;
    }
}


/**************************************************************************
 * Generic set_info
 **************************************************************************/

static CPU_SET_INFO( arm7 )
{
    arm_state *cpustate = get_safe_token(device);

    switch (state)
    {
        /* --- the following bits of info are set as 64-bit signed integers --- */

        /* interrupt lines/exceptions */
        case CPUINFO_INT_INPUT_STATE + ARM7_IRQ_LINE:                   set_irq_line(cpustate, ARM7_IRQ_LINE, info->i); break;
        case CPUINFO_INT_INPUT_STATE + ARM7_FIRQ_LINE:                  set_irq_line(cpustate, ARM7_FIRQ_LINE, info->i); break;
        case CPUINFO_INT_INPUT_STATE + ARM7_ABORT_EXCEPTION:            set_irq_line(cpustate, ARM7_ABORT_EXCEPTION, info->i); break;
        case CPUINFO_INT_INPUT_STATE + ARM7_ABORT_PREFETCH_EXCEPTION:   set_irq_line(cpustate, ARM7_ABORT_PREFETCH_EXCEPTION, info->i); break;
        case CPUINFO_INT_INPUT_STATE + ARM7_UNDEFINE_EXCEPTION:         set_irq_line(cpustate, ARM7_UNDEFINE_EXCEPTION, info->i); break;

        /* registers shared by all operating modes */
        case CPUINFO_INT_REGISTER + ARM7_R0:            ARM7REG( 0) = info->i;                  break;
        case CPUINFO_INT_REGISTER + ARM7_R1:            ARM7REG( 1) = info->i;                  break;
        case CPUINFO_INT_REGISTER + ARM7_R2:            ARM7REG( 2) = info->i;                  break;
        case CPUINFO_INT_REGISTER + ARM7_R3:            ARM7REG( 3) = info->i;                  break;
        case CPUINFO_INT_REGISTER + ARM7_R4:            ARM7REG( 4) = info->i;                  break;
        case CPUINFO_INT_REGISTER + ARM7_R5:            ARM7REG( 5) = info->i;                  break;
        case CPUINFO_INT_REGISTER + ARM7_R6:            ARM7REG( 6) = info->i;                  break;
        case CPUINFO_INT_REGISTER + ARM7_R7:            ARM7REG( 7) = info->i;                  break;
        case CPUINFO_INT_REGISTER + ARM7_R8:            ARM7REG( 8) = info->i;                  break;
        case CPUINFO_INT_REGISTER + ARM7_R9:            ARM7REG( 9) = info->i;                  break;
        case CPUINFO_INT_REGISTER + ARM7_R10:           ARM7REG(10) = info->i;                  break;
        case CPUINFO_INT_REGISTER + ARM7_R11:           ARM7REG(11) = info->i;                  break;
        case CPUINFO_INT_REGISTER + ARM7_R12:           ARM7REG(12) = info->i;                  break;
        case CPUINFO_INT_REGISTER + ARM7_R13:           ARM7REG(13) = info->i;                  break;
        case CPUINFO_INT_REGISTER + ARM7_R14:           ARM7REG(14) = info->i;                  break;
        case CPUINFO_INT_REGISTER + ARM7_R15:           ARM7REG(15) = info->i;                  break;
        case CPUINFO_INT_REGISTER + ARM7_CPSR:          SET_CPSR(info->i);                      break;

        case CPUINFO_INT_PC:
        case CPUINFO_INT_REGISTER + ARM7_PC:            R15 = info->i;                          break;
        case CPUINFO_INT_SP:                            SetRegister(cpustate, 13,info->i);                break;

        /* FIRQ Mode Shadowed Registers */
        case CPUINFO_INT_REGISTER + ARM7_FR8:           ARM7REG(eR8_FIQ)  = info->i;            break;
        case CPUINFO_INT_REGISTER + ARM7_FR9:           ARM7REG(eR9_FIQ)  = info->i;            break;
        case CPUINFO_INT_REGISTER + ARM7_FR10:          ARM7REG(eR10_FIQ) = info->i;            break;
        case CPUINFO_INT_REGISTER + ARM7_FR11:          ARM7REG(eR11_FIQ) = info->i;            break;
        case CPUINFO_INT_REGISTER + ARM7_FR12:          ARM7REG(eR12_FIQ) = info->i;            break;
        case CPUINFO_INT_REGISTER + ARM7_FR13:          ARM7REG(eR13_FIQ) = info->i;            break;
        case CPUINFO_INT_REGISTER + ARM7_FR14:          ARM7REG(eR14_FIQ) = info->i;            break;
        case CPUINFO_INT_REGISTER + ARM7_FSPSR:         ARM7REG(eSPSR_FIQ) = info->i;           break;

        /* IRQ Mode Shadowed Registers */
        case CPUINFO_INT_REGISTER + ARM7_IR13:          ARM7REG(eR13_IRQ) = info->i;            break;
        case CPUINFO_INT_REGISTER + ARM7_IR14:          ARM7REG(eR14_IRQ) = info->i;            break;
        case CPUINFO_INT_REGISTER + ARM7_ISPSR:         ARM7REG(eSPSR_IRQ) = info->i;           break;

        /* Supervisor Mode Shadowed Registers */
        case CPUINFO_INT_REGISTER + ARM7_SR13:          ARM7REG(eR13_SVC) = info->i;            break;
        case CPUINFO_INT_REGISTER + ARM7_SR14:          ARM7REG(eR14_SVC) = info->i;            break;
        case CPUINFO_INT_REGISTER + ARM7_SSPSR:         ARM7REG(eSPSR_SVC) = info->i;           break;

        /* Abort Mode Shadowed Registers */
        case CPUINFO_INT_REGISTER + ARM7_AR13:          ARM7REG(eR13_ABT) = info->i;            break;
        case CPUINFO_INT_REGISTER + ARM7_AR14:          ARM7REG(eR14_ABT) = info->i;            break;
        case CPUINFO_INT_REGISTER + ARM7_ASPSR:         ARM7REG(eSPSR_ABT) = info->i;           break;

        /* Undefined Mode Shadowed Registers */
        case CPUINFO_INT_REGISTER + ARM7_UR13:          ARM7REG(eR13_UND) = info->i;            break;
        case CPUINFO_INT_REGISTER + ARM7_UR14:          ARM7REG(eR14_UND) = info->i;            break;
        case CPUINFO_INT_REGISTER + ARM7_USPSR:         ARM7REG(eSPSR_UND) = info->i;           break;
    }
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

CPU_GET_INFO( arm7 )
{
    arm_state *cpustate = (device != NULL && device->token != NULL) ? get_safe_token(device) : NULL;

    switch (state)
    {
        /* --- the following bits of info are returned as 64-bit signed integers --- */

        /* cpu implementation data */
        case CPUINFO_INT_CONTEXT_SIZE:                  info->i = sizeof(arm_state);                 break;
        case CPUINFO_INT_INPUT_LINES:                   info->i = ARM7_NUM_LINES;               break;
        case CPUINFO_INT_DEFAULT_IRQ_VECTOR:            info->i = 0;                            break;
        case CPUINFO_INT_ENDIANNESS:                    info->i = ENDIANNESS_LITTLE;                    break;
        case CPUINFO_INT_CLOCK_MULTIPLIER:              info->i = 1;                            break;
        case CPUINFO_INT_CLOCK_DIVIDER:                 info->i = 1;                            break;
        case CPUINFO_INT_MIN_INSTRUCTION_BYTES:         info->i = 2;                            break;
        case CPUINFO_INT_MAX_INSTRUCTION_BYTES:         info->i = 4;                            break;
        case CPUINFO_INT_MIN_CYCLES:                    info->i = 3;                            break;
        case CPUINFO_INT_MAX_CYCLES:                    info->i = 4;                            break;

        case CPUINFO_INT_DATABUS_WIDTH_PROGRAM: info->i = 32;                   break;
        case CPUINFO_INT_ADDRBUS_WIDTH_PROGRAM: info->i = 32;                   break;
        case CPUINFO_INT_ADDRBUS_SHIFT_PROGRAM: info->i = 0;                    break;
        case CPUINFO_INT_DATABUS_WIDTH_DATA:    info->i = 0;                    break;
        case CPUINFO_INT_ADDRBUS_WIDTH_DATA:    info->i = 0;                    break;
        case CPUINFO_INT_ADDRBUS_SHIFT_DATA:    info->i = 0;                    break;
        case CPUINFO_INT_DATABUS_WIDTH_IO:      info->i = 0;                    break;
        case CPUINFO_INT_ADDRBUS_WIDTH_IO:      info->i = 0;                    break;
        case CPUINFO_INT_ADDRBUS_SHIFT_IO:      info->i = 0;                    break;

        /* interrupt lines/exceptions */
        case CPUINFO_INT_INPUT_STATE + ARM7_IRQ_LINE:                   info->i = cpustate->pendingIrq; break;
        case CPUINFO_INT_INPUT_STATE + ARM7_FIRQ_LINE:                  info->i = cpustate->pendingFiq; break;
        case CPUINFO_INT_INPUT_STATE + ARM7_ABORT_EXCEPTION:            info->i = cpustate->pendingAbtD; break;
        case CPUINFO_INT_INPUT_STATE + ARM7_ABORT_PREFETCH_EXCEPTION:   info->i = cpustate->pendingAbtP; break;
        case CPUINFO_INT_INPUT_STATE + ARM7_UNDEFINE_EXCEPTION:         info->i = cpustate->pendingUnd; break;

        /* registers shared by all operating modes */
        case CPUINFO_INT_REGISTER + ARM7_R0:    info->i = ARM7REG( 0);                          break;
        case CPUINFO_INT_REGISTER + ARM7_R1:    info->i = ARM7REG( 1);                          break;
        case CPUINFO_INT_REGISTER + ARM7_R2:    info->i = ARM7REG( 2);                          break;
        case CPUINFO_INT_REGISTER + ARM7_R3:    info->i = ARM7REG( 3);                          break;
        case CPUINFO_INT_REGISTER + ARM7_R4:    info->i = ARM7REG( 4);                          break;
        case CPUINFO_INT_REGISTER + ARM7_R5:    info->i = ARM7REG( 5);                          break;
        case CPUINFO_INT_REGISTER + ARM7_R6:    info->i = ARM7REG( 6);                          break;
        case CPUINFO_INT_REGISTER + ARM7_R7:    info->i = ARM7REG( 7);                          break;
        case CPUINFO_INT_REGISTER + ARM7_R8:    info->i = ARM7REG( 8);                          break;
        case CPUINFO_INT_REGISTER + ARM7_R9:    info->i = ARM7REG( 9);                          break;
        case CPUINFO_INT_REGISTER + ARM7_R10:   info->i = ARM7REG(10);                          break;
        case CPUINFO_INT_REGISTER + ARM7_R11:   info->i = ARM7REG(11);                          break;
        case CPUINFO_INT_REGISTER + ARM7_R12:   info->i = ARM7REG(12);                          break;
        case CPUINFO_INT_REGISTER + ARM7_R13:   info->i = ARM7REG(13);                          break;
        case CPUINFO_INT_REGISTER + ARM7_R14:   info->i = ARM7REG(14);                          break;
        case CPUINFO_INT_REGISTER + ARM7_R15:   info->i = ARM7REG(15);                          break;

        case CPUINFO_INT_PREVIOUSPC:            info->i = 0;    /* not implemented */           break;
        case CPUINFO_INT_PC:
        case CPUINFO_INT_REGISTER + ARM7_PC:    info->i = R15;                                  break;
        case CPUINFO_INT_SP:                    info->i = GetRegister(cpustate, 13);            break;

        /* FIRQ Mode Shadowed Registers */
        case CPUINFO_INT_REGISTER + ARM7_FR8:   info->i = ARM7REG(eR8_FIQ);                     break;
        case CPUINFO_INT_REGISTER + ARM7_FR9:   info->i = ARM7REG(eR9_FIQ);                     break;
        case CPUINFO_INT_REGISTER + ARM7_FR10:  info->i = ARM7REG(eR10_FIQ);                    break;
        case CPUINFO_INT_REGISTER + ARM7_FR11:  info->i = ARM7REG(eR11_FIQ);                    break;
        case CPUINFO_INT_REGISTER + ARM7_FR12:  info->i = ARM7REG(eR12_FIQ);                    break;
        case CPUINFO_INT_REGISTER + ARM7_FR13:  info->i = ARM7REG(eR13_FIQ);                    break;
        case CPUINFO_INT_REGISTER + ARM7_FR14:  info->i = ARM7REG(eR14_FIQ);                    break;
        case CPUINFO_INT_REGISTER + ARM7_FSPSR: info->i = ARM7REG(eSPSR_FIQ);                   break;

        /* IRQ Mode Shadowed Registers */
        case CPUINFO_INT_REGISTER + ARM7_IR13:  info->i = ARM7REG(eR13_IRQ);                    break;
        case CPUINFO_INT_REGISTER + ARM7_IR14:  info->i = ARM7REG(eR14_IRQ);                    break;
        case CPUINFO_INT_REGISTER + ARM7_ISPSR: info->i = ARM7REG(eSPSR_IRQ);                   break;

        /* Supervisor Mode Shadowed Registers */
        case CPUINFO_INT_REGISTER + ARM7_SR13:  info->i = ARM7REG(eR13_SVC);                    break;
        case CPUINFO_INT_REGISTER + ARM7_SR14:  info->i = ARM7REG(eR14_SVC);                    break;
        case CPUINFO_INT_REGISTER + ARM7_SSPSR: info->i = ARM7REG(eSPSR_SVC);                   break;

        /* Abort Mode Shadowed Registers */
        case CPUINFO_INT_REGISTER + ARM7_AR13:  info->i = ARM7REG(eR13_ABT);                    break;
        case CPUINFO_INT_REGISTER + ARM7_AR14:  info->i = ARM7REG(eR14_ABT);                    break;
        case CPUINFO_INT_REGISTER + ARM7_ASPSR: info->i = ARM7REG(eSPSR_ABT);                   break;

        /* Undefined Mode Shadowed Registers */
        case CPUINFO_INT_REGISTER + ARM7_UR13:  info->i = ARM7REG(eR13_UND);                    break;
        case CPUINFO_INT_REGISTER + ARM7_UR14:  info->i = ARM7REG(eR14_UND);                    break;
        case CPUINFO_INT_REGISTER + ARM7_USPSR: info->i = ARM7REG(eSPSR_UND);                   break;

        /* --- the following bits of info are returned as pointers to data or functions --- */
        case CPUINFO_FCT_SET_INFO:              info->setinfo = CPU_SET_INFO_NAME(arm7);                  break;
        case CPUINFO_FCT_INIT:                  info->init = CPU_INIT_NAME(arm7);                         break;
        case CPUINFO_FCT_RESET:                 info->reset = CPU_RESET_NAME(arm7);                       break;
        case CPUINFO_FCT_EXIT:                  info->exit = CPU_EXIT_NAME(arm7);                         break;
        case CPUINFO_FCT_EXECUTE:               info->execute = CPU_EXECUTE_NAME(arm7);                   break;
        case CPUINFO_FCT_BURN:                  info->burn = NULL;                              break;
        case CPUINFO_FCT_DISASSEMBLE:           info->disassemble = CPU_DISASSEMBLE_NAME(arm7);                  break;
        case CPUINFO_PTR_INSTRUCTION_COUNTER:   info->icount = &ARM7_ICOUNT;                    break;

        /* --- the following bits of info are returned as NULL-terminated strings --- */
        case CPUINFO_STR_NAME:                  strcpy(info->s, "ARM7");                        break;
        case CPUINFO_STR_CORE_FAMILY:           strcpy(info->s, "Acorn Risc Machine");          break;
        case CPUINFO_STR_CORE_VERSION:          strcpy(info->s, "1.3");                         break;
        case CPUINFO_STR_CORE_FILE:             strcpy(info->s, __FILE__);                      break;
        case CPUINFO_STR_CORE_CREDITS:          strcpy(info->s, "Copyright Steve Ellenoff, sellenoff@hotmail.com"); break;

        case CPUINFO_STR_FLAGS:
            sprintf(info->s, "%c%c%c%c%c%c%c %s",
                    (ARM7REG(eCPSR) & N_MASK) ? 'N' : '-',
                    (ARM7REG(eCPSR) & Z_MASK) ? 'Z' : '-',
                    (ARM7REG(eCPSR) & C_MASK) ? 'C' : '-',
                    (ARM7REG(eCPSR) & V_MASK) ? 'V' : '-',
                    (ARM7REG(eCPSR) & I_MASK) ? 'I' : '-',
                    (ARM7REG(eCPSR) & F_MASK) ? 'F' : '-',
                    (ARM7REG(eCPSR) & T_MASK) ? 'T' : '-',
                    GetModeText(ARM7REG(eCPSR)));
        break;

        /* registers shared by all operating modes */
        case CPUINFO_STR_REGISTER + ARM7_PC:    sprintf(info->s, "PC  :%08x", R15);            break;
        case CPUINFO_STR_REGISTER + ARM7_R0:    sprintf(info->s, "R0  :%08x", ARM7REG( 0));    break;
        case CPUINFO_STR_REGISTER + ARM7_R1:    sprintf(info->s, "R1  :%08x", ARM7REG( 1));    break;
        case CPUINFO_STR_REGISTER + ARM7_R2:    sprintf(info->s, "R2  :%08x", ARM7REG( 2));    break;
        case CPUINFO_STR_REGISTER + ARM7_R3:    sprintf(info->s, "R3  :%08x", ARM7REG( 3));    break;
        case CPUINFO_STR_REGISTER + ARM7_R4:    sprintf(info->s, "R4  :%08x", ARM7REG( 4));    break;
        case CPUINFO_STR_REGISTER + ARM7_R5:    sprintf(info->s, "R5  :%08x", ARM7REG( 5));    break;
        case CPUINFO_STR_REGISTER + ARM7_R6:    sprintf(info->s, "R6  :%08x", ARM7REG( 6));    break;
        case CPUINFO_STR_REGISTER + ARM7_R7:    sprintf(info->s, "R7  :%08x", ARM7REG( 7));    break;
        case CPUINFO_STR_REGISTER + ARM7_R8:    sprintf(info->s, "R8  :%08x", ARM7REG( 8));    break;
        case CPUINFO_STR_REGISTER + ARM7_R9:    sprintf(info->s, "R9  :%08x", ARM7REG( 9));    break;
        case CPUINFO_STR_REGISTER + ARM7_R10:   sprintf(info->s, "R10 :%08x", ARM7REG(10));    break;
        case CPUINFO_STR_REGISTER + ARM7_R11:   sprintf(info->s, "R11 :%08x", ARM7REG(11));    break;
        case CPUINFO_STR_REGISTER + ARM7_R12:   sprintf(info->s, "R12 :%08x", ARM7REG(12));    break;
        case CPUINFO_STR_REGISTER + ARM7_R13:   sprintf(info->s, "R13 :%08x", ARM7REG(13));    break;
        case CPUINFO_STR_REGISTER + ARM7_R14:   sprintf(info->s, "R14 :%08x", ARM7REG(14));    break;
        case CPUINFO_STR_REGISTER + ARM7_R15:   sprintf(info->s, "R15 :%08x", ARM7REG(15));    break;

        /* FIRQ Mode Shadowed Registers */
        case CPUINFO_STR_REGISTER + ARM7_FR8:   sprintf(info->s, "FR8 :%08x", ARM7REG(eR8_FIQ)  ); break;
        case CPUINFO_STR_REGISTER + ARM7_FR9:   sprintf(info->s, "FR9 :%08x", ARM7REG(eR9_FIQ)  ); break;
        case CPUINFO_STR_REGISTER + ARM7_FR10:  sprintf(info->s, "FR10:%08x", ARM7REG(eR10_FIQ) ); break;
        case CPUINFO_STR_REGISTER + ARM7_FR11:  sprintf(info->s, "FR11:%08x", ARM7REG(eR11_FIQ) ); break;
        case CPUINFO_STR_REGISTER + ARM7_FR12:  sprintf(info->s, "FR12:%08x", ARM7REG(eR12_FIQ) ); break;
        case CPUINFO_STR_REGISTER + ARM7_FR13:  sprintf(info->s, "FR13:%08x", ARM7REG(eR13_FIQ) ); break;
        case CPUINFO_STR_REGISTER + ARM7_FR14:  sprintf(info->s, "FR14:%08x", ARM7REG(eR14_FIQ) ); break;
        case CPUINFO_STR_REGISTER + ARM7_FSPSR: sprintf(info->s, "FR16:%08x", ARM7REG(eSPSR_FIQ)); break;

        /* IRQ Mode Shadowed Registers */
        case CPUINFO_STR_REGISTER + ARM7_IR13:  sprintf(info->s, "IR13:%08x", ARM7REG(eR13_IRQ) ); break;
        case CPUINFO_STR_REGISTER + ARM7_IR14:  sprintf(info->s, "IR14:%08x", ARM7REG(eR14_IRQ) ); break;
        case CPUINFO_STR_REGISTER + ARM7_ISPSR: sprintf(info->s, "IR16:%08x", ARM7REG(eSPSR_IRQ)); break;

        /* Supervisor Mode Shadowed Registers */
        case CPUINFO_STR_REGISTER + ARM7_SR13:  sprintf(info->s, "SR13:%08x", ARM7REG(eR13_SVC) ); break;
        case CPUINFO_STR_REGISTER + ARM7_SR14:  sprintf(info->s, "SR14:%08x", ARM7REG(eR14_SVC) ); break;
        case CPUINFO_STR_REGISTER + ARM7_SSPSR: sprintf(info->s, "SR16:%08x", ARM7REG(eSPSR_SVC)); break;

        /* Abort Mode Shadowed Registers */
        case CPUINFO_STR_REGISTER + ARM7_AR13:  sprintf(info->s, "AR13:%08x", ARM7REG(eR13_ABT) ); break;
        case CPUINFO_STR_REGISTER + ARM7_AR14:  sprintf(info->s, "AR14:%08x", ARM7REG(eR14_ABT) ); break;
        case CPUINFO_STR_REGISTER + ARM7_ASPSR: sprintf(info->s, "AR16:%08x", ARM7REG(eSPSR_ABT)); break;

        /* Undefined Mode Shadowed Registers */
        case CPUINFO_STR_REGISTER + ARM7_UR13:  sprintf(info->s, "UR13:%08x", ARM7REG(eR13_UND) ); break;
        case CPUINFO_STR_REGISTER + ARM7_UR14:  sprintf(info->s, "UR14:%08x", ARM7REG(eR14_UND) ); break;
        case CPUINFO_STR_REGISTER + ARM7_USPSR: sprintf(info->s, "UR16:%08x", ARM7REG(eSPSR_UND)); break;
    }
}

/* TEST COPROC CALLBACK HANDLERS - Used for example on how to implement only */
#if TEST_COPROC_FUNCS

static WRITE32_HANDLER(test_do_callback)
{
    LOG(("test_do_callback opcode=%x, =%x\n", offset, data));
}
static READ32_HANDLER(test_rt_r_callback)
{
    UINT32 data=0;
    LOG(("test_rt_r_callback opcode=%x\n", offset));
    return data;
}
static WRITE32_HANDLER(test_rt_w_callback)
{
    LOG(("test_rt_w_callback opcode=%x, data from ARM7 register=%x\n", offset, data));
}
static void test_dt_r_callback(arm_state *cpustate, UINT32 insn, UINT32 *prn, UINT32 (*read32)(arm_state *cpustate, UINT32 addr))
{
    LOG(("test_dt_r_callback: insn = %x\n", insn));
}
static void test_dt_w_callback(arm_state *cpustate, UINT32 insn, UINT32 *prn, void (*write32)(arm_state *cpustate, UINT32 addr, UINT32 data))
{
    LOG(("test_dt_w_callback: opcode = %x\n", insn));
}
#endif
