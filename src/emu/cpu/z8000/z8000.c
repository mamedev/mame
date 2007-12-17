/*****************************************************************************
 *
 *   z8000.c
 *   Portable Z8000(2) emulator
 *   Z8000 MAME interface
 *
 *   Copyright (C) 1998,1999,2000 Juergen Buchmueller, all rights reserved.
 *   You can contact me at juergen@mame.net or pullmoll@stop1984.com
 *
 *   - This source code is released as freeware for non-commercial purposes
 *     as part of the M.A.M.E. (Multiple Arcade Machine Emulator) project.
 *     The licensing terms of MAME apply to this piece of code for the MAME
 *     project and derviative works, as defined by the MAME license. You
 *     may opt to make modifications, improvements or derivative works under
 *     that same conditions, and the MAME project may opt to keep
 *     modifications, improvements or derivatives under their terms exclusively.
 *
 *   - Alternatively you can choose to apply the terms of the "GPL" (see
 *     below) to this - and only this - piece of code or your derivative works.
 *     Note that in no case your choice can have any impact on any other
 *     source code of the MAME project, or binary, or executable, be it closely
 *     or losely related to this piece of code.
 *
 *  -  At your choice you are also free to remove either licensing terms from
 *     this file and continue to use it under only one of the two licenses. Do this
 *     if you think that licenses are not compatible (enough) for you, or if you
 *     consider either license 'too restrictive' or 'too free'.
 *
 *  -  GPL (GNU General Public License)
 *     This program is free software; you can redistribute it and/or
 *     modify it under the terms of the GNU General Public License
 *     as published by the Free Software Foundation; either version 2
 *     of the License, or (at your option) any later version.
 *
 *     This program is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public License
 *     along with this program; if not, write to the Free Software
 *     Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 *****************************************************************************/

#include "debugger.h"
#include "z8000.h"
#include "z8000cpu.h"
#include "osd_cpu.h"

#define VERBOSE 0


#if VERBOSE
#define LOG(x)	logerror x
#else
#define LOG(x)
#endif

/* opcode execution table */
Z8000_exec *z8000_exec = NULL;

typedef union {
    UINT8   B[16]; /* RL0,RH0,RL1,RH1...RL7,RH7 */
    UINT16  W[16]; /* R0,R1,R2...R15 */
    UINT32  L[8];  /* RR0,RR2,RR4..RR14 */
    UINT64  Q[4];  /* RQ0,RQ4,..RQ12 */
}   z8000_reg_file;

typedef struct {
    UINT16  op[4];      /* opcodes/data of current instruction */
	UINT16	ppc;		/* previous program counter */
    UINT16  pc;         /* program counter */
    UINT16  psap;       /* program status pointer */
    UINT16  fcw;        /* flags and control word */
    UINT16  refresh;    /* refresh timer/counter */
    UINT16  nsp;        /* system stack pointer */
    UINT16  irq_req;    /* CPU is halted, interrupt or trap request */
    UINT16  irq_srv;    /* serviced interrupt request */
    UINT16  irq_vec;    /* interrupt vector */
    z8000_reg_file regs;/* registers */
	int nmi_state;		/* NMI line state */
	int irq_state[2];	/* IRQ line states (NVI, VI) */
    int (*irq_callback)(int irqline);
}   z8000_Regs;

int z8000_ICount;

/* current CPU context */
static z8000_Regs Z;

/* zero, sign and parity flags for logical byte operations */
static UINT8 z8000_zsp[256];

/* conversion table for Z8000 DAB opcode */
#include "z8000dab.h"

/**************************************************************************
 * This is the register file layout:
 *
 * BYTE        WORD         LONG           QUAD
 * msb   lsb       bits           bits           bits
 * RH0 - RL0   R 0 15- 0    RR 0  31-16    RQ 0  63-48
 * RH1 - RL1   R 1 15- 0          15- 0          47-32
 * RH2 - RL2   R 2 15- 0    RR 2  31-16          31-16
 * RH3 - RL3   R 3 15- 0          15- 0          15- 0
 * RH4 - RL4   R 4 15- 0    RR 4  31-16    RQ 4  63-48
 * RH5 - RL5   R 5 15- 0          15- 0          47-32
 * RH6 - RL6   R 6 15- 0    RR 6  31-16          31-16
 * RH7 - RL7   R 7 15- 0          15- 0          15- 0
 *             R 8 15- 0    RR 8  31-16    RQ 8  63-48
 *             R 9 15- 0          15- 0          47-32
 *             R10 15- 0    RR10  31-16          31-16
 *             R11 15- 0          15- 0          15- 0
 *             R12 15- 0    RR12  31-16    RQ12  63-48
 *             R13 15- 0          15- 0          47-32
 *             R14 15- 0    RR14  31-16          31-16
 *             R15 15- 0          15- 0          15- 0
 *
 * Note that for LSB_FIRST machines we have the case that the RR registers
 * use the lower numbered R registers in the higher bit positions.
 * And also the RQ registers use the lower numbered RR registers in the
 * higher bit positions.
 * That's the reason for the ordering in the following pointer table.
 **************************************************************************/
#ifdef	LSB_FIRST
	/* pointers to byte (8bit) registers */
	static UINT8	*pRB[16] =
	{
		&Z.regs.B[ 7],&Z.regs.B[ 5],&Z.regs.B[ 3],&Z.regs.B[ 1],
		&Z.regs.B[15],&Z.regs.B[13],&Z.regs.B[11],&Z.regs.B[ 9],
		&Z.regs.B[ 6],&Z.regs.B[ 4],&Z.regs.B[ 2],&Z.regs.B[ 0],
		&Z.regs.B[14],&Z.regs.B[12],&Z.regs.B[10],&Z.regs.B[ 8]
	};

	static UINT16	*pRW[16] =
	{
        &Z.regs.W[ 3],&Z.regs.W[ 2],&Z.regs.W[ 1],&Z.regs.W[ 0],
        &Z.regs.W[ 7],&Z.regs.W[ 6],&Z.regs.W[ 5],&Z.regs.W[ 4],
        &Z.regs.W[11],&Z.regs.W[10],&Z.regs.W[ 9],&Z.regs.W[ 8],
        &Z.regs.W[15],&Z.regs.W[14],&Z.regs.W[13],&Z.regs.W[12]
    };

    /* pointers to long (32bit) registers */
	static UINT32	*pRL[16] =
	{
		&Z.regs.L[ 1],&Z.regs.L[ 1],&Z.regs.L[ 0],&Z.regs.L[ 0],
		&Z.regs.L[ 3],&Z.regs.L[ 3],&Z.regs.L[ 2],&Z.regs.L[ 2],
		&Z.regs.L[ 5],&Z.regs.L[ 5],&Z.regs.L[ 4],&Z.regs.L[ 4],
		&Z.regs.L[ 7],&Z.regs.L[ 7],&Z.regs.L[ 6],&Z.regs.L[ 6]
    };

#else	/* MSB_FIRST */

    /* pointers to byte (8bit) registers */
	static UINT8	*pRB[16] =
	{
		&Z.regs.B[ 0],&Z.regs.B[ 2],&Z.regs.B[ 4],&Z.regs.B[ 6],
		&Z.regs.B[ 8],&Z.regs.B[10],&Z.regs.B[12],&Z.regs.B[14],
		&Z.regs.B[ 1],&Z.regs.B[ 3],&Z.regs.B[ 5],&Z.regs.B[ 7],
		&Z.regs.B[ 9],&Z.regs.B[11],&Z.regs.B[13],&Z.regs.B[15]
	};

	/* pointers to word (16bit) registers */
	static UINT16	*pRW[16] =
	{
		&Z.regs.W[ 0],&Z.regs.W[ 1],&Z.regs.W[ 2],&Z.regs.W[ 3],
		&Z.regs.W[ 4],&Z.regs.W[ 5],&Z.regs.W[ 6],&Z.regs.W[ 7],
		&Z.regs.W[ 8],&Z.regs.W[ 9],&Z.regs.W[10],&Z.regs.W[11],
		&Z.regs.W[12],&Z.regs.W[13],&Z.regs.W[14],&Z.regs.W[15]
	};

	/* pointers to long (32bit) registers */
	static UINT32	*pRL[16] =
	{
		&Z.regs.L[ 0],&Z.regs.L[ 0],&Z.regs.L[ 1],&Z.regs.L[ 1],
		&Z.regs.L[ 2],&Z.regs.L[ 2],&Z.regs.L[ 3],&Z.regs.L[ 3],
		&Z.regs.L[ 4],&Z.regs.L[ 4],&Z.regs.L[ 5],&Z.regs.L[ 5],
		&Z.regs.L[ 6],&Z.regs.L[ 6],&Z.regs.L[ 7],&Z.regs.L[ 7]
	};

#endif

/* pointers to quad word (64bit) registers */
static UINT64   *pRQ[16] = {
    &Z.regs.Q[ 0],&Z.regs.Q[ 0],&Z.regs.Q[ 0],&Z.regs.Q[ 0],
    &Z.regs.Q[ 1],&Z.regs.Q[ 1],&Z.regs.Q[ 1],&Z.regs.Q[ 1],
    &Z.regs.Q[ 2],&Z.regs.Q[ 2],&Z.regs.Q[ 2],&Z.regs.Q[ 2],
    &Z.regs.Q[ 3],&Z.regs.Q[ 3],&Z.regs.Q[ 3],&Z.regs.Q[ 3]};

INLINE UINT16 RDOP(void)
{
	UINT16 res = cpu_readop16(PC);
    PC += 2;
    return res;
}

INLINE UINT8 RDMEM_B(UINT16 addr)
{
	return program_read_byte_16be(addr);
}

INLINE UINT16 RDMEM_W(UINT16 addr)
{
	addr &= ~1;
	return program_read_word_16be(addr);
}

INLINE UINT32 RDMEM_L(UINT16 addr)
{
	UINT32 result;
	addr &= ~1;
	result = program_read_word_16be(addr) << 16;
	return result + program_read_word_16be(addr + 2);
}

INLINE void WRMEM_B(UINT16 addr, UINT8 value)
{
	program_write_byte_16be(addr, value);
}

INLINE void WRMEM_W(UINT16 addr, UINT16 value)
{
	addr &= ~1;
	program_write_word_16be(addr, value);
}

INLINE void WRMEM_L(UINT16 addr, UINT32 value)
{
	addr &= ~1;
	program_write_word_16be(addr, value >> 16);
	program_write_word_16be((UINT16)(addr + 2), value & 0xffff);
}

INLINE UINT8 RDPORT_B(int mode, UINT16 addr)
{
	if( mode == 0 )
	{
		return io_read_byte_8(addr);
	}
	else
	{
		/* how to handle MMU reads? */
		return 0x00;
	}
}

INLINE UINT16 RDPORT_W(int mode, UINT16 addr)
{
	if( mode == 0 )
	{
		return io_read_byte_8((UINT16)(addr)) +
			  (io_read_byte_8((UINT16)(addr+1)) << 8);
	}
	else
	{
		/* how to handle MMU reads? */
		return 0x0000;
	}
}

INLINE UINT32 RDPORT_L(int mode, UINT16 addr)
{
	if( mode == 0 )
	{
		return	io_read_byte_8((UINT16)(addr)) +
			   (io_read_byte_8((UINT16)(addr+1)) <<  8) +
			   (io_read_byte_8((UINT16)(addr+2)) << 16) +
			   (io_read_byte_8((UINT16)(addr+3)) << 24);
	}
	else
	{
		/* how to handle MMU reads? */
		return 0x00000000;
	}
}

INLINE void WRPORT_B(int mode, UINT16 addr, UINT8 value)
{
	if( mode == 0 )
	{
        io_write_byte_8(addr,value);
	}
	else
	{
		/* how to handle MMU writes? */
    }
}

INLINE void WRPORT_W(int mode, UINT16 addr, UINT16 value)
{
	if( mode == 0 )
	{
		io_write_byte_8((UINT16)(addr),value & 0xff);
		io_write_byte_8((UINT16)(addr+1),(value >> 8) & 0xff);
	}
	else
	{
		/* how to handle MMU writes? */
    }
}

INLINE void WRPORT_L(int mode, UINT16 addr, UINT32 value)
{
	if( mode == 0 )
	{
		io_write_byte_8((UINT16)(addr),value & 0xff);
		io_write_byte_8((UINT16)(addr+1),(value >> 8) & 0xff);
		io_write_byte_8((UINT16)(addr+2),(value >> 16) & 0xff);
		io_write_byte_8((UINT16)(addr+3),(value >> 24) & 0xff);
	}
	else
	{
		/* how to handle MMU writes? */
	}
}

#include "z8000ops.c"
#include "z8000tbl.c"

INLINE void set_irq(int type)
{
    switch ((type >> 8) & 255)
    {
        case Z8000_TRAP >> 8:
            if (IRQ_SRV >= Z8000_TRAP)
                return; /* double TRAP.. very bad :( */
            IRQ_REQ = type;
            break;
        case Z8000_NMI >> 8:
            if (IRQ_SRV >= Z8000_NMI)
                return; /* no NMIs inside trap */
            IRQ_REQ = type;
            break;
        case Z8000_SEGTRAP >> 8:
            if (IRQ_SRV >= Z8000_SEGTRAP)
                return; /* no SEGTRAPs inside NMI/TRAP */
            IRQ_REQ = type;
            break;
        case Z8000_NVI >> 8:
            if (IRQ_SRV >= Z8000_NVI)
                return; /* no NVIs inside SEGTRAP/NMI/TRAP */
            IRQ_REQ = type;
            break;
        case Z8000_VI >> 8:
            if (IRQ_SRV >= Z8000_VI)
                return; /* no VIs inside NVI/SEGTRAP/NMI/TRAP */
            IRQ_REQ = type;
            break;
        case Z8000_SYSCALL >> 8:
            LOG(("Z8K#%d SYSCALL $%02x\n", cpu_getactivecpu(), type & 0xff));
            IRQ_REQ = type;
            break;
        default:
            logerror("Z8000 invalid Cause_Interrupt %04x\n", type);
            return;
    }
    /* set interrupt request flag, reset HALT flag */
    IRQ_REQ = type & ~Z8000_HALT;
}


INLINE void Interrupt(void)
{
    UINT16 fcw = FCW;

    if (IRQ_REQ & Z8000_NVI)
    {
        int type = (*Z.irq_callback)(0);
        set_irq(type);
    }

    if (IRQ_REQ & Z8000_VI)
    {
        int type = (*Z.irq_callback)(1);
        set_irq(type);
    }

   /* trap ? */
   if ( IRQ_REQ & Z8000_TRAP )
   {
        CHANGE_FCW(fcw | F_S_N);/* swap to system stack */
        PUSHW( SP, PC );        /* save current PC */
        PUSHW( SP, fcw );       /* save current FCW */
        PUSHW( SP, IRQ_REQ );   /* save interrupt/trap type tag */
        IRQ_SRV = IRQ_REQ;
        IRQ_REQ &= ~Z8000_TRAP;
        PC = TRAP;
        LOG(("Z8K#%d trap $%04x\n", cpu_getactivecpu(), PC ));
   }
   else
   if ( IRQ_REQ & Z8000_SYSCALL )
   {
        CHANGE_FCW(fcw | F_S_N);/* swap to system stack */
        PUSHW( SP, PC );        /* save current PC */
        PUSHW( SP, fcw );       /* save current FCW */
        PUSHW( SP, IRQ_REQ );   /* save interrupt/trap type tag */
        IRQ_SRV = IRQ_REQ;
        IRQ_REQ &= ~Z8000_SYSCALL;
        PC = SYSCALL;
        LOG(("Z8K#%d syscall $%04x\n", cpu_getactivecpu(), PC ));
   }
   else
   if ( IRQ_REQ & Z8000_SEGTRAP )
   {
        CHANGE_FCW(fcw | F_S_N);/* swap to system stack */
        PUSHW( SP, PC );        /* save current PC */
        PUSHW( SP, fcw );       /* save current FCW */
        PUSHW( SP, IRQ_REQ );   /* save interrupt/trap type tag */
        IRQ_SRV = IRQ_REQ;
        IRQ_REQ &= ~Z8000_SEGTRAP;
        PC = SEGTRAP;
        LOG(("Z8K#%d segtrap $%04x\n", cpu_getactivecpu(), PC ));
   }
   else
   if ( IRQ_REQ & Z8000_NMI )
   {
        CHANGE_FCW(fcw | F_S_N);/* swap to system stack */
        PUSHW( SP, PC );        /* save current PC */
        PUSHW( SP, fcw );       /* save current FCW */
        PUSHW( SP, IRQ_REQ );   /* save interrupt/trap type tag */
        IRQ_SRV = IRQ_REQ;
        fcw = RDMEM_W( NMI );
        PC = RDMEM_W( NMI + 2 );
        IRQ_REQ &= ~Z8000_NMI;
        CHANGE_FCW(fcw);
        PC = NMI;
        LOG(("Z8K#%d NMI $%04x\n", cpu_getactivecpu(), PC ));
    }
    else
    if ( (IRQ_REQ & Z8000_NVI) && (FCW & F_NVIE) )
    {
        CHANGE_FCW(fcw | F_S_N);/* swap to system stack */
        PUSHW( SP, PC );        /* save current PC */
        PUSHW( SP, fcw );       /* save current FCW */
        PUSHW( SP, IRQ_REQ );   /* save interrupt/trap type tag */
        IRQ_SRV = IRQ_REQ;
        fcw = RDMEM_W( NVI );
        PC = RDMEM_W( NVI + 2 );
        IRQ_REQ &= ~Z8000_NVI;
        CHANGE_FCW(fcw);
        LOG(("Z8K#%d NVI $%04x\n", cpu_getactivecpu(), PC ));
    }
    else
    if ( (IRQ_REQ & Z8000_VI) && (FCW & F_VIE) )
    {
        CHANGE_FCW(fcw | F_S_N);/* swap to system stack */
        PUSHW( SP, PC );        /* save current PC */
        PUSHW( SP, fcw );       /* save current FCW */
        PUSHW( SP, IRQ_REQ );   /* save interrupt/trap type tag */
        IRQ_SRV = IRQ_REQ;
        fcw = RDMEM_W( IRQ_VEC );
        PC = RDMEM_W( VEC00 + 2 * (IRQ_REQ & 0xff) );
        IRQ_REQ &= ~Z8000_VI;
        CHANGE_FCW(fcw);
        LOG(("Z8K#%d VI [$%04x/$%04x] fcw $%04x, pc $%04x\n", cpu_getactivecpu(), IRQ_VEC, VEC00 + VEC00 + 2 * (IRQ_REQ & 0xff), FCW, PC ));
    }
}


static void z8000_reset(void)
{
	int (*save_irqcallback)(int) = Z.irq_callback;
	memset(&Z, 0, sizeof(z8000_Regs));
	Z.irq_callback = save_irqcallback;
	FCW = RDMEM_W( 2 ); /* get reset FCW */
	PC	= RDMEM_W( 4 ); /* get reset PC  */
	change_pc(PC);
}

static void z8000_exit(void)
{
	z8000_deinit();
}

static int z8000_execute(int cycles)
{
    z8000_ICount = cycles;

    do
    {
        /* any interrupt request pending? */
        if (IRQ_REQ)
			Interrupt();

		CALL_MAME_DEBUG;

		if (IRQ_REQ & Z8000_HALT)
        {
            z8000_ICount = 0;
        }
        else
        {
            Z8000_exec *exec;
            Z.op[0] = RDOP();
            exec = &z8000_exec[Z.op[0]];

            if (exec->size > 1)
                Z.op[1] = RDOP();
            if (exec->size > 2)
                Z.op[2] = RDOP();

            z8000_ICount -= exec->cycles;
            (*exec->opcode)();

        }
    } while (z8000_ICount > 0);

    return cycles - z8000_ICount;

}

static void z8000_get_context(void *dst)
{
	if( dst )
		*(z8000_Regs*)dst = Z;
}

static void z8000_set_context(void *src)
{
	if( src )
	{
		Z = *(z8000_Regs*)src;
		change_pc(PC);
	}
}

static void set_irq_line(int irqline, int state)
{
	if (irqline == INPUT_LINE_NMI)
	{
		if (Z.nmi_state == state)
			return;

	    Z.nmi_state = state;

	    if (state != CLEAR_LINE)
		{
			if (IRQ_SRV >= Z8000_NMI)	/* no NMIs inside trap */
				return;
			IRQ_REQ = Z8000_NMI;
			IRQ_VEC = NMI;
		}
	}
	else if (irqline < 2)
	{
		Z.irq_state[irqline] = state;
		if (irqline == 0)
		{
			if (state == CLEAR_LINE)
			{
				if (!(FCW & F_NVIE))
					IRQ_REQ &= ~Z8000_NVI;
			}
			else
			{
				if (FCW & F_NVIE)
					IRQ_REQ |= Z8000_NVI;
	        }
		}
		else
		{
			if (state == CLEAR_LINE)
			{
				if (!(FCW & F_VIE))
					IRQ_REQ &= ~Z8000_VI;
			}
			else
			{
				if (FCW & F_VIE)
					IRQ_REQ |= Z8000_VI;
			}
		}
	}
}



/**************************************************************************
 * Generic set_info
 **************************************************************************/

static void z8000_set_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:	set_irq_line(INPUT_LINE_NMI, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + 0:				set_irq_line(0, info->i);				break;
		case CPUINFO_INT_INPUT_STATE + 1:				set_irq_line(1, info->i);				break;

		case CPUINFO_INT_PC:							PC = info->i; change_pc(PC);	 		break;
		case CPUINFO_INT_REGISTER + Z8000_PC:			PC = info->i;							break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + Z8000_NSP:			NSP = info->i;							break;
		case CPUINFO_INT_REGISTER + Z8000_FCW:			FCW = info->i;							break;
		case CPUINFO_INT_REGISTER + Z8000_PSAP:			PSAP = info->i;							break;
		case CPUINFO_INT_REGISTER + Z8000_REFRESH:		REFRESH = info->i;						break;
		case CPUINFO_INT_REGISTER + Z8000_IRQ_REQ:		IRQ_REQ = info->i;						break;
		case CPUINFO_INT_REGISTER + Z8000_IRQ_SRV:		IRQ_SRV = info->i;						break;
		case CPUINFO_INT_REGISTER + Z8000_IRQ_VEC:		IRQ_VEC = info->i;						break;
		case CPUINFO_INT_REGISTER + Z8000_R0:			RW( 0) = info->i;						break;
		case CPUINFO_INT_REGISTER + Z8000_R1:			RW( 1) = info->i;						break;
		case CPUINFO_INT_REGISTER + Z8000_R2:			RW( 2) = info->i;						break;
		case CPUINFO_INT_REGISTER + Z8000_R3:			RW( 3) = info->i;						break;
		case CPUINFO_INT_REGISTER + Z8000_R4:			RW( 4) = info->i;						break;
		case CPUINFO_INT_REGISTER + Z8000_R5:			RW( 5) = info->i;						break;
		case CPUINFO_INT_REGISTER + Z8000_R6:			RW( 6) = info->i;						break;
		case CPUINFO_INT_REGISTER + Z8000_R7:			RW( 7) = info->i;						break;
		case CPUINFO_INT_REGISTER + Z8000_R8:			RW( 8) = info->i;						break;
		case CPUINFO_INT_REGISTER + Z8000_R9:			RW( 9) = info->i;						break;
		case CPUINFO_INT_REGISTER + Z8000_R10:			RW(10) = info->i;						break;
		case CPUINFO_INT_REGISTER + Z8000_R11:			RW(11) = info->i;						break;
		case CPUINFO_INT_REGISTER + Z8000_R12:			RW(12) = info->i;						break;
		case CPUINFO_INT_REGISTER + Z8000_R13:			RW(13) = info->i;						break;
		case CPUINFO_INT_REGISTER + Z8000_R14:			RW(14) = info->i;						break;
		case CPUINFO_INT_REGISTER + Z8000_R15:			RW(15) = info->i;						break;
	}
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

void z8000_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(Z);					break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 2;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0xff;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_BE;					break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 2;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 6;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 16;							break;

		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO: 		info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO: 		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:	info->i = Z.nmi_state;					break;
		case CPUINFO_INT_INPUT_STATE + 0:				info->i = Z.irq_state[0];				break;
		case CPUINFO_INT_INPUT_STATE + 1:				info->i = Z.irq_state[1];				break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = PPC;							break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + Z8000_PC:			info->i = PC;							break;
		case CPUINFO_INT_SP:
        case CPUINFO_INT_REGISTER + Z8000_NSP:			info->i = NSP;							break;
        case CPUINFO_INT_REGISTER + Z8000_FCW:			info->i = FCW;							break;
		case CPUINFO_INT_REGISTER + Z8000_PSAP:			info->i = PSAP;							break;
		case CPUINFO_INT_REGISTER + Z8000_REFRESH:		info->i = REFRESH;						break;
		case CPUINFO_INT_REGISTER + Z8000_IRQ_REQ:		info->i = IRQ_REQ;						break;
		case CPUINFO_INT_REGISTER + Z8000_IRQ_SRV:		info->i = IRQ_SRV;						break;
		case CPUINFO_INT_REGISTER + Z8000_IRQ_VEC:		info->i = IRQ_VEC;						break;
		case CPUINFO_INT_REGISTER + Z8000_R0:			info->i = RW( 0);						break;
		case CPUINFO_INT_REGISTER + Z8000_R1:			info->i = RW( 1);						break;
		case CPUINFO_INT_REGISTER + Z8000_R2:			info->i = RW( 2);						break;
		case CPUINFO_INT_REGISTER + Z8000_R3:			info->i = RW( 3);						break;
		case CPUINFO_INT_REGISTER + Z8000_R4:			info->i = RW( 4);						break;
		case CPUINFO_INT_REGISTER + Z8000_R5:			info->i = RW( 5);						break;
		case CPUINFO_INT_REGISTER + Z8000_R6:			info->i = RW( 6);						break;
		case CPUINFO_INT_REGISTER + Z8000_R7:			info->i = RW( 7);						break;
		case CPUINFO_INT_REGISTER + Z8000_R8:			info->i = RW( 8);						break;
		case CPUINFO_INT_REGISTER + Z8000_R9:			info->i = RW( 9);						break;
		case CPUINFO_INT_REGISTER + Z8000_R10:			info->i = RW(10);						break;
		case CPUINFO_INT_REGISTER + Z8000_R11:			info->i = RW(11);						break;
		case CPUINFO_INT_REGISTER + Z8000_R12:			info->i = RW(12);						break;
		case CPUINFO_INT_REGISTER + Z8000_R13:			info->i = RW(13);						break;
		case CPUINFO_INT_REGISTER + Z8000_R14:			info->i = RW(14);						break;
		case CPUINFO_INT_REGISTER + Z8000_R15:			info->i = RW(15);						break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = z8000_set_info;			break;
		case CPUINFO_PTR_GET_CONTEXT:					info->getcontext = z8000_get_context;	break;
		case CPUINFO_PTR_SET_CONTEXT:					info->setcontext = z8000_set_context;	break;
		case CPUINFO_PTR_INIT:							info->init = z8000_init;				break;
		case CPUINFO_PTR_RESET:							info->reset = z8000_reset;				break;
		case CPUINFO_PTR_EXIT:							info->exit = z8000_exit;				break;
		case CPUINFO_PTR_EXECUTE:						info->execute = z8000_execute;			break;
		case CPUINFO_PTR_BURN:							info->burn = NULL;						break;
#ifdef MAME_DEBUG
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = z8000_dasm;			break;
#endif /* MAME_DEBUG */
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &z8000_ICount;			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "Z8002");				break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "Zilog Z8000");			break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s, "1.1");					break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright (C) 1998,1999 Juergen Buchmueller, all rights reserved."); break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
				Z.fcw & 0x8000 ? 's':'.',
				Z.fcw & 0x4000 ? 'n':'.',
				Z.fcw & 0x2000 ? 'e':'.',
				Z.fcw & 0x1000 ? '2':'.',
				Z.fcw & 0x0800 ? '1':'.',
				Z.fcw & 0x0400 ? '?':'.',
				Z.fcw & 0x0200 ? '?':'.',
				Z.fcw & 0x0100 ? '?':'.',
				Z.fcw & 0x0080 ? 'C':'.',
				Z.fcw & 0x0040 ? 'Z':'.',
				Z.fcw & 0x0020 ? 'S':'.',
				Z.fcw & 0x0010 ? 'V':'.',
				Z.fcw & 0x0008 ? 'D':'.',
				Z.fcw & 0x0004 ? 'H':'.',
				Z.fcw & 0x0002 ? '?':'.',
				Z.fcw & 0x0001 ? '?':'.');
            break;

		case CPUINFO_STR_REGISTER + Z8000_PC:			sprintf(info->s, "PC :%04X", Z.pc);		break;
		case CPUINFO_STR_REGISTER + Z8000_NSP:			sprintf(info->s, "SP :%04X", Z.nsp);	break;
		case CPUINFO_STR_REGISTER + Z8000_FCW:			sprintf(info->s, "FCW:%04X", Z.fcw);	break;
		case CPUINFO_STR_REGISTER + Z8000_PSAP:			sprintf(info->s, "NSP:%04X", Z.psap);	break;
		case CPUINFO_STR_REGISTER + Z8000_REFRESH:		sprintf(info->s, "REFR:%04X", Z.refresh); break;
		case CPUINFO_STR_REGISTER + Z8000_IRQ_REQ:		sprintf(info->s, "IRQR:%04X", Z.irq_req); break;
		case CPUINFO_STR_REGISTER + Z8000_IRQ_SRV:		sprintf(info->s, "IRQS:%04X", Z.irq_srv); break;
		case CPUINFO_STR_REGISTER + Z8000_IRQ_VEC:		sprintf(info->s, "IRQV:%04X", Z.irq_vec); break;
#ifdef	LSB_FIRST
#define REG_XOR 3
#else
#define REG_XOR 0
#endif
		case CPUINFO_STR_REGISTER + Z8000_R0:			sprintf(info->s, "R0 :%04X", Z.regs.W[ 0^REG_XOR]); break;
		case CPUINFO_STR_REGISTER + Z8000_R1:			sprintf(info->s, "R1 :%04X", Z.regs.W[ 1^REG_XOR]); break;
		case CPUINFO_STR_REGISTER + Z8000_R2:			sprintf(info->s, "R2 :%04X", Z.regs.W[ 2^REG_XOR]); break;
		case CPUINFO_STR_REGISTER + Z8000_R3:			sprintf(info->s, "R3 :%04X", Z.regs.W[ 3^REG_XOR]); break;
		case CPUINFO_STR_REGISTER + Z8000_R4:			sprintf(info->s, "R4 :%04X", Z.regs.W[ 4^REG_XOR]); break;
		case CPUINFO_STR_REGISTER + Z8000_R5:			sprintf(info->s, "R5 :%04X", Z.regs.W[ 5^REG_XOR]); break;
		case CPUINFO_STR_REGISTER + Z8000_R6:			sprintf(info->s, "R6 :%04X", Z.regs.W[ 6^REG_XOR]); break;
		case CPUINFO_STR_REGISTER + Z8000_R7:			sprintf(info->s, "R7 :%04X", Z.regs.W[ 7^REG_XOR]); break;
		case CPUINFO_STR_REGISTER + Z8000_R8:			sprintf(info->s, "R8 :%04X", Z.regs.W[ 8^REG_XOR]); break;
		case CPUINFO_STR_REGISTER + Z8000_R9:			sprintf(info->s, "R9 :%04X", Z.regs.W[ 9^REG_XOR]); break;
		case CPUINFO_STR_REGISTER + Z8000_R10:			sprintf(info->s, "R10:%04X", Z.regs.W[10^REG_XOR]); break;
		case CPUINFO_STR_REGISTER + Z8000_R11:			sprintf(info->s, "R11:%04X", Z.regs.W[11^REG_XOR]); break;
		case CPUINFO_STR_REGISTER + Z8000_R12:			sprintf(info->s, "R12:%04X", Z.regs.W[12^REG_XOR]); break;
		case CPUINFO_STR_REGISTER + Z8000_R13:			sprintf(info->s, "R13:%04X", Z.regs.W[13^REG_XOR]); break;
		case CPUINFO_STR_REGISTER + Z8000_R14:			sprintf(info->s, "R14:%04X", Z.regs.W[14^REG_XOR]); break;
		case CPUINFO_STR_REGISTER + Z8000_R15:			sprintf(info->s, "R15:%04X", Z.regs.W[15^REG_XOR]); break;
	}
}
