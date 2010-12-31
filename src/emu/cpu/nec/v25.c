/****************************************************************************

    NEC V25/V35 emulator

    ---------------------------------------------

    TODO:

    Using V20/V30 cycle counts for now. Cycle counts change
    depending on whether the internal RAM access is enabled.
    Also, the clock divider can be changed to 1/2, 1/4 or 1/8.

    IBRK flag (trap I/O instructions) included but not implemented.

    Most special function registers not implemented yet.

    It would be nice if the internal ram area was viewable in the debugger.

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

/* The interrupt number of a pending external interrupt pending NMI is 2.   */
/* For INTR interrupts, the level is caught on the bus during an INTA cycle */

#define INT_IRQ 0x01
#define NMI_IRQ 0x02

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
	unsigned int i,j,c;

	static const WREGS wreg_name[8]={ AW, CW, DW, BW, SP, BP, IX, IY };
	static const BREGS breg_name[8]={ AL, CL, DL, BL, AH, CH, DH, BH };

	memset( &nec_state->ram.w, 0, sizeof(nec_state->ram.w));

	nec_state->ip = 0;
	nec_state->IBRK = 1;
	nec_state->F0 = 0;
	nec_state->F1 = 0;
	nec_state->TF = 0;
	nec_state->IF = 0;
	nec_state->DF = 0;
	nec_state->RB = 7;
	nec_state->SignVal = 0;
	nec_state->int_vector = 0;
	nec_state->pending_irq = 0;
	nec_state->nmi_state = 0;
	nec_state->irq_state = 0;
	nec_state->poll_state = 0;
	nec_state->mode_state = nec_state->MF = (nec_state->config->v25v35_decryptiontable) ? 0 : 1;
	nec_state->AuxVal = 0;
	nec_state->OverVal = 0;
	nec_state->ZeroVal = 0;
	nec_state->CarryVal = 0;
	nec_state->ParityVal = 0;

	nec_state->PRC = 0x4E;
	nec_state->IDB = 0xFF;

	Sreg(PS) = 0xffff;

	CHANGE_PC;

    for (i = 0;i < 256; i++)
    {
		for (j = i, c = 0; j > 0; j >>= 1)
			if (j & 1) c++;
		parity_table[i] = !(c & 1);
    }

	nec_state->ZeroVal = nec_state->ParityVal = 1;

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

	nec_state->poll_state = 1;
}

static CPU_EXIT( v25 )
{

}

static void nec_interrupt(v25_state_t *nec_state, unsigned int_num, unsigned mode)
{
    UINT32 dest_seg, dest_off;

    i_pushf(nec_state);
	nec_state->TF = nec_state->IF = 0;

	switch(mode)
	{
		case 0:
			nec_state->MF = nec_state->mode_state;
			break;
		case 1:	/* BRKN: force native mode */
			nec_state->MF = 1;
			break;
		case 2:	/* BRKS: force secure mode */
			if (nec_state->config->v25v35_decryptiontable)
				nec_state->MF = 0;
			else
				logerror("%06x: BRKS executed with no decryption table\n",PC(nec_state));
	}

	if (int_num == -1)
	{
		int_num = (*nec_state->irq_callback)(nec_state->device, 0);

		nec_state->irq_state = CLEAR_LINE;
		nec_state->pending_irq &= ~INT_IRQ;
	}

    dest_off = read_mem_word(int_num*4);
    dest_seg = read_mem_word(int_num*4+2);

	PUSH(Sreg(PS));
	PUSH(nec_state->ip);
	nec_state->ip = (WORD)dest_off;
	Sreg(PS) = (WORD)dest_seg;
	CHANGE_PC;
}

static void nec_trap(v25_state_t *nec_state)
{
	nec_instruction[fetchop(nec_state)](nec_state);
	nec_interrupt(nec_state, 1,0);
}

static void external_int(v25_state_t *nec_state)
{
	if( nec_state->pending_irq & NMI_IRQ )
	{
		nec_interrupt(nec_state, NEC_NMI_INT_VECTOR,0);
		nec_state->pending_irq &= ~NMI_IRQ;
	}
	else if( nec_state->pending_irq )
	{
		/* the actual vector is retrieved after pushing flags */
		/* and clearing the IF */
		nec_interrupt(nec_state, (UINT32)-1,0);
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
	if (irqline == INPUT_LINE_NMI)
	{
		if( nec_state->nmi_state == state ) return;
	    nec_state->nmi_state = state;
		if (state != CLEAR_LINE)
		{
			nec_state->pending_irq |= NMI_IRQ;
		}
	}
	else
	{
		nec_state->irq_state = state;
		if (state == CLEAR_LINE)
		{
	//      if (!nec_state->IF)  NS010718 fix interrupt request loss
				nec_state->pending_irq &= ~INT_IRQ;
		}
		else
		{
	//      if (nec_state->IF)   NS010718 fix interrupt request loss
				nec_state->pending_irq |= INT_IRQ;
		}
	}
}

static void set_poll_line(v25_state_t *nec_state, int state)
{
	nec_state->poll_state = state;
}

static CPU_DISASSEMBLE( v25 )
{
	v25_state_t *nec_state = get_safe_token(device);

	return necv_dasm_one(buffer, pc, oprom, nec_state->config);
}

static void v25_init(legacy_cpu_device *device, device_irq_callback irqcallback, int type)
{
	const nec_config *config = device->baseconfig().static_config() ? (const nec_config *)device->baseconfig().static_config() : &default_config;
	v25_state_t *nec_state = get_safe_token(device);

	nec_state->config = config;


	state_save_register_device_item_array(device, 0, nec_state->ram.w);

	state_save_register_device_item(device, 0, nec_state->ip);
	state_save_register_device_item(device, 0, nec_state->IBRK);
	state_save_register_device_item(device, 0, nec_state->F0);
	state_save_register_device_item(device, 0, nec_state->F1);
	state_save_register_device_item(device, 0, nec_state->TF);
	state_save_register_device_item(device, 0, nec_state->IF);
	state_save_register_device_item(device, 0, nec_state->DF);
	state_save_register_device_item(device, 0, nec_state->MF);
	state_save_register_device_item(device, 0, nec_state->RB);
	state_save_register_device_item(device, 0, nec_state->PRC);
	state_save_register_device_item(device, 0, nec_state->IDB);
	state_save_register_device_item(device, 0, nec_state->SignVal);
	state_save_register_device_item(device, 0, nec_state->int_vector);
	state_save_register_device_item(device, 0, nec_state->pending_irq);
	state_save_register_device_item(device, 0, nec_state->nmi_state);
	state_save_register_device_item(device, 0, nec_state->irq_state);
	state_save_register_device_item(device, 0, nec_state->poll_state);
	state_save_register_device_item(device, 0, nec_state->mode_state);
	state_save_register_device_item(device, 0, nec_state->AuxVal);
	state_save_register_device_item(device, 0, nec_state->OverVal);
	state_save_register_device_item(device, 0, nec_state->ZeroVal);
	state_save_register_device_item(device, 0, nec_state->CarryVal);
	state_save_register_device_item(device, 0, nec_state->ParityVal);

	nec_state->irq_callback = irqcallback;
	nec_state->device = device;
	nec_state->program = device->space(AS_PROGRAM);
	nec_state->direct = &nec_state->program->direct();
	nec_state->io = device->space(AS_IO);
}



static CPU_EXECUTE( v25 )
{
	v25_state_t *nec_state = get_safe_token(device);
	int prev_ICount;

	while(nec_state->icount>0) {
		/* Dispatch IRQ */
		if (nec_state->pending_irq && nec_state->no_interrupt==0)
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

	v25_init(device, irqcallback, 0);
	nec_state->fetch_xor = 0;
	nec_state->chip_type=V20_TYPE;
	nec_state->prefetch_size = 4;		/* 3 words */
	nec_state->prefetch_cycles = 4;		/* four cycles per byte */
}

static CPU_INIT( v35 )
{
	v25_state_t *nec_state = get_safe_token(device);

	v25_init(device, irqcallback, 1);
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
		case CPUINFO_INT_INPUT_STATE + 0:					set_irq_line(nec_state, 0, info->i);				break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:		set_irq_line(nec_state, INPUT_LINE_NMI, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + NEC_INPUT_LINE_POLL:	set_poll_line(nec_state, info->i);					break;

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
		case CPUINFO_INT_REGISTER + NEC_VECTOR:			nec_state->int_vector = info->i;					break;
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
		case DEVINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_LITTLE;					break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 8;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 80;							break;

		case DEVINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 16;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 20;					break;
		case DEVINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = 0;					break;
		case DEVINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case DEVINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case DEVINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 16;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 16;					break;
		case DEVINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO:		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + 0:					info->i = (nec_state->pending_irq & INT_IRQ) ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:		info->i = nec_state->nmi_state;				break;
		case CPUINFO_INT_INPUT_STATE + NEC_INPUT_LINE_POLL:	info->i = nec_state->poll_state;				break;

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
		case CPUINFO_INT_REGISTER + NEC_VECTOR:			info->i = nec_state->int_vector;					break;
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
		case DEVINFO_STR_NAME:							strcpy(info->s, "NEC");					break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "NEC V-Series");		break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.0");					break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);				break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Alex W. Jackson, based on NEC V emulator by Bryan McPhail"); break;

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

        case CPUINFO_STR_REGISTER + NEC_PC:				sprintf(info->s, "PC:%04X", (Sreg(PS)<<4) + nec_state->ip); break;
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
        case CPUINFO_STR_REGISTER + NEC_VECTOR:			sprintf(info->s, "V:%02X", nec_state->int_vector); break;
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
		case DEVINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 8;					break;
		case DEVINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 8;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(v25);					break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "V25");					break;

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
		case DEVINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 16;					break;
		case DEVINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 16;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(v35);					break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "V35");					break;

		default:										CPU_GET_INFO_CALL(v25v35);				break;
	}
}

DEFINE_LEGACY_CPU_DEVICE(V25, v25);
DEFINE_LEGACY_CPU_DEVICE(V35, v35);
