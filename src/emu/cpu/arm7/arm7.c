/*****************************************************************************
 *
 *   arm7.c
 *   Portable CPU Emulator for 32-bit ARM v3/4/5/6
 *
 *   Copyright Steve Ellenoff, all rights reserved.
 *   Thumb, DSP, and MMU support and many bugfixes by R. Belmont and Ryan Holtz.
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

    ** This is a plain vanilla implementation of an ARM7 cpu which incorporates my ARM7 core.
       It can be used as is, or used to demonstrate how to utilize the arm7 core to create a cpu
       that uses the core, since there are numerous different mcu packages that incorporate an arm7 core.

       See the notes in the arm7core.c file itself regarding issues/limitations of the arm7 core.
    **
*****************************************************************************/
#include "emu.h"
#include "debugger.h"
#include "arm7.h"
#include "arm7core.h"   //include arm7 core
#include "arm7thmb.h"
#include "arm7help.h"


/* prototypes of coprocessor functions */
static DECLARE_WRITE32_DEVICE_HANDLER(arm7_do_callback);
static DECLARE_READ32_DEVICE_HANDLER(arm7_rt_r_callback);
static DECLARE_WRITE32_DEVICE_HANDLER(arm7_rt_w_callback);
void arm7_dt_r_callback(arm_state *cpustate, UINT32 insn, UINT32 *prn, UINT32 (*read32)(arm_state *cpustate, UINT32 addr));
void arm7_dt_w_callback(arm_state *cpustate, UINT32 insn, UINT32 *prn, void (*write32)(arm_state *cpustate, UINT32 addr, UINT32 data));

// holder for the co processor Data Transfer Read & Write Callback funcs
void (*arm7_coproc_dt_r_callback)(arm_state *cpustate, UINT32 insn, UINT32 *prn, UINT32 (*read32)(arm_state *cpustate, UINT32 addr));
void (*arm7_coproc_dt_w_callback)(arm_state *cpustate, UINT32 insn, UINT32 *prn, void (*write32)(arm_state *cpustate, UINT32 addr, UINT32 data));


INLINE arm_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == ARM7 || device->type() == ARM7_BE || device->type() == ARM7500 || device->type() == ARM9 || device->type() == ARM920T || device->type() == PXA255);
	return (arm_state *)downcast<legacy_cpu_device *>(device)->token();
}

void set_cpsr( arm_state *cpustate, UINT32 val)
{
	if (cpustate->archFlags & eARM_ARCHFLAGS_MODE26)
	{
		if ((val & 0x10) != (ARM7REG(eCPSR) & 0x10))
		{
			if (val & 0x10)
			{
				// 26 -> 32
				val = (val & 0x0FFFFF3F) | (R15 & 0xF0000000) /* N Z C V */ | ((R15 & 0x0C000000) >> (26 - 6)) /* I F */;
				R15 = R15 & 0x03FFFFFC;
			}
			else
			{
				// 32 -> 26
				R15 = (R15 & 0x03FFFFFC) /* PC */ | (val & 0xF0000000) /* N Z C V */ | ((val & 0x000000C0) << (26 - 6)) /* I F */ | (val & 0x00000003) /* M1 M0 */;
			}
		}
		else
		{
			if (!(val & 0x10))
			{
				// mirror bits in pc
				R15 = (R15 & 0x03FFFFFF) | (val & 0xF0000000) /* N Z C V */ | ((val & 0x000000C0) << (26 - 6)) /* I F */;
			}
		}
	}
	else
	{
		val |= 0x10; // force valid mode
	}
	ARM7REG(eCPSR) = val;
}


/**************************************************************************
 * ARM TLB IMPLEMENTATION
 **************************************************************************/

enum
{
    TLB_COARSE = 0,
    TLB_FINE,
};

enum
{
    FAULT_NONE = 0,
    FAULT_DOMAIN,
    FAULT_PERMISSION,
};

INLINE UINT32 arm7_tlb_get_first_level_descriptor( arm_state *cpustate, UINT32 vaddr )
{
    UINT32 entry_paddr = ( COPRO_TLB_BASE & COPRO_TLB_BASE_MASK ) | ( ( vaddr & COPRO_TLB_VADDR_FLTI_MASK ) >> COPRO_TLB_VADDR_FLTI_MASK_SHIFT );
    return cpustate->program->read_dword( entry_paddr );
}

INLINE UINT32 arm7_tlb_get_second_level_descriptor( arm_state *cpustate, UINT32 granularity, UINT32 first_desc, UINT32 vaddr )
{
    UINT32 desc_lvl2 = vaddr;

    switch( granularity )
    {
        case TLB_COARSE:
            desc_lvl2 = ( first_desc & COPRO_TLB_CFLD_ADDR_MASK ) | ( ( vaddr & COPRO_TLB_VADDR_CSLTI_MASK ) >> COPRO_TLB_VADDR_CSLTI_MASK_SHIFT );
            break;
        case TLB_FINE:
            LOG( ( "ARM7: Attempting to get second-level TLB descriptor of fine granularity\n" ) );
            break;
        default:
            // We shouldn't be here
            LOG( ( "ARM7: Attempting to get second-level TLB descriptor of invalid granularity (%d)\n", granularity ) );
            break;
    }

    return cpustate->program->read_dword( desc_lvl2 );
}

INLINE int detect_fault( arm_state *cpustate, int permission, int ap, int flags)
{
	switch (permission)
	{
		case 0 : // "No access - Any access generates a domain fault"
		{
			return FAULT_DOMAIN;
		}
		break;
		case 1 : // "Client - Accesses are checked against the access permission bits in the section or page descriptor"
		{
			switch (ap)
			{
				case 0 :
				{
					int s = (COPRO_CTRL & COPRO_CTRL_SYSTEM) ? 1 : 0;
					int r = (COPRO_CTRL & COPRO_CTRL_ROM) ? 1 : 0;
					if (s == 0)
					{
						if (r == 0) // "Any access generates a permission fault"
						{
							return FAULT_PERMISSION;
						}
						else // "Any write generates a permission fault"
						{
							if (flags & ARM7_TLB_WRITE)
							{
								return FAULT_PERMISSION;
							}
						}
					}
					else
					{
						if (r == 0) // "Only Supervisor read permitted"
						{
							if ((GET_MODE == eARM7_MODE_USER) || (flags & ARM7_TLB_WRITE))
							{
								return FAULT_PERMISSION;
							}
						}
						else // "Reserved" -> assume same behaviour as S=0/R=0 case
						{
							return FAULT_PERMISSION;
						}
					}
				}
				break;
				case 1 : // "Access allowed only in Supervisor mode"
				{
					if (GET_MODE == eARM7_MODE_USER)
					{
						return FAULT_PERMISSION;
					}
				}
				break;
				case 2 : // "Writes in User mode cause permission fault"
				{
					if ((GET_MODE == eARM7_MODE_USER) && (flags & ARM7_TLB_WRITE))
					{
						return FAULT_PERMISSION;
					}
				}
				break;
				case 3 : // "All access types permitted in both modes"
				{
					return FAULT_NONE;
				}
				break;
			}
		}
		break;
		case 2 : // "Reserved - Reserved. Currently behaves like the no access mode"
		{
			return FAULT_DOMAIN;
		}
		break;
		case 3 : // "Manager - Accesses are not checked against the access permission bits so a permission fault cannot be generated"
		{
			return FAULT_NONE;
		}
		break;
	}
	return FAULT_NONE;
}

int arm7_tlb_translate(arm_state *cpustate, UINT32 *addr, int flags)
{
    UINT32 desc_lvl1;
    UINT32 desc_lvl2 = 0;
    UINT32 paddr, vaddr = *addr;
    UINT8 domain, permission;

    if (vaddr < 32 * 1024 * 1024)
    {
    	UINT32 pid = ((COPRO_FCSE_PID >> 25) & 0x7F);
    	if (pid > 0)
    	{
    		//LOG( ( "ARM7: FCSE PID vaddr %08X -> %08X\n", vaddr, vaddr + (pid * (32 * 1024 * 1024))) );
    		vaddr = vaddr + (((COPRO_FCSE_PID >> 25) & 0x7F) * (32 * 1024 * 1024));
    	}
    }

    desc_lvl1 = arm7_tlb_get_first_level_descriptor( cpustate, vaddr );

    paddr = vaddr;

#if ARM7_MMU_ENABLE_HACK
    if ((R15 == (cpustate->mmu_enable_addr + 4)) || (R15 == (cpustate->mmu_enable_addr + 8)))
    {
        LOG( ( "ARM7: fetch flat, PC = %08x, vaddr = %08x\n", R15, vaddr ) );
    	*addr = vaddr;
    	return TRUE;
    }
    else
    {
    	cpustate->mmu_enable_addr = 1;
    }
#endif

	domain = (desc_lvl1 >> 5) & 0xF;
	permission = (COPRO_DOMAIN_ACCESS_CONTROL >> (domain << 1)) & 3;

    switch( desc_lvl1 & 3 )
    {
        case COPRO_TLB_UNMAPPED:
            // Unmapped, generate a translation fault
            if (flags & ARM7_TLB_ABORT_D)
            {
	            LOG( ( "ARM7: Translation fault on unmapped virtual address, PC = %08x, vaddr = %08x\n", R15, vaddr ) );
            	COPRO_FAULT_STATUS_D = (5 << 0); // 5 = section translation fault
            	COPRO_FAULT_ADDRESS = vaddr;
            	cpustate->pendingAbtD = 1;
            }
            else if (flags & ARM7_TLB_ABORT_P)
            {
	            LOG( ( "ARM7: Translation fault on unmapped virtual address, PC = %08x, vaddr = %08x\n", R15, vaddr ) );
            	cpustate->pendingAbtP = 1;
            }
        	return FALSE;
            break;
        case COPRO_TLB_COARSE_TABLE:
            // Entry is the physical address of a coarse second-level table
            if ((permission == 1) || (permission == 3))
            {
	            desc_lvl2 = arm7_tlb_get_second_level_descriptor( cpustate, TLB_COARSE, desc_lvl1, vaddr );
            }
            else
            {
                fatalerror("ARM7: Not Yet Implemented: Coarse Table, Section Domain fault on virtual address, vaddr = %08x, domain = %08x, PC = %08x\n", vaddr, domain, R15);
            }
            break;
        case COPRO_TLB_SECTION_TABLE:
        	{
            // Entry is a section
        	UINT8 ap = (desc_lvl1 >> 10) & 3;
        	int fault = detect_fault( cpustate, permission, ap, flags);
        	if (fault == FAULT_NONE)
            {
            	paddr = ( desc_lvl1 & COPRO_TLB_SECTION_PAGE_MASK ) | ( vaddr & ~COPRO_TLB_SECTION_PAGE_MASK );
            }
            else
            {
                if (flags & ARM7_TLB_ABORT_D)
                {
                	LOG( ( "ARM7: Section Table, Section %s fault on virtual address, vaddr = %08x, PC = %08x\n", (fault == FAULT_DOMAIN) ? "domain" : "permission", vaddr, R15 ) );
        			COPRO_FAULT_STATUS_D = ((fault == FAULT_DOMAIN) ? (9 << 0) : (13 << 0)) | (domain << 4); // 9 = section domain fault, 13 = section permission fault
                	COPRO_FAULT_ADDRESS = vaddr;
            	    cpustate->pendingAbtD = 1;
    				LOG( ( "vaddr %08X desc_lvl1 %08X domain %d permission %d ap %d s %d r %d mode %d read %d write %d\n",
    					vaddr, desc_lvl1, domain, permission, ap, (COPRO_CTRL & COPRO_CTRL_SYSTEM) ? 1 : 0, (COPRO_CTRL & COPRO_CTRL_ROM) ? 1 : 0,
    					GET_MODE, flags & ARM7_TLB_READ ? 1 : 0,  flags & ARM7_TLB_WRITE ? 1 : 0) );
            	}
            	else if (flags & ARM7_TLB_ABORT_P)
            	{
                	LOG( ( "ARM7: Section Table, Section %s fault on virtual address, vaddr = %08x, PC = %08x\n", (fault == FAULT_DOMAIN) ? "domain" : "permission", vaddr, R15 ) );
            	    cpustate->pendingAbtP = 1;
            	}
            	return FALSE;
            }
        	}
            break;
        case COPRO_TLB_FINE_TABLE:
            // Entry is the physical address of a fine second-level table
            fatalerror("ARM7: Not Yet Implemented: fine second-level TLB lookup, PC = %08x, vaddr = %08x\n", R15, vaddr);
            break;
        default:
            // Entry is the physical address of a three-legged termite-eaten table
            break;
    }

    if( ( desc_lvl1 & 3 ) == COPRO_TLB_COARSE_TABLE || ( desc_lvl1 & 3 ) == COPRO_TLB_FINE_TABLE )
    {
        switch( desc_lvl2 & 3 )
        {
            case COPRO_TLB_UNMAPPED:
                // Unmapped, generate a translation fault
                if (flags & ARM7_TLB_ABORT_D)
                {
	                LOG( ( "ARM7: Translation fault on unmapped virtual address, vaddr = %08x, PC %08X\n", vaddr, R15 ) );
                	COPRO_FAULT_STATUS_D = (7 << 0) | (domain << 4); // 7 = page translation fault
                	COPRO_FAULT_ADDRESS = vaddr;
	        	    cpustate->pendingAbtD = 1;
	            }
	            else if (flags & ARM7_TLB_ABORT_P)
	            {
	                LOG( ( "ARM7: Translation fault on unmapped virtual address, vaddr = %08x, PC %08X\n", vaddr, R15 ) );
            		cpustate->pendingAbtP = 1;
		        }
            	return FALSE;
                break;
            case COPRO_TLB_LARGE_PAGE:
                // Large page descriptor
                paddr = ( desc_lvl2 & COPRO_TLB_LARGE_PAGE_MASK ) | ( vaddr & ~COPRO_TLB_LARGE_PAGE_MASK );
                break;
            case COPRO_TLB_SMALL_PAGE:
                // Small page descriptor
                {
					UINT8 ap = ((((desc_lvl2 >> 4) & 0xFF) >> (((vaddr >> 10) & 3) << 1)) & 3);
					int fault = detect_fault( cpustate, permission, ap, flags);
					if (fault == FAULT_NONE)
            		{
                		paddr = ( desc_lvl2 & COPRO_TLB_SMALL_PAGE_MASK ) | ( vaddr & ~COPRO_TLB_SMALL_PAGE_MASK );
            		}
            		else
            		{
            			if (flags & ARM7_TLB_ABORT_D)
            			{
            				// hapyfish expects a data abort when something tries to write to a read-only memory location from user mode
		                	LOG( ( "ARM7: Page Table, Section %s fault on virtual address, vaddr = %08x, PC = %08x\n", (fault == FAULT_DOMAIN) ? "domain" : "permission", vaddr, R15 ) );
		            		COPRO_FAULT_STATUS_D = ((fault == FAULT_DOMAIN) ? (11 << 0) : (15 << 0)) | (domain << 4); // 11 = page domain fault, 15 = page permission fault
        		    		COPRO_FAULT_ADDRESS = vaddr;
            		    	cpustate->pendingAbtD = 1;
		    				LOG( ( "vaddr %08X desc_lvl2 %08X domain %d permission %d ap %d s %d r %d mode %d read %d write %d\n",
		    					vaddr, desc_lvl2, domain, permission, ap, (COPRO_CTRL & COPRO_CTRL_SYSTEM) ? 1 : 0, (COPRO_CTRL & COPRO_CTRL_ROM) ? 1 : 0,
		    					GET_MODE, flags & ARM7_TLB_READ ? 1 : 0,  flags & ARM7_TLB_WRITE ? 1 : 0) );
		            	}
		            	else if (flags & ARM7_TLB_ABORT_P)
		            	{
		                	LOG( ( "ARM7: Page Table, Section %s fault on virtual address, vaddr = %08x, PC = %08x\n", (fault == FAULT_DOMAIN) ? "domain" : "permission", vaddr, R15 ) );
            		    	cpustate->pendingAbtP = 1;
		            	}
	            		return FALSE;
            		}
            	}
                break;
            case COPRO_TLB_TINY_PAGE:
                // Tiny page descriptor
                if( ( desc_lvl1 & 3 ) == 1 )
                {
                    LOG( ( "ARM7: It would appear that we're looking up a tiny page from a coarse TLB lookup.  This is bad. vaddr = %08x\n", vaddr ) );
                }
                paddr = ( desc_lvl2 & COPRO_TLB_TINY_PAGE_MASK ) | ( vaddr & ~COPRO_TLB_TINY_PAGE_MASK );
                break;
        }
    }
	*addr = paddr;
    return TRUE;
}

static CPU_TRANSLATE( arm7 )
{
	arm_state *cpustate = (device != NULL) ? (arm_state *)device->token() : NULL;

	/* only applies to the program address space and only does something if the MMU's enabled */
	if( space == AS_PROGRAM && ( COPRO_CTRL & COPRO_CTRL_MMU_EN ) )
	{
		return arm7_tlb_translate(cpustate, address, 0);
	}
	return TRUE;
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
	cpustate->program = device->space(AS_PROGRAM);
	cpustate->direct = &cpustate->program->direct();

	// setup co-proc callbacks
	arm7_coproc_do_callback = arm7_do_callback;
	arm7_coproc_rt_r_callback = arm7_rt_r_callback;
	arm7_coproc_rt_w_callback = arm7_rt_w_callback;
	arm7_coproc_dt_r_callback = arm7_dt_r_callback;
	arm7_coproc_dt_w_callback = arm7_dt_w_callback;
}

static CPU_RESET( arm7 )
{
	arm_state *cpustate = get_safe_token(device);

	// must call core reset
	arm7_core_reset(device);

	cpustate->archRev = 4;	// ARMv4
	cpustate->archFlags = eARM_ARCHFLAGS_T;	// has Thumb
}

static CPU_RESET( arm7_be )
{
	arm_state *cpustate = get_safe_token(device);

	CPU_RESET_CALL( arm7 );
	cpustate->endian = ENDIANNESS_BIG;
}

static CPU_RESET( arm7500 )
{
	arm_state *cpustate = get_safe_token(device);

	// must call core reset
	arm7_core_reset(device);

	cpustate->archRev = 3;	// ARMv3
	cpustate->archFlags = eARM_ARCHFLAGS_MODE26;
}

static CPU_RESET( arm9 )
{
	arm_state *cpustate = get_safe_token(device);

	// must call core reset
	arm7_core_reset(device);

	cpustate->archRev = 5;	// ARMv5
	cpustate->archFlags = eARM_ARCHFLAGS_T | eARM_ARCHFLAGS_E;	// has TE extensions
}

static CPU_RESET( arm920t )
{
	arm_state *cpustate = get_safe_token(device);

	// must call core reset
	arm7_core_reset(device);

	cpustate->archRev = 4;	// ARMv4
	cpustate->archFlags = eARM_ARCHFLAGS_T;	// has T extension
}

static CPU_RESET( pxa255 )
{
	arm_state *cpustate = get_safe_token(device);

	// must call core reset
	arm7_core_reset(device);

	cpustate->archRev = 5;	// ARMv5
	cpustate->archFlags = eARM_ARCHFLAGS_T | eARM_ARCHFLAGS_E | eARM_ARCHFLAGS_XSCALE;	// has TE and XScale extensions
}

static CPU_RESET( sa1110 )
{
	arm_state *cpustate = get_safe_token(device);

	// must call core reset
	arm7_core_reset(device);

	cpustate->archRev = 4;	// ARMv4
	cpustate->archFlags = eARM_ARCHFLAGS_SA;	// has StrongARM, no Thumb, no Enhanced DSP
}

static CPU_EXIT( arm7 )
{
	/* nothing to do here */
}

#define UNEXECUTED() \
	R15 += 4; \
	ARM7_ICOUNT +=2; /* Any unexecuted instruction only takes 1 cycle (page 193) */ \

static CPU_EXECUTE( arm7 )
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
			thumb_handler[(insn & 0xffc0) >> 6](cpustate, pc, insn);

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
				if (temp1 != temp2) fatalerror( "%08X: 32-bit and 26-bit modes are out of sync (%08X %08X)\n", pc, temp1, temp2);
			}
#endif

            insn = cpustate->direct->read_decrypted_dword(raddr);

            /* process condition codes for this instruction */
            switch (insn >> INSN_COND_SHIFT)
            {
				case COND_EQ:
					if (Z_IS_CLEAR(GET_CPSR))
						{ UNEXECUTED();  goto skip_exec; }
					break;
				case COND_NE:
					if (Z_IS_SET(GET_CPSR))
						{ UNEXECUTED();  goto skip_exec; }
					break;
				case COND_CS:
					if (C_IS_CLEAR(GET_CPSR))
						{ UNEXECUTED();  goto skip_exec; }
					break;
				case COND_CC:
					if (C_IS_SET(GET_CPSR))
						{ UNEXECUTED();  goto skip_exec; }
					break;
				case COND_MI:
					if (N_IS_CLEAR(GET_CPSR))
						{ UNEXECUTED();  goto skip_exec; }
					break;
				case COND_PL:
					if (N_IS_SET(GET_CPSR))
						{ UNEXECUTED();  goto skip_exec; }
					break;
				case COND_VS:
					if (V_IS_CLEAR(GET_CPSR))
						{ UNEXECUTED();  goto skip_exec; }
					break;
				case COND_VC:
					if (V_IS_SET(GET_CPSR))
						{ UNEXECUTED();  goto skip_exec; }
					break;
				case COND_HI:
					if (C_IS_CLEAR(GET_CPSR) || Z_IS_SET(GET_CPSR))
						{ UNEXECUTED();  goto skip_exec; }
					break;
				case COND_LS:
					if (C_IS_SET(GET_CPSR) && Z_IS_CLEAR(GET_CPSR))
						{ UNEXECUTED();  goto skip_exec; }
					break;
				case COND_GE:
					if (!(GET_CPSR & N_MASK) != !(GET_CPSR & V_MASK)) /* Use x ^ (x >> ...) method */
						{ UNEXECUTED();  goto skip_exec; }
					break;
				case COND_LT:
					if (!(GET_CPSR & N_MASK) == !(GET_CPSR & V_MASK))
						{ UNEXECUTED();  goto skip_exec; }
					break;
				case COND_GT:
					if (Z_IS_SET(GET_CPSR) || (!(GET_CPSR & N_MASK) != !(GET_CPSR & V_MASK)))
						{ UNEXECUTED();  goto skip_exec; }
					break;
				case COND_LE:
					if (Z_IS_CLEAR(GET_CPSR) && (!(GET_CPSR & N_MASK) == !(GET_CPSR & V_MASK)))
					  { UNEXECUTED();  goto skip_exec; }
					break;
				case COND_NV:
					{ UNEXECUTED();  goto skip_exec; }
					break;
            }
            /*******************************************************************/
            /* If we got here - condition satisfied, so decode the instruction */
            /*******************************************************************/
			ops_handler[((insn & 0xF000000) >> 24)](cpustate, insn);
		}

skip_exec:

        ARM7_CHECKIRQ;

        /* All instructions remove 3 cycles.. Others taking less / more will have adjusted this # prior to here */
        ARM7_ICOUNT -= 3;
    } while (ARM7_ICOUNT > 0);
}

static void set_irq_line(arm_state *cpustate, int irqline, int state)
{
    // must call core
    arm7_core_set_irq_line(cpustate, irqline, state);
}

static CPU_DISASSEMBLE( arm7 )
{
	CPU_DISASSEMBLE( arm7arm );
	CPU_DISASSEMBLE( arm7thumb );

    arm_state *cpustate = get_safe_token(device);

    if (T_IS_SET(GET_CPSR))
    	return CPU_DISASSEMBLE_CALL(arm7thumb);
    else
    	return CPU_DISASSEMBLE_CALL(arm7arm);
}

static CPU_DISASSEMBLE( arm7_be )
{
	CPU_DISASSEMBLE( arm7arm_be );
	CPU_DISASSEMBLE( arm7thumb_be );

	arm_state *cpustate = get_safe_token(device);

	if (T_IS_SET(GET_CPSR))
		return CPU_DISASSEMBLE_CALL(arm7thumb_be);
	else
		return CPU_DISASSEMBLE_CALL(arm7arm_be);
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
    arm_state *cpustate = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;

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

        case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM: info->i = 32;                   break;
        case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM: info->i = 32;                   break;
        case CPUINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM: info->i = 0;                    break;
        case CPUINFO_INT_DATABUS_WIDTH + AS_DATA:    info->i = 0;                    break;
        case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:    info->i = 0;                    break;
        case CPUINFO_INT_ADDRBUS_SHIFT + AS_DATA:    info->i = 0;                    break;
        case CPUINFO_INT_DATABUS_WIDTH + AS_IO:      info->i = 0;                    break;
        case CPUINFO_INT_ADDRBUS_WIDTH + AS_IO:      info->i = 0;                    break;
        case CPUINFO_INT_ADDRBUS_SHIFT + AS_IO:      info->i = 0;                    break;

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
        case CPUINFO_INT_REGISTER + ARM7_PC:    info->i = GET_PC;                                  break;
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
	case CPUINFO_FCT_TRANSLATE:	    	info->translate = CPU_TRANSLATE_NAME(arm7);		break;

        /* --- the following bits of info are returned as NULL-terminated strings --- */
        case CPUINFO_STR_NAME:                  strcpy(info->s, "ARM7");                        break;
        case CPUINFO_STR_FAMILY:           strcpy(info->s, "Acorn Risc Machine");          break;
        case CPUINFO_STR_VERSION:          strcpy(info->s, "2.0");                         break;
        case CPUINFO_STR_SOURCE_FILE:             strcpy(info->s, __FILE__);                      break;
        case CPUINFO_STR_CREDITS:          strcpy(info->s, "Copyright Steve Ellenoff, sellenoff@hotmail.com"); break;

        case CPUINFO_STR_FLAGS:
            sprintf(info->s, "%c%c%c%c%c%c%c%c %s",
                    (ARM7REG(eCPSR) & N_MASK) ? 'N' : '-',
                    (ARM7REG(eCPSR) & Z_MASK) ? 'Z' : '-',
                    (ARM7REG(eCPSR) & C_MASK) ? 'C' : '-',
                    (ARM7REG(eCPSR) & V_MASK) ? 'V' : '-',
                    (ARM7REG(eCPSR) & Q_MASK) ? 'Q' : '-',
                    (ARM7REG(eCPSR) & I_MASK) ? 'I' : '-',
                    (ARM7REG(eCPSR) & F_MASK) ? 'F' : '-',
                    (ARM7REG(eCPSR) & T_MASK) ? 'T' : '-',
                    GetModeText(ARM7REG(eCPSR)));
        break;

        /* registers shared by all operating modes */
        case CPUINFO_STR_REGISTER + ARM7_PC:    sprintf(info->s, "PC  :%08x", GET_PC);            break;
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


CPU_GET_INFO( arm7_be )
{
	switch (state)
	{
		case CPUINFO_INT_ENDIANNESS:		info->i = ENDIANNESS_BIG;								break;
		case CPUINFO_FCT_RESET:				info->reset = CPU_RESET_NAME(arm7_be);					break;
		case CPUINFO_FCT_DISASSEMBLE:		info->disassemble = CPU_DISASSEMBLE_NAME(arm7_be);		break;
		case CPUINFO_STR_NAME:				strcpy(info->s, "ARM7 (big endian)");					break;
		default:							CPU_GET_INFO_CALL(arm7);
	}
}

CPU_GET_INFO( arm7500 )
{
    switch (state)
    {
        case CPUINFO_FCT_RESET:		info->reset = CPU_RESET_NAME(arm7500);		break;
        case CPUINFO_STR_NAME:		strcpy(info->s, "ARM7500");				break;
		default:					CPU_GET_INFO_CALL(arm7);
		break;
    }
}

CPU_GET_INFO( arm9 )
{
    switch (state)
    {
        case CPUINFO_FCT_RESET:                 info->reset = CPU_RESET_NAME(arm9);                       break;
        case CPUINFO_STR_NAME:             strcpy(info->s, "ARM9");                        break;
	default:	CPU_GET_INFO_CALL(arm7);
		break;
    }
}

CPU_GET_INFO( arm920t )
{
    switch (state)
    {
        case CPUINFO_FCT_RESET:                 info->reset = CPU_RESET_NAME(arm920t);                       break;
        case CPUINFO_STR_NAME:             strcpy(info->s, "ARM920T");                        break;
	default:	CPU_GET_INFO_CALL(arm7);
		break;
    }
}

CPU_GET_INFO( pxa255 )
{
    switch (state)
    {
        case CPUINFO_FCT_RESET:            info->reset = CPU_RESET_NAME(pxa255);                       break;
        case CPUINFO_STR_NAME:             strcpy(info->s, "PXA255");                        break;
	default:	CPU_GET_INFO_CALL(arm7);
		break;
    }
}

CPU_GET_INFO( sa1110 )
{
    switch (state)
    {
        case CPUINFO_FCT_RESET:            info->reset = CPU_RESET_NAME(sa1110);                       break;
        case CPUINFO_STR_NAME:             strcpy(info->s, "SA1110");                        break;
	default:	CPU_GET_INFO_CALL(arm7);
		break;
    }
}

/* ARM system coprocessor support */

static WRITE32_DEVICE_HANDLER( arm7_do_callback )
{
    arm_state *cpustate = get_safe_token(device);
	cpustate->pendingUnd = 1;
}

static READ32_DEVICE_HANDLER( arm7_rt_r_callback )
{
    arm_state *cpustate = get_safe_token(device);
    UINT32 opcode = offset;
    UINT8 cReg = ( opcode & INSN_COPRO_CREG ) >> INSN_COPRO_CREG_SHIFT;
    UINT8 op2 =  ( opcode & INSN_COPRO_OP2 )  >> INSN_COPRO_OP2_SHIFT;
    UINT8 op3 =    opcode & INSN_COPRO_OP3;
    UINT8 cpnum = (opcode & INSN_COPRO_CPNUM) >> INSN_COPRO_CPNUM_SHIFT;
    UINT32 data = 0;

//    printf("cpnum %d cReg %d op2 %d op3 %d (%x)\n", cpnum, cReg, op2, op3, GET_REGISTER(cpustate, 15));

    // we only handle system copro here
    if (cpnum != 15)
    {
    	if (cpustate->archFlags & eARM_ARCHFLAGS_XSCALE)
	{
		// handle XScale specific CP14
		if (cpnum == 14)
		{
			switch( cReg )
			{
				case 1:	// clock counter
					data = (UINT32)cpustate->device->total_cycles();
					break;

				default:
					break;
			}
		}
		else
		{
			fatalerror("XScale: Unhandled coprocessor %d (archFlags %x)\n", cpnum, cpustate->archFlags);
		}

		return data;
	}
	else
	{
		LOG( ("ARM7: Unhandled coprocessor %d (archFlags %x)\n", cpnum, cpustate->archFlags) );
		cpustate->pendingUnd = 1;
		return 0;
	}
    }

    switch( cReg )
    {
        case 4:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
            // RESERVED
            LOG( ( "arm7_rt_r_callback CR%d, RESERVED\n", cReg ) );
            break;
        case 0:             // ID
	    switch(op2)
	    {
	    	case 0:
		    switch (cpustate->archRev)
		    {
		    	case 3:	// ARM6 32-bit
				data = 0x41;
				break;

			case 4: // ARM7/SA11xx
				if (cpustate->archFlags & eARM_ARCHFLAGS_SA)
				{
					// ARM Architecture Version 4
					// Part Number 0xB11 (SA1110)
					// Stepping B5
			        	data = 0x69 | ( 0x01 << 16 ) | ( 0xB11 << 4 ) | 0x9;
				}
				else
				{
					if (device->type() == ARM920T)
					{
						data = (0x41 << 24) | (1 << 20) | (2 << 16) | (0x920 << 4) | (0 << 0); // ARM920T (S3C24xx)
					}
					else if (device->type() == ARM7500)
					{
						data = (0x41 << 24) | (0 << 20) | (1 << 16) | (0x710 << 4) | (0 << 0); // ARM7500
					}
					else
					{
						data = 0x41 | (1 << 23) | (7 << 12); // <-- where did this come from?
					}
				}
				break;

			case 5:	// ARM9/10/XScale
				data = 0x41 | (9 << 12);
				if (cpustate->archFlags & eARM_ARCHFLAGS_T)
				{
					if (cpustate->archFlags & eARM_ARCHFLAGS_E)
					{
						if (cpustate->archFlags & eARM_ARCHFLAGS_J)
						{
							data |= (6<<16);	// v5TEJ
						}
						else
						{
							data |= (5<<16);	// v5TE
						}
					}
					else
					{
						data |= (4<<16);	// v5T
					}
				}
				break;

			case 6:	// ARM11
				data = 0x41 | (10<< 12) | (7<<16);	// v6
				break;
		    }
		    break;
	    	case 1:	// cache type
			data = 0x0f0d2112;	// HACK: value expected by ARMWrestler (probably Nintendo DS ARM9's value)
			//data = (6 << 25) | (1 << 24) | (0x172 << 12) | (0x172 << 0); // ARM920T (S3C24xx)
			break;
		case 2: // TCM type
			data = 0;
			break;
		case 3: // TLB type
			data = 0;
			break;
		case 4:	// MPU type
			data = 0;
			break;
	    }
            LOG( ( "arm7_rt_r_callback, ID\n" ) );
            break;
        case 1:             // Control
            data = COPRO_CTRL | 0x70;	// bits 4-6 always read back as "1" (bit 3 too in XScale)
            break;
        case 2:             // Translation Table Base
            data = COPRO_TLB_BASE;
            break;
        case 3:             // Domain Access Control
            LOG( ( "arm7_rt_r_callback, Domain Access Control\n" ) );
            data = COPRO_DOMAIN_ACCESS_CONTROL;
            break;
        case 5:             // Fault Status
            LOG( ( "arm7_rt_r_callback, Fault Status\n" ) );
            switch (op3)
            {
            	case 0: data = COPRO_FAULT_STATUS_D; break;
            	case 1: data = COPRO_FAULT_STATUS_P; break;
            }
            break;
        case 6:             // Fault Address
            LOG( ( "arm7_rt_r_callback, Fault Address\n" ) );
            data = COPRO_FAULT_ADDRESS;
            break;
        case 13:            // Read Process ID (PID)
            LOG( ( "arm7_rt_r_callback, Read PID\n" ) );
            data = COPRO_FCSE_PID;
            break;
        case 14:            // Read Breakpoint
            LOG( ( "arm7_rt_r_callback, Read Breakpoint\n" ) );
            break;
        case 15:            // Test, Clock, Idle
            LOG( ( "arm7_rt_r_callback, Test / Clock / Idle \n" ) );
            break;
    }

    return data;
}

static WRITE32_DEVICE_HANDLER( arm7_rt_w_callback )
{
    arm_state *cpustate = get_safe_token(device);
    UINT32 opcode = offset;
    UINT8 cReg = ( opcode & INSN_COPRO_CREG ) >> INSN_COPRO_CREG_SHIFT;
    UINT8 op2 =  ( opcode & INSN_COPRO_OP2 )  >> INSN_COPRO_OP2_SHIFT;
    UINT8 op3 =    opcode & INSN_COPRO_OP3;
    UINT8 cpnum = (opcode & INSN_COPRO_CPNUM) >> INSN_COPRO_CPNUM_SHIFT;

    // handle XScale specific CP14 - just eat writes for now
    if (cpnum != 15)
    {
	    if (cpnum == 14)
	    {
	    	LOG( ("arm7_rt_w_callback: write %x to XScale CP14 reg %d\n", data, cReg) );
	    	return;
	    }
	    else
	    {
	    	LOG( ("ARM7: Unhandled coprocessor %d\n", cpnum) );
			cpustate->pendingUnd = 1;
			return;
	    }
    }

    switch( cReg )
    {
        case 0:
        case 4:
        case 10:
        case 11:
        case 12:
            // RESERVED
            LOG( ( "arm7_rt_w_callback CR%d, RESERVED = %08x\n", cReg, data) );
            break;
        case 1:             // Control
            LOG( ( "arm7_rt_w_callback Control = %08x (%d) (%d)\n", data, op2, op3 ) );
            LOG( ( "    MMU:%d, Address Fault:%d, Data Cache:%d, Write Buffer:%d\n",
                   data & COPRO_CTRL_MMU_EN, ( data & COPRO_CTRL_ADDRFAULT_EN ) >> COPRO_CTRL_ADDRFAULT_EN_SHIFT,
                   ( data & COPRO_CTRL_DCACHE_EN ) >> COPRO_CTRL_DCACHE_EN_SHIFT,
                   ( data & COPRO_CTRL_WRITEBUF_EN ) >> COPRO_CTRL_WRITEBUF_EN_SHIFT ) );
            LOG( ( "    Endianness:%d, System:%d, ROM:%d, Instruction Cache:%d\n",
                   ( data & COPRO_CTRL_ENDIAN ) >> COPRO_CTRL_ENDIAN_SHIFT,
                   ( data & COPRO_CTRL_SYSTEM ) >> COPRO_CTRL_SYSTEM_SHIFT,
                   ( data & COPRO_CTRL_ROM ) >> COPRO_CTRL_ROM_SHIFT,
                   ( data & COPRO_CTRL_ICACHE_EN ) >> COPRO_CTRL_ICACHE_EN_SHIFT ) );
            LOG( ( "    Int Vector Adjust:%d\n", ( data & COPRO_CTRL_INTVEC_ADJUST ) >> COPRO_CTRL_INTVEC_ADJUST_SHIFT ) );
#if ARM7_MMU_ENABLE_HACK
            if (((data & COPRO_CTRL_MMU_EN) != 0) && ((COPRO_CTRL & COPRO_CTRL_MMU_EN) == 0))
            {
            	cpustate->mmu_enable_addr = R15;
            }
            if (((data & COPRO_CTRL_MMU_EN) == 0) && ((COPRO_CTRL & COPRO_CTRL_MMU_EN) != 0))
            {
            	if (!arm7_tlb_translate( cpustate, &R15, 0))
            	{
            		fatalerror("ARM7_MMU_ENABLE_HACK translate failed\n");
            	}
            }
#endif
            COPRO_CTRL = data & COPRO_CTRL_MASK;
            break;
        case 2:             // Translation Table Base
            LOG( ( "arm7_rt_w_callback TLB Base = %08x (%d) (%d)\n", data, op2, op3 ) );
            COPRO_TLB_BASE = data;
            break;
        case 3:             // Domain Access Control
            LOG( ( "arm7_rt_w_callback Domain Access Control = %08x (%d) (%d)\n", data, op2, op3 ) );
            COPRO_DOMAIN_ACCESS_CONTROL = data;
            break;
        case 5:             // Fault Status
            LOG( ( "arm7_rt_w_callback Fault Status = %08x (%d) (%d)\n", data, op2, op3 ) );
            switch (op3)
            {
            	case 0: COPRO_FAULT_STATUS_D = data; break;
            	case 1: COPRO_FAULT_STATUS_P = data; break;
            }
            break;
        case 6:             // Fault Address
            LOG( ( "arm7_rt_w_callback Fault Address = %08x (%d) (%d)\n", data, op2, op3 ) );
            COPRO_FAULT_ADDRESS = data;
            break;
        case 7:             // Cache Operations
//            LOG( ( "arm7_rt_w_callback Cache Ops = %08x (%d) (%d)\n", data, op2, op3 ) );
            break;
        case 8:             // TLB Operations
            LOG( ( "arm7_rt_w_callback TLB Ops = %08x (%d) (%d)\n", data, op2, op3 ) );
            break;
        case 9:             // Read Buffer Operations
            LOG( ( "arm7_rt_w_callback Read Buffer Ops = %08x (%d) (%d)\n", data, op2, op3 ) );
            break;
        case 13:            // Write Process ID (PID)
            LOG( ( "arm7_rt_w_callback Write PID = %08x (%d) (%d)\n", data, op2, op3 ) );
            COPRO_FCSE_PID = data;
            break;
        case 14:            // Write Breakpoint
            LOG( ( "arm7_rt_w_callback Write Breakpoint = %08x (%d) (%d)\n", data, op2, op3 ) );
            break;
        case 15:            // Test, Clock, Idle
            LOG( ( "arm7_rt_w_callback Test / Clock / Idle = %08x (%d) (%d)\n", data, op2, op3 ) );
            break;
    }
}

void arm7_dt_r_callback(arm_state *cpustate, UINT32 insn, UINT32 *prn, UINT32 (*read32)(arm_state *cpustate, UINT32 addr))
{
	UINT8 cpn = (insn >> 8) & 0xF;
	if ((cpustate->archFlags & eARM_ARCHFLAGS_XSCALE) && (cpn == 0))
	{
		LOG( ( "arm7_dt_r_callback: DSP Coprocessor 0 (CP0) not yet emulated (PC %08x)\n", GET_PC ) );
	}
	else
	{
		cpustate->pendingUnd = 1;
	}
}

void arm7_dt_w_callback(arm_state *cpustate, UINT32 insn, UINT32 *prn, void (*write32)(arm_state *cpustate, UINT32 addr, UINT32 data))
{
	UINT8 cpn = (insn >> 8) & 0xF;
	if ((cpustate->archFlags & eARM_ARCHFLAGS_XSCALE) && (cpn == 0))
	{
		LOG( ( "arm7_dt_w_callback: DSP Coprocessor 0 (CP0) not yet emulated (PC %08x)\n", GET_PC ) );
	}
	else
	{
		cpustate->pendingUnd = 1;
	}
}

DEFINE_LEGACY_CPU_DEVICE(ARM7, arm7);
DEFINE_LEGACY_CPU_DEVICE(ARM7_BE, arm7_be);
DEFINE_LEGACY_CPU_DEVICE(ARM7500, arm7500);
DEFINE_LEGACY_CPU_DEVICE(ARM9, arm9);
DEFINE_LEGACY_CPU_DEVICE(ARM920T, arm920t);
DEFINE_LEGACY_CPU_DEVICE(PXA255, pxa255);
DEFINE_LEGACY_CPU_DEVICE(SA1110, sa1110);
