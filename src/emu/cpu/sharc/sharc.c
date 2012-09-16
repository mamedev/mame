/* Analog Devices ADSP-2106x SHARC emulator v2.0

   Written by Ville Linde
*/

#include "emu.h"
#include "debugger.h"
#include "sharc.h"

CPU_DISASSEMBLE( sharc );

enum
{
	SHARC_PC=1,		SHARC_PCSTK,	SHARC_MODE1,	SHARC_MODE2,
	SHARC_ASTAT,	SHARC_STKY,		SHARC_IRPTL,	SHARC_IMASK,
	SHARC_IMASKP,	SHARC_USTAT1,	SHARC_USTAT2,	SHARC_LCNTR,
	SHARC_R0,		SHARC_R1,		SHARC_R2,		SHARC_R3,
	SHARC_R4,		SHARC_R5,		SHARC_R6,		SHARC_R7,
	SHARC_R8,		SHARC_R9,		SHARC_R10,		SHARC_R11,
	SHARC_R12,		SHARC_R13,		SHARC_R14,		SHARC_R15,
	SHARC_SYSCON,	SHARC_SYSSTAT,	SHARC_MRF,		SHARC_MRB,
	SHARC_STSTKP,	SHARC_PCSTKP,	SHARC_LSTKP,
	SHARC_FADDR,	SHARC_DADDR,
	SHARC_I0,		SHARC_I1,		SHARC_I2,		SHARC_I3,
	SHARC_I4,		SHARC_I5,		SHARC_I6,		SHARC_I7,
	SHARC_I8,		SHARC_I9,		SHARC_I10,		SHARC_I11,
	SHARC_I12,		SHARC_I13,		SHARC_I14,		SHARC_I15,
	SHARC_M0,		SHARC_M1,		SHARC_M2,		SHARC_M3,
	SHARC_M4,		SHARC_M5,		SHARC_M6,		SHARC_M7,
	SHARC_M8,		SHARC_M9,		SHARC_M10,		SHARC_M11,
	SHARC_M12,		SHARC_M13,		SHARC_M14,		SHARC_M15,
	SHARC_L0,		SHARC_L1,		SHARC_L2,		SHARC_L3,
	SHARC_L4,		SHARC_L5,		SHARC_L6,		SHARC_L7,
	SHARC_L8,		SHARC_L9,		SHARC_L10,		SHARC_L11,
	SHARC_L12,		SHARC_L13,		SHARC_L14,		SHARC_L15,
	SHARC_B0,		SHARC_B1,		SHARC_B2,		SHARC_B3,
	SHARC_B4,		SHARC_B5,		SHARC_B6,		SHARC_B7,
	SHARC_B8,		SHARC_B9,		SHARC_B10,		SHARC_B11,
	SHARC_B12,		SHARC_B13,		SHARC_B14,		SHARC_B15,
};

struct SHARC_DAG 
{
	UINT32 i[8];
	UINT32 m[8];
	UINT32 b[8];
	UINT32 l[8];
};

union SHARC_REG 
{
	INT32 r;
	float f;
};

struct DMA_REGS 
{
	UINT32 control;
	UINT32 int_index;
	UINT32 int_modifier;
	UINT32 int_count;
	UINT32 chain_ptr;
	UINT32 gen_purpose;
	UINT32 ext_index;
	UINT32 ext_modifier;
	UINT32 ext_count;
};

struct LADDR 
{
	UINT32 addr;
	UINT32 code;
	UINT32 loop_type;
};

struct DMA_OP 
{
	UINT32 src;
	UINT32 dst;
	UINT32 chain_ptr;
	INT32 src_modifier;
	INT32 dst_modifier;
	INT32 src_count;
	INT32 dst_count;
	INT32 pmode;
	INT32 chained_direction;
	emu_timer *timer;
	bool active;
};

typedef struct _SHARC_REGS SHARC_REGS;
struct _SHARC_REGS
{
	UINT32 pc;
	SHARC_REG r[16];
	SHARC_REG reg_alt[16];
	UINT64 mrf;
	UINT64 mrb;

	UINT32 pcstack[32];
	UINT32 lcstack[6];
	UINT32 lastack[6];
	UINT32 lstkp;

	UINT32 faddr;
	UINT32 daddr;
	UINT32 pcstk;
	UINT32 pcstkp;
	LADDR laddr;
	UINT32 curlcntr;
	UINT32 lcntr;

	/* Data Address Generator (DAG) */
	SHARC_DAG dag1;		// (DM bus)
	SHARC_DAG dag2;		// (PM bus)
	SHARC_DAG dag1_alt;
	SHARC_DAG dag2_alt;

	DMA_REGS dma[12];

	/* System registers */
	UINT32 mode1;
	UINT32 mode2;
	UINT32 astat;
	UINT32 stky;
	UINT32 irptl;
	UINT32 imask;
	UINT32 imaskp;
	UINT32 ustat1;
	UINT32 ustat2;

	UINT32 flag[4];

	UINT32 syscon;
	UINT32 sysstat;

	struct
	{
		UINT32 mode1;
		UINT32 astat;
	} status_stack[5];
	INT32 status_stkp;

	UINT64 px;

	UINT16 *internal_ram;
	UINT16 *internal_ram_block0, *internal_ram_block1;

	device_irq_acknowledge_callback irq_callback;
	legacy_cpu_device *device;
	address_space *program;
	address_space *data;
	void (*opcode_handler)(SHARC_REGS *cpustate);
	int icount;
	UINT64 opcode;

	UINT32 nfaddr;

	INT32 idle;
	INT32 irq_active;
	INT32 active_irq_num;

	SHARC_BOOT_MODE boot_mode;

	DMA_OP dma_op[12];
	UINT32 dma_status;

	INT32 interrupt_active;

	UINT32 iop_delayed_reg;
	UINT32 iop_delayed_data;
	emu_timer *delayed_iop_timer;

	UINT32 delay_slot1, delay_slot2;

	INT32 systemreg_latency_cycles;
	INT32 systemreg_latency_reg;
	UINT32 systemreg_latency_data;
	UINT32 systemreg_previous_data;

	UINT32 astat_old;
	UINT32 astat_old_old;
	UINT32 astat_old_old_old;
};


static void sharc_dma_exec(SHARC_REGS *cpustate, int channel);
static void check_interrupts(SHARC_REGS *cpustate);

static void (* sharc_op[512])(SHARC_REGS *cpustate);



#define ROPCODE(pc)		((UINT64)(cpustate->internal_ram[((pc-0x20000) * 3) + 0]) << 32) | \
						((UINT64)(cpustate->internal_ram[((pc-0x20000) * 3) + 1]) << 16) | \
						((UINT64)(cpustate->internal_ram[((pc-0x20000) * 3) + 2]) << 0)

INLINE SHARC_REGS *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == ADSP21062);
	return (SHARC_REGS *)downcast<legacy_cpu_device *>(device)->token();
}

INLINE void CHANGE_PC(SHARC_REGS *cpustate, UINT32 newpc)
{
	cpustate->pc = newpc;
	cpustate->daddr = newpc;
	cpustate->faddr = newpc+1;
	cpustate->nfaddr = newpc+2;
}

INLINE void CHANGE_PC_DELAYED(SHARC_REGS *cpustate, UINT32 newpc)
{
	cpustate->nfaddr = newpc;

	cpustate->delay_slot1 = cpustate->pc;
	cpustate->delay_slot2 = cpustate->daddr;
}

static TIMER_CALLBACK(sharc_iop_delayed_write_callback)
{
	SHARC_REGS *cpustate = (SHARC_REGS *)ptr;

	switch (cpustate->iop_delayed_reg)
	{
		case 0x1c:
		{
			if (cpustate->iop_delayed_data & 0x1)
			{
				sharc_dma_exec(cpustate, 6);
			}
			break;
		}

		case 0x1d:
		{
			if (cpustate->iop_delayed_data & 0x1)
			{
				sharc_dma_exec(cpustate, 7);
			}
			break;
		}

		default:	fatalerror("SHARC: sharc_iop_delayed_write: unknown IOP register %02X\n", cpustate->iop_delayed_reg);
	}

	cpustate->delayed_iop_timer->adjust(attotime::never, 0);
}

static void sharc_iop_delayed_w(SHARC_REGS *cpustate, UINT32 reg, UINT32 data, int cycles)
{
	cpustate->iop_delayed_reg = reg;
	cpustate->iop_delayed_data = data;

	cpustate->delayed_iop_timer->adjust(cpustate->device->cycles_to_attotime(cycles), 0);
}


/* IOP registers */
static UINT32 sharc_iop_r(SHARC_REGS *cpustate, UINT32 address)
{
	switch (address)
	{
		case 0x00: return 0;	// System configuration

		case 0x37:		// DMA status
		{
			return cpustate->dma_status;
		}
		default:		fatalerror("sharc_iop_r: Unimplemented IOP reg %02X at %08X\n", address, cpustate->pc);
	}
	return 0;
}

static void sharc_iop_w(SHARC_REGS *cpustate, UINT32 address, UINT32 data)
{
	switch (address)
	{
		case 0x00: break;		// System configuration
		case 0x02: break;		// External Memory Wait State Configuration

		case 0x08: break;		// Message Register 0
		case 0x09: break;		// Message Register 1
		case 0x0a: break;		// Message Register 2
		case 0x0b: break;		// Message Register 3
		case 0x0c: break;		// Message Register 4
		case 0x0d: break;		// Message Register 5
		case 0x0e: break;		// Message Register 6
		case 0x0f: break;		// Message Register 7

		// DMA 6
		case 0x1c:
		{
			cpustate->dma[6].control = data;
			sharc_iop_delayed_w(cpustate, 0x1c, data, 1);
			break;
		}

		case 0x20: break;

		case 0x40: cpustate->dma[6].int_index = data; return;
		case 0x41: cpustate->dma[6].int_modifier = data; return;
		case 0x42: cpustate->dma[6].int_count = data; return;
		case 0x43: cpustate->dma[6].chain_ptr = data; return;
		case 0x44: cpustate->dma[6].gen_purpose = data; return;
		case 0x45: cpustate->dma[6].ext_index = data; return;
		case 0x46: cpustate->dma[6].ext_modifier = data; return;
		case 0x47: cpustate->dma[6].ext_count = data; return;

		// DMA 7
		case 0x1d:
		{
			cpustate->dma[7].control = data;
			sharc_iop_delayed_w(cpustate, 0x1d, data, 30);
			break;
		}

		case 0x48: cpustate->dma[7].int_index = data; return;
		case 0x49: cpustate->dma[7].int_modifier = data; return;
		case 0x4a: cpustate->dma[7].int_count = data; return;
		case 0x4b: cpustate->dma[7].chain_ptr = data; return;
		case 0x4c: cpustate->dma[7].gen_purpose = data; return;
		case 0x4d: cpustate->dma[7].ext_index = data; return;
		case 0x4e: cpustate->dma[7].ext_modifier = data; return;
		case 0x4f: cpustate->dma[7].ext_count = data; return;

		default:		fatalerror("sharc_iop_w: Unimplemented IOP reg %02X, %08X at %08X\n", address, data, cpustate->pc);
	}
}


#include "sharcmem.c"
#include "sharcdma.c"
#include "sharcops.c"
#include "sharcops.h"



static void build_opcode_table(void)
{
	int i, j;
	int num_ops = sizeof(sharc_opcode_table) / sizeof(SHARC_OP);

	for (i=0; i < 512; i++)
	{
		sharc_op[i] = sharcop_unimplemented;
	}

	for (i=0; i < 512; i++)
	{
		UINT16 op = i << 7;

		for (j=0; j < num_ops; j++)
		{
			if ((sharc_opcode_table[j].op_mask & op) == sharc_opcode_table[j].op_bits)
			{
				if (sharc_op[i] != sharcop_unimplemented)
				{
					fatalerror("build_opcode_table: table already filled! (i=%04X, j=%d)\n", i, j);
				}
				else
				{
					sharc_op[i] = sharc_opcode_table[j].handler;
				}
			}
		}
	}
}

/*****************************************************************************/

void sharc_external_iop_write(device_t *device, UINT32 address, UINT32 data)
{
	SHARC_REGS *cpustate = get_safe_token(device);
	if (address == 0x1c)
	{
		if (data != 0)
		{
			cpustate->dma[6].control = data;
		}
	}
	else
	{
		mame_printf_debug("SHARC IOP write %08X, %08X\n", address, data);
		sharc_iop_w(cpustate, address, data);
	}
}

void sharc_external_dma_write(device_t *device, UINT32 address, UINT64 data)
{
	SHARC_REGS *cpustate = get_safe_token(device);
	switch ((cpustate->dma[6].control >> 6) & 0x3)
	{
		case 2:			// 16/48 packing
		{
			int shift = address % 3;
			UINT64 r = pm_read48(cpustate, cpustate->dma[6].int_index);

			r &= ~((UINT64)(0xffff) << (shift*16));
			r |= (data & 0xffff) << (shift*16);

			pm_write48(cpustate, cpustate->dma[6].int_index, r);

			if (shift == 2)
			{

				cpustate->dma[6].int_index += cpustate->dma[6].int_modifier;
			}
			break;
		}
		default:
		{
			fatalerror("sharc_external_dma_write: unimplemented packing mode %d\n", (cpustate->dma[6].control >> 6) & 0x3);
		}
	}
}

static CPU_INIT( sharc )
{
	SHARC_REGS *cpustate = get_safe_token(device);
	const sharc_config *cfg = (const sharc_config *)device->static_config();
	int saveindex;

	cpustate->boot_mode = cfg->boot_mode;

	cpustate->irq_callback = irqcallback;
	cpustate->device = device;
	cpustate->program = device->space(AS_PROGRAM);
	cpustate->data = device->space(AS_DATA);

	build_opcode_table();

	cpustate->internal_ram = auto_alloc_array(device->machine(), UINT16, 2 * 0x10000);		// 2x 128KB
	cpustate->internal_ram_block0 = &cpustate->internal_ram[0];
	cpustate->internal_ram_block1 = &cpustate->internal_ram[0x20000/2];

	cpustate->delayed_iop_timer = device->machine().scheduler().timer_alloc(FUNC(sharc_iop_delayed_write_callback), cpustate);

	for (int i=0; i < 12; i++)
	{
		cpustate->dma_op[i].active = false;
		cpustate->dma_op[i].timer = device->machine().scheduler().timer_alloc(FUNC(sharc_dma_callback), cpustate);
	}

	device->save_item(NAME(cpustate->pc));
	device->save_pointer(NAME(&cpustate->r[0].r), ARRAY_LENGTH(cpustate->r));
	device->save_pointer(NAME(&cpustate->reg_alt[0].r), ARRAY_LENGTH(cpustate->reg_alt));
	device->save_item(NAME(cpustate->mrf));
	device->save_item(NAME(cpustate->mrb));

	device->save_item(NAME(cpustate->pcstack));
	device->save_item(NAME(cpustate->lcstack));
	device->save_item(NAME(cpustate->lastack));
	device->save_item(NAME(cpustate->lstkp));

	device->save_item(NAME(cpustate->faddr));
	device->save_item(NAME(cpustate->daddr));
	device->save_item(NAME(cpustate->pcstk));
	device->save_item(NAME(cpustate->pcstkp));
	device->save_item(NAME(cpustate->laddr.addr));
	device->save_item(NAME(cpustate->laddr.code));
	device->save_item(NAME(cpustate->laddr.loop_type));
	device->save_item(NAME(cpustate->curlcntr));
	device->save_item(NAME(cpustate->lcntr));

	device->save_item(NAME(cpustate->dag1.i));
	device->save_item(NAME(cpustate->dag1.m));
	device->save_item(NAME(cpustate->dag1.b));
	device->save_item(NAME(cpustate->dag1.l));
	device->save_item(NAME(cpustate->dag2.i));
	device->save_item(NAME(cpustate->dag2.m));
	device->save_item(NAME(cpustate->dag2.b));
	device->save_item(NAME(cpustate->dag2.l));
	device->save_item(NAME(cpustate->dag1_alt.i));
	device->save_item(NAME(cpustate->dag1_alt.m));
	device->save_item(NAME(cpustate->dag1_alt.b));
	device->save_item(NAME(cpustate->dag1_alt.l));
	device->save_item(NAME(cpustate->dag2_alt.i));
	device->save_item(NAME(cpustate->dag2_alt.m));
	device->save_item(NAME(cpustate->dag2_alt.b));
	device->save_item(NAME(cpustate->dag2_alt.l));

	for (saveindex = 0; saveindex < ARRAY_LENGTH(cpustate->dma); saveindex++)
	{
		device->save_item(NAME(cpustate->dma[saveindex].control), saveindex);
		device->save_item(NAME(cpustate->dma[saveindex].int_index), saveindex);
		device->save_item(NAME(cpustate->dma[saveindex].int_modifier), saveindex);
		device->save_item(NAME(cpustate->dma[saveindex].int_count), saveindex);
		device->save_item(NAME(cpustate->dma[saveindex].chain_ptr), saveindex);
		device->save_item(NAME(cpustate->dma[saveindex].gen_purpose), saveindex);
		device->save_item(NAME(cpustate->dma[saveindex].ext_index), saveindex);
		device->save_item(NAME(cpustate->dma[saveindex].ext_modifier), saveindex);
		device->save_item(NAME(cpustate->dma[saveindex].ext_count), saveindex);
	}

	device->save_item(NAME(cpustate->mode1));
	device->save_item(NAME(cpustate->mode2));
	device->save_item(NAME(cpustate->astat));
	device->save_item(NAME(cpustate->stky));
	device->save_item(NAME(cpustate->irptl));
	device->save_item(NAME(cpustate->imask));
	device->save_item(NAME(cpustate->imaskp));
	device->save_item(NAME(cpustate->ustat1));
	device->save_item(NAME(cpustate->ustat2));

	device->save_item(NAME(cpustate->flag));

	device->save_item(NAME(cpustate->syscon));
	device->save_item(NAME(cpustate->sysstat));

	for (saveindex = 0; saveindex < ARRAY_LENGTH(cpustate->status_stack); saveindex++)
	{
		device->save_item(NAME(cpustate->status_stack[saveindex].mode1), saveindex);
		device->save_item(NAME(cpustate->status_stack[saveindex].astat), saveindex);
	}
	device->save_item(NAME(cpustate->status_stkp));

	device->save_item(NAME(cpustate->px));

	device->save_pointer(NAME(cpustate->internal_ram), 2 * 0x10000);

	device->save_item(NAME(cpustate->opcode));

	device->save_item(NAME(cpustate->nfaddr));

	device->save_item(NAME(cpustate->idle));
	device->save_item(NAME(cpustate->irq_active));
	device->save_item(NAME(cpustate->active_irq_num));

	for (saveindex = 0; saveindex < ARRAY_LENGTH(cpustate->dma_op); saveindex++)
	{
		device->save_item(NAME(cpustate->dma_op[saveindex].src), saveindex);
		device->save_item(NAME(cpustate->dma_op[saveindex].dst), saveindex);
		device->save_item(NAME(cpustate->dma_op[saveindex].chain_ptr), saveindex);
		device->save_item(NAME(cpustate->dma_op[saveindex].src_modifier), saveindex);
		device->save_item(NAME(cpustate->dma_op[saveindex].dst_modifier), saveindex);
		device->save_item(NAME(cpustate->dma_op[saveindex].src_count), saveindex);
		device->save_item(NAME(cpustate->dma_op[saveindex].dst_count), saveindex);
		device->save_item(NAME(cpustate->dma_op[saveindex].pmode), saveindex);
		device->save_item(NAME(cpustate->dma_op[saveindex].chained_direction), saveindex);
		device->save_item(NAME(cpustate->dma_op[saveindex].active), saveindex);
	}

	device->save_item(NAME(cpustate->dma_status));

	device->save_item(NAME(cpustate->interrupt_active));

	device->save_item(NAME(cpustate->iop_delayed_reg));
	device->save_item(NAME(cpustate->iop_delayed_data));

	device->save_item(NAME(cpustate->delay_slot1));
	device->save_item(NAME(cpustate->delay_slot2));

	device->save_item(NAME(cpustate->systemreg_latency_cycles));
	device->save_item(NAME(cpustate->systemreg_latency_reg));
	device->save_item(NAME(cpustate->systemreg_latency_data));
	device->save_item(NAME(cpustate->systemreg_previous_data));

	device->save_item(NAME(cpustate->astat_old));
	device->save_item(NAME(cpustate->astat_old_old));
	device->save_item(NAME(cpustate->astat_old_old_old));
}

static CPU_RESET( sharc )
{
	SHARC_REGS *cpustate = get_safe_token(device);
	memset(cpustate->internal_ram, 0, 2 * 0x10000 * sizeof(UINT16));

	switch(cpustate->boot_mode)
	{
		case BOOT_MODE_EPROM:
		{
			cpustate->dma[6].int_index		= 0x20000;
			cpustate->dma[6].int_modifier	= 1;
			cpustate->dma[6].int_count		= 0x100;
			cpustate->dma[6].ext_index		= 0x400000;
			cpustate->dma[6].ext_modifier	= 1;
			cpustate->dma[6].ext_count		= 0x600;
			cpustate->dma[6].control		= 0x2a1;

			sharc_dma_exec(cpustate, 6);
			dma_op(cpustate, 6);
			
			cpustate->dma_op[6].timer->adjust(attotime::never, 0);
			break;
		}

		case BOOT_MODE_HOST:
			break;

		default:
			fatalerror("SHARC: Unimplemented boot mode %d\n", cpustate->boot_mode);
	}

	cpustate->pc = 0x20004;
	cpustate->daddr = cpustate->pc + 1;
	cpustate->faddr = cpustate->daddr + 1;
	cpustate->nfaddr = cpustate->faddr+1;

	cpustate->idle = 0;
	cpustate->stky = 0x5400000;

	cpustate->interrupt_active = 0;
}

static CPU_EXIT( sharc )
{
	/* TODO */
}

static void sharc_set_irq_line(SHARC_REGS *cpustate, int irqline, int state)
{
	if (state == ASSERT_LINE)
	{
		cpustate->irq_active |= 1 << (8-irqline);
	}
	else
	{
		cpustate->irq_active &= ~(1 << (8-irqline));
	}
}

void sharc_set_flag_input(device_t *device, int flag_num, int state)
{
	SHARC_REGS *cpustate = get_safe_token(device);
	if (flag_num >= 0 && flag_num < 4)
	{
		// Check if flag is set to input in MODE2 (bit == 0)
		if ((cpustate->mode2 & (1 << (flag_num+15))) == 0)
		{
			cpustate->flag[flag_num] = state ? 1 : 0;
		}
		else
		{
			fatalerror("sharc_set_flag_input: flag %d is set output!\n", flag_num);
		}
	}
}

static void check_interrupts(SHARC_REGS *cpustate)
{
	int i;
	if ((cpustate->imask & cpustate->irq_active) && (cpustate->mode1 & MODE1_IRPTEN) && !cpustate->interrupt_active &&
		cpustate->pc != cpustate->delay_slot1 && cpustate->pc != cpustate->delay_slot2)
	{
		int which = 0;
		for (i=0; i < 32; i++)
		{
			if (cpustate->irq_active & (1 << i))
			{
				break;
			}
			which++;
		}

		if (cpustate->idle)
		{
			PUSH_PC(cpustate, cpustate->pc+1);
		}
		else
		{
			PUSH_PC(cpustate, cpustate->daddr);
		}

		cpustate->irptl |= 1 << which;

		if (which >= 6 && which <= 8)
		{
			PUSH_STATUS_STACK(cpustate);
		}

		CHANGE_PC(cpustate, 0x20000 + (which * 0x4));

		/* TODO: alter IMASKP */

		cpustate->active_irq_num = which;
		cpustate->irq_active &= ~(1 << which);

		cpustate->interrupt_active = 1;
	}
}

static CPU_EXECUTE( sharc )
{
	SHARC_REGS *cpustate = get_safe_token(device);

	if (cpustate->idle && cpustate->irq_active == 0)
	{
		cpustate->icount = 0;
		debugger_instruction_hook(device, cpustate->daddr);
	}
	if (cpustate->irq_active != 0)
	{
		check_interrupts(cpustate);
		cpustate->idle = 0;
	}

	while (cpustate->icount > 0 && !cpustate->idle)
	{
		cpustate->pc = cpustate->daddr;
		cpustate->daddr = cpustate->faddr;
		cpustate->faddr = cpustate->nfaddr;
		cpustate->nfaddr++;

		cpustate->astat_old_old_old = cpustate->astat_old_old;
		cpustate->astat_old_old = cpustate->astat_old;
		cpustate->astat_old = cpustate->astat;

		cpustate->opcode = ROPCODE(cpustate->pc);

		debugger_instruction_hook(device, cpustate->pc);

		// handle looping
		if (cpustate->pc == cpustate->laddr.addr)
		{
			switch (cpustate->laddr.loop_type)
			{
				case 0:		// arithmetic condition-based
				{
					int condition = cpustate->laddr.code;

					{
						UINT32 looptop = TOP_PC(cpustate);
						if (cpustate->pc - looptop > 2)
						{
							cpustate->astat = cpustate->astat_old_old_old;
						}
					}

					if (DO_CONDITION_CODE(cpustate, condition))
					{
						POP_LOOP(cpustate);
						POP_PC(cpustate);
					}
					else
					{
						CHANGE_PC(cpustate, TOP_PC(cpustate));
					}

					cpustate->astat = cpustate->astat_old;
					break;
				}
				case 1:		// counter-based, length 1
				{
					//fatalerror("SHARC: counter-based loop, length 1 at %08X\n", cpustate->pc);
					//break;
				}
				case 2:		// counter-based, length 2
				{
					//fatalerror("SHARC: counter-based loop, length 2 at %08X\n", cpustate->pc);
					//break;
				}
				case 3:		// counter-based, length >2
				{
					--cpustate->lcstack[cpustate->lstkp];
					--cpustate->curlcntr;
					if (cpustate->curlcntr == 0)
					{
						POP_LOOP(cpustate);
						POP_PC(cpustate);
					}
					else
					{
						CHANGE_PC(cpustate, TOP_PC(cpustate));
					}
				}
			}
		}
		
		sharc_op[(cpustate->opcode >> 39) & 0x1ff](cpustate);




		// System register latency effect
		if (cpustate->systemreg_latency_cycles > 0)
		{
			--cpustate->systemreg_latency_cycles;
			if (cpustate->systemreg_latency_cycles <= 0)
			{
				systemreg_write_latency_effect(cpustate);
			}
		}

		--cpustate->icount;
	};
}

/**************************************************************************
 * Generic set_info
 **************************************************************************/

static CPU_SET_INFO( sharc )
{
	SHARC_REGS *cpustate = get_safe_token(device);

	switch (state)
	{
		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + SHARC_PC:			cpustate->pc = info->i;						break;
		case CPUINFO_INT_REGISTER + SHARC_FADDR:		cpustate->faddr = info->i;					break;
		case CPUINFO_INT_REGISTER + SHARC_DADDR:		cpustate->daddr = info->i;					break;

		case CPUINFO_INT_REGISTER + SHARC_R0:			cpustate->r[0].r = info->i;					break;
		case CPUINFO_INT_REGISTER + SHARC_R1:			cpustate->r[1].r = info->i;					break;
		case CPUINFO_INT_REGISTER + SHARC_R2:			cpustate->r[2].r = info->i;					break;
		case CPUINFO_INT_REGISTER + SHARC_R3:			cpustate->r[3].r = info->i;					break;
		case CPUINFO_INT_REGISTER + SHARC_R4:			cpustate->r[4].r = info->i;					break;
		case CPUINFO_INT_REGISTER + SHARC_R5:			cpustate->r[5].r = info->i;					break;
		case CPUINFO_INT_REGISTER + SHARC_R6:			cpustate->r[6].r = info->i;					break;
		case CPUINFO_INT_REGISTER + SHARC_R7:			cpustate->r[7].r = info->i;					break;
		case CPUINFO_INT_REGISTER + SHARC_R8:			cpustate->r[8].r = info->i;					break;
		case CPUINFO_INT_REGISTER + SHARC_R9:			cpustate->r[9].r = info->i;					break;
		case CPUINFO_INT_REGISTER + SHARC_R10:			cpustate->r[10].r = info->i;				break;
		case CPUINFO_INT_REGISTER + SHARC_R11:			cpustate->r[11].r = info->i;				break;
		case CPUINFO_INT_REGISTER + SHARC_R12:			cpustate->r[12].r = info->i;				break;
		case CPUINFO_INT_REGISTER + SHARC_R13:			cpustate->r[13].r = info->i;				break;
		case CPUINFO_INT_REGISTER + SHARC_R14:			cpustate->r[14].r = info->i;				break;
		case CPUINFO_INT_REGISTER + SHARC_R15:			cpustate->r[15].r = info->i;				break;

		case CPUINFO_INT_REGISTER + SHARC_I0:			cpustate->dag1.i[0] = info->i;				break;
		case CPUINFO_INT_REGISTER + SHARC_I1:			cpustate->dag1.i[1] = info->i;				break;
		case CPUINFO_INT_REGISTER + SHARC_I2:			cpustate->dag1.i[2] = info->i;				break;
		case CPUINFO_INT_REGISTER + SHARC_I3:			cpustate->dag1.i[3] = info->i;				break;
		case CPUINFO_INT_REGISTER + SHARC_I4:			cpustate->dag1.i[4] = info->i;				break;
		case CPUINFO_INT_REGISTER + SHARC_I5:			cpustate->dag1.i[5] = info->i;				break;
		case CPUINFO_INT_REGISTER + SHARC_I6:			cpustate->dag1.i[6] = info->i;				break;
		case CPUINFO_INT_REGISTER + SHARC_I7:			cpustate->dag1.i[7] = info->i;				break;
		case CPUINFO_INT_REGISTER + SHARC_I8:			cpustate->dag2.i[0] = info->i;				break;
		case CPUINFO_INT_REGISTER + SHARC_I9:			cpustate->dag2.i[1] = info->i;				break;
		case CPUINFO_INT_REGISTER + SHARC_I10:			cpustate->dag2.i[2] = info->i;				break;
		case CPUINFO_INT_REGISTER + SHARC_I11:			cpustate->dag2.i[3] = info->i;				break;
		case CPUINFO_INT_REGISTER + SHARC_I12:			cpustate->dag2.i[4] = info->i;				break;
		case CPUINFO_INT_REGISTER + SHARC_I13:			cpustate->dag2.i[5] = info->i;				break;
		case CPUINFO_INT_REGISTER + SHARC_I14:			cpustate->dag2.i[6] = info->i;				break;
		case CPUINFO_INT_REGISTER + SHARC_I15:			cpustate->dag2.i[7] = info->i;				break;

		case CPUINFO_INT_REGISTER + SHARC_M0:			cpustate->dag1.m[0] = info->i;				break;
		case CPUINFO_INT_REGISTER + SHARC_M1:			cpustate->dag1.m[1] = info->i;				break;
		case CPUINFO_INT_REGISTER + SHARC_M2:			cpustate->dag1.m[2] = info->i;				break;
		case CPUINFO_INT_REGISTER + SHARC_M3:			cpustate->dag1.m[3] = info->i;				break;
		case CPUINFO_INT_REGISTER + SHARC_M4:			cpustate->dag1.m[4] = info->i;				break;
		case CPUINFO_INT_REGISTER + SHARC_M5:			cpustate->dag1.m[5] = info->i;				break;
		case CPUINFO_INT_REGISTER + SHARC_M6:			cpustate->dag1.m[6] = info->i;				break;
		case CPUINFO_INT_REGISTER + SHARC_M7:			cpustate->dag1.m[7] = info->i;				break;
		case CPUINFO_INT_REGISTER + SHARC_M8:			cpustate->dag2.m[0] = info->i;				break;
		case CPUINFO_INT_REGISTER + SHARC_M9:			cpustate->dag2.m[1] = info->i;				break;
		case CPUINFO_INT_REGISTER + SHARC_M10:			cpustate->dag2.m[2] = info->i;				break;
		case CPUINFO_INT_REGISTER + SHARC_M11:			cpustate->dag2.m[3] = info->i;				break;
		case CPUINFO_INT_REGISTER + SHARC_M12:			cpustate->dag2.m[4] = info->i;				break;
		case CPUINFO_INT_REGISTER + SHARC_M13:			cpustate->dag2.m[5] = info->i;				break;
		case CPUINFO_INT_REGISTER + SHARC_M14:			cpustate->dag2.m[6] = info->i;				break;
		case CPUINFO_INT_REGISTER + SHARC_M15:			cpustate->dag2.m[7] = info->i;				break;

		case CPUINFO_INT_REGISTER + SHARC_L0:			cpustate->dag1.l[0] = info->i;				break;
		case CPUINFO_INT_REGISTER + SHARC_L1:			cpustate->dag1.l[1] = info->i;				break;
		case CPUINFO_INT_REGISTER + SHARC_L2:			cpustate->dag1.l[2] = info->i;				break;
		case CPUINFO_INT_REGISTER + SHARC_L3:			cpustate->dag1.l[3] = info->i;				break;
		case CPUINFO_INT_REGISTER + SHARC_L4:			cpustate->dag1.l[4] = info->i;				break;
		case CPUINFO_INT_REGISTER + SHARC_L5:			cpustate->dag1.l[5] = info->i;				break;
		case CPUINFO_INT_REGISTER + SHARC_L6:			cpustate->dag1.l[6] = info->i;				break;
		case CPUINFO_INT_REGISTER + SHARC_L7:			cpustate->dag1.l[7] = info->i;				break;
		case CPUINFO_INT_REGISTER + SHARC_L8:			cpustate->dag2.l[0] = info->i;				break;
		case CPUINFO_INT_REGISTER + SHARC_L9:			cpustate->dag2.l[1] = info->i;				break;
		case CPUINFO_INT_REGISTER + SHARC_L10:			cpustate->dag2.l[2] = info->i;				break;
		case CPUINFO_INT_REGISTER + SHARC_L11:			cpustate->dag2.l[3] = info->i;				break;
		case CPUINFO_INT_REGISTER + SHARC_L12:			cpustate->dag2.l[4] = info->i;				break;
		case CPUINFO_INT_REGISTER + SHARC_L13:			cpustate->dag2.l[5] = info->i;				break;
		case CPUINFO_INT_REGISTER + SHARC_L14:			cpustate->dag2.l[6] = info->i;				break;
		case CPUINFO_INT_REGISTER + SHARC_L15:			cpustate->dag2.m[7] = info->i;				break;

		case CPUINFO_INT_REGISTER + SHARC_B0:			cpustate->dag1.b[0] = info->i;				break;
		case CPUINFO_INT_REGISTER + SHARC_B1:			cpustate->dag1.b[1] = info->i;				break;
		case CPUINFO_INT_REGISTER + SHARC_B2:			cpustate->dag1.b[2] = info->i;				break;
		case CPUINFO_INT_REGISTER + SHARC_B3:			cpustate->dag1.b[3] = info->i;				break;
		case CPUINFO_INT_REGISTER + SHARC_B4:			cpustate->dag1.b[4] = info->i;				break;
		case CPUINFO_INT_REGISTER + SHARC_B5:			cpustate->dag1.b[5] = info->i;				break;
		case CPUINFO_INT_REGISTER + SHARC_B6:			cpustate->dag1.b[6] = info->i;				break;
		case CPUINFO_INT_REGISTER + SHARC_B7:			cpustate->dag1.b[7] = info->i;				break;
		case CPUINFO_INT_REGISTER + SHARC_B8:			cpustate->dag2.b[0] = info->i;				break;
		case CPUINFO_INT_REGISTER + SHARC_B9:			cpustate->dag2.b[1] = info->i;				break;
		case CPUINFO_INT_REGISTER + SHARC_B10:			cpustate->dag2.b[2] = info->i;				break;
		case CPUINFO_INT_REGISTER + SHARC_B11:			cpustate->dag2.b[3] = info->i;				break;
		case CPUINFO_INT_REGISTER + SHARC_B12:			cpustate->dag2.b[4] = info->i;				break;
		case CPUINFO_INT_REGISTER + SHARC_B13:			cpustate->dag2.b[5] = info->i;				break;
		case CPUINFO_INT_REGISTER + SHARC_B14:			cpustate->dag2.b[6] = info->i;				break;
		case CPUINFO_INT_REGISTER + SHARC_B15:			cpustate->dag2.b[7] = info->i;				break;
	}
}

static CPU_SET_INFO( adsp21062 )
{
	if (state >= CPUINFO_INT_INPUT_STATE && state <= CPUINFO_INT_INPUT_STATE + 2)
	{
		sharc_set_irq_line(get_safe_token(device), state-CPUINFO_INT_INPUT_STATE, info->i);
		return;
	}
	else if (state >= CPUINFO_INT_INPUT_STATE + SHARC_INPUT_FLAG0 && state <= CPUINFO_INT_INPUT_STATE + SHARC_INPUT_FLAG3)
	{
		sharc_set_flag_input(device, state-(CPUINFO_INT_INPUT_STATE + SHARC_INPUT_FLAG0), info->i);
		return;
	}
	switch(state)
	{
		default:	CPU_SET_INFO_CALL(sharc);		break;
	}
}


static CPU_READ( sharc )
{
	SHARC_REGS *cpustate = get_safe_token(device);
	if (space == AS_PROGRAM)
	{
		int address = offset >> 3;

		if (address >= 0x20000 && address < 0x30000)
		{
			switch (size)
			{
				case 1:
				{
					int frac = offset & 7;
					*value = (pm_read48(cpustate, offset >> 3) >> ((frac^7) * 8)) & 0xff;
					break;
				}
				case 8:
				{
					*value = pm_read48(cpustate, offset >> 3);
					break;
				}
			}
		}
		else
		{
			*value = 0;
		}
	}
	else if (space == AS_DATA)
	{
		int address = offset >> 2;
		if (address >= 0x20000)
		{
			switch (size)
			{
				case 1:
				{
					int frac = offset & 3;
					*value = (dm_read32(cpustate, offset >> 2) >> ((frac^3) * 8)) & 0xff;
					break;
				}
				case 2:
				{
					int frac = (offset >> 1) & 1;
					*value = (dm_read32(cpustate, offset >> 2) >> ((frac^1) * 16)) & 0xffff;
					break;
				}
				case 4:
				{
					*value = dm_read32(cpustate, offset >> 2);
					break;
				}
			}
		}
		else
		{
			*value = 0;
		}
	}
	return 1;
}

static CPU_READOP( sharc )
{
	SHARC_REGS *cpustate = get_safe_token(device);
	UINT64 mask = (size < 8) ? (((UINT64)1 << (8 * size)) - 1) : ~(UINT64)0;
	int shift = 8 * (offset & 7);
	offset >>= 3;

	if (offset >= 0x20000 && offset < 0x28000)
	{
		UINT64 op = ((UINT64)(cpustate->internal_ram_block0[((offset-0x20000) * 3) + 0]) << 32) |
					((UINT64)(cpustate->internal_ram_block0[((offset-0x20000) * 3) + 1]) << 16) |
					((UINT64)(cpustate->internal_ram_block0[((offset-0x20000) * 3) + 2]) << 0);
		*value = (op >> shift) & mask;
	}
	else if (offset >= 0x28000 && offset < 0x30000)
	{
		UINT64 op = ((UINT64)(cpustate->internal_ram_block1[((offset-0x28000) * 3) + 0]) << 32) |
					((UINT64)(cpustate->internal_ram_block1[((offset-0x28000) * 3) + 1]) << 16) |
					((UINT64)(cpustate->internal_ram_block1[((offset-0x28000) * 3) + 2]) << 0);
		*value = (op >> shift) & mask;
	}

	return 1;
}

// This is just used to stop the debugger from complaining about executing from I/O space
static ADDRESS_MAP_START( internal_pgm, AS_PROGRAM, 64, adsp21062_device )
	AM_RANGE(0x20000, 0x7ffff) AM_RAM
ADDRESS_MAP_END

static CPU_GET_INFO( sharc )
{
	SHARC_REGS *cpustate = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;

	switch(state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(SHARC_REGS);				break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 32;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_LITTLE;					break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 8;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 8;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 40;							break;

		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 64;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM: info->i = 24;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM: info->i = -3;					break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_DATA:	info->i = 32;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:	info->i = 32;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_DATA:	info->i = -2;					break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_IO:		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_IO:		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_IO:		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE:					info->i = CLEAR_LINE;					break;

		case CPUINFO_INT_PREVIOUSPC:					/* not implemented */					break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + SHARC_PC:			info->i = cpustate->pc;						break;
		case CPUINFO_INT_REGISTER + SHARC_PCSTK:		info->i = cpustate->pcstk;					break;
		case CPUINFO_INT_REGISTER + SHARC_PCSTKP:		info->i = cpustate->pcstkp;					break;
		case CPUINFO_INT_REGISTER + SHARC_LSTKP:		info->i = cpustate->lstkp;					break;
		case CPUINFO_INT_REGISTER + SHARC_FADDR:		info->i = cpustate->faddr;					break;
		case CPUINFO_INT_REGISTER + SHARC_DADDR:		info->i = cpustate->daddr;					break;
		case CPUINFO_INT_REGISTER + SHARC_MODE1:		info->i = cpustate->mode1;					break;
		case CPUINFO_INT_REGISTER + SHARC_MODE2:		info->i = cpustate->mode2;					break;
		case CPUINFO_INT_REGISTER + SHARC_ASTAT:		info->i = cpustate->astat;					break;
		case CPUINFO_INT_REGISTER + SHARC_IRPTL:		info->i = cpustate->irptl;					break;
		case CPUINFO_INT_REGISTER + SHARC_IMASK:		info->i = cpustate->imask;					break;
		case CPUINFO_INT_REGISTER + SHARC_USTAT1:		info->i = cpustate->ustat1;					break;
		case CPUINFO_INT_REGISTER + SHARC_USTAT2:		info->i = cpustate->ustat2;					break;
		case CPUINFO_INT_REGISTER + SHARC_STSTKP:		info->i = cpustate->status_stkp;			break;

		case CPUINFO_INT_REGISTER + SHARC_R0:			info->i = cpustate->r[0].r;					break;
		case CPUINFO_INT_REGISTER + SHARC_R1:			info->i = cpustate->r[1].r;					break;
		case CPUINFO_INT_REGISTER + SHARC_R2:			info->i = cpustate->r[2].r;					break;
		case CPUINFO_INT_REGISTER + SHARC_R3:			info->i = cpustate->r[3].r;					break;
		case CPUINFO_INT_REGISTER + SHARC_R4:			info->i = cpustate->r[4].r;					break;
		case CPUINFO_INT_REGISTER + SHARC_R5:			info->i = cpustate->r[5].r;					break;
		case CPUINFO_INT_REGISTER + SHARC_R6:			info->i = cpustate->r[6].r;					break;
		case CPUINFO_INT_REGISTER + SHARC_R7:			info->i = cpustate->r[7].r;					break;
		case CPUINFO_INT_REGISTER + SHARC_R8:			info->i = cpustate->r[8].r;					break;
		case CPUINFO_INT_REGISTER + SHARC_R9:			info->i = cpustate->r[9].r;					break;
		case CPUINFO_INT_REGISTER + SHARC_R10:			info->i = cpustate->r[10].r;				break;
		case CPUINFO_INT_REGISTER + SHARC_R11:			info->i = cpustate->r[11].r;				break;
		case CPUINFO_INT_REGISTER + SHARC_R12:			info->i = cpustate->r[12].r;				break;
		case CPUINFO_INT_REGISTER + SHARC_R13:			info->i = cpustate->r[13].r;				break;
		case CPUINFO_INT_REGISTER + SHARC_R14:			info->i = cpustate->r[14].r;				break;
		case CPUINFO_INT_REGISTER + SHARC_R15:			info->i = cpustate->r[15].r;				break;

		case CPUINFO_INT_REGISTER + SHARC_I0:			info->i = cpustate->dag1.i[0];				break;
		case CPUINFO_INT_REGISTER + SHARC_I1:			info->i = cpustate->dag1.i[1];				break;
		case CPUINFO_INT_REGISTER + SHARC_I2:			info->i = cpustate->dag1.i[2];				break;
		case CPUINFO_INT_REGISTER + SHARC_I3:			info->i = cpustate->dag1.i[3];				break;
		case CPUINFO_INT_REGISTER + SHARC_I4:			info->i = cpustate->dag1.i[4];				break;
		case CPUINFO_INT_REGISTER + SHARC_I5:			info->i = cpustate->dag1.i[5];				break;
		case CPUINFO_INT_REGISTER + SHARC_I6:			info->i = cpustate->dag1.i[6];				break;
		case CPUINFO_INT_REGISTER + SHARC_I7:			info->i = cpustate->dag1.i[7];				break;
		case CPUINFO_INT_REGISTER + SHARC_I8:			info->i = cpustate->dag2.i[0];				break;
		case CPUINFO_INT_REGISTER + SHARC_I9:			info->i = cpustate->dag2.i[1];				break;
		case CPUINFO_INT_REGISTER + SHARC_I10:			info->i = cpustate->dag2.i[2];				break;
		case CPUINFO_INT_REGISTER + SHARC_I11:			info->i = cpustate->dag2.i[3];				break;
		case CPUINFO_INT_REGISTER + SHARC_I12:			info->i = cpustate->dag2.i[4];				break;
		case CPUINFO_INT_REGISTER + SHARC_I13:			info->i = cpustate->dag2.i[5];				break;
		case CPUINFO_INT_REGISTER + SHARC_I14:			info->i = cpustate->dag2.i[6];				break;
		case CPUINFO_INT_REGISTER + SHARC_I15:			info->i = cpustate->dag2.i[7];				break;

		case CPUINFO_INT_REGISTER + SHARC_M0:			info->i = cpustate->dag1.m[0];				break;
		case CPUINFO_INT_REGISTER + SHARC_M1:			info->i = cpustate->dag1.m[1];				break;
		case CPUINFO_INT_REGISTER + SHARC_M2:			info->i = cpustate->dag1.m[2];				break;
		case CPUINFO_INT_REGISTER + SHARC_M3:			info->i = cpustate->dag1.m[3];				break;
		case CPUINFO_INT_REGISTER + SHARC_M4:			info->i = cpustate->dag1.m[4];				break;
		case CPUINFO_INT_REGISTER + SHARC_M5:			info->i = cpustate->dag1.m[5];				break;
		case CPUINFO_INT_REGISTER + SHARC_M6:			info->i = cpustate->dag1.m[6];				break;
		case CPUINFO_INT_REGISTER + SHARC_M7:			info->i = cpustate->dag1.m[7];				break;
		case CPUINFO_INT_REGISTER + SHARC_M8:			info->i = cpustate->dag2.m[0];				break;
		case CPUINFO_INT_REGISTER + SHARC_M9:			info->i = cpustate->dag2.m[1];				break;
		case CPUINFO_INT_REGISTER + SHARC_M10:			info->i = cpustate->dag2.m[2];				break;
		case CPUINFO_INT_REGISTER + SHARC_M11:			info->i = cpustate->dag2.m[3];				break;
		case CPUINFO_INT_REGISTER + SHARC_M12:			info->i = cpustate->dag2.m[4];				break;
		case CPUINFO_INT_REGISTER + SHARC_M13:			info->i = cpustate->dag2.m[5];				break;
		case CPUINFO_INT_REGISTER + SHARC_M14:			info->i = cpustate->dag2.m[6];				break;
		case CPUINFO_INT_REGISTER + SHARC_M15:			info->i = cpustate->dag2.m[7];				break;

		case CPUINFO_INT_REGISTER + SHARC_L0:			info->i = cpustate->dag1.l[0];				break;
		case CPUINFO_INT_REGISTER + SHARC_L1:			info->i = cpustate->dag1.l[1];				break;
		case CPUINFO_INT_REGISTER + SHARC_L2:			info->i = cpustate->dag1.l[2];				break;
		case CPUINFO_INT_REGISTER + SHARC_L3:			info->i = cpustate->dag1.l[3];				break;
		case CPUINFO_INT_REGISTER + SHARC_L4:			info->i = cpustate->dag1.l[4];				break;
		case CPUINFO_INT_REGISTER + SHARC_L5:			info->i = cpustate->dag1.l[5];				break;
		case CPUINFO_INT_REGISTER + SHARC_L6:			info->i = cpustate->dag1.l[6];				break;
		case CPUINFO_INT_REGISTER + SHARC_L7:			info->i = cpustate->dag1.l[7];				break;
		case CPUINFO_INT_REGISTER + SHARC_L8:			info->i = cpustate->dag2.l[0];				break;
		case CPUINFO_INT_REGISTER + SHARC_L9:			info->i = cpustate->dag2.l[1];				break;
		case CPUINFO_INT_REGISTER + SHARC_L10:			info->i = cpustate->dag2.l[2];				break;
		case CPUINFO_INT_REGISTER + SHARC_L11:			info->i = cpustate->dag2.l[3];				break;
		case CPUINFO_INT_REGISTER + SHARC_L12:			info->i = cpustate->dag2.l[4];				break;
		case CPUINFO_INT_REGISTER + SHARC_L13:			info->i = cpustate->dag2.l[5];				break;
		case CPUINFO_INT_REGISTER + SHARC_L14:			info->i = cpustate->dag2.l[6];				break;
		case CPUINFO_INT_REGISTER + SHARC_L15:			info->i = cpustate->dag2.l[7];				break;

		case CPUINFO_INT_REGISTER + SHARC_B0:			info->i = cpustate->dag1.b[0];				break;
		case CPUINFO_INT_REGISTER + SHARC_B1:			info->i = cpustate->dag1.b[1];				break;
		case CPUINFO_INT_REGISTER + SHARC_B2:			info->i = cpustate->dag1.b[2];				break;
		case CPUINFO_INT_REGISTER + SHARC_B3:			info->i = cpustate->dag1.b[3];				break;
		case CPUINFO_INT_REGISTER + SHARC_B4:			info->i = cpustate->dag1.b[4];				break;
		case CPUINFO_INT_REGISTER + SHARC_B5:			info->i = cpustate->dag1.b[5];				break;
		case CPUINFO_INT_REGISTER + SHARC_B6:			info->i = cpustate->dag1.b[6];				break;
		case CPUINFO_INT_REGISTER + SHARC_B7:			info->i = cpustate->dag1.b[7];				break;
		case CPUINFO_INT_REGISTER + SHARC_B8:			info->i = cpustate->dag2.b[0];				break;
		case CPUINFO_INT_REGISTER + SHARC_B9:			info->i = cpustate->dag2.b[1];				break;
		case CPUINFO_INT_REGISTER + SHARC_B10:			info->i = cpustate->dag2.b[2];				break;
		case CPUINFO_INT_REGISTER + SHARC_B11:			info->i = cpustate->dag2.b[3];				break;
		case CPUINFO_INT_REGISTER + SHARC_B12:			info->i = cpustate->dag2.b[4];				break;
		case CPUINFO_INT_REGISTER + SHARC_B13:			info->i = cpustate->dag2.b[5];				break;
		case CPUINFO_INT_REGISTER + SHARC_B14:			info->i = cpustate->dag2.b[6];				break;
		case CPUINFO_INT_REGISTER + SHARC_B15:			info->i = cpustate->dag2.b[7];				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(sharc);		break;
		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(sharc);	break;
		case CPUINFO_FCT_EXIT:							info->exit = CPU_EXIT_NAME(sharc);		break;
		case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(sharc);break;
		case CPUINFO_FCT_BURN:							info->burn = NULL;						break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(sharc);			break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &cpustate->icount;			break;
		case CPUINFO_FCT_READ:							info->read = CPU_READ_NAME(sharc);		break;
		case CPUINFO_FCT_READOP:						info->readop = CPU_READOP_NAME(sharc);	break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM: info->internal_map64 = ADDRESS_MAP_NAME(internal_pgm); break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_FAMILY:					strcpy(info->s, "SHARC");				break;
		case CPUINFO_STR_VERSION:					strcpy(info->s, "2.01");				break;
		case CPUINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CREDITS:					strcpy(info->s, "Copyright Ville Linde"); break;

		case CPUINFO_STR_FLAGS:							strcpy(info->s, " ");					break;

		case CPUINFO_STR_REGISTER + SHARC_PC:			sprintf(info->s, "PC: %08X", cpustate->pc); break;
		case CPUINFO_STR_REGISTER + SHARC_PCSTK:		sprintf(info->s, "PCSTK: %08X", cpustate->pcstk); break;
		case CPUINFO_STR_REGISTER + SHARC_PCSTKP:		sprintf(info->s, "PCSTKP: %08X", cpustate->pcstkp); break;
		case CPUINFO_STR_REGISTER + SHARC_LSTKP:		sprintf(info->s, "LSTKP: %08X", cpustate->lstkp); break;
		case CPUINFO_STR_REGISTER + SHARC_FADDR:		sprintf(info->s, "FADDR: %08X", cpustate->faddr); break;
		case CPUINFO_STR_REGISTER + SHARC_DADDR:		sprintf(info->s, "DADDR: %08X", cpustate->daddr); break;
		case CPUINFO_STR_REGISTER + SHARC_MODE1:		sprintf(info->s, "MODE1: %08X", cpustate->mode1); break;
		case CPUINFO_STR_REGISTER + SHARC_MODE2:		sprintf(info->s, "MODE2: %08X", cpustate->mode2); break;
		case CPUINFO_STR_REGISTER + SHARC_ASTAT:		sprintf(info->s, "ASTAT: %08X", cpustate->astat); break;
		case CPUINFO_STR_REGISTER + SHARC_IRPTL:		sprintf(info->s, "IRPTL: %08X", cpustate->irptl); break;
		case CPUINFO_STR_REGISTER + SHARC_IMASK:		sprintf(info->s, "IMASK: %08X", cpustate->imask); break;
		case CPUINFO_STR_REGISTER + SHARC_USTAT1:		sprintf(info->s, "USTAT1: %08X", cpustate->ustat1); break;
		case CPUINFO_STR_REGISTER + SHARC_USTAT2:		sprintf(info->s, "USTAT2: %08X", cpustate->ustat2); break;
		case CPUINFO_STR_REGISTER + SHARC_STSTKP:		sprintf(info->s, "STSTKP: %08X", cpustate->status_stkp); break;

		case CPUINFO_STR_REGISTER + SHARC_R0:			sprintf(info->s, "R0: %08X", (UINT32)cpustate->r[0].r); break;
		case CPUINFO_STR_REGISTER + SHARC_R1:			sprintf(info->s, "R1: %08X", (UINT32)cpustate->r[1].r); break;
		case CPUINFO_STR_REGISTER + SHARC_R2:			sprintf(info->s, "R2: %08X", (UINT32)cpustate->r[2].r); break;
		case CPUINFO_STR_REGISTER + SHARC_R3:			sprintf(info->s, "R3: %08X", (UINT32)cpustate->r[3].r); break;
		case CPUINFO_STR_REGISTER + SHARC_R4:			sprintf(info->s, "R4: %08X", (UINT32)cpustate->r[4].r); break;
		case CPUINFO_STR_REGISTER + SHARC_R5:			sprintf(info->s, "R5: %08X", (UINT32)cpustate->r[5].r); break;
		case CPUINFO_STR_REGISTER + SHARC_R6:			sprintf(info->s, "R6: %08X", (UINT32)cpustate->r[6].r); break;
		case CPUINFO_STR_REGISTER + SHARC_R7:			sprintf(info->s, "R7: %08X", (UINT32)cpustate->r[7].r); break;
		case CPUINFO_STR_REGISTER + SHARC_R8:			sprintf(info->s, "R8: %08X", (UINT32)cpustate->r[8].r); break;
		case CPUINFO_STR_REGISTER + SHARC_R9:			sprintf(info->s, "R9: %08X", (UINT32)cpustate->r[9].r); break;
		case CPUINFO_STR_REGISTER + SHARC_R10:			sprintf(info->s, "R10: %08X", (UINT32)cpustate->r[10].r); break;
		case CPUINFO_STR_REGISTER + SHARC_R11:			sprintf(info->s, "R11: %08X", (UINT32)cpustate->r[11].r); break;
		case CPUINFO_STR_REGISTER + SHARC_R12:			sprintf(info->s, "R12: %08X", (UINT32)cpustate->r[12].r); break;
		case CPUINFO_STR_REGISTER + SHARC_R13:			sprintf(info->s, "R13: %08X", (UINT32)cpustate->r[13].r); break;
		case CPUINFO_STR_REGISTER + SHARC_R14:			sprintf(info->s, "R14: %08X", (UINT32)cpustate->r[14].r); break;
		case CPUINFO_STR_REGISTER + SHARC_R15:			sprintf(info->s, "R15: %08X", (UINT32)cpustate->r[15].r); break;

		case CPUINFO_STR_REGISTER + SHARC_I0:			sprintf(info->s, "I0: %08X", (UINT32)cpustate->dag1.i[0]); break;
		case CPUINFO_STR_REGISTER + SHARC_I1:			sprintf(info->s, "I1: %08X", (UINT32)cpustate->dag1.i[1]); break;
		case CPUINFO_STR_REGISTER + SHARC_I2:			sprintf(info->s, "I2: %08X", (UINT32)cpustate->dag1.i[2]); break;
		case CPUINFO_STR_REGISTER + SHARC_I3:			sprintf(info->s, "I3: %08X", (UINT32)cpustate->dag1.i[3]); break;
		case CPUINFO_STR_REGISTER + SHARC_I4:			sprintf(info->s, "I4: %08X", (UINT32)cpustate->dag1.i[4]); break;
		case CPUINFO_STR_REGISTER + SHARC_I5:			sprintf(info->s, "I5: %08X", (UINT32)cpustate->dag1.i[5]); break;
		case CPUINFO_STR_REGISTER + SHARC_I6:			sprintf(info->s, "I6: %08X", (UINT32)cpustate->dag1.i[6]); break;
		case CPUINFO_STR_REGISTER + SHARC_I7:			sprintf(info->s, "I7: %08X", (UINT32)cpustate->dag1.i[7]); break;
		case CPUINFO_STR_REGISTER + SHARC_I8:			sprintf(info->s, "I8: %08X", (UINT32)cpustate->dag2.i[0]); break;
		case CPUINFO_STR_REGISTER + SHARC_I9:			sprintf(info->s, "I9: %08X", (UINT32)cpustate->dag2.i[1]); break;
		case CPUINFO_STR_REGISTER + SHARC_I10:			sprintf(info->s, "I10: %08X", (UINT32)cpustate->dag2.i[2]); break;
		case CPUINFO_STR_REGISTER + SHARC_I11:			sprintf(info->s, "I11: %08X", (UINT32)cpustate->dag2.i[3]); break;
		case CPUINFO_STR_REGISTER + SHARC_I12:			sprintf(info->s, "I12: %08X", (UINT32)cpustate->dag2.i[4]); break;
		case CPUINFO_STR_REGISTER + SHARC_I13:			sprintf(info->s, "I13: %08X", (UINT32)cpustate->dag2.i[5]); break;
		case CPUINFO_STR_REGISTER + SHARC_I14:			sprintf(info->s, "I14: %08X", (UINT32)cpustate->dag2.i[6]); break;
		case CPUINFO_STR_REGISTER + SHARC_I15:			sprintf(info->s, "I15: %08X", (UINT32)cpustate->dag2.i[7]); break;

		case CPUINFO_STR_REGISTER + SHARC_M0:			sprintf(info->s, "M0: %08X", (UINT32)cpustate->dag1.m[0]); break;
		case CPUINFO_STR_REGISTER + SHARC_M1:			sprintf(info->s, "M1: %08X", (UINT32)cpustate->dag1.m[1]); break;
		case CPUINFO_STR_REGISTER + SHARC_M2:			sprintf(info->s, "M2: %08X", (UINT32)cpustate->dag1.m[2]); break;
		case CPUINFO_STR_REGISTER + SHARC_M3:			sprintf(info->s, "M3: %08X", (UINT32)cpustate->dag1.m[3]); break;
		case CPUINFO_STR_REGISTER + SHARC_M4:			sprintf(info->s, "M4: %08X", (UINT32)cpustate->dag1.m[4]); break;
		case CPUINFO_STR_REGISTER + SHARC_M5:			sprintf(info->s, "M5: %08X", (UINT32)cpustate->dag1.m[5]); break;
		case CPUINFO_STR_REGISTER + SHARC_M6:			sprintf(info->s, "M6: %08X", (UINT32)cpustate->dag1.m[6]); break;
		case CPUINFO_STR_REGISTER + SHARC_M7:			sprintf(info->s, "M7: %08X", (UINT32)cpustate->dag1.m[7]); break;
		case CPUINFO_STR_REGISTER + SHARC_M8:			sprintf(info->s, "M8: %08X", (UINT32)cpustate->dag2.m[0]); break;
		case CPUINFO_STR_REGISTER + SHARC_M9:			sprintf(info->s, "M9: %08X", (UINT32)cpustate->dag2.m[1]); break;
		case CPUINFO_STR_REGISTER + SHARC_M10:			sprintf(info->s, "M10: %08X", (UINT32)cpustate->dag2.m[2]); break;
		case CPUINFO_STR_REGISTER + SHARC_M11:			sprintf(info->s, "M11: %08X", (UINT32)cpustate->dag2.m[3]); break;
		case CPUINFO_STR_REGISTER + SHARC_M12:			sprintf(info->s, "M12: %08X", (UINT32)cpustate->dag2.m[4]); break;
		case CPUINFO_STR_REGISTER + SHARC_M13:			sprintf(info->s, "M13: %08X", (UINT32)cpustate->dag2.m[5]); break;
		case CPUINFO_STR_REGISTER + SHARC_M14:			sprintf(info->s, "M14: %08X", (UINT32)cpustate->dag2.m[6]); break;
		case CPUINFO_STR_REGISTER + SHARC_M15:			sprintf(info->s, "M15: %08X", (UINT32)cpustate->dag2.m[7]); break;

		case CPUINFO_STR_REGISTER + SHARC_L0:			sprintf(info->s, "L0: %08X", (UINT32)cpustate->dag1.l[0]); break;
		case CPUINFO_STR_REGISTER + SHARC_L1:			sprintf(info->s, "L1: %08X", (UINT32)cpustate->dag1.l[1]); break;
		case CPUINFO_STR_REGISTER + SHARC_L2:			sprintf(info->s, "L2: %08X", (UINT32)cpustate->dag1.l[2]); break;
		case CPUINFO_STR_REGISTER + SHARC_L3:			sprintf(info->s, "L3: %08X", (UINT32)cpustate->dag1.l[3]); break;
		case CPUINFO_STR_REGISTER + SHARC_L4:			sprintf(info->s, "L4: %08X", (UINT32)cpustate->dag1.l[4]); break;
		case CPUINFO_STR_REGISTER + SHARC_L5:			sprintf(info->s, "L5: %08X", (UINT32)cpustate->dag1.l[5]); break;
		case CPUINFO_STR_REGISTER + SHARC_L6:			sprintf(info->s, "L6: %08X", (UINT32)cpustate->dag1.l[6]); break;
		case CPUINFO_STR_REGISTER + SHARC_L7:			sprintf(info->s, "L7: %08X", (UINT32)cpustate->dag1.l[7]); break;
		case CPUINFO_STR_REGISTER + SHARC_L8:			sprintf(info->s, "L8: %08X", (UINT32)cpustate->dag2.l[0]); break;
		case CPUINFO_STR_REGISTER + SHARC_L9:			sprintf(info->s, "L9: %08X", (UINT32)cpustate->dag2.l[1]); break;
		case CPUINFO_STR_REGISTER + SHARC_L10:			sprintf(info->s, "L10: %08X", (UINT32)cpustate->dag2.l[2]); break;
		case CPUINFO_STR_REGISTER + SHARC_L11:			sprintf(info->s, "L11: %08X", (UINT32)cpustate->dag2.l[3]); break;
		case CPUINFO_STR_REGISTER + SHARC_L12:			sprintf(info->s, "L12: %08X", (UINT32)cpustate->dag2.l[4]); break;
		case CPUINFO_STR_REGISTER + SHARC_L13:			sprintf(info->s, "L13: %08X", (UINT32)cpustate->dag2.l[5]); break;
		case CPUINFO_STR_REGISTER + SHARC_L14:			sprintf(info->s, "L14: %08X", (UINT32)cpustate->dag2.l[6]); break;
		case CPUINFO_STR_REGISTER + SHARC_L15:			sprintf(info->s, "L15: %08X", (UINT32)cpustate->dag2.l[7]); break;

		case CPUINFO_STR_REGISTER + SHARC_B0:			sprintf(info->s, "B0: %08X", (UINT32)cpustate->dag1.b[0]); break;
		case CPUINFO_STR_REGISTER + SHARC_B1:			sprintf(info->s, "B1: %08X", (UINT32)cpustate->dag1.b[1]); break;
		case CPUINFO_STR_REGISTER + SHARC_B2:			sprintf(info->s, "B2: %08X", (UINT32)cpustate->dag1.b[2]); break;
		case CPUINFO_STR_REGISTER + SHARC_B3:			sprintf(info->s, "B3: %08X", (UINT32)cpustate->dag1.b[3]); break;
		case CPUINFO_STR_REGISTER + SHARC_B4:			sprintf(info->s, "B4: %08X", (UINT32)cpustate->dag1.b[4]); break;
		case CPUINFO_STR_REGISTER + SHARC_B5:			sprintf(info->s, "B5: %08X", (UINT32)cpustate->dag1.b[5]); break;
		case CPUINFO_STR_REGISTER + SHARC_B6:			sprintf(info->s, "B6: %08X", (UINT32)cpustate->dag1.b[6]); break;
		case CPUINFO_STR_REGISTER + SHARC_B7:			sprintf(info->s, "B7: %08X", (UINT32)cpustate->dag1.b[7]); break;
		case CPUINFO_STR_REGISTER + SHARC_B8:			sprintf(info->s, "B8: %08X", (UINT32)cpustate->dag2.b[0]); break;
		case CPUINFO_STR_REGISTER + SHARC_B9:			sprintf(info->s, "B9: %08X", (UINT32)cpustate->dag2.b[1]); break;
		case CPUINFO_STR_REGISTER + SHARC_B10:			sprintf(info->s, "B10: %08X", (UINT32)cpustate->dag2.b[2]); break;
		case CPUINFO_STR_REGISTER + SHARC_B11:			sprintf(info->s, "B11: %08X", (UINT32)cpustate->dag2.b[3]); break;
		case CPUINFO_STR_REGISTER + SHARC_B12:			sprintf(info->s, "B12: %08X", (UINT32)cpustate->dag2.b[4]); break;
		case CPUINFO_STR_REGISTER + SHARC_B13:			sprintf(info->s, "B13: %08X", (UINT32)cpustate->dag2.b[5]); break;
		case CPUINFO_STR_REGISTER + SHARC_B14:			sprintf(info->s, "B14: %08X", (UINT32)cpustate->dag2.b[6]); break;
		case CPUINFO_STR_REGISTER + SHARC_B15:			sprintf(info->s, "B15: %08X", (UINT32)cpustate->dag2.b[7]); break;
	}
}

CPU_GET_INFO( adsp21062 )
{
	switch(state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(adsp21062);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "ADSP21062");			break;

		default:										CPU_GET_INFO_CALL(sharc);				break;
	}
}

DEFINE_LEGACY_CPU_DEVICE(ADSP21062, adsp21062);
