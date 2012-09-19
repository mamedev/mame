/*****************************************************************************

    h6280.c - Portable HuC6280 emulator

    Copyright Bryan McPhail, mish@tendril.co.uk

    This source code is based (with permission!) on the 6502 emulator by
    Juergen Buchmueller.  It is released as part of the Mame emulator project.
    Let me know if you intend to use this code in any other project.


    NOTICE:

    This code is around 99% complete!  Several things are unimplemented,
    some due to lack of time, some due to lack of documentation, mainly
    due to lack of programs using these features.

    csh, csl opcodes are not supported.

    I am unsure if flag B is set upon execution of rti.

    Cycle counts should be quite accurate.


    Changelog, version 1.02:
        JMP + indirect X (0x7c) opcode fixed.
        SMB + RMB opcodes fixed in disassembler.
        change_pc function calls removed.
        TSB & TRB now set flags properly.
        BIT opcode altered.

    Changelog, version 1.03:
        Swapped IRQ mask for IRQ1 & IRQ2 (thanks Yasuhiro)

    Changelog, version 1.04, 28/9/99-22/10/99:
        Adjusted RTI (thanks Karl)
        TST opcodes fixed in disassembler (missing break statements in a case!).
        TST behaviour fixed.
        SMB/RMB/BBS/BBR fixed in disassembler.

    Changelog, version 1.05, 8/12/99-16/12/99:
        Added CAB's timer implementation (note: irq ack & timer reload are changed).
        Fixed STA IDX.
        Fixed B flag setting on BRK.
        Assumed CSH & CSL to take 2 cycles each.

        Todo:  Performance could be improved by precalculating timer fire position.

    Changelog, version 1.06, 4/5/00 - last opcode bug found?
        JMP indirect was doing a EAL++; instead of EAD++; - Obviously causing
        a corrupt read when L = 0xff!  This fixes Bloody Wolf and Trio The Punch!

    Changelog, version 1.07, 3/9/00:
        Changed timer to be single shot - fixes Crude Buster music in level 1.

    Changelog, version 1.08, 8/11/05: (Charles MacDonald)

        Changed timer implementation, no longer single shot and reading the
        timer registers returns the count only. Fixes the following:
        - Mesopotamia: Music tempo & in-game timer
        - Dragon Saber: DDA effects
        - Magical Chase: Music tempo and speed regulation
        - Cadash: Allows the first level to start
        - Turrican: Allows the game to start

        Changed PLX and PLY to set NZ flags. Fixes:
        - Afterburner: Graphics unpacking
        - Aoi Blink: Collision detection with background

        Fixed the decimal version of ADC/SBC to *not* update the V flag,
        only the binary ones do.

        Fixed B flag handling so it is always set outside of an interrupt;
        even after being set by PLP and RTI.

        Fixed P state after reset to set I and B, leaving T, D cleared and
        NVZC randomized (cleared in this case).

        Fixed interrupt processing order (Timer has highest priority followed
        by IRQ1 and finally IRQ2).

    Changelog, version 1.09, 1/07/06: (Rob Bohms)

        Added emulation of the T flag, fixes PCE Ankuku Densetsu title screen

    Changelog, version 1.10, 5/09/07: (Wilbert Pol)

        - Taking of interrupts is delayed to respect a pending instruction already
          in the instruction pipeline; fixes After Burner.
        - Added 1 cycle for decimal mode ADC and SBC instructions.
        - Changed cycle counts for CSH and CSL instructions to 3.
        - Added T flag support to the SBC instruction.
        - Fixed ADC T flag to set the Z flag based on the value read.
        - Added 3 cycle penalty to ADC, AND, EOR, ORA, and SBC instructions
          when the T flag is set.
        - Fixed cycle count and support for 65536 byte blocks for the TAI, TDD,
          TIA, TII, and TIN instructions.
        - Fixed RDWORD macro in the disassembler.
        - Fixed setting of N and V flags in the TST instructions.
        - Removed unneeded debug_mmr code.
        - Fixed TSB and TRB instructions.
        - Added 1 delay when accessing the VDC or VCE areas.
        - Implemented low and high speed cpu modes.

    Changelog, version 1.11, 18/09/07: (Wilbert Pol)

        - Improvements to the handling of taking of delayed interrupts.

******************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "h6280.h"

static void set_irq_line(h6280_Regs* cpustate, int irqline, int state);

/* include the macros */
#include "h6280ops.h"

INLINE h6280_Regs *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == H6280);
	return (h6280_Regs *)downcast<legacy_cpu_device *>(device)->token();
}

/* include the opcode macros, functions and function pointer tables */
#include "tblh6280.c"

/*****************************************************************************/
static CPU_INIT( h6280 )
{
	h6280_Regs* cpustate = get_safe_token(device);

	device->save_item(NAME(cpustate->ppc.w.l));
	device->save_item(NAME(cpustate->pc.w.l));
	device->save_item(NAME(cpustate->sp.w.l));
	device->save_item(NAME(cpustate->zp.w.l));
	device->save_item(NAME(cpustate->ea.w.l));
	device->save_item(NAME(cpustate->a));
	device->save_item(NAME(cpustate->x));
	device->save_item(NAME(cpustate->y));
	device->save_item(NAME(cpustate->p));
	device->save_item(NAME(cpustate->mmr));
	device->save_item(NAME(cpustate->irq_mask));
	device->save_item(NAME(cpustate->timer_status));
	device->save_item(NAME(cpustate->timer_ack));
	device->save_item(NAME(cpustate->clocks_per_cycle));
	device->save_item(NAME(cpustate->timer_value));
	device->save_item(NAME(cpustate->timer_load));
	device->save_item(NAME(cpustate->nmi_state));
	device->save_item(NAME(cpustate->irq_state[0]));
	device->save_item(NAME(cpustate->irq_state[1]));
	device->save_item(NAME(cpustate->irq_state[2]));
	device->save_item(NAME(cpustate->irq_pending));

	#if LAZY_FLAGS
	device->save_item(NAME(cpustate->NZ));
	#endif
	device->save_item(NAME(cpustate->io_buffer));

	cpustate->irq_callback = irqcallback;
	cpustate->device = device;
	cpustate->program = &device->space(AS_PROGRAM);
	cpustate->direct = &cpustate->program->direct();
	cpustate->io = &device->space(AS_IO);
}

static CPU_RESET( h6280 )
{
	h6280_Regs* cpustate = get_safe_token(device);

	device_irq_acknowledge_callback save_irqcallback;
	int i;

	/* wipe out the h6280 structure */
	save_irqcallback = cpustate->irq_callback;
	memset(cpustate, 0, sizeof(h6280_Regs));
	cpustate->irq_callback = save_irqcallback;
	cpustate->device = device;
	cpustate->program = &device->space(AS_PROGRAM);
	cpustate->direct = &cpustate->program->direct();
	cpustate->io = &device->space(AS_IO);

	/* set I and B flags */
	P = _fI | _fB;

    /* stack starts at 0x01ff */
	cpustate->sp.d = 0x1ff;

    /* read the reset vector into PC */
	PCL = RDMEM(cpustate, H6280_RESET_VEC);
	PCH = RDMEM(cpustate, (H6280_RESET_VEC+1));

	/* CPU starts in low speed mode */
    cpustate->clocks_per_cycle = 4;

	/* timer off by default */
	cpustate->timer_status=0;
	cpustate->timer_load = 128 * 1024;

    /* clear pending interrupts */
	for (i = 0; i < 3; i++)
		cpustate->irq_state[i] = CLEAR_LINE;
	cpustate->nmi_state = CLEAR_LINE;

	cpustate->irq_pending = 0;
}

static CPU_EXIT( h6280 )
{
	/* nothing */
}

static CPU_EXECUTE( h6280 )
{
	int in;
	h6280_Regs* cpustate = get_safe_token(device);

	if ( cpustate->irq_pending == 2 ) {
		cpustate->irq_pending--;
	}

	/* Execute instructions */
	do
    {
		cpustate->ppc = cpustate->pc;

		debugger_instruction_hook(device, PCW);

		/* Execute 1 instruction */
		in=RDOP();
		PCW++;
		insnh6280[in](cpustate);

		if ( cpustate->irq_pending ) {
			if ( cpustate->irq_pending == 1 ) {
				if ( !(P & _fI) ) {
					cpustate->irq_pending--;
					CHECK_AND_TAKE_IRQ_LINES;
				}
			} else {
				cpustate->irq_pending--;
			}
		}

		/* Check internal timer */
		if(cpustate->timer_status)
		{
			if(cpustate->timer_value<=0)
			{
				if ( ! cpustate->irq_pending )
					cpustate->irq_pending = 1;
				while( cpustate->timer_value <= 0 )
					cpustate->timer_value += cpustate->timer_load;
				set_irq_line(cpustate, 2,ASSERT_LINE);
			}
		}
	} while (cpustate->ICount > 0);
}

/*****************************************************************************/

static void set_irq_line(h6280_Regs* cpustate, int irqline, int state)
{
	if (irqline == INPUT_LINE_NMI)
	{
		if ( state != ASSERT_LINE ) return;
		cpustate->nmi_state = state;
		CHECK_IRQ_LINES;
	}
	else if (irqline < 3)
	{
		/* If the state has not changed, just return */
		if ( cpustate->irq_state[irqline] == state )
			return;

	    cpustate->irq_state[irqline] = state;

		CHECK_IRQ_LINES;
	}
}



/*****************************************************************************/

READ8_HANDLER( h6280_irq_status_r )
{
	int status;
	h6280_Regs *cpustate = get_safe_token(&space.device());

	switch (offset&3)
	{
	default: return cpustate->io_buffer;
	case 3:
		{
			status=0;
			if(cpustate->irq_state[1]!=CLEAR_LINE) status|=1; /* IRQ 2 */
			if(cpustate->irq_state[0]!=CLEAR_LINE) status|=2; /* IRQ 1 */
			if(cpustate->irq_state[2]!=CLEAR_LINE) status|=4; /* TIMER */
			return status|(cpustate->io_buffer&(~H6280_IRQ_MASK));
		}
	case 2: return cpustate->irq_mask|(cpustate->io_buffer&(~H6280_IRQ_MASK));
	}
}

WRITE8_HANDLER( h6280_irq_status_w )
{
	h6280_Regs *cpustate = get_safe_token(&space.device());
	cpustate->io_buffer=data;
	switch (offset&3)
	{
		default:cpustate->io_buffer=data;break;
		case 2: /* Write irq mask */
			cpustate->irq_mask=data&0x7;
			CHECK_IRQ_LINES;
			break;

		case 3: /* Timer irq ack */
			set_irq_line(cpustate, 2, CLEAR_LINE);
			break;
	}
}

READ8_HANDLER( h6280_timer_r )
{
	/* only returns countdown */
	h6280_Regs *cpustate = get_safe_token(&space.device());
	return ((cpustate->timer_value >> 10)&0x7F)|(cpustate->io_buffer&0x80);
}

WRITE8_HANDLER( h6280_timer_w )
{
	h6280_Regs *cpustate = get_safe_token(&space.device());
	cpustate->io_buffer=data;
	switch (offset & 1) {
		case 0: /* Counter preload */
			cpustate->timer_load=cpustate->timer_value=((data&127)+1)*1024;
			return;

		case 1: /* Counter enable */
			if(data&1)
			{	/* stop -> start causes reload */
				if(cpustate->timer_status==0) cpustate->timer_value=cpustate->timer_load;
			}
			cpustate->timer_status=data&1;
			return;
	}
}

static CPU_TRANSLATE( h6280 )
{
	h6280_Regs* cpustate = get_safe_token(device);

	if (space == AS_PROGRAM)
		*address = TRANSLATED(*address);

	return TRUE;
}

UINT8 h6280io_get_buffer(device_t *device)
{
	h6280_Regs* cpustate = get_safe_token(device);
	return cpustate->io_buffer;
}
void h6280io_set_buffer(device_t *device, UINT8 data)
{
	h6280_Regs* cpustate = get_safe_token(device);
	cpustate->io_buffer=data;
}


/*****************************************************************************/

/**************************************************************************
 * Generic set_info
 **************************************************************************/

static CPU_SET_INFO( h6280 )
{
	h6280_Regs* cpustate = get_safe_token(device);

	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + 0:			   set_irq_line(cpustate, 0, info->i);				break;
		case CPUINFO_INT_INPUT_STATE + 1:			   set_irq_line(cpustate, 1, info->i);				break;
		case CPUINFO_INT_INPUT_STATE + 2:			   set_irq_line(cpustate, 2, info->i);				break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI: set_irq_line(cpustate, INPUT_LINE_NMI, info->i);	break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + H6280_PC:		PCW = info->i;									break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + H6280_S:		S = info->i;									break;
		case CPUINFO_INT_REGISTER + H6280_P:		P = info->i;									break;
		case CPUINFO_INT_REGISTER + H6280_A:		A = info->i;									break;
		case CPUINFO_INT_REGISTER + H6280_X:		X = info->i;									break;
		case CPUINFO_INT_REGISTER + H6280_Y:		Y = info->i;									break;
		case CPUINFO_INT_REGISTER + H6280_IRQ_MASK: cpustate->irq_mask = info->i; CHECK_IRQ_LINES;	break;
		case CPUINFO_INT_REGISTER + H6280_TIMER_STATE: cpustate->timer_status = info->i;			break;
		case CPUINFO_INT_REGISTER + H6280_NMI_STATE: set_irq_line( cpustate, INPUT_LINE_NMI, info->i );	break;
		case CPUINFO_INT_REGISTER + H6280_IRQ1_STATE: set_irq_line( cpustate, 0, info->i ); 		break;
		case CPUINFO_INT_REGISTER + H6280_IRQ2_STATE: set_irq_line( cpustate, 1, info->i ); 		break;
		case CPUINFO_INT_REGISTER + H6280_IRQT_STATE: set_irq_line( cpustate, 2, info->i ); 		break;
		case CPUINFO_INT_REGISTER + H6280_M1:		cpustate->mmr[0] = info->i;						break;
		case CPUINFO_INT_REGISTER + H6280_M2:		cpustate->mmr[1] = info->i;						break;
		case CPUINFO_INT_REGISTER + H6280_M3:		cpustate->mmr[2] = info->i;						break;
		case CPUINFO_INT_REGISTER + H6280_M4:		cpustate->mmr[3] = info->i;						break;
		case CPUINFO_INT_REGISTER + H6280_M5:		cpustate->mmr[4] = info->i;						break;
		case CPUINFO_INT_REGISTER + H6280_M6:		cpustate->mmr[5] = info->i;						break;
		case CPUINFO_INT_REGISTER + H6280_M7:		cpustate->mmr[6] = info->i;						break;
		case CPUINFO_INT_REGISTER + H6280_M8:		cpustate->mmr[7] = info->i;						break;
	}
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

CPU_GET_INFO( h6280 )
{
	h6280_Regs* cpustate = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(h6280_Regs);			break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 3;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_LITTLE;			break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 7;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 2;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 17 + 6*65536;					break;

		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM: info->i = 21;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM: info->i = 0;					break;
		case CPUINFO_INT_LOGADDR_WIDTH_PROGRAM: info->i = 16;					break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_DATA:	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_IO:		info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_IO:		info->i = 2;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_IO:		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + 0:				info->i = cpustate->irq_state[0];		break;
		case CPUINFO_INT_INPUT_STATE + 1:				info->i = cpustate->irq_state[1];		break;
		case CPUINFO_INT_INPUT_STATE + 2:				info->i = cpustate->irq_state[2];		break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:	info->i = cpustate->nmi_state;			break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = cpustate->ppc.d;				break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + H6280_PC:			info->i = PCD;							break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + H6280_S:			info->i = S;							break;
		case CPUINFO_INT_REGISTER + H6280_P:			info->i = P;							break;
		case CPUINFO_INT_REGISTER + H6280_A:			info->i = A;							break;
		case CPUINFO_INT_REGISTER + H6280_X:			info->i = X;							break;
		case CPUINFO_INT_REGISTER + H6280_Y:			info->i = Y;							break;
		case CPUINFO_INT_REGISTER + H6280_IRQ_MASK:		info->i = cpustate->irq_mask;			break;
		case CPUINFO_INT_REGISTER + H6280_TIMER_STATE:	info->i = cpustate->timer_status;		break;
		case CPUINFO_INT_REGISTER + H6280_NMI_STATE:	info->i = cpustate->nmi_state;			break;
		case CPUINFO_INT_REGISTER + H6280_IRQ1_STATE:	info->i = cpustate->irq_state[0];		break;
		case CPUINFO_INT_REGISTER + H6280_IRQ2_STATE:	info->i = cpustate->irq_state[1];		break;
		case CPUINFO_INT_REGISTER + H6280_IRQT_STATE:	info->i = cpustate->irq_state[2];		break;
		case CPUINFO_INT_REGISTER + H6280_M1:			info->i = cpustate->mmr[0];				break;
		case CPUINFO_INT_REGISTER + H6280_M2:			info->i = cpustate->mmr[1];				break;
		case CPUINFO_INT_REGISTER + H6280_M3:			info->i = cpustate->mmr[2];				break;
		case CPUINFO_INT_REGISTER + H6280_M4:			info->i = cpustate->mmr[3];				break;
		case CPUINFO_INT_REGISTER + H6280_M5:			info->i = cpustate->mmr[4];				break;
		case CPUINFO_INT_REGISTER + H6280_M6:			info->i = cpustate->mmr[5];				break;
		case CPUINFO_INT_REGISTER + H6280_M7:			info->i = cpustate->mmr[6];				break;
		case CPUINFO_INT_REGISTER + H6280_M8:			info->i = cpustate->mmr[7];				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(h6280);		break;
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(h6280);				break;
		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(h6280);			break;
		case CPUINFO_FCT_EXIT:							info->exit = CPU_EXIT_NAME(h6280);				break;
		case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(h6280);		break;
		case CPUINFO_FCT_BURN:							info->burn = NULL;								break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(h6280);break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &cpustate->ICount;				break;
		case CPUINFO_FCT_TRANSLATE:						info->translate = CPU_TRANSLATE_NAME(h6280);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "HuC6280");			break;
		case CPUINFO_STR_FAMILY:					strcpy(info->s, "Hudsonsoft 6280");	break;
		case CPUINFO_STR_VERSION:					strcpy(info->s, "1.11");			break;
		case CPUINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);			break;
		case CPUINFO_STR_CREDITS:					strcpy(info->s, "Copyright Bryan McPhail, mish@tendril.co.uk"); break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "%c%c%c%c%c%c%c%c",
				cpustate->p & 0x80 ? 'N':'.',
				cpustate->p & 0x40 ? 'V':'.',
				cpustate->p & 0x20 ? 'R':'.',
				cpustate->p & 0x10 ? 'B':'.',
				cpustate->p & 0x08 ? 'D':'.',
				cpustate->p & 0x04 ? 'I':'.',
				cpustate->p & 0x02 ? 'Z':'.',
				cpustate->p & 0x01 ? 'C':'.');
			break;

		case CPUINFO_STR_REGISTER + H6280_PC:			sprintf(info->s, "PC:%04X", cpustate->pc.d);	break;
        case CPUINFO_STR_REGISTER + H6280_S:			sprintf(info->s, "S:%02X", cpustate->sp.b.l);	break;
        case CPUINFO_STR_REGISTER + H6280_P:			sprintf(info->s, "P:%02X", cpustate->p);		break;
        case CPUINFO_STR_REGISTER + H6280_A:			sprintf(info->s, "A:%02X", cpustate->a);		break;
		case CPUINFO_STR_REGISTER + H6280_X:			sprintf(info->s, "X:%02X", cpustate->x);		break;
		case CPUINFO_STR_REGISTER + H6280_Y:			sprintf(info->s, "Y:%02X", cpustate->y);		break;
		case CPUINFO_STR_REGISTER + H6280_IRQ_MASK:		sprintf(info->s, "IM:%02X", cpustate->irq_mask);break;
		case CPUINFO_STR_REGISTER + H6280_TIMER_STATE:	sprintf(info->s, "TMR:%02X", cpustate->timer_status); break;
		case CPUINFO_STR_REGISTER + H6280_NMI_STATE:	sprintf(info->s, "NMI:%X", cpustate->nmi_state);	  break;
		case CPUINFO_STR_REGISTER + H6280_IRQ1_STATE:	sprintf(info->s, "IRQ1:%X", cpustate->irq_state[0]);  break;
		case CPUINFO_STR_REGISTER + H6280_IRQ2_STATE:	sprintf(info->s, "IRQ2:%X", cpustate->irq_state[1]);  break;
		case CPUINFO_STR_REGISTER + H6280_IRQT_STATE:	sprintf(info->s, "IRQT:%X", cpustate->irq_state[2]);  break;
		case CPUINFO_STR_REGISTER + H6280_M1:			sprintf(info->s, "M1:%02X", cpustate->mmr[0]); break;
		case CPUINFO_STR_REGISTER + H6280_M2:			sprintf(info->s, "M2:%02X", cpustate->mmr[1]); break;
		case CPUINFO_STR_REGISTER + H6280_M3:			sprintf(info->s, "M3:%02X", cpustate->mmr[2]); break;
		case CPUINFO_STR_REGISTER + H6280_M4:			sprintf(info->s, "M4:%02X", cpustate->mmr[3]); break;
		case CPUINFO_STR_REGISTER + H6280_M5:			sprintf(info->s, "M5:%02X", cpustate->mmr[4]); break;
		case CPUINFO_STR_REGISTER + H6280_M6:			sprintf(info->s, "M6:%02X", cpustate->mmr[5]); break;
		case CPUINFO_STR_REGISTER + H6280_M7:			sprintf(info->s, "M7:%02X", cpustate->mmr[6]); break;
		case CPUINFO_STR_REGISTER + H6280_M8:			sprintf(info->s, "M8:%02X", cpustate->mmr[7]); break;
	}
}

DEFINE_LEGACY_CPU_DEVICE(H6280, h6280);
