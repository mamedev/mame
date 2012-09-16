/*****************************************************************************
 *
 *   z8000.c
 *   Portable Z8000(2) emulator
 *   Z8000 MAME interface
 *
 *   Copyright Juergen Buchmueller, all rights reserved.
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
 *     TODO:
 *     - make the z8001 opcodes to be dynamic (i.e. to take segmented mode flag into account and use the non-segmented mode)
 *     - dissassembler doesn't work at all with the z8001
 *
 *****************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "z8000.h"

#define VERBOSE 0


#define LOG(x)	do { if (VERBOSE) logerror x; } while (0)

union z8000_reg_file
{
    UINT8   B[16]; /* RL0,RH0,RL1,RH1...RL7,RH7 */
    UINT16  W[16]; /* R0,R1,R2...R15 */
    UINT32  L[8];  /* RR0,RR2,RR4..RR14 */
    UINT64  Q[4];  /* RQ0,RQ4,..RQ12 */
};

struct z8000_state
{
    UINT32  op[4];      /* opcodes/data of current instruction */
	UINT32	ppc;		/* previous program counter */
    UINT32  pc;         /* program counter */
    UINT16  psapseg;    /* program status pointer, segment (Z8001 only) */
    UINT16  psapoff;    /* program status pointer, offset */
    UINT16  fcw;        /* flags and control word */
    UINT16  refresh;    /* refresh timer/counter */
    UINT16  nspseg;     /* system stack pointer, segment (Z8001 only) */
    UINT16  nspoff;     /* system stack pointer, offset */
    UINT16  irq_req;    /* CPU is halted, interrupt or trap request */
    UINT16  irq_srv;    /* serviced interrupt request */
    UINT16  irq_vec;    /* interrupt vector */
    unsigned int op_valid;  /* bit field indicating if given op[] field is already initialized */
    z8000_reg_file regs;/* registers */
	int nmi_state;		/* NMI line state */
	int irq_state[2];	/* IRQ line states (NVI, VI) */
	device_irq_acknowledge_callback irq_callback;
	legacy_cpu_device *device;
	address_space *program;
	direct_read_data *direct;
	address_space *io;
	int icount;
};

#include "z8000cpu.h"

INLINE z8000_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == Z8001 || device->type() == Z8002);
	return (z8000_state *)downcast<legacy_cpu_device *>(device)->token();
}

/* opcode execution table */
Z8000_exec *z8000_exec = NULL;

/* zero, sign and parity flags for logical byte operations */
static UINT8 z8000_zsp[256];

INLINE int segmented_mode(z8000_state *cpustate)
{
  if (cpustate->device->type() == Z8001 && (cpustate->fcw & F_SEG))
    return 1;
  return 0;
}

INLINE UINT32 addr_add(z8000_state *cpustate, UINT32 addr, UINT32 addend)
{
    if (segmented_mode(cpustate))
        return (addr & 0xffff0000) | ((addr + addend) & 0xffff);
    else
        return (addr + addend) & 0xffff;
}

INLINE UINT32 addr_sub(z8000_state *cpustate, UINT32 addr, UINT32 subtrahend)
{
    if (segmented_mode(cpustate))
        return (addr & 0xffff0000) | ((addr - subtrahend) & 0xffff);
    else
        return (addr - subtrahend) & 0xffff;
}

/* conversion table for Z8000 DAB opcode */
#include "z8000dab.h"

INLINE UINT16 RDOP(z8000_state *cpustate)
{
	UINT16 res = cpustate->direct->read_decrypted_word(cpustate->pc);
    cpustate->pc += 2;
    return res;
}

INLINE UINT32 get_operand (z8000_state *cpustate, int opnum)
{
    int i;
    assert (cpustate->device->type() == Z8001 || cpustate->device->type() == Z8002);

    for (i = 0; i < opnum; i++)
        assert (cpustate->op_valid & (1 << i));

    if (! (cpustate->op_valid & (1 << opnum)))
    {
        cpustate->op[opnum] = cpustate->direct->read_decrypted_word(cpustate->pc);
        cpustate->pc += 2;
        cpustate->op_valid |= (1 << opnum);
    }
    return cpustate->op[opnum];
}

INLINE UINT32 get_addr_operand (z8000_state *cpustate, int opnum)
{
    int i;
    assert (cpustate->device->type() == Z8001 || cpustate->device->type() == Z8002);

    for (i = 0; i < opnum; i++)
        assert (cpustate->op_valid & (1 << i));

    if (! (cpustate->op_valid & (1 << opnum)))
    {
        UINT32 seg = cpustate->direct->read_decrypted_word(cpustate->pc);
        cpustate->pc += 2;
        if (segmented_mode(cpustate))
        {
            if (seg & 0x8000)
            {
                cpustate->op[opnum] = ((seg & 0x7f00) << 8) | cpustate->direct->read_decrypted_word(cpustate->pc);
                cpustate->pc += 2;
            }
            else
                cpustate->op[opnum] = ((seg & 0x7f00) << 8) | (seg & 0xff);
        }
        else
            cpustate->op[opnum] = seg;
        cpustate->op_valid |= (1 << opnum);
    }
    return cpustate->op[opnum];
}


INLINE UINT8 RDMEM_B(z8000_state *cpustate, UINT32 addr)
{
	return cpustate->program->read_byte(addr);
}

INLINE UINT16 RDMEM_W(z8000_state *cpustate, UINT32 addr)
{
	addr &= ~1;
    /* hack for m20 driver: BIOS accesses 0x7f0000 and expects a segmentation violation */
    if (addr >= 0x7f0000) {
        cpustate->irq_req = Z8000_SEGTRAP;
        return 0xffff;
    }
	return cpustate->program->read_word(addr);
}

INLINE UINT32 RDMEM_L(z8000_state *cpustate, UINT32 addr)
{
	UINT32 result;
	addr &= ~1;
	result = cpustate->program->read_word(addr) << 16;
	return result + cpustate->program->read_word(addr_add(cpustate, addr, 2));
}

INLINE void WRMEM_B(z8000_state *cpustate, UINT32 addr, UINT8 value)
{
	cpustate->program->write_byte(addr, value);
}

INLINE void WRMEM_W(z8000_state *cpustate, UINT32 addr, UINT16 value)
{
	addr &= ~1;
	cpustate->program->write_word(addr, value);
}

INLINE void WRMEM_L(z8000_state *cpustate, UINT32 addr, UINT32 value)
{
	addr &= ~1;
	cpustate->program->write_word(addr, value >> 16);
	cpustate->program->write_word(addr_add(cpustate, addr, 2), value & 0xffff);
}

INLINE UINT8 RDPORT_B(z8000_state *cpustate, int mode, UINT16 addr)
{
	if(mode == 0)
	{
		return cpustate->io->read_byte(addr);
	}
	else
	{
		/* how to handle MMU reads? */
		return 0x00;
	}
}

INLINE UINT16 RDPORT_W(z8000_state *cpustate, int mode, UINT16 addr)
{
	if(mode == 0)
	{
        if (cpustate->device->type() == Z8001)
        {
            return cpustate->io->read_word((UINT16)addr);
        }
        else
        {
            return cpustate->io->read_byte((UINT16)(addr)) +
                  (cpustate->io->read_byte((UINT16)(addr+1)) << 8);
        }
	}
	else
	{
		/* how to handle MMU reads? */
		return 0x0000;
	}
}

INLINE void WRPORT_B(z8000_state *cpustate, int mode, UINT16 addr, UINT8 value)
{
	if(mode == 0)
	{
        cpustate->io->write_byte(addr,value);
	}
	else
	{
		/* how to handle MMU writes? */
    }
}

INLINE void WRPORT_W(z8000_state *cpustate, int mode, UINT16 addr, UINT16 value)
{
	if(mode == 0)
	{
        if (cpustate->device->type() == Z8001)
        {
            cpustate->io->write_word((UINT16)addr, value);
        }
        else
        {
            cpustate->io->write_byte((UINT16)(addr),value & 0xff);
            cpustate->io->write_byte((UINT16)(addr+1),(value >> 8) & 0xff);
        }
	}
	else
	{
		/* how to handle MMU writes? */
    }
}

INLINE void cycles(z8000_state *cpustate, int cycles)
{
	cpustate->icount -= cycles;
}

#include "z8000ops.c"
#include "z8000tbl.c"

INLINE void set_irq(z8000_state *cpustate, int type)
{
    switch ((type >> 8) & 255)
    {
        case Z8000_EPU >> 8:
            if (cpustate->irq_srv >= Z8000_EPU)
                return;
            cpustate->irq_req = type;
            break;
        case Z8000_TRAP >> 8:
            if (cpustate->irq_srv >= Z8000_TRAP)
                return; /* double TRAP.. very bad :( */
            cpustate->irq_req = type;
            break;
        case Z8000_NMI >> 8:
            if (cpustate->irq_srv >= Z8000_NMI)
                return; /* no NMIs inside trap */
            cpustate->irq_req = type;
            break;
        case Z8000_SEGTRAP >> 8:
            if (cpustate->irq_srv >= Z8000_SEGTRAP)
                return; /* no SEGTRAPs inside NMI/TRAP */
            cpustate->irq_req = type;
            break;
        case Z8000_NVI >> 8:
            if (cpustate->irq_srv >= Z8000_NVI)
                return; /* no NVIs inside SEGTRAP/NMI/TRAP */
            cpustate->irq_req = type;
            break;
        case Z8000_VI >> 8:
            if (cpustate->irq_srv >= Z8000_VI)
                return; /* no VIs inside NVI/SEGTRAP/NMI/TRAP */
            cpustate->irq_req = type;
            break;
        case Z8000_SYSCALL >> 8:
            LOG(("Z8K '%s' SYSCALL $%02x\n", cpustate->device->tag(), type & 0xff));
            cpustate->irq_req = type;
            break;
        default:
            logerror("Z8000 invalid Cause_Interrupt %04x\n", type);
            return;
    }
    /* set interrupt request flag, reset HALT flag */
    cpustate->irq_req = type & ~Z8000_HALT;
}

#define PUSH_PC() do { \
if (cpustate->device->type() == Z8001) \
    PUSHL(cpustate, SP, make_segmented_addr(cpustate->pc));        /* save current cpustate->pc */ \
 else \
    PUSHW(cpustate, SP, cpustate->pc);        /* save current cpustate->pc */ \
} while (0)

#define GET_PC(VEC) (cpustate->device->type() == Z8001 ? segmented_addr(RDMEM_L(cpustate, VEC + 4)) : RDMEM_W(cpustate, VEC + 2))
#define GET_FCW(VEC) (cpustate->device->type() == Z8001 ? RDMEM_W(cpustate, VEC + 2) : RDMEM_W(cpustate, VEC))


INLINE void Interrupt(z8000_state *cpustate)
{
    UINT16 fcw = cpustate->fcw;

    if (cpustate->irq_req & Z8000_NVI)
    {
        int type = (*cpustate->irq_callback)(cpustate->device, 0);
        set_irq(cpustate, type | Z8000_NVI);
    }

    if (cpustate->irq_req & Z8000_VI)
    {
        int type = (*cpustate->irq_callback)(cpustate->device, 1);
        set_irq(cpustate, type | Z8000_VI);
    }

    /* trap ? */
    if (cpustate->irq_req & Z8000_EPU)
    {
        CHANGE_FCW(cpustate, fcw | F_S_N);/* swap to system stack */
        PUSH_PC();
        PUSHW(cpustate, SP, fcw);       /* save current cpustate->fcw */
        PUSHW(cpustate, SP, cpustate->irq_req);   /* save interrupt/trap type tag */
        cpustate->irq_srv = cpustate->irq_req;
        cpustate->irq_req &= ~Z8000_EPU;
        CHANGE_FCW(cpustate, GET_FCW(EPU));
        cpustate->pc = GET_PC(EPU);
        LOG(("Z8K '%s' ext instr trap $%04x\n", cpustate->device->tag(), cpustate->pc));
    }
    else
    if (cpustate->irq_req & Z8000_TRAP)
    {
        CHANGE_FCW(cpustate, fcw | F_S_N);/* swap to system stack */
        PUSH_PC();
        PUSHW(cpustate, SP, fcw);       /* save current cpustate->fcw */
        PUSHW(cpustate, SP, cpustate->irq_req);   /* save interrupt/trap type tag */
        cpustate->irq_srv = cpustate->irq_req;
        cpustate->irq_req &= ~Z8000_TRAP;
        CHANGE_FCW(cpustate, GET_FCW(TRAP));
        cpustate->pc = GET_PC(TRAP);
        LOG(("Z8K '%s' priv instr trap $%04x\n", cpustate->device->tag(), cpustate->pc));
    }
    else
    if (cpustate->irq_req & Z8000_SYSCALL)
    {
        CHANGE_FCW(cpustate, fcw | F_S_N);/* swap to system stack */
        PUSH_PC();
        PUSHW(cpustate, SP, fcw);       /* save current cpustate->fcw */
        PUSHW(cpustate, SP, cpustate->irq_req);   /* save interrupt/trap type tag */
        cpustate->irq_srv = cpustate->irq_req;
        cpustate->irq_req &= ~Z8000_SYSCALL;
        CHANGE_FCW(cpustate, GET_FCW(SYSCALL));
        cpustate->pc = GET_PC(SYSCALL);
        LOG(("Z8K '%s' syscall $%04x\n", cpustate->device->tag(), cpustate->pc));
    }
    else
    if (cpustate->irq_req & Z8000_SEGTRAP)
    {
        CHANGE_FCW(cpustate, fcw | F_S_N);/* swap to system stack */
        PUSH_PC();
        PUSHW(cpustate, SP, fcw);       /* save current cpustate->fcw */
        PUSHW(cpustate, SP, cpustate->irq_req);   /* save interrupt/trap type tag */
        cpustate->irq_srv = cpustate->irq_req;
        cpustate->irq_req &= ~Z8000_SEGTRAP;
        CHANGE_FCW(cpustate, GET_FCW(SEGTRAP));
        cpustate->pc = GET_PC(SEGTRAP);
        LOG(("Z8K '%s' segtrap $%04x\n", cpustate->device->tag(), cpustate->pc));
    }
    else
    if (cpustate->irq_req & Z8000_NMI)
    {
        CHANGE_FCW(cpustate, fcw | F_S_N);/* swap to system stack */
        PUSH_PC();
        PUSHW(cpustate, SP, fcw);       /* save current cpustate->fcw */
        PUSHW(cpustate, SP, cpustate->irq_req);   /* save interrupt/trap type tag */
        cpustate->irq_srv = cpustate->irq_req;
        cpustate->pc = RDMEM_W(cpustate,  NMI);
        cpustate->irq_req &= ~Z8000_NMI;
        CHANGE_FCW(cpustate, GET_FCW(NMI));
        cpustate->pc = GET_PC(NMI);
        LOG(("Z8K '%s' NMI $%04x\n", cpustate->device->tag(), cpustate->pc));
    }
    else
    if ((cpustate->irq_req & Z8000_NVI) && (cpustate->fcw & F_NVIE))
    {
        CHANGE_FCW(cpustate, fcw | F_S_N);/* swap to system stack */
        PUSH_PC();
        PUSHW(cpustate, SP, fcw);       /* save current cpustate->fcw */
        PUSHW(cpustate, SP, cpustate->irq_req);   /* save interrupt/trap type tag */
        cpustate->irq_srv = cpustate->irq_req;
        cpustate->pc = GET_PC(NVI); //RDMEM_W(cpustate,  NVI + 2);
        cpustate->irq_req &= ~Z8000_NVI;
        CHANGE_FCW(cpustate, GET_FCW(NVI));
        LOG(("Z8K '%s' NVI $%04x\n", cpustate->device->tag(), cpustate->pc));
    }
    else
    if ((cpustate->irq_req & Z8000_VI) && (cpustate->fcw & F_VIE))
    {
        CHANGE_FCW(cpustate, fcw | F_S_N);/* swap to system stack */
        PUSH_PC();
        PUSHW(cpustate, SP, fcw);       /* save current cpustate->fcw */
        PUSHW(cpustate, SP, cpustate->irq_req);   /* save interrupt/trap type tag */
        cpustate->irq_srv = cpustate->irq_req;
		if (cpustate->device->type() == Z8001)
			cpustate->pc =  segmented_addr(RDMEM_L(cpustate, VEC00 + 4 * (cpustate->irq_req & 0xff)));
		else
			cpustate->pc = RDMEM_W(cpustate, VEC00 + 2 * (cpustate->irq_req & 0xff));
        cpustate->irq_req &= ~Z8000_VI;
        CHANGE_FCW(cpustate, GET_FCW(VI));
        //printf ("z8k VI (vec 0x%x)\n", cpustate->irq_req & 0xff);
        LOG(("Z8K '%s' VI [$%04x/$%04x] fcw $%04x, pc $%04x\n", cpustate->device->tag(), cpustate->irq_vec, VEC00 + (cpustate->device->type() == Z8001 ? 4 : 2) * (cpustate->irq_req & 0xff), cpustate->fcw, cpustate->pc));
    }
}

static CPU_INIT( z8001 )
{
	z8000_state *cpustate = get_safe_token(device);

	cpustate->irq_callback = irqcallback;
	cpustate->device = device;
	cpustate->program = device->space(AS_PROGRAM);
	cpustate->direct = &cpustate->program->direct();
	cpustate->io = device->space(AS_IO);

	/* already initialized? */
	if(z8000_exec == NULL)
		z8000_init_tables();
}

static CPU_INIT( z8002 )
{
	z8000_state *cpustate = get_safe_token(device);

	cpustate->irq_callback = irqcallback;
	cpustate->device = device;
	cpustate->program = device->space(AS_PROGRAM);
	cpustate->direct = &cpustate->program->direct();
	cpustate->io = device->space(AS_IO);

	/* already initialized? */
	if(z8000_exec == NULL)
		z8000_init_tables();
}

static CPU_RESET( z8001 )
{
	z8000_state *cpustate = get_safe_token(device);

	device_irq_acknowledge_callback save_irqcallback = cpustate->irq_callback;
	memset(cpustate, 0, sizeof(*cpustate));
	cpustate->irq_callback = save_irqcallback;
	cpustate->device = device;
	cpustate->program = device->space(AS_PROGRAM);
	cpustate->direct = &cpustate->program->direct();
	cpustate->io = device->space(AS_IO);
	cpustate->fcw = RDMEM_W(cpustate,  2); /* get reset cpustate->fcw */
	if(cpustate->fcw & F_SEG)
	{
		cpustate->pc = ((RDMEM_W(cpustate,  4) & 0x0700) << 8) | (RDMEM_W(cpustate, 6) & 0xffff); /* get reset cpustate->pc  */
	}
	else
	{
		cpustate->pc = RDMEM_W(cpustate,  4); /* get reset cpustate->pc  */
	}
}

static CPU_RESET( z8002 )
{
	z8000_state *cpustate = get_safe_token(device);

	device_irq_acknowledge_callback save_irqcallback = cpustate->irq_callback;
	memset(cpustate, 0, sizeof(*cpustate));
	cpustate->irq_callback = save_irqcallback;
	cpustate->device = device;
	cpustate->program = device->space(AS_PROGRAM);
	cpustate->direct = &cpustate->program->direct();
	cpustate->io = device->space(AS_IO);
	cpustate->fcw = RDMEM_W(cpustate,  2); /* get reset cpustate->fcw */
	cpustate->pc = RDMEM_W(cpustate,  4); /* get reset cpustate->pc  */
}

static CPU_EXIT( z8000 )
{
	z8000_deinit_tables();
}

static CPU_EXECUTE( z8000 )
{
	z8000_state *cpustate = get_safe_token(device);

    do
    {
        /* any interrupt request pending? */
        if (cpustate->irq_req)
			Interrupt(cpustate);

		debugger_instruction_hook(device, cpustate->pc);

		if (cpustate->irq_req & Z8000_HALT)
        {
            cpustate->icount = 0;
        }
        else
        {
            Z8000_exec *exec;
            cpustate->op[0] = RDOP(cpustate);
            cpustate->op_valid = 1;
            exec = &z8000_exec[cpustate->op[0]];

            cpustate->icount -= exec->cycles;
            (*exec->opcode)(cpustate);
            cpustate->op_valid = 0;
        }
    } while (cpustate->icount > 0);

}

static void set_irq_line(z8000_state *cpustate, int irqline, int state)
{
	if (irqline == INPUT_LINE_NMI)
	{
		if (cpustate->nmi_state == state)
			return;

	    cpustate->nmi_state = state;

	    if (state != CLEAR_LINE)
		{
			if (cpustate->irq_srv >= Z8000_NMI)	/* no NMIs inside trap */
				return;
			cpustate->irq_req = Z8000_NMI;
			cpustate->irq_vec = NMI;
		}
	}
	else if (irqline < 2)
	{
		cpustate->irq_state[irqline] = state;
		if (irqline == 0)
		{
			if (state == CLEAR_LINE)
			{
				if (!(cpustate->fcw & F_NVIE))
					cpustate->irq_req &= ~Z8000_NVI;
			}
			else
			{
				if (cpustate->fcw & F_NVIE)
					cpustate->irq_req |= Z8000_NVI;
	        }
		}
		else
		{
			if (state == CLEAR_LINE)
			{
				if (!(cpustate->fcw & F_VIE))
					cpustate->irq_req &= ~Z8000_VI;
			}
			else
			{
				if (cpustate->fcw & F_VIE)
					cpustate->irq_req |= Z8000_VI;
			}
		}
	}
}



/**************************************************************************
 * Generic set_info
 **************************************************************************/

static CPU_SET_INFO( z8002 )
{
	z8000_state *cpustate = get_safe_token(device);

	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:	set_irq_line(cpustate, INPUT_LINE_NMI, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + 0:				set_irq_line(cpustate, 0, info->i);				break;
		case CPUINFO_INT_INPUT_STATE + 1:				set_irq_line(cpustate, 1, info->i);				break;

		case CPUINFO_INT_PC:							cpustate->pc = info->i; 							break;
		case CPUINFO_INT_REGISTER + Z8000_PC:			cpustate->pc = info->i;							break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + Z8000_NSP:			cpustate->nspoff = info->i;							break;
		case CPUINFO_INT_REGISTER + Z8000_FCW:			cpustate->fcw = info->i;							break;
		case CPUINFO_INT_REGISTER + Z8000_PSAP:			cpustate->psapoff = info->i;							break;
		case CPUINFO_INT_REGISTER + Z8000_REFRESH:		cpustate->refresh = info->i;						break;
		case CPUINFO_INT_REGISTER + Z8000_IRQ_REQ:		cpustate->irq_req = info->i;						break;
		case CPUINFO_INT_REGISTER + Z8000_IRQ_SRV:		cpustate->irq_srv = info->i;						break;
		case CPUINFO_INT_REGISTER + Z8000_IRQ_VEC:		cpustate->irq_vec = info->i;						break;
		case CPUINFO_INT_REGISTER + Z8000_R0:			cpustate->RW( 0) = info->i;						break;
		case CPUINFO_INT_REGISTER + Z8000_R1:			cpustate->RW( 1) = info->i;						break;
		case CPUINFO_INT_REGISTER + Z8000_R2:			cpustate->RW( 2) = info->i;						break;
		case CPUINFO_INT_REGISTER + Z8000_R3:			cpustate->RW( 3) = info->i;						break;
		case CPUINFO_INT_REGISTER + Z8000_R4:			cpustate->RW( 4) = info->i;						break;
		case CPUINFO_INT_REGISTER + Z8000_R5:			cpustate->RW( 5) = info->i;						break;
		case CPUINFO_INT_REGISTER + Z8000_R6:			cpustate->RW( 6) = info->i;						break;
		case CPUINFO_INT_REGISTER + Z8000_R7:			cpustate->RW( 7) = info->i;						break;
		case CPUINFO_INT_REGISTER + Z8000_R8:			cpustate->RW( 8) = info->i;						break;
		case CPUINFO_INT_REGISTER + Z8000_R9:			cpustate->RW( 9) = info->i;						break;
		case CPUINFO_INT_REGISTER + Z8000_R10:			cpustate->RW(10) = info->i;						break;
		case CPUINFO_INT_REGISTER + Z8000_R11:			cpustate->RW(11) = info->i;						break;
		case CPUINFO_INT_REGISTER + Z8000_R12:			cpustate->RW(12) = info->i;						break;
		case CPUINFO_INT_REGISTER + Z8000_R13:			cpustate->RW(13) = info->i;						break;
		case CPUINFO_INT_REGISTER + Z8000_R14:			cpustate->RW(14) = info->i;						break;
		case CPUINFO_INT_REGISTER + Z8000_R15:			cpustate->RW(15) = info->i;						break;
	}
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

CPU_GET_INFO( z8002 )
{
	z8000_state *cpustate = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(z8000_state);			break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 2;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0xff;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_BIG;				break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 2;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 6;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 2;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 744;							break;

		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM: info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM: info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_DATA:	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_IO:		info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_IO:		info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_IO:		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:	info->i = cpustate->nmi_state;					break;
		case CPUINFO_INT_INPUT_STATE + 0:				info->i = cpustate->irq_state[0];				break;  /* NVI */
		case CPUINFO_INT_INPUT_STATE + 1:				info->i = cpustate->irq_state[1];				break;  /* VI */

		case CPUINFO_INT_PREVIOUSPC:					info->i = cpustate->ppc;							break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + Z8000_PC:			info->i = cpustate->pc;							break;
		case CPUINFO_INT_SP:
        case CPUINFO_INT_REGISTER + Z8000_NSP:			info->i = cpustate->nspoff;							break;
        case CPUINFO_INT_REGISTER + Z8000_FCW:			info->i = cpustate->fcw;							break;
		case CPUINFO_INT_REGISTER + Z8000_PSAP:			info->i = cpustate->psapoff;							break;
		case CPUINFO_INT_REGISTER + Z8000_REFRESH:		info->i = cpustate->refresh;						break;
		case CPUINFO_INT_REGISTER + Z8000_IRQ_REQ:		info->i = cpustate->irq_req;						break;
		case CPUINFO_INT_REGISTER + Z8000_IRQ_SRV:		info->i = cpustate->irq_srv;						break;
		case CPUINFO_INT_REGISTER + Z8000_IRQ_VEC:		info->i = cpustate->irq_vec;						break;
		case CPUINFO_INT_REGISTER + Z8000_R0:			info->i = cpustate->RW( 0);						break;
		case CPUINFO_INT_REGISTER + Z8000_R1:			info->i = cpustate->RW( 1);						break;
		case CPUINFO_INT_REGISTER + Z8000_R2:			info->i = cpustate->RW( 2);						break;
		case CPUINFO_INT_REGISTER + Z8000_R3:			info->i = cpustate->RW( 3);						break;
		case CPUINFO_INT_REGISTER + Z8000_R4:			info->i = cpustate->RW( 4);						break;
		case CPUINFO_INT_REGISTER + Z8000_R5:			info->i = cpustate->RW( 5);						break;
		case CPUINFO_INT_REGISTER + Z8000_R6:			info->i = cpustate->RW( 6);						break;
		case CPUINFO_INT_REGISTER + Z8000_R7:			info->i = cpustate->RW( 7);						break;
		case CPUINFO_INT_REGISTER + Z8000_R8:			info->i = cpustate->RW( 8);						break;
		case CPUINFO_INT_REGISTER + Z8000_R9:			info->i = cpustate->RW( 9);						break;
		case CPUINFO_INT_REGISTER + Z8000_R10:			info->i = cpustate->RW(10);						break;
		case CPUINFO_INT_REGISTER + Z8000_R11:			info->i = cpustate->RW(11);						break;
		case CPUINFO_INT_REGISTER + Z8000_R12:			info->i = cpustate->RW(12);						break;
		case CPUINFO_INT_REGISTER + Z8000_R13:			info->i = cpustate->RW(13);						break;
		case CPUINFO_INT_REGISTER + Z8000_R14:			info->i = cpustate->RW(14);						break;
		case CPUINFO_INT_REGISTER + Z8000_R15:			info->i = cpustate->RW(15);						break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(z8002);		break;
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(z8002);				break;
		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(z8002);			break;
		case CPUINFO_FCT_EXIT:							info->exit = CPU_EXIT_NAME(z8000);				break;
		case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(z8000);		break;
		case CPUINFO_FCT_BURN:							info->burn = NULL;								break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(z8000);break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &cpustate->icount;				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "Z8002");						break;
		case CPUINFO_STR_FAMILY:					strcpy(info->s, "Zilog Z8000");					break;
		case CPUINFO_STR_VERSION:					strcpy(info->s, "1.1");							break;
		case CPUINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);						break;
		case CPUINFO_STR_CREDITS:					strcpy(info->s, "Copyright Juergen Buchmueller, all rights reserved."); break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
				cpustate->fcw & 0x8000 ? 's':'.',
				cpustate->fcw & 0x4000 ? 'n':'.',
				cpustate->fcw & 0x2000 ? 'e':'.',
				cpustate->fcw & 0x1000 ? '2':'.',
				cpustate->fcw & 0x0800 ? '1':'.',
				cpustate->fcw & 0x0400 ? '?':'.',
				cpustate->fcw & 0x0200 ? '?':'.',
				cpustate->fcw & 0x0100 ? '?':'.',
				cpustate->fcw & 0x0080 ? 'C':'.',
				cpustate->fcw & 0x0040 ? 'Z':'.',
				cpustate->fcw & 0x0020 ? 'S':'.',
				cpustate->fcw & 0x0010 ? 'V':'.',
				cpustate->fcw & 0x0008 ? 'D':'.',
				cpustate->fcw & 0x0004 ? 'H':'.',
				cpustate->fcw & 0x0002 ? '?':'.',
				cpustate->fcw & 0x0001 ? '?':'.');
            break;

		case CPUINFO_STR_REGISTER + Z8000_PC:			sprintf(info->s, "pc :%08X", cpustate->pc);		break;
		case CPUINFO_STR_REGISTER + Z8000_NSP:			sprintf(info->s, "SP :%04X", cpustate->nspoff);	break;
		case CPUINFO_STR_REGISTER + Z8000_FCW:			sprintf(info->s, "fcw:%04X", cpustate->fcw);	break;
		case CPUINFO_STR_REGISTER + Z8000_PSAP:			sprintf(info->s, "psapoff:%04X", cpustate->psapoff);	break;
		case CPUINFO_STR_REGISTER + Z8000_REFRESH:		sprintf(info->s, "REFR:%04X", cpustate->refresh); break;
		case CPUINFO_STR_REGISTER + Z8000_IRQ_REQ:		sprintf(info->s, "IRQR:%04X", cpustate->irq_req); break;
		case CPUINFO_STR_REGISTER + Z8000_IRQ_SRV:		sprintf(info->s, "IRQS:%04X", cpustate->irq_srv); break;
		case CPUINFO_STR_REGISTER + Z8000_IRQ_VEC:		sprintf(info->s, "IRQV:%04X", cpustate->irq_vec); break;
		case CPUINFO_STR_REGISTER + Z8000_R0:			sprintf(info->s, "R0 :%04X", cpustate->RW(0)); break;
		case CPUINFO_STR_REGISTER + Z8000_R1:			sprintf(info->s, "R1 :%04X", cpustate->RW(1)); break;
		case CPUINFO_STR_REGISTER + Z8000_R2:			sprintf(info->s, "R2 :%04X", cpustate->RW(2)); break;
		case CPUINFO_STR_REGISTER + Z8000_R3:			sprintf(info->s, "R3 :%04X", cpustate->RW(3)); break;
		case CPUINFO_STR_REGISTER + Z8000_R4:			sprintf(info->s, "R4 :%04X", cpustate->RW(4)); break;
		case CPUINFO_STR_REGISTER + Z8000_R5:			sprintf(info->s, "R5 :%04X", cpustate->RW(5)); break;
		case CPUINFO_STR_REGISTER + Z8000_R6:			sprintf(info->s, "R6 :%04X", cpustate->RW(6)); break;
		case CPUINFO_STR_REGISTER + Z8000_R7:			sprintf(info->s, "R7 :%04X", cpustate->RW(7)); break;
		case CPUINFO_STR_REGISTER + Z8000_R8:			sprintf(info->s, "R8 :%04X", cpustate->RW(8)); break;
		case CPUINFO_STR_REGISTER + Z8000_R9:			sprintf(info->s, "R9 :%04X", cpustate->RW(9)); break;
		case CPUINFO_STR_REGISTER + Z8000_R10:			sprintf(info->s, "R10:%04X", cpustate->RW(10)); break;
		case CPUINFO_STR_REGISTER + Z8000_R11:			sprintf(info->s, "R11:%04X", cpustate->RW(11)); break;
		case CPUINFO_STR_REGISTER + Z8000_R12:			sprintf(info->s, "R12:%04X", cpustate->RW(12)); break;
		case CPUINFO_STR_REGISTER + Z8000_R13:			sprintf(info->s, "R13:%04X", cpustate->RW(13)); break;
		case CPUINFO_STR_REGISTER + Z8000_R14:			sprintf(info->s, "R14:%04X", cpustate->RW(14)); break;
		case CPUINFO_STR_REGISTER + Z8000_R15:			sprintf(info->s, "R15:%04X", cpustate->RW(15)); break;
	}
}

/*
handling for the z8001
*/

CPU_GET_INFO( z8001 )
{
	switch (state)
	{
		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM: info->i = 20;					break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_IO:		info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_IO:		info->i = 16;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(z8001);		break;
		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(z8001);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "Zilog Z8001");			break;

		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 2;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 8;							break;

		default:										CPU_GET_INFO_CALL(z8002);				break;
	}
}

DEFINE_LEGACY_CPU_DEVICE(Z8001, z8001);
DEFINE_LEGACY_CPU_DEVICE(Z8002, z8002);
