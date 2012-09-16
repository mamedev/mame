/***************************************************************************

    am29000.c
    Core implementation of the Am29000 emulator

    Written by Philip Bennett

    Features missing:
    * MMU
    * Some instructions
    * Various exceptions

***************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "am29000.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define MAX_EXCEPTIONS				(4)

#define PFLAG_FETCH_EN				(1 << 0)
#define PFLAG_DECODE_EN				(1 << 1)
#define PFLAG_EXECUTE_EN			(1 << 2)
#define PFLAG_WRITEBACK_EN			(1 << 3)
#define PFLAG_IRQ					(1 << 4)
#define PFLAG_LOADSTORE				(1 << 5)
#define PFLAG_MULTI_LOADSTORE		(1 << 6)
#define PFLAG_JUMP					(1 << 7)
#define PFLAG_JUMP2					(1 << 8)
#define PFLAG_IRET					(1 << 9)
#define PFLAG_TIMER_LOADED			(1 << 10)

#define PFLAG_RA_DEPENDENCY			(1 << 26)
#define PFLAG_RB_DEPENDENCY			(1 << 27)

#define PFLAG_MEM_MULTIPLE			(1 << 29)
#define PFLAG_REG_WRITEBACK			(1 << 30)
#define PFLAG_MEM_WRITEBACK			(1 << 31)

#define MMU_PROGRAM_ACCESS			(0)
#define MMU_DATA_ACCESS				(1)

#define FREEZE_MODE					(am29000->cps & CPS_FZ)
#define SUPERVISOR_MODE				(am29000->cps & CPS_SM)
#define USER_MODE					(~am29000->cps & CPS_SM)
#define REGISTER_IS_PROTECTED(x)	(am29000->rbp & (1 << ((x) >> 4)))

#define INST_RB_FIELD(x)			((x) & 0xff)
#define INST_RA_FIELD(x)			(((x) >> 8) & 0xff)
#define INST_RC_FIELD(x)			(((x) >> 16) & 0xff)
#define INST_SA_FIELD(x)			(((x) >> 8) & 0xff)

#define FIELD_RA					0
#define FIELD_RB					1
#define FIELD_RC					2

#define SIGNAL_EXCEPTION(x)			(signal_exception(am29000, x))


#define GET_ALU_FC					((am29000->alu >> ALU_FC_SHIFT) & ALU_FC_MASK)
#define GET_ALU_BP					((am29000->alu >> ALU_BP_SHIFT) & ALU_BP_MASK)
#define GET_CHC_CR					((am29000->chc >> CHC_CR_SHIFT) & CHC_CR_MASK)

#define SET_ALU_FC(x)				do { am29000->alu &= ~(ALU_FC_MASK << ALU_FC_SHIFT); am29000->alu |= ((x) & ALU_FC_MASK) << ALU_FC_SHIFT; } while(0)
#define SET_ALU_BP(x)				do { am29000->alu &= ~(ALU_BP_MASK << ALU_BP_SHIFT); am29000->alu |= ((x) & ALU_BP_MASK) << ALU_BP_SHIFT; } while(0)
#define SET_CHC_CR(x)				do { am29000->chc &= ~(CHC_CR_MASK << CHC_CR_SHIFT); am29000->chc |= ((x) & CHC_CR_MASK) << CHC_CR_SHIFT; } while(0)


/***************************************************************************
    STRUCTURES & TYPEDEFS
***************************************************************************/

struct am29000_state 
{
	INT32			icount;
	UINT32			pc;

	/* General purpose */
	UINT32			r[256];		// TODO: There's only 192 implemented!

	/* TLB */
	UINT32			tlb[128];

	/* Protected SPRs */
	UINT32			vab;
	UINT32			ops;
	UINT32			cps;
	UINT32			cfg;
	UINT32			cha;
	UINT32			chd;
	UINT32			chc;
	UINT32			rbp;
	UINT32			tmc;
	UINT32			tmr;
	UINT32			pc0;
	UINT32			pc1;
	UINT32			pc2;
	UINT32			mmu;
	UINT32			lru;

	/* Unprotected SPRs */
	UINT32			ipc;
	UINT32			ipa;
	UINT32			ipb;
	UINT32			q;
	UINT32			alu;
	UINT32			fpe;
	UINT32			inte;
	UINT32			fps;

	/* Pipeline state */
	UINT32			exceptions;
	UINT32			exception_queue[MAX_EXCEPTIONS];

	UINT8			irq_active;
	UINT8			irq_lines;

	UINT32			exec_ir;
	UINT32			next_ir;

	UINT32			pl_flags;
	UINT32			next_pl_flags;

	UINT32			iret_pc;

	UINT32			exec_pc;
	UINT32			next_pc;

	address_space *program;
	direct_read_data *direct;
	address_space *data;
	direct_read_data *datadirect;
	address_space *io;
};


/***************************************************************************
    STATE ACCESSORS
***************************************************************************/

INLINE am29000_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == AM29000);
	return (am29000_state *)downcast<legacy_cpu_device *>(device)->token();
}

static CPU_INIT( am29000 )
{
	am29000_state *am29000 = get_safe_token(device);

	am29000->program = device->space(AS_PROGRAM);
	am29000->direct = &am29000->program->direct();
	am29000->data = device->space(AS_DATA);
	am29000->datadirect = &am29000->data->direct();
	am29000->io = device->space(AS_IO);
	am29000->cfg = (PRL_AM29000 | PRL_REV_D) << CFG_PRL_SHIFT;

	/* Register state for saving */
	device->save_item(NAME(am29000->icount));
	device->save_item(NAME(am29000->pc));
	device->save_item(NAME(am29000->r));
	device->save_item(NAME(am29000->tlb));

	device->save_item(NAME(am29000->vab));
	device->save_item(NAME(am29000->ops));
	device->save_item(NAME(am29000->cps));
	device->save_item(NAME(am29000->cfg));
	device->save_item(NAME(am29000->cha));
	device->save_item(NAME(am29000->chd));
	device->save_item(NAME(am29000->chc));
	device->save_item(NAME(am29000->rbp));
	device->save_item(NAME(am29000->tmc));
	device->save_item(NAME(am29000->tmr));
	device->save_item(NAME(am29000->pc0));
	device->save_item(NAME(am29000->pc1));
	device->save_item(NAME(am29000->pc2));
	device->save_item(NAME(am29000->mmu));
	device->save_item(NAME(am29000->lru));

	device->save_item(NAME(am29000->ipc));
	device->save_item(NAME(am29000->ipa));
	device->save_item(NAME(am29000->ipb));
	device->save_item(NAME(am29000->q));

	device->save_item(NAME(am29000->alu));
	device->save_item(NAME(am29000->fpe));
	device->save_item(NAME(am29000->inte));
	device->save_item(NAME(am29000->fps));

	device->save_item(NAME(am29000->exceptions));
	device->save_item(NAME(am29000->exception_queue));

	device->save_item(NAME(am29000->irq_active));
	device->save_item(NAME(am29000->irq_lines));

	device->save_item(NAME(am29000->exec_ir));
	device->save_item(NAME(am29000->next_ir));

	device->save_item(NAME(am29000->pl_flags));
	device->save_item(NAME(am29000->next_pl_flags));

	device->save_item(NAME(am29000->iret_pc));
	device->save_item(NAME(am29000->exec_pc));
	device->save_item(NAME(am29000->next_pc));
}

static CPU_RESET( am29000 )
{
	am29000_state *am29000 = get_safe_token(device);

	am29000->cps = CPS_FZ | CPS_RE | CPS_PD | CPS_PI | CPS_SM | CPS_DI | CPS_DA;
	am29000->cfg &= ~(CFG_DW | CFG_CD);
	am29000->chc &= ~CHC_CV;

	am29000->pc = 0;
	am29000->next_pl_flags = 0;
	am29000->exceptions = 0;
	am29000->irq_lines = 0;
}


static CPU_EXIT( am29000 )
{

}


static void signal_exception(am29000_state *am29000, UINT32 type)
{
	am29000->exception_queue[am29000->exceptions++] = type;
}

static void external_irq_check(am29000_state *am29000)
{
	int mask = (am29000->cps >> CPS_IM_SHIFT) & CPS_IM_MASK;
	int irq_en = !(am29000->cps & CPS_DI) && !(am29000->cps & CPS_DA);
	int i;

	/* Clear interrupt pending bit to begin with */
	am29000->cps &= ~CPS_IP;

	for (i = 0; i < 4; ++i)
	{
		if (!(am29000->irq_active & (1 << i)) && (am29000->irq_lines & (1 << i)))
		{
			if (irq_en)
			{
				if (i <= mask)
				{
					am29000->irq_active |= (1 << i);
					SIGNAL_EXCEPTION(EXCEPTION_INTR0 + i);
					am29000->pl_flags |= PFLAG_IRQ;
					return;
				}
			}
			/* Set interrupt pending bit if interrupt was disabled */
			am29000->cps |= CPS_IP;
		}
		else
			am29000->irq_active &= ~(1 << i);
	}
}

static UINT32 read_program_word(am29000_state *state, UINT32 address)
{
	/* TODO: ROM enable? */
	if (state->cps & CPS_PI || state->cps & CPS_RE)
		return state->direct->read_decrypted_dword(address);
	else
	{
		fatalerror("Am29000 instruction MMU translation enabled!\n");
	}
	return 0;
}

/***************************************************************************
    HELPER FUNCTIONS
***************************************************************************/

INLINE UINT32 get_abs_reg(am29000_state *am29000, UINT8 r, UINT32 iptr)
{
	if (r & 0x80)
	{
		/* Stack pointer access */
		r = ((am29000->r[1] >> 2) & 0x7f) + (r & 0x7f);
		r |= 0x80;
	}
	else if (r == 0)
	{
		/* Indirect pointer access */
		r = (iptr >> IPX_SHIFT) & 0xff;
	}
	else if (r > 1 && r < 64)
	{
		fatalerror("Am29000: Undefined register access (%d)\n", r);
	}
	return r;
}


/***************************************************************************
    CORE INCLUDE
***************************************************************************/

#include "am29ops.h"


/***************************************************************************
    PIPELINE STAGES
***************************************************************************/

INLINE void fetch_decode(am29000_state *am29000)
{
	UINT32 inst;
	op_info op;

	inst = read_program_word(am29000, am29000->pc);
	am29000->next_ir = inst;

	op = op_table[inst >> 24];

	/* Illegal instruction */
	/* TODO: This should be checked at this point */
#if 0
	if (op.flags & IFLAG_ILLEGAL)
	{
		fatalerror("Illegal instruction: %x PC:%x PC0:%x PC1:%x\n", inst, am29000->pc, am29000->pc0, am29000->pc1);
		SIGNAL_EXCEPTION(EXCEPTION_ILLEGAL_OPCODE);
		return;
	}
#endif

	/* Privledge violations */
	if (USER_MODE)
	{
		if ((op.flags & IFLAG_SUPERVISOR_ONLY))
		{
			SIGNAL_EXCEPTION(EXCEPTION_PROTECTION_VIOLATION);
			return;
		}

		if ((op.flags & IFLAG_SPR_ACCESS))
		{
			/* TODO: Is this the right place to check this? */
			if (INST_SA_FIELD(inst) < 128)
			{
				SIGNAL_EXCEPTION(EXCEPTION_PROTECTION_VIOLATION);
				return;
			}
		}

		/* Register bank protection */
		if ((op.flags & IFLAG_RA_PRESENT) && REGISTER_IS_PROTECTED(INST_RA_FIELD(inst)))
		{
			SIGNAL_EXCEPTION(EXCEPTION_PROTECTION_VIOLATION);
			return;
		}

		if ((op.flags & IFLAG_RB_PRESENT) && REGISTER_IS_PROTECTED(INST_RB_FIELD(inst)))
		{
			SIGNAL_EXCEPTION(EXCEPTION_PROTECTION_VIOLATION);
			return;
		}

		if ((op.flags & IFLAG_RC_PRESENT) && REGISTER_IS_PROTECTED(INST_RC_FIELD(inst)))
		{
			SIGNAL_EXCEPTION(EXCEPTION_PROTECTION_VIOLATION);
			return;
		}
	}

	if (am29000->pl_flags & PFLAG_IRET)
		am29000->next_pc = am29000->iret_pc;
	else
		am29000->next_pc += 4;
}

/***************************************************************************
    CORE EXECUTION LOOP
***************************************************************************/

static CPU_EXECUTE( am29000 )
{
	am29000_state *am29000 = get_safe_token(device);
	UINT32 call_debugger = (device->machine().debug_flags & DEBUG_FLAG_ENABLED) != 0;

	external_irq_check(am29000);

	do
	{
		am29000->next_pl_flags = PFLAG_EXECUTE_EN;

		if (!FREEZE_MODE)
		{
			am29000->pc1 = am29000->pc0;
			am29000->pc0 = am29000->pc;
		}

		if (am29000->exceptions)
		{
			am29000->ops = am29000->cps;
			am29000->cps &= ~(CPS_TE | CPS_TP | CPS_TU | CPS_FZ | CPS_LK | CPS_WM | CPS_PD | CPS_PI | CPS_SM | CPS_DI | CPS_DA);
			am29000->cps |= (CPS_FZ | CPS_PD | CPS_PI | CPS_SM | CPS_DI | CPS_DA);

			if (am29000->pl_flags & PFLAG_IRET)
			{
				am29000->pc0 = am29000->iret_pc;
				am29000->pc1 = am29000->next_pc;
			}


			if (am29000->cfg & CFG_VF)
			{
				UINT32 vaddr = am29000->vab | am29000->exception_queue[0] * 4;
				UINT32 vect = am29000->datadirect->read_decrypted_dword(vaddr);

				am29000->pc = vect & ~3;
				am29000->next_pc = am29000->pc;
			}
			else
			{
				fatalerror("Am29000: Non vectored interrupt fetch!\n");
			}

			am29000->exceptions = 0;
			am29000->pl_flags = 0;
		}

		if (call_debugger)
			debugger_instruction_hook(device, am29000->pc);

		fetch_decode(am29000);

		if (am29000->pl_flags & PFLAG_EXECUTE_EN)
		{
			if (!FREEZE_MODE)
				am29000->pc2 = am29000->pc1;

			op_table[am29000->exec_ir >> 24].opcode(am29000);
		}

		am29000->exec_ir = am29000->next_ir;
		am29000->pl_flags = am29000->next_pl_flags;
		am29000->exec_pc = am29000->pc;
		am29000->pc = am29000->next_pc;
	} while (--am29000->icount > 0);
}

static void set_irq_line(am29000_state *am29000, int line, int state)
{
	if (state)
		am29000->irq_lines |= (1 << line);
	else
		am29000->irq_lines &= ~(1 << line);

	// TODO : CHECK IRQs
}

/***************************************************************************
    DISASSEMBLY HOOK
***************************************************************************/

extern CPU_DISASSEMBLE( am29000 );


static CPU_SET_INFO( am29000 )
{
	am29000_state *am29000 = get_safe_token(device);

	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + AM29000_INTR0:	set_irq_line(am29000, AM29000_INTR0, info->i);		break;
		case CPUINFO_INT_INPUT_STATE + AM29000_INTR1:	set_irq_line(am29000, AM29000_INTR1, info->i);		break;
		case CPUINFO_INT_INPUT_STATE + AM29000_INTR2:	set_irq_line(am29000, AM29000_INTR2, info->i);		break;
		case CPUINFO_INT_INPUT_STATE + AM29000_INTR3:	set_irq_line(am29000, AM29000_INTR3, info->i);		break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + AM29000_PC:		am29000->pc = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_VAB:	am29000->vab = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_OPS:	am29000->ops = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_CPS:	am29000->cps = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_CFG:	am29000->cfg = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_CHA:	am29000->cha = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_CHD:	am29000->chd = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_CHC:	am29000->chc = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_RBP:	am29000->rbp = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_TMC:	am29000->tmc = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_TMR:	am29000->tmr = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_PC0:	am29000->pc0 = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_PC1:	am29000->pc1 = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_PC2:	am29000->pc2 = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_MMU:	am29000->mmu = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_LRU:	am29000->lru = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_IPC:	am29000->ipc = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_IPA:	am29000->ipa = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_IPB:	am29000->ipb = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_Q:		am29000->q = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_ALU:	am29000->alu = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_BP:		SET_ALU_BP(info->i);							break;
		case CPUINFO_INT_REGISTER + AM29000_FC:		SET_ALU_FC(info->i);							break;
		case CPUINFO_INT_REGISTER + AM29000_CR:		SET_CHC_CR(info->i);							break;
		case CPUINFO_INT_REGISTER + AM29000_FPE:	am29000->fpe = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_INTE:	am29000->inte = info->i;						break;
		case CPUINFO_INT_REGISTER + AM29000_FPS:	am29000->fps = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R1:		am29000->r[1] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R64:	am29000->r[64] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R65:	am29000->r[65] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R66:	am29000->r[66] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R67:	am29000->r[67] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R68:	am29000->r[68] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R69:	am29000->r[69] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R70:	am29000->r[70] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R71:	am29000->r[71] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R72:	am29000->r[72] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R73:	am29000->r[73] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R74:	am29000->r[74] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R75:	am29000->r[75] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R76:	am29000->r[76] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R77:	am29000->r[77] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R78:	am29000->r[78] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R79:	am29000->r[79] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R80:	am29000->r[80] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R81:	am29000->r[81] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R82:	am29000->r[82] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R83:	am29000->r[83] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R84:	am29000->r[84] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R85:	am29000->r[85] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R86:	am29000->r[86] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R87:	am29000->r[87] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R88:	am29000->r[88] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R89:	am29000->r[89] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R90:	am29000->r[90] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R91:	am29000->r[91] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R92:	am29000->r[92] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R93:	am29000->r[93] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R94:	am29000->r[94] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R95:	am29000->r[95] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R96:	am29000->r[96] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R97:	am29000->r[97] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R98:	am29000->r[98] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R99:	am29000->r[99] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R100:	am29000->r[100] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R101:	am29000->r[101] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R102:	am29000->r[102] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R103:	am29000->r[103] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R104:	am29000->r[104] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R105:	am29000->r[105] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R106:	am29000->r[106] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R107:	am29000->r[107] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R108:	am29000->r[108] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R109:	am29000->r[109] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R110:	am29000->r[110] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R111:	am29000->r[111] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R112:	am29000->r[112] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R113:	am29000->r[113] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R114:	am29000->r[114] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R115:	am29000->r[115] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R116:	am29000->r[116] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R117:	am29000->r[117] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R118:	am29000->r[118] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R119:	am29000->r[119] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R120:	am29000->r[120] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R121:	am29000->r[121] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R122:	am29000->r[122] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R123:	am29000->r[123] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R124:	am29000->r[124] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R125:	am29000->r[125] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R126:	am29000->r[126] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R127:	am29000->r[127] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R128:	am29000->r[128] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R129:	am29000->r[129] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R130:	am29000->r[130] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R131:	am29000->r[131] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R132:	am29000->r[132] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R133:	am29000->r[133] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R134:	am29000->r[134] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R135:	am29000->r[135] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R136:	am29000->r[136] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R137:	am29000->r[137] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R138:	am29000->r[138] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R139:	am29000->r[139] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R140:	am29000->r[140] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R141:	am29000->r[141] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R142:	am29000->r[142] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R143:	am29000->r[143] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R144:	am29000->r[144] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R145:	am29000->r[145] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R146:	am29000->r[146] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R147:	am29000->r[147] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R148:	am29000->r[148] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R149:	am29000->r[149] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R150:	am29000->r[150] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R151:	am29000->r[151] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R152:	am29000->r[152] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R153:	am29000->r[153] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R154:	am29000->r[154] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R155:	am29000->r[155] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R156:	am29000->r[156] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R157:	am29000->r[157] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R158:	am29000->r[158] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R159:	am29000->r[159] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R160:	am29000->r[160] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R161:	am29000->r[161] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R162:	am29000->r[162] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R163:	am29000->r[163] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R164:	am29000->r[164] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R165:	am29000->r[165] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R166:	am29000->r[166] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R167:	am29000->r[167] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R168:	am29000->r[168] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R169:	am29000->r[169] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R170:	am29000->r[170] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R171:	am29000->r[171] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R172:	am29000->r[172] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R173:	am29000->r[173] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R174:	am29000->r[174] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R175:	am29000->r[175] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R176:	am29000->r[176] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R177:	am29000->r[177] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R178:	am29000->r[178] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R179:	am29000->r[179] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R180:	am29000->r[180] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R181:	am29000->r[181] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R182:	am29000->r[182] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R183:	am29000->r[183] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R184:	am29000->r[184] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R185:	am29000->r[185] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R186:	am29000->r[186] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R187:	am29000->r[187] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R188:	am29000->r[188] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R189:	am29000->r[189] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R190:	am29000->r[190] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R191:	am29000->r[191] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R192:	am29000->r[192] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R193:	am29000->r[193] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R194:	am29000->r[194] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R195:	am29000->r[195] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R196:	am29000->r[196] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R197:	am29000->r[197] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R198:	am29000->r[198] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R199:	am29000->r[199] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R200:	am29000->r[200] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R201:	am29000->r[201] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R202:	am29000->r[202] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R203:	am29000->r[203] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R204:	am29000->r[204] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R205:	am29000->r[205] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R206:	am29000->r[206] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R207:	am29000->r[207] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R208:	am29000->r[208] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R209:	am29000->r[209] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R210:	am29000->r[210] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R211:	am29000->r[211] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R212:	am29000->r[212] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R213:	am29000->r[213] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R214:	am29000->r[214] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R215:	am29000->r[215] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R216:	am29000->r[216] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R217:	am29000->r[217] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R218:	am29000->r[218] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R219:	am29000->r[219] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R220:	am29000->r[220] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R221:	am29000->r[221] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R222:	am29000->r[222] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R223:	am29000->r[223] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R224:	am29000->r[224] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R225:	am29000->r[225] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R226:	am29000->r[226] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R227:	am29000->r[227] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R228:	am29000->r[228] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R229:	am29000->r[229] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R230:	am29000->r[230] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R231:	am29000->r[231] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R232:	am29000->r[232] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R233:	am29000->r[233] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R234:	am29000->r[234] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R235:	am29000->r[235] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R236:	am29000->r[236] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R237:	am29000->r[237] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R238:	am29000->r[238] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R239:	am29000->r[239] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R240:	am29000->r[240] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R241:	am29000->r[241] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R242:	am29000->r[242] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R243:	am29000->r[243] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R244:	am29000->r[244] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R245:	am29000->r[245] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R246:	am29000->r[246] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R247:	am29000->r[247] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R248:	am29000->r[248] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R249:	am29000->r[249] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R250:	am29000->r[250] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R251:	am29000->r[251] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R252:	am29000->r[252] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R253:	am29000->r[253] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R254:	am29000->r[254] = info->i;							break;
		case CPUINFO_INT_REGISTER + AM29000_R255:	am29000->r[255] = info->i;							break;
	}
}


/**************************************************************************
 * Generic get_info
 **************************************************************************/

CPU_GET_INFO( am29000 )
{
	am29000_state *am29000 = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(am29000_state);		break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 1;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_BIG;				break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 4;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 4;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 2;							break;

		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:			info->i = 32;							break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM:			info->i = 32;							break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM:			info->i = 0;							break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_DATA:			info->i = 32;							break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:			info->i = 32;							break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_DATA:			info->i = 0;							break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_IO:				info->i = 32;							break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_IO:				info->i = 32;							break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_IO:				info->i = 0;							break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + AM29000_PC:		info->i = am29000->pc;				break;
		case CPUINFO_INT_REGISTER + AM29000_VAB:	info->i = am29000->vab;				break;
		case CPUINFO_INT_REGISTER + AM29000_OPS:	info->i = am29000->ops;				break;
		case CPUINFO_INT_REGISTER + AM29000_CPS:	info->i = am29000->cps;				break;
		case CPUINFO_INT_REGISTER + AM29000_CFG:	info->i = am29000->cfg;				break;
		case CPUINFO_INT_REGISTER + AM29000_CHA:	info->i = am29000->cha;				break;
		case CPUINFO_INT_REGISTER + AM29000_CHD:	info->i = am29000->chd;				break;
		case CPUINFO_INT_REGISTER + AM29000_CHC:	info->i = am29000->chc;				break;
		case CPUINFO_INT_REGISTER + AM29000_RBP:	info->i = am29000->rbp;				break;
		case CPUINFO_INT_REGISTER + AM29000_TMC:	info->i = am29000->tmc;				break;
		case CPUINFO_INT_REGISTER + AM29000_TMR:	info->i = am29000->tmr;				break;
		case CPUINFO_INT_REGISTER + AM29000_PC0:	info->i = am29000->pc0;				break;
		case CPUINFO_INT_REGISTER + AM29000_PC1:	info->i = am29000->pc1;				break;
		case CPUINFO_INT_REGISTER + AM29000_PC2:	info->i = am29000->pc2;				break;
		case CPUINFO_INT_REGISTER + AM29000_MMU:	info->i = am29000->mmu;				break;
		case CPUINFO_INT_REGISTER + AM29000_LRU:	info->i = am29000->lru;				break;
		case CPUINFO_INT_REGISTER + AM29000_IPC:	info->i = am29000->ipc;				break;
		case CPUINFO_INT_REGISTER + AM29000_IPA:	info->i = am29000->ipa;				break;
		case CPUINFO_INT_REGISTER + AM29000_IPB:	info->i = am29000->ipb;				break;
		case CPUINFO_INT_REGISTER + AM29000_Q:		info->i = am29000->q;				break;
		case CPUINFO_INT_REGISTER + AM29000_ALU:	info->i = am29000->alu;				break;
		case CPUINFO_INT_REGISTER + AM29000_BP:		info->i = GET_ALU_BP;				break;
		case CPUINFO_INT_REGISTER + AM29000_FC:		info->i = GET_ALU_FC;				break;
		case CPUINFO_INT_REGISTER + AM29000_CR:		info->i = GET_CHC_CR;				break;
		case CPUINFO_INT_REGISTER + AM29000_FPE:	info->i = am29000->fpe;				break;
		case CPUINFO_INT_REGISTER + AM29000_INTE:	info->i = am29000->inte;			break;
		case CPUINFO_INT_REGISTER + AM29000_FPS:	info->i = am29000->fps;				break;
		case CPUINFO_INT_REGISTER + AM29000_R1:		info->i = am29000->r[1];			break;
		case CPUINFO_INT_REGISTER + AM29000_R64:	info->i = am29000->r[64];			break;
		case CPUINFO_INT_REGISTER + AM29000_R65:	info->i = am29000->r[65];			break;
		case CPUINFO_INT_REGISTER + AM29000_R66:	info->i = am29000->r[66];			break;
		case CPUINFO_INT_REGISTER + AM29000_R67:	info->i = am29000->r[67];			break;
		case CPUINFO_INT_REGISTER + AM29000_R68:	info->i = am29000->r[68];			break;
		case CPUINFO_INT_REGISTER + AM29000_R69:	info->i = am29000->r[69];			break;
		case CPUINFO_INT_REGISTER + AM29000_R70:	info->i = am29000->r[70];			break;
		case CPUINFO_INT_REGISTER + AM29000_R71:	info->i = am29000->r[71];			break;
		case CPUINFO_INT_REGISTER + AM29000_R72:	info->i = am29000->r[72];			break;
		case CPUINFO_INT_REGISTER + AM29000_R73:	info->i = am29000->r[73];			break;
		case CPUINFO_INT_REGISTER + AM29000_R74:	info->i = am29000->r[74];			break;
		case CPUINFO_INT_REGISTER + AM29000_R75:	info->i = am29000->r[75];			break;
		case CPUINFO_INT_REGISTER + AM29000_R76:	info->i = am29000->r[76];			break;
		case CPUINFO_INT_REGISTER + AM29000_R77:	info->i = am29000->r[77];			break;
		case CPUINFO_INT_REGISTER + AM29000_R78:	info->i = am29000->r[78];			break;
		case CPUINFO_INT_REGISTER + AM29000_R79:	info->i = am29000->r[79];			break;
		case CPUINFO_INT_REGISTER + AM29000_R80:	info->i = am29000->r[80];			break;
		case CPUINFO_INT_REGISTER + AM29000_R81:	info->i = am29000->r[81];			break;
		case CPUINFO_INT_REGISTER + AM29000_R82:	info->i = am29000->r[82];			break;
		case CPUINFO_INT_REGISTER + AM29000_R83:	info->i = am29000->r[83];			break;
		case CPUINFO_INT_REGISTER + AM29000_R84:	info->i = am29000->r[84];			break;
		case CPUINFO_INT_REGISTER + AM29000_R85:	info->i = am29000->r[85];			break;
		case CPUINFO_INT_REGISTER + AM29000_R86:	info->i = am29000->r[86];			break;
		case CPUINFO_INT_REGISTER + AM29000_R87:	info->i = am29000->r[87];			break;
		case CPUINFO_INT_REGISTER + AM29000_R88:	info->i = am29000->r[88];			break;
		case CPUINFO_INT_REGISTER + AM29000_R89:	info->i = am29000->r[89];			break;
		case CPUINFO_INT_REGISTER + AM29000_R90:	info->i = am29000->r[90];			break;
		case CPUINFO_INT_REGISTER + AM29000_R91:	info->i = am29000->r[91];			break;
		case CPUINFO_INT_REGISTER + AM29000_R92:	info->i = am29000->r[92];			break;
		case CPUINFO_INT_REGISTER + AM29000_R93:	info->i = am29000->r[93];			break;
		case CPUINFO_INT_REGISTER + AM29000_R94:	info->i = am29000->r[94];			break;
		case CPUINFO_INT_REGISTER + AM29000_R95:	info->i = am29000->r[95];			break;
		case CPUINFO_INT_REGISTER + AM29000_R96:	info->i = am29000->r[96];			break;
		case CPUINFO_INT_REGISTER + AM29000_R97:	info->i = am29000->r[97];			break;
		case CPUINFO_INT_REGISTER + AM29000_R98:	info->i = am29000->r[98];			break;
		case CPUINFO_INT_REGISTER + AM29000_R99:	info->i = am29000->r[99];			break;
		case CPUINFO_INT_REGISTER + AM29000_R100:	info->i = am29000->r[100];			break;
		case CPUINFO_INT_REGISTER + AM29000_R101:	info->i = am29000->r[101];			break;
		case CPUINFO_INT_REGISTER + AM29000_R102:	info->i = am29000->r[102];			break;
		case CPUINFO_INT_REGISTER + AM29000_R103:	info->i = am29000->r[103];			break;
		case CPUINFO_INT_REGISTER + AM29000_R104:	info->i = am29000->r[104];			break;
		case CPUINFO_INT_REGISTER + AM29000_R105:	info->i = am29000->r[105];			break;
		case CPUINFO_INT_REGISTER + AM29000_R106:	info->i = am29000->r[106];			break;
		case CPUINFO_INT_REGISTER + AM29000_R107:	info->i = am29000->r[107];			break;
		case CPUINFO_INT_REGISTER + AM29000_R108:	info->i = am29000->r[108];			break;
		case CPUINFO_INT_REGISTER + AM29000_R109:	info->i = am29000->r[109];			break;
		case CPUINFO_INT_REGISTER + AM29000_R110:	info->i = am29000->r[110];			break;
		case CPUINFO_INT_REGISTER + AM29000_R111:	info->i = am29000->r[111];			break;
		case CPUINFO_INT_REGISTER + AM29000_R112:	info->i = am29000->r[112];			break;
		case CPUINFO_INT_REGISTER + AM29000_R113:	info->i = am29000->r[113];			break;
		case CPUINFO_INT_REGISTER + AM29000_R114:	info->i = am29000->r[114];			break;
		case CPUINFO_INT_REGISTER + AM29000_R115:	info->i = am29000->r[115];			break;
		case CPUINFO_INT_REGISTER + AM29000_R116:	info->i = am29000->r[116];			break;
		case CPUINFO_INT_REGISTER + AM29000_R117:	info->i = am29000->r[117];			break;
		case CPUINFO_INT_REGISTER + AM29000_R118:	info->i = am29000->r[118];			break;
		case CPUINFO_INT_REGISTER + AM29000_R119:	info->i = am29000->r[119];			break;
		case CPUINFO_INT_REGISTER + AM29000_R120:	info->i = am29000->r[120];			break;
		case CPUINFO_INT_REGISTER + AM29000_R121:	info->i = am29000->r[121];			break;
		case CPUINFO_INT_REGISTER + AM29000_R122:	info->i = am29000->r[122];			break;
		case CPUINFO_INT_REGISTER + AM29000_R123:	info->i = am29000->r[123];			break;
		case CPUINFO_INT_REGISTER + AM29000_R124:	info->i = am29000->r[124];			break;
		case CPUINFO_INT_REGISTER + AM29000_R125:	info->i = am29000->r[125];			break;
		case CPUINFO_INT_REGISTER + AM29000_R126:	info->i = am29000->r[126];			break;
		case CPUINFO_INT_REGISTER + AM29000_R127:	info->i = am29000->r[127];			break;
		case CPUINFO_INT_REGISTER + AM29000_R128:	info->i = am29000->r[128];			break;
		case CPUINFO_INT_REGISTER + AM29000_R129:	info->i = am29000->r[129];			break;
		case CPUINFO_INT_REGISTER + AM29000_R130:	info->i = am29000->r[130];			break;
		case CPUINFO_INT_REGISTER + AM29000_R131:	info->i = am29000->r[131];			break;
		case CPUINFO_INT_REGISTER + AM29000_R132:	info->i = am29000->r[132];			break;
		case CPUINFO_INT_REGISTER + AM29000_R133:	info->i = am29000->r[133];			break;
		case CPUINFO_INT_REGISTER + AM29000_R134:	info->i = am29000->r[134];			break;
		case CPUINFO_INT_REGISTER + AM29000_R135:	info->i = am29000->r[135];			break;
		case CPUINFO_INT_REGISTER + AM29000_R136:	info->i = am29000->r[136];			break;
		case CPUINFO_INT_REGISTER + AM29000_R137:	info->i = am29000->r[137];			break;
		case CPUINFO_INT_REGISTER + AM29000_R138:	info->i = am29000->r[138];			break;
		case CPUINFO_INT_REGISTER + AM29000_R139:	info->i = am29000->r[139];			break;
		case CPUINFO_INT_REGISTER + AM29000_R140:	info->i = am29000->r[140];			break;
		case CPUINFO_INT_REGISTER + AM29000_R141:	info->i = am29000->r[141];			break;
		case CPUINFO_INT_REGISTER + AM29000_R142:	info->i = am29000->r[142];			break;
		case CPUINFO_INT_REGISTER + AM29000_R143:	info->i = am29000->r[143];			break;
		case CPUINFO_INT_REGISTER + AM29000_R144:	info->i = am29000->r[144];			break;
		case CPUINFO_INT_REGISTER + AM29000_R145:	info->i = am29000->r[145];			break;
		case CPUINFO_INT_REGISTER + AM29000_R146:	info->i = am29000->r[146];			break;
		case CPUINFO_INT_REGISTER + AM29000_R147:	info->i = am29000->r[147];			break;
		case CPUINFO_INT_REGISTER + AM29000_R148:	info->i = am29000->r[148];			break;
		case CPUINFO_INT_REGISTER + AM29000_R149:	info->i = am29000->r[149];			break;
		case CPUINFO_INT_REGISTER + AM29000_R150:	info->i = am29000->r[150];			break;
		case CPUINFO_INT_REGISTER + AM29000_R151:	info->i = am29000->r[151];			break;
		case CPUINFO_INT_REGISTER + AM29000_R152:	info->i = am29000->r[152];			break;
		case CPUINFO_INT_REGISTER + AM29000_R153:	info->i = am29000->r[153];			break;
		case CPUINFO_INT_REGISTER + AM29000_R154:	info->i = am29000->r[154];			break;
		case CPUINFO_INT_REGISTER + AM29000_R155:	info->i = am29000->r[155];			break;
		case CPUINFO_INT_REGISTER + AM29000_R156:	info->i = am29000->r[156];			break;
		case CPUINFO_INT_REGISTER + AM29000_R157:	info->i = am29000->r[157];			break;
		case CPUINFO_INT_REGISTER + AM29000_R158:	info->i = am29000->r[158];			break;
		case CPUINFO_INT_REGISTER + AM29000_R159:	info->i = am29000->r[159];			break;
		case CPUINFO_INT_REGISTER + AM29000_R160:	info->i = am29000->r[160];			break;
		case CPUINFO_INT_REGISTER + AM29000_R161:	info->i = am29000->r[161];			break;
		case CPUINFO_INT_REGISTER + AM29000_R162:	info->i = am29000->r[162];			break;
		case CPUINFO_INT_REGISTER + AM29000_R163:	info->i = am29000->r[163];			break;
		case CPUINFO_INT_REGISTER + AM29000_R164:	info->i = am29000->r[164];			break;
		case CPUINFO_INT_REGISTER + AM29000_R165:	info->i = am29000->r[165];			break;
		case CPUINFO_INT_REGISTER + AM29000_R166:	info->i = am29000->r[166];			break;
		case CPUINFO_INT_REGISTER + AM29000_R167:	info->i = am29000->r[167];			break;
		case CPUINFO_INT_REGISTER + AM29000_R168:	info->i = am29000->r[168];			break;
		case CPUINFO_INT_REGISTER + AM29000_R169:	info->i = am29000->r[169];			break;
		case CPUINFO_INT_REGISTER + AM29000_R170:	info->i = am29000->r[170];			break;
		case CPUINFO_INT_REGISTER + AM29000_R171:	info->i = am29000->r[171];			break;
		case CPUINFO_INT_REGISTER + AM29000_R172:	info->i = am29000->r[172];			break;
		case CPUINFO_INT_REGISTER + AM29000_R173:	info->i = am29000->r[173];			break;
		case CPUINFO_INT_REGISTER + AM29000_R174:	info->i = am29000->r[174];			break;
		case CPUINFO_INT_REGISTER + AM29000_R175:	info->i = am29000->r[175];			break;
		case CPUINFO_INT_REGISTER + AM29000_R176:	info->i = am29000->r[176];			break;
		case CPUINFO_INT_REGISTER + AM29000_R177:	info->i = am29000->r[177];			break;
		case CPUINFO_INT_REGISTER + AM29000_R178:	info->i = am29000->r[178];			break;
		case CPUINFO_INT_REGISTER + AM29000_R179:	info->i = am29000->r[179];			break;
		case CPUINFO_INT_REGISTER + AM29000_R180:	info->i = am29000->r[180];			break;
		case CPUINFO_INT_REGISTER + AM29000_R181:	info->i = am29000->r[181];			break;
		case CPUINFO_INT_REGISTER + AM29000_R182:	info->i = am29000->r[182];			break;
		case CPUINFO_INT_REGISTER + AM29000_R183:	info->i = am29000->r[183];			break;
		case CPUINFO_INT_REGISTER + AM29000_R184:	info->i = am29000->r[184];			break;
		case CPUINFO_INT_REGISTER + AM29000_R185:	info->i = am29000->r[185];			break;
		case CPUINFO_INT_REGISTER + AM29000_R186:	info->i = am29000->r[186];			break;
		case CPUINFO_INT_REGISTER + AM29000_R187:	info->i = am29000->r[187];			break;
		case CPUINFO_INT_REGISTER + AM29000_R188:	info->i = am29000->r[188];			break;
		case CPUINFO_INT_REGISTER + AM29000_R189:	info->i = am29000->r[189];			break;
		case CPUINFO_INT_REGISTER + AM29000_R190:	info->i = am29000->r[190];			break;
		case CPUINFO_INT_REGISTER + AM29000_R191:	info->i = am29000->r[191];			break;
		case CPUINFO_INT_REGISTER + AM29000_R192:	info->i = am29000->r[192];			break;
		case CPUINFO_INT_REGISTER + AM29000_R193:	info->i = am29000->r[193];			break;
		case CPUINFO_INT_REGISTER + AM29000_R194:	info->i = am29000->r[194];			break;
		case CPUINFO_INT_REGISTER + AM29000_R195:	info->i = am29000->r[195];			break;
		case CPUINFO_INT_REGISTER + AM29000_R196:	info->i = am29000->r[196];			break;
		case CPUINFO_INT_REGISTER + AM29000_R197:	info->i = am29000->r[197];			break;
		case CPUINFO_INT_REGISTER + AM29000_R198:	info->i = am29000->r[198];			break;
		case CPUINFO_INT_REGISTER + AM29000_R199:	info->i = am29000->r[199];			break;
		case CPUINFO_INT_REGISTER + AM29000_R200:	info->i = am29000->r[200];			break;
		case CPUINFO_INT_REGISTER + AM29000_R201:	info->i = am29000->r[201];			break;
		case CPUINFO_INT_REGISTER + AM29000_R202:	info->i = am29000->r[202];			break;
		case CPUINFO_INT_REGISTER + AM29000_R203:	info->i = am29000->r[203];			break;
		case CPUINFO_INT_REGISTER + AM29000_R204:	info->i = am29000->r[204];			break;
		case CPUINFO_INT_REGISTER + AM29000_R205:	info->i = am29000->r[205];			break;
		case CPUINFO_INT_REGISTER + AM29000_R206:	info->i = am29000->r[206];			break;
		case CPUINFO_INT_REGISTER + AM29000_R207:	info->i = am29000->r[207];			break;
		case CPUINFO_INT_REGISTER + AM29000_R208:	info->i = am29000->r[208];			break;
		case CPUINFO_INT_REGISTER + AM29000_R209:	info->i = am29000->r[209];			break;
		case CPUINFO_INT_REGISTER + AM29000_R210:	info->i = am29000->r[210];			break;
		case CPUINFO_INT_REGISTER + AM29000_R211:	info->i = am29000->r[211];			break;
		case CPUINFO_INT_REGISTER + AM29000_R212:	info->i = am29000->r[212];			break;
		case CPUINFO_INT_REGISTER + AM29000_R213:	info->i = am29000->r[213];			break;
		case CPUINFO_INT_REGISTER + AM29000_R214:	info->i = am29000->r[214];			break;
		case CPUINFO_INT_REGISTER + AM29000_R215:	info->i = am29000->r[215];			break;
		case CPUINFO_INT_REGISTER + AM29000_R216:	info->i = am29000->r[216];			break;
		case CPUINFO_INT_REGISTER + AM29000_R217:	info->i = am29000->r[217];			break;
		case CPUINFO_INT_REGISTER + AM29000_R218:	info->i = am29000->r[218];			break;
		case CPUINFO_INT_REGISTER + AM29000_R219:	info->i = am29000->r[219];			break;
		case CPUINFO_INT_REGISTER + AM29000_R220:	info->i = am29000->r[220];			break;
		case CPUINFO_INT_REGISTER + AM29000_R221:	info->i = am29000->r[221];			break;
		case CPUINFO_INT_REGISTER + AM29000_R222:	info->i = am29000->r[222];			break;
		case CPUINFO_INT_REGISTER + AM29000_R223:	info->i = am29000->r[223];			break;
		case CPUINFO_INT_REGISTER + AM29000_R224:	info->i = am29000->r[224];			break;
		case CPUINFO_INT_REGISTER + AM29000_R225:	info->i = am29000->r[225];			break;
		case CPUINFO_INT_REGISTER + AM29000_R226:	info->i = am29000->r[226];			break;
		case CPUINFO_INT_REGISTER + AM29000_R227:	info->i = am29000->r[227];			break;
		case CPUINFO_INT_REGISTER + AM29000_R228:	info->i = am29000->r[228];			break;
		case CPUINFO_INT_REGISTER + AM29000_R229:	info->i = am29000->r[229];			break;
		case CPUINFO_INT_REGISTER + AM29000_R230:	info->i = am29000->r[230];			break;
		case CPUINFO_INT_REGISTER + AM29000_R231:	info->i = am29000->r[231];			break;
		case CPUINFO_INT_REGISTER + AM29000_R232:	info->i = am29000->r[232];			break;
		case CPUINFO_INT_REGISTER + AM29000_R233:	info->i = am29000->r[233];			break;
		case CPUINFO_INT_REGISTER + AM29000_R234:	info->i = am29000->r[234];			break;
		case CPUINFO_INT_REGISTER + AM29000_R235:	info->i = am29000->r[235];			break;
		case CPUINFO_INT_REGISTER + AM29000_R236:	info->i = am29000->r[236];			break;
		case CPUINFO_INT_REGISTER + AM29000_R237:	info->i = am29000->r[237];			break;
		case CPUINFO_INT_REGISTER + AM29000_R238:	info->i = am29000->r[238];			break;
		case CPUINFO_INT_REGISTER + AM29000_R239:	info->i = am29000->r[239];			break;
		case CPUINFO_INT_REGISTER + AM29000_R240:	info->i = am29000->r[240];			break;
		case CPUINFO_INT_REGISTER + AM29000_R241:	info->i = am29000->r[241];			break;
		case CPUINFO_INT_REGISTER + AM29000_R242:	info->i = am29000->r[242];			break;
		case CPUINFO_INT_REGISTER + AM29000_R243:	info->i = am29000->r[243];			break;
		case CPUINFO_INT_REGISTER + AM29000_R244:	info->i = am29000->r[244];			break;
		case CPUINFO_INT_REGISTER + AM29000_R245:	info->i = am29000->r[245];			break;
		case CPUINFO_INT_REGISTER + AM29000_R246:	info->i = am29000->r[246];			break;
		case CPUINFO_INT_REGISTER + AM29000_R247:	info->i = am29000->r[247];			break;
		case CPUINFO_INT_REGISTER + AM29000_R248:	info->i = am29000->r[248];			break;
		case CPUINFO_INT_REGISTER + AM29000_R249:	info->i = am29000->r[249];			break;
		case CPUINFO_INT_REGISTER + AM29000_R250:	info->i = am29000->r[250];			break;
		case CPUINFO_INT_REGISTER + AM29000_R251:	info->i = am29000->r[251];			break;
		case CPUINFO_INT_REGISTER + AM29000_R252:	info->i = am29000->r[252];			break;
		case CPUINFO_INT_REGISTER + AM29000_R253:	info->i = am29000->r[253];			break;
		case CPUINFO_INT_REGISTER + AM29000_R254:	info->i = am29000->r[254];			break;
		case CPUINFO_INT_REGISTER + AM29000_R255:	info->i = am29000->r[255];			break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(am29000);			break;
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(am29000);				break;
		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(am29000);				break;
		case CPUINFO_FCT_EXIT:							info->exit = CPU_EXIT_NAME(am29000);				break;
		case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(am29000);			break;
		case CPUINFO_FCT_BURN:							info->burn = NULL;									break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(am29000);	break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &am29000->icount;					break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "Am29000");							break;
		case CPUINFO_STR_FAMILY:						strcpy(info->s, "AMD Am29000");						break;
		case CPUINFO_STR_VERSION:						strcpy(info->s, "1.0");								break;
		case CPUINFO_STR_SOURCE_FILE:					strcpy(info->s, __FILE__);							break;
		case CPUINFO_STR_CREDITS:						strcpy(info->s, "Copyright Philip Bennett");		break;

		case CPUINFO_STR_FLAGS:							sprintf(info->s, "%c%c%c%c%c%c%c%c%c|%3d",	am29000->alu & ALU_V ? 'V' : '.',
																									am29000->alu & ALU_Z ? 'Z' : '.',
																									am29000->alu & ALU_N ? 'N' : '.',
																									am29000->alu & ALU_C ? 'C' : '.',

																									am29000->cps & CPS_IP ? 'I' : '.',
																									am29000->cps & CPS_FZ ? 'F' : '.',
																									am29000->cps & CPS_SM ? 'S' : 'U',
																									am29000->cps & CPS_DI ? 'I' : '.',
																									am29000->cps & CPS_DA ? 'D' : '.',
																									(am29000->r[1] >> 2) & 0x7f); break;

		case CPUINFO_STR_REGISTER + AM29000_PC: 		sprintf(info->s, "PC: %08X", am29000->pc);			break;
		case CPUINFO_STR_REGISTER + AM29000_VAB:		sprintf(info->s, "VAB: %08X", am29000->vab);		break;
		case CPUINFO_STR_REGISTER + AM29000_OPS:		sprintf(info->s, "OPS: %08X", am29000->ops);		break;
		case CPUINFO_STR_REGISTER + AM29000_CPS:		sprintf(info->s, "CPS: %08X", am29000->cps);		break;
		case CPUINFO_STR_REGISTER + AM29000_CFG:		sprintf(info->s, "CFG: %08X", am29000->cfg);		break;
		case CPUINFO_STR_REGISTER + AM29000_CHA:		sprintf(info->s, "CHA: %08X", am29000->cha);		break;
		case CPUINFO_STR_REGISTER + AM29000_CHD:		sprintf(info->s, "CHD: %08X", am29000->chd);		break;
		case CPUINFO_STR_REGISTER + AM29000_CHC:		sprintf(info->s, "CHC: %08X", am29000->chc);		break;
		case CPUINFO_STR_REGISTER + AM29000_RBP:		sprintf(info->s, "RBP: %08X", am29000->rbp);		break;
		case CPUINFO_STR_REGISTER + AM29000_TMC:		sprintf(info->s, "TMC: %08X", am29000->tmc);		break;
		case CPUINFO_STR_REGISTER + AM29000_TMR:		sprintf(info->s, "TMR: %08X", am29000->tmr);		break;
		case CPUINFO_STR_REGISTER + AM29000_PC0:		sprintf(info->s, "PC0: %08X", am29000->pc0);		break;
		case CPUINFO_STR_REGISTER + AM29000_PC1:		sprintf(info->s, "PC1: %08X", am29000->pc1);		break;
		case CPUINFO_STR_REGISTER + AM29000_PC2:		sprintf(info->s, "PC2: %08X", am29000->pc2);		break;
		case CPUINFO_STR_REGISTER + AM29000_MMU:		sprintf(info->s, "MMU: %08X", am29000->mmu);		break;
		case CPUINFO_STR_REGISTER + AM29000_LRU:		sprintf(info->s, "LRU: %08X", am29000->lru);		break;
		case CPUINFO_STR_REGISTER + AM29000_IPC:		sprintf(info->s, "IPC: %08X", am29000->ipc);		break;
		case CPUINFO_STR_REGISTER + AM29000_IPA:		sprintf(info->s, "IPA: %08X", am29000->ipa);		break;
		case CPUINFO_STR_REGISTER + AM29000_IPB:		sprintf(info->s, "IPB: %08X", am29000->ipb);		break;
		case CPUINFO_STR_REGISTER + AM29000_Q:			sprintf(info->s, "Q: %08X", am29000->q);			break;
		case CPUINFO_STR_REGISTER + AM29000_ALU:		sprintf(info->s, "ALU: %08X", am29000->alu);		break;
		case CPUINFO_STR_REGISTER + AM29000_BP:			sprintf(info->s, "BP: %08X", GET_ALU_BP);			break;
		case CPUINFO_STR_REGISTER + AM29000_FC:			sprintf(info->s, "FC: %08X", GET_ALU_FC);			break;
		case CPUINFO_STR_REGISTER + AM29000_CR:			sprintf(info->s, "CR: %08X", GET_CHC_CR);			break;
		case CPUINFO_STR_REGISTER + AM29000_FPE:		sprintf(info->s, "FPE: %08X", am29000->fpe);		break;
		case CPUINFO_STR_REGISTER + AM29000_INTE:		sprintf(info->s, "INTE: %08X", am29000->inte);		break;
		case CPUINFO_STR_REGISTER + AM29000_FPS:		sprintf(info->s, "FPS: %08X", am29000->fps);		break;
		case CPUINFO_STR_REGISTER + AM29000_R1:			sprintf(info->s, "R1: %08X", am29000->r[1]);		break;
		case CPUINFO_STR_REGISTER + AM29000_R64:		sprintf(info->s, "R64: %08X", am29000->r[64]);		break;
		case CPUINFO_STR_REGISTER + AM29000_R65:		sprintf(info->s, "R65: %08X", am29000->r[65]);		break;
		case CPUINFO_STR_REGISTER + AM29000_R66:		sprintf(info->s, "R66: %08X", am29000->r[66]);		break;
		case CPUINFO_STR_REGISTER + AM29000_R67:		sprintf(info->s, "R67: %08X", am29000->r[67]);		break;
		case CPUINFO_STR_REGISTER + AM29000_R68:		sprintf(info->s, "R68: %08X", am29000->r[68]);		break;
		case CPUINFO_STR_REGISTER + AM29000_R69:		sprintf(info->s, "R69: %08X", am29000->r[69]);		break;
		case CPUINFO_STR_REGISTER + AM29000_R70:		sprintf(info->s, "R70: %08X", am29000->r[70]);		break;
		case CPUINFO_STR_REGISTER + AM29000_R71:		sprintf(info->s, "R71: %08X", am29000->r[71]);		break;
		case CPUINFO_STR_REGISTER + AM29000_R72:		sprintf(info->s, "R72: %08X", am29000->r[72]);		break;
		case CPUINFO_STR_REGISTER + AM29000_R73:		sprintf(info->s, "R73: %08X", am29000->r[73]);		break;
		case CPUINFO_STR_REGISTER + AM29000_R74:		sprintf(info->s, "R74: %08X", am29000->r[74]);		break;
		case CPUINFO_STR_REGISTER + AM29000_R75:		sprintf(info->s, "R75: %08X", am29000->r[75]);		break;
		case CPUINFO_STR_REGISTER + AM29000_R76:		sprintf(info->s, "R76: %08X", am29000->r[76]);		break;
		case CPUINFO_STR_REGISTER + AM29000_R77:		sprintf(info->s, "R77: %08X", am29000->r[77]);		break;
		case CPUINFO_STR_REGISTER + AM29000_R78:		sprintf(info->s, "R78: %08X", am29000->r[78]);		break;
		case CPUINFO_STR_REGISTER + AM29000_R79:		sprintf(info->s, "R79: %08X", am29000->r[79]);		break;
		case CPUINFO_STR_REGISTER + AM29000_R80:		sprintf(info->s, "R80: %08X", am29000->r[80]);		break;
		case CPUINFO_STR_REGISTER + AM29000_R81:		sprintf(info->s, "R81: %08X", am29000->r[81]);		break;
		case CPUINFO_STR_REGISTER + AM29000_R82:		sprintf(info->s, "R82: %08X", am29000->r[82]);		break;
		case CPUINFO_STR_REGISTER + AM29000_R83:		sprintf(info->s, "R83: %08X", am29000->r[83]);		break;
		case CPUINFO_STR_REGISTER + AM29000_R84:		sprintf(info->s, "R84: %08X", am29000->r[84]);		break;
		case CPUINFO_STR_REGISTER + AM29000_R85:		sprintf(info->s, "R85: %08X", am29000->r[85]);		break;
		case CPUINFO_STR_REGISTER + AM29000_R86:		sprintf(info->s, "R86: %08X", am29000->r[86]);		break;
		case CPUINFO_STR_REGISTER + AM29000_R87:		sprintf(info->s, "R87: %08X", am29000->r[87]);		break;
		case CPUINFO_STR_REGISTER + AM29000_R88:		sprintf(info->s, "R88: %08X", am29000->r[88]);		break;
		case CPUINFO_STR_REGISTER + AM29000_R89:		sprintf(info->s, "R89: %08X", am29000->r[89]);		break;
		case CPUINFO_STR_REGISTER + AM29000_R90:		sprintf(info->s, "R90: %08X", am29000->r[90]);		break;
		case CPUINFO_STR_REGISTER + AM29000_R91:		sprintf(info->s, "R91: %08X", am29000->r[91]);		break;
		case CPUINFO_STR_REGISTER + AM29000_R92:		sprintf(info->s, "R92: %08X", am29000->r[92]);		break;
		case CPUINFO_STR_REGISTER + AM29000_R93:		sprintf(info->s, "R93: %08X", am29000->r[93]);		break;
		case CPUINFO_STR_REGISTER + AM29000_R94:		sprintf(info->s, "R94: %08X", am29000->r[94]);		break;
		case CPUINFO_STR_REGISTER + AM29000_R95:		sprintf(info->s, "R95: %08X", am29000->r[95]);		break;
		case CPUINFO_STR_REGISTER + AM29000_R96:		sprintf(info->s, "R96: %08X", am29000->r[96]);		break;
		case CPUINFO_STR_REGISTER + AM29000_R97:		sprintf(info->s, "R97: %08X", am29000->r[97]);		break;
		case CPUINFO_STR_REGISTER + AM29000_R98:		sprintf(info->s, "R98: %08X", am29000->r[98]);		break;
		case CPUINFO_STR_REGISTER + AM29000_R99:		sprintf(info->s, "R99: %08X", am29000->r[99]);		break;
		case CPUINFO_STR_REGISTER + AM29000_R100:		sprintf(info->s, "R100: %08X", am29000->r[100]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R101:		sprintf(info->s, "R101: %08X", am29000->r[101]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R102:		sprintf(info->s, "R102: %08X", am29000->r[102]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R103:		sprintf(info->s, "R103: %08X", am29000->r[103]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R104:		sprintf(info->s, "R104: %08X", am29000->r[104]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R105:		sprintf(info->s, "R105: %08X", am29000->r[105]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R106:		sprintf(info->s, "R106: %08X", am29000->r[106]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R107:		sprintf(info->s, "R107: %08X", am29000->r[107]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R108:		sprintf(info->s, "R108: %08X", am29000->r[108]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R109:		sprintf(info->s, "R109: %08X", am29000->r[109]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R110:		sprintf(info->s, "R110: %08X", am29000->r[110]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R111:		sprintf(info->s, "R111: %08X", am29000->r[111]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R112:		sprintf(info->s, "R112: %08X", am29000->r[112]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R113:		sprintf(info->s, "R113: %08X", am29000->r[113]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R114:		sprintf(info->s, "R114: %08X", am29000->r[114]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R115:		sprintf(info->s, "R115: %08X", am29000->r[115]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R116:		sprintf(info->s, "R116: %08X", am29000->r[116]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R117:		sprintf(info->s, "R117: %08X", am29000->r[117]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R118:		sprintf(info->s, "R118: %08X", am29000->r[118]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R119:		sprintf(info->s, "R119: %08X", am29000->r[119]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R120:		sprintf(info->s, "R120: %08X", am29000->r[120]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R121:		sprintf(info->s, "R121: %08X", am29000->r[121]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R122:		sprintf(info->s, "R122: %08X", am29000->r[122]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R123:		sprintf(info->s, "R123: %08X", am29000->r[123]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R124:		sprintf(info->s, "R124: %08X", am29000->r[124]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R125:		sprintf(info->s, "R125: %08X", am29000->r[125]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R126:		sprintf(info->s, "R126: %08X", am29000->r[126]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R127:		sprintf(info->s, "R127: %08X", am29000->r[127]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R128:		sprintf(info->s, "R128: %08X", am29000->r[128]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R129:		sprintf(info->s, "R129: %08X", am29000->r[129]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R130:		sprintf(info->s, "R130: %08X", am29000->r[130]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R131:		sprintf(info->s, "R131: %08X", am29000->r[131]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R132:		sprintf(info->s, "R132: %08X", am29000->r[132]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R133:		sprintf(info->s, "R133: %08X", am29000->r[133]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R134:		sprintf(info->s, "R134: %08X", am29000->r[134]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R135:		sprintf(info->s, "R135: %08X", am29000->r[135]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R136:		sprintf(info->s, "R136: %08X", am29000->r[136]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R137:		sprintf(info->s, "R137: %08X", am29000->r[137]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R138:		sprintf(info->s, "R138: %08X", am29000->r[138]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R139:		sprintf(info->s, "R139: %08X", am29000->r[139]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R140:		sprintf(info->s, "R140: %08X", am29000->r[140]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R141:		sprintf(info->s, "R141: %08X", am29000->r[141]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R142:		sprintf(info->s, "R142: %08X", am29000->r[142]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R143:		sprintf(info->s, "R143: %08X", am29000->r[143]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R144:		sprintf(info->s, "R144: %08X", am29000->r[144]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R145:		sprintf(info->s, "R145: %08X", am29000->r[145]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R146:		sprintf(info->s, "R146: %08X", am29000->r[146]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R147:		sprintf(info->s, "R147: %08X", am29000->r[147]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R148:		sprintf(info->s, "R148: %08X", am29000->r[148]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R149:		sprintf(info->s, "R149: %08X", am29000->r[149]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R150:		sprintf(info->s, "R150: %08X", am29000->r[150]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R151:		sprintf(info->s, "R151: %08X", am29000->r[151]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R152:		sprintf(info->s, "R152: %08X", am29000->r[152]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R153:		sprintf(info->s, "R153: %08X", am29000->r[153]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R154:		sprintf(info->s, "R154: %08X", am29000->r[154]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R155:		sprintf(info->s, "R155: %08X", am29000->r[155]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R156:		sprintf(info->s, "R156: %08X", am29000->r[156]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R157:		sprintf(info->s, "R157: %08X", am29000->r[157]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R158:		sprintf(info->s, "R158: %08X", am29000->r[158]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R159:		sprintf(info->s, "R159: %08X", am29000->r[159]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R160:		sprintf(info->s, "R160: %08X", am29000->r[160]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R161:		sprintf(info->s, "R161: %08X", am29000->r[161]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R162:		sprintf(info->s, "R162: %08X", am29000->r[162]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R163:		sprintf(info->s, "R163: %08X", am29000->r[163]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R164:		sprintf(info->s, "R164: %08X", am29000->r[164]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R165:		sprintf(info->s, "R165: %08X", am29000->r[165]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R166:		sprintf(info->s, "R166: %08X", am29000->r[166]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R167:		sprintf(info->s, "R167: %08X", am29000->r[167]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R168:		sprintf(info->s, "R168: %08X", am29000->r[168]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R169:		sprintf(info->s, "R169: %08X", am29000->r[169]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R170:		sprintf(info->s, "R170: %08X", am29000->r[170]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R171:		sprintf(info->s, "R171: %08X", am29000->r[171]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R172:		sprintf(info->s, "R172: %08X", am29000->r[172]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R173:		sprintf(info->s, "R173: %08X", am29000->r[173]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R174:		sprintf(info->s, "R174: %08X", am29000->r[174]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R175:		sprintf(info->s, "R175: %08X", am29000->r[175]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R176:		sprintf(info->s, "R176: %08X", am29000->r[176]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R177:		sprintf(info->s, "R177: %08X", am29000->r[177]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R178:		sprintf(info->s, "R178: %08X", am29000->r[178]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R179:		sprintf(info->s, "R179: %08X", am29000->r[179]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R180:		sprintf(info->s, "R180: %08X", am29000->r[180]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R181:		sprintf(info->s, "R181: %08X", am29000->r[181]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R182:		sprintf(info->s, "R182: %08X", am29000->r[182]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R183:		sprintf(info->s, "R183: %08X", am29000->r[183]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R184:		sprintf(info->s, "R184: %08X", am29000->r[184]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R185:		sprintf(info->s, "R185: %08X", am29000->r[185]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R186:		sprintf(info->s, "R186: %08X", am29000->r[186]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R187:		sprintf(info->s, "R187: %08X", am29000->r[187]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R188:		sprintf(info->s, "R188: %08X", am29000->r[188]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R189:		sprintf(info->s, "R189: %08X", am29000->r[189]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R190:		sprintf(info->s, "R190: %08X", am29000->r[190]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R191:		sprintf(info->s, "R191: %08X", am29000->r[191]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R192:		sprintf(info->s, "R192: %08X", am29000->r[192]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R193:		sprintf(info->s, "R193: %08X", am29000->r[193]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R194:		sprintf(info->s, "R194: %08X", am29000->r[194]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R195:		sprintf(info->s, "R195: %08X", am29000->r[195]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R196:		sprintf(info->s, "R196: %08X", am29000->r[196]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R197:		sprintf(info->s, "R197: %08X", am29000->r[197]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R198:		sprintf(info->s, "R198: %08X", am29000->r[198]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R199:		sprintf(info->s, "R199: %08X", am29000->r[199]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R200:		sprintf(info->s, "R200: %08X", am29000->r[200]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R201:		sprintf(info->s, "R201: %08X", am29000->r[201]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R202:		sprintf(info->s, "R202: %08X", am29000->r[202]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R203:		sprintf(info->s, "R203: %08X", am29000->r[203]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R204:		sprintf(info->s, "R204: %08X", am29000->r[204]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R205:		sprintf(info->s, "R205: %08X", am29000->r[205]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R206:		sprintf(info->s, "R206: %08X", am29000->r[206]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R207:		sprintf(info->s, "R207: %08X", am29000->r[207]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R208:		sprintf(info->s, "R208: %08X", am29000->r[208]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R209:		sprintf(info->s, "R209: %08X", am29000->r[209]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R210:		sprintf(info->s, "R210: %08X", am29000->r[210]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R211:		sprintf(info->s, "R211: %08X", am29000->r[211]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R212:		sprintf(info->s, "R212: %08X", am29000->r[212]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R213:		sprintf(info->s, "R213: %08X", am29000->r[213]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R214:		sprintf(info->s, "R214: %08X", am29000->r[214]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R215:		sprintf(info->s, "R215: %08X", am29000->r[215]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R216:		sprintf(info->s, "R216: %08X", am29000->r[216]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R217:		sprintf(info->s, "R217: %08X", am29000->r[217]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R218:		sprintf(info->s, "R218: %08X", am29000->r[218]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R219:		sprintf(info->s, "R219: %08X", am29000->r[219]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R220:		sprintf(info->s, "R220: %08X", am29000->r[220]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R221:		sprintf(info->s, "R221: %08X", am29000->r[221]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R222:		sprintf(info->s, "R222: %08X", am29000->r[222]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R223:		sprintf(info->s, "R223: %08X", am29000->r[223]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R224:		sprintf(info->s, "R224: %08X", am29000->r[224]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R225:		sprintf(info->s, "R225: %08X", am29000->r[225]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R226:		sprintf(info->s, "R226: %08X", am29000->r[226]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R227:		sprintf(info->s, "R227: %08X", am29000->r[227]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R228:		sprintf(info->s, "R228: %08X", am29000->r[228]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R229:		sprintf(info->s, "R229: %08X", am29000->r[229]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R230:		sprintf(info->s, "R230: %08X", am29000->r[230]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R231:		sprintf(info->s, "R231: %08X", am29000->r[231]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R232:		sprintf(info->s, "R232: %08X", am29000->r[232]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R233:		sprintf(info->s, "R233: %08X", am29000->r[233]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R234:		sprintf(info->s, "R234: %08X", am29000->r[234]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R235:		sprintf(info->s, "R235: %08X", am29000->r[235]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R236:		sprintf(info->s, "R236: %08X", am29000->r[236]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R237:		sprintf(info->s, "R237: %08X", am29000->r[237]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R238:		sprintf(info->s, "R238: %08X", am29000->r[238]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R239:		sprintf(info->s, "R239: %08X", am29000->r[239]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R240:		sprintf(info->s, "R240: %08X", am29000->r[240]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R241:		sprintf(info->s, "R241: %08X", am29000->r[241]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R242:		sprintf(info->s, "R242: %08X", am29000->r[242]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R243:		sprintf(info->s, "R243: %08X", am29000->r[243]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R244:		sprintf(info->s, "R244: %08X", am29000->r[244]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R245:		sprintf(info->s, "R245: %08X", am29000->r[245]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R246:		sprintf(info->s, "R246: %08X", am29000->r[246]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R247:		sprintf(info->s, "R247: %08X", am29000->r[247]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R248:		sprintf(info->s, "R248: %08X", am29000->r[248]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R249:		sprintf(info->s, "R249: %08X", am29000->r[249]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R250:		sprintf(info->s, "R250: %08X", am29000->r[250]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R251:		sprintf(info->s, "R251: %08X", am29000->r[251]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R252:		sprintf(info->s, "R252: %08X", am29000->r[252]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R253:		sprintf(info->s, "R253: %08X", am29000->r[253]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R254:		sprintf(info->s, "R254: %08X", am29000->r[254]);	break;
		case CPUINFO_STR_REGISTER + AM29000_R255:		sprintf(info->s, "R255: %08X", am29000->r[255]);	break;
	}
}


DEFINE_LEGACY_CPU_DEVICE(AM29000, am29000);
