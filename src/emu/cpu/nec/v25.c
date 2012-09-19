/****************************************************************************

    NEC V25/V35 emulator

    ---------------------------------------------

    TODO:

    Using V20/V30 cycle counts for now. V25/V35 cycle counts
    vary based on whether internal RAM access is enabled (RAMEN).
    Likewise, the programmable clock divider (PCK) currently only
    affects the timers, not instruction execution.

    BTCLR and STOP instructions not implemented.

    IBRK flag (trap I/O instructions) not implemented.

    Interrupt macro service function not implemented.

    Port implementation is incomplete: mode control registers are ignored.

    Timer implementation is incomplete: polling is not implemented
    (reading any of the registers just returns the last value written)

    Serial interface and DMA functions not implemented.
    Note that these functions differ considerably between
    the V25/35 and the V25+/35+.

    Make internal RAM into a real RAM region, and use an
    internal address map (remapped when IDB is written to)
    instead of memory access wrapper functions.
    That way the internal RAM would be visible to the debugger,
    among other benefits.

****************************************************************************/

#include "emu.h"
#include "debugger.h"

typedef UINT8 BOOLEAN;
typedef UINT8 BYTE;
typedef UINT16 WORD;
typedef UINT32 DWORD;

#include "nec.h"
#include "v25priv.h"

/* default configuration */
static const nec_config default_config =
{
	NULL
};

extern int necv_dasm_one(char *buffer, UINT32 eip, const UINT8 *oprom, const nec_config *config);

INLINE v25_state_t *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == V25 || device->type() == V35);
	return (v25_state_t *)downcast<legacy_cpu_device *>(device)->token();
}

static TIMER_CALLBACK(v25_timer_callback)
{
	v25_state_t *nec_state = (v25_state_t *)ptr;
	nec_state->pending_irq |= param;
}

INLINE void prefetch(v25_state_t *nec_state)
{
	nec_state->prefetch_count--;
}

static void do_prefetch(v25_state_t *nec_state, int previous_ICount)
{
	int diff = previous_ICount - (int) nec_state->icount;

	/* The implementation is not accurate, but comes close.
     * It does not respect that the V30 will fetch two bytes
     * at once directly, but instead uses only 2 cycles instead
     * of 4. There are however only very few sources publicly
     * available and they are vague.
     */
	while (nec_state->prefetch_count<0)
	{
		nec_state->prefetch_count++;
		if (diff>nec_state->prefetch_cycles)
			diff -= nec_state->prefetch_cycles;
		else
			nec_state->icount -= nec_state->prefetch_cycles;
	}

	if (nec_state->prefetch_reset)
	{
		nec_state->prefetch_count = 0;
		nec_state->prefetch_reset = 0;
		return;
	}

	while (diff>=nec_state->prefetch_cycles && nec_state->prefetch_count < nec_state->prefetch_size)
	{
		diff -= nec_state->prefetch_cycles;
		nec_state->prefetch_count++;
	}

}

INLINE UINT8 fetch(v25_state_t *nec_state)
{
	prefetch(nec_state);
	return nec_state->direct->read_raw_byte((Sreg(PS)<<4)+nec_state->ip++, nec_state->fetch_xor);
}

INLINE UINT16 fetchword(v25_state_t *nec_state)
{
	UINT16 r = FETCH();
	r |= (FETCH()<<8);
	return r;
}

#define nec_state_t v25_state_t

#include "v25instr.h"
#include "necmacro.h"
#include "necea.h"
#include "necmodrm.h"

static UINT8 parity_table[256];

static UINT8 fetchop(v25_state_t *nec_state)
{
	UINT8 ret;

	prefetch(nec_state);
	ret = nec_state->direct->read_decrypted_byte(( Sreg(PS)<<4)+nec_state->ip++, nec_state->fetch_xor);

	if (nec_state->MF == 0)
		if (nec_state->config->v25v35_decryptiontable)
		{
			ret = nec_state->config->v25v35_decryptiontable[ret];
		}
	return ret;
}



/***************************************************************************/

static CPU_RESET( v25 )
{
	v25_state_t *nec_state = get_safe_token(device);
	int tmp;
	attotime time;

	nec_state->ip = 0;
	nec_state->IBRK = 1;
	nec_state->F0 = 0;
	nec_state->F1 = 0;
	nec_state->TF = 0;
	nec_state->IF = 0;
	nec_state->DF = 0;
	nec_state->SignVal = 0;
	nec_state->AuxVal = 0;
	nec_state->OverVal = 0;
	nec_state->ZeroVal = 1;
	nec_state->CarryVal = 0;
	nec_state->ParityVal = 1;
	nec_state->pending_irq = 0;
	nec_state->unmasked_irq = INT_IRQ | NMI_IRQ;
	nec_state->bankswitch_irq = 0;
	nec_state->priority_inttu = 7;
	nec_state->priority_intd = 7;
	nec_state->priority_intp = 7;
	nec_state->priority_ints0 = 7;
	nec_state->priority_ints1 = 7;
	nec_state->IRQS = nec_state->ISPR = 0;
	nec_state->nmi_state = 0;
	nec_state->irq_state = 0;
	nec_state->poll_state = 1;
	nec_state->mode_state = nec_state->MF = (nec_state->config->v25v35_decryptiontable) ? 0 : 1;
	nec_state->intp_state[0] = 0;
	nec_state->intp_state[1] = 0;
	nec_state->intp_state[2] = 0;
	nec_state->halted = 0;

	nec_state->TM0 = nec_state->MD0 = nec_state->TM1 = nec_state->MD1 = 0;
	nec_state->TMC0 = nec_state->TMC1 = 0;

	nec_state->RAMEN = 1;
	nec_state->TB = 20;
	nec_state->PCK = 8;
	nec_state->IDB = 0xFFE00;

	tmp = nec_state->PCK << nec_state->TB;
	time = attotime::from_hz(nec_state->device->unscaled_clock()) * tmp;
	nec_state->timers[3]->adjust(time, INTTB, time);

	nec_state->timers[0]->adjust(attotime::never);
	nec_state->timers[1]->adjust(attotime::never);
	nec_state->timers[2]->adjust(attotime::never);

	SetRB(7);
	Sreg(PS) = 0xffff;
	Sreg(SS) = 0;
	Sreg(DS0) = 0;
	Sreg(DS1) = 0;

	CHANGE_PC;
}

static CPU_EXIT( v25 )
{

}

static void nec_interrupt(v25_state_t *nec_state, unsigned int_num, INTSOURCES source)
{
    UINT32 dest_seg, dest_off;

    i_pushf(nec_state);
	nec_state->TF = nec_state->IF = 0;
	nec_state->MF = nec_state->mode_state;

	switch(source)
	{
		case BRKN:	/* force native mode */
			nec_state->MF = 1;
			break;
		case BRKS:	/* force secure mode */
			if (nec_state->config->v25v35_decryptiontable)
				nec_state->MF = 0;
			else
				logerror("%06x: BRKS executed with no decryption table\n",PC(nec_state));
			break;
		case INT_IRQ:	/* get vector */
			int_num = (*nec_state->irq_callback)(nec_state->device, 0);
			break;
		default:
			break;
	}

    dest_off = read_mem_word(int_num*4);
    dest_seg = read_mem_word(int_num*4+2);

	PUSH(Sreg(PS));
	PUSH(nec_state->ip);
	nec_state->ip = (WORD)dest_off;
	Sreg(PS) = (WORD)dest_seg;
	CHANGE_PC;
}

static void nec_bankswitch(v25_state_t *nec_state, unsigned bank_num)
{
	int tmp = CompressFlags();

	nec_state->TF = nec_state->IF = 0;
	nec_state->MF = nec_state->mode_state;

	SetRB(bank_num);

	Wreg(PSW_SAVE) = tmp;
	Wreg(PC_SAVE) = nec_state->ip;
	nec_state->ip = Wreg(VECTOR_PC);
	CHANGE_PC;
}

static void nec_trap(v25_state_t *nec_state)
{
	nec_instruction[fetchop(nec_state)](nec_state);
	nec_interrupt(nec_state, NEC_TRAP_VECTOR, BRK);
}

#define INTERRUPT(source, vector, priority) \
	if(pending & (source)) {				\
		nec_state->IRQS = vector;				\
		nec_state->ISPR |= (1 << (priority));	\
		nec_state->pending_irq &= ~(source);	\
		if(nec_state->bankswitch_irq & (source))	\
			nec_bankswitch(nec_state, priority);	\
		else									\
			nec_interrupt(nec_state, vector, source);	\
		break;	/* break out of loop */	\
	}

/* interrupt sources subject to priority control */
#define SOURCES	(INTTU0 | INTTU1 | INTTU2 | INTD0 | INTD1 | INTP0 | INTP1 | INTP2 \
				| INTSER0 | INTSR0 | INTST0 | INTSER1 | INTSR1 | INTST1 | INTTB)

static void external_int(v25_state_t *nec_state)
{
	int pending = nec_state->pending_irq & nec_state->unmasked_irq;

	if (pending & NMI_IRQ)
	{
		nec_interrupt(nec_state, NEC_NMI_VECTOR, NMI_IRQ);
		nec_state->pending_irq &= ~NMI_IRQ;
	}
	else if (pending & SOURCES)
	{
		for(int i = 0; i < 8; i++)
		{
			if (nec_state->ISPR & (1 << i)) break;

			if (nec_state->priority_inttu == i)
			{
				INTERRUPT(INTTU0, NEC_INTTU0_VECTOR, i)
				INTERRUPT(INTTU1, NEC_INTTU1_VECTOR, i)
				INTERRUPT(INTTU2, NEC_INTTU2_VECTOR, i)
			}

			if (nec_state->priority_intd == i)
			{
				INTERRUPT(INTD0, NEC_INTD0_VECTOR, i)
				INTERRUPT(INTD1, NEC_INTD1_VECTOR, i)
			}

			if (nec_state->priority_intp == i)
			{
				INTERRUPT(INTP0, NEC_INTP0_VECTOR, i)
				INTERRUPT(INTP1, NEC_INTP1_VECTOR, i)
				INTERRUPT(INTP2, NEC_INTP2_VECTOR, i)
			}

			if (nec_state->priority_ints0 == i)
			{
				INTERRUPT(INTSER0, NEC_INTSER0_VECTOR, i)
				INTERRUPT(INTSR0, NEC_INTSR0_VECTOR, i)
				INTERRUPT(INTST0, NEC_INTST0_VECTOR, i)
			}

			if (nec_state->priority_ints1 == i)
			{
				INTERRUPT(INTSER1, NEC_INTSER1_VECTOR, i)
				INTERRUPT(INTSR1, NEC_INTSR1_VECTOR, i)
				INTERRUPT(INTST1, NEC_INTST1_VECTOR, i)
			}

			if (i == 7)
				INTERRUPT(INTTB, NEC_INTTB_VECTOR, 7)
		}
	}
	else if (pending & INT_IRQ)
	{
		/* the actual vector is retrieved after pushing flags */
		/* and clearing the IF */
		nec_interrupt(nec_state, (UINT32)-1, INT_IRQ);
		nec_state->irq_state = CLEAR_LINE;
		nec_state->pending_irq &= ~INT_IRQ;
	}
}

/****************************************************************************/
/*                             OPCODES                                      */
/****************************************************************************/

#include "necinstr.c"
#include "v25instr.c"

/*****************************************************************************/

static void set_irq_line(v25_state_t *nec_state, int irqline, int state)
{
	switch (irqline)
	{
		case 0:
			nec_state->irq_state = state;
			if (state == CLEAR_LINE)
				nec_state->pending_irq &= ~INT_IRQ;
			else
			{
				nec_state->pending_irq |= INT_IRQ;
				nec_state->halted = 0;
			}
			break;
		case INPUT_LINE_NMI:
			if (nec_state->nmi_state == state) return;
		    nec_state->nmi_state = state;
			if (state != CLEAR_LINE)
			{
				nec_state->pending_irq |= NMI_IRQ;
				nec_state->halted = 0;
			}
			break;
		case NEC_INPUT_LINE_INTP0:
		case NEC_INPUT_LINE_INTP1:
		case NEC_INPUT_LINE_INTP2:
			irqline -= NEC_INPUT_LINE_INTP0;
			if (nec_state->intp_state[irqline] == state) return;
			nec_state->intp_state[irqline] = state;
			if (state != CLEAR_LINE)
				nec_state->pending_irq |= (INTP0 << irqline);
			break;
		case NEC_INPUT_LINE_POLL:
			nec_state->poll_state = state;
			break;
	}
}

static CPU_DISASSEMBLE( v25 )
{
	v25_state_t *nec_state = get_safe_token(device);

	return necv_dasm_one(buffer, pc, oprom, nec_state->config);
}

static void v25_init(legacy_cpu_device *device, device_irq_acknowledge_callback irqcallback)
{
	const nec_config *config = device->static_config() ? (const nec_config *)device->static_config() : &default_config;
	v25_state_t *nec_state = get_safe_token(device);

	unsigned int i, j, c;

	static const WREGS wreg_name[8]={ AW, CW, DW, BW, SP, BP, IX, IY };
	static const BREGS breg_name[8]={ AL, CL, DL, BL, AH, CH, DH, BH };

	for (i = 0; i < 256; i++)
	{
		for (j = i, c = 0; j > 0; j >>= 1)
			if (j & 1) c++;
		parity_table[i] = !(c & 1);
	}

	for (i = 0; i < 256; i++)
	{
		Mod_RM.reg.b[i] = breg_name[(i & 0x38) >> 3];
		Mod_RM.reg.w[i] = wreg_name[(i & 0x38) >> 3];
	}

	for (i = 0xc0; i < 0x100; i++)
	{
		Mod_RM.RM.w[i] = wreg_name[i & 7];
		Mod_RM.RM.b[i] = breg_name[i & 7];
	}

	memset(nec_state, 0, sizeof(*nec_state));

	nec_state->config = config;

	for (i = 0; i < 4; i++)
		nec_state->timers[i] = device->machine().scheduler().timer_alloc(FUNC(v25_timer_callback), nec_state);

	device->save_item(NAME(nec_state->ram.w));
	device->save_item(NAME(nec_state->intp_state));

	device->save_item(NAME(nec_state->ip));
	device->save_item(NAME(nec_state->IBRK));
	device->save_item(NAME(nec_state->F0));
	device->save_item(NAME(nec_state->F1));
	device->save_item(NAME(nec_state->TF));
	device->save_item(NAME(nec_state->IF));
	device->save_item(NAME(nec_state->DF));
	device->save_item(NAME(nec_state->MF));
	device->save_item(NAME(nec_state->RBW));
	device->save_item(NAME(nec_state->RBB));
	device->save_item(NAME(nec_state->SignVal));
	device->save_item(NAME(nec_state->AuxVal));
	device->save_item(NAME(nec_state->OverVal));
	device->save_item(NAME(nec_state->ZeroVal));
	device->save_item(NAME(nec_state->CarryVal));
	device->save_item(NAME(nec_state->ParityVal));
	device->save_item(NAME(nec_state->pending_irq));
	device->save_item(NAME(nec_state->unmasked_irq));
	device->save_item(NAME(nec_state->bankswitch_irq));
	device->save_item(NAME(nec_state->priority_inttu));
	device->save_item(NAME(nec_state->priority_intd));
	device->save_item(NAME(nec_state->priority_intp));
	device->save_item(NAME(nec_state->priority_ints0));
	device->save_item(NAME(nec_state->priority_ints1));
	device->save_item(NAME(nec_state->IRQS));
	device->save_item(NAME(nec_state->ISPR));
	device->save_item(NAME(nec_state->nmi_state));
	device->save_item(NAME(nec_state->irq_state));
	device->save_item(NAME(nec_state->poll_state));
	device->save_item(NAME(nec_state->mode_state));
	device->save_item(NAME(nec_state->halted));
	device->save_item(NAME(nec_state->TM0));
	device->save_item(NAME(nec_state->MD0));
	device->save_item(NAME(nec_state->TM1));
	device->save_item(NAME(nec_state->MD1));
	device->save_item(NAME(nec_state->TMC0));
	device->save_item(NAME(nec_state->TMC1));
	device->save_item(NAME(nec_state->RAMEN));
	device->save_item(NAME(nec_state->TB));
	device->save_item(NAME(nec_state->PCK));
	device->save_item(NAME(nec_state->IDB));

	nec_state->irq_callback = irqcallback;
	nec_state->device = device;
	nec_state->program = &device->space(AS_PROGRAM);
	nec_state->direct = &nec_state->program->direct();
	nec_state->io = &device->space(AS_IO);
}



static CPU_EXECUTE( v25 )
{
	v25_state_t *nec_state = get_safe_token(device);
	int prev_ICount;

	int pending = nec_state->pending_irq & nec_state->unmasked_irq;

	if (nec_state->halted && pending)
	{
		for(int i = 0; i < 8; i++)
		{
			if (nec_state->ISPR & (1 << i)) break;

			if (nec_state->priority_inttu == i && (pending & (INTTU0|INTTU1|INTTU2)))
				nec_state->halted = 0;

			if (nec_state->priority_intd == i && (pending & (INTD0|INTD1)))
				nec_state->halted = 0;

			if (nec_state->priority_intp == i && (pending & (INTP0|INTP1|INTP2)))
				nec_state->halted = 0;

			if (nec_state->priority_ints0 == i && (pending & (INTSER0|INTSR0|INTST0)))
				nec_state->halted = 0;

			if (nec_state->priority_ints1 == i && (pending & (INTSER1|INTSR1|INTST1)))
				nec_state->halted = 0;

			if (i == 7 && (pending & INTTB))
				nec_state->halted = 0;
		}
	}

	if (nec_state->halted)
	{
		nec_state->icount = 0;
		debugger_instruction_hook(device, (Sreg(PS)<<4) + nec_state->ip);
		return;
	}

	while(nec_state->icount>0) {
		/* Dispatch IRQ */
		if (nec_state->no_interrupt==0 && (nec_state->pending_irq & nec_state->unmasked_irq))
		{
			if (nec_state->pending_irq & NMI_IRQ)
				external_int(nec_state);
			else if (nec_state->IF)
				external_int(nec_state);
		}

		/* No interrupt allowed between last instruction and this one */
		if (nec_state->no_interrupt)
			nec_state->no_interrupt--;

		debugger_instruction_hook(device, (Sreg(PS)<<4) + nec_state->ip);
		prev_ICount = nec_state->icount;
		nec_instruction[fetchop(nec_state)](nec_state);
		do_prefetch(nec_state, prev_ICount);
    }
}

/* Wrappers for the different CPU types */
static CPU_INIT( v25 )
{
	v25_state_t *nec_state = get_safe_token(device);

	v25_init(device, irqcallback);
	nec_state->fetch_xor = 0;
	nec_state->chip_type=V20_TYPE;
	nec_state->prefetch_size = 4;		/* 3 words */
	nec_state->prefetch_cycles = 4;		/* four cycles per byte */
}

static CPU_INIT( v35 )
{
	v25_state_t *nec_state = get_safe_token(device);

	v25_init(device, irqcallback);
	nec_state->fetch_xor = BYTE_XOR_LE(0);
	nec_state->chip_type=V30_TYPE;
	nec_state->prefetch_size = 6;		/* 3 words */
	nec_state->prefetch_cycles = 2;		/* two cycles per byte / four per word */

}



/**************************************************************************
 * Generic set_info
 **************************************************************************/

static CPU_SET_INFO( v25 )
{
	v25_state_t *nec_state = get_safe_token(device);

	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + 0:						set_irq_line(nec_state, 0, info->i);				break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:			set_irq_line(nec_state, INPUT_LINE_NMI, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + NEC_INPUT_LINE_INTP0:	set_irq_line(nec_state, NEC_INPUT_LINE_INTP0, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + NEC_INPUT_LINE_INTP1:	set_irq_line(nec_state, NEC_INPUT_LINE_INTP1, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + NEC_INPUT_LINE_INTP2:	set_irq_line(nec_state, NEC_INPUT_LINE_INTP2, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + NEC_INPUT_LINE_POLL:		set_irq_line(nec_state, NEC_INPUT_LINE_POLL, info->i);	break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + NEC_PC:
			if( info->i - (Sreg(PS)<<4) < 0x10000 )
			{
				nec_state->ip = info->i - (Sreg(PS)<<4);
			}
			else
			{
				Sreg(PS) = info->i >> 4;
				nec_state->ip = info->i & 0x0000f;
			}
			break;
		case CPUINFO_INT_REGISTER + NEC_IP:				nec_state->ip = info->i;							break;
		case CPUINFO_INT_SP:
			if( info->i - (Sreg(SS)<<4) < 0x10000 )
			{
				Wreg(SP) = info->i - (Sreg(SS)<<4);
			}
			else
			{
				Sreg(SS) = info->i >> 4;
				Wreg(SP) = info->i & 0x0000f;
			}
			break;
		case CPUINFO_INT_REGISTER + NEC_SP:				Wreg(SP) = info->i;					break;
		case CPUINFO_INT_REGISTER + NEC_FLAGS:			ExpandFlags(info->i);					break;
		case CPUINFO_INT_REGISTER + NEC_AW:				Wreg(AW) = info->i;					break;
		case CPUINFO_INT_REGISTER + NEC_CW:				Wreg(CW) = info->i;					break;
		case CPUINFO_INT_REGISTER + NEC_DW:				Wreg(DW) = info->i;					break;
		case CPUINFO_INT_REGISTER + NEC_BW:				Wreg(BW) = info->i;					break;
		case CPUINFO_INT_REGISTER + NEC_BP:				Wreg(BP) = info->i;					break;
		case CPUINFO_INT_REGISTER + NEC_IX:				Wreg(IX) = info->i;					break;
		case CPUINFO_INT_REGISTER + NEC_IY:				Wreg(IY) = info->i;					break;
		case CPUINFO_INT_REGISTER + NEC_ES:				Sreg(DS1) = info->i;					break;
		case CPUINFO_INT_REGISTER + NEC_CS:				Sreg(PS) = info->i;					break;
		case CPUINFO_INT_REGISTER + NEC_SS:				Sreg(SS) = info->i;					break;
		case CPUINFO_INT_REGISTER + NEC_DS:				Sreg(DS0) = info->i;					break;
	}
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

static CPU_GET_INFO( v25v35 )
{
	v25_state_t *nec_state = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;
	int flags;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(v25_state_t);					break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 1;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0xff;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_LITTLE;					break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 2;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 8;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 80;							break;

		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM:	info->i = 20;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM:	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_DATA:	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_IO:		info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_IO:		info->i = 17;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_IO:		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + 0:						info->i = (nec_state->pending_irq & INT_IRQ) ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:			info->i = nec_state->nmi_state;				break;
		case CPUINFO_INT_INPUT_STATE + NEC_INPUT_LINE_INTP0:	info->i = nec_state->intp_state[0];			break;
		case CPUINFO_INT_INPUT_STATE + NEC_INPUT_LINE_INTP1:	info->i = nec_state->intp_state[1];			break;
		case CPUINFO_INT_INPUT_STATE + NEC_INPUT_LINE_INTP2:	info->i = nec_state->intp_state[2];			break;
		case CPUINFO_INT_INPUT_STATE + NEC_INPUT_LINE_POLL:		info->i = nec_state->poll_state;			break;

		case CPUINFO_INT_PREVIOUSPC:					/* not supported */						break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + NEC_PC:				info->i = ((Sreg(PS)<<4) + nec_state->ip);	break;
		case CPUINFO_INT_REGISTER + NEC_IP:				info->i = nec_state->ip;							break;
		case CPUINFO_INT_SP:							info->i = (Sreg(SS)<<4) + Wreg(SP); break;
		case CPUINFO_INT_REGISTER + NEC_SP:				info->i = Wreg(SP);					break;
		case CPUINFO_INT_REGISTER + NEC_FLAGS:			info->i = CompressFlags();				break;
		case CPUINFO_INT_REGISTER + NEC_AW:				info->i = Wreg(AW);					break;
		case CPUINFO_INT_REGISTER + NEC_CW:				info->i = Wreg(CW);					break;
		case CPUINFO_INT_REGISTER + NEC_DW:				info->i = Wreg(DW);					break;
		case CPUINFO_INT_REGISTER + NEC_BW:				info->i = Wreg(BW);					break;
		case CPUINFO_INT_REGISTER + NEC_BP:				info->i = Wreg(BP);					break;
		case CPUINFO_INT_REGISTER + NEC_IX:				info->i = Wreg(IX);					break;
		case CPUINFO_INT_REGISTER + NEC_IY:				info->i = Wreg(IY);					break;
		case CPUINFO_INT_REGISTER + NEC_ES:				info->i = Sreg(DS1);					break;
		case CPUINFO_INT_REGISTER + NEC_CS:				info->i = Sreg(PS);					break;
		case CPUINFO_INT_REGISTER + NEC_SS:				info->i = Sreg(SS);					break;
		case CPUINFO_INT_REGISTER + NEC_DS:				info->i = Sreg(DS0);					break;
		case CPUINFO_INT_REGISTER + NEC_PENDING:		info->i = nec_state->pending_irq;				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(v25);			break;
		case CPUINFO_FCT_INIT:							/* set per-CPU */						break;
		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(v25);				break;
		case CPUINFO_FCT_EXIT:							info->exit = CPU_EXIT_NAME(v25);					break;
		case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(v25);			break;
		case CPUINFO_FCT_BURN:							info->burn = NULL;						break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(v25);			break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &nec_state->icount;				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "NEC");					break;
		case CPUINFO_STR_FAMILY:					strcpy(info->s, "NEC V-Series");		break;
		case CPUINFO_STR_VERSION:					strcpy(info->s, "2.0");					break;
		case CPUINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CREDITS:					strcpy(info->s, "Bryan McPhail (V25/V35 support added by Alex W. Jackson)"); break;

		case CPUINFO_STR_FLAGS:
            flags = CompressFlags();
            sprintf(info->s, "%c %d %c%c%c%c%c%c%c%c%c%c%c%c",
                flags & 0x8000 ? 'N':'S',
                (flags & 0x7000) >> 12,
                flags & 0x0800 ? 'O':'.',
                flags & 0x0400 ? 'D':'.',
                flags & 0x0200 ? 'I':'.',
                flags & 0x0100 ? 'T':'.',
                flags & 0x0080 ? 'S':'.',
                flags & 0x0040 ? 'Z':'.',
                flags & 0x0020 ? '1':'.',
                flags & 0x0010 ? 'A':'.',
                flags & 0x0008 ? '0':'.',
                flags & 0x0004 ? 'P':'.',
                flags & 0x0002 ? '.':'I',
                flags & 0x0001 ? 'C':'.');
            break;

        case CPUINFO_STR_REGISTER + NEC_PC:				sprintf(info->s, "PC:%05X", (Sreg(PS)<<4) + nec_state->ip); break;
        case CPUINFO_STR_REGISTER + NEC_IP:				sprintf(info->s, "IP:%04X", nec_state->ip); break;
        case CPUINFO_STR_REGISTER + NEC_SP:				sprintf(info->s, "SP:%04X", Wreg(SP)); break;
        case CPUINFO_STR_REGISTER + NEC_FLAGS:			sprintf(info->s, "F:%04X", CompressFlags()); break;
        case CPUINFO_STR_REGISTER + NEC_AW:				sprintf(info->s, "AW:%04X", Wreg(AW)); break;
        case CPUINFO_STR_REGISTER + NEC_CW:				sprintf(info->s, "CW:%04X", Wreg(CW)); break;
        case CPUINFO_STR_REGISTER + NEC_DW:				sprintf(info->s, "DW:%04X", Wreg(DW)); break;
        case CPUINFO_STR_REGISTER + NEC_BW:				sprintf(info->s, "BW:%04X", Wreg(BW)); break;
        case CPUINFO_STR_REGISTER + NEC_BP:				sprintf(info->s, "BP:%04X", Wreg(BP)); break;
        case CPUINFO_STR_REGISTER + NEC_IX:				sprintf(info->s, "IX:%04X", Wreg(IX)); break;
        case CPUINFO_STR_REGISTER + NEC_IY:				sprintf(info->s, "IY:%04X", Wreg(IY)); break;
        case CPUINFO_STR_REGISTER + NEC_ES:				sprintf(info->s, "DS1:%04X", Sreg(DS1)); break;
        case CPUINFO_STR_REGISTER + NEC_CS:				sprintf(info->s, "PS:%04X", Sreg(PS)); break;
        case CPUINFO_STR_REGISTER + NEC_SS:				sprintf(info->s, "SS:%04X", Sreg(SS)); break;
        case CPUINFO_STR_REGISTER + NEC_DS:				sprintf(info->s, "DS0:%04X", Sreg(DS0)); break;
	}
}


/**************************************************************************
 * CPU-specific set_info
 **************************************************************************/

CPU_GET_INFO( v25 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 8;					break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_IO:		info->i = 8;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(v25);					break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "V25");					break;

		default:										CPU_GET_INFO_CALL(v25v35);				break;
	}
}


/**************************************************************************
 * CPU-specific set_info
 **************************************************************************/

CPU_GET_INFO( v35 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 16;					break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_IO:		info->i = 16;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(v35);					break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "V35");					break;

		default:										CPU_GET_INFO_CALL(v25v35);				break;
	}
}

DEFINE_LEGACY_CPU_DEVICE(V25, v25);
DEFINE_LEGACY_CPU_DEVICE(V35, v35);
