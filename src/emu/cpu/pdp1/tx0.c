/*
    TX-0 emulator

    Two variants:
    * initial model 64kWord RAM
    * later model 8kWord RAM

    Raphael Nabet 2004
*/

#include "cpuintrf.h"
#include "debugger.h"
#include "tx0.h"

#define LOG 0
#define LOG_EXTRA 0

static void execute_instruction_64kw(void);
static void execute_instruction_8kw(void);
static void pulse_reset(void);


/* TX-0 Registers */
typedef struct
{
	/* processor registers */
	int mbr;		/* memory buffer register (18 bits) */
	int ac;			/* accumulator (18 bits) */
	int mar;		/* memory address register (16 (64kW) or 13 (8kW) bits) */
	int pc;			/* program counter (16 (64kW) or 13 (8kW) bits) */
	int ir;			/* instruction register (2 (64kW) or 5 (8kW) bits) */
	int lr;			/* live register (18 bits) */
	int xr;			/* index register (14 bits) (8kW only) */
	int pf;			/* program flags (6 bits expandable to 10) (8kW only) */

	/* operator panel switches */
	int tbr;		/* toggle switch buffer register (18 bits) */
	int tac;		/* toggle switch accumulator (18 bits) */
	int tss[16];	/* toggle switch storage (18 bits * 16) */
	unsigned int cm_sel : 16;	/* individual cm select (1 bit * 16) */
	unsigned int lr_sel : 16;	/* individual lr select (1 bit * 16) */
	unsigned int gbl_cm_sel : 1;/* global cm select (1 bit) */
	unsigned int stop_cyc0 : 1;	/* stop on cycle 0 */
	unsigned int stop_cyc1 : 1;	/* stop on cycle 1 */

	/* processor state flip-flops */
	unsigned int run : 1;		/* processor is running */
	unsigned int rim : 1;		/* processor is in read-in mode */
	unsigned int cycle : 2;		/* 0 -> fetch */
								/* 1 -> execute (except for taken branches) */
								/* 2 -> extra execute cycle for SXA and ADO */

	unsigned int ioh : 1;		/* i-o halt: processor is executing an Input-Output Transfer wait */
	unsigned int ios : 1;		/* i-o synchronizer: set on i-o operation completion */

	/* additional emulator state variables */
	int rim_step;			/* current step in rim execution */

	int address_mask;		/* address mask */
	int ir_mask;			/* IR mask */

	/* 8 standard I/O handlers:
        0: cpy (8kW only)
        1: r1l
        2: dis
        3: r3l
        4: prt
        5: reserved
        6: p6h
        7: p7h */
	void (*io_handlers[8])(void);
	/* select instruction handler */
	void (*sel_handler)(void);
	/* called when reset line is pulsed: IO devices should reset */
	void (*io_reset_callback)(void);
}
tx0_Regs;

#define io_handler_rim 3

static tx0_Regs tx0;

#define PC		tx0.pc
#define IR		tx0.ir
#define MBR		tx0.mbr
#define MAR		tx0.mar
#define AC		tx0.ac
#define LR		tx0.lr
#define XR		tx0.xr
#define PF		tx0.pf

#define ADDRESS_MASK_64KW	0177777
#define ADDRESS_MASK_8KW	0017777

#define INCREMENT_PC_64KW	(PC = (PC+1) & ADDRESS_MASK_64KW)
#define INCREMENT_PC_8KW	(PC = (PC+1) & ADDRESS_MASK_8KW)

/* public globals */
static signed int tx0_ICount;


static int tx0_read(offs_t address)
{
	if ((address >= 16) || (tx0.gbl_cm_sel) || ((tx0.cm_sel >> address) & 1))
		/* core memory (CM) */
		return READ_TX0_18BIT(address);
	else if ((tx0.lr_sel >> address) & 1)
		/* live register (LR) */
		return LR;
	else
		/* toggle switch storage (TSS) */
		return tx0.tss[address];
}

static void tx0_write(offs_t address, int data)
{
	if ((address >= 16) || (tx0.gbl_cm_sel) || ((tx0.cm_sel >> address) & 1))
		/* core memory (CM) */
		WRITE_TX0_18BIT(address, data);
	else if ((tx0.lr_sel >> address) & 1)
		/* live register (LR) */
		LR = data;
	else
		/* toggle switch storage (TSS) */
		/* TSS is read-only */
		;
}

static void tx0_init_common(int is_64kw, int index, int clock, const void *config, int (*irqcallback)(int))
{
	int i;
	tx0_reset_param_t *param = (tx0_reset_param_t *) config;

	/* clean-up */
	memset (&tx0, 0, sizeof (tx0));

	/* set up params and callbacks */
	for (i=0; i<8; i++)
		tx0.io_handlers[i] = (param) ? param->io_handlers[i] : NULL;
	tx0.sel_handler = (param) ? param->sel_handler : NULL;
	tx0.io_reset_callback = (param) ? param->io_reset_callback : NULL;

	tx0.address_mask = is_64kw ? ADDRESS_MASK_64KW : ADDRESS_MASK_8KW;
	tx0.ir_mask = is_64kw ? 03 : 037;
}

static void tx0_init_64kw(int index, int clock, const void *config, int (*irqcallback)(int))
{
	tx0_init_common(1, index, clock, config, irqcallback);
}

static void tx0_init_8kw(int index, int clock, const void *config, int (*irqcallback)(int))
{
	tx0_init_common(0, index, clock, config, irqcallback);
}

static void tx0_reset(void)
{
	/* reset CPU flip-flops */
	pulse_reset();

	tx0.gbl_cm_sel = 1;	/* HACK */
}

static void tx0_get_context(void *dst)
{
	if (dst)
		*(tx0_Regs *) dst = tx0;
}

static void tx0_set_context(void *src)
{
	if (src)
		tx0 = *(tx0_Regs *) src;
}


/* execute instructions on this CPU until icount expires */
static int tx0_execute_64kw(int cycles)
{
	tx0_ICount = cycles;

	do
	{
		CALL_MAME_DEBUG;


		if (tx0.ioh && tx0.ios)
		{
			tx0.ioh = 0;
			tx0.ios = 0;
		}


		if ((! tx0.run) && (! tx0.rim))
			tx0_ICount = 0;	/* if processor is stopped, just burn cycles */
		else if (tx0.rim)
		{
			switch (tx0.rim_step)
			{
			case 0:
				/* read first word as instruction */
				AC = 0;
				if (tx0.io_handlers[io_handler_rim])
					(*tx0.io_handlers[io_handler_rim])();	/* data will be transferred to AC */
				tx0.rim_step = 1;
				break;

			case 1:
				if (! tx0.ios)
				{	/* transfer incomplete: wait some more */
					tx0_ICount = 0;
				}
				else
				{	/* data transfer complete */
					tx0.ios = 0;

					MBR = AC;
					IR = MBR >> 16;		/* basic opcode */
					if ((IR == 2) || (IR == 1)) 	/* trn or add instruction? */
					{
						PC = MBR & ADDRESS_MASK_64KW;
						tx0.rim = 0;	/* exit read-in mode */
						tx0.run = (IR == 2) ? 1 : 0;	/* stop if add instruction */
						tx0.rim_step = 0;
					}
					else if ((IR == 0) || (IR == 3))	/* sto or opr instruction? */
					{
						MAR = MBR & ADDRESS_MASK_64KW;
						tx0.rim_step = 2;
					}
				}
				break;

			case 2:
				/* read second word as data */
				AC = 0;
				if (tx0.io_handlers[io_handler_rim])
					(*tx0.io_handlers[io_handler_rim])();	/* data will be transferred to AC */
				tx0.rim_step = 3;
				break;

			case 3:
				if (! tx0.ios)
				{	/* transfer incomplete: wait some more */
					tx0_ICount = 0;
				}
				else
				{	/* data transfer complete */
					tx0.ios = 0;

					tx0_write(MAR, MBR = AC);

					tx0.rim_step = 0;
				}
				break;
			}
		}
		else
		{
			if (tx0.cycle == 0)
			{	/* fetch new instruction */
				MBR = tx0_read(MAR = PC);
				INCREMENT_PC_64KW;
				IR = MBR >> 16;		/* basic opcode */
				MAR = MBR & ADDRESS_MASK_64KW;
			}

			if (! tx0.ioh)
			{
				if ((tx0.stop_cyc0 && (tx0.cycle == 0))
					|| (tx0.stop_cyc1 && (tx0.cycle == 1)))
					tx0.run = 0;

				execute_instruction_64kw();	/* execute instruction */
			}

			tx0_ICount --;
		}
	}
	while (tx0_ICount > 0);

	return cycles - tx0_ICount;
}

/* execute instructions on this CPU until icount expires */
static int tx0_execute_8kw(int cycles)
{
	tx0_ICount = cycles;

	do
	{
		CALL_MAME_DEBUG;


		if (tx0.ioh && tx0.ios)
		{
			tx0.ioh = 0;
			tx0.ios = 0;
		}


		if ((! tx0.run) && (! tx0.rim))
			tx0_ICount = 0;	/* if processor is stopped, just burn cycles */
		else if (tx0.rim)
		{
			switch (tx0.rim_step)
			{
			case 0:
				/* read first word as instruction */
				AC = 0;
				if (tx0.io_handlers[io_handler_rim])
					(*tx0.io_handlers[io_handler_rim])();	/* data will be transferred to AC */
				tx0.rim_step = 1;
				break;

			case 1:
				if (! tx0.ios)
				{	/* transfer incomplete: wait some more */
					tx0_ICount = 0;
				}
				else
				{	/* data transfer complete */
					tx0.ios = 0;

					MBR = AC;
					IR = MBR >> 13;		/* basic opcode */
					if ((IR == 16) || (IR == 8)) 	/* trn or add instruction? */
					{
						PC = MBR & ADDRESS_MASK_8KW;
						tx0.rim = 0;	/* exit read-in mode */
						tx0.run = (IR == 16) ? 1 : 0;	/* stop if add instruction */
						tx0.rim_step = 0;
					}
					else if ((IR == 0) || (IR == 24))	/* sto or opr instruction? */
					{
						MAR = MBR & ADDRESS_MASK_8KW;
						tx0.rim_step = 2;
					}
				}
				break;

			case 2:
				/* read second word as data */
				AC = 0;
				if (tx0.io_handlers[io_handler_rim])
					(*tx0.io_handlers[io_handler_rim])();	/* data will be transferred to AC */
				tx0.rim_step = 3;
				break;

			case 3:
				if (! tx0.ios)
				{	/* transfer incomplete: wait some more */
					tx0_ICount = 0;
				}
				else
				{	/* data transfer complete */
					tx0.ios = 0;

					tx0_write(MAR, MBR = AC);

					tx0.rim_step = 0;
				}
				break;
			}
		}
		else
		{
			if (tx0.cycle == 0)
			{	/* fetch new instruction */
				MBR = tx0_read(MAR = PC);
				INCREMENT_PC_8KW;
				IR = MBR >> 13;		/* basic opcode */
				MAR = MBR & ADDRESS_MASK_8KW;
			}

			if (! tx0.ioh)
			{
				if ((tx0.stop_cyc0 && (tx0.cycle == 0))
					|| (tx0.stop_cyc1 && (tx0.cycle == 1)))
					tx0.run = 0;

				execute_instruction_8kw();	/* execute instruction */
			}

			tx0_ICount -= 1;
		}
	}
	while (tx0_ICount > 0);

	return cycles - tx0_ICount;
}


static void tx0_set_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
	/* --- the following bits of info are set as 64-bit signed integers --- */
	case CPUINFO_INT_SP:						(void) info->i;	/* no SP */					break;
	case CPUINFO_INT_PC:
	case CPUINFO_INT_REGISTER + TX0_PC:			PC = info->i & tx0.address_mask;			break;
	case CPUINFO_INT_REGISTER + TX0_IR:			IR = info->i & tx0.ir_mask; /* weird idea */break;
	case CPUINFO_INT_REGISTER + TX0_MBR:		MBR = info->i & 0777777;					break;
	case CPUINFO_INT_REGISTER + TX0_MAR:		MAR = info->i & tx0.address_mask;			break;
	case CPUINFO_INT_REGISTER + TX0_AC:			AC = info->i & 0777777;						break;
	case CPUINFO_INT_REGISTER + TX0_LR:			LR = info->i & 0777777;						break;
	case CPUINFO_INT_REGISTER + TX0_XR:			XR = info->i & 0037777;						break;
	case CPUINFO_INT_REGISTER + TX0_PF:			PF = info->i & 077;							break;
	case CPUINFO_INT_REGISTER + TX0_TBR:		tx0.tbr = info->i & 0777777;				break;
	case CPUINFO_INT_REGISTER + TX0_TAC:		tx0.tac = info->i & 0777777;				break;
	case CPUINFO_INT_REGISTER + TX0_TSS00:
	case CPUINFO_INT_REGISTER + TX0_TSS01:
	case CPUINFO_INT_REGISTER + TX0_TSS02:
	case CPUINFO_INT_REGISTER + TX0_TSS03:
	case CPUINFO_INT_REGISTER + TX0_TSS04:
	case CPUINFO_INT_REGISTER + TX0_TSS05:
	case CPUINFO_INT_REGISTER + TX0_TSS06:
	case CPUINFO_INT_REGISTER + TX0_TSS07:
	case CPUINFO_INT_REGISTER + TX0_TSS10:
	case CPUINFO_INT_REGISTER + TX0_TSS11:
	case CPUINFO_INT_REGISTER + TX0_TSS12:
	case CPUINFO_INT_REGISTER + TX0_TSS13:
	case CPUINFO_INT_REGISTER + TX0_TSS14:
	case CPUINFO_INT_REGISTER + TX0_TSS15:
	case CPUINFO_INT_REGISTER + TX0_TSS16:
	case CPUINFO_INT_REGISTER + TX0_TSS17:		tx0.tss[state-(CPUINFO_INT_REGISTER + TX0_TSS00)] = info->i & 0777777;	break;
	case CPUINFO_INT_REGISTER + TX0_CM_SEL:		tx0.cm_sel = info->i & 0177777;				break;
	case CPUINFO_INT_REGISTER + TX0_LR_SEL:		tx0.lr_sel = info->i & 0177777;				break;
	case CPUINFO_INT_REGISTER + TX0_GBL_CM_SEL:	tx0.gbl_cm_sel = info->i ? 1 : 0;			break;
	case CPUINFO_INT_REGISTER + TX0_STOP_CYC0:	tx0.stop_cyc0 = info->i ? 1 : 0;			break;
	case CPUINFO_INT_REGISTER + TX0_STOP_CYC1:	tx0.stop_cyc1 = info->i ? 1 : 0;			break;
	case CPUINFO_INT_REGISTER + TX0_RUN:		tx0.run = info->i ? 1 : 0;					break;
	case CPUINFO_INT_REGISTER + TX0_RIM:		tx0.rim = info->i ? 1 : 0;					break;
#if LOG
	case CPUINFO_INT_REGISTER + TX0_CYCLE:		logerror("tx0_set_reg to cycle counter ignored\n");/* no way!*/ break;
	case CPUINFO_INT_REGISTER + TX0_IOH:		logerror("tx0_set_reg to ioh flip-flop ignored\n");/* no way!*/ break;
	case CPUINFO_INT_REGISTER + TX0_IOS:		logerror("tx0_set_reg to ios flip-flop ignored\n");/* no way!*/ break;
#endif
	case CPUINFO_INT_REGISTER + TX0_RESET:		pulse_reset();							break;
	case CPUINFO_INT_REGISTER + TX0_IO_COMPLETE:tx0.ios = 1;							break;
	}
}


void tx0_64kw_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
	/* --- the following bits of info are returned as 64-bit signed integers --- */
	case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(tx0);					break;
	case CPUINFO_INT_INPUT_LINES:					info->i = 0;							break;
	case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
	case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_BE;	/*don't care*/	break;
	case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
	case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 4;							break;
	case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 4;							break;
	case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
	case CPUINFO_INT_MAX_CYCLES:					info->i = 3;							break;

	case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 32;					break;
	case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 16;					break;
	case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = -2;					break;
	case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
	case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
	case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
	case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 0;					break;
	case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO: 		info->i = 0;					break;
	case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO: 		info->i = 0;					break;

	case CPUINFO_INT_SP:							info->i = 0;	/* no SP */				break;
	case CPUINFO_INT_PC:							info->i = PC;							break;
	case CPUINFO_INT_PREVIOUSPC:					info->i = 0;	/* TODO??? */			break;

	case CPUINFO_INT_REGISTER + TX0_PC:				info->i = PC;							break;
	case CPUINFO_INT_REGISTER + TX0_IR:				info->i = IR;							break;
	case CPUINFO_INT_REGISTER + TX0_MBR:			info->i = MBR;							break;
	case CPUINFO_INT_REGISTER + TX0_MAR:			info->i = MAR;							break;
	case CPUINFO_INT_REGISTER + TX0_AC:				info->i = AC;							break;
	case CPUINFO_INT_REGISTER + TX0_LR:				info->i = LR;							break;
	case CPUINFO_INT_REGISTER + TX0_XR:				info->i = XR;							break;
	case CPUINFO_INT_REGISTER + TX0_PF:				info->i = PF;							break;
	case CPUINFO_INT_REGISTER + TX0_TBR:			info->i = tx0.tbr;						break;
	case CPUINFO_INT_REGISTER + TX0_TAC:			info->i = tx0.tac;						break;
	case CPUINFO_INT_REGISTER + TX0_TSS00:
	case CPUINFO_INT_REGISTER + TX0_TSS01:
	case CPUINFO_INT_REGISTER + TX0_TSS02:
	case CPUINFO_INT_REGISTER + TX0_TSS03:
	case CPUINFO_INT_REGISTER + TX0_TSS04:
	case CPUINFO_INT_REGISTER + TX0_TSS05:
	case CPUINFO_INT_REGISTER + TX0_TSS06:
	case CPUINFO_INT_REGISTER + TX0_TSS07:
	case CPUINFO_INT_REGISTER + TX0_TSS10:
	case CPUINFO_INT_REGISTER + TX0_TSS11:
	case CPUINFO_INT_REGISTER + TX0_TSS12:
	case CPUINFO_INT_REGISTER + TX0_TSS13:
	case CPUINFO_INT_REGISTER + TX0_TSS14:
	case CPUINFO_INT_REGISTER + TX0_TSS15:
	case CPUINFO_INT_REGISTER + TX0_TSS16:
	case CPUINFO_INT_REGISTER + TX0_TSS17:			info->i = tx0.tss[state-(CPUINFO_INT_REGISTER + TX0_TSS00)]; break;
	case CPUINFO_INT_REGISTER + TX0_CM_SEL:			info->i = tx0.cm_sel;					break;
	case CPUINFO_INT_REGISTER + TX0_LR_SEL:			info->i = tx0.lr_sel;					break;
	case CPUINFO_INT_REGISTER + TX0_GBL_CM_SEL:		info->i = tx0.gbl_cm_sel;				break;
	case CPUINFO_INT_REGISTER + TX0_STOP_CYC0:		info->i = tx0.stop_cyc0;				break;
	case CPUINFO_INT_REGISTER + TX0_STOP_CYC1:		info->i = tx0.stop_cyc1;				break;
	case CPUINFO_INT_REGISTER + TX0_RUN:			info->i = tx0.run;						break;
	case CPUINFO_INT_REGISTER + TX0_RIM:			info->i = tx0.rim;						break;
	case CPUINFO_INT_REGISTER + TX0_CYCLE:			info->i = tx0.cycle;					break;
	case CPUINFO_INT_REGISTER + TX0_IOH:			info->i = tx0.ioh;						break;
	case CPUINFO_INT_REGISTER + TX0_IOS:			info->i = tx0.ios;						break;

	/* --- the following bits of info are returned as pointers to data or functions --- */
	case CPUINFO_PTR_SET_INFO:						info->setinfo = tx0_set_info;			break;
	case CPUINFO_PTR_GET_CONTEXT:					info->getcontext = tx0_get_context;		break;
	case CPUINFO_PTR_SET_CONTEXT:					info->setcontext = tx0_set_context;		break;
	case CPUINFO_PTR_INIT:							info->init = tx0_init_64kw;				break;
	case CPUINFO_PTR_RESET:							info->reset = tx0_reset;				break;
	case CPUINFO_PTR_EXECUTE:						info->execute = tx0_execute_64kw;		break;
	case CPUINFO_PTR_BURN:							info->burn = NULL;						break;

#ifdef MAME_DEBUG
	case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = tx0_dasm_64kw;		break;
#endif /* MAME_DEBUG */
	case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &tx0_ICount;				break;

	/* --- the following bits of info are returned as NULL-terminated strings --- */
	case CPUINFO_STR_NAME: 							strcpy(info->s = cpuintrf_temp_str(), "TX-0");	break;
	case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s = cpuintrf_temp_str(), "TX-0");	break;
	case CPUINFO_STR_CORE_VERSION:					strcpy(info->s = cpuintrf_temp_str(), "1.0");	break;
	case CPUINFO_STR_CORE_FILE:						strcpy(info->s = cpuintrf_temp_str(), __FILE__);	break;
	case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s = cpuintrf_temp_str(), "Raphael Nabet");	break;

    case CPUINFO_STR_FLAGS:							strcpy(info->s = cpuintrf_temp_str(), "");	break;

	case CPUINFO_STR_REGISTER + TX0_PC:				sprintf(info->s = cpuintrf_temp_str(), "PC:0%06o", PC); break;
	case CPUINFO_STR_REGISTER + TX0_IR:				sprintf(info->s = cpuintrf_temp_str(), "IR:0%02o", IR); break;
	case CPUINFO_STR_REGISTER + TX0_MBR:			sprintf(info->s = cpuintrf_temp_str(), "MBR:0%06o", MBR); break;
	case CPUINFO_STR_REGISTER + TX0_MAR:			sprintf(info->s = cpuintrf_temp_str(), "MAR:0%06o", MAR); break;
	case CPUINFO_STR_REGISTER + TX0_AC:				sprintf(info->s = cpuintrf_temp_str(), "AC:0%06o", AC); break;
	case CPUINFO_STR_REGISTER + TX0_LR:				sprintf(info->s = cpuintrf_temp_str(), "LR:0%06o", LR); break;
	case CPUINFO_STR_REGISTER + TX0_XR:				sprintf(info->s = cpuintrf_temp_str(), "XR:0%05o", XR); break;
	case CPUINFO_STR_REGISTER + TX0_PF:				sprintf(info->s = cpuintrf_temp_str(), "PF:0%02o", PF); break;							break;
	case CPUINFO_STR_REGISTER + TX0_TBR:			sprintf(info->s = cpuintrf_temp_str(), "TBR:0%06o", tx0.tbr); break;
	case CPUINFO_STR_REGISTER + TX0_TAC:			sprintf(info->s = cpuintrf_temp_str(), "TAC:0%06o", tx0.tac); break;
	case CPUINFO_STR_REGISTER + TX0_TSS00:
	case CPUINFO_STR_REGISTER + TX0_TSS01:
	case CPUINFO_STR_REGISTER + TX0_TSS02:
	case CPUINFO_STR_REGISTER + TX0_TSS03:
	case CPUINFO_STR_REGISTER + TX0_TSS04:
	case CPUINFO_STR_REGISTER + TX0_TSS05:
	case CPUINFO_STR_REGISTER + TX0_TSS06:
	case CPUINFO_STR_REGISTER + TX0_TSS07:
	case CPUINFO_STR_REGISTER + TX0_TSS10:
	case CPUINFO_STR_REGISTER + TX0_TSS11:
	case CPUINFO_STR_REGISTER + TX0_TSS12:
	case CPUINFO_STR_REGISTER + TX0_TSS13:
	case CPUINFO_STR_REGISTER + TX0_TSS14:
	case CPUINFO_STR_REGISTER + TX0_TSS15:
	case CPUINFO_STR_REGISTER + TX0_TSS16:
	case CPUINFO_STR_REGISTER + TX0_TSS17:			sprintf(info->s = cpuintrf_temp_str(), "TSS%02o:0%06o", state-(CPUINFO_STR_REGISTER + TX0_TSS00), tx0.tss[state-(CPUINFO_STR_REGISTER + TX0_TSS00)]); break;
	case CPUINFO_STR_REGISTER + TX0_CM_SEL:			sprintf(info->s = cpuintrf_temp_str(), "CMSEL:0%06o", tx0.cm_sel); break;
	case CPUINFO_STR_REGISTER + TX0_LR_SEL:			sprintf(info->s = cpuintrf_temp_str(), "LRSEL:0%06o", tx0.lr_sel); break;
	case CPUINFO_STR_REGISTER + TX0_GBL_CM_SEL:		sprintf(info->s = cpuintrf_temp_str(), "GBLCMSEL:%X", tx0.gbl_cm_sel); break;
	case CPUINFO_STR_REGISTER + TX0_STOP_CYC0:		sprintf(info->s = cpuintrf_temp_str(), "STOPCYC0:%X", tx0.stop_cyc0); break;
	case CPUINFO_STR_REGISTER + TX0_STOP_CYC1:		sprintf(info->s = cpuintrf_temp_str(), "STOPCYC1:%X", tx0.stop_cyc1); break;
	case CPUINFO_STR_REGISTER + TX0_RUN:			sprintf(info->s = cpuintrf_temp_str(), "RUN:%X", tx0.run); break;
	case CPUINFO_STR_REGISTER + TX0_RIM:			sprintf(info->s = cpuintrf_temp_str(), "RIM:%X", tx0.rim); break;
	case CPUINFO_STR_REGISTER + TX0_CYCLE:			sprintf(info->s = cpuintrf_temp_str(), "CYCLE:%X", tx0.cycle); break;
	case CPUINFO_STR_REGISTER + TX0_IOH:			sprintf(info->s = cpuintrf_temp_str(), "IOH:%X", tx0.ioh); break;
	case CPUINFO_STR_REGISTER + TX0_IOS:			sprintf(info->s = cpuintrf_temp_str(), "IOS:%X", tx0.ios); break;
	}
}

void tx0_8kw_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
	/* --- the following bits of info are returned as 64-bit signed integers --- */
	case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(tx0);					break;
	case CPUINFO_INT_INPUT_LINES:					info->i = 0;							break;
	case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
	case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_BE;	/*don't care*/	break;
	case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
	case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 4;							break;
	case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 4;							break;
	case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
	case CPUINFO_INT_MAX_CYCLES:					info->i = 3;							break;

	case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 32;					break;
	case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 13;					break;
	case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = -2;					break;
	case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
	case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
	case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
	case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 0;					break;
	case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO: 		info->i = 0;					break;
	case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO: 		info->i = 0;					break;

	case CPUINFO_INT_SP:							info->i = 0;	/* no SP */				break;
	case CPUINFO_INT_PC:							info->i = PC;							break;
	case CPUINFO_INT_PREVIOUSPC:					info->i = 0;	/* TODO??? */			break;

	case CPUINFO_INT_REGISTER + TX0_PC:				info->i = PC;							break;
	case CPUINFO_INT_REGISTER + TX0_IR:				info->i = IR;							break;
	case CPUINFO_INT_REGISTER + TX0_MBR:			info->i = MBR;							break;
	case CPUINFO_INT_REGISTER + TX0_MAR:			info->i = MAR;							break;
	case CPUINFO_INT_REGISTER + TX0_AC:				info->i = AC;							break;
	case CPUINFO_INT_REGISTER + TX0_LR:				info->i = LR;							break;
	case CPUINFO_INT_REGISTER + TX0_XR:				info->i = XR;							break;
	case CPUINFO_INT_REGISTER + TX0_PF:				info->i = PF;							break;
	case CPUINFO_INT_REGISTER + TX0_TBR:			info->i = tx0.tbr;						break;
	case CPUINFO_INT_REGISTER + TX0_TAC:			info->i = tx0.tac;						break;
	case CPUINFO_INT_REGISTER + TX0_TSS00:
	case CPUINFO_INT_REGISTER + TX0_TSS01:
	case CPUINFO_INT_REGISTER + TX0_TSS02:
	case CPUINFO_INT_REGISTER + TX0_TSS03:
	case CPUINFO_INT_REGISTER + TX0_TSS04:
	case CPUINFO_INT_REGISTER + TX0_TSS05:
	case CPUINFO_INT_REGISTER + TX0_TSS06:
	case CPUINFO_INT_REGISTER + TX0_TSS07:
	case CPUINFO_INT_REGISTER + TX0_TSS10:
	case CPUINFO_INT_REGISTER + TX0_TSS11:
	case CPUINFO_INT_REGISTER + TX0_TSS12:
	case CPUINFO_INT_REGISTER + TX0_TSS13:
	case CPUINFO_INT_REGISTER + TX0_TSS14:
	case CPUINFO_INT_REGISTER + TX0_TSS15:
	case CPUINFO_INT_REGISTER + TX0_TSS16:
	case CPUINFO_INT_REGISTER + TX0_TSS17:			info->i = tx0.tss[state-(CPUINFO_INT_REGISTER + TX0_TSS00)]; break;
	case CPUINFO_INT_REGISTER + TX0_CM_SEL:			info->i = tx0.cm_sel;					break;
	case CPUINFO_INT_REGISTER + TX0_LR_SEL:			info->i = tx0.lr_sel;					break;
	case CPUINFO_INT_REGISTER + TX0_GBL_CM_SEL:		info->i = tx0.gbl_cm_sel;				break;
	case CPUINFO_INT_REGISTER + TX0_STOP_CYC0:		info->i = tx0.stop_cyc0;				break;
	case CPUINFO_INT_REGISTER + TX0_STOP_CYC1:		info->i = tx0.stop_cyc1;				break;
	case CPUINFO_INT_REGISTER + TX0_RUN:			info->i = tx0.run;						break;
	case CPUINFO_INT_REGISTER + TX0_RIM:			info->i = tx0.rim;						break;
	case CPUINFO_INT_REGISTER + TX0_CYCLE:			info->i = tx0.cycle;					break;
	case CPUINFO_INT_REGISTER + TX0_IOH:			info->i = tx0.ioh;						break;
	case CPUINFO_INT_REGISTER + TX0_IOS:			info->i = tx0.ios;						break;

	/* --- the following bits of info are returned as pointers to data or functions --- */
	case CPUINFO_PTR_SET_INFO:						info->setinfo = tx0_set_info;			break;
	case CPUINFO_PTR_GET_CONTEXT:					info->getcontext = tx0_get_context;		break;
	case CPUINFO_PTR_SET_CONTEXT:					info->setcontext = tx0_set_context;		break;
	case CPUINFO_PTR_INIT:							info->init = tx0_init_8kw;				break;
	case CPUINFO_PTR_RESET:							info->reset = tx0_reset;				break;
	case CPUINFO_PTR_EXECUTE:						info->execute = tx0_execute_8kw;		break;
	case CPUINFO_PTR_BURN:							info->burn = NULL;						break;

#ifdef MAME_DEBUG
	case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = tx0_dasm_8kw;	break;
#endif /* MAME_DEBUG */
	case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &tx0_ICount;				break;

	/* --- the following bits of info are returned as NULL-terminated strings --- */
	case CPUINFO_STR_NAME: 							strcpy(info->s = cpuintrf_temp_str(), "TX-0");	break;
	case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s = cpuintrf_temp_str(), "TX-0");	break;
	case CPUINFO_STR_CORE_VERSION:					strcpy(info->s = cpuintrf_temp_str(), "1.0");	break;
	case CPUINFO_STR_CORE_FILE:						strcpy(info->s = cpuintrf_temp_str(), __FILE__);	break;
	case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s = cpuintrf_temp_str(), "Raphael Nabet");	break;

    case CPUINFO_STR_FLAGS:							strcpy(info->s = cpuintrf_temp_str(), "");	break;

	case CPUINFO_STR_REGISTER + TX0_PC:				sprintf(info->s = cpuintrf_temp_str(), "PC:0%06o", PC); break;
	case CPUINFO_STR_REGISTER + TX0_IR:				sprintf(info->s = cpuintrf_temp_str(), "IR:0%02o", IR); break;
	case CPUINFO_STR_REGISTER + TX0_MBR:			sprintf(info->s = cpuintrf_temp_str(), "MBR:0%06o", MBR); break;
	case CPUINFO_STR_REGISTER + TX0_MAR:			sprintf(info->s = cpuintrf_temp_str(), "MAR:0%06o", MAR); break;
	case CPUINFO_STR_REGISTER + TX0_AC:				sprintf(info->s = cpuintrf_temp_str(), "AC:0%06o", AC); break;
	case CPUINFO_STR_REGISTER + TX0_LR:				sprintf(info->s = cpuintrf_temp_str(), "LR:0%06o", LR); break;
	case CPUINFO_STR_REGISTER + TX0_XR:				sprintf(info->s = cpuintrf_temp_str(), "XR:0%05o", XR); break;
	case CPUINFO_STR_REGISTER + TX0_PF:				sprintf(info->s = cpuintrf_temp_str(), "PF:0%02o", PF); break;							break;
	case CPUINFO_STR_REGISTER + TX0_TBR:			sprintf(info->s = cpuintrf_temp_str(), "TBR:0%06o", tx0.tbr); break;
	case CPUINFO_STR_REGISTER + TX0_TAC:			sprintf(info->s = cpuintrf_temp_str(), "TAC:0%06o", tx0.tac); break;
	case CPUINFO_STR_REGISTER + TX0_TSS00:
	case CPUINFO_STR_REGISTER + TX0_TSS01:
	case CPUINFO_STR_REGISTER + TX0_TSS02:
	case CPUINFO_STR_REGISTER + TX0_TSS03:
	case CPUINFO_STR_REGISTER + TX0_TSS04:
	case CPUINFO_STR_REGISTER + TX0_TSS05:
	case CPUINFO_STR_REGISTER + TX0_TSS06:
	case CPUINFO_STR_REGISTER + TX0_TSS07:
	case CPUINFO_STR_REGISTER + TX0_TSS10:
	case CPUINFO_STR_REGISTER + TX0_TSS11:
	case CPUINFO_STR_REGISTER + TX0_TSS12:
	case CPUINFO_STR_REGISTER + TX0_TSS13:
	case CPUINFO_STR_REGISTER + TX0_TSS14:
	case CPUINFO_STR_REGISTER + TX0_TSS15:
	case CPUINFO_STR_REGISTER + TX0_TSS16:
	case CPUINFO_STR_REGISTER + TX0_TSS17:			sprintf(info->s = cpuintrf_temp_str(), "TSS%02o:0%06o", state-(CPUINFO_STR_REGISTER + TX0_TSS00), tx0.tss[state-(CPUINFO_STR_REGISTER + TX0_TSS00)]); break;
	case CPUINFO_STR_REGISTER + TX0_CM_SEL:			sprintf(info->s = cpuintrf_temp_str(), "CMSEL:0%06o", tx0.cm_sel); break;
	case CPUINFO_STR_REGISTER + TX0_LR_SEL:			sprintf(info->s = cpuintrf_temp_str(), "LRSEL:0%06o", tx0.lr_sel); break;
	case CPUINFO_STR_REGISTER + TX0_GBL_CM_SEL:		sprintf(info->s = cpuintrf_temp_str(), "GBLCMSEL:%X", tx0.gbl_cm_sel); break;
	case CPUINFO_STR_REGISTER + TX0_STOP_CYC0:		sprintf(info->s = cpuintrf_temp_str(), "STOPCYC0:%X", tx0.stop_cyc0); break;
	case CPUINFO_STR_REGISTER + TX0_STOP_CYC1:		sprintf(info->s = cpuintrf_temp_str(), "STOPCYC1:%X", tx0.stop_cyc1); break;
	case CPUINFO_STR_REGISTER + TX0_RUN:			sprintf(info->s = cpuintrf_temp_str(), "RUN:%X", tx0.run); break;
	case CPUINFO_STR_REGISTER + TX0_RIM:			sprintf(info->s = cpuintrf_temp_str(), "RIM:%X", tx0.rim); break;
	case CPUINFO_STR_REGISTER + TX0_CYCLE:			sprintf(info->s = cpuintrf_temp_str(), "CYCLE:%X", tx0.cycle); break;
	case CPUINFO_STR_REGISTER + TX0_IOH:			sprintf(info->s = cpuintrf_temp_str(), "IOH:%X", tx0.ioh); break;
	case CPUINFO_STR_REGISTER + TX0_IOS:			sprintf(info->s = cpuintrf_temp_str(), "IOS:%X", tx0.ios); break;
	}
}


/* execute one instruction */
static void execute_instruction_64kw(void)
{
	if (! tx0.cycle)
	{
		tx0.cycle = 1;	/* most frequent case */
		switch (IR)
		{
		case 0:			/* STOre */
		case 1:			/* ADD */
			break;

		case 2:		/* TRansfer on Negative */
			if (AC & 0400000)
			{
				PC = MAR & ADDRESS_MASK_64KW;
				tx0.cycle = 0;	/* instruction only takes one cycle if branch
                                    is taken */
			}
			break;

		case 3:		/* OPeRate */
			if (MAR & 0100000)
				/* (0.8) CLL = Clear the left nine digital positions of the AC */
				AC &= 0000777;

			if (MAR & 0040000)
				/* (0.8) CLR = Clear the right nine digital positions of the AC */
				AC &= 0777000;

			if (((MAR & 0030000) >> 12) == 1)
				/* (0.8) IOS In-Out Stop = Stop machine so that an In-Out command
                    (specified by digits 6 7 8 of MAR) may be executed */
				tx0.ioh = 1;

			if (((MAR & 0007000) >> 9) != 0)
			{
				/* ((MAR & 0007000) >> 9) is device ID */
				/* 7: */
				/* (0.8) P7H = Punch holes 1-6 in flexo tape specified by AC
                    digital positions 2, 5, 8, 11, 14, and 17.  Also punches a 7th
                    hole on tape. */
				/* 6: */
				/* (0.8) P6H = Same as P7H but no seventh hole */
				/* 4: */
				/* (0.8) PNT = Print one flexowriter character specified by AC
                    digits 2, 5, 8, 11, 14, and 17. */
				/* 1: */
				/* (0.8) R1C = Read one line of flexo tape so that tape positions
                    1, 2, 3, 4, 5, and 6 will be put in the AC digital positions 0,
                    3, 6, 9, 12 and 15. */
				/* 3: */
				/* (0.8) R3C = Read one line of flexo tape into AC digits 0, 3, 6,
                    9, 12 and 15.  Then cycle the AC one digital position; read the
                    next line on tape into AC digits 0, 3, 6, 9, 12 and 15, cycle
                    the AC right one digital position and read the third and last
                    line into AC digits 0, 3, 6, 9, 12 and 15.  (This command is
                    equal to a triple CYR-R1C.) */
				/* 2: */
				/* (0.8) DIS = Intensify a point on the scope with x and y
                    coordinates where x is specified by AC digits 0-8 with digit 0
                    being used as the sign and y is specified by AC digits 9-17
                    with digit 9 being used as the sign for y.  The complement
                    system is in effect when the signs are negative. */
				/* (5 is undefined) */
				int index = (MAR & 0007000) >> 9;

				if (tx0.io_handlers[index])
					(*tx0.io_handlers[index])();
				tx0.ioh = 1;
			}
			break;
		}
	}
	else
	{
		tx0.cycle = 0;	/* always true */
		switch (IR)
		{
		case 0:			/* STOre */
			tx0_write(MAR, (MBR = AC));
			break;

		case 1:			/* ADD */
			MBR = tx0_read(MAR);

			AC = AC + MBR;
			AC = (AC + (AC >> 18)) & 0777777;	/* propagate carry around */

			if (AC == 0777777)		/* check for -0 */
				AC = 0;
			break;

		case 2:		/* TRansfer on Negative */
			break;

		case 3:		/* OPeRate */
			if ((MAR & 0000104) == 0000100)
				/* (1.1) PEN = Read the light pen flip-flops 1 and 2 into AC(0) and
                    AC(1). */
				/*...*/;

			if ((MAR & 0000104) == 0000004)
				/* (1.1) TAC = Insert a one in each digital position of the AC
                    wherever there is a one in the corresponding digital position
                    of the TAC. */
				/*...*/;

			if (MAR & 0000040)
				/* (1.2) COM = Complement every digit in the accumulator */
				AC ^= 0777777;

			if ((MAR & 0000003) == 1)
				/* (1.2) AMB = Store the contents of the AC in the MBR. */
				MBR = AC;

			if ((MAR & 0000003) == 3)
				/* (1.2) TBR = Store the contents of the TBR in the MBR. */
				/*...*/;

			if ((MAR & 0000003) == 2)
				/* (1.3) LMB = Store the contents of the LR in the MBR. */
				MBR = LR;
			break;

			if (((MAR & 0000600) >> 7) == 1)
				/* (1.3) MLR = Store the contents of the MBR (memory buffer
                    register) in the live reg. */
				LR = MBR;

			if (((MAR & 0000600) >> 7) == 2)
				/* (1.4) SHR = Shift the AC right one place, i.e. multiply the AC
                    by 2^-1 */
				AC >>= 1;

			if (((MAR & 0000600) >> 7) == 3)
				/* (1.4) CYR = Cycle the AC right one digital position (AC(17) will
                    become AC(0)) */
				AC = (AC >> 1) | ((AC & 1) << 17);

			if (MAR & 0000020)
				/* (1.4) PAD = Partial add AC to MBR, that is, for every digital
                    position of the MBR that contains a one, complement the digit
                    in the corresponding digital position of the AC.  This is also
                    called a half add. */
				AC ^= MBR;

			if (MAR & 0000010)
			{	/* (1.7) CRY = Partial add the 18 digits of the AC to the
                    corresponding 18 digits of the carry.

                    To determine what the 18 digits of the carry are, use the
                    following rule:

                    "Grouping the AC and MBR digits into pairs and proceeding from
                    right to left, assign the carry digit of the next pair to a one
                    if in the present pair MBR = 1 and AC = 0 or if in the present
                    pair AC = 1 and carry 1.

                    (Note: the 0th digit pair determines the 17th pair's carry
                    digit)" */
				AC ^= MBR;

				AC = AC + MBR;
				AC = (AC + (AC >> 18)) & 0777777;	/* propagate carry around */

				if (AC == 0777777)		/* check for -0 */
					AC = 0;
			}

			if (((MAR & 0030000) >> 12) == 3)
				/* (1.8) Hlt = Halt the computer */
				tx0.run = 0;

			break;
		}
	}
}

static void indexed_address_eval(void)
{
	MAR = MAR + XR;
	MAR = (MAR + (MAR >> 14)) & 0037777;	/* propagate carry around */
	//if (MAR == 0037777)       /* check for -0 */
	//  MAR = 0;
	if (MAR & 0020000)			/* fix negative (right???) */
		MAR = (MAR + 1) & 0017777;
}

/* execute one instruction */
static void execute_instruction_8kw(void)
{
	if (! tx0.cycle)
	{
		tx0.cycle = 1;	/* most frequent case */
		switch (IR)
		{
		case 0:		/* STOre */
		case 1:		/* STore indeXed */
		case 2:		/* Store indeX in Address */
		case 3:		/* ADd One */
		case 4:		/* Store LR */
		case 5:		/* Store Lr indeXed */
		case 6:		/* STore Zero */
		case 8:		/* ADD */
		case 9:		/* ADd indeXed */
		case 10:	/* LoaD indeX */
		case 11:	/* AUgment indeX */
		case 12:	/* Load LR */
		case 13:	/* Load Lr indeXed */
		case 14:	/* LoaD Ac */
		case 15:	/* Load Ac indeXed */
			break;

		case 16:	/* TRansfer on Negative */
			if (AC & 0400000)
			{
				PC = MAR & 0017777;
				tx0.cycle = 0;	/* instruction only takes one cycle if branch
                                    is taken */
			}
			break;

		case 17:	/* Transfer on ZEro */
			if ((AC == 0000000) || (AC == 0777777))
			{
				PC = MAR & 0017777;
				tx0.cycle = 0;	/* instruction only takes one cycle if branch
                                    is taken */
			}
			break;

		case 18:	/* Transfer and Set indeX */
			XR = PC;
			PC = MAR & 0017777;
			tx0.cycle = 0;	/* instruction only takes one cycle if branch
                                is taken */
			break;

		case 19:	/* Transfer and IndeX */
			if ((XR != 0000000) && (XR != 0037777))
			{
				if (XR & 0020000)
					XR ++;
				else
					XR--;
				PC = MAR & 0017777;
				tx0.cycle = 0;	/* instruction only takes one cycle if branch
                                    is taken */
			}
			break;

		case 21:	/* TRansfer indeXed */
			indexed_address_eval();
		case 20:	/* TRAnsfer */
			PC = MAR & 0017777;
			tx0.cycle = 0;	/* instruction only takes one cycle if branch
                                is taken */
			break;

		case 22:	/* Transfer on external LeVel */
			/*if (...)
            {
                PC = MAR & 0017777;
                tx0.cycle = 0;*/	/* instruction only takes one cycle if branch
                                    is taken */
			/*}*/
			break;

		case 24:	/* OPeRate */
		case 25:
		case 26:
		case 27:
		case 28:
		case 29:
		case 30:
		case 31:
			if (((IR & 001) == 00) && ((MAR & 017000) == 004000))
			{	/* Select class instruction */
				if (IR & 004)
					/* (0.8???) CLA = CLear Ac */
					AC = 0;

				/* (IOS???) SEL = SELect */
				if (tx0.sel_handler)
					(*tx0.sel_handler)();
			}
			else
			{	/* Normal operate class instruction */
				if (((IR & 001) == 01) && ((MAR & 017000) == 011000))
					/* (0.6) CLL = CLear Left 9 bits of ac */
					AC &= 0000777;

				if (((IR & 001) == 01) && ((MAR & 017000) == 012000))
					/* (0.6) CLR = CLear Right 9 bits of ac */
					AC &= 0777000;

				if (IR & 002)
					/* (0.7) AMB = transfer Ac to MBr */
					MBR = AC;

				if (IR & 004)
					/* (0.8) CLA = CLear Ac */
					AC = 0;

				if (((IR & 001) == 01) && ((MAR & 010000) == 000000))
				{	/* (IOS) In-Out group commands */
					/* ((MAR & 0007000) >> 9) is device ID */
					/* 0: */
					/* (***) CPY = CoPY synchronizes transmission of information
                        between in-out equipment and computer. */
					/* 1: */
					/* (IOS) R1L = Read 1 Line of tape from PETR into AC bits 0, 3,
                        6, 9, 12, 15, with CYR before read (inclusive or) */
					/* 3: */
					/* (IOS) R3L = Read 3 Lines of tape from PETR into AC bits 0,
                        3, 6, 9, 12, 15, with CYR before each read (inclusive or) */
					/* 2: */
					/* (IOS) DIS = DISplay a point on scope (AC bits 0-8 specify x
                        coordinate, AC bits 9-17 specify y coordinate). The
                        coordinate (0, 0) is usually at the lower left hand corner
                        of the scope.  A console switch is available to relocate
                        (0,0) to the center. */
					/* 6: */
					/* (IOS) P6H = Punch one 6-bit line of flexo tape (without 7th
                        hole) from ac bits 2, 5, 8, 11, 14, 17.  Note: lines
                        without 7th hole are ignored by PETR. */
					/* 7: */
						/* (IOS) P7H = same as P6H, but with 7th hole */
					/* 4: */
					/* (IOS) PRT = Print one six bit flexo character from AC bits
                        2, 5, 8, 11, 14, 17. */
					/* (5 is undefined) */
					int index = (MAR & 0007000) >> 9;

					if (tx0.io_handlers[index])
						(*tx0.io_handlers[index])();
					tx0.ioh = 1;
				}

				if (((IR & 001) == 00) && ((MAR & 010000) == 010000))
				{	/* (IOS) EX0 through EX7 = operate user's EXternal equipment. */
					switch ((MAR & 0007000) >> 9)
					{
					/* ... */
					}
				}
			}
			break;
		}
	}
	else
	{
		if (((IR != 2) && (IR != 3)) || (tx0.cycle == 2))
			tx0.cycle = 0;
		else
			tx0.cycle = 2;	/* SXA and ADO have an extra cycle 2 */
		switch (IR)
		{
		case 1:		/* STore indeXed */
			indexed_address_eval();
		case 0:		/* STOre */
			tx0_write(MAR, (MBR = AC));
			break;

		case 2:		/* Store indeX in Address */
			if (tx0.cycle)
			{	/* cycle 1 */
				MBR = tx0_read(MAR);
				MBR = (MBR & 0760000) | (XR & 0017777);
			}
			else
			{	/* cycle 2 */
				tx0_write(MAR, MBR);
			}
			break;

		case 3:		/* ADd One */
			if (tx0.cycle)
			{	/* cycle 1 */
				AC = tx0_read(MAR) + 1;

				#if 0
					AC = (AC + (AC >> 18)) & 0777777;	/* propagate carry around */
					if (AC == 0777777)		/* check for -0 (right???) */
						AC = 0;
				#else
					if (AC >= 0777777)
						AC = (AC + 1) & 0777777;
				#endif
			}
			else
			{	/* cycle 2 */
				tx0_write(MAR, (MBR = AC));
			}
			break;

		case 5:		/* Store Lr indeXed */
			indexed_address_eval();
		case 4:		/* Store LR */
			tx0_write(MAR, (MBR = LR));
			break;

		case 6:		/* STore Zero */
			tx0_write(MAR, (MBR = 0));
			break;

		case 9:		/* ADd indeXed */
			indexed_address_eval();
		case 8:		/* ADD */
			MBR = tx0_read(MAR);

			AC = AC + MBR;
			AC = (AC + (AC >> 18)) & 0777777;	/* propagate carry around */

			if (AC == 0777777)		/* check for -0 */
				AC = 0;
			break;

		case 10:	/* LoaD indeX */
			MBR = tx0_read(MAR);
			XR = (MBR & 0017777) | ((MBR >> 4) & 0020000);
			break;

		case 11:	/* AUgment indeX */
			MBR = tx0_read(MAR);

			XR = XR + ((MBR & 0017777) | ((MBR >> 4) & 0020000));
			XR = (XR + (XR >> 14)) & 0037777;	/* propagate carry around */

			//if (XR == 0037777)        /* check for -0 */
			//  XR = 0;
			break;

		case 13:	/* Load Lr indeXed */
			indexed_address_eval();
		case 12:	/* Load LR */
			LR = MBR = tx0_read(MAR);
			break;

		case 15:	/* Load Ac indeXed */
			indexed_address_eval();
		case 14:	/* LoaD Ac */
			AC = MBR = tx0_read(MAR);
			break;

		case 16:	/* TRansfer on Negative */
		case 17:	/* Transfer on ZEro */
		case 18:	/* Transfer and Set indeX */
		case 19:	/* Transfer and IndeX */
		case 20:	/* TRAnsfer */
		case 21:	/* TRansfer indeXed */
		case 22:	/* Transfer on external LeVel */
			break;

		case 24:	/* OPeRate */
		case 25:
		case 26:
		case 27:
		case 28:
		case 29:
		case 30:
		case 31:
			if (((IR & 001) == 00) && ((MAR & 017000) == 004000))
			{	/* Select class instruction */
			}
			else
			{	/* Normal operate class instruction */
				if (((IR & 001) == 00) && ((MAR & 017000) == 003000))
				{	/* (1.1) PEN = set ac bit 0 from light PEN ff, and ac bit 1 from
                        light gun ff.  (ffs contain one if pen or gun saw displayed
                        point.)  Then clear both light pen and light gun ffs */
					/*AC = (AC & 0177777) |?...;*/
					/*... = 0;*/
				}

				if (((IR & 001) == 00) && ((MAR & 017000) == 001000))
					/* (1.1) TAC = transfer TAC into ac (inclusive or) */
					AC |= tx0.tac;

				if (((IR & 001) == 00) && ((MAR & 017000) == 002000))
					/* (1.2) TBR = transfer TBR into mbr (inclusive or) */
					MBR |= tx0.tbr;

				if (((IR & 001) == 00) && ((MAR & 017000) == 006000))
					/* (1.2) RPF = Read Program Flag register into mbr (inclusive or) */
					MBR |= PF << 8;

				if (MAR & 0000040)
					/* (1.2) COM = COMplement ac */
					AC ^= 0777777;

				if ((! (MAR & 0000400)) && (MAR & 0000100))
				{	/* (1.2) XMB = Transfer XR contents to MBR */
					MBR = XR;
					if (XR & 0020000)
						MBR |= 0740000;
				}

				if (MAR & 0000004)
				{
					switch (MAR & 0000003)
					{
					case 0000003:	/* (1.2) And LR and MBR */
						MBR &= LR;
						break;

					case 0000001:	/* (1.3) Or LR into MBR */
						MBR |= LR;
						break;

					default:
						if (LOG)
							logerror("unrecognized instruction");
						break;
					}
				}

				if (((! (MAR & 0000400)) && (MAR & 0000200)) && ((! (MAR & 0000004)) && (MAR & 0000002)))
				{	/* LMB and MBL used simultaneously interchange LR and MBR */
					int tmp = MBR;
					MBR = LR;
					LR = tmp;
				}
				else if ((! (MAR & 0000400)) && (MAR & 0000200))
					/* (1.4) MBL = Transfer MBR contents to LR */
					LR = MBR;
				else if ((! (MAR & 0000004)) && (MAR & 0000002))
					/* (1.4) LMB = Store the contents of the LR in the MBR. */
					MBR = LR;

				if (MAR & 0000020)
					/* (1.5) PAD = Partial ADd mbr to ac */
					AC ^= MBR;

				if (MAR & 0000400)
				{
					switch (MAR & 0000300)
					{
					case 0000000:	/* (1.6) CYR = CYcle ac contents Right one binary
                                        position (AC(17) -> AC(0)) */
						AC = (AC >> 1) | ((AC & 1) << 17);
						break;

					case 0000200:	/* (1.6) CYcle ac contents Right one binary
                                        position (AC(0) unchanged) */
						AC = (AC >> 1) | (AC & 0400000);
						break;

					default:
						if (LOG)
							logerror("unrecognized instruction");
						break;
					}
				}

				if (((IR & 001) == 00) && ((MAR & 017000) == 007000))
					/* (1.6) SPF = Set Program Flag register from mbr */
					PF = (MBR >> 8) & 077;

				if (MAR & 0000010)
				{	/* (1.7?) CRY = Partial ADd the 18 digits of the AC to the
                        corresponding 18 digits of the carry. */
					AC ^= MBR;

					AC = AC + MBR;
					AC = (AC + (AC >> 18)) & 0777777;	/* propagate carry around */

					if (AC == 0777777)		/* check for -0 */
						AC = 0;
				}

				if ((! (MAR & 0000004)) && (MAR & 0000001))
					/* (1.8) MBX = Transfer MBR contents to XR */
					XR = (MBR & 0017777) | ((MBR >> 4) & 0020000);

				if (((IR & 001) == 01) && ((MAR & 017000) == 010000))
					/* (1.8) HLT = HaLT the computer and sound chime */
					tx0.run = 0;
			}
			break;

		default:		/* Illegal */
			/* ... */
			break;
		}
	}
}

/*
    Simulate a pulse on reset line:
    reset most registers and flip-flops, and initialize a few emulator state
    variables.
*/
static void pulse_reset(void)
{
	/* processor registers */
	PC = 0;			/* ??? */
	IR = 0;			/* ??? */
	/*MBR = 0;*/	/* ??? */
	/*MAR = 0;*/	/* ??? */
	/*AC = 0;*/		/* ??? */
	/*LR = 0;*/		/* ??? */

	/* processor state flip-flops */
	tx0.run = 0;		/* ??? */
	tx0.rim = 0;		/* ??? */
	tx0.ioh = 0;		/* ??? */
	tx0.ios = 0;		/* ??? */

	tx0.rim_step = 0;

	/* now, we kindly ask IO devices to reset, too */
	if (tx0.io_reset_callback)
		(*tx0.io_reset_callback)();
}
