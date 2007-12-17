/*****************************************************************************
 *
 *   m6509.c
 *   Portable 6509 emulator V1.0beta1
 *
 *   Copyright (c) 2000 Peter Trauner, all rights reserved.
 *   documentation by vice emulator team
 *
 *   - This source code is released as freeware for non-commercial purposes.
 *   - You are free to use and redistribute this code in modified or
 *     unmodified form, provided you list me in the credits.
 *   - If you modify this source code, you must add a notice to each modified
 *     source file that it has been changed.  If you're a nice person, you
 *     will clearly mark each change too.  :)
 *   - If you wish to use this for commercial purposes, please contact me at
 *     pullmoll@t-online.de
 *   - The author of this copywritten work reserves the right to change the
 *     terms of its usage and license at any time, including retroactively
 *   - This entire notice must remain in the source code.
 *
 *****************************************************************************/
/*
  2000 March 10 PeT added SO input line

The basic difference is the amount of RAM these machines have been supplied with. The B128 and the CBM *10
models had 128k RAM, the others 256k. This implies some banking scheme, as the 6502 can only address 64k. And
indeed those machines use a 6509, that can address 1 MByte of RAM. It has 2 registers at addresses 0 and 1. The
indirect bank register at address 1 determines the bank (0-15) where the opcodes LDA (zp),Y and STA (zp),Y
take the data from. The exec bank register at address 0 determines the bank where all other read and write
addresses take place.

 vice writes to bank register only with zeropage operand
 0, 1 are bank register in all banks

 lda  (zp),y
 sta  (zp),y

*/

#include "debugger.h"
#include "m6509.h"

#include "ops02.h"
#include "ill02.h"
#include "ops09.h"

#define M6502_NMI_VEC	0xfffa
#define M6502_RST_VEC	0xfffc
#define M6502_IRQ_VEC	0xfffe
#define M6509_RST_VEC	M6502_RST_VEC
#define M6509_IRQ_VEC	M6502_IRQ_VEC
#define M6509_NMI_VEC	M6502_NMI_VEC

#define VERBOSE 0

#if VERBOSE
#define LOG(x)	logerror x
#else
#define LOG(x)
#endif


typedef struct {
	UINT8	subtype;		/* currently selected cpu sub type */
	void	(**insn)(void); /* pointer to the function pointer table */
	PAIR	ppc;			/* previous program counter */
	/* pc.w.h contains the current page pc_bank.w.h for better speed */
	PAIR	pc; 			/* program counter */
	PAIR	sp; 			/* stack pointer (always 100 - 1FF) */
	PAIR	zp; 			/* zero page address */
	PAIR	ea; 			/* effective address */
	UINT8	a;				/* Accumulator */
	UINT8	x;				/* X index register */
	UINT8	y;				/* Y index register */
	PAIR   pc_bank; 	   /* 4 bits, addressed over address 0 */
	PAIR   ind_bank;	   /* 4 bits, addressed over address 1 */
	UINT8	p;				/* Processor status */
	UINT8	pending_irq;	/* nonzero if an IRQ is pending */
	UINT8	after_cli;		/* pending IRQ and last insn cleared I */
	UINT8	nmi_state;
	UINT8	irq_state;
	UINT8	so_state;
	int 	(*irq_callback)(int irqline);	/* IRQ callback */
	read8_handler rdmem_id;					/* readmem callback for indexed instructions */
	write8_handler wrmem_id;				/* readmem callback for indexed instructions */
}	m6509_Regs;

static int m6502_ICount = 0;

static m6509_Regs m6509;

/***************************************************************
 * include the opcode macros, functions and tables
 ***************************************************************/

#include "t6509.c"

static READ8_HANDLER( m6509_read_00000 )
{
	return m6509.pc_bank.b.h2;
}

static READ8_HANDLER( m6509_read_00001 )
{
	return m6509.ind_bank.b.h2;
}

static WRITE8_HANDLER( m6509_write_00000 )
{
	m6509.pc_bank.b.h2=data&0xf;
	m6509.pc.w.h=m6509.pc_bank.w.h;
	change_pc(PCD);
}

static WRITE8_HANDLER( m6509_write_00001 )
{
	m6509.ind_bank.b.h2=data&0xf;
}

static ADDRESS_MAP_START(m6509_mem, ADDRESS_SPACE_PROGRAM, 8)
	AM_RANGE(0x00000, 0x00000) AM_MIRROR(0xF0000) AM_READWRITE(m6509_read_00000, m6509_write_00000)
	AM_RANGE(0x00001, 0x00001) AM_MIRROR(0xF0000) AM_READWRITE(m6509_read_00001, m6509_write_00001)
ADDRESS_MAP_END

static void m6509_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	m6509.rdmem_id = program_read_byte_8;
	m6509.wrmem_id = program_write_byte_8;
	m6509.irq_callback = irqcallback;
}

static void m6509_reset (void)
{
	m6509.insn = insn6509;

	m6509.pc_bank.d=m6509.ind_bank.d=0;
	m6509.pc_bank.b.h2=m6509.ind_bank.b.h2=0xf; /* cbm500 needs this */
	m6509.pc.w.h=m6509.pc_bank.w.h;
	/* wipe out the rest of the m6509 structure */
	/* read the reset vector into PC */
	PCL = RDMEM(M6509_RST_VEC|PB);
	PCH = RDMEM((M6509_RST_VEC+1)|PB);

	m6509.sp.d = 0x01ff;
	m6509.p = F_T|F_B|F_I|F_Z|(P&F_D);	/* set T, I and Z flags */
	m6509.pending_irq = 0;	/* nonzero if an IRQ is pending */
	m6509.after_cli = 0;	/* pending IRQ and last insn cleared I */
	m6509.irq_callback = NULL;

	change_pc(PCD);
}

static void m6509_exit(void)
{
	/* nothing to do yet */
}

static void m6509_get_context (void *dst)
{
	if( dst )
		*(m6509_Regs*)dst = m6509;
}

static void m6509_set_context (void *src)
{
	if( src )
	{
		m6509 = *(m6509_Regs*)src;
		change_pc(PCD);
	}
}



INLINE void m6509_take_irq(void)
{
	if( !(P & F_I) )
	{
		EAD = M6509_IRQ_VEC;
		EAWH = PBWH;
		m6502_ICount -= 2;
		PUSH(PCH);
		PUSH(PCL);
		PUSH(P & ~F_B);
		P |= F_I;		/* knock out D and set I flag */
		PCL = RDMEM(EAD);
		PCH = RDMEM(EAD+1);
		LOG(("M6509#%d takes IRQ ($%04x)\n", cpu_getactivecpu(), PCD));
		/* call back the cpuintrf to let it clear the line */
		if (m6509.irq_callback) (*m6509.irq_callback)(0);
		change_pc(PCD);
	}
	m6509.pending_irq = 0;
}

static int m6509_execute(int cycles)
{
	m6502_ICount = cycles;

	change_pc(PCD);

	do
	{
		UINT8 op;
		PPC = PCD;

		CALL_MAME_DEBUG;

		/* if an irq is pending, take it now */
		if( m6509.pending_irq )
			m6509_take_irq();

		op = RDOP();
		(*m6509.insn[op])();

		/* check if the I flag was just reset (interrupts enabled) */
		if( m6509.after_cli )
		{
			LOG(("M6509#%d after_cli was >0", cpu_getactivecpu()));
			m6509.after_cli = 0;
			if (m6509.irq_state != CLEAR_LINE)
			{
				LOG((": irq line is asserted: set pending IRQ\n"));
				m6509.pending_irq = 1;
			}
			else
			{
				LOG((": irq line is clear\n"));
			}
		}
		else
		if( m6509.pending_irq )
			m6509_take_irq();

	} while (m6502_ICount > 0);

	return cycles - m6502_ICount;
}

static void m6509_set_irq_line(int irqline, int state)
{
	if (irqline == INPUT_LINE_NMI)
	{
		if (m6509.nmi_state == state) return;
		m6509.nmi_state = state;
		if( state != CLEAR_LINE )
		{
			LOG(( "M6509#%d set_nmi_line(ASSERT)\n", cpu_getactivecpu()));
			EAD = M6509_NMI_VEC;
			EAWH = PBWH;
			m6502_ICount -= 2;
			PUSH(PCH);
			PUSH(PCL);
			PUSH(P & ~F_B);
			P |= F_I;		/* knock out D and set I flag */
			PCL = RDMEM(EAD);
			PCH = RDMEM(EAD+1);
			LOG(("M6509#%d takes NMI ($%04x)\n", cpu_getactivecpu(), PCD));
			change_pc(PCD);
		}
	}
	else
	{
		if( irqline == M6509_SET_OVERFLOW )
		{
			if( m6509.so_state && !state )
			{
				LOG(( "M6509#%d set overflow\n", cpu_getactivecpu()));
				P|=F_V;
			}
			m6509.so_state=state;
			return;
		}
		m6509.irq_state = state;
		if( state != CLEAR_LINE )
		{
			LOG(( "M6509#%d set_irq_line(ASSERT)\n", cpu_getactivecpu()));
			m6509.pending_irq = 1;
		}
	}
}

/**************************************************************************
 * Generic set_info
 **************************************************************************/

static void m6509_set_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + M6509_IRQ_LINE:	m6509_set_irq_line(M6509_IRQ_LINE, info->i); break;
		case CPUINFO_INT_INPUT_STATE + M6509_SET_OVERFLOW:m6509_set_irq_line(M6509_SET_OVERFLOW, info->i); break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:	m6509_set_irq_line(INPUT_LINE_NMI, info->i); break;

		case CPUINFO_INT_PC:							PCW = info->i; change_pc(PCD);			break;
		case CPUINFO_INT_REGISTER + M6509_PC:			m6509.pc.w.l = info->i;					break;
		case CPUINFO_INT_SP:							S = info->i;							break;
		case CPUINFO_INT_REGISTER + M6509_S:			m6509.sp.b.l = info->i;					break;
		case CPUINFO_INT_REGISTER + M6509_P:			m6509.p = info->i;						break;
		case CPUINFO_INT_REGISTER + M6509_A:			m6509.a = info->i;						break;
		case CPUINFO_INT_REGISTER + M6509_X:			m6509.x = info->i;						break;
		case CPUINFO_INT_REGISTER + M6509_Y:			m6509.y = info->i;						break;
		case CPUINFO_INT_REGISTER + M6509_PC_BANK:		m6509.pc_bank.b.h2 = info->i;			break;
		case CPUINFO_INT_REGISTER + M6509_IND_BANK:		m6509.ind_bank.b.h2 = info->i;			break;
		case CPUINFO_INT_REGISTER + M6509_EA:			m6509.ea.w.l = info->i;					break;
		case CPUINFO_INT_REGISTER + M6509_ZP:			m6509.zp.w.l = info->i;					break;

		/* --- the following bits of info are set as pointers to data or functions --- */
		case CPUINFO_PTR_M6502_READINDEXED_CALLBACK:	m6509.rdmem_id = (read8_handler) info->f; break;
		case CPUINFO_PTR_M6502_WRITEINDEXED_CALLBACK:	m6509.wrmem_id = (write8_handler) info->f; break;
	}
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

void m6509_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(m6502);				break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 2;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_LE;					break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 3;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 10;							break;

		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 20;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO: 		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO: 		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + M6509_IRQ_LINE:	info->i = m6509.irq_state;				break;
		case CPUINFO_INT_INPUT_STATE + M6509_SET_OVERFLOW:info->i = m6509.so_state;				break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:	info->i = m6509.nmi_state;				break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = m6509.ppc.w.l;				break;

		case CPUINFO_INT_PC:							info->i = PCD;							break;
		case CPUINFO_INT_REGISTER + M6509_PC:			info->i = m6509.pc.w.l;					break;
		case CPUINFO_INT_SP:							info->i = S;							break;
		case CPUINFO_INT_REGISTER + M6509_S:			info->i = m6509.sp.b.l;					break;
		case CPUINFO_INT_REGISTER + M6509_P:			info->i = m6509.p;						break;
		case CPUINFO_INT_REGISTER + M6509_A:			info->i = m6509.a;						break;
		case CPUINFO_INT_REGISTER + M6509_X:			info->i = m6509.x;						break;
		case CPUINFO_INT_REGISTER + M6509_Y:			info->i = m6509.y;						break;
		case CPUINFO_INT_REGISTER + M6509_PC_BANK:		info->i = m6509.pc_bank.b.h2;			break;
		case CPUINFO_INT_REGISTER + M6509_IND_BANK:		info->i = m6509.ind_bank.b.h2;			break;
		case CPUINFO_INT_REGISTER + M6509_EA:			info->i = m6509.ea.w.l;					break;
		case CPUINFO_INT_REGISTER + M6509_ZP:			info->i = m6509.zp.w.l;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = m6509_set_info;			break;
		case CPUINFO_PTR_GET_CONTEXT:					info->getcontext = m6509_get_context;	break;
		case CPUINFO_PTR_SET_CONTEXT:					info->setcontext = m6509_set_context;	break;
		case CPUINFO_PTR_INIT:							info->init = m6509_init;				break;
		case CPUINFO_PTR_RESET:							info->reset = m6509_reset;				break;
		case CPUINFO_PTR_EXIT:							info->exit = m6509_exit;				break;
		case CPUINFO_PTR_EXECUTE:						info->execute = m6509_execute;			break;
		case CPUINFO_PTR_BURN:							info->burn = NULL;						break;
#ifdef MAME_DEBUG
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = m6502_dasm;			break;
#endif
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &m6502_ICount;			break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP:			info->internal_map = construct_map_m6509_mem; break;
		case CPUINFO_PTR_M6502_READINDEXED_CALLBACK:	info->f = (genf *) m6509.rdmem_id;		break;
		case CPUINFO_PTR_M6502_WRITEINDEXED_CALLBACK:	info->f = (genf *) m6509.wrmem_id;		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "M6509");				break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "MOS Technology 6509"); break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s, "1.0beta");				break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright (c) 1998 Juergen Buchmueller\nCopyright (c) 2000 Peter Trauner\nall rights reserved."); break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "%c%c%c%c%c%c%c%c",
				m6509.p & 0x80 ? 'N':'.',
				m6509.p & 0x40 ? 'V':'.',
				m6509.p & 0x20 ? 'R':'.',
				m6509.p & 0x10 ? 'B':'.',
				m6509.p & 0x08 ? 'D':'.',
				m6509.p & 0x04 ? 'I':'.',
				m6509.p & 0x02 ? 'Z':'.',
				m6509.p & 0x01 ? 'C':'.');
			break;

		case CPUINFO_STR_REGISTER + M6509_PC:			sprintf(info->s, "PC:%04X", m6509.pc.w.l); break;
		case CPUINFO_STR_REGISTER + M6509_S:			sprintf(info->s, "S:%02X", m6509.sp.b.l); break;
		case CPUINFO_STR_REGISTER + M6509_P:			sprintf(info->s, "P:%02X", m6509.p); break;
		case CPUINFO_STR_REGISTER + M6509_A:			sprintf(info->s, "A:%02X", m6509.a); break;
		case CPUINFO_STR_REGISTER + M6509_X:			sprintf(info->s, "X:%02X", m6509.x); break;
		case CPUINFO_STR_REGISTER + M6509_Y:			sprintf(info->s, "Y:%02X", m6509.y); break;
		case CPUINFO_STR_REGISTER + M6509_PC_BANK:		sprintf(info->s, "M0:%01X", m6509.pc_bank.b.h2); break;
		case CPUINFO_STR_REGISTER + M6509_IND_BANK:		sprintf(info->s, "M1:%01X", m6509.ind_bank.b.h2); break;
		case CPUINFO_STR_REGISTER + M6509_EA:			sprintf(info->s, "EA:%04X", m6509.ea.w.l); break;
		case CPUINFO_STR_REGISTER + M6509_ZP:			sprintf(info->s, "ZP:%03X", m6509.zp.w.l); break;
	}
}

